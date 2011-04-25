/*************************************************************************
*                                                                       
*                Copyright (C) 2005 Vimicro CO.,LTD     		 
*
* FILE NAME                                  VERSION                
*                                                                       
*  VIM_MID_API.h		   0.2                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*     VC0578 's function aip head file                          
*
*	Version		Author			Date		Description		
*  ---------------------------------------------------------------						 
*   0.1			angela		2005-11-15	The first version. 
*   0.2			angela		2006-6-20	updata for528
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
#ifndef _VIM_5XRDK_MAPI_H_
#define _VIM_5XRDK_MAPI_H_

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************
	Desc: set sensor mirror and flip callback 
	Para: BIT0 -- mirror
		  BIT1 -- flip
*******************************************************************/
typedef enum _VIM_MAPI_SENSOR_MIMODE
{
	VIM_SENSOR_NORMAL=0x03,
	VIM_SENSOR_MIRROR=1,
	VIM_SENSOR_FLIP=2,
	VIM_SENSOR_ROTATION=0
}VIM_MAPI_SENSOR_MIMODE;
typedef struct _VIM_MAPI_ZOOM_PARAMETER
{
	UINT8 CurrentZoomStep; 
	TSize InitAMemSize; 
	TSize InitSourceSize;
	UINT16 ZoomW;
	UINT16 ZoomH;
}VIM_MAPI_ZOOM_PARAMETER,*PVIM_MAPI_PARAMETER;
typedef struct tag_VIM_PREVIEW
{
	TSize Size;
	TPoint Point;
	VIM_HAPI_RESOLUTION bOutputIndex;
	VIM_HAPI_PREVIEW_MODE Mode;
	VIM_MAPI_SENSOR_MIMODE  SensorMirror; 
	VIM_HAPI_SENSORMODE  SensorMode; 
	VIM_HAPI_SPECIAL_EFFECT   SpecialEffect;
	VIM_MAPI_ZOOM_PARAMETER ZoomPara;
	UINT8 brightness;
	UINT8 contrast;
}VIM_PREVIEW,*PVIM_PREVIEW;
 typedef enum _VIM_VIDEO_MODE
{
	VIM_VIDEO_STARTCAPTURE=0,
	VIM_VIDEO_STARTDISPLAYE=1,	
	VIM_VIDEO_PAUSE=2,
	VIM_VIDEO_STOP=3
}VIM_VIDEO_MODE;

 typedef enum _VIM_YUV_TYPE
{
	VIM_YUV_UYVY422=0

}VIM_YUV_TYPE;

 typedef enum _VIM_CAPTURE_MODE
{
	VIM_CAPTUREDONE=0,
	VIM_CAPTURESTILL,
	VIM_ENCODE,
	VIM_CAPTURETHUMB,	
	VIM_MULTISHOT,		
	VIM_CAPTUREVIDEO,	
	VIM_CAPTUREAVI,
	VIM_CAPTUREUNKNOWN,
	VIM_CAPTURECAPTURE,
	VIM_CAPMULTIFRAME,
	CAPTURETHUMB_GETYUV_MODE
}VIM_CAPTURE_MODE;
 
 typedef void (*VIM_CallBack)(UINT8 Status, UINT32 Byte_Length);
typedef UINT8* (*VIM_AVI_AudioCalLBack)(UINT32 *Byte_Length);
typedef struct tag_VIM_CAPTURE
{
	VIM_CAPTURE_MODE Mode;
	VIM_RESULT CaptureError;
	TSize Size;
	TSize ThumbSize;
	TSize MinSize;        //if capture size is 640*480 then it is 4*3
	UINT32 CapFileLength;
	HUGE UINT8 *BufPoint;
	HUGE UINT8 *MallocPr;
	HUGE UINT8 *IndexPoint;
	HUGE UINT8 *ThumbBuf;
	UINT32 BufLength;
	UINT32 MarbJbufLength;
	UINT8 QualityMode;
	UINT8 RecaptureTimes;
	VIM_CallBack CapCallback;
	VIM_AVI_AudioCalLBack pAudioCall;
	VIM_HAPI_SAVE_MODE SaveMode;
	HUGE void* pFile_Nameptr;
	UINT8 ReCapSize;
}VIM_CAPTURE,*PVIM_CAPTURE;
typedef enum _VIM_DECODE_STATUS
{
	VIM_DECODE_DONE=0,
	VIM_DECODE_BUFERROR,
	VIM_DECODE_SHOULDREWRITE,
	VIM_DECODE_ING
}VIM_DECODE_STATUS;

