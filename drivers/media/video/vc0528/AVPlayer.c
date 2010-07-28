/*
 *  VC0838RDK/AVPlayer.c
 *
 *  Copyright (C) 2004 Vimicro Corporation
 *  
 *  Written by TangMian	Email: tangmian@vimicro.com	Tel: 010-68948888-8902
 *
 *  Overview:
 *   This is the AV Player for 838 device. This file include some functions to be used to play Audio+Video.
 *   We support Mjpeg+MP3, Mjpeg+Midi, Mjpeg+Midi+Lyric file. 
 *   
 *	2005,7,17		Updated By TangMian
 *
 *	2005,9,21		Update By TangMian
 *
 *	2005,10,9		Update By TangMian
 *				Add Playing avi file(MJpeg + IMA ADPCM) function
 */

#include <linux/string.h>


#include "VIM_HIGH_API.h"
#include "AVPlayer.h"


#define	MP3_FIFO_SIZE		8 * 1024
#define	MAX_ADPCM_FIFO_SIZE	16 * 1024

#define 	NOTUSED		0


static	KAVVERSION	AV_VER={1, 3, 3, 6, 1, 16};	// Ver 1.33 2006/04/05



static	UINT32		dState = STATE_UNLOAD;

static	pKHEADER	pkHeader;
static	pKVIDEO		pkVideo;
static	pKAMP3		pkAmp3;
static	pKAMIDI		pkAmidi;
static	pKAVIINFO	pkAvi;

static	UINT8		bTicker = 50;

static	UINT16		wStartX = 0;
static	UINT16		wStartY = 0;
static	UINT16		wWidth = 128;
static	UINT16		wHeight = 160;
UINT32 Avilength=0;


UINT8 			g_avbuffer[MAX_ADPCM_FIFO_SIZE];

UINT8*			pbMidiS = NULL;
UINT8*			pbDataS = NULL;
UINT32			dDataS = 0;
UINT8			bSourc;
UINT8			g_SkipCount;
static	UINT8*		pbFileStream_Ptr = NULL;
static	UINT8			bAVEnd = 0;			// 0: Not End;	1: End
static	UINT32			dTimeNow = 0;
static  UINT32			dTimeBase = 0;
static	UINT8			b12Flag = 0;			// 0: Display Jpeg From Buffer0; 1: Display Jpeg From Buffer1
static	UINT8			g_bReSynFlag = 0;			// 0: no need synchronize, 1: resynchronize
static 	TIMEINFO		g_Time;
static	UINT32			g_AVDisJPGSize;
static	pKVAVMP3		pAvMp3; 

static 	UINT8			g_bFrameFlag;
static	PEFUNCTION		pAvfunction = NULL;
static SINT32 s_AVFileCursor;		// AV file read cursor
static SINT32 s_TotalReadCount;
AVFILEBUF g_AVFileBufInfo;	// AV file buffer 


void		ReadAviBuffer(UINT32 mode, UINT8* bBuffer, UINT32 dBufferSize, UINT32* pdRealSize, UINT32 dOffset);
void 		ReadAVBuffer(UINT32 mode, UINT8* bBuffer, UINT32 dBufferSize, UINT32* pdRealSize, UINT32 dOffset);
void 		STimer_Handler(UINT8* pbBuffer);
void 		AV_Isr(void);

UINT32		AVlr_GetCurTime(void);
static UINT8 btempBufferHEADER[12];
static UINT8 btempBufferKVIDEO[68];
static UINT8 btempBufferKAMP3[60];
static UINT8 btempBufferKVAVMP3[92];
// VMI play mode buffer, 20Kx2
static UINT8 btempBufferPIC1[20*1024];
static UINT8 btempBufferPIC2[20*1024];
// AVI play mode buffer, 20K, can use btempBufferPIC1 to reduce memory cost
//static UINT8 g_AVDisJPGBuf[MAXJPGSIZE];

static KAVIINFO s_AVI_info;
#if SUPPORT_FLASH
static	pKVAVMIDI		pAvMidi;
void* V_Malloc(UINT32 nbyte);
void V_Free(void* mem);
static void LoadMusic(UINT8 devid,HUGE UINT8 *pbFilePtr, UINT32 dFileSize, UINT8 bSrc);
static void StartPlayMusic(UINT32 dPos, UINT32 dRepeat, PEFUNCTION pFun);
static void StopPlayMusic(void);
void ReadMidiBuffer(void);
#endif

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/

#if BIG_ENDIAN
/***************************************************************
Description:
		Little endian to big endian convertion

Parameters:
		pBuf:		source and destine address
		size:			buffer length, make sure size is multiple of 4
		
Returns:
		NULL

****************************************************************/
void LittleToBigEndian(void* pBuf, UINT32 size)
{
	UINT8 byVal1, byVal2, byVal3, byVal4, *pbyBuf;
	UINT32 i;

	pbyBuf = (UINT8*)pBuf;
	for(i=0;i<size/4;i+=4)
	{
		byVal1= pbyBuf[i];
		byVal2= pbyBuf[i+1];
		byVal3= pbyBuf[i+2];
		byVal4= pbyBuf[i+3];
		pbyBuf[i]=byVal4;
		pbyBuf[i+1]=byVal3;
		pbyBuf[i+2]=byVal2;
		pbyBuf[i+3]=byVal1;
	}
}
#endif

/***************************************************************
Description:
		AV file buffer intialization function, fill AV file buffer.

Parameters:
		pFile_Ptr:  File Handler
		filelength:   AV filelength
		
Returns:
		>=0: Number of bytes be read, <0: Error

****************************************************************/

SINT32 VPlr_AVFileBuf_Init(void* pFile_Ptr, SINT32  filelength)
{
	SINT32	err = 0;
	if(filelength>AV_FILEBUF_LENGTH)
	{
		VIM_USER_SeekFile(pFile_Ptr,0);
		VIM_USER_ReadFile(pFile_Ptr,g_AVFileBufInfo.AVfilebuf, AV_FILEBUF_LENGTH);
		s_AVFileCursor=AV_FILEBUF_LENGTH;		
	}	
	else
	{
		VIM_USER_SeekFile(pFile_Ptr,0);
		VIM_USER_ReadFile(pFile_Ptr,g_AVFileBufInfo.AVfilebuf, filelength);
		s_AVFileCursor=filelength;
	}
	g_AVFileBufInfo.Cursor_read = 0;
	g_AVFileBufInfo.Cursor_write = 0;
	g_AVFileBufInfo.pFile_Ptr = pFile_Ptr;
	g_AVFileBufInfo.filelength = filelength;
	s_TotalReadCount=0;
	return err;
}
/***************************************************************
Description:
		Fill AV file buffer.

Parameters:
		pFile_Ptr:  File Handler
		filelength:   AV filelength
		
Returns:
		status code
	
****************************************************************/

SINT32 VPlr_AVFileBuf_Fill(void* pFile_Ptr, SINT32  filelength)
{
	SINT32	err = 0;
	SINT32 empty, readsize,empty1, unreadsize;

	// end of file
	if(filelength==s_AVFileCursor)
		return 1;
	// Caculate the bytes to the end of the file 
	unreadsize=filelength-s_AVFileCursor;
	
	// AV file buffer is full	
	if(g_AVFileBufInfo.Cursor_read == g_AVFileBufInfo.Cursor_write)
		return 2;

	if(g_AVFileBufInfo.Cursor_read>g_AVFileBufInfo.Cursor_write)
	{
		empty = g_AVFileBufInfo.Cursor_read - g_AVFileBufInfo.Cursor_write;

#if DBGAVFILE
		VIM_USER_PrintDec( "empty=",empty);
#endif
		// caculate read size, 16K, 8K or 4K , 20ms or 10ms, 06.01.05
		if(empty>AV_FILEBUF_LENGTH/2)
		{
			//over half empty, fill buffer as much as possible 
			readsize=empty &0xffffe000;
		}
		else if(empty>0x4000)
		{
			readsize=0x4000;
		}
		else if(empty>0x3000)
		{
			readsize=0x3000;
		}
		else if(empty>0x2000)
		{
			readsize=0x2000;
		}
		else if(empty>0x1000)
		{
			readsize=0x1000;
		}
		else
		{
			readsize=0;
		}

		// no need to fill
		if(readsize==0)
			return 3;

		// to the end of file
		if(unreadsize<readsize)
			readsize = unreadsize;
		
		// read data into buffer
		VIM_USER_SeekFile(pFile_Ptr,s_AVFileCursor);
		VIM_USER_ReadFile(pFile_Ptr,g_AVFileBufInfo.AVfilebuf+g_AVFileBufInfo.Cursor_write, readsize);
		g_AVFileBufInfo.Cursor_write += readsize;
		s_AVFileCursor += readsize;

#if DBGAVFILE
		VIM_USER_PrintDec( "readsize=\r\n",readsize);
#endif
		
	}
	else
	{
		empty = AV_FILEBUF_LENGTH -g_AVFileBufInfo.Cursor_write + g_AVFileBufInfo.Cursor_read;

#if DBGAVFILE
		VIM_USER_PrintDec( "empty=",empty);
#endif

		// caculate read size, 16K, 8K or 4K , 20ms or 10ms, 06.01.05
		if(empty>AV_FILEBUF_LENGTH/2)
		{
			//over half empty, fill buffer as much as possible 
			readsize=empty &0xffffe000;
		}
		else if(empty>0x4000)
		{
			readsize=0x4000;
		}
		else if(empty>0x3000)
		{
			readsize=0x3000;
		}
		else if(empty>0x2000)
		{
			readsize=0x2000;
		}
		else if(empty>0x1000)
		{
			readsize=0x1000;
		}
		else
		{
			readsize=0;
		}

		// no need to fill
		if(readsize==0)
			return 3;

		// to the end of file
		if(unreadsize<readsize)
			readsize = unreadsize;

		// size to buffer head, this value is 4KxN
		empty1 = AV_FILEBUF_LENGTH -g_AVFileBufInfo.Cursor_write;
		if( empty1 >readsize )	
		{
			// read data into buffer
			VIM_USER_SeekFile(pFile_Ptr,s_AVFileCursor);
			VIM_USER_ReadFile(pFile_Ptr,g_AVFileBufInfo.AVfilebuf+g_AVFileBufInfo.Cursor_write, readsize);
			g_AVFileBufInfo.Cursor_write += readsize;
			s_AVFileCursor += readsize;
		}
		else if (empty1 ==readsize )
		{
			// read data into buffer
			VIM_USER_SeekFile(pFile_Ptr,s_AVFileCursor);
			VIM_USER_ReadFile(pFile_Ptr,g_AVFileBufInfo.AVfilebuf+g_AVFileBufInfo.Cursor_write,readsize);
			g_AVFileBufInfo.Cursor_write =0;
			s_AVFileCursor += readsize;
		}
		else
		{
			// read data into buffer
			VIM_USER_SeekFile(pFile_Ptr,s_AVFileCursor);
			VIM_USER_ReadFile(pFile_Ptr,g_AVFileBufInfo.AVfilebuf+g_AVFileBufInfo.Cursor_write, empty1 );
			s_AVFileCursor += empty1;					
			g_AVFileBufInfo.Cursor_write=0;	
			VIM_USER_SeekFile(pFile_Ptr,s_AVFileCursor);
			VIM_USER_ReadFile(pFile_Ptr,g_AVFileBufInfo.AVfilebuf+g_AVFileBufInfo.Cursor_write, readsize - empty1);
			g_AVFileBufInfo.Cursor_write =readsize - empty1;
			s_AVFileCursor +=( readsize - empty1);		
		}	
#if DBGAVFILE
		VIM_USER_PrintDec( "readsize=\r\n",readsize);
#endif
	}

	return err;
}

