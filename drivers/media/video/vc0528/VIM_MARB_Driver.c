
/*************************************************************************
*                                                                       
*                Copyright (C) 2005 Vimicro CO.,LTD     		 
*
* FILE NAME                                  VERSION                
*                                                                       
* VIM_MARB_Driver.c			   0.2                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*     VC0578 's marb moudle sub driver                                 
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
#include"VIM_COMMON.h"
///////////////////////////tools start ////////////////////////////////
UINT32 VIM_MARB_ReadBuf(HUGE UINT8 *pbuf, UINT32 len, UINT32 start, UINT32 end, UINT32 pos)
{
	UINT32 size, len1;

	while(len)
	{
		size = end - pos;
		len1 = (len > size) ? size : len;
		VIM_HIF_ReadSram(pos, pbuf, len1);
		pbuf += len1;
		len -= len1;
		pos += len1;
		pos = (pos == end) ? start : pos;
	}
	return pos;
}
UINT32 VIM_MARB_WriteBuf(HUGE UINT8 *pbuf, UINT32 len, UINT32 start, UINT32 end, UINT32 pos)
{
	UINT32 size, len1;

	while(len)
	{
		size = end - pos;
		len1 = (len > size) ? size : len;
		VIM_HIF_WriteSram(pos, pbuf, len1);
		pbuf += len1;
		len -= len1;
		pos += len1;
		pos = (pos == end) ? start : pos;
	}
	return pos;
}

static UINT32 VIM_MARB_GetBmemSize(VIM_DISP_BFORMAT fmt, TSize size)
{
	UINT32 len;

	len = (UINT32)size.cx & 0xffff;
	len *= (UINT32)size.cy & 0xffff;
	switch(fmt & 0xf)
	{
	case VIM_DISP_BLAYER_RGB1:
		len += 7;
		len >>= 3;
		break;
	case VIM_DISP_BLAYER_RGB2:
		len += 3;
		len >>= 2;
		break;
	case VIM_DISP_BLAYER_RGB4:
		len ++;
		len >>= 1;
		break;
	case VIM_DISP_BLAYER_RGB8:
		break;
	case VIM_DISP_BLAYER_RGB444:
		len *= 3;
		len += 1;
		len >>= 1;
		break;
	case VIM_DISP_BLAYER_RGB555:
	case VIM_DISP_BLAYER_RGB565:
		len <<= 1;
		break;
	case VIM_DISP_BLAYER_RGB666:
		len *= 18;
		len += 7;
		len >>= 3;
		break;
	case VIM_DISP_BLAYER_RGB24:
		len *= 3;
		break;
	case VIM_DISP_BLAYER_RGB32:
		len <<= 2;
		break;
	default:
		break;
	}
	return ((len + 3) >> 2) << 2;
}

static UINT32 VIM_MARB_GetAmemSize(VIM_DISP_BUFFERMODER fmt, TSize size)
{
	UINT32 len;

	len = (UINT32)size.cx & 0xffff;
	switch(fmt)
	{
	case VIM_DISP_LINEBUF:
		len <<= 5;
		break;
	case VIM_DISP_TWOFRAME:
		len *= (UINT32)size.cy & 0xffff;
		len <<= 2;
		break;
	case VIM_DISP_ONEFRAME:
		len *= (UINT32)size.cy & 0xffff;
		len += 3;
		len >>= 2;
		len <<= 3;
		break;
	case VIM_DISP_NODISP:
		len=0;
	default:
		break;
	}
	return len;
}

static UINT32 VIM_MARB_GetDecLbuf1Size(UINT8 fmt, UINT16 width)
{
	TSize size;

	size.cx = width;
	size.cy = 0;
	switch(fmt)
	{
	case JPEG_422:
	case JPEG_411:
	case JPEG_444:
	case JPEG_400:
		size.cy = 8;
		break;
	case JPEG_420:
		size.cy = 16;
		break;
	default:
		break;
	}
	//size.cy += 5;
	return VIM_JPEG_GetYuvSize(JPEG_422, size);
}

static UINT32 VIM_MARB_GetDecLbuf0Size(VIM_JPEG_YUVMODE fmt, UINT16 width)
{
	TSize size;

	size.cy = 0;
	switch(fmt)
	{
	case JPEG_422:
		size.cx = ((width + 15) >> 4) << 4;
		size.cy = 8;
		break;
	case JPEG_411:
		size.cx = ((width + 31) >> 5) << 5;
		size.cy = 8;
		break;
	case JPEG_444:
	case JPEG_400:
		size.cx = ((width + 7) >> 3) << 3;
		size.cy = 8;
		break;
	case JPEG_420:
		size.cx = ((width + 15) >> 4) << 4;
		size.cy = 16;
		break;
	default:
		break;
	}
	if(fmt == JPEG_444)
		fmt = JPEG_422;
	return VIM_JPEG_GetYuvSize(fmt, size);
}
//////////////////////////tools  end///////////////////////////
/********************************************************************************
  Description:
	start captue by marb
	clear by hardware
  Parameters:
	void
  Returns:
  	void
  Remarks:
*********************************************************************************/
void VIM_MARB_StartCapture(void)
{
	VIM_HIF_SetReg8(V5_REG_MARB_MARB_CTRL,  BIT1);
}
/********************************************************************************
  Description:
	stop captue video by marb
	clear by hardware
  Parameters:
	void
  Returns:
  	void
  Remarks:
*********************************************************************************/
void VIM_MARB_StopCapture(void)
{
	VIM_HIF_SetReg8(V5_REG_MARB_MARB_CTRL,  BIT2);
}
/********************************************************************************
  Description:
	send decode command  by marb
	clear by hardware
  Parameters:
	void
  Returns:
  	void
  Remarks:
*********************************************************************************/
void VIM_MARB_StartDecode(void)
{
	VIM_HIF_SetReg8(V5_REG_MARB_MARB_CTRL,  BIT3);
}

/********************************************************************************
  Description:
	update the timer vale to the timer value register 
  Parameters:
	void
  Returns:
  	void
  Remarks:
*********************************************************************************/
void VIM_MARB_RefreshTimerVal(void)
{
	VIM_HIF_SetReg8(V5_REG_MARB_MARB_CTRL,  BIT7);
}
/********************************************************************************
  Description:
	reset marb 's sub module
  Parameters:
	Module:
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
	  VIM_MARB_NO_RESET=0,
	  //set ALL reset
	  VIM_MARB_RESET_ALL
  Returns:
  	void
  Remarks:
*********************************************************************************/

