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
*   0.2			angela		2006-06-06	update for 528
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

static const UINT8 gFirstLevelIntPriority[VIM_FIRST_LEVEL_INT_NUM]
	= {INT_MARBADD,INT_JPEG, INT_MARB,INT_SIF, INT_ISP, INT_LBUF, INT_GE, INT_LCDC};



//----------------------------------------------------------------
//---------------------------------------------------------------------------------
// Internal functions

#if VIM_BUSTYPE == VIM_MULTI16
/***************************************************************
Description:
		set 8bit register use multi16

Parameters:
		adr: register address
		val:	   register value

Returns:
		void
****************************************************************/
 void VIM_HIF_SetReg8(UINT32 adr, UINT8 val)       
{
	UINT16 uTemp;

	VIM_SET_RSLOW_BY_GPIO();
	*(volatile UINT16 *)(VIM_REG_INDEX) = (UINT16)adr;
	
	VIM_SET_RSHIGH_BY_GPIO();
	uTemp = *(volatile UINT16 *)(VIM_REG_VALUE);         
	
	if((adr & 0x1) == 0)
	{
		*(volatile UINT16 *)(VIM_REG_VALUE) = (uTemp&0xff00)|val;     
	}
	else
	{
		*(volatile UINT16 *)(VIM_REG_VALUE) = (uTemp&0xff)|(val<<8);
	}
}
/***************************************************************
Description:
		get 8bit register use multi16

Parameters:
		adr: register address
		val:	   register value

Returns:
		return uint 8
****************************************************************/
UINT8 VIM_HIF_GetReg8(UINT32 adr)
{
UINT8 TEMP;

	VIM_SET_RSLOW_BY_GPIO();
	*(volatile UINT16 *)(VIM_REG_INDEX) = (UINT16)adr;
		
	VIM_SET_RSHIGH_BY_GPIO();
	if((adr & 0x1) == 0)
	{
		TEMP=(UINT8)*(volatile UINT16 *)(VIM_REG_VALUE);
	}
	else
		TEMP= (UINT8)((*(volatile UINT16 *)(VIM_REG_VALUE))>>8);
	return TEMP;
}
/***************************************************************
Description:
		set 16bit register use multi16

Parameters:
		adr: register address
		val:	   register value

Returns:
		void
****************************************************************/
void VIM_HIF_SetReg16(UINT32 adr, UINT16 val)       
{
	UINT16 uTemp;

	if((adr & 0x1) == 0)
	{
		VIM_SET_RSLOW_BY_GPIO();
		*(volatile UINT16 *)(VIM_REG_INDEX) = (UINT16)adr;
		VIM_SET_RSHIGH_BY_GPIO();
		*(volatile UINT16 *)(VIM_REG_VALUE) = val;
	}
	else
	{
		VIM_SET_RSLOW_BY_GPIO();
		*(volatile UINT16 *)(VIM_REG_INDEX) = (UINT16)adr-0x01;
		VIM_SET_RSHIGH_BY_GPIO();
		uTemp = *(volatile UINT16 *)(VIM_REG_VALUE);
		*(volatile UINT16 *)(VIM_REG_VALUE) = (uTemp&0xff) | (val<<8);	// È¡µÍ°ËÎ»

		VIM_SET_RSLOW_BY_GPIO();
		*(volatile UINT16 *)(VIM_REG_INDEX) = (UINT16)adr+0x01;
		VIM_SET_RSHIGH_BY_GPIO();
		uTemp = *(volatile UINT16 *)(VIM_REG_VALUE);
		*(volatile UINT16 *)(VIM_REG_VALUE) = (uTemp&0xff00) | (val>>8);	// È¡¸ß°ËÎ»
	}
}
/***************************************************************
Description:
		get 16bit register use multi16

Parameters:
		adr: register address
		val:	   register value

Returns:
		return uint 16
****************************************************************/
UINT16 VIM_HIF_GetReg16(UINT32 adr)
{
	UINT16 uTemp;
	if((adr & 0x1) == 0)
	{
		VIM_SET_RSLOW_BY_GPIO();
		*(volatile UINT16 *)(VIM_REG_INDEX) = (UINT16)adr;			
		VIM_SET_RSHIGH_BY_GPIO();
		uTemp = (UINT16)(*(volatile UINT16 *)(VIM_REG_VALUE));
	}
	else
	{
		VIM_SET_RSLOW_BY_GPIO();
		*(volatile UINT16 *)(VIM_REG_INDEX) = (UINT16)adr-0x01;			
		VIM_SET_RSHIGH_BY_GPIO();
		uTemp = (0xff&((UINT16)(*(volatile UINT16 *)(VIM_REG_VALUE))>>8));

		VIM_SET_RSLOW_BY_GPIO();
		*(volatile UINT16 *)(VIM_REG_INDEX) = (UINT16)adr+0x01;			
		VIM_SET_RSHIGH_BY_GPIO();
		uTemp |= ((UINT16)(*(volatile UINT16 *)(VIM_REG_VALUE))<<8);
	}
	return uTemp;
}

/***************************************************************
Description:
		read sdram 8bit

Parameters:
		adr: sdram start address
		buf: the head point of buf
		size:	  the lenght of data(byte)

Returns:
		void
****************************************************************/

