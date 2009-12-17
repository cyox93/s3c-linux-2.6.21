/*
 * Regulator support for WM8400
 *
 * Copyright 2008 Wolfson Microelectronics PLC.
 *
 * Author: Mark Brown <broonie@opensource.wolfsonmicro.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 */

#include <asm/byteorder.h>

#include <linux/bug.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/regulator/driver.h>
#include <linux/mfd/wm8400/bus.h>
#include <linux/mfd/wm8400/registers.h>

static int wm8400_ldo_is_enabled(struct regulator_dev *dev)
{
	struct wm8400_regulator *reg = rdev_get_drvdata(dev);
	struct wm8400 *wm8400 = container_of(reg, struct wm8400, regulator);
	u16 val;

	val = wm8400_reg_read(wm8400, WM8400_LDO1_CONTROL + rdev_get_id(dev));
	return (val & WM8400_LDO1_ENA) == 0;
}

static int wm8400_ldo_enable(struct regulator_dev *dev)
{
	struct wm8400_regulator *reg = rdev_get_drvdata(dev);
	struct wm8400 *wm8400 = container_of(reg, struct wm8400, regulator);

	return wm8400_set_bits(wm8400, WM8400_LDO1_CONTROL + rdev_get_id(dev),
			       WM8400_LDO1_ENA, WM8400_LDO1_ENA);
}

static int wm8400_ldo_disable(struct regulator_dev *dev)
{
	struct wm8400_regulator *reg = rdev_get_drvdata(dev);
	struct wm8400 *wm8400 = container_of(reg, struct wm8400, regulator);

	return wm8400_set_bits(wm8400, WM8400_LDO1_CONTROL + rdev_get_id(dev),
			       WM8400_LDO1_ENA, 0);
}

static int wm8400_ldo_get_voltage(struct regulator_dev *dev)
{
	struct wm8400_regulator *reg = rdev_get_drvdata(dev);
	struct wm8400 *wm8400 = container_of(reg, struct wm8400, regulator);
	u16 val;

	val = wm8400_reg_read(wm8400, WM8400_LDO1_CONTROL + rdev_get_id(dev));
	val &= WM8400_LDO1_VSEL_MASK;

	if (val < 15)
		return 900000 + (val * 50000);
	else
		return 1600000 + ((val - 14) * 100000);
}

static int wm8400_ldo_set_voltage(struct regulator_dev *dev,
				  int min_uV, int max_uV)
{
	struct wm8400_regulator *reg = rdev_get_drvdata(dev);
	struct wm8400 *wm8400 = container_of(reg, struct wm8400, regulator);
	u16 val;

	if (min_uV < 900000 || min_uV > 3300000)
		return -EINVAL;

	if (min_uV < 1700000) {
		/* Steps of 50mV from 900mV;  */
		val = (min_uV - 850001) / 50000;

		if ((val * 50000) + 900000 > max_uV) {
			return -EINVAL;
		}
		BUG_ON((val * 50000) + 900000 < min_uV);
	} else {
		/* Steps of 100mV from 1700mV */
		val = ((min_uV - 1600001) / 100000);

		if ((val * 100000) + 1700000 > max_uV) {
			return -EINVAL;
		}
		BUG_ON((val * 100000) + 1700000 < min_uV);

		val += 0xf;
	}

	return wm8400_set_bits(wm8400, WM8400_LDO1_CONTROL + rdev_get_id(dev),
			       WM8400_LDO1_VSEL_MASK, val);
}

static struct regulator_ops wm8400_ldo_ops = {
	.is_enabled = wm8400_ldo_is_enabled,
	.enable = wm8400_ldo_enable,
	.disable = wm8400_ldo_disable,
	.get_voltage = wm8400_ldo_get_voltage,
	.set_voltage = wm8400_ldo_set_voltage,
};

static int wm8400_dcdc_is_enabled(struct regulator_dev *dev)
{
	struct wm8400_regulator *reg = rdev_get_drvdata(dev);
	struct wm8400 *wm8400 = container_of(reg, struct wm8400, regulator);
	int offset = rdev_get_id(dev) * 2;
	u16 val;

	val = wm8400_reg_read(wm8400, WM8400_DCDC1_CONTROL_1 + offset);
	return (val & WM8400_DC1_ENA) == 0;
}

