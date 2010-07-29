/* linux/arch/arm/mach-s3c2416/mach-smdk2416.c
 *
 * Copyright (c) 2007 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *	Ryu Euiyoul <ryu.real@gmail.com>
 *
 * http://www.fluff.org/ben/smdk2443/
 *
 * Thanks to Samsung for the loan of an SMDK2416
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>

#if defined (CONFIG_S3C24XX_WM8350_PMU)
#include <linux/mfd/wm8350/core.h>
#include <linux/mfd/wm8350/pmic.h>
#include <linux/mfd/wm8350/gpio.h>
#include <linux/mfd/wm8350/comparator.h>
#include <linux/fb.h>
#include <linux/regulator/machine.h>
#endif

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <asm/setup.h>
#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/mach-types.h>

#include <asm/arch/regs-serial.h>
#include <asm/arch/regs-gpio.h>
#include <asm/arch/regs-gpioj.h>
#include <asm/arch/regs-lcd.h>
#include <asm/arch/regs-mem.h>

#include <asm/arch/idle.h>
#include <asm/arch/fb.h>

#include <asm/plat-s3c24xx/s3c2410.h>
#include <asm/plat-s3c24xx/s3c2440.h>
#include <asm/plat-s3c24xx/clock.h>
#include <asm/plat-s3c24xx/devs.h>
#include <asm/plat-s3c24xx/cpu.h>

#include <asm/plat-s3c24xx/common-smdk.h>
#include <asm/plat-s3c24xx/s3c2416.h>

#include <asm/arch/nand.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#include <asm/arch/hsmmc.h>

static struct map_desc smdk2416_iodesc[] __initdata = {
	IODESC_ENT(CS8900),
};

#define UCON S3C2410_UCON_DEFAULT | S3C2440_UCON_FCLK
#define ULCON S3C2410_LCON_CS8 | S3C2410_LCON_PNONE | S3C2410_LCON_STOPB
#define UFCON S3C2410_UFCON_RXTRIG8 | S3C2410_UFCON_FIFOMODE

static struct s3c24xx_uart_clksrc smdk2416_serial_clocks[] = {
	[0] = {
		.name		= "pclk",
		.divisor	= 1,
		.min_baud	= 0,
		.max_baud	= 0,
	},
	[1] = {
		.name		= "esysclk",
		.divisor	= 1,
		.min_baud	= 0,
		.max_baud	= 0,
	}

};

static struct s3c2410_uartcfg smdk2416_uartcfgs[] __initdata = {
	[0] = {
		.hwport	     = 0,
		.flags	     = 0,
		.ucon	     = 0x3c5,
		.ulcon	     = 0x03,
		.ufcon	     = 0x51,
	},
	[1] = {
		.hwport	     = 1,
		.flags	     = 0,
		/* Use PCLK */
		.ucon	     = 0x3c5,
		//.ucon	     = 0xfc5,
		.ulcon	     = 0x03,
		.ufcon	     = 0x51,
		.clocks	     = smdk2416_serial_clocks,
		.clocks_size = ARRAY_SIZE(smdk2416_serial_clocks),
	},
	/* IR port */
	[2] = {
		.hwport	     = 2,
		.flags	     = 0,
		.ucon	     = 0x3c5,
//		.ucon	     = 0xfc5,
		.ulcon	     = 0x03,
//		.ulcon	     = 0x43,
		.ufcon	     = 0x51,
		.clocks	     = smdk2416_serial_clocks,
		.clocks_size = ARRAY_SIZE(smdk2416_serial_clocks),
	}
};

#ifdef CONFIG_MACH_CANOPUS
// for atheros power maangement driver
static struct resource wlan_ar6000_pm_dev_resource[] = {
	[0] = {
		.start = IRQ_EINT4,
		.end   = IRQ_EINT4,
		.flags = IORESOURCE_IRQ,
	}
};

static struct platform_device wlan_ar6000_pm_dev = {
	.name			= "wlan_ar6000_pm_dev",
	.id			= -1,
	.num_resources		= ARRAY_SIZE(wlan_ar6000_pm_dev_resource),
	.resource		= wlan_ar6000_pm_dev_resource,
};
#endif

