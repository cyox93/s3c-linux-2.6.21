/*
*                Copyright (C) 2005 Vimicro CO.,LTD     		 
*
* FILE NAME                                  VERSION                
*                                                                       
* VIM_LCDIF_Driver.c			   0.3
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*     5X 's LCDIF moudle sub driver                                 
*
*	Version		Author			Date		Description		
*  ---------------------------------------------------------------						 
*   	0.1			liuhd			2005-11-16	The first version. 
*     0.2			angela		2006-06-09	update for 528
*     0.3			guoying		2007-02-10	new lcd if info
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
const UINT8 gLcdcDelay[] = {	0x0e, 0xff,			//DEDLY  value   : 15 ~ 256
							0x01, 0x3e,			//DEINTV value   :  2 ~ 63
							0x0e, 0xff };		//LINEINTV value : 15 ~ 256

/********************************************************************************
  Description:
	  tools for caculate the delay inter value
  Parameters:
	void
  Returns:
  	void
  Remarks:
*********************************************************************************/

static UINT8 VIM_LCDIF_GetDeInterVal(UINT16 wcy)
{
	UINT8 len;

	len = (UINT8)(wcy & 0x1f);
	wcy >>= 5;
	len = (UINT8)(len +(wcy & 0x1f) + 1);
	wcy >>= 5;
	len = (UINT8)(len + (wcy & 0x1f));
	return len;
}
/********************************************************************************
  Description:
	  set write cycle and 3 delay value
  Parameters:
	void
  Returns:
  	void
  Remarks:
*********************************************************************************/

 void VIM_LCDIF_DispSetWrTiming(UINT16 x,UINT8 HeadNum)
{
	UINT8 i, j, delay[3];

	
	VIM_HIF_SetReg16(V5_REG_LCDIF_WCONT_L, x);
	delay[1] = (UINT8)(VIM_LCDIF_GetDeInterVal(x) + 1);
	delay[0] = (UINT8)(delay[1] * HeadNum);
	delay[2] = delay[1];
	if(VIM_BUSTYPE!=VIM_MULTI16)	//for 8-bit bus panel
	{
		delay[1] <<= 1;
		delay[2] <<= 1;
	}

	for(i = 0; i < 3; i++)
	{
		j = i << 1;
		delay[i] -= 1;
		delay[i] = delay[i] < gLcdcDelay[j] ? gLcdcDelay[j] : delay[i];
		delay[i] = delay[i] > gLcdcDelay[j+1] ? gLcdcDelay[j+1] : delay[i];
	}

	VIM_HIF_SetReg8(V5_REG_LCDC_DEDLY,delay[0]);
	VIM_HIF_SetReg8(V5_REG_LCDC_DEINTV,delay[1]);
	VIM_HIF_SetReg8(V5_REG_LCDC_LINEINTV,delay[2]);
	
}

/********************************************************************************
Description:
	init lcdif parm 
Parameters:
	 PTLcdifParm  parm
	 parm->rsflag:	when send head data,set RS signal value
	 parm->headnum:	when send head data,set head number
	 parm->stxp:		set position of the 1th start x address value in the HEAD_DATA register group. 		
	 parm->styp:		set position of the 1th start y address value in the HEAD_DATA register group. 
	 parm->endxp:	set position of the 1th end x address value in the HEAD_DATA register group. 
	 parm->endyp:	set position of the 1th end y address value in the HEAD_DATA register group. 

	 //if the start x, end x, start y, end y address value only use 1 register to store, then set Repeat_stxp, _styp, _endxp, _endyp=0.
	 parm->Repeat_stxp:	set position of the 2th start x address value in the HEAD_DATA register group.	//new parameter
	 parm->Repeat_styp:	set position of the 2th start y address value in the HEAD_DATA register group.
 	 parm->Repeat_endxp:	set position of the 2th end x address value in the HEAD_DATA register group.
	 parm->Repeat_endyp:	set position of the 2th end y address value in the HEAD_DATA register group.

	 parm->Head_Process_flag: if start x, end x, start y, end y address value need 2 registers to store then set this flag=0x03.
	 parm->Start_x_comm_code: set the address of register which stroes start x value.
	 parm->End_x_comm_code: set the address of register which stroes end x value.
	 parm->Start_y_comm_code: set the address of register which stroes start y value.
	 parm->End_y_comm_code: set the address of register which stroes end y value.
	  
	 parm->fmt:         Set display format and mapping of the image data
	 parm->wcy:		Set the write access parameter for built-on SRAM type LCD access
	 parm->dedelay:	The number of clock cycles from address enable assertion to data enable assertion. The min value is 15 and the max value is 255
	 parm->deinteval:	The minimum number of clock cycles between the two data enable signals. The min value is 2 and the max value is 63. 
	 parm->lineinteval:  he number of clock cycles from the last data enable assertion to the address enable assertion in the next line. The min value is 15 and the max value is 255.
	 parm->rcy: 	 	Set the read access parameter for built-on SRAM type LCD access
  Returns:
  	void
  Remarks:
  *********************************************************************************/

