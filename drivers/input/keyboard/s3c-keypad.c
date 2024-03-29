/*
 * drivers/input/keyboard/s3c-keypad.c
 * KeyPad Interface on S3C 
 *
 * $Id: s3c-keypad.c,v 1.2 2008/02/29 01:57:09 wizardsj Exp $
 * 
 * This file is subject to the terms and conditions of the GNU General Public
 * License. See the file COPYING in the main directory of this archive
 * for more details.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/delay.h>
#include <asm/irq.h>

#include <asm/arch/regs-gpio.h>
#include <asm/arch/regs-irq.h>
#include "s3c-keypad.h"

#include <asm/uaccess.h>
#include <linux/proc_fs.h>


#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#endif

#ifdef S3C_KEYPAD_DEBUG
#undef S3C_KEYPAD_DEBUG
#endif

//#define S3C_KEYPAD_DEBUG 

#ifdef S3C_KEYPAD_DEBUG
#define DPRINTK(x...) printk("S3C-Keypad " x)
#else
#define DPRINTK(x...)		/* !!!! */
#endif

#define DEVICE_NAME "s3c-keypad"

#define TRUE 1
#define FALSE 0

static struct timer_list keypad_scan_timer;
static struct timer_list endkey_scan_timer;
#ifdef CONFIG_HAS_WAKELOCK
static struct wake_lock key_wake_lock;
#endif

static int curr_key_irq = 0;
static int key_irq_press = 0;
static int key_press = 0;
static int key_value = 0;
static int endkey_press = 0;


/* for the proc file system */
#define S3C_KEYPAD_PROC_DIR	"s3c-keypad"
#define S3C_KEYPAD_PROC_VALUE	"value"
#define S3C_KEYPAD_MAX_LENGTH	10

#define atoi(str) simple_strtoul(((str != NULL) ? str : ""), NULL, 0)

int key_code = 0;
int key_state = 0;

struct s3c_keypad *proc_s3c_keypad = NULL;
struct proc_dir_entry *proc_s3c_keypad_dir = NULL;
struct proc_dir_entry *proc_s3c_keypad_value = NULL;

static int s3c_keypad_proc_init(struct s3c_keypad *s3c_keypad);
static int s3c_keypad_proc_clear(int level);
static int s3c_keypad_proc_value_read(char *buf, char **start, off_t offset, int count,
			 int *eof, void *data);
static int s3c_keypad_proc_value_write(struct file *file, const char __user *buffer,
			  unsigned long count, void *data);


static void key_bh_handler(struct work_struct *work);

static void keypad_irq_mask(void)
{
	u32 mask;

	mask = __raw_readl(S3C2410_INTMSK);
	mask |= 0x21;

	__raw_writel(mask, S3C2410_INTMSK);
}

static void keypad_irq_unmask(void)
{
	u32 mask;

	mask = __raw_readl(S3C2410_INTMSK);
	mask &= ~(0x21);

	__raw_writel(mask, S3C2410_INTMSK);
}

static void keypad_irq_ack(void)
{
	__raw_writel(0x21, S3C2410_SRCPND);
	__raw_writel(0x21, S3C2410_INTPND);
}

static int keypad_scan_gpio_data(bool flag)
{
	u32 mskd, mskh;

	mskd = __raw_readl(S3C2410_GPDDAT) & ~(0x00007800);
	mskh = __raw_readl(S3C2410_GPHDAT) & ~(0x00000010);

	if (flag) {
		__raw_writel(mskd | 0x00007800, S3C2410_GPDDAT);
		__raw_writel(mskh | 0x00000010, S3C2410_GPHDAT);
	} else {
		__raw_writel(mskd, S3C2410_GPDDAT);
		__raw_writel(mskh, S3C2410_GPHDAT);
	}

	udelay(KEYPAD_DELAY);

	return 0;
}

