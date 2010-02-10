/*_____________________ Include Header _______________________________________*/
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>

#include <linux/mfd/wm8350/core.h>
#include <linux/mfd/wm8350/supply.h>
#include <linux/mfd/wm8350/comparator.h>

/*_____________________ Constants Definitions _______________________________*/
#define	ADC_MINOR	132	

#define PMIC_ADC_REG_READ		_IOWR('p', 0xb0, int)
#define PMIC_ADC_REG_WRITE		_IOWR('p', 0xb1, int)
#define PMIC_ADC_BAT_STATUS		_IOWR('p', 0xb2, int)

#define CHG_MODE_STATUS		0x3000

typedef enum _t_wm8350_bat_status {
	BAT_UVOLTAGE = 0,
	BAT_HOT_STATUS,
	BAT_COLD_STATUS,
	BAT_FAIL_STATUS,
	BAT_TO_STATUS,
	BAT_END_STATUS,
	BAT_START_STATUS,
	BAT_FAST_RDY_STATUS,
	BAT_3P9_STATUS,
	BAT_3P1_STATUS,
	BAT_2P85_STATUS,
	BAT_CHG_MODE_STATUS,
} wm8350_bat_status;

typedef struct {
	unsigned int reg_addr;
	unsigned int reg_value;
} _t_wm8350_adc_data;

typedef struct {
	unsigned int command;
	unsigned int status;
	unsigned int uvolt;	
} _t_wm8350_bat_data;
	
struct wm8350 *wm8350_adc = NULL;


/*_____________________ Program Body ________________________________________*/
static int wm8350_adc_open(struct inode *inode, struct file *filp)
{
	return	0; 
}

static int wm8350_adc_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static int wm8350_bat_fn_status(struct wm8350 *wm8350, unsigned long arg)
{
	_t_wm8350_bat_data bat_data = {0};

	if (copy_from_user((void *)&bat_data, 
				(const void *)arg,
				sizeof(bat_data))) 
		return -EFAULT;

	switch(bat_data.command) {
		case BAT_UVOLTAGE:
			bat_data.uvolt = wm8350_read_battery_uvolts(wm8350); 
			break;

		case BAT_HOT_STATUS:
			bat_data.status = (wm8350_get_bat_status() &
						WM8350_CHG_BAT_HOT_EINT) >> 15;
			break;

		case BAT_COLD_STATUS:
			bat_data.status = (wm8350_get_bat_status() &
						WM8350_CHG_BAT_COLD_EINT) >> 14;
			break;

		case BAT_FAIL_STATUS:
			bat_data.status = (wm8350_get_bat_status() &
						WM8350_CHG_BAT_FAIL_EINT) >> 13;
			break;

		case BAT_TO_STATUS:
			bat_data.status = (wm8350_get_bat_status() &
						WM8350_CHG_TO_EINT) >> 12;
			break;

		case BAT_END_STATUS:
			bat_data.status = (wm8350_get_bat_status() &
						WM8350_CHG_END_EINT) >> 11;
			break;

		case BAT_START_STATUS:
			bat_data.status = (wm8350_get_bat_status() &
						WM8350_CHG_START_EINT) >> 10;
			break;

		case BAT_FAST_RDY_STATUS:
			bat_data.status = (wm8350_get_bat_status() &
						WM8350_CHG_FAST_RDY_EINT) >> 9;
			break;

		case BAT_3P9_STATUS:
			bat_data.status = (wm8350_get_bat_status() &
						WM8350_CHG_VBATT_LT_3P9_EINT) >> 2;
			break;

		case BAT_3P1_STATUS:
			bat_data.status = (wm8350_get_bat_status() &
						WM8350_CHG_VBATT_LT_3P1_EINT) >> 1;
			break;

		case BAT_2P85_STATUS:
			bat_data.status = wm8350_get_bat_status() &
						WM8350_CHG_VBATT_LT_2P85_EINT;
			break;

		case BAT_CHG_MODE_STATUS:
			bat_data.status = (wm8350_reg_read(wm8350, 
						WM8350_BATTERY_CHARGER_CONTROL_2) &
						CHG_MODE_STATUS) >> 12;					
			break;

		default:
			printk("wm8350 adc : unsupported bat command 0x%x\n", bat_data.command);	
			return 0;	
	}

	if (copy_to_user((void *)arg, 
				&bat_data,sizeof(bat_data))) 
		return -EFAULT;

	return 0;
}

static int wm8350_adc_ioctl(struct inode *inode, struct file *file,
				unsigned int cmd, unsigned long arg)
{
	_t_wm8350_adc_data adc_data = {0};

	switch (cmd) {
		case PMIC_ADC_REG_READ:
			if (copy_from_user((void *)&adc_data,
						(const void *)arg,
						sizeof(adc_data))) 
				return -EFAULT;
			
			
			adc_data.reg_value = wm8350_reg_read(wm8350_adc, adc_data.reg_addr);

			if (copy_to_user((void *)arg, 
						&adc_data, sizeof(adc_data)))
				return -EFAULT;

			break;

		case PMIC_ADC_REG_WRITE:
			if (copy_from_user((void *)&adc_data,
						(const void *)arg,
						sizeof(adc_data))) 
				return -EFAULT;

			wm8350_reg_write(wm8350_adc, adc_data.reg_addr, adc_data.reg_value);
			break;

		case PMIC_ADC_BAT_STATUS:
			if (wm8350_bat_fn_status(wm8350_adc, arg) < 0)
				return -EFAULT;
			break;

		default:
			printk("wm8350 adc : unsupported ioctl command 0x%x\n", cmd);	
			break;
	}

	return 0;
}

static struct file_operations wm8350_adc_fops = {
	.owner		= THIS_MODULE,
	.ioctl		= wm8350_adc_ioctl,
	.open		= wm8350_adc_open,
	.release	= wm8350_adc_release,
};

static struct miscdevice wm8350_adc_miscdev = {
	.minor		= ADC_MINOR,
	.name		= "pmic_adc",
	.fops		= &wm8350_adc_fops,
};

static int __init wm8350_adc_probe(struct platform_device *pdev)
{
	int ret = 0;

	printk(KERN_INFO "pmic adc probe...\n");

	wm8350_adc = platform_get_drvdata(pdev);
	ret = misc_register(&wm8350_adc_miscdev);
	if (ret < 0) {
		printk(KERN_ERR "wm8350 adc driver failed\n");
		return ret;
	}

	return ret;
}

static int __devexit wm8350_adc_remove(struct platform_device *pdev)
{

	printk(KERN_INFO "pmic adc remove() of TS called !\n");
	misc_deregister(&wm8350_adc_miscdev);
	return 0;
}

struct platform_driver wm8350_adc_driver = {
	.probe	= wm8350_adc_probe,
	.remove = __devexit(wm8350_adc_remove),
	.driver	= {
		.name = "wm8350-adc",
	},
};

static int __init wm8350_adc_init(void)
{
	return platform_driver_register(&wm8350_adc_driver);
}

static void __exit wm8350_adc_exit(void)
{
	platform_driver_unregister(&wm8350_adc_driver);
}

module_init(wm8350_adc_init);
module_exit(wm8350_adc_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("adc driver for WM8350");
