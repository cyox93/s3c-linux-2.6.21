/* linux/include/asm-arm/arch-s3c2410/regs-timer.h
 *
 * Copyright (c) 2003 Simtec Electronics <linux@simtec.co.uk>
 *		      http://www.simtec.co.uk/products/SWLINUX/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * S3C2410 Timer configuration
*/


#ifndef __ASM_ARCH_REGS_TIMER_H
#define __ASM_ARCH_REGS_TIMER_H "$Id: timer.h,v 1.4 2003/05/06 19:30:50 ben Exp $"

#define S3C2410_TIMERREG(x) (S3C24XX_VA_TIMER + (x))
#define S3C2410_TIMERREG2(tmr,reg) S3C2410_TIMERREG((reg)+0x0c+((tmr)*0x0c))

#define S3C2410_TCFG0	      S3C2410_TIMERREG(0x00)
#define S3C2410_TCFG1	      S3C2410_TIMERREG(0x04)
#define S3C2410_TCON	      S3C2410_TIMERREG(0x08)

#define S3C2410_TCFG_PRESCALER0_MASK (255<<0)
#define S3C2410_TCFG_PRESCALER1_MASK (255<<8)
#define S3C2410_TCFG_PRESCALER1_SHIFT (8)
#define S3C2410_TCFG_DEADZONE_MASK   (255<<16)
#define S3C2410_TCFG_DEADZONE_SHIFT  (16)

#define S3C2410_TCFG1_MUX4_DIV2	  (0<<16)
#define S3C2410_TCFG1_MUX4_DIV4	  (1<<16)
#define S3C2410_TCFG1_MUX4_DIV8	  (2<<16)
#define S3C2410_TCFG1_MUX4_DIV16  (3<<16)
#define S3C2410_TCFG1_MUX4_TCLK1  (4<<16)
#define S3C2410_TCFG1_MUX4_MASK	  (15<<16)
#define S3C2410_TCFG1_MUX4_SHIFT  (16)

#define S3C2410_TCFG1_MUX3_DIV2	  (0<<12)
#define S3C2410_TCFG1_MUX3_DIV4	  (1<<12)
#define S3C2410_TCFG1_MUX3_DIV8	  (2<<12)
#define S3C2410_TCFG1_MUX3_DIV16  (3<<12)
#define S3C2410_TCFG1_MUX3_TCLK1  (4<<12)
#define S3C2410_TCFG1_MUX3_MASK	  (15<<12)


#define S3C2410_TCFG1_MUX2_DIV2	  (0<<8)
#define S3C2410_TCFG1_MUX2_DIV4	  (1<<8)
#define S3C2410_TCFG1_MUX2_DIV8	  (2<<8)
#define S3C2410_TCFG1_MUX2_DIV16  (3<<8)
#define S3C2410_TCFG1_MUX2_TCLK1  (4<<8)
#define S3C2410_TCFG1_MUX2_MASK	  (15<<8)


#define S3C2410_TCFG1_MUX1_DIV2	  (0<<4)
#define S3C2410_TCFG1_MUX1_DIV4	  (1<<4)
#define S3C2410_TCFG1_MUX1_DIV8	  (2<<4)
#define S3C2410_TCFG1_MUX1_DIV16  (3<<4)
#define S3C2410_TCFG1_MUX1_TCLK0  (4<<4)
#define S3C2410_TCFG1_MUX1_MASK	  (15<<4)

#define S3C2410_TCFG1_MUX0_DIV2	  (0<<0)
#define S3C2410_TCFG1_MUX0_DIV4	  (1<<0)
#define S3C2410_TCFG1_MUX0_DIV8	  (2<<0)
#define S3C2410_TCFG1_MUX0_DIV16  (3<<0)
#define S3C2410_TCFG1_MUX0_TCLK0  (4<<0)
#define S3C2410_TCFG1_MUX0_MASK	  (15<<0)

/* for each timer, we have an count buffer, an compare buffer and
 * an observation buffer
*/

/* WARNING - timer 4 has no buffer reg, and it's observation is at +4 */

