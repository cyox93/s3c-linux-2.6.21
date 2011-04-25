/*                Copyright (C) 2005 Vimicro CO.,LTD     		 
*
* FILE NAME                                  VERSION                
*                                                                       
* VIM_SIF_Driver.c(only use for 528)	 0.2                   
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*     VC0578 's SIF + ISP moudle sub driver                                 
*
*	Version		Author			Date		Description		
*  ---------------------------------------------------------------						 
*   0.1			LiuHD			2005-11-16	The first version. 
     0.2			angela 			2006-06-08	updata for 528 delete isp
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
/********************************************************************************
* Description:
*	Init i2c
*	The pSensorInfo is gotton from sensor.c.
* Parameters:
*	pSensorInfo: The pointer to TSensorInfo structure.
	UINT32 MCLK: mclk for 578
* Return:
*	VIM_SUCCEED: init i2c VIM_SUCCEED
*	
* Note:
*	
**********************************************************************************/
VIM_RESULT VIM_SIF_SifInitI2c(const TSnrInfo *pSensorInfo,UINT32 dwMclk)
{
UINT8 bMode;
UINT16 div;
	bMode=VIM_HIF_GetReg8(V5_REG_SIF_SNROP_MODE);
	bMode&=(~BIT5);
	bMode&=(~BIT2);
	if(pSensorInfo->bustype==1)	//serial bus
	{
		bMode|=BIT5;
	}
	else
	{
		if(pSensorInfo->i2ctype==1)//ov
			bMode|=BIT2;
		div = (UINT16)(((dwMclk / (UINT32)pSensorInfo->i2crate) + 3) >> 2) << 2;
		if(div < 0x40)
			div = 0x40;
		VIM_HIF_SetReg8(V5_REG_SIF_I2C_CLKCNTH, (UINT8)(div>>8));
		VIM_HIF_SetReg8(V5_REG_SIF_I2C_CLKCNTL,  (UINT8)div);
		//set i2c address
		VIM_SIF_SetI2cDevAddr(pSensorInfo->i2caddress);	
	}
	VIM_HIF_SetReg8(V5_REG_SIF_SNROP_MODE,bMode);
	return VIM_SUCCEED;

}



/********************************************************************************
* Description:
*	Init SIF according to the pSensorInfo information.
*	The pSensorInfo is gotton from sensor.c.
* Parameters:
*	pSensorInfo: The pointer to TSensorInfo structure.
* Return:
*	TRUE: init SIF+ISP success.
*	FALSE: pSensorInfo is NULL.
* Note:
*	Init public SIF parameters, they are the same for different sensor resolution.
**********************************************************************************/
VIM_RESULT VIM_SIF_SifSensorInit(const TSnrInfo *pSensorInfo)
{
	UINT8 i = 0;
	UINT8 byMode = 0;
	if (NULL == pSensorInfo)
	{
		return VIM_ERROR_SENSOR_NOINFO;
	}
	byMode=VIM_HIF_GetReg8(V5_REG_SIF_SNR_MODE);
	byMode&=(~BIT7);
	byMode&=(~BIT6);
	byMode&=(~BIT5);

	#if 0
	byMode = ((VIM_SIF_SNSOR_CCIR656_SYNC & 0x1) <<7//Sensor output data sync mode:0--normal,1--CCIR656      BIT7     
			|((pSensorInfo->snrtype & 0x1) << 6) // Sensor type, YUV or BAYER
			| (0x1 << 5));	// Enable sync generate to sensor.
	
	#elif 1
	byMode = ((VIM_SIF_SNSOR_NORMAL_SYNC & 0x1) <<7//Sensor output data sync mode:0--normal,1--CCIR656      BIT7     
			|((pSensorInfo->snrtype & 0x1) << 6) // Sensor type, YUV or BAYER
			| (0x1 << 5));	// Enable sync generate to sensor.
	#elif 0
	//for normal signal
	byMode = ((VIM_SNR_NORMAL&0x80)//Sensor output data sync mode:0--normal,1--CCIR656      BIT7     
			|(pSensorInfo->snrtype) // Sensor type, YUV or BAYER
			| (0x1 << 5));	// Enable sync generate to sensor.
	#elif 0
	//for CCIR656 signal
	byMode = ((VIM_SNR_CCIR656&0x80)//Sensor output data sync mode:0--normal,1--CCIR656      BIT7     
			|(pSensorInfo->snrtype) // Sensor type, YUV or BAYER
			| (0x1 << 5));	// Enable sync generate to sensor.
	#endif
	VIM_HIF_SetReg8(V5_REG_SIF_SNR_MODE, byMode);
	VIM_USER_DelayMs(1);

	// set 0x085c edge chos
	byMode=0;//VIM_HIF_GetReg8(V5_REG_SIF_EDGE_CHOS);
	//byMode=VIM_HIF_GetReg8(V5_REG_SIF_EDGE_CHOS);//angela 0314
 	if(pSensorInfo->pclk)
	{
		byMode|=BIT1;	//use clock
		byMode|=((pSensorInfo->clkcfg&7)<<2);
	}
	VIM_HIF_SetReg8(V5_REG_SIF_EDGE_CHOS, byMode);

	
	// Set sif register according to the table.
	for(i = 0; i < pSensorInfo->dsif.len ; i++)
	{
		VIM_HIF_SetReg8((UINT32)(pSensorInfo->dsif.regval[i].adr | V5_REG_SIF_BASE), (UINT8)pSensorInfo->dsif.regval[i].val);
		VIM_USER_DelayMs(pSensorInfo->dsif.regval[i].wait);
	}

	//guoying 2/28/2008 modified
	if(VIM_SNR_NORMAL == pSensorInfo->snrtype||VIM_SNR_CCIR656 == pSensorInfo->snrtype||VIM_SNR_YUV == pSensorInfo->snrtype)		//YUV
	{
		return VIM_SUCCEED;
	}		//guoying 3/6/2008
	else// RGB Type
	{
		return VIM_ERROR_SENSOR_RGB;
	}
}


