/*************************************************************************
*                                                                       
*                Copyright (C) 2005 Vimicro CO.,LTD     		 
*
* FILE NAME                                  VERSION                
*                                                                       
* VIM_HIGH_API.c			   		1.4      
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*     VC0528 's USER aip file                        
*
*	Version		Author			Date		Description		
*  ---------------------------------------------------------------						 
*   0.1			angela		2005-11-15	The first version. 
*   0.2			angela  		2006-06-06	update for528
*   0.3			angela  		2006-12-08	change to 1.0 version 
*   1.1			angela  		2006-12-19	update
*   1.2			angela  		2007-2-25	update
*   1.3			angela  		2007-3-22	update
*   1.4			guoying		2007-08-03	update
*   1.5			guoying 		2007-12-25	update
*   1.6			guoying 		2008-07-03	update
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
static char g_Code_Version[]={" VC0528 V1.6 2008-07-03 "};
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
UINT16 VIM_HAPI_InitVc05x(void)
{
VIM_RESULT result;
//#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
	VIM_USER_PrintString((char *)g_Code_Version);
//#endif	
	VIM_MAPI_InitIfo();
	result=VIM_MAPI_InitHif();
	if(result)
		return (UINT16)result;
	  
	//Init sensor
	result=VIM_MAPI_AutoFindSensor();
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
	if(result)
		VIM_USER_PrintString("have not find sensor");
	else
		VIM_USER_PrintString((char *)gVc0528_Info.pSensorInfo->desc);
#endif	

	
	//Init lcd
	VIM_LCDIF_ResetPanel();
	VIM_MAPI_SwitchPanel(VIM_HAPI_LCDPANEL_MAIN);
	VIM_HAPI_SetLCDColordep(VIM_HAPI_COLORDEP_16BIT);
	// init interrupt
	VIM_HIF_ClearIntModule(INT_ALL);
	VIM_HAPI_SetWorkMode(VIM_HAPI_MODE_BYPASS);
	
	return VIM_SUCCEED;
}
/********************************************************************************
Description:
	Set 5x chip work mode
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

UINT16 VIM_HAPI_SetWorkMode(VIM_HAPI_WORKMODE WorkMode)
{
VIM_RESULT result;

	if(WorkMode==gVc0528_Info.ChipWorkMode)
		return VIM_SUCCEED;
	if((gVc0528_Info.ChipWorkMode==VIM_HAPI_MODE_BYPASS)||(gVc0528_Info.ChipWorkMode==VIM_HAPI_MODE_DIRECTDISPLAY))
		{
			VIM_SET_XCLKON();
			VIM_HIF_SetBypassOrNormal((PVIM_BYPASS_DEF)&gVc0528_Info.pUserInfo->BypassDef,
				VIM_HIF_NORMALTYPE);
			if((gVc0528_Info.ChipWorkMode==VIM_HAPI_MODE_BYPASS))
			{
				if(VIM_BYPASS_RESETCHIP)
					VIM_USER_Reset();
				result=VIM_MAPI_InitHif();
				if(result)
				{
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintDec("\n******** VIM_HAPI_SetWorkMode error*********** ,result =",result);
#endif	
					return (UINT16)result;
				}
				VIM_SIF_SifInitI2c(gVc0528_Info.pSensorInfo,
					gVc0528_Info.pUserInfo->WorkMClk.Clkout); // Init i2c
				result = VIM_SIF_SifSensorInit(gVc0528_Info.pSensorInfo);
				if (result) 
				{
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
					VIM_USER_PrintDec("\n sensor init error,result =",result);
#endif	
			 		 // return	result;
				}
			}
			VIM_DISP_SetGbufFormat((VIM_DISP_BFORMAT)gVc0528_Info.LcdStatus.ColorDep); //angela 2006-8-3 for changed value
			VIM_HIF_SetModClkOn(VIM_HIF_CLOCK_ALL);
			VIM_HIF_SetExterPinCrlEn(VIM_HIF_PIN_ALL,DISABLE);// 09-21 enable from eternal
			VIM_MAPI_SwitchPanel(gVc0528_Info.LcdStatus.NowPanel);
		}
	VIM_HIF_SetModClkOn(VIM_HIF_CLOCK_ALL);

	switch(WorkMode)
	{
		case VIM_HAPI_MODE_CAMERAON:
			VIM_MARB_Set1TSramMode(VIM_MARB_1TSRAM_ALL,VIM_MARB_1TSRAM_POWERON);
			VIM_SIF_SetSensorState(gVc0528_Info.pSensorInfo,VIM_SIF_SENSOR_POWERON);
			VIM_HIF_SetModClkClose(VIM_HIF_CLOCK_GE);
			break;
		case VIM_HAPI_MODE_BYPASS:
			VIM_HAPI_SetPreviewMode(VIM_HAPI_PREVIEW_OFF);
			VIM_SIF_SetSensorState(gVc0528_Info.pSensorInfo,VIM_SIF_SENSOR_STANDBY);

			
			VIM_HIF_SetExterPinValue(VIM_HIF_PIN_ALL,1);

			VIM_HIF_SetExterPinValue(VIM_HIF_PIN_INT,VIM_BYPASS_PIN_INT);
			VIM_HIF_SetExterPinValue(VIM_HIF_PIN_LCDRES,VIM_BYPASS_PIN_LCDRES);	
			VIM_HIF_SetExterPinValue(VIM_HIF_PIN_CSRSTN,VIM_BYPASS_PIN_CSRSTN);
			VIM_HIF_SetExterPinValue(VIM_HIF_PIN_CSPWD,VIM_BYPASS_PIN_CSPWD);		
			VIM_HIF_SetExterPinValue(VIM_HIF_PIN_CSENB,VIM_BYPASS_PIN_CSENB);
			VIM_HIF_SetExterPinValue(VIM_HIF_PIN_GPIO4,VIM_BYPASS_PIN_GPIO4);		
			VIM_HIF_SetExterPinValue(VIM_HIF_PIN_GPIO3,VIM_BYPASS_PIN_GPIO3);	
			VIM_HIF_SetExterPinValue(VIM_HIF_PIN_GPIO2,VIM_BYPASS_PIN_GPIO2);	
			VIM_HIF_SetExterPinValue(VIM_HIF_PIN_GPIO1,VIM_BYPASS_PIN_GPIO1);	
			VIM_HIF_SetExterPinValue(VIM_HIF_PIN_GPIO0,VIM_BYPASS_PIN_GPIO0);	
			VIM_HIF_SetExterPinCrlEn(VIM_HIF_PIN_ALL,ENABLE);// 09-21 enable from eternal
			
			VIM_MARB_Set1TSramMode(VIM_MARB_1TSRAM_ALL,VIM_MARB_1TSRAM_OFF);
			VIM_HIF_SetModClkClose(VIM_HIF_CLOCK_ALL);
			VIM_HIF_SetLdoStatus(VIM_LDO_OFF);
			VIM_HIF_SetPllStatus(VIM_HIF_PLLPOWERDOWN);		
			VIM_HIF_SetBypassOrNormal((PVIM_BYPASS_DEF)&gVc0528_Info.pUserInfo->BypassDef,
				VIM_HIF_BYPASSTYE);
			VIM_SET_XCLKOFF();
			break;
		case VIM_HAPI_MODE_PHOTOPROCESS:		
			VIM_HAPI_SetPreviewMode(VIM_HAPI_PREVIEW_OFF);
			VIM_SIF_SetSensorState(gVc0528_Info.pSensorInfo,VIM_SIF_SENSOR_STANDBY);
			VIM_MARB_Set1TSramMode(VIM_MARB_1TSRAM_ALL,VIM_MARB_1TSRAM_POWERON);
			VIM_HIF_SetModClkClose(VIM_HIF_CLOCK_GE|VIM_HIF_CLOCK_SIF|VIM_HIF_CLOCK_ISP);
			break;
		case VIM_HAPI_MODE_DIRECTDISPLAY:
			VIM_HAPI_SetPreviewMode(VIM_HAPI_PREVIEW_OFF);
			VIM_MARB_Set1TSramMode(VIM_MARB_1TSRAM_ALL,VIM_MARB_1TSRAM_STANDBY);
			VIM_HIF_SetModClkClose(VIM_HIF_CLOCK_ALL);
			VIM_HIF_SetBypassOrNormal((PVIM_BYPASS_DEF)&gVc0528_Info.pUserInfo->BypassDef,
				VIM_HIF_BYPASSTYE);
			break;
		default:
			return VIM_ERROR_PARAMETER;
	}
	gVc0528_Info.ChipWorkMode=WorkMode;
	return VIM_SUCCEED;
}

UINT16 VIM_HAPI_GetWorkMode(void)
{
   return gVc0528_Info.ChipWorkMode;
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
UINT16 VIM_HAPI_SetCaptureParameter(UINT16 CapWidth, UINT16 CapHeight)
{

	if(gVc0528_Info.ChipWorkMode!=VIM_HAPI_MODE_CAMERAON)
		return VIM_ERROR_WORKMODE;
	gVc0528_Info.CaptureStatus.Size.cx=(CapWidth>>2)<<2;
	gVc0528_Info.CaptureStatus.Size.cy=(CapHeight>>1)<<1;
	gVc0528_Info.CaptureStatus.ReCapSize=0;
	if(CapWidth%16||CapHeight%8)		//angela 2007-3-12
		return VIM_ERROR_CAPTURE_PARAMETER;
	return VIM_SUCCEED;
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
UINT16 VIM_HAPI_SetPreviewParameter(UINT16 Lcd_OffsetW,UINT16 Lcd_OffsetH,UINT16 Width, UINT16 Height)
{

	if(gVc0528_Info.ChipWorkMode!=VIM_HAPI_MODE_CAMERAON)
		return VIM_ERROR_WORKMODE;
	
	gVc0528_Info.PreviewStatus.Size.cx=Width&0xfffe;
	gVc0528_Info.PreviewStatus.Size.cy=Height; 			//if preview mode is linebuf mode,it must be mult of 8
	gVc0528_Info.PreviewStatus.Point.x=Lcd_OffsetW&0xfffc;
	gVc0528_Info.PreviewStatus.Point.y=Lcd_OffsetH;


	return VIM_SUCCEED;
}
/********************************************************************************
Description:
	SET preview MODE
Parameters:
	MODE: preview mode
		VIM_PREVIEW_OFF: preview off
		VIM_HAPI_PREVIEW_ON: preview on without frame
		VIM_HAPI_PREVIEW_FRAMEON: preview on with frame

	Return:
		VIM_SUCCEED: set mode ok
		VIM_ERROR_WORKMODE: error work mode
		VIM_ERROR_????????: Return from middle API function
	Note:
		This function must be used in Camera open mode
	Remarks:
		state: valid
*********************************************************************************/
UINT16 VIM_HAPI_SetPreviewMode(VIM_HAPI_PREVIEW_MODE MODE)
{
VIM_RESULT result;
TSize Amem,InputSize,CaptureSize,SourceWin;
TPoint pt;
	
	if(gVc0528_Info.ChipWorkMode!=VIM_HAPI_MODE_CAMERAON)
		return VIM_ERROR_WORKMODE;

       switch(MODE)
	{
		case VIM_HAPI_PREVIEW_ON:
		case VIM_HAPI_PREVIEW_FRAMEON:		
			VIM_SIF_EnableSyncGen(DISABLE);
			VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER,DISABLE);
#ifndef CONFIG_MACH_CANOPUS
			VIM_MAPI_Delay_Frame(1);
#endif
			//set chip work mode
                    if(MODE!=VIM_HAPI_PREVIEW_FRAMEON)
		 		VIM_MAPI_SetChipMode(VIM_MARB_PREVIEW_MODE,VIM_IPP_HAVE_NOFRAME);	//actually capture mode
			else
			{
				VIM_MAPI_SetChipMode(VIM_MARB_PREVIEW_MODE,VIM_IPP_HAVEFRAME);
			}
			gVc0528_Info.PreviewStatus.bOutputIndex=VIM_HAPI_MAXPIXEL;
                  	result = VIM_SIF_SetSensorResolution(gVc0528_Info.pSensorInfo,
                  		(VIM_SIF_RESOLUTION)gVc0528_Info.PreviewStatus.bOutputIndex);
			if(result)
		           return (UINT16)result;
			//get input size
			result = VIM_SIF_GetSensorResolution(gVc0528_Info.pSensorInfo, 
				(VIM_SIF_RESOLUTION)gVc0528_Info.PreviewStatus.bOutputIndex, &InputSize);
			if(result)
		           return (UINT16)result;

			VIM_IPP_SetImageSize(InputSize.cx, InputSize.cy);
			CaptureSize=gVc0528_Info.CaptureStatus.Size;

			//change capture size for fast preview mode	//7/25/2007 guoying add 
			if((CaptureSize.cx>InputSize.cx)||(CaptureSize.cy>InputSize.cy))
			{
				if(VIM_USER_FAST_PREVIEW)
				
				{
					CaptureSize.cx/=2;
					CaptureSize.cy/=2;
					
				}
			}

			//get source window size
			result = VIM_IPP_ToolCaculateLessSrcWindow(InputSize,CaptureSize,&SourceWin);
			if(result)
				return (UINT16)result;
			//SourceWin.cx=InputSize.cx;		//guoying 3/3/2008
			//SourceWin.cy=InputSize.cy;	
			
			//get source window start position	
			pt.x=((InputSize.cx-SourceWin.cx)/2)&0xfffe;
			pt.y=((InputSize.cy-SourceWin.cy)/2)&0xfffe;
			result = VIM_IPP_SetCaptureSize(gVc0528_Info.pSensorInfo,pt, SourceWin, CaptureSize); 
			if(result)
				return (UINT16)result;
			
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
			VIM_USER_PrintDec("\n sensor output width: ",InputSize.cx);
			VIM_USER_PrintDec("\n sensor output height: ",InputSize.cy);
			VIM_USER_PrintDec("\n Source window width: ",SourceWin.cx);
			VIM_USER_PrintDec("\n Source window height: ",SourceWin.cy);
			VIM_USER_PrintDec("\n Capture window width:  ",CaptureSize.cx);
			VIM_USER_PrintDec("\n Capture window height:  ",CaptureSize.cy);
#endif	

			//get part size of source win will display in LCD
			result = VIM_IPP_ToolCaculateBigDstWindow(SourceWin, gVc0528_Info.PreviewStatus.Size,&InputSize);
			if(result)
				return (UINT16)result;

			//InputSize.cx=gVc0528_Info.PreviewStatus.Size.cx;		//guoying 3/3/2008
			//InputSize.cy=gVc0528_Info.PreviewStatus.Size.cy;
			
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
			VIM_USER_PrintDec("\n  display InputSize window width:",InputSize.cx);
			VIM_USER_PrintDec("\n  display InputSize window height:  ",InputSize.cy);
#endif	

			if(InputSize.cx<gVc0528_Info.PreviewStatus.Size.cx)	//angela 2007-3-7
				InputSize.cx=gVc0528_Info.PreviewStatus.Size.cx;
			if(InputSize.cy<gVc0528_Info.PreviewStatus.Size.cy)
				InputSize.cy=gVc0528_Info.PreviewStatus.Size.cy;			
			
			result = VIM_IPP_SetDispalySize( pt, SourceWin, InputSize);
	 		if(result)
				return (UINT16)result;
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
			VIM_USER_PrintDec("\n new display InputSize window width:",InputSize.cx);
			VIM_USER_PrintDec("\n new display InputSize window height:  ",InputSize.cy);
			VIM_USER_PrintDec("preivew  window width:",gVc0528_Info.PreviewStatus.Size.cx);
			VIM_USER_PrintDec("preivew window height:  ",gVc0528_Info.PreviewStatus.Size.cy);			
#endif	

			result = VIM_IPP_SetThumbSize((UINT8)gVc0528_Info.CaptureStatus.ThumbSize.cx, (UINT8)gVc0528_Info.CaptureStatus.ThumbSize.cy,VIM_IPP_THUMB_FROM_CAPTURE);
			if(result)
				return (UINT16)result;
			
	 		gVc0528_Info.PreviewStatus.ZoomPara.InitSourceSize=SourceWin;
	 		gVc0528_Info.PreviewStatus.ZoomPara.InitAMemSize = InputSize;
	 		pt.x=(((InputSize.cx-gVc0528_Info.PreviewStatus.Size.cx)>>1)>>1)<<1;
			pt.y=(InputSize.cy-gVc0528_Info.PreviewStatus.Size.cy)>>1;
			VIM_DISP_SetA_Memory(pt,InputSize);
			
			VIM_DISP_SetA_DisplayPanel(gVc0528_Info.PreviewStatus.Point,gVc0528_Info.PreviewStatus.Size); //angela 2007-3-7

			result = VIM_MAPI_AdjustPoint((VIM_DISP_ROTATEMODE)gVc0528_Info.LcdStatus.ARotationMode,
					VIM_DISP_NOTCHANGE);
			if(result)
				return (UINT16)result;
			VIM_DISP_SetRotateMode(VIM_DISP_ALAYER,(VIM_DISP_ROTATEMODE)gVc0528_Info.LcdStatus.ARotationMode);
			
			VIM_JPEG_SetSize(JPEG_422,CaptureSize);
			// angela added it for set capture ratio before preview on
			result=VIM_MAPI_SetCaptureRatio(gVc0528_Info.CaptureStatus.QualityMode);

			if(result)
				return (UINT16)result;
			gVc0528_Info.MarbStatus.ALayerMode = VIM_USER_PREVIEW_ALAYER_MODE;//ALAYER_1FRAMEBUF;
CHAGNEAMODE:
			result =VIM_MARB_SetMap(gVc0528_Info.MarbStatus.WorkMode,gVc0528_Info.MarbStatus.ALayerMode,
		                  (PVIM_MARB_Map)&gVc0528_Info.MarbStatus.MapList);
			if(result)
			{
				if(gVc0528_Info.MarbStatus.ALayerMode == VIM_DISP_LINEBUF)
					return (UINT16)result;
				else if(gVc0528_Info.MarbStatus.ALayerMode == VIM_DISP_TWOFRAME)
					{
						gVc0528_Info.MarbStatus.ALayerMode =VIM_DISP_ONEFRAME;
						goto CHAGNEAMODE;
					}
				else
					{
						gVc0528_Info.MarbStatus.ALayerMode =VIM_DISP_LINEBUF;
						goto CHAGNEAMODE;
					}
			}
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
				VIM_USER_PrintDec(" user a layter mode :",gVc0528_Info.MarbStatus.ALayerMode);
#endif	
			////////////zoom ///////////////////
			if ( gVc0528_Info.PreviewStatus.ZoomPara.InitSourceSize.cx >= 
				(CaptureSize.cx/2 + VIM_USER_MIN_PIXEL_ONESTEP * VIM_USER_MAX_STEP))
			{
				result=VIM_MAPI_Preview_CaptureZoom(gVc0528_Info.PreviewStatus.ZoomPara.CurrentZoomStep);
			}
			else
			{
				result=VIM_MAPI_Preview_DisplayZoom(gVc0528_Info.PreviewStatus.ZoomPara.CurrentZoomStep);
			}
			VIM_JPEG_SetBitRateControlEn(ENABLE);//angela   
			VIM_IPP_SetDisplayDropFrame(0xffff);
			VIM_SIF_EnableSyncGen(ENABLE);	
			
			VIM_MAPI_Delay_Frame(2);   //Delay for the stable sensor data output
			VIM_DISP_ResetState();
			VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER,ENABLE);				
			break;
		case VIM_HAPI_PREVIEW_OFF:
		default:
			VIM_USER_StopTimer();
			VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER,DISABLE);//angela 2007-1-25