void VIM_MARB_ResetSubModule(VIM_MARB_SUB_RESET Module)
{
	VIM_HIF_SetReg8(V5_REG_MARB_MARB_RESET,  Module);
	VIM_USER_DelayMs(1); 
	VIM_HIF_SetReg8(V5_REG_MARB_MARB_RESET,  0);
}
/********************************************************************************
  Description:
	set marb mode
  Parameters:
	mode:
		// Preview modes
		VIM_MARB_PREVIEW_MODE, // Auto Bit Rate Control enable

		// Capture modes
		VIM_MARB_CAPTURESTILL_MODE,
		VIM_MARB_CAPTURESTILLTHUMB_MODE,
		VIM_MARB_CAPTUREVIDEO_MODE,
		VIM_MARB_CAPTUREAVI_MODE,	//include header and index

		VIM_MARB_MULTISHOT_MODE,
		VIM_MARB_MULTISHOTTHUMB_MODE,

		// Display modes
		VIM_MARB_DISPLAYSTILL_MODE,
		VIM_MARB_DISPLAYVIDEO_MODE,

		// Encode modes
		VIM_MARB_ENCODE_MODE,

		// Decode modes
		VIM_MARB_DECODEYUV_MODE, // Without IPP
		VIM_MARB_DECODERGB_MODE,

		// Special modes
		VIM_MARB_DIRECTDISPLAY_MODE,
		VIM_MARB_BYPASS_MODE,
  Returns:
  	void
  Remarks:
*********************************************************************************/
void VIM_MARB_SetMode(VIM_MARB_WORKMODE bMode)
{
UINT8 temp;
	temp=VIM_HIF_GetReg8(V5_REG_MARB_MODE_CTRL_0);//angela 0228
	temp&=0x80;
	switch(bMode)
	{
		case VIM_MARB_PREVIEW_MODE:
		case VIM_MARB_CAPTURESTILL_MODE:
		case VIM_MARB_ENCODE_MODE:
			temp|=0x38;
			break;
		case VIM_MARB_CAPTURESTILLTHUMB_MODE:
			temp|=0x30;
			break;
		case VIM_MARB_CAPTUREVIDEO_MODE:
		case VIM_MARB_CAPTUREAVI_MODE:
			temp|=0x3a;
			break;
		case VIM_MARB_MULTISHOT_MODE:
			temp|=0x39;
			break;
		case VIM_MARB_MULTISHOTTHUMB_MODE:
			temp|=0x31;
			break;
		case VIM_MARB_DISPLAYSTILL_MODE:
		case VIM_MARB_DISPLAYSTILLBYPASS_MODE:
		case VIM_MARB_DISPLAYVIDEO_MODE:
		case VIM_MARB_DECODEYUV_MODE:
		case VIM_MARB_DECODERGB_MODE:
			temp|=0xbc;
			break;
		case VIM_MARB_DIRECTDISPLAY_MODE:
		case VIM_MARB_BYPASS_MODE:
		default:
			break;
	}
	VIM_HIF_SetReg8(V5_REG_MARB_MODE_CTRL_0,temp);
	VIM_HIF_SetReg8(V5_REG_MARB_MODE_CTRL_1,4);
}

/********************************************************************************
  Description:
    set the sram in marb ' mode
  Parameters:
  	Num:	
		VIM_MARB_1TSRAM_0=BIT0,
		VIM_MARB_1TSRAM_1=BIT1,
		VIM_MARB_1TSRAM_2=BIT2,
		VIM_MARB_1TSRAM_1AND2=(BIT2|BIT1),
		VIM_MARB_1TSRAM_0AND1=(BIT0|BIT1),
		VIM_MARB_1TSRAM_ALL=(BIT2|BIT1|BIT0),
	Mode: 
		VIM_MARB_1TSRAM_POWERON=0,
		VIM_MARB_1TSRAM_STANDBY=1,
		VIM_MARB_1TSRAM_OFF=2
  Returns:
  	void
  Remarks: 
*********************************************************************************/

void VIM_MARB_Set1TSramMode(VIM_MARB_1TSRAM_NUM Num,VIM_MARB_1TSRAM_MODE Mode)
{
	UINT8 Temp,Temp2;
	Temp=VIM_HIF_GetReg8(V5_REG_MARB_1T_CTRL1);
	Temp2=VIM_HIF_GetReg8(V5_REG_MARB_MEMORY_CTRL);
	switch(Mode)
	{
	case VIM_MARB_1TSRAM_POWERON:
		Temp2&=(~(Num<<4));//clock on
		VIM_HIF_SetReg8(V5_REG_MARB_MEMORY_CTRL,Temp2);
		Temp2|=Num;	// power on
		VIM_HIF_SetReg8(V5_REG_MARB_MEMORY_CTRL,Temp2);
		Temp|=Num;	//enable wake up
		//Temp|=0x77;	//angela 1017 for disable ecc wake up
		VIM_HIF_SetReg8(V5_REG_MARB_1T_CTRL1,Temp);
		break;
	case VIM_MARB_1TSRAM_STANDBY:
		Temp2|=Num;	// power on
		VIM_HIF_SetReg8(V5_REG_MARB_MEMORY_CTRL,Temp2);
		Temp&=(~Num);	// stand by
		VIM_HIF_SetReg8(V5_REG_MARB_1T_CTRL1,Temp);
		Temp2|=(Num<<4);//clock off
		VIM_HIF_SetReg8(V5_REG_MARB_MEMORY_CTRL,Temp2);
		break;
	case VIM_MARB_1TSRAM_OFF:
		Temp&=(~Num);	// stand by
		VIM_HIF_SetReg8(V5_REG_MARB_1T_CTRL1,Temp);
		Temp2|=(Num<<4);//clock off
		VIM_HIF_SetReg8(V5_REG_MARB_MEMORY_CTRL,Temp2);
		Temp2&=(~Num);//power off	
		VIM_HIF_SetReg8(V5_REG_MARB_MEMORY_CTRL,Temp2);
		break;
	}
}
/********************************************************************************
  Description:
  	when in multi shot mode ,set the max frame 
  Parameters:
	FramCount: the max frame 
  Returns:
  	void
  Remarks:
*********************************************************************************/

void VIM_MARB_SetMultiFrameCount(UINT8 FramCount)
{
	FramCount--;
	VIM_HIF_SetReg8(V5_REG_MARB_MSHOT_FCNT,FramCount);
}


UINT8 VIM_MARB_GetModeCtrl0(void)
{
	return VIM_HIF_GetReg8(V5_REG_MARB_MODE_CTRL_0);
}
/********************************************************************************
  Description:
	set the enable or disable function about stop video by timer
  Parameters:
	Enable:
		ENABLE
		DISABLE
  Returns:
  	void
  Remarks:
*********************************************************************************/

