/*************************************************************************
*                                                                       
*                Copyright (C) 2005 Vimicro CO.,LTD     		 
*
* FILE NAME                                  VERSION                
*                                                                       
* VIM_MID_API.c			   0.1                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*     VC0578 's function aip file                        
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
#include"VIM_COMMON.h"

VIM_STATUS gVc0528_Info;			//578 all work mode information define
VIM_JPEG_Index gVc0528_JpegInfo;	//jepg information define
VIM_HIF_TISR  gVc0528_Isr;//VIM_HIF_ISR;
/********************************************************************************
  Description:
  	init context
  Parameters:
  Returns:
  	void
  Remarks:
*********************************************************************************/
void VIM_MAPI_InitIfo(void)
{
	VIM_USER_MemSet(( UINT8 *)&gVc0528_Info,0,sizeof(gVc0528_Info));
	gVc0528_Info.pDriverIcInfo=(PTPanelInfo)&gDriverIcInfo;
	gVc0528_Info.pSensorInfo=(PTSnrInfo)&gPSensorInfo[0];
	gVc0528_Info.pUserInfo=(PVIM_USER_INITINFO)&g_UserInfo;
	gVc0528_Info.LcdStatus.Size=gVc0528_Info.pUserInfo->MainPanel.Size;
	gVc0528_Info.LcdStatus.NowPanel=VIM_HAPI_LCDPANEL_MAIN;
	gVc0528_Info.MarbStatus.ALayerMode=VIM_DISP_LINEBUF;
	gVc0528_Info.MarbStatus.MapList.jbufnowblock_int=3;
	gVc0528_Info.MarbStatus.WorkMode=VIM_MARB_BYPASS_MODE;
	gVc0528_Info.CaptureStatus.QualityMode=VIM_HAPI_CPTURE_QUALITY_MID;
	gVc0528_Info.VideoStatus.Mode=VIM_VIDEO_STOP;
	gVc0528_Info.CaptureStatus.ThumbSize.cx=0;
	gVc0528_Info.CaptureStatus.ThumbSize.cy=0;
	gVc0528_Info.LcdStatus.ColorDep=VIM_HAPI_COLORDEP_16BIT;
	VIM_HIF_ClearIntModule(INT_ALL);
}
/********************************************************************************
* Description:
*	Auto to find sensor from the list
*	
* Parameters:
*	void
* Return:
*	VIM_SUCCEED: 		find the fit sensor
*	VIM_ERRO_SENSOR_AUTOFIND: not find sensor.
**********************************************************************************/
VIM_RESULT VIM_MAPI_AutoFindSensor(void)
{
       VIM_RESULT Result;
	UINT8 i = 0;
	for(i=0; i<100; i++)
	{
		if(gPSensorInfo[i]==NULL)
		{
			gVc0528_Info.pSensorInfo = gPSensorInfo[i];
			return VIM_ERROR_SENSOR_AUTOFIND; //sensor error;
		}
		VIM_HIF_ResetSubModule(VIM_HIF_RESET_SIF);
		gVc0528_Info.pSensorInfo = gPSensorInfo[i];
		//VIM_SIF_SifIspInit(gVc0528_Info.pSensorInfo,
		VIM_SIF_SifInitI2c(gVc0528_Info.pSensorInfo,
			gVc0528_Info.pUserInfo->WorkMClk.Clkout); // Init i2c
			
		//reset sensor here
		VIM_SIF_SetSensorPowerOn(gVc0528_Info.pSensorInfo);//having reset sensor
 		if(gVc0528_Info.pSensorInfo->snrIdChkCall!= NULL)
		{
			VIM_USER_DelayMs(100);
		 	if(gVc0528_Info.pSensorInfo->snrIdChkCall() == SUCCEED)
		 	{
		 		VIM_USER_DelayMs(100);
		 		Result = VIM_SIF_SifSensorInit(gVc0528_Info.pSensorInfo);
		 		if (Result) 
		 		   return	Result;
		 		return VIM_SUCCEED;
		 	}
		}
	}

	return VIM_ERROR_SENSOR_AUTOFIND; //sensor error;
}
/********************************************************************************
  Description:
	Switch panel 
 Parameters:
	switch cs and point and info
  Returns:
  	void
  Remarks:
*********************************************************************************/

void VIM_MAPI_SwitchPanel(VIM_HAPI_LCDPANEL bPanel)
{
TPoint point={0,0};
TSize size={0,0};
	if(bPanel==VIM_HAPI_LCDPANEL_SUB)	//sub
	{
		VIM_LCDIF_SetBiasSpt(gVc0528_Info.pUserInfo->SubPanel.Point);
		gVc0528_Info.LcdStatus.Size=gVc0528_Info.pUserInfo->SubPanel.Size;
		gVc0528_Info.LcdStatus.NowPanel=bPanel;
		VIM_LCDIF_InitLcdIFParm(gVc0528_Info.pDriverIcInfo->slave);
		VIM_DISP_SetB0_DisplayPanel(point,size);
		VIM_DISP_SetB1_DisplayPanel(point,size);
		VIM_DISP_GetBSize(VIM_DISP_B1LAYER,&size);
	}
	else
	{
		VIM_LCDIF_SetBiasSpt(gVc0528_Info.pUserInfo->MainPanel.Point);
		gVc0528_Info.LcdStatus.Size=gVc0528_Info.pUserInfo->MainPanel.Size;
		gVc0528_Info.LcdStatus.NowPanel=bPanel;
		VIM_LCDIF_InitLcdIFParm(gVc0528_Info.pDriverIcInfo->master);
		VIM_DISP_SetB0_DisplayPanel(point,size);
		VIM_DISP_SetB1_DisplayPanel(point,size);
		VIM_DISP_GetBSize(VIM_DISP_B1LAYER,&size);
	}
}

/********************************************************************************

	Description:
		Test the read and write register 

	Parameters:
		Return:
		return 0:The function succeed
			Return 1:  read and write register wrong 
			Return 2:  read and write register wrong (MCLK domain)
			Return 3:  read and write Sram wrong (MCLK domain)

Note£º
	This function can be used in Camera open mode ;
						or Photo process mode; 
						or direct display mode ;
Remarks:
		state: valid

*********************************************************************************/
#define TESTSRAMNUM 48
VIM_RESULT VIM_MAPI_TestRegister (void)
{
UINT8 Test[3],TestSram[TESTSRAMNUM];
UINT16 Test16[2],i;
	Test[0]=0x55;
	Test[1]=0;
	VIM_HIF_SetReg8(V5_REG_BIU_BYPASS_ADDR_L,Test[0]);
	Test[1]=VIM_HIF_GetReg8(V5_REG_BIU_BYPASS_ADDR_L);
	if(Test[0]!=Test[1])
		return VIM_ERROR_HIF_READWRITE_REG8;

	Test[0]=VIM_USER_CheckBigendian();
	if(VIM_ENDIANTYPE!=Test[0])
		return VIM_ERROR_DEFINE_ENDIAN;

	Test16[0]=0x1155;
	Test16[1]=0;
	VIM_HIF_SetReg16(V5_REG_BIU_MEM_LOW_WORD_L,Test16[0]);
	Test16[1]=VIM_HIF_GetReg16(V5_REG_BIU_MEM_LOW_WORD_L);
	if(Test16[0]!=Test16[1])
		return VIM_ERROR_HIF_READWRITE_REG16;
	
	
	Test[0]=120;
	VIM_HIF_SetReg8(V5_REG_IPP_IMGWD,Test[0]);
	Test[2]=VIM_HIF_GetReg8(V5_REG_IPP_IMGWD);
	if(Test[0]!=Test[2])
		return VIM_ERROR_IPP_READWRITE_REG8;




	for(i=0;i<TESTSRAMNUM;i++)
		TestSram[i]=i;
	VIM_HIF_WriteSram(0, TestSram, TESTSRAMNUM);
	VIM_USER_MemSet(TestSram,0,TESTSRAMNUM);
	VIM_HIF_ReadSram(0, TestSram, TESTSRAMNUM);
	for(i=0;i<TESTSRAMNUM;i++)
		{
			if(TestSram[i]!=i)
				return VIM_ERROR_READWRITE_SRAM;
		}


	
	return VIM_SUCCEED;
}
/********************************************************************************
  Description:
  	init control
  Parameters:
  Returns:
  	void
  Remarks:
*********************************************************************************/
VIM_RESULT VIM_MAPI_InitHif(void)
{
VIM_RESULT Result;

	VIM_SET_XCLKON();
	VIM_USER_DelayMs(10);	//angela 2007-2-3
#if VIM_BUSTYPE==VIM_MULTI16
//->	VIM_HIF_SetMulti16();
#endif
	// init delay
	VIM_HIF_SetDelay(&(gVc0528_Info.pUserInfo->WorkTiming));        				
	//power on
    VIM_HIF_ResetSubModule(VIM_HIF_RESET_GLOBE);	                			
	// init value
	Result=VIM_HIF_InitPll(&(gVc0528_Info.pUserInfo->WorkMClk));  			
	if(Result)
		return Result;
	
	VIM_HIF_SetReg8(V5_REG_BIU_RESET_CORE, 0);	//angela 2006 11 17 update for ldo on
	VIM_HIF_SetLdoStatus(VIM_LDO_ON);
	VIM_USER_DelayMs(10);
	VIM_HIF_SetPllStatus(VIM_HIF_PLLPOWERON);  							
	VIM_HIF_SetReg8(V5_REG_BIU_RESET_CORE, 1); 						
	VIM_USER_DelayMs(10);
	VIM_HIF_SetReg8(V5_REG_BIU_STDBY, 0);      					
	VIM_HIF_SetReg8(V5_REG_BIU_PLL_RESET,1);   				
	VIM_USER_DelayMs(100);
	VIM_HIF_SetReg8(V5_REG_BIU_PLL_RESET,0);   				
	VIM_USER_DelayMs(100);
	VIM_HIF_SetModClkOn(VIM_HIF_CLOCK_ALL);    			
	//reset marb
	VIM_MARB_Set1TSramMode(VIM_MARB_1TSRAM_0,VIM_MARB_1TSRAM_POWERON);      	
	VIM_MARB_ResetSubModule(VIM_MARB_RESET_ALL); 								
	
	VIM_USER_DelayMs(1); 
	VIM_HIF_InitMarbRereshTime(&(gVc0528_Info.pUserInfo->WorkMClk)); 	          
	VIM_USER_DelayMs(5); 

	//test 
	Result=VIM_MAPI_TestRegister();
	if(Result)
		return Result;
	//init interrupt 
	Result=VIM_HIF_InitInerruptLevel(&(gVc0528_Info.pUserInfo->InitInterrupt)); 
	if(Result)
		return Result;
		
	// gpio init
	VIM_HIF_SetGpioInfo(VIM_PUTOUT,gVc0528_Info.pUserInfo->InitGpio.GpioValue); 

	//gpio bypass or register bypass
	VIM_HIF_InitBypass(gVc0528_Info.pUserInfo->BypassDef.BypassAddress,
	gVc0528_Info.pUserInfo->BypassDef.BypassType); 				                 

	// set one cs or two cs
	VIM_HIF_SetLcdCsActiveEn(ENABLE); 							                  
	VIM_HIF_SetLcdCsMode(gVc0528_Info.pUserInfo->BypassDef.BypassSubCtl);
	VIM_HIF_SwitchPanelCs(VIM_HAPI_LCDPANEL_MAIN);	

	//clear interrupt 
	VIM_HIF_ClearIntModule(INT_ALL);  											
	return VIM_SUCCEED;
}

/********************************************************************************
  Description:
  Parameters:
  note:
  	bBlock:
  		point = jbuf*bBlock/maxblock
  Returns:
  	void
  Remarks:
*********************************************************************************/
VIM_RESULT VIM_MAPI_SetJbufIntBlcok(UINT8 bBlock)
{
	if(bBlock>=VIM_MARB_JBUF_MAXBLOCK)
		return VIM_ERROR_JBUF_INTERRUPT_BLOCK;
	gVc0528_Info.MarbStatus.MapList.jbufnowblock_int=bBlock;
	return VIM_SUCCEED;
}

