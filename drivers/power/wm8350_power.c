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
#include <linux/circ_buf.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>

#include <asm/io.h>
#include <asm/arch/regs-s3c2416-clock.h>
#include <asm/arch/regs-gpio.h>

#include <linux/mfd/wm8350/supply.h>
#include <linux/mfd/wm8350/core.h>
#include <linux/mfd/wm8350/gpio.h>
#include <linux/mfd/wm8350/comparator.h>

#include <asm/plat-s3c24xx/s3c2416.h>

#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#endif

#define WM8350_POWER_VERSION	"0.5"

#define WM8350_BATT_SUPPLY	1
#define WM8350_USB_SUPPLY	2
#define WM8350_LINE_SUPPLY	4

#define DEV_NAME	"wm8350_bat"
#define CIRC_BUF_MAX	16	
#define CIRC_ADD(elem,cir_buf,size)				\
	down(&event_mutex);					\
	if(CIRC_SPACE(cir_buf.head, cir_buf.tail, size)){	\
		cir_buf.buf[cir_buf.head] = (char)elem;		\
		cir_buf.head = (cir_buf.head + 1) & (size - 1);	\
	} else {						\
		pr_debug("Failed to notify event to the user\n");\
	}							\
	up(&event_mutex);

#define CIRC_REMOVE(elem,cir_buf,size)				\
	down(&event_mutex);					\
	if(CIRC_CNT(cir_buf.head, cir_buf.tail, size)){         \
		elem = (int)cir_buf.buf[cir_buf.tail];          \
		cir_buf.tail = (cir_buf.tail + 1) & (size - 1); \
	} else {                                                \
		elem = -1;                                      \
		pr_debug("No valid notified event\n");           \
	}							\
	up(&event_mutex);

typedef struct {
	int reg;
	int value;
} wm8350_reg_info;

#define WM8350_BATTERY_VOLTAGE		_IOR('P', 0xa4, int)
#define WM8350_BATTERY_ADC		_IOR('P', 0xae, int)
#define WM8350_AC_ONLINE		_IOR('P', 0xa5, int)
#define WM8350_USB_ONLINE		_IOR('P', 0xa6, int)
#define WM8350_BAT_STATUS		_IOWR('P', 0xa7, int)
#define WM8350_BAT_NOTIFY_USER		_IOR('P', 0xa8, int)
#define WM8350_BAT_GET_NOTIFY		_IOR('P', 0xa9, int)
#define WM8350_BAT_SUBSCRIBE		_IOR('P', 0xaa, int)
#define WM8350_BAT_UNSUBSCRIBE		_IOR('P', 0xab, int)
#define WM8350_BAT_REMOVE_USER		_IOR('P', 0xac, int)
#define WM8350_BAT_LED_CONTROL		_IOWR('P', 0xab, int)
#define WM8350_REG_READ			_IOWR('P', 0xac, wm8350_reg_info*)
#define WM8350_REG_WRITE		_IOWR('P', 0xad, wm8350_reg_info*)
#define WM8350_PHONE_DIR		_IOR('P', 0xaf, int)

#define CHG_MODE_STATUS		0x3000

typedef enum _t_wm8350_chg_status {
	WM8350_BAT_UVOLTAGE = 0,
	WM8350_CHG_MODE_STATUS,
	WM8350_LINE_STATUS,
} wm8350_chg_status;

typedef enum {
	WM8350_EVENT_CHARGING = 0,
	WM8350_EVENT_BATTERY,
	WM8350_EVENT_FULL_CHARGED,
	WM8350_EVENT_FAULT,
	WM8350_EVENT_PIR,		/*!< \note temporary code for K5 canopus pir sensor */
	WM8350_BAT_EVENT_NUM,
} type_chg_event;

typedef enum {
	WM8350_BAT_GREEN_LED = 0,
	WM8350_BAT_RED_LED,
	WM8350_BAT_LED_ALL_ON,
	WM8350_BAT_LED_ALL_OFF,
} wm8350_led_control;

typedef struct {
	void (*func) (void *);
	void *param;
} wm8350_bat_event_callback_t;

typedef struct {
	// keeps a list of subsecibed clients to an event
	struct list_head list;

	// Callback function with parameter, called whend event occurs
	wm8350_bat_event_callback_t callback;
} wm8350_bat_event_callback_list_t;

typedef struct {
	unsigned int command;
	unsigned int status;
	unsigned int uvolt;	
} _t_wm8350_chg_data;

typedef struct {
	unsigned int command;
	unsigned int control;
} _t_wm8350_led_data;

static int wm8350_bat_major;
static struct class *wm8350_dev_class;
static struct fasync_struct *wm8350_bat_queue;
static struct circ_buf wm8350_event;
static struct list_head wm8350_bat_events[WM8350_BAT_EVENT_NUM];
static struct wm8350 *wm8350_bat = NULL;

#ifdef CONFIG_HAS_WAKELOCK
static struct wake_lock _bat_wake_lock;
static struct wake_lock _fault_led_wake_lock;
#endif

static DECLARE_MUTEX(event_mutex);

static int bat_fault_led_status = 0;

static int _ac_count;
static int _ac_state;

