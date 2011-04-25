/*************************************************************************
*                                                                       
*                Copyright (C) 2005 Vimicro CO.,LTD     		 
*
* FILE NAME                                  VERSION                
*                                                                       
* VIM_IPP_Driver.c			   0.1                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*     5X 's ipp line buf moudle sub driver                                 
*
*	Version		Author			Date		Description		
*  ---------------------------------------------------------------						 
*   0.1			angela		2005-11-8	The first version. 
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


///////////////////////////////tools for ipp caculate start/////////////////////////////

static UINT16	VIM_IPP_MaxDivisor(UINT16 a, UINT16 b)
{
	while(a && b)
	{
		if(a > b)
			a %= b;
		else
			b %= a;
	}
	if(a)
		return a;
	return b;
}
static void VIM_IPP_CalcSizerDownFactor(UINT16 src, UINT16 dst, PVIM_IPP_SzDnFct pfct)
{
	UINT32 tmps = 0, tmpd = 0, maxdiv = 0;

	maxdiv = VIM_IPP_MaxDivisor(src, dst);
	tmps = src / maxdiv;
	tmpd = dst / maxdiv;
	if(tmps > 0x80)
	{
		tmpd = (tmpd << 7) / tmps + 1;
		tmps = 0x80;
	}
	pfct->w1 = (UINT8)(tmps & 0x7f);
	pfct->w2 = (UINT8)(tmpd & 0x7f);
	pfct->s1 = (UINT16)(tmps / tmpd);
	pfct->s2 = (UINT16)(pfct->s1 + ((tmps % tmpd) ? 1 : 0));
	pfct->s1 = (UINT16)(0x10000 / pfct->s1);
	pfct->s2 = (UINT16)(0x10000 / pfct->s2);
	pfct->c  = (UINT8)(tmpd - (tmps % tmpd));
}

static void VIM_IPP_CalcSizerUpFactor(UINT16 src, UINT16 dst, PVIM_IPP_SzUpFct pfct)
{
	UINT32 tmps = 0, tmpd = 0, maxdiv = 0;

	maxdiv = VIM_IPP_MaxDivisor(src, dst);
	tmps = src / maxdiv;
	tmpd = dst / maxdiv;
	if(tmpd > 0x100)
	{
		tmps = (tmps << 8) / tmpd;
		tmpd = 0x100;
	}
	if(tmps == tmpd)
		tmps --;
	pfct->w1 = (UINT8)tmps;
	pfct->w2 = (UINT8)tmpd;
	pfct->s  = (UINT16)(0x10000 / tmpd);
}
///////////////////////////////tools for ipp caculate end/////////////////////////////

/********************************************************************************
	Description:
		IPP reset 
	Parameters:
		byEffect:	special control setting
	Note:
	Remarks:
*********************************************************************************/

void VIM_IPP_Reset(void)
{
UINT8 Temp;
	VIM_USER_DelayMs(VIM_IPP_DOWNTOUP_DELAY);
	Temp = VIM_HIF_GetReg8(V5_REG_IPP_SIZCTRL0);
	// Reset all the internal registers except control registers
	VIM_HIF_SetReg8(V5_REG_IPP_SIZCTRL0, Temp | BIT4);
	VIM_USER_DelayMs(1);
	// Restore reset bit in V5_REG_IPP_SIZCTRL register.
	VIM_HIF_SetReg8(V5_REG_IPP_SIZCTRL0, Temp & (~BIT4));
}
/********************************************************************************
	Description:
		set ipp and line buf work mode
	Parameters:
		byMode:	work mode
			VIM_JPEG_MODE_PREVIEW=0,
			VIM_JPEG_MODE_CAPTURE,
			VIM_JPEG_MODE_DISPLAY,
			VIM_JPEG_MODE_DECODE,
			VIM_JPEG_MODE_ENCODE
		HaveFrame:
			VIM_IPP_HAVEFRAME = 0x1,
			VIM_IPP_HAVE_NOFRAME=0X0;
	Returns:
	Remarks:
*********************************************************************************/
void VIM_IPP_SetMode(VIM_IPP_MODE bMode,VIM_IPP_HAVEFRAM bHaveFrame)
{
	UINT8 bTempIpp,bSizerCtrl;
	UINT8 bLbufYUVmode=0,bLineMode=0;

	//VIM_IPP_Reset();//angela discard it for zoom out problem ,ipp reset must be used in one frame stream have passed ipp
	
	VIM_SIF_StartCaptureEnable(DISABLE);
	//ready set lbuf working mode, bit0-2 is working mode 
	bLineMode=VIM_HIF_GetReg8(V5_REG_LBUF_LBUF_MODE);
	bLineMode&=0xe0;	
	// Line buf select by lcdc  or ipp, bit 3=1 yuv data from lcdc
	bLbufYUVmode=VIM_HIF_GetReg8(V5_REG_LBUF_YUV_MODE);
	bLbufYUVmode&=0xf7;
	bLbufYUVmode|=(bHaveFrame<<3);
	//ready set ipp working mode, bit5-7 is working mode
	bTempIpp=VIM_HIF_GetReg8(V5_REG_IPP_SPECTRL);
	bTempIpp&=0x9f;
	//selete ipp image data from lbuf/sif/isp bit 5-6 is data select
	bSizerCtrl=VIM_HIF_GetReg8(V5_REG_IPP_SIZCTRL0);
	bSizerCtrl&=0x16;					//thumbnail sizer disanble
	bSizerCtrl|=(bHaveFrame<<7);	//no frame or have frame

	switch(bMode)
		{
		case VIM_IPP_MODE_PREVIEW:
			bTempIpp|=(0x01<<5);
			bSizerCtrl|=(1<<5);		// select from sif 
			bSizerCtrl|=(0x1<<3);		// display sizer enable
			bLineMode|=01;			//set line buf capture mode
			break;
		case VIM_IPP_MODE_CAPTURE:
			bTempIpp|=(0x01<<5);
			bSizerCtrl|=(1<<5);		// select from  sif 
			bSizerCtrl|=(0x1<<3);		// display sizer enable
			bLineMode|=01;			//set line buf capture mode
			break;
		case VIM_IPP_MODE_DISPLAY:
			bTempIpp|=(3<<5);		// set ipp display mode
			bSizerCtrl|=(0<<5);		// select from line buf
			bSizerCtrl|=(0x1<<3);		// display sizer enable
			bSizerCtrl&=(~(BIT1|BIT2));//disable capture sizer
			bLineMode|=02;			//set line buf display mode
			break;
		case VIM_IPP_MODE_DECODE:
			bTempIpp|=(2<<5);		//set ipp decode mode
			bSizerCtrl|=(0<<5);		// select from line buf
			bSizerCtrl|=(0x1<<3);		// display sizer enable
			bSizerCtrl&=(~(BIT1|BIT2));//disable capture sizer
			bLineMode|=04;			//set line buf decode mode 100
			if (bHaveFrame)
				bLineMode|=0x2<<3;	//data pass to ipp,lcdc, marb read data from line buffer 1
			else
				bLineMode|=0x1<<3;	//data pass to ipp unit,marb read data from line buffer 1
			break;
		case VIM_IPP_MODE_CAPTURE_WITHTHUMB:
			bTempIpp|=(0x01<<5);
			bSizerCtrl|=(1<<5);		// select from isp
			bSizerCtrl|=1;			// thumb nail sizer enable
			bLineMode|=01;			//set line buf capture mode
			VIM_HIF_SetReg8(V5_REG_IPP_TSIZER_OUT,1);//out for rgb
			break;
		default:
			break;
		}
	//line buffer working mode set 
	VIM_HIF_SetReg8(V5_REG_LBUF_LBUF_MODE,bLineMode);
	//set YUV data form LCD(with frame) / from IPP(no frame)
	VIM_HIF_SetReg8(V5_REG_LBUF_YUV_MODE,bLbufYUVmode);
	//set IPP working mode
	VIM_HIF_SetReg8(V5_REG_IPP_SPECTRL,bTempIpp);
	//set data form, and with frame or not
	VIM_HIF_SetReg8(V5_REG_IPP_SIZCTRL0,bSizerCtrl);
}
/********************************************************************************
	Description:
		set special control 
	Parameters:
		Effect:	special control setting
			VIM_IPP_EFFECT_NORMAL = 0x0,
			VIM_IPP_EFFECT_SEPHIA = 0x1,
			VIM_IPP_EFFECT_SPECIALCOLOR = 0x3,
			VIM_IPP_EFFECT_NEGATIVE = 0x5,
			VIM_IPP_EFFECT_SKETCH = 0x7
	Note:

	Remarks:
*********************************************************************************/
void VIM_IPP_SetEffect(UINT8 bEffect)
{
UINT8 bTemp;
	bTemp=(VIM_HIF_GetReg8(V5_REG_IPP_SPECTRL)&0xE0);
	bTemp|=bEffect;
	VIM_HIF_SetReg8(V5_REG_IPP_SPECTRL,bTemp);
}
/********************************************************************************
	Description:
		Set the UV offset of uv sephia effect only
	Parameters:
		bUoffset: the offset of U
		bVoffset:the offset of V
	Remarks:
*********************************************************************************/

