/*
 * wm8350_pmu.c  --  Power Management Driver for Wolfson WM8350 PMIC
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
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/mfd/wm8350/core.h>
#include <linux/mfd/wm8350/pmic.h>
#include <linux/regulator/driver.h>

#define WM8350_REG_VERSION	"0.7"

/* debug */
#define WM8350_DEBUG 0
#if WM8350_DEBUG
#define dbg(format, arg...) printk(KERN_DEBUG format, ## arg)
#else
#define dbg(format, arg...)
#endif

/* hundredths of uA, 405 = 4.05 uA */
static const int isink_cur[] = {
	405, 482, 573, 681, 810, 963, 1146, 1362, 1620, 1927, 2291, 2725,
	3240, 3853, 4582, 5449, 6480, 7706, 9164, 10898, 12960, 15412, 18328,
	21796, 25920, 30824, 36656, 43592, 51840, 61648, 73313, 87184,
	103680, 123297, 146626, 174368, 207360, 246594, 293251, 348737,
	414720, 493188, 586503, 697473, 829440, 986376, 1173005, 1394946,
	1658880, 1972752, 2346011, 2789892, 3317760, 3945504, 4692021,
	5579785, 6635520, 7891008, 9384042, 11159570, 13271040, 15782015,
	18768085, 22319140
};

static int get_isink_val(int min_uA, int max_uA)
{
	int i, min_huA = min_uA * 100, max_huA = max_uA * 100;

	for (i = ARRAY_SIZE(isink_cur) - 1; i >= 0; i--) {
		if (min_huA <= isink_cur[i] && max_huA >= isink_cur[i])
			return i;
	}
	return 0;
}

static inline int wm8350_ldo_val_to_mvolts(unsigned int val)
{
	if (val < 16)
		return (val * 50) + 900;
	else
		return ((val - 16) * 100) + 1800;

}

static inline unsigned int wm8350_ldo_mvolts_to_val(int mV)
{
	if (mV < 1800)
		return (mV - 900) / 50;
	else
		return ((mV - 1800) / 100) + 16;
}

static inline int wm8350_dcdc_val_to_mvolts(unsigned int val)
{
	return (val * 25) + 850;
}

static inline unsigned int wm8350_dcdc_mvolts_to_val(int mV)
{
	return (mV - 850) / 25;
}

