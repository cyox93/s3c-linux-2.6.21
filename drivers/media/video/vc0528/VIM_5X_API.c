/*************************************************************************
*                                                                       
*                Copyright (C) 2006 Vimicro CO.,LTD     		 
*
* FILE NAME                                  VERSION                
*                                                                       
* VIM_5X_API.c			   0.1                    
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
#include"VIM_COMMON.h"
#include"VIM_5X_API.h"
#if VIM_USER_SUPPORT568_API

#ifdef BOTH_5XSUPPORT
UINT8 IsVc0568Flag=0;
#endif

/********************************************************************************
Description:
Initialize 5X(global parameters, 5X modules)
	Return
	if return V5H_SUCCEED initialize succeed,
	Return 1:  read and write register wrong 
	Return 2:  read and write register wrong (MCLK domain)
	Return 3:  read and write sram wrong	
	Return 4:  Clk 's parameter  wrong
Note:
	This function should be called before V5x can work, before Initialize, 5x chip should be reset.
	This function only can be called one time.
	After this function, the 528 will be the bypass mode
*******************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT16 V5S_HAPI_InitVc05x(void)
#else
UINT8 V5H_Init_568(void)
#endif
{
#ifdef BOTH_5XSUPPORT
UINT8 Error;
#if VIM_BUSTYPE==VIM_MULTI16
	VIM_HIF_SetMulti16();
#endif
	Error=VIM_MAPI_TestRegister();
	if(Error==VIM_ERROR_HIF_READWRITE_REG8)
		return Error;
	if(VIM_HIF_GetReg32(V5_REG_CPM_CHIP_ID_0)==0x05681100)
	{
		IsVc0568Flag=1;//vc568
		return( (UINT16)V5H_Init_568());
	}
	else
	{
		IsVc0568Flag=0;//vc528
	  	return( VIM_HAPI_InitVc05x());
	}
#endif	
	return( VIM_HAPI_InitVc05x());

}
/********************************************************************************
Description:
	Set 5x work mode
Parameters:
	WorkMode:
		V5H_MODE_CAMERAON
		V5H_MODE_BYPASS
		V5H_MODE_PHOTOPROCESS
		V5H_MODE_DIRECTDISPLAY
	Return:
		V5H_SUCCEED: set work mode ok
		V5H_FAILED:     error work mode,or no this work mode
	Note:
		If the original state and the des state is the same 
		,the function will return V5H_SUCCEED and doing nothing.
********************************************************************************/

#ifdef BOTH_5XSUPPORT
UINT16 V5S_HAPI_SetWorkMode(UINT8 WorkMode)
#else
UINT8 V5H_WorkMode_Set(UINT8 WorkMode)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return((UINT16)V5H_WorkMode_Set(WorkMode));
	else
#endif
		return(VIM_HAPI_SetWorkMode((VIM_HAPI_WORKMODE)WorkMode));

}

#ifdef BOTH_5XSUPPORT
UINT16 V5S_HAPI_GetWorkMode(void)
#else
UINT8 V5H_WorkMode_Get(void)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return((UINT16)V5H_WorkMode_Get());
	else//528
#endif
		return(VIM_HAPI_GetWorkMode());

}