UINT32 VIM_HIF_ReadSram(UINT32 adr, HUGE UINT8 *buf, UINT32 size)
{
	UINT32 actSize;
	UINT16 uTemp;

	if(adr > V5_SRAM_TOTALSIZE)
	{
		return 0;
	}
	if(size > (V5_SRAM_TOTALSIZE - adr))
	{
		size = V5_SRAM_TOTALSIZE - adr;
	}

	size >>= 1;
	
	actSize = size << 1;

	//Set sram start address
	adr += V5_SRAM_BASE;
	
	VIM_HIF_SetReg8(V5_REG_BIU_SEL_WRITE_READ, 0x0);

	VIM_HIF_SetReg8(V5_REG_BIU_MEM_LOW_WORD_L, (UINT8)adr);
	VIM_HIF_SetReg8(V5_REG_BIU_MEM_LOW_WORD_H, (UINT8)(adr>>8));
	VIM_HIF_SetReg8(V5_REG_BIU_MEM_HIGH_WORD,  (UINT8)(adr>>16));

	VIM_SET_RSLOW_BY_GPIO();
	//Set multi6 emory port addr
	*(volatile UINT16 *)(VIM_REG_INDEX) = (UINT16)V5_MULTI16_MEM_FLG;
        
	VIM_SET_RSHIGH_BY_GPIO();
	while(size--)
	{
		uTemp = *(volatile UINT16*)(VIM_REG_VALUE);
		*buf++ = (UINT8)uTemp;
		*buf++ = (UINT8)(uTemp>>8);
	}
	
	return actSize;
}
/***************************************************************
Description:
		write sdram 8bit reverse

Parameters:
		adr: sdram start address
		buf: the head point of buf
		size:	  the lenght of data(byte)

Returns:
		void
****************************************************************/
UINT32 VIM_HIF_WriteSramReverse(UINT32 adr, const HUGE UINT8 *buf, UINT32 size)
{
	UINT32 actSize;
	UINT16 uTemp;
        
	if(adr > V5_SRAM_TOTALSIZE)                    
	{
		return 0;
	}
	if(size > (V5_SRAM_TOTALSIZE - adr))
	{
		size = V5_SRAM_TOTALSIZE - adr;
	}

	size >>= 1;
	actSize = size << 1;

	//Set sram start address
	adr += V5_SRAM_BASE;

	VIM_HIF_SetReg8(V5_REG_BIU_SEL_WRITE_READ, 0x1);

	VIM_HIF_SetReg8(V5_REG_BIU_MEM_LOW_WORD_L, (UINT8)adr);
	VIM_HIF_SetReg8(V5_REG_BIU_MEM_LOW_WORD_H, (UINT8)(adr>>8));
	VIM_HIF_SetReg8(V5_REG_BIU_MEM_HIGH_WORD,  (UINT8)(adr>>16));

	VIM_SET_RSLOW_BY_GPIO();	
	//Set multi6 emory port addr
	*(volatile UINT16 *)(VIM_REG_INDEX) = (UINT16)V5_MULTI16_MEM_FLG;
        
	VIM_SET_RSHIGH_BY_GPIO();
	while(size--)
	{
		uTemp =  ((UINT16)(*buf++))<<8;//*buf++;
		uTemp += *buf++;
		*(volatile UINT16*)(VIM_REG_VALUE) = uTemp;
	}

	return actSize;
}
/***************************************************************
Description:
		write sdram 8bit

Parameters:
		adr: sdram start address
		buf: the head point of buf
		size:	  the lenght of data(byte)

Returns:
		void
****************************************************************/

 UINT32 VIM_HIF_WriteSram(UINT32 adr, const HUGE UINT8 *buf, UINT32 size)
{
	UINT32 actSize;
	UINT16 uTemp;
        
	if(adr > V5_SRAM_TOTALSIZE)                    
	{
		return 0;
	}
	if(size > (V5_SRAM_TOTALSIZE - adr))
	{
		size = V5_SRAM_TOTALSIZE - adr;
	}

	size >>= 1;
	actSize = size << 1;

	//Set sram start address
	adr += V5_SRAM_BASE;

	VIM_HIF_SetReg8(V5_REG_BIU_SEL_WRITE_READ, 0x1);

	VIM_HIF_SetReg8(V5_REG_BIU_MEM_LOW_WORD_L, (UINT8)adr);
	VIM_HIF_SetReg8(V5_REG_BIU_MEM_LOW_WORD_H, (UINT8)(adr>>8));
	VIM_HIF_SetReg8(V5_REG_BIU_MEM_HIGH_WORD,  (UINT8)(adr>>16));

	VIM_SET_RSLOW_BY_GPIO();	
	//Set multi6 emory port addr
	*(volatile UINT16 *)(VIM_REG_INDEX) = (UINT16)V5_MULTI16_MEM_FLG;
        
	VIM_SET_RSHIGH_BY_GPIO();
	while(size--)
	{
		uTemp = *buf++;
		uTemp += ((UINT16)(*buf++))<<8;
		*(volatile UINT16*)(VIM_REG_VALUE) = uTemp;
	}

	return actSize;
}
/***************************************************************
Description:
		write sdram 8bit on word

Parameters:
		adr: sdram start address
		buf: the head point of buf
		size:	  the lenght of data(byte)

Returns:
		void
****************************************************************/
// size in byte unit
UINT32 VIM_HIF_WriteSramOnWord(UINT32 adr, UINT32 dat, UINT32 size)
{
	UINT32 actSize;
        
	if(adr > V5_SRAM_TOTALSIZE)                    
	{
		return 0;
	}
	if(size > (V5_SRAM_TOTALSIZE - adr))
	{
		size = V5_SRAM_TOTALSIZE - adr;
	}

	size >>= 2;
	actSize = size << 2;

	//Set sram start address
	adr += V5_SRAM_BASE;

	VIM_HIF_SetReg8(V5_REG_BIU_SEL_WRITE_READ, 0x1);

	VIM_HIF_SetReg8(V5_REG_BIU_MEM_LOW_WORD_L, (UINT8)adr);
	VIM_HIF_SetReg8(V5_REG_BIU_MEM_LOW_WORD_H, (UINT8)(adr>>8));
	VIM_HIF_SetReg8(V5_REG_BIU_MEM_HIGH_WORD,  (UINT8)(adr>>16));

	VIM_SET_RSLOW_BY_GPIO();	
	//Set multi6 emory port addr
	*(volatile UINT16 *)(VIM_REG_INDEX) = (UINT16)V5_MULTI16_MEM_FLG;
        
	VIM_SET_RSHIGH_BY_GPIO();
	while(size--)
	{
		*(volatile UINT16*)(VIM_REG_VALUE) = (UINT16)dat;
		*(volatile UINT16*)(VIM_REG_VALUE) = (UINT16)(dat >> 16);
	}

	return actSize;
}