static struct platform_device *smdk2416_devices[] __initdata = {
	&s3c_device_spi0,
	&s3c_device_spi1,
	&s3c_device_wdt,
	&s3c_device_i2c,
	&s3c_device_lcd,
	&s3c_device_rtc,
	&s3c_device_adc,
	&s3c_device_iis,
	&s3c_device_usbgadget,
	&s3c_device_usb,
	//&s3c_device_hsmmc0,
	&s3c_device_hsmmc1,
#ifndef CONFIG_MACH_CANOPUS
  	&s3c_device_smc911x,
#else
	&wlan_ar6000_pm_dev,
#endif
	&s3c_device_keypad,

};

static struct s3c24xx_board smdk2416_board __initdata = {
	.devices       = smdk2416_devices,
	.devices_count = ARRAY_SIZE(smdk2416_devices)
};

static void __init smdk2416_map_io(void)
{
	s3c24xx_init_io(smdk2416_iodesc, ARRAY_SIZE(smdk2416_iodesc));
	s3c24xx_init_clocks(12000000);
	s3c24xx_init_uarts(smdk2416_uartcfgs, ARRAY_SIZE(smdk2416_uartcfgs));
	s3c24xx_set_board(&smdk2416_board);
}

static void smdk2416_cs89x0_set(void)
{
	u32 val;

	val = readl(S3C_BANK_CFG);
	val &= ~((1<<8)|(1<<9)|(1<<10));
	writel(val, S3C_BANK_CFG);

	/* Bank1 Idle cycle ctrl. */
	writel(0xf, S3C_SSMC_SMBIDCYR1);

	/* Bank1 Read Wait State cont. = 14 clk          Tacc? */
	writel(12, S3C_SSMC_SMBWSTRDR1);

	/* Bank1 Write Wait State ctrl. */
	writel(12, S3C_SSMC_SMBWSTWRR1);

	/* Bank1 Output Enable Assertion Delay ctrl.     Tcho? */
	writel(2, S3C_SSMC_SMBWSTOENR1);

	/* Bank1 Write Enable Assertion Delay ctrl. */
	writel(2, S3C_SSMC_SMBWSTWENR1);

	/* SMWAIT active High, Read Byte Lane Enabl      WS1? */
	val = readl(S3C_SSMC_SMBCR1);

	val |=  ((1<<15)|(1<<7));
	writel(val, S3C_SSMC_SMBCR1);

	val = readl(S3C_SSMC_SMBCR1);
	val |=  ((1<<2)|(1<<0));
	writel(val, S3C_SSMC_SMBCR1);

	val = readl(S3C_SSMC_SMBCR1);
	val &= ~((3<<20)|(3<<12));
	writel(val, S3C_SSMC_SMBCR1);

	val = readl(S3C_SSMC_SMBCR1);
	val &= ~(3<<4);
	writel(val, S3C_SSMC_SMBCR1);

	val = readl(S3C_SSMC_SMBCR1);
	val |= (1<<4);

	writel(val, S3C_SSMC_SMBCR1);

}

static void smdk2416_smc911x_set(void)
{
	u32 val;

	/* Bank1 Idle cycle ctrl. */
	writel(0xf, S3C_SSMC_SMBIDCYR4);

	/* Bank1 Read Wait State cont. = 14 clk          Tacc? */
	writel(12, S3C_SSMC_SMBWSTRDR4);

	/* Bank1 Write Wait State ctrl. */
	writel(12, S3C_SSMC_SMBWSTWRR4);

	/* Bank1 Output Enable Assertion Delay ctrl.     Tcho? */
	writel(2, S3C_SSMC_SMBWSTOENR4);

	/* Bank1 Write Enable Assertion Delay ctrl. */
	writel(2, S3C_SSMC_SMBWSTWENR4);

	/* SMWAIT active High, Read Byte Lane Enabl      WS1? */
	val = readl(S3C_SSMC_SMBCR4);

	val |=  ((1<<15)|(1<<7));
	writel(val, S3C_SSMC_SMBCR4);

	val = readl(S3C_SSMC_SMBCR4);
	val |=  ((1<<2)|(1<<0));
	writel(val, S3C_SSMC_SMBCR4);

	val = readl(S3C_SSMC_SMBCR4);
	val &= ~((3<<20)|(3<<12));
	writel(val, S3C_SSMC_SMBCR4);

	val = readl(S3C_SSMC_SMBCR4);
	val &= ~(3<<4);
	writel(val, S3C_SSMC_SMBCR4);

	val = readl(S3C_SSMC_SMBCR4);
	val |= (1<<4);

	writel(val, S3C_SSMC_SMBCR4);

}