void VIM_MARB_StopVideoByTimerEn(BOOL Enable)
{
       UINT8 Temp;
	Temp=VIM_HIF_GetReg8(V5_REG_MARB_MODE_CTRL_1);
	Temp&=(~BIT2);
	Temp|=(Enable<<2);
	VIM_HIF_SetReg8(V5_REG_MARB_MODE_CTRL_1,Temp);
	
}
/********************************************************************************
  Description:
	set the timer mode (auto reset or reset by host)
  Parameters:
	Enable:
		ENABLE
		DISABLE
  Returns:
  	void
  Remarks:
*********************************************************************************/
void VIM_MARB_ResetTimerAutoEn(BOOL Enable)
{
	UINT8 Temp;
	Temp=VIM_HIF_GetReg8(V5_REG_MARB_MODE_CTRL_1);
	Temp&=(~BIT3);
	Temp|=(Enable<<3);
	VIM_HIF_SetReg8(V5_REG_MARB_MODE_CTRL_1,Temp);
}
/********************************************************************************
  Description:
	reset marb_ipp and marb_jpeg if jbuf_err ,tbuf_err or recapture_err occurred
  Parameters:
	Enable:
		ENABLE
		DISABLE
  Returns:
  	void
  Remarks:
*********************************************************************************/

void VIM_MARB_ResetSubModuleAutoEn(BOOL Enable)
{
UINT8 Temp;
	Temp=VIM_HIF_GetReg8(V5_REG_MARB_MODE_CTRL_1);
	Temp&=(~BIT4);
	if(!Enable)
		Temp|=(1<<4);
	VIM_HIF_SetReg8(V5_REG_MARB_MODE_CTRL_1,Temp);
}
/********************************************************************************
  Description:
	set jbuf memary 
  Parameters:
	StartAdd: the start address of sram
	Size: the size of sram
  Returns:
  	void
  Remarks:
*********************************************************************************/

void VIM_MARB_SetJbufMem(UINT32 StartAdd,UINT32 Size)
{
	VIM_HIF_SetReg8(V5_REG_MARB_JBUF_START_2,(UINT8)StartAdd);
	VIM_HIF_SetReg8(V5_REG_MARB_JBUF_START_1,(UINT8)(StartAdd>>8));
	VIM_HIF_SetReg8(V5_REG_MARB_JBUF_START_0,(UINT8)(StartAdd>>16));
	
	VIM_HIF_SetReg8(V5_REG_MARB_JBUF_END_2,(UINT8)(StartAdd+Size));
	VIM_HIF_SetReg8(V5_REG_MARB_JBUF_END_1,(UINT8)((StartAdd+Size)>>8));
	VIM_HIF_SetReg8(V5_REG_MARB_JBUF_END_0,(UINT8)((StartAdd+Size)>>16));
}
/********************************************************************************
  Description:
	An interrupt is generated when ¡°jbuf_intv¡± data is write or read
	It is actual interval -1 or frame 
  Parameters:
 	 Mode:
	  	VIM_MARB_INTJBUF_BY_LENGTH=0,
		VIM_MARB_INTJBUF_BY_FRAME=1
	Length: actual interval 
  Returns:
  	void
  Remarks:
*********************************************************************************/

void VIM_MARB_SetJbufInterval(VIM_MARB_INTERVAL_INTERRUPT_MODE Mode,UINT32 Length)
{
	UINT8 Temp;
	Temp=VIM_HIF_GetReg8(V5_REG_MARB_MODE_CTRL_0);
	Temp&=0x7f;
	Temp|=(Mode<<7);
	VIM_HIF_SetReg8(V5_REG_MARB_MODE_CTRL_0,Temp);
	if(!Mode)
	{
		VIM_HIF_SetReg8(V5_REG_MARB_JBUF_INTV_2,(UINT8)Length);
		VIM_HIF_SetReg8(V5_REG_MARB_JBUF_INTV_1,(UINT8)(Length>>8));
		VIM_HIF_SetReg8(V5_REG_MARB_JBUF_INTV_0,(UINT8)(Length>>16));
	}
	else
	{
		VIM_HIF_SetReg8(V5_REG_MARB_FRAME_INTERV,(UINT8)Length);
	}
}
/********************************************************************************
  Description:
	when encode mode ,when(write point - read point) > this value ,interrupt will be generated
	when decode mode,when(read point - write point) > this value ,interrupt will be generated
  Parameters:
	Length: actual interval 
  Returns:
  	void
  Remarks:
*********************************************************************************/

void VIM_MARB_SetJbufUpThreshold(UINT32 Length)
{
	VIM_HIF_SetReg8(V5_REG_MARB_JBUF_UP_2,(UINT8)Length);
	VIM_HIF_SetReg8(V5_REG_MARB_JBUF_UP_1,(UINT8)(Length>>8));
	VIM_HIF_SetReg8(V5_REG_MARB_JBUF_UP_0,(UINT8)(Length>>16));
}
/********************************************************************************
  Description:
	when encode mode ,when(write point - read point) > this value ,interrupt will be generated
	when decode mode,when(read point - write point) > this value ,interrupt will be generated
  Parameters:
	Length: actual interval 
  Returns:
  	void
  Remarks:
*********************************************************************************/

UINT32 VIM_MARB_GetJbufUpThreshold(void)
{
UINT32 Length;
	Length=(UINT32)(VIM_HIF_GetReg8(V5_REG_MARB_JBUF_UP_2));
	Length|=(UINT32)(VIM_HIF_GetReg8(V5_REG_MARB_JBUF_UP_1))<<8;
	Length|=(UINT32)(VIM_HIF_GetReg8(V5_REG_MARB_JBUF_UP_0))<<16;
return Length;
}
/********************************************************************************
  Description:
	when encode mode ,when(write point - read point) < this value ,interrupt will be generated
	when decode mode,when(read point - write point) <this value ,interrupt will be generated
  Parameters:
	Length: actual interval 
  Returns:
  	void
  Remarks:
*********************************************************************************/

void VIM_MARB_SetJbufLowThreshold(UINT32 Length)
{
	VIM_HIF_SetReg8(V5_REG_MARB_JBUF_LOW_2,(UINT8)Length);
	VIM_HIF_SetReg8(V5_REG_MARB_JBUF_LOW_1,(UINT8)(Length>>8));
	VIM_HIF_SetReg8(V5_REG_MARB_JBUF_LOW_0,(UINT8)(Length>>16));
}
/********************************************************************************
  Description:
	set jbuf memary start address
  Parameters:
	void
  Returns:
  	StartAdd: the start address of sram
  Remarks:
*********************************************************************************/

UINT32 VIM_MARB_GetJbufStartAddr(void)
{
       UINT32 Temp=0;
	Temp=(UINT32)(VIM_HIF_GetReg8(V5_REG_MARB_JBUF_START_2));
	Temp|=(UINT32)(VIM_HIF_GetReg8(V5_REG_MARB_JBUF_START_1))<<8;
	Temp|=(UINT32)(VIM_HIF_GetReg8(V5_REG_MARB_JBUF_START_0))<<16;
	return Temp;
}
/********************************************************************************
  Description:
	set jbuf memary end address
  Parameters:
	void
  Returns:
  	EndAdd: the end address of sram
  Remarks:
*********************************************************************************/

