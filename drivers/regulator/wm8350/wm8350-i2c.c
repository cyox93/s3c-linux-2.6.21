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
#include <linux/fb.h>

#define WM8350_I2C_VERSION "0.7"

static unsigned short normal_i2c[] = { 0x1a, I2C_CLIENT_END };

I2C_CLIENT_INSMOD;

static struct i2c_driver wm8350_i2c_driver;
static struct i2c_client client_template;

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

static int wm8350_i2c_probe(struct i2c_adapter *adapter, int addr, int kind)
{
	struct i2c_client *i2c;
	struct wm8350 *wm8350;
	int ret = 0;

	printk(KERN_INFO "wm8350: I2C client driver version %s\n",
	       WM8350_I2C_VERSION);

	client_template.addr = addr;
	client_template.adapter = adapter;

	wm8350 = kzalloc(sizeof(struct wm8350), GFP_KERNEL);
	if (wm8350 == NULL) {
		return -ENOMEM;
	}

	i2c = kmemdup(&client_template, sizeof(client_template), GFP_KERNEL);
	if (i2c == NULL) {
		kfree(wm8350);
		return -ENOMEM;
	}

	mutex_init(&wm8350->work_mutex);
	i2c_set_clientdata(i2c, wm8350);
	wm8350->i2c_client = i2c;
	wm8350_set_io(wm8350, WM8350_IO_I2C, NULL, NULL);

	ret= i2c_attach_client(i2c);
	if (ret < 0) {
		printk(KERN_ERR "%s: failed to attach i2c at %x\n", __func__, addr);
		goto err;
	}
		
	ret = wm8350_device_init(wm8350);
	if (ret < 0) {
		printk(KERN_ERR "%s: failed to initialise device\n", __func__);
		goto err;
	}

	ret = wm8350_dev_init(wm8350);
	if (ret < 0) {
		printk(KERN_ERR "%s: faile to initialise wm8350\n", __func__);
		goto err;
	}

	/* Initialize the WM8350 event handling */
	wm8350_event_list_init();

	INIT_WORK(&wm8350->work, wm8350_irq_work);
	ret = request_irq(IRQ_EINT12, wm8350_irq_handler,
			IRQF_DISABLED, "wm8350", wm8350);

	/* unmask all & clear sticky */
	wm8350_reg_write(wm8350, WM8350_SYSTEM_INTERRUPTS_MASK, 0x0);
	schedule_work(&wm8350->work);

	return ret;

err:
	wm8350_device_exit(wm8350);
	kfree(wm8350);
	kfree(i2c);
	return ret;
}

static int wm8350_i2c_detach(struct i2c_client *client)
{
	int err;
	struct wm8350 *wm8350 = i2c_get_clientdata(client);

	err = i2c_detach_client(client);
	if (err) {
		return err;
	}

	free_irq(IRQ_EINT12, wm8350);
	wm8350_device_exit(wm8350);
	kfree(client);
	kfree(wm8350);

	return 0;
}

static int wm8350_i2c_attach(struct i2c_adapter *adapter)
{
	return i2c_probe(adapter, &addr_data, wm8350_i2c_probe);
}

static struct i2c_driver wm8350_i2c_driver = {
	.driver = {
		   .name = "wm8350-i2c",
		   .owner = THIS_MODULE,
		   },
	.id =				I2C_DRIVERID_WM8350,
	.attach_adapter =	wm8350_i2c_attach,
	.detach_client =	wm8350_i2c_detach,
	.command =			NULL,
};

static struct i2c_client client_template = {
	.name = "wm8350-i2c",
	.driver = &wm8350_i2c_driver,
};

static int __init wm8350_i2c_init(void)
{
	return i2c_add_driver(&wm8350_i2c_driver);
}

static void __exit wm8350_i2c_exit(void)
{
	i2c_del_driver(&wm8350_i2c_driver);
}

module_init(wm8350_i2c_init);
module_exit(wm8350_i2c_exit);