#elif VIM_BUSTYPE == VIM_MULTI8
/***************************************************************
Description:
		set 8bit register use multi8

Parameters:
		adr: register address
		val:	   register value

Returns:
		void
****************************************************************/
void VIM_HIF_SetReg8(UINT32 adr, UINT8 val)     
{
	VIM_SET_RSLOW_BY_GPIO();	
	*(volatile UINT8 *)(VIM_REG_INDEX) = (UINT8)V5_MULTI8_REG_WORDH;
	
	VIM_SET_RSHIGH_BY_GPIO();
	*(volatile UINT8 *)(VIM_REG_VALUE) = (UINT8)(adr>>8);
	
	VIM_SET_RSLOW_BY_GPIO();	
	*(volatile UINT8 *)(VIM_REG_INDEX) = (UINT8)V5_MULTI8_REG_WORDL;
	
	VIM_SET_RSHIGH_BY_GPIO();
	*(volatile UINT8 *)(VIM_REG_VALUE) = (UINT8)adr;
	
	VIM_SET_RSLOW_BY_GPIO();
	*(volatile UINT8 *)(VIM_REG_INDEX) = (UINT8)V5_MULTI8_REG_PORT;
	
	VIM_SET_RSHIGH_BY_GPIO();
	*(volatile UINT8 *)(VIM_REG_VALUE) = val;
}
/***************************************************************
Description:
		get 8bit register use multi8

Parameters:
		adr: register address
		val:	   register value

Returns:
		return uint 8
****************************************************************/
UINT8 VIM_HIF_GetReg8(UINT32 adr)
{
	VIM_SET_RSLOW_BY_GPIO();	
	*(volatile UINT8 *)(VIM_REG_INDEX) = (UINT8)V5_MULTI8_REG_WORDH;

	VIM_SET_RSHIGH_BY_GPIO();
	*(volatile UINT8 *)(VIM_REG_VALUE) = (UINT8)(adr>>8);
	
	VIM_SET_RSLOW_BY_GPIO();	
	*(volatile UINT8 *)(VIM_REG_INDEX) = (UINT8)V5_MULTI8_REG_WORDL;
	
	VIM_SET_RSHIGH_BY_GPIO();
	*(volatile UINT8 *)(VIM_REG_VALUE) = (UINT8)adr;
	
	VIM_SET_RSLOW_BY_GPIO();	
	*(volatile UINT8 *)(VIM_REG_INDEX) = (UINT8)V5_MULTI8_REG_PORT;

	VIM_SET_RSHIGH_BY_GPIO();
	
	return *(volatile UINT8 *)(VIM_REG_VALUE);
}
/***************************************************************
Description:
		set 16bit register use multi8

Parameters:
		adr: register address
		val:	   register value

Returns:
		void
****************************************************************/
void VIM_HIF_SetReg16(UINT32 adr, UINT16 val)       
{
		VIM_HIF_SetReg8(adr, (UINT8)val);
		VIM_HIF_SetReg8(adr+0x01,(UINT8)(val >> 8));
}
/***************************************************************
Description:
		get 16bit register use multi8

Parameters:
		adr: register address
		val:	   register value

Returns:
		return uint 16 
****************************************************************/
UINT16 VIM_HIF_GetReg16(UINT32 adr)
{
UINT16 byValue;
		byValue = VIM_HIF_GetReg8(adr);
		byValue |= (VIM_HIF_GetReg8(adr+0x01)<<8);
		return byValue;
}
/***************************************************************
Description:
		read sdram 8bit

Parameters:
		adr: sdram start address
		buf: the head point of buf
		size:	  the lenght of data(byte)

Returns:
		void
****************************************************************/
 UINT32 VIM_HIF_ReadSram(UINT32 adr, HUGE UINT8 *buf, UINT32 size)
{
	UINT32 actSize;

	if(adr > V5_SRAM_TOTALSIZE)
	{
		return 0;
	}
	if(size > (V5_SRAM_TOTALSIZE - adr))
	{
		size = V5_SRAM_TOTALSIZE - adr;
	}

	actSize = size;

	//Set sram start address
	adr += V5_SRAM_BASE;

	VIM_HIF_SetReg8(V5_REG_BIU_SEL_WRITE_READ, 0x0);

	VIM_HIF_SetReg8(V5_REG_BIU_MEM_LOW_WORD_L, (UINT8)adr);
	VIM_HIF_SetReg8(V5_REG_BIU_MEM_LOW_WORD_H, (UINT8)(adr>>8));
	VIM_HIF_SetReg8(V5_REG_BIU_MEM_HIGH_WORD,  (UINT8)(adr>>16));

	VIM_SET_RSLOW_BY_GPIO();	
	//Set multi8 emory port addr
	*(volatile UINT8 *)(VIM_REG_INDEX) = V5_MULTI8_MEM_PORT;

	VIM_SET_RSHIGH_BY_GPIO();
	while(size--)
	{
		*buf++ = *(volatile UINT8 *)(VIM_REG_VALUE);   
	}

	return actSize;
}
/***************************************************************
Description:
		write sdram 8bit reverse

Parameters:
		adr: sdram start address
		buf: the head point of buf
		size:	  the lenght of data(byte)

Returns:
		void
****************************************************************/
UINT32 VIM_HIF_WriteSramReverse(UINT32 adr, const HUGE UINT8 *buf, UINT32 size)
{
	UINT32 actSize,i;

	if(adr > V5_SRAM_TOTALSIZE)
	{
		return 0;
	}
	if(size > (V5_SRAM_TOTALSIZE - adr))
	{
		size = V5_SRAM_TOTALSIZE - adr;
	}

	actSize = size;

	//Set sram start address
	adr += V5_SRAM_BASE;

	VIM_HIF_SetReg8(V5_REG_BIU_SEL_WRITE_READ, 0x1);

	VIM_HIF_SetReg8(V5_REG_BIU_MEM_LOW_WORD_L, (UINT8)adr);
	VIM_HIF_SetReg8(V5_REG_BIU_MEM_LOW_WORD_H, (UINT8)(adr>>8));
	VIM_HIF_SetReg8(V5_REG_BIU_MEM_HIGH_WORD,  (UINT8)(adr>>16));

	VIM_SET_RSLOW_BY_GPIO();	
	//Set multi8 emory port addr
	*(volatile UINT8 *)(VIM_REG_INDEX) = V5_MULTI8_MEM_PORT;

	VIM_SET_RSHIGH_BY_GPIO();
	for(i=0;i<size/2;i++)
	{
		*(volatile UINT8 *)(VIM_REG_VALUE) = buf[1];
		*(volatile UINT8 *)(VIM_REG_VALUE) = buf[0];
		buf+=2;
	}
	return actSize;
}
/***************************************************************
Description:
		write sdram 8bit

Parameters:
		adr: sdram start address
		buf: the head point of buf
		size:	  the lenght of data(byte)

Returns:
		void
****************************************************************/
UINT32 VIM_HIF_WriteSram(UINT32 adr, const HUGE UINT8 *buf, UINT32 size)
{
	UINT32 actSize;

	if(adr > V5_SRAM_TOTALSIZE)
	{
		return 0;
	}
	if(size > (V5_SRAM_TOTALSIZE - adr))
	{
		size = V5_SRAM_TOTALSIZE - adr;
	}

	actSize = size;

	//Set sram start address
	adr += V5_SRAM_BASE;

	VIM_HIF_SetReg8(V5_REG_BIU_SEL_WRITE_READ, 0x1);

	VIM_HIF_SetReg8(V5_REG_BIU_MEM_LOW_WORD_L, (UINT8)adr);
	VIM_HIF_SetReg8(V5_REG_BIU_MEM_LOW_WORD_H, (UINT8)(adr>>8));
	VIM_HIF_SetReg8(V5_REG_BIU_MEM_HIGH_WORD,  (UINT8)(adr>>16));

	VIM_SET_RSLOW_BY_GPIO();	
	//Set multi8 emory port addr
	*(volatile UINT8 *)(VIM_REG_INDEX) = V5_MULTI8_MEM_PORT;

	VIM_SET_RSHIGH_BY_GPIO();
	while(size--)
	{
		*(volatile UINT8 *)(VIM_REG_VALUE) = *buf++;
	}
	return actSize;
}
/***************************************************************
Description:
		write sdram 8bit on word

Parameters:
		adr: sdram start address
		buf: the head point of buf
		size:	  the lenght of data(byte)

Returns:
		void
****************************************************************/
// size in byte unit
UINT32 VIM_HIF_WriteSramOnWord(UINT32 adr, UINT32 dat, UINT32 size)
{
	UINT32 actSize;
        
	if(adr > V5_SRAM_TOTALSIZE)                    
	{
		return 0;
	}
	if(size > (V5_SRAM_TOTALSIZE - adr))
	{
		size = V5_SRAM_TOTALSIZE - adr;
	}

	size >>= 2;
	actSize = size << 2;

	//Set sram start address
	adr += V5_SRAM_BASE;

	VIM_HIF_SetReg8(V5_REG_BIU_SEL_WRITE_READ, 0x1);

	VIM_HIF_SetReg8(V5_REG_BIU_MEM_LOW_WORD_L, (UINT8)adr);
	VIM_HIF_SetReg8(V5_REG_BIU_MEM_LOW_WORD_H, (UINT8)(adr>>8));
	VIM_HIF_SetReg8(V5_REG_BIU_MEM_HIGH_WORD,  (UINT8)(adr>>16));

	VIM_SET_RSLOW_BY_GPIO();	
	//Set multi8 emory port addr
	*(volatile UINT8 *)(VIM_REG_INDEX) = V5_MULTI8_MEM_PORT;
        
	VIM_SET_RSHIGH_BY_GPIO();
	while(size--)
	{
		*(volatile UINT8 *)(VIM_REG_VALUE) = (UINT8)dat;
		*(volatile UINT8 *)(VIM_REG_VALUE) = (UINT8)(dat >> 8);
		*(volatile UINT8 *)(VIM_REG_VALUE) = (UINT8)(dat >> 16);
		*(volatile UINT8 *)(VIM_REG_VALUE) = (UINT8)(dat >> 24);
	}

	return actSize;
}

