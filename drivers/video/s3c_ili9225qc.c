#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/tty.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/string.h>
#include <linux/ioctl.h>
#include <linux/clk.h>

#include <asm/io.h>
#include <asm/uaccess.h>

#include <asm/mach/map.h>
#include <asm/arch/regs-lcd.h>
#include <asm/arch/regs-gpio.h>

#if defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416)
#include <asm/arch/regs-s3c2450-clock.h>
#elif defined(CONFIG_CPU_S3C6400)
#include <asm/arch/regs-s3c6400-clock.h>
#elif defined(CONFIG_CPU_S3C6410)
#include <asm/arch/regs-s3c6410-clock.h>
#endif

#include "s3cfb.h"

#define ON 		1
#define OFF		0

#define DEFAULT_BACKLIGHT_LEVEL		2

#define H_FP		8		/* front porch */
#define H_SW		3		/* Hsync width */
#define H_BP		13		/* Back porch */

#define V_FP		5		/* front porch */
#define V_SW		1		/* Vsync width */
#define V_BP		7		/* Back porch */

extern struct s3c_fb_info info[S3C_FB_NUM];
s3c_win_info_t window_info;

//------------------ Virtual Screen -----------------------
#if defined(CONFIG_FB_VIRTUAL_SCREEN)
vs_info_t vs_info;

#define START_VIRTUAL_LCD 		11
#define STOP_VIRTUAL_LCD 		10
#define SET_VIRTUAL_LCD 		12

#define VS_MOVE_LEFT			15
#define VS_MOVE_RIGHT			16
#define VS_MOVE_UP			17
#define VS_MOVE_DOWN			18

#define MAX_DISPLAY_OFFSET		200
#define DEF_DISPLAY_OFFSET		100

int virtual_display_offset = DEF_DISPLAY_OFFSET;
#endif

//------------------ OSD (On Screen Display) -----------------------
#define START_OSD		1
#define STOP_OSD		0

// QCIF OSD image
#define H_RESOLUTION_OSD	176 /* horizon pixel  x resolition */
#define V_RESOLUTION_OSD	220 	/* line cnt       y resolution */


#define ALPHA_UP		3
#define ALPHA_DOWN		4
#define MOVE_LEFT		5
#define MOVE_RIGHT		6
#define MOVE_UP			7
#define MOVE_DOWN		8

#define MAX_ALPHA_LEVEL		0x0f

int osd_alpha_level = MAX_ALPHA_LEVEL;
int osd_left_top_x = 0;
int osd_left_top_y = 0;
int osd_right_bottom_x = H_RESOLUTION_OSD-1;
int osd_left_bottom_y = V_RESOLUTION_OSD -1;
//------------------------------------------------------------------------

#define H_RESOLUTION	176 	/* horizon pixel  x resolition */
#define V_RESOLUTION	220 	/* line cnt       y resolution */

#define H_RESOLUTION_VIRTUAL	176 	/* horizon pixel  x resolition */
#define V_RESOLUTION_VIRTUAL	440 	/* line cnt       y resolution */

#define VFRAME_FREQ     60	/* frame rate freq */
#define PIXEL_BPP8	8
#define PIXEL_BPP16	16	/*  RGB 5-6-5 format for SMDK EVAL BOARD */
#define PIXEL_BPP24	24	/*  RGB 8-8-8 format for SMDK EVAL BOARD */

#define LCD_PIXEL_CLOCK (VFRAME_FREQ *(H_FP+H_SW+H_BP+H_RESOLUTION) * (V_FP+V_SW+V_BP+V_RESOLUTION))
#define PIXEL_CLOCK	VFRAME_FREQ * LCD_PIXEL_CLOCK	/*  vclk = frame * pixel_count */

#define MAX_DISPLAY_BRIGHTNESS		9
#define DEF_DISPLAY_BRIGHTNESS		4

int display_brightness = DEF_DISPLAY_BRIGHTNESS;

void set_brightness(int);

struct s3c_fb_mach_info mach_info = {

#if defined(CONFIG_CPU_S3C2443) ||  defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416)
	.vidcon0= S3C_VIDCON0_VIDOUT_RGB_IF | S3C_VIDCON0_PNRMODE_RGB_P | S3C_VIDCON0_CLKDIR_DIVIDED | S3C_VIDCON0_VCLKEN_ENABLE |S3C_VIDCON0_CLKSEL_F_HCLK,
	.vidcon1= S3C_VIDCON1_IHSYNC_INVERT | S3C_VIDCON1_IVSYNC_INVERT,
	.vidtcon0= S3C_VIDTCON0_VBPD(V_BP-1) | S3C_VIDTCON0_VFPD(V_FP-1) | S3C_VIDTCON0_VSPW(V_SW-1),
	.vidtcon1= S3C_VIDTCON1_HBPD(H_BP-1) | S3C_VIDTCON1_HFPD(H_FP-1 ) | S3C_VIDTCON1_HSPW(H_SW-1),

#elif defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
	.vidcon0 = S3C_VIDCON0_INTERLACE_F_PROGRESSIVE | S3C_VIDCON0_VIDOUT_RGB_IF | S3C_VIDCON0_L1_DATA16_SUB_16_MODE
		| S3C_VIDCON0_L0_DATA16_MAIN_16_MODE | S3C_VIDCON0_PNRMODE_RGB_P
		| S3C_VIDCON0_CLKVALUP_ALWAYS | S3C_VIDCON0_CLKDIR_DIVIDED | S3C_VIDCON0_CLKSEL_F_HCLK |
		S3C_VIDCON0_ENVID_DISABLE | S3C_VIDCON0_ENVID_F_DISABLE,
	.vidcon1 = S3C_VIDCON1_IHSYNC_INVERT | S3C_VIDCON1_IVSYNC_INVERT |S3C_VIDCON1_IVDEN_NORMAL,
	.vidtcon0 = S3C_VIDTCON0_VBPDE(0) | S3C_VIDTCON0_VBPD(V_BP-1) | S3C_VIDTCON0_VFPD(V_FP-1) | S3C_VIDTCON0_VSPW(V_SW-1),
	.vidtcon1 = S3C_VIDTCON1_VFPDE(0) | S3C_VIDTCON1_HBPD(H_BP-1) | S3C_VIDTCON1_HFPD(H_FP-1) | S3C_VIDTCON1_HSPW(H_SW-1),
#endif
	.vidtcon2= S3C_VIDTCON2_LINEVAL(V_RESOLUTION-1) | S3C_VIDTCON2_HOZVAL(H_RESOLUTION-1),

#if defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
	.dithmode = ( S3C_DITHMODE_RDITHPOS_5BIT |S3C_DITHMODE_GDITHPOS_6BIT | S3C_DITHMODE_BDITHPOS_5BIT ) & S3C_DITHMODE_DITHERING_DISABLE,
#endif

#if defined (CONFIG_FB_BPP_8)
	.wincon0=  S3C_WINCONx_BYTSWP_ENABLE |S3C_WINCONx_BURSTLEN_4WORD |  S3C_WINCONx_BPPMODE_F_8BPP_PAL,  // 4word burst, 8bpp-palletized,
	.wincon1=  S3C_WINCONx_HAWSWP_ENABLE | S3C_WINCONx_BURSTLEN_4WORD |  S3C_WINCONx_BPPMODE_F_16BPP_565 | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_ALPHA_SEL_1,  // 4word burst, 16bpp for OSD
	//.wincon1=  S3C_WINCONx_HAWSWP_ENABLE | S3C_WINCONx_BURSTLEN_4WORD |  S3C_WINCONx_BPPMODE_F_16BPP_A555 | S3C_WINCONx_BLD_PIX_PIXEL| S3C_WINCONx_ALPHA_SEL_1,  // 4word burst, 16bpp for OSD

#elif defined (CONFIG_FB_BPP_16)
	#if defined(CONFIG_CPU_S3C2443) ||  defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416)
	.wincon0=  S3C_WINCONx_HAWSWP_ENABLE | S3C_WINCONx_BURSTLEN_4WORD |  S3C_WINCONx_BPPMODE_F_16BPP_565,  // 4word burst, 16bpp,
	.wincon1=  S3C_WINCONx_HAWSWP_ENABLE | S3C_WINCONx_BURSTLEN_4WORD |  S3C_WINCONx_BPPMODE_F_16BPP_565 | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_ALPHA_SEL_1,  // 4word burst, 16bpp for OSD
	//.wincon1=  S3C_WINCONx_HAWSWP_ENABLE | S3C_WINCONx_BURSTLEN_4WORD |  S3C_WINCONx_BPPMODE_F_16BPP_A555 | S3C_WINCONx_BLD_PIX_PIXEL| S3C_WINCONx_ALPHA_SEL_1,  // 4word burst, 16bpp for OSD
	#elif defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
//	.wincon0 =  S3C_WINCONx_ENLOCAL_DMA | S3C_WINCONx_BUFSEL_0 | S3C_WINCONx_BUFAUTOEN_DISABLE | S3C_WINCONx_BITSWP_DISABLE |
	.wincon0 =  S3C_WINCONx_ENLOCAL_DMA | S3C_WINCONx_BUFSEL_1 | S3C_WINCONx_BUFAUTOEN_DISABLE | S3C_WINCONx_BITSWP_DISABLE |
		S3C_WINCONx_BYTSWP_DISABLE | S3C_WINCONx_HAWSWP_ENABLE|
		S3C_WINCONx_BURSTLEN_16WORD | S3C_WINCONx_BPPMODE_F_16BPP_565 |
#if defined(CONFIG_FB_DOUBLE_BUFFERING)
		//S3C_WINCONx_BUFAUTOEN_ENABLE |
#endif
		S3C_WINCONx_ENWIN_F_DISABLE,  // 4word burst, 16bpp,

	.wincon1 =  S3C_WINCONx_ENLOCAL_DMA | S3C_WINCONx_BUFSEL_0 | S3C_WINCONx_BUFAUTOEN_DISABLE | S3C_WINCONx_BITSWP_DISABLE |
		S3C_WINCONx_BYTSWP_DISABLE | S3C_WINCONx_HAWSWP_ENABLE |
		S3C_WINCONx_BURSTLEN_16WORD | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_BPPMODE_F_16BPP_565 |

#if defined(CONFIG_FB_DOUBLE_BUFFERING)
		//S3C_WINCONx_BUFAUTOEN_ENABLE |
#endif
		S3C_WINCONx_ALPHA_SEL_1 | S3C_WINCONx_ENWIN_F_DISABLE,  // 4word burst, 16bpp,