void VIM_IPP_SetEffectUVoffset(UINT8 bUoffset,UINT8 bVoffset)
{
	VIM_HIF_SetReg8(V5_REG_IPP_UOFFSET,bUoffset);
	VIM_HIF_SetReg8(V5_REG_IPP_VOFFSET,bVoffset);
}
/********************************************************************************
	Description:
		Set the UV offset of uv sephia effect only
	Parameters:
		bUoffset: the offset of U
		bVoffset:the offset of V
	Remarks:
*********************************************************************************/

void VIM_IPP_SetEffectUVThreshold(UINT8 bU_up,UINT8 bU_down,UINT8 bV_up,UINT8 bV_down)
{
	VIM_HIF_SetReg8(V5_REG_IPP_UUPTHRS,bU_up);
	VIM_HIF_SetReg8(V5_REG_IPP_UDWTHRS,bU_down);
	VIM_HIF_SetReg8(V5_REG_IPP_VUPTHRS,bV_up);
	VIM_HIF_SetReg8(V5_REG_IPP_VDWTHRS,bV_down);
}

/********************************************************************************
	Description:
		select caputre down size or caputre up size
	Parameters:
		byEffect:	special control setting
	Note:
	Remarks:
*********************************************************************************/

void VIM_IPP_CaputreSizeSelect(VIM_IPP_CAPSIZE_MODE bMode)
{
UINT8 Temp;
	Temp = VIM_HIF_GetReg8(V5_REG_IPP_SIZCTRL0);
	Temp&=0xf9;
	Temp|=bMode;
	VIM_HIF_SetReg8(V5_REG_IPP_SIZCTRL0,Temp);
}
/********************************************************************************
	Description:
		select caputre down size or caputre up size
	Parameters:
		byEffect:	special control setting
	Note:
	Remarks:
*********************************************************************************/

VIM_IPP_CAPSIZE_MODE VIM_IPP_GetCaputreSizeSelect(void)
{
UINT8 Temp;
	Temp = VIM_HIF_GetReg8(V5_REG_IPP_SIZCTRL0);
	Temp&=0x06;
	return (VIM_IPP_CAPSIZE_MODE)(Temp);
}
/********************************************************************************
	Description:
		Config input image size in IPP module
	Parameters:
		wWidth:		input image width to IPP module
		wHeight:		input image height to IPP module
	Remarks:
*********************************************************************************/
void VIM_IPP_SetImageSize(UINT16 wWidth, UINT16 wHeight)
{

	VIM_HIF_SetReg16(V5_REG_IPP_IMGWD, wWidth);
	VIM_HIF_SetReg16(V5_REG_IPP_IMGHT, wHeight);

	wWidth=VIM_HIF_GetReg16(V5_REG_IPP_IMGWD);
	wHeight=VIM_HIF_GetReg16(V5_REG_IPP_IMGHT);
	wHeight = VIM_HIF_GetReg16(V5_REG_IPP_DISHT);	//update window 
	VIM_HIF_SetReg16(V5_REG_IPP_DISHT,wHeight);
}