#endif // VIM_MULTI8

////////////////////////end internal function/////////////////////

/*-----------------------------------------------------------------------------
  Summary:
	Set register value by 32 bit
  Parameters:
	adr : Register address£¬UINT32
	val : Register value£¬UINT32
  Note:
  Returns: void
  Remarks:
--------------------------------------------------------------------------------*/
void	VIM_HIF_SetReg32(UINT32 adr,UINT32 val)
{
		VIM_HIF_SetReg16(adr,val);
		VIM_HIF_SetReg16(adr+0x02,val>>16);	
}

/*-----------------------------------------------------------------------------
  Summary:
	Get register value by 32 bit
  Parameters:
	adr : Register address£¬UINT32
  Note:
  Returns: Register value£¬UINT32
  Remarks:
--------------------------------------------------------------------------------*/
UINT32	VIM_HIF_GetReg32(UINT32 adr)
{
	UINT32 byValue = 0;
	byValue = VIM_HIF_GetReg16(adr);
	byValue |= (VIM_HIF_GetReg16(adr+0x02)<<16);
	return byValue;
}



/*------------------------------------------
 V5_REG_BIU_REG_8_FLG				(V5_REG_BIU_BASE+0x0B0)
	//Register port when 8 bit multiplex bus is used.
 V5_REG_BIU_REG_8_LOW_WORD			(V5_REG_BIU_BASE+0x0B2)
	//De fault value 8¡¯b0  Register low address when 8 bit multiplex bus is used.
 V5_REG_BIU_REG_8_HIGH_WORD			(V5_REG_BIU_BASE+0x0B4)
        //De fault value 8¡¯b0  Register high address when 8 bit multiplex bus is used.

V5_REG_BIU_SEL_8_16					(V5_REG_BIU_BASE+0x08C)
	This register express data bus width of host.
	0: it is 8 bit data width.  
	1: it is 16 bit data width.  
	De fault value - 1¡¯b0
********************************************************************************
Description:    //set the 8 /16 bit width  ÉèÖÃÎ»¿í²¢³õÊ¼»¯
   
De fault value 
*********************************************************************************
------------------------------------------*/

void VIM_HIF_SetMulti16(void)
{
	VIM_SET_RSLOW_BY_GPIO();	
	*(volatile UINT8 *)(VIM_REG_INDEX) = (UINT8)V5_MULTI8_REG_WORDH;
	
	VIM_SET_RSHIGH_BY_GPIO();
	*(volatile UINT8 *)(VIM_REG_VALUE) = (UINT8)(V5_REG_BIU_SEL_8_16>>8);//(0x18);
	
	VIM_SET_RSLOW_BY_GPIO();	
	*(volatile UINT8 *)(VIM_REG_INDEX) = (UINT8)V5_MULTI8_REG_WORDL;
	
	VIM_SET_RSHIGH_BY_GPIO();
	*(volatile UINT8 *)(VIM_REG_VALUE) = (UINT8)(V5_REG_BIU_SEL_8_16);//(UINT8)0x8c;
	
	VIM_SET_RSLOW_BY_GPIO();
	*(volatile UINT8 *)(VIM_REG_INDEX) = (UINT8)V5_MULTI8_REG_PORT;
	
	VIM_SET_RSHIGH_BY_GPIO();
	*(volatile UINT8 *)(VIM_REG_VALUE) = 1;
}




/*-----------------------------------------------------------------------------
  Summary:
	Control Modul Reset
  Parameters:
	wVal : Set which Modul Reset£¬enum VIM_HIF_CpmModReset_CTRLenum
  Note: Ä¬ÈÏ 1:OFF 0:ON
  Returns: void
  Remarks: 
  			This register is the software reset control register and can be cleared by the hardware reset.
			[0] LCDIFRST bit
			Software reset of LCD I/F unit
			[1] LCDCRST bit
			Software reset of LCDC unit
			[2] MARBRST bit
			Software reset of MARB codec unit
			[3] GERST bit
			Software reset GE unit
			[4] JPEGRST bit
			Software reset of JPEG unit
			[5] LBUFRST bit
			Software reset of LBUF unit
			[6] IPPRST bit
			Software reset of IPP unit.
			[7] SIFRST bit
			Software reset of SIF unit.
			[8] ISPRST bit
			Software reset of ISP unit.
			[15:9] Reserved
--------------------------------------------------------------------------------*/
void VIM_HIF_ResetSubModule(VIM_HIF_RESET wVal)
{
    if   (wVal!=0xffff)
    {
		VIM_HIF_SetReg8(V5_REG_CPM_RSTCTRL1_L, (UINT8)(wVal&0xff));
		VIM_HIF_SetReg8(V5_REG_CPM_RSTCTRL1_H, (UINT8)((wVal&0xff00)>>8));
	}
	else 
	{
		VIM_HIF_SetReg8(V5_REG_CPM_RSTCTRL2, (UINT8)(0x80));     
	}
}

/*-----------------------------------------------------------------------------
  Summary:
	Control clock On
  Parameters:
	wVal : Set which Modul On£¬enum VIM_HIF_CLK_CTRL
  Note: Ä¬ÈÏ 1:OFF 0:ON
  Returns: void
  Remarks: 
--------------------------------------------------------------------------------*/
void VIM_HIF_SetModClkOn(UINT16 wVal)
{
	UINT16 wClockSetting = 0;
	wClockSetting = VIM_HIF_GetReg16(V5_REG_CPM_CLKOFF_L);//VIM_HIF_GetModClkVal();
	wClockSetting &= ~wVal;
	VIM_HIF_SetReg8(V5_REG_CPM_CLKOFF_L, (UINT8)(wClockSetting));
	VIM_HIF_SetReg8(V5_REG_CPM_CLKOFF_H, (UINT8)(wClockSetting>>8));
}

