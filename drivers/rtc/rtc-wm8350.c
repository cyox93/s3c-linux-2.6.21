/*
 *	Real Time Clock driver for Wolfson Microelectronics WM8350
 *
 *	Copyright (C) 2007 Wolfson Microelectronics PLC.
 *
 *  Author: Liam Girdwood
 *          liam.girdwood@wolfsonmicro.com or linux@wolfsonmicro.com
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  Revision history
 *    10th Apr 2007   Initial version.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/rtc.h>
#include <linux/bcd.h>
#include <linux/interrupt.h>
#include <linux/ioctl.h>
#include <linux/completion.h>
#include <linux/mfd/wm8350/rtc.h>
#include <linux/mfd/wm8350/core.h>
#include <linux/delay.h>
#include <linux/platform_device.h>

#define WM8350_RTC_VERSION	"0.2"
#define WM8350_SET_ALM_RETRIES	5
#define WM8350_SET_TIME_RETRIES	5
#define WM8350_GET_TIME_RETRIES	5

/*
 * Debug
 */
#define WM8350_DEBUG 0

#ifdef WM8350_DEBUG
#define dbg(format, arg...) \
	printk(KERN_DEBUG ": " format "\n" , ## arg)
#else
#define dbg(format, arg...) do {} while (0)
#endif

/*
 * Read current time and date in RTC
 */
static int wm8350_rtc_readtime(struct device *dev, struct rtc_time *tm)
{
	struct wm8350_rtc *wm_rtc = to_wm8350_rtc_device(dev);
	struct wm8350 *wm8350 = to_wm8350_from_rtc(wm_rtc);
	u16 time1[4], time2[4];
	int retries = WM8350_GET_TIME_RETRIES, ret;

	/*
	 * Read the time twice and compare.
	 * If time1 == time2, then time is valid else retry.
	 */
	do {
		ret = wm8350_block_read(wm8350, WM8350_RTC_SECONDS_MINUTES,
					4, time1);
		if (ret < 0)
			return ret;
		ret = wm8350_block_read(wm8350, WM8350_RTC_SECONDS_MINUTES,
					4, time2);
		if (ret < 0)
			return ret;

		if (!memcmp(time1, time2, 8)) {
			tm->tm_sec = time1[0] & WM8350_RTC_SECS_MASK;

			tm->tm_min = (time1[0] & WM8350_RTC_MINS_MASK)
			    >> WM8350_RTC_MINS_SHIFT;

			tm->tm_hour = time1[1] & WM8350_RTC_HRS_MASK;

			tm->tm_wday = ((time1[1] >> WM8350_RTC_DAY_SHIFT)
				       & 0x7) - 1;

			tm->tm_mon = ((time1[2] & WM8350_RTC_MTH_MASK)
				      >> WM8350_RTC_MTH_SHIFT) - 1;

			tm->tm_mday = (time1[2] & WM8350_RTC_DATE_MASK);

			tm->tm_year = ((time1[3] & WM8350_RTC_YHUNDREDS_MASK)
				       >> WM8350_RTC_YHUNDREDS_SHIFT) * 100;
			tm->tm_year += time1[3] & WM8350_RTC_YUNITS_MASK;

			tm->tm_yday = rtc_year_days(tm->tm_mday, tm->tm_mon,
						    tm->tm_year);
			tm->tm_year -= 1900;

			return 0;
		}
	} while (retries--);

	printk(KERN_ERR "wm8350-rtc: failed to read RTC time\n");
	return -EIO;
}

/*
 * Set current time and date in RTC
 */
static int wm8350_rtc_settime(struct device *dev, struct rtc_time *tm)
{
	struct wm8350_rtc *wm_rtc = to_wm8350_rtc_device(dev);
	struct wm8350 *wm8350 = to_wm8350_from_rtc(wm_rtc);
	u16 time[4];
	u16 rtc_ctrl;
	int ret, retries = WM8350_SET_TIME_RETRIES;

	dbg("%s tm->tm_wday %d, tm->tm_mon %d",
	    __FUNCTION__, tm->tm_wday, tm->tm_mon);
	time[0] = tm->tm_sec;
	time[0] |= tm->tm_min << WM8350_RTC_MINS_SHIFT;
	time[1] = tm->tm_hour;
	time[1] |= (tm->tm_wday + 1) << WM8350_RTC_DAY_SHIFT;
	time[2] = tm->tm_mday;
	time[2] |= (tm->tm_mon + 1) << WM8350_RTC_MTH_SHIFT;
	time[3] = ((tm->tm_year + 1900) / 100) << WM8350_RTC_YHUNDREDS_SHIFT;
	time[3] |= (tm->tm_year + 1900) % 100;

	/* Set RTC_SET to stop the clock */
	ret = wm8350_set_bits(wm8350, WM8350_RTC_TIME_CONTROL, WM8350_RTC_SET);
	if (ret < 0)
		return ret;

	/* Wait until confirmation of stopping */
	do {
		rtc_ctrl = wm8350_reg_read(wm8350, WM8350_RTC_TIME_CONTROL);
		schedule_timeout_interruptible(msecs_to_jiffies(1));
	} while (retries-- && !(rtc_ctrl & WM8350_RTC_STS));

	if (!retries) {
		printk(KERN_ERR "wm8350-rtc time out on set confirmation\n");
		return -EIO;
	}

	/* Write time to RTC */
	ret = wm8350_block_write(wm8350, WM8350_RTC_SECONDS_MINUTES, 4, time);
	if (ret < 0)
		return ret;

	/* Clear RTC_SET to start the clock */
	ret = wm8350_clear_bits(wm8350, WM8350_RTC_TIME_CONTROL,
				WM8350_RTC_SET);
	return ret;
}

/*
 * Read alarm time and date in RTC
 */
static int wm8350_rtc_readalarm(struct device *dev, struct rtc_wkalrm *alrm)
{
	struct wm8350_rtc *wm_rtc = to_wm8350_rtc_device(dev);
	struct wm8350 *wm8350 = to_wm8350_from_rtc(wm_rtc);
	struct rtc_time *tm = &alrm->time;
	u16 time[3];
	int ret;

	ret = wm8350_block_read(wm8350, WM8350_ALARM_SECONDS_MINUTES, 3, time);
	if (ret < 0)
		return ret;

	tm->tm_sec = time[0] & WM8350_RTC_ALMSECS_MASK;
	tm->tm_min = (time[0] & WM8350_RTC_ALMMINS_MASK)
	    >> WM8350_RTC_ALMMINS_SHIFT;
	tm->tm_hour = time[1] & WM8350_RTC_ALMHRS_MASK;
	tm->tm_wday = -1;
	tm->tm_mday = -1;
	tm->tm_mon = -1;
	tm->tm_year = -1;

	return 0;
}

/*
 * Set alarm time and date in RTC
 */
static int wm8350_rtc_setalarm(struct device *dev, struct rtc_wkalrm *alrm)
{
	struct wm8350_rtc *wm_rtc = to_wm8350_rtc_device(dev);
	struct wm8350 *wm8350 = to_wm8350_from_rtc(wm_rtc);
	struct rtc_time *tm = &alrm->time;
	u16 rtc_ctrl;
	u16 time[3];
	int ret, retries = WM8350_SET_ALM_RETRIES;

	time[0] = tm->tm_sec;
	time[0] |= tm->tm_min << WM8350_RTC_ALMMINS_SHIFT;
	time[1] = tm->tm_hour;
	time[1] |= WM8350_RTC_ALMDAY_MASK;
	time[2] = WM8350_RTC_ALMDATE_MASK;
	time[2] |= WM8350_RTC_ALMMTH_MASK;

	/* Set RTC_SET to stop the clock */
	ret = wm8350_set_bits(wm8350, WM8350_RTC_TIME_CONTROL,
			      WM8350_RTC_ALMSET);
	if (ret < 0)
		return ret;

	/* Wait until confirmation of stopping */
	do {
		rtc_ctrl = wm8350_reg_read(wm8350, WM8350_RTC_TIME_CONTROL);
		schedule_timeout_interruptible(msecs_to_jiffies(1));
	} while (retries-- && !(rtc_ctrl & WM8350_RTC_ALMSTS));

	/* Write time to RTC */
	ret = wm8350_block_write(wm8350, WM8350_ALARM_SECONDS_MINUTES,
				 3, time);
	if (ret < 0)
		return ret;

	/* Clear RTC_ALMSET to enable the alarm */
	ret = wm8350_clear_bits(wm8350, WM8350_RTC_TIME_CONTROL,
				WM8350_RTC_ALMSET);
	return ret;
}

static int wm8350_rtc_set_pie(struct wm8350_rtc *wm_rtc, unsigned long freq)
{
	struct wm8350 *wm8350 = to_wm8350_from_rtc(wm_rtc);
	int ret, power;
	u16 reg;

	power = ffs(freq);
	if ((power == fls(freq)) && freq < 1025) {
		wm_rtc->pie_freq = freq;
		reg = wm8350_reg_read(wm8350, WM8350_RTC_TIME_CONTROL);
		reg &= ~WM8350_RTC_DSW_MASK;
		reg |= (power + 1) << WM8350_RTC_DSW_SHIFT;
		ret = wm8350_reg_write(wm8350, WM8350_RTC_TIME_CONTROL, reg);
		return ret;
	}
	return -EINVAL;
}

/*
 * Handle commands from user-space
 */
static int wm8350_rtc_ioctl(struct device *dev, unsigned int cmd,
			    unsigned long arg)
{
	struct wm8350_rtc *wm_rtc = to_wm8350_rtc_device(dev);
	struct wm8350 *wm8350 = to_wm8350_from_rtc(wm_rtc);

	switch (cmd) {
	case RTC_AIE_OFF:	/* alarm off */
		wm8350_mask_irq(wm8350, WM8350_IRQ_RTC_ALM);
		wm_rtc->alarm_enabled = 0;
		break;
	case RTC_AIE_ON:	/* alarm on */
		wm8350_unmask_irq(wm8350, WM8350_IRQ_RTC_ALM);
		wm_rtc->alarm_enabled = 1;
		break;
	case RTC_UIE_OFF:	/* update off */
		wm8350_mask_irq(wm8350, WM8350_IRQ_RTC_SEC);
		wm_rtc->update_enabled = 0;
		break;
	case RTC_UIE_ON:	/* update on */
		wm8350_unmask_irq(wm8350, WM8350_IRQ_RTC_SEC);
		wm_rtc->update_enabled = 1;
		break;
	case RTC_PIE_ON:
		if (!wm_rtc->pie_enabled) {
			enable_irq(wm_rtc->per_irq);
			wm_rtc->pie_enabled = 1;
		}
		break;
	case RTC_PIE_OFF:
		if (wm_rtc->pie_enabled) {
			disable_irq(wm_rtc->per_irq);
			wm_rtc->pie_enabled = 0;
		}
		break;
	case RTC_IRQP_READ:	/* read periodic alarm frequency */
		return wm_rtc->pie_freq;
	case RTC_IRQP_SET:	/* set periodic alarm frequency */
		return wm8350_rtc_set_pie(wm_rtc, arg);
	default:
		return -ENOIOCTLCMD;
	}

	return 0;
}

/*
 * Provide additional RTC information in /proc/driver/rtc
 */
static int wm8350_rtc_proc(struct device *dev, struct seq_file *seq)
{
	struct wm8350_rtc *wm_rtc = to_wm8350_rtc_device(dev);

	seq_printf(seq, "update_IRQ\t: %s\n",
		   wm_rtc->update_enabled ? "yes" : "no");
	seq_printf(seq, "periodic_IRQ\t: %s\n",
		   wm_rtc->pie_enabled ? "yes" : "no");
	seq_printf(seq, "periodic_freq\t: %ld\n",
		   (unsigned long)wm_rtc->pie_freq);
	return 0;
}

static void wm8350_rtc_alarm_handler(struct wm8350 *wm8350, int irq, void *data)
{
	struct rtc_device *rtc = wm8350->rtc.rtc;

	rtc_update_irq(rtc, 1, RTC_IRQF | RTC_AF);
}

static void wm8350_rtc_update_handler(struct wm8350 *wm8350, int irq,
				      void *data)
{
	struct rtc_device *rtc = wm8350->rtc.rtc;

	rtc_update_irq(rtc, 1, RTC_IRQF | RTC_UF);
}

/*
 * Optional IRQ handler for the RTC DSW gpio.
 * This will usually have it's own irq pin to the CPU in order to minimize
 * any latency and as such we must pass in the irq number.
 */
static irqreturn_t wm8350_rtc_periodic_irq(int irq, void *dev_id)
{
	struct device *dev = dev_id;
	struct wm8350_rtc *wm_rtc = to_wm8350_rtc_device(dev);

	rtc_update_irq(wm_rtc->rtc, 1, RTC_IRQF | RTC_PF);
	return 0;
}

static const struct rtc_class_ops wm8350_rtc_ops = {
	.ioctl = wm8350_rtc_ioctl,
	.read_time = wm8350_rtc_readtime,
	.set_time = wm8350_rtc_settime,
	.read_alarm = wm8350_rtc_readalarm,
	.set_alarm = wm8350_rtc_setalarm,
	.proc = wm8350_rtc_proc,
};

static int wm8350_rtc_probe(struct platform_device *pdev)
{
	struct wm8350 *wm8350 = platform_get_drvdata(pdev);
	struct wm8350_rtc *wm_rtc = &wm8350->rtc;
	int ret = 0;

	/* enable the RTC */
	ret = wm8350_set_bits(wm8350, WM8350_POWER_MGMT_5, WM8350_RTC_TICK_ENA);
	if (ret < 0) {
		printk(KERN_ERR "wm8350-rtc: failed to enable RTC\n");
		return ret;
	}

	wm_rtc->rtc = rtc_device_register("wm8350-rtc", &pdev->dev,
					  &wm8350_rtc_ops, THIS_MODULE);
	if (IS_ERR(wm_rtc->rtc)) {
		printk(KERN_ERR "wm8350-rtc: failed to register RTC/n");
		return PTR_ERR(wm_rtc->rtc);
	}
	printk("wm8350: RTC version %s\n", WM8350_RTC_VERSION);

	wm8350_register_irq(wm8350, WM8350_IRQ_RTC_SEC,
			    wm8350_rtc_update_handler, NULL);
	wm8350_register_irq(wm8350, WM8350_IRQ_RTC_ALM,
			    wm8350_rtc_alarm_handler, NULL);
	if (wm_rtc->per_irq) {
		ret = request_irq(wm_rtc->per_irq, wm8350_rtc_periodic_irq,
				  IRQF_DISABLED, "wm8350-rtc timer",
				  &pdev->dev);
		if (ret < 0) {
			printk(KERN_ERR
			       "wm8350-rtc: failed to get timer irq %d\n",
			       wm_rtc->per_irq);
			wm_rtc->per_irq = 0;
		}
	}

	return 0;
}

static int __exit wm8350_rtc_remove(struct platform_device *pdev)
{
	struct wm8350 *wm8350 = platform_get_drvdata(pdev);
	struct wm8350_rtc *wm_rtc = &wm8350->rtc;
	int ret;

	if (wm_rtc->per_irq)
		free_irq(wm_rtc->per_irq, wm8350);
	wm8350_mask_irq(wm8350, WM8350_IRQ_RTC_SEC);
	wm8350_mask_irq(wm8350, WM8350_IRQ_RTC_ALM);
	wm8350_free_irq(wm8350, WM8350_IRQ_RTC_SEC);
	wm8350_free_irq(wm8350, WM8350_IRQ_RTC_ALM);
	rtc_device_unregister(wm_rtc->rtc);

	ret = wm8350_clear_bits(wm8350, WM8350_POWER_MGMT_5,
				WM8350_RTC_TICK_ENA);
	if (ret < 0)
		printk(KERN_ERR "wm8350-rtc: failed to enable RTC\n");

	return 0;
}

struct platform_driver wm8350_rtc_driver = {
	.probe = wm8350_rtc_probe,
	.remove = wm8350_rtc_remove,
	.driver = {
		.name = "wm8350-rtc",
	},
};

static int __init wm8350_rtc_init(void)
{
	return platform_driver_register(&wm8350_rtc_driver);
}

static void __exit wm8350_rtc_exit(void)
{
	platform_driver_unregister(&wm8350_rtc_driver);
}

module_init(wm8350_rtc_init);
module_exit(wm8350_rtc_exit);

MODULE_AUTHOR("Liam Girdwood");
MODULE_DESCRIPTION("RTC driver WM8350");
MODULE_LICENSE("GPL");
