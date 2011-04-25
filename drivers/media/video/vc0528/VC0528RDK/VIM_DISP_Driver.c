
/*************************************************************************
*                                                                       
*                Copyright (C) 2005 Vimicro CO.,LTD     		 
*
* FILE NAME                                  VERSION                
*                                                                       
* VIM_HIF_Driver.c								   	  0.2                   
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*     VC0528 (Host Interface and cpm ) implement function.                                 
*
*	Version		Author			Date		Description		
*  ---------------------------------------------------------------						 
*   0.1			maning		2005-11-2	The first version. 
*   0.2			angela		2006-06-07	update for 528
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
#include "VIM_COMMON.h"
/*-----------------------------------------------------------------------------
  Summary:
	Set DISP Work Mode
  Parameters:
	bWorkMode : work mode include VIM_DISP_WORKMODE
  Note:
  Returns: void
  Remarks:
--------------------------------------------------------------------------------*/
VIM_RESULT VIM_DISP_SetWorkMode(VIM_DISP_WORKMODE bWorkMode)
{
UINT16 value16 = 0,Temp=0;
	value16 = VIM_HIF_GetReg16(V5_REG_LCDC_DC);
	switch(bWorkMode)
	{
		case VIM_DISP_AFIRST:
			value16&=(~(BIT3|BIT6|BIT7|BIT8|BIT12));// disable overlay disable blend a PRI normal
			break;
		case VIM_DISP_BFIRST:
			value16&=(~(BIT6|BIT7|BIT12));// disable overlay disable blend  normal
			value16|=BIT8;  //b PRI
			break;
		case VIM_DISP_B0_OVERLAY:
			value16&=(~(BIT7|BIT8|BIT12));// disable overlay disable blend a PRI normal
			value16|=BIT6;  //overlay enable
			value16|=BIT8;   //b PRI
			break;
		case VIM_DISP_B0_BLEND:
			value16&=(~(BIT6|BIT8|BIT12));// disable overlay disable blend a PRI normal
			value16|=BIT7;  //blend enable
			value16|=BIT8;   //b PRI
			break;
		case VIM_DISP_ALL_OVERLAY:
			value16&=(~(BIT7|BIT8|BIT12));// disable overlay disable blend a PRI normal
			value16|=BIT6;  //overlay enable
			value16|=BIT8;   //b PRI
			Temp=(3<<2)|BIT6; //case 4  ,b1 have high PRI
			break;
		case VIM_DISP_ALL_BLEND:
			value16&=(~(BIT6|BIT8|BIT12));// disable overlay disable blend a PRI normal
			value16|=BIT7;  //blend enable
			value16|=BIT8;   //b PRI
			Temp=(3<<2)|BIT6; //case 4  ,b1 have high PRI
			break;
		case VIM_DISP_DECODE:
			value16|=BIT12;  //
			break;
		default:
			return VIM_ERROR_DISP_PARAM;
	}
	VIM_HIF_SetReg8(V5_REG_LCDC_UPDATE,Temp);
	VIM_HIF_SetReg16(V5_REG_LCDC_DC,value16);
	VIM_DISP_ResetState();
	return VIM_SUCCEED;
}
/*-----------------------------------------------------------------------------
  Summary:
	Set Dither Select
  Parameters:
	byDitherSelect : enum VIM_DISP_DITHERSEL
  Note:
  Returns: 
  Remarks:
--------------------------------------------------------------------------------*/
VIM_RESULT VIM_DISP_SetDitherSelet(VIM_DISP_DITHERSEL byDitherSelect)
{
	UINT16 value16 = 0;

	if(byDitherSelect > VIM_DISP_RGB332)
		return VIM_ERROR_DISP_PARAM;

	value16 = VIM_HIF_GetReg16(V5_REG_LCDC_DC);
	value16 &= ~(BIT1|BIT2);
	value16 |= (byDitherSelect<<1);	// SFBUF VBUFM
	VIM_HIF_SetReg16(V5_REG_LCDC_DC,value16);
	return VIM_SUCCEED;
}