static void _wm8350_ac_detect_work(struct work_struct *work);
static void _wm8350_full_check_work(struct work_struct *work);
static void _wm8350_bat_fault_led_work(struct work_struct *work);

DECLARE_DELAYED_WORK(_ac_detect, _wm8350_ac_detect_work);
DECLARE_DELAYED_WORK(_full_check, _wm8350_full_check_work);
DECLARE_DELAYED_WORK(_bat_fault_led, _wm8350_bat_fault_led_work);

static int wm8350_bat_green_led_show(struct device *dev, struct device_attribute *attr, 
					char *buf)
{
	struct wm8350 *wm8350 = dev_get_drvdata(dev);
	int ret;

	ret = snprintf(buf, PAGE_SIZE, "%d\n",
			(wm8350_gpio_get_dir(wm8350, 10) ==  WM8350_GPIO_DIR_OUT) ? 1 : 0);
		
	return ret;
}

static int wm8350_bat_green_led_store(struct device *dev, struct device_attribute *attr,
					const char *buf, size_t len)
{
	unsigned long value = simple_strtoul(buf, NULL, 10);
	struct wm8350 *wm8350 = dev_get_drvdata(dev);

	if (value > 1)
		return -ERANGE;

	wm8350_gpio_set_status(wm8350, 10, !value);
	wm8350_gpio_set_dir(wm8350, 10, value ? WM8350_GPIO_DIR_OUT : WM8350_GPIO_DIR_IN);

	return len;
}

static int wm8350_bat_red_led_show(struct device *dev, struct device_attribute *attr, 
					char *buf)
{
	struct wm8350 *wm8350 = dev_get_drvdata(dev);
	int ret;

	ret = snprintf(buf, PAGE_SIZE, "%d\n",
			(wm8350_gpio_get_dir(wm8350, 11) ==  WM8350_GPIO_DIR_OUT) ? 1 : 0);
		
	return ret;
}

static int wm8350_bat_red_led_store(struct device *dev, struct device_attribute *attr,
					const char *buf, size_t len)
{
	unsigned long value = simple_strtoul(buf, NULL, 10);
	struct wm8350 *wm8350 = dev_get_drvdata(dev);

	if (value > 1)
		return -ERANGE;

	wm8350_gpio_set_status(wm8350, 11, !value);
	wm8350_gpio_set_dir(wm8350, 11, value ? WM8350_GPIO_DIR_OUT : WM8350_GPIO_DIR_IN);

	return len;
}

static DEVICE_ATTR(green_led, 0644,
		wm8350_bat_green_led_show,
		wm8350_bat_green_led_store);

static DEVICE_ATTR(red_led, 0644,
		wm8350_bat_red_led_show,
		wm8350_bat_red_led_store);

/* battery voltage sensing */
static int wm8350_aux2_show(struct device *dev, struct device_attribute *attr,
					char *buf)
{
	struct wm8350 *wm8350 = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "%x\n", wm8350_read_aux2_adc(wm8350));

}

static int wm8350_aux3_show(struct device *dev, struct device_attribute *attr,
					char *buf)
{
	struct wm8350 *wm8350 = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "%x\n", wm8350_read_aux3_adc(wm8350));

}

static DEVICE_ATTR(aux2_adc, 0444,
		wm8350_aux2_show,
		NULL);

static DEVICE_ATTR(aux3_adc, 0444,
		wm8350_aux3_show,
		NULL);

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
		printk(KERN_DEBUG "%s: USB fast charge > 500mA\n", __func__);
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

static void wm8350_bat_led_status(struct wm8350 *wm8350, int state)
{
	int red, green;

	switch (state) {
	case WM8350_BAT_RED_LED: 
		red = 1; green = 0;
		break;
	case WM8350_BAT_GREEN_LED:
		red = 0; green = 1;
		break;
	case WM8350_BAT_LED_ALL_ON:
		red = 1; green = 1;
		break;
	case WM8350_BAT_LED_ALL_OFF:
		red = 0; green = 0;
		break;

	default:
		printk(KERN_DEBUG "BATTERRY LED NOT CONTROL..\n");
		return ;
	}

	wm8350_gpio_set_status(wm8350, 10, !green);
	wm8350_gpio_set_dir(wm8350, 10, green ? WM8350_GPIO_DIR_OUT : WM8350_GPIO_DIR_IN);
	wm8350_gpio_set_status(wm8350, 11, !red);
	wm8350_gpio_set_dir(wm8350, 11, red ? WM8350_GPIO_DIR_OUT : WM8350_GPIO_DIR_IN);
}
static void wm8350_bat_fault_led_control(int val)
{
	struct wm8350 *wm8350 = wm8350_bat;

	if (val)
		wm8350_bat_led_status(wm8350, WM8350_BAT_LED_ALL_ON);
	else
		wm8350_bat_led_status(wm8350, WM8350_BAT_LED_ALL_OFF);
}

static type_chg_event _main_state = WM8350_EVENT_BATTERY;

