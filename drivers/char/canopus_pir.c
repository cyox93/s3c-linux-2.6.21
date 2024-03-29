/*-----------------------------------------------------------------------------
 * FILE NAME : canopus_pir.c
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

/*_____________________ Constants Definitions _______________________________*/
#define DRVNAME				"canopus-pir"
#define DEV_NAME			"pir"
#define _DEFAULT_PIR_DECTECTION_TIMEOUT	350

#define PIR_IOCTL_S_ENABLE		_IOW ('Q', 0x80, int)
#define PIR_IOCTL_REISR_ENABLE	_IOW ('Q', 0x81, int)

/*_____________________ Type definitions ____________________________________*/

/*_____________________ Imported Variables __________________________________*/
extern void q_wm8350_notify_pir_event(void);
static void _pir_enable_work_cb(struct work_struct *work);
/*_____________________ Variables Definitions _______________________________*/

/*_____________________ Local Declarations __________________________________*/
static struct fasync_struct *_pir_queue;
static struct platform_device *_pdev;
static int _pir_major;
static struct class *_pir_dev_class;
static struct work_struct _pir_work;
static int _pir_is_enabled = false;
atomic_t _irq_is_enabled; 
atomic_t _reirq_is_enabled;

static struct timer_list _pir_timer;
static atomic_t _pir_pin_state;
static unsigned int _pir_detect_timeout = _DEFAULT_PIR_DECTECTION_TIMEOUT;

static DECLARE_DELAYED_WORK(_enable_work, _pir_enable_work_cb);

/*_____________________ Program Body ________________________________________*/
static int
_pir_timeout_show(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t len)
{
	printk("pir detect timeout %d ms\n", _pir_detect_timeout);

	return 1;
}

static int
_pir_timeout_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t len)
{
	_pir_detect_timeout = simple_strtoull(buf, NULL, 0);
	printk("pir detect timeout %d ms\n", _pir_detect_timeout);

	return len;
}

static DEVICE_ATTR(pir_timeout, 0644,
		_pir_timeout_show,
		_pir_timeout_store);

static int
_pir_create_file(struct platform_device *pdev)
{
	return device_create_file(&(pdev->dev), &dev_attr_pir_timeout);
}

static void _pir_enable_work_cb(struct work_struct *work)
{
	atomic_set(&_pir_pin_state, false);

	atomic_set(&_irq_is_enabled, true);
	__raw_writel(1UL << 2, S3C2410_SRCPND);
	__raw_writel(1UL << 2, S3C2410_INTPND);
	enable_irq(IRQ_EINT2);
}

static void
_pir_irq_work_cb(struct work_struct *work)
{
	if((_pir_is_enabled)&& atomic_read(&_reirq_is_enabled))
		q_wm8350_notify_pir_event();
}

static void
_pir_timer_handler(unsigned long data)
{
	if (atomic_read(&_pir_pin_state)) {
		del_timer(&_pir_timer);
		atomic_set(&_pir_pin_state, false);

		schedule_work(&_pir_work);
		printk(KERN_DEBUG"[%8lu] pir sensor detected [over %d]\n",
				jiffies_to_msecs(jiffies), _pir_detect_timeout);
	}
}

static irqreturn_t
_pir_irq_handler(int irq, void *data)
{
	unsigned int timestamp;

	if (s3c2410_gpio_getpin(S3C2410_GPF2) & 0x4) {
		atomic_set(&_pir_pin_state, true);

		del_timer(&_pir_timer);
		_pir_timer.expires = jiffies + msecs_to_jiffies(_pir_detect_timeout);
		add_timer(&_pir_timer);
	} else {
		if (atomic_read(&_pir_pin_state)) {
			del_timer(&_pir_timer);
			atomic_set(&_pir_pin_state, false);
		}
	}

	return IRQ_HANDLED;
}

static int
canopus_pir_ioctl(struct inode *inode, struct file *file,
				unsigned int cmd, unsigned long arg)
{
	int enable;

	switch(cmd){
	case PIR_IOCTL_S_ENABLE:
		if (get_user(enable, (int __user *)arg))
			return -EFAULT;

		enable = (enable) ? true : false;

		if (enable != _pir_is_enabled) {

			_pir_is_enabled = enable;
			cancel_delayed_work(&_enable_work);

			if (atomic_read(&_irq_is_enabled)){
				disable_irq(IRQ_EINT2);
				atomic_set(&_irq_is_enabled, false);
				atomic_set(&_reirq_is_enabled, false);
			}else{ 
				if(_pir_is_enabled)	
					schedule_delayed_work(&_enable_work, msecs_to_jiffies(30000));
				else
					atomic_set(&_reirq_is_enabled, false);					
			}

			if (!q_hw_ver(KTQOOK_MP)) {
				s3c2410_gpio_setpin(S3C2410_GPF5, (_pir_is_enabled) ? 1 : 0);
			} else {
				s3c2410_gpio_setpin(S3C2410_GPF5, (_pir_is_enabled) ? 0 : 1);
			}
		}
		break;

	case PIR_IOCTL_REISR_ENABLE:
		if (get_user(enable, (int __user *)arg))
			return -EFAULT;

		enable = (enable) ? true : false;

		if(enable)
			atomic_set(&_reirq_is_enabled, true);
		else
			atomic_set(&_reirq_is_enabled, false);
		return 0;
	default:
		printk(KERN_ERR "PIR DEVICE DRIVER: No pir Device driver Command defined\n");
		return -ENXIO;
	}

	return 0;
}

