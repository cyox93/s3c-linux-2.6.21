/*-----------------------------------------------------------------------------
 * FILE NAME : drivers/rtc/rtc-wakers.c
 * 
 * PURPOSE : suspend wakers
 * 
 * Copyright 1999 - 2010 UniData Communication Systems, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * NOTES: N/A
 *---------------------------------------------------------------------------*/

/*_____________________ Include Header ______________________________________*/
#include <linux/ctype.h>
#include <linux/module.h>
#include <linux/rtc.h>
#include <linux/err.h>
#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#endif

/*_____________________ Constants Definitions _______________________________*/
struct waker {
	struct list_head    link;
	char                name[24];
	unsigned long       second;
	unsigned long       expired;
};

/*_____________________ Type definitions ____________________________________*/

/*_____________________ Imported Variables __________________________________*/

/*_____________________ Variables Definitions _______________________________*/

/*_____________________ Local Declarations __________________________________*/
static DEFINE_SPINLOCK(list_lock);
static LIST_HEAD(list_wakers);
static struct wake_lock wakers_wake_lock;

static ssize_t rtc_sysfs_show_wakers(struct class_device *dev, char *buf);
static ssize_t rtc_sysfs_store_wakers(struct class_device *dev, const char *buf, size_t n);

static const CLASS_DEVICE_ATTR(wakers, S_IRUGO | S_IWUSR,
		rtc_sysfs_show_wakers, rtc_sysfs_store_wakers);

static void _alarm_triggered_func(void *p);
static struct rtc_task alarm_rtc_task = {
	.func = _alarm_triggered_func
};

/*_____________________ internal functions __________________________________*/
static void
_alarm_triggered_func(void *p)
{
	struct rtc_device *rtc = (struct rtc_device *)p;
	if (!(rtc->irq_data & RTC_AF))
		return;
#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_timeout(&wakers_wake_lock, 5 * HZ);
#endif
}

static int
_print_waker(char *buf, struct waker *waker, unsigned long now)
{
	struct rtc_time tm;

	rtc_time_to_tm(waker->expired, &tm);

	return sprintf(buf, "%s\t%d\t%d/%d/%d %02d:%02d:%02d\t%d\n",
			waker->name,
			waker->second,
			tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec,
			(int)(waker->expired - now));
}

static int
_cmp_rtc_time(struct rtc_time *tm1, struct rtc_time *tm2)
{
	int *t1 = &tm1->tm_sec;
	int *t2 = &tm2->tm_sec;
	int i, t;

	for (i = 5; i >= 0; i--) {
		t = t1[i] - t2[i];
		if (t) return t;
	}

	return 0;
}

static int
_cmp_time(unsigned long time1, unsigned long time2)
{
	return (int)(time1 - time2);
}

static unsigned long
_get_now(struct class_device *dev)
{
	struct rtc_time tm;
	unsigned long now;

	rtc_read_time(dev, &tm);
	rtc_tm_to_time(&tm, &now);

	return now;
}

static struct waker *
_get_waker(struct class_device *dev, const char *name)
{
	struct waker *waker, *nwaker;

	list_for_each_entry_safe(waker, nwaker, &list_wakers, link) {
		if (!strcmp(waker->name, name))
			return waker;
	}

	return NULL;
}

static int
_get_alarm_time(struct class_device *dev)
{
	struct waker *waker, *nwaker;
	unsigned long now;
	int diff = -10, min = 10000;

	now = _get_now(dev);
	list_for_each_entry_safe(waker, nwaker, &list_wakers, link) {
		diff = _cmp_time(waker->expired, now);
		if (diff <= -5) {
			list_del(&waker->link);
			kfree(waker);
		} else {
			if (diff < min) min = diff;
		}
	}

	if (min < 2) min = now + 2;
	else min = now + min;

	return min;
}

