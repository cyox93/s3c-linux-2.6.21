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
 * Copyright (c) 2011 Unidata Communication Systems, Inc.
 * SamrtCard Detect Driver for Unidata Canopus 
 *
 */

#include <linux/errno.h>
#include <linux/module.h>
#include <linux/wait.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/irq.h>
#include <linux/poll.h>

#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>
#include <asm/uaccess.h>
#include <asm/plat-s3c24xx/s3c2416.h>
#include <linux/delay.h>

#define	SC_IOCTL_S_POWER_ON		0
#define	SC_IOCTL_S_POWER_OFF		1
#define	SC_IOCTL_S_POWER_RESET		2
#define	SC_IOCTL_G_POWER_STATUS		3
#define	SC_IOCTL_G_CARD_STATUS		4
#define	SC_IOCTL_S_READY		5
#define	SC_IOCTL_S_POWER_TOGGLE		6

static void sc_irq_work(struct work_struct *work);

static struct fasync_struct *_sc_queue;
static struct platform_device *_pdev;

static const char sc_name[] = "canopus-sc";
static const char sc_mini_name[] = "canopus_sc";

static DECLARE_DELAYED_WORK(work, sc_irq_work);
static DECLARE_WAIT_QUEUE_HEAD(sc_wait);
static DEFINE_SPINLOCK(sc_lock);

static int last_state;
static int update;

static int is_ready;

static ssize_t sc_read(struct file *f, char __user *d, size_t s, loff_t *o)
{
	ssize_t ret;
	unsigned int data;

	if (s < sizeof(unsigned char))
		return -EINVAL;

	spin_lock(&sc_lock);
	if (!is_ready)
		last_state = data = 0;
	else
		last_state = data = s3c2410_gpio_getpin(S3C2410_GPF5) ? 1 : 0;
	update = 0;
	spin_unlock(&sc_lock);
	
	char result = (data) ? 'O' : 'X';
	ret = put_user(result, d);
	if (ret == 0)
		ret = sizeof(char);
	return ret;
}

static unsigned int sc_poll(struct file *f, struct poll_table_struct *p)
{
	unsigned int data;
	
	poll_wait(f, &sc_wait, p);

	spin_lock(&sc_lock);
	if (!is_ready)
		data = 0;
	else
		data = s3c2410_gpio_getpin(S3C2410_GPF5) ? 1 : 0;
	if (last_state != data)
		update = 1;
	spin_unlock(&sc_lock);
	
	return (update) ? POLLIN | POLLWRNORM : 0;
}

static int
sc_ioctl(struct inode *inode, struct file *file,
				unsigned int cmd, unsigned long arg)
{
	switch(cmd){
	case SC_IOCTL_S_POWER_ON:
		//printk("SmartCard Power ON\n");

		if (s3c2410_gpio_getpin(S3C2410_GPF2))
			break;

		s3c2410_gpio_setpin(S3C2410_GPF3, 0);
		mdelay(100);
		s3c2410_gpio_setpin(S3C2410_GPF3, 1);
		//mdelay(100);
		break;
	case SC_IOCTL_S_POWER_OFF:
		//printk("SmartCard Power OFF\n");

		if (!s3c2410_gpio_getpin(S3C2410_GPF2))
			break;

		s3c2410_gpio_setpin(S3C2410_GPF3, 0);
		mdelay(100);
		s3c2410_gpio_setpin(S3C2410_GPF3, 1);
		//mdelay(100);
		break;
	case SC_IOCTL_S_POWER_RESET:
		//printk("SmartCard Power RESET\n");

		s3c2410_gpio_setpin(S3C2410_GPG7, 1);
		mdelay(100);
		s3c2410_gpio_setpin(S3C2410_GPG7, 0);
		//mdelay(100);
		break;
	case SC_IOCTL_G_POWER_STATUS: {
		int value;

		value = s3c2410_gpio_getpin(S3C2410_GPF2) ? 1 : 0;

		//printk("SmartCard Power STATUS = [%s]\n", (value ? "On" : "Off"));

		if (copy_to_user
				((int *)arg, &value,
				 sizeof(int)))
			return -EFAULT;
	}
		break;
	case SC_IOCTL_G_CARD_STATUS: {
		int value;

		value = s3c2410_gpio_getpin(S3C2410_GPF5) ? 1 : 0;

		//printk("SmartCard Card STATUS = [%s]\n", (value ? "On" : "Off"));

		if (copy_to_user
				((int *)arg, &value,
				 sizeof(int)))
			return -EFAULT;
	}
		break;
	case SC_IOCTL_S_READY: {
		if (!is_ready) {
			is_ready = 1;
		}
	}
		break;
	case SC_IOCTL_S_POWER_TOGGLE: {
		//printk("SmartCard Power Toggle\n");

		s3c2410_gpio_setpin(S3C2410_GPF3, 0);
		mdelay(100);
		s3c2410_gpio_setpin(S3C2410_GPF3, 1);
	}
		break;
	default:
		//printk(KERN_ERR "PIR DEVICE DRIVER: No pir Device driver Command defined\n");
		return -ENXIO;
	}

	return 0;
}

static int
sc_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int
sc_release(struct inode *inode, struct file *file)
{
	return 0;
}

static int
sc_fasync(int fd, struct file *filp, int mode)
{
	return fasync_helper(fd, filp, mode, &_sc_queue);
}

