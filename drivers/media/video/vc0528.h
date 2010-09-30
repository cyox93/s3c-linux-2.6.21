#ifndef __VC0528_H__
#define __VC0528_H__
/*-----------------------------------------------------------------------------
 * file name : vc0528.h
 * 
 * purpose : 
 * 
 * copyright 1999 - 2010 unidata communication systems, inc.
 * all right reserved. 
 *
 * this software is confidential and proprietary to unidata 
 * communication systems, inc. no part of this software may 
 * be reproduced, stored, transmitted, disclosed or used in any
 * form or by any means other than as expressly provide by the 
 * written license agreement between unidata communication 
 * systems and its licensee.
 *
 * notes: n/a
 *---------------------------------------------------------------------------*/

/*_____________________ Include Header ______________________________________*/

/*_____________________ Variables Definitions _______________________________*/

/*_____________________ Type Definitions ____________________________________*/
typedef struct
{
	unsigned short addr;
	unsigned short data;
	unsigned int bypass;
} __attribute__((packed)) ctrl_16_info;

typedef struct
{
	unsigned int addr;
	unsigned int data;
	unsigned int bypass;
} __attribute__((packed)) ctrl_32_info;

typedef struct
{
	unsigned int size;
	unsigned int data_idx[2][20];
	unsigned int bypass;
} __attribute__((packed)) ctrl_32_infos;

typedef struct
{
	unsigned int size;
	unsigned int wr_sel;
	unsigned int data_idx[20][2];
	unsigned char *pdata_idx; 
} __attribute__((packed)) ctrl_jpeg_file;

typedef struct
{
	unsigned long  frbuf_size;
	unsigned long  frame_size;
	unsigned int   frame_rate;
	unsigned int   frame_max;
	unsigned int   frame_idx[15];
	unsigned long *frame_buf;
	unsigned long *pdata_idx; 
} __attribute__((packed)) ctrl_jpeg_read;

typedef struct
{
	unsigned int size;
	unsigned int data[300][2];
} __attribute__((packed)) ctrl_sensor_test;


/*_____________________ Constants Definitions _______________________________*/

/* VC0528 address */
#define _INDEX_ADDR 					0xF3600000  // index address 
#define _VALUE_ADDR  					0xF3600008  // value address

/* host control register */
#define MEM_8_FLG 						0x18b6   
#define REG_8_HIGH_WORD 				0x18b4 		// index address high 8bit
#define REG_8_LOW_WORD 					0x18b2 		// index address low  8bit
#define REG_8_FLG 						0x18b0
#define BYPASS_SEL 						0x1890
#define SEL_8_16 						0x188C
#define SEL_PORT 						0x1888
#define SEL_WRITE_READ 					0x1886
#define MUL_U2IA 						0x1880
#define MEM_FLG 						0x1850
#define MEM_HIGH_WORD 					0x1848
#define MEM_LOW_WORD 					0x1844
#define CMD_BLOCK_PARAMETERS_OFFSET 	0x1802

/* address map tale of VC0528 */
//Reserved: 200000h~FFFFFFh
#define SDRAM_TABLE 					0x100000	// On-chip SRAM_Table 		  : 100000h~1FFFFFh
#define LCDC_UNIT_CTR  					0x002800	// LCDC unit control register : 002800h~002BFFh
#define MARB_UNIT_CTR  					0x002400	// MARB unit control register : 002400h~0027FFh
#define IPP_UNIT_CTR  					0x001C00	// IPP unit control register  : 001C00h~001FFFh
#define BIU_UNIT_CTR  					0x001800	// BIU unit control register  : 001800h~001BFFh
#define CPM_UNIT_CTR   	    			0x001400	// CPM unit control register  : 001400h~0017FFh
#define LBUF_UNIT_CTR  					0x000C00	// LBUF unit control register : 000C00h~000fFFh
#define SIF_UNIT_CTR   					0x000800	// SIF unit control register  : 000800h~000BFFh
#define LCDIF_UNIT_CTR  				0x000400	// LCDIF unit control register: 000400h~0007FFh
#define JPEG_UNIT_CTR   				0x000000	// JPEG unit control register : 000000h~0003FFh

