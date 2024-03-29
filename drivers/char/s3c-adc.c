/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Copyright (c) 2004 Arnaud Patard <arnaud.patard@rtp-net.org>
 * iPAQ H1940 touchscreen support
 *
 * ChangeLog
 *
 * 2004-09-05: Herbert Pötzl <herbert@13thfloor.at>
 *	- added clock (de-)allocation code
 *
 * 2005-03-06: Arnaud Patard <arnaud.patard@rtp-net.org>
 *      - h1940_ -> s3c24xx (this driver is now also used on the n30
 *        machines :P)
 *      - Debug messages are now enabled with the config option
 *        TOUCHSCREEN_S3C_DEBUG
 *      - Changed the way the value are read
 *      - Input subsystem should now work
 *      - Use ioremap and readl/writel
 *
 * 2005-03-23: Arnaud Patard <arnaud.patard@rtp-net.org>
 *      - Make use of some undocumented features of the touchscreen
 *        controller
 *
 */


#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/serio.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/clk.h>
#include <linux/mutex.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/hardware.h>
#include <asm/uaccess.h>

#include <asm/arch/regs-adc.h>
#include <asm/arch/irqs.h>


#define ADC_MINOR 	131
#define ADC_INPUT_PIN   _IOW('S', 0x0c, unsigned long)


#if defined(CONFIG_CPU_S3C2443)
#define ADC_ENV(x)  (S3C2410_ADCCON_PRSCVL(49))
#else
#define ADC_ENV(x)  (S3C2410_ADCCON_PRSCVL(49) | S3C2410_ADCCON_SELMUX(x))
#endif

struct s3c_adc_mach_info {
       int             delay;
       int             presc;
       int	       resol_bit;
};


/* ADC default configuration */
struct s3c_adc_mach_info s3c_adc_cfg __initdata = {
                .delay = 10000,
                .presc = 49,
//#if defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416) || defined(CONFIG_CPU_S3C6410)
		//.resol_bit = 12,
//#else
		.resol_bit = 10,
//#endif
};


static struct clk	*adc_clock;
static void __iomem 	*base_addr;


static DEFINE_MUTEX(adc_mutex);

static unsigned long data_for_ADCCON;
static unsigned long data_for_ADCTSC;

static int adc_port =  0;

static void s3c_adc_save_SFR_on_ADC(void) {
	
	data_for_ADCCON = readl(base_addr+S3C2410_ADCCON);
	data_for_ADCTSC = readl(base_addr+S3C2410_ADCTSC);
}

static void s3c_adc_restore_SFR_on_ADC(void) {
	
	writel(data_for_ADCCON, base_addr+S3C2410_ADCCON);
	writel(data_for_ADCTSC, base_addr+S3C2410_ADCTSC);
}


static int s3c_adc_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO " s3c_adc_open() entered\n");
	return 0;
}

static ssize_t
s3c_adc_read(struct file *file, char __user * buffer,
		size_t size, loff_t * pos)
{
	unsigned long data0;
	unsigned long data1;
	int  adc_value = 0;

	printk(KERN_INFO " s3c_adc_read() entered\n");
	
        mutex_lock(&adc_mutex);
	s3c_adc_save_SFR_on_ADC();
	
	writel(0x0, base_addr+S3C2410_ADCCON);
	writel(0x58, base_addr+S3C2410_ADCTSC);
	
	udelay(10);

	writel(ADC_ENV(adc_port)|S3C2410_ADCCON_STDBM, base_addr+S3C2410_ADCCON);
	
#if defined(CONFIG_CPU_S3C2443)
	writel(S3C2410_ADCCON_SELMUX(adc_port), base_addr+S3C2410_ADCMUX);
#endif

	udelay(10);

	writel(S3C2410_ADCCON_PRSCEN|ADC_ENV(adc_port)|S3C2410_ADCCON_ENABLE_START, base_addr+S3C2410_ADCCON);
	
       do{
	   	data0 = readl(base_addr+S3C2410_ADCCON);
	}
	while(!(data0 & S3C2410_ADCCON_ECFLG));
		
	data1 = readl(base_addr+S3C2410_ADCDAT0);
	
	adc_value = data1 & S3C2410_ADCDAT0_XPDATA_MASK;
	
	s3c_adc_restore_SFR_on_ADC();   
	mutex_unlock(&adc_mutex);

	printk(KERN_INFO " Converted Value: %03d\n",  adc_value);

	if (copy_to_user(buffer, &adc_value, sizeof(unsigned int))) {
		return -EFAULT;
	}
	return sizeof(unsigned int);
}


