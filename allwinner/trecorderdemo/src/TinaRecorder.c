#include "TinaRecorder.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <CdxQueue.h>
#include <AwPool.h>
#include <CdxBinary.h>
#include <CdxMuxer.h>
#include "memoryAdapter.h"
#include "awencoder.h"
#include "RecorderWriter.h"

typedef struct DemoRecoderContext
{
    AwEncoder*       mAwEncoder;
    VideoEncodeConfig videoConfig;
    AudioEncodeConfig audioConfig;
    CdxMuxerT* pMuxer;
    int muxType;
    char mSavePath[1024];
    CdxWriterT* pStream;
    unsigned char* extractDataBuff;
    unsigned int extractDataLength;
    pthread_t muxerThreadId ;
    pthread_t audioDataThreadId ;
    pthread_t videoDataThreadId ;
    AwPoolT *mDataPool;
	mAudioDataQueue *dataQueue;
    int  mRecordDuration;
    int videoEos;
    int audioEos;
    bool muxThreadExitFlag;
}DemoRecoderContext;

typedef struct
{
	int (*setVideoInPort)(void *hdl,vInPort *vPort);
	int (*setAudioInPort)(void *hdl,aInPort *aPort);
	int (*setDispOutPort)(void *hdl,dispOutPort *dispPort);
	int (*setOutput)(void *hdl,rOutPort *output);

	int (*start)(void *hdl);
	int (*stop)(void *hdl);
	int (*release)(void *hdl);
	int (*prepare)(void *hdl);
	int (*reset)(void *hdl);

	int (*setEncodeSize)(void *hdl,int width,int height);
	int (*setEncodeBitRate)(void *hdl,int bitrate);
	int (*setOutputFormat)(void *hdl,int format);
	int (*setCallback)(void *hdl,Rcallback callback);
	int (*setEncodeVideoFormat)(void *hdl,int format);
	int (*setEncodeAudioFormat)(void *hdl,int format);

	int (*setEncodeFramerate)(void *hdl,int framerate);

	int (*getRecorderTime)(void *hdl);
	int (*captureCurrent)(void *hdl,char *path,int format);
	int (*setMaxRecordTime)(void *hdl,int ms);
	int (*setParameter)(void *hdl,int cmd,int parameter);

	int encodeWidth;
	int encodeHeight;
	int encodeBitrate;
	int encodeFramerate;
	int outputFormat;
	Rcallback callback;
	int encodeVFormat;
	int encodeAFormat;
	int maxRecordTime;
	vInPort vport;
	aInPort aport;
	dispOutPort dispport;
	rOutPort outport;
	void *EncoderContext;
}MediaRecorder;

static void NotifyCallbackForAwEncorder(void* pUserData, int msg, void* param)
{
	MediaRecorder *hdlTina = (MediaRecorder *)param;
	if(hdlTina == NULL)
		return -1;
    DemoRecoderContext* pDemoRecoder = (DemoRecoderContext*)hdlTina->EncoderContext;
	dataParam dataparam;
	memset(&dataparam, 0, sizeof(dataParam));

    switch(msg)
    {
        case AWENCODER_VIDEO_ENCODER_NOTIFY_RETURN_BUFFER:
        {
            int id = *((int*)param);
			dataparam.bufferId = id;
			hdlTina->vport.queue(&dataparam);
			break;
        }
        default:
        {
            printf("warning: unknown callback from AwRecorder.\n");
            break;
        }
    }

    return ;
}

int onVideoDataEnc(void *app,CdxMuxerPacketT *buff)
{
	MediaRecorder *hdlTina = (MediaRecorder *)app;
	if(hdlTina == NULL)
		return -1;
    DemoRecoderContext* pDemoRecoder = (DemoRecoderContext*)hdlTina->EncoderContext;

    CdxMuxerPacketT *packet = NULL;
    if (!buff)
        return 0;

    packet = (CdxMuxerPacketT*)malloc(sizeof(CdxMuxerPacketT));
    packet->buflen = buff->buflen;
    packet->length = buff->length;
    packet->buf = malloc(buff->buflen);
    memcpy(packet->buf, buff->buf, packet->buflen);
    packet->pts = buff->pts;
    packet->type = buff->type;
    packet->streamIndex  = buff->streamIndex;
    packet->duration = buff->duration;

    CdxQueuePush(pDemoRecoder->dataQueue,packet);
    return 0;
}