void VIM_LCDIF_InitLcdIFParm(PTLcdifParm parm)
{
	UINT8 i;
	VIM_HIF_SetReg16(V5_REG_LCDIF_HEAD_RS_FLAG_L, parm->rsflag);
	VIM_HIF_SetReg16(V5_REG_LCDIF_HEAD_NUM_L,  parm->headnum );
	VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, parm->stxp);
	VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, parm->styp);
	VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, parm->endxp);
	VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, parm->endyp);

	if((parm->SpecialLCD_flag)==1)											//wendy guo 1/24/2007
	{
		VIM_HIF_SetReg16(V5_REG_LCDIF_RPT_START_X, parm->Repeat_stxp);
		VIM_HIF_SetReg16(V5_REG_LCDIF_RPT_START_Y, parm->Repeat_styp);
		VIM_HIF_SetReg16(V5_REG_LCDIF_RPT_END_X, parm->Repeat_endxp);
		VIM_HIF_SetReg16(V5_REG_LCDIF_RPT_END_Y, parm->Repeat_endyp);
	}


	if( parm->config) //& V5B_LCDIF_CS1_SEL)  //angela changed it 
		VIM_HIF_SwitchPanelCs(VIM_LCDIF_CS2);	//cs2 enable
	else
		VIM_HIF_SwitchPanelCs(VIM_LCDIF_CS1);	//cs1 enable

	VIM_HIF_SetReg8(V5_REG_LCDIF_DMAP, parm->fmt);
#if DELAY_USE_INFO==0  // not use the lcdifo value
	VIM_LCDIF_DispSetWrTiming(parm->wcy,parm->headnum);
#else
	VIM_HIF_SetReg16(V5_REG_LCDIF_WCONT_L, parm->wcy);
	VIM_HIF_SetReg8(V5_REG_LCDC_DEDLY,parm->dedelay);
	VIM_HIF_SetReg8(V5_REG_LCDC_DEINTV,parm->deinteval);
	VIM_HIF_SetReg8(V5_REG_LCDC_LINEINTV,parm->lineinteval);
#endif
	VIM_HIF_SetReg16(V5_REG_LCDIF_RCONT_L, parm->rcy);
	
	//set the head value according to the head request.
	for(i=0;i<parm->headnum;i++)
	{
		VIM_HIF_SetReg16(V5_REG_LCDIF_HEAD_DATA0 + (i << 1), parm->head[i] );
	}


	if((parm->SpecialLCD_flag)==1)																//wendy guo 1/24/2007
		{

			VIM_HIF_SetReg16(V5_REG_LCDIF_HEAD_PROC_FLG, parm->Head_Process_flag);				//0x03
			VIM_HIF_SetReg16(V5_REG_LCDIF_COMM_START_X, parm->Start_x_comm_code);			//00000000,0001,0000
			VIM_HIF_SetReg16(V5_REG_LCDIF_COMM_END_X, parm->End_x_comm_code);
			VIM_HIF_SetReg16(V5_REG_LCDIF_COMM_START_Y, parm->Start_y_comm_code);			//00000000,0007,0006
			VIM_HIF_SetReg16(V5_REG_LCDIF_COMM_END_Y, parm->End_y_comm_code);		
		}
	
	// head enable when set 1
	VIM_HIF_SetReg8(V5_REG_LCDIF_HEAD_CONFIG_L, ENABLE);
	
	//update the all position of x and y to the register. It will set 1 after setting all other register
	VIM_HIF_SetReg8(V5_REG_LCDIF_HEAD_CONFIG_L, 0x9);

	//V5B_LcdifSetBiasSpt(parm->biaspt);	//angela changed it
}