/********************************************************************************
* Description:
*	Adjust SIF's setting when adjust sensor's resolution.
*	
* Parameters:
*	pSensorInfo: The pointer to TSnrInfo structure.
*	byResolution: Sensor Resolution: VGA or SXGA or QXGA....			
* Return:
*	TRUE: 	Set SIF success.
*	FALSE:	 pSensorInfo is NULL 
* Note:
*	
**********************************************************************************/
VIM_RESULT VIM_SIF_SetSensorResolution(const TSnrInfo *pSensorInfo, VIM_SIF_RESOLUTION byResolution)
{
	//UINT8 uReg;
	TSize SensorOutputSize; 
	UINT16 wVStart;
	int  i = 0;

	if (NULL == pSensorInfo)
	{
		return VIM_ERROR_SENSOR_NOINFO;
	}
	if((byResolution>=pSensorInfo->snrSizeCfg.len))
		return VIM_ERROR_SENSOR_RESOLUTION;	

	
	/* set sif */
	SensorOutputSize = pSensorInfo->snrSizeCfg.pSizeCfg[byResolution].size;
	
	/*SIF module will output Href sync signal to ISP and IPP module*/
	VIM_HIF_SetReg8(V5_REG_SIF_HREF_LENGTHH, (UINT8)((SensorOutputSize.cx - 1)>>8));
	VIM_HIF_SetReg8(V5_REG_SIF_HREF_LENGTHH+1, (UINT8)(SensorOutputSize.cx - 1));

	/*get SIF module  Vref sync signal to ISP and IPP module*/
	wVStart = ((UINT8)VIM_HIF_GetReg8(V5_REG_SIF_VREF_STARTH))<<8;
	wVStart+=VIM_HIF_GetReg8(V5_REG_SIF_VREF_STARTH+1);
	

	/*set SIF module will output Vref sync signal to ISP and IPP module*/
	//VIM_HIF_SetReg16(V5_REG_SIF_VREF_STOPH, SensorOutputSize.cy + wVStart);
	VIM_HIF_SetReg8(V5_REG_SIF_VREF_STOPH, (UINT8)((SensorOutputSize.cy + wVStart)>>8));
	VIM_HIF_SetReg8(V5_REG_SIF_VREF_STOPH+1, (UINT8)(SensorOutputSize.cy + wVStart));

	/*config sensor's mclk ratio sensor's clk = SIF's clk / (sel+1)*/
	VIM_HIF_SetReg8(V5_REG_SIF_SNRCLK_CNT, pSensorInfo->snrSizeCfg.pSizeCfg[byResolution].snrclkratio);

	/*config sensor's output pixel clk ratio  pixel's clk = SIF's clk / (sel+1)*/
	VIM_HIF_SetReg8(V5_REG_SIF_PIXRATE_SEL, pSensorInfo->snrSizeCfg.pSizeCfg[byResolution].snrpixelratio);

	//isp  init
	for(i = 0; i < pSensorInfo->snrSizeCfg.pSizeCfg[byResolution].visp.len; i++)
	{
		VIM_HIF_SetReg8((UINT32)(pSensorInfo->snrSizeCfg.pSizeCfg[byResolution].visp.regval[i].adr), 
			(UINT8)pSensorInfo->snrSizeCfg.pSizeCfg[byResolution].visp.regval[i].val);
		VIM_USER_DelayMs(pSensorInfo->snrSizeCfg.pSizeCfg[byResolution].visp.regval[i].wait);
	}


	if(pSensorInfo->snrrst)
   		VIM_SIF_SensorReset();
	for(i = 0; i < pSensorInfo->snrSizeCfg.pSizeCfg[byResolution].snr.len ; i++)
	{
		pSensorInfo->snrSetRegCall((UINT16)(pSensorInfo->snrSizeCfg.pSizeCfg[byResolution].snr.regval[i].adr), 
			(UINT16)pSensorInfo->snrSizeCfg.pSizeCfg[byResolution].snr.regval[i].val);
		VIM_USER_DelayMs(pSensorInfo->snrSizeCfg.pSizeCfg[byResolution].snr.regval[i].wait);
	}

	
	return VIM_SUCCEED;
}