int onAudioDataEnc(void *app,CdxMuxerPacketT *buff)
{
	MediaRecorder *hdlTina = (MediaRecorder *)app;
	if(hdlTina == NULL)
		return -1;
    DemoRecoderContext* pDemoRecoder = (DemoRecoderContext*)hdlTina->EncoderContext;
    CdxMuxerPacketT *packet = NULL;
    if (!buff)
        return 0;
    packet = (CdxMuxerPacketT*)malloc(sizeof(CdxMuxerPacketT));
    packet->buflen = buff->buflen;
    packet->length = buff->length;
    packet->buf = malloc(buff->buflen);
    memcpy(packet->buf, buff->buf, packet->buflen);
    packet->pts = buff->pts;
    packet->type = buff->type;
    packet->streamIndex = buff->streamIndex;
    packet->duration = buff->duration;
    CdxQueuePush(pDemoRecoder->dataQueue,packet);

    return 0;
}


void* MuxerThread(void *param)

{
	MediaRecorder *hdlTina = (MediaRecorder *)param;
	if(hdlTina == NULL)
		return -1;
    DemoRecoderContext* p = (DemoRecoderContext*)hdlTina->EncoderContext;
    CdxMuxerPacketT *mPacket = NULL;
    RecoderWriterT *rw;
    char rm_cmd[MAX_URL_LEN + 4];
    CdxMuxerMediaInfoT mediainfo;
    int fs_cache_size = 0;

    if(p->mSavePath)
    {
        if ((p->pStream = CdxWriterCreat(p->mSavePath)) == NULL)
        {
            printf("CdxWriterCreat() failed\n");
            return 0;
        }
        rw = (RecoderWriterT*)p->pStream;
        rw->file_mode = FD_FILE_MODE;
        strncpy(rw->file_path, p->mSavePath,MAX_URL_LEN);
        if(access(rw->file_path,F_OK) == 0){
            printf("rm the exit file: %s \n",rw->file_path);
            strcpy(rm_cmd,"rm ");
            strcat(rm_cmd,rw->file_path);
            system(rm_cmd);
        }
        RWOpen(p->pStream);
		p->pMuxer = CdxMuxerCreate(p->muxType, p->pStream);
		if(p->pMuxer == NULL)
		{
	            printf("CdxMuxerCreate failed\n");
	            return 0;
		}
    }

    memset(&mediainfo, 0, sizeof(CdxMuxerMediaInfoT));
    if(hdlTina->aport.enable)
	{
		switch (p->audioConfig.nType)
		{
            case AUDIO_ENCODE_PCM_TYPE:
				mediainfo.audio.eCodecFormat = AUDIO_ENCODER_PCM_TYPE;
				break;
            case AUDIO_ENCODE_AAC_TYPE:
				mediainfo.audio.eCodecFormat = AUDIO_ENCODER_AAC_TYPE;
				break;
            case AUDIO_ENCODE_MP3_TYPE:
				mediainfo.audio.eCodecFormat = AUDIO_ENCODER_MP3_TYPE;
				break;
            case AUDIO_ENCODE_LPCM_TYPE:
				mediainfo.audio.eCodecFormat = AUDIO_ENCODER_LPCM_TYPE;
				break;
            default:
				printf("unlown audio type(%d)\n", p->audioConfig.nType);
				break;
		}
	    mediainfo.audioNum = 1;
		mediainfo.audio.nAvgBitrate = p->audioConfig.nBitrate;
		mediainfo.audio.nBitsPerSample = p->audioConfig.nSamplerBits;
		mediainfo.audio.nChannelNum = p->audioConfig.nOutChan;
		mediainfo.audio.nMaxBitRate = p->audioConfig.nBitrate;
		mediainfo.audio.nSampleRate = p->audioConfig.nOutSamplerate;
		mediainfo.audio.nSampleCntPerFrame = 1024; // 固定的,一帧数据的采样点数目
    }

    if(hdlTina->vport.enable && (p->muxType == CDX_MUXER_MOV || p->muxType == CDX_MUXER_TS))
	{
		mediainfo.videoNum = 1;
		if(p->videoConfig.nType == VIDEO_ENCODE_H264)
		    mediainfo.video.eCodeType = VENC_CODEC_H264;
		else if(p->videoConfig.nType == VIDEO_ENCODE_JPEG)
		    mediainfo.video.eCodeType = VENC_CODEC_JPEG;
		else
		{
	            printf("cannot suppot this video type\n");
	            return 0;
		}
		mediainfo.video.nWidth    =  p->videoConfig.nOutWidth;
		mediainfo.video.nHeight   =  p->videoConfig.nOutHeight;
		mediainfo.video.nFrameRate = p->videoConfig.nFrameRate;
		oneMinuteVideoFrameCount = p->videoConfig.nFrameRate*60;
		printf("oneMinuteVideoFrameCount = %d\n",oneMinuteVideoFrameCount);
    }

    printf("******************* mux mediainfo *****************************\n");
    printf("videoNum                   : %d\n", mediainfo.videoNum);
    printf("videoTYpe                  : %d\n", mediainfo.video.eCodeType);
    printf("framerate                  : %d\n", mediainfo.video.nFrameRate);
    printf("width                      : %d\n", mediainfo.video.nWidth);
    printf("height                     : %d\n", mediainfo.video.nHeight);
    printf("audioNum                   : %d\n", mediainfo.audioNum);
    printf("audioFormat                : %d\n", mediainfo.audio.eCodecFormat);
    printf("audioChannelNum            : %d\n", mediainfo.audio.nChannelNum);
    printf("audioSmpleRate             : %d\n", mediainfo.audio.nSampleRate);
    printf("audioBitsPerSample         : %d\n", mediainfo.audio.nBitsPerSample);
    printf("**************************************************************\n");

    if(p->pMuxer)
    {
		CdxMuxerSetMediaInfo(p->pMuxer, &mediainfo);
#if FS_WRITER
        fs_cache_size = 1024 * 1024;
        CdxMuxerControl(p->pMuxer, SET_FS_SIMPLE_CACHE_SIZE, &fs_cache_size);
        int fs_mode = FSWRITEMODE_SIMPLECACHE;
        CdxMuxerControl(p->pMuxer, SET_FS_WRITE_MODE, &fs_mode);
#endif
    }

    if(p->extractDataLength > 0 && p->pMuxer)
    {
        if(p->pMuxer)
		{
            CdxMuxerWriteExtraData(p->pMuxer, p->extractDataBuff, p->extractDataLength, 0);
		}
    }

    if(p->pMuxer)
    {
		CdxMuxerWriteHeader(p->pMuxer);
    }

	while(p->audioEos==0) || p->videoEos==0)
	{
		if(!hdlTina->vport.enable && p->audioEos)
			break;

		if(!hdlTina->aport.enable && p->videoEos)
			break;

		while (!CdxQueueEmpty(p->dataQueue))
		{
			mPacket = CdxQueuePop(p->dataQueue);
			if(p->pMuxer)
			{
				if(CdxMuxerWritePacket(p->pMuxer, mPacket) < 0)
				{
					loge("+++++++ CdxMuxerWritePacket failed");
				}
			}

			free(mPacket->buf);
			free(mPacket);

		}
		usleep(1*1000);
	}

    if(p->pMuxer)
    {
        CdxMuxerWriteTrailer(p->pMuxer);
    }

    if(p->pMuxer)
    {
		CdxMuxerClose(p->pMuxer);
        p->pMuxer = NULL;
    }
    if(p->pStream)
    {
        RWClose(p->pStream);
        CdxWriterDestroy(p->pStream);
        p->pStream = NULL;
        rw = NULL;
    }

    p->muxThreadExitFlag = true;
    return 0;
}