void VIM_IPP_GetImageSize(UINT16 *wWidth, UINT16 *wHeight)
{
	*wWidth = VIM_HIF_GetReg16(V5_REG_IPP_IMGWD);
	*wHeight = VIM_HIF_GetReg16(V5_REG_IPP_IMGHT);
}

/********************************************************************************
	Description:
		Config sizer window size in IPP module
	Parameters:
		wWidth:		sizer window width to IPP module
		wHeight:	sizer window height to IPP module
	Remarks:
*********************************************************************************/
void VIM_IPP_SetWindowSize(UINT16 wWidth, UINT16 wHeight)
{
	VIM_HIF_SetReg16(V5_REG_IPP_WINWD, wWidth);
	VIM_HIF_SetReg16(V5_REG_IPP_WINHT, wHeight);
}


void VIM_IPP_GetWindowSize(UINT16 *wWidth, UINT16 *wHeight)
{
	*wWidth = VIM_HIF_GetReg16(V5_REG_IPP_WINWD);
	*wHeight = VIM_HIF_GetReg16(V5_REG_IPP_WINHT);
}
/********************************************************************************
	Description:
		Config the X, Y coordinate of sizer's start pixel in IPP module
	Parameters:
		wStartx:	X coordinate of sizer's start pixel
		wStarty:	Y coordinate of sizer's start pixel
	Remarks:
*********************************************************************************/
void VIM_IPP_SetWindowPos(UINT16 wStart_X, UINT16 wStart_Y)
{
	VIM_HIF_SetReg16(V5_REG_IPP_WINSTX, wStart_X);
	VIM_HIF_SetReg16(V5_REG_IPP_WINSTY, wStart_Y);
}


void VIM_IPP_GetWindowPos(UINT16 *wStart_X, UINT16 *wStart_Y)
{
	*wStart_X = VIM_HIF_GetReg16(V5_REG_IPP_WINSTX);
	*wStart_Y = VIM_HIF_GetReg16(V5_REG_IPP_WINSTY);
}

/********************************************************************************
	Description:
		Config sizer window after display  size in IPP module
	Parameters:
		wWidth:	sizer window width 
		wHeight:	sizer window height
	Remarks:
*********************************************************************************/
void VIM_IPP_SetDispalyWindowSize(UINT16 wWidth, UINT16 wHeight)
{
	VIM_HIF_SetReg16(V5_REG_IPP_DISWD, wWidth);
	VIM_HIF_SetReg16(V5_REG_IPP_DISHT, wHeight);
}


void VIM_IPP_GetDispalyWindowSize(UINT16 *wWidth, UINT16 *wHeight)
{
	*wWidth = VIM_HIF_GetReg16(V5_REG_IPP_DISWD);
	*wHeight = VIM_HIF_GetReg16(V5_REG_IPP_DISHT);
}
/********************************************************************************
	Description:
		enable the ipp sizer bypass
	Parameters:
		bBypassbit: bypass sizer 
			VIM_IPP_BYPASS_TUMB_H=BIT0,
			VIM_IPP_BYPASS_TUMB_W=BIT1,
			VIM_IPP_BYPASS_DIS_H=BIT2,
			VIM_IPP_BYPASS_DIS_W=BIT3,
			VIM_IPP_BYPASS_CAP_UP_H=BIT4,
			VIM_IPP_BYPASS_CAP_UP_W=BIT5,
			VIM_IPP_BYPASS_CAP_DOWN_H=BIT6,
			VIM_IPP_BYPASS_CAP_DOWN_W=BIT7,
	Remarks:
*********************************************************************************/

void VIM_IPP_SetSizeBypassEnable(UINT8 bBypassbit)
{
       UINT8 bTemp;
	bTemp=VIM_HIF_GetReg8(V5_REG_IPP_SIZCTRL1);
	bTemp|=bBypassbit;
	VIM_HIF_SetReg8(V5_REG_IPP_SIZCTRL1,bTemp);
}
/********************************************************************************
	Description:
		enable the ipp sizer bypass
	Parameters:
		bBypassbit: bypass sizer 
			VIM_IPP_BYPASS_TUMB_H=BIT0,
			VIM_IPP_BYPASS_TUMB_W=BIT1,
			VIM_IPP_BYPASS_DIS_H=BIT2,
			VIM_IPP_BYPASS_DIS_W=BIT3,
			VIM_IPP_BYPASS_CAP_UP_H=BIT4,
			VIM_IPP_BYPASS_CAP_UP_W=BIT5,
			VIM_IPP_BYPASS_CAP_DOWN_H=BIT6,
			VIM_IPP_BYPASS_CAP_DOWN_W=BIT7,
	Remarks:
*********************************************************************************/

void VIM_IPP_SetSizeBypassDisable(UINT8 bBypassbit)
{
      UINT8 bTemp;
	bTemp=VIM_HIF_GetReg8(V5_REG_IPP_SIZCTRL1);
	bTemp&=(~bBypassbit);
	VIM_HIF_SetReg8(V5_REG_IPP_SIZCTRL1,bTemp);
}
/********************************************************************************
	Description:
		Set the down factor of down width display sizer 
	Parameters:
		factor: the factor w1 ;w2 ;s1; s2;c 
	Remarks:
*********************************************************************************/

void VIM_IPP_SetDisplaySizerWFactor(PVIM_IPP_SzDnFct factor)
{
	VIM_HIF_SetReg8(V5_REG_IPP_DISH_W1,factor->w1);
	VIM_HIF_SetReg8(V5_REG_IPP_DISH_W2,factor->w2);
	VIM_HIF_SetReg16(V5_REG_IPP_DISH_S1,factor->s1);
	VIM_HIF_SetReg16(V5_REG_IPP_DISH_S2,factor->s2);
	VIM_HIF_SetReg8(V5_REG_IPP_DISH_C,factor->c);
}
/********************************************************************************
	Description:
		Set the down factor of down heigth  display sizer 
	Parameters:
		factor: the factor w1 ;w2 ;s1; s2;c 
	Remarks:
*********************************************************************************/

