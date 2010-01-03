/*
 * Bus driver for WM8400
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
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/mfd/wm8400/bus.h>
#include <linux/mfd/wm8400/registers.h>

struct
{
	u16  readable;    /* Mask of readable bits */
	u16  writable;    /* Mask of writable bits */
	u16  vol;         /* Mask of volatile bits */
	int  is_codec;    /* Register controlled by codec reset */
	u16  default_val; /* Value on reset */
} reg_data[] =
{
	{ 0xFFFF, 0xFFFF, 0x0000, 0, 0x6172 }, /* R0 */
	{ 0x7000, 0x0000, 0x8000, 0, 0x0000 }, /* R1 */
	{ 0xFF17, 0xFF17, 0x0000, 0, 0x0000 }, /* R2 */
	{ 0xEBF3, 0xEBF3, 0x0000, 1, 0x6000 }, /* R3 */
	{ 0x3CF3, 0x3CF3, 0x0000, 1, 0x0000 }, /* R4  */
	{ 0xF1F8, 0xF1F8, 0x0000, 1, 0x4050 }, /* R5  */
	{ 0xFC1F, 0xFC1F, 0x0000, 1, 0x4000 }, /* R6  */
	{ 0xDFDE, 0xDFDE, 0x0000, 1, 0x01C8 }, /* R7  */
	{ 0xFCFC, 0xFCFC, 0x0000, 1, 0x0000 }, /* R8  */
	{ 0xEFFF, 0xEFFF, 0x0000, 1, 0x0040 }, /* R9  */
	{ 0xEFFF, 0xEFFF, 0x0000, 1, 0x0040 }, /* R10 */
	{ 0x27F7, 0x27F7, 0x0000, 1, 0x0004 }, /* R11 */
	{ 0x01FF, 0x01FF, 0x0000, 1, 0x00C0 }, /* R12 */
	{ 0x01FF, 0x01FF, 0x0000, 1, 0x00C0 }, /* R13 */
	{ 0x1FEF, 0x1FEF, 0x0000, 1, 0x0000 }, /* R14 */
	{ 0x0163, 0x0163, 0x0000, 1, 0x0100 }, /* R15 */
	{ 0x01FF, 0x01FF, 0x0000, 1, 0x00C0 }, /* R16 */
	{ 0x01FF, 0x01FF, 0x0000, 1, 0x00C0 }, /* R17 */
	{ 0x1FFF, 0x0FFF, 0x0000, 1, 0x0000 }, /* R18 */
	{ 0xFFFF, 0xFFFF, 0x0000, 1, 0x1000 }, /* R19 */
	{ 0xFFFF, 0xFFFF, 0x0000, 1, 0x1010 }, /* R20 */
	{ 0xFFFF, 0xFFFF, 0x0000, 1, 0x1010 }, /* R21 */
	{ 0x0FDD, 0x0FDD, 0x0000, 1, 0x8000 }, /* R22 */
	{ 0x1FFF, 0x1FFF, 0x0000, 1, 0x0800 }, /* R23 */
	{ 0x0000, 0x01DF, 0x0000, 1, 0x008B }, /* R24 */
	{ 0x0000, 0x01DF, 0x0000, 1, 0x008B }, /* R25 */
	{ 0x0000, 0x01DF, 0x0000, 1, 0x008B }, /* R26 */
	{ 0x0000, 0x01DF, 0x0000, 1, 0x008B }, /* R27 */
	{ 0x0000, 0x01FF, 0x0000, 1, 0x0000 }, /* R28 */
	{ 0x0000, 0x01FF, 0x0000, 1, 0x0000 }, /* R29 */
	{ 0x0000, 0x0077, 0x0000, 1, 0x0066 }, /* R30 */
	{ 0x0000, 0x0033, 0x0000, 1, 0x0022 }, /* R31 */
	{ 0x0000, 0x01FF, 0x0000, 1, 0x0079 }, /* R32 */
	{ 0x0000, 0x01FF, 0x0000, 1, 0x0079 }, /* R33 */
	{ 0x0000, 0x0003, 0x0000, 1, 0x0003 }, /* R34 */
	{ 0x0000, 0x01FF, 0x0000, 1, 0x0003 }, /* R35 */
	{ 0x0000, 0x0000, 0x0000, 0, 0x0000 }, /* R36 */
	{ 0x0000, 0x003F, 0x0000, 1, 0x0100 }, /* R37 */
	{ 0x0000, 0x0000, 0x0000, 0, 0x0000 }, /* R38 */
	{ 0x0000, 0x000F, 0x0000, 0, 0x0000 }, /* R39 */
	{ 0x0000, 0x00FF, 0x0000, 1, 0x0000 }, /* R40 */
	{ 0x0000, 0x01B7, 0x0000, 1, 0x0000 }, /* R41 */
	{ 0x0000, 0x01B7, 0x0000, 1, 0x0000 }, /* R42 */
	{ 0x0000, 0x01FF, 0x0000, 1, 0x0000 }, /* R43 */
	{ 0x0000, 0x01FF, 0x0000, 1, 0x0000 }, /* R44 */
	{ 0x0000, 0x00FD, 0x0000, 1, 0x0000 }, /* R45 */
	{ 0x0000, 0x00FD, 0x0000, 1, 0x0000 }, /* R46 */
	{ 0x0000, 0x01FF, 0x0000, 1, 0x0000 }, /* R47 */
	{ 0x0000, 0x01FF, 0x0000, 1, 0x0000 }, /* R48 */
	{ 0x0000, 0x01FF, 0x0000, 1, 0x0000 }, /* R49 */
	{ 0x0000, 0x01FF, 0x0000, 1, 0x0000 }, /* R50 */
	{ 0x0000, 0x01B3, 0x0000, 1, 0x0180 }, /* R51 */
	{ 0x0000, 0x0077, 0x0000, 1, 0x0000 }, /* R52 */
	{ 0x0000, 0x0077, 0x0000, 1, 0x0000 }, /* R53 */
	{ 0x0000, 0x00FF, 0x0000, 1, 0x0000 }, /* R54 */
	{ 0x0000, 0x0001, 0x0000, 1, 0x0000 }, /* R55 */
	{ 0x0000, 0x003F, 0x0000, 1, 0x0000 }, /* R56 */
	{ 0x0000, 0x004F, 0x0000, 1, 0x0000 }, /* R57 */
	{ 0x0000, 0x00FD, 0x0000, 1, 0x0000 }, /* R58 */
	{ 0x0000, 0x0000, 0x0000, 0, 0x0000 }, /* R59 */
	{ 0x1FFF, 0x1FFF, 0x0000, 1, 0x0000 }, /* R60 */
	{ 0xFFFF, 0xFFFF, 0x0000, 1, 0x0000 }, /* R61 */
	{ 0x03FF, 0x03FF, 0x0000, 1, 0x0000 }, /* R62 */
	{ 0x007F, 0x007F, 0x0000, 1, 0x0000 }, /* R63 */
	{ 0x0000, 0x0000, 0x0000, 0, 0x0000 }, /* R64 */
	{ 0xDFFF, 0xDFFF, 0x0000, 0, 0x0000 }, /* R65 */
	{ 0xDFFF, 0xDFFF, 0x0000, 0, 0x0000 }, /* R66 */
	{ 0xDFFF, 0xDFFF, 0x0000, 0, 0x0000 }, /* R67 */
	{ 0xDFFF, 0xDFFF, 0x0000, 0, 0x0000 }, /* R68 */
	{ 0x0000, 0x0000, 0x0000, 0, 0x0000 }, /* R69 */
	{ 0xFFFF, 0xFFFF, 0x0000, 0, 0x4400 }, /* R70 */
	{ 0x23FF, 0x23FF, 0x0000, 0, 0x0000 }, /* R71 */
	{ 0xFFFF, 0xFFFF, 0x0000, 0, 0x4400 }, /* R72 */
	{ 0x23FF, 0x23FF, 0x0000, 0, 0x0000 }, /* R73 */
	{ 0x0000, 0x0000, 0x0000, 0, 0x0000 }, /* R74 */
	{ 0x000E, 0x000E, 0x0000, 0, 0x0008 }, /* R75 */
	{ 0xE00F, 0xE00F, 0x0000, 0, 0x0000 }, /* R76 */
	{ 0x0000, 0x0000, 0x0000, 0, 0x0000 }, /* R77 */
	{ 0x03C0, 0x03C0, 0x0000, 0, 0x02C0 }, /* R78 */
	{ 0xFFFF, 0x0000, 0xffff, 0, 0x0000 }, /* R79 */
	{ 0xFFFF, 0xFFFF, 0x0000, 0, 0x0000 }, /* R80 */
	{ 0xFFFF, 0x0000, 0xffff, 0, 0x0000 }, /* R81 */
	{ 0x2BFF, 0x0000, 0xffff, 0, 0x0000 }, /* R82 */
	{ 0x0000, 0x0000, 0x0000, 0, 0x0000 }, /* R83 */
	{ 0x80FF, 0x80FF, 0x0000, 0, 0x00ff }, /* R84 */
};

