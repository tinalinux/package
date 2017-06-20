/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : recoderdemo.c
 * Description : recoderdemo
 * History :
 *
 */

#define LOG_TAG "recoderdemo"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <errno.h>

#include <cdx_config.h>
#include <cdx_log.h>

#include <CdxQueue.h>
#include <AwPool.h>
#include <CdxBinary.h>
#include <CdxMuxer.h>

#include "memoryAdapter.h"
#include "awencoder.h"

#include "RecoderWriter.h"
#include "csi.h"
#include "disp.h"
#include <signal.h>

#define SAVE_VIDEO_FRAME (0)
#define AUDIO_INPUT (0)
#define VIDEO_INPUT (1)

static const int STATUS_IDEL   = 0;

FILE* inputPCM = NULL;
FILE* inputYUV = NULL;

char pcm_path[128] = {0};
char yuv_path[128] = {0};

int videoEos = 0;
int audioEos = 0;
#define CAPTURE_FRAME_NUM 500

typedef struct DemoRecoderContext
{
    AwEncoder*       mAwEncoder;
    int callbackData;
    pthread_mutex_t mMutex;

    VideoEncodeConfig videoConfig;
    AudioEncodeConfig audioConfig;


    CdxMuxerT* pMuxer;
    int muxType;
    char pUrl[1024];
    CdxWriterT* pStream;
    char*       pOutUrl;

    unsigned char* extractDataBuff;
    unsigned int extractDataLength;

    pthread_t muxerThreadId ;
    pthread_t audioDataThreadId ;
    pthread_t videoDataThreadId ;
    AwPoolT *pool;
    CdxQueueT *dataQueue;
    int exitFlag ;
    unsigned char* pAddrPhyY;
    unsigned char* pAddrPhyC;
    int     bUsed;

    FILE* fpSaveVideoFrame;
}DemoRecoderContext;

int save_frame_to_file(void *str, void *start, int length) {
	FILE *fp = NULL;
	fp = fopen(str, "ab+");
	if(!fp) {
		printf("can not fopen (%s)\n",strerror(errno));
                return -1;
        }
        if(fwrite(start, length, 1, fp)) {
                printf("write finish!\n");
                fclose(fp);
                return 0;
        }
        else {
                printf(" can not fwrite(%s)!\n",strerror(errno));
		fclose(fp);
                return -1;
        }
        return 0;
}