void VIM_IPP_SetDisplaySizerHFactor(PVIM_IPP_SzDnFct factor)
{
	VIM_HIF_SetReg8(V5_REG_IPP_DISV_H1,factor->w1);
	VIM_HIF_SetReg8(V5_REG_IPP_DISV_H2,factor->w2);
	VIM_HIF_SetReg16(V5_REG_IPP_DISV_S1,factor->s1);
	VIM_HIF_SetReg16(V5_REG_IPP_DISV_S2,factor->s2);
	VIM_HIF_SetReg8(V5_REG_IPP_DISV_C,factor->c);
}
/********************************************************************************
	Description:
		Set Sourouse Window and display Window
	Parameters:
		pt: the start point of source window
		winSize: the width and height of source window
		dispSize:the width and height of display window
	Remarks:
*********************************************************************************/
VIM_RESULT VIM_IPP_SetDispalySize(TPoint pt, TSize winSize, TSize dispSize)
{
VIM_IPP_SzDnFct DownFc;

UINT16 width,Height;
// check error
	if((dispSize.cx>IPP_MAX_DISPLAYSIZER_X)||(dispSize.cy>IPP_MAX_DISPLAYSIZER_Y))
		return VIM_ERROR_DISPLAY_WINDOW;
	VIM_IPP_GetImageSize(&width,&Height);
	if((width<winSize.cx)||(Height<winSize.cy))
		return VIM_ERROR_SOURCE_WINDOW;
	//set source window start position
	VIM_IPP_SetWindowPos(pt.x,pt.y);
	//set source window size
	VIM_IPP_SetWindowSize(winSize.cx,winSize.cy);
	//set display target size
	VIM_IPP_SetDispalyWindowSize(dispSize.cx,dispSize.cy);
	//get display factor and set display factor	
	
	 if(winSize.cx>=dispSize.cx)
	{
		VIM_IPP_CalcSizerDownFactor(winSize.cx,dispSize.cx,&DownFc);
		if(DownFc.w1==DownFc.w2)
			   VIM_IPP_SetSizeBypassEnable(VIM_IPP_BYPASS_DIS_W);
		else
		{
			VIM_IPP_SetDisplaySizerWFactor(&DownFc);
			VIM_IPP_SetSizeBypassDisable(VIM_IPP_BYPASS_DIS_W);
		}
	 }
	else
		return VIM_ERROR_DISPLAY_WINDOW;


	 if(winSize.cy>=dispSize.cy)
	{
		VIM_IPP_CalcSizerDownFactor(winSize.cy,dispSize.cy,&DownFc);
#if(VIM_IPP_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintDec("\n DownFc:-W1  ",DownFc.w1);
		VIM_USER_PrintDec("\n DownFc:-W2  ",DownFc.w2);
		VIM_USER_PrintDec("\n DownFc:-s1  ",DownFc.s1);
		VIM_USER_PrintDec("\n DownFc:-s2  ",DownFc.s2);
		VIM_USER_PrintDec("\n DownFc:-c  ",DownFc.c);
#endif	
		if(DownFc.w1==DownFc.w2)
		//if((DownFc.w1/DownFc.w2==1)&&(winSize.cx*2>=winSize.cy*3))	//angela 2007-2-21
		{
#if(VIM_IPP_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintString("\n ***********enable bypass ipp Y**********");

#endif	
			VIM_IPP_SetSizeBypassEnable(VIM_IPP_BYPASS_DIS_H);
		}
		else
		{
			VIM_IPP_SetDisplaySizerHFactor(&DownFc);
			VIM_IPP_SetSizeBypassDisable(VIM_IPP_BYPASS_DIS_H);
		}
	}
	else
		return VIM_ERROR_DISPLAY_WINDOW;
	
	Height = VIM_HIF_GetReg16(V5_REG_IPP_DISHT);	//update window 
	VIM_HIF_SetReg16(V5_REG_IPP_DISHT,Height);
	
	return VIM_SUCCEED;
}

/********************************************************************************
	Description:
		Config sizer window after capture  size in IPP module
	Parameters:
		wWidth:		sizer window width
		wHeight:	sizer window height 
	Remarks:
*********************************************************************************/
void VIM_IPP_SetCaptureWindowSize(UINT16 wWidth, UINT16 wHeight)
{
	VIM_HIF_SetReg16(V5_REG_IPP_CAPWD, wWidth);
	VIM_HIF_SetReg16(V5_REG_IPP_CAPHT, wHeight);
}
void VIM_IPP_GetCaptureWindowSize(PTSize Size)
{
	Size->cx = VIM_HIF_GetReg16(V5_REG_IPP_CAPWD);
	Size->cy = VIM_HIF_GetReg16(V5_REG_IPP_CAPHT);
}
/********************************************************************************
	Description:
		Set the down factor of down width capture sizer 
	Parameters:
		factor: the factor w1 ;w2 ;s1; s2;c 
	Remarks:
*********************************************************************************/

void VIM_IPP_SetCaptureSizerWFactor(PVIM_IPP_SzDnFct factor)
{
	VIM_HIF_SetReg8(V5_REG_IPP_CAPDH_W1,factor->w1);
	VIM_HIF_SetReg8(V5_REG_IPP_CAPDH_W2,factor->w2);
	VIM_HIF_SetReg16(V5_REG_IPP_CAPDH_S1,factor->s1);
	VIM_HIF_SetReg16(V5_REG_IPP_CAPDH_S2,factor->s2);
	VIM_HIF_SetReg8(V5_REG_IPP_CAPDH_C,factor->c);
}
/********************************************************************************
	Description:
		Set the down factor of down heigth  capture sizer 
	Parameters:
		factor: the factor w1 ;w2 ;s1; s2;c 
	Remarks:
*********************************************************************************/