static struct file_operations fops =
{
	.owner		= THIS_MODULE,
	.read		= sc_read,
	.poll		= sc_poll,
	.ioctl		= sc_ioctl,
	.open		= sc_open,
	.release	= sc_release,
	.fasync		= sc_fasync,
};
static struct miscdevice miscdev =
{
	.minor		= MISC_DYNAMIC_MINOR,
	.name		= sc_mini_name,
	.fops		= &fops
};

static void sc_irq_work(struct work_struct *work)
{
	//printk("SmartCard Event = %d\n", s3c2410_gpio_getpin(S3C2410_GPF5) ? 1 : 0);
	wake_up_interruptible(&sc_wait);
}

static irqreturn_t sc_irq_handler(int irq, void *data)
{
	int i;
	cancel_delayed_work(&work);
	i = schedule_delayed_work(&work, msecs_to_jiffies(100));
	return IRQ_HANDLED;
}

static int __init canopus_sc_probe(struct platform_device *pdev)
{
	int ret;

	/* UART */
	s3c2410_gpio_cfgpin(S3C2410_GPH2, S3C2410_GPH2_TXD0);
	s3c2410_gpio_cfgpin(S3C2410_GPH3, S3C2410_GPH3_RXD0);

	// set clockout0 to 12MHz
	s3c2410_gpio_cfgpin(S3C2443_GPH13, S3C2443_GPH13_CLKOUT0);
	struct clk *clkout0;
	clkout0 = clk_get(NULL, "clkout0");
	clk_enable(clkout0);

	s3c2410_gpio_setpin(S3C2410_GPF2, 0);
	s3c2410_gpio_cfgpin(S3C2410_GPF2, S3C2410_GPF2_INP);

	/* reset pin to low */
	s3c2410_gpio_setpin(S3C2410_GPG7, 0);
	s3c2410_gpio_cfgpin(S3C2410_GPG7, S3C2410_GPG7_OUTP);
	
	/* toggle power pin */
	/* FIXME: power on logic is not proper embedded device.
	 *	  Need to change circuit & fix driver also.
	 */
	s3c2410_gpio_pullup(S3C2410_GPF3, 0);
	s3c2410_gpio_cfgpin(S3C2410_GPF3, S3C2410_GPF3_OUTP);

	/* Card detect interrupt */
	s3c2410_gpio_pullup(S3C2410_GPF5, 0);
	s3c2410_gpio_cfgpin(S3C2410_GPF5, S3C2410_GPF5_EINT5);
	
	if (s3c2410_gpio_getpin(S3C2410_GPF2)) {
		mdelay(100);
		s3c2410_gpio_setpin(S3C2410_GPG7, 1);
		mdelay(100);
		s3c2410_gpio_setpin(S3C2410_GPG7, 0);
		mdelay(100);
	} else {
		mdelay(100);
		s3c2410_gpio_setpin(S3C2410_GPF3, 0);
		mdelay(100);
		s3c2410_gpio_setpin(S3C2410_GPF3, 1);
		mdelay(100);
	}

#if 0
	printk("------------ SmartCard Event insert = %d\n", s3c2410_gpio_getpin(S3C2410_GPF5) ? 1 : 0);
	printk("------------ SmartCard Event on/off = %d\n", s3c2410_gpio_getpin(S3C2410_GPF2) ? 1 : 0);
#endif
	
	ret = request_irq(IRQ_EINT5, sc_irq_handler,
			  SA_INTERRUPT|SA_TRIGGER_RISING|SA_TRIGGER_FALLING,
			  "canopus-sc", 0);
	
	ret = misc_register(&miscdev);
	return ret;
}


static int canopus_sc_remove(struct platform_device *pdev)
{
	s3c2410_gpio_cfgpin(S3C2410_GPF5, S3C2410_GPF5_INP);
		
	misc_deregister(&miscdev);
	free_irq(IRQ_EINT5, 0);
	return 0;
}

static struct platform_driver canopus_sc_driver = {
       .probe          = canopus_sc_probe,
       .remove         = canopus_sc_remove,
       .driver		= {
		.owner	= THIS_MODULE,
		.name	= sc_name,
	},
};

static char banner[] __initdata = KERN_INFO "S3C Smartcard Detect driver, (c) 2011 Unidata Communication Systems, Inc.\n";

int __init canopus_smartcard_init(void)
{
	int rc;

	/* only support on SK ATM phone */
	if (!q_hw_ver(SKATM)) 
		return -ENXIO;
	
	_pdev = platform_device_alloc(sc_name, 0);
	if (!_pdev)
		return -ENOMEM;

	rc = platform_device_add(_pdev);
	if (rc)
		goto undo_malloc;
	
	printk(banner);
	return platform_driver_register(&canopus_sc_driver);

undo_malloc:
	platform_device_put(_pdev);
	_pdev = 0;
	return -1;
}

void __exit canopus_smartcard_exit(void)
{
	if (!q_hw_ver(SKATM)) 
		return;
	
	platform_driver_unregister(&canopus_sc_driver);
}

module_init(canopus_smartcard_init);
module_exit(canopus_smartcard_exit);

MODULE_AUTHOR("Yong-Sung Lee <yslee@udcsystems.com>");
MODULE_DESCRIPTION("canopus Smart Card driver");
MODULE_LICENSE("GPL");