/*-----------------------------------------------------------------------------
  Summary:
	Set Buffer Mode
  Parameters:
		byBufferMode : VIM_DISP_BUFFERMODER
  Note:
  Returns: 
  Remarks:
--------------------------------------------------------------------------------*/
VIM_RESULT VIM_DISP_SetBufferMode(VIM_DISP_BUFFERMODER byBufferMode)
{
	UINT16 value16 = 0;

	if(byBufferMode==VIM_DISP_NODISP)
		byBufferMode=VIM_DISP_LINEBUF;
	if((byBufferMode != 0) && (byBufferMode != 2) && (byBufferMode != 3))
		return VIM_ERROR_DISP_MODE;

	value16 = VIM_HIF_GetReg16(V5_REG_LCDC_DC);
	value16 &= ~(BIT14|BIT15);
	value16 |=( byBufferMode<<14);	// SFBUF VBUFM	//angela
	VIM_HIF_SetReg16(V5_REG_LCDC_DC,value16);
	return VIM_SUCCEED;
}

/*-----------------------------------------------------------------------------
  Summary:
	Get Buffer Mode
  Parameters:
		byBufferMode : VIM_DISP_BUFFERMODER
  Note:
  Returns: 
  Remarks:
--------------------------------------------------------------------------------*/
VIM_DISP_BUFFERMODER VIM_DISP_GetBufferMode(void)
{
	UINT16 value16 = 0;

	value16 = VIM_HIF_GetReg16(V5_REG_LCDC_DC);
	value16 &= (BIT14|BIT15);
	value16 >>=14;	// SFBUF VBUFM	//angela

	return (VIM_DISP_BUFFERMODER)value16;
}

/*-----------------------------------------------------------------------------
  Summary:
	Set Dither enable
  Parameters:
	Enable :enable or not
  Note:
  Returns: 
  Remarks:
--------------------------------------------------------------------------------*/
VIM_RESULT VIM_DISP_SetDitherEnable(BOOL Enable)
{
	UINT16 value16 = 0;

	value16 = VIM_HIF_GetReg16(V5_REG_LCDC_DC);
	value16 &= ~(BIT0);
	value16 |= (Enable);	// SFBUF VBUFM
	VIM_HIF_SetReg16(V5_REG_LCDC_DC,value16);
	return VIM_SUCCEED;
}
/***************************************************************
Description:
		set layer enable or not

Parameters:
		bLayer: which layer
		Enable: enalbe or not

Returns:
		void
****************************************************************/
VIM_RESULT VIM_DISP_SetLayerEnable(VIM_DISP_LAYER bLayer,BOOL Enable)
{
UINT8 byMode;
	//VIM_DISP_ResetState();
	byMode = VIM_HIF_GetReg8(V5_REG_LCDC_DE);

	if(bLayer&VIM_DISP_ALAYER)
	{
		//byMode&=(~(BIT1|BIT4));
		byMode&=(~(BIT1));//|BIT4));//angela 2006-9-6
		byMode|=(Enable<<1);
		byMode|=(Enable<<4);
	}
	if(bLayer&VIM_DISP_B0LAYER)
	{
		byMode&=(~(BIT2));
		byMode|=(Enable<<2);
	}
	if(bLayer&VIM_DISP_B1LAYER)
	{
		byMode&=(~(BIT5));
		byMode|=(Enable<<5);
	}

	VIM_HIF_SetReg8(V5_REG_LCDC_DE, byMode);
#ifndef CONFIG_MACH_CANOPUS
	VIM_USER_DelayMs(1);
#endif
	return VIM_SUCCEED;
}
/*-----------------------------------------------------------------------------
  Summary:
	Set Dither enable
  Parameters:
	Enable :enable or not
  Note:
  Returns: 
  Remarks:
--------------------------------------------------------------------------------*/
VIM_RESULT VIM_DISP_SaveAramEnable(BOOL Enable)
{
	UINT16 value16 = 0;

	value16 = VIM_HIF_GetReg16(V5_REG_LCDC_DE);
	value16 &= ~(BIT0);
	value16 |= (Enable);	// SFBUF VBUFM
	if(Enable==DISABLE)
		value16 &= ~(BIT1);
	VIM_HIF_SetReg16(V5_REG_LCDC_DE,value16);
	return VIM_SUCCEED;
}

