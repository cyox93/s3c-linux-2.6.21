/*************************************************************************
*                                                                       
*             Copyright (C) 2005 Vimicro CO.,LTD     		 
*
* FILE NAME                                  VERSION                
*                                                                       
* VIM_USER_Info.c						           	  0.2                    
*                                                             
* DESCRIPTION                                                           
*                                                                       
*     vc 5X 's customer modify function                        
*
*	Version		Author			Date		Description		
*  ---------------------------------------------------------------						 
*   0.1			angela  2005-11-01	The first version. 
*   0.2			angela  2006-06-06	update for528
*   0.3			angela  2007-02-25	update 
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
#include "..\vc0528demo\inc\nucleus.h"  //"..\GUI\GUI\src\nucleus.h"
#include"VIM_COMMON.h"


#include "..\vc0528demo\inc\thin_usr.h"//"..\FS_library\fat_thin\thin_usr.h"
#include "..\vc0528demo\inc\fat_thin.h"//"..\FS_library\fat_thin\fat_thin.h"

const VIM_USER_INITINFO g_UserInfo=
{
	{
		VIM_BYPASS_USE_REG,	//	VIM_HIF_USERREG,
						  		//VIM_BYPASS_BYREG
		VIM_BYPASS_SUB_CS1,	//VIM_BYPASS_SUB_CS2		  
		0x90	
				// this register address is not in the  panel register address list
	},
	// main panel
	{
		{0,0},
		//{320,240}		//when use demo.prj to test the rdk function, don't use {320, 320} parameters! if used, the rotate picture will produce error!
		{240,320}
	},
	// sub panel
	{
		{0,0},
		{96,64}
	},
	// interrupt 
	{
		VIM_INT_LEVER_HIGH,
		2
	},
	//GPIO  out only
	{
		0xff//value
	},
	//MCLK = (CLKIN*(M/N)*(1/NO))/CLKDIV/2
	{
/*///////////////1 ratio/////////////////
		12000, 	//clk in  
		12000,
		64,	// 24,//Pll m  128
	 	2,//4, 		//Pll n  [4:0]: pll_n  >2  2
	 	4,		//0, //no( 1;2;4,8): pll_od
	 	4,//4,//4		//Clk output div*/
/*///////////////2 ratio/////////////////
		12000, 	//clk in  
		24000,
		64,	// 24,//Pll m  128
	 	2,//4, 		//Pll n  [4:0]: pll_n  >2  2
	 	2,		//0, //no( 1;2;4,8): pll_od
	 	4,//4,//4		//Clk output div*/
/*///////////////4 ratio/////////////////
		12000, 	//clk in  
		48000,
		64,	// 24,//Pll m  128
	 	2,//4, 		//Pll n  [4:0]: pll_n  >2  2
	 	2,		//0, //no( 1;2;4,8): pll_od
	 	2,//4,//4		//Clk output div*/
/*///////////////1 ratio/////////////////
		24000, 	//clk in  
		24000,
		32,	// 24,//Pll m  128
	 	2,//4, 		//Pll n  [4:0]: pll_n  >2  2
	 	2,		//0, //no( 1;2;4,8): pll_od
	 	4,//4,//4	*/

	/*	24000, 	//clk in  
		24000,
		32,	// 24,//Pll m  128
	 	2,//4, 		//Pll n  [4:0]: pll_n  >2  2
	 	2,		//0, //no( 1;2;4,8): pll_od
	 	4,//4,//4	
	 */

	/*	24000, 	//clk in  
		45000,
		45,///32,	// 24,//Pll m  128
		2,/////4, 		//Pll n  [4:0]: pll_n  >2  2
	 	2,		//0, //no( 1;2;4,8): pll_od
	 	3,//3,/////4,//4	

	 	*/

		/*13000, 	//clk in  				//guoying 1/22/2008
		45000,
		90,///32,	// 24,//Pll m  128
		13,/////4, 		//Pll n  [4:0]: pll_n  >2  2
	 	2,		//0, //no( 1;2;4,8): pll_od
	 	1,//3,/////4,//4	*/

	

