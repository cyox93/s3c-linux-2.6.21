/*-----------------------------------------------------------------------------
 * FILE NAME : ucc_drv.c
 * 
 * PURPOSE : ucc driver for WPU-7800 (make carrier by it self)
 * 
 * Copyright ⓒ 1999 - 2007 UniData Communication Systems, Inc.
 * All right reserved. 
 * 
 * This software is confidential and proprietary to UniData 
 * Communication Systems, Inc. No Part of this software may 
 * be reproduced, stored, transmitted, disclosed or used in any
 * form or by any means other than as expressly provide by the 
 * written license agreement between UniData Communication 
 * Systems and its licensee.
 *
 * NOTES: 
 *
 *---------------------------------------------------------------------------*/

/*_____________________ Include Header _______________________________________*/
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <asm/io.h>
#include <asm/hardware.h>
#include <linux/kernel.h>	
#include <linux/module.h>	
#include <linux/init.h>		
#include <linux/slab.h>		
#include <linux/stddef.h>
#include <asm/uaccess.h>	
#include <linux/fcntl.h>
#include "ucc_drv.h"
#include <linux/pm.h>
#include <linux/clk.h>

#include <linux/platform_device.h>

/*_____________________ Constants Definitions _______________________________*/
#define chr_dev_name	"ucc_dev"
#define chr_dev_major	241

/*_____________________ Program Body ________________________________________*/
static int ucc_ioctl(struct inode *inode, struct file *file,  unsigned int cmd, unsigned long arg);

static int ucc_open(struct inode *inode, struct file *filp)
{
	int number  = MINOR (inode->i_rdev);

	printk("Ucc Driver Open : Minor Number id %x\n", number);
	
	return 0;	
}

static int ucc_release(struct inode *minode, struct file *filp)
{
	printk("ucc release, \n");

	return 0;		
}

struct file_operations ucc_fops =
{
	.owner = THIS_MODULE,
	.ioctl = ucc_ioctl,	
	.open = ucc_open,
	.release = ucc_release,
};

int ucc_init(void)
{	
	int registration;

	printk("ucc_init\n");
	
	registration = register_chrdev(chr_dev_major, chr_dev_name, &ucc_fops);
	if (registration < 0)
		return registration;

	return 0;
}

void ucc_deinit(void)
{	
	printk("ucc exit \n");
	
	unregister_chrdev(chr_dev_major, chr_dev_name);	
}	