#ifndef CONFIG_MACH_CANOPUS
			VIM_MAPI_Delay_Frame(1);
		 	VIM_SIF_EnableSyncGen(DISABLE);//next frame
			VIM_USER_DelayMs(50);/*added by shiyong for bug10586*/
#else
		 	VIM_SIF_EnableSyncGen(DISABLE);//next frame
			VIM_USER_DelayMs(1);
#endif
		 	pt.x=pt.y=Amem.cx=Amem.cy=0;
		 	VIM_DISP_SetA_Memory(pt,Amem);
		}
	gVc0528_Info.PreviewStatus.Mode=MODE;
	return VIM_SUCCEED;
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
UINT16 VIM_HAPI_SetPreviewZoom(VIM_HAPI_CHANGE_MODE Mode,UINT8 step)
{
	VIM_RESULT Result;
	TSize CaptureSize=gVc0528_Info.CaptureStatus.Size,InputSize;
	if(gVc0528_Info.ChipWorkMode==VIM_HAPI_MODE_BYPASS)
		return VIM_ERROR_WORKMODE;

	VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER,DISABLE);	
	VIM_MAPI_Delay_Frame(1);   //Delay for the stable sensor data output
	switch(Mode)
	{
		case VIM_HAPI_CHANGE_ADD:
			if (++gVc0528_Info.PreviewStatus.ZoomPara.CurrentZoomStep >VIM_USER_MAX_STEP)
				gVc0528_Info.PreviewStatus.ZoomPara.CurrentZoomStep = VIM_USER_MAX_STEP;
			break;
		case VIM_HAPI_CHANGE_DEC:
			if(gVc0528_Info.PreviewStatus.ZoomPara.CurrentZoomStep)
				gVc0528_Info.PreviewStatus.ZoomPara.CurrentZoomStep--;
			break;
		case VIM_HAPI_SET_STEP:
			gVc0528_Info.PreviewStatus.ZoomPara.CurrentZoomStep = step;
			break;
		case VIM_HAPI_CHANGE_NORMAL:
			gVc0528_Info.PreviewStatus.ZoomPara.CurrentZoomStep=0;
		default:
			break;
	}

	if (gVc0528_Info.PreviewStatus.Mode == VIM_HAPI_PREVIEW_OFF)
		return VIM_SUCCEED;