/*		13000, 	//clk in  				//guoying 1/22/2008//have tested, OK!
		39000,
		48,///32,	// 24,//Pll m  128
		2,/////4, 		//Pll n  [4:0]: pll_n  >2  2
	 	2,		//0, //no( 1;2;4,8): pll_od
	 	2,//3,/////4,//4
*/
	 	
		13000, 	//clk in  				//guoying 1/22/2008//have tested, capture 1280x960 OK!
		45000,
		56,///32,	// 24,//Pll m  128
		2,/////4, 		//Pll n  [4:0]: pll_n  >2  2
	 	2,		//0, //no( 1;2;4,8): pll_od
	 	2,//3,/////4,//4	

/*
		24000, 	//clk in  
		24000,
		32,	// 24,//Pll m  128
	 	2,//4, 		//Pll n  [4:0]: pll_n  >2  2
	 	2,		//0, //no( 1;2;4,8): pll_od
	 	4,//4,//4
*/


/*		13000, 	//clk in  				//guoying 1/22/2008//have tested, can't preview 
		45000,
		225,///32,	// 24,//Pll m  128
		13,/////4, 		//Pll n  [4:0]: pll_n  >2  2
	 	2,		//0, //no( 1;2;4,8): pll_od
	 	2,//3,/////4,//4	
*/
	//have some line on the LCD screen!
/*		13000, 	//clk in  
		52000,
		32,///32,	// 24,//Pll m  128
		2,/////4, 		//Pll n  [4:0]: pll_n  >2  2
	 	2,		//0, //no( 1;2;4,8): pll_od
	 	1,//3,/////4,//4	
*/

	//output 43mhz capture 1280x960 OK
	/*	13000, 	//clk in  
		43000,
		54,///32,	// 24,//Pll m  128
		2,/////4, 		//Pll n  [4:0]: pll_n  >2  2
	 	2,		//0, //no( 1;2;4,8): pll_od
	 	2,//3,/////4,//4	
*/

/*		13000, 	//clk in  
		45000,
		56,///32,	// 24,//Pll m  128
		2,/////4, 		//Pll n  [4:0]: pll_n  >2  2
	 	2,		//0, //no( 1;2;4,8): pll_od
	 	2,//3,/////4,//4	

*/
	},
	//TIMING DELAY 
	{
		8,  //addr_delay_sel
		8,//data_out_delay_sel
		3,//data_in_delay_sel
		3,//data_oen_delay_sel
		0//cs_delay_sel
	},
};
#define DEMOTEST	
#ifndef DEMOTEST	
UINT8 FastPreview;
UINT8 g_Reverse;
#endif
/***************************************************************
Description:
		Read File based on customer's file system.

Parameters:
		pFile_Ptr:  File Handler
		pbBuffer:   data buffer
		pFile_Size: number of bytes to be read	

Returns:
		VIM_SUCCEED: means read file ok
		VIM_ERROR_USER_READFILE: means have something error in read
		file  process
****************************************************************/
UINT32 VIM_USER_ReadFile(HUGE void* pFile_Ptr,  HUGE UINT8* pbBuffer, UINT32 dFile_Size)
{

	UINT32 LENGTH=0;
#ifdef DEMOTEST		
	LENGTH=f_read(pbBuffer, 1, dFile_Size, (F_FILE *)pFile_Ptr);
#endif
	return LENGTH;
}


/***************************************************************
Description:
		Seek File based on customer's file system.

Parameters:
		pFile_Ptr:  File Handler
		dOffset:	   File offset

Returns:
		VIM_SUCCEED: means read file ok
		VIM_ERROR_USER_SEEKFILE: means have something error in seek
		file  process

****************************************************************/