#ifdef CONFIG_I2C
static int wm8400_read_i2c_device(struct wm8400 *wm8400, char reg,
				  int bytes, char *dest)
{
	int ret;

	BUG_ON(reg + (bytes / 2) > ARRAY_SIZE(wm8400->reg_cache));

	ret = i2c_master_send(wm8400->i2c_client, &reg, 1);
	if (ret < 0)
		return ret;
	return i2c_master_recv(wm8400->i2c_client, dest, bytes);
}

static int wm8400_write_i2c_device(struct wm8400 *wm8400, char reg,
				   int bytes, const char *src)
{
	/* we add 1 byte for device register */
	u8 msg[(WM8400_MAX_REGISTER << 1) + 1];

	if (bytes > ((WM8400_MAX_REGISTER << 1) + 1))
		return -EINVAL;

	msg[0] = reg;
	memcpy(&msg[1], src, bytes);
	return i2c_master_send(wm8400->i2c_client, msg, bytes + 1) - 1;
}
#endif

#ifdef CONFIG_SPI
static int wm8400_read_spi_device(struct wm8400 *wm8400, char reg,
				  int bytes, char *dest)
{
	int ret;
	u8 tx_msg[2], rx_msg[2];
	int done;

	BUG_ON(bytes % 2);

	/* don't support incremental read with SPI */
	if (bytes > 2) {
		done = 0;
		while (bytes) {
			ret = wm8400_read_spi_device(wm8400, reg, 2, dest);
			if (ret != 2)
				return ret;

			done += 2;
			bytes -= 2;
			dest += 2;
			reg++;
		}

		return done;
	}

	BUG_ON(reg > ARRAY_SIZE(wm8400->reg_cache));

	tx_msg[0] = 0x80;
	tx_msg[1] = reg;

	ret = spi_write_then_read(wm8400->spi_device, tx_msg, sizeof(tx_msg),
				  rx_msg, sizeof(tx_msg));
	if (ret < 0) {
		printk(KERN_ERR "%s: io failure %d\n", __func__, ret);
		return ret;
	}

	*dest++ = rx_msg[2];
	*dest = rx_msg[3];
	return 2;
}

