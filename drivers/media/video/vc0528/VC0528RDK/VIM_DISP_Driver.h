

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
*   0.2			angela  		2006-06-08	update for528
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

#ifndef _VIM_5XRDK_DISP_H_
#define _VIM_5XRDK_DISP_H_

#ifdef __cplusplus
extern "C" {
#endif

// set B layer is B0/B1/ALL
typedef enum _VIM_DISP_LAYER_enum
{
	VIM_DISP_ALAYER = 1,
	VIM_DISP_B0LAYER= 2,
	VIM_DISP_B1LAYER=4,
	VIM_DISP_ALLBLAYER=6,
	VIM_DISP_ALLLAYER=7
}VIM_DISP_LAYER;

// work mode
typedef enum VIM_DISP_WORKMODE
{
	VIM_DISP_AFIRST,
	VIM_DISP_BFIRST,
	VIM_DISP_B0_OVERLAY,
	VIM_DISP_B0_BLEND,
	VIM_DISP_ALL_OVERLAY,
	VIM_DISP_ALL_BLEND,
	VIM_DISP_DECODE
}VIM_DISP_WORKMODE;

// B layer Graphic buffer data format
typedef enum VIM_DISP_BFORMAT{
	VIM_DISP_BLAYER_RGB1=0,
	VIM_DISP_BLAYER_RGB2,
	VIM_DISP_BLAYER_RGB4,
	VIM_DISP_BLAYER_RGB8,
	VIM_DISP_BLAYER_RGB444,
	VIM_DISP_BLAYER_RGB555,
	VIM_DISP_BLAYER_RGB565,
	VIM_DISP_BLAYER_RGB666,
	VIM_DISP_BLAYER_RGB24,
	VIM_DISP_BLAYER_RGB32
}VIM_DISP_BFORMAT;

typedef enum _VIM_DISP_ROTATEMODE{
	VIM_DISP_ROTATE_0,
	VIM_DISP_ROTATE_90,
	VIM_DISP_ROTATE_180,
	VIM_DISP_ROTATE_270,
	VIM_DISP_MIRROR_0,
	VIM_DISP_MIRROR_90,
	VIM_DISP_MIRROR_180,
	VIM_DISP_MIRROR_270,
	VIM_DISP_NOTCHANGE
}VIM_DISP_ROTATEMODE;

typedef enum _VIM_DISP_BUFFERMODER{
	VIM_DISP_LINEBUF=0,
	VIM_DISP_NODISP=1,
	VIM_DISP_TWOFRAME= 2,
	VIM_DISP_ONEFRAME
}VIM_DISP_BUFFERMODER;

typedef enum _VIM_DISP_DITHERSEL{
	VIM_DISP_RGB444,
	VIM_DISP_RGB565,
	VIM_DISP_RGB666,
	VIM_DISP_RGB332
}VIM_DISP_DITHERSEL;

#define AM_MAXPIXEL	511

void VIM_IPP_GetWindowPos(UINT16 *wStart_X, UINT16 *wStart_Y);

VIM_RESULT VIM_DISP_SetWorkMode(VIM_DISP_WORKMODE bWorkMode);
VIM_RESULT VIM_DISP_SetDitherSelet(VIM_DISP_DITHERSEL byDitherSelect);
VIM_RESULT VIM_DISP_SetDitherSelet(VIM_DISP_DITHERSEL byDitherSelect);
VIM_RESULT VIM_DISP_SetBufferMode(VIM_DISP_BUFFERMODER byBufferMode);
VIM_DISP_BUFFERMODER VIM_DISP_GetBufferMode(void);
VIM_RESULT VIM_DISP_SetDitherEnable(BOOL Enable);
VIM_RESULT VIM_DISP_SetLayerEnable(VIM_DISP_LAYER bLayer,BOOL Enable);
VIM_RESULT VIM_DISP_SaveAramEnable(BOOL Enable);
void VIM_DISP_ResetState(void);
VIM_RESULT VIM_DISP_Update(VIM_DISP_LAYER BLayer);
VIM_RESULT VIM_DISP_SetRotateMode(VIM_DISP_LAYER Layer,VIM_DISP_ROTATEMODE RotateMode);
VIM_RESULT VIM_DISP_SetOverlayKeyColor(VIM_DISP_LAYER BLayer,UINT32 dwKeyColor);

VIM_RESULT VIM_DISP_SetBlendRatio(VIM_DISP_LAYER BLayer,UINT8 byBlendRatio);
VIM_RESULT VIM_DISP_SetGbufFormat(VIM_DISP_BFORMAT byGbufFmt);
void VIM_DISP_GetGbufFormat(VIM_DISP_BFORMAT * byGbufFmt);
/////////////////////////////////////////////////////////////////////
void VIM_DISP_GetAWinsize(PTSize winSize);
VIM_RESULT VIM_DISP_GetBSize(VIM_DISP_LAYER BLayer,PTSize pTSize);
VIM_RESULT VIM_DISP_SetA_Memory(TPoint memPoint,TSize memSize);
VIM_RESULT VIM_DISP_SetA_DisplayPanel(TPoint startPoint,TSize winSize);
VIM_RESULT VIM_DISP_SetB0_DisplayPanel(TPoint startPoint,TSize winSize);
VIM_RESULT VIM_DISP_SetB1_DisplayPanel(TPoint startPoint,TSize winSize);
VIM_RESULT VIM_DISP_SetB0_RefreshBlock(TPoint startPoint,TSize winSize);
VIM_RESULT VIM_DISP_SetB1_RefreshBlock(TPoint startPoint,TSize winSize);
VIM_RESULT VIM_DISP_GetBStartPoint(VIM_DISP_LAYER BLayer,PTPoint pStartPoint);
void VIM_DISP_EnableVrgbMode(UINT8 bEnable);
void VIM_DISP_EnableDecodeSpecial(UINT8 bEnable);
UINT8 VIM_DISP_GetDecodeSpecial(void);

#ifdef __cplusplus
}
#endif

#endif


