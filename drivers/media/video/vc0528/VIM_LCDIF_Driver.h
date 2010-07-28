
/*************************************************************************
*                                                                       
*                Copyright (C) 2005 Vimicro CO.,LTD     		 
*
* FILE NAME                                  VERSION                
*                                                                       
* VIM_LCDIF_Driver.C			   0.2                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*     VC5X 's lcdif moudle sub driver  head file                               
*
*	Version		Author			Date		Description		
*  ---------------------------------------------------------------						 
*   0.1			Liuhd		2005-11-10	The first version. 
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

#ifndef _VIM_5XRDK_LCDIF_H_
#define _VIM_5XRDK_LCDIF_H_
#include  "VIM_COMMON.h"
#include  "VIM_HIF_Driver.h"

#ifdef __cplusplus
extern "C" {
#endif


#define  DELAY_USE_INFO		1			//wendy guo modifyied from 0 to 1, 2/16/2007



//lcdif head config
typedef enum _VIM_LCDIF_SEL_CS
{
	VIM_LCDIF_CS1						=0,
	VIM_LCDIF_CS2		
}VIM_LCDIF_SEL_CS;



void VIM_LCDIF_InitLcdIFParm(PTLcdifParm parm);
void VIM_LCDIF_ResetPanel(void);
void VIM_LCDIF_RsLExW(UINT16 uVal);
void VIM_LCDIF_RsHExW(UINT16 uVal);
UINT16 VIM_LCDIF_RsLExR(void);
UINT16 VIM_LCDIF_RsHExR(void);
void VIM_LCDIF_SetBiasSpt(TPoint pt);


#ifdef __cplusplus
}
#endif


#endif