VIM_RESULT VIM_SIF_GetSensorResolution(const TSnrInfo *pSensorInfo, VIM_SIF_RESOLUTION bySensorResolution, TSize *InputSize)
{
	if (NULL == pSensorInfo)
	{
		return VIM_ERROR_SENSOR_NOINFO;
	}
	
	if ((bySensorResolution > VIM_SIF_HALFPIXEL))
	{
		return VIM_ERROR_SENSOR_RESOLUTION;
	}

	if (NULL == InputSize)
	{
		return VIM_ERROR_SENSOR_INPUTSIZE;
	}

	*InputSize = pSensorInfo->snrSizeCfg.pSizeCfg[bySensorResolution].size;

	return VIM_SUCCEED;
}

/*void VIM_SIF_SetTailBlank(UINT32 x)
{
	VIM_HIF_SetReg8(V5_REG_ISP_TAIL_BLANK_L, (UINT8)(x & 0xff));
	VIM_HIF_SetReg8(V5_REG_ISP_TAIL_BLANK_M, (UINT8)((x >> 8) & 0xffff));
}*/
/********************************************************************************
* Description:
*	change clock
*	
* Parameters:
*	const TSnrInfo *pSensorInf: 
		The pointer to TSnrInfo  structure.
*				
* Return:
*	TRUE: 	Set SIF success.
*	FALSE:	 pSensorInfo is NULL 
* Note:
*	
**********************************************************************************/
VIM_RESULT VIM_SIF_SetSensorClk(const TSnrInfo *pSensorInfo,VIM_SIF_CLKTYPE Type)
{
	UINT8 byResolution,bCratio,bPratio,div=1;//default rgb;
	UINT16 wWidth,wHeigth;
			
	VIM_IPP_GetImageSize(&wWidth,&wHeigth);
	for(byResolution=0;byResolution<pSensorInfo->snrSizeCfg.len;byResolution++)
	{
		if(pSensorInfo->snrSizeCfg.pSizeCfg[byResolution].size.cx==wWidth)
			break;
	}
	if(byResolution==pSensorInfo->snrSizeCfg.len)
		return VIM_ERROR_SENSOR_NOROSELUTION;
	bCratio=pSensorInfo->snrSizeCfg.pSizeCfg[byResolution].snrclkratio;
	bPratio=pSensorInfo->snrSizeCfg.pSizeCfg[byResolution].snrpixelratio;
	if(pSensorInfo->snrtype)	//yuv type
		div=2;
	div*=Type;
	bCratio = ((bCratio + 1) << div) - 1;
	bPratio = ((bPratio + 1) << div) - 1;	

	if(VIM_HIF_GetReg8(V5_REG_SIF_SNRCLK_CNT)!=bCratio)
		VIM_HIF_SetReg8(V5_REG_SIF_SNRCLK_CNT, bCratio);
	if(VIM_HIF_GetReg8(V5_REG_SIF_PIXRATE_SEL)!=bPratio)
		VIM_HIF_SetReg8(V5_REG_SIF_PIXRATE_SEL, bPratio);

	return VIM_SUCCEED;
}
/********************************************************************************
* Description:
*	 SIF config  sequence  when sensor is power on
*	
* Parameters:
*	const TSnrInfo *pSensorInf: 
		The pointer to TSnrInfo  structure.
*				
* Return:
*	TRUE: 	Set SIF success.
*	FALSE:	 pSensorInfo is NULL 
* Note:
*	
**********************************************************************************/
VIM_RESULT VIM_SIF_SetSensorPowerOn(const TSnrInfo *pSensorInfo)
{
	//UINT8 byMode;
	int  i = 0;
	UINT32 temp;
	if (NULL == pSensorInfo)
	{
		return VIM_ERROR_SENSOR_NOINFO;
	}
	for(i = 0; i < pSensorInfo->sifpwronseq.len;i++)
	{
		#if 0
		VIM_HIF_SetReg8((UINT32)(pSensorInfo->sifpwronseq.regval[i].adr | V5_REG_SIF_BASE), (UINT8)pSensorInfo->sifpwronseq.regval[i].val);
		//guoying 2/27/2008
		#else
		temp=VIM_HIF_GetReg8((UINT32)(pSensorInfo->sifpwronseq.regval[i].adr | V5_REG_SIF_BASE));
		temp&=0xfc;	
		temp|=((UINT8)pSensorInfo->sifpwronseq.regval[i].val)&0x03|0x60;			//bit5=1 Sync_en=1
		
		VIM_HIF_SetReg8((UINT32)(pSensorInfo->sifpwronseq.regval[i].adr | V5_REG_SIF_BASE), temp);
		#endif	//guoying 3/6/2008
		
		VIM_USER_DelayMs(pSensorInfo->sifpwronseq.regval[i].wait);
	}
	return VIM_SUCCEED;
}



