/*
 * Battery driver for wm8350 PMIC
 *
 * Copyright © 2007 Wolfson Microelectronics PLC.
 *
 * Based on OLCP Battery Driver
 *
 * Copyright © 2006  David Woodhouse <dwmw2@infradead.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/mfd/wm8350/supply.h>
#include <linux/mfd/wm8350/core.h>
#include <linux/mfd/wm8350/comparator.h>

#define WM8350_POWER_VERSION	"0.5"

#define WM8350_BATT_SUPPLY	1
#define WM8350_USB_SUPPLY	2
#define WM8350_LINE_SUPPLY	4

static inline int wm8350_charge_time_min(struct wm8350 *wm8350, int min)
{
	if (wm8350->rev == WM8351_REV_M)
		return (((min - 60) / 30) & 0xf) << 8;
	else if (wm8350->rev < WM8350_REV_G) 
		return (((min - 30) / 15) & 0xf) << 8;
	else 
		return (((min - 30) / 30) & 0xf) << 8;
}

static int wm8350_charger_config(struct wm8350 *wm8350,
				 struct wm8350_charger_policy *policy)
{
	u16 reg, eoc_mA, fast_limit_mA;

	/* make sure USB fast charge current is not > 500mA */
	if (policy->fast_limit_USB_mA > 500) {
		printk(KERN_ERR "%s: USB fast charge > 500mA\n", __func__);
		return -EINVAL;
	}

	eoc_mA = WM8351_CHG_EOC_mA(policy->eoc_mA);

	wm8350_reg_unlock(wm8350);
	reg = wm8350_reg_read(wm8350, WM8350_BATTERY_CHARGER_CONTROL_1)
	    & WM8350_CHG_ENA_R168;
	wm8350_reg_write(wm8350, WM8350_BATTERY_CHARGER_CONTROL_1,
			reg | eoc_mA | policy->trickle_start_mV |
			WM8350_CHG_TRICKLE_TEMP_CHOKE |
			WM8350_CHG_TRICKLE_USB_CHOKE |
			WM8350_CHG_FAST_USB_THROTTLE);

	if (wm8350->power.is_usb_supply) {
		fast_limit_mA =
			WM8350_CHG_FAST_LIMIT_mA(policy->fast_limit_USB_mA);
		wm8350_reg_write(wm8350, WM8350_BATTERY_CHARGER_CONTROL_2,
			policy->charge_mV | policy->trickle_charge_USB_mA |
			fast_limit_mA | wm8350_charge_time_min(wm8350,
				policy->charge_timeout));
	} else {
		fast_limit_mA =
			WM8350_CHG_FAST_LIMIT_mA(policy->fast_limit_mA);
		wm8350_reg_write(wm8350, WM8350_BATTERY_CHARGER_CONTROL_2,
			policy->charge_mV | policy->trickle_charge_mA |
			fast_limit_mA | wm8350_charge_time_min(wm8350,
				policy->charge_timeout));
	}
	wm8350_reg_lock(wm8350);
	return 0;
}

static int wm8350_charger_enable(struct wm8350 *wm8350, int enable)
{
	if (enable) {
		/* enable */
		wm8350_reg_unlock(wm8350);
		wm8350_set_bits(wm8350, WM8350_POWER_MGMT_5, WM8350_CHG_ENA);
		wm8350_reg_lock(wm8350);
	} else {
		/* disable */
		wm8350_reg_unlock(wm8350);
		wm8350_clear_bits(wm8350, WM8350_POWER_MGMT_5, WM8350_CHG_ENA);
		wm8350_reg_lock(wm8350);
	}
	return 0;
}

static int wm8350_fast_charger_mode(struct wm8350 *wm8350)
{
	u16 status;

	status = wm8350_reg_read(wm8350, WM8350_INT_STATUS_1)
	    & ~wm8350_reg_read(wm8350, WM8350_INT_STATUS_1_MASK);

	if (status & WM8350_CHG_FAST_RDY_EINT) {
		printk(KERN_INFO "wm8350-power: fast charger ready\n");
		wm8350_reg_unlock(wm8350);
		wm8350_set_bits(wm8350, WM8350_BATTERY_CHARGER_CONTROL_1,
			WM8350_CHG_FAST);
		wm8350_reg_lock(wm8350);
	}

	return 0;	
}

