/*
 * SuperH On-Chip RTC Support
 *
 * Copyright (C) 2006  Paul Mundt
 *
 * Based on the old arch/sh/kernel/cpu/rtc.c by:
 *
 *  Copyright (C) 2000  Philipp Rumpf <prumpf@tux.org>
 *  Copyright (C) 1999  Tetsuya Okada & Niibe Yutaka
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/bcd.h>
#include <linux/rtc.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/seq_file.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <asm/io.h>

#ifdef CONFIG_CPU_SH3
#define rtc_reg_size		sizeof(u16)
#define RTC_BIT_INVERTED	0	/* No bug on SH7708, SH7709A */
#elif defined(CONFIG_CPU_SH4)
#define rtc_reg_size		sizeof(u32)
#define RTC_BIT_INVERTED	0x40	/* bug on SH7750, SH7750S */
#endif

#define RTC_REG(r)	((r) * rtc_reg_size)

#define R64CNT  	RTC_REG(0)
#define RSECCNT 	RTC_REG(1)
#define RMINCNT 	RTC_REG(2)
#define RHRCNT  	RTC_REG(3)
#define RWKCNT  	RTC_REG(4)
#define RDAYCNT 	RTC_REG(5)
#define RMONCNT 	RTC_REG(6)
#define RYRCNT  	RTC_REG(7)
#define RSECAR  	RTC_REG(8)
#define RMINAR  	RTC_REG(9)
#define RHRAR   	RTC_REG(10)
#define RWKAR   	RTC_REG(11)
#define RDAYAR  	RTC_REG(12)
#define RMONAR  	RTC_REG(13)
#define RCR1    	RTC_REG(14)
#define RCR2    	RTC_REG(15)

/* RCR1 Bits */
#define RCR1_CF		0x80	/* Carry Flag             */
#define RCR1_CIE	0x10	/* Carry Interrupt Enable */
#define RCR1_AIE	0x08	/* Alarm Interrupt Enable */
#define RCR1_AF		0x01	/* Alarm Flag             */

/* RCR2 Bits */
#define RCR2_PEF	0x80	/* PEriodic interrupt Flag */
#define RCR2_PESMASK	0x70	/* Periodic interrupt Set  */
#define RCR2_RTCEN	0x08	/* ENable RTC              */
#define RCR2_ADJ	0x04	/* ADJustment (30-second)  */
#define RCR2_RESET	0x02	/* Reset bit               */
#define RCR2_START	0x01	/* Start bit               */

struct sh_rtc {
	void __iomem *regbase;
	unsigned long regsize;
	struct resource *res;
	unsigned int alarm_irq, periodic_irq, carry_irq;
	struct rtc_device *rtc_dev;
	spinlock_t lock;
};

static irqreturn_t sh_rtc_interrupt(int irq, void *id, struct pt_regs *regs)
{
	struct platform_device *pdev = id;
	struct sh_rtc *rtc = platform_get_drvdata(pdev);
	unsigned int tmp, events = 0;

	spin_lock(&rtc->lock);

	tmp = readb(rtc->regbase + RCR1);

	if (tmp & RCR1_AF)
		events |= RTC_AF | RTC_IRQF;

	tmp &= ~(RCR1_CF | RCR1_AF);

	writeb(tmp, rtc->regbase + RCR1);

	rtc_update_irq(&rtc->rtc_dev->class_dev, 1, events);

	spin_unlock(&rtc->lock);

	return IRQ_HANDLED;
}

static irqreturn_t sh_rtc_periodic(int irq, void *id, struct pt_regs *regs)
{
	struct sh_rtc *rtc = dev_get_drvdata(id);

	spin_lock(&rtc->lock);

	rtc_update_irq(&rtc->rtc_dev->class_dev, 1, RTC_PF | RTC_IRQF);

	spin_unlock(&rtc->lock);

	return IRQ_HANDLED;
}

