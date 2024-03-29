/*
 * core.c  --  Voltage/Current Regulator framework.
 *
 * Copyright 2007, 2008 Wolfson Microelectronics PLC.
 *
 * Author: Liam Girdwood <liam.girdwood@wolfsonmicro.com>
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/suspend.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>

#define REGULATOR_VERSION "0.5"

static DEFINE_MUTEX(regulator_list_mutex);
static LIST_HEAD(regulator_list);
static LIST_HEAD(regulator_map_list);

/**
 * struct regulator_dev
 *
 * Voltage / Current regulator class device. One for each regulator.
 */
struct regulator_dev {
	struct regulator_desc *desc;
	int use_count;

	/* lists we belong to */
	struct list_head list; /* list of all regulators */
	struct list_head slist; /* list of supplied regulators */

	/* lists we own */
	struct list_head consumer_list; /* consumers we supply */
	struct list_head supply_list; /* regulators we supply */

	struct blocking_notifier_head notifier;
	struct mutex mutex; /* consumer lock */
	struct module *owner;
	struct device *dev;
	struct regulation_constraints *constraints;
	struct regulator_dev *supply;	/* for tree */

	void *reg_data;		/* regulator_dev data */
};

/**
 * struct regulator_map
 *
 * Used to provide symbolic supply names to devices.
 */
struct regulator_map {
	struct list_head list;
	struct device *dev;
	const char *supply;
	struct regulator_dev *regulator;
};

/*
 * struct regulator
 *
 * One for each consumer device.
 */
struct regulator {
	struct device *dev;
	struct list_head list;
	int uA_load;
	int min_uV;
	int max_uV;
	int enabled; /* client has called enabled */
	char *supply_name;
	struct device_attribute dev_attr;
	struct regulator_dev *rdev;
};

static int _regulator_is_enabled(struct regulator_dev *rdev);
static int _regulator_disable(struct regulator_dev *rdev);
static int _regulator_get_voltage(struct regulator_dev *rdev);
static int _regulator_get_current_limit(struct regulator_dev *rdev);
static unsigned int _regulator_get_mode(struct regulator_dev *rdev);
static void _notifier_call_chain(struct regulator_dev *rdev,
				  unsigned long event, void *data);

/* gets the regulator for a given consumer device */
static struct regulator *get_device_regulator(struct device *dev)
{
	struct regulator *regulator = NULL;
	struct regulator_dev *rdev;

	mutex_lock(&regulator_list_mutex);
	list_for_each_entry(rdev, &regulator_list, list) {
		mutex_lock(&rdev->mutex);
		list_for_each_entry(regulator, &rdev->consumer_list, list) {
			if (regulator->dev == dev) {
				mutex_unlock(&rdev->mutex);
				mutex_unlock(&regulator_list_mutex);
				return regulator;
			}
		}
		mutex_unlock(&rdev->mutex);
	}
	mutex_unlock(&regulator_list_mutex);
	return NULL;
}

/* Platform voltage constraint check */
static int regulator_check_voltage(struct regulator_dev *rdev,
				   int *min_uV, int *max_uV)
{
	BUG_ON(*min_uV > *max_uV);

	if (!rdev->constraints) {
		printk(KERN_ERR "%s: no constraints for %s\n", __func__,
		       rdev->desc->name);
		return -ENODEV;
	}
	if (!(rdev->constraints->valid_ops_mask & REGULATOR_CHANGE_VOLTAGE)) {
		printk(KERN_ERR "%s: operation not allowed for %s\n",
		       __func__, rdev->desc->name);
		return -EPERM;
	}

	if (*max_uV > rdev->constraints->max_uV)
		*max_uV = rdev->constraints->max_uV;
	if (*min_uV < rdev->constraints->min_uV)
		*min_uV = rdev->constraints->min_uV;

	if (*min_uV > *max_uV)
		return -EINVAL;

	return 0;
}

/* current constraint check */
static int regulator_check_current_limit(struct regulator_dev *rdev,
					int *min_uA, int *max_uA)
{
	BUG_ON(*min_uA > *max_uA);

	if (!rdev->constraints) {
		printk(KERN_ERR "%s: no constraints for %s\n", __func__,
		       rdev->desc->name);
		return -ENODEV;
	}
	if (!(rdev->constraints->valid_ops_mask & REGULATOR_CHANGE_CURRENT)) {
		printk(KERN_ERR "%s: operation not allowed for %s\n",
		       __func__, rdev->desc->name);
		return -EPERM;
	}

	if (*max_uA > rdev->constraints->max_uA)
		*max_uA = rdev->constraints->max_uA;
	if (*min_uA < rdev->constraints->min_uA)
		*min_uA = rdev->constraints->min_uA;

	if (*min_uA > *max_uA)
		return -EINVAL;

	return 0;
}

/* operating mode constraint check */
static int regulator_check_mode(struct regulator_dev *rdev, int mode)
{
	if (!rdev->constraints) {
		printk(KERN_ERR "%s: no constraints for %s\n", __func__,
		       rdev->desc->name);
		return -ENODEV;
	}
	if (!(rdev->constraints->valid_ops_mask & REGULATOR_CHANGE_MODE)) {
		printk(KERN_ERR "%s: operation not allowed for %s\n",
		       __func__, rdev->desc->name);
		return -EPERM;
	}
	if (!(rdev->constraints->valid_modes_mask & mode)) {
		printk(KERN_ERR "%s: invalid mode %x for %s\n",
		       __func__, mode, rdev->desc->name);
		return -EINVAL;
	}
	return 0;
}

/* dynamic regulator mode switching constraint check */
static int regulator_check_drms(struct regulator_dev *rdev)
{
	if (!rdev->constraints) {
		printk(KERN_ERR "%s: no constraints for %s\n", __func__,
		       rdev->desc->name);
		return -ENODEV;
	}
	if (!(rdev->constraints->valid_ops_mask & REGULATOR_CHANGE_DRMS)) {
		printk(KERN_ERR "%s: operation not allowed for %s\n",
		       __func__, rdev->desc->name);
		return -EPERM;
	}
	return 0;
}

static ssize_t device_requested_uA_show(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
	struct regulator *regulator;

	regulator = get_device_regulator(dev);
	if (regulator == NULL)
		return 0;

	return sprintf(buf, "%d\n", regulator->uA_load);
}

static ssize_t regulator_uV_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);
	ssize_t ret;

	mutex_lock(&rdev->mutex);
	ret = sprintf(buf, "%d\n", _regulator_get_voltage(rdev));
	mutex_unlock(&rdev->mutex);

	return ret;
}

static ssize_t regulator_uA_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", _regulator_get_current_limit(rdev));
}

