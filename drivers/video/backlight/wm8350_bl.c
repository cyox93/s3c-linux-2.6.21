/*
 *  WM8350 Backlight Driver
 *
 *  Copyright(C) 2007 Wolfson Microelectronics PLC.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <linux/fb.h>
#include <linux/platform_device.h>
#include <linux/backlight.h>
#include <linux/regulator/consumer.h>
#include <linux/mfd/wm8350/core.h>

#define WM8350_BL_VERSION "0.4"

struct wm8350_backlight {
	struct backlight_properties props;
	struct backlight_device *device;
	struct regulator *dcdc;
	struct regulator *isink;
	struct notifier_block notifier;
	struct work_struct work;
	struct mutex mutex; /* backlight mutex */
	int intensity;
	int suspend;
	int retries;
};

/* hundredths of uA, 405 = 4.05 uA */
static const int intensity_huA[] = {
	405, 482, 573, 681, 810, 963, 1146, 1362, 1620, 1927, 2291, 2725,
	3240, 3853, 4582, 5449, 6480, 7706, 9164, 10898, 12960, 15412, 18328,
	21796, 25920, 30824, 36656, 43592, 51840, 61648, 73313, 87184,
	103680, 123297, 146626, 174368, 207360, 246594, 293251, 348737,
	414720, 493188, 586503, 697473, 829440, 986376, 1173005, 1394946,
	1658880, 1972752, 2346011, 2789892, 3317760, 3945504, 4692021,
	5579785, 6635520, 7891008, 9384042, 11159570, 13271040, 15782015,
	18768085, 22319140,
};

static void bl_work(struct work_struct *work)
{
	struct wm8350_backlight *bl =
	    container_of(work, struct wm8350_backlight, work);
	struct regulator *isink = bl->isink;

	mutex_lock(&bl->mutex);
	if (bl->intensity >= 0 && bl->intensity < ARRAY_SIZE(intensity_huA)) {
		bl->retries = 0;
		regulator_set_current_limit(isink,
				      intensity_huA[bl->intensity] / 100,
				      intensity_huA[bl->intensity] / 100);
	} else
		printk(KERN_ERR "wm8350: Backlight intensity error\n");
	mutex_unlock(&bl->mutex);
}

static int wm8350_bl_notifier(struct notifier_block *self,
			      unsigned long event, void *data)
{
	struct wm8350_backlight *bl =
	    container_of(self, struct wm8350_backlight, notifier);
	struct regulator *isink = bl->isink;

	if (event & REGULATOR_EVENT_UNDER_VOLTAGE)
		printk(KERN_ERR "wm8350: BL DCDC undervoltage\n");
	if (event & REGULATOR_EVENT_REGULATION_OUT)
		printk(KERN_ERR "wm8350: BL ISINK out of regulation\n");

	mutex_lock(&bl->mutex);
	if (bl->retries) {
		bl->retries--;
		regulator_disable(isink);
		regulator_set_current_limit(isink, bl->intensity,
			bl->intensity);
		regulator_enable(isink);
	} else {
		printk(KERN_ERR
		       "wm8350: BL regulation retry failure - disabled\n");
		bl->intensity = 0;
		regulator_disable(isink);
	}
	mutex_unlock(&bl->mutex);
	return NOTIFY_DONE;
}

static int wm8350_bl_send_intensity(struct backlight_device *bd)
{
	struct wm8350_backlight *bl =
	    (struct wm8350_backlight *)class_get_devdata(&bd->class_dev);
	int intensity = bd->props.brightness;

	if (bd->props.power != FB_BLANK_UNBLANK)
		intensity = 0;
	if (bd->props.fb_blank != FB_BLANK_UNBLANK)
		intensity = 0;
	if (bl->suspend)
		intensity = 0;

	mutex_lock(&bl->mutex);
	bl->intensity = intensity;
	mutex_unlock(&bl->mutex);
	schedule_work(&bl->work);

	return 0;
}

#ifdef CONFIG_PM
static int wm8350_bl_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct wm8350_backlight *bl =
	    (struct wm8350_backlight *)platform_get_drvdata(pdev);

	bl->suspend = 1;
	backlight_update_status(bl->device);
	return 0;
}

static int wm8350_bl_resume(struct platform_device *pdev)
{
	struct wm8350_backlight *bl =
	    (struct wm8350_backlight *)platform_get_drvdata(pdev);

	bl->suspend = 0;
	backlight_update_status(bl->device);
	return 0;
}
#else
#define wm8350_bl_suspend	NULL
#define wm8350_bl_resume	NULL
#endif

static int wm8350_bl_get_intensity(struct backlight_device *bd)
{
	struct wm8350_backlight *bl =
	    (struct wm8350_backlight *)class_get_devdata(&bd->class_dev);
	return bl->intensity;
}

