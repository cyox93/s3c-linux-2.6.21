/* linux/arch/arm/plat-s3c24xx/common-smdk.c
 *
 * Copyright (c) 2006 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *
 * Common code for SMDK2410 and SMDK2440 boards
 *
 * http://www.fluff.org/ben/smdk2440/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/platform_device.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>
#include <asm/mach/flash.h>

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>

#include <asm/arch/regs-gpio.h>
#include <asm/arch/leds-gpio.h>
#include <asm/arch/regs-irq.h>

#include <asm/arch/nand.h>

#include <asm/plat-s3c24xx/common-smdk.h>
#include <asm/plat-s3c24xx/devs.h>
#include <asm/plat-s3c24xx/pm.h>

/* LED devices */

static struct s3c24xx_led_platdata smdk_pdata_led4 = {
	.gpio		= S3C2410_GPF4,
	.flags		= S3C24XX_LEDF_ACTLOW | S3C24XX_LEDF_TRISTATE,
	.name		= "led4",
	.def_trigger	= "timer",
};

static struct s3c24xx_led_platdata smdk_pdata_led5 = {
	.gpio		= S3C2410_GPF5,
	.flags		= S3C24XX_LEDF_ACTLOW | S3C24XX_LEDF_TRISTATE,
	.name		= "led5",
	.def_trigger	= "nand-disk",
};

static struct s3c24xx_led_platdata smdk_pdata_led6 = {
	.gpio		= S3C2410_GPF6,
	.flags		= S3C24XX_LEDF_ACTLOW | S3C24XX_LEDF_TRISTATE,
	.name		= "led6",
};

static struct s3c24xx_led_platdata smdk_pdata_led7 = {
	.gpio		= S3C2410_GPF7,
	.flags		= S3C24XX_LEDF_ACTLOW | S3C24XX_LEDF_TRISTATE,
	.name		= "led7",
};

static struct s3c24xx_led_platdata smdk_pdata_led_key_bl = {
	.gpio		= S3C2410_GPD15,
	.flags		= 0,
	.name		= "key-backlight",
};

static struct platform_device smdk_led4 = {
	.name		= "s3c24xx_led",
	.id		= 0,
	.dev		= {
		.platform_data = &smdk_pdata_led4,
	},
};

static struct platform_device smdk_led5 = {
	.name		= "s3c24xx_led",
	.id		= 1,
	.dev		= {
		.platform_data = &smdk_pdata_led5,
	},
};

static struct platform_device smdk_led6 = {
	.name		= "s3c24xx_led",
	.id		= 2,
	.dev		= {
		.platform_data = &smdk_pdata_led6,
	},
};

static struct platform_device smdk_led7 = {
	.name		= "s3c24xx_led",
	.id		= 3,
	.dev		= {
		.platform_data = &smdk_pdata_led7,
	},
};

static struct platform_device smdk_led_key_bl = {
	.name		= "s3c24xx_led",
	.id		= 4,
	.dev		= {
		.platform_data = &smdk_pdata_led_key_bl,
	},
};

/* NAND parititon from 2.4.18-swl5 */

static struct mtd_partition smdk_default_nand_part[] = {
	[0] = {
		.name	= "Boot Agent",
		.size	= SZ_16K,
		.offset	= 0,
	},
	[1] = {
		.name	= "S3C2410 flash partition 1",
		.offset = 0,
		.size	= SZ_2M,
	},
	[2] = {
		.name	= "S3C2410 flash partition 2",
		.offset = SZ_4M,
		.size	= SZ_4M,
	},
	[3] = {
		.name	= "S3C2410 flash partition 3",
		.offset	= SZ_8M,
		.size	= SZ_2M,
	},
	[4] = {
		.name	= "S3C2410 flash partition 4",
		.offset = SZ_1M * 10,
		.size	= SZ_4M,
	},
	[5] = {
		.name	= "S3C2410 flash partition 5",
		.offset	= SZ_1M * 14,
		.size	= SZ_1M * 10,
	},
	[6] = {
		.name	= "S3C2410 flash partition 6",
		.offset	= SZ_1M * 24,
		.size	= SZ_1M * 24,
	},
	[7] = {
		.name	= "S3C2410 flash partition 7",
		.offset = SZ_1M * 48,
		.size	= SZ_16M,
	}
};

