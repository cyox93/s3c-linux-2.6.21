/*-----------------------------------------------------------------------------
 * FILE NAME : vc0528.c
 * 
 * PURPOSE : vc0525 test device dirver for Canopus K5
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
#include <linux/mm.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/wait.h>
#include <linux/ioport.h>
#include <linux/poll.h>     
#include <linux/proc_fs.h>
#include <linux/circ_buf.h>
#include <linux/platform_device.h>
#include <linux/sched.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <asm/hardware.h>
#include <asm/system.h>     
#include <asm/ioctl.h>
#include <asm/unistd.h>
#include <asm/plat-s3c24xx/s3c2416.h>
#include <asm/arch/regs-s3c2416-clock.h>
#include <asm/arch/regs-gpio.h>
#include <asm/arch/regs-irq.h>
#include <asm/arch/regs-timer.h>
#include <asm/arch/regs-mem.h>

/* for V4L2 interface */
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <linux/interrupt.h>
#include <linux/videodev2.h>
#include <linux/videodev.h>
#include <media/video-buf.h>
#include <media/v4l2-common.h>

#include "./vc0528/VIM_COMMON.h"
#include "./vc0528/bmp.h"
#include "vc0528.h"

/*_____________________ Constants Definitions _______________________________*/
#define DRVNAME 		"canopus-vc0528"
#define DEV_NAME 		"video_ctr"
#define DEV_VERSION DEV_NAME    "v0.1"