/*-----------------------------------------------------------------------------
  Summary:
	Control clock Off	
  Parameters:
	wVal : Set which Modul Off£¬enum VIM_HIF_CLK_CTRL
  Note: Ä¬ÈÏ 1:OFF 0:ON
  Returns: void
  Remarks: 
--------------------------------------------------------------------------------*/

void VIM_HIF_SetModClkClose(UINT16 wVal)
{
	UINT16 wClockSetting = 0;
	wClockSetting = VIM_HIF_GetReg16(V5_REG_CPM_CLKOFF_L);//VIM_HIF_GetModClkVal();

	wClockSetting |= wVal;
	VIM_HIF_SetReg8(V5_REG_CPM_CLKOFF_L, (UINT8)(wClockSetting));
	VIM_HIF_SetReg8(V5_REG_CPM_CLKOFF_H, (UINT8)(wClockSetting>>8));
}

/*-----------------------------------------------------------------------------
  Summary:
	528 ldo mode 
  Parameters:
  Returns: void
  Remarks: 
--------------------------------------------------------------------------------*/
void VIM_HIF_SetLdoStatus(VIM_LDOMODE wVal)
{
	VIM_HIF_SetReg8(V5_REG_BIU_STDBY_PR_SEL, 1);//from register
	if(wVal==VIM_LDO_ON)
	{
		VIM_HIF_SetReg8(V5_REG_BIU_STDBY_PR, 0);
#ifndef CONFIG_MACH_CANOPUS
		VIM_USER_DelayMs(10); 
		VIM_USER_DelayMs(10);
#else
		VIM_USER_DelayMs(1);
		VIM_USER_DelayMs(1);
#endif
	}
	else
	{
#ifndef CONFIG_MACH_CANOPUS
		VIM_HIF_SetReg8(V5_REG_BIU_STDBY, 1);
		VIM_USER_DelayMs(10); 
		VIM_HIF_SetReg8(V5_REG_BIU_STDBY_PR, 1);
		VIM_USER_DelayMs(10); 
#else
		VIM_HIF_SetReg8(V5_REG_BIU_STDBY, 1);
		VIM_USER_DelayMs(1);
		VIM_HIF_SetReg8(V5_REG_BIU_STDBY_PR, 1);
		VIM_USER_DelayMs(1);
#endif
	}
}
/***************************************************************
Description:
		init extern pin

Parameters:
		byPin: witch pin to in or out
		byInorOut:the direction of pin

Returns:
		void
****************************************************************/
void VIM_HIF_SetExterPinCrlEn(VIM_HIF_EXTPIN_CTRL byPin,BOOL Enable)
{
UINT16 temp = 0,temp1 = 0;
	temp=VIM_HIF_GetReg16(V5_REG_BIU_EXTPIN_CONTRAL);
	temp1=VIM_HIF_GetReg16(V5_REG_BIU_EXTPIN_DIRECT);
	if(Enable==ENABLE)
	{
		temp|=byPin;
		temp1&=(~byPin);//output
	}
	else
	{
		temp&=(~byPin);
		temp1|=byPin;
	}
	VIM_HIF_SetReg16(V5_REG_BIU_EXTPIN_CONTRAL, temp);
	VIM_HIF_SetReg16(V5_REG_BIU_EXTPIN_DIRECT, temp1);
}
/***************************************************************
Description:
		init extern pin value

Parameters:
		byPin: witch pin to in or out
		byInorOut:the direction of pin

Returns:
		void
****************************************************************/
void VIM_HIF_SetExterPinValue(VIM_HIF_EXTPIN_CTRL byPin,BOOL Value)
{
UINT16 temp = 0;
	temp=VIM_HIF_GetReg16(V5_REG_BIU_EXTPIN_VALUE);
	if(Value==1)
		temp|=byPin;
	else
		temp&=(~byPin);

	VIM_HIF_SetReg16(V5_REG_BIU_EXTPIN_VALUE, temp);
}
/*-----------------------------------------------------------------------------
  Summary:
	Initialize Bypass Type and Address of bypass register
  Parameters:
	 byAddr : Register address£¬UINT8, V5_REG_BIU_BASE + byBypassRegAdr
	 byPassType : Set Bypass or Normal Type,V5_REG_BIU_BYPASS_FROM_SEL
  Note: this method only call when Initialize process
  Returns: 
  Remarks:
--------------------------------------------------------------------------------*/
void VIM_HIF_InitBypass(UINT8 byAddr,VIM_BYPASSTYPE byType)
{
	if(byType==VIM_BYPASS_USE_GPIO)
		VIM_SET_NORMAL_BY_GPIO();
	// Set bypass type
   	 VIM_HIF_SetReg8(V5_REG_BIU_BYPASS_FROM_SEL, byType);
	// Set Address of bypass register
	VIM_HIF_SetReg8(V5_REG_BIU_BYPASS_ADDR_L, byAddr);
	VIM_HIF_SetReg8(V5_REG_BIU_BYPASS_ADDR_M, 0x18);
	VIM_HIF_SetReg8(V5_REG_BIU_BYPASS_ADDR_H, 0x0);
}

/*-----------------------------------------------------------------------------
  Summary:
	Set bypass or normal style
  Parameters:
	byOrNormal : enum VIM_HIF_BiuBypassType(NormalType = 0x0,ByPassType = 0x1)
  Note: 
  Returns:
  Remarks:
--------------------------------------------------------------------------------*/
void VIM_HIF_SetBypassOrNormal(PVIM_BYPASS_DEF pBypassInfo,VIM_HIF_BYPASSMODE byOrNormal)
{
UINT16 wValue=0x1800;
  	if(pBypassInfo->BypassType==VIM_BYPASS_USE_REG)
  	{
  		wValue|=pBypassInfo->BypassAddress;//angela 2006-3-9 for big panel can not read //VIM_HIF_GetReg8(V5_REG_BIU_BYPASS_ADDR_L);
		if(VIM_BUSTYPE==VIM_MULTI16)
			VIM_HIF_SetReg16(wValue, byOrNormal);	  
		else
			VIM_HIF_SetReg8(wValue, byOrNormal);		//angela 2007-2-28  
  	}
  	else
  	{	
  		if(byOrNormal==VIM_HIF_NORMALTYPE)
  			VIM_SET_NORMAL_BY_GPIO();
  		else
  			VIM_SET_BYPASS_BY_GPIO();
  	}
}

