/*************************************************************************
*                                                                       
*             Copyright (C) 2006 Vimicro CO.,LTD     		 
*
* FILE NAME                                  VERSION                
*                                                                       
* VIM_HIF_Driver.h				0.2                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*     VIMICRO 5X's Host Interface (HIF) module head file.                                 
*
*	Version		Author			Date		Description		
*  ---------------------------------------------------------------						 
*   0.1			maning		2005-11-2	The first version. 
*   0.2			angela  		2006-06-06	update for528
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

#ifndef _VIM_5XRDK_HIF_H_
#define _VIM_5XRDK_HIF_H_

#ifdef __cplusplus
extern "C" {
#endif



//---------------------------------------------------------------
// Define Typed
#define V5_MULTI8_REG_PORT			0xb0
#define V5_MULTI8_REG_WORDL		0xb2
#define V5_MULTI8_REG_WORDH		0xb4
#define V5_MULTI8_MEM_PORT		0xb6

#define V5_MULTI16_MEM_FLG		0x1850



#define VIM_MULTI8 0
#define VIM_MULTI16 1

//---------------------------------------------------------------

// Set Bypass or Normal Type
typedef enum _VIM_HIF_BYPASSMODE
{
	VIM_HIF_NORMALTYPE = 0x0,
	VIM_HIF_BYPASSTYE = 0x1
}VIM_HIF_BYPASSMODE;


typedef enum _VIM_HIF_DIRMODE
{
	VIM_PUTIN = 0x0,
	VIM_PUTOUT = 0x1
}VIM_HIF_DIRMODE;
// Set or Get Register
// Set Bypass or Normal Type
typedef enum _VIM_HIF_PLLMODE
{
	VIM_HIF_PLLPOWERON = 0x0,
	VIM_HIF_PLLPOWERDOWN = 0x1,
	VIM_HIF_PLLBYPASS=0X02
}VIM_HIF_PLLMODE;

//CpmModReset_CTRL
typedef enum _VIM_HIF_RESET
{
	VIM_HIF_RESET_ISP    = BIT8,
	VIM_HIF_RESET_SIF    = BIT7,                 
	VIM_HIF_RESET_IPP    = BIT6,
	VIM_HIF_RESET_LBUF   = BIT5,
	VIM_HIF_RESET_JPEG   = BIT4,
	VIM_HIF_RESET_MARB   = BIT2,
	VIM_HIF_RESET_LCDC   = BIT1,
	VIM_HIF_RESET_LCDIF  = BIT0,
	VIM_HIF_RESET_GLOBE  = 0xffff    //全部模块复位
}VIM_HIF_RESET;	

typedef enum _VIM_HIF_CLK_CTRL
{
		VIM_HIF_CLOCK_SIF			=BIT11,
		VIM_HIF_CLOCK_ISP			=BIT10,
		VIM_HIF_CLOCK_IPP    		=BIT9,
		VIM_HIF_CLOCK_LBUF 		=BIT8,
		VIM_HIF_CLOCK_JPEG     		=BIT7,
		VIM_HIF_CLOCK_JPEG_CORE	=BIT6,
		VIM_HIF_CLOCK_GE                	=BIT5,
		VIM_HIF_CLOCK_MARB          	=BIT4,
		VIM_HIF_CLOCK_MARB_FAST	=BIT3,
		VIM_HIF_CLOCK_LCDC            	=BIT2,
		VIM_HIF_CLOCK_LCDIF            =BIT1,
		VIM_HIF_CLOCK_CPM               =BIT0,
		VIM_HIF_CLOCK_ALL                =0xfff
}VIM_HIF_CLK_CTRL;
typedef enum _VIM_HIF_EXTPIN_CTRL
{
		VIM_HIF_PIN_INT    			=BIT9,
		VIM_HIF_PIN_LCDRES 		=BIT8,
		VIM_HIF_PIN_CSRSTN     		=BIT7,
		VIM_HIF_PIN_CSPWD			=BIT6,
		VIM_HIF_PIN_CSENB			=BIT5,
		VIM_HIF_PIN_GPIO4          	=BIT4,
		VIM_HIF_PIN_GPIO3			=BIT3,
		VIM_HIF_PIN_GPIO2            	=BIT2,
		VIM_HIF_PIN_GPIO1            	=BIT1,
		VIM_HIF_PIN_GPIO0               	=BIT0,
		VIM_HIF_PIN_ALL           	=0xfff
}VIM_HIF_EXTPIN_CTRL;

typedef struct tag_TAcceContextInt
{/*register and sram access context store function if they are interrupted in process*/
	UINT32              addr;
	UINT8               stat; 
} TAcceContext, *PTAcceContext;
///------------------------register---------------------
void	VIM_HIF_SetReg8(UINT32 adr, UINT8 val);
UINT8	VIM_HIF_GetReg8(UINT32 adr);
void	VIM_HIF_SetReg16(UINT32 adr,UINT16 val);
UINT16	VIM_HIF_GetReg16(UINT32 adr);
void	VIM_HIF_SetReg32(UINT32 adr,UINT32 val);
UINT32	VIM_HIF_GetReg32(UINT32 adr);
//----------------------------------------------------------------------------
// Sram
UINT32 VIM_HIF_ReadSram(UINT32 adr, HUGE UINT8 *buf, UINT32 size);
UINT32 VIM_HIF_WriteSram(UINT32 adr, const HUGE UINT8 *buf, UINT32 size);
UINT32 VIM_HIF_WriteSramReverse(UINT32 adr,  const HUGE UINT8 *buf, UINT32 size);
UINT32 VIM_HIF_WriteSramOnWord(UINT32 adr, UINT32 dat, UINT32 size);
//------------------------cofig ---------------------------------------
void VIM_HIF_SetMulti16(void);
void VIM_HIF_InitBypass(UINT8 byAddr,VIM_BYPASSTYPE byType);
void VIM_HIF_SetBypassOrNormal(PVIM_BYPASS_DEF pBypassInfo,VIM_HIF_BYPASSMODE byOrNormal);
VIM_RESULT VIM_HIF_InitPll(PVIM_CLK ClkInfo);
VIM_RESULT VIM_HIF_InitMarbRereshTime(PVIM_CLK ClkInfo);
void VIM_HIF_SetPllStatus(VIM_HIF_PLLMODE Mode);
void VIM_HIF_SetDelay(PVIM_TIMING Timing);
void VIM_HIF_SetGpioInfo(VIM_HIF_DIRMODE Mode,UINT8 Value);
void VIM_HIF_SetLcdCsMode(VIM_BYPASS_SUBPANELCTL Cs);
void VIM_HIF_SwitchPanelCs(UINT8 Panel);
void VIM_HIF_SetLcdCsActiveEn(BOOL Enable);
void VIM_HIF_SetLdoStatus(VIM_LDOMODE wVal);
void VIM_HIF_SetExterPinCrlEn(VIM_HIF_EXTPIN_CTRL byPin,BOOL Enable);
 void VIM_HIF_SetExterPinValue(VIM_HIF_EXTPIN_CTRL byPin,BOOL Value);