static int s3c_adc_ioctl(struct inode *inode, struct file *file,
	unsigned int cmd, unsigned long arg)
{
	
       printk(KERN_INFO " s3c_adc_ioctl(cmd:: %d) entered\n", cmd);

	switch (cmd) {

		case ADC_INPUT_PIN:
			  adc_port = (unsigned int) arg;
			  
                       if (adc_port >= 4)
                            printk(" %d is already reserved for TouchScreen\n", adc_port);
                      return 0;
					  
              default:
			return -ENOIOCTLCMD;
	
	}
}

static struct file_operations s3c_adc_fops = {
	.owner		= THIS_MODULE,
	.read		= s3c_adc_read,
	.open		= s3c_adc_open,
	.ioctl		= s3c_adc_ioctl,
};

static struct miscdevice s3c_adc_miscdev = {
	.minor		= ADC_MINOR,
	.name		= "adc",
	.fops		= &s3c_adc_fops,
};


/*
 * The functions for inserting/removing us as a module.
 */

static int __init s3c_adc_probe(struct platform_device *pdev)
{
	int ret;

	adc_clock = clk_get(NULL, "adc");
	if (!adc_clock) {
		printk(KERN_ERR "failed to get adc clock source\n");
		return -ENOENT;
	}
	clk_enable(adc_clock);

	base_addr=ioremap(S3C24XX_PA_ADC, 0x20);

	if (base_addr == NULL) {
		printk(KERN_ERR "Failed to remap register block\n");
		return -ENOMEM;
	}

	if ((s3c_adc_cfg.presc&0xff) > 0)
		writel(S3C2410_ADCCON_PRSCEN | S3C2410_ADCCON_PRSCVL(s3c_adc_cfg.presc&0xFF), base_addr+S3C2410_ADCCON);
	else
		writel(0, base_addr+S3C2410_ADCCON);


	/* Initialise registers */
	if ((s3c_adc_cfg.delay&0xffff) > 0)
		writel(s3c_adc_cfg.delay & 0xffff,  base_addr+S3C2410_ADCDLY);
	
	ret = misc_register(&s3c_adc_miscdev);
	if (ret) {
		printk (KERN_ERR "cannot register miscdev on minor=%d (%d)\n",
			ADC_MINOR, ret);
		return ret;
	}
	
	printk(KERN_INFO "S3C ADC driver successfully probed !\n");
	
	return 0;
}


static int s3c_adc_remove(struct device *dev)
{
	printk(KERN_INFO "s3c_adc_remove() of TS called !\n");
	return 0;
}


#define s3c_adc_suspend NULL
#define s3c_adc_resume  NULL

static struct platform_driver s3c_adc_driver = {
       .probe          = s3c_adc_probe,
       .remove         = s3c_adc_remove,
       .suspend        = s3c_adc_suspend,
       .resume         = s3c_adc_resume,
       .driver		= {
		.owner	= THIS_MODULE,
		.name	= "s3c2410-adc",
	},
};

static char banner[] __initdata = KERN_INFO "S3C ADC driver, (c) 2007 Samsung Electronics\n";

int __init s3c_adc_init(void)
{
	printk(banner);
	return platform_driver_register(&s3c_adc_driver);
}

void __exit s3c_adc_exit(void)
{
	platform_driver_unregister(&s3c_adc_driver);
}

module_init(s3c_adc_init);
module_exit(s3c_adc_exit);

MODULE_AUTHOR("alinuxguy@samsung.com>");
MODULE_DESCRIPTION("s3c adc driver");
MODULE_LICENSE("GPL");
