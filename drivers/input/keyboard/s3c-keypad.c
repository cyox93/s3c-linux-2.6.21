/*
 * drivers/input/keyboard/s3c-keypad.c
 * KeyPad Interface on S3C 
 *
 * $Id: s3c-keypad.c,v 1.2 2008/02/29 01:57:09 wizardsj Exp $
 * 
 * This file is subject to the terms and conditions of the GNU General Public
 * License. See the file COPYING in the main directory of this archive
 * for more details.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/delay.h>
#include <asm/irq.h>

#include <asm/arch/regs-gpio.h>
#include <asm/arch/regs-irq.h>
#include "s3c-keypad.h"

#ifdef S3C_KEYPAD_DEBUG
#undef S3C_KEYPAD_DEBUG
#endif

//#define S3C_KEYPAD_DEBUG 

#ifdef S3C_KEYPAD_DEBUG
#define DPRINTK(x...) printk("S3C-Keypad " x)
#else
#define DPRINTK(x...)		/* !!!! */
#endif

#define DEVICE_NAME "s3c-keypad"

#define TRUE 1
#define FALSE 0

static struct timer_list keypad_scan_timer;
static struct timer_list endkey_scan_timer;

static int curr_key_irq = 0;
static int key_irq_press = 0;
static int key_press = 0;
static int key_value = 0;
static int endkey_press = 0;

static void key_bh_handler(struct work_struct *work);

static void keypad_irq_mask(void)
{
	u32 mask;

	mask = __raw_readl(S3C2410_INTMSK);
	mask |= 0x21;

	__raw_writel(mask, S3C2410_INTMSK);
}

static void keypad_irq_unmask(void)
{
	u32 mask;

	mask = __raw_readl(S3C2410_INTMSK);
	mask &= ~(0x21);

	__raw_writel(mask, S3C2410_INTMSK);
}

static void keypad_irq_ack(void)
{
	__raw_writel(0x21, S3C2410_SRCPND);
	__raw_writel(0x21, S3C2410_INTPND);
}

static int keypad_scan_gpio_data(bool flag)
{
	u32 msk;

	msk = __raw_readl(S3C2410_GPDDAT) & ~(0x00007800);

	if (flag) 
		__raw_writel(msk | 0x00007800, S3C2410_GPDDAT);
	else
		__raw_writel(msk, S3C2410_GPDDAT);

	udelay(KEYPAD_DELAY);

	return 0;
}

static int keypad_scan_gpio_configure(int port)
{
	u32 con, udp, dat;
	
	/* GPDCON */
	con = __raw_readl(S3C2410_GPDCON) & ~(0x3fc00000);
	__raw_writel(con | (0x01 << (22 + (2 * port))), S3C2410_GPDCON);

	/* GPDUDP */
	udp = __raw_readl(S3C2410_GPDUP) & ~(0x3fc00000);
	__raw_writel(udp | (0x01 << (22 + (2 * port))), S3C2410_GPDUP);

	/* GPDDAT */
	dat = __raw_readl(S3C2410_GPDDAT) & ~(0x00007800);
	__raw_writel(dat | ~(0x01 << (11 + port)), S3C2410_GPDDAT);
	
	udelay(KEYPAD_DELAY);

	return 0;
}

static int keypad_irq_configure(void)
{
	u32 con, udp, dat, msk;

	msk = __raw_readl(S3C2410_GPGCON) & ~(0x00000fff);
	con = __raw_readl(S3C2410_GPDCON) & ~(0x3fc00000);
	udp = __raw_readl(S3C2410_GPDUP) & ~(0x3fc00000);
	dat = __raw_readl(S3C2410_GPDDAT) & ~(0x00007800);

	__raw_writel(msk | 0x00000aaa, S3C2410_GPGCON);
	__raw_writel(con | 0x15400000, S3C2410_GPDCON);
	__raw_writel(udp | 0x15400000, S3C2410_GPDUP);
	__raw_writel(dat, S3C2410_GPDDAT);

	return 0;
}