/********************************************************************************
  Description:
  	set chip work mode
  Parameters:
  Returns:
  	void
  Remarks:
*********************************************************************************/
void VIM_MAPI_SetChipMode(VIM_MARB_WORKMODE bMode,VIM_IPP_HAVEFRAM bHaveFrame)
{
	//if(gVc0528_Info.MarbStatus.WorkMode==bMode)
	//	return ;
	gVc0528_Info.MarbStatus.WorkMode=bMode;
	VIM_MARB_SetMode(bMode);
	switch(bMode)
	{
		case VIM_MARB_PREVIEW_MODE: 
			VIM_IPP_SetMode(VIM_IPP_MODE_PREVIEW,bHaveFrame);
			VIM_JPEG_SetMode(VIM_JPEG_MODE_PREVIEW);
			break;
		case VIM_MARB_CAPTURESTILL_MODE:
		case VIM_MARB_CAPTUREVIDEO_MODE:
		case VIM_MARB_MULTISHOT_MODE:
			VIM_IPP_SetMode(VIM_IPP_MODE_CAPTURE,bHaveFrame);
			VIM_JPEG_SetMode(VIM_JPEG_MODE_CAPTURE_MJPEG);
			break;
		case VIM_MARB_CAPTURESTILLTHUMB_MODE:
		case VIM_MARB_MULTISHOTTHUMB_MODE:
			VIM_IPP_SetMode(VIM_IPP_MODE_CAPTURE_WITHTHUMB,bHaveFrame);
			VIM_JPEG_SetMode(VIM_JPEG_MODE_CAPTURE_MJPEG);
			break;
		case VIM_MARB_CAPTUREAVI_MODE:
			VIM_IPP_SetMode(VIM_IPP_MODE_CAPTURE,bHaveFrame);
			VIM_JPEG_SetMode(VIM_JPEG_MODE_CAPTURE_AVI);
			break;
		case VIM_MARB_ENCODE_MODE:
			VIM_IPP_SetMode(VIM_IPP_MODE_CAPTURE,bHaveFrame);
			VIM_JPEG_SetMode(VIM_JPEG_MODE_ENCODE);
			break;
		case VIM_MARB_DISPLAYSTILL_MODE:
			VIM_IPP_SetMode(VIM_IPP_MODE_DISPLAY,bHaveFrame);
			VIM_JPEG_SetMode(VIM_JPEG_MODE_DISPLAY_JPEG);
			break;
		case VIM_MARB_DISPLAYSTILLBYPASS_MODE:
			VIM_IPP_SetMode(VIM_IPP_MODE_DISPLAY,bHaveFrame);
			VIM_JPEG_SetMode(VIM_JPEG_MODE_DISPLAY_YUV);
			break;
		case VIM_MARB_DISPLAYVIDEO_MODE:
			VIM_IPP_SetMode(VIM_IPP_MODE_DISPLAY,bHaveFrame);
			VIM_JPEG_SetMode(VIM_JPEG_MODE_DISPLAY_VIDEO);
			break;
		case VIM_MARB_DECODEYUV_MODE:
			VIM_IPP_SetMode(VIM_IPP_MODE_DECODE,bHaveFrame);
			VIM_JPEG_SetMode(VIM_JPEG_MODE_DECODE_JPEGTOYUV);
			break;
		case VIM_MARB_DECODERGB_MODE:
			VIM_IPP_SetMode(VIM_IPP_MODE_DECODE,bHaveFrame);
			VIM_JPEG_SetMode(VIM_JPEG_MODE_DECODE_JPEGTORGB);
			break;
		default:
			break;
	}

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
		this funcion should be called after caculte correct VWC
	Remarks:
		state: valid
*********************************************************************************/
VIM_RESULT VIM_MAPI_SetCaptureRatio(UINT8 ratio)
{
  UINT32 dwVwc;
	if((!ratio)||(ratio>63))
		return VIM_ERROR_PARAMETER;
	 dwVwc=VIM_JPEG_GetVideoWordCount();
	 dwVwc=dwVwc/ratio;
	 VIM_JPEG_SetTargetWordCount(dwVwc);
        VIM_JPEG_SetTargetCompressRatio(ratio);
        VIM_JPEG_SetBitRateControlEn(ENABLE);
	 return VIM_SUCCEED;		 
}
/********************************************************************************
Description:
	the capture done call back function
Parameters:
	Remarks:
		state: valid
*********************************************************************************/

void VIM_MAPI_ISR_JbufDone(void)
{
UINT32 length,dwSavelen=0;
	if(gVc0528_Info.CaptureStatus.CaptureError)
		goto CAPUTREERROR;	
GETLENGTH:
	length=dwSavelen=VIM_MARB_GetJbufRWSize(gVc0528_Info.MarbStatus.Jpgpoint);
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
	VIM_USER_PrintDec("\n VIM_MAPI_ISR_JbufDone,length is ",length);
#endif
	switch(gVc0528_Info.CaptureStatus.Mode)
	{
		case VIM_CAPTURESTILL:
		case VIM_CAPTURECAPTURE:
			if(length<=gVc0528_Info.CaptureStatus.BufLength)
			{
				gVc0528_Info.MarbStatus.Jpgpoint=VIM_MARB_ReadJpegData(gVc0528_Info.CaptureStatus.BufPoint,length,
					gVc0528_Info.MarbStatus.Jpgpoint);
				gVc0528_Info.CaptureStatus.BufLength-=length;
				gVc0528_Info.CaptureStatus.BufPoint+=length;
				gVc0528_Info.CaptureStatus.CapFileLength+=length;
			}
			else
			{
				if(gVc0528_Info.CaptureStatus.SaveMode==VIM_HAPI_RAM_SAVE)
				{
					gVc0528_Info.CaptureStatus.CaptureError=VIM_ERROR_CAPTURE_BUFFULL;
					goto CAPUTREERROR;	
				}
				else
				{
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
					VIM_USER_PrintDec("\n length>userbuf=%d ",gVc0528_Info.CaptureStatus.BufLength);
#endif
					gVc0528_Info.MarbStatus.Jpgpoint=VIM_MARB_ReadJpegData(gVc0528_Info.CaptureStatus.BufPoint,gVc0528_Info.CaptureStatus.BufLength,
						gVc0528_Info.MarbStatus.Jpgpoint);
					dwSavelen=gVc0528_Info.CaptureStatus.BufLength;
					gVc0528_Info.CaptureStatus.CapFileLength+=gVc0528_Info.CaptureStatus.BufLength;
					gVc0528_Info.CaptureStatus.BufPoint+=gVc0528_Info.CaptureStatus.BufLength;
					gVc0528_Info.CaptureStatus.BufLength=0;
				}
			}
			if(gVc0528_Info.CaptureStatus.SaveMode==VIM_HAPI_ROM_SAVE)
			{
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
				VIM_USER_PrintDec("\n save file ,length is ",(VIM_USER_BUFLENGTH-gVc0528_Info.CaptureStatus.BufLength));
#endif

				gVc0528_Info.CaptureStatus.CaptureError=VIM_USER_WriteFile(gVc0528_Info.CaptureStatus.pFile_Nameptr,
					gVc0528_Info.CaptureStatus.MallocPr,(VIM_USER_BUFLENGTH-gVc0528_Info.CaptureStatus.BufLength));
				gVc0528_Info.CaptureStatus.BufPoint=gVc0528_Info.CaptureStatus.MallocPr;
				gVc0528_Info.CaptureStatus.BufLength=VIM_USER_BUFLENGTH;
				if(gVc0528_Info.CaptureStatus.CaptureError)
					goto CAPUTREERROR;	
				if(length!=dwSavelen)
					goto GETLENGTH;
			}

			gVc0528_Info.CaptureStatus.Mode=VIM_CAPTUREDONE;
			goto CAPUTREOK;
			break;
	}
CAPUTREERROR:
	if(gVc0528_Info.CaptureStatus.CapCallback)
	{
		VIM_USER_StopTimer();
		VIM_HIF_ClearIntModule(INT_MARB);
		VIM_HIF_ClearIntModule(INT_JPEG);
		gVc0528_Info.CaptureStatus.CapCallback(VIM_HAPI_BUF_ERROR,gVc0528_Info.CaptureStatus.CaptureError);
	}
	if(gVc0528_Info.CaptureStatus.SaveMode==VIM_HAPI_ROM_SAVE)//angela2007-1-24
	{
		gVc0528_Info.CaptureStatus.CaptureError=VIM_USER_FreeMemory(gVc0528_Info.CaptureStatus.MallocPr);
		#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
			VIM_USER_PrintString("\n free memory because capture error!  ");
		#endif
		gVc0528_Info.CaptureStatus.MallocPr=0;
		gVc0528_Info.CaptureStatus.BufPoint=0;
	}
	return;
CAPUTREOK:
	if((gVc0528_Info.PreviewStatus.Mode==VIM_HAPI_PREVIEW_FRAMEON)&&(gVc0528_Info.VideoStatus.Mode==VIM_VIDEO_STOP))
	{
		VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER,DISABLE);	
		gVc0528_Info.PreviewStatus.Mode=VIM_HAPI_PREVIEW_OFF;
	}
	if(gVc0528_Info.CaptureStatus.ThumbBuf)
	{
		VIM_MARB_ReadThumbData(gVc0528_Info.CaptureStatus.ThumbBuf,gVc0528_Info.MarbStatus.MapList.thumbsize,
			gVc0528_Info.MarbStatus.MapList.thumbstart);
	}
	if(gVc0528_Info.CaptureStatus.SaveMode==VIM_HAPI_ROM_SAVE)
	{
		gVc0528_Info.CaptureStatus.CaptureError=VIM_USER_FreeMemory(gVc0528_Info.CaptureStatus.MallocPr);
		#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
			VIM_USER_PrintString("\n free memory because capture finish!  ");
		#endif
		gVc0528_Info.CaptureStatus.BufPoint=0;
		gVc0528_Info.CaptureStatus.MallocPr=0;
		if(gVc0528_Info.CaptureStatus.CaptureError)
				goto CAPUTREERROR;	
	}
	if(gVc0528_Info.PreviewStatus.Mode==VIM_HAPI_PREVIEW_OFF)
		VIM_MAPI_EnableSyncGen(DISABLE);	//angela 2006-9-4
	if(gVc0528_Info.CaptureStatus.CapCallback)
	{
		VIM_USER_StopTimer();
		gVc0528_Info.CaptureStatus.CapCallback(VIM_HAPI_CAPTURE_END,gVc0528_Info.CaptureStatus.CapFileLength);
	}
	return;
}
/********************************************************************************
Description:
	the one frame have been encode call back function
Parameters:
	Remarks:
		state: valid
*********************************************************************************/

void VIM_MAPI_ISR_OneFrameDone(void)
{
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
	VIM_USER_PrintDec("\n VIM_MAPI_ISR_OneFrameDone,rw=",VIM_MARB_GetJbufRWSize(gVc0528_Info.MarbStatus.Jpgpoint));
#endif
}

/********************************************************************************
Description:
	fifo count  call back function
Parameters:
	Remarks:
		state: valid
*********************************************************************************/

void VIM_MAPI_ISR_FifoCont(void)
{
UINT32 length,OneFifocount=VIM_MARB_GetJbufUpThreshold();
	length=VIM_MARB_GetJbufRWSize(gVc0528_Info.MarbStatus.Jpgpoint);
	switch(gVc0528_Info.CaptureStatus.Mode)
	{
		case VIM_CAPTURESTILL:
FIFOREAD:
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
	VIM_USER_PrintDec("\n VIM_MAPI_ISR_FifoCont length=",length);
	//VIM_USER_PrintDec("\n userbuf length=",gVc0528_Info.CaptureStatus.BufLength);
	
	VIM_USER_PrintDec("\n fifo count=",OneFifocount);

	VIM_USER_PrintDec("\n JBUF SIZE=",gVc0528_Info.MarbStatus.MapList.jbufsize);
#endif
			if(length<=gVc0528_Info.CaptureStatus.BufLength)
			{
				gVc0528_Info.MarbStatus.Jpgpoint=VIM_MARB_ReadJpegData(gVc0528_Info.CaptureStatus.BufPoint,length,
						gVc0528_Info.MarbStatus.Jpgpoint);
				gVc0528_Info.CaptureStatus.BufPoint+=length;
				gVc0528_Info.CaptureStatus.BufLength-=length;
				gVc0528_Info.CaptureStatus.CapFileLength+=length;
			}
			else
			{
				if(gVc0528_Info.CaptureStatus.SaveMode==VIM_HAPI_RAM_SAVE)
				{
					gVc0528_Info.CaptureStatus.CaptureError=VIM_ERROR_CAPTURE_BUFFULL;
					goto FIFOERROR;	
				}
				else
				{
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
					VIM_USER_PrintHex("\n  more than mallocbuf=",gVc0528_Info.CaptureStatus.BufLength);
#endif	
					gVc0528_Info.MarbStatus.Jpgpoint=VIM_MARB_ReadJpegData(gVc0528_Info.CaptureStatus.BufPoint,gVc0528_Info.CaptureStatus.BufLength,
						gVc0528_Info.MarbStatus.Jpgpoint);
					gVc0528_Info.CaptureStatus.BufPoint+=gVc0528_Info.CaptureStatus.BufLength;
					gVc0528_Info.CaptureStatus.CapFileLength+=gVc0528_Info.CaptureStatus.BufLength;
					gVc0528_Info.CaptureStatus.BufLength=0;
				}
			}
			if((gVc0528_Info.CaptureStatus.SaveMode==VIM_HAPI_ROM_SAVE)&&(gVc0528_Info.CaptureStatus.BufLength<=OneFifocount))
			{
				length=VIM_USER_BUFLENGTH-gVc0528_Info.CaptureStatus.BufLength;
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
				VIM_USER_PrintDec("\n VIM_MAPI_ISR_FifoCont should save =",length);
				VIM_USER_PrintDec(" file length  =",gVc0528_Info.CaptureStatus.CapFileLength);
#endif			

#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
//VIM_USER_PrintHex("\n before write gVc0528_Info.CaptureStatus.BufPoint=",(int)gVc0528_Info.CaptureStatus.BufPoint);
#endif	
				gVc0528_Info.CaptureStatus.CaptureError=VIM_USER_WriteFile(gVc0528_Info.CaptureStatus.pFile_Nameptr,
					gVc0528_Info.CaptureStatus.MallocPr,length);
				if(gVc0528_Info.CaptureStatus.CaptureError)
					goto FIFOERROR;	
				gVc0528_Info.CaptureStatus.BufLength=VIM_USER_BUFLENGTH;
				gVc0528_Info.CaptureStatus.BufPoint=gVc0528_Info.CaptureStatus.MallocPr;
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
	//VIM_USER_PrintHex("\n after write gVc0528_Info.CaptureStatus.BufPoint=",(int)gVc0528_Info.CaptureStatus.BufPoint);
#endif		
			}
			length=VIM_MARB_GetJbufRWSize(gVc0528_Info.MarbStatus.Jpgpoint);
			if(length==gVc0528_Info.MarbStatus.MapList.jbufsize)
			{
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
				VIM_USER_PrintString("\n length==gVc0528_Info.MarbStatus.MapList.jbufsize");
#endif	
				goto FIFOOK;
			}
			if(length>=(OneFifocount-2*VIM_MARB_JBUF_GAPINTERVAL))
				goto FIFOREAD;
			else
				goto FIFOOK;
			break;
	}
FIFOERROR:
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
	VIM_USER_PrintDec("\n VIM_MAPI_ISR_FifoCont error",gVc0528_Info.CaptureStatus.CaptureError);
#endif	
	VIM_USER_StopTimer();
	VIM_HIF_ClearIntModule(INT_MARB);
	VIM_HIF_ClearIntModule(INT_JPEG);
	if(gVc0528_Info.CaptureStatus.CapCallback)
		gVc0528_Info.CaptureStatus.CapCallback(VIM_HAPI_BUF_ERROR,gVc0528_Info.CaptureStatus.CaptureError);
	return;
FIFOOK:
	if(gVc0528_Info.CaptureStatus.CapCallback)
		gVc0528_Info.CaptureStatus.CapCallback(VIM_HAPI_FIFO_FULL,gVc0528_Info.CaptureStatus.CapFileLength);
	return;

}
/********************************************************************************
Description:
	the capture done call back function
Parameters:
	Remarks:
		state: valid
*********************************************************************************/