UINT32 VIM_MARB_GetJbufEndAddr(void)
{
UINT32 Temp=0;
	Temp=(UINT32)(VIM_HIF_GetReg8(V5_REG_MARB_JBUF_END_2));
	Temp|=(UINT32)(VIM_HIF_GetReg8(V5_REG_MARB_JBUF_END_1))<<8;
	Temp|=(UINT32)(VIM_HIF_GetReg8(V5_REG_MARB_JBUF_END_0))<<16;
	return Temp;
}
/********************************************************************************
  Description:
  	Read the read/wirte point of chip jbuf
  Parameters:
	void
  Returns:
  	void
  Remarks:
*********************************************************************************/
UINT32 VIM_MARB_GetJbufChipPointer(void)
{
UINT32 Temp=0;
	VIM_HIF_SetReg8(V5_REG_MARB_JBUF_PT_0,0);		//update
	Temp=(UINT32)VIM_HIF_GetReg8(V5_REG_MARB_JBUF_PT_2);
	Temp|=(UINT32)(VIM_HIF_GetReg8(V5_REG_MARB_JBUF_PT_1))<<8;
	Temp|=(UINT32)(VIM_HIF_GetReg8(V5_REG_MARB_JBUF_PT_0))<<16;
	return Temp;
}
/********************************************************************************
  Description:
  	Read the last jpeg point 
  Parameters:
	void
  Returns:
  	void
  Remarks:
*********************************************************************************/
UINT32 VIM_MARB_GetJpegChipLastPointer(void)
{
UINT32 Temp=0;
	VIM_HIF_SetReg8(V5_REG_MARB_LAST_PT_0,0);//update
	Temp=(UINT32)VIM_HIF_GetReg8(V5_REG_MARB_LAST_PT_2);
	Temp|=(UINT32)(VIM_HIF_GetReg8(V5_REG_MARB_LAST_PT_1))<<8;
	Temp|=(UINT32)(VIM_HIF_GetReg8(V5_REG_MARB_LAST_PT_0))<<16;
	return Temp;
}
/********************************************************************************
  Description:
	set thumbnail buf memary 
  Parameters:
	StartAdd: the start address of sram
	Size: the size of sram
  Returns:
  	void
  Remarks:
*********************************************************************************/

void VIM_MARB_SetThumbbufMem(UINT32 StartAdd,UINT32 Size)
{
	VIM_HIF_SetReg8(V5_REG_MARB_TBUF_START_2,(UINT8)StartAdd);
	VIM_HIF_SetReg8(V5_REG_MARB_TBUF_START_1,(UINT8)(StartAdd>>8));
	VIM_HIF_SetReg8(V5_REG_MARB_TBUF_START_0,(UINT8)(StartAdd>>16));
	
	VIM_HIF_SetReg8(V5_REG_MARB_TBUF_END_2,(UINT8)(StartAdd+Size));
	VIM_HIF_SetReg8(V5_REG_MARB_TBUF_END_1,(UINT8)((StartAdd+Size)>>8));
	VIM_HIF_SetReg8(V5_REG_MARB_TBUF_END_0,(UINT8)((StartAdd+Size)>>16));
}
/********************************************************************************
  Description:
	when encode mode ,when(write point - read point) > this value ,interrupt will be generated
	when decode mode,when(read point - write point) > this value ,interrupt will be generated
  Parameters:
	Length: actual interval 
  Returns:
  	void
  Remarks:
*********************************************************************************/

void VIM_MARB_SetThumbbufUpThreshold(UINT32 Length)
{
	VIM_HIF_SetReg8(V5_REG_MARB_TBUF_UP_2,(UINT8)Length);
	VIM_HIF_SetReg8(V5_REG_MARB_TBUF_UP_1,(UINT8)(Length>>8));
	VIM_HIF_SetReg8(V5_REG_MARB_TBUF_UP_0,(UINT8)(Length>>16));
}
/********************************************************************************
  Description:
	when encode mode ,when(write point - read point) < this value ,interrupt will be generated
	when decode mode,when(read point - write point) <this value ,interrupt will be generated
  Parameters:
	Length: actual interval 
  Returns:
  	void
  Remarks:
*********************************************************************************/

void VIM_MARB_SetThumbbufLowThreshold(UINT32 Length)
{
	VIM_HIF_SetReg8(V5_REG_MARB_TBUF_LOW_2,(UINT8)Length);
	VIM_HIF_SetReg8(V5_REG_MARB_TBUF_LOW_1,(UINT8)(Length>>8));
	VIM_HIF_SetReg8(V5_REG_MARB_TBUF_LOW_0,(UINT8)(Length>>16));
}
/********************************************************************************
  Description:
	set jbuf memary start address
  Parameters:
	void
  Returns:
  	StartAdd: the start address of sram
  Remarks:
*********************************************************************************/

UINT32 VIM_MARB_GetThumbbufStartAddr(void)
{
       UINT32 Temp=0;
	Temp=(UINT32)(VIM_HIF_GetReg8(V5_REG_MARB_TBUF_START_2));
	Temp|=(UINT32)(VIM_HIF_GetReg8(V5_REG_MARB_TBUF_START_1))<<8;
	Temp|=(UINT32)(VIM_HIF_GetReg8(V5_REG_MARB_TBUF_START_0))<<16;
	return Temp;
}
/********************************************************************************
  Description:
	set jbuf memary end address
  Parameters:
	void
  Returns:
  	EndAdd: the end address of sram
  Remarks:
*********************************************************************************/

UINT32 VIM_MARB_GetThumbbufEndAddr(void)
{
UINT32 Temp=0;
	Temp=(UINT32)(VIM_HIF_GetReg8(V5_REG_MARB_TBUF_END_2));
	Temp|=(UINT32)(VIM_HIF_GetReg8(V5_REG_MARB_TBUF_END_1))<<8;
	Temp|=(UINT32)(VIM_HIF_GetReg8(V5_REG_MARB_TBUF_END_0))<<16;
	return Temp;
}


/********************************************************************************
  Description:
	set index buf memary 
  Parameters:
	StartAdd: the start address of sram
	Size: the size of sram
  Returns:
  	void
  Remarks:
*********************************************************************************/

