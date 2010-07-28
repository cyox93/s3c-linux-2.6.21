#ifndef __AVPLAYER_H_
#define __AVPLAYER_H_



#ifdef __cplusplus
extern "C" {
#endif


#define		STATE_UNLOAD		0x1
#define		STATE_LOAD			0x2
#define		STATE_PLAY			0x3
#define		STATE_PAUSE		0x4
#define		STATE_BUSY			0x5
#define		STATE_END			0x6
#define		STATE_ERROR		0x7

#define		VMFFHEADER			0x46464D56		//VMFF File Header
#define		RIFFHEADER			0x46464952		//RIFF File Header
#define		AVIHEADER			0x20495641		//AVI File Header
#define 		VENDORVIMI			0x494d4956		//AVI vendor ID, VIMI

#define		LISTHEADER			0x5453494C		//LIST Header
#define		VIDSHEADER			0x76696473		//vids Header
#define		AUDSHEADER			0x73647561		//auds Header
#define		JUNKHEADER			0x4B4E554A		//Junk Header
#define		MOVIHEADER			0x69766F6D		//movi Header
#define		INDXHEADER			0x31786469		//index1 Header
#define		STRFHEADER			0x66727473		//strf Header

#define		MMP3_MODE			0x00			//Mjpeg+Mp3
#define		MMIDI_MODE			0x01			//Mjpeg+Midi
#define		AVI_MODE			0x02			//Mjpeg+Adpcm



typedef struct __KAVVERSION{
	UINT8	bM_Ver;
	UINT8	bS_Ver;
	UINT8	bC_Ver;
	
	UINT8	bYear;		//Based on 2000
	UINT8	bMonth;
	UINT8	bDay;
} KAVVERSION, *pKAVVERSION;


typedef struct __KHEADER
{
	UINT32	dSize;
	UINT32	dMode;
}KHEADER, *pKHEADER;

typedef struct __KVIDEO
{
	UINT32	dSize;
	UINT32	dPerFrame;
	UINT32	dBufSize;
	UINT32	dIndexFlag;
	UINT32	dIndexOffset;
	UINT32	dTotalFrame;
	UINT32	dStartTime;
	UINT32	dStartX;
	UINT32	dStartY;
	UINT32	dWidth;
	UINT32	dHeight;
	UINT32	dTicker;
	UINT32	dReserve[4];
}KVIDEO, *pKVIDEO;


typedef struct __KAMP3
{
	UINT32	dSize;
	UINT32	dMp3Size;
	UINT32	dFrameTime;
	UINT32	dTotalTime;
	UINT32	dFrameLength;
	UINT32	dTotalFrame;
	UINT32	dIndexFlag;
	UINT32	dIndexOffset;
	UINT32	dSampleRate;
	UINT32	dBitrate;
	UINT32	dReserve[4];
}KAMP3, *pKAMP3;

typedef struct __KAMIDI
{
	UINT32	dSize;
	UINT32	dTotalTime;
	UINT32	dFileType;
	UINT32	dMidiType;
	UINT32	dTrackNum;
	UINT32	dIndexFlag;
	UINT32	dIndexOffset;
	UINT32	dReserve[7];
}KAMIDI, *pKAMIDI;



typedef struct __KVAVMP3
{
	UINT32	dPauseTime;				//存放Pause的时间
	UINT32	dRepeat;				//存放重复播放的时间
	UINT32	dBufSize;				//放Jpeg的Buffer的大小
	UINT32	dMp3Size;				//在AV文件中实际的MP3数据的大小
	UINT32	dMp3Offset;				//在AV文件中实际的MP3的偏移量
	UINT32	dTotalTime;				//在AV文件中的总时间
	UINT32	dFrameLength;				//在AV文件中一桢的长度
	UINT32	dTotalFrame;				//在AV文件中MP3的总桢数
	UINT32	dSampleRate;				//在AV文件中MP3的采样率
	UINT32	dBitRate;				//在AV文件中MP3的Bitrate
	UINT32	dRealOffset;				//在AV文件中的偏移量
	UINT32	dMp3RealOffset;				//在AV文件中实际的MP3数据的偏移量
	UINT32	dFrameOffset;				//在AV文件中MP3桢内的偏移量
	UINT32	dTicker[2];				//在AV文件中Jpeg显示的时间
	UINT32	dBufferSize[2];				//在AV文件中存放的两个Jpeg文件的大小
	UINT8	bBufFlag[2];				//标示bBuffer中是否存有Jpeg文件  0:没有;  1:有
	UINT8*	pbBuffer[2];				//在AV文件中用于存放两张连续的Jpeg文件
	UINT8	bFramePad;				//在AV文件中目前的MP3桢中是否有Padding
}KVAVMP3, *pKVAVMP3;

typedef struct __KVAVMIDI
{
	UINT32	dSize;					//在AV文件中Jpeg部分数据的大小
	UINT32	dOffset;				//在AV文件中Jpeg部分数据的偏移量
	UINT32	dBufSize;				//放Jpeg的Buffer的大小
	UINT32	dTicker[2];				//在AV文件中Jpeg显示的时间
	UINT32	dBufferSize[2];				//在AV文件中存放的两个Jpeg文件的大小
	UINT8*	pbSrcPtr;				//指向AV文件中JPEG数据开始的地方
	UINT8*	pbJpegPtr;				//指向AV文件中JPEG数据开始的地方,随后会在读取Jpeg的过程中移动
	UINT8	bBufFlag[2];				//标示bBuffer中是否存有Jpeg文件  0:没有;  1:有
	UINT8*	pbBuffer[2];				//在AV文件中用于存放两张连续的Jpeg文件
}KVAVMIDI, *pKVAVMIDI;


typedef struct __KAVIINFO
{
	UINT32	dTotalTime;				//AVI Total Time
	UINT32	dPerFrame;				//Video Frame rate
	UINT32	dTotalFrame;				//Video Total Frame
	UINT32	dSampleRate;				//Audio Sample Rate
	UINT32	dBlockSize;				//Audio ADPCM Block Size
	UINT32	dWidth;					//Jpeg Width
	UINT32	dHeight;				//Jpeg Height
	UINT32	dDataOffset;				//Video/Audio Data Start Position
	UINT32	dIndexNum;				//Number of Index(include Video index and Audio index)
	UINT32	dAIndexc;				//Audio Index Counter
	UINT32	dVIndexc;				//Video Index Counter
	UINT32	dAIndex;				//Number of Audio Index
	UINT32	dVIndex;				//Number of Video index
	UINT32	dChunkOffset;				//Audio offset in Chunk
	UINT32	dJpegSize[2];				//The Size of every jpeg buffer
	UINT8	bOffsetFlag;				//0: Offset from 'RIFF' header; 1: Offset from 'movi' header
	UINT8	bFlag;					//Oder Flag:	0: vids(00), auds(01); 1: vids(01), auds(00)
	UINT8	bIndexFlag;				//Index Flag:	0: No; 1: Yes
	UINT8	bInterFlag;				//Interleave Flag: 0: No; 1: Yes
	UINT8	bChannel;				//Audio Channel: 1: mono; 2: stero
	UINT8	bSource;				//Data Source: 0: RAM; 1: ROM
	UINT8*	pbSptr;					//Address of Data
	UINT8*	pbIndex;				//Address of index1
	UINT8*	pbAIndex;				//Address of Audio Index
	UINT8*	pbVIndex;				//Address of Video Index
	UINT8*	pbBuffer[2];				//Jpeg Buffer
	
}KAVIINFO, *pKAVIINFO;
/** API result enum  */
typedef enum _VRESULT
{

	VRES_SUCCESS							=   0,
	VRES_ERROR								= -1,
	VRES_OUT_BUFFER_FULL					= -2,
	VMAMIDI_INVALID_DATA					= -3,
	VRES_BUFFER_NULL						= -4,
	VRES_FILE_TOO_SMALL					= -5,
	VRES_FILE_TOO_LARGE					= -6,
	VRES_HWEVTP_EMPTY					= -7,
	VRES_EVTP_EMPTY						= -8,
	VRES_END_FLAG							= -9,
	VRES_HAVE_SAME_FILE_TYPE				= -10,
	VRES_OUT_OF_STREAMFIFO				= -11,
	VRES_STREAM_PORT_ENABLE				= -12,
	VRES_REVERB_ACTIVE					= -13,
	VRES_REVERB_NOT_START				= -14,
	VRES_SPORT_NOT_OPEN					= -15,
	VRES_SPORT_NOT_STOP					= -16,
	VRES_SPORT_NOT_START					= -17,
	VRES_THE_PARSER_NOT_FOUND			= -18,
	VRES_SEEK_TO_END						= -19,
	VRES_INVALID_INPUT_PARAMETERS		= -20,
	VRES_NOT_SEEKABLE						= -21,
	VRES_WAVE_NOT_LOADED_ALL			= -22,
	VRES_SPORT_LOCKED						= -23,
	VRES_ERROR_MUSIC_TYPE				= -24,
	VRES_QUEUE_EMPTY						= - 25,
	VRES_WAVEMSG_ERROR						= -26,
	VRES_LAYER_I_II								= -27,			//ADDED BY zl 2006.01.11
	VRES_MPEG_2_5                 = -28,			//ADDED BY zl 2006.01.11
	VRES_PLR_OCCUPY						= -29,
	VRES_RESERVED							= -99,
	VRES_SMAF_WAVE_INFO2_DATA_UNLOAD	= -100,
	VRES_SMAF_ATR_WATE_FINISH       		= -101,
	VRES_VMD_INVALID_DATA				= -102,
 	VRES_VMD_SMAF_LOAD_NOT_SMAF_FILE	= -103,
    VRES_ERROR_PARAMETERS				  	= -104,
    VRES_SYNTH_HAS_CREATED				= -105,
    VRES_SYNTH_NO_EXIST					= -106,
    VRES_ERROR_AUDIO_MODE				= -107,
    VRES_MP3FIFO_NOT_FREE				= -108,
    VRES_STMFIFO0_NOT_FREE				= -109,
    VRES_STMFIFO1_NOT_FREE				= -110,
    VRES_STMFIFO2_NOT_FREE				= -111,
    VRES_STMFIFO3_NOT_FREE				= -112,
    VRES_EVENTFIFO_NOT_FREE				= -113,
    VRES_COMMANDFIFO_NOT_FREE			= -114,
    VRES_STREAM_DATA_NO_EXIST			= -115,
    VRES_STREAM_DATA_END				= -116,
    VRES_COMMANDFIFO_NOT_EMPTY			= -117,
    VRES_ERROR_IPCU_CLASSID				= -118,
    VRES_ERROR_IPCU_MESSAGEID			= -119,
    VRES_IPCU_ERRORID				= -120,
    VRES_ERROR_TIMEOUT					= -121,
    VRES_ERROR_READBBUFF				= -122,
    VRES_FAT_RES_TIMEOUT				= -123,
    VRES_NAND_FULL						= -124

}VRESULT;
typedef		void (*PEFUNCTION)(void );
#define DBGAVFILE				0
#define CHECK_VENDOR			0			// AVI check vendor, only support VIMI
#define AV_FILEBUF_LENGTH		0xC000		// 48K bytes
// support mobile flash or not 
#define 	SUPPORT_FLASH			0
#define BIG_ENDIAN				0			// 1: big endian; 0: little endian

typedef struct __AVFILEBUF
{
	SINT32	Cursor_read;
	SINT32 	Cursor_write;
	void* pFile_Ptr;
	SINT32  filelength;
	UINT8 AVfilebuf[AV_FILEBUF_LENGTH];
}AVFILEBUF, *pAVFILEBUF;
//define MAXJPGSIZE 0x5000		// 20K for one JPG file

// Use the following macro, adjust audio synchronization, either audio or video ended, the AVI will
// stop play
// unit ms, the different start point between audio and video, audio SP=0, video Start Point can be adjust
#define TIME_START_POINT_ADJUST				(00)		

typedef struct __TIMEINFO
{
	SINT32	dwBase;		// paused time base
	SINT32 	dwAudio;	// audio data time, write to ADPCM FIFO, caculated by data size
	SINT32 	dwSystem;	// current system time, global clock, used for video synchronize
}TIMEINFO, *pTIMEINFO;

SINT32 VPlr_AVFileBuf_Fill(void* pFile_Ptr, SINT32  filelength);
SINT32 VPlr_AVFileBuf_Init(void* pFile_Ptr, SINT32  filelength);
SINT32 VPlr_AVFileBuf_Read( UINT8* pbBuffer, UINT32 d_Size);
void VPlr_AVFileBuf_CursorSet( SINT32 RorW, SINT32 offset);
void VPlr_AVFileBuf_FileCursorSet( SINT32 step );
void VPlr_AVFileBuf_SetTimePoint( UINT32 TimePoint);


//VRESULT	AVlr_Initialize(void);
VRESULT  AVlr_Load(HUGE UINT8 *pbFilePtr, UINT32 dFileSize, UINT8 bSrc, HUGE UINT8* pIndexBuf);
VRESULT	AVlr_Unload(void);
VRESULT	AVlr_Play(UINT32 dSeekPos, UINT32 dRepeat, PEFUNCTION pFunc);
VRESULT	AVlr_Stop(void);
VRESULT	AVlr_Pause(void);
VRESULT	AVlr_Resume(void);
UINT32	AVlr_GetTotalTime(void);
UINT32	AVlr_GetCurTime(void);
UINT32   AVlr_GetCurrentStaus(void );

void AVDisplaySize(UINT16 x, UINT16 y, UINT16 width, UINT16 height);
#ifdef __cplusplus
}
#endif

#endif