void VIM_IPP_SetCaptureSizerHFactor(PVIM_IPP_SzDnFct factor)
{
	VIM_HIF_SetReg8(V5_REG_IPP_CAPDV_H1,factor->w1);
	VIM_HIF_SetReg8(V5_REG_IPP_CAPDV_H2,factor->w2);
	VIM_HIF_SetReg16(V5_REG_IPP_CAPDV_S1,factor->s1);
	VIM_HIF_SetReg16(V5_REG_IPP_CAPDV_S2,factor->s2);
	VIM_HIF_SetReg8(V5_REG_IPP_CAPDV_C,factor->c);
}

/********************************************************************************
	Description:
		Set the down factor of up width capture sizer 
	Parameters:
		factor: the factor w1 ;w2 ;s1; s2;c 
	Remarks:
*********************************************************************************/

void VIM_IPP_SetCaptureUpSizerWFactor(PVIM_IPP_SzUpFct factor)
{
	VIM_HIF_SetReg8(V5_REG_IPP_CAPUH_W1,factor->w1);
	VIM_HIF_SetReg8(V5_REG_IPP_CAPUH_W2,factor->w2);
	VIM_HIF_SetReg16(V5_REG_IPP_CAPUH_S,factor->s);
}
/********************************************************************************
	Description:
		Set the down factor of up heigth  capture sizer 
	Parameters:
		factor: the factor w1 ;w2 ;s1; s2;c 
	Remarks:
*********************************************************************************/

void VIM_IPP_SetCaptureUpSizerHFactor(PVIM_IPP_SzUpFct factor)
{
	VIM_HIF_SetReg8(V5_REG_IPP_CAPUV_H1,factor->w1);
	VIM_HIF_SetReg8(V5_REG_IPP_CAPUV_H2,factor->w2);
	VIM_HIF_SetReg16(V5_REG_IPP_CAPUV_S,factor->s);
}

/********************************************************************************
	Description:
		Set the down factor of down width thumb sizer 
	Parameters:
		factor: the factor w1 ;w2 ;s1; s2;c 
	Remarks:
*********************************************************************************/

void VIM_IPP_SetThumbSizerWFactor(PVIM_IPP_SzDnFct factor)
{
	VIM_HIF_SetReg8(V5_REG_IPP_THUMBH_W1,factor->w1);
	VIM_HIF_SetReg8(V5_REG_IPP_THUMBH_W2,factor->w2);
	VIM_HIF_SetReg16(V5_REG_IPP_THUMBH_S1,factor->s1);
	VIM_HIF_SetReg16(V5_REG_IPP_THUMBH_S2,factor->s2);
	VIM_HIF_SetReg8(V5_REG_IPP_THUMBH_C,factor->c);
}
/********************************************************************************
	Description:
		Set the down factor of down heigth  thumb sizer 
	Parameters:
		factor: the factor w1 ;w2 ;s1; s2;c 
	Remarks:
*********************************************************************************/

void VIM_IPP_SetThumbSizerHFactor(PVIM_IPP_SzDnFct factor)
{
	VIM_HIF_SetReg8(V5_REG_IPP_THUMBV_H1,factor->w1);
	VIM_HIF_SetReg8(V5_REG_IPP_THUMBV_H2,factor->w2);
	VIM_HIF_SetReg16(V5_REG_IPP_THUMBV_S1,factor->s1);
	VIM_HIF_SetReg16(V5_REG_IPP_THUMBV_S2,factor->s2);
	VIM_HIF_SetReg8(V5_REG_IPP_THUMBV_C,factor->c);
}

/********************************************************************************
	Description:
		Set thumbnail size
	Parameters:
		Width: the width of thumbnail size
		height: the height of thumbnail size
		0 means 256
	Remarks:
*********************************************************************************/
VIM_RESULT VIM_IPP_SetThumbSize(UINT8 bWidth, UINT8 bheight,VIM_IPP_THUMB_SELECT ThumbSelect)
{
VIM_IPP_SzDnFct DownFc;
TSize Size;
UINT16 wHeightCal;
	if((!bWidth)||(!bheight))
		return VIM_SUCCEED;
	if(ThumbSelect==VIM_IPP_THUMB_FROM_CAPTURE)
		VIM_IPP_GetCaptureWindowSize(&Size);
	else
		VIM_IPP_GetDispalyWindowSize(&Size.cx,&Size.cy);
	VIM_HIF_SetReg8(V5_REG_IPP_THUMB_WD,bWidth);
	VIM_HIF_SetReg8(V5_REG_IPP_THUMB_HT,bheight);
	VIM_HIF_SetReg8(V5_REG_IPP_THUMB_INPUT,ThumbSelect); //thumb from 
	wHeightCal=(UINT16)(((Size.cy* bWidth / Size.cx) >> 1) << 1);
	if((bWidth>Size.cx)||(bheight>Size.cy))
		return VIM_ERROR_THUMB_WINDOW;
	if(bheight>wHeightCal)
		return VIM_ERROR_THUMB_HEIGHT_MORE;
	VIM_IPP_CalcSizerDownFactor(Size.cx,bWidth,&DownFc);
	VIM_IPP_SetThumbSizerWFactor(&DownFc);
	VIM_IPP_CalcSizerDownFactor(Size.cy,wHeightCal,&DownFc);
	VIM_IPP_SetThumbSizerHFactor(&DownFc);
	return VIM_SUCCEED;
}
/********************************************************************************
	Description:
		Get thumbnail size
	Parameters:
		Width: the width of thumbnail size
		height: the height of thumbnail size
		0 means 256
	Remarks:
*********************************************************************************/
void VIM_IPP_GetThumbSize(PTSize Size)
{
	Size->cx=VIM_HIF_GetReg8(V5_REG_IPP_THUMB_WD);
	Size->cy=VIM_HIF_GetReg8(V5_REG_IPP_THUMB_HT);

}
/********************************************************************************
	Description:
		set the drop frame register in capture sizer
	Parameters:
		 wDropFrame: 0xffff: meas all will be kept
					 0x01: meas one will be kept
					 every bit means every frames  continuous
	Remarks:
*********************************************************************************/

