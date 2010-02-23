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
#if 0
	&s3c_device_onenand,
#endif
	&smdk_led4,
	&smdk_led5,
	&smdk_led6,
	&smdk_led7,
};

//#define WPU7800_EVM_GPIO
#define WPU7800_WS_GPIO

#ifdef WPU7800_WS_GPIO
#include <asm/arch/regs-lcd.h>
#include <linux/delay.h>

void key_led(bool flag)
{
	if (flag) { 
		s3c2410_gpio_setpin(S3C2410_GPA12, 1);
	} else {
		s3c2410_gpio_setpin(S3C2410_GPA12, 0);
	}
}

void Key_gpio_init(void)
{
	s3c2410_gpio_cfgpin(S3C2410_GPG0, S3C2410_GPG0_OUTP);
	s3c2410_gpio_cfgpin(S3C2410_GPG1, S3C2410_GPG1_OUTP);
	s3c2410_gpio_cfgpin(S3C2410_GPG2, S3C2410_GPG2_OUTP);	
	s3c2410_gpio_cfgpin(S3C2410_GPG3, S3C2410_GPG3_OUTP);

	s3c2410_gpio_cfgpin(S3C2410_GPF1, S3C2410_GPF1_INP);
	s3c2410_gpio_cfgpin(S3C2410_GPF2, S3C2410_GPF2_INP);
	s3c2410_gpio_cfgpin(S3C2410_GPF3, S3C2410_GPF3_INP);	
	s3c2410_gpio_cfgpin(S3C2410_GPF5, S3C2410_GPF5_INP);
	s3c2410_gpio_cfgpin(S3C2410_GPF6, S3C2410_GPF6_INP);	
	s3c2410_gpio_cfgpin(S3C2410_GPF7, S3C2410_GPF7_INP);	

	__raw_writel(0xaaaa, S3C2410_GPFUP);

	s3c2410_gpio_setpin(S3C2410_GPG0, 0);
	s3c2410_gpio_setpin(S3C2410_GPG1, 0);
	s3c2410_gpio_setpin(S3C2410_GPG2, 0);
	s3c2410_gpio_setpin(S3C2410_GPG3, 0);	

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
	mdelay(10);
	s3c2410_gpio_setpin(S3C2410_GPH6, 0);
	gpio_wifi_power_down(0);
	mdelay(10);
	s3c2410_gpio_setpin(S3C2410_GPH6, 1);
	gpio_wifi_power_down(1);
}

void wifi_gpio_init (void)
{	
	s3c2410_gpio_cfgpin(S3C2410_GPH6, S3C2410_GPH6_OUTP);
	s3c2410_gpio_cfgpin(S3C2410_GPH12, S3C2410_GPH12_OUTP);
	s3c2410_gpio_cfgpin(S3C2410_GPH7, S3C2410_GPH7_OUTP);

	s3c2410_gpio_pullup(S3C2410_GPH6, 2); /* WiFi Reset pull-up enable */

	s3c2410_gpio_pullup(S3C2410_GPL0, 2); /* SD DAT0 pull-up enable */
	s3c2410_gpio_pullup(S3C2410_GPL1, 2); /* SD DAT1 pull-up enable */
	s3c2410_gpio_pullup(S3C2410_GPL2, 2); /* SD DAT2 pull-up enable */
	s3c2410_gpio_pullup(S3C2410_GPL3, 2); /* SD DAT3 pull-up enable */
	s3c2410_gpio_pullup(S3C2410_GPL8, 2); /* SD CMD pull-up enable */

	s3c2410_gpio_setpin(S3C2410_GPH6, 1);	
	s3c2410_gpio_setpin(S3C2410_GPH7, 1);
	
	gpio_wifi_power(0); 
}

void lcd_power(bool flag)
{
	if (flag) { 
		s3c2410_gpio_setpin(S3C2410_GPB0, 1);
	} else {
		s3c2410_gpio_setpin(S3C2410_GPB0, 0);
	}
}

void lcd_reset(void)
{
	mdelay(10);
	s3c2410_gpio_setpin(S3C2410_GPB1, 0);
	mdelay(10);
	s3c2410_gpio_setpin(S3C2410_GPB1, 1);
	mdelay(50);
}

void lcd_backlight(int control)
{
	int i;

	if (!control) {
		s3c2410_gpio_setpin(S3C2410_GPB2, 0);
	}
	else {	
		s3c2410_gpio_setpin(S3C2410_GPB2, 1);
		udelay(500);
			
		for (i=0;i < control;i++) {
			s3c2410_gpio_setpin(S3C2410_GPB2, 0);
			udelay(50);
			s3c2410_gpio_setpin(S3C2410_GPB2, 1);
			udelay(50);
		}
	}
}

void vd_bus_inout_set(int flag)
{
	u32 mask;

	mask = __raw_readl(S3C2410_GPDCON) & ~(0x003fffff);

	if(flag) {
		__raw_writel(0x5555, S3C2410_GPCCON);
		__raw_writel(mask | 0x00000000, S3C2410_GPDCON);
	}
	else {
		__raw_writel(0x55555555, S3C2410_GPCCON);
		__raw_writel(mask | 0x00155555, S3C2410_GPDCON);
	}
}