/*-----------------------------------------------------------------------------
  Summary:
	Reset all Display except register
  Parameters:
  Note:
  Returns: 
  Remarks:
--------------------------------------------------------------------------------*/
void VIM_DISP_ResetState(void)
{
	VIM_HIF_SetReg8(V5_REG_LCDC_DRST, 0x80);
}

/*-----------------------------------------------------------------------------
  Summary:
	Update B Layer
  Parameters:
	BLayer : VIM_DISP_BLAYER,select update B0/B1/ALL
  Note:
  Returns: 
  Remarks:
--------------------------------------------------------------------------------*/
VIM_RESULT VIM_DISP_Update(VIM_DISP_LAYER BLayer)
{
	UINT8 value = 0;

	if((BLayer !=VIM_DISP_B0LAYER)&&( BLayer != VIM_DISP_B1LAYER))
		return VIM_ERROR_DISP_BLAYER;

	value = VIM_HIF_GetReg8(V5_REG_LCDC_UPDATE);

	// update_b
	if(BLayer == VIM_DISP_B0LAYER)
		value |= BIT0;
	else
		value |= BIT7;

	VIM_HIF_SetReg8(V5_REG_LCDC_UPDATE,value);
	return VIM_SUCCEED;
}
/*-----------------------------------------------------------------------------
  Summary:
	set Rotate mode
  Parameters:
	byARotateMode : A layer Rotate Mode
	byBRotateMode : B0 layer Rotate Mode
  Note:
  Returns: 
  Remarks:
--------------------------------------------------------------------------------*/
VIM_RESULT VIM_DISP_SetRotateMode(VIM_DISP_LAYER Layer,VIM_DISP_ROTATEMODE RotateMode)
{
	UINT8 value = 0,bTemp=0;
	TSize Size;
	
	if(RotateMode > VIM_DISP_NOTCHANGE || RotateMode > VIM_DISP_NOTCHANGE)
		return VIM_ERROR_DISP_PARAM;
	value = VIM_HIF_GetReg8(V5_REG_LCDC_RM);
	if(Layer&VIM_DISP_ALAYER)
	{
		if(RotateMode%2)
		{
			VIM_DISP_GetAWinsize(&Size);
			VIM_HIF_SetReg16(V5_REG_LCDC_AW_L,Size.cy-1);
			VIM_HIF_SetReg16(V5_REG_LCDC_AH_L,Size.cx-1);
		}
		value &= 0xf8;
		value |= (RotateMode & 0x7);
	}

	if(Layer&VIM_DISP_B0LAYER)
	{
		if(RotateMode%2)
		{
			bTemp = VIM_HIF_GetReg16(V5_REG_LCDC_BW0_L);
			VIM_HIF_SetReg16(V5_REG_LCDC_BW0_L,VIM_HIF_GetReg16(V5_REG_LCDC_BH0_L));
			VIM_HIF_SetReg16(V5_REG_LCDC_BH0_L,bTemp);

		}
		value &= 0xc7;
		value |= ((RotateMode & 0x7) << 3);
	}
	if(Layer&VIM_DISP_B1LAYER)
	{
		if(RotateMode%2)
		{
			bTemp = VIM_HIF_GetReg16(V5_REG_LCDC_BW1_L);
			VIM_HIF_SetReg16(V5_REG_LCDC_BW1_L,VIM_HIF_GetReg16(V5_REG_LCDC_BH1_L));
			VIM_HIF_SetReg16(V5_REG_LCDC_BH1_L,bTemp);
		}
		value &= 0xc7;
		value |= ((RotateMode & 0x7) << 3);
	}
	VIM_HIF_SetReg8(V5_REG_LCDC_RM,value);
	return VIM_SUCCEED;
}
/*-----------------------------------------------------------------------------
  Summary:
	Set A layer Display panel startpoint and size
  Parameters:
	startPoint	: start Point in LCD 
	winSize		: size in LCD
  Note:
       this function modify by Amanda Deng 2005-12-13 16:41 
       important calss: *******
       V5_REG_LCDC_AW value really is width-1
       V5_REG_LCDC_AH value really is height-1
       
  Returns: 
  Remarks:
--------------------------------------------------------------------------------*/
VIM_RESULT VIM_DISP_SetA_DisplayPanel(TPoint startPoint,TSize winSize)
{
	if(startPoint.x > 511 || startPoint.y > 511)
		return VIM_ERROR_DISP_PARAM;

	if(winSize.cx > 511 || winSize.cy > 512)
		return VIM_ERROR_DISP_PARAM;

	// Set start Point
	VIM_HIF_SetReg16(V5_REG_LCDC_AX_L,startPoint.x);
	VIM_HIF_SetReg16(V5_REG_LCDC_AY_L,startPoint.y);
	// Set Panel Size
	if(winSize.cx)
		winSize.cx--;
	if(winSize.cy)
		winSize.cy--;
	VIM_HIF_SetReg16(V5_REG_LCDC_AW_L,winSize.cx);
	VIM_HIF_SetReg16(V5_REG_LCDC_AH_L,winSize.cy);
	return VIM_SUCCEED;
}
void VIM_DISP_GetAWinsize(PTSize winSize)
{
	winSize->cx=VIM_HIF_GetReg16(V5_REG_LCDC_AW_L)+1;
	winSize->cy=VIM_HIF_GetReg16(V5_REG_LCDC_AH_L)+1;
}
/*-----------------------------------------------------------------------------
  Summary:
	Set A layer memory panel startpoint and size
  Parameters:
	memPoint	: start Point in memory buffer 
	memSize		: size of memory buffer
  Note:
  Returns: 
  Remarks:
--------------------------------------------------------------------------------*/
VIM_RESULT VIM_DISP_SetA_Memory(TPoint memPoint,TSize memSize)
{
	if(memPoint.x > AM_MAXPIXEL || memPoint.y > AM_MAXPIXEL)
		return VIM_ERROR_DISP_PARAM;

	if(memSize.cx > AM_MAXPIXEL || memSize.cy > AM_MAXPIXEL)
		return VIM_ERROR_DISP_PARAM;
   
	// Set mem Point
	VIM_HIF_SetReg16(V5_REG_LCDC_ADX0_L,memPoint.x);
	VIM_HIF_SetReg16(V5_REG_LCDC_ADY0_L,memPoint.y);
	// Set mem Size
	VIM_HIF_SetReg16(V5_REG_LCDC_AMW_L,memSize.cx);
	VIM_HIF_SetReg16(V5_REG_LCDC_AMH_L,memSize.cy);
	return VIM_SUCCEED;
}