void VIM_MAPI_ISR_JbufError(void)
{
UINT8 bIntetype,i,addflag,marbflag;
	if(gVc0528_Info.CaptureStatus.RecaptureTimes<VIM_USER_RECAPTUREMAXTIMES)
	{
		VIM_JPEG_ResetState();
		VIM_MAPI_ISR_Recapture();
		gVc0528_Info.CaptureStatus.BufPoint-=gVc0528_Info.CaptureStatus.CapFileLength;
		gVc0528_Info.CaptureStatus.BufLength+=gVc0528_Info.CaptureStatus.CapFileLength;
		gVc0528_Info.CaptureStatus.CapFileLength=0;
		gVc0528_Info.MarbStatus.Jpgpoint=gVc0528_Info.MarbStatus.MapList.jbufstart;
		if(gVc0528_Info.CaptureStatus.SaveMode==VIM_HAPI_ROM_SAVE)
		{
			gVc0528_Info.CaptureStatus.BufPoint=gVc0528_Info.CaptureStatus.MallocPr;
			gVc0528_Info.CaptureStatus.BufLength=VIM_USER_BUFLENGTH;
			VIM_USER_SeekFile(gVc0528_Info.CaptureStatus.pFile_Nameptr,0);
		}
	       VIM_MARB_StartCapture();
       	return;
	}

	for(i=0;i<8;i++)
	{
		bIntetype=gVc0528_Isr.byFirstLevelInt[i];
		if(bIntetype==INT_MARBADD)
			addflag=gVc0528_Isr.bSecondLevelIntFlag[i];
		else if(bIntetype==INT_MARB)
			marbflag=gVc0528_Isr.bSecondLevelIntFlag[i];
	}

#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
	VIM_USER_PrintString("\n VIM_MAPI_ISR_JbufError");
#endif
	if(addflag&BIT3)
	{
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintString("\n recapture error");
#endif	
		gVc0528_Info.CaptureStatus.CaptureError=VIM_ERROR_CAPTURE_RECAPTURE;
	}
	else if(addflag&0x7)
	{
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintString("\n marb resfresh fail error");
#endif	
		gVc0528_Info.CaptureStatus.CaptureError=VIM_ERROR_CAPTURE_MARB;
	}
	else if(marbflag&BIT3)
	{
		gVc0528_Info.CaptureStatus.CaptureError=VIM_ERROR_CAPTURE_OVERFLOW;
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintString("\n jbuf overflow error");
#endif		
	}
	else
	{
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintString("\n error unknow");
		VIM_USER_PrintHex("\n marbflag=",marbflag);
		VIM_USER_PrintHex("\n addflag=",addflag);
#endif		
		gVc0528_Info.CaptureStatus.CaptureError=VIM_ERROR_CAPTURE_OVERFLOW;
	}
	if(gVc0528_Info.CaptureStatus.CapCallback)
	{
		VIM_USER_StopTimer();
		VIM_MARB_RegisterISR(MARB_INT_JBUF_FIFOCNT,NULL);//0307
		VIM_HIF_ClearIntModule(INT_MARB);
		VIM_HIF_ClearIntModule(INT_JPEG);
		gVc0528_Info.CaptureStatus.CapCallback(VIM_HAPI_BUF_ERROR,gVc0528_Info.CaptureStatus.CaptureError);
	}
	
}
/********************************************************************************
Description:
	the one frame have been encode call back function
Parameters:
	Remarks:
		state: valid
*********************************************************************************/

void VIM_MAPI_ISR_Recapture(void)
{
UINT8 qf;
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
	VIM_USER_PrintDec("\n qf is =",VIM_JPEG_GetQFactor());
	VIM_USER_PrintString("\n VIM_MAPI_ISR_Recapture");
	VIM_USER_PrintDec("\n recapture time is=",gVc0528_Info.CaptureStatus.RecaptureTimes);
#endif
	if(gVc0528_Info.CaptureStatus.RecaptureTimes>=VIM_USER_RECAPTUREMAXTIMES){
		//breakpoint();
		VIM_MAPI_ISR_JbufError();
	}
	else
	{
		VIM_JPEG_SetBitRateControlEn(DISABLE);
		qf=VIM_JPEG_GetQFactor();
		if((qf+VIM_USER_QFONESTEP)<127)
			VIM_JPEG_SetBrcQF(qf+VIM_USER_QFONESTEP);
		else
			VIM_JPEG_SetBrcQF(127);
		gVc0528_Info.CaptureStatus.RecaptureTimes++;
	}
}
/********************************************************************************
Description:
	init interrupt of capture function
Parameters:
	Note:
		this funcion should be called in the capture function high api
	Remarks:
		state: valid
*********************************************************************************/

void VIM_MAPI_InitCapInterrupt(void)
{
	//clear interrupt
	VIM_HIF_ClearIntModule(INT_MARB);
	VIM_HIF_ClearIntModule(INT_MARBADD);//angela 2006-12-14
	VIM_HIF_ClearIntModule(INT_JPEG);
	VIM_MARB_RegisterISR(MARB_INT_ALL,NULL);
	VIM_JPEG_RegisterISR(VIM_JPEG_JPEG_INT_ALL,NULL);
	
	// enable  interrupt of jpeg done
	VIM_MARB_RegisterISR(MARB_INT_JPEG_DONE, VIM_MAPI_ISR_JbufDone);
	VIM_MARB_SetIntEnable(MARB_INT_JPEG_DONE,ENABLE);

	//enalbe jbuf error interrrupt
	VIM_MARB_RegisterISR(MARB_INT_MEM0_REF_ERR,VIM_MAPI_ISR_JbufError);
	VIM_MARB_RegisterISR(MARB_INT_MEM1_REF_ERR,VIM_MAPI_ISR_JbufError);
	VIM_MARB_RegisterISR(MARB_INT_MEM2_REF_ERR,VIM_MAPI_ISR_JbufError);
	VIM_MARB_RegisterISR(MARB_INT_RECAP_ERR,VIM_MAPI_ISR_JbufError);
	VIM_MARB_SetIntEnable(MARB_INT_RECAP_ERR,ENABLE);
	VIM_MARB_SetIntEnable(MARB_INT_MEM2_REF_ERR,ENABLE);
	VIM_MARB_SetIntEnable(MARB_INT_MEM1_REF_ERR,ENABLE);
	VIM_MARB_SetIntEnable(MARB_INT_MEM0_REF_ERR,ENABLE);
	

	switch(gVc0528_Info.CaptureStatus.Mode)
	{
	case VIM_MULTISHOT:
		// set marb fifocnt 
		VIM_MARB_RegisterISR(MARB_INT_JBUF_FIFOCNT, VIM_MAPI_ISR_JbufDone);
		VIM_MARB_SetIntEnable(MARB_INT_JBUF_FIFOCNT,ENABLE);

		// set jpeg done 
		VIM_JPEG_RegisterISR(VIM_INT_JPEG_ENC_DONE,VIM_MAPI_ISR_OneFrameDone);
		VIM_JPEG_SetIntEnable(VIM_INT_JPEG_ENC_DONE,ENABLE);
		//printk("INIT ISR: VIM_MULTISHOT\n");
		break;
	case VIM_CAPTUREVIDEO:
	case VIM_CAPTURESTILL:
		//error over flow
		VIM_MARB_RegisterISR(MARB_INT_JBUF_ERR,VIM_MAPI_ISR_JbufError);
		VIM_MARB_SetIntEnable(MARB_INT_JBUF_ERR,ENABLE);
		// set marb fifocnt 
		VIM_MARB_RegisterISR(MARB_INT_JBUF_FIFOCNT, VIM_MAPI_ISR_FifoCont);
		VIM_MARB_SetIntEnable(MARB_INT_JBUF_FIFOCNT,ENABLE);
		// set jpeg done 
		//VIM_JPEG_RegisterISR(VIM_INT_JPEG_ENC_DONE,VIM_MAPI_ISR_OneFrameDone);
		//VIM_JPEG_SetIntEnable(VIM_INT_JPEG_ENC_DONE,ENABLE);
		//printk("INIT ISR: VIM_CAPTUREVIDEO, VIM_CAPTURESTILL\n");
		break;
	case VIM_CAPTURECAPTURE:
		VIM_MARB_RegisterISR(MARB_INT_RECAP,VIM_MAPI_ISR_Recapture);
		VIM_MARB_SetIntEnable(MARB_INT_RECAP,ENABLE);	
		//printk("INIT ISR: VIM_CAPTURECAPTURE\n");
		break;
	default:
		break;
	}
	
}

/********************************************************************************
Description:
	for caculate window
parameters: 
	FrameCount:        need delay frame counter
Return:
     	void
Remarks:
*********************************************************************************/
void VIM_MAPI_Delay_Frame(UINT8 FrameCount)
{
UINT16 iStartFram,iCurrentFrame;
UINT32 i=800*FrameCount;
	if(i==0) i=300;

	iStartFram=VIM_JPEG_GetFrameCount();
	while(i--)
	{
		iCurrentFrame=VIM_JPEG_GetFrameCount();
		if((iCurrentFrame-iStartFram)>FrameCount)
			break;
		VIM_USER_DelayMs(1);
	}
}
/********************************************************************************
Description:
	for display or decode function to start marb and jpeg module
parameters: 
	void
Return:
     	void
Remarks:
*********************************************************************************/
void VIM_MAPI_SetDisplayJpegMode(void)
{
	VIM_SIF_EnableSyncGen(DISABLE);
      	VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER,DISABLE);	
      // VIM_USER_DelayMs(100); 
      	VIM_MAPI_SetChipMode(VIM_MARB_DISPLAYSTILL_MODE,VIM_IPP_HAVE_NOFRAME);
     // 	VIM_USER_DelayMs(100); 
      	VIM_IPP_SetDisplayDropFrame(0xffff);
}
/********************************************************************************
Description:
	for display or decode function to start marb and jpeg module
parameters: 
	void
Return:
     	void
Remarks:
*********************************************************************************/
VIM_RESULT VIM_MAPI_SetDecodeJpegMode(VIM_HAPI_DECODE_MODE DecodeMode)
{
	VIM_RESULT result;
	switch(DecodeMode)
		{
		case VIM_DECODE_WITHFRAME_TO_YUV:
			if(gVc0528_Info.LcdStatus.WorkMode!=VIM_HAPI_LCDMODE_OVERLAY)
				return VIM_ERROR_LCDWORKMODE;
			//VIM_DISP_EnableVrgbMode(DISABLE);
			VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER,ENABLE);
			VIM_HIF_SetModClkClose(VIM_HIF_CLOCK_LCDIF);
			VIM_DISP_SetWorkMode(VIM_DISP_DECODE);	//decode enable	
			VIM_MAPI_SetChipMode(VIM_MARB_DECODEYUV_MODE,VIM_IPP_HAVEFRAME);
			break;
		case VIM_DECODE_RESIZE_TO_YUV:
			result = VIM_DISP_SetWorkMode(VIM_DISP_AFIRST);
			if(result)
				return result;
			//VIM_DISP_EnableVrgbMode(DISABLE);
			//VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER,DISABLE);
			//VIM_DISP_SetWorkMode(VIM_DISP_NORMAL);	//decode disable
			VIM_MAPI_SetChipMode(VIM_MARB_DECODEYUV_MODE,VIM_IPP_HAVE_NOFRAME);
			break;			
		case VIM_DECODE_RESIZE_TO_RGB:
			VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER,ENABLE);
			VIM_HIF_SetModClkClose(VIM_HIF_CLOCK_LCDIF);
			VIM_DISP_SetWorkMode(VIM_DISP_DECODE);	//decode enable
			VIM_MAPI_SetChipMode(VIM_MARB_DECODERGB_MODE,VIM_IPP_HAVEFRAME);
			break;		
		case VIM_DECODE_WITHFRAME_TO_RGB:
			if(gVc0528_Info.LcdStatus.WorkMode!=VIM_HAPI_LCDMODE_OVERLAY)
				return VIM_ERROR_LCDWORKMODE;
			//VIM_DISP_EnableVrgbMode(DISABLE);
			VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER,ENABLE);
			VIM_HIF_SetModClkClose(VIM_HIF_CLOCK_LCDIF);
			VIM_DISP_SetWorkMode(VIM_DISP_DECODE);	//decode enable
			VIM_MAPI_SetChipMode(VIM_MARB_DECODERGB_MODE,VIM_IPP_HAVEFRAME);
			break;
		default: 
			break;
		}
	//VIM_HIF_SetModClkClose(VIM_HIF_CLOCK_SIF | VIM_HIF_CLOCK_ISP);
	//VIM_SIF_EnableSyncGen(ENABLE);
	return VIM_SUCCEED;

}
/********************************************************************************
Description:
	start decode do
parameters: 
	void
Return:
     	void
Remarks:
*********************************************************************************/

void VIM_MAPI_StartDecode(void)
{
	VIM_MARB_ResetSubModule(VIM_MARB_SUB_RESET_IPP);
       VIM_MARB_ResetSubModule(VIM_MARB_SUB_RESET_JPEG);	
	VIM_MARB_StartDecode();
	VIM_JPEG_StartDecode();
}
/********************************************************************************
Description:
	set jpeg information to jpeg module and marb map memory
parameters: 
	void
Return:
	VIM_SUCCEED:set parameter ok and map memory ok
Remarks:
*********************************************************************************/
VIM_RESULT VIM_MAPI_SetDisplayJpegParameter(void)
{
	VIM_RESULT result;

	//set line buffer YUV mode, jpeg module image size, video word counter ect.
	result = VIM_JPEG_SetSize((VIM_JPEG_YUVMODE)gVc0528_JpegInfo.YUVType, gVc0528_JpegInfo.JpgSize);
	if(result)
			return result;

DISPLAYCHAGNEAMODE:
	result =VIM_MARB_SetMap(gVc0528_Info.MarbStatus.WorkMode,gVc0528_Info.MarbStatus.ALayerMode,
		                  (PVIM_MARB_Map)&gVc0528_Info.MarbStatus.MapList);
	if(result)
	{
		if(gVc0528_Info.MarbStatus.ALayerMode == VIM_DISP_LINEBUF)
			return result;
		else
		{
			gVc0528_Info.MarbStatus.ALayerMode =VIM_DISP_LINEBUF;
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
			VIM_USER_PrintString("****should change to lienbuf mode****");
#endif
			goto DISPLAYCHAGNEAMODE;
		}
	}

	//angela added it 2006-8-22 for auto just a mode
	VIM_JPEG_AdjustClkBlank();
	
	//set jpeg each table from jpeg file
       VIM_JPEG_SetJpegInfo(&gVc0528_JpegInfo);
	//ready start do display
	VIM_MAPI_StartDecode();
	return VIM_SUCCEED;
}
/********************************************************************************
Description:
	set capture information to jpeg module and marb map memory
parameters: 
	void
Return:
	VIM_SUCCEED:set parameter ok and map memory ok
Remarks:
*********************************************************************************/

VIM_RESULT VIM_MAPI_SetAutoDisplayJpegParameter(void)
{
	VIM_RESULT result;
	//set line buffer YUV mode, jpeg module image size, video word counter ect.
	result = VIM_JPEG_SetSize(JPEG_422, gVc0528_JpegInfo.JpgSize);
	if(result)
		return result;
	//marb map memory
	result = VIM_MARB_SetMap(gVc0528_Info.MarbStatus.WorkMode,gVc0528_Info.MarbStatus.ALayerMode,
		                  (PVIM_MARB_Map)&gVc0528_Info.MarbStatus.MapList);
	if(result)
		return result;
	//check if have pic in jbuffer and end flag can auto decode
	result = VIM_MAPI_CheckPictureInJBufForAutoDecode();
	if(result)
		return result;
	return VIM_SUCCEED;
}
/********************************************************************************
Description:
	write current frame data to jbuf to decode by jpeg module
parameters: 
	Truelen: the true data length of this frame  
	start	    : the start address for jpeg data write to        
Return:
	void
Remarks:
*********************************************************************************/
void VIM_MAPI_WriteOneFrameData(UINT32 Truelen,HUGE UINT8 *start)
{
 UINT32 marblen, offset=0,Waittimes=0xfff,templen;
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
 UINT8 JpegStatus=0;
#endif

	VIM_MARB_SetJpegbufGapratio(8);
	marblen=gVc0528_Info.MarbStatus.MapList.jbufsize;	
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintHex("\n JbufSize 	 : ", marblen);
		VIM_USER_PrintHex("\n JpegFileSize : ", Truelen);
#endif

	while(Waittimes--)
	{
		if((VIM_MARB_GetStatus()&0x01)&&Truelen)	//have decode more than 9/10 jbuf clean by less than ?
		{
			templen=VIM_MARB_GetJbufRWSize(gVc0528_Info.MarbStatus.Jpgpoint);
			if(templen>=Truelen)
			{
				if(!gVc0528_JpegInfo.frmEnd)	//agnela 2007-2-25
					gVc0528_Info.MarbStatus.Jpgpoint=VIM_MARB_WriteJpegData(start+offset, Truelen,gVc0528_Info.MarbStatus.Jpgpoint,0);
				else
					gVc0528_Info.MarbStatus.Jpgpoint=VIM_MARB_WriteJpegData(start+offset,Truelen,gVc0528_Info.MarbStatus.Jpgpoint,1);
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
				VIM_USER_PrintHex("\n end of write : ", Truelen);
#endif
				Truelen=0;	
			}
			else
			{
				gVc0528_Info.MarbStatus.Jpgpoint=VIM_MARB_WriteJpegData(start+offset, templen,gVc0528_Info.MarbStatus.Jpgpoint,0);
				offset+=templen;
				Truelen-=templen;
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
				//VIM_USER_PrintHex("\n middle of write : ", templen);
#endif
			}
		}
		VIM_USER_DelayMs(5);
		//file is not end then break waite next time call for jpeg display
		if((!gVc0528_JpegInfo.frmEnd)&&(!Truelen))
		  	break;
		//not check LCDC frame end because dislay size is not 16 
		if((gVc0528_JpegInfo.frmEnd)&&(!Truelen))
		{
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
				JpegStatus=VIM_JPEG_GetStatus();
				//VIM_USER_PrintHex("\n Jpegstatus : ", JpegStatus);
				if(JpegStatus&0x2)
			   		break;	
#else
				if(VIM_JPEG_GetStatus()&0x2)
			   		break;	
#endif
		}
	}
}

