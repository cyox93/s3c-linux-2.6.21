/* linux/include/asm-arm/arch-s3c2410/regs-irq.h
 *
 * Copyright (c) 2003 Simtec Electronics <linux@simtec.co.uk>
 *		      http://www.simtec.co.uk/products/SWLINUX/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/


#ifndef ___ASM_ARCH_REGS_IRQ_H
#define ___ASM_ARCH_REGS_IRQ_H "$Id: irq.h,v 1.3 2003/03/25 21:29:06 ben Exp $"

/* interrupt controller */

#define S3C2410_IRQREG(x)   ((x) + S3C24XX_VA_IRQ)
#define S3C2410_EINTREG(x)  ((x) + S3C24XX_VA_GPIO)
#define S3C24XX_EINTREG(x)  ((x) + S3C24XX_VA_GPIO2)

#define S3C2410_SRCPND	       S3C2410_IRQREG(0x000)
#define S3C2410_INTMOD	       S3C2410_IRQREG(0x004)
#define S3C2410_INTMSK	       S3C2410_IRQREG(0x008)
#if defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416) 
#define S3C2410_PRIORITY       S3C2410_IRQREG(0x030)
#else
#define S3C2410_PRIORITY       S3C2410_IRQREG(0x00C) 
#endif
#define S3C2410_INTPND	       S3C2410_IRQREG(0x010)
#define S3C2410_INTOFFSET      S3C2410_IRQREG(0x014)
#define S3C2410_SUBSRCPND      S3C2410_IRQREG(0x018)
#define S3C2410_INTSUBMSK      S3C2410_IRQREG(0x01C)

#if defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416) 
#define S3C2450_SRCPND	       S3C2410_IRQREG(0x040)
#define S3C2450_INTMOD	       S3C2410_IRQREG(0x044)
#define S3C2450_INTMSK	       S3C2410_IRQREG(0x048)
#define S3C2450_INTPND	       S3C2410_IRQREG(0x050)
#define S3C2450_INTOFFSET      S3C2410_IRQREG(0x054)
#define S3C2450_PRIORITY       S3C2410_IRQREG(0x070)
#endif

/* mask: 0=enable, 1=disable
 * 1 bit EINT, 4=EINT4, 23=EINT23
 * EINT0,1,2,3 are not handled here.
*/

#define S3C2410_EINTMASK       S3C2410_EINTREG(0x0A4)
#define S3C2410_EINTPEND       S3C2410_EINTREG(0X0A8)
#define S3C2412_EINTMASK       S3C2410_EINTREG(0x0B4)
#define S3C2412_EINTPEND       S3C2410_EINTREG(0X0B8)

#define S3C24XX_EINTMASK       S3C24XX_EINTREG(0x0A4)
#define S3C24XX_EINTPEND       S3C24XX_EINTREG(0X0A8)

#if defined (CONFIG_CPU_S3C6400) || defined (CONFIG_CPU_S3C6410) 

/* interrupt controller */
#define S3C_VIC0REG(x)		((x) + S3C24XX_VA_IRQ)
#define S3C_VIC1REG(x)		((x) + S3C24XX_VA_IRQ+0x100000)
#define S3C_EINTREG(x)		((x) + S3C24XX_VA_GPIO)

#define S3C_EINT0CON0		S3C_EINTREG(0x900)
#define S3C_EINT0MASK		S3C_EINTREG(0x924)

#define S3C_VIC0IRQSTATUS	S3C_VIC0REG(0x000)
#define S3C_VIC1IRQSTATUS	S3C_VIC1REG(0x000)

#define S3C_VIC0FIQSTATUS	S3C_VIC0REG(0x004)
#define S3C_VIC1FIQSTATUS	S3C_VIC1REG(0x004)

#define S3C_VIC0RAWINTR 	S3C_VIC0REG(0x008)
#define S3C_VIC1RAWINTR 	S3C_VIC1REG(0x008)

#define S3C_VIC0INTSELECT	S3C_VIC0REG(0x00C)
#define S3C_VIC1INTSELECT	S3C_VIC1REG(0x00C)

#define S3C_VIC0INTENABLE	S3C_VIC0REG(0x010)
#define S3C_VIC1INTENABLE	S3C_VIC1REG(0x010)

#define S3C_VIC0INTENCLEAR	S3C_VIC0REG(0x014)
#define S3C_VIC1INTENCLEAR	S3C_VIC1REG(0x014)

#define S3C_VIC0SOFTINT 	S3C_VIC0REG(0x018)
#define S3C_VIC1SOFTINT 	S3C_VIC1REG(0x018)

#define S3C_VIC0SOFTINTCLEAR	S3C_VIC0REG(0x01C)
#define S3C_VIC1SOFTINTCLEAR	S3C_VIC1REG(0x01C)

#define S3C_VIC0PROTECTION	S3C_VIC0REG(0x020)
#define S3C_VIC1PROTECTION	S3C_VIC1REG(0x020)

#define S3C_VIC0SWPRIORITYMASK	S3C_VIC0REG(0x024)
#define S3C_VIC1SWPRIORITYMASK	S3C_VIC1REG(0x024)

#define S3C_VIC0PRIORITYDAISY	S3C_VIC0REG(0x028)
#define S3C_VIC1PRIORITYDAISY	S3C_VIC1REG(0x028)

#define S3C_VIC0VECTADDR0	S3C_VIC0REG(0x100)
#define S3C_VIC1VECTADDR0	S3C_VIC1REG(0x100)

#define S3C_VIC0VECTADDR1	S3C_VIC0REG(0x104)
#define S3C_VIC1VECTADDR1	S3C_VIC1REG(0x104)

#define S3C_VIC0VECTADDR2	S3C_VIC0REG(0x108)
#define S3C_VIC1VECTADDR2	S3C_VIC1REG(0x108)


#define S3C_VIC0ADDRESS 	S3C_VIC0REG(0xF00)
#define S3C_VIC1ADDRESS 	S3C_VIC1REG(0xF00)

#elif defined(CONFIG_CPU_S3C2443) || defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416)
#define S3C_PRIORITY		S3C2410_PRIORITY
#endif

#endif /* ___ASM_ARCH_REGS_IRQ_H */
