#
# Makefile for regulator drivers.
#

obj-$(CONFIG_REGULATOR) += core.o
obj-$(CONFIG_REGULATOR_FIXED_VOLTAGE) += fixed.o

ccflags-$(CONFIG_REGULATOR_DEBUG) += -DDEBUG

obj-$(CONFIG_REGULATOR_WM8350) += wm8350/