static ssize_t regulator_opmode_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);
	int mode = _regulator_get_mode(rdev);

	switch (mode) {
	case REGULATOR_MODE_FAST:
		return sprintf(buf, "fast\n");
	case REGULATOR_MODE_NORMAL:
		return sprintf(buf, "normal\n");
	case REGULATOR_MODE_IDLE:
		return sprintf(buf, "idle\n");
	case REGULATOR_MODE_STANDBY:
		return sprintf(buf, "standby\n");
	}
	return sprintf(buf, "unknown\n");
}

static ssize_t regulator_state_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);
	int state = _regulator_is_enabled(rdev);

	if (state > 0)
		return sprintf(buf, "enabled\n");
	else if (state == 0)
		return sprintf(buf, "disabled\n");
	else
		return sprintf(buf, "unknown\n");
}

static ssize_t regulator_min_uA_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);

	if (!rdev->constraints)
		return sprintf(buf, "constraint not defined\n");

	return sprintf(buf, "%d\n", rdev->constraints->min_uA);
}

static ssize_t regulator_max_uA_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);

	if (!rdev->constraints)
		return sprintf(buf, "constraint not defined\n");

	return sprintf(buf, "%d\n", rdev->constraints->max_uA);
}

static ssize_t regulator_min_uV_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);

	if (!rdev->constraints)
		return sprintf(buf, "constraint not defined\n");

	return sprintf(buf, "%d\n", rdev->constraints->min_uV);
}

static ssize_t regulator_max_uV_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);

	if (!rdev->constraints)
		return sprintf(buf, "constraint not defined\n");

	return sprintf(buf, "%d\n", rdev->constraints->max_uV);
}

static ssize_t regulator_total_uA_show(struct device *dev,
				      struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);
	struct regulator *regulator;
	int uA = 0;

	mutex_lock(&rdev->mutex);
	list_for_each_entry(regulator, &rdev->consumer_list, list)
	    uA += regulator->uA_load;
	mutex_unlock(&rdev->mutex);
	return sprintf(buf, "%d\n", uA);
}

static ssize_t regulator_num_users_show(struct device *dev,
				      struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);
	return sprintf(buf, "%d\n", rdev->use_count);
}

static ssize_t regulator_type_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);

	switch (rdev->desc->type) {
	case REGULATOR_VOLTAGE:
		return sprintf(buf, "voltage\n");
	case REGULATOR_CURRENT:
		return sprintf(buf, "current\n");
	}
	return sprintf(buf, "unknown\n");
}

static ssize_t regulator_suspend_mem_uV_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);

	if (!rdev->constraints)
		return sprintf(buf, "not defined\n");
	return sprintf(buf, "%d\n", rdev->constraints->state_mem.uV);
}

static ssize_t regulator_suspend_disk_uV_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);

	if (!rdev->constraints)
		return sprintf(buf, "not defined\n");
	return sprintf(buf, "%d\n", rdev->constraints->state_disk.uV);
}

static ssize_t regulator_suspend_standby_uV_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);

	if (!rdev->constraints)
		return sprintf(buf, "not defined\n");
	return sprintf(buf, "%d\n", rdev->constraints->state_standby.uV);
}

static ssize_t suspend_opmode_show(struct regulator_dev *rdev,
	unsigned int mode, char *buf)
{
	switch (mode) {
	case REGULATOR_MODE_FAST:
		return sprintf(buf, "fast\n");
	case REGULATOR_MODE_NORMAL:
		return sprintf(buf, "normal\n");
	case REGULATOR_MODE_IDLE:
		return sprintf(buf, "idle\n");
	case REGULATOR_MODE_STANDBY:
		return sprintf(buf, "standby\n");
	}
	return sprintf(buf, "unknown\n");
}

static ssize_t regulator_suspend_mem_mode_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);

	if (!rdev->constraints)
		return sprintf(buf, "not defined\n");
	return suspend_opmode_show(rdev,
		rdev->constraints->state_mem.mode, buf);
}

static ssize_t regulator_suspend_disk_mode_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);

	if (!rdev->constraints)
		return sprintf(buf, "not defined\n");
	return suspend_opmode_show(rdev,
		rdev->constraints->state_disk.mode, buf);
}

static ssize_t regulator_suspend_standby_mode_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);

	if (!rdev->constraints)
		return sprintf(buf, "not defined\n");
	return suspend_opmode_show(rdev,
		rdev->constraints->state_standby.mode, buf);
}

static ssize_t regulator_suspend_mem_state_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);

	if (!rdev->constraints)
		return sprintf(buf, "not defined\n");

	if (rdev->constraints->state_mem.enabled)
		return sprintf(buf, "enabled\n");
	else
		return sprintf(buf, "disabled\n");
}

static ssize_t regulator_suspend_disk_state_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);

	if (!rdev->constraints)
		return sprintf(buf, "not defined\n");

	if (rdev->constraints->state_disk.enabled)
		return sprintf(buf, "enabled\n");
	else
		return sprintf(buf, "disabled\n");
}

static ssize_t regulator_suspend_standby_state_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);

	if (!rdev->constraints)
		return sprintf(buf, "not defined\n");

	if (rdev->constraints->state_standby.enabled)
		return sprintf(buf, "enabled\n");
	else
		return sprintf(buf, "disabled\n");
}
static struct device_attribute regulator_dev_attrs[] = {
	__ATTR(microvolts, 0444, regulator_uV_show, NULL),
	__ATTR(microamps, 0444, regulator_uA_show, NULL),
	__ATTR(opmode, 0444, regulator_opmode_show, NULL),
	__ATTR(state, 0444, regulator_state_show, NULL),
	__ATTR(min_microvolts, 0444, regulator_min_uV_show, NULL),
	__ATTR(min_microamps, 0444, regulator_min_uA_show, NULL),
	__ATTR(max_microvolts, 0444, regulator_max_uV_show, NULL),
	__ATTR(max_microamps, 0444, regulator_max_uA_show, NULL),
	__ATTR(requested_microamps, 0444, regulator_total_uA_show, NULL),
	__ATTR(num_users, 0444, regulator_num_users_show, NULL),
	__ATTR(type, 0444, regulator_type_show, NULL),
	__ATTR(suspend_mem_microvolts, 0444,
		regulator_suspend_mem_uV_show, NULL),
	__ATTR(suspend_disk_microvolts, 0444,
		regulator_suspend_disk_uV_show, NULL),
	__ATTR(suspend_standby_microvolts, 0444,
		regulator_suspend_standby_uV_show, NULL),
	__ATTR(suspend_mem_mode, 0444,
		regulator_suspend_mem_mode_show, NULL),
	__ATTR(suspend_disk_mode, 0444,
		regulator_suspend_disk_mode_show, NULL),
	__ATTR(suspend_standby_mode, 0444,
		regulator_suspend_standby_mode_show, NULL),
	__ATTR(suspend_mem_state, 0444,
		regulator_suspend_mem_state_show, NULL),
	__ATTR(suspend_disk_state, 0444,
		regulator_suspend_disk_state_show, NULL),
	__ATTR(suspend_standby_state, 0444,
		regulator_suspend_standby_state_show, NULL),
	__ATTR_NULL,
};