void* AudioInputThread(void *param)
{
    int ret = 0;
	MediaRecorder *hdlTina = (MediaRecorder *)param;
	if(hdlTina == NULL)
		return -1;
    DemoRecoderContext* p = (DemoRecoderContext*)hdlTina->EncoderContext;

    int num = 0;
    int size2 = 0;
    int64_t audioPts = 0;
	int readOnceSize = 1024*4;
	int readActual = 0;

    AudioInputBuffer audioInputBuffer;
    memset(&audioInputBuffer, 0x00, sizeof(AudioInputBuffer));

    audioInputBuffer.nLen = readOnceSize;
    audioInputBuffer.pData = (char *)malloc(audioInputBuffer.nLen);

    while(1)
    {
		if(!p->audioEos)
		{
		    readActual = hdlTina->aport.readData(audioInputBuffer.pData,readOnceSize);
			audioInputBuffer.nLen = readActual;
			audioInputBuffer.nPts = hdlTina->aport.getpts();

            while(ret < 0)
            {
				ret = AwEncoderWritePCMdata(p->mAwEncoder,&audioInputBuffer);
				usleep(10*1000);
		    }
            usleep(20*1000);
		}
		else
		{
			break;
		}
    }
    p->audioEos = 1;
    return 0;
}

void* VideoInputThread(void *param)
{
    int ret = 0;
	MediaRecorder *hdlTina = (MediaRecorder *)param;
	if(hdlTina == NULL)
		return -1;
    DemoRecoderContext* p = (DemoRecoderContext*)hdlTina->EncoderContext;
    struct ScMemOpsS* memops = NULL;

    VideoInputBuffer videoInputBuffer;
    int sizeY = p->videoConfig.nSrcHeight* p->videoConfig.nSrcWidth;

    long long videoPts = 0;
	char *addr;
	int bufSize = 0;
	dataParam paramData;

    int ret = -1;
    int num = 0;
    int llh_sizey = 0;
    int llh_sizec = 0;
    while(1)
    {
		ret = -1;
		if(!p->videoConfig.bUsePhyBuf)
			goto VIDEO_INPUT_BUFFER;

		hdlTina->vport.dequeue(&videoInputBuffer.pAddrPhyY,&bufSize,&paramData);
		videoInputBuffer.nID = paramData.bufferId.;
		videoInputBuffer.nPts = paramData.pts;
		videoInputBuffer.nLen = bufSize;

		while(ret < 0)
		{
            ret = AwEncoderWriteYUVdata(p->mAwEncoder,&videoInputBuffer);
            if(ret < 0)
			{
				printf("write yuv data retry!\n");
	        }
	        usleep(10*1000);
		}
    }

    p->videoEos = 1;
    return 0;
}