//change capture size
	VIM_SIF_GetSensorResolution(gVc0528_Info.pSensorInfo, (VIM_SIF_RESOLUTION)gVc0528_Info.PreviewStatus.bOutputIndex, &InputSize);


	if ( gVc0528_Info.PreviewStatus.ZoomPara.InitSourceSize.cx >= 
		(CaptureSize.cx/2 + VIM_USER_MIN_PIXEL_ONESTEP * VIM_USER_MAX_STEP))
	{
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
			VIM_USER_PrintString("\n capture zoom:");
#endif	
		Result=VIM_MAPI_Preview_CaptureZoom(gVc0528_Info.PreviewStatus.ZoomPara.CurrentZoomStep);
	}
	else
	{
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
			VIM_USER_PrintString("\n display zoom:");
#endif	
		Result=VIM_MAPI_Preview_DisplayZoom(gVc0528_Info.PreviewStatus.ZoomPara.CurrentZoomStep);
	}

	VIM_MAPI_Delay_Frame(2);   //Delay for the stable sensor data output
	VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER,ENABLE);	
	return (UINT16)Result;
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
UINT16 VIM_HAPI_SetCaptureQuality(VIM_HAPI_CPTURE_QUALITY Quality)
{
  VIM_RESULT Result;
       if(gVc0528_Info.ChipWorkMode!=VIM_HAPI_MODE_CAMERAON)
		return VIM_ERROR_WORKMODE;
       gVc0528_Info.CaptureStatus.QualityMode=Quality;
       if (gVc0528_Info.PreviewStatus.Mode == VIM_HAPI_PREVIEW_OFF)
		return VIM_SUCCEED;
	Result=VIM_MAPI_SetCaptureRatio(Quality);
	return (UINT16)Result;		 
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
UINT16 VIM_HAPI_SetCaptureRatio(UINT8 ratio)
{
  VIM_RESULT Result;
       if(gVc0528_Info.ChipWorkMode!=VIM_HAPI_MODE_CAMERAON)
		return VIM_ERROR_WORKMODE;
       gVc0528_Info.CaptureStatus.QualityMode=ratio;
	Result=VIM_MAPI_SetCaptureRatio(ratio);
	return (UINT16)Result;		 
}
/********************************************************************************
Description:
	Capture a still(jpeg)
Parameters:
      SaveMode:
      				VIM_HAPI_RAM_SAVE,   save in the still buf (UINT8)
				VIM_HAPI_ROM_SAVE  stillbuf is the file id  of file 
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
UINT16 VIM_HAPI_CaptureStill(VIM_HAPI_SAVE_MODE SaveMode,HUGE void *StillBuf,UINT32 BUF_Length,VIM_HAPI_CallBack pUCallBack)
{
	UINT32 dwCount=500000,dwTwc;
	VIM_RESULT result;

	//check the chip working mode 
	if(gVc0528_Info.ChipWorkMode!=VIM_HAPI_MODE_CAMERAON) 
		return VIM_ERROR_WORKMODE;
	//check the preview  mode 
      	if(gVc0528_Info.PreviewStatus.Mode==VIM_HAPI_PREVIEW_OFF)
       		return VIM_ERROR_PREVIEWMODE;

	//check if capture still or frame
       if(gVc0528_Info.PreviewStatus.Mode!=VIM_HAPI_PREVIEW_FRAMEON)
       {
		VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER,DISABLE);	
		VIM_MAPI_Delay_Frame(1);
		gVc0528_Info.PreviewStatus.Mode=VIM_HAPI_PREVIEW_OFF;
		result=VIM_MAPI_FastPreview();					//7/25/2007 guoying add
		if(result)
			return (UINT16)result;

		VIM_MARB_SetMap(gVc0528_Info.MarbStatus.WorkMode,VIM_DISP_NODISP,
			(PVIM_MARB_Map)&gVc0528_Info.MarbStatus.MapList);
       }
       
       dwTwc=VIM_JPEG_GetTargetWordCount();
       // init parameter
	gVc0528_Info.CaptureStatus.SaveMode=SaveMode;
	if(SaveMode==VIM_HAPI_RAM_SAVE)
	{
		gVc0528_Info.CaptureStatus.BufPoint=StillBuf;
	}
	else
	{
		gVc0528_Info.CaptureStatus.MallocPr=gVc0528_Info.CaptureStatus.BufPoint=VIM_USER_MallocMemory(VIM_USER_BUFLENGTH);
		if(gVc0528_Info.CaptureStatus.MallocPr==NULL)
			return VIM_ERROR_BUFPOINT;
		gVc0528_Info.CaptureStatus.pFile_Nameptr=StillBuf;
		BUF_Length=VIM_USER_BUFLENGTH;	
		result = VIM_USER_SeekFile(gVc0528_Info.CaptureStatus.pFile_Nameptr,0);
		if(result)
			return result;
	}
	if(gVc0528_Info.CaptureStatus.BufPoint==NULL)
		return VIM_ERROR_BUFPOINT;

	gVc0528_Info.CaptureStatus.RecaptureTimes=0;
	gVc0528_Info.CaptureStatus.ThumbBuf=0;
	gVc0528_Info.CaptureStatus.CapFileLength=0;
	gVc0528_Info.CaptureStatus.CapCallback=pUCallBack;
	gVc0528_Info.CaptureStatus.BufLength=BUF_Length;	
	gVc0528_Info.CaptureStatus.CaptureError=VIM_SUCCEED;
	gVc0528_Info.MarbStatus.Jpgpoint=gVc0528_Info.MarbStatus.MapList.jbufstart;



	// can enable recapture function
	if((dwTwc+VIM_USER_RECAPTURE_OFFSET)<=gVc0528_Info.MarbStatus.MapList.jbufsize)
	{
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintDec("\n start restart capture ,dwTwc =",dwTwc);
		VIM_USER_PrintDec("\n jbuf size =",gVc0528_Info.MarbStatus.MapList.jbufsize);
#endif	
		gVc0528_Info.CaptureStatus.Mode=VIM_CAPTURECAPTURE;
		VIM_MARB_SetRecaptureEn(ENABLE);
		VIM_MARB_SetRecaptureInfo(VIM_USER_RECAPTUREMAXTIMES,(dwTwc+VIM_USER_RECAPTURE_OFFSET));
	}
	else
	{
		// set buffer gapratio
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintString("\n start one capture ");
		VIM_USER_PrintDec("\n jbuf size =",gVc0528_Info.MarbStatus.MapList.jbufsize);
#endif	
		VIM_MARB_SetJpegbufGapratio(1);
		VIM_MARB_SetRecaptureEn(DISABLE);
		gVc0528_Info.CaptureStatus.Mode=VIM_CAPTURESTILL;
	}

	// change timer if call back is not zero
	VIM_USER_StopTimer();
	if(pUCallBack)
	{
		result=VIM_USER_StartTimer(VIM_USER_CAPTURE_TIMER);
		if(result)
			return (UINT16)result;
	}
	
	// init intrrupt
	VIM_MAPI_InitCapInterrupt();
	VIM_MARB_ResetSubModuleAutoEn(ENABLE);

	//reset state
	VIM_JPEG_ResetState();
       VIM_MARB_StartCapture();

       
	//-----------------------------------------------------------
	if(gVc0528_Info.CaptureStatus.CapCallback==NULL)
	{
		while((gVc0528_Info.CaptureStatus.Mode!=VIM_CAPTUREDONE)&&(dwCount--))
		{
			VIM_USER_DelayMs(1); 
			_ISR_HIF_IntHandle();
			if(gVc0528_Info.CaptureStatus.CaptureError!=VIM_SUCCEED)
				return (UINT16)gVc0528_Info.CaptureStatus.CaptureError;
		}
		if(gVc0528_Info.CaptureStatus.Mode!=VIM_CAPTUREDONE)
			return VIM_ERROR_NOINITERRUPT;
	}

	
		
	return VIM_SUCCEED;
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
UINT16 VIM_HAPI_CaptureThumbStill(VIM_HAPI_SAVE_MODE SaveMode,HUGE void *StillBuf,HUGE void *ThumbBuf,UINT32 BUF_Length,VIM_HAPI_CallBack pUCallBack)
{

	UINT32 dwCount=5000,dwTwc;
      VIM_RESULT result;
	//check the chip working mode 
	if(gVc0528_Info.ChipWorkMode!=VIM_HAPI_MODE_CAMERAON) 
		return VIM_ERROR_WORKMODE;
	//check the preview  mode 
      	if(gVc0528_Info.PreviewStatus.Mode==VIM_HAPI_PREVIEW_OFF)
       		return VIM_ERROR_PREVIEWMODE;

	//check if capture still or frame
       if(gVc0528_Info.PreviewStatus.Mode!=VIM_HAPI_PREVIEW_FRAMEON)
       {
		VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER,DISABLE);
		gVc0528_Info.PreviewStatus.Mode=VIM_HAPI_PREVIEW_OFF;

		VIM_MAPI_SetChipMode(VIM_MARB_CAPTURESTILLTHUMB_MODE,VIM_IPP_HAVE_NOFRAME);
       }
       else //frame on
	      	return VIM_ERROR_CANNOTCAPT_FRAMTHUMB;

       //set memary
	VIM_MARB_SetMap(gVc0528_Info.MarbStatus.WorkMode,VIM_DISP_NODISP,
	(PVIM_MARB_Map)&gVc0528_Info.MarbStatus.MapList);
       dwTwc=VIM_JPEG_GetTargetWordCount();
       // init parameter

	gVc0528_Info.CaptureStatus.SaveMode=SaveMode;
	if(SaveMode==VIM_HAPI_RAM_SAVE)
	{
		gVc0528_Info.CaptureStatus.BufPoint=StillBuf;
	}
	else
	{
		gVc0528_Info.CaptureStatus.MallocPr=gVc0528_Info.CaptureStatus.BufPoint=VIM_USER_MallocMemory(VIM_USER_BUFLENGTH);
		if(gVc0528_Info.CaptureStatus.MallocPr==NULL)
			return VIM_ERROR_BUFPOINT;
		gVc0528_Info.CaptureStatus.pFile_Nameptr=StillBuf;
		BUF_Length=VIM_USER_BUFLENGTH;	
	}
	if(gVc0528_Info.CaptureStatus.BufPoint==NULL)
		return VIM_ERROR_BUFPOINT;
	gVc0528_Info.CaptureStatus.ThumbBuf=ThumbBuf;
	gVc0528_Info.CaptureStatus.CapFileLength=0;
	gVc0528_Info.CaptureStatus.CapCallback=pUCallBack;
	gVc0528_Info.CaptureStatus.BufLength=BUF_Length;	
	gVc0528_Info.CaptureStatus.CaptureError=VIM_SUCCEED;
	gVc0528_Info.MarbStatus.Jpgpoint=gVc0528_Info.MarbStatus.MapList.jbufstart;

	// can enable recapture function
	if((dwTwc+VIM_USER_RECAPTURE_OFFSET)<=gVc0528_Info.MarbStatus.MapList.jbufsize)
	{
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintDec("\n start restart capture ,dwTwc =",dwTwc);
		VIM_USER_PrintDec("\n jbuf size =",gVc0528_Info.MarbStatus.MapList.jbufsize);
#endif	
		gVc0528_Info.CaptureStatus.Mode=VIM_CAPTURECAPTURE;
		VIM_MARB_SetRecaptureEn(ENABLE);
		VIM_MARB_SetRecaptureInfo(VIM_USER_RECAPTUREMAXTIMES,BUF_Length);
	}
	else
	{
		// set buffer gapratio
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintString("\n start one capture ");
#endif	
		VIM_MARB_SetJpegbufGapratio(1);
		VIM_MARB_SetRecaptureEn(DISABLE);
		gVc0528_Info.CaptureStatus.Mode=VIM_CAPTURESTILL;
	}

	// change timer if call back is not zero
	VIM_USER_StopTimer();
	if(pUCallBack)
	{
		result=VIM_USER_StartTimer(VIM_USER_CAPTURE_TIMER);
		if(result)
			return (UINT16)result;
	}
	
	// init intrrupt
	VIM_MAPI_InitCapInterrupt();
	VIM_MARB_ResetSubModuleAutoEn(ENABLE);

	//reset state
	VIM_JPEG_ResetState();
       VIM_MARB_StartCapture();
	//-----------------------------------------------------------
	if(gVc0528_Info.CaptureStatus.CapCallback==NULL)
	{
		while((gVc0528_Info.CaptureStatus.Mode!=VIM_CAPTUREDONE)&&(dwCount--))
		{
			VIM_USER_DelayMs(1); 
			_ISR_HIF_IntHandle();
			if(gVc0528_Info.CaptureStatus.CaptureError!=VIM_SUCCEED)
				return (UINT16)gVc0528_Info.CaptureStatus.CaptureError;
		}
		if(gVc0528_Info.CaptureStatus.Mode!=VIM_CAPTUREDONE)
			return VIM_ERROR_NOINITERRUPT;
	}
	return VIM_SUCCEED;
}
/********************************************************************************
Description:
	set capture video information()
	UINT16 VIM_HAPI_StartCaptureVideo(void *StillBuf,UINT32 BUF_Length,VIM_HAPI_CallBack pUCallBack)

Parameters:
      SaveMode:
      				VIM_HAPI_RAM_SAVE,   save in the still buf (UINT8)
				VIM_HAPI_ROM_SAVE  stillbuf is the file id  of file 
      bFramRate: frame rate (per second) if 0.do not start timer to capture (customer will shot jpeg themsleves)
	dwMaxFrameCount: the max frame count which the user wanted
					if it is zero ,will start get quick capture mode
Return:
	VIM_SUCCEED:  ok
Note:
Remarks:

state: 
	valid
*********************************************************************************/

UINT16 VIM_HAPI_SetCaptureVideoInfo(VIM_HAPI_SAVE_MODE SaveMode,UINT8 bFramRate,UINT32 dwMaxFrameCount )
{
	gVc0528_Info.CaptureStatus.SaveMode=SaveMode;
	gVc0528_Info.VideoStatus.VideoFrameRate=bFramRate;
	gVc0528_Info.VideoStatus.MaxFrame=dwMaxFrameCount;
	return VIM_SUCCEED;
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
UINT16 VIM_HAPI_StartCaptureVideo(HUGE void *StillBuf,UINT32 BUF_Length,VIM_HAPI_CallBack pUCallBack)
{
	UINT32 dwTwc=VIM_JPEG_GetTargetWordCount();
      VIM_RESULT result;
      TSize InputSize;
	//check the chip working mode 
	if(gVc0528_Info.ChipWorkMode!=VIM_HAPI_MODE_CAMERAON) 
		return VIM_ERROR_WORKMODE;
	//check the preview  mode 
      	if(gVc0528_Info.PreviewStatus.Mode==VIM_HAPI_PREVIEW_OFF)
       		return VIM_ERROR_PREVIEWMODE;

	result = VIM_SIF_GetSensorResolution(gVc0528_Info.pSensorInfo, (VIM_SIF_RESOLUTION)gVc0528_Info.PreviewStatus.bOutputIndex, &InputSize);
	if((gVc0528_Info.CaptureStatus.Size.cx>InputSize.cx)||(gVc0528_Info.CaptureStatus.Size.cy>InputSize.cy))
		return VIM_ERROR_CANNOT_CAPVIDEO;

	//check if capture still or frame
       if(gVc0528_Info.PreviewStatus.Mode!=VIM_HAPI_PREVIEW_FRAMEON)
		VIM_MAPI_SetChipMode(VIM_MARB_CAPTURESTILL_MODE,VIM_IPP_HAVE_NOFRAME);
       else //frame on
       	VIM_MAPI_SetChipMode(VIM_MARB_CAPTURESTILL_MODE,VIM_IPP_HAVEFRAME);
       // init parameter
	if(gVc0528_Info.CaptureStatus.SaveMode==VIM_HAPI_RAM_SAVE)
		gVc0528_Info.CaptureStatus.BufPoint=StillBuf;
	else
	{
		gVc0528_Info.CaptureStatus.pFile_Nameptr=StillBuf;
		BUF_Length=VIM_USER_BUFLENGTH;	
	}
	gVc0528_Info.CaptureStatus.ThumbBuf=0;
	gVc0528_Info.CaptureStatus.CapFileLength=0;
	gVc0528_Info.CaptureStatus.CapCallback=0;
	gVc0528_Info.CaptureStatus.BufLength=BUF_Length;	
	gVc0528_Info.CaptureStatus.CaptureError=VIM_SUCCEED;
	gVc0528_Info.VideoStatus.Mode=VIM_VIDEO_STARTCAPTURE;
	gVc0528_Info.VideoStatus.CapCallback=pUCallBack;
	gVc0528_Info.VideoStatus.VideoLength=0;
	gVc0528_Info.VideoStatus.NowFrame=0;
#ifdef CONFIG_MACH_CANOPUS
	gVc0528_Info.VideoStatus.Abort = 0;
#endif
	// can enable recapture function
	if((dwTwc+VIM_USER_RECAPTURE_OFFSET)<=gVc0528_Info.MarbStatus.MapList.jbufsize)
	{
		#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintDec("\n start restart capture ,dwTwc =",dwTwc);
		VIM_USER_PrintDec("\n jbuf size =",gVc0528_Info.MarbStatus.MapList.jbufsize);
		#endif	
		gVc0528_Info.CaptureStatus.Mode=VIM_CAPTURECAPTURE;
		VIM_MARB_SetRecaptureEn(ENABLE);
		VIM_MARB_SetRecaptureInfo(VIM_USER_RECAPTUREMAXTIMES,(dwTwc+VIM_USER_RECAPTURE_OFFSET));
	}
	else
	{
		// set buffer gapratio
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintString("\n start one capture ");
		VIM_USER_PrintDec("\n jbuf size =",gVc0528_Info.MarbStatus.MapList.jbufsize);
#endif	
		VIM_MARB_SetJpegbufGapratio(1);
		VIM_MARB_SetRecaptureEn(DISABLE);
		gVc0528_Info.CaptureStatus.Mode=VIM_CAPTURESTILL;
	}
	
	// init intrrupt
	VIM_MAPI_InitCapInterrupt();
	VIM_MARB_ResetSubModuleAutoEn(ENABLE);
	// change timer if gVc0528_Info.VideoStatus.VideoFrameRate is not zero
	if(gVc0528_Info.VideoStatus.VideoFrameRate)
	{
		gVc0528_Info.VideoStatus.VideoFrameRate=1000/gVc0528_Info.VideoStatus.VideoFrameRate;
		VIM_USER_StopTimer();
		result=VIM_USER_StartTimer(gVc0528_Info.VideoStatus.VideoFrameRate);
		if(result)
			return (UINT16)result;
		#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintDec("\n start TIMER interval is ",gVc0528_Info.VideoStatus.VideoFrameRate);
		#endif	
	}
	if(!gVc0528_Info.VideoStatus.MaxFrame)	//angela add it for quick get one frame 2006-12-08
	{
		VIM_JPEG_ResetState();
     		VIM_MARB_StartCapture();
	}
	return VIM_SUCCEED;
}

/********************************************************************************
Description:
	Get one frame(jpeg) with preview on 
Parameters:
	*dwOneLen: the total length captured from VIM_HAPI_StartCaptureVideo()
	StillBuf:the head point of jpeg
	BUF_Length:the main buf len
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
UINT16 VIM_HAPI_GetOneJpeg(HUGE void *StillBuf,UINT32 BUF_Length,UINT32 *dwOneLen)
{
	UINT32 dwCount=5000, dwTwc=VIM_JPEG_GetTargetWordCount();
	//check the chip working mode 
	if(gVc0528_Info.ChipWorkMode!=VIM_HAPI_MODE_CAMERAON) 
		return VIM_ERROR_WORKMODE;
	//check the preview  mode 
      	if(gVc0528_Info.PreviewStatus.Mode==VIM_HAPI_PREVIEW_OFF)
       	return VIM_ERROR_PREVIEWMODE;
	if(gVc0528_Info.VideoStatus.Mode!=VIM_VIDEO_STARTCAPTURE)
       	return VIM_ERROR_VIDEO_MODE;		
	
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintDec("\n get one frame video length is ",gVc0528_Info.VideoStatus.VideoLength);
		VIM_USER_PrintHex(" JPUF size    ",gVc0528_Info.MarbStatus.MapList.jbufsize);
#endif	

      	if(gVc0528_Info.CaptureStatus.SaveMode==VIM_HAPI_RAM_SAVE)
	{
		gVc0528_Info.CaptureStatus.BufPoint=StillBuf;
	}
	else
	{
		gVc0528_Info.CaptureStatus.MallocPr=gVc0528_Info.CaptureStatus.BufPoint=VIM_USER_MallocMemory(VIM_USER_BUFLENGTH);
		if(gVc0528_Info.CaptureStatus.MallocPr==NULL)
			return VIM_ERROR_BUFPOINT;
		BUF_Length=VIM_USER_BUFLENGTH;	
	}
	if(gVc0528_Info.CaptureStatus.BufPoint==NULL)
		return VIM_ERROR_BUFPOINT;
	// if use the maxframe ,will be stop when more than maxframe
	if(gVc0528_Info.VideoStatus.MaxFrame)
	{
		if(gVc0528_Info.VideoStatus.NowFrame<gVc0528_Info.VideoStatus.MaxFrame)
			gVc0528_Info.VideoStatus.NowFrame++;
		else
			return VIM_ERROR_MAX_FRAMECOUNT;
	}
	gVc0528_Info.CaptureStatus.RecaptureTimes=0;
	gVc0528_Info.CaptureStatus.ThumbBuf=0;
	gVc0528_Info.CaptureStatus.CapFileLength=0;
	gVc0528_Info.CaptureStatus.Mode=VIM_CAPTURECAPTURE;
      	gVc0528_Info.CaptureStatus.BufLength=BUF_Length;
      	gVc0528_Info.MarbStatus.Jpgpoint=gVc0528_Info.MarbStatus.MapList.jbufstart;

      	if((dwTwc>BUF_Length)&&(gVc0528_Info.CaptureStatus.SaveMode==VIM_HAPI_RAM_SAVE))
		return VIM_ERROR_CAPTURE_BUFFULL;

	//reset state
	VIM_JPEG_ResetState();
       VIM_MARB_StartCapture();
	//-----------------------------------------------------------
	if(gVc0528_Info.CaptureStatus.CapCallback==NULL)
	{
#ifndef CONFIG_MACH_CANOPUS
		while((gVc0528_Info.CaptureStatus.Mode!=VIM_CAPTUREDONE)&&(dwCount--))
#else
		while((gVc0528_Info.CaptureStatus.Mode!=VIM_CAPTUREDONE)
				&& (dwCount--)
				&& !gVc0528_Info.VideoStatus.Abort)
#endif
		{
			VIM_USER_DelayMs(2); 
			_ISR_HIF_IntHandle();
			if(gVc0528_Info.CaptureStatus.CaptureError!=VIM_SUCCEED)
				return (UINT16)gVc0528_Info.CaptureStatus.CaptureError;
		}
		if(gVc0528_Info.CaptureStatus.Mode!=VIM_CAPTUREDONE)
		{
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
			VIM_USER_PrintString("\n ***********NoNo Int ********* ");
#endif	
			if(gVc0528_Info.CaptureStatus.SaveMode==VIM_HAPI_ROM_SAVE)
			{
					gVc0528_Info.CaptureStatus.CaptureError=VIM_USER_FreeMemory(gVc0528_Info.CaptureStatus.MallocPr);
					gVc0528_Info.CaptureStatus.MallocPr=0;
			}
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
			VIM_USER_PrintDec("no int length=",VIM_MARB_GetJbufRWSize(gVc0528_Info.MarbStatus.Jpgpoint));
#endif
			gVc0528_Info.CaptureStatus.CaptureError=VIM_ERROR_NOINITERRUPT;
			return VIM_ERROR_NOINITERRUPT;
		}
	}
	*dwOneLen=gVc0528_Info.CaptureStatus.CapFileLength;
	gVc0528_Info.VideoStatus.VideoLength+=gVc0528_Info.CaptureStatus.CapFileLength;
	return VIM_SUCCEED;
}
/********************************************************************************
Description:
	Get quick one frame(jpeg) with preview on 
Parameters:
	StillBuf:the head point of jpeg
	BUF_Length:the main buf len
	*dwOneLen: if get picture ok ,will return the picture length
Return:
	VIM_SUCCEED:  ok
	VIM_ERROR_WORKMODE(V5H_ERROR_PREVIEWMODE): error work mode
	VIM_ERROR_PREVIEWMODE: preview error
	VIM_ERROR_NOINITERRUPT :no pic this time ,please call this function next time
Note:
	this function should be used after calling VIM_HAPI_StartCaptureVideo () 
		bFramRate = 0
Remarks:
state: 
	valid
*********************************************************************************/
UINT16 VIM_HAPI_GetQuickOneJpeg(HUGE void *StillBuf,UINT32 BUF_Length,UINT32 *dwOneLen)
{
	UINT32 dwTwc=VIM_JPEG_GetTargetWordCount();
	//check the chip working mode 
	if(gVc0528_Info.ChipWorkMode!=VIM_HAPI_MODE_CAMERAON) 
		return VIM_ERROR_WORKMODE;
	//check the preview  mode 
      	if(gVc0528_Info.PreviewStatus.Mode==VIM_HAPI_PREVIEW_OFF)
       	return VIM_ERROR_PREVIEWMODE;
	if(gVc0528_Info.VideoStatus.Mode!=VIM_VIDEO_STARTCAPTURE)
       	return VIM_ERROR_VIDEO_MODE;		
	
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
	//VIM_USER_PrintDec("\n NowFrame is ",gVc0528_Info.VideoStatus.NowFrame);
#endif


	gVc0528_Info.CaptureStatus.BufPoint=StillBuf;
	if(gVc0528_Info.CaptureStatus.BufPoint==NULL)
		return VIM_ERROR_BUFPOINT;
	gVc0528_Info.CaptureStatus.Mode=VIM_CAPTURECAPTURE;
	gVc0528_Info.CaptureStatus.RecaptureTimes=0;
	gVc0528_Info.CaptureStatus.ThumbBuf=0;
	gVc0528_Info.CaptureStatus.CapFileLength=0;
      	gVc0528_Info.CaptureStatus.BufLength=BUF_Length;
      	gVc0528_Info.MarbStatus.Jpgpoint=gVc0528_Info.MarbStatus.MapList.jbufstart;

      	if((dwTwc>BUF_Length)&&(gVc0528_Info.CaptureStatus.SaveMode==VIM_HAPI_RAM_SAVE))
		return VIM_ERROR_CAPTURE_BUFFULL;


	_ISR_HIF_IntHandle();
	if(gVc0528_Info.CaptureStatus.CaptureError!=VIM_SUCCEED)
		return (UINT16)gVc0528_Info.CaptureStatus.CaptureError;
	switch(gVc0528_Info.CaptureStatus.Mode)
	{
	case VIM_CAPTUREDONE:
		//reset state
		gVc0528_Info.VideoStatus.NowFrame=0;
		VIM_JPEG_ResetState();
     		VIM_MARB_StartCapture();
		*dwOneLen=gVc0528_Info.CaptureStatus.CapFileLength;

		return VIM_SUCCEED;
	default:
		if(VIM_USER_GETQUICKPIC_TIMES)
		{
			gVc0528_Info.VideoStatus.NowFrame++;
			if(gVc0528_Info.VideoStatus.NowFrame>=VIM_USER_GETQUICKPIC_TIMES)
			{
				gVc0528_Info.VideoStatus.NowFrame=0;
				VIM_JPEG_ResetState();
     				VIM_MARB_StartCapture();
			}
		}
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintDec("no int length=",VIM_MARB_GetJbufRWSize(gVc0528_Info.MarbStatus.Jpgpoint));
#endif
		return VIM_ERROR_NOINITERRUPT;

	}
	return VIM_SUCCEED;
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
UINT16 VIM_HAPI_StopCapture(void)
{
	//check the chip working mode 
	if(gVc0528_Info.ChipWorkMode!=VIM_HAPI_MODE_CAMERAON) 
		return VIM_ERROR_WORKMODE;
	//check the preview  mode 
      	if(gVc0528_Info.PreviewStatus.Mode==VIM_HAPI_PREVIEW_OFF)
       	return VIM_ERROR_PREVIEWMODE;
	if(gVc0528_Info.VideoStatus.Mode!=VIM_VIDEO_STARTCAPTURE)
       	return VIM_ERROR_VIDEO_MODE;
	VIM_USER_StopTimer();
	VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER,DISABLE);
    VIM_MAPI_Delay_Frame(1);
    VIM_SIF_EnableSyncGen(DISABLE);	//angela 2006-9-4
	gVc0528_Info.VideoStatus.Mode=VIM_VIDEO_STOP;
#ifdef CONFIG_MACH_CANOPUS
	gVc0528_Info.VideoStatus.Abort=0;
#endif
	gVc0528_Info.CaptureStatus.CapFileLength=gVc0528_Info.VideoStatus.VideoLength;
	if((gVc0528_Info.VideoStatus.CapCallback)&&(gVc0528_Info.CaptureStatus.CaptureError==0))
	 	gVc0528_Info.VideoStatus.CapCallback(VIM_HAPI_CAPTURE_END,gVc0528_Info.VideoStatus.VideoLength);
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintString("**************VIM_HAPI_StopCapture");
#endif
	return VIM_SUCCEED;
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
UINT32 VIM_HAPI_GetCaptureLength(void)
{
	//check the chip working mode 
	if(gVc0528_Info.ChipWorkMode!=VIM_HAPI_MODE_CAMERAON) 
		return 0;
	return gVc0528_Info.CaptureStatus.CapFileLength;
}

/********************************************************************************
Description:
	Get one frame from Multishot buffer
Parameters:
      framebuf: the buffer to store the muti frame.
      buflength: the total length of multishot buffer
      framenumber: the frame number to get( first is 1)      
     *pframelength: the frame(jpeg) length. if 0, no this frame in the buffer 
Return:
	the buffer point of this frame
Note:

Remarks:
state: 
	valid
*********************************************************************************/
HUGE UINT8* VIM_HAPI_GetOneFrame(HUGE UINT8 *framebuf, UINT32 buflength, UINT8 framenumber, UINT32 *pframelength)
{
	UINT8 i=0, *ptmp;
	UINT32 length=0;
	buflength/=2;
	while(buflength--)
	{
		if( (*framebuf)==0xff&&(*(framebuf+1))==0xd8)
			ptmp=framebuf;
		else if( (*framebuf)==0xff&&(*(framebuf+1))==0xd9)
		{
			i++;
			if(i==framenumber)
			{
				if(framebuf>ptmp)
					length=framebuf+2-ptmp;
				break;
			}
		}
		framebuf+=2;
	}
	*pframelength=length;
	return ptmp;
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
UINT16 VIM_HAPI_SetLCDWorkMode(VIM_HAPI_LCDWORKMODE byABLayerMode, UINT16 wValue)
{
	UINT32 dwKeyColor = 0;
	if ((VIM_HAPI_MODE_BYPASS == gVc0528_Info.ChipWorkMode) || (VIM_HAPI_MODE_DIRECTDISPLAY ==gVc0528_Info.ChipWorkMode))
	{
		return VIM_ERROR_WORKMODE;
	}
	switch(byABLayerMode)
	{
		case VIM_HAPI_LCDMODE_AFIRST:
			// Enable A Layer, diable overlay and blending
			VIM_DISP_SetWorkMode(VIM_DISP_AFIRST);
			break;
		case VIM_HAPI_LCDMODE_BLONLY:
			// Disable A Layer, diable overlay and blending
			VIM_SIF_EnableSyncGen(DISABLE);
 			VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER,DISABLE);
			VIM_DISP_SetWorkMode(VIM_DISP_BFIRST);
			break;
		case VIM_HAPI_LCDMODE_OVERLAY:
			// Enable A Layer, enable overlay, disable blending
			dwKeyColor = (((UINT32)wValue) & 0x1f) << 3; // B 
			dwKeyColor |= (((UINT32)wValue) & 0x7e0) << 5; // G
			dwKeyColor |= (((UINT32)wValue) & 0xf800) << 8; // R 
			VIM_DISP_SetOverlayKeyColor(VIM_DISP_ALLLAYER,dwKeyColor);
			VIM_DISP_SetWorkMode(VIM_DISP_ALL_OVERLAY);
			VIM_DISP_SetLayerEnable(VIM_DISP_ALLLAYER,ENABLE);
			break;
		case VIM_HAPI_LCDMODE_BLEND:
			// Enable A Layer, disable overlay, enable blending
			VIM_DISP_SetBlendRatio(VIM_DISP_ALLBLAYER, (UINT8)wValue);
			VIM_DISP_SetWorkMode(VIM_DISP_ALL_BLEND);
			VIM_DISP_SetLayerEnable(VIM_DISP_ALLLAYER,ENABLE);
			break;
		default:
			return VIM_ERROR_PARAMETER;
	}
	gVc0528_Info.LcdStatus.WorkMode=byABLayerMode;
	return VIM_SUCCEED;
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
	Note�����
		Only write to the 568 internal ram, not update to LCD.
	This function can be used in Camera open mode or Photo process mode 
Remarks:
	valid
*********************************************************************************/

UINT16 VIM_HAPI_DrawLCDRctngl(VIM_HAPI_LAYER BLayer,UINT16 startx, UINT16 starty,UINT16 Width,UINT16 Height, HUGE  UINT8 *pwImage)
{
	TSize BSize;
	UINT32 offset,i;
	TPoint startPoint,point;

	if ((VIM_HAPI_MODE_BYPASS == gVc0528_Info.ChipWorkMode) || (VIM_HAPI_MODE_DIRECTDISPLAY ==gVc0528_Info.ChipWorkMode))
	{
		return VIM_ERROR_WORKMODE;
	}

	if(BLayer > VIM_DISP_ALLLAYER || BLayer < VIM_DISP_B0LAYER)
		return VIM_ERROR_PARAMETER;

	if ((NULL == pwImage) || (0 ==Width) || (0 == Height))
	{
		return VIM_ERROR_PARAMETER;
	}

	point.x=startx;
	point.y=starty;
	// Get B Layer star point and size
	////b start point  is always 0,0 VIM_DISP_GetBStartPoint
	if(BLayer & VIM_DISP_B0LAYER)
	{
		VIM_DISP_GetBSize(VIM_DISP_B0LAYER,&BSize);
		// When LCD rotate,change BSize width and height
		startPoint=gVc0528_Info.LcdStatus.B0Startpoint;

		if((point.x<startPoint.x)||(point.y<startPoint.y))
			return VIM_ERROR_LCD_DRAWOVERRANGE;

		if(gVc0528_Info.LcdStatus.BRotationMode%2)
			BSize.cx=BSize.cy;

		offset = (point.y - startPoint.y) * BSize.cx * 2 + (point.x - startPoint.x) * 2;
		if(BSize.cx==Width)
		{
			if (VIM_USER_LCD_DRAWDATA_LITTLEENDIAN)
				VIM_HIF_WriteSramReverse(gVc0528_Info.MarbStatus.MapList.layerB0start + offset,
					(UINT8 *)(pwImage),Width * 2 *Height);
			else
				VIM_HIF_WriteSram(gVc0528_Info.MarbStatus.MapList.layerB0start + offset,
					(UINT8 *)(pwImage),Width * 2 *Height);
		}
		else
		{
			for(i = 0; i < Height; i++)
			{
				if (VIM_USER_LCD_DRAWDATA_LITTLEENDIAN)
					VIM_HIF_WriteSramReverse(gVc0528_Info.MarbStatus.MapList.layerB0start + offset,
						(UINT8 *)(pwImage + (i * Width * 2)),Width * 2);
				else
					VIM_HIF_WriteSram(gVc0528_Info.MarbStatus.MapList.layerB0start + offset,
						(UINT8 *)(pwImage + (i * Width * 2)),Width * 2);

				offset+=BSize.cx*2;//angela changed it
			}
		}
	}
	if(BLayer & VIM_DISP_B1LAYER)
	{
		VIM_DISP_GetBSize(VIM_DISP_B1LAYER,&BSize);
		// When LCD rotate,change BSize width and height

		startPoint=gVc0528_Info.LcdStatus.B1Startpoint;

		if((point.x<startPoint.x)||(point.y<startPoint.y))
			return VIM_ERROR_LCD_DRAWOVERRANGE;

		if(gVc0528_Info.LcdStatus.BRotationMode%2)
			BSize.cx=BSize.cy;

		offset = (point.y - startPoint.y) * BSize.cx * 2 + (point.x- startPoint.x) * 2;
		if(BSize.cx==Width)
		{
			if (VIM_USER_LCD_DRAWDATA_LITTLEENDIAN)
				VIM_HIF_WriteSramReverse(gVc0528_Info.MarbStatus.MapList.layerB1start + offset,
					(UINT8 *)(pwImage),Width * 2 *Height);
			else
				VIM_HIF_WriteSram(gVc0528_Info.MarbStatus.MapList.layerB1start + offset,
					(UINT8 *)(pwImage),Width * 2 *Height);
		}
		else
		{
			for(i = 0; i < Height; i++)
			{
				if (VIM_USER_LCD_DRAWDATA_LITTLEENDIAN)
					VIM_HIF_WriteSramReverse(gVc0528_Info.MarbStatus.MapList.layerB1start + offset,
						(UINT8 *)(pwImage + (i * Width * 2)),Width * 2);
				else
					VIM_HIF_WriteSram(gVc0528_Info.MarbStatus.MapList.layerB1start + offset ,
						(UINT8 *)(pwImage + (i * Width * 2)),(Width * 2));

				offset+=BSize.cx*2;//angela changed it
			}
		}
	}

	return VIM_SUCCEED;
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

UINT16 VIM_HAPI_UpdateLCD(VIM_HAPI_LAYER BLayer,UINT16 startx, UINT16 starty,UINT16 Width,UINT16 Height)
{

	TPoint refreshPoint;
	TSize size = {0,0};
	TPoint startPoint,point,Newpoint;


	if ((VIM_HAPI_MODE_BYPASS == gVc0528_Info.ChipWorkMode) || (VIM_HAPI_MODE_DIRECTDISPLAY ==gVc0528_Info.ChipWorkMode))
	{
		return VIM_ERROR_WORKMODE;
	}
	if(BLayer > VIM_DISP_B1LAYER || BLayer < VIM_DISP_B0LAYER)
		return VIM_ERROR_PARAMETER;
	
	if ((0 == Width) || (0 == Height))
	{
		return VIM_ERROR_PARAMETER;
	}

	//get B layer display position on the LCD screen
	////b start point  is always 0,0 VIM_DISP_GetBStartPoint
	VIM_DISP_GetBStartPoint((VIM_DISP_LAYER)BLayer,&startPoint);

	point.x=startx;
	point.y=starty;
	size.cx=Width;
	size.cy=Height;

	if((VIM_MAPI_FindUpdateRange(size,point)==4)&&(gVc0528_Info.PreviewStatus.Mode!=VIM_HAPI_PREVIEW_OFF)
		&&(gVc0528_Info.LcdStatus.WorkMode>=VIM_HAPI_LCDMODE_OVERLAY))
		return VIM_SUCCEED;
	
	VIM_MAPI_GetNewPoint((VIM_DISP_ROTATEMODE)gVc0528_Info.LcdStatus.BRotationMode,&point,
		size,&Newpoint);

	if((Newpoint.x<startPoint.x)||(Newpoint.y<startPoint.y))
		return VIM_ERROR_LCD_UPDATERANGE;
	
	refreshPoint.x =Newpoint.x-startPoint.x;
	refreshPoint.y =Newpoint.y-startPoint.y;	

	if(gVc0528_Info.LcdStatus.BRotationMode % 2)
	{
		Width = size.cx;
		size.cx = size.cy;
		size.cy = Width;
	}
	
	//set B refreshed position and size
	if(BLayer & VIM_DISP_B0LAYER)
		VIM_DISP_SetB0_RefreshBlock(refreshPoint,size);
	if(BLayer & VIM_DISP_B1LAYER)
		VIM_DISP_SetB1_RefreshBlock(refreshPoint,size);

	//set update control register
	VIM_DISP_Update((VIM_DISP_LAYER)BLayer);
	
	return VIM_SUCCEED;

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
UINT16 VIM_HAPI_SelectLCDPanel(VIM_HAPI_LCDPANEL byPanel)
{
	if(gVc0528_Info.LcdStatus.NowPanel == byPanel)//
		return VIM_SUCCEED;

	if(byPanel > VIM_HAPI_LCDPANEL_SUB)
		return VIM_ERROR_PARAMETER; 
	if ((VIM_HAPI_MODE_BYPASS == gVc0528_Info.ChipWorkMode) || (VIM_HAPI_MODE_DIRECTDISPLAY ==gVc0528_Info.ChipWorkMode))
	{
		if(VIM_BYPASS_SUB_CS1 == gVc0528_Info.pUserInfo->BypassDef.BypassSubCtl)
		{	
			VIM_HIF_SetBypassOrNormal((PVIM_BYPASS_DEF)&gVc0528_Info.pUserInfo->BypassDef,
				VIM_HIF_NORMALTYPE);
			if(byPanel == VIM_HAPI_LCDPANEL_MAIN)
			{
				VIM_HIF_SwitchPanelCs(VIM_BYPASS_SUB_CS1);
			}
			else if (gDriverIcInfo.slave->config == SUBPANEL)
			{
				VIM_HIF_SwitchPanelCs(VIM_BYPASS_SUB_CS2);			
			}
			VIM_HIF_SetBypassOrNormal((PVIM_BYPASS_DEF)&gVc0528_Info.pUserInfo->BypassDef,
				VIM_HIF_BYPASSTYE);		
		}
	}
	else
	{
		// Set LCDIF
		VIM_MAPI_SwitchPanel(byPanel);
	}
	gVc0528_Info.LcdStatus.NowPanel = (VIM_HAPI_LCDPANEL)byPanel;


	return VIM_SUCCEED;
}

/********************************************************************************
* Description:
*	select a window in the lcd panel ,can draw rgb data in b layer 
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
*	3 if lcd mirror have been changed ,please recall this function to effect the change.
State: valid
*********************************************************************************/

UINT16 VIM_HAPI_SetLCDSize(VIM_HAPI_LAYER BLayer,UINT16 wStartx,UINT16 wStarty,UINT16 Width,UINT16 Height)
{
	TPoint StartPoint,Newpoint;
	TSize size,resize;
	VIM_RESULT result;
	StartPoint.x = wStartx;
	StartPoint.y = wStarty;
	size.cx = Width;
	size.cy = Height;
	if ((VIM_HAPI_MODE_BYPASS == gVc0528_Info.ChipWorkMode) || (VIM_HAPI_MODE_DIRECTDISPLAY ==gVc0528_Info.ChipWorkMode))
	{
		return VIM_ERROR_WORKMODE;
	}			

	if(gVc0528_Info.PreviewStatus.Mode!=VIM_HAPI_PREVIEW_OFF)
	{
		VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER,DISABLE);	//angela 2006-12-12
		VIM_MAPI_Delay_Frame(1);
	}
	
	switch(BLayer)
	{
		case VIM_HAPI_B0_LAYER:
		{
			VIM_DISP_GetBSize(VIM_DISP_B0LAYER,&resize);
			result=VIM_DISP_SetB0_DisplayPanel(StartPoint,size);
			gVc0528_Info.LcdStatus.B0Startpoint=StartPoint;
			VIM_MAPI_GetNewPoint((VIM_DISP_ROTATEMODE)gVc0528_Info.LcdStatus.BRotationMode,&StartPoint,size,&Newpoint);
			VIM_HIF_SetReg16(V5_REG_LCDC_BX0_L,Newpoint.x);
			VIM_HIF_SetReg16(V5_REG_LCDC_BY0_L,Newpoint.y);
			VIM_DISP_SetRotateMode(VIM_DISP_B0LAYER,(VIM_DISP_ROTATEMODE)gVc0528_Info.LcdStatus.BRotationMode);

		}
		break;
		case VIM_HAPI_B1_LAYER:
		{
			VIM_DISP_GetBSize(VIM_DISP_B1LAYER,&resize);
			result=VIM_DISP_SetB1_DisplayPanel(StartPoint,size);
			gVc0528_Info.LcdStatus.B1Startpoint=StartPoint;
			VIM_MAPI_GetNewPoint((VIM_DISP_ROTATEMODE)gVc0528_Info.LcdStatus.BRotationMode,&StartPoint,size,&Newpoint);
			VIM_HIF_SetReg16(V5_REG_LCDC_BX1_L,Newpoint.x);
			VIM_HIF_SetReg16(V5_REG_LCDC_BY1_L,Newpoint.y);
			VIM_DISP_SetRotateMode(VIM_DISP_B1LAYER,(VIM_DISP_ROTATEMODE)gVc0528_Info.LcdStatus.BRotationMode);
		}
		break;
		default:
			return VIM_ERROR_PARAMETER;
	}
	if(result)
		return result;
	if(((resize.cx*resize.cy)!=(size.cx*size.cy)))	
	result=VIM_MARB_SetMap(gVc0528_Info.MarbStatus.WorkMode,gVc0528_Info.MarbStatus.ALayerMode,
		(PVIM_MARB_Map)&gVc0528_Info.MarbStatus.MapList);

	
	if(gVc0528_Info.PreviewStatus.Mode!=VIM_HAPI_PREVIEW_OFF)
	{
		VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER,ENABLE);	//angela 2006-12-12
	}
	return result;
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
UINT16 VIM_HAPI_DrawLCDPureColor(VIM_HAPI_LAYER BLayer,UINT16 startx, UINT16 starty,UINT16 Width,UINT16 Height,UINT16 wRGBColor)
{
	TSize BSize;
	UINT32 offset,i;
	TPoint startPoint;
	UINT32 dwColor = 0;
	if ((VIM_HAPI_MODE_BYPASS == gVc0528_Info.ChipWorkMode) || (VIM_HAPI_MODE_DIRECTDISPLAY ==gVc0528_Info.ChipWorkMode))
	{
		return VIM_ERROR_WORKMODE;
	}

	if(BLayer > VIM_DISP_ALLLAYER || BLayer < VIM_DISP_B0LAYER)
		return VIM_ERROR_PARAMETER;

	if ( (0 ==Width) || (0 == Height))
	{
		return VIM_ERROR_PARAMETER;
	}
	dwColor = (((UINT32)wRGBColor) << 16) | wRGBColor;
	// Get B Layer star point and size
	////b start point  is always 0,0 VIM_DISP_GetBStartPoint
	if(BLayer & VIM_DISP_B0LAYER)
	{
		VIM_DISP_GetBSize(VIM_DISP_B0LAYER,&BSize);
		// When LCD rotate,change BSize width and height
		startPoint=gVc0528_Info.LcdStatus.B0Startpoint;

		if((startx<startPoint.x)||(starty<startPoint.y))
			return VIM_ERROR_LCD_DRAWOVERRANGE;

		if(gVc0528_Info.LcdStatus.BRotationMode%2)
			BSize.cx=BSize.cy;

			
		offset = (starty - startPoint.y) * BSize.cx * 2 + (startx- startPoint.x) * 2;
		if(BSize.cx==Width)
		{
			VIM_HIF_WriteSramOnWord(gVc0528_Info.MarbStatus.MapList.layerB0start + offset,
				dwColor,Width * 2 *Height);
		}
		for(i = 0; i < Height; i++)
		{
			VIM_HIF_WriteSramOnWord(gVc0528_Info.MarbStatus.MapList.layerB0start + offset,
				dwColor,Width * 2);
			offset+=BSize.cx*2;//angela changed it
		}
	}
	if(BLayer & VIM_DISP_B1LAYER)
	{
		VIM_DISP_GetBSize(VIM_DISP_B1LAYER,&BSize);
		// When LCD rotate,change BSize width and height
		startPoint=gVc0528_Info.LcdStatus.B1Startpoint;

		if((startx<startPoint.x)||(starty<startPoint.y))
			return VIM_ERROR_LCD_DRAWOVERRANGE;

		if(gVc0528_Info.LcdStatus.BRotationMode%2)
			BSize.cx=BSize.cy;

			
		offset = (starty - startPoint.y) * BSize.cx * 2 + (startx- startPoint.x) * 2;
		if(BSize.cx==Width)
		{
			VIM_HIF_WriteSramOnWord(gVc0528_Info.MarbStatus.MapList.layerB1start + offset,
				dwColor,Width * 2 *Height);
		}
		for(i = 0; i < Height; i++)
		{
			VIM_HIF_WriteSramOnWord(gVc0528_Info.MarbStatus.MapList.layerB1start + offset ,
				dwColor,(Width * 2));
			offset+=BSize.cx*2;//angela changed it
		}
	}
	return VIM_SUCCEED;
}

/********************************************************************************
* Description:
*	Set B Layer color depth
* Parameters:
*	byColorDepth:	
		VIM_HAPI_COLORDEP_16BIT=6,
		VIM_HAPI_COLORDEP_15BIT=5
		VIM_HAPI_COLORDEP_12BIT
 * Return:
*	VIM_SUCCEED:				Set color depth success.
*	VIM_ERROR_PARAMETER:		byColorDepth is unknown.
*	VIM_ERROR_WORKMODE:		Current mode is V5H_MODE_BYPASS or V5H_MODE_DIRECTDISPLAY
* Note: 
*	 
State: valid
*********************************************************************************/
UINT16 VIM_HAPI_SetLCDColordep(VIM_HAPI_COLORDEP_MODE byColorDepth)
{
	if ((VIM_HAPI_MODE_BYPASS == gVc0528_Info.ChipWorkMode) || (VIM_HAPI_MODE_DIRECTDISPLAY ==gVc0528_Info.ChipWorkMode))
	{
		return VIM_ERROR_WORKMODE;
	}
	if(byColorDepth>VIM_HAPI_COLORDEP_16BIT||byColorDepth<VIM_HAPI_COLORDEP_12BIT)
		return VIM_ERROR_PARAMETER;
	
	gVc0528_Info.LcdStatus.ColorDep=byColorDepth;
	VIM_DISP_SetGbufFormat((VIM_DISP_BFORMAT)byColorDepth);
	return VIM_SUCCEED;
}

/********************************************************************************
* Description:
*	Mirror or flip the display image on LCD, includes A layer and B layer.
* Parameters:
*	byMirrorFlipMode:
	Rotate_Degree_0,
	Rotate_Degree_90,
	Rotate_Degree_180,
	Rotate_Degree_270,
	Rotate_Mirror_0,
	Rotate_Mirror_90,
	Rotate_Mirror_180,
	Rotate_Mirror_270,

*	byABLayer:
	VIM_HAPI_B0_Layer = 1,
	VIM_HAPI_B1_Layer,
	VIM_HAPI_ALL_BLayer,
	VIM_HAPI_A_Layer,
	VIM_HAPI_All_Layer
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
UINT16 VIM_HAPI_SetLCDMirror(VIM_HAPI_ROTATEMODE MirrorFlipMode, VIM_HAPI_LAYER Layer)
{
	if ((VIM_HAPI_MODE_BYPASS == gVc0528_Info.ChipWorkMode) || (VIM_HAPI_MODE_DIRECTDISPLAY ==gVc0528_Info.ChipWorkMode))
	{
		return VIM_ERROR_WORKMODE;
	}

	if(MirrorFlipMode > VIM_DISP_MIRROR_270)
		return VIM_ERROR_PARAMETER;		

	switch(Layer)
	{
	case VIM_HAPI_ALLB_LAYER:	
		gVc0528_Info.LcdStatus.BRotationMode=(VIM_DISP_ROTATEMODE)MirrorFlipMode;
		break;
	case VIM_HAPI_A_LAYER:
		gVc0528_Info.LcdStatus.ARotationMode =(VIM_DISP_ROTATEMODE)MirrorFlipMode;
		break;
	case VIM_HAPI_ALL_LAYER:
		gVc0528_Info.LcdStatus.BRotationMode=(VIM_DISP_ROTATEMODE)MirrorFlipMode;
		gVc0528_Info.LcdStatus.ARotationMode =(VIM_DISP_ROTATEMODE)MirrorFlipMode;
		break;
	default:
		return VIM_ERROR_PARAMETER;
	}

	return VIM_SUCCEED;
}




/********************************************************************************
Description:
	Set thumb  parameter(still, multishot, video)
Parameters:
	ThumWidth:  the width of Thumbnail image.
	ThumHeight: the height of Thumbnail image.
	Return:
		V5H_SUCCEED: set ok
		V5H_ERROR_WORKMODE: error work mode
	Note:
		This function must be used before first preview or change the resolution of display.
		This function must be used in Camera open mode
	Remarks:
	State:Valid.
*********************************************************************************/
UINT16 VIM_HAPI_SetThumbParameter(UINT16 ThumWidth,UINT16 ThumHeight)
{
	if(gVc0528_Info.ChipWorkMode!=VIM_HAPI_MODE_CAMERAON)
		return VIM_ERROR_WORKMODE;
	gVc0528_Info.CaptureStatus.ThumbSize.cx=(UINT16)ThumWidth;
	gVc0528_Info.CaptureStatus.ThumbSize.cy=(UINT16)ThumHeight;
	return VIM_SUCCEED;
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
UINT16 VIM_HAPI_GetPictureSize(UINT16 *picWidth, UINT16 *picHeight)
{
	*picWidth =gVc0528_JpegInfo.ImageSize.cx; 
	*picHeight =	gVc0528_JpegInfo.ImageSize.cy;
	return VIM_SUCCEED;
}

/********************************************************************************
Description:
	get JPG Decod size 
Parameters:
	picWidth:  the width of captured image.
	picHeight: the height of captured image.
	Return:
		V5H_SUCCEED: set ok
	Note:
		This function must be used after decode 
	Remarks:
	State:Valid.
*********************************************************************************/
UINT16 VIM_HAPI_GetDecodeSize(UINT16 *picWidth, UINT16 *picHeight)
{
TSize size;
	VIM_DISP_GetAWinsize(&size);
	*picWidth=size.cx;
	*picHeight=size.cy;
	return VIM_SUCCEED;
}
/***************************************************************************************************************************************
Description:
	set display mode and parameter, display one frame or more frame jpeg picture on LCD
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
Modify History:
	Version			Modifyby				DataTime					modified
	   0.1			Amanda Deng			2005.12.20				create first version
	   0.2			Amanda Deng			2005.01.06				only set display work mode and parameter
	   0.3			Amanda Deng			2005.01.18				add check if have pic in jbuf  function
	   0.4			Amanda Deng			2005.02.08				add file save mode (in RAM/ROM)
****************************************************************************************************************************************/
UINT16 VIM_HAPI_ReadyToDisplay(VIM_HAPI_SAVE_MODE SaveMode,HUGE void *jpegBuf, UINT32 length,UINT16 LcdOffsetX, UINT16 LcdOffsetY, UINT16 DisplayWidth, UINT16 DisplayHeight)
{
	VIM_RESULT result;
       TSize DisplaySize,DestDisplaySize;
	TPoint pt = {0, 0};
	UINT32 readCnt=0;
	
	//check the chip working mode 
	if(gVc0528_Info.ChipWorkMode!=VIM_HAPI_MODE_CAMERAON&&gVc0528_Info.ChipWorkMode!=VIM_HAPI_MODE_PHOTOPROCESS) 
		return VIM_ERROR_WORKMODE;
	
	gVc0528_Info.DisplayStatus.SaveMode=SaveMode;
	gVc0528_Info.DisplayStatus.FileLength=0;
	if(SaveMode==VIM_HAPI_RAM_SAVE)
	{
		gVc0528_Info.DisplayStatus.BufPoint=jpegBuf;
		gVc0528_Info.DisplayStatus.BufLength=length;
		readCnt=length;
	}
	else
	{
		gVc0528_Info.DisplayStatus.pFileHandle=jpegBuf;
		gVc0528_Info.DisplayStatus.BufOffset=0;
		gVc0528_Info.DisplayStatus.BufLength=VIM_USER_BUFLENGTH;
		length=VIM_USER_BUFLENGTH;
		gVc0528_Info.DisplayStatus.MallocPr=gVc0528_Info.DisplayStatus.BufPoint=VIM_USER_MallocMemory(VIM_USER_BUFLENGTH);
		if(gVc0528_Info.DisplayStatus.BufPoint==NULL)
			return VIM_ERROR_BUFPOINT;
		//read file from ROM to user define RAM
		result = VIM_USER_SeekFile(gVc0528_Info.DisplayStatus.pFileHandle,0);
		if(result)
			goto DISPLAYERROR;
		readCnt = VIM_USER_ReadFile(gVc0528_Info.DisplayStatus.pFileHandle,  gVc0528_Info.DisplayStatus.BufPoint, VIM_USER_BUFLENGTH);
	}

	//disable syncgen
	VIM_SIF_EnableSyncGen(DISABLE);		//angela 2006-8-15
	
		
	//get jpeg information( YVU mode, image size and other table parameters
	result = VIM_MAPI_GetJpegInformation(gVc0528_Info.DisplayStatus.BufPoint,readCnt);
	if(result)
		goto DISPLAYERROR;

	// set all module to display jpeg mode
	VIM_MAPI_SetDisplayJpegMode();  
	gVc0528_Info.PreviewStatus.Mode=VIM_HAPI_PREVIEW_OFF;
	//check display size>{4,4},must less than (LCD size),get display size
	result = VIM_MAPI_CheckDisplaySize(LcdOffsetX,LcdOffsetY,DisplayWidth,DisplayHeight,&DisplaySize);
	if(result)
		goto DISPLAYERROR;
	//from version 0.2 need display whole pic in LCD, so source size =jpeg image size
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintDec("\n jpeg Image width:  ",gVc0528_JpegInfo.ImageSize.cx);
		VIM_USER_PrintDec("\n jpeg Image height:  ",gVc0528_JpegInfo.ImageSize.cy);
		VIM_USER_PrintDec("\n yuv is:  ",gVc0528_JpegInfo.YUVType);
#endif	
	//get real display size(display image in user define display size, maybe not full display in LCD, only part of user define display size)
	if(VIM_USER_DISPLAY_FULLSCREEN)
		result = VIM_IPP_ToolCaculateBigDstWindow(gVc0528_JpegInfo.ImageSize,DisplaySize,&DestDisplaySize);
	else
		result = VIM_IPP_ToolCaculateLessDisplayWindow(gVc0528_JpegInfo.ImageSize,DisplaySize,&DestDisplaySize);
	if(result)
		goto DISPLAYERROR;

	//set ipp module image size
	VIM_IPP_SetImageSize(gVc0528_JpegInfo.JpgSize.cx, gVc0528_JpegInfo.JpgSize.cy);
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintDec("\n Dest DisplaySize width:  ",DestDisplaySize.cx);
		VIM_USER_PrintDec("\n Dest DisplaySize height:  ",DestDisplaySize.cy);
#endif	
	//set ipp module source window size, display size
	result = VIM_IPP_SetDispalySize(pt, gVc0528_JpegInfo.ImageSize, DestDisplaySize);//angela 2007-3-5
	if(result)
		goto DISPLAYERROR;
		

	pt.x=pt.y=0;
	if(DestDisplaySize.cx>DisplaySize.cx)	
		pt.x=(((DestDisplaySize.cx-DisplaySize.cx)>>1))&0xfffc;
	if(DestDisplaySize.cy>DisplaySize.cy)	
		pt.y=(((DestDisplaySize.cy-DisplaySize.cy)>>1))&0xfffc;


#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintDec("\n a mem offset x:  ",pt.x);
		VIM_USER_PrintDec("\n a mem offset y:  ",pt.y);
#endif		
	result =VIM_DISP_SetA_Memory(pt,DestDisplaySize);
	if(result)
		goto DISPLAYERROR;
	
	pt.x=(LcdOffsetX)&0xfffc;
	pt.y=(LcdOffsetY)&0xfffc;
	if(DestDisplaySize.cx<=DisplaySize.cx)	
		pt.x=(((DisplaySize.cx-DestDisplaySize.cx)>>1)+LcdOffsetX)&0xfffc;
	else
		DestDisplaySize.cx=DisplaySize.cx;
	if(DestDisplaySize.cy<=DisplaySize.cy)	
		pt.y=(((DisplaySize.cy-DestDisplaySize.cy)>>1)+LcdOffsetY)&0xfffc;
	else
		DestDisplaySize.cy=DisplaySize.cy;
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintDec("\n a display offset x:  ",pt.x);
		VIM_USER_PrintDec("\n a display offset y:  ",pt.y);
		VIM_USER_PrintDec("\n new DisplaySize width:  ",DestDisplaySize.cx);
		VIM_USER_PrintDec("\n new DisplaySize height:  ",DestDisplaySize.cy);
#endif	

	VIM_DISP_SetA_DisplayPanel(pt,DestDisplaySize);
	result = VIM_MAPI_AdjustPoint((VIM_DISP_ROTATEMODE)gVc0528_Info.LcdStatus.ARotationMode,
					VIM_DISP_NOTCHANGE);
	if(result)
			goto DISPLAYERROR;
	VIM_DISP_SetRotateMode(VIM_DISP_ALAYER,(VIM_DISP_ROTATEMODE)gVc0528_Info.LcdStatus.ARotationMode);

#if VIM_USER_SUPPORT_REALTIME_ROTATION
		VIM_DISP_GetAWinsize(&DestDisplaySize);
		VIM_HIF_SetReg16(V5_REG_LCDC_BGW_L, DestDisplaySize.cx);
		VIM_HIF_SetReg16(V5_REG_LCDC_BGH_L, DestDisplaySize.cy);
#endif

	//enable a layer
	VIM_DISP_ResetState();
	VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER,ENABLE);
	
	
	//set layer A mode, normal display use ALAYER_LINEBUF mode
	gVc0528_Info.MarbStatus.ALayerMode = VIM_USER_DISPLAY_USELINEBUF;
	//set jpeg module parameter and marb map memory
	if(gVc0528_Info.DisplayStatus.BufPoint)
		result = VIM_MAPI_SetDisplayJpegParameter();
	else
		result = VIM_MAPI_SetAutoDisplayJpegParameter();
DISPLAYERROR:
	if(result)
	{
		if(SaveMode==VIM_HAPI_ROM_SAVE)
		{
			VIM_USER_FreeMemory(gVc0528_Info.DisplayStatus.MallocPr);
			gVc0528_Info.CaptureStatus.MallocPr=0;
		}
		return (UINT16)result;
	}
	return VIM_SUCCEED;
}
/***************************************************************************************************************************************
Description:
	Write one frame data to Jbuf for decode,if file end return file end, if do ok and have next frame,return VIM_SUCCEED
Parameters:
      jpegBuf: 	the buffer point of jpeg
      length: 	the length of jpeg (=>jpeg file length)
Return:
	VIM_SUCCEED:  ok, write one frame data to jbuf succeed
	VIM_ERROR_WORKMODE: error work mode
	VIM_ERROR_YUV_UNKNOW: next frame jpeg yuv mode error
	VIM_ERROR_FILE_END: file end
	VIM_ERROR_?????????: return form mid api
Note:
	LcdOffsetX+DisplayWidth<=LCD width
	LcdOffsetY+DisplayHeigth<=LCD height
	length>=jpeg file length
Remarks:
state: 
	valid
Modify History:
	Version			Modifyby				DataTime					modified
	   0.1			Amanda Deng			2005.12.20				create first version
	   0.2			Amanda Deng			2006.01.06				only write one frame for display function, user can call it by itself need
	   0.3			Amanda Deng 		2006.01.16				add teo parameter for this function
          1.0			angela				2006.12.8				change it to can seek file
 ****************************************************************************************************************************************/
UINT16 VIM_HAPI_DisplayOneFrame(HUGE UINT8 *jpegBuf, UINT32 length)
{
	UINT32  readCnt=0;
	VIM_RESULT result;
	//if define this current frame address then display the define
	if((jpegBuf)&&(length))	
	{
		gVc0528_Info.DisplayStatus.BufPoint=jpegBuf;
		gVc0528_Info.DisplayStatus.BufLength=length;
		result = VIM_MAPI_GetJpegInformation(gVc0528_Info.DisplayStatus.BufPoint,gVc0528_Info.DisplayStatus.BufLength);
		if(result)
			return (UINT16)result;
	}	
	
	gVc0528_Info.MarbStatus.Jpgpoint=gVc0528_Info.MarbStatus.MapList.jbufstart;
	if (gVc0528_JpegInfo.frmEnd ==TRUE)
	{
	        VIM_JPEG_SetJpegInfo(&gVc0528_JpegInfo);
		//ready start do display
		VIM_MAPI_StartDecode();
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintDec("\n VIM_HAPI_DisplayOneFrame! =",gVc0528_Info.DisplayStatus.FileLength);
#endif	
		VIM_MAPI_WriteOneFrameData(gVc0528_JpegInfo.eop-gVc0528_JpegInfo.offset, gVc0528_Info.DisplayStatus.BufPoint+gVc0528_JpegInfo.offset);
		gVc0528_Info.DisplayStatus.BufPoint+=gVc0528_JpegInfo.eop;
		gVc0528_Info.DisplayStatus.BufLength-=gVc0528_JpegInfo.eop;
		result = VIM_MAPI_GetJpegInformation(gVc0528_Info.DisplayStatus.BufPoint,gVc0528_Info.DisplayStatus.BufLength);

		if((result)||(gVc0528_JpegInfo.frmEnd==FALSE))
		{

			if(gVc0528_Info.DisplayStatus.SaveMode==VIM_HAPI_RAM_SAVE)
				goto SHOULDSTOPVIDEO;
			else
			{
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
				VIM_USER_PrintString("\n should read file!");
#endif	
				gVc0528_Info.DisplayStatus.BufPoint-=(VIM_USER_BUFLENGTH-gVc0528_Info.DisplayStatus.BufLength);
				result=VIM_USER_SeekFile(gVc0528_Info.DisplayStatus.pFileHandle,gVc0528_Info.DisplayStatus.FileLength);
				if(result)
					goto SHOULDSTOPVIDEO;
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
				VIM_USER_PrintDec("\n seek file ! =",gVc0528_Info.DisplayStatus.FileLength);
#endif	
				readCnt = VIM_USER_ReadFile(gVc0528_Info.DisplayStatus.pFileHandle,  gVc0528_Info.DisplayStatus.BufPoint, VIM_USER_BUFLENGTH);
				if(!readCnt)
				{
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
				VIM_USER_PrintString("\n file end!");
#endif					
					goto SHOULDSTOPVIDEO;
				}
				else
				{
					result = VIM_MAPI_GetJpegInformation(gVc0528_Info.DisplayStatus.BufPoint,readCnt);
					if((result)||(gVc0528_JpegInfo.frmEnd==FALSE))
					{
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
						VIM_USER_PrintDec("\n VIM_MAPI_GetJpegInformation error !readCnt=",readCnt);
#endif	
						goto SHOULDSTOPVIDEO;
					}
					else
						gVc0528_Info.DisplayStatus.BufLength=VIM_USER_BUFLENGTH;
				}
			}
		}
	}
	else
		goto SHOULDSTOPVIDEO;
	return VIM_SUCCEED;

SHOULDSTOPVIDEO:
	gVc0528_Info.DisplayStatus.BufPoint-=(VIM_USER_BUFLENGTH-gVc0528_Info.DisplayStatus.BufLength);
	result=(VIM_RESULT)VIM_HAPI_StopDisplayVideo();
	return result;
}
/***************************************************************************************************************************************
Description:
	Display a picture (need not to wait until having been displayed on screen)
Parameters:
      jpegBuf: 	the buffer point of jpeg
      length: 	the length of jpeg ( must be file length)
Return:
	VIM_SUCCEED:  ok, write one frame data to jbuf succeed
	VIM_ERROR_WORKMODE: error work mode
	VIM_ERROR_YUV_UNKNOW: next frame jpeg yuv mode error
	VIM_ERROR_FILE_END: file end
	VIM_ERROR_?????????: return form mid api
Note:
	LcdOffsetX+DisplayWidth<=LCD width
	LcdOffsetY+DisplayHeigth<=LCD height
	length>=jpeg file length
	Note the jpeg buf should not be zero.
Remarks:
state: 
	valid
Modify History:
	
****************************************************************************************************************************************/
UINT16 VIM_HAPI_QuickDisplayOneFrame(HUGE UINT8 *jpegBuf, UINT32 length)
{
	UINT32  templen,Truelen;

	//if define this current frame address then display the define
	if(!jpegBuf||!length)
		return VIM_ERROR_CANNOT_QUICK_DISPLAY;
	if((jpegBuf)&&(length))	
	{
		gVc0528_Info.DisplayStatus.BufPoint=jpegBuf;
		gVc0528_Info.DisplayStatus.BufLength=length;
		VIM_JPEG_ToolJpegParse(jpegBuf, length, &gVc0528_JpegInfo,0);
		if((VIM_JPEG_YUVMODE)gVc0528_JpegInfo.YUVType == JPEG_UNKNOWN) 
			return VIM_ERROR_YUV_UNKNOW;
	}	
	gVc0528_Info.MarbStatus.Jpgpoint=gVc0528_Info.MarbStatus.MapList.jbufstart;
	VIM_JPEG_SetJpegInfo(&gVc0528_JpegInfo);
	//ready start do display
	VIM_MAPI_StartDecode();

	templen=VIM_MARB_GetJbufRWSize(gVc0528_Info.MarbStatus.Jpgpoint);
	Truelen=length-gVc0528_JpegInfo.offset+2;
	VIM_DISP_ResetState();
	if(templen>=Truelen)
	{
			gVc0528_Info.MarbStatus.Jpgpoint=VIM_MARB_WriteJpegData(jpegBuf+gVc0528_JpegInfo.offset,Truelen,gVc0528_Info.MarbStatus.Jpgpoint,1);
	}
	else
	{
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintString("\n VIM_ERROR_CANNOT_QUICK_DISPLAY : ");
#endif
		return VIM_ERROR_CANNOT_QUICK_DISPLAY;
	}
	//VIM_MAPI_WriteOneFrameData(gVc0528_JpegInfo.eop-gVc0528_JpegInfo.offset, gVc0528_Info.DisplayStatus.BufPoint+gVc0528_JpegInfo.offset);
	return VIM_SUCCEED;
}
/***************************************************************************************************************************************
Description:
	display Video(*.AVI,*.MJPG) on LCD
Parameters:
Return:
	VIM_SUCCEED:  ok
Note:
Remarks:
state: 
	valid
Modify History:
	Version			Modifyby				DataTime					modified
	   0.1			Amanda Deng			2006.03.27				create first version
****************************************************************************************************************************************/
UINT16 VIM_HAPI_StopDisplayVideo(void)
{
	if(gVc0528_Info.VideoStatus.Mode!=VIM_VIDEO_STARTDISPLAYE)
       	return VIM_ERROR_VIDEO_MODE;
	VIM_USER_StopTimer();
	gVc0528_Info.VideoStatus.Mode=VIM_VIDEO_STOP;
	if(gVc0528_Info.DisplayStatus.SaveMode==VIM_HAPI_ROM_SAVE)
	{
		VIM_USER_FreeMemory(gVc0528_Info.DisplayStatus.MallocPr);
	#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintString("\n free memory because video stop!");
	#endif	
		gVc0528_Info.CaptureStatus.MallocPr=0;
	}
	if(gVc0528_Info.VideoStatus.CapCallback)
	 	gVc0528_Info.VideoStatus.CapCallback(VIM_HAPI_CAPTURE_END,0);
	return VIM_SUCCEED;
}
/***************************************************************************************************************************************
Description:
	display Video(*.AVI,*.MJPG) on LCD
Parameters:
	pUCallBack
	     bFramRate: frame rate (per second) if 0.do not start timer to capture (customer will display jpeg themsleves)
	dwMaxFrameCount: the max frame count which the user wanted

Return:
Note:

Remarks:
state: 
	valid
Modify History:
	Version			Modifyby				DataTime					modified
	   0.1			Amanda Deng			2006.03.27				create first version
	 
****************************************************************************************************************************************/
UINT16 VIM_HAPI_StartDisplayVideo(VIM_HAPI_CallBack pUCallBack,UINT8 bFrameRate)
{
	UINT16 result;

#if VIM_USER_SUPPORT_REALTIME_ROTATION
	VIM_HIF_SetReg16(V5_REG_LCDC_BGW_L, gVc0528_Info.pUserInfo->MainPanel.Size.cx);
	VIM_HIF_SetReg16(V5_REG_LCDC_BGH_L, gVc0528_Info.pUserInfo->MainPanel.Size.cy);

	gVc0528_Info.MarbStatus.ALayerMode = VIM_USER_DISPLAY_USELINEBUF;
	result = VIM_MAPI_SetDisplayJpegParameter();
	if(result)
		goto STARTDISPLAYVIDEOERROR;
#endif
	
	gVc0528_Info.VideoStatus.Mode=VIM_VIDEO_STARTDISPLAYE;
	gVc0528_Info.VideoStatus.VideoFrameRate=bFrameRate;
	VIM_USER_StopTimer();
	gVc0528_Info.VideoStatus.CapCallback=pUCallBack;
	result=VIM_USER_StartTimer(1000/bFrameRate);
	if(result)
		return (UINT16)result;
	return VIM_SUCCEED;

#if VIM_USER_SUPPORT_REALTIME_ROTATION
STARTDISPLAYVIDEOERROR:
	if(result)
	{
		if(gVc0528_Info.DisplayStatus.MallocPr)
		{
			VIM_USER_FreeMemory(gVc0528_Info.DisplayStatus.MallocPr);
			gVc0528_Info.CaptureStatus.MallocPr=0;
		}
		return (UINT16)result;
	}
#endif		
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
Modify History:
	Version			Modifyby				DataTime					modified
	   0.1			Amanda Deng			2005.12.20				create first version
	   0.2			Amanda Deng			2005.01.06				only set display work mode and parameter
	   0.3			Amanda Deng			2005.02.08				add file save mode (in RAM/ROM)
****************************************************************************************************************************************/
UINT16 VIM_HAPI_Display_Jpeg(VIM_HAPI_SAVE_MODE SaveMode,HUGE void *jpegBuf, UINT32 length,UINT16 LcdOffsetX, UINT16 LcdOffsetY, UINT16 DisplayWidth, UINT16 DisplayHeight)
{
      	UINT32  readCnt=0;
	UINT16 result;

	
      	result=VIM_HAPI_ReadyToDisplay(SaveMode, jpegBuf, length, LcdOffsetX, LcdOffsetY, DisplayWidth,  DisplayHeight);
	if(result)
		return result;
	
	if(gVc0528_Info.DisplayStatus.BufPoint)
	{
		gVc0528_Info.MarbStatus.Jpgpoint=gVc0528_Info.MarbStatus.MapList.jbufstart;
		
		if (gVc0528_JpegInfo.frmEnd ==TRUE)
			VIM_MAPI_WriteOneFrameData(gVc0528_Info.DisplayStatus.FileLength-gVc0528_JpegInfo.offset, gVc0528_Info.DisplayStatus.BufPoint+gVc0528_JpegInfo.offset);
		else
		{
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
			VIM_USER_PrintDec(" first write ",gVc0528_JpegInfo.eop - gVc0528_JpegInfo.offset);
#endif
			gVc0528_Info.DisplayStatus.FileLength+=gVc0528_JpegInfo.eop;

			VIM_MAPI_WriteOneFrameData(((VIM_USER_BUFLENGTH - gVc0528_JpegInfo.offset)>>2)<<2, //angela 2007-2-25
					gVc0528_Info.DisplayStatus.BufPoint+gVc0528_JpegInfo.offset);

			if(gVc0528_JpegInfo.offset%4)		//angela 2007-2-25
				VIM_USER_SeekFile(gVc0528_Info.DisplayStatus.pFileHandle,VIM_USER_BUFLENGTH-(4-(gVc0528_JpegInfo.offset%4)));
			//need read data from ROM to RAM then display
			while((gVc0528_JpegInfo.frmEnd==FALSE))
			{
				readCnt = VIM_USER_ReadFile(gVc0528_Info.DisplayStatus.pFileHandle, gVc0528_Info.DisplayStatus.BufPoint, VIM_USER_BUFLENGTH);
				if(readCnt<VIM_USER_BUFLENGTH)
				{
					gVc0528_JpegInfo.frmEnd=TRUE;
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
					VIM_USER_PrintDec(" read finish ",gVc0528_Info.DisplayStatus.FileLength);
#endif
				}
				gVc0528_Info.DisplayStatus.FileLength+=readCnt;
				//gVc0528_Info.DisplayStatus.BufLength-=readCnt;
				VIM_MAPI_WriteOneFrameData(readCnt, gVc0528_Info.DisplayStatus.BufPoint);
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)

				VIM_USER_PrintDec("\n have finish file data:  ",readCnt);
#endif	

			}
		}
		if(SaveMode==VIM_HAPI_ROM_SAVE)
		{
			VIM_USER_FreeMemory(gVc0528_Info.DisplayStatus.MallocPr);
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
			VIM_USER_PrintString("\n free memory because file display finish!  ");
#endif	
			gVc0528_Info.CaptureStatus.MallocPr=0;
		}
	}
	else
		VIM_MAPI_JpegAutoDecode(gVc0528_Info.CaptureStatus.CapFileLength); 
	return VIM_SUCCEED;
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
UINT16 VIM_HAPI_DisplayPan(UINT16 LcdOffsetX, UINT16 LcdOffsetY, UINT16 JpegOffsetX, UINT16 JpegOffsetY,UINT16 DisplayWidth, UINT16 DisplayHeight)
{
	VIM_RESULT result;
       TSize DisplaySize,DestDisplaySize;
	TPoint pt = {0, 0};
	UINT32 readCnt=0;
	
	//check the chip working mode 
	if(gVc0528_Info.ChipWorkMode!=VIM_HAPI_MODE_CAMERAON&&gVc0528_Info.ChipWorkMode!=VIM_HAPI_MODE_PHOTOPROCESS) 
		return VIM_ERROR_WORKMODE;
	
	gVc0528_Info.DisplayStatus.FileLength=0;
	if(gVc0528_Info.DisplayStatus.SaveMode==VIM_HAPI_ROM_SAVE)
	{
		gVc0528_Info.DisplayStatus.BufOffset=0;
		gVc0528_Info.DisplayStatus.BufLength=VIM_USER_BUFLENGTH;
		gVc0528_Info.DisplayStatus.MallocPr=gVc0528_Info.DisplayStatus.BufPoint=VIM_USER_MallocMemory(VIM_USER_BUFLENGTH);
		if(gVc0528_Info.DisplayStatus.BufPoint==NULL)
			return VIM_ERROR_BUFPOINT;
		//read file from ROM to user define RAM
		result = VIM_USER_SeekFile(gVc0528_Info.DisplayStatus.pFileHandle,0);
		if(result)
			goto DISPLAYPANERROR;
		readCnt = VIM_USER_ReadFile(gVc0528_Info.DisplayStatus.pFileHandle,  gVc0528_Info.DisplayStatus.BufPoint, VIM_USER_BUFLENGTH);
	}

	//disable syncgen
	VIM_SIF_EnableSyncGen(DISABLE);		//angela 2006-8-15
	
	//get jpeg information( YVU mode, image size and other table parameters
	result = VIM_MAPI_GetJpegInformation(gVc0528_Info.DisplayStatus.BufPoint,readCnt);
	if(result)
		goto DISPLAYPANERROR;

	// set all module to display jpeg mode
	VIM_MAPI_SetDisplayJpegMode();  
	gVc0528_Info.PreviewStatus.Mode=VIM_HAPI_PREVIEW_OFF;


	//check display size>{4,4},must less than (LCD size),get display size
	result = VIM_MAPI_CheckDisplaySize(LcdOffsetX,LcdOffsetY,DisplayWidth,DisplayHeight,&DisplaySize);
	if(result)
		goto DISPLAYPANERROR;
	result = VIM_MAPI_CheckDisplayPanSize(JpegOffsetX,JpegOffsetY,DisplaySize,&DestDisplaySize,&pt);	
	if(result)
		goto DISPLAYPANERROR;
	
	//from version 0.2 need display whole pic in LCD, so source size =jpeg image size
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintDec("\n jpeg Image width:  ",gVc0528_JpegInfo.ImageSize.cx);
		VIM_USER_PrintDec("\n jpeg Image height:  ",gVc0528_JpegInfo.ImageSize.cy);
		VIM_USER_PrintDec("\n yuv is:  %d",gVc0528_JpegInfo.YUVType);
#endif	


	//set ipp module image size
	VIM_IPP_SetImageSize(gVc0528_JpegInfo.JpgSize.cx, gVc0528_JpegInfo.JpgSize.cy);


#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintDec("\n jpeg offset x:  ",pt.x);
		VIM_USER_PrintDec("\n jpeg offset y:  ",pt.y);
		VIM_USER_PrintDec("\n Dest DisplaySize width:  ",DestDisplaySize.cx);
		VIM_USER_PrintDec("\n Dest DisplaySize height:  ",DestDisplaySize.cy);
#endif	
	//set ipp module source window size, display size
	result = VIM_IPP_SetDispalySize(pt, DestDisplaySize, DestDisplaySize);
	if(result)
		goto DISPLAYPANERROR;
		
	//set LCDC A layer memory size and window size
	pt.x=pt.y=0;
	if(DestDisplaySize.cx>DisplaySize.cx)	
		pt.x=(JpegOffsetX)&0xfffc;
	if(DestDisplaySize.cy>DisplaySize.cy)	
		pt.y=(JpegOffsetY)&0xfffc;
	
	
	result =VIM_DISP_SetA_Memory(pt,DestDisplaySize);
	if(result)
		goto DISPLAYPANERROR;
	
	pt.x=(LcdOffsetX)&0xfffc;
	pt.y=(LcdOffsetY)&0xfffc;
	if(DestDisplaySize.cx<=DisplaySize.cx)	
		pt.x=(((DisplaySize.cx-DestDisplaySize.cx)>>1)+LcdOffsetX)&0xfffc;
	else
		DestDisplaySize.cx=DisplaySize.cx;
	if(DestDisplaySize.cy<=DisplaySize.cy)	
		pt.y=(((DisplaySize.cy-DestDisplaySize.cy)>>1)+LcdOffsetY)&0xfffc;
	else
		DestDisplaySize.cy=DisplaySize.cy;
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintDec("\n a display offset x:  ",pt.x);
		VIM_USER_PrintDec("\n a display offset y:  ",pt.y);
		VIM_USER_PrintDec("\n new DisplaySize width:  ",DestDisplaySize.cx);
		VIM_USER_PrintDec("\n new DisplaySize height:  ",DestDisplaySize.cy);
#endif	

	VIM_DISP_SetA_DisplayPanel(pt,DestDisplaySize);
	result = VIM_MAPI_AdjustPoint((VIM_DISP_ROTATEMODE)gVc0528_Info.LcdStatus.ARotationMode,
					VIM_DISP_NOTCHANGE);
	if(result)
			goto DISPLAYPANERROR;
	VIM_DISP_SetRotateMode(VIM_DISP_ALAYER,(VIM_DISP_ROTATEMODE)gVc0528_Info.LcdStatus.ARotationMode);


	//enable a layer
	VIM_DISP_ResetState();
	VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER,ENABLE);
	
	
	//set layer A mode, normal display use ALAYER_LINEBUF mode
	gVc0528_Info.MarbStatus.ALayerMode = VIM_USER_DISPLAY_USELINEBUF;
	//set jpeg module parameter and marb map memory
	if(gVc0528_Info.DisplayStatus.BufPoint)
		result = VIM_MAPI_SetDisplayJpegParameter();
	else
		result = VIM_MAPI_SetAutoDisplayJpegParameter();

	
	if(gVc0528_Info.DisplayStatus.BufPoint)
		{
			gVc0528_Info.MarbStatus.Jpgpoint=gVc0528_Info.MarbStatus.MapList.jbufstart;
			
			if (gVc0528_JpegInfo.frmEnd ==TRUE)
				VIM_MAPI_WriteOneFrameData(gVc0528_Info.DisplayStatus.FileLength-gVc0528_JpegInfo.offset, gVc0528_Info.DisplayStatus.BufPoint+gVc0528_JpegInfo.offset);
			else
			{
	#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
				VIM_USER_PrintDec(" first write ",gVc0528_JpegInfo.eop - gVc0528_JpegInfo.offset);
	#endif
				gVc0528_Info.DisplayStatus.FileLength+=gVc0528_JpegInfo.eop;
				VIM_MAPI_WriteOneFrameData(VIM_USER_BUFLENGTH - gVc0528_JpegInfo.offset, 
					gVc0528_Info.DisplayStatus.BufPoint+gVc0528_JpegInfo.offset);
				//need read data from ROM to RAM then display
				//while((gVc0528_JpegInfo.frmEnd==FALSE)&&(gVc0528_Info.DisplayStatus.BufLength))
				while((gVc0528_JpegInfo.frmEnd==FALSE))
				{
					readCnt = VIM_USER_ReadFile(gVc0528_Info.DisplayStatus.pFileHandle, gVc0528_Info.DisplayStatus.BufPoint, VIM_USER_BUFLENGTH);
					if(readCnt<VIM_USER_BUFLENGTH)
					{
						gVc0528_JpegInfo.frmEnd=TRUE;
	#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
						VIM_USER_PrintDec(" read finish ",gVc0528_Info.DisplayStatus.FileLength);
	#endif
					}
					gVc0528_Info.DisplayStatus.FileLength+=readCnt;
					//gVc0528_Info.DisplayStatus.BufLength-=readCnt;
					VIM_MAPI_WriteOneFrameData(readCnt, gVc0528_Info.DisplayStatus.BufPoint);
	#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)

					VIM_USER_PrintDec("\n have finish file data:  ",readCnt);
	#endif	

				}
			}
			if(gVc0528_Info.DisplayStatus.SaveMode==VIM_HAPI_ROM_SAVE)
			{
				VIM_USER_FreeMemory(gVc0528_Info.DisplayStatus.MallocPr);
	#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
				VIM_USER_PrintString("\n free memory because file display finish!  ");
	#endif	
				gVc0528_Info.CaptureStatus.MallocPr=0;
			}
		}
	
DISPLAYPANERROR:
	if(result)
	{
		if(gVc0528_Info.DisplayStatus.SaveMode==VIM_HAPI_ROM_SAVE)
		{
			VIM_USER_FreeMemory(gVc0528_Info.DisplayStatus.MallocPr);
			gVc0528_Info.CaptureStatus.MallocPr=0;
		}
		return (UINT16)result;
	}
 	return VIM_SUCCEED;



}

/***************************************************************************************************************************************
Description:
	display zoom function(cutting the image from the source jpeg picture by ZoomStep parameter)
Parameters:
      	JpegOffsetX,JpegOffsetY	:	the start coordinate of jpeg picture
    	ZoomStep:	the step number when zoom the jpeg picture
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
	   0.2			Heju 				2008.02.26				modified

****************************************************************************************************************************************/
UINT16 VIM_HAPI_DisplayZoom(UINT16 JpegOffsetX, UINT16 JpegOffsetY,VIM_HAPI_DISPLAY_ZOOM ZoomStep)
{
	VIM_RESULT result;
	UINT32 readCnt=0;
	
	//check the chip working mode 
	if(gVc0528_Info.ChipWorkMode!=VIM_HAPI_MODE_CAMERAON&&gVc0528_Info.ChipWorkMode!=VIM_HAPI_MODE_PHOTOPROCESS) 
		return VIM_ERROR_WORKMODE;
	
	result=VIM_MAPI_ReadyToDisplayZoom(JpegOffsetX,JpegOffsetY,ZoomStep);
	if(result)
		return result;
	
	if(gVc0528_Info.DisplayStatus.BufPoint)
	{
		gVc0528_Info.MarbStatus.Jpgpoint=gVc0528_Info.MarbStatus.MapList.jbufstart;
		
		if (gVc0528_JpegInfo.frmEnd ==TRUE)
			VIM_MAPI_WriteOneFrameData(gVc0528_Info.DisplayStatus.FileLength-gVc0528_JpegInfo.offset, gVc0528_Info.DisplayStatus.BufPoint+gVc0528_JpegInfo.offset);
		else
		{
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
			VIM_USER_PrintDec(" first write ",gVc0528_JpegInfo.eop - gVc0528_JpegInfo.offset);
#endif
			gVc0528_Info.DisplayStatus.FileLength+=gVc0528_JpegInfo.eop;

			VIM_MAPI_WriteOneFrameData(((VIM_USER_BUFLENGTH - gVc0528_JpegInfo.offset)>>2)<<2, //angela 2007-2-25
					gVc0528_Info.DisplayStatus.BufPoint+gVc0528_JpegInfo.offset);

			if(gVc0528_JpegInfo.offset%4)		//angela 2007-2-25
				VIM_USER_SeekFile(gVc0528_Info.DisplayStatus.pFileHandle,VIM_USER_BUFLENGTH-(4-(gVc0528_JpegInfo.offset%4)));
			//need read data from ROM to RAM then display
			while((gVc0528_JpegInfo.frmEnd==FALSE))
			{
				readCnt = VIM_USER_ReadFile(gVc0528_Info.DisplayStatus.pFileHandle, gVc0528_Info.DisplayStatus.BufPoint, VIM_USER_BUFLENGTH);
				if(readCnt<VIM_USER_BUFLENGTH)
				{
					gVc0528_JpegInfo.frmEnd=TRUE;
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
					VIM_USER_PrintDec(" read finish ",gVc0528_Info.DisplayStatus.FileLength);
#endif
				}
				gVc0528_Info.DisplayStatus.FileLength+=readCnt;
				//gVc0528_Info.DisplayStatus.BufLength-=readCnt;
				VIM_MAPI_WriteOneFrameData(readCnt, gVc0528_Info.DisplayStatus.BufPoint);
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)

				VIM_USER_PrintDec("\n have finish file data:  ",readCnt);
#endif	

			}
		}
		if(gVc0528_Info.DisplayStatus.SaveMode==VIM_HAPI_ROM_SAVE)
		{
			VIM_USER_FreeMemory(gVc0528_Info.DisplayStatus.MallocPr);
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
			VIM_USER_PrintString("\n free memory because file display finish!  ");
#endif	
			gVc0528_Info.CaptureStatus.MallocPr=0;
		}
	}	

 	return VIM_SUCCEED;



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
UINT16 VIM_HAPI_Decode_Jpeg(VIM_HAPI_SAVE_MODE SaveMode,VIM_HAPI_DECODE_MODE DecodeMode,HUGE void *jpegBuf, HUGE UINT8 *pDest,UINT32 jpeglength,UINT32 Destlength,UINT16 TargetWidth,UINT16 TargetHeight)
{
	VIM_RESULT result;
	UINT32  readCnt=0;
	
	if(gVc0528_Info.ChipWorkMode!=VIM_HAPI_MODE_CAMERAON&&gVc0528_Info.ChipWorkMode!=VIM_HAPI_MODE_PHOTOPROCESS) 
		return VIM_ERROR_WORKMODE;

	gVc0528_Info.DisplayStatus.DecPoint=pDest;
	gVc0528_Info.DisplayStatus.DecLength=Destlength;
      	result=VIM_MAPI_ReadyToDecode(SaveMode, jpegBuf,DecodeMode, jpeglength, TargetWidth,  TargetHeight);
	if(result)
		return result;

	if(gVc0528_Info.DisplayStatus.BufPoint)
	{
		gVc0528_Info.MarbStatus.Jpgpoint=gVc0528_Info.MarbStatus.MapList.jbufstart;
		if (gVc0528_JpegInfo.frmEnd ==TRUE)
			result=VIM_MAPI_DecodeOneFrame(gVc0528_Info.DisplayStatus.FileLength-gVc0528_JpegInfo.offset, gVc0528_Info.DisplayStatus.BufPoint+gVc0528_JpegInfo.offset);
		else
		{
			//result=VIM_ERROR_DECODE;

			if(SaveMode==VIM_HAPI_RAM_SAVE)
				return VIM_ERROR_DECODE_ERROR;
			
	
			//guoying add 1/14/2008
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
			VIM_USER_PrintDec(" first decode ",gVc0528_JpegInfo.eop - gVc0528_JpegInfo.offset);
#endif
			gVc0528_Info.DisplayStatus.FileLength+=gVc0528_JpegInfo.eop;	//real jpeg data length

			VIM_MAPI_DecodeOneFrame(((VIM_USER_BUFLENGTH - gVc0528_JpegInfo.offset)>>2)<<2, //decode the first block jpeg data in the VIM_USER_BUFLENGTH area
					gVc0528_Info.DisplayStatus.BufPoint+gVc0528_JpegInfo.offset);

			if(gVc0528_JpegInfo.offset%4)		//angela 2007-2-25
				VIM_USER_SeekFile(gVc0528_Info.DisplayStatus.pFileHandle,VIM_USER_BUFLENGTH-(4-(gVc0528_JpegInfo.offset%4)));

			//loop to decode the next block jpeg data in the VIM_USER_BUFLENGTH area
			while((gVc0528_JpegInfo.frmEnd==FALSE))
			{
				
				readCnt= VIM_USER_ReadFile(gVc0528_Info.DisplayStatus.pFileHandle, (gVc0528_Info.DisplayStatus.BufPoint), (VIM_USER_BUFLENGTH));
				
				if(readCnt<VIM_USER_BUFLENGTH)
				{
					gVc0528_JpegInfo.frmEnd=TRUE;
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
					VIM_USER_PrintDec(" read jpeg data finish ",gVc0528_Info.DisplayStatus.FileLength);
#endif
				}	

				gVc0528_Info.DisplayStatus.FileLength+=readCnt;

				VIM_MAPI_DecodeOneFrame(readCnt, gVc0528_Info.DisplayStatus.BufPoint);
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)

				VIM_USER_PrintDec("\n have finish file data:  ",readCnt);
#endif	
			}
	
					
		}
		

	}
	else
		VIM_MAPI_JpegAutoDecode(gVc0528_Info.CaptureStatus.CapFileLength);
	VIM_HIF_SetModClkOn(VIM_HIF_CLOCK_LCDIF);

	if(SaveMode==VIM_HAPI_ROM_SAVE)
	{
			VIM_USER_FreeMemory(gVc0528_Info.DisplayStatus.MallocPr);
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
			VIM_USER_PrintString("\n free memory because file display finish!  ");
#endif	
			gVc0528_Info.CaptureStatus.MallocPr=0;
	}	
	return result;

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

UINT16 VIM_HAPI_YuvToRgb(HUGE UINT8 *pYUV, HUGE UINT8 *pRgb,UINT16 Source_Width,UINT16 Source_height,UINT8 Method)
{
	switch(Method)
		{
		case VIM_CON_UYVY_565:
			VIM_JPEG_Toolyuv422_Rgb565((UINT8*)pYUV,(UINT8*)pRgb,Source_Width, Source_height);
			break;
		default:
			break;
		}
	return VIM_SUCCEED;
}



UINT16 VIM_HAPI_SetPreviewWhiteBalance(VIM_HAPI_WHITEBALANCE_MODE Mode,UINT8 Step)
{

	if(gVc0528_Info.ChipWorkMode!=VIM_HAPI_MODE_CAMERAON)
		return VIM_ERROR_WORKMODE;

	if(gVc0528_Info.pSensorInfo->snrWhiteBalanceCall!=NULL)
	{
		if(Mode==VIM_HAPI_WHITEBALANCE_MANUAL&&(Step>VIM_USER_WHITEBALANCE_MANUALSTEP))
			gVc0528_Info.pSensorInfo->snrWhiteBalanceCall(Mode,VIM_USER_WHITEBALANCE_MANUALSTEP);
		else
			gVc0528_Info.pSensorInfo->snrWhiteBalanceCall(Mode,Step);
		return VIM_SUCCEED;
	}
	else
		return VIM_ERROR_WHITEBALANCE;	
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
		1-5
	Return:
		VIM_SUCCEED: set brightness ok
		VIM_ERROR_WORKMODE: error work mode
	Note:
		This function must be used in Camera open mode, and only be effect with RGB sensor
	Remarks:
		state: valid

*********************************************************************************/

UINT16 VIM_HAPI_SetPreviewBrightness(VIM_HAPI_CHANGE_MODE Mode,UINT8 Step)
{

	if(gVc0528_Info.ChipWorkMode!=VIM_HAPI_MODE_CAMERAON)
		return VIM_ERROR_WORKMODE;
	switch(Mode)
		{
		case VIM_HAPI_CHANGE_ADD:
			gVc0528_Info.PreviewStatus.brightness+=Step;
			if(gVc0528_Info.PreviewStatus.brightness>VIM_USER_BRITNESSMAX_STEP)
			{				
				gVc0528_Info.PreviewStatus.brightness=VIM_USER_BRITNESSMAX_STEP;
			}
			break;

		case VIM_HAPI_CHANGE_DEC:
			if(gVc0528_Info.PreviewStatus.brightness&&(Step<=gVc0528_Info.PreviewStatus.brightness))
				gVc0528_Info.PreviewStatus.brightness-=Step;
			else
				gVc0528_Info.PreviewStatus.brightness=0;
			break;
		case VIM_HAPI_SET_STEP:// 1-26
			if(Step<=VIM_USER_BRITNESSMAX_STEP)
				gVc0528_Info.PreviewStatus.brightness=Step;
			else
				return VIM_ERROR_BRIGHT_CONTRAST;
			break;
		default:
		case VIM_HAPI_CHANGE_NORMAL:
			gVc0528_Info.PreviewStatus.brightness=gVc0528_Info.pSensorInfo->brightness;
				break;
		}
	if(gVc0528_Info.pSensorInfo->snrBrightnessCall!=NULL)
	{
		gVc0528_Info.pSensorInfo->snrBrightnessCall(gVc0528_Info.PreviewStatus.brightness);
		return VIM_SUCCEED;
	}
	else
		return VIM_ERROR_BRIGHT_CONTRAST;	
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
		1-5
	Return:
		VIM_SUCCEED: set contrast ok
		VIM_ERROR_WORKMODE: error work mode
	Note:
		This function must be used in Camera open mode, and only be effect with RGB sensor
	Remarks:
		state: valid
*********************************************************************************/
UINT16 VIM_HAPI_SetPreviewContrast(VIM_HAPI_CHANGE_MODE Mode,UINT8 Step)
{

	if(gVc0528_Info.ChipWorkMode!=VIM_HAPI_MODE_CAMERAON)
		return VIM_ERROR_WORKMODE;
	switch(Mode)
		{
		case VIM_HAPI_CHANGE_ADD:
			gVc0528_Info.PreviewStatus.contrast+=Step;
			if(gVc0528_Info.PreviewStatus.contrast>VIM_USER_CONTRASTMAX_STEP)
			{				
				gVc0528_Info.PreviewStatus.contrast=VIM_USER_CONTRASTMAX_STEP;
			}
			break;
		case VIM_HAPI_CHANGE_DEC:
			if(gVc0528_Info.PreviewStatus.contrast&&(Step<=gVc0528_Info.PreviewStatus.contrast))
				gVc0528_Info.PreviewStatus.contrast-=Step;
			else
				gVc0528_Info.PreviewStatus.contrast=0;
			break;
		case VIM_HAPI_SET_STEP:// 1-26
			if(Step<=VIM_USER_CONTRASTMAX_STEP)
				gVc0528_Info.PreviewStatus.contrast=Step;
			else
				return VIM_ERROR_BRIGHT_CONTRAST;
			break;
		default:
		case VIM_HAPI_CHANGE_NORMAL:
			gVc0528_Info.PreviewStatus.contrast=gVc0528_Info.pSensorInfo->contrast;
				break;
		}
	if(gVc0528_Info.pSensorInfo->snrContrastCall!=NULL)
	{
		gVc0528_Info.pSensorInfo->snrContrastCall(gVc0528_Info.PreviewStatus.contrast);
		return VIM_SUCCEED;
	}
	else
		return VIM_ERROR_BRIGHT_CONTRAST;		
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
UINT16 VIM_HAPI_SetPreviewMirror(VIM_HAPI_SENSORMIRROR mode)
{
	if(gVc0528_Info.ChipWorkMode!=VIM_HAPI_MODE_CAMERAON)
		return VIM_ERROR_WORKMODE;
	
	if(gVc0528_Info.PreviewStatus.Mode==VIM_HAPI_PREVIEW_OFF)
		{
			gVc0528_Info.PreviewStatus.SensorMirror=(VIM_MAPI_SENSOR_MIMODE)mode;
			return VIM_SUCCEED;  //angela changed for set preview mirror in preview on
		}
	VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER,DISABLE);	
	VIM_USER_DelayMs(100);
	if(gVc0528_Info.pSensorInfo->snrMirrorFlipCall!=NULL)
	{
		gVc0528_Info.pSensorInfo->snrMirrorFlipCall(mode);
		VIM_DISP_ResetState();
	}
	VIM_USER_DelayMs(100);// one frame end
	VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER,ENABLE);	
	gVc0528_Info.PreviewStatus.SensorMirror=(VIM_MAPI_SENSOR_MIMODE)mode;

return VIM_SUCCEED;
}
/********************************************************************************
Description:
	sensor work mode 
Parameters:
	mode:
		VIM_HAPI_SENSOR_OUT = 0x0,		out door condition
		VIM_HAPI_SENSOR_50HZ=0x1,		in door 50hz
		VIM_HAPI_SENSOR_60HZ=2,		in door	60hz
		VIM_HAPI_SENSOR_NIGHT=3,		night
	Return:
		VIM_SUCCEED: set mirror ok
		VIM_ERROR_WORKMODE: error work mode
	Note:
		This function must be used in Camera open mode
Remarks:
	state: valid
*********************************************************************************/
UINT16 VIM_HAPI_SetPreviewExMode(VIM_HAPI_SENSORMODE mode)
{

	if(gVc0528_Info.ChipWorkMode!=VIM_HAPI_MODE_CAMERAON)
		return VIM_ERROR_WORKMODE;
	
	if(gVc0528_Info.PreviewStatus.Mode==VIM_HAPI_PREVIEW_OFF)
		{
			gVc0528_Info.PreviewStatus.SensorMode=(VIM_HAPI_SENSORMODE)mode;
			return VIM_SUCCEED;  //angela changed for set preview mirror in preview on
		}
	VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER,DISABLE);	
	//V5M_PreviewMirror(mode);
	VIM_USER_DelayMs(100);
	if(gVc0528_Info.pSensorInfo->snrModeCall!=NULL)
	{
		gVc0528_Info.pSensorInfo->snrModeCall(mode);
		VIM_DISP_ResetState();
	}
	VIM_USER_DelayMs(100);// one frame end
	VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER,ENABLE);	
	gVc0528_Info.PreviewStatus.SensorMode=(VIM_HAPI_SENSORMODE)mode;

return VIM_SUCCEED;
}

/********************************************************************************
Description:
	Set special effect
Parameters:
	Mode:
		 VIM_HAPI_SPECIAL_NORMAL.
		 VIM_HAPI_SPECIAL_MONOCHROME,	 //���怨ㅼ�
		 VIM_HAPI_SPECIAL_SEPIA,		//���������
		 VIM_HAPI_SPECIAL_NEGATIVE,	//������
		 VIM_HAPI_SPECIAL_REDONLY,	 //�����옙�꿨����
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
UINT16 VIM_HAPI_SetEffect(VIM_HAPI_SPECIAL_EFFECT Mode)
{
UINT8 SpeCtrl;
#if 1
	if(gVc0528_Info.ChipWorkMode==VIM_HAPI_MODE_BYPASS)
		return VIM_ERROR_WORKMODE;
	SpeCtrl=(VIM_HIF_GetReg8(V5_REG_IPP_SPECTRL)&0xE0);
	switch(Mode)
		{
		case  VIM_HAPI_SPECIAL_MONOCHROME: 
			SpeCtrl|=0x01;
			SpeCtrl|=0x01<<1;
			VIM_IPP_SetEffectUVThreshold(0,0,0,0);
			break;
		case  VIM_HAPI_SPECIAL_REDONLY: 
			SpeCtrl|=0x01;
			SpeCtrl|=0x01<<1;
			VIM_IPP_SetEffectUVThreshold(128,85,255,128);
			break;
		case  VIM_HAPI_SPECIAL_SEPIA:
			SpeCtrl|=0x01;
			SpeCtrl|=0x00<<1;
			VIM_IPP_SetEffectUVoffset(85,128);
			break;
		case  VIM_HAPI_SPECIAL_NEGATIVE:
			SpeCtrl|=0x01;
			SpeCtrl|=0x02<<1;
			break;
		/*case  VIM_HAPI_SPECIAL_RELIEF:
			SpeCtrl|=0x01;
			SpeCtrl|=0x04<<1;
			break;*/

		case  VIM_HAPI_SPECIAL_NORMAL:
		default:
			break;
		}
	VIM_HIF_SetReg8(V5_REG_IPP_SPECTRL,SpeCtrl);
	gVc0528_Info.PreviewStatus.SpecialEffect=(VIM_HAPI_SPECIAL_EFFECT)Mode;
	#endif
	return VIM_SUCCEED;
}


/********************************************************************************
Description:
	get frame number of capture(video)
Parameters:

Return:
	frame number captured
Note:
		
Remarks:
state: 
	valid
*********************************************************************************/
UINT32 VIM_HAPI_GetFrmCount(void)
{
	if(gVc0528_Info.ChipWorkMode!=VIM_HAPI_MODE_CAMERAON) 
       		return VIM_ERROR_WORKMODE;
	return gVc0528_Info.VideoStatus.NowFrame;
}
/********************************************************************************
Description:
	interrupt service routine
Parameters:

Return:
	VIM_SUCCEED:  ok
	VIM_ERROR_WORKMODE: error work mode
Note:
	This function should be called by the user's CPU's external interrupt ISR, or a 50ms timer when 
	start preview or start capture.
		
Remarks:
state: 
	valid
*********************************************************************************/
void VIM_HAPI_Timer(void)
{
	UINT32 dwOnelen;
	UINT16 result;
	if(gVc0528_Info.ChipWorkMode==VIM_HAPI_MODE_BYPASS ||gVc0528_Info.ChipWorkMode==VIM_HAPI_MODE_DIRECTDISPLAY)
       		return ;
	 if((gVc0528_Info.VideoStatus.Mode==VIM_VIDEO_STARTCAPTURE)&&(gVc0528_Info.VideoStatus.VideoFrameRate))
	 {
	 	result=VIM_HAPI_GetOneJpeg(gVc0528_Info.CaptureStatus.BufPoint,gVc0528_Info.CaptureStatus.BufLength,&dwOnelen);
	 	if(result)
	 	{
	 		if(gVc0528_Info.VideoStatus.CapCallback)
	 		{
	 			if((result!=VIM_ERROR_CAPTURE_BUFFULL)&&(result!=VIM_ERROR_MAX_FRAMECOUNT))
	 				gVc0528_Info.VideoStatus.CapCallback(VIM_HAPI_BUF_ERROR,result);
	 		}
	 		VIM_HAPI_StopCapture();
	 	}
	 	else
	 	{
	 		if(gVc0528_Info.VideoStatus.CapCallback)
	 			gVc0528_Info.VideoStatus.CapCallback(VIM_HAPI_ONEFRAME_END,dwOnelen);
	 	}
	 }
#if VIM_USER_SURPORT_AVI
	else  if((gVc0528_Info.VideoStatus.Mode==VIM_VIDEO_STARTCAPTURE)&&(!gVc0528_Info.VideoStatus.VideoFrameRate)
		&&(AVI_Status.NowStuatus==AVI_OPEN))
	 {
	 	Avi_Timer();
	 }
#endif
	 else if((gVc0528_Info.VideoStatus.Mode==VIM_VIDEO_STARTDISPLAYE)&&(gVc0528_Info.VideoStatus.VideoFrameRate))
	 {
	      	result=VIM_HAPI_DisplayOneFrame(NULL,0);
		if (result)
   			VIM_HAPI_StopDisplayVideo();
		else
	 	{
	 		if(gVc0528_Info.VideoStatus.CapCallback)
	 			gVc0528_Info.VideoStatus.CapCallback(VIM_HAPI_ONEFRAME_END,0);
	 	}	
	 }
	 else
	 	_ISR_HIF_IntHandle();
	 return ;
}

#ifdef CONFIG_MACH_CANOPUS
void VIM_HAPI_SetBuf(HUGE void *StillBuf,UINT32 BUF_Length)
{
	gVc0528_Info.CaptureStatus.BufPoint = StillBuf;
	gVc0528_Info.CaptureStatus.BufLength = BUF_Length;
}

void VIM_HAPI_AbortCapture()
{
	if (gVc0528_Info.VideoStatus.Mode == VIM_VIDEO_STARTCAPTURE)
		gVc0528_Info.VideoStatus.Abort = 1;
}
#endif

#if VIM_USER_SUPPORT_REALTIME_ROTATION
/********************************************************************************
Description:
	change display Video size on LCD
Parameters:
	UINT16 pt_x: the start point X on the LCD when play the video 
	UINT16 pt_y: the start point Y on the LCD when play the video
	UINT16 width: the width of display video on the LCD
	UINT16 height: the height of display video on the LCD
	
Return:
Note:

Remarks:
state: 
	valid
Modify History:
	 
********************************************************************************/
UINT16 VIM_HAPI_ChangeVideoSize(UINT16 pt_x,UINT16 pt_y,UINT16 width,UINT16 height)
{
	VIM_RESULT result;
       TSize DisplaySize,DestDisplaySize,SoucseSize;
	TPoint pt = {0, 0};

	UINT8 bZoomOut=1;
	
	if(gVc0528_Info.ChipWorkMode!=VIM_HAPI_MODE_CAMERAON&&gVc0528_Info.ChipWorkMode!=VIM_HAPI_MODE_PHOTOPROCESS) 
		return VIM_ERROR_WORKMODE;
		
	VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER,DISABLE);	

	DisplaySize.cx=width;
	DisplaySize.cy=height;
	
	//from version 0.2 need display whole pic in LCD, so source size =jpeg image size
	SoucseSize.cx=gVc0528_JpegInfo.ImageSize.cx;
	SoucseSize.cy=gVc0528_JpegInfo.ImageSize.cy;

	//get real display size(display image in user define display size, maybe not full display in LCD, only part of user define display size)
	if(VIM_USER_DISPLAY_FULLSCREEN)
		result = VIM_IPP_ToolCaculateBigDstWindow(SoucseSize,DisplaySize,&DestDisplaySize);
	else
		result = VIM_IPP_ToolCaculateLessDisplayWindow(SoucseSize,DisplaySize,&DestDisplaySize);
	if(result)
		goto CHANGEDISPLAYMPEG4ERROR;

	if((DisplaySize.cx/2<DestDisplaySize.cx)&&(DestDisplaySize.cx<=DisplaySize.cx))
		DestDisplaySize.cx=DisplaySize.cx;
	else	 if (DestDisplaySize.cx<=DisplaySize.cx/2)
		DestDisplaySize.cx=DestDisplaySize.cx*2;
	else
		bZoomOut=0;
	if((DisplaySize.cy/2<DestDisplaySize.cy)&&(DestDisplaySize.cy<=DisplaySize.cy))
		DestDisplaySize.cy=DisplaySize.cy;
	else	 if (DestDisplaySize.cy<=DisplaySize.cy/2)
		DestDisplaySize.cy=DestDisplaySize.cy*2;	

	if(bZoomOut&&DestDisplaySize.cy<SoucseSize.cy)/*to avoid the situation of diffrent sizer selection of width and height*/
		DestDisplaySize.cy=SoucseSize.cy;
	else if(!bZoomOut&&DestDisplaySize.cy>SoucseSize.cy)
		DestDisplaySize.cy=SoucseSize.cy;		
	

	//DestDisplaySize.cx=DestDisplaySize.cx&0xfff8;
	//DestDisplaySize.cy=DestDisplaySize.cy&0xfff8;
	//set ipp module image size
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintDec("[VC0508][HAPI]Dest DisplaySize width:  ",DestDisplaySize.cx);
		VIM_USER_PrintDec("[VC0508][HAPI] Dest DisplaySize height:  ",DestDisplaySize.cy);
#endif	
	//set ipp module source window size, display size


	result = VIM_IPP_SetDispalySize(pt, SoucseSize, DestDisplaySize);
	if(result)
		goto CHANGEDISPLAYMPEG4ERROR;


			
	//set LCDC A layer memory size and window size
	pt.x=pt.y=0;
	if(DestDisplaySize.cx>DisplaySize.cx)	
		pt.x=(((DestDisplaySize.cx-DisplaySize.cx)>>1))&0xfffc;
	if(DestDisplaySize.cy>DisplaySize.cy)	
		pt.y=(((DestDisplaySize.cy-DisplaySize.cy)>>1))&0xfffc;


#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintDec("[VC0508][HAPI]a mem offset x:  ",pt.x);
		VIM_USER_PrintDec("[VC0508][HAPI]a mem offset y:  ",pt.y);
#endif		
	result =VIM_DISP_SetA_Memory(pt,DestDisplaySize);
	if(result)
		goto CHANGEDISPLAYMPEG4ERROR;
	
	pt.x=pt_x&0xfffc;
	pt.y=pt_y&0xfffc;
	if(DestDisplaySize.cx<=DisplaySize.cx)	
		pt.x=(((DisplaySize.cx-DestDisplaySize.cx)>>1)+pt_x)&0xfffc;
	else
		DestDisplaySize.cx=DisplaySize.cx;
	if(DestDisplaySize.cy<=DisplaySize.cy)	
		pt.y=(((DisplaySize.cy-DestDisplaySize.cy)>>1)+pt_y)&0xfffc;
	else
		DestDisplaySize.cy=DisplaySize.cy;
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintDec("[VC0578][HAPI]a display offset x:  ",pt.x);
		VIM_USER_PrintDec("[VC0578][HAPI] a display offset y:  ",pt.y);
		VIM_USER_PrintDec("[VC0578][HAPI] new DisplaySize width:  ",DestDisplaySize.cx);
		VIM_USER_PrintDec("[VC0578][HAPI] new DisplaySize height:  ",DestDisplaySize.cy);
#endif	

	VIM_DISP_SetA_DisplayPanel(pt,DestDisplaySize);
	result = VIM_MAPI_AdjustPoint((VIM_DISP_ROTATEMODE)gVc0528_Info.LcdStatus.ARotationMode,
					VIM_DISP_NOTCHANGE);
	if(result)
		return result;
	VIM_DISP_SetRotateMode(VIM_DISP_ALAYER,(VIM_DISP_ROTATEMODE)gVc0528_Info.LcdStatus.ARotationMode);
	//enable a layer
	VIM_DISP_ResetState();
	VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER,ENABLE);

	    //pisel rate
       VIM_JPEG_AdjustClkBlank();

	return VIM_SUCCEED;
CHANGEDISPLAYMPEG4ERROR:
	if(gVc0528_Info.DisplayStatus.MallocPr)
	{
		VIM_USER_FreeMemory(gVc0528_Info.DisplayStatus.MallocPr);
		gVc0528_Info.CaptureStatus.MallocPr=0;
	}

	{

		VIM_USER_StopTimer();
	}	
	return result;
}
#endif
/********************************************************************************
Description:
	display yuv data from the data buffer
Parameters:
	VIM_HAPI_DISPYUV_Data *struc: the head point of struc buffer
Return:
	VIM_SUCCEED:  ok
	VIM_ERROR_WORKMODE: error work mode
Note:
	This function used for change rgb565 data to yuv 422 uyvy
		
Remarks:
state: 
	valid
*********************************************************************************/