/*-----------------------------------------------------------------------------
  Summary:
	Set Delay Timing
  Parameters:
	VIM_TIMING
	UINT8 AddrDelay;
	UINT8 DataOutDelay;
	UINT8 DataInDelay;
	UINT8 DataRdDelay;
	UINT8 CsDelay;
  Note: 
  Returns:
  Remarks:
--------------------------------------------------------------------------------*/
void VIM_HIF_SetDelay(PVIM_TIMING Timing)
{
	VIM_HIF_SetReg8(V5_REG_BIU_ADDR_DELAY_SEL, Timing->AddrDelay);
	VIM_HIF_SetReg8(V5_REG_BIU_DATA_OUT_DELAY_SEL, Timing->DataOutDelay);
	VIM_HIF_SetReg8(V5_REG_BIU_DATA_IN_DELAY_SEL, Timing->DataInDelay);
	VIM_HIF_SetReg8(V5_REG_BIU_DATA_OEN_SEL, Timing->DataRdDelay);
	VIM_HIF_SetReg8(V5_REG_BIU_CS_DELAY_SEL, Timing->CsDelay);
	VIM_HIF_SetReg8(V5_REG_BIU_WR_THRU_DELAY_SEL, Timing->WrDelay);
}
/*-----------------------------------------------------------------------------
  Summary:
	Set PLL clk
  Parameters:
	PVIM_CLK
		UINT32	Clkin;
		UINT16 M;
		UINT8 N;
		UINT8 NO;
		UINT8 DIV;
  Note: 
  	out= (in*m/n)/no/div/2
  Returns:
  Remarks:
--------------------------------------------------------------------------------*/
VIM_RESULT VIM_HIF_InitPll(PVIM_CLK ClkInfo)
{
UINT32 dwValue,dwValue1,dwOutValue;
#if 1
	//if((ClkInfo->Clkin<1000)||(ClkInfo->Clkin>25000))		//angela 2006-12-15
		//return VIM_ERROR_PLL_INCLK;
	dwValue=ClkInfo->Clkin/ClkInfo->N;
	if((dwValue<1000)||(dwValue>25000)||(ClkInfo->N<2)||(ClkInfo->N>16))
		return VIM_ERROR_PLL_N;
	if(ClkInfo->M<2)
		return VIM_ERROR_PLL_M;
	if((ClkInfo->NO>8)||(ClkInfo->NO%2))
		return VIM_ERROR_PLL_NO;
	dwOutValue=ClkInfo->Clkin*((UINT32)ClkInfo->M)/((UINT32)ClkInfo->N);
	if((dwOutValue<200000)||(dwOutValue>1000000))
		return VIM_ERROR_PLL_OUT;
	dwOutValue=dwOutValue/ClkInfo->NO/ClkInfo->DIV/2;
	if((dwOutValue<(ClkInfo->Clkout-1000))||(dwOutValue>(ClkInfo->Clkout+1000)))
		return VIM_ERROR_PLL_OUT;

	VIM_HIF_SetReg8(V5_REG_BIU_PLL_RESET,1);	//reset pll high active
	VIM_USER_DelayMs(1);
	VIM_HIF_SetReg8(V5_REG_BIU_PLL_RESET,0);
	
	//ClkInfo->Clkout=dwOutValue;
	//guoying 1/23/2008, M value [7:0], total 8bit
	VIM_HIF_SetReg8(V5_REG_BIU_PLL_M,ClkInfo->M); //m
	
	VIM_HIF_SetReg8(V5_REG_BIU_PLL_N,ClkInfo->N); //n

	dwValue=0;
	dwValue1=ClkInfo->NO;
	while(dwValue1!=1)
	{
		dwValue1/=2;
		dwValue++;
	}
	VIM_HIF_SetReg8(V5_REG_BIU_PLL_OD,dwValue);  //od
	VIM_HIF_SetReg8(V5_REG_BIU_PLL_CLKCTRL,ClkInfo->DIV);  //od

	dwOutValue*=1000;//hz
	dwOutValue>>=15; // mclk/32767/
	if(dwOutValue)
		dwOutValue--;
	else
		return VIM_ERROR_PLL_MARB;
	dwOutValue>>=6; 
	//dwOutValue=0;

	VIM_HIF_SetReg8(V5_REG_MARB_CLK32_DIV_0, (UINT8)(dwOutValue>>8));
	VIM_HIF_SetReg8(V5_REG_MARB_CLK32_DIV_1, (UINT8)(dwOutValue));
#else
	VIM_HIF_SetReg8(V5_REG_BIU_PLL_M,4); //m
	VIM_HIF_SetReg8(V5_REG_BIU_PLL_N,2); //n
	VIM_HIF_SetReg8(V5_REG_BIU_PLL_OD,1);
	VIM_HIF_SetReg8(V5_REG_BIU_PLL_CLKCTRL,2);
	VIM_HIF_SetReg8(V5_REG_MARB_CLK32_DIV_0, 0);
	VIM_HIF_SetReg8(V5_REG_MARB_CLK32_DIV_1, 0);
	
#endif
	return VIM_SUCCEED;
}
VIM_RESULT VIM_HIF_InitMarbRereshTime(PVIM_CLK ClkInfo)
{
UINT32 dwOutValue=ClkInfo->Clkout;
	dwOutValue*=1000;//hz
	dwOutValue>>=15; // mclk/32767/
	if(dwOutValue)
		dwOutValue--;
	else
		return VIM_ERROR_PLL_MARB;
	dwOutValue>>=3; 		//angela 1110 for up it to 256k
	//dwOutValue=0;

	VIM_HIF_SetReg8(V5_REG_MARB_CLK32_DIV_0, (UINT8)(dwOutValue>>8));
	VIM_HIF_SetReg8(V5_REG_MARB_CLK32_DIV_1, (UINT8)(dwOutValue));
	return VIM_SUCCEED;
}
/*-----------------------------------------------------------------------------
  Summary:
	Set PLL clk
  Parameters:
	PVIM_CLK
		UINT32	Clkin;
		UINT16 M;
		UINT8 N;
		UINT8 NO;
		UINT8 DIV;
  Note: 
  	out= (in*m/n)/no/div/2
  Returns:
  Remarks:
--------------------------------------------------------------------------------*/
void VIM_HIF_SetPllStatus(VIM_HIF_PLLMODE Mode)
{
	switch(Mode)
	{
		case VIM_HIF_PLLPOWERON:
			VIM_HIF_SetReg8(V5_REG_BIU_PLL_OE,0);//disable output
			VIM_HIF_SetReg8(V5_REG_BIU_XCLK_DISABLE,1);//xclk disable
			VIM_USER_DelayMs(1); 
			VIM_HIF_SetReg8(V5_REG_BIU_PLL_BP,0);//bypass disable
			VIM_HIF_SetReg8(V5_REG_BIU_PLL_PD,0);//power down disable
			VIM_HIF_SetReg8(V5_REG_BIU_XCLK_DISABLE,0);//xclk enable
			VIM_USER_DelayMs(1); 
			VIM_HIF_SetReg8(V5_REG_BIU_PLL_OE,1);//enable out put
#ifndef CONFIG_MACH_CANOPUS
			VIM_USER_DelayMs(10);
#else
			VIM_USER_DelayMs(1);
#endif
			break;
		case VIM_HIF_PLLPOWERDOWN:
			VIM_HIF_SetReg8(V5_REG_BIU_PLL_OE,0);//disable output
			VIM_HIF_SetReg8(V5_REG_BIU_XCLK_DISABLE,1);//xclk disable
			VIM_USER_DelayMs(1); 
			VIM_HIF_SetReg8(V5_REG_BIU_PLL_BP,1);//bypass enable
			VIM_HIF_SetReg8(V5_REG_BIU_PLL_PD,1);//power down enable
#ifndef CONFIG_MACH_CANOPUS
			VIM_USER_DelayMs(10);
#else
			VIM_USER_DelayMs(1);
#endif
			break;
		case VIM_HIF_PLLBYPASS:
			VIM_HIF_SetReg8(V5_REG_BIU_PLL_OE,0);//disable output
			VIM_HIF_SetReg8(V5_REG_BIU_XCLK_DISABLE,1);//xclk disable
			VIM_USER_DelayMs(1); 
			VIM_HIF_SetReg8(V5_REG_BIU_PLL_BP,1);//bypass enable
			VIM_HIF_SetReg8(V5_REG_BIU_PLL_PD,0);//power down disable
			VIM_HIF_SetReg8(V5_REG_BIU_XCLK_DISABLE,0);//xclk enalbe
			VIM_USER_DelayMs(1); 
			VIM_HIF_SetReg8(V5_REG_BIU_PLL_OE,1);//enable output
#ifndef CONFIG_MACH_CANOPUS
			VIM_USER_DelayMs(10);
#else
			VIM_USER_DelayMs(1);
#endif
			break;
		default:
			break;
	}
}
/*-----------------------------------------------------------------------------
  Summary:
	Set Interrupt 
  Parameters:

  Note: 

  Returns:
  Remarks:
--------------------------------------------------------------------------------*/
VIM_RESULT VIM_HIF_InitInerruptLevel(PVIM_INTERRUPT Interrupt)
{
	UINT8 ctrl=0;
	if(Interrupt->LeverWidth>16)
		return VIM_ERROR_INTERRUPT_INIT;
	switch(Interrupt->Int_Type)
	{
		case VIM_INT_LEVER_LOW:
			ctrl = (1)|(1<<1)|(0<<2)|(Interrupt->LeverWidth<<3);
		// levertype 1 ; activelow(lever) 1; edgeonly 0, sigwidth leverwidth
		break;
		case VIM_INT_LEVER_HIGH:
			ctrl = (1)|(0<<1)|(0<<2)|(Interrupt->LeverWidth<<3);
		// levertype 1 ; activehigh(lever)  0; edgeonly 0, sigwidth leverwidth
		break;
		case VIM_INT_ADGE_DOWN:
			ctrl = (0)|(0<<1)|(1<<2)|(Interrupt->LeverWidth<<3);
		// triggerType 0 ; activehigh(lever only) 0; edgeActiveDown 1, sigwidth leverwidth
		break;
		case VIM_INT_ADGE_UP:
			ctrl = (0)|(0<<1)|(0<<2)|(Interrupt->LeverWidth<<3);
		// triggerType 0 ; activehigh(lever only) 0; edgeActiveUp 0, sigwidth leverwidth
		break;
		default:
		break;
		}
	VIM_HIF_SetReg8(V5_REG_CPM_INTCTRL,ctrl);
	return VIM_SUCCEED;
}
/*-----------------------------------------------------------------------------
  Summary:
	set gpio value
  Parameters:

  Note: 

  Returns:
  Remarks:
--------------------------------------------------------------------------------*/
void VIM_HIF_SetGpioInfo(VIM_HIF_DIRMODE Mode,UINT8 Value)
{
	VIM_HIF_SetReg8(V5_REG_CPM_GPIO_MODE,0);
	if(Mode==VIM_PUTOUT)
		VIM_HIF_SetReg8(V5_REG_CPM_GPIO_DIR,0xff);
	else 
		VIM_HIF_SetReg8(V5_REG_CPM_GPIO_DIR,0);
	VIM_HIF_SetReg8(V5_REG_CPM_GPIO_P0,Value);
}
/*-----------------------------------------------------------------------------
  Summary:
	set panel cs mode
  Parameters:
	BOOL Enable;
		ENABLE:
			when cs is not active ,the data bus with lcd panel is output 0
		DISABLE
			the function is disable
  Note: 

  Returns:
  Remarks:
--------------------------------------------------------------------------------*/
void VIM_HIF_SetLcdCsActiveEn(BOOL Enable)
{
UINT8 bValue;
	bValue=VIM_HIF_GetReg8(V5_REG_BIU_LCDIF_CONFIG);
	bValue&=(~BIT2);
	bValue|=(Enable<<2);
	VIM_HIF_SetReg8(V5_REG_BIU_LCDIF_CONFIG,bValue);
}
/*-----------------------------------------------------------------------------
  Summary:
	set panel one cs use two panel
  Parameters:
	VIM_BYPASS_SUB_CS1:
		two cs use one cs input
	VIM_BYPASS_SUB_CS2:
		two cs use two cs input
  Note: 

  Returns:
  Remarks:
--------------------------------------------------------------------------------*/
void VIM_HIF_SetLcdCsMode(VIM_BYPASS_SUBPANELCTL Cs)
{
UINT8 bValue;
	bValue=VIM_HIF_GetReg8(V5_REG_BIU_LCDIF_CONFIG);
	bValue&=(~BIT1);
	bValue|=(Cs<<1);
	VIM_HIF_SetReg8(V5_REG_BIU_LCDIF_CONFIG,bValue);
}
/*-----------------------------------------------------------------------------
  Summary:
	set panel one cs use two panel
  Parameters:
	VIM_BYPASS_SUB_CS1:
		two cs use one cs input
	VIM_BYPASS_SUB_CS2:
		two cs use two cs input
  Note: 

  Returns:
  Remarks:
--------------------------------------------------------------------------------*/
void VIM_HIF_SwitchPanelCs(UINT8 Panel)
{
UINT8 bValue;
	bValue=VIM_HIF_GetReg8(V5_REG_BIU_LCDIF_CONFIG);
	bValue&=(~BIT0);
	bValue|=(Panel);
	VIM_HIF_SetReg8(V5_REG_BIU_LCDIF_CONFIG,bValue);
}

