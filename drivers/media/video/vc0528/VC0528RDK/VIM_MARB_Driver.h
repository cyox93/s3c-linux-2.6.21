/*************************************************************************
*                                                                       
*                Copyright (C) 2005 Vimicro CO.,LTD     		 
*
* FILE NAME                                  VERSION                
*                                                                       
* VIM_MARB_Driver.h			   0.2                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*     VC0578 's marb moudle sub driver  head file                               
*
*	Version		Author			Date		Description		
*  ---------------------------------------------------------------						 
*   0.1			angela		2005-11-3	The first version. 
*   0.2			angela		2006-06-09	update for 528
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
#ifndef _VIM_5XRDK_MARB_H_
#define _VIM_5XRDK_MARB_H_

#ifdef __cplusplus
extern "C" {
#endif

#define VIM_MARB_JBUF_MINSIZE	0X500
#define VIM_MARB_JBUF_MAXBLOCK	10
#define   VIM_MARB_JBUF_GAPINTERVAL	40

 typedef enum _VIM_MARB_SUB_RESET
 {
	  //reset marb_ipp module
	  VIM_MARB_SUB_RESET_IPP     = BIT0, 
	  //reset marb_jpeg module
	  VIM_MARB_SUB_RESET_JPEG     = BIT1, 
	  //reset 1T-SRAM 0
	  VIM_MARB_SUB_RESET_1TSRAM0    = BIT2, 
	  //reset 1T-SRAM 1
	  VIM_MARB_SUB_RESET_1TSRAM1    = BIT3, 
	  //reset 1T-SRAM 2
	  VIM_MARB_SUB_RESET_1TSRAM2    = BIT4, 
	//set 0 to all bit
	  VIM_MARB_NO_RESET  =  0, 
	  //set ALL reset
	  VIM_MARB_RESET_ALL=(BIT0|BIT1|BIT2|BIT3|BIT4)
 }VIM_MARB_SUB_RESET;

typedef enum _VIM_MARB_INT_TYPE
{
	MARB_INT_JBUF_FIFOCNT		= 0,
	MARB_INT_JBUF_INTERV		= 1, //ignore
	MARB_INT_JPEG_DONE			= 2,
	MARB_INT_JBUF_ERR			= 3,
	MARB_INT_TBUF_FIFOCNT		= 4,
	MARB_INT_TBUF_INTERV	= 5, //ignore
	MARB_INT_THUMB_DONE		= 6,
	MARB_INT_THUMB_ERR			= 7,
	MARB_INT_MEM0_REF_ERR		= 8,
	MARB_INT_MEM1_REF_ERR		= 9, //ignore
	MARB_INT_MEM2_REF_ERR		= 10,
	MARB_INT_RECAP_ERR			= 11,
	MARB_INT_RECAP				= 12,
	MARB_INT_TIMER				=13,
	MARB_INT_ALL			= VIM_MARB_INT_NUM
}VIM_MARB_INT_TYPE;
#define MARB_SLV_INT_ALL 6

typedef enum _VIM_MARB_WORKMODE {
	// Preview modes
	VIM_MARB_PREVIEW_MODE, // Auto Bit Rate Control enable

	// Capture modes
	VIM_MARB_CAPTURESTILL_MODE,
	VIM_MARB_CAPTURESTILLTHUMB_MODE,
	VIM_MARB_CAPTUREVIDEO_MODE,
	VIM_MARB_CAPTUREAVI_MODE,	//include header and index

	VIM_MARB_MULTISHOT_MODE,
	VIM_MARB_MULTISHOTTHUMB_MODE,

	// Encode modes
	VIM_MARB_ENCODE_MODE,

	// Display modes
	VIM_MARB_DISPLAYSTILL_MODE,
	VIM_MARB_DISPLAYSTILLBYPASS_MODE,
	VIM_MARB_DISPLAYVIDEO_MODE,

	// Decode modes
	VIM_MARB_DECODEYUV_MODE, // Without IPP
	VIM_MARB_DECODERGB_MODE,

	// Special modes
	VIM_MARB_DIRECTDISPLAY_MODE,
	VIM_MARB_BYPASS_MODE
}VIM_MARB_WORKMODE;

 typedef enum _VIM_MARB_INTERVAL_INTERRUPT_MODE
 {
	VIM_MARB_INTJBUF_BY_LENGTH=0,
	VIM_MARB_INTJBUF_BY_FRAME=1
 }VIM_MARB_INTERVAL_INTERRUPT_MODE;
  typedef enum _VIM_MARB_1TSRAM_NUM
 {
	VIM_MARB_1TSRAM_0=BIT0,
	VIM_MARB_1TSRAM_1=BIT1,
	VIM_MARB_1TSRAM_2=BIT2,
	VIM_MARB_1TSRAM_1AND2=(BIT2|BIT1),
	VIM_MARB_1TSRAM_0AND1=(BIT0|BIT1),
	VIM_MARB_1TSRAM_ALL=(BIT2|BIT1|BIT0)
 }VIM_MARB_1TSRAM_NUM;
   typedef enum _VIM_MARB_1TSRAM_MODE
 {
	VIM_MARB_1TSRAM_POWERON=0,
	VIM_MARB_1TSRAM_STANDBY=1,
	VIM_MARB_1TSRAM_OFF=2
 }VIM_MARB_1TSRAM_MODE;
 typedef enum _VIM_MARB_STATUS
 {
	VIM_MARB_JBUF_UPTHRESHOLD=BIT0,
	VIM_MARB_JBUF_DOWN=BIT2,
	VIM_MARB_JBUF_OVERLOW=BIT3
}VIM_MARB_STATUS;

typedef struct tag_VIM_MARB_Map {
		UINT32	jbufnowblock_int;			// 2, 3
		UINT32	jbufstart, jbufsize;			// 2, 3
		UINT32	lbuf0start;				//4//4
		UINT32	lbuf1start;				//5
		UINT32  lbufend;				//6
		UINT32	thumbstart, thumbsize;			//7,  8
		UINT32	indexstart, indexsize;			//9, 10
		UINT32	layerA0start, layerA0end;		//13, 14
		UINT32	layerA1start, layerA1end;		//15, 16
		UINT32	layerB0start, layerB0end;		//0, 1
		UINT32	layerB1start, layerB1end;		//0, 1
}VIM_MARB_Map, *PVIM_MARB_Map;



//  capture along
void VIM_MARB_StartCapture(void);
void VIM_MARB_StopCapture(void);
void VIM_MARB_StartDecode(void);
void VIM_MARB_SetMultiFrameCount(UINT8 FramCount);
void VIM_MARB_SetRecaptureInfo(UINT8 MaxTime,UINT32 MaxLength);
void VIM_MARB_SetRecaptureEn(BOOL Enable);
UINT8 VIM_MARB_GetRecaptureTime(void);
//  marb moudul 
UINT8 VIM_MARB_GetStatus(void);
void VIM_MARB_SetMode(VIM_MARB_WORKMODE bMode);
void VIM_MARB_ResetSubModule(VIM_MARB_SUB_RESET Module);
void VIM_MARB_Set1TSramMode(VIM_MARB_1TSRAM_NUM Num,VIM_MARB_1TSRAM_MODE Mode);
//timer
void VIM_MARB_RefreshTimerVal(void);
void VIM_MARB_StopVideoByTimerEn(BOOL Enable);
void VIM_MARB_ResetTimerAutoEn(BOOL Enable);
void VIM_MARB_ResetSubModuleAutoEn(BOOL Enable);
void VIM_MARB_SetTimerMs(UINT32 Ms,UINT32 mclk);
void VIM_MARB_StartTimer(void);
void VIM_MARB_StopTimer(void);
// jbuf read
void VIM_MARB_SetJbufInterval(VIM_MARB_INTERVAL_INTERRUPT_MODE Mode,UINT32 Length);
void VIM_MARB_SetJbufUpThreshold(UINT32 Length);
void VIM_MARB_SetJbufLowThreshold(UINT32 Length);
UINT32 VIM_MARB_GetJbufChipPointer(void);
UINT32 VIM_MARB_GetThumbBufChipPointer(void);
UINT32 VIM_MARB_GetJpegChipLastPointer(void);
UINT32 VIM_MARB_GetNowCapFrameCount(void) ;
void VIM_MARB_SetJpegbufGapratio(UINT8 ratio);
void VIM_MARB_SetThumbbufGapratio(UINT8 ratio);
UINT32 VIM_MARB_GetJbufUpThreshold(void);

VIM_RESULT VIM_MARB_SetMap(VIM_MARB_WORKMODE bWorkMode,VIM_DISP_BUFFERMODER Aformat,PVIM_MARB_Map Maping);


UINT8 VIM_MARB_GetModeCtrl0(void);
UINT32 VIM_MARB_GetJbufStartAddr(void);
UINT32 VIM_MARB_GetJbufEndAddr(void);
UINT32 VIM_MARB_ReadBuf(HUGE UINT8 *pbuf, UINT32 len, UINT32 start, UINT32 end, UINT32 pos);
UINT32 VIM_MARB_WriteBuf(HUGE UINT8 *pbuf, UINT32 len, UINT32 start, UINT32 end, UINT32 pos);
UINT32 VIM_MARB_GetJbufRWSize(UINT32 Jbufpos);
UINT32 VIM_MARB_WriteJpegData(HUGE UINT8 *Start,UINT32 length, UINT32 pos,UINT8 end);


void _ISR_MarbIntHandle(UINT8 wIntFlag);
void _ISR_Marb1IntHandle(UINT8 wIntFlag);
void VIM_MARB_RegisterISR(VIM_MARB_INT_TYPE bySecondLevelInt, PVIM_Isr  Isr);
void VIM_MARB_SetIntEnable(VIM_MARB_INT_TYPE bySecondLevelInt,BOOL Eanble);
UINT32 VIM_MARB_ReadJpegData(HUGE UINT8 *Start, UINT32 length, UINT32 pos);
UINT32 VIM_MARB_ReadThumbData(HUGE UINT8 *Start, UINT32 length, UINT32 pos);

#ifdef __cplusplus
}
#endif

#endif /* _RDK_HIF_H_ */
