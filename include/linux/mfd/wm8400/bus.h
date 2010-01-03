/*
 * wm8400 bus interface.
 *
 * Copyright 2007 Wolfson Microelectronics plc
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __LINUX_MFD_WM8400_BUS_H
#define __LINUX_MFD_WM8400_BUS_H

#include <linux/i2c.h>
#include <linux/spi/spi.h>

#define WM8400_MAX_REGISTER 0x54

struct regulator_dev;

struct wm8400_regulator {
	struct device dev;
	struct regulator_dev *rdev[6];
};

struct wm8400 {
#if defined(CONFIG_I2C) || defined(CONFIG_I2C_MODULE)
	struct i2c_client *i2c_client;
#endif
#if defined(CONFIG_SPI) || defined(CONFIG_SPI_MODULE)
	struct spi_client *spi_client;
#endif

	struct device *dev;

	int (*read_dev)(struct wm8400 *wm8400, char reg, int size, char *dest);
	int (*write_dev)(struct wm8400 *wm8400, char reg, int size,
			 const char *src);

	struct wm8400_regulator regulator;
	struct device codec;

	struct mutex io_lock;
	u16 reg_cache[WM8400_MAX_REGISTER + 1];
};

enum wm8400_io_mode {
	WM8400_IO_I2C = 1,
	WM8400_IO_SPI,
	WM8400_IO_CUSTOM,
};

#define WM8400_LDO1 0
#define WM8400_LDO2 1
#define WM8400_LDO3 2
#define WM8400_LDO4 3

#define WM8400_DCDC1 0
#define WM8400_DCDC2 1

int wm8400_init(struct wm8400 *wm8400, int io,
		int (*read_dev)(struct wm8400 *wm8400, char reg, int size,
				char *dest),
		int (*write_dev)(struct wm8400 *wm8400, char reg, int size,
				 const char *src));
void wm8400_exit(struct wm8400 *wm8400);

#endif
