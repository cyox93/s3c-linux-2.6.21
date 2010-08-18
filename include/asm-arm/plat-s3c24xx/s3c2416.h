/* linux/include/asm-arm/plat-s4c24xx/s3c2416.h
 *
 * Copyright (c) 2008 Samsung Electronics
 *	Ryu Euiyoul <ryu.real@gmail.com>
 *
 * Header file for s3c2416 cpu support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifdef CONFIG_CPU_S3C2416

struct s3c2410_uartcfg;

extern  int s3c2416_init(void);

extern void s3c2416_map_io(struct map_desc *mach_desc, int size);

extern void s3c2416_init_uarts(struct s3c2410_uartcfg *cfg, int no);

extern void s3c2416_init_clocks(int xtal);

extern  int s3c2443_baseclk_add(void);

#else
#define s3c2416_init_clocks NULL
#define s3c2416_init_uarts NULL
#define s3c2416_map_io NULL
#define s3c2416_init NULL
#endif

#ifdef CONFIG_MACH_CANOPUS
#define q_hw_ver( model )	( q_hw_ver_is_##model() )

#define q_hw_ver_is_7800()	( ((q_hw_version() & 0x18) == 0x10 && q_hw_version() != 0x15) ? 1 : 0 )
#define q_hw_ver_is_7800_ES2()	( (q_hw_version() == 0x10) ? 1 : 0 )
#define q_hw_ver_is_7800_TP()	( (q_hw_version() == 0x11) ? 1 : 0 )
#define q_hw_ver_is_7800_MP()	( (q_hw_version() == 0x12) ? 1 : 0 )
#define q_hw_ver_is_7800_MP1()	( (q_hw_version() == 0x13) ? 1 : 0 )	// MP 1.2-1
#define q_hw_ver_is_7800_MP2()	( (q_hw_version() == 0x14) ? 1 : 0 )	// MP 1.3

#define q_hw_ver_is_SWP2000()	( (q_hw_version() == 0x15) ? 1 : 0 )	// TP 1.0

#define q_hw_ver_is_SKBB()	( ((q_hw_version() & 0x1c) == 0x18) ? 1 : 0 )
#define q_hw_ver_is_SKBB_ES()	( (q_hw_version() == 0x18) ? 1 : 0 )
#define q_hw_ver_is_SKBB_PP()	( (q_hw_version() == 0x19) ? 1 : 0 )

#define q_hw_ver_is_KTQOOK()	( ((q_hw_version() & 0x1c) == 0x1c) ? 1 : 0 )
#define q_hw_ver_is_KTQOOK_TP()	( (q_hw_version() == 0x1d) ? 1 : 0 )	// TP
#define q_hw_ver_is_KTQOOK_TP2() ( (q_hw_version() == 0x1e) ? 1 : 0 )	// TP2

extern int q_hw_version(void);

#define Q_BOOT_FLAG_CLEAR		0x00
#define Q_BOOT_FLAG_REBOOT		0x01
#define Q_BOOT_FLAG_LCD_INIT		0x03

extern void q_boot_flag_set(int flag);
extern int q_boot_flag_get(void);
#endif