void lcd_gpio_init(void)
{
	s3c2410_gpio_cfgpin(S3C2410_GPB0, S3C2410_GPB0_OUTP);
	s3c2410_gpio_cfgpin(S3C2410_GPB1, S3C2410_GPB1_OUTP);
	s3c2410_gpio_cfgpin(S3C2410_GPB2, S3C2410_GPB2_OUTP);	

	s3c2410_gpio_cfgpin(S3C2410_GPC0, S3C2410_GPC0_OUTP); // read
	s3c2410_gpio_cfgpin(S3C2410_GPC1, S3C2410_GPC1_OUTP); // wirte
	s3c2410_gpio_cfgpin(S3C2410_GPC2, S3C2410_GPC2_OUTP); // cs
	s3c2410_gpio_cfgpin(S3C2410_GPC4, S3C2410_GPC4_OUTP); // RS

	vd_bus_inout_set(0);

	s3c2410_gpio_setpin(S3C2410_GPC0, 1);
	s3c2410_gpio_setpin(S3C2410_GPC1, 1);
	s3c2410_gpio_setpin(S3C2410_GPC2, 1);
	s3c2410_gpio_setpin(S3C2410_GPC4, 1);

	s3c2410_gpio_setpin(S3C2410_GPB0, 1);
	s3c2410_gpio_setpin(S3C2410_GPB1, 1);
	s3c2410_gpio_setpin(S3C2410_GPB2, 1);

	udelay(100);
}

void audio_ext_clock(bool flag)
{
	if (flag) { 
		s3c2410_gpio_setpin(S3C2410_GPE2, 1);
	} else {
		s3c2410_gpio_setpin(S3C2410_GPE2, 0);
	}
}

void speaker_amp(bool flag)
{
	if (flag) { 
		s3c2410_gpio_setpin(S3C2410_GPH2, 1);
	} else {
		s3c2410_gpio_setpin(S3C2410_GPH2, 0);
	}
}

void vibrator_control(bool flag)
{
	if (flag) { 
		s3c2410_gpio_setpin(S3C2410_GPH5, 1);
	} else {
		s3c2410_gpio_setpin(S3C2410_GPH5, 0);
	}
}

void charger_green_led(bool flag)
{
	if (flag) { 
		s3c2410_gpio_setpin(S3C2410_GPB3, 1);
	} else {
		s3c2410_gpio_setpin(S3C2410_GPB3, 0);
	}
}

void charger_red_led(bool flag)
{
	if (flag) { 
		s3c2410_gpio_setpin(S3C2410_GPB4, 1);
	} else {
		s3c2410_gpio_setpin(S3C2410_GPB4, 0);
	}
}

void wpu7800_gpio_init(void)
{
	printk("wpu7800_gpio_init\n");

	// Speaker AMP
	s3c2410_gpio_cfgpin(S3C2410_GPH2, S3C2410_GPH2_OUTP);
	s3c2410_gpio_setpin(S3C2410_GPH2, 0);

	// key LED
	s3c2410_gpio_cfgpin(S3C2410_GPA12, S3C2410_GPA12_OUT);
	s3c2410_gpio_setpin(S3C2410_GPA12, 0);

	// Audio Ext Clock
	s3c2410_gpio_cfgpin(S3C2410_GPE2, S3C2410_GPE2_CDCLK);
	s3c2410_gpio_setpin(S3C2410_GPE2, 0);

	// Vibrator
	s3c2410_gpio_cfgpin(S3C2410_GPH5, S3C2410_GPH5_OUTP);
	s3c2410_gpio_setpin(S3C2410_GPH5, 0);

	// Charger LED
	s3c2410_gpio_cfgpin(S3C2410_GPB3, S3C2410_GPB3_OUTP); // Green
	s3c2410_gpio_cfgpin(S3C2410_GPB4, S3C2410_GPB4_OUTP); // Red
	s3c2410_gpio_setpin(S3C2410_GPB3, 0);
	s3c2410_gpio_setpin(S3C2410_GPB4, 0);

   	 // Wi-Fi
	wifi_gpio_init();

	// LCD
	lcd_gpio_init();

	// KeyPAD 	
	Key_gpio_init();
	
}
#endif 

void __init smdk_machine_init(void)
{
	/* Configure the LEDs (even if we have no LED support)*/

	s3c2410_gpio_cfgpin(S3C2410_GPF4, S3C2410_GPF4_OUTP);
	s3c2410_gpio_cfgpin(S3C2410_GPF5, S3C2410_GPF5_OUTP);
	s3c2410_gpio_cfgpin(S3C2410_GPF6, S3C2410_GPF6_OUTP);
	s3c2410_gpio_cfgpin(S3C2410_GPF7, S3C2410_GPF7_OUTP);

	s3c2410_gpio_setpin(S3C2410_GPF4, 1);
	s3c2410_gpio_setpin(S3C2410_GPF5, 1);
	s3c2410_gpio_setpin(S3C2410_GPF6, 1);
	s3c2410_gpio_setpin(S3C2410_GPF7, 1);

	wpu7800_gpio_init();
	
#if 0
	s3c_device_nand.dev.platform_data = &smdk_nand_info;
#endif
	
	//For s3c nand partition
	s3c_device_nand.dev.platform_data = &nand_mtd_info;

	platform_add_devices(smdk_devs, ARRAY_SIZE(smdk_devs));

	s3c2410_pm_init();
}

EXPORT_SYMBOL(gpio_wifi_power);
EXPORT_SYMBOL(gpio_wifi_power_down);
EXPORT_SYMBOL(gpio_wifi_reset);
EXPORT_SYMBOL(lcd_power);
EXPORT_SYMBOL(lcd_reset);
EXPORT_SYMBOL(lcd_backlight);
EXPORT_SYMBOL(vd_bus_inout_set);
EXPORT_SYMBOL(key_led);
EXPORT_SYMBOL(audio_ext_clock);
EXPORT_SYMBOL(speaker_amp);
EXPORT_SYMBOL(vibrator_control);
EXPORT_SYMBOL(charger_green_led);
EXPORT_SYMBOL(charger_red_led);

