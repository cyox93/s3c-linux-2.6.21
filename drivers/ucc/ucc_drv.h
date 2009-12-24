/*-----------------------------------------------------------------------------
 * FILE NAME : ucc_dev.h
 * 
 * PURPOSE : ucc driver for WPU-7800 (make carrier by it self)
 * 
 * Copyright ⓒ 1999 - 2007 UniData Communication Systems, Inc.
 * All right reserved. 
 * 
 * This software is confidential and proprietary to UniData 
 * Communication Systems, Inc. No Part of this software may 
 * be reproduced, stored, transmitted, disclosed or used in any
 * form or by any means other than as expressly provide by the 
 * written license agreement between UniData Communication 
 * Systems and its licensee.
 *
 * NOTES: 
 *
 *---------------------------------------------------------------------------*/

#define IOCTLUCC_PM_MODE				_IOWR('U', 0xc0, int) 
#define IOCTLUCC_POWER				_IOWR('U', 0xc1, int) 
#define IOCTLUCC_LCD					_IOWR('U', 0xc2, int) 
#define IOCTLUCC_BACKLIGHT			_IOWR('U', 0xc3, int) 
#define IOCTLUCC_CAMERA				_IOWR('U', 0xc4, int) 
#define IOCTLUCC_WLAN					_IOWR('U', 0xc5, int) 
#define IOCTLUCC_AUDIO				_IOWR('U', 0xc6, int) 
#define IOCTLUCC_KEY_LED				_IOWR('U', 0xc7, int) 
#define IOCTLUCC_VIBRATOR				_IOWR('U', 0xc8, int) 
#define IOCTLUCC_CHARGER				_IOWR('U', 0xc9, int) 

#define IOCTLUCC_CPU_REG_READ		_IOWR('U', 0xca, int) 
#define IOCTLUCC_CPU_REG_WRITE		_IOWR('U', 0xcb, int) 
#define IOCTLUCC_PMIC_REG_READ		_IOWR('U', 0xcc, int) 
#define IOCTLUCC_PMIC_REG_WRITE		_IOWR('U', 0xcd, int) 

#define IOCTLUCC_INIT					_IOWR('U', 0xce, int) 
#define IOCTLUCC_DEINIT				_IOWR('U', 0xcf, int) 

#define IOCTLUCC_KEEPALIVE				_IOWR('U', 0xd0, int) 
#define IOCTLUCC_WDOG_START				_IOWR('U', 0xd1, int) 
#define IOCTLUCC_WDOG_STOP				_IOWR('U', 0xd2, int) 

/*_____________________ Type definitions ____________________________________*/
typedef struct
{
		unsigned char opt1;
		unsigned char opt2;
		unsigned char opt3;	
		unsigned long reg_addr;
		unsigned long reg_value;
}  _t_ucc_data;

/*_____________________ Imported Variables __________________________________*/

extern void key_led(bool flag);
extern void gpio_wifi_power(bool flag);
extern void gpio_wifi_reset(bool flag);
extern void gpio_wifi_power_down(bool flag);
extern void lcd_power(int flag);
extern void lcd_reset(void);
extern void lcd_module_init (void);
extern void lcd_backlight(int control);
extern void audio_ext_clock(bool flag);
extern void speaker_amp(bool flag);
extern void vibrator_control(bool flag);
extern void charger_green_led(bool flag);
extern void charger_red_led(bool flag);
extern int s3c2450_timer_setup (int channel, int usec, unsigned long g_tcnt, unsigned long g_tcmp);

		
