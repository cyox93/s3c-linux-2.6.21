/*
 * wdt.h  --  Watchdog Driver for Wolfson WM8350 PMIC
 *
 * Copyright 2007 Wolfson Microelectronics PLC
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  Revision history
 *    8th Feb 2007   Initial version.
 *
 */

#ifndef __LINUX_MFD_WM8350_WDT_H_
#define __LINUX_MFD_WM8350_WDT_H_

#include <linux/device.h>

#define WM8350_WDOG_HIB_MODE                    0x0080
#define WM8350_WDOG_DEBUG                       0x0040
#define WM8350_WDOG_MODE_MASK                   0x0030
#define WM8350_WDOG_TO_MASK                     0x0007

#define WM8350_IRQ_SYS_WDOG_TO			24

struct wm8350_wdg {
	struct device dev;
};
#define to_wm8350_wdg_device(d) container_of(d, struct wm8350_wdg, dev)

#endif