/********************************************************************************
Description:
	Set Capture parameter(still, multishot, video)
Parameters:
	CapWidth:  the width of captured image.
	CapHeight: the height of captured image.
	ThumWidth: Thumbnail image width(when capture with thumbnail) , and it's heith is decided by
	     CapWidth: CapHeight(same ration).ThumWidth should no larger than CapWidth.
	     The real width and height of thumbnail can get by V5H_Capture_GetThumbRealSize().
	Return:
		V5H_SUCCEED: set ok
		V5H_ERROR_WORKMODE: error work mode
	Note:
		This function must be used before first preview or change the resolution of display.
		This function must be used in Camera open mode
	Remarks:
	State:Valid.
*********************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT16 V5S_HAPI_SetCaptureParameter(UINT16 CapWidth, UINT16 CapHeight,UINT16 ThumWidth)
#else
UINT8 V5H_Capture_Parameter(UINT16 CapWidth, UINT16 CapHeight,UINT16 ThumWidth)
#endif
{
UINT16 ThumHeight,Error;
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return((UINT16)V5H_Capture_Parameter(CapWidth,CapHeight,ThumWidth));
	else//528
#endif
	{
		ThumHeight=(UINT16)(((UINT32)ThumWidth*(UINT32)CapHeight)/CapWidth);
		
		Error=VIM_HAPI_SetCaptureParameter(CapWidth,CapHeight);
		if(Error)
			return Error;
		Error=VIM_HAPI_SetThumbParameter(ThumWidth,ThumHeight);
		return Error;
	}


}
/********************************************************************************
Description:
	set preview parameters
Parameters:
	Width:  the width of preview area on lcd.
	height: the height of preview area on lcd.
	Lcd_OffsetW: offset x of start point on lcd.
	Lcd_OffsetH: offset y of start point  on lcd.
	Return:
		VIM_ERROR_WORKMODE:  work error mode
		VIM_SUCCEED:  set parameter ok
		VIM_ERROR_PREVIEW_PARAMETER:  error parameter
Note:
	Preview parameters should be set before start preview, 
	and these parameters only can come to valid when start preview 
Remarks:
	state: Valid
*********************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT16 V5S_HAPI_SetPreviewParameter(UINT16 Lcd_OffsetW,UINT16 Lcd_OffsetH,UINT16 Width, UINT16 Height)
#else
UINT8 V5H_Preview_Parameter(UINT16 Lcd_OffsetW,UINT16 Lcd_OffsetH,UINT16 Width, UINT16 Height)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return((UINT8)V5H_Preview_Parameter(Lcd_OffsetW,Lcd_OffsetH,Width, Height));
	else//528
#endif
		return((UINT16)VIM_HAPI_SetPreviewParameter(Lcd_OffsetW,Lcd_OffsetH,Width, Height));


} 
/********************************************************************************
Description:
	SET preview MODE
Parameters:
528	MODE: preview mode
		VIM_PREVIEW_OFF: preview off
		VIM_HAPI_PREVIEW_ON: preview on without frame
		VIM_HAPI_PREVIEW_FRAMEON: preview on with frame
568 MODE
	V5H_PREVIEW_OFF=0,
	V5H_PREVIEW_ON,	
	V5H_PREVIEW_FRAMEON
	bIndex: is the index of resolution (see the sensorinfo.c)
	Return:
		VIM_SUCCEED: set mode ok
		VIM_ERROR_WORKMODE: error work mode
		VIM_ERROR_????????: Return from middle API function
	Note:
		This function must be used in Camera open mode
	Remarks:
		state: valid
*********************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT16 V5S_HAPI_SetPreviewMode(UINT8 MODE)
#else
UINT8 V5H_Preview_MODE(UINT8 MODE)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return((UINT8)V5H_Preview_MODE((V5H_PREVIEW_MODE)MODE));
	else//528
#endif
		return((UINT16)VIM_HAPI_SetPreviewMode((VIM_HAPI_PREVIEW_MODE)MODE));
}

/********************************************************************************
Description:
	zoom in or zoom out
Parameters:
	Mode:
		VIM_HAPI_CHANGE_NORMAL=0,
		 	auto set normal status(preview mode).
		 	in this mode the step has no meaning.
		VIM_HAPI_CHANGE_ADD,
			when in add mode.
			the step 's meaning is that 
			source window's width  will zoom in one step
		VIM_HAPI_CHANGE_DEC,
			when in decrease mode.
			the step 's meaning is that 
			source window's width  will zoom out one step
		VIM_HAPI_SET_STEP,
		       set the direct zoom step.
	Return:
		VIM_HAPI_SUCCEED: zoom ok
	Note:
		If preview is on, zoom will take effect immediately.
		Otherwise, this function only 	save the zoom step. The zoom has effect when turn on preview.
	Remarks:
		state: valid
*********************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT16 V5S_HAPI_Preview_Zoom(UINT8 Mode,UINT8 step)
#else
UINT8 V5H_Preview_Zoom(UINT8 Mode,UINT8 step)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return((UINT16)V5H_Preview_Zoom(Mode,step));
	else//528
#endif
		return((UINT16)VIM_HAPI_SetPreviewZoom((VIM_HAPI_CHANGE_MODE)Mode,step));
}

/********************************************************************************
Description:
	set capture quitlity 
Parameters:
	ratio: the Compression ratio
		
	Return:
		VIM_SUCCEED: set mode ok
		VIM_ERROR_PARAMETER: can not support this value
	Note:
		after calling this funcion. it will take few seconds to make effect.
	Remarks:
		state: valid
*********************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT16 V5S_HAPI_SetCaptureQuality(UINT8 Quality)
#else
UINT8 V5H_Capture_Quality(UINT8 Quality)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return((UINT16)V5H_Capture_Quality(Quality));
	else//528
#endif
		return((UINT16)VIM_HAPI_SetCaptureQuality((VIM_HAPI_CPTURE_QUALITY) Quality));

}

/********************************************************************************
Description:
	set capture ratio 
Parameters:
	ratio: the Compression ratio
		
	Return:
		VIM_SUCCEED: set mode ok
		VIM_ERROR_PARAMETER: can not support this value
	Note:
		after calling this funcion. it will take few seconds to make effect.
	Remarks:
		state: valid
*********************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT16 V5S_HAPI_SetCaptureRatio(UINT8 ratio)
#else
UINT8 V5H_Capture_CompressionRatio(UINT8 ratio)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return((UINT16)V5H_Capture_CompressionRatio(ratio));
	else//528
#endif
		return(VIM_HAPI_SetCaptureRatio(ratio));

}
/********************************************************************************
Description:
	Capture a still(jpeg)
Parameters:
    528  SaveMode:
      	VIM_HAPI_RAM_SAVE,   save in the still buf (UINT8)
	VIM_HAPI_ROM_SAVE  stillbuf is the file id  of file 
  568  this para is not use!
      StillBuf: the buffer to store the jpeg or the file id
      BUF_Length: the buffer length
      pUCallBack: callback function
Return:
	VIM_SUCCEED:  ok
	VIM_ERROR_WORKMODE(V5H_ERROR_PREVIEWMODE): error work mode
	VIM_ERROR_PREVIEWMODE: preview error
Note:
	if(pUCallBack==NULL) this function will return after finish capturing,
	else it will return before  finish capturing, users should judge the end in callback function.
Remarks:
	This function should be called when preview on.
       pUCallBack return status:
 	V5H_CAPTURE_END: capture end
	V5H_BUF_ERROR:     buffer length is not enough for JPEG     
state: 
	valid
*********************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT16 V5S_HAPI_CaptureStill(HUGE UINT8*StillBuf,UINT32 BUF_Length,V5H_CallBack pUCallBack)
#else
UINT8 V5H_Capture_Still(HUGE UINT8*StillBuf,UINT32 BUF_Length,V5H_CallBack pUCallBack)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return((UINT16)V5H_Capture_Still(StillBuf,BUF_Length,(V5H_CallBack) pUCallBack));
	else//528
#endif
	{
		return((UINT16)VIM_HAPI_CaptureStill(VIM_HAPI_RAM_SAVE,(HUGE void *)StillBuf, BUF_Length,(VIM_HAPI_CallBack) pUCallBack));
	}
}

/********************************************************************************
Description:
	get frame number of capture(mulishot or MJPEG)
Parameters:

Return:
	frame number captured
Note:
		
Remarks:
state: 
	valid
*********************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT32 V5S_Capture_GetFrmCount(void)
#else
UINT32 V5H_Capture_GetFrmCount(void)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return((UINT32)V5H_Capture_GetFrmCount());
	else//528
#endif
	{
		return((UINT16)VIM_HAPI_GetFrmCount());
	}
}

/********************************************************************************
Description:
	Capture video(motion-JPEG)
Parameters:
      interval: in each 16 frames from sensor, which frame is discard(0), which frame is captured(1)
      		    total 16bits, each bit means this frame is discard or captured.
      VideoBuf: the buffer to store the video(MJPEG)
      Buflength: the length of VideoBuf
      pUCallBack: callback function
Return:
	V5H_SUCCEED:  ok
	V5H_ERROR_WORKMODE(V5H_ERROR_PREVIEWMODE): error work mode
	V5H_ERROR_PREVIEWMODE: preview error
Note:
	if(pUCallBack==NULL) this function will return after finish capturing,
	else it will return before  finish capturing, users should judge the end in callback function.
Remarks:
	This function should be called when preview on.
       pUCallBack return status:
 	V5H_CAPTURE_END: capture end
	V5H_BUF_ERROR:     buffer length is not enough for JPEG   
state: 
	valid
*********************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT8 V5S_HAPI_Capture_Mjpeg(UINT16 interval, HUGE UINT8 *VideoBuf, UINT32 Buflength, V5H_CallBack pUCallBack)
#else
UINT8 V5H_Capture_Mjpeg(UINT16 interval, HUGE UINT8 *VideoBuf, UINT32 Buflength, V5H_CallBack pUCallBack)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return((UINT16)V5H_Capture_Mjpeg(interval, VideoBuf, Buflength, pUCallBack));
	else//528
#endif
	{
		VIM_HAPI_SetCaptureVideoInfo(VIM_HAPI_RAM_SAVE,15,0xffff);
		return((UINT16)VIM_HAPI_StartCaptureVideo((HUGE void *)VideoBuf,Buflength,pUCallBack));
	}
}
/********************************************************************************
Description:
	Capture a still(jpeg) with thumbnail
Parameters:
      SaveMode:
      				VIM_HAPI_RAM_SAVE,   save in the still buf (UINT8)
				VIM_HAPI_ROM_SAVE  stillbuf is the file id  of file 
      StillBuf: the buffer to store the jpeg or the file id
      ThumbBuf:the head point of  buffer to store the thumbnail data(must large than thumb width and heigth)
      BUF_Length: the buffer length
      pUCallBack: callback function
Return:
	VIM_SUCCEED:  ok
	VIM_ERROR_WORKMODE(V5H_ERROR_PREVIEWMODE): error work mode
	VIM_ERROR_PREVIEWMODE: preview error
Note:
	if(pUCallBack==NULL) this function will return after finish capturing,
	else it will return before  finish capturing, users should judge the end in callback function.
Remarks:
	This function should be called when preview on.
       pUCallBack return status:
 	V5H_CAPTURE_END: capture end
	V5H_BUF_ERROR:     buffer length is not enough for JPEG     
state: 
	valid
*********************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT16 V5S_HAPI_CaptureThumbStill(HUGE UINT8 *ThumbBuf,HUGE UINT8 *StillBuf,UINT32 BUF_Length,V5H_CallBack pUCallBack)
#else
UINT8 V5H_Capture_ThumbStill(HUGE UINT8 *ThumbBuf,HUGE UINT8 *StillBuf,UINT32 BUF_Length,V5H_CallBack pUCallBack)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return((UINT16)V5H_Capture_ThumbStill(ThumbBuf,StillBuf,BUF_Length,(V5H_CallBack) pUCallBack));
	else//528
#endif
	{
		return((UINT16)VIM_HAPI_CaptureThumbStill(VIM_HAPI_RAM_SAVE,(HUGE void *)StillBuf,(HUGE void *)ThumbBuf,BUF_Length,(VIM_HAPI_CallBack) pUCallBack));
	}
}
/********************************************************************************
Description:
	start Capture a stream still(jpeg)
Parameters:
      StillBuf: the buffer to store the jpeg or the file id
      BUF_Length: the buffer length
      pUCallBack: callback function (must set it)
Return:
	VIM_SUCCEED:  ok
	VIM_ERROR_WORKMODE(V5H_ERROR_PREVIEWMODE): error work mode
	VIM_ERROR_PREVIEWMODE: preview error
Note:
	if(pUCallBack==NULL) this function will return after finish capturing,
	else it will return before  finish capturing, users should judge the end in callback function.
Remarks:
	This function should be called when preview on.
       pUCallBack return status:
 	V5H_CAPTURE_END: capture end
	V5H_BUF_ERROR:     buffer length is not enough for JPEG     
state: 
	valid
*********************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT16 V5S_HAPI_StartCaptureVideo(HUGE UINT8 *StillBuf,UINT32 BUF_Length)
#else
UINT8 V5H_Capture_MultiFrame_Start(HUGE UINT8 *StillBuf,UINT32 BUF_Length)
#endif
{
#ifdef BOTH_5XSUPPORT	
	if(IsVc0568Flag==1)//vc568
		return((UINT16)V5H_Capture_MultiFrame_Start(StillBuf,BUF_Length));//?lhd
	else//528
#endif
	{
		VIM_HAPI_SetCaptureVideoInfo(VIM_HAPI_RAM_SAVE,0,1000);
		return((UINT16)VIM_HAPI_StartCaptureVideo((HUGE void *)StillBuf,BUF_Length,NULL));
	}
}

/********************************************************************************
Description:
	Multishot
Parameters:
      FrameCount: frame number of mutishot
      Framedrop: in each 16 frames from sensor, which frame is discard(0), which frame is captured(1)
      		    total 16bits, each bit means this frame is discard or captured.
      jpegBuf: the buffer to store the mutishot frame.
      Buflength: the length of jpegbuf
      pUCallBack: callback function
Return:
	V5H_SUCCEED:  ok
	V5H_ERROR_WORKMODE(V5H_ERROR_PREVIEWMODE): error work mode
	V5H_ERROR_PREVIEWMODE: preview error
Note:
	if(pUCallBack==NULL) this function will return after finish capturing,
	else it will return before  finish capturing, users should judge the end in callback function.
Remarks:
	This function should be called when preview on.
       pUCallBack return status:
       V5H_ONEFRAME_END: One frame has been captured
 	V5H_CAPTURE_END: capture end
	V5H_BUF_ERROR:     buffer length is not enough for JPEG   
state: 
	valid
*********************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT8 V5S_Capture_MULTI(UINT8 FrameCount, UINT16 Framedrop, HUGE UINT8 *jpegBuf, UINT32 Buflength, V5H_CallBack pUCallBack)	
#else
UINT8 V5H_Capture_MULTI(UINT8 FrameCount, UINT16 Framedrop,HUGE UINT8 *jpegBuf, UINT32 Buflength, V5H_CallBack pUCallBack)	
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
	{
		return((UINT16)V5H_Capture_MULTI(FrameCount,Framedrop,jpegBuf,Buflength,pUCallBack));//?lhd
	}
	else//528
#endif
	{
		VIM_HAPI_SetCaptureVideoInfo(VIM_HAPI_RAM_SAVE,15,FrameCount);
		return((UINT16)VIM_HAPI_StartCaptureVideo((HUGE void *)jpegBuf,Buflength,pUCallBack));
	}
}
/********************************************************************************
Description:
	Get one frame(jpeg) with preview on 
Parameters:
	*dwOneLen: the total length captured from VIM_HAPI_StartCaptureVideo()
	StillBuf:the head point of jpeg
	BUF_Length:the main buf len,should be larger than jpeg file length!
Return:
	VIM_SUCCEED:  ok
	VIM_ERROR_WORKMODE(V5H_ERROR_PREVIEWMODE): error work mode
	VIM_ERROR_PREVIEWMODE: preview error
Note:
	this function should be used when calling VIM_HAPI_StartCaptureVideo () 
		bFramRate = 0
	else 
		this function will be called by rdk
Remarks:
state: 
	valid
*********************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT16 V5S_HAPI_GetOneJpeg(HUGE UINT8 *StillBuf,UINT32 BUF_Length,UINT32 *dwOneLen)
#else
UINT8 V5H_Capture_MultiFrame_GetOne(HUGE UINT8 *StillBuf,UINT32 BUF_Length, UINT32 *dwOneLen)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return((UINT16)V5H_Capture_MultiFrame_GetOne(StillBuf,BUF_Length,dwOneLen));
	else//528
#endif
	{
		return((UINT16)VIM_HAPI_GetOneJpeg((HUGE void *)StillBuf,BUF_Length,(UINT32 *)dwOneLen));
	}
}
#ifdef BOTH_5XSUPPORT
UINT16  V5S_Capture_GetOneFrame(UINT8 *VideoBuf,UINT32* len,UINT32 BufLength)
#else
UINT8  V5H_Capture_GetOneFrame(UINT8 *VideoBuf,UINT32* len,UINT32 BufLength)
#endif
{
UINT16 result;
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return((UINT16)V5H_Capture_GetOneFrame(VideoBuf,len,BufLength));
	else//528
#endif
	{
		result=VIM_HAPI_GetQuickOneJpeg((HUGE void *)VideoBuf,BufLength,len);
		switch(result)
		{
		case VIM_SUCCEED:
			return result;
		case VIM_ERROR_NOINITERRUPT:
			return V5H_ERROR_NOINITERRUPT;
		case VIM_ERROR_BUFFERLENGTHS:
			return V5H_ERROR_BUFLENS;
		default:
			return result;
		}

	}
}

/********************************************************************************
Description:
	Stop capture video(motion-JPEG)
Parameters:

Return:
	V5H_SUCCEED:  ok
	V5H_ERROR_WORKMODE(V5H_ERROR_PREVIEWMODE): error work mode
Note:

Remarks:
state: 
	valid
*********************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT8 V5S_HAPI_StopCapture(void)
#else
UINT8 V5H_Capture_Stop(void)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return((UINT8)V5H_Capture_Stop());
	else//528
#endif
	{
		return((UINT8)VIM_HAPI_StopCapture());
	}
}


/********************************************************************************
Description:
	Stop capture one JPEG with preview on 
Parameters:

Return:
	The total length captured(byte count)
Note:
	This function will stop the preview
Remarks:
state: 
	valid
*********************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT16 V5S_HAPI_MultiFrame_Stop(void)
#else
UINT32 V5H_Capture_MultiFrame_Stop(void)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return((UINT16)V5H_Capture_Stop());
	else//528
#endif
	{
		return((UINT16)VIM_HAPI_StopCapture());
	}
}



/********************************************************************************
Description:
	Get the  captured length(byte count).
Parameters:

Return:
		length
Note:
	The function can be used to get the length of capture still, mutishot, MJPEG	
Remarks:
state: 
	valid
*********************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT32 V5S_HAPI_GetCaptureLength(void)
#else
UINT32 V5H_Capture_GetLength(void)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return(V5H_Capture_GetLength());
	else//528
#endif
	{
		return(VIM_HAPI_GetCaptureLength());
	}

}
/*************************************************************
Description:
	This function will choose a mode about relationship of a layer and b layer .
	and overlay keycolor(blending value)
Parameters:
	Mode: LCD work mode
			V5H_LCDMODE_AFIRST,
			V5H_LCDMODE_BLONLY,
			V5H_LCDMODE_OVERLAY,
			V5H_LCDMODE_BLEND,
	Value:
		In V5H_ V5H_LCDMODE_AFIRST,V5H_ LCDMODE_BLONLY, mode
			This value has no effect.
		Else in V5H_ LCDMODE _OVERLAY, mode
			The value is the key color which color will be transparent
		in	V5H_ LCDMODE _BLEND, mode
			The value is the Blending value(0-255)
	Return:
		V5H_SUCCEED: set ok
		V5H_ERROR_WORKMODE: error work mode
		V5H_FAILED: error parameter
	Note
		In V5H_ LCDMODE _BLEND mode, the image should be the 15bit colordep.
	This function can be used in Camera open mode or Photo process mode
	or direct display mode
*********************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT16 V5S_HAPI_SetLCDWorkMode(UINT8 byABLayerMode, UINT16 wValue)
#else
UINT8 V5H_LCD_WorkMode(UINT8 byABLayerMode, UINT16 wValue)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return(V5H_LCD_WorkMode(byABLayerMode,wValue));
	else//528
#endif
	{
		return(VIM_HAPI_SetLCDWorkMode((VIM_HAPI_LCDWORKMODE) byABLayerMode,  wValue));
	}
}

/********************************************************************************
Description:
	Draw a Rectangle to 5x chip's  B Layer memory
Parameters:
	startx: the x coordinate of the Rectangle start point(pixel, start from 0)
	starty: the y coordinate of the Rectangle start point(pixel, start from 0)
	Width: the width of the Rectangle(pixel)
	Height: the height of the Rectangle(pixel)
	uspImage: the point of memory which to fill the Rectangle
	Return:
		V5H_SUCCEED: set ok
		V5H_ERROR_WORKMODE: error work mode
		V5H_FAILED: error parameter
	Note£º
		Only write to the 568 internal ram, not update to LCD.
	This function can be used in Camera open mode or Photo process mode 
Remarks:
	valid
*********************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT16 V5S_HAPI_SetLCDSize(UINT16 wStartx,UINT16 wStarty,UINT16 Width,UINT16 Height)
#else
UINT8 V5H_SetLCDSize(UINT16 wStartx,UINT16 wStarty,UINT16 Width,UINT16 Height)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return 0;
	else//548
#endif
	{
		return(VIM_HAPI_SetLCDSize(VIM_HAPI_B0_LAYER,wStartx,wStarty,Width,Height));
	}
}
/********************************************************************************
Description:
	Draw a Rectangle to 5x chip's  B Layer memory
Parameters:
	startx: the x coordinate of the Rectangle start point(pixel, start from 0)
	starty: the y coordinate of the Rectangle start point(pixel, start from 0)
	Width: the width of the Rectangle(pixel)
	Height: the height of the Rectangle(pixel)
	uspImage: the point of memory which to fill the Rectangle
	Return:
		V5H_SUCCEED: set ok
		V5H_ERROR_WORKMODE: error work mode
		V5H_FAILED: error parameter
	Note£º
		Only write to the 568 internal ram, not update to LCD.
	This function can be used in Camera open mode or Photo process mode 
Remarks:
	valid
*********************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT16 V5S_HAPI_DrawLCDRctngl(UINT16 startx, UINT16 starty,UINT16 Width,UINT16 Height, HUGE  UINT16 *pwImage)
#else
UINT8 V5H_LCD_Rctngl(UINT16 startx, UINT16 starty,UINT16 Width,UINT16 Height, HUGE  UINT16 *pwImage)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return(V5H_LCD_Rctngl(startx,starty,Width,Height,pwImage));
	else//528
#endif
	{
		return(VIM_HAPI_DrawLCDRctngl(VIM_HAPI_B0_LAYER,startx,  starty,Width, Height, (HUGE  UINT8 *)pwImage));
	}
}
/********************************************************************************
Description:
	Update a region from B layer buffer to lcd panel
Parameters:
	startx: the start x coordinate of region(it must be a whole number multiple by 4)
	starty: the start y coordinate of region
	Width: the width of the Rectangle(pixel)
	Height: the height of the Rectangle(pixel)
	Return:
		V5H_SUCCEED: update ok
		V5H_ERROR_WORKMODE: error work mode
		V5H_FAILED: error parameter(update erea overflow)
	Note:
	This function can be used in Camera open mode or Photo process mode 
Remarks:
State: valid
*********************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT16 V5S_HAPI_UpdateLCD(UINT16 startx, UINT16 starty,UINT16 Width,UINT16 Height)
#else
UINT8 V5H_LCD_Update(UINT16 startx, UINT16 starty,UINT16 Width,UINT16 Height)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return(V5H_LCD_Update(startx,starty,Width,Height));
	else//528
#endif
	{
		return(VIM_HAPI_UpdateLCD(VIM_HAPI_B0_LAYER, startx,  starty, Width,Height));
	}

}

/********************************************************************************
* Description:
*	Select main panel or sub panel
* Parameters:
*	byPanel:	Panel Index.
*				VIM_LCDPANEL_MAIN
*				VIM_LCDPANEL_SUB
* Return:
*	VIM_SUCCEED:				Select the LCD panel success.
*	VIM_ERROR_PARAMETER:		byPanel is unknown.
* Note:
*	1. After init 578, call it to power on panel
*	2. When use high api functions, first call it to switch panel to init LCDC and LCDIF.
State: valid
*********************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT16 V5S_HAPI_SelectLCDPanel(UINT8 byPanel)
#else
UINT8 V5H_LCD_Panel(UINT8 byPanel)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return(V5H_LCD_Panel(byPanel));
	else//528
#endif
	{
		return( VIM_HAPI_SelectLCDPanel((VIM_HAPI_LCDPANEL )byPanel));
	}

}

/********************************************************************************
* Description:
*	Copy pure color into a Rectangle area on LCD( B Layer)
* Parameters:
*	wStartx:		The left coordinate of the Rectangle(pixel)
*	wStarty:		The up coordinate of the Rectangle(pixel)
*	wWidth:			The width of the Rectangle(pixel)
*	wHeight:		The height of the Rectangle(pixel)
*	wRGB565Color:	The color in RGB565 format.
* Return:
*	VIM_SUCCEED:			Copy pure color data to B layer success.
*	VIM_ERROR_PARAMETER:		Rectangle area is out of B win area
*							or wWidth is 0 or wHeight is 0
*	VIM_ERROR_WORKMODE:		Current mode is V5H_MODE_BYPASS or V5H_MODE_DIRECTDISPLAY
* Note:
*	1. Only write data to B layer, but does not update it.
*	2. The data format in B layer is RGB565.
State: valid
*********************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT16 V5S_HAPI_DrawLCDPureColor(UINT16 startx, UINT16 starty,UINT16 Width,UINT16 Height,UINT16 wRGBColor)
#else
UINT8 V5H_LCD_PureColor(UINT16 startx, UINT16 starty,UINT16 Width,UINT16 Height,UINT16 wRGBColor)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return(V5H_LCD_PureColor(startx,starty,Width,Height,wRGBColor));
	else//528
#endif
	{
		return( VIM_HAPI_DrawLCDPureColor(	VIM_HAPI_B0_LAYER ,startx,  starty,Width,Height,wRGBColor));
	}

}

/********************************************************************************
* Description:
*	Set B Layer color depth
* Parameters:
*	528 byColorDepth:	
		VIM_HAPI_COLORDEP_16BIT=6,
		VIM_HAPI_COLORDEP_15BIT=5,
		VIM_HAPI_COLORDEP_12BIT=4,
	568 byColorDepth:	
	   	V5H_COLORDEP_24BIT=4,
		V5H_COLORDEP_15BIT=5,
		V5H_COLORDEP_16BIT=6,

 * Return:
*	VIM_SUCCEED:			Set color depth success.
*	VIM_ERROR_PARAMETER:		byColorDepth is unknown.
*	VIM_ERROR_WORKMODE:		Current mode is V5H_MODE_BYPASS or V5H_MODE_DIRECTDISPLAY
* Note: 
*	 
State: valid
*********************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT16 V5S_HAPI_SetLCDColordep(UINT8 byColorDepth)
#else
UINT8 V5H_LCD_Colordep(UINT8 byColorDepth)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return(V5H_LCD_Colordep((V5H_COLORDEP_MODE)byColorDepth));
	else//528
#endif
	{
		return( VIM_HAPI_SetLCDColordep((VIM_HAPI_COLORDEP_MODE) byColorDepth));
	}

}

/********************************************************************************
* Description:
*	Mirror or flip the display image on LCD, includes A layer and B layer.
* Parameters:
* byMirrorFlipMode:
	Rotate_Degree_0,
	Rotate_Degree_90,
	Rotate_Degree_180,
	Rotate_Degree_270,
	Rotate_Mirror_0,
	Rotate_Mirror_90,
	Rotate_Mirror_180,
	Rotate_Mirror_270,
*528	byABLayer:
	VIM_HAPI_B0_Layer = 1,
	VIM_HAPI_B1_Layer,
	VIM_HAPI_ALL_BLayer,
	VIM_HAPI_A_Layer,
	VIM_HAPI_All_Layer
*568	byABLayer:
	V5H_LCD_ALAYER=0,
	V5H_LCD_BLAYER,
	V5H_LCD_ALL
* Return:
*	VIM_SUCCEED:			Mirror or flip success.
*	VIM_ERROR_PARAMETER:		byMirrorFlipMode is unknown  
*							or byABLayer is unknown  
*	VIM_ERROR_WORKMODE:		Current mode is V5H_MODE_BYPASS or V5H_MODE_DIRECTDISPLAY
* Note:
*	1. Only write data to B layer, but does not update it.
*	2. The data format in B layer is RGB565.
State: valid
*********************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT16 V5S_HAPI_LCDMirror(UINT8 MirrorFlipMode, UINT8 Layer)
#else
UINT8 V5H_LCD_Mirror(UINT8 MirrorFlipMode, UINT8 Layer)
#endif
{
UINT16 Error;
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return(V5H_LCD_Mirror(MirrorFlipMode,(V5H_LCD_LAYER)Layer));
	else//528
#endif
	{
		Error=VIM_HAPI_SetLCDMirror((VIM_HAPI_ROTATEMODE) MirrorFlipMode, (VIM_HAPI_LAYER) Layer);
		if(gVc0528_Info.PreviewStatus.Mode!=VIM_HAPI_PREVIEW_OFF)
			Error=VIM_HAPI_SetPreviewMode(gVc0528_Info.PreviewStatus.Mode);
	}
	return Error;
}





/********************************************************************************
Description:
	get JPG picture size 
Parameters:
	picWidth:  the width of captured image.
	picHeight: the height of captured image.
	Return:
		V5H_SUCCEED: set ok
	Note:
		This function must be used before first preview or change the resolution of display.
		This function must be used in Camera open mode
	Remarks:
	State:Valid.
*********************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT16 V5S_HAPI_GetPictureSize(UINT8 *ptr, UINT32 length, UINT16* SizeX,UINT16* SizeY)
#else
UINT8 V5H_Tool_GetJpegSize(UINT8 *ptr, UINT32 length, UINT16* SizeX,UINT16* SizeY)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return(V5H_Tool_GetJpegSize(ptr,length,SizeX,SizeY));
	else//528
#endif
	{
		return( VIM_HAPI_GetPictureSize(SizeX,SizeY));
	}

}

#ifdef BOTH_5XSUPPORT
UINT16 V5S_HAPI_DisplayVideo(HUGE UINT8* VideoBuf , UINT32 buflen,UINT32 Delay_ms,UINT16 Lcd_OffsetX, UINT16 Lcd_OffsetY, UINT16 Display_W, UINT16 Display_H)
#else
UINT8 V5H_Display_Mjpeg(HUGE UINT8* VideoBuf , UINT32 buflen,UINT32 Delay_ms,UINT16 Lcd_OffsetX, UINT16 Lcd_OffsetY, UINT16 Display_W, UINT16 Display_H)

#endif
{
UINT16 Error;
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return( V5H_Display_Mjpeg(VideoBuf ,buflen,Delay_ms, Lcd_OffsetX, Lcd_OffsetY, Display_W, Display_H));
	else//528
#endif
	{
		Error=VIM_HAPI_ReadyToDisplay(VIM_HAPI_RAM_SAVE, (HUGE void *)VideoBuf, buflen, Lcd_OffsetX, Lcd_OffsetY, Display_W,  Display_H);
		if(Error)
			return Error;
		while (VIM_HAPI_DisplayOneFrame(NULL,0)==VIM_SUCCEED)
		{
			VIM_USER_DelayMs(Delay_ms);
		}
		return Error;
	}
	
}

/***************************************************************************************************************************************
Description:
	display jpeg picture on LCD
Parameters:
      jpegBuf: 	the buffer point of jpeg
      length: 	the length of jpeg (=>jpeg file length)
      LcdOffsetX,LcdOffsetY		: the display coordinate on LCD panel.
      DisplayWidth,DisplayHeigth	: the display width and height.
Return:
	VIM_SUCCEED:  ok
	VIM_ERROR_WORKMODE: error work mode
	VIM_ERROR_YUV_UNKNOW:jpeg file yuv mode error, can not display
	VIM_ERROR_PARAMETER: marb jbuf size less than capture size, so data is not good
	VIM_ERROR_?????????: return form mid api
Note:
	LcdOffsetX+DisplayWidth<=LCD width
	LcdOffsetY+DisplayHeigth<=LCD height
	length=file length(this is important)
Remarks:
state: 
	valid
	****************************************************************************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT16 V5S_HAPI_Display_Jpeg(HUGE UINT8 *jpegBuf, UINT32 length,UINT16 LcdOffsetX, UINT16 LcdOffsetY, UINT16 DisplayWidth, UINT16 DisplayHeight)
#else
UINT8 V5H_Display_Jpeg(HUGE UINT8 *jpegBuf, UINT32 length,UINT16 LcdOffsetX, UINT16 LcdOffsetY, UINT16 DisplayWidth, UINT16 DisplayHeight)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return(V5H_Display_Jpeg(jpegBuf,length, LcdOffsetX, LcdOffsetY,  DisplayWidth,  DisplayHeight));
	else//528
#endif
	{
		return( VIM_HAPI_Display_Jpeg(VIM_HAPI_RAM_SAVE,(HUGE void *)jpegBuf, length, LcdOffsetX, LcdOffsetY,  DisplayWidth,  DisplayHeight));
	}

}
/***************************************************************************************************************************************
Description:
	display pan function(cut the display size for jpeg image to 1:1 display on LCD
Parameters:
      	JpegOffsetX,JpegOffsetY	    :the start coordinate of jpeg picture
      	LcdOffsetX,LcdOffsetY	    : the display coordinate on LCD panel.
      	DisplayWidth,DisplayHeigth: the display width and height.
Return:
	VIM_SUCCEED:  ok
	VIM_ERROR_WORKMODE: error work mode
	VIM_ERROR_YUV_UNKNOW:jpeg file yuv mode error, can not display
	VIM_ERROR_PARAMETER: marb jbuf size less than capture size, so data is not good
	VIM_ERROR_?????????: return form mid api
Note:
Remarks:
state: 
	valid
Modify History:
	Version			Modifyby				DataTime					modified
	   0.1			Amanda Deng			2006.01.08				create first version

****************************************************************************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT16 V5S_HAPI_DisplayPan(HUGE UINT8 *pbyJpegBuf, UINT32 dwBufLength,UINT16 LcdOffsetX, UINT16 LcdOffsetY, UINT16 JpegOffsetX, UINT16 JpegOffsetY,UINT16 DisplayWidth, UINT16 DisplayHeight)
#else
UINT8 V5H_DisplayJpeg_Pan(HUGE UINT8 *pbyJpegBuf, UINT32 dwBufLength,UINT16 LcdOffsetX, UINT16 LcdOffsetY, UINT16 JpegOffsetX, UINT16 JpegOffsetY,UINT16 DisplayWidth, UINT16 DisplayHeight)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return(V5H_DisplayJpeg_Pan(pbyJpegBuf,dwBufLength,JpegOffsetX,  JpegOffsetY, LcdOffsetX, LcdOffsetY,  DisplayWidth,  DisplayHeight));
	else//528
#endif
	{
		return( VIM_HAPI_DisplayPan( LcdOffsetX, LcdOffsetY,  JpegOffsetX,  JpegOffsetY, DisplayWidth, DisplayHeight));
	}

}

/***************************************************************************************************************************************
Description:
	decode a jpeg file to yuv or rgb data file
Parameters:
	DecodeMode: deocde mode select
      	jpegBuf: the buffer point of jpeg
      	pDest: the deocde data store address
      	jpeglength: 	the length of jpeg (=jpeg file length)
      	Destlength:	decode data length
      	TargetWidth,TargetHeight: the display width and height.
Return:
	VIM_SUCCEED:  ok
	VIM_ERROR_WORKMODE: error work mode
	VIM_ERROR_YUV_UNKNOW:jpeg file yuv mode error, can not display
	VIM_ERROR_PARAMETER: marb jbuf size less than capture size, so data is not good
	VIM_ERROR_?????????: return form mid api
Note:
Remarks:
state: 
	valid
Modify History:
	Version			Modifyby				DataTime					modified
	   0.1			Amanda Deng			2006.01.04				create first version

****************************************************************************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT16 V5S_HAPI_Decode_Jpeg(UINT8 DecodeMode,HUGE void *jpegBuf,HUGE UINT8 *pDest,UINT32 jpeglength,UINT32 Destlength,UINT16 TargetWidth,UINT16 TargetHeight)
#else
UINT8 V5H_Decode_Jpeg(UINT8 DecodeMode,HUGE UINT8 *jpegBuf, HUGE UINT8 *pDest,UINT32 jpeglength,UINT32 Destlength,UINT16 TargetWidth,UINT16 TargetHeight)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return(V5H_Decode_Jpeg(DecodeMode,jpegBuf,(HUGE UINT8 *)pDest,jpeglength,Destlength,TargetWidth,TargetHeight));
	else//528
#endif
	{
		return(  VIM_HAPI_Decode_Jpeg(VIM_HAPI_RAM_SAVE, (VIM_HAPI_DECODE_MODE)DecodeMode,(HUGE void *)jpegBuf, (HUGE UINT8 *)pDest,jpeglength,Destlength,TargetWidth,TargetHeight));
	}

}

/********************************************************************************
Description:
	Convert yuv format to rgb format
Parameters:
     *pYUV: the head point of yuv
      *pRgb: the head point of rgb
      Source_Width: the width of yuv pic
      Source_height: the height of rgb pic
	Method:
		V5H_CON_UYVY_565,
		V5H_CON_YUYV_565,
		V5H_CON_YYUV_565,
Return:
		The function status(V5H_FUNCTION_RETURN)

Remarks:
state: 
	valid
*********************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT16 V5S_HAPI_YuvToRgb(HUGE UINT8 *pYUV, HUGE UINT8 *pRgb,UINT16 Source_Width,UINT16 Source_height,UINT8 Method)
#else
UINT8 V5H_Tool_YuvToRgb(HUGE UINT8 *pYUV, HUGE UINT8 *pRgb,UINT16 Source_Width,UINT16 Source_height,UINT8 Method)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return(V5H_Tool_YuvToRgb(pYUV, pRgb,Source_Width,Source_height,Method));
	else //528
#endif
	{
		return( VIM_HAPI_YuvToRgb( pYUV, pRgb,Source_Width,Source_height,Method));
	}

}




/********************************************************************************
Description:
	adjust the Brightness
Parameters:
	mode:
		VIM_HAPI_CHANGE_NORMAL=0,
		VIM_HAPI_CHANGE_ADD,
		VIM_HAPI_CHANGE_DEC,
		VIM_HAPI_SET_STEP
	step: add or decrease value
		VIM_STEP_DEFAULT,
		VIM_BRIGHT_STEP_ONE,
		VIM_BRIGHT_STEP_TWO,
		VIM_BRIGHT_STEP_THREE,
	Return:
		VIM_SUCCEED: set brightness ok
		VIM_ERROR_WORKMODE: error work mode
	Note:
		This function must be used in Camera open mode, and only be effect with RGB sensor
	Remarks:
		state: valid

*********************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT16 V5S_HAPI_Preview_Brightness(UINT8 Mode,UINT8 Step)
#else
UINT8 V5H_Preview_Brightness(UINT8 Mode,UINT8 Step)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return(V5H_Preview_Brightness(Mode,Step));
	else //528
#endif
	{
		return( VIM_HAPI_SetPreviewBrightness((VIM_HAPI_CHANGE_MODE)Mode,Step));
	}

}

/********************************************************************************
Description:
	adjust the Contrast
Parameters:
	mode:
		VIM_HAPI_CHANGE_NORMAL=0,
		VIM_HAPI_CHANGE_ADD,
		VIM_HAPI_CHANGE_DEC,
		VIM_HAPI_SET_STEP
	step: add or decrease value
		VIM_STEP_DEFAULT,
		VIM_CONTRAST_STEP_ONE,
		VIM_ CONTRAST _STEP_TWO,
		VIM_ CONTRAST _STEP_THREE,
		VIM_ CONTRAST _STEP_FORE,
		VIM_ CONTRAST _STEP_FIVE
	Return:
		VIM_SUCCEED: set contrast ok
		VIM_ERROR_WORKMODE: error work mode
	Note:
		This function must be used in Camera open mode, and only be effect with RGB sensor
	Remarks:
		state: valid
*********************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT16 V5S_HAPI_Preview_Contrast(UINT8 Mode,UINT8 Step)
#else
UINT8 V5H_Preview_Contrast(UINT8 Mode,UINT8 Step)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return(V5H_Preview_Contrast(Mode,Step));
	else //528
#endif
	{
		return( VIM_HAPI_SetPreviewContrast((VIM_HAPI_CHANGE_MODE)Mode,Step));
	}

}

/********************************************************************************
Description:
	mirror or flip the image from sensor. 
Parameters:
	mode:
		VIM_ SENSOR_NORMAL:  defalut value
		VIM_ SENSOR _MIRROR:  left right mirror
		VIM_ SENSOR _FLIP:        up down mirror
		VIM_ SENSOR _ROTATION: mirror+flip
	Return:
		VIM_SUCCEED: set mirror ok
		VIM_ERROR_WORKMODE: error work mode
	Note:
		This function must be used in Camera open mode
Remarks:
	state: valid
*********************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT16 V5S_HAPI_Preview_Mirror(UINT8 mode)
#else
UINT8 V5H_Preview_Mirror(UINT8 mode)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return(V5H_Preview_Mirror(mode));
	else //528
#endif
	{
		return(VIM_HAPI_SetPreviewMirror((VIM_HAPI_SENSORMIRROR)mode));
	}

}



/********************************************************************************
Description:

Parameters:
	mode:
		V5H_EXP_UNKOWN=0,
		V5H_EXP_INDOOR=1,
		V5H_EXP_OUTDOOR,
		V5H_EXP_NIGHT
	Return:
		VIM_SUCCEED: set mirror ok
		VIM_ERROR_WORKMODE: error work mode
	Note:
		This function must be used in Camera open mode
Remarks:
	state: valid
*********************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT8 V5S_Preview_ExposureMode(UINT8 mode)
#else
UINT8 V5H_Preview_ExposureMode(UINT8 mode)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return(V5H_Preview_ExposureMode(mode));
	else //528
#endif
	{
		return(VIM_HAPI_SetPreviewExMode((VIM_HAPI_SENSORMODE)mode));
	}
	
}

/********************************************************************************
Description:

Parameters:
	mode:
		V5H_EXP_UNKOWN=0,
		V5H_EXP_INDOOR=1,
		V5H_EXP_OUTDOOR,
		V5H_EXP_NIGHT
	Return:
		VIM_SUCCEED: set mirror ok
		VIM_ERROR_WORKMODE: error work mode
	Note:
		This function must be used in Camera open mode
Remarks:
	state: valid
*********************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT8 V5S_Preview_ExposureTYPE(UINT8 mode)
#else
UINT8 V5H_Preview_ExposureTYPE(UINT8 mode)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return(V5H_Preview_ExposureMode(mode));
	else //528
#endif
	{
		return(VIM_HAPI_SetPreviewExMode((VIM_HAPI_SENSORMODE)(mode+VIM_HAPI_SENSOR_60HZ)));
	}
	
}
/********************************************************************************
Description:
	Set special effect
Parameters:
	Mode:
		 VIM_HAPI_SPECIAL_NORMAL.
		 VIM_HAPI_SPECIAL_MONOCHROME,	 //ºÚ°×
		 VIM_HAPI_SPECIAL_SEPIA,		//»³¾É
		 VIM_HAPI_SPECIAL_NEGATIVE,	//¸ºÆ¬
		 VIM_HAPI_SPECIAL_RELIEF,//¸¡µñ
		 VIM_HAPI_SPECIAL_SKETCH	//ËØÃè: only RGB sensor
		 VIM_HAPI_SPECIAL_REDONLY,	 //ºìÉ«Í»³ö
	Y = ( 77R + 150G + 29B)/256
	U = (-43R - 85G + 128B)/256 + 128
	V = (128R - 107G - 21B)/256 + 128
	Return:
		VIM_SUCCEED: set effect ok
		VIM_ERROR_WORKMODE: error work mode
	Note:
		This function must be used in Camera open mode
Remarks:
	state: 	valid
*********************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT16 V5S_HAPI_Preview_Effect(UINT8 Mode)
#else
UINT8 V5H_Preview_Effect(UINT8 Mode)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return(V5H_Preview_Effect(Mode));
	else //528
#endif
	{
		return(VIM_HAPI_SetEffect((VIM_HAPI_SPECIAL_EFFECT)Mode));
	}
}

#ifdef BOTH_5XSUPPORT
HUGE UINT8* V5S_Capture_GetMultishotFrame(UINT8 *framebuf, UINT32 buflength, UINT8 framenumber, UINT32 *pframelength)
#else
HUGE UINT8* V5H_Capture_GetMultishotFrame(UINT8 *framebuf, UINT32 buflength, UINT8 framenumber, UINT32 *pframelength)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return(V5H_Capture_GetMultishotFrame(framebuf, buflength, framenumber, pframelength));
	else
#endif
		return VIM_HAPI_GetOneFrame(framebuf, buflength, framenumber, pframelength);
}


/********************************************************************************
Description:
	Capture AVI 
Parameters:
      interval: in each 16 frames from sensor, which frame is discard(0), which frame is captured(1)
      		    total 16bits, each bit means this frame is discard or captured.
      	TotalFrame: the customer can use the parameter to contral the total frame.
      			and this parameter also decide the buf have be wasted.
      VideoBuf: the buffer to store the video(MJPEG)
      Buflength: the length of VideoBuf 
      pUCallBack: callback function
Return:
	V5H_SUCCEED:  ok
	V5H_ERROR_WORKMODE(V5H_ERROR_PREVIEWMODE): error work mode
	V5H_ERROR_PREVIEWMODE: preview error
Note:
	if(pUCallBack==NULL) this function will return after finish capturing,
	else it will return before  finish capturing, users should judge the end in callback function.
Remarks:
	This function should be called when preview on.
       pUCallBack return status:
 	V5H_CAPTURE_END: capture end
	V5H_BUF_ERROR:     buffer length is not enough for JPEG   
state: 
	valid
*********************************************************************************/
/*
#ifdef BOTH_5XSUPPORT
UINT8 V5S_HAPI_Capture_AVI(UINT16 interval,UINT16 TotalFrame,UINT8 *VideoBuf, UINT32 Buflength, V5H_CallBack pUCallBack)
#else
UINT8 V5H_Capture_AVI(UINT16 interval,UINT16 TotalFrame,UINT8 *VideoBuf, UINT32 Buflength, V5H_CallBack pUCallBack)
#endif
{
AviInfo info;
	if(IsVc0568Flag==1)//vc568
	{
		return(V5H_Capture_AVI(interval, TotalFrame, VideoBuf, Buflength,pUCallBack));
	}
	else
	{
			info.dFileMaxSize=Buflength;
			info.wCaptureWidth=gVc0528_Info.CaptureStatus.Size.cx;
			info.wCaptureHeight=gVc0528_Info.CaptureStatus.Size.cy;
			info.wFrameRate=3;
			Avi_LoadInfo(&info,RAMSAVE,pUCallBack);
			return(Avi_StartCapture(VideoBuf));
	}
}
*/