/* ----------------S3C NAND partition information ---------------------*/
struct mtd_partition s3c_partition_info[] = {
        {
                .name		= "Bootloader",
                .offset		= 0,
                .size		= (256*SZ_1K),
#if 0
                .mask_flags	= MTD_CAP_NANDFLASH,
#endif
        },
        {
                .name		= "Kernel",
                .offset		= (256*SZ_1K),    /* Block number is 0x10 */
                .size		= (2*SZ_1M) - (256*SZ_1K),
#if 0
                .mask_flags	= MTD_CAP_NANDFLASH,
#endif
        },
#ifdef CONFIG_SPLIT_ROOT_FILESYSTEM
        {
                .name		= "Root - Cramfs",
                .offset		= (2*SZ_1M),    /* Block number is 0x80 */
                .size		= (24*SZ_1M),
        },
#endif
        {
                .name		= "File System",
                .offset		= MTDPART_OFS_APPEND,
                .size		= MTDPART_SIZ_FULL,
        }
};

struct s3c_nand_mtd_info nand_mtd_info = {
	.chip_nr = 1,
	.mtd_part_nr = ARRAY_SIZE(s3c_partition_info),
	.partition = s3c_partition_info,
};

struct s3c_nand_mtd_info * get_board_nand_mtd_info (void)
{
	return &nand_mtd_info;
}

struct flash_platform_data s3c_onenand_data = {
	.parts		= s3c_partition_info,
	.nr_parts	= ARRAY_SIZE(s3c_partition_info),
};


/* ---------------------------------------------------------------------*/


static struct s3c2410_nand_set smdk_nand_sets[] = {
	[0] = {
		.name		= "NAND",
		.nr_chips	= 1,
		.nr_partitions	= ARRAY_SIZE(smdk_default_nand_part),
		.partitions	= smdk_default_nand_part,
	},
};

/* choose a set of timings which should suit most 512Mbit
 * chips and beyond.
*/

static struct s3c2410_platform_nand smdk_nand_info = {
	.tacls		= 20,
	.twrph0		= 60,
	.twrph1		= 20,
	.nr_sets	= ARRAY_SIZE(smdk_nand_sets),
	.sets		= smdk_nand_sets,
};

/* devices we initialise */

static struct platform_device __initdata *smdk_devs[] = {
	&s3c_device_nand,
#ifndef CONFIG_MACH_CANOPUS
	&s3c_device_onenand,
	&smdk_led4,
	&smdk_led5,
	&smdk_led6,
	&smdk_led7,
#else
	&smdk_led_key_bl,
#endif	// CONFIG_MACH_CANOPUS
};

#ifdef CONFIG_MACH_CANOPUS
#include <asm/arch/regs-lcd.h>
#include <linux/delay.h>
void Key_gpio_init(void)
{
	u32 mask, mask1, mask2;

	mask  = __raw_readl(S3C2410_INTMSK);
	mask1 = __raw_readl(S3C2410_EXTINT0) & ~(0x00000007);
	mask2 = __raw_readl(S3C2410_EXTINT1) & ~(0x00777777);
	
	__raw_writel(mask1 | 0x00000002, S3C2410_EXTINT0);
	__raw_writel(mask2 | 0x00222222, S3C2410_EXTINT1);

	__raw_writel(mask | 0x21, S3C2410_INTMSK);
	__raw_writel(0x21, S3C2410_SRCPND);
	__raw_writel(0x21, S3C2410_INTPND);

	s3c2410_gpio_cfgpin(S3C2410_GPD11, S3C2410_GPD11_OUTP);
	s3c2410_gpio_cfgpin(S3C2410_GPD12, S3C2410_GPD12_OUTP);
	s3c2410_gpio_cfgpin(S3C2410_GPD13, S3C2410_GPD13_OUTP);	
	s3c2410_gpio_cfgpin(S3C2410_GPD14, S3C2410_GPD14_OUTP);

	s3c2410_gpio_setpin(S3C2410_GPD11, 0);
	s3c2410_gpio_setpin(S3C2410_GPD12, 0);
	s3c2410_gpio_setpin(S3C2410_GPD13, 0);
	s3c2410_gpio_setpin(S3C2410_GPD14, 0);	

	s3c2410_gpio_cfgpin(S3C2410_GPF0, S3C2410_GPF0_EINT0);
	s3c2410_gpio_cfgpin(S3C2410_GPG0, S3C2410_GPG0_EINT8);
	s3c2410_gpio_cfgpin(S3C2410_GPG1, S3C2410_GPG1_EINT9);
	s3c2410_gpio_cfgpin(S3C2410_GPG2, S3C2410_GPG2_EINT10);	
	s3c2410_gpio_cfgpin(S3C2410_GPG3, S3C2410_GPG3_EINT11);
	s3c2410_gpio_cfgpin(S3C2410_GPG4, S3C2410_GPG4_EINT12);	
	s3c2410_gpio_cfgpin(S3C2410_GPG5, S3C2410_GPG5_EINT13);	

	s3c2410_gpio_pullup(S3C2410_GPF0, 0);
	s3c2410_gpio_pullup(S3C2410_GPG0, 2);
	s3c2410_gpio_pullup(S3C2410_GPG1, 2);
	s3c2410_gpio_pullup(S3C2410_GPG2, 2);
	s3c2410_gpio_pullup(S3C2410_GPG3, 2);
	s3c2410_gpio_pullup(S3C2410_GPG4, 2);
	s3c2410_gpio_pullup(S3C2410_GPG5, 2);

	mdelay(100);
}