static inline void sh_rtc_setpie(struct device *dev, unsigned int enable)
{
	struct sh_rtc *rtc = dev_get_drvdata(dev);
	unsigned int tmp;

	spin_lock_irq(&rtc->lock);

	tmp = readb(rtc->regbase + RCR2);

	if (enable) {
		tmp &= ~RCR2_PESMASK;
		tmp |= RCR2_PEF | (2 << 4);
	} else
		tmp &= ~(RCR2_PESMASK | RCR2_PEF);

	writeb(tmp, rtc->regbase + RCR2);

	spin_unlock_irq(&rtc->lock);
}

static inline void sh_rtc_setaie(struct device *dev, unsigned int enable)
{
	struct sh_rtc *rtc = dev_get_drvdata(dev);
	unsigned int tmp;

	spin_lock_irq(&rtc->lock);

	tmp = readb(rtc->regbase + RCR1);

	if (enable)
		tmp |= RCR1_AIE;
	else
		tmp &= ~RCR1_AIE;

	writeb(tmp, rtc->regbase + RCR1);

	spin_unlock_irq(&rtc->lock);
}

static int sh_rtc_open(struct device *dev)
{
	struct sh_rtc *rtc = dev_get_drvdata(dev);
	unsigned int tmp;
	int ret;

	tmp = readb(rtc->regbase + RCR1);
	tmp &= ~RCR1_CF;
	tmp |= RCR1_CIE;
	writeb(tmp, rtc->regbase + RCR1);

	ret = request_irq(rtc->periodic_irq, sh_rtc_periodic, SA_INTERRUPT,
			  "sh-rtc period", dev);
	if (unlikely(ret)) {
		dev_err(dev, "request period IRQ failed with %d, IRQ %d\n",
			ret, rtc->periodic_irq);
		return ret;
	}

	ret = request_irq(rtc->carry_irq, sh_rtc_interrupt, SA_INTERRUPT,
			  "sh-rtc carry", dev);
	if (unlikely(ret)) {
		dev_err(dev, "request carry IRQ failed with %d, IRQ %d\n",
			ret, rtc->carry_irq);
		free_irq(rtc->periodic_irq, dev);
		goto err_bad_carry;
	}

	ret = request_irq(rtc->alarm_irq, sh_rtc_interrupt, SA_INTERRUPT,
			  "sh-rtc alarm", dev);
	if (unlikely(ret)) {
		dev_err(dev, "request alarm IRQ failed with %d, IRQ %d\n",
			ret, rtc->alarm_irq);
		goto err_bad_alarm;
	}

	return 0;

err_bad_alarm:
	free_irq(rtc->carry_irq, dev);
err_bad_carry:
	free_irq(rtc->periodic_irq, dev);

	return ret;
}

static void sh_rtc_release(struct device *dev)
{
	struct sh_rtc *rtc = dev_get_drvdata(dev);

	sh_rtc_setpie(dev, 0);

	free_irq(rtc->periodic_irq, dev);
	free_irq(rtc->carry_irq, dev);
	free_irq(rtc->alarm_irq, dev);
}

static int sh_rtc_proc(struct device *dev, struct seq_file *seq)
{
	struct sh_rtc *rtc = dev_get_drvdata(dev);
	unsigned int tmp;

	tmp = readb(rtc->regbase + RCR1);
	seq_printf(seq, "alarm_IRQ\t: %s\n",
		   (tmp & RCR1_AIE) ? "yes" : "no");
	seq_printf(seq, "carry_IRQ\t: %s\n",
		   (tmp & RCR1_CIE) ? "yes" : "no");

	tmp = readb(rtc->regbase + RCR2);
	seq_printf(seq, "periodic_IRQ\t: %s\n",
		   (tmp & RCR2_PEF) ? "yes" : "no");

	return 0;
}

static int sh_rtc_ioctl(struct device *dev, unsigned int cmd, unsigned long arg)
{
	unsigned int ret = -ENOIOCTLCMD;

	switch (cmd) {
	case RTC_PIE_OFF:
	case RTC_PIE_ON:
		sh_rtc_setpie(dev, cmd == RTC_PIE_ON);
		ret = 0;
		break;
	case RTC_AIE_OFF:
	case RTC_AIE_ON:
		sh_rtc_setaie(dev, cmd == RTC_AIE_ON);
		ret = 0;
		break;
	}

	return ret;
}