static int wm8350_get_supplies(struct wm8350 *wm8350)
{
	u16 sm, ov, co, chrg;
	int supplies = 0;

	sm = wm8350_reg_read(wm8350, 233);
	ov = wm8350_reg_read(wm8350, 227);
	co = wm8350_reg_read(wm8350, 231);
	chrg = wm8350_reg_read(wm8350, WM8350_BATTERY_CHARGER_CONTROL_2);

	/* USB_SM */
	sm >>= 8;
	sm &= 0x7;

	/* USB_LIMIT_OVRDE */
	ov >>= 10;
	ov &= 0x01;

	/* WALL_FB_OVRDE */
	co >>= 14;
	co &= 0x01;

	/* CHG_ISEL */
	chrg &= WM8350_CHG_ISEL_MASK;

	if (((sm == 0x01) || (sm == 0x05) || (sm == 0x07)) && !ov)
		supplies = WM8350_USB_SUPPLY;
	else if (((sm == 0x01) || (sm == 0x05) || (sm == 0x07)) && ov &&
		 (chrg == 0x00))
		supplies = WM8350_USB_SUPPLY | WM8350_BATT_SUPPLY;
	else if (co)
		supplies = WM8350_LINE_SUPPLY;
	else
		supplies = WM8350_BATT_SUPPLY;

	return supplies;
}

static int wm8350_batt_status(struct wm8350 *wm8350)
{
	u16 chrg;

	chrg = wm8350_reg_read(wm8350, WM8350_BATTERY_CHARGER_CONTROL_2);
	chrg &= WM8350_CHG_STS_MASK;
	chrg >>= 12;

	switch (chrg) {
	case 0x00:
		return POWER_SUPPLY_STATUS_DISCHARGING;
	case 0x01:
	case 0x02:
		return POWER_SUPPLY_STATUS_CHARGING;
	}
	return POWER_SUPPLY_STATUS_UNKNOWN;
}

static ssize_t supply_state_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct wm8350 *wm8350 = dev_get_drvdata(dev);
	char *supply;
	int state;

	state = wm8350_get_supplies(wm8350);
	switch (state) {
	case WM8350_BATT_SUPPLY:
		supply = "Battery";
		break;
	case WM8350_USB_SUPPLY:
		supply = "USB";
		break;
	case WM8350_LINE_SUPPLY:
		supply = "Line";
		break;
	case WM8350_USB_SUPPLY | WM8350_BATT_SUPPLY:
		supply = "Battery and USB";
		break;
	default:
		return 0;
	}

	return sprintf(buf, "%s\n", supply);
}

static DEVICE_ATTR(supply_state, 0444, supply_state_show, NULL);

static ssize_t battery_state_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct wm8350 *wm8350 = dev_get_drvdata(dev);
	char *batt;
	int state;

	state = wm8350_batt_status(wm8350);
	switch (state) {
	case POWER_SUPPLY_STATUS_DISCHARGING:
		batt = "Discharging";
		break;
	case POWER_SUPPLY_STATUS_CHARGING:
		batt = "Charging";
		break;
	case POWER_SUPPLY_STATUS_UNKNOWN:
		batt = "Unknown";
		break;
	default:
		return 0;
	}

	return sprintf(buf, "%s\n", batt);
}

static DEVICE_ATTR(battery_state, 0444, battery_state_show, NULL);

static ssize_t charger_state_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct wm8350 *wm8350 = dev_get_drvdata(dev);
	char *charge;
	int state;

	state = wm8350_reg_read(wm8350, WM8350_BATTERY_CHARGER_CONTROL_2) &
	    WM8350_CHG_STS_MASK;
	switch (state) {
	case WM8350_CHG_STS_OFF:
		charge = "Charger Off";
		break;
	case WM8350_CHG_STS_TRICKLE:
		charge = "Trickle Charging";
		break;
	case WM8350_CHG_STS_FAST:
		charge = "Fast Charging";
		break;
	default:
		return 0;
	}

	return sprintf(buf, "%s\n", charge);
}