/***************************************************************
Description:
		Read from AV file buffer.

Parameters:
		pbBuffer:	   destined data buffer
		d_Size:      data size
		
Returns:
		Error code

****************************************************************/

SINT32 VPlr_AVFileBuf_Read( UINT8* pbBuffer, UINT32 d_Size)
{
	SINT32	err = 0;
	SINT32  datasize, datatotop;
	// get the data size in buffer
	if(g_AVFileBufInfo.Cursor_read>g_AVFileBufInfo.Cursor_write)
		datasize = AV_FILEBUF_LENGTH - (g_AVFileBufInfo.Cursor_read - g_AVFileBufInfo.Cursor_write); 
	else if (g_AVFileBufInfo.Cursor_read<g_AVFileBufInfo.Cursor_write)
		datasize = g_AVFileBufInfo.Cursor_write - g_AVFileBufInfo.Cursor_read;
	else // Cursor_write==Cursor_read, buffer full or buffer empty, or file size less than ring buffer length
	{
		if(g_AVFileBufInfo.filelength==s_AVFileCursor) 
		{
			if(s_TotalReadCount==0)
				datasize = s_AVFileCursor;
			else
				datasize = 0;				
		}		
		else
			datasize = AV_FILEBUF_LENGTH;
	}
	
	if (datasize<d_Size)
	{
#if DBGAVFILE
		Uart_Printf( "error occured, datasize=%d, d_Size=%d\r\n",datasize, d_Size);
		VIM_USER_PrintDec( "Cursor_read= \r\n", g_AVFileBufInfo.Cursor_read);
		VIM_USER_PrintDec( "Cursor_write= \r\n", g_AVFileBufInfo.Cursor_write);
		while(1);
#endif		
		return -1;
	}	

	// copy data to destined data buffer
	datatotop =AV_FILEBUF_LENGTH - g_AVFileBufInfo.Cursor_read;
	if(datatotop > d_Size)
	{
		memcpy(pbBuffer, g_AVFileBufInfo.AVfilebuf+g_AVFileBufInfo.Cursor_read, d_Size);
		g_AVFileBufInfo.Cursor_read += d_Size;
	}
	else if (datatotop == d_Size)
	{
		memcpy(pbBuffer, g_AVFileBufInfo.AVfilebuf+g_AVFileBufInfo.Cursor_read, d_Size);
		g_AVFileBufInfo.Cursor_read = 0;
	}
	else
	{
		memcpy(pbBuffer, g_AVFileBufInfo.AVfilebuf+g_AVFileBufInfo.Cursor_read, datatotop);
		g_AVFileBufInfo.Cursor_read = 0;	
		memcpy(pbBuffer+datatotop, g_AVFileBufInfo.AVfilebuf+g_AVFileBufInfo.Cursor_read, d_Size-datatotop);
		g_AVFileBufInfo.Cursor_read += (d_Size-datatotop) ;			
	}	
	// check total read data size from buffer, this value should less than AV file size
	s_TotalReadCount+=d_Size;
#if DBGAVFILE
	//VIM_USER_PrintDec( "s_TotalReadCount=", s_TotalReadCount);
	//VIM_USER_PrintDec( "d_size =", d_Size);
	VIM_USER_PrintDec( "Cursor_read= ", g_AVFileBufInfo.Cursor_read);
	VIM_USER_PrintDec( "Cursor_write= ", g_AVFileBufInfo.Cursor_write);
#endif	
		
	return err;
}

/***************************************************************
Description:
		Set AV file buffer, R/W cursor
		This function is very danger, do not use it unless you know what you do
		
Parameters:
		RorW:	   0: Read cursor;   	1: Write cursor
		offset:        cursor value		
Returns:
		NULL

****************************************************************/

void VPlr_AVFileBuf_CursorSet( SINT32 RorW, SINT32 offset)
{
	if (RorW)
		g_AVFileBufInfo.Cursor_write = offset;
	else
		g_AVFileBufInfo.Cursor_read = offset;
}

/***************************************************************
Description:
		Set AV file read cursor
		This function used to skip forward or backward, when play VMI
		
Parameters:
		step:         	 step value, ( >0 forward, 	<0 backward)		
		
Returns:
		NULL

****************************************************************/

void VPlr_AVFileBuf_FileCursorSet( SINT32 step )
{
	SINT32 temp;
	SINT32 ret;
	UINT8   bBuf[512];
	// clear ring buffer
	do
	{
		ret=VPlr_AVFileBuf_Read(bBuf, 512);
	}while(ret==0);
	
	temp = s_AVFileCursor +step;
	if(temp<0x120)	// data start offset
	{
		s_AVFileCursor = 0x120; 
	}
	else if (temp>g_AVFileBufInfo.filelength-0x8000) //data end keep 32K bytes
	{
		s_AVFileCursor = g_AVFileBufInfo.filelength-0x8000; 
	}
	else
	{
		s_AVFileCursor = temp;
	}
}
/***************************************************************
Description:
		Set AV file read cursor by time
		This function used to skip forward or backward, when play VMI
		
Parameters:
		step:         	 step value, ( >0 forward, 	<0 backward)		
		
Returns:
		NULL

****************************************************************/
void VPlr_AVFileBuf_SetTimePoint( UINT32 TimePoint)
{
	SINT32 BytesPerSecond, Step;
	UINT32 totalTime,curTime;
	
	totalTime=AVlr_GetTotalTime();
	if (TimePoint>=totalTime)
		return;
	
	curTime=AVlr_GetCurTime();
	BytesPerSecond = g_AVFileBufInfo.filelength/totalTime;
	Step = (TimePoint-curTime)*BytesPerSecond ;
	Step = Step &(0xfffff000);
	VPlr_AVFileBuf_FileCursorSet(Step);		
}

/***************************************************************
Description:
		Read File based on customer's file system.

Parameters:
		pFile_Ptr:  File Handler
		pbBuffer:   data buffer
		pFile_Size: number of bytes to be read	

Returns:
		>=0: Number of bytes be read, <0: Error

****************************************************************/
SINT32 VPlr_ReadFile(void* pFile_Ptr, UINT8* pbBuffer, UINT32 dFile_Size)
{
	SINT32	err = 0;
	VIM_USER_SeekFile(pFile_Ptr,Avilength);
	VIM_USER_ReadFile(pFile_Ptr,pbBuffer,dFile_Size);
	Avilength += dFile_Size;
	return err;
}


/***************************************************************
Description:
		Seek File based on customer's file system.

Parameters:
		pFile_Ptr:  File Handler
		dOffset:	   File offset

Returns:
		0: Success, <0: Error

****************************************************************/

SINT32 VPlr_SeekFile(void* pFile_Ptr, UINT32 dOffset)
{
	SINT32	err = 0;
	Avilength = dOffset;
	

	return err;
}



/***************************************************************
Description:
		Set Display Size on LCD

Parameters:
		x:		Start X Position
		y:		Start Y Position
		width:		Display width
		height:		Display	height

Returns:
		NULL

****************************************************************/
void AVDisplaySize(UINT16 x, UINT16 y, UINT16 width, UINT16 height)
{
	wStartX = x;
	wStartY = y;
	wWidth = width;
	wHeight = height;
}

/***************************************************************
Description:
		Display Jpeg

Parameters:
		pbBuf:		Jpeg Buffer
		dLength:	Buffer Length

Returns:
		NULL

****************************************************************/
void AVDisplayJpeg(UINT8* pbBuf, UINT32 dLength)
{
	V5H_Display_Vmi(pbBuf, dLength, wStartX, wStartY, wWidth, wHeight);
}


/***************************************************************
Description:
		ADPCM End ISR

Parameters:
		...

Returns:
		...		

****************************************************************/
static void AVInitFunc(UINT8 bMode)
{
	UINT8 temp;
	temp =	AV_VER.bC_Ver;  // to avoid warning
	
	/*if (bMode == 1)
		AV_audio_GetFunc(ReadAVBuffer, AV_Isr);
	else
		AV_audio_GetFunc(ReadAviBuffer, AV_Isr);*/
}



/***************************************************************
Description:
		ADPCM End ISR

Parameters:
		...

Returns:
		...		

****************************************************************/
void AdpcmEnd()
{
	//AV_audio_AdpcmEnd();
}

/***************************************************************
Description:
		Start Timer

Parameters:
		time:	timer ticker

Returns:
		NULL

****************************************************************/
static void AVStartTimer(UINT32 time)
{
	//AV_audio_StartTimer(time);
	VIM_USER_StartTimer(time);

}


/***************************************************************
Description:
		Stop Timer

Parameters:
		NULL

Returns:
		NULL

****************************************************************/
static void AVStopTimer()
{
	VIM_USER_PrintString("\nAVStopTimer");
	VIM_USER_StopTimer();
}


/***************************************************************
Description:
		Open PA

Parameters:
		NULL

Returns:
		NULL

****************************************************************/
static void AVOpenPA()
{
	//AV_audio_OpenPA();
}


/***************************************************************
Description:
		Close PA

Parameters:
		NULL

Returns:
		NULL

****************************************************************/
static void AVClosePA()
{
	//AV_audio_ClosePA();
}