static void _change_state(int new_state)
{
	const char *mode = "";
	struct list_head *p;
	struct wm8350 *wm8350 = wm8350_bat;
	wm8350_bat_event_callback_list_t *temp = NULL;

	/* state control */
	if (new_state == _main_state)
		return;

	_main_state = new_state;
	
	if (!list_empty(&wm8350_bat_events[_main_state])) {
		list_for_each(p, &wm8350_bat_events[_main_state]) {
			temp = list_entry(p, wm8350_bat_event_callback_list_t, list);
			temp->callback.func(temp->callback.param);
		}
	}

	cancel_delayed_work(&_bat_fault_led);
	
	switch (_main_state) {
	case WM8350_EVENT_CHARGING:
		mode = "WM8350_EVENT_CHARGING";
		wm8350_bat_led_status(wm8350, WM8350_BAT_RED_LED);
		break;
	case WM8350_EVENT_BATTERY:
		mode = "WM8350_EVENT_BATTERY";
		wm8350_bat_led_status(wm8350, WM8350_BAT_LED_ALL_OFF);
		break;
	case WM8350_EVENT_FULL_CHARGED:
		mode = "WM8350_EVENT_FULL_CHARGED";
		wm8350_bat_led_status(wm8350, WM8350_BAT_GREEN_LED);
		break;
	case WM8350_EVENT_FAULT:
		mode = "WM8350_EVENT_FAULT";
		bat_fault_led_status = 0;
		_wm8350_bat_fault_led_work(NULL);
		break;
	}
	printk(KERN_DEBUG "wm8350-power: mode = %s\n", mode);
}

static void _fault_process(int irq)
{
	int vol;
	bool over_39v = false;
	struct wm8350 *wm8350 = wm8350_bat;
	type_chg_event state;
	
	if (_main_state == WM8350_EVENT_BATTERY ||
	    _main_state == WM8350_EVENT_FULL_CHARGED)
		return;

	/* disabled during chaging power source */
	if (_ac_count > 0)
		return;

	if (q_hw_ver(7800_ES2)) {
		if ((vol = wm8350_read_battery_uvolts(wm8350)) > 3900000)
			over_39v = true;
	} else {
		if ((vol = wm8350_read_aux2_adc(wm8350)) > 0x77d)
			over_39v = true;
	}
	
	state = (over_39v) ? WM8350_EVENT_FULL_CHARGED : WM8350_EVENT_FAULT;
	_change_state(state);
}

static void wm8350_charger_handler(struct wm8350 *wm8350, int irq, void *data)
{
	struct wm8350_power *power = &wm8350->power;
	struct wm8350_charger_policy *policy = power->policy;

#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_timeout(&_bat_wake_lock, 5 * HZ);
#endif

	mutex_lock(&power->charger_mutex);
	switch (irq) {
	case WM8350_IRQ_CHG_BAT_HOT:
		printk(KERN_DEBUG "wm8350-power: battery too hot\n");
		_fault_process(irq);
		break;
	case WM8350_IRQ_CHG_BAT_COLD:
		printk(KERN_DEBUG "wm8350-power: battery too cold\n");
		_fault_process(irq);
		break;
	case WM8350_IRQ_CHG_BAT_FAIL:
		printk(KERN_DEBUG "wm8350-power: battery failed\n");
		_fault_process(irq);
		break;
	case WM8350_IRQ_CHG_TO:
		printk(KERN_DEBUG "wm8350-power: charger timeout\n");
		_fault_process(irq);
		break;
	case WM8350_IRQ_CHG_END:
		printk(KERN_DEBUG "wm8350-power: charger stopped\n");
		if (_main_state != WM8350_EVENT_BATTERY)
			_change_state(WM8350_EVENT_FULL_CHARGED);
		break;
	case WM8350_IRQ_CHG_START:
		printk(KERN_DEBUG "wm8350-power: charger started\n");
		if (_main_state != WM8350_EVENT_FULL_CHARGED)
			_change_state(WM8350_EVENT_CHARGING);
		break;
	case WM8350_IRQ_CHG_FAST_RDY:
		/* we are ready to fast charge */
		printk(KERN_DEBUG "wm8350-power: fast charger ready\n");
		wm8350_charger_config(wm8350, policy);
		wm8350_reg_unlock(wm8350);
		wm8350_set_bits(wm8350, WM8350_BATTERY_CHARGER_CONTROL_1,
			WM8350_CHG_FAST);
		wm8350_reg_lock(wm8350);
		wm8350_charger_enable(wm8350, 1);
		break;
	case WM8350_IRQ_CHG_VBATT_LT_3P9:
		printk(KERN_DEBUG "wm8350-power: battery < 3.9V\n");
		wm8350_charger_enable(wm8350, 1);
		break;
	case WM8350_IRQ_CHG_VBATT_LT_3P1:
		printk(KERN_DEBUG "wm8350-power: battery < 3.1V\n");
		wm8350_charger_enable(wm8350, 1);
		break;
	case WM8350_IRQ_CHG_VBATT_LT_2P85:
		printk(KERN_DEBUG "wm8350-power: battery < 2.85V\n");
		wm8350_charger_enable(wm8350, 1);
		break;
	case WM8350_IRQ_EXT_USB_FB:
		printk(KERN_DEBUG "wm8350-power: USB is now supply\n");
		power->is_usb_supply = 1;
		wm8350_charger_config(wm8350, policy);
		wm8350_charger_enable(wm8350, 1);
		break;
	case WM8350_IRQ_EXT_WALL_FB:
		printk(KERN_DEBUG "wm8350-power: AC power is now supply\n");
		power->is_usb_supply = 0;
		wm8350_charger_config(wm8350, policy);
		wm8350_charger_enable(wm8350, 1);
		schedule_delayed_work(&_ac_detect, msecs_to_jiffies(10));
		break;
	case WM8350_IRQ_EXT_BAT_FB:
		printk(KERN_DEBUG "wm8350-power: Battery is now supply\n");
		power->is_usb_supply = 0;
		break;
	default:
		printk(KERN_DEBUG "wm8350-power: irq %d don't care\n", irq);
	}

	mutex_unlock(&power->charger_mutex);
}