static int wm8400_dcdc_enable(struct regulator_dev *dev)
{
	struct wm8400_regulator *reg = rdev_get_drvdata(dev);
	struct wm8400 *wm8400 = container_of(reg, struct wm8400, regulator);
	int offset = rdev_get_id(dev) * 2;

	return wm8400_set_bits(wm8400, WM8400_DCDC1_CONTROL_1 + offset,
			       WM8400_DC1_ENA, WM8400_DC1_ENA);
}

static int wm8400_dcdc_disable(struct regulator_dev *dev)
{
	struct wm8400_regulator *reg = rdev_get_drvdata(dev);
	struct wm8400 *wm8400 = container_of(reg, struct wm8400, regulator);
	int offset = rdev_get_id(dev) * 2;

	return wm8400_set_bits(wm8400, WM8400_DCDC1_CONTROL_1 + offset,
			       WM8400_DC1_ENA, 0);
}

static int wm8400_dcdc_get_voltage(struct regulator_dev *dev)
{
	struct wm8400_regulator *reg = rdev_get_drvdata(dev);
	struct wm8400 *wm8400 = container_of(reg, struct wm8400, regulator);
	u16 val;
	int offset = rdev_get_id(dev) * 2;

	val = wm8400_reg_read(wm8400, WM8400_DCDC1_CONTROL_1 + offset);
	val &= WM8400_DC1_VSEL_MASK;

	return 850000 + (25000 * val);
}

static int wm8400_dcdc_set_voltage(struct regulator_dev *dev,
				  int min_uV, int max_uV)
{
	struct wm8400_regulator *reg = rdev_get_drvdata(dev);
	struct wm8400 *wm8400 = container_of(reg, struct wm8400, regulator);
	u16 val;
	int offset = rdev_get_id(dev) * 2;

	if (min_uV < 850000)
		return -EINVAL;

	val = (min_uV - 825001) / 25000;

	if (850000 + (25000 * val) > max_uV)
		return -EINVAL;
	BUG_ON(850000 + (25000 * val) < min_uV);

	return wm8400_set_bits(wm8400, WM8400_DCDC1_CONTROL_1 + offset,
			       WM8400_DC1_VSEL_MASK, val);
}

static unsigned int wm8400_dcdc_get_mode(struct regulator_dev *dev)
{
	struct wm8400 *wm8400 = container_of(rdev_get_drvdata(dev),
					     struct wm8400, regulator);
	int offset = rdev_get_id(dev) * 2;
	u16 data[2];
	int ret;

	ret = wm8400_block_read(wm8400, WM8400_DCDC1_CONTROL_1 + offset, 2,
				data);
	if (ret != 0)
		return 0;

	/* Datasheet: hibernate */
	if (data[0] & WM8400_DC1_SLEEP)
		return REGULATOR_MODE_STANDBY;

	/* Datasheet: standby */
	if (!(data[0] & WM8400_DC1_ACTIVE))
		return REGULATOR_MODE_IDLE;

	/* Datasheet: active with or without force PWM */
	if (data[1] & WM8400_DC1_FRC_PWM)
		return REGULATOR_MODE_FAST;
	else
		return REGULATOR_MODE_NORMAL;
}

static int wm8400_dcdc_set_mode(struct regulator_dev *dev, unsigned int mode)
{
	struct wm8400 *wm8400 = container_of(rdev_get_drvdata(dev),
					     struct wm8400, regulator);
	int offset = rdev_get_id(dev) * 2;
	int ret;

	switch (mode) {
	case REGULATOR_MODE_FAST:
		/* Datasheet: active with force PWM */
		ret = wm8400_set_bits(wm8400, WM8400_DCDC1_CONTROL_2 + offset,
				      WM8400_DC1_FRC_PWM, WM8400_DC1_FRC_PWM);
		if (ret != 0)
			return ret;

		return wm8400_set_bits(wm8400, WM8400_DCDC1_CONTROL_1 + offset,
				       WM8400_DC1_ACTIVE | WM8400_DC1_SLEEP,
				       WM8400_DC1_ACTIVE);

	case REGULATOR_MODE_NORMAL:
		/* Datasheet: active */
		ret = wm8400_set_bits(wm8400, WM8400_DCDC1_CONTROL_2 + offset,
				      WM8400_DC1_FRC_PWM, 0);
		if (ret != 0)
			return ret;

		return wm8400_set_bits(wm8400, WM8400_DCDC1_CONTROL_1 + offset,
				       WM8400_DC1_ACTIVE | WM8400_DC1_SLEEP,
				       WM8400_DC1_ACTIVE);

	case REGULATOR_MODE_IDLE:
		/* Datasheet: standby */
		ret = wm8400_set_bits(wm8400, WM8400_DCDC1_CONTROL_1 + offset,
				      WM8400_DC1_ACTIVE, 0);
		if (ret != 0)
			return ret;
		return wm8400_set_bits(wm8400, WM8400_DCDC1_CONTROL_1 + offset,
				       WM8400_DC1_SLEEP, 0);

	default:
		return -EINVAL;
	}
}

