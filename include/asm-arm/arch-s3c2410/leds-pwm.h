/*-----------------------------------------------------------------------------
 * FILE NAME : leds-pwm.h
 * 
 * PURPOSE : LEDs PWM driver for Canopus
 * 
 * Copyright 1999 - 2010 UniData Communication Systems, Inc.
 * All right reserved. 
 * 
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 *
 * NOTES: N/A
 *---------------------------------------------------------------------------*/

#ifndef __ASM_ARCH_LEDS_PWM_H
#define __ASM_ARCH_LEDS_PWM_H "leds-pwm.h"

struct s3c_pwm_led_platdata {
	unsigned int timer;
	int brightness;
	bool invert;

	char *name;
	char *def_trigger;
};

#endif /* __ASM_ARCH_LEDS_PWM_H */