VIM_RESULT VIM_USER_SeekFile(HUGE void* pFile_Ptr, UINT32 dOffset)
{
UINT8 ret;
#ifdef DEMOTEST		
	ret=f_seek((F_FILE *)pFile_Ptr,dOffset,0);
	if(ret)
		return VIM_ERROR_USER_SEEKFILE;
	else
#endif
		return VIM_SUCCEED;
}

/***************************************************************
Description:
		Write File based on customer's file system.

Parameters:
		pFile_Ptr:  File Handler
		pbBuffer:   data buffer
		pFile_Size: number of bytes to be read	

Returns:
		VIM_SUCCEED: means read file ok
		VIM_ERROR_USER_WRITEFILE: means have something error in write
		file  process
****************************************************************/
VIM_RESULT VIM_USER_WriteFile(HUGE void* pFile_Ptr, const HUGE  UINT8* pbBuffer, UINT32 dFile_Size)
{
	UINT32 LENGTH;
#ifdef DEMOTEST		
	F_SPACE space;
	#if(VIM_USER_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintHex("\n VIM_USER_WriteFile ,FILEID =",(int)pFile_Ptr);
	#endif	
	fn_getfreespace(&space);
	if(space.free<dFile_Size+10000)
		return VIM_ERROR_USER_WRITEFILE;

	//VIM_USER_PrintDec(" space.free  =",space.free);
	
	LENGTH=f_write(pbBuffer, 1, dFile_Size, (F_FILE *)pFile_Ptr);
	#if(VIM_USER_DEBUG)&&(VIM_528RDK_DEBUG)
		VIM_USER_PrintDec("\n LENGTH  =",LENGTH);
		VIM_USER_PrintDec("\n real size  =",dFile_Size);
	#endif	
	if(LENGTH!=dFile_Size)
		return VIM_ERROR_USER_WRITEFILE;
	else
#endif
		return VIM_SUCCEED;
}
/***************************************************************
Description:
		Malloc Function

Parameters:
		UINT32 dwbyte :the length of buffer which is needed

Returns:
		void *: return the head point of buf (any format)
		if return 0: means malloc error

****************************************************************/
extern void* MemMalloc(UINT32 size);
HUGE void* VIM_USER_MallocMemory(UINT32 dwbyte)
{
#ifdef DEMOTEST		
#if(VIM_USER_DEBUG)&&(VIM_528RDK_DEBUG)
	VIM_USER_PrintDec("\n malloc memory:  ",dwbyte);
#endif
	return MemMalloc(dwbyte);
#else
	return 0;
#endif
}

/***************************************************************
Description:
		Free Function

Parameters:
		void* mem :the head point of buf
		
Returns:
		VIM_SUCCEED: means free ok
		VIM_ERROR_USER_FREEMEM: means have something error in free process

****************************************************************/
extern void MemFree(void *Prl);
VIM_RESULT VIM_USER_FreeMemory(void* mem)
{
#ifdef DEMOTEST		
	MemFree(mem);
#if(VIM_USER_DEBUG)&&(VIM_528RDK_DEBUG)
	VIM_USER_PrintString("  free memory !");
#endif
#endif
	return VIM_SUCCEED;

}

/***************************************************************
Description:
		Sets buffers to a specified character.

Parameters:
		pbSrc		: Pointer to destination
		bValue		: Character to set
		dCount		: Number of characters

Returns:
		void

****************************************************************/
void	VIM_USER_MemSet(HUGE UINT8 *pbDest,  UINT8 bValue, UINT32 dCount)
{
	UINT32	dSize = dCount;

	while (dSize --)
	{
		pbDest[dSize] = bValue;
	}
}


