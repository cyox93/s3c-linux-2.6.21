/*
 * wm8350-i2c.c  --  Generic I2C driver for Wolfson WM8350 PMIC
 *
 * This driver defines and configures the WM8350 for the Freescale i.MX32ADS.
 *
 * Copyright 2007 Wolfson Microelectronics PLC.
 *
 * Author: Liam Girdwood
 *         liam.girdwood@wolfsonmicro.com or linux@wolfsonmicro.com
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  Revision history
 *    23rd Jan 2007   Initial version.
 *
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/bitops.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/regulator/machine.h>
#include <linux/mfd/wm8350/pmic.h>
#include <linux/mfd/wm8350/gpio.h>
#include <linux/mfd/wm8350/core.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/leds.h>
#include <linux/fb.h>

#define WM8350_I2C_VERSION "0.7"

static void wm8350_irq_work(struct work_struct *work)
{
	wm8350_irq_worker(work);
}

static irqreturn_t wm8350_irq_handler(int irq, void *data)
{
	struct wm8350 *wm8350 = (struct wm8350 *)data;

	schedule_work(&wm8350->work);
	return IRQ_HANDLED;
}

#define I2C_DRIVERID_WM8350 0xfefe	/* TODO: liam need proper id */

static int wm8350_i2c_probe(struct i2c_client *i2c)
{
	struct wm8350 *wm8350;
	struct wm8350_platform_data *pdata = i2c->dev.platform_data;
	int ret = 0;

	printk(KERN_INFO "wm8350: I2C client driver version %s\n",
	       WM8350_I2C_VERSION);

	wm8350 = kzalloc(sizeof(struct wm8350), GFP_KERNEL);
	if (wm8350 == NULL) {
		kfree(i2c);
		return -ENOMEM;
	}

	mutex_init(&wm8350->work_mutex);
	i2c_set_clientdata(i2c, wm8350);
	wm8350->i2c_client = i2c;
	wm8350_set_io(wm8350, WM8350_IO_I2C, NULL, NULL);

	ret = wm8350_device_init(wm8350);
	if (ret < 0) {
		printk(KERN_ERR "%s: failed to initialise device\n", __func__);
		goto err;
	}

	if (pdata && pdata->board_pmic_init) {
		ret = pdata->board_pmic_init(wm8350);
		if (ret < 0) {
			printk(KERN_ERR "%s: wm8350 failed board init\n",
				__func__);
			goto err;
		}
	}

	INIT_WORK(&wm8350->work, wm8350_irq_work);
	ret = request_irq(i2c->irq, wm8350_irq_handler,
			  IRQF_DISABLED, "wm8350", wm8350);

	/* unmask all & clear sticky */
	wm8350_reg_write(wm8350, WM8350_SYSTEM_INTERRUPTS_MASK, 0x0);
	schedule_work(&wm8350->work);

	return ret;
err:
	wm8350_device_exit(wm8350);
	kfree(wm8350);
	return ret;
}

static int wm8350_i2c_remove(struct i2c_client *i2c)
{
	struct wm8350 *wm8350 = i2c_get_clientdata(i2c);

	if (i2c->irq)
		free_irq(i2c->irq, wm8350);
	flush_scheduled_work();

	wm8350_device_exit(wm8350);
	kfree(wm8350);
	return 0;
}

static struct i2c_driver wm8350_i2c_driver = {
	.driver = {
		   .name = "wm8350-i2c",
		   .owner = THIS_MODULE,
		   },
	.id = I2C_DRIVERID_WM8350,
	.probe = wm8350_i2c_probe,
	.remove = wm8350_i2c_remove,
};

static int __init imx32ads_wm8350_pmic_init(void)
{
	return i2c_add_driver(&wm8350_i2c_driver);
}

/* init early so consumer devices can complete system boot */
subsys_initcall(imx32ads_wm8350_pmic_init);