typedef struct tag_VIM_DISPLAY
{
	HUGE UINT8 *DecPoint;		//decode target buffer point
	UINT32 DecLength;	//jpeg data buffer length
	UINT16 DecodeHeight;		//
	HUGE UINT8 *BufPoint;		//jpeg data buffer point
	HUGE UINT8 *MallocPr;
	UINT32 BufLength;	//jpeg data buffer length
	UINT32 BufOffset;	//jpeg data buffer length
	void* pFileHandle;	//jpeg file handle, use in ROM
	UINT32 FileLength;	//jpeg file length, use in ROM
	TPoint LcdWantPt;
	TSize LcdWantSize;
	VIM_DECODE_STATUS ReadOk;
	VIM_HAPI_SAVE_MODE SaveMode;
}VIM_DISPLAY,*PVIM_DISPLAY;

typedef struct tag_VIM_LCD
{
	TSize Size;
	TPoint B0Startpoint;
	TPoint B1Startpoint;
	VIM_HAPI_LCDWORKMODE WorkMode;
	VIM_HAPI_COLORDEP_MODE ColorDep;
	VIM_DISP_ROTATEMODE ARotationMode;
	VIM_DISP_ROTATEMODE BRotationMode;
	VIM_HAPI_LCDPANEL NowPanel;
}VIM_LCD,*PVIM_LCD;
typedef struct tag_VIM_MARB
{
	VIM_MARB_Map MapList;
	VIM_DISP_BUFFERMODER ALayerMode;
	VIM_MARB_WORKMODE WorkMode;
	UINT32 Jpgpoint;
}VIM_MARB,*PVIM_MARB;


typedef struct tag_VIM_VIDEO
{
	VIM_VIDEO_MODE Mode;
	VIM_CallBack CapCallback;
	UINT32	VideoFrameRate;		//VIM_WORK_MODE
	UINT32	MaxFrame;		//VIM_WORK_MODE
	UINT32	NowFrame;		//VIM_WORK_MODE	
	UINT32	VideoLength;		//VIM_WORK_MODE
}VIM_VIDEO,*PVIM_VIDEO;
typedef struct tag_VIM_STATUS{
	PTSnrInfo	pSensorInfo;
	PTPanelInfo   pDriverIcInfo;
	PVIM_USER_INITINFO	pUserInfo;
	UINT8	ChipWorkMode;		//VIM_WORK_MODE
	VIM_PREVIEW	PreviewStatus;
	VIM_CAPTURE CaptureStatus;
	VIM_DISPLAY DisplayStatus;
	VIM_LCD LcdStatus;
	VIM_MARB MarbStatus;
	VIM_VIDEO	VideoStatus;
}VIM_STATUS, *PVIM_STATUS;


typedef struct _VIM_DISPYUV_Data_
{
	UINT32 *yuv422;
	UINT16 SourceSize_W;
	UINT16 SourceSize_H;

	UINT16 DestSize_W;
	UINT16 DestSize_H;

	UINT16 DestOffset_X;
	UINT16 DestOffset_Y;
}VIM_DISPYUV_Data;
extern VIM_STATUS gVc0528_Info;
extern VIM_JPEG_Index gVc0528_JpegInfo;
extern VIM_HIF_TISR  gVc0528_Isr;
VIM_RESULT VIM_MAPI_TestRegister (void);
VIM_RESULT VIM_MAPI_InitHif(void);
VIM_RESULT VIM_MAPI_AutoFindSensor(void);
void VIM_MAPI_InitCapInterrupt(void);
void VIM_MAPI_ISR_OneFrameDone(void);
void VIM_MAPI_ISR_LBufRead(void);
void VIM_MAPI_ISR_Recapture(void);
void VIM_MAPI_StartDecode(void);



