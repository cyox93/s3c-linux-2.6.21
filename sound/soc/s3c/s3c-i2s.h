/*
 * s3c24xx-i2s.c  --  ALSA Soc Audio Layer
 *
 * Copyright 2005 Wolfson Microelectronics PLC.
 * Author: Graeme Gregory
 *         graeme.gregory@wolfsonmicro.com or linux@wolfsonmicro.com
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  Revision history
 *    10th Nov 2006   Initial version.
 */

#ifndef S3C24XXI2S_H_
#define S3C24XXI2S_H_

/* clock sources */
#define S3C24XX_CLKSRC_PCLK 0
#define S3C24XX_CLKSRC_MPLL 1

/* Clock dividers */
#define S3C24XX_DIV_MCLK	0
#define S3C24XX_DIV_BCLK	1
#define S3C24XX_DIV_PRESCALER	2

/* prescaler */
#if !defined(CONFIG_CPU_S3C6400) && !defined(CONFIG_CPU_S3C6410)
#define S3C24XX_PRESCALE(a,b) \
	(((a - 1) << S3C2410_IISPSR_INTSHIFT) | ((b - 1) << S3C2410_IISPSR_EXTSHFIT))
#else
#define S3C24XX_PRESCALE(a,b) \
	(((a - 1) << S3C_IISPSR_INTSHIFT) | ((b - 1) << S3C_IISPSR_INTSHIFT))
#endif

u32 s3c_i2s_get_clockrate(void);

#ifdef CONFIG_CPU_S3C6410
extern struct snd_soc_cpu_dai s3c_i2s_v40_dai;
#endif

#endif /*S3C24XXI2S_H_*/