/********************************************************************************
  Description:
	check if have pic in jbuf for auto decode 
  Parameters  :
	void
  Return:
  	VIM_SUCCEED	: ok, can auto decode
  	VIM_ERROR_PARAMETER:capture file length more than jbuf size
  	VIM_ERROR_NOPIC_INBUF: jbuffer have no pic end flag
  Remarks: Jpeg data must be captured/encoded by v578 chip
*********************************************************************************/
VIM_RESULT VIM_MAPI_CheckPictureInJBufForAutoDecode(void)
{
	UINT8 dat[4];
	
	#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintDec("\n gVc0528_Info.CaptureStatus.CapFileLength =",gVc0528_Info.CaptureStatus.CapFileLength);
		VIM_USER_PrintDec("\n gVc0528_Info.MarbStatus.MapList.jbufsize =",gVc0528_Info.MarbStatus.MapList.jbufsize);
		#endif	
	//check if have pic in jbuffer and if have end flag
	if(gVc0528_Info.CaptureStatus.CapFileLength>=gVc0528_Info.MarbStatus.MapList.jbufsize)
		return VIM_ERROR_PARAMETER;
	else
	{
		VIM_HIF_ReadSram(gVc0528_Info.MarbStatus.MapList.jbufstart+gVc0528_Info.CaptureStatus.CapFileLength-4, dat, 4);
		//if((dat[2]!=0xff)||(dat[3]!=0xd9))
		//	return VIM_ERROR_NOPIC_INBUF;
		VIM_HIF_WriteSram(gVc0528_Info.MarbStatus.MapList.jbufstart+gVc0528_Info.CaptureStatus.CapFileLength-4, dat, 4);
	}
	return VIM_SUCCEED;
}
/********************************************************************************
  Description:
	check user define display size is good or not 
  Parameters  :
	LcdOffsetX,LcdOffsetY:user define start position on LCD
	DisplayWidth,DisplayHeight:user define display size on LCD
	DisplaySize:get the real display size (if a have rotation)
  Return:
  	VIM_SUCCEED	: ok
  	VIM_ERROR_PARAMETER:startposition+displaysize>LCD size
  Remarks: 
*********************************************************************************/
VIM_RESULT VIM_MAPI_CheckDisplaySize(UINT16 LcdOffsetX, UINT16 LcdOffsetY, UINT16 DisplayWidth, UINT16 DisplayHeight, TSize *DisplaySize)
{
	TSize Size,lcddis;
	TPoint pt = {0, 0};
	//check display size>{4,4},must less than (LCD size)
       Size.cx=(DisplayWidth&0xfffe);	
	Size.cy=(DisplayHeight&0xfffe);	
	if((Size.cx<4)||(Size.cy<4))
	   	return VIM_ERROR_PARAMETER;
	//check display if large than lcd size
	pt.x=(LcdOffsetX&0xfffc);
	pt.y=LcdOffsetY;	
	lcddis=gVc0528_Info.LcdStatus.Size;
	if (gVc0528_Info.LcdStatus.ARotationMode%2)
	{
		lcddis.cx=gVc0528_Info.LcdStatus.Size.cy;	
		lcddis.cy=gVc0528_Info.LcdStatus.Size.cx;	
	}
	if(((pt.x+Size.cx)>lcddis.cx)||((pt.y+Size.cy)>lcddis.cy))
	   	return VIM_ERROR_PARAMETER;
	DisplaySize->cx=Size.cx;	
	DisplaySize->cy=Size.cy;	
	gVc0528_Info.DisplayStatus.LcdWantPt=pt;
	gVc0528_Info.DisplayStatus.LcdWantSize=Size;
	return VIM_SUCCEED;
}
/********************************************************************************
  Description:
	check display size if overflow by downsize
  Parameters  :
	bYUVmode:jpeg YUV mode
	SourceSize:jpeg source size
	DisplaySize:decode target size
  Return:
  	VIM_SUCCEED	: ok
  	VIM_ERROR_DECODE_DOWNSIZEOVERFLOW:420 DisplaySize.cy/SourceSize.cy<1/16
  	                                                                   422 DisplaySize.cy/SourceSize.cy<1/8
  	                                                                   444 DisplaySize.cy/SourceSize.cy<1/8
  	                                                                   410 DisplaySize.cy/SourceSize.cy<1/8
  	                                                                   400 DisplaySize.cy/SourceSize.cy<1/8
  Remarks: 
        this function add in 2006/2/24, because 578 hardware hace bug so need add this check
  Modify History:
	Version			Modifyby				DataTime					modified
	   0.1			Amanda Deng			2006.02.24				create first version

*********************************************************************************/
VIM_RESULT VIM_MAPI_CheckDecodeTargetSize(VIM_JPEG_YUVMODE bYUVmode,TSize SourceSize, TSize DisplaySize)
{
	if(bYUVmode==JPEG_420)
	{
		if (DisplaySize.cy*16<SourceSize.cy)
			return VIM_ERROR_DECODE_DOWNSIZEOVERFLOW;
	}
	else
	{
		if (DisplaySize.cy*8<SourceSize.cy)
			return VIM_ERROR_DECODE_DOWNSIZEOVERFLOW;
	}
	return VIM_SUCCEED;
}
/********************************************************************************
   Description:
	check user define display size is good or not 
  Parameters  :
	JpegOffsetX,JpegOffsetY:user define start position on jpeg imagesize
	DisplaySize:user define display size on LCD
	DestDisplaySize:get the real display size (if displaysize+startposition>jpeg imagesize)
  Return:
  	VIM_SUCCEED	: ok
  	VIM_ERROR_PARAMETER:startposition>jpeg imagesize
  Remarks: 
*********************************************************************************/
VIM_RESULT VIM_MAPI_CheckDisplayPanSize(UINT16 JpegOffsetX, UINT16 JpegOffsetY, TSize DisplaySize,TSize *DestDisplaySize,TPoint* pt )
{
	TSize SourceWin;

	//get source window size and source start position
	SourceWin.cx=DestDisplaySize->cx=DisplaySize.cx;
	SourceWin.cy=DestDisplaySize->cy=DisplaySize.cy;	
	pt->x=JpegOffsetX&0xfffe;
	pt->y=JpegOffsetY&0xfffe;
	if((pt->x>=gVc0528_JpegInfo.JpgSize.cx)||(pt->y>=gVc0528_JpegInfo.JpgSize.cy))
	{
		pt->x=pt->y=0;
		return VIM_ERROR_PARAMETER;
	}
	if((pt->x+SourceWin.cx)>gVc0528_JpegInfo.JpgSize.cx)
	{
		DestDisplaySize->cx=gVc0528_JpegInfo.JpgSize.cx-pt->x;
	      //return VIM_ERROR_PARAMETER;
	}
	if((pt->y+SourceWin.cy)>gVc0528_JpegInfo.JpgSize.cy)
	{
		DestDisplaySize->cy=gVc0528_JpegInfo.JpgSize.cy-pt->y;
	      //return VIM_ERROR_PARAMETER;
	}
	return VIM_SUCCEED;
}
/********************************************************************************
  Description:
	check user define display size is good or not 
  Parameters  :
  	mPt:a layer memory start position
  	MemorySize:a layer memory size
  	DisplaySize:user define display size on LCD
	LcdOffsetX,LcdOffsetY:user define start position on LCD
  Return:
  	VIM_SUCCEED	: ok
  	VIM_ERROR_????????: return from disp module
  Remarks: 
*********************************************************************************/
VIM_RESULT VIM_MAPI_SetALayerDisplaySize(TPoint mPt,TSize MemorySize, TSize DisplaySize, UINT16 LcdOffsetX, UINT16 LcdOffsetY)
{
	VIM_RESULT result;
	TPoint pt = {0, 0};
	TSize lcddis,winsize;

	#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintDec("\n layer a memory x start : ", mPt.x);
		VIM_USER_PrintDec("\n layer a memory y start : ", mPt.y);
		VIM_USER_PrintDec("\n layer a memory width : ",  MemorySize.cx);
		VIM_USER_PrintDec("\n layer a memory height : ", MemorySize.cy);
	#endif
       //get display size
	result = VIM_DISP_SetA_Memory(mPt,MemorySize);
	if(result)
		return result;
	//set A layer window size and start position in memory panel (window size = display size)
	pt.x=LcdOffsetX;
	pt.y=LcdOffsetY;	
	//because user define display size is after rotation
	winsize=MemorySize;
	lcddis=DisplaySize;
	if (gVc0528_Info.LcdStatus.ARotationMode%2)
	{
      		winsize.cx=MemorySize.cy;
		winsize.cy=MemorySize.cx;
		lcddis.cx=DisplaySize.cy;
		lcddis.cy=DisplaySize.cx;
	}
	if(winsize.cx<DisplaySize.cx)
	{
		pt.x=(lcddis.cx-winsize.cx)>>1;
		pt.x+=LcdOffsetX;
	}
	if(winsize.cy<lcddis.cy)
	{
		pt.y=(lcddis.cy-winsize.cy)>>1;
		pt.y+=LcdOffsetY;
	}	
	pt.x&=0xfffc;
	pt.y&=0xfffc;
	#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintDec("\n layer a on lcd x start : ", pt.x);
		VIM_USER_PrintDec("\n layer a on lcd y start : ", pt.y);
		VIM_USER_PrintDec("\n layer a window width : ",  winsize.cx);
		VIM_USER_PrintDec("\n layer a window height : ", winsize.cy);
	#endif
	result = VIM_DISP_SetA_DisplayPanel(pt,winsize);
	if(result)
		return result;
	return VIM_SUCCEED;
}