//----------------------------------------------------------------------------	
			
void VIM_HIF_ResetSubModule(VIM_HIF_RESET wVal);      //refer to the former enum data 

//---------------------------------------------------------------------------------
void   VIM_HIF_SetModClkOn(UINT16 wVal);    //模式控制的开关  refer to the former enum data 
void   VIM_HIF_SetModClkClose(UINT16 wVal);	
VIM_RESULT VIM_HIF_InitInerruptLevel(PVIM_INTERRUPT Interrupt);

//add by lhd for interrupt

	#define VIM_INT_NUM  8
typedef enum _VIM_HIF_INT_TYPE
{
		//SIF
		INT_SIF		= 0,
		//ISP
		INT_ISP		= 1,
		//LBUF
		INT_LBUF	= 2,
		//JPEG
		INT_JPEG	= 3,
		//GE
		INT_GE		= 4,
		//MARB
		INT_MARB	= 5,
		//LCDC
		INT_LCDC	= 6,
		// MARB ADD
		INT_MARBADD	= 7,
		//ALL
		INT_ALL		= VIM_INT_NUM
}VIM_HIF_INT_TYPE;

	
typedef void (*PVIM_Isr)(void);
// First level interrupt number
#define VIM_FIRST_LEVEL_INT_NUM 	8// angela changed it
// Marb int
#define VIM_MARB_INT_NUM 			14 // angela changed it
// Marb int OLD 8个+addition marb interrupt 6个
// JPEG int
#define VIM_JPEG_JPEG_INT_NUM 		4 //maybe 16 in future
//LBUF int
#define VIM_JPEG_LBUF_INT_NUM 		1 //maybe 16 in future


typedef struct tag_VIM_HIF_TISR{
	UINT8	byFirstLevelInt[VIM_FIRST_LEVEL_INT_NUM]; /* First level (module level) interrupt numbers */
	UINT8	bSecondLevelIntFlag[VIM_FIRST_LEVEL_INT_NUM]; /* The interrupt flag of first level interrupt */
	PVIM_Isr     marbisr[VIM_MARB_INT_NUM]; /* Marb module's ISRs */
	PVIM_Isr     jpegisr[VIM_JPEG_JPEG_INT_NUM]; /* Jpeg module's ISRs */
} VIM_HIF_TISR, *PVIM_HIF_TISR;


void VIM_HIF_SetIntModuleEn(VIM_HIF_INT_TYPE byIntEnEnum,BOOL Enable);
UINT8 VIM_HIF_GetIntEnableSec(VIM_HIF_INT_TYPE byOffset);
void VIM_HIF_SetIntEnableSec(VIM_HIF_INT_TYPE byOffset, UINT8 uVal);
void _ISR_HIF_IntHandle(void);
 void VIM_HIF_ClearIntModule(VIM_HIF_INT_TYPE byIntEnEnum);

//---------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* _RDK_HIF_H_ */