#if defined (CONFIG_S3C24XX_WM8350_PMU)
static void s3c_nop_release(struct device *dev)
{
	/* Nothing */
}

static struct platform_device s3c_wm8350_codec_device = {
	.name = "wm8350-codec",
	.id = 1,
	.dev = {
		.release = s3c_nop_release,
	},
};

static struct platform_device s3c_wm8350_pcm_device = {
	.name = "s3c24xx-pcm",
	.id = 1,
	.dev = {
		.release = s3c_nop_release,
	},
};

static struct platform_device s3c_wm8350_iis_device = {
	.name = "s3c-i2s",
	.id = 1,
	.dev = {
		.release = s3c_nop_release,
	},
};

static struct platform_device wm8350_power_device = {
	.name = "wm8350-power",
	.dev = {
		.release = s3c_nop_release,
	},
};

static struct platform_device s3c_audio_device = {
	.name = "smdk2416-audio",
	.dev = {
		.release = s3c_nop_release,
	},
};


static struct regulator_consumer_supply dcdc1_consumers[] = {
{
	.dev	= NULL,
	.supply	= "cpu_vcc",
},};

/* CPU */
static struct regulator_init_data dcdc1_data = {
	.constraints = {
		.min_uV = 1275000,
		.max_uV = 1600000,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
			REGULATOR_CHANGE_MODE,
		.valid_modes_mask = REGULATOR_MODE_NORMAL |
			REGULATOR_MODE_FAST,
		.state_mem = {
			.uV = 1300000,
			.mode = REGULATOR_MODE_NORMAL,
			.enabled = 1,
		},
		.initial_state = PM_SUSPEND_MEM,
		.always_on = 1,
		.boot_on = 1,
	},
	.num_consumer_supplies = ARRAY_SIZE(dcdc1_consumers),
	.consumer_supplies = dcdc1_consumers,
};

/* System IO - Low */
static struct regulator_init_data dcdc3_data = {
	.constraints = {
		.min_uV = 1800000,
		.max_uV = 1800000,
		.state_mem = {
			.uV = 1800000,
			.mode = REGULATOR_MODE_NORMAL,
			.enabled = 1,
		},
		.initial_state = PM_SUSPEND_MEM,
		.always_on = 1,
		.boot_on = 1,
	},
};

/* System IO - High */
static struct regulator_init_data dcdc4_data = {
	.constraints = {
		.min_uV = 3400000,
		.max_uV = 3400000,
		.state_mem = {
			.uV = 3400000,
			.mode = REGULATOR_MODE_NORMAL,
			.enabled = 1,
		},
		.initial_state = PM_SUSPEND_MEM,
		.always_on = 1,
		.boot_on = 1,
	},
};

static struct regulator_init_data ldo1_data = {
	.constraints = {
		.min_uV = 1200000,
		.max_uV = 1200000,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
		.apply_uV = 1,
	},
//	.num_consumer_supplies = ARRAY_SIZE(ldo2_consumers),
//	.consumer_supplies = ldo2_consumers,
};

/* Wifi 1.8V */
static struct regulator_consumer_supply ldo2_consumers[] = {
	{
		.supply	= "wifi-io",
	},
};

static struct regulator_init_data ldo2_data = {
	.constraints = {
		.min_uV = 1800000,
		.max_uV = 1800000,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
		.apply_uV = 1,
		.always_on = 1,
		.boot_on = 1,
	},
	.num_consumer_supplies = ARRAY_SIZE(ldo2_consumers),
	.consumer_supplies = ldo2_consumers,
};

