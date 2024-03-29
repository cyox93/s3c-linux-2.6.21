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
#include <asm/arch/leds-pwm.h>
#include <asm/arch/regs-irq.h>

#include <asm/arch/nand.h>

#include <asm/plat-s3c24xx/common-smdk.h>
#include <asm/plat-s3c24xx/devs.h>
#include <asm/plat-s3c24xx/pm.h>
#include <asm/plat-s3c24xx/s3c2416.h>

#ifdef CONFIG_MACH_CANOPUS
#include <asm/arch/regs-s3c2416-clock.h>
#endif	// CONFIG_MACH_CANOPUS

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

static struct s3c24xx_led_platdata smdk_pdata_led_pir = {
	.gpio		= S3C2410_GPB0,
	.flags		= 0,
	.name		= "monitor-indicator",
};

static struct s3c24xx_led_platdata smdk_pdata_led_moter = {
	.gpio		= S3C2410_GPH5,
	.flags		= 0,
	.name		= "moter",
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

static struct platform_device smdk_led_pir = {
	.name		= "s3c24xx_led",
	.id		= 5,
	.dev		= {
		.platform_data = &smdk_pdata_led_pir,
	},
};

static struct platform_device smdk_led_moter = {
	.name		= "s3c24xx_led",
	.id		= 6,
	.dev		= {
		.platform_data = &smdk_pdata_led_moter,
	},
};

#ifdef CONFIG_MACH_CANOPUS
static struct s3c_pwm_led_platdata smdk_pdata_led_cam_flash = {
	.timer		= 1,
	.brightness	= 0,
	.invert		= 0,
	.name		= "camera-flash",
};

static struct platform_device smdk_led_cam_flash = {
	.name		= "s3c_pwm_led",
	.id		= 6,
	.dev		= {
		.platform_data = &smdk_pdata_led_cam_flash,
	},
};
#endif	// CONFIG_MACH_CANOPUS

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
#ifdef CONFIG_MACH_CANOPUS
struct mtd_partition s3c_partition_info[] = {
        {
                .name		= "Bootloader",
                .offset		= 0,
                .size		= (384*SZ_1K),		/* 384 KBytes */
        },
        {
                .name		= "Kernel",
                .offset		= MTDPART_OFS_APPEND,
                .size		= (5*SZ_1M),		/* 5 MBytes */
        },
        {
                .name		= "Kernel2",
                .offset		= MTDPART_OFS_APPEND,
                .size		= (5*SZ_1M),		/* 5 MBytes */
        },
        {
                .name		= "ROrign",
                .offset		= MTDPART_OFS_APPEND,
                .size		= (9*SZ_1M),		/* 9 MBytes */
        },
        {
                .name		= "Image",
                .offset		= MTDPART_OFS_APPEND,
                .size		= (40*SZ_1M),		/* 40 MBytes*/
        },
        {
                .name		= "Rootfs",
                .offset		= MTDPART_OFS_APPEND,
                .size		= (70016*SZ_1K),	/* 68.38 Mbytes */
        },
        {
                .name		= "Param",
                .offset		= MTDPART_OFS_APPEND,
                .size		= (128*SZ_1K*2),	/* 256 KBytes */
        }
};
#else	/* ! CONFIG_MACH_CANOPUS*/
struct mtd_partition s3c_partition_info[] = {
        {
                .name		= "Bootloader",
                .offset		= 0,
                .size		= (256*SZ_1K),
                .mask_flags	= MTD_CAP_NANDFLASH,
        },
        {
                .name		= "Kernel",
                .offset		= (256*SZ_1K),    /* Block number is 0x10 */
                .size		= (2*SZ_1M) - (256*SZ_1K),
                .mask_flags	= MTD_CAP_NANDFLASH,
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
#endif


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
	&smdk_led_moter,
#endif	// CONFIG_MACH_CANOPUS
};

#ifdef CONFIG_MACH_CANOPUS
static struct resource canopus_vc0528_resources[] = {
      [0] = {
              .start  = S3C_PA_SMC9115,
              .end    = S3C_PA_SMC9115 + 0x1ff,
              .flags  = IORESOURCE_MEM,
      },
};

static struct platform_device canopus_vc0528 = {
	.name		= "canopus_vc0528",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(canopus_vc0528_resources),
	.resource	= canopus_vc0528_resources,
};

static struct platform_device __initdata *kt_devs[] = {
    &smdk_led_pir,
    &smdk_led_cam_flash,
    &canopus_vc0528,
};
#endif	// CONFIG_MACH_CANOPUS

#ifdef CONFIG_MACH_CANOPUS
#include <asm/arch/regs-lcd.h>
#include <linux/delay.h>

static int _hw_version;

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
}

void gpio_wifi_power_down(bool flag)
{
	s3c2410_gpio_setpin(S3C2410_GPF7, (flag ? 0 : 1));
}

void gpio_wifi_reset(void)
{
	unsigned int pin;

	pin = S3C2410_GPF6;

	mdelay(1);
	s3c2410_gpio_setpin(pin, 0);
	mdelay(10);
	s3c2410_gpio_setpin(pin, 1);
	mdelay(1);
}

void wifi_gpio_init (void)
{	
	unsigned long mask;

	// wifi reset
	s3c2410_gpio_setpin(S3C2410_GPF6, 1);	
	s3c2410_gpio_cfgpin(S3C2410_GPF6, S3C2410_GPF6_OUTP);

	// wifi power down
	s3c2410_gpio_setpin(S3C2410_GPF7, 1);
	s3c2410_gpio_cfgpin(S3C2410_GPF7, S3C2410_GPF7_OUTP);

	s3c2410_gpio_cfgpin(S3C2410_GPL0, S3C2410_GPL0_SD0_DAT0);
	s3c2410_gpio_cfgpin(S3C2410_GPL1, S3C2410_GPL1_SD0_DAT1);
	s3c2410_gpio_cfgpin(S3C2410_GPL2, S3C2410_GPL2_SD0_DAT02);
	s3c2410_gpio_cfgpin(S3C2410_GPL3, S3C2410_GPL3_SD0_DAT3);

	s3c2410_gpio_cfgpin(S3C2410_GPL8, S3C2410_GPL8_SD0_CMD);
	s3c2410_gpio_cfgpin(S3C2410_GPL9, S3C2410_GPL9_SD0_CLK);


	s3c2410_gpio_pullup(S3C2410_GPL0, 2); /* SD DAT0 pull-up enable */
	s3c2410_gpio_pullup(S3C2410_GPL1, 2); /* SD DAT1 pull-up enable */
	s3c2410_gpio_pullup(S3C2410_GPL2, 2); /* SD DAT2 pull-up enable */
	s3c2410_gpio_pullup(S3C2410_GPL3, 2); /* SD DAT3 pull-up enable */
	s3c2410_gpio_pullup(S3C2410_GPL8, 2); /* SD CMD pull-up enable */

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
	unsigned int pin;

	pin = S3C2410_GPG6;

	mdelay(1);
	s3c2410_gpio_setpin(pin, 0);
	mdelay(10);
	s3c2410_gpio_setpin(pin, 1);
	mdelay(50);
}

static int _lcd_panel_id;

int q_lcd_panel_id(void) { return _lcd_panel_id; }

static void
_get_lcd_panel_id(void)
{
	// set gpio in for lcd panel id
	s3c2410_gpio_cfgpin(S3C2410_GPD8, S3C2410_GPD8_INP);
	s3c2410_gpio_cfgpin(S3C2410_GPD9, S3C2410_GPD9_INP);
	s3c2410_gpio_cfgpin(S3C2410_GPD10, S3C2410_GPD10_INP);

	// set pull up/down disable for lcd panel id
	s3c2410_gpio_pullup(S3C2410_GPD8, 0);
	s3c2410_gpio_pullup(S3C2410_GPD9, 0);
	s3c2410_gpio_pullup(S3C2410_GPD10, 0);

	_lcd_panel_id = (__raw_readl(S3C2410_GPDDAT) & 0x700) >> 8;
	printk(KERN_INFO "CANOPUS LCD Panel ID [0x%x]\n", _lcd_panel_id);
}

void lcd_gpio_init(void)
{
	u32 val;

	// set gpio out for lcd reset 
	s3c2410_gpio_setpin(S3C2410_GPG6, 1);
	s3c2410_gpio_cfgpin(S3C2410_GPG6, S3C2410_GPG6_OUTP);

	if (!q_hw_ver(KTQOOK)) {
		// set lcd interface
		val = __raw_readl(S3C2410_GPCCON) & 0x0fcc0;
		__raw_writel(val | 0xaaaa022a, S3C2410_GPCCON);

		// set lcd interface
		val = __raw_readl(S3C2410_GPDCON) & ~(0x003fffff);
		__raw_writel(val | 0x002aaaaa, S3C2410_GPDCON);
	} else {
		// set camera backend reset
		s3c2410_gpio_setpin(S3C2410_GPG7, 1);
		s3c2410_gpio_cfgpin(S3C2410_GPG7, S3C2410_GPG7_OUTP);

		// set clockout0 to 12MHz
		s3c2410_gpio_cfgpin(S3C2443_GPH13, S3C2443_GPH13_CLKOUT0);
	}
}

void
q_camera_backend_reset(int reset)
{
	if (q_hw_ver(KTQOOK))
		s3c2410_gpio_setpin(S3C2410_GPG7, (reset ? 0 : 1));
}

void speaker_amp(bool flag)
{
	if (flag) { 
		s3c2410_gpio_setpin(S3C2410_GPH8, 1);
	} else {
		s3c2410_gpio_setpin(S3C2410_GPH8, 0);
	}
}

int q_hw_version(void) { return _hw_version; }

#include <linux/proc_fs.h>
static struct proc_dir_entry *_proc_dir;

static int _proc_hw_version(char *buf)
{
	char *p = buf;

	p += sprintf(p, "%d\n", _hw_version);

	return p - buf;
}

static void _get_hw_version(void)
{
	_hw_version = 0;

	// set version bit to input pullup
	s3c2410_gpio_cfgpin(S3C2410_GPE7, S3C2410_GPE7_INP);
	s3c2410_gpio_pullup(S3C2410_GPE7, 2);

	s3c2410_gpio_cfgpin(S3C2410_GPE8, S3C2410_GPE8_INP);
	s3c2410_gpio_pullup(S3C2410_GPE8, 2);

	s3c2410_gpio_cfgpin(S3C2410_GPE9, S3C2410_GPE9_INP);
	s3c2410_gpio_pullup(S3C2410_GPE9, 2);

	s3c2410_gpio_cfgpin(S3C2410_GPE10, S3C2410_GPE9_INP);
	s3c2410_gpio_pullup(S3C2410_GPE10, 2);

	s3c2410_gpio_cfgpin(S3C2410_GPH6, S3C2410_GPH6_INP);
	s3c2410_gpio_pullup(S3C2410_GPH6, 2);

	_hw_version += (s3c2410_gpio_getpin(S3C2410_GPE7) ? 1 << 0 : 0);
	_hw_version += (s3c2410_gpio_getpin(S3C2410_GPE8) ? 1 << 1 : 0);
	_hw_version += (s3c2410_gpio_getpin(S3C2410_GPE9) ? 1 << 2 : 0);
	_hw_version += (s3c2410_gpio_getpin(S3C2410_GPE10) ? 1 << 3 : 0);
	_hw_version += (s3c2410_gpio_getpin(S3C2410_GPH6) ? 1 << 4 : 0);

	// set version bit to disable pullup
	s3c2410_gpio_pullup(S3C2410_GPE7, 0);
	s3c2410_gpio_pullup(S3C2410_GPE8, 0);
	s3c2410_gpio_pullup(S3C2410_GPE9, 0);
	s3c2410_gpio_pullup(S3C2410_GPE10, 0);
	s3c2410_gpio_pullup(S3C2410_GPH6, 0);

	printk(KERN_INFO "CANOPUS H/W ver. 0x%x\n", _hw_version);

	/* proc */
	_proc_dir = create_proc_entry("hwversion", 0, NULL);
	_proc_dir->read_proc = (read_proc_t *)_proc_hw_version;
	_proc_dir->data = NULL;
}

void canopus_gpio_init(void)
{
	printk("canopus_gpio_init\n");

	_get_hw_version();
	_get_lcd_panel_id();

	// Speaker AMP
	s3c2410_gpio_cfgpin(S3C2410_GPH8, S3C2410_GPH8_OUTP);
	s3c2410_gpio_setpin(S3C2410_GPH8, 0);

	// key LED
	s3c2410_gpio_cfgpin(S3C2410_GPD15, S3C2410_GPD15_OUTP);
	s3c2410_gpio_setpin(S3C2410_GPD15, 0);

	// moter
	s3c2410_gpio_cfgpin(S3C2410_GPH5, S3C2410_GPH5_OUTP);
	s3c2410_gpio_setpin(S3C2410_GPH5, 0);

   	 // Wi-Fi
	wifi_gpio_init();

	// KeyPAD 	
	Key_gpio_init();

	if (q_hw_ver(KTQOOK)) {
		// phone direction
		s3c2410_gpio_pullup(S3C2410_GPE6, 0);
		s3c2410_gpio_setpin(S3C2410_GPE6, 0);
		s3c2410_gpio_cfgpin(S3C2410_GPE6, S3C2410_GPE6_INP);
	}

	// set clockout0 for usb
	if (q_hw_ver(SWP2000)
			|| q_hw_ver(7800_MP2)) {
		s3c2410_gpio_cfgpin(S3C2443_GPH13, S3C2443_GPH13_CLKOUT0);
	} else if (q_hw_ver(KTQOOK_TP2)
			|| q_hw_ver(KTQOOK_MP)
			|| q_hw_ver(SKATM)) {
		s3c2410_gpio_cfgpin(S3C2443_GPH14, S3C2443_GPH14_CLKOUT1);
	}
}

int q_boot_flag_get(void)
{
	return __raw_readl(S3C2443_INFORM3);
}

void q_boot_flag_set(int flag)
{
	__raw_writel(flag, S3C2443_INFORM3);
}

void q_clock_init(int clkout, int out_freq)
{
	struct clk *dclk, *uclk, *clkout0;
	uint32_t freq = 0, div, cmp, val;

	uclk = clk_get(NULL, "upll");
	if (IS_ERR(uclk)) {
		printk(KERN_ERR "%s : failed to get upll\n", __func__);
	} else {
		clk_put(uclk);

		if (clkout == 0)
			dclk = clk_get(NULL, "dclk0");
		else
			dclk = clk_get(NULL, "dclk1");

		if (IS_ERR(dclk)) {
			printk(KERN_ERR "%s : failed to get dclk0\n", __func__);
		} else {
			clk_put(dclk);

			if (clkout == 0)
				clkout0 = clk_get(NULL, "clkout0");
			else
				clkout0 = clk_get(NULL, "clkout1");

			if (IS_ERR(clkout0)) {
				printk(KERN_ERR "%s : failed to get clkout0\n", __func__);
			} else {
				clk_set_parent(dclk, uclk);
				clk_set_parent(clkout0, dclk);

				// clk_enable(clkout0);

				freq = clk_get_rate(uclk);
				div = freq / out_freq;

				if (div > 0) div--;
				cmp = div/2;

				out_freq = freq / (div + 1);

				if (clkout == 0) {
					val = __raw_readl(S3C2410_DCLKCON) & ~((0xf << 8) | (0xf << 4));
					__raw_writel(val | (div << 4) | (cmp << 8), S3C2410_DCLKCON);
				} else {
					val = __raw_readl(S3C2410_DCLKCON) & ~((0xf << 24) | (0xf << 20));
					__raw_writel(val | (div << 20) | (cmp << 24), S3C2410_DCLKCON);
				}

				clk_put(uclk);

				// printk("upll[%d], div[%d], usb[%d]\n", freq, div, out_freq);
			}
		}
	}
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
	q_s3c2416_init_clocks();

	if (q_hw_ver(SWP2000)
			|| q_hw_ver(7800_MP2)) {
		q_clock_init(0, 48000000); /* for USB */
	} else if (q_hw_ver(KTQOOK_TP2)
			|| q_hw_ver(KTQOOK_MP)
			|| q_hw_ver(SKATM)) {
		q_clock_init(1, 48000000); /* for USB */
	}

	if (q_hw_ver(SKATM)) {
		q_clock_init(0, 12000000); /* for SmartCard reader */
	}
#endif	// CONFIG_MACH_CANOPUS
	
	//For s3c nand partition
	s3c_device_nand.dev.platform_data = &nand_mtd_info;

	platform_add_devices(smdk_devs, ARRAY_SIZE(smdk_devs));

	if (q_hw_ver(KTQOOK))
		platform_add_devices(kt_devs, ARRAY_SIZE(kt_devs));

	s3c2410_pm_init();

#ifdef CONFIG_MACH_CANOPUS
	q_param_proc_init();
#endif
}

#ifdef CONFIG_MACH_CANOPUS
EXPORT_SYMBOL(gpio_wifi_power);
EXPORT_SYMBOL(gpio_wifi_power_down);
EXPORT_SYMBOL(gpio_wifi_reset);
EXPORT_SYMBOL(lcd_reset);
EXPORT_SYMBOL(lcd_gpio_init);
EXPORT_SYMBOL(speaker_amp);
EXPORT_SYMBOL(q_hw_version);
EXPORT_SYMBOL(q_boot_flag_set);
EXPORT_SYMBOL(q_boot_flag_get);
EXPORT_SYMBOL(q_lcd_panel_id);
EXPORT_SYMBOL(q_camera_backend_reset);
#endif	// CONFIG_MACH_CANOPUS