/***************************************************************
Description:
		Start Channel and Prepare to play

Parameters:
		mode:		Different Channel	1: MP3 Channel;	2: ADPCM Channel
		bSource:	Data in RAM or in ROM
Returns:
		NULL

****************************************************************/
//extern UINT16 g_BitsPerSample;
static void StartChannel(UINT8 bMode, UINT8 bSource)
{
	/*if (bMode == 1)
		AV_audio_StartChannel(bMode, bSource, pAvMp3->dSampleRate, 0, 0, g_avbuffer);
	else if(bMode == 2)
		AV_audio_StartChannel(bMode, bSource, pkAvi->dSampleRate, pkAvi->dBlockSize, 0, g_avbuffer);
	else if(bMode == 3)//jsguo modified 12.22 --channel
		AV_audio_StartChannel(bMode, bSource, pkAvi->dSampleRate,g_BitsPerSample,  pkAvi->bChannel, g_avbuffer);
*/
}

/***************************************************************
Description:
		Stop MP3 Channel

Parameters:
		NULL

Returns:
		NULL

****************************************************************/
static void StopChannel(UINT8 bMode)
{
	//AV_audio_StopChannel(bMode);
}


/***************************************************************
Description:
		Get CHIP Fifo empty size.

Parameters:
		bMode:	Different Fifo		1: MP3 Fifo;	2: ADPCM Fifo
		size:	return size

Returns:
		NULL

****************************************************************/
static void AVGetFifoSize(UINT8 bMode, UINT32* size)
{
	//AV_audio_GetFifoSize(bMode, size);
	*size=0x1000;
}

/***************************************************************
Description:
		Write data to CHIP Fifo.

Parameters:
		bMode:	Different Fifo		1: MP3 Fifo;	2: ADPCM Fifo
		buf:	Data Buffer
		size:	Buffer size

Returns:
		NULL

****************************************************************/
static void AVWriteFifo(UINT8 bMode, UINT8* buf, UINT32 size)
{
	//AV_audio_WriteFifo(bMode, buf, size);
}

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/



void AV_Isr(void)
{
	STimer_Handler(g_avbuffer);
}