static struct regulator_consumer_supply ldo3_consumers[] = {
{
	.dev	= &s3c_wm8350_codec_device.dev,
	.supply	= "codec_avdd",
},};

/* Codec */
static struct regulator_init_data ldo3_data = {
	.constraints = {
		.min_uV = 3300000,
		.max_uV = 3300000,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
		.apply_uV = 1,
		.always_on = 1,
		.boot_on = 1,
	},
	.num_consumer_supplies = ARRAY_SIZE(ldo3_consumers),
	.consumer_supplies = ldo3_consumers,
};

/* Wifi 1.2V */
static struct regulator_consumer_supply ldo4_consumers[] = {
	{
		.supply	= "wifi-core",
	},
};

static struct regulator_init_data ldo4_data = {
	.constraints = {
		.min_uV = 1200000,
		.max_uV = 1200000,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
		.apply_uV = 1,
		.always_on = 1,
		.boot_on = 1,
	},
	.num_consumer_supplies = ARRAY_SIZE(ldo4_consumers),
	.consumer_supplies = ldo4_consumers,
};

static struct platform_device wm8350_regulator_devices[] = {
{
	.name = "wm8350-regulator",
	.id = WM8350_DCDC_1,
	.dev = {
		.platform_data = &dcdc1_data,
	},
},
{
	.name = "wm8350-regulator",
	.id = WM8350_DCDC_3,
	.dev = {
		.platform_data = &dcdc3_data,
	},
},
{
	.name = "wm8350-regulator",
	.id = WM8350_DCDC_4,
	.dev = {
		.platform_data = &dcdc4_data,
	},
},
{
	.name = "wm8350-regulator",
	.id = WM8350_LDO_1,
	.dev = {
		.platform_data = &ldo1_data,
	},
},
{
	.name = "wm8350-regulator",
	.id = WM8350_LDO_2,
	.dev = {
		.platform_data = &ldo2_data,
	},
},
{
	.name = "wm8350-regulator",
	.id = WM8350_LDO_3,
	.dev = {
		.platform_data = &ldo3_data,
	},
},
{
	.name = "wm8350-regulator",
	.id = WM8350_LDO_4,
	.dev = {
		.platform_data = &ldo4_data,
	},
},
};

static void init_wm8350_audio(void)
{
	int err;

	err = platform_device_register(&s3c_wm8350_pcm_device);
	if (err < 0) {
		dev_err(&s3c_wm8350_pcm_device.dev,
				"Unable to register WM8350 pcm device\n");
		return;
	}

	err = platform_device_register(&s3c_wm8350_iis_device);
	if (err < 0) {
		dev_err(&s3c_wm8350_iis_device.dev,
				"Unable to register WM8350 i2s device\n");
		return;
	}
}

static inline void s3c_init_wm8350(void)
{
	init_wm8350_audio();
}

struct wm8350_charger_policy wm8350_charger = {
	.eoc_mA			= 90,/* end of charge current (mA)  */
	.charge_mV		= WM8350_CHG_4_20V, /* charge voltage */
	.fast_limit_mA		= 750,/* fast charge current limit */
	.fast_limit_USB_mA	= 400,/* USB fast charge current limit */
	.charge_timeout		= 300,	/* charge timeout (mins) */
	.trickle_start_mV	= WM8350_CHG_TRICKLE_3_1V, /* trickle charge starts at mV */
	.trickle_charge_mA	= WM8350_CHG_TRICKLE_100mA, /* trickle charge current */
};

int s3c_wm8350_device_register(struct wm8350 *wm8350)
{
	int err;

	platform_set_drvdata(&wm8350_power_device, wm8350);
	wm8350->power.policy = &wm8350_charger;
	err = platform_device_register(&wm8350_power_device);
	if (err < 0) {
		dev_err(&wm8350_power_device.dev,
				"Unable to register WM8350 Power device\n");
		return err;
	}

	platform_set_drvdata(&s3c_audio_device, wm8350);
	err = platform_device_register(&s3c_audio_device);
	if (err < 0) {
		dev_err(&s3c_audio_device.dev,
				"Unable to register WM8350 Audio device\n");
		return err;
	}

	platform_set_drvdata(&s3c_wm8350_codec_device, wm8350);
	err = platform_device_register(&s3c_wm8350_codec_device);
	if (err < 0) {
		dev_err(&s3c_wm8350_codec_device.dev,
				"Unable to register WM8350 codec device\n");
		return err;
	}


	return err;
}

