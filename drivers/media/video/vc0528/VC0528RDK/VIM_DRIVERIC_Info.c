/*************************************************************************
*                                                                       
*             Copyright (C) 2005 Vimicro CO.,LTD     		 
*
* FILE NAME                                  VERSION                
*                                                                       
* VIM_DRIVERIC_Info.c					           	  0.1                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*    VC0578 LCD driver ic information file                        
*
*	Version		Author			Date		Description		
*  ---------------------------------------------------------------						 
*   0.1			liuhuadian  			2005-11-01	The first version. 
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
#include "VIM_DRIVERIC_Info.h"

#if V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_HD66773_16BIT
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
	0xfffa,		//rsflag
	3,			//head num
	{0x21, 0, 0x22},
	3, 4, 0, 0,	//stxp, styp, endxp, endyp;
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt
	0x460, 0x460, 
	//0xce0, 0x460,  
	35,11,14,

	NULL
	//0x1580,0x460, //for bird  s6d0123
	//56,19,19
};
#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_HD66773_8BIT
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
	0xffcc,		//rsflag
	6,			//head num
	{0,0x21, 0, 0,0,0x22},
	7, 5, 0, 0,	//
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
    
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_2TIMES|PANEL_RGB,		//fmt
	0x460, 0x460,
	0x47,0x0d,0x0b,

	NULL
};
#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_S6D0110_16BIT
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
	0xfffa,		//rsflag
	3,			//head num
	{0x21, 0, 0x22},
	3,4, 0, 0,	//startx starty endx endy
   	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt
	0x460, 0x460,
	0x3C,0x0d,0x0b,

	NULL
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_S6D0118_8BIT
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
	0xffcc,		//rsflag
	6,			//head num
	{0,0x21, 0, 0,0,0x22},
	7,5, 0, 0,	//startx starty endx endy
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_2TIMES|PANEL_RGB,		//fmt
	0x460, 0x460,
	0x3C,0x0d,0x0b,

	NULL
};
#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_HD66777_16BIT
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
	0xffea,		//rsflag
	5,			//head num
	{0x20, 0, 0x21, 0, 0x22},
	3, 7, 0, 0,	//
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,

	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt
	0x460, 0x460,
	35,11,14,

	NULL
};
#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_LDS183_16BIT
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
     0xffb6,            //rsflag
     7,                 //head num
     {0x002a, 0x0000, 0x0000, 0x002b, 0x0000, 0x0000, 0x002c},
     3, 9, 5, 11,  //stxp, styp, endxp, endyp
    0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    COMMAND_ONLY,
    0x0, 0x0, 0x0, 0x0,
	
     MAINPANEL,                 //config
     PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt,
     0x460, 0x880,
     0x2a,0x06,0x06,

	NULL
};
#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_LDS183_8BIT
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
     0xffb6,            //rsflag
     7,                 //head num
     {0x002a, 0x0000, 0x0000, 0x002b, 0x0000, 0x0000, 0x002c},
     3, 9, 5, 11,  //stxp, styp, endxp, endyp
    0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    COMMAND_ONLY,
    0x0, 0x0, 0x0, 0x0,
	
     MAINPANEL,                 //config
     PANEL_FMT565_2TIMES|PANEL_RGB,		//fmt,
     0x460, 0x880,
     0x54,0x0c,0x0c,

	NULL
};
#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_PL2001A_16BIT
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
     0xfffa,            //rsflag
     4,                 //head num
     {0x0018, 0x0000, 0x0019, 0x0000},
     3, 7, 0, 0,  //stxp, styp, endxp, endyp
    0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    COMMAND_ONLY,
    0x0, 0x0, 0x0, 0x0,
	
     MAINPANEL,                 //config
     PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt,
     0x461, 0x7ca0,
     0x37,0x0d,0x0e,

	NULL
};
#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_S6B33B2_S6B33BG_16BIT
const TLcdifParm gDriverIc_LcdIF =
{
     0x00,
     0xffc0,            //rsflag
     6,                 //head num
     {0x0043, 0x0000, 0x0000, 0x0042, 0x0000, 0x0000},
     3, 9, 5, 11,  //stxp, styp, endxp, endyp
     0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
     COMMAND_ONLY,
     0x0, 0x0, 0x0, 0x0,
	
     MAINPANEL,                 //config
     PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt,
     0x460, 0x7ca0,
     0x40,0x08,0x02,

	NULL
};
#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_S6B33B2_S6B33BG_8BIT
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
     0xffc0,            //rsflag
     6,                 //head num
     {0x43, 0,0, 0x42,0,0},
     3, 9, 5, 11,  //stxp, styp, endxp, endyp
    0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    COMMAND_ONLY,
    0x0, 0x0, 0x0, 0x0,
	
     MAINPANEL,                 //config
     PANEL_FMT565_2TIMES|PANEL_RGB,		//fmt,
     0x460, 0x7ca0,
     0x40,0x8,0x02,

	NULL
};
#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_HD66772_HD66774_16BIT
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
	0xfffa,		//rsflag
	3,			//head num
	{0x21, 0, 0x22},
	3, 4, 0, 0,	//stxp, styp, endxp, endyp;
   	 0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt
	0x460, 0x460,
	0x11,0x5,0xe,	//UINT8 dedelay,deinteval,lineinteval;
	NULL	//UINT8 dedelay,deinteval,lineinteval;
};
#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_HX8301_HX8609A_16BIT
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
	0xfffa,		//rsflag
	3,			//head num
	{0x21, 0, 0x22},
	3, 4, 0, 0,	//stxp, styp, endxp, endyp;
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt
	0x460, 0x460,
	0x11,0x5,0xe,	//UINT8 dedelay,deinteval,lineinteval;
	NULL	//UINT8 dedelay,deinteval,lineinteval;
};
#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_SSD1781_8BIT
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
     0xffb6,            //rsflag
     7,                 //head num
     {0x0015,0x0000,0x0000,0x0075,0x0000,0x0000,0x005c},
    3, 9, 5, 11,  //stxp, styp, endxp, endyp
    0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    COMMAND_ONLY,
    0x0, 0x0, 0x0, 0x0,
	
    MAINPANEL,                 //config
    PANEL_FMT565_2TIMES|PANEL_RGB,              //fmt : 8bit bus/RGB565
    0x881, 0x7ca0,
    0x5f,0x1f,0x1f,

	NULL
};
#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_HD66781S_HD66783_16BIT
const TLcdifParm gDriverIc_LcdIF =
{
 0x00,
 0xffea,  //rsflag
 5,   //head num
 {0x200, 0x000, 0x201, 0x000, 0x202},
 3, 0x0807, 0, 0, //stxp, styp, endxp, endyp;
  0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
  COMMAND_ONLY,
  0x0, 0x0, 0x0, 0x0,
 
 MAINPANEL,  //main panel or sub panel
 PANEL_FMT565_1TIMES|PANEL_RGB,  //fmt
 0x460, 0x460,
 0x1d,0x5,0xe, //UINT8 dedelay,deinteval,lineinteval;
	NULL //UINT8 dedelay,deinteval,lineinteval;
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_S6D0110A_8BIT
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
	0xffcc,		//rsflag
	6,			//head num
	{0,0x21, 0, 0,0,0x22},
	7,5, 0, 0,	//startx starty endx endy
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_2TIMES|PANEL_RGB,		//fmt
	0x460, 0x460,
	0x3C,0x0d,0x0b,

	NULL
};
#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_uPD161801_uPD161861_16BIT
const TLcdifParm gDriverIc_LcdIF =
{
    0,
    0xfff0,            //rsflag
    4,                 //head num
    {0x0600,0x0000,0x0700,0x0000},
    3, 0x0807, 0, 0,  //stxp, styp, endxp, endyp
    0,0,0,0,
    COMMAND_ONLY,
    0,0,0,0,
    MAINPANEL,                 //config
    PANEL_FMT565_1TIMES|PANEL_RGB,            
    0x420, 0xce0,	//---lhd test
    17,5,14,

	NULL
};
#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_S1D19102_S1D17D01_16BIT
const TLcdifParm gDriverIc_LcdIF =
{
	0,
	0xfeee,  //rsflag
	9,   //head num
	{0x1500, 0x0000, 0x0000,0x0000,0x7500, 0x000, 0x0000, 0x0000,0x5c00},
	4, 0x0608, 12,0x0e10,//, 0x0e10, //stxp, styp, endxp, endyp;
	0,0,0,0,
	COMMAND_ONLY,
	0,0,0,0, 
	MAINPANEL,  //main panel or sub panel
	PANEL_FMT565_1TIMES|PANEL_RGB,//fmt
	
	0x461, 0x460,			//Heju modified! for 45m cpu clock
	52,6,0,

	
	NULL
	
};
#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_S1D19120_16BIT
void LcdRotate_S1D19120(UINT8 degree, UINT8 WorkMode)
{
	if ((VIM_HAPI_MODE_BYPASS == WorkMode) || (VIM_HAPI_MODE_DIRECTDISPLAY ==WorkMode))
	{
		switch(degree)
		{
			case VIM_HAPI_ROTATE_0:
				VIM_DISP_BySetRSlow((UINT16)0xbc<<8);
				VIM_DISP_BySetRShigh((UINT16)0x00<<8);
				break;
				
			case VIM_HAPI_ROTATE_90:
				VIM_DISP_BySetRSlow((UINT16)0xbc<<8);
				VIM_DISP_BySetRShigh((UINT16)0x09<<8);				
				break;
				
			case VIM_HAPI_ROTATE_180:
				VIM_DISP_BySetRSlow((UINT16)0xbc<<8);
				VIM_DISP_BySetRShigh((UINT16)0x03<<8);				
				break;
				
			case VIM_HAPI_ROTATE_270:
				VIM_DISP_BySetRSlow((UINT16)0xbc<<8);
				VIM_DISP_BySetRShigh((UINT16)0x0a<<8);
				break;
				
			case VIM_HAPI_MIRROR_0:
				VIM_DISP_BySetRSlow((UINT16)0xbc<<8);
				VIM_DISP_BySetRShigh((UINT16)0x02<<8);				
				break;
				
			case VIM_HAPI_MIRROR_90:
				VIM_DISP_BySetRSlow((UINT16)0xbc<<8);
				VIM_DISP_BySetRShigh((UINT16)0x08<<8);				
				break;
				
			case VIM_HAPI_MIRROR_180:	
				VIM_DISP_BySetRSlow((UINT16)0xbc<<8);
				VIM_DISP_BySetRShigh((UINT16)0x01<<8);				
				break;
				
			case VIM_HAPI_MIRROR_270:
				VIM_DISP_BySetRSlow((UINT16)0xbc<<8);
				VIM_DISP_BySetRShigh((UINT16)0x0b<<8);				
				break;
				
			
		}

	
	}
	else
	{
		switch(degree)
		{
			case VIM_HAPI_ROTATE_0:
				VIM_LCDIF_RsLExW((UINT16)0xbc<<8);
				VIM_LCDIF_RsHExW((UINT16)0x00<<8);

				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 4);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 0x0608);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 12);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 0x0e10);
				break;
				
			case VIM_HAPI_ROTATE_90:
				VIM_LCDIF_RsLExW((UINT16)0xbc<<8);
				VIM_LCDIF_RsHExW((UINT16)0x09<<8);

				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 0x0608);				//exchange  parm->styp and  parm->stxp
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 4);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 0x0e10);				//exchange
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 12);
		
				break;
				
			case VIM_HAPI_ROTATE_180:
				VIM_LCDIF_RsLExW((UINT16)0xbc<<8);
				VIM_LCDIF_RsHExW((UINT16)0x03<<8);
				
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 4);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 0x0608);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 12);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 0x0e10);
				
				break;
				
			case VIM_HAPI_ROTATE_270:
				VIM_LCDIF_RsLExW((UINT16)0xbc<<8);
				VIM_LCDIF_RsHExW((UINT16)0x0a<<8);

				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 0x0608);				//exchange  parm->styp and  parm->stxp
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 4);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 0x0e10);				//exchange
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 12);
				
				break;
				
			case VIM_HAPI_MIRROR_0:
				VIM_LCDIF_RsLExW((UINT16)0xbc<<8);
				VIM_LCDIF_RsHExW((UINT16)0x02<<8);
		
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 4);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 0x0608);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 12);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 0x0e10);
				break;
				
			case VIM_HAPI_MIRROR_90:
				VIM_LCDIF_RsLExW((UINT16)0xbc<<8);
				VIM_LCDIF_RsHExW((UINT16)0x08<<8);

				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 0x0608);				//exchange  parm->styp and  parm->stxp
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 4);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 0x0e10);				//exchange
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 12);
				
				break;
				
			case VIM_HAPI_MIRROR_180:
				VIM_LCDIF_RsLExW((UINT16)0xbc<<8);
				VIM_LCDIF_RsHExW((UINT16)0x01<<8);
		
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 4);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 0x0608);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 12);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 0x0e10);		
				break;
				
			case VIM_HAPI_MIRROR_270:
				VIM_LCDIF_RsLExW((UINT16)0xbc<<8);
				VIM_LCDIF_RsHExW((UINT16)0x0b<<8);

				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 0x0608);				//exchange  parm->styp and  parm->stxp
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 4);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 0x0e10);				//exchange
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 12);
				break;
				
			default:
				break;
		}

	
	}
	
return;	
}
const TLcdifParm gDriverIc_LcdIF =
{
#if 1
 0x00,
 0xfeee,  //rsflag
 9,   //head num
 {0x1500, 0x0000, 0x0000,0x0000,0x7500, 0x000, 0x0000, 0x0000,0x5c00},
 4, 0x0608, 12,0x0e10,//, 0x0e10, //stxp, styp, endxp, endyp;
    0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    COMMAND_ONLY,
    0x0, 0x0, 0x0, 0x0,
 
 MAINPANEL,  //main panel or sub panel
 PANEL_FMT565_1TIMES|PANEL_RGB,//fmt


 0x461, 0x460,			//Heju modified! for 45m cpu clock
 52,6,0,



LcdRotate_S1D19120

#else
	 0x00,
	 0xffee,  //rsflag
	 5,
	{0x1500, 0x0000, 0x0000,0x0000,0x5c00},
	 0x0608, 4, 12,0x0e10,//, 0x0e10, //stxp, styp, endxp, endyp;
	 0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
	 COMMAND_ONLY,
	 0x0, 0x0, 0x0, 0x0,
	 
	 MAINPANEL,  //main panel or sub panel
	 PANEL_FMT565_1TIMES|PANEL_RGB,//fmt
	// 0x421, 0x460,
	// 25,2,0

	 0x421, 0x460,
	 60,5,0,
	 LcdRotate_S1D19120
#endif
};
#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_R61508_16BIT
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
	0xfffa,		//rsflag
	3,			//head num
	{0x21, 0, 0x22},
	3, 4, 0, 0,	//stxp, styp, endxp, endyp;
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,	
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt
	0x460, 0x460, 
	35,11,14,

	NULL
};
#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_ST7636_8BIT
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
     0xffb6,            //rsflag
     7,                 //head num
     {0x0015,0x0000,0x0000,0x0075,0x0000,0x0000,0x005c},
    3, 9, 5, 11,  //stxp, styp, endxp, endyp
    0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    COMMAND_ONLY,
    0x0, 0x0, 0x0, 0x0,
	
    MAINPANEL,                 //config
    PANEL_FMT565_2TIMES|PANEL_RGB,              //fmt : 8bit bus/RGB565
    0x881, 0x460,
    0x5f,0x1f,0x1f,

	NULL
};
#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_S1D19105D_16BIT
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
       0xffb6,            //rsflag
     7,                 //head num
     {0x1500,0x0000,0x0000,0x7500,0x0000,0x0000,0x5c00},
    4, 6, 10, 12,  //stxp, styp, endxp, endyp
    0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    COMMAND_ONLY,
    0x0, 0x0, 0x0, 0x0,
	
    MAINPANEL,                 //config
    PANEL_FMT565_1TIMES|PANEL_RGB,              //fmt : 16bit bus/RGB565
    0x881, 0x7ca0,
    0x5f,0x1f,0x1f,

	NULL
};
#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_uPD161963_8BIT
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
   	0xffcc,		//rsflag
	6,			//head num
	{0,0x21, 0, 0,0,0x22},
	7,5, 0, 0,	//startx starty endx endy
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_2TIMES|PANEL_RGB,		//fmt
	0x460, 0x460,
	0x3C,0x0d,0x0b,

	NULL
};
#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_UPD161690_8BIT
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
   	0xffea,		//rsflag
	5,			//head num
	{0x03, 0, 0x05,0,0x0b},
	3,7, 0, 0,	//startx starty endx endy
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT444_2TIMES_BIG|PANEL_RGB,		//fmt
	0x881, 0x460,
	    0x5f,0x1f,0x1f,

	NULL
};
#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_S1D19111D_8BIT
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
    0xffb6,            //rsflag
     7,                 //head num
    {0x15,0x00,0x00,0x75,0x00,0x00,0x5c},
    3, 5, 9, 11,  //stxp, styp, endxp, endyp
    0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    COMMAND_ONLY,
    0x0, 0x0, 0x0, 0x0,
	
    MAINPANEL,                 //config
    PANEL_FMT565_2TIMES|PANEL_RGB,              //fmt : 16bit bus/RGB565
   
   0x460, 0x460,
  0x3C,0x0d,0x0b,

	NULL

};
#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_LGDP4511_16BIT
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
    	0xfffa,		//rsflag
	3,			//head num
	{0x21, 0, 0x22},
	3, 4, 0, 0,	//stxp, styp, endxp, endyp;
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt
	0xce0, 0x460,  
	35,11,14,

	NULL
};
#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_TL1711_16BIT
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
   	0xfffa,		//rsflag
	3,			//head num
	{0x21, 0, 0x22},
	3, 4, 0, 0,	//stxp, styp, endxp, endyp;
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt
	0xce0, 0x460,  
	35,11,14,

	NULL
};
#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_HX8309_16BIT
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
   	0xfffa,		//rsflag
	3,			//head num
	{0x21, 0, 0x22},
	3, 4, 0, 0,	//stxp, styp, endxp, endyp;
   	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt
	0xce1, 0x460,  
	35,11,14,

	NULL
};
#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_S6B33B9_16BIT
const TLcdifParm gDriverIc_LcdIF =
{
    0x00,
    0xffc0,            //rsflag
    6,                 //head num
    {0x0043, 0x0000, 0x0000, 0x0042, 0x0000, 0x0000},
    3, 9, 5, 11,  //stxp, styp, endxp, endyp
    0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    COMMAND_ONLY,
    0x0, 0x0, 0x0, 0x0,
	
     MAINPANEL,                 //config
     PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt,
     0x880, 0x7ca0,
     0x30,0x08,0x08,

	NULL
};
 #elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_S6D0123_16BIT
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
	0xfffa,		//rsflag
	3,			//head num
	{0x21, 0, 0x22},
	3,4, 0, 0,	//startx starty endx endy
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt
   //	0xce1, 0x7ca0,
    // 0x5f,0x0f,0x0f
    0x461, 0x460,
	0x3C,0x0d,0x0b,

	NULL
};
 #elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_LDS506_8BIT
const TLcdifParm gDriverIc_LcdIF =
{
#if 0
    0xffd6,            //rsflag
    7,                 //head num
    {0x06,0x00,0x00,0x07,0x00,0x00,0x08},
    0x03, 0x09, 0x05, 0x0b,  //stxp, styp, endxp, endyp
    MAINPANEL,                 //config
    PANEL_FMT565_1TIMES|PANEL_RGB,            
    0x420, 0xce0,	//---lhd test
    19,3,14
 #else
    0x00,
    0xffb6,            //rsflag
    7,                 //head num
    {0x0110,0,0,0x0118,0,0x0200},
    0, 0, 0, 0x0,  //stxp, styp, endxp, endyp
    0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    COMMAND_ONLY,
    0x0, 0x0, 0x0, 0x0,
 
    MAINPANEL,                 //config
    PANEL_FMT565_1TIMES|PANEL_RGB,            
    0x461, 0xce0,	//---lhd test
    0x35,0x0d,14,

	NULL
 #endif
};
 #elif  V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_LGDP4216_16BIT
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
	0xffea,		//rsflag
	5,			//head num
	{0x07, 0, 0x06,0,0x05},
	3, 7, 0, 0,	//stxp, styp, endxp, endyp;
   	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
       0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt
	 0x881, 0x7ca0,
    	0x5f,0x1f,0x1f,

	NULL
};
 #elif  V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_R61500_16BIT
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
	0xfffa,		//rsflag
	3,			//head num
	{0x21, 0, 0x22},
	3, 4, 0, 0,	//stxp, styp, endxp, endyp;
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt
	//0xce0, 0X460,  
	//35,11,14
	  0x461, 0x460,
	0x3C,0x0d,0x0b,

	NULL
};
 #elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_IS2320_16BIT
const TLcdifParm gDriverIc_LcdIF =
{
    0x00,
    0xffea,            //rsflag
    6,                 //head num
    {0x42, 0x00, 0x43, 0x00, 0x44,0x00},
    3, 0x070b, 0, 0,  //stxp, styp, endxp, endyp
    0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    COMMAND_ONLY,
    0x0, 0x0, 0x0, 0x0,

     MAINPANEL,                 //config
     PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt,
     0x880, 0x7ca0,
     0x30,0x08,0x08,

	NULL
};
 #elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_C1E2_04_16BIT
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
    0xfffa,            //rsflag
    4,                 //head num
    {0x07,0x00,0x08,0x00},
    3, 7, 0, 0,  //stxp, styp, endxp, endyp
    0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    COMMAND_ONLY,
    0x0, 0x0, 0x0, 0x0,
	
    MAINPANEL,                 //config
    PANEL_FMT565_1TIMES|PANEL_RGB,            
    0x881, 0x7ca0,
    0x5f,0x1f,0x1f,

	NULL
};
 #elif  V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_R61503B_16BIT
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
	0xffea,		//rsflag
	5,			//head num
	{0x20, 0, 0x21,0,0x22},
	3, 7, 0, 0,	//stxp, styp, endxp, endyp;
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt
	//0xce0, 0X460,  
	//35,11,14
	  0x461, 0x460,
	0x3C,0x0d,0x0b,

	NULL
};
 #elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_S6D0118_16BIT
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
	0xfffa,		//rsflag
	3,			//head num
	{0x21, 0, 0x22},
	3,4, 0, 0,	//startx starty endx endy
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt
	0x460, 0x460,
	0x3C,0x0d,0x0b,

	NULL
};
 #elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_HX8312A_16BIT
const TLcdifParm gDriverIc_LcdIF =
{
    0x00,
    0xfff8,            //rsflag
    3,                 //head num
    {0x4200, 0x4300, 0x4400},
    1, 0x0305, 0, 0,  //stxp, styp, endxp, endyp
    0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    COMMAND_ONLY,
    0x0, 0x0, 0x0, 0x0,

     MAINPANEL,                 //config
     PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt,
     0x880, 0x7ca0,
     0x30,0x08,0x08,

	NULL
};
 #elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_S6B33BC_16BIT
const TLcdifParm gDriverIc_LcdIF =
{
    0x00,
    0xffc0,            //rsflag
    6,                 //head num
    {0x0043, 0x0000, 0x0000, 0x0042, 0x0000, 0x0000},
    3, 9, 5, 11,  //stxp, styp, endxp, endyp
    0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    COMMAND_ONLY,
    0x0, 0x0, 0x0, 0x0,
	
     MAINPANEL,                 //config
     PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt,
     0x880, 0x7ca0,
     0x30,0x08,0x08,

	NULL
};
 #elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_S6B33BC_8BIT
const TLcdifParm gDriverIc_LcdIF =
{
    0x00,
    0xffc0,            //rsflag
    6,                 //head num
    {0x43, 0x00, 0x00, 0x42, 0x00, 0x00},
    3, 9, 5, 11,  //stxp, styp, endxp, endyp
    0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    COMMAND_ONLY,
    0x0, 0x0, 0x0, 0x0,
	
     MAINPANEL,                 //config
     PANEL_FMT565_2TIMES|PANEL_RGB,		//fmt,

     0x880, 0x7ca0,
     0x5f,0x0f,0x0f,

	NULL
};
 #elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_IS2102_IS2202_16BIT
const TLcdifParm gDriverIc_LcdIF =
{
     0x00,
     0xfff8,            //rsflag
     3,                 //head num
     {0x4200, 0x4300, 0x4400},
     1, 0x0305, 0, 0,  //stxp, styp, endxp, endyp
     0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
     COMMAND_ONLY,
     0x0, 0x0, 0x0, 0x0,
	
     MAINPANEL,                 //config
     PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt,
     0x880, 0x7ca0,
     0x30,0x08,0x08,

	NULL
};
 #elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_TL1772_8BIT
const TLcdifParm gDriverIc_LcdIF =
{
 0x00,
 0xffcc,  //rsflag
 6,   //head num
 {0,0x21, 0, 0,0,0x22},
 7, 5, 0, 0,  //stxp, styp, endxp, endyp
  0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
  COMMAND_ONLY,
  0x0, 0x0, 0x0, 0x0,
 
 MAINPANEL,  //main panel or sub panel
 PANEL_FMT565_2TIMES|PANEL_RGB,  //fmt
 0x460, 0x460,
 0x47,0x0d,0x0b,

	NULL
};
 #elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_S6D0144_16BIT
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
	0xfffa,		//rsflag
	3,			//head num
	{0x21, 0, 0x22},
	3,4, 0, 0,	//startx starty endx endy
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt
	0x460, 0x460,
	0x3C,0x0d,0x0b,

	NULL
};
  #elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_S6D0134_16BIT
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
	0xfffa,		//rsflag
	3,			//head num
	{0x21, 0, 0x22},
	3,4, 0, 0,	//startx starty endx endy
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt
	0xce1, 0x460,
	60,12,14,

	NULL
};
 #elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_HX8309_8BIT
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
   	0xffcc,		//rsflag
	6,			//head num
	{0,0x21, 0,0, 0,0x22},
	7, 5, 0, 0,	//stxp, styp, endxp, endyp;
   	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
       0x0, 0x0, 0x0, 0x0,

	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_2TIMES|PANEL_RGB,		//fmt
	0xce1, 0x460,  
	35,11,14,

	NULL
};
 #elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_SPFD5414_16BIT
const TLcdifParm gDriverIc_LcdIF =
{
 0x00,
 0xfbde,  //rsflag
 11,   //head num
 {0x002a, 0x0000, 0x0000,0x0000, 0x0000, 0x002b, 0x0000, 0x0000, 0x0000, 0x0000,0x002c},
  0x0305, 0x0d0f, 0x0709, 0x1113,  //stxp, styp, endxp, endyp
  0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
  COMMAND_ONLY,
  0x0, 0x0, 0x0, 0x0,
 
 MAINPANEL,  //main panel or sub panel
 PANEL_FMT565_1TIMES|PANEL_RGB,  //fmt
 0x460, 0x460,
 0x3C,0x0d,0x0b,

	NULL
};
#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_SSD1289_16BIT		//agnela


const TLcdifParm gDriverIc_LcdIF =
{
  0x00,
  0xffea,  //rsflag
  5,   //head num
  {0x004e, 0x0000, 0x004f,0x0000,0x0022},
  // 2,6, 3,10,//, 0x0e10, //stxp, styp, endxp, endyp;
  3, 0X0807, 0, 0, //stxp, styp, endxp, endyp; 
   0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
   COMMAND_ONLY,
   0x0, 0x0, 0x0, 0x0,
  
  MAINPANEL,  //main panel or sub panel
  PANEL_FMT565_1TIMES|PANEL_RGB,//fmt
  0x440, 0x460,
  // 0x880, 0x7ca0,
 33,11,14,

	NULL
};


#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_SPFD54126_16BIT
const TLcdifParm gDriverIc_LcdIF =
{
 0x00,
 0xfbde,  //rsflag
 11,   //head num
 {0x002a, 0x0000, 0x0000,0x0000, 0x0000, 0x002b, 0x0000, 0x0000, 0x0000, 0x0000,0x002c},
  0x0305, 0x0d0f, 0x0709, 0x1113,  //stxp, styp, endxp, endyp
  0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
  COMMAND_ONLY,
  0x0, 0x0, 0x0, 0x0,
  
 MAINPANEL,  //main panel or sub panel
 PANEL_FMT565_1TIMES|PANEL_RGB,  //fmt
 0x460, 0x460,
 50,7,49,

	NULL
};
 
#elif  V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_R61503B_8BIT
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
	0xfccc,		//rsflag
	10,			//head num
	{0,0x20, 0,0, 0,0x21,0,0,0,0x22},
	7, 15, 0, 0,	//stxp, styp, endxp, endyp;
       0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
       0x0, 0x0, 0x0, 0x0,

	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_2TIMES|PANEL_RGB,		//fmt

	0x420, 0x460,
	0x3C,0x3,0x0b,

	NULL
};
#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_UC1697_8BIT		//凌达1/23/2007
const TLcdifParm gDriverIc_LcdIF =	
{
    0x01,							//special LCD flag
    0xfff0,            //rsflag			//because RS=0, send the control register, so 1100,0000
    4,                 //head num
    {0x00,0x00,0x00,0x00},
     1, 5, 0, 0, //stxp, styp, endxp, endyp; 
     3, 7, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    COMMAND_VALUE_ONEBYTE|REPEAT_COORDINATE,
    0x0010, 0x0000, 0x0076, 0x0000,
    MAINPANEL,                 //config
    PANEL_FMT565_2TIMES|PANEL_RGB,              //fmt : 8bit bus/RGB565
    0x460, 0x460,
    0x47,0x1d, 0x1b,

	NULL
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_TL1763_16BIT	//韶丰TL1763
const TLcdifParm gDriverIc_LcdIF =
{
 0x00,			//special LCD flag
 0xffea,  		//rsflag
 5,   				//head num
 {0x20, 0, 0x21, 0, 0x22},
  3, 0x0807,0,0,  
  0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
  COMMAND_ONLY,
  0x0, 0x0, 0x0, 0x0,
       
 MAINPANEL, 		 //main panel or sub panel
 PANEL_FMT565_1TIMES|PANEL_RGB,  //fmt
 
 0x880, 0x7ca0,
 0x30,0x08,0x08,

	NULL			
 
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_ST7712_8BIT	//天宇LCD ST7712
const TLcdifParm gDriverIc_LcdIF =
{
 0x00,			//special LCD flag
 0xffcc,		//rsflag
 6,			//head num
{0,0x21, 0,0, 0,0x22},
 7, 5,0,0,   
 0, 0, 0, 0,	//stxp, styp, endxp, endyp,Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos
 COMMAND_ONLY,
 0x0, 0x0, 0x0, 0x0,
  
 MAINPANEL, 		 //main panel or sub panel
 PANEL_FMT565_2TIMES|PANEL_RGB,  //fmt
 
 0x881, 0x7ca0,
 0x7f, 0x1f, 0x1f,

	NULL
 

};
/////
#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_S1D19501_16BIT	//塞路风京东方LCD
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
       0xffb6,            //rsflag
     7,                 //head num
     {0x1500,0x0000,0x0000,0x7500,0x0000,0x0000,0x5c00},
    4, 6, 10, 12,  //stxp, styp, endxp, endyp
    0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    COMMAND_ONLY,
    0x0, 0x0, 0x0, 0x0,
	
    MAINPANEL,                 //config
    PANEL_FMT565_1TIMES|PANEL_RGB,              //fmt : 16bit bus/RGB565
    0x881, 0x7ca0,
    0x5f,0x1f,0x1f,

	NULL
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_HX8346A_16BIT	//普阳
void LcdRotate_HX8346A_16BIT(UINT8 degree, UINT8 WorkMode)
{
	if ((VIM_HAPI_MODE_BYPASS == WorkMode) || (VIM_HAPI_MODE_DIRECTDISPLAY ==WorkMode))
	{
		switch(degree)
		{
			case VIM_HAPI_ROTATE_0:
				VIM_DISP_BySetRSlow((UINT16)0x16);
				VIM_DISP_BySetRShigh((UINT16)0x00);
				break;
				
			case VIM_HAPI_ROTATE_90:
				VIM_DISP_BySetRSlow((UINT16)0x16);
				VIM_DISP_BySetRShigh((UINT16)0xc0;				
				break;
				
			case VIM_HAPI_ROTATE_180:
				VIM_DISP_BySetRSlow((UINT16)0x16);
				VIM_DISP_BySetRShigh((UINT16)0x60);				
				break;
				
			case VIM_HAPI_ROTATE_270:
				VIM_DISP_BySetRSlow((UINT16)0x16);
				VIM_DISP_BySetRShigh((UINT16)0xa0);
				break;
				
			case VIM_HAPI_MIRROR_0:
				VIM_DISP_BySetRSlow((UINT16)0x16);
				VIM_DISP_BySetRShigh((UINT16)0x40);
				
			case VIM_HAPI_MIRROR_90:
				VIM_DISP_BySetRSlow((UINT16)0x16);
				VIM_DISP_BySetRShigh((UINT16)0xe0);				
				break;
				
			case VIM_HAPI_MIRROR_180:	
				VIM_DISP_BySetRSlow((UINT16)0x16);
				VIM_DISP_BySetRShigh((UINT16)0x20);				
				break;
				
			case VIM_HAPI_MIRROR_270:
				VIM_DISP_BySetRSlow((UINT16)0x16);
				VIM_DISP_BySetRShigh((UINT16)0x80);				
				break;
				
			
		}

	
	}
	else
	{
		switch(degree)
		{
			case VIM_HAPI_ROTATE_0:
				VIM_LCDIF_RsLExW((UINT16)0x16);
				VIM_LCDIF_RsHExW((UINT16)0x00);

				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 0x0703);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 0x0f0b);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 0);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 0x1713);	
				break;
				
			case VIM_HAPI_ROTATE_90:
				VIM_LCDIF_RsLExW((UINT16)0x16);
				VIM_LCDIF_RsHExW((UINT16)0xc0);

				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 0x0f0b);				//exchange  parm->styp and  parm->stxp
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 0x0703);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 0x1713);				//exchange
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 0);
		
				break;
				
			case VIM_HAPI_ROTATE_180:
				VIM_LCDIF_RsLExW((UINT16)0x16);
				VIM_LCDIF_RsHExW((UINT16)0x20);
				
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 0x0703);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 0x0f0b);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 0);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 0x1713);	
				
				break;
				
			case VIM_HAPI_ROTATE_270:
				VIM_LCDIF_RsLExW((UINT16)0x16);
				VIM_LCDIF_RsHExW((UINT16)0xa0);

				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 0x0f0b);				//exchange  parm->styp and  parm->stxp
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 0x0703);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 0x1713);				//exchange
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 0);
				
				break;
				
			case VIM_HAPI_MIRROR_0:
				VIM_LCDIF_RsLExW((UINT16)0x16);
				VIM_LCDIF_RsHExW((UINT16)0x40);
		
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 0x0703);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 0x0f0b);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 0);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 0x1713);	
				break;
				
			case VIM_HAPI_MIRROR_90:
				VIM_LCDIF_RsLExW((UINT16)0x16);
				VIM_LCDIF_RsHExW((UINT16)0xe0);

				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 0x0f0b);				//exchange  parm->styp and  parm->stxp
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 0x0703);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 0x1713);				//exchange
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 0);
				
				break;
				
			case VIM_HAPI_MIRROR_180:
				VIM_LCDIF_RsLExW((UINT16)0x16);
				VIM_LCDIF_RsHExW((UINT16)0x60);
		
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 0x0703);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 0x0f0b);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 0);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 0x1713);		
				break;
				
			case VIM_HAPI_MIRROR_270:
				VIM_LCDIF_RsLExW((UINT16)0x16);
				VIM_LCDIF_RsHExW((UINT16)0x80);

				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 0x0f0b);				//exchange  parm->styp and  parm->stxp
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 0x0703);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 0x1713);				//exchange
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 0);
				break;
				
			default:
				break;
		}

	
	}
	
return;	
}

const TLcdifParm gDriverIc_LcdIF =
{
     0x00,
     0xeaaa,
     13,
     {0x0003,0x0000,0x0002,0x0000,		//start x
       0x0007,0x0000,0x0006,0x0000,		//start y
       //0x0005,0x00ef,0x0004,0x0000,		//end x
       0x0009,0x003f,0x0008,0x0001,		//end y
       0x0022
     },
   
     0x0703, 0x0f0b,0x0, 0x1713,  //stxp, styp, endxp, endyp
     0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
     COMMAND_ONLY,
     0x0, 0x0, 0x0, 0x0,
	
     MAINPANEL,                 //config
     PANEL_FMT565_1TIMES|PANEL_RGB,              //fmt : 16bit bus/RGB565
  	
     0x421, 0xce0,
     45,3,14,

	LcdRotate_HX8346A_16BIT
	
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_R61505U_16BIT	//Ginwave s818
const TLcdifParm gDriverIc_LcdIF =
{
   	0x00,
	0xffea,		//rsflag
	5,			//head num
	{0x20, 0, 0x21,0,0x22},
	3, 0x0807, 0, 0,	//stxp, styp, endxp, endyp;
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt
	
	  0x461, 0x460,
	  0x3C,0x0d,0x0b,

	NULL
	 // 0x460, 0x460,
	 // 0x47,0x1d,0x1b
	 //0x881, 0x460,
	 //0x5f,0x1f,0x1f
	
	
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_LGDP4531_16BIT	//S6223 project
const TLcdifParm gDriverIc_LcdIF =
{
   	0x00,
	0xffea,		//rsflag
	5,			//head num
	{0x20, 0, 0x21,0,0x22},
	3, 0x0807, 0, 0,	//stxp, styp, endxp, endyp;
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt
	
	  0x461, 0x460,
	  0x3C,0x0d,0x0b,

	NULL
	
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_ILI9221_16BIT	//s6225 project
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
    	0xfffa,		//rsflag
	3,			//head num
	{0x21, 0, 0x22},
	3, 4, 0, 0,	//stxp, styp, endxp, endyp;
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt
	0xce0, 0x460,  
	35,11,14,

	NULL
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_S6D0139_16BIT	//s6223 project
const TLcdifParm gDriverIc_LcdIF =
{
   	0x00,
	0xffea,		//rsflag
	5,			//head num
	{0x20, 0, 0x21,0,0x22},
	3, 0x0807, 0, 0,	//stxp, styp, endxp, endyp;
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt
	
	0x881, 0x7ca0,
    	0x5f,0x1f,0x1f,

	NULL
    	
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_LGDP4512_8BIT	//东立DS600 project
const TLcdifParm gDriverIc_LcdIF =
{

 0x00,			//special LCD flag
 0xffcc,		//rsflag
 6,			//head num
{0,0x21, 0,0, 0,0x22},
 7, 5,0,0,   
 0, 0, 0, 0,	//stxp, styp, endxp, endyp,Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos
 COMMAND_ONLY,
 0x0, 0x0, 0x0, 0x0,
  
 MAINPANEL, 		 //main panel or sub panel
 PANEL_FMT565_2TIMES|PANEL_RGB,  //fmt
 
 0x881, 0x7ca0,
 0x7f, 0x1f, 0x1f,

	NULL
    	
};
#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_ST7787_16BIT	//众和伟业z240 project

void LcdRotate_ST7787_16BIT(UINT8 degree, UINT8 WorkMode)
{
	if ((VIM_HAPI_MODE_BYPASS == WorkMode) || (VIM_HAPI_MODE_DIRECTDISPLAY ==WorkMode))
	{
		switch(degree)
		{
			case VIM_HAPI_ROTATE_0:
				VIM_DISP_BySetRSlow((UINT8)0x36);
				VIM_DISP_BySetRShigh((UINT8)0x00);
				break;
				
			case VIM_HAPI_ROTATE_90:
				VIM_DISP_BySetRSlow((UINT8)0x36);
				VIM_DISP_BySetRShigh((UINT8)0xc0;				
				break;
				
			case VIM_HAPI_ROTATE_180:
				VIM_DISP_BySetRSlow((UINT8)0x36);
				VIM_DISP_BySetRShigh((UINT8)0x60);				
				break;
				
			case VIM_HAPI_ROTATE_270:
				VIM_DISP_BySetRSlow((UINT8)0x36);
				VIM_DISP_BySetRShigh((UINT8)0xa0);
				break;
				
			case VIM_HAPI_MIRROR_0:
				VIM_DISP_BySetRSlow((UINT8)0x36);
				VIM_DISP_BySetRShigh((UINT8)0x40);
				
			case VIM_HAPI_MIRROR_90:
				VIM_DISP_BySetRSlow((UINT8)0x36);
				VIM_DISP_BySetRShigh((UINT8)0xe0);				
				break;
				
			case VIM_HAPI_MIRROR_180:	
				VIM_DISP_BySetRSlow((UINT8)0x36);
				VIM_DISP_BySetRShigh((UINT8)0x20);				
				break;
				
			case VIM_HAPI_MIRROR_270:
				VIM_DISP_BySetRSlow((UINT8)0x36);
				VIM_DISP_BySetRShigh((UINT8)0x80);				
				break;
				
			
		}

	
	}
	else
	{
		switch(degree)
		{
			case VIM_HAPI_ROTATE_0:
				VIM_LCDIF_RsLExW((UINT8)0x36);
				VIM_LCDIF_RsHExW((UINT8)0x00);

				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 0x0305);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 0x0d0f);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 0x0709);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 0x1113);		
				break;
				
			case VIM_HAPI_ROTATE_90:
				VIM_LCDIF_RsLExW((UINT8)0x36);
				VIM_LCDIF_RsHExW((UINT8)0xc0);

				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 0x0d0f);				//exchange  parm->styp and  parm->stxp
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 0x0305);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 0x1113);				//exchange
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 0x0709);
		
				break;
				
			case VIM_HAPI_ROTATE_180:
				VIM_LCDIF_RsLExW((UINT8)0x36);
				VIM_LCDIF_RsHExW((UINT8)0x20);
				
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 0x0305);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 0x0d0f);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 0x0709);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 0x1113);		
				
				break;
				
			case VIM_HAPI_ROTATE_270:
				VIM_LCDIF_RsLExW((UINT8)0x36);
				VIM_LCDIF_RsHExW((UINT8)0xa0);

				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 0x0d0f);				//exchange  parm->styp and  parm->stxp
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 0x0305);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 0x1113);				//exchange
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 0x0709);
				
				break;
				
			case VIM_HAPI_MIRROR_0:
				VIM_LCDIF_RsLExW((UINT8)0x36);
				VIM_LCDIF_RsHExW((UINT8)0x40);
		
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 0x0305);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 0x0d0f);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 0x0709);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 0x1113);	
				break;
				
			case VIM_HAPI_MIRROR_90:
				VIM_LCDIF_RsLExW((UINT8)0x36);
				VIM_LCDIF_RsHExW((UINT8)0xe0);

				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 0x0d0f);				//exchange  parm->styp and  parm->stxp
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 0x0305);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 0x1113);				//exchange
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 0x0709);
				
				break;
				
			case VIM_HAPI_MIRROR_180:
				VIM_LCDIF_RsLExW((UINT8)0x36);
				VIM_LCDIF_RsHExW((UINT8)0x60);
		
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 0x0305);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 0x0d0f);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 0x0709);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 0x1113);		
				break;
				
			case VIM_HAPI_MIRROR_270:
				VIM_LCDIF_RsLExW((UINT8)0x36);
				VIM_LCDIF_RsHExW((UINT8)0x80);

				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 0x0d0f);				//exchange  parm->styp and  parm->stxp
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 0x0305);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 0x1113);				//exchange
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 0x0709);
				break;
				
			default:
				break;
		}

	
	}
	
return;	
}

const TLcdifParm gDriverIc_LcdIF =
{
    	0x00,
	0xfbde,		//rsflag
	11,			//head num
	{0x002a, 0x0000, 0x0000, 0x0000, 0x00ef, 0x002b,0x0000, 0x0000, 0x0001, 0x003f,0x002c},
	0x0305, 0x0d0f, 0x0709, 0x1113,	//stxp, styp, endxp, endyp;
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt
	
	0x881, 0x7ca0,
    	0x5f,0x1f,0x1f,

	LcdRotate_ST7787_16BIT	
};



#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_ILI9160_8BIT	//众和伟业z150  project
const TLcdifParm gDriverIc_LcdIF =
{
   	 0x00,			//special LCD flag
	 0xffcc,		//rsflag
 	6,			//head num
	{0,0x21, 0,0, 0,0x22},
 	7, 5,0,0,   
 	0, 0, 0, 0,	//stxp, styp, endxp, endyp,Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos
 	COMMAND_ONLY,
 	0x0, 0x0, 0x0, 0x0,
  
 	MAINPANEL, 		 //main panel or sub panel
 	PANEL_FMT565_2TIMES|PANEL_RGB,  //fmt
 
 	0x460, 0x880,
     	0x2a,0x06,0x06,

	NULL
    	
};

/////

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_ILI9320_16BIT	//达成无限529 project
const TLcdifParm gDriverIc_LcdIF =
{
   
     	0x00,
	0xffea,		//rsflag
	5,			//head num
	{0x20, 0, 0x21,0,0x22},
	3, 0x0807, 0, 0,	//stxp, styp, endxp, endyp;
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt
	
	0x461, 0x461,
 	0x3c, 0x0d, 0x0b,

	NULL
    	
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_C1L5_06_16BIT	////互芯to ZhengLei Sopio	
const TLcdifParm gDriverIc_LcdIF =
{
   #if 1
     	0x00,
	0xffea,		//rsflag
	6,			//head num
	{0x18, 0, 0x19,0, 0x17, 0},
	3, 7, 0, 0,	//stxp, styp, endxp, endyp;
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt
	
	0x461, 0x880,
     	0x3c, 0x0d, 0x0b,

	NULL
#elif
   	0x00,
	0xfeaa,		//rsflag
	9,			//head num
	{0x09, 0, 0x10,0, 0x11, 0x7f, 0x12, 0x9f,0x17},
	3, 7, 0xb, 0xf,	//stxp, styp, endxp, endyp;
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt
	
	0x461, 0x880,
     	0x3c, 0x0d, 0x0b
 #endif   	
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_ST7637_8BIT	////互芯to ZhengLei Sopio	
const TLcdifParm gDriverIc_LcdIF =
{
   
     	0x00,
	0xffb6,		//rsflag
	7,			//head num
	//{0x2a, 04, 0x83, 0x2b,04,0x83,0x2c},
	{0x2a, 00, 0x7f, 0x2b,00,0x7f,0x2c},
	3, 9, 5, 0xb,	//stxp, styp, endxp, endyp;
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_2TIMES|PANEL_RGB,		//fmt

     	//0x420, 0x420,		//also preview OK
     	//19, 3, 14
	0x881, 0x460,
       0x5f,0x1f,0x1f,

	NULL
      	
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_L1E2_16BIT	// 中兴
const TLcdifParm gDriverIc_LcdIF =
{
     	0x00,
	0xfbde,		//rsflag
	11,			//head num
	{0x2a, 00, 00, 00, 0xaf, 0x2b,00,00,00, 0xdb, 0x2c},
	0x0305, 0x0d0f, 0x0709, 0x1113,	//stxp, styp, endxp, endyp;
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt

	0x461,0x461,
	0x3c, 0x0d, 0x0b,

	NULL
////////1.5 version add:
#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_R61509_16BIT	//8/24/2007 to Brave 9/14/2007 test OK
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
	0xffea,		//rsflag
	5,			//head num
	{0x200, 0, 0x201, 0, 0x202},
	3, 0x0807, 0, 0,	//stxp, styp, endxp, endyp;
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,	
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt
	
	 0x421, 0xce0,
    	 45,3,14,

	NULL
	
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_HX8347_A01_16BIT //to WangJian//10/10/2007

void LcdRotate_HX8347_A01_16BIT(UINT8 degree, UINT8 WorkMode)
{
	if ((VIM_HAPI_MODE_BYPASS == WorkMode) || (VIM_HAPI_MODE_DIRECTDISPLAY ==WorkMode))
	{
		switch(degree)
		{
			case VIM_HAPI_ROTATE_0:
				VIM_DISP_BySetRSlow((UINT16)0x16);
				VIM_DISP_BySetRShigh((UINT16)0x00);
				break;
				
			case VIM_HAPI_ROTATE_90:
				VIM_DISP_BySetRSlow((UINT16)0x16);
				VIM_DISP_BySetRShigh((UINT16)0xc0;				
				break;
				
			case VIM_HAPI_ROTATE_180:
				VIM_DISP_BySetRSlow((UINT16)0x16);
				VIM_DISP_BySetRShigh((UINT16)0x20);				
				break;
				
			case VIM_HAPI_ROTATE_270:
				VIM_DISP_BySetRSlow((UINT16)0x16);
				VIM_DISP_BySetRShigh((UINT16)0xa0);
				break;
				
			case VIM_HAPI_MIRROR_0:
				VIM_DISP_BySetRSlow((UINT16)0x16);
				VIM_DISP_BySetRShigh((UINT16)0x40);
				
			case VIM_HAPI_MIRROR_90:
				VIM_DISP_BySetRSlow((UINT16)0x16);
				VIM_DISP_BySetRShigh((UINT16)0xe0);				
				break;
				
			case VIM_HAPI_MIRROR_180:	
				VIM_DISP_BySetRSlow((UINT16)0x16);
				VIM_DISP_BySetRShigh((UINT16)0x60);				
				break;
				
			case VIM_HAPI_MIRROR_270:
				VIM_DISP_BySetRSlow((UINT16)0x16);
				VIM_DISP_BySetRShigh((UINT16)0x80);				
				break;
				
			
		}

	
	}
	else
	{
		switch(degree)
		{
			case VIM_HAPI_ROTATE_0:
				VIM_LCDIF_RsLExW((UINT16)0x16);
				VIM_LCDIF_RsHExW((UINT16)0x00);

				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 0x0703);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 0x0f0b);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 0);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 0x1713);	
				break;
				
			case VIM_HAPI_ROTATE_90:
				VIM_LCDIF_RsLExW((UINT16)0x16);
				VIM_LCDIF_RsHExW((UINT16)0xc0);

				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 0x0f0b);				//exchange  parm->styp and  parm->stxp
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 0x0703);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 0x1713);				//exchange
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 0);
		
				break;
				
			case VIM_HAPI_ROTATE_180:
				VIM_LCDIF_RsLExW((UINT16)0x16);
				VIM_LCDIF_RsHExW((UINT16)0x20);
				
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 0x0703);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 0x0f0b);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 0);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 0x1713);	
				
				break;
				
			case VIM_HAPI_ROTATE_270:
				VIM_LCDIF_RsLExW((UINT16)0x16);
				VIM_LCDIF_RsHExW((UINT16)0xa0);

				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 0x0f0b);				//exchange  parm->styp and  parm->stxp
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 0x0703);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 0x1713);				//exchange
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 0);
				
				break;
				
			case VIM_HAPI_MIRROR_0:
				VIM_LCDIF_RsLExW((UINT16)0x16);
				VIM_LCDIF_RsHExW((UINT16)0x40);
		
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 0x0703);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 0x0f0b);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 0);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 0x1713);	
				break;
				
			case VIM_HAPI_MIRROR_90:
				VIM_LCDIF_RsLExW((UINT16)0x16);
				VIM_LCDIF_RsHExW((UINT16)0xe0);

				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 0x0f0b);				//exchange  parm->styp and  parm->stxp
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 0x0703);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 0x1713);				//exchange
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 0);
				
				break;
				
			case VIM_HAPI_MIRROR_180:
				VIM_LCDIF_RsLExW((UINT16)0x16);
				VIM_LCDIF_RsHExW((UINT16)0x60);
		
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 0x0703);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 0x0f0b);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 0);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 0x1713);		
				break;
				
			case VIM_HAPI_MIRROR_270:
				VIM_LCDIF_RsLExW((UINT16)0x16);
				VIM_LCDIF_RsHExW((UINT16)0x80);

				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 0x0f0b);				//exchange  parm->styp and  parm->stxp
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 0x0703);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 0x1713);				//exchange
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 0);
				break;
				
			default:
				break;
		}

	
	}
	
return;	
}
const TLcdifParm gDriverIc_LcdIF =
{
     0x00,
     0xeaaa,
     13,
     {0x0003,0x0000,0x0002,0x0000,  //start x
      0x0007,0x0000,0x0006,0x0000,  //start y
      0x0009,0x003f,0x0008,0x0001,  //end y
      0x0022
     },
   
     0x0703, 0x0f0b,0x0, 0x1713,  //stxp, styp, endxp, endyp
     0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
     COMMAND_ONLY,
     0x0, 0x0, 0x0, 0x0,
 
     MAINPANEL,                 //config
     PANEL_FMT565_1TIMES|PANEL_RGB,              //fmt : 16bit bus/RGB565
   
     0x421, 0xce0,
     45,3,14,

	LcdRotate_HX8347_A01_16BIT
 
};




 #elif  V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_SPFD5408A_16BIT	//to WangJian //10/10/2007
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
	0xffea,		//rsflag
	5,			//head num
	{0x20, 0, 0x21,0,0x22},
	3, 0x0807, 0, 0,	//stxp, styp, endxp, endyp;
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt
	
	 0x461, 0x460,
	0x3C,0x0d,0x0b,

	NULL
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_HX8345A_8BIT		//上海展帆//to WangJian 10/11/2007
const TLcdifParm gDriverIc_LcdIF =
{
 0x00,			//special LCD flag
 0xffcc,		//rsflag
 6,			//head num
{0,0x21, 0,0, 0,0x22},
 7, 5,0,0,   

 0, 0, 0, 0,	//stxp, styp, endxp, endyp,Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos
 COMMAND_ONLY,
 0x0, 0x0, 0x0, 0x0,
  
 MAINPANEL, 		 //main panel or sub panel
 PANEL_FMT565_2TIMES|PANEL_RGB,  //fmt
 
 //0x881, 0x7ca0,
 //0x7f, 0x1f, 0x1f
 0xce1, 0xce1
 35,11,14,

	NULL

};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_S6B33BF_16BIT	//上海展帆//to WangJian 10/11/2007
const TLcdifParm gDriverIc_LcdIF =
{
   	 0x00,			//special LCD flag
	 0xffc0,		//rsflag
 	6,			//head num
	{0x42, 0,0xA1, 0x43, 0, 0x83},
 	3, 9, 5,11,   
 	0, 0, 0, 0,	//stxp, styp, endxp, endyp,Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos
 	COMMAND_ONLY,
 	0x0, 0x0, 0x0, 0x0,
  
 	MAINPANEL, 		 //main panel or sub panel
 	PANEL_FMT565_1TIMES|PANEL_RGB,  //fmt
 
 	0x461,0x461,
	0x3c, 0x0d, 0x0b,

	NULL
    	
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_HX8345_16BIT	//深圳普阳 //to Brave, 12/3/2007 //客户反馈OK
const TLcdifParm gDriverIc_LcdIF =
{
   
	 0x00,			//special LCD flag
	 0xfffa,		//rsflag
 	3,			//head num
	{0x21, 0, 0x22},
 	3, 4, 0, 0,	//stxp, styp, endxp, endyp;	//the customer have tested, the parameter is ok!
 	0, 0, 0, 0,	//stxp, styp, endxp, endyp,Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos
 	COMMAND_ONLY,
 	0x0, 0x0, 0x0, 0x0,
  
 	MAINPANEL, 		 //main panel or sub panel
 	PANEL_FMT565_1TIMES|PANEL_RGB,  //fmt
 
 	0x881, 0x460,
	0x5f, 0x1f, 0x1f,

	NULL				
    	
};


#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_NT75751_8BIT	//深圳优创 //to Brave, 12/4/2007 
const TLcdifParm gDriverIc_LcdIF =
{
   
	0x00,		//special LCD flag
	0xffc0,		//rsflag
 	6,			//head num
	{0x43, 0,0x0, 0x42, 0, 0x0},
 	3, 9, 5,11,   
 	0, 0, 0, 0,	//stxp, styp, endxp, endyp,Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos
 	COMMAND_ONLY,
 	0x0, 0x0, 0x0, 0x0,
  
 	MAINPANEL, 		 //main panel or sub panel
 	PANEL_FMT565_2TIMES|PANEL_RGB,  //fmt
 
 	0x461,0x461,
	0x3c, 0x0d, 0x0b,

	NULL		
    	
};
#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_NT39102_8BIT	//上海华勤to Terry //11/28/2007//客户反馈OK
const TLcdifParm gDriverIc_LcdIF =
{
   	 0x00,			//special LCD flag
	 0xfbde,		//rsflag
 	11,			//head num
	{0x2A, 0,0,0,0x7f, 0x2B,0,0,0,0x9f, 0x2C},
 	0x0305, 0x0d0f, 0x0709,0x1113,   	//stxp, styp, endxp, endyp;
 	0, 0, 0, 0,	//Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos
 	COMMAND_ONLY,
 	0x0, 0x0, 0x0, 0x0,
  
 	MAINPANEL, 		 //main panel or sub panel
 	PANEL_FMT565_2TIMES|PANEL_RGB,  //fmt
 
 	0x461,0x461,
	0x3c, 0x0d, 0x0b,

	NULL
	
 	
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_NT39102_16BIT	//to zhenghui//11/13/2007
const TLcdifParm gDriverIc_LcdIF =
{
   	 0x00,			//special LCD flag
	 0xfbde,		//rsflag
 	11,			//head num
	{0x2A, 0,0,0,0x7f, 0x2B,0,0,0,0x9f, 0x2C},
 	0x0305, 0x0d0f, 0x0709,0x1113,   	//stxp, styp, endxp, endyp;
 	0, 0, 0, 0,	//Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos
 	COMMAND_ONLY,
 	0x0, 0x0, 0x0, 0x0,
  
 	MAINPANEL, 		 //main panel or sub panel
 	PANEL_FMT565_1TIMES|PANEL_RGB,  //fmt
 
 	0x461,0x461,
	0x3c, 0x0d, 0x0b,

	NULL
 	
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_S6B33B6X_8BIT	//to ZhongXi
const TLcdifParm gDriverIc_LcdIF =
{
   	 0x00,			//special LCD flag
	 0xffc0,		//rsflag
 	6,			//head num
	{0x42, 0,0x83, 0x43, 0, 0x83},
 	3, 9, 5,11,   
 	0, 0, 0, 0,	//stxp, styp, endxp, endyp,Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos
 	COMMAND_ONLY,
 	0x0, 0x0, 0x0, 0x0,
  
 	MAINPANEL, 		 //main panel or sub panel
 	PANEL_FMT565_2TIMES|PANEL_RGB,  //fmt
 
 	0x461,0x461,
	0x3c, 0x0d, 0x0b,

	NULL
    	
};


 #elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_S6B33BF_8BIT	//波导BoDao//to ShangFeng 11/6/2007
const TLcdifParm gDriverIc_LcdIF =
{
    0x00,
    0xffc0,            //rsflag
    6,                 //head num
    {0x43, 0x00, 0x00, 0x42, 0x00, 0x00},
    3, 9, 5, 11,  //stxp, styp, endxp, endyp
    0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    COMMAND_ONLY,
    0x0, 0x0, 0x0, 0x0,
	
     MAINPANEL,                 //config
     PANEL_FMT565_2TIMES|PANEL_RGB,		//fmt,

     0x461,0x461,
     0x3c, 0x0d, 0x0b,

	NULL
	
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_S6B33BL_8BIT	//波导//to ShangFeng 11/26/2007
const TLcdifParm gDriverIc_LcdIF =
{
   	 0x00,			//special LCD flag
	 0xffc0,		//rsflag
 	6,			//head num
	{0x43, 0x00, 0x00, 0x42, 0x00, 0x00},
 	3, 9, 5,11,   
 	0, 0, 0, 0,	//stxp, styp, endxp, endyp,Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos
 	COMMAND_ONLY,
 	0x0, 0x0, 0x0, 0x0,
  
 	MAINPANEL, 		 //main panel or sub panel
 	PANEL_FMT565_2TIMES|PANEL_RGB,  //fmt
 
 	0x461,0x461,
	0x3c, 0x0d, 0x0b,

	NULL
    	
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_ST7669_8BIT    // to 宁波义隆达 //11/28/2007//客户反馈参数OK

const TLcdifParm gDriverIc_LcdIF =

{

    0x00,
    0xfbde,     //rsflag
    11,         //head num
    {0x2a, 00, 00, 00, 0x83, 0x2b,00,00,00, 0xA1, 0x2c},
    0x0305, 0x0d0f, 0x0709, 0x1113, //stxp, styp, endxp, endyp;
    0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    COMMAND_ONLY,
    0x0, 0x0, 0x0, 0x0,

    MAINPANEL,      //main panel or sub panel
    PANEL_FMT565_2TIMES|PANEL_RGB,      //fmt

    0x461,0x461,
    0x3c, 0x0d, 0x0b,

	NULL

};

///////
#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_NT39102_8BIT	//上海华勤to Terry //11/28/2007//客户反馈OK
const TLcdifParm gDriverIc_LcdIF =
{
   	 0x00,			//special LCD flag
	 0xfbde,		//rsflag
 	11,			//head num
	{0x2A, 0,0,0,0x7f, 0x2B,0,0,0,0x9f, 0x2C},
 	0x0305, 0x0d0f, 0x0709,0x1113,   	//stxp, styp, endxp, endyp;
 	0, 0, 0, 0,	//Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos
 	COMMAND_ONLY,
 	0x0, 0x0, 0x0, 0x0,
  
 	MAINPANEL, 		 //main panel or sub panel
 	PANEL_FMT565_2TIMES|PANEL_RGB,  //fmt
 
 	0x461,0x461,
	0x3c, 0x0d, 0x0b,

	NULL
	
 	
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_HX8345_16BIT	//深圳普阳 //to Brave, 12/3/2007 //客户反馈OK
const TLcdifParm gDriverIc_LcdIF =
{
   
	 0x00,			//special LCD flag
	 0xfffa,		//rsflag
 	3,			//head num
	{0x21, 0, 0x22},
 	3, 4, 0, 0,	//stxp, styp, endxp, endyp;	//the customer have tested, the parameter is ok!
 	0, 0, 0, 0,	//stxp, styp, endxp, endyp,Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos
 	COMMAND_ONLY,
 	0x0, 0x0, 0x0, 0x0,
  
 	MAINPANEL, 		 //main panel or sub panel
 	PANEL_FMT565_1TIMES|PANEL_RGB,  //fmt
 
 	0x881, 0x460,
	0x5f, 0x1f, 0x1f,

	NULL				
    	
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_NT75751_8BIT	//深圳优创 //to Brave, 12/4/2007 
const TLcdifParm gDriverIc_LcdIF =
{
   
	0x00,		//special LCD flag
	0xffc0,		//rsflag
 	6,			//head num
	{0x43, 0,0x0, 0x42, 0, 0x0},
 	3, 9, 5,11,   
 	0, 0, 0, 0,	//stxp, styp, endxp, endyp,Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos
 	COMMAND_ONLY,
 	0x0, 0x0, 0x0, 0x0,
  
 	MAINPANEL, 		 //main panel or sub panel
 	PANEL_FMT565_2TIMES|PANEL_RGB,  //fmt
 
 	0x461,0x461,
	0x3c, 0x0d, 0x0b,

	NULL		
    	
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_ILI9325_16BIT	//to WangJian //10/10/2007//反馈OK
void LcdRotate_ILI9325_16BIT(UINT8 degree, UINT8 WorkMode)
{
	if ((VIM_HAPI_MODE_BYPASS == WorkMode) || (VIM_HAPI_MODE_DIRECTDISPLAY ==WorkMode))
	{
		switch(degree)
		{
			case VIM_HAPI_ROTATE_0:
				VIM_DISP_BySetRSlow((UINT16)0x03);
				VIM_DISP_BySetRShigh((UINT16)0x1030);
				break;
				
			case VIM_HAPI_ROTATE_90:
				VIM_DISP_BySetRSlow((UINT16)0x03);
				VIM_DISP_BySetRShigh((UINT16)0x1028);				
				break;
				
			case VIM_HAPI_ROTATE_180:
				VIM_DISP_BySetRSlow((UINT16)0x03);
				VIM_DISP_BySetRShigh((UINT16)0x1000);				
				break;
				
			case VIM_HAPI_ROTATE_270:
				VIM_DISP_BySetRSlow((UINT16)0x03);
				VIM_DISP_BySetRShigh((UINT16)0x1018);
				break;
				
			case VIM_HAPI_MIRROR_0:
				VIM_DISP_BySetRSlow((UINT16)0x03);
				VIM_DISP_BySetRShigh((UINT16)0x1020);
				
			case VIM_HAPI_MIRROR_90:
				VIM_DISP_BySetRSlow((UINT16)0x03);
				VIM_DISP_BySetRShigh((UINT16)0x1038);				
				break;
				
			case VIM_HAPI_MIRROR_180:	
				VIM_DISP_BySetRSlow((UINT16)0x03);
				VIM_DISP_BySetRShigh((UINT16)0x1010);				
				break;
				
			case VIM_HAPI_MIRROR_270:
				VIM_DISP_BySetRSlow((UINT16)0x03);
				VIM_DISP_BySetRShigh((UINT16)0x1008);				
				break;
	
		}

	}
	else
	{
		switch(degree)
		{
			case VIM_HAPI_ROTATE_0:
				VIM_LCDIF_RsLExW((UINT16)0x03);
				VIM_LCDIF_RsHExW((UINT16)0x1030);

				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 3);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 0x0807);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 0);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 0);	
				break;
				
			case VIM_HAPI_ROTATE_90:
				VIM_LCDIF_RsLExW((UINT16)0x03);
				VIM_LCDIF_RsHExW((UINT16)0x1028);

				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 0x0807);				//exchange  parm->styp and  parm->stxp
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 3);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 0);				//exchange
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 0);
		
				break;
				
			case VIM_HAPI_ROTATE_180:
				VIM_LCDIF_RsLExW((UINT16)0x03);
				VIM_LCDIF_RsHExW((UINT16)0x1000);
				
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 3);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 0x0807);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 0);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 0);	
				
				break;
				
			case VIM_HAPI_ROTATE_270:
				VIM_LCDIF_RsLExW((UINT16)0x03);
				VIM_LCDIF_RsHExW((UINT16)0x1018);

				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 0x0807);				//exchange  parm->styp and  parm->stxp
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 3);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 0);				//exchange
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 0);
				
				break;
				
			case VIM_HAPI_MIRROR_0:
				VIM_LCDIF_RsLExW((UINT16)0x03);
				VIM_LCDIF_RsHExW((UINT16)0x1020);
		
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 3);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 0x0807);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 0);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 0);	
				break;
				
			case VIM_HAPI_MIRROR_90:
				VIM_LCDIF_RsLExW((UINT16)0x03);
				VIM_LCDIF_RsHExW((UINT16)0x1038);

				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 0x0807);				//exchange  parm->styp and  parm->stxp
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 3);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 0);				//exchange
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 0);
				
				break;
				
			case VIM_HAPI_MIRROR_180:
				VIM_LCDIF_RsLExW((UINT16)0x03);
				VIM_LCDIF_RsHExW((UINT16)0x1010);
		
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 3);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 0x0807);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 0);				
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 0);		
				break;
				
			case VIM_HAPI_MIRROR_270:
				VIM_LCDIF_RsLExW((UINT16)0x03);
				VIM_LCDIF_RsHExW((UINT16)0x1008);

				VIM_HIF_SetReg16(V5_REG_LCDIF_START_X_POS_L, 0x0807);				//exchange  parm->styp and  parm->stxp
				VIM_HIF_SetReg16(V5_REG_LCDIF_START_Y_POS_L, 3);
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_X_POS_L, 0);				//exchange
				VIM_HIF_SetReg16(V5_REG_LCDIF_END_Y_POS_L, 0);
				break;
				
			default:
				break;
		}

	}
	
return;	
}

const TLcdifParm gDriverIc_LcdIF =
{
   
     	0x00,
	0xffea,		//rsflag
	5,			//head num
	{0x20, 0, 0x21,0,0x22},
	3, 0x0807, 0, 0,	//stxp, styp, endxp, endyp;
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt
	
	0x461, 0x461,
 	0x3c, 0x0d, 0x0b,

	LcdRotate_ILI9325_16BIT

    	
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_TLS8301_8BIT		//to WangJian //11/23/2007
const TLcdifParm gDriverIc_LcdIF =
{
 0x00,
 0xfccc,  //rsflag
 10,   //head num
 {0x00,0x20, 0, 0, 0x00,0x21, 0, 0, 0x00,0x22},
 7, 0x0d0f, 0, 0,  //stxp, styp, endxp, endyp
  0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
  COMMAND_ONLY,
  0x0, 0x0, 0x0, 0x0,
 
 MAINPANEL,  //main panel or sub panel
 PANEL_FMT565_2TIMES|PANEL_RGB,  //fmt
 0x460, 0x460,
 0x47,0x0d,0x0b,

	NULL

};

#elif  V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_R61503U_16BIT			//to wangjian//客户反馈ok

const TLcdifParm gDriverIc_LcdIF =

{
    0x00,
    0xffea,     //rsflag
    5,          //head num
    {0x20, 0, 0x21,0,0x22},
    3, 7, 0, 0, //stxp, styp, endxp, endyp;
    0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    COMMAND_ONLY,
    0x0, 0x0, 0x0, 0x0,

    MAINPANEL,      //main panel or sub panel
    PANEL_FMT565_1TIMES|PANEL_RGB,      //fmt

    0x461, 0x460,
    0x3C,0x0d,0x0b,

	NULL

};


#elif  V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_R61503U_8BIT	//上海华勤//to Terry Zhang//12/10/2007
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
	0xfccc,		//rsflag
	10,			//head num
	{0,0x20, 0,0, 0,0x21,0,0,0,0x22},
	7, 15, 0, 0,	//stxp, styp, endxp, endyp;
       0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
       0x0, 0x0, 0x0, 0x0,

	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_2TIMES|PANEL_RGB,		//fmt

	0x420, 0x460,
	0x3C,0x3,0x0b,

	NULL
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_ST7637_16BIT	//波导//to wangjian//12/21/2007	
const TLcdifParm gDriverIc_LcdIF =
{
   
     	0x00,
	0xffb6,		//rsflag
	7,			//head num
	
	{0x2a, 00, 0x7f, 0x2b,00,0x7f,0x2c},
	3, 9, 5, 0xb,	//stxp, styp, endxp, endyp;
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt

       0x461,0x461,
	0x3c, 0x0d, 0x0b,

	NULL	
      	
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_SPFD54124A_16BIT	//苏州摩本//to WangJian//12/19/2007
const TLcdifParm gDriverIc_LcdIF =
{
 0x00,
 0xfbde,  //rsflag
 11,   //head num
 {0x002a, 0x0000, 0x0000,0x0000, 0x0000, 0x002b, 0x0000, 0x0000, 0x0000, 0x0000,0x002c},
  0x0305, 0x0d0f, 0x0709, 0x1113,  //stxp, styp, endxp, endyp
  0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
  COMMAND_ONLY,
  0x0, 0x0, 0x0, 0x0,
  
 MAINPANEL,  //main panel or sub panel
 PANEL_FMT565_1TIMES|PANEL_RGB,  //fmt

0x421, 0x421,
45,3,14,

	NULL

};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_ST7732_16BIT	//上海展帆9/5/2007
const TLcdifParm gDriverIc_LcdIF =//maybe have problem in stxp,styp,endxp,endyp, pls see the driver ic data sheet!
{

    	0x00,
	0xfbde,		//rsflag
	11,			//head num
	{0x2a, 0x00, 0x00, 0x00, 0x81, 0x2b,0x00, 0x00, 0x00, 0xa0,0x2c},
	0x05, 0x0f, 0x09, 0x13,	//stxp, styp, endxp, endyp;
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt
	
	0x461,0x461,
	0x3c, 0x0d, 0x0b,

	NULL	
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_ILI9320_8BIT	//展望科技//to wangjian//12/13/2007//12/21/2007 modified
const TLcdifParm gDriverIc_LcdIF =
{
   
     	0x00,
	0xfccc,		//rsflag
	10,			//head num
	{0x00,0x20, 0, 0, 0x00,  0x21, 0, 0, 0x00,0x22},
	7, 0x0d0f, 0, 0,	//stxp, styp, endxp, endyp;
	
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_2TIMES|PANEL_RGB,		//fmt
	
	0x421, 0xce0,
     	78, 6, 16,

	NULL
    	
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_ILI9220_16BIT	//爱摩//to ZhongXi//12/24/2007
const TLcdifParm gDriverIc_LcdIF =
{
   
     	0x00,
	0xfffa,		//rsflag
	3,			//head num
	{0x21, 0, 0x22},
	3, 4, 0, 0,	//stxp, styp, endxp, endyp;
	
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt
	
	0x461, 0x461,
 	0x3c, 0x0d, 0x0b,

	NULL
	
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_ILI9220_8BIT	//to WangChuanFu//天朗互联//1/11/2008//爱摩//to ZhongXi//12/24/2007
const TLcdifParm gDriverIc_LcdIF =
{
   
     	0x00,
	0xffcc,		//rsflag
	6,			//head num
	{0x00,0x21, 0, 0, 0x00,0x22},
	7, 5, 0, 0,	//stxp, styp, endxp, endyp;
	
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_2TIMES|PANEL_RGB,		//fmt
	
	0x461, 0x461,
 	0x3c, 0x0d, 0x0b,
	NULL
 	     	
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_SPFD54124B_16BIT	//波导//to wangjian//1/3/2007
const TLcdifParm gDriverIc_LcdIF =
{
 0x00,
 0xfbde,  //rsflag
 11,   //head num
 {0x2a, 0x00, 0x00,0x00, 0x00, 0x2b, 0x00, 0x00, 0x00, 0x00,0x2c},
  0x0305, 0x0d0f, 0x0709, 0x1113,  //stxp, styp, endxp, endyp
  0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
  COMMAND_ONLY,
  0x0, 0x0, 0x0, 0x0,
  
 MAINPANEL,  //main panel or sub panel
 PANEL_FMT565_1TIMES|PANEL_RGB,  //fmt

0x421, 0x421,
45,3,14,

	NULL

};

#elif  V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_S6D0164_8BIT	//to ZhengHui//12/25/2007//1/7/2007//to 天朗互联
const TLcdifParm gDriverIc_LcdIF =
{
	
	0x00,
	0xfccc,		//rsflag	//command: RS=1, RAM data: RS=0
	10,			//head num
	{0x00, 0x20, 0x0, 0x0, 0x00, 0x21, 0x0, 0x0, 0x00,0x22},
	7, 15, 0, 0,	//stxp, styp, endxp, endyp;
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_2TIMES|PANEL_RGB,		//fmt
	
     	0x420,0xce0,
	0x3c, 3, 14,
	NULL
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_LGDP4532_8BIT	//上海华勤//to wangjian and shangfeng//1/7/2007
const TLcdifParm gDriverIc_LcdIF =
{

 0x00,			//special LCD flag
0xfccc,		//rsflag
10,			//head num
{0,0x20, 0,0, 0,0x21,0,0,0,0x22},
7, 0x0d0f, 0, 0,	//stxp, styp, endxp, endyp;

 0, 0, 0, 0,	//stxp, styp, endxp, endyp,Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos
 COMMAND_ONLY,
 0x0, 0x0, 0x0, 0x0,
  
 MAINPANEL, 		 //main panel or sub panel
 PANEL_FMT565_2TIMES|PANEL_RGB,  //fmt
 
 0x421, 0xce0,
 78,6,16,

	NULL
    	
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_SPFD5414_8BIT	//to Terry zhang //11/13/2007
const TLcdifParm gDriverIc_LcdIF =
{
 0x00,
 0xfbde,  //rsflag
 11,   //head num
 {0x2a, 0x00, 0x00,0x00, 0x00, 0x2b, 0x00, 0x00, 0x00, 0x00,0x2c},
  0x0305, 0x0d0f, 0x0709, 0x1113,  //stxp, styp, endxp, endyp
  0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
  COMMAND_ONLY,
  0x0, 0x0, 0x0, 0x0,
 
 MAINPANEL,  //main panel or sub panel
 PANEL_FMT565_2TIMES|PANEL_RGB,  //fmt
 0x460, 0x460,
 0x3C,0x0d,0x0b,

	NULL
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_LGDP4522_8BIT	//上海华勤//to Terry Zhang//12/10/2007
const TLcdifParm gDriverIc_LcdIF =
{

 0x00,			//special LCD flag
 0xffcc,		//rsflag
 6,			//head num
{0,0x21, 0,0, 0,0x22},
 7, 5,0,0,   
 0, 0, 0, 0,	//stxp, styp, endxp, endyp,Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos
 COMMAND_ONLY,
 0x0, 0x0, 0x0, 0x0,
  
 MAINPANEL, 		 //main panel or sub panel
 PANEL_FMT565_2TIMES|PANEL_RGB,  //fmt
 
 0x401,0x460,
 0x3c, 0x0d, 0x0b,

NULL
    	
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_MC2TA7402_8BIT	//上海华勤//to Terry Zhang//12/10/2007
const TLcdifParm gDriverIc_LcdIF =
{

 0x00,			//special LCD flag
 0xffcc,		//rsflag
 6,			//head num
{0,0x21, 0,0, 0,0x22},
 7, 5,0,0,   
 0, 0, 0, 0,	//stxp, styp, endxp, endyp,Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos
 COMMAND_ONLY,
 0x0, 0x0, 0x0, 0x0,
  
 MAINPANEL, 		 //main panel or sub panel
 PANEL_FMT565_2TIMES|PANEL_RGB,  //fmt
  
 0x461,0x461,
 0x3c, 0x0d, 0x0b,

	NULL
    	
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_NT39118_8BIT	//鼎智8bit // to Brave 9/11/2007
const TLcdifParm gDriverIc_LcdIF =
{
   	 0x00,			//special LCD flag
	 0xfbde,		//rsflag
 	11,			//head num
	{0x2A, 0,0,0,0x7f, 0x2B,0,0,0,0x9f, 0x2C},
 	0x0305, 0x0d0f, 0x0709,0x1113,   	//stxp, styp, endxp, endyp;
 	0, 0, 0, 0,	//Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos
 	COMMAND_ONLY,
 	0x0, 0x0, 0x0, 0x0,
  
 	MAINPANEL, 		 //main panel or sub panel
 	PANEL_FMT565_2TIMES|PANEL_RGB,  //fmt
 
 	0x461,0x461,
	0x3c, 0x0d, 0x0b,

	NULL
 	
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_S6D0164_16BIT	//to BraveFeng//10/31/2007
const TLcdifParm gDriverIc_LcdIF =
{
   	0x00,
	0xffea,		//rsflag
	5,			//head num
	{0x20, 0, 0x21,0,0x22},
	3, 7, 0, 0,	//stxp, styp, endxp, endyp;
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt

	0x461,0x461,
	0x3c, 0x0d, 0x0b,

	NULL
    	
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_SSD1784_8BIT		//to brave feng//11/12/2007
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
     0xffb6,            //rsflag
     7,                 //head num
     {0x0015,0x0000,0x0000,0x0075,0x0000,0x0000,0x005c},
    3, 9, 5, 11,  //stxp, styp, endxp, endyp
    0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    COMMAND_ONLY,
    0x0, 0x0, 0x0, 0x0,
	
    MAINPANEL,                 //config
    PANEL_FMT565_2TIMES|PANEL_RGB,              //fmt : 8bit bus/RGB565
    0x881, 0x7ca0,
    0x5f,0x1f,0x1f,

	NULL
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_NT3911_16BIT	//讯锐//to Brave Feng 12/12/2007
const TLcdifParm gDriverIc_LcdIF =
{
   	 0x00,			//special LCD flag
	 0xfffa,		//rsflag
 	3,			//head num
	{0x21, 0,0x22},
 	3, 4,0,0,   
 	0, 0, 0, 0,	//stxp, styp, endxp, endyp,Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos
 	COMMAND_ONLY,
 	0x0, 0x0, 0x0, 0x0,
  
 	MAINPANEL, 		 //main panel or sub panel
 	PANEL_FMT565_1TIMES|PANEL_RGB,  //fmt
 
 	 0x461,0x461,
	0x3c, 0x0d, 0x0b,

	NULL
    	
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_FT1503_16BIT	//讯锐科技//to BraveFeng//12/14/2007
const TLcdifParm gDriverIc_LcdIF =
{
   	0x00,
	0xffea,		//rsflag
	5,			//head num
	{0x20, 0, 0x21,0,0x22},
	3, 0x0807, 0, 0,	//stxp, styp, endxp, endyp;
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt

	0x461,0x461,
	0x3c, 0x0d, 0x0b,

	NULL
    	
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_LGDP4524_8BIT	//上海华勤//to Terry Zhang//12/10/2007
const TLcdifParm gDriverIc_LcdIF =							//调试过OK!天宇的LCD
{

 0x00,			//special LCD flag
 0xffcc,		//rsflag
 6,			//head num
{0,0x21, 0,0, 0,0x22},
 7, 5,0,0,   
 0, 0, 0, 0,	//stxp, styp, endxp, endyp,Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos
 COMMAND_ONLY,
 0x0, 0x0, 0x0, 0x0,
  
 MAINPANEL, 		 //main panel or sub panel
 PANEL_FMT565_2TIMES|PANEL_RGB,  //fmt

0x460,0x461,
0x3c, 0x0d, 0x0b,

NULL
    	
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_LGDP4524_16BIT	//天脉//wangjian //5/12/2008
const TLcdifParm gDriverIc_LcdIF =
{

 0x00,			//special LCD flag
 0xfffa,		//rsflag
 3,			//head num
{0x21, 0,0x22},
 3, 4,0,0,   
 0, 0, 0, 0,	//stxp, styp, endxp, endyp,Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos
 COMMAND_ONLY,
 0x0, 0x0, 0x0, 0x0,
  
 MAINPANEL, 		 //main panel or sub panel
 PANEL_FMT565_1TIMES|PANEL_RGB,  //fmt

0x461,0x461,
0x3c, 0x0d, 0x0b
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_SPFD5420A_16IT	//Brave //3/11/2008 调试
const TLcdifParm gDriverIc_LcdIF =
{
	0x00,
	0xffea,		//rsflag
	5,			//head num
	{0x200, 0, 0x201, 0, 0x202},
	3, 0x0807, 0, 0,	//stxp, styp, endxp, endyp;
    	0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
    	COMMAND_ONLY,
    	0x0, 0x0, 0x0, 0x0,	
	
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt
	
	0x461,0x461,
	45, 0x05, 0x0b
	
	//0x421, 0x421,			//also preview OK!		24M MCLK 时调试
	//45,3,14
};

#elif V5_MAINDRIVERIC_TYPE == V5_DEF_LCD_HX8340B_8BIT //to Brave//4/18/2008
const TLcdifParm gDriverIc_LcdIF =
{
      0x00,
     0xeaaa,
     13,
     {0x0003,0x0000,0x0002,0x0000,  //start x//start y
     //0x0005, 0x003f, 0x0004, 0x0001,	//end x
      
      0x0007,0x0000, 0x0006, 0x0000, 		//start y
      0x0009,0x00ef, 0x0008, 0x0000, 		//end y
     //0x0009,0x003f, 0x0008, 0x0001, 		//end y
     	
      0x0022
     },
   
     0x0703, 0x0f0b, 0x0, 0x1713, 	//stxp, styp, endx, endy
     0, 0, 0, 0,   //Repeat_start_x_pos,Repeat_start_y_pos,Repeat_end_x_pos,Repeat_end_y_pos,
     COMMAND_ONLY,
     0x0, 0x0, 0x0, 0x0,
 
     MAINPANEL,                 //config
     PANEL_FMT565_2TIMES|PANEL_RGB,              //fmt : 16bit bus/RGB565
   
     0x881, 0x460,
     0x5f, 0x1f, 0x1f	
  
  
};
#else
#endif



/*--------------------------sub panel driver---------*/
#if V5_SUBDRIVERIC_TYPE == NOSUB
const TLcdifParm gSubDriverIc_LcdIF ={ 0 };