static int keypad_scan_gpio_gph_data(bool flag)
{
	u32 mskh;

	mskh = __raw_readl(S3C2410_GPHDAT) & ~(0x00000010);

	if (flag) {
		__raw_writel(mskh | 0x00000010, S3C2410_GPHDAT);
	} else {
		__raw_writel(mskh, S3C2410_GPHDAT);
	}

	udelay(KEYPAD_DELAY);

	return 0;
}

static int keypad_scan_gpio_configure(int port)
{
	u32 con, udp, dat;

	if (port < 4) {
		/* GPDCON */
		con = __raw_readl(S3C2410_GPDCON) & ~(0x3fc00000);
		__raw_writel(con | (0x01 << (22 + (2 * port))), S3C2410_GPDCON);

		/* GPDUDP */
		udp = __raw_readl(S3C2410_GPDUP) & ~(0x3fc00000);
		__raw_writel(udp | (0x01 << (22 + (2 * port))), S3C2410_GPDUP);

		/* GPDDAT */
		dat = __raw_readl(S3C2410_GPDDAT) & ~(0x00007800);
		__raw_writel(dat | (~(0x01 << (11 + port)) & 0x00007800), S3C2410_GPDDAT);
	} else {
		/* GPHCON */
		con = __raw_readl(S3C2410_GPHCON) & ~(0x00000300);
		__raw_writel(con | (0x01 << 8), S3C2410_GPHCON);

		/* GPHUDP */
		udp = __raw_readl(S3C2410_GPHUP) & ~(0x00000300);
		__raw_writel(udp | (0x01 << 8), S3C2410_GPHUP);
		
		/* GPHDAT */
		dat = __raw_readl(S3C2410_GPHDAT) & ~(0x00000010);
		__raw_writel(dat | (~(0x01 << 4) & 0x00000010), S3C2410_GPHDAT);
	}
	udelay(KEYPAD_DELAY);

	return 0;
}

static int keypad_irq_configure(void)
{
	u32 cond, udpd, datd, conh, udph, dath, msk;

	msk = __raw_readl(S3C2410_GPGCON) & ~(0x00000fff);
	cond = __raw_readl(S3C2410_GPDCON) & ~(0x3fc00000);
	udpd = __raw_readl(S3C2410_GPDUP) & ~(0x3fc00000);
	datd = __raw_readl(S3C2410_GPDDAT) & ~(0x00007800);
	
	conh = __raw_readl(S3C2410_GPHCON) & ~(0x00000300);
	udph = __raw_readl(S3C2410_GPHUP) & ~(0x00000300);
	dath = __raw_readl(S3C2410_GPHDAT) & ~(0x00000010);


	__raw_writel(msk | 0x00000aaa, S3C2410_GPGCON);
	__raw_writel(cond | 0x15400000, S3C2410_GPDCON);
	__raw_writel(udpd | 0x15400000, S3C2410_GPDUP);
	__raw_writel(datd, S3C2410_GPDDAT);

	__raw_writel(conh | 0x00000100, S3C2410_GPHCON);
	__raw_writel(udph | 0x00000100, S3C2410_GPHUP);
	__raw_writel(dath, S3C2410_GPHDAT);

	return 0;
}