#define WM8350_I2C_ADDR			(0x34 >> 1)

static int config_s3c_wm8350_gpio(struct wm8350 *wm8350)
{
	wm8350_gpio_config(wm8350, 2, WM8350_GPIO_DIR_OUT,
			   WM8350_GPIO2_32KHZ_OUT, WM8350_GPIO_ACTIVE_LOW,
			   WM8350_GPIO_PULL_NONE, WM8350_GPIO_INVERT_OFF,
			   WM8350_GPIO_DEBOUNCE_OFF);

	// cradle led
	if (!q_hw_ver(SKBB)) {
		wm8350_gpio_config(wm8350, 10, WM8350_GPIO_DIR_IN,
				   WM8350_GPIO10_GPIO_IN, WM8350_GPIO_ACTIVE_LOW,
				   WM8350_GPIO_PULL_NONE, WM8350_GPIO_INVERT_OFF,
				   WM8350_GPIO_DEBOUNCE_OFF);

		wm8350_gpio_config(wm8350, 11, WM8350_GPIO_DIR_IN,
				   WM8350_GPIO11_GPIO_IN, WM8350_GPIO_ACTIVE_LOW,
				   WM8350_GPIO_PULL_NONE, WM8350_GPIO_INVERT_OFF,
				   WM8350_GPIO_DEBOUNCE_OFF);
	} else {
		wm8350_gpio_config(wm8350, 10, WM8350_GPIO_DIR_OUT,
				   WM8350_GPIO10_GPIO_OUT, WM8350_GPIO_ACTIVE_LOW,
				   WM8350_GPIO_PULL_NONE, WM8350_GPIO_INVERT_OFF,
				   WM8350_GPIO_DEBOUNCE_OFF);

		wm8350_gpio_config(wm8350, 11, WM8350_GPIO_DIR_OUT,
				   WM8350_GPIO11_GPIO_OUT, WM8350_GPIO_ACTIVE_LOW,
				   WM8350_GPIO_PULL_NONE, WM8350_GPIO_INVERT_OFF,
				   WM8350_GPIO_DEBOUNCE_OFF);
	}

	// line sw
	wm8350_gpio_config(wm8350, 12, WM8350_GPIO_DIR_OUT,
			   WM8350_GPIO12_LINE_EN_OUT, WM8350_GPIO_ACTIVE_LOW,
			   WM8350_GPIO_PULL_NONE, WM8350_GPIO_INVERT_OFF,
			   WM8350_GPIO_DEBOUNCE_ON);

	/* Chager LED initial */
	wm8350_gpio_set_status(wm8350, 10, 1);
	wm8350_gpio_set_status(wm8350, 11, 1);

	// avoid lcd backlight flicker
	if (q_hw_ver(KTQOOK_TP)) {
		wm8350_gpio_config(wm8350, 4, WM8350_GPIO_DIR_OUT,
				   WM8350_GPIO11_GPIO_OUT, WM8350_GPIO_ACTIVE_LOW,
				   WM8350_GPIO_PULL_NONE, WM8350_GPIO_INVERT_OFF,
				   WM8350_GPIO_DEBOUNCE_OFF);
		wm8350_gpio_set_status(wm8350, 4, 1);
	}

	return 0;
}

