/*
 * LED driver for WM8350 driven LEDS.
 *
 * Copyright(C) 2007 Wolfson Microelectronics PLC.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/err.h>
#include <linux/mfd/wm8350/pmic.h>
#include <linux/regulator/consumer.h>

#define WM8350_LED_VERSION "0.4"

struct wm8350_led {
	struct work_struct work;
	struct mutex mutex; /* led mutex */
	struct regulator *isink;
	struct regulator *dcdc;
	struct notifier_block notifier;
	enum led_brightness value;
	struct led_classdev cdev;
	int retries;
	int half_value;
	int full_value;
	int current_value;
};
#define to_wm8350_led(led_cdev) \
	container_of(led_cdev, struct wm8350_led, cdev)

static void led_work(struct work_struct *work)
{
	struct wm8350_led *led = container_of(work, struct wm8350_led, work);

	mutex_lock(&led->mutex);
	switch (led->value) {
	case LED_OFF:
		led->current_value = 0;
		break;
	case LED_HALF:
		led->retries = 0;
		led->current_value = led->half_value;
		break;
	case LED_FULL:
		led->retries = 0;
		led->current_value = led->full_value;
		break;
	}

	regulator_set_current_limit(led->isink, led->current_value,
		led->current_value);
	mutex_unlock(&led->mutex);
}

static void wm8350_led_set(struct led_classdev *led_cdev,
			   enum led_brightness value)
{
	struct wm8350_led *led = to_wm8350_led(led_cdev);

	mutex_lock(&led->mutex);
	led->value = value;
	mutex_unlock(&led->mutex);
	schedule_work(&led->work);
}

static int wm8350_led_notifier(struct notifier_block *self,
			       unsigned long event, void *data)
{
	struct wm8350_led *led =
	    container_of(self, struct wm8350_led, notifier);

	if (event & REGULATOR_EVENT_UNDER_VOLTAGE)
		printk(KERN_ERR "wm8350: LED DCDC undervoltage\n");
	if (event & REGULATOR_EVENT_REGULATION_OUT)
		printk(KERN_ERR "wm8350: LED ISINK out of regulation\n");

	mutex_lock(&led->mutex);
	if (led->retries) {
		led->retries--;
		regulator_disable(led->isink);
		regulator_set_current_limit(led->isink, led->value, led->value);
		regulator_enable(led->isink);
	} else {
		printk(KERN_ERR
		       "wm8350: LED regulation retry failure - disabled\n");
		led->current_value = 0;
		regulator_disable(led->isink);
	}
	mutex_unlock(&led->mutex);
	return NOTIFY_DONE;
}

#ifdef CONFIG_PM
static int wm8350_led_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct wm8350_led *led =
	    (struct wm8350_led *)platform_get_drvdata(pdev);

	led_classdev_suspend(&led->cdev);
	return 0;
}

static int wm8350_led_resume(struct platform_device *pdev)
{
	struct wm8350_led *led =
	    (struct wm8350_led *)platform_get_drvdata(pdev);

	led_classdev_resume(&led->cdev);
	return 0;
}
#endif

static void wm8350_led_shutdown(struct platform_device *pdev)
{
	struct wm8350_led *led =
	    (struct wm8350_led *)platform_get_drvdata(pdev);

	mutex_lock(&led->mutex);
	led->value = LED_OFF;
	regulator_disable(led->isink);
	mutex_unlock(&led->mutex);
}

static int wm8350_led_probe(struct platform_device *pdev)
{
	struct regulator *isink, *dcdc;
	struct wm8350_led *led;
	struct wm8350_led_platform_data *pdata = pdev->dev.platform_data;
	struct wm8350_pmic *pmic;
	int ret;

	printk(KERN_INFO "wm8350: LED driver %s\n", WM8350_LED_VERSION);

	if (pdata == NULL) {
		printk(KERN_ERR "%s: no platform data\n", __func__);
		return -ENODEV;
	}

	isink = regulator_get(&pdev->dev, "led_isink");
	if (IS_ERR(isink) || isink == NULL) {
		printk(KERN_ERR "%s: cant get ISINK\n", __func__);
		return PTR_ERR(isink);
	}

	dcdc = regulator_get(&pdev->dev, "led_vcc");
	if (IS_ERR(dcdc) || dcdc == NULL) {
		printk(KERN_ERR "%s: cant get DCDC\n", __func__);
		regulator_put(isink);
		return PTR_ERR(dcdc);
	}

	led = kzalloc(sizeof(*led), GFP_KERNEL);
	if (led == NULL) {
		regulator_put(isink);
		regulator_put(dcdc);
		return -ENOMEM;
	}

	led->retries = pdata->retries;
	led->half_value = pdata->half_value;
	led->full_value = pdata->full_value;
	led->cdev.brightness_set = wm8350_led_set;
	led->cdev.default_trigger = (char *)pdata->default_trigger;
	led->cdev.name = pdata->name;
	led->isink = isink;
	led->dcdc = dcdc;
	pmic = regulator_get_drvdata(led->isink);

	mutex_init(&led->mutex);
	INIT_WORK(&led->work, led_work);
	led->value = LED_OFF;
	platform_set_drvdata(pdev, led);

	ret = led_classdev_register(&pdev->dev, &led->cdev);
	if (ret < 0) {
		regulator_put(isink);
		regulator_put(dcdc);
		kfree(led);
		return ret;
	}

	led->notifier.notifier_call = wm8350_led_notifier;
	regulator_register_notifier(isink, &led->notifier);
	regulator_register_notifier(dcdc, &led->notifier);

	regulator_set_current_limit(isink, 0, 0);
	regulator_enable(led->isink);
	return 0;
}

static int wm8350_led_remove(struct platform_device *pdev)
{
	struct wm8350_led *led =
	    (struct wm8350_led *)platform_get_drvdata(pdev);

	led_classdev_unregister(&led->cdev);
	schedule_work(&led->work);
	flush_scheduled_work();
	regulator_set_current_limit(led->isink, 0, 0);
	regulator_disable(led->isink);
	regulator_unregister_notifier(led->dcdc, &led->notifier);
	regulator_unregister_notifier(led->isink, &led->notifier);
	regulator_put(led->dcdc);
	regulator_put(led->isink);
	kfree(led);
	return 0;
}

struct platform_driver wm8350_led_driver = {
	.driver = {
		   .name = "wm8350-led",
		   .owner = THIS_MODULE,
		   },
	.probe = wm8350_led_probe,
	.remove = wm8350_led_remove,
	.shutdown = wm8350_led_shutdown,
	.suspend = wm8350_led_suspend,
	.resume = wm8350_led_resume,
};

static int __devinit wm8350_led_init(void)
{
	return platform_driver_register(&wm8350_led_driver);
}

static void wm8350_led_exit(void)
{
	platform_driver_unregister(&wm8350_led_driver);
}

module_init(wm8350_led_init);
module_exit(wm8350_led_exit);

MODULE_AUTHOR("Liam Girdwood <lg@opensource.wolfsonmicro.com>");
MODULE_DESCRIPTION("WM8350 LED driver");
MODULE_LICENSE("GPL");