/***************************************************************
Description:
		Load  the AV file to Player, and operate the prime initialization to 
		ready for play
Parameters:
		*pbFilePtr :	the start address of music data in RAM or file handler of music file in ROM    	
		dFileSize  :	Music file size
		dSrc       :	Music source type
				0: Music data in RAM
				1: Music data in ROM

Returns:
		if  success return  RESULT_SUCCESS ,  else  ERROR

****************************************************************/
VRESULT  AVlr_Load(HUGE UINT8 *pbFilePtr, UINT32 dFileSize, UINT8 bSrc, HUGE UINT8* pIndexBuf)
{
	UINT8	bBuf[560];
	UINT8*	pbVir;
	UINT32	dFileType, dSize, dNum, dOff, i;
#if CHECK_VENDOR
	UINT32   dVender;
#endif

#if SUPPORT_FLASH
	//KVMI	info;
	VRESULT	result = VRES_SUCCESS;
#endif

	if (dState != STATE_UNLOAD)
		return VRES_ERROR;

	dState = STATE_BUSY;
	
	V5H_VMI_init();
	// initial av file buffer
	VPlr_AVFileBuf_Init(pbFilePtr,dFileSize);
	pkHeader = (KHEADER*)btempBufferHEADER;
	bSourc = bSrc;
	
	if (!bSrc)
		VIM_USER_MemCpy(bBuf, pbFilePtr, 560);
	else
	{
		VPlr_AVFileBuf_Read(bBuf,560);
	}

	dFileType = ((UINT32)bBuf[3] << 24) + ((UINT32)bBuf[2] << 16) + 
		    ((UINT32)bBuf[1] << 8) + ((UINT32)bBuf[0] << 0);

	if (dFileType == VMFFHEADER)
		VIM_USER_MemCpy((UINT8*)pkHeader, (bBuf + 4), 8);
	else if (dFileType == RIFFHEADER)
	{
		i = ((UINT32)bBuf[11] << 24) + ((UINT32)bBuf[10] << 16) + 
		    ((UINT32)bBuf[9] << 8) + ((UINT32)bBuf[8] << 0);
		if (i != AVIHEADER)
		{
			dState = STATE_ERROR;
			return VRES_ERROR;
		}	
		else
			pkHeader->dMode = 0x2;		// AVI File
	}
	else
	{
		dState = STATE_ERROR;
		return VRES_ERROR;
	}	
	
#if BIG_ENDIAN
	LittleToBigEndian(&pkHeader->dMode, 4);
#endif

	switch(pkHeader->dMode)
	{
		case MMP3_MODE:
			AVInitFunc(1);
			pkVideo = (KVIDEO*)btempBufferKVIDEO;
			pkAmp3 = (KAMP3*)btempBufferKAMP3;
			VIM_USER_MemCpy((UINT8*)pkVideo, (bBuf + 28), 64);
			VIM_USER_MemCpy((UINT8*)pkAmp3, (bBuf + 96), 56);
#if BIG_ENDIAN
			LittleToBigEndian(pkVideo, 64);
			LittleToBigEndian(pkAmp3, 56);
#endif
			pAvMp3 = (KVAVMP3*)btempBufferKVAVMP3;
			pAvMp3->dTotalTime = pkAmp3->dTotalTime;
			pAvMp3->dMp3Size = pkAmp3->dMp3Size;
			pAvMp3->dFrameLength = pkAmp3->dFrameLength;
			pAvMp3->dTotalFrame = pkAmp3->dTotalFrame;
			pAvMp3->dSampleRate = pkAmp3->dSampleRate;
			pAvMp3->dBitRate = pkAmp3->dBitrate;
			pAvMp3->dBufSize = pkVideo->dBufSize;
			// added by lizg according to Gujj [
			if((pkVideo->dPerFrame==0)||(pkVideo->dTotalFrame==0))
			{
				//printf("invalid pic frame rate/number, quit");
				dState = STATE_ERROR;
				return VRES_ERROR;
			}
			// added by lizg according to Gujj ]

			if (!bSrc)
			{
				pbDataS = pbFilePtr + 160;
			}
			else
			{
				pbFileStream_Ptr = pbFilePtr;
				// change read cursor, make sure the data in the buffer is not outdate
				// this function should be remove in the later
				VPlr_AVFileBuf_CursorSet(0, 160 + 128);
			}
				
			pAvMp3->dRealOffset = 128;
			pAvMp3->dFrameOffset = 0;
			pAvMp3->dMp3Offset = 128;
			// added by lizg
			if (pkVideo->dBufSize>sizeof(btempBufferPIC1))
			{
#if DBGAVFILE
				Uart_Printf( "Max JPG size in VMI is %d, larger than display buffer, which size is %d", pkVideo->dBufSize, sizeof(btempBufferPIC1));
#endif
				dState = STATE_ERROR;
				return VRES_ERROR;
			}
			pAvMp3->bBufFlag[0] = pAvMp3->bBufFlag[1] = 0;
			pAvMp3->pbBuffer[0] = (UINT8*)btempBufferPIC1;
			pAvMp3->pbBuffer[1] = (UINT8*)btempBufferPIC2;
			b12Flag = 0;
			break;
			
		case AVI_MODE:
#if CHECK_VENDOR
			dVender = *(UINT32*)&bBuf[0x28];
			if (dVender!=VENDORVIMI)  // VIMI
			{
				dState = STATE_ERROR;
				return VRES_ERROR;
			}	
#endif 
			// AVI file can not use read file FIFO method, for the audio data is seperated with video data
			AVInitFunc(2);
			pkAvi = &s_AVI_info;
			pkAvi->dPerFrame = ((UINT32)bBuf[35] << 24) + ((UINT32)bBuf[34] << 16) + 
		    			   ((UINT32)bBuf[33] << 8) + ((UINT32)bBuf[32] << 0);
		    	pkAvi->dTotalFrame = ((UINT32)bBuf[51] << 24) + ((UINT32)bBuf[50] << 16) + 
					     ((UINT32)bBuf[49] << 8) + ((UINT32)bBuf[48] << 0);
			pkAvi->dPerFrame /= 1000;
		    	if (pkAvi->dPerFrame == 0)	//Calculate Frame rate by using dwRate and dwScale
		    	{
		    		dNum = ((UINT32)bBuf[75] << 24) + ((UINT32)bBuf[74] << 16) + 
		    		       ((UINT32)bBuf[73] << 8) + ((UINT32)bBuf[72] << 0);
		    		dOff = ((UINT32)bBuf[79] << 24) + ((UINT32)bBuf[78] << 16) + 
		    		       ((UINT32)bBuf[77] << 8) + ((UINT32)bBuf[76] << 0);
		    		pkAvi->dPerFrame = dOff/dNum;
		    	}
		    	pkAvi->dTotalTime = pkAvi->dTotalFrame * pkAvi->dPerFrame;
		    	if (pkAvi->dTotalTime == 0)
		    	{
		    		dState = STATE_ERROR;
		    		return VRES_ERROR;
		    	}
		    	dNum = ((UINT32)bBuf[47] << 24) + ((UINT32)bBuf[46] << 16) + 
		    	       ((UINT32)bBuf[45] << 8) + ((UINT32)bBuf[44] << 0);
		    	if (dNum & 0x10)
		    		pkAvi->bIndexFlag = 1;
		    	else
		    		pkAvi->bIndexFlag = 0;
		    	if (dNum & 0x100)
		    		pkAvi->bInterFlag = 1;
		    	else
		    		pkAvi->bInterFlag = 0;
		    	pkAvi->dWidth = ((UINT32)bBuf[67] << 24) + ((UINT32)bBuf[66] << 16) + 
		    		        ((UINT32)bBuf[65] << 8) + ((UINT32)bBuf[64] << 0);
		    	pkAvi->dHeight = ((UINT32)bBuf[71] << 24) + ((UINT32)bBuf[70] << 16) + 
		    		         ((UINT32)bBuf[69] << 8) + ((UINT32)bBuf[68] << 0);
		    	dNum = ((UINT32)bBuf[95] << 24) + ((UINT32)bBuf[94] << 16) + 
		    	       ((UINT32)bBuf[93] << 8) + ((UINT32)bBuf[92] << 0);
			dOff = ((UINT32)bBuf[111] << 24) + ((UINT32)bBuf[110] << 16) + 
		    	       ((UINT32)bBuf[109] << 8) + ((UINT32)bBuf[108] << 0);
		    	dSize = 0;

			dSize = 96 + dNum;
			pbVir = bBuf + 96 + dNum;
			i = ((UINT32)pbVir[3] << 24) + ((UINT32)pbVir[2] << 16) + 
			    ((UINT32)pbVir[1] << 8) + ((UINT32)pbVir[0] << 0);
			if (i != LISTHEADER)
			{
				dState = STATE_ERROR;
		    		return VRES_ERROR;
			}
			dNum = ((UINT32)pbVir[7] << 24) + ((UINT32)pbVir[6] << 16) + 
			       ((UINT32)pbVir[5] << 8) + ((UINT32)pbVir[4] << 0);
			dSize += (8 + dNum);
			
			pbVir += 16;
			i = ((UINT32)pbVir[3] << 24) + ((UINT32)pbVir[2] << 16) + 
			    ((UINT32)pbVir[1] << 8) + ((UINT32)pbVir[0] << 0);
			pbVir += (4 + i);
			i = ((UINT32)pbVir[3] << 24) + ((UINT32)pbVir[2] << 16) + 
			    ((UINT32)pbVir[1] << 8) + ((UINT32)pbVir[0] << 0);
			if (i!= STRFHEADER)
			{
		    		return VRES_ERROR;
			}
			pbVir += 8;
			i = ((UINT32)pbVir[1] << 8) + ((UINT32)pbVir[0]);
		    	if (i != 0x11)		//IMA ADPCM
		    	{			//Maybe PCM 
		    		//V_Free(pkAvi);
		    		//return VRES_ERROR;
		    	}
		    	pkAvi->bChannel = ((UINT32)pbVir[3] << 8) + ((UINT32)pbVir[2]);
		    	pkAvi->dSampleRate = ((UINT32)pbVir[7] << 24) + ((UINT32)pbVir[6] << 16) + 
					     ((UINT32)pbVir[5] << 8) + ((UINT32)pbVir[4] << 0);
			i = ((UINT32)pbVir[13] << 8) + ((UINT32)pbVir[12] << 0);
			if (pkAvi->bChannel == 2)
				i = i >> 1;
			pkAvi->dBlockSize = (i << 1) - 1;
			
			pbVir = bBuf + dSize;
			
			i = ((UINT32)pbVir[3] << 24) + ((UINT32)pbVir[2] << 16) + 
			    ((UINT32)pbVir[1] << 8) + ((UINT32)pbVir[0] << 0);
			if (i == JUNKHEADER)
			{
				dNum =  ((UINT32)pbVir[7] << 24) + ((UINT32)pbVir[6] << 16) + 
					((UINT32)pbVir[5] << 8) + ((UINT32)pbVir[4] << 0);
				dSize += (8 + dNum);
			}
			pkAvi->dDataOffset = dSize;
			if (bSrc)		//seek to LIST 'movi'
			{
				VPlr_SeekFile(pbFilePtr, pkAvi->dDataOffset);
				VPlr_ReadFile(pbFilePtr, bBuf, 36);
				pbVir = bBuf;
			}
			else
				pbVir = pbFilePtr + pkAvi->dDataOffset;
			i = ((UINT32)pbVir[3] << 24) + ((UINT32)pbVir[2] << 16) + 
			    ((UINT32)pbVir[1] << 8) + ((UINT32)pbVir[0] << 0);
			if (i != LISTHEADER)
			{
		    		return VRES_ERROR;
			}
			dNum =  ((UINT32)pbVir[7] << 24) + ((UINT32)pbVir[6] << 16) + 
				((UINT32)pbVir[5] << 8) + ((UINT32)pbVir[4] << 0);
			i = ((UINT32)pbVir[11] << 24) + ((UINT32)pbVir[10] << 16) + 
			    ((UINT32)pbVir[9] << 8) + ((UINT32)pbVir[8] << 0);
			if (i != MOVIHEADER)
			{
				dState = STATE_ERROR;
		    		return VRES_ERROR;
			}
			if (bSrc)		// seek to index1
			{
				dNum += (pkAvi->dDataOffset + 8);
				VPlr_SeekFile(pbFilePtr, dNum);
				VPlr_ReadFile(pbFilePtr, bBuf, 8);
				pbVir = bBuf;
			}
			else
				//pbVir += pkAvi->dDataOffset + 8 + dNum;
				pbVir = pbFilePtr + pkAvi->dDataOffset + 8 + dNum;
			i = ((UINT32)pbVir[3] << 24) + ((UINT32)pbVir[2] << 16) + 
			    ((UINT32)pbVir[1] << 8) + ((UINT32)pbVir[0] << 0);
			if (i != INDXHEADER)
			{
				dState = STATE_ERROR;
		    		return VRES_ERROR;
			}
			dNum = ((UINT32)pbVir[7] << 24) + ((UINT32)pbVir[6] << 16) + 
				((UINT32)pbVir[5] << 8) + ((UINT32)pbVir[4] << 0);
			pkAvi->dIndexNum = dNum/16;
			if (bSrc)	//Read index1 data
			{
				//pkAvi->pbIndex = (UINT8*)V_Malloc(dNum);
				pkAvi->pbIndex = pIndexBuf;
				dSize = dNum/512;
				dOff = dNum%512;
				pbVir = pkAvi->pbIndex;
				for(i=0; i<dSize; i++)
				{
					VPlr_ReadFile(pbFilePtr, pbVir, 512);
					pbVir += 512;
				}
				VPlr_ReadFile(pbFilePtr, pbVir, dOff);
				pbFileStream_Ptr = pbFilePtr;
			}
			else
			{
				pkAvi->pbIndex = pbVir + 8;
			}
			pkAvi->pbAIndex = pkAvi->pbVIndex = pkAvi->pbIndex;
			if (pkAvi->pbAIndex[2] == 'w')
			{
				while(1)
				{
					pkAvi->pbVIndex += 16;
					if (pkAvi->pbVIndex[2] == 'd')
						break;
				}
			}
			else if (pkAvi->pbVIndex[2] == 'd')
			{
				while(1)
				{
					pkAvi->pbAIndex += 16;
					if (pkAvi->pbAIndex[2] == 'w')
						break;
				}
			}
			else
			{
				dState = STATE_ERROR;
				return VRES_ERROR;
			}
			pkAvi->dAIndex = pkAvi->dVIndex = 0;
			pbVir = pkAvi->pbIndex;
			dNum = dSize = 0;
			for (i=0; i<pkAvi->dIndexNum; i++)
			{
				dOff = i * 16 + 2;
				if (pbVir[dOff] == 'w')
					pkAvi->dAIndex ++;		// Number of Audio Index
				else
				{
					dNum = ((UINT32)pbVir[dOff + 13] << 24) + ((UINT32)pbVir[dOff + 12] << 16) + 
					       ((UINT32)pbVir[dOff + 11] << 8) + ((UINT32)pbVir[dOff + 10] << 0);
					if (dNum > dSize)
						dSize = dNum;
					pkAvi->dVIndex ++;		// Number of Video Index
				}
			}
			pbVir = pkAvi->pbIndex + 8;
			dNum = ((UINT32)pbVir[3] << 24) + ((UINT32)pbVir[2] << 16) + 
			       ((UINT32)pbVir[1] << 8) + ((UINT32)pbVir[0] << 0);
			if (dNum <= pkAvi->dDataOffset)
				pkAvi->bOffsetFlag = 1;
			else
				pkAvi->bOffsetFlag = 0;
			pkAvi->dJpegSize[0] = pkAvi->dJpegSize[1] = 0;
			pkAvi->dAIndexc = pkAvi->dVIndexc = 0;
			pkAvi->dChunkOffset = 0;
			pkAvi->pbSptr = pbFilePtr;
			pkAvi->bSource = bSrc;
			break;
			
#if SUPPORT_FLASH
		case MMIDI_MODE:
			pkVideo = (KVIDEO*)V_Malloc(sizeof(KVIDEO));
			pkAmidi = (KAMIDI*)V_Malloc(sizeof(KAMIDI));
			VIM_USER_MemCpy((UINT8*)pkVideo, (bBuf + 28), 64);
			VIM_USER_MemCpy((UINT8*)pkAmidi, (bBuf + 96), 56);
			pAvMidi = (KVAVMIDI*)V_Malloc(sizeof(KVAVMIDI));
			pAvMidi->dBufSize = pkVideo->dBufSize;
			//info.bMode = 1;
			//AV_audio_init(info, ReadAVBuffer, STimer_Handler);
			if (!bSrc)
			{
				VIM_USER_MemCpy((UINT8*)bBuf, (pbFilePtr + 160), 8);
				dSize = ((UINT32)bBuf[7] << 24) + ((UINT32)bBuf[6] << 16) + 
		    			((UINT32)bBuf[5] << 8) + ((UINT32)bBuf[4] << 0);
				LoadMusic(0, (pbFilePtr + 168), dSize, bSrc);
				dDataS = dSize;
			}
			else
			{
				pbFileStream_Ptr = pbFilePtr;
				VPlr_ReadFile(pbFilePtr, bBuf, 8);
				dSize = ((UINT32)bBuf[7] << 24) + ((UINT32)bBuf[6] << 16) + 
		    			((UINT32)bBuf[5] << 8) + ((UINT32)bBuf[4] << 0);
				VPlr_SeekFile((void*)pbFilePtr, 168);
				pbMidiS = (HUGE UINT8*)V_Malloc(sizeof(char) * dSize);
				dNum = dFileSize/128;
				dOff = dFileSize%128;
				pbVir = pbMidiS;
				for(i=0; i<dNum; i++)
				{
					VPlr_ReadFile((void*)pbFilePtr, pbVir, 128);
					pbVir += 128;
				}
				VPlr_ReadFile((void*)pbFilePtr, pbVir, dOff);
				LoadMusic(0, pbMidiS, dSize, 0);
				dDataS = dSize;
			}
			if (result != VRES_SUCCESS)
			{
				pbFileStream_Ptr = NULL;
				if (pbMidiS != NULL)
				{
					V_Free(pbMidiS);
					pbMidiS = NULL;
				}
				V_Free(pAvMidi);
				V_Free(pkAmidi);
				V_Free(pkVideo);
				V_Free(pkHeader);
				
				return VRES_ERROR;
			}
			if (bSrc)	// In ROM
			{
				VPlr_ReadFile((void*)pbFilePtr, bBuf, 4);	// Read "JPEG" Label
				VPlr_ReadFile((void*)pbFilePtr, bBuf, 4);
				pAvMidi->dSize = (UINT32)(bBuf[3] << 24) + (UINT32)(bBuf[2] << 16) 
			               		 + (UINT32)(bBuf[1] << 8) + (UINT32)(bBuf[0] << 0);
				pAvMidi->pbJpegPtr = pbFilePtr;
			}
			else		// In RAM
			{
				pAvMidi->pbJpegPtr = pbFilePtr + 168 + dDataS + 4;
				pAvMidi->dSize = (UINT32)(pAvMidi->pbJpegPtr[3] << 24) + (UINT32)(pAvMidi->pbJpegPtr[2] << 16) 
			               		 + (UINT32)(pAvMidi->pbJpegPtr[1] << 8) + (UINT32)(pAvMidi->pbJpegPtr[0] << 0);
				pAvMidi->pbJpegPtr += 4;
				pAvMidi->pbSrcPtr = pAvMidi->pbJpegPtr;
				pAvMidi->pbSrcPtr = pAvMidi->pbJpegPtr;
			}
			pAvMidi->dOffset = 0;
			pAvMidi->bBufFlag[0] = pAvMidi->bBufFlag[1] = 0;
			pAvMidi->pbBuffer[0] = (UINT8*)V_Malloc(sizeof(char) * (pAvMidi->dBufSize));
			pAvMidi->pbBuffer[1] = (UINT8*)V_Malloc(sizeof(char) * (pAvMidi->dBufSize));
			b12Flag = 0;
			break;
#endif			
	
		default:
			dState = STATE_ERROR;
			return VRES_ERROR;
			break;
	}

	dState = STATE_LOAD;
	
	return  VRES_SUCCESS;
}