static DEVICE_ATTR(charger_state, 0444, charger_state_show, NULL);

static void wm8350_charger_handler(struct wm8350 *wm8350, int irq, void *data)
{
	struct wm8350_power *power = &wm8350->power;
	struct wm8350_charger_policy *policy = power->policy;

	mutex_lock(&power->charger_mutex);
	switch (irq) {
	case WM8350_IRQ_CHG_BAT_HOT:
		printk(KERN_ERR "wm8350-power: battery too hot\n");
		break;
	case WM8350_IRQ_CHG_BAT_COLD:
		printk(KERN_ERR "wm8350-power: battery too cold\n");
		break;
	case WM8350_IRQ_CHG_BAT_FAIL:
		printk(KERN_ERR "wm8350-power: battery failed\n");
		break;
	case WM8350_IRQ_CHG_TO:
		printk(KERN_INFO "wm8350-power: charger timeout\n");
		break;
	case WM8350_IRQ_CHG_END:
		printk(KERN_INFO "wm8350-power: charger stopped\n");
		break;
	case WM8350_IRQ_CHG_START:
		printk(KERN_INFO "wm8350-power: charger started\n");
		break;
	case WM8350_IRQ_CHG_FAST_RDY:
		/* we are ready to fast charge */
		printk(KERN_INFO "wm8350-power: fast charger ready\n");
		wm8350_charger_config(wm8350, policy);
		wm8350_reg_unlock(wm8350);
		wm8350_set_bits(wm8350, WM8350_BATTERY_CHARGER_CONTROL_1,
			WM8350_CHG_FAST);
		wm8350_reg_lock(wm8350);
		wm8350_charger_enable(wm8350, 1);
		break;
	case WM8350_IRQ_CHG_VBATT_LT_3P9:
		printk(KERN_WARNING "wm8350-power: battery < 3.9V\n");
		wm8350_charger_enable(wm8350, 1);
		break;
	case WM8350_IRQ_CHG_VBATT_LT_3P1:
		printk(KERN_WARNING "wm8350-power: battery < 3.1V\n");
		wm8350_charger_enable(wm8350, 1);
		break;
	case WM8350_IRQ_CHG_VBATT_LT_2P85:
		printk(KERN_WARNING "wm8350-power: battery < 2.85V\n");
		wm8350_charger_enable(wm8350, 1);
		break;
	case WM8350_EXT_USB_FB_EINT:
		printk(KERN_INFO "wm8350-power: USB is now supply\n");
		power->is_usb_supply = 1;
		wm8350_charger_config(wm8350, policy);
		wm8350_charger_enable(wm8350, 1);
		break;
	case WM8350_EXT_WALL_FB_EINT:
		printk(KERN_INFO "wm8350-power: AC is now supply\n");
		power->is_usb_supply = 0;
		wm8350_charger_config(wm8350, policy);
		wm8350_charger_enable(wm8350, 1);
		break;
	case WM8350_EXT_BAT_FB_EINT:
		printk(KERN_INFO "wm8350-power: Battery is now supply\n");
		power->is_usb_supply = 0;
		break;
	case WM8350_IRQ_SYS_HYST_COMP_FAIL:
		printk(KERN_INFO "wm8350-power : shut down system\n");
		wm8350_set_bits(wm8350, WM8350_POWER_CHECK_COMPARATOR, WM8350_PCCMP_ERRACT);
		break;
	default:
		printk(KERN_ERR "wm8350-power: irq %d don't care\n", irq);
	}
	mutex_unlock(&power->charger_mutex);
}

/*********************************************************************
 *		AC Power
 *********************************************************************/
