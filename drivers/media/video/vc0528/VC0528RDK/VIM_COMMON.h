/*************************************************************************
*                                                                       
*             Copyright (C) 2005 Vimicro CO.,LTD     		 
*
* FILE NAME                                  VERSION                
*                                                                       
* VIM_COMMON.h						           	  0.1                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*     vc0578 's RDK COMMON head file                
*
*	Version		Author			Date		Description		
*  ---------------------------------------------------------------						 
*   0.1			angela  2005-11-01	The first version. 
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
#define VIM_LITTLEENDIAN 0
#define VIM_BIGENDIAN 1


#include"VIM_COMMON_Typedef.h"
#include"VIM_COMMON_Debug.h"
#include"VIM_COMMON_Regdef.h"
#include"VIM_DRIVERIC_Info.h"
#include "VIM_SENSOR_Info.h"




#include"VIM_USER_Info.h"
#include "VIM_HIF_Driver.h"
#include "VIM_SIF_Driver.h"
#include "VIM_DISP_Driver.h"
#include "VIM_LCDIF_Driver.h"
#include "VIM_IPP_Driver.h"
#include "VIM_JPEG_Driver.h"
#include "VIM_MARB_Driver.h"
#include "VIM_HIGH_API.h"
#include "VIM_MID_API.h"
#if VIM_USER_SURPORT_AVI==1
#include "VIM_AVI_Recorder.h"
#endif
#ifndef _VIM_0578RDK_COMMON_H_
#define _VIM_0578RDK_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif




#ifdef __cplusplus
}
#endif

#endif 