void VIM_IPP_SetCaptureDropFrame(UINT16 wDropFrame)
{
	VIM_HIF_SetReg16(V5_REG_IPP_CAPFRMDP,wDropFrame);
}
/********************************************************************************
	Description:
		set the drop frame register in display sizer
	Parameters:
		 wDropFrame: 0xffff: meas all will be kept
					 0x01: meas one will be kept
					 every bit means every frames  
	Remarks:
*********************************************************************************/

void VIM_IPP_SetDisplayDropFrame(UINT16 wDropFrame)
{
	VIM_HIF_SetReg16(V5_REG_IPP_DISFRMDP,wDropFrame);
}

/********************************************************************************
	Description:
		Set Sourouse Window and capture Window
	Parameters:
		pt: the start point of source window
		winSize: the width and height of source window
		dispSize:the width and height of display window
	Remarks:
*********************************************************************************/
VIM_RESULT VIM_IPP_SetCaptureSize(TSnrInfo *pSensorInfo,TPoint pt, TSize winSize, TSize CaptureSize)
{
VIM_IPP_SzDnFct DownFc;
VIM_IPP_SzUpFct UpFc;
UINT16 width,Height;
VIM_IPP_CAPSIZE_MODE SourceSelect;
       // check error
       if((CaptureSize.cx>(2048))||(CaptureSize.cy>2048))//angela 2006-3-10
	//if((CaptureSize.cx>(2048-32))||(CaptureSize.cy>2048))
		return VIM_ERROR_CAPTURE_WINDOW;
	VIM_IPP_GetImageSize(&width,&Height);
	if((width<winSize.cx)||(Height<winSize.cy))
		return VIM_ERROR_SOURCE_WINDOW;
	if((width>640)||(Height>480))
		return VIM_ERROR_SOURCE_WINDOW;
	// normal sensor clock
	VIM_SIF_SetSensorClk(pSensorInfo,VIM_SIF_SENSOR_CLKNORMAL);
      //get factor and set factor	
	SourceSelect=VIM_IPP_GetCaputreSizeSelect();
	if(winSize.cx>=CaptureSize.cx)
	{
		if(winSize.cy<CaptureSize.cy)
			return VIM_ERROR_CAPTURE_WINDOW;
		if(SourceSelect!=VIM_IPP_CAPMODE_DOWN)
			      VIM_IPP_Reset();
		VIM_IPP_CaputreSizeSelect(VIM_IPP_CAPMODE_DOWN);
		VIM_IPP_CalcSizerDownFactor(winSize.cx,CaptureSize.cx,&DownFc);
		VIM_IPP_SetCaptureSizerWFactor(&DownFc);
		if(DownFc.w1==DownFc.w2)
			VIM_IPP_SetSizeBypassEnable(VIM_IPP_BYPASS_CAP_DOWN_W);
		else
			VIM_IPP_SetSizeBypassDisable(VIM_IPP_BYPASS_CAP_DOWN_W);
		VIM_IPP_CalcSizerDownFactor(winSize.cy,CaptureSize.cy,&DownFc);
		VIM_IPP_SetCaptureSizerHFactor(&DownFc);
		if(DownFc.w1==DownFc.w2)
			VIM_IPP_SetSizeBypassEnable(VIM_IPP_BYPASS_CAP_DOWN_H);
		else
			VIM_IPP_SetSizeBypassDisable(VIM_IPP_BYPASS_CAP_DOWN_H);
	}
	else
	{
		if(winSize.cy>=CaptureSize.cy)
			return VIM_ERROR_CAPTURE_WINDOW;
		if((winSize.cx>IPP_CSIZER_HIN_MAX)||(winSize.cy>IPP_CSIZER_VIN_MAX))
			return VIM_ERROR_UP_SOURCE_WINDOW;
		if((CaptureSize.cx>IPP_CSIZER_HOUT_MAX)||(CaptureSize.cy>IPP_CSIZER_VOUT_MAX))
			return VIM_ERROR_UP_SOURCE_WINDOW;		
		if(CaptureSize.cx>(winSize.cx<<1))
			return VIM_ERROR_UP_RESIZE_OVER;			
		if(SourceSelect!=VIM_IPP_CAPMODE_UP)
			     VIM_IPP_Reset();
		VIM_IPP_CaputreSizeSelect(VIM_IPP_CAPMODE_UP);

		VIM_IPP_CalcSizerUpFactor(winSize.cx,CaptureSize.cx,&UpFc);
		VIM_IPP_SetCaptureUpSizerWFactor(&UpFc);
		if(UpFc.w1==UpFc.w2)
			VIM_IPP_SetSizeBypassEnable(VIM_IPP_BYPASS_CAP_UP_W);
		else
			VIM_IPP_SetSizeBypassDisable(VIM_IPP_BYPASS_CAP_UP_W);
		VIM_IPP_CalcSizerUpFactor(winSize.cy,CaptureSize.cy,&UpFc);
		VIM_IPP_SetCaptureUpSizerHFactor(&UpFc);
		
		if(UpFc.w1==UpFc.w2)
			VIM_IPP_SetSizeBypassEnable(VIM_IPP_BYPASS_CAP_UP_H);
		else
		{
			VIM_IPP_SetSizeBypassDisable(VIM_IPP_BYPASS_CAP_UP_H);

			//if use 45Mhz MCLK, don't call VIM_SIF_SetSensorClk(pSensorInfo,VIM_SIF_SENSOR_HALF);
			
		}
		// clk should changed 
	}
      // set value
	VIM_IPP_SetWindowPos(pt.x,pt.y);
	VIM_IPP_SetWindowSize(winSize.cx,winSize.cy);
	VIM_IPP_SetCaptureWindowSize(CaptureSize.cx,CaptureSize.cy);
	//UPDATA
	Height = VIM_HIF_GetReg16(V5_REG_IPP_DISHT);	//update window 
	VIM_HIF_SetReg16(V5_REG_IPP_DISHT,Height);
	return VIM_SUCCEED;
}