UINT16 VIM_HAPI_DisplayYUV(VIM_HAPI_DISPYUV_Data *struc)
{
	UINT16 result;

	
      	result=VIM_MAPI_ReadyToDisplayYUV((VIM_DISPYUV_Data *)struc);
	if(result)
		return result;
	
	VIM_MAPI_WriteOneFrameYUVData((UINT32)(struc->SourceSize_W*struc->SourceSize_H*2), (UINT8 *)(struc->yuv422));
//	VIM_MARB_WriteJpegData( (UINT8 *)(struc->yuv422),(UINT32)(struc->SourceSize_W*struc->SourceSize_H*2),gVc0528_Info.MarbStatus.Jpgpoint,1);
		
	return VIM_SUCCEED;	
}

/********************************************************************************
Description:
	Convert RGB565 to yuv 422 data
Parameters:
	UINT8 *pRGB : the head point of rgb data buf
	UINT8 *pYUV: the head point of yuv data buf 422 uyvy
	UINT16 Width: the width of image
	UINT16 Height: the height of image
Return:
	VIM_SUCCEED:  ok
	VIM_ERROR_WORKMODE: error work mode
Note:
	This function used for change rgb565 data to yuv 422 uyvy
		
Remarks:
state: 
	valid
*********************************************************************************/

