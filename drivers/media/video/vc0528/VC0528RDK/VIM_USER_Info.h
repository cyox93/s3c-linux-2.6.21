 /*************************************************************************
*                                                                       
*             Copyright (C) 2005 Vimicro CO.,LTD     		 
*
* FILE NAME                                  VERSION                
*                                                                       
* VIM_USER_Info.h						           	  0.2                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*     vc5x 's customer modify function                        
*
*	Version		Author			Date		Description		
*  ---------------------------------------------------------------						 
*   0.1			angela  2005-11-01	The first version. 
*   0.2			angela  2006-06-06	update for528
*   1.0			angela  2006-12-08	update 
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

#ifndef _VIM_5XRDK_USER_INFO_H_
#define _VIM_5XRDK_USER_INFO_H_

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************************
*     this define select the bus type multi 16 or multi8
* 	VIM_MULTI8    VIM_MULTI16
**********************************************************************************/
#define VIM_BUSTYPE		VIM_MULTI16		
/********************************************************************************
*     this define select the endian
* 	VIM_LITTLEENDIAN    VIM_BIGENDIAN
**********************************************************************************/
#define VIM_ENDIANTYPE		VIM_LITTLEENDIAN  

/********************************************************************************
*     this define for special infineon platform
* 	if other platform ,please do not define it
**********************************************************************************/
#ifndef __INFINEON_PLATFORM__
#define __INFINEON_PLATFORM__		0
#endif
/********************************************************************************
*    when access to v5x ,please define this address
**********************************************************************************/
//  CS address
#define VIM_REG_INDEX		0x8000000	// vc0528 a8(rs) is 0,means address 
#define VIM_REG_VALUE		0x8000100    // vc0528 a8(rs) is 1,means value 

/********************************************************************************
*    Write read IO control by GPIO
*    If user uses Address bus to switch them, need not implement them
**********************************************************************************/
#define VIM_SET_RSLOW_BY_GPIO() //(rBASEBAND_PDATG &= ~BIT2)
#define VIM_SET_RSHIGH_BY_GPIO() //(rBASEBAND_PDATG |= BIT2)


/********************************************************************************
*    when in the bypass mode
*   the external pin will be set according to the value
**********************************************************************************/
#define	VIM_BYPASS_PIN_INT    			0	// external interrupt pin 
#define	VIM_BYPASS_PIN_LCDRES 		1	//lcd reset pin 
#define	VIM_BYPASS_PIN_CSRSTN     		0	// sensor reset pin
#define	VIM_BYPASS_PIN_CSPWD			0	// sensor power down pin  
#define	VIM_BYPASS_PIN_CSENB			0	// sensor power enable pin  
#define	VIM_BYPASS_PIN_GPIO4          	0	// gpio 4
#define	VIM_BYPASS_PIN_GPIO3			0	// gpio 3
#define	VIM_BYPASS_PIN_GPIO2            	0	// gpio 2
#define	VIM_BYPASS_PIN_GPIO1            	1	// gpio 1
#define	VIM_BYPASS_PIN_GPIO0               	1	// gpio 0

#define VIM_BYPASS_RESETCHIP			0
/********************************************************************************
*    Used for GPIO switching bypass and normal modes
*   If user does not use GPIO to switch bypass and normal, does not implement them.
*	High to bypass, low to normal
**********************************************************************************/
#define VIM_SET_BYPASS_BY_GPIO()	
#define VIM_SET_NORMAL_BY_GPIO()	

/********************************************************************************
*     When switch to bypass mode, disable base band to vimicro 5X's clock for saving power
**********************************************************************************/
#define VIM_SET_XCLKOFF()
#define VIM_SET_XCLKON()