/*********************************************************************
 *		Timer Driver	
 *********************************************************************/

#define CHECK_AC_COUNT	100

static void _wm8350_full_check_work(struct work_struct *work)
{
	struct wm8350 *wm8350 = wm8350_bat;

	if (_main_state == WM8350_EVENT_CHARGING &&
	    wm8350_batt_status(wm8350) == POWER_SUPPLY_STATUS_DISCHARGING) {
		_change_state(WM8350_EVENT_FULL_CHARGED);
	}
}

static void _wm8350_ac_detect_work(struct work_struct *work)
{
	int state;
	struct wm8350 *wm8350 = wm8350_bat;
	struct wm8350_power *power = &wm8350->power;

	mutex_lock(&power->charger_mutex);

	state = wm8350_get_supplies(wm8350);
	cancel_delayed_work(&_full_check);

	if (++_ac_count > CHECK_AC_COUNT) {
		_ac_count = 0;
		
		if (_ac_state == WM8350_BATT_SUPPLY) {
			if (_main_state != WM8350_EVENT_BATTERY)
				_change_state(WM8350_EVENT_BATTERY);
		} else {
			if (_main_state == WM8350_EVENT_BATTERY) {
				_change_state(WM8350_EVENT_CHARGING);
				schedule_delayed_work(&_full_check, msecs_to_jiffies(60*1000));
			}
		}
	} else {
		if(state != _ac_state) {
			_ac_count = 1;
			_ac_state = state;
		}
		schedule_delayed_work(&_ac_detect, msecs_to_jiffies(10));
	}

	mutex_unlock(&power->charger_mutex);
}

static void _wm8350_bat_fault_led_work(struct work_struct *work)
{
#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_timeout(&_fault_led_wake_lock, 1 * HZ);
#endif

	if (bat_fault_led_status) {
		bat_fault_led_status = 0;
		wm8350_bat_fault_led_control(1);
	} else {
		bat_fault_led_status = 1;
		wm8350_bat_fault_led_control(0);
	}

	schedule_delayed_work(&_bat_fault_led, msecs_to_jiffies(500));
}

/*********************************************************************
 *		Control Driver	
 *********************************************************************/
static int wm8350_bat_event_subscribe(type_chg_event event,
					wm8350_bat_event_callback_t callback)
{
	wm8350_bat_event_callback_list_t *new = NULL;

	pr_debug("Event: %d Subscribe\n", event);

	/* Check wheter the event & callback are valid? */
	if (event >= WM8350_BAT_EVENT_NUM) {
		printk(KERN_DEBUG "Invalid Event : %d\n", event);
		return -EINVAL;
	}

	if (NULL == callback.func) {
		pr_debug("Null or Invalid Callback\n");
		return -EINVAL;
	}

	/* Create a new linked list entry */
	new = kmalloc(sizeof(wm8350_bat_event_callback_list_t), GFP_KERNEL);
	if (NULL == new) {
		return -ENOMEM;
	}

	/* Initialize the list node fields */
	new->callback.func = callback.func;
	new->callback.param = callback.param;
	INIT_LIST_HEAD(&new->list);

	/* Obtain the lock to access the list */
	if (down_interruptible(&event_mutex)) {
		kfree(new);
		return -EINTR;
	}

	/* Add this entry to the event list */
	list_add_tail(&new->list, &wm8350_bat_events[event]);

	/* Release the lock */
	up(&event_mutex);

	return 0;
}

static int wm8350_bat_event_unsubscribe(type_chg_event event,
				   wm8350_bat_event_callback_t callback)
{
	struct list_head *p;
	struct list_head *n;
	wm8350_bat_event_callback_list_t *temp = NULL;
	int ret = -1;

	pr_debug("Event:%d Unsubscribe\n", event);

	/* Check whether the event & callback are valid? */
	if (event >= WM8350_BAT_EVENT_NUM) {
		printk(KERN_DEBUG "Invalid Event:%d\n", event);
		return -EINVAL;
	}

	if (NULL == callback.func) {
		pr_debug("Null or Invalid Callback\n");
		return -EINVAL;
	}

	/* Obtain the lock to access the list */
	if (down_interruptible(&event_mutex)) {
		return -EINTR;
	}

	/* Find the entry in the list */
	list_for_each_safe(p, n, &wm8350_bat_events[event]) {
		temp = list_entry(p, wm8350_bat_event_callback_list_t, list);
		if (temp->callback.func == callback.func
		    && temp->callback.param == callback.param) {
			/* Remove the entry from the list */
			list_del(p);
			kfree(temp);
			ret = 0;
			break;
		}
	}

	/* Release the lock */
	up(&event_mutex);

	return ret;
}