UINT16 VIM_HAPI_RGB565toYUV422(UINT8 *pRGB, UINT8 *pYUV,UINT16 Width,UINT16 Height)
{
UINT16 i,j;
UINT8 v0;
UINT8 r,g,b;
	for(i=0;i<Height;i++)
	{
		for(j=0;j<Width;j++)
		{
			b = (((*pRGB)&0x1f)<<3);
			g =((*(pRGB+1)&0x7)<<5)|((*(pRGB)&0xe0)>>3); pRGB++;
			r =((*pRGB)&0xf8); pRGB++;

			if(!(j%2))
			{

				*pYUV=(-43*r - 85*g + 128*b )/256+128;//u0
				pYUV++;
				v0=(128*r - 107*g - 21*b)/256+ 128;
			}
			else
			{
				*pYUV=v0;
				pYUV++;
			}
			*pYUV= (77*r + 150*g + 29*b )/256;//y
			pYUV++;
		}
	}
return 0;
}

/********************************************************************************
Description:
	encode YUV422 to jpeg data
Parameters:
	UINT8 *pJpeg : the head point of jpeg data buf
	UINT8 *pYUV: the head point of yuv data buf 422 uyvy
	UINT16 Width: the width of image
	UINT16 Height: the height of image
	UINT32 JpbufLen: the data lengh in jpeg buffer
Return:
	VIM_SUCCEED:  ok
	VIM_ERROR_WORKMODE: error work mode
Note:
	This function used for encode yuv422 to jpeg
		
Remarks:
state: 
	valid
*********************************************************************************/

