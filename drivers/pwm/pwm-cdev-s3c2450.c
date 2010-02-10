/*-----------------------------------------------------------------------------
 * FILE NAME : pwm-cdev-s3c2450.c
 * 
 * PURPOSE : pwm character driver for WPU-7800 (make carrier by it self)
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
/*_____________________ Include Header _______________________________________*/
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>

/*_____________________ Constants Definitions _______________________________*/
#define	PWM_MINOR	133		
#define PWM_USEC	10

#define IOCTL_PWM_START		_IOWR('p', 0xb6, int)
#define IOCTL_PWM_STOP		_IOWR('p', 0xb7, int)

extern int s3c2450_timer_setup(int channel, int usec, unsigned long g_tcnt, unsigned long g_tcmp);
extern int s3c2450_pwm_stop(int channel);

typedef struct {
	int channel;
	unsigned long g_tcnt;
	unsigned long g_tcmp;
} _t_s3c2450_pwm_data;

/*_____________________ Program Body ________________________________________*/
static int s3c2450_pwm_open(struct inode *inode, struct file *filp)
{
	return	0; 
}

static int s3c2450_pwm_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static int s3c2450_pwm_ioctl(struct inode *inode, struct file *file,
				unsigned int cmd, unsigned long arg)
{
	_t_s3c2450_pwm_data pwm_data = {0};

	switch(cmd) {
		case IOCTL_PWM_START:
			if (copy_from_user((void *)&pwm_data,
						(const void *)arg,
						sizeof(pwm_data))) 
				return -EFAULT;
			
			if (pwm_data.channel > 3) {
				printk("s3c pwm : unsupported channel %d\n", pwm_data.channel);
				return -EINVAL; 
			}

			if (pwm_data.g_tcnt < pwm_data.g_tcmp) {
				printk("s3c pwm : unsupported duty\n");
				return -EINVAL;
			}
			
			s3c2450_timer_setup(pwm_data.channel, PWM_USEC,
					pwm_data.g_tcnt, pwm_data.g_tcmp);
			break;

		case IOCTL_PWM_STOP:
			if (copy_from_user((void *)&pwm_data,
						(const void *)arg,
						sizeof(pwm_data))) 
				return -EFAULT;
			
			if (pwm_data.channel > 3) {
				printk("s3c pwm : unsupported channel %d\n", pwm_data.channel);
				return -EINVAL; 
			}

			s3c2450_pwm_stop(pwm_data.channel);
			break;

		default:
			printk("s3c pwm : unsupported ioctl command 0x%x\n", cmd);
			break;
	}

	return 0;
}

static struct file_operations s3c2450_pwm_fops = {
	.owner		= THIS_MODULE,
	.ioctl		= s3c2450_pwm_ioctl,
	.open		= s3c2450_pwm_open,
	.release	= s3c2450_pwm_release,
};

static struct miscdevice s3c2450_pwm_miscdev = {
	.minor		= PWM_MINOR,
	.name		= "pwm",
	.fops		= &s3c2450_pwm_fops,
};

static int  __init s3c2450_pwm_init(void)
{
	int ret;

	printk(KERN_INFO "S3c PWM Control Driver\n");
	
	ret = misc_register(&s3c2450_pwm_miscdev);
	if (ret < 0) {
		printk(KERN_ERR "s3c pwm control driver failed\n");
		return ret;
	}

	return ret;
}

static void __exit s3c2450_pwm_exit(void)
{
	misc_deregister(&s3c2450_pwm_miscdev);
}

module_init(s3c2450_pwm_init);
module_exit(s3c2450_pwm_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("pwm control driver for S3C");