/*-----------------------------------------------------------------------------
  Summary:
	Set B0 layer Display panel startpoint and size
  Parameters:
	startPoint	: start Point in LCD 
	winSize		: size in LCD
  Note: set BDX0\BDY0 is 0,BMW0 is winSize's width
  Returns: 
  Remarks:
--------------------------------------------------------------------------------*/
VIM_RESULT VIM_DISP_SetB0_DisplayPanel(TPoint startPoint,TSize winSize)
{
	if(startPoint.x > 511 || startPoint.y > 511)
		return VIM_ERROR_DISP_PARAM;

	if(winSize.cx > 512 || winSize.cy > 512)
		return VIM_ERROR_DISP_PARAM;

	// Set start Point
	VIM_HIF_SetReg16(V5_REG_LCDC_BX0_L,startPoint.x);
	VIM_HIF_SetReg16(V5_REG_LCDC_BY0_L,startPoint.y);
	
	if(winSize.cx)
		winSize.cx--;
	if(winSize.cy)
		winSize.cy--;

	// Set Panel Size
	VIM_HIF_SetReg16(V5_REG_LCDC_BW0_L,winSize.cx);
	VIM_HIF_SetReg16(V5_REG_LCDC_BH0_L, winSize.cy);
	// Set Memory Width
	VIM_HIF_SetReg16(V5_REG_LCDC_BMW0_L,winSize.cx);
	
	// Set BDX0 and BDY0
	VIM_HIF_SetReg16(V5_REG_LCDC_BDX0_L,0);
	VIM_HIF_SetReg16(V5_REG_LCDC_BDY0_L,0);
	return VIM_SUCCEED;
}

