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
 * Copyright (c) 2010 Yong-Sung Lee <yslee@udcsystems.com> 
 * Headphone Jack Detect for Unidata Canopus 
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

static void jack_irq_work(struct work_struct *work);

static struct platform_device *_pdev;

static const char jack_name[] = "canopus-jack";
static const char jack_mini_name[] = "canopus_jack";

static DECLARE_DELAYED_WORK(work, jack_irq_work);
static DECLARE_WAIT_QUEUE_HEAD(jack_wait);
static DEFINE_SPINLOCK(jack_lock);

static int last_state;
static int update;

static ssize_t jack_read(struct file *f, char __user *d, size_t s, loff_t *o)
{
	ssize_t ret;
	unsigned int data;

	if (s < sizeof(unsigned int))
		return -EINVAL;

	spin_lock(&jack_lock);
	last_state = data = s3c2410_gpio_getpin(S3C2410_GPF2) ? 1 : 0;
	update = 0;
	spin_unlock(&jack_lock);
	
	ret = put_user(data, (unsigned int __user *)d);
	if (ret == 0)
		ret = sizeof(unsigned int);
	return ret;
}

static unsigned int jack_poll(struct file *f, struct poll_table_struct *p)
{
	unsigned int data;
	
	poll_wait(f, &jack_wait, p);

	spin_lock(&jack_lock);
	data = s3c2410_gpio_getpin(S3C2410_GPF2) ? 1 : 0;
	if (last_state != data)
		update = 1;
	spin_unlock(&jack_lock);
	
	return (update) ? POLLIN | POLLWRNORM : 0;
}

static struct file_operations fops =
{
	.owner		= THIS_MODULE,
	.read		= jack_read,
	.poll		= jack_poll
};

static struct miscdevice miscdev =
{
	.minor		= MISC_DYNAMIC_MINOR,
	.name		= jack_mini_name,
	.fops		= &fops
};

static void jack_irq_work(struct work_struct *work)
{
	wake_up_interruptible(&jack_wait);
}

static irqreturn_t jack_irq_handler(int irq, void *data)
{
	int i;
	cancel_delayed_work(&work);
	i = schedule_delayed_work(&work, msecs_to_jiffies(50));
	return IRQ_HANDLED;
}

static int __init canopus_jack_probe(struct platform_device *pdev)
{
	int ret;

	ret = request_irq(IRQ_EINT2, jack_irq_handler,
			  SA_INTERRUPT|SA_TRIGGER_RISING|SA_TRIGGER_FALLING,
			  "canopus-jack", 0);
	
	ret = misc_register(&miscdev);
	return ret;
}


static int canopus_jack_remove(struct platform_device *pdev)
{
	misc_deregister(&miscdev);
	free_irq(IRQ_EINT2, 0);
	return 0;
}

static struct platform_driver canopus_jack_driver = {
       .probe          = canopus_jack_probe,
       .remove         = canopus_jack_remove,
       .driver		= {
		.owner	= THIS_MODULE,
		.name	= jack_name,
	},
};

static char banner[] __initdata = KERN_INFO "S3C Jack driver, (c) 2010 Unidata Communication Systems, Inc.\n";

int __init canopus_jack_init(void)
{
	int rc;

	if (q_hw_ver(KTQOOK)
			|| q_hw_ver(SKATM)) return -ENXIO;
	
	_pdev = platform_device_alloc(jack_name, 0);
	if (!_pdev)
		return -ENOMEM;

	rc = platform_device_add(_pdev);
	if (rc)
		goto undo_malloc;
	
	printk(banner);
	return platform_driver_register(&canopus_jack_driver);

undo_malloc:
	platform_device_put(_pdev);
	_pdev = 0;
	return -1;
}

void __exit canopus_jack_exit(void)
{
	if (q_hw_ver(KTQOOK)
			|| q_hw_ver(SKATM)) return;
	
	platform_driver_unregister(&canopus_jack_driver);
}

module_init(canopus_jack_init);
module_exit(canopus_jack_exit);

MODULE_AUTHOR("Yong-Sung Lee <yslee@udcsystems.com>");
MODULE_DESCRIPTION("canopus jack status driver");
MODULE_LICENSE("GPL");
