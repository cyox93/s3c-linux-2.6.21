/*-----------------------------------------------------------------------------
 * FILE NAME : vc0528-core.c
 * 
 * PURPOSE : 
 * 
 * Copyright 1999 - 2011 UniData Communication Systems, Inc.
 * All right reserved. 
 * 
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 *
 * NOTES:
 *
 *---------------------------------------------------------------------------*/

/*_____________________ Include Header ______________________________________*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/ioctl.h>
#include <linux/mutex.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/dma.h>
#include <asm/irq.h>

#include <asm/mach/map.h>
#include <asm/arch/dma.h>
#include <asm/plat-s3c24xx/s3c2416.h>
#include <asm/arch/regs-mem.h>

#include <media/vc0528.h>
#include "VC0528RDK/VIM_COMMON.h"

/*_____________________ Constants Definitions _______________________________*/
/* lcd handle for vc0528 camera backend ic */
#define _VC0528_PA_ADDRESS		(0x20000000)
#define _VC0528_PA_LCD_DATA		(_VC0528_PA_ADDRESS + 4)
#define _VC0528_PA_BE_DATA		(_VC0528_PA_ADDRESS + 8)
#define _VC0528_DMA_DCON		(S3C2410_DCON_SYNC_HCLK|S3C2416_DCON_WHOLE_SERV)

/*_____________________ Type definitions ____________________________________*/
#define _VC0528_MODE_CHECK(_m_)		\
{					\
	if (atomic_read(&g_vc0528->mode) != _m_) {	\
		printk(KERN_DEBUG "%s : vc0528 mismatch mode[%d] should be[%d]\n", \
			__func__, atomic_read(&g_vc0528->mode), VC0528_MODE_NORMAL); \
		mutex_unlock(&g_vc0528->lock);	\
		return -1; 		\
	}				\
}

/*_____________________ Imported Variables __________________________________*/
vc0528_data *g_vc0528;

/*_____________________ Variables Definitions _______________________________*/

/*_____________________ Local Declarations __________________________________*/
void __iomem *VIM_REG_INDEX;
void __iomem *VIM_REG_VALUE;

static void __iomem *_index_addr;
static void __iomem *_be_data_addr;
static void __iomem *_lcd_data_addr;

static struct s3c2410_dma_client _vc0528_dma_client = {
	.name		= "vc0528-dma",
};
static void *_vc0528_dma_done;

/*_____________________ Internal Functions __________________________________*/
static void
_vc0528_mode_bypass(int set)
{
	__raw_writew(0x1890, _index_addr);
	if (set)
		__raw_writew(0x1, _be_data_addr);
	else
		__raw_writew(0x0, _be_data_addr);
}

static void
_vc0528_be_write(u16 reg, u16 data)
{
	__raw_writew(reg, _index_addr);
	__raw_writew(data, _be_data_addr);
}

static u16
_vc0528_be_read(u16 reg)
{
	__raw_writew(reg, _index_addr);
	return __raw_readw(_be_data_addr);
}

static void
_vc0528_dma_finish(struct s3c2410_dma_chan *dma_ch, void *buf_id,
		int size, enum s3c2410_dma_buffresult result)
{
	complete(_vc0528_dma_done);
	_vc0528_dma_done = NULL;
}

static void
_vc0528_capture_cb(UINT8 status, UINT32 length)
{
	switch(status) {
	case VIM_HAPI_BUF_ERROR:
		if (g_vc0528->cap_func)
			g_vc0528->cap_func(g_vc0528, g_vc0528->cap_param, -1);

		VIM_HAPI_SetPreviewMode(VIM_HAPI_PREVIEW_ON);
		if (!g_vc0528->overlay_on)
			VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER,DISABLE);
		break;
	case VIM_HAPI_ONEFRAME_END:
		if (g_vc0528->cap_func)
			g_vc0528->cap_func(g_vc0528, g_vc0528->cap_param, length);
		break;
	case VIM_HAPI_CAPTURE_END:
		if (g_vc0528->cap_func)
			g_vc0528->cap_func(g_vc0528, g_vc0528->cap_param, 0);
		break;
	default:
		break;
	}
}