#define S3C2410_TCNTB(tmr)    S3C2410_TIMERREG2(tmr, 0x00)
#define S3C2410_TCMPB(tmr)    S3C2410_TIMERREG2(tmr, 0x04)
#define S3C2410_TCNTO(tmr)    S3C2410_TIMERREG2(tmr, (((tmr) == 4) ? 0x04 : 0x08))

#define S3C2410_TCON_T4RELOAD	  (1<<22)
#define S3C2410_TCON_T4MANUALUPD  (1<<21)
#define S3C2410_TCON_T4START	  (1<<20)

#define S3C2410_TCON_T3RELOAD	  (1<<19)
#define S3C2410_TCON_T3INVERT	  (1<<18)
#define S3C2410_TCON_T3MANUALUPD  (1<<17)
#define S3C2410_TCON_T3START	  (1<<16)

#define S3C2410_TCON_T2RELOAD	  (1<<15)
#define S3C2410_TCON_T2INVERT	  (1<<14)
#define S3C2410_TCON_T2MANUALUPD  (1<<13)
#define S3C2410_TCON_T2START	  (1<<12)

#define S3C2410_TCON_T1RELOAD	  (1<<11)
#define S3C2410_TCON_T1INVERT	  (1<<10)
#define S3C2410_TCON_T1MANUALUPD  (1<<9)
#define S3C2410_TCON_T1START	  (1<<8)

#define S3C2410_TCON_T0DEADZONE	  (1<<4)
#define S3C2410_TCON_T0RELOAD	  (1<<3)
#define S3C2410_TCON_T0INVERT	  (1<<2)
#define S3C2410_TCON_T0MANUALUPD  (1<<1)
#define S3C2410_TCON_T0START	  (1<<0)

#if defined (CONFIG_CPU_S3C6400) || defined (CONFIG_CPU_S3C6410) 
/* TIMER */
#define S3C_TIMERREG(x) (S3C24XX_VA_TIMER + (x))
#define S3C_TIMERREG2(tmr,reg) S3C_TIMERREG((reg)+0x0c+((tmr)*0x0c))

#define S3C_TCFG0	      S3C_TIMERREG(0x00)
#define S3C_TCFG1	      S3C_TIMERREG(0x04)
#define S3C_TCON	      S3C_TIMERREG(0x08)
#define S3C_TINT_CSTAT	S3C_TIMERREG(0x44)


#define S3C_TCFG_PRESCALER0_MASK (255<<0)
#define S3C_TCFG_PRESCALER1_MASK (255<<8)
#define S3C_TCFG_PRESCALER1_SHIFT (8)
#define S3C_TCFG_PRESCALER0_SHIFT (0)
#define S3C_TCFG_DEADZONE_MASK	 (255<<16)
#define S3C_TCFG_DEADZONE_SHIFT  (16)

#define S3C_TCFG1_MUX4_DIV1	  (0<<16)
#define S3C_TCFG1_MUX4_DIV2	  (1<<16)
#define S3C_TCFG1_MUX4_DIV4	  (2<<16)
#define S3C_TCFG1_MUX4_DIV8	  (3<<16)
#define S3C_TCFG1_MUX4_DIV16  (4<<16)
#define S3C_TCFG1_MUX4_TCLK1  (5<<16)
#define S3C_TCFG1_MUX4_MASK	  (15<<16)

#define S3C_TCFG1_MUX3_DIV1	  (0<<12)
#define S3C_TCFG1_MUX3_DIV2	  (1<<12)
#define S3C_TCFG1_MUX3_DIV4	  (2<<12)
#define S3C_TCFG1_MUX3_DIV8	  (3<<12)
#define S3C_TCFG1_MUX3_DIV16  (4<<12)
#define S3C_TCFG1_MUX3_TCLK1  (5<<12)
#define S3C_TCFG1_MUX3_MASK	  (15<<12)