static int keypad_scan(struct s3c_keypad *pdata)
{
	int i,j;
	int col_cnt = 0, row_cnt = 0;
	int col_val = 0, row_val = 0, value;
	int ret = KEY_PRESS_STATE;
	u32 mask, rval, rval_sum = 0;
	struct input_dev *dev = pdata->dev;

	/* keypad scan input setting */
	mask = __raw_readl(S3C2410_GPGCON) & ~(0x00000fff);
	__raw_writel(mask, S3C2410_GPGCON); 

	keypad_scan_gpio_data(true);
	udelay(KEYPAD_DELAY);

	/* keypad scan routine */
	col_cnt = 0;
	for (i=0; i<KEYPAD_COLUMNS; i++) {
		keypad_scan_gpio_configure(i);

		rval = __raw_readl(S3C2410_GPGDAT);
		rval &= 0x3f;
		rval_sum += rval;

		if ((rval != 0x3f)) {
			row_cnt = 0;
			for (j=0; j<KEYPAD_ROWS; j++) {
				if (((~rval) >> j) & 0x01) {
					if (col_cnt || row_val) {
						return KEY_PRESS_STATE;
					}
					
					col_val = i;
					row_val = j;
					col_cnt++;
					row_cnt++;
				}
			}
		}
	}
	
	value = (col_val * 6) + row_val;

	/* key press event */
	if (col_cnt && row_cnt && !key_press && 
			((rval_sum/4) != 0x3f)) {

		input_report_key(dev, pdata->keycodes[value], 1);
		DPRINTK("key pressed : %d\n", pdata->keycodes[value]);

		key_value = value;
		key_press = 1;
		
		return KEY_PRESS_STATE;
	}
	
	/* key release event */
	if (col_cnt && row_cnt && key_press && 
			((rval_sum/4) != 0x3f) && (key_value != value)) {
		input_report_key(dev, pdata->keycodes[key_value], 0);
		DPRINTK("key released : %d\n", pdata->keycodes[key_value]);
		key_press = 0;

		return KEY_PRESS_STATE;
	}

	/* key release event */
	if (!col_cnt && !row_cnt && ((rval_sum/4) == 0x3f)) {
		input_report_key(dev, pdata->keycodes[key_value], 0);
		DPRINTK("key released : %d\n", pdata->keycodes[key_value]);
		key_press = 0;
		
		keypad_scan_gpio_data(false);
		keypad_irq_configure();
		return KEY_RELEASE_STATE;
	}

	return ret;
}

static int endkey_scan(struct s3c_keypad *pdata)
{
	u32 endkey;
	int ret = KEY_PRESS_STATE;
	struct input_dev *dev = pdata->dev;

	endkey = __raw_readl(S3C2410_GPFDAT) & 0x01;

	if (!endkey && !endkey_press) {
		endkey_press = 1;
		
		input_report_key(dev, pdata->keycodes[12], 1);
		input_sync(dev);

		DPRINTK("key Pressed : %d\n", pdata->keycodes[12]);

		return KEY_PRESS_STATE;
	}

	if (endkey && endkey_press) {
		input_report_key(dev, pdata->keycodes[12], 0);
		input_sync(dev);
		
		endkey_press = 0;

		DPRINTK("key Released : %d\n", pdata->keycodes[12]);
		
		ret = KEY_RELEASE_STATE;
	}

	return ret;
}

static void keypad_scan_timer_handler(unsigned long data)
{	
	struct s3c_keypad *pdata = (struct s3c_keypad *)data;
	
	if (keypad_scan(pdata) == KEY_PRESS_STATE) {
		del_timer(&keypad_scan_timer);
		keypad_scan_timer.expires = jiffies + msecs_to_jiffies(50);
		add_timer(&keypad_scan_timer);
	}
	else {
		keypad_irq_ack();
		keypad_irq_unmask();
		key_irq_press = 0;
	}
}