/***************************************************************
Description:
		Unload  the AV File

Parameters:
		void

Returns:
		if  success return  RESULT_SUCCESS ,  else  VRES_VPLRUNLOAD_ERROR

****************************************************************/
VRESULT  AVlr_Unload(void)
{
#if SUPPORT_FLASH
	VRESULT result = VRES_SUCCESS;
#endif

	if (!((dState == STATE_END)|(dState == STATE_ERROR)))
		return VRES_ERROR;

	V5H_VMI_End();
	switch(pkHeader->dMode)
	{
		case MMP3_MODE:
			pAvMp3->bBufFlag[0] = pAvMp3->bBufFlag[1] = 0;
			break;
			
#if SUPPORT_FLASH
		case MMIDI_MODE:
			//AV_audio_unload(dFuncId);
			if (result == VRES_SUCCESS)
			{
				pAvMidi->bBufFlag[0] = pAvMidi->bBufFlag[1] = 0;
				V_Free(pAvMidi->pbBuffer[1]);
				V_Free(pAvMidi->pbBuffer[0]);
				
				if (pbMidiS != NULL)
				{
					V_Free(pbMidiS);
				}
				V_Free(pAvMidi);
				V_Free(pkAmidi);
				V_Free(pkVideo);
			}
			else
				return VRES_ERROR;
			break;
#endif	

		case AVI_MODE:
			break;
			
		default:
			break;
	}
	
	pbFileStream_Ptr = NULL;
	pbMidiS = NULL;
	dState = STATE_UNLOAD;
	
	return  VRES_SUCCESS;
}



/***************************************************************
Description:
		Play AV File

Parameters:
		dSeekPos :	the position of music , the unit is millisecond	
		dRepeat	:	Number of playback
		pFunc	:	Call Back function that will be called at the end of playing music

Returns:
		if  success return  RESULT_SUCCESS ,  else  ERROR

****************************************************************/
VRESULT  AVlr_Play(UINT32 dSeekPos, UINT32 dRepeat, PEFUNCTION pFunc)
{
	UINT32	dTm;
	
	if (dState != STATE_LOAD)
		return VRES_ERROR;
	
	dState = STATE_BUSY;
	switch(pkHeader->dMode)
	{
		case MMP3_MODE:
			g_bFrameFlag=0;
			if (pbFileStream_Ptr == NULL)
				StartChannel(1, 0);
			else
			{
				StartChannel(1, 2);
				//Vdev_SetTimer(1,0xffffffff);
				//Vdev_StartTimer(1);
				VIM_USER_Caculate(0);
			}
				
			dTm = 1000/(pkVideo->dPerFrame);
			pAvMp3->dPauseTime = 0;
			if (pFunc != NULL)
				pAvfunction = pFunc;
			pAvMp3->dRepeat = dRepeat;
			bTicker = dTm;
			g_SkipCount=0;
			g_bReSynFlag = 0;
			dTimeBase=0;
			AVStartTimer(bTicker);
			break;
			
		case AVI_MODE:
			if(pkAvi->dAIndex>0)  // video with audio
			{
				if (pbFileStream_Ptr == NULL)
					StartChannel(3, 0);
				else
					StartChannel(3, 1);
				// adpcm data size = 0x3b00, sent in StartChannel
				g_Time.dwAudio= (0x3b00*2*1000)/(pkAvi->dSampleRate* pkAvi->bChannel);
				//g_Time.dwAudio= TIME_DIFF_VIDEO_AUDIO;
			}

			// Start long timer used for playback time count
			//Vdev_SetTimer(1,0xffffffff);
			//Vdev_StartTimer(1);
			VIM_USER_Caculate(0);
			AVStartTimer(pkAvi->dPerFrame);
			// initial time counter
			if(pkAvi->dAIndex==0)
				g_Time.dwBase=0;
			else
				g_Time.dwBase=TIME_START_POINT_ADJUST;
			g_Time.dwSystem=0;
			// initial stop callback function
			if (pFunc != NULL) // stop call this function
				pAvfunction = pFunc;
			break;
			
#if SUPPORT_FLASH
		case MMIDI_MODE:
			StartPlayMusic(0, dRepeat, pFunc);
			bTicker = 50;
			AVStartTimer(50);
			break;
#endif	
		default:
			dState = STATE_ERROR;
			return VRES_ERROR;
			break;
	}
	
	bAVEnd = 0;
	dTimeNow = 0;
	dTimeBase = 0;
	dState = STATE_PLAY;
	
	return VRES_SUCCESS;
}


/***************************************************************
Description:
		Stop  Playing AV File   

Parameters:
		void
		
Returns:
		if  success return  RESULT_SUCCESS ,  else  ERROR

****************************************************************/
VRESULT AVlr_Stop(void)
{
	if (!((dState == STATE_PLAY) | (dState == STATE_PAUSE) | (dState == STATE_ERROR)))
		return VRES_ERROR;
	// Open PA for exit in pause mode
	AVOpenPA();
	AVStopTimer();
	VIM_USER_Caculate(1);
	switch(pkHeader->dMode)
	{
		case MMP3_MODE:
			StopChannel(1);
			break;
			
#if SUPPORT_FLASH
		case MMIDI_MODE:
			StopPlayMusic();
			if (pbMidiS != NULL)
				VPlr_SeekFile((void*)(pbFileStream_Ptr), (dDataS + 43 * 4 + 4));
			else
				pAvMidi->pbJpegPtr = pAvMidi->pbSrcPtr;
			pAvMidi->dOffset = 0;
			pAvMidi->bBufFlag[0] = pAvMidi->bBufFlag[1] = 0;
			b12Flag = 0;
			dTimeNow = 0;
			dTimeBase = 0;
			break;
#endif			

		case AVI_MODE:
			StopChannel(2);
			break;
		default:
			return VRES_ERROR;
	}
	
	dTimeNow = 0;
	dTimeBase = 0;
	dState = STATE_END;

	return VRES_SUCCESS;
}


/***************************************************************
Description:
		Pause  Playing AV File   

Parameters:
		void
		
Returns:
		if  success return  RESULT_SUCCESS ,  else  ERROR

****************************************************************/
VRESULT AVlr_Pause(void)
{
	if (dState != STATE_PLAY)
		return VRES_ERROR;

	switch(pkHeader->dMode)
	{
		case MMP3_MODE:
			pAvMp3->dPauseTime = AVlr_GetCurTime();
			AVStopTimer();
			VIM_USER_Caculate(1);
			AVClosePA();
			break;
			
		case MMIDI_MODE:
			break;
			
		case AVI_MODE:
			if (pkAvi->dAIndex==0)  // video without audio, just stop timer
			{
				// Stop long timer, the long timer will be reset when stop
				g_Time.dwBase += VIM_USER_GetMs();
				VIM_USER_Caculate(1);
				AVStopTimer();
			}	
			break;
		default:
			return VRES_ERROR;
	}
	
	dState = STATE_PAUSE;

	return VRES_SUCCESS;
}


/***************************************************************
Description:
		Resume  Playing AV File

Parameters:
		void
		
Returns:
		if  success return  RESULT_SUCCESS ,  else  ERROR

****************************************************************/
VRESULT AVlr_Resume(void)
{
	if (dState != STATE_PAUSE)
		return VRES_ERROR;
	
	switch(pkHeader->dMode)
	{
		case MMP3_MODE:
			//Vdev_SetTimer(1,0xffffffff);
			VIM_USER_Caculate(0);
			dTimeBase = pAvMp3->dPauseTime; 
			AVStartTimer(bTicker);
			AVOpenPA();
			break;
			
		case MMIDI_MODE:
			break;
			
		case AVI_MODE:
			// start long timer
			if(pkAvi->dAIndex>0)  // with audio
				AVOpenPA();
			//Vdev_SetTimer(1,0xffffffff);
			VIM_USER_Caculate(0);
			AVStartTimer(pkAvi->dPerFrame);

			break;
		default:
			return VRES_ERROR;
	}
	
	dState = STATE_PLAY;

	return VRES_SUCCESS;
}


/***************************************************************
Description:
		Get AV File Total Time 

Parameters:
		void
		
Returns:
		Total Time(ms)

****************************************************************/
UINT32 AVlr_GetTotalTime(void)
{
	UINT32	dTotalTime;	
	
	if (dState == STATE_UNLOAD)
		return VRES_ERROR;

	switch(pkHeader->dMode)
	{
		case MMP3_MODE:
			dTotalTime = pkAmp3->dTotalTime;
			break;
		case MMIDI_MODE:
			dTotalTime = pkAmidi->dTotalTime;
			break;
		case AVI_MODE:
			dTotalTime = pkAvi->dTotalTime;
			break;
		default:
			return 0;
	}
	
	
	return dTotalTime;
}


/***************************************************************
Description:
		Get AV File Current Time 

Parameters:
		void
		
Returns:
		Total Time(ms)

****************************************************************/
UINT32 AVlr_GetCurTime(void)
{
	UINT32	dCurTime;

	if ((dState != STATE_PAUSE) && (dState != STATE_PLAY))
		return VRES_ERROR;

	switch(pkHeader->dMode)
	{
	case MMP3_MODE:
		if (dState == STATE_PLAY)
			dCurTime = dTimeNow;
		else
			dCurTime = pAvMp3->dPauseTime;
		break;

	case MMIDI_MODE:
		break;
		
	case AVI_MODE:
		// by lizg for AVI player
		dCurTime= g_Time.dwBase+ VIM_USER_GetMs();
		break;

	default:
		break;
	}	
	return dCurTime;
}