void VIM_MARB_SetIndexbufMem(UINT32 StartAdd,UINT32 Size)
{
	VIM_HIF_SetReg8(V5_REG_MARB_IDX_START_2,(UINT8)StartAdd);
	VIM_HIF_SetReg8(V5_REG_MARB_IDX_START_1,(UINT8)(StartAdd>>8));
	VIM_HIF_SetReg8(V5_REG_MARB_IDX_START_0,(UINT8)(StartAdd>>16));
	
	VIM_HIF_SetReg8(V5_REG_MARB_IDX_END_2,(UINT8)(StartAdd+Size));
	VIM_HIF_SetReg8(V5_REG_MARB_IDX_END_1,(UINT8)((StartAdd+Size)>>8));
	VIM_HIF_SetReg8(V5_REG_MARB_IDX_END_0,(UINT8)((StartAdd+Size)>>16));
}
/********************************************************************************
  Description:
  	Read the read/wirte point of chip thumb buf
  Parameters:
	void
  Returns:
  	void
  Remarks:
*********************************************************************************/
UINT32 VIM_MARB_GetThumbBufChipPointer(void)
{
UINT32 Temp=0;
	VIM_HIF_SetReg8(V5_REG_MARB_TBUF_PT_0,0);//update
	Temp=(UINT32)VIM_HIF_GetReg8(V5_REG_MARB_TBUF_PT_2);
	Temp|=(UINT32)(VIM_HIF_GetReg8(V5_REG_MARB_TBUF_PT_1))<<8;
	Temp|=(UINT32)(VIM_HIF_GetReg8(V5_REG_MARB_TBUF_PT_0))<<16;
	return Temp;
}

/********************************************************************************
  Description:
  	get the frame count have been captured when captuer video mode
  Parameters:
	void
  Returns:
  	void
  Remarks:
  	clear by start capture
*********************************************************************************/

UINT32 VIM_MARB_GetNowCapFrameCount(void) 
{
UINT32 Temp=0;
	Temp=(UINT32)VIM_HIF_GetReg8(V5_REG_MARB_FRAME_CNT_3);
	Temp|=(UINT32)(VIM_HIF_GetReg8(V5_REG_MARB_FRAME_CNT_2))<<8;
	Temp|=(UINT32)(VIM_HIF_GetReg8(V5_REG_MARB_FRAME_CNT_1))<<16;
	Temp|=(UINT32)(VIM_HIF_GetReg8(V5_REG_MARB_FRAME_CNT_0))<<24;
	return Temp;
}
/********************************************************************************
  Description:
  	get the status of marb status
  Parameters:
	void
  Returns:
  	void
  Remarks:
*********************************************************************************/

UINT8 VIM_MARB_GetStatus(void)
{
	return VIM_HIF_GetReg8(V5_REG_MARB_MARB_STAT_0);
}
/********************************************************************************
  Description:
  	set recapture information
  Parameters:
	UINT32 MaxLength;
		the max Length of jbuf.
	UINT8 MaxTime:
		the max times of recapture
  Returns:
  	void
  Remarks:
*********************************************************************************/

void VIM_MARB_SetRecaptureInfo(UINT8 MaxTimes,UINT32 MaxLength)
{
	MaxLength>>=2;
	VIM_HIF_SetReg8(V5_REG_MARB_RECAP_THRESHOLD,MaxTimes);
	VIM_HIF_SetReg8(V5_REG_MARB_RECAP_FRM_SIZE0,(MaxLength>>24));
	VIM_HIF_SetReg8(V5_REG_MARB_RECAP_FRM_SIZE1,(MaxLength>>16));
	VIM_HIF_SetReg8(V5_REG_MARB_RECAP_FRM_SIZE2,(MaxLength>>8));
	VIM_HIF_SetReg8(V5_REG_MARB_RECAP_FRM_SIZE3,MaxLength);
}
/********************************************************************************
  Description:
	return the recapture times
  Parameters:

  Returns:
  	void
  Remarks:
*********************************************************************************/

UINT8 VIM_MARB_GetRecaptureTime(void)
{
	return VIM_HIF_GetReg8(V5_REG_MARB_RECAP_TIMES);

}
/********************************************************************************
  Description:
  	set recapture enable
  Parameters:
	UINT8 Enable;
		ENABLE
		DISABLE
  Returns:
  	void
  Remarks:
*********************************************************************************/

void VIM_MARB_SetRecaptureEn(BOOL Enable)
{
UINT8 temp=0;
	temp|=Enable;
	temp|=(Enable<<1);
	VIM_HIF_SetReg8(V5_REG_MARB_RECAP_CTRL,temp);
}
/********************************************************************************
  Description:
  	set Timer Interval
  Parameters:
	Ms:Set the ms 
	mclk: the mclk of chip
  Returns:
  	void
  Remarks:
*********************************************************************************/

void VIM_MARB_SetTimerMs(UINT32 Ms,UINT32 mclk)
{
	UINT16 Num1Ms;
		Num1Ms=mclk/1000;// 1000 us=(1000000/mclk)(us)*Num
		VIM_HIF_SetReg8(V5_REG_MARB_SUB_TIMER1,(UINT8)Num1Ms);
		VIM_HIF_SetReg8(V5_REG_MARB_SUB_TIMER0,(UINT8)(Num1Ms>>8));
		VIM_HIF_SetReg8(V5_REG_MARB_TIMER2,(UINT8)Ms);
		VIM_HIF_SetReg8(V5_REG_MARB_TIMER1,(UINT8)(Ms>>8));
		VIM_HIF_SetReg8(V5_REG_MARB_TIMER2,(UINT8)(Ms>>16));
}
/********************************************************************************
  Description:
	start a timer in marb mooudle
	clear by hardware
  Parameters:
	void
  Returns:
  	void
  Remarks:
*********************************************************************************/
void VIM_MARB_StartTimer(void)
{
	VIM_HIF_SetReg8(V5_REG_MARB_MARB_CTRL,  BIT5);
}
/********************************************************************************
  Description:
	stop a timer in marb mooudle
	clear by hardware
  Parameters:
	void
  Returns:
  	void
  Remarks:
*********************************************************************************/
void VIM_MARB_StopTimer(void)
{
	VIM_HIF_SetReg8(V5_REG_MARB_MARB_CTRL,  BIT6);
}

void VIM_MARB_SetJpegbufGapratio(UINT8 ratio)
{
      UINT32 StartAddr, EndAddr,JbufLength;

	StartAddr = VIM_MARB_GetJbufStartAddr();
	EndAddr = VIM_MARB_GetJbufEndAddr()+4;
	
	JbufLength = EndAddr-StartAddr;  
	JbufLength *= ratio;
	JbufLength /= VIM_MARB_JBUF_MAXBLOCK;
	VIM_MARB_SetJbufUpThreshold(JbufLength + VIM_MARB_JBUF_GAPINTERVAL);
	VIM_MARB_SetJbufLowThreshold(JbufLength - VIM_MARB_JBUF_GAPINTERVAL);
}