static void endkey_scan_timer_handler(unsigned long data)
{
	struct s3c_keypad *pdata = (struct s3c_keypad *)data;

	if (endkey_scan(pdata) == KEY_PRESS_STATE) {
		del_timer(&endkey_scan_timer);
		endkey_scan_timer.expires = jiffies + msecs_to_jiffies(50);
		add_timer(&endkey_scan_timer);
	}
	else {
		keypad_irq_ack();
		keypad_irq_unmask();
		key_irq_press = 0;
	}
}

static void key_bh_handler(struct work_struct *work)
{
	if (curr_key_irq == 16) {
		endkey_scan_timer.expires = jiffies + msecs_to_jiffies(0);
		add_timer(&endkey_scan_timer);
	}
	else {
		keypad_scan_timer.expires = jiffies + msecs_to_jiffies(0);
		add_timer(&keypad_scan_timer);
	}
}

static irqreturn_t s3c_keypad_isr(int irq, void *data)
{
	struct s3c_keypad *s3c_keypad = (struct s3c_keypad *)data;

	if (key_irq_press) goto out;

	keypad_irq_mask();
	key_irq_press = 1;
	curr_key_irq = irq;

	schedule_work(&s3c_keypad->work);

out:
	return IRQ_HANDLED;
}

static int s3c_keypad_request_irq(struct s3c_keypad *keypad)
{
	int ret = 0;

	ret = request_irq(IRQ_EINT0, s3c_keypad_isr, SA_INTERRUPT|SA_TRIGGER_FALLING,
			DEVICE_NAME, (void *)keypad);
	if (ret)
		printk("request_irq failed (IRQ_EINT0)\n");

	ret = request_irq(IRQ_EINT8, s3c_keypad_isr, SA_INTERRUPT|SA_TRIGGER_FALLING,
			DEVICE_NAME, (void *)keypad);
	if (ret)
		printk("request_irq failed (IRQ_EINT8)\n");

	ret = request_irq(IRQ_EINT9, s3c_keypad_isr, SA_INTERRUPT|SA_TRIGGER_FALLING,
			DEVICE_NAME, (void *)keypad);
	if (ret)
		printk("request_irq failed (IRQ_EINT9)\n");

	ret = request_irq(IRQ_EINT10, s3c_keypad_isr, SA_INTERRUPT|SA_TRIGGER_FALLING,
			DEVICE_NAME, (void *)keypad);
	if (ret)
		printk("request_irq failed (IRQ_EINT10)\n");

	ret = request_irq(IRQ_EINT11, s3c_keypad_isr, SA_INTERRUPT|SA_TRIGGER_FALLING,
			DEVICE_NAME, (void *)keypad);
	if (ret)
		printk("request_irq failed (IRQ_EINT11)\n");

	ret = request_irq(IRQ_EINT12, s3c_keypad_isr, SA_INTERRUPT|SA_TRIGGER_FALLING,
			DEVICE_NAME, (void *)keypad);
	if (ret)
		printk("request_irq failed (IRQ_EINT12)\n");

	ret = request_irq(IRQ_EINT13, s3c_keypad_isr, SA_INTERRUPT|SA_TRIGGER_FALLING,
			DEVICE_NAME, (void *)keypad);
	if (ret)
		printk("request_irq failed (IRQ_EINT13)\n");

	keypad_irq_unmask();

	return ret;
}

static int s3c_keypad_free_irq(struct s3c_keypad *keypad)
{
	free_irq(IRQ_EINT0, (void *)keypad);
	free_irq(IRQ_EINT8, (void *)keypad);
	free_irq(IRQ_EINT9, (void *)keypad);
	free_irq(IRQ_EINT10, (void *)keypad);
	free_irq(IRQ_EINT11, (void *)keypad);
	free_irq(IRQ_EINT12, (void *)keypad);
	free_irq(IRQ_EINT13, (void *)keypad);

	return 0;
}

