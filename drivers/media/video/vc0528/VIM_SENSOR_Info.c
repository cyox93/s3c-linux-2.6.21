
/*************************************************************************
*                                                                       
*             Copyright (C) 2005 Vimicro CO.,LTD     		 
*
* FILE NAME                                  VERSION                
*                                                                       
* VIM_SENSOR_Info.c				0.4                  
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*    VC0578  sensor iformation file                      
*
*	Version		Author			Date		Description		
*  ---------------------------------------------------------------						 
*   0.1			yaoweiquan  		2005-11-01	The first version. 
*   0.2			angela   			2006-06-08	update for 528
*   0.3			yaoweiquan		2006-11-2 	update with new structure
*   1.0			angela			2006-12-8 	update 
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

UINT32 g_Ae_para[10];
UINT16 g_Awb_para[10];

/*****************************************************************************************************
		SENSOR/SIF/ISP information
*****************************************************************************************************/
//0.3M
#if V5_DEF_SNR_SIV120D_YUV	

void SensorSetReg_SIV120D(UINT16 uAddr, UINT16 uVal)
{
         V5B_SifI2cWriteByte((UINT8)uAddr, (UINT8)uVal);
}

void SensorGetReg_SIV120D(UINT8 uAddr, UINT16 *uVal)
{
	V5B_SifI2cReadByte((UINT8)uAddr, (UINT8*)uVal);
}

UINT8 CheckId_SIV120D(void)
{
	UINT8 temp1;
	UINT8 temp2; 
	
	 V5B_SifI2cWriteByte(0x00, 0x00); 
        V5B_SifI2cReadByte(0x01, &temp1);
	 V5B_SifI2cReadByte(0x02, &temp2);    
        if((temp1==0x12) && (temp2==0x13))
                return SUCCEED; 
        else 
                return FAILED;  
}

void SensorSetMirrorFlip_SIV120D(UINT8 val)
{
        UINT8 x;
//	val=~val;
	if(val==0x01)
		val=0x2;
	else if(val==0x02)
		val=0x01;
	V5B_SifI2cWriteByte(0x00, 0x00); 
 	V5B_SifI2cReadByte(0x04, &x);
 	x&=0xfc;//clear bit0,bit1
	x |= (val&0x03);
 	V5B_SifI2cWriteByte(0x04, x);
}

/******************************************************************
         Desc: set sensor contrast  callback 
         Para: 1-5 step
*******************************************************************/
void SensorSetContrast_SIV120D(UINT8 val)
{
	switch(val)
	{	
	case 1:
		V5B_SifI2cWriteByte(0x00, 0x03);     
		V5B_SifI2cWriteByte(0xac, 0x80);              
		break;  
	case 2:    
		V5B_SifI2cWriteByte(0x00, 0x03);     
		V5B_SifI2cWriteByte(0xac, 0x88);                                            
		break;
   	case 3:      
		V5B_SifI2cWriteByte(0x00, 0x03);     
		V5B_SifI2cWriteByte(0xac, 0x90);                          
		break; 
        case 4:     
		V5B_SifI2cWriteByte(0x00, 0x03);     
		V5B_SifI2cWriteByte(0xac, 0x98);                           
		break;  
	case 5:     
		V5B_SifI2cWriteByte(0x00, 0x03);     
		V5B_SifI2cWriteByte(0xac, 0xa0);                          
		break; 
	default:
		break;
	}					
}

/******************************************************************
         Desc: set sensor Brightness  callback 
         Para: 1-5 step
*******************************************************************/
void SensorSetBrightness_SIV120D(UINT8 val)
{
	switch(val)
	{
	case 1:
		V5B_SifI2cWriteByte(0x00, 0x03);     
		V5B_SifI2cWriteByte(0xab, 0xac);   
		break;	
	case 2:
		V5B_SifI2cWriteByte(0x00, 0x03);     
		V5B_SifI2cWriteByte(0xab, 0x96);   
		break;
	case 3:
		V5B_SifI2cWriteByte(0x00, 0x03);     
		V5B_SifI2cWriteByte(0xab, 0x80);   
		break;
	case 4:
		V5B_SifI2cWriteByte(0x00, 0x03);     
		V5B_SifI2cWriteByte(0xab, 0x16);   
		break;
	case 5:
		V5B_SifI2cWriteByte(0x00, 0x03);     
		V5B_SifI2cWriteByte(0xab, 0x2c);   
		break;
	default:
		break;		
	}
}

/******************************************************************
         Desc: set sensor mode  callback 
         Para:  val: 	1 50hz,	2 60hz,	3 night, 4 outdoor.
*******************************************************************/
void SensorSetMode_SIV120D(UINT8 val)
{
	/*
	UINT8 uTemp;
	switch (val)
	{	
		case 1://50Hz 20fps
					V5B_SifI2cWriteByte(0x00,0x01);					
					V5B_SifI2cWriteByte(0x35,0x76);
					V5B_SifI2cWriteByte(0x11,0x05);
			break;
		case 2://60Hz 20fps
					V5B_SifI2cWriteByte(0x00,0x01);					
					V5B_SifI2cWriteByte(0x35,0x62);
					V5B_SifI2cWriteByte(0x11,0x06);
			break;
		case 4:  // 3 and 4 reverse,night mode
					V5B_SifI2cWriteByte(0x00,0x01);
					V5B_SifI2cReadByte(0x35, &uTemp);	
			if ((uTemp&0x76) == 0x76)				//50Hz 10fps
			{		
					V5B_SifI2cWriteByte(0x11,0x0a);
				}
			else	//60Hz 10fps
			{

					V5B_SifI2cWriteByte(0x11,0x0c);
				}
			
			break;
		case 3:	//outdoor
			V5B_SifI2cWriteByte(0x00,0x00);
			V5B_SifI2cReadByte(0x35, &uTemp);	
			if ((uTemp&0x76) == 0x76)				//50Hz 20fps
			{		
					V5B_SifI2cWriteByte(0x11,0x05);
				}
			else	//60Hz 20fps
			{
					V5B_SifI2cWriteByte(0x11,0x06);
				}
				
			break;
		default:
			break;
	}
	*/
}

const TReg gSifPwrOnSeqRegVal_SIV120D[] =
{
	{0x00, 0x66,	1},
	{0x00, 0x67,	1}
};

const TReg gSifRegVal_SIV120D[] =
{        
 	  {0x02,  0x00, 0},
	  {0x03,  0x0a, 0},
	  {0x04,  0x05, 0},
	  
   	  {0x01,  0x41, 0} 
};

const TReg gSnrSizeRegValVGA_SIV120D[] = 
{      
	//bank0
	{0x00,0x00,1},
	{0x04,0x00,1}, // valiable v sync mode
//	{0x04,0x80,1},   // fix 30f v sync fix mode 
	{0x05,0x03,1},
	{0x10,0x34,1},
	{0x11,0x27,1},
	{0x12,0x21,1},
	{0x16,0xc6,1},
	{0x17,0xaa,1},
	//24MHz 30fps
	{0x20,0x00,1},	//P_BNKT  
	{0x21,0x01,1},	//P_HBNKT 
	{0x22,0x01,1},	//P_ROWFIL
	{0x23,0x01,1},	//P_VBNKT


	//bank1:AE
	{0x00,0x01,1}, 
	{0x11,0x1e,1},  // 4fps at lowlux            
//	{0x11,0x14,1},  // 6fps at lowlux            
//	{0x11,0x0c,1},  // 10fps at lowlux            
//	{0x11,0x04,1},  // 30fps at lowlux            
	{0x12,0x78,1},	// D65 target 0x74
	{0x13,0x78,1},	// CWF target 0x74
	{0x14,0x78,1},	// A target   0x74
	{0x1E,0x08,1},	// ini gain	0x08
	{0x34,0x7d,1},  //           
	{0x40,0x60,1}, 	// Max x6          

 	{0x41,0x20,1},  //AG_TOP1  0x28 
 	{0x42,0x20,1},  //AG_TOP0  0x28
	{0x43,0x00,1},  //AG_MIN1  0x08
 	{0x44,0x00,1},  //AG_MIN0  0x08
	{0x45,0x00,1},  //G50_dec  0x09
	{0x46,0x0a,1},  //G33_dec  0x17
	{0x47,0x10,1},  //G25_dec  0x1d
	{0x48,0x13,1},  //G20_dec  0x21
	{0x49,0x15,1},  //G12_dec  0x23
	{0x4a,0x18,1},  //G09_dec  0x24
	{0x4b,0x1a,1},  //G06_dec  0x26
	{0x4c,0x1d,1},  //G03_dec  0x27
	{0x4d,0x20,1},  //G100_inc 0x27
	{0x4e,0x10,1},  //G50_inc  0x1a
	{0x4f,0x0a,1},  //G33_inc  0x14
	{0x50,0x08,1},  //G25_inc  0x11
	{0x51,0x06,1},  //G20_inc  0x0f
	{0x52,0x05,1},  //G12_inc  0x0d
	{0x53,0x04,1},  //G09_inc  0x0c
	{0x54,0x02,1},  //G06_inc  0x0a
	{0x55,0x01,1},  //G03_inc  0x09 


	//AWB
	{0x00,0x02,1},
	{0x10,0xd3,1},
	{0x11,0xc0,1},
	{0x12,0x80,1},
	{0x13,0x80,1},
	{0x14,0x80,1},
	{0x15,0xfe,1},	// R gain Top
	{0x16,0x80,1},	// R gain bottom 
	{0x17,0xcb,1},	// B gain Top
	{0x18,0x80,1},	// B gain bottom 0x80
	{0x19,0x94,1},	// Cr top value 0x90
	{0x1a,0x6c,1},	// Cr bottom value 0x70
	{0x1b,0x94,1},	// Cb top value 0x90
	{0x1c,0x6c,1},	// Cb bottom value 0x70
	{0x1d,0x94,1},	// 0xa0
	{0x1e,0x6c,1},	// 0x60
	{0x20,0xe8,1},	// AWB luminous top value
	{0x21,0x30,1},	// AWB luminous bottom value 0x20
	{0x22,0xa4,1},
	{0x23,0x20,1},
	{0x25,0x20,1},
	{0x26,0x0f,1},
	{0x27,0x60,1},	// BRTSRT 
	{0x28,0xcb,1},	// BRTRGNTOP result 0xad
//*	{0x29,0xa0,1},	// BRTRGNBOT 
	{0x29,0xc8,1},	// BRTRGNBOT 
	{0x2a,0xa0,1},	// BRTBGNTOP result 0x90
	{0x2b,0x98,1},  // BRTBGNBOT
	{0x2c,0x88,1},	// RGAINCONT
	{0x2d,0x88,1},	// BGAINCONT

	{0x30,0x00,1},
	{0x31,0x10,1},
	{0x32,0x00,1},
	{0x33,0x10,1},
	{0x34,0x02,1},
	{0x35,0x76,1},
	{0x36,0x01,1},
	{0x37,0xd6,1},
	{0x40,0x01,1},
	{0x41,0x04,1},
	{0x42,0x08,1},
	{0x43,0x10,1},
	{0x44,0x12,1},
	{0x45,0x35,1},
	{0x46,0x64,1},
	{0x50,0x33,1},
	{0x51,0x20,1},
	{0x52,0xe5,1},
	{0x53,0xfb,1},
	{0x54,0x13,1},
	{0x55,0x26,1},
	{0x56,0x07,1},
	{0x57,0xf5,1},
	{0x58,0xea,1},
	{0x59,0x21,1},

	{0x62,0x9c,1},	// G gain

	{0x63,0xb3,1},	// R D30 to D20
	{0x64,0xc3,1},	// B D30 to D20
	{0x65,0xb3,1},	// R D20 to D30
	{0x66,0xc3,1},	// B D20 to D30

	{0x67,0xdd,1},	// R D65 to D30
	{0x68,0xa0,1},	// B D65 to D30
	{0x69,0xdd,1},	// R D30 to D65
	{0x6a,0xa0,1},	// B D30 to D65

	
	//IDP
	{0x00,0x03,1},
	{0x10,0xff,1},
	{0x11,0x0d,1},
	{0x12,0x3d,1},
	{0x14,0x04,1}, // don't change

	{0xc0,0x24,1}, // windows size define
	{0xc1,0x00,1},
	{0xc2,0x80,1},
	{0xc3,0x00,1},
	{0xc4,0xe0,1},

	//DPCNR     
	{0x17,0x28,1},  // DPCNRCTRL
	{0x18,0x04,1},  // DPTHR
	{0x19,0x50,1},  // C DP Number ( Normal [7:6] Dark [5:4] ) | [3:0] DPTHRMIN
	{0x1A,0x50,1},  // G DP Number ( Normal [7:6] Dark [5:4] ) | [3:0] DPTHRMAX
	{0x1B,0x12,1},  // DPTHRSLP( [7:4] @ Normal | [3:0] @ Dark )
	{0x1C,0x00,1},  // NRTHR
	{0x1D,0x0f,1},  // [5:0] NRTHRMIN 0x48
	{0x1E,0x0f,1},  // [5:0] NRTHRMAX 0x48
	{0x1F,0x3f,1},  // NRTHRSLP( [7:4] @ Normal | [3:0] @ Dark )	0x2f
	{0x20,0x04,1},  // IllumiInfo STRTNOR
	{0x21,0x0f,1},  // IllumiInfo STRTDRK
	//Gamma                   ssignme  
	{0x30,0x00,1}, 	//0x0 
	{0x31,0x04,1}, 	//0x3 
	{0x32,0x0b,1}, 	//0xb 
	{0x33,0x24,1},	//0x1f
	{0x34,0x49,1},	//0x43
	{0x35,0x66,1},	//0x5f
	{0x36,0x7C,1},	//0x74
	{0x37,0x8D,1},	//0x85
	{0x38,0x9B,1},	//0x94
	{0x39,0xAA,1},	//0xA2ssignme
	{0x3a,0xb6,1},	//0xAF
	{0x3b,0xca,1},	//0xC6
	{0x3c,0xdc,1},	//0xDB
	{0x3d,0xef,1},	//0xEF
	{0x3e,0xf8,1},	//0xF8
	{0x3f,0xFF,1},	//0xFF
	//Shading Register Setting                                    
	{0x40,0x0a,1},                                                    
	{0x41,0xdc,1},                                                    
	{0x42,0x44,1},                                                    
	{0x43,0x33,1},                                                    
	{0x44,0x33,1},                                                    
	{0x45,0x11,1},                                                    
	{0x46,0x11,1},	// left R gain[7:4], right R gain[3:0]             
	{0x47,0x22,1},	// top R gain[7:4], bottom R gain[3:0]             
	{0x48,0x01,1},	// left Gr gain[7:4], right Gr gain[3:0] 0x21           
	{0x49,0x01,1},	// top Gr gain[7:4], bottom Gr gain[3:0]           
	{0x4a,0x01,1},	// left Gb gain[7:4], right Gb gain[3:0] 0x02          
	{0x4b,0x01,1},	// top Gb gain[7:4], bottom Gb gain[3:0]           
	{0x4c,0x12,1},	// left B gain[7:4], right B gain[3:0]             
	{0x4d,0x10,1},	// top B gain[7:4], bottom B gain[3:0]             
	{0x4e,0x04,1},	// X-axis center high[3:2], Y-axis center high[1:0]
	{0x4f,0x50,1},	// X-axis center low[7:0] 0x50                     
	{0x50,0xf8,1},	// Y-axis center low[7:0] 0xf6                     
	{0x51,0x80,1},	// Shading Center Gain                             
	{0x52,0x00,1},	// Shading R Offset                                
	{0x53,0x00,1},	// Shading Gr Offset                               
	{0x54,0x00,1},	// Shading Gb Offset                               
	{0x55,0x00,1},	// Shading B Offset   
	//Interpolation
/*
	{0x60,0x57,1},	//INT outdoor condition
	{0x61,0x57,1},	//INT normal condition
*/
	{0x62,0x77,1},	//ASLPCTRL 7:4 GE, 3:0 YE
	{0x63,0xb7,1},	//YDTECTRL (YE) [7] fixed,
	{0x64,0xb7,1},	//GPEVCTRL (GE) [7] fixed,
/*	
	{0x66,0x0c,1},	//SATHRMIN
	{0x67,0xff,1},
	{0x68,0x04,1},	//SATHRSRT
	{0x69,0x08,1},	//SATHRSLP

	{0x6a,0xaf,1},	//PTDFATHR [7] fixed, [5:0] value
	{0x6b,0x60,1},	//PTDLOTHR [6] fixed, [5:0] value
*/
	{0x6d,0x88,1},	//YFLTCTRL
	//Color matrix (D65) - Daylight
	{0x71,0x42,1},	//0x40       
	{0x72,0xbf,1},	//0xb9      
	{0x73,0x00,1},	//0x07      
	{0x74,0x0f,1},	//0x15       
	{0x75,0x31,1},	//0x21       
	{0x76,0x00,1},	//0x0a       
	{0x77,0x00,1},	//0xf8     
	{0x78,0xbc,1},	//0xc5       
	{0x79,0x44,1},	//0x46       
	//Color matrix (D30) - CWF
	{0x7a,0x3d,1},	//0x3a	
	{0x7b,0xcd,1},  //0xcd 
	{0x7c,0xfa,1},  //0xfa 
	{0x7d,0x12,1},  //0x12 
	{0x7e,0x2c,1},  //0x2c 
	{0x7f,0x02,1},  //0x02 
	{0x80,0xf7,1},  //0xf7 
	{0x81,0xc7,1},  //0xc7 
	{0x82,0x42,1},  //0x42 
	//Color matrix (D20) - A
	{0x83,0x3a,1},	//0x38         
	{0x84,0xcd,1},  //0xc4       
	{0x85,0xfa,1},  //0x04    
	{0x86,0x12,1},  //0x07      
	{0x87,0x2c,1},  //0x25       
	{0x88,0x02,1},  //0x14       
	{0x89,0xf7,1},  //0xf0   
	{0x8a,0xc7,1},  //0xc2       
	{0x8b,0x42,1},  //0x4f       

	{0x8c,0x10,1},	//CMA select

	{0x8d,0x04,1},	//programmable edge
	{0x8e,0x02,1},	//PROGEVAL
	{0x8f,0x00,1},	//Cb/Cr coring assignme
	    	
	{0x90,0x10,1},	//GEUGAIN
	{0x91,0x10,1},	//GEDGAIN
	{0x92,0x02,1},	//Ucoring [7:4] max, [3:0] min
//	{0x94,0x02,1},	//Uslope (1/128)
//*	{0x96,0xf0,1},	//Dcoring [7:4] max, [3:0] min
	{0x96,0x02,1},	//Dcoring [7:4] max, [3:0] min

	{0x9f,0x0c,1},	//YEUGAIN
	{0xa0,0x0c,1},	//YEDGAIN
	{0xa1,0x22,1},	//Yecore [7:4]upper [3:0]down

	{0xa9,0x0f,1},	// Cr saturation 0x12
	{0xaa,0x12,1},	// Cb saturation 0x12
	{0xab,0x82,1},	// Brightness
	{0xae,0x40,1},	// Hue
	{0xaf,0x86,1},	// Hue
	{0xb9,0x10,1},	// 0x20 lowlux color
	{0xba,0x20,1},	// 0x10 lowlux color            

	//inverse color space conversion
	{0xcc,0x40,1},
	{0xcd,0x00,1},
	{0xce,0x58,1},
	{0xcf,0x40,1},
	{0xd0,0xea,1},
	{0xd1,0xd3,1},
	{0xd2,0x40,1},
	{0xd3,0x6f,1},
	{0xd4,0x00,1},
	
	//ee/nr
	{0xdd,0x39,1},         
	{0xde,0xe3,1},	//NOIZCTRL         

	//dark offset
	{0xdf,0x10,1},
	{0xe0,0x60,1},
	{0xe1,0x90,1},
	{0xe2,0x08,1},
	{0xe3,0x0a,1},
	//memory speed
	{0xe5,0x15,1},
	{0xe6,0x20,1},
	{0xe7,0x04,1},

	//Sensor On    
	{0x00,0x00,1},
	{0x03,0x05,1},
};

const TSnrSizeCfg gSensorSizeCfg_SIV120D[] = 
{
	{
              {0,NULL},
              {0,NULL},
	      	{640, 480},
		1,
		1

	}
};

const TSnrInfo gSensorInfo_SIV120D = 
{
       "SIV120D yuv mode",
	VIM_SNR_YUV,							//yuv mode
	0,							//not use PCLK of sensor
	0,							//[0]: VSync delay enable; [1]: Fifo VSync delay enable; [2]: Use PCLK Negtive Edge.
	0,							//0 -- I2C, 1 -- Serial bus;
	0,							//[0]: 1 -- OV I2C bus
	100,							//I2C Rate : 100KHz
	0x66,						//I2C address
	0x66,						//ISP I2C address for special sensor
	0x07,						//power config
	0,							//reset sensor
	3,							//brightness 
	3,							//contrast

	{0,NULL},					//sensor standby                                                          
       {sizeof(gSnrSizeRegValVGA_SIV120D) / sizeof(TReg), (PTReg)&gSnrSizeRegValVGA_SIV120D},                                                       //snr initial value
	{sizeof(gSifPwrOnSeqRegVal_SIV120D) / sizeof(TReg), (PTReg)&gSifPwrOnSeqRegVal_SIV120D},	//Sif config sequence(Reg.800 bit0~2) when sensor power on                
	{0, NULL},											//Sif config sequence(Reg.800 bit0~2) when sensor standby             
	{sizeof(gSifRegVal_SIV120D) / sizeof(TReg), (PTReg)&gSifRegVal_SIV120D},			//sif initial value                                                       
	{sizeof(gSensorSizeCfg_SIV120D) / sizeof(TSnrSizeCfg), (PTSnrSizeCfg)&gSensorSizeCfg_SIV120D},	//size configure                                                          

	SensorSetReg_SIV120D,																				
	SensorGetReg_SIV120D,																				
	CheckId_SIV120D,																				
	SensorSetMirrorFlip_SIV120D,																			
	SensorSetContrast_SIV120D,																									//Sensor switch size callback
	SensorSetBrightness_SIV120D,																									//set et callback
	NULL,	
	SensorSetMode_SIV120D,
	NULL,																						
	NULL,																						
	{	//ythd of flash,et of flash
		0x20,			10
	},
};

#if 0
void SensorSetReg_size(UINT16 init_size)
{
	gSensorInfo_SIV120D.Initpoweron.len = init_size;
}

/* new paramete write */
void SensorSetReg_data(UINT32 index, UINT32 addr, UINT16 data)
{
	gSnrSizeRegValVGA_SIV120D[index].adr  = addr; 	//register address
	gSnrSizeRegValVGA_SIV120D[index].val  = data; 	//register setting value
	gSnrSizeRegValVGA_SIV120D[index].wait = 1; 		//ms
}

/* new paramete check */
void SensorSetReg_check1(UINT32 size)
{
	UINT32 cnt;

	printk("size: %04d  %04d\n",size,gSensorInfo_SIV120D.Initpoweron.len);
	for(cnt=0; cnt<size; cnt++){
		printk("%03d   0x%02x   0x%02x   0x%02x\n",
				cnt,
				gSnrSizeRegValVGA_SIV120D[cnt].adr, 	//register address
				gSnrSizeRegValVGA_SIV120D[cnt].val, 	//register setting value
				gSnrSizeRegValVGA_SIV120D[cnt].wait 	//ms		
			  );
	}
}

/* orgenal paramete check */
void SensorSetReg_check2(void)
{
	UINT32 size = sizeof(gSnrSizeRegValVGA_SIV120D) / sizeof(TReg);
	UINT32 cnt; 

	printk("size: %04d  %04d\n",size, gSensorInfo_SIV120D.Initpoweron.len);
	for(cnt=0; cnt<size; cnt++){
		printk("%03d   0x%02x   0x%02x   0x%02x\n",
				cnt,
				gSnrSizeRegValVGA_SIV120D[cnt].adr, 	//register address
				gSnrSizeRegValVGA_SIV120D[cnt].val, 	//register setting value
				gSnrSizeRegValVGA_SIV120D[cnt].wait 	//ms		
			  );
	}
}
#endif
#endif


#if V5_DEF_SNR_MT9V111_YUV

void SensorSetReg_MT9V111(UINT16 uAddr, UINT16 uVal)
{
         V5B_SifI2cWriteWord((UINT8)uAddr, (UINT16)uVal);
}

void SensorGetReg_MT9V111(UINT8 uAddr, UINT16 *uVal)
{
	V5B_SifI2cReadWord((UINT8)uAddr, (UINT16*)uVal);
}

UINT8 CheckId_MT9V111(void)
{
	 UINT16 temp;
	 V5B_SifI2cWriteWord(0x01, 4);
	 V5B_SifI2cReadWord(0xff, &temp);


	 if(temp==0x823a) 
	 	return SUCCEED;
	 else
	 	return FAILED; 
}

void SensorSetMirrorFlip_MT9V111(UINT8 val)
{
        UINT16 x;
	if(val==0x00)
		val=0x03;
	else if(val==0x03)
		val=0x00;
	
	V5B_SifI2cWriteWord(0x01, 4);
	V5B_SifI2cReadWord(0x20, &x);
	x&=0x3f5f;
	x |= (val&0x03)<<14;
	x |= (val&0x01)<<5;
	x |= (val&0x02)<<6;
	
	V5B_SifI2cWriteWord(0x20, x); 
}

void SensorSetContrast_MT9V111(UINT8 val)
{
	V5B_SifI2cWriteWord(0x01, 0x01); 
	switch(val)
	{	
	case 1:                 
		V5B_SifI2cWriteWord(0x53, 0x1e14);  //gamma0.6
		V5B_SifI2cWriteWord(0x54, 0x462e);  
		V5B_SifI2cWriteWord(0x55, 0x876a);  
		V5B_SifI2cWriteWord(0x56, 0xb7a0);  
		V5B_SifI2cWriteWord(0x57, 0xe0cc);  
		V5B_SifI2cWriteWord(0x58, 0x0000);  
		                          

		break;                    
        case 2:                                                    
		V5B_SifI2cWriteWord(0x53, 0x160d);  //gamma0.7
		V5B_SifI2cWriteWord(0x54, 0x3923);  
		V5B_SifI2cWriteWord(0x55, 0x7c5d);  
		V5B_SifI2cWriteWord(0x56, 0xb197);  
		V5B_SifI2cWriteWord(0x57, 0xe0c9);  
		V5B_SifI2cWriteWord(0x58, 0x0000);  

		break;
	case 3:                                
		V5B_SifI2cWriteWord(0x53, 0x1009);  //gamma0.8
		V5B_SifI2cWriteWord(0x54, 0x2f1b);  
		V5B_SifI2cWriteWord(0x55, 0x7252);  
		V5B_SifI2cWriteWord(0x56, 0xab8f);  
		V5B_SifI2cWriteWord(0x57, 0xe0c6);  
		V5B_SifI2cWriteWord(0x58, 0x0000);  

		break; 
        case 4:                                
		V5B_SifI2cWriteWord(0x53, 0x0d07);  //gamma0.85
		V5B_SifI2cWriteWord(0x54, 0x2b18);  
		V5B_SifI2cWriteWord(0x55, 0x6d4d);  
		V5B_SifI2cWriteWord(0x56, 0xa88b);  
		V5B_SifI2cWriteWord(0x57, 0xe0c4);  
		V5B_SifI2cWriteWord(0x58, 0x0000);  

		break;  
	case 5:                                
		V5B_SifI2cWriteWord(0x53, 0x0b06);  //gamma0.9
		V5B_SifI2cWriteWord(0x54, 0x2715);  
		V5B_SifI2cWriteWord(0x55, 0x6849);  
		V5B_SifI2cWriteWord(0x56, 0xa587);  
		V5B_SifI2cWriteWord(0x57, 0xe0c3);  
		V5B_SifI2cWriteWord(0x58, 0x0000);  

		break; 
	default:
		break;
	}
}

/******************************************************************
         Desc: set sensor Brightness  callback 
         Para: 1-5 step
*******************************************************************/
void SensorSetBrightness_MT9V111(UINT8 val)
{
	V5B_SifI2cWriteWord(0x01, 0x01);
	switch(val)
	{
	case 1:
		V5B_SifI2cWriteWord(0x2e, 0x4c);
		break;	
	case 2:
		V5B_SifI2cWriteWord(0x2e, 0x5c);
		break;
	case 3:
		V5B_SifI2cWriteWord(0x2e, 0x6c);
		break;
	case 4:
		V5B_SifI2cWriteWord(0x2e, 0x7c);
		break;
	case 5:
		V5B_SifI2cWriteWord(0x2e, 0x8c);
		break;
	default:
		break;		
	}
}

/******************************************************************
         Desc: set sensor mode  callback 
         Para:  val: 	1 50hz,	2 60hz,	3 night, 4 outdoor.
*******************************************************************/
void SensorSetMode_MT9V111(UINT8 val)
{
	V5B_SifI2cWriteWord(0x01, 0x0001);
	switch (val)
	{	
		case 1:
			V5B_SifI2cWriteWord(0x5b, 0x0003);	//50Hz 
			V5B_SifI2cWriteWord(0x37, 0x00a0);	//20-27fps
			break;
		case 2:
//			V5B_SifI2cWriteWord(0x01, 0x0001);	
			V5B_SifI2cWriteWord(0x5b, 0x0001);	//60Hz 
			V5B_SifI2cWriteWord(0x37, 0x00c0);	//20-27fps
			break;
		case 4:  // 3 and 4 reverse
//			V5B_SifI2cWriteWord(0x01, 0x0001);
			V5B_SifI2cWriteWord(0x37, 0x0100);	//night lowest 12.5fps	
			break;
		case 3:	
//			V5B_SifI2cWriteWord(0x01, 0x0001);	
			V5B_SifI2cWriteWord(0x37, 0x0080);	//outdoor 25fps	uper
			break;
		default:
			break;
	}
}
const TReg gSifPwrOnSeqRegVal_MT9V111[] =
{
	{0x00, 0x63,	1},
	{0x00, 0x62,	1},
	{0x00, 0x63,	1}
};

const TReg gSifRegVal_MT9V111[] =
{        
         {0x01,  0x41, 0},
	 {0x02,  0x00, 0},
	 {0x03,  0x0a, 0},
	 {0x04,  0x04, 0},
	 {0x2c,  0x02, 0},
	 {0x2d,  0x80, 0},
	 {0x2e,  0x01, 0},
	 {0x2f,  0xe0, 0}, 
};

const TReg gSnrSizeRegValVGA_MT9V111[] = 
{        
        //tuning image quality by ywq 2007-01-23
	 {0x01, 0x0004, 0},                
	 {0x0d, 0x0001, 30},             
	 {0x0d, 0x0000, 0},                
	 {0x31, 0x002a, 0},                
	 {0x01, 0x0001, 0},                
	 {0x34, 0x0000, 0},  
	 {0x3d, 0x01da, 0},
	 {0x01, 0x0004, 0}, //sensor core    
	 {0x05, 0x007b, 0},                
	 {0x06, 0x000e, 0},                
	 {0x07, 0x3002, 0},                
	 {0x01, 0x0001, 0}, //IFP   
//	 {0x2e, 0x1077, 0},	//AE target 
	 {0x2e, 0x107b, 0},	//AE target 
	 {0x38, 0x00a0, 0},	//changed for lowest frame rate=20fps in normal mode 2006-12-13
	 {0x38, 0x0878, 0},
	 {0x39, 0x014b, 1},	//flicker free
	 {0x59, 0x021f, 1},  
	 {0x5a, 0x028c, 1},
	 {0x5b, 0x0003, 1},
	 {0x5c, 0x2321, 1},  
	 {0x5d, 0x2a28, 1},
	
	 {0x53, 0x0a06, 1},	//gamma0.8, contrast1.65	2007-01-30
	 {0x54, 0x2212, 1},
	 {0x55, 0x7346, 1},
	 {0x56, 0xba9c, 1},
	 {0x57, 0xe0cf, 1},   
	 {0x58, 0x0000, 1},
};

const TSnrSizeCfg gSensorSizeCfg_MT9V111[] = 
{
	{
     		{0,NULL},
             	{0,NULL},
		{640, 480},
		1,
		1


	}

};

const TSnrInfo gSensorInfo_MT9V111 = 
{
       "MT9V111 yuv mode",
	VIM_SNR_YUV,							//yuv mode
	0,							//not use PCLK of sensor
	0,							//[0]: VSync delay enable; [1]: Fifo VSync delay enable; [2]: Use PCLK Negtive Edge.
	0,							//0 -- I2C, 1 -- Serial bus;
	0,							//[0]: 1 -- OV I2C bus
	80,							//I2C Rate : 100KHz
	0xb8,							//I2C address
	0xb8,							//ISP I2C address for special sensor
	0x03,							//power config
	0,							//reset sensor
                                                		
	3,							//brightness 
	3,							//contrast

	{0,NULL},											//sensor standby                                                                                  																									
	//{0,NULL},											//sensor initial setting                                                                                													
       {sizeof(gSnrSizeRegValVGA_MT9V111) / sizeof(TReg), (PTReg)&gSnrSizeRegValVGA_MT9V111},                                                       //snr initial value
	{sizeof(gSifPwrOnSeqRegVal_MT9V111) / sizeof(TReg), (PTReg)&gSifPwrOnSeqRegVal_MT9V111},	//Sif config sequence(Reg.800 bit0~2) when sensor power on                                              	
	{0, NULL},											//Sif config sequence(Reg.800 bit0~2) when sensor standby                                               					
	{sizeof(gSifRegVal_MT9V111) / sizeof(TReg), (PTReg)&gSifRegVal_MT9V111},			//sif initial value                                                                                     	
	{sizeof(gSensorSizeCfg_MT9V111) / sizeof(TSnrSizeCfg), (PTSnrSizeCfg)&gSensorSizeCfg_MT9V111},	//size configure                                                                                        	

	SensorSetReg_MT9V111,																				
	SensorGetReg_MT9V111,																				
	CheckId_MT9V111,																				
	SensorSetMirrorFlip_MT9V111,																						
	SensorSetContrast_MT9V111,
	SensorSetBrightness_MT9V111,	
	NULL,	
	SensorSetMode_MT9V111,	
	NULL,																									
	NULL,																									
	{	//ythd of flash,et of flash
		0x20,			10
	},
};

#endif

/******************************************************************
	Transform from 568(v4.2d) RDK sensor.c at 2006-11-20 
	Use gc's sensor setting at 2006-12-7 9:50
******************************************************************/
#if V5_DEF_SNR_OV7649_YUV

void SensorSetReg_OV7649(UINT16 uAddr, UINT16 uVal)
{
         V5B_SifI2cWriteByte((UINT8)uAddr, (UINT8)uVal);
}

void SensorGetReg_OV7649(UINT8 uAddr, UINT16 *uVal)
{
	V5B_SifI2cReadByte((UINT8)uAddr, (UINT8*)uVal);
}

UINT8 CheckId_OV7649(void)
{
	UINT8 temp1;
	UINT8 temp2; 


        V5B_SifI2cReadByte(0x0a, &temp1);
	V5B_SifI2cReadByte(0x0b, &temp2);    
        if((temp1==0x76) && (temp2==0x48))
                return SUCCEED; 
        else 
                return FAILED; 
}

void SensorSetMirrorFlip_OV7649(UINT8 val)
{
 	UINT8 x1,x2;
	if (val==1)
		val=2;
	else if (val==2)
	  	val=1;
	V5B_SifI2cReadByte(0x12,&x1); 
	V5B_SifI2cReadByte(0x75,&x2);
	
	V5B_SifI2cWriteByte(0x12,(UINT8)((x1&0xbf)|((val&0x01)<<6)));
	V5B_SifI2cWriteByte(0x75,(UINT8)((x2&0x7f)|((val&0x02)<<6)));
}

const TReg gSifPwrOnSeqRegVal_OV7649[] =
{
	{0x00, 0x66,	1},
	{0x00, 0x67,	1},
	{0x00, 0x66,	1}
};

const TReg gSifRegVal_OV7649[] =
{        
        {0x08, 	0x01,	0},
	{0x09, 	0x0c,	0},
//	{0x02, 	0x02,	0},
	{0x02, 	0x00,	0},
	{0x03, 	0x1b,	0},
	{0x04, 	0x05,	0},
	{0x2c, 	0x02,	0},
 	{0x2d, 	0x80,	0},
 	{0x2e, 	0x01,	0},
 	{0x2f, 	0xe0,	0},
 	{0x40, 	0x01,	0},
	{0x41, 	0xd0,	0},
	{0x42, 	0x01,	0},
	{0x43, 	0xe0,	0},
	{0x01, 	0x45,	0}	
};

const TReg gSnrSizeRegValVGA_OV7649[] = 
{      
	{0x12, 0x80, 1}, //~1	;sensor soft reset
	{0x12, 0x54, 1}, //~1
	{0x65, 0x00, 0},   
	{0x67, 0x94, 0}, //  ;YUV matrix
	{0x68, 0x7a, 0}, 
	{0x69, 0x08, 0}, 
	{0x6c, 0x11, 0}, // ;color matrix r
	{0x6d, 0x33, 0}, // ;color matrix g
	{0x6e, 0x22, 0}, // ;color matrix b
	{0x6f, 0x00, 0}, 
	{0x74, 0x60, 0}, // ;AGC Maximum 8x gain
	{0x75, 0x06, 0}, // ;Auto banding filter
	{0x77, 0xc4, 0}, // ;AEC/AGC Control zone limit
	{0x20, 0xd0, 0}, // ;Edge enhancement enable
	{0x24, 0x80, 0}, // ;AE upper limit
	{0x25, 0x70, 0}, // ;AE lower limit
	{0x26, 0x22, 0}, // ;edge enhancement
	{0x2a, 0x11, 0}, 
	{0x2d, 0x05, 0}, 
	{0x2f, 0x9d, 0}, 
	{0x30, 0x00, 0}, 
	{0x31, 0xc4, 0}, 
	{0x60, 0x86, 0}, 
	{0x61, 0xe0, 0}, 
	{0x15, 0x14, 0}, 
	{0x1f, 0x41, 0}, 
	{0x03, 0xa4, 0}, //  ;saturation control
	{0x04, 0x30, 0}, //  ;hue control
	{0x05, 0x88, 0}, 
	{0x06, 0x60, 0}, //  ;brightness
	{0x75, 0x8e, 0}, 
	{0x2A, 0x91, 0}, 
	{0x2B, 0x98, 0},
//	{0x2B, 0x00, 0},  
	{0x13, 0xa3, 0}, 	
};

const TSnrSizeCfg gSensorSizeCfg_OV7649[] = 
{
	{
          	{0,NULL},
              	{0,NULL},
	      	{640, 480},
		1,
		1

	}

};

const TSnrInfo gSensorInfo_OV7649 = 
{
       "OV7649 yuv mode",
	VIM_SNR_YUV,							//yuv mode
	0,							//not use PCLK of sensor
	0,							//[0]: VSync delay enable; [1]: Fifo VSync delay enable; [2]: Use PCLK Negtive Edge.
	0,							//0 -- I2C, 1 -- Serial bus;
	1,							//[0]: 1 -- OV I2C bus
	80,							//I2C Rate : 100KHz
	0x42,							//I2C address
	0x42,							//ISP I2C address for special sensor
	0x06,							//power config
	0,							//reset sensor
	3,							//brightness 
	3,							//contrast

	{0,NULL},											//sensor standby                                                                                  											
       {sizeof(gSnrSizeRegValVGA_OV7649) / sizeof(TReg), (PTReg)&gSnrSizeRegValVGA_OV7649},                                                       //snr initial value
	{sizeof(gSifPwrOnSeqRegVal_OV7649) / sizeof(TReg), (PTReg)&gSifPwrOnSeqRegVal_OV7649},		//Sif config sequence(Reg.800 bit0~2) when sensor power on                                              
	{0, NULL},											//Sif config sequence(Reg.800 bit0~2) when sensor standby                                               
	{sizeof(gSifRegVal_OV7649) / sizeof(TReg), (PTReg)&gSifRegVal_OV7649},				//sif initial value                                                                                     
	{sizeof(gSensorSizeCfg_OV7649) / sizeof(TSnrSizeCfg), (PTSnrSizeCfg)&gSensorSizeCfg_OV7649},	//size configure                                                                                        

	SensorSetReg_OV7649,										
	SensorGetReg_OV7649,										
	CheckId_OV7649,											
	SensorSetMirrorFlip_OV7649,									
	NULL,
	NULL,
	NULL,
	NULL,												
	NULL,												
	NULL,												
	{	//ythd of flash,et of flash
		0x20,			10
	},
};

#endif
#if V5_DEF_SNR_OV7660_YUV

void SensorSetReg_OV7660(UINT16 uAddr, UINT16 uVal)
{
	V5B_SifI2cWriteByte((UINT8)uAddr, (UINT8)uVal);
}

 

void SensorGetReg_OV7660(UINT8 uAddr, UINT16 *uVal)
{
	V5B_SifI2cReadByte((UINT8)uAddr, (UINT8*)uVal);
}

 

UINT8 CheckId_OV7660(void)
{
    UINT8 temp1;
    UINT8 temp2; 


	V5B_SifI2cReadByte(0x0a, &temp1);
	V5B_SifI2cReadByte(0x0b, &temp2);    
	if((temp1==0x76) && (temp2==0x60))
		return SUCCEED; 
	else 
 		return FAILED; 
}

 

/******************************************************************

         Desc: set sensor mirror and flip callback 

         Para: BIT0 -- mirror   BIT1 -- flip

*******************************************************************/

void SensorSetMirrorFlip_OV7660(UINT8 val)

{

        UINT8 x;

    val=~val;

    V5B_SifI2cReadByte(0x1e, &x);

    V5B_SifI2cWriteByte(0x1e, (x&0xcf)|((val&0x03)<<4));

}

 

/******************************************************************

         Desc: set sensor contrast  callback 

         Para: 1-5 step

*******************************************************************/

void SensorSetContrast_OV7660(UINT8 val)

{
    switch(val)
    {        
    case 1:                 
        V5B_SifI2cWriteByte(0x13, 0xe2);  //

      				
		V5B_SifI2cWriteByte(0x6c, 0x20);  
		V5B_SifI2cWriteByte(0x6d, 0x50);  
		V5B_SifI2cWriteByte(0x6e, 0x80);  
		V5B_SifI2cWriteByte(0x6f, 0xc0);  
		V5B_SifI2cWriteByte(0x70, 0xc0);  
		V5B_SifI2cWriteByte(0x71, 0xa0);  
		V5B_SifI2cWriteByte(0x72, 0x90);  
		V5B_SifI2cWriteByte(0x73, 0x78);  
		V5B_SifI2cWriteByte(0x74, 0x78);  
		V5B_SifI2cWriteByte(0x75, 0x78);  
		V5B_SifI2cWriteByte(0x76, 0x40);  
		V5B_SifI2cWriteByte(0x77, 0x20);  
		V5B_SifI2cWriteByte(0x78, 0x20);  
		V5B_SifI2cWriteByte(0x79, 0x20);  
		V5B_SifI2cWriteByte(0x7a, 0x1e);  
		V5B_SifI2cWriteByte(0x7b, 0x09);
		
		
		V5B_SifI2cWriteByte(0x7c, 0x02);
		V5B_SifI2cWriteByte(0x7d, 0x07);
		V5B_SifI2cWriteByte(0x7e, 0x17);
		V5B_SifI2cWriteByte(0x7f, 0x47);
		V5B_SifI2cWriteByte(0x80, 0x5f);
		V5B_SifI2cWriteByte(0x81, 0x73);
		V5B_SifI2cWriteByte(0x82, 0x85);
		V5B_SifI2cWriteByte(0x83, 0x94);
		V5B_SifI2cWriteByte(0x84, 0xa3);
		V5B_SifI2cWriteByte(0x85, 0xb2);
		V5B_SifI2cWriteByte(0x86, 0xc2);
		V5B_SifI2cWriteByte(0x87, 0xca);
		V5B_SifI2cWriteByte(0x88, 0xda);
		V5B_SifI2cWriteByte(0x89, 0xea);
		V5B_SifI2cWriteByte(0x8a, 0xf9);
		break;  
        case 2:                                                    
        V5B_SifI2cWriteByte(0x13, 0xe2);  //

		 
        V5B_SifI2cWriteByte(0x6c, 0x20);  
		V5B_SifI2cWriteByte(0x6d, 0x40);  
		V5B_SifI2cWriteByte(0x6e, 0x80);  
		V5B_SifI2cWriteByte(0x6f, 0x90);  
		V5B_SifI2cWriteByte(0x70, 0x90);  
		V5B_SifI2cWriteByte(0x71, 0x80);  
		V5B_SifI2cWriteByte(0x72, 0x70);  
		V5B_SifI2cWriteByte(0x73, 0x70);  
		V5B_SifI2cWriteByte(0x74, 0x60);  
		V5B_SifI2cWriteByte(0x75, 0x50);  
		V5B_SifI2cWriteByte(0x76, 0x40);  
		V5B_SifI2cWriteByte(0x77, 0x40);  
		V5B_SifI2cWriteByte(0x78, 0x30);  
		V5B_SifI2cWriteByte(0x79, 0x28);  
		V5B_SifI2cWriteByte(0x7a, 0x20);  
		V5B_SifI2cWriteByte(0x7b, 0x1d);
		
		
		V5B_SifI2cWriteByte(0x7c, 0x02);
		V5B_SifI2cWriteByte(0x7d, 0x06);
		V5B_SifI2cWriteByte(0x7e, 0x16);
		V5B_SifI2cWriteByte(0x7f, 0x3a);
		V5B_SifI2cWriteByte(0x80, 0x4c);
		V5B_SifI2cWriteByte(0x81, 0x5c);
		V5B_SifI2cWriteByte(0x82, 0x6a);
		V5B_SifI2cWriteByte(0x83, 0x78);
		V5B_SifI2cWriteByte(0x84, 0x84);
		V5B_SifI2cWriteByte(0x85, 0x8e);
		V5B_SifI2cWriteByte(0x86, 0x9e);
		V5B_SifI2cWriteByte(0x87, 0xae);
		V5B_SifI2cWriteByte(0x88, 0xc2);
		V5B_SifI2cWriteByte(0x89, 0xda);
		V5B_SifI2cWriteByte(0x8a, 0xea);
		break;
	case 3:                                
		
        V5B_SifI2cWriteByte(0x13, 0xe2);  // Close AE


        
		V5B_SifI2cWriteByte(0x6c, 0x40);  
		V5B_SifI2cWriteByte(0x6d, 0x30);  
		V5B_SifI2cWriteByte(0x6e, 0x4b);  
		V5B_SifI2cWriteByte(0x6f, 0x60);  
		V5B_SifI2cWriteByte(0x70, 0x70);  
		V5B_SifI2cWriteByte(0x71, 0x70);  
		V5B_SifI2cWriteByte(0x72, 0x70);  
		V5B_SifI2cWriteByte(0x73, 0x70);  
		V5B_SifI2cWriteByte(0x74, 0x60);  
		V5B_SifI2cWriteByte(0x75, 0x60);  
		V5B_SifI2cWriteByte(0x76, 0x50);  
		V5B_SifI2cWriteByte(0x77, 0x48);  
		V5B_SifI2cWriteByte(0x78, 0x3a);  
		V5B_SifI2cWriteByte(0x79, 0x2e);  
		V5B_SifI2cWriteByte(0x7a, 0x28);  
		V5B_SifI2cWriteByte(0x7b, 0x22);  
		V5B_SifI2cWriteByte(0x7c, 0x04);  
		V5B_SifI2cWriteByte(0x7d, 0x07);  
		V5B_SifI2cWriteByte(0x7e, 0x10);  
		V5B_SifI2cWriteByte(0x7f, 0x28);  
		V5B_SifI2cWriteByte(0x80, 0x36);  
		V5B_SifI2cWriteByte(0x81, 0x44);  
		V5B_SifI2cWriteByte(0x82, 0x52);  
		V5B_SifI2cWriteByte(0x83, 0x60);  
		V5B_SifI2cWriteByte(0x84, 0x6c);  
		V5B_SifI2cWriteByte(0x85, 0x78);  
		V5B_SifI2cWriteByte(0x86, 0x8c);  
		V5B_SifI2cWriteByte(0x87, 0x9e);  
		V5B_SifI2cWriteByte(0x88, 0xbb);  
		V5B_SifI2cWriteByte(0x89, 0xd2);  
		V5B_SifI2cWriteByte(0x8a, 0xe6);  
		break;  
	case 4:   
		
        V5B_SifI2cWriteByte(0x13, 0xe2);  //

		V5B_SifI2cWriteByte(0x6c, 0x40);  
		V5B_SifI2cWriteByte(0x6d, 0x50);  
		V5B_SifI2cWriteByte(0x6e, 0x50);  
		V5B_SifI2cWriteByte(0x6f, 0x58);  
		V5B_SifI2cWriteByte(0x70, 0x60);  
		V5B_SifI2cWriteByte(0x71, 0x60);  
		V5B_SifI2cWriteByte(0x72, 0x60);  
		V5B_SifI2cWriteByte(0x73, 0x60);  
		V5B_SifI2cWriteByte(0x74, 0x58);  
		V5B_SifI2cWriteByte(0x75, 0x58);  
		V5B_SifI2cWriteByte(0x76, 0x58);  
		V5B_SifI2cWriteByte(0x77, 0x48);  
		V5B_SifI2cWriteByte(0x78, 0x40);  
		V5B_SifI2cWriteByte(0x79, 0x30);  
		V5B_SifI2cWriteByte(0x7a, 0x28);  
		V5B_SifI2cWriteByte(0x7b, 0x26);  
		
		
		V5B_SifI2cWriteByte(0x7c, 0x04);  
		V5B_SifI2cWriteByte(0x7d, 0x09);  
		V5B_SifI2cWriteByte(0x7e, 0x13);  
		V5B_SifI2cWriteByte(0x7f, 0x29);  
		V5B_SifI2cWriteByte(0x80, 0x35);  
		V5B_SifI2cWriteByte(0x81, 0x41);  
		V5B_SifI2cWriteByte(0x82, 0x4d);  
		V5B_SifI2cWriteByte(0x83, 0x59);  
		V5B_SifI2cWriteByte(0x84, 0x64);  
		V5B_SifI2cWriteByte(0x85, 0x6f);  
		V5B_SifI2cWriteByte(0x86, 0x85);  
		V5B_SifI2cWriteByte(0x87, 0x97);  
		V5B_SifI2cWriteByte(0x88, 0xb7);  
		V5B_SifI2cWriteByte(0x89, 0xcf);  
		V5B_SifI2cWriteByte(0x8a, 0xe3);  


		
		break; 
	case 5:                                
		V5B_SifI2cWriteByte(0x13, 0xe2);  //
		
		V5B_SifI2cWriteByte(0x6c, 0x40);
		V5B_SifI2cWriteByte(0x6d, 0x50);
		V5B_SifI2cWriteByte(0x6e, 0x58);
		V5B_SifI2cWriteByte(0x6f, 0x50);
		V5B_SifI2cWriteByte(0x70, 0x50);
		V5B_SifI2cWriteByte(0x71, 0x50);
		V5B_SifI2cWriteByte(0x72, 0x50);
		V5B_SifI2cWriteByte(0x73, 0x50);
		V5B_SifI2cWriteByte(0x74, 0x48);
		V5B_SifI2cWriteByte(0x75, 0x48);
		V5B_SifI2cWriteByte(0x76, 0x48);
		V5B_SifI2cWriteByte(0x77, 0x48);
		V5B_SifI2cWriteByte(0x78, 0x40);
		V5B_SifI2cWriteByte(0x79, 0x40);
		V5B_SifI2cWriteByte(0x7a, 0x30);
		V5B_SifI2cWriteByte(0x7b, 0x2e);
		
		
		V5B_SifI2cWriteByte(0x7c, 0x04);
		V5B_SifI2cWriteByte(0x7d, 0x09);
		V5B_SifI2cWriteByte(0x7e, 0x14);
		V5B_SifI2cWriteByte(0x7f, 0x28);
		V5B_SifI2cWriteByte(0x80, 0x32);
		V5B_SifI2cWriteByte(0x81, 0x3c);
		V5B_SifI2cWriteByte(0x82, 0x46);
		V5B_SifI2cWriteByte(0x83, 0x4f);
		V5B_SifI2cWriteByte(0x84, 0x58);
		V5B_SifI2cWriteByte(0x85, 0x61);
		V5B_SifI2cWriteByte(0x86, 0x73);
		V5B_SifI2cWriteByte(0x87, 0x85);
		V5B_SifI2cWriteByte(0x88, 0xa5);
		V5B_SifI2cWriteByte(0x89, 0xc5);
		V5B_SifI2cWriteByte(0x8a, 0xdd);
             break; 
	default:
		break;

    }

}

 

/******************************************************************

         Desc: set sensor Brightness  callback 

         Para: 1-5 step

*******************************************************************/

void SensorSetBrightness_OV7660(UINT8 val)
{
    switch(val)
    {
    case 1:
             V5B_SifI2cWriteByte(0x13, 0xe7);  //
		     V5B_SifI2cWriteByte(0x24, 0x48);
             V5B_SifI2cWriteByte(0x25, 0x38);
             break;       
    case 2:
             V5B_SifI2cWriteByte(0x13, 0xe7);  //
		     V5B_SifI2cWriteByte(0x24, 0x58);
             V5B_SifI2cWriteByte(0x25, 0x48);
             break;
    case 3:
             V5B_SifI2cWriteByte(0x13, 0xe7);  //
		     V5B_SifI2cWriteByte(0x24, 0x75);
             V5B_SifI2cWriteByte(0x25, 0x68);
             break;
    case 4:
             V5B_SifI2cWriteByte(0x13, 0xe7);  //
		     V5B_SifI2cWriteByte(0x24, 0x98);
             V5B_SifI2cWriteByte(0x25, 0x88);
             break;
    case 5:
             V5B_SifI2cWriteByte(0x13, 0xe7);  //
		     V5B_SifI2cWriteByte(0x24, 0xb8);
             V5B_SifI2cWriteByte(0x25, 0xa8);
             break;

    default:

             break;                 

    }

}

 

/******************************************************************

         Desc: set sensor mode  callback 

         Para:  val:     1 50hz,      2 60hz,      3 night, 4 outdoor.

*******************************************************************/

void SensorSetMode_OV7660(UINT8 val)
{
    UINT8 uTemp;
    switch (val)
    {        
             case 1:
                       V5B_SifI2cWriteByte(0x3b, 0x0a);      //50Hz 
                       V5B_SifI2cWriteByte(0x92, 0x66);      //25fps
                       V5B_SifI2cWriteByte(0x93, 0x00);
                       V5B_SifI2cWriteByte(0x13, 0xe2);
                       V5B_SifI2cWriteByte(0x2d, 0x00);
                       V5B_SifI2cWriteByte(0x2e, 0x00);
                       V5B_SifI2cWriteByte(0x9d, 0x4c);     
                       V5B_SifI2cWriteByte(0x9e, 0x3f);
                       V5B_SifI2cWriteByte(0x13, 0xe7);
                       break;

             case 2:
                       V5B_SifI2cWriteByte(0x3b, 0x02);      //60Hz 
                       V5B_SifI2cWriteByte(0x92, 0x00);      //30fps
                       V5B_SifI2cWriteByte(0x93, 0x00);
                       V5B_SifI2cWriteByte(0x13, 0xe2);
                       V5B_SifI2cWriteByte(0x2d, 0x00);
                       V5B_SifI2cWriteByte(0x2e, 0x00);
					   V5B_SifI2cWriteByte(0x9d, 0x4c);     
                       V5B_SifI2cWriteByte(0x9e, 0x3f);
                       V5B_SifI2cWriteByte(0x13, 0xe7);
                       break;
             case 4:   // 3 and 4 reverse
                       V5B_SifI2cReadByte(0x3b, &uTemp);       //night  min12.5fps
                       V5B_SifI2cWriteByte(0x3b, (UINT8)((0xa0)|(uTemp&0x08)));     
                       break;

             case 3:     
                       V5B_SifI2cReadByte(0x3b, &uTemp);                 //outdoor
                       V5B_SifI2cWriteByte(0x3b, (UINT8)((0x02)|(uTemp&0x08)));
                       V5B_SifI2cWriteByte(0x13, 0xe2);
                       V5B_SifI2cWriteByte(0x2d, 0x00);
                       V5B_SifI2cWriteByte(0x2e, 0x00);
                       V5B_SifI2cWriteByte(0x13, 0xe7);               
                       break;
             default:
                       break;
    }

}

 

const TReg gSifPwrOnSeqRegVal_OV7660[] =
{
    {0x00, 0x62, 1},
    {0x00, 0x63, 1},
    {0x00, 0x62, 1}
};

 

const TReg gSifRegVal_OV7660[] =
{        
    {0x02,    0x02,  0},
    {0x03,        0x08,  0},
    {0x04,        0x04,  0},
    {0x2c,        0x02,  0},
    {0x2d,        0x80,  0},
    {0x2e,        0x01,  0},
    {0x2f,         0xe0,  0},
    {0x01,        0x45,  0},
        {0x2c,  0x02,  0},
        {0x2d,  0x80,  0},
        {0x2e,  0x01,  0},
        {0x2f,  0xe0,  0},
};

 

/* Sensor size control configure info */

 

const TReg gSnrSizeRegValVGA_OV7660[] = 
{      
     {0x12, 0x80, 10},
     {0x11, 0x40, 1}, //changed by ywq 2006-10-26 14:33
	 {0x92, 0x66, 1}, //changed by ywq 2006-10-27 17:39 for dummy pixel = 0 2006-12-19 17:40;
	 {0x93, 0x00, 1},
	 {0x6a, 0x98, 1}, //50Hz for 24M
	 {0x9d, 0x98, 1}, //50Hz
	 {0x9e, 0x7e, 1}, //60Hz
//	 {0x6a, 0xa5, 1}, //50Hz for 26M
//	 {0x9d, 0xa5, 1}, //50Hz
//	 {0x9e, 0x8a, 1}, //60Hz	
	 {0x3b, 0x0a, 1}, 
	 {0x13, 0xf2, 1},
	 {0x14, 0x26, 1},
	 {0x10, 0x00, 1},
	 {0x00, 0x00, 1}, 

	 {0x13, 0xf5, 1}, //changed by ywq enable AWB	2006-10-9 15:03
	 {0x01, 0x88, 100},
	 {0x02, 0xa5, 100}, 
//	 {0x13, 0xf7, 3},	
	 {0x12, 0x00, 3},	 
	 {0x04, 0x00, 3},
	 {0x18, 0x01, 3},	 
	 {0x17, 0x13, 3},
	 {0x32, 0x92, 3},
	 {0x19, 0x02, 3}, 
	 {0x1a, 0x7a, 3},
	 {0x03, 0x00, 3},
	 {0x0e, 0x84, 3},
	 {0x0f, 0x62, 3}, 
	 {0x15, 0x02, 3},
	 {0x16, 0x02, 3},
	 {0x1b, 0x01, 3},
	 {0x1e, 0x01, 3},
//	 {0x29, 0x30, 3},
	 {0x29, 0x20, 3},
	 {0x33, 0x00, 3},
	 {0x34, 0x07, 3},
	 {0x35, 0x84, 3},
	 {0x36, 0x00, 3},
	 {0x38, 0x13, 3},
	 {0x39, 0x43, 3},
	 {0x3a, 0x00, 3},
	 {0x3c, 0x6c, 3},
	 {0x3d, 0x90, 3},
	 {0x3f, 0x29, 3},
//	 {0x3f, 0x26, 3},	//edge enhancement factor changed 2006-12-13 14:45
	 {0x40, 0xc1, 3},
	 {0x41, 0x20, 3},
//	 {0x41, 0x30, 3},	//edge enhancement changed 2006-12-13 14:45
	 {0x6b, 0x0a, 3},
	 {0xa1, 0xc8, 3},
	 {0x69, 0x80, 3},
	 {0x43, 0xf0, 3},
	 {0x44, 0x10, 3},
	 {0x45, 0x78, 3},
	 {0x46, 0xa8, 3},
	 {0x47, 0x60, 3},
	 {0x48, 0x80, 3},
	 {0x59, 0xba, 3}, 
	 {0x5a, 0x9a, 3},
	 {0x5b, 0x22, 3},
	 {0x5c, 0xb9, 3}, 
	 {0x5d, 0x9b, 3},
	 {0x5e, 0x10, 3}, 
	 {0x5f, 0xe0, 3},
	 {0x60, 0x85, 3},	 
	 {0x61, 0x60, 3},
	 {0x9f, 0x9d, 3},	 
	 {0xa0, 0xa0, 3},
	 {0x4f, 0x66, 3},
	 {0x50, 0x6b, 3}, 
	 {0x51, 0x05, 3},
	 {0x52, 0x19, 3},
	 {0x53, 0x40, 3},
	 {0x54, 0x59, 3}, 
	 {0x55, 0x40, 3},
	 {0x56, 0x40, 3},
	 {0x57, 0x40, 3},
	 {0x58, 0x0d, 3},
	 {0x8b, 0xcc, 3},
	 {0x8c, 0xcc, 3},
	 {0x8d, 0xcf, 3},
	 {0x6c, 0x40, 3},	//gamma
	 {0x6d, 0x30, 3},
	 {0x6e, 0x4b, 3},
	 {0x6f, 0x60, 3},
	 {0x70, 0x70, 3},
	 {0x71, 0x70, 3},
	 {0x72, 0x70, 1},
	 {0x73, 0x70, 1},
	 {0x74, 0x60, 1},
	 {0x75, 0x60, 1},
	 {0x76, 0x50, 1},
	 {0x77, 0x48, 1},
	 {0x78, 0x3a, 1},
	 {0x79, 0x2e, 1},
	 {0x7a, 0x28, 1},
	 {0x7b, 0x22, 1},
	 {0x7c, 0x04, 3},
	 {0x7d, 0x07, 1},
	 {0x7e, 0x10, 1},
	 {0x7f, 0x28, 1},
	 {0x80, 0x36, 1}, 
	 {0x81, 0x44, 1},
	 {0x82, 0x52, 1},
	 {0x83, 0x60, 1}, 
	 {0x84, 0x6c, 1},
	 {0x85, 0x78, 1}, 
	 {0x86, 0x8c, 1},
	 {0x87, 0x9e, 1},	 
	 {0x88, 0xbb, 1},
	 {0x89, 0xd2, 1},	 
	 {0x8a, 0xe6, 1},
//	 {0x14, 0x2e, 1},
	 {0x24, 0x78, 1}, 
	 {0x25, 0x68, 1},
/*	 {0x2a, 0x00, 1},	//2b=70 the right side has several dark columns in captured image
	 {0x2b, 0x70, 3},	//changed following settings for capturing video and mirror LCD 2006-12-19 15:59
	 {0x92, 0x19, 3},
	 {0x6a, 0x85, 3}, 	//50Hz for 24M
	 {0x9d, 0x85, 3}, 	//50Hz
	 {0x9e, 0x6f, 3}, 	//60Hz
//	 {0x6a, 0x91, 3}, 	//50Hz for 26M
//	 {0x9d, 0x91, 3}, 	//50Hz
//	 {0x9e, 0x78, 3}, 	//60Hz
*/	 {0x13, 0xf7, 1},
};

 /* Sensor size control configure info */

const TSnrSizeCfg gSensorSizeCfg_OV7660[] = 
{
    {
         	{0,NULL},
             {0,NULL},
             {640, 480},
		1,
		1
    }
};

 

const TSnrInfo gSensorInfo_OV7660 = 
{
       "OV7660 yuv mode",
    VIM_SNR_YUV,                                                              //yuv mode
    0,                                                              //not use PCLK of sensor
    0,                                                              //[0]: VSync delay enable; [1]: Fifo VSync delay enable; [2]: Use PCLK Negtive Edge.
    0,                                                              //0 -- I2C, 1 -- Serial bus;
    1,                                                              //[0]: 1 -- OV I2C bus
	100,							//I2C Rate : 100KHz
    0x42,                                                                 //I2C address
    0x42,                                                                 //ISP I2C address for special sensor
    0x02,                                                                 //power config
    0,                                                              //reset sensor
    3,                                                              //brightness 
	4,							//contrast
    {0,NULL},                                                                                                                          //sensor standby
    {sizeof(gSnrSizeRegValVGA_OV7660) / sizeof(TReg), (PTReg)&gSnrSizeRegValVGA_OV7660},                                                       //snr initial value
    {sizeof(gSifPwrOnSeqRegVal_OV7660) / sizeof(TReg), (PTReg)&gSifPwrOnSeqRegVal_OV7660},                   //Sif config sequence(Reg.800 bit0~2) when sensor power on
    {0, NULL},                                                                                                                                                                                                                          //Sif config sequence(Reg.800 bit0~2) when sensor standby
    {sizeof(gSifRegVal_OV7660) / sizeof(TReg), (PTReg)&gSifRegVal_OV7660},                                                 //sif initial value
    {sizeof(gSensorSizeCfg_OV7660) / sizeof(TSnrSizeCfg), (PTSnrSizeCfg)&gSensorSizeCfg_OV7660},         //size configure 
    SensorSetReg_OV7660,                                                                                                                                                                              //set reg callback
    SensorGetReg_OV7660,                                                                                                                                                                              //get reg callback
    CheckId_OV7660,                                                                                                                                                                                //Sensor ID Check Call
    SensorSetMirrorFlip_OV7660,                                                                                                                                                                                                                             //Set Sensor Mirror Flip Call
    SensorSetContrast_OV7660,
    SensorSetBrightness_OV7660,
    NULL,	
    SensorSetMode_OV7660,
    NULL,                                                                                                                                                                                                                                 //Sensor switch size callback
    NULL,                                                                                                                                                                                                                                 //set et callback
    {        //ythd of flash,et of flash
		0x20,			10
    },
};
#endif



#if V5_DEF_SNR_OV7670_YUV

/* write sensor register callback */

void SensorSetReg_OV7670(UINT16 uAddr, UINT16 uVal)

{
         V5B_SifI2cWriteByte((UINT8)uAddr, (UINT8)uVal);
}

void SensorGetReg_OV7670(UINT8 uAddr, UINT16 *uVal)
{
	V5B_SifI2cReadByte((UINT8)uAddr, (UINT8*)uVal);
}

UINT8 CheckId_OV7670(void)
{
	 UINT8 temp1;
         UINT8 temp2; 


         V5B_SifI2cReadByte(0x0a, &temp1);
         V5B_SifI2cReadByte(0x0b, &temp2);    
         if((temp1==0x76) && (temp2==0x73))
                 return SUCCEED; 
         else 
                 return FAILED; 
}

/******************************************************************

         Desc: set sensor mirror and flip callback 

         Para: BIT0 -- mirror

                     BIT1 -- flip

*******************************************************************/

void SensorSetMirrorFlip_OV7670(UINT8 val)
{
        UINT8 x;
	val=~val;
	V5B_SifI2cReadByte(0x1e, &x);
	V5B_SifI2cWriteByte(0x1e, (UINT8)(x&0xcf)|((val&0x03)<<4));
}

/******************************************************************

         Desc: set sensor contrast  callback 

         Para: 1-5 step

*******************************************************************/

void SensorSetContrast_OV7670(UINT8 val)
{
	switch(val)
	{	
	case 1:                 
		V5B_SifI2cWriteByte(0x7a, 0x1c);  
		V5B_SifI2cWriteByte(0x7b, 0x30);  
		V5B_SifI2cWriteByte(0x7c, 0x3F);  
		V5B_SifI2cWriteByte(0x7d, 0x54);  
		V5B_SifI2cWriteByte(0x7e, 0x6F);  
		V5B_SifI2cWriteByte(0x7f, 0x79);  
		V5B_SifI2cWriteByte(0x80, 0x82);  
		V5B_SifI2cWriteByte(0x81, 0x8B);  
		V5B_SifI2cWriteByte(0x82, 0x92);  
		V5B_SifI2cWriteByte(0x83, 0x99);  
		V5B_SifI2cWriteByte(0x84, 0xA0);  
		V5B_SifI2cWriteByte(0x85, 0xAC);  
		V5B_SifI2cWriteByte(0x86, 0xB7);  
		V5B_SifI2cWriteByte(0x87, 0xCA);  
		V5B_SifI2cWriteByte(0x88, 0xDB);  
		V5B_SifI2cWriteByte(0x89, 0xEB);
		break;  
	case 2:                                                    
		V5B_SifI2cWriteByte(0x7a, 0x20);  
		V5B_SifI2cWriteByte(0x7b, 0x20);  
		V5B_SifI2cWriteByte(0x7c, 0x2d);  
		V5B_SifI2cWriteByte(0x7d, 0x42);  
		V5B_SifI2cWriteByte(0x7e, 0x60);  
		V5B_SifI2cWriteByte(0x7f, 0x6b);  
		V5B_SifI2cWriteByte(0x80, 0x76);  
		V5B_SifI2cWriteByte(0x81, 0x80);  
		V5B_SifI2cWriteByte(0x82, 0x8a);  
		V5B_SifI2cWriteByte(0x83, 0x92);  
		V5B_SifI2cWriteByte(0x84, 0x99);  
		V5B_SifI2cWriteByte(0x85, 0xa6);  
		V5B_SifI2cWriteByte(0x86, 0xaf);  
		V5B_SifI2cWriteByte(0x87, 0xc4);  
		V5B_SifI2cWriteByte(0x88, 0xd7);  
		V5B_SifI2cWriteByte(0x89, 0xe8);  
		break;
   	case 3:                                
		V5B_SifI2cWriteByte(0x7a, 0x19);  
		V5B_SifI2cWriteByte(0x7b, 0x0c);  
		V5B_SifI2cWriteByte(0x7c, 0x18);  
		V5B_SifI2cWriteByte(0x7d, 0x2f);  
		V5B_SifI2cWriteByte(0x7e, 0x54);  
		V5B_SifI2cWriteByte(0x7f, 0x64);  
		V5B_SifI2cWriteByte(0x80, 0x71);  
		V5B_SifI2cWriteByte(0x81, 0x7d);  
		V5B_SifI2cWriteByte(0x82, 0x88);  
		V5B_SifI2cWriteByte(0x83, 0x91);  
		V5B_SifI2cWriteByte(0x84, 0x98);  
		V5B_SifI2cWriteByte(0x85, 0xa7);  
		V5B_SifI2cWriteByte(0x86, 0xb4);  
		V5B_SifI2cWriteByte(0x87, 0xcb);  
		V5B_SifI2cWriteByte(0x88, 0xde);  
		V5B_SifI2cWriteByte(0x89, 0xed); 
		break; 
        case 4:                                
		V5B_SifI2cWriteByte(0x7a, 0x19);  
		V5B_SifI2cWriteByte(0x7b, 0x09);  
		V5B_SifI2cWriteByte(0x7c, 0x13);  
		V5B_SifI2cWriteByte(0x7d, 0x29);  
		V5B_SifI2cWriteByte(0x7e, 0x4f);  
		V5B_SifI2cWriteByte(0x7f, 0x5f);  
		V5B_SifI2cWriteByte(0x80, 0x6e);  
		V5B_SifI2cWriteByte(0x81, 0x7b);  
		V5B_SifI2cWriteByte(0x82, 0x87);  
		V5B_SifI2cWriteByte(0x83, 0x91);  
		V5B_SifI2cWriteByte(0x84, 0x9a);  
		V5B_SifI2cWriteByte(0x85, 0xab);  
		V5B_SifI2cWriteByte(0x86, 0xb6);  
		V5B_SifI2cWriteByte(0x87, 0xcd);  
		V5B_SifI2cWriteByte(0x88, 0xdf);  
		V5B_SifI2cWriteByte(0x89, 0xed);
		break;  
	case 5:                                
		V5B_SifI2cWriteByte(0x7a, 0x2C);  
		V5B_SifI2cWriteByte(0x7b, 0x11);  
		V5B_SifI2cWriteByte(0x7c, 0x1a);  
		V5B_SifI2cWriteByte(0x7d, 0x2a);  
		V5B_SifI2cWriteByte(0x7e, 0x42);  
		V5B_SifI2cWriteByte(0x7f, 0x4c);  
		V5B_SifI2cWriteByte(0x80, 0x56);  
		V5B_SifI2cWriteByte(0x81, 0x5f);  
		V5B_SifI2cWriteByte(0x82, 0x67);  
		V5B_SifI2cWriteByte(0x83, 0x70);  
		V5B_SifI2cWriteByte(0x84, 0x78);  
		V5B_SifI2cWriteByte(0x85, 0x87);  
		V5B_SifI2cWriteByte(0x86, 0x95);  
		V5B_SifI2cWriteByte(0x87, 0xaf);  
		V5B_SifI2cWriteByte(0x88, 0xc8);  
		V5B_SifI2cWriteByte(0x89, 0xdf); 
		break; 
	default:
		break;
	}					
}

/******************************************************************

         Desc: set sensor Brightness  callback 

         Para: 1-5 step

*******************************************************************/

void SensorSetBrightness_OV7670(UINT8 val)
{
	switch(val)
	{
	case 1:
		V5B_SifI2cWriteByte(0x55, 0xc8);

		break;	
	case 2:
		V5B_SifI2cWriteByte(0x55, 0xb8);

		break;
	case 3:
		V5B_SifI2cWriteByte(0x55, 0xa8);

		break;
	case 4:
		V5B_SifI2cWriteByte(0x55, 0x98);

		break;
	case 5:
		V5B_SifI2cWriteByte(0x55, 0x88);

		break;
	default:
		break;		
	}
}

/******************************************************************

         Desc: set sensor mod  callback 

         Para:  val: 	1	50hz
         	    		2	60hz
         	     	    	3	night 
         	     	     	4	outdoor

*******************************************************************/

void SensorSetMode_OV7670(UINT8 val)
{
	UINT8 uTemp;
	switch (val)
	{	
		case 1:
            V5B_SifI2cWriteByte(0x3b, 0x0a);	//50Hz 
			V5B_SifI2cWriteByte(0x92, 0x19);	//25fps
			V5B_SifI2cWriteByte(0x13, 0xe2);
			V5B_SifI2cWriteByte(0x2d, 0x00);
			V5B_SifI2cWriteByte(0x2e, 0x00);
			V5B_SifI2cWriteByte(0x9d, 0x4c);
			V5B_SifI2cWriteByte(0x9e, 0x3f);
			V5B_SifI2cWriteByte(0x13, 0xe7);


			break;
		case 2:
		

            V5B_SifI2cWriteByte(0x3b, 0x02);	//60Hz 
			V5B_SifI2cWriteByte(0x92, 0x00);	//30fps
            V5B_SifI2cWriteByte(0x93, 0x00);
			V5B_SifI2cWriteByte(0x13, 0xe2);
			V5B_SifI2cWriteByte(0x2d, 0x00);
			V5B_SifI2cWriteByte(0x2e, 0x00);
			V5B_SifI2cWriteByte(0x9d, 0x4c);
			V5B_SifI2cWriteByte(0x9e, 0x3f);
			V5B_SifI2cWriteByte(0x13, 0xe7);
			break;
		case 4:  // 3 and 4 reverse
			V5B_SifI2cReadByte(0x3b, &uTemp);	//night  min12.5fps
			V5B_SifI2cWriteByte(0x3b, (UINT8)((0xa0)|(uTemp&0x08)));
			V5B_SifI2cWriteByte(0x92, 0x10);  
            V5B_SifI2cWriteByte(0x92, 0x10);  
			
			break;
		case 3:	
			V5B_SifI2cReadByte(0x3b, &uTemp);		//outdoor
			V5B_SifI2cWriteByte(0x3b, (UINT8)((0x02)|(uTemp&0x08)));
			V5B_SifI2cWriteByte(0x13, 0xe2);
			V5B_SifI2cWriteByte(0x2d, 0x00);
			V5B_SifI2cWriteByte(0x2e, 0x00);
			V5B_SifI2cWriteByte(0x13, 0xe7);		
			break;
		default:
			break;
	}
}

/******************************************************************

         Desc: get sensor ET  callback 			//add by guoying 9/6/2007
       
         Para:  val: 	*RegBuf					//the data buffer pointer
*******************************************************************/
void SensorGetET_OV7670(UINT8 *RegBuf)
{
	UINT8 uTemp;

#if 1
	//at first close the AE
	V5B_SifI2cReadByte(0x13, &uTemp);
	uTemp&=~(BIT0|BIT2);
	V5B_SifI2cWriteByte(0x13, uTemp);	

	
	V5B_SifI2cWriteByte(0x11,0x01);

        V5B_SifI2cWriteByte(0x6b,0x0a);

        V5B_SifI2cWriteByte(0x2a,0x00);

    	V5B_SifI2cWriteByte(0x2b,0x00);

    //V5B_SifI2cWriteByte(0x92,0x3a);
    	V5B_SifI2cWriteByte(0x92,0x3a);

    	V5B_SifI2cWriteByte(0x93,0x00);

    //V5B_SifI2cWriteByte(0x9d,0x09);
    
    	V5B_SifI2cWriteByte(0x9d,0x11);
    
    	V5B_SifI2cWriteByte(0x9e,0x07);

    	V5B_SifI2cWriteByte(0xa5,0x63);

    	V5B_SifI2cWriteByte(0xab,0x77);


	V5B_SifI2cReadByte(0x13, &uTemp);
	uTemp|=(BIT0|BIT2);
	V5B_SifI2cWriteByte(0x13, uTemp);	
	//Delay(10);
#endif		
	return;
	
}
/******************************************************************

         Desc: set sensor ET  callback 		//add by guoying 9/6/2007
       
         Para:  val: 	*RegBuf				//the data buffer pointer
*******************************************************************/
void SensorSetET_OV7670(UINT8 *RegBuf, UINT8 div)
{

	V5B_SifI2cWriteByte(0x11,0x01);
        V5B_SifI2cWriteByte(0x6b,0x4a);
        V5B_SifI2cWriteByte(0x2a,0x00);
	V5B_SifI2cWriteByte(0x2b,0x00);
	V5B_SifI2cWriteByte(0x92,0x66);
	V5B_SifI2cWriteByte(0x93,0x00);
	V5B_SifI2cWriteByte(0x9d,0x7f);
	V5B_SifI2cWriteByte(0x9e,0x6a);
	V5B_SifI2cWriteByte(0xa5,0x05);
	V5B_SifI2cWriteByte(0xab,0x07);	



	return;
	
}

const TReg gSifPwrOnSeqRegVal_OV7670[] =
{
	{0x00, 0x63,	1},
	{0x00, 0x62,	1},
	{0x00, 0x63,	1}
};

const TReg gSifRegVal_OV7670[] =
{        
        {0x02, 0x02, 0},
        {0x03, 0x1b, 0},
        {0x04, 0x05, 0},
        {0x21, 0x0a, 0},
        {0x23, 0xea, 0},
        {0x2c, 0x02, 0},
        {0x2d, 0x80, 0},
        {0x2e, 0x01, 0},
        {0x2f, 0xe0, 0},
        {0x01, 0x45, 0} 
};


/* Sensor size control configure info */

const TReg gSnrSizeRegValVGA_OV7670[] = 
{      
   	
	{0x12, 0x80, 10},   // address , data, delay
	{0x11, 0x01, 3},
	{0x3a, 0x04, 1},
	{0x12, 0x00, 1},
	{0x17, 0x13, 1},
	{0x18, 0x01, 1},
	{0x32, 0xb6, 1}, 
	{0x19, 0x02, 1},
	{0x1a, 0x7a, 1},
	{0x03, 0x0a, 1}, 
	{0x0c, 0x00, 1},
	{0x3e, 0x00, 1}, 
	{0x70, 0x3a, 1},
	{0x71, 0x35, 1},     
	{0x72, 0x11, 1},
	{0x73, 0xf0, 1},     
	{0xa2, 0x02, 1},

     {0x7a, 0x19, 10},  // New Gamma
	 {0x7b, 0x3b, 10},
     {0x7c, 0x4b, 10},
     {0x7d, 0x60, 10},
     {0x7e, 0x7b, 10},
     {0x7f, 0x85, 10},
	 {0x80, 0x8e, 10},
     {0x81, 0x96, 10},
	 {0x82, 0x9d, 10},
     {0x83, 0xa3, 10},
	 {0x84, 0xa9, 10},
     {0x85, 0xb5, 10},
	 {0x86, 0xbf, 10},
     {0x87, 0xd0, 10},
     {0x88, 0xdf, 10},
     {0x89, 0xed, 10},

	{0x13, 0xe0, 1},
	{0x00, 0x00, 1},
	{0x10, 0x00, 1},
	{0x0d, 0x40, 1},
	{0x14, 0x18, 1},
	{0xa5, 0x05, 1},
	{0xab, 0x07, 1},
	
        {0x24, 0x75, 1},	
	{0x25, 0x45, 1},

	{0x26, 0xe3, 1},
	{0x55, 0xA8, 1},	
	
        {0x9f, 0x85, 1},   
	{0xa0, 0x75, 1},   

	{0xa1, 0x0b, 1},
	{0xa6, 0xd8, 1},
	{0xa7, 0xd8, 1},
	{0xa8, 0xf0, 1},
	{0xa9, 0x90, 1}, 
	{0xaa, 0x14, 1},      //{0xaa, 0x94, 1},  //average
	{0x13, 0xe5, 1},
	{0x0e, 0x61, 1}, 
	{0x0f, 0x4b, 1},
	{0x16, 0x02, 1}, 
	{0x1e, 0x37, 1},  // {0x1e, 0x07, 1}, Flip and Mirror
	{0x21, 0x02, 1},     
	{0x22, 0x91, 1},
	{0x29, 0x07, 1},     
	{0x33, 0x0b, 1},
	{0x35, 0x0b, 1},
	{0x37, 0x1d, 1}, 
	{0x38, 0x71, 1},
	{0x39, 0x2a, 1},
	{0x3c, 0x78, 1},
	{0x4d, 0x40, 1}, 
	{0x4e, 0x20, 1},
	{0x69, 0x00, 1},
	{0x6b, 0x4a, 1},
	{0x74, 0x19, 1},
	{0x8d, 0x4f, 1},
	{0x8e, 0x00, 1},
	{0x8f, 0x00, 1},
	{0x90, 0x00, 1},
	{0x91, 0x00, 1},
	{0x92, 0x19, 1},
	{0x96, 0x00, 1},
	{0x9a, 0x80, 3},
	{0xb0, 0x84, 3},
	{0xb1, 0x0c, 1},
	{0xb2, 0x0e, 1},
	{0xb3, 0x82, 1},
	{0xb8, 0x0a, 1},
	{0x43, 0x14, 1},
	{0x44, 0xf0, 1},
	{0x45, 0x34, 1},
	{0x46, 0x58, 1},
	{0x47, 0x28, 1},
	{0x48, 0x3a, 1},
	{0x59, 0x88, 3},
	{0x5a, 0x88, 1},
	{0x5b, 0x44, 1},
	{0x5c, 0x67, 1},
	{0x5d, 0x49, 1}, 
	{0x5e, 0x0e, 1},
	
    {0x62, 0x00, 1},	//Truly lens shading
	{0x63, 0x00, 1},
	{0x64, 0x0f, 1},
	{0x65, 0x10, 1}, 
	{0x66, 0x05, 1},
	{0x94, 0x0f, 1}, 
	{0x95, 0x1a, 1},
	
	{0x6c, 0x0a, 1},     
	{0x6d, 0x55, 1},
	{0x6e, 0x11, 1},     
	{0x6f, 0x9f, 1},
	{0x6a, 0x40, 1},
	{0x01, 0x40, 1}, 
	{0x02, 0x40, 1},
	{0x13, 0xe7, 1},
	
        {0x4f, 0xa9, 10},  
	{0x50, 0x9d, 10},
	{0x51, 0x0c, 10},
	{0x52, 0x1f, 10},
	{0x53, 0x84, 10},
	{0x54, 0xa3, 10},
	{0x58, 0x1e, 10},
	
	{0x41, 0x08, 1}, 
	{0x3f, 0x00, 1},
	{0x75, 0x03, 1},	
	{0x76, 0xe1, 1}, 
	{0x4c, 0x00, 1},
	{0x77, 0x01, 1}, 
	{0x3d, 0x82, 1},
	{0x4b, 0x09, 1},     
	{0xc9, 0x60, 1},
	{0x41, 0x38, 1},     
	{0x56, 0x60, 1},	
	{0x34, 0x11, 1},
	{0x3b, 0x08, 1}, 	
	{0xa4, 0x88, 1},
	{0x96, 0x00, 1},
	{0x97, 0x30, 1},
	{0x98, 0x20, 1}, 
	{0x99, 0x30, 1},
	{0x9a, 0x84, 1},
	{0x9b, 0x29, 1},
	{0x9c, 0x03, 1},
	{0x9d, 0x4c, 1},
	{0x9e, 0x3f, 1},
	{0x78, 0x04, 1},
	{0x79, 0x01, 1},
	{0xc8, 0xf0, 1},
	{0x79, 0x0f, 1},
	{0xc8, 0x00, 1},
	{0x79, 0x10, 1},
	{0xc8, 0x7e, 1},
	{0x79, 0x0a, 1},
	{0xc8, 0x80, 1},
	{0x79, 0x0b, 1},
	{0xc8, 0x01, 1},
	{0x79, 0x0c, 1},
	{0xc8, 0x0f, 1},
	{0x79, 0x0d, 1},
	{0xc8, 0x20, 1},
	{0x79, 0x09, 1},
	{0xc8, 0x80, 1},
	{0x79, 0x02, 1},
	{0xc8, 0xc0, 1},
	{0x79, 0x03, 1},
	{0xc8, 0x40, 1},
	{0x79, 0x05, 1}, 
	{0xc8, 0x30, 1},
	{0x79, 0x26, 1},
	{0xf1, 0x10, 1}, 
	{0x0f, 0x1d, 1},
	{0x0f, 0x1f, 1}, 
	{0x92, 0x66, 1}, 	
	{0x93, 0x00, 1},
	{0x9d, 0x7f, 1},
	{0x9e, 0x6a, 1}, 

};


/* Sensor size control configure info */

const TSnrSizeCfg gSensorSizeCfg_OV7670[] = 
{
	{
		
               {0,NULL},//{sizeof(gSizeRegSnrVal_OV7670) / sizeof(TReg), (PTReg)&gSizeRegSnrVal_OV7670},	//9/7/2007 add by guoying 
               {0,NULL},
	       {640, 480},
		1,
		1
	

	}

};

 

const TSnrInfo gSensorInfo_OV7670 = 

{
       "OV7670 yuv mode",
	VIM_SNR_YUV,							//yuv mode
	1,	//0x85c bit1  //High priority for 0x806				         //not use PCLK of sensor
	//0,							//guoying 1/23/2008, can't use this parameter, must use sensor pclk!
	0,							//[0]: VSync delay enable; [1]: Fifo VSync delay enable; [2]: Use PCLK Negtive Edge.
	0,							//0 -- I2C, 1 -- Serial bus;
	1,							//[0]: 1 -- OV I2C bus
	80,							//I2C Rate : 100KHz
	0x42,							//I2C address
	0x42,							//ISP I2C address for special sensor
	0x03,							//power config
	0,							//reset sensor

	3,							//brightness 
	3,							//contrast
	
	{0,NULL},														//sensor standby
       {sizeof(gSnrSizeRegValVGA_OV7670) / sizeof(TReg), (PTReg)&gSnrSizeRegValVGA_OV7670},                                                       //snr initial value
	
	{sizeof(gSifPwrOnSeqRegVal_OV7670) / sizeof(TReg), (PTReg)&gSifPwrOnSeqRegVal_OV7670},		//Sif config sequence(Reg.800 bit0~2) when sensor power on
	{0, NULL},																								//Sif config sequence(Reg.800 bit0~2) when sensor standby
	{sizeof(gSifRegVal_OV7670) / sizeof(TReg), (PTReg)&gSifRegVal_OV7670},						//sif initial value
																							//isp initial value
	{sizeof(gSensorSizeCfg_OV7670) / sizeof(TSnrSizeCfg), (PTSnrSizeCfg)&gSensorSizeCfg_OV7670},		//size configure 

	SensorSetReg_OV7670,																				//set reg callback
	SensorGetReg_OV7670,																				//get reg callback
	CheckId_OV7670,																				//Sensor ID Check Call
	SensorSetMirrorFlip_OV7670,																									//Set Sensor Mirror Flip Call
	SensorSetContrast_OV7670,																									//Sensor switch size callback
	SensorSetBrightness_OV7670,																									//set et callback
	NULL,	//set et callback
	SensorSetMode_OV7670,

	NULL,
	NULL,

	
	{	//ythd of flash,et of flash
		0x20,			10
	},
};

#endif


#if V5_DEF_SNR_HV7131GP_YUV	//HYCA3

void SensorSetReg_HV7131GP(UINT16 uAddr, UINT16 uVal)
{
         V5B_SifI2cWriteByte((UINT8)uAddr, (UINT8)uVal);
}

void SensorGetReg_HV7131GP(UINT8 uAddr, UINT16 *uVal)
{
	V5B_SifI2cReadByte((UINT8)uAddr, (UINT8*)uVal);
}

UINT8 CheckId_HV7131GP(void)
{
	UINT8 temp;


	V5B_SifI2cReadByte(0, &temp);
	if(temp==0x40) 
		return SUCCEED;	
	else
		return FAILED;
}

void SensorSetMirrorFlip_HV7131GP(UINT8 val)
{
        UINT8 x;
	val = ~val;
	V5B_SifI2cReadByte(0x01, &x);
	V5B_SifI2cWriteByte(0x01, (UINT8)((x&0xf3)|((val&0x03)<<2)));
}

void SensorSetContrast_HV7131GP(UINT8 val)
{
	switch(val)
	{	
	case 1:                 
		V5B_SifI2cWriteByte(0x40, 0x00);  
		V5B_SifI2cWriteByte(0x41, 0x04);  
		V5B_SifI2cWriteByte(0x42, 0x1c);  
		V5B_SifI2cWriteByte(0x43, 0x34);  
		V5B_SifI2cWriteByte(0x44, 0x54);  
		V5B_SifI2cWriteByte(0x45, 0x78);  
		V5B_SifI2cWriteByte(0x46, 0x90);  
		V5B_SifI2cWriteByte(0x47, 0xa4);  
		V5B_SifI2cWriteByte(0x48, 0xe0);  
		V5B_SifI2cWriteByte(0x49, 0xf4);  
		V5B_SifI2cWriteByte(0x50, 0x40);
		V5B_SifI2cWriteByte(0x51, 0x80);
		V5B_SifI2cWriteByte(0x52, 0x60);
		V5B_SifI2cWriteByte(0x53, 0x40);
		V5B_SifI2cWriteByte(0x54, 0x24);
		V5B_SifI2cWriteByte(0x55, 0x18);
		V5B_SifI2cWriteByte(0x56, 0x14);
		V5B_SifI2cWriteByte(0x57, 0x0f);
		V5B_SifI2cWriteByte(0x58, 0x05);
		V5B_SifI2cWriteByte(0x59, 0x02);
		break;  
	case 2:                                                    
		V5B_SifI2cWriteByte(0x40, 0x00);
		V5B_SifI2cWriteByte(0x41, 0x04);
		V5B_SifI2cWriteByte(0x42, 0x0a);
		V5B_SifI2cWriteByte(0x43, 0x12);
		V5B_SifI2cWriteByte(0x44, 0x20);
		V5B_SifI2cWriteByte(0x45, 0x39);
		V5B_SifI2cWriteByte(0x46, 0x52);
		V5B_SifI2cWriteByte(0x47, 0x66);
		V5B_SifI2cWriteByte(0x48, 0xae);
		V5B_SifI2cWriteByte(0x49, 0xdd);
		V5B_SifI2cWriteByte(0x50, 0x40);
		V5B_SifI2cWriteByte(0x51, 0x20);
		V5B_SifI2cWriteByte(0x52, 0x20);
		V5B_SifI2cWriteByte(0x53, 0x1c);
		V5B_SifI2cWriteByte(0x54, 0x19);
		V5B_SifI2cWriteByte(0x55, 0x19);
		V5B_SifI2cWriteByte(0x56, 0x14);
		V5B_SifI2cWriteByte(0x57, 0x12);
		V5B_SifI2cWriteByte(0x58, 0x0b);
		V5B_SifI2cWriteByte(0x59, 0x08);
		break;
   	case 3:                                
		V5B_SifI2cWriteByte(0x40, 0x00);
		V5B_SifI2cWriteByte(0x41, 0x00);
		V5B_SifI2cWriteByte(0x42, 0x02);
		V5B_SifI2cWriteByte(0x43, 0x08);
		V5B_SifI2cWriteByte(0x44, 0x14);
		V5B_SifI2cWriteByte(0x45, 0x2e);
		V5B_SifI2cWriteByte(0x46, 0x44);
		V5B_SifI2cWriteByte(0x47, 0x5a);
		V5B_SifI2cWriteByte(0x48, 0xa6);
		V5B_SifI2cWriteByte(0x49, 0xdd);
		V5B_SifI2cWriteByte(0x50, 0x00);
		V5B_SifI2cWriteByte(0x51, 0x0a);
		V5B_SifI2cWriteByte(0x52, 0x18);
		V5B_SifI2cWriteByte(0x53, 0x18);
		V5B_SifI2cWriteByte(0x54, 0x1a);
		V5B_SifI2cWriteByte(0x55, 0x16);
		V5B_SifI2cWriteByte(0x56, 0x16);
		V5B_SifI2cWriteByte(0x57, 0x13);
		V5B_SifI2cWriteByte(0x58, 0x0d);
		V5B_SifI2cWriteByte(0x59, 0x08);
		break; 
        case 4:                                
		V5B_SifI2cWriteByte(0x40, 0x00);
		V5B_SifI2cWriteByte(0x41, 0x00);
		V5B_SifI2cWriteByte(0x42, 0x01);
		V5B_SifI2cWriteByte(0x43, 0x06);
		V5B_SifI2cWriteByte(0x44, 0x10);
		V5B_SifI2cWriteByte(0x45, 0x26);
		V5B_SifI2cWriteByte(0x46, 0x3a);
		V5B_SifI2cWriteByte(0x47, 0x4e);
		V5B_SifI2cWriteByte(0x48, 0x94);
		V5B_SifI2cWriteByte(0x49, 0xdd);
		V5B_SifI2cWriteByte(0x50, 0x00);
		V5B_SifI2cWriteByte(0x51, 0x05);
		V5B_SifI2cWriteByte(0x52, 0x14);
		V5B_SifI2cWriteByte(0x53, 0x14);
		V5B_SifI2cWriteByte(0x54, 0x16);
		V5B_SifI2cWriteByte(0x55, 0x14);
		V5B_SifI2cWriteByte(0x56, 0x14);
		V5B_SifI2cWriteByte(0x57, 0x11);
		V5B_SifI2cWriteByte(0x58, 0x12);
		V5B_SifI2cWriteByte(0x59, 0x08);
		break;  
	case 5:                                
		V5B_SifI2cWriteByte(0x40, 0x00);
		V5B_SifI2cWriteByte(0x41, 0x00);
		V5B_SifI2cWriteByte(0x42, 0x00);
		V5B_SifI2cWriteByte(0x43, 0x02);
		V5B_SifI2cWriteByte(0x44, 0x0a);
		V5B_SifI2cWriteByte(0x45, 0x1c);
		V5B_SifI2cWriteByte(0x46, 0x2e);
		V5B_SifI2cWriteByte(0x47, 0x3e);
		V5B_SifI2cWriteByte(0x48, 0x86);
		V5B_SifI2cWriteByte(0x49, 0xdd);
		V5B_SifI2cWriteByte(0x50, 0x00);
		V5B_SifI2cWriteByte(0x51, 0x00);
		V5B_SifI2cWriteByte(0x52, 0x08);
		V5B_SifI2cWriteByte(0x53, 0x10);
		V5B_SifI2cWriteByte(0x54, 0x12);
		V5B_SifI2cWriteByte(0x55, 0x12);
		V5B_SifI2cWriteByte(0x56, 0x10);
		V5B_SifI2cWriteByte(0x57, 0x12);
		V5B_SifI2cWriteByte(0x58, 0x15);
		V5B_SifI2cWriteByte(0x59, 0x08);
		break; 
	default:
		break;
	}					
}

/******************************************************************
         Desc: set sensor Brightness  callback 
         Para: 1-5 step
*******************************************************************/
void SensorSetBrightness_HV7131GP(UINT8 val)
{
	switch(val)
	{
	case 1:
		V5B_SifI2cWriteByte(0x66, 0x60);
		break;	
	case 2:
		V5B_SifI2cWriteByte(0x66, 0x70);
		break;
	case 3:
		V5B_SifI2cWriteByte(0x66, 0x80);
		break;
	case 4:
		V5B_SifI2cWriteByte(0x66, 0x8a);
		break;
	case 5:
		V5B_SifI2cWriteByte(0x66, 0x94);
		break;
	default:
		break;		
	}
}

/******************************************************************
         Desc: set sensor mode  callback 
         Para:  val: 	1 50hz,	2 60hz,	3 night, 4 outdoor.
*******************************************************************/
void SensorSetMode_HV7131GP(UINT8 val)
{	
	UINT8 uTemp1,uTemp2,uTemp3;
	UINT32 uTemp;
	switch (val)
	{	
		case 1:
			V5B_SifI2cWriteByte(0x60, 0xb8);	//disable AE
			V5B_SifI2cWriteByte(0x6a, 0x01);	//50Hz 1 step
			V5B_SifI2cWriteByte(0x6b, 0xd4);
			V5B_SifI2cWriteByte(0x6c, 0xc0);
			V5B_SifI2cWriteByte(0x6d, 0x09);	//50Hz 20fps
			V5B_SifI2cWriteByte(0x6e, 0x27);
			V5B_SifI2cWriteByte(0x6f, 0xc0);
			V5B_SifI2cWriteByte(0x60, 0xb9);	//enable AE
			break;
		case 2:
			V5B_SifI2cWriteByte(0x60, 0xb8);
			V5B_SifI2cWriteByte(0x6a, 0x01);	//60Hz 1 step
			V5B_SifI2cWriteByte(0x6b, 0x86);
			V5B_SifI2cWriteByte(0x6c, 0xa0);
			V5B_SifI2cWriteByte(0x6d, 0x07);	//60Hz 20fps
			V5B_SifI2cWriteByte(0x6e, 0xa1);
			V5B_SifI2cWriteByte(0x6f, 0x20);
			V5B_SifI2cWriteByte(0x60, 0xb9);
			break;
		case 4:  // 3 and 4 reverse
			V5B_SifI2cWriteByte(0x60, 0xb8);
			V5B_SifI2cReadByte(0x6a, &uTemp1);	
			V5B_SifI2cReadByte(0x6b, &uTemp2);
			V5B_SifI2cReadByte(0x6c, &uTemp3);
			uTemp = (UINT32)((uTemp1<<16)|(uTemp2<<8)|(uTemp3));
			if (uTemp == 0x01d4c0)				//50Hz 1 step			
			{
				V5B_SifI2cWriteByte(0x6d, 0x12);
				V5B_SifI2cWriteByte(0x6e, 0x4f);
				V5B_SifI2cWriteByte(0x6f, 0x80);	//night  50Hz min10fps
			}
			else 
			{	
				V5B_SifI2cWriteByte(0x6d, 0x0f);
				V5B_SifI2cWriteByte(0x6e, 0x42);
				V5B_SifI2cWriteByte(0x6f, 0x40);	//night  60Hz min10fps
			}
			V5B_SifI2cWriteByte(0x60, 0xb9);
			break;
		case 3:	
			V5B_SifI2cWriteByte(0x60, 0xb8);
			V5B_SifI2cReadByte(0x6a, &uTemp1);	
			V5B_SifI2cReadByte(0x6b, &uTemp2);
			V5B_SifI2cReadByte(0x6c, &uTemp3);
			uTemp = (UINT32)((uTemp1<<16)|(uTemp2<<8)|(uTemp3));
			if (uTemp == 0x01d4c0)				//50Hz 1 step
			{
				V5B_SifI2cWriteByte(0x6d, 0x09);	//outdoor 50Hz
				V5B_SifI2cWriteByte(0x6e, 0x27);
				V5B_SifI2cWriteByte(0x6f, 0xc0);	
			}
			else 
			{	
				V5B_SifI2cWriteByte(0x6d, 0x07);	//outdoor 50Hz
				V5B_SifI2cWriteByte(0x6e, 0xa1);
				V5B_SifI2cWriteByte(0x6f, 0x20);	
			}
			V5B_SifI2cWriteByte(0x60, 0xb9);
			break;
		default:
			break;
	}
}
const TReg gSifPwrOnSeqRegVal_HV7131GP[] =
{
	{0x00, 0x67,	1},
	{0x00, 0x66,	1},
	{0x00, 0x67,	1}
};

const TReg gSifRegVal_HV7131GP[] =
{        
        {0x01, 0x41, 0},
 	{0x02, 0x00, 0},
 	{0x03, 0x0a, 0},
 	{0x04, 0x05, 0},
 	{0x2c, 0x02, 0},
 	{0x2d, 0x80, 0},
 	{0x2e, 0x01, 0},
 	{0x2f, 0xe0, 0},
};

const TReg gSnrSizeRegValVGA_HV7131GP[] = 
{      
	{0x01,	0x03,	1},
	{0x03,	0x00,	1},

	//windows
	{0x09,	0x02,	1},
	{0x0b,	0x02,	1},
	{0x0c,	0x01,	1},
	{0x0d,	0xe8,	1},	//changed for successfully capturing xga 2006-12-12 
	{0x0e,	0x02,	1},
	{0x0f,	0x80,	1},

	{0x31,	0x29,	1},

	{0x14,	0x18,	1},
	{0x15,	0x18,	1},
	{0x16,	0x18,	1},
	{0x17,	0x12,	1},
	{0x18,	0x0a,	1},
	{0x19,	0x38,	1},
	{0x1a,	0x12,	1},
	{0x1b,	0x37,	1},
	
	{0x21,	0xa0,	1},
	{0x22,	0xa0,	1},
	{0x23,	0xa0,	1},
	
	{0x60,	0xb9,	1},
/*	{0x63,	0x00,	1},	//add start by ywq for erasing flicker @ 12M mclk	 2006-10-11 14:09
	{0x64,	0xea,	1},
	{0x65,	0x60,	1},	//add close 
	{0x6a,	0x00,	1},
	{0x6b,	0xea,	1},
	{0x6c,	0x60,	1},
	{0x6d,	0x06,	1},	//max ET=0.03s,frame rate doesn't change 
	{0x6e,	0x1a,	1},
	{0x6f,	0x80,	1},
*/	
	{0x63,	0x01,	1},	//changed start by ywq for erasing flicker @ 24M mclk	 2006-10-11 14:09
	{0x64,	0xd4,	1},
	{0x65,	0xc0,	1},
	{0x6a,	0x01,	1},
	{0x6b,	0xd4,	1},
	{0x6c,	0xc0,	1},
	{0x6d,	0x12,	1},	//max ET=0.1s, min frame rate=10fps
	{0x6e,	0x4f,	1},
	{0x6f,	0x80,	1},	//changed close
	
	{0x66,	0x80,	1},
	{0x67,	0xf6,	1},
	{0x76,	0x20,	1},
	{0x77,	0x75,	1},
	{0x80,	0x0a,	1},
	{0x81,	0x24,	1},
	{0x8a,	0xe9,	1},
	{0x8b,	0x28,	1},
	{0x90,	0x01,	1},
	{0x94,	0x7f,	1},
	{0x95,	0x3f,	1},

	//color matrix
	{0x34,	0x41,	1},
	{0x35,	0xc9,	1},
	{0x36,	0xf6,	1},
	{0x37,	0x0c,	0},
	{0x38,	0x31,	0},
	{0x39,	0x04,	0},
	{0x3a,	0xf9,	1},
	{0x3b,	0xba,	1},
	{0x3c,	0x4d,	1},

	//gamma
/*	{0x40,	0x00,	1},
	{0x41,	0x04,	1},
	{0x42,	0x12,	1},
	{0x43,	0x21,	1},
	{0x44,	0x37,	1},
	{0x45,	0x55,	1},
	{0x46,	0x6b,	1},
	{0x47,	0x7d,	1},
	{0x48,	0xb4,	1},
	{0x49,	0xdd,	1},

	{0x50,	0x40,	1},
	{0x51,	0x4b,	1},
	{0x52,	0x3c,	1},
	{0x53,	0x2c,	0},
	{0x54,	0x1e,	0},
	{0x55,	0x16,	0},
	{0x56,	0x12,	1},
	{0x57,	0x0e,	1},
	{0x58,	0x0a,	1},
	{0x59,	0x09,	1}, 
*/	
/*	{0x40,	0x27,	1},
	{0x41,	0x29,	1},
	{0x42,	0x2d,	1},
	{0x43,	0x32,	1},
	{0x44,	0x3c,	1},
	{0x45,	0x4e,	1},
	{0x46,	0x5f,	1},
	{0x47,	0x6e,	1},
	{0x48,	0xa5,	1},
	{0x49,	0xd4,	1},

	{0x50,	0x17,	1},
	{0x51,	0x16,	1},
	{0x52,	0x15,	1},
	{0x53,	0x13,	0},
	{0x54,	0x11,	0},
	{0x55,	0x10,	0},
	{0x56,	0x0f,	1},
	{0x57,	0x0d,	1},
	{0x58,	0x0c,	1},
	{0x59,	0x0a,	1},
	
	{0x40,	0x00,	1},	//modified at 2006-11-21 
	{0x41,	0x01,	1},
	{0x42,	0x04,	1},
	{0x43,	0x0a,	1},
	{0x44,	0x18,	1},
	{0x45,	0x32,	1},
	{0x46,	0x4a,	1},
	{0x47,	0x60,	1},
	{0x48,	0xa8,	1},
	{0x49,	0xdd,	1},
	{0x50,	0x10,	1},
	{0x51,	0x10,	1},
	{0x52,	0x18,	1},
	{0x53,	0x18,	1},
	{0x54,	0x1a,	1},
	{0x55,	0x18,	1},
	{0x56,	0x16,	1},
	{0x57,	0x12,	1},
	{0x58,	0x0d,	1},
	{0x59,	0x08,	1},  
*/	
	{0x40,	0x00,	1},	//modified at 2006-12-6 11:13 
	{0x41,	0x00,	1},
	{0x42,	0x02,	1},
	{0x43,	0x08,	1},
	{0x44,	0x14,	1},
	{0x45,	0x2e,	1},
	{0x46,	0x44,	1},
	{0x47,	0x5a,	1},
	{0x48,	0xa6,	1},
	{0x49,	0xdd,	1},
	{0x50,	0x00,	1},
	{0x51,	0x0a,	1},
	{0x52,	0x18,	1},
	{0x53,	0x18,	1},
	{0x54,	0x1a,	1},
	{0x55,	0x16,	1},
	{0x56,	0x16,	1},
	{0x57,	0x13,	1},
	{0x58,	0x0d,	1},
	{0x59,	0x08,	1}  
};

const TSnrSizeCfg gSensorSizeCfg_HV7131GP[] = 
{
	{
              {0,NULL},
               	{0,NULL},
	      	{640, 480},
		1,
		1


	}

};

const TSnrInfo gSensorInfo_HV7131GP = 
{
       "HV7131GP yuv mode",
	VIM_SNR_YUV,							//yuv mode
	0,							//not use PCLK of sensor
	0,							//[0]: VSync delay enable; [1]: Fifo VSync delay enable; [2]: Use PCLK Negtive Edge.
	0,							//0 -- I2C, 1 -- Serial bus;
	0,							//[0]: 1 -- OV I2C bus
	80,							//I2C Rate : 100KHz
	0x22,							//I2C address
	0x22,							//ISP I2C address for special sensor
	0x07,							//power config
	0,							//reset sensor
	3,							//brightness 
	3,							//contrast

	{0,NULL},											//sensor standby                                                                                  											
       {sizeof(gSnrSizeRegValVGA_HV7131GP) / sizeof(TReg), (PTReg)&gSnrSizeRegValVGA_HV7131GP},                                                       //snr initial value
	{sizeof(gSifPwrOnSeqRegVal_HV7131GP) / sizeof(TReg), (PTReg)&gSifPwrOnSeqRegVal_HV7131GP},	//Sif config sequence(Reg.800 bit0~2) when sensor power on                                        	
	{0, NULL},											//Sif config sequence(Reg.800 bit0~2) when sensor standby                                         	
	{sizeof(gSifRegVal_HV7131GP) / sizeof(TReg), (PTReg)&gSifRegVal_HV7131GP},			//sif initial value                                                                               	
	{sizeof(gSensorSizeCfg_HV7131GP) / sizeof(TSnrSizeCfg), (PTSnrSizeCfg)&gSensorSizeCfg_HV7131GP},//size configure                                                                                  	

	SensorSetReg_HV7131GP,											
	SensorGetReg_HV7131GP,											
	CheckId_HV7131GP,											
	SensorSetMirrorFlip_HV7131GP,										
	SensorSetContrast_HV7131GP,																									//Sensor switch size callback
	SensorSetBrightness_HV7131GP,																									//set et callback
	NULL,	
	SensorSetMode_HV7131GP,
	NULL,													
	NULL,													
	{	//ythd of flash,et of flash
		0x20,			10
	},
};

#endif
/******************************************************************
	Transform from 568(v4.2d) RDK sensor.c at 2006-11-20 
******************************************************************/
#if V5_DEF_SNR_HV7131RP_YUV	//HYCA3R

void SensorSetReg_HV7131RP(UINT16 uAddr, UINT16 uVal)
{
         V5B_SifI2cWriteByte((UINT8)uAddr, (UINT8)uVal);
}

void SensorGetReg_HV7131RP(UINT8 uAddr, UINT16 *uVal)
{
	V5B_SifI2cReadByte((UINT8)uAddr, (UINT8*)uVal);
}

UINT8 CheckId_HV7131RP(void)
{
	UINT8 temp;

	V5B_SifI2cReadByte(0, &temp);
	if(temp==0x42) 
		return SUCCEED;
	else
		return FAILED;  
}

void SensorSetMirrorFlip_HV7131RP(UINT8 val)
{
        UINT8 x;
	V5B_SifI2cReadByte(0x01,&x);
	val=~val;
	V5B_SifI2cWriteByte(0x01,(UINT8)((x&0x0cf)|((val&0x03)<<4)));
}

/******************************************************************
         Desc: set sensor contrast  callback 
         Para: 1-5 step
*******************************************************************/
void SensorSetContrast_HV7131RP(UINT8 val)
{
	switch(val)
	{	
	case 1:                 
		V5B_SifI2cWriteByte(0x5a, 0x6d);  
		break;  
	case 2:                                                    
		V5B_SifI2cWriteByte(0x5a, 0x7d);  
		break;
   	case 3:                                
		V5B_SifI2cWriteByte(0x5a, 0x8d);  
		break; 
        case 4:                                
		V5B_SifI2cWriteByte(0x5a, 0x9d); 
		break;  
	case 5:                                
		V5B_SifI2cWriteByte(0x5a, 0xa6); 
		break; 
	default:
		break;
	}					
}

/******************************************************************
         Desc: set sensor Brightness  callback 
         Para: 1-5 step
*******************************************************************/
void SensorSetBrightness_HV7131RP(UINT8 val)
{
	switch(val)
	{
	case 1:
		V5B_SifI2cWriteByte(0x5b, 0xe0);
		break;	
	case 2:
		V5B_SifI2cWriteByte(0x5b, 0xf0);
		break;
	case 3:
		V5B_SifI2cWriteByte(0x5b, 0x00);
		break;
	case 4:
		V5B_SifI2cWriteByte(0x5b, 0x10);
		break;
	case 5:
		V5B_SifI2cWriteByte(0x5b, 0x20);
		break;
	default:
		break;		
	}
}

/******************************************************************
         Desc: set sensor mode  callback 
         Para:  val: 	1 50hz,	2 60hz,	3 night, 4 outdoor.
*******************************************************************/
void SensorSetMode_HV7131RP(UINT8 val)
{	
	UINT8 uTemp1,uTemp2,uTemp3;
	UINT32 uTemp;
	switch (val)
	{	
		case 1:
			V5B_SifI2cWriteByte(0x70, 0xb8);	//disable AE
			V5B_SifI2cWriteByte(0x7a, 0x01);	//50Hz 1 step
			V5B_SifI2cWriteByte(0x7b, 0xd4);
			V5B_SifI2cWriteByte(0x7c, 0xc0);
			V5B_SifI2cWriteByte(0x7d, 0x09);	//50Hz 20fps
			V5B_SifI2cWriteByte(0x7e, 0x27);
			V5B_SifI2cWriteByte(0x7f, 0xc0);
			V5B_SifI2cWriteByte(0x70, 0xb9);	//enable AE
			break;
		case 2:
			V5B_SifI2cWriteByte(0x70, 0xb8);
			V5B_SifI2cWriteByte(0x7a, 0x01);	//60Hz 1 step
			V5B_SifI2cWriteByte(0x7b, 0x86);
			V5B_SifI2cWriteByte(0x7c, 0xa0);
			V5B_SifI2cWriteByte(0x7d, 0x07);	//60Hz 20fps
			V5B_SifI2cWriteByte(0x7e, 0xa1);
			V5B_SifI2cWriteByte(0x7f, 0x20);
			V5B_SifI2cWriteByte(0x70, 0xb9);
			break;
		case 4:  // 3 and 4 reverse
			V5B_SifI2cWriteByte(0x70, 0xb8);
			V5B_SifI2cReadByte(0x7a, &uTemp1);	
			V5B_SifI2cReadByte(0x7b, &uTemp2);
			V5B_SifI2cReadByte(0x7c, &uTemp3);
			uTemp = (UINT32)((uTemp1<<16)|(uTemp2<<8)|(uTemp3));
			if (uTemp == 0x01d4c0)				//50Hz 1 step			
			{
				V5B_SifI2cWriteByte(0x7d, 0x12);
				V5B_SifI2cWriteByte(0x7e, 0x4f);
				V5B_SifI2cWriteByte(0x7f, 0x80);	//night  50Hz min10fps
			}
			else 
			{	
				V5B_SifI2cWriteByte(0x7d, 0x0f);
				V5B_SifI2cWriteByte(0x7e, 0x42);
				V5B_SifI2cWriteByte(0x7f, 0x40);	//night  60Hz min10fps
			}
			V5B_SifI2cWriteByte(0x70, 0xb9);
			break;
		case 3:	
			V5B_SifI2cWriteByte(0x70, 0xb8);
			V5B_SifI2cReadByte(0x7a, &uTemp1);	
			V5B_SifI2cReadByte(0x7b, &uTemp2);
			V5B_SifI2cReadByte(0x7c, &uTemp3);
			uTemp = (UINT32)((uTemp1<<16)|(uTemp2<<8)|(uTemp3));
			if (uTemp == 0x01d4c0)				//50Hz 1 step
			{
				V5B_SifI2cWriteByte(0x7d, 0x09);	//outdoor 50Hz
				V5B_SifI2cWriteByte(0x7e, 0x27);
				V5B_SifI2cWriteByte(0x7f, 0xc0);	
			}
			else 
			{	
				V5B_SifI2cWriteByte(0x7d, 0x07);	//outdoor 50Hz
				V5B_SifI2cWriteByte(0x7e, 0xa1);
				V5B_SifI2cWriteByte(0x7f, 0x20);	
			}
			V5B_SifI2cWriteByte(0x70, 0xb9);
			break;
		default:
			break;
	}
}

const TReg gSifPwrOnSeqRegVal_HV7131RP[] =
{
	{0x00, 0x67,	1},
	{0x00, 0x66,	1},
	{0x00, 0x67,	1}
};

const TReg gSifRegVal_HV7131RP[] =
{        
        {0x01,  0x01, 	0},
 	{0x02,  0x02,	0},
//	{0x03,  0x0a, 	0},
 	{0x03,  0x08, 	0},	//changed by ywq for successfully capturing 2006-10-27 9:42
 	{0x04,  0x04, 	0},
 	{0x08, 	0x04, 	0},
 	{0x09, 	0xb0, 	0},
 	{0x2c, 	0x02,	0},
 	{0x2d, 	0x80,	0},
 	{0x2e, 	0x01,	0},
 	{0x2f, 	0xe0,	0},
};

const TReg gSnrSizeRegValVGA_HV7131RP[] = 
{      
	{0x01,	0x03,	1},
	{0x03,	0x00,	1},
//windows
/*	{0x09,	0x02,	1},
	{0x0b,	0x02,	1},
	{0x0c,	0x01,	1},
	{0x0d,	0xe0,	1},
	{0x0e,	0x02,	1},
	{0x0f,	0x80,	1},
*/	
/*	{0x08,	0x00,	1},
	{0x09,	0x04,	1},
	{0x0a,	0x00,	1},
	{0x0b,	0x04,	1},
		
	{0x0c,	0x01,	1},	//changed by ywq for successfully capturing 2006-10-26 14:52
	{0x0d,	0xe6,	1},
	{0x0e,	0x02,	1},
	{0x0f,	0x80,	1},
*/	
//	{0x31,	0x29,	1},
	{0x14,	0x22,	1},
	{0x15,	0x18,	1},
	{0x16,	0x30,	1},
	{0x17,	0x20,	1},
	{0x18,	0x12,	1},
	{0x19,	0x68,	1},
	{0x1a,	0x1a,	1},
	{0x1b,	0x37,	1},
	{0x21,	0x00,	1},
	{0x22,	0x00,	1},
	{0x23,	0x00,	1},
	{0x30,	0x7e,	1},
	{0x31,	0x60,	1},
	{0x5a,	0x8d,	1},
	{0x5b,	0x00,	1},
//	{0x5c,	0x9a,	1},
	{0x5c,	0x80,	1},	//changed 2006-12-6 14:34
	{0x5d,	0x03,	1},
	{0x5e,	0x07,	1},
	{0x60,	0x38,	1},
	{0x61,	0x28,	1},
	{0x62,	0x28,	1},
	{0x70,	0x69,	1},
	{0x71,	0xad,	1},
	{0x72,	0x89,	1},
//	{0x73,	0x00,	1},	//add by ywq to earse flicker for 12M mclk&pclk		2006-10-16 17:53
//	{0x74,	0xea,	1},
//	{0x75,	0x60,	1},
	{0x73,	0x01,	1},	//add by ywq to earse flicker for 24M mclk&pclk		2006-10-16 17:53
	{0x74,	0xd4,	1},
	{0x75,	0xc0,	1},
	{0x76,	0x40,	1},
	{0x77,	0x40,	1},
	{0x78,	0xf6,	1},
	{0x79,	0x24,	1},
	{0x7a,	0x01,	1},
	{0x7b,	0xd4,	1},
	{0x7c,	0xc0,	1},
	{0x7d,	0x12,	1},	//50Hz max step 10 2007-1-8 
	{0x7e,	0x4f,	1},
	{0x7f,	0x80,	1},
	{0x80,	0x38,	1},
	{0x87,	0x35,	1},
	{0x88,	0x35,	1},
	{0x89,	0x30,	1},
	{0x8b,	0x10,	1},
	{0x8c,	0x3f,	1},
	{0x91,	0x24,	1},
	{0x92,	0x9e,	1},
	{0x93,	0x9c,	1},
	{0x33,	0x21,	1},
	{0x34,	0x38,	1},
	{0x35,	0x13,	1},
	{0x36,	0x13,	1},
	{0x37,	0x37,	1},
	{0x38,	0xd1,	1},
	{0x39,	0xf8,	1},
	{0x3a,	0x11,	1},
	{0x3b,	0x2a,	1},
	{0x3c,	0x08,	1},
	{0x3d,	0xf1,	1},
	{0x3e,	0xb7,	1},
	{0x3f,	0x58,	1},
/*
	{0x40,	0x00,	1},	//gamma
	{0x41,	0x01,	1},
	{0x42,	0x04,	1},
	{0x43,	0x0a,	1},
	{0x44,	0x18,	1},
	{0x45,	0x32,	1},
	{0x46,	0x4a,	1},
	{0x47,	0x60,	1},
	{0x48,	0xa8,	1},
	{0x49,	0xdd,	1},
	{0x4a,	0x10,	1},
	{0x4b,	0x10,	1},
	{0x4c,	0x18,	1},
	{0x4d,	0x18,	1},
	{0x4e,	0x1a,	1},
	{0x4f,	0x18,	1},
	{0x50,	0x16,	1},
	{0x51,	0x12,	1},
	{0x52,	0x0d,	1},
	{0x53,	0x08,	1},
	*/
	{0x40,	0x00,	1},	//changed 2006-12-6 13:49
	{0x41,	0x04,	1},
	{0x42,	0x0a,	1},
	{0x43,	0x12,	1},
	{0x44,	0x20,	1},
	{0x45,	0x39,	1},
	{0x46,	0x52,	1},
	{0x47,	0x66,	1},
	{0x48,	0xae,	1},
	{0x49,	0xdd,	1},

	{0x4a,	0x40,	1},
	{0x4b,	0x20,	1},
	{0x4c,	0x20,	1},
	{0x4d,	0x1c,	0},
	{0x4e,	0x19,	0},
	{0x4f,	0x19,	0},
	{0x50,	0x14,	1},
	{0x51,	0x12,	1},
	{0x52,	0x0b,	1},
	{0x53,	0x08,	1}, 
};

const TSnrSizeCfg gSensorSizeCfg_HV7131RP[] = 
{
	{
            {0,NULL},
              {0,NULL},
	      	{640, 480},
		1,
		1


	}

};

const TSnrInfo gSensorInfo_HV7131RP = 
{
       "HV7131RP yuv mode",
	VIM_SNR_YUV,							//yuv mode
	0,							//not use PCLK of sensor
	0,							//[0]: VSync delay enable; [1]: Fifo VSync delay enable; [2]: Use PCLK Negtive Edge.
	0,							//0 -- I2C, 1 -- Serial bus;
	0,							//[0]: 1 -- OV I2C bus
	80,							//I2C Rate : 100KHz
	0x22,							//I2C address
	0x22,							//ISP I2C address for special sensor
	0x07,							//power config
	0,							//reset sensor
	3,							//brightness 
	3,							//contrast

	{0,NULL},											//sensor standby                                                                                  											
        {sizeof(gSnrSizeRegValVGA_HV7131RP) / sizeof(TReg), (PTReg)&gSnrSizeRegValVGA_HV7131RP},                                                       //snr initial value
	{sizeof(gSifPwrOnSeqRegVal_HV7131RP) / sizeof(TReg), (PTReg)&gSifPwrOnSeqRegVal_HV7131RP},	//Sif config sequence(Reg.800 bit0~2) when sensor power on                                        	
	{0, NULL},											//Sif config sequence(Reg.800 bit0~2) when sensor standby                                         	
	{sizeof(gSifRegVal_HV7131RP) / sizeof(TReg), (PTReg)&gSifRegVal_HV7131RP},			//sif initial value                                                                               	
	{sizeof(gSensorSizeCfg_HV7131RP) / sizeof(TSnrSizeCfg), (PTSnrSizeCfg)&gSensorSizeCfg_HV7131RP},//size configure                                                                                  	

	SensorSetReg_HV7131RP,											
	SensorGetReg_HV7131RP,											
	CheckId_HV7131RP,											
	SensorSetMirrorFlip_HV7131RP,										
	SensorSetContrast_HV7131RP,																									//Sensor switch size callback
	SensorSetBrightness_HV7131RP,																									//set et callback
	NULL,	
	SensorSetMode_HV7131RP,
	NULL,													
	NULL,													
	{	//ythd of flash,et of flash
		0x20,			10
	},
};

#endif
/******************************************************************
	Transform from 568(v4.2d) RDK sensor.c at 2006-10-27 
******************************************************************/
#if V5_DEF_SNR_NOON30PC11_YUV

/* write sensor register callback */

void SensorSetReg_NOON30PC11(UINT16 uAddr, UINT16 uVal)

{
         V5B_SifI2cWriteByte((UINT8)uAddr, (UINT8)uVal);
}

void SensorGetReg_NOON30PC11(UINT8 uAddr, UINT16 *uVal)
{
	V5B_SifI2cReadByte((UINT8)uAddr, (UINT8*)uVal);
}

UINT8 CheckId_NOON30PC11(void)
{
	UINT8 temp;

	V5B_SifI2cReadByte(0x28, &temp);
	if(temp==0x11) 
		return SUCCEED;
	else
		return FAILED;  
}

/******************************************************************

         Desc: set sensor mirror and flip callback 

         Para: BIT0 -- mirror

                     BIT1 -- flip

*******************************************************************/

void SensorSetMirrorFlip_NOON30PC11(UINT8 val)
{
       UINT8 x;
	V5B_SifI2cReadByte(0x01,&x);
	if (val==1)
		val=2;
	else if (val==2)
		val=1;
	V5B_SifI2cWriteByte(0x01,(UINT8)((x&0xfc)|(val&0x03)));
}

void SensorSetContrast_NOON30PC11(UINT8 val)
{
	switch(val)
	{	
	case 1:                 
		V5B_SifI2cWriteByte(0x33, 0x08);	
		V5B_SifI2cWriteByte(0x34, 0x11);
		V5B_SifI2cWriteByte(0x35, 0x20);
		V5B_SifI2cWriteByte(0x36, 0x31);  
		V5B_SifI2cWriteByte(0x37, 0x43);	
		V5B_SifI2cWriteByte(0x38, 0x55); 
		V5B_SifI2cWriteByte(0x39, 0x6d); 
		V5B_SifI2cWriteByte(0x3a, 0x81); 
		V5B_SifI2cWriteByte(0x3b, 0x92);	
		V5B_SifI2cWriteByte(0x3c, 0xaf); 
		V5B_SifI2cWriteByte(0x3d, 0xc6); 
		V5B_SifI2cWriteByte(0x3e, 0xe9); 
		V5B_SifI2cWriteByte(0x3f, 0xff);	

		break;  
	case 2:                                                    
		V5B_SifI2cWriteByte(0x33, 0x04);	
		V5B_SifI2cWriteByte(0x34, 0x0a);
		V5B_SifI2cWriteByte(0x35, 0x18);
		V5B_SifI2cWriteByte(0x36, 0x26);  
		V5B_SifI2cWriteByte(0x37, 0x34);	
		V5B_SifI2cWriteByte(0x38, 0x46); 
		V5B_SifI2cWriteByte(0x39, 0x5d); 
		V5B_SifI2cWriteByte(0x3a, 0x74); 
		V5B_SifI2cWriteByte(0x3b, 0x87);	
		V5B_SifI2cWriteByte(0x3c, 0xa5); 
		V5B_SifI2cWriteByte(0x3d, 0xbe); 
		V5B_SifI2cWriteByte(0x3e, 0xe3); 
		V5B_SifI2cWriteByte(0x3f, 0xff);  
		break;
   	case 3:                                
		V5B_SifI2cWriteByte(0x33, 0x01);	
		V5B_SifI2cWriteByte(0x34, 0x07);
		V5B_SifI2cWriteByte(0x35, 0x0c);
		V5B_SifI2cWriteByte(0x36, 0x14);  
		V5B_SifI2cWriteByte(0x37, 0x20);	
		V5B_SifI2cWriteByte(0x38, 0x33); 
		V5B_SifI2cWriteByte(0x39, 0x51); 
		V5B_SifI2cWriteByte(0x3a, 0x65); 
		V5B_SifI2cWriteByte(0x3b, 0x77);	
		V5B_SifI2cWriteByte(0x3c, 0x95); 
		V5B_SifI2cWriteByte(0x3d, 0xae); 
		V5B_SifI2cWriteByte(0x3e, 0xda); 
		V5B_SifI2cWriteByte(0x3f, 0xfb);  
		break; 
        case 4:                                
		V5B_SifI2cWriteByte(0x33, 0x00);	// 00
		V5B_SifI2cWriteByte(0x34, 0x03);     	// 01
		V5B_SifI2cWriteByte(0x35, 0x06);     	// 02
		V5B_SifI2cWriteByte(0x36, 0x0a);     	// 07
		V5B_SifI2cWriteByte(0x37, 0x10);	// 0d
		V5B_SifI2cWriteByte(0x38, 0x1e);  	// 19
		V5B_SifI2cWriteByte(0x39, 0x35);  	// 2e
		V5B_SifI2cWriteByte(0x3a, 0x4a);  	// 41
		V5B_SifI2cWriteByte(0x3b, 0x5e);	// 55
		V5B_SifI2cWriteByte(0x3c, 0x81);  	// 78
		V5B_SifI2cWriteByte(0x3d, 0x9f);  	// 96
		V5B_SifI2cWriteByte(0x3e, 0xd0);  	// c8
		V5B_SifI2cWriteByte(0x3f, 0xff);  	// ff
		break;  
	case 5:                                
		V5B_SifI2cWriteByte(0x33, 0x00);	
		V5B_SifI2cWriteByte(0x34, 0x00);
		V5B_SifI2cWriteByte(0x35, 0x01);
		V5B_SifI2cWriteByte(0x36, 0x02);  
		V5B_SifI2cWriteByte(0x37, 0x07);	
		V5B_SifI2cWriteByte(0x38, 0x11); 
		V5B_SifI2cWriteByte(0x39, 0x25); 
		V5B_SifI2cWriteByte(0x3a, 0x38); 
		V5B_SifI2cWriteByte(0x3b, 0x4c);	
		V5B_SifI2cWriteByte(0x3c, 0x6e); 
		V5B_SifI2cWriteByte(0x3d, 0x8c); 
		V5B_SifI2cWriteByte(0x3e, 0xc1); 
		V5B_SifI2cWriteByte(0x3f, 0xff);
		break; 
	default:
		break;
	}					
}

/******************************************************************
         Desc: set sensor Brightness  callback 
         Para: 1-5 step
*******************************************************************/
void SensorSetBrightness_NOON30PC11(UINT8 val)
{
	switch(val)
	{
	case 1:
		V5B_SifI2cWriteByte(0x15, 0xb0);
		break;	
	case 2:
		V5B_SifI2cWriteByte(0x15, 0xa0);
		break;
	case 3:
		V5B_SifI2cWriteByte(0x15, 0x90);
		break;
	case 4:
		V5B_SifI2cWriteByte(0x15, 0x80);
		break;
	case 5:
		V5B_SifI2cWriteByte(0x15, 0x10);
		break;
	default:
		break;		
	}
}

/******************************************************************
         Desc: set sensor mode  callback 
         Para:  val: 	1 50hz,	2 60hz,	3 night, 4 outdoor.
*******************************************************************/
void SensorSetMode_NOON30PC11(UINT8 val)
{	
	UINT8 uTemp;
	switch (val)
	{	
		case 1:
			V5B_SifI2cWriteByte(0x0b, 0x10);	//disable AE & set 50Hz
			
			V5B_SifI2cWriteByte(0x71, 0x01);	//50Hz 20fps
			V5B_SifI2cWriteByte(0x72, 0x24);
			V5B_SifI2cWriteByte(0x73, 0xf8);
			VIM_USER_DelayMs(50);
			V5B_SifI2cWriteByte(0x0b, 0x9c);	//enable AE
			
			break;
		case 2:
			V5B_SifI2cWriteByte(0x0b, 0x00);	//disable AE & set 60Hz
		
			V5B_SifI2cWriteByte(0x71, 0x00);	//60Hz 20fps
			V5B_SifI2cWriteByte(0x72, 0xf4);
			V5B_SifI2cWriteByte(0x73, 0x24);
			VIM_USER_DelayMs(50);
			V5B_SifI2cWriteByte(0x0b, 0x8c);
	
			break;
		case 4:   // 3 and 4 reverse
			V5B_SifI2cReadByte(0x0b, &uTemp);	
			if (uTemp == 0x9c)				//50Hz			
			{	
				V5B_SifI2cWriteByte(0x0b, 0x10);
				V5B_SifI2cWriteByte(0x71, 0x02);
				V5B_SifI2cWriteByte(0x72, 0x49);
				V5B_SifI2cWriteByte(0x73, 0xf0);	//night  50Hz min10fps
				VIM_USER_DelayMs(50);
				V5B_SifI2cWriteByte(0x0b, 0x9c);
			}
			else 
			{	
				V5B_SifI2cWriteByte(0x0b, 0x00);
				V5B_SifI2cWriteByte(0x71, 0x01);
				V5B_SifI2cWriteByte(0x72, 0xe8);
				V5B_SifI2cWriteByte(0x73, 0x48);	//night  60Hz min10fps
				VIM_USER_DelayMs(50);
				V5B_SifI2cWriteByte(0x0b, 0x8c);
			}
			
			break;
		case 3:	
			V5B_SifI2cReadByte(0x0b, &uTemp);	
			if (uTemp == 0x9c)				//50Hz 1 step
			{	
				V5B_SifI2cWriteByte(0x0b, 0x10);
				V5B_SifI2cWriteByte(0x71, 0x01);	//outdoor 50Hz
				V5B_SifI2cWriteByte(0x72, 0x24);
				V5B_SifI2cWriteByte(0x73, 0xf8);	
				VIM_USER_DelayMs(50);
				V5B_SifI2cWriteByte(0x0b, 0x9c);
			}
			else 
			{	
				V5B_SifI2cWriteByte(0x0b, 0x00);
				V5B_SifI2cWriteByte(0x71, 0x00);	//outdoor 50Hz
				V5B_SifI2cWriteByte(0x72, 0xf4);
				V5B_SifI2cWriteByte(0x73, 0x24);	
				VIM_USER_DelayMs(50);
				V5B_SifI2cWriteByte(0x0b, 0x8c);
			}
			
			break;
		default:
			break;
	}
}

const TReg gSifPwrOnSeqRegVal_NOON30PC11[] =
{
	{0x00, 0x67,	1},
	{0x00, 0x66,	1},
	{0x00, 0x67,	1}
};
const TReg gSifRegVal_NOON30PC11[] =
{        
        {0x02,	0x00,	0},
	{0x08, 	0x01,	0},
	{0x09, 	0x0c,	0},
	{0x03, 	0x0a,	0},
	{0x04, 	0x04,	0},
	{0x01, 	0x41,	0},
	{0x2c, 	0x02,	0},
	{0x2d, 	0x80,	0},
	{0x2e, 	0x01,	0},
	{0x2f, 	0xe0,	0},
       
         {0x2c, 0x02,    0},
         {0x2d, 0x80,    0},
         {0x2e, 0x01,    0},
         {0x2f, 0xe0,     0},
};

/* Sensor size control configure info */

const TReg gSnrSizeRegValVGA_NOON30PC11[] = 
{        
        {0x04,	0x01,	1},
	{0x04,	0x03,	1},
	{0x04,	0x01,	1},
	{0x00,	0x0C,	1},
	{0x01,	0x1b,	1},
	{0x02,	0x00,	1}, 
	{0x03,	0xa8,	1},
	{0x05,	0x00,	1},
	{0x06,	0x01,	1},
	{0x07,	0x1B,	1},
	{0x08,	0x71,	1},
	{0x09,	0x30,	1}, 
	{0x0a,	0xc3,	1},
	{0x0c,	0x65,	1}, 
	{0x0d,	0x23,	1},
	{0x10,	0x80,	1},
	{0x11,	0x88,	1}, 
	{0x12,	0x01,	1},
	{0x13,	0x08,	1}, 
	{0x15,	0x80,	1},
	{0x17,	0x80,	1},
	{0x18,	0x80,	1},
	{0x19,	0x65,	1},
	{0x20,	0x20,	1},
	{0x1e,	0x7F,	1},
	{0x1f,	0x08,	1},
	{0x60,	0x11,	1},
	{0x50,	0x40,	1},
	{0x51,	0x20,	1}, 
	{0x52,	0x30,	1}, 
	{0x53,	0x68,	1},
	{0x54,	0x18,	1}, 
	{0x55,	0x70,	1},
	{0x56,	0x28,	1},
	{0x59,	0xc3,	1},
	{0x5A,	0x50,	1},
	{0x5B,	0x20,	1},
	{0x5C,	0x20,	1},
	{0x5D,	0x02,	1},
	{0x5E,	0x00,	1},
	{0x5F,	0x15,	1},
	{0x40,	0x80,	1},
	{0x41,	0x80,	1},
	{0x26,	0x2f,	1},
	{0x2a,	0x61,	1},
	{0x2b,	0x43,	1},
	{0x2c,	0x11,	1},
	{0x2d,	0x13,	1},
	{0x2e,	0x56,	1},
	{0x2f,	0x03,	1},
	{0x30,	0x02,	1}, 
	{0x31,	0x3d,	1},
	{0x32,	0x80,	1},
	{0x33,  0x01, 	1},  // Gamma Setting 0.55	2006-12-5 15:54
  	{0x34,  0x07, 	1},  
  	{0x35,  0x0c, 	1},  
  	{0x36,  0x14, 	1},  
  	{0x37,  0x20, 	1},  
  	{0x38,  0x33, 	1},  
  	{0x39,  0x51, 	1},  
  	{0x3a,  0x65, 	1},  
  	{0x3b,  0x77, 	1},  
  	{0x3c,  0x95, 	1},  
  	{0x3d,  0xae, 	1},  
  	{0x3e,  0xda, 	1},  
  	{0x3f,  0xfb, 	1}, 
	{0xF0,	0x48,	1},
	{0xF2,	0x02,	1},
	{0xF3,	0x07,	1},
	{0xF4,	0x89,	1}, 
	{0xF5,	0x81,	1},
	{0xF6,	0x07,	1}, 
	{0xF7,	0x86,	1},
	{0xF8,	0x8a,	1},
	{0xF9,	0x94,	1}, 
	{0xFA,	0x1f,	1},
	{0xA6,	0x00,	1}, 
	{0xA7,	0x09,	1},
	{0xA8,	0x15,	1},
	{0xA9,	0x30,	1},
	{0xAA,	0x5c,	1},
	{0xAB,	0x88,	1},
	{0xAC,	0xa8,	1},
	{0xAD,	0xc0,	1},
	{0xAE,	0xe6,	1},
	{0xd5,	0x01,	1},
	{0xd6,	0x40,	1}, 
	{0xd7,	0x00,	1}, 
	{0xd8,	0xf0,	1},
	{0xcc,	0x00,	1}, 
	{0x27,	0x80,	1},
	{0x44,	0x80,	1},
	{0x45,	0x45,	1},
	{0x46,	0x30,	1},
	{0x48,	0x00,	1},
	{0x4F,	0x78,	1},
	{0x4D,	0x20,	1},
	{0x4E,	0x8D,	1},
	{0x4F,	0x90,	1},
	{0x5f,	0x15,	1},
	{0x61,	0x0b,	1},
	{0x62,	0x55,	1},
	{0x63,	0x0b,	1},
	{0x64,	0x0b,	1},
	{0x65,	0x24,	1},
	{0x66,	0x48,	1},
	{0x67,	0x48,	1},
	{0x68,	0x48,	1},
	{0x74,	0x3A,	1}, 
	{0x75,	0x98,	1},
	{0x76,	0x30,	1},
	{0x77,	0xd4,	1},
	{0x71,	0x02,	1},	//max exp @10fps 2007-1-9
	{0x72,	0x49,	1},
	{0x73,	0xF0,	1}, 
	{0x88,	0x00,	1},
	{0x89,	0x6c,	1}, 
	{0x8a,	0x00,	1},
	{0x8b,	0x14,	1},
	{0x8d,	0x07,	1}, 
	{0x8e,	0x07,	1},
	{0x8f,	0x07,	1}, 
	{0xa0,	0x4e,	1},
	{0xa1,	0x41,	1},
	{0x24,	0x22,	1},
	{0x25,	0x18,	1},
	{0x91,	0x60,	1},
	{0x93,	0x3f,	1},
	{0x97,	0x55,	1},
	{0x99,	0x37,	1},
	{0x9a,	0x10,	1},
	{0x9b,	0x61,	1}, 
	{0x9e,	0x03,	1}, 
	{0x9f,	0x01,	1},
	{0xb0,	0x60,	1}, 
	{0xb1,	0x40,	1},
	{0xb2,	0x40,	1},
	{0x6C,	0x01,	1},
	{0x6D,	0x86,	1},
	{0x6E,	0xa0,	1},
	{0xe0,	0x44,	1},
	{0xe1,	0x3c,	1},
	{0xe2,	0x34,	1},
	{0xe3,	0x2c,	1},
	{0xe4,	0x88,	1},
	{0xe5,	0x88,	1},
	{0xe6,	0x55,	1},
	{0xe7,	0x55,	1},
	{0xe8,	0x30,	1},
	{0xe9,	0x88,	1},
	{0xfb,	0x10,	1},
	{0xfc,	0x11,	1},
	{0x0f,	0x22,	1},
	{0x0f,	0x94,	1}, 
	{0x0b,	0x9C,	1},
	{0x0e,	0xe9,	1},	//enable AWB both indoor & outdoor 2007-1-9
	{0x04,	0x00,	1},

};

const TSnrSizeCfg gSensorSizeCfg_NOON30PC11[] = 
{
	{
              {0,NULL},//{sizeof(gSifSizeRegValVGA_NOON30PC11) / sizeof(TReg), (PTReg)&gSifSizeRegValVGA_NOON30PC11},
              {0,NULL},//{sizeof(gSifSizeRegValVGA_NOON30PC11) / sizeof(TReg), (PTReg)&gSifSizeRegValVGA_NOON30PC11},
	     	{640, 480},
		1,
		1


	}

};

const TSnrInfo gSensorInfo_NOON30PC11 = 

{
       "NOON30PC11 yuv mode",
	VIM_SNR_YUV,							//yuv mode
	0,							//not use PCLK of sensor
	0,							//[0]: VSync delay enable; [1]: Fifo VSync delay enable; [2]: Use PCLK Negtive Edge.
	0,							//0 -- I2C, 1 -- Serial bus;
	0,							//[0]: 1 -- OV I2C bus
	80,							//I2C Rate : 100KHz
	0x60,							//I2C address
	0x60,							//ISP I2C address for special sensor
	0x07,							//power config
	0,							//reset sensor

	3,							//brightness 
	3,							//contrast
	{0,NULL},														//sensor standby
    	{sizeof(gSnrSizeRegValVGA_NOON30PC11) / sizeof(TReg), (PTReg)&gSnrSizeRegValVGA_NOON30PC11},                                                       //snr initial value
	{sizeof(gSifPwrOnSeqRegVal_NOON30PC11) / sizeof(TReg), (PTReg)&gSifPwrOnSeqRegVal_NOON30PC11},		//Sif config sequence(Reg.800 bit0~2) when sensor power on
	{0, NULL},																								//Sif config sequence(Reg.800 bit0~2) when sensor standby
	{sizeof(gSifRegVal_NOON30PC11) / sizeof(TReg), (PTReg)&gSifRegVal_NOON30PC11},						//sif initial value
	{sizeof(gSensorSizeCfg_NOON30PC11) / sizeof(TSnrSizeCfg), (PTSnrSizeCfg)&gSensorSizeCfg_NOON30PC11},		//size configure 

	SensorSetReg_NOON30PC11,																				//set reg callback
	SensorGetReg_NOON30PC11,																				//get reg callback
	CheckId_NOON30PC11,																				//Sensor ID Check Call
	SensorSetMirrorFlip_NOON30PC11,	//Set Sensor Mirror Flip Call
	SensorSetContrast_NOON30PC11,																									//Sensor switch size callback
	SensorSetBrightness_NOON30PC11,																									//set et callback
	NULL,	
	SensorSetMode_NOON30PC11,
	NULL,																									//Sensor switch size callback
	NULL,																									//set et callback
	{	//ythd of flash,et of flash
		0x20,			10
	},
};

#endif


/******************************************************************
	Transform from 568(v4.2d) RDK sensor.c at 2006-11-21 
******************************************************************/
#if V5_DEF_SNR_S5K83A_YUV	

void SensorSetReg_S5K83A(UINT16 uAddr, UINT16 uVal)
{
         V5B_SifI2cWriteByte((UINT8)uAddr, (UINT8)uVal);
}

void SensorGetReg_S5K83A(UINT8 uAddr, UINT16 *uVal)
{
	V5B_SifI2cReadByte((UINT8)uAddr, (UINT8*)uVal);
}

UINT8 CheckId_S5K83A(void)
{
	UINT8 temp1;
	UINT8 temp2;

	V5B_SifI2cWriteByte(0xec, 00);
	V5B_SifI2cReadByte(0xb2, &temp1);
	V5B_SifI2cReadByte(0xb3, &temp2);
	if((temp1==0xc4) && (temp2==0x01)) 
		return SUCCEED;
	else
		return FAILED; 
}

void SensorSetMirrorFlip_S5K83A(UINT8 val)
{
	V5B_SifI2cWriteByte(0xec,0);
	switch(val)
	{
		case 0:
			V5B_SifI2cWriteByte(0x75, 0x05);
			break;
		case 1:
			V5B_SifI2cWriteByte(0x75, 0x06);
			break;
		case 2:
			V5B_SifI2cWriteByte(0x75, 0x07);
			break;
		default:
			V5B_SifI2cWriteByte(0x75, 0x04);
			break;
	}
}

/******************************************************************
         Desc: set sensor Brightness  callback 
         Para: 1-5 step
*******************************************************************/
void SensorSetBrightness_S5K83A(UINT8 val)
{	
	V5B_SifI2cWriteByte(0xec, 0x00);
	switch(val)
	{
	case 1:
		V5B_SifI2cWriteByte(0x76, 0x60);
		break;	
	case 2:
		V5B_SifI2cWriteByte(0x76, 0x70);
		break;
	case 3:
		V5B_SifI2cWriteByte(0x76, 0x80);
		break;
	case 4:
		V5B_SifI2cWriteByte(0x76, 0x90);
		break;
	case 5:
		V5B_SifI2cWriteByte(0x76, 0xa0);
		break;
	default:
		break;		
	}
}

/******************************************************************
         Desc: set sensor mode  callback 
         Para:  val: 	1 50hz,	2 60hz,	3 night, 4 outdoor.
*******************************************************************/
void SensorSetMode_S5K83A(UINT8 val)
{
	V5B_SifI2cWriteByte(0xec, 0x00);
	switch (val)
	{	
		case 1:
			V5B_SifI2cWriteByte(0x74, 0x04);	//30fps
			V5B_SifI2cWriteByte(0x73, 0x00);
			break;
		case 2:
			V5B_SifI2cWriteByte(0x74, 0x08);	//30fps
			V5B_SifI2cWriteByte(0x73, 0x00);
			break;
		case 4:  // 3 and 4 reverse
			V5B_SifI2cWriteByte(0x73, 0x21);	
			break;
		case 3:	
			V5B_SifI2cWriteByte(0x73, 0x00);		
			break;
		default:
			break;
	}
}

const TReg gSifPwrOnSeqRegVal_S5K83A[] =
{
	{0x00, 0x67,	1},
	{0x00, 0x66,	1},
	{0x00, 0x67,	1}
};

const TReg gSifRegVal_S5K83A[] =
{        
        {0x01, 0x41,	0},
	{0x02, 0x02,	0},			
	{0x03, 0x0a,	0},
	{0x04, 0x04,	0},
};

const TReg gSnrSizeRegValVGA_S5K83A[] = 
{      
// from gc
	{0xec, 0x05, 0},
	{0x57, 0x60, 0},
//	{0x57, 0x20, 0},
	{0x23, 0x53, 0},
	{0x25, 0x65, 0},
	{0x04, 0xf0, 0},
	{0xec, 0x06, 0},
	{0x01, 0x07, 0},
	{0xec, 0x01, 0},
	{0x00, 0x06, 0},
	{0x01, 0x02, 0},
	{0xec, 0x01, 0},
	{0x48, 0xEA, 0},
	{0x49, 0xD5, 0},
	{0x4A, 0x41, 0},
	{0x4B, 0x41, 0},
	{0x4C, 0xC9, 0},
	{0x4D, 0xF5, 0},
	{0x5B, 0x4D, 0},
	{0x5C, 0x96, 0},
	{0x5D, 0x1D, 0},
	{0xec, 0x00, 0},
	{0x35, 0x04, 0},
	{0x3b, 0x08, 0},
	{0x40, 0x1e, 0},
	{0x41, 0x2d, 0},
	{0x42, 0x21, 0},
	{0x43, 0x2d, 0},
	{0x44, 0x2C, 0},
	{0x45, 0x25, 0},
	{0xec, 0x04, 0},
	
	
	{0xa1, 0xfe, 0},
	
	{0xec, 0x00, 0},
	{0x79, 0xfe, 0},
	{0x7a, 0x02, 0},
	{0xec, 0x01, 0},
	{0x0b, 0x27, 0},
	{0xa8, 0x00, 0},
	{0xec, 0x01, 0},
	{0x0c, 0x56, 0},
	{0x0d, 0xf6, 0},
	{0xec, 0x01, 0},
	{0x82, 0x64, 0},
	{0x83, 0x5f, 0},
	{0x84, 0x58, 0},
	{0x85, 0x51, 0},
	{0x86, 0x50, 0},
	{0x87, 0x52, 0},
	{0x88, 0x57, 0},
	{0x89, 0x5c, 0},
	{0x8a, 0x62, 0},
	{0x8b, 0x53, 0},
	{0x8c, 0x4B, 0},
	{0x8d, 0x47, 0},
	{0x8e, 0x40, 0},
	{0x8f, 0x40, 0},
	{0x90, 0x40, 0},
	{0x91, 0x44, 0},
	{0x92, 0x49, 0},
	{0x93, 0x54, 0},
	{0xec, 0x01, 0},
	{0x94, 0x01, 0},
	{0x95, 0x40, 0},
	{0x96, 0x00, 0},
	{0x97, 0xf0, 0},
	{0xec, 0x01, 0},
	{0x98, 0x23, 0},
	{0x99, 0x28, 0},
	{0x9a, 0x00, 0},
	{0x9b, 0x00, 0},
	{0x9c, 0x18, 0},
	{0x9d, 0x18, 0},
	{0x9e, 0x00, 0},
	{0x9f, 0x00, 0},
	{0xa0, 0x18, 0},
	{0xa1, 0x18, 0},
	{0xa2, 0x00, 0},
	{0xa3, 0x00, 0},
	{0xa4, 0x10, 0},
	{0xa5, 0x20, 0},
	{0xa6, 0x00, 0},
	{0xa7, 0x00, 0},
	{0xec, 0x00, 0},
	{0x6d, 0x01, 0},
	{0x6c, 0x10, 0},
	{0xec, 0x01, 0},
	{0x60, 0x03, 0},
	{0x61, 0x06, 0},
	{0x62, 0x12, 0},
	{0x63, 0x59, 0},
	{0x64, 0x00, 0},
	{0x65, 0x0C, 0},
	{0x66, 0xC0, 0},
	{0x67, 0xB8, 0},
	{0x68, 0xE8, 0},
	{0x69, 0x5B, 0},
	{0x6a, 0x04, 0},
	{0x6b, 0x06, 0},
	{0x6c, 0x10, 0},
	{0x6d, 0x60, 0},
	{0x6e, 0x00, 0},
	{0x6f, 0xFF, 0},
	{0x70, 0xA8, 0},
	{0x71, 0x94, 0},
	{0x72, 0xD2, 0},
	{0x73, 0x1B, 0},
	{0x74, 0x04, 0},
	{0x75, 0x09, 0},
	{0x76, 0x13, 0},
	{0x77, 0x63, 0},
	{0x78, 0x00, 0},
	{0x79, 0xfb, 0},
	{0x7a, 0x9e, 0},
	{0x7b, 0x93, 0},
	{0x7c, 0xb3, 0},
	{0x7d, 0x1b, 0},
	{0xec, 0x00, 0},
	{0x48, 0xfe, 0},
	{0x49, 0xfe, 0},
	{0x4a, 0x20, 0},
	{0x4b, 0x20, 0},
	{0x4c, 0xe0, 0},
	{0x4d, 0xf0, 0},
	{0x4e, 0x10, 0},
	{0x4f, 0x50, 0},
	{0x50, 0xfe, 0},
	{0x51, 0xfe, 0},
	{0x52, 0x20, 0},
	{0x53, 0x40, 0},
	{0x54, 0xe0, 0},
	{0x55, 0xd0, 0},
	{0x56, 0x10, 0},
	{0x57, 0x7f, 0},
	{0x58, 0xfe, 0},
	{0x59, 0xfe, 0},
	{0x5a, 0x20, 0},
	{0x5b, 0x20, 0},
	{0x5c, 0xe8, 0},
	{0x5d, 0xd0, 0},
	{0x5e, 0x35, 0},
	{0x5f, 0x7e, 0},
	{0xec, 0x00, 0},
	{0x7e, 0xf0, 0},
	{0x2d, 0x71, 0},
	{0xec, 0x04, 0},
	{0x85, 0x2d, 0},
	{0x81, 0x01, 0},
	{0xec, 0x04, 0},
	{0xb1, 0x1e, 0},
	{0xb2, 0x05, 0},
	{0xec, 0x00, 0},
	{0x86, 0x30, 0},
	{0x87, 0x00, 0},
	{0xab, 0x00, 0},
	{0xec, 0x01, 0},
	{0xc0, 0x00, 0},
	{0xc1, 0x06, 0},
	{0xc2, 0x00, 0},
	{0xc3, 0xd2, 0},
	{0xc4, 0x00, 0},
	{0xc5, 0x43, 0},
	{0xc6, 0x00, 0},
	{0xc7, 0x8a, 0},
	{0xec, 0x00, 0},
	{0x78, 0x6a, 0},
	{0xec, 0x00, 0},
	{0x7e, 0xd0, 0},
	{0xec, 0x04, 0},
	{0xc9, 0x20, 0},
	{0xec, 0x01, 0},
	{0x47, 0x06, 0},
	{0xec, 0x00, 0},
	{0x05, 0x00, 0},
	{0x44, 0x29, 0},
	{0x45, 0x26, 0},
	{0x79, 0xfd, 0},
	{0x7a, 0x00, 0},
	{0xec, 0x01, 0},
	{0x98, 0x2d, 0},
	{0x99, 0x27, 0},
	{0x9a, 0x10, 0},
	{0x9b, 0x16, 0},
	{0x9c, 0x17, 0},
	{0x9d, 0x18, 0},
	{0x9e, 0x00, 0},
	{0x9f, 0x00, 0},
	{0xa0, 0x18, 0},
	{0xa1, 0x18, 0},
	{0xa2, 0x00, 0},
	{0xa3, 0x00, 0},
	{0xa4, 0x10, 0},
	{0xa5, 0x19, 0},
	{0xa6, 0x00, 0},
	{0xa7, 0x03, 0},
	{0xec, 0x00, 0},
	{0x04, 0x11, 0},
	{0x5b, 0x40, 0},
	{0xec, 0x00, 0},
	{0xa4, 0x24, 0},
	{0xa5, 0x26, 0},
	{0xec, 0x00, 0},
	{0x78, 0x6e, 0},
	{0x2d, 0x5a, 0},
	{0x73, 0x10, 0},
	{0xec, 0x0a, 0},
	{0x22, 0x21, 0},
	{0xec, 0x00, 0},
	{0xa4, 0x2e, 0},
	{0xa5, 0x25, 0},
	{0x40, 0x1e, 0},
	{0x41, 0x37, 0},
	{0x42, 0x24, 0},
	{0x43, 0x2d, 0},
	{0x44, 0x2d, 0},
	{0x45, 0x25, 0},
	{0xec, 0x04, 0},
	{0xa1, 0xfd, 0},
	{0xec, 0x01, 0},
	{0x98, 0x26, 0},
	{0x99, 0x22, 0},
	{0x9a, 0x0b, 0},
	{0x9b, 0x0e, 0},
	{0xec, 0x00, 0},
	{0x31, 0x00, 0},
	{0xec, 0x04, 0},
	{0xd0, 0x81, 0},
	{0xd1, 0x83, 0},
	{0xd2, 0x23, 0},
	{0xec, 0x00, 0},
	{0x3a, 0x06, 0},
	{0xec, 0x04, 0},
	{0x8d, 0x06, 0},
	{0x8f, 0x06, 0},
	{0x91, 0x06, 0},
	{0xec, 0x09, 0},
	{0x00, 0x08, 0},
	{0x01, 0x00, 0},
	{0x02, 0x07, 0},
	{0x03, 0x05, 0},
	{0x04, 0x04, 0},
	{0x05, 0x06, 0},
	{0x06, 0x06, 0},
	{0x07, 0x08, 0},
	{0x08, 0xFE, 0},
	{0x09, 0xFE, 0},
	{0x0A, 0x20, 0},
	{0x0B, 0x20, 0},
	{0x0C, 0xE0, 0},
	{0x0D, 0xF0, 0},
	{0x0E, 0x50, 0},
	{0x0F, 0x50, 0},
	{0x10, 0xFE, 0},
	{0x11, 0xFE, 0},
	{0x12, 0x20, 0},
	{0x13, 0x40, 0},
	{0x14, 0xE0, 0},
	{0x15, 0xF0, 0},
	{0x16, 0x50, 0},
	{0x17, 0x50, 0},
	{0x18, 0xFE, 0},
	{0x19, 0xFE, 0},
	{0x1A, 0x20, 0},
	{0x1B, 0x40, 0},
	{0x1C, 0xE0, 0},
	{0x1D, 0xF0, 0},
	{0x1E, 0x50, 0},
	{0x1F, 0x50, 0},
	{0x20, 0x40, 0},
	{0x21, 0x01, 0},
	{0x22, 0x10, 0},
	{0x23, 0x80, 0},
	{0x24, 0xD0, 0},
	{0x25, 0xFD, 0},
	{0x26, 0x01, 0},
	{0x27, 0x00, 0},
	{0x28, 0x5A, 0},
	{0x29, 0x60, 0},
	{0x2A, 0x1F, 0},
	{0x2B, 0x12, 0},
	{0x2C, 0x08, 0},
	{0x2D, 0x18, 0},
	{0x2E, 0x40, 0},
	{0x2F, 0x9E, 0},
	{0x30, 0x00, 0},
	{0x31, 0x0C, 0},
	{0x32, 0xC0, 0},
	{0x33, 0xB8, 0},
	{0x34, 0xF0, 0},
	{0x35, 0x5B, 0},
	{0x36, 0x08, 0},
	{0x37, 0x10, 0},
	{0x38, 0x3D, 0},
	{0x39, 0x90, 0},
	{0x3A, 0x00, 0},
	{0x3B, 0xFF, 0},
	{0x3C, 0xA8, 0},
	{0x3D, 0x94, 0},
	{0x3E, 0xC0, 0},
	{0x3F, 0x1B, 0},
	{0x40, 0x08, 0},
	{0x41, 0x18, 0},
	{0x42, 0x35, 0},
	{0x43, 0x98, 0},
	{0x44, 0x00, 0},
	{0x45, 0x01, 0},
	{0x46, 0xA6, 0},
	{0x47, 0x93, 0},
	{0x48, 0xC4, 0},
	{0x49, 0x5B, 0},
	{0x4A, 0x06, 0},
	{0x4B, 0x02, 0},
	{0x4C, 0x06, 0},
	{0x4D, 0x02, 0},
	{0x4E, 0x06, 0},
	{0x4F, 0x08, 0},
	{0x50, 0x00, 0},
	{0x51, 0x07, 0},
	{0x52, 0x05, 0},
	{0x53, 0x04, 0},
	{0x54, 0x06, 0},
	{0x55, 0x06, 0},
	{0x56, 0x08, 0},
	{0x57, 0xFE, 0},
	{0x58, 0xFE, 0},
	{0x59, 0x20, 0},
	{0x5A, 0x20, 0},
	{0x5B, 0xE0, 0},
	{0x5C, 0xF0, 0},
	{0x5D, 0x50, 0},
	{0x5E, 0x50, 0},
	{0x5F, 0xFE, 0},
	{0x60, 0xFE, 0},
	{0x61, 0x20, 0},
	{0x62, 0x40, 0},
	{0x63, 0xE0, 0},
	{0x64, 0xF0, 0},
	{0x65, 0x50, 0},
	{0x66, 0x50, 0},
	{0x67, 0xFE, 0},
	{0x68, 0xFE, 0},
	{0x69, 0x20, 0},
	{0x6A, 0x40, 0},
	{0x6B, 0xE0, 0},
	{0x6C, 0xF0, 0},
	{0x6D, 0x50, 0},
	{0x6E, 0x50, 0},
	{0x6F, 0x40, 0},
	{0x70, 0x01, 0},
	{0x71, 0x10, 0},
	{0x72, 0x80, 0},
	{0x73, 0xD0, 0},
	{0x74, 0xFD, 0},
	{0x75, 0x01, 0},
	{0x76, 0x03, 0},
	{0x77, 0x30, 0},
	{0x78, 0x4A, 0},
	{0x79, 0x08, 0},
	{0x7A, 0x00, 0},
	{0x7B, 0x08, 0},
	{0x7C, 0x18, 0},
	{0x7D, 0x40, 0},
	{0x7E, 0x9E, 0},
	{0x7F, 0x00, 0},
	{0x80, 0x0C, 0},
	{0x81, 0xC0, 0},
	{0x82, 0xB8, 0},
	{0x83, 0xF0, 0},
	{0x84, 0x5B, 0},
	{0x85, 0x08, 0},
	{0x86, 0x10, 0},
	{0x87, 0x3D, 0},
	{0x88, 0x90, 0},
	{0x89, 0x00, 0},
	{0x8A, 0xFF, 0},
	{0x8B, 0xA8, 0},
	{0x8C, 0x94, 0},
	{0x8D, 0xC0, 0},
	{0x8E, 0x1B, 0},
	{0x8F, 0x08, 0},
	{0x90, 0x18, 0},
	{0x91, 0x35, 0},
	{0x92, 0x98, 0},
	{0x93, 0x00, 0},
	{0x94, 0x01, 0},
	{0x95, 0xA6, 0},
	{0x96, 0x93, 0},
	{0x97, 0xC4, 0},
	{0x98, 0x5B, 0},
	{0x99, 0x04, 0},
	{0x9A, 0x01, 0},
	{0x9B, 0x01, 0},
	{0x9C, 0x01, 0},
	{0x9D, 0x01, 0},
	{0xec, 0x0a, 0},
	{0x00, 0x08, 0},
	{0x01, 0x00, 0},
	{0x02, 0x07, 0},
	{0x03, 0x05, 0},
	{0x04, 0x04, 0},
	{0x05, 0x06, 0},
	{0x06, 0x06, 0},
	{0x07, 0x08, 0},
	{0x08, 0xFE, 0},
	{0x09, 0xFE, 0},
	{0x0A, 0x20, 0},
	{0x0B, 0x20, 0},
	{0x0C, 0xE0, 0},
	{0x0D, 0xF0, 0},
	{0x0E, 0x50, 0},
	{0x0F, 0x50, 0},
	{0x10, 0xFE, 0},
	{0x11, 0xFE, 0},
	{0x12, 0x20, 0},
	{0x13, 0x40, 0},
	{0x14, 0xE0, 0},
	{0x15, 0xF0, 0},
	{0x16, 0x50, 0},
	{0x17, 0x50, 0},
	{0x18, 0xFE, 0},
	{0x19, 0xFE, 0},
	{0x1A, 0x20, 0},
	{0x1B, 0x40, 0},
	{0x1C, 0xE0, 0},
	{0x1D, 0xF0, 0},
	{0x1E, 0x50, 0},
	{0x1F, 0x50, 0},
	{0x20, 0x40, 0},
	{0x21, 0x01, 0},
	{0x22, 0x21, 0},
	{0x23, 0x80, 0},
	{0x24, 0xD0, 0},
	{0x25, 0xFD, 0},
	{0x26, 0x01, 0},
	{0x27, 0x03, 0},
	{0x28, 0x30, 0},
	{0x29, 0x4A, 0},
	{0x2A, 0x08, 0},
	{0x2B, 0x00, 0},
	{0x2C, 0x08, 0},
	{0x2D, 0x18, 0},
	{0x2E, 0x40, 0},
	{0x2F, 0x9E, 0},
	{0x30, 0x00, 0},
	{0x31, 0x0C, 0},
	{0x32, 0xC0, 0},
	{0x33, 0xB8, 0},
	{0x34, 0xF0, 0},
	{0x35, 0x5B, 0},
	{0x36, 0x08, 0},
	{0x37, 0x10, 0},
	{0x38, 0x3D, 0},
	{0x39, 0x90, 0},
	{0x3A, 0x00, 0},
	{0x3B, 0xFF, 0},
	{0x3C, 0xA8, 0},
	{0x3D, 0x94, 0},
	{0x3E, 0xC0, 0},
	{0x3F, 0x1B, 0},
	{0x40, 0x08, 0},
	{0x41, 0x18, 0},
	{0x42, 0x35, 0},
	{0x43, 0x98, 0},
	{0x44, 0x00, 0},
	{0x45, 0x01, 0},
	{0x46, 0xA6, 0},
	{0x47, 0x93, 0},
	{0x48, 0xC4, 0},
	{0x49, 0x5B, 0},
	{0x4A, 0x05, 0},
	{0x4B, 0x01, 0},
	{0x4C, 0x01, 0},
	{0x4D, 0x01, 0},
	{0x4E, 0x01, 0},
	{0xec, 0x01, 0},
	{0x1a, 0xd0, 0},
	{0xec, 0x05, 0},
	{0x2a, 0x06, 0},
	{0x3a, 0x07, 0},
	{0x32, 0x06, 0},
	{0xec, 0x00, 0},
	{0x72, 0xD2, 0},
	{0x73, 0x30, 0},
	{0x78, 0x60, 0},
	{0xab, 0x58, 0},
	{0xec, 0x05, 0},
	{0x1b, 0x06, 0},
	{0xec, 0x01, 0},
	{0xc5, 0x05, 0},
	{0xc7, 0x9a, 0},
	{0xec, 0x01, 0},
	{0x41, 0x1a, 0},
	{0x42, 0x1a, 0},
	{0xec, 0x00, 0},
	{0x0a, 0x01, 0},
	{0x3e, 0x10, 0},
	{0x8e, 0xe0, 0},
	{0x74, 0x04, 0},
	{0x7f, 0x30, 0},
	{0x83, 0xa0, 0},
	{0x39, 0x07, 0},
	{0x3a, 0x09, 0},
	{0x7e, 0xf0, 0},
	{0x02, 0x00, 0},
	{0x76, 0x80, 0},
	{0x4d, 0x90, 0},
	{0x53, 0x30, 0},
	{0x5b, 0x30, 0},
	{0xec, 0x04, 0},
	{0xc9, 0x00, 0},
	{0xca, 0x5b, 0},
	{0xcb, 0x20, 0},
	{0xec, 0x01, 0},
	{0x21, 0xfd, 0},
	{0x22, 0xfd, 0},
 };

const TSnrSizeCfg gSensorSizeCfg_S5K83A[] = 
{
	{
       	{0,NULL},
               	{0,NULL},
	      	{640, 480},
		1,
		1


	}

};

const TSnrInfo gSensorInfo_S5K83A = 
{
       "S5K83A yuv mode",
	VIM_SNR_YUV,							//yuv mode
	0,							//not use PCLK of sensor
	0,							//[0]: VSync delay enable; [1]: Fifo VSync delay enable; [2]: Use PCLK Negtive Edge.
	0,							//0 -- I2C, 1 -- Serial bus;
	0,							//[0]: 1 -- OV I2C bus
	20,							//I2C Rate : 100KHz
	0x5a,							//I2C address
	0x5a,							//ISP I2C address for special sensor
	0x07,							//power config
	0,							//reset sensor
	3,							//brightness 
	3,							//contrast

	{0,NULL},											//sensor standby                                                                                  								
       {sizeof(gSnrSizeRegValVGA_S5K83A) / sizeof(TReg), (PTReg)&gSnrSizeRegValVGA_S5K83A},                                                       //snr initial value
	{sizeof(gSifPwrOnSeqRegVal_S5K83A) / sizeof(TReg), (PTReg)&gSifPwrOnSeqRegVal_S5K83A},		//Sif config sequence(Reg.800 bit0~2) when sensor power on                                        
	{0, NULL},											//Sif config sequence(Reg.800 bit0~2) when sensor standby                                         
	{sizeof(gSifRegVal_S5K83A) / sizeof(TReg), (PTReg)&gSifRegVal_S5K83A},				//sif initial value                                                                               
	{sizeof(gSensorSizeCfg_S5K83A) / sizeof(TSnrSizeCfg), (PTSnrSizeCfg)&gSensorSizeCfg_S5K83A},	//size configure                                                                                  

	SensorSetReg_S5K83A,										
	SensorGetReg_S5K83A,										
	CheckId_S5K83A,											
	SensorSetMirrorFlip_S5K83A,									
	NULL,
	SensorSetBrightness_S5K83A,																									//set et callback
	NULL,	
	SensorSetMode_S5K83A,
	NULL,												
	NULL,												
	{	//ythd of flash,et of flash
		0x20,			10
	},
};

#endif

/******************************************************************
	Transform from 568(v4.2d) RDK sensor.c at 2006-11-21 
******************************************************************/
#if V5_DEF_SNR_MC501CB_YUV	

void SensorSetReg_MC501CB(UINT16 uAddr, UINT16 uVal)
{
         V5B_SifI2cWriteByte((UINT8)uAddr, (UINT8)uVal);
}

void SensorGetReg_MC501CB(UINT8 uAddr, UINT16 *uVal)
{
	V5B_SifI2cReadByte((UINT8)uAddr, (UINT8*)uVal);
}

UINT8 CheckId_MC501CB(void)
{
	UINT8 temp;

	V5B_SifI2cWriteByte(0x03, 0x00);
	V5B_SifI2cReadByte(0x04, &temp);
	if(temp==0x62) 
		return SUCCEED;
	else
		return FAILED;  
}

void SensorSetMirrorFlip_MC501CB(UINT8 val)
{
        UINT8 x;
	V5B_SifI2cWriteByte(0x03,0x00);
	V5B_SifI2cReadByte(0x11,&x);
	if (val==0)
		val=3;
	else if (val==3)
	 	val=0;
	
	 V5B_SifI2cWriteByte(0x11,(UINT8)((x&0xfc)|(val&0x03)));
}

/******************************************************************
         Desc: set sensor contrast  callback 
         Para: 1-5 step
*******************************************************************/
void SensorSetContrast_MC501CB(UINT8 val)
{
	switch(val)
	{	
	case 1:    
		V5B_SifI2cWriteByte(0x03, 0x01);              
		V5B_SifI2cWriteByte(0x60, 0x01);  
		V5B_SifI2cWriteByte(0x61, 0x0a);  
		V5B_SifI2cWriteByte(0x62, 0x25);  
		V5B_SifI2cWriteByte(0x63, 0x33);  
		V5B_SifI2cWriteByte(0x64, 0x49);  
		V5B_SifI2cWriteByte(0x65, 0x65);  
		V5B_SifI2cWriteByte(0x66, 0x79);  
		V5B_SifI2cWriteByte(0x67, 0x87);  
		V5B_SifI2cWriteByte(0x68, 0x93);  
		V5B_SifI2cWriteByte(0x69, 0x9b);  
		V5B_SifI2cWriteByte(0x6a, 0xa5);
		V5B_SifI2cWriteByte(0x6b, 0xb1);
		V5B_SifI2cWriteByte(0x6c, 0xc7);
		V5B_SifI2cWriteByte(0x6d, 0xda);
		V5B_SifI2cWriteByte(0x6e, 0xec);
		V5B_SifI2cWriteByte(0x6f, 0xff);
		break;  
	case 2:    
		V5B_SifI2cWriteByte(0x03, 0x01);                                                
		V5B_SifI2cWriteByte(0x60, 0x01);  
		V5B_SifI2cWriteByte(0x61, 0x06);  
		V5B_SifI2cWriteByte(0x62, 0x1d);  
		V5B_SifI2cWriteByte(0x63, 0x2b);  
		V5B_SifI2cWriteByte(0x64, 0x3c);  
		V5B_SifI2cWriteByte(0x65, 0x59);  
		V5B_SifI2cWriteByte(0x66, 0x71);  
		V5B_SifI2cWriteByte(0x67, 0x84);  
		V5B_SifI2cWriteByte(0x68, 0x93);  
		V5B_SifI2cWriteByte(0x69, 0xa0);  
		V5B_SifI2cWriteByte(0x6a, 0xae);
		V5B_SifI2cWriteByte(0x6b, 0xbb);
		V5B_SifI2cWriteByte(0x6c, 0xd2);
		V5B_SifI2cWriteByte(0x6d, 0xe3);
		V5B_SifI2cWriteByte(0x6e, 0xf0);
		V5B_SifI2cWriteByte(0x6f, 0xff);
		break;
   	case 3:      
   		V5B_SifI2cWriteByte(0x03, 0x01);                          
		V5B_SifI2cWriteByte(0x60, 0x01);  
		V5B_SifI2cWriteByte(0x61, 0x03);  
		V5B_SifI2cWriteByte(0x62, 0x13);  
		V5B_SifI2cWriteByte(0x63, 0x1e);  
		V5B_SifI2cWriteByte(0x64, 0x30);  
		V5B_SifI2cWriteByte(0x65, 0x4f);  
		V5B_SifI2cWriteByte(0x66, 0x69);  
		V5B_SifI2cWriteByte(0x67, 0x80);  
		V5B_SifI2cWriteByte(0x68, 0x93);  
		V5B_SifI2cWriteByte(0x69, 0xa5);  
		V5B_SifI2cWriteByte(0x6a, 0xb5);
		V5B_SifI2cWriteByte(0x6b, 0xc3);
		V5B_SifI2cWriteByte(0x6c, 0xd9);
		V5B_SifI2cWriteByte(0x6d, 0xea);
		V5B_SifI2cWriteByte(0x6e, 0xf6);
		V5B_SifI2cWriteByte(0x6f, 0xff);
		break; 
        case 4:     
        	V5B_SifI2cWriteByte(0x03, 0x01);                           
		V5B_SifI2cWriteByte(0x60, 0x01);  
		V5B_SifI2cWriteByte(0x61, 0x00);  
		V5B_SifI2cWriteByte(0x62, 0x0a);  
		V5B_SifI2cWriteByte(0x63, 0x14);  
		V5B_SifI2cWriteByte(0x64, 0x24);  
		V5B_SifI2cWriteByte(0x65, 0x44);  
		V5B_SifI2cWriteByte(0x66, 0x5e);  
		V5B_SifI2cWriteByte(0x67, 0x77);  
		V5B_SifI2cWriteByte(0x68, 0x93);  
		V5B_SifI2cWriteByte(0x69, 0xa9);  
		V5B_SifI2cWriteByte(0x6a, 0xbb);
		V5B_SifI2cWriteByte(0x6b, 0xca);
		V5B_SifI2cWriteByte(0x6c, 0xe0);
		V5B_SifI2cWriteByte(0x6d, 0xef);
		V5B_SifI2cWriteByte(0x6e, 0xfa);
		V5B_SifI2cWriteByte(0x6f, 0xff);
		break;  
	case 5:     
		V5B_SifI2cWriteByte(0x03, 0x01);                           
		V5B_SifI2cWriteByte(0x60, 0x01);  
		V5B_SifI2cWriteByte(0x61, 0x00);  
		V5B_SifI2cWriteByte(0x62, 0x05);  
		V5B_SifI2cWriteByte(0x63, 0x0b);  
		V5B_SifI2cWriteByte(0x64, 0x1b);  
		V5B_SifI2cWriteByte(0x65, 0x37);  
		V5B_SifI2cWriteByte(0x66, 0x54);  
		V5B_SifI2cWriteByte(0x67, 0x72);  
		V5B_SifI2cWriteByte(0x68, 0x93);  
		V5B_SifI2cWriteByte(0x69, 0xae);  
		V5B_SifI2cWriteByte(0x6a, 0xc3);
		V5B_SifI2cWriteByte(0x6b, 0xd1);
		V5B_SifI2cWriteByte(0x6c, 0xe6);
		V5B_SifI2cWriteByte(0x6d, 0xf6);
		V5B_SifI2cWriteByte(0x6e, 0xfd);
		V5B_SifI2cWriteByte(0x6f, 0xff);
		break; 
	default:
		break;
	}					
}

/******************************************************************
         Desc: set sensor Brightness  callback 
         Para: 1-5 step
*******************************************************************/
void SensorSetBrightness_MC501CB(UINT8 val)
{
	switch(val)
	{
	case 1:
		V5B_SifI2cWriteByte(0x03, 0x01);  //has to place here
		V5B_SifI2cWriteByte(0x19, 0xa0);
		break;	
	case 2:
		V5B_SifI2cWriteByte(0x03, 0x01);
		V5B_SifI2cWriteByte(0x19, 0x90);
		break;
	case 3:
		V5B_SifI2cWriteByte(0x03, 0x01);
		V5B_SifI2cWriteByte(0x19, 0x00);
		break;
	case 4:
		V5B_SifI2cWriteByte(0x03, 0x01);
		V5B_SifI2cWriteByte(0x19, 0x10);
		break;
	case 5:
		V5B_SifI2cWriteByte(0x03, 0x01);
		V5B_SifI2cWriteByte(0x19, 0x20);
		break;
	default:
		break;		
	}
}

/******************************************************************
         Desc: set sensor mode  callback 
         Para:  val: 	1 50hz,	2 60hz,	3 night, 4 outdoor.
*******************************************************************/
void SensorSetMode_MC501CB(UINT8 val)
{	
	UINT8 uTemp;
	switch (val)
	{	
		case 1:
			V5B_SifI2cWriteByte(0x03, 0x03);  //has to place here
			V5B_SifI2cWriteByte(0x10, 0x10);	//disable AE & set 50Hz
			
			V5B_SifI2cWriteByte(0x38, 0x01);	//50Hz 20fps
			V5B_SifI2cWriteByte(0x39, 0x24);
			V5B_SifI2cWriteByte(0x3a, 0xf8);
			VIM_USER_DelayMs(50);
			V5B_SifI2cWriteByte(0x10, 0x9c);	//enable AE
			
			break;
		case 2:
			V5B_SifI2cWriteByte(0x03, 0x03);
			V5B_SifI2cWriteByte(0x10, 0x00);	//disable AE & set 60Hz
		
			V5B_SifI2cWriteByte(0x38, 0x00);	//60Hz 20fps
			V5B_SifI2cWriteByte(0x39, 0xf4);
			V5B_SifI2cWriteByte(0x3a, 0x24);
			VIM_USER_DelayMs(50);
			V5B_SifI2cWriteByte(0x10, 0x8c);
	
			break;
		case 4:  // 3 and 4 reverse
			V5B_SifI2cWriteByte(0x03, 0x03);
			V5B_SifI2cReadByte(0x10, &uTemp);	
			if (uTemp == 0x9c)				//50Hz			
			{	
				V5B_SifI2cWriteByte(0x10, 0x10);
				V5B_SifI2cWriteByte(0x38, 0x02);
				V5B_SifI2cWriteByte(0x39, 0x49);
				V5B_SifI2cWriteByte(0x3a, 0xf0);	//night  50Hz min10fps
				VIM_USER_DelayMs(50);
				V5B_SifI2cWriteByte(0x10, 0x9c);
			}
			else 
			{	
				V5B_SifI2cWriteByte(0x10, 0x00);
				V5B_SifI2cWriteByte(0x38, 0x01);
				V5B_SifI2cWriteByte(0x39, 0xe8);
				V5B_SifI2cWriteByte(0x3a, 0x48);	//night  60Hz min10fps
				VIM_USER_DelayMs(50);
				V5B_SifI2cWriteByte(0x10, 0x8c);
			}
			
			break;
		case 3:	
			V5B_SifI2cWriteByte(0x03, 0x03);
			V5B_SifI2cReadByte(0x10, &uTemp);	
			if (uTemp == 0x9c)				//50Hz 1 step
			{	
				V5B_SifI2cWriteByte(0x10, 0x10);
				V5B_SifI2cWriteByte(0x38, 0x01);	//outdoor 50Hz
				V5B_SifI2cWriteByte(0x39, 0x24);
				V5B_SifI2cWriteByte(0x3a, 0xf8);	
				VIM_USER_DelayMs(50);
				V5B_SifI2cWriteByte(0x10, 0x9c);
			}
			else 
			{	
				V5B_SifI2cWriteByte(0x10, 0x00);
				V5B_SifI2cWriteByte(0x38, 0x00);	//outdoor 50Hz
				V5B_SifI2cWriteByte(0x39, 0xf4);
				V5B_SifI2cWriteByte(0x3a, 0x24);	
				VIM_USER_DelayMs(50);
				V5B_SifI2cWriteByte(0x10, 0x8c);
			}
			
			break;
		default:
			break;
	}
}

const TReg gSifPwrOnSeqRegVal_MC501CB[] =
{
	{0x00, 0x67,	1},
	{0x00, 0x66,	1},
	{0x00, 0x67,	1}
};

const TReg gSifRegVal_MC501CB[] =
{        
        {0x01,  0x01, 	0},
 	{0x02,  0x02,	0},
 	{0x03,  0x0a, 	0},
 	{0x04,  0x04, 	0},
};

const TReg gSnrSizeRegValVGA_MC501CB[] = 
{      
	{0x01,	0x01,	1},
	{0x01,	0x03,	1},
	{0x01,	0x01,	1},
	{0x00,	0x00,	1},
	{0x02,	0x00,	1},
	{0x03,	0x00,	1},	//page 0
	{0x10,	0x00,	1},
	{0x11,	0x80,	1},
	{0x12,	0x0c,	1},
	{0x13,	0x00,	1},
	{0x14,	0x01,	1},
	{0x15,	0x00,	1},
	{0x16,	0x05,	1},
	{0x17,	0x01,	1},
	{0x18,	0xe0,	1},
	{0x19,	0x02,	1},
	{0x1a,	0x80,	1},
	{0x1b,	0x00,	1}, 	//HblankH changed by ywq 2006-10-23 13:32
	{0x1c,	0xc4,	1}, 	//HblankL should be greater than 196 and should be multiple of 4
	{0x1d,	0x00,	1},	//VsyncH
	{0x1e,	0x14,	1},	//VsyncL
	{0x1f,	0x09,	1},
	{0x20,	0xa8,	1},
	{0x21,	0x00,	1},
	{0x40,	0x33,	1},
	{0x41,	0x77,	1},
	{0x42,	0x53,	1},
	{0x43,	0xb0,	1},
	{0x44,	0x10,	1},
	{0x45,	0x00,	1},
	{0x46,	0x00,	1},
	{0x47,	0x8f,	1},
	{0x48,	0x4a,	1},
	{0x49,	0x00,	1},
	{0x4a,	0x03,	1},
	{0x85,	0x50,	1},	
	{0x91,	0x70,	1},
	{0x92,	0x72,	1},
	
	{0x03,	0x01,	1},	//page 1
	{0x10,	0x03,	1},
	{0x11,	0x03,	1},
	{0x12,	0x10,	1}, //enable brightness control
	{0x13,	0x00,	1},
	{0x14,	0x80,	1},
	{0x20,	0x0f,	1},
	{0x30,	0x0f,	1},
	{0x33,	0x38,	1},
	{0x35,	0x20,	1},
	{0x36,	0x2f,	1},
	{0x38,	0x54,	1},
	{0x39,	0x23,	1},
	{0x3a,	0x0f,	1},
	{0x3b,	0x25,	1},
	{0x3c,	0x7a,	1},
	{0x3d,	0x15,	1},
	{0x3e,	0x13,	1},
	{0x3f,	0x2c,	1},
	{0x40,	0x80,	1},
	{0x41,	0x82,	1},
	{0x42,	0x09,	1},
	{0x43,	0x85,	1},
	{0x44,	0x04,	1},
	{0x45,	0x85,	1},
	{0x46,	0x01,	1},
	{0x47,	0x89,	1},
	{0x48,	0x89,	1},
	{0x49,	0x12,	1},
	{0x60,	0x01,	1},
	{0x61,	0x03,	1},
	{0x62,	0x13,	1},
	{0x63,	0x1e,	1},
	{0x64,	0x30,	1},
	{0x65,	0x4f,	1},
	{0x66,	0x69,	1},
	{0x67,	0x80,	1},
	{0x68,	0x93,	1},
	{0x69,	0xa5,	1},
	{0x6a,	0xb5,	1},
	{0x6b,	0xc3,	1},
	{0x6c,	0xd9,	1},
	{0x6d,	0xea,	1},
	{0x6e,	0xf6,	1},
	{0x6f,	0xff,	1},
	{0x90,	0x80,	1},	
	{0x91,	0x9d,	1},
	{0x92,	0x02,	1},
	{0x93,	0x94,	1},
	{0x95,	0x00,	1},
	{0x96,	0x08,	1},
	{0xa0,	0x1a,	1},
	{0xa1,	0x00,	1},
	{0xa3,	0x10,	1},
	{0xa4,	0x08,	1},
	{0xa5,	0x20,	1},
	{0xb0,	0x22,	1},
	{0xb1,	0x11,	1},
	{0xc0,	0xc3,	1},
	{0xc2,	0x44,	1},
	{0xc4,	0x30,	1},
	{0xc5,	0x10,	1},
	{0xc6,	0x08,	1},
	{0xc7,	0x7f,	1},
	{0xd0,	0x07,	1},
	{0xd1,	0x80,	1},
	{0xd2,	0x80,	1},
	{0xd3,	0x50,	1},
	{0xd4,	0x20,	1},
	{0xd5,	0x1c,	1},
	{0xd6,	0x50,	1},
	{0xd7,	0x40,	1},
	
	{0x03,	0x02,	1},	//page 2
	{0x10,	0x00,	1},
	{0x11,	0x01,	1},
	{0x12,	0x40,	1},
	{0x13,	0x00,	1},
	{0x14,	0xf0,	1},
	{0x15,	0x00,	1},
	{0x16,	0x00,	1},
	{0x17,	0x00,	1},
	{0x18,	0x00,	1},
	{0x19,	0x01,	1},
	{0x1a,	0x40,	1},
	{0x1b,	0x00,	1},
	{0x1c,	0xf0,	1},
	{0x1d,	0x10,	1},	
	{0x1e,	0x00,	1},
	{0x1f,	0x10,	1},
	{0x20,	0x00,	1},
	{0x21,	0x55,	1},
	{0x50,	0x46,	1},  //add by ywq for 50Hz flicker free	2006-10-23 11:09
	{0x51,	0x3a,	1},  //add by ywq for 60Hz flicker free	2006-10-23 11:09
	
	{0x03,	0x03,	1},	//page 3
	{0x10,	0x0c,	1},
	{0x11,	0x80,	1},
	{0x12,	0x30,	1},
	{0x13,	0xa5,	1},
	{0x14,	0x30,	1},
	{0x15,	0x51,	1},
	{0x16,	0x47,	1},
	{0x19,	0x30,	1},
	{0x1C,	0x43,	1},
	{0x1d,	0x23,	1},
	{0x1e,	0x20,	1},
	{0x26,	0x95,	1},
	{0x33,	0x00,	1},
	{0x34,	0x30,	1},
	{0x35,	0xd4,	1},
	{0x36,	0x01,	1},
	{0x37,	0x00,	1},
	{0x38,	0x02,	1},	//max exposure time decides min frame rate=10fps 50Hz
	{0x39,	0x49,	1},
	{0x3a,	0xf0,	1},	
	{0x3b,	0x3a,	1},	//EXP100Hz
	{0x3c,	0x98,	1},
	{0x3d,	0x30,	1},	//EXP120Hz
	{0x3e,	0xd4,	1},
	{0x50,	0x16,	1},
	{0x51,	0x10,	1},
	{0x52,	0x50,	1},
	{0x53,	0x10,	1},
	{0x54,	0x10,	1},
	{0x55,	0x28,	1},
	{0x56,	0x30,	1},
	{0x57,	0x30,	1},
	{0x58,	0x60,	1},
	{0x59,	0x22,	1},
	{0x5a,	0x1c,	1},
	{0x5b,	0x19,	1},
	{0x5c,	0x16,	1},
	{0x5d,	0x14,	1},
	{0x5e,	0x20,	1},
	{0x10,	0x8c,	1},
	{0x10,	0x9c,	1},	//changed by ywq manual flicker 100Hz	2006-10-17 14:56
	
	{0x03,	0x04,	1},	//page 4
	{0x10,	0xe9,	1},
	{0x19,	0x55,	1},
	{0x40,	0x3e,	1},
	{0x41,	0x20,	1},
	{0x42,	0x35,	1},
	{0x43,	0x50,	1},
	{0x44,	0x20,	1},
	{0x45,	0x50,	1},
	{0x46,	0x20,	1},
	{0x60,	0x80,	1},
	{0x61,	0x80,	1},
	
	{0x03,	0x00,	1},	//page 0
	{0x01,	0x00,	1},
};

const TSnrSizeCfg gSensorSizeCfg_MC501CB[] = 
{
	{
            {0,NULL},
               	{0,NULL},
	      	{640, 480},
		1,
		1


	}
	
};

const TSnrInfo gSensorInfo_MC501CB = 
{
       "MC501CB yuv mode",
	VIM_SNR_YUV,							//yuv mode
	0,							//not use PCLK of sensor
	0,							//[0]: VSync delay enable; [1]: Fifo VSync delay enable; [2]: Use PCLK Negtive Edge.
	0,							//0 -- I2C, 1 -- Serial bus;
	0,							//[0]: 1 -- OV I2C bus
	80,							//I2C Rate : 100KHz
	0x60,							//I2C address
	0x60,							//ISP I2C address for special sensor
	0x07,							//power config
	0,							//reset sensor
	3,							//brightness 
	3,							//contrast

	{0,NULL},											//sensor standby                                                                                  											                        
       {sizeof(gSnrSizeRegValVGA_MC501CB) / sizeof(TReg), (PTReg)&gSnrSizeRegValVGA_MC501CB},                                                       //snr initial value
	{sizeof(gSifPwrOnSeqRegVal_MC501CB) / sizeof(TReg), (PTReg)&gSifPwrOnSeqRegVal_MC501CB},	//Sif config sequence(Reg.800 bit0~2) when sensor power on                                                              	
	{0, NULL},											//Sif config sequence(Reg.800 bit0~2) when sensor standby                                                                 
	{sizeof(gSifRegVal_MC501CB) / sizeof(TReg), (PTReg)&gSifRegVal_MC501CB},			//sif initial value                                                                                                       
	{sizeof(gSensorSizeCfg_MC501CB) / sizeof(TSnrSizeCfg), (PTSnrSizeCfg)&gSensorSizeCfg_MC501CB},	//size configure                                                                                  

	SensorSetReg_MC501CB,													
	SensorGetReg_MC501CB,													
	CheckId_MC501CB,													
	SensorSetMirrorFlip_MC501CB,												
	SensorSetContrast_MC501CB,																									//Sensor switch size callback
	SensorSetBrightness_MC501CB,																									//set et callback
	NULL,	
	SensorSetMode_MC501CB,
	NULL,															
	NULL,															
	{	//ythd of flash,et of flash
		0x20,			10
	},
};

#endif





/******************************************************************
	Transform from 568(v4.2c) RDK sensor.c at 2006-11-24 
******************************************************************/
#if V5_DEF_SNR_PO3030K_YUV	

void SensorSetReg_PO3030K(UINT16 uAddr, UINT16 uVal)
{
         V5B_SifI2cWriteByte((UINT8)uAddr, (UINT8)uVal);
}

void SensorGetReg_PO3030K(UINT8 uAddr, UINT16 *uVal)
{
	V5B_SifI2cReadByte((UINT8)uAddr, (UINT8*)uVal);
}

UINT8 CheckId_PO3030K(void)
{
	UINT8 temp1,temp2; 

        V5B_SifI2cReadByte(0x00, &temp1); 
        V5B_SifI2cReadByte(0x01, &temp2);   
        
        if(temp1==0x30&&temp2==0x30) 
                return SUCCEED; 
        else 
                return FAILED; 
}

void SensorSetMirrorFlip_PO3030K(UINT8 val)
{
	UINT8 x;
	val=~val;
 	
 	V5B_SifI2cReadByte(0x1e, &x);
	x&=0x3f;
	val = (val&0x03)<<6;
	x |= val;
	V5B_SifI2cWriteByte(0x1e, x);
}

///////////////////////////////////////////////////////////////////

/******************************************************************
         Desc: set sensor contrast  callback 
         Para: 1-5 step
*******************************************************************/
void SensorSetContrast_PO3030K(UINT8 val)
{
	switch(val)
	{	
	case 1:    
		V5B_SifI2cWriteByte(0x9c, 0x56);              
		break;  
	case 2:    
		V5B_SifI2cWriteByte(0x9c, 0x76);                                                
		break;
   	case 3:      
   		V5B_SifI2cWriteByte(0x9c, 0x96);                          
		break; 
        case 4:     
        V5B_SifI2cWriteByte(0x9c, 0xb6);                           
		break;  
	case 5:     
		V5B_SifI2cWriteByte(0x9c, 0xd6);                           
		break; 
	default:
		break;
	}					
}

/******************************************************************
         Desc: set sensor Brightness  callback 
         Para: 1-5 step
*******************************************************************/
void SensorSetBrightness_PO3030K(UINT8 val)
{
	switch(val)
	{
	case 1:
		V5B_SifI2cWriteByte(0x9b, 0xa0);
		break;	
	case 2:
		V5B_SifI2cWriteByte(0x9b, 0x90);
		break;
	case 3:
		V5B_SifI2cWriteByte(0x9b, 0x00);
		break;
	case 4:
		V5B_SifI2cWriteByte(0x9b, 0x10);
		break;
	case 5:
		V5B_SifI2cWriteByte(0x9b, 0x20);
		break;
	default:
		break;		
	}
}



/******************************************************************
         Desc: set sensor mode  callback 
         Para:  val: 	1 50hz,	2 60hz,	3 night, 4 outdoor.
*******************************************************************/
void SensorSetMode_PO3030K(UINT8 val)
{
	switch (val)
	{	
		case 1:
			/*
			V5B_SifI2cWriteByte(0xd4, 0x2c);
			VIM_USER_DelayMs(50);
			V5B_SifI2cWriteByte(0x46, 0x40);	//50Hz 
			VIM_USER_DelayMs(50);
			V5B_SifI2cWriteByte(0x1a, 0x02);
			V5B_SifI2cWriteByte(0x1b, 0x9a);
			V5B_SifI2cWriteByte(0xb0, 0x02);
			V5B_SifI2cWriteByte(0xb1, 0x9a);
			VIM_USER_DelayMs(50);
			V5B_SifI2cWriteByte(0xbb, 0x02);
			V5B_SifI2cWriteByte(0xbc, 0x9a);
			VIM_USER_DelayMs(50);
			V5B_SifI2cWriteByte(0xd4, 0x3c);
			VIM_USER_DelayMs(50);
			*/
    
			V5B_SifI2cWriteByte(0x46, 0x40);
            V5B_SifI2cWriteByte(0x49, 0x85);
            V5B_SifI2cWriteByte(0x4a, 0x55);
            V5B_SifI2cWriteByte(0xb9, 0x03);
            V5B_SifI2cWriteByte(0xba, 0xe6);
            V5B_SifI2cWriteByte(0xbb, 0x07);
            V5B_SifI2cWriteByte(0xbc, 0xcc);

			break;
		case 2:
			/*
			V5B_SifI2cWriteByte(0xd4, 0x2c);
			VIM_USER_DelayMs(50);
			V5B_SifI2cWriteByte(0x46, 0x20);	//60Hz 
			VIM_USER_DelayMs(50);
			V5B_SifI2cWriteByte(0x1a, 0x02);
			V5B_SifI2cWriteByte(0x1b, 0x9a);
			V5B_SifI2cWriteByte(0xb0, 0x02);
			V5B_SifI2cWriteByte(0xb1, 0x9a);
			VIM_USER_DelayMs(50);
			V5B_SifI2cWriteByte(0xbb, 0x02);
			V5B_SifI2cWriteByte(0xbc, 0x9a);
			VIM_USER_DelayMs(50);
			V5B_SifI2cWriteByte(0xd4, 0x3c);
			VIM_USER_DelayMs(50);
			*/

            V5B_SifI2cWriteByte(0x46, 0x20);
            V5B_SifI2cWriteByte(0x49, 0x6f);
            V5B_SifI2cWriteByte(0x4a, 0x1c);
            V5B_SifI2cWriteByte(0xb9, 0x03);
            V5B_SifI2cWriteByte(0xba, 0xe6);
            V5B_SifI2cWriteByte(0xbb, 0x07);
            V5B_SifI2cWriteByte(0xbc, 0xcc);

			break;
		case 4:  // 3 and 4 reverse
				//night  min10fps
			/*
			V5B_SifI2cWriteByte(0xd4, 0x2c);
			VIM_USER_DelayMs(50);
			V5B_SifI2cWriteByte(0x1a, 0x02);
			V5B_SifI2cWriteByte(0x1b, 0x99);
			V5B_SifI2cWriteByte(0xb0, 0x02);
			V5B_SifI2cWriteByte(0xb1, 0x9a);
			VIM_USER_DelayMs(50);
			V5B_SifI2cWriteByte(0xbb, 0x02);
			V5B_SifI2cWriteByte(0xbc, 0x9a);
			VIM_USER_DelayMs(50);
			V5B_SifI2cWriteByte(0xd4, 0x3c);
			VIM_USER_DelayMs(50);
			*/

            V5B_SifI2cWriteByte(0x46, 0x00);
            V5B_SifI2cWriteByte(0xb9, 0x0f);
            V5B_SifI2cWriteByte(0xba, 0x98);
            V5B_SifI2cWriteByte(0xbb, 0x3e);
            V5B_SifI2cWriteByte(0xbc, 0x60);


			break;
		case 3:	
				//outdoor
			/*
			V5B_SifI2cWriteByte(0xd4, 0x2c);
			VIM_USER_DelayMs(50);
			V5B_SifI2cWriteByte(0x1a, 0x02);
			V5B_SifI2cWriteByte(0x1b, 0x9a);
			V5B_SifI2cWriteByte(0xb0, 0x02);
			V5B_SifI2cWriteByte(0xb1, 0x9a);
			VIM_USER_DelayMs(50);
			V5B_SifI2cWriteByte(0xbb, 0x02);
			V5B_SifI2cWriteByte(0xbc, 0x9a);
			VIM_USER_DelayMs(50);
			V5B_SifI2cWriteByte(0xd4, 0x3c);
			VIM_USER_DelayMs(50);
			*/

            V5B_SifI2cWriteByte(0x46, 0x00);
            V5B_SifI2cWriteByte(0xb9, 0x03);
            V5B_SifI2cWriteByte(0xba, 0xe6);
            V5B_SifI2cWriteByte(0xbb, 0x07);
            V5B_SifI2cWriteByte(0xbc, 0xcc);

			V5B_SifI2cWriteByte(0x46, 0x40);
            V5B_SifI2cWriteByte(0x49, 0x85);
            V5B_SifI2cWriteByte(0x4a, 0x55);

			break;
		default:
			break;
	}
}

const TReg gSifPwrOnSeqRegVal_PO3030K[] =
{
	
	{0x00, 0x61,	1},
	{0x00, 0x60,	1},
	{0x00, 0x61,	1}
	
};

const TReg gSifRegVal_PO3030K[] =
{        
    {0x02,  0x00, 0},
  	{0x03,  0x08, 0},
	{0x04,  0x04, 0},
	{0x5c,  0x01, 0},
	{0x01,  0x41, 0}
};

const TReg gSnrSizeRegValVGA_PO3030K[] = 
{   
	/*
	{0x12, 0x06, 2},
	{0x13, 0x08, 2},
	{0x1e, 0x06, 2},
	{0x21, 0x00, 2},
	{0x24, 0x02, 2},
	{0x32, 0x69, 2},
	{0x36, 0x35, 2},
	{0x39, 0x60, 2},
	{0x3b, 0x00, 2}, //ADCoffset
	{0x4d, 0xfe, 2},
	{0x53, 0x1c, 2},
	{0x5c, 0x6c, 2}, //edge gain x1.5
	{0x5d, 0x04, 2},
	{0x5f, 0x08, 2},
	{0x60, 0x08, 2},
	{0x61, 0x08, 2},
	{0x62, 0x08, 2},
	{0x63, 0x00, 2},
	{0x64, 0x00, 2},
	{0x65, 0x10, 2},
	{0x66, 0x14, 2},
	{0x73, 0x68, 2},
	{0x82, 0x01, 2},
	{0x83, 0x80, 2},
	{0x85, 0x0c, 2},
	{0x86, 0xb6, 2},
	{0x89, 0x00, 2},
	{0xad, 0x3f, 2},
	{0xae, 0x24, 2},
	{0xb3, 0x68, 2},
	{0xb4, 0x03, 2},
	{0xb8, 0x10, 2},
	{0xb9, 0x10, 2},
	{0xba, 0xcc, 2},
	{0xbb, 0x1f, 2},
	{0xbc, 0x30, 2},
	{0xbf, 0x02, 2},
	{0x8e, 0x30, 2}, //color correction
	{0x8f, 0x90, 2},
	{0x90, 0x00, 2},
	{0x91, 0x85, 2},
	{0x92, 0x34, 2},
	{0x93, 0x8f, 2},
	{0x94, 0x02, 2},
	{0x95, 0x96, 2},
	{0x96, 0x34, 2},
	
	{0xd8, 0x80, 2}, //Rgamma	
	{0x76, 0x00, 2}, 
	{0x77, 0x00, 2},
	{0x78, 0x01, 2},
	{0x79, 0x04, 2},
	{0x7a, 0x0c, 2},
	{0x7b, 0x1e, 2},
	{0x7c, 0x34, 2},
	{0x7d, 0x67, 2},
	{0x7e, 0x90, 2},
	{0x7f, 0xb3, 2},
	{0x80, 0xd1, 2},
	{0x81, 0xea, 2},
	{0xd8, 0xa0, 2}, //Ggamma
	{0x76, 0x00, 2}, 
	{0x77, 0x00, 2},
	{0x78, 0x01, 2},
	{0x79, 0x04, 2},
	{0x7a, 0x0c, 2},
	{0x7b, 0x1e, 2},
	{0x7c, 0x34, 2},
	{0x7d, 0x67, 2},
	{0x7e, 0x90, 2},
	{0x7f, 0xb3, 2},
	{0x80, 0xd1, 2},
	{0x81, 0xea, 2},
	{0xd8, 0xc0, 2}, //Bgamma
	{0x76, 0x00, 2}, 
	{0x77, 0x00, 2},
	{0x78, 0x01, 2},
	{0x79, 0x04, 2},
	{0x7a, 0x0c, 2},
	{0x7b, 0x1e, 2},
	{0x7c, 0x34, 2},
	{0x7d, 0x67, 2},
	{0x7e, 0x90, 2},
	{0x7f, 0xb3, 2},
	{0x80, 0xd1, 2},
	{0x81, 0xea, 2},
	

	{0x97, 0x2a, 2},
	{0x98, 0x2a, 2},
	{0x9b, 0x20, 2}, //brightness
	{0x9c, 0x90, 2}, //contrast
	{0x4f, 0x2a, 2},
	{0x36, 0x35, 2}, // for 30fps
	{0x33, 0x37, 2},
	{0x2D, 0x00, 2},
	{0x2E, 0x87, 2},
	{0x2F, 0x03, 2},
	{0x30, 0x0B, 2},
	{0x08, 0x00, 2},
	{0x09, 0x9B, 2},
	{0x0C, 0x03, 2},
	{0x0D, 0x1B, 2},
	{0x34, 0x0A, 2},
	{0x40, 0x06, 2},
	{0x41, 0x38, 2},
	{0x04, 0x03, 2},
	{0x05, 0x1F, 2},
	{0x23, 0x01, 2},
	{0x46, 0x40, 2}, //50Hz flicker erase
	{0x49, 0x85, 2},
	{0x4a, 0x55, 2},
	{0x4b, 0x6f, 2},
	{0x4c, 0x1c, 2},
	*/


    // Pixel Plus

    {0x12, 0x06, 2},
	{0x13, 0x08, 2},
	{0x1e, 0x06, 2},
	{0x21, 0x00, 2},
	{0x24, 0x02, 2},
	{0x32, 0x69, 2},
	{0x36, 0x35, 2},
	{0x39, 0x60, 2},
	// {0x3b, 0x00, 2}, //ADCoffset
	{0x4d, 0xfe, 2},
	{0x53, 0x1c, 2},
	{0x5c, 0x70, 2},   //{0x5c, 0x6c, 2}, //edge gain x1.5
	{0x5d, 0x04, 2},
	{0x5f, 0x08, 2},
	{0x60, 0x08, 2},
	{0x61, 0x08, 2},
	{0x62, 0x08, 2},
	{0x63, 0x00, 2},
	{0x64, 0x00, 2},
	{0x65, 0x10, 2},
	{0x66, 0x14, 2},
	{0x73, 0x68, 2},
	{0x82, 0x01, 2},
	{0x83, 0x80, 2},
	{0x85, 0x0c, 2},
	{0x86, 0xb6, 2},
	{0x89, 0x00, 2},
	{0xad, 0x3f, 2},
	{0xae, 0x24, 2},
	{0xb3, 0x68, 2},
	{0xb4, 0x03, 2},
	{0xb8, 0x10, 2},
	{0xb9, 0x07, 2},  //{0xb9, 0x10, 2},
	{0xba, 0xcc, 2},
	{0xbb, 0x1f, 2},
	{0xbc, 0x30, 2},
	{0xbf, 0x02, 2},
	
	/*
	{0x8e, 0x30, 2}, //color correction
	{0x8f, 0x90, 2},
	{0x90, 0x00, 2},
	{0x91, 0x85, 2},
	{0x92, 0x34, 2},
	{0x93, 0x8f, 2},
	{0x94, 0x02, 2},
	{0x95, 0x96, 2},
	{0x96, 0x34, 2},
    */	

    {0x8e, 0x42, 2}, //color correction
	{0x8f, 0x95, 2},
	{0x90, 0x8d, 2},
	{0x91, 0x89, 2},
	{0x92, 0x3a, 2},
	{0x93, 0x90, 2},
	{0x94, 0x84, 2},
	{0x95, 0x89, 2},
	{0x96, 0x2d, 2},



    /*
	{0xd8, 0x80, 2}, //Rgamma	
	{0x76, 0x00, 2}, 
	{0x77, 0x00, 2},
	{0x78, 0x01, 2},
	{0x79, 0x04, 2},
	{0x7a, 0x0c, 2},
	{0x7b, 0x1e, 2},
	{0x7c, 0x34, 2},
	{0x7d, 0x67, 2},
	{0x7e, 0x90, 2},
	{0x7f, 0xb3, 2},
	{0x80, 0xd1, 2},
	{0x81, 0xea, 2},
	*/


    {0xd8, 0x80, 2}, //Rgamma	
	{0x76, 0x00, 2}, 
	{0x77, 0x0d, 2},
	{0x78, 0x18, 2},
	{0x79, 0x22, 2},
	{0x7a, 0x2c, 2},
	{0x7b, 0x3e, 2},
	{0x7c, 0x4f, 2},
	{0x7d, 0x6f, 2},
	{0x7e, 0x8e, 2},
	{0x7f, 0xac, 2},
	{0x80, 0xc8, 2},
	{0x81, 0xe5, 2},


    /*
	{0xd8, 0xa0, 2}, //Ggamma
	{0x76, 0x00, 2}, 
	{0x77, 0x00, 2},
	{0x78, 0x01, 2},
	{0x79, 0x04, 2},
	{0x7a, 0x0c, 2},
	{0x7b, 0x1e, 2},
	{0x7c, 0x34, 2},
	{0x7d, 0x67, 2},
	{0x7e, 0x90, 2},
	{0x7f, 0xb3, 2},
	{0x80, 0xd1, 2},
	{0x81, 0xea, 2},
    */

    {0xd8, 0xa0, 2}, //Ggamma
	{0x76, 0x00, 2}, 
	{0x77, 0x0d, 2},
	{0x78, 0x18, 2},
	{0x79, 0x22, 2},
	{0x7a, 0x2c, 2},
	{0x7b, 0x3e, 2},
	{0x7c, 0x4f, 2},
	{0x7d, 0x6f, 2},
	{0x7e, 0x8e, 2},
	{0x7f, 0xac, 2},
	{0x80, 0xc8, 2},
	{0x81, 0xe5, 2},



    /*
	{0xd8, 0xc0, 2}, //Bgamma
	{0x76, 0x00, 2}, 
	{0x77, 0x00, 2},
	{0x78, 0x01, 2},
	{0x79, 0x04, 2},
	{0x7a, 0x0c, 2},
	{0x7b, 0x1e, 2},
	{0x7c, 0x34, 2},
	{0x7d, 0x67, 2},
	{0x7e, 0x90, 2},
	{0x7f, 0xb3, 2},
	{0x80, 0xd1, 2},
	{0x81, 0xea, 2},
	*/

    {0xd8, 0xc0, 2}, //Bgamma
	{0x76, 0x00, 2}, 
	{0x77, 0x0d, 2},
	{0x78, 0x18, 2},
	{0x79, 0x22, 2},
	{0x7a, 0x2c, 2},
	{0x7b, 0x3e, 2},
	{0x7c, 0x4f, 2},
	{0x7d, 0x6f, 2},
	{0x7e, 0x8e, 2},
	{0x7f, 0xac, 2},
	{0x80, 0xc8, 2},
	{0x81, 0xe5, 2},

    {0x4f, 0x2a, 2},  // add by fengyu (Vsync pulse)
    
	{0x46, 0x40, 2},
    {0x49, 0x85, 2},
    {0x4a, 0x55, 2},
	{0xb9, 0x03, 2},
	{0xba, 0xe6, 2},
	{0xbb, 0x07, 2},
	{0xbc, 0xcc, 2}


/*
	{0x97, 0x2a, 2},
	{0x98, 0x2a, 2},
	{0x9b, 0x20, 2}, //brightness
	{0x9c, 0x90, 2}, //contrast
	{0x4f, 0x2a, 2},
	{0x36, 0x35, 2}, // for 30fps
	{0x33, 0x37, 2},
	{0x2D, 0x00, 2},
	{0x2E, 0x87, 2},
	{0x2F, 0x03, 2},
	{0x30, 0x0B, 2},
	{0x08, 0x00, 2},
	{0x09, 0x9B, 2},
	{0x0C, 0x03, 2},
	{0x0D, 0x1B, 2},
	{0x34, 0x0A, 2},
	{0x40, 0x06, 2},
	{0x41, 0x38, 2},
	{0x04, 0x03, 2},
	{0x05, 0x1F, 2},
	{0x23, 0x01, 2},
	{0x46, 0x40, 2}, //50Hz flicker erase
	{0x49, 0x85, 2},
	{0x4a, 0x55, 2},
	{0x4b, 0x6f, 2},
	{0x4c, 0x1c, 2}
*/
};

const TSnrSizeCfg gSensorSizeCfg_PO3030K[] = 
{
	{
               	{0,NULL},
               	{0,NULL},
	      	{640, 480},
		1,
		1


	}
	
};

const TSnrInfo gSensorInfo_PO3030K = 
{
       "PO3030K yuv mode",
	VIM_SNR_YUV,							//yuv mode
	0,							//not use PCLK of sensor
	0,							//[0]: VSync delay enable; [1]: Fifo VSync delay enable; [2]: Use PCLK Negtive Edge.
	0,							//0 -- I2C, 1 -- Serial bus;
	0,							//[0]: 1 -- OV I2C bus
	100,							//I2C Rate : 100KHz
	0xdc,							//I2C address
	0xdc,							//ISP I2C address for special sensor
	0x01,							//power config
	0,							//reset sensor
	3,							//brightness 
	3,							//contrast

	{0,NULL},											//sensor standby                                                                                  								
        {sizeof(gSnrSizeRegValVGA_PO3030K) / sizeof(TReg), (PTReg)&gSnrSizeRegValVGA_PO3030K},                                                       //snr initial value
	{sizeof(gSifPwrOnSeqRegVal_PO3030K) / sizeof(TReg), (PTReg)&gSifPwrOnSeqRegVal_PO3030K},	//Sif config sequence(Reg.800 bit0~2) when sensor power on                                        
	{0, NULL},											//Sif config sequence(Reg.800 bit0~2) when sensor standby                                         
	{sizeof(gSifRegVal_PO3030K) / sizeof(TReg), (PTReg)&gSifRegVal_PO3030K},			//sif initial value                                                                               
	{sizeof(gSensorSizeCfg_PO3030K) / sizeof(TSnrSizeCfg), (PTSnrSizeCfg)&gSensorSizeCfg_PO3030K},	//size configure                                                                                  

	SensorSetReg_PO3030K,										
	SensorGetReg_PO3030K,										
	CheckId_PO3030K,											
	SensorSetMirrorFlip_PO3030K,									
	SensorSetContrast_PO3030K,
	SensorSetBrightness_PO3030K,
	NULL,	
	SensorSetMode_PO3030K,
	NULL,
	NULL,												
	{	//ythd of flash,et of flash
		0x20,			10
	},
};

#endif



/******************************************************************
	Add  V5_DEF_SNR_SIV100A_YUV 2007-1-25 
******************************************************************/
#if V5_DEF_SNR_SIV100A_YUV	

void SensorSetReg_SIV100A(UINT16 uAddr, UINT16 uVal)
{
         V5B_SifI2cWriteByte((UINT8)uAddr, (UINT8)uVal);
}

void SensorGetReg_SIV100A(UINT8 uAddr, UINT16 *uVal)
{
	V5B_SifI2cReadByte((UINT8)uAddr, (UINT8*)uVal);
}

UINT8 CheckId_SIV100A(void)
{
	UINT8 temp1;
	UINT8 temp2; 

        V5B_SifI2cReadByte(0x01, &temp1);
	 V5B_SifI2cReadByte(0x02, &temp2);    
        if((temp1==0x0C) && (temp2==0x10))
        //if((temp1==0x12) && (temp2==0x13))
                return SUCCEED; 
        else 
                return FAILED;  
}

void SensorSetMirrorFlip_SIV100A(UINT8 val)
{
        UINT8 x;
//	val=~val;
	if(val==0x01)
		val=0x2;
	else if(val==0x02)
		val=0x01;
 	V5B_SifI2cReadByte(0x04, &x);
 	x&=0xfc;//clear bit0,bit1
	x |= (val&0x03);
 	V5B_SifI2cWriteByte(0x04, x);
}

/******************************************************************
         Desc: set sensor contrast  callback 
         Para: 1-5 step
*******************************************************************/
void SensorSetContrast_SIV100A(UINT8 val)
{
	switch(val)
	{	
	case 1:    
		V5B_SifI2cWriteByte(0xe0, 0x0c);              
		break;  
	case 2:    
		V5B_SifI2cWriteByte(0xe0, 0x10);                                                
		break;
   	case 3:      
   		V5B_SifI2cWriteByte(0xe0, 0x14);                          
		break; 
        case 4:     
        	V5B_SifI2cWriteByte(0xe0, 0x17);                           
		break;  
	case 5:     
		V5B_SifI2cWriteByte(0xe0, 0x1a);                           
		break; 
	default:
		break;
	}					
}

/******************************************************************
         Desc: set sensor Brightness  callback 
         Para: 1-5 step
*******************************************************************/
void SensorSetBrightness_SIV100A(UINT8 val)
{
	switch(val)
	{
	case 1:
		V5B_SifI2cWriteByte(0xe3, 0xa0);
		break;	
	case 2:
		V5B_SifI2cWriteByte(0xe3, 0x90);
		break;
	case 3:
		V5B_SifI2cWriteByte(0xe3, 0x00);
		break;
	case 4:
		V5B_SifI2cWriteByte(0xe3, 0x10);
		break;
	case 5:
		V5B_SifI2cWriteByte(0xe3, 0x20);
		break;
	default:
		break;		
	}
}

/******************************************************************
         Desc: set sensor mode  callback 
         Para:  val: 	1 50hz,	2 60hz,	3 night, 4 outdoor.
*******************************************************************/
void SensorSetMode_SIV100A(UINT8 val)
{	
	UINT8 uTemp;
	switch (val)
	{	
		case 1:
			V5B_SifI2cWriteByte(0x40, 0x02);	//disable AE & set 50Hz
			V5B_SifI2cWriteByte(0x33, 0x05);	//50Hz 20fps
			VIM_USER_DelayMs(100);
			V5B_SifI2cWriteByte(0x40, 0x9f);	//enable AE
			VIM_USER_DelayMs(100);
			break;
		case 2:
			V5B_SifI2cWriteByte(0x40, 0x00);	//disable AE & set 60Hz
			V5B_SifI2cWriteByte(0x33, 0x06);	//60Hz 20fps
			VIM_USER_DelayMs(100);
			V5B_SifI2cWriteByte(0x40, 0x9d);
			VIM_USER_DelayMs(100);
	
			break;
		case 4:  // 3 and 4 reverse
			V5B_SifI2cReadByte(0x40, &uTemp);	
			if ((uTemp&0x02) == 0x02)				//50Hz			
			{	
				V5B_SifI2cWriteByte(0x40, 0x02);
				V5B_SifI2cWriteByte(0x33, 0x0a);	//50Hz 10fps
				VIM_USER_DelayMs(100);
				V5B_SifI2cWriteByte(0x40, 0x9f);
				VIM_USER_DelayMs(100);
			}
			else 
			{	
				V5B_SifI2cWriteByte(0x40, 0x00);
				V5B_SifI2cWriteByte(0x33, 0x0c);	//60Hz 10fps
				VIM_USER_DelayMs(100);
				V5B_SifI2cWriteByte(0x40, 0x9d);
				VIM_USER_DelayMs(100);
			}
			
			break;
		case 3:	
			V5B_SifI2cReadByte(0x40, &uTemp);	
			if ((uTemp&0x02) == 0x02)				//50Hz 
			{	
				V5B_SifI2cWriteByte(0x40, 0x02);
				V5B_SifI2cWriteByte(0x33, 0x05);	//outdoor 50Hz
				VIM_USER_DelayMs(100);
				V5B_SifI2cWriteByte(0x40, 0x9e);
				VIM_USER_DelayMs(100);
			}
			else 
			{	
				V5B_SifI2cWriteByte(0x40, 0x00);
				V5B_SifI2cWriteByte(0x33, 0x06);	//outdoor 60Hz
				VIM_USER_DelayMs(100);
				V5B_SifI2cWriteByte(0x40, 0x9c);
				VIM_USER_DelayMs(100);
			}
			
			break;
		default:
			break;
	}
}

const TReg gSifPwrOnSeqRegVal_SIV100A[] =
{
	{0x00, 0x66,	1},
	{0x00, 0x67,	1}
};

const TReg gSifRegVal_SIV100A[] =
{        
 	  {0x02,  0x00, 0},
	  {0x03,  0x0a, 0},
	  {0x04,  0x05, 0},
   	  {0x01,  0x41, 0} 
};

const TReg gSnrSizeRegValVGA_SIV100A[] = 
{      
	 {0x04, 0x03, 1}, 
        {0x10, 0x0c, 1},
	 {0x11, 0x05, 1},
	 {0x12, 0x0a, 1},
	 {0x18, 0x20, 1},
	 {0x19, 0x3c, 1},
	 {0x20, 0x00, 1}, 
//	 {0x21, 0xc4, 1}, //60Hz-24fps
	 {0x21, 0x12, 1}, //60Hz-30fps
	 {0x22, 0x03, 1},
//	 {0x34, 0x64, 1},
	 {0x34, 0x7a, 1},
	 {0x33, 0x0c, 1},
	
	 {0x23, 0x00, 1}, 
//	 {0x24, 0x9c, 1}, //50Hz-25fps
	 {0x24, 0x12, 1}, //50Hz-30fps
	 {0x25, 0x03, 1},
//	 {0x35, 0x7d, 1},
	 {0x35, 0x93, 1},
	 {0x33, 0x0c, 1},
	 
	 {0x40, 0x9e, 1}, //AE enable 
//	 {0x41, 0x80, 1},
	 {0x41, 0x8a, 1}, //changed 2007-01-29
//	 {0x42, 0x45, 1}, //flashing, use default value 0x77
	 {0x43, 0xc0, 1}, 
	 {0x44, 0x4f, 1},
	 {0x45, 0x24, 1},
	 {0x46, 0x0c, 1},
	 {0x4c, 0x0c, 1},
	 {0x5a, 0x00, 1},
	 {0x60, 0x9c, 1}, // auto white balance
	 {0x62, 0x21, 1},
	 {0x63, 0x80, 1},
	 {0x64, 0x80, 1},
	 {0x65, 0xc8, 1},
	 {0x66, 0xa0, 1},
	 {0x67, 0xbc, 1},
	 {0x68, 0x78, 1},
	 {0x69, 0x20, 1},
	 {0x6a, 0x30, 1},
	 {0x80, 0xbf, 1},
	 {0x83, 0x40, 1},
	 {0x85, 0xa1, 1},
	 {0x86, 0x00, 1},
	 {0x87, 0x24, 1},
	 {0x88, 0x0f, 1},
	 {0x89, 0x10, 1},
	 {0x8d, 0x7c, 1},
	 {0xa0, 0x40, 1}, // black level cancealment
	 {0xa5, 0x14, 1},
	 {0xa6, 0x14, 1},
	 {0xa7, 0x14, 1},
	 {0xa8, 0x14, 1},

	 {0xc0, 0x00, 1}, //gamma 2 it seems the clarity is higher using default gamma when preview on LCD panel
	 {0xc1, 0x04, 1},
	 {0xc2, 0x09, 1},
	 {0xc3, 0x14, 1},
	 {0xc4, 0x28, 1},
	 {0xc5, 0x3e, 1},
	 {0xc6, 0x57, 1},
	 {0xc7, 0x73, 1},
	 {0xc8, 0x88, 1},
	 {0xc9, 0x9c, 1},
	 {0xca, 0xab, 1},
	 {0xcb, 0xc6, 1},
	 {0xcc, 0xdc, 1},
	 {0xcd, 0xf0, 1},
	 {0xce, 0xf9, 1},
	 {0xcf, 0xff, 1},
	 
	 {0xd0, 0x33, 1}, //color matrix
	 {0xd1, 0xd3, 1},
	 {0xd2, 0xfa, 1},
	 {0xd3, 0x13, 1},
	 {0xd4, 0x1A, 1},
	 {0xd5, 0x13, 1},
	 {0xd6, 0xf7, 1},
	 {0xd7, 0xb6, 1},
	 {0xd8, 0x53, 1},
	 {0xd9, 0xa8, 1},
	 {0xda, 0x20, 1},
	 {0xdb, 0x24, 1},
	 {0xdc, 0xf0, 1},
 // contrast & saturation
//	 {0xe0, 0x14, 1}, //default gamma with this value
	 {0xe0, 0x12, 1}, // or gamma 2 with this value, 
	 {0xe1, 0x14, 1}, //?0x14
	 {0xe2, 0x14, 1}, //?0x14
	 {0xe3, 0x00, 1},
//	 {0xe3, 0x10, 1},
	 {0xe4, 0xff, 1},
	 {0xe5, 0x00, 1},
	 {0xe6, 0xff, 1},
	 {0xe7, 0x00, 1},
	 {0xe8, 0xff, 1},
	 {0xe9, 0x00, 1},
	 {0xea, 0x24, 1},
	 {0xeb, 0xc8, 1},
	 {0x03, 0x05, 1},
	 {0x7a, 0xb0, 1},
	 {0x7b, 0xb0, 1},
};

const TSnrSizeCfg gSensorSizeCfg_SIV100A[] = 
{
	{
              {0,NULL},
              {0,NULL},
	      	{640, 480},
		1,
		1

	}
};

const TSnrInfo gSensorInfo_SIV100A = 
{
       "SIV100A yuv mode",
	VIM_SNR_YUV,							//yuv mode
	0,							//not use PCLK of sensor
	0,							//[0]: VSync delay enable; [1]: Fifo VSync delay enable; [2]: Use PCLK Negtive Edge.
	0,							//0 -- I2C, 1 -- Serial bus;
	0,							//[0]: 1 -- OV I2C bus
	100,							//I2C Rate : 100KHz
	0x66,						//I2C address
	0x66,						//ISP I2C address for special sensor
	0x07,						//power config
	0,							//reset sensor
	3,							//brightness 
	3,							//contrast

	{0,NULL},					//sensor standby                                                          
       {sizeof(gSnrSizeRegValVGA_SIV100A) / sizeof(TReg), (PTReg)&gSnrSizeRegValVGA_SIV100A},                                                       //snr initial value
	{sizeof(gSifPwrOnSeqRegVal_SIV100A) / sizeof(TReg), (PTReg)&gSifPwrOnSeqRegVal_SIV100A},	//Sif config sequence(Reg.800 bit0~2) when sensor power on                
	{0, NULL},											//Sif config sequence(Reg.800 bit0~2) when sensor standby             
	{sizeof(gSifRegVal_SIV100A) / sizeof(TReg), (PTReg)&gSifRegVal_SIV100A},			//sif initial value                                                       
	{sizeof(gSensorSizeCfg_SIV100A) / sizeof(TSnrSizeCfg), (PTSnrSizeCfg)&gSensorSizeCfg_SIV100A},	//size configure                                                          

	SensorSetReg_SIV100A,																				
	SensorGetReg_SIV100A,																				
	CheckId_SIV100A,																				
	SensorSetMirrorFlip_SIV100A,																			
	SensorSetContrast_SIV100A,																									//Sensor switch size callback
	SensorSetBrightness_SIV100A,																									//set et callback
	NULL,	
	SensorSetMode_SIV100A,
	NULL,																						
	NULL,																						
	{	//ythd of flash,et of flash
		0x20,			10
	},
};

#if 0
const TSnrInfo gSensorInfo_SIV100A = 
{
       "SIV100A yuv mode",
	VIM_SNR_YUV,							//yuv mode
	0,							//not use PCLK of sensor
	0,							//[0]: VSync delay enable; [1]: Fifo VSync delay enable; [2]: Use PCLK Negtive Edge.
	0,							//0 -- I2C, 1 -- Serial bus;
	0,							//[0]: 1 -- OV I2C bus
	100,							//I2C Rate : 100KHz
	0x66,						//I2C address
	0x66,						//ISP I2C address for special sensor
	0x07,						//power config
	0,							//reset sensor
	3,							//brightness 
	3,							//contrast

	{0,NULL},					//sensor standby                                                          
       {sizeof(gSnrSizeRegValVGA_SIV100A) / sizeof(TReg), (PTReg)&gSnrSizeRegValVGA_SIV100A},                                                       //snr initial value
	{sizeof(gSifPwrOnSeqRegVal_SIV100A) / sizeof(TReg), (PTReg)&gSifPwrOnSeqRegVal_SIV100A},	//Sif config sequence(Reg.800 bit0~2) when sensor power on                
	{0, NULL},											//Sif config sequence(Reg.800 bit0~2) when sensor standby             
	{sizeof(gSifRegVal_SIV100A) / sizeof(TReg), (PTReg)&gSifRegVal_SIV100A},			//sif initial value                                                       
	{sizeof(gSensorSizeCfg_SIV100A) / sizeof(TSnrSizeCfg), (PTSnrSizeCfg)&gSensorSizeCfg_SIV100A},	//size configure                                                          

	SensorSetReg_SIV100A,																				
	SensorGetReg_SIV100A,																				
	CheckId_SIV100A,																				
	SensorSetMirrorFlip_SIV100A,																			
	SensorSetContrast_SIV100A,																									//Sensor switch size callback
	SensorSetBrightness_SIV100A,																									//set et callback
	NULL,	
	SensorSetMode_SIV100A,
	NULL,																						
	NULL,																						
	{	//ythd of flash,et of flash
		0x20,			10
	},
};
#endif

#endif

#if V5_DEF_SNR_OV6680_YUV

/* write sensor register callback */

void SensorSetReg_OV6680(UINT16 uAddr, UINT16 uVal)

{
         V5B_SifI2cWriteByte((UINT8)uAddr, (UINT8)uVal);
}

void SensorGetReg_OV6680(UINT8 uAddr, UINT16 *uVal)
{
	V5B_SifI2cReadByte((UINT8)uAddr, (UINT8*)uVal);
}

UINT8 CheckId_OV6680(void)
{
	     UINT8 temp1;
         UINT8 temp2; 

         V5B_SifI2cReadByte(0x0a, &temp1);
         V5B_SifI2cReadByte(0x0b, &temp2);    
         if((temp1==0x66) && (temp2==0x81))
		 {      
                 return SUCCEED; 
		 }
         else 
         {      
			     return FAILED; 
		 }
}

/******************************************************************

         Desc: set sensor mirror and flip callback 

         Para: BIT0 -- mirror

                     BIT1 -- flip

*******************************************************************/

void SensorSetMirrorFlip_OV6680(UINT8 val)
{
        UINT8 x;
	val=~val;
	V5B_SifI2cReadByte(0x0c, &x);
	V5B_SifI2cWriteByte(0x0c, (UINT8)(x&0x3f)|((val&0x03)<<6));
}

/******************************************************************

         Desc: set sensor contrast  callback 

         Para: 1-5 step

*******************************************************************/

void SensorSetContrast_OV6680(UINT8 val)
{
	switch(val)
	{	
	case 1:                 
		V5B_SifI2cWriteByte(0x81, 0x1c);  
		V5B_SifI2cWriteByte(0x82, 0x30);  
		V5B_SifI2cWriteByte(0x83, 0x3F);  
		V5B_SifI2cWriteByte(0x84, 0x54);  
		V5B_SifI2cWriteByte(0x85, 0x6F);  
		V5B_SifI2cWriteByte(0x86, 0x79);  
		V5B_SifI2cWriteByte(0x87, 0x82);  
		V5B_SifI2cWriteByte(0x88, 0x8B);  
		V5B_SifI2cWriteByte(0x89, 0x92);  
		V5B_SifI2cWriteByte(0x8a, 0x99);  
		V5B_SifI2cWriteByte(0x8b, 0xA0);  
		V5B_SifI2cWriteByte(0x8c, 0xAC);  
		V5B_SifI2cWriteByte(0x8d, 0xB7);  
		V5B_SifI2cWriteByte(0x8e, 0xCA);  
		V5B_SifI2cWriteByte(0x8f, 0xDB);  
		V5B_SifI2cWriteByte(0x90, 0xEB);
		break;  
	case 2:                                                    
		V5B_SifI2cWriteByte(0x81, 0x20);  
		V5B_SifI2cWriteByte(0x82, 0x20);  
		V5B_SifI2cWriteByte(0x83, 0x2d);  
		V5B_SifI2cWriteByte(0x84, 0x42);  
		V5B_SifI2cWriteByte(0x85, 0x60);  
		V5B_SifI2cWriteByte(0x86, 0x6b);  
		V5B_SifI2cWriteByte(0x87, 0x76);  
		V5B_SifI2cWriteByte(0x88, 0x80);  
		V5B_SifI2cWriteByte(0x89, 0x8a);  
		V5B_SifI2cWriteByte(0x8a, 0x92);  
		V5B_SifI2cWriteByte(0x8b, 0x99);  
		V5B_SifI2cWriteByte(0x8c, 0xa6);  
		V5B_SifI2cWriteByte(0x8d, 0xaf);  
		V5B_SifI2cWriteByte(0x8e, 0xc4);  
		V5B_SifI2cWriteByte(0x8f, 0xd7);  
		V5B_SifI2cWriteByte(0x90, 0xe8);  
		break;
   	case 3:                                
		V5B_SifI2cWriteByte(0x81, 0x19);  
		V5B_SifI2cWriteByte(0x82, 0x0c);  
		V5B_SifI2cWriteByte(0x83, 0x18);  
		V5B_SifI2cWriteByte(0x84, 0x2f);  
		V5B_SifI2cWriteByte(0x85, 0x54);  
		V5B_SifI2cWriteByte(0x86, 0x64);  
		V5B_SifI2cWriteByte(0x87, 0x71);  
		V5B_SifI2cWriteByte(0x88, 0x7d);  
		V5B_SifI2cWriteByte(0x89, 0x88);  
		V5B_SifI2cWriteByte(0x8a, 0x91);  
		V5B_SifI2cWriteByte(0x8b, 0x98);  
		V5B_SifI2cWriteByte(0x8c, 0xa7);  
		V5B_SifI2cWriteByte(0x8d, 0xb4);  
		V5B_SifI2cWriteByte(0x8e, 0xcb);  
		V5B_SifI2cWriteByte(0x8f, 0xde);  
		V5B_SifI2cWriteByte(0x90, 0xed); 
		break; 
        case 4:                                
		V5B_SifI2cWriteByte(0x81, 0x19);  
		V5B_SifI2cWriteByte(0x82, 0x09);  
		V5B_SifI2cWriteByte(0x83, 0x13);  
		V5B_SifI2cWriteByte(0x84, 0x29);  
		V5B_SifI2cWriteByte(0x85, 0x4f);  
		V5B_SifI2cWriteByte(0x86, 0x5f);  
		V5B_SifI2cWriteByte(0x87, 0x6e);  
		V5B_SifI2cWriteByte(0x88, 0x7b);  
		V5B_SifI2cWriteByte(0x89, 0x87);  
		V5B_SifI2cWriteByte(0x8a, 0x91);  
		V5B_SifI2cWriteByte(0x8b, 0x9a);  
		V5B_SifI2cWriteByte(0x8c, 0xab);  
		V5B_SifI2cWriteByte(0x8d, 0xb6);  
		V5B_SifI2cWriteByte(0x8e, 0xcd);  
		V5B_SifI2cWriteByte(0x8f, 0xdf);  
		V5B_SifI2cWriteByte(0x90, 0xed);
		break;  
	case 5:                                
		V5B_SifI2cWriteByte(0x81, 0x2C);  
		V5B_SifI2cWriteByte(0x82, 0x11);  
		V5B_SifI2cWriteByte(0x83, 0x1a);  
		V5B_SifI2cWriteByte(0x84, 0x2a);  
		V5B_SifI2cWriteByte(0x85, 0x42);  
		V5B_SifI2cWriteByte(0x86, 0x4c);  
		V5B_SifI2cWriteByte(0x87, 0x56);  
		V5B_SifI2cWriteByte(0x88, 0x5f);  
		V5B_SifI2cWriteByte(0x89, 0x67);  
		V5B_SifI2cWriteByte(0x8a, 0x70);  
		V5B_SifI2cWriteByte(0x8b, 0x78);  
		V5B_SifI2cWriteByte(0x8c, 0x87);  
		V5B_SifI2cWriteByte(0x8d, 0x95);  
		V5B_SifI2cWriteByte(0x8e, 0xaf);  
		V5B_SifI2cWriteByte(0x8f, 0xc8);  
		V5B_SifI2cWriteByte(0x90, 0xdf); 
		break; 
	default:
		break;
	}					
}

/******************************************************************

         Desc: set sensor Brightness  callback 

         Para: 1-5 step

*******************************************************************/

void SensorSetBrightness_OV6680(UINT8 val)
{
	switch(val)
	{
	case 1:
        V5B_SifI2cWriteByte(0x60, 0xe7);
        V5B_SifI2cWriteByte(0x69, 0x04);
		V5B_SifI2cWriteByte(0x73, 0x06);
        V5B_SifI2cWriteByte(0x72, 0x50);
 


		break;	
	case 2:
        
        
        V5B_SifI2cWriteByte(0x60, 0xe7);
        V5B_SifI2cWriteByte(0x69, 0x04);
		V5B_SifI2cWriteByte(0x73, 0x06);
        V5B_SifI2cWriteByte(0x72, 0x30);
 



		break;
	case 3:
      

        V5B_SifI2cWriteByte(0x60, 0xe7);
        V5B_SifI2cWriteByte(0x69, 0x04);
		V5B_SifI2cWriteByte(0x73, 0x02);
        V5B_SifI2cWriteByte(0x72, 0x00);




		break;
	case 4:
		
		
		V5B_SifI2cWriteByte(0x60, 0xe7);
        V5B_SifI2cWriteByte(0x69, 0x04);
		V5B_SifI2cWriteByte(0x73, 0x02);
        V5B_SifI2cWriteByte(0x72, 0x30);



		break;
	case 5:
	    

        V5B_SifI2cWriteByte(0x60, 0xe7);
        V5B_SifI2cWriteByte(0x69, 0x04);
		V5B_SifI2cWriteByte(0x73, 0x02);
        V5B_SifI2cWriteByte(0x72, 0x50);

		break;
	default:
		break;		
	}
}

/******************************************************************

         Desc: set sensor mod  callback 

         Para:  val: 	1	50hz
         	    		2	60hz
         	     	    	3	night 
         	     	     	4	outdoor

*******************************************************************/

void SensorSetMode_OV6680(UINT8 val)
{

	switch (val)
	{	
		case 1:
			V5B_SifI2cWriteByte(0x0f, 0x25);   // 50hz 
            V5B_SifI2cWriteByte(0x4f, 0x83);	//50Hz; bandfilter
			V5B_SifI2cWriteByte(0x13, 0xcd);    //AWB Enable
			V5B_SifI2cWriteByte(0x2d, 0x00);    //LSB of insert dummy line
			V5B_SifI2cWriteByte(0x2e, 0x00);    //MSB of insert dummy line
			V5B_SifI2cWriteByte(0x13, 0xc0);    //AGC and AEC enable

			break;
		case 2:
			
			V5B_SifI2cWriteByte(0x13, 0xe0);
			V5B_SifI2cWriteByte(0x0f, 0x21);   // 60hz 
		    V5B_SifI2cWriteByte(0x50, 0x6d);
			V5B_SifI2cWriteByte(0x2d, 0x00);
			V5B_SifI2cWriteByte(0x2e, 0x00);


			break;
		case 4:  // 3 and 4 reverse
          
            V5B_SifI2cWriteByte(0x0f, 0x21);   // Night mode 
			V5B_SifI2cWriteByte(0x13, 0xcd);

			V5B_SifI2cWriteByte(0x2d, 0x10);
			V5B_SifI2cWriteByte(0x2e, 0x10);


            //V5B_SifI2cWriteByte(0x2d, 0x10);
			//V5B_SifI2cWriteByte(0x2e, 0x01);

			V5B_SifI2cWriteByte(0x13, 0xc0);
            

			break;
		case 3:	
		
			V5B_SifI2cWriteByte(0x13, 0xcd);  // Outdoor
			V5B_SifI2cWriteByte(0x2d, 0x00);
			V5B_SifI2cWriteByte(0x2e, 0x00);
			V5B_SifI2cWriteByte(0x13, 0xc0);
			

			break;
		default:
			break;
	}
}

const TReg gSifPwrOnSeqRegVal_OV6680[] =   //528 sif 0x800  power on
{
	{0x00, 0x63,	1},   //power on
	{0x00, 0x62,	1},   // reset
	{0x00, 0x63,	1}    // sensor enable
};

const TReg gSifRegVal_OV6680[] =  // SIF config (0x800)
{        
        {0x02, 0x02, 0},      // config
        {0x03, 0x1b, 0},      // Vsync
        {0x04, 0x05, 0},      // Hsync
        {0x21, 0x00, 0},      // vref_start  // fengyu
        {0x23, 0xea, 0},      // verf_stop
        {0x2c, 0x01, 0},      // Colmax registers
        {0x2d, 0x90, 0},      //
        {0x2e, 0x01, 0},      // Rowmax register
        {0x2f, 0x90, 0},      //
        {0x01, 0x45, 0} 
};

/* Sensor size control configure info */

const TReg gSnrSizeRegValVGA_OV6680[] = 
{      
	{0x12, 0x80, 0},	// SCCB registers reset
	{0x5c, 0x40, 0}, // decrease analog current
    {0x3d, 0x5a, 0},	// These three settings must be placed in sequence at initialization.
	{0x12, 0x00, 0},
	{0x0c, 0xd6, 0},   //{0x0c, 0x16, 0},  // Mirror and Flip  (fengyu)
	{0x3f, 0x1e, 0},	
    {0x0d, 0x30, 0},
    {0x06, 0x40, 0}, 
	{0x28, 0xd0, 0},
	{0x09, 0x01, 0},
	{0x17, 0x40, 0},
	{0x19, 0x03, 0},
	{0x18, 0xc8, 0},
	{0x1a, 0xc8, 0},
	{0x5a, 0xc8, 0},
	{0x5b, 0xc8, 0},
	{0x13, 0xf7, 0},
	{0x24, 0x50, 0}, 
	{0x25, 0x40, 0}, 
	{0x26, 0x92, 0},
	{0x21, 0x23, 0},
	{0x0f, 0x25, 0},
	{0x27, 0x34, 0},
	{0x29, 0x2C, 0},
	{0x5d, 0xA4, 0},
	{0x60, 0xe7, 0},
	{0x73, 0x06, 0},
	{0x74, 0x30, 0},
	{0x48, 0xc4, 0},
	{0x22, 0x81, 0},
	{0x3c, 0x45, 0},
	{0x20, 0x80, 0},
	{0x30, 0x80, 0},
	{0x31, 0x09, 0},
	{0x32, 0x00, 0},
	{0x33, 0x18, 0},
	{0x35, 0x00, 0},    
	{0x34, 0x12, 0},   
	{0x36, 0x10, 0},   
	{0x37, 0x1a, 0}, 
	{0x40, 0x00, 0},
	{0x41, 0x09, 0},
	{0x42, 0x00, 0},
	{0x43, 0x00, 0},
	{0x44, 0x00, 0},
	{0x45, 0x00, 0},
	{0x46, 0x00, 0},
	{0x47, 0x00, 0},
	{0x63, 0xef, 0},	
	{0x64, 0x02, 0},
	{0x65, 0x04, 0},
	{0x4a, 0x08, 0},
	{0x4b, 0x01, 0},
	{0x4c, 0xc0, 0},
	{0x59, 0x0b, 0},
	{0x5e, 0x20, 0},
	{0x5f, 0x31, 0},
    //saturation
	{0x79, 0x74, 0},
	{0x7a, 0x5f, 0},
	{0x7b, 0x15, 0},
	{0x7c, 0x14, 0},
	{0x7d, 0x68, 0},
	{0x7e, 0x7c, 0},
	{0x7f, 0x1e, 0},
	{0x66, 0xf2, 0},
    //gamma
    {0x81, 0x16, 0},
	{0x82, 0x0b, 0},
	{0x83, 0x16, 0},
	{0x84, 0x2a, 0},
	{0x85, 0x50, 0},
	{0x86, 0x60, 0},
	{0x87, 0x6e, 0},
	{0x88, 0x7b, 0},
	{0x89, 0x87, 0},
	{0x8a, 0x92, 0},
	{0x8b, 0x9c, 0},
	{0x8c, 0xac, 0},
	{0x8d, 0xba, 0},
	{0x8e, 0xd1, 0},
	{0x8f, 0xe2, 0},
	{0x90, 0xef, 0},
    {0x91, 0x00, 0},
	{0x11, 0x00, 0},
	{0x2a, 0x00, 0},
	{0x2b, 0x00, 0},
	{0x2c, 0x00, 0},
	{0x2d, 0x00, 0},
	{0x2e, 0x00, 0}	
};

/* Sensor size control configure info */

const TSnrSizeCfg gSensorSizeCfg_OV6680[] = 
{
	{
               {0,NULL}, 
               {0,NULL},     // sensor interface
	       {400, 400},  
//		0,         //0x805  MCLK (0:24M)
//		0          //0x806  PCLK (0:24M)
		1,
		1

	},

};

 

const TSnrInfo gSensorInfo_OV6680 = 

{
       "OV6680 yuv mode",
	VIM_SNR_YUV,							//yuv mode
	1,	//0x85c bit1  //High priority for 0x806				         //not use PCLK of sensor
	0,	//0x85c Bit4-bit2						//[0]: VSync delay enable; [1]: Fifo VSync delay enable; [2]: Use PCLK Negtive Edge.
	0,							//0 -- I2C, 1 -- Serial bus;
	1,							//[0]: 1 -- OV I2C bus
	80,							//I2C Rate : 100KHz
	0xc0,							//I2C address
	0xc0, 							//ISP I2C address for special sensor
	0x03,							//power config
	0,							//reset sensor

	3,							//brightness 
	3,							//contrast
	
	{0,NULL},														//sensor standby
	{sizeof(gSnrSizeRegValVGA_OV6680) / sizeof(TReg), (PTReg)&gSnrSizeRegValVGA_OV6680},    // Sensor Size List above      //snr initial value	
	
	{sizeof(gSifPwrOnSeqRegVal_OV6680) / sizeof(TReg), (PTReg)&gSifPwrOnSeqRegVal_OV6680},		//Sif config sequence(Reg.800 bit0~2) when sensor power on
	{0, NULL},																								//Sif config sequence(Reg.800 bit0~2) when sensor standby
	{sizeof(gSifRegVal_OV6680) / sizeof(TReg), (PTReg)&gSifRegVal_OV6680},						//sif initial value
																							//isp initial value
	{sizeof(gSensorSizeCfg_OV6680) / sizeof(TSnrSizeCfg), (PTSnrSizeCfg)&gSensorSizeCfg_OV6680},		//size configure 

	SensorSetReg_OV6680,																				//set reg callback
	SensorGetReg_OV6680,																				//get reg callback
	CheckId_OV6680,																				//Sensor ID Check Call
	SensorSetMirrorFlip_OV6680,																									//Set Sensor Mirror Flip Call
	SensorSetContrast_OV6680,																									//Sensor switch size callback
	SensorSetBrightness_OV6680,																									//set et callback
	NULL,	
	SensorSetMode_OV6680,
	NULL,																									//Sensor switch size callback
	NULL,
	{	//ythd of flash,et of flash
		0x20,			10
	},
};

#endif

////////////////////////////////////////////////////////////////
//
//    SP80818  (fengyu)
//
////////////////////////////////////////////////////////////////

#if V5_DEF_SNR_SP80818_YUV

/* write sensor register callback */

void SensorSetReg_SP80818(UINT16 uAddr, UINT16 uVal)

{
         V5B_SifI2cWriteByte((UINT8)uAddr, (UINT8)uVal);
}

void SensorGetReg_SP80818(UINT8 uAddr, UINT16 *uVal)
{
	V5B_SifI2cReadByte((UINT8)uAddr, (UINT8*)uVal);
}

UINT8 CheckId_SP80818(void)
{
	     UINT8 temp1;
         UINT8 temp2; 

         V5B_SifI2cReadByte(0x02, &temp1);
         V5B_SifI2cReadByte(0x02, &temp2); 
		 



         if((temp1==0x24) && (temp2==0x24))
		 {       
                 return SUCCEED; 
		 }
         else 
         {        
			     return FAILED; 
		 }
}

/******************************************************************

         Desc: set sensor mirror and flip callback 

         Para: BIT0 -- mirror

                     BIT1 -- flip

*******************************************************************/

void SensorSetMirrorFlip_SP80818(UINT8 val)
{
        UINT8 x;
	val=~val;
	V5B_SifI2cReadByte(0x0c, &x);
	V5B_SifI2cWriteByte(0x0c, (UINT8)(x&0x1f)|((val&0x03)<<5));
}

/******************************************************************

         Desc: set sensor contrast  callback 

         Para: 1-5 step

*******************************************************************/

void SensorSetContrast_SP80818(UINT8 val)
{
	switch(val)
	{	
	case 1:     
        V5B_SifI2cWriteByte(0x26, 0x8e);  
        V5B_SifI2cWriteByte(0x13, 0x60); 
		V5B_SifI2cWriteByte(0x14, 0x60); 
		
		break;  
	case 2:                                                    
		
		V5B_SifI2cWriteByte(0x26, 0x8e);  
        V5B_SifI2cWriteByte(0x13, 0x70); 
		V5B_SifI2cWriteByte(0x14, 0x70); 
		
		break;
   	case 3:                                
		
		V5B_SifI2cWriteByte(0x26, 0x8e);  
        V5B_SifI2cWriteByte(0x13, 0x80); 
		V5B_SifI2cWriteByte(0x14, 0x80); 

		break; 
    
	case 4:                                
		
		V5B_SifI2cWriteByte(0x26, 0x8e);  
        V5B_SifI2cWriteByte(0x13, 0x90); 
		V5B_SifI2cWriteByte(0x14, 0x90); 
		
		break;  
	case 5:                                
		
		V5B_SifI2cWriteByte(0x26, 0x8e);  
        V5B_SifI2cWriteByte(0x13, 0xa0); 
		V5B_SifI2cWriteByte(0x14, 0xa0); 
		break; 

	default:
		break;
	}					
}

/******************************************************************

         Desc: set sensor Brightness  callback 

         Para: 1-5 step

*******************************************************************/

void SensorSetBrightness_SP80818(UINT8 val)
{
	switch(val)
	{
	case 1:
		V5B_SifI2cWriteByte(0x77, 0xc0);

		break;	
	case 2:
		V5B_SifI2cWriteByte(0x77, 0xe0);

		break;
	case 3:
		V5B_SifI2cWriteByte(0x77, 0x00);

		break;
	case 4:
		V5B_SifI2cWriteByte(0x77, 0x20);

		break;
	case 5:   //Bright
		V5B_SifI2cWriteByte(0x77, 0x40);

		break;
	default:
		break;		
	}
}

/******************************************************************

         Desc: set sensor mod  callback 

         Para:  val: 	1	50hz
         	    		2	60hz
         	     	    	3	night 
         	     	     	4	outdoor

*******************************************************************/

void SensorSetMode_SP80818(UINT8 val)
{
	//UINT8 uTemp;
	switch (val)
	{	
		case 1:
            
             
                                 V5B_SifI2cWriteByte(0x9 ,0x1f);
		 		 		 		 V5B_SifI2cWriteByte(0xe7,0x0c);
		 		 		 		 V5B_SifI2cWriteByte(0xe8,0x8b);
		 		 		 		 V5B_SifI2cWriteByte(0x13,0x80);
		 		 		 		 V5B_SifI2cWriteByte(0x14,0xa0);
		 		 		 		 V5B_SifI2cWriteByte(0x24,0x00);
		 		 		 		 V5B_SifI2cWriteByte(0x26,0x8f);
		 		 		 		 V5B_SifI2cWriteByte(0x43,0x41);
		 		 		 		 V5B_SifI2cWriteByte(0x46,0x00);
		 		 		 		 V5B_SifI2cWriteByte(0x60,0xa0);
		 		 		 		 V5B_SifI2cWriteByte(0x61,0x65);
		 		 		 		 V5B_SifI2cWriteByte(0x62,0x35);
		 		 		 		 V5B_SifI2cWriteByte(0x63,0x10);
		 		 		 		 V5B_SifI2cWriteByte(0x69,0x07);
		 		 		 		 V5B_SifI2cWriteByte(0x6a,0x9a);
		 		 		 		 V5B_SifI2cWriteByte(0xea,0xa0);
		 		 		 		 V5B_SifI2cWriteByte(0xeb,0x80);
		 		 		 		 V5B_SifI2cWriteByte(0xec,0x50);
		 		 		 		 V5B_SifI2cWriteByte(0xed,0x10);
		 		 		 		 V5B_SifI2cWriteByte(0x77,0x00);
		 		 		 		 V5B_SifI2cWriteByte(0x78,0xa0);
		 		 		 		 V5B_SifI2cWriteByte(0x79,0xa0);
		 		 		 		 V5B_SifI2cWriteByte(0x7c,0x20);
		 		 		 		 V5B_SifI2cWriteByte(0x7d,0x25);
		 		 		 		 V5B_SifI2cWriteByte(0x31,0x11);
		 		 		 		 V5B_SifI2cWriteByte(0x32,0x19);
		 		 		 		 V5B_SifI2cWriteByte(0x33,0x26);
		 		 		 		 V5B_SifI2cWriteByte(0x34,0x2f);
		 		 		 		 V5B_SifI2cWriteByte(0x35,0x37);
		 		 		 		 V5B_SifI2cWriteByte(0x36,0x45);
		 		 		 		 V5B_SifI2cWriteByte(0x37,0x51);
		 		 		 		 V5B_SifI2cWriteByte(0x38,0x5c);
		 		 		 		 V5B_SifI2cWriteByte(0x39,0x65);
		 		 		 		 V5B_SifI2cWriteByte(0x3a,0x77);
		 		 		 		 V5B_SifI2cWriteByte(0x3b,0x87);
		 		 		 		 V5B_SifI2cWriteByte(0x3c,0x95);
		 		 		 		 V5B_SifI2cWriteByte(0x3d,0xae);
		 		 		 		 V5B_SifI2cWriteByte(0x3e,0xc5);
		 		 		 		 V5B_SifI2cWriteByte(0x3f,0xda);
		 		 		 		 V5B_SifI2cWriteByte(0x81,0x11);
		 		 		 		 V5B_SifI2cWriteByte(0x82,0x19);
		 		 		 		 V5B_SifI2cWriteByte(0x83,0x26);
		 		 		 		 V5B_SifI2cWriteByte(0x84,0x2f);
		 		 		 		 V5B_SifI2cWriteByte(0x85,0x37);
		 		 		 		 V5B_SifI2cWriteByte(0x86,0x45);
		 		 		 		 V5B_SifI2cWriteByte(0x87,0x51);
		 		 		 		 V5B_SifI2cWriteByte(0x88,0x5c);
		 		 		 		 V5B_SifI2cWriteByte(0x89,0x65);
		 		 		 		 V5B_SifI2cWriteByte(0x8a,0x77);
		 		 		 		 V5B_SifI2cWriteByte(0x8b,0x87);
		 		 		 		 V5B_SifI2cWriteByte(0x8c,0x95);
		 		 		 		 V5B_SifI2cWriteByte(0x8d,0xae);
		 		 		 		 V5B_SifI2cWriteByte(0x8e,0xc5);
		 		 		 		 V5B_SifI2cWriteByte(0x8f,0xda);
		 		 		 		 V5B_SifI2cWriteByte(0x91,0x11);
		 		 		 		 V5B_SifI2cWriteByte(0x92,0x19);
		 		 		 		 V5B_SifI2cWriteByte(0x93,0x26);
		 		 		 		 V5B_SifI2cWriteByte(0x94,0x2f);
		 		 		 		 V5B_SifI2cWriteByte(0x95,0x37);
		 		 		 		 V5B_SifI2cWriteByte(0x96,0x45);
		 		 		 		 V5B_SifI2cWriteByte(0x97,0x51);
		 		 		 		 V5B_SifI2cWriteByte(0x98,0x5c);
		 		 		 		 V5B_SifI2cWriteByte(0x99,0x65);
		 		 		 		 V5B_SifI2cWriteByte(0x9a,0x77);
		 		 		 		 V5B_SifI2cWriteByte(0x9b,0x87);
		 		 		 		 V5B_SifI2cWriteByte(0x9c,0x95);
		 		 		 		 V5B_SifI2cWriteByte(0x9d,0xae);
		 		 		 		 V5B_SifI2cWriteByte(0x9e,0xc5);
		 		 		 		 V5B_SifI2cWriteByte(0x9f,0xda);
		 		 		 		 V5B_SifI2cWriteByte(0x0d,0x19);
		 		 		 		 V5B_SifI2cWriteByte(0xe0,0x00);
                     
			break;

		case 2:
			
			    

                                 V5B_SifI2cWriteByte(0x09,0x1f);
		 		 		 		 V5B_SifI2cWriteByte(0xe7,0x0c);
		 		 		 		 V5B_SifI2cWriteByte(0xe8,0xa6); //
		 		 		 		 V5B_SifI2cWriteByte(0x13,0x80);
		 		 		 		 V5B_SifI2cWriteByte(0x14,0xa0);
		 		 		 		 V5B_SifI2cWriteByte(0x24,0x00);
		 		 		 		 V5B_SifI2cWriteByte(0x26,0x8f);
		 		 		 		 V5B_SifI2cWriteByte(0x43,0x41);
		 		 		 		 V5B_SifI2cWriteByte(0x46,0x00);
		 		 		 		 V5B_SifI2cWriteByte(0x60,0xa0);
		 		 		 		 V5B_SifI2cWriteByte(0x61,0x65);
		 		 		 		 V5B_SifI2cWriteByte(0x62,0x35);
		 		 		 		 V5B_SifI2cWriteByte(0x63,0x10);
		 		 		 		 V5B_SifI2cWriteByte(0x69,0x09); //
		 		 		 		 V5B_SifI2cWriteByte(0x6a,0x14); //
		 		 		 		 V5B_SifI2cWriteByte(0xea,0xa0);
		 		 		 		 V5B_SifI2cWriteByte(0xeb,0x80);
		 		 		 		 V5B_SifI2cWriteByte(0xec,0x50);
		 		 		 		 V5B_SifI2cWriteByte(0xed,0x10);
		 		 		 		 V5B_SifI2cWriteByte(0x77,0x00);
		 		 		 		 V5B_SifI2cWriteByte(0x78,0xa0);
		 		 		 		 V5B_SifI2cWriteByte(0x79,0xa0);
		 		 		 		 V5B_SifI2cWriteByte(0x7c,0x20);
		 		 		 		 V5B_SifI2cWriteByte(0x7d,0x25);
		 		 		 		 V5B_SifI2cWriteByte(0x31,0x11);
		 		 		 		 V5B_SifI2cWriteByte(0x32,0x19);
		 		 		 		 V5B_SifI2cWriteByte(0x33,0x26);
		 		 		 		 V5B_SifI2cWriteByte(0x34,0x2f);
		 		 		 		 V5B_SifI2cWriteByte(0x35,0x37);
		 		 		 		 V5B_SifI2cWriteByte(0x36,0x45);
		 		 		 		 V5B_SifI2cWriteByte(0x37,0x51);
		 		 		 		 V5B_SifI2cWriteByte(0x38,0x5c);
		 		 		 		 V5B_SifI2cWriteByte(0x39,0x65);
		 		 		 		 V5B_SifI2cWriteByte(0x3a,0x77);
		 		 		 		 V5B_SifI2cWriteByte(0x3b,0x87);
		 		 		 		 V5B_SifI2cWriteByte(0x3c,0x95);
		 		 		 		 V5B_SifI2cWriteByte(0x3d,0xae);
		 		 		 		 V5B_SifI2cWriteByte(0x3e,0xc5);
		 		 		 		 V5B_SifI2cWriteByte(0x3f,0xda);
		 		 		 		 V5B_SifI2cWriteByte(0x81,0x11);
		 		 		 		 V5B_SifI2cWriteByte(0x82,0x19);
		 		 		 		 V5B_SifI2cWriteByte(0x83,0x26);
		 		 		 		 V5B_SifI2cWriteByte(0x84,0x2f);
		 		 		 		 V5B_SifI2cWriteByte(0x85,0x37);
		 		 		 		 V5B_SifI2cWriteByte(0x86,0x45);
		 		 		 		 V5B_SifI2cWriteByte(0x87,0x51);
		 		 		 		 V5B_SifI2cWriteByte(0x88,0x5c);
		 		 		 		 V5B_SifI2cWriteByte(0x89,0x65);
		 		 		 		 V5B_SifI2cWriteByte(0x8a,0x77);
		 		 		 		 V5B_SifI2cWriteByte(0x8b,0x87);
		 		 		 		 V5B_SifI2cWriteByte(0x8c,0x95);
		 		 		 		 V5B_SifI2cWriteByte(0x8d,0xae);
		 		 		 		 V5B_SifI2cWriteByte(0x8e,0xc5);
		 		 		 		 V5B_SifI2cWriteByte(0x8f,0xda);
		 		 		 		 V5B_SifI2cWriteByte(0x91,0x11);
		 		 		 		 V5B_SifI2cWriteByte(0x92,0x19);
		 		 		 		 V5B_SifI2cWriteByte(0x93,0x26);
		 		 		 		 V5B_SifI2cWriteByte(0x94,0x2f);
		 		 		 		 V5B_SifI2cWriteByte(0x95,0x37);
		 		 		 		 V5B_SifI2cWriteByte(0x96,0x45);
		 		 		 		 V5B_SifI2cWriteByte(0x97,0x51);
		 		 		 		 V5B_SifI2cWriteByte(0x98,0x5c);
		 		 		 		 V5B_SifI2cWriteByte(0x99,0x65);
		 		 		 		 V5B_SifI2cWriteByte(0x9a,0x77);
		 		 		 		 V5B_SifI2cWriteByte(0x9b,0x87);
		 		 		 		 V5B_SifI2cWriteByte(0x9c,0x95);
		 		 		 		 V5B_SifI2cWriteByte(0x9d,0xae);
		 		 		 		 V5B_SifI2cWriteByte(0x9e,0xc5);
		 		 		 		 V5B_SifI2cWriteByte(0x9f,0xda);
		 		 		 		 V5B_SifI2cWriteByte(0x0d,0x19);
		 		 		 		 V5B_SifI2cWriteByte(0xe0,0x00);
                      
			break;

		case 4:  // 3 and 4 reverse
			   
                                 V5B_SifI2cWriteByte(0x09,0x07);
		 		 		 		 V5B_SifI2cWriteByte(0xe7,0x0c);
		 		 		 		 V5B_SifI2cWriteByte(0xe8,0x8b); 
		 		 		 		 V5B_SifI2cWriteByte(0x13,0x80);
		 		 		 		 V5B_SifI2cWriteByte(0x14,0x80);
		 		 		 		 V5B_SifI2cWriteByte(0x24,0xff);
		 		 		 		 V5B_SifI2cWriteByte(0x26,0x87);
		 		 		 		 V5B_SifI2cWriteByte(0x43,0xc1);
		 		 		 		 V5B_SifI2cWriteByte(0x46,0x05);
		 		 		 		 V5B_SifI2cWriteByte(0x60,0xd0);
		 		 		 		 V5B_SifI2cWriteByte(0x61,0x85);
		 		 		 		 V5B_SifI2cWriteByte(0x62,0x55);
		 		 		 		 V5B_SifI2cWriteByte(0x63,0x20);
		 		 		 		 V5B_SifI2cWriteByte(0x69,0x07); 
		 		 		 		 V5B_SifI2cWriteByte(0x6a,0x9a); 
		 		 		 		 V5B_SifI2cWriteByte(0xea,0xb0);
		 		 		 		 V5B_SifI2cWriteByte(0xeb,0x90);
		 		 		 		 V5B_SifI2cWriteByte(0xec,0x60);
		 		 		 		 V5B_SifI2cWriteByte(0xed,0x20);
		 		 		 		 V5B_SifI2cWriteByte(0x77,0x0f);
		 		 		 		 V5B_SifI2cWriteByte(0x78,0x90);
		 		 		 		 V5B_SifI2cWriteByte(0x79,0x90);
		 		 		 		 V5B_SifI2cWriteByte(0x7c,0x20);
		 		 		 		 V5B_SifI2cWriteByte(0x7d,0x40);
		 		 		 		 V5B_SifI2cWriteByte(0x31,0x08);
		 		 		 		 V5B_SifI2cWriteByte(0x32,0x0d);
		 		 		 		 V5B_SifI2cWriteByte(0x33,0x16);
		 		 		 		 V5B_SifI2cWriteByte(0x34,0x1e);
		 		 		 		 V5B_SifI2cWriteByte(0x35,0x24);
		 		 		 		 V5B_SifI2cWriteByte(0x36,0x30);
		 		 		 		 V5B_SifI2cWriteByte(0x37,0x3b);
		 		 		 		 V5B_SifI2cWriteByte(0x38,0x45);
		 		 		 		 V5B_SifI2cWriteByte(0x39,0x4f);
		 		 		 		 V5B_SifI2cWriteByte(0x3a,0x61);
		 		 		 		 V5B_SifI2cWriteByte(0x3b,0x71);
		 		 		 		 V5B_SifI2cWriteByte(0x3c,0x80);
		 		 		 		 V5B_SifI2cWriteByte(0x3d,0x9d);
		 		 		 		 V5B_SifI2cWriteByte(0x3e,0xb8);
		 		 		 		 V5B_SifI2cWriteByte(0x3f,0xd1);
		 		 		 		 V5B_SifI2cWriteByte(0x81,0x08);
		 		 		 		 V5B_SifI2cWriteByte(0x82,0x0d);
		 		 		 		 V5B_SifI2cWriteByte(0x83,0x16);
		 		 		 		 V5B_SifI2cWriteByte(0x84,0x1e);
		 		 		 		 V5B_SifI2cWriteByte(0x85,0x24);
		 		 		 		 V5B_SifI2cWriteByte(0x86,0x30);
		 		 		 		 V5B_SifI2cWriteByte(0x87,0x3b);
		 		 		 		 V5B_SifI2cWriteByte(0x88,0x45);
		 		 		 		 V5B_SifI2cWriteByte(0x89,0x4f);
		 		 		 		 V5B_SifI2cWriteByte(0x8a,0x61);
		 		 		 		 V5B_SifI2cWriteByte(0x8b,0x71);
		 		 		 		 V5B_SifI2cWriteByte(0x8c,0x80);
		 		 		 		 V5B_SifI2cWriteByte(0x8d,0x9d);
		 		 		 		 V5B_SifI2cWriteByte(0x8e,0xb8);
		 		 		 		 V5B_SifI2cWriteByte(0x8f,0xd1);
		 		 		 		 V5B_SifI2cWriteByte(0x91,0x08);
		 		 		 		 V5B_SifI2cWriteByte(0x92,0x0d);
		 		 		 		 V5B_SifI2cWriteByte(0x93,0x16);
		 		 		 		 V5B_SifI2cWriteByte(0x94,0x1e);
		 		 		 		 V5B_SifI2cWriteByte(0x95,0x24);
		 		 		 		 V5B_SifI2cWriteByte(0x96,0x30);
		 		 		 		 V5B_SifI2cWriteByte(0x97,0x3b);
		 		 		 		 V5B_SifI2cWriteByte(0x98,0x45);
		 		 		 		 V5B_SifI2cWriteByte(0x99,0x4f);
		 		 		 		 V5B_SifI2cWriteByte(0x9a,0x61);
		 		 		 		 V5B_SifI2cWriteByte(0x9b,0x71);
		 		 		 		 V5B_SifI2cWriteByte(0x9c,0x80);
		 		 		 		 V5B_SifI2cWriteByte(0x9d,0x9d);
		 		 		 		 V5B_SifI2cWriteByte(0x9e,0xb8);
		 		 		 		 V5B_SifI2cWriteByte(0x9f,0xd1);
		 		 		 		 V5B_SifI2cWriteByte(0x0d,0x1b);
		 		 		 		 V5B_SifI2cWriteByte(0xe0,0x00);
			break;

		case 3:	
		       
			                     V5B_SifI2cWriteByte(0x9 ,0x1f);
		 		 		 		 V5B_SifI2cWriteByte(0xe7,0x0c);
		 		 		 		 V5B_SifI2cWriteByte(0xe8,0x0d);
		 		 		 		 V5B_SifI2cWriteByte(0x13,0x80);
		 		 		 		 V5B_SifI2cWriteByte(0x14,0xa0);
		 		 		 		 V5B_SifI2cWriteByte(0x24,0x00);
		 		 		 		 V5B_SifI2cWriteByte(0x26,0x8d);
		 		 		 		 V5B_SifI2cWriteByte(0x43,0x41);
		 		 		 		 V5B_SifI2cWriteByte(0x46,0x00);
		 		 		 		 V5B_SifI2cWriteByte(0x60,0xa0);
		 		 		 		 V5B_SifI2cWriteByte(0x61,0x65);
		 		 		 		 V5B_SifI2cWriteByte(0x62,0x35);
		 		 		 		 V5B_SifI2cWriteByte(0x63,0x10);
		 		 		 		 V5B_SifI2cWriteByte(0x69,0x00);
		 		 		 		 V5B_SifI2cWriteByte(0x6a,0xb6);
		 		 		 		 V5B_SifI2cWriteByte(0xea,0xa0);
		 		 		 		 V5B_SifI2cWriteByte(0xeb,0x80);
		 		 		 		 V5B_SifI2cWriteByte(0xec,0x50);
		 		 		 		 V5B_SifI2cWriteByte(0xed,0x10);
		 		 		 		 V5B_SifI2cWriteByte(0x77,0x00);
		 		 		 		 V5B_SifI2cWriteByte(0x78,0xa0);
		 		 		 		 V5B_SifI2cWriteByte(0x79,0xa0);
		 		 		 		 V5B_SifI2cWriteByte(0x7c,0x20);
		 		 		 		 V5B_SifI2cWriteByte(0x7d,0x25);
		 		 		 		 V5B_SifI2cWriteByte(0x31,0x11);
		 		 		 		 V5B_SifI2cWriteByte(0x32,0x19);
		 		 		 		 V5B_SifI2cWriteByte(0x33,0x26);
		 		 		 		 V5B_SifI2cWriteByte(0x34,0x2f);
		 		 		 		 V5B_SifI2cWriteByte(0x35,0x37);
		 		 		 		 V5B_SifI2cWriteByte(0x36,0x45);
		 		 		 		 V5B_SifI2cWriteByte(0x37,0x51);
		 		 		 		 V5B_SifI2cWriteByte(0x38,0x5c);
		 		 		 		 V5B_SifI2cWriteByte(0x39,0x65);
		 		 		 		 V5B_SifI2cWriteByte(0x3a,0x77);
		 		 		 		 V5B_SifI2cWriteByte(0x3b,0x87);
		 		 		 		 V5B_SifI2cWriteByte(0x3c,0x95);
		 		 		 		 V5B_SifI2cWriteByte(0x3d,0xae);
		 		 		 		 V5B_SifI2cWriteByte(0x3e,0xc5);
		 		 		 		 V5B_SifI2cWriteByte(0x3f,0xda);
		 		 		 		 V5B_SifI2cWriteByte(0x81,0x11);
		 		 		 		 V5B_SifI2cWriteByte(0x82,0x19);
		 		 		 		 V5B_SifI2cWriteByte(0x83,0x26);
		 		 		 		 V5B_SifI2cWriteByte(0x84,0x2f);
		 		 		 		 V5B_SifI2cWriteByte(0x85,0x37);
		 		 		 		 V5B_SifI2cWriteByte(0x86,0x45);
		 		 		 		 V5B_SifI2cWriteByte(0x87,0x51);
		 		 		 		 V5B_SifI2cWriteByte(0x88,0x5c);
		 		 		 		 V5B_SifI2cWriteByte(0x89,0x65);
		 		 		 		 V5B_SifI2cWriteByte(0x8a,0x77);
		 		 		 		 V5B_SifI2cWriteByte(0x8b,0x87);
		 		 		 		 V5B_SifI2cWriteByte(0x8c,0x95);
		 		 		 		 V5B_SifI2cWriteByte(0x8d,0xae);
		 		 		 		 V5B_SifI2cWriteByte(0x8e,0xc5);
		 		 		 		 V5B_SifI2cWriteByte(0x8f,0xda);
		 		 		 		 V5B_SifI2cWriteByte(0x91,0x11);
		 		 		 		 V5B_SifI2cWriteByte(0x92,0x19);
		 		 		 		 V5B_SifI2cWriteByte(0x93,0x26);
		 		 		 		 V5B_SifI2cWriteByte(0x94,0x2f);
		 		 		 		 V5B_SifI2cWriteByte(0x95,0x37);
		 		 		 		 V5B_SifI2cWriteByte(0x96,0x45);
		 		 		 		 V5B_SifI2cWriteByte(0x97,0x51);
		 		 		 		 V5B_SifI2cWriteByte(0x98,0x5c);
		 		 		 		 V5B_SifI2cWriteByte(0x99,0x65);
		 		 		 		 V5B_SifI2cWriteByte(0x9a,0x77);
		 		 		 		 V5B_SifI2cWriteByte(0x9b,0x87);
		 		 		 		 V5B_SifI2cWriteByte(0x9c,0x95);
		 		 		 		 V5B_SifI2cWriteByte(0x9d,0xae);
		 		 		 		 V5B_SifI2cWriteByte(0x9e,0xc5);
		 		 		 		 V5B_SifI2cWriteByte(0x9f,0xda);
		 		 		 		 V5B_SifI2cWriteByte(0xd ,0x11);
		 		 		 		 V5B_SifI2cWriteByte(0xe0,0x00);
                       
			break;

		default:
			break;
	}
}

const TReg gSifPwrOnSeqRegVal_SP80818[] =   //528 sif 0x800  power on
{

	{0x00, 0x64,	1},   //power on
	{0x00, 0x65,	1},   // reset
	{0x00, 0x67,	1}    // sensor enable
  	
};

const TReg gSifRegVal_SP80818[] =  // SIF config (0x800)
{        
        {0x02, 0x02, 0},      // config
        {0x03, 0x02, 0},      // Vsync  {0x03, 0x1b, 0},      
        {0x04, 0x04, 0},      // Hsync
        {0x21, 0x00, 0},      // vref_start  // fengyu
        {0x22, 0x01, 0},      // add by fengyu (verf_stop)
        {0x23, 0xe0, 0},      // vref_stop
        {0x2c, 0x02, 0},      // Colmax registers
        {0x2d, 0x80, 0},      //
        {0x2e, 0x01, 0},      // Rowmax register
        {0x2f, 0xe0, 0},      //
        {0x01, 0x41, 0}       // Sensor operator mode control register {0x01, 0x45, 0}
};

/* Sensor size control configure info */

const TReg gSnrSizeRegValVGA_SP80818[] = 
{      	

         {0x0d,0x19,1},
		 {0xe0,0x81,1},
		 {0x5d,0x02,1},
		 {0x09,0x1f,1},// 1F
		 {0x0c,0x60,1},// Flip and Mirror
		 {0x0e,0x10,1},
		 {0x0f,0x96,1},
		 {0x10,0xc0,1},
		 {0x11,0xf0,1},
		 {0x12,0x10,1},//40
		 {0x13,0xc0,1},
		 {0x14,0x80,1},
		 {0x15,0xd9,1},
		 {0x16,0xba,1},
		 {0x17,0xdc,1},
		 {0x19,0xd0,1},
		 {0x1a,0x20,1},
		 {0x1f,0xd0,1},
		 {0x20,0x00,1},
		 {0x21,0x08,1},
		 {0x22,0x08,1},
		 {0x23,0xff,1},//c0
		 {0x24,0x00,1},
		 {0x27,0x54,1},
		 {0x28,0xa4,1},
		 {0x29,0x62,1},
		 {0x2a,0x9e,1},

		 {0x40,0x00,1},//40
		 {0x41,0xdf,1},//a4
		 {0x43,0x44,1},// 46,00,42,c8,c9,40
		 {0x47,0x71,1},//70
		 {0x48,0x12,1},//00
		 {0x5e,0x16,1},
		 {0x5f,0x10,1},
		 {0x64,0x04,1}, // 1a
		 {0x65,0x50,1},
		 {0x66,0xb5,1},
		 {0x67,0x83,1},
		 {0x80,0xc3,1},//C3
		 {0xcd,0x1a,1},//00

         //Color Matrix
		 /*
		 {0x49,0x27,1},
		 {0x4a,0x00,1},
		 {0x4b,0x40,1},
		 {0x4c,0x00,1},
		 {0x4d,0x13,1},
		 {0x4e,0x00,1},
		 {0x4f,0xd7,1},
		 {0x50,0x03,1},
		 {0x51,0x51,1},
		 {0x52,0x03,1},
		 {0x53,0xd6,1},
		 {0x54,0x00,1},
		 {0x55,0xc2,1},
		 {0x56,0x00,1},
		 {0x57,0x54,1},
		 {0x58,0x03,1},
		 {0x59,0xe6,1},
		 {0x5a,0x03,1},
         */

         {0x49,0x26,1},
		 {0x4a,0x00,1},
		 {0x4b,0x4b,1},
		 {0x4c,0x00,1},
		 {0x4d,0x11,1},
		 {0x4e,0x00,1},
		 {0x4f,0xe6,1},
		 {0x50,0x03,1},
		 {0x51,0x84,1},
		 {0x52,0x03,1},
		 {0x53,0x96,1},
		 {0x54,0x00,1},
		 {0x55,0x82,1},
		 {0x56,0x00,1},
		 {0x57,0x79,1},
		 {0x58,0x03,1},
		 {0x59,0x03,1},
		 {0x5a,0x00,1},


		 {0x31,0x11,1},
		 {0x32,0x19,1},
		 {0x33,0x26,1},
		 {0x34,0x2f,1},
		 {0x35,0x37,1},
		 {0x36,0x45,1},
		 {0x37,0x51,1},
		 {0x38,0x5c,1},
		 {0x39,0x65,1},
		 {0x3a,0x77,1},
		 {0x3b,0x87,1},
		 {0x3c,0x95,1},
		 {0x3d,0xae,1},
		 {0x3e,0xc5,1},
		 {0x3f,0xda,1},

		 {0x81,0x11,1},
		 {0x82,0x19,1},
		 {0x83,0x26,1},
		 {0x84,0x2f,1},
		 {0x85,0x37,1},
		 {0x86,0x45,1},
		 {0x87,0x51,1},
		 {0x88,0x5c,1},
		 {0x89,0x65,1},
		 {0x8a,0x77,1},
		 {0x8b,0x87,1},
		 {0x8c,0x95,1},
		 {0x8d,0xae,1},
		 {0x8e,0xc5,1},
		 {0x8f,0xda,1},

		 {0x91,0x11,1},
		 {0x92,0x19,1},
		 {0x93,0x26,1},
		 {0x94,0x2f,1},
		 {0x95,0x37,1},
		 {0x96,0x45,1},
		 {0x97,0x51,1},
		 {0x98,0x5c,1},
		 {0x99,0x65,1},
		 {0x9a,0x77,1},
		 {0x9b,0x87,1},
		 {0x9c,0x95,1},
		 {0x9d,0xae,1},
		 {0x9e,0xc5,1},
		 {0x9f,0xda,1},

		 {0xe8,0x8b,1},
		 {0x06,0x00,1},//00
		 {0x26,0x87,1}, //c6
		 {0x46,0x03,1},
		 {0x60,0xb0,1},
		 {0x61,0x80,1},//70
		 {0x62,0x3a,1},//35
		 {0x63,0x10,1},
		 {0x69,0x07,1},
		 {0x6a,0x14,1},
		 {0x77,0x00,1},
		 {0x78,0x80,1},//80,bo
		 {0x79,0x80,1},//80,bo
		 {0x7c,0x05,1},//01//02
		 {0x7d,0x20,1},//0A

		 {0x7a,0x87,1}

};

/* Sensor size control configure info */

const TSnrSizeCfg gSensorSizeCfg_SP80818[] = 
{
	{
               {0,NULL},
               {0,NULL},     // sensor interface
	       {640, 480},  
		//0,         //0x805  MCLK (0:24M)
		//0          //0x806  PCLK (0:24M)
		1,
		1

	},

};

 

const TSnrInfo gSensorInfo_SP80818 = 

{
       "SP80818 yuv mode",
	VIM_SNR_YUV,							//yuv mode
	0,	//0x85c bit1  //High priority for 0x806				         // use PCLK of sensor
	0,	//0x85c Bit4-bit2						//[0]: VSync delay enable; [1]: Fifo VSync delay enable; [2]: Use PCLK Negtive Edge.
	0,							//0 -- I2C, 1 -- Serial bus;
	0,							//[0]: 1 -- OV I2C bus, 0 -- Normal sensor
	100,							//I2C Rate : 100KHz
	0x30,							//I2C address
	0x30, 							//ISP I2C address for special sensor
	0x03,							//power config
	0,							//reset sensor  OV Sensor SXGA->VGA hardware reset.

	3,							//brightness 
	3,							//contrast
	
	{0,NULL},														//sensor standby
	{sizeof(gSnrSizeRegValVGA_SP80818) / sizeof(TReg), (PTReg)&gSnrSizeRegValVGA_SP80818},    // Sensor Size List above                                                  //snr initial value	
	
	{sizeof(gSifPwrOnSeqRegVal_SP80818) / sizeof(TReg), (PTReg)&gSifPwrOnSeqRegVal_SP80818},		//Sif config sequence(Reg.800 bit0~2) when sensor power on
	{0, NULL},																								//Sif config sequence(Reg.800 bit0~2) when sensor standby
	{sizeof(gSifRegVal_SP80818) / sizeof(TReg), (PTReg)&gSifRegVal_SP80818},						//sif initial value
																							//isp initial value
	{sizeof(gSensorSizeCfg_SP80818) / sizeof(TSnrSizeCfg), (PTSnrSizeCfg)&gSensorSizeCfg_SP80818},		//size configure 

	SensorSetReg_SP80818,																				//set reg callback
	SensorGetReg_SP80818,																				//get reg callback
	CheckId_SP80818,																				//Sensor ID Check Call
	SensorSetMirrorFlip_SP80818,																									//Set Sensor Mirror Flip Call
	SensorSetContrast_SP80818,																									//Sensor switch size callback
	SensorSetBrightness_SP80818,																									//set et callback
	NULL,	
	SensorSetMode_SP80818,
	NULL,																									//Sensor switch size callback
	NULL,
	{	//ythd of flash,et of flash
		0x20,			10
	},
};

#endif


///////////////////////////////////////////////////////////////
//
//      fengyu (MT9V112)
//
///////////////////////////////////////////////////////////////

#if V5_DEF_SNR_MT9V112_YUV

void SensorSetReg_MT9V112(UINT16 uAddr, UINT16 uVal)
{
         V5B_SifI2cWriteWord((UINT8)uAddr, (UINT16)uVal);
}

void SensorGetReg_MT9V112(UINT8 uAddr, UINT16 *uVal)
{
	V5B_SifI2cReadWord((UINT8)uAddr, (UINT16*)uVal);
}

UINT8 CheckId_MT9V112(void)
{
	 UINT16 temp;
	 V5B_SifI2cWriteWord(0xf0, 0);
	 V5B_SifI2cReadWord(0xff, &temp);
     

	 if(temp==0x1229)  
	  	return SUCCEED;
	 else
	  	return FAILED; 
}

void SensorSetMirrorFlip_MT9V112(UINT8 val)
{
     UINT16 x;
	 val&=0x03;
	 if(val==0x01)
	  	val=0x2;
	 else if(val==0x02)
	  	val=0x01;
	 else if(val==0x00)
	  	val=0x03;
	 else if(val==0x03)
	  	val=0x00;
	 
	 V5B_SifI2cWriteWord(0xf0, 0);
	 V5B_SifI2cReadWord(0x20, &x);
	 x&=0xfffc;
	 x|=val;
	 V5B_SifI2cWriteWord(0x20, x); 
}

void SensorSetContrast_MT9V112(UINT8 val)
{
	
	V5B_SifI2cWriteWord(0xf0, 0x01); 
	switch(val)
	{	
	case 1:                 
		V5B_SifI2cWriteWord(0xdc, 0x1307);  //gamma0.6
		V5B_SifI2cWriteWord(0xdd, 0x4528);  
		V5B_SifI2cWriteWord(0xde, 0x9a73);  
		V5B_SifI2cWriteWord(0xdf, 0xd3b9);  
		V5B_SifI2cWriteWord(0xe0, 0xffeb);  
		V5B_SifI2cWriteWord(0xe1, 0x0000);  
		                          

		break;                    
        case 2:                                                    
		V5B_SifI2cWriteWord(0xdc, 0x0b04);  //gamma0.7
		V5B_SifI2cWriteWord(0xdd, 0x331b);  
		V5B_SifI2cWriteWord(0xde, 0x8d61);  
		V5B_SifI2cWriteWord(0xdf, 0xd0b2);  
		V5B_SifI2cWriteWord(0xe0, 0xffea);  
		V5B_SifI2cWriteWord(0xe1, 0x0000);  

		break;
	case 3:                                
		V5B_SifI2cWriteWord(0xdc, 0x0703);  //gamma0.8
		V5B_SifI2cWriteWord(0xdd, 0x2712);  
		V5B_SifI2cWriteWord(0xde, 0x7e50);  
		V5B_SifI2cWriteWord(0xdf, 0xccaa);  
		V5B_SifI2cWriteWord(0xe0, 0xffe8);  
		V5B_SifI2cWriteWord(0xe1, 0x0000);  

		break; 
        case 4:                                
		V5B_SifI2cWriteWord(0xdc, 0x0502);  //gamma0.85
		V5B_SifI2cWriteWord(0xdd, 0x210f);  
		V5B_SifI2cWriteWord(0xde, 0x7648);  
		V5B_SifI2cWriteWord(0xdf, 0xcca7);  
		V5B_SifI2cWriteWord(0xe0, 0xffe8);  
		V5B_SifI2cWriteWord(0xe1, 0x0000);  

		break;  
	case 5:                                
		V5B_SifI2cWriteWord(0xdc, 0x0402);  //gamma0.9
		V5B_SifI2cWriteWord(0xdd, 0x1b0c);  
		V5B_SifI2cWriteWord(0xde, 0x6e3f);  
		V5B_SifI2cWriteWord(0xdf, 0xcba3);  
		V5B_SifI2cWriteWord(0xe0, 0xffe8);  
		V5B_SifI2cWriteWord(0xe1, 0x0000);  

		break; 
	default:
		break;
	}
	
}

/******************************************************************
         Desc: set sensor Brightness  callback 
         Para: 1-5 step
*******************************************************************/
void SensorSetBrightness_MT9V112(UINT8 val)
{
	
	V5B_SifI2cWriteWord(0xf0, 0x02);
	switch(val)
	{
	case 1:
		V5B_SifI2cWriteWord(0x2e, 0x092c);
		break;	
	case 2:
		V5B_SifI2cWriteWord(0x2e, 0x094c);
		break;
	case 3:
		V5B_SifI2cWriteWord(0x2e, 0x096c);
		break;
	case 4:
		V5B_SifI2cWriteWord(0x2e, 0x098c);
		break;
	case 5:
		V5B_SifI2cWriteWord(0x2e, 0x09ac);
		break;
	default:
		break;		
	}
	
}

/******************************************************************
         Desc: set sensor mode  callback 
         Para:  val: 	1 50hz,	2 60hz,	3 night, 4 outdoor.
*******************************************************************/
void SensorSetMode_MT9V112(UINT8 val)
{
	
	V5B_SifI2cWriteWord(0xf0, 0x0002);
	switch (val)
	{	
		case 1:
		
			V5B_SifI2cWriteWord(0x5b, 0x0001);	//50Hz 
			V5B_SifI2cWriteWord(0x37, 0x00a0);	//20-27fps
			V5B_SifI2cWriteWord(0x67, 0x1010);
			break;
		case 2:
	        
			V5B_SifI2cWriteWord(0x5b, 0x0003);	//60Hz 
			V5B_SifI2cWriteWord(0x37, 0x00c0);	//20-27fps
			V5B_SifI2cWriteWord(0x67, 0x1010);
			break;
		case 4:  // 3 and 4 reverse
           
            V5B_SifI2cWriteWord(0xf0, 0x0002);
			V5B_SifI2cWriteWord(0x37, 0x0300);	//night lowest 12.5fps
			V5B_SifI2cWriteWord(0x67, 0x4010);

            
			break;
		case 3:	
	        
			V5B_SifI2cWriteWord(0x37, 0x0080);	//outdoor 25fps	uper
			V5B_SifI2cWriteWord(0x67, 0x1010);
			break;
		default:
			break;
	}
	
}
const TReg gSifPwrOnSeqRegVal_MT9V112[] =
{
	{0x00, 0x63,	1},
	{0x00, 0x62,	1},
	{0x00, 0x63,	1}
};

const TReg gSifRegVal_MT9V112[] =
{        
     {0x01,  0x41, 0},
	 {0x02,  0x00, 0},
	 {0x03,  0x09, 0},
	 {0x04,  0x05, 0},
	 
	 {0x2c,  0x02, 0},
	 {0x2d,  0x80, 0},
	 {0x2e,  0x01, 0},
	 {0x2f,  0xe0, 0}, 
};

const TReg gSnrSizeRegValVGA_MT9V112[] = 
{        
/*
    {0xf0, 0x0000, 1},
	{0x0d, 0x0001, 1},
	{0x0d, 0x0000, 1},

	{0xf0, 0x0001, 1},
	{0x06, 0x708e, 1},
	{0x9b, 0x0280, 1},
	{0xf0, 0x0002, 1},
	{0xc8, 0x1f0b, 1},
	
	
	{0xf0, 0x0002, 1},		//color correction
	{0x02, 0x00ae, 1},		
	{0x03, 0x2923, 1},		
	{0x04, 0x04a4, 1},		
	{0x09, 0x00cf, 1},		
	{0x0a, 0x00d0, 1},	
	{0x0b, 0x0030, 1},	
	{0x0c, 0x00fd, 1},	
	{0x0d, 0x0083, 1},	
	{0x0e, 0x0011, 1},	
	{0x0f, 0x0073, 1},	
	{0x10, 0x005d, 1},
	{0x11, 0x00b9, 1},	
//	{0x5b, 0x0003, 1},		//flicker control
	{0xf0, 0x0001, 1},
	{0x34, 0x0000, 1},
	{0x35, 0xff00, 1},
	{0xDC, 0x210c, 1},		//gamma context B
	{0xDD, 0x5e3b, 1},
	{0xDE, 0xb492, 1},
	{0xDF, 0xe1cd, 1},
	{0xE0, 0xfff1, 1},
	{0xE1, 0x0000, 1},
	
	{0x05, 0x0004, 1},
	{0xf0, 0x0002, 1},
	{0x2e, 0x0c55, 1},		//ytarget 
	//{0x36, 0x1810, 1},
	{0x37, 0x0080, 1},      // AE Gain Zone Limit
	{0x67, 0x1010, 1},      // AE Digital Gain Adjustment Limits

	// Gamma
	{0xF0, 0x0001, 1},
	{0x53, 0x0905, 1},
	{0x54, 0x2412, 1},
	{0x55, 0x6D49, 1},
	{0x56, 0xB692, 1},
	{0x57, 0xFFDB, 1},
	{0x58, 0x0000, 1},
	{0xDC, 0x0905, 1},
	{0xDD, 0x2412, 1},
	{0xDE, 0x6D49, 1},
	{0xDF, 0xB692, 1},
	{0xE0, 0xFFDB, 1},
	{0xE1, 0x0000, 1},

    // Saturation
	{0x25, 0x0015, 1},      

	// Row Noise
	{0xF0, 0x0000, 1},
	{0x30, 0x0460, 1},
*/
	// 26Mhz Flicker Setting

	/*
    {0xF0, 0x0000, 1},

	{0x05, 0x00BD, 1},     
	{0x06, 0x000D, 1},     
	{0x07, 0x00BD, 1},     
	{0x08, 0x000D, 1},     
	{0x20, 0x0700, 1},    // Mirror and Flip 
	{0x21, 0x0400, 1},     

	{0xF0, 0x0002, 1},

	{0x39, 0x0345, 1},     
	{0x3A, 0x0345, 1},     
	{0x3B, 0x055B, 1},     
	{0x3C, 0x055B, 1},     
	//{0x57, 0x0205, 1},     // 60Hz
	{0x58, 0x026D, 1},     // 50Hz
	//{0x59, 0x0205, 1},     // 60Hz
	{0x5A, 0x026D, 1},     // 50Hz
	//{0x5C, 0x120D, 1},     // 60Hz 
	{0x5D, 0x1611, 1},     // 50Hz 
	{0x64, 0x5E1C, 1},    // Flicker parameter
	*/

	//3xx Setting

	{0xf0, 0x0000, 1},
	{0xc8, 0x1f0b, 1},
	
	
	{0xf0, 0x0001, 1},
	{0x05, 0x0008, 1},
	{0x34, 0x0008, 1},
	{0x2e, 0x0c48, 1},
	
	
	{0xf0, 0x0000, 1},
	{0x05, 0x0096, 1},
	{0x06, 0x000d, 1},
	{0x07, 0x0096, 1},
	{0x08, 0x000d, 1},
	{0x20, 0x0703, 1},		//0x0703   //modi by brave 071018
	{0x21, 0x0400, 1},
	
	
	{0xf0, 0x0002, 1},
	{0x39, 0x031e, 1},
	{0x3a, 0x031e, 1},
	{0x3b, 0x055b, 1},
	{0x3c, 0x055b, 1},
	{0x57, 0x01f5, 1},
	{0x58, 0x0259, 1},
	{0x59, 0x01f5, 1},
	{0x5a, 0x0259, 1},
	{0x5c, 0x120d, 1},
	{0x5d, 0x1611, 1},
//	{0x5b, 0x0003, 1},
	{0x64, 0x5e1c, 1},
	{0x5b, 0x0001, 1},
	
	
	{0xf0, 0x0001, 1},
	{0x53, 0x0905, 1},
	{0x54, 0x2412, 1},
	{0x55, 0x6d49, 1},
	{0x56, 0xb692, 1},
	{0x57, 0xffdb, 1},
	{0x58, 0x0000, 1},

	{0xdc, 0x0905, 1},
	{0xdd, 0x2412, 1},
	{0xde, 0x6d49, 1},
//	{0x36, 0x1810, 1},
	{0xdf, 0xb692, 1},
	{0xe0, 0xffdb, 1},
	{0xe1, 0x0000, 1},
	
	
	{0xf0, 0x0002, 1},
	{0x22, 0xa078, 1},
	{0x23, 0xa078, 1},
	{0x24, 0x7f00, 1},
	{0x28, 0xea02, 1},
	{0x29, 0x867a, 1},
	{0x5e, 0x524c, 1},
	{0x5f, 0x2024, 1},
	{0x60, 0x0002, 1},
	{0x02, 0x00ee, 1},
	{0x03, 0x3923, 1},
	{0x04, 0x0724, 1},
	{0x09, 0x00c0, 1},
	{0x0a, 0x0079, 1},
	{0x0b, 0x0004, 1},
	{0x0c, 0x005c, 1},
	{0x0d, 0x00d9, 1},
	{0x0e, 0x0053, 1},
	{0x0f, 0x0021, 1},
	{0x10, 0x00a4, 1},
	{0x11, 0x00e5, 1},
	{0x15, 0x0000, 1},
	{0x16, 0x0000, 1},
	{0x17, 0x0000, 1},
	{0x18, 0x0000, 1},
	{0x19, 0x0000, 1},
	{0x1a, 0x0000, 1},
	{0x1b, 0x0000, 1},
	{0x1c, 0x0000, 1},
	{0x1d, 0x0000, 1},
	{0x1e, 0x0000, 1},

	{0xf0, 0x0001, 1},
	{0x06, 0x701e, 1},
	{0x00, 0x0100, 1},
	{0x06, 0x700e, 1},
	
	
	{0xf0, 0x0002, 1},
	{0x5e, 0x5e41, 1},
	{0x5f, 0x3723, 1},
	
	
	{0xf0, 0x0000, 1},
	{0x34, 0xc019, 1},
	{0x40, 0x1800, 1},
	{0x76, 0x7358, 1},
	{0x04, 0x0282, 1},
	{0x03, 0x01e2, 1},
	
	
	{0xf0, 0x0001, 1},
	{0xa0, 0x0282, 1},
	{0xa3, 0x01e2, 1},
	{0xa6, 0x0282, 1},
	{0xa9, 0x01e2, 1},
	
	
	{0xf0, 0x0000, 1},
	{0x5f, 0x3630, 1},
	{0x30, 0x043e, 1},
	
	
	{0xf0, 0x0001, 1},
	{0x3b, 0x044e, 1},
	
	
	{0xf0, 0x0002, 1},
	{0x36, 0x2010, 1},
	{0x37, 0x8160, 1},

};

const TSnrSizeCfg gSensorSizeCfg_MT9V112[] = 
{
	{
     		{0,NULL},
            {0,NULL},
		{640, 480},
		1,
		1


	}

};

const TSnrInfo gSensorInfo_MT9V112 = 
{
       "MT9V112 yuv mode",
	VIM_SNR_YUV,							//yuv mode
	0,							//not use PCLK of sensor
	0,							//[0]: VSync delay enable; [1]: Fifo VSync delay enable; [2]: Use PCLK Negtive Edge.
	0,							//0 -- I2C, 1 -- Serial bus;
	0,							//[0]: 1 -- OV I2C bus
	80,							//I2C Rate : 100KHz
	0xba,							//I2C address
	0xba,							//ISP I2C address for special sensor
	0x03,							//power config
	0,							//reset sensor
                                                		
	3,							//brightness 
	3,							//contrast

	{0,NULL},											//sensor standby                                                                                  																									
	//{0,NULL},											//sensor initial setting                                                                                													
       {sizeof(gSnrSizeRegValVGA_MT9V112) / sizeof(TReg), (PTReg)&gSnrSizeRegValVGA_MT9V112},                                                       //snr initial value
	{sizeof(gSifPwrOnSeqRegVal_MT9V112) / sizeof(TReg), (PTReg)&gSifPwrOnSeqRegVal_MT9V112},	//Sif config sequence(Reg.800 bit0~2) when sensor power on                                              	
	{0, NULL},											//Sif config sequence(Reg.800 bit0~2) when sensor standby                                               					
	{sizeof(gSifRegVal_MT9V112) / sizeof(TReg), (PTReg)&gSifRegVal_MT9V112},			//sif initial value                                                                                     	
	{sizeof(gSensorSizeCfg_MT9V112) / sizeof(TSnrSizeCfg), (PTSnrSizeCfg)&gSensorSizeCfg_MT9V112},	//size configure                                                                                        	

	SensorSetReg_MT9V112,																				
	SensorGetReg_MT9V112,																				
	CheckId_MT9V112,																				
	SensorSetMirrorFlip_MT9V112,																						
	SensorSetContrast_MT9V112,
	SensorSetBrightness_MT9V112,	
	NULL,	
	SensorSetMode_MT9V112,	
	NULL,																									
	NULL,																									
	{	//ythd of flash,et of flash
		0x20,			10
	},
};

#endif

/////////////////////////////////////////////////////////////////
//
//      S5KA3AFX (fengyu)
//
/////////////////////////////////////////////////////////////////

#if V5_DEF_SNR_S5KA3AFX_YUV	

void SensorSetReg_S5KA3AFX(UINT16 uAddr, UINT16 uVal)
{
         V5B_SifI2cWriteByte((UINT8)uAddr, (UINT8)uVal);
}

void SensorGetReg_S5KA3AFX(UINT8 uAddr, UINT16 *uVal)
{
	V5B_SifI2cReadByte((UINT8)uAddr, (UINT8*)uVal);
}

UINT8 CheckId_S5KA3AFX(void)
{
	UINT8 temp1;
	UINT8 temp2;


	V5B_SifI2cWriteByte(0xef, 00);  //Page select
	V5B_SifI2cReadByte(0xb2, &temp1);
	V5B_SifI2cReadByte(0xb3, &temp2);
	if((temp1==0xc5) && (temp2==0x02)) 
		return SUCCEED;
	else
		return FAILED; 
}

void SensorSetMirrorFlip_S5KA3AFX(UINT8 val)
{
	V5B_SifI2cWriteByte(0xef,02); // Page Select
	switch(val)
	{
		case 0:
			V5B_SifI2cWriteByte(0x02, 0x40);
			break;
		case 1:
			V5B_SifI2cWriteByte(0x02, 0x80);
			break;
		case 2:
			V5B_SifI2cWriteByte(0x02, 0xc0);
			break;
		default:
			V5B_SifI2cWriteByte(0x02, 0x00);
			break;
	}
}

/******************************************************************
         Desc: set sensor Brightness  callback 
         Para: 1-5 step
*******************************************************************/
void SensorSetBrightness_S5KA3AFX(UINT8 val)
{	
	V5B_SifI2cWriteByte(0xef, 0x01);
	switch(val)
	{
	case 1:
		
		V5B_SifI2cWriteByte(0xae, 0xa6);
		V5B_SifI2cWriteByte(0xaf, 0xa6);
		V5B_SifI2cWriteByte(0xef, 0x00);
		V5B_SifI2cWriteByte(0x73, 0xa6);
		break;
		
	case 2:
		
		V5B_SifI2cWriteByte(0xae, 0x98);
		V5B_SifI2cWriteByte(0xaf, 0x98);
		V5B_SifI2cWriteByte(0xef, 0x00);
		V5B_SifI2cWriteByte(0x73, 0x98);
		break;

	case 3:

		V5B_SifI2cWriteByte(0xae, 0x88);
		V5B_SifI2cWriteByte(0xaf, 0x88);
		V5B_SifI2cWriteByte(0xef, 0x00);
		V5B_SifI2cWriteByte(0x73, 0x88);
		break;

	case 4:

		V5B_SifI2cWriteByte(0xae, 0x08);
		V5B_SifI2cWriteByte(0xaf, 0x08);
		V5B_SifI2cWriteByte(0xef, 0x00);
		V5B_SifI2cWriteByte(0x73, 0x08);
		break;

	case 5:

		V5B_SifI2cWriteByte(0xae, 0x18);
		V5B_SifI2cWriteByte(0xaf, 0x18);
		V5B_SifI2cWriteByte(0xef, 0x00);
		V5B_SifI2cWriteByte(0x73, 0x18);
		break;

	default:
		break;		
	}
}

/******************************************************************
         Desc: set sensor mode  callback 
         Para:  val: 	1 50hz,	2 60hz,	3 night, 4 outdoor.
*******************************************************************/
void SensorSetMode_S5KA3AFX(UINT8 val)
{

    UINT8 temp;
	V5B_SifI2cWriteByte(0xef, 01);  //Page select
	V5B_SifI2cReadByte(0x70, &temp);


	switch (val)
	{	
		case 1:

			temp|=0x0c;
			V5B_SifI2cWriteByte(0x70,temp);	//30fps
			break;

		case 2:

			temp&=0xfb;	//30fps
			V5B_SifI2cWriteByte(0x70, temp);
			break;

		case 4:  // 3 and 4 reverse

			V5B_SifI2cWriteByte(0xb5, 0xc0);
            V5B_SifI2cWriteByte(0xaf, 0x15);
			V5B_SifI2cWriteByte(0xe2, 0x16);
			V5B_SifI2cWriteByte(0xe3, 0x13);
			break;

		case 3:	

            temp&=0xf3;	//30fps
			V5B_SifI2cWriteByte(0x70, temp);		
			break;

		default:
			break;
	}
}

const TReg gSifPwrOnSeqRegVal_S5KA3AFX[] =
{
	{0x00, 0x67,	1},
	{0x00, 0x66,	1},
	{0x00, 0x67,	1}
};

const TReg gSifRegVal_S5KA3AFX[] =
{        
    {0x01, 0x41,	0},
	{0x02, 0x02,	0},			
	{0x03, 0x0a,	0},
	{0x04, 0x04,	0},
};

const TReg gSnrSizeRegValVGA_S5KA3AFX[] = 
{      
	{0xef, 0x01, 1},
	{0x70, 0x00, 1},  
	{0xef, 0x00, 1},
	{0x0d, 0x03, 1},  
	{0x0e, 0x00, 1},
	{0x0f, 0x1F, 1},
	{0x10, 0x00, 1},
	{0x11, 0x37, 1},
	{0x12, 0x08, 1},
	{0x13, 0x42, 1},
	{0x14, 0x00, 1},
	{0x15, 0x12, 1},
	{0x16, 0x9E, 1},
	{0x19, 0x00, 1},
	{0x17, 0x00, 1},
	{0x18, 0x2A, 1},
	{0x19, 0x01, 1},
	{0x17, 0x00, 1},
	{0x18, 0x2B, 1},
	{0x19, 0x02, 1},
	{0x17, 0x00, 1},
	{0x18, 0x39, 1},
	{0x19, 0x03, 1},
	{0x17, 0x00, 1},
	{0x18, 0x28, 1},
	{0x19, 0x04, 1},
	{0x17, 0x00, 1},
	{0x18, 0x3F, 1},
	{0x19, 0x05, 1},
	{0x17, 0x00, 1},
	{0x18, 0x56, 1},
	{0x19, 0x06, 1},
	{0x17, 0x00, 1},
	{0x18, 0x68, 1},
	{0x19, 0x07, 1},
	{0x17, 0x00, 1},
	{0x18, 0x08, 1},
	{0x19, 0x08, 1},
	{0x17, 0x00, 1},
	{0x18, 0x0F, 1},
	{0x19, 0x09, 1},
	{0x17, 0x00, 1},
	{0x18, 0x27, 1},
	{0x19, 0x0A, 1},
	{0x17, 0x00, 1},
	{0x18, 0x16, 1},
	{0x19, 0x0B, 1},
	{0x17, 0x00, 1},
	{0x18, 0x26, 1},
	{0x19, 0x0C, 1},
	{0x17, 0x00, 1},
	{0x18, 0x37, 1},
	{0x19, 0x0D, 1},
	{0x17, 0x00, 1},
	{0x18, 0x3F, 1},
	{0x19, 0x0E, 1},
	{0x17, 0x00, 1},
	{0x18, 0x1A, 1},
	{0x19, 0x0F, 1},
	{0x17, 0x00, 1},
	{0x18, 0x0B, 1},
	{0x19, 0x10, 1},
	{0x17, 0x00, 1},
	{0x18, 0x16, 1},
	{0x19, 0x11, 1},
	{0x17, 0x00, 1},
	{0x18, 0x13, 1},
	{0x19, 0x12, 1},
	{0x17, 0x00, 1},
	{0x18, 0x09, 1},
	{0x19, 0x13, 1},
	{0x17, 0x00, 1},
	{0x18, 0x11, 1},
	{0x19, 0x14, 1},
	{0x17, 0x00, 1},
	{0x18, 0x0E, 1},
	{0x19, 0x15, 1},
	{0x17, 0x07, 1},
	{0x18, 0xFA, 1},
	{0x19, 0x16, 1},
	{0x17, 0x00, 1},
	{0x18, 0x07, 1},
	{0x19, 0x17, 1},
	{0x17, 0x00, 1},
	{0x18, 0x01, 1},
	{0x19, 0x18, 1},
	{0x17, 0x00, 1},
	{0x18, 0x00, 1},
	{0x19, 0x19, 1},
	{0x17, 0x07, 1},
	{0x18, 0xF1, 1},
	{0x19, 0x1A, 1},
	{0x17, 0x07, 1},
	{0x18, 0xEA, 1},
	{0x19, 0x1B, 1},
	{0x17, 0x07, 1},
	{0x18, 0xCB, 1},
	{0x19, 0x1C, 1},
	{0x17, 0x00, 1},
	{0x18, 0x02, 1},
	{0x19, 0x1D, 1},
	{0x17, 0x07, 1},
	{0x18, 0xEA, 1},
	{0x19, 0x1E, 1},
	{0x17, 0x07, 1},
	{0x18, 0xDB, 1},
	{0x19, 0x1F, 1},
	{0x17, 0x07, 1},
	{0x18, 0xDD, 1},
	{0x19, 0x20, 1},
	{0x17, 0x07, 1},
	{0x18, 0xC7, 1},
	{0x19, 0x21, 1},
	{0x17, 0x07, 1},
	{0x18, 0xAA, 1},
	{0x19, 0x22, 1},
	{0x17, 0x07, 1},
	{0x18, 0x91, 1},
	{0x19, 0x40, 1},
	{0xef, 0x02, 1},   
	{0x3a, 0x20, 1},  
	{0x65, 0x08, 1},  
	{0x0a, 0x90, 1},  
	{0x01, 0x26, 1},  
	{0x03, 0x78, 1},  
	{0x05, 0x00, 1},  
	{0x07, 0xE5, 1},  
	{0x09, 0x0E, 1},  
	{0x0b, 0x86, 1},	
	{0x13, 0xa0, 1},  
	{0x1c, 0x58, 1},  
	{0x24, 0x3d, 1},  
	{0x26, 0x25, 1},  
	{0x29, 0x8f, 1},  
	{0x4b, 0x5b, 1},  
	{0x56, 0x05, 1},  
	{0x58, 0x61, 1},  
	{0x5f, 0x06, 1},  
	{0x67, 0x3c, 1},  
	{0x68, 0x10, 1},  
	{0x69, 0x13, 1},  
	{0x28, 0x9B, 1},   
	{0xef, 0x01, 1},
	{0xd1, 0x64, 1},   
	{0xef, 0x00, 1},
	{0xef, 0x00, 1},
	{0x27, 0x40, 1},  
	{0x29, 0x60, 1},   
	{0xef, 0x00, 1},             //0xef00
	{0x55, 0x00, 1},             //0x5500
	{0x58, 0x00, 1},             //0x5800
	{0x59, 0x12, 1},             //0x5989
	{0x5a, 0x02, 1},             //0x5a01
	{0x5b, 0x60, 1},             //0x5bf3
	{0x5c, 0x00, 1},             //0x5c00
	{0x5d, 0x03, 1},             //0x5d95
	{0x5e, 0x01, 1},             //0x5e01
	{0x5f, 0xe0, 1},             //0x5fe0
	{0x60, 0x00, 1},             //0x6000
	{0x61, 0x38, 1},             //0x6189
	{0x62, 0x02, 1},             //0x6201
	{0x63, 0x3e, 1},             //0x63f3
	{0x64, 0x00, 1},             //0x6400
	{0x65, 0x4a, 1},             //0x6526
	{0x66, 0x01, 1},             //0x6601
	{0x67, 0xb9, 1},             //0x67e0
	{0xef, 0x01, 1},             //0xef01
	{0x77, 0x01, 1},             //0x7701
	{0x78, 0x01, 1},             //0x7801
	{0xef, 0x00, 1},
	{0xd9, 0xc0, 1},    
	{0xef, 0x00, 1},
	{0xad, 0x03, 1},	
	{0xae, 0x20, 1},	
	{0xaf, 0x00, 1},	
	{0xb0, 0xb0, 1},
	{0xef, 0x01, 1},  
	{0x83, 0x79, 1},  
	{0x84, 0x36, 1},  
	{0xa8, 0x41, 1},  
	{0xa9, 0xd4, 1},  
	{0xaf, 0x04, 1},  
	{0xb4, 0xa0, 1},  
	{0xb5, 0xa0, 1},  
	{0xbc, 0x40, 1},
	{0xb7, 0x46, 1},  
	{0xb8, 0x46, 1},    
	{0x71, 0x56, 1},  
	{0x72, 0x07, 1},  
	{0x73, 0x18, 1},	
	{0x74, 0x60, 1},  
	{0x75, 0x40, 1},  
	{0x76, 0x1c, 1},  
	{0xa6, 0xcf, 1},  
	{0xab, 0x12, 1},  
	{0xad, 0x80, 1},  
	{0xcc, 0x09, 1},  
	{0xd5, 0x22, 1},  
	{0xe0, 0x08, 1},  
	{0xe2, 0x14, 1}, //08
	{0xe1, 0x06, 1},  
	{0xe3, 0x11, 1}, //06  
	{0xe4, 0x00, 1},  
	{0xe5, 0x26, 1},  
	{0xe6, 0x00, 1},  
	{0xe7, 0x26, 1},  
	{0xe8, 0x68, 1},  
	{0xef, 0x00, 1},  
	{0xc8, 0x0a, 1},   
	{0xef, 0x00, 1},
	{0x4c, 0x78, 1},
	{0x4d, 0xb7, 1},
	{0x4e, 0x80, 1},
	{0x4f, 0x9c, 1},
	{0x50, 0x6c, 1},
	{0x51, 0x90, 1},
	{0x52, 0x80, 1},
	{0x53, 0xc7, 1},
	{0x54, 0x87, 1}, 
	{0xef, 0x00, 1},
	{0x30, 0x01, 1},
	{0x32, 0x00, 1},
	{0x36, 0x00, 1},
	{0x3a, 0x00, 1},
	{0x33, 0x00, 1},
	{0x34, 0x00, 1},
	{0x37, 0x00, 1},
	{0x38, 0x00, 1},
	{0x3b, 0x00, 1},
	{0x3c, 0x00, 1},
	{0x31, 0xf8, 1},
	{0x35, 0xce, 1},
	{0x39, 0xca, 1},
	{0x3f, 0x01, 1},
	{0x40, 0x36, 1},
	{0x3d, 0x01, 1},
	{0x3e, 0x18, 1},   
	{0xef, 0x00, 1},  
	{0x48, 0x00, 1},
	{0x49, 0x00, 1},
	{0x4A, 0x08, 1},
	{0x4B, 0x04, 1},
	{0x48, 0x01, 1},
	{0x49, 0x00, 1},
	{0x4A, 0x10, 1},
	{0x4B, 0x50, 1},
	{0x48, 0x02, 1},
	{0x49, 0x00, 1},
	{0x4A, 0x20, 1},
	{0x4B, 0x66, 1},
	{0x48, 0x03, 1},
	{0x49, 0x00, 1},
	{0x4A, 0x40, 1},
	{0x4B, 0x5C, 1},
	{0x48, 0x04, 1},
	{0x49, 0x00, 1},
	{0x4A, 0x80, 1},
	{0x4B, 0x47, 1},
	{0x48, 0x05, 1},
	{0x49, 0x01, 1},
	{0x4A, 0x00, 1},
	{0x4B, 0x2E, 1},
	{0x48, 0x06, 1},
	{0x49, 0x01, 1},
	{0x4A, 0xC0, 1},
	{0x4B, 0x20, 1},
	{0x48, 0x07, 1},
	{0x49, 0x02, 1},
	{0x4A, 0x80, 1},
	{0x4B, 0x17, 1},
	{0x48, 0x08, 1},
	{0x49, 0x03, 1},
	{0x4A, 0x00, 1},
	{0x4B, 0x13, 1},
	{0x48, 0x09, 1},
	{0x49, 0x03, 1},
	{0x4A, 0x80, 1},
	{0x4B, 0x11, 1},
	{0x48, 0x0A, 1},
	{0x49, 0x03, 1},
	{0x4A, 0xC0, 1},
	{0x4B, 0x10, 1},
	{0x48, 0x0B, 1},
	{0x4B, 0x0F, 1},      
	{0x48, 0x10, 1},
	{0x49, 0x00, 1},
	{0x4A, 0x08, 1},
	{0x4B, 0x04, 1},
	{0x48, 0x11, 1},
	{0x49, 0x00, 1},
	{0x4A, 0x10, 1},
	{0x4B, 0x50, 1},
	{0x48, 0x12, 1},
	{0x49, 0x00, 1},
	{0x4A, 0x20, 1},
	{0x4B, 0x66, 1},
	{0x48, 0x13, 1},
	{0x49, 0x00, 1},
	{0x4A, 0x40, 1},
	{0x4B, 0x5C, 1},
	{0x48, 0x14, 1},
	{0x49, 0x00, 1},
	{0x4A, 0x80, 1},
	{0x4B, 0x47, 1},
	{0x48, 0x15, 1},
	{0x49, 0x01, 1},
	{0x4A, 0x00, 1},
	{0x4B, 0x2E, 1},
	{0x48, 0x16, 1},
	{0x49, 0x01, 1},
	{0x4A, 0xC0, 1},
	{0x4B, 0x20, 1},
	{0x48, 0x17, 1},
	{0x49, 0x02, 1},
	{0x4A, 0x80, 1},
	{0x4B, 0x17, 1},
	{0x48, 0x18, 1},
	{0x49, 0x03, 1},
	{0x4A, 0x00, 1},
	{0x4B, 0x13, 1},
	{0x48, 0x19, 1},
	{0x49, 0x03, 1},
	{0x4A, 0x80, 1},
	{0x4B, 0x11, 1},
	{0x48, 0x1A, 1},
	{0x49, 0x03, 1},
	{0x4A, 0xC0, 1},
	{0x4B, 0x10, 1},
	{0x48, 0x1B, 1},
	{0x4B, 0x0F, 1},
	{0x48, 0x20, 1},
	{0x49, 0x00, 1},
	{0x4A, 0x08, 1},
	{0x4B, 0x04, 1},
	{0x48, 0x21, 1},
	{0x49, 0x00, 1},
	{0x4A, 0x10, 1},
	{0x4B, 0x50, 1},
	{0x48, 0x22, 1},
	{0x49, 0x00, 1},
	{0x4A, 0x20, 1},
	{0x4B, 0x66, 1},
	{0x48, 0x23, 1},
	{0x49, 0x00, 1},
	{0x4A, 0x40, 1},
	{0x4B, 0x5C, 1},
	{0x48, 0x24, 1},
	{0x49, 0x00, 1},
	{0x4A, 0x80, 1},
	{0x4B, 0x47, 1},
	{0x48, 0x25, 1},
	{0x49, 0x01, 1},
	{0x4A, 0x00, 1},
	{0x4B, 0x2E, 1},
	{0x48, 0x26, 1},
	{0x49, 0x01, 1},
	{0x4A, 0xC0, 1},
	{0x4B, 0x20, 1},
	{0x48, 0x27, 1},
	{0x49, 0x02, 1},
	{0x4A, 0x80, 1},
	{0x4B, 0x17, 1},
	{0x48, 0x28, 1},
	{0x49, 0x03, 1},
	{0x4A, 0x00, 1},
	{0x4B, 0x13, 1},
	{0x48, 0x29, 1},
	{0x49, 0x03, 1},
	{0x4A, 0x80, 1},
	{0x4B, 0x11, 1},
	{0x48, 0x2A, 1},
	{0x49, 0x03, 1},
	{0x4A, 0xC0, 1},
	{0x4B, 0x10, 1},
	{0x48, 0x2B, 1},
	{0x4B, 0x0F, 1},
	{0x48, 0x80, 1},
	{0xef, 0x00, 1},
	{0x8c, 0x0c, 1},	
	{0x74, 0x01, 1},	
	{0x79, 0x46, 1},  
	{0x75, 0x01, 1},	
	{0x76, 0xc0, 1},  
	{0x77, 0x02, 1},	
	{0x78, 0x00, 1},  
	{0x72, 0xa0, 1},   
	{0xd8, 0x00, 1},   
	{0xef, 0x01, 1},
	{0x85, 0x00, 1},  
	{0x86, 0xf0, 1},	
	{0xef, 0x01, 1},
	{0xb9, 0x08, 1},  
	{0xba, 0x03, 1},  
	{0xbb, 0x01, 1},
	{0xbc, 0x40, 1}, //50
	{0xbd, 0x60, 1},  
	{0xeb, 0x41, 1},    
	{0xef, 0x00, 1},
	{0x93, 0x00, 1},
	{0x94, 0xc0, 1},
	{0x95, 0xe0, 1},  
	{0x96, 0xf0, 1},  
	{0x97, 0xf0, 1},
	{0x98, 0x78, 1},
	{0x99, 0x78, 1},
	{0xA0, 0x70, 1}, //7c
	{0xA1, 0x3c, 1},
	{0xA2, 0x1e, 1},
	{0xA3, 0x0e, 1},
	{0xA4, 0x07, 1},
	{0xA5, 0x03, 1},
	{0xA6, 0x01, 1},  
	{0xef, 0x01, 1},
	{0xdd, 0x31, 1},
	{0xcd, 0x21, 1},
	{0xde, 0x31, 1},  
	{0xef, 0x00, 1},
	{0x57, 0x00, 1},
	{0xef, 0x00, 1},
	{0x43, 0x40, 1},  
	{0x42, 0x43, 1},  
	{0x44, 0x54, 1},  
	{0xef, 0x02, 1},
	{0x1d, 0x00, 1}, //AE start
	{0x0e, 0x01, 1}, //AE start
	{0xef, 0x01, 1},  
	{0xa5, 0x01, 1},  
	{0xe9, 0x84, 1},  
	{0x70, 0x97, 1},  
	{0xef, 0x00, 1},
	{0x2f, 0x00, 1},
	{0xb9, 0x02, 1},
	{0xba, 0x80, 1},
	{0xbb, 0x80, 1},
	{0xbc, 0x88, 1},
	{0xbd, 0x10, 1},
	{0xd1, 0x05, 1},
	{0xd2, 0xf5, 1},
	{0xef, 0x00, 1},
	{0xbe, 0xcf, 1},  
	{0xc0, 0xcf, 1},
	{0x2d, 0x12, 1},    
	{0xef, 0x01, 1},
	{0x93, 0x00, 1},  
	{0x85, 0x00, 1}, //20
	{0x86, 0xf0, 1},
	{0x87, 0x4e, 1},  
	{0x88, 0xa0, 1}                                 

 };

const TSnrSizeCfg gSensorSizeCfg_S5KA3AFX[] = 
{
	{
       	{0,NULL},
               	{0,NULL},
	      	{640, 480},
		1,
		1

	}

};

const TSnrInfo gSensorInfo_S5KA3AFX = 
{
       "S5KA3AFX yuv mode",
	VIM_SNR_YUV,							//yuv mode
	1,							//not use PCLK of sensor
	0,							//[0]: VSync delay enable; [1]: Fifo VSync delay enable; [2]: Use PCLK Negtive Edge.
	0,							//0 -- I2C, 1 -- Serial bus;
	0,							//[0]: 1 -- OV I2C bus
	20,							//I2C Rate : 100KHz
	0xc4,							//I2C address
	0xc4,							//ISP I2C address for special sensor
	0x07,							//power config
	0,							//reset sensor
	3,							//brightness 
	3,							//contrast

	{0,NULL},											//sensor standby                                                                                  								
       {sizeof(gSnrSizeRegValVGA_S5KA3AFX) / sizeof(TReg), (PTReg)&gSnrSizeRegValVGA_S5KA3AFX},                                                       //snr initial value
	{sizeof(gSifPwrOnSeqRegVal_S5KA3AFX) / sizeof(TReg), (PTReg)&gSifPwrOnSeqRegVal_S5KA3AFX},		//Sif config sequence(Reg.800 bit0~2) when sensor power on                                        
	{0, NULL},											//Sif config sequence(Reg.800 bit0~2) when sensor standby                                         
	{sizeof(gSifRegVal_S5KA3AFX) / sizeof(TReg), (PTReg)&gSifRegVal_S5KA3AFX},				//sif initial value                                                                               
	{sizeof(gSensorSizeCfg_S5KA3AFX) / sizeof(TSnrSizeCfg), (PTSnrSizeCfg)&gSensorSizeCfg_S5KA3AFX},	//size configure                                                                                  

	SensorSetReg_S5KA3AFX,										
	SensorGetReg_S5KA3AFX,										
	CheckId_S5KA3AFX,											
	SensorSetMirrorFlip_S5KA3AFX,									
	NULL,
	SensorSetBrightness_S5KA3AFX,																									//set et callback
	NULL,
	SensorSetMode_S5KA3AFX,
	NULL,												
	NULL,												
	{	//ythd of flash,et of flash
		0x20,			10
	},
};

#endif


////////////////////////////////////////////////////////////////
//
//    GC306  (fengyu)
//
////////////////////////////////////////////////////////////////

#if V5_DEF_SNR_GC306_YUV

/* write sensor register callback */

void SensorSetReg_GC306(UINT16 uAddr, UINT16 uVal)
{
         V5B_SifI2cWriteByte((UINT8)uAddr, (UINT8)uVal);
}

void SensorGetReg_GC306(UINT8 uAddr, UINT16 *uVal)
{
	V5B_SifI2cReadByte((UINT8)uAddr, (UINT8*)uVal);
}

UINT8 CheckId_GC306(void)
{
	     UINT8 temp1; 

         V5B_SifI2cReadByte(0x00, &temp1); 

        
		 if(temp1==0x97) 
		 {       
                 return SUCCEED; 
		 }
         else 
         {        
			     return FAILED; 
		 }
}

/******************************************************************

         Desc: set sensor mirror and flip callback 

         Para: BIT0 -- mirror

                     BIT1 -- flip

*******************************************************************/

void SensorSetMirrorFlip_GC306(UINT8 val)
{
        UINT8 x;
	val=~val;
	V5B_SifI2cReadByte(0x0e, &x);
	V5B_SifI2cWriteByte(0x0e, (UINT8)(x&0xcf)|((val&0x03)<<4));
}

/******************************************************************

         Desc: set sensor contrast  callback 

         Para: 1-5 step

*******************************************************************/

void SensorSetContrast_GC306(UINT8 val)
{
	switch(val)
	{	
	case 1:     
        
		V5B_SifI2cWriteByte(0xa1, 0x28);
		break;  

	case 2:                                                    
			
		V5B_SifI2cWriteByte(0xa1, 0x38);
		break;

   	case 3:                                
		
		V5B_SifI2cWriteByte(0xa1, 0x48);
		break; 
    
	case 4:                                
		
		V5B_SifI2cWriteByte(0xa1, 0x58);
		break; 
		
	case 5:                                
		
		V5B_SifI2cWriteByte(0xa1, 0x68);
		break; 

	default:
		break;
	}					
}

/******************************************************************

         Desc: set sensor Brightness  callback 

         Para: 1-5 step

*******************************************************************/

void SensorSetBrightness_GC306(UINT8 val)
{
	switch(val)
	{
	case 1:
	 	
        V5B_SifI2cWriteByte(0xd1, 0x28);
		break;	

	case 2:
	
        V5B_SifI2cWriteByte(0xd1, 0x48);
		break;

	case 3:
		
        V5B_SifI2cWriteByte(0xd1, 0x68);
		break;

	case 4:
		
        V5B_SifI2cWriteByte(0xd1, 0x88);
		break;

	case 5:   //Bright
		
        V5B_SifI2cWriteByte(0xd1, 0xa8);
		break;

	default:
		break;		
	}
}

/******************************************************************

         Desc: set sensor mod  callback 

         Para:  val: 	1	50hz
         	    		2	60hz
         	     	    	3	night 
         	     	     	4	outdoor

*******************************************************************/

void SensorSetMode_GC306(UINT8 val)
{

	switch (val)
	{	
		case 1:
			
			V5B_SifI2cWriteByte(0x59, 0xf1);
            V5B_SifI2cWriteByte(0x5a, 0xf1);
            V5B_SifI2cWriteByte(0x5b, 0xf1);
            V5B_SifI2cWriteByte(0x5c, 0xf1);

            V5B_SifI2cWriteByte(0x61, 0x40);  
            V5B_SifI2cWriteByte(0x63, 0x48); 
            V5B_SifI2cWriteByte(0x65, 0x46); 
            V5B_SifI2cWriteByte(0x67, 0x40);  
            V5B_SifI2cWriteByte(0x68, 0x40);  


            V5B_SifI2cWriteByte(0x40, 0x5c);
            V5B_SifI2cWriteByte(0x19, 0x02);
            V5B_SifI2cWriteByte(0x1a, 0x20);
            V5B_SifI2cWriteByte(0xa1, 0x40);

            V5B_SifI2cWriteByte(0x90, 0x0a);
            V5B_SifI2cWriteByte(0x91, 0x14);
            V5B_SifI2cWriteByte(0x92, 0x1e);
            V5B_SifI2cWriteByte(0x93, 0x34);
            V5B_SifI2cWriteByte(0x94, 0x4a);
            V5B_SifI2cWriteByte(0x95, 0x5f);
            V5B_SifI2cWriteByte(0x96, 0x74);
            V5B_SifI2cWriteByte(0x97, 0x86);
            V5B_SifI2cWriteByte(0x98, 0xa6);
            V5B_SifI2cWriteByte(0x99, 0xc0);
            V5B_SifI2cWriteByte(0x9a, 0xd7);
            V5B_SifI2cWriteByte(0x9b, 0xed);
            V5B_SifI2cWriteByte(0x9c, 0xfe);

            V5B_SifI2cWriteByte(0xd6, 0x90);
			break;

		case 2:
			
			V5B_SifI2cWriteByte(0x59, 0xf1);
            V5B_SifI2cWriteByte(0x5a, 0xf1);
            V5B_SifI2cWriteByte(0x5b, 0xf1);
            V5B_SifI2cWriteByte(0x5c, 0xf1);

            V5B_SifI2cWriteByte(0x61, 0x40);  
            V5B_SifI2cWriteByte(0x63, 0x48); 
            V5B_SifI2cWriteByte(0x65, 0x46); 
            V5B_SifI2cWriteByte(0x67, 0x40);  
            V5B_SifI2cWriteByte(0x68, 0x40);  


            V5B_SifI2cWriteByte(0x40, 0x5c);
            V5B_SifI2cWriteByte(0x19, 0x02);
            V5B_SifI2cWriteByte(0x1a, 0x20);
            V5B_SifI2cWriteByte(0xa1, 0x40);

            V5B_SifI2cWriteByte(0x90, 0x0a);
            V5B_SifI2cWriteByte(0x91, 0x14);
            V5B_SifI2cWriteByte(0x92, 0x1e);
            V5B_SifI2cWriteByte(0x93, 0x34);
            V5B_SifI2cWriteByte(0x94, 0x4a);
            V5B_SifI2cWriteByte(0x95, 0x5f);
            V5B_SifI2cWriteByte(0x96, 0x74);
            V5B_SifI2cWriteByte(0x97, 0x86);
            V5B_SifI2cWriteByte(0x98, 0xa6);
            V5B_SifI2cWriteByte(0x99, 0xc0);
            V5B_SifI2cWriteByte(0x9a, 0xd7);
            V5B_SifI2cWriteByte(0x9b, 0xed);
            V5B_SifI2cWriteByte(0x9c, 0xfe);

			V5B_SifI2cWriteByte(0xd6, 0x78);   
			break;

		case 4:  // 3 and 4 reverse
			    
            V5B_SifI2cWriteByte(0x59, 0xf1);
            V5B_SifI2cWriteByte(0x5a, 0xf1);
			V5B_SifI2cWriteByte(0x5b, 0xf1);
			V5B_SifI2cWriteByte(0x5c, 0xf1);

			V5B_SifI2cWriteByte(0x61, 0x40);  //   40 manual gain grG
			V5B_SifI2cWriteByte(0x63, 0x48);  //   57 manual gain R
			V5B_SifI2cWriteByte(0x65, 0x46);  //   5e manual gain B
			V5B_SifI2cWriteByte(0x67, 0x40);  //   40 manual gain bgG
			V5B_SifI2cWriteByte(0x68, 0x40);  // global gain. 0x20, 1.0

			V5B_SifI2cWriteByte(0x40, 0x5c);
			V5B_SifI2cWriteByte(0x41, 0x9d);
			V5B_SifI2cWriteByte(0x19, 0x34);
			V5B_SifI2cWriteByte(0x1a, 0x43);
			V5B_SifI2cWriteByte(0xa1, 0x40);

			V5B_SifI2cWriteByte(0xd8, 0x0f);

			V5B_SifI2cWriteByte(0x90, 0x0a);
			V5B_SifI2cWriteByte(0x91, 0x14);
			V5B_SifI2cWriteByte(0x92, 0x1e);
			V5B_SifI2cWriteByte(0x93, 0x34);
			V5B_SifI2cWriteByte(0x94, 0x4a);
			V5B_SifI2cWriteByte(0x95, 0x5f);
			V5B_SifI2cWriteByte(0x96, 0x74);
			V5B_SifI2cWriteByte(0x97, 0x86);
			V5B_SifI2cWriteByte(0x98, 0xa6);
			V5B_SifI2cWriteByte(0x99, 0xc0);
			V5B_SifI2cWriteByte(0x9a, 0xd7);
			V5B_SifI2cWriteByte(0x9b, 0xed);
			V5B_SifI2cWriteByte(0x9c, 0xfe);

			V5B_SifI2cWriteByte(0x82, 0x00);
			V5B_SifI2cWriteByte(0x87, 0x90);
			V5B_SifI2cWriteByte(0x88, 0x10);
			V5B_SifI2cWriteByte(0x89, 0x08);
			break;

		case 3:	
		        
            V5B_SifI2cWriteByte(0x59, 0xf1);
            V5B_SifI2cWriteByte(0x5a, 0xf1);
            V5B_SifI2cWriteByte(0x5b, 0xf1);
            V5B_SifI2cWriteByte(0x5c, 0xf1);

            V5B_SifI2cWriteByte(0x61, 0x40);  //   40 manual gain grG
            V5B_SifI2cWriteByte(0x63, 0x48); //   57 manual gain R
            V5B_SifI2cWriteByte(0x65, 0x46); //   5e manual gain B
            V5B_SifI2cWriteByte(0x67, 0x40);  //   40 manual gain bgG
            V5B_SifI2cWriteByte(0x68, 0x40);  // global gain. 0x20, 1.0


            V5B_SifI2cWriteByte(0x40, 0x5c);
            V5B_SifI2cWriteByte(0x19, 0x02);
            V5B_SifI2cWriteByte(0x1a, 0x20);
            V5B_SifI2cWriteByte(0xa1, 0x40);

            V5B_SifI2cWriteByte(0x90, 0x0a);
            V5B_SifI2cWriteByte(0x91, 0x14);
            V5B_SifI2cWriteByte(0x92, 0x1e);
            V5B_SifI2cWriteByte(0x93, 0x34);
            V5B_SifI2cWriteByte(0x94, 0x4a);
            V5B_SifI2cWriteByte(0x95, 0x5f);
            V5B_SifI2cWriteByte(0x96, 0x74);
            V5B_SifI2cWriteByte(0x97, 0x86);
            V5B_SifI2cWriteByte(0x98, 0xa6);
            V5B_SifI2cWriteByte(0x99, 0xc0);
            V5B_SifI2cWriteByte(0x9a, 0xd7);
            V5B_SifI2cWriteByte(0x9b, 0xed);
            V5B_SifI2cWriteByte(0x9c, 0xfe);
			break;

		default:
			break;
	}
}

const TReg gSifPwrOnSeqRegVal_GC306[] =   //528 sif 0x800  power on
{

	{0x00, 0x63,	1},   //power on
	{0x00, 0x62,	1},   // reset
	{0x00, 0x63,	1}    // sensor enable
  	
};

const TReg gSifRegVal_GC306[] =  // SIF config (0x800)
{        
        {0x02, 0x02, 0},      // config
        {0x03, 0x02, 0},      // Vsync  {0x03, 0x1b, 0},      
        {0x04, 0x04, 0},      // Hsync
        {0x21, 0x00, 0},      // vref_start  // fengyu
        {0x22, 0x01, 0},      // add by fengyu (verf_stop)
        {0x23, 0xe0, 0},      // vref_stop
        {0x2c, 0x02, 0},      // Colmax registers
        {0x2d, 0x80, 0},      //
        {0x2e, 0x01, 0},      // Rowmax register
        {0x2f, 0xe0, 0},      //
        {0x01, 0x41, 0}       // Sensor operator mode control register {0x01, 0x45, 0}
};

/* Sensor size control configure info */

const TReg gSnrSizeRegValVGA_GC306[] = 
{      	
 
    {0xf0, 0x00, 1},    
	{0x01, 0x42, 1},   
	{0x02, 0x22, 1},   

	{0x03, 0x04, 1},
	{0x04, 0x90, 1},
	{0x05, 0x00, 1},
	{0x06, 0x00, 1},
	{0x07, 0x00, 1},
	{0x08, 0x04, 1},
	{0x09, 0x01, 1},
	{0x0a, 0xe8, 1},
	{0x0b, 0x02, 1},
	{0x0c, 0x88, 1},

	{0x0d, 0x22, 1},
	{0x0e, 0x20, 1},
	{0x0f, 0x80, 1},
	{0x10, 0x24, 1},
	{0x11, 0x10, 1},

	{0x12, 0x10, 1},
	{0x13, 0x00, 1},
	{0x14, 0x00, 1},
	{0x15, 0x08, 1},
	{0x16, 0x04, 1},

	{0x17, 0x00, 1},
	{0x18, 0x02, 1},
	{0x19, 0x02, 1},
	{0x1a, 0x20, 1},
	{0x1b, 0x00, 1},
	{0x1c, 0x02, 1},
	{0x1d, 0x02, 1},
	{0x1e, 0x00, 1},

	{0x40, 0x10, 1},  
	{0x41, 0x00, 1},  
	{0xb0, 0x00, 1},  
	{0xba, 0x00, 1},  
	{0xbb, 0x00, 1},
	{0xbc, 0x00, 1},

	{0x40, 0x7c, 1},  
	{0x41, 0x9d, 1}, 
	{0x42, 0x30, 1}, 
	{0x43, 0x60, 1}, 
	{0x44, 0xe0, 1},
	{0x45, 0x27, 1}, 
	{0x46, 0x20, 1},
	{0x47, 0x00, 1},
	{0x48, 0x00, 1},
	{0x49, 0x00, 1},
	{0x4a, 0x00, 1},
	{0x4b, 0x00, 1},
	{0x4c, 0x00, 1},
	{0x4d, 0x00, 1},
	{0x4e, 0x23, 1}, 
	{0x4f, 0x2a, 1}, 

	{0x50, 0x00, 1},
	{0x51, 0x20, 1},
	{0x52, 0x40, 1},
	{0x53, 0x60, 1},
	{0x54, 0x80, 1},
	{0x55, 0xa0, 1},
	{0x56, 0xc0, 1},
	{0x57, 0xe0, 1},     

	{0x59, 0xef, 1},  
	{0x5a, 0xef, 1},  
	{0x5b, 0xef, 1},  
	{0x5c, 0xef, 1},  

	{0x61, 0x30, 1},  
	{0x63, 0x30, 1},  
	{0x65, 0x30, 1},  
	{0x67, 0x30, 1},  
	{0x68, 0x40, 1},  


	{0x69, 0x41, 1},   
	{0x6a, 0xf8, 1},  
	{0x6b, 0xf9, 1},  
	{0x6c, 0xff, 1},  
	{0x6d, 0x53, 1},  
	{0x6e, 0xe7, 1},  


	{0x70, 0x00, 1},  
	{0x71, 0x10, 1},
	{0x72, 0x00, 1},
	{0x73, 0x14, 1},
	{0x74, 0x00, 1},
	{0x75, 0x10, 1},
	{0x76, 0x28, 1},
	{0x77, 0x3c, 1},
	{0x78, 0x50, 1},

	{0x80, 0x18, 1}, 
	{0x81, 0x18, 1},  
	{0x87, 0x92, 1},
	{0x82, 0x24, 1},   

	{0x83, 0x14, 1},
	{0x84, 0x26, 1},
	{0x85, 0x0a, 1},

	{0x88, 0x0a, 1},
	{0x89, 0x04, 1},

	{0x8b, 0x3f, 1},
	{0x8c, 0x11, 1},      

	{0x90, 0x18, 1},
	{0x91, 0x2b, 1},
	{0x92, 0x3d, 1},
	{0x93, 0x5b, 1},
	{0x94, 0x77, 1},
	{0x95, 0x8f, 1},
	{0x96, 0xa3, 1},
	{0x97, 0xb5, 1},
	{0x98, 0xcf, 1},
	{0x99, 0xe1, 1},
	{0x9a, 0xef, 1},
	{0x9b, 0xf8, 1},
	{0x9c, 0xfe, 1},

	{0x9d, 0x80, 1},
	{0x9e, 0x40, 1},  

	{0xa0, 0x60, 1},  	
	{0xa1, 0x48, 1},  
	{0xa2, 0x24, 1},
	{0xa3, 0x24, 1},

	{0xa4, 0xc0, 1}, 
	{0xa5, 0x02, 1}, 
	{0xa6, 0x60, 1}, 
	{0xa7, 0x04, 1}, 

	{0xa8, 0xf4, 1},
	{0xa9, 0x0c, 1},
	{0xaa, 0x01, 1},
	{0xab, 0x00, 1},
	{0xac, 0xf8, 1},
	{0xad, 0x10, 1},
	{0xae, 0x80, 1},
	{0xaf, 0x80, 1},

	{0xb1, 0x10, 1},  
	{0xb2, 0xff, 1},  

	{0xbf, 0x10, 1},
	{0xc0, 0x20, 1},
	{0xc1, 0xf0, 1},
	{0xc2, 0x01, 1},
	{0xc3, 0x80, 1},
	{0xc4, 0x01, 1},
	{0xc5, 0x21, 1},
	{0xc6, 0x70, 1},
	{0xc7, 0x00, 1},
	{0xc8, 0x00, 1},
	{0xc9, 0x00, 1},
	{0xca, 0x40, 1},
	{0xcb, 0x40, 1},
	{0xcc, 0x40, 1},
	{0xcd, 0x40, 1},
	{0xce, 0x40, 1},
	{0xcf, 0x40, 1},

	{0xd0, 0x00, 1},
	{0xd1, 0x68, 1},  
	{0xd2, 0x46, 1},
	{0xd3, 0x00, 1},
	{0xd4, 0x80, 1},
	{0xd5, 0xf0, 1},
	{0xd6, 0x90, 1},
	{0xd7, 0x02, 1},  
	{0xd8, 0x06, 1},
	{0xd9, 0x01, 1}, 
	{0xda, 0x0c, 1}, 
	{0xdb, 0x00, 1},
	{0xdc, 0x00, 1},
	{0xdd, 0x40, 1}, 

	{0xe0, 0x02, 1},
	{0xe1, 0x02, 1},
	{0xe2, 0x26, 1},
	{0xe3, 0x1c, 1},
	{0xe4, 0x11, 1},
	{0xe5, 0x08, 1},
	{0xe6, 0x1d, 1},
	{0xe7, 0x14, 1},

	{0xf1, 0x00, 1},  
	{0xf2, 0x00, 1},
	{0xf3, 0x50, 1},
	{0xf4, 0x08, 1},
	{0xf5, 0x08, 1},
	{0xf6, 0x00, 1},
	{0xf7, 0x08, 1},
	{0xf8, 0x08, 1},

	{0xf9, 0x14, 1},  
	{0xfa, 0x0c, 1},  
	{0xfb, 0x0b, 1},  
	{0xfc, 0x50, 1},  

	{0xf0, 0x01, 1},   

	{0x00, 0xd2, 1},    
	{0x01, 0x74, 1},
	{0x02, 0xa0, 1},
	{0x03, 0x48, 1},
	{0x04, 0x40, 1},
	{0x05, 0x7c, 1},
	{0x06, 0x19, 1},
	{0x07, 0x02, 1},
	{0x08, 0x1a, 1},
	{0x09, 0x20, 1},
	{0x0a, 0x68, 1},
	{0x0b, 0x40, 1},
	{0x0c, 0xf5, 1},
	{0x0d, 0x08, 1},
	{0x0e, 0xf3, 1},
	{0x0f, 0x50, 1},
	{0x10, 0xf6, 1},
	{0x11, 0x00, 1},  


	{0xf0, 0x00, 1},   
	{0xf1, 0x01, 1}, 
	
    {0x44, 0xe2, 1},  // 0xd2

};

/* Sensor size control configure info */

const TSnrSizeCfg gSensorSizeCfg_GC306[] = 
{
	{
               {0,NULL},
               {0,NULL},     // sensor interface
	       {640, 480},  
//		0,         //0x805  MCLK (0:24M)
//		0          //0x806  PCLK (0:24M)
		1,
		1


	},

};

 

const TSnrInfo gSensorInfo_GC306 = 

{
       "GC306 yuv mode",
	VIM_SNR_YUV,							//yuv mode
	0,	//0x85c bit1  //High priority for 0x806				         // use PCLK of sensor
	0,	//0x85c Bit4-bit2						//[0]: VSync delay enable; [1]: Fifo VSync delay enable; [2]: Use PCLK Negtive Edge.
	0,							//0 -- I2C, 1 -- Serial bus;
	0,							//[0]: 1 -- OV I2C bus, 0 -- Normal sensor
	100,							//I2C Rate : 100KHz
	0x20,							//I2C address
	0x20, 							//ISP I2C address for special sensor
	0x03,							//power config
	0,							//reset sensor  OV Sensor SXGA->VGA hardware reset.

	3,							//brightness 
	3,							//contrast
	
	{0,NULL},														//sensor standby
	{sizeof(gSnrSizeRegValVGA_GC306) / sizeof(TReg), (PTReg)&gSnrSizeRegValVGA_GC306},    // Sensor Size List above                                                  //snr initial value	
	
	{sizeof(gSifPwrOnSeqRegVal_GC306) / sizeof(TReg), (PTReg)&gSifPwrOnSeqRegVal_GC306},		//Sif config sequence(Reg.800 bit0~2) when sensor power on
	{0, NULL},																								//Sif config sequence(Reg.800 bit0~2) when sensor standby
	{sizeof(gSifRegVal_GC306) / sizeof(TReg), (PTReg)&gSifRegVal_GC306},						//sif initial value
																							//isp initial value
	{sizeof(gSensorSizeCfg_GC306) / sizeof(TSnrSizeCfg), (PTSnrSizeCfg)&gSensorSizeCfg_GC306},		//size configure 

	SensorSetReg_GC306,																				//set reg callback
	SensorGetReg_GC306,																				//get reg callback
	CheckId_GC306,																				//Sensor ID Check Call
	SensorSetMirrorFlip_GC306,																									//Set Sensor Mirror Flip Call
	SensorSetContrast_GC306,																									//Sensor switch size callback
	SensorSetBrightness_GC306,																									//set et callback
	NULL,	
	SensorSetMode_GC306,
	NULL,																									//Sensor switch size callback
	NULL,
	{	//ythd of flash,et of flash
		0x20,			10
	},
};

#endif


////////////////////////////////////////////////////////////////
//
//    OM6802  (fengyu)
//
////////////////////////////////////////////////////////////////

#if V5_DEF_SNR_OM6802_YUV

/* write sensor register callback */

void SensorSetReg_OM6802(UINT16 uAddr, UINT16 uVal)
{
         V5B_SifI2cWriteByte((UINT8)uAddr, (UINT8)uVal);
}

void SensorGetReg_OM6802(UINT8 uAddr, UINT16 *uVal)
{
	V5B_SifI2cReadByte((UINT8)uAddr, (UINT8*)uVal);
}

UINT8 CheckId_OM6802(void)
{
   
	     UINT8 temp1;

         V5B_SifI2cReadByte(0x00, &temp1); 

         if(temp1==0xff) 
		 {               
			 return SUCCEED; 
		 }
         else 
         {        
			   return FAILED;
		 }
	

       
}

/******************************************************************

         Desc: set sensor mirror and flip callback 

         Para: BIT0 -- mirror

                     BIT1 -- flip

*******************************************************************/

void SensorSetMirrorFlip_OM6802(UINT8 val)
{
    
    if(val == 0)  // Normal
		
		V5B_SifI2cWriteByte(0xfd, 0x40);
	

	else if(val == 1) // Mirror

		V5B_SifI2cWriteByte(0xfd, 0xc0); 
	

	else if(val == 2) // Flip

		V5B_SifI2cWriteByte(0xfd, 0x00);
		

	else if(val == 3) // Rotation

		V5B_SifI2cWriteByte(0xfd, 0x80);
        
}

/******************************************************************

         Desc: set sensor contrast  callback 

         Para: 1-5 step

*******************************************************************/

void SensorSetContrast_OM6802(UINT8 val)
{
	switch(val)
	{	
	case 1:     
        
		V5B_SifI2cWriteByte(0xe7, 0xa0);
		break;  

	case 2:                                                    
			
		V5B_SifI2cWriteByte(0xe7, 0xd0);
		break;

   	case 3:                                
		
		V5B_SifI2cWriteByte(0xe7, 0x00);
		break; 
    
	case 4:                                
		
		V5B_SifI2cWriteByte(0xe7, 0x30);
		break; 
		
	case 5:                                
		
		V5B_SifI2cWriteByte(0xe7, 0x60);
		break; 

	default:
		break;
	}					
}

/******************************************************************

         Desc: set sensor Brightness  callback 

         Para: 1-5 step

*******************************************************************/

void SensorSetBrightness_OM6802(UINT8 val)
{
	switch(val)
	{
	case 1:
		
        V5B_SifI2cWriteByte(0xe6, 0x80);
		break;
		
	case 2:
	
        V5B_SifI2cWriteByte(0xe6, 0xb0);
		break;

	case 3:
		
        V5B_SifI2cWriteByte(0xe6, 0xe0);
		break;

	case 4:
		
        V5B_SifI2cWriteByte(0xe6, 0x10);
		break;

	case 5:   //Brightness
		
        V5B_SifI2cWriteByte(0xe6, 0x40);
		break;

	default:
		break;		
	}
}

/******************************************************************

         Desc: set sensor mod  callback 

         Para:  val: 	1	50hz
         	    		2	60hz
         	     	    	3	night 
         	     	     	4	outdoor

*******************************************************************/

void SensorSetMode_OM6802(UINT8 val)
{
	//UINT8 uTemp;
	switch (val)
	{	
		case 1:  //50hz

			V5B_SifI2cWriteByte(0xe9, 0x5f);
			break;

		case 2:  //60hz
			
			V5B_SifI2cWriteByte(0xe9, 0x6f);
			break;

		case 4:  // Night Mode
			    
			V5B_SifI2cWriteByte(0xe9, 0xdf);
			break;

		case 3:	
		        
            V5B_SifI2cWriteByte(0xe9, 0x4f);
			break;

		default:
			break;
	}
}

const TReg gSifPwrOnSeqRegVal_OM6802[] =   //528 sif 0x800  power on
{

	{0x00, 0x62,	1},   //power on
	{0x00, 0x63,	1},   // reset
	{0x00, 0x62,	1}    // sensor enable
  	
};

const TReg gSifRegVal_OM6802[] =  // SIF config (0x800)
{        
        {0x02, 0x02, 0},      // config
        {0x03, 0x02, 0},      // Vsync  {0x03, 0x1b, 0},      
        {0x04, 0x04, 0},      // Hsync
		{0x05, 0x01, 0},      // add by fengyu
        {0x06, 0x01, 0},      //
        {0x21, 0x00, 0},      // vref_start  // fengyu
        {0x22, 0x01, 0},      // add by fengyu (verf_stop)
        {0x23, 0xe0, 0},      // vref_stop
        {0x2c, 0x02, 0},      // Colmax registers
        {0x2d, 0x80, 0},      //
        {0x2e, 0x01, 0},      // Rowmax register
        {0x2f, 0xe0, 0},      //
        {0x01, 0x41, 0}       // Sensor operator mode control register {0x01, 0x45, 0}
};

/* Sensor size control configure info */

const TReg gSnrSizeRegValVGA_OM6802[] = 
{      	
	
	{0xdf, 0x12, 3}, 
	{0x5b, 0x88, 3}, 
	{0xed, 0xf4, 3}, 
	{0xee, 0x01, 3}, 
	{0xf2, 0xef, 3}, 
	{0xfe, 0x78, 3}, 
	{0xe7, 0x00, 3}, 
	{0xe6, 0xe0, 3}, 
	{0xe8, 0x25, 3}, 
	{0xf0, 0xc5, 3}, 
	{0xe9, 0x5f, 3}, 
	{0x86, 0xff, 3}, 
	{0x98, 0xf4, 3}, 
	{0x9a, 0x14, 3}, 
	{0x9b, 0x32, 3}, 
	{0x99, 0xa0, 3}, 
	{0x28, 0x10, 3}, 
	{0x29, 0x20, 3}, 
	{0x3a, 0x20, 3}, 
	{0x3e, 0x00, 3}, 
	{0x15, 0x2f, 3}, 
	{0x16, 0xf6, 3}, 
	{0x17, 0xfa, 3}, 
	{0x18, 0xf7, 3}, 
	{0x19, 0x22, 3}, 
	{0x1a, 0xf7, 3}, 
	{0x1b, 0xfe, 3}, 
	{0x1c, 0xf0, 3}, 
	{0x1d, 0x2e, 3}, 
	{0x01, 0x75, 3}, 
	{0x03, 0x20, 3}, 
	{0x0b, 0xef, 3}, 
	{0xfd, 0x40, 3}, 
	{0xdd, 0xd3, 3},  //  UV Swap
	
};

/* Sensor size control configure info */

const TSnrSizeCfg gSensorSizeCfg_OM6802[] = 
{
	{
               {0,NULL},
               {0,NULL},     // sensor interface
	       {640, 480},  
		1,         //0x805  MCLK (0:24M)
		1          //0x806  PCLK (0:24M)
	},

};

 

const TSnrInfo gSensorInfo_OM6802 = 

{
       "OM6802 yuv mode",
	VIM_SNR_YUV,							//yuv mode
	0,	//0x85c bit1  //High priority for 0x806				         // use PCLK of sensor
	0,	//0x85c Bit4-bit2						//[0]: VSync delay enable; [1]: Fifo VSync delay enable; [2]: Use PCLK Negtive Edge.
	0,							//0 -- I2C, 1 -- Serial bus;
	0,							//[0]: 1 -- OV I2C bus, 0 -- Normal sensor
	1,							//I2C Rate : 100KHz
	0x68,							//I2C address
	0x68, 							//ISP I2C address for special sensor
	0x03,							//power config
	0,							//reset sensor  OV Sensor SXGA->VGA hardware reset.

	3,							//brightness 
	3,							//contrast
	
	{0,NULL},														//sensor standby
	{sizeof(gSnrSizeRegValVGA_OM6802) / sizeof(TReg), (PTReg)&gSnrSizeRegValVGA_OM6802},    // Sensor Size List above                                                  //snr initial value	
	
	{sizeof(gSifPwrOnSeqRegVal_OM6802) / sizeof(TReg), (PTReg)&gSifPwrOnSeqRegVal_OM6802},		//Sif config sequence(Reg.800 bit0~2) when sensor power on
	{0, NULL},																								//Sif config sequence(Reg.800 bit0~2) when sensor standby
	{sizeof(gSifRegVal_OM6802) / sizeof(TReg), (PTReg)&gSifRegVal_OM6802},						//sif initial value
																							//isp initial value
	{sizeof(gSensorSizeCfg_OM6802) / sizeof(TSnrSizeCfg), (PTSnrSizeCfg)&gSensorSizeCfg_OM6802},		//size configure 

	SensorSetReg_OM6802,																				//set reg callback
	SensorGetReg_OM6802,																				//get reg callback
	CheckId_OM6802,																				//Sensor ID Check Call
	SensorSetMirrorFlip_OM6802,																									//Set Sensor Mirror Flip Call
	SensorSetContrast_OM6802,																									//Sensor switch size callback
	SensorSetBrightness_OM6802,																									//set et callback
	NULL,	
	SensorSetMode_OM6802,
	NULL,																									//Sensor switch size callback
	NULL,
	{	//ythd of flash,et of flash
		0x20,			10
	},
};

#endif


/////////////////////////////////////////////////////////////
//
//            OV7725(fengyu)
//
/////////////////////////////////////////////////////////////


#if V5_DEF_SNR_OV7725_YUV

/* write sensor register callback */

void SensorSetReg_OV7725(UINT16 uAddr, UINT16 uVal)

{
         V5B_SifI2cWriteByte((UINT8)uAddr, (UINT8)uVal);
}

void SensorGetReg_OV7725(UINT8 uAddr, UINT16 *uVal)
{
	V5B_SifI2cReadByte((UINT8)uAddr, (UINT8*)uVal);
}

UINT8 CheckId_OV7725(void)
{
	 UINT8 temp1;
     UINT8 temp2; 

     V5B_SifI2cReadByte(0x0a, &temp1);
     V5B_SifI2cReadByte(0x0b, &temp2); 

     if((temp1==0x77) && (temp2==0x21))
             return SUCCEED; 
     else 
             return FAILED; 
}

/******************************************************************

         Desc: set sensor mirror and flip callback 

         Para: BIT0 -- mirror

               BIT1 -- flip

*******************************************************************/

void SensorSetMirrorFlip_OV7725(UINT8 val)
{
    UINT8 x;
	val=~val;
	V5B_SifI2cReadByte(0x0c, &x);
	V5B_SifI2cWriteByte(0x0c, (UINT8)(x&0x3f)|((val&0x03)<<6));
}

/******************************************************************

         Desc: set sensor contrast  callback 

         Para: 1-5 step

*******************************************************************/

void SensorSetContrast_OV7725(UINT8 val)
{
    UINT8 x;

	switch(val)
	{
		
	case 1:                 

		V5B_SifI2cReadByte(0xa6, &x);
    	V5B_SifI2cWriteByte(0xa6, (UINT8)(x|0x04));

        V5B_SifI2cWriteByte(0x9c, 0x10);

		break;  

	case 2:                                                    
		
        V5B_SifI2cReadByte(0xa6, &x);
    	V5B_SifI2cWriteByte(0xa6, (UINT8)(x|0x04));

        V5B_SifI2cWriteByte(0x9c, 0x18);

		break;

   	case 3:                                
		
		V5B_SifI2cReadByte(0xa6, &x);
    	V5B_SifI2cWriteByte(0xa6, (UINT8)(x|0x04));

		V5B_SifI2cWriteByte(0x9c, 0x20);

		break; 

    case 4:                                
		
        V5B_SifI2cReadByte(0xa6, &x);
    	V5B_SifI2cWriteByte(0xa6, (UINT8)(x|0x04));

		V5B_SifI2cWriteByte(0x9c, 0x28);

		break;  

	case 5:   
		
		V5B_SifI2cReadByte(0xa6, &x);
    	V5B_SifI2cWriteByte(0xa6, (UINT8)(x|0x04));

		V5B_SifI2cWriteByte(0x9c, 0x30);
		
		break; 
	default:
		break;
	}					
}

/******************************************************************

         Desc: set sensor Brightness  callback 

         Para: 1-5 step

*******************************************************************/

void SensorSetBrightness_OV7725(UINT8 val)
{
	UINT8 x;

	switch(val)
	{
	case 1:

        V5B_SifI2cReadByte(0xab, &x);
    	V5B_SifI2cWriteByte(0xab, (UINT8)(x|0x08));
		
        V5B_SifI2cReadByte(0xa6, &x);
    	V5B_SifI2cWriteByte(0xa6, (UINT8)(x|0x04));

		V5B_SifI2cWriteByte(0x9b, 0x58);

		break;	

	case 2:
	
        V5B_SifI2cReadByte(0xab, &x);
    	V5B_SifI2cWriteByte(0xab, (UINT8)(x|0x08));

        V5B_SifI2cReadByte(0xa6, &x);
    	V5B_SifI2cWriteByte(0xa6, (UINT8)(x|0x04));

		V5B_SifI2cWriteByte(0x9b, 0x38);

		break;

	case 3:

		V5B_SifI2cReadByte(0xab, &x);
    	V5B_SifI2cWriteByte(0xab, (UINT8)(x&0xf7));
		
        V5B_SifI2cReadByte(0xa6, &x);
    	V5B_SifI2cWriteByte(0xa6, (UINT8)(x|0x04));
		
		V5B_SifI2cWriteByte(0x9b, 0x08);

		break;

	case 4:
		
        V5B_SifI2cReadByte(0xab, &x);
    	V5B_SifI2cWriteByte(0xab, (UINT8)(x&0xf7));
		
        V5B_SifI2cReadByte(0xa6, &x);
    	V5B_SifI2cWriteByte(0xa6, (UINT8)(x|0x04));
				
		V5B_SifI2cWriteByte(0x9b, 0x28);


		break;

	case 5:
		
        V5B_SifI2cReadByte(0xab, &x);
    	V5B_SifI2cWriteByte(0xab, (UINT8)(x&0xf7));
		
        V5B_SifI2cReadByte(0xa6, &x);
    	V5B_SifI2cWriteByte(0xa6, (UINT8)(x|0x04));

		V5B_SifI2cWriteByte(0x9b, 0x48);

		break;
	default:
		break;		
	}
}

/******************************************************************

         Desc: set sensor mod  callback 

         Para:  val: 	1	50hz
         	    		2	60hz
         	     	    	3	night 
         	     	     	4	outdoor

*******************************************************************/

void SensorSetMode_OV7725(UINT8 val)
{
	switch (val)
	{	
		case 1:

			V5B_SifI2cWriteByte(0x11, 0x01);
			V5B_SifI2cWriteByte(0x2b, 0x00);
            
			break;

		case 2:
		    
			V5B_SifI2cWriteByte(0x11, 0x01);
			V5B_SifI2cWriteByte(0x2b, 0x9e);

			break;

		case 4:  // 3 and 4 reverse

            V5B_SifI2cWriteByte(0x11, 0x07);
			 			
			break;

		case 3:	

			V5B_SifI2cWriteByte(0x11, 0x01);
					
			break;

		default:
			break;
	}
}

const TReg gSifPwrOnSeqRegVal_OV7725[] =
{
	{0x00, 0x63,	1},
	{0x00, 0x62,	1},
	{0x00, 0x63,	1}
};

const TReg gSifRegVal_OV7725[] =
{        
    {0x02,  0x02,  0},
    {0x03,  0x08,  0},
    {0x04,  0x04,  0},
    {0x2c,  0x02,  0},
    {0x2d,  0x80,  0},
    {0x2e,  0x01,  0},
    {0x2f,  0xe0,  0},
    {0x01,  0x45,  0},
    {0x2c,  0x02,  0},
    {0x2d,  0x80,  0},
    {0x2e,  0x01,  0},
    {0x2f,  0xe0,  0},  
	
};

/* Sensor size control configure info */

const TReg gSnrSizeRegValVGA_OV7725[] = 
{      
   	    {0x12, 0x80, 1},
		{0x3d, 0x03, 1},
		{0x17, 0x22, 1},
		{0x18, 0xa4, 1},
		{0x19, 0x07, 1},
		{0x1a, 0xf0, 1},
		{0x32, 0x00, 1},
		{0x29, 0xa0, 1},
		{0x2c, 0xf0, 1},
		{0x2a, 0x00, 1},
		{0x11, 0x01, 1}, 
		{0x42, 0x7f, 1},
		{0x4d, 0x09, 1},
		{0x63, 0xe0, 1},
		{0x64, 0xff, 1},
		{0x65, 0x20, 1},
		{0x66, 0x00, 1},
		{0x67, 0x48, 1},
		{0x13, 0xf0, 1},
		{0x0d, 0x41, 1}, 
		{0x0f, 0xc5, 1},
		{0x14, 0x11, 1},
		{0x22, 0x3f, 1}, 
		{0x23, 0x07, 1}, 
		{0x24, 0x40, 1},
		{0x25, 0x30, 1},
		{0x26, 0xa1, 1},
		{0x2b, 0x00, 1}, 
		{0x6b, 0xaa, 1},
		{0x13, 0xff, 1},
		{0x90, 0x05, 1},
		{0x91, 0x01, 1},
		{0x92, 0x03, 1},
		{0x93, 0x00, 1},
		{0x94, 0xb0, 1},
		{0x95, 0x9d, 1},
		{0x96, 0x13, 1},
		{0x97, 0x16, 1},
		{0x98, 0x7b, 1},
		{0x99, 0x91, 1},
		{0x9a, 0x1e, 1},
		{0x9b, 0x08, 1},
		{0x9c, 0x20, 1},
		{0x9e, 0x81, 1},
		{0xa6, 0x06, 1},
		{0x7e, 0x0c, 1},
		{0x7f, 0x16, 1},
		{0x80, 0x2a, 1},
		{0x81, 0x4e, 1},
		{0x82, 0x61, 1},
		{0x83, 0x6f, 1},
		{0x84, 0x7b, 1},
		{0x85, 0x86, 1},
		{0x86, 0x8e, 1},
		{0x87, 0x97, 1},
		{0x88, 0xa4, 1},
		{0x89, 0xaf, 1},
		{0x8a, 0xc5, 1},
		{0x8b, 0xd7, 1},
		{0x8c, 0xe8, 1},
		{0x8d, 0x20, 1},
		{0x33, 0x66, 1},
		{0x22, 0x99, 1},
		{0x23, 0x03, 1},
		{0x4a, 0x10, 1},
		{0x49, 0x10, 1},
		{0x4b, 0x14, 1},
		{0x4c, 0x17, 1},
		{0x46, 0x05, 1},
		{0x0e, 0x65, 1}

};

/* Sensor size control configure info */

const TSnrSizeCfg gSensorSizeCfg_OV7725[] = 
{
	{
              {0,NULL},
               {0,NULL},
	       {640, 480},
		1,
		1


	}

};

 

const TSnrInfo gSensorInfo_OV7725 = 

{
       "OV7725 yuv mode",
	VIM_SNR_YUV,							//yuv mode
	1,	//0x85c bit1  //High priority for 0x806				         //not use PCLK of sensor
	0,							//[0]: VSync delay enable; [1]: Fifo VSync delay enable; [2]: Use PCLK Negtive Edge.
	0,							//0 -- I2C, 1 -- Serial bus;
	1,							//[0]: 1 -- OV I2C bus
	80,							//I2C Rate : 100KHz
	0x42,							//I2C address
	0x42,							//ISP I2C address for special sensor
	0x03,							//power config
	0,							//reset sensor

	3,							//brightness 
	3,							//contrast
	
	{0,NULL},														//sensor standby
       {sizeof(gSnrSizeRegValVGA_OV7725) / sizeof(TReg), (PTReg)&gSnrSizeRegValVGA_OV7725},                                                       //snr initial value
	
	{sizeof(gSifPwrOnSeqRegVal_OV7725) / sizeof(TReg), (PTReg)&gSifPwrOnSeqRegVal_OV7725},		//Sif config sequence(Reg.800 bit0~2) when sensor power on
	{0, NULL},																								//Sif config sequence(Reg.800 bit0~2) when sensor standby
	{sizeof(gSifRegVal_OV7725) / sizeof(TReg), (PTReg)&gSifRegVal_OV7725},						//sif initial value
																							//isp initial value
	{sizeof(gSensorSizeCfg_OV7725) / sizeof(TSnrSizeCfg), (PTSnrSizeCfg)&gSensorSizeCfg_OV7725},		//size configure 

	SensorSetReg_OV7725,																				//set reg callback
	SensorGetReg_OV7725,																				//get reg callback
	CheckId_OV7725,																				//Sensor ID Check Call
	SensorSetMirrorFlip_OV7725,																									//Set Sensor Mirror Flip Call
	SensorSetContrast_OV7725,																									//Sensor switch size callback
	SensorSetBrightness_OV7725,																									//set et callback
	NULL,
	SensorSetMode_OV7725,
	NULL,																									//Sensor switch size callback
	NULL,
	{	//ythd of flash,et of flash
		0x20,			10
	},
};

#endif


////////////////////////////////////////////////////////////////////
//
//      SIV100B (fengyu)
//
////////////////////////////////////////////////////////////////////


/******************************************************************
	Add  V5_DEF_SNR_SIV100A_YUV 2007-1-25 
******************************************************************/
#if V5_DEF_SNR_SIV100B_YUV	

void SensorSetReg_SIV100B(UINT16 uAddr, UINT16 uVal)
{
         V5B_SifI2cWriteByte((UINT8)uAddr, (UINT8)uVal);
}

void SensorGetReg_SIV100B(UINT8 uAddr, UINT16 *uVal)
{
	V5B_SifI2cReadByte((UINT8)uAddr, (UINT8*)uVal);
}

UINT8 CheckId_SIV100B(void)
{
	UINT8 temp1;
	UINT8 temp2; 

        V5B_SifI2cReadByte(0x01, &temp1);
	 V5B_SifI2cReadByte(0x02, &temp2);    

        if((temp1==0x0C) && (temp2==0x11))
                return SUCCEED; 
        else 
                return FAILED;  
}

void SensorSetMirrorFlip_SIV100B(UINT8 val)
{
        UINT8 x;
//	val=~val;
	if(val==0x01)
		val=0x2;
	else if(val==0x02)
		val=0x01;
 	V5B_SifI2cReadByte(0x04, &x);
 	x&=0xfc;//clear bit0,bit1
	x |= (val&0x03);
 	V5B_SifI2cWriteByte(0x04, x);
}

/******************************************************************
         Desc: set sensor contrast  callback 
         Para: 1-5 step
*******************************************************************/
void SensorSetContrast_SIV100B(UINT8 val)
{
	switch(val)
	{	
	case 1:    
		V5B_SifI2cWriteByte(0xD4, 0x04);              
		break; 
		
	case 2:    
		V5B_SifI2cWriteByte(0xD4, 0x0A);                                                
		break;

   	case 3:      
   		V5B_SifI2cWriteByte(0xD4, 0x10);                          
		break; 

    case 4:     
        V5B_SifI2cWriteByte(0xD4, 0x18);                           
		break;  

	case 5:     
		V5B_SifI2cWriteByte(0xD4, 0x20);                           
		break; 

	default:
		break;
	}					
}

/******************************************************************
         Desc: set sensor Brightness  callback 
         Para: 1-5 step
*******************************************************************/
void SensorSetBrightness_SIV100B(UINT8 val)
{
	switch(val)
	{
	case 1:
		V5B_SifI2cWriteByte(0xD7, 0x80);
		break;	
	case 2:
		V5B_SifI2cWriteByte(0xD7, 0xC0);
		break;
	case 3:
		V5B_SifI2cWriteByte(0xD7, 0x00);
		break;
	case 4:
		V5B_SifI2cWriteByte(0xD7, 0x40);
		break;
	case 5:
		V5B_SifI2cWriteByte(0xD7, 0x70);
		break;
	default:
		break;		
	}
}

/******************************************************************
         Desc: set sensor mode  callback 
         Para:  val: 	1 50hz,	2 60hz,	3 night, 4 outdoor.
*******************************************************************/
void SensorSetMode_SIV100B(UINT8 val)
{	
	switch (val)
	{	
		case 1:

            V5B_SifI2cWriteByte(0x32, 0x10);
			V5B_SifI2cWriteByte(0x40, 0x82);			
			break;

		case 2:
			
			V5B_SifI2cWriteByte(0x32, 0x10);
			V5B_SifI2cWriteByte(0x40, 0x80);				
			break;

		case 4:  // 3 and 4 reverse

			V5B_SifI2cWriteByte(0x40, 0x00);  // Disable AE
			V5B_SifI2cWriteByte(0x32, 0x40);
			break;

		case 3:	
	
			V5B_SifI2cWriteByte(0x32, 0x10);
			V5B_SifI2cWriteByte(0x40, 0x82);						
			break;

		default:
			break;
	}
}

/******************************************************************

         Desc: get sensor ET  callback 			//add by guoying 12/28/2007
         	then calculate new ET value, set into the registers.
       
         Para:  val: 	*RegBuf					//the data buffer pointer
*******************************************************************/
void SensorGetET_SIV100B(UINT8 *RegBuf)
{
	UINT8 uTemp;
	
	//at first close the AE
	V5B_SifI2cReadByte(0x40, &uTemp);		//dsiable AE
	uTemp&=~BIT7;
	V5B_SifI2cWriteByte(0x40, uTemp);	

	//Get ET register value and store to the buffer
	V5B_SifI2cReadByte(0x30, &uTemp);		
	*RegBuf++=uTemp;
	VIM_USER_PrintDec(" AECdata_old= ",uTemp);
	
	V5B_SifI2cReadByte(0x31, &uTemp);		
	//AECdata|=(UINT16)(uTemp&0xff);
	*RegBuf++=uTemp;
	VIM_USER_PrintDec(" AECdata_old= ",uTemp);


	return;
	
}
/******************************************************************

         Desc: enable sensor ET  callback 		//add by guoying 12/28/2007
       
         Para:  val: 	*RegBuf				//the data buffer pointer
*******************************************************************/
void SensorSetET_SIV100B(UINT8 *RegBuf, UINT8 div)
{
	UINT8 uTemp;
	UINT16 AECdata=0;
	
	//calculate the new AE value
	uTemp=*RegBuf++;
	AECdata|=(UINT16)((uTemp&0xff)<<8);			

	uTemp=*RegBuf++;
	AECdata|=(UINT16)(uTemp&0xff);				

	AECdata=(UINT16)(AECdata/div);

	uTemp=AECdata%100;
	AECdata-=uTemp;						//取100的整数倍!
	
	uTemp=(UINT8)((AECdata>>8)&0xff);				
	V5B_SifI2cWriteByte(0x30, uTemp);	

	uTemp=(UINT8)(AECdata&0xff);				
	V5B_SifI2cWriteByte(0x31, uTemp);	
	
	
	
	return;
	
}


const TReg gSifPwrOnSeqRegVal_SIV100B[] =
{
	{0x00, 0x63,	1},
	{0x00, 0x62,	1},
	{0x00, 0x63,	1}
};



const TReg gSifRegVal_SIV100B[] =
{        
 	  {0x02,  0x00, 0},
	  {0x03,  0x0a, 0},
	  {0x04,  0x05, 0},
   	  {0x01,  0x41, 0} 
};

const TReg gSnrSizeRegValVGA_SIV100B[] = 
{      	 

     // 24Mhz
     {0x04, 0x00, 1},  //0312
	 {0x11, 0x04, 1}, //0x0a#0x09#black sun 2.8V
	 {0x12, 0x0a, 1},
	 {0x13, 0x1f, 1},  //ABS 2.8V
	 {0x16, 0x89, 1},
	 {0x1b, 0x90, 1},
	 {0x1f, 0x52, 1},

     //SIV100B  50Hz - 25.00FPS(100/4) 24MHz

	 {0x23, 0x00, 1},
	 {0x24, 0x2f, 1},
	 {0x25, 0x43, 1},
	 {0x35, 0x8d, 1},

	 {0x33, 0x10, 1},

     //AE 

	 {0x40, 0x82, 1}, 
	 {0x41, 0x88, 1}, //AE target 0x84 0108
	 {0x42, 0x7f, 1}, //Don't change
	 {0x43, 0xc0, 1},
	 {0x44, 0x38, 1},
	 {0x45, 0x28, 1}, //0x20 #Gain changer(Normal)0x2f (r:noise)
	 {0x46, 0x08, 1}, //0x0a #Gain changer(N Down) 0x12->0x08
	 {0x47, 0x15, 1}, 
	 {0x48, 0x1e, 1}, //0x1d #Range 1 0x0d (r:AE hunting)
	 {0x49, 0x13, 1}, //0x0e #Range 2
	 {0x4a, 0x63, 1}, //0x73 #Rgane 3
	 {0x4b, 0x82, 1}, //Shut Rate control 0x89
	 {0x4c, 0x3c, 1},
	 {0x4e, 0x17, 1}, //AEADC Off
	 {0x4f, 0x8a, 1}, //20050302
	 {0x50, 0x94, 1},

	 {0x5a, 0x00, 1},  	//AF Off

     // Auto White Balance  0105

	 {0x60, 0xc8, 1},    //cb 0106
	 {0x61, 0x88, 1},    //0x8e 0105 condition
	 {0x62, 0x01, 1},    //AWB Lange #0x01
	 {0x63, 0x80, 1}, 	 
	 {0x64, 0x80, 1},
	 {0x65, 0xd2, 1},	//R Gain Top
	 {0x66, 0x80, 1},	//R Gain Bottom
	 {0x67, 0xd6, 1},	//B Gain Top
	 {0x68, 0x80, 1},	//B Gain Bottom 0108
	 {0x69, 0x8a, 1},	//0x90	#Cr Top Value
	 {0x6a, 0x73, 1},	//Cr Bottom Value   
	 {0x6b, 0x90, 1},	//Cb Top Value   #0x90 a7
	 {0x6c, 0x70, 1},	//Cb Bottom Value   #0x70
	 {0x6d, 0x88, 1},	//0x84
	 {0x6e, 0x77, 1},	//0x7b
	 {0x6f, 0x44, 1},	//0x84 #AWB URNG 0109
	 {0x70, 0xd8, 1},	//0xd8 0109
	 {0x71, 0x60, 1},	//0x70
	 {0x72, 0x05, 1},
	 {0x73, 0x02, 1},	//0x30
	 {0x74, 0x0c, 1},        //0x07 0x10 0109
	 {0x75, 0x04, 1},	//0x32
	 {0x76, 0x20, 1},	//0x32
	 {0x77, 0xb7, 1},	//0xb0 0109
	 {0x78, 0x95, 1},	//0xb5 0109

      //IDP     

	 {0x80, 0xaf, 1},        //shading on 0105
	 {0x81, 0x0d, 1},
	 {0x83, 0x00, 1},  	//shading on/off
	 {0x86, 0xaa, 1},
	 {0x87, 0x04, 1},        //0x18
	 {0x88, 0x2a, 1},  	//DPC start Gain 0x20
	 {0x89, 0x0f, 1},

	 {0x92, 0x44, 1},	//filter control
	 {0x93, 0x10, 1},	//0111
	 {0x94, 0x30, 1},	//0418
	 {0x95, 0x50, 1},	//0418
	 {0x96, 0x10, 1}, 	//0111
	 {0x97, 0x20, 1},	//0418
	 {0x98, 0x30, 1},	//0418
	 {0x99, 0x28, 1},
	 {0x9a, 0x50, 1},	//0111

     //Shading 0106

	 {0xa4, 0x88, 1},
	 {0xa5, 0x88, 1},
	 {0xa6, 0x88, 1},
	 {0xa7, 0xaa, 1},
	 {0xa8, 0x88, 1},
	 {0xa9, 0x44, 1}, //0x12
	 {0xaa, 0x36, 1},
	 {0xab, 0x11, 1}, //0x00
	 {0xac, 0x13, 1},
	 {0xad, 0x00, 1}, //0x00
	 {0xae, 0x01, 1},
	 {0xaf, 0x98, 1}, //shading x position
	 {0xb0, 0x90, 1},

     //Gamma   

	 {0xb1, 0x00, 1},                      
	 {0xb2, 0x08, 1},                    
	 {0xb3, 0x11, 1},            
	 {0xb4, 0x25, 1},           
	 {0xb5, 0x45, 1},           
	 {0xb6, 0x5f, 1},          
	 {0xb7, 0x74, 1},          
	 {0xb8, 0x87, 1},           
	 {0xb9, 0x97, 1},          
	 {0xba, 0xa5, 1},           
	 {0xbb, 0xb2, 1},          
	 {0xbc, 0xc9, 1},           
	 {0xbd, 0xdd, 1},          
	 {0xbe, 0xf0, 1},          
	 {0xbf, 0xf8, 1},           
	 {0xc0, 0xff, 1},     

     //Color Matrix

	 {0xc1, 0x3d, 1},
	 {0xc2, 0xc6, 1},
	 {0xc3, 0xfd, 1},
	 {0xc4, 0x10, 1},  //0111
	 {0xc5, 0x21, 1},  //0111
	 {0xc6, 0x10, 1},  //0111
	 {0xc7, 0xf3, 1},
	 {0xc8, 0xbd, 1},
	 {0xc9, 0x50, 1},

     //Edge
	 {0xca, 0x90, 1},
	 {0xcb, 0x18, 1},  //0111 
	 {0xcc, 0x20, 1},  //0111 
	 {0xcd, 0x06, 1},  //0111 
	 {0xce, 0x06, 1},  //0111 
	 {0xcf, 0x10, 1},
	 {0xd0, 0x20, 1},
	 {0xd1, 0x2a, 1}, //Edge start gain	
	 {0xd2, 0x86, 1},
	 {0xd3, 0x00, 1},


      //Contrast

	 {0xd4, 0x10, 1}, //Contrast 0x14
	 {0xd5, 0x14, 1}, //color gain
	 {0xd6, 0x14, 1}, //color gain
	 {0xd7, 0x00, 1}, //Y Setup (e:down)
	 {0xd8, 0x00, 1},
	 {0xd9, 0x00, 1},
	 {0xda, 0x00, 1},
	 {0xdb, 0xff, 1},
	 {0xdc, 0x00, 1},

     //Saturation

	 {0xe1, 0x2a, 1}, //color suppress Start Gain 0x30
	 {0xe2, 0x2b, 1}, //color suppress slpoe

	 {0x40, 0x82, 1}, //AEBLC OFF
	 {0x03, 0x05, 1},

};



const TReg gSizeRegSnrVal_SIV100B[] = 		 
{ 
									
              {0x40, 0x82, 1}						//guoying add for AE enable, 12/28/2007
        
};


const TSnrSizeCfg gSensorSizeCfg_SIV100B[] = 
{
	{	//if VIM_USER_MCLK_45M=0, pls use tag_TRegGroup setting
               {sizeof(gSizeRegSnrVal_SIV100B) / sizeof(TReg), (PTReg)&gSizeRegSnrVal_SIV100B},	//12/28/2007 add by guoying 
              
              {0,NULL},
	      	{640, 480},
		
		1,								//if not use PCLK of sensor, must set these parameters are 1, 1	//guoying 2/14/2008
		1


	}
};

const TSnrInfo gSensorInfo_SIV100B = 
{
       "SIV100B yuv mode",
	///1,							//yuv mode
	VIM_SNR_YUV,				//yuv mode
	0,							//not use PCLK of sensor
	0,							//[0]: VSync delay enable; [1]: Fifo VSync delay enable; [2]: Use PCLK Negtive Edge.
	0,							//0 -- I2C, 1 -- Serial bus;
	0,							//[0]: 1 -- OV I2C bus
	100,							//I2C Rate : 100KHz
	0x67,						//I2C address
	0x67,						//ISP I2C address for special sensor
	0x07,						//power config
	0,							//reset sensor
	3,							//brightness 
	3,							//contrast

	{0,NULL},					//sensor standby                                                          
       {sizeof(gSnrSizeRegValVGA_SIV100B) / sizeof(TReg), (PTReg)&gSnrSizeRegValVGA_SIV100B},                                                       //snr initial value
	{sizeof(gSifPwrOnSeqRegVal_SIV100B) / sizeof(TReg), (PTReg)&gSifPwrOnSeqRegVal_SIV100B},	//Sif config sequence(Reg.800 bit0~2) when sensor power on                
	{0, NULL},											//Sif config sequence(Reg.800 bit0~2) when sensor standby             
	{sizeof(gSifRegVal_SIV100B) / sizeof(TReg), (PTReg)&gSifRegVal_SIV100B},			//sif initial value                                                       
	{sizeof(gSensorSizeCfg_SIV100B) / sizeof(TSnrSizeCfg), (PTSnrSizeCfg)&gSensorSizeCfg_SIV100B},	//size configure                                                          

	SensorSetReg_SIV100B,																				
	SensorGetReg_SIV100B,																				
	CheckId_SIV100B,																				
	SensorSetMirrorFlip_SIV100B,																			
	SensorSetContrast_SIV100B,																									//Sensor switch size callback
	SensorSetBrightness_SIV100B,																									//set et callback
	NULL,
	SensorSetMode_SIV100B,
	
	NULL,									//if VIM_USER_MCLK_45M=1, pls set NULL
       NULL,

	{	//ythd of flash,et of flash
		0x20,			10
	},
};

#endif



/////////////////////////////////////////////////////////////
//
//            ET8EC3(fengyu)
//
/////////////////////////////////////////////////////////////


#if V5_DEF_SNR_ET8EC3_YUV

/* write sensor register callback */

void SensorSetReg_ET8EC3(UINT16 uAddr, UINT16 uVal)

{
         V5B_SifI2cWriteByte((UINT8)uAddr, (UINT8)uVal);
}

void SensorGetReg_ET8EC3(UINT8 uAddr, UINT16 *uVal)
{
	V5B_SifI2cReadByte((UINT8)uAddr, (UINT8*)uVal);
}

UINT8 CheckId_ET8EC3(void)
{
	 
	
	 UINT8 temp1;
     UINT8 temp2;
	 UINT8 temp3;


     V5B_SifI2cReadByte(0x01, &temp1);
	 V5B_SifI2cReadByte(0x7f, &temp2);
     V5B_SifI2cReadByte(0x00, &temp3);
    

     if((temp1=0x1f)&&(temp2=0x18)&&(temp3=0x00)) 
             return SUCCEED; 
     else 
             return FAILED; 
}

/******************************************************************

         Desc: set sensor mirror and flip callback 

         Para: BIT0 -- mirror

               BIT1 -- flip

*******************************************************************/

void SensorSetMirrorFlip_ET8EC3(UINT8 val)
{
    
	UINT8 x;
	val=~val;
	V5B_SifI2cReadByte(0x04, &x);
	V5B_SifI2cWriteByte(0x04, (UINT8)(x&0x3f)|((val&0x03)<<6));
	
}

/******************************************************************

         Desc: set sensor contrast  callback 

         Para: 1-5 step

*******************************************************************/

void SensorSetContrast_ET8EC3(UINT8 val)
{
	switch(val)
	{
		
	case 1:                 

        V5B_SifI2cWriteByte(0x11, 0x60);

		break;  

	case 2:                                                    
	
        V5B_SifI2cWriteByte(0x11, 0x80);

		break;

   	case 3:                                
		
		V5B_SifI2cWriteByte(0x11, 0xA0);

		break; 

    case 4:                                
		
		V5B_SifI2cWriteByte(0x11, 0xC0);

		break;  

	case 5:   
		
		V5B_SifI2cWriteByte(0x11, 0xE0);
		
		break; 
	default:
		break;
	}

}

/******************************************************************

         Desc: set sensor Brightness  callback 

         Para: 1-5 step

*******************************************************************/

void SensorSetBrightness_ET8EC3(UINT8 val)
{

	switch(val)
	{
	case 1:

		V5B_SifI2cWriteByte(0x12, 0xA0);

		break;	

	case 2:
	
		V5B_SifI2cWriteByte(0x12, 0xC0);

		break;

	case 3:

		V5B_SifI2cWriteByte(0x12, 0x00);

		break;

	case 4:
		
		V5B_SifI2cWriteByte(0x12, 0x20);

		break;

	case 5:
		
		V5B_SifI2cWriteByte(0x12, 0x40);

		break;
	default:
		break;		
	}
	
}

/******************************************************************

         Desc: set sensor mod  callback 

         Para:  val: 	1	50hz
         	    		2	60hz
         	     	    	3	night 
         	     	     	4	outdoor

*******************************************************************/

void SensorSetMode_ET8EC3(UINT8 val)
{

	switch (val)
	{	
		case 1:

			V5B_SifI2cWriteByte(0x02, 0x00);
			V5B_SifI2cWriteByte(0x04, 0x1f);
            
			break;

		case 2:
		    
			V5B_SifI2cWriteByte(0x02, 0x40);
			V5B_SifI2cWriteByte(0x04, 0x1f);

			break;

		case 4:  // 3 and 4 reverse

            V5B_SifI2cWriteByte(0x04, 0x2f);
			 			
			break;

		case 3:	

			V5B_SifI2cWriteByte(0x02, 0x00);
			V5B_SifI2cWriteByte(0x04, 0x1f);
					
			break;

		default:
			break;
	}

}

const TReg gSifPwrOnSeqRegVal_ET8EC3[] =
{	
	{0x00, 0x63,	3},
	{0x00, 0x62,	3},
	{0x00, 0x63,	3}
};

const TReg gSifRegVal_ET8EC3[] =
{        
    {0x02,  0x00,  0},  //YCbCr Order
    {0x03,  0x08,  0},
    {0x04,  0x04,  0},
    {0x2c,  0x02,  0},
    {0x2d,  0x80,  0},
    {0x2e,  0x01,  0},
    {0x2f,  0xe0,  0},
    {0x01,  0x45,  0},
    {0x2c,  0x02,  0},
    {0x2d,  0x80,  0},
    {0x2e,  0x01,  0},
    {0x2f,  0xe0,  0},  
	
};

/* Sensor size control configure info */

const TReg gSnrSizeRegValVGA_ET8EC3[] = 
{      

	{0x00, 0x70, 1},
	{0x01, 0x0f, 1},
	{0x02, 0x00, 1},
	{0x03, 0x00, 1},
	{0x04, 0x1f, 1},
	{0x05, 0x00, 1},
	{0x06, 0x0d, 1},
	{0x07, 0xc0, 1},
	{0x08, 0x08, 1},
	{0x09, 0x44, 1},
	{0x0a, 0x00, 1}, 
	{0x0b, 0x40, 1},
	{0x0c, 0x40, 1},
	{0x0d, 0x00, 1},
	{0x0e, 0x2f, 1},
	{0x0f, 0x04, 1},
	{0x10, 0x22, 1},
	{0x11, 0x9a, 1},
	{0x12, 0x08, 1},
	{0x13, 0x08, 1}, 
	{0x14, 0x08, 1},
	{0x15, 0x36, 1},
	{0x16, 0x3a, 1}, 
	{0x17, 0x01, 1}, 
	{0x18, 0x2a, 1},
	{0x19, 0x85, 1},
	{0x1a, 0x20, 1},
	{0x1b, 0x22, 1}, 
	{0x1c, 0x52, 1},
	{0x1d, 0x44, 1},
	{0x1e, 0x38, 1},
	{0x1f, 0x00, 1},
	{0x20, 0x00, 1},
	{0x21, 0x01, 1},
	{0x22, 0x27, 1},
	{0x23, 0x40, 1},
	{0x24, 0x27, 1},
	{0x25, 0x00, 1},
	{0x26, 0x00, 1},
	{0x27, 0x00, 1},
	{0x28, 0x23, 1},
	{0x29, 0x03, 1},
	{0x2a, 0x44, 1},
	{0x2b, 0xb0, 1},
	{0x2c, 0x80, 1},
	{0x2d, 0x40, 1},
	{0x2e, 0x00, 1},
	{0x2f, 0x00, 1},
	{0x30, 0x00, 1},
	{0x31, 0x00, 1},
	{0x32, 0x00, 1},
	{0x33, 0x00, 1},
	{0x34, 0x00, 1},
	{0x35, 0x00, 1},
	{0x36, 0x00, 1},
	{0x37, 0x00, 1},
	{0x38, 0x00, 1},
	{0x39, 0x8c, 1},
	{0x3a, 0xcf, 1},
	{0x3b, 0x80, 1},
	{0x3c, 0x00, 1},
	{0x3d, 0x17, 1},
	{0x3e, 0x05, 1},
	{0x3f, 0x9c, 1},
	{0x40, 0xa0, 1},
	{0x41, 0x00, 1},
	{0x42, 0x00, 1},
	{0x43, 0x00, 1},
	{0x44, 0x06, 1},
	{0x45, 0x17, 1},

    {0x46, 0x00, 1},
	{0x47, 0x20, 1},
	{0x48, 0x39, 1},
	{0x49, 0x18, 1},
	{0x4a, 0x00, 1},
	{0x4b, 0x50, 1},
	{0x4c, 0x0c, 1},
	{0x4d, 0xe0, 1},
	{0x4e, 0x20, 1},
	{0x4f, 0x89, 1},
	{0x50, 0x07, 1},
	{0x51, 0x2f, 1},
	{0x52, 0x02, 1},
	{0x53, 0x00, 1},
	{0x54, 0x30, 1},
	{0x55, 0x50, 1},

	{0x56, 0x40, 1},
	{0x57, 0x06, 1},
	{0x58, 0x02, 1},
	{0x59, 0x23, 1},
	{0x5a, 0x08, 1},
	{0x5b, 0x04, 1}

};

/* Sensor size control configure info */

const TSnrSizeCfg gSensorSizeCfg_ET8EC3[] = 
{
	{
              {0,NULL},
               {0,NULL},
	       {640, 480},
		1,
		1

	}

};

 

const TSnrInfo gSensorInfo_ET8EC3 = 

{
       "ET8EC3 yuv mode",
	VIM_SNR_YUV,							//yuv mode
	0,	//0x85c bit1  //High priority for 0x806				         //not use PCLK of sensor
	0,							//[0]: VSync delay enable; [1]: Fifo VSync delay enable; [2]: Use PCLK Negtive Edge.
	0,							//0 -- I2C, 1 -- Serial bus;
	0,							//[0]: 1 -- OV I2C bus
	80,							//I2C Rate : 100KHz
	0x78,							//I2C address
	0x78,							//ISP I2C address for special sensor
	0x03,							//power config
	0,							//reset sensor

	3,							//brightness 
	3,							//contrast
	
	{0,NULL},														//sensor standby
       {sizeof(gSnrSizeRegValVGA_ET8EC3) / sizeof(TReg), (PTReg)&gSnrSizeRegValVGA_ET8EC3},                                                       //snr initial value
	
	{sizeof(gSifPwrOnSeqRegVal_ET8EC3) / sizeof(TReg), (PTReg)&gSifPwrOnSeqRegVal_ET8EC3},		//Sif config sequence(Reg.800 bit0~2) when sensor power on
	{0, NULL},																								//Sif config sequence(Reg.800 bit0~2) when sensor standby
	{sizeof(gSifRegVal_ET8EC3) / sizeof(TReg), (PTReg)&gSifRegVal_ET8EC3},						//sif initial value
																							//isp initial value
	{sizeof(gSensorSizeCfg_ET8EC3) / sizeof(TSnrSizeCfg), (PTSnrSizeCfg)&gSensorSizeCfg_ET8EC3},		//size configure 

	SensorSetReg_ET8EC3,																				//set reg callback
	SensorGetReg_ET8EC3,																				//get reg callback
	CheckId_ET8EC3,																				//Sensor ID Check Call
	SensorSetMirrorFlip_ET8EC3,																									//Set Sensor Mirror Flip Call
	SensorSetContrast_ET8EC3,																									//Sensor switch size callback
	SensorSetBrightness_ET8EC3,																									//set et callback
	NULL,
	SensorSetMode_ET8EC3,
	NULL,																									//Sensor switch size callback
	NULL,
	{	//ythd of flash,et of flash
		0x20,			10
	},
};

#endif

/////////////////////////////////////////////////////////////////////////////////
//
//     BF3403 
//
/////////////////////////////////////////////////////////////////////////////////


#if V5_DEF_SNR_BF3403_YUV    

/* write sensor register callback */

void SensorSetReg_BF3403(UINT16 uAddr, UINT16 uVal)

{
         V5B_SifI2cWriteByte((UINT8)uAddr, (UINT8)uVal); 
}


void SensorGetReg_BF3403(UINT8 uAddr, UINT16 *uVal) 
{  
	V5B_SifI2cReadByte((UINT8)uAddr, (UINT8*)uVal);
}


UINT8 CheckId_BF3403(void)
{
         UINT8 temp1;
         UINT8 temp2; 
         V5B_SifI2cReadByte(0xfc, &temp1);  //0xfc  0x34
         V5B_SifI2cReadByte(0xfd, &temp2);  //0xfd  0x03


         if((temp1==0x34) && (temp2==0x03))
                 return SUCCEED; 
         else 
                 return FAILED;
}

/******************************************************************

         Desc: set sensor mirror and flip callback 

         Para: BIT0 -- mirror

                     BIT1 -- flip

*******************************************************************/

void SensorSetMirrorFlip_BF3403(UINT8 val) 
{
        UINT8 x;
        val=~val;
        V5B_SifI2cReadByte(0x1e, &x);
        V5B_SifI2cWriteByte(0x1e, (UINT8)(x&0xcf)|((val&0x03)<<4)); 
}

/******************************************************************

         Desc: set sensor contrast  callback 

         Para: 1-5 step

*******************************************************************/

void SensorSetContrast_BF3403(UINT8 val) 
{
 
	switch(val)
	{ 
 
	case 1:                 
         V5B_SifI2cWriteByte(0x56, 0x38);
         break;  

    case 2:                                                    
  
		V5B_SifI2cWriteByte(0x56, 0x40);  //Default
        break;

    case 3:                                
        V5B_SifI2cWriteByte(0x56, 0x48);
        break; 

    case 4:  
		
        V5B_SifI2cWriteByte(0x56, 0x50);
        break;  

     case 5: 
		 
        V5B_SifI2cWriteByte(0x56, 0x55);
        break;

     default:
        break;
	}     
}

/******************************************************************

         Desc: set sensor Brightness  callback 

         Para: 1-5 step

*******************************************************************/

void SensorSetBrightness_BF3403(UINT8 val) 
{
     switch(val)
	 {
       case 1:
            V5B_SifI2cWriteByte(0x55, 0x90);
            break;
 
	   case 2:
            V5B_SifI2cWriteByte(0x55, 0x88);
            break;

       case 3:
            V5B_SifI2cWriteByte(0x55, 0x00);   //default
            break;
 
	   case 4:
            V5B_SifI2cWriteByte(0x55, 0x08);
            break;

       case 5:
            V5B_SifI2cWriteByte(0x55, 0x10);
            break;

       default:
            break;
	 }
}

/******************************************************************

         Desc: set sensor mod  callback 

         Para:  val:  1 50hz
                2 60hz
                     3 night 
                      4 outdoor

*******************************************************************/

void SensorSetMode_BF3403(UINT8 val)
{
 
	  switch (val)
	  { 
        case 1:   // 24MHz
          V5B_SifI2cWriteByte(0xf0, 0x01);  
          V5B_SifI2cWriteByte(0x3b, 0x08);
          V5B_SifI2cWriteByte(0x92, 0x66); 
          V5B_SifI2cWriteByte(0x8e, 0x04);
          V5B_SifI2cWriteByte(0x8f, 0x00);
          V5B_SifI2cWriteByte(0x80, 0x01); 
          V5B_SifI2cWriteByte(0xf0, 0x00);
          break;

        case 2:
          V5B_SifI2cWriteByte(0xf0, 0x01);  
          V5B_SifI2cWriteByte(0x3b, 0xe2);
          V5B_SifI2cWriteByte(0x92, 0x00); 
          V5B_SifI2cWriteByte(0x8e, 0x04);
          V5B_SifI2cWriteByte(0x8f, 0x00);
          V5B_SifI2cWriteByte(0x80, 0x00); 
          V5B_SifI2cWriteByte(0xf0, 0x00);
          break;

         case 4:  // 3 and 4 reverse
           V5B_SifI2cWriteByte(0xf0, 0x01);  
           V5B_SifI2cWriteByte(0x3b, 0xe2);
           V5B_SifI2cWriteByte(0x8e, 0x0f);
           V5B_SifI2cWriteByte(0x8f, 0xa0);
           V5B_SifI2cWriteByte(0xf0, 0x00);
           break;

         case 3:  //outdoor
           V5B_SifI2cWriteByte(0xf0, 0x01);  
           V5B_SifI2cWriteByte(0x3b, 0xe2);
           V5B_SifI2cWriteByte(0x8e, 0x04);
           V5B_SifI2cWriteByte(0x8f, 0x00);
           V5B_SifI2cWriteByte(0xf0, 0x00);
           break;

         default:
           break;
	  }
}

const TReg gSifPwrOnSeqRegVal_BF3403[] = 
{  
	{0x00, 0x63, 1},  
	{0x00, 0x62, 1},  
	{0x00, 0x63, 1} 
};

const TReg gSifRegVal_BF3403[] =
{        
    {0x01,  0x01,  0},
	{0x02,  0x02,  0},
	{0x03,  0x0a,  0},
	{0x04,  0x04,  0},
};


/* Sensor size control configure info */ //BYD BF3403 Settings

const TReg gSnrSizeRegValVGA_BF3403[] =   //BF3403
{
	{0xf0, 0x01, 10},
	{0x16, 0x00, 1},
	{0x1e, 0x00, 1},   //0X30   rotation 180
	{0x26, 0xc8, 1},
	{0x27, 0xc8, 1},
	{0x1f, 0x40, 1},
	{0x22, 0x40, 1},
	{0x29, 0x04, 1},
	{0x2f, 0x01, 1},
	{0x6b, 0x20, 1},
	{0x81, 0x02, 1},
	{0xa1, 0x80, 1},
	{0x51, 0x2f, 1},
	{0x52, 0x93, 1},
	{0x53, 0x04, 1},
	{0x54, 0x82, 1},
    {0x57, 0x2f, 1},
	{0x58, 0x8d, 1},
    {0x59, 0x02, 1},
    {0x5a, 0x92, 1},
    {0x5b, 0x30, 1},

	{0xb1, 0xee, 1},
	{0xb2, 0xee, 1},
	{0x3f, 0xa0, 1},
	{0x39, 0x80, 1},
	{0x40, 0x28, 1},
	{0x41, 0x28, 1},
	{0x42, 0x30, 1},
	{0x43, 0x29, 1},
	{0x44, 0x23, 1},
	{0x45, 0x1b, 1},
	{0x46, 0x17, 1},
	{0x47, 0x0f, 1},
	{0x48, 0x0c, 1},
	{0x49, 0x0a, 1},
	{0x4b, 0x07, 1},
	{0x4c, 0x07, 1},
	{0x4e, 0x06, 1},
	{0x4f, 0x06, 1},
	{0x50, 0x06, 1},

	{0x11, 0x80, 1},
	{0x92, 0x66, 1},
	{0x3b, 0xe2, 1},
	{0x8e, 0x04, 1},
	{0x8f, 0x00, 1},
	{0x80, 0x01, 1},
	{0x9d, 0x99, 1},   //  banding filiter step for 50hz
 // {0x9e, 0x7f, 1},   //  banding filiter step for 60hz
	{0x88, 0x02, 1},
	{0x89, 0xaa, 1},
	{0xf0, 0x00, 1},
};

/* Sensor size control configure info */

const TSnrSizeCfg gSensorSizeCfg_BF3403[] = 
{  
	{
	     {0,NULL},
	     {0,NULL},
	     {640, 480},
		1,
		1


	}

};

 
const TSnrInfo gSensorInfo_BF3403 = 

{
       "BF3403 yuv mode",
        VIM_SNR_YUV,       //yuv mode
        1, //0x85c bit1  //High priority for 0x806             //not use PCLK of sensor   modi by brave
        0,       //[0]: VSync delay enable; [1]: Fifo VSync delay enable; [2]: Use PCLK Negtive Edge.
        0,       //0 -- I2C, 1 -- Serial bus;
        1,       //[0]: 1 -- OV I2C bus
        80,       //I2C Rate : 100KHz
        0xdc,       //I2C address
        0xdc,       //ISP I2C address for special sensor
        0x03,       //power config
        0,       //reset sensor

        3,       //brightness 
        3,       //contrast
 
	   {0,NULL},              //sensor standby
       {sizeof(gSnrSizeRegValVGA_BF3403) / sizeof(TReg), (PTReg)&gSnrSizeRegValVGA_BF3403},  //snr initial value
	   {sizeof(gSifPwrOnSeqRegVal_BF3403) / sizeof(TReg), (PTReg)&gSifPwrOnSeqRegVal_BF3403},  //Sif config sequence(Reg.800 bit0~2) when sensor power on
	   {0, NULL},                        //Sif config sequence(Reg.800 bit0~2) when sensor standby
	   {sizeof(gSifRegVal_BF3403) / sizeof(TReg), (PTReg)&gSifRegVal_BF3403},      //sif initial value
                       //isp initial value
	   {sizeof(gSensorSizeCfg_BF3403) / sizeof(TSnrSizeCfg), (PTSnrSizeCfg)&gSensorSizeCfg_BF3403},  //size configure 

       SensorSetReg_BF3403,                    //set reg callback
       SensorGetReg_BF3403,                    //get reg callback
       CheckId_BF3403,                    //Sensor ID Check Call
       SensorSetMirrorFlip_BF3403,                         //Set Sensor Mirror Flip Call
       SensorSetContrast_BF3403,                         //Sensor switch size callback
       SensorSetBrightness_BF3403,                         //set et callback
	NULL,
       SensorSetMode_BF3403,
       NULL,                         //Sensor switch size callback
       NULL,
	   { //ythd of flash,et of flash
          0x20,   10
	   },
};

#endif


#if V5_DEF_SNR_TEST_YUV

/* write sensor register callback */

void SensorSetReg_Test(UINT16 uAddr, UINT16 uVal)

{
     	VIM_SIF_I2cWriteWord(uAddr,uVal);
}

void SensorGetReg_Test(UINT8 uAddr, UINT16 *uVal)
{
	///V5B_SifI2cReadByte((UINT8)uAddr, (UINT8*)uVal);
	V5B_SifI2cReadByte((UINT8)uAddr, (UINT8*)uVal);
}

UINT8 CheckId_Test(void)
{


   //              return SUCCEED; 
   			return FAILED;

}

/******************************************************************

         Desc: set sensor mirror and flip callback 

         Para: BIT0 -- mirror

                     BIT1 -- flip

*******************************************************************/

void SensorSetMirrorFlip_Test(UINT8 val)
{
}

/******************************************************************

         Desc: set sensor contrast  callback 

         Para: 1-5 step

*******************************************************************/

void SensorSetContrast_Test(UINT8 val)
{
}

/******************************************************************

         Desc: set sensor Brightness  callback 

         Para: 1-5 step

*******************************************************************/

void SensorSetBrightness_Test(UINT8 val)
{
	
}

/******************************************************************

         Desc: set sensor mod  callback 

         Para:  val: 	1	50hz
         	    		2	60hz
         	     	    	3	night 
         	     	     	4	outdoor

*******************************************************************/

void SensorSetMode_Test(UINT8 val)
{
	
}

/******************************************************************

         Desc: get sensor ET  callback 			//add by guoying 9/6/2007
       
         Para:  val: 	*RegBuf					//the data buffer pointer
*******************************************************************/
void SensorGetET_Test(UINT8 *RegBuf)
{
	
	
}
/******************************************************************

         Desc: set sensor ET  callback 		//add by guoying 9/6/2007
       
         Para:  val: 	*RegBuf				//the data buffer pointer
*******************************************************************/
void SensorSetET_Test(UINT8 *RegBuf, UINT8 div)
{

	
	
}

const TReg gSifPwrOnSeqRegVal_Test[] =
{
#if 1
	{0x00, 0x63,	1},//2 //2pwn,//1 //0 rst
	{0x00, 0x62,	1},
	{0x00, 0x63,	1}
#else
	{0x00, 0x63,	1},
	{0x00, 0x62,	1},
	{0x00, 0x62,	1}
#endif
};

const TReg gSifRegVal_Test[] =
{        
	{0x00, 0xe3, 1},
        {0x02, 0xb0, 1},
        {0x04, 0x1d, 1},      

      /*  {0x02, 0x02, 0},
        {0x03, 0x1b, 0},
        {0x04, 0x05, 0},
        {0x21, 0x0a, 0},
        {0x23, 0xea, 0},
        {0x2c, 0x02, 0},
        {0x2d, 0x80, 0},
        {0x2e, 0x01, 0},
        {0x2f, 0xe0, 0},
        {0x01, 0x45, 0} */
};


/* Sensor size control configure info */

const TReg gSnrSizeRegValVGA_Test[] = 
{      
   	     {0x01, 0x0300, 2},
        {0x01, 0x158c, 2},
        {0x04, 0x0000, 2},
		{0x04, 0x01f0, 2},
        {0x01, 0x0a08, 2},
        {0x01, 0x0b13, 2},
        {0x01, 0x1000, 2},
        {0x01, 0x1180, 2},
        {0x01, 0x124e, 2},
        {0x01, 0x1607, 2},
        {0x01, 0x30d0, 2} 
      /*  {0x0103, 0x03, 2},
        {0x01, 0x158c, 2},
        {0x04, 0x0000, 2},
		{0x04, 0x01f0, 2},
        {0x01, 0x0a08, 2},
        {0x01, 0x0b13, 2},
        {0x01, 0x1000, 2},
        {0x01, 0x1180, 2},
        {0x01, 0x124e, 2},
        {0x01, 0x1607, 2},
        {0x01, 0x30d0, 2}*/

};


/* Sensor size control configure info */

const TSnrSizeCfg gSensorSizeCfg_Test[] = 
{
	{
		
               {0,NULL},//{sizeof(gSizeRegSnrVal_OV7670) / sizeof(TReg), (PTReg)&gSizeRegSnrVal_OV7670},	//9/7/2007 add by guoying 
               {0,NULL},
	       {640, 480},

		1,
		1


	}

};

 

const TSnrInfo gSensorInfo_Test = 

{
       "TEST yuv mode",
	VIM_SNR_YUV,							//yuv mode
	1,	//0x85c bit1  //High priority for 0x806				         //not use PCLK of sensor
	//0,							//guoying 1/23/2008, can't use this parameter, must use sensor pclk!
	0,							//[0]: VSync delay enable; [1]: Fifo VSync delay enable; [2]: Use PCLK Negtive Edge.
	0,							//0 -- I2C, 1 -- Serial bus;
	1,							//[0]: 1 -- OV I2C bus
	80,							//I2C Rate : 100KHz
	0x88,//0x88,							//I2C address
	0x89,							//ISP I2C address for special sensor
	0x03,							//power config
	0,							//reset sensor

	3,							//brightness 
	3,							//contrast
	
	{0,NULL},														//sensor standby
       {sizeof(gSnrSizeRegValVGA_Test) / sizeof(TReg), (PTReg)&gSnrSizeRegValVGA_Test},                                                       //snr initial value
	
	{sizeof(gSifPwrOnSeqRegVal_Test) / sizeof(TReg), (PTReg)&gSifPwrOnSeqRegVal_Test},		//Sif config sequence(Reg.800 bit0~2) when sensor power on
	{0, NULL},																								//Sif config sequence(Reg.800 bit0~2) when sensor standby
	{sizeof(gSifRegVal_Test) / sizeof(TReg), (PTReg)&gSifRegVal_Test},						//sif initial value
																							//isp initial value
	{sizeof(gSensorSizeCfg_Test) / sizeof(TSnrSizeCfg), (PTSnrSizeCfg)&gSensorSizeCfg_Test},		//size configure 

	SensorSetReg_Test,																				//set reg callback
	SensorGetReg_Test,																				//get reg callback
	CheckId_Test,																				//Sensor ID Check Call
	SensorSetMirrorFlip_Test,																									//Set Sensor Mirror Flip Call
	SensorSetContrast_Test,																									//Sensor switch size callback
	SensorSetBrightness_Test,																									//set et callback
	NULL,	//set et callback
	SensorSetMode_Test,

	NULL,
	NULL,
	
	
	{	//ythd of flash,et of flash
		0x20,			10
	},
};

#endif


#if V5_DEF_SNR_FUJITSU_CCIR656			//FUJITSU TV chip testing	//guoying add
/* write sensor register callback */

void SensorSetReg_FUJITSU(UINT16 uAddr, UINT16 uVal)

{
         //V5B_SifI2cWriteByte((UINT8)uAddr, (UINT8)uVal);
}

void SensorGetReg_FUJITSU(UINT8 uAddr, UINT16 *uVal)
{
	//V5B_SifI2cReadByte((UINT8)uAddr, (UINT8*)uVal);
	 *uVal=0;
}

UINT8 CheckId_FUJITSU(void)
{

         //return SUCCEED; 		//if use FUJITSU TV chip to test, return SUCCEED
		return FAILED;		//else return FAILED
}

/******************************************************************

         Desc: set sensor mirror and flip callback 

         Para: BIT0 -- mirror

                     BIT1 -- flip

*******************************************************************/

void SensorSetMirrorFlip_FUJITSU(UINT8 val)
{

}

/******************************************************************

         Desc: set sensor contrast  callback 

         Para: 1-5 step

*******************************************************************/

void SensorSetContrast_FUJITSU(UINT8 val)
{
					
}

/******************************************************************

         Desc: set sensor Brightness  callback 

         Para: 1-5 step

*******************************************************************/

void SensorSetBrightness_FUJITSU(UINT8 val)
{

}

/******************************************************************

         Desc: set sensor mod  callback 

         Para:  val: 	1	50hz
         	    		2	60hz
         	     	    	3	night 
         	     	     	4	outdoor

*******************************************************************/

void SensorSetMode_FUJITSU(UINT8 val)
{

}

const TReg gSifPwrOnSeqRegVal_FUJITSU[] =
{

	{0x00, 0xe3,	1},		//ccir 656
	{0x00, 0xe2,	1},
	{0x00, 0xe3,	1}

};


const TReg gSifRegVal_FUJITSU[] =
 
{        
        //{0x02, 0x02, 0},		//normal setting
        {0x02, 0xb3, 0},		//ccir656 setting
/*//0x02 reg
[7:6] tref_lh_blk: Timing reference value for the head of blanking line in CCIR656, tref_lh_blk;
[5:4] tref_lt_blk: Timing reference value for the tail of blanking line in CCIR656, tref_lt_blk;
[3:2] Internal delay count (sif_clk) when handle YUV data
[1:0]: the YUV data format from sensor
    00: sensor outputs 'UYVY';
    01: sensor outputs 'VYUY';
    10: sensor outputs 'YUYV';
    11: sensor outputs 'YVYU';
 
*/


        {0x03, 0x1b, 0},
 /*//0x03 reg
 [5]: Vref_full, internal verf is high constantly because sensor's Hsync is constant level between rows.
 0: when sensor's Hsync has level change between rows.
 1: when sensor's Hsync has not any level change between rows,
[4]: vd_mode - vertical sync signal for sensor width selection
 0: vsync send to sensor more than 1 row
1: vsync send to sensor less than 1 row
[3]: vsync_mode - vertical sync signal for isp width selection
 0 : vsync send to isp more than 1 row;
1: vsync send to isp less than 1 row
[2]: v_bypass - input vertical signal bypass selection.
 0 : SIF generate vertical signal for ISP.
1: the input vertical signal is bypass to ISP
[1]: v_polarity - input vertical signal polarity flag
1: input vertical signal is active high;
0: input vertical signal is active low;
[0]: reserved
 
*/


        //{0x04, 0x05, 0},		//normal setting
        {0x04, 0x1d, 0},			//ccir656 setting
/*//0x04 reg
[7:6] tref_lh_vld: Timing reference value for the head of valid line in CCIR656, tref_lh_vld;
[5:4] tref_lt_vld: Timing reference value for the tail of valid line in CCIR656, tref_lt_vld;
[3]: h_bypass - input horizontal signal bypass selection. If reg85c [1] = 1, using sensor's pclk as sample clock, h_bypass must be "0". 
 0: SIF generate horizontal signal to ISP;
1: the input horizontal signal is bypass to ISP;
[2]: h_polarity - input horizontal signal polarity flag,
0: input horizontal signal is active low;
1: input horizontal signal is active high;
[1]: h_flag - input horizontal signal flag,
0: input horizontal signal is href or hsync;
1: input horizontal signal is drdy;
[0]: reserved;
*/


  	 {0x20, 0x00, 0},				//high byte of start position 
        {0x21, 0x00, 0},				//low byte of start position
        {0x22, 0x01, 0},				//high byte of end position
        {0x23, 0x20, 0},				//low byte of end position
/*(20h, 21h, 22h, 23h)
These registers indicate the position of vref, which is output to ISP/Special Effect.*/

        
       {0x2c, 0x02, 0},				//0x168=360
       {0x2d, 0xd0, 0},				//0x2d0=720
/*(2ch, 2dh)
Thses registers indicate the maximum of columns in a frame.*/


        {0x2e, 0x01, 0},		
        {0x2f, 0x20, 0},
/*(2eh, 2fh)
Thses registers indicate the maximum of rows in a frame*/     


        {0x01, 0x45, 0},


        {0x5c, 0x13, 0},			//0x5c=0x13, tv image will be ok!
        {0x93, 0x01, 0},			//******
        {0x92, 0x04, 0}
        
        
};


/* Sensor size control configure info */

const TReg gSnrSizeRegValVGA_FUJITSU[] = 
{      
   	
	{0x12, 0x80, 10},   // address , data, delay

};

/* Sensor size control configure info */

const TSnrSizeCfg gSensorSizeCfg_FUJITSU[] = 
{
	{
              {0,NULL},
              {0,NULL},
	       {640,288},
		0,
		0
	}

};

 

const TSnrInfo gSensorInfo_FUJITSU = 

{
       "FUJITSU TV chip CCIR656 mode",
	VIM_SNR_YUV,////1,							//yuv mode
	1,	//0x85c bit1  //High priority for 0x806				         //not use PCLK of sensor
	0,							//[0]: VSync delay enable; [1]: Fifo VSync delay enable; [2]: Use PCLK Negtive Edge.
	0,							//0 -- I2C, 1 -- Serial bus;
	1,							//[0]: 1 -- OV I2C bus
	80,							//I2C Rate : 100KHz
	0x42,							//I2C address
	0x42,							//ISP I2C address for special sensor
	0x03,							//power config
	0,							//reset sensor

	3,							//brightness 
	3,							//contrast
	
	{0,NULL},														//sensor standby
       {sizeof(gSnrSizeRegValVGA_FUJITSU) / sizeof(TReg), (PTReg)&gSnrSizeRegValVGA_FUJITSU},                                                       //snr initial value
      // {0,NULL},	//guoying 2/18/2008
	
	{sizeof(gSifPwrOnSeqRegVal_FUJITSU) / sizeof(TReg), (PTReg)&gSifPwrOnSeqRegVal_FUJITSU},		//Sif config sequence(Reg.800 bit0~2) when sensor power on
	{0, NULL},																								//Sif config sequence(Reg.800 bit0~2) when sensor standby
	{sizeof(gSifRegVal_FUJITSU) / sizeof(TReg), (PTReg)&gSifRegVal_FUJITSU},						//sif initial value
																							//isp initial value
	{sizeof(gSensorSizeCfg_FUJITSU) / sizeof(TSnrSizeCfg), (PTSnrSizeCfg)&gSensorSizeCfg_FUJITSU},		//size configure 

	SensorSetReg_FUJITSU,						//set reg callback
	SensorGetReg_FUJITSU,						//get reg callback
	CheckId_FUJITSU,							//Sensor ID Check Call
	SensorSetMirrorFlip_FUJITSU,				//Set Sensor Mirror Flip Call
	SensorSetContrast_FUJITSU,					//Sensor switch size callback
	SensorSetBrightness_FUJITSU,				//set et callback
	NULL,
	SensorSetMode_FUJITSU,
	NULL,										//Sensor switch size callback
	NULL,
	{	//ythd of flash,et of flash
		0x20,			10
	},
};
#endif

#if V5_DEF_SNR_SANYO_NORMAL

/* write sensor register callback */

void SensorSetReg_SANYO(UINT16 uAddr, UINT16 uVal)

{
     	V5B_SifI2cWriteByte((UINT8)uAddr,(UINT8)uVal);
}

void SensorGetReg_SANYO(UINT8 uAddr, UINT16 *uVal)
{
	V5B_SifI2cReadByte((UINT8)uAddr, (UINT8*)uVal);

}

UINT8 CheckId_SANYO(void)
{


                 return SUCCEED; 

}

/******************************************************************

         Desc: set sensor mirror and flip callback 

         Para: BIT0 -- mirror

                     BIT1 -- flip

*******************************************************************/

void SensorSetMirrorFlip_SANYO(UINT8 val)
{
}

/******************************************************************

         Desc: set sensor contrast  callback 

         Para: 1-5 step

*******************************************************************/

void SensorSetContrast_SANYO(UINT8 val)
{
}

/******************************************************************

         Desc: set sensor Brightness  callback 

         Para: 1-5 step

*******************************************************************/

void SensorSetBrightness_SANYO(UINT8 val)
{
	
}

/******************************************************************

         Desc: set sensor mod  callback 

         Para:  val: 	1	50hz
         	    		2	60hz
         	     	    	3	night 
         	     	     	4	outdoor

*******************************************************************/

void SensorSetMode_SANYO(UINT8 val)
{
	
}

/******************************************************************

         Desc: get sensor ET  callback 			//add by guoying 9/6/2007
       
         Para:  val: 	*RegBuf					//the data buffer pointer
*******************************************************************/
void SensorGetET_SANYO(UINT8 *RegBuf)
{
	
	
}
/******************************************************************

         Desc: set sensor ET  callback 		//add by guoying 9/6/2007
       
         Para:  val: 	*RegBuf				//the data buffer pointer
*******************************************************************/
void SensorSetET_SANYO(UINT8 *RegBuf, UINT8 div)
{

	
	
}

const TReg gSifPwrOnSeqRegVal_SANYO[] =
{

	{0x00, 0x63,	1},
	{0x00, 0x62,	1},
	{0x00, 0x63,	1}

};

const TReg gSifRegVal_SANYO[] =
{        

        {0x02, 0x00, 0},		//normal setting

/*//0x02 reg
[7:6] tref_lh_blk: Timing reference value for the head of blanking line in CCIR656, tref_lh_blk;
[5:4] tref_lt_blk: Timing reference value for the tail of blanking line in CCIR656, tref_lt_blk;
[3:2] Internal delay count (sif_clk) when handle YUV data
[1:0]: the YUV data format from sensor
    00: sensor outputs 'UYVY';
    01: sensor outputs 'VYUY';
    10: sensor outputs 'YUYV';
    11: sensor outputs 'YVYU';
 
*/


        {0x03, 0x1b, 0},
 /*//0x03 reg
 [5]: Vref_full, internal verf is high constantly because sensor's Hsync is constant level between rows.
 0: when sensor's Hsync has level change between rows.
 1: when sensor's Hsync has not any level change between rows,
[4]: vd_mode - vertical sync signal for sensor width selection
 0: vsync send to sensor more than 1 row
1: vsync send to sensor less than 1 row
[3]: vsync_mode - vertical sync signal for isp width selection
 0 : vsync send to isp more than 1 row;
1: vsync send to isp less than 1 row
[2]: v_bypass - input vertical signal bypass selection.
 0 : SIF generate vertical signal for ISP.
1: the input vertical signal is bypass to ISP
[1]: v_polarity - input vertical signal polarity flag
1: input vertical signal is active high;
0: input vertical signal is active low;
[0]: reserved
 
*/


        {0x04, 0x05, 0},		//normal setting
       // {0x04, 0x1d, 0},			//ccir656 setting
/*//0x04 reg
[7:6] tref_lh_vld: Timing reference value for the head of valid line in CCIR656, tref_lh_vld;
[5:4] tref_lt_vld: Timing reference value for the tail of valid line in CCIR656, tref_lt_vld;
[3]: h_bypass - input horizontal signal bypass selection. If reg85c [1] = 1, using sensor's pclk as sample clock, h_bypass must be "0". 
 0: SIF generate horizontal signal to ISP;
1: the input horizontal signal is bypass to ISP;
[2]: h_polarity - input horizontal signal polarity flag,
0: input horizontal signal is active low;
1: input horizontal signal is active high;
[1]: h_flag - input horizontal signal flag,
0: input horizontal signal is href or hsync;
1: input horizontal signal is drdy;
[0]: reserved;
*/


	 {0x20, 0x00, 0},				//high byte of start position 
        {0x21, 0x00, 0},				//low byte of start position
        {0x22, 0x00, 0},				//high byte of end position
        {0x23, 0xf0, 0},				//low byte of end position
   /*(20h, 21h, 22h, 23h)
These registers indicate the position of vref, which is output to ISP/Special Effect.*/

       							 //0x280=640
       {0x2c, 0x02, 0},				//0x168=360
       {0x2d, 0x80, 0},				//0x2d0=720
       //{0x2d, 0xf8, 0},	 			//0x2f8=760
/*(2ch, 2dh)
Thses registers indicate the maximum of columns in a frame.*/


        {0x2e, 0x00, 0},				//0x0120=288
        {0x2f, 0xf0, 0},				//0x00f0=240
        // {0x2f, 0x78, 0},				//0x00f0=240
       
/*(2eh, 2fh)
Thses registers indicate the maximum of rows in a frame*/     


        {0x01, 0x45, 2},

        
};


/* Sensor size control configure info */

const TReg gSnrSizeRegValVGA_SANYO[] = 
{      
  	{0x00, 0x0f, 0},
       {0x01, 0x01, 0},			//output 320x240
      //  {0x01, 0x06, 0},			//output 720x480
      //{0x01, 0x21, 0},			//output 640x480
       {0x02, 0x43, 0},
       {0x03, 0x00, 0},
	{0x04, 0x03, 0},			//30 fps
	//{0x04, 0x0b, 2},				//15 fps
  	{0x05, 0x15, 0},
   	{0x06, 0x80, 0},		
   	{0x07, 0x00, 0},		
   	
   	{0x08, 0x80, 0},	
   	{0x09, 0x00, 0},	
   	{0x0a, 0x04, 0},	
   	{0x10, 0x02, 0}

  
 
};


/* Sensor size control configure info */

const TSnrSizeCfg gSensorSizeCfg_SANYO[] = 
{
	{
		
               {0,NULL},//{sizeof(gSizeRegSnrVal_OV7670) / sizeof(TReg), (PTReg)&gSizeRegSnrVal_OV7670},	//9/7/2007 add by guoying 
               {0,NULL},
	        {640, 240},			 //guoying 5/13/2008	//for 640x480
	
		1,
		1

	}

};

 

const TSnrInfo gSensorInfo_SANYO = 

{
       "SANYO yuv mode",
	VIM_SNR_YUV,							//yuv mode
	1,	//0x85c bit1  //High priority for 0x806				         //not use PCLK of sensor
	//0,							//guoying 1/23/2008, can't use this parameter, must use sensor pclk!
	0,							//[0]: VSync delay enable; [1]: Fifo VSync delay enable; [2]: Use PCLK Negtive Edge.
	0,							//0 -- I2C, 1 -- Serial bus;
	1,							//[0]: 1 -- OV I2C bus
	80,							//I2C Rate : 100KHz
	0x88,//0x88,							//I2C address
	//0x89,							//ISP I2C address for special sensor
	0x88,		
	0x03,							//power config
	0,							//reset sensor

	3,							//brightness 
	3,							//contrast
	
	{0,NULL},														//sensor standby
       {sizeof(gSnrSizeRegValVGA_SANYO) / sizeof(TReg), (PTReg)&gSnrSizeRegValVGA_SANYO},                                                       //snr initial value
	
	{sizeof(gSifPwrOnSeqRegVal_SANYO) / sizeof(TReg), (PTReg)&gSifPwrOnSeqRegVal_SANYO},		//Sif config sequence(Reg.800 bit0~2) when sensor power on
	{0, NULL},																								//Sif config sequence(Reg.800 bit0~2) when sensor standby
	{sizeof(gSifRegVal_SANYO) / sizeof(TReg), (PTReg)&gSifRegVal_SANYO},						//sif initial value
																							//isp initial value
	{sizeof(gSensorSizeCfg_SANYO) / sizeof(TSnrSizeCfg), (PTSnrSizeCfg)&gSensorSizeCfg_SANYO},		//size configure 

	SensorSetReg_SANYO,																				//set reg callback
	SensorGetReg_SANYO,																				//get reg callback
	CheckId_SANYO,																				//Sensor ID Check Call
	SensorSetMirrorFlip_SANYO,																									//Set Sensor Mirror Flip Call
	SensorSetContrast_SANYO,																									//Sensor switch size callback
	SensorSetBrightness_SANYO,																									//set et callback
	NULL,	//set et callback
	SensorSetMode_SANYO,
	NULL,
	NULL,
	
	
	{	//ythd of flash,et of flash
		0x20,			10
	},
};

#endif


/////////////////////////////////////////////////////////////////////////////////

const PTSnrInfo gPSensorInfo[] =
{
	//0.3M
#if V5_DEF_SNR_SIV120D_YUV	
	(const PTSnrInfo)&gSensorInfo_SIV120D,			//V5_DEF_SNR_MT9V111_YUV
#endif
#if	V5_DEF_SNR_MT9V111_YUV
	(const PTSnrInfo)&gSensorInfo_MT9V111,			//V5_DEF_SNR_MT9V111_YUV
#endif
#if	V5_DEF_SNR_MT9V112_YUV
	(const PTSnrInfo)&gSensorInfo_MT9V112,			//V5_DEF_SNR_MT9V112_YUV
#endif
#if	V5_DEF_SNR_OV7649_YUV
	(const PTSnrInfo)&gSensorInfo_OV7649,			//V5_DEF_SNR_OV7649_YUV
#endif
#if	V5_DEF_SNR_OV7660_YUV
	(const PTSnrInfo)&gSensorInfo_OV7660,			//V5_DEF_SNR_OV7660_YUV
#endif
#if	V5_DEF_SNR_OV7670_YUV
	(const PTSnrInfo)&gSensorInfo_OV7670,			//V5_DEF_SNR_OV7670_YUV
#endif
#if	V5_DEF_SNR_HV7131GP_YUV
	(const PTSnrInfo)&gSensorInfo_HV7131GP,			//V5_DEF_SNR_HV7131GP(HYNIX7131GP)_YUV
#endif
#if	V5_DEF_SNR_HV7131RP_YUV
	(const PTSnrInfo)&gSensorInfo_HV7131RP,			//V5_DEF_SNR_HV7131RP_YUV
#endif
#if	V5_DEF_SNR_NOON30PC11_YUV
	(const PTSnrInfo)&gSensorInfo_NOON30PC11,		//V5_DEF_SNR_NOON30PC11_YUV
#endif

#if	V5_DEF_SNR_S5K83A_YUV
	(const PTSnrInfo)&gSensorInfo_S5K83A,			//V5_DEF_SNR_S5K83A_YUV
#endif
#if	V5_DEF_SNR_MC501CB_YUV
	(const PTSnrInfo)&gSensorInfo_MC501CB,			//V5_DEF_SNR_MC501CB_YUV
#endif

#if	V5_DEF_SNR_PO3030K_YUV
	(const PTSnrInfo)&gSensorInfo_PO3030K,			//V5_DEF_SNR_PO3030K_YUV
#endif
#if	V5_DEF_SNR_SIV100A_YUV
	(const PTSnrInfo)&gSensorInfo_SIV100A,			//V5_DEF_SNR_SIV100A_YUV
#endif
/*
#if	V5_DEF_SNR_SIV120D_YUV
	(const PTSnrInfo)&gSensorInfo_SIV120D,			//V5_DEF_SNR_SIV100A_YUV
#endif
*/
#if	V5_DEF_SNR_OV6680_YUV                           //fengyu
	(const PTSnrInfo)&gSensorInfo_OV6680,			//V5_DEF_SNR_OV6680_YUV
#endif

#if	V5_DEF_SNR_SP80818_YUV                           //fengyu
	(const PTSnrInfo)&gSensorInfo_SP80818,			//V5_DEF_SNR_SP80818_YUV
#endif

#if	V5_DEF_SNR_GC306_YUV                           //fengyu
	(const PTSnrInfo)&gSensorInfo_GC306,			//V5_DEF_SNR_GC306_YUV
#endif

#if	V5_DEF_SNR_OM6802_YUV                           //fengyu
	(const PTSnrInfo)&gSensorInfo_OM6802,			//V5_DEF_SNR_OM6802_YUV
#endif

#if	V5_DEF_SNR_S5KA3AFX_YUV                         //fengyu
	(const PTSnrInfo)&gSensorInfo_S5KA3AFX,			//V5_DEF_SNR_S5KA3AFX_YUV
#endif

#if	V5_DEF_SNR_OV7725_YUV                           //fengyu
	(const PTSnrInfo)&gSensorInfo_OV7725,			//V5_DEF_SNR_OV7725_YUV
#endif

#if	V5_DEF_SNR_SIV100B_YUV                          //fengyu
	(const PTSnrInfo)&gSensorInfo_SIV100B,			//V5_DEF_SNR_SIV100B_YUV
#endif

#if	V5_DEF_SNR_ET8EC3_YUV                          //fengyu
	(const PTSnrInfo)&gSensorInfo_ET8EC3,			//V5_DEF_SNR_ET8EC3_YUV
#endif

#if	V5_DEF_SNR_BF3403_YUV                          //fengyu
	(const PTSnrInfo)&gSensorInfo_BF3403,			//V5_DEF_SNR_ET8EC3_YUV
#endif
#if	V5_DEF_SNR_TEST_YUV
	(const PTSnrInfo)&gSensorInfo_Test,			//V5_DEF_SNR_ET8EC3_YUV
#endif

#if	V5_DEF_SNR_FUJITSU_CCIR656			//guoying
	(const PTSnrInfo)&gSensorInfo_FUJITSU,			//V5_DEF_SNR_FUJITSU_CCIR656
#endif

#if	V5_DEF_SNR_SANYO_NORMAL
	(const PTSnrInfo)&gSensorInfo_Test,			
#endif
	NULL												//Avoid no sensor define
};