/********************************************************************************
*    Select driver ic
**********************************************************************************/
#define V5_MAINDRIVERIC_TYPE 	V5_DEF_LCD_S1D19120_16BIT//V5_DEF_LCD_S1D19102_S1D17D01_16BIT//V5_DEF_LCD_S1D19102_S1D17D01_16BIT//V5_DEF_LCD_LGDP4524_8BIT//V5_DEF_LCD_ILI9320_16BIT//V5_DEF_LCD_S1D19120_16BIT//V5_DEF_LCD_S1D19102_S1D17D01_16BIT//V5_DEF_LCD_S1D19120_16BIT//V5_DEF_LCD_R61503B_8BIT//
#define V5_SUBDRIVERIC_TYPE 	NOSUB

/********************************************************************************
*  select sensor
**********************************************************************************/
//0.3M
#define V5_DEF_SNR_MT9V111_YUV		1	//MI360SOC

#define V5_DEF_SNR_OV7660_YUV 		1
#define V5_DEF_SNR_OV7670_YUV			1
#define V5_DEF_SNR_OV7649_YUV			1
#define V5_DEF_SNR_HV7131RP_YUV		1	//HYCA3R
#define V5_DEF_SNR_NOON30PC11_YUV	1
#define V5_DEF_SNR_HV7131GP_YUV		1
#define V5_DEF_SNR_S5K83A_YUV			1
#define V5_DEF_SNR_MC501CB_YUV		1

#define V5_DEF_SNR_PO3030K_YUV		1

#define V5_DEF_SNR_SIV100A_YUV		1

#define  V5_DEF_SNR_OV6680_YUV		1
#define  V5_DEF_SNR_SP80818_YUV		1
#define  V5_DEF_SNR_MT9V112_YUV		1

#define  V5_DEF_SNR_GC306_YUV			1


//release 1.5 version add:
#define V5_DEF_SNR_S5KA3AFX_YUV		1
#define V5_DEF_SNR_OV7725_YUV			1
#define V5_DEF_SNR_SIV100B_YUV		1
#define V5_DEF_SNR_ET8EC3_YUV			1
#define V5_DEF_SNR_BF3403_YUV			1

#define  V5_DEF_SNR_TEST_YUV			1		//test for some sensor or TV chip
#define  V5_DEF_SNR_FUJITSU_CCIR656	1		//test for FUJITSU TV chip
#define   V5_DEF_SNR_SANYO_NORMAL		1	

//OM6802 is a special sensor, must check this sensor ID at last.
#define  V5_DEF_SNR_OM6802_YUV		1


/********************************************************************************
*  use for software define
**********************************************************************************/

//select	sensor output size   
extern UINT8 FastPreview;

/*-----------------for preview ---------------*/
#define VIM_USER_PREVIEW_ALAYER_MODE VIM_DISP_LINEBUF//VIM_DISP_LINEBUF//VIM_DISP_ONEFRAME // To make view size almost the same to different capture size
#define VIM_USER_FAST_PREVIEW 1//FastPreview  // To make view size almost the same to different capture size

#define VIM_USER_ZOOM_MIN_WIDHT 0  // To make view size almost the same to different capture size
							 // If don't limit, set it to 0
							 // 320 is the zoom source size widht of 640x480
#define VIM_USER_MAX_AMEM_WIDTH	0	// To make zoom has the same view space
#define VIM_USER_SURPPORT_DISPLAYZOOM	0	// To make zoom has the same view space
#define VIM_USER_MAX_STEP		5		//means the max step of zoom
#define VIM_USER_MIN_PIXEL_ONESTEP	5	//meas thi limit of min pixel per step

