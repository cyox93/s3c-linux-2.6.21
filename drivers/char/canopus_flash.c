/*-----------------------------------------------------------------------------
   * FILE NAME : canopus_flash.c
   * 
   * PURPOSE : Motion detection sensor for Canopus K5
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
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/major.h>
#include <linux/module.h>
#include <linux/err.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <asm/hardware.h>

#include <linux/version.h>
#include <linux/fs.h>
#include <linux/wait.h>
#include <linux/ioport.h>
#include <linux/poll.h>     
#include <linux/proc_fs.h>
#include <asm/system.h>     
#include <asm/ioctl.h>
#include <asm/unistd.h>
#include <linux/circ_buf.h>

#include <linux/platform_device.h>

#include <asm/plat-s3c24xx/s3c2416.h>
#include <asm/arch/regs-s3c2416-clock.h>
#include <asm/arch/regs-gpio.h>
#include <asm/arch/regs-irq.h>
#include <asm/arch/regs-timer.h>


/*_____________________ Constants Definitions _______________________________*/
#define DRVNAME 			"canopus-flash"
#define DEV_NAME 			 "flash"
#define DEV_VERSION DEV_NAME "v0.1"
#define CIRC_BUF_MAX			16	
#define MAX_FLASH_BRIGHTNESS	100
#define DEF_FLASH_BRIGHTNESS	80
#define canopus_flash_suspend 	NULL
#define canopus_flash_resume  	NULL
#define FLASH_IOCTL_S_ENABLE 	_IOW ('Q', 0x80 , int)
#define FLASH_IOCTL_S_LEVEL 	_IOW ('Q', 0x81 , int)

int flash_brightness = DEF_FLASH_BRIGHTNESS;
int flash_backup_brightness = DEF_FLASH_BRIGHTNESS;
int flash_backlight_power_state = 1;
int flash_user_brightness;

/*_____________________ Local Declarations __________________________________*/
static struct platform_device *_pdev;
static int _flash_major;
static struct class *_flash_dev_class;
static int _is_enabled = false;
static void set_brightness(int);

/*_____________________ Program Body ________________________________________*/
static int
canopus_flash_ioctl(struct inode *inode, struct file *file,
				unsigned int cmd, unsigned long arg)
{

	int enable;

	switch(cmd){

	case FLASH_IOCTL_S_ENABLE:
			if (get_user(enable, (int __user *)arg))
				return -EFAULT;
			enable = (enable) ? true : false;

			if (enable != _is_enabled){
					if (enable) set_brightness(80);
					else set_brightness(0);

					_is_enabled = enable;
			}
			
			return 0;
	case FLASH_IOCTL_S_LEVEL:
				set_brightness(arg);
			return 0;

	default:
			printk(KERN_ERR "FLASH DEVICE DRIVER: No FLASH Device driver Command defined\n");
			return -ENXIO;
	}

	return 0;
}
	

static int
canopus_flash_open(struct inode *inode, struct file *file)
{
	return 0;
}


/* about fasycn funtions */
static int
canopus_flash_release(struct inode *inode, struct file *file)
{
	return 0;
}