#define V5_MULTI8_REG_PORT  			0xb0
#define V5_MULTI8_REG_WORDL 			0xb2

#define V5_MULTI8_REG_WORDH 			0xb4
#define V5_MULTI8_MEM_PORT 				0xb6

#define V5_MULTI8_MEM_FLG 				0x1850

#define VIM_MULTI8  					0
#define VIM_MULTI16 					1


/* for unit test */
#define _SSMC_IP_CMD_BASE 				0x10
#define _VC0528_IO_CMD_BASE 			0x20
#define _VC0528_FUNC_CMD_BASE 	    	0x30
#define _VC0525_IOCTL_CMD_BASE 			0x50

#define _VC0528_LCDC_CMD_BASE 			0x60
#define _VC0528_MABR_CMD_BASE 			0x70
#define _VC0528_IPP_CMD_BASE 			0x80
#define _VC0528_BIU_CMD_BASE 			0x90
#define _VC0528_CMP_CMD_BASE 			0xa0
#define _VC0528_LBF_CMD_BASE 			0xb0
#define _VC0528_SIF_CMD_BASE 			0xc0
#define _VC0528_JPEG_CMD_BASE 			0xd0

#define _CMD1(offset)  					(_SSMC_IP_CMD_BASE 	   + offset)
#define _CMD2(offset)  					(_VC0528_IO_CMD_BASE    + offset)
#define _CMD3(offset)  					(_VC0528_FUNC_CMD_BASE  + offset)
#define _CMD4(offset)  					(_VC0525_IOCTL_CMD_BASE + offset)
#define _CMD5(offset)  		 			(_VC0528_LCDC_CMD_BASE  + offset)
#define _CMD6(offset)  		   			(_VC0528_MABR_CMD_BASE  + offset)
#define _CMD7(offset)  		 			(_VC0528_IPP_CMD_BASE   + offset)
#define _CMD8(offset)  			    	(_VC0528_BIU_CMD_BASE   + offset)
#define _CMD9(offset)  					(_VC0528_CMP_CMD_BASE   + offset)
#define _CMD10(offset) 					(_VC0528_LBF_CMD_BASE   + offset)
#define _CMD11(offset) 					(_VC0528_SIF_CMD_BASE   + offset)
#define _CMD12(offset) 					(_VC0528_JPEG_CMD_BASE  + offset)

/* smc control */
#define SSMC_SMBIDCYR0 					_IOW  ('Q', _CMD1(0x00) , ctrl_32_info)
#define SSMC_SMBWSTRDR0 				_IOW  ('Q', _CMD1(0x01) , ctrl_32_info)
#define SSMC_SMBWSTWRR0					_IOW  ('Q', _CMD1(0x02) , ctrl_32_info)
#define SSMC_SMBWSTOENR0 				_IOW  ('Q', _CMD1(0x03) , ctrl_32_info)
#define SSMC_SMBWSTWENR0 				_IOW  ('Q', _CMD1(0x04) , ctrl_32_info)
#define SSMC_SMBCR0 					_IOW  ('Q', _CMD1(0x05) , ctrl_32_info)
#define SSMC_SSMCCR 					_IOW  ('Q', _CMD1(0x06) , ctrl_32_info)
#define SSMC_WRITE 						_IOWR ('Q', _CMD1(0x07) , ctrl_32_info)
#define SSMC_INIT 						_IOW  ('Q', _CMD1(0x08) , ctrl_32_info)
#define SMC_CTL_STARTNR 				_CMD1(0x00)
#define SMC_CTL_ENDNR 		 			_CMD1(0x08)