#elif V5_SUBDRIVERIC_TYPE == V5_DEF_LCD_HD66777_16BIT
const TLcdifParm gSubDriverIc_LcdIF =
{
	0xffea,		//rsflag
	5,			//head num
	{0x20, 0, 0x21, 0, 0x22},
	3, 7, 0, 0,	//
	MAINPANEL,		//main panel or sub panel
	PANEL_FMT565_1TIMES|PANEL_RGB,		//fmt
	0x460, 0x460,
	0x47,0x1d,0x1b,
};
#elif V5_SUBDRIVERIC_TYPE == V5_DEF_LCD_SSD1788_8BIT
const TLcdifParm gSubDriverIc_LcdIF =
{
     0xffb6,            //rsflag
     7,                 //head num
     {0x0015,0x0000,0x0000,0x0075,0x0000,0x0000,0x005c},
     3, 9, 5, 11,  //stxp, styp, endxp, endyp
    SUBPANEL,                 //config
    PANEL_FMT444_3TIMES_BIG|PANEL_RGB,              //fmt : 8bit bus/RGB565
     0x881, 0x7ca0,
    0x5f,0xf,0xf
};
#elif V5_SUBDRIVERIC_TYPE == V5_DEF_LCD_S6B33B5_8BIT
const TLcdifParm gSubDriverIc_LcdIF =
{
     0xffc0,            //rsflag
     6,                 //head num
     {0x0043,0x0000,0x0000,0x0042,0x0000,0x0000},
     3, 9, 5, 11,  //stxp, styp, endxp, endyp
     SUBPANEL,                 //config
     PANEL_FMT565_2TIMES|PANEL_RGB,              //fmt : 8bit bus/RGB565
     0x880, 0x7ca0,
     0x5f,0x0f,0x0f
};
#elif V5_SUBDRIVERIC_TYPE == V5_DEF_LCD_HD66768_8BIT
const TLcdifParm gSubDriverIc_LcdIF =
{
	0xffcc,		//rsflag
	6,			//head num
	{0,0x21, 0, 0,0,0x22},
	7, 5, 0, 0,	//
	SUBPANEL,		//main panel or sub panel
	PANEL_FMT565_2TIMES|PANEL_RGB,		//fmt
	0x460, 0x460,
	0x47,0x0d,0x0b
};
#elif V5_SUBDRIVERIC_TYPE == V5_DEF_LCD_SSD1332_8BIT
const TLcdifParm gSubDriverIc_LcdIF =
{
    0xffc0,            //rsflag
     6,                 //head num
     {0x0015,0x0000,0x0000,0x0075,0x0000,0x0000},
     3, 9, 5, 11,  //stxp, styp, endxp, endyp
    MAINPANEL,                 //config
    PANEL_FMT565_2TIMES|PANEL_RGB,              //fmt : 8bit bus/RGB565
     0x881, 0x7ca0,
    0x5f,0xf,0xf
};
#else

#endif
const TPanelInfo gDriverIcInfo=
{
	(PTLcdifParm)&gDriverIc_LcdIF,
	(PTLcdifParm)&gSubDriverIc_LcdIF,
};