static int keypad_scan(struct s3c_keypad *pdata)
{
	int i,j;
	int col_cnt = 0, row_cnt = 0;
	int col_val = 0, row_val = 0, value;
	int ret = KEY_PRESS_STATE;
	u32 mask, rval, rval_sum = 0;
	struct input_dev *dev = pdata->dev;

	/* keypad scan input setting */
	mask = __raw_readl(S3C2410_GPGCON) & ~(0x00000fff);
	__raw_writel(mask, S3C2410_GPGCON); 

	keypad_scan_gpio_data(true);
	udelay(KEYPAD_DELAY);

	/* keypad scan routine */
	col_cnt = 0;
	for (i=0; i<KEYPAD_COLUMNS; i++) {
		keypad_scan_gpio_configure(i);

		rval = __raw_readl(S3C2410_GPGDAT);
		rval &= 0x3f;
		rval_sum += rval;


		DPRINTK("keypad_scan : i[%d], rval[%d], rval_sum[%d]\n", i, rval, rval_sum);
		if (col_cnt || row_val) {
			break;
		}

		if ((rval != 0x3f)) {
			row_cnt = 0;
			for (j=0; j<KEYPAD_ROWS; j++) {
				if (((~rval) >> j) & 0x01) {
					if (col_cnt || row_val) {
						return KEY_PRESS_STATE;
					}
					
					col_val = i;
					row_val = j;
					col_cnt++;
					row_cnt++;
				}
			}
		}
	}
	
	value = (col_val * 6) + row_val;

	DPRINTK("keypad_scan : col_cnt[%d], row_cnt[%d], col_val[%d], row_val[%d], key_press[%d], value[%d]\n", col_cnt, row_cnt, col_val, row_val, key_press, value);
	
	/* key press event */
	if (col_cnt && row_cnt && !key_press && 
			((rval_sum/KEYPAD_COLUMNS) != 0x3f)) {

		input_report_key(dev, pdata->keycodes[value], 1);
		DPRINTK("key pressed : %d\n", pdata->keycodes[value]);

		key_code = pdata->keycodes[value];
		key_state = 1;

		key_value = value;
		key_press = 1;
		
		return KEY_PRESS_STATE;
	}
	
	/* key release event */
	if (col_cnt && row_cnt && key_press && 
			((rval_sum/KEYPAD_COLUMNS) != 0x3f) && (key_value != value)) {
		input_report_key(dev, pdata->keycodes[key_value], 0);
		DPRINTK("key released : %d\n", pdata->keycodes[key_value]);

		key_code = pdata->keycodes[key_value];
		key_state = 0;

		key_press = 0;

		return KEY_PRESS_STATE;
	}

	/* key release event */
	if (!col_cnt && !row_cnt && ((rval_sum/KEYPAD_COLUMNS) == 0x3f)) {
		input_report_key(dev, pdata->keycodes[key_value], 0);
		DPRINTK("key released : %d\n", pdata->keycodes[key_value]);

		key_code = pdata->keycodes[key_value];
		key_state = 0;

		key_press = 0;
		
		keypad_scan_gpio_data(false);
		keypad_irq_configure();
		return KEY_RELEASE_STATE;
	}

	return ret;
}

static int endkey_scan(struct s3c_keypad *pdata)
{
	u32 endkey;
	int ret = KEY_PRESS_STATE;
	struct input_dev *dev = pdata->dev;

	endkey = __raw_readl(S3C2410_GPFDAT) & 0x01;
	DPRINTK("endkey scan: endkey, endkey_press [%d, %d]\n", endkey, endkey_press);
	
	if (!endkey) {
		if (!endkey_press)  {
			input_report_key(dev, pdata->keycodes[12], 1);
			input_sync(dev);
			
			key_code = pdata->keycodes[12];
			key_state = 1;

			endkey_press = 1;
		}
		DPRINTK("key Pressed : %d\n", pdata->keycodes[12]);

		return KEY_PRESS_STATE;
	} else {
		if (endkey_press) {
			input_report_key(dev, pdata->keycodes[12], 0);
			input_sync(dev);
			
			key_code = pdata->keycodes[12];
			key_state = 0;
			
			endkey_press = 0;
		}
		DPRINTK("key Released : %d\n", pdata->keycodes[12]);
		ret = KEY_RELEASE_STATE;
	}

	return ret;
}

static void keypad_scan_timer_handler(unsigned long data)
{	
	struct s3c_keypad *pdata = (struct s3c_keypad *)data;
	
	if (keypad_scan(pdata) == KEY_PRESS_STATE) {
		del_timer(&keypad_scan_timer);
		keypad_scan_timer.expires = jiffies + msecs_to_jiffies(50);
		add_timer(&keypad_scan_timer);
	}
	else {
		keypad_irq_ack();
		keypad_irq_unmask();
		key_irq_press = 0;
#ifdef CONFIG_HAS_WAKELOCK
		wake_lock_timeout(&key_wake_lock, 5*HZ);
#endif
	}
}