static int wm8350_ac_get_prop(struct power_supply *psy,
			      enum power_supply_property psp,
			      union power_supply_propval *val)
{
	struct wm8350 *wm8350 = dev_get_drvdata(psy->dev->parent);
	int ret = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = !!(wm8350_get_supplies(wm8350) &
				 WM8350_LINE_SUPPLY);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = wm8350_read_line_uvolts(wm8350);
		break;
	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}

static enum power_supply_property wm8350_ac_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
};

/*********************************************************************
 *		USB Power
 *********************************************************************/
static int wm8350_usb_get_prop(struct power_supply *psy,
			       enum power_supply_property psp,
			       union power_supply_propval *val)
{
	struct wm8350 *wm8350 = dev_get_drvdata(psy->dev->parent);
	int ret = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = !!(wm8350_get_supplies(wm8350) &
				 WM8350_USB_SUPPLY);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = wm8350_read_usb_uvolts(wm8350);
		break;
	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}

static enum power_supply_property wm8350_usb_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
};

/*********************************************************************
 *		Battery properties
 *********************************************************************/

static int wm8350_bat_get_property(struct power_supply *psy,
				   enum power_supply_property psp,
				   union power_supply_propval *val)
{
	struct wm8350 *wm8350 = dev_get_drvdata(psy->dev->parent);
	int ret = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		val->intval = wm8350_batt_status(wm8350);
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = !!(wm8350_get_supplies(wm8350) &
				 WM8350_BATT_SUPPLY);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = wm8350_read_battery_uvolts(wm8350);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static enum power_supply_property wm8350_bat_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
};

/*********************************************************************
 *		Initialisation
 *********************************************************************/

static void wm8350_init_charger(struct wm8350 *wm8350)
{
	/* register our interest in charger events */
	wm8350_register_irq(wm8350, WM8350_IRQ_CHG_BAT_HOT,
			    wm8350_charger_handler, NULL);
	wm8350_unmask_irq(wm8350, WM8350_IRQ_CHG_BAT_HOT);
	wm8350_register_irq(wm8350, WM8350_IRQ_CHG_BAT_COLD,
			    wm8350_charger_handler, NULL);
	wm8350_unmask_irq(wm8350, WM8350_IRQ_CHG_BAT_COLD);
	wm8350_register_irq(wm8350, WM8350_IRQ_CHG_BAT_FAIL,
			    wm8350_charger_handler, NULL);
	wm8350_unmask_irq(wm8350, WM8350_IRQ_CHG_BAT_FAIL);
	wm8350_register_irq(wm8350, WM8350_IRQ_CHG_TO,
			    wm8350_charger_handler, NULL);
	wm8350_unmask_irq(wm8350, WM8350_IRQ_CHG_TO);
	wm8350_register_irq(wm8350, WM8350_IRQ_CHG_END,
			    wm8350_charger_handler, NULL);
	wm8350_unmask_irq(wm8350, WM8350_IRQ_CHG_END);
	wm8350_register_irq(wm8350, WM8350_IRQ_CHG_START,
			    wm8350_charger_handler, NULL);
	wm8350_unmask_irq(wm8350, WM8350_IRQ_CHG_START);
	wm8350_register_irq(wm8350, WM8350_IRQ_CHG_FAST_RDY,
			    wm8350_charger_handler, NULL);
	wm8350_unmask_irq(wm8350, WM8350_IRQ_CHG_FAST_RDY);
	wm8350_register_irq(wm8350, WM8350_IRQ_CHG_VBATT_LT_3P9,
			    wm8350_charger_handler, NULL);
	wm8350_unmask_irq(wm8350, WM8350_IRQ_CHG_VBATT_LT_3P9);
	wm8350_register_irq(wm8350, WM8350_IRQ_CHG_VBATT_LT_3P1,
			    wm8350_charger_handler, NULL);
	wm8350_unmask_irq(wm8350, WM8350_IRQ_CHG_VBATT_LT_3P1);
	wm8350_register_irq(wm8350, WM8350_IRQ_CHG_VBATT_LT_2P85,
			    wm8350_charger_handler, NULL);
	wm8350_unmask_irq(wm8350, WM8350_IRQ_CHG_VBATT_LT_2P85);

	/* and supply change events */
	wm8350_register_irq(wm8350, WM8350_EXT_USB_FB_EINT,
			    wm8350_charger_handler, NULL);
	wm8350_unmask_irq(wm8350, WM8350_EXT_USB_FB_EINT);
	wm8350_register_irq(wm8350, WM8350_EXT_WALL_FB_EINT,
			    wm8350_charger_handler, NULL);
	wm8350_unmask_irq(wm8350, WM8350_EXT_WALL_FB_EINT);
	wm8350_register_irq(wm8350, WM8350_EXT_BAT_FB_EINT,
			    wm8350_charger_handler, NULL);
	wm8350_unmask_irq(wm8350, WM8350_EXT_BAT_FB_EINT);

	/* system monitoring */
	wm8350_register_irq(wm8350, WM8350_IRQ_SYS_HYST_COMP_FAIL,
				wm8350_charger_handler, NULL);
	wm8350_unmask_irq(wm8350, WM8350_IRQ_SYS_HYST_COMP_FAIL);
}