static int wm8400_write_spi_device(struct wm8400 *wm8400, char reg,
				   int bytes, const char *src)
{
	u8 msg[4];
	int done;

	BUG_ON(bytes % 2);
	BUG_ON(reg > ARRAY_SIZE(wm8400->reg_cache));

	/* don't support incremental write with SPI */
	if (bytes > 2) {
		done = 0;
		while (bytes) {
			ret = wm8400_write_spi_device(wm8400, reg, 2, src);
			if (ret != 2)
				return ret;

			bytes -= 2;
			done += 2;
			src += 2;
			reg++;
		}

		return done;
	}

	msg[0] = 0;
	msg[1] = reg;
	msg[2] = *src++;
	msg[3] = *src;
	return spi_write(wm8400->spi_device, msg, sizeof(msg));
}
#endif

static int wm8400_read(struct wm8400 *wm8400, u8 reg, int num_regs, u16 *dest)
{
	int i, ret = 0;
	int bytes = num_regs * 2;

	if (wm8400->read_dev == NULL)
		return -ENODEV;

	BUG_ON(reg + num_regs - 1 > ARRAY_SIZE(wm8400->reg_cache));

	/* If there are any volatile reads then read back the entire block */
	for (i = reg; i < reg + num_regs; i++)
		if (reg_data[i].vol) {
			ret = wm8400->read_dev(wm8400, reg, bytes,
					       (char *)dest);
			if (ret != bytes)
				return -EIO;
			for (i = 0; i < num_regs; i++)
				dest[i] = be16_to_cpu(dest[i]);

			return 0;
		}

	/* Otherwise use the cache */
	memcpy(dest, &wm8400->reg_cache[reg], num_regs * 2);

	return 0;
}

static int wm8400_write(struct wm8400 *wm8400, u8 reg, int num_regs,
			u16 *src)
{
	int ret, i;

	if (wm8400->write_dev == NULL)
		return -ENODEV;

	BUG_ON(reg + num_regs - 1 > ARRAY_SIZE(wm8400->reg_cache));

	for (i = 0; i < num_regs; i++) {
		BUG_ON(!reg_data[reg + i].writable);
		wm8400->reg_cache[reg + i] = src[i];
		src[i] = cpu_to_be16(src[i]);
	}

	/* Do the actual I/O */
	ret = wm8400->write_dev(wm8400, reg, num_regs * 2, (char *)src);
	if (ret != num_regs * 2)
		return -EIO;

	return 0;
}

