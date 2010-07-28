#ifndef __CANOPUS_VC0528_H__
#define __CANOPUS_VC0528_H__
/*-----------------------------------------------------------------------------
 * file name : canopus_vc0528.h
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
//	unsigned char data_idx[20][2];
	unsigned int data_idx[20][2];
	unsigned char *pdata_idx; 
} __attribute__((packed)) ctrl_jpeg_file;


/*_____________________ Constants Definitions _______________________________*/
/* for unit test */
#define _SSMC_IP_CMD_BASE 				0x10
#define _VC0528_IO_CMD_BASE 			0x20
#define _VC0528_FUNC_CMD_BASE 	    	0x30
#define _VC0525_IOCTL_CMD_BASE 			0x40

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

// v4l2 camera control cmd
#define VC0528_CAMERA_CAPTURE_FRAME		_IO   ('Q', _CMD3(0x07))
#define VC0528_CAMERA_CAPTURE_STILL 	_IO   ('Q', _CMD3(0x08))
#define VC0528_CAMERA_JPEG_READ 		_IOWR ('Q', _CMD3(0x09) , ctrl_jpeg_file)
#define VC0528_CAMERA_JPEG_WRITE 		_IOWR ('Q', _CMD3(0x0A) , ctrl_jpeg_file)

// v4l2 lcd control cmd
#define VC0528_LCD_GUI_DROW_1 			_IO ('Q', _CMD3(0x0B))
#define VC0528_LCD_GUI_DROW_2 			_IO ('Q', _CMD3(0x0C))

#define V5_FUNC_STARTNR 				_CMD3(0x00)
#define V5_FUNC_ENDNR 					_CMD3(0x0C)


/*_____________________ Function Declarations _______________________________*/
//extern int s3c24xx_smc_init(void);
extern int s3c24xx_smc_write_read(unsigned long addr, unsigned long data);
extern int s3c24xx_smc_read(unsigned long addr);
extern unsigned int s3c24xx_smc_read_bust(unsigned long addr);
extern int stc24xx_smc_write(unsigned long addr,unsigned long data);
extern void s3c24xx_smc_init(void);
extern void s3c24xx_smc_status(void);
extern void s3c24xx_smc_data_write(ctrl_32_info args);

extern int canopus_vc0528_clk_on(void);
extern int canopus_vc0528_set_multi16(void);
extern void canopus_vc0528_reset_core(void);
extern int canopus_vc0528_init_pll(void);
extern int canopus_vc0528_pll_power_on(void);
extern void canopus_vc0528_8_write(ctrl_32_info args);
extern ctrl_32_info canopus_vc0528_8_read(ctrl_32_info args);
extern void canopus_vc0528_8_write_burst(ctrl_32_infos args);
extern void canopus_vc0528_16_write(ctrl_32_info args);
extern ctrl_32_info canopus_vc0528_16_read(ctrl_32_info args);
extern void canopus_vc0528_16_write_burst(ctrl_32_infos args);
#endif /* __CANOPUS_VC0528_H__ */