void VIM_MARB_SetThumbbufGapratio(UINT8 ratio)
{
	UINT32 StartAddr, EndAddr,JbufLength;

	StartAddr = VIM_MARB_GetThumbbufStartAddr();
	EndAddr = VIM_MARB_GetThumbbufEndAddr();
	
	JbufLength = EndAddr-StartAddr;  
	JbufLength *= ratio;
	JbufLength /= VIM_MARB_JBUF_MAXBLOCK;
	VIM_MARB_SetThumbbufUpThreshold(JbufLength + VIM_MARB_JBUF_GAPINTERVAL);
	VIM_MARB_SetThumbbufLowThreshold(JbufLength - VIM_MARB_JBUF_GAPINTERVAL);
}
/********************************************************************************
  Description:
  	set marb maping list
  Parameters:
  Returns:
  	void
  Remarks:
*********************************************************************************/
VIM_RESULT VIM_MARB_SetMap(VIM_MARB_WORKMODE bWorkMode, VIM_DISP_BUFFERMODER Aformat,PVIM_MARB_Map Maping)
{
VIM_DISP_BFORMAT fmt;
TSize size;
UINT32 dwLength,dwLength1;
VIM_JPEG_YUVMODE YUVMode;
	
	//VIM_MARB_ResetSubModule(VIM_MARB_RESET_ALL);
	VIM_MARB_ResetSubModule((VIM_MARB_SUB_RESET_IPP|VIM_MARB_SUB_RESET_JPEG));		//guoying 6/11/2008//avoid when capture image producing the black line(points) in overlay mode
	
	// Set b 1 layer memary;
	Maping->layerB1end=V5_SRAM_TOTALSIZE;
	//printk("Maping->layerB1end: 0x%x\n",Maping->layerB1end);
	VIM_DISP_GetGbufFormat(&fmt);
	//printk("fmt: 0x%x\n",fmt);
	VIM_DISP_GetBSize(VIM_DISP_B1LAYER,&size);
	//printk("B1Layer size:cx-0x%x, cy-0x%x\n",size.cx,size.cy);
	dwLength=VIM_MARB_GetBmemSize(fmt,size);
	//printk("dwLength-0x%x\n",dwLength);
	if(Maping->layerB1end>dwLength)
		Maping->layerB1start=Maping->layerB1end-dwLength;	
	else
		return VIM_ERROR_B1_MAP;
	VIM_HIF_SetReg32(V5_REG_LCDC_GBBA1_L,Maping->layerB1start);
	
	// Set b 0 layer memary;
	Maping->layerB0end=Maping->layerB1start;
	//printk("Maping->layerB0end: 0x%x\n",Maping->layerB1start);
	VIM_DISP_GetGbufFormat(&fmt);
	VIM_DISP_GetBSize(VIM_DISP_B0LAYER,&size);
	//printk("B0Layer size:cx-0x%x, cy-0x%x\n",size.cx,size.cy);
	dwLength=VIM_MARB_GetBmemSize(fmt,size);
	//printk("dwLength-0x%x\n",dwLength);
	if(Maping->layerB0end>dwLength)
		Maping->layerB0start=Maping->layerB0end-dwLength;
	else
		return VIM_ERROR_B0_MAP;
	//printk("Maping->layerB0start-0x%x\n",Maping->layerB0start);

	VIM_HIF_SetReg32(V5_REG_LCDC_GBBA0_L,Maping->layerB0start);

	if(bWorkMode==VIM_MARB_DIRECTDISPLAY_MODE)
		return VIM_SUCCEED;
	
	// set a layer 
	Maping->layerA1end=Maping->layerB0start;
	//VIM_DISP_GetAMemorySize(&size);
	VIM_DISP_GetAWinsize(&size);//2006-3-7

/////////////////////////////////////////////////////////////////////////////////////
#if VIM_USER_SUPPORT_REALTIME_ROTATION
	if(bWorkMode == VIM_MARB_DISPLAYSTILL_MODE)
	{
		//size.cx = 320;
		size.cx = VIM_HIF_GetReg16(V5_REG_LCDC_BGW_L);
		//size.cy = 240;
		size.cy = VIM_HIF_GetReg16(V5_REG_LCDC_BGH_L);
	}
#endif
/////////////////////////////////////////////////////////////////////////////////////
	
	if((VIM_HIF_GetReg8(V5_REG_LCDC_RM)&0x7)%2)	//angela 2006-8-14
	{
		size.cx=size.cy;
		size.cy=VIM_HIF_GetReg16(V5_REG_LCDC_AW_L)+1;
	}
	dwLength = VIM_MARB_GetAmemSize(Aformat, size);
	if(dwLength>Maping->layerA1end)
		return VIM_ERROR_ALAYER_MAP;
	Maping->layerA1start = Maping->layerA1end- (dwLength >> 1);
	Maping->layerA0end = Maping->layerA1start;
	Maping->layerA0start = Maping->layerA1end -dwLength;
	VIM_DISP_SetBufferMode((VIM_DISP_BUFFERMODER)Aformat);
	VIM_HIF_SetReg32(V5_REG_LCDC_VBBA0_L,Maping->layerA0start);
	VIM_HIF_SetReg32(V5_REG_LCDC_VBBA1_L,Maping->layerA1start);
	
	// set index buf
	Maping->indexstart=Maping->layerA0start;
	if(bWorkMode==VIM_MARB_CAPTUREAVI_MODE)
	{
		if((Maping->indexsize==0)||(Maping->indexsize>Maping->layerA0start))
			return VIM_ERROR_INDEX_MAP;
		Maping->indexstart=Maping->layerA0start-Maping->indexsize;
	}
	else
		Maping->indexsize=0;
	VIM_MARB_SetIndexbufMem(Maping->indexstart,Maping->indexsize);

	// set thumbnail buf
	Maping->thumbstart=Maping->indexstart;
	if(bWorkMode==VIM_MARB_CAPTURESTILLTHUMB_MODE)
	{
		VIM_IPP_GetThumbSize(&size);
		dwLength=VIM_MARB_GetBmemSize(VIM_DISP_BLAYER_RGB565,size);
		if(!dwLength)
			return VIM_ERROR_THUMB_MAP;
		Maping->thumbsize=dwLength+4;
		if(Maping->thumbsize>Maping->indexstart)
			return VIM_ERROR_THUMB_MAP;
		Maping->thumbstart=Maping->indexstart-Maping->thumbsize;
	}
	else
		Maping->thumbsize=0;
	VIM_MARB_SetThumbbufMem(Maping->thumbstart,Maping->thumbsize);
	
	// set line buf
	Maping->lbuf1start=Maping->lbufend=Maping->thumbstart;
	//line buf 0
	YUVMode=VIM_JPEG_GetYUVMode();
	//size.cx=VIM_JPEG_LineGetWidth();
	VIM_JPEG_GetSize(&size);//2006-3-7
	dwLength=VIM_MARB_GetDecLbuf0Size(YUVMode,size.cx);
	if(bWorkMode<=VIM_MARB_ENCODE_MODE)
		dwLength1=dwLength;
	else if((bWorkMode==VIM_MARB_DISPLAYSTILL_MODE)||
		(bWorkMode==VIM_MARB_DISPLAYVIDEO_MODE))
		dwLength1=0;
	else  //decode
	{
		VIM_IPP_GetDispalyWindowSize(&size.cx,&size.cy);
		dwLength1=VIM_MARB_GetDecLbuf1Size(YUVMode,size.cx);
	}
	if((dwLength1+dwLength)>(Maping->lbufend))
		return VIM_ERROR_LINEBUF_MAP;
	Maping->lbuf1start=Maping->lbufend-dwLength1;
	Maping->lbuf0start=Maping->lbuf1start-dwLength;
	dwLength=(Maping->lbuf0start)>>2;
	VIM_HIF_SetReg8(V5_REG_LBUF_INI_ADDR_L,(UINT8)dwLength);
	VIM_HIF_SetReg8(V5_REG_LBUF_INI_ADDR_M,(UINT8)(dwLength>>8));
	VIM_HIF_SetReg8(V5_REG_LBUF_INI_ADDR_H,(UINT8)(dwLength>>16));
       
	// set jbuf
	Maping->jbufstart=0;
	Maping->jbufsize=Maping->lbuf0start;
	if(Maping->jbufsize<VIM_MARB_JBUF_MINSIZE)
		return VIM_ERROR_JBUF_MAP;
	VIM_MARB_SetJbufMem(Maping->jbufstart,Maping->jbufsize-4);

	// set JBUF up thur
	dwLength=Maping->jbufsize;
	dwLength=dwLength*Maping->jbufnowblock_int/VIM_MARB_JBUF_MAXBLOCK;
	VIM_MARB_SetJbufUpThreshold(dwLength+VIM_MARB_JBUF_GAPINTERVAL);
	VIM_MARB_SetJbufLowThreshold(dwLength-VIM_MARB_JBUF_GAPINTERVAL);
	VIM_MARB_Set1TSramMode(VIM_MARB_1TSRAM_ALL,VIM_MARB_1TSRAM_POWERON);
#if(VIM_MARB_DEBUG)&&(VIM_528RDK_DEBUG)
	VIM_USER_PrintHex(" the b0 size is   ",Maping->layerB0end-Maping->layerB0start);
	VIM_USER_PrintHex(" the b1 size is   ",Maping->layerB1end-Maping->layerB1start);

	VIM_USER_PrintHex(" the line size is   ",Maping->lbufend-Maping->lbuf0start);
	VIM_USER_PrintHex(" a size is   ",Maping->layerA1end-Maping->layerA0start);
	VIM_USER_PrintHex(" thumb nail size    ",Maping->thumbsize);
	VIM_USER_PrintHex(" JPUF size    ",Maping->jbufsize);
#endif


	return VIM_SUCCEED;
}