static void regulator_dev_release(struct device *dev)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);
	kfree(rdev);
}

static struct class regulator_class = {
	.name = "regulator",
	.dev_release = regulator_dev_release,
	.dev_attrs = regulator_dev_attrs,
};

/* Calculate the new optimum regulator operating mode based on the new total
 * consumer load. All locks held by caller */
static void drms_uA_update(struct regulator_dev *rdev)
{
	struct regulator *sibling;
	int current_uA = 0, output_uV, input_uV, err;
	unsigned int mode;

	err = regulator_check_drms(rdev);
	if (err < 0 || !rdev->desc->ops->get_optimum_mode ||
	    !rdev->desc->ops->get_voltage || !rdev->desc->ops->set_mode);
	return;

	/* get output voltage */
	output_uV = rdev->desc->ops->get_voltage(rdev);
	if (output_uV <= 0)
		return;

	/* get input voltage */
	if (rdev->supply && rdev->supply->desc->ops->get_voltage)
		input_uV = rdev->supply->desc->ops->get_voltage(rdev->supply);
	else
		input_uV = rdev->constraints->input_uV;
	if (input_uV <= 0)
		return;

	/* calc total requested load */
	list_for_each_entry(sibling, &rdev->consumer_list, list)
	    current_uA += sibling->uA_load;

	/* now get the optimum mode for our new total regulator load */
	mode = rdev->desc->ops->get_optimum_mode(rdev, input_uV,
						  output_uV, current_uA);

	/* check the new mode is allowed */
	err = regulator_check_mode(rdev, mode);
	if (err == 0)
		rdev->desc->ops->set_mode(rdev, mode);
}

static int suspend_set_state(struct regulator_dev *rdev,
	struct regulator_state *rstate)
{
	int ret = 0;

	/* enable & disable are mandatory for suspend control */
	if (!rdev->desc->ops->set_suspend_enable ||
		!rdev->desc->ops->set_suspend_disable) {
		printk(KERN_ERR "%s: no way to set suspend state\n",
			__func__);
		return -EINVAL;
	}

	if (rstate->enabled)
		ret = rdev->desc->ops->set_suspend_enable(rdev);
	else
		ret = rdev->desc->ops->set_suspend_disable(rdev);
	if (ret < 0) {
		printk(KERN_ERR "%s: failed to enabled/disable\n", __func__);
		return ret;
	}

	if (rdev->desc->ops->set_suspend_voltage && rstate->uV > 0) {
		ret = rdev->desc->ops->set_suspend_voltage(rdev, rstate->uV);
		if (ret < 0) {
			printk(KERN_ERR "%s: failed to set voltage\n",
				__func__);
			return ret;
		}
	}

	if (rdev->desc->ops->set_suspend_mode && rstate->mode > 0) {
		ret = rdev->desc->ops->set_suspend_mode(rdev, rstate->mode);
		if (ret < 0) {
			printk(KERN_ERR "%s: failed to set mode\n", __func__);
			return ret;
		}
	}
	return ret;
}

/* locks held by caller */
static int suspend_prepare(struct regulator_dev *rdev, suspend_state_t state)
{

	if (!rdev->constraints)
		return -EINVAL;

	switch (state) {
	case PM_SUSPEND_STANDBY:
		return suspend_set_state(rdev,
			&rdev->constraints->state_standby);
	case PM_SUSPEND_MEM:
		return suspend_set_state(rdev,
			&rdev->constraints->state_mem);
	case PM_SUSPEND_MAX:
		return suspend_set_state(rdev,
			&rdev->constraints->state_disk);
	default:

		return -EINVAL;
	}
}

static void print_constraints(struct regulator_dev *rdev)
{
	struct regulation_constraints *constraints = rdev->constraints;
	char buf[80];
	int count;

	if (rdev->desc->type == REGULATOR_VOLTAGE) {
		if (constraints->min_uV == constraints->max_uV)
			count = sprintf(buf, "%d mV ",
					constraints->min_uV / 1000);
		else
			count = sprintf(buf, "%d <--> %d mV ",
					constraints->min_uV / 1000,
					constraints->max_uV / 1000);
	} else {
		if (constraints->min_uA == constraints->max_uA)
			count = sprintf(buf, "%d mA ",
					constraints->min_uA / 1000);
		else
			count = sprintf(buf, "%d <--> %d mA ",
					constraints->min_uA / 1000,
					constraints->max_uA / 1000);
	}
	if (constraints->valid_modes_mask & REGULATOR_MODE_FAST)
		count += sprintf(buf + count, "fast ");
	if (constraints->valid_modes_mask & REGULATOR_MODE_NORMAL)
		count += sprintf(buf + count, "normal ");
	if (constraints->valid_modes_mask & REGULATOR_MODE_IDLE)
		count += sprintf(buf + count, "idle ");
	if (constraints->valid_modes_mask & REGULATOR_MODE_STANDBY)
		count += sprintf(buf + count, "standby");

	printk(KERN_INFO "regulator: %s: %s\n", rdev->desc->name, buf);
}

/**
 * set_machine_constraints - sets regulator constraints
 * @regulator: regulator source
 *
 * Allows platform initialisation code to define and constrain
 * regulator circuits e.g. valid voltage/current ranges, etc.  NOTE:
 * Constraints *must* be set by platform code in order for some
 * regulator operations to proceed i.e. set_voltage, set_current_limit,
 * set_mode.
 */
static int set_machine_constraints(struct regulator_dev *rdev,
	struct regulation_constraints *constraints)
{
	int ret = 0;

	rdev->constraints = constraints;

	/* do we need to apply the constraint voltage */
	if (rdev->constraints->apply_uV &&
		rdev->constraints->min_uV == rdev->constraints->max_uV &&
		rdev->desc->ops->set_voltage) {
		ret = rdev->desc->ops->set_voltage(rdev,
			rdev->constraints->min_uV, rdev->constraints->max_uV);
			if (ret < 0) {
				printk(KERN_ERR "%s: failed to apply %duV"
					" constraint\n", __func__,
					rdev->constraints->min_uV);
				rdev->constraints = NULL;
				goto out;
			}
	}

	/* are we enabled at boot time by firmware / bootloader */
	if (rdev->constraints->boot_on)
		rdev->use_count = 1;

	/* do we need to setup our suspend state */
	if (constraints->initial_state)
		ret = suspend_prepare(rdev, constraints->initial_state);

	print_constraints(rdev);
out:
	return ret;
}

/**
 * set_supply - set regulator supply regulator
 * @regulator: regulator name
 * @supply: supply regulator name
 *
 * Called by platform initialisation code to set the supply regulator for this
 * regulator. This ensures that a regulators supply will also be enabled by the
 * core if it's child is enabled.
 */
