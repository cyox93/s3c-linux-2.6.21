/*************************************************************************
*                                                                       
*             Copyright (C) 2005 Vimicro CO.,LTD     		 
*
* FILE NAME                                  VERSION                
*                                                                       
* VIM_DRIVERIC_Info.ch				           	  0.1                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*    VC0578 LCD driver ic information head file                     
*
*	Version		Author			Date		Description		
*  ---------------------------------------------------------------						 
*   0.1			liuhuadian  2005-11-01	The first version. 
*  ---------------------------------------------------------------
*                                                                       
*************************************************************************/
/****************************************************************************
This source code has been made available to you by VIMICRO on an
AS-IS basis. Anyone receiving this source code is licensed under VIMICRO
copyrights to use it in any way he or she deems fit, including copying it,
modifying it, compiling it, and redistributing it either with or without
modifications. Any person who transfers this source code or any derivative
work must include the VIMICRO copyright notice and this paragraph in
the transferred software.
****************************************************************************/



#ifndef _VIM_5XRDK_DRIVERINFO_H_
#define _VIM_5XRDK_DRIVERINFO_H_

#ifdef __cplusplus
extern "C" {
#endif

enum PANELCSenum {
	MAINPANEL=0,
	SUBPANEL=0x10
};
enum PANELFMTenum {
	PANEL_FMT565_1TIMES=0,
	PANEL_FMT565_2TIMES=4,
	
	PANEL_FMT555_1TIMES=1,

	PANEL_FMT666_1TIMES=3,
	PANEL_FMT666_2TIMES_LITTLE=5,
	PANEL_FMT666_2TIMES_BIG=6,
	
	PANEL_FMT444_1TIMES=2,
	PANEL_FMT444_2TIMES_BIG=8,
	PANEL_FMT444_2TIMES_LITTLE=9,
	PANEL_FMT444_3TIMES_BIG=12,
	PANEL_FMT444_3TIMES_LITTLE=13	

};
enum PANELRGBenum {
	PANEL_RGB=0<<4,
	PANEL_RBG=1<<4,
	PANEL_GRB=2<<4,
	PANEL_GBR=3<<4,
	PANEL_BRG=4<<4,
	PANEL_BGR=5<<4
};

enum DRIVERICFLAGenum {
	COMMAND_ONLY=0,
	COMMAND_VALUE_ONEBYTE=BIT1,
	REPEAT_COORDINATE=BIT0
};

#define PANEL_CONVERSE_HIGHLOW 0x80

//define LCD panel type    //angela change
#define NOSUB									1

//Hitachi
#define V5_DEF_LCD_HD66773_16BIT				2
#define V5_DEF_LCD_HD66773_8BIT				3
#define V5_DEF_LCD_HD66777_16BIT	       		4
#define V5_DEF_LCD_HD66768_8BIT				12
#define V5_DEF_LCD_HD66772_HD66774_16BIT		13
#define V5_DEF_LCD_HD66781S_HD66783_16BIT	16

////LDS
#define V5_DEF_LCD_LDS183_16BIT				5
#define V5_DEF_LCD_LDS183_8BIT					6
#define V5_DEF_LCD_LDS506_8BIT					33


//Solomon
#define V5_DEF_LCD_SSD1784_8BIT				103
#define V5_DEF_LCD_SSD1788_8BIT				8
#define V5_DEF_LCD_SSD1781_8BIT				15
#define V5_DEF_LCD_SSD1773_8BIT				19
#define V5_DEF_LCD_SSD1332_8BIT				23
#define  V5_DEF_LCD_SSD1289_16BIT				48

//Samsung
#define V5_DEF_LCD_S6B33B5_8BIT				9
#define V5_DEF_LCD_S6B33B2_S6B33BG_16BIT		10
#define V5_DEF_LCD_S6B33B2_S6B33BG_8BIT		11
#define V5_DEF_LCD_S6D0110A_8BIT				17
#define  V5_DEF_LCD_S6D0110_16BIT          		20
#define V5_DEF_LCD_S1D19102_S1D17D01_16BIT	21
#define V5_DEF_LCD_S6D0118_8BIT				22
#define V5_DEF_LCD_S1D19105D_16BIT			24
#define V5_DEF_LCD_S1D19111D_8BIT				27
#define V5_DEF_LCD_S6B33B9_16BIT				31
#define V5_DEF_LCD_S6D0123_16BIT				32
#define V5_DEF_LCD_S6D0118_16BIT 				39
#define V5_DEF_LCD_S6B33BC_16BIT				41
#define V5_DEF_LCD_S6B33BC_8BIT				42
#define V5_DEF_LCD_S6D0144_16BIT				45
#define V5_DEF_LCD_S6D0134_16BIT				71	//heju add??
#define V5_DEF_LCD_S6D0164_8BIT				95
#define V5_DEF_LCD_S6D0164_16BIT				102
#define V5_DEF_LCD_S1D19120_16BIT				51
#define V5_DEF_LCD_S1D19501_16BIT				55			//塞路风京东方lcd 3/19/2007
#define V5_DEF_LCD_S6D0139_16BIT				62			//6223 project
#define V5_DEF_LCD_S6B33B6X_8BIT				70			//西安中兴to zhongxi
#define V5_DEF_LCD_S6B33BF_8BIT				73			//BoDao 11/6/2007//
#define V5_DEF_LCD_S6B33BF_16BIT				78			//上海展帆//10/2007
#define V5_DEF_LCD_S6B33BL_8BIT				80			//波导to ShangFeng //11/26/2007 //80 is Last one!

//HiMark
#define V5_DEF_LCD_HX8301_HX8609A_16BIT		14
#define V5_DEF_LCD_HX8309_16BIT				30
#define V5_DEF_LCD_HX8312A_16BIT 				40
#define V5_DEF_LCD_HX8309_8BIT				46
#define V5_DEF_LCD_HX8346A_16BIT				58			//普阳lcd 4/19/2007
#define V5_DEF_LCD_HX8347_A01_16BIT			75			//上海闻泰to WangJian //10/2007
#define V5_DEF_LCD_HX8345A_8BIT				79			//上海展帆//10/2007
#define V5_DEF_LCD_HX8345_16BIT				83			//深圳普阳to Brave//12/03/2007//客户反馈OK
#define V5_DEF_LCD_HX8340B_8BIT				109

//Nec
#define V5_DEF_LCD_uPD161801_uPD161861_16BIT	18
#define V5_DEF_LCD_uPD161963_8BIT					25
#define V5_DEF_LCD_UPD161690_8BIT 				26


//LG
#define V5_DEF_LCD_LGDP4511_16BIT				28
#define V5_DEF_LCD_LGDP4216_16BIT				34
#define V5_DEF_LCD_LGDP4531_16BIT				60			//6223
#define V5_DEF_LCD_LGDP4532_8BIT				96
#define V5_DEF_LCD_LGDP4512_8BIT				63			//立东DS600 project
#define V5_DEF_LCD_LGDP4522_8BIT				99
#define V5_DEF_LCD_LGDP4524_8BIT				107
#define V5_DEF_LCD_LGDP4524_16BIT				110

//Tomato
#define V5_DEF_LCD_TL1711_16BIT				29
#define V5_DEF_LCD_TL1772_8BIT					44
#define V5_DEF_LCD_TL1763_16BIT				56			//韶丰TL1763


//Renesas
#define V5_DEF_LCD_R61500_16BIT				35
#define V5_DEF_LCD_R61503B_16BIT				38
#define V5_DEF_LCD_R61503B_8BIT				50
#define V5_DEF_LCD_R61508_16BIT				53
#define V5_DEF_LCD_R61505U_16BIT				59			//Ginwave s818
#define V5_DEF_LCD_R61509_16BIT				72			//TopWise		9/10/2007 //
#define V5_DEF_LCD_R61503U_16BIT				87
#define V5_DEF_LCD_R61503U_8BIT				88


//Toppoly
#define V5_DEF_LCD_C1E2_04_16BIT				37
#define V5_DEF_LCD_PL2001A_16BIT				7

//Isron
#define V5_DEF_LCD_IS2320_16BIT				36
#define V5_DEF_LCD_IS2102_IS2202_16BIT		43

//Orise Tech
#define V5_DEF_LCD_SPFD5414_8BIT				98
#define V5_DEF_LCD_SPFD5414_16BIT				47
#define V5_DEF_LCD_SPFD54126_16BIT			49
#define V5_DEF_LCD_SPFD54124A_16BIT   			90
#define V5_DEF_LCD_SPFD54124B_16BIT			94			///1/3/2007
#define V5_DEF_LCD_SPFD5408A_16BIT			76			//same as above
#define V5_DEF_LCD_SPFD5420A_16IT				108


//sitronix
#define V5_DEF_LCD_ST7636_8BIT					52
#define V5_DEF_LCD_ST7712_8BIT					57			//天宇lcd ST7712
#define V5_DEF_LCD_ST7787_16BIT				64			//众和伟业z240 project
#define V5_DEF_LCD_ST7669_8BIT					81			//宁波义隆达//11/28/2007
#define V5_DEF_LCD_ST7637_16BIT   				89
#define V5_DEF_LCD_ST7732_16BIT 				91
#define V5_DEF_LCD_ST7637_8BIT					68			//互芯to ZhengLei Sopio

//ULTRA
#define V5_DEF_LCD_UC1697_8BIT				54			//凌达lcd 1/23/2007

//ILITEK
#define V5_DEF_LCD_ILI9220_8BIT				106			//天朗互联//to wangchuanfu//1/11/2008
#define V5_DEF_LCD_ILI9220_16BIT				93			//zhong xi fan kui ok!
#define V5_DEF_LCD_ILI9221_16BIT				61			//6225 project
#define V5_DEF_LCD_ILI9160_8BIT				65			//众和伟业z150 project
#define V5_DEF_LCD_ILI9320_16BIT				66			//达成无限to ZhengHui
#define V5_DEF_LCD_ILI9320_8BIT				92			//展望//12/26/2007
#define V5_DEF_LCD_ILI9325_16BIT				85	


//TPO
#define V5_DEF_LCD_C1L5_06_16BIT				67			//互芯to ZhengLei Sopio
#define V5_DEF_LCD_C1L5_06_8BIT				77			//翼龙达//10/2007
#define V5_DEF_LCD_L1E2_16BIT					69			// 中兴

//NOVATEK
#define V5_DEF_LCD_NT39102_16BIT				74			//to zhenghui//11/13/2007
#define V5_DEF_LCD_NT39102_8BIT				82			//上海华勤to Terry //11/28/2007//客户反馈OK
#define V5_DEF_LCD_NT75751_8BIT				84			//深圳优创to Brave//12/03/2007//客户反馈OK
#define V5_DEF_LCD_NT39118_8BIT				101
#define V5_DEF_LCD_NT3911_16BIT				104

//TLS
#define V5_DEF_LCD_TLS8301_8BIT				86

//MagnaChip
#define V5_DEF_LCD_MC2TA7402_8BIT				100

//FocalTech
#define V5_DEF_LCD_FT1503_16BIT				105

//#define V5_DEF_LCD_LGDP4524_8BIT				107

//#define V5_DEF_LCD_SPFD5420A_16IT				108

//#define V5_DEF_LCD_HX8340B_8BIT				109

//#define V5_DEF_LCD_LGDP4524_16BIT				110

void	PanelSetReg(UINT8 type, UINT32 adr, UINT16 val);

typedef void (*PLcdRotate)(UINT8 degree, UINT8 WorkMode);						//guoying 1/10/2007VIM_HAPI_ROTATEMODE degree
typedef void (*PPanelCallback)(void);

typedef struct tag_TLcdifParm {
	UINT8	SpecialLCD_flag;						//wendy guo 1/24/2007, this flag =1, means use Repeat_stxp... parameters
	UINT16	rsflag;
	UINT16	headnum;
	UINT16	head[16];
	UINT16	stxp, styp, endxp, endyp;
	UINT16	Repeat_stxp, Repeat_styp, Repeat_endxp, Repeat_endyp;
	UINT16    Head_Process_flag;                                                                                                                                                //new parameter
	UINT16    Start_x_comm_code,End_x_comm_code,Start_y_comm_code,End_y_comm_code;
	UINT8	config;
	UINT8	fmt;
	UINT16	wcy, rcy;
	UINT8 	dedelay,deinteval,lineinteval;
	PLcdRotate	lcdRotate;
} TLcdifParm, *PTLcdifParm;


typedef struct tag_TPanelInfo {
	PTLcdifParm master;
	PTLcdifParm slave;
	PPanelCallback	pBacklightOnEx;		//extra backlight on function
} TPanelInfo, *PTPanelInfo;

#define VIM_DISP_BySetRSlow(adr)    *(volatile UINT16 *)0x8000000=adr
#define VIM_DISP_BySetRShigh(val)   *(volatile UINT16 *)0x8000200=val
#if 0
typedef struct tag_TLcdifParm_SpecialLCD {
	UINT16	rsflag;
	UINT16	headnum;
	UINT16	head[16];
	UINT16	stxp, styp, endxp, endyp, Repeat_stxp, Repeat_styp, Repeat_endxp, Repeat_endyp;
	UINT8	config;
	UINT8	fmt;
	UINT16	wcy, rcy;
	UINT8 dedelay,deinteval,lineinteval;
} TLcdifParm_SplLCD, *PTLcdifParm_SplLCD;
#endif
extern const TPanelInfo gDriverIcInfo;
#ifdef __cplusplus
}
#endif

#endif


