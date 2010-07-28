/*-----------------------------------------------------------------------------
   * FILE NAME : canopus_vc0528.c
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

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <asm/hardware.h>
#include <asm/system.h>     
#include <asm/ioctl.h>
#include <asm/unistd.h>
#include <asm/plat-s3c24xx/s3c2416.h>
#include <asm/plat-s3c24xx/canopus_vc0528.h>
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

#include "../media/video/vc0528/VIM_HIGH_API.h"
#include "../media/video/vc0528/VIM_COMMON_Debug.h"
#include "../media/video/vc0528/bmp.h"

/*_____________________ Constants Definitions _______________________________*/
#define DRVNAME 			    "canopus-vc0528"
#define DEV_NAME 			    "video_ctr"
#define DEV_VERSION DEV_NAME    "v0.1"
#define canopus_bedev_suspend 	NULL
#define canopus_bedev_resume  	NULL
#define VIDEO_TEXT_ADDR 	    0xB8000
#define VIDEO_TEXT_SIZE 	    0x8000

/* VC0528 address */
#define _INDEX_ADDR 		0xF3600000  // index address 
#define _VALUE_ADDR  		0xF3600008  // value address

/* host control register */
#define MEM_8_FLG 			0x18b6   
#define REG_8_HIGH_WORD 	0x18b4 		// index address high 8bit
#define REG_8_LOW_WORD 		0x18b2 		// index address low  8bit
#define REG_8_FLG 			0x18b0
#define BYPASS_SEL 			0x1890
#define SEL_8_16 			0x188C
#define SEL_PORT 			0x1888
#define SEL_WRITE_READ 		0x1886
#define MUL_U2IA 			0x1880
#define MEM_FLG 			0x1850
#define MEM_HIGH_WORD 		0x1848
#define MEM_LOW_WORD 		0x1844
#define CMD_BLOCK_PARAMETERS_OFFSET 0x1802

/* address map tale of VC0528 */
//Reserved: 200000h~FFFFFFh
#define SDRAM_TABLE 		0x100000	// On-chip SRAM_Table 		  : 100000h~1FFFFFh
#define LCDC_UNIT_CTR  		0x002800	// LCDC unit control register : 002800h~002BFFh
#define MARB_UNIT_CTR  		0x002400	// MARB unit control register : 002400h~0027FFh
#define IPP_UNIT_CTR  		0x001C00	// IPP unit control register  : 001C00h~001FFFh
#define BIU_UNIT_CTR  		0x001800	// BIU unit control register  : 001800h~001BFFh
#define CPM_UNIT_CTR   		0x001400	// CPM unit control register  : 001400h~0017FFh
#define LBUF_UNIT_CTR  		0x000C00	// LBUF unit control register : 000C00h~000fFFh
#define SIF_UNIT_CTR   		0x000800	// SIF unit control register  : 000800h~000BFFh
#define LCDIF_UNIT_CTR  	0x000400	// LCDIF unit control register: 000400h~0007FFh
#define JPEG_UNIT_CTR   	0x000000	// JPEG unit control register : 000000h~0003FFh

#define V5_MULTI8_REG_PORT 	0xb0
#define V5_MULTI8_REG_WORDL 0xb2

#define V5_MULTI8_REG_WORDH 0xb4
#define V5_MULTI8_MEM_PORT 	0xb6

#define V5_MULTI8_MEM_FLG 	0x1850

#define VIM_MULTI8  		0
#define VIM_MULTI16 		1