/********************************************************************************
Description:
	for caculate window
parameters: 
	src:         preview off
	dst:         preview on without frame
	pwin:       preview on with frame
	
Return:
		VIM_SUCCEED: cacaulte ok
		VIM_ERROR_CACAULATELE_SOURCE_WINDOW: cacaulte less source window error
Remarks:
*********************************************************************************/
#if 0
VIM_RESULT VIM_IPP_ToolCaculateLessSrcWindow(TSize src, TSize dst,TSize *win)
{
UINT16 div,adjusttimes=0;
	if(dst.cx>dst.cy)
		div=dst.cy;
	else
		div=dst.cx;
ADJUSTDIV:
	while((dst.cx%div)||(dst.cy%div))
	{
		div--;
		if(!div)
			return VIM_ERROR_CACAULATELE_SOURCE_WINDOW;
	}
	if((adjusttimes++>VIM_IPP_ADJUST_NUM)||(div>10))
		goto DIVOK;
	else
	{
		dst.cx/=div;
		dst.cy/=div;
		if(dst.cx%2)
			dst.cx++;
		if(dst.cy%2)
			dst.cy++;		
		goto ADJUSTDIV;
	}
DIVOK:
	dst.cx/=div;
	dst.cy/=div;
	div=1;
	
	while((dst.cy*div<=src.cy)&&(dst.cx*div<=src.cx))
	{
		div++;
		if((div>(src.cy+1))||(div>(src.cx+1)))
			return VIM_ERROR_CACAULATELE_SOURCE_WINDOW;
	}
	win->cy=(dst.cy*(div-1))&0xfffe;
	win->cx=(dst.cx*(div-1))&0xfffe;
	
	return VIM_SUCCEED;
}
#else
/*VIM_RESULT VIM_IPP_ToolCaculateLessSrcWindow(TSize src, TSize dst,TSize *win)
{
	TSize winsize,winsize1;

 	if ((src.cy/src.cx)>(dst.cy/dst.cx))
	{
		winsize.cx=src.cx&0xfffc;
		winsize.cy=(winsize.cx*dst.cy/dst.cx)&0xfffc;
	}
	else if ((src.cy/src.cx)<(dst.cy/dst.cx))
	{
		winsize.cy=src.cy&0xfffc;
		winsize.cx=(winsize.cy*dst.cx/dst.cy)&0xfffc;
	}
	else
	{
		winsize.cy=src.cy&0xfffc;
		winsize.cx=(winsize.cy*dst.cx/dst.cy)&0xfffc;
		winsize1.cx=src.cx&0xfffc;
		winsize1.cy=(winsize1.cx*dst.cy/dst.cx)&0xfffc;
		if (winsize1.cx<=winsize.cx)
		{
			winsize.cx=winsize1.cx;
			winsize.cy=winsize1.cy;
		}
	}
	win->cx=winsize.cx;
	win->cy=winsize.cy;	
	return VIM_SUCCEED;	
}
*/
VIM_RESULT VIM_IPP_ToolCaculateLessSrcWindow(TSize src, TSize dst,TSize *win)
{
	UINT16 div,temp=0;
	

	if((dst.cx>src.cx)||(dst.cy>src.cy))
		temp=1;
	

	if(dst.cx>dst.cy)
		div=dst.cy;
	else
		div=dst.cx;
	while((dst.cx%div)||(dst.cy%div))
	{
		div--;
		if(!div)
			return VIM_ERROR_CACAULATELE_SOURCE_WINDOW;
	}
	dst.cx/=div;
	dst.cy/=div;
RECAPSOURSE:	
	div=1;


	while((dst.cy*div<=src.cy)&&(dst.cx*div<=src.cx))
	{
		div++;
		if((div>(src.cy+1))||(div>(src.cx+1)))
			return VIM_ERROR_CACAULATELE_SOURCE_WINDOW;
	}
	win->cy=(dst.cy*(div-1))&0xfffe;
	win->cx=(dst.cx*(div-1))&0xfffe;

	if(temp)
	{
		if((win->cx>IPP_CSIZER_HIN_MAX)||(win->cy>IPP_CSIZER_VIN_MAX))
		{
			src.cx=IPP_CSIZER_HIN_MAX;
			src.cy=IPP_CSIZER_VIN_MAX;
			goto RECAPSOURSE;
		}
	}
	return VIM_SUCCEED;
}
#endif
/********************************************************************************
Description:
	for caculate window
parameters: 
	src:         preview off
	dst:         preview on without frame
	pwin:       preview on with frame
	
Return:
		VIM_SUCCEED: cacaulte ok
		VIM_ERROR_CACAULATELE_SOURCE_WINDOW: cacaulte big source window error
Remarks:
*********************************************************************************/
#define VIM_IPP_ADJUST_NUM	0
#if 0
VIM_RESULT VIM_IPP_ToolCaculateBigDstWindow(TSize src, TSize dstdis,TSize *win)
{
	UINT16 div,adjusttimes=0;
	TSize dst	;
ADJUSTDIV:

	dst.cx=(dstdis.cx<=dstdis.cy)?dstdis.cy : dstdis.cx;
	dst.cy=(dstdis.cx<=dstdis.cy)?dstdis.cy : dstdis.cx;
	
	if(src.cx>src.cy)
		div=src.cy;
	else
		div=src.cx;

	while((src.cx%div)||(src.cy%div))
	{
		div--;
		if(!div)
			return VIM_ERROR_CACAULATELE_SOURCE_WINDOW;
	}
	
	if((adjusttimes++>VIM_IPP_ADJUST_NUM)||(div>10))
		goto DIVOK;
	else
	{
		src.cx/=div;
		src.cy/=div;
		if(src.cx%2)
			src.cx++;
		if(src.cy%2)
			src.cy++;		
		goto ADJUSTDIV;
	}

DIVOK:
	src.cx/=div;
	src.cy/=div;
	div=1;


	while((src.cy*div<dst.cy)||(src.cx*div<dst.cx))
	{
		div++;
		if((div>(dst.cy+1))&&(div>(dst.cx+1)))
			return VIM_ERROR_CACAULATELE_SOURCE_WINDOW;
	}
	win->cy=(src.cy*div+1)&0xfffe;
	win->cx=(src.cx*div+1)&0xfffe;

	// adjust the cx cy should be more than x or y
	if(win->cx>IPP_MAX_DISPLAYSIZER_X)
	{
		if(src.cx%2)
			src.cx++;
		if(src.cy%2)
			src.cy++;		
		goto ADJUSTDIV;
	}
	if(win->cy>IPP_MAX_DISPLAYSIZER_Y)
	{
		if(src.cx%2)
			src.cx++;
		if(src.cy%2)
			src.cy++;		
		goto ADJUSTDIV;
	}
	return VIM_SUCCEED;		
	
}
#else
VIM_RESULT VIM_IPP_ToolCaculateBigDstWindow(TSize src, TSize dst,TSize *win)
{
	TSize winsize,winsize1;
	if(dst.cx>src.cx)			//2006-11-17 angela 
		dst.cx=src.cx;
	if(dst.cy>src.cy)			//2006-11-17 angela 
		dst.cy=src.cy;
	
REPEATCACULATE:	
 	 if ((src.cy/src.cx)>(dst.cy/dst.cx))
	{

		winsize.cx=(dst.cx)&0xfffc;
		winsize.cy=(((UINT32)winsize.cx*(UINT32)src.cy/(UINT32)src.cx))&0xfffc;
	}
	else if ((src.cy/src.cx)<(dst.cy/dst.cx))
	{

		winsize.cy=(dst.cy)&0xfffc;
		winsize.cx=(((UINT32)winsize.cy*(UINT32)src.cx/(UINT32)src.cy))&0xfffc;
	}
	else
	{
		winsize.cy=(dst.cy)&0xfffc;
		winsize.cx=(((UINT32)winsize.cy*(UINT32)src.cx/(UINT32)src.cy))&0xfffc;
		winsize1.cx=(dst.cx)&0xfffc;
		winsize1.cy=(((UINT32)winsize1.cx*(UINT32)src.cy/(UINT32)src.cx))&0xfffc;
		if (winsize1.cx>=winsize.cx)
		{
			winsize.cx=winsize1.cx;
			winsize.cy=winsize1.cy;
		}
	}
	win->cx=winsize.cx;
	win->cy=winsize.cy;	
	if((win->cx>IPP_MAX_DISPLAYSIZER_X)||(win->cy>IPP_MAX_DISPLAYSIZER_Y))
		return VIM_ERROR_DISPLAY_WINDOW;

	if(win->cx>AM_MAXPIXEL)
	{
		dst.cy-=4;
		goto	REPEATCACULATE;
	}
	if(win->cy>AM_MAXPIXEL)
	{
		dst.cx-=4;
		goto	REPEATCACULATE;
	}
	if(((VIM_HIF_GetReg8(V5_REG_IPP_SPECTRL)>>5)&0x3)==3)//display mode
	{
		if((((win->cx-dst.cx)>>1)+dst.cx)>IPP_MAX_DISPLAYSIZER_RAM)	//angela 2007-3-5
		{
	#if(VIM_IPP_DEBUG)&&(VIM_528RDK_DEBUG)
			VIM_USER_PrintString("\n ***********DISPLAY IS MORE THAN 320**********");
	#endif	
			dst.cy-=4;
			goto	REPEATCACULATE;
		}
	}
	return VIM_SUCCEED;	
}
#endif