UINT16 VIM_HAPI_Encode422(UINT8 *pYUV, UINT8 *pJpeg,UINT16 Width,UINT16 Height,UINT32 JpbufLen)
{
UINT32 dwCount=500000;
TSize Size;
	//check the chip working mode 
	if(gVc0528_Info.ChipWorkMode!=VIM_HAPI_MODE_CAMERAON) 
		return VIM_ERROR_WORKMODE;

     	VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER,DISABLE);	
	VIM_SIF_EnableSyncGen(DISABLE);
	VIM_MAPI_Delay_Frame(1);
      	//set mode
      	Size.cx=Width;
	Size.cy=Height;
      	VIM_MAPI_SetChipMode(VIM_MARB_ENCODE_MODE,VIM_IPP_HAVE_NOFRAME);
	VIM_JPEG_SetSize(JPEG_422,Size);
	VIM_MARB_SetMap(gVc0528_Info.MarbStatus.WorkMode,VIM_DISP_NODISP,
		(PVIM_MARB_Map)&gVc0528_Info.MarbStatus.MapList);


       // init parameter
	gVc0528_Info.CaptureStatus.SaveMode=VIM_HAPI_RAM_SAVE;
	gVc0528_Info.CaptureStatus.BufPoint=pJpeg;

	if(gVc0528_Info.CaptureStatus.BufPoint==NULL)
		return VIM_ERROR_BUFPOINT;

	gVc0528_Info.CaptureStatus.RecaptureTimes=0;
	gVc0528_Info.CaptureStatus.ThumbBuf=0;
	gVc0528_Info.CaptureStatus.CapFileLength=0;
	gVc0528_Info.CaptureStatus.CapCallback=NULL;
	gVc0528_Info.CaptureStatus.BufLength=JpbufLen;	
	gVc0528_Info.CaptureStatus.CaptureError=VIM_SUCCEED;
	gVc0528_Info.MarbStatus.Jpgpoint=gVc0528_Info.MarbStatus.MapList.jbufstart;

	//set qf
	VIM_JPEG_SetBitRateControlEn(DISABLE);
	VIM_JPEG_SetBrcQF(10);

	// set buffer gapratio
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
	VIM_USER_PrintString("\n start one capture ");
	VIM_USER_PrintDec("\n jbuf size =",gVc0528_Info.MarbStatus.MapList.jbufsize);