/**
 * wm8400_reg_read - Single register read
 *
 * @wm8400: Pointer to wm8400 control structure
 * @reg:    Register to read
 *
 * @return  Read value
 */
u16 wm8400_reg_read(struct wm8400 *wm8400, u8 reg)
{
	u16 val;

	mutex_lock(&wm8400->io_lock);
	wm8400_read(wm8400, reg, 1, &val);
	mutex_unlock(&wm8400->io_lock);

	return val;
}
EXPORT_SYMBOL_GPL(wm8400_reg_read);

int wm8400_block_read(struct wm8400 *wm8400, u8 reg, int count, u16 *data)
{
	int ret;

	mutex_lock(&wm8400->io_lock);

	ret = wm8400_read(wm8400, reg, count, data);

	mutex_unlock(&wm8400->io_lock);

	return ret;
}
EXPORT_SYMBOL_GPL(wm8400_block_read);

/**
 * wm8400_set_bits - Bitmask write
 *
 * @wm8400: Pointer to wm8400 control structure
 * @reg:    Register to access
 * @mask:   Mask of bits to change
 * @val:    Value to set for masked bits
 */
int wm8400_set_bits(struct wm8400 *wm8400, u8 reg, u16 mask, u16 val)
{
	u16 tmp;
	int ret;

	mutex_lock(&wm8400->io_lock);

	ret = wm8400_read(wm8400, reg, 1, &tmp);
	tmp = (tmp & ~mask) | val;
	if (ret == 0)
		ret = wm8400_write(wm8400, reg, 1, &tmp);

	mutex_unlock(&wm8400->io_lock);

	return ret;
}
EXPORT_SYMBOL_GPL(wm8400_set_bits);

/**
 * wm8400_stop_codec - Place the codec subsystem in reset
 */
void wm8400_stop_codec(struct wm8400 *wm8400)
{
	u16 val;
	int i;

	mutex_lock(&wm8400->io_lock);

	val = wm8400->reg_cache[WM8400_POWER_MANAGEMENT_1] & ~WM8400_CODEC_ENA;
	wm8400_write(wm8400, WM8400_POWER_MANAGEMENT_1, 1, &val);

	/* Reset all codec registers to their initial value */
	for (i = 0; i < ARRAY_SIZE(wm8400->reg_cache); i++)
		if (reg_data[i].is_codec)
			wm8400->reg_cache[i] = reg_data[i].default_val;

	mutex_unlock(&wm8400->io_lock);
}
EXPORT_SYMBOL_GPL(wm8400_stop_codec);

int wm8400_start_codec(struct wm8400 *wm8400)
{
	return wm8400_set_bits(wm8400, WM8400_POWER_MANAGEMENT_1,
			       WM8400_CODEC_ENA, WM8400_CODEC_ENA);
}
EXPORT_SYMBOL_GPL(wm8400_start_codec);

static int wm8400_bus_match(struct device *dev, struct device_driver *drv)
{
	if (!strcmp(dev->bus_id, drv->name))
		return 1;
	return 0;
}

static int wm8400_bus_suspend(struct device *dev, pm_message_t state)
{
	int ret = 0;

	if (dev->driver && dev->driver->suspend)
		ret = dev->driver->suspend(dev, state);

	return ret;
}

static int wm8400_bus_resume(struct device *dev)
{
	int ret = 0;

	if (dev->driver && dev->driver->resume)
		ret = dev->driver->resume(dev);

	return ret;
}

static void wm8400_bus_release(struct device *dev)
{
}

struct bus_type wm8400_bus_type = {
	.name = "wm8400",
	.match = wm8400_bus_match,
	.suspend = wm8400_bus_suspend,
	.resume = wm8400_bus_resume,
};
EXPORT_SYMBOL_GPL(wm8400_bus_type);

static int wm8400_register_device(struct wm8400 *wm8400, struct device *dev,
				  const char *name)
{
	int ret;

	strcpy(dev->bus_id, name);
	dev->bus = &wm8400_bus_type;
	dev->parent = wm8400->dev;
	dev->release = wm8400_bus_release;

	dev_err(wm8400->dev, "Registering with parent\n");

	ret = device_register(dev);
	if (ret != 0) {
		printk(KERN_ERR "wm8400: Failed to register %s\n", name);
		return ret;
	}

	return 0;
}

