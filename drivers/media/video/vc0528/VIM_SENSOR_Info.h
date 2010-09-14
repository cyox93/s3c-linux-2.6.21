
/*************************************************************************
*                                                                       
*             Copyright (C) 2005 Vimicro CO.,LTD     		 
*
* FILE NAME                                  VERSION                
*                                                                       
* VIM_SENSOR_Info.h				           	  0.1                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*    VC0578  sensor iformation head file                      
*
*	Version		Author			Date		Description		
*  ---------------------------------------------------------------						 
*   0.1			yaoweiquan  			2005-11-01	The first version. 
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
#ifndef _VIM_0528RDK_SENSORINFO_H_
#define _VIM_0528RDK_SENSORINFO_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef void	(*PSensorSetReg)(UINT16 uAddr, UINT16 uVal);
typedef void	(*PSensorGetReg)(UINT8 uAddr, UINT16 *uVal);
typedef UINT8	(*PSnrIdChk)(void);
typedef void	(*PSnrMirrorFlip)(UINT8 val);
typedef void	(*PSnrContrast)(UINT8 val);
typedef void	(*PSnrBrightness)(UINT8 val);
typedef void	(*PSnrWhiteBalance)(UINT8 mode,UINT8 val);
typedef void	(*PSnrMode)(UINT8 val);
typedef void	(*PVispIsrCallback)(void);

//============== sif type =======================
//I2c Structure
typedef struct tag_TI2cBatch
{
	UINT8 RegBytes;
	UINT8 RegAddr;
	UINT8 RegVal[3];
} TI2cBatch, *PTI2cBatch;

typedef struct tag_TAeParm
{
	UINT8	ytarget;
	UINT8	ythreshold;
	UINT8	ytop;
	UINT8	gtarget;						//gain target
	UINT8	gthreshold;						//gain threshold
	UINT8	gdelay;
	UINT8	gain;
	UINT8	minet;							//if minet=0,et can decrease as it shoule be.if minet=1,it means 
	UINT8	maxgain;
	UINT8	mingain;
	UINT8	speed;							//adjust ae speed 0,1,2 
} TAeParm, *PTAeParm;
typedef struct tag_TFlashParm
{
	UINT8 yflashthd;
	UINT8 flashstep;
} TFlashParm, *PTFlashParm;

typedef struct tag_THistParm
{
	UINT8	ratio;							//percent of whole frame pixel, 1 is recommend.
} THistParm, *PTHistParm;

typedef struct tag_TSnrSizeCfg
{
	TRegGroup	snr;
	TRegGroup	visp;
	TSize		size;
	UINT8		snrclkratio;		//SNR CLK = MCLK / ratio
	UINT8		snrpixelratio;		//SNR Pixel freq = SNR CLK / pixel ratio
} TSnrSizeCfg, *PTSnrSizeCfg;

typedef struct tag_TTSnrSizeCfgGroup
{
	UINT8 len;
	TSnrSizeCfg *pSizeCfg;
} TSnrSizeCfgGroup, *PTSnrSizeCfgGroup;

typedef enum tag_TSnrType			//guoying 2/27/2008 add
{
	VIM_SNR_NORMAL=~BIT7,
	VIM_SNR_CCIR656=BIT7,
	VIM_SNR_YUV=BIT6	
	
}TSnrType;

typedef void	(*PSnrGetEt) (UINT8 *RegBuf);
typedef void	(*PSnrSetEt) (UINT8 *RegBuf, UINT8 div);
typedef struct tag_TSensorInfo
{
	char				desc[40];	

	//UINT8				snrtype;			//sensor type(RGB or YUV)
	TSnrType				snrtype;			//sensor type: YUV, CCIR656
	UINT8				pclk;				//use PCLK of sensor
	UINT8				clkcfg;				//[0]: VSync delay enable; [1]: Fifo VSync delay enable; [2]: Use PCLK Negtive Edge.
	UINT8				bustype;			//[0]: 0 -- I2C, 1 -- Serial bus;
	UINT8				i2ctype;			//[0]: 1 -- OV I2C bus
	UINT16				i2crate;			//I2C rate : KHz				
	UINT8				i2caddress;			//I2C address
	UINT8				i2cispaddress;		//ISP I2C address
	UINT8				pwrcfg;				//sensor power initial configure(SIF REG801 BIT[0]~BIT[2])
	UINT8				snrrst;				//Reset sensor enable

	UINT8				brightness;			//brightness
	UINT8				contrast;			//contrast

	TRegGroup			standby;			//sensor standby register
	TRegGroup			Initpoweron;				//SENSOR ISP initial configure

	TRegGroup			sifpwronseq;		//Sif config sequence(Reg.800 bit0~2) when sensor power on
	TRegGroup			sifstdbyseq;		//Sif config sequence(Reg.800 bit0~2) when sensor standby
	TRegGroup			dsif;				//SIF initial configure
	TSnrSizeCfgGroup		snrSizeCfg;			//sensor size configure information

	PSensorSetReg		snrSetRegCall;		//set reg callback
	PSensorGetReg		snrGetRegCall;
	PSnrIdChk			snrIdChkCall;
	PSnrMirrorFlip			snrMirrorFlipCall;
	PSnrContrast			snrContrastCall;
	PSnrBrightness		snrBrightnessCall;
	PSnrWhiteBalance		snrWhiteBalanceCall;
	PSnrMode			snrModeCall;
	PSnrGetEt			snrGetEt;
	PSnrSetEt			snrSetEt;
	TFlashParm			flashparm;

} TSnrInfo, *PTSnrInfo;

extern const PTSnrInfo gPSensorInfo[];
#define V5B_SifI2cWriteByte(addr,val)VIM_SIF_I2cWriteByte(addr,val)
#define V5B_SifI2cWriteWord(addr,val)VIM_SIF_I2cWriteWord(addr,val)
#define V5B_SifI2cReadByte(addr,val)	VIM_SIF_I2cReadByte(addr,val)
#define V5B_SifI2cReadWord(addr,val)	VIM_SIF_I2cReadWord(addr,val)
#define V5B_SifI2cAeBatch(num,pval)  VIM_SIF_I2cAeBatch(num,pval)

#define FAILED FALSE
#define SUCCEED TRUE

extern void SensorSetReg_size(UINT16 init_size);
extern void SensorSetReg_data(UINT32 index, UINT32 addr, UINT16 data);
extern void SensorSetReg_check1(UINT32 size);
extern void SensorSetReg_check2(void);
#ifdef __cplusplus
}
#endif

#endif 
