/*_____________________ Include Header _______________________________________*/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/circ_buf.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/platform_device.h>

#include <linux/mfd/wm8350/core.h>

/*_____________________ Constants Definitions _______________________________*/
#define DEV_NAME	"wm8350_cdev"
#define CIRC_BUF_MAX	16	
#define CIRC_ADD(elem,cir_buf,size)				\
	down(&event_mutex);					\
	if(CIRC_SPACE(cir_buf.head, cir_buf.tail, size)){	\
		cir_buf.buf[cir_buf.head] = (char)elem;		\
		cir_buf.head = (cir_buf.head + 1) & (size - 1);	\
	} else {						\
		pr_info("Failed to notify event to the user\n");\
	}							\
	up(&event_mutex);

#define CIRC_REMOVE(elem,cir_buf,size)				\
	down(&event_mutex);					\
	if(CIRC_CNT(cir_buf.head, cir_buf.tail, size)){         \
		elem = (int)cir_buf.buf[cir_buf.tail];          \
		cir_buf.tail = (cir_buf.tail + 1) & (size - 1); \
	} else {                                                \
		elem = -1;                                      \
		pr_info("No valid notified event\n");           \
	}							\
	up(&event_mutex);

static int wm8350_cdev_major;
static struct class *wm8350_cdev_class;
static struct fasync_struct *wm8350_cdev_queue;
static struct circ_buf wm8350_events;

static struct wm8350 *wm8350_cdev = NULL;

static DECLARE_MUTEX(event_mutex);

typedef struct {
	unsigned int reg_addr;
	unsigned int reg_value;
} register_info;

#define WM8350_CDEV_REG_READ		_IOWR('P', 0xa0, register_info*)
#define WM8350_CDEV_REG_WRITE		_IOWR('p', 0Xa1, register_info*)
#define WM8350_CDEV_NOTIFY_USER		_IOR('P', 0xa2, int)
#define WM8350_CDEV_GET_NOTIFY		_IOR('P', 0xa3, int)
#define WM8350_CDEV_SUBSCRIBE		_IOR('P', 0xa4, int)
#define WM8350_CDEV_UNSUBSCRIBE		_IOR('P', 0xa5, int)
#define WM8350_CDEV_REMOVE_USER		_IOR('P', 0xa6, int)

/*_____________________ Program Body ________________________________________*/
static void callbackfn(void *event)
{
	printk(KERN_INFO "\n\n DETECTED WM8350 EVENT : %d\n\n",
			(unsigned int)event);
}

static void user_notify_callback(void *event)
{
	CIRC_ADD((int)event, wm8350_events, CIRC_BUF_MAX);
	kill_fasync(&wm8350_cdev_queue, SIGIO, POLL_IN);
}

static int wm8350_cdev_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int wm8350_cdev_release(struct inode *inode, struct file *file)
{
	int i;
	wm8350_event_callback_t event_sub;

	for (i=0; i < EVENT_NB; i++) {
		event_sub.func = user_notify_callback;
		event_sub.param = (void *)i;
		wm8350_event_unsubscribe(i, event_sub);

		event_sub.func = callbackfn;
		wm8350_event_unsubscribe(i, event_sub);
	}
	
	wm8350_events.head = wm8350_events.tail = 0;

	return 0;
}

static int wm8350_cdev_ioctl(struct inode *inode, struct file *file,
				unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	register_info reg_info;
	wm8350_event_callback_t event_sub;
	type_event event;

	if (_IOC_TYPE(cmd) != 'P')
		return -ENOTTY;
	
	switch (cmd) {
		case WM8350_CDEV_REG_READ:
			if (copy_from_user(&reg_info, (register_info *) arg,
						sizeof(register_info))) {
				return -EFAULT;
			}
			
			reg_info.reg_value = wm8350_reg_read(wm8350_cdev, reg_info.reg_addr);

			if (copy_to_user((register_info *)arg, &reg_info,
						sizeof(register_info))) {
				return -EFAULT;
			}
			break;

		case WM8350_CDEV_REG_WRITE:
			if (copy_from_user(&reg_info, (register_info *) arg,
						sizeof(register_info))) {
				return -EFAULT;
			}
			
			wm8350_reg_write(wm8350_cdev, reg_info.reg_addr, reg_info.reg_value);

			if (copy_to_user((register_info *)arg, &reg_info,
						sizeof(register_info))) {
				return -EFAULT;
			}
			break;

		case WM8350_CDEV_SUBSCRIBE:
			if (get_user(event, (int __user *)arg)) {
				return -EFAULT;
			}
			event_sub.func = callbackfn;
			event_sub.param = (void *)event;
			ret = wm8350_event_subscribe(event, event_sub);
			pr_debug("subscribe done\n");
			break;

		case WM8350_CDEV_UNSUBSCRIBE:
			if (get_user(event, (int __user *)arg)) {
				return -EFAULT;
			}
			event_sub.func = callbackfn;
			event_sub.param = (void *)event;
			ret = wm8350_event_unsubscribe(event, event_sub);
			pr_debug("unsubscribe done\n");
			break;


		case WM8350_CDEV_NOTIFY_USER:
			if (get_user(event, (int __user *)arg)) {
				return -EFAULT;
			}
			event_sub.func = user_notify_callback;
			event_sub.param = (void *)event;
			ret = wm8350_event_subscribe(event, event_sub);
			break;

		case WM8350_CDEV_GET_NOTIFY:
			CIRC_REMOVE(event, wm8350_events, CIRC_BUF_MAX);
			if (put_user(event, (int __user *)arg)) {
				return -EFAULT;
			}
			break;

		case WM8350_CDEV_REMOVE_USER:
			if (get_user(event, (int __user *)arg)) {
				return -EFAULT;
			}
			event_sub.func = user_notify_callback;
			event_sub.param = (void *)event;
			ret = wm8350_event_unsubscribe(event, event_sub);
			break;

		default:
			printk(KERN_ERR "%d unsupported ioctl command\n", (int)cmd);
			return -EINVAL;
	}

	return ret;
}