/********************************************************************************
Description:
	Display avi  jpeg on LCD
Parameters:
      jpegBuf: the buffer point of jpeg
      length: the real  length of  vmi jpeg 
      Lcd_OffsetX,Lcd_OffsetY: the display coordinate on LCD panel.
     Display_W,Display_H: the display width and height.
Return:
	V5H_SUCCEED:  ok
	V5H_ERROR_WORKMODE(V5H_ERROR_PREVIEWMODE): error work mode
	V5H_FAILED: JPEG error
Note:
	first call V5H_DisplayAVI_Jpeg_Init,then only call V5H_DisplayAVI_Jpeg 
Remarks:
state: 
	valid
*********************************************************************************/
UINT8 vmi_flag=0;
#ifdef BOTH_5XSUPPORT
void V5S_VMI_init(void)
#else
void V5H_VMI_init(void)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		(V5H_VMI_init());
	else
#endif
		vmi_flag=1;
		//return VIM_HAPI_ReadyToDisplay();

}
#ifdef BOTH_5XSUPPORT
UINT8 V5S_Display_Vmi(UINT8 *jpegBuf, UINT32 length,UINT16 Lcd_OffsetX, UINT16 Lcd_OffsetY, UINT16 Display_W, UINT16 Display_H)
#else
UINT8 V5H_Display_Vmi(UINT8 *jpegBuf, UINT32 length,UINT16 Lcd_OffsetX, UINT16 Lcd_OffsetY, UINT16 Display_W, UINT16 Display_H)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return(V5H_Display_Vmi(jpegBuf,length,Lcd_OffsetX,Lcd_OffsetY,Display_W,Display_H));
	else