/***************************************************************
Description:
		Copies count bytes from pbSrc to pbDest.

Parameters:
		pbDest		: New buffer
		pbSrc		: Buffer to copy from
		dCount		: Number of bytes to copy	

Returns:
		void

****************************************************************/
void	VIM_USER_MemCpy(HUGE UINT8 *pbDest, const HUGE UINT8 *pbSrc, UINT32 dCount)
{
	UINT32	dSize = 0;

	while (dSize < dCount)
	{
		pbDest[dSize] = pbSrc[dSize];
		dSize++;
	}
}
/***************************************************************
Description:
		Print data

Parameters:
		string		:the head point of sting
		data:  the data needed  to print (0x...)

Returns:
		void

****************************************************************/
extern void Uart_Printf(char *fmt,...);
void VIM_USER_PrintHex(char *string,UINT32 data)
{
	Uart_Printf("\n%s0x%x",string,data);
}
/***************************************************************
Description:
		Print data

Parameters:
		string		:the head point of sting
		data:  the data needed  to print (dec format)

Returns:
		void

****************************************************************/
void VIM_USER_PrintDec(char *string,UINT32 data)
{
	Uart_Printf("\n%s%d",string,data);
}

/***************************************************************
Description:
		Print data

Parameters:
		string		:the head point of sting

Returns:
		void

****************************************************************/
void VIM_USER_PrintString(char *string)
{
	Uart_Printf("\n%s",string);
}
/***************************************************************
Description:
		Delay function

Parameters:
		ms		:the num ms want to be delay

Returns:
		void

****************************************************************/
extern void Delay(int time); // 1ms 
void VIM_USER_DelayMs(UINT32 ms)
{
	Delay(ms);
	
}
/***************************************************************
Description:
		Check the System is Big Endian or Little Endian
		
Parameters:
		void
		
Returns:
		1 :	Big Endian
		0 :	Little Endian
****************************************************************/
UINT8 VIM_USER_CheckBigendian(void)
{
 	union{
   		UINT32	l;
		UINT8	c[4];
	} u;

 	u.l = 1;
 	return ( u.c[3]==1 );
}

/***************************************************************
Description:
		Start Timer and set timer interval

Parameters:
		interval : the interval of every timer(ms)

Returns:
		0: true
		1-255: error

****************************************************************/
extern UINT8 g_DemoFlage;
NU_TIMER video_timer;
extern void 	InitTimer0ForCommandParase(void (*func)(),int interval);
extern void User_TimerProcess(UNSIGNED val);

VIM_RESULT VIM_USER_StartTimer(UINT32 Intervel)
{
#ifdef DEMOTEST		
#if(VIM_USER_DEBUG)&&(VIM_528RDK_DEBUG)
	VIM_USER_PrintDec("  start timer =",Intervel);
#endif
	if(!g_DemoFlage)
		InitTimer0ForCommandParase(VIM_HAPI_Timer,Intervel);
	else
	{
		NU_Create_Timer(&video_timer,"display video",User_TimerProcess,1,1,(unsigned long)(Intervel/10),NU_DISABLE_TIMER);
		NU_Control_Timer(&video_timer,NU_ENABLE_TIMER);
	}
#endif
	return VIM_SUCCEED;
}
/***************************************************************
Description:
		cancel Timer 
Parameters:

Returns:
		0: true
		1-255: error

****************************************************************/
extern void Timer0_stop(void);
VIM_RESULT VIM_USER_StopTimer(void)
{
#ifdef DEMOTEST		
#if(VIM_USER_DEBUG)&&(VIM_528RDK_DEBUG)
	VIM_USER_PrintString("  stop timer");
#endif
	if(!g_DemoFlage)
		Timer0_stop();
	else
		NU_Control_Timer(&video_timer,NU_DISABLE_TIMER);
#endif
	return VIM_SUCCEED;
}

/***************************************************************
Description:
	chip reset 
Parameters:

Returns:

****************************************************************/

void VIM_USER_Reset(void)
{
	//reset jpeg ic.
	(*(volatile unsigned *)0x1d20044) = 0x7f;
	Delay(100);
	(*(volatile unsigned *)0x1d20044) = 0xff;
	Delay(100);
}