/********************************************************************************
* Description:
*	 SIF config  sequence  when sensor is standby
*	
* Parameters:
*	const TSnrInfo *pSensorInfo *: 
		The pointer to TSensorInfo  structure.
*				
* Return:
*	TRUE: 	Set SIF success.
*	FALSE:	 pSensorInfo is NULL or byResolution is not (VGA or SXGA  QXGA...).
* Note:
*	
**********************************************************************************/
static VIM_RESULT VIM_SIF_SetSensorStandby(const TSnrInfo *pSensorInfo)
{
	int  i = 0;
	if (NULL == pSensorInfo)
	{
		return VIM_ERROR_SENSOR_NOINFO;
	}
	for(i = 0; i < pSensorInfo->sifstdbyseq.len ; i++)
	{
		VIM_HIF_SetReg8((UINT32)(pSensorInfo->sifstdbyseq.regval[i].adr | V5_REG_SIF_BASE), (UINT8)pSensorInfo->sifstdbyseq.regval[i].val);
		VIM_USER_DelayMs(pSensorInfo->sifstdbyseq.regval[i].wait);
	}
	return VIM_SUCCEED;
}
/********************************************************************************
* Description:
*	Set sensor state (power on, power off, standby and sleep)
* Parameters:
*	pSensorInfo: 	The pointer to TSensorInfo structure.
*	bySensorState:
*		SENSOR_POWERON,
*		SENSOR_POWEROFF,
*		SENSOR_STANDBY,
*		SENSOR_SLEEP
*	byResolution: Sensor Resolution: VGA or SXGA, used only on 	SENSOR_POWERON state.
* Return:
*	TRUE:	Set sensor state success
*	FALSE:	 pSensorInfo is NULL or bySensorState is unknown or byResolution is not VGA or SXGA
* Note:
*	
**********************************************************************************/
VIM_RESULT VIM_SIF_SetSensorState(const TSnrInfo *pSensorInfo, VIM_SIF_SENSORSTATE bySensorState)
{
	int i = 0;	
	TRegGroup RegGroup;

	if (NULL == pSensorInfo)
	{
		return VIM_ERROR_SENSOR_NOINFO;
	}
	
	if (bySensorState == VIM_SIF_SENSOR_STANDBY)
	{
		return VIM_ERROR_SENSOR_STATE;
	}

		
	switch(bySensorState)
	{
		case VIM_SIF_SENSOR_POWERON:
			VIM_SIF_SetSensorPowerOn(pSensorInfo);

			if (pSensorInfo->snrrst) // When switch resolution, whether need to reset sensor.
			{
				VIM_SIF_SensorReset();
			}
			RegGroup = pSensorInfo->Initpoweron;//pSensorInfo->snrSizeCfg.pSizeCfg[byResolution].snr;
			break;
		case VIM_SIF_SENSOR_STANDBY:
			RegGroup = pSensorInfo->standby;
			break;
		/*case VIM_SIF_SENSOR_SLEEP:
			RegGroup = pSensorInfo->sleep;
			break;
		case VIM_SIF_SENSOR_POWEROFF:
			RegGroup = pSensorInfo->poweroff;
			break;*/
		default:
			break;
	}

	for(i = 0; i < RegGroup.len; i++)
	{
		pSensorInfo->snrSetRegCall((UINT16)RegGroup.regval[i].adr, (UINT16)RegGroup.regval[i].val);
		VIM_USER_DelayMs(RegGroup.regval[i].wait);	//guoying 3/24/2008 testing
	}

	//if ((VIM_SIF_SENSOR_POWEROFF == bySensorState)|(VIM_SIF_SENSOR_STANDBY == bySensorState)|
		//(VIM_SIF_SENSOR_SLEEP == bySensorState))
	if (VIM_SIF_SENSOR_STANDBY == bySensorState)
	{
		VIM_SIF_SetSensorStandby(pSensorInfo);
	}

	return VIM_SUCCEED;
}