//static int wm8350_init(struct wm8350 *wm8350)
int wm8350_dev_init(struct wm8350 *wm8350)
{
	int i, ret;
	u16 data;

#if 0
	/* dont assert RTS when hibernating */
	wm8350_set_bits(wm8350, WM8350_SYSTEM_HIBERNATE, WM8350_RST_HIB_MODE);
#endif

	wm8350_reg_unlock(wm8350);
	wm8350_set_bits(wm8350, WM8350_SYSTEM_CONTROL_1, WM8350_IRQ_POL);
	wm8350_reg_lock(wm8350);

	s3c2410_gpio_pullup(S3C2410_GPF1, 0);
	s3c2410_gpio_cfgpin(S3C2410_GPF1, S3C2410_GPF1_EINT1);
//	set_irq_type(IRQ_EINT1, IRQT_BOTHEDGE);

	/* Shutdown threshold value 3.1v off , 3.2v on */
	wm8350_reg_unlock(wm8350);
	data = wm8350_reg_read(wm8350, WM8350_POWER_CHECK_COMPARATOR)
		& ~(WM8350_PCCMP_OFF_THR_MASK | WM8350_PCCMP_ON_THR_MASK);
	wm8350_reg_write(wm8350, WM8350_POWER_CHECK_COMPARATOR, data | 0x23);
	wm8350_reg_lock(wm8350);

	data = wm8350_reg_read(wm8350, WM8350_DIGITISER_CONTROL_2);
	wm8350_reg_write(wm8350, WM8350_DIGITISER_CONTROL_2, data | WM8350_AUXADC_CAL);

	config_s3c_wm8350_gpio(wm8350);

#if 0
	/* Sw1 --> PWR_ON */
	wm8350_register_irq(wm8350, WM8350_IRQ_WKUP_ONKEY,
			    imx32ads_switch_handler, NULL);
	wm8350_unmask_irq(wm8350, WM8350_IRQ_WKUP_ONKEY);
#endif

	for (i = 0; i < ARRAY_SIZE(wm8350_regulator_devices); i++) {
		platform_set_drvdata(&wm8350_regulator_devices[i], wm8350);
		ret = platform_device_register(&wm8350_regulator_devices[i]);
		if (ret < 0)
			goto unwind;
	}

	/* now register other clients */
	return s3c_wm8350_device_register(wm8350);
unwind:
	for (i--; i >= 0; i--)
		platform_device_unregister(&wm8350_regulator_devices[i]);

	return ret;
}
EXPORT_SYMBOL(wm8350_dev_init);

#if 0
static struct wm8350_platform_data wm8350_data = {
	.board_pmic_init = wm8350_dev_init,
};


static const struct i2c_board_info s3c_i2c_info[] = {
{
	.driver_name	= "wm8350-i2c",
	.type		= "wm8350-i2c",
	.addr		= WM8350_I2C_ADDR,
	.platform_data	= &wm8350_data,
	.irq		= IRQ_EINT12,},
};

static int s3c_register_wm8350(void)
{
	return i2c_register_board_info(0, s3c_i2c_info,
		ARRAY_SIZE(s3c_i2c_info));
}
#endif
#endif

static void __init smdk2416_machine_init(void)
{
#ifndef CONFIG_MACH_CANOPUS
	/* SROM init for NFS */
	//smdk2416_cs89x0_set();
	smdk2416_smc911x_set();
#endif	// CONFIG_MACH_CANOPUS

	smdk_machine_init();

#if defined (CONFIG_S3C24XX_WM8350_PMU) 
//	s3c_register_wm8350();
	s3c_init_wm8350();
#endif
}

static void __init smdk2416_fixup (struct machine_desc *desc, struct tag *tags,
	      char **cmdline, struct meminfo *mi)
{
	/*
	 * Bank start addresses are not present in the information
	 * passed in from the boot loader.  We could potentially
	 * detect them, but instead we hard-code them.
	 */
	mi->bank[0].start = 0x30000000;

#if defined(CONFIG_MACH_CANOPUS)
	mi->bank[0].size = 64*1024*1024;
#else	// CONFIG_MACH_CANOPUS
#if defined(CONFIG_VIDEO_SAMSUNG)
	mi->bank[0].size = 49*1024*1024;
#elif defined(CONFIG_PP_S3C2443)
	mi->bank[0].size = 60*1024*1024;
#else
	mi->bank[0].size = 64*1024*1024;
#endif
#endif	// CONFIG_MACH_CANOPUS
	mi->bank[0].node = 0;

	mi->nr_banks = 1;
}


