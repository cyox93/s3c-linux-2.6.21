
ifeq ($(CONFIG_PM_DEBUG),y)
EXTRA_CFLAGS	+=	-DDEBUG
endif

obj-y				:= main.o process.o console.o
obj-$(CONFIG_PM_LEGACY)		+= pm.o
obj-$(CONFIG_SOFTWARE_SUSPEND)	+= swsusp.o disk.o snapshot.o swap.o user.o
obj-$(CONFIG_WAKELOCK)		+= wakelock.o
obj-$(CONFIG_USER_WAKELOCK)	+= userwakelock.o
obj-$(CONFIG_EARLYSUSPEND)	+= earlysuspend.o

obj-$(CONFIG_MAGIC_SYSRQ)	+= poweroff.o