#endif
	{
		if(vmi_flag)
		{
			VIM_HAPI_ReadyToDisplay(VIM_HAPI_RAM_SAVE,jpegBuf,length,Lcd_OffsetX,Lcd_OffsetY,Display_W,Display_H);
			vmi_flag=0;
		}
		return(VIM_HAPI_QuickDisplayOneFrame(jpegBuf,length));
	}
}

#ifdef BOTH_5XSUPPORT
void V5S_VMI_End(void)
#else
void V5H_VMI_End(void)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		V5H_VMI_End();
	//else
#endif
}
/********************************************************************************
Description:
	interrupt service routine
Parameters:

Return:
	V5H_SUCCEED:  ok
	V5H_ERROR_WORKMODE: error work mode
Note:
	This function should be called by the user's CPU's external interrupt ISR, or a 50ms timer when 
	start preview or start capture.
		
Remarks:
state: 
	valid
*********************************************************************************/
#ifdef BOTH_5XSUPPORT
UINT8 V5S_HAPI_Timer(void)
#else
UINT8 V5H_IntHandle(void)
#endif
{
#ifdef BOTH_5XSUPPORT
	if(IsVc0568Flag==1)//vc568
		return((UINT16)V5H_IntHandle());
	else
#endif
		VIM_HAPI_Timer();
		return 0;
}


#endif
