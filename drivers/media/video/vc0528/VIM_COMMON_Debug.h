/*************************************************************************
*                                                                       
*             Copyright (C) 2005 Vimicro CO.,LTD     		 
*
* FILE NAME                                  VERSION                
*                                                                       
* VIM_COMMON_Tebug.h						           	  0.1                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*     vc0528 's debuf define head file                          
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
#ifndef _VIM_5XRDK_COMMON_DEBUG_H_
#define _VIM_5XRDK_COMMON_DEBUG_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DEBUG_FLAG
#define DEBUG_FLAG

#if 1 
#define		VIM_528RDK_DEBUG		0

#define		VIM_HIGH_API_DEBUG		0
#define		VIM_MID_API_DEBUG		0
#define		VIM_SIF_DEBUG			0
#define		VIM_HIF_DEBUG			0
#define		VIM_LCDIF_DEBUG			0
#define		VIM_IPP_DEBUG			0
#define		VIM_MARB_DEBUG			0
#define		VIM_JPEG_DEBUG			0
#define		VIM_DISP_DEBUG			0
#define		VIM_USER_DEBUG			0
#else 
#define		VIM_528RDK_DEBUG		1

#define		VIM_HIGH_API_DEBUG		1
#define		VIM_MID_API_DEBUG		1
#define		VIM_SIF_DEBUG			1
#define		VIM_HIF_DEBUG			1
#define		VIM_LCDIF_DEBUG			1
#define		VIM_IPP_DEBUG			1
#define		VIM_MARB_DEBUG			1
#define		VIM_JPEG_DEBUG			1
#define		VIM_DISP_DEBUG			1
#define		VIM_USER_DEBUG			1
#endif



typedef enum _VIM_RESULT
{
/////////////succeed/////////////////////////////////////////
	VIM_SUCCEED=0,
/////////////common error///////1-10//////////////////////////
	VIM_ERROR_TIME_OUT=1,
	VIM_ERROR_PARAMETER=2,
	VIM_ERROR_FILE_END=3,
/////////////high api error 100-200////////////////////////////
	VIM_ERROR_WORKMODE=100,
	VIM_ERROR_PREVIEWMODE=101,
	VIM_ERROR_BUFFERLENGTHS=102,
	VIM_ERROR_NOINITERRUPT=103,
	VIM_ERROR_BRIGHT_CONTRAST=104,
	VIM_ERROR_LCDWORKMODE=105,
	VIM_ERROR_NOPIC_INBUF=106,
	VIM_ERROR_BUFPOINT=107,
	VIM_ERROR_CANNOT_CAPVIDEO=108,
	VIM_ERROR_VIDEO_MODE=109,
	VIM_ERROR_MAX_FRAMECOUNT=110,
	VIM_ERROR_LCD_UPDATERANGE=111,
	VIM_ERROR_LCD_DRAWOVERRANGE=112,
	VIM_ERROR_CANNOTCAPT_FRAMTHUMB=113,
	VIM_ERROR_PREVIEW_PARA=114,
	VIM_ERROR_FASET_PREVIEW=115,
	VIM_ERROR_IPP_CANNOTSUPPORT=116,
	VIM_ERROR_DECODE=117,
	VIM_ERROR_CANNOT_QUICK_DISPLAY=118,
	VIM_ERROR_WHITEBALANCE=119,
/////////////mid api error 200-300////////////////////////////
	VIM_ERROR_HIF_READWRITE_REG8=201,
	VIM_ERROR_DEFINE_ENDIAN=202,
	VIM_ERROR_HIF_READWRITE_REG16=203,
	VIM_ERROR_IPP_READWRITE_REG8=204,
	VIM_ERROR_READWRITE_SRAM=205,
	VIM_ERROR_JBUF_INTERRUPT_BLOCK=206,

	VIM_ERROR_PREVIEW_CAPTURE_ZOOM_PARA=210,
	VIM_ERROR_PREVIEW_DISPLAY_ZOOM_PARA=211,

	VIM_ERROR_CAPTURE_RECAPTURE=212,
	VIM_ERROR_CAPTURE_OVERFLOW=213,	
	VIM_ERROR_CAPTURE_MARB=214,	
	VIM_ERROR_CAPTURE_BUFFULL=215,	
	VIM_ERROR_CAPTURE_ROMOVERFLOW=216,	

	VIM_ERROR_DECODE_DOWNSIZEOVERFLOW=217,	//this add by amanda,because 578 420 height can not down size more than 1/16
	                                                                                    //444,422,410,400 can not downsize more than 1/8
	VIM_ERROR_ADJUST_OFFSET=218,
	VIM_ERROR_PREVIEW_BASE_ZOOM_PARA=219,

/////////////sensor(isp) interface error 300-400////////////////////////////
	VIM_ERROR_SENSOR_NOINFO=301,
	VIM_ERROR_SENSOR_AUTOFIND=302,
	VIM_ERROR_SENSOR_RESOLUTION=303,
	VIM_ERROR_SENSOR_INPUTSIZE=304,
	VIM_ERROR_SENSOR_STATE=305,
	VIM_ERROR_SENSOR_NOROSELUTION=306,
	VIM_ERROR_SENSOR_RGB=307,
/////////////lcd interface error 400-500////////////////////////////




/////////////hif(cpm) interface error 500-600////////////////////////////
	VIM_ERROR_PLL_INCLK=501,
	VIM_ERROR_PLL_N=502,
	VIM_ERROR_PLL_M=503,
	VIM_ERROR_PLL_OUT=504,
	VIM_ERROR_PLL_NO=505,
	VIM_ERROR_PLL_MARB=506,
	VIM_ERROR_INTERRUPT_INIT=507,
	VIM_ERROR_INTERRUPT_EN=508,
/////////////ipp(line buf)  error 600-700////////////////////////////
	VIM_ERROR_DISPLAY_WINDOW=601,
	VIM_ERROR_CAPTURE_WINDOW=602,
	VIM_ERROR_SOURCE_WINDOW=603,
	VIM_ERROR_THUMB_WINDOW=604,
	VIM_ERROR_CACAULATELE_SOURCE_WINDOW=605,		//add by amanda deng
	VIM_ERROR_CAPTURE_PARAMETER=606,			//add by amanda deng
	VIM_ERROR_PREVIEW_PARAMETER=607,			//add by amanda deng
	VIM_ERROR_UP_SOURCE_WINDOW=608,
	VIM_ERROR_UP_CAPTURE_WINDOW=609,
	VIM_ERROR_UP_RESIZE_OVER=610,
	VIM_ERROR_THUMB_HEIGHT_MORE=611,
/////////////marb error 700-800////////////////////////////
	VIM_ERROR_INDEX_MAP=701,
	VIM_ERROR_THUMB_MAP=702,
	VIM_ERROR_LINEBUF_MAP=703,
	VIM_ERROR_JBUF_MAP=704,
	VIM_ERROR_ALAYER_MAP=705,
	VIM_ERROR_B1_MAP=706,
	VIM_ERROR_B0_MAP=707,
/////////////jpeg (line buf)  error 800-900////////////////////////////
	VIM_ERROR_DECODE_ERROR=801,
	VIM_ERROR_ENCODE_ERROR=802,
	VIM_ERROR_DELAY_FRAME=803,
	VIM_ERROR_YUV_UNKNOW=804,


/////////////disp error 900-1000////////////////////////////
	VIM_ERROR_DISP_PARAM = 901,
	VIM_ERROR_DISP_MODE = 902,
	VIM_ERROR_DISP_BLAYER = 903,
	VIM_ERROR_DISP_B0B1_SUPERPOS = 904,

/////////////user error 1000-1100////////////////////////////
	VIM_ERROR_USER_FREEMEM=1000,
	VIM_ERROR_USER_READFILE=1001,
	VIM_ERROR_USER_WRITEFILE=1002,
	VIM_ERROR_USER_SEEKFILE=1003,
	VIM_ERROR_USER_STARTTIMER=1004,
	VIM_ERROR_USER_STOPTIMER=1005
}VIM_RESULT;






#endif




#ifdef __cplusplus
}
#endif

#endif 