static void wm8350_bat_event_list_init(void)
{
	int i;
	for (i=0; i<WM8350_BAT_EVENT_NUM; i++) {
		INIT_LIST_HEAD(&wm8350_bat_events[i]);
	}

	sema_init(&event_mutex, 1);
}

static void wm8350_bat_callbackfn(void *event)
{
	printk(KERN_DEBUG "\n\n DETECT WM8350 BAT EVENT : %d\n\n",
			(unsigned int)event);
}

static void wm8350_bat_user_notify_callback(void *event)
{
	printk(KERN_DEBUG "user_notify_callback ...\n");

	CIRC_ADD((int)event, wm8350_event, CIRC_BUF_MAX);
	kill_fasync(&wm8350_bat_queue, SIGIO, POLL_IN);
}

static int wm8350_bat_open(struct inode *inode, struct file *file)
{
	return 0;
}
static int wm8350_bat_release(struct inode *inode, struct file *file)
{
	int i;
	wm8350_bat_event_callback_t event_sub;

	for(i=0; i<WM8350_BAT_EVENT_NUM; i++) {
		event_sub.func = wm8350_bat_user_notify_callback;
		event_sub.param = (void *)i;
		wm8350_bat_event_unsubscribe(i, event_sub);	

		event_sub.func = wm8350_bat_callbackfn;
		wm8350_bat_event_unsubscribe(i, event_sub);
	}

	wm8350_event.head = wm8350_event.tail = 0;

	return 0;
}

static int wm8350_bat_get_status(struct wm8350 *wm8350, unsigned long arg)
{
	_t_wm8350_chg_data chg_data = {0};

	if (copy_from_user((void *)&chg_data,
				(const void *)arg,
				sizeof(chg_data)))
		return -EFAULT;

	switch (chg_data.command) {
		case WM8350_BAT_UVOLTAGE:
			chg_data.uvolt = wm8350_read_battery_uvolts(wm8350);
			break;

		case WM8350_CHG_MODE_STATUS:
			chg_data.status = (wm8350_reg_read(wm8350, 
					WM8350_BATTERY_CHARGER_CONTROL_2) &
					CHG_MODE_STATUS) >> 12;
			break;
	
		case WM8350_LINE_STATUS:
			chg_data.status = (wm8350_get_supplies(wm8350) & 
					WM8350_LINE_SUPPLY) >> 2; 
				break;

		default:
			printk(KERN_DEBUG "wm8350 bat : unsupported command 0x%x\n", chg_data.command);
			return 0;
	}

	if (copy_to_user((void *)arg,
				&chg_data, sizeof(chg_data)))
		return -EFAULT;

	return 0;
}

static int wm8350_bat_led_control(struct wm8350 *wm8350, unsigned long arg)
{
	_t_wm8350_led_data led_data = {0};

	if (copy_from_user((void *)&led_data,
				(const void *)arg,
				sizeof(led_data)))
		return -EFAULT;

	int port = 10;

	if (led_data.command == WM8350_BAT_RED_LED) port = 11;
	if (led_data.control) {
		wm8350_gpio_set_status(wm8350, port, 0);
		wm8350_gpio_set_dir(wm8350, port, WM8350_GPIO_DIR_OUT);
	} else {
		wm8350_gpio_set_status(wm8350, port, 1);
		wm8350_gpio_set_dir(wm8350, port, WM8350_GPIO_DIR_IN);
	}

	return 0;
}