#define dprintk(level,fmt, arg...)					\
	do {								\
		if (3 >= (level))				\
			printk(KERN_DEBUG "vc0528: " fmt , ## arg);	\
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

unsigned char jpeg_buf[8]={
	0x11,
	0x22,
	0x33,
	0x44,
	0x55,
	0x66,
	0x77,
	0x88
};


/*_____________________ Program Body ________________________________________*/
static void 
canopus_vc0528_reset(void)
{
	mdelay(100);
}


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
canopus_vc0528_hapi_init()
{
	unsigned short value16 = 0;
	value16 = VIM_HAPI_InitVc05x();
	dprintk(1,"InitVc05x: 0x%x\n",value16);
}

static void 
canopus_vc0528_init(void)
{
	dprintk(1,"[vc0528]: init \n");
	canopus_vc0528_normal_mode();
	_lcd_vc0528_trigger_lock = 0x0;
	mdelay(100);
	canopus_vc0528_reset();
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
canopus_v0528_camera_capture(void)
{
	dprintk(1,"[vc0528]: camera capture \n");
	
	mdelay(100);
    VIM_HAPI_SetWorkMode( VIM_HAPI_MODE_CAMERAON); 	//through mode power on lcd
    VIM_HAPI_SetLCDSize( VIM_HAPI_B0_LAYER,0,0,0,0);
    VIM_HAPI_SetLCDSize( VIM_HAPI_B1_LAYER,0,0,0,0);
    //VIM_HAPI_SetLCDWorkMode( VIM_HAPI_LCDMODE_AFIRST,0);
    VIM_HAPI_SetLCDWorkMode( VIM_HAPI_LCDMODE_BLONLY,0);
    VIM_HAPI_SetLCDColordep( VIM_HAPI_COLORDEP_16BIT);
    VIM_HAPI_SetCaptureParameter( 240,320);
    VIM_HAPI_SetPreviewParameter( 0,0,240,320);
	canopus_vc0528_write(0x803,0x2);
	VIM_HAPI_SetPreviewMode( VIM_HAPI_PREVIEW_FRAMEON);
}


unsigned int Jpeg_Length;
unsigned char JpegBuf[0x160000];

// jpeg encode: CPU( SDRAM) <- vc0528 <- Camera
static void
canopus_v0528_camera_captur_still(void)
{
	unsigned int cnt;
	int fd_image = 0;
	char image_file[100] = "./camera128x160.jpg";

	VIM_HAPI_SetWorkMode(VIM_HAPI_MODE_CAMERAON); 	//through mode power on lcd
//	VIM_HAPI_SetCaptureParameter(192,240);
//	VIM_HAPI_SetPreviewParameter(0,0,192,240);
//	VIM_HAPI_SetCaptureParameter(226,283);
//	VIM_HAPI_SetPreviewParameter(0,0,226,283);
//	VIM_HAPI_SetCaptureParameter(240,320);
//	VIM_HAPI_SetPreviewParameter(0,0,240,320);
	VIM_HAPI_SetCaptureParameter(640,480);
	VIM_HAPI_SetPreviewParameter(0,0,240,320);
	canopus_vc0528_write(0x803,0x2);
	VIM_HAPI_SetLCDWorkMode(VIM_HAPI_LCDMODE_OVERLAY,0);
	VIM_HAPI_SetPreviewMode(VIM_HAPI_PREVIEW_ON);
	mdelay(100);
	VIM_HAPI_CaptureStill(VIM_HAPI_RAM_SAVE,&JpegBuf,0x160000,0);
	Jpeg_Length=VIM_HAPI_GetCaptureLength();
	dprintk(1,"Jpeg_Length:0x%x\n",Jpeg_Length);

	for(cnt = 0; cnt <5 ;cnt++)
	{
		dprintk(1,"%d: 0x%x ,0x%x ,0x%x ,0x%x \n",cnt,JpegBuf[cnt],JpegBuf[cnt+1],JpegBuf[cnt+2],JpegBuf[cnt+3]);
	}

}


#if 0
// jpeg decode: CPU( SDRAM) -> vc0528 -> LCD
static void
canopus_vc0528_jpeg_view(void)
{
	VIM_HAPI_SetWorkMode( VIM_HAPI_MODE_CAMERAON);//through mode power on lcd
	VIM_HAPI_SetLCDSize( VIM_HAPI_B0_LAYER,0,0,0,0);
	VIM_HAPI_SetLCDSize( VIM_HAPI_B1_LAYER,0,0,0,0);
	VIM_HAPI_SetLCDWorkMode( VIM_HAPI_LCDMODE_OVERLAY,0);
	VIM_HAPI_SetLCDColordep( VIM_HAPI_COLORDEP_16BIT);
	VIM_HAPI_Display_Jpeg( VIM_HAPI_RAM_SAVE, test_1212 ,1536, 0, 0,64,96);
}

//jpeg preview: vc0528 <- Camera
canopus_vc0528_jpeg_camera_view(void)
{
	VIM_HAPI_SetWorkMode( VIM_HAPI_MODE_CAMERAON);//through mode power on lcd
	VIM_HAPI_SetCaptureParameter( 128,160);
	VIM_HAPI_SetPreviewParameter( 0,0,128,160);
	VIM_HAPI_SetLCDWorkMode( VIM_HAPI_LCDMODE_OVERLAY,0);
	VIM_HAPI_SetPreviewMode( VIM_HAPI_PREVIEW_ON);
}
#endif

#if 0
static void
canopus_v0528_camera_captur_still(void)
{
	dprintk(1,"[vc0528]: camera capture still\n");
	testcapture(1280,960);
	VIM_HAPI_SetLcdWorkMode(VIM_HAPI_LCDMODE_AFIRST,0);
	VIM_HAPI_SetCaptureParameter(1280,960);
	VIM_HAPI_SetPreviewParameter(0.0.240,320);
	VIM_HAPI_SetPreviewMode(VIM_HAPI_PREVIEW_ON,1);
	Delay(1000);
}
#endif

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

ctrl_jpeg_file
canopus_v0528_camera_jpeg_read(ctrl_jpeg_file args)
{
	args.size = 0x08;
	args.data_idx[2][0] = 0x55556666;
	args.data_idx[3][0] = 0x77778888;
	args.pdata_idx = &jpeg_buf;
	return args;
}


static int
canopus_bedev_ioctl(struct inode *inode, struct file *file,
				unsigned int cmd, unsigned long args)
{

	int enable;
	int err,size;
	int loop;
	unsigned char *buf,*ubuf;

	ctrl_32_info  	ctrl_info;
	ctrl_32_infos 	ctrl_infos;
	ctrl_jpeg_file 	ctrl_jpeg;

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
	case VC0528_CAMERA_JPEG_READ:
			dprintk(1,"VC0528_CAMERA_JPEG_READ\n");
#if 1			
			copy_from_user((void*)&ctrl_jpeg,args,size);
			ubuf = ctrl_jpeg.pdata_idx; 
			ctrl_jpeg = canopus_v0528_camera_jpeg_read(ctrl_jpeg);
			//buf = ctrl_jpeg.pdata_idx;
			//dprintk(1,"ker: 0x%x, 0x%x\n",*buf,*(buf+1));

			ctrl_jpeg.size = Jpeg_Length;
			for(loop=0;loop<(ctrl_jpeg.size);loop++)
			{
				put_user(JpegBuf[loop],&ubuf[loop]);				
			}
//			memset(JpegBuf,0,0x160000); 	// buffer clear !!
//			dprintk(1,"ker: 0x%x, 0x%x\n",*ctrl_jpeg.pdata_idx[0],*ctrl_jpeg.pdata_idx[1]);
			copy_to_user((void*)args,(const void*)
					&ctrl_jpeg,(unsigned long)size);
#endif 			
			break;
	case VC0528_CAMERA_JPEG_WRITE:
			dprintk(1,"VC0528_CAMERA_JPEG_WRITE\n");
#if 0
			copy_from_user((void*)&ctrl_info,args,size);
			ctrl_info = canopus_v0528_camera_captur_still(ctrl_info);
			copy_to_user((void*)args,(const void*)
					&ctrl_info,(unsigned long)size);
#endif 		
			for(loop=0;loop<((Jpeg_Length/8)-0xb);loop++)
			{
				dprintk(1,"0x%x, 0x%x, 0x%x, 0x%x ",JpegBuf[loop],JpegBuf[loop+1],JpegBuf[loop+2],JpegBuf[loop+3]);
				dprintk(1,"0x%x, 0x%x, 0x%x, 0x%x ",JpegBuf[loop+4],JpegBuf[loop+5],JpegBuf[loop+6],JpegBuf[loop+7]);
				dprintk(1,"0x%x, 0x%x, 0x%x, 0x%x \n",JpegBuf[loop+8],JpegBuf[loop+9],JpegBuf[loop+10],JpegBuf[loop+11]);
			}

			break;
	case VC0528_LCD_GUI_DROW_1:
			canopus_v0528_lcd_gui_mode_1();
			break;
	case VC0528_LCD_GUI_DROW_2:
			canopus_v0528_lcd_gui_mode_2();
			break;
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
	

static 
canopus_iomep_mmap(struct file *filp, struct vm_area_struct *vma)
{
	vma->vm_flags |= VM_RESERVED;
	vma->vm_flags |= VM_IO;

	if(ioremap_page_range(vma, vma->vm_start, VIDEO_TEXT_ADDR,
				VIDEO_TEXT_SIZE,vma->vm_page_prot)) 
//	if(ioremap_page_range(NULL, NULL, VIDEO_TEXT_ADDR,
//				VIDEO_TEXT_SIZE,NULL)) 
	return -EAGAIN;

	return 0;
}

static int
canopus_bedev_open(struct inode *inode, struct file *file)
{
	struct vc0528_dev *h, *dev = NULL;

	dprintk(1, "canopus_bedev_open\n");

	return 0;
}


/* about fasycn funtions */
static int
canopus_bedev_release(struct inode *inode, struct file *file)
{
	return 0;
}


static ssize_t 
canopus_bedev_read(struct file *file, char __user *buf , size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t 
canopus_bedev_write(struct file *file, const char __user *buf , size_t count, loff_t *ppos)
{
	char tbuf,vbuf[10];
	unsigned long level; 
	int cnt;

	for(cnt=0;cnt<count;cnt++ ) {
		get_user(tbuf,(char*)buf++);
		vbuf[cnt]=tbuf;
	}
	vbuf[cnt+1]=NULL;

	level = simple_strtoul(&vbuf,NULL,10);
//	set_value(level);
 	return 0;
}

static struct 
file_operations canopus_bedev_fos = {
	.owner		= THIS_MODULE,
	.ioctl		= canopus_bedev_ioctl,
	.open		= canopus_bedev_open,
	.release	= canopus_bedev_release,
	.read 		= canopus_bedev_read,
	.write 		= canopus_bedev_write,
	.mmap 		= canopus_iomep_mmap,
};

static int __init
canopus_bedev_probe(struct platform_device *pdev)
{
	struct class_device *bedev_device;

	_bedev_major = register_chrdev(0, DEV_NAME, &canopus_bedev_fos);

	_be_dev_class = class_create(THIS_MODULE, DEV_NAME);
	if (IS_ERR(_be_dev_class)) {
		dprintk(1,KERN_ERR "error creating flash dev class\n");
		return -1;
	}

	bedev_device = 
		class_device_create(_be_dev_class, 
				NULL, MKDEV(_bedev_major, 0), NULL, DEV_NAME);
	if (IS_ERR(bedev_device)) {
		dprintk(1,KERN_ERR "error creating flash class device\n");
		return -1;
	}

	return 0;
}

static int
canopus_bedev_remove(struct device *dev)
{
	unregister_chrdev(_bedev_major, DEV_NAME);
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

/*_____________________ VC0528 Debug code ___________________________________*/



/*_____________________ VC0528 Debug code ___________________________________*/
void
s3c24xx_smc_reg_write(unsigned long addr, unsigned long data)
{
	*(unsigned long*)addr = data;
	dprintk(1,"smd rd:0x%x: 0x%x\n",addr,data);
}


unsigned int 
s3c24xx_smc_reg_read(unsigned long addr)
{
	unsigned long rd_data;
		
	rd_data = (*(unsigned long*)addr);
	addr+=4;
	dprintk(1,"smc rd:0x%x: 0x%x\n",addr,(*(unsigned long*)addr));
	return rd_data;
}

unsigned int 
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

void
s3c24xx_smc_status(void)
{
	dprintk(1,"Bank Status Read [0x%x]:  0x%x\n",S3C_SSMC_SMBSR2,(*(unsigned long*)S3C_SSMC_SMBSR2)); 	   
	dprintk(1,"SMC Status Read  [0x%x]:  0x%x\n",S3C_SSMC_SSMCSR,(*(unsigned long*)S3C_SSMC_SSMCSR)); 	   
}

void
s3c24xx_smc_init(void)
{
 	(*(unsigned long*)S3C_SSMC_SMBIDCYR4)   = 0xf; 		/* Bank Idle Cycle Control Registor */
	(*(unsigned long*)S3C_SSMC_SMBWSTRDR4)  = 0x1F; 	/* Bank Read Wait State Contro Register */
	(*(unsigned long*)S3C_SSMC_SMBWSTWRR4)  = 0x1F; 	/* Bank Write Wait State Control Register */	
	(*(unsigned long*)S3C_SSMC_SMBWSTOENR4) = 0x2;
	(*(unsigned long*)S3C_SSMC_SMBWSTWENR4) = 0x2;	
	(*(unsigned long*)S3C_SSMC_SMBCR4)      = 0x303010;
	(*(unsigned long*)S3C_SSMC_SSMCCR)      = 0x3;
}

int 
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

void
s3c24xx_smc_data_write(ctrl_32_info args)
{
	dprintk(1,"data write:addr:0x%x, data:0x%x\n",args.addr,args.data);
//	(*(unsigned long*)args.addr) = args.data;
}

int 
canopus_vc0528_clk_on(void)
{
	dprintk(1,"canopus_vc0528_clk_on\n");
	return 0;
}

int 
canopus_vc0528_set_multi16(void)
{
	unsigned short value;

	dprintk(1,"canopus_vc0528_set_multi16\n");

#if 1	
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
#endif 

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

void
canopus_vc0528_reset_core(void)
{
	q_camera_backend_reset(1);
	msleep(100);
	q_camera_backend_reset(0);
	msleep(100);
}

int
canopus_vc0528_init_pll(void)
{
	(*(volatile unsigned char*)0xF3600006) = V5_MULTI8_REG_WORDH;    // 8bit high index write
	return 0;
}

int 
canopus_vc0528_pll_power_on(void)
{	
	return 0;
}

void
canopus_vc0528_8_wirte(ctrl_32_info args)
{
	/* 16bit add write */
	__raw_writew((unsigned short)args.addr, _index_addr);
	__raw_writew((unsigned short)args.data, _be_data_addr);
	udelay(10);
	dprintk(1,"write 8:addr:0x%x, data:0x%x\n",args.addr,args.data);
}

ctrl_32_info 
canopus_vc0528_8_read(ctrl_32_info args)
{
	__raw_writew((unsigned short)args.addr, _index_addr);
    args.data = __raw_readw(_be_data_addr);
	udelay(10);
	dprintk(1,"read 8:addr:0x%x, data:0x%x\n",args.addr,args.data);
	return args;
}

void 
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

void
canopus_vc0528_16_write_burst(ctrl_32_infos args)
{
	int cnt;
	dprintk(1,"canopus_vc0528_16_wiite_bust\n");
	
	for(cnt=0;cnt<args.size;cnt++){
		dprintk(1,"addr:0x%x, 0x%x\n",args.data_idx[cnt][0],args.data_idx[cnt][1]);
	}
}

void
canopus_vc0528_8_write_burst(ctrl_32_infos args)
{
	int cnt;
	dprintk(1,"canopus_vc0528_8_write_bust\n");

	for(cnt=0;cnt<args.size;cnt++){
		dprintk(1,"addr:0x%x, 0x%x\n",args.data_idx[cnt][0],args.data_idx[cnt][1]);
	}
	return 0;
}





/*_____________________ Linux Macro _________________________________________*/
module_init(canopus_bedev_init);
module_exit(canopus_bedev_exit);

MODULE_AUTHOR("yongsuk@udcsystems.com");
MODULE_DESCRIPTION( "flash driver");
MODULE_LICENSE( "GPL");