/********************************************************************************
  Description:
  	disable interrupt by module
  Parameters:
  Returns:
  	void
  Remarks:
*********************************************************************************/

void VIM_HIF_SetIntModuleEn(VIM_HIF_INT_TYPE byIntEnEnum,BOOL Enable)
{
UINT8 bTemp;
		bTemp = VIM_HIF_GetReg8(V5_REG_CPM_INTEN);
		if(Enable==DISABLE)
		{
			if(byIntEnEnum==INT_ALL)
				bTemp=0;
			else
				bTemp &=(~ (0x1<<byIntEnEnum));
		}
		else
		{
			if(byIntEnEnum==INT_ALL)
				bTemp=0xff;
			else
				bTemp |=(0x1<<byIntEnEnum);		
		}
		VIM_HIF_SetReg8(V5_REG_CPM_INTEN, bTemp);
}
/***************************************************************/
/*V5_REG_CPM_INTSERV	 Default value 00
			This register is the interrupt serve flag, when corresponding bit is ¡°1¡±, means the interrupt is
			under serving, ¡°0¡± means interrupt service over.
			[0] sif interrupt souce service
			[1] isp interrupt souce service
			[2] lbuf interrupt souce service
			[3] jpeg interrupt souce service
			[4] ge interrupt souce service
			[5] marb interrupt souce service
			[6] lcdc interrupt souce service
			*/