MediaRecorder *CreateTinaRecorder()
{
	MediaRecorder *tinaRec = (MediaRecorder *)malloc(sizeof(MediaRecorder));
	if(tinaRec == NULL)
		return NULL;
	memset(tinaRec, 0, sizeof(MediaRecorder));
	tinaRec->setVideoInPort			=	TinasetVideoInPort;
	tinaRec->setAudioInPort			=	TinasetAudioInPort;
	tinaRec->setDispOutPort			=	TinasetDispOutPort;
	tinaRec->setOutput			=	TinasetOutput;
	tinaRec->start				=	Tinastart;
	tinaRec->stop				=	Tinastop;
	tinaRec->release			=	Tinarelease;
	tinaRec->reset				=	Tinareset;
	tinaRec->setEncodeSize			=	TinasetEncodeSize;
	tinaRec->setEncodeBitRate		=	TinasetEncodeBitRate;
	tinaRec->setOutputFormat		=	TinasetOutputFormat;
	tinaRec->setCallback			=	TinasetCallback;
	tinaRec->setEncodeVideoFormat	=   TinasetEncodeVideoFormat;
	tinaRec->setEncodeAudioFormat   =   TinasetEncodeAudioFormat;
	tinaRec->getRecorderTime        =   TinagetRecorderTime;
	tinaRec->captureCurrent         =   TinacaptureCurrent;
	tinaRec->setMaxRecordTime       =   TinasetMaxRecordTime;
	tinaRec->setParameter           =   TinasetParameter;

	return tinaRec;
}

