/*************************************************************************
*                                                                       
*                Copyright (C) 2005 Vimicro CO.,LTD     		 
*
* FILE NAME                                  VERSION                
*                                                                       
*  VIM_HIGH_API.h		   0.1                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*     VC0528 's HIGH aip head file                          
*
*	Version		Author			Date		Description		
*  ---------------------------------------------------------------						 
*   0.1			angela		2005-11-15	The first version. 
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
#ifndef _VIM_5XRDK_HAPI_H_
#define _VIM_5XRDK_HAPI_H_

#ifdef __cplusplus
extern "C" {
#endif


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

typedef enum _VIM_HAPI_WORKMODE
{
	VIM_HAPI_MODE_CAMERAON=1,
	VIM_HAPI_MODE_BYPASS,
	VIM_HAPI_MODE_PHOTOPROCESS,
	VIM_HAPI_MODE_DIRECTDISPLAY
}VIM_HAPI_WORKMODE;
 typedef enum _VIM_HAPI_LCDWORKMODE
{
	VIM_HAPI_LCDMODE_AFIRST=0,
	VIM_HAPI_LCDMODE_BLONLY,
	VIM_HAPI_LCDMODE_OVERLAY,
	VIM_HAPI_LCDMODE_BLEND
}VIM_HAPI_LCDWORKMODE;
 
 typedef enum _VIM_HAPI_LCDPANEL
{
	VIM_HAPI_LCDPANEL_MAIN=0,
	VIM_HAPI_LCDPANEL_SUB
}VIM_HAPI_LCDPANEL;
 
// set B layer is B0/B1/ALL
typedef enum _VIM_HAPI_LAYER_enum
{
	VIM_HAPI_B0_LAYER = 2,
	VIM_HAPI_B1_LAYER=4,
	VIM_HAPI_ALLB_LAYER=6,
	VIM_HAPI_A_LAYER=1,
	VIM_HAPI_ALL_LAYER=7
}VIM_HAPI_LAYER;

typedef enum _VIM_HAPI_ROTATEMODE{
	VIM_HAPI_ROTATE_0,
	VIM_HAPI_ROTATE_90,
	VIM_HAPI_ROTATE_180,
	VIM_HAPI_ROTATE_270,
	VIM_HAPI_MIRROR_0,
	VIM_HAPI_MIRROR_90,
	VIM_HAPI_MIRROR_180,
	VIM_HAPI_MIRROR_270
}VIM_HAPI_ROTATEMODE;

typedef enum tag_VIM_HAPI_RESOLUTION
{
	VIM_HAPI_MAXPIXEL = 0,
	VIM_HAPI_HALFPIXEL
}VIM_HAPI_RESOLUTION;


typedef enum tag_VIM_HAPI_SENSORMIRROR
{
	VIM_HAPI_SENSOR_NORMAL = 0x3,
	VIM_HAPI_SENSOR_FLIP=0x2,
	VIM_HAPI_SENSOR_MIRROR=1
}VIM_HAPI_SENSORMIRROR;

typedef enum tag_VIM_HAPI_SENSORMODE
{
	
	VIM_HAPI_SENSOR_50HZ=0x1,
	VIM_HAPI_SENSOR_60HZ=2,
	VIM_HAPI_SENSOR_OUT = 0x3,
	VIM_HAPI_SENSOR_NIGHT=4
}VIM_HAPI_SENSORMODE;

 typedef enum  _VIM_HAPI_PREVIEW_MODE
{
	VIM_HAPI_PREVIEW_OFF=0,
	VIM_HAPI_PREVIEW_ON,	
	VIM_HAPI_PREVIEW_FRAMEON
} VIM_HAPI_PREVIEW_MODE;


 typedef enum _VIM_HAPI_COLORDEP_MODE
{
	VIM_HAPI_COLORDEP_12BIT=4,
	VIM_HAPI_COLORDEP_15BIT,
	VIM_HAPI_COLORDEP_16BIT
 }VIM_HAPI_COLORDEP_MODE;

 typedef enum _VIM_HAPI_CPTURE_QUALITY
{
	VIM_HAPI_CPTURE_RATIO_MODE=0,
	VIM_HAPI_CPTURE_QUALITY_LOW=20,
	VIM_HAPI_CPTURE_QUALITY_MID=14,
	VIM_HAPI_CPTURE_QUALITY_HIGH=10,
	VIM_HAPI_CPTURE_QUALITY_PERFECT=7
	
}VIM_HAPI_CPTURE_QUALITY;
 
  typedef enum  _VIM_HAPI_SAVE_MODE
{
	VIM_HAPI_RAM_SAVE,
	VIM_HAPI_ROM_SAVE
}VIM_HAPI_SAVE_MODE;
  
 typedef enum  _VIM_HAPI_CHANGE_MODE
{
	VIM_HAPI_CHANGE_NORMAL=0,
	VIM_HAPI_CHANGE_ADD,
	VIM_HAPI_CHANGE_DEC,
	VIM_HAPI_SET_STEP
}VIM_HAPI_CHANGE_MODE;
 typedef enum  _VIM_HAPI_WHITEBALANCE_MODE
{
	VIM_HAPI_WHITEBALANCE_AUTO=0,
	VIM_HAPI_WHITEBALANCE_SUNNY,
	VIM_HAPI_WHITEBALANCE_CLOUDY,
	VIM_HAPI_WHITEBALANCE_TUNGSTEN,
	VIM_HAPI_WHITEBALANCE_FLUORESCENT,
	VIM_HAPI_WHITEBALANCE_MANUAL
}VIM_HAPI_WHITEBALANCE_MODE;
 typedef enum _VIM_HAPI_CONTRAST_STEP
{
	VIM_HAPI_CONTRAST_STEP_ONE=1,
	VIM_HAPI_CONTRAST_STEP_TWO,
	VIM_HAPI_CONTRAST_STEP_THREE,
	VIM_HAPI_CONTRAST_STEP_MAX=3
}VIM_HAPI_CONTRAST_STEP;


typedef enum  _VIM_HAPI_SPECIAL_EFFECT
{
	VIM_HAPI_SPECIAL_NORMAL=0,
	VIM_HAPI_SPECIAL_MONOCHROME,
	VIM_HAPI_SPECIAL_SEPIA,
	VIM_HAPI_SPECIAL_NEGATIVE, 
	//VIM_HAPI_SPECIAL_RELIEF,
	VIM_HAPI_SPECIAL_REDONLY=6
	//VIM_HAPI_SPECIAL_SKETCH,

} VIM_HAPI_SPECIAL_EFFECT; 
 
 typedef enum _VIM_HAPI_CALLBACK_STATUS
{
	VIM_HAPI_CAPTURE_END=0,
	VIM_HAPI_FIFO_FULL,
	VIM_HAPI_ONEFRAME_END,
	VIM_HAPI_BUF_ERROR,
	VIM_HAPI_UNKNOWN_ERROR

}VIM_HAPI_CALLBACK_STATUS;
 typedef enum _VIM_HAPI_DECODE_MODE
{
        VIM_DECODE_WITHFRAME_TO_YUV=0,
        VIM_DECODE_RESIZE_TO_YUV=1,
        VIM_DECODE_WITHFRAME_TO_RGB=2,
        VIM_DECODE_RESIZE_TO_RGB
 }VIM_HAPI_DECODE_MODE;

 typedef enum _VIM_HAPI_YUVTORGB
{
	VIM_CON_UYVY_565=0,
	VIM_CON_YUYV_565,
	VIM_CON_YYUV_565
}VIM_HAPI_YUVTORGB;

typedef enum _VIM_HAPI_GPIOPIN
{
 VIM_PIN_GPIO4=0x10,
 VIM_PIN_GPIO3=0x08,
 VIM_PIN_GPIO2=0x04,   
 VIM_PIN_GPIO1=0x02,  
 VIM_PIN_GPIO0=0x01  
}VIM_HAPI_GPIOPIN;

typedef struct _VIM_HAPI_DISPYUV_Data_
{
	UINT32 *yuv422;
	UINT16 SourceSize_W;
	UINT16 SourceSize_H;

	UINT16 DestSize_W;
	UINT16 DestSize_H;

	UINT16 DestOffset_X;
	UINT16 DestOffset_Y;
}VIM_HAPI_DISPYUV_Data;

typedef enum  _VIM_HAPI_DISPLAY_ZOOM
{
	VIM_HAPI_DISPLAY_NORMAL=0,	//(VIM_USER_DISPLAY_ZOOMDIVISOR-VIM_HAPI_DISPLAY_NORMAL)/VIM_USER_DISPLAY_ZOOMDIVISOR=(5-0)/5
	VIM_HAPI_DISPLAY_STEP1=1,		//(VIM_USER_DISPLAY_ZOOMDIVISOR-VIM_HAPI_DISPLAY_STEP1)/VIM_USER_DISPLAY_ZOOMDIVISOR=(5-1)/5
	VIM_HAPI_DISPLAY_STEP2=2,		//(VIM_USER_DISPLAY_ZOOMDIVISOR-VIM_HAPI_DISPLAY_STEP2)/VIM_USER_DISPLAY_ZOOMDIVISOR=(5-2)/5
	VIM_HAPI_DISPLAY_STEP3=3,		//(VIM_USER_DISPLAY_ZOOMDIVISOR-VIM_HAPI_DISPLAY_STEP3)/VIM_USER_DISPLAY_ZOOMDIVISOR=(5-3)/5
	VIM_HAPI_DISPLAY_STEP4=4,		//(VIM_USER_DISPLAY_ZOOMDIVISOR-VIM_HAPI_DISPLAY_STEP4)/VIM_USER_DISPLAY_ZOOMDIVISOR=(5-4)/5
	VIM_HAPI_DISPLAY_STEP5=5		//(VIM_USER_DISPLAY_ZOOMDIVISOR-VIM_HAPI_DISPLAY_STEP5)/VIM_USER_DISPLAY_ZOOMDIVISOR=(5-5)/5
	
}VIM_HAPI_DISPLAY_ZOOM;

typedef void (*VIM_HAPI_CallBack)(UINT8 Status, UINT32 Byte_Length);


UINT16 VIM_HAPI_InitVc05x(void);
///////////////////////chip mode ///////////////////////////////////
UINT16 VIM_HAPI_SetWorkMode(VIM_HAPI_WORKMODE WorkMode);
UINT16 VIM_HAPI_GetWorkMode(void);


///////////////////////////about preview/////////////////////////////////////
UINT16 VIM_HAPI_SetCaptureParameter(UINT16 CapWidth, UINT16 CapHeight);
UINT16 VIM_HAPI_SetPreviewParameter(UINT16 Lcd_OffsetW,UINT16 Lcd_OffsetH,UINT16 Width, UINT16 Height);
UINT16 VIM_HAPI_SetThumbParameter(UINT16 ThumWidth,UINT16 ThumHeight);
UINT16 VIM_HAPI_SetPreviewMode(VIM_HAPI_PREVIEW_MODE MODE);
UINT16 VIM_HAPI_SetPreviewZoom(VIM_HAPI_CHANGE_MODE Mode,UINT8 step);
UINT16 VIM_HAPI_SetPreviewMirror(VIM_HAPI_SENSORMIRROR mode);
UINT16 VIM_HAPI_SetEffect(VIM_HAPI_SPECIAL_EFFECT Mode);
UINT16 VIM_HAPI_SetPreviewBrightness(VIM_HAPI_CHANGE_MODE Mode,UINT8 Step);
UINT16 VIM_HAPI_SetPreviewContrast(VIM_HAPI_CHANGE_MODE Mode,UINT8 Step);
UINT16 VIM_HAPI_SetPreviewExMode(VIM_HAPI_SENSORMODE mode);
UINT16 VIM_HAPI_SetPreviewWhiteBalance(VIM_HAPI_WHITEBALANCE_MODE Mode,UINT8 Step);


///////////////////////////////lcd mode//////////////////////////////////////
UINT16 VIM_HAPI_SelectLCDPanel(VIM_HAPI_LCDPANEL byPanel);
UINT16 VIM_HAPI_SetLCDMirror(VIM_HAPI_ROTATEMODE MirrorFlipMode, VIM_HAPI_LAYER Layer);
UINT16 VIM_HAPI_SetLCDSize(VIM_HAPI_LAYER BLayer,UINT16 wStartx,UINT16 wStarty,UINT16 Width,UINT16 Height);
UINT16 VIM_HAPI_SetLCDColordep(VIM_HAPI_COLORDEP_MODE byColorDepth);
UINT16 VIM_HAPI_SetLCDWorkMode(VIM_HAPI_LCDWORKMODE byABLayerMode, UINT16 wValue);

/////////////////////////////b layer////////////////////////////
UINT16 VIM_HAPI_DrawLCDRctngl(VIM_HAPI_LAYER BLayer,UINT16 startx, UINT16 starty,UINT16 Width,UINT16 Height, HUGE  UINT8 *pwImage);
UINT16 VIM_HAPI_DrawLCDPureColor(VIM_HAPI_LAYER BLayer,UINT16 startx, UINT16 starty,UINT16 Width,UINT16 Height,UINT16 wRGBColor);
UINT16 VIM_HAPI_UpdateLCD(VIM_HAPI_LAYER BLayer,UINT16 startx, UINT16 starty,UINT16 Width,UINT16 Height);




/////////////capture//////////////////////////////////////////////
void VIM_HAPI_Timer(void);
UINT16 VIM_HAPI_SetCaptureQuality(VIM_HAPI_CPTURE_QUALITY Quality);
UINT16 VIM_HAPI_SetCaptureRatio(UINT8 ratio);
UINT16 VIM_HAPI_CaptureStill(VIM_HAPI_SAVE_MODE SaveMode,HUGE void *StillBuf,UINT32 BUF_Length,VIM_HAPI_CallBack pUCallBack);
UINT16 VIM_HAPI_CaptureThumbStill(VIM_HAPI_SAVE_MODE SaveMode,HUGE void *StillBuf,HUGE void *ThumbBuf,UINT32 BUF_Length,VIM_HAPI_CallBack pUCallBack);
UINT32 VIM_HAPI_GetCaptureLength(void);
UINT16 VIM_HAPI_SetCaptureVideoInfo(VIM_HAPI_SAVE_MODE SaveMode,UINT8 bFramRate,UINT32 dwMaxFrameCount );
UINT16 VIM_HAPI_StartCaptureVideo(HUGE void *StillBuf,UINT32 BUF_Length,VIM_HAPI_CallBack pUCallBack);
UINT16 VIM_HAPI_GetOneJpeg(HUGE void *StillBuf,UINT32 BUF_Length,UINT32 *dwOneLen);
UINT16 VIM_HAPI_StopCapture(void);
HUGE UINT8* VIM_HAPI_GetOneFrame(HUGE UINT8 *framebuf, UINT32 buflength, UINT8 framenumber, UINT32 *pframelength);
UINT32 VIM_HAPI_GetFrmCount(void);
UINT16 VIM_HAPI_GetPictureSize(UINT16 *picWidth, UINT16 *picHeight);
UINT16 VIM_HAPI_GetQuickOneJpeg(HUGE void *StillBuf,UINT32 BUF_Length,UINT32 *dwOneLen);
UINT16 VIM_HAPI_GetDecodeSize(UINT16 *picWidth, UINT16 *picHeight);


/////////////display//////////////////////////////////////////////

UINT16 VIM_HAPI_ReadyToDisplay(VIM_HAPI_SAVE_MODE SaveMode,HUGE void *jpegBuf, UINT32 length,UINT16 LcdOffsetX, UINT16 LcdOffsetY, UINT16 DisplayWidth, UINT16 DisplayHeight);
UINT16 VIM_HAPI_DisplayOneFrame(HUGE UINT8 *jpegBuf, UINT32 length);
UINT16 VIM_HAPI_QuickDisplayOneFrame(HUGE UINT8 *jpegBuf, UINT32 length);
UINT16 VIM_HAPI_Display_Jpeg(VIM_HAPI_SAVE_MODE SaveMode,HUGE void *jpegBuf, UINT32 length,UINT16 LcdOffsetX, UINT16 LcdOffsetY, UINT16 DisplayWidth, UINT16 DisplayHeight);
UINT16 VIM_HAPI_DisplayPan(UINT16 LcdOffsetX, UINT16 LcdOffsetY, UINT16 JpegOffsetX, UINT16 JpegOffsetY,UINT16 DisplayWidth, UINT16 DisplayHeight);
UINT16 VIM_HAPI_StopDisplayVideo(void);
UINT16 VIM_HAPI_Decode_Jpeg(VIM_HAPI_SAVE_MODE SaveMode,VIM_HAPI_DECODE_MODE DecodeMode,HUGE void *jpegBuf, HUGE UINT8 *pDest,UINT32 jpeglength,UINT32 Destlength,UINT16 TargetWidth,UINT16 TargetHeight);
UINT16 VIM_HAPI_YuvToRgb(HUGE UINT8 *pYUV, HUGE UINT8 *pRgb,UINT16 Source_Width,UINT16 Source_height,UINT8 Method);
UINT16 VIM_HAPI_ChangeVideoSize(UINT16 pt_x,UINT16 pt_y,UINT16 width,UINT16 height);
UINT16 VIM_HAPI_DisplayYUV(VIM_HAPI_DISPYUV_Data *struc);
UINT16 VIM_HAPI_Encode422(UINT8 *pYUV, UINT8 *pJpeg,UINT16 Width,UINT16 Height,UINT32 JpbufLen);
void VIM_HAPI_LCDInternalRotate(VIM_HAPI_ROTATEMODE degree,VIM_HAPI_WORKMODE WorkMode);
UINT16 VIM_HAPI_DisplayZoom(UINT16 JpegOffsetX, UINT16 JpegOffsetY,VIM_HAPI_DISPLAY_ZOOM ZoomStep);
UINT16 VIM_HAPI_RGB565toYUV422(UINT8 *pRGB, UINT8 *pYUV,UINT16 Width,UINT16 Height);

void VIM_HAPI_SetGpioValue(VIM_HAPI_GPIOPIN GPIOpin,BOOL IObit);

#ifdef __cplusplus
}
#endif

#endif /* _RDK_HIF_H_ */