/********************************************************************************
* Description:
*	Reset sensor 
* Parameters:
*	None
* Return:
*	None
* Note:
*	Sensor reset when V5_REG_SIF_SNR_MODE bit0 changes from 1 to 0 or 0 to 1.
**********************************************************************************/
void VIM_SIF_SensorReset(void)
{
	UINT8 byMode;
	
	byMode = VIM_HIF_GetReg8(V5_REG_SIF_SNR_MODE);
	if(byMode & BIT0)
	{
		VIM_HIF_SetReg8(V5_REG_SIF_SNR_MODE, byMode & (~BIT0));
	}
	else
	{
		VIM_HIF_SetReg8(V5_REG_SIF_SNR_MODE, byMode | BIT0);
	}

#ifndef CONFIG_MACH_CANOPUS
	VIM_USER_DelayMs(50);
	VIM_HIF_SetReg8(V5_REG_SIF_SNR_MODE, byMode);
	VIM_USER_DelayMs(50);
#else
	VIM_USER_DelayMs(5);
	VIM_HIF_SetReg8(V5_REG_SIF_SNR_MODE, byMode);
	VIM_USER_DelayMs(1);
#endif
}



/********************************************************************************
* Description:
*	 Enable/Disable sync signal to ISP and IPP.
* Parameters:
*	bEnable: 
*		TRUE: Enable
*		FALSE: Disable
* Return:
*	None
* Note:
*	1. Set V5_REG_SIF_SNROP_MODE bit6
*	2. When disable sync generate, ISP and IPP will not work.
**********************************************************************************/
void VIM_SIF_EnableSyncGen(UINT8 bEnable)
{
	UINT8 byMode;
	
	byMode = VIM_HIF_GetReg8(V5_REG_SIF_SNROP_MODE);
	if(bEnable)
	{
		byMode |= BIT6;
	}
	else
	{
		byMode &= ~BIT6;
	}

	VIM_HIF_SetReg8(V5_REG_SIF_SNROP_MODE, byMode);
}


void VIM_SIF_StartCaptureEnable(UINT8 bEnable)
{
	UINT8 byMode;
	
	byMode = VIM_HIF_GetReg8(V5_REG_SIF_SNROP_MODE);
	if(bEnable)
	{
		byMode |= BIT6|BIT7;
	}
	else
	{
		byMode &=~BIT7;
	}

	VIM_HIF_SetReg8(V5_REG_SIF_SNROP_MODE, byMode);

}






/********************************************************************************
		I2C bus operation
*********************************************************************************/