int DestroyTinaRecorder(MediaRecorder *hdl)
{
	free(hdl);
	return 0;
}

int TinasetVideoInPort(void *hdl,vInPort *vPort)
{
	MediaRecorder *hdlTina = (MediaRecorder *)hdl;
	if(hdl == NULL)
		return -1;

	if(vInPort != NULL)
		memcpy(&hdlTina->vport, vPort, sizeof(vInPort));
	else
		hdlTina->vport.enable = 0;

	return 0;
}

int TinasetAudioInPort(void *hdl,aInPort *aPort)
{
	MediaRecorder *hdlTina = (MediaRecorder *)hdl;
	if(hdl == NULL)
		return -1;

	if(aInPort != NULL)
		memcpy(&hdlTina->aport, aPort, sizeof(aInPort));
	else
		hdlTina->aport.enable = 0;

	return 0;
}

int TinasetDispOutPort(void *hdl,dispOutPort *dispPort)
{
	MediaRecorder *hdlTina = (MediaRecorder *)hdl;
	if(hdl == NULL)
		return -1;

	if(dispPort != NULL)
		memcpy(&hdlTina->dispport,dispPort,sizeof(dispOutPort));
	else
		hdlTina->dispport.enable = 0;

	return 0;
}

int TinasetOutput(void *hdl,rOutPort *output)
{
	MediaRecorder *hdlTina = (MediaRecorder *)hdl;
	if(hdl == NULL)
		return -1;

	if(output != NULL)
		memcpy(&hdlTina->outport,output,sizeof(rOutPort));
	else
		hdlTina->outport.enable = 0;

	return 0;
}

int TinasetEncodeSize(void *hdl,int width,int height)
{
	MediaRecorder *hdlTina = (MediaRecorder *)hdl;
	if(hdl == NULL)
		return -1;

	hdlTina->encodeWidth = width;
	hdlTina->encodeHeight = height;
	return 0;
}

int TinasetEncodeBitRate(void *hdl,int bitrate)
{
	MediaRecorder *hdlTina = (MediaRecorder *)hdl;
	if(hdl == NULL)
		return -1;

	hdlTina->encodeBitrate = bitrate;
	return 0;
}

int TinasetOutputFormat(void *hdl,int format)
{
	MediaRecorder *hdlTina = (MediaRecorder *)hdl;
	if(hdl == NULL)
		return -1;

	hdlTina->outputFormat = format;
	return 0;
}
int TinasetCallback(void *hdl,Rcallback callback)
{
	MediaRecorder *hdlTina = (MediaRecorder *)hdl;
	if(hdl == NULL)
		return -1;

	hdlTina->callback = callback;
	return 0;
}


int TinasetEncodeVideoFormat(void *hdl,int format)
{
	MediaRecorder *hdlTina = (MediaRecorder *)hdl;
	if(hdl == NULL)
		return -1;
}

int TinasetEncodeAudioFormat(void *hdl,int format)
{
	MediaRecorder *hdlTina = (MediaRecorder *)hdl;
	if(hdl == NULL)
		return -1;
	hdlTina->encodeAFormat = format;
	return 0;
}

int TinagetRecorderTime(void *hdl)
{
	MediaRecorder *hdlTina = (MediaRecorder *)hdl;
	if(hdl == NULL)
		return -1;
}

int TinacaptureCurrent(void *hdl,char *path,int format)
{
	MediaRecorder *hdlTina = (MediaRecorder *)hdl;
	if(hdl == NULL)
		return -1;
}

int TinasetMaxRecordTime(void *hdl,int ms)
{
	MediaRecorder *hdlTina = (MediaRecorder *)hdl;
	if(hdl == NULL)
		return -1;
	hdlTina->maxRecordTime = ms;
	return 0;
}