static int set_supply(struct regulator_dev *rdev,
	struct regulator_dev *supply_rdev)
{
	int err;

	err = sysfs_create_link(&rdev->dev->kobj, &supply_rdev->dev->kobj,
				"supply");
	if (err) {
		printk(KERN_ERR
		       "%s: could not add device link %s err %d\n",
		       __func__, supply_rdev->dev->kobj.name, err);
		       goto out;
	}
	rdev->supply = supply_rdev;
	list_add(&rdev->slist, &supply_rdev->supply_list);
out:
	return err;
}

/**
 * set_consumer_device_supply: Bind a regulator to a symbolic supply
 * @regulator: regulator source
 * @dev:       device the supply applies to
 * @supply:    symbolic name for supply
 *
 * Allows platform initialisation code to map physical regulator
 * sources to symbolic names for supplies for use by devices.  Devices
 * should use these symbolic names to request regulators, avoiding the
 * need to provide board-specific regulator names as platform data.
 */
static int set_consumer_device_supply(struct regulator_dev *rdev,
	struct device *consumer_dev, const char *supply)
{
	struct regulator_map *node;

	if (supply == NULL)
		return -EINVAL;

	node = kmalloc(sizeof(struct regulator_map), GFP_KERNEL);
	if (node == NULL)
		return -ENOMEM;

	node->regulator = rdev;
	node->dev = consumer_dev;
	node->supply = supply;

	list_add(&node->list, &regulator_map_list);
	return 0;
}

#define REG_STR_SIZE	32

static struct regulator *create_regulator(struct regulator_dev *rdev,
					  struct device *dev,
					  const char *supply_name)
{
	struct regulator *regulator;
	char buf[REG_STR_SIZE];
	int err, size;

	regulator = kzalloc(sizeof(*regulator), GFP_KERNEL);
	if (regulator == NULL)
		return NULL;

	mutex_lock(&rdev->mutex);
	regulator->rdev = rdev;
	list_add(&regulator->list, &rdev->consumer_list);

	if (dev) {
		/* create a 'requested_microamps_name' sysfs entry */
		size = scnprintf(buf, REG_STR_SIZE, "microamps_requested_%s",
			supply_name);
		if (size >= REG_STR_SIZE)
			goto overflow_err;

		regulator->dev = dev;
		regulator->dev_attr.attr.name = kstrdup(buf, GFP_KERNEL);
		if (regulator->dev_attr.attr.name == NULL)
			goto attr_name_err;

		regulator->dev_attr.attr.owner = THIS_MODULE;
		regulator->dev_attr.attr.mode = 0444;
		regulator->dev_attr.show = device_requested_uA_show;
		err = device_create_file(dev, &regulator->dev_attr);
		if (err < 0) {
			printk(KERN_WARNING "%s: could not add regulator_dev"
				" load sysfs\n", __func__);
			goto attr_name_err;
		}

		/* also add a link to the device sysfs entry */
		size = scnprintf(buf, REG_STR_SIZE, "%s-%s",
				 dev->kobj.name, supply_name);
		if (size >= REG_STR_SIZE)
			goto attr_err;

		regulator->supply_name = kstrdup(buf, GFP_KERNEL);
		if (regulator->supply_name == NULL)
			goto attr_err;

		err = sysfs_create_link(&rdev->dev->kobj, &dev->kobj,
					buf);
		if (err) {
			printk(KERN_WARNING
			       "%s: could not add device link %s err %d\n",
			       __func__, dev->kobj.name, err);
			device_remove_file(dev, &regulator->dev_attr);
			goto link_name_err;
		}
	}
	mutex_unlock(&rdev->mutex);
	return regulator;
link_name_err:
	kfree(regulator->supply_name);
attr_err:
	device_remove_file(regulator->dev, &regulator->dev_attr);
attr_name_err:
	kfree(regulator->dev_attr.attr.name);
overflow_err:
	list_del(&regulator->list);
	kfree(regulator);
	mutex_unlock(&rdev->mutex);
	return NULL;
}

/**
 * regulator_get - lookup and obtain a reference to a regulator.
 * @dev: device for regulator "consumer"
 * @id: Supply name or regulator ID.
 *
 * Returns a struct regulator corresponding to the regulator producer,
 * or IS_ERR() condition containing errno.  Use of supply names
 * configured via regulator_set_device_supply() is strongly
 * encouraged.
 */
struct regulator *regulator_get(struct device *dev, const char *id)
{
	struct regulator_dev *rdev;
	struct regulator_map *map;
	struct regulator *regulator = ERR_PTR(-ENODEV);

	if (id == NULL) {
		printk(KERN_ERR "regulator: get() with no identifier\n");
		return regulator;
	}

	mutex_lock(&regulator_list_mutex);

	list_for_each_entry(map, &regulator_map_list, list) {
		if (dev == map->dev &&
		    strcmp(map->supply, id) == 0) {
			rdev = map->regulator;
			goto found;
		}
	}
	printk(KERN_ERR "regulator: Unable to get requested regulator: %s\n",
	       id);
	mutex_unlock(&regulator_list_mutex);
	return regulator;

found:
	if (!try_module_get(rdev->owner))
		goto out;

	regulator = create_regulator(rdev, dev, id);
	if (regulator == NULL) {
		regulator = ERR_PTR(-ENOMEM);
		module_put(rdev->owner);
	}

out:
	mutex_unlock(&regulator_list_mutex);
	return regulator;
}
EXPORT_SYMBOL_GPL(regulator_get);

/**
 * regulator_put - "free" the regulator source
 * @regulator: regulator source
 *
 * Note: drivers must ensure that all regulator_enable calls made on this
 * regulator source are balanced by regulator_disable calls prior to calling
 * this function.
 */
void regulator_put(struct regulator *regulator)
{
	struct regulator_dev *rdev;

	if (regulator == NULL || IS_ERR(regulator))
		return;

	if (regulator->enabled) {
		printk(KERN_WARNING "Releasing supply %s while enabled\n",
		       regulator->supply_name);
		WARN_ON(regulator->enabled);
		regulator_disable(regulator);
	}

	mutex_lock(&regulator_list_mutex);
	rdev = regulator->rdev;

	/* remove any sysfs entries */
	if (regulator->dev) {
		sysfs_remove_link(&rdev->dev->kobj, regulator->supply_name);
		kfree(regulator->supply_name);
		device_remove_file(regulator->dev, &regulator->dev_attr);
		kfree(regulator->dev_attr.attr.name);
	}
	list_del(&regulator->list);
	kfree(regulator);

	module_put(rdev->owner);
	mutex_unlock(&regulator_list_mutex);
}
EXPORT_SYMBOL_GPL(regulator_put);