/* VC0528 control  */
// unit read write cmd
#define VC0528_8_WRITE 					_IOW  ('Q', _CMD2(0x00) , ctrl_32_info) 
#define VC0528_8_READ 					_IOWR ('Q', _CMD2(0x01) , ctrl_32_info)
#define VC0528_8_WRITE_BURST 			_IOWR ('Q', _CMD2(0x02) , ctrl_32_infos)
#define VC0528_16_WRTIE 				_IOWR ('Q', _CMD2(0x03) , ctrl_32_info)
#define VC0528_16_READ 					_IOWR ('Q', _CMD2(0x04) , ctrl_32_info)
#define VC0528_16_WRITE_BURST 			_IOWR ('Q', _CMD2(0x05) , ctrl_32_infos)
#define V5_RW_IO_STARTNR 				_CMD2(0x00)
#define V5_RW_IO_ENDNR 					_CMD2(0x05)

// function set-up cmd
#define VC0528_RESET_CORE				_IOW  ('Q', _CMD3(0x00) , ctrl_32_info)
#define VC0528_INIT 					_IOW  ('Q', _CMD3(0x01) , ctrl_32_info)
#define VC0528_SET_MULTI16 				_IOW  ('Q', _CMD3(0x02) , ctrl_32_info)
#define VC0528_CLK_ON 					_IOWR ('Q', _CMD3(0x03) , ctrl_32_info)
#define VC0528_INIT_PLL 				_IOWR ('Q', _CMD3(0x04) , ctrl_32_info)
#define VC0528_PLL_POWER_ON				_IOWR ('Q', _CMD3(0x05) , ctrl_32_info)
#define VC0528_BYPASS_MODE				_IO   ('Q', _CMD3(0x06))
#define VC0528_NORMAL_MODE				_IO   ('Q', _CMD3(0x07))

// camera control cmd
#define VC0528_CAMERA_CAPTURE_FRAME		_IO   ('Q', _CMD3(0x08))
#define VC0528_CAMERA_CAPTURE_STILL 	_IO   ('Q', _CMD3(0x09))
#define VC0528_CAMERA_PREVIEW 		 	_IO   ('Q', _CMD3(0x0A))
#define VC0528_CAMERA_JPEG_READ 		_IOWR ('Q', _CMD3(0x0B) , ctrl_jpeg_read)
#define VC0528_CAMERA_JPEG_WRITE 		_IOWR ('Q', _CMD3(0x0C) , ctrl_jpeg_file)
#define VC0528_CAMERA_TEST_INIT 		_IOWR ('Q', _CMD3(0x0D) , ctrl_sensor_test)
#define VC0528_CAMERA_SENSOR_NEW_SET	_IOWR ('Q', _CMD3(0x0E) , ctrl_sensor_test)
#define VC0528_CAMERA_SENSOR_ORG_SET	_IOWR ('Q', _CMD3(0x0F) , ctrl_sensor_test)
#define VC0528_CAMERA_SENSOR_CHECK_NEW	_IOWR ('Q', _CMD3(0x10) , ctrl_sensor_test)
#define VC0528_CAMERA_SENSOR_CHECK_ORG	_IOWR ('Q', _CMD3(0x11) , ctrl_sensor_test)

// lcd control cmd
#define VC0528_LCD_GUI_DROW_1 			_IO   ('Q', _CMD3(0x12))
#define VC0528_LCD_GUI_DROW_2 			_IO   ('Q', _CMD3(0x13))
#define VC0528_LCD_MODE_AFIRST 			_IO   ('Q', _CMD3(0x14))
#define VC0528_LCD_MODE_BLONLY_0		_IO   ('Q', _CMD3(0x15))
#define VC0528_LCD_MODE_BLONLY_1		_IO   ('Q', _CMD3(0x16))
#define VC0528_LCD_MODE_BLEND 			_IO   ('Q', _CMD3(0x17))

#define V5_FUNC_STARTNR 				_CMD3(0x00)
#define V5_FUNC_ENDNR 					_CMD3(0x17)

/*_____________________ Function Declarations _______________________________*/
extern int canopus_bedev_ioctl(unsigned int cmd, void *args);
#endif /* __VC0528_H__ */