VIM_RESULT VIM_MAPI_SetCaptureRatio(UINT8 ratio);
void VIM_MAPI_InitIfo(void);
void VIM_MAPI_SwitchPanel(VIM_HAPI_LCDPANEL bPanel);
void VIM_MAPI_SetChipMode(VIM_MARB_WORKMODE bMode,VIM_IPP_HAVEFRAM bHaveFrame);
void VIM_MAPI_Delay_Frame(UINT8 FrameCount);

void VIM_MAPI_WriteOneFrameData(UINT32 Truelen,HUGE UINT8 *start);
void VIM_MAPI_JpegAutoDecode(UINT32 Datasize);
VIM_RESULT VIM_MAPI_DecodeOneFrame(UINT32 Truelen,HUGE UINT8 *start);
VIM_RESULT VIM_MAPI_CheckPictureInJBufForAutoDecode(void);
VIM_RESULT VIM_MAPI_CheckDisplaySize(UINT16 LcdOffsetX, UINT16 LcdOffsetY, UINT16 DisplayWidth, UINT16 DisplayHeight,TSize *DisplaySize);
VIM_RESULT VIM_MAPI_CheckDisplayPanSize(UINT16 JpegOffsetX, UINT16 JpegOffsetY, TSize DisplaySize,TSize *DestDisplaySize,TPoint* pt );
VIM_RESULT VIM_MAPI_CheckDecodeTargetSize(VIM_JPEG_YUVMODE bYUVmode,TSize SourceSize, TSize DisplaySize);
VIM_RESULT VIM_MAPI_SetALayerDisplaySize(TPoint mPt,TSize MemorySize, TSize DisplaySize, UINT16 LcdOffsetX, UINT16 LcdOffsetY);
VIM_RESULT VIM_MAPI_Preview_CaptureZoom(UINT8 step);
VIM_RESULT VIM_MAPI_Preview_DisplayZoom(UINT8 step);
VIM_RESULT VIM_MAPI_GetJpegInformation(HUGE UINT8 *jpegBuf,UINT32 length);
void VIM_MAPI_SetDisplayJpegMode(void);
VIM_RESULT VIM_MAPI_SetDecodeJpegMode(VIM_HAPI_DECODE_MODE DecodeMode);
VIM_RESULT VIM_MAPI_SetDisplayJpegParameter(void);
VIM_RESULT VIM_MAPI_SetAutoDisplayJpegParameter(void);


VIM_RESULT VIM_MAPI_AdjustPoint(VIM_DISP_ROTATEMODE byARotateMode,VIM_DISP_ROTATEMODE byBRotateMode);
VIM_RESULT VIM_MAPI_FastPreview(void);
VIM_RESULT VIM_MAPI_GetNewPoint(VIM_DISP_ROTATEMODE byRotateMode,PTPoint OldPoint,TSize OldSize,PTPoint NewPoint);
void VIM_MAPI_EnableSyncGen(UINT8 bEnable);


VIM_RESULT VIM_MAPI_ReadyToDecode(VIM_HAPI_SAVE_MODE SaveMode,HUGE void *jpegBuf,VIM_HAPI_DECODE_MODE DecodeMode, UINT32 length,UINT16 DisplayWidth, UINT16 DisplayHeight);

UINT8 VIM_MAPI_FindUpdateRange(TSize size, TPoint Point);
void VIM_MAPI_RGB565toYUV(UINT8 *pRGB, UINT8 *pYUV);
void VIM_MAPI_SendYuvData(UINT8 *pbuf, TSize Size,VIM_YUV_TYPE fmt);

VIM_RESULT VIM_MAPI_SetDisplayYUVParameter(TSize size);
void VIM_MAPI_SetDisplayYUVMode(void);
UINT16 VIM_MAPI_ReadyToDisplayYUV(VIM_DISPYUV_Data *struc);
void VIM_MAPI_WriteOneFrameYUVData(UINT32 Truelen,HUGE UINT8 *start);


VIM_RESULT VIM_MAPI_ReadyToDisplayZoom(UINT16 JpegOffsetX, UINT16 JpegOffsetY, VIM_HAPI_DISPLAY_ZOOM Step);

#ifdef __cplusplus
}
#endif

#endif /* _RDK_HIF_H_ */