MACHINE_START(SMDK2416, "SMDK2416")
	/* Maintainer: Ben Dooks <ben@fluff.org> */
	.phys_io	= S3C2410_PA_UART,
	.io_pg_offst	= (((u32)S3C24XX_VA_UART) >> 18) & 0xfffc,
	.boot_params	= S3C2410_SDRAM_PA + 0x100,

	.init_irq	= s3c24xx_init_irq,
	.map_io		= smdk2416_map_io,
	.fixup      	= smdk2416_fixup,
	.init_machine	= smdk2416_machine_init,
	.timer		= &s3c24xx_timer,
MACHINE_END


/*
 * HS-MMC GPIO Set function for S3C2416 SMDK board
 */
void hsmmc_set_gpio (uint channel, uint width)
{
	switch (channel) {

	/* can supports 1 and 4 bit bus */
	case 0:
		/* GPIO E : Command, Clock */
		s3c2410_gpio_cfgpin(S3C2410_GPE5,  S3C2450_GPE5_SD0_CLK);
		s3c2410_gpio_cfgpin(S3C2410_GPE6, S3C2450_GPE6_SD0_CMD);

		if (width == 1) {
			/* GPIO E : MMC DATA0[0] */
			s3c2410_gpio_cfgpin(S3C2410_GPE7,  S3C2450_GPE7_SD0_DAT0);
		}
		else if (width == 4) {
			/* GPIO E : MMC DATA0[0:3] */
			s3c2410_gpio_cfgpin(S3C2410_GPE7,  S3C2450_GPE7_SD0_DAT0);
			s3c2410_gpio_cfgpin(S3C2410_GPE8,  S3C2450_GPE8_SD0_DAT1);
			s3c2410_gpio_cfgpin(S3C2410_GPE9,  S3C2450_GPE9_SD0_DAT2);
			s3c2410_gpio_cfgpin(S3C2410_GPE10,  S3C2450_GPE10_SD0_DAT3);
		}
		break;

	/* can supports 1, 4, and 8 bit bus */
	case 1:
		/* GPIO L : Command, Clock */
		s3c2410_gpio_cfgpin(S3C2443_GPL8, S3C2450_GPL8_SD1CMD);
		s3c2410_gpio_cfgpin(S3C2443_GPL9, S3C2450_GPL9_SD1CLK);

		/* GPIO J : Chip detect, LED, Write Protect */
		s3c2410_gpio_cfgpin(S3C2443_GPJ13, S3C2450_GPJ13_SD1LED);
		s3c2410_gpio_cfgpin(S3C2443_GPJ14, S3C2450_GPJ14_nSD1CD);

		s3c2410_gpio_cfgpin(S3C2443_GPJ15, S3C2450_GPJ15_nSD1WP); /* write protect enable */
		s3c2410_gpio_setpin(S3C2443_GPJ15, 1);

		if (width == 1) {
			/* GPIO L : MMC DATA1[0] */
			s3c2410_gpio_cfgpin(S3C2443_GPL0, S3C2450_GPL0_SD1DAT0);
		}
		else if (width == 4) {
			/* GPIO L : MMC DATA1[0:3] */
			s3c2410_gpio_cfgpin(S3C2443_GPL0, S3C2450_GPL0_SD1DAT0);
			s3c2410_gpio_cfgpin(S3C2443_GPL1, S3C2450_GPL1_SD1DAT1);
			s3c2410_gpio_cfgpin(S3C2443_GPL2, S3C2450_GPL2_SD1DAT2);
			s3c2410_gpio_cfgpin(S3C2443_GPL3, S3C2450_GPL3_SD1DAT3);
		}
		else if (width == 8) {
			/* GPIO L : MMC DATA1[0:7] */
			s3c2410_gpio_cfgpin(S3C2443_GPL0, S3C2450_GPL0_SD1DAT0);
			s3c2410_gpio_cfgpin(S3C2443_GPL1, S3C2450_GPL1_SD1DAT1);
			s3c2410_gpio_cfgpin(S3C2443_GPL2, S3C2450_GPL2_SD1DAT2);
			s3c2410_gpio_cfgpin(S3C2443_GPL3, S3C2450_GPL3_SD1DAT3);

			s3c2410_gpio_cfgpin(S3C2443_GPL4, S3C2450_GPL4_SD1DAT4);
			s3c2410_gpio_cfgpin(S3C2443_GPL5, S3C2450_GPL5_SD1DAT5);
			s3c2410_gpio_cfgpin(S3C2443_GPL6, S3C2450_GPL6_SD1DAT6);
			s3c2410_gpio_cfgpin(S3C2443_GPL7, S3C2450_GPL7_SD1DAT7);
		}
		break;

	default:
		break;
	}
}