static int wm8350_bat_ioctl(struct inode *inode, struct file *file,
				unsigned int cmd, unsigned long arg)
{
	
	int ret = 0, online;
	struct wm8350 *wm8350 = wm8350_bat;
	wm8350_bat_event_callback_t event_sub;
	type_chg_event event;
	wm8350_reg_info reg_info;

	if (_IOC_TYPE(cmd) != 'P')
		return -ENOTTY;

	switch (cmd) {
		case WM8350_BATTERY_VOLTAGE:
			online = wm8350_read_battery_uvolts(wm8350);
			if (copy_to_user((int *) arg, &online, sizeof(int))) {
				return -EFAULT;
			}
			break;

		case WM8350_BATTERY_ADC:
			online = wm8350_read_aux2_adc(wm8350);
			if (copy_to_user((int *) arg, &online, sizeof(int))) {
				return -EFAULT;
			}
			break;

		case WM8350_AC_ONLINE:
			online = !!(wm8350_get_supplies(wm8350) & WM8350_LINE_SUPPLY);
			if (copy_to_user((int *) arg, &online, sizeof(int))) {
				return -EFAULT;
			}
			break;

		case WM8350_USB_ONLINE:
			online = !!(wm8350_get_supplies(wm8350) & WM8350_USB_SUPPLY);
			if (copy_to_user((int *) arg, &online, sizeof(int))) {
				return -EFAULT;
			}
			break;

		case WM8350_BAT_STATUS:
			ret = wm8350_bat_get_status(wm8350, arg);
			break;
		
		case WM8350_BAT_LED_CONTROL:
			ret = wm8350_bat_led_control(wm8350, arg);
			break;

		case WM8350_BAT_SUBSCRIBE:
			if (get_user(event, (int __user *)arg)) {
				return -EFAULT;
			}
			event_sub.func = wm8350_bat_callbackfn;
			event_sub.param = (void *)event;
			ret = wm8350_bat_event_subscribe(event, event_sub);
			pr_debug("subscribe done\n");
			break;
		
		case WM8350_BAT_UNSUBSCRIBE:
			if (get_user(event, (int __user *)arg)) {
				return -EFAULT;
			}
			event_sub.func = wm8350_bat_callbackfn;
			event_sub.param = (void *)event;
			ret = wm8350_bat_event_unsubscribe(event, event_sub);
			pr_debug("unsubscribe done\n");
			break;

		case WM8350_BAT_NOTIFY_USER:
			if (get_user(event, (int __user *)arg)) {
				return -EFAULT;
			}
			event_sub.func = wm8350_bat_user_notify_callback;
			event_sub.param = (void *)event;
			ret = wm8350_bat_event_subscribe(event, event_sub);
			break;

		case WM8350_BAT_GET_NOTIFY:
			CIRC_REMOVE(event, wm8350_event, CIRC_BUF_MAX);
			if (put_user(event, (int __user *)arg)) {
				return -EFAULT;
			}
			break;

		case WM8350_BAT_REMOVE_USER:
			if (get_user(event, (int __user *)arg)) {
				return -EFAULT;
			}
			event_sub.func = wm8350_bat_user_notify_callback;
			event_sub.param = (void *)event;
			ret = wm8350_bat_event_unsubscribe(event, event_sub);
			break;
		case WM8350_REG_READ:
			if (copy_from_user(&reg_info, (wm8350_reg_info *) arg,
					   sizeof(wm8350_reg_info))) {
				return -EFAULT;
			}

			reg_info.value = wm8350_reg_read(wm8350, reg_info.reg);
			pr_debug("read reg %d %x\n", reg_info.reg, reg_info.value);

			if (copy_to_user((wm8350_reg_info *) arg, &reg_info,
					 sizeof(wm8350_reg_info))) {
				return -EFAULT;
			}

			ret = 0;
			break;

		case WM8350_REG_WRITE:
			if (copy_from_user(&reg_info, (wm8350_reg_info *) arg,
					   sizeof(wm8350_reg_info))) {
				return -EFAULT;
			}

			ret = wm8350_reg_write(wm8350, reg_info.reg, reg_info.value);
			pr_debug("write reg %d %x\n", reg_info.reg, reg_info.value);

			if (copy_to_user((wm8350_reg_info *) arg, &reg_info,
					 sizeof(wm8350_reg_info))) {
				return -EFAULT;
			}

			break;

		case WM8350_PHONE_DIR:
			if (q_hw_ver(KTQOOK)) {
				online = s3c2410_gpio_getpin(S3C2410_GPE6) ? 1 : 0;
			} else {
				online = 0;
			}

			if (copy_to_user((int *) arg, &online, sizeof(int))) {
				return -EFAULT;
			}
			break;

		default:
			printk(KERN_DEBUG "wm8350 bat : unsupported ioctl command 0x%x\n", cmd);
			break;
	}

	return ret;
}

static int wm8350_bat_fasync(int fd, struct file *filp, int mode)
{
	return fasync_helper(fd, filp, mode, &wm8350_bat_queue);
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
	/* hard shutdown when its voltage is under PCCMP_OFF_THR */
	wm8350_set_bits(wm8350, WM8350_POWER_CHECK_COMPARATOR, WM8350_PCCMP_ERRACT);
	
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
	wm8350_register_irq(wm8350, WM8350_IRQ_EXT_USB_FB,
			    wm8350_charger_handler, NULL);
	wm8350_unmask_irq(wm8350, WM8350_IRQ_EXT_USB_FB);
	wm8350_register_irq(wm8350, WM8350_IRQ_EXT_WALL_FB,
			    wm8350_charger_handler, NULL);
	wm8350_unmask_irq(wm8350, WM8350_IRQ_EXT_WALL_FB);
	wm8350_register_irq(wm8350, WM8350_IRQ_EXT_BAT_FB,
			    wm8350_charger_handler, NULL);
	wm8350_unmask_irq(wm8350, WM8350_IRQ_EXT_BAT_FB);
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
	wm8350_mask_irq(wm8350, WM8350_IRQ_EXT_USB_FB);
	wm8350_free_irq(wm8350, WM8350_IRQ_EXT_USB_FB);
	wm8350_mask_irq(wm8350, WM8350_IRQ_EXT_WALL_FB);
	wm8350_free_irq(wm8350, WM8350_IRQ_EXT_WALL_FB);
	wm8350_mask_irq(wm8350, WM8350_IRQ_EXT_BAT_FB);
	wm8350_free_irq(wm8350, WM8350_IRQ_EXT_BAT_FB);
}

static struct file_operations wm8350_bat_fos = {
	.owner		= THIS_MODULE,
	.ioctl		= wm8350_bat_ioctl,
	.open		= wm8350_bat_open,
	.release	= wm8350_bat_release,
	.fasync		= wm8350_bat_fasync,
};