/***************************************************************/
 void VIM_HIF_SetIntServ(UINT8 byVal)               //see V5B_CpmInt_ServeFenum
{
	VIM_HIF_SetReg8(V5_REG_CPM_INTSERV, byVal);
}
/********************************************************************************
  Description:
  	get the second lever interrupt flag 
  Parameters:
  Returns:
  	void
  Remarks:
*********************************************************************************/

UINT8 VIM_HIF_GetIntFlagSec(VIM_HIF_INT_TYPE byOffset)        //¶þ¼¶  ÖÐ¶Ï±êÖ¾
{
	UINT8 x;
	x = VIM_HIF_GetReg8(V5_REG_CPM_INTFLAG0+(byOffset<<1));

  return  (x );
}
/********************************************************************************
  Description:
  	clear interrupt by module
  Parameters:
  Returns:
  	void
  Remarks:
*********************************************************************************/

void VIM_HIF_ClearIntModule(VIM_HIF_INT_TYPE byIntEnEnum)
{
	UINT8  i = 0;
	VIM_HIF_SetIntModuleEn(byIntEnEnum,DISABLE);
	if(byIntEnEnum<INT_ALL)
	{
		VIM_HIF_SetIntEnableSec(byIntEnEnum,DISABLE);	//angela 2006-12-13
		gVc0528_Isr.bSecondLevelIntFlag[byIntEnEnum]=0;
		VIM_HIF_SetIntServ(0x1<<byIntEnEnum);		
		VIM_HIF_GetIntFlagSec((VIM_HIF_INT_TYPE)byIntEnEnum);
		VIM_HIF_SetIntServ(0); 
	}
	else
	{
		VIM_USER_MemSet((UINT8 *)&gVc0528_Isr,0,sizeof(VIM_HIF_TISR));// ??addd?
		 for(i=0; i<VIM_FIRST_LEVEL_INT_NUM; i++)
		{
			VIM_HIF_SetIntEnableSec((VIM_HIF_INT_TYPE)i,DISABLE);	//angela 2006-12-13
	 		VIM_HIF_SetIntServ(0x1<<i);		
			VIM_HIF_GetIntFlagSec((VIM_HIF_INT_TYPE)i);
			VIM_HIF_SetIntServ(0); 
		}
	}
}





/********************************************************************************
  Description:
  	get the second lever enable
  Parameters:
  Returns:
  	void
  Remarks:
*********************************************************************************/

UINT8 VIM_HIF_GetIntEnableSec(VIM_HIF_INT_TYPE byOffset)
{
	UINT8 x;

	x = (UINT8)VIM_HIF_GetReg8(V5_REG_CPM_INTEN0+(byOffset<<1));
    	return (UINT16)x & 0xff;
}
/********************************************************************************
  Description:
  	set the second lever enable
  Parameters:
  Returns:
  	void
  Remarks:
*********************************************************************************/

void VIM_HIF_SetIntEnableSec(VIM_HIF_INT_TYPE byOffset, UINT8 uVal)
{
	VIM_HIF_SetReg8(V5_REG_CPM_INTEN0+(byOffset<<1), uVal);
}


static void VIM_HIF_HandleISR(VIM_HIF_INT_TYPE byFirstLevelInt, UINT8 bSecondLevelIntFlag)
{
	switch (byFirstLevelInt)
	{
	case INT_MARB:
		_ISR_MarbIntHandle(bSecondLevelIntFlag);
		break;
	case INT_JPEG:
		_ISR_JpegJpegIntHandle(bSecondLevelIntFlag);
		break;
	case INT_LBUF:
		_ISR_JpegLbufIntHandle(bSecondLevelIntFlag);
		break;
	case INT_SIF:
		break;
	case INT_ISP:
		break;
	case INT_GE:
		break;
	case INT_LCDC:
		break;
	case INT_MARBADD:
#if(VIM_HIF_DEBUG)&&(VIM_528RDK_DEBUG)
	VIM_USER_PrintHex("\n INT_MARBADD flag= ",bSecondLevelIntFlag);
#endif
		_ISR_Marb1IntHandle(bSecondLevelIntFlag);
		break;
	default:
		break;
	}
}
/*Interrupt process*/
/***************************************************************
  Summary:
	INterrupt inr handle
  Parameters:
	void
  Note: 
  	
  Returns:
  Remarks:
			Default value ff
			Interrupt enable register. Enable interrupt request to first level flag, corresponding bit is ¡°1¡±,
			¡°0¡± the request is masked.
			[0] sif  interrupt enable
			[1] isp  interrupt enable
			[2] lbuf interrupt enable
			[3] jpeg interrupt enable
			[4] ge 	 interrupt enable
			[5] marb interrupt enable
			[6] lcdc interrupt enable			
**************************************************************/
void _ISR_HIF_IntHandle(void)
{
	UINT8  i, intEn, intFlg,x;
	UINT8 byIntNum = 0;
	//interrupt protect
	intEn = VIM_HIF_GetReg8(V5_REG_CPM_INTEN);
	intFlg = VIM_HIF_GetReg8(V5_REG_CPM_INTFLAG);
	if(0 == intEn || 0 == intFlg)
		return;
	byIntNum = 0;

	for(i=0; i<VIM_FIRST_LEVEL_INT_NUM; i++)
	{
		x = (UINT8)(0x1<<gFirstLevelIntPriority[i]);
		if((intEn&x) && (intFlg&x))
		{
			VIM_HIF_SetIntServ(x);
			gVc0528_Isr.byFirstLevelInt[byIntNum]=gFirstLevelIntPriority[i];
			gVc0528_Isr.bSecondLevelIntFlag[byIntNum] = VIM_HIF_GetIntFlagSec((VIM_HIF_INT_TYPE)gFirstLevelIntPriority[i]);	
			byIntNum++;
			VIM_HIF_SetIntServ(0); 
		}
	}

	for(i=0; i< byIntNum; i++)
	{
#if(VIM_HIF_DEBUG)&&(VIM_528RDK_DEBUG)
	VIM_USER_PrintHex("\n interrupt gVc0528_Isr.byFirstLevelInt[byIntNum]",gVc0528_Isr.byFirstLevelInt[i]);
	VIM_USER_PrintHex("\n interrupt gVc0528_Isr.bSecondLevelIntFlag[i]",gVc0528_Isr.bSecondLevelIntFlag[i]);
	VIM_USER_PrintHex("VIM_HIF_GetIntEnableSec=",VIM_HIF_GetIntEnableSec(INT_MARBADD));
#endif
		VIM_HIF_HandleISR((VIM_HIF_INT_TYPE)gVc0528_Isr.byFirstLevelInt[i],gVc0528_Isr.bSecondLevelIntFlag[i]);
	}
//interrupt protect
}