EXPORT_SYMBOL(hsmmc_set_gpio);

#define HOST_CAPS (MMC_CAP_4_BIT_DATA | MMC_CAP_MULTIWRITE | \
			MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED)

/* Channel 0 : added HS-MMC channel */
struct s3c_hsmmc_cfg s3c_hsmmc0_platform = {
	.hwport = 0,
	.enabled = 1,
	.host_caps = HOST_CAPS,
	.bus_width = 4,
	.highspeed = 0,

	/* ctrl for mmc */
	.fd_ctrl[MMC_MODE_MMC] = {
		.ctrl2 = 0xC0000100,			/* ctrl2 for mmc */
		.ctrl3[SPEED_NORMAL] = 0,		/* ctrl3 for low speed */
		.ctrl3[SPEED_HIGH]   = 0,		/* ctrl3 for high speed */
		.ctrl4 = 0x3,
	},

	/* ctrl for sd */
	.fd_ctrl[MMC_MODE_SD] = {
		.ctrl2 = 0xC0000100,			/* ctrl2 for sd */
		.ctrl3[SPEED_NORMAL] = 0,		/* ctrl3 for low speed */
		.ctrl3[SPEED_HIGH]   = 0,		/* ctrl3 for high speed */
		.ctrl4 = 0x3,
	},

	.clk_name[0] = "hsmmc",				/* 1st clock source */
	.clk_name[1] = "esysclk",			/* 2nd clock source hsmmc-epll by Ben Dooks */
	.clk_name[2] = "hsmmc-ext",			/* 3rd clock source */
};

/* Channel 1 : default HS-MMC channel */
struct s3c_hsmmc_cfg s3c_hsmmc1_platform = {
	.hwport = 1,
	.enabled = 1,
	.host_caps = HOST_CAPS | MMC_CAP_8_BIT_DATA,
	.bus_width = 8,
	.highspeed = 0,

	/* ctrl for mmc */
	.fd_ctrl[MMC_MODE_MMC] = {
		.ctrl2 = 0xC0000100,			/* ctrl2 for mmc */
		.ctrl3[SPEED_NORMAL] = 0,		/* ctrl3 for low speed */
		.ctrl3[SPEED_HIGH]   = 0,		/* ctrl3 for high speed */
		.ctrl4 = 0x3,
	},

	/* ctrl for sd */
	.fd_ctrl[MMC_MODE_SD] = {
		.ctrl2 = 0xC0000100,			/* ctrl2 for sd */
		.ctrl3[SPEED_NORMAL] = 0,		/* ctrl3 for low speed */
		.ctrl3[SPEED_HIGH]   = 0,		/* ctrl3 for high speed */
		.ctrl4 = 0x3,
	},

	.clk_name[0] = "hsmmc",				/* 1st clock source */
	.clk_name[1] = "esysclk",			/* 2nd clock source hsmmc-epll by Ben Dooks */
	.clk_name[2] = "hsmmc-ext",			/* 3rd clock source */
};

#ifdef CONFIG_MACH_CANOPUS
void
gpio_wifi_core_power(bool flag)
{
	struct regulator *wifi_io = regulator_get(NULL, "wifi-io");
	struct regulator *wifi_core = regulator_get(NULL, "wifi-core");

	if (flag) {
		if (wifi_core) regulator_enable(wifi_core);
		if (wifi_io) regulator_enable(wifi_io);
	} else {
		if (wifi_io) regulator_disable(wifi_io);
		if (wifi_core) regulator_disable(wifi_core);
	}
}

EXPORT_SYMBOL(gpio_wifi_core_power);
#endif	// CONFIG_MACH_CANOPUS