/* 3.3V LDO  */
void gpio_wifi_power(bool flag)
{
	unsigned long val;
	
	if (flag) { 		
		val = __raw_readl(S3C2410_GPHDAT);
		__raw_writel((val |= 0x1000), S3C2410_GPHDAT);
	} else {
		val = __raw_readl(S3C2410_GPHDAT);		
		__raw_writel((val&0xefff), S3C2410_GPHDAT);		
	}
}

void gpio_wifi_power_down(bool flag)
{
	if (flag) { 
		s3c2410_gpio_setpin(S3C2410_GPH7, 1);
	} else {
		s3c2410_gpio_setpin(S3C2410_GPH7, 0);
	}
}

void gpio_wifi_reset(void)
{
	mdelay(1);
	s3c2410_gpio_setpin(S3C2410_GPH6, 0);
	mdelay(10);
	s3c2410_gpio_setpin(S3C2410_GPH6, 1);
	mdelay(1);
}

void wifi_gpio_init (void)
{	
	unsigned long mask;

	s3c2410_gpio_cfgpin(S3C2410_GPH6, S3C2410_GPH6_OUTP);
	s3c2410_gpio_cfgpin(S3C2410_GPH7, S3C2410_GPH7_OUTP);
	s3c2410_gpio_cfgpin(S3C2410_GPH12, S3C2410_GPH12_OUTP);

	s3c2410_gpio_cfgpin(S3C2410_GPL0, S3C2410_GPL0_SD0_DAT0);
	s3c2410_gpio_cfgpin(S3C2410_GPL1, S3C2410_GPL1_SD0_DAT1);
	s3c2410_gpio_cfgpin(S3C2410_GPL2, S3C2410_GPL2_SD0_DAT02);
	s3c2410_gpio_cfgpin(S3C2410_GPL3, S3C2410_GPL3_SD0_DAT3);

	s3c2410_gpio_cfgpin(S3C2410_GPL8, S3C2410_GPL8_SD0_CMD);
	s3c2410_gpio_cfgpin(S3C2410_GPL9, S3C2410_GPL9_SD0_CLK);

	s3c2410_gpio_pullup(S3C2410_GPH6, 2); /* WiFi Reset pull-up enable */

	s3c2410_gpio_pullup(S3C2410_GPL0, 2); /* SD DAT0 pull-up enable */
	s3c2410_gpio_pullup(S3C2410_GPL1, 2); /* SD DAT1 pull-up enable */
	s3c2410_gpio_pullup(S3C2410_GPL2, 2); /* SD DAT2 pull-up enable */
	s3c2410_gpio_pullup(S3C2410_GPL3, 2); /* SD DAT3 pull-up enable */
	s3c2410_gpio_pullup(S3C2410_GPL8, 2); /* SD CMD pull-up enable */

	s3c2410_gpio_setpin(S3C2410_GPH6, 1);	
	s3c2410_gpio_setpin(S3C2410_GPH7, 1);

	// set EXTINT4 for WOW
	s3c2410_gpio_cfgpin(S3C2410_GPF4, S3C2410_GPF4_EINT4);
	s3c2410_gpio_pullup(S3C2410_GPF4, 0);

	// set rising edge triggered
	mask = __raw_readl(S3C2410_EXTINT0) & ~(0x7<<16);
	__raw_writel(mask | (0x4 << 16) , S3C2410_EXTINT0);

	gpio_wifi_power(0); 
}