/********************************************************************************
  Description:
	config I2C bus device address
  Parameters:
	adr: I2C bus device address
  Remarks:
*********************************************************************************/
void VIM_SIF_SetI2cDevAddr(UINT8 adr)
{
	adr >>= 1;
	adr |= 0x80;
	VIM_HIF_SetReg8(V5_REG_SIF_IIC_DEVADDR, adr);
}




/********************************************************************************
  Description:
	I2C write function 
  Parameters:
	byAddr: the address of I2C slave device's register
	byVal: value
  Remarks:
*********************************************************************************/
UINT16 VIM_SIF_I2cWriteByte(UINT8 byAddr, UINT8 byVal)
{
	UINT8 uState = 0;
	UINT32	count = I2C_WRDELAY;
	
	while(count--)
	{
		uState = VIM_HIF_GetReg8(V5_REG_SIF_BUS_STATE);
		if((uState&0x2) == 0x2)
		{
			break;
		}
	}
	
	if(!count)
		return FAILED;
	VIM_HIF_SetReg8(V5_REG_SIF_SNRADDR, byAddr);
	VIM_HIF_SetReg8(V5_REG_SIF_SNRWRDATAH, byVal);
	VIM_HIF_SetReg8(V5_REG_SIF_IIC_BYTE, 1);
	VIM_HIF_SetReg8(V5_REG_SIF_SNRACSCTR, 0x1);

	return SUCCEED;
}

/********************************************************************************
  Description:
	I2C write function
  Parameters:
	uAddr: the address of I2C slave device's register 
	uVal: value
  Remarks:
*********************************************************************************/
UINT16 VIM_SIF_I2cWriteWord(UINT8 uAddr, UINT16 uVal)
{
	UINT8 uState = 0;
	UINT32	count = I2C_WRDELAY;
	
	while(count--)
	{
		uState = VIM_HIF_GetReg8(V5_REG_SIF_BUS_STATE);
		if((uState&0x2) == 0x2)
		{
			break;
		}
	}
	if(!count)
		return FAILED;
	VIM_HIF_SetReg8(V5_REG_SIF_SNRADDR, uAddr);
	VIM_HIF_SetReg8(V5_REG_SIF_SNRWRDATAH, (UINT8)(uVal >> 8));
	VIM_HIF_SetReg8(V5_REG_SIF_SNRWRDATAM, (UINT8)uVal);
	VIM_HIF_SetReg8(V5_REG_SIF_IIC_BYTE, 2);
	VIM_HIF_SetReg8(V5_REG_SIF_SNRACSCTR, 0x1);
	return SUCCEED;
}

/********************************************************************************
  Description:
	I2C write function
  Parameters:
	uAddr: the address of I2C slave device's register 
	uVal: value
  Remarks:
*********************************************************************************/
UINT16 VIM_SIF_I2cWriteWord16bit(UINT16 uAddr, UINT16 uVal)
{
	UINT8 uState = 0;
	UINT32	count = I2C_WRDELAY;
	
	while(count--)
	{
		uState = VIM_HIF_GetReg8(V5_REG_SIF_BUS_STATE);
		if((uState&0x2) == 0x2)
		{
			break;
		}
	}
	if(!count)
		return FAILED;
	VIM_HIF_SetReg8(V5_REG_SIF_SNRADDR, (UINT8)(uAddr>>8));
	VIM_HIF_SetReg8(V5_REG_SIF_SNRWRDATAH, (UINT8)(uAddr));

	VIM_HIF_SetReg8(V5_REG_SIF_SNRWRDATAM, (UINT8)(uVal >> 8));
	VIM_HIF_SetReg8(V5_REG_SIF_SNRWRDATAL, (UINT8)uVal);
	VIM_HIF_SetReg8(V5_REG_SIF_IIC_BYTE, 3);
	VIM_HIF_SetReg8(V5_REG_SIF_SNRACSCTR, 0x1);
	return SUCCEED;
}