static void endkey_scan_timer_handler(unsigned long data)
{
	struct s3c_keypad *pdata = (struct s3c_keypad *)data;

	if (endkey_scan(pdata) == KEY_PRESS_STATE) {
		del_timer(&endkey_scan_timer);
		endkey_scan_timer.expires = jiffies + msecs_to_jiffies(50);
		add_timer(&endkey_scan_timer);
	}
	else {
		keypad_irq_ack();
		keypad_irq_unmask();
		key_irq_press = 0;
#ifdef CONFIG_HAS_WAKELOCK
		wake_lock_timeout(&key_wake_lock, 5*HZ);
#endif
	}
}

static void key_bh_handler(struct work_struct *work)
{
#ifdef CONFIG_HAS_WAKELOCK
	wake_lock(&key_wake_lock);
#endif
	if (curr_key_irq == 16) {
		endkey_scan_timer.expires = jiffies + msecs_to_jiffies(0);
		add_timer(&endkey_scan_timer);
	}
	else {
		keypad_scan_timer.expires = jiffies + msecs_to_jiffies(0);
		add_timer(&keypad_scan_timer);
	}
}

static irqreturn_t s3c_keypad_isr(int irq, void *data)
{
	struct s3c_keypad *s3c_keypad = (struct s3c_keypad *)data;

	if (key_irq_press) goto out;

	keypad_irq_mask();
	key_irq_press = 1;
	curr_key_irq = irq;

	schedule_work(&s3c_keypad->work);

out:
	return IRQ_HANDLED;
}

static int s3c_keypad_request_irq(struct s3c_keypad *keypad)
{
	int ret = 0;

	ret = request_irq(IRQ_EINT0, s3c_keypad_isr, SA_INTERRUPT|SA_TRIGGER_FALLING,
			DEVICE_NAME, (void *)keypad);
	if (ret)
		printk("request_irq failed (IRQ_EINT0)\n");

	ret = request_irq(IRQ_EINT8, s3c_keypad_isr, SA_INTERRUPT|SA_TRIGGER_FALLING,
			DEVICE_NAME, (void *)keypad);
	if (ret)
		printk("request_irq failed (IRQ_EINT8)\n");

	ret = request_irq(IRQ_EINT9, s3c_keypad_isr, SA_INTERRUPT|SA_TRIGGER_FALLING,
			DEVICE_NAME, (void *)keypad);
	if (ret)
		printk("request_irq failed (IRQ_EINT9)\n");

	ret = request_irq(IRQ_EINT10, s3c_keypad_isr, SA_INTERRUPT|SA_TRIGGER_FALLING,
			DEVICE_NAME, (void *)keypad);
	if (ret)
		printk("request_irq failed (IRQ_EINT10)\n");

	ret = request_irq(IRQ_EINT11, s3c_keypad_isr, SA_INTERRUPT|SA_TRIGGER_FALLING,
			DEVICE_NAME, (void *)keypad);
	if (ret)
		printk("request_irq failed (IRQ_EINT11)\n");

	ret = request_irq(IRQ_EINT12, s3c_keypad_isr, SA_INTERRUPT|SA_TRIGGER_FALLING,
			DEVICE_NAME, (void *)keypad);
	if (ret)
		printk("request_irq failed (IRQ_EINT12)\n");

	ret = request_irq(IRQ_EINT13, s3c_keypad_isr, SA_INTERRUPT|SA_TRIGGER_FALLING,
			DEVICE_NAME, (void *)keypad);
	if (ret)
		printk("request_irq failed (IRQ_EINT13)\n");

	return ret;
}