#endif	
	VIM_MARB_SetJpegbufGapratio(1);
	VIM_MARB_SetRecaptureEn(DISABLE);
	gVc0528_Info.CaptureStatus.Mode=VIM_CAPTURESTILL;


	// init intrrupt
	VIM_MAPI_InitCapInterrupt();
	VIM_MARB_ResetSubModuleAutoEn(ENABLE);

	//reset state
	VIM_JPEG_ResetState();
       VIM_MARB_StartCapture();

       // start encode
       VIM_JPEG_StartEncode();
       while(dwCount--)
       {
		if(VIM_JPEG_GetStatus()&0x8)
		{
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
			VIM_USER_PrintString("\n end of head  ");
#endif	

			break;	
		}
       }
	//send data

       VIM_MAPI_SendYuvData(pYUV,Size,VIM_YUV_UYVY422);
	//-----------------------------------------------------------
	if(gVc0528_Info.CaptureStatus.CapCallback==NULL)
	{
		while((gVc0528_Info.CaptureStatus.Mode!=VIM_CAPTUREDONE)&&(dwCount--))
		{
			VIM_USER_DelayMs(1); 
			_ISR_HIF_IntHandle();
			if(gVc0528_Info.CaptureStatus.CaptureError!=VIM_SUCCEED)
				return (UINT16)gVc0528_Info.CaptureStatus.CaptureError;
		}
		if(gVc0528_Info.CaptureStatus.Mode!=VIM_CAPTUREDONE)
			return VIM_ERROR_NOINITERRUPT;
	}
	return VIM_SUCCEED;

	
}