static int
_parse_waker(const char *buf, char **name, long *second)
{
	char *p = buf, *n = NULL;

	int len = 0;
	int ret = -1;

	*name = NULL;
	*second = 0;

	while (*p && !isspace(*p) && (*p != '\n'))
		p++;

	len = p - buf;
	if (!len) return -1;

	n = kzalloc(len + 1, GFP_KERNEL);
	memcpy(n, buf, len);

	while (isspace(*p) || (*p == 'n')) p++;

	if (!*p) {
		ret = 0;
		goto _end_parse;
	}

	*second = simple_strtol(p, NULL, 0);
	ret = 0;

_end_parse:
	if (ret) {
		if (n) kfree(n);
	} else {
		*name = n;
	}

	return ret;
}

static void
_waker_set(struct class_device *dev, const char *name, long second)
{
	struct waker *waker;
	unsigned long now;

	if (!name) return ;

	waker = _get_waker(dev, name);
	if (second > 0) {
		if (!waker) {
			waker = kzalloc(sizeof(*waker), GFP_KERNEL);
			strncpy(waker->name, name, sizeof(waker->name) - 1);

			list_add(&waker->link, &list_wakers);
		}

		waker->second = second;
		waker->expired = _get_now(dev) + second;
	} else {
		if (waker) {
			list_del(&waker->link);
			kfree(waker);
		}
	}
}

static ssize_t rtc_sysfs_show_wakers(struct class_device *dev, char *buf)
{
	unsigned long irqflags, now;
	struct rtc_time tm;
	struct waker *waker;
	int len = 0;
	char *p = buf;

	spin_lock_irqsave(&list_lock, irqflags);

	rtc_read_time(dev, &tm);
	p += sprintf(p, "---------------- %d/%d/%d %02d:%02d:%02d ----------------\n",
			tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec);

	p += sprintf(p, "name\tsecond\talarm_time\t\tremain\n");

	rtc_tm_to_time(&tm, &now);
	list_for_each_entry(waker, &list_wakers, link) {
		p += _print_waker(p, waker, now);
	}

	spin_unlock_irqrestore(&list_lock, irqflags);

	len = p - buf;

	return len;
}

static ssize_t rtc_sysfs_store_wakers(struct class_device *dev, const char *buf, size_t n)
{
	unsigned long irqflags;
	char *name = NULL;
	long second = 0;
	int ret = -1;

	spin_lock_irqsave(&list_lock, irqflags);
	ret = _parse_waker(buf, &name, &second);
	if (ret < 0 || second < 0)
		goto bad_name;

	_waker_set(dev, name, second);

	if (name) kfree(name);

bad_name:
	spin_unlock_irqrestore(&list_lock, irqflags);
	return n;
}

/*_____________________ Program Body ________________________________________*/
int wakers_set_alarm(struct class_device *dev)
{
	struct rtc_wkalrm alarm;
	unsigned long irqflags = 0;
	unsigned long ret = 0;

	spin_lock_irqsave(&list_lock, irqflags);
	ret = _get_alarm_time(dev);
	spin_unlock_irqrestore(&list_lock, irqflags);

	if (ret) {
		rtc_time_to_tm(ret, &alarm.time);
		alarm.enabled = 0;
		alarm.pending = 0;
		alarm.time.tm_wday = -1;
		alarm.time.tm_yday = -1;
		alarm.time.tm_isdst = -1;

		rtc_set_alarm(dev, &alarm);
	}

	return ret;
}
EXPORT_SYMBOL(wakers_set_alarm);

int wakers_register(struct class_device *dev)
{
	int err;
	struct rtc_device *rtc = to_rtc_device(dev);

#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_init(&wakers_wake_lock, WAKE_LOCK_SUSPEND, "wakers");
#endif

	alarm_rtc_task.private_data = rtc;
	err = rtc_irq_register(dev, &alarm_rtc_task);
	if (err) {
		dev_err(dev->dev, "failed to register rtc irq\n");
#ifdef CONFIG_HAS_WAKELOCK
		wake_lock_destroy(&wakers_wake_lock);
#endif

		return err;
	}

	return class_device_create_file(dev, &class_device_attr_wakers);
}
EXPORT_SYMBOL(wakers_register);

void wakers_unregister(struct class_device *dev)
{
	rtc_irq_unregister(dev, &alarm_rtc_task);
	class_device_remove_file(dev, &class_device_attr_wakers);
#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_destroy(&wakers_wake_lock);
#endif
}
EXPORT_SYMBOL(wakers_unregister);