/********************************************************************************
  Description:
	get jpeg information 
  Parameters  :
	jpegBuf:	jpeg file data start address
	length: jpeg file length
  Return:
  	VIM_SUCCEED	: ok, can auto decode
  	VIM_ERROR_PARAMETER:capture file length more than jbuf size
  	VIM_ERROR_YUV_UNKNOW: jpeg file yuv format error
  	VIM_ERROR_NOPIC_INBUF: jbuffer have no pic end flag
  Remarks: Jpeg data must be captured/encoded by v578 chip
*********************************************************************************/
VIM_RESULT VIM_MAPI_GetJpegInformation(HUGE UINT8 *jpegBuf,UINT32 length)
{
	VIM_RESULT result;
	if(jpegBuf)	
	{
		VIM_JPEG_ToolJpegParse(jpegBuf, length, &gVc0528_JpegInfo,1);
		if((VIM_JPEG_YUVMODE)gVc0528_JpegInfo.YUVType == JPEG_UNKNOWN) 
			return VIM_ERROR_YUV_UNKNOW;
		if(gVc0528_JpegInfo.frmEnd ==TRUE)
			gVc0528_Info.DisplayStatus.FileLength+=gVc0528_JpegInfo.eop;
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintDec("\n gVc0528_Info.DisplayStatus.FileLength=",gVc0528_Info.DisplayStatus.FileLength);
#endif
	}
	else
	{
		gVc0528_Info.DisplayStatus.BufPoint=0;
		gVc0528_JpegInfo.ImageSize=gVc0528_Info.CaptureStatus.Size;
		if(gVc0528_Info.CaptureStatus.ReCapSize)
		{
			gVc0528_JpegInfo.ImageSize.cx=gVc0528_Info.CaptureStatus.Size.cx<<1;
			gVc0528_JpegInfo.ImageSize.cy=gVc0528_Info.CaptureStatus.Size.cy<<1;
		}
		VIM_JPEG_GetJpegSize(JPEG_422, gVc0528_JpegInfo.ImageSize, &(gVc0528_JpegInfo.JpgSize));
		//check if have epic in jbuffer and end flag can auto decode
		result = VIM_MAPI_CheckPictureInJBufForAutoDecode();
		if(result)
			return result;
	}
	return VIM_SUCCEED;
}
/********************************************************************************
  Description:
	Decode jpeg ,  data is in sram jbuf. 
  Parameters  :
	Datasize  : size of jpeg data in sram.
  Return:
  Remarks: Jpeg data must be captured/encoded by v578 chip
*********************************************************************************/
void VIM_MAPI_JpegAutoDecode(UINT32 Datasize)
{
	UINT8 dat[4];
	VIM_JPEG_ResetState();
	VIM_IPP_Reset();
	VIM_JPEG_SetDefaultHT();
       VIM_JPEG_SetUseCustomQTEnable(DISABLE);
	VIM_JPEG_SetUseSameQTEnable(DISABLE);
	VIM_JPEG_FeedHeaderEnable(DISABLE);
	VIM_JPEG_ResetState();
	VIM_MAPI_StartDecode();
	VIM_HIF_ReadSram(VIM_MARB_GetJbufStartAddr()+Datasize-4, dat, 4);
	VIM_HIF_WriteSram(VIM_MARB_GetJbufStartAddr()+Datasize-4, dat, 4);
}
/********************************************************************************
Description:
	init interrupt of decode jpeg function
Parameters:
	Note:
Remarks:
		state: valid
*********************************************************************************/
void VIM_MAPI_InitDecodeInterrupt(void)
{
	//clear interrupt
	VIM_HIF_ClearIntModule(INT_LBUF);
	VIM_HIF_ClearIntModule(INT_JPEG);
	VIM_JPEG_RegisterISR(VIM_JPEG_JPEG_INT_ALL,NULL);
	// set jpeg done 
	VIM_JPEG_RegisterISR(VIM_INT_JPEG_LINBUF_READ,VIM_MAPI_ISR_LBufRead);
	VIM_JPEG_SetIntEnable(VIM_INT_JPEG_LINBUF_READ,ENABLE);
}
/********************************************************************************
Description:
	line buffer 1 data read call back function
Parameters:
Remarks:
state: valid
*********************************************************************************/
void VIM_MAPI_ISR_LBufRead(void)
{
	UINT32 vwc;
	TSize newsize,displaysize;
	static UINT8 times=0;
	VIM_DISP_GetAWinsize(&displaysize);
	newsize.cx=displaysize.cx;
	newsize.cy=VIM_HIF_GetReg8(V5_REG_LBUF_LBUF_STATUS);
	newsize.cy>>= 2;
	newsize.cy&=0x1f;
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintDec("\n ** new  decode line counter : ", newsize.cy);
#endif
	if (!newsize.cy) 
	{
		times++;
		if(VIM_DISP_GetDecodeSpecial()&&(times>5))
		{
	#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
			VIM_USER_PrintDec("\n ** VIM_JPEG_RestartDecode ", times);
	#endif
			VIM_JPEG_RestartDecode();
		}
		return;	 

	}
	gVc0528_Info.DisplayStatus.DecodeHeight+=newsize.cy;
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintDec("DecodeHeight height:",gVc0528_Info.DisplayStatus.DecodeHeight); 
		VIM_USER_PrintDec("DecLength :",gVc0528_Info.DisplayStatus.DecLength);
#endif
	vwc = VIM_JPEG_GetYuvSize(JPEG_422, newsize);

	if(gVc0528_Info.DisplayStatus.DecLength<vwc)	
	{
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintHex("\n decode buffer length : ", gVc0528_Info.DisplayStatus.BufLength);
		VIM_USER_PrintHex("\n decode data length   : ", vwc);
		VIM_USER_PrintString("\n decode buf error!");
#endif
		gVc0528_Info.DisplayStatus.ReadOk=VIM_DECODE_BUFERROR;
		return;
	}
	VIM_MARB_ReadBuf(gVc0528_Info.DisplayStatus.DecPoint, vwc, gVc0528_Info.MarbStatus.MapList.lbuf1start, 
		gVc0528_Info.MarbStatus.MapList.lbufend, gVc0528_Info.MarbStatus.MapList.lbuf1start);

	gVc0528_Info.DisplayStatus.DecPoint+=vwc;
	gVc0528_Info.DisplayStatus.DecLength-=vwc;
	
	if((gVc0528_JpegInfo.frmEnd)&&(gVc0528_Info.DisplayStatus.DecodeHeight >= displaysize.cy))
		gVc0528_Info.DisplayStatus.ReadOk=VIM_DECODE_DONE;
	
	else
		VIM_JPEG_RestartDecode();
	
}
/********************************************************************************
Description:
	write current frame data to jbuf to decode by jpeg module
parameters: 
	Truelen: the true data length fro decode
Return:
	VIM_SUCCEED:
	VIM_ERROR_TIME_OUT:
	VIM_ERROR_BUFFERLENGTHS:
Remarks:
*********************************************************************************/
VIM_RESULT VIM_MAPI_DecodeOneFrame(UINT32 Truelen,HUGE UINT8 *start)
{
	UINT32 marblen, offset=0;
	UINT32 Waittimes=0x3ffffff,Waittimes1=0x3ffff;


	marblen=gVc0528_Info.MarbStatus.MapList.jbufsize;	
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintHex("\n JbufSize 	 : ", marblen);
		VIM_USER_PrintHex("\n JpegFileSize : ", Truelen);
		VIM_USER_PrintDec("\n current file length 	 : ", gVc0528_Info.DisplayStatus.FileLength);
#endif

	while(Waittimes--)
	{
		gVc0528_Info.DisplayStatus.ReadOk=VIM_DECODE_ING;
		if(VIM_MARB_GetJbufRWSize(gVc0528_Info.MarbStatus.Jpgpoint)>Truelen)
		{
			if(gVc0528_JpegInfo.frmEnd)
			{
				gVc0528_Info.MarbStatus.Jpgpoint=VIM_MARB_WriteJpegData(start+offset,Truelen,gVc0528_Info.MarbStatus.Jpgpoint,1);
				Truelen=0;	
			}
			else
			{

				gVc0528_Info.MarbStatus.Jpgpoint=VIM_MARB_WriteJpegData(start+offset,Truelen,gVc0528_Info.MarbStatus.Jpgpoint,0);
				offset+=Truelen;
				Truelen-=Truelen;
			}
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
			VIM_USER_PrintHex("\n end of write : ", Truelen);
#endif
		}
		else
		{
			gVc0528_Info.MarbStatus.Jpgpoint=VIM_MARB_WriteJpegData(start+offset,marblen,gVc0528_Info.MarbStatus.Jpgpoint,0);
			offset+=marblen;
			Truelen-=marblen;
		}

		while((Waittimes1--)&&(gVc0528_Info.DisplayStatus.ReadOk==VIM_DECODE_ING))
		{
			VIM_USER_DelayMs(5); 
			VIM_MAPI_ISR_LBufRead();
			if((VIM_MARB_GetJbufRWSize(gVc0528_Info.MarbStatus.Jpgpoint)==marblen)&&(Truelen))
				gVc0528_Info.DisplayStatus.ReadOk=VIM_DECODE_SHOULDREWRITE;

		}
		if((Waittimes1==0xffffffff)&&(!Truelen))
			return VIM_ERROR_TIME_OUT;
		Waittimes1=0x3fff;
		if(gVc0528_Info.DisplayStatus.ReadOk==VIM_DECODE_DONE)
			break;
		if(gVc0528_Info.DisplayStatus.ReadOk==VIM_DECODE_BUFERROR)
			return VIM_ERROR_BUFFERLENGTHS;
		VIM_USER_DelayMs(1);

		if((!gVc0528_JpegInfo.frmEnd)&&(!Truelen))
		  	break;

	}
	return VIM_SUCCEED;
}
//end writern by Amanda Deng



/********************************************************************************
Description:
	set zoom (can capture)
Parameters:
	UINT8 step:
		the step 

Remarks:
	state: 	valid
*********************************************************************************/
VIM_RESULT VIM_MAPI_Preview_CaptureZoom(UINT8 bstep)
{
	UINT32 dwZoomW,dwZoomH;
	TSize CaptureSize,ZoomSourcSize = gVc0528_Info.PreviewStatus.ZoomPara.InitSourceSize,ZoomBaseSize,InputSize;
	TPoint pt;
	VIM_RESULT result;


	CaptureSize=gVc0528_Info.CaptureStatus.Size;
	ZoomBaseSize=ZoomSourcSize;
//change capture size
	VIM_SIF_GetSensorResolution(gVc0528_Info.pSensorInfo, (VIM_SIF_RESOLUTION)gVc0528_Info.PreviewStatus.bOutputIndex, &InputSize);
	if((CaptureSize.cx>InputSize.cx)||(CaptureSize.cy>InputSize.cy))
	{
		if(VIM_USER_FAST_PREVIEW)
		{
			CaptureSize.cx/=2;
			CaptureSize.cy/=2;
		}
	}
	if(bstep)
	{
		if((CaptureSize.cx/2) > gVc0528_Info.PreviewStatus.ZoomPara.InitAMemSize.cx)
		{
			ZoomBaseSize.cx = CaptureSize.cx/2;
			ZoomBaseSize.cy = CaptureSize.cy/2;
		}
		else
			ZoomBaseSize = gVc0528_Info.PreviewStatus.ZoomPara.InitAMemSize;
#if (VIM_USER_ZOOM_MIN_WIDHT)
			{
				if (ZoomBaseSize.cx < VIM_USER_ZOOM_MIN_WIDHT)
				{
					ZoomBaseSize.cy = VIM_USER_ZOOM_MIN_WIDHT *  ZoomBaseSize.cy / ZoomBaseSize.cx;
					ZoomBaseSize.cx = VIM_USER_ZOOM_MIN_WIDHT;
				}
			}
#endif
		if(((ZoomSourcSize.cx/ZoomSourcSize.cy)>(ZoomBaseSize.cx/ZoomBaseSize.cy))||(((ZoomSourcSize.cx/ZoomSourcSize.cy)
			==(ZoomBaseSize.cx/ZoomBaseSize.cy))&&((ZoomSourcSize.cx%ZoomSourcSize.cy)>=(ZoomBaseSize.cx%ZoomBaseSize.cy))))
		{
			dwZoomW = (bstep * (ZoomSourcSize.cx -ZoomBaseSize.cx))  / VIM_USER_MAX_STEP;
			dwZoomH = (dwZoomW * ZoomSourcSize.cy)  / ZoomSourcSize.cx;

		}
		else if(((ZoomSourcSize.cx/ZoomSourcSize.cy)<(ZoomBaseSize.cx/ZoomBaseSize.cy))||(((ZoomSourcSize.cx/ZoomSourcSize.cy)
			==(ZoomBaseSize.cx/ZoomBaseSize.cy))&&((ZoomSourcSize.cx%ZoomSourcSize.cy)<(ZoomBaseSize.cx%ZoomBaseSize.cy))))
		{
			dwZoomH = (bstep * (ZoomSourcSize.cy-ZoomBaseSize.cy))  / VIM_USER_MAX_STEP;
			dwZoomW = (dwZoomH * ZoomSourcSize.cx)  / ZoomSourcSize.cy;
		}
		else
			return VIM_ERROR_PREVIEW_BASE_ZOOM_PARA;
		ZoomSourcSize.cx -= dwZoomW;
		ZoomSourcSize.cy -= dwZoomH;
		ZoomSourcSize.cx &= 0xfffe;
		ZoomSourcSize.cy &= 0xfffe;
		
		if ((ZoomSourcSize.cx < CaptureSize.cx/2)||(ZoomSourcSize.cx < gVc0528_Info.PreviewStatus.ZoomPara.InitAMemSize.cx)
			||((ZoomSourcSize.cy < CaptureSize.cy/2)
			   ||(ZoomSourcSize.cy < gVc0528_Info.PreviewStatus.ZoomPara.InitAMemSize.cy)))
		{
			ZoomSourcSize.cx=ZoomBaseSize.cx;
		}

		
		result = VIM_IPP_ToolCaculateLessSrcWindow(ZoomSourcSize,CaptureSize,&ZoomBaseSize);
		if(result)
			return result;
		if((ZoomBaseSize.cx<gVc0528_Info.PreviewStatus.ZoomPara.InitAMemSize.cx)||
			(ZoomBaseSize.cy<gVc0528_Info.PreviewStatus.ZoomPara.InitAMemSize.cy))
		{
			ZoomBaseSize=gVc0528_Info.PreviewStatus.ZoomPara.InitAMemSize;
		}
	}
	VIM_SIF_EnableSyncGen(DISABLE);
	
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
	VIM_USER_PrintDec("\n zoom capture ,the source x is",ZoomBaseSize.cx);
	VIM_USER_PrintDec("\n zoom capture ,the source y is",ZoomBaseSize.cy);
#endif
	VIM_IPP_GetImageSize(&InputSize.cx,&InputSize.cy);
	pt.x=((InputSize.cx-ZoomBaseSize.cx)/2)&0xfffe;
	pt.y=((InputSize.cy-ZoomBaseSize.cy)/2)&0xfffe;
	

	result=VIM_IPP_SetCaptureSize(gVc0528_Info.pSensorInfo,pt, ZoomBaseSize, CaptureSize);
	if(result)
		return result;

		
	result=VIM_IPP_SetDispalySize(pt,ZoomBaseSize, gVc0528_Info.PreviewStatus.ZoomPara.InitAMemSize);
	if(result)
		return result;
	
	VIM_SIF_EnableSyncGen(ENABLE);

	return VIM_SUCCEED;

}

/********************************************************************************
Description:
	set zoom (can not capture)
Parameters:
	UINT8 step:
		the step 

Remarks:
	state: 	valid
*********************************************************************************/

VIM_RESULT VIM_MAPI_Preview_DisplayZoom(UINT8 bstep)
{
#if VIM_USER_SURPPORT_DISPLAYZOOM
UINT32 dwZoomW,dwZoomH;
TSize ZoomSourcSize = gVc0528_Info.PreviewStatus.ZoomPara.InitSourceSize,ZoomBaseSize;
TSize AmSize=gVc0528_Info.PreviewStatus.ZoomPara.InitAMemSize;
TPoint pt;
VIM_RESULT result;

	if(bstep)
	{
#if (VIM_USER_MAX_AMEM_WIDTH)
		{
			if ((ZoomSourcSize.cx > VIM_USER_MAX_AMEM_WIDTH)&&
				(gVc0528_Info.PreviewStatus.ZoomPara.InitAMemSize.cx<VIM_USER_MAX_AMEM_WIDTH)) 
			{
				ZoomBaseSize.cx = VIM_USER_MAX_AMEM_WIDTH;
				ZoomBaseSize.cy = ZoomBaseSize.cx * ZoomSourcSize.cy / ZoomSourcSize.cx;
			}
			else
				ZoomBaseSize=ZoomSourcSize;
		}
#else
		{
			if (ZoomSourcSize.cx > AM_MAXPIXEL) 
			{
				ZoomBaseSize.cx = AM_MAXPIXEL;
				ZoomBaseSize.cy = ZoomBaseSize.cx * ZoomSourcSize.cy / ZoomSourcSize.cx;
			}
			else
				ZoomBaseSize=ZoomSourcSize;
		}
#endif
		dwZoomW = (bstep * (ZoomBaseSize.cx - gVc0528_Info.PreviewStatus.ZoomPara.InitAMemSize.cx))  / VIM_USER_MAX_STEP;
		dwZoomH = (dwZoomW * ZoomSourcSize.cy)  / ZoomSourcSize.cx;

		AmSize.cx += dwZoomW;
		AmSize.cy += dwZoomH;
		AmSize.cx &= 0xfff0;
		AmSize.cy &= 0xfff0;
		if ((ZoomSourcSize.cx < AmSize.cx) ||(ZoomSourcSize.cy < AmSize.cy)
		    || (AmSize.cx > AM_MAXPIXEL) ||(AmSize.cy > AM_MAXPIXEL))
		{
			return VIM_ERROR_PREVIEW_DISPLAY_ZOOM_PARA;
		}
	}

	VIM_SIF_EnableSyncGen(DISABLE);
	VIM_USER_DelayMs(10);
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
	VIM_USER_PrintDec(" zoom display  ,the AmSize x is",AmSize.cx);
	VIM_USER_PrintDec(" zoom display ,the AmSize y is",AmSize.cy);
#endif
	VIM_IPP_GetWindowPos(&pt.x,&pt.y);
	result=VIM_IPP_SetDispalySize(pt,ZoomSourcSize, AmSize);
	if(result)
		return result;
	pt.x=(((AmSize.cx-gVc0528_Info.PreviewStatus.Size.cx)>>1)>>1)<<1;
	pt.y=(AmSize.cy-gVc0528_Info.PreviewStatus.Size.cy)>>1;
	VIM_DISP_SetA_Memory(pt,AmSize);
	VIM_SIF_EnableSyncGen(ENABLE);
#endif
	return VIM_SUCCEED;
}

