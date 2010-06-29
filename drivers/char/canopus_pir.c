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

#if 0
#include "dev_int.h"
#endif

#define DRVNAME 			"canopus-pir"

#define DEV_NAME 			 "pir"
#define DEV_VERSION DEV_NAME "v0.1"
#define CIRC_BUF_MAX		16	
#define PIR_ON 				0x0001
#define PIR_OFF 			0x0002
#define PIR_NOTIFY_USER 	0x0004


/* struct define */
typedef struct {
	void (*func) (void *);
	void *param;
} pir_event_callback_t;

typedef struct {
	// keeps a list of subsecibed clients to an event
	struct list_head list;

	// Callback function with parameter, called whend event occurs
	pir_event_callback_t callback;
} pir_event_callback_list_t;

static struct fasync_struct *_pir_queue;
static struct circ_buf _pir_event;
static struct platform_device *_pdev;
static int _pir_major;
static struct class *_pir_dev_class;
static struct work_struct _pir_work;
static int _is_enabled = false;

static DECLARE_MUTEX(event_mutex);

static void
_pir_irq_work(struct work_struct *work)
{
	kill_fasync(&_pir_queue, SIGIO, POLL_IN);
}

static irqreturn_t
_pir_irq_handler(int irq, void *data)
{
	schedule_work(&_pir_work);
	return IRQ_HANDLED;
}

static int
canopus_pir_ioctl(struct inode *inode, struct file *file,
				unsigned int cmd, unsigned long arg)
{
	switch(cmd){
		case PIR_ON:
		{
			if (!_is_enabled) {
				enable_irq(IRQ_EINT2);
				_is_enabled = true;
			}
			return 1;
		}
		case PIR_OFF:
		{
			if (_is_enabled) {
				disable_irq(IRQ_EINT2);
				_is_enabled = false;
			}
			return 1;
		}
		case PIR_NOTIFY_USER:
		{
			return 1;
		}
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

	_pir_dev_class = class_create(THIS_MODULE, DEV_NAME);
	if (IS_ERR(_pir_dev_class)) {
		printk( "error creating pir dev class\n");
		return -1;
	}

	pir_device = 
		class_device_create(_pir_dev_class, NULL, MKDEV(_pir_major, 0),
				NULL, DEV_NAME);
	if (IS_ERR(pir_device)) {
		printk( "error creating pir class device\n");
		return -1;
	}

	s3c2410_gpio_pullup(S3C2410_GPF2, 0);
	s3c2410_gpio_cfgpin(S3C2410_GPF2, S3C2410_GPF2_EINT2);

	INIT_WORK(&_pir_work, _pir_irq_work);
	request_irq(IRQ_EINT2, _pir_irq_handler,
			SA_INTERRUPT|SA_TRIGGER_RISING, "pir", NULL);

	disable_irq(IRQ_EINT2);

	return 0;
}

static int
canopus_pir_remove(struct device *dev)
{
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

module_init(canopus_pir_init);
module_exit(canopus_pir_exit);

MODULE_AUTHOR("yongsuk@udcsystems.com");
MODULE_DESCRIPTION( "DIR senser driver");
MODULE_LICENSE( "GPL");


