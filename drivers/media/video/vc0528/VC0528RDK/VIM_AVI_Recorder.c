
/*
 *  VC0868RDK/AviRecorder.c
 *
 *  Copyright (C) 2005 Vimicro Corporation
 *  
 *  Written by angela	Email: heju@vimicro.com	Tel: 010-68948888-8779
 *
 *  

 *	
 */
#include <string.h>
#include"VIM_COMMON.h"

#if VIM_USER_SURPORT_AVI==1

#define CAPJPEGRATIO	gVc0528_Info.CaptureStatus.QualityMode
#define MAXINTERVEL 200//(MS)
#define INDEXBUFLENGTH 0X4000//BYTE about6m 320x240
#define AUDIOBUFLENGTH 0X15000//BYTE
#define ONEFRAMELENGTH 0X15000//BYTE about6m 320x240
#define BITTOLITTLE

#define DEBUGAVI	
extern void Uart_Printf(char *fmt,...);
#define AviPrintf	Uart_Printf

/***************************************************************
Description:
		Start Timer and set timer interval

Parameters:
		interval : the interval of every timer(ms)

Returns:
		0: true
		1-255: error

****************************************************************/
UINT8 Avi_StartTimer(UINT32 Intervel)
{
	UINT8 result=0;
	result=VIM_USER_StartTimer(Intervel);
	return result;
}