static int debug = 0;
#define dprintk(level,fmt, arg...)				\
	do {							\
		if (debug >= (level))				\
			printk("dev528: " fmt , ## arg);	\
	} while (0)


/*_____________________ Local Declarations __________________________________*/
static struct platform_device *_pdev;
static int _bedev_major;
static struct class *_be_dev_class;

static void __iomem *smc_base;
static void __iomem *_index_addr    = (unsigned short *)(0xF3600000); 
static void __iomem *_be_data_addr  = (unsigned short *)(0xF3600008);
static void __iomem *_lcd_data_addr = (unsigned short *)(0xF3600004);
int test_value;

extern int _lcd_vc0528_trigger_lock;
extern void _lcd_prepare_write(void);
extern void q_camera_backend_reset(int reset);

unsigned int Jpeg_Length;
unsigned int Jframe_Length[10];
unsigned char JpegBuf[0x160000];

/*_____________________ Program Body ________________________________________*/

static void
canopus_vc0528_bypass_mode(void)
{
	_lcd_vc0528_trigger_lock = 0x1;
	__raw_writew((unsigned short ) 0x1890, _index_addr);
	__raw_writew((unsigned short ) 0x1, _be_data_addr);
	udelay(10);
	_lcd_prepare_write();
}

static void
canopus_vc0528_normal_mode(void)
{
	_lcd_vc0528_trigger_lock = 0x0;
	mdelay(2);
	__raw_writew((unsigned short ) 0x1890, _index_addr);
	__raw_writew((unsigned short ) 0x0, _be_data_addr);
	udelay(10);
}

static void
canopus_vc0528_hapi_init(void)
{
	unsigned short value16 = 0;
	value16 = VIM_HAPI_InitVc05x();
	dprintk(1,"InitVc05x: 0x%x\n",value16);
}

static void 
canopus_vc0528_init(void)
{
	canopus_vc0528_normal_mode();
	_lcd_vc0528_trigger_lock = 0x0;
	mdelay(10);
	canopus_vc0528_hapi_init();
}

static void
canopus_vc0528_write(unsigned short addr, unsigned short data)
{
	__raw_writew((unsigned short )addr, _index_addr);
	__raw_writew((unsigned short )data, _be_data_addr);
//	udelay(10);
}

static unsigned short
canopus_vc0528_read(unsigned short addr)
{
	unsigned short data;
	__raw_writew((unsigned short )addr, _index_addr);
	data = __raw_readw(_be_data_addr);
	udelay(10);
	return data;
}

static int 
canopus_vc0528_set_multi16(void)
{
	unsigned short value;

	_index_addr    = (unsigned short *)(0xF3600000);
	_be_data_addr  = (unsigned short *)(0xF3600008);
	_lcd_data_addr = (unsigned short *)(0xF3600004);

	// set bus witdh to 16bit
	__raw_writeb(V5_MULTI8_REG_WORDH, _index_addr);
	__raw_writeb(0x18, _be_data_addr);
	__raw_writeb(V5_MULTI8_REG_WORDL, _index_addr);
	__raw_writeb(0x8c, _be_data_addr);
	__raw_writeb(V5_MULTI8_REG_PORT, _index_addr);
	__raw_writeb(0x1, _be_data_addr);

	// set delay
	__raw_writew(0x18f0, _index_addr);
	__raw_writew(0x0, _be_data_addr);

	__raw_writew(0x18f2, _index_addr);
	__raw_writew(0x0, _be_data_addr);

	__raw_writew(0x18f4, _index_addr);
	__raw_writew(0x0, _be_data_addr);

	__raw_writew(0x18f6, _index_addr);
	__raw_writew(0x0, _be_data_addr);

	__raw_writew(0x18f8, _index_addr);
	__raw_writew(0x0, _be_data_addr);

	__raw_writew(0x18fa, _index_addr);
	__raw_writew(0x0, _be_data_addr);
#if 0
	// set through mode
	__raw_writew(0x1890, _index_addr);
	__raw_writew(0x1, _be_data_addr);
#endif
	return 0;
}

static void
canopus_vc0528_reset_core(void)
{
	q_camera_backend_reset(1);
	msleep(10);
	q_camera_backend_reset(0);
	msleep(10);
	canopus_vc0528_set_multi16();	
}

unsigned int cnt_jpeg_tap;
//static void 
void 
canopus_v0528_camera_jpeg_read(void *args, int size)
{
	int fcnt;
	int frame_rate;

	unsigned char *buf,*ubuf;
	ctrl_jpeg_read *jpeg =(ctrl_jpeg_read*)args;
	
	dprintk(1,"____________ Fill buffer ____________\n");
	Jpeg_Length = 0;
	frame_rate  = jpeg->frame_rate; 
	buf = (void __force *)jpeg->frame_buf; 
	VIM_HAPI_BufferPosition(buf,jpeg->frbuf_size,jpeg->frame_rate,jpeg->frame_max);
//	mdelay(5); 

	/* start capture video */
	for(fcnt=0;fcnt<frame_rate;fcnt++){
		mdelay(40);     // minimum 30f/s JPEG 1frame make time  

		Jframe_Length[fcnt] = VIM_HAPI_Timer2();
		
		if(Jframe_Length[fcnt] < jpeg->frbuf_size){
			Jpeg_Length += Jframe_Length[fcnt];
			jpeg->frame_idx[fcnt] = Jframe_Length[fcnt];
		}else{
			dprintk(1,"#### Jpg_size:0x%08x > frb_size:0x%08x, frb_addr:0x%08x\n",Jframe_Length[fcnt],jpeg->frbuf_size,buf);
		}

		if(0xd9 != buf[Jpeg_Length-1])
		{
			dprintk(1,"____________ Jpeg_Length error ____________\n");
			dprintk(1,"Jpeg_Length_end-1:0x%02lx\n",buf[Jpeg_Length-1]);
			VIM_HAPI_StopCapture();
		}
	}

	jpeg->frame_size = Jpeg_Length;
}

static void
canopus_v0528_camera_capture_still(void)
{
	unsigned int cnt;
	unsigned int offset;
	unsigned int fcnt;

	int  h,m,s,us,jiffies;
	char timestr[13];
	long start_jiffies;

	VIM_HAPI_SetWorkMode(VIM_HAPI_MODE_CAMERAON); 	//through mode power on lcd
	VIM_HAPI_SetCaptureParameter(640,480);
	VIM_HAPI_SetPreviewParameter(0,0,240,320);
	canopus_vc0528_write(0x803,0x2);
	VIM_HAPI_SetPreviewMode(VIM_HAPI_PREVIEW_ON);
	VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER,DISABLE);
//	VIM_HAPI_SetCaptureQuality(VIM_HAPI_CPTURE_QUALITY_LIMITE);
	VIM_HAPI_SetCaptureQuality(VIM_HAPI_CPTURE_QUALITY_LOW);
//	VIM_HAPI_SetCaptureQuality(VIM_HAPI_CPTURE_QUALITY_MID);
//	VIM_HAPI_SetCaptureQuality(VIM_HAPI_CPTURE_QUALITY_HIGH);
	VIM_HAPI_SetCaptureVideoInfo(VIM_HAPI_RAM_SAVE,10,0xffffffff);
	VIM_HAPI_StartCaptureVideo(&JpegBuf,0x160000,NULL);
	mdelay(100);
}