#ifdef CONFIG_MACH_CANOPUS
static struct wm8350 *_wm8350;
extern void kernel_restart(char *cmd);

static void
_pmic_dcdc_low_power_mode(int addr, int mode, int trig)
{
	uint val;

	val = wm8350_reg_read(_wm8350, addr)
		& ~(WM8350_DCDC_HIB_MODE_MASK | WM8350_DCDC_HIB_TRIG_MASK);
	wm8350_reg_write(_wm8350, addr, val | (mode | trig));
}

static void
_pmic_dcdc2_low_power_mode(int mode, int trig)
{
	uint val;

	val = wm8350_reg_read(_wm8350, WM8350_DCDC2_CONTROL)
		& ~(WM8350_DCDC2_HIB_MODE_MASK | WM8350_DCDC2_HIB_TRIG_MASK);
	wm8350_reg_write(_wm8350, WM8350_DCDC2_CONTROL, val | (mode | trig));
}

static void
_pmic_ldo_low_power_mode(int addr, int mode, int trig)
{
	uint val;

	val = wm8350_reg_read(_wm8350, addr)
		& ~(WM8350_LDO_HIB_MODE_MASK | WM8350_LDO_HIB_TRIG_MASK);
	wm8350_reg_write(_wm8350, addr, val | (mode | trig));
}

static void
_wm8350_init_hibernation(void)
{
	uint value;

	// set hibernate behavior
	_pmic_dcdc_low_power_mode(WM8350_DCDC1_LOW_POWER,
			WM8350_DCDC_HIB_MODE_DISABLE, WM8350_DCDC_HIB_TRIG_REG);
	_pmic_dcdc2_low_power_mode(WM8350_DCDC2_HIB_MODE_DISABLE, WM8350_DCDC2_HIB_TRIG_REG);
	_pmic_dcdc_low_power_mode(WM8350_DCDC3_LOW_POWER,
			WM8350_DCDC_HIB_MODE_DISABLE, WM8350_DCDC_HIB_TRIG_REG);
	_pmic_dcdc_low_power_mode(WM8350_DCDC4_LOW_POWER,
			WM8350_DCDC_HIB_MODE_DISABLE, WM8350_DCDC_HIB_TRIG_REG);

	_pmic_ldo_low_power_mode(WM8350_LDO1_LOW_POWER,
			WM8350_LDO_HIB_MODE_DISABLE, WM8350_LDO_HIB_TRIG_REG);
	_pmic_ldo_low_power_mode(WM8350_LDO2_LOW_POWER,
			WM8350_LDO_HIB_MODE_DISABLE, WM8350_LDO_HIB_TRIG_REG);
	_pmic_ldo_low_power_mode(WM8350_LDO3_LOW_POWER,
			WM8350_LDO_HIB_MODE_DISABLE, WM8350_LDO_HIB_TRIG_REG);
	_pmic_ldo_low_power_mode(WM8350_LDO4_LOW_POWER,
			WM8350_LDO_HIB_MODE_DISABLE, WM8350_LDO_HIB_TRIG_REG);

	// set dcdc4 timeout to 5
	value = wm8350_reg_read(_wm8350, WM8350_DCDC4_TIMEOUTS) & ~(0xf << 10);
	wm8350_reg_write(_wm8350, WM8350_DCDC4_TIMEOUTS, value | (5 << 10));
}

void wm8350_power_off(void)
{
	if (!_wm8350) return ;

	if (wm8350_get_supplies(_wm8350) & WM8350_LINE_SUPPLY) {
		kernel_restart(NULL);
	} else {
		wm8350_gpio_config(_wm8350, 2, WM8350_GPIO_DIR_IN,
				   WM8350_GPIO2_32KHZ_IN, WM8350_GPIO_ACTIVE_LOW,
				   WM8350_GPIO_PULL_NONE, WM8350_GPIO_INVERT_OFF,
				   WM8350_GPIO_DEBOUNCE_OFF);

		u16 val = wm8350_reg_read(_wm8350, 0x5);
		val |= (1<<15);
		wm8350_reg_write(_wm8350, 0x5, val);
	}
}

void
q_wm8350_notify_pir_event(void)
{
	wm8350_bat_user_notify_callback((void *)WM8350_EVENT_PIR);
}
#endif

