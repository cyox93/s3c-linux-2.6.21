/*************************************************************************
*                                                                       
*                Copyright (C) 2005 Vimicro CO.,LTD     		 
*
* FILE NAME                                  VERSION                
*                                                                       
*  VIM_5X_API.c			   0.1                       
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*     VC0528 's to 568 USER aip file                 
*
*	Version		Author			Date		Description		
*  ---------------------------------------------------------------						 
*   0.1			angela		2006-10-31	The first version. 
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
#ifndef _VIM_5X_API_H_
#define _VIM_5X_API_H_

#ifdef __cplusplus
extern "C" {
#endif
//#define BOTH_5XSUPPORT



#ifndef __INFINEON_PLATFORM__
#define __INFINEON_PLATFORM__		0
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
#endif



#ifndef SINT8
#define SINT8 char
#endif

//unsigned 8-bit integer
#ifndef UINT8
#define UINT8 unsigned char
#endif

//signed 16-bit integer
#ifndef SINT16
#define SINT16 short
#endif

//unsigned 16-bit integer
#ifndef UINT16
#define UINT16 unsigned short
#endif

//signed 32-bit integer
#ifndef SINT32
#define SINT32 long
#endif

//unsigned 32-bit integer
#ifndef UINT32
#define UINT32 unsigned long
#endif

#ifndef INT32
#define INT32 long
#endif

#ifndef INT16
#define INT16 short
#endif

#ifndef BOOL
#define BOOL unsigned char
#endif



 typedef enum _V5H_CALLBACK_STATUS
{
	V5H_CAPTURE_END=0,
	V5H_FIFO_FULL,
	V5H_ONEFRAME_END,
	V5H_BUF_ERROR,
	V5H_UNKNOWN_ERROR
}V5H_CALLBACK_STATUS;

 typedef enum _V5H_FUNCTION_RETURN
{
	V5H_SUCCEED=0,
	V5H_ERROR_PARAMETER,
	V5H_ERROR_WORKMODE,
	V5H_ERROR_PREVIEWMODE,
	V5H_ERROR_BUFLENS,	
	V5H_ERROR_NOINITERRUPT,
	V5H_ERROR_OVERLIMIT,
	V5H_ERROR_NOPIC_INBUF,
	V5H_ERROR_TIME_OUT,
	V5H_FAILED
}V5H_FUNCTION_RETURN;

 typedef enum _V5H_EXP_TYPE
{
	V5H_EXP_UNKOWN=0,
	V5H_EXP_INDOOR=1,
	V5H_EXP_OUTDOOR,
	V5H_EXP_NIGHT
}V5H_EXP_TYPE;



typedef enum _V5H_DECODE_MODE

{
         DECODE_WITHFRAME_TO_YUV=0,
         DECODE_RESIZE_TO_YUV=1,
         DECODE_WITHFRAME_TO_RGB=2,
         DECODE_RESIZE_TO_RGB
        // DECODE_RESIZE_TO_YUV_PART,
       //  DECODE_RESIZE_TO_RGB_PART
         
}V5H_DECODE_MODE;

typedef enum _V5H_TCL_EXP_MODE
{
	V5H_EXP_TCL_INDOOR=0,
	V5H_EXP_TCL_LOWLIGHT=1,
	V5H_EXP_TCL_OUTDOOR=2
	
}V5H_TCL_EXP_MODE;

typedef enum _V5H_EXP_MODE
{
	V5H_EXP_AUTO=0,
	V5H_EXP_50HZ=1,
	V5H_EXP_60HZ=2
	
} V5H_EXP_MODE;

typedef enum _V5H_INTERRUPT_TYPE
{
    	V5H_INT_LEVER_LOW=1,
       V5H_INT_LEVER_HIGH,
       V5H_INT_ADGE_DOWN,
       V5H_INT_ADGE_UP
}V5H_INTERRUPT_TYPE;

typedef enum _V5H_BUS_TYPE
{
	V5H_BUSTYPE_SEP=1,
	V5H_BUSTYPE_8BIT,
	V5H_BUSTYPE_16BIT
}V5H_BUS_TYPE;
typedef enum _V5H_BYPASS_TYPE
{
	V5H_BYPASS_BYGPIO,
	V5H_BYPASS_BYREG=1

}V5H_BYPASS_TYPE;
typedef enum _V5H_BYPASS_SUBPANELCTL
{
	V5H_BYPASS_SUB_CS2,
	V5H_BYPASS_SUB_CS1

}V5H_BYPASS_SUBPANELCTL;

 typedef enum _V5H_CPTURE_QUALITY
{
	V5H_CPTURE_RATIO_MODE=0,
	V5H_CPTURE_QUALITY_LOW=20,
	V5H_CPTURE_QUALITY_MID=14,
	V5H_CPTURE_QUALITY_HIGH=10,
	V5H_CPTURE_QUALITY_PERFECT=7
	
}V5H_CPTURE_QUALITY;
 typedef enum _V5H_SENSOR_MIMODE
{
	V5H_SENSOR_NORMAL=0x03,
	V5H_SENSOR_MIRROR=1,
	V5H_SENSOR_FLIP=2,
	V5H_SENSOR_ROTATION=0
}V5H_SENSOR_MIMODE;
typedef enum _V5H_WORK_MODE
{
	V5H_MODE_CAMERAON=1,
	V5H_MODE_BYPASS,
	V5H_MODE_PHOTOPROCESS,
	V5H_MODE_DIRECTDISPLAY
}V5H_WORK_MODE;


 typedef enum _V5H_PREVIEW_MODE
{
	V5H_PREVIEW_OFF=0,
	V5H_PREVIEW_ON,	
	V5H_PREVIEW_FRAMEON
}V5H_PREVIEW_MODE;

typedef enum _V5H_BRIGHT_STEP
{
	V5H_BRIGHT_STEP_ONE=1,
	V5H_BRIGHT_STEP_TWO,
	V5H_BRIGHT_STEP_THREE,
	V5H_BRIGHT_STEP_MAX=3
}V5H_BRIGHT_STEP;

typedef enum _V5H_CONTRAST_STEP
{
	V5H_CONTRAST_STEP_ONE=1,
	V5H_CONTRAST_STEP_TWO,
	V5H_CONTRAST_STEP_THREE,
	V5H_CONTRAST_STEP_MAX=3
}V5H_CONTRAST_STEP;

typedef enum _V5H_SHARPNESS_STEP
{

	V5H_SHARPNESS_STEP_ONE,
	V5H_SHARPNESS_STEP_TWO,
	V5H_SHARPNESS_STEP_THREE,
	V5H_SHARPNESS_STEP_MAX=3
}V5H_SHARPNESS_STEP;



typedef enum _V5H_SPECIAL_EFFECT
{
	V5H_SPECIAL_NORMAL=0,
	V5H_SPECIAL_MONOCHROME,
	V5H_SPECIAL_SEPIA,
	V5H_SPECIAL_NEGATIVE, 
	V5H_SPECIAL_RELIEF, 
	V5H_SPECIAL_SKETCH,
	V5H_SPECIAL_REDONLY
}V5H_SPECIAL_EFFECT;

typedef enum _V5H_CHANGE_MODE
{
	V5H_CHANGE_NORMAL=0,
	V5H_CHANGE_ADD,
	V5H_CHANGE_DEC,
	V5H_SET_STEP
}V5H_CHANGE_MODE;

typedef enum _V5H_ZOOM_STEP
{
	V5H_ZOOM_STEPONE=1,
	V5H_ZOOM_STEPTWO=2,
	V5H_ZOOM_STEPTHREE=3,
	V5H_ZOOM_FOUR=4
}V5H_ZOOM_STEP;



typedef enum _V5H_LCD_ONOFF
{
	V5H_LCDMODE_OFF=0,
	V5H_LCDMODE_ON
}V5H_LCD_ONOFF;

typedef enum _V5H_LCD_PANEL
{
	V5H_LCDPANEL_MAIN=0,
	V5H_LCDPANEL_SUB
}V5H_LCD_PANEL;

 typedef enum _V5H_COLORDEP_MODE
{
	V5H_COLORDEP_24BIT=4,
	V5H_COLORDEP_15BIT,
	V5H_COLORDEP_16BIT
	
}V5H_COLORDEP_MODE;

 typedef enum _V5H_LCD_WORKMODE
{
	V5H_LCDMODE_AFIRST=0,
	V5H_LCDMODE_BLONLY,
	V5H_LCDMODE_OVERLAY,
	V5H_LCDMODE_BLEND
}V5H_LCD_WORKMODE;

typedef enum _V5H_LCD_MIMODE
{
	V5H_LCDMI_NORMAL=0,
	V5H_LCDMI_MIRROR=6, 
	V5H_LCDMI_FLIP=4,  //180+mirror
	V5H_LCDMI_ROTATION180=2
}V5H_LCD_MIMODE;

typedef enum _V5H_LCD_LAYER
{
	V5H_LCD_ALAYER=0,
	V5H_LCD_BLAYER,
	V5H_LCD_ALL
}V5H_LCD_LAYER;
typedef enum _V5H_TOOL_YUVTORGB
{
	V5H_CON_UYVY_565=0,
	V5H_CON_YUYV_565,
	V5H_CON_YYUV_565
}V5H_TOOL_YUVTORGB;
typedef void (*V5H_CallBack)(UINT8 Status, UINT32 Byte_Length);



UINT8 V5H_Init_568(void);
UINT8 V5H_Test_Register (void);
UINT8 V5H_IntHandle(void);
UINT8 V5H_CaculateVideoFrame(void);

UINT8 V5H_WorkMode_Set(UINT8 WorkMode);
UINT8 V5H_WorkMode_Get(void);


UINT8 V5H_Preview_MODE(UINT8 MODE);
UINT8 V5H_Preview_ExposureTYPE(UINT8 TYPE);
UINT8 V5H_Preview_ExposureMode(UINT8 MODE);
UINT8 V5H_Preview_Brightness(UINT8 Mode,UINT8 Step);
UINT8 V5H_Preview_Contrast(UINT8 Mode,UINT8 Step);
UINT8 V5H_Preview_Sharpness(UINT8 Step);
UINT8 V5H_Preview_Mirror(UINT8 mode);
UINT8 V5H_Preview_Effect(UINT8 Mode);
UINT8 V5H_Preview_GetMirror(void);
UINT8 V5H_Preview_Zoom(UINT8 Mode,UINT8 step);

UINT8 V5H_Preview_Parameter(UINT16 Lcd_OffsetW,UINT16 Lcd_OffsetH,UINT16 Width, UINT16 Height);		//have changed

UINT8 V5H_Capture_MultiFrame_Start(UINT8 *VideoBuf, UINT32 Buflength);
UINT8  V5H_Capture_MultiFrame_GetOne(UINT8 *VideoBuf, UINT32 Buflength,UINT32* len);
UINT32 V5H_Capture_MultiFrame_Stop(void);
UINT8 V5H_Capture_Parameter(UINT16 CapWidth, UINT16 CapHeight,UINT16 ThumWidth);
UINT8 V5H_Capture_CompressionRatio(UINT8 ratio);
UINT8 V5H_Capture_Quality(UINT8 Quality);
UINT8 V5H_Capture_Still(HUGE UINT8 *StillBuf,UINT32 BUF_Length,V5H_CallBack pUCallBack);
UINT8 V5H_Capture_ThumbStill( HUGE UINT8*   Thumbuf, HUGE UINT8*StillBuf, UINT32 StillBuf_Length,V5H_CallBack pUCallBack);
UINT8 V5H_Capture_Mjpeg(UINT16 Framedrop, HUGE UINT8 *VideoBuf, UINT32 Buflength, V5H_CallBack pUCallBack);
UINT8 V5H_Capture_AVI(UINT16 interval,UINT16 TotalFrame,HUGE UINT8 *VideoBuf, UINT32 Buflength, V5H_CallBack pUCallBack);
UINT8 V5H_Set_AVI_TotalTime(UINT32 Time_Ms);
UINT8 V5H_Capture_MULTI(UINT8 FrameCount, UINT16 Framedrop, HUGE UINT8 *jpegBuf, UINT32 Buflength, V5H_CallBack pUCallBack);
UINT8 V5H_Capture_Stop(void);
UINT32 V5H_Capture_GetLength(void);
UINT32 V5H_Capture_GetFrmCount(void);
UINT8* V5H_Capture_GetMultishotFrame(UINT8 *framebuf, UINT32 buflength, UINT8 framenumber, UINT32 *pframelength);
UINT8 V5H_Capture_GetThumbRealSize(UINT16 *Width,UINT16 *Heigth);


UINT8 V5H_Encode_Jpeg(HUGE UINT8 *pYUV, HUGE UINT8 *pDest,UINT32 YuvLength, UINT32 JpgLength,UINT16 Source_Width,UINT16 Source_height);
UINT8 V5H_Decode_Jpeg(UINT8 DecodeMode,HUGE UINT8 *jpegBuf, HUGE UINT8 *pDest,UINT32 jpeglength,UINT32 Destlength,UINT16 Target_Width,UINT16 Target_Height);
UINT8 V5H_Display_Jpeg(HUGE UINT8 *jpegBuf, UINT32 length,UINT16 Lcd_OffsetX, UINT16 Lcd_OffsetY, UINT16 Display_W, UINT16 Display_H);
UINT8 V5H_Display_Mjpeg(HUGE UINT8* VideoBuf , UINT32 buflen,UINT32 Delay_ms,UINT16 Lcd_OffsetX, UINT16 Lcd_OffsetY, UINT16 Display_W, UINT16 Display_H);
UINT8 V5H_DisplayJpeg_Zoom(UINT8 ZoomStep,UINT16 Jpeg_OffsetX, UINT16 Jpeg_OffsetY,UINT16 Lcd_OffsetX, UINT16 Lcd_OffsetY, UINT16 Lcd_Display_W, UINT16 Lcd_Display_H);
UINT8 V5H_DisplayJpeg_Pan( HUGE UINT8 *pbyJpegBuf, UINT32 dwBufLength, UINT16 Jpeg_OffsetX, UINT16 Jpeg_OffsetY, UINT16 Lcd_OffsetX, UINT16 Lcd_OffsetY, UINT16 Lcd_Display_W, UINT16 Lcd_Display_H);
UINT8 V5H_DisplayBypass_Jpeg_init(UINT16 Lcd_OffsetX, UINT16 Lcd_OffsetY, UINT16 Display_W, UINT16 Display_H);
void V5H_DisplayBypass_Jpeg(HUGE UINT8 *YUV422Buf, UINT32 length);
UINT8 V5H_Display_Vmi(HUGE UINT8 *jpegBuf, UINT32 length,UINT16 Lcd_OffsetX, UINT16 Lcd_OffsetY, UINT16 Display_W, UINT16 Display_H);
void V5H_VMI_End(void);
void V5H_VMI_init(void);

UINT8 V5H_LCD_Update(UINT16 startx, UINT16 starty,UINT16 Width,UINT16 Height);
UINT8 V5H_LCD_Rctngl( UINT16 startx, UINT16 starty,UINT16 Width,UINT16 Height,UINT16*uspImage);
UINT8 V5H_LCD_WorkMode(UINT8 Mode,UINT16 Value);
UINT8 V5H_LCD_GetWorkMode(void);
UINT8 V5H_LCD_Colordep (UINT8 Colordep);
UINT8 V5H_LCD_Panel(UINT8 Panel);
UINT8 V5H_LCD_PureColor(UINT16 startx, UINT16 starty,UINT16 Width,UINT16 Height,UINT16 color);
UINT8 V5H_LCD_Mirror(UINT8 MODE,UINT8 LAYER);

UINT8 V5H_Tool_YuvToRgb(HUGE UINT8 *pYUV, HUGE UINT8 *pRgb,UINT16 Source_Width,UINT16 Source_height,UINT8 Method);
UINT8 V5H_Tool_GetJpegSize(UINT8 *ptr, UINT32 length, UINT16* SizeX,UINT16* SizeY);
UINT8  V5H_Capture_GetOneFrame(UINT8 *VideoBuf,UINT32* len,UINT32 BufLength);
UINT8 V5H_SetLCDSize(UINT16 wStartx,UINT16 wStarty,UINT16 Width,UINT16 Height);


#ifdef  BOTH_5XSUPPORT

UINT16 V5S_HAPI_InitVc05x(void);
UINT16 V5S_HAPI_SetWorkMode(UINT8 WorkMode);
UINT16 V5S_HAPI_GetWorkMode(void);
UINT16 V5S_HAPI_SetCaptureParameter(UINT16 CapWidth, UINT16 CapHeight,UINT16 ThumWidth);
UINT16 V5S_HAPI_SetPreviewParameter(UINT16 Lcd_OffsetW,UINT16 Lcd_OffsetH,UINT16 Width, UINT16 Height);
UINT16 V5S_HAPI_SetPreviewMode(UINT8 MODE);
UINT16 V5S_HAPI_Preview_Zoom(UINT8 Mode,UINT8 step);
UINT16 V5S_HAPI_SetCaptureQuality(UINT8 Quality);
UINT16 V5S_HAPI_SetCaptureRatio(UINT8 ratio);
UINT16 V5S_HAPI_CaptureStill(HUGE UINT8* StillBuf,UINT32 BUF_Length,V5H_CallBack pUCallBack);
UINT32 V5S_Capture_GetFrmCount(void);
UINT8 V5S_HAPI_Capture_Mjpeg(UINT16 interval, HUGE UINT8 *VideoBuf, UINT32 Buflength, V5H_CallBack pUCallBack);
UINT16 V5S_HAPI_CaptureThumbStill(HUGE UINT8 *ThumbBuf,HUGE UINT8 *StillBuf,UINT32 BUF_Length,V5H_CallBack pUCallBack);
UINT16 V5S_HAPI_StartCaptureVideo(HUGE UINT8 *StillBuf,UINT32 BUF_Length);

UINT8 V5S_Capture_MULTI(UINT8 FrameCount, UINT16 Framedrop, HUGE UINT8 *jpegBuf, UINT32 Buflength, V5H_CallBack pUCallBack);
UINT16 V5S_HAPI_GetOneJpeg(HUGE UINT8 *StillBuf,UINT32 BUF_Length,UINT32 *dwOneLen);
UINT8 V5S_HAPI_StopCapture(void);
UINT32 V5S_HAPI_GetCaptureLength(void);
UINT16 V5S_HAPI_SetLCDWorkMode(UINT8 byABLayerMode, UINT16 wValue);
UINT16 V5S_HAPI_DrawLCDRctngl(UINT16 startx, UINT16 starty,UINT16 Width,UINT16 Height, HUGE  UINT16 *pwImage);
UINT16 V5S_HAPI_UpdateLCD(UINT16 startx, UINT16 starty,UINT16 Width,UINT16 Height);

UINT16 V5S_HAPI_SelectLCDPanel(UINT8 byPanel);
UINT16 V5S_HAPI_DrawLCDPureColor(UINT16 startx, UINT16 starty,UINT16 Width,UINT16 Height,UINT16 wRGBColor);
UINT16 V5S_HAPI_SetLCDColordep(UINT8 byColorDepth);
UINT16 V5S_HAPI_LCDMirror(UINT8 MirrorFlipMode, UINT8 Layer);
UINT16 V5S_HAPI_GetPictureSize(UINT8 *ptr, UINT32 length, UINT16* SizeX,UINT16* SizeY);

UINT16 V5S_HAPI_DisplayVideo(HUGE UINT8* VideoBuf , UINT32 buflen,UINT32 Delay_ms,UINT16 Lcd_OffsetX, UINT16 Lcd_OffsetY, UINT16 Display_W, UINT16 Display_H);

UINT16 V5S_HAPI_Display_Jpeg(HUGE UINT8 *jpegBuf, UINT32 length,UINT16 LcdOffsetX, UINT16 LcdOffsetY, UINT16 DisplayWidth, UINT16 DisplayHeight);
UINT16 V5S_HAPI_DisplayPan(HUGE UINT8 *pbyJpegBuf, UINT32 dwBufLength,UINT16 LcdOffsetX, UINT16 LcdOffsetY, UINT16 JpegOffsetX, UINT16 JpegOffsetY,UINT16 DisplayWidth, UINT16 DisplayHeight);
UINT16 V5S_HAPI_Decode_Jpeg(UINT8 DecodeMode,HUGE void *jpegBuf, HUGE UINT8 *pDest,UINT32 jpeglength,UINT32 Destlength,UINT16 TargetWidth,UINT16 TargetHeight);
UINT16 V5S_HAPI_YuvToRgb(HUGE UINT8 *pYUV, HUGE UINT8 *pRgb,UINT16 Source_Width,UINT16 Source_height,UINT8 Method);

UINT16 V5S_HAPI_Preview_Brightness(UINT8 Mode,UINT8 Step);
UINT16 V5S_HAPI_Preview_Contrast(UINT8 Mode,UINT8 Step);
UINT16 V5S_HAPI_Preview_Mirror(UINT8 mode);
UINT16 V5S_HAPI_Preview_Effect(UINT8 Mode);
UINT8* V5S_Capture_GetMultishotFrame(UINT8 *framebuf, UINT32 buflength, UINT8 framenumber, UINT32 *pframelength);
UINT8 V5S_HAPI_Capture_AVI(UINT16 interval,UINT16 TotalFrame,UINT8 *VideoBuf, UINT32 Buflength, V5H_CallBack pUCallBack);
UINT8 V5S_HAPI_Timer(void);
UINT16  V5S_Capture_GetOneFrame(UINT8 *VideoBuf,UINT32* len,UINT32 BufLength);
UINT16 V5S_HAPI_SetLCDSize(UINT16 wStartx,UINT16 wStarty,UINT16 Width,UINT16 Height);

#endif















#ifdef __cplusplus
}
#endif

#endif /* _RDK_HIF_H_ */
