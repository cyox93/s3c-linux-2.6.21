/*************************************************************************
*                                                                       
*                Copyright (C) 2006 Vimicro CO.,LTD     		 
*
* FILE NAME                                  VERSION                
*                                                                       
* VIM_AVI_Recorder.h			   1.1                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*     avi recorder file                 
*
*	Version		Author			Date		Description		
*  ---------------------------------------------------------------						 
*   0.1			angela		2006-10-31	The first version. 
*   1.1			angela		2006-12-31	add huge
*  ---------------------------------------------------------------
*                                                                       
*************************************************************************/
#ifndef __AVPLAYER_H_
#define __AVPLAYER_H_





#ifdef __cplusplus
extern "C" {
#endif



typedef struct tag_AviFileHeader{
	UINT32 RIFF;
	UINT32 FileLength;
	UINT32 AVIFileType;

} AviFileHeader,*PAviFileHeader;

typedef struct tag_AviMainHeader{
	UINT32 LIST;
	UINT32  ListSize;
	UINT32  Hdrl;
	UINT32  Avih;
	UINT32 MainHeaderSize;
	UINT32  dwMicroSecPerFrame;
	UINT32  dwMaxBytesPerSec;
	UINT32  dwReserved1;
	UINT32  dwFlags;
	UINT32  dwTotalFrames;
	UINT32  dwInitialFrames;
	UINT32  dwStreams;
	UINT32  dwSuggestedBufferSize;
	UINT32  dwWidth;
	UINT32  dwHeight;
	UINT32  dwScale;
	UINT32  dwRate;
	UINT32  dwStart;
	UINT32  dwLength;
}AviMainHeader,*PAviMainHeader;

typedef struct tag_AviVideoHeader{
	UINT32 LIST;
	UINT32  ListSize;
	UINT32  Strl;
	UINT32  Strh;
	UINT32 StramHeaderSize;
	UINT32  fccType;//"vids"
	UINT32  fccHandler;//"mjpg"
	UINT32   dwFlags;
	UINT32   dwReserved1;
	UINT32   dwInitialFrames;
	UINT32   dwScale;
	UINT32   dwRate;
	UINT32   dwStart;
	UINT32   dwLength;
	UINT32   dwSuggestedBufferSize;
	UINT32   dwQuality;
	UINT32   dwSampleSize;
	UINT32 unkonw1;//0xa000
	UINT32 unkonw2;	//00000
} AviVideoHeader,*PAviVideoHeader;

typedef struct tag_AviVideoFormat{
	UINT32  StreamFormat;//"strf"
	UINT32  StreamFormatSize1;//0x28
	UINT32  StreamFormatSize2;//0x28
	UINT32  dwWidth;
	UINT32  dwHeight;
	UINT32 unkonw1;	// 0x01 0x00 0x18 0x00
	UINT32 Format;//"mjpeg"
	UINT32 unkonw2;//0xa000
	UINT32 unkonw3;	//00000
	UINT32 unkonw4;
	UINT32 unkonw5;
	UINT32 unkonw6;
} AviVideoFormat,*PAviVideoFormat;
typedef struct tag_AviAudioHeader{
	UINT32 LIST;
	UINT32  ListSize;
	UINT32  Strl;
	UINT32  Strh;
	UINT32 StramHeaderSize;
	UINT32  fccType;//"auds"
	UINT32  fccHandler;//"mjpg"
	UINT32   dwFlags;
	UINT32   dwReserved1;
	UINT32   dwInitialFrames;
	UINT32   dwScale;
	UINT32   dwRate;
	UINT32   dwStart;
	UINT32   dwLength;
	UINT32   dwSuggestedBufferSize;
	UINT32   dwQuality;
	UINT32   dwSampleSize;
	UINT32 unkonw1;//0xa000
	UINT32 unkonw2;	//00000
} AviAudioHeader,*PAviAudioHeader;

typedef struct tag_AviAudioFormat{
    UINT32  StreamFormat;
    UINT32  StreamFormatSize;//0x10
    UINT32  wFormatTag_nChannels;//   WORD  nChannels;
    UINT32 nSamplesPerSec;
    UINT32 nAvgBytesPerSec;
    UINT32  nBlockAlign_wBitsPerSample;
    UINT32  cbSize;
}  AviAudioFormat,*PAviAudioFormat;///WAVEFORMATEX;


typedef struct tag_AviVideoIndex{
	UINT32  dc00;//"00dc"
	UINT32  vode0;//"0x10 00 00 00"
	UINT32  Position;//0x10
	UINT32  Length;//0x01,0 0x01 0
}AviVideoIndex,*PAviVideoIndex;
typedef struct tag_AviAudioIndex{
	UINT32  wb01;//"wb01"
	UINT32  Audio;//"0x10 00 00 00"
	UINT32  Position;//0x10
	UINT32  Length;//0x01,0 0x01 0
}AviAudioIndex,*PAviAudioIndex;


typedef struct tag_AviVideoInsert{
	UINT32  dc00;//"00dc"
	UINT32  Length;

}AviVideoInsert,*PAviVideoInsert;
typedef struct tag_AudioInsert{
	UINT32  wb01;//"wb01"
	UINT32  Length;

}AviAudioInsert,*PAviAudioInsert;

typedef struct tag_AviStreamt{
	UINT32 LIST;
	UINT32  ListSize;
	UINT32  movi;
} AviStream,*PAviStream;

typedef struct tag_AviIndexHeader{
	UINT32 idx1;
	UINT32  Length;
} AviIndexHeader,*PAviIndexHeader;

/*********************************************/
typedef struct tag_AviInfo
{
	UINT16 wCaptureWidth;
	UINT16 wCaptureHeight;
	UINT16 wFrameRate;
	UINT32 dFileMaxSize;
	UINT8 bVol;	//0-15
	
	/*UINT16 wFormatTag;		//AUDIO //0X11 adpcm 0x01 pcm
	UINT16 nChannels;		// channels number 1 or 2
	UINT32 nSamplesPerSec;  //for audio Sample rate, in samples per second (Hertz),
							//at which each channel must be played or recorded.
							//Common values for nSamplesPerSec are 8.0 kilohertz (kHz), 
							//11.025 kHz, 22.05 kHz, and 44.1 kHz.
	UINT32 nAvgBytesPerSec;//Required average data-transfer rate
							//This parameter must be equal to the
							//product of nSamplesPerSec and nBlockAlign.
	UINT16	nBlockAlign;		//The block alignment is the minimum atomic unit of data for the wFormatTag format type.
	UINT16 wBitsPerSample;//Bits per sample for the wFormatTag format type
	UINT32 cbSize;	//store extra attributes for the wFormatTag
	*/
	
}AviInfo,*PAviInfo;

typedef void (*Avi_CallBack)(UINT8 Status, UINT32 Byte_Length);

 typedef enum _AVI_NowStatus
{
	AVI_OPEN =0X01,
	AVI_TIMER=0X02,
	AVI_END=0X00,
	AVI_ERROR=0X03
}AVI_NowStatus;
 
typedef struct tag_AviStatus
{
	AviInfo AviInformation;
	Avi_CallBack AviCallBack;
	HUGE UINT8 * pbFilePoint;
	UINT32 dFileLength;
	UINT8 bSaveType;
	HUGE UINT8 * TempBuf;
	HUGE UINT8 * OneFrame;//2006-12-08
	UINT32 OneFrameLength;//2006-12-08
	UINT8 SameFrameNum;//2006-12-08
	HUGE UINT8 * pIndexBufPoint;
	UINT32 dIndexLength;
	UINT32 dPosition;
	UINT32 dTotalFrame;
	UINT32  bErrorNum;	
	AVI_NowStatus NowStuatus;
}AviStatus,*PAviStatus;


#define RAMSAVE		0X0
#define ROMSAVE		0X01

#define AVISUCCESS		0
#define AVIFAIL		0X01
#define AVIERROR_NOJPEG 0X01
#define AVIERROR_OVERMS 0X02
#define AVIERROR_AUDIO 0X03


/*#define AVI_OPEN 0X01
#define AVI_TIMER 0X02
#define AVI_END 0X03
#define AVI_ERROR 0X04*/



void Avi_LoadInfo(PAviInfo Info,UINT8 bSaveType,Avi_CallBack pAvi_Callback);
UINT8 Avi_StartCapture(void *str);
UINT8 Avi_Timer(void);
UINT8 Avi_StopCapture(void);

extern AviStatus AVI_Status;
#ifdef __cplusplus
}
#endif

#endif