/* locks held by regulator_enable() */
static int _regulator_enable(struct regulator_dev *rdev)
{
	int ret = -EINVAL;

	if (!rdev->constraints) {
		printk(KERN_ERR "%s: %s has no constraints\n",
		       __func__, rdev->desc->name);
		return ret;
	}

	/* do we need to enable the supply regulator first */
	if (rdev->supply) {
		ret = _regulator_enable(rdev->supply);
		if (ret < 0) {
			printk(KERN_ERR "%s: failed to enable %s: %d\n",
			       __func__, rdev->desc->name, ret);
			return ret;
		}
	}

	/* check voltage and requested load before enabling */
	if (rdev->desc->ops->enable) {

		if (rdev->constraints &&
			(rdev->constraints->valid_ops_mask &
			REGULATOR_CHANGE_DRMS))
			drms_uA_update(rdev);

		ret = rdev->desc->ops->enable(rdev);
		if (ret < 0) {
			printk(KERN_ERR "%s: failed to enable %s: %d\n",
			       __func__, rdev->desc->name, ret);
			return ret;
		}
		rdev->use_count++;
		return ret;
	}

	return ret;
}

/**
 * regulator_enable - enable regulator output
 * @regulator: regulator source
 *
 * Enable the regulator output at the predefined voltage or current value.
 * NOTE: the output value can be set by other drivers, boot loader or may be
 * hardwired in the regulator.
 * NOTE: calls to regulator_enable() must be balanced with calls to
 * regulator_disable().
 */
int regulator_enable(struct regulator *regulator)
{
	int ret;

	if (regulator->enabled) {
		printk(KERN_CRIT "Regulator %s already enabled\n",
		       regulator->supply_name);
		WARN_ON(regulator->enabled);
		return 0;
	}

	mutex_lock(&regulator->rdev->mutex);
	regulator->enabled = 1;
	ret = _regulator_enable(regulator->rdev);
	if (ret != 0)
		regulator->enabled = 0;
	mutex_unlock(&regulator->rdev->mutex);
	return ret;
}
EXPORT_SYMBOL_GPL(regulator_enable);

/* locks held by regulator_disable() */
static int _regulator_disable(struct regulator_dev *rdev)
{
	int ret = 0;

	/* are we the last user and permitted to disable ? */
	if (rdev->use_count == 1 && !rdev->constraints->always_on) {

		/* we are last user */
		if (rdev->desc->ops->disable) {
			ret = rdev->desc->ops->disable(rdev);
			if (ret < 0) {
				printk(KERN_ERR "%s: failed to disable %s\n",
				       __func__, rdev->desc->name);
				return ret;
			}
		}

		/* decrease our supplies ref count and disable if required */
		if (rdev->supply)
			_regulator_disable(rdev->supply);

		rdev->use_count = 0;
	} else if (rdev->use_count > 1) {

		if (rdev->constraints &&
			(rdev->constraints->valid_ops_mask &
			REGULATOR_CHANGE_DRMS))
			drms_uA_update(rdev);

		rdev->use_count--;
	}
	return ret;
}

/**
 * regulator_disable - disable regulator output
 * @regulator: regulator source
 *
 * Disable the regulator output voltage or current.
 * NOTE: this will only disable the regulator output if no other consumer
 * devices have it enabled.
 * NOTE: calls to regulator_enable() must be balanced with calls to
 * regulator_disable().
 */
int regulator_disable(struct regulator *regulator)
{
	int ret;

	if (!regulator->enabled) {
		printk(KERN_ERR "%s: not in use by this consumer\n",
			__func__);
		return 0;
	}

	mutex_lock(&regulator->rdev->mutex);
	regulator->enabled = 0;
	regulator->uA_load = 0;
	ret = _regulator_disable(regulator->rdev);
	mutex_unlock(&regulator->rdev->mutex);
	return ret;
}
EXPORT_SYMBOL_GPL(regulator_disable);

/* locks held by regulator_force_disable() */
static int _regulator_force_disable(struct regulator_dev *rdev)
{
	int ret = 0;

	/* force disable */
	if (rdev->desc->ops->disable) {
		/* ah well, who wants to live forever... */
		ret = rdev->desc->ops->disable(rdev);
		if (ret < 0) {
			printk(KERN_ERR "%s: failed to force disable %s\n",
			       __func__, rdev->desc->name);
			return ret;
		}
		/* notify other consumers that power has been forced off */
		_notifier_call_chain(rdev, REGULATOR_EVENT_FORCE_DISABLE,
			NULL);
	}

	/* decrease our supplies ref count and disable if required */
	if (rdev->supply)
		_regulator_disable(rdev->supply);

	rdev->use_count = 0;
	return ret;
}

/**
 * regulator_force_disable - force disable regulator output
 * @regulator: regulator source
 *
 * Forcibly disable the regulator output voltage or current.
 * NOTE: this *will* disable the regulator output even if other consumer
 * devices have it enabled. This should be used for situations when device
 * damage will likely occur if the regulator is not disabled (e.g. over temp).
 */
int regulator_force_disable(struct regulator *regulator)
{
	int ret;

	mutex_lock(&regulator->rdev->mutex);
	regulator->enabled = 0;
	regulator->uA_load = 0;
	ret = _regulator_force_disable(regulator->rdev);
	mutex_unlock(&regulator->rdev->mutex);
	return ret;
}
EXPORT_SYMBOL_GPL(regulator_force_disable);

static int _regulator_is_enabled(struct regulator_dev *rdev)
{
	int ret;

	mutex_lock(&rdev->mutex);

	/* sanity check */
	if (!rdev->desc->ops->is_enabled) {
		ret = -EINVAL;
		goto out;
	}

	ret = rdev->desc->ops->is_enabled(rdev);
out:
	mutex_unlock(&rdev->mutex);
	return ret;
}

/**
 * regulator_is_enabled - is the regulator output enabled
 * @regulator: regulator source
 *
 * Returns zero for disabled otherwise return number of enable requests.
 */
int regulator_is_enabled(struct regulator *regulator)
{
	return _regulator_is_enabled(regulator->rdev);
}
EXPORT_SYMBOL_GPL(regulator_is_enabled);

/**
 * regulator_set_voltage - set regulator output voltage
 * @regulator: regulator source
 * @min_uV: Minimum required voltage in uV
 * @max_uV: Maximum acceptable voltage in uV
 *
 * Sets a voltage regulator to the desired output voltage. This can be set
 * during any regulator state. IOW, regulator can be disabled or enabled.
 *
 * If the regulator is enabled then the voltage will change to the new value
 * immediately otherwise if the regulator is disabled the regulator will
 * output at the new voltage when enabled.
 *
 * NOTE: If the regulator is shared between several devices then the lowest
 * request voltage that meets the system constraints will be used.
 * NOTE: Regulator system constraints must be set for this regulator before
 * calling this function otherwise this call will fail.
 */