int TinasetParameter(void *hdl,int cmd,int parameter)
{
	MediaRecorder *hdlTina = (MediaRecorder *)hdl;
	if(hdl == NULL)
		return -1;

	return 0;
}

int Tinastart(void *hdl)
{
	MediaRecorder *hdlTina = (MediaRecorder *)hdl;
	if(hdl == NULL)
		return -1;
	DemoRecoderContext *demoRecoder = (DemoRecoderContext *)hdl->EncoderContext;

    //start encode
    AwEncoderStart(demoRecoder->mAwEncoder);

    AwEncoderGetExtradata(demoRecoder->mAwEncoder,&demoRecoder->extractDataBuff,&demoRecoder->extractDataLength);

    if(hdlTina->aport.enable){
        pthread_create(&demoRecoder->audioDataThreadId, NULL, AudioInputThread, hdl);
    }

    if(hdlTina->vport.enable && (demoRecoder->muxType == CDX_MUXER_MOV || demoRecoder[i].muxType == CDX_MUXER_TS))
    {
        pthread_create(&demoRecoder[i].videoDataThreadId, NULL, VideoInputThread, hdl);
    }

	if(hdlTina->outport.enable)
	{
	    pthread_create(&demoRecoder->muxerThreadId, NULL, MuxerThread, hdl);
	}
}

int Tinastop(void *hdl)
{

}

int Tinarelease(void *hdl)
{

}