/*-----------------------------------------------------------------------------
  Summary:
	set B0 layer Refresh Block start point and size
  Parameters:
	startPoint	: start Point in B0 layer 
	winSize		: size in B0 layer
  Note:
  Returns: 
  Remarks:
--------------------------------------------------------------------------------*/
VIM_RESULT VIM_DISP_SetB0_RefreshBlock(TPoint startPoint,TSize winSize)
{
	if(startPoint.x > 511 || startPoint.y > 511)
		return VIM_ERROR_DISP_PARAM;

	if(winSize.cx > 511 || winSize.cy > 512)
		return VIM_ERROR_DISP_PARAM;

	// Set start Point
	VIM_HIF_SetReg16(V5_REG_LCDC_BRX0_L,startPoint.x);
	VIM_HIF_SetReg16(V5_REG_LCDC_BRY0_L,startPoint.y);

	if(winSize.cx)
		winSize.cx--;
	if(winSize.cy)
		winSize.cy--;	
	// Set Panel Size
	VIM_HIF_SetReg16(V5_REG_LCDC_BRW0_L,winSize.cx);
	VIM_HIF_SetReg16(V5_REG_LCDC_BRH0_L,winSize.cy);
	return VIM_SUCCEED;
}

/*-----------------------------------------------------------------------------
  Summary:
	Set B1 layer Display panel startpoint and size
  Parameters:
	startPoint	: start Point in LCD 
	winSize		: size in LCD
  Note: set BDX1/BDY1 is 0,BMW1 is winSize's width
  Returns: 
  Remarks:
--------------------------------------------------------------------------------*/
VIM_RESULT VIM_DISP_SetB1_DisplayPanel(TPoint startPoint,TSize winSize)
{
	if(startPoint.x > 511 || startPoint.y > 511)
		return VIM_ERROR_DISP_PARAM;

	if(winSize.cx > 511 || winSize.cy > 512)
		return VIM_ERROR_DISP_PARAM;

	// Set start Point
	VIM_HIF_SetReg16(V5_REG_LCDC_BX1_L,startPoint.x);
	VIM_HIF_SetReg16(V5_REG_LCDC_BY1_L,startPoint.y);
	
	if(winSize.cx)
		winSize.cx--;
	if(winSize.cy)
		winSize.cy--;
	// Set Panel Size
	VIM_HIF_SetReg16(V5_REG_LCDC_BW1_L,winSize.cx);
	VIM_HIF_SetReg16(V5_REG_LCDC_BH1_L,winSize.cy);
	// Set Memory Width
	VIM_HIF_SetReg16(V5_REG_LCDC_BMW1_L,winSize.cx);
	// Set BDX0 and BDY0
	VIM_HIF_SetReg16(V5_REG_LCDC_BDX1_L,0);
	VIM_HIF_SetReg16(V5_REG_LCDC_BDY1_L,0);
	return VIM_SUCCEED;
}

/*-----------------------------------------------------------------------------
  Summary:
	set B1 layer Refresh Block start point and size
  Parameters:
	startPoint	: start Point in B1 layer 
	winSize		: size in B1 layer
  Note:
  Returns: 
  Remarks:
--------------------------------------------------------------------------------*/
VIM_RESULT VIM_DISP_SetB1_RefreshBlock(TPoint startPoint,TSize winSize)
{
	if(startPoint.x > 511 || startPoint.y > 511)
		return VIM_ERROR_DISP_PARAM;

	if(winSize.cx > 511 || winSize.cy > 512)
		return VIM_ERROR_DISP_PARAM;

	// Set start Point
	VIM_HIF_SetReg16(V5_REG_LCDC_BRX1_L,startPoint.x);
	VIM_HIF_SetReg16(V5_REG_LCDC_BRY1_L,startPoint.y);

	if(winSize.cx)
		winSize.cx--;
	if(winSize.cy)
		winSize.cy--;
	// Set Panel Size
	VIM_HIF_SetReg16(V5_REG_LCDC_BRW1_L,winSize.cx);
	VIM_HIF_SetReg16(V5_REG_LCDC_BRH1_L,winSize.cy);
	return VIM_SUCCEED;	

}