unsigned int wm8400_dcdc_get_optimum_mode(struct regulator_dev *dev,
					  int input_uV, int output_uV,
					  int load_uA)
{
	return REGULATOR_MODE_IDLE;
}

static struct regulator_ops wm8400_dcdc_ops = {
	.is_enabled = wm8400_dcdc_is_enabled,
	.enable = wm8400_dcdc_enable,
	.disable = wm8400_dcdc_disable,
	.get_voltage = wm8400_dcdc_get_voltage,
	.set_voltage = wm8400_dcdc_set_voltage,
	.get_mode = wm8400_dcdc_get_mode,
	.set_mode = wm8400_dcdc_set_mode,
	.get_optimum_mode = wm8400_dcdc_get_optimum_mode,
};

static struct regulator_desc regulators[] = {
	{
		.name = "LDO1",
		.id = WM8400_LDO1,
		.ops = &wm8400_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	{
		.name = "LDO2",
		.id = WM8400_LDO2,
		.ops = &wm8400_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	{
		.name = "LDO3",
		.id = WM8400_LDO3,
		.ops = &wm8400_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	{
		.name = "LDO4",
		.id = WM8400_LDO4,
		.ops = &wm8400_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	{
		.name = "DCDC1",
		.id = WM8400_DCDC1,
		.ops = &wm8400_dcdc_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	{
		.name = "DCDC2",
		.id = WM8400_DCDC2,
		.ops = &wm8400_dcdc_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
};

int wm8400_regulator_probe(struct device *dev)
{
	int ret;
	int i;
	struct wm8400_regulator *regulator
		= container_of(dev, struct wm8400_regulator, dev);

	BUILD_BUG_ON(ARRAY_SIZE(regulators) != ARRAY_SIZE(regulator->rdev));

	/* Set regulator state software control to active mode so we have
	 * full software control of the regulators.
	 */
	wm8400_set_bits(container_of(regulator, struct wm8400, regulator),
			WM8400_PM_GENERAL, WM8400_PWR_STATE_MASK, 0);

	for (i = 0; i < ARRAY_SIZE(regulators); i++) {
		regulator->rdev[i] = regulator_register(&regulators[i], regulator);
		if (IS_ERR(regulator->rdev[i])) {
			dev_err(dev, "failed to register %s err %ld\n",
				regulators[i].name, PTR_ERR(regulator->rdev[i]));
			regulator->rdev[i] = 0;
			ret = PTR_ERR(regulator->rdev[i]);
			goto err;
		}
	}

	return 0;

err:
	return ret;
}

static int __exit wm8400_regulator_remove(struct device *dev)
{
	struct wm8400_regulator *regulator
		= container_of(dev, struct wm8400_regulator, dev);
	int i;

	for (i = 0; i < ARRAY_SIZE(regulator->rdev); i++) {
		regulator_unregister(regulator->rdev[i]);
	}

	return 0;
}

struct device_driver wm8400_regulator_driver = {
	.name = "wm8400-regulator",
	.bus = &wm8400_bus_type,
	.probe = wm8400_regulator_probe,
	.remove	= __exit_p(wm8400_regulator_remove),
};

int wm8400_regulator_init(void)
{
	return driver_register(&wm8400_regulator_driver);
}

void wm8400_regulator_exit(void)
{
	driver_unregister(&wm8400_regulator_driver);
}