static void
_vc0528_timer_cb(struct work_struct *work)
{
	if (!g_vc0528->capture_on) return ;

	mutex_lock(&g_vc0528->lock);

	VIM_HAPI_Timer();

	if (g_vc0528->cap_interval)
		schedule_delayed_work(&g_vc0528->capture_work,
				msecs_to_jiffies(g_vc0528->cap_interval));

	mutex_unlock(&g_vc0528->lock);
}

/*_____________________ Program Body ________________________________________*/
void
vc0528_map_io(void)
{
	static atomic_t _init = ATOMIC_INIT(0);

	if (atomic_read(&_init)) return ;

	_index_addr    = ioremap(_VC0528_PA_ADDRESS, 0x100);
	_be_data_addr  = _index_addr + 0x08;
	_lcd_data_addr = _index_addr + 0x04;

	VIM_REG_INDEX = _index_addr;
	VIM_REG_VALUE = _be_data_addr;

	g_vc0528 = kmalloc(sizeof(vc0528_data), GFP_KERNEL);
	if (!g_vc0528) {
		printk(KERN_ERR "vc0528 data malloc failed\n");
		return ;
	}

	// smc init
 	__raw_writel(0xf, S3C_SSMC_SMBIDCYR4);
 	__raw_writel(0x1f, S3C_SSMC_SMBWSTRDR4);
 	__raw_writel(0x1f, S3C_SSMC_SMBWSTWRR4);
 	__raw_writel(0x2, S3C_SSMC_SMBWSTOENR4);
 	__raw_writel(0x2, S3C_SSMC_SMBWSTWENR4);
 	__raw_writel(0x303010, S3C_SSMC_SMBCR4);
 	__raw_writel(0x3, S3C_SSMC_SSMCCR);

	memset(g_vc0528, 0, sizeof(vc0528_data));
	mutex_init(&g_vc0528->lock);
	init_MUTEX(&g_vc0528->busy_lock);
	init_MUTEX(&g_vc0528->param_lock);

	INIT_DELAYED_WORK(&g_vc0528->capture_work, _vc0528_timer_cb);

	atomic_set(&g_vc0528->mode, VC0528_MODE_BYPASS);

	atomic_set(&_init, 1);
}

void
vc0528_lcd_reg(u16 reg)
{
	__raw_writew((unsigned short)reg, _index_addr);
}

u16
vc0528_lcd_read(u16 reg)
{
	__raw_writew(reg, _index_addr);
	return __raw_readw(_lcd_data_addr);
}

void
vc0528_lcd_write(u16 reg, u16 data)
{
	__raw_writew(reg, _index_addr);
	__raw_writew(data, _lcd_data_addr);
}

void
vc0528_lcd_data(u16 data)
{
	__raw_writew(data, _lcd_data_addr);
}

void
vc0528_dma_free(void)
{
	s3c2410_dma_free(DMACH_XD0, &_vc0528_dma_client);
}

void
vc0528_dma_set(void)
{
	if (s3c2410_dma_request(DMACH_XD0, &_vc0528_dma_client, NULL)) {
		printk(KERN_WARNING "Unable to get DMA channel.\n");
		return;
	}

	s3c2410_dma_set_buffdone_fn(DMACH_XD0, _vc0528_dma_finish);
	s3c2410_dma_devconfig(DMACH_XD0, S3C2410_DMASRC_MEM, 1, (u_long)_VC0528_PA_LCD_DATA);
	s3c2410_dma_config(DMACH_XD0, 2, _VC0528_DMA_DCON);
	s3c2410_dma_setflags(DMACH_XD0, S3C2410_DMAF_AUTOSTART);
}

void
vc0528_dma_request(int type, dma_addr_t addr, int size)
{
	int mode;

	if(mutex_trylock(&g_vc0528->lock)){
		DECLARE_COMPLETION_ONSTACK(complete);

		mode = vc0528_mode_get();
		if (mode == VC0528_MODE_NORMAL) {
			if (g_vc0528->overlay_on)
				VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER, DISABLE);
			_vc0528_mode_bypass(1);
		}

		_vc0528_dma_done = &complete;
		g_vc0528->dma_type = type;

		if (type == VC0528_DMA_LCD_WRITE) {
			vc0528_lcd_write(0x20, (u16)(0 & 0xff));
			vc0528_lcd_write(0x21, (u16)(0 & 0xff));

			vc0528_lcd_reg(0x22);

			s3c2410_dma_enqueue(DMACH_XD0, NULL, addr, size);
		}

		wait_for_completion(&complete);

		if (mode == VC0528_MODE_NORMAL) {
			if (g_vc0528->overlay_on)
				VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER, ENABLE);
			_vc0528_mode_bypass(0);
		}

		g_vc0528->dma_addr = addr;
		g_vc0528->dma_size = size;

		mutex_unlock(&g_vc0528->lock);
	}
}