/********************************************************************************
Description:
	adjust point because of rotaion of lcd 
Parameters:
	UINT8 step:
		the step 

Remarks:
	state: 	valid
*********************************************************************************/

VIM_RESULT VIM_MAPI_GetNewPoint(VIM_DISP_ROTATEMODE byRotateMode,PTPoint OldPoint,TSize OldSize,PTPoint NewPoint)
{
	switch(byRotateMode)
	{
		case VIM_DISP_ROTATE_90:
		case VIM_DISP_MIRROR_90:
			NewPoint->x=OldPoint->y;
			NewPoint->y=gVc0528_Info.LcdStatus.Size.cy-OldPoint->x-OldSize.cx;
			if(NewPoint->y>gVc0528_Info.LcdStatus.Size.cy)
				return VIM_ERROR_ADJUST_OFFSET;
			break;
		case VIM_DISP_ROTATE_180:
		case VIM_DISP_MIRROR_180:
			NewPoint->x=gVc0528_Info.LcdStatus.Size.cx-OldPoint->x-OldSize.cx;
			NewPoint->y=gVc0528_Info.LcdStatus.Size.cy-OldPoint->y-OldSize.cy;
			if(NewPoint->x>gVc0528_Info.LcdStatus.Size.cx)
				return VIM_ERROR_ADJUST_OFFSET;
			if(NewPoint->y>gVc0528_Info.LcdStatus.Size.cy)
				return VIM_ERROR_ADJUST_OFFSET;
			break;
		case VIM_DISP_ROTATE_270:
		case VIM_DISP_MIRROR_270:
			NewPoint->y=OldPoint->x;
			NewPoint->x=gVc0528_Info.LcdStatus.Size.cx-OldPoint->y-OldSize.cy;
			break;
		default:
			NewPoint->x=OldPoint->x;
			NewPoint->y=OldPoint->y;
			break;

	}
	return VIM_SUCCEED;
}


/********************************************************************************
Description:
	adjust point because of rotaion of lcd 
Parameters:
	UINT8 step:
		the step 

Remarks:
	state: 	valid
*********************************************************************************/

VIM_RESULT VIM_MAPI_AdjustPoint(VIM_DISP_ROTATEMODE byARotateMode,VIM_DISP_ROTATEMODE byBRotateMode)
{
TPoint	Point,Point1;
TSize Size,Size1;
	if(byARotateMode > VIM_DISP_NOTCHANGE || byBRotateMode > VIM_DISP_NOTCHANGE)
		return VIM_ERROR_DISP_PARAM;

	if(VIM_DISP_NOTCHANGE!=byARotateMode)
		{
			Point.x=VIM_HIF_GetReg16(V5_REG_LCDC_AX_L);
			Point.y=VIM_HIF_GetReg16(V5_REG_LCDC_AY_L);
			VIM_DISP_GetAWinsize(&Size);
			switch(byARotateMode)
			{
				case VIM_DISP_ROTATE_90:
				case VIM_DISP_MIRROR_90:
					VIM_HIF_SetReg16(V5_REG_LCDC_AX_L,Point.y);
					Point.y=gVc0528_Info.LcdStatus.Size.cy-Point.x-Size.cx;
					if(Point.y>gVc0528_Info.LcdStatus.Size.cy)
						return VIM_ERROR_ADJUST_OFFSET;
					VIM_HIF_SetReg16(V5_REG_LCDC_AY_L,Point.y);
					break;
				case VIM_DISP_ROTATE_180:
				case VIM_DISP_MIRROR_180:
					Point.x=gVc0528_Info.LcdStatus.Size.cx-Point.x-Size.cx;
					Point.y=gVc0528_Info.LcdStatus.Size.cy-Point.y-Size.cy;
					if(Point.x>gVc0528_Info.LcdStatus.Size.cx)
						return VIM_ERROR_ADJUST_OFFSET;
					if(Point.y>gVc0528_Info.LcdStatus.Size.cy)
						return VIM_ERROR_ADJUST_OFFSET;
					VIM_HIF_SetReg16(V5_REG_LCDC_AX_L,Point.x);
					VIM_HIF_SetReg16(V5_REG_LCDC_AY_L,Point.y);
					break;
				case VIM_DISP_ROTATE_270:
				case VIM_DISP_MIRROR_270:
					VIM_HIF_SetReg16(V5_REG_LCDC_AY_L,Point.x);
					Point.x=gVc0528_Info.LcdStatus.Size.cx-Point.y-Size.cy;
					if(Point.x>gVc0528_Info.LcdStatus.Size.cx)
						return VIM_ERROR_ADJUST_OFFSET;
					VIM_HIF_SetReg16(V5_REG_LCDC_AX_L,Point.x);
					break;
				default:
					if(gVc0528_Info.PreviewStatus.Size.cx+gVc0528_Info.PreviewStatus.Point.x>gVc0528_Info.LcdStatus.Size.cx)
						return VIM_ERROR_PREVIEW_PARA;
					if(gVc0528_Info.PreviewStatus.Size.cy+gVc0528_Info.PreviewStatus.Point.y>gVc0528_Info.LcdStatus.Size.cy)
						return VIM_ERROR_PREVIEW_PARA;
					break;
			}
		}
	Point.x=VIM_HIF_GetReg16(V5_REG_LCDC_AX_L)&0xfffc;  //angela 2006-8-10
	Point.y=VIM_HIF_GetReg16(V5_REG_LCDC_AY_L)&0xfffc;
	VIM_HIF_SetReg16(V5_REG_LCDC_AX_L,Point.x);
	VIM_HIF_SetReg16(V5_REG_LCDC_AY_L,Point.y);
	if(VIM_DISP_NOTCHANGE==byBRotateMode)
		return VIM_SUCCEED;
	Point.x=VIM_HIF_GetReg16(V5_REG_LCDC_BX0_L);
	Point.y=VIM_HIF_GetReg16(V5_REG_LCDC_BY0_L);
	VIM_DISP_GetBSize(VIM_DISP_B0LAYER,&Size);

	
	Point1.x=VIM_HIF_GetReg16(V5_REG_LCDC_BX1_L);
	Point1.y=VIM_HIF_GetReg16(V5_REG_LCDC_BY1_L);
	VIM_DISP_GetBSize(VIM_DISP_B1LAYER,&Size1);


	switch(byBRotateMode)
	{
		case VIM_DISP_ROTATE_90:
		case VIM_DISP_MIRROR_90:
			VIM_HIF_SetReg16(V5_REG_LCDC_BX0_L,Point.y);
			VIM_HIF_SetReg16(V5_REG_LCDC_BX1_L,Point1.y);
			Point.y=gVc0528_Info.LcdStatus.Size.cy-Point.x-Size.cx;
			Point1.y=gVc0528_Info.LcdStatus.Size.cy-Point1.x-Size1.cx;
			if(Point.y>gVc0528_Info.LcdStatus.Size.cy)
				return VIM_ERROR_ADJUST_OFFSET;
			if(Point1.y>gVc0528_Info.LcdStatus.Size.cy)
				return VIM_ERROR_ADJUST_OFFSET;
			VIM_HIF_SetReg16(V5_REG_LCDC_BY0_L,Point.y);
			VIM_HIF_SetReg16(V5_REG_LCDC_BY1_L,Point1.y);
			break;
		case VIM_DISP_ROTATE_180:
		case VIM_DISP_MIRROR_180:
			Point.x=gVc0528_Info.LcdStatus.Size.cx-Point.x-Size.cx;
			Point1.x=gVc0528_Info.LcdStatus.Size.cx-Point1.x-Size1.cx;
			Point.y=gVc0528_Info.LcdStatus.Size.cy-Point.y-Size.cy;
			Point1.y=gVc0528_Info.LcdStatus.Size.cy-Point1.y-Size1.cy;
			if(Point.x>gVc0528_Info.LcdStatus.Size.cx)
				return VIM_ERROR_ADJUST_OFFSET;
			if(Point1.x>gVc0528_Info.LcdStatus.Size.cx)
				return VIM_ERROR_ADJUST_OFFSET;
			if(Point.y>gVc0528_Info.LcdStatus.Size.cy)
				return VIM_ERROR_ADJUST_OFFSET;
			if(Point1.y>gVc0528_Info.LcdStatus.Size.cy)
				return VIM_ERROR_ADJUST_OFFSET;
			VIM_HIF_SetReg16(V5_REG_LCDC_BX0_L,Point.x);
			VIM_HIF_SetReg16(V5_REG_LCDC_BX1_L,Point1.x);
			VIM_HIF_SetReg16(V5_REG_LCDC_BY0_L,Point.y);
			VIM_HIF_SetReg16(V5_REG_LCDC_BY1_L,Point1.y);
			break;
		case VIM_DISP_ROTATE_270:
		case VIM_DISP_MIRROR_270:
			VIM_HIF_SetReg16(V5_REG_LCDC_BY0_L,Point.x);
			VIM_HIF_SetReg16(V5_REG_LCDC_BY1_L,Point1.x);
			Point.x=gVc0528_Info.LcdStatus.Size.cx-Point.y-Size.cy;
			Point1.x=gVc0528_Info.LcdStatus.Size.cx-Point1.y-Size1.cy;
			if(Point.x>gVc0528_Info.LcdStatus.Size.cx)
				return VIM_ERROR_ADJUST_OFFSET;
			if(Point1.x>gVc0528_Info.LcdStatus.Size.cx)
				return VIM_ERROR_ADJUST_OFFSET;
			VIM_HIF_SetReg16(V5_REG_LCDC_BX0_L,Point.x);
			VIM_HIF_SetReg16(V5_REG_LCDC_BX1_L,Point1.x);
			break;
		default:
			break;
	}
	return VIM_SUCCEED;
}


/********************************************************************************
Description:
	disable or enable syncgen ,and delay one frame.
Parameters:

Remarks:
	state: 	valid
*********************************************************************************/

void VIM_MAPI_EnableSyncGen(UINT8 bEnable)
{
UINT32 i=400;
UINT16 iCurrentFrame,iStartFram;
	iStartFram=VIM_JPEG_GetFrameCount();
	VIM_SIF_EnableSyncGen(bEnable);
	if(bEnable==DISABLE)
	{
		while(i--)
		{
			iCurrentFrame=VIM_JPEG_GetFrameCount();
			if((iCurrentFrame-iStartFram)>1)
				break;
			VIM_USER_DelayMs(1);
		}
	}
}

/********************************************************************************
Description:
	change the input size of preview
Parameters:
	void

Remarks:
	state: 	valid
*********************************************************************************/

VIM_RESULT VIM_MAPI_FastPreview(void)
{

      VIM_RESULT result;
	TSize InputSize,CaptureSize,SourceWin;
	TPoint pt;

	UINT8 array[VIM_USER_MAX_ARRAY_AE], *RegBuf=array;			//12/28/007  
	if( VIM_USER_FAST_PREVIEW)
	{
			result = VIM_SIF_GetSensorResolution(gVc0528_Info.pSensorInfo, (VIM_SIF_RESOLUTION)gVc0528_Info.PreviewStatus.bOutputIndex, &InputSize);
			if((gVc0528_Info.CaptureStatus.Size.cx>InputSize.cx)||(gVc0528_Info.CaptureStatus.Size.cy>InputSize.cy))
			{
				
				 if(gVc0528_Info.PreviewStatus.Mode==VIM_HAPI_PREVIEW_FRAMEON)
					return VIM_ERROR_FASET_PREVIEW;
				
				VIM_IPP_GetWindowSize(&SourceWin.cx,&SourceWin.cy);
				VIM_SIF_EnableSyncGen(DISABLE);

							
				VIM_IPP_SetImageSize(InputSize.cx, InputSize.cy);
				
				CaptureSize=gVc0528_Info.CaptureStatus.Size;
				
				pt.x=((InputSize.cx-SourceWin.cx)/2)&0xfffe;
				pt.y=((InputSize.cy-SourceWin.cy)/2)&0xfffe ;
		#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
			VIM_USER_PrintDec(" fast preview ipp resize ,the SourceWin x is",SourceWin.cx);
			VIM_USER_PrintDec(" fast preview ipp resize ,the SourceWin y is",SourceWin.cy);
		#endif
				result = VIM_IPP_SetCaptureSize(gVc0528_Info.pSensorInfo,pt, SourceWin, CaptureSize); 
				if(result)
					return result;
				
	
				if(gVc0528_Info.pSensorInfo->snrGetEt!= NULL)				//guoying add 1/11/2007
				 	gVc0528_Info.pSensorInfo->snrGetEt(RegBuf);
				 				
				VIM_JPEG_SetSize(JPEG_422,CaptureSize);
				VIM_MAPI_SetCaptureRatio(gVc0528_Info.CaptureStatus.QualityMode);
				// angela added it for set capture ratio before preview on
				result = VIM_IPP_SetThumbSize((UINT8)gVc0528_Info.CaptureStatus.ThumbSize.cx, (UINT8)gVc0528_Info.CaptureStatus.ThumbSize.cy,VIM_IPP_THUMB_FROM_CAPTURE);
				if(result)
					return result;
				
				result =VIM_MARB_SetMap(gVc0528_Info.MarbStatus.WorkMode,VIM_DISP_NODISP,
				                  (PVIM_MARB_Map)&gVc0528_Info.MarbStatus.MapList);
				if(result)
					return result;
				VIM_JPEG_SetBitRateControlEn(DISABLE);//angela   
				VIM_IPP_SetDisplayDropFrame(0xffff);
				VIM_SIF_EnableSyncGen(ENABLE);	

				if(gVc0528_Info.pSensorInfo->snrSetEt!= NULL)				//guoying add 1/11/2007
				 	gVc0528_Info.pSensorInfo->snrSetEt(RegBuf,2);

				VIM_MAPI_Delay_Frame(2);   //Delay for the stable sensor data output

	
				return VIM_SUCCEED;
			}
	}
	return VIM_SUCCEED;
}


UINT8 VIM_MAPI_FindUpdateRange(TSize size, TPoint Point)
{
UINT8 InCon=0;
	if((Point.x>=gVc0528_Info.PreviewStatus.Point.x)&&
		(Point.x<=(gVc0528_Info.PreviewStatus.Point.x+
		gVc0528_Info.PreviewStatus.Size.cx)))
	{
		if((Point.y>=gVc0528_Info.PreviewStatus.Point.y)&&
		(Point.y<=(gVc0528_Info.PreviewStatus.Point.y+
		gVc0528_Info.PreviewStatus.Size.cy)))
			InCon++;
	}
	if(((Point.x+size.cx)>=gVc0528_Info.PreviewStatus.Point.x)&&
		((Point.x+size.cx)<=(gVc0528_Info.PreviewStatus.Point.x+
		gVc0528_Info.PreviewStatus.Size.cx)))
	{
		if((Point.y>=gVc0528_Info.PreviewStatus.Point.y)&&
		(Point.y<=(gVc0528_Info.PreviewStatus.Point.y+
		gVc0528_Info.PreviewStatus.Size.cy)))
			InCon++;
	}
	if(((Point.x+size.cx)>=gVc0528_Info.PreviewStatus.Point.x)&&
		((Point.x+size.cx)<=(gVc0528_Info.PreviewStatus.Point.x+
		gVc0528_Info.PreviewStatus.Size.cx)))
	{
		if(((Point.y+size.cy)>=gVc0528_Info.PreviewStatus.Point.y)&&
		((Point.y+size.cy)<=(gVc0528_Info.PreviewStatus.Point.y+
		gVc0528_Info.PreviewStatus.Size.cy)))
			InCon++;
	}
	if((Point.x>=gVc0528_Info.PreviewStatus.Point.x)&&
		(Point.x<=(gVc0528_Info.PreviewStatus.Point.x+
		gVc0528_Info.PreviewStatus.Size.cx)))
	{
		if(((Point.y+size.cy)>=gVc0528_Info.PreviewStatus.Point.y)&&
		((Point.y+size.cy)<=(gVc0528_Info.PreviewStatus.Point.y+
		gVc0528_Info.PreviewStatus.Size.cy)))
			InCon++;
	}
	return InCon;
}