int regulator_set_voltage(struct regulator *regulator, int min_uV, int max_uV)
{
	struct regulator_dev *rdev = regulator->rdev;
	int ret;

	mutex_lock(&rdev->mutex);

	/* sanity check */
	if (!rdev->desc->ops->set_voltage) {
		ret = -EINVAL;
		goto out;
	}

	/* constraints check */
	ret = regulator_check_voltage(rdev, &min_uV, &max_uV);
	if (ret < 0)
		goto out;
	regulator->min_uV = min_uV;
	regulator->max_uV = max_uV;
	ret = rdev->desc->ops->set_voltage(rdev, min_uV, max_uV);

out:
	mutex_unlock(&rdev->mutex);
	return ret;
}
EXPORT_SYMBOL_GPL(regulator_set_voltage);

static int _regulator_get_voltage(struct regulator_dev *rdev)
{
	/* sanity check */
	if (rdev->desc->ops->get_voltage)
		return rdev->desc->ops->get_voltage(rdev);
	else
		return -EINVAL;
}

/**
 * regulator_get_voltage - get regulator output voltage
 * @regulator: regulator source
 *
 * This returns the current regulator voltage in uV.
 *
 * NOTE: If the regulator is disabled it will return the voltage value. This
 * function should not be used to determine regulator state.
 */
int regulator_get_voltage(struct regulator *regulator)
{
	int ret;

	mutex_lock(&regulator->rdev->mutex);

	ret = _regulator_get_voltage(regulator->rdev);

	mutex_unlock(&regulator->rdev->mutex);

	return ret;
}
EXPORT_SYMBOL_GPL(regulator_get_voltage);

/**
 * regulator_set_current_limit - set regulator output current limit
 * @regulator: regulator source
 * @min_uA: Minimuum supported current in uA
 * @max_uA: Maximum supported current in uA
 *
 * Sets current sink to the desired output current. This can be set during
 * any regulator state. IOW, regulator can be disabled or enabled.
 *
 * If the regulator is enabled then the current will change to the new value
 * immediately otherwise if the regulator is disabled the regulator will
 * output at the new current when enabled.
 *
 * NOTE: Regulator system constraints must be set for this regulator before
 * calling this function otherwise this call will fail.
 */
int regulator_set_current_limit(struct regulator *regulator,
			       int min_uA, int max_uA)
{
	struct regulator_dev *rdev = regulator->rdev;
	int ret;

	mutex_lock(&rdev->mutex);

	/* sanity check */
	if (!rdev->desc->ops->set_current_limit) {
		ret = -EINVAL;
		goto out;
	}

	/* constraints check */
	ret = regulator_check_current_limit(rdev, &min_uA, &max_uA);
	if (ret < 0)
		goto out;

	ret = rdev->desc->ops->set_current_limit(rdev, min_uA, max_uA);
out:
	mutex_unlock(&rdev->mutex);
	return ret;
}
EXPORT_SYMBOL_GPL(regulator_set_current_limit);

static int _regulator_get_current_limit(struct regulator_dev *rdev)
{
	int ret;

	mutex_lock(&rdev->mutex);

	/* sanity check */
	if (!rdev->desc->ops->get_current_limit) {
		ret = -EINVAL;
		goto out;
	}

	ret = rdev->desc->ops->get_current_limit(rdev);
out:
	mutex_unlock(&rdev->mutex);
	return ret;
}

/**
 * regulator_get_current_limit - get regulator output current
 * @regulator: regulator source
 *
 * This returns the current supplied by the specified current sink in uA.
 *
 * NOTE: If the regulator is disabled it will return the current value. This
 * function should not be used to determine regulator state.
 */
int regulator_get_current_limit(struct regulator *regulator)
{
	return _regulator_get_current_limit(regulator->rdev);
}
EXPORT_SYMBOL_GPL(regulator_get_current_limit);

/**
 * regulator_set_mode - set regulator operating mode
 * @regulator: regulator source
 * @mode: operating mode - one of the REGULATOR_MODE constants
 *
 * Set regulator operating mode to increase regulator efficiency or improve
 * regulation performance.
 *
 * NOTE: Regulator system constraints must be set for this regulator before
 * calling this function otherwise this call will fail.
 */
int regulator_set_mode(struct regulator *regulator, unsigned int mode)
{
	struct regulator_dev *rdev = regulator->rdev;
	int ret;

	mutex_lock(&rdev->mutex);

	/* sanity check */
	if (!rdev->desc->ops->set_mode) {
		ret = -EINVAL;
		goto out;
	}

	/* constraints check */
	ret = regulator_check_mode(rdev, mode);
	if (ret < 0)
		goto out;

	ret = rdev->desc->ops->set_mode(rdev, mode);
out:
	mutex_unlock(&rdev->mutex);
	return ret;
}
EXPORT_SYMBOL_GPL(regulator_set_mode);

static unsigned int _regulator_get_mode(struct regulator_dev *rdev)
{
	int ret;

	mutex_lock(&rdev->mutex);

	/* sanity check */
	if (!rdev->desc->ops->get_mode) {
		ret = -EINVAL;
		goto out;
	}

	ret = rdev->desc->ops->get_mode(rdev);
out:
	mutex_unlock(&rdev->mutex);
	return ret;
}

/**
 * regulator_get_mode - get regulator operating mode
 * @regulator: regulator source
 *
 * Get the current regulator operating mode.
 */
unsigned int regulator_get_mode(struct regulator *regulator)
{
	return _regulator_get_mode(regulator->rdev);
}
EXPORT_SYMBOL_GPL(regulator_get_mode);

/**
 * regulator_set_optimum_mode - set regulator optimum operating mode
 * @regulator: regulator source
 * @uA_load: load current
 *
 * Notifies the regulator core of a new device load. This is then used by
 * DRMS (if enabled by constraints) to set the most efficient regulator
 * operating mode for the new regulator loading.
 *
 * Consumer devices notify their supply regulator of the maximum power
 * they will require (can be taken from device datasheet in the power
 * consumption tables) when they change operational status and hence power
 * state. Examples of operational state changes that can affect power
 * consumption are :-
 *
 *    o Device is opened / closed.
 *    o Device I/O is about to begin or has just finished.
 *    o Device is idling in between work.
 *
 * This information is also exported via sysfs to userspace.
 *
 * DRMS will sum the total requested load on the regulator and change
 * to the most efficient operating mode if platform constraints allow.
 *
 * Returns the new regulator mode or error.
 */