void
vc0528_lock(void)
{
	mutex_lock(&g_vc0528->lock);
}

int
vc0528_trylock(void)
{
	return mutex_trylock(&g_vc0528->lock);
}

void
vc0528_unlock(void)
{
	mutex_unlock(&g_vc0528->lock);
}

int
vc0528_is_locked(void)
{
	return mutex_is_locked(&g_vc0528->lock);
}

int
vc0528_init(void)
{
	int ret;

	mutex_lock(&g_vc0528->lock);

	_vc0528_mode_bypass(false);
	atomic_set(&g_vc0528->mode, VC0528_MODE_NORMAL);
	ret = VIM_HAPI_InitVc05x();
	mutex_unlock(&g_vc0528->lock);

	if (ret) {
		printk(KERN_DEBUG"VC0528 init failed\n");
		return -1;
	}

	return ret;
}

void
vc0528_reset(void)
{
	q_camera_backend_reset(1);
	msleep(10);
	q_camera_backend_reset(0);
	msleep(5);

	VIM_HIF_SetMulti16();
	_vc0528_mode_bypass(true);
	atomic_set(&g_vc0528->mode, VC0528_MODE_BYPASS);
}

void
vc0528_end(void)
{
	mutex_lock(&g_vc0528->lock);

	VIM_HAPI_SetPreviewMode(VIM_HAPI_PREVIEW_OFF);
	VIM_HAPI_SetWorkMode(VIM_HAPI_MODE_BYPASS); 	//through mode power on lcd
	atomic_set(&g_vc0528->mode, VC0528_MODE_BYPASS);

	mutex_unlock(&g_vc0528->lock);

	if (g_vc0528->dma_addr)
		vc0528_dma_request(VC0528_DMA_LCD_WRITE,
				g_vc0528->dma_addr, g_vc0528->dma_size);
}

int
vc0528_camera_on(void)
{
	mutex_lock(&g_vc0528->lock);

	_VC0528_MODE_CHECK(VC0528_MODE_NORMAL);

	VIM_HAPI_SetWorkMode(VIM_HAPI_MODE_CAMERAON);
	VIM_HAPI_SetCaptureParameter(g_vc0528->v2f.fmt.pix.width,
			g_vc0528->v2f.fmt.pix.height);
	VIM_HAPI_SetPreviewParameter(g_vc0528->win.w.left, g_vc0528->win.w.top,
			g_vc0528->win.w.width, g_vc0528->win.w.height);
	VIM_HAPI_SetCaptureQuality(VIM_HAPI_CPTURE_QUALITY_LOW);

	// set camera hsyn polarity to high
	_vc0528_be_write(0x803,0x2);

	VIM_HAPI_SetPreviewMode(VIM_HAPI_PREVIEW_ON);
	VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER,DISABLE);

	mutex_unlock(&g_vc0528->lock);

	return 0;
}

int
vc0528_preview_start(void)
{
	int ret = 0;

	mutex_lock(&g_vc0528->lock);

	_VC0528_MODE_CHECK(VC0528_MODE_NORMAL);

	ret = VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER,ENABLE);
	mutex_unlock(&g_vc0528->lock);

	if (ret) {
		printk(KERN_DEBUG"vc0528 start preview failed[%d]\n", ret);
		return -1;
	}

	g_vc0528->overlay_pid = current->pid;

	return 0;
}