static int __init wm8350_power_probe(struct platform_device *pdev)
{
	int ret;
	struct wm8350 *wm8350 = platform_get_drvdata(pdev);
	struct wm8350_power *power = &wm8350->power;
	struct wm8350_charger_policy *policy = power->policy;
	struct power_supply *usb = &power->usb;
	struct power_supply *battery = &power->battery;
	struct power_supply *ac = &power->ac;

	struct class_device *wm8350_device;

	printk(KERN_DEBUG "wm8350: power driver %s\n", WM8350_POWER_VERSION);

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

	wm8350_bat = platform_get_drvdata(pdev);
	wm8350_bat_major = register_chrdev(0, DEV_NAME, &wm8350_bat_fos);
	if (wm8350_bat_major < 0) {
		printk(KERN_DEBUG "unable to get a major for wm8350_bat\n");
		ret = wm8350_bat_major;
		goto usb_failed;
	}

	wm8350_dev_class = class_create(THIS_MODULE, DEV_NAME);
	if (IS_ERR(wm8350_dev_class)) {
		printk(KERN_DEBUG "error creating wm8350 dev class\n");
		ret = -1;
		goto class_failed;
	}

	wm8350_device = 
		class_device_create(wm8350_dev_class, NULL, MKDEV(wm8350_bat_major, 0),
				NULL, DEV_NAME);
	if (IS_ERR(wm8350_device)) {
		printk(KERN_DEBUG "error creating wm8350 class device\n");
		ret = -1;
		goto dev_failed;
	}

	wm8350_event.buf = kmalloc(CIRC_BUF_MAX * sizeof(char), GFP_KERNEL);
	if (NULL == wm8350_event.buf) {
		ret = -ENOMEM;
		goto buf_failed;
	}

	wm8350_bat_event_list_init();
	wm8350_event.head = wm8350_event.tail = 0;

	ret = device_create_file(&pdev->dev, &dev_attr_supply_state);
	if (ret < 0)
		printk(KERN_DEBUG "wm8350: failed to add supply sysfs\n");
	ret = device_create_file(&pdev->dev, &dev_attr_battery_state);
	if (ret < 0)
		printk(KERN_DEBUG "wm8350: failed to add battery sysfs\n");
	ret = device_create_file(&pdev->dev, &dev_attr_charger_state);
	if (ret < 0)
		printk(KERN_DEBUG "wm8350: failed to add charge sysfs\n");
	
	ret = device_create_file(&pdev->dev, &dev_attr_green_led);
	if (ret <0)
		printk(KERN_DEBUG "wm8350: failed to add green led sysfs\n");

	ret = device_create_file(&pdev->dev, &dev_attr_red_led);
	if (ret <0)
		printk(KERN_DEBUG "wm8350: failed to add red led sysfs\n");

	ret = device_create_file(&pdev->dev, &dev_attr_aux2_adc);
	if (ret <0)
		printk(KERN_DEBUG "wm8350: failed to add aux2 adc sysfs\n");
	ret = device_create_file(&pdev->dev, &dev_attr_aux3_adc);
	if (ret <0)
		printk(KERN_DEBUG "wm8350: failed to add aux3 adc sysfs\n");

	ret = 0;

#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_init(&_bat_wake_lock, WAKE_LOCK_SUSPEND, "wm8350-power");
	wake_lock_init(&_fault_led_wake_lock, WAKE_LOCK_SUSPEND, "fault-led");
#endif

	if (policy == NULL)
		printk(KERN_DEBUG "%s: no charger policy - "
			"charging disabled\n", __func__);
	else {
		wm8350_init_charger(wm8350);
		power->charger_enabled =
			wm8350_charger_config(wm8350, policy);
			wm8350_charger_enable(wm8350, 0);
			wm8350_charger_enable(wm8350, 1);
		if (power->charger_enabled < 0) {
			printk(KERN_DEBUG "%s: failed to enable charger\n",
				__func__);
			free_charger_irq(wm8350);
		}
	}

#ifdef CONFIG_MACH_CANOPUS
	_wm8350 = wm8350;
	pm_power_off = wm8350_power_off;

	_wm8350_init_hibernation();
#endif
	schedule_delayed_work(&_ac_detect, msecs_to_jiffies(10));
	
	return ret;

buf_failed:
	class_device_destroy(wm8350_dev_class, MKDEV(wm8350_bat_major, 0));
dev_failed:
	class_destroy(wm8350_dev_class);
class_failed:
	unregister_chrdev(wm8350_bat_major, DEV_NAME);
usb_failed:
	power_supply_unregister(battery);
battery_failed:
	power_supply_unregister(ac);

	return ret;
}

static int wm8350_power_remove(struct platform_device *pdev)
{
	struct wm8350 *wm8350 = platform_get_drvdata(pdev);
	struct wm8350_power *power = &wm8350->power;
	
#ifdef CONFIG_MACH_CANOPUS
	_wm8350 = NULL;
#endif
	if (power->charger_enabled >= 0) {
		wm8350_charger_enable(wm8350, 0);
		free_charger_irq(wm8350);
	}
	device_remove_file(&pdev->dev, &dev_attr_supply_state);
	device_remove_file(&pdev->dev, &dev_attr_battery_state);
	device_remove_file(&pdev->dev, &dev_attr_charger_state);
	power_supply_unregister(&power->battery);
	power_supply_unregister(&power->ac);
	power_supply_unregister(&power->usb);

	wm8350_bat = NULL;
	class_device_destroy(wm8350_dev_class, MKDEV(wm8350_bat_major, 0));
	class_destroy(wm8350_dev_class);
	unregister_chrdev(wm8350_bat_major, DEV_NAME);

	cancel_delayed_work(&_bat_fault_led);

#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_destroy(&_bat_wake_lock);
	wake_lock_destroy(&_fault_led_wake_lock);
#endif

	return 0;
}

struct platform_driver wm8350_power_driver = {
	.probe = wm8350_power_probe,
	.remove = wm8350_power_remove,
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