static int ucc_ioctl(struct inode *inode, struct file *file,
			  unsigned int cmd, unsigned long arg)
{	
	_t_ucc_data ucc_data;	
	unsigned long flags;
	int temp, i = 0;

	switch (cmd) {
		case IOCTLUCC_INIT:
			printk("init ucc\n");
			break;

		case IOCTLUCC_DEINIT:
			printk("deinit ucc\n");
			break;

		case IOCTLUCC_PM_MODE:	
			printk("IOCTLUCC_PM_MODE [0x%x] ... 'kernel Not Support' \n", arg);			
			break;			

		case IOCTLUCC_POWER:	
			printk("IOCTLUCC_POWER [0x%x] ... 'kernel Not Support' \n", arg);			
			break;	

		case IOCTLUCC_WLAN:		
			//printk("IOCTLUCC_WLAN [0x%x]\n", arg);
			if (arg == 0) gpio_wifi_power(0);
			if (arg == 1) gpio_wifi_power(1);
			if (arg == 2) gpio_wifi_core_power(0);
			if (arg == 3) gpio_wifi_core_power(1);
			if (arg == 0x10) gpio_wifi_reset();
			if (arg == 0x20) gpio_wifi_power_down(0);
			if (arg == 0x21) gpio_wifi_power_down(1);
			break;					

		case IOCTLUCC_AUDIO :			
			//printk("IOCTLUCC_AUDIO [0x%x]\n", arg);
			if (arg == 0) audio_ext_clock(0);
			if (arg == 1) audio_ext_clock(1);
			if (arg == 0x10) speaker_amp(0);
			if (arg == 0x11) speaker_amp(1);
			break;	

		case IOCTLUCC_KEY_LED :			
			//printk("IOCTLUCC_KEY_LED [0x%x]\n", arg);
			if (arg) 
				key_led(1);
			else
				key_led(0);
			break;	

		case IOCTLUCC_VIBRATOR :			
			//printk("IOCTLUCC_VIBRATOR [0x%x]\n", arg);
			if (arg) 
				vibrator_control(1);
			else
				vibrator_control(0);
			break;	

		case IOCTLUCC_CHARGER :			
			//printk("IOCTLUCC_CHARGER [0x%x]\n", arg);
			if (arg == 0) charger_red_led(0);
			if (arg == 1) charger_red_led(1);
			if (arg == 0x10) charger_green_led(0);
			if (arg == 0x11) charger_green_led(1);
			break;		
			
		case IOCTLUCC_CPU_REG_WRITE :			
			//printk("IOCTLUCC_CPU_REG_WRITE\n");
			if (copy_from_user((void *) &ucc_data, 
						(const void *) arg,  
						sizeof(ucc_data))) {
				return -EFAULT;
			}			
			/* GPIO Register */
			if ((ucc_data.reg_addr & 0xfff00000) == 0x56000000) 
				ucc_data.reg_addr = (ucc_data.reg_addr & 0x000fffff)|0xf6400000;
			/* LCD Register */
			if ((ucc_data.reg_addr & 0xfff00000) == 0x4c800000) 
				ucc_data.reg_addr = (ucc_data.reg_addr & 0x000fffff)|0xf0300000;
			
			local_irq_save(flags);
			__raw_writel(ucc_data.reg_value, ucc_data.reg_addr);
			local_irq_restore(flags);
			break;		

		case IOCTLUCC_CPU_REG_READ :			
			//printk("IOCTLUCC_CPU_REG_READ\n");
			if (copy_from_user((void *) &ucc_data, 
						(const void *) arg,  
						sizeof(ucc_data))) {
				return -EFAULT;
			}
			/* GPIO Register */
			if ((ucc_data.reg_addr & 0xfff00000) == 0x56000000)
				ucc_data.reg_addr = (ucc_data.reg_addr & 0x000fffff)|0xf6400000;
			/* LCD Register */
			if ((ucc_data.reg_addr & 0xfff00000) == 0x4c800000)
				ucc_data.reg_addr = (ucc_data.reg_addr & 0x000fffff)|0xf0300000;
			local_irq_save(flags);
			ucc_data.reg_value = __raw_readl(ucc_data.reg_addr);
			local_irq_restore(flags);
			copy_to_user((void *)arg,&ucc_data,sizeof(ucc_data));
			break;		

		case IOCTLUCC_PMIC_REG_WRITE :			
			//printk("IOCTLUCC_PMIC_REG_WRITE\n");
			if (copy_from_user((void *) &ucc_data, 
						(const void *) arg,  
						sizeof(ucc_data))) {
				return -EFAULT;
			}
			ucc_wm8350_reg_write(ucc_data.reg_addr, ucc_data.reg_value);
			break;		

		case IOCTLUCC_PMIC_REG_READ :			
			//printk("IOCTLUCC_PMIC_REG_READ\n");
			if (copy_from_user((void *) &ucc_data, 
						(const void *) arg,  
						sizeof(ucc_data))) {
				return -EFAULT;
			}			
			ucc_data.reg_value = ucc_wm8350_reg_read(ucc_data.reg_addr);
			copy_to_user((void *)arg,&ucc_data,sizeof(ucc_data));
			break;			

		default:
			printk("ucc : unsupported ioctl command 0x%x\n", cmd);		
		return -EINVAL;
	}
	
	return 0;
}

/*
 * Module entry points
 */

module_init(ucc_init);
module_exit(ucc_deinit);

MODULE_DESCRIPTION("ucc device driver");
MODULE_AUTHOR("Samsung");

MODULE_LICENSE("GPL");
		