void lcd_reset(void)
{
	mdelay(10);
	s3c2410_gpio_setpin(S3C2410_GPB1, 0);
	//mdelay(10);
	mdelay(20);
	s3c2410_gpio_setpin(S3C2410_GPB1, 1);
	//mdelay(50);
	mdelay(150);
}

void lcd_gpio_init(void)
{
	u32 val;

	// set gpio out for lcd reset 
	s3c2410_gpio_cfgpin(S3C2410_GPB1, S3C2410_GPB1_OUTP);
	s3c2410_gpio_setpin(S3C2410_GPB1, 1);

	// set lcd interface
	val = __raw_readl(S3C2410_GPCCON) & 0x0fcc0;
	__raw_writel(val | 0xaaaa033a, S3C2410_GPCCON);

	// set lcd interface
	val = __raw_readl(S3C2410_GPDCON) & ~(0x003fffff);
	__raw_writel(val | 0x002aaaaa, S3C2410_GPDCON);

	// set gpio in for lcd panel id
	s3c2410_gpio_cfgpin(S3C2410_GPD8, S3C2410_GPD8_INP);
	s3c2410_gpio_cfgpin(S3C2410_GPD9, S3C2410_GPD9_INP);
	s3c2410_gpio_cfgpin(S3C2410_GPD10, S3C2410_GPD10_INP);

	// set pull up/down disable for lcd panel id
	s3c2410_gpio_pullup(S3C2410_GPD8, 0);
	s3c2410_gpio_pullup(S3C2410_GPD9, 0);
	s3c2410_gpio_pullup(S3C2410_GPD10, 0);
}

void speaker_amp(bool flag)
{
	if (flag) { 
		s3c2410_gpio_setpin(S3C2410_GPH8, 1);
	} else {
		s3c2410_gpio_setpin(S3C2410_GPH8, 0);
	}
}

void canopus_gpio_init(void)
{
	printk("canopus_gpio_init\n");

	// Speaker AMP
	s3c2410_gpio_cfgpin(S3C2410_GPH8, S3C2410_GPH8_OUTP);
	s3c2410_gpio_setpin(S3C2410_GPH8, 0);

	// key LED
	s3c2410_gpio_cfgpin(S3C2410_GPD15, S3C2410_GPD15_OUTP);
	s3c2410_gpio_setpin(S3C2410_GPD15, 0);

   	 // Wi-Fi
	wifi_gpio_init();

	// KeyPAD 	
	Key_gpio_init();
	
}
#endif	// CONFIG_MACH_CANOPUS

void __init smdk_machine_init(void)
{
#ifndef CONFIG_MACH_CANOPUS
	/* Configure the LEDs (even if we have no LED support)*/

	s3c2410_gpio_cfgpin(S3C2410_GPF4, S3C2410_GPF4_OUTP);
	s3c2410_gpio_cfgpin(S3C2410_GPF5, S3C2410_GPF5_OUTP);
	s3c2410_gpio_cfgpin(S3C2410_GPF6, S3C2410_GPF6_OUTP);
	s3c2410_gpio_cfgpin(S3C2410_GPF7, S3C2410_GPF7_OUTP);

	s3c2410_gpio_setpin(S3C2410_GPF4, 1);
	s3c2410_gpio_setpin(S3C2410_GPF5, 1);
	s3c2410_gpio_setpin(S3C2410_GPF6, 1);
	s3c2410_gpio_setpin(S3C2410_GPF7, 1);

	s3c_device_nand.dev.platform_data = &smdk_nand_info;
#else
	canopus_gpio_init();
#endif	// CONFIG_MACH_CANOPUS
	
	//For s3c nand partition
	s3c_device_nand.dev.platform_data = &nand_mtd_info;

	platform_add_devices(smdk_devs, ARRAY_SIZE(smdk_devs));

	s3c2410_pm_init();
}

#ifdef CONFIG_MACH_CANOPUS
EXPORT_SYMBOL(gpio_wifi_power);
EXPORT_SYMBOL(gpio_wifi_power_down);
EXPORT_SYMBOL(gpio_wifi_reset);
EXPORT_SYMBOL(lcd_reset);
EXPORT_SYMBOL(lcd_gpio_init);
EXPORT_SYMBOL(speaker_amp);
#endif	// CONFIG_MACH_CANOPUS

