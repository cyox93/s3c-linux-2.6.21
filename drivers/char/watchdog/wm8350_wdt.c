/*
 * Watchdog driver for the wm8350
 *
 * Copyright (C) 2007 Wolfson Microelectronics <linux@wolfsonmicro.com>
 * Based on SoftDog driver by Alan Cox <alan@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation
 *
 * 27/04/2007 Initial release
 */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/watchdog.h>
#include <linux/init.h>
#include <linux/mfd/wm8350/core.h>

#define WM8350_WDT_VERSION "0.2"

static unsigned long wm8350_wdt_users;
static struct miscdevice wm8350_wdt_miscdev;

static int wm8350_wdt_configure(struct wm8350 *wm8350, u16 value)
{
	int ret;
	u16 reg;

	/* Unlock the Register */
	wm8350_reg_unlock(wm8350);

	/* Write to Watchdog Config Register */
	reg = wm8350_reg_read(wm8350, WM8350_SYSTEM_CONTROL_2) & 0xff80;
	ret = wm8350_reg_write(wm8350, WM8350_SYSTEM_CONTROL_2, reg | value);

	/* Lock the Register */
	wm8350_reg_lock(wm8350);
	return ret;
}

static int wm8350_wdt_kick(struct wm8350 *wm8350)
{
	int ret;
	u16 reg;

	/* keep alive */
	reg = wm8350_reg_read(wm8350, WM8350_SYSTEM_CONTROL_2) & 0xff80;
	ret = wm8350_reg_write(wm8350, WM8350_SYSTEM_CONTROL_2, reg);

	return ret;
}

/*
 * Allow only one person to hold it open
 */
static int wm8350_wdt_open(struct inode *inode, struct file *file)
{
	nonseekable_open(inode, file);
	if (test_and_set_bit(1, &wm8350_wdt_users))
		return -EBUSY;

	return 0;
}

static int wm8350_wdt_release(struct inode *inode, struct file *file)
{
	printk(KERN_CRIT "WATCHDOG: Device closed - timer will not stop\n");
	clear_bit(1, &wm8350_wdt_users);
	return 0;
}

static ssize_t wm8350_wdt_write(struct file *file,
				const char __user *data, size_t len,
				loff_t *ppos)
{
	struct wm8350_wdg *wm_wdt =
	    to_wm8350_wdg_device(wm8350_wdt_miscdev.this_device);
	struct wm8350 *wm8350 = to_wm8350_from_wdg(wm_wdt);

	if (len)
		/* Write to Watchdog Config Register */
		wm8350_reg_write(wm8350, WM8350_SYSTEM_CONTROL_2, 0);

	return len;
}

static struct watchdog_info ident = {
	.options = WDIOF_SETTIMEOUT | WDIOF_KEEPALIVEPING,
	.identity = "WM8350 Watchdog",
};

/* Use the interrupt then reset to get us 8 secs */
static u16 wm8350_wdt_configs[] = {
	0x00,			/* Watchdog disabled */
	0x23,			/* 1 second */
	0x24,			/* 2 second */
	0x25,			/* 4 second */
	0x25,			/* 4 second */
	0x35,			/* 8 second */
	0x35,			/* 8 second */
	0x35,			/* 8 second */
	0x35,			/* 8 second */
};

static int wm8350_wdt_ioctl(struct inode *inode, struct file *file,
			    unsigned int cmd, unsigned long arg)
{
	struct wm8350_wdg *wm_wdt =
	    to_wm8350_wdg_device(wm8350_wdt_miscdev.this_device);
	struct wm8350 *wm8350 = to_wm8350_from_wdg(wm_wdt);
	int ret = -ENOTTY, time, i;
	void __user *argp = (void __user *)arg;
	int __user *p = argp;
	u16 reg;

	switch (cmd) {
	case WDIOC_GETSUPPORT:
		ret = copy_to_user(argp, &ident, sizeof(ident)) ? -EFAULT : 0;
		break;
	case WDIOC_GETSTATUS:
		ret = put_user(0, p);
		break;
	case WDIOC_SETTIMEOUT:
		ret = get_user(time, p);
		if (ret)
			break;

		if (time < 0 || time >= ARRAY_SIZE(wm8350_wdt_configs)) {
			ret = -EINVAL;
			break;
		}

		ret = wm8350_wdt_configure(wm8350, wm8350_wdt_configs[time]);
		break;
	case WDIOC_GETTIMEOUT:
		reg = wm8350_reg_read(wm8350, WM8350_SYSTEM_CONTROL_2);
		for (i = 0; i < ARRAY_SIZE(wm8350_wdt_configs); i++) {
			if (wm8350_wdt_configs[i] > reg)
				break;
		}
		ret = put_user(i - 1, p);
		break;

	case WDIOC_KEEPALIVE:
		/* Write to Watchdog Config Register */
		ret = wm8350_wdt_kick(wm8350);
		break;
	}
	return ret;
}

static const struct file_operations wm8350_wdt_fops = {
	.owner = THIS_MODULE,
	.llseek = no_llseek,
	.write = wm8350_wdt_write,
	.ioctl = wm8350_wdt_ioctl,
	.open = wm8350_wdt_open,
	.release = wm8350_wdt_release,
};

static struct miscdevice wm8350_wdt_miscdev = {
	.minor = WATCHDOG_MINOR,
	.name = "watchdog",
	.fops = &wm8350_wdt_fops,
};

static int wm8350_wdt_probe(struct platform_device *pdev)
{
	struct wm8350 *wm8350 = platform_get_drvdata(pdev);
	int ret;

	printk(KERN_INFO "wm8350: watchdog driver %s\n", WM8350_WDT_VERSION);

	/* Turn off Watchdog here just to make sure that it isnt
	 * turned on until we can start kicking it.
	 */
	wm8350_reg_unlock(wm8350);
	ret = wm8350_clear_bits(wm8350, WM8350_SYSTEM_CONTROL_2,
				WM8350_WDOG_HIB_MODE);
	wm8350_reg_lock(wm8350);
	if (ret < 0) {
		printk(KERN_ERR "%s: failed to reset watchdog\n", __func__);
		return ret;
	}

	wm8350_wdt_miscdev.this_device = &pdev->dev;
	return misc_register(&wm8350_wdt_miscdev);
}

static int __exit wm8350_wdt_remove(struct platform_device *pdev)
{
	misc_deregister(&wm8350_wdt_miscdev);
	return 0;
}

static struct platform_driver wm8350_wdt_driver = {
	.probe = wm8350_wdt_probe,
	.remove = wm8350_wdt_remove,
	.driver = {
		.name = "wm8350-wdt",
	},
};

static int __init wm8350_wdt_init(void)
{
	return platform_driver_register(&wm8350_wdt_driver);
}

static void __exit wm8350_wdt_exit(void)
{
	platform_driver_unregister(&wm8350_wdt_driver);
}

module_init(wm8350_wdt_init);
module_exit(wm8350_wdt_exit);

MODULE_AUTHOR("Graeme Gregory");
MODULE_DESCRIPTION("WM8350 Watchdog");
MODULE_LICENSE("GPL");