static int
canopus_pir_open(struct inode *inode, struct file *file)
{
	return 0;
}

/* about fasycn funtions */
static int
canopus_pir_release(struct inode *inode, struct file *file)
{
	return 0;
}

static int
canopus_pir_fasync(int fd, struct file *filp, int mode)
{
	return fasync_helper(fd, filp, mode, &_pir_queue);
}

static struct 
file_operations canopus_pir_fos = {
	.owner		= THIS_MODULE,
	.ioctl		= canopus_pir_ioctl,
	.open		= canopus_pir_open,
	.release	= canopus_pir_release,
	.fasync		= canopus_pir_fasync,
};

static int __init
canopus_pir_probe(struct platform_device *pdev)
{
	struct class_device *pir_device;

	_pir_major = register_chrdev(0, DEV_NAME, &canopus_pir_fos);
	if (_pir_major < 0) {
		printk(KERN_ERR "unable to get a major for canopus pir\n");
		return _pir_major;
	}

	_pir_dev_class = class_create(THIS_MODULE, DEV_NAME);
	if (IS_ERR(_pir_dev_class)) {
		printk( "error creating pir dev class\n");
		return -1;
	}

	pir_device = class_device_create(_pir_dev_class,
			NULL, MKDEV(_pir_major, 0), NULL, DEV_NAME);
	if (IS_ERR(pir_device)) {
		printk( "error creating pir class device\n");
		class_destroy(_pir_dev_class);
		return -1;
	}

	s3c2410_gpio_pullup(S3C2410_GPF2, 0);
	s3c2410_gpio_cfgpin(S3C2410_GPF2, S3C2410_GPF2_EINT2);

	// off
	s3c2410_gpio_cfgpin(S3C2410_GPF5, S3C2410_GPF5_OUTP);

	if (!q_hw_ver(KTQOOK_MP)) {
		s3c2410_gpio_setpin(S3C2410_GPF5, 0);
	} else {
		s3c2410_gpio_setpin(S3C2410_GPF5, 1);
	}

	init_timer(&_pir_timer);
	_pir_timer.function = _pir_timer_handler;

	INIT_WORK(&_pir_work, _pir_irq_work_cb);
	request_irq(IRQ_EINT2, _pir_irq_handler,
			SA_INTERRUPT|SA_TRIGGER_RISING|SA_TRIGGER_FALLING, "pir", NULL);

	disable_irq(IRQ_EINT2);

	_pir_create_file(pdev);

	return 0;
}

static int
canopus_pir_remove(struct device *dev)
{
	class_device_destroy(_pir_dev_class, MKDEV(_pir_major, 0));
	class_destroy(_pir_dev_class);

	unregister_chrdev(_pir_major, DEV_NAME);

	return 0;
}

#define canopus_pir_suspend NULL
#define canopus_pir_resume  NULL

static struct 
platform_driver canopus_pir_driver = {
	.probe          = canopus_pir_probe,
	.remove         = canopus_pir_remove,
	.suspend        = canopus_pir_suspend,
	.resume         = canopus_pir_resume,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= DRVNAME,
	},
};

int 
__init canopus_pir_init(void)
{
	int rc;

	if (!q_hw_ver(KTQOOK)) return 0;

	_pdev = platform_device_alloc(DRVNAME, 0);
	if (!_pdev)
		return -ENOMEM;

	rc = platform_device_add(_pdev);
	if (rc)
		goto undo_malloc;

	return platform_driver_register(&canopus_pir_driver);

undo_malloc:
	platform_device_put(_pdev);

	return -1;
}

void 
__exit canopus_pir_exit(void)
{
	if (!q_hw_ver(KTQOOK)) return ;

	platform_driver_unregister(&canopus_pir_driver);
	platform_device_unregister(_pdev);
}

/*_____________________ Linux Macro _________________________________________*/
module_init(canopus_pir_init);
module_exit(canopus_pir_exit);

MODULE_AUTHOR("yongsuk@udcsystems.com");
MODULE_DESCRIPTION( "PIR senser driver");
MODULE_LICENSE( "GPL");