/***************************************************************
Description:
		Read Avi File from ROM or RAM

Parameters:
		mode	   : 0: AV File In RAM; 1: AV File In ROM
		bBuffer	   : buffer address
		dBufferSize : number bytes to be read
		pdRealSize  : number bytes read in fact
		dOffset	   : file offset

Returns:
		  NULL
		  
****************************************************************/
void ReadAviBuffer(UINT32 mode, UINT8* bBuffer, UINT32 dBufferSize, UINT32* pdRealSize, UINT32 dOffset)
{
	UINT32	dNum, dOff, dTemp, dTempSize = 0;
	UINT8*	pSptr;
	UINT8*	pDptr;
	
	if (pkAvi->dAIndexc >= pkAvi->dAIndex)
	{
		*pdRealSize = 0;
		return;
	}

	pDptr = bBuffer;
	if (mode == 0)		//AV File In RAM
	{
		if(pkAvi->dChunkOffset)
		{
			dTemp = ((UINT32)pkAvi->pbAIndex[11] << 24) + ((UINT32)pkAvi->pbAIndex[10] << 16) + 
				((UINT32)pkAvi->pbAIndex[9] << 8) + ((UINT32)pkAvi->pbAIndex[8] << 0);
			dNum =  ((UINT32)pkAvi->pbAIndex[15] << 24) + ((UINT32)pkAvi->pbAIndex[14] << 16) + 
				((UINT32)pkAvi->pbAIndex[13] << 8) + ((UINT32)pkAvi->pbAIndex[12] << 0);
			if (pkAvi->bOffsetFlag)
				dTemp += (pkAvi->dDataOffset + 8 + 8);
			pSptr = pkAvi->pbSptr + dTemp + (dNum - pkAvi->dChunkOffset); 
			if(dBufferSize < pkAvi->dChunkOffset)
			{
				VIM_USER_MemCpy((HUGE UINT8*)pDptr, (HUGE UINT8*)pSptr, dBufferSize);
				pkAvi->dChunkOffset -= dBufferSize;
				*pdRealSize = dBufferSize;
				return;
			}
			else
			{
				VIM_USER_MemCpy((HUGE UINT8*)pDptr, (HUGE UINT8*)pSptr, pkAvi->dChunkOffset);
				pDptr += pkAvi->dChunkOffset;
				dTempSize = pkAvi->dChunkOffset;
				pkAvi->dChunkOffset = 0;
				pkAvi->dAIndexc ++;
				if (pkAvi->dAIndexc >= pkAvi->dAIndex)
				{
					*pdRealSize = dTempSize;
					return;
				}
				else
				{
					pkAvi->pbAIndex += 16;	
					while(1)				// Find Next Audio Index
					{
						if (pkAvi->pbAIndex[2] == 'w')
							break;
						else
							pkAvi->pbAIndex += 16;
					}
				}
			}
		}
		while(1)
		{
			if (pkAvi->dAIndexc >= pkAvi->dAIndex)
			{
				*pdRealSize = dTempSize;
				return;
			}
			
			dTemp = dBufferSize - dTempSize;
			dNum = ((UINT32)pkAvi->pbAIndex[15] << 24) + ((UINT32)pkAvi->pbAIndex[14] << 16) + 
			       ((UINT32)pkAvi->pbAIndex[13] << 8) + ((UINT32)pkAvi->pbAIndex[12] << 0);
			dOff =  ((UINT32)pkAvi->pbAIndex[11] << 24) + ((UINT32)pkAvi->pbAIndex[10] << 16) + 
				((UINT32)pkAvi->pbAIndex[9] << 8) + ((UINT32)pkAvi->pbAIndex[8] << 0);
			if (pkAvi->bOffsetFlag)
			{
				dOff += (pkAvi->dDataOffset + 8);
				// Fix me //
				dOff += 8;
				// Fix me //
			}
			pSptr = pkAvi->pbSptr + dOff;
			if (dTemp < dNum)
			{
				VIM_USER_MemCpy((HUGE UINT8*)pDptr, (HUGE UINT8*)pSptr, dTemp);
				pkAvi->dChunkOffset = dNum - dTemp;
				*pdRealSize = dTempSize + dTemp;
				return;
			}
			else
			{
				VIM_USER_MemCpy((HUGE UINT8*)pDptr, (HUGE UINT8*)pSptr, dNum);
				dTempSize += dNum;
				pDptr += dNum;
				pkAvi->dAIndexc ++;
				if (pkAvi->dAIndexc >= pkAvi->dAIndex)
				{
					*pdRealSize = dTempSize;
					return;
				}
				else
				{
					pkAvi->pbAIndex += 16;	
					while(1)				// Find Next Audio Index
					{
						if (pkAvi->pbAIndex[2] == 'w')
							break;
						else
							pkAvi->pbAIndex += 16;
					}
				}
			}
		}
	}
	else			//AV File In ROM
	{
		if(pkAvi->dChunkOffset)
		{
			dTemp = ((UINT32)pkAvi->pbAIndex[11] << 24) + ((UINT32)pkAvi->pbAIndex[10] << 16) + 
				((UINT32)pkAvi->pbAIndex[9] << 8) + ((UINT32)pkAvi->pbAIndex[8] << 0);
			dNum =  ((UINT32)pkAvi->pbAIndex[15] << 24) + ((UINT32)pkAvi->pbAIndex[14] << 16) + 
				((UINT32)pkAvi->pbAIndex[13] << 8) + ((UINT32)pkAvi->pbAIndex[12] << 0);
			if (pkAvi->bOffsetFlag)
				dTemp += (pkAvi->dDataOffset + 8 + 8);
			dTemp += (dNum - pkAvi->dChunkOffset);
			VPlr_SeekFile((void*)(pkAvi->pbSptr), dTemp);
			//pSptr = pkAvi->pbSptr + dTemp + (dNum - pkAvi->dChunkOffset); 
			if(dBufferSize < pkAvi->dChunkOffset)
			{
				VPlr_ReadFile((void*)(pkAvi->pbSptr), pDptr, dBufferSize);
				//Vmd_MemCpy((HUGE UINT8*)pDptr, (HUGE UINT8*)pSptr, dBufferSize);
				pkAvi->dChunkOffset -= dBufferSize;
				*pdRealSize = dBufferSize;
				return;
			}
			else
			{
				VPlr_ReadFile((void*)(pkAvi->pbSptr), pDptr, pkAvi->dChunkOffset);
				//Vmd_MemCpy((HUGE UINT8*)pDptr, (HUGE UINT8*)pSptr, pkAvi->dChunkOffset);
				pDptr += pkAvi->dChunkOffset;
				dTempSize = pkAvi->dChunkOffset;
				pkAvi->dChunkOffset = 0;
				pkAvi->dAIndexc ++;
				if (pkAvi->dAIndexc >= pkAvi->dAIndex)
				{
					*pdRealSize = dTempSize;
					return;
				}
				else
				{
					pkAvi->pbAIndex += 16;	
					while(1)				// Find Next Audio Index
					{
						if (pkAvi->pbAIndex[2] == 'w')
							break;
						else
							pkAvi->pbAIndex += 16;
					}
				}
			}
		}
		while(1)
		{
			if (pkAvi->dAIndexc >= pkAvi->dAIndex)
			{
				*pdRealSize = dTempSize;
				return;
			}
			
			dTemp = dBufferSize - dTempSize;
			dNum = ((UINT32)pkAvi->pbAIndex[15] << 24) + ((UINT32)pkAvi->pbAIndex[14] << 16) + 
			       ((UINT32)pkAvi->pbAIndex[13] << 8) + ((UINT32)pkAvi->pbAIndex[12] << 0);
			dOff =  ((UINT32)pkAvi->pbAIndex[11] << 24) + ((UINT32)pkAvi->pbAIndex[10] << 16) + 
				((UINT32)pkAvi->pbAIndex[9] << 8) + ((UINT32)pkAvi->pbAIndex[8] << 0);
			if (pkAvi->bOffsetFlag)
			{
				dOff += (pkAvi->dDataOffset + 8);
				// Fix me //
				dOff += 8;
				// Fix me //
			}
			VPlr_SeekFile((void*)(pkAvi->pbSptr), dOff);
			//pSptr = pkAvi->pbSptr + dOff;
			if (dTemp < dNum)
			{
				VPlr_ReadFile((void*)(pkAvi->pbSptr), pDptr, dTemp);
				//Vmd_MemCpy((HUGE UINT8*)pDptr, (HUGE UINT8*)pSptr, dTemp);
				pkAvi->dChunkOffset = dNum - dTemp;
				*pdRealSize = dTempSize + dTemp;
				return;
			}
			else
			{
				VPlr_ReadFile((void*)(pkAvi->pbSptr), pDptr, dNum);
				//Vmd_MemCpy((HUGE UINT8*)pDptr, (HUGE UINT8*)pSptr, dNum);
				dTempSize += dNum;
				pDptr += dNum;
				pkAvi->dAIndexc ++;
				if (pkAvi->dAIndexc >= pkAvi->dAIndex)
				{
					*pdRealSize = dTempSize;
					return;
				}
				else
				{
					pkAvi->pbAIndex += 16;	
					while(1)				// Find Next Audio Index
					{
						if (pkAvi->pbAIndex[2] == 'w')
							break;
						else
							pkAvi->pbAIndex += 16;
					}
				}
			}
		}
	}
}