static int sh_rtc_read_time(struct device *dev, struct rtc_time *tm)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct sh_rtc *rtc = platform_get_drvdata(pdev);
	unsigned int sec128, sec2, yr, yr100, cf_bit;

	do {
		unsigned int tmp;

		spin_lock_irq(&rtc->lock);

		tmp = readb(rtc->regbase + RCR1);
		tmp &= ~RCR1_CF; /* Clear CF-bit */
		tmp |= RCR1_CIE;
		writeb(tmp, rtc->regbase + RCR1);

		sec128 = readb(rtc->regbase + R64CNT);

		tm->tm_sec	= BCD2BIN(readb(rtc->regbase + RSECCNT));
		tm->tm_min	= BCD2BIN(readb(rtc->regbase + RMINCNT));
		tm->tm_hour	= BCD2BIN(readb(rtc->regbase + RHRCNT));
		tm->tm_wday	= BCD2BIN(readb(rtc->regbase + RWKCNT));
		tm->tm_mday	= BCD2BIN(readb(rtc->regbase + RDAYCNT));
		tm->tm_mon	= BCD2BIN(readb(rtc->regbase + RMONCNT));

#if defined(CONFIG_CPU_SH4)
		yr  = readw(rtc->regbase + RYRCNT);
		yr100 = BCD2BIN(yr >> 8);
		yr &= 0xff;
#else
		yr  = readb(rtc->regbase + RYRCNT);
		yr100 = BCD2BIN((yr == 0x99) ? 0x19 : 0x20);
#endif

		tm->tm_year = (yr100 * 100 + BCD2BIN(yr)) - 1900;

		sec2 = readb(rtc->regbase + R64CNT);
		cf_bit = readb(rtc->regbase + RCR1) & RCR1_CF;

		spin_unlock_irq(&rtc->lock);
	} while (cf_bit != 0 || ((sec128 ^ sec2) & RTC_BIT_INVERTED) != 0);

#if RTC_BIT_INVERTED != 0
	if ((sec128 & RTC_BIT_INVERTED))
		tm->tm_sec--;
#endif

	dev_dbg(&dev, "%s: tm is secs=%d, mins=%d, hours=%d, "
		"mday=%d, mon=%d, year=%d, wday=%d\n",
		__FUNCTION__,
		tm->tm_sec, tm->tm_min, tm->tm_hour,
		tm->tm_mday, tm->tm_mon, tm->tm_year, tm->tm_wday);

	if (rtc_valid_tm(tm) < 0)
		dev_err(dev, "invalid date\n");

	return 0;
}

static int sh_rtc_set_time(struct device *dev, struct rtc_time *tm)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct sh_rtc *rtc = platform_get_drvdata(pdev);
	unsigned int tmp;
	int year;

	spin_lock_irq(&rtc->lock);

	/* Reset pre-scaler & stop RTC */
	tmp = readb(rtc->regbase + RCR2);
	tmp |= RCR2_RESET;
	writeb(tmp, rtc->regbase + RCR2);

	writeb(BIN2BCD(tm->tm_sec),  rtc->regbase + RSECCNT);
	writeb(BIN2BCD(tm->tm_min),  rtc->regbase + RMINCNT);
	writeb(BIN2BCD(tm->tm_hour), rtc->regbase + RHRCNT);
	writeb(BIN2BCD(tm->tm_wday), rtc->regbase + RWKCNT);
	writeb(BIN2BCD(tm->tm_mday), rtc->regbase + RDAYCNT);
	writeb(BIN2BCD(tm->tm_mon),  rtc->regbase + RMONCNT);

#ifdef CONFIG_CPU_SH3
	year = tm->tm_year % 100;
	writeb(BIN2BCD(year), rtc->regbase + RYRCNT);
#else
	year = (BIN2BCD((tm->tm_year + 1900) / 100) << 8) |
		BIN2BCD(tm->tm_year % 100);
	writew(year, rtc->regbase + RYRCNT);