	.wincon2 = S3C_WINCONx_ENLOCAL_DMA | S3C_WINCONx_BITSWP_DISABLE |
		S3C_WINCONx_BYTSWP_DISABLE | S3C_WINCONx_HAWSWP_ENABLE|
		S3C_WINCONx_BURSTLEN_4WORD | S3C_WINCONx_BURSTLEN_16WORD | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_BPPMODE_F_16BPP_565 |
		S3C_WINCONx_ALPHA_SEL_1 | S3C_WINCONx_ENWIN_F_DISABLE,

	.wincon3 = S3C_WINCONx_BITSWP_DISABLE | S3C_WINCONx_BYTSWP_DISABLE | S3C_WINCONx_HAWSWP_ENABLE |
		S3C_WINCONx_BURSTLEN_4WORD | S3C_WINCONx_BURSTLEN_16WORD | S3C_WINCONx_BLD_PIX_PLANE |
		S3C_WINCONx_BPPMODE_F_16BPP_565 | S3C_WINCONx_ALPHA_SEL_1 | S3C_WINCONx_ENWIN_F_DISABLE,

	.wincon4 = S3C_WINCONx_BITSWP_DISABLE | S3C_WINCONx_BYTSWP_DISABLE | S3C_WINCONx_HAWSWP_ENABLE|
		S3C_WINCONx_BURSTLEN_4WORD | S3C_WINCONx_BURSTLEN_16WORD | S3C_WINCONx_BLD_PIX_PLANE |
		S3C_WINCONx_BPPMODE_F_16BPP_565 | S3C_WINCONx_ALPHA_SEL_1 | S3C_WINCONx_ENWIN_F_DISABLE,

	#endif
#elif defined (CONFIG_FB_BPP_24)
	.wincon0=  S3C_WINCONx_HAWSWP_DISABLE | S3C_WINCONx_BURSTLEN_16WORD |  S3C_WINCONx_BPPMODE_F_24BPP_888,  // 4word burst, 24bpp,
	.wincon1=  S3C_WINCONx_HAWSWP_DISABLE | S3C_WINCONx_BURSTLEN_16WORD |  S3C_WINCONx_BPPMODE_F_24BPP_888 | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_ALPHA_SEL_1,  // 4word burst, 24bpp for OSD
	#if defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
	.wincon2 = S3C_WINCONx_HAWSWP_DISABLE | S3C_WINCONx_BURSTLEN_16WORD |  S3C_WINCONx_BPPMODE_F_24BPP_888 | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_ALPHA_SEL_1,
	.wincon3 = S3C_WINCONx_HAWSWP_DISABLE | S3C_WINCONx_BURSTLEN_16WORD |  S3C_WINCONx_BPPMODE_F_24BPP_888 | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_ALPHA_SEL_1,
	.wincon4 = S3C_WINCONx_HAWSWP_DISABLE | S3C_WINCONx_BURSTLEN_16WORD |  S3C_WINCONx_BPPMODE_F_24BPP_888 | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_ALPHA_SEL_1,
	#endif
#endif

#if defined(CONFIG_CPU_S3C2443) ||  defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416)
	.vidosd0a= S3C_VIDOSDxA_OSD_LTX_F(0) | S3C_VIDOSDxA_OSD_LTY_F(0),
	.vidosd0b= S3C_VIDOSDxB_OSD_RBX_F(H_RESOLUTION-1) | S3C_VIDOSDxB_OSD_RBY_F(V_RESOLUTION-1),

	.vidosd1a= S3C_VIDOSDxA_OSD_LTX_F(0) | S3C_VIDOSDxA_OSD_LTY_F(0),
	.vidosd1b= S3C_VIDOSDxB_OSD_RBX_F(H_RESOLUTION_OSD-1) | S3C_VIDOSDxB_OSD_RBY_F(V_RESOLUTION_OSD-1),
	.vidosd1c= S3C_VIDOSDxC_ALPHA1_B(MAX_ALPHA_LEVEL) | S3C_VIDOSDxC_ALPHA1_G(MAX_ALPHA_LEVEL) |S3C_VIDOSDxC_ALPHA1_R(MAX_ALPHA_LEVEL),
#elif defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
	.vidosd0a = S3C_VIDOSDxA_OSD_LTX_F(0) | S3C_VIDOSDxA_OSD_LTY_F(0),
	.vidosd0b = S3C_VIDOSDxB_OSD_RBX_F(H_RESOLUTION-1) | S3C_VIDOSDxB_OSD_RBY_F(V_RESOLUTION-1),
	.vidosd0c = S3C_VIDOSDxD_OSDSIZE(H_RESOLUTION*V_RESOLUTION),

	.vidosd1a = S3C_VIDOSDxA_OSD_LTX_F(0) | S3C_VIDOSDxA_OSD_LTY_F(0),
	.vidosd1b = S3C_VIDOSDxB_OSD_RBX_F(H_RESOLUTION_OSD-1) | S3C_VIDOSDxB_OSD_RBY_F(V_RESOLUTION_OSD-1),
	.vidosd1c= S3C_VIDOSDxC_ALPHA1_B(MAX_ALPHA_LEVEL) | S3C_VIDOSDxC_ALPHA1_G(MAX_ALPHA_LEVEL) |S3C_VIDOSDxC_ALPHA1_R(MAX_ALPHA_LEVEL),
	.vidosd1d = S3C_VIDOSDxD_OSDSIZE(H_RESOLUTION*V_RESOLUTION),

	.vidosd2a = S3C_VIDOSDxA_OSD_LTX_F(0) | S3C_VIDOSDxA_OSD_LTY_F(0),
	.vidosd2b = S3C_VIDOSDxB_OSD_RBX_F(H_RESOLUTION_OSD-1) | S3C_VIDOSDxB_OSD_RBY_F(V_RESOLUTION_OSD-1),
	.vidosd2c = S3C_VIDOSDxC_ALPHA1_B(MAX_ALPHA_LEVEL) | S3C_VIDOSDxC_ALPHA1_G(MAX_ALPHA_LEVEL) |S3C_VIDOSDxC_ALPHA1_R(MAX_ALPHA_LEVEL),
	.vidosd2d = S3C_VIDOSDxD_OSDSIZE(H_RESOLUTION*V_RESOLUTION),

	.vidosd3a = S3C_VIDOSDxA_OSD_LTX_F(0) | S3C_VIDOSDxA_OSD_LTY_F(0),
	.vidosd3b = S3C_VIDOSDxB_OSD_RBX_F(H_RESOLUTION_OSD-1) | S3C_VIDOSDxB_OSD_RBY_F(V_RESOLUTION_OSD-1),
	.vidosd3c = S3C_VIDOSDxC_ALPHA1_B(MAX_ALPHA_LEVEL) | S3C_VIDOSDxC_ALPHA1_G(MAX_ALPHA_LEVEL) |S3C_VIDOSDxC_ALPHA1_R(MAX_ALPHA_LEVEL),

	.vidosd4a = S3C_VIDOSDxA_OSD_LTX_F(0) | S3C_VIDOSDxA_OSD_LTY_F(0),
	.vidosd4b = S3C_VIDOSDxB_OSD_RBX_F(H_RESOLUTION_OSD-1) | S3C_VIDOSDxB_OSD_RBY_F(V_RESOLUTION_OSD-1),
	.vidosd4c = S3C_VIDOSDxC_ALPHA1_B(MAX_ALPHA_LEVEL) | S3C_VIDOSDxC_ALPHA1_G(MAX_ALPHA_LEVEL) |S3C_VIDOSDxC_ALPHA1_R(MAX_ALPHA_LEVEL),
#endif

#if defined(CONFIG_CPU_S3C2443) ||  defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416)
	.vidintcon= S3C_VIDINTCON0_FRAMESEL0_VSYNC | S3C_VIDINTCON0_FRAMESEL1_NONE | S3C_VIDINTCON0_INTFRMEN_ENABLE | S3C_VIDINTCON0_INTEN_ENABLE,
#elif defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
//	.vidintcon0 = S3C_VIDINTCON0_FRAMESEL0_BACK | S3C_VIDINTCON0_FRAMESEL1_NONE | S3C_VIDINTCON0_INTFRMEN_DISABLE | S3C_VIDINTCON0_FIFOSEL_WIN0 | S3C_VIDINTCON0_FIFOLEVEL_25 | S3C_VIDINTCON0_INTFIFOEN_DISABLE | S3C_VIDINTCON0_INTEN_DISABLE,
	.vidintcon0 = S3C_VIDINTCON0_FRAMESEL0_VSYNC | S3C_VIDINTCON0_FRAMESEL1_NONE | S3C_VIDINTCON0_INTFRMEN_DISABLE | S3C_VIDINTCON0_FIFOSEL_WIN0 | S3C_VIDINTCON0_FIFOLEVEL_25 | S3C_VIDINTCON0_INTFIFOEN_DISABLE | S3C_VIDINTCON0_INTEN_ENABLE,

	.vidintcon1 = 0,
#endif

	.width=	H_RESOLUTION,
	.height= V_RESOLUTION,
	.xres=	H_RESOLUTION,
	.yres=	V_RESOLUTION,

	.xoffset=	0,
	.yoffset=	0,

#if defined(CONFIG_FB_VIRTUAL_SCREEN)
	.xres_virtual =	H_RESOLUTION_VIRTUAL,
	.yres_virtual =	V_RESOLUTION_VIRTUAL,
#else
	.xres_virtual =	H_RESOLUTION,
	.yres_virtual =	V_RESOLUTION,
#endif

	.osd_width=	H_RESOLUTION_OSD,
	.osd_height=	V_RESOLUTION_OSD,
	.osd_xres=	H_RESOLUTION_OSD,
	.osd_yres=	V_RESOLUTION_OSD,

	.osd_xres_virtual=	H_RESOLUTION_OSD,
	.osd_yres_virtual=	V_RESOLUTION_OSD,

#if defined (CONFIG_FB_BPP_8)
	.bpp=		PIXEL_BPP8,
	.bytes_per_pixel= 1,
	#if defined(CONFIG_CPU_S3C2443) ||  defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416)
	.wpalcon=	W0PAL_24BIT,
	#elif defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
	.wpalcon=	W0PAL_16BIT,
	#endif
#elif defined (CONFIG_FB_BPP_16)
	.bpp=		PIXEL_BPP16,
	.bytes_per_pixel= 2,

#elif defined (CONFIG_FB_BPP_24)
        .bpp=		PIXEL_BPP24,
        .bytes_per_pixel= 4,

#endif
      	.pixclock=	PIXEL_CLOCK,

	.w1keycon0=	S3C_WxKEYCON0_KEYBLEN_DISABLE | S3C_WxKEYCON0_KEYEN_F_DISABLE | S3C_WxKEYCON0_DIRCON_MATCH_FG_IMAGE |
			S3C_WxKEYCON0_COMPKEY(0x0),
	.w1keycon1=	S3C_WxKEYCON1_COLVAL(0xffffff),
	.w2keycon0=	S3C_WxKEYCON0_KEYBLEN_DISABLE | S3C_WxKEYCON0_KEYEN_F_DISABLE | S3C_WxKEYCON0_DIRCON_MATCH_FG_IMAGE |
			S3C_WxKEYCON0_COMPKEY(0x0),
	.w2keycon1=	S3C_WxKEYCON1_COLVAL(0xffffff),
	.w3keycon0=	S3C_WxKEYCON0_KEYBLEN_DISABLE | S3C_WxKEYCON0_KEYEN_F_DISABLE | S3C_WxKEYCON0_DIRCON_MATCH_FG_IMAGE |
			S3C_WxKEYCON0_COMPKEY(0x0),
	.w3keycon1=	S3C_WxKEYCON1_COLVAL(0xffffff),
	.w4keycon0=	S3C_WxKEYCON0_KEYBLEN_DISABLE | S3C_WxKEYCON0_KEYEN_F_DISABLE | S3C_WxKEYCON0_DIRCON_MATCH_FG_IMAGE |
			S3C_WxKEYCON0_COMPKEY(0x0),
	.w4keycon1=	S3C_WxKEYCON1_COLVAL(0xffffff),


      	.hsync_len= 	H_SW,
	.vsync_len=	V_SW,

      	.left_margin= 	H_FP,
	.upper_margin=	V_FP,
      	.right_margin= 	H_BP,
	.lower_margin=	V_BP,

      	.sync= 		0,
	.cmap_static=	1,
};

#if defined(CONFIG_S3C6400_PWM) || defined(CONFIG_S3C2450_PWM) || defined(CONFIG_S3C2416_PWM)
void set_brightness(int val)
{

#if defined(CONFIG_S3C6400_PWM)
	int channel = 1;  // must use channel-1
#elif defined(CONFIG_S3C2450_PWM) || defined(CONFIG_S3C2416_PWM)
	int channel = 3;  // must use channel-3
#endif
	int usec = 0;       // don't care value
	unsigned long tcnt=1000;
	unsigned long tcmp=0;

	if(val < 0) val=0;
	if(val > MAX_DISPLAY_BRIGHTNESS) val=MAX_DISPLAY_BRIGHTNESS;

	display_brightness = val;

	switch (val) {
		case 0:
			tcmp= 0;
			break;
		case 1:
			tcmp= 50;
			break;
		case 2:
			tcmp= 100;
			break;
		case 3:
			tcmp= 150;
			break;
		case 4:
			tcmp= 200;
			break;
		case 5:
			tcmp= 250;
			break;
		case 6:
			tcmp= 300;
			break;
		case 7:
			tcmp= 350;
			break;
		case 8:
			tcmp= 400;
			break;
		case 9:
			tcmp= 450;
			break;
	}  // end of switch (level)

#if defined(CONFIG_S3C6400_PWM)
	s3c6400_timer_setup (channel, usec, tcnt, tcmp);
#else
	s3c2450_timer_setup (channel, usec, tcnt, tcmp);
#endif

}
#endif


#if defined(CONFIG_FB_VIRTUAL_SCREEN)
void set_virtual_display_offset(int val)
{
	if(val < 1)
	   val = 1;
	if(val > MAX_DISPLAY_OFFSET)
	   val = MAX_DISPLAY_OFFSET;

	virtual_display_offset = val;
}

int set_vs_info(vs_info_t vs_info_from_app )
{

	/* check invalid value */
	if(vs_info_from_app.width != H_RESOLUTION || vs_info_from_app.height != V_RESOLUTION ){
		return 1;
	}
	if(!(vs_info_from_app.bpp==8 ||vs_info_from_app.bpp==16 ||vs_info_from_app.bpp==24 || vs_info_from_app.bpp==32) ){
		return 1;
	}
	if(vs_info_from_app.offset<0){
		return 1;
	}
	if(vs_info_from_app.v_width != H_RESOLUTION_VIRTUAL  || vs_info_from_app.v_height != V_RESOLUTION_VIRTUAL){
		return 1;
	}

	/* save virtual screen information */
	vs_info = vs_info_from_app;
	set_virtual_display_offset(vs_info.offset);
	return 0;
}


int set_virtual_display_register(int vs_cmd)
{
	int PageWidth, Offset;
	int ShiftValue;

	PageWidth = mach_info.xres * mach_info.bytes_per_pixel;
	Offset = (mach_info.xres_virtual - mach_info.xres) * mach_info.bytes_per_pixel;

	switch(vs_cmd){
		case SET_VIRTUAL_LCD:
			/* size of buffer */
			#if defined(CONFIG_CPU_S3C2443) ||  defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416)
			mach_info.vidw00add2b0 = S3C_VIDWxxADD2_OFFSIZE_F(Offset) | (S3C_VIDWxxADD2_PAGEWIDTH_F(PageWidth));
			mach_info.vidw00add2b1 = S3C_VIDWxxADD2_OFFSIZE_F(Offset) | (S3C_VIDWxxADD2_PAGEWIDTH_F(PageWidth));

			__raw_writel(mach_info.vidw00add2b0, S3C_VIDW00ADD2B0);
			__raw_writel(mach_info.vidw00add2b1, S3C_VIDW00ADD2B1);
			#elif defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
			mach_info.vidw00add2 = S3C_VIDWxxADD2_OFFSIZE_F(Offset) | (S3C_VIDWxxADD2_PAGEWIDTH_F(PageWidth));
			__raw_writel(mach_info.vidw00add2, S3C_VIDW00ADD2);
			#endif

			break;

		case VS_MOVE_LEFT:
			if(mach_info.xoffset < virtual_display_offset){
				ShiftValue = mach_info.xoffset;
			}
			else ShiftValue = virtual_display_offset;
			mach_info.xoffset -= ShiftValue;

			/* For buffer start address */
			mach_info.vidw00add0b0 = mach_info.vidw00add0b0 - ShiftValue*mach_info.bytes_per_pixel;
			mach_info.vidw00add0b1 = mach_info.vidw00add0b1 - ShiftValue*mach_info.bytes_per_pixel;
			break;

		case VS_MOVE_RIGHT:
			if((vs_info.v_width - (mach_info.xoffset + vs_info.width) )< (virtual_display_offset)){
				ShiftValue = vs_info.v_width - (mach_info.xoffset + vs_info.width);
			}
			else ShiftValue = virtual_display_offset;
			mach_info.xoffset += ShiftValue;

			/* For buffer start address */
			mach_info.vidw00add0b0 = mach_info.vidw00add0b0 + ShiftValue*mach_info.bytes_per_pixel;
			mach_info.vidw00add0b1 = mach_info.vidw00add0b1 + ShiftValue*mach_info.bytes_per_pixel;
			break;

		case VS_MOVE_UP:
			if(mach_info.yoffset < virtual_display_offset){
				ShiftValue = mach_info.yoffset;
			}
			else ShiftValue = virtual_display_offset;
			mach_info.yoffset -= ShiftValue;

			/* For buffer start address */
			mach_info.vidw00add0b0 = mach_info.vidw00add0b0 - ShiftValue*mach_info.xres_virtual*mach_info.bytes_per_pixel;
			mach_info.vidw00add0b1 = mach_info.vidw00add0b1 - ShiftValue*mach_info.xres_virtual*mach_info.bytes_per_pixel;
			break;

		case VS_MOVE_DOWN:
			if((vs_info.v_height - (mach_info.yoffset + vs_info.height) )< (virtual_display_offset)){
				ShiftValue = vs_info.v_height - (mach_info.yoffset + vs_info.height);
			}
			else ShiftValue = virtual_display_offset;
			mach_info.yoffset += ShiftValue;

			/* For buffer start address */
			mach_info.vidw00add0b0 = mach_info.vidw00add0b0 + ShiftValue*mach_info.xres_virtual*mach_info.bytes_per_pixel;
			mach_info.vidw00add0b1 = mach_info.vidw00add0b1 + ShiftValue*mach_info.xres_virtual*mach_info.bytes_per_pixel;
			break;

		default:
			return -EINVAL;
	}

	/* End address */
	mach_info.vidw00add1b0 = S3C_VIDWxxADD1_VBASEL_F(mach_info.vidw00add0b0 + (PageWidth + Offset) * (mach_info.yres));
	mach_info.vidw00add1b1 = S3C_VIDWxxADD1_VBASEL_F(mach_info.vidw00add0b1 + (PageWidth + Offset) * (mach_info.yres));

	__raw_writel(mach_info.vidw00add0b0, S3C_VIDW00ADD0B0);
	__raw_writel(mach_info.vidw00add0b1, S3C_VIDW00ADD0B1);

	__raw_writel(mach_info.vidw00add1b0, S3C_VIDW00ADD1B0);
	__raw_writel(mach_info.vidw00add1b1, S3C_VIDW00ADD1B1);

	return 0;
}
#endif


/*
 * As LCD-Brightness is best related to Display and hence FrameBuffer,
 * so we put the brightness control in /dev/fb.
 */
#define GET_DISPLAY_BRIGHTNESS   	 _IOR('F', 1, u_int)             /* get brightness */
#define SET_DISPLAY_BRIGHTNESS    	 _IOW('F', 2, u_int)             /* set brightness */


#if defined(CONFIG_FB_VIRTUAL_SCREEN)
#define SET_VS_START 			_IO('F', 103)
#define SET_VS_STOP 			_IO('F', 104)
#define SET_VS_INFO 			_IOW('F', 105, vs_info_t)
#define SET_VS_MOVE 			_IOW('F', 106, u_int)
#endif

#define SET_OSD_START 			_IO('F', 201)
#define SET_OSD_STOP 			_IO('F', 202)
#define SET_OSD_ALPHA_UP 		_IO('F', 203)
#define SET_OSD_ALPHA_DOWN 		_IO('F', 204)
#define SET_OSD_MOVE_LEFT 		_IO('F', 205)
#define SET_OSD_MOVE_RIGHT		_IO('F', 206)
#define SET_OSD_MOVE_UP 		_IO('F', 207)
#define SET_OSD_MOVE_DOWN		_IO('F', 208)
#define SET_OSD_INFO			_IOW('F', 209, s3c_win_info_t)
#if 0
#define SET_OSD_BRIGHT			_IOW('F', 210, int)
#endif

#define SET_COLOR_KEY_START 		_IO('F', 300)
#define SET_COLOR_KEY_STOP 		_IO('F', 301)
#define SET_COLOR_KEY_ALPHA_START 	_IO('F', 302)
#define SET_COLOR_KEY_ALPHA_STOP 	_IO('F', 303)
#define SET_COLOR_KEY_INFO		_IOW('F', 304, s3c_color_key_info_t)
#define SET_COLOR_KEY_COLVAL		_IOW('F', 305, s3c_color_val_info_t)

#if defined(CONFIG_FB_DOUBLE_BUFFERING)
#define GET_FB_NUM			_IOWR('F', 306, u_int)
#endif

//#if defined(CONFIG_G3D)
#define S3C_FBIO_CHANGE_FB		_IOW('F', 81,int)
//#endif

#define GET_FB_INFO			_IOR('F', 307, struct s3c_fb_dma_info)
#define SET_FB_CHANGE_REQ		_IOW('F', 308, int)
#define SET_VSYNC_INT			_IOW('F', 309, int)
#define VIRT_TO_PHYS			_IOWR('F', 310, u_int)

static void s3c_fb_change_fb(struct fb_info *info);
int s3c_fb_ioctl(struct fb_info *info, unsigned int cmd, unsigned long arg)
{
	struct s3c_fb_info *fbi = container_of(info, struct s3c_fb_info, fb);
	struct fb_var_screeninfo *var= &fbi->fb.var;
	struct s3c_fb_dma_info dma_info;

	struct vm_area_struct *tvma;
	struct page *tpage;
	u_int virt_addr, phys_addr, offset;
	struct mm_struct *mm = current->mm;

	int brightness;
	s3c_win_info_t win_info_from_app;
	s3c_color_key_info_t colkey_info;
	s3c_color_val_info_t colval_info;

#if defined(CONFIG_FB_DOUBLE_BUFFERING)
	u_int f_num_val;
#endif

#if defined(CONFIG_FB_VIRTUAL_SCREEN)
	vs_info_t vs_info_from_app;
#endif

	switch(cmd){
		case VIRT_TO_PHYS:
				
				if(copy_from_user((void *)&virt_addr, (const void *)arg, sizeof(u_int)))
					return -EFAULT;
				//printk("\n   KERNEL : virt addr = 0x%p\n", virt_addr);
				offset = virt_addr & ~PAGE_MASK;
				//printk("    KERNEL : offset = 0x%x\n", offset);

				tvma = find_vma(mm, virt_addr);
				
				if (tvma->vm_start <= mm->start_brk &&  tvma->vm_end >= mm->brk) {
					printk("This virt address is on heap !\n");
					//printk("heap start :0x%p !\n", mm->start_brk);
					//printk("heap end :0x%p !\n", mm->brk);
				}
				
				tpage = follow_page(tvma, virt_addr, FOLL_GET);
			
				phys_addr = (page_to_pfn(tpage) << PAGE_SHIFT) + offset;
				//printk("    KERNEL  : phys addr: %p \n", phys_addr);
				
				if(copy_to_user((void *)arg, (const void *) &phys_addr, sizeof(u_int)))
					return -EFAULT;
				
				return phys_addr;
				
		case GET_FB_INFO:
				dma_info.map_dma_f1 = fbi->map_dma_f1;
				dma_info.map_dma_f2 = fbi->map_dma_f2;

				if(copy_to_user((void *) arg, (const void *) &dma_info, sizeof(struct s3c_fb_dma_info)))
					return -EFAULT;
				break;
#if defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
		case SET_VSYNC_INT:
				mach_info.vidintcon0 &= ~S3C_VIDINTCON0_FRAMESEL0_MASK;
				mach_info.vidintcon0 |= S3C_VIDINTCON0_FRAMESEL0_VSYNC;

				if (arg)
					mach_info.vidintcon0 |= S3C_VIDINTCON0_INTFRMEN_ENABLE;
				else
					mach_info.vidintcon0 &= ~S3C_VIDINTCON0_INTFRMEN_ENABLE;

				__raw_writel(mach_info.vidintcon0, S3C_VIDINTCON0);
				break;
#endif

#if defined(CONFIG_FB_VIRTUAL_SCREEN)
		case SET_VS_START:
				mach_info.wincon0 &= ~(S3C_WINCONx_ENWIN_F_ENABLE);
				__raw_writel(mach_info.wincon0|S3C_WINCONx_ENWIN_F_ENABLE, S3C_WINCON0);

				fbi->fb.var.xoffset = mach_info.xoffset;
				fbi->fb.var.yoffset = mach_info.yoffset;
				break;

		case SET_VS_STOP:
				break;

		case SET_VS_INFO:
				if(copy_from_user(&vs_info_from_app, (vs_info_t *) arg, sizeof(vs_info_t)))
					return -EFAULT;

				if(set_vs_info(vs_info_from_app)){
					printk("Error SET_VS_INFO\n");
					return -EINVAL;
				}

				set_virtual_display_register(SET_VIRTUAL_LCD);

				fbi->fb.var.xoffset = mach_info.xoffset;
				fbi->fb.var.yoffset = mach_info.yoffset;
				break;

		case SET_VS_MOVE:
				set_virtual_display_register(arg);

				fbi->fb.var.xoffset = mach_info.xoffset;
				fbi->fb.var.yoffset = mach_info.yoffset;
				break;
#endif

		case SET_OSD_INFO :
				if(copy_from_user
				 (&win_info_from_app, (s3c_win_info_t *) arg,
				   sizeof(s3c_win_info_t)))
					return -EFAULT;

				s3c_fb_init_win(fbi, win_info_from_app.Bpp,
				win_info_from_app.LeftTop_x, win_info_from_app.LeftTop_y, win_info_from_app.Width, win_info_from_app.Height, OFF);
				break;

		case SET_OSD_START :
				s3c_fb_win_onoff(fbi, ON); // on
				break;

		case SET_OSD_STOP:
				s3c_fb_win_onoff(fbi, OFF); // off
				break;

		case SET_OSD_ALPHA_UP:
				if(osd_alpha_level < MAX_ALPHA_LEVEL) osd_alpha_level ++;
				s3c_fb_set_alpha_level(fbi);
				break;

		case SET_OSD_ALPHA_DOWN:
				if(osd_alpha_level > 0) osd_alpha_level --;
				s3c_fb_set_alpha_level(fbi);
				break;

		case SET_OSD_MOVE_LEFT:
				if(var->xoffset>0) var->xoffset--;
				s3c_fb_set_position_win(fbi, var->xoffset, var->yoffset, var->xres, var->yres);
				break;

		case SET_OSD_MOVE_RIGHT:
				if(var->xoffset < (H_RESOLUTION - var->xres)) var->xoffset ++;
				s3c_fb_set_position_win(fbi, var->xoffset, var->yoffset, var->xres, var->yres);
				break;

		case SET_OSD_MOVE_UP:
				if(var->yoffset>0) var->yoffset--;
				s3c_fb_set_position_win(fbi, var->xoffset, var->yoffset, var->xres, var->yres);
				break;

		case SET_OSD_MOVE_DOWN:
				if(var->yoffset < (V_RESOLUTION - var->yres)) var->yoffset ++;
				s3c_fb_set_position_win(fbi, var->xoffset, var->yoffset, var->xres, var->yres);
				break;

#if defined(CONFIG_FB_DOUBLE_BUFFERING)
		case GET_FB_NUM:
				if(copy_from_user((void *)&f_num_val, (const void *)arg, sizeof(u_int)))
					return -EFAULT;

				if(copy_to_user((void *)arg, (const void *) &f_num_val, sizeof(u_int)))
					return -EFAULT;

				break;

		case SET_FB_CHANGE_REQ:
				s3c_fb_change_buff(0, (int) arg);
				break;
#endif

		case S3C_FBIO_CHANGE_FB:
				s3c_fb_change_fb(info);
				break;

		case SET_DISPLAY_BRIGHTNESS:
				if(copy_from_user(&brightness, (int *) arg, sizeof(int)))
					return -EFAULT;
				#if defined(CONFIG_S3C6400_PWM) || defined(CONFIG_S3C2450_PWM) || defined(CONFIG_S3C2416_PWM)
				set_brightness(brightness);
				#endif
				break;

		case GET_DISPLAY_BRIGHTNESS:
				if(copy_to_user((void *)arg, (const void *) &display_brightness, sizeof(int)))
					return -EFAULT;
				break;

		//(WAITFORVSYNC)
    		case FBIO_WAITFORVSYNC:
  			{
  				u_int32_t crt;

  				if (get_user(crt, (u_int32_t __user *)arg))
  					return -EFAULT;

  				return s3cfb_wait_for_sync(crt);
  			}
    		//(WAITFORVSYNC)

		case SET_COLOR_KEY_START:
				s3c_fb_color_key_onoff(fbi, ON);
				break;

		case SET_COLOR_KEY_STOP:
				s3c_fb_color_key_onoff(fbi, OFF);
				break;

		case SET_COLOR_KEY_ALPHA_START:
				s3c_fb_color_key_alpha_onoff(fbi, ON);
				break;

		case SET_COLOR_KEY_ALPHA_STOP:
				s3c_fb_color_key_alpha_onoff(fbi, OFF);
				break;

		case SET_COLOR_KEY_INFO:
				if(copy_from_user(&colkey_info, (s3c_color_key_info_t *) arg,
				   sizeof(s3c_color_key_info_t)))
					return -EFAULT;

				s3c_fb_setup_color_key_register(fbi, colkey_info);
				break;

		case SET_COLOR_KEY_COLVAL:
				if(copy_from_user(&colval_info, (s3c_color_val_info_t *) arg,
				   sizeof(s3c_color_val_info_t)))
					return -EFAULT;

				s3c_fb_set_color_value(fbi, colval_info);
				break;

		default:
			return -EINVAL;
	}

	return 0;
}