/***************************************************************
Description:
		Read AV File from ROM or RAM

Parameters:
		mode	   : 0: AV File In RAM; 1: AV File In ROM
		bBuffer	   : buffer address
		dBufferSize : number bytes to be read
		pdRealSize  : number bytes read in fact
		dOffset	   : file offset

Returns:
		  NULL
		  
****************************************************************/
void ReadAVBuffer(UINT32 mode, UINT8* bBuffer, UINT32 dBufferSize, UINT32* pdRealSize, UINT32 dOffset)
{
	UINT32	FrameLen;
	UINT8	bStopFlag;
	UINT8*	pMp3Dptr, *pJPGDptr;
	UINT32* pJpgSize;
	UINT32* pJpgTicker;
	SINT32   ret;
	UINT8 	bData1,bDataBuf[10];
	UINT8	bCheckMp3Frame, bCheckJpgFrame;
	SINT32	dTimeout;

	// tow buffer full, no need read data
	if ((pAvMp3->bBufFlag[0] == 1)&&(pAvMp3->bBufFlag[1] == 1))
	{
		return;
	}	

	// mode 2, use the file read ring buffer
	if(mode==2)
	{
		bStopFlag = 0;
		// mp3 data buffer poiter
		pMp3Dptr=bBuffer;
		*pdRealSize = 0;
		while(!bStopFlag)
		{
			if (g_bFrameFlag==0)
			{
				// Get frame flag
				ret = VPlr_AVFileBuf_Read(&g_bFrameFlag, 1);
				// End of file, exit
				if (ret== -1)
				{
					// not support repeat play, it has some problems
					bAVEnd = 1;
					return;
				}	
			}
			if(g_bFrameFlag==1) // MP3 data
			{
				// get mp3 format, 4 bytes
				VPlr_AVFileBuf_Read(pMp3Dptr, 4);				
				if ((pMp3Dptr[2]&0x02)==0)
				{
					// no padding,
					FrameLen = pAvMp3->dFrameLength ;
				}
				else
				{
					// padding one byte
					FrameLen = pAvMp3->dFrameLength +1;
				}	
				// get one frame mp3 data
				VPlr_AVFileBuf_Read(pMp3Dptr+4, FrameLen-4);	
				pMp3Dptr += FrameLen;
				(*pdRealSize) += FrameLen;
				// mp3 buffer full, exit
				if(dBufferSize-(*pdRealSize)< pAvMp3->dFrameLength+1)
				{
					bStopFlag=1;
					continue;
				}	
				g_bFrameFlag=0;
			}
			else if(g_bFrameFlag==2) // JPEG data
			{
				// tow buffer full, set stop flag
				if ((pAvMp3->bBufFlag[0] == 1)&&(pAvMp3->bBufFlag[1] == 1))
				{
					bStopFlag = 1;
					continue;
				}
				// set buffer pointer
				if(pAvMp3->bBufFlag[0] == 0)
				{
					pJPGDptr = pAvMp3->pbBuffer[0];
					pJpgSize = &pAvMp3->dBufferSize[0];
					pJpgTicker=&pAvMp3->dTicker[0];
					pAvMp3->bBufFlag[0] = 1;
				}
				else if (pAvMp3->bBufFlag[1] == 0)
				{
					pJPGDptr = pAvMp3->pbBuffer[1];
					pJpgSize = &pAvMp3->dBufferSize[1];
					pJpgTicker=&pAvMp3->dTicker[1];
					pAvMp3->bBufFlag[1] = 1;
				}
				
				// get jpeg ticker
				VPlr_AVFileBuf_Read((UINT8*)pJpgTicker, 4);
#if BIG_ENDIAN
				LittleToBigEndian(pJpgTicker, 4);
#endif
				if(*pJpgTicker>0x2932E00)	//12*60*60*1000, 12 hours
				{
					// error data, continue next loop
					g_bFrameFlag=0;
					continue;
				}	
				// get jpeg format, 18 bytes
				VPlr_AVFileBuf_Read(pJPGDptr, 18);
				// get jpeg file size
				*pJpgSize = ((UINT32)pJPGDptr[17]<<24)+((UINT32)pJPGDptr[16]<<16)+((UINT32)pJPGDptr[15]<<8)+pJPGDptr[14];
				if(*pJpgSize>0x5000)	//20K
				{
					// error data, continue next loop
					g_bFrameFlag=0;
					continue;
				}	
				pJPGDptr+=18;
				VPlr_AVFileBuf_Read(pJPGDptr, *pJpgSize -18);
				g_bFrameFlag=0;
			}
			else // error data
			{
				UINT32 time1, time2;
				dTimeout = 0x8000; // 32K, max time about 180ms 
				g_bReSynFlag=0;
				// fill file FIFO
				VPlr_AVFileBuf_Fill(g_AVFileBufInfo.pFile_Ptr, g_AVFileBufInfo.filelength);
#if DBGAVFILE
				VIM_USER_PrintString("error data, try to get next frame\n");
#endif
				time1 = VIM_USER_GetMs();
				while(dTimeout>0)
				{
					// step 1, check frame flag
					ret = VPlr_AVFileBuf_Read(&bData1, 1);
					// End of file, exit
					if (ret== -1)
					{
						bAVEnd = 1;
						return;
					}
					dTimeout--;
					if(bData1==1)
					{
						bCheckMp3Frame=1;
					}
					else if (bData1==2)
					{
						bCheckJpgFrame=1;
					}
					else
						continue;
					
					// step 2, check second byte
					if(bCheckMp3Frame==1)
					{
						ret = VPlr_AVFileBuf_Read(bDataBuf, 2);
						// End of file, exit
						if (ret== -1)
						{
							bAVEnd = 1;
							return;
						}
						dTimeout-=2;

						if((bDataBuf[0]==0xff)&&(bDataBuf[1]&&0xe0==0xe0))
						{
							bCheckMp3Frame=2;
							break;
						}
						else
						{
							bCheckMp3Frame=0;
							continue;
						}	
					}
					
					if(bCheckJpgFrame==1)
					{
						ret = VPlr_AVFileBuf_Read(bDataBuf, 6);
						// End of file, exit
						if (ret== -1)
						{
							bAVEnd = 1;
							return;
						}
						dTimeout-=6;

						if((bDataBuf[4]==0xff)&&(bDataBuf[5]==0xd8))
						{
							bCheckJpgFrame=2;
							break;
						}
						else
						{
							bCheckJpgFrame=0;
							continue;
						}	
					}
				}

				if (dTimeout<=0)
				{
					bAVEnd = 1;
					return;
				}
				
				// step 3, get frame data
				if(bCheckMp3Frame==2)
				{
					// get mp3 format, 4 bytes
					pMp3Dptr[0]=bDataBuf[0];
					pMp3Dptr[1]=bDataBuf[1];
					VPlr_AVFileBuf_Read(pMp3Dptr+2, 2);				
					if ((pMp3Dptr[2]&0x02)==0)
					{
						// no padding,
						FrameLen = pAvMp3->dFrameLength ;
					}
					else
					{
						// padding one byte
						FrameLen = pAvMp3->dFrameLength +1;
					}	
					// get one frame mp3 data
					VPlr_AVFileBuf_Read(pMp3Dptr+4, FrameLen-4);	
					pMp3Dptr += FrameLen;
					(*pdRealSize) += FrameLen;
					g_bFrameFlag=0;
					// mp3 buffer full, exit
					if(dBufferSize-(*pdRealSize)< pAvMp3->dFrameLength+1)
					{
						bStopFlag=1;
						continue;
					}	
				}
				if(bCheckJpgFrame==2)
				{
					// set buffer pointer
					if(pAvMp3->bBufFlag[0] == 0)
					{
						pJPGDptr = pAvMp3->pbBuffer[0];
						pJpgSize = &pAvMp3->dBufferSize[0];
						pJpgTicker=&pAvMp3->dTicker[0];
						pAvMp3->bBufFlag[0] = 1;
					}
					else if (pAvMp3->bBufFlag[1] == 0)
					{
						pJPGDptr = pAvMp3->pbBuffer[1];
						pJpgSize = &pAvMp3->dBufferSize[1];
						pJpgTicker=&pAvMp3->dTicker[1];
						pAvMp3->bBufFlag[1] = 1;
					}
					// tow buffer full, set stop flag
					if ((pAvMp3->bBufFlag[0] == 1)&&(pAvMp3->bBufFlag[1] == 1))
						bStopFlag = 1;
					
					// get jpeg ticker
					*pJpgTicker=bDataBuf[0]+((UINT32)bDataBuf[1]<<8)+((UINT32)bDataBuf[2]<<16)+((UINT32)bDataBuf[3]<<24);
					if(*pJpgTicker>0x2932E00)	//12*60*60*1000, 12 hours
					{
						// error data, continue next loop
						continue;
					}	

					// get jpeg format, 18 bytes
					pJPGDptr[0]=bDataBuf[4];
					pJPGDptr[1]=bDataBuf[5];
					VPlr_AVFileBuf_Read(pJPGDptr+2, 16);
					// get jpeg file size
					*pJpgSize = ((UINT32)pJPGDptr[17]<<24)+((UINT32)pJPGDptr[16]<<16)+((UINT32)pJPGDptr[15]<<8)+pJPGDptr[14];
					if(*pJpgSize>0x5000)	//20K
					{
						// error data, continue next loop
						continue;
					}	
					pJPGDptr+=18;
					VPlr_AVFileBuf_Read(pJPGDptr, *pJpgSize -18);
					g_bFrameFlag=0;
				}
				time2 = VIM_USER_GetMs();
#if DBGAVFILE
				Uart_Printf("error recover time=%d, dTimeout=%d\n",time2-time1, dTimeout);
#endif
			}
		}
	}
}	

UINT8 AviGetJpgData(SINT32 FrameNum)
{
	UINT32	dOffset, dSize;
	UINT8*	pSptr;
	UINT8*	pDptr, *pIndex;
	
	if (FrameNum >= (pkAvi->dVIndex))		// End
		return 1;
	// get index address
	pIndex = pkAvi->pbVIndex + 16*FrameNum;

	pSptr = pIndex+8;
	// offset from data
	dOffset = ((UINT32)pSptr[3]<<24)+((UINT32)pSptr[2]<<16)+((UINT32)pSptr[1]<<8)+(UINT32)pSptr[0];
	// offset in file
	dOffset += (pkAvi->dDataOffset + 8);
	// get JPEG size
	pSptr = pIndex+12;
	dSize = ((UINT32)pSptr[3]<<24)+((UINT32)pSptr[2]<<16)+((UINT32)pSptr[1]<<8)+(UINT32)pSptr[0];
	// read JPG data
	if (pkAvi->bSource)			// ROM
	{
		pDptr = btempBufferPIC1;
		//pDptr = g_AVDisJPGBuf;	
		VPlr_SeekFile((void*)(pkAvi->pbSptr), dOffset + 8);
		VPlr_ReadFile((void*)(pkAvi->pbSptr), pDptr, dSize);
		//pkAvi->dJpegSize[1] = dSize;
		g_AVDisJPGSize = dSize;
	}
	return 0;
}
int g_StopAVIErrorSign=0;