#endif

	/* Start RTC */
	tmp = readb(rtc->regbase + RCR2);
	tmp &= ~RCR2_RESET;
	tmp |= RCR2_RTCEN | RCR2_START;
	writeb(tmp, rtc->regbase + RCR2);

	spin_unlock_irq(&rtc->lock);

	return 0;
}

static struct rtc_class_ops sh_rtc_ops = {
	.open		= sh_rtc_open,
	.release	= sh_rtc_release,
	.ioctl		= sh_rtc_ioctl,
	.read_time	= sh_rtc_read_time,
	.set_time	= sh_rtc_set_time,
	.proc		= sh_rtc_proc,
};

static int __devinit sh_rtc_probe(struct platform_device *pdev)
{
	struct sh_rtc *rtc;
	struct resource *res;
	int ret = -ENOENT;

	rtc = kzalloc(sizeof(struct sh_rtc), GFP_KERNEL);
	if (unlikely(!rtc))
		return -ENOMEM;

	spin_lock_init(&rtc->lock);

	rtc->periodic_irq = platform_get_irq(pdev, 0);
	if (unlikely(rtc->periodic_irq < 0)) {
		dev_err(&pdev->dev, "No IRQ for period\n");
		goto err_badres;
	}

	rtc->carry_irq = platform_get_irq(pdev, 1);
	if (unlikely(rtc->carry_irq < 0)) {
		dev_err(&pdev->dev, "No IRQ for carry\n");
		goto err_badres;
	}

	rtc->alarm_irq = platform_get_irq(pdev, 2);
	if (unlikely(rtc->alarm_irq < 0)) {
		dev_err(&pdev->dev, "No IRQ for alarm\n");
		goto err_badres;
	}

	res = platform_get_resource(pdev, IORESOURCE_IO, 0);
	if (unlikely(res == NULL)) {
		dev_err(&pdev->dev, "No IO resource\n");
		goto err_badres;
	}

	rtc->regsize = res->end - res->start + 1;

	rtc->res = request_mem_region(res->start, rtc->regsize, pdev->name);
	if (unlikely(!rtc->res)) {
		ret = -EBUSY;
		goto err_badres;
	}

	rtc->regbase = (void __iomem *)rtc->res->start;
	if (unlikely(!rtc->regbase)) {
		ret = -EINVAL;
		goto err_badmap;
	}

	rtc->rtc_dev = rtc_device_register("sh", &pdev->dev,
					   &sh_rtc_ops, THIS_MODULE);
	if (IS_ERR(rtc)) {
		ret = PTR_ERR(rtc->rtc_dev);
		goto err_badmap;
	}

	platform_set_drvdata(pdev, rtc);

	return 0;

err_badmap:
	release_resource(rtc->res);
err_badres:
	kfree(rtc);

	return ret;
}

static int __devexit sh_rtc_remove(struct platform_device *pdev)
{
	struct sh_rtc *rtc = platform_get_drvdata(pdev);

	if (likely(rtc->rtc_dev))
		rtc_device_unregister(rtc->rtc_dev);

	sh_rtc_setpie(&pdev->dev, 0);
	sh_rtc_setaie(&pdev->dev, 0);

	release_resource(rtc->res);

	platform_set_drvdata(pdev, NULL);

	kfree(rtc);

	return 0;
}
static struct platform_driver sh_rtc_platform_driver = {
	.driver		= {
		.name	= "sh-rtc",
		.owner	= THIS_MODULE,
	},
	.probe		= sh_rtc_probe,
	.remove		= __devexit_p(sh_rtc_remove),
};

static int __init sh_rtc_init(void)
{
	return platform_driver_register(&sh_rtc_platform_driver);
}

static void __exit sh_rtc_exit(void)
{
	platform_driver_unregister(&sh_rtc_platform_driver);
}

module_init(sh_rtc_init);
module_exit(sh_rtc_exit);

MODULE_DESCRIPTION("SuperH on-chip RTC driver");
MODULE_AUTHOR("Paul Mundt <lethal@linux-sh.org>");
MODULE_LICENSE("GPL");