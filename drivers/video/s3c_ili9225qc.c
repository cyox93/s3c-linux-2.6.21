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
#include <linux/platform_device.h>
#include <linux/interrupt.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/dma.h>

#include <asm/irq.h>

#include <asm/mach/map.h>
#include <asm/arch/dma.h>
#include <asm/arch/regs-lcd.h>
#include <asm/arch/regs-gpio.h>
#include <asm/arch/regs-timer.h>

#if defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416)
#include <asm/arch/regs-s3c2450-clock.h>
#elif defined(CONFIG_CPU_S3C6400)
#include <asm/arch/regs-s3c6400-clock.h>
#elif defined(CONFIG_CPU_S3C6410)
#include <asm/arch/regs-s3c6410-clock.h>
#endif

#include <asm/plat-s3c24xx/s3c2416.h>
#include "unidata-logo.h"
#include "mylg070-logo.h"
#include "skbb-logo.h"
#include "ktqook-logo.h"

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

int _h_resolution_osd = H_RESOLUTION_OSD;
int _v_resolution_osd = V_RESOLUTION_OSD;

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

#define MAX_DISPLAY_BRIGHTNESS		100
#define DEF_DISPLAY_BRIGHTNESS		80

int _h_resolution = H_RESOLUTION;
int _v_resolution = V_RESOLUTION;

int _h_resolution_virtual = H_RESOLUTION_VIRTUAL;
int _v_resolution_virtual = V_RESOLUTION_VIRTUAL;

int display_brightness = DEF_DISPLAY_BRIGHTNESS;
int backup_brightness = DEF_DISPLAY_BRIGHTNESS;
int backlight_power_state = 1;
int lcd_power_state = 1;

void set_brightness(int);
void backlight_power(int);
static void lcd_ili9225b_power(int set);
static void _lcd_panel_init(void);

struct s3c_fb_mach_info mach_info = {

#if defined(CONFIG_CPU_S3C2443) ||  defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416)
	.vidcon0= S3C_VIDCON0_VIDOUT_I80IF0 | S3C_VIDCON0_L0_DATA16_MAIN_16_MODE | S3C_VIDCON0_CLKDIR_DIVIDED | S3C_VIDCON0_VCLKEN_ENABLE |S3C_VIDCON0_CLKSEL_F_HCLK,
	.vidcon1= S3C_VIDCON1_IHSYNC_INVERT | S3C_VIDCON1_IVSYNC_INVERT | S3C_VIDCON1_IVDEN_INVERT,
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
	.wincon0=  S3C_WINCONx_HAWSWP_ENABLE | S3C_WINCONx_BURSTLEN_16WORD |  S3C_WINCONx_BPPMODE_F_16BPP_565,  // 4word burst, 16bpp,
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
	.vidintcon= 0,
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

	.backlight_min = 0,
	.backlight_max = 100,
	.backlight_default = 80,

	.backlight_power = backlight_power,
	.set_brightness = set_brightness,

	.lcd_power = lcd_ili9225b_power,
};

static void
_backlight_set_pwm(unsigned long tcnt, unsigned long tcmp)
{
	unsigned long tcon;

	tcon = __raw_readl(S3C2410_TCON);

	if (tcmp) {
		s3c2410_gpio_cfgpin(S3C2410_GPB2, S3C2410_GPB2_TOUT2);
		s3c2410_gpio_setpin(S3C2410_GPB2, 0);

		__raw_writel(tcnt, S3C2410_TCNTB(2));
		__raw_writel(tcmp, S3C2410_TCMPB(2));

		if (!(tcon & S3C2410_TCON_T2START)) {
			tcon |= S3C2410_TCON_T2MANUALUPD;
			__raw_writel(tcon, S3C2410_TCON);

			tcon &= ~(S3C2410_TCON_T2MANUALUPD);
			tcon |= (S3C2410_TCON_T2START | S3C2410_TCON_T2RELOAD) ;
			__raw_writel(tcon, S3C2410_TCON);
		}
	} else {
		tcon &= ~(S3C2410_TCON_T2MANUALUPD | S3C2410_TCON_T2START | S3C2410_TCON_T2RELOAD);
		__raw_writel(tcon, S3C2410_TCON);

		s3c2410_gpio_setpin(S3C2410_GPB2, 0);
		s3c2410_gpio_cfgpin(S3C2410_GPB2, S3C2410_GPB2_OUTP);
	}
}

void set_brightness(int val)
{

	unsigned long tcnt=0x5000;
	unsigned long tcmp=1;

	if(val < 0) val=0;
	if(val > MAX_DISPLAY_BRIGHTNESS) val=MAX_DISPLAY_BRIGHTNESS;

	display_brightness = val;
	if (!backlight_power_state) {
		backup_brightness = val;
		return ;
	}

	tcmp = tcnt * val / 100;
	if (tcnt == tcmp) tcmp--;

//	s3c2450_timer_setup (channel, usec, tcnt, tcmp);
	_backlight_set_pwm(tcnt, tcmp);
}