/***************************************************************
Description:
		Short Timer Handler

Parameters:
		None

Returns:
		  Return VRES_SUCCESS if success, else return error value.

****************************************************************/
void STimer_Handler(UINT8* pbBuffer)
{
	//UINT16 wReadPtr, wWritePtr;
	UINT32 wNum;	//, wTemp;
	UINT32 dDataN;
	SINT32 dTimeDiff;
	SINT32 FrameNum;


	// Adjust click used for syschronize audio and video, only used in VMI playback mode
	if(pkHeader->dMode == MMP3_MODE)
	{
		if(g_bReSynFlag<8)
		{

			if (pAvMp3->bBufFlag[b12Flag] == 1)
			{
				dTimeBase= pAvMp3->dTicker[b12Flag];
				VIM_USER_Caculate(0);
				g_bReSynFlag++;
			}
		}
	}
	// get current time
	dTimeNow = dTimeBase + VIM_USER_GetMs();

	if (bAVEnd)
	{
		dState=STATE_END;
		return;
	}
	
	switch(pkHeader->dMode)
	{
	case MMP3_MODE:
		// Read data into AV file buffer from SD card or Nand
		VPlr_AVFileBuf_Fill(g_AVFileBufInfo.pFile_Ptr, g_AVFileBufInfo.filelength);

		if(pAvMp3->bBufFlag[b12Flag] == 1)
		{
			// get the difference between current time and video ticker
			dTimeDiff=dTimeNow-pAvMp3->dTicker[b12Flag];
			if (((dTimeDiff>-bTicker)&&(dTimeDiff<bTicker)) |(g_SkipCount>3))  
			{
				// display video data
				AVDisplayJpeg(pAvMp3->pbBuffer[b12Flag], pAvMp3->dBufferSize[b12Flag]);
				g_SkipCount=0;
				pAvMp3->bBufFlag[b12Flag] = 0;
				b12Flag = !b12Flag;
			}
			else if(dTimeDiff>=bTicker)
			{
				//video out of date, discard it
				pAvMp3->bBufFlag[b12Flag] = 0;
				b12Flag = !b12Flag;
				g_SkipCount++;
			}
			else if(dTimeDiff<=-bTicker)
			{
				//video is ahead of audio, display next time
#if DBGAVFILE
		VIM_USER_PrintString( "video is ahead of audio, display next time");
#endif	
				return;
			}
		}

		AVGetFifoSize(1, &wNum);
		if (wNum >= (pAvMp3->dFrameLength * 2))
		{
#if DBGAVFILE
		VIM_USER_PrintDec( "wNum=",wNum);
#endif		
			if(wNum>8192)
			{
				//Get Fifo size maybe return error, clip it
				wNum = 8192;
			}
			// ReadAVBuffer will cost long time when read error data, so set busy satus flag
			dState = STATE_BUSY;
			ReadAVBuffer(2, pbBuffer, wNum, &dDataN, 0);
			dState = STATE_PLAY;
			//bAVEnd  is set in ReadAVBuffer, when read to the end of file
			if(bAVEnd)
			{
				// stopped 
				g_StopAVIErrorSign=1;
				AVlr_Stop();
				AVlr_Unload();
#if DBGAVFILE
				Uart_Printf("VMI file ended!\r\n");
#endif
				if (pAvfunction != NULL)
					pAvfunction();
				return;
			}
			AVWriteFifo(1, pbBuffer, dDataN);
		}
		else
			{
#if DBGAVFILE
		VIM_USER_PrintDec( "wNum=",wNum);
		VIM_USER_PrintDec( "pAvMp3->dFrameLength =",pAvMp3->dFrameLength );

#endif	
			}
		g_StopAVIErrorSign=0;
		break;

#if SUPPORT_FLASH
	case MMIDI_MODE:
		ReadMidiBuffer();
		if((pAvMidi->bBufFlag[b12Flag] == 1) && (dTimeNow >= pAvMp3->dTicker[b12Flag]))
		{
			AVDisplayJpeg(pAvMidi->pbBuffer[b12Flag], pAvMidi->dBufferSize[b12Flag]);
			pAvMidi->bBufFlag[b12Flag] = 0;

			if(b12Flag)
				b12Flag = 0;
			else
				b12Flag = 1;
			dTimeNow = 0;
		}
		break;
#endif

	case AVI_MODE:
		if ((dState != STATE_PAUSE)&&(pkAvi->dAIndex>0)) // with audio
		{
			// send audio data to ADPCM FIFO
			AVGetFifoSize(2, &wNum);
			ReadAviBuffer(pkAvi->bSource, pbBuffer, wNum, &dDataN, 0);
			AVWriteFifo(2, pbBuffer, dDataN);
			// 4-bit width per sample
			g_Time.dwAudio+= (dDataN*2*1000)/(pkAvi->dSampleRate* pkAvi->bChannel);
		}
		// display video 
		// get current time
		g_Time.dwSystem = g_Time.dwBase +VIM_USER_GetMs();
		// get video frames number	
		if (g_Time.dwSystem>0)
			FrameNum = g_Time.dwSystem/pkAvi->dPerFrame;
		else
			FrameNum = 0;
		if (pkAvi->dAIndex>0)  //with audio
		{
			if (g_Time.dwSystem> (g_Time.dwAudio+TIME_START_POINT_ADJUST))
			{
				if (dState == STATE_PAUSE)
				{
					// pasued 
					AVStopTimer();
					// Stop long timer, the long timer will be reset when stop
					g_Time.dwBase = TIME_START_POINT_ADJUST+g_Time.dwAudio;
					VIM_USER_Caculate(1);
					AVClosePA(); //close PA to avoid noise
				}
				else
				{
					 // stopped 
					 AVlr_Stop();
					 AVlr_Unload();
#if DBGAVFILE
					Uart_Printf( "ended for audio\n");
#endif
					bAVEnd=1;
					if (pAvfunction != NULL)
						pAvfunction();
				}
				return;
			}
		}
		// get video data
		if (FrameNum >1)
			FrameNum--;
		else
			FrameNum=0;
		bAVEnd=AviGetJpgData(FrameNum);
		// auto stop when the video ended
		if (bAVEnd)	
		{
			// stopped 
			AVlr_Stop();
			AVlr_Unload();
#if DBGAVFILE			 
			Uart_Printf( "ended for video\n");
#endif
			if (pAvfunction != NULL)
				pAvfunction();
			return;
		}
		// display picture
		//AVDisplayJpeg(g_AVDisJPGBuf, g_AVDisJPGSize);	
		AVDisplayJpeg(btempBufferPIC1, g_AVDisJPGSize);	
		break;
		
	default:
		break;
	}
}
UINT32   AVlr_GetCurrentStaus(void )
{
    return dState;
}

#if SUPPORT_FLASH
/***************************************************************
Description:
		Malloc Function

Parameters:
		...

Returns:
		...		

****************************************************************/

void* V_Malloc(UINT32 nbyte)
{
	void* mem;

	//mem=(void *)OSMalloc(nbyte);
	mem = (void*)VIM_USER_MallocMemory(sizeof(char) * nbyte);
	//Uart_Printf( "malloc(%d)\n",nbyte);
	//mem = 0;
	return mem;
}

/***************************************************************
Description:
		Free Function.

Parameters:
		...

Returns:
		...		

****************************************************************/
void V_Free(void* mem)
{
	//myfree(mem);
	GUI_Free(mem);
	Uart_Printf( "free()\n");
}

/***************************************************************
Description:
		Play Music

Parameters:
		dPos:		Position of Music
		dRepeat:	Repeat Number
		pFun:		Callback Function

Returns:
		NULL

****************************************************************/
static void LoadMusic(UINT8 devid,HUGE UINT8 *pbFilePtr, UINT32 dFileSize, UINT8 bSrc)
{
	//AV_audio_LoadMusic(devid, pbFilePtr, dFileSize, bSrc);
}


/***************************************************************
Description:
		Play Music

Parameters:
		dPos:		Position of Music
		dRepeat:	Repeat Number
		pFun:		Callback Function

Returns:
		NULL

****************************************************************/
static void StartPlayMusic(UINT32 dPos, UINT32 dRepeat, PEFUNCTION pFun)
{
	AV_audio_StartPlayMusic(dPos, dRepeat, pFun);
}


/***************************************************************
Description:
		Stop Music

Parameters:
		NULL

Returns:
		NULL

****************************************************************/
static void StopPlayMusic(void)
{
	AV_audio_StopPlayMusic();
}

/***************************************************************
Description:
		Read Jpeg File from ROM

Parameters:
		NULL

Returns:
		  NULL
		  
****************************************************************/
void ReadMidiBuffer(void)
{
	UINT32	dTemp, j;
	UINT8	bBuf[22];
	UINT8*	pbSptr;

	if ((pAvMidi->bBufFlag[0] == 1) && (pAvMidi->bBufFlag[1] == 1))
		return;
	if (pAvMidi->dOffset >= pAvMidi->dSize)
		return;

	if(pAvMidi->bBufFlag[0] == 0)
		j = 0;
	else if(pAvMidi->bBufFlag[1] == 0)
		j = 1;
	if (pbMidiS)
	{
		VPlr_ReadFile((void*)(pAvMidi->pbJpegPtr), bBuf, 22);
		dTemp = ((UINT32)bBuf[3] << 24) + ((UINT32)bBuf[2] << 16) +
				((UINT32)bBuf[1] << 8) + ((UINT32)bBuf[0] << 0);
		pAvMidi->dTicker[j] = dTemp;
		pAvMidi->dOffset +=4;
		dTemp = ((UINT32)bBuf[21] << 24) + ((UINT32)bBuf[20] << 16) + 
				((UINT32)bBuf[19] << 8) + ((UINT32)bBuf[18] << 0);
		pAvMidi->dBufferSize[j] = dTemp;
		pbSptr = bBuf + 4;
		VIM_USER_MemCpy((HUGE UINT8*)pAvMidi->pbBuffer[j], (HUGE UINT8*)pbSptr, 18);
		pbSptr = pAvMidi->pbBuffer[j] + 18;
		VPlr_ReadFile((void*)(pAvMidi->pbJpegPtr), pbSptr, (dTemp - 18));
		pAvMidi->bBufFlag[j] = 1;
		pAvMidi->dOffset += dTemp;
	}
	else
	{
		dTemp = ((UINT32)pAvMidi->pbJpegPtr[3] << 24) + ((UINT32)pAvMidi->pbJpegPtr[2] << 16) +
			((UINT32)pAvMidi->pbJpegPtr[1] << 8) + ((UINT32)pAvMidi->pbJpegPtr[0] << 0);
		pAvMidi->dTicker[j] = dTemp;
		pAvMidi->pbJpegPtr +=4;
		pAvMidi->dOffset +=4;
		dTemp = ((UINT32)pAvMidi->pbJpegPtr[17] << 24) + ((UINT32)pAvMidi->pbJpegPtr[16] << 16) + 
			((UINT32)pAvMidi->pbJpegPtr[15] << 8) + ((UINT32)pAvMidi->pbJpegPtr[14] << 0);
		pAvMidi->dBufferSize[j] = dTemp;
		VIM_USER_MemCpy((HUGE UINT8*)pAvMidi->pbBuffer[j], (HUGE UINT8*)pAvMidi->pbJpegPtr, dTemp);
		pAvMidi->bBufFlag[j] = 1;
		pAvMidi->pbJpegPtr += dTemp;
		pAvMidi->dOffset += dTemp;
	}
}
#endif