static void free_charger_irq(struct wm8350 *wm8350)
{
	wm8350_mask_irq(wm8350, WM8350_IRQ_CHG_BAT_HOT);
	wm8350_free_irq(wm8350, WM8350_IRQ_CHG_BAT_HOT);
	wm8350_mask_irq(wm8350, WM8350_IRQ_CHG_BAT_COLD);
	wm8350_free_irq(wm8350, WM8350_IRQ_CHG_BAT_COLD);
	wm8350_mask_irq(wm8350, WM8350_IRQ_CHG_BAT_FAIL);
	wm8350_free_irq(wm8350, WM8350_IRQ_CHG_BAT_FAIL);
	wm8350_mask_irq(wm8350, WM8350_IRQ_CHG_TO);
	wm8350_free_irq(wm8350, WM8350_IRQ_CHG_TO);
	wm8350_mask_irq(wm8350, WM8350_IRQ_CHG_END);
	wm8350_free_irq(wm8350, WM8350_IRQ_CHG_END);
	wm8350_mask_irq(wm8350, WM8350_IRQ_CHG_START);
	wm8350_free_irq(wm8350, WM8350_IRQ_CHG_START);
	wm8350_mask_irq(wm8350, WM8350_IRQ_CHG_VBATT_LT_3P9);
	wm8350_free_irq(wm8350, WM8350_IRQ_CHG_VBATT_LT_3P9);
	wm8350_mask_irq(wm8350, WM8350_IRQ_CHG_VBATT_LT_3P1);
	wm8350_free_irq(wm8350, WM8350_IRQ_CHG_VBATT_LT_3P1);
	wm8350_mask_irq(wm8350, WM8350_IRQ_CHG_VBATT_LT_2P85);
	wm8350_free_irq(wm8350, WM8350_IRQ_CHG_VBATT_LT_2P85);
	wm8350_mask_irq(wm8350, WM8350_EXT_USB_FB_EINT);
	wm8350_free_irq(wm8350, WM8350_EXT_USB_FB_EINT);
	wm8350_mask_irq(wm8350, WM8350_EXT_WALL_FB_EINT);
	wm8350_free_irq(wm8350, WM8350_EXT_WALL_FB_EINT);
	wm8350_mask_irq(wm8350, WM8350_EXT_BAT_FB_EINT);
	wm8350_free_irq(wm8350, WM8350_EXT_BAT_FB_EINT);

	wm8350_mask_irq(wm8350, WM8350_IRQ_SYS_HYST_COMP_FAIL);
	wm8350_free_irq(wm8350, WM8350_IRQ_SYS_HYST_COMP_FAIL);
}