int regulator_set_optimum_mode(struct regulator *regulator, int uA_load)
{
	struct regulator_dev *rdev = regulator->rdev;
	struct regulator *consumer;
	int ret, output_uV, input_uV, total_uA_load = 0;
	unsigned int mode;

	mutex_lock(&rdev->mutex);

	regulator->uA_load = uA_load;
	ret = regulator_check_drms(rdev);
	if (ret < 0)
		goto out;
	ret = -EINVAL;

	/* sanity check */
	if (!rdev->desc->ops->get_optimum_mode)
		goto out;

	/* get output voltage */
	output_uV = rdev->desc->ops->get_voltage(rdev);
	if (output_uV <= 0) {
		printk(KERN_ERR "%s: invalid output voltage found for %s\n",
			__func__, rdev->desc->name);
		goto out;
	}

	/* get input voltage */
	if (rdev->supply && rdev->supply->desc->ops->get_voltage)
		input_uV = rdev->supply->desc->ops->get_voltage(rdev->supply);
	else
		input_uV = rdev->constraints->input_uV;
	if (input_uV <= 0) {
		printk(KERN_ERR "%s: invalid input voltage found for %s\n",
			__func__, rdev->desc->name);
		goto out;
	}

	/* calc total requested load for this regulator */
	list_for_each_entry(consumer, &rdev->consumer_list, list)
	    total_uA_load += consumer->uA_load;

	mode = rdev->desc->ops->get_optimum_mode(rdev,
						 input_uV, output_uV,
						 total_uA_load);
	if (ret <= 0) {
		printk(KERN_ERR "%s: failed to get optimum mode for %s @"
			" %d uA %d -> %d uV\n", __func__, rdev->desc->name,
			total_uA_load, input_uV, output_uV);
		goto out;
	}

	ret = rdev->desc->ops->set_mode(rdev, mode);
	if (ret <= 0) {
		printk(KERN_ERR "%s: failed to set optimum mode %x for %s\n",
			__func__, mode, rdev->desc->name);
		goto out;
	}
	ret = mode;
out:
	mutex_unlock(&rdev->mutex);
	return ret;
}
EXPORT_SYMBOL_GPL(regulator_set_optimum_mode);

/**
 * regulator_register_notifier - register regulator event notifier
 * @regulator: regulator source
 * @notifier_block: notifier block
 *
 * Register notifier block to receive regulator events.
 */
int regulator_register_notifier(struct regulator *regulator,
			      struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&regulator->rdev->notifier,
						nb);
}
EXPORT_SYMBOL_GPL(regulator_register_notifier);

/**
 * regulator_unregister_notifier - unregister regulator event notifier
 * @regulator: regulator source
 * @notifier_block: notifier block
 *
 * Unregister regulator event notifier block.
 */
int regulator_unregister_notifier(struct regulator *regulator,
				struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&regulator->rdev->notifier,
						  nb);
}
EXPORT_SYMBOL_GPL(regulator_unregister_notifier);

/* notify regulator consumers and downstream regulator consumers */
static void _notifier_call_chain(struct regulator_dev *rdev,
				  unsigned long event, void *data)
{
	struct regulator_dev *_rdev;

	/* call rdev chain first */
	mutex_lock(&rdev->mutex);
	blocking_notifier_call_chain(&rdev->notifier, event, NULL);
	mutex_unlock(&rdev->mutex);

	/* now notify regulator we supply */
	list_for_each_entry(_rdev, &rdev->supply_list, slist)
		_notifier_call_chain(_rdev, event, data);
}

/**
 * regulator_bulk_get - get multiple regulator consumers
 *
 * @dev:           Device to supply
 * @num_consumers: Number of consumers to register
 * @consumers:     Configuration of consumers; clients are stored here.
 *
 * @return 0 on success, an errno on failure.
 *
 * This helper function allows drivers to get several regulator
 * consumers in one operation.  If any of the regulators cannot be
 * acquired then any regulators that were allocated will be freed
 * before returning to the caller.
 */
int regulator_bulk_get(struct device *dev, int num_consumers,
		       struct regulator_bulk_data *consumers)
{
	int i;
	int ret;

	for (i = 0; i < num_consumers; i++)
		consumers[i].consumer = NULL;

	for (i = 0; i < num_consumers; i++) {
		consumers[i].consumer = regulator_get(dev,
						      consumers[i].supply);
		if (IS_ERR(consumers[i].consumer)) {
			dev_err(dev, "Failed to get supply '%s'\n",
				consumers[i].supply);
			ret = PTR_ERR(consumers[i].consumer);
			consumers[i].consumer = NULL;
			goto err;
		}
	}

	return 0;

err:
	for (i = 0; i < num_consumers && consumers[i].consumer; i++)
		regulator_put(consumers[i].consumer);

	return ret;
}
EXPORT_SYMBOL_GPL(regulator_bulk_get);

/**
 * regulator_bulk_enable - enable multiple regulator consumers
 *
 * @num_consumers: Number of consumers
 * @consumers:     Consumer data; clients are stored here.
 * @return         0 on success, an errno on failure
 *
 * This convenience API allows consumers to enable multiple regulator
 * clients in a single API call.  If any consumers cannot be enabled
 * then any others that were enabled will be disabled again prior to
 * return.
 */
int regulator_bulk_enable(int num_consumers,
			  struct regulator_bulk_data *consumers)
{
	int i;
	int ret;

	for (i = 0; i < num_consumers; i++) {
		ret = regulator_enable(consumers[i].consumer);
		if (ret != 0)
			goto err;
	}

	return 0;

err:
	printk(KERN_ERR "Failed to enable %s\n", consumers[i].supply);
	for (i = 0; i < num_consumers; i++)
		regulator_disable(consumers[i].consumer);

	return ret;
}
EXPORT_SYMBOL_GPL(regulator_bulk_enable);

/**
 * regulator_bulk_disable - disable multiple regulator consumers
 *
 * @num_consumers: Number of consumers
 * @consumers:     Consumer data; clients are stored here.
 * @return         0 on success, an errno on failure
 *
 * This convenience API allows consumers to disable multiple regulator
 * clients in a single API call.  If any consumers cannot be enabled
 * then any others that were disabled will be disabled again prior to
 * return.
 */
int regulator_bulk_disable(int num_consumers,
			   struct regulator_bulk_data *consumers)
{
	int i;
	int ret;

	for (i = 0; i < num_consumers; i++) {
		ret = regulator_disable(consumers[i].consumer);
		if (ret != 0)
			goto err;
	}

	return 0;

err:
	printk(KERN_ERR "Failed to disable %s\n", consumers[i].supply);
	for (i = 0; i < num_consumers; i++)
		regulator_enable(consumers[i].consumer);

	return ret;
}
EXPORT_SYMBOL_GPL(regulator_bulk_disable);

/**
 * regulator_bulk_free - free multiple regulator consumers
 *
 * @num_consumers: Number of consumers
 * @consumers:     Consumer data; clients are stored here.
 *
 * This convenience API allows consumers to free multiple regulator
 * clients in a single API call.
 */
void regulator_bulk_free(int num_consumers,
			 struct regulator_bulk_data *consumers)
{
	int i;

	for (i = 0; i < num_consumers; i++) {
		regulator_put(consumers[i].consumer);
		consumers[i].consumer = NULL;
	}
}
EXPORT_SYMBOL_GPL(regulator_bulk_free);

