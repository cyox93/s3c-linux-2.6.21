/*                Copyright (C) 2005 Vimicro CO.,LTD     		 
*
* FILE NAME                                  VERSION                
*                                                                       
* VIM_SIF_Driver.H			   0.2                   
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*     VC0578 's SIF+ISP moudle sub head file                               
*
*	Version		Author			Date		Description		
*  ---------------------------------------------------------------						 
*   0.1			LiuHD			2005-11-16	The first version. 
     0.2			angela 			2006-06-08	updata for 528
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


#ifndef _VIM_0528RDK_SIF_H_
#define _VIM_0528RDK_SIF_H_

#ifdef __cplusplus
extern "C" {
#endif

#define I2C_WRDELAY 0x7f00
typedef enum tag_VIM_SIF_RESOLUTION
{
	VIM_SIF_MAXPIXEL = 0,
	VIM_SIF_HALFPIXEL
}VIM_SIF_RESOLUTION;

typedef enum  tag_VIM_SIF_OUTPUT_SYNC
{
	VIM_SIF_SNSOR_NORMAL_SYNC    =0,
	VIM_SIF_SNSOR_CCIR656_SYNC    =1
}VIM_SIF_OUTPUT_SYNC;



typedef enum tag_VIM_SIF_SENSORSTATE
{
	VIM_SIF_SENSOR_POWERON, /* Sensor is on power on state */
//	VIM_SIF_SENSOR_POWEROFF, /* Sensor is on power off state */
	VIM_SIF_SENSOR_STANDBY /* Sensor is on standby state */
//	VIM_SIF_SENSOR_SLEEP /* Sensor is on sleep state  */
}VIM_SIF_SENSORSTATE;

typedef enum _VIM_SIF_CLKTYPE
{
	VIM_SIF_SENSOR_CLKNORMAL=0, /* Sensor normal clk */
	VIM_SIF_SENSOR_HALF=1 /* Sensor half clk ,yuv is 1/4*/
}VIM_SIF_CLKTYPE;


VIM_RESULT VIM_SIF_SifInitI2c(const TSnrInfo *pSensorInfo,UINT32 dwMclk);
VIM_RESULT VIM_SIF_SifSensorInit(const TSnrInfo *pSensorInfo);
VIM_RESULT VIM_SIF_SetSensorResolution(const TSnrInfo *pSensorInfo, VIM_SIF_RESOLUTION byResolution);
VIM_RESULT VIM_SIF_SetSensorState(const TSnrInfo *pSensorInfo, VIM_SIF_SENSORSTATE bySensorState);
VIM_RESULT VIM_SIF_SetSensorPowerOn(const TSnrInfo *pSensorInfo);
VIM_RESULT VIM_SIF_GetSensorResolution(const TSnrInfo *pSensorInfo, VIM_SIF_RESOLUTION bySensorResolution, TSize *InputSize);
VIM_RESULT VIM_SIF_SetSensorClk(const TSnrInfo *pSensorInfo,VIM_SIF_CLKTYPE Type);


void 	VIM_SIF_SensorReset(void);
void 	VIM_SIF_EnableSyncGen(UINT8 bEnable);
void        VIM_SIF_StartCaptureEnable(UINT8 bEnable);
void 	VIM_SIF_SetI2cDevAddr(UINT8 adr);
UINT16 	VIM_SIF_I2cWriteByte(UINT8 byAddr, UINT8 byVal);
UINT16 	VIM_SIF_I2cWriteWord(UINT8 uAddr, UINT16 uVal);
UINT16 	VIM_SIF_I2cReadByte(UINT8 byAddr, UINT8 *pbyVal);
UINT16 	VIM_SIF_I2cReadWord(UINT8 uAddr, UINT16 *uVal);
void 	VIM_SIF_I2cAeBatch(UINT8 uNum, TI2cBatch *AeData);


/*
  FLASH FUNCTION
*/
#define SENSOR_TYPE_YUV	1
#define SENSOR_TYPE_RGB	0


#ifdef __cplusplus
}
#endif

#endif