/********************************************************************************
Description:
	ready to decode
Parameters:

Remarks:
	state: 	valid
*********************************************************************************/
VIM_RESULT VIM_MAPI_ReadyToDecode(VIM_HAPI_SAVE_MODE SaveMode,HUGE void *jpegBuf,VIM_HAPI_DECODE_MODE DecodeMode, UINT32 length,UINT16 DisplayWidth, UINT16 DisplayHeight)
{
VIM_RESULT result;
UINT32 readCnt;
TSize DisplaySize,DestDisplaySize;
TPoint pt;
	//DisplayWidth=(DisplayWidth&0xfffe);	
	//DisplayHeight=(DisplayHeight&0xfffe);

	gVc0528_Info.PreviewStatus.Mode=VIM_HAPI_PREVIEW_OFF;

	gVc0528_Info.DisplayStatus.SaveMode=SaveMode;
	gVc0528_Info.DisplayStatus.FileLength=0;
	gVc0528_Info.DisplayStatus.DecodeHeight=0;
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
			goto DECODEERROR;
		readCnt = VIM_USER_ReadFile(gVc0528_Info.DisplayStatus.pFileHandle,  gVc0528_Info.DisplayStatus.BufPoint, VIM_USER_BUFLENGTH);
	}

	//disable syncgen
	VIM_SIF_EnableSyncGen(DISABLE);		//angela 2006-8-15
	
	//get jpeg information( YVU mode, image size and other table parameters
	result = VIM_MAPI_GetJpegInformation(gVc0528_Info.DisplayStatus.BufPoint,readCnt);
	if(result)
		goto DECODEERROR;
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintDec("\n jpeg Image width:  ",gVc0528_JpegInfo.ImageSize.cx);
		VIM_USER_PrintDec("\n jpeg Image height:  ",gVc0528_JpegInfo.ImageSize.cy);
		VIM_USER_PrintDec("\n yuv is:  ",gVc0528_JpegInfo.YUVType);
#endif		
	// set decode jpeg mode
	result = VIM_MAPI_SetDecodeJpegMode(DecodeMode);
	if(result)
		goto DECODEERROR;

	// caculate the target
	DisplaySize.cx=DisplayWidth;
	DisplaySize.cy=DisplayHeight;	
	//get real display size(display image in user define display size, maybe not full display in LCD, only part of user define display size)
	//result = VIM_IPP_ToolCaculateLessSrcWindow(gVc0528_JpegInfo.ImageSize,DisplaySize,&DestDisplaySize);
	if(VIM_USER_DISPLAY_FULLSCREEN)
		result = VIM_IPP_ToolCaculateBigDstWindow(gVc0528_JpegInfo.ImageSize,DisplaySize,&DestDisplaySize);
	else
		result = VIM_IPP_ToolCaculateLessDisplayWindow(gVc0528_JpegInfo.ImageSize,DisplaySize,&DestDisplaySize);

	if(result)
		goto DECODEERROR;
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintDec("\n DestDisplaySize width:  ",DestDisplaySize.cx);
		VIM_USER_PrintDec("\n DestDisplaySize height:  ",DestDisplaySize.cy);
#endif	
	//if mul more than 8 ,lcdc special bit should be set
	
	VIM_DISP_EnableDecodeSpecial(DISABLE);
	if(gVc0528_JpegInfo.YUVType==JPEG_420)
	{
		if (DestDisplaySize.cy*16<gVc0528_JpegInfo.JpgSize.cy)
			VIM_DISP_EnableDecodeSpecial(ENABLE);
	}
	else
	{
		if (DestDisplaySize.cy*8<gVc0528_JpegInfo.JpgSize.cy)
			VIM_DISP_EnableDecodeSpecial(ENABLE);
	}
	
	//set ipp module image size
	VIM_IPP_SetImageSize(gVc0528_JpegInfo.JpgSize.cx, gVc0528_JpegInfo.JpgSize.cy);

	//set ipp module source window size, display size
	pt.x=0;//(((gVc0528_JpegInfo.ImageSize.cx-DestDisplaySize.cx)>>1))&0xfffc;
	pt.y=0;//(((gVc0528_JpegInfo.ImageSize.cy-DestDisplaySize.cy)>>1))&0xfffc;
	result = VIM_IPP_SetDispalySize(pt, gVc0528_JpegInfo.ImageSize, DestDisplaySize);//angela 2007-3-28
	if(result)
		goto DECODEERROR;
	
	//set LCDC A layer memory size and window size
	pt.x=pt.y=0;
	if(DestDisplaySize.cx>DisplaySize.cx)	
		pt.x=(((DestDisplaySize.cx-DisplaySize.cx)>>1))&0xfffc;

	result =VIM_DISP_SetA_Memory(pt,DestDisplaySize);
	if(result)
		goto DECODEERROR;
	
	pt.x=0;////(((DestDisplaySize.cx-DisplaySize.cx)>>1))&0xfffc;//angela 2007-3-27
	pt.y=0;//(((DestDisplaySize.cy-DisplaySize.cy)>>1))&0xfffc;
	if(DestDisplaySize.cx>DisplaySize.cx)	
		DestDisplaySize.cx=DisplaySize.cx;
	if(DestDisplaySize.cy>DisplaySize.cy)	
		DestDisplaySize.cy=DisplaySize.cy;
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintDec(" new DisplaySize width:  ",DestDisplaySize.cx);
		VIM_USER_PrintDec(" new DisplaySize height:  ",DestDisplaySize.cy);
#endif	

	
	VIM_DISP_SetA_DisplayPanel(pt,DestDisplaySize);
	result = VIM_MAPI_AdjustPoint((VIM_DISP_ROTATEMODE)gVc0528_Info.LcdStatus.ARotationMode,
					VIM_DISP_NOTCHANGE);
	if(result)
			goto DECODEERROR;
	VIM_DISP_SetRotateMode(VIM_DISP_ALAYER,(VIM_DISP_ROTATEMODE)gVc0528_Info.LcdStatus.ARotationMode);

	VIM_DISP_ResetState();
	
	//set layer A mode, normal display use ALAYER_LINEBUF mode
	gVc0528_Info.MarbStatus.ALayerMode = VIM_USER_DISPLAY_USELINEBUF;
	//set jpeg module parameter and marb map memory
	if(gVc0528_Info.DisplayStatus.BufPoint)
		result = VIM_MAPI_SetDisplayJpegParameter();
	else
		result = VIM_MAPI_SetAutoDisplayJpegParameter();
	
DECODEERROR:
	if(result)
	{
		if(SaveMode==VIM_HAPI_ROM_SAVE)
		{
			VIM_USER_FreeMemory(gVc0528_Info.DisplayStatus.MallocPr);
			gVc0528_Info.CaptureStatus.MallocPr=0;
		}
		return result;
	}
	return VIM_SUCCEED;

}
/********************************************************************************
Description:
	set jpeg information to jpeg module and marb map memory
parameters: 
	TSize size: the display data size
Return:
	VIM_SUCCEED:set parameter ok and map memory ok
Remarks:
*********************************************************************************/
VIM_RESULT VIM_MAPI_SetDisplayYUVParameter(TSize size)
{
	VIM_RESULT result;

	//set line buffer YUV mode, jpeg module image size, video word counter ect.
	result = VIM_JPEG_SetSize(JPEG_422, size);
	if(result)
			return result;

DISPLAYCHAGNEAMODE:
	result =VIM_MARB_SetMap(gVc0528_Info.MarbStatus.WorkMode,gVc0528_Info.MarbStatus.ALayerMode,
		                  (PVIM_MARB_Map)&gVc0528_Info.MarbStatus.MapList);
	if(result)
	{
		if(gVc0528_Info.MarbStatus.ALayerMode == VIM_DISP_LINEBUF)
			return result;
		else
		{
			gVc0528_Info.MarbStatus.ALayerMode =VIM_DISP_LINEBUF;
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
			VIM_USER_PrintString("****should change to lienbuf mode****");
#endif
			goto DISPLAYCHAGNEAMODE;
		}
	}

	//angela added it 2006-8-22 for auto just a mode
	VIM_JPEG_AdjustClkBlank();

	//ready start do display
	VIM_MAPI_StartDecode();
	return VIM_SUCCEED;
}

/********************************************************************************
Description:
	Set chip mode(VIM_MARB_DISPLAYSTILLBYPASS_MODE)
parameters: 
	void
	
Return:
	void
Remarks:
*********************************************************************************/	
void VIM_MAPI_SetDisplayYUVMode(void)
{
	VIM_SIF_EnableSyncGen(DISABLE);
      	VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER,DISABLE);	
      // VIM_USER_DelayMs(100); 
      	VIM_MAPI_SetChipMode(VIM_MARB_DISPLAYSTILLBYPASS_MODE,VIM_IPP_HAVE_NOFRAME);
     // 	VIM_USER_DelayMs(100); 
      	VIM_IPP_SetDisplayDropFrame(0xffff);
}

/********************************************************************************
Description:
	Set the display parameters to be ready for display YUV data.
parameters: 
	VIM_HAPI_DISPYUV_Data *struc: the head point of structure
	
Return:
	void
Remarks:
*********************************************************************************/	
UINT16 VIM_MAPI_ReadyToDisplayYUV(VIM_DISPYUV_Data *struc)
{
	VIM_RESULT result;
	TSize DisplaySize,DestDisplaySize;
	TSize size1,size2;
	
	TPoint pt = {0, 0};
	
	//result = VIM_MAPI_CheckDisplaySize(struc->Dest_Offset_Pt.x,struc->Dest_Offset_Pt.y,struc->DestSize.cx,struc->DestSize.cy,&DisplaySize);
	result = VIM_MAPI_CheckDisplaySize(struc->DestOffset_X,struc->DestOffset_Y,struc->DestSize_W,struc->DestSize_H,&DisplaySize);
	if(result)
		return result;

	size1.cx = DisplaySize.cx;
	size1.cy = DisplaySize.cy;

	size2.cx = struc->SourceSize_W;
	size2.cy = struc->SourceSize_H;
	
	VIM_MAPI_SetDisplayYUVMode(); 

	
	if(VIM_USER_DISPLAY_FULLSCREEN)
		result = VIM_IPP_ToolCaculateBigDstWindow(size2,size1,&DestDisplaySize);
	else
		result = VIM_IPP_ToolCaculateLessDisplayWindow(size2,size1,&DestDisplaySize);
	if(result)
		return result;
#if(VIM_HIGH_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintDec("\n Dest DisplaySize width:  ",DestDisplaySize.cx);
		VIM_USER_PrintDec("\n Dest DisplaySize height:  ",DestDisplaySize.cy);
#endif

	//set ipp module image size
	VIM_IPP_SetImageSize(struc->SourceSize_W,struc->SourceSize_H);

	//set ipp module source window size, display size
	result = VIM_IPP_SetDispalySize(pt, size2, DestDisplaySize);//angela 2007-3-5
	if(result)
		return result;



	pt.x=pt.y=0;
	if(DestDisplaySize.cx>DisplaySize.cx)	
		pt.x=(((DestDisplaySize.cx-DisplaySize.cx)>>1))&0xfffc;
	if(DestDisplaySize.cy>DisplaySize.cy)	
		pt.y=(((DestDisplaySize.cy-DisplaySize.cy)>>1))&0xfffc;

	
	result =VIM_DISP_SetA_Memory(pt,DestDisplaySize);
	if(result)
		return result;
	

	pt.x=(struc->DestOffset_X)&0xfffc;
	pt.y=(struc->DestOffset_Y)&0xfffc;
	
	if(DestDisplaySize.cx<=DisplaySize.cx)	
		pt.x=(((DisplaySize.cx-DestDisplaySize.cx)>>1)+struc->DestOffset_X)&0xfffc;
	else
		DestDisplaySize.cx=DisplaySize.cx;
	if(DestDisplaySize.cy<=DisplaySize.cy)	
		pt.y=(((DisplaySize.cy-DestDisplaySize.cy)>>1)+struc->DestOffset_Y)&0xfffc;
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


	//enable a layer
	VIM_DISP_ResetState();
	VIM_DISP_SetLayerEnable(VIM_DISP_ALAYER,ENABLE);



	result = VIM_MAPI_SetDisplayYUVParameter(size2);
DISPLAYERROR:
	if(result)
		return result;
	
	return 0;
}




/********************************************************************************
Description:
	write current YUV422 data to jbuf
parameters: 
	Truelen: the true data length of this frame  
	start	    : the start address for YUV422 data
Return:
	void
Remarks:
*********************************************************************************/
void VIM_MAPI_WriteOneFrameYUVData(UINT32 Truelen,HUGE UINT8 *start)
{
 UINT32 marblen, offset=0,Waittimes=0xfff,templen;
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
 //UINT8 JpegStatus=0;
#endif

	VIM_MARB_SetJpegbufGapratio(8);
	marblen=gVc0528_Info.MarbStatus.MapList.jbufsize;	
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintHex("\n JbufSize 	 : ", marblen);
		VIM_USER_PrintHex("\n JpegFileSize : ", Truelen);
#endif

	gVc0528_Info.MarbStatus.Jpgpoint = 0;

	while(Waittimes--)
	{
		if(Truelen)
		{
			templen=VIM_MARB_GetJbufRWSize(gVc0528_Info.MarbStatus.Jpgpoint);
			if(templen>=Truelen)
			{
				gVc0528_Info.MarbStatus.Jpgpoint=VIM_MARB_WriteJpegData(start+offset, Truelen,gVc0528_Info.MarbStatus.Jpgpoint,0);
				Truelen=0;	
			}
			else
			{
				gVc0528_Info.MarbStatus.Jpgpoint=VIM_MARB_WriteJpegData(start+offset, templen,gVc0528_Info.MarbStatus.Jpgpoint,0);
				offset+=templen;
				Truelen-=templen;
			}
		}
		if(!Truelen)
		{
			break;	
		}
		VIM_USER_DelayMs(5);
	}
}