/********************************************************************************
Description:
	used LCD internel register to rotate LCD preview
Parameters:
	degree: rotate degree
	VIM_HAPI_ROTATE_0,
	VIM_HAPI_ROTATE_90,
	VIM_HAPI_ROTATE_180,
	VIM_HAPI_ROTATE_270,
	VIM_HAPI_MIRROR_0,
	VIM_HAPI_MIRROR_90,
	VIM_HAPI_MIRROR_180,
	VIM_HAPI_MIRROR_270
Return:
	
Remarks:
state: 
	valid
*********************************************************************************/
void VIM_HAPI_LCDInternalRotate(VIM_HAPI_ROTATEMODE degree, VIM_HAPI_WORKMODE WorkMode)
{

	if(gVc0528_Info.pDriverIcInfo->master->lcdRotate!=NULL)
		gVc0528_Info.pDriverIcInfo->master->lcdRotate((VIM_HAPI_ROTATEMODE)degree, (VIM_HAPI_WORKMODE)WorkMode);
	
		
	return;
}

/********************************************************************************
Description:
 This function can control the GPIO port(pin) output status.
 
Parameters:
 GPIOpin: this is the definition of GPIO pin from GPIO0 to GPIO4 (BIT0 to BIT4)
 IObit:  this is the description of IO pin output status
 
Return:

Remarks:
state: 
 	valid
*********************************************************************************/
void VIM_HAPI_SetGpioValue(VIM_HAPI_GPIOPIN GPIOpin,BOOL IObit)
{
 UINT8 temp;
 
 if ((VIM_HAPI_MODE_BYPASS == gVc0528_Info.ChipWorkMode) || (VIM_HAPI_MODE_DIRECTDISPLAY ==gVc0528_Info.ChipWorkMode))
 {
 
  VIM_HIF_SetBypassOrNormal((PVIM_BYPASS_DEF)&gVc0528_Info.pUserInfo->BypassDef,
    VIM_HIF_NORMALTYPE);
 
  VIM_HIF_SetExterPinCrlEn((VIM_HIF_EXTPIN_CTRL)GPIOpin,ENABLE);   //only enable GPIO pins
  VIM_HIF_SetExterPinValue((VIM_HIF_EXTPIN_CTRL)GPIOpin,IObit); 
  
  VIM_HIF_SetBypassOrNormal((PVIM_BYPASS_DEF)&gVc0528_Info.pUserInfo->BypassDef,
    VIM_HIF_BYPASSTYE);  
 } 
 else
 {
  temp=VIM_HIF_GetReg8(V5_REG_CPM_GPIO_P0);
  
  if(IObit)
   temp|=GPIOpin;
  else
   temp&=(~GPIOpin);
  VIM_HIF_SetReg8(V5_REG_CPM_GPIO_P0,temp);
 }
}