/********************************************************************************
  Description:
  	interrupt of marb
  Parameters:
  Returns:
  	void
  Remarks:
*********************************************************************************/
static const UINT8 gMarbIntPriority[8]	= 
{ 
	MARB_INT_JBUF_ERR, MARB_INT_THUMB_ERR, MARB_INT_JBUF_INTERV, MARB_INT_JBUF_FIFOCNT, 
	MARB_INT_JPEG_DONE, MARB_INT_TBUF_FIFOCNT,  MARB_INT_THUMB_DONE, MARB_INT_TBUF_INTERV
};
void _ISR_MarbIntHandle(UINT8 wIntFlag)
{
	UINT8 i = 0;
	UINT8 bySecondLevelInt = 0;

	for(i=0; i<8; i++)
	{
		bySecondLevelInt = gMarbIntPriority[i];
		if(TSTINT(wIntFlag, bySecondLevelInt))
		{
			if(gVc0528_Isr.marbisr[bySecondLevelInt])
			{
				gVc0528_Isr.marbisr[bySecondLevelInt]();
			}
		}
	}
}
static const UINT8 gMarbSlvIntPriority[MARB_SLV_INT_ALL]	= 
{ 
	MARB_INT_MEM0_REF_ERR, MARB_INT_RECAP, MARB_INT_MEM0_REF_ERR, MARB_INT_MEM1_REF_ERR, MARB_INT_MEM2_REF_ERR
};
void _ISR_Marb1IntHandle(UINT8 wIntFlag)
{
	UINT8 i = 0;
	UINT8 bySecondLevelInt = 0;

	for(i=0; i<MARB_SLV_INT_ALL; i++)
	{
		bySecondLevelInt = gMarbSlvIntPriority[i];
		if(TSTINT(wIntFlag, (bySecondLevelInt-8)))// real value about marb additional
		{

			if(gVc0528_Isr.marbisr[bySecondLevelInt])
			{
#if(VIM_HIF_DEBUG)&&(VIM_528RDK_DEBUG)
				printk("\n && INT_MARBADD _ISR_Marb1IntHandle i:%02d, sec_level:%02d\n",i,bySecondLevelInt);
#endif
				gVc0528_Isr.marbisr[bySecondLevelInt]();
			}
		}
	}
}
/********************************************************************************
  Description:
  	register isr 
  Parameters:
  	bySecondLevelInt:
  			MARB_INT_JBUF_FIFOCNT		= 0,
			MARB_INT_JBUF_RESERV		= 1, //ignore
			MARB_INT_JPEG_DONE			= 2,
			MARB_INT_JBUF_ERR			= 3,
			MARB_INT_TBUF_FIFOCNT		= 4,
			MARB_INT_TBUF_RESERV		= 5, //ignore
			MARB_INT_THUMB_DONE		= 6,
			MARB_INT_THUMB_ERR			= 7,
			MARB_INT_MEM0_REF_ERR		= 8,
			MARB_INT_MEM1_REF_ERR		= 9, //ignore
			MARB_INT_MEM2_REF_ERR		= 10,
			MARB_INT_RECAP_ERROR		=11,
			MARB_INT_RECAP				=12,
			MARB_INT_TIMER				=13,
			MARB_INT_ALL			= VIM_MARB_INT_NUM
  Returns:
  	void
  Remarks:
*********************************************************************************/