static int __init s3c_keypad_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct input_dev *input_dev;
	int key, code;
	struct s3c_keypad *s3c_keypad;

	s3c_keypad = kzalloc(sizeof(struct s3c_keypad), GFP_KERNEL);
	input_dev = input_allocate_device();

	if (!s3c_keypad || !input_dev) {
		kfree(s3c_keypad);
		input_free_device(input_dev);
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, s3c_keypad);
	s3c_keypad->dev = input_dev;

	/* create and register the input driver */
	set_bit(EV_KEY, input_dev->evbit);
	set_bit(EV_REP, input_dev->evbit);
	s3c_keypad->nr_rows = KEYPAD_ROWS;
	s3c_keypad->no_cols = KEYPAD_COLUMNS;
	s3c_keypad->total_keys = MAX_KEYPAD_NR;

	for(key = 0; key < s3c_keypad->total_keys; key++){
		code = s3c_keypad->keycodes[key] = keypad_keycode[key];
		if(code<=0)
			continue;
		set_bit(code & KEY_MAX, input_dev->keybit);
	}

	input_dev->name = DEVICE_NAME;
	input_dev->phys = "s3c-keypad/input0";
	input_dev->cdev.dev = &pdev->dev;
	input_dev->private = s3c_keypad;
	
	input_dev->id.bustype = BUS_HOST;
	input_dev->id.vendor = 0x0001;
	input_dev->id.product = 0x0001;
	input_dev->id.version = 0x0001;

	input_dev->keycode = keypad_keycode;
	
	input_register_device(input_dev);

	/* Scan timer init */
	init_timer(&keypad_scan_timer);
	keypad_scan_timer.function = keypad_scan_timer_handler;
	keypad_scan_timer.data = (unsigned long)s3c_keypad;

	/* Scan endkey timer init */
	init_timer(&endkey_scan_timer);
	endkey_scan_timer.function = endkey_scan_timer_handler;
	endkey_scan_timer.data = (unsigned long)s3c_keypad;
	
	INIT_WORK(&s3c_keypad->work, key_bh_handler);
	ret = s3c_keypad_request_irq(s3c_keypad);
	if (ret)
		goto out;

	printk( DEVICE_NAME " Initialized\n");
	return 0;

out:
	input_unregister_device(input_dev);
	kfree(s3c_keypad);
	s3c_keypad_free_irq(s3c_keypad);
	return ret;
}

static int s3c_keypad_remove(struct platform_device *pdev)
{
	struct s3c_keypad *s3c_keypad = platform_get_drvdata(pdev);

	input_unregister_device(s3c_keypad->dev);
	kfree(pdev->dev.platform_data);
	s3c_keypad_free_irq(s3c_keypad);

	del_timer(&keypad_scan_timer);	
	printk(DEVICE_NAME " Removed.\n");
	return 0;
}

#ifdef CONFIG_PM
static int s3c_keypad_suspend(struct platform_device *dev, pm_message_t state)
{
	/* TODO */
	return 0;
}

static int s3c_keypad_resume(struct platform_device *dev)
{
	/* TODO */
	return 0;
}
#else
#define s3c_keypad_suspend NULL
#define s3c_keypad_resume  NULL
#endif /* CONFIG_PM */

static struct platform_driver s3c_keypad_driver = {
	.probe		= s3c_keypad_probe,
	.remove		= s3c_keypad_remove,
	.suspend	= s3c_keypad_suspend,
	.resume		= s3c_keypad_resume,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "s3c-keypad",
	},
};

static int __init s3c_keypad_init(void)
{
	int ret;

	ret = platform_driver_register(&s3c_keypad_driver);
	
	if(!ret)
	   printk(KERN_INFO "S3C Keypad Driver\n");

	return ret;
}

static void __exit s3c_keypad_exit(void)
{
	platform_driver_unregister(&s3c_keypad_driver);
}

module_init(s3c_keypad_init);
module_exit(s3c_keypad_exit);

MODULE_AUTHOR("Samsung");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("KeyPad interface for Samsung S3C");