//* a notify callback for AwEncorder.
void NotifyCallbackForAwEncorder(void* pUserData, int msg, void* param)
{
    DemoRecoderContext* pDemoRecoder = (DemoRecoderContext*)pUserData;

    switch(msg)
    {
        case AWENCODER_VIDEO_ENCODER_NOTIFY_RETURN_BUFFER:
        {
            int id = *((int*)param);
            if(id == 0)
            {
                pDemoRecoder->bUsed = 0;
            }
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
    CdxMuxerPacketT *packet = NULL;
    DemoRecoderContext* pDemoRecoder = (DemoRecoderContext*)app;
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

#if SAVE_VIDEO_FRAME
    if(pDemoRecoder->fpSaveVideoFrame)
    {
        fwrite(packet->buf, 1, packet->buflen, pDemoRecoder->fpSaveVideoFrame);
    }
#endif

    CdxQueuePush(pDemoRecoder->dataQueue,packet);
    return 0;
}
int onAudioDataEnc(void *app,CdxMuxerPacketT *buff)
{
    CdxMuxerPacketT *packet = NULL;
    DemoRecoderContext* pDemoRecoder = (DemoRecoderContext*)app;
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

    CdxQueuePush(pDemoRecoder->dataQueue,packet);

    return 0;
}

void* MuxerThread(void *param)
{
    //int ret = 0;
    int i =0;
    CdxMuxerPacketT *mPacket = NULL;
    DemoRecoderContext *p = (DemoRecoderContext*)param;
    RecoderWriterT *rw = NULL;
#if FS_WRITER
    CdxFsCacheMemInfo fs_cache_mem;
    int fs_mode = FSWRITEMODE_DIRECT;
//    int fs_cache_size = 1024 * 1024;
#endif
    logd("MuxerThread");


    if(p->pUrl)
    {
        if ((p->pStream = CdxWriterCreat(p->pUrl)) == NULL)
        {
            loge("CdxWriterCreat() failed");
            return 0;
        }
        rw = (RecoderWriterT*)p->pStream;
        rw->file_mode = FD_FILE_MODE;
        strcpy(rw->file_path, p->pUrl);
        RWOpen(p->pStream);
        p->pMuxer = CdxMuxerCreate(p->muxType, p->pStream);
        if(p->pMuxer == NULL)
        {
            loge("CdxMuxerCreate failed");
            return 0;
        }
        logd("MuxerThread init ok");
    }

    CdxMuxerMediaInfoT mediainfo;
    memset(&mediainfo, 0, sizeof(CdxMuxerMediaInfoT));
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
            loge("unlown audio type(%d)", p->audioConfig.nType);
            break;
    }
#if AUDIO_INPUT
    mediainfo.audioNum = 1;
#endif
#if VIDEO_INPUT
    mediainfo.videoNum = 1;
#endif
    if(p->muxType == CDX_MUXER_AAC)
    {
        mediainfo.videoNum = 0;
    }

    mediainfo.audio.nAvgBitrate = p->audioConfig.nBitrate;
    mediainfo.audio.nBitsPerSample = p->audioConfig.nSamplerBits;
    mediainfo.audio.nChannelNum = p->audioConfig.nOutChan;
    mediainfo.audio.nMaxBitRate = p->audioConfig.nBitrate;
    mediainfo.audio.nSampleRate = p->audioConfig.nOutSamplerate;
    mediainfo.audio.nSampleCntPerFrame = 1024; // aac

    if(p->videoConfig.nType == VIDEO_ENCODE_H264)
        mediainfo.video.eCodeType = VENC_CODEC_H264;
    else if(p->videoConfig.nType == VIDEO_ENCODE_JPEG)
        mediainfo.video.eCodeType = VENC_CODEC_JPEG;
    else
    {
        loge("cannot suppot this video type");
        return 0;
    }

    mediainfo.video.nWidth    = p->videoConfig.nOutWidth;
    mediainfo.video.nHeight   = p->videoConfig.nOutHeight;
    mediainfo.video.nFrameRate = p->videoConfig.nFrameRate;

    logd("******************* mux mediainfo *****************************");
    logd("videoNum                   : %d", mediainfo.videoNum);
    logd("audioNum                   : %d", mediainfo.audioNum);
    logd("videoTYpe                  : %d", mediainfo.video.eCodeType);
    logd("framerate                  : %d", mediainfo.video.nFrameRate);
    logd("width                      : %d", mediainfo.video.nWidth);
    logd("height                     : %d", mediainfo.video.nHeight);
    logd("**************************************************************");

    if(p->pMuxer)
    {
        CdxMuxerSetMediaInfo(p->pMuxer, &mediainfo);
#if FS_WRITER
        memset(&fs_cache_mem, 0, sizeof(CdxFsCacheMemInfo));

        fs_cache_mem.m_cache_size = 512 * 1024;
        fs_cache_mem.mp_cache = (cdx_uint8*)malloc(fs_cache_mem.m_cache_size);
        if (fs_cache_mem.mp_cache == NULL)
        {
            loge("fs_cache_mem.mp_cache malloc failed\n");
            return NULL;
        }
        CdxMuxerControl(p->pMuxer, SET_CACHE_MEM, &fs_cache_mem);
        fs_mode = FSWRITEMODE_CACHETHREAD;

        /*
        fs_cache_size = 1024 * 1024;
        CdxMuxerControl(p->pMuxer, SET_FS_SIMPLE_CACHE_SIZE, &fs_cache_size);
        fs_mode = FSWRITEMODE_SIMPLECACHE;
        */
        //fs_mode = FSWRITEMODE_DIRECT;
        CdxMuxerControl(p->pMuxer, SET_FS_WRITE_MODE, &fs_mode);
#endif
    }

    logd("extractDataLength %d",p->extractDataLength);
    if(p->extractDataLength > 0 && p->pMuxer)
    {
        logd("demo WriteExtraData");
        if(p->pMuxer)
            CdxMuxerWriteExtraData(p->pMuxer, p->extractDataBuff, p->extractDataLength, 0);
    }

    if(p->pMuxer)
    {
        logd("write head");
        CdxMuxerWriteHeader(p->pMuxer);
    }

#if AUDIO_INPUT && VIDEO_INPUT
    while ((audioEos==0) || (videoEos==0))
#elif VIDEO_INPUT
    while(videoEos==0)
#elif AUDIO_INPUT
    while(audioEos==0)
#endif
    {
        while (!CdxQueueEmpty(p->dataQueue))
        {
            mPacket = CdxQueuePop(p->dataQueue);
            i++;
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
        logd("write trailer");
        CdxMuxerWriteTrailer(p->pMuxer);
    }

    logd("CdxMuxerClose");
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

#if FS_WRITER
    if(fs_cache_mem.mp_cache)
    {
        free(fs_cache_mem.mp_cache);
        fs_cache_mem.mp_cache = NULL;
    }
#endif
    logd("MuxerThread has finish!");
    p->exitFlag  = 1;

    return 0;
}


void* AudioInputThread(void *param)
{
    int ret = 0;
    //int i =0;
    DemoRecoderContext *p = (DemoRecoderContext*)param;

    logd("AudioInputThread");
    int num = 0;
    int size2 = 0;
    int64_t audioPts = 0;

    AudioInputBuffer audioInputBuffer;
    memset(&audioInputBuffer, 0x00, sizeof(AudioInputBuffer));
    audioInputBuffer.nLen = 1024*4; //176400;
    audioInputBuffer.pData = (char*)malloc(audioInputBuffer.nLen);

    while(num<1024)
    {
        ret = -1;
        if(!audioEos)
        {
            if (inputPCM == NULL)
            {
                printf("before fread, inputPCM is NULL\n");
                break;
            }
            size2 = fread(audioInputBuffer.pData, 1, audioInputBuffer.nLen, inputPCM);
            if(size2 < audioInputBuffer.nLen)
            {
                logd("read error");
                audioEos = 1;
            }

            while(ret)
            {
                audioInputBuffer.nPts = audioPts;
                ret = AwEncoderWritePCMdata(p->mAwEncoder,&audioInputBuffer);
                //logd("=== WritePCMdata audioPts : %lld", audioPts);

                usleep(10*1000);
            }
            usleep(20*1000);
            audioPts += 23;
        }
        num ++;
    }
    logd("audio read data finish!");
    audioEos = 1;

    return 0;
}

typedef struct
{
	pthread_t csi_thread_id;
	int run_flag;
	DemoRecoderContext *pContext;
}csi_handle;

void *csi_mgr_thread_function(void *pParam)
{
	csi_handle *hdl = (csi_handle *)pParam;
	unsigned long long csiV4l2CurMsec;
	unsigned long long csiV4l2DiffMsec;
	CSI_BUF csiV4l2Buf;
	disp_rect info;
	disp_rectsz rect;
	VideoInputBuffer videoInputBuffer;
	int pts = 0;
	int number = 0;
	int width32_align = 0;
	int height32_align = 0;
	char source_data_path[30];
	int ret = 0;
	struct win_info win;
	unsigned int fbAddr[3];

	memset(&info,0,sizeof(disp_rect));

	csi_stream_on(csi_get_fd());
	while(hdl->run_flag && number < CAPTURE_FRAME_NUM)
	{
		csi_buf_dequeue(&csiV4l2Buf, &csiV4l2CurMsec, &csiV4l2DiffMsec);
		if(csiV4l2DiffMsec == 0)
		{
			printf("get buffer diff 0!\n");
			continue;
		}

		if(csiV4l2Buf.m_v4l2Buf[csiV4l2Buf.m_v4l2BufIdx].m_addr == 0)
		{
			printf("get null address!\n");
			continue;
		}

		info.width = CSI_WIDTH_MAX;//disp_screenwidth_get();
		info.height = CSI_HEIGHT_MAX;//disp_screenheight_get();

		width32_align = (info.width + 31)/32 * 32;
		height32_align = (info.height + 31)/32 *32;

#if 1
		sprintf(source_data_path, "/mnt/UDISK/%s%d%s", "source_data", number+1, ".yuv");
		ret = save_frame_to_file(source_data_path, csiV4l2Buf.m_buf_start, info.width*info.height);
		ret = save_frame_to_file(source_data_path, csiV4l2Buf.m_buf_start + width32_align*height32_align, info.width*info.height/2);
#endif
#if 1
		memset(&win, 0, sizeof(struct win_info));
		fbAddr[0] = (unsigned char *)csiV4l2Buf.m_v4l2Buf[csiV4l2Buf.m_v4l2BufIdx].m_addr;
		fbAddr[1] = (unsigned char *)(fbAddr[0]+ width32_align*height32_align);

		win.scn_win.nDisplayWidth = disp_screenwidth_get();
		win.scn_win.nDisplayHeight = disp_screenheight_get();
		win.scn_win.nLeftOff = 0;
		win.scn_win.nTopOff = 0;
		win.scn_win.nWidth = info.width;
		win.scn_win.nHeight = info.height;

		win.src_win.nWidth = info.width;
		win.src_win.nHeight= info.height;
		win.src_win.nTopOffset = 0;
		win.src_win.nLeftOffset = 0;
		win.src_win.nBottomOffset = 0;
		win.src_win.nRightOffset = 0;
		win.nScreenHeight = 480;
		win.nScreenWidth = 800;
		ret = disp_video_layer_param_set(fbAddr, VIDEO_PIXEL_FORMAT_NV21, &win);
#endif
		videoInputBuffer.pAddrPhyY = (unsigned char *)csiV4l2Buf.m_v4l2Buf[csiV4l2Buf.m_v4l2BufIdx].m_addr;
		videoInputBuffer.pAddrPhyC = (unsigned char *)(videoInputBuffer.pAddrPhyY + width32_align*height32_align);
		videoInputBuffer.nID = 0;
		videoInputBuffer.nLen = width32_align*height32_align*3/2;
		printf("buf len:%d\n",videoInputBuffer.nLen);
		videoInputBuffer.nPts = pts;
		pts += 30;
		hdl->pContext->bUsed = 1;

		AwEncoderWriteYUVdata(hdl->pContext->mAwEncoder,&videoInputBuffer);
		while(hdl->pContext->bUsed == 1)
		{
			usleep(1000*2);
		}

		csi_buf_enqueue(&csiV4l2Buf);
		number++;
	}
	videoEos = 1;
	csi_stream_off(csi_get_fd());
	csi_uninit();
	return NULL;
}

unsigned int csi_thread_init(csi_handle *hdl)
{
	int ret = 0;


	printf("Entering %s init ...\n", __func__);

	ret = csi_init();
	if(ret == 0){
		printf("csi init ok!\n");
	}else if(ret == -1){
		printf("csi not exit!\n");
		ret = -1;
		goto errHdl;
	}else{
		printf("csi init fail!\n");
		ret = -2;
		goto errHdl;
	}
	ret = pthread_create(&hdl->csi_thread_id, NULL, csi_mgr_thread_function, hdl);
	if(ret != 0) {
		printf("create csi mgr pthread failed!\n");
		ret = -5;
		goto errHdl;
	}

errHdl:
	return ret;
}
#if 1
void disp_mgr_init()
{
	int fdDisp;
	disp_init();
	fdDisp = disp_get_cur_fd();
    if(fdDisp < 1)
    {
        printf("open disp failed");
        return;
    }
	disp_lcd_on(fdDisp);
	disp_open_backlight();
}

void release_de(int signb)
{
	disp_uninit();
	exit(1);
}
#endif
//* the main method.
int main(int argc, char *argv[])
{
    DemoRecoderContext demoRecoder;

    EncDataCallBackOps mEncDataCallBackOps;
    CdxMuxerPacketT *mPacket = NULL;
	csi_handle *hdl = (csi_handle *)malloc(sizeof(csi_handle));
	void *tret;
	hdl->run_flag = 1;
#if 1
	disp_mgr_init();
	disp_video_chanel_open();
#endif

    mEncDataCallBackOps.onAudioDataEnc = onAudioDataEnc;
    mEncDataCallBackOps.onVideoDataEnc = onVideoDataEnc;

    printf("\n");
    printf("************************************************************************\n");
    printf("* This program implements a simple recoder.\n");
    printf("* Inplemented by Allwinner ALD-AL3 department.\n");
    printf("***********************************************************************\n");
#if 1
	signal(SIGINT, release_de);
#endif

    //* create a demoRecoder.
    memset(&demoRecoder, 0, sizeof(DemoRecoderContext));

    demoRecoder.pool = AwPoolCreate(NULL);
    demoRecoder.dataQueue = CdxQueueCreate(demoRecoder.pool);
#if SAVE_VIDEO_FRAME
    demoRecoder.fpSaveVideoFrame = fopen("/mnt/UDISK/video.dat", "wb");
    if(demoRecoder.fpSaveVideoFrame == NULL)
    {
        printf("open file /mnt/UDISK/video.dat failed, errno(%d)\n", errno);
    }
#endif
    //VideoEncodeConfig videoConfig;
    memset(&demoRecoder.videoConfig, 0x00, sizeof(VideoEncodeConfig));
    demoRecoder.videoConfig.nType       = VIDEO_ENCODE_H264;
    demoRecoder.videoConfig.nFrameRate  = 30;
    demoRecoder.videoConfig.nOutHeight  = 480;//1080;
    demoRecoder.videoConfig.nOutWidth   = 640;//1920;
    demoRecoder.videoConfig.nSrcHeight  = 480;//1080;
    demoRecoder.videoConfig.nSrcWidth   = 640;//1920;
    demoRecoder.videoConfig.nBitRate    = 3*1000*1000;
    demoRecoder.videoConfig.bUsePhyBuf  = 1;
    demoRecoder.videoConfig.nInputYuvFormat = VIDEO_PIXEL_YUV420_NV21;

    //AudioEncodeConfig audioConfig;
    demoRecoder.audioConfig.nType = AUDIO_ENCODE_AAC_TYPE;
    demoRecoder.audioConfig.nInChan = 2;
    demoRecoder.audioConfig.nInSamplerate = 44100;
    demoRecoder.audioConfig.nOutChan = 2;
    demoRecoder.audioConfig.nOutSamplerate = 44100;
    demoRecoder.audioConfig.nSamplerBits = 16;


    demoRecoder.muxType = CDX_MUXER_MOV;

    if(demoRecoder.muxType == CDX_MUXER_TS &&
            demoRecoder.audioConfig.nType == AUDIO_ENCODE_PCM_TYPE)
    {
        demoRecoder.audioConfig.nFrameStyle = 2;
    }

    if(demoRecoder.muxType == CDX_MUXER_TS &&
            demoRecoder.audioConfig.nType == AUDIO_ENCODE_AAC_TYPE)
    {
        demoRecoder.audioConfig.nFrameStyle = 1;
    }

    if(demoRecoder.muxType == CDX_MUXER_AAC)
    {
        demoRecoder.audioConfig.nType = AUDIO_ENCODE_AAC_TYPE;
        demoRecoder.audioConfig.nFrameStyle = 0;
    }

    if(demoRecoder.muxType == CDX_MUXER_MP3)
    {
        demoRecoder.audioConfig.nType = AUDIO_ENCODE_MP3_TYPE;
    }

    pthread_mutex_init(&demoRecoder.mMutex, NULL);

    demoRecoder.mAwEncoder = AwEncoderCreate(&demoRecoder);
    if(demoRecoder.mAwEncoder == NULL)
    {
        printf("can not create AwRecorder, quit.\n");
        exit(-1);
    }

    //* set callback to recoder.
    AwEncoderSetNotifyCallback(demoRecoder.mAwEncoder,NotifyCallbackForAwEncorder,&(demoRecoder));

    if(demoRecoder.muxType == CDX_MUXER_AAC || demoRecoder.muxType == CDX_MUXER_MP3)
    {
        AwEncoderInit(demoRecoder.mAwEncoder, NULL, &demoRecoder.audioConfig,&mEncDataCallBackOps);
        videoEos = 1;
    }
    else
    {
        AwEncoderInit(demoRecoder.mAwEncoder, &demoRecoder.videoConfig,
                    &demoRecoder.audioConfig,&mEncDataCallBackOps);
    }

    switch(demoRecoder.muxType)
    {
        case CDX_MUXER_AAC:
            sprintf(demoRecoder.pUrl,  "/mnt/UDISK/save.aac");
            break;
        case CDX_MUXER_MP3:
            sprintf(demoRecoder.pUrl,  "/mnt/UDISK/save.mp3");
            break;
        case CDX_MUXER_TS:
            sprintf(demoRecoder.pUrl,  "/mnt/UDISK/save.ts");
            break;
        default:
            sprintf(demoRecoder.pUrl,  "/mnt/UDISK/save.mp4");
            break;
     }

    AwEncoderStart(demoRecoder.mAwEncoder);

    AwEncoderGetExtradata(demoRecoder.mAwEncoder,&demoRecoder.extractDataBuff,
                    &demoRecoder.extractDataLength);
#if SAVE_VIDEO_FRAME
    if(demoRecoder.fpSaveVideoFrame)
    {
        fwrite(demoRecoder.extractDataBuff, 1, demoRecoder.extractDataLength,
                    demoRecoder.fpSaveVideoFrame);
    }
#endif

#if AUDIO_INPUT
    pthread_create(&demoRecoder.audioDataThreadId, NULL, AudioInputThread, &demoRecoder);
#endif

#if VIDEO_INPUT
	hdl->pContext = &demoRecoder;
	csi_thread_init(hdl);
#endif


    pthread_create(&demoRecoder.muxerThreadId, NULL, MuxerThread, &demoRecoder);


    while (demoRecoder.exitFlag == 0)
    {
        logd("wait MuxerThread finish!");
        usleep(1000*1000);
    }

    if(demoRecoder.muxerThreadId)
        pthread_join(demoRecoder.muxerThreadId,     NULL);
#if AUDIO_INPUT
    if(demoRecoder.audioDataThreadId)
        pthread_join(demoRecoder.audioDataThreadId, NULL);
#endif

    printf("destroy AwRecorder.\n");
    while (!CdxQueueEmpty(demoRecoder.dataQueue))
    {
        logd("free a packet");
        mPacket = CdxQueuePop(demoRecoder.dataQueue);
        free(mPacket->buf);
        free(mPacket);
    }
    CdxQueueDestroy(demoRecoder.dataQueue);
    AwPoolDestroy(demoRecoder.pool);

    if(demoRecoder.mAwEncoder != NULL)
    {
        AwEncoderStop(demoRecoder.mAwEncoder);
        AwEncoderDestory(demoRecoder.mAwEncoder);
        demoRecoder.mAwEncoder = NULL;
    }

    pthread_mutex_destroy(&demoRecoder.mMutex);
#if SAVE_VIDEO_FRAME
    if(demoRecoder.fpSaveVideoFrame)
        fclose(demoRecoder.fpSaveVideoFrame);
#endif

#if AUDIO_INPUT
    if (inputPCM)
    {
         fclose(inputPCM);
         inputPCM = NULL;
    }
#endif
	hdl->run_flag = 0;
	pthread_join(hdl->csi_thread_id,&tret);
	free(hdl);
#if 1
    disp_video_chanel_close();
    disp_uninit();
#endif
    printf("\n");
    printf("**************************************************************\n");
    printf("* Quit the program, goodbye!\n");
    printf("**************************************************************\n");
    printf("\n");

    return 0;
}
