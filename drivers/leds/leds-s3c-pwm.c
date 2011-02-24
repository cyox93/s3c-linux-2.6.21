/*-----------------------------------------------------------------------------
 * FILE NAME : leds-s3c-pwm.c
 * 
 * PURPOSE : LEDs PWM driver for Canopus K5
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

/*_____________________ Include Header ______________________________________*/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/leds.h>

#include <asm/hardware.h>
#include <asm/io.h>

#include <asm/plat-s3c24xx/s3c2416.h>
#include <asm/arch/regs-s3c2416-clock.h>
#include <asm/arch/regs-gpio.h>
#include <asm/arch/regs-timer.h>
#include <asm/arch/leds-pwm.h>

/*_____________________ Constants Definitions _______________________________*/
#define _MAX_PWM_BRIGHTNESS		(100)

/*_____________________ Type definitions ____________________________________*/
struct s3c_pwm_led {
	struct led_classdev		 cdev;
	struct s3c_pwm_led_platdata	*pdata;
};

/*_____________________ Imported Variables __________________________________*/

/*_____________________ Variables Definitions _______________________________*/

/*_____________________ Local Declarations __________________________________*/

/*_____________________ Program Body ________________________________________*/
static inline struct s3c_pwm_led *
_pdev_to_pwm(struct platform_device *dev)
{
	return platform_get_drvdata(dev);
}

static inline struct s3c_pwm_led *
_to_pwm(struct led_classdev *led_cdev)
{
	return container_of(led_cdev, struct s3c_pwm_led, cdev);
}

static void
_s3c_pwm_led_set(struct led_classdev *led_cdev, enum led_brightness value)
{
	struct s3c_pwm_led *led = _to_pwm(led_cdev);
	struct s3c_pwm_led_platdata *pd = led->pdata;

	unsigned int pin;
	unsigned int manual, start, reload, invert;

	unsigned long tcon;
	unsigned long tcnt=0x80;
	unsigned long tcmp=1;
	int val = (int)value;

	pin = S3C2410_GPB0 + pd->timer;
	tcon = __raw_readl(S3C2410_TCON);

	if(val < 0) val = 0;
	if(val > _MAX_PWM_BRIGHTNESS) val= _MAX_PWM_BRIGHTNESS;

	tcmp = tcnt * val / 100;
	if (tcnt == tcmp) tcmp--;

	switch (pd->timer) {
	case 0:
		start  = S3C2410_TCON_T0START;
		manual = S3C2410_TCON_T0MANUALUPD;
		invert = S3C2410_TCON_T0INVERT;
		reload = S3C2410_TCON_T0RELOAD;

		break;

	case 1:
		start  = S3C2410_TCON_T1START;
		manual = S3C2410_TCON_T1MANUALUPD;
		invert = S3C2410_TCON_T1INVERT;
		reload = S3C2410_TCON_T1RELOAD;

		break;

	case 2:
		start  = S3C2410_TCON_T2START;
		manual = S3C2410_TCON_T2MANUALUPD;
		invert = S3C2410_TCON_T2INVERT;
		reload = S3C2410_TCON_T2RELOAD;

		break;

	case 3:
		start  = S3C2410_TCON_T3START;
		manual = S3C2410_TCON_T3MANUALUPD;
		invert = S3C2410_TCON_T3INVERT;
		reload = S3C2410_TCON_T3RELOAD;

		break;

	case 4:
		start  = S3C2410_TCON_T4START;
		manual = S3C2410_TCON_T4MANUALUPD;
		invert = 0;				// not supported invert
		reload = S3C2410_TCON_T4RELOAD;

		break;
	default :
		printk(KERN_ERR "%s : Invalid timer index [%d]\n", __func__, pd->timer);
		return ;
	}

	if (tcmp) {
		// set tout
		s3c2410_gpio_cfgpin(pin, (0x02 << (1 << pd->timer)));
		s3c2410_gpio_setpin(pin, 0);

		__raw_writel(tcnt, S3C2410_TCNTB(pd->timer));
		__raw_writel(tcmp, S3C2410_TCMPB(pd->timer));

		if (!(tcon & start)) {
			tcon |= manual;
			__raw_writel(tcon, S3C2410_TCON);

			tcon &= ~(manual);
			tcon |= (start | reload) ;
			if (pd->invert) tcon |= invert;

			__raw_writel(tcon, S3C2410_TCON);
		}
	} else {
		tcon &= ~(manual | start | reload);
		__raw_writel(tcon, S3C2410_TCON);

		s3c2410_gpio_setpin(pin, 0);
		// set output
		s3c2410_gpio_cfgpin(pin, (0x01 << (1 << pd->timer)));
	}
}