int Tinaprepare(void *hdl)
{
	MediaRecorder *hdlTina = (MediaRecorder *)hdl;
	if(hdl == NULL)
		return -1;

    EncDataCallBackOps mEncDataCallBackOps;
    mEncDataCallBackOps.onAudioDataEnc = onAudioDataEnc;
    mEncDataCallBackOps.onVideoDataEnc = onVideoDataEnc;
    //* create the demoRecoder.
    DemoRecoderContext *demoRecoder = (DemoRecoderContext *)malloc(sizeof(DemoRecoderContext));
	if(demoRecoder == NULL)
		goto error_release;
    memset(demoRecoder, 0, sizeof(DemoRecoderContext));
	hdl->EncoderContext = (void *)demoRecoder;

    //create the queue to store the encoded video data and encoded audio data
    demoRecoder->mDataPool = AwPoolCreate(NULL);
	demoRecoder->dataQueue = CdxQueueCreate(demoRecoder->mDataPool);

    //the total record duration
    demoRecoder->mRecordDuration = hdlTina->maxRecordTime/1000;

	switch(hdlTina->outputFormat)
	{
		case TR_OUTPUT_TS:
		demoRecoder->muxType       = CDX_MUXER_TS;
			break;
		case TR_OUTPUT_AAC:
			if(hdlTina->vport.enable != 0)
			{
				printf("vport not disable when record AAC!\n");
				goto error_release;
			}
		demoRecoder->muxType       = CDX_MUXER_AAC;
			break;
		case TR_OUTPUT_MP3:
			if(hdlTina->vport.enable != 0)
			{
				printf("vport not disable when record MP3!\n");
				goto error_release;
			}
		demoRecoder->muxType       = CDX_MUXER_MP3;
			break;
		case TR_OUTPUT_MOV:
            demoRecoder->muxType       = CDX_MUXER_MOV;
			break;
		case TR_OUTPUT_JPG:
			if(hdlTina->aport.enable != 0)
			{
				printf("aport not disable when taking picture!\n");
				goto error_release;
			}
			break;
		default:
			printf("output format not supported!\n");
			goto error_release;
			break;
	}

        //config VideoEncodeConfig
    memset(&demoRecoder->videoConfig, 0x00, sizeof(VideoEncodeConfig));
    if(hdlTina->vport.enable && (demoRecoder->muxType == CDX_MUXER_MOV || demoRecoder->muxType == CDX_MUXER_TS))
	{
		switch(hdlTina->vport.format)
		{
			case TR_VIDEO_H264:
				demoRecoder->videoConfig.nType	   = VIDEO_ENCODE_H264;
				break;
			case TR_VIDEO_MJPEG:
				demoRecoder->videoConfig.nType	   = VIDEO_ENCODE_JPEG;
				break;
			default:
				printf("video encoder format not support!\n");
				goto error_release;
				break;
		}

		switch(hdlTina->vport.format)
		{
			case TR_PIXEL_YUV420SP:
				demoRecoder->videoConfig.nInputYuvFormat = VIDEO_PIXEL_YUV420_NV12;
				break;
			case TR_PIXEL_YVU420SP:
				demoRecoder->videoConfig.nInputYuvFormat = VIDEO_PIXEL_YUV420_NV21;
				break;
			case TR_PIXEL_YUV420P:
				demoRecoder->videoConfig.nInputYuvFormat = VIDEO_PIXEL_YUV420_YU12;
				break;
			case TR_PIXEL_YVU420P:
				demoRecoder->videoConfig.nInputYuvFormat = VIDEO_PIXEL_YVU420_YV12;
				break;
			case TR_PIXEL_YUV422SP:
				demoRecoder->videoConfig.nInputYuvFormat = VIDEO_PIXEL_YUV422SP;
				break;
			case TR_PIXEL_YVU422SP:
				demoRecoder->videoConfig.nInputYuvFormat = VIDEO_PIXEL_YVU422SP;
				break;
			case TR_PIXEL_YUV422P:
				demoRecoder->videoConfig.nInputYuvFormat = VIDEO_PIXEL_YUV422P;
				break;
			case TR_PIXEL_YVU422P:
				demoRecoder->videoConfig.nInputYuvFormat = VIDEO_PIXEL_YVU422P;
				break;
			case TR_PIXEL_YUYV422:
				demoRecoder->videoConfig.nInputYuvFormat = VIDEO_PIXEL_YUYV422;
				break;
			case TR_PIXEL_UYVY422:
				demoRecoder->videoConfig.nInputYuvFormat = VIDEO_PIXEL_UYVY422;
				break;
			case TR_PIXEL_YVYU422:
				demoRecoder->videoConfig.nInputYuvFormat = VIDEO_PIXEL_YVYU422;
				break;
			case TR_PIXEL_VYUY422:
				demoRecoder->videoConfig.nInputYuvFormat = VIDEO_PIXEL_VYUY422;
				break;
			case TR_PIXEL_ARGB:
				demoRecoder->videoConfig.nInputYuvFormat = VIDEO_PIXEL_ARGB;
				break;
			case TR_PIXEL_RGBA:
				demoRecoder->videoConfig.nInputYuvFormat = VIDEO_PIXEL_RGBA;
				break;
			case TR_PIXEL_ABGR:
				demoRecoder->videoConfig.nInputYuvFormat = VIDEO_PIXEL_ABGR;
				break;
			case TR_PIXEL_BGRA:
				demoRecoder->videoConfig.nInputYuvFormat = VIDEO_PIXEL_BGRA;
				break;
			case TR_PIXEL_TILE_32X32:
				demoRecoder->videoConfig.nInputYuvFormat = VIDEO_PIXEL_YUV420_MB32;
				break;
			case TR_PIXEL_TILE_128X32:
				demoRecoder->videoConfig.nInputYuvFormat = VIDEO_PIXEL_TILE_128X32;
				break;
			default:
				printf("input pixel format is not supported!\n");
				goto error_release;
				break;
		}

		hdlTina->vport.getSize(&demoRecoder->videoConfig.nSrcWidth,&demoRecoder->videoConfig.nSrcHeight);
		demoRecoder->videoConfig.nOutHeight  = hdlTina->encodeHeight;
		demoRecoder->videoConfig.nOutWidth   = hdlTina->encodeHeight;
		demoRecoder->videoConfig.nBitRate  = hdlTina->encodeBitrate;
		demoRecoder->videoConfig.nFrameRate = hdlTina->encodeFramerate;
		demoRecoder->videoConfig.bUsePhyBuf  = 1;
    }

	if(hdlTina->aport.enable)
	{
        //config AudioEncodeConfig
        memset(&demoRecoder->audioConfig, 0x00, sizeof(AudioEncodeConfig));
		switch(hdlTina->encodeAFormat)
		{
			case TR_AUDIO_PCM:
				demoRecoder->audioConfig.nType		= AUDIO_ENCODE_PCM_TYPE;
				break;
			case TR_AUDIO_AAC:
				demoRecoder->audioConfig.nType		= AUDIO_ENCODE_AAC_TYPE;
				break;
			case TR_AUDIO_MP3:
				demoRecoder->audioConfig.nType		= AUDIO_ENCODE_MP3_TYPE;
				break;
			case TR_AUDIO_LPCM:
				demoRecoder->audioConfig.nType		= AUDIO_ENCODE_LPCM_TYPE;
				break;
			default:
				printf("audio format not supported!\n");
				goto error_release;
				break;
		}

        demoRecoder->audioConfig.nInChan = hdlTina->aport.getAudioChannels();
        demoRecoder->audioConfig.nInSamplerate = hdlTina->aport.getAudioSampleRate();
        demoRecoder->audioConfig.nOutChan = 2;
        demoRecoder->audioConfig.nOutSamplerate = 44100;
        demoRecoder->audioConfig.nSamplerBits = 16;

        if(demoRecoder->muxType == CDX_MUXER_TS && demoRecoder->audioConfig.nType == AUDIO_ENCODE_PCM_TYPE)
        {
            demoRecoder->audioConfig.nFrameStyle = 2;
        }

        if(demoRecoder->muxType == CDX_MUXER_TS && demoRecoder->audioConfig.nType == AUDIO_ENCODE_AAC_TYPE)
        {
            demoRecoder->audioConfig.nFrameStyle = 1;//not add head when encode aac,because use ts muxer
        }

        if(demoRecoder->muxType == CDX_MUXER_AAC)
        {
            demoRecoder->audioConfig.nType = AUDIO_ENCODE_AAC_TYPE;
            demoRecoder->audioConfig.nFrameStyle = 0;//add head when encode aac
        }

        if(demoRecoder->muxType == CDX_MUXER_MP3)
        {
            demoRecoder->audioConfig.nType = AUDIO_ENCODE_MP3_TYPE;
        }
	}

	if(hdlTina->outport.enable)
	{
	    //store the muxer file path
	    strncpy(demoRecoder->mSavePath,hdlTina->outport.url,MAX_URL_LEN);
	    demoRecoder->mAwEncoder = AwEncoderCreate(hdl);
	    if(demoRecoder->mAwEncoder == NULL)
	    {
	        printf("can not create AwRecorder, quit.\n");
	        goto error_release;
	    }

	    //* set callback to recoder,if the encoder has used the buf ,it will callback to app
	    AwEncoderSetNotifyCallback(demoRecoder->mAwEncoder,NotifyCallbackForAwEncorder,hdl);
	    if(hdlTina->aport.enable && (demoRecoder->muxType == CDX_MUXER_AAC || demoRecoder->muxType == CDX_MUXER_MP3))
		{
	        AwEncoderInit(demoRecoder->mAwEncoder, NULL, &demoRecoder->audioConfig,&mEncDataCallBackOps);
	        demoRecoder->videoEos = 1;
	    }
		else if(hdlTina->vport.enable && !hdlTina->aport.enable && (demoRecoder->muxType == CDX_MUXER_MOV || demoRecoder->muxType == CDX_MUXER_TS))
	    {
	        AwEncoderInit(demoRecoder->mAwEncoder, &demoRecoder->videoConfig, NULL,&mEncDataCallBackOps);
	        demoRecoder->audioEos = 1;
	    }
		else if(hdlTina->vport.enable && hdlTina->aport.enable && (demoRecoder->muxType == CDX_MUXER_MOV || demoRecoder->muxType == CDX_MUXER_TS))
	    {
	        AwEncoderInit(demoRecoder->mAwEncoder, &demoRecoder->videoConfig, &demoRecoder->audioConfig,&mEncDataCallBackOps);
	    }
	}

}

int Tinareset(void *hdl)
{

}