static int wm8350_cdev_fasync(int fd, struct file *filp, int mode)
{
	return fasync_helper(fd, filp, mode, &wm8350_cdev_queue);
}

static struct file_operations wm8350_cdev_fos = {
	.owner		= THIS_MODULE,
	.ioctl		= wm8350_cdev_ioctl,
	.open		= wm8350_cdev_open,
	.release	= wm8350_cdev_release,
	.fasync		= wm8350_cdev_fasync,
};

static int __init wm8350_cdev_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct class_device *wm8350_cdevice;
	
	wm8350_cdev = platform_get_drvdata(pdev);

	wm8350_cdev_major = register_chrdev(0, DEV_NAME, &wm8350_cdev_fos);
	if (wm8350_cdev_major < 0) {
		printk(KERN_ERR "unable to get a major for wm8350_cdev\n");
		return -wm8350_cdev_major;
	}

	wm8350_cdev_class = class_create(THIS_MODULE, DEV_NAME);
	if (IS_ERR(wm8350_cdev_class)) {
		printk(KERN_ERR "Error creating wm8350 class\n");
		ret = -1;
		goto err;
	}
	
	wm8350_cdevice = 
		class_device_create(wm8350_cdev_class, NULL, MKDEV(wm8350_cdev_major, 0),
				NULL, DEV_NAME);
	if (IS_ERR(wm8350_cdevice)) {
		printk(KERN_ERR "Error creating wm8350 class device\n");
		ret = -1;
		goto err1;
	}

	wm8350_events.buf = kmalloc(CIRC_BUF_MAX * sizeof(char), GFP_KERNEL);
	if (NULL == wm8350_events.buf) {
		ret = -ENOMEM;
		goto err2;
	}
	wm8350_events.head = wm8350_events.tail = 0;

	printk(KERN_INFO "WM8350 Chatacter device: successfully loaded\n");

	return ret;

err2:
	class_device_destroy(wm8350_cdev_class, MKDEV(wm8350_cdev_major, 0));
err1:
	class_destroy(wm8350_cdev_class);
err:
	unregister_chrdev(wm8350_cdev_major, DEV_NAME);

	return ret;
	
}

static int __devexit wm8350_cdev_remove(struct platform_device *pdev)
{
	wm8350_cdev = NULL;

	class_device_destroy(wm8350_cdev_class, MKDEV(wm8350_cdev_major, 0));
	class_destroy(wm8350_cdev_class);

	unregister_chrdev(wm8350_cdev_major, DEV_NAME);

	printk(KERN_INFO "WM8350 Character device: successfully unloaded\n");

	return 0;
}

static struct platform_driver wm8350_cdev_driver = {
	.probe  = wm8350_cdev_probe,
	.remove = __devexit(wm8350_cdev_remove),
	.driver = {
		.name = "wm8350-cdev",
	},
};

static int __init wm8350_cdev_init(void)
{
	return platform_driver_register(&wm8350_cdev_driver);
}

static void  __exit wm8350_cdev_exit(void)
{
	platform_driver_unregister(&wm8350_cdev_driver);
}

module_init(wm8350_cdev_init);
module_exit(wm8350_cdev_exit);

MODULE_DESCRIPTION("WM8350_cdev protocol /dev entries driver");
MODULE_AUTHOR("Samsung");

MODULE_LICENSE("GPL");

