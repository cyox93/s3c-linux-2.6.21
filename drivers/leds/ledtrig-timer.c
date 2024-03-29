/*
 * LED Kernel Timer Trigger
 *
 * Copyright 2005-2006 Openedhand Ltd.
 *
 * Author: Richard Purdie <rpurdie@openedhand.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/device.h>
#include <linux/sysdev.h>
#include <linux/timer.h>
#include <linux/ctype.h>
#include <linux/leds.h>
#include "leds.h"

struct timer_trig_data {
	unsigned long delay_on;		/* milliseconds on */
	unsigned long delay_off;	/* milliseconds off */
#ifdef CONFIG_MACH_CANOPUS
	int brightness;			/* original brightness */
#endif	// CONFIG_MACH_CANOPUS
	struct timer_list timer;
	
	struct led_classdev *led_cdev;
	struct work_struct active_work;
	struct work_struct deactive_work;
};

static void led_timer_function(unsigned long data)
{
	struct led_classdev *led_cdev = (struct led_classdev *) data;
	struct timer_trig_data *timer_data = led_cdev->trigger_data;
	unsigned long brightness = LED_OFF;
	unsigned long delay = timer_data->delay_off;

	if (!timer_data->delay_on || !timer_data->delay_off) {
		led_set_brightness(led_cdev, LED_OFF);
		return;
	}

	if (!led_cdev->brightness) {
#ifndef CONFIG_MACH_CANOPUS
		brightness = LED_FULL;
#else	// CONFIG_MACH_CANOPUS
		if (!timer_data->brightness)
			brightness = LED_FULL;
		else
			brightness = timer_data->brightness;
#endif	// CONFIG_MACH_CANOPUS

		delay = timer_data->delay_on;
	}

	led_set_brightness(led_cdev, brightness);

	mod_timer(&timer_data->timer, jiffies + msecs_to_jiffies(delay));
}

static ssize_t led_delay_on_show(struct class_device *dev, char *buf)
{
	struct led_classdev *led_cdev = class_get_devdata(dev);
	struct timer_trig_data *timer_data = led_cdev->trigger_data;

	sprintf(buf, "%lu\n", timer_data->delay_on);

	return strlen(buf) + 1;
}

static ssize_t led_delay_on_store(struct class_device *dev, const char *buf,
				size_t size)
{
	struct led_classdev *led_cdev = class_get_devdata(dev);
	struct timer_trig_data *timer_data = led_cdev->trigger_data;
	int ret = -EINVAL;
	char *after;
	unsigned long state = simple_strtoul(buf, &after, 10);
	size_t count = after - buf;

	if (*after && isspace(*after))
		count++;

	if (count == size) {
		timer_data->delay_on = state;
		mod_timer(&timer_data->timer, jiffies + 1);
		ret = count;
	}

	return ret;
}

static ssize_t led_delay_off_show(struct class_device *dev, char *buf)
{
	struct led_classdev *led_cdev = class_get_devdata(dev);
	struct timer_trig_data *timer_data = led_cdev->trigger_data;

	sprintf(buf, "%lu\n", timer_data->delay_off);

	return strlen(buf) + 1;
}

static ssize_t led_delay_off_store(struct class_device *dev, const char *buf,
				size_t size)
{
	struct led_classdev *led_cdev = class_get_devdata(dev);
	struct timer_trig_data *timer_data = led_cdev->trigger_data;
	int ret = -EINVAL;
	char *after;
	unsigned long state = simple_strtoul(buf, &after, 10);
	size_t count = after - buf;

	if (*after && isspace(*after))
		count++;

	if (count == size) {
		timer_data->delay_off = state;
		mod_timer(&timer_data->timer, jiffies + 1);
		ret = count;
	}

	return ret;
}

static CLASS_DEVICE_ATTR(delay_on, 0644, led_delay_on_show,
			led_delay_on_store);
static CLASS_DEVICE_ATTR(delay_off, 0644, led_delay_off_show,
			led_delay_off_store);

static void active_work_handle(struct work_struct *work)
{
	int rc;
	struct timer_trig_data *timer_data = container_of(work, struct timer_trig_data, active_work);
	struct led_classdev *led_cdev = timer_data->led_cdev;
	
	rc = class_device_create_file(led_cdev->class_dev,
				      &class_device_attr_delay_on);
	if (rc) goto err_out;
	rc = class_device_create_file(led_cdev->class_dev,
				      &class_device_attr_delay_off);
	if (rc) goto err_out_delayon;

	return;

err_out_delayon:
	class_device_remove_file(led_cdev->class_dev,
				&class_device_attr_delay_on);
err_out:
	led_cdev->trigger_data = NULL;
	kfree(timer_data);
	BUG();
}

static void deactive_work_handle(struct work_struct *work)
{
	struct timer_trig_data *timer_data = container_of(work, struct timer_trig_data, deactive_work);
	struct led_classdev *led_cdev = timer_data->led_cdev;

	class_device_remove_file(led_cdev->class_dev,
				 &class_device_attr_delay_on);
	class_device_remove_file(led_cdev->class_dev,
				 &class_device_attr_delay_off);
	kfree(timer_data);
}

static void timer_trig_activate(struct led_classdev *led_cdev)
{
	struct timer_trig_data *timer_data;

	timer_data = kzalloc(sizeof(struct timer_trig_data), GFP_ATOMIC);
	if (!timer_data)
		return;

	led_cdev->trigger_data = timer_data;

#ifdef CONFIG_MACH_CANOPUS
	timer_data->brightness = led_cdev->brightness;
#endif	// CONFIG_MACH_CANOPUS
	init_timer(&timer_data->timer);
	timer_data->timer.function = led_timer_function;
	timer_data->timer.data = (unsigned long) led_cdev;
	timer_data->led_cdev = led_cdev;
	INIT_WORK(&timer_data->active_work, active_work_handle);
	INIT_WORK(&timer_data->deactive_work, deactive_work_handle);
	schedule_work(&timer_data->active_work);
}

static void timer_trig_deactivate(struct led_classdev *led_cdev)
{
	struct timer_trig_data *timer_data = led_cdev->trigger_data;

	if (timer_data) {
		del_timer_sync(&timer_data->timer);
		schedule_work(&timer_data->deactive_work);
	}
}

static struct led_trigger timer_led_trigger = {
	.name     = "timer",
	.activate = timer_trig_activate,
	.deactivate = timer_trig_deactivate,
};

static int __init timer_trig_init(void)
{
	return led_trigger_register(&timer_led_trigger);
}

static void __exit timer_trig_exit(void)
{
	led_trigger_unregister(&timer_led_trigger);
}

module_init(timer_trig_init);
module_exit(timer_trig_exit);

MODULE_AUTHOR("Richard Purdie <rpurdie@openedhand.com>");
MODULE_DESCRIPTION("Timer LED trigger");
MODULE_LICENSE("GPL");