int
vc0528_preview_stop(void)
{
	int ret = 0;

	mutex_lock(&g_vc0528->lock);

	_VC0528_MODE_CHECK(VC0528_MODE_NORMAL);

	g_vc0528->overlay_pid = 0;
	ret = VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER,DISABLE);
	mutex_unlock(&g_vc0528->lock);

	if (g_vc0528->dma_addr)
		vc0528_dma_request(VC0528_DMA_LCD_WRITE,
				g_vc0528->dma_addr, g_vc0528->dma_size);

	if (ret) {
		printk(KERN_DEBUG"vc0528 stop preview failed[%d]\n", ret);
		return -1;
	}

	return 0;
}

int
vc0528_capture_still(void *buf, unsigned long size)
{
	int ret;

	mutex_lock(&g_vc0528->lock);

	VIM_HAPI_CaptureStill(VIM_HAPI_RAM_SAVE, buf, size, NULL);
	ret = VIM_HAPI_GetCaptureLength();

	VIM_HAPI_SetPreviewMode(VIM_HAPI_PREVIEW_ON);
	if (!g_vc0528->overlay_on)
		VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER,DISABLE);

	mutex_unlock(&g_vc0528->lock);

	return ret;
}

int
vc0528_capture_video(int framerate, void *buf, unsigned long size, vc0528_func_t func, void *param)
{
	int ret;

	mutex_lock(&g_vc0528->lock);

	g_vc0528->cap_func = func;
	g_vc0528->cap_param = param;

	ret = VIM_HAPI_SetCaptureVideoInfo(VIM_HAPI_RAM_SAVE, framerate, 0xfffffff);
	ret = VIM_HAPI_StartCaptureVideo(buf, size, _vc0528_capture_cb);

	mutex_unlock(&g_vc0528->lock);

	return (ret ? -1 : 0);
}

void
vc0528_capture_set_info(void *buf, unsigned long size)
{
	VIM_HAPI_SetBuf(buf,size);
}

int
vc0528_capture_stop(void)
{
	int ret;

	if (mutex_trylock(&g_vc0528->lock)) {
		ret =  VIM_HAPI_StopCapture();

		VIM_HAPI_SetPreviewMode(VIM_HAPI_PREVIEW_ON);
		if (!g_vc0528->overlay_on)
			VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER,DISABLE);

		mutex_unlock(&g_vc0528->lock);
	} else {
		VIM_HAPI_AbortCapture();
	}
}

int
vc0528_timer_start(unsigned int interval)
{
	g_vc0528->cap_interval = interval;
	schedule_delayed_work(&g_vc0528->capture_work, msecs_to_jiffies(30));

	return 0;
}

int
vc0528_timer_stop(void)
{
	g_vc0528->cap_interval = 0;
	cancel_delayed_work(&g_vc0528->capture_work);

	return 0;
}

int inline
vc0528_mode_get(void)
{
	return atomic_read(&g_vc0528->mode);
}


/*_____________________ Linux Macro _________________________________________*/
EXPORT_SYMBOL(vc0528_map_io);
EXPORT_SYMBOL(vc0528_lcd_reg);
EXPORT_SYMBOL(vc0528_lcd_read);
EXPORT_SYMBOL(vc0528_lcd_write);
EXPORT_SYMBOL(vc0528_lcd_data);
EXPORT_SYMBOL(vc0528_dma_set);
EXPORT_SYMBOL(vc0528_dma_free);
EXPORT_SYMBOL(vc0528_dma_request);
EXPORT_SYMBOL(vc0528_lock);
EXPORT_SYMBOL(vc0528_trylock);
EXPORT_SYMBOL(vc0528_unlock);
EXPORT_SYMBOL(vc0528_is_locked);

EXPORT_SYMBOL(vc0528_init);
EXPORT_SYMBOL(vc0528_reset);
EXPORT_SYMBOL(vc0528_end);

EXPORT_SYMBOL(vc0528_camera_on);
EXPORT_SYMBOL(vc0528_preview_start);
EXPORT_SYMBOL(vc0528_preview_stop);

EXPORT_SYMBOL(vc0528_capture_still);
EXPORT_SYMBOL(vc0528_capture_video);
EXPORT_SYMBOL(vc0528_capture_stop);

EXPORT_SYMBOL(vc0528_timer_start);
EXPORT_SYMBOL(vc0528_timer_stop);

EXPORT_SYMBOL(vc0528_mode_get);

EXPORT_SYMBOL(g_vc0528);