/*-----------------------------------------------------------------------------
  Summary:
	set Blend Ratio
  Parameters:
	BLayer : select B0 / B1 / all layer
	byBlendRatio : Blend Ratio (0-255)
  Note:
  Returns: 
  Remarks:
--------------------------------------------------------------------------------*/
VIM_RESULT VIM_DISP_SetBlendRatio(VIM_DISP_LAYER BLayer,UINT8 byBlendRatio)
{
	if((BLayer <VIM_DISP_B0LAYER)||( BLayer > VIM_DISP_ALLBLAYER))
		return VIM_ERROR_DISP_BLAYER;
	if(byBlendRatio > 255)
		return VIM_ERROR_DISP_PARAM;

	if(BLayer & VIM_DISP_B0LAYER)
		VIM_HIF_SetReg8(V5_REG_LCDC_BR0_L, byBlendRatio);
	if(BLayer & VIM_DISP_B1LAYER)
		VIM_HIF_SetReg8(V5_REG_LCDC_BR1_L, byBlendRatio);
	return VIM_SUCCEED;
}

/*-----------------------------------------------------------------------------
  Summary:
	Set B layer graphic buffer format
  Parameters:
	BLayer : select B0 / B1 / all layer
	byGbufFmt : graphic buffer formate VIM_DISP_BFORMAT
  Note:
  Returns: 
  Remarks:
--------------------------------------------------------------------------------*/
VIM_RESULT VIM_DISP_SetGbufFormat(VIM_DISP_BFORMAT byGbufFmt)
{
	if(byGbufFmt > VIM_DISP_BLAYER_RGB32)
		return VIM_ERROR_DISP_PARAM;

	VIM_HIF_SetReg8(V5_REG_LCDC_GBUFFMT, byGbufFmt);
	return VIM_SUCCEED;
}

/*-----------------------------------------------------------------------------
  Summary:
	Get B layer graphic buffer format
  Parameters:
	BLayer : select B0 / B1 / all layer
	byGbufFmt : point of graphic buffer formate VIM_DISP_Blayer_FORMAT
  Note:
  Returns: 
  Remarks:
--------------------------------------------------------------------------------*/
void VIM_DISP_GetGbufFormat(VIM_DISP_BFORMAT * byGbufFmt)
{
	*byGbufFmt =(VIM_DISP_BFORMAT)VIM_HIF_GetReg8(V5_REG_LCDC_GBUFFMT);	
}

/*-----------------------------------------------------------------------------
  Summary:
	set overlay key color
  Parameters:
	BLayer : select B0 / B1 / all layer
	dwKeyColor : key color
  Note:
  Returns: 
  Remarks:
--------------------------------------------------------------------------------*/
VIM_RESULT VIM_DISP_SetOverlayKeyColor(VIM_DISP_LAYER BLayer,UINT32 dwKeyColor)
{
	if(BLayer > VIM_DISP_ALLLAYER)
		return VIM_ERROR_DISP_BLAYER;

	dwKeyColor&=0xffffff; //clear okcm use key color
	if(BLayer & VIM_DISP_B0LAYER)
	{
		VIM_HIF_SetReg32(V5_REG_LCDC_OKC0_L,dwKeyColor);
	}
	if(BLayer & VIM_DISP_B1LAYER)
	{
		VIM_HIF_SetReg32(V5_REG_LCDC_OKC1_L,dwKeyColor);
	}
	return VIM_SUCCEED;
}