/***************************************************************
Description:
		cancel Timer 
Parameters:

Returns:
		0: true
		1-255: error

****************************************************************/
UINT8 Avi_StopTimer(void)
{
	UINT8 result=0;
	result=VIM_USER_StopTimer();	
	return result;
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
void Avi_MemCpy(HUGE UINT8 *pbDest, const HUGE UINT8 *pbSrc, UINT32 dCount)
{
	VIM_USER_MemCpy(pbDest,pbSrc,dCount);
	//memcpy(pbDest,pbSrc,dCount);
}
/***************************************************************
Description:
		Start Capture(get one frame)

Parameters:
		str : the head point of one jpeg
		length: the length of jpeg

Returns:
		0: true
		1-255: error

****************************************************************/
UINT8 Avi_StartCaptureJpeg(HUGE UINT8 *str,UINT32 *length)
{
	UINT8 result;		
#if 1
	VIM_HAPI_SetCaptureVideoInfo(VIM_HAPI_RAM_SAVE,0,0);
	result=VIM_HAPI_StartCaptureVideo(str,0X10000,0);
	AVI_Status.SameFrameNum=0;
	if(!result)
	{
RECAP:
		VIM_USER_DelayMs(80);
		result=VIM_HAPI_GetQuickOneJpeg(str,ONEFRAMELENGTH,length);
		if(result==VIM_ERROR_NOINITERRUPT)
			goto RECAP;
	}
#else
	VIM_HAPI_SetCaptureVideoInfo(VIM_HAPI_RAM_SAVE,0,10000);

	result=VIM_HAPI_StartCaptureVideo(str,0X10000,0);
	if(!result)
		result=VIM_HAPI_GetOneJpeg(str,0X10000,length);

#endif
	return result;
}

/***************************************************************
Description:
		get one frame and preview again

Parameters:
		str : the head point of one jpeg
		length: the length of jpeg

Returns:
		0: true
		1-255: error

****************************************************************/
UINT8 Avi_GetOneJpeg(HUGE UINT8 *str,UINT32 *length,UINT32 Buflength)
{
	UINT8 result;
#if 1
	if(Buflength<ONEFRAMELENGTH)
	{
#ifdef DEBUGAVI
		AviPrintf("Buflength<ONEFRAMELENGTH  Buflength=\n",Buflength);
#endif 	
		return VIM_ERROR_BUFFERLENGTHS;
	}
	result=VIM_HAPI_GetQuickOneJpeg(AVI_Status.OneFrame,ONEFRAMELENGTH,&AVI_Status.OneFrameLength);
	if(result==VIM_ERROR_NOINITERRUPT)
	{
		Avi_MemCpy(str,AVI_Status.OneFrame,AVI_Status.OneFrameLength);
		*length=AVI_Status.OneFrameLength;
		AVI_Status.SameFrameNum++;
//#ifdef DEBUGAVI

		AviPrintf("same frame=. %d length=%d\n",AVI_Status.SameFrameNum,AVI_Status.OneFrameLength);
//#endif 
		if(AVI_Status.SameFrameNum>15)
			return VIM_ERROR_NOINITERRUPT;
		else
			return VIM_SUCCEED;
			
	}
	else if(result==VIM_SUCCEED)
	{
		Avi_MemCpy(str,AVI_Status.OneFrame,AVI_Status.OneFrameLength);
		*length=AVI_Status.OneFrameLength;	
		AVI_Status.SameFrameNum=0;
//#ifdef DEBUGAVI

		AviPrintf(".");
//#endif 
	}
#else
	result=VIM_HAPI_GetOneJpeg(str,Buflength,length);

#endif
#ifdef DEBUGAVI
	if(result)
		AviPrintf("Avi_GetOneJpeg error num=. 0x%x\n",result);
#endif 
	return result;

}

/***************************************************************
Description:
		Stop capture and stop preview(get the last one )

Parameters:

Returns:
		0: true
		1-255: error

****************************************************************/
UINT8 Avi_StopCaptureJpeg(void)
{
	UINT8 result;
	result=VIM_HAPI_StopCapture();
	return result;
}

/***************************************************************
Description:
		Start Recoter audio

Parameters:

Returns:
		0: true
		1-255: error

****************************************************************/

UINT8 Avi_StartRecorder(void)
{
	UINT8 result=0;
	return result;
}

/***************************************************************
Description:
		get adpcm 

Parameters:
		str : the head point of one adpcm
		length: the length of adpcm

Returns:
		0: true
		1-255: error

****************************************************************/
UINT8 Avi_GetAudio(HUGE UINT8 *str,UINT32 *length,UINT32 Buflength)
{
UINT8 result=1;
#if 0
	if(RecorderStatus%13==0)
	{
			if(Buflength>=sizeof(adpcmAudio))
				{
				memcpy(str,adpcmAudio,sizeof(adpcmAudio));
				*length=sizeof(adpcmAudio);
				result=0;
				}
			else 
				result=1;
	}
	RecorderStatus++;
#else
	//if(0)
		{
			//result=APP_ReadAudioFile(1,str,Buflength,length);
		}
#endif
	return result;
}

/***************************************************************
Description:
		Stop recorder

Parameters:

Returns:
		0: true
		1-255: error

****************************************************************/

UINT8 Avi_StopRecorder(void)
{
	UINT8 result=0;
	//result=APPAVI_StopAudioR();

	return result;
}
/***************************************************************
Description:
		Set avi vol 

Parameters:


Returns:
		0: true
		1-255: error

****************************************************************/
UINT8 Avi_SetVol(UINT8 bVol)
{
	UINT8 result=0;

		//result=Vmac_RecSetVol(0,bVol);
		//if(result)
		//	return result;
		//result=Vmac_RecSetVol(1,bVol);
		return result;
}
/***************************************************************
Description:
		Malloc Function

Parameters:
		...

Returns:
		...		

****************************************************************/

void* Avi_Malloc(UINT32 nbyte)
{
	void* mem;

	//mem=(void *)OSMalloc(nbyte);
	mem = (void*)VIM_USER_MallocMemory(nbyte);
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

void Avi_Free(void* mem)
{
	VIM_USER_FreeMemory(mem);	
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
UINT32 Avi_WriteFile(void* pFile_Ptr, HUGE UINT8* pbBuffer, UINT32 dFile_Size)
{
	return VIM_USER_WriteFile(pFile_Ptr,pbBuffer,dFile_Size);
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
UINT32 Avi_SeekFile(void* pFile_Ptr, UINT32 dOffset)
{
	return VIM_USER_SeekFile(pFile_Ptr,dOffset);
}

//////////////////////////////////////////////////
unsigned char Avi_header[]=
{
  0x52,0x49,0x46,0x46,0x00,0x00,0x00,0x00,0x41,0x56,0x49,0x20,0x4c,0x49,0x53,0x54,//0
  0xc0,0x00,0x00,0x00,0x68,0x64,0x72,0x6c,0x61,0x76,0x69,0x68,0x38,0x00,0x00,0x00,//1
  0xd8,0x9c,0x02,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x10,0x00,0x01,0x00,//2
  0x59,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x30,0x00,0x00,//3
  0x40,0x01,0x00,0x00,0xf0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//4
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x4c,0x49,0x53,0x54,0x74,0x00,0x00,0x00,//5
  0x73,0x74,0x72,0x6c,0x73,0x74,0x72,0x68,0x38,0x00,0x00,0x00,0x76,0x69,0x64,0x73,//6
  0x6d,0x6a,0x70,0x67,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//7
  0x87,0x3b,0x00,0x00,0xa8,0x5b,0x01,0x00,0x00,0x00,0x00,0x00,0x59,0x00,0x00,0x00,//8
  0x00,0x30,0x00,0x00,0x10,0x27,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//9
  0x00,0x00,0x00,0x00,0x73,0x74,0x72,0x66,0x28,0x00,0x00,0x00,0x28,0x00,0x00,0x00,//a
  0x40,0x01,0x00,0x00,0xf0,0x00,0x00,0x00,0x01,0x00,0x18,0x00,0x4d,0x4a,0x50,0x47,//b
  0x00,0xc2,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//c
  0x00,0x00,0x00,0x00,

  0x4C,0x49,0x53,0x54,0x5c,0x00,0x00,0x00,0x73,0x74,0x72,0x6C,0x73,0x74,0x72,0x68,
  0x38, 0x00, 0x00, 0x00, 0x61, 0x75, 0x64, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x40, 0x1F, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
  0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x73, 0x74, 0x72, 0x66, 0x14, 0x00, 0x00, 0x00, 
  0x01, 0x00, 0x01, 0x00,0x40, 0x1F, 0x00, 0x00, 0x80, 0x3E, 0x00, 0x00, 
  0x02, 0x00, 0x10, 0x00, 0x02, 0x00, 0x10, 0x00,

  
/*  0x4C,0x49,0x53,0x54,0x5c,0x00,0x00,0x00,0x73,0x74,0x72,0x6C,0x73,0x74,0x72,0x68,
  0x38, 0x00, 0x00, 0x00, 0x61, 0x75, 0x64, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x40, 0x1F, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
  0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x73, 0x74, 0x72, 0x66,
  0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,0x40, 0x1F, 0x00, 0x00, 0x80, 0x3E, 0x00, 0x00, 
  0x02, 0x00, 0x10, 0x00,*/

  0x4c,0x49,0x53,0x54,0xff,0xff,0xff,0xff,0x6d,0x6f,0x76,0x69 //d
};
unsigned char AvI_VideoInBuf[8]={0x30,0x30,0x64,0x63,0x0,0,0,0};
unsigned char AvI_AudioInBuf[8]={0x30,0x31,0x77,0x62,0x0,0,0,0};
unsigned char AvI_VideoIndex[8]={0x30,0x30,0x64,0x63,0x10,0,0,0};
unsigned char AvI_AudioIndex[8]={0x30,0x31,0x77,0x62,0x0,0,0,0};
unsigned char AvI_indexHeader[8]={0x69,0x64,0x78,0x31,0,0,0,0};
UINT32 AVIHeaderLength=sizeof(Avi_header);
/////////////////////////////////interface of recorder //////////////////////////////////////
AviStatus AVI_Status;
PAviFileHeader pAVI_FileHead;
PAviMainHeader pAVI_MainHead;
PAviVideoHeader	pAVI_VideoHead;
PAviVideoFormat	pAVI_VideoFormat;
PAviAudioHeader	pAVI_AudioHead;
PAviAudioFormat pAVI_AudioFormat;
PAviStream pAVI_Stream;
PAviVideoInsert pAVI_VideoInsert;
PAviAudioInsert pAVI_AudioInsert;
PAviIndexHeader pAVI_IndexHeader;

PAviVideoIndex pAVI_VideoIndex;
PAviAudioIndex pAVI_AudioIndex;

/***************************************************************
Description:
		Load information to code

Parameters:

Returns:

****************************************************************/

void Avi_LoadInfo(PAviInfo Info,UINT8 bSaveType,Avi_CallBack pAvi_Callback)
{
	AVI_Status.AviInformation.dFileMaxSize=Info->dFileMaxSize;
	AVI_Status.AviInformation.wCaptureHeight=Info->wCaptureHeight;
	AVI_Status.AviInformation.wCaptureWidth=Info->wCaptureWidth;
	AVI_Status.AviInformation.wFrameRate=Info->wFrameRate;
	AVI_Status.AviInformation.bVol=Info->bVol;
	AVI_Status.AviCallBack=pAvi_Callback;
	AVI_Status.bSaveType=bSaveType;
	AVI_Status.NowStuatus=AVI_END;
}
/***************************************************************
Description:
		Start Caputer avi

Parameters:

Returns:
		0: true
		1-255: error

****************************************************************/
UINT8 Avi_StartCapture(void *str)
{
UINT8 * PHeadPoint=Avi_header;
UINT32 length,intervel;

	if(AVI_Status.NowStuatus!=AVI_END)
		return AVIFAIL;
	pAVI_FileHead=(PAviFileHeader)PHeadPoint;
	PHeadPoint+=sizeof(AviFileHeader);
	pAVI_MainHead=(PAviMainHeader)PHeadPoint;
	PHeadPoint+=sizeof(AviMainHeader);
	pAVI_VideoHead=(PAviVideoHeader)PHeadPoint;
	PHeadPoint+=sizeof(AviVideoHeader);
	pAVI_VideoFormat=(PAviVideoFormat)PHeadPoint;
	PHeadPoint+=sizeof(AviVideoFormat);
	pAVI_AudioHead=(PAviAudioHeader)PHeadPoint;
	PHeadPoint+=sizeof(AviAudioHeader);
	pAVI_AudioFormat=(PAviAudioFormat)PHeadPoint;
	PHeadPoint+=sizeof(AviAudioFormat);
	pAVI_Stream=(PAviStream)PHeadPoint;
	AVI_Status.dFileLength=0;
	AVI_Status.dTotalFrame=0;
	AVI_Status.NowStuatus=AVI_OPEN;
	//AvI_VideoIndex
	Avi_SetVol(AVI_Status.AviInformation.bVol);
	if(Avi_StartRecorder())
	{
		AVI_Status.bErrorNum=AVIERROR_AUDIO;//no jpeg
		goto STARTFAILBACK;
	}

	AVI_Status.pIndexBufPoint=(UINT8 *)Avi_Malloc(INDEXBUFLENGTH);
	AVI_Status.OneFrame=(UINT8 *)Avi_Malloc(ONEFRAMELENGTH);
	pAVI_IndexHeader=(PAviIndexHeader)AVI_Status.pIndexBufPoint;
	AVI_Status.dIndexLength=8;
	AVI_Status.dPosition=4;
	Avi_MemCpy(AVI_Status.pIndexBufPoint,AvI_indexHeader,8);
	AVI_Status.pIndexBufPoint+=8;
	
	intervel=1000/AVI_Status.AviInformation.wFrameRate;
	if(intervel>MAXINTERVEL)
			intervel=100;
	pAVI_MainHead->dwMicroSecPerFrame=intervel*1000;
	switch(AVI_Status.bSaveType)
		{
			case RAMSAVE:
				AVI_Status.pbFilePoint=(UINT8 *)str+AVIHeaderLength;
				AVI_Status.dFileLength+=AVIHeaderLength;
				Avi_MemCpy(AVI_Status.pbFilePoint,AvI_VideoInBuf,8);	//insert
				pAVI_VideoInsert=(PAviVideoInsert)AVI_Status.pbFilePoint;
				AVI_Status.pbFilePoint+=8;
				AVI_Status.dFileLength+=8;
				if(Avi_StartCaptureJpeg(AVI_Status.pbFilePoint,&length))
				{
					AVI_Status.bErrorNum=AVIERROR_NOJPEG;//no jpeg
					goto STARTFAILBACK;
				}
#ifdef DEBUGAVI
AviPrintf("\n start have video. 0x%x\n",length);
#endif 
				pAVI_VideoInsert->Length=BITTOLITTLE(length);
				AVI_Status.pbFilePoint+=length;
				AVI_Status.dFileLength+=length;
				AVI_Status.dTotalFrame+=1;

				Avi_MemCpy(AVI_Status.pIndexBufPoint,AvI_VideoIndex,8);
				pAVI_VideoIndex=(PAviVideoIndex)AVI_Status.pIndexBufPoint;
				pAVI_VideoIndex->Position=BITTOLITTLE(AVI_Status.dPosition);
				pAVI_VideoIndex->Length=BITTOLITTLE(length);
				AVI_Status.pIndexBufPoint+=16;
				AVI_Status.dPosition+=8;
				AVI_Status.dPosition+=length;
				AVI_Status.dIndexLength+=16;
#ifdef DEBUGAVI
AviPrintf("\nindex length. 0x%x\n",AVI_Status.dIndexLength);
#endif 

				break;
			case ROMSAVE:
				Avi_SeekFile(str,AVIHeaderLength);
				AVI_Status.dFileLength+=AVIHeaderLength;
				AVI_Status.pbFilePoint=(UINT8 *)str;
				AVI_Status.TempBuf=(UINT8 *)Avi_Malloc(AVI_Status.AviInformation.wCaptureWidth*AVI_Status.AviInformation.wCaptureHeight
					/CAPJPEGRATIO*2);
				//Avi_WriteFile(AVI_Status.pbFilePoint,AvI_VideoInBuf,8);//insert
				

				pAVI_VideoInsert=(PAviVideoInsert)AVI_Status.TempBuf;
				AVI_Status.dFileLength+=8;
				if(Avi_StartCaptureJpeg(AVI_Status.TempBuf+8,&length))
				{
					AVI_Status.bErrorNum=AVIERROR_NOJPEG;//no jpeg
					goto STARTFAILBACK;
				}
#ifdef DEBUGAVI
AviPrintf("\n start have video. 0x%x\n",length);
#endif 
				pAVI_VideoInsert->Length=BITTOLITTLE(length);
				Avi_WriteFile((void *)AVI_Status.pbFilePoint,AVI_Status.TempBuf,length+8);
				AVI_Status.dFileLength+=length;
				AVI_Status.dTotalFrame+=1;

				Avi_MemCpy(AVI_Status.pIndexBufPoint,AvI_VideoIndex,8);
				pAVI_VideoIndex=(PAviVideoIndex)AVI_Status.pIndexBufPoint;
				pAVI_VideoIndex->Position=BITTOLITTLE(AVI_Status.dPosition);
				pAVI_VideoIndex->Length=BITTOLITTLE(length);
				AVI_Status.pIndexBufPoint+=16;
				AVI_Status.dPosition+=8;
				AVI_Status.dPosition+=length;
				AVI_Status.dIndexLength+=16;
#ifdef DEBUGAVI
AviPrintf("\nindex length. 0x%x\n",AVI_Status.dIndexLength);
#endif 
				break;
			default:
				goto STARTFAILBACK;
		}
	Avi_StartTimer(intervel/2);
	AVI_Status.AviCallBack(AVI_OPEN,AVI_Status.dIndexLength+AVI_Status.dFileLength);
	return AVISUCCESS;
STARTFAILBACK:
	Avi_StopCapture();
	AVI_Status.AviCallBack(AVI_ERROR,AVI_Status.bErrorNum);
#ifdef DEBUGAVI
AviPrintf("errornum. %d\n",AVI_Status.bErrorNum);
#endif 
	return AVIFAIL;
}

/***************************************************************
Description:
		code used in timer

Parameters:

Returns:
		0: true
		1-255: error

****************************************************************/
UINT8 Avi_Timer(void)
{
	UINT32 Audiolength,VideoLength,LastLength;
	switch(AVI_Status.bSaveType)
		{
			case RAMSAVE:
				LastLength=AVI_Status.AviInformation.dFileMaxSize-AVI_Status.dIndexLength-16-AVI_Status.dFileLength-8-AUDIOBUFLENGTH;
				if(Avi_GetAudio(AVI_Status.pbFilePoint+8,&Audiolength,LastLength)==0)
				{
					#ifdef DEBUGAVI
					AviPrintf("timer have audio. 0x%x\n",Audiolength);
					AviPrintf("last length. 0x%x\n",LastLength);
					#endif 
					Avi_MemCpy(AVI_Status.pbFilePoint,AvI_AudioInBuf,8);	//insert	
					pAVI_AudioInsert=(PAviAudioInsert)AVI_Status.pbFilePoint;
					pAVI_AudioInsert->Length=BITTOLITTLE(Audiolength);
					AVI_Status.pbFilePoint+=(Audiolength+8);
					AVI_Status.dFileLength+=(Audiolength+8);
					Avi_MemCpy(AVI_Status.pIndexBufPoint,AvI_AudioIndex,8);
					pAVI_AudioIndex=(PAviAudioIndex)AVI_Status.pIndexBufPoint;
					pAVI_AudioIndex->Position=BITTOLITTLE(AVI_Status.dPosition);
					pAVI_AudioIndex->Length=BITTOLITTLE(Audiolength);
					AVI_Status.pIndexBufPoint+=16;
					AVI_Status.dPosition+=8;
					AVI_Status.dPosition+=Audiolength;
					AVI_Status.dIndexLength+=16;
					LastLength=AVI_Status.AviInformation.dFileMaxSize-AVI_Status.dIndexLength-16-AVI_Status.dFileLength-8-AUDIOBUFLENGTH;
				}

				if(Avi_GetOneJpeg(AVI_Status.pbFilePoint+8,&VideoLength,LastLength))
				{
					#ifdef DEBUGAVI
						AviPrintf("Avi_GetOneJpeg error.\n");
					#endif 				
					return Avi_StopCapture();
				}
					
				#ifdef DEBUGAVI
				AviPrintf("timer have video. 0x%x\n",VideoLength);
				AviPrintf("last length. 0x%x\n",LastLength);
				#endif 
				Avi_MemCpy(AVI_Status.pbFilePoint,AvI_VideoInBuf,8);	//insert
				pAVI_VideoInsert=(PAviVideoInsert)AVI_Status.pbFilePoint;
				pAVI_VideoInsert->Length=BITTOLITTLE(VideoLength);
				AVI_Status.pbFilePoint+=(VideoLength+8);
				AVI_Status.dFileLength+=(VideoLength+8);
				AVI_Status.dTotalFrame+=1;

				Avi_MemCpy(AVI_Status.pIndexBufPoint,AvI_VideoIndex,8);
				pAVI_VideoIndex=(PAviVideoIndex)AVI_Status.pIndexBufPoint;
				pAVI_VideoIndex->Position=BITTOLITTLE(AVI_Status.dPosition);
				pAVI_VideoIndex->Length=BITTOLITTLE(VideoLength);
				AVI_Status.pIndexBufPoint+=16;
				AVI_Status.dPosition+=8;
				AVI_Status.dPosition+=VideoLength;
				AVI_Status.dIndexLength+=16;
				break;
			case ROMSAVE:
				LastLength=AVI_Status.AviInformation.dFileMaxSize-AVI_Status.dIndexLength-16-AVI_Status.dFileLength-8-AUDIOBUFLENGTH;
				if(Avi_GetAudio(AVI_Status.pbFilePoint+8,&Audiolength,LastLength)==0)
				{
					#ifdef DEBUGAVI
					AviPrintf("timer have audio. 0x%x\n",Audiolength);
					AviPrintf("last length. %d\n",LastLength);
					#endif 
					Avi_MemCpy(AVI_Status.pbFilePoint,AvI_AudioInBuf,8);	//insert	
					pAVI_AudioInsert=(PAviAudioInsert)AVI_Status.pbFilePoint;
					pAVI_AudioInsert->Length=BITTOLITTLE(Audiolength);
					AVI_Status.pbFilePoint+=(Audiolength+8);
					AVI_Status.dFileLength+=(Audiolength+8);

					Avi_MemCpy(AVI_Status.pIndexBufPoint,AvI_AudioIndex,8);
					pAVI_AudioIndex=(PAviAudioIndex)AVI_Status.pIndexBufPoint;
					pAVI_AudioIndex->Position=BITTOLITTLE(AVI_Status.dPosition);
					pAVI_AudioIndex->Length=BITTOLITTLE(Audiolength);
					AVI_Status.pIndexBufPoint+=16;
					AVI_Status.dPosition+=8;
					AVI_Status.dPosition+=Audiolength;
					AVI_Status.dIndexLength+=16;
					LastLength=AVI_Status.AviInformation.dFileMaxSize-AVI_Status.dIndexLength-16-AVI_Status.dFileLength-8-AUDIOBUFLENGTH;
				}

				if((Avi_GetOneJpeg(AVI_Status.TempBuf+8,&VideoLength,LastLength))||
					((AVI_Status.dIndexLength+16)>INDEXBUFLENGTH))
				{
					#ifdef DEBUGAVI
					AviPrintf("Avi_GetOneJpeg error.\n");
					#endif 				
					return Avi_StopCapture();
				}
					
				#ifdef DEBUGAVI
				AviPrintf("timer have video. 0x%x\n",VideoLength);
				AviPrintf("last length. %d\n",LastLength);
				#endif 
				Avi_MemCpy(AVI_Status.TempBuf,AvI_VideoInBuf,8);	//insert
				pAVI_VideoInsert=(PAviVideoInsert)AVI_Status.TempBuf;
				pAVI_VideoInsert->Length=BITTOLITTLE(VideoLength);
				AVI_Status.bErrorNum=Avi_WriteFile((void *)AVI_Status.pbFilePoint,AVI_Status.TempBuf,VideoLength+8);
				if(AVI_Status.bErrorNum)
					goto FAILBACK;
				AVI_Status.dFileLength+=(VideoLength+8);
				AVI_Status.dTotalFrame+=1;

				Avi_MemCpy(AVI_Status.pIndexBufPoint,AvI_VideoIndex,8);
				pAVI_VideoIndex=(PAviVideoIndex)AVI_Status.pIndexBufPoint;
				pAVI_VideoIndex->Position=BITTOLITTLE(AVI_Status.dPosition);
				pAVI_VideoIndex->Length=BITTOLITTLE(VideoLength);
				AVI_Status.pIndexBufPoint+=16;
				AVI_Status.dPosition+=8;
				AVI_Status.dPosition+=VideoLength;
				AVI_Status.dIndexLength+=16;
				break;
			default:
				goto FAILBACK;
		}
	AVI_Status.AviCallBack(AVI_TIMER,AVI_Status.dIndexLength+AVI_Status.dFileLength);
	return AVISUCCESS;
FAILBACK:
	Avi_StopCapture();
	AVI_Status.AviCallBack(AVI_ERROR,AVI_Status.bErrorNum);
	return AVIFAIL;

}

/***************************************************************
Description:
		Stop Caputure

Parameters:

Returns:
		0: true
		1-255: error

****************************************************************/
UINT8 Avi_StopCapture(void)
{
UINT32 LastLength,Audiolength;
	if(AVI_Status.NowStuatus==AVI_END)
		return AVIFAIL;
	Avi_StopTimer();
	Avi_StopCaptureJpeg();
	AVI_Status.NowStuatus=AVI_END;
	Avi_Free(AVI_Status.OneFrame);	//angela 2006-12-8
	if(Avi_StopRecorder())
	{
		AVI_Status.bErrorNum=AVIERROR_AUDIO;
		goto STOPFAILBACK;
	}
	LastLength=AVI_Status.AviInformation.dFileMaxSize-AVI_Status.dIndexLength-16-AVI_Status.dFileLength-8;
	switch(AVI_Status.bSaveType)
	{
		case RAMSAVE:
			if(Avi_GetAudio(AVI_Status.pbFilePoint+8,&Audiolength,LastLength)==0)
			{
				Avi_MemCpy(AVI_Status.pbFilePoint,AvI_AudioInBuf,8);	//insert	
				pAVI_AudioInsert=(PAviAudioInsert)AVI_Status.pbFilePoint;
				pAVI_AudioInsert->Length=BITTOLITTLE(Audiolength);
				AVI_Status.pbFilePoint+=(Audiolength+8);
				AVI_Status.dFileLength+=(Audiolength+8);
#ifdef DEBUGAVI
AviPrintf("Have audio. 0x%x\n",Audiolength);
#endif	
				Avi_MemCpy(AVI_Status.pIndexBufPoint,AvI_AudioIndex,8);
				pAVI_AudioIndex=(PAviAudioIndex)AVI_Status.pIndexBufPoint;
				pAVI_AudioIndex->Position=BITTOLITTLE(AVI_Status.dPosition);
				pAVI_AudioIndex->Length=BITTOLITTLE(Audiolength);
				AVI_Status.pIndexBufPoint+=16;
				AVI_Status.dPosition+=8;
				AVI_Status.dPosition+=Audiolength;
				AVI_Status.dIndexLength+=16;
				LastLength=AVI_Status.AviInformation.dFileMaxSize-AVI_Status.dIndexLength-16-AVI_Status.dFileLength-8;
			}
			pAVI_FileHead->FileLength=BITTOLITTLE(AVI_Status.dFileLength+AVI_Status.dIndexLength);
#ifdef DEBUGAVI
AviPrintf("FileLength. 0x%x\n",pAVI_FileHead->FileLength);
#endif			
			pAVI_MainHead->ListSize=sizeof(AviMainHeader)+2*sizeof(AviVideoHeader)+sizeof(AviVideoFormat)+sizeof(AviAudioFormat)-8;
			pAVI_MainHead->dwTotalFrames=BITTOLITTLE(AVI_Status.dTotalFrame);
#ifdef DEBUGAVI
AviPrintf("dwTotalFrames. %d\n",pAVI_MainHead->dwTotalFrames);
#endif		
			pAVI_MainHead->dwWidth=BITTOLITTLE(AVI_Status.AviInformation.wCaptureWidth);
			pAVI_MainHead->dwHeight=BITTOLITTLE(AVI_Status.AviInformation.wCaptureHeight);
			pAVI_MainHead->dwStreams=BITTOLITTLE(0x02);

			pAVI_VideoHead->dwScale=BITTOLITTLE(0x64);
			pAVI_VideoHead->dwRate=BITTOLITTLE(AVI_Status.AviInformation.wFrameRate*100);
			pAVI_VideoHead->dwLength=BITTOLITTLE(AVI_Status.dTotalFrame);
			pAVI_VideoFormat->dwWidth=BITTOLITTLE(AVI_Status.AviInformation.wCaptureWidth);
			pAVI_VideoFormat->dwHeight=BITTOLITTLE(AVI_Status.AviInformation.wCaptureHeight);

			pAVI_AudioFormat->wFormatTag_nChannels=BITTOLITTLE((0x01<<16)|0x11);
			pAVI_AudioFormat->nSamplesPerSec=BITTOLITTLE(8000);
			pAVI_AudioFormat->nAvgBytesPerSec=BITTOLITTLE(0x00ffffff);
			pAVI_AudioFormat->nBlockAlign_wBitsPerSample=BITTOLITTLE((4<<16)|0x0400);
			pAVI_AudioFormat->cbSize=BITTOLITTLE(0x7f90002);
			pAVI_AudioFormat->StreamFormatSize=BITTOLITTLE(0x14);
			
			pAVI_AudioHead->ListSize=sizeof(AviAudioFormat)+sizeof(AviVideoHeader)-8;
			
			pAVI_Stream->ListSize=AVI_Status.dPosition;

			pAVI_IndexHeader->Length=AVI_Status.dIndexLength-8;
			AVI_Status.pbFilePoint-=AVI_Status.dFileLength;
			Avi_MemCpy(AVI_Status.pbFilePoint,Avi_header,AVIHeaderLength);
			AVI_Status.pbFilePoint+=AVI_Status.dFileLength;
			AVI_Status.pIndexBufPoint-=AVI_Status.dIndexLength;
			Avi_MemCpy(AVI_Status.pbFilePoint,AVI_Status.pIndexBufPoint,AVI_Status.dIndexLength);
			AVI_Status.dFileLength+=AVI_Status.dIndexLength;
			//*length=AVI_Status.dFileLength;
			Avi_Free(AVI_Status.pIndexBufPoint);
			break;
		case ROMSAVE:
			if(Avi_GetAudio(AVI_Status.pbFilePoint+8,&Audiolength,LastLength)==0)
			{
				Avi_MemCpy(AVI_Status.pbFilePoint,AvI_AudioInBuf,8);	//insert	
				pAVI_AudioInsert=(PAviAudioInsert)AVI_Status.pbFilePoint;
				pAVI_AudioInsert->Length=BITTOLITTLE(Audiolength);
				AVI_Status.pbFilePoint+=(Audiolength+8);
				AVI_Status.dFileLength+=(Audiolength+8);
#ifdef DEBUGAVI
AviPrintf("Have audio. 0x%x\n",Audiolength);
#endif	
				Avi_MemCpy(AVI_Status.pIndexBufPoint,AvI_AudioIndex,8);
				pAVI_AudioIndex=(PAviAudioIndex)AVI_Status.pIndexBufPoint;
				pAVI_AudioIndex->Position=BITTOLITTLE(AVI_Status.dPosition);
				pAVI_AudioIndex->Length=BITTOLITTLE(Audiolength);
				AVI_Status.pIndexBufPoint+=16;
				AVI_Status.dPosition+=8;
				AVI_Status.dPosition+=Audiolength;
				AVI_Status.dIndexLength+=16;
				LastLength=AVI_Status.AviInformation.dFileMaxSize-AVI_Status.dIndexLength-16-AVI_Status.dFileLength-8;
			}
			pAVI_FileHead->FileLength=BITTOLITTLE(AVI_Status.dFileLength+AVI_Status.dIndexLength);
#ifdef DEBUGAVI
AviPrintf("FileLength. 0x%x\n",pAVI_FileHead->FileLength);
#endif			
			pAVI_MainHead->ListSize=sizeof(AviMainHeader)+2*sizeof(AviVideoHeader)+sizeof(AviVideoFormat)+sizeof(AviAudioFormat)-8;
			pAVI_MainHead->dwTotalFrames=BITTOLITTLE(AVI_Status.dTotalFrame);
#ifdef DEBUGAVI
AviPrintf("dwTotalFrames. %d\n",pAVI_MainHead->dwTotalFrames);
#endif		
			pAVI_MainHead->dwWidth=BITTOLITTLE(AVI_Status.AviInformation.wCaptureWidth);
			pAVI_MainHead->dwHeight=BITTOLITTLE(AVI_Status.AviInformation.wCaptureHeight);
			pAVI_MainHead->dwStreams=BITTOLITTLE(0x02);

			pAVI_VideoHead->dwScale=BITTOLITTLE(0x64);
			pAVI_VideoHead->dwRate=BITTOLITTLE(AVI_Status.AviInformation.wFrameRate*100);
			pAVI_VideoHead->dwLength=BITTOLITTLE(AVI_Status.dTotalFrame);
			pAVI_VideoFormat->dwWidth=BITTOLITTLE(AVI_Status.AviInformation.wCaptureWidth);
			pAVI_VideoFormat->dwHeight=BITTOLITTLE(AVI_Status.AviInformation.wCaptureHeight);

			pAVI_AudioFormat->wFormatTag_nChannels=BITTOLITTLE((0x01<<16)|0x11);
			pAVI_AudioFormat->nSamplesPerSec=BITTOLITTLE(8000);
			pAVI_AudioFormat->nAvgBytesPerSec=BITTOLITTLE(0x00ffffff);
			pAVI_AudioFormat->nBlockAlign_wBitsPerSample=BITTOLITTLE((4<<16)|0x0400);
			pAVI_AudioFormat->cbSize=BITTOLITTLE(0x7f90002);
			pAVI_AudioFormat->StreamFormatSize=BITTOLITTLE(0x14);
			
			pAVI_AudioHead->ListSize=sizeof(AviAudioFormat)+sizeof(AviVideoHeader)-8;
			
			pAVI_Stream->ListSize=AVI_Status.dPosition;

			pAVI_IndexHeader->Length=AVI_Status.dIndexLength-8;
			//AVI_Status.pbFilePoint-=AVI_Status.dFileLength;
			Avi_SeekFile((void *)AVI_Status.pbFilePoint,0);
			Avi_WriteFile((void *)AVI_Status.pbFilePoint,Avi_header,AVIHeaderLength);
			//AVI_Status.pbFilePoint+=AVI_Status.dFileLength;
			Avi_SeekFile((void *)AVI_Status.pbFilePoint,AVI_Status.dFileLength);
			AVI_Status.pIndexBufPoint-=AVI_Status.dIndexLength;
			Avi_WriteFile((void *)AVI_Status.pbFilePoint,AVI_Status.pIndexBufPoint,AVI_Status.dIndexLength);

			//Avi_MemCpy(AVI_Status.pbFilePoint,AVI_Status.pIndexBufPoint,AVI_Status.dIndexLength);
			AVI_Status.dFileLength+=AVI_Status.dIndexLength;
			//*length=AVI_Status.dFileLength;
			Avi_Free(AVI_Status.TempBuf);
			Avi_Free(AVI_Status.pIndexBufPoint);
			break;
		default:
			goto STOPFAILBACK;
		}
	AVI_Status.AviCallBack(AVI_END,AVI_Status.dFileLength);
	return AVISUCCESS;
STOPFAILBACK:
	AVI_Status.AviCallBack(AVI_ERROR,AVI_Status.bErrorNum);
#ifdef DEBUGAVI
AviPrintf("errornum. %d\n",AVI_Status.bErrorNum);
#endif 
	return AVIFAIL;
}
#endif