static ssize_t 
canopus_flash_read(struct file *file, char __user *buf , size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t 
canopus_flash_write(struct file *file, const char __user *buf , size_t count, loff_t *ppos)
{
	char tbuf,vbuf[10];
	unsigned long brightness_level; 
	int cnt;

	for(cnt=0;cnt<count;cnt++ ) {
		get_user(tbuf,(char*)buf++);
		vbuf[cnt]=tbuf;
	}
	vbuf[cnt+1]=NULL;

	brightness_level = simple_strtoul(&vbuf,NULL,10);
	set_brightness(brightness_level);
 	return 0;
}

static struct 
file_operations canopus_flash_fos = {
	.owner		= THIS_MODULE,
	.ioctl		= canopus_flash_ioctl,
	.open		= canopus_flash_open,
	.release	= canopus_flash_release,
	.read 		= canopus_flash_read,
	.write 		= canopus_flash_write,
};

static int __init
canopus_flash_probe(struct platform_device *pdev)
{
	struct class_device *flash_device;

	_flash_major = register_chrdev(0, DEV_NAME, &canopus_flash_fos);

	_flash_dev_class = class_create(THIS_MODULE, DEV_NAME);
	if (IS_ERR(_flash_dev_class)) {
		printk(KERN_ERR "error creating flash dev class\n");
		return -1;
	}

	flash_device = 
		class_device_create(_flash_dev_class, 
				NULL, MKDEV(_flash_major, 0), NULL, DEV_NAME);
	if (IS_ERR(flash_device)) {
		printk(KERN_ERR "error creating flash class device\n");
		return -1;
	}

	return 0;
}

static int
canopus_flash_remove(struct device *dev)
{
	unregister_chrdev(_flash_major, DEV_NAME);
	return 0;
}


static struct 
platform_driver canopus_flash_driver = {
	.probe          = canopus_flash_probe,
	.remove         = canopus_flash_remove,
	.suspend        = canopus_flash_suspend,
	.resume         = canopus_flash_resume,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= DRVNAME,
	},
};

#if 1
static void
_backlight_set_pwm(unsigned long tcnt, unsigned long tcmp)
{
	unsigned long tcon;

	tcon = __raw_readl(S3C2410_TCON);

	if (tcmp) {
		s3c2410_gpio_cfgpin(S3C2410_GPB1, S3C2410_GPB1_TOUT1);
		s3c2410_gpio_setpin(S3C2410_GPB1, 0);

		__raw_writel(tcnt, S3C2410_TCNTB(1));
		__raw_writel(tcmp, S3C2410_TCMPB(1));

		tcon |= S3C2410_TCON_T1MANUALUPD;
		__raw_writel(tcon, S3C2410_TCON);

		tcon &= ~(S3C2410_TCON_T1MANUALUPD);
		tcon |= (S3C2410_TCON_T1START | S3C2410_TCON_T1RELOAD) ;
		__raw_writel(tcon, S3C2410_TCON);
	} else {
		tcon &= ~(S3C2410_TCON_T1MANUALUPD | S3C2410_TCON_T1START | S3C2410_TCON_T1RELOAD);
		__raw_writel(tcon, S3C2410_TCON);

		s3c2410_gpio_setpin(S3C2410_GPB1, 0);
		s3c2410_gpio_cfgpin(S3C2410_GPB1, S3C2410_GPB1_OUTP);
	}
}

static void set_brightness(int val)
{

	unsigned long tcnt=0x5000;
	unsigned long tcmp=1;

	if(val < 0) val=0;
	if(val > MAX_FLASH_BRIGHTNESS) val=MAX_FLASH_BRIGHTNESS;

	flash_brightness = val;
	if (!flash_backlight_power_state ) {
		flash_backup_brightness = val;
		return ;
	}

	tcmp = tcnt * val / 100;
	if (tcnt == tcmp) tcmp--;

//	s3c2450_timer_setup (channel, usec, tcnt, tcmp);
	_backlight_set_pwm(tcnt, tcmp);
}
#endif

int 
__init canopus_flash_init(void)
{
	int rc;

	if (!q_hw_ver(KTQOOK)) return 0;

	_pdev = platform_device_alloc(DRVNAME, 0);
	if (!_pdev)
		return -ENOMEM;

	rc = platform_device_add(_pdev);
	if (rc)
		goto undo_malloc;

	return platform_driver_register(&canopus_flash_driver);

undo_malloc:
	platform_device_put(_pdev);

	return -1;
}

void 
__exit canopus_flash_exit(void)
{
	if (!q_hw_ver(KTQOOK)) return ;

	platform_driver_unregister(&canopus_flash_driver);
	platform_device_unregister(_pdev);
}

/*_____________________ Linux Macro _________________________________________*/
module_init(canopus_flash_init);
module_exit(canopus_flash_exit);

MODULE_AUTHOR("yongsuk@udcsystems.com");
MODULE_DESCRIPTION( "flash driver");
MODULE_LICENSE( "GPL");