/**
 * wm8400_init - Initialisation of the driver
 *
 * The WM8400 supports both I2C and SPI control.  This function, which must
 * be called prior to any other WM8400 bus operation, is
 */
int wm8400_init(struct wm8400 *wm8400, int io,
		int (*read_dev)(struct wm8400 *wm8400, char reg, int size,
				char *dest),
		int (*write_dev)(struct wm8400 *wm8400, char reg, int size,
				 const char *src))
{
	u16 reg;
	int ret, i;

	mutex_init(&wm8400->io_lock);

	switch (io) {
#if defined(CONFIG_I2C) || defined(CONFIG_I2C_MODULE)
	case WM8400_IO_I2C:
		wm8400->read_dev = wm8400_read_i2c_device;
		wm8400->write_dev = wm8400_write_i2c_device;
		wm8400->dev = &wm8400->i2c_client->dev;
		break;
#endif
#if defined(CONFIG_SPI) || defined(CONFIG_SPI_MODULE)
	case WM8400_IO_SPI:
		wm8400->read_dev = wm8400_read_spi_device;
		wm8400->write_dev = wm8400_write_spi_device;
		wm8400->dev = &wm8400->spi_client->dev;
		break;
#endif
	case WM8400_IO_CUSTOM:
		wm8400->read_dev = read_dev;
		wm8400->write_dev = write_dev;
		break;
	default:
		printk(KERN_ERR "wm8400: Unknown I/O mechansim %d selected\n",
		       io);
		wm8400->read_dev = NULL;
		wm8400->write_dev = NULL;
		return -EINVAL;
	}

	/* Check that this is actually a WM8400 */
	ret = wm8400->read_dev(wm8400, WM8400_RESET_ID, 2, (char *)&reg);
	if (ret != 2) {
		dev_err(wm8400->dev, "Chip ID register read failed\n");
		return -EIO;
	}
	if (be16_to_cpu(reg) != 0x6172) {
		dev_err(wm8400->dev, "Device is not a WM8400\n");
		return -ENODEV;
	}

	/* We don't know what state the hardware is in and since this
	 * is a PMIC we can't reset it.
	 */
	ret = wm8400->read_dev(wm8400, 0, sizeof(wm8400->reg_cache),
			       (char *)&wm8400->reg_cache);
	if (ret != sizeof(wm8400->reg_cache)) {
		dev_err(wm8400->dev, "Register cache read failed\n");
		return -EIO;
	}
	for (i = 0; i < ARRAY_SIZE(wm8400->reg_cache); i++) {
		wm8400->reg_cache[i] = be16_to_cpu(wm8400->reg_cache[i]);
	}

	/* If the codec is in reset use hard coded values */
	if (!(wm8400->reg_cache[WM8400_POWER_MANAGEMENT_1] & WM8400_CODEC_ENA))
		for (i = 0; i < ARRAY_SIZE(wm8400->reg_cache); i++)
			if (reg_data[i].is_codec)
				wm8400->reg_cache[i] = reg_data[i].default_val;

	ret = wm8400_read(wm8400, WM8400_ID, 1, &reg);
	if (ret != 0) {
		dev_err(wm8400->dev, "ID register read failed: %d\n", ret);
		return ret;
	}
	reg = (reg & WM8400_CHIP_REV_MASK) >> WM8400_CHIP_REV_SHIFT;
	dev_info(wm8400->dev, "WM8400 revision %x\n", reg);

	/* We've got the chip, now register client devices */
	ret = wm8400_register_device(wm8400, &wm8400->regulator.dev,
				     "wm8400-regulator");

	ret = wm8400_register_device(wm8400, &wm8400->codec,
				     "wm8400-codec");

	return 0;
}
EXPORT_SYMBOL_GPL(wm8400_init);

void wm8400_exit(struct wm8400 *wm8400)
{
	device_unregister(&wm8400->regulator.dev);
}
EXPORT_SYMBOL_GPL(wm8400_exit);


static int __init wm8400_module_init(void)
{
	int ret;
	ret = bus_register(&wm8400_bus_type);
	if (ret != 0)
		goto err;

	ret = wm8400_regulator_init();
	if (ret != 0)
		goto err_bus;

	return 0;

err_bus:
	bus_unregister(&wm8400_bus_type);
err:
	return ret;
}
module_init(wm8400_module_init);

static void __exit wm8400_module_exit(void)
{
	wm8400_regulator_exit();
	bus_unregister(&wm8400_bus_type);
}
module_exit(wm8400_module_exit);

MODULE_LICENSE("GPL");