/*-----------------------------------------------------------------------------
  Summary:
	Get B layer size
  Parameters:
	BLayer :	select B0 / B1 / all layer
	pTSize :	point of B layer memory size
  Note:
  Returns: 
  Remarks:
--------------------------------------------------------------------------------*/
VIM_RESULT VIM_DISP_GetBSize(VIM_DISP_LAYER BLayer,PTSize pTSize)
{
	if((BLayer !=VIM_DISP_B0LAYER)&&( BLayer != VIM_DISP_B1LAYER))
		return VIM_ERROR_DISP_BLAYER;	// Error

	if(BLayer & VIM_DISP_B0LAYER)
	{
		pTSize->cx = VIM_HIF_GetReg16(V5_REG_LCDC_BW0_L);
		pTSize->cy = VIM_HIF_GetReg16(V5_REG_LCDC_BH0_L);
	}
	else if(BLayer & VIM_DISP_B1LAYER)
	{
		pTSize->cx = VIM_HIF_GetReg16(V5_REG_LCDC_BW1_L);
		pTSize->cy = VIM_HIF_GetReg16(V5_REG_LCDC_BH1_L);
	}

	if(pTSize->cx)
	{
		pTSize->cx++;
	}
	if(pTSize->cy)
	{
		pTSize->cy++;
	}

	return VIM_SUCCEED;
}

/*-----------------------------------------------------------------------------
  Summary:
	Get B layer start point
  Parameters:
	BLayer :	select B0 / B1
	pTSize :	point of B layer memory size
  Note:
  Returns: 
  Remarks:
--------------------------------------------------------------------------------*/
VIM_RESULT VIM_DISP_GetBStartPoint(VIM_DISP_LAYER BLayer,PTPoint pStartPoint)
{
	if(BLayer > VIM_DISP_B1LAYER || BLayer < VIM_DISP_B0LAYER)
		return VIM_ERROR_DISP_BLAYER;	// Error

	if(BLayer == VIM_DISP_B0LAYER)
	{
		pStartPoint->x = VIM_HIF_GetReg16(V5_REG_LCDC_BX0_L);
		pStartPoint->y = VIM_HIF_GetReg16(V5_REG_LCDC_BY0_L);
	}
	else if(BLayer == VIM_DISP_B1LAYER)
	{
		pStartPoint->x = VIM_HIF_GetReg16(V5_REG_LCDC_BX1_L);
		pStartPoint->y = VIM_HIF_GetReg16(V5_REG_LCDC_BY1_L);
	}

	return VIM_SUCCEED;
}

/*-----------------------------------------------------------------------------
  Summary:
	set enable lcdc RGB 565 mode
  Parameters:
	bEnable :enable or not
  Note:
  Returns: 
  Remarks:
--------------------------------------------------------------------------------*/
void VIM_DISP_EnableVrgbMode(UINT8 bEnable)
{
	UINT8 byMode;
	byMode = VIM_HIF_GetReg8(V5_REG_LCDC_DE);
	byMode &= 0xfe;
	byMode |= (bEnable & 0x01);
	VIM_HIF_SetReg8(V5_REG_LCDC_DE, byMode);
}

/*-----------------------------------------------------------------------------
  Summary:
	set enable lcdc RGB 565 mode
  Parameters:
	bEnable :enable or not
  Note:
  Returns: 
  Remarks:
--------------------------------------------------------------------------------*/
void VIM_DISP_EnableDecodeSpecial(UINT8 bEnable)
{
	UINT8 byMode;
	byMode = VIM_HIF_GetReg8(V5_REG_LCDC_BR1_H);
	byMode &= 0x1f;
	byMode |= ((bEnable & 0x01)<<5);
	VIM_HIF_SetReg8(V5_REG_LCDC_BR1_H, byMode);
}

/*-----------------------------------------------------------------------------
  Summary:
	set enable lcdc RGB 565 mode
  Parameters:
	bEnable :enable or not
  Note:
  Returns: 
  Remarks:
--------------------------------------------------------------------------------*/
UINT8 VIM_DISP_GetDecodeSpecial(void)
{
	UINT8 byMode;
	byMode = VIM_HIF_GetReg8(V5_REG_LCDC_BR1_H);
	return (byMode>>5)&1;
}


