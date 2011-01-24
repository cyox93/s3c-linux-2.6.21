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
#define DRVNAME 			    "canopus-vc0528"
#define DEV_NAME 			    "video_ctr"
#define DEV_VERSION DEV_NAME    "v0.1"
#define canopus_bedev_suspend 	NULL
#define canopus_bedev_resume  	NULL

static int debug = 0;
#define dprintk(level,fmt, arg...)					\
	do {								\
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

extern void _lcd_prepare_write(void);
extern void q_camera_backend_reset(int reset);

unsigned int Jpeg_Length;
unsigned int Jframe_Length[10];
unsigned char JpegBuf[0x160000];

/*_____________________ Program Body ________________________________________*/
static void
canopus_vc0528_bypass_mode(void)
{
	__raw_writew((unsigned short ) 0x1890, _index_addr);
	__raw_writew((unsigned short ) 0x1, _be_data_addr);
	udelay(10);
	_lcd_prepare_write();
}

static void
canopus_vc0528_normal_mode(void)
{
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
	dprintk(1,"init \n");
	canopus_vc0528_normal_mode();
	mdelay(10);
	canopus_vc0528_hapi_init();
}

static void
canopus_vc0528_write(unsigned short addr, unsigned short data)
{
	__raw_writew((unsigned short )addr, _index_addr);
	__raw_writew((unsigned short )data, _be_data_addr);
	udelay(10);
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

static void
canopus_vc0528_reset_core(void)
{
	q_camera_backend_reset(1);
	msleep(10);
	q_camera_backend_reset(0);
	msleep(10);
}

static int
canopus_vc0528_init_pll(void)
{
	return 0;
}

static int 
canopus_vc0528_pll_power_on(void)
{	
	return 0;
}

/* smc cntrol function */
static void
s3c24xx_smc_reg_write(unsigned long addr, unsigned long data)
{
	*(unsigned long*)addr = data;
	dprintk(1,"smd rd:0x%x: 0x%x\n",addr,data);
}

static unsigned int 
s3c24xx_smc_reg_read(unsigned long addr)
{
	unsigned long rd_data;
		
	rd_data = (*(unsigned long*)addr);
	addr+=4;
	dprintk(1,"smc rd:0x%x: 0x%x\n",addr,(*(unsigned long*)addr));
	return rd_data;
}

static unsigned int 
s3c24xx_smc_reg_read_bust(unsigned long addr)
{
	int cnt;
		
	for(cnt = 0; cnt <10 ; cnt++){
		dprintk(1,"smd rd[%d]:0x%x: 0x%x\n",cnt,addr,(*(unsigned long*)addr));
		addr+=4;
	}

	return 0;
}

int 
s3c24xx_smc_write_read(unsigned long addr,unsigned long data)
{
 	(*(unsigned long*)addr)   = data; 			
	dprintk(1,"Read  [0x%x]:  0x%x\n",addr,(*(unsigned long*)addr)); 	   
	return 0;
}

static void
s3c24xx_smc_init(void)
{
 	(*(unsigned long*)S3C_SSMC_SMBIDCYR4)   = 0xf; 		/* Bank Idle Cycle Control Registor */
	(*(unsigned long*)S3C_SSMC_SMBWSTRDR4)  = 0x1f; 	/* Bank Read Wait State Contro Register */
	(*(unsigned long*)S3C_SSMC_SMBWSTWRR4)  = 0x1f; 	/* Bank Write Wait State Control Register */	
	(*(unsigned long*)S3C_SSMC_SMBWSTOENR4) = 0x2;
	(*(unsigned long*)S3C_SSMC_SMBWSTWENR4) = 0x2;	
	(*(unsigned long*)S3C_SSMC_SMBCR4)      = 0x303010;
	(*(unsigned long*)S3C_SSMC_SSMCCR)      = 0x3;
}

static int 
s3c24xx_smc_test(void)
{
	int cnt;
	unsigned long smc_temp = (unsigned long*)smc_base;
	unsigned long val;
	
 	val = (*(unsigned long*)S3C_SSMC_SMBWSTRDR2);
	dprintk(1,"smc temp:0x%x, smc_base:0x%x\n",S3C_SSMC_SMBWSTRDR2,val);
	(*(unsigned long*)S3C_SSMC_SMBIDCYR2) = 0x1f; 
 	val = (*(unsigned long*)S3C_SSMC_SMBIDCYR2);
	dprintk(1,"smc temp:0x%x, smc_base:0x%x\n",S3C_SSMC_SMBIDCYR2,val);
}

static void
s3c24xx_smc_data_write(ctrl_32_info args)
{
	dprintk(1,"data write:addr:0x%x, data:0x%x\n",args.addr,args.data);
//	(*(unsigned long*)args.addr) = args.data;
}


/* VC0528 cntrol function */
static int 
canopus_vc0528_clk_on(void)
{
	dprintk(1,"canopus_vc0528_clk_on\n");
	return 0;
}

static void
canopus_vc0528_8_wirte(ctrl_32_info args)
{
	/* 16bit add write */
	__raw_writew((unsigned short)args.addr, _index_addr);
	__raw_writew((unsigned short)args.data, _be_data_addr);
	udelay(10);
	dprintk(1,"write 8:addr:0x%x, data:0x%x\n",args.addr,args.data);
}

static ctrl_32_info 
canopus_vc0528_8_read(ctrl_32_info args)
{
	__raw_writew((unsigned short)args.addr, _index_addr);
    args.data = __raw_readw(_be_data_addr);
	udelay(10);
	dprintk(1,"read 8:addr:0x%x, data:0x%x\n",args.addr,args.data);
	return args;
}

static void 
canopus_vc0528_16_write(ctrl_32_info args)
{
	/* 16bit add write */
	__raw_writew((unsigned short)args.addr, _index_addr);
	__raw_writew((unsigned short)args.data, _be_data_addr);
	udelay(10);
	dprintk(1,"write 16:addr:0x%x, data:0x%x\n",args.addr,args.data);
}

ctrl_32_info
canopus_vc0528_16_read(ctrl_32_info args)
{
	__raw_writew((unsigned short)args.addr, _index_addr);
    args.data = __raw_readw(_be_data_addr);
	udelay(10);
	dprintk(1,"read 16:addr:0x%x, data:0x%x\n",args.addr,args.data);
	return args;
}

static void
canopus_vc0528_16_write_burst(ctrl_32_infos args)
{
	int cnt;
	dprintk(1,"canopus_vc0528_16_wiite_bust\n");
	
	for(cnt=0;cnt<args.size;cnt++){
		dprintk(1,"addr:0x%x, 0x%x\n",args.data_idx[cnt][0],args.data_idx[cnt][1]);
	}
}

static void
canopus_vc0528_8_write_burst(ctrl_32_infos args)
{
	int cnt;
	dprintk(1,"canopus_vc0528_8_write_bust\n");

	for(cnt=0;cnt<args.size;cnt++){
		dprintk(1,"addr:0x%x, 0x%x\n",args.data_idx[cnt][0],args.data_idx[cnt][1]);
	}
	return 0;
}


unsigned int cnt_jpeg_tap;
spinlock_t 	slock_vc0528;
static void 
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
	//mutex_lock(&vc0528_lock);
	spin_lock(&slock_vc0528);
	VIM_HAPI_BufferPosition(buf,jpeg->frbuf_size,jpeg->frame_rate,jpeg->frame_max);

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
	spin_unlock(&slock_vc0528);
#if 0
	dprintk(1,"# frbuf_size:0x%x\n",jpeg->frbuf_size);
	dprintk(1,"# frame_rate:0x%x\n",jpeg->frame_rate);
	dprintk(1,"# frame_max:0x%x\n",jpeg->frame_max);
	dprintk(1,"# frame_buf:0x%x\n",jpeg->frame_buf);
	dprintk(1,"# pdata_idx:0x%x\n",jpeg->pdata_idx);
	dprintk(1,"# frame_size:0x%x\n",jpeg->frame_size);
#endif	
}


static int 
canopus_vc0528_set_multi16(void)
{
	unsigned short value;

	dprintk(1,"canopus_vc0528_set_multi16\n");

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
	__raw_writew( 0x18f0, _index_addr);
	__raw_writew( 0x0, _be_data_addr);

	__raw_writew( 0x18f2, _index_addr);
	__raw_writew( 0x0, _be_data_addr);

	__raw_writew( 0x18f4, _index_addr);
	__raw_writew( 0x0, _be_data_addr);

	__raw_writew( 0x18f6, _index_addr);
	__raw_writew( 0x0, _be_data_addr);

	__raw_writew( 0x18f8, _index_addr);
	__raw_writew( 0x0, _be_data_addr);

	__raw_writew( 0x18fa, _index_addr);
	__raw_writew( 0x0, _be_data_addr);
#if 0
	// set through mode
	__raw_writew( 0x1890, _index_addr);
	__raw_writew( 0x1, _be_data_addr);
#endif
	return 0;
}

static void
canopus_v0528_camera_capture(void)
{
	dprintk(1,"[vc0528]: camera capture \n");
	
	mdelay(100);
    VIM_HAPI_SetWorkMode(VIM_HAPI_MODE_CAMERAON); 			//through mode power on lcd
    VIM_HAPI_SetLCDSize(VIM_HAPI_B0_LAYER,0,0,0,0);
    VIM_HAPI_SetLCDSize(VIM_HAPI_B1_LAYER,0,0,0,0);
	/*VIM_HAPI_SetLCDWorkMode(VIM_HAPI_LCDMODE_AFIRST,0);*/
    VIM_HAPI_SetLCDWorkMode(VIM_HAPI_LCDMODE_BLONLY,0);
    VIM_HAPI_SetLCDColordep(VIM_HAPI_COLORDEP_16BIT);
    VIM_HAPI_SetCaptureParameter(240,320);
    VIM_HAPI_SetPreviewParameter(0,0,240,320);
	canopus_vc0528_write(0x803,0x2);
	VIM_HAPI_SetPreviewMode(VIM_HAPI_PREVIEW_FRAMEON);
}

static void
canopus_vc0528_lcd_mode_afirst(void)
{
    VIM_HAPI_SetLCDWorkMode(VIM_HAPI_LCDMODE_AFIRST,0);
}

static void
canopus_vc0528_lcd_mode_blonly_0(void)
{
    VIM_HAPI_SetLCDWorkMode(VIM_HAPI_LCDMODE_BLONLY,0);
}

static void
canopus_vc0528_lcd_mode_blonly_1(void)
{
    VIM_HAPI_SetLCDWorkMode(VIM_HAPI_LCDMODE_BLONLY,1);
}

static void
canopus_vc0528_lcd_mode_blend(void)
{
    VIM_HAPI_SetLCDWorkMode(VIM_HAPI_LCDMODE_BLEND,0);
}

static void
canopus_v0528_sub_module_reset(void)
{
    VIM_HIF_ResetSubModule(VIM_HIF_RESET_GLOBE);	                			
}

static void
canopus_v0528_camera_captur_still(void)
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
#if 1 
//	VIM_HAPI_SetCaptureParameter(192,240);
//	VIM_HAPI_SetPreviewParameter(0,0,192,240);
//	VIM_HAPI_SetCaptureParameter(226,283);
//	VIM_HAPI_SetPreviewParameter(0,0,226,283);
//	VIM_HAPI_SetCaptureParameter(240,320);
//	VIM_HAPI_SetPreviewParameter(0,0,240,320);
#endif 
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

static void
canopus_v0528_lcd_gui_mode_1(void)
{
	dprintk(1,"[vc0528]: lcd_gui_mode_1\n");
	
	VIM_HAPI_SetWorkMode(VIM_HAPI_MODE_CAMERAON); 		//through mode power on lcd
	VIM_HAPI_SetLCDSize(VIM_HAPI_B0_LAYER,0,0,0,0);
	VIM_HAPI_SetLCDSize(VIM_HAPI_B1_LAYER,0,0,0,0);
	VIM_HAPI_SetLCDWorkMode(VIM_HAPI_LCDMODE_BLONLY,0);
	VIM_HAPI_SetLCDColordep(VIM_HAPI_COLORDEP_16BIT);

	VIM_HAPI_SetLCDSize(VIM_HAPI_B0_LAYER,0,0,128,80);
	VIM_HAPI_SetLCDSize(VIM_HAPI_B1_LAYER,0,80,128,80);
	VIM_HAPI_SetLCDWorkMode(VIM_HAPI_LCDMODE_BLONLY,0);
	VIM_HAPI_SetLCDColordep(VIM_HAPI_COLORDEP_16BIT);
	VIM_HAPI_DrawLCDPureColor(VIM_HAPI_B0_LAYER,0,0,128,80,0xf800);
	VIM_HAPI_UpdateLCD(VIM_HAPI_B0_LAYER,0,0,128,80);
	mdelay(1000);
	VIM_HAPI_DrawLCDPureColor(VIM_HAPI_B1_LAYER,0,80,128,80,0x1f);
	VIM_HAPI_UpdateLCD(VIM_HAPI_B1_LAYER,0,80,128,80);
	mdelay(1000);
	VIM_HAPI_DrawLCDRctngl(VIM_HAPI_B0_LAYER,0,0,64,48,( UINT8 *)rgb64x48);
	VIM_HAPI_UpdateLCD(VIM_HAPI_B0_LAYER,0,0,64,48);
	mdelay(1000);
	VIM_HAPI_DrawLCDRctngl(VIM_HAPI_B1_LAYER,0,80,64,48,( UINT8 *)rgb64x48);
	VIM_HAPI_UpdateLCD(VIM_HAPI_B1_LAYER,0,80,64,48);
	mdelay(1000);
	VIM_HAPI_SetLCDMirror(VIM_HAPI_ROTATE_90,VIM_HAPI_ALLB_LAYER);
	VIM_HAPI_SetLCDSize(VIM_HAPI_B0_LAYER,10,10,150,54);
	VIM_HAPI_SetLCDSize(VIM_HAPI_B1_LAYER,10,64,150,64);
	VIM_HAPI_DrawLCDRctngl(VIM_HAPI_B0_LAYER,96,20,64,48,(UINT8 *)rgb64x48);
	VIM_HAPI_UpdateLCD(VIM_HAPI_B0_LAYER,96,20,64,48);
	mdelay(1000);
	VIM_HAPI_DrawLCDRctngl(VIM_HAPI_B1_LAYER,30,80,64,48,(UINT8 *)rgb64x48);
	VIM_HAPI_UpdateLCD(VIM_HAPI_B1_LAYER,30,80,64,48);
	mdelay(1000);
}

static void
canopus_v0528_lcd_gui_mode_2(void)
{
	dprintk(1,"[vc0528]: lcd_gui_mode_2\n");

	VIM_HAPI_SetWorkMode(VIM_HAPI_MODE_CAMERAON);//through mode power on lcd
	dprintk(1, "------------ layer 0------------\n");
	VIM_HAPI_SetLCDSize(VIM_HAPI_B0_LAYER,0,0,240,270);
	dprintk(1, "------------ layer 1------------\n");
	VIM_HAPI_SetLCDSize(VIM_HAPI_B1_LAYER,0,0,0,0);
	dprintk(1, "--------------------------------\n");
	VIM_HAPI_SetLCDWorkMode(VIM_HAPI_LCDMODE_BLONLY,0);
	VIM_HAPI_SetLCDColordep(VIM_HAPI_COLORDEP_16BIT);

	VIM_HAPI_DrawLCDPureColor(VIM_HAPI_B0_LAYER,0,0,240,270,0xf800);
	VIM_HAPI_UpdateLCD(VIM_HAPI_B0_LAYER,0,0,240,270);
	mdelay(1000);
	
	VIM_HAPI_DrawLCDPureColor(VIM_HAPI_B1_LAYER,0,0,240,270,0x1f);
	VIM_HAPI_UpdateLCD(VIM_HAPI_B0_LAYER,0,0,240,270);
	mdelay(1000);

#if 0
	VIM_HAPI_DrawLCDPureColor(VIM_HAPI_B1_LAYER,0,80,128,80,0x1f);
	VIM_HAPI_UpdateLCD(VIM_HAPI_B1_LAYER,0,80,128,80);
	mdelay(1000);
	VIM_HAPI_DrawLCDRctngl(VIM_HAPI_B0_LAYER,0,0,64,48,(UINT8*)rgb64x48);
	VIM_HAPI_UpdateLCD(VIM_HAPI_B0_LAYER,0,0,64,48);
	mdelay(1000);
	VIM_HAPI_DrawLCDRctngl(VIM_HAPI_B1_LAYER,0,80,64,48,(UINT8*)rgb64x48);
	VIM_HAPI_UpdateLCD(VIM_HAPI_B1_LAYER,0,80,64,48);
	mdelay(1000);
	VIM_HAPI_SetLCDMirror(VIM_HAPI_ROTATE_90,VIM_HAPI_ALLB_LAYER);
	VIM_HAPI_SetLCDSize(VIM_HAPI_B0_LAYER,10,10,150,54);
	VIM_HAPI_SetLCDSize(VIM_HAPI_B1_LAYER,10,64,150,64);
	VIM_HAPI_DrawLCDRctngl(VIM_HAPI_B0_LAYER,96,20,64,48,(UINT8*)rgb64x48);
	VIM_HAPI_UpdateLCD(VIM_HAPI_B0_LAYER,96,20,64,48);
	mdelay(1000);
	VIM_HAPI_DrawLCDRctngl(VIM_HAPI_B1_LAYER,30,80,64,48,(UINT8*)rgb64x48);
	VIM_HAPI_UpdateLCD(VIM_HAPI_B1_LAYER,30,80,64,48);
	mdelay(1000);
#endif 
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
//	if(_IOC_NR(cmd) >= V5_IO_MAXNR);

	size = _IOC_SIZE(cmd);

	switch(cmd){

	/* smc control command */
	case SSMC_INIT:
			s3c24xx_smc_init();
			break;
	case SSMC_WRITE:
			copy_from_user((void*)&ctrl_info,args,size);
			dprintk(1, "add:0x%x, data:0x%x\n",ctrl_info.addr,ctrl_info.data);
			s3c24xx_smc_data_write(ctrl_info);
			break;

	/* vc0528 control command */
	case VC0528_INIT:
			canopus_vc0528_init();
			break;
	case VC0528_BYPASS_MODE:
			canopus_vc0528_bypass_mode();
			break;
	case VC0528_NORMAL_MODE:
			canopus_vc0528_normal_mode();
			break;
	case VC0528_CLK_ON:
			canopus_vc0528_clk_on();
 			break;
	case VC0528_SET_MULTI16: 
			canopus_vc0528_set_multi16();
 			break;
	case VC0528_RESET_CORE:	
			canopus_vc0528_reset_core();
 			break;
	case VC0528_INIT_PLL: 
			canopus_vc0528_init_pll();
 			break;			
	case VC0528_PLL_POWER_ON:
			canopus_vc0528_pll_power_on();
 			break;
	case VC0528_CAMERA_CAPTURE_FRAME:
			canopus_v0528_camera_capture();
			break;
	case VC0528_CAMERA_CAPTURE_STILL:
			canopus_v0528_camera_captur_still();
			break;
	case VC0528_CAMERA_PREVIEW:
			canopus_v0528_camera_preview();
			break;
	case VC0528_CAMERA_JPEG_READ:
			canopus_v0528_camera_jpeg_read(args,size);
			break;
	case VC0528_CAMERA_JPEG_WRITE:
			dprintk(1,"VC0528_CAMERA_JPEG_WRITE\n");
			break;
	case VC0528_LCD_MODE_AFIRST:
			canopus_vc0528_lcd_mode_afirst();
			break;
	case VC0528_LCD_MODE_BLONLY_0:
			canopus_vc0528_lcd_mode_blonly_0();
			break;
	case VC0528_LCD_MODE_BLONLY_1:
			canopus_vc0528_lcd_mode_blonly_1();
			break;
	case VC0528_LCD_MODE_BLEND:
			canopus_vc0528_lcd_mode_blend();
			break;
	case VC0528_LCD_GUI_DROW_1:
			canopus_v0528_lcd_gui_mode_1();
			break;
	case VC0528_LCD_GUI_DROW_2:
			canopus_v0528_lcd_gui_mode_2();
			break;
	case VC0528_SUB_MODULE_RESET:
			canopus_v0528_sub_module_reset();
			break;

			/* hw test code */
#if 0			
	case VC0528_CAMERA_SENSOR_NEW_SET:
			copy_from_user((void*)&ctrl_test,(unsigned long*)args,size);
			SensorSetReg_size(ctrl_test.size);
			break;

	case VC0528_CAMERA_SENSOR_CHECK_NEW:
			SensorSetReg_check1(ctrl_test.size);
			break;

	case VC0528_CAMERA_SENSOR_CHECK_ORG:
			SensorSetReg_check2();
			break;
#endif
			/* 16/8bit read/write command */ 			
	case VC0528_8_WRITE:
			copy_from_user((void*)&ctrl_info,args,size);
			canopus_vc0528_8_wirte(ctrl_info);
			//dprintk(1, "add:0x%x, data:0x%x\n",ctrl_info.addr,ctrl_info.data);
 			break;
	case VC0528_8_READ:
			copy_from_user((void*)&ctrl_info,args,size);
			ctrl_info = canopus_vc0528_8_read(ctrl_info);
 			copy_to_user((void*)args,(const void*)
					      &ctrl_info,(unsigned long)size);
			//dprintk(1, "add:0x%x, data:0x%x\n",ctrl_info.addr,ctrl_info.data);
 			break;
	case VC0528_16_WRTIE:
			copy_from_user((void*)&ctrl_info,args,size);
			canopus_vc0528_16_write(ctrl_info);
			//dprintk(1, "add:0x%x, data:0x%x\n",ctrl_info.addr,ctrl_info.data);
 			break;
	case VC0528_16_READ:
			copy_from_user((void*)&ctrl_info,args,size);
			ctrl_info = canopus_vc0528_16_read(ctrl_info);
			copy_to_user((void*)args,(const void*)
					      &ctrl_info,(unsigned long)size);
			//dprintk(1, "add:0x%x, data:0x%x\n",ctrl_info.addr,ctrl_info.data);
 			break;	
	case VC0528_16_WRITE_BURST:
			copy_from_user((void*)&ctrl_infos,args,size);
			dprintk(1, "VC0528_16_WRITE_BURST\n");
			canopus_vc0528_16_write_burst(ctrl_infos);
 			break;
	case VC0528_8_WRITE_BURST:
			copy_from_user((void*)&ctrl_infos,args,size);
			dprintk(1, "VC0528_8_WRITE_BURST\n");
			canopus_vc0528_8_write_burst(ctrl_infos);
 			break;
	default:
			dprintk(1,KERN_ERR "FLASH DEVICE DRIVER: No FLASH Device driver Command defined\n");
			return -ENXIO;
	}

	return 0;
}