static int
s3c_pwm_led_remove(struct platform_device *dev)
{
	struct s3c_pwm_led *led = _pdev_to_pwm(dev);

	led_classdev_unregister(&led->cdev);
	kfree(led);

	return 0;
}

static int
s3c_pwm_led_probe(struct platform_device *dev)
{
	unsigned long tcfg0;
	struct s3c_pwm_led_platdata *pdata = dev->dev.platform_data;
	struct s3c_pwm_led *led;
	int ret;

	/* when 66MHz PCLK, */
	tcfg0 = __raw_readl(S3C2410_TCFG0);
	tcfg0 &= ~S3C2410_TCFG_PRESCALER0_MASK;
	tcfg0 |= (PRESCALE - 1);
	__raw_writel(tcfg0, S3C2410_TCFG0);

	led = kzalloc(sizeof(struct s3c_pwm_led), GFP_KERNEL);
	if (led == NULL) {
		dev_err(&dev->dev, "No memory for device\n");
		return -ENOMEM;
	}

	platform_set_drvdata(dev, led);

	led->cdev.brightness_set = _s3c_pwm_led_set;
	led->cdev.default_trigger = pdata->def_trigger;
	led->cdev.name = pdata->name;

	led->pdata = pdata;

	_s3c_pwm_led_set(&led->cdev, pdata->brightness);

	/* register our new led device */

	ret = led_classdev_register(&dev->dev, &led->cdev);
	if (ret < 0) {
		dev_err(&dev->dev, "led_classdev_register failed\n");
		goto exit_err1;
	}

	return 0;

 exit_err1:
	kfree(led);
	return ret;
}


#ifdef CONFIG_PM
unsigned long _tcfg0;

static int
s3c_pwm_led_suspend(struct platform_device *dev, pm_message_t state)
{
	struct s3c_pwm_led *led = _pdev_to_pwm(dev);

	led_classdev_suspend(&led->cdev);

	_tcfg0 = __raw_readl(S3C2410_TCFG0);
	return 0;
}

static int
s3c_pwm_led_resume(struct platform_device *dev)
{
	struct s3c_pwm_led *led = _pdev_to_pwm(dev);

	__raw_writel(_tcfg0, S3C2410_TCFG0);

	led_classdev_resume(&led->cdev);
	return 0;
}
#else
#define s3c_pwm_led_suspend NULL
#define s3c_pwm_led_resume NULL
#endif

static struct platform_driver s3c_pwm_led_driver = {
	.probe		= s3c_pwm_led_probe,
	.remove		= s3c_pwm_led_remove,
	.suspend	= s3c_pwm_led_suspend,
	.resume		= s3c_pwm_led_resume,
	.driver		= {
		.name		= "s3c_pwm_led",
		.owner		= THIS_MODULE,
	},
};

static int __init
s3c_pwm_led_init(void)
{
	return platform_driver_register(&s3c_pwm_led_driver);
}

static void __exit
s3c_pwm_led_exit(void)
{
 	platform_driver_unregister(&s3c_pwm_led_driver);
}

/*_____________________ Linux Macro _________________________________________*/
module_init(s3c_pwm_led_init);
module_exit(s3c_pwm_led_exit);

MODULE_AUTHOR("Kim, Kyoung Ho <kimkh@udcsystems.com>");
MODULE_DESCRIPTION("S3C PWM LED driver");
MODULE_LICENSE("GPL");