static int wm8350_isink_set_current(struct regulator_dev *rdev, int min_uA,
	int max_uA)
{
	struct wm8350 *wm8350 = rdev_get_drvdata(rdev);
	int isink = rdev_get_id(rdev);
	u16 val;

	switch (isink) {
	case WM8350_ISINK_A:
		val = wm8350_reg_read(wm8350, WM8350_CURRENT_SINK_DRIVER_A) &
		    ~WM8350_CS1_ISEL_MASK;
		wm8350_reg_write(wm8350, WM8350_CURRENT_SINK_DRIVER_A, val |
				 get_isink_val(min_uA, max_uA));
		break;
	case WM8350_ISINK_B:
		val = wm8350_reg_read(wm8350, WM8350_CURRENT_SINK_DRIVER_B) &
		    ~WM8350_CS1_ISEL_MASK;
		wm8350_reg_write(wm8350, WM8350_CURRENT_SINK_DRIVER_B, val |
				 get_isink_val(min_uA, max_uA));
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int wm8350_isink_get_current(struct regulator_dev *rdev)
{
	struct wm8350 *wm8350 = rdev_get_drvdata(rdev);
	int isink = rdev_get_id(rdev);
	u16 val;

	switch (isink) {
	case WM8350_ISINK_A:
		val = wm8350_reg_read(wm8350, WM8350_CURRENT_SINK_DRIVER_A) &
		    WM8350_CS1_ISEL_MASK;
		break;
	case WM8350_ISINK_B:
		val = wm8350_reg_read(wm8350, WM8350_CURRENT_SINK_DRIVER_B) &
		    WM8350_CS1_ISEL_MASK;
		break;
	default:
		return 0;
	}

	return val * 1000;
}

/* turn on ISINK followed by DCDC */
static int wm8350_isink_enable(struct regulator_dev *rdev)
{
	struct wm8350 *wm8350 = rdev_get_drvdata(rdev);
	int isink = rdev_get_id(rdev);

	/* TODO: add ISINK's C,D,E */
	switch (isink) {
	case WM8350_ISINK_A:
		switch (wm8350->pmic.isink_A_dcdc) {
		case WM8350_DCDC_2:
		case WM8350_DCDC_5:
			wm8350_set_bits(wm8350, WM8350_POWER_MGMT_7,
					WM8350_CS1_ENA);
			wm8350_set_bits(wm8350, WM8350_CSA_FLASH_CONTROL,
					WM8350_CS1_DRIVE);
			wm8350_set_bits(wm8350, WM8350_DCDC_LDO_REQUESTED,
					1 << (wm8350->pmic.isink_A_dcdc -
					      WM8350_DCDC_1));
			break;
		default:
			return -EINVAL;
		}
		break;
	case WM8350_ISINK_B:
		switch (wm8350->pmic.isink_B_dcdc) {
		case WM8350_DCDC_2:
		case WM8350_DCDC_5:
			wm8350_set_bits(wm8350, WM8350_POWER_MGMT_7,
					WM8350_CS2_ENA);
			wm8350_set_bits(wm8350, WM8350_CSB_FLASH_CONTROL,
					WM8350_CS2_DRIVE);
			wm8350_set_bits(wm8350, WM8350_DCDC_LDO_REQUESTED,
					1 << (wm8350->pmic.isink_B_dcdc -
					      WM8350_DCDC_1));
			break;
		default:
			return -EINVAL;
		}
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int wm8350_isink_disable(struct regulator_dev *rdev)
{
	struct wm8350 *wm8350 = rdev_get_drvdata(rdev);
	int isink = rdev_get_id(rdev);

	/* TODO: add ISINK's C,D,E */
	switch (isink) {
	case WM8350_ISINK_A:
		switch (wm8350->pmic.isink_A_dcdc) {
		case WM8350_DCDC_2:
		case WM8350_DCDC_5:
			wm8350_clear_bits(wm8350, WM8350_DCDC_LDO_REQUESTED,
					  1 << (wm8350->pmic.isink_A_dcdc -
						WM8350_DCDC_1));
			wm8350_clear_bits(wm8350, WM8350_POWER_MGMT_7,
					  WM8350_CS1_ENA);
			break;
		default:
			return -EINVAL;
		}
		break;
	case WM8350_ISINK_B:
		switch (wm8350->pmic.isink_B_dcdc) {
		case WM8350_DCDC_2:
		case WM8350_DCDC_5:
			wm8350_clear_bits(wm8350, WM8350_DCDC_LDO_REQUESTED,
					  1 << (wm8350->pmic.isink_B_dcdc -
						WM8350_DCDC_1));
			wm8350_clear_bits(wm8350, WM8350_POWER_MGMT_7,
					  WM8350_CS2_ENA);
			break;
		default:
			return -EINVAL;
		}
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int wm8350_isink_is_enabled(struct regulator_dev *rdev)
{
	struct wm8350 *wm8350 = rdev_get_drvdata(rdev);
	int isink = rdev_get_id(rdev);

	switch (isink) {
	case WM8350_ISINK_A:
		return wm8350_reg_read(wm8350, WM8350_CURRENT_SINK_DRIVER_A) &
		    0x8000;
	case WM8350_ISINK_B:
		return wm8350_reg_read(wm8350, WM8350_CURRENT_SINK_DRIVER_B) &
		    0x8000;
	}
	return -EINVAL;
}

int wm8350_isink_set_flash(struct wm8350 *wm8350, int isink, u16 mode,
			   u16 trigger, u16 duration, u16 on_ramp, u16 off_ramp,
			   u16 drive)
{
	switch (isink) {
	case WM8350_ISINK_A:
		wm8350_reg_write(wm8350, WM8350_CSA_FLASH_CONTROL,
				 (mode ? WM8350_CS1_FLASH_MODE : 0) |
				 (trigger ? WM8350_CS1_TRIGSRC : 0) |
				 duration | on_ramp | off_ramp | drive);
		break;
	case WM8350_ISINK_B:
		wm8350_reg_write(wm8350, WM8350_CSB_FLASH_CONTROL,
				 (mode ? WM8350_CS2_FLASH_MODE : 0) |
				 (trigger ? WM8350_CS2_TRIGSRC : 0) |
				 duration | on_ramp | off_ramp | drive);
		break;
	default:
		return -EINVAL;
	}
	return 0;
}
EXPORT_SYMBOL_GPL(wm8350_isink_set_flash);

static int wm8350_dcdc_set_voltage(struct regulator_dev *rdev, int min_uV,
	int max_uV)
{
	struct wm8350 *wm8350 = rdev_get_drvdata(rdev);
	int volt_reg, dcdc = rdev_get_id(rdev), mV,
		min_mV = min_uV / 1000, max_mV = max_uV / 1000;
	u16 val;

	if (min_mV < 850 || min_mV > 4025)
		return -EINVAL;
	if (max_mV < 850 || max_mV > 4025)
		return -EINVAL;

	/* step size is 25mV */
	mV = (min_mV - 826) / 25;
	if (wm8350_dcdc_val_to_mvolts(mV) > max_mV)
		return -EINVAL;
	BUG_ON(wm8350_dcdc_val_to_mvolts(mV) < min_mV);

	switch (dcdc) {
	case WM8350_DCDC_1:
		volt_reg = WM8350_DCDC1_CONTROL;
		break;
	case WM8350_DCDC_3:
		volt_reg = WM8350_DCDC3_CONTROL;
		break;
	case WM8350_DCDC_4:
		volt_reg = WM8350_DCDC4_CONTROL;
		break;
	case WM8350_DCDC_6:
		volt_reg = WM8350_DCDC6_CONTROL;
		break;
	case WM8350_DCDC_2:
	case WM8350_DCDC_5:
	default:
		return -EINVAL;
	}

	/* all DCDC's have same mV bits */
	val = wm8350_reg_read(wm8350, volt_reg) & ~WM8350_DC1_VSEL_MASK;
	wm8350_reg_write(wm8350, volt_reg, val | mV);
	return 0;
}

static int wm8350_dcdc_get_voltage(struct regulator_dev *rdev)
{
	struct wm8350 *wm8350 = rdev_get_drvdata(rdev);
	int volt_reg, dcdc = rdev_get_id(rdev);
	u16 val;

	switch (dcdc) {
	case WM8350_DCDC_1:
		volt_reg = WM8350_DCDC1_CONTROL;
		break;
	case WM8350_DCDC_3:
		volt_reg = WM8350_DCDC3_CONTROL;
		break;
	case WM8350_DCDC_4:
		volt_reg = WM8350_DCDC4_CONTROL;
		break;
	case WM8350_DCDC_6:
		volt_reg = WM8350_DCDC6_CONTROL;
		break;
	case WM8350_DCDC_2:
	case WM8350_DCDC_5:
	default:
		return -EINVAL;
	}

	/* all DCDC's have same mV bits */
	val = wm8350_reg_read(wm8350, volt_reg) & WM8350_DC1_VSEL_MASK;
	return wm8350_dcdc_val_to_mvolts(val) * 1000;
}

static int wm8350_dcdc_set_suspend_voltage(struct regulator_dev *rdev, int uV)
{
	struct wm8350 *wm8350 = rdev_get_drvdata(rdev);
	int volt_reg, mV = uV / 1000, dcdc = rdev_get_id(rdev);
	u16 val;

	dbg("%s %d mV %d\n", __FUNCTION__, dcdc, mV);

	if (mV && (mV < 850 || mV > 4025)) {
		printk(KERN_ERR
		       "wm8350: dcdc %d image voltage %d mV out of range\n",
		       dcdc, mV);
		return -EINVAL;
	}
	if (mV == 0)
		mV = 850;

	switch (dcdc) {
	case WM8350_DCDC_1:
		volt_reg = WM8350_DCDC1_LOW_POWER;
		break;
	case WM8350_DCDC_3:
		volt_reg = WM8350_DCDC3_LOW_POWER;
		break;
	case WM8350_DCDC_4:
		volt_reg = WM8350_DCDC4_LOW_POWER;
		break;
	case WM8350_DCDC_6:
		volt_reg = WM8350_DCDC6_LOW_POWER;
		break;
	case WM8350_DCDC_2:
	case WM8350_DCDC_5:
	default:
		return -EINVAL;
	}

	/* all DCDC's have same mV bits */
	val = wm8350_reg_read(wm8350, volt_reg) & ~WM8350_DC1_VSEL_MASK;
	wm8350_reg_write(wm8350, volt_reg,
			 val | wm8350_dcdc_mvolts_to_val(mV));
	return 0;
}

static int wm8350_dcdc_set_suspend_enable(struct regulator_dev *rdev)
{
	struct wm8350 *wm8350 = rdev_get_drvdata(rdev);
	int dcdc = rdev_get_id(rdev);
	u16 val;

	switch (dcdc) {
	case WM8350_DCDC_1:
		val = wm8350_reg_read(wm8350, WM8350_DCDC1_LOW_POWER)
			& ~WM8350_DCDC_HIB_MODE_MASK;
		wm8350_reg_write(wm8350, WM8350_DCDC1_LOW_POWER,
			wm8350->pmic.dcdc1_hib_mode);
		break;
	case WM8350_DCDC_3:
		val = wm8350_reg_read(wm8350, WM8350_DCDC3_LOW_POWER)
			& ~WM8350_DCDC_HIB_MODE_MASK;
		wm8350_reg_write(wm8350, WM8350_DCDC3_LOW_POWER,
			wm8350->pmic.dcdc3_hib_mode);
		break;
	case WM8350_DCDC_4:
		val = wm8350_reg_read(wm8350, WM8350_DCDC4_LOW_POWER)
			& ~WM8350_DCDC_HIB_MODE_MASK;
		wm8350_reg_write(wm8350, WM8350_DCDC4_LOW_POWER,
			wm8350->pmic.dcdc4_hib_mode);
		break;
	case WM8350_DCDC_6:
		val = wm8350_reg_read(wm8350, WM8350_DCDC6_LOW_POWER)
			& ~WM8350_DCDC_HIB_MODE_MASK;
		wm8350_reg_write(wm8350, WM8350_DCDC6_LOW_POWER,
			wm8350->pmic.dcdc6_hib_mode);
		break;
	case WM8350_DCDC_2:
	case WM8350_DCDC_5:
	default:
		return -EINVAL;
	}

	return 0;
}

static int wm8350_dcdc_set_suspend_disable(struct regulator_dev *rdev)
{
	struct wm8350 *wm8350 = rdev_get_drvdata(rdev);
	int dcdc = rdev_get_id(rdev);
	u16 val;

	switch (dcdc) {
	case WM8350_DCDC_1:
		val = wm8350_reg_read(wm8350, WM8350_DCDC1_LOW_POWER);
		wm8350->pmic.dcdc1_hib_mode = val & WM8350_DCDC_HIB_MODE_MASK;
		wm8350_reg_write(wm8350, WM8350_DCDC1_LOW_POWER,
			WM8350_DCDC_HIB_MODE_DIS);
		break;
	case WM8350_DCDC_3:
		val = wm8350_reg_read(wm8350, WM8350_DCDC3_LOW_POWER);
		wm8350->pmic.dcdc3_hib_mode = val & WM8350_DCDC_HIB_MODE_MASK;
		wm8350_reg_write(wm8350, WM8350_DCDC3_LOW_POWER,
			WM8350_DCDC_HIB_MODE_DIS);
		break;
	case WM8350_DCDC_4:
		val = wm8350_reg_read(wm8350, WM8350_DCDC4_LOW_POWER);
		wm8350->pmic.dcdc4_hib_mode = val & WM8350_DCDC_HIB_MODE_MASK;
		wm8350_reg_write(wm8350, WM8350_DCDC4_LOW_POWER,
			WM8350_DCDC_HIB_MODE_DIS);
		break;
	case WM8350_DCDC_6:
		val = wm8350_reg_read(wm8350, WM8350_DCDC6_LOW_POWER);
		wm8350->pmic.dcdc6_hib_mode = val & WM8350_DCDC_HIB_MODE_MASK;
		wm8350_reg_write(wm8350, WM8350_DCDC6_LOW_POWER,
			WM8350_DCDC_HIB_MODE_DIS);
		break;
	case WM8350_DCDC_2:
	case WM8350_DCDC_5:
	default:
		return -EINVAL;
	}

	return 0;
}

static int wm8350_dcdc25_set_suspend_enable(struct regulator_dev *rdev)
{
	struct wm8350 *wm8350 = rdev_get_drvdata(rdev);
	int dcdc = rdev_get_id(rdev);
	u16 val;

	switch (dcdc) {
	case WM8350_DCDC_2:
		val = wm8350_reg_read(wm8350, WM8350_DCDC2_CONTROL)
		    & ~WM8350_DC2_HIB_MODE_MASK;
		wm8350_reg_write(wm8350, WM8350_DCDC2_CONTROL, val |
				 WM8350_DC2_HIB_MODE_ACTIVE);
		break;
	case WM8350_DCDC_5:
		val = wm8350_reg_read(wm8350, WM8350_DCDC5_CONTROL)
		    & ~WM8350_DC2_HIB_MODE_MASK;
		wm8350_reg_write(wm8350, WM8350_DCDC5_CONTROL, val |
				 WM8350_DC5_HIB_MODE_ACTIVE);
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int wm8350_dcdc25_set_suspend_disable(struct regulator_dev *rdev)
{
	struct wm8350 *wm8350 = rdev_get_drvdata(rdev);
	int dcdc = rdev_get_id(rdev);
	u16 val;

	switch (dcdc) {
	case WM8350_DCDC_2:
		val = wm8350_reg_read(wm8350, WM8350_DCDC2_CONTROL)
		    & ~WM8350_DC2_HIB_MODE_MASK;
		wm8350_reg_write(wm8350, WM8350_DCDC2_CONTROL, val |
				 WM8350_DC2_HIB_MODE_DISABLE);
		break;
	case WM8350_DCDC_5:
		val = wm8350_reg_read(wm8350, WM8350_DCDC5_CONTROL)
		    & ~WM8350_DC2_HIB_MODE_MASK;
		wm8350_reg_write(wm8350, WM8350_DCDC5_CONTROL, val |
				 WM8350_DC2_HIB_MODE_DISABLE);
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int wm8350_dcdc_set_suspend_mode(struct regulator_dev *rdev,
	unsigned int mode)
{
	struct wm8350 *wm8350 = rdev_get_drvdata(rdev);
	int dcdc = rdev_get_id(rdev);
	u16 *hib_mode;

	switch (dcdc) {
	case WM8350_DCDC_1:
		hib_mode = &wm8350->pmic.dcdc1_hib_mode;
		break;
	case WM8350_DCDC_3:
		hib_mode = &wm8350->pmic.dcdc3_hib_mode;
		break;
	case WM8350_DCDC_4:
		hib_mode = &wm8350->pmic.dcdc4_hib_mode;
		break;
	case WM8350_DCDC_6:
		hib_mode = &wm8350->pmic.dcdc6_hib_mode;
		break;
	case WM8350_DCDC_2:
	case WM8350_DCDC_5:
	default:
		return -EINVAL;
	}

	switch (mode) {
	case REGULATOR_MODE_NORMAL:
		*hib_mode = WM8350_DCDC_HIB_MODE_IMAGE;
		break;
	case REGULATOR_MODE_IDLE:
		*hib_mode = WM8350_DCDC_HIB_MODE_STANDBY;
		break;
	case REGULATOR_MODE_STANDBY:
		*hib_mode = WM8350_DCDC_HIB_MODE_LDO_IM;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int wm8350_ldo_set_suspend_voltage(struct regulator_dev *rdev, int uV)
{
	struct wm8350 *wm8350 = rdev_get_drvdata(rdev);
	int volt_reg, mV = uV / 1000, ldo = rdev_get_id(rdev);
	u16 val;

	dbg("%s %d mV %d\n", __FUNCTION__, ldo, mV);

	if (mV < 900 || mV > 3300) {
		printk(KERN_ERR "wm8350: ldo %d voltage %d mV out of range\n",
		       ldo, mV);
		return -EINVAL;
	}

	switch (ldo) {
	case WM8350_LDO_1:
		volt_reg = WM8350_LDO1_LOW_POWER;
		break;
	case WM8350_LDO_2:
		volt_reg = WM8350_LDO2_LOW_POWER;
		break;
	case WM8350_LDO_3:
		volt_reg = WM8350_LDO3_LOW_POWER;
		break;
	case WM8350_LDO_4:
		volt_reg = WM8350_LDO4_LOW_POWER;
		break;
	default:
		return -EINVAL;
	}

	/* all LDO's have same mV bits */
	val = wm8350_reg_read(wm8350, volt_reg) & ~WM8350_LDO1_VSEL_MASK;
	wm8350_reg_write(wm8350, volt_reg,
			 val | wm8350_ldo_mvolts_to_val(mV));
	return 0;
}

static int wm8350_ldo_set_suspend_enable(struct regulator_dev *rdev)
{
	struct wm8350 *wm8350 = rdev_get_drvdata(rdev);
	int volt_reg, ldo = rdev_get_id(rdev);
	u16 val;

	switch (ldo) {
	case WM8350_LDO_1:
		volt_reg = WM8350_LDO1_LOW_POWER;
		break;
	case WM8350_LDO_2:
		volt_reg = WM8350_LDO2_LOW_POWER;
		break;
	case WM8350_LDO_3:
		volt_reg = WM8350_LDO3_LOW_POWER;
		break;
	case WM8350_LDO_4:
		volt_reg = WM8350_LDO4_LOW_POWER;
		break;
	default:
		return -EINVAL;
	}

	/* all LDO's have same mV bits */
	val = wm8350_reg_read(wm8350, volt_reg) & ~WM8350_LDO1_HIB_MODE_MASK;
	wm8350_reg_write(wm8350, volt_reg, val);
	return 0;
}

static int wm8350_ldo_set_suspend_disable(struct regulator_dev *rdev)
{
	struct wm8350 *wm8350 = rdev_get_drvdata(rdev);
	int volt_reg, ldo = rdev_get_id(rdev);
	u16 val;

	switch (ldo) {
	case WM8350_LDO_1:
		volt_reg = WM8350_LDO1_LOW_POWER;
		break;
	case WM8350_LDO_2:
		volt_reg = WM8350_LDO2_LOW_POWER;
		break;
	case WM8350_LDO_3:
		volt_reg = WM8350_LDO3_LOW_POWER;
		break;
	case WM8350_LDO_4:
		volt_reg = WM8350_LDO4_LOW_POWER;
		break;
	default:
		return -EINVAL;
	}

	/* all LDO's have same mV bits */
	val = wm8350_reg_read(wm8350, volt_reg) & ~WM8350_LDO1_HIB_MODE_MASK;
	wm8350_reg_write(wm8350, volt_reg, WM8350_LDO1_HIB_MODE_DIS);
	return 0;
}

static int wm8350_ldo_set_voltage(struct regulator_dev *rdev, int min_uV,
	int max_uV)
{
	struct wm8350 *wm8350 = rdev_get_drvdata(rdev);
	int volt_reg, ldo = rdev_get_id(rdev), mV, min_mV = min_uV / 1000,
		max_mV = max_uV / 1000;
	u16 val;

	if (min_mV < 900 || min_mV > 3300)
		return -EINVAL;
	if (max_mV < 900 || max_mV > 3300)
		return -EINVAL;

	if (min_mV < 1800) {
		/* step size is 50mV < 1800mV */
		mV = (min_mV - 851) / 50;
		if (wm8350_ldo_val_to_mvolts(mV) > max_mV)
			return -EINVAL;
		BUG_ON(wm8350_ldo_val_to_mvolts(mV) < min_mV);
	} else {
		/* step size is 100mV > 1800mV */
		mV = ((min_mV - 1701) / 100) + 16;
		if (wm8350_ldo_val_to_mvolts(mV) > max_mV)
			return -EINVAL;
		BUG_ON(wm8350_ldo_val_to_mvolts(mV) < min_mV);
	}

	switch (ldo) {
	case WM8350_LDO_1:
		volt_reg = WM8350_LDO1_CONTROL;
		break;
	case WM8350_LDO_2:
		volt_reg = WM8350_LDO2_CONTROL;
		break;
	case WM8350_LDO_3:
		volt_reg = WM8350_LDO3_CONTROL;
		break;
	case WM8350_LDO_4:
		volt_reg = WM8350_LDO4_CONTROL;
		break;
	default:
		return -EINVAL;
	}

	/* all LDO's have same mV bits */
	val = wm8350_reg_read(wm8350, volt_reg) & ~WM8350_LDO1_VSEL_MASK;
	wm8350_reg_write(wm8350, volt_reg, val | mV);
	return 0;
}

static int wm8350_ldo_get_voltage(struct regulator_dev *rdev)
{
	struct wm8350 *wm8350 = rdev_get_drvdata(rdev);
	int volt_reg, ldo = rdev_get_id(rdev);
	u16 val;

	switch (ldo) {
	case WM8350_LDO_1:
		volt_reg = WM8350_LDO1_CONTROL;
		break;
	case WM8350_LDO_2:
		volt_reg = WM8350_LDO2_CONTROL;
		break;
	case WM8350_LDO_3:
		volt_reg = WM8350_LDO3_CONTROL;
		break;
	case WM8350_LDO_4:
		volt_reg = WM8350_LDO4_CONTROL;
		break;
	default:
		return -EINVAL;
	}

	/* all LDO's have same mV bits */
	val = wm8350_reg_read(wm8350, volt_reg) & WM8350_LDO1_VSEL_MASK;
	return wm8350_ldo_val_to_mvolts(val) * 1000;
}

int wm8350_dcdc_set_slot(struct wm8350 *wm8350, int dcdc, u16 start,
			 u16 stop, u16 fault)
{
	int slot_reg;
	u16 val;

	dbg("%s %d start %d stop %d\n", __FUNCTION__, dcdc, start, stop);

	/* slot valid ? */
	if (start > 15 || stop > 15)
		return -EINVAL;

	switch (dcdc) {
	case WM8350_DCDC_1:
		slot_reg = WM8350_DCDC1_TIMEOUTS;
		break;
	case WM8350_DCDC_2:
		slot_reg = WM8350_DCDC2_TIMEOUTS;
		break;
	case WM8350_DCDC_3:
		slot_reg = WM8350_DCDC3_TIMEOUTS;
		break;
	case WM8350_DCDC_4:
		slot_reg = WM8350_DCDC4_TIMEOUTS;
		break;
	case WM8350_DCDC_5:
		slot_reg = WM8350_DCDC5_TIMEOUTS;
		break;
	case WM8350_DCDC_6:
		slot_reg = WM8350_DCDC6_TIMEOUTS;
		break;
	default:
		return -EINVAL;
	}

	val = wm8350_reg_read(wm8350, slot_reg) &
	    ~(WM8350_DC1_ENSLOT_MASK | WM8350_DC1_SDSLOT_MASK |
	      WM8350_DC1_ERRACT_MASK);
	wm8350_reg_write(wm8350, slot_reg,
			 val | (start << WM8350_DC1_ENSLOT_SHIFT) |
			 (stop << WM8350_DC1_SDSLOT_SHIFT) |
			 (fault << WM8350_DC1_ERRACT_SHIFT));

	return 0;
}
EXPORT_SYMBOL_GPL(wm8350_dcdc_set_slot);

int wm8350_ldo_set_slot(struct wm8350 *wm8350, int ldo, u16 start, u16 stop)
{
	int slot_reg;
	u16 val;

	dbg("%s %d start %d stop %d\n", __FUNCTION__, ldo, start, stop);

	/* slot valid ? */
	if (start > 15 || stop > 15)
		return -EINVAL;

	switch (ldo) {
	case WM8350_LDO_1:
		slot_reg = WM8350_LDO1_TIMEOUTS;
		break;
	case WM8350_LDO_2:
		slot_reg = WM8350_LDO2_TIMEOUTS;
		break;
	case WM8350_LDO_3:
		slot_reg = WM8350_LDO3_TIMEOUTS;
		break;
	case WM8350_LDO_4:
		slot_reg = WM8350_LDO4_TIMEOUTS;
		break;
	default:
		return -EINVAL;
	}

	val = wm8350_reg_read(wm8350, slot_reg) & ~WM8350_LDO1_SDSLOT_MASK;
	wm8350_reg_write(wm8350, slot_reg, val | ((start << 10) | (stop << 6)));
	return 0;
}
EXPORT_SYMBOL_GPL(wm8350_ldo_set_slot);

int wm8350_dcdc25_set_mode(struct wm8350 *wm8350, int dcdc, u16 mode,
			   u16 ilim, u16 ramp, u16 feedback)
{
	u16 val;

	dbg("%s %d mode: %s %s\n", __FUNCTION__, dcdc,
	    mode ? "normal" : "boost", ilim ? "low" : "normal");

	switch (dcdc) {
	case WM8350_DCDC_2:
		val = wm8350_reg_read(wm8350, WM8350_DCDC2_CONTROL)
		    & ~(WM8350_DC2_MODE_MASK | WM8350_DC2_ILIM_MASK |
			WM8350_DC2_RMP_MASK | WM8350_DC2_FBSRC_MASK);
		wm8350_reg_write(wm8350, WM8350_DCDC2_CONTROL, val |
				 (mode << WM8350_DC2_MODE_SHIFT) |
				 (ilim << WM8350_DC2_ILIM_SHIFT) |
				 (ramp << WM8350_DC2_RMP_SHIFT) |
				 (feedback << WM8350_DC2_FBSRC_SHIFT));
		break;
	case WM8350_DCDC_5:
		val = wm8350_reg_read(wm8350, WM8350_DCDC5_CONTROL)
		    & ~(WM8350_DC5_MODE_MASK | WM8350_DC5_ILIM_MASK |
			WM8350_DC5_RMP_MASK | WM8350_DC5_FBSRC_MASK);
		wm8350_reg_write(wm8350, WM8350_DCDC5_CONTROL, val |
				 (mode << WM8350_DC5_MODE_SHIFT) |
				 (ilim << WM8350_DC5_ILIM_SHIFT) |
				 (ramp << WM8350_DC5_RMP_SHIFT) |
				 (feedback << WM8350_DC5_FBSRC_SHIFT));
		break;
	default:
		return -EINVAL;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(wm8350_dcdc25_set_mode);

static int wm8350_dcdc_enable(struct regulator_dev *rdev)
{
	struct wm8350 *wm8350 = rdev_get_drvdata(rdev);
	int dcdc = rdev_get_id(rdev);
	u16 shift;

	dbg("%s %d --> %s\n", __FUNCTION__, dcdc, enable ? "on" : "off");

	if (dcdc < WM8350_DCDC_1 || dcdc > WM8350_DCDC_6)
		return -EINVAL;

	shift = dcdc - WM8350_DCDC_1;
	wm8350_set_bits(wm8350, WM8350_DCDC_LDO_REQUESTED, 1 << shift);
	return 0;
}

static int wm8350_dcdc_disable(struct regulator_dev *rdev)
{
	struct wm8350 *wm8350 = rdev_get_drvdata(rdev);
	int dcdc = rdev_get_id(rdev);
	u16 shift;

	dbg("%s %d --> %s\n", __FUNCTION__, dcdc, enable ? "on" : "off");

	if (dcdc < WM8350_DCDC_1 || dcdc > WM8350_DCDC_6)
		return -EINVAL;

	shift = dcdc - WM8350_DCDC_1;
	wm8350_clear_bits(wm8350, WM8350_DCDC_LDO_REQUESTED, 1 << shift);

	return 0;
}

static int wm8350_ldo_enable(struct regulator_dev *rdev)
{
	struct wm8350 *wm8350 = rdev_get_drvdata(rdev);
	int ldo = rdev_get_id(rdev);
	u16 shift;

	dbg("%s %d --> %s\n", __FUNCTION__, ldo, enable ? "on" : "off");

	if (ldo < WM8350_LDO_1 || ldo > WM8350_LDO_4)
		return -EINVAL;

	shift = (ldo - WM8350_LDO_1) + 8;
	wm8350_set_bits(wm8350, WM8350_DCDC_LDO_REQUESTED, 1 << shift);
	return 0;
}

static int wm8350_ldo_disable(struct regulator_dev *rdev)
{
	struct wm8350 *wm8350 = rdev_get_drvdata(rdev);
	int ldo = rdev_get_id(rdev);
	u16 shift;

	dbg("%s %d --> %s\n", __FUNCTION__, ldo, enable ? "on" : "off");

	if (ldo < WM8350_LDO_1 || ldo > WM8350_LDO_4)
		return -EINVAL;

	shift = (ldo - WM8350_LDO_1) + 8;
	wm8350_clear_bits(wm8350, WM8350_DCDC_LDO_REQUESTED, 1 << shift);
	return 0;
}

static int force_continuous_enable(struct wm8350 *wm8350, int dcdc, int enable)
{
	int reg = 0, ret;

	switch (dcdc) {
	case WM8350_DCDC_1:
		reg = WM8350_DCDC1_FORCE_PWM;
		break;
	case WM8350_DCDC_3:
		reg = WM8350_DCDC3_FORCE_PWM;
		break;
	case WM8350_DCDC_4:
		reg = WM8350_DCDC4_FORCE_PWM;
		break;
	case WM8350_DCDC_6:
		reg = WM8350_DCDC6_FORCE_PWM;
		break;
	default:
		return -EINVAL;
	}

	if (enable)
		ret = wm8350_set_bits(wm8350, reg,
			WM8350_DCDC1_FORCE_PWM_ENA);
	else
		ret = wm8350_clear_bits(wm8350, reg,
			WM8350_DCDC1_FORCE_PWM_ENA);
	return ret;
}

static int wm8350_dcdc_set_mode(struct regulator_dev *rdev, unsigned int mode)
{
	struct wm8350 *wm8350 = rdev_get_drvdata(rdev);
	int dcdc = rdev_get_id(rdev);
	u16 val;

	if (dcdc < WM8350_DCDC_1 || dcdc > WM8350_DCDC_6)
		return -EINVAL;

	if (dcdc == WM8350_DCDC_2 || dcdc == WM8350_DCDC_5)
		return -EINVAL;

	val = 1 << (dcdc - WM8350_DCDC_1);

	switch (mode) {
	case REGULATOR_MODE_FAST:
		/* force continuous mode */
		wm8350_set_bits(wm8350, WM8350_DCDC_ACTIVE_OPTIONS, val);
		wm8350_clear_bits(wm8350, WM8350_DCDC_SLEEP_OPTIONS, val);
		force_continuous_enable(wm8350, dcdc, 1);
		break;
	case REGULATOR_MODE_NORMAL:
		/* active / pulse skipping */
		wm8350_set_bits(wm8350, WM8350_DCDC_ACTIVE_OPTIONS, val);
		wm8350_clear_bits(wm8350, WM8350_DCDC_SLEEP_OPTIONS, val);
		force_continuous_enable(wm8350, dcdc, 0);
		break;
	case REGULATOR_MODE_IDLE:
		/* standby mode */
		force_continuous_enable(wm8350, dcdc, 0);
		wm8350_clear_bits(wm8350, WM8350_DCDC_SLEEP_OPTIONS, val);
		wm8350_clear_bits(wm8350, WM8350_DCDC_ACTIVE_OPTIONS, val);
		break;
	case REGULATOR_MODE_STANDBY:
		/* LDO mode */
		force_continuous_enable(wm8350, dcdc, 0);
		wm8350_set_bits(wm8350, WM8350_DCDC_SLEEP_OPTIONS, val);
		break;
	}

	return 0;
}

static unsigned int wm8350_dcdc_get_mode(struct regulator_dev *rdev)
{
	struct wm8350 *wm8350 = rdev_get_drvdata(rdev);
	int dcdc = rdev_get_id(rdev);
	u16 mask, sleep, active, force;
	int mode = REGULATOR_MODE_NORMAL;

	if (dcdc < WM8350_DCDC_1 || dcdc > WM8350_DCDC_6)
		return -EINVAL;

	if (dcdc == WM8350_DCDC_2 || dcdc == WM8350_DCDC_5)
		return -EINVAL;

	mask = 1 << (dcdc - WM8350_DCDC_1);
	active = wm8350_reg_read(wm8350, WM8350_DCDC_ACTIVE_OPTIONS) & mask;
	sleep = wm8350_reg_read(wm8350, WM8350_DCDC_SLEEP_OPTIONS) & mask;
	force = wm8350_reg_read(wm8350, WM8350_DCDC1_FORCE_PWM)
	    & WM8350_DCDC1_FORCE_PWM_ENA;
	dbg("mask %x active %x sleep %x force %x", mask, active, sleep, force);

	if (active && !sleep) {
		if (force)
			mode = REGULATOR_MODE_FAST;
		else
			mode = REGULATOR_MODE_NORMAL;
	} else if (!active && !sleep)
		mode = REGULATOR_MODE_IDLE;
	else if (!sleep)
		mode = REGULATOR_MODE_STANDBY;

	return mode;
}

static unsigned int wm8350_ldo_get_mode(struct regulator_dev *rdev)
{
	return REGULATOR_MODE_NORMAL;
}

struct wm8350_dcdc_efficiency {
	int uA_load_min;
	int uA_load_max;
	unsigned int mode;
};

static const struct wm8350_dcdc_efficiency dcdc1_6_efficiency[] = {
	{0, 10000, REGULATOR_MODE_STANDBY},       /* 0 - 10mA - LDO */
	{10000, 100000, REGULATOR_MODE_IDLE},     /* 10mA - 100mA - Standby */
	{100000, 1000000, REGULATOR_MODE_NORMAL}, /* > 100mA - Active */
	{-1, -1, REGULATOR_MODE_NORMAL},
};

static const struct wm8350_dcdc_efficiency dcdc3_4_efficiency[] = {
	{0, 10000, REGULATOR_MODE_STANDBY},      /* 0 - 10mA - LDO */
	{10000, 100000, REGULATOR_MODE_IDLE},    /* 10mA - 100mA - Standby */
	{100000, 800000, REGULATOR_MODE_NORMAL}, /* > 100mA - Active */
	{-1, -1, REGULATOR_MODE_NORMAL},
};

static unsigned int get_mode(int uA, const struct wm8350_dcdc_efficiency *eff)
{
	int i = 0;

	while (eff[i].uA_load_min != -1) {
		if (uA >= eff[i].uA_load_min && uA <= eff[i].uA_load_max)
			return eff[i].mode;
	}
	return REGULATOR_MODE_NORMAL;
}

/* Query the regulator for it's most efficient mode @ uV,uA
 * WM8350 regulator efficiency is pretty similar over
 * different input and output uV.
 */
static unsigned int wm8350_dcdc_get_optimum_mode(struct regulator_dev *rdev,
						 int input_uV, int output_uV,
						 int output_uA)
{
	int dcdc = rdev_get_id(rdev), mode;

	switch (dcdc) {
	case WM8350_DCDC_1:
	case WM8350_DCDC_6:
		mode = get_mode(output_uA, dcdc1_6_efficiency);
		break;
	case WM8350_DCDC_3:
	case WM8350_DCDC_4:
		mode = get_mode(output_uA, dcdc3_4_efficiency);
		break;
	default:
		mode = REGULATOR_MODE_NORMAL;
		break;
	}
	return mode;
}

static int wm8350_dcdc_is_enabled(struct regulator_dev *rdev)
{
	struct wm8350 *wm8350 = rdev_get_drvdata(rdev);
	int dcdc = rdev_get_id(rdev), shift;

	if (dcdc < WM8350_DCDC_1 || dcdc > WM8350_DCDC_6)
		return -EINVAL;

	shift = dcdc - WM8350_DCDC_1;
	return wm8350_reg_read(wm8350, WM8350_DCDC_LDO_REQUESTED)
	    & (1 << shift);
}

static int wm8350_ldo_is_enabled(struct regulator_dev *rdev)
{
	struct wm8350 *wm8350 = rdev_get_drvdata(rdev);
	int ldo = rdev_get_id(rdev), shift;

	if (ldo < WM8350_LDO_1 || ldo > WM8350_LDO_4)
		return -EINVAL;

	shift = (ldo - WM8350_LDO_1) + 8;
	return wm8350_reg_read(wm8350, WM8350_DCDC_LDO_REQUESTED)
	    & (1 << shift);
}

static struct regulator_ops wm8350_dcdc_ops = {
	.set_voltage = wm8350_dcdc_set_voltage,
	.get_voltage = wm8350_dcdc_get_voltage,
	.enable = wm8350_dcdc_enable,
	.disable = wm8350_dcdc_disable,
	.get_mode = wm8350_dcdc_get_mode,
	.set_mode = wm8350_dcdc_set_mode,
	.get_optimum_mode = wm8350_dcdc_get_optimum_mode,
	.is_enabled = wm8350_dcdc_is_enabled,
	.set_suspend_voltage = wm8350_dcdc_set_suspend_voltage,
	.set_suspend_enable = wm8350_dcdc_set_suspend_enable,
	.set_suspend_disable = wm8350_dcdc_set_suspend_disable,
	.set_suspend_mode = wm8350_dcdc_set_suspend_mode,
};

static struct regulator_ops wm8350_dcdc2_5_ops = {
	.enable = wm8350_dcdc_enable,
	.disable = wm8350_dcdc_disable,
	.is_enabled = wm8350_dcdc_is_enabled,
	.set_suspend_enable = wm8350_dcdc25_set_suspend_enable,
	.set_suspend_disable = wm8350_dcdc25_set_suspend_disable,
};

static struct regulator_ops wm8350_ldo_ops = {
	.set_voltage = wm8350_ldo_set_voltage,
	.get_voltage = wm8350_ldo_get_voltage,
	.enable = wm8350_ldo_enable,
	.disable = wm8350_ldo_disable,
	.is_enabled = wm8350_ldo_is_enabled,
	.get_mode = wm8350_ldo_get_mode,
	.set_suspend_voltage = wm8350_ldo_set_suspend_voltage,
	.set_suspend_enable = wm8350_ldo_set_suspend_enable,
	.set_suspend_disable = wm8350_ldo_set_suspend_disable,
};

static struct regulator_ops wm8350_isink_ops = {
	.set_current_limit = wm8350_isink_set_current,
	.get_current_limit = wm8350_isink_get_current,
	.enable = wm8350_isink_enable,
	.disable = wm8350_isink_disable,
	.is_enabled = wm8350_isink_is_enabled,
};

static struct regulator_desc wm8350_reg[NUM_WM8350_REGULATORS] = {
	{
	 .name = "DCDC1",
	 .id = WM8350_DCDC_1,
	 .ops = &wm8350_dcdc_ops,
	 .irq = WM8350_IRQ_UV_DC1,
	 .type = REGULATOR_VOLTAGE,
	 .owner = THIS_MODULE,
	 },
	{
	 .name = "DCDC2",
	 .id = WM8350_DCDC_2,
	 .ops = &wm8350_dcdc2_5_ops,
	 .irq = WM8350_IRQ_UV_DC2,
	 .type = REGULATOR_VOLTAGE,
	 .owner = THIS_MODULE,
	 },
	{
	 .name = "DCDC3",
	 .id = WM8350_DCDC_3,
	 .ops = &wm8350_dcdc_ops,
	 .irq = WM8350_IRQ_UV_DC3,
	 .type = REGULATOR_VOLTAGE,
	 .owner = THIS_MODULE,
	 },
	{
	 .name = "DCDC4",
	 .id = WM8350_DCDC_4,
	 .ops = &wm8350_dcdc_ops,
	 .irq = WM8350_IRQ_UV_DC4,
	 .type = REGULATOR_VOLTAGE,
	 .owner = THIS_MODULE,
	 },
	{
	 .name = "DCDC5",
	 .id = WM8350_DCDC_5,
	 .ops = &wm8350_dcdc2_5_ops,
	 .irq = WM8350_IRQ_UV_DC5,
	 .type = REGULATOR_VOLTAGE,
	 .owner = THIS_MODULE,
	 },
	{
	 .name = "DCDC6",
	 .id = WM8350_DCDC_6,
	 .ops = &wm8350_dcdc_ops,
	 .irq = WM8350_IRQ_UV_DC6,
	 .type = REGULATOR_VOLTAGE,
	 .owner = THIS_MODULE,
	 },
	{
	 .name = "LDO1",
	 .id = WM8350_LDO_1,
	 .ops = &wm8350_ldo_ops,
	 .irq = WM8350_IRQ_UV_LDO1,
	 .type = REGULATOR_VOLTAGE,
	 .owner = THIS_MODULE,
	 },
	{
	 .name = "LDO2",
	 .id = WM8350_LDO_2,
	 .ops = &wm8350_ldo_ops,
	 .irq = WM8350_IRQ_UV_LDO2,
	 .type = REGULATOR_VOLTAGE,
	 .owner = THIS_MODULE,
	 },
	{
	 .name = "LDO3",
	 .id = WM8350_LDO_3,
	 .ops = &wm8350_ldo_ops,
	 .irq = WM8350_IRQ_UV_LDO3,
	 .type = REGULATOR_VOLTAGE,
	 .owner = THIS_MODULE,
	 },
	{
	 .name = "LDO4",
	 .id = WM8350_LDO_4,
	 .ops = &wm8350_ldo_ops,
	 .irq = WM8350_IRQ_UV_LDO4,
	 .type = REGULATOR_VOLTAGE,
	 .owner = THIS_MODULE,
	 },
	{
	 .name = "ISINKA",
	 .id = WM8350_ISINK_A,
	 .ops = &wm8350_isink_ops,
	 .irq = WM8350_IRQ_CS1,
	 .type = REGULATOR_CURRENT,
	 .owner = THIS_MODULE,
	 },
	{
	 .name = "ISINKB",
	 .id = WM8350_ISINK_B,
	 .ops = &wm8350_isink_ops,
	 .irq = WM8350_IRQ_CS2,
	 .type = REGULATOR_CURRENT,
	 .owner = THIS_MODULE,
	 },
};

static void pmic_uv_handler(struct wm8350 *wm8350, int irq, void *data)
{
	struct regulator_dev *rdev = (struct regulator_dev *)data;

	if (irq == WM8350_IRQ_CS1 || irq == WM8350_IRQ_CS2)
		regulator_notifier_call_chain(rdev,
					      REGULATOR_EVENT_REGULATION_OUT,
					      wm8350);
	else
		regulator_notifier_call_chain(rdev,
					      REGULATOR_EVENT_UNDER_VOLTAGE,
					      wm8350);
}

static int wm8350_regulator_probe(struct platform_device *pdev)
{
	struct wm8350 *wm8350 = dev_get_drvdata(&pdev->dev);
	struct wm8350_pmic *wm8350_pmic = &wm8350->pmic;
	int ret;
	u16 val;
//TODO add mutex
	if (pdev->id < WM8350_DCDC_1 || pdev->id > WM8350_ISINK_B)
		return -ENODEV;

	if (wm8350_pmic->rdev[pdev->id])
		return -EINVAL;

	/* do any regulatior specific init */
	switch (pdev->id) {
	case WM8350_DCDC_1:
		val = wm8350_reg_read(wm8350, WM8350_DCDC1_LOW_POWER);
		wm8350->pmic.dcdc1_hib_mode = val & WM8350_DCDC_HIB_MODE_MASK;
		break;
	case WM8350_DCDC_3:
		val = wm8350_reg_read(wm8350, WM8350_DCDC3_LOW_POWER);
		wm8350->pmic.dcdc3_hib_mode = val & WM8350_DCDC_HIB_MODE_MASK;
		break;
	case WM8350_DCDC_4:
		val = wm8350_reg_read(wm8350, WM8350_DCDC4_LOW_POWER);
		wm8350->pmic.dcdc4_hib_mode = val & WM8350_DCDC_HIB_MODE_MASK;
		break;
	case WM8350_DCDC_6:
		val = wm8350_reg_read(wm8350, WM8350_DCDC6_LOW_POWER);
		wm8350->pmic.dcdc6_hib_mode = val & WM8350_DCDC_HIB_MODE_MASK;
		break;
	}


	/* register regulator */
	wm8350_pmic->rdev[pdev->id] =
		    regulator_register(&wm8350_reg[pdev->id], &pdev->dev);
	if (IS_ERR(wm8350_pmic->rdev[pdev->id])) {
		printk(KERN_ERR "%s: failed to register %s\n",
			__func__, wm8350_reg[pdev->id].name);
		return PTR_ERR(wm8350_pmic->rdev[pdev->id]);
	}

	/* register regulator IRQ */
	ret = wm8350_register_irq(wm8350, wm8350_reg[pdev->id].irq,
					  pmic_uv_handler,
					  wm8350_pmic->rdev[pdev->id]);
	if (ret < 0) {
		regulator_unregister(wm8350_pmic->rdev[pdev->id]);
		wm8350_pmic->rdev[pdev->id] = NULL;
		printk(KERN_ERR "%s: failed to register regulator %s IRQ\n",
			__func__, wm8350_reg[pdev->id].name);
		return ret;
	}

	/* enable IRQ */
	wm8350_unmask_irq(wm8350, wm8350_reg[pdev->id].irq);
	return 0;
}

static int wm8350_regulator_remove(struct platform_device *pdev)
{
	struct wm8350 *wm8350 = dev_get_drvdata(&pdev->dev);
	struct wm8350_pmic *wm8350_pmic = &wm8350->pmic;

	if (pdev->id < WM8350_DCDC_1 || pdev->id > WM8350_ISINK_B)
		return -ENODEV;

	if (wm8350_pmic->rdev[pdev->id] == NULL)
		return -EINVAL;

	wm8350_mask_irq(wm8350, wm8350_reg[pdev->id].irq);
	wm8350_free_irq(wm8350, wm8350_reg[pdev->id].irq);
	regulator_unregister(wm8350_pmic->rdev[pdev->id]);
	wm8350_pmic->rdev[pdev->id] = NULL;
	return 0;
}

static struct platform_driver wm8350_regulator_driver = {
	.probe = wm8350_regulator_probe,
	.remove = wm8350_regulator_remove,
	.driver		= {
		.name	= "wm8350-regulator",
	},
};

static int __init wm8350_regulator_init(void)
{
	printk(KERN_INFO "wm8350 regulator version %s\n", WM8350_REG_VERSION);
	return platform_driver_register(&wm8350_regulator_driver);
}
subsys_initcall(wm8350_regulator_init);

static void __exit wm8350_regulator_exit(void)
{
	platform_driver_unregister(&wm8350_regulator_driver);
}
module_exit(wm8350_regulator_exit);

/* Module information */
MODULE_AUTHOR
    ("Liam Girdwood, liam.girdwood@wolfsonmicro.com, www.wolfsonmicro.com");
MODULE_DESCRIPTION("WM8350 regulator driver");
MODULE_LICENSE("GPL");