static void
canopus_v0528_camera_preview(void)
{
	unsigned int cnt;
	unsigned int offset;
	unsigned int fcnt;

	int  h,m,s,us,jiffies;
	char timestr[13];
	long start_jiffies;

	VIM_HAPI_SetWorkMode(VIM_HAPI_MODE_CAMERAON); 	//through mode power on lcd
	VIM_HAPI_SetCaptureParameter(640,480);
	VIM_HAPI_SetPreviewParameter(0,0,240,320);
	canopus_vc0528_write(0x803,0x2);
	VIM_HAPI_SetPreviewMode(VIM_HAPI_PREVIEW_ON);
	VIM_HAPI_SetCaptureQuality(VIM_HAPI_CPTURE_QUALITY_LIMITE);
	VIM_HAPI_SetCaptureVideoInfo(VIM_HAPI_RAM_SAVE,10,0xffffffff);
	VIM_HAPI_StartCaptureVideo(NULL,0x160000,NULL);
	mdelay(100);
}

int
canopus_bedev_ioctl(unsigned int cmd, void *args)
{

	int enable;
	int err,size;
	int loop;
	int cnt;
	unsigned char *buf,*ubuf;

	ctrl_32_info  		ctrl_info;
	ctrl_32_infos 		ctrl_infos;
	ctrl_jpeg_file 		ctrl_jpeg;
	ctrl_sensor_test 	ctrl_test;

	if(_IOC_TYPE(cmd) != 'Q') return -EINVAL;

	size = _IOC_SIZE(cmd);

	switch(cmd){
	case VC0528_INIT:
		canopus_vc0528_init();
		break;
	case VC0528_BYPASS_MODE:
		canopus_vc0528_bypass_mode();
		break;
	case VC0528_RESET_CORE:
		canopus_vc0528_reset_core();
		break;
	case VC0528_CAMERA_CAPTURE_STILL:
		canopus_v0528_camera_capture_still();
		break;
	case VC0528_CAMERA_PREVIEW:
		canopus_v0528_camera_preview();
		break;
	case VC0528_CAMERA_JPEG_READ:
		canopus_v0528_camera_jpeg_read(args,size);
		break;
	default:
		dprintk(1,KERN_ERR "FLASH DEVICE DRIVER: No FLASH Device driver Command defined\n");
		return -ENXIO;
	}

	return 0;
}

/******************************* Device Driver *******************************/

static void
s3c24xx_smc_init(void)
{
	writel(0xf,  S3C_SSMC_SMBIDCYR4); /* Bank Idle Cycle Control Registor */
	writel(0x1f, S3C_SSMC_SMBWSTRDR4); /* Bank Read Wait State Contro Register */
	writel(0x1f, S3C_SSMC_SMBWSTWRR4); /* Bank Write Wait State Control Register */
	writel(0x2,  S3C_SSMC_SMBWSTOENR4);
	writel(0x2,  S3C_SSMC_SMBWSTWENR4);
	writel(0x303010, S3C_SSMC_SMBCR4);
	writel(0x3,  S3C_SSMC_SSMCCR);
}

int __init
canopus_bedev_probe(struct platform_device *pdev)
{
	printk("\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ canopus_bedev_probe\n");
	s3c24xx_smc_init();
	canopus_vc0528_set_multi16();
	canopus_vc0528_bypass_mode();	
	return 0;
}

static int
canopus_bedev_remove(struct device *dev)
{
	printk("\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ canopus_bedev_remove\n");
	return 0;
}

static int canopus_bedev_suspend(struct device *dev, pm_message_t state)
{
	printk("\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ canopus_bedev_suspend\n");
	return 0;
}

static int canopus_bedev_resume(struct device *dev)
{
	printk("\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ canopus_bedev_resume\n");
	return 0;
}

static struct 
platform_driver canopus_bedev_driver = {
	.probe          = canopus_bedev_probe,
	.remove         = canopus_bedev_remove,
	.suspend        = canopus_bedev_suspend,
	.resume         = canopus_bedev_resume,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= DRVNAME,
	},
};

int 
__init canopus_bedev_init(void)
{
	int rc;

	if (!q_hw_ver(KTQOOK)) return 0;

	_pdev = platform_device_alloc(DRVNAME, 0);
	if (!_pdev)
		return -ENOMEM;

	rc = platform_device_add(_pdev);
	if (rc)
		goto undo_malloc;

	return platform_driver_register(&canopus_bedev_driver);

undo_malloc:
	platform_device_put(_pdev);

	return -1;
}

void 
__exit canopus_bedev_exit(void)
{
	if (!q_hw_ver(KTQOOK)) return ;
	platform_driver_unregister(&canopus_bedev_driver);
	platform_device_unregister(_pdev);
}

/*_____________________ Linux Macro _________________________________________*/
module_init(canopus_bedev_init);
module_exit(canopus_bedev_exit);

MODULE_AUTHOR("yongsuk@udcsystems.com");
MODULE_DESCRIPTION( "Canopus Camera Backend Driver");
MODULE_LICENSE( "GPL");