void VIM_MARB_RegisterISR(VIM_MARB_INT_TYPE bySecondLevelInt, PVIM_Isr  Isr)
{
UINT8 i;
	if(bySecondLevelInt>=MARB_INT_ALL)
	{
		for(i=0;i<MARB_INT_ALL;i++)
			gVc0528_Isr.marbisr[i] = Isr;
		return;
	}
	gVc0528_Isr.marbisr[bySecondLevelInt] = Isr;
}
/********************************************************************************
  Description:
  	set enable marb interrupt
  Parameters:
  	bySecondLevelInt:
  			MARB_INT_JBUF_FIFOCNT		= 0,
			MARB_INT_JBUF_RESERV		= 1, //ignore
			MARB_INT_JPEG_DONE			= 2,
			MARB_INT_JBUF_ERR			= 3,
			MARB_INT_TBUF_FIFOCNT		= 4,
			MARB_INT_TBUF_RESERV		= 5, //ignore
			MARB_INT_THUMB_DONE		= 6,
			MARB_INT_THUMB_ERR			= 7,
			MARB_INT_MEM0_REF_ERR		= 8,
			MARB_INT_MEM1_REF_ERR		= 9, //ignore
			MARB_INT_MEM2_REF_ERR		= 10,
			MARB_INT_RECAP_ERROR		=11,
			MARB_INT_RECAP				=12,
			MARB_INT_TIMER				=13,
			MARB_INT_ALL			= VIM_MARB_INT_NUM
  Returns:
  	void
  Remarks:
*********************************************************************************/
void VIM_MARB_SetIntEnable(VIM_MARB_INT_TYPE bySecondLevelInt,BOOL Eanble)
{
	UINT8 bInten;
	if(bySecondLevelInt<MARB_INT_MEM0_REF_ERR)
	{
		bInten=VIM_HIF_GetIntEnableSec(INT_MARB);
		if(Eanble==ENABLE)
			bInten |= (0x1<<bySecondLevelInt);
		else
			bInten &= (~(0x1<<bySecondLevelInt));	
		VIM_HIF_SetIntEnableSec(INT_MARB,bInten);
		if(bInten)
			VIM_HIF_SetIntModuleEn(INT_MARB,ENABLE);
		else
			VIM_HIF_SetIntModuleEn(INT_MARB,DISABLE);
	}
	else if(bySecondLevelInt<MARB_INT_ALL)
	{
		bInten=VIM_HIF_GetIntEnableSec(INT_MARBADD);
		if(Eanble==ENABLE)
			bInten |= (0x1<<(bySecondLevelInt-8));
		else
			bInten &= (~(0x1<<(bySecondLevelInt-8)));	
		VIM_HIF_SetIntEnableSec(INT_MARBADD,bInten);
		if(bInten)
			VIM_HIF_SetIntModuleEn(INT_MARBADD,ENABLE);
		else
			VIM_HIF_SetIntModuleEn(INT_MARBADD,DISABLE);
	}
	else
	{
		VIM_HIF_SetIntEnableSec(INT_MARBADD,0xff);
		VIM_HIF_SetIntEnableSec(INT_MARB,0xff);
		VIM_HIF_SetIntModuleEn(INT_MARB,ENABLE);
		VIM_HIF_SetIntModuleEn(INT_MARBADD,ENABLE);
	}
}

/********************************************************************************
Description:
	MARB get jbuf  
	start address is 0
	end  get from register
parameters: 
	Jbufpos: host point
Return:
     	void
Remarks:
*********************************************************************************/
UINT32 VIM_MARB_GetJbufRWSize(UINT32 Jbufpos)
{
	UINT32 dwEndAddr; 
	UINT32 dwChipPoint, dwHostPoint;
	
	dwHostPoint=Jbufpos;
	dwEndAddr=VIM_MARB_GetJbufEndAddr()+4;
	dwChipPoint = VIM_MARB_GetJbufChipPointer();


#if(VIM_528RDK_DEBUG)&&(VIM_MARB_DEBUG)
		VIM_USER_PrintDec("\n dwChipPoint=",dwChipPoint);
		VIM_USER_PrintDec("\n dwHostPoint=",dwHostPoint);
		VIM_USER_PrintDec("\n dwEndAddr=",dwEndAddr);
#endif
	//if read position more than write position then error
	//decode
	if(VIM_MARB_GetModeCtrl0() & BIT2)
	{
		if(dwHostPoint < dwChipPoint)
			return dwChipPoint - dwHostPoint;
	}
	//encode
	else	{
		if(dwHostPoint <= dwChipPoint)
			return dwChipPoint - dwHostPoint;
	}
	//return free buffer size
	return dwEndAddr - dwHostPoint + dwChipPoint ;

}

/***************************************************************************************************
Description:
	MARB write jpeg data
	start address is 0
	end  get from register
parameters: 
	*Start:       jpeg data start position
	length:       data length
	pos:		   write position
Return:
		VIM_SUCCEED: cacaulte ok
		VIM_ERROR_BUFLENS: buffer length error
Remarks:
*****************************************************************************************************/
UINT32 VIM_MARB_WriteJpegData(HUGE UINT8 *Start,UINT32 length, UINT32 pos,UINT8 end)
{
	UINT32 dwEndAddr=VIM_MARB_GetJbufEndAddr()+4;
	UINT32 writepos;//,marblen;
	UINT8 dat[4]={0xd9,0xff,0xff,0xd9};
	
	//marblen = VIM_MARB_GetJbufRWSize(pos);
	if(!end)
	{
		writepos=VIM_MARB_WriteBuf(Start, length,0,dwEndAddr,pos);
		return writepos;
	}
	else 
	{
		if(length&0x1)	//because multi16 write data by even, so when file end length maybe is odd
		{
#if(VIM_MARB_DEBUG)&&(VIM_528RDK_DEBUG)
			VIM_USER_PrintDec("\n length : ", (length&0x1));
#endif
			dat[0]=Start[length-1];	
			length=length-1;
		}
		writepos=VIM_MARB_WriteBuf(Start, length,0,dwEndAddr,pos);

		//if(gVc0528_JpegInfo.frmEnd)		//angela 2007-2-25
		{
			writepos=VIM_MARB_WriteBuf(dat, 4,0,dwEndAddr,writepos);
#if(VIM_MARB_DEBUG)&&(VIM_528RDK_DEBUG)
			VIM_USER_PrintHex("\n WriteSram : ", 4);
#endif
		}
		return writepos;
	}
	
}
/***************************************************************************************************
Description:
	MARB read jpeg data
	start address is 0
	end  get from register
parameters: 
	*Start:       jpeg data start position
	length:       data length
	pos:		   write position
Return:
		VIM_SUCCEED: cacaulte ok
		VIM_ERROR_BUFLENS: buffer length error
Remarks:
*****************************************************************************************************/
UINT32 VIM_MARB_ReadJpegData(HUGE UINT8 *Start, UINT32 length, UINT32 pos)
{
	UINT32 dwEndAddr=VIM_MARB_GetJbufEndAddr()+4;
	if(Start==NULL)
		return pos;
	pos=VIM_MARB_ReadBuf(Start, length, 0, dwEndAddr, pos);
	return pos;
}
/***************************************************************************************************
Description:
	MARB read thumb data
	start address is 0
	end  get from register
parameters: 
	*Start:       thumb data start position
	length:       data length
	pos:		   write position
Return:
		VIM_SUCCEED: cacaulte ok
		VIM_ERROR_BUFLENS: buffer length error
Remarks:
*****************************************************************************************************/
UINT32 VIM_MARB_ReadThumbData(HUGE UINT8 *Start, UINT32 length, UINT32 pos)
{
	UINT32 dwEndAddr=VIM_MARB_GetThumbbufEndAddr()+4;
	if(Start==NULL)
		return pos;
	pos=VIM_MARB_ReadBuf(Start, length, 0, dwEndAddr, pos);
	return pos;
}