#define S3C_TCFG1_MUX2_DIV1	  (0<<8)
#define S3C_TCFG1_MUX2_DIV2	  (1<<8)
#define S3C_TCFG1_MUX2_DIV4	  (2<<8)
#define S3C_TCFG1_MUX2_DIV8	  (3<<8)
#define S3C_TCFG1_MUX2_DIV16  (4<<8)
#define S3C_TCFG1_MUX2_TCLK1  (5<<8)
#define S3C_TCFG1_MUX2_MASK	  (15<<8)

#define S3C_TCFG1_MUX1_DIV1	  (0<<4)
#define S3C_TCFG1_MUX1_DIV2	  (1<<4)
#define S3C_TCFG1_MUX1_DIV4	  (2<<4)
#define S3C_TCFG1_MUX1_DIV8	  (3<<4)
#define S3C_TCFG1_MUX1_DIV16  (4<<4)
#define S3C_TCFG1_MUX1_TCLK0  (5<<4)
#define S3C_TCFG1_MUX1_MASK	  (15<<4)

#define S3C_TCFG1_MUX0_DIV1	  (0<<0)
#define S3C_TCFG1_MUX0_DIV2	  (1<<0)
#define S3C_TCFG1_MUX0_DIV4	  (2<<0)
#define S3C_TCFG1_MUX0_DIV8	  (3<<0)
#define S3C_TCFG1_MUX0_DIV16  (4<<0)
#define S3C_TCFG1_MUX0_TCLK0  (5<<0)
#define S3C_TCFG1_MUX0_MASK	  (15<<0)

/* for each timer, we have an count buffer, an compare buffer and
 * an observation buffer
*/

/* WARNING - timer 4 has no buffer reg, and it's observation is at +4 */
/* 32bit timer used */

#define S3C_TCNTB(tmr)	  S3C_TIMERREG2(tmr, 0x00)
#define S3C_TCMPB(tmr)	  S3C_TIMERREG2(tmr, 0x04)
#define S3C_TCNTO(tmr)	  S3C_TIMERREG2(tmr, (((tmr) == 4) ? 0x04 : 0x08))

#define S3C_TCON_T4RELOAD	  (1<<22)
#define S3C_TCON_T4MANUALUPD  (1<<21)
#define S3C_TCON_T4START	  (1<<20)

#define S3C_TCON_T3RELOAD	  (1<<19)
#define S3C_TCON_T3INVERT	  (1<<18)
#define S3C_TCON_T3MANUALUPD  (1<<17)
#define S3C_TCON_T3START	  (1<<16)

#define S3C_TCON_T2RELOAD	  (1<<15)
#define S3C_TCON_T2INVERT	  (1<<14)
#define S3C_TCON_T2MANUALUPD  (1<<13)
#define S3C_TCON_T2START	  (1<<12)

#define S3C_TCON_T1RELOAD	  (1<<11)
#define S3C_TCON_T1INVERT	  (1<<10)
#define S3C_TCON_T1MANUALUPD  (1<<9)
#define S3C_TCON_T1START	  (1<<8)

#define S3C_TCON_T0DEADZONE	  (1<<4)
#define S3C_TCON_T0RELOAD	  (1<<3)
#define S3C_TCON_T0INVERT	  (1<<2)
#define S3C_TCON_T0MANUALUPD  (1<<1)
#define S3C_TCON_T0START	  (1<<0)

/* Interrupt Control and Status register*/
#define S3C_TINT_CSTAT_T4INT	(1<<9)
#define S3C_TINT_CSTAT_T3INT	(1<<8)
#define S3C_TINT_CSTAT_T2INT	(1<<7)
#define S3C_TINT_CSTAT_T1INT	(1<<6)
#define S3C_TINT_CSTAT_T0INT	(1<<5)
#define S3C_TINT_CSTAT_T4INTEN	(1<<4)
#define S3C_TINT_CSTAT_T3INTEN	(1<<3)
#define S3C_TINT_CSTAT_T2INTEN	(1<<2)
#define S3C_TINT_CSTAT_T1INTEN	(1<<1)
#define S3C_TINT_CSTAT_T0INTEN	(1<<0)


#endif

#endif /*  __ASM_ARCH_REGS_TIMER_H */