#define VIM_USER_BRITNESSMAX_STEP		11		//means the max step of britness
#define VIM_USER_CONTRASTMAX_STEP		11		//means the max step of contrast
#define VIM_USER_WHITEBALANCE_MANUALSTEP		11		//means the max step of manual of white balance
/*-----------------for capture ---------------*/
#define VIM_USER_RECAPTURE_OFFSET	10000//BYTE ,TWC+VIM_USER_RECAPTURE_OFFSET <JBUF ,THEN RECAPTURE 7
#define VIM_USER_BUFLENGTH	0x20000//0x1ffff//0x20000  
#define VIM_USER_RECAPTUREMAXTIMES	10
#define VIM_USER_QFONESTEP	10
#define VIM_USER_CAPTURE_TIMER   5 //(MS ) means when capture ,timer interval check done interrupt
#define VIM_USER_GETQUICKPIC_TIMES   5//(MS ) when use VIM_HAPI_GetQuickOneJpeg this funciton ,if no interrupt times more than this value,it  will restart capture
#define VIM_USER_SURPORT_AVI	1	// 1 means support avi capture ,0 means not
#define VIM_USER_MAX_ARRAY_AE	  10	//the max number of array which store the AE register value. add by guoying 12/28/2007
//#define VIM_USER_MCLK_45M		1			//if 1, MCLK=45M, CLKIN=13Mhz ; if 0, MCLK=24M, CLKIN=24Mhz.

/*--------------for display-----------------*/
#define VIM_USER_SUPPORT_REALTIME_ROTATION	1//support rotaion real time

#define VIM_USER_DISPLAY_USELINEBUF	VIM_DISP_ONEFRAME	// 0 means use one frame  1 mean s use line buf when display
#define VIM_USER_DISPLAY_FULLSCREEN	1	// 0 means auto find position  1 means will  full fill the area

#define VIM_USER_DISPLAY_ZOOMDIVISOR	5
#define VIM_USER_DISPLAY_ZOOMOFFSETMAXSTEP	5	// display zoom的时候先除的一个值
#define VIM_USER_DISPLAY_ZOOMMINPIXEL	10	// display zoom的时候先除的一个值
/*--------------for lcd update-------------*/
extern UINT8 g_Reverse;
#define VIM_USER_LCD_DRAWDATA_LITTLEENDIAN g_Reverse//  about VIM_HAPI_DrawLCDRctngl function. when 0 data little endian .if 1 big endian


/*-----------------for 568api support ---------------*/
#define VIM_USER_SUPPORT568_API	1



/*******************end for user modify*****************************************/




//------Infineon------//
#if __INFINEON_PLATFORM__

#define 	HUGE		huge
#define	SHUGE		shuge
#define 	_FAR		_far	

#else

#define 	HUGE
#define	SHUGE
#define	_FAR

#endif


//--------------------------------------------------------------
// change Big Endian and Little Endian
#if VIM_ENDIANTYPE == VIM_LITTLEENDIAN

#define VIM_ENDIAN16(Value16)  (Value16)
#define VIM_ENDIAN32(Value32)  (Value32)

#else /*VIM_BIGENDIAN*/

#define VIM_ENDIAN16(Value16)  ((((Value16)&0xFF00)>>8)|\
                                (((Value16)&0x00FF)<<8))

#define VIM_ENDIAN32(Value32)  ((((Value32)&0x000000FF)<<24)|\
                                (((Value32)&0x0000FF00)<< 8)|\
                                (((Value32)&0x00FF0000)>> 8)|\
                                (((Value32)&0xFF000000)>>24))

#endif /*VIM_LITTLEENDIAN*/

// Set Bypass or Normal Type
typedef enum _VIM_BYPASSTYPE
{
	VIM_BYPASS_USE_GPIO = 0x0,
	VIM_BYPASS_USE_REG = 0x1
}VIM_BYPASSTYPE;

typedef enum _VIM_LDOMODE
{
	VIM_LDO_ON = 0x0,
	VIM_LDO_OFF = 0x1
}VIM_LDOMODE;

