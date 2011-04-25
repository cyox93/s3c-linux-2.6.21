/*************************************************************************
*                                                                       
*                Copyright (C) 2005 Vimicro CO.,LTD     		 
*
* FILE NAME                                  VERSION                
*                                                                       
* VIM_IPP_Driver.h			   0.2                   
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*     5X 's IPP linebuf  moudle sub driver  head file                               
*
*	Version		Author			Date		Description		
*  ---------------------------------------------------------------						 
*   0.1			angela		2005-11-8	The first version. 
*   0.2			angela  		2006-06-09	update for528
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
#ifndef _VIM_5XRDK_IPP_H_
#define _VIM_5XRDK_IPP_H_

#ifdef __cplusplus
extern "C" {
#endif

#define VIM_IPP_DOWNTOUP_DELAY	200

typedef enum _VIM_IPP_MODE
{
	VIM_IPP_MODE_PREVIEW=0,
	VIM_IPP_MODE_CAPTURE,
	VIM_IPP_MODE_DISPLAY,
	VIM_IPP_MODE_DECODE,
	VIM_IPP_MODE_CAPTURE_WITHTHUMB
	
}VIM_IPP_MODE;
typedef enum _VIM_IPP_BYPPASS
{
	VIM_IPP_BYPASS_TUMB_H=BIT0,
	VIM_IPP_BYPASS_TUMB_W=BIT1,
	VIM_IPP_BYPASS_DIS_H=BIT2,
	VIM_IPP_BYPASS_DIS_W=BIT3,
	VIM_IPP_BYPASS_CAP_UP_H=BIT4,
	VIM_IPP_BYPASS_CAP_UP_W=BIT5,
	VIM_IPP_BYPASS_CAP_DOWN_H=BIT6,
	VIM_IPP_BYPASS_CAP_DOWN_W=BIT7
}VIM_IPP_BYPPASS;
typedef enum _VIM_IPP_CAPSIZE_MODE
{
	VIM_IPP_CAPMODE_DOWN= 0x4,
	VIM_IPP_CAPMODE_UP= 0x2
}VIM_IPP_CAPSIZE_MODE;

typedef enum _VIM_IPP_THUMB_SELECT
{
	VIM_IPP_THUMB_FROM_CAPTURE= 0x2,
	VIM_IPP_THUMB_FROM_DISPLAY= 0x1
}VIM_IPP_THUMB_SELECT;
typedef enum _VIM_IPP_EFFECT
{
	VIM_IPP_EFFECT_NORMAL = 0x0,
	VIM_IPP_EFFECT_SEPHIA = 0x1,
	VIM_IPP_EFFECT_SPECIALCOLOR = 0x3,
	VIM_IPP_EFFECT_NEGATIVE = 0x5,
	VIM_IPP_EFFECT_SKETCH = 0x7
}VIM_IPP_EFFECT;

typedef enum _VIM_IPP_HAVEFRAM
{
	VIM_IPP_HAVEFRAME = 0x1,
	VIM_IPP_HAVE_NOFRAME=0X0
}VIM_IPP_HAVEFRAM;

typedef struct tag_VIM_IPP_SzDnFct
{
	UINT8	w1;
	UINT8	w2;
	UINT16	s1;
	UINT16	s2;
	UINT8	c;
} VIM_IPP_SzDnFct, *PVIM_IPP_SzDnFct;

typedef struct tag_VIM_IPP_SzUpFct
{
	UINT8	w1;
	UINT8	w2;
	UINT16	s;
} VIM_IPP_SzUpFct, *PVIM_IPP_SzUpFct;

#define IPP_CSIZER_HIN_MAX		0x400
#define IPP_CSIZER_VIN_MAX		0x300
#define IPP_CSIZER_HOUT_MAX		0x800
#define IPP_CSIZER_VOUT_MAX		0x600

#define IPP_MAX_DISPLAYSIZER_Y		512
#define IPP_MAX_DISPLAYSIZER_X		1024

#define IPP_MAX_DISPLAYSIZER_RAM		320

void VIM_IPP_Reset(void);
void VIM_IPP_SetMode(VIM_IPP_MODE bMode,VIM_IPP_HAVEFRAM bHaveFrame);
void VIM_IPP_GetThumbSize(PTSize Size);
void VIM_IPP_GetCaptureWindowSize(PTSize Size);
void VIM_IPP_SetCaptureWindowSize(UINT16 wWidth, UINT16 wHeight);
void VIM_IPP_SetImageSize(UINT16 wWidth, UINT16 wHeight);
void VIM_IPP_GetImageSize(UINT16 *wWidth, UINT16 *wHeight);
void VIM_IPP_GetWindowSize(UINT16 *wWidth, UINT16 *wHeight);
void VIM_IPP_GetDispalyWindowSize(UINT16 *wWidth, UINT16 *wHeight);

VIM_RESULT VIM_IPP_SetDispalySize(TPoint pt, TSize winSize, TSize dispSize);
VIM_RESULT VIM_IPP_SetCaptureSize(TSnrInfo *pSensorInfo,TPoint pt, TSize winSize, TSize CaptureSize);

VIM_RESULT VIM_IPP_SetThumbSize(UINT8 bWidth, UINT8 bheight,VIM_IPP_THUMB_SELECT ThumbSelect);
void VIM_IPP_SetDisplayDropFrame(UINT16 wDropFrame);
void VIM_IPP_SetSizeBypassDisable(UINT8 bBypassbit);
void VIM_IPP_SetEffectUVoffset(UINT8 bUoffset,UINT8 bVoffset);
void VIM_IPP_SetEffectUVThreshold(UINT8 bU_up,UINT8 bU_down,UINT8 bV_up,UINT8 bV_down);
void VIM_JPEG_SetBitRateControlEn(BOOL bEnable);


// tools 
VIM_RESULT VIM_IPP_ToolCaculateLessSrcWindow(TSize src, TSize dst,TSize *win);
VIM_RESULT VIM_IPP_ToolCaculateBigDstWindow(TSize src, TSize dst,TSize *win);
VIM_RESULT VIM_IPP_ToolCaculateLessDisplayWindow(TSize src, TSize dst,TSize *dis);
#ifdef __cplusplus
}
#endif

#endif /* _RDK_HIF_H_ */