void backlight_power(int val)
{
	if (val) {
		backlight_power_state = 1;
		set_brightness(backup_brightness);
	} else {
		set_brightness(0);
		backlight_power_state = 0;
	}
}

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
	if(vs_info_from_app.width != _h_resolution || vs_info_from_app.height != _v_resolution ){
		return 1;
	}
	if(!(vs_info_from_app.bpp==8 ||vs_info_from_app.bpp==16 ||vs_info_from_app.bpp==24 || vs_info_from_app.bpp==32) ){
		return 1;
	}
	if(vs_info_from_app.offset<0){
		return 1;
	}
	if(vs_info_from_app.v_width != _h_resolution_virtual  || vs_info_from_app.v_height != _v_resolution_virtual){
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
				if(var->xoffset < (_h_resolution - var->xres)) var->xoffset ++;
				s3c_fb_set_position_win(fbi, var->xoffset, var->yoffset, var->xres, var->yres);
				break;

		case SET_OSD_MOVE_UP:
				if(var->yoffset>0) var->yoffset--;
				s3c_fb_set_position_win(fbi, var->xoffset, var->yoffset, var->xres, var->yres);
				break;

		case SET_OSD_MOVE_DOWN:
				if(var->yoffset < (_v_resolution - var->yres)) var->yoffset ++;
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
				set_brightness(brightness);
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

	if(win_num==0){ 	
		mach_info.vidcon0 = mach_info.vidcon0 & ~(S3C_VIDCON0_ENVID_ENABLE | S3C_VIDCON0_ENVID_F_ENABLE);
		__raw_writel(mach_info.vidcon0, S3C_VIDCON0);

		mach_info.vidcon0 |=  S3C_VIDCON0_CLKVAL_F(3);

		__raw_writel(0x7, S3C_SYSIFCON0);

		lcd_module_init();
 	}

        /* For buffer start address */
	__raw_writel(VideoPhysicalTemp_f1, S3C_VIDW00ADD0B0+(0x08*win_num));
	#if defined(CONFIG_CPU_S3C2443)||defined(CONFIG_CPU_S3C2450)
	if(win_num==0) __raw_writel(VideoPhysicalTemp_f2, S3C_VIDW00ADD0B1);	
	#elif defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
	if(win_num<2) __raw_writel(VideoPhysicalTemp_f2, S3C_VIDW00ADD0B1+(0x08*win_num));	
	#endif
	#if defined(CONFIG_FB_VIRTUAL_SCREEN)
	if(win_num==0){
		mach_info.vidw00add0b0=VideoPhysicalTemp_f1;
		mach_info.vidw00add0b1=VideoPhysicalTemp_f2;
	}
	#endif
	
	PageWidth = var->xres * mach_info.bytes_per_pixel;
	Offset = (var->xres_virtual - var->xres) * mach_info.bytes_per_pixel;
	#if defined(CONFIG_FB_VIRTUAL_SCREEN)
	if(win_num==0) Offset=0;
	#endif
	
	/* End address */
	__raw_writel(S3C_VIDWxxADD1_VBASEL_F((unsigned long) VideoPhysicalTemp_f1 + (PageWidth + Offset) * (var->yres)), 
				S3C_VIDW00ADD1B0+(0x08*win_num));
	#if defined(CONFIG_CPU_S3C2443)||defined(CONFIG_CPU_S3C2450)
	if(win_num==0) 
		__raw_writel(S3C_VIDWxxADD1_VBASEL_F((unsigned long) VideoPhysicalTemp_f2 + (PageWidth + Offset) * (var->yres)), 
				S3C_VIDW00ADD1B1);	
	#elif defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
	if(win_num<2) 
		__raw_writel(S3C_VIDWxxADD1_VBASEL_F((unsigned long) VideoPhysicalTemp_f2 + (PageWidth + Offset) * (var->yres)), 
				S3C_VIDW00ADD1B1+(0x08*win_num));	
	#endif
	#if defined(CONFIG_FB_VIRTUAL_SCREEN)
	if(win_num==0){
		mach_info.vidw00add1b0=S3C_VIDWxxADD1_VBASEL_F((unsigned long) VideoPhysicalTemp_f1 + (PageWidth + Offset) * (var->yres));
		mach_info.vidw00add1b1=S3C_VIDWxxADD1_VBASEL_F((unsigned long) VideoPhysicalTemp_f2 + (PageWidth + Offset) * (var->yres));
	}
	#endif
	
	/* size of buffer */
	#if defined(CONFIG_CPU_S3C2443)||defined(CONFIG_CPU_S3C2450)
	__raw_writel(S3C_VIDWxxADD2_OFFSIZE_F(Offset) | (S3C_VIDWxxADD2_PAGEWIDTH_F(PageWidth)), S3C_VIDW00ADD2B0+(0x08*win_num));
	if(win_num==0) __raw_writel(S3C_VIDWxxADD2_OFFSIZE_F(Offset) | (S3C_VIDWxxADD2_PAGEWIDTH_F(PageWidth)), S3C_VIDW00ADD2B1);
	#elif defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
	__raw_writel(S3C_VIDWxxADD2_OFFSIZE_F(Offset) | (S3C_VIDWxxADD2_PAGEWIDTH_F(PageWidth)), S3C_VIDW00ADD2+(0x04*win_num));
	#endif
	
	switch(win_num){
	case 0:
		__raw_writel(mach_info.wincon0, S3C_WINCON0);
		__raw_writel(mach_info.vidcon0, S3C_VIDCON0);
		__raw_writel(mach_info.vidcon1, S3C_VIDCON1);
		__raw_writel(mach_info.vidtcon0, S3C_VIDTCON0);
		__raw_writel(mach_info.vidtcon1, S3C_VIDTCON1);
		__raw_writel(mach_info.vidtcon2, S3C_VIDTCON2);

		#if defined(CONFIG_CPU_S3C2443) || defined(CONFIG_CPU_S3C2450)
		__raw_writel(mach_info.vidintcon, S3C_VIDINTCON);
		#elif defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
		__raw_writel(mach_info.dithmode, S3C_DITHMODE);
		__raw_writel(mach_info.vidintcon0, S3C_VIDINTCON0);
		__raw_writel(mach_info.vidintcon1, S3C_VIDINTCON1);
		#endif
		__raw_writel(mach_info.vidosd0a, S3C_VIDOSD0A);
		__raw_writel(mach_info.vidosd0b, S3C_VIDOSD0B);
		#if defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
		__raw_writel(mach_info.vidosd0c, S3C_VIDOSD0C);
		#endif
		__raw_writel(mach_info.wpalcon, S3C_WPALCON);

		s3c_fb_win_onoff(fbi, ON);
		break;

	case 1:
		__raw_writel(mach_info.wincon1, S3C_WINCON1);
		__raw_writel(mach_info.vidosd1a, S3C_VIDOSD1A);
		__raw_writel(mach_info.vidosd1b, S3C_VIDOSD1B);
		__raw_writel(mach_info.vidosd1c, S3C_VIDOSD1C);
		#if defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
		__raw_writel(mach_info.vidosd1d, S3C_VIDOSD1D);
		#endif
		__raw_writel(mach_info.wpalcon, S3C_WPALCON);
		s3c_fb_win_onoff(fbi, OFF);
		break;

	#if defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
	case 2:
		__raw_writel(mach_info.wincon2, S3C_WINCON2);
		__raw_writel(mach_info.vidosd2a, S3C_VIDOSD2A);
		__raw_writel(mach_info.vidosd2b, S3C_VIDOSD2B);
		__raw_writel(mach_info.vidosd2c, S3C_VIDOSD2C);
		__raw_writel(mach_info.vidosd2d, S3C_VIDOSD2D);
		__raw_writel(mach_info.wpalcon, S3C_WPALCON);
		s3c_fb_win_onoff(fbi, OFF);
		break; 

	case 3:
		__raw_writel(mach_info.wincon3, S3C_WINCON3);
		__raw_writel(mach_info.vidosd3a, S3C_VIDOSD3A);
		__raw_writel(mach_info.vidosd3b, S3C_VIDOSD3B);
		__raw_writel(mach_info.vidosd3c, S3C_VIDOSD3C);
		__raw_writel(mach_info.wpalcon, S3C_WPALCON);
		s3c_fb_win_onoff(fbi, OFF);
		break;

	case 4:
		__raw_writel(mach_info.wincon4, S3C_WINCON4);
		__raw_writel(mach_info.vidosd4a, S3C_VIDOSD4A);
		__raw_writel(mach_info.vidosd4b, S3C_VIDOSD4B);
		__raw_writel(mach_info.vidosd4c, S3C_VIDOSD4C);
		__raw_writel(mach_info.wpalcon, S3C_WPALCON);
		s3c_fb_win_onoff(fbi, OFF);
		break;
	#endif
	}

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


/*_____________________ Type definitions ____________________________________*/

#define _lcd_ili9225b_reg(reg)			_lcd_handle.set_reg(reg)
#define _lcd_ili9225b_reg_read(reg)		_lcd_handle.reg_read(reg)
#define _lcd_ili9225b_reg_write(reg, data)	_lcd_handle.reg_write(reg, data)
#define lcd_prepare_write(x, y)			_lcd_handle.prepare_write(x, y)
#define lcd_write_pixel(color)			_lcd_handle.write_pixel(color)
#define lcd_set_command_mode(set)		_lcd_handle.command_mode(set)
#define lcd_trigger(info)			_lcd_handle.trigger(info)

typedef struct __QLcdHandle {
	int is_init;

	void (*trigger)(struct fb_info *info);

	void (*set_reg)(int reg);
	int (*reg_read)(int reg);
	void (*reg_write)(int reg, int data);
	void (*prepare_write)(int x, int y);
	void (*write_pixel)(int color);
	void (*command_mode)(int set);
} _QLcdHandle;

/*_____________________ Imported Variables __________________________________*/
extern void lcd_reset(void);
extern void lcd_gpio_init(void);
extern int q_lcd_panel_id(void);
extern void q_camera_backend_reset(int reset);

/*_____________________ Local Declarations __________________________________*/
static _QLcdHandle _lcd_handle;

/* lcd handle for s3c2416 lcd module in i80 mode */
static inline void
_lcd_i80_cmd(int control)
{
	__raw_writel(control | S3C_I80SIFCCON0_COM_ENABLE, S3C_SIFCCON0);
}

static void
_lcd_s3c_i80_reg(int reg)
{
	// set RS, CS0, WR
	_lcd_i80_cmd(S3C_I80SIFCCON0_CS0_CON_ENABLE
			| S3C_I80SIFCCON0_RS_CON_LOW
			| S3C_I80SIFCCON0_WR_CON_ENABLE);

	__raw_writel(reg, S3C_SIFCCON1);

	_lcd_i80_cmd(S3C_I80SIFCCON0_RS_CON_HIGH);
}

static int
_lcd_s3c_i80_reg_read(int reg)
{
	int data;

	// set RS, CS0, WR
	_lcd_i80_cmd(S3C_I80SIFCCON0_CS0_CON_ENABLE
			| S3C_I80SIFCCON0_RS_CON_LOW
			| S3C_I80SIFCCON0_WR_CON_ENABLE);

	__raw_writel(reg, S3C_SIFCCON1);

	// set CS0
	_lcd_i80_cmd(S3C_I80SIFCCON0_CS0_CON_ENABLE
			| S3C_I80SIFCCON0_RS_CON_HIGH);

	// set CS0, OE
	_lcd_i80_cmd(S3C_I80SIFCCON0_CS0_CON_ENABLE
			| S3C_I80SIFCCON0_RS_CON_HIGH
			| S3C_I80SIFCCON0_OE_CON_ENABLE);

	data = __raw_readl(S3C_SIFCCON2);

	_lcd_i80_cmd(S3C_I80SIFCCON0_RS_CON_HIGH);

	return data & 0x3ffff;
}

static void
_lcd_s3c_i80_reg_write(int reg, int data)
{
	// set RS, CS0, WR
	_lcd_i80_cmd(S3C_I80SIFCCON0_CS0_CON_ENABLE
			| S3C_I80SIFCCON0_RS_CON_LOW
			| S3C_I80SIFCCON0_WR_CON_ENABLE);

	__raw_writel(reg, S3C_SIFCCON1);

	// set CS0
	_lcd_i80_cmd(S3C_I80SIFCCON0_CS0_CON_ENABLE
			| S3C_I80SIFCCON0_RS_CON_HIGH);

	// set CS0, WR
	_lcd_i80_cmd(S3C_I80SIFCCON0_CS0_CON_ENABLE
			| S3C_I80SIFCCON0_RS_CON_HIGH
			| S3C_I80SIFCCON0_WR_CON_ENABLE);

	__raw_writel(data, S3C_SIFCCON1);

	_lcd_i80_cmd(S3C_I80SIFCCON0_RS_CON_HIGH);
}

static void
_lcd_s3c_i80_prepare_write(int x, int y)
{
	_lcd_ili9225b_reg_write (0x20, x & 0xff);
	_lcd_ili9225b_reg_write (0x21, y & 0xff);

	_lcd_ili9225b_reg (0x22);
}

static void
_lcd_s3c_i80_write_pixel(int color)
{	
	// set CS0, WR
	_lcd_i80_cmd(S3C_I80SIFCCON0_CS0_CON_ENABLE
			| S3C_I80SIFCCON0_RS_CON_HIGH
			| S3C_I80SIFCCON0_WR_CON_ENABLE);

	__raw_writel(color, S3C_SIFCCON1);

	_lcd_i80_cmd(S3C_I80SIFCCON0_RS_CON_HIGH);
}

static void
_lcd_s3c_i80_set_command_mode (int set)
{
	if (set)
		__raw_writel(S3C_I80SIFCCON0_COM_ENABLE
				| S3C_I80SIFCCON0_RS_CON_HIGH,
				S3C_SIFCCON0);
	else {
		lcd_prepare_write(0, 0);
		__raw_writel(S3C_I80SIFCCON0_COM_DISABLE, S3C_SIFCCON0);
	}
}

static void
_lcd_s3c_i80_trigger(struct fb_info *info)
{
	__raw_writel((1<<0), S3C_CPUTRIGCON2);
}

static void
_lcd_s3c_i80_init(int init)
{
	if (init)
		lcd_reset();

	_lcd_handle.set_reg		= _lcd_s3c_i80_reg;
	_lcd_handle.reg_read		= _lcd_s3c_i80_reg_read;
	_lcd_handle.reg_write		= _lcd_s3c_i80_reg_write;
	_lcd_handle.prepare_write	= _lcd_s3c_i80_prepare_write;
	_lcd_handle.write_pixel		= _lcd_s3c_i80_write_pixel;
	_lcd_handle.command_mode	= _lcd_s3c_i80_set_command_mode;
	_lcd_handle.trigger		= _lcd_s3c_i80_trigger;

	_lcd_handle.is_init = true;
}

/* lcd handle for vc0528 camera backend ic */
#define _VC0528_PA_ADDRESS		(0x20000000)

typedef struct __QLcdDma {
	void __iomem *reg_base;

	int is_requested;
	int on_request;
} _QLcdDma;

static _QLcdDma _lcd_dma;

static void __iomem *_index_addr;
static void __iomem *_lcd_data_addr;
static void __iomem *_be_data_addr;

static void
_lcd_vc0528_reg(int reg)
{
	__raw_writew((unsigned short)reg, _index_addr);
}

static int
_lcd_vc0528_reg_read(int reg)
{
	int data;

	__raw_writew((unsigned short)reg, _index_addr);
	data = __raw_readw(_lcd_data_addr);

	return data & 0x3ffff;
}

static void
_lcd_vc0528_reg_write(int reg, int data)
{
	__raw_writew((unsigned short)reg, _index_addr);
	__raw_writew((unsigned short)data, _lcd_data_addr);
}

static void
_lcd_vc0528_prepare_write(int x, int y)
{
	_lcd_vc0528_reg_write(0x20, x & 0xff);
	_lcd_vc0528_reg_write(0x21, y & 0xff);

	_lcd_vc0528_reg(0x22);
}

static void
_lcd_vc0528_write_pixel(int color)
{
	__raw_writew(color, _lcd_data_addr);
}

static void
_lcd_vc0528_set_command_mode(int set)
{

}

#define VM0528_DMA_DCON	(S3C2410_DCON_SYNC_HCLK|S3C2416_DCON_WHOLE_SERV)

static struct s3c2410_dma_client vm0528_dma_client = {
	.name		= "vc0528-lcd-dma",
};

static void *dma_vm0528_done;

static void vm0528_dma_finish(struct s3c2410_dma_chan *dma_ch, void *buf_id,
	int size, enum s3c2410_dma_buffresult result){
	complete(dma_vm0528_done);
}

int s3c_fb_suspend_lcd(struct s3c_fb_info *info)
{
	if (!q_hw_ver(KTQOOK))
		return 0;

	s3c2410_dma_free(DMACH_XD0, &vm0528_dma_client);
}

int s3c_fb_resume_lcd(struct s3c_fb_info *info)
{
	if (!q_hw_ver(KTQOOK))
		return 0;

	if (s3c2410_dma_request(DMACH_XD0, &vm0528_dma_client, NULL)) {
		printk(KERN_WARNING "Unable to get DMA channel.\n");
		return;
	}
	s3c2410_dma_set_buffdone_fn(DMACH_XD0, vm0528_dma_finish);
	s3c2410_dma_devconfig(DMACH_XD0, S3C2410_DMASRC_MEM, 1, (u_long) _VC0528_PA_ADDRESS+4);
	s3c2410_dma_config(DMACH_XD0, 2, VM0528_DMA_DCON);
	s3c2410_dma_setflags(DMACH_XD0, S3C2410_DMAF_AUTOSTART);
}

int _lcd_vc0528_trigger_lock = 0x1;
_lcd_vc0528_trigger(struct fb_info *info)
{
	int i, size;
	struct s3c_fb_info *fbi = container_of(info, struct s3c_fb_info, fb);
	unsigned short *fb_ptr = (unsigned short *)fbi->map_cpu_f1;
	DECLARE_COMPLETION_ONSTACK(complete);
	dma_vm0528_done = &complete;
	
	s3c2410_dma_enqueue(DMACH_XD0, NULL, (dma_addr_t) fbi->map_dma_f1, fbi->map_size_f1);
	wait_for_completion(&complete);
}

static void
_lcd_vc0528_init(int init)
{
	int ret = -1;
	uint val;

	if (_lcd_handle.is_init) return ;

	_index_addr    = ioremap(_VC0528_PA_ADDRESS, 0x100);
	_be_data_addr  = _index_addr + 0x08;
	_lcd_data_addr = _index_addr + 0x04;

	if (init) {
		q_camera_backend_reset(1);
		msleep(50);
		q_camera_backend_reset(0);
		msleep(10);

		// set bus witdh to 16bit
		__raw_writeb(0xb4, _index_addr);
		__raw_writeb(0x18, _be_data_addr);
		__raw_writeb(0xb2, _index_addr);
		__raw_writeb(0x8c, _be_data_addr);
		__raw_writeb(0xb0, _index_addr);
		__raw_writeb(0x1, _be_data_addr);

		// set through mode
		__raw_writew(0x1890, _index_addr);
		__raw_writew(0x1, _be_data_addr);

		lcd_reset();
	}

	_lcd_handle.set_reg		= _lcd_vc0528_reg;
	_lcd_handle.reg_read		= _lcd_vc0528_reg_read;
	_lcd_handle.reg_write		= _lcd_vc0528_reg_write;
	_lcd_handle.prepare_write	= _lcd_vc0528_prepare_write;
	_lcd_handle.write_pixel		= _lcd_vc0528_write_pixel;
	_lcd_handle.command_mode	= _lcd_vc0528_set_command_mode;
	_lcd_handle.trigger		= _lcd_vc0528_trigger;

	s3c_fb_resume_lcd(NULL);
}

static void
lcd_ili9225b_power(int set)
{
	int id = q_lcd_panel_id();

	if (!_lcd_handle.is_init) return ;
	if (lcd_power_state == set) return ;

	if (!set) {
		lcd_power_state = set;
		lcd_set_command_mode(1);

		if (id == _LCD_PANEL_HSD24) {
			_lcd_ili9225b_reg_write(0x0007, 0x0131); // Set D1=0, D0=1
			mdelay(10);
			_lcd_ili9225b_reg_write(0x0007, 0x0130); // Set D1=0, D0=0
			mdelay(10);
			_lcd_ili9225b_reg_write(0x0007, 0x0000); // display OFF
			//************* Power OFF sequence **************//
			_lcd_ili9225b_reg_write(0x0010, 0x0080); // SAP, BT[3:0], APE, AP, DSTB, SLP
			_lcd_ili9225b_reg_write(0x0011, 0x0000); // DC1[2:0], DC0[2:0], VC[2:0]
			_lcd_ili9225b_reg_write(0x0012, 0x0000); // VREG1OUT voltage
			_lcd_ili9225b_reg_write(0x0013, 0x0000); // VDV[4:0] for VCOM amplitude
			mdelay(20); // Dis-charge capacitor power voltage
			_lcd_ili9225b_reg_write(0x0010, 0x0082); // SAP, BT[3:0], APE, AP, DSTB, SLP
		} else {
			_lcd_ili9225b_reg_write(0x0007,0x0000);
			mdelay(50);
			_lcd_ili9225b_reg_write(0x0011,0x0007);
			mdelay(50);
			_lcd_ili9225b_reg_write(0x0010,0x0A02);
		}

		lcd_set_command_mode(0);
	} else {
		lcd_set_command_mode(1);

		if (id == _LCD_PANEL_HSD24) {
			//*************Power On sequence ******************//
			_lcd_ili9225b_reg_write(0x0010, 0x0000); // SAP, BT[3:0], AP, DSTB, SLP, STB
			_lcd_ili9225b_reg_write(0x0011, 0x0007); // DC1[2:0], DC0[2:0], VC[2:0]
			_lcd_ili9225b_reg_write(0x0012, 0x0000); // VREG1OUT voltage
			_lcd_ili9225b_reg_write(0x0013, 0x0000); // VDV[4:0] for VCOM amplitude
			_lcd_ili9225b_reg_write(0x0007, 0x0001);
			mdelay(20); // Dis-charge capacitor power voltage
			_lcd_ili9225b_reg_write(0x0010, 0x1690); // SAP, BT[3:0], AP, DSTB, SLP, STB
			_lcd_ili9225b_reg_write(0x0011, 0x0227); // Set DC1[2:0], DC0[2:0], VC[2:0]
			mdelay(10); // Delay 50ms
			_lcd_ili9225b_reg_write(0x0012, 0x000D); // External reference voltage= Vci;
			mdelay(10); // Delay 50ms
			_lcd_ili9225b_reg_write(0x0013, 0x1200); // VDV[4:0] for VCOM amplitude
			_lcd_ili9225b_reg_write(0x0029, 0x0007); // VCM[5:0] for VCOMH
			mdelay(10); // Delay 50ms
			_lcd_ili9225b_reg_write(0x0007, 0x0133); // 262K color and display ON
		} else {
			_lcd_ili9225b_reg_write(0x0010,0x0A00);
			lcd_set_command_mode(0);

			lcd_reset();
			lcd_set_command_mode(1);
			_lcd_panel_init();
			lcd_prepare_write(0, 0);
		}

		lcd_set_command_mode(0);
		mdelay(2);
		lcd_power_state = set;

		lcd_trigger(info);
	}
}

static void s3c_fb_change_fb(struct fb_info *info)
{
	if ((lcd_power_state)&&(_lcd_vc0528_trigger_lock)){
		lcd_trigger(info);
	}
}

#define _LCD_PANEL_BYD		0
#define _LCD_PANEL_TRULY	1
#define _LCD_PANEL_TCL		2
#define _LCD_PANEL_HSD24	7 /* K5 320x240 LCD Panel */

static const char *_lcd_panel_str[8] = {
	"BYD", "TRULY", "TCL", NULL,
	NULL, NULL, NULL, "HSD2.4"
};

static void
_lcd_panel_set_display(int on)
{
	if (on)
		_lcd_ili9225b_reg_write(0x0007,0x1017);
	else
		_lcd_ili9225b_reg_write(0x0007,0x0000);
}

static void
_lcd_panel_init_tcl(void)
{
	_lcd_ili9225b_reg_write(0x0000,0x0001);
	_lcd_ili9225b_reg_write(0x0001,0x011C);
	_lcd_ili9225b_reg_write(0x0002,0x0100);	//set 1 line inversion 
	_lcd_ili9225b_reg_write(0x0003,0x1030);

	_lcd_ili9225b_reg_write(0x0007,0x0000);
	_lcd_ili9225b_reg_write(0x0008,0x0808);
	_lcd_ili9225b_reg_write(0x000B,0x0100);
	_lcd_ili9225b_reg_write(0x000C,0x0000);
	_lcd_ili9225b_reg_write(0x000F,0x0d01);	//Oscillator Control
       	
	//power on sequence//
	_lcd_ili9225b_reg_write(0x0010,0x0000);
	mdelay(10);
	_lcd_ili9225b_reg_write(0x0011,0x0008);
	_lcd_ili9225b_reg_write(0x0012,0x6332);
	mdelay(40);
	_lcd_ili9225b_reg_write(0x0013,0x0000);
	mdelay(40);

	_lcd_ili9225b_reg_write(0x0010,0x0A00);

	mdelay(10);
	_lcd_ili9225b_reg_write(0x0011,0x1038);
	_lcd_ili9225b_reg_write(0x0012,0x6332);
	mdelay(40);
	_lcd_ili9225b_reg_write(0x0013,0x0068);
	mdelay(40);
	_lcd_ili9225b_reg_write(0x0014,0x4a60);

	// Gamma Control Setting//
	_lcd_ili9225b_reg_write(0x0050,0x0400);
	_lcd_ili9225b_reg_write(0x0051,0x060B);
	_lcd_ili9225b_reg_write(0x0052,0x0C0A);
	_lcd_ili9225b_reg_write(0x0053,0x0105);
	_lcd_ili9225b_reg_write(0x0054,0x0A0C);
	_lcd_ili9225b_reg_write(0x0055,0x0B06);
	_lcd_ili9225b_reg_write(0x0056,0x0004);
	_lcd_ili9225b_reg_write(0x0057,0x0501);
	_lcd_ili9225b_reg_write(0x0058,0x0E00);
	_lcd_ili9225b_reg_write(0x0059,0x000E);

	//set GRAM area //	
	_lcd_ili9225b_reg_write(0x0020,0x0000);
	_lcd_ili9225b_reg_write(0x0021,0x0000);
	_lcd_ili9225b_reg_write(0x0030,0x0000);
	_lcd_ili9225b_reg_write(0x0031,0x00DB);
	_lcd_ili9225b_reg_write(0x0032,0x0000);
	_lcd_ili9225b_reg_write(0x0033,0x0000);
	_lcd_ili9225b_reg_write(0x0034,0x00DB);
	_lcd_ili9225b_reg_write(0x0035,0x0000);

	_lcd_ili9225b_reg_write(0x0036,0x00AF);
	_lcd_ili9225b_reg_write(0x0037,0x0000);
	_lcd_ili9225b_reg_write(0x0038,0x00DB);
	_lcd_ili9225b_reg_write(0x0039,0x0000);
	mdelay(50);
	_lcd_ili9225b_reg_write(0x0007,0x1017); 
}

static void
_lcd_panel_init_truly(void)
{
	_lcd_ili9225b_reg_write(0x0010,0x0000); 
	_lcd_ili9225b_reg_write(0x0011,0x0000); 
	_lcd_ili9225b_reg_write(0x0012,0x0000); 
	_lcd_ili9225b_reg_write(0x0013,0x0000); 
	_lcd_ili9225b_reg_write(0x0014,0x0000); 
	mdelay(40);                       
	_lcd_ili9225b_reg_write(0x0011,0x0018); 
	_lcd_ili9225b_reg_write(0x0012,0x6121); 
	_lcd_ili9225b_reg_write(0x0013,0x0043);
	_lcd_ili9225b_reg_write(0x0014,0x414A); //414E
	_lcd_ili9225b_reg_write(0x0010,0x0800);
	mdelay(10);                       
	_lcd_ili9225b_reg_write(0x0011,0x103B); 
	mdelay(30);                       
	_lcd_ili9225b_reg_write(0x0001,0x011C);
	_lcd_ili9225b_reg_write(0x0002,0x0100); 
	_lcd_ili9225b_reg_write(0x0003,0x1030); 
	_lcd_ili9225b_reg_write(0x0007,0x0000); 
	_lcd_ili9225b_reg_write(0x0008,0x0808); 
	_lcd_ili9225b_reg_write(0x000B,0x1100);
	_lcd_ili9225b_reg_write(0x000C,0x0000); 
	mdelay(15);
	_lcd_ili9225b_reg_write(0x0015,0x0020);
	_lcd_ili9225b_reg_write(0x0020,0x0000);
	_lcd_ili9225b_reg_write(0x0021,0x0000);
	_lcd_ili9225b_reg_write(0x0030,0x0000); 
	_lcd_ili9225b_reg_write(0x0031,0x00DB); 
	_lcd_ili9225b_reg_write(0x0032,0x0000); 
	_lcd_ili9225b_reg_write(0x0033,0x0000); 
	_lcd_ili9225b_reg_write(0x0034,0x00DB); 
	_lcd_ili9225b_reg_write(0x0035,0x0000); 
	_lcd_ili9225b_reg_write(0x0036,0x00AF); 
	_lcd_ili9225b_reg_write(0x0037,0x0000); 
	_lcd_ili9225b_reg_write(0x0038,0x00DB); 
	_lcd_ili9225b_reg_write(0x0039,0x0000); 
	_lcd_ili9225b_reg_write(0x0050,0x0404); 
	_lcd_ili9225b_reg_write(0x0051,0x0404); 
	_lcd_ili9225b_reg_write(0x0052,0x0404); 
	_lcd_ili9225b_reg_write(0x0053,0x0404); 
	_lcd_ili9225b_reg_write(0x0054,0x0404); 
	_lcd_ili9225b_reg_write(0x0055,0x0404); 
	_lcd_ili9225b_reg_write(0x0056,0x0404); 
	_lcd_ili9225b_reg_write(0x0057,0x0404); 
	_lcd_ili9225b_reg_write(0x0058,0x0C00);
	_lcd_ili9225b_reg_write(0x0059,0x000C); 
	_lcd_ili9225b_reg_write(0x000F,0x0801); 
	_lcd_ili9225b_reg_write(0x00EC,0x124A);
	_lcd_ili9225b_reg_write(0x0007,0x0012); 
	mdelay(50);                      
	_lcd_ili9225b_reg_write(0x0007,0x1017); 
	_lcd_ili9225b_reg(0x0022);
}

static void
_lcd_panel_init_hsd24(void)
{
	_lcd_ili9225b_reg_write(0x00E3,0x3008);          // Set internal timing
	_lcd_ili9225b_reg_write(0x00E7,0x0012);          // Set internal timing
	_lcd_ili9225b_reg_write(0x00EF,0x1231);          // Set internal timing
	_lcd_ili9225b_reg_write(0x00E5,0x78F0);          // Set internal timing

	//************* Start Initial Sequence **********//
	_lcd_ili9225b_reg_write(0x0001, 0x0100); // set SS and SM bit
	_lcd_ili9225b_reg_write(0x0002, 0x0200); // set 1 line inversion
	_lcd_ili9225b_reg_write(0x0003, 0x1030); // set GRAM write direction and BGR=1.
	_lcd_ili9225b_reg_write(0x0004, 0x0000); // Resize register
	_lcd_ili9225b_reg_write(0x0008, 0x0207); // set the back porch and front porch
	_lcd_ili9225b_reg_write(0x0009, 0x0000); // set non-display area refresh cycle ISC[3:0]
	_lcd_ili9225b_reg_write(0x000A, 0x0000); // FMARK function
	_lcd_ili9225b_reg_write(0x000C, 0x0000); // RGB interface setting
	_lcd_ili9225b_reg_write(0x000D, 0x0000); // Frame marker Position
	_lcd_ili9225b_reg_write(0x000F, 0x0000); // RGB interface polarity


	//*************Power On sequence ****************//
	_lcd_ili9225b_reg_write(0x0010, 0x0000); // SAP, BT[3:0], AP, DSTB, SLP, STB
	_lcd_ili9225b_reg_write(0x0011, 0x0007); // DC1[2:0], DC0[2:0], VC[2:0]
	_lcd_ili9225b_reg_write(0x0012, 0x0000); // VREG1OUT voltage
	_lcd_ili9225b_reg_write(0x0013, 0x0000); // VDV[4:0] for VCOM amplitude
	_lcd_ili9225b_reg_write(0x0007, 0x0001);
	msleep(200); // Dis-charge capacitor power voltage
	_lcd_ili9225b_reg_write(0x0010, 0x1690); // SAP, BT[3:0], AP, DSTB, SLP, STB
	_lcd_ili9225b_reg_write(0x0011, 0x0227); // Set DC1[2:0], DC0[2:0], VC[2:0]
	msleep(50); // Delay 50ms
	_lcd_ili9225b_reg_write(0x0012, 0x000D); // External reference voltage= Vci;
	msleep(50); // Delay 50ms
	_lcd_ili9225b_reg_write(0x0013, 0x1200); // VDV[4:0] for VCOM amplitude
	_lcd_ili9225b_reg_write(0x0029, 0x0007); // VCM[5:0] for VCOMH
	_lcd_ili9225b_reg_write(0x002B, 0x000C); // Set Frame Rate
	msleep(50); // Delay 50ms
	_lcd_ili9225b_reg_write(0x0020, 0x0000); // GRAM horizontal Address
	_lcd_ili9225b_reg_write(0x0021, 0x0000); // GRAM Vertical Address


	// ----------- Adjust the Gamma Curve ----------//
	_lcd_ili9225b_reg_write(0x0030, 0x0000);
	_lcd_ili9225b_reg_write(0x0031, 0x0404);
	_lcd_ili9225b_reg_write(0x0032, 0x0003);
	_lcd_ili9225b_reg_write(0x0035, 0x0405);
	_lcd_ili9225b_reg_write(0x0036, 0x0808);
	_lcd_ili9225b_reg_write(0x0037, 0x0407);
	_lcd_ili9225b_reg_write(0x0038, 0x0303);
	_lcd_ili9225b_reg_write(0x0039, 0x0707);
	_lcd_ili9225b_reg_write(0x003C, 0x0504);
	_lcd_ili9225b_reg_write(0x003D, 0x0808);

	//------------------ Set GRAM area ---------------//
	_lcd_ili9225b_reg_write(0x0050, 0x0000); // Horizontal GRAM Start Address
	_lcd_ili9225b_reg_write(0x0051, 0x00EF); // Horizontal GRAM End Address
	_lcd_ili9225b_reg_write(0x0052, 0x0000); // Vertical GRAM Start Address
	_lcd_ili9225b_reg_write(0x0053, 0x013F); // Vertical GRAM End Address
	_lcd_ili9225b_reg_write(0x0060, 0xA700); // Gate Scan Line
	_lcd_ili9225b_reg_write(0x0061, 0x0001); // NDL,VLE, REV
	_lcd_ili9225b_reg_write(0x006A, 0x0000); // set scrolling line

	//-------------- Partial Display Control ---------//
	_lcd_ili9225b_reg_write(0x0080, 0x0000);
	_lcd_ili9225b_reg_write(0x0081, 0x0000);
	_lcd_ili9225b_reg_write(0x0082, 0x0000);
	_lcd_ili9225b_reg_write(0x0083, 0x0000);
	_lcd_ili9225b_reg_write(0x0084, 0x0000);
	_lcd_ili9225b_reg_write(0x0085, 0x0000);

	//-------------- Panel Control -------------------//
	_lcd_ili9225b_reg_write(0x0090, 0x0010);
	_lcd_ili9225b_reg_write(0x0092, 0x0000);
	_lcd_ili9225b_reg_write(0x0007, 0x0133); // 262K color and display ON
	_lcd_ili9225b_reg(0x22);
}

void _lcd_prepare_write(void)
{
	lcd_prepare_write(0, 0);
}

static void
_lcd_panel_init(void)
{
	int id = q_lcd_panel_id();

	printk("Initialize %s LCD panel\n",
			_lcd_panel_str[id] ? _lcd_panel_str[id] : "unknown");

	switch (id) {
	case _LCD_PANEL_HSD24:
		_lcd_panel_init_hsd24();
		break;
	case _LCD_PANEL_TRULY:
		_lcd_panel_init_truly();
		break;
	case _LCD_PANEL_TCL:
	default:
		_lcd_panel_init_tcl();
		break;
	}
}

void _lcd_set_resolution(void)
{
	int id = q_lcd_panel_id();

	switch (id) {
	case _LCD_PANEL_HSD24:
		_h_resolution_osd = 240;
		_v_resolution_osd = 320;

		_h_resolution = 240;
		_v_resolution = 320;

		_h_resolution_virtual = 240;
		_v_resolution_virtual = 640;

		break;
	default:
		break;
	}

	osd_right_bottom_x = _h_resolution_osd-1;
	osd_left_bottom_y = _v_resolution_osd -1;

	mach_info.vidtcon2= S3C_VIDTCON2_LINEVAL(_v_resolution-1) | S3C_VIDTCON2_HOZVAL(_h_resolution-1);
#if defined(CONFIG_CPU_S3C2443) ||  defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416)
	mach_info.vidosd0b= S3C_VIDOSDxB_OSD_RBX_F(_h_resolution-1) | S3C_VIDOSDxB_OSD_RBY_F(_v_resolution-1);
	mach_info.vidosd1b= S3C_VIDOSDxB_OSD_RBX_F(_h_resolution_osd-1) | S3C_VIDOSDxB_OSD_RBY_F(_v_resolution_osd-1);
#elif defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
	mach_info.vidosd0b = S3C_VIDOSDxB_OSD_RBX_F(_h_resolution-1) | S3C_VIDOSDxB_OSD_RBY_F(_v_resolution-1);
	mach_info.vidosd0c = S3C_VIDOSDxD_OSDSIZE(_h_resolution*_v_resolution);
	mach_info.vidosd1b = S3C_VIDOSDxB_OSD_RBX_F(_h_resolution_osd-1) | S3C_VIDOSDxB_OSD_RBY_F(_v_resolution_osd-1);
	mach_info.vidosd1d = S3C_VIDOSDxD_OSDSIZE(_h_resolution*_v_resolution);
	mach_info.vidosd2b = S3C_VIDOSDxB_OSD_RBX_F(_h_resolution_osd-1) | S3C_VIDOSDxB_OSD_RBY_F(_v_resolution_osd-1);
	mach_info.vidosd2d = S3C_VIDOSDxD_OSDSIZE(_h_resolution*_v_resolution);
	mach_info.vidosd3b = S3C_VIDOSDxB_OSD_RBX_F(_h_resolution_osd-1) | S3C_VIDOSDxB_OSD_RBY_F(_v_resolution_osd-1);
	mach_info.vidosd4b = S3C_VIDOSDxB_OSD_RBX_F(_h_resolution_osd-1) | S3C_VIDOSDxB_OSD_RBY_F(_v_resolution_osd-1);
#endif

	mach_info.width= _h_resolution;
	mach_info.height= _v_resolution;
	mach_info.xres=	_h_resolution;
	mach_info.yres=	_v_resolution;

#if defined(CONFIG_FB_VIRTUAL_SCREEN)
	mach_info.xres_virtual = _h_resolution_virtual;
	mach_info.yres_virtual = _v_resolution_virtual;
#else
	mach_info.xres_virtual = _h_resolution;
	mach_info.yres_virtual = _v_resolution;
#endif

	mach_info.osd_width = _h_resolution_osd;
	mach_info.osd_height = _v_resolution_osd;
	mach_info.osd_xres = _h_resolution_osd;
	mach_info.osd_yres = _v_resolution_osd;

	mach_info.osd_xres_virtual = _h_resolution_osd;
	mach_info.osd_yres_virtual = _v_resolution_osd;

      	mach_info.pixclock = VFRAME_FREQ
		* (VFRAME_FREQ *(H_FP+H_SW+H_BP+_h_resolution) * (V_FP+V_SW+V_BP+_v_resolution));
}

void lcd_module_init (void)
{
	/* set window size */
	int i = _h_resolution * _v_resolution;
	u16 *logo = NULL;

	if (q_boot_flag_get() != Q_BOOT_FLAG_LCD_INIT) {
		if (q_hw_ver(KTQOOK))
			_lcd_vc0528_init(1);
		else
			_lcd_s3c_i80_init(1);
	} else {
		if (q_hw_ver(KTQOOK))
			_lcd_vc0528_init(0);
		else
			_lcd_s3c_i80_init(0);

		q_boot_flag_set(Q_BOOT_FLAG_CLEAR);

		return ;
	}

	if (q_hw_ver(7800))
		logo = (u16 *)&_mylg070_logo[12];
	else if (q_hw_ver(SKBB))
		logo = (u16 *)&_skbb_logo[12];
	else if (q_hw_ver(KTQOOK))
		logo = (u16 *)&_kt_logo[12];

	lcd_set_command_mode(1);

	_lcd_panel_init();

	lcd_prepare_write(0, 0);
	while (i-- > 0) {
		lcd_write_pixel(*logo);
		logo++;
	}

	lcd_set_command_mode(0);
}

void SetLcdPort(void)
{
	lcd_gpio_init();
}

void Init_LDI(void)
{
	printk(KERN_INFO "LCD TYPE :: S3C_ILI9225QC LCD will only be set port\n");

	SetLcdPort();
	_lcd_set_resolution();
}

int
_s3c_lcd_get_value(char *buf, int count)
{
	int value = 0, first = 1;
	char *p;

	if ((strlen(buf) > 3)
			&& (buf[1] == 'x' || buf[1] == 'X')) {
		p = &buf[2]; count -= 2;
		while (*p && count) {
			if (!first) value <<= 4;
			else first = 0;

			if (*p >= 'a' && *p <= 'f') {
				value += (*p - 'a' + 10);
			} else if (*p >= 'A' && *p <= 'F') {
				value += (*p - 'A' + 10);
			} else if (*p >= '0' && *p <= '9') {
				value += (*p - '0');
			} else
				return -1;

			count--; p++;
		}
	} else {
		p = buf;

		while (*p && count) {
			if (!first) value *= 10;
			else first = 0;

			if (*p >= '0' && *p <= '9') {
				value += (*p - '0');
			} else
				return -1;

			count--; p++;
		}
	}

	return value;
}

static int
_s3c_lcd_get_param(char *buf, size_t len, int *param1, int *param2)
{
	char *breg, *bvalue = NULL;
	int creg, cvalue = 0;
	int reg, value;

	breg = bvalue = buf;

	if (len < 1) return -1;
	if (buf[len-1] == '\n') {
		buf[len-1] = '\0';
		len--;
	}

	while (*bvalue) {
		if (*bvalue == ' ') {
			creg = bvalue - buf;
			cvalue = strlen(buf) - creg - 1;
			bvalue++;
			break;
		}

		bvalue++;
	}

	if (creg) *param1 = _s3c_lcd_get_value(breg, creg);
	if (cvalue) *param2 = _s3c_lcd_get_value(bvalue, cvalue);

	return 0;
}


static int
s3c_ili9225b_lcd_reg_read_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t len)
{
	int reg = 0, count = 0, value;
	int i;

	_s3c_lcd_get_param(buf, strlen(buf), &reg, &count);

	if (reg < 0) {
		printk("Invalid register\n");
		return -1;
	}

	if (count <= 0) count = 1;

	lcd_set_command_mode(1);
	for (i = 0; i < count; i++) {
		value = _lcd_ili9225b_reg_read(reg + i);
		printk("Reg : %04X = %04X\n", reg + i, value);
	}
	lcd_set_command_mode(0);

	return len;
}

static int
s3c_ili9225b_lcd_reg_write_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t len)
{
	int reg = -1, value = -1;

	_s3c_lcd_get_param(buf, strlen(buf), &reg, &value);

	if (reg < 0 || value < 0) {
		printk("Invalid param\n");
	}

	lcd_set_command_mode(1);
	_lcd_ili9225b_reg_write(reg, value);
	lcd_set_command_mode(0);

	printk("Reg : %04X = %04X\n", reg, value);

	return len;
}

static DEVICE_ATTR(lcd_reg_read, 0644,
		   NULL,
		   s3c_ili9225b_lcd_reg_read_store);

static DEVICE_ATTR(lcd_reg_write, 0644,
		   NULL,
		   s3c_ili9225b_lcd_reg_write_store);

int
s3c_ili9225b_device_create_file(struct platform_device *pdev)
{
	int ret;

	ret = device_create_file(&(pdev->dev), &dev_attr_lcd_reg_read);
	if (ret < 0) return ret;

	return device_create_file(&(pdev->dev), &dev_attr_lcd_reg_write);
}