/********************************************************************************
  Description:
	I2C read function
  Parameters:
	byAddr:		the address of I2C slave device's register 
	pbyVal:		return the value of this register
  Remarks:
*********************************************************************************/
UINT16 VIM_SIF_I2cReadByte(UINT8 byAddr, UINT8 *pbyVal)
{
	UINT8 uState = 0;
	UINT32	count = I2C_WRDELAY;
	
	while(count--)
	{
		uState = VIM_HIF_GetReg8(V5_REG_SIF_BUS_STATE);
		if((uState&0x2) == 0x2)
		{
			break;
		}
	}
	if(!count)

		return FAILED;
	VIM_HIF_SetReg8(V5_REG_SIF_SNRADDR, byAddr);
	VIM_HIF_SetReg8(V5_REG_SIF_IIC_BYTE, 1);
	VIM_HIF_SetReg8(V5_REG_SIF_SNRACSCTR, 0x2);
	count = I2C_WRDELAY;
	while(count--)
	{
		uState = VIM_HIF_GetReg8(V5_REG_SIF_BUS_STATE);
		if((uState&0x2) == 0x2)
		{	
			break;
		}
	}
	if(!count)
		return FAILED;
	*pbyVal = VIM_HIF_GetReg8(V5_REG_SIF_SNRRDDATAH);
	return SUCCEED;
}

/********************************************************************************
  Description:
	I2C read function
  Parameters:
	uAddr:		the address of I2C slave device's register 
	uVal:		return the value of this register
  Remarks:
*********************************************************************************/
UINT16 VIM_SIF_I2cReadWord(UINT8 uAddr, UINT16 *uVal)
{
	UINT8 uState = 0;

	UINT8 x1 = 0, x2 = 0;
	UINT32	count = I2C_WRDELAY;
	
	while(count--)
	{
		uState = VIM_HIF_GetReg8(V5_REG_SIF_BUS_STATE);
		if((uState&0x2) == 0x2)
		{
			break;
		}
	}
	if(!count)
		return FAILED;
	VIM_HIF_SetReg8(V5_REG_SIF_SNRADDR, uAddr);
	VIM_HIF_SetReg8(V5_REG_SIF_IIC_BYTE, 2);
	VIM_HIF_SetReg8(V5_REG_SIF_SNRACSCTR, 0x2);
	count = I2C_WRDELAY;
	while(count--)
	{
		uState = VIM_HIF_GetReg8(V5_REG_SIF_BUS_STATE);
		if((uState&0x2) == 0x2)
		{
			break;
		}
	}

	if(!count)
		return FAILED;
	x1 = VIM_HIF_GetReg8(V5_REG_SIF_SNRRDDATAH);
	x2 = VIM_HIF_GetReg8(V5_REG_SIF_SNRRDDATAM);

	*uVal = ((UINT16)x1 << 8) | ((UINT16)x2);
	return SUCCEED;
}




/********************************************************************************
  Description:
	I2C bus batch write 
  Parameters:
	uNum: AeData length, must in [0, 7]
	AeData: data
  Note:
	The write data number must same at every register's address
	The update time is according to Ae Window's setting
  Remarks:
*********************************************************************************/
void VIM_SIF_I2cAeBatch(UINT8 uNum, TI2cBatch *AeData)
{
	UINT8	i, j, reg_B_ctrl = 0;
	UINT8	uRegAeCount = 0;
	UINT16	regCnt, reg_B_cnt = 0;
	
	if((uNum > 0) && (uNum < 8))
	{
		for(i = 0; i < uNum; i++)
		{
			regCnt = AeData[i].RegBytes & 0x3;
			regCnt <<= i*2;
			reg_B_cnt |= regCnt;
			VIM_HIF_SetReg8(V5_REG_SIF_AE0_DATA+uRegAeCount, AeData[i].RegAddr);
			uRegAeCount++;
			for(j = 0; j < AeData[i].RegBytes; j++)
			{
				VIM_HIF_SetReg8(V5_REG_SIF_AE0_DATA+uRegAeCount, AeData[i].RegVal[j]);
				uRegAeCount++;
			}
		}
		VIM_HIF_SetReg8(V5_REG_SIF_IIC_BYTE, AeData[0].RegBytes & 0x3);
		VIM_HIF_SetReg8(V5_REG_SIF_B_CNTL, (UINT8)reg_B_cnt);
		VIM_HIF_SetReg8(V5_REG_SIF_B_CNTH, (UINT8)(reg_B_cnt >> 8));
		
		reg_B_ctrl = uNum;	//The number of register addresses
		reg_B_ctrl |= 0x8;	//AE data update done bit.

		VIM_HIF_SetReg8(V5_REG_SIF_AEBUS_CTRL, (UINT8)reg_B_ctrl);
	}
}