static int s3c_keypad_free_irq(struct s3c_keypad *keypad)
{
	free_irq(IRQ_EINT0, (void *)keypad);
	free_irq(IRQ_EINT8, (void *)keypad);
	free_irq(IRQ_EINT9, (void *)keypad);
	free_irq(IRQ_EINT10, (void *)keypad);
	free_irq(IRQ_EINT11, (void *)keypad);
	free_irq(IRQ_EINT12, (void *)keypad);
	free_irq(IRQ_EINT13, (void *)keypad);

	return 0;
}


static int s3c_keypad_proc_init(struct s3c_keypad *s3c_keypad) 
{
	if (!s3c_keypad) {
		return -ENOMEM;
	}

	proc_s3c_keypad = s3c_keypad;

	/* start make proc dir and files */
	proc_s3c_keypad_dir = proc_mkdir(S3C_KEYPAD_PROC_DIR, NULL);
	if (!proc_s3c_keypad_dir) {
		return -ENOMEM;
	}

	/* read only key value file in proc dir */
	proc_s3c_keypad_value = create_proc_entry(S3C_KEYPAD_PROC_VALUE, 0644, proc_s3c_keypad_dir);
	if (!proc_s3c_keypad_value) {
		s3c_keypad_proc_clear(1);
		return -ENOMEM;
	}

	proc_s3c_keypad_value->read_proc = s3c_keypad_proc_value_read;
	proc_s3c_keypad_value->write_proc = s3c_keypad_proc_value_write;
	proc_s3c_keypad_value->owner = THIS_MODULE;
	/* End make proc dir and files */

	return 0;
}

static int s3c_keypad_proc_clear(int level) 
{
	if (proc_s3c_keypad_dir == NULL) 
		return -1;

	switch (level) {
		case 0:
			remove_proc_entry(S3C_KEYPAD_PROC_VALUE, proc_s3c_keypad_dir);
		case 1:
			remove_proc_entry(S3C_KEYPAD_PROC_DIR, NULL);
		default:
			break;
	}
	return 0;
}

static int s3c_keypad_proc_value_read(char *buf, char **start, off_t offset, int count,
			 int *eof, void *data)
{
	int len;

	len = sprintf(buf, "%d:%d\n", key_code, key_state);
	return len;
}