typedef enum _VIM_BYPASS_SUBPANELCTL
{
	VIM_BYPASS_SUB_CS2=0,
	VIM_BYPASS_SUB_CS1

}VIM_BYPASS_SUBPANELCTL;
typedef struct tag_VIM_BYPASS_DEF
{
	VIM_BYPASSTYPE BypassType;
	VIM_BYPASS_SUBPANELCTL BypassSubCtl;
	UINT8 BypassAddress;
	
}VIM_BYPASS_DEF,*PVIM_BYPASS_DEF;
typedef struct tag_VIM_PANEINIT
{
	TPoint Point;
	TSize Size;
} VIM_PANEINIT,*PVIM_PANEINIT;

typedef enum _VIM_INTERRUPT_TYPE
{
    	VIM_INT_LEVER_LOW=1,
       VIM_INT_LEVER_HIGH,
       VIM_INT_ADGE_DOWN,
       VIM_INT_ADGE_UP
}VIM_INTERRUPT_TYPE;
typedef struct tag_VIM_GPIO
{
	UINT8 GpioValue;   //if out ,the value 
} VIM_GPIO,*PVIM_GPIO;

typedef struct tag_VIM_CLK
{
	UINT32	Clkin;
	UINT32	Clkout;
	//UINT8 M;
	UINT8 M;
	UINT8 N;
	UINT8 NO;
	UINT8 DIV;
} VIM_CLK,*PVIM_CLK;
typedef struct tag_VIM_INTERRUPT
{
	VIM_INTERRUPT_TYPE Int_Type; 
	UINT8 LeverWidth;  //if type is lever 
} VIM_INTERRUPT,* PVIM_INTERRUPT;
typedef struct tag_VIM_TIMING
{
	UINT8 AddrDelay;
	UINT8 DataOutDelay;
	UINT8 DataInDelay;
	UINT8 DataRdDelay;
	UINT8 CsDelay;
	UINT8 WrDelay;
} VIM_TIMING,* PVIM_TIMING;



 typedef enum _VIM_EXP_TYPE
{
	VIM_EXP_UNKOWN=0,
	VIM_EXP_INDOOR=1,
	VIM_EXP_OUTDOOR,
	VIM_EXP_NIGHT
}VIM_EXP_TYPE;

typedef struct tag_VIM_USER_INITINFO
{
	VIM_BYPASS_DEF BypassDef;
	VIM_PANEINIT MainPanel;
	VIM_PANEINIT SubPanel;
	VIM_INTERRUPT InitInterrupt;
	VIM_GPIO InitGpio;
	VIM_CLK WorkMClk;
	VIM_TIMING WorkTiming;
}VIM_USER_INITINFO,*PVIM_USER_INITINFO;

UINT32 VIM_USER_ReadFile(HUGE void* pFile_Ptr,  HUGE UINT8* pbBuffer, UINT32 dFile_Size);
VIM_RESULT VIM_USER_SeekFile(HUGE void* pFile_Ptr, UINT32 dOffset);
VIM_RESULT VIM_USER_WriteFile(HUGE void* pFile_Ptr, const HUGE  UINT8* pbBuffer, UINT32 dFile_Size);
HUGE void* VIM_USER_MallocMemory(UINT32 dwbyte);
VIM_RESULT VIM_USER_FreeMemory(HUGE void* mem);
void	VIM_USER_MemSet(HUGE UINT8 *pbDest,  UINT8 bValue, UINT32 dCount);
void	VIM_USER_MemCpy(HUGE UINT8 *pbDest, const HUGE UINT8 *pbSrc, UINT32 dCount);
void VIM_USER_PrintHex(char *string,UINT32 data);
void VIM_USER_PrintDec(char *string,UINT32 data);
void VIM_USER_PrintString(char *string);
void VIM_USER_DelayMs(UINT32 ms);
extern const VIM_USER_INITINFO g_UserInfo;
UINT8 VIM_USER_CheckBigendian(void);
VIM_RESULT VIM_USER_StartTimer(UINT32 Intervel);
VIM_RESULT VIM_USER_StopTimer(void);
void VIM_USER_Reset(void);
#ifdef __cplusplus
}
#endif

#endif 