/**
 * regulator_notifier_call_chain - call regulator event notifier
 * @regulator: regulator source
 * @event: notifier block
 * @data:
 *
 * Called by regulator drivers to notify clients a regulator event has
 * occurred. We also notify regulator clients downstream.
 */
int regulator_notifier_call_chain(struct regulator_dev *rdev,
				  unsigned long event, void *data)
{
	_notifier_call_chain(rdev, event, data);
	return NOTIFY_DONE;

}
EXPORT_SYMBOL_GPL(regulator_notifier_call_chain);

/**
 * regulator_register - register regulator
 * @regulator: regulator source
 * @reg_data: private regulator data
 *
 * Called by regulator drivers to register a regulator.
 * Returns 0 on success.
 */
struct regulator_dev *regulator_register(struct regulator_desc *regulator_desc,
	struct device *dev)
{
	struct regulator_dev *rdev;
	struct regulator_init_data *init_data = dev->platform_data;
	int ret, i;

	if (regulator_desc == NULL)
		return ERR_PTR(-EINVAL);

	if (regulator_desc->name == NULL || regulator_desc->ops == NULL)
		return ERR_PTR(-EINVAL);

	if (!regulator_desc->type == REGULATOR_VOLTAGE &&
	    !regulator_desc->type == REGULATOR_CURRENT)
		return ERR_PTR(-EINVAL);

	rdev = kzalloc(sizeof(struct regulator_dev), GFP_KERNEL);
	if (rdev == NULL)
		return ERR_PTR(-ENOMEM);

	mutex_lock(&regulator_list_mutex);

	mutex_init(&rdev->mutex);
	rdev->reg_data = dev_get_drvdata(dev);
	rdev->owner = regulator_desc->owner;
	rdev->desc = regulator_desc;
	rdev->dev = dev;
	INIT_LIST_HEAD(&rdev->consumer_list);
	INIT_LIST_HEAD(&rdev->supply_list);
	INIT_LIST_HEAD(&rdev->list);
	INIT_LIST_HEAD(&rdev->slist);
	BLOCKING_INIT_NOTIFIER_HEAD(&rdev->notifier);

	/* preform any regulator specific init */
	if (init_data->regulator_init) {
		ret = init_data->regulator_init(rdev->reg_data);
		if (ret < 0) {
			kfree(rdev);
			rdev = ERR_PTR(ret);
			goto out;
		}
	}

	/* set regulator constraints */
	ret = set_machine_constraints(rdev, &init_data->constraints);
	if (ret < 0) {
		kfree(rdev);
		rdev = ERR_PTR(ret);
		goto out;
	}

	/* set supply regulator if it exists */
	if (init_data->supply_regulator_dev) {
		ret = set_supply(rdev,
			dev_get_drvdata(init_data->supply_regulator_dev));
		if (ret < 0) {
			kfree(rdev);
			rdev = ERR_PTR(ret);
			goto out;
		}
	}

	/* add consumers devices */
	for (i = 0; i < init_data->num_consumer_supplies; i++) {
		ret = set_consumer_device_supply(rdev,
			init_data->consumer_supplies[i].dev,
			init_data->consumer_supplies[i].supply);
		/* TODO: err rewind */
	}

	rdev->dev->class = &regulator_class;
	list_add(&rdev->list, &regulator_list);

out:
	mutex_unlock(&regulator_list_mutex);
	return rdev;
}
EXPORT_SYMBOL_GPL(regulator_register);

/**
 * regulator_unregister - unregister regulator
 * @regulator: regulator source
 *
 * Called by regulator drivers to unregister a regulator.
 */
void regulator_unregister(struct regulator_dev *rdev)
{
	if (rdev == NULL)
		return;

	mutex_lock(&regulator_list_mutex);
	list_del(&rdev->list);
	if (rdev->supply)
		sysfs_remove_link(&rdev->dev->kobj, "supply");
//	device_unregister(rdev->dev);
	mutex_unlock(&regulator_list_mutex);
}
EXPORT_SYMBOL_GPL(regulator_unregister);

/**
 * regulator_suspend_prepare: prepare regulators for system wide suspend
 * @state: system suspend state
 *
 * Configure each regulator with it's suspend operating parameters for state.
 * This will usually be called by machine suspend code prior to supending.
 */
int regulator_suspend_prepare(suspend_state_t state)
{
	struct regulator_dev *rdev;
	int ret = 0;

	/* ON is handled by regulator active state */
	if (state == PM_SUSPEND_ON)
		return -EINVAL;

	mutex_lock(&regulator_list_mutex);
	list_for_each_entry(rdev, &regulator_list, list) {

		mutex_lock(&rdev->mutex);
		ret = suspend_prepare(rdev, state);
		mutex_unlock(&rdev->mutex);

		if (ret < 0) {
			printk(KERN_ERR "%s: failed to prepare %s\n",
				__func__, rdev->desc->name);
			goto out;
		}
	}
out:
	mutex_unlock(&regulator_list_mutex);
	return ret;
}
EXPORT_SYMBOL_GPL(regulator_suspend_prepare);

/**
 * rdev_get_drvdata - get rdev regulator driver data
 * @regulator: regulator
 *
 * Get rdev regulator driver private data. This call can be used in the
 * regulator driver context.
 */
void *rdev_get_drvdata(struct regulator_dev *rdev)
{
	return rdev->reg_data;
}
EXPORT_SYMBOL_GPL(rdev_get_drvdata);

/**
 * regulator_get_drvdata - get regulator driver data
 * @regulator: regulator
 *
 * Get regulator driver private data. This call can be used in the consumer
 * driver context when non API regulator specific functions need to be called.
 */
void *regulator_get_drvdata(struct regulator *regulator)
{
	return regulator->rdev->reg_data;
}
EXPORT_SYMBOL_GPL(regulator_get_drvdata);

/**
 * regulator_set_drvdata - set regulator driver data
 * @regulator: regulator
 * @data: data
 */
void regulator_set_drvdata(struct regulator *regulator, void *data)
{
	regulator->rdev->reg_data = data;
}
EXPORT_SYMBOL_GPL(regulator_set_drvdata);

/**
 * regulator_get_id - get regulator ID
 * @regulator: regulator
 */
int rdev_get_id(struct regulator_dev *rdev)
{
	return rdev->desc->id;
}
EXPORT_SYMBOL_GPL(rdev_get_id);

void *regulator_get_init_drvdata(struct regulator_init_data *reg_init_data)
{
	return reg_init_data->driver_data;
}
EXPORT_SYMBOL_GPL(regulator_get_init_drvdata);

static int __init regulator_init(void)
{
	printk(KERN_INFO "regulator: core version %s\n", REGULATOR_VERSION);
	return class_register(&regulator_class);
}

/* init early to allow our consumers to complete system booting */
core_initcall(regulator_init);