/********************************************************************************
Description:
	Calculate the RGB565 data to YUV data according the formula.
on LCD
Parameters:
      UINT8 *pbuf: the head point of RGB data buffer
      UINT8 *pYUV: the head point of YUV data buffer
                      
Remarks:
//	Y = ( 77R + 150G + 29B)/256
//	U = (-43R - 85G + 128B)/256 + 128
//	V = (128R - 107G - 21B)/256 + 128
state: 
	valid

********************************************************************************/
void VIM_MAPI_RGB565toYUV(UINT8 *pRGB, UINT8 *pYUV)
{
	UINT8 r,g,b;
	//r =((*pRGB)&0xf8)|0x4; 
	//g =(((*pRGB)&0x7)<<5)|(((*(pRGB+1))&0xe0)>>3)|0x2; pRGB++;
	//b = (((*pRGB)&0x1f)<<3)|0x4;pRGB++ ; 
	b = (((*pRGB)&0x1f)<<3);
	g =((*(pRGB+1)&0x7)<<5)|((*(pRGB)&0xe0)>>3); pRGB++;

	r =((*pRGB)&0xf8); pRGB++;
	
	*pYUV = (77*r + 150*g + 29*b )/256;    pYUV++;   // y
	*pYUV = (-43*r - 85*g + 128*b )/256+128;  pYUV++;   // u
	*pYUV = (128*r - 107*g - 21*b)/256+ 128;             // v
	
	//*pYUV = (char)(0.257*r + 0.504*g + 0.098*b + 16);    pYUV++;   // y
	//*pYUV = (char)(-0.148*r - 0.291*g + 0.439*b + 128);  pYUV++;   // u
	//*pYUV = (char)(0.439*r - 0.368*g - 0.071*b + 128);             // v
}

/********************************************************************************
Description:
	write the Y0 data  to Y0 block, Y1 data to Y1 block, U data to U block and V data to V block 
on LCD
Parameters:
      UINT8 *pbuf: the head point of yuv data buf 422 uyvy
      UINT16 Width: the data width of one line
      UINT16 Height:	 the lines number of image which need to encode
                 
Remarks:
state: 
	valid

********************************************************************************/
void VIM_MAPI_WriteUYVYEncodeDataLine(UINT8 *pbuf,UINT16 Width,UINT16 Height)
{
UINT16 uint,i,j,m,leavex,uint1;
	uint=uint1=Width/16;
	leavex=Width%16;
	if(leavex)
		uint1++;
	if(!Height)
		return;
	for(i=0;i<uint1;i++)
	{
		for(j=0;j<8;j++)	//y0 block
			for(m=0;m<8;m++)
				{
					if(((m>leavex)&&(i==uint))||(Height==j))
						VIM_JPEG_WriteYUVData(0);
					else
						VIM_JPEG_WriteYUVData(pbuf[i*32+j*Width*2+m*2+1]+128);
				}
		for(j=0;j<8;j++)	//y1 block
			for(m=0;m<8;m++)
				{
					if((m>(leavex+8))&&(i==uint)||(Height==j))
						VIM_JPEG_WriteYUVData(0);
					else
						VIM_JPEG_WriteYUVData(pbuf[i*32+16+2*j*Width+2*m+1]+128);
				}
		for(j=0;j<8;j++)	//u block
			for(m=0;m<8;m++)
				{
					if((m>(leavex*2))&&(i==uint)||(Height==j))
						VIM_JPEG_WriteYUVData(0);
					else
						VIM_JPEG_WriteYUVData(pbuf[i*32+j*Width*2+4*m]+128);
				}
		for(j=0;j<8;j++)	//v block
			for(m=0;m<8;m++)
				{
					if((m>(leavex*2))&&(i==uint)||(Height==j))
						VIM_JPEG_WriteYUVData(0);
					else
						VIM_JPEG_WriteYUVData(pbuf[i*32+j*Width*2+4*m+2]+128);
				}
	}
}

/*
block to block 
*/

/********************************************************************************
Description:
	Write the YUV data to yuv data buffer
on LCD
Parameters:
      UINT8 *pbuf: the head point of yuv data buf 422 uyvy
      TSize Size: the size of image
      VIM_YUV_TYPE fmt:	 the YUV type: YUV422
                 
Remarks:
state: 
	valid

********************************************************************************/
void VIM_MAPI_SendYuvData(UINT8 *pbuf, TSize Size,VIM_YUV_TYPE fmt)
{
	UINT32 i = 0;

	switch(fmt)
	{
		case VIM_YUV_UYVY422:
			for(i=0;i<(Size.cy/8);i++)
			{
				VIM_MAPI_WriteUYVYEncodeDataLine(pbuf+i*8*(Size.cx<<1),Size.cx,8);
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
				VIM_USER_PrintDec("VIM_JPEG_GetJpegWordCount  ",VIM_JPEG_GetJpegWordCount());
				VIM_USER_PrintDec("VIM_MARB_GetJbufChipPointer  ",VIM_MARB_GetJbufChipPointer());
				VIM_USER_PrintDec("VIM_JPEG_GetJpegBlockCount  ",VIM_JPEG_GetJpegBlockCount());


#endif				
			}
			VIM_MAPI_WriteUYVYEncodeDataLine(pbuf+i*8*(Size.cx<<1),Size.cx,(Size.cy%8));
			break;
		default:
			break;
	}

}


/********************************************************************************
Description:
	set display mode and display size parameters, ready to display one frame jpeg picture.
on LCD
Parameters:
      UINT16 JpegOffsetX: the start coordinate X offset of jpeg picture
      UINT16 JpegOffsetY: the start coordinate Y offset of jpeg picture
      Step:	 the step number when zoom the jpeg picture
                 
Return:
	VIM_SUCCEED:  ok
	VIM_ERROR_WORKMODE: error work mode
	VIM_ERROR_YUV_UNKNOW:jpeg file yuv mode error, can not display
	VIM_ERROR_PARAMETER: marb jbuf size less than capture size, so data is not good
	VIM_ERROR_?????????: return form mid api

Remarks:
state: 
	valid

********************************************************************************/
VIM_RESULT VIM_MAPI_ReadyToDisplayZoom(UINT16 JpegOffsetX, UINT16 JpegOffsetY, VIM_HAPI_DISPLAY_ZOOM Step)
{
	VIM_RESULT result;
       TSize DisplaySize,DestDisplaySize,SourseSize;
	TPoint pt = {0, 0};
	UINT32 readCnt=0;
	
	if(gVc0528_Info.DisplayStatus.SaveMode==VIM_HAPI_RAM_SAVE)
	{
	}
	else
	{
		gVc0528_Info.DisplayStatus.FileLength=0;
		gVc0528_Info.DisplayStatus.BufOffset=0;
		gVc0528_Info.DisplayStatus.BufLength=VIM_USER_BUFLENGTH;
		gVc0528_Info.DisplayStatus.MallocPr=gVc0528_Info.DisplayStatus.BufPoint=VIM_USER_MallocMemory(VIM_USER_BUFLENGTH);
		if(gVc0528_Info.DisplayStatus.BufPoint==NULL)
			return VIM_ERROR_BUFPOINT;
		//read file from ROM to user define RAM
		result = VIM_USER_SeekFile(gVc0528_Info.DisplayStatus.pFileHandle,0);
		if(result)
			goto DISPLAYZOOMERROR;
		readCnt = VIM_USER_ReadFile(gVc0528_Info.DisplayStatus.pFileHandle,  gVc0528_Info.DisplayStatus.BufPoint, VIM_USER_BUFLENGTH);

		//get jpeg information( YVU mode, image size and other table parameters
		result = VIM_MAPI_GetJpegInformation(gVc0528_Info.DisplayStatus.BufPoint,readCnt);
		if(result)
			goto DISPLAYZOOMERROR;
	}

	//disable syncgen
	VIM_SIF_EnableSyncGen(DISABLE);	//angela 2006-8-15
		

	// set all module to display jpeg mode
	VIM_MAPI_SetDisplayJpegMode();  
	gVc0528_Info.PreviewStatus.Mode=VIM_HAPI_PREVIEW_OFF;

	//from version 0.2 need display whole pic in LCD, so source size =jpeg image size
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
	VIM_USER_PrintDec("[VC0578][HAPI]jpeg Image width:  ",gVc0528_JpegInfo.ImageSize.cx);
	VIM_USER_PrintDec("[VC0578][HAPI]jpeg Image height:  ",gVc0528_JpegInfo.ImageSize.cy);
	VIM_USER_PrintDec("[VC0578][HAPI]yuv is:  ",gVc0528_JpegInfo.YUVType);
#endif	

	DisplaySize=gVc0528_Info.DisplayStatus.LcdWantSize;
	SourseSize=gVc0528_JpegInfo.ImageSize;

	//get real display size(display image in user define display size, maybe not full display in LCD, only part of user define display size)
	if(VIM_USER_DISPLAY_FULLSCREEN)
		result = VIM_IPP_ToolCaculateBigDstWindow(gVc0528_JpegInfo.ImageSize,DisplaySize,&DestDisplaySize);
	else
		result = VIM_IPP_ToolCaculateLessDisplayWindow(gVc0528_JpegInfo.ImageSize,DisplaySize,&DestDisplaySize);
	if(result)
		goto DISPLAYZOOMERROR;
	
	// zoom size
	SourseSize.cx=DestDisplaySize.cx+(SourseSize.cx-DestDisplaySize.cx)*(VIM_USER_DISPLAY_ZOOMDIVISOR-Step)/VIM_USER_DISPLAY_ZOOMDIVISOR;
	SourseSize.cy=DestDisplaySize.cy+(SourseSize.cy-DestDisplaySize.cy)*(VIM_USER_DISPLAY_ZOOMDIVISOR-Step)/VIM_USER_DISPLAY_ZOOMDIVISOR;



	if(DestDisplaySize.cx<DisplaySize.cx)
	{
		SourseSize.cx=gVc0528_JpegInfo.ImageSize.cx;
	}
	if(DestDisplaySize.cy<DisplaySize.cy)
	{
		SourseSize.cy=gVc0528_JpegInfo.ImageSize.cy;
	}

	DestDisplaySize.cy&=0xfff8;
	DestDisplaySize.cx&=0xfff8;		


	pt.x=(gVc0528_JpegInfo.ImageSize.cx-SourseSize.cx)/VIM_USER_DISPLAY_ZOOMOFFSETMAXSTEP;
	if(pt.x<VIM_USER_DISPLAY_ZOOMMINPIXEL)
		pt.x=VIM_USER_DISPLAY_ZOOMMINPIXEL;
	pt.x*=JpegOffsetX;
	if((pt.x+SourseSize.cx)>(gVc0528_JpegInfo.ImageSize.cx))
		pt.x=(gVc0528_JpegInfo.ImageSize.cx-SourseSize.cx);
	pt.y=(gVc0528_JpegInfo.ImageSize.cy-SourseSize.cy)/VIM_USER_DISPLAY_ZOOMOFFSETMAXSTEP;
	if(pt.y<VIM_USER_DISPLAY_ZOOMMINPIXEL)
		pt.y=VIM_USER_DISPLAY_ZOOMMINPIXEL;
	pt.y*=JpegOffsetY;
	if((pt.y+SourseSize.cy)>(gVc0528_JpegInfo.ImageSize.cy))
		pt.y=(gVc0528_JpegInfo.ImageSize.cy-SourseSize.cy);
	
	pt.x&=0xfff8;
	pt.y&=0xfff8;


	if((SourseSize.cx>=DestDisplaySize.cx)&&(SourseSize.cy<DestDisplaySize.cy))
		DestDisplaySize.cy=SourseSize.cy;
	//VIM_IPP_SetEffect(VIM_IPP_EFFECT_FOUR_COLOR);
	//set ipp module source window size, display size
	result = VIM_IPP_SetDispalySize(pt, SourseSize, DestDisplaySize);
	if(result)
		goto DISPLAYZOOMERROR;
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintDec("[VC0578][HAPI]jpeg offset x:  ",pt.x);
		VIM_USER_PrintDec("[VC0578][HAPI]jpeg offset y:  ",pt.y);
#endif	
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintDec("[VC0578][HAPI]SourseSize  width:  ",SourseSize.cx);
		VIM_USER_PrintDec("[VC0578][HAPI]SourseSize height:  ",SourseSize.cy);
#endif			

	//set LCDC A layer memory size and window size

	pt.x=pt.y=0;
	if(DestDisplaySize.cx>DisplaySize.cx)	
		pt.x=(((DestDisplaySize.cx-DisplaySize.cx)>>1))&0xfffc;
	if(DestDisplaySize.cy>DisplaySize.cy)	
		pt.y=(((DestDisplaySize.cy-DisplaySize.cy)>>1))&0xfffc;	

	//DestDisplaySize.cx=SourseSize.cx;
#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintDec("[VC0578][HAPI]a mem offset x:  ",pt.x);
		VIM_USER_PrintDec("[VC0578][HAPI]a mem offset y:  ",pt.y);
		VIM_USER_PrintDec("[VC0578][HAPI]a mem Width:  ",DestDisplaySize.cx);
		VIM_USER_PrintDec("[VC0578][HAPI]a mem Height :  ",DestDisplaySize.cy);		
#endif		
	result =VIM_DISP_SetA_Memory(pt,DestDisplaySize);
	if(result)
		goto DISPLAYZOOMERROR;
	
	pt=gVc0528_Info.DisplayStatus.LcdWantPt;
	if(DestDisplaySize.cx<=DisplaySize.cx)	
		pt.x=(((DisplaySize.cx-DestDisplaySize.cx)>>1)+gVc0528_Info.DisplayStatus.LcdWantPt.x)&0xfffc;
	else
		DestDisplaySize.cx=DisplaySize.cx;
	if(DestDisplaySize.cy<=DisplaySize.cy)	
		pt.y=(((DisplaySize.cy-DestDisplaySize.cy)>>1)+gVc0528_Info.DisplayStatus.LcdWantPt.y)&0xfffc;
	else
		DestDisplaySize.cy=DisplaySize.cy;


#if(VIM_MID_API_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintDec("[VC0578][HAPI]a display offset x:  ",pt.x);
		VIM_USER_PrintDec("[VC0578][HAPI] a display offset y:  ",pt.y);
		VIM_USER_PrintDec("[VC0578][HAPI] new DisplaySize width:  ",DestDisplaySize.cx);
		VIM_USER_PrintDec("[VC0578][HAPI] new DisplaySize height:  ",DestDisplaySize.cy);
#endif	

	VIM_DISP_SetA_DisplayPanel(pt,DestDisplaySize);
	result = VIM_MAPI_AdjustPoint((VIM_DISP_ROTATEMODE)gVc0528_Info.LcdStatus.ARotationMode,
					VIM_DISP_NOTCHANGE);
	if(result)
			goto DISPLAYZOOMERROR;
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


DISPLAYZOOMERROR:
	if(result)
	{
		if(gVc0528_Info.DisplayStatus.SaveMode==VIM_HAPI_ROM_SAVE)
		{
			VIM_USER_FreeMemory(gVc0528_Info.DisplayStatus.MallocPr);
			gVc0528_Info.CaptureStatus.MallocPr=0;
		}
		return result;
	}
	return VIM_SUCCEED;
}