static int __init wm8350_power_probe(struct platform_device *pdev)
{
	struct wm8350 *wm8350 = platform_get_drvdata(pdev);
	struct wm8350_power *power = &wm8350->power;
	struct wm8350_charger_policy *policy = power->policy;
	struct power_supply *usb = &power->usb;
	struct power_supply *battery = &power->battery;
	struct power_supply *ac = &power->ac;
	int ret;

	printk(KERN_INFO "wm8350: power driver %s\n", WM8350_POWER_VERSION);

	mutex_init(&power->charger_mutex);

	ac->name = "wm8350-ac";
	ac->type = POWER_SUPPLY_TYPE_MAINS;
	ac->properties = wm8350_ac_props;
	ac->num_properties = ARRAY_SIZE(wm8350_ac_props);
	ac->get_property = wm8350_ac_get_prop;
	ret = power_supply_register(&pdev->dev, ac);
	if (ret)
		return ret;

	battery->name = "wm8350-battery";
	battery->properties = wm8350_bat_props;
	battery->num_properties = ARRAY_SIZE(wm8350_bat_props);
	battery->get_property = wm8350_bat_get_property;
	battery->use_for_apm = 1;
	ret = power_supply_register(&pdev->dev, battery);
	if (ret)
		goto battery_failed;

	usb->name = "wm8350-usb",
	usb->type = POWER_SUPPLY_TYPE_USB;
	usb->properties = wm8350_usb_props;
	usb->num_properties = ARRAY_SIZE(wm8350_usb_props);
	usb->get_property = wm8350_usb_get_prop;
	ret = power_supply_register(&pdev->dev, usb);
	if (ret)
		goto usb_failed;

	ret = device_create_file(&pdev->dev, &dev_attr_supply_state);
	if (ret < 0)
		printk(KERN_WARNING "wm8350: failed to add supply sysfs\n");
	ret = device_create_file(&pdev->dev, &dev_attr_battery_state);
	if (ret < 0)
		printk(KERN_WARNING "wm8350: failed to add battery sysfs\n");
	ret = device_create_file(&pdev->dev, &dev_attr_charger_state);
	if (ret < 0)
		printk(KERN_WARNING "wm8350: failed to add charge sysfs\n");
	ret = 0;
	goto success;

usb_failed:
	power_supply_unregister(battery);
battery_failed:
	power_supply_unregister(ac);
success:

	if (policy == NULL)
		printk(KERN_INFO "%s: no charger policy - "
			"charging disabled\n", __func__);
	else {
		wm8350_init_charger(wm8350);
		power->charger_enabled =
			wm8350_charger_config(wm8350, policy);
			wm8350_fast_charger_mode(wm8350);
			wm8350_charger_enable(wm8350, 1);
		if (power->charger_enabled < 0) {
			printk(KERN_ERR "%s: failed to enable charger\n",
				__func__);
			free_charger_irq(wm8350);
		}
	}

	return ret;
}

static int __devexit wm8350_power_remove(struct platform_device *pdev)
{
	struct wm8350 *wm8350 = platform_get_drvdata(pdev);
	struct wm8350_power *power = &wm8350->power;

	if (power->charger_enabled) {
		wm8350_charger_enable(wm8350, 0);
		free_charger_irq(wm8350);
	}
	device_remove_file(&pdev->dev, &dev_attr_supply_state);
	device_remove_file(&pdev->dev, &dev_attr_battery_state);
	device_remove_file(&pdev->dev, &dev_attr_charger_state);
	power_supply_unregister(&power->battery);
	power_supply_unregister(&power->ac);
	power_supply_unregister(&power->usb);
	return 0;
}

struct platform_driver wm8350_power_driver = {
	.probe = wm8350_power_probe,
	.remove = __devexit(wm8350_power_remove),
	.driver = {
		.name = "wm8350-power",
	},
};

static int __init wm8350_bat_init(void)
{
	return platform_driver_register(&wm8350_power_driver);
}

static void __exit wm8350_bat_exit(void)
{
	platform_driver_unregister(&wm8350_power_driver);
}

module_init(wm8350_bat_init);
module_exit(wm8350_bat_exit);

MODULE_AUTHOR("Graeme Gregory <gg@opensource.wolfsonmicro.com,"
	      "Liam Girdwood <lg@opensource.wolfsonmicro.com");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Battery driver for WM8350");