/********************************************************************************
Description:
	for caculate window
parameters: 
	src:         preview off
	dst:         preview on without frame
	pwin:       preview on with frame
	
Return:
		VIM_SUCCEED: cacaulte ok
		VIM_ERROR_CACAULATELE_SOURCE_WINDOW: cacaulte less source window error
Remarks:
*********************************************************************************/
VIM_RESULT VIM_IPP_ToolCaculateLessDisplayWindow(TSize src, TSize dst,TSize *dis)
{
	TSize destsize,destsize1;
 	//dst.cx=(dst.cx<src.cx)?dst.cx : src.cx;
	//dst.cy=(dst.cy<src.cy)?dst.cy : src.cy;
	if((src.cx<dst.cx)&&(src.cy<dst.cy))  //2006-8-4 angela 
	{
		destsize.cy=src.cy&0xfffc;
		destsize.cx=src.cx&0xfffc;
	}
 	else if ((src.cy/src.cx)>(dst.cy/dst.cx))
	{
		destsize.cy=dst.cy&0xfffc;
		destsize.cx=(destsize.cy*src.cx/src.cy)&0xfffc;
	}
	else if ((src.cy/src.cx)<(dst.cy/dst.cx))
	{
		destsize.cx=dst.cx&0xfffc;
		destsize.cy=(destsize.cx*src.cy/src.cx)&0xfffc;
	}
	else
	{
		destsize.cy=dst.cy&0xfffc;
		/*destsize.cx=destsize.cy*src.cx;
		dst.cy=destsize.cx;
		destsize.cx/=src.cy;
		destsize1.cy=destsize.cx;
		destsize.cx&=0xfffc;*/
		destsize.cx=((UINT32)destsize.cy*(UINT32)src.cx/(UINT32)src.cy)&0xfffc;	//guoying 12/29/2007, 强制转换成UINT32
		destsize1.cx=dst.cx&0xfffc;
		destsize1.cy=((UINT32)destsize1.cx*(UINT32)src.cy/(UINT32)src.cx)&0xfffc;
		if (destsize1.cx<=destsize.cx)
		{
			destsize.cx=destsize1.cx;
			destsize.cy=destsize1.cy;
		}
	}
	dis->cx=destsize.cx;
	dis->cy=destsize.cy;	
	if((dis->cx>IPP_MAX_DISPLAYSIZER_X)||(dis->cy>IPP_MAX_DISPLAYSIZER_Y))
		return VIM_ERROR_DISPLAY_WINDOW;
	return VIM_SUCCEED;
}