static struct backlight_ops wm8350_bl_ops = {
	.get_brightness = wm8350_bl_get_intensity,
	.update_status = wm8350_bl_send_intensity,
};

static int wm8350_bl_probe(struct platform_device *pdev)
{
	struct regulator *isink, *dcdc;
	struct wm8350_backlight *bl;
	struct wm8350_bl_platform_data *pdata = pdev->dev.platform_data;
	struct wm8350_pmic *pmic;
	int ret;

	printk(KERN_INFO "wm8350: backlight driver %s\n", WM8350_BL_VERSION);

	if (pdata == NULL) {
		printk(KERN_ERR "%s: no platform data\n", __func__);
		return -ENODEV;
	}

	isink = regulator_get(&pdev->dev, "bl_isink");
	if (IS_ERR(isink) || isink == NULL) {
		printk(KERN_ERR "%s: cant get ISINK\n", __func__);
		return PTR_ERR(isink);
	}

	dcdc = regulator_get(&pdev->dev, "bl_vcc");
	if (IS_ERR(dcdc) || dcdc == NULL) {
		printk(KERN_ERR "%s: cant get DCDC\n", __func__);
		regulator_put(isink);
		return PTR_ERR(dcdc);
	}

	bl = kzalloc(sizeof(*bl), GFP_KERNEL);
	if (bl == NULL) {
		regulator_put(isink);
		regulator_put(dcdc);
		return -ENOMEM;
	}

	mutex_init(&bl->mutex);
	INIT_WORK(&bl->work, bl_work);
	bl->props.max_brightness = pdata->max_brightness;
	bl->props.power = pdata->power;
	bl->props.brightness = pdata->brightness;
	bl->retries = pdata->retries;
	bl->dcdc = dcdc;
	bl->isink = isink;
	platform_set_drvdata(pdev, bl);
	pmic = regulator_get_drvdata(bl->isink);

	bl->device = backlight_device_register("wm8350-bl-dev", &pdev->dev,
					       bl, &wm8350_bl_ops);
	if (IS_ERR(bl->device)) {
		ret = PTR_ERR(bl->device);
		regulator_put(dcdc);
		regulator_put(isink);
		kfree(bl);
		return ret;
	}

	bl->notifier.notifier_call = wm8350_bl_notifier;
	regulator_register_notifier(dcdc, &bl->notifier);
	regulator_register_notifier(isink, &bl->notifier);
	bl->device->props = bl->props;

	regulator_set_current_limit(isink, 0, 0);
	regulator_enable(isink);
	backlight_update_status(bl->device);
	return 0;
}

static int wm8350_bl_remove(struct platform_device *pdev)
{
	struct wm8350_backlight *bl =
	    (struct wm8350_backlight *)platform_get_drvdata(pdev);
	struct regulator *isink = bl->isink, *dcdc = bl->dcdc;

	bl->intensity = 0;
	backlight_update_status(bl->device);
	schedule_work(&bl->work);
	flush_scheduled_work();
	backlight_device_unregister(bl->device);

	regulator_set_current_limit(isink, 0, 0);
	regulator_disable(isink);
	regulator_unregister_notifier(isink, &bl->notifier);
	regulator_unregister_notifier(dcdc, &bl->notifier);
	regulator_put(isink);
	regulator_put(dcdc);
	return 0;
}

static void wm8350_bl_shutdown(struct platform_device *pdev)
{
	struct wm8350_backlight *bl =
	    (struct wm8350_backlight *)platform_get_drvdata(pdev);
	struct regulator *isink = bl->isink;

	regulator_set_current_limit(isink, 0, 0);
	regulator_disable(isink);
}

struct platform_driver imx32ads_backlight_driver = {
	.driver = {
		   .name = "wm8350-bl",
		   .owner = THIS_MODULE,
		   },
	.probe = wm8350_bl_probe,
	.remove = wm8350_bl_remove,
	.shutdown = wm8350_bl_shutdown,
	.suspend = wm8350_bl_suspend,
	.resume = wm8350_bl_resume,
};

static int __devinit imx32ads_backlight_init(void)
{
	return platform_driver_register(&imx32ads_backlight_driver);
}

static void imx32ads_backlight_exit(void)
{
	platform_driver_unregister(&imx32ads_backlight_driver);
}

module_init(imx32ads_backlight_init);
module_exit(imx32ads_backlight_exit);

MODULE_AUTHOR("Liam Girdwood <lg@opensource.wolfsonmicro.com>");
MODULE_DESCRIPTION("WM8350 Backlight driver");
MODULE_LICENSE("GPL");