static int s3c_keypad_proc_value_write(struct file *file, const char __user *buffer,
			  unsigned long count, void *data)
{
	struct input_dev *input_dev;
	char *buf = NULL, *buf_end;

	if (count > S3C_KEYPAD_MAX_LENGTH)
		count = S3C_KEYPAD_MAX_LENGTH;

	buf = kmalloc(sizeof(char) * (count + 1), GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	if (copy_from_user(buf, buffer, count)) {
		kfree(buf);
		return -EFAULT;
	}

	buf[count] = '\0';

	/* work around \n when echo'ing into proc */
	if (buf[count - 1] == '\n')
		buf[count - 1] = '\0';


	key_code = (int)simple_strtoul(buf, &buf_end, 0);
	key_state = (int)simple_strtoul(buf_end+1, NULL, 0);
	
	if (proc_s3c_keypad) {
		input_dev = proc_s3c_keypad->dev;
		input_report_key(input_dev, key_code, key_state);
	}

	kfree(buf);

	return count;
}


static int __init s3c_keypad_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct input_dev *input_dev;
	int key, code;
	struct s3c_keypad *s3c_keypad;

	s3c_keypad = kzalloc(sizeof(struct s3c_keypad), GFP_KERNEL);
	input_dev = input_allocate_device();

	if (!s3c_keypad || !input_dev) {
		kfree(s3c_keypad);
		input_free_device(input_dev);
		return -ENOMEM;
	}

	/* start make proc dir and files */
	if (s3c_keypad_proc_init(s3c_keypad) != 0) {
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, s3c_keypad);
	s3c_keypad->dev = input_dev;

	/* create and register the input driver */
	set_bit(EV_KEY, input_dev->evbit);
	set_bit(EV_REP, input_dev->evbit);
	s3c_keypad->nr_rows = KEYPAD_ROWS;
	s3c_keypad->no_cols = KEYPAD_COLUMNS;
	s3c_keypad->total_keys = MAX_KEYPAD_NR;

	for(key = 0; key < s3c_keypad->total_keys; key++){
		code = s3c_keypad->keycodes[key] = keypad_keycode[key];
		if(code<=0)
			continue;
		set_bit(code & KEY_MAX, input_dev->keybit);
	}

	input_dev->name = DEVICE_NAME;
	input_dev->phys = "s3c-keypad/input0";
	input_dev->cdev.dev = &pdev->dev;
	input_dev->private = s3c_keypad;
	
	input_dev->id.bustype = BUS_HOST;
	input_dev->id.vendor = 0x0001;
	input_dev->id.product = 0x0001;
	input_dev->id.version = 0x0001;

	input_dev->keycode = keypad_keycode;
	
	input_register_device(input_dev);

	/* Scan timer init */
	init_timer(&keypad_scan_timer);
	keypad_scan_timer.function = keypad_scan_timer_handler;
	keypad_scan_timer.data = (unsigned long)s3c_keypad;

	/* Scan endkey timer init */
	init_timer(&endkey_scan_timer);
	endkey_scan_timer.function = endkey_scan_timer_handler;
	endkey_scan_timer.data = (unsigned long)s3c_keypad;

#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_init(&key_wake_lock, WAKE_LOCK_SUSPEND, "s3c-keypad");
#endif
	
	keypad_irq_mask();
	key_irq_press = 1;
	curr_key_irq = 0;

	INIT_WORK(&s3c_keypad->work, key_bh_handler);
	ret = s3c_keypad_request_irq(s3c_keypad);
	if (ret)
		goto out;

	printk( DEVICE_NAME " Initialized\n");

	/* force scan after init */
	keypad_irq_mask();
	keypad_scan_timer.expires = jiffies + msecs_to_jiffies(50);
	add_timer(&keypad_scan_timer);

	return 0;

out:
	input_unregister_device(input_dev);
	kfree(s3c_keypad);
	s3c_keypad_free_irq(s3c_keypad);
	return ret;
}

static int s3c_keypad_remove(struct platform_device *pdev)
{
	struct s3c_keypad *s3c_keypad = platform_get_drvdata(pdev);

#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_destroy(&key_wake_lock);
#endif

	s3c_keypad_proc_clear(0);

	input_unregister_device(s3c_keypad->dev);
	kfree(pdev->dev.platform_data);
	s3c_keypad_free_irq(s3c_keypad);

	del_timer(&keypad_scan_timer);	
	printk(DEVICE_NAME " Removed.\n");
	return 0;
}

#ifdef CONFIG_PM
static int s3c_keypad_suspend(struct platform_device *dev, pm_message_t state)
{
	/* TODO */
	return 0;
}

static int s3c_keypad_resume(struct platform_device *dev)
{
	/* TODO */
	return 0;
}
#else
#define s3c_keypad_suspend NULL
#define s3c_keypad_resume  NULL
#endif /* CONFIG_PM */

static struct platform_driver s3c_keypad_driver = {
	.probe		= s3c_keypad_probe,
	.remove		= s3c_keypad_remove,
	.suspend	= s3c_keypad_suspend,
	.resume		= s3c_keypad_resume,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "s3c-keypad",
	},
};

static int __init s3c_keypad_init(void)
{
	int ret;

	ret = platform_driver_register(&s3c_keypad_driver);
	
	if(!ret)
	   printk(KERN_INFO "S3C Keypad Driver\n");

	return ret;
}

static void __exit s3c_keypad_exit(void)
{
	platform_driver_unregister(&s3c_keypad_driver);
}

module_init(s3c_keypad_init);
module_exit(s3c_keypad_exit);

MODULE_AUTHOR("Samsung");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("KeyPad interface for Samsung S3C");