/********************************************************************************
  Description:
	  lcd reset signal. Low active
  Parameters:
	void
  Returns:
  	void
  Remarks:
*********************************************************************************/
void VIM_LCDIF_ResetPanel(void)
{
#ifndef CONFIG_MACH_CANOPUS
	VIM_HIF_SetReg8(V5_REG_LCDIF_LCD_RESET, 0x0);
//#ifndef NCSC
	VIM_USER_DelayMs(100);
//#endif
	VIM_HIF_SetReg8(V5_REG_LCDIF_LCD_RESET, 1);
//#ifndef NCSC
	VIM_USER_DelayMs(20);	//wait signal stable to avoid writing reg fail.
//#endif
#endif
}


/********************************************************************************
  Description:
	 Set output data to this register.  the data put in this register is output through LCD_D[15-0].  
	 During this process LCD_RS signal is kept in Low.
  Parameters:
	uVal:	 the data put in this register
  Returns:
  	void
  Remarks:
*********************************************************************************/
void VIM_LCDIF_RsLExW(UINT16 uVal)
{
	VIM_HIF_SetReg16(V5_REG_LCDIF_EXW0_L, uVal);
	VIM_HIF_SetReg8(V5_REG_LCDIF_EXW_EN, 0x1);
}


/********************************************************************************
  Description:
	 Set output data to this register.  the data put in this register is output through LCD_D[15-0].  
	 During this process LCD_RS signal is kept in High
	 Parameters:
	uVal:	 the data put in this register
  Returns:
  	void
  Remarks:
*********************************************************************************/
void VIM_LCDIF_RsHExW(UINT16 uVal)
{
	VIM_HIF_SetReg16(V5_REG_LCDIF_EXW1_L, uVal);
	VIM_HIF_SetReg8(V5_REG_LCDIF_EXW_EN, 0x2);
}

/********************************************************************************
  Description:
	 Read current data on DIS_D[15-0] from this register,LCD_RS signal is kept in Low
 Parameters:
	void
  Returns:
  	the data  in this register
  Remarks:
*********************************************************************************/

UINT16 VIM_LCDIF_RsLExR(void)
{
	VIM_HIF_SetReg8(V5_REG_LCDIF_EXRS, 0x0);
	return (UINT16)VIM_HIF_GetReg16(V5_REG_LCDIF_EXR0_L);
}

/********************************************************************************
  Description:
	 Read current data on DIS_D[15-0] from this register,LCD_RS signal is kept in High
 Parameters:
	void
  Returns:
  	the data  in this register
  Remarks:
*********************************************************************************/
UINT16 VIM_LCDIF_RsHExR(void)
{
	VIM_HIF_SetReg8(V5_REG_LCDIF_EXRS, 0x1);
	return (UINT16)VIM_HIF_GetReg16(V5_REG_LCDIF_EXR0_L);
}



/********************************************************************************
  Description:
	set the x,y bias value
 Parameters:
	TPoint pt:  set the x/y bias value
  Returns:
  	void
  Remarks:
*********************************************************************************/
void VIM_LCDIF_SetBiasSpt(TPoint pt)
{
	VIM_HIF_SetReg8(V5_REG_LCDIF_BIAS_X, (UINT8)pt.x );
	VIM_HIF_SetReg8(V5_REG_LCDIF_BIAS_Y, (UINT8)pt.y );
	
}