/*
 * s3c_fb_map_video_memory():
 *	Allocates the DRAM memory for the frame buffer.  This buffer is
 *	remapped into a non-cached, non-buffered, memory region to
 *	allow palette and pixel writes to occur without flushing the
 *	cache.  Once this area is remapped, all virtual memory
 *	access to the video memory should occur at the new region.
 */
 int __init s3c_fb_map_video_memory(struct s3c_fb_info *fbi)
{
	dprintk("map_video_memory(fbi=%p)\n", fbi);

	fbi->map_size_f1 = PAGE_ALIGN(fbi->fb.fix.smem_len + PAGE_SIZE);
	fbi->map_cpu_f1 = dma_alloc_writecombine(fbi->dev, fbi->map_size_f1,
					      &fbi->map_dma_f1, GFP_KERNEL);

	fbi->map_size_f1 = fbi->fb.fix.smem_len;

	if (fbi->map_cpu_f1) {
		/* prevent initial garbage on screen */
		printk("Window[%d]- FB1 : map_video_memory: clear %p:%08x\n",
			fbi->win_id, fbi->map_cpu_f1, fbi->map_size_f1);
		memset(fbi->map_cpu_f1, 0xf0, fbi->map_size_f1);

		fbi->screen_dma_f1 = fbi->map_dma_f1;
		fbi->fb.screen_base = fbi->map_cpu_f1;
		fbi->fb.fix.smem_start = fbi->screen_dma_f1;

		printk("           FB1 : map_video_memory: dma=%08x cpu=%p size=%08x\n",
			fbi->map_dma_f1, fbi->map_cpu_f1, fbi->fb.fix.smem_len);
	}
	if( !fbi->map_cpu_f1)
		return -ENOMEM;


//#if !defined(CONFIG_FB_VIRTUAL_SCREEN) && defined(CONFIG_FB_DOUBLE_BUFFERING)
#if  defined(CONFIG_FB_DOUBLE_BUFFERING)
	#if defined(CONFIG_CPU_S3C2443) ||  defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416)
	if(fbi->win_id<1){  // WIN0 support double-buffer

	#elif defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
	if(fbi->win_id<2){  // WIN0, WIN1 support double-buffer
	#endif

	fbi->map_size_f2 = PAGE_ALIGN(fbi->fb.fix.smem_len + PAGE_SIZE);
	fbi->map_cpu_f2 = dma_alloc_writecombine(fbi->dev, fbi->map_size_f2,
					      &fbi->map_dma_f2, GFP_KERNEL);

	fbi->map_size_f2 = fbi->fb.fix.smem_len;

	if (fbi->map_cpu_f2) {
		/* prevent initial garbage on screen */
		printk("Window[%d] - FB2 : map_video_memory: clear %p:%08x\n",
			fbi->win_id, fbi->map_cpu_f2, fbi->map_size_f2);
		memset(fbi->map_cpu_f2, 0xf0, fbi->map_size_f2);

		fbi->screen_dma_f2 = fbi->map_dma_f2;

		printk("            FB2 : map_video_memory: dma=%08x cpu=%p size=%08x\n",
			fbi->map_dma_f2, fbi->map_cpu_f2, fbi->fb.fix.smem_len);
	}
	if( !fbi->map_cpu_f2)
		return -ENOMEM;
	}
#endif

	return 0;
}



void s3c_fb_unmap_video_memory(struct s3c_fb_info *fbi)
{
	dma_free_writecombine(fbi->dev, fbi->map_size_f1, fbi->map_cpu_f1,  fbi->map_dma_f1);

//#if !defined(CONFIG_FB_VIRTUAL_SCREEN) && defined(CONFIG_FB_DOUBLE_BUFFERING)
#if  defined(CONFIG_FB_DOUBLE_BUFFERING)
	dma_free_writecombine(fbi->dev, fbi->map_size_f2, fbi->map_cpu_f2,  fbi->map_dma_f2);
#endif
}

/*
 * s3c_fb_init_registers - Initialise all LCD-related registers
 */
void lcd_module_init(void);
int s3c_fb_init_registers(struct s3c_fb_info *fbi)
{
	u_long flags = 0;
	u_long PageWidth = 0, Offset = 0;
	int win_num =  fbi->win_id;

	struct clk *lcd_clock;
	struct fb_var_screeninfo *var= &fbi->fb.var;

	unsigned long VideoPhysicalTemp_f1 = fbi->screen_dma_f1;
	unsigned long VideoPhysicalTemp_f2 = fbi->screen_dma_f2;

	/* Initialise LCD with values from hare */

	local_irq_save(flags);

	lcd_module_init();

	local_irq_restore(flags);

	return 0;
 }

/* s3c_fb_set_lcdaddr
 *
 * initialise lcd controller address pointers
 */
 void s3c_fb_set_lcdaddr(struct s3c_fb_info *fbi)
{
	u32 start;
	unsigned long VideoPhysicalTemp_f1 = fbi->screen_dma_f1;

	start = fbi->fb.fix.line_length * fbi->fb.var.yoffset;

        /* For buffer start address */
	mach_info.vidw00add0b0 = VideoPhysicalTemp_f1+start;
        mach_info.vidw00add1b0 = mach_info.vidw00add0b0+(fbi->fb.fix.line_length*fbi->fb.var.yres);

	__raw_writel(mach_info.vidw00add0b0, S3C_VIDW00ADD0B0);
	__raw_writel(mach_info.vidw00add1b0, S3C_VIDW00ADD1B0);

}


void s3c_fb_set_fb_change(int req_fb) {

	info[req_fb].fb_change_ready = 0;

	#if defined(CONFIG_CPU_S3C2443) ||  defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416) && defined(CONFIG_FB_DOUBLE_BUFFERING)
		// Software-based trigger
		__raw_writel((1<<0), S3C_CPUTRIGCON2);
	#elif (defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)) && defined(CONFIG_FB_DOUBLE_BUFFERING)
		// Software-based trigger
		__raw_writel((3<<0), S3C_TRIGCON);
	#endif // #if defined(CONFIG_CPU_S3C2443)
}


/* s3c_fb_activate_var
 *
 * activate (set) the controller from the given framebuffer
 * information
 */
void s3c_fb_activate_var(struct s3c_fb_info *fbi,
				   struct fb_var_screeninfo *var)
{
	dprintk("%s: var->bpp   = %d\n", __FUNCTION__, var->bits_per_pixel);

	switch (var->bits_per_pixel) {
	case 8:
		mach_info.wincon0=  S3C_WINCONx_BYTSWP_ENABLE | S3C_WINCONx_BURSTLEN_4WORD |  S3C_WINCONx_BPPMODE_F_8BPP_PAL;  // 4word burst, 8bpp-palletized,
		mach_info.wincon1=  S3C_WINCONx_HAWSWP_ENABLE | S3C_WINCONx_BURSTLEN_4WORD |  S3C_WINCONx_BPPMODE_F_16BPP_565 | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_ALPHA_SEL_1;  // 4word burst, 16bpp for OSD

		#if defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
		mach_info.wincon2=  S3C_WINCONx_HAWSWP_ENABLE | S3C_WINCONx_BURSTLEN_4WORD |  S3C_WINCONx_BPPMODE_F_16BPP_565 | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_ALPHA_SEL_1;  // 4word burst, 16bpp for OSD
		mach_info.wincon3=  S3C_WINCONx_HAWSWP_ENABLE | S3C_WINCONx_BURSTLEN_4WORD |  S3C_WINCONx_BPPMODE_F_16BPP_565 | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_ALPHA_SEL_1;  // 4word burst, 16bpp for OSD
		mach_info.wincon4=  S3C_WINCONx_HAWSWP_ENABLE | S3C_WINCONx_BURSTLEN_4WORD |  S3C_WINCONx_BPPMODE_F_16BPP_565 | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_ALPHA_SEL_1;  // 4word burst, 16bpp for OSD
		#endif

		mach_info.bpp=		PIXEL_BPP8;
		mach_info.bytes_per_pixel= 1;

		#if defined(CONFIG_CPU_S3C2443) ||  defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416)
		mach_info.wpalcon=	S3C_WPALCON_W0PAL_24BIT;
		#elif defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
		mach_info.wpalcon=	S3C_WPALCON_W0PAL_16BIT;
		#endif
		break;
	case 16:
		mach_info.wincon0=  S3C_WINCONx_HAWSWP_ENABLE | S3C_WINCONx_BURSTLEN_4WORD |  S3C_WINCONx_BPPMODE_F_16BPP_565;  // 4word burst, 16bpp,
		mach_info.wincon1=  S3C_WINCONx_HAWSWP_ENABLE | S3C_WINCONx_BURSTLEN_4WORD |  S3C_WINCONx_BPPMODE_F_16BPP_565 | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_ALPHA_SEL_1;  //
		#if defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
		mach_info.wincon2=  S3C_WINCONx_HAWSWP_ENABLE | S3C_WINCONx_BURSTLEN_4WORD |  S3C_WINCONx_BPPMODE_F_16BPP_565 | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_ALPHA_SEL_1;  //
		mach_info.wincon3=  S3C_WINCONx_HAWSWP_ENABLE | S3C_WINCONx_BURSTLEN_4WORD |  S3C_WINCONx_BPPMODE_F_16BPP_565 | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_ALPHA_SEL_1;  //
		mach_info.wincon4=  S3C_WINCONx_HAWSWP_ENABLE | S3C_WINCONx_BURSTLEN_4WORD |  S3C_WINCONx_BPPMODE_F_16BPP_565 | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_ALPHA_SEL_1;  //
		#endif

		mach_info.bpp=		PIXEL_BPP16;
		mach_info.bytes_per_pixel= 2;
		break;
	case 24:
		mach_info.wincon0=  S3C_WINCONx_HAWSWP_DISABLE | S3C_WINCONx_BURSTLEN_16WORD |  S3C_WINCONx_BPPMODE_F_24BPP_888;  // 4word burst, 24bpp,,
		mach_info.wincon1= S3C_WINCONx_HAWSWP_DISABLE | S3C_WINCONx_BURSTLEN_16WORD |  S3C_WINCONx_BPPMODE_F_24BPP_888 | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_ALPHA_SEL_1;  // 4word burst, 24bpp for OSD

		#if defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
		mach_info.wincon2= S3C_WINCONx_HAWSWP_DISABLE | S3C_WINCONx_BURSTLEN_16WORD |  S3C_WINCONx_BPPMODE_F_24BPP_888 | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_ALPHA_SEL_1;  // 4word burst, 24bpp for OSD
		mach_info.wincon3= S3C_WINCONx_HAWSWP_DISABLE | S3C_WINCONx_BURSTLEN_16WORD |  S3C_WINCONx_BPPMODE_F_24BPP_888 | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_ALPHA_SEL_1;  // 4word burst, 24bpp for OSD
		mach_info.wincon4= S3C_WINCONx_HAWSWP_DISABLE | S3C_WINCONx_BURSTLEN_16WORD |  S3C_WINCONx_BPPMODE_F_24BPP_888 | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_ALPHA_SEL_1;  // 4word burst, 24bpp for OSD
		#endif

        	mach_info.bpp=		PIXEL_BPP24;
		mach_info.bytes_per_pixel= 4;
		break;

	case 32:
		mach_info.bytes_per_pixel= 4;
		break;
	}

	/* write new registers */
	__raw_writel(mach_info.wincon0, S3C_WINCON0);
	__raw_writel(mach_info.wincon1, S3C_WINCON1);

	#if defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
	__raw_writel(mach_info.wincon2, S3C_WINCON2);
	__raw_writel(mach_info.wincon3, S3C_WINCON3);
	__raw_writel(mach_info.wincon4, S3C_WINCON4);
	#endif

	__raw_writel(mach_info.wpalcon, S3C_WPALCON);

#if 0
	/* set lcd address pointers */
	s3c_fb_set_lcdaddr(fbi);
#endif

	//__raw_writel(mach_info.wincon0|S3C_WINCONx_ENWIN_F_ENABLE|S3C_WINCONx_BUFAUTOEN_ENABLE, S3C_WINCON0);   /* Double buffer auto enable bit */
	__raw_writel(mach_info.wincon0|S3C_WINCONx_ENWIN_F_ENABLE, S3C_WINCON0);
	__raw_writel(mach_info.vidcon0|S3C_VIDCON0_ENVID_ENABLE|S3C_VIDCON0_ENVID_F_ENABLE, S3C_VIDCON0);

}


int s3c_fb_init_win (struct s3c_fb_info *fbi, int Bpp, int LeftTop_x, int LeftTop_y, int Width, int Height, int OnOff)
{
	s3c_fb_win_onoff(fbi, OFF); // off
	s3c_fb_set_bpp(fbi, Bpp);
	s3c_fb_set_position_win(fbi, LeftTop_x, LeftTop_y, Width, Height);
	s3c_fb_set_size_win(fbi, Width, Height);
	s3c_fb_set_memory_size_win(fbi);
	s3c_fb_win_onoff(fbi, OnOff); // off

	return 0;
}


int s3c_fb_win_onoff(struct s3c_fb_info *fbi, int On)
{
	int win_num =  fbi->win_id;

	if(On)
		__raw_writel(__raw_readl(S3C_WINCON0+(0x04*win_num))|S3C_WINCONx_ENWIN_F_ENABLE,
					S3C_WINCON0+(0x04*win_num));   // ON
	else
		__raw_writel(__raw_readl(S3C_WINCON0+(0x04*win_num))&~(S3C_WINCONx_ENWIN_F_ENABLE),
					S3C_WINCON0+(0x04*win_num));   // OFF

	return 0;
}


int s3c_fb_set_alpha_level(struct s3c_fb_info *fbi)
{
	unsigned long alpha_val;

	int win_num =  fbi->win_id;

	if(win_num==0){
		printk("WIN0 do not support alpha blending.\n");
		return -1;
	}

	alpha_val = S3C_VIDOSDxC_ALPHA1_B(osd_alpha_level) | S3C_VIDOSDxC_ALPHA1_G(osd_alpha_level) |S3C_VIDOSDxC_ALPHA1_R(osd_alpha_level);

	#if defined(CONFIG_CPU_S3C2443) ||  defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416)
	__raw_writel(alpha_val, S3C_VIDOSD1C);

	#elif defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
	__raw_writel(alpha_val, S3C_VIDOSD0C+(0x10*win_num));
	#endif

	return 0;

}


int s3c_fb_set_alpha_mode(struct s3c_fb_info *fbi, int Alpha_mode, int Alpha_level)
{
	unsigned long alpha_val;

	int win_num =  fbi->win_id;

	if(win_num==0){
		printk("WIN0 do not support alpha blending.\n");
		return -1;
	}

	switch(Alpha_mode){
		case 0: // Plane Blending
			__raw_writel(__raw_readl(S3C_WINCON0+(0x04*win_num))|S3C_WINCONx_BLD_PIX_PLANE,
					S3C_WINCON0+(0x04*win_num));
			break;

		case 1: // Pixel Blending & chroma(color) key
			__raw_writel(__raw_readl(S3C_WINCON0+(0x04*win_num))|S3C_WINCONx_BLD_PIX_PIXEL|S3C_WINCONx_ALPHA_SEL_0,
					S3C_WINCON0+(0x04*win_num));
			break;
	}

	osd_alpha_level = Alpha_level;
	alpha_val = S3C_VIDOSDxC_ALPHA1_B(osd_alpha_level) | S3C_VIDOSDxC_ALPHA1_G(osd_alpha_level) |S3C_VIDOSDxC_ALPHA1_R(osd_alpha_level);

#if defined(CONFIG_CPU_S3C2443) ||  defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416)
	__raw_writel(alpha_val, S3C_VIDOSD1C);

#elif defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
	__raw_writel(alpha_val, S3C_VIDOSD0C+(0x10*win_num));
#endif
	return 0;

}

/*------------------------- Set up color key ------------------------------*/

int s3c_fb_color_key_alpha_onoff(struct s3c_fb_info *fbi, int On)
{
	int win_num =  fbi->win_id;
	win_num--;

	if(On)
		__raw_writel(__raw_readl(S3C_W1KEYCON0+(0x08*win_num))|S3C_WxKEYCON0_KEYBLEN_ENABLE,
					S3C_W1KEYCON0+(0x08*win_num));   // ON
	else
		__raw_writel(__raw_readl(S3C_W1KEYCON0+(0x08*win_num))&~(S3C_WxKEYCON0_KEYBLEN_ENABLE),
					S3C_W1KEYCON0+(0x08*win_num));   // OFF
	return 0;
}

int s3c_fb_color_key_onoff(struct s3c_fb_info *fbi, int On)
{
	int win_num =  fbi->win_id;
	win_num--;

	if(On)
		__raw_writel(__raw_readl(S3C_W1KEYCON0+(0x08*win_num))|S3C_WxKEYCON0_KEYEN_F_ENABLE,
					S3C_W1KEYCON0+(0x08*win_num));   // ON
	else
		__raw_writel(__raw_readl(S3C_W1KEYCON0+(0x08*win_num))&~(S3C_WxKEYCON0_KEYEN_F_ENABLE),
					S3C_W1KEYCON0+(0x08*win_num));   // OFF
	return 0;
}


int s3c_fb_setup_color_key_register(struct s3c_fb_info *fbi, s3c_color_key_info_t colkey_info)
{
	unsigned int compkey = 0;

	int win_num =  fbi->win_id;

	if(win_num==0){
		printk("WIN0 do not support color-key.\n");
		return -1;
	}

	win_num--;

	if(fbi->fb.var.bits_per_pixel == PIXEL_BPP16) {
		/* RGB 5-6-5 mode */
		compkey  = (((colkey_info.compkey_red & 0x1F) << 19) | 0x70000);
		compkey |= (((colkey_info.compkey_green & 0x3F) << 10) | 0x300);
		compkey |= (((colkey_info.compkey_blue  & 0x1F)  << 3 )| 0x7);
	}
	else if (fbi->fb.var.bits_per_pixel == PIXEL_BPP24) {
		/* currently RGB 8-8-8 mode  */
		compkey  = ((colkey_info.compkey_red & 0xFF) << 16);
		compkey |= ((colkey_info.compkey_green & 0xFF) << 8);
		compkey |= ((colkey_info.compkey_blue & 0xFF) << 0);
	}
	else
		printk("Invalid BPP has been given !\n");


	if(colkey_info.direction == COLOR_KEY_DIR_BG_DISPLAY)
		__raw_writel(S3C_WxKEYCON0_COMPKEY(compkey) | S3C_WxKEYCON0_DIRCON_MATCH_FG_IMAGE, S3C_W1KEYCON0+(0x08*win_num));

	else if (colkey_info.direction == COLOR_KEY_DIR_FG_DISPLAY)
		__raw_writel(S3C_WxKEYCON0_COMPKEY(compkey) | S3C_WxKEYCON0_DIRCON_MATCH_BG_IMAGE, S3C_W1KEYCON0+(0x08*win_num));
	else
		printk("Color key direction is not correct :: %d!!!\n", colkey_info.direction);
	return 0;

}


int s3c_fb_set_color_value(struct s3c_fb_info *fbi, s3c_color_val_info_t colval_info)
{
	unsigned int colval = 0;

	int win_num =  fbi->win_id;

	if(win_num==0){
		printk("WIN0 do not support color-key value.\n");
		return -1;
	}

	win_num--;

	if(fbi->fb.var.bits_per_pixel == PIXEL_BPP16) {
		/* RGB 5-6-5 mode */
		colval  = (((colval_info.colval_red   & 0x1F) << 19) | 0x70000);
		colval |= (((colval_info.colval_green & 0x3F) << 10) | 0x300);
		colval |= (((colval_info.colval_blue  & 0x1F)  << 3 )| 0x7);
	}
	else if (fbi->fb.var.bits_per_pixel == PIXEL_BPP24) {
		/* currently RGB 8-8-8 mode  */
		colval  = ((colval_info.colval_red  & 0xFF) << 16);
		colval |= ((colval_info.colval_green & 0xFF) << 8);
		colval |= ((colval_info.colval_blue  & 0xFF) << 0);
	}
	else
		printk("Invalid BPP has been given !\n");

	__raw_writel(S3C_WxKEYCON1_COLVAL(colval), S3C_W1KEYCON1+(0x08*win_num));

	return 0;
}


/*--------------------------------------------------------------------------*/

int s3c_fb_set_position_win(struct s3c_fb_info *fbi, int LeftTop_x, int LeftTop_y, int Width, int Height)
{
	struct fb_var_screeninfo *var= &fbi->fb.var;
	int win_num =  fbi->win_id;
#if 0
	if(win_num==0){
		printk("WIN0 do not support window position control.\n");
		return -1;
	}
#endif

#if defined(CONFIG_CPU_S3C2443) ||  defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416)
	__raw_writel(S3C_VIDOSDxA_OSD_LTX_F(LeftTop_x) | S3C_VIDOSDxA_OSD_LTY_F(LeftTop_y),
				S3C_VIDOSD0A+(0x0c*win_num));
	__raw_writel(S3C_VIDOSDxB_OSD_RBX_F(Width-1 + LeftTop_x) | S3C_VIDOSDxB_OSD_RBY_F(Height-1 + LeftTop_y),
				S3C_VIDOSD0B+(0x0c*win_num));

#elif defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
	__raw_writel(S3C_VIDOSDxA_OSD_LTX_F(LeftTop_x) | S3C_VIDOSDxA_OSD_LTY_F(LeftTop_y),
				S3C_VIDOSD0A+(0x10*win_num));
	__raw_writel(S3C_VIDOSDxB_OSD_RBX_F(Width-1 + LeftTop_x) | S3C_VIDOSDxB_OSD_RBY_F(Height-1 + LeftTop_y),
				S3C_VIDOSD0B+(0x10*win_num));
#endif

	var->xoffset=LeftTop_x;
	var->yoffset=LeftTop_y;

	return 0;
}


int s3c_fb_set_size_win(struct s3c_fb_info *fbi, int Width, int Height)
{
	struct fb_var_screeninfo *var= &fbi->fb.var;
	int win_num =  fbi->win_id;

#if 0
	if(win_num==0){
		printk("WIN0 do not support window size control.\n");
		return -1;
	}
#endif
	#if defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
	if(win_num==1)
		__raw_writel(S3C_VIDOSD0C_OSDSIZE(Width*Height),S3C_VIDOSD1D);

	else if(win_num==2)
		__raw_writel(S3C_VIDOSD0C_OSDSIZE(Width*Height),S3C_VIDOSD2D);
	#endif

	var->xres = Width;
	var->yres = Height;
	var->xres_virtual = Width;
	var->yres_virtual= Height;

	return 0;
}


int s3c_fb_set_bpp(struct s3c_fb_info *fbi, int Bpp)
{
	struct fb_var_screeninfo *var= &fbi->fb.var;
	int win_num =  fbi->win_id;

	switch(Bpp){
		case 1:
		case 2:
		case 4:
		case 8:
			// What should I do???
			break;

		case 16:
			__raw_writel(__raw_readl(S3C_WINCON0+(0x04*win_num))|S3C_WINCONx_BPPMODE_F_16BPP_565,
					S3C_WINCON0+(0x04*win_num));
			var->bits_per_pixel=16;
			break;

		case 24:
			__raw_writel(__raw_readl(S3C_WINCON0+(0x04*win_num))|S3C_WINCONx_BPPMODE_F_24BPP_888,
					S3C_WINCON0+(0x04*win_num));
			var->bits_per_pixel=24;
			break;

		case 32:
			var->bits_per_pixel=32;
			break;
	}

	return 0;
}


int s3c_fb_set_memory_size_win(struct s3c_fb_info *fbi)
{
	struct fb_var_screeninfo *var= &fbi->fb.var;
	int win_num =  fbi->win_id;

	unsigned long Offset = 0;
	unsigned long PageWidth = 0;
	unsigned long FrameBufferSize = 0;

	PageWidth = var->xres * mach_info.bytes_per_pixel;
	Offset = (var->xres_virtual - var->xres) * mach_info.bytes_per_pixel;
	#if defined(CONFIG_FB_VIRTUAL_SCREEN)
	if(win_num==0) Offset=0;
	#endif

	__raw_writel(S3C_VIDWxxADD1_VBASEL_F((unsigned long)__raw_readl(S3C_VIDW00ADD0B0+(0x08*win_num)) + (PageWidth + Offset) * (var->yres)),
				S3C_VIDW00ADD1B0+(0x08*win_num));
	if(win_num==1)
		__raw_writel(S3C_VIDWxxADD1_VBASEL_F((unsigned long)__raw_readl(S3C_VIDW00ADD0B1+(0x08*win_num)) + (PageWidth + Offset) * (var->yres)),
				S3C_VIDW00ADD1B1+(0x08*win_num));

	/* size of frame buffer */
	FrameBufferSize = S3C_VIDWxxADD2_OFFSIZE_F(Offset) | (S3C_VIDWxxADD2_PAGEWIDTH_F(PageWidth));
	#if defined(CONFIG_CPU_S3C2443) ||  defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416)
	__raw_writel(FrameBufferSize, S3C_VIDW00ADD2B0+(0x08*win_num));

	if(win_num==0)
		__raw_writel(FrameBufferSize, S3C_VIDW00ADD2B1);

	#elif defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
	__raw_writel(FrameBufferSize, S3C_VIDW00ADD2+(0x04*win_num));
	#endif
	return 0;

}


int s3c_fb_set_out_path(struct s3c_fb_info *fbi, int Path)
{
#if defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
	switch(Path){
		case 0: // RGB I/F
			__raw_writel(__raw_readl(S3C_VIDCON0) | S3C_VIDCON0_VIDOUT_RGB_IF | S3C_VIDCON0_PNRMODE_RGB_P, S3C_VIDCON0);
			break;

		case 1: // TV Encoder T/F
			__raw_writel(__raw_readl(S3C_VIDCON0) | S3C_VIDCON0_VIDOUT_TV, S3C_VIDCON0);
			break;

		case 2: // Indirect I80 I/F-0
			__raw_writel(__raw_readl(S3C_VIDCON0) | S3C_VIDCON0_VIDOUT_I80IF0, S3C_VIDCON0);
			break;

		case 3: // Indirect I80 I/F-1
			__raw_writel(__raw_readl(S3C_VIDCON0) | S3C_VIDCON0_VIDOUT_I80IF1, S3C_VIDCON0);
			break;

		case 4: // TV Encoder & RGB I/F
			__raw_writel(__raw_readl(S3C_VIDCON0) | S3C_VIDCON0_VIDOUT_TVNRGBIF, S3C_VIDCON0);
			break;

		case 6: // TV Encoder &  Indirect I80 I/F-0
			__raw_writel(__raw_readl(S3C_VIDCON0) | S3C_VIDCON0_VIDOUT_TVNI80IF0, S3C_VIDCON0);
			break;

		case 7: // TV Encoder &  Indirect I80 I/F-1
			__raw_writel(__raw_readl(S3C_VIDCON0) | S3C_VIDCON0_VIDOUT_TVNI80IF1, S3C_VIDCON0);
			break;

		default: // RGB I/F
			__raw_writel(__raw_readl(S3C_VIDCON0) | S3C_VIDCON0_VIDOUT_RGB_IF | S3C_VIDCON0_PNRMODE_RGB_P, S3C_VIDCON0);
			break;
	}
#endif

	return 0;
}

/*
 * FIXME: (yslee)
 *
 * 아래 드라이버 코드는 새로 작성되어야 한다.
 * S3C2416 LCD controller를 사용치 않고 GPIO를 직접 access하는 이 방식은 속도가 늦은
 * 문제가 있다. 이를 LCDC의 DMA controller를 이용하여 copy 되도록 하여야 한다.
 * 
 */	


extern void lcd_reset(void);
extern void lcd_backlight(int control);
extern void vd_bus_inout_set(int flag);

//#define WPU7800_EVM_LCD
//#define WPU7800_WS_LCD
#define WPU7800_ES_LCD

#ifdef WPU7800_EVM_LCD
void lcd_ili9225b_reg(int reg)
{	
	__raw_writel(0x41, S3C2410_GPCDAT);
	__raw_writel((reg<<10)|0x41, S3C2410_GPCDAT);
	__raw_writel((reg>>6), S3C2410_GPDDAT);
	__raw_writel(0x57, S3C2410_GPCDAT);
}

void lcd_ili9225b_data(int data)
{	
	__raw_writel(0x51, S3C2410_GPCDAT);
	__raw_writel((data<<10)|0x51, S3C2410_GPCDAT);
	__raw_writel((data>>6), S3C2410_GPDDAT);
	__raw_writel(0x57, S3C2410_GPCDAT);	
}

void lcd_write_pixel(int color)
{	
	__raw_writel(0x51, S3C2410_GPCDAT);
	__raw_writel((color<<10)|0x51, S3C2410_GPCDAT);
	__raw_writel((color>>6), S3C2410_GPDDAT);
	__raw_writel(0x57, S3C2410_GPCDAT);	
}

int lcd_ili9225b_read(int addr)
{	
	int data, data1;
	__raw_writel(0x41, S3C2410_GPCDAT);
	__raw_writel((addr<<10)|0x41, S3C2410_GPCDAT);
	__raw_writel((addr>>6), S3C2410_GPDDAT);
	__raw_writel(0x57, S3C2410_GPCDAT);	

	__raw_writel(0x52, S3C2410_GPCDAT);
	data = __raw_readl(S3C2410_GPCDAT);
	data1 = __raw_readl(S3C2410_GPDDAT);
	__raw_writel(0x57, S3C2410_GPCDAT);	

	data = (data>>10)|(data1<<6);

	return data;
}
#endif

#ifdef WPU7800_WS_LCD
void lcd_ili9225b_reg(int reg)
{	
	__raw_writel(0x41, S3C2410_GPCDAT);
	__raw_writel((reg<<9)|0x41, S3C2410_GPCDAT);
	__raw_writel((((reg>>7)&0x01)|(((reg>>8)&0xffff)<<2)), S3C2410_GPDDAT);
	__raw_writel(0x57, S3C2410_GPCDAT);
}

void lcd_ili9225b_data(int data)
{	
	__raw_writel(0x51, S3C2410_GPCDAT);
	__raw_writel((data<<9)|0x51, S3C2410_GPCDAT);
	__raw_writel((((data>>7)&0x01)|(((data>>8)&0xffff)<<2)), S3C2410_GPDDAT);
	__raw_writel(0x57, S3C2410_GPCDAT);	
}

void lcd_write_pixel(int color)
{	
	__raw_writel(0x51, S3C2410_GPCDAT);
	__raw_writel((color<<9)|0x51, S3C2410_GPCDAT);
	__raw_writel((((color>>7)&0x01)|(((color>>8)&0xffff)<<2)), S3C2410_GPDDAT);
	__raw_writel(0x57, S3C2410_GPCDAT);	
}

int lcd_ili9225b_read(int addr)
{	
	int datav, data, data1;
	
	__raw_writel(0x41, S3C2410_GPCDAT);
	__raw_writel((addr<<9)|0x41, S3C2410_GPCDAT);
	__raw_writel((((addr>>7)&0x01)|(((addr>>8)&0xffff)<<2)), S3C2410_GPDDAT);
	__raw_writel(0x57, S3C2410_GPCDAT);	

	vd_bus_inout_set(1);

	__raw_writel(0x52, S3C2410_GPCDAT);
	data = __raw_readl(S3C2410_GPCDAT);
	data1 = __raw_readl(S3C2410_GPDDAT);
	__raw_writel(0x57, S3C2410_GPCDAT);	

	datav  = ((data>>9) & 0x007f) |((data1&0x0001)<<7)|(((data1>>2)&0xffff)<<8); 

	vd_bus_inout_set(0);

	return datav;
}
#endif

#ifdef WPU7800_ES_LCD
void lcd_ili9225b_reg(int reg)
{	
	int mask;
	
	mask = __raw_readl(S3C2410_GPDDAT) & ~(0x07ff);

	__raw_writel(0x41, S3C2410_GPCDAT);
	__raw_writel((reg<<8)|0x41, S3C2410_GPCDAT);
	__raw_writel((reg>>8)|mask, S3C2410_GPDDAT);
	__raw_writel(0x57, S3C2410_GPCDAT);
}

void lcd_ili9225b_data(int data)
{	
	int mask;
	
	mask = __raw_readl(S3C2410_GPDDAT) & ~(0x07ff);

	__raw_writel(0x51, S3C2410_GPCDAT);
	__raw_writel((data<<8)|0x51, S3C2410_GPCDAT);
	__raw_writel((data>>8)|mask, S3C2410_GPDDAT);
	__raw_writel(0x57, S3C2410_GPCDAT);	
}

void lcd_write_pixel(int color)
{	
	int  mask;
	
	mask = __raw_readl(S3C2410_GPDDAT) & ~(0x07ff);

	__raw_writel(0x51, S3C2410_GPCDAT);
	__raw_writel((color<<8)|0x51, S3C2410_GPCDAT);
	__raw_writel((color>>8)|mask, S3C2410_GPDDAT);
	__raw_writel(0x57, S3C2410_GPCDAT);	
}

int lcd_ili9225b_read(int addr)
{	
	int mask, data, data1;
	
	mask = __raw_readl(S3C2410_GPDDAT) & ~(0x07ff);

	__raw_writel(0x41, S3C2410_GPCDAT);
	__raw_writel((addr<<8)|0x41, S3C2410_GPCDAT);
	__raw_writel((addr>>8)|mask, S3C2410_GPDDAT);
	__raw_writel(0x57, S3C2410_GPCDAT);	

	vd_bus_inout_set(1);

	__raw_writel(0x52, S3C2410_GPCDAT);
	data = __raw_readl(S3C2410_GPCDAT);
	data1 = __raw_readl(S3C2410_GPDDAT);
	__raw_writel(0x57, S3C2410_GPCDAT);	

	data = (data>>8)|(data1<<8);

	vd_bus_inout_set(0);

	return data;
}
#endif

#if 0
void lcd_test_color_pattern(void)
{
	int i, x=176, y=220;	

	//printk("+++ lcd_test_color_pattern\n");

	lcd_ili9225b_reg(0x22);
	
	for (i=0;i<(x*y);i++) {
		if (i<(x * (y/2))) {
			if (((i%x)>=0) && ((i%x)<88) ) lcd_ili9225b_data(0xffff); 	
			if (((i%x)>=88) && ((i%x)<176))  lcd_ili9225b_data(0x07e0);	 	
		} else {
			if (((i%x)>=0) && ((i%x)<88)) lcd_ili9225b_data(0); 	
			if (((i%x)>=88) && ((i%x)<176)) lcd_ili9225b_data(0x001f);
		}
	}
}
#endif

void _lcd_ili9225b_reg_write(int reg, int data)
{
	lcd_ili9225b_reg(reg);
	lcd_ili9225b_data(data);
}

void lcd_prepare_write(int x, int y)
{
	_lcd_ili9225b_reg_write (0x20, x & 0xff);
	_lcd_ili9225b_reg_write (0x21, y & 0xff);

	lcd_ili9225b_reg(0x22);
}

static void s3c_fb_change_fb(struct fb_info *info)
{
	int i = H_RESOLUTION * V_RESOLUTION;
	u16 __iomem *buf = (u16 __iomem *)info->screen_base;

	
	lcd_prepare_write(0, 0);
	while (i-- > 0) {
		lcd_write_pixel(*buf++);
	}
}

void lcd_module_init (void)
{
	//printk("+++ lcd_module_init\n");
	
      //************* Start Initial Sequence **********//
       _lcd_ili9225b_reg_write(0x0001, 0x011C);               // set SS and NL bit
       _lcd_ili9225b_reg_write(0x0002, 0x0100);               // set 1 line inversion
       _lcd_ili9225b_reg_write(0x0003, 0x1030);               // set GRAM write direction and BGR=1.
       _lcd_ili9225b_reg_write(0x0008, 0x0808);               // set BP and FP
       _lcd_ili9225b_reg_write(0x000C, 0x0000);               // RGB interface setting     R0Ch=0x0110 for RGB 18Bit and R0Ch=0111for RGB16Bit
       _lcd_ili9225b_reg_write(0x000F, 0x0801);               // Set frame rate
       _lcd_ili9225b_reg_write(0x0020, 0x0000);                  // Set GRAM Address
       _lcd_ili9225b_reg_write(0x0021, 0x0000);                  // Set GRAM Address
    //*************Power On sequence ****************//
       mdelay(50);        // Delay 50ms
       _lcd_ili9225b_reg_write(0x0010, 0x0A00);                   // Set SAP,DSTB,STB
                                                               // Set APON,PON,AON,VCI1EN,VC
       _lcd_ili9225b_reg_write(0x0011, 0x1038);
       mdelay(50);        // Delay 50ms
       _lcd_ili9225b_reg_write(0x0012, 0x1121);                  // Internal reference voltage= Vci;
       _lcd_ili9225b_reg_write(0x0013, 0x0066);                  // Set GVDD
       _lcd_ili9225b_reg_write(0x0014, 0x5F60);                  // Set VCOMH/VCOML voltage
//------------------------ Set GRAM area --------------------------------//
       _lcd_ili9225b_reg_write (0x30, 0x0000);
       _lcd_ili9225b_reg_write (0x31, 0x00DB);
       _lcd_ili9225b_reg_write (0x32, 0x0000);
       _lcd_ili9225b_reg_write (0x33, 0x0000);
       _lcd_ili9225b_reg_write (0x34, 0x00DB);
       _lcd_ili9225b_reg_write (0x35, 0x0000);
       _lcd_ili9225b_reg_write (0x36, 0x00AF);
       _lcd_ili9225b_reg_write (0x37, 0x0000);
       _lcd_ili9225b_reg_write (0x38, 0x00DB);
       _lcd_ili9225b_reg_write (0x39, 0x0000);
// ----------- Adjust the Gamma       Curve ----------//
       _lcd_ili9225b_reg_write(0x0050, 0x0400);
       _lcd_ili9225b_reg_write(0x0051, 0x060B);
       _lcd_ili9225b_reg_write(0x0052, 0x0C0A);
       _lcd_ili9225b_reg_write(0x0053, 0x0105);
       _lcd_ili9225b_reg_write(0x0054, 0x0A0C);
       _lcd_ili9225b_reg_write(0x0055, 0x0B06);
       _lcd_ili9225b_reg_write(0x0056, 0x0004);
       _lcd_ili9225b_reg_write(0x0057, 0x0501);
       _lcd_ili9225b_reg_write(0x0058, 0x0E00);
       _lcd_ili9225b_reg_write(0x0059, 0x000E);
       mdelay(50);        // Delay 50ms
       _lcd_ili9225b_reg_write(0x0007, 0x1017);
       lcd_ili9225b_reg(0x22);	   

	/* set window size */
	int xs = 0;
	int xe = 176 -1;
	int ys = 0;
	int ye = 220 -1;
	_lcd_ili9225b_reg_write (0x36, xs+xe);
	_lcd_ili9225b_reg_write (0x37, xs);
        _lcd_ili9225b_reg_write (0x38, ys+ye);
        _lcd_ili9225b_reg_write (0x39, ys);

	int i = H_RESOLUTION * V_RESOLUTION;
	
	lcd_prepare_write(0, 0);
	while (i-- > 0) {
		lcd_write_pixel(0);
	}
}

void SetLcdPort(void)
{
	lcd_reset();
	lcd_backlight(5);
}

void Init_LDI(void)
{
	printk(KERN_INFO "LCD TYPE :: S3C_ILI9225QC LCD will be initialized\n");

	SetLcdPort();
}

EXPORT_SYMBOL(lcd_prepare_write);
EXPORT_SYMBOL(lcd_write_pixel);
EXPORT_SYMBOL(lcd_module_init);
