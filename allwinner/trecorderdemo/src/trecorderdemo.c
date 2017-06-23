
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
    AwPoolT *mAudioDataPool;
    CdxQueueT *mAudioDataQueue;
    AwPoolT *mVideoDataPool;
    CdxQueueT *mVideoDataQueue;
    unsigned char* pAddrVirY;
    unsigned char* pAddrVirC;
    int     bUsed;
    int  mRecordDuration;
    FILE*  fpSaveVideoFrame;
    FILE*  fpSaveAudioFrame;

    FILE* inputPCM;
    FILE* inputYUV;
    int videoEos;
    int audioEos;
    bool muxThreadExitFlag;
    int recorderIndex;
}DemoRecoderContext;

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
                //printf("---- pDemoRecoder->bUsed: %d , %p, pDemoRecoder: %p\n", pDemoRecoder->bUsed, &pDemoRecoder->bUsed, pDemoRecoder);
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

    CdxQueuePush(pDemoRecoder->mVideoDataQueue,packet);
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
    packet->duration = buff->duration;
    CdxQueuePush(pDemoRecoder->mAudioDataQueue,packet);

    return 0;
}

void* MuxerThread(void *param)
{
    //int ret = 0;
    DemoRecoderContext *p = (DemoRecoderContext*)param;
    int audioFrameCount;
    int videoFrameCount;
    int oneMinuteAudioFrameCount;
    int oneMinuteVideoFrameCount;
    int file_count = 0;
    bool reopen_flag;
    CdxMuxerPacketT *mPacket;
    RecoderWriterT *rw;
    char path[256];
    char recorder_count_str[10];
    char file_count_str[15];
    char rm_cmd[128];
    CdxMuxerMediaInfoT mediainfo;
    int fs_cache_size = 0;
    printf("MuxerThread begin\n");
#if FS_WRITER
    //CdxFsCacheMemInfo fs_cache_mem;
#endif
REOPEN:
    printf("MuxerThread file_count = %d\n",file_count);
    audioFrameCount = 0;
    videoFrameCount = 0;
    reopen_flag = false;
    mPacket = NULL;
    rw = NULL;
    if(file_count >= 10){
	printf("only save 10 file,so set the file_count=0 to rewrite\n");
	file_count = 0;
    }
    if(p->mSavePath)
    {
        if ((p->pStream = CdxWriterCreat(p->mSavePath)) == NULL)
        {
            printf("CdxWriterCreat() failed\n");
            return 0;
        }
        rw = (RecoderWriterT*)p->pStream;
        rw->file_mode = FD_FILE_MODE;

        strcpy(path,p->mSavePath);
        strcat(path,"recorder");
        sprintf(recorder_count_str,"%d",p->recorderIndex);
        strcat(path,recorder_count_str);
        strcat(path,"_save");
        sprintf(file_count_str,"%d",file_count);
        strcat(path,file_count_str);
        switch (p->muxType)
        {
            case CDX_MUXER_MOV:
		strcat(path,".mp4");
		break;
            case CDX_MUXER_TS:
		strcat(path,".ts");
		break;
            case CDX_MUXER_AAC:
		strcat(path,".aac");
		break;
            case CDX_MUXER_MP3:
		strcat(path,".mp3");
		break;
            default:
		printf("error:the muxer type is not support\n");
		break;
        }
        strcpy(rw->file_path, path);
        if(access(rw->file_path,F_OK) == 0){
            printf("rm the exit file: %s \n",rw->file_path);
            strcpy(rm_cmd,"rm ");
            strcat(rm_cmd,rw->file_path);
            printf("rm_cmd is %s\n",rm_cmd);
            system(rm_cmd);
        }
        RWOpen(p->pStream);
	p->pMuxer = CdxMuxerCreate(p->muxType, p->pStream);
	if(p->pMuxer == NULL)
	{
            printf("CdxMuxerCreate failed\n");
            return 0;
	}
	printf("MuxerThread init ok\n");
    }

    memset(&mediainfo, 0, sizeof(CdxMuxerMediaInfoT));
    if(p->inputPCM != NULL){
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
	oneMinuteAudioFrameCount = 60*p->audioConfig.nInSamplerate/mediainfo.audio.nSampleCntPerFrame;
	printf("oneMinuteAudioFrameCount = %d\n",oneMinuteAudioFrameCount);
    }

    if((p->inputYUV!=NULL)&&(p->muxType == CDX_MUXER_MOV || p->muxType == CDX_MUXER_TS)){
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
		/*
        memset(&fs_cache_mem, 0, sizeof(CdxFsCacheMemInfo));

        fs_cache_mem.m_cache_size = 512 * 1024;
        fs_cache_mem.mp_cache = (cdx_int8*)malloc(fs_cache_mem.m_cache_size);
        if (fs_cache_mem.mp_cache == NULL)
        {
            printf("fs_cache_mem.mp_cache malloc failed\n");
            return NULL;
        }
        CdxMuxerControl(p->pMuxer, SET_CACHE_MEM, &fs_cache_mem);
        int fs_mode = FSWRITEMODE_CACHETHREAD;
	  */

        fs_cache_size = 1024 * 1024;
        CdxMuxerControl(p->pMuxer, SET_FS_SIMPLE_CACHE_SIZE, &fs_cache_size);
        int fs_mode = FSWRITEMODE_SIMPLECACHE;

        //int fs_mode = FSWRITEMODE_DIRECT;
        CdxMuxerControl(p->pMuxer, SET_FS_WRITE_MODE, &fs_mode);
#endif
    }

    printf("extractDataLength %u\n",p->extractDataLength);
    if(p->extractDataLength > 0 && p->pMuxer)
    {
        printf("demo WriteExtraData\n");
        if(p->pMuxer){
            CdxMuxerWriteExtraData(p->pMuxer, p->extractDataBuff, p->extractDataLength, 0);
	    printf("demo WriteExtraData finish\n");
	}
    }

    if(p->pMuxer)
    {
        printf("write head\n");
	CdxMuxerWriteHeader(p->pMuxer);
    }
    while (((p->audioEos==0) || (p->videoEos==0) || (!CdxQueueEmpty(p->mAudioDataQueue)) || (!CdxQueueEmpty(p->mVideoDataQueue))) && (!reopen_flag))
    {
	if((p->audioEos==1) && (p->videoEos==1)){
		printf("audio and video input stream has finish,but we has not mux all packet\n");
	}
        while (((!CdxQueueEmpty(p->mAudioDataQueue)) || (!CdxQueueEmpty(p->mVideoDataQueue))) && (!reopen_flag))
        {
            if((p->inputPCM != NULL) && (p->inputYUV != NULL)){//has audio and video
                if((audioFrameCount < oneMinuteAudioFrameCount) && ((3*videoFrameCount) > (2*audioFrameCount))
                    && (!CdxQueueEmpty(p->mAudioDataQueue))){
                    mPacket = CdxQueuePop(p->mAudioDataQueue);
                    if(mPacket == NULL){
			continue;
                    }
                    audioFrameCount++;
                    if((audioFrameCount %100) == 0){
			printf("111audioFrameCount = %d\n",audioFrameCount);
                    }
                }else if((videoFrameCount < oneMinuteVideoFrameCount) && ((3*videoFrameCount) <= (2*audioFrameCount))
                    && (!CdxQueueEmpty(p->mVideoDataQueue))){
                    mPacket = CdxQueuePop(p->mVideoDataQueue);
                    if(mPacket == NULL){
			continue;
                    }
                    videoFrameCount++;
                    if((videoFrameCount %100) == 0){
			printf("333videoFrameCount = %d\n",videoFrameCount);
                    }
                }else if((audioFrameCount < oneMinuteAudioFrameCount) && (!CdxQueueEmpty(p->mAudioDataQueue))){
                    mPacket = CdxQueuePop(p->mAudioDataQueue);
                    if(mPacket == NULL){
			continue;
                    }
                    audioFrameCount++;
                    if((audioFrameCount %100) == 0){
			printf("222audioFrameCount = %d\n",audioFrameCount);
                    }
                }else if((videoFrameCount < oneMinuteVideoFrameCount) && (!CdxQueueEmpty(p->mVideoDataQueue))){
                    mPacket = CdxQueuePop(p->mVideoDataQueue);
                    if(mPacket == NULL){
			continue;
                    }
                    videoFrameCount++;
                    if((videoFrameCount %100) == 0){
			printf("444videoFrameCount = %d\n",videoFrameCount);
                    }
                }else{
                //maybe the audioFrameCount(videoFrameCount )is enough,but the mVideoDataQueue(mAudioDataQueue) is empty,
                //so we should usleep 10 ms and continue to pop the packet
			//printf("it is not good to run here\n");
                    usleep(10*1000);
                    continue;
                }
                if((audioFrameCount >= oneMinuteAudioFrameCount) && (videoFrameCount >= oneMinuteVideoFrameCount)){
                    reopen_flag = true;
                }
            }else if((p->inputPCM != NULL) && (p->inputYUV == NULL)){//only has audio
		mPacket = CdxQueuePop(p->mAudioDataQueue);
		if(mPacket == NULL){
		    continue;
		}
		audioFrameCount++;
		if(audioFrameCount >= oneMinuteAudioFrameCount){
		    reopen_flag = true;
		}
            }else if((p->inputPCM == NULL) && (p->inputYUV != NULL)){//only has video
                mPacket = CdxQueuePop(p->mVideoDataQueue);
                if(mPacket == NULL){
                    continue;
                }
                videoFrameCount++;
                if(videoFrameCount >= oneMinuteVideoFrameCount){
                    reopen_flag = true;
                }
            }

            if(p->pMuxer)
            {
                if(CdxMuxerWritePacket(p->pMuxer, mPacket) < 0)
                {
                    printf("+++++++ CdxMuxerWritePacket failed\n");
                }
            }
            free(mPacket->buf);
            free(mPacket);
            mPacket = NULL;
        }
        usleep(1000);

    }

    if(p->pMuxer)
    {
        printf("write trailer\n");
        CdxMuxerWriteTrailer(p->pMuxer);
    }

    printf("CdxMuxerClose\n");
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

    if(reopen_flag){
        printf("need to reopen another file to save the mp4\n");
        file_count++;
        goto REOPEN;
    }

#if FS_WRITER
	/*
    if(fs_cache_mem.mp_cache)
    {
        free(fs_cache_mem.mp_cache);
        fs_cache_mem.mp_cache = NULL;
    }*/
#endif
    printf("MuxerThread has finish!\n");
    p->muxThreadExitFlag = true;
    return 0;
}


void* AudioInputThread(void *param)
{
    int ret = 0;
    //int i =0;
    DemoRecoderContext *p = (DemoRecoderContext*)param;

    printf("AudioInputThread\n");
    int num = 0;
    int size2 = 0;
    int64_t audioPts = 0;

    AudioInputBuffer audioInputBuffer;
    memset(&audioInputBuffer, 0x00, sizeof(AudioInputBuffer));
    audioInputBuffer.nLen = 1024*4; //176400;
    audioInputBuffer.pData = (char*)malloc(audioInputBuffer.nLen);
    int one_audio_frame_time = audioInputBuffer.nLen*1000/(p->audioConfig.nInSamplerate*p->audioConfig.nInChan*p->audioConfig.nSamplerBits/8);
    int need_recorder_num = p->mRecordDuration*1000/one_audio_frame_time;
    while(num < need_recorder_num)
    {
	ret = -1;
	if(!p->audioEos)
	{
	    if (p->inputPCM == NULL)
	    {
	        printf("before fread, inputPCM is NULL\n");
                break;
	    }
            size2 = fread(audioInputBuffer.pData, 1, audioInputBuffer.nLen, p->inputPCM);
            if(size2 < audioInputBuffer.nLen)
            {
		printf("audio has read to end,read from the begining again\n");
		fseek(p->inputPCM,0,SEEK_SET);
		continue;
		//printf("read error\n");
		//p->audioEos = 1;
            }
            while(ret < 0)
            {
		audioInputBuffer.nPts = audioPts;
		ret = AwEncoderWritePCMdata(p->mAwEncoder,&audioInputBuffer);
		//printf("=== WritePCMdata audioPts : %lld\n", audioPts);
		//printf("after write pcm ,sleep 10ms ,because the encoder maybe has no enough buf,ret = %d\n",ret);
		if(ret<0){
			printf("write pcm fail\n");
		}
		usleep(10*1000);
            }
            usleep(20*1000);
            audioPts += 23;
	}
	num ++;
    }
    printf("audio read data finish!\n");
    p->audioEos = 1;
    while(!p->muxThreadExitFlag){
	usleep(100*1000);
    }
    if (p->inputPCM)
    {
        fclose(p->inputPCM);
        p->inputPCM = NULL;
    }
    printf("exit audio input thread\n");
    return 0;
}

void* VideoInputThread(void *param)
{
    DemoRecoderContext *p = (DemoRecoderContext*)param;
    printf("VideoInputThread\n");
    struct ScMemOpsS* memops = NULL;

    VideoInputBuffer videoInputBuffer;
    int sizeY = p->videoConfig.nSrcHeight* p->videoConfig.nSrcWidth;

    if(p->videoConfig.bUsePhyBuf)
    {
        memops = MemAdapterGetOpsS();
        if(memops == NULL)
        {
            printf("memops is NULL\n");
            return NULL;
        }
        CdcMemOpen(memops);
	p->pAddrVirY = CdcMemPalloc(memops, sizeY);
	p->pAddrVirC = CdcMemPalloc(memops, sizeY/2);
	printf("==== palloc demoRecoder.pAddrPhyY: %p\n", p->pAddrVirY);
	printf("sizeY = %d\n",sizeY);
	printf("==== palloc demoRecoder.pAddrPhyC: %p\n", p->pAddrVirC);
    }
    else
    {
        memset(&videoInputBuffer, 0x00, sizeof(VideoInputBuffer));
        videoInputBuffer.nLen = p->videoConfig.nSrcHeight* p->videoConfig.nSrcWidth *3/2;
        videoInputBuffer.pData = (unsigned char*)malloc(videoInputBuffer.nLen);
    }
    long long videoPts = 0;
    int ret = -1;
    int num = 0;
    int llh_sizey = 0;
    int llh_sizec = 0;
    while(num < ((p->mRecordDuration)*(p->videoConfig.nFrameRate)))
    {
	ret = -1;

	if(p->videoConfig.bUsePhyBuf)
	{
            while(1)
            {
		if(p->bUsed == 0)//if bUsed==0,means this buf has been returned by video decoder
		{
			break;
		}
		//printf("==== wait buf return, demoRecoder.bUsed: %d \n", demoRecoder.bUsed);
		usleep(10*1000);
            }
            videoInputBuffer.nID = 0;
            llh_sizey = fread(p->pAddrVirY, 1, sizeY,  p->inputYUV);
            llh_sizec = fread(p->pAddrVirC, 1, sizeY/2, p->inputYUV);
            //printf("llh_sizey = %d,llh_sizec = %d\n",llh_sizey,llh_sizec);
            if((llh_sizey<sizeY) || (llh_sizec<(sizeY/2))){
		//printf("phy:video has read to end,read from the begining again\n");
		fseek(p->inputYUV,0,SEEK_SET);
		continue;
            }
            CdcMemFlushCache(memops, p->pAddrVirY, sizeY);
            CdcMemFlushCache(memops, p->pAddrVirC, sizeY/2);

            videoInputBuffer.pAddrPhyY = CdcMemGetPhysicAddressCpu(memops, p->pAddrVirY);
            videoInputBuffer.pAddrPhyC = CdcMemGetPhysicAddressCpu(memops, p->pAddrVirC);
            //printf("==== palloc videoInputBuffer.pAddrPhyY: %p\n", videoInputBuffer.pAddrPhyY);
            //printf("==== palloc videoInputBuffer.pAddrPhyC: %p\n", videoInputBuffer.pAddrPhyC);

	}
	else
	{
            int size1;
            size1 = fread(videoInputBuffer.pData, 1, videoInputBuffer.nLen, p->inputYUV);
            if(size1 < videoInputBuffer.nLen)
            {
		//printf("video has read to end,read from the begining again\n");
		fseek(p->inputYUV,0,SEEK_SET);
		continue;
		//printf("read error\n");
		//p->videoEos = 1;
            }
	}

	while(ret < 0)
	{
	    videoInputBuffer.nPts = videoPts;
	    p->bUsed = 1;
	    //printf("==== writeYUV used: %d", demoRecoder.bUsed);
            ret = AwEncoderWriteYUVdata(p->mAwEncoder,&videoInputBuffer);
            if(ret<0){
		printf("write yuv data fail\n");
            }
            usleep(10*1000);
	}
	usleep(27*1000);
	videoPts += 33;
	num ++;
	if((num % 500) == 0){
	    printf("num = %d\n",num);
	}
    }

    if(p->videoConfig.bUsePhyBuf){
	while(1)
	{
            if(p->bUsed == 0)//if bUsed==0,means this buf has not been returned by video decoder
            {
		break;
            }
            //printf("==== wait buf return, demoRecoder.bUsed: %d \n", demoRecoder.bUsed);
            usleep(10000);
	}
	printf("==== freee demoRecoder.pAddrVirY: %p\n", p->pAddrVirY);
	if(p->pAddrVirY)
	{
	    CdcMemPfree(memops, p->pAddrVirY);
	}
	printf("==== freee demoRecoder.pAddrVirY  end\n");

        if(p->pAddrVirC)
	{
	    CdcMemPfree(memops, p->pAddrVirC);
	}
        if (memops)
        {
	    CdcMemClose(memops);
	    memops = NULL;
        }
    }

    printf("video read data finish!\n");
    p->videoEos = 1;
    while(!p->muxThreadExitFlag){
	usleep(100*1000);
    }
    if (p->inputYUV)
    {
        fclose(p->inputYUV);
        p->inputYUV = NULL;
    }
    printf("exit video input thread\n");
    return 0;
}

static void showHelp(void)
{
    printf("******************************************************************************************\n");
    printf("the command usage is like this: \n");
    printf("./trecorderdemo argv[1] argv[2] argv[3] argv[4] argv[5] argv[6] argv[7] argv[8]\n");
    printf(" argv[1]: the num of recorder,now support:1,2\n");
    printf(" argv[2]: the first video yuv data absolute path and file, if not input yuv data,this arg set null,for example:/mnt/UDISK/1080p.yuv \n");
    printf(" argv[3]: the first audio pcm data absolute path and file, if not input pcm data,this arg set null,for example:/mnt/UDISK/music.pcm \n");
    printf(" argv[4]: the first output absolute path ,for example:/mnt/UDISK/ \n");
    printf(" argv[5]: the first video encoded format ,support: jpeg/h264 ,use null arg to declare no video\n");
    printf(" argv[6]: the first audio encoded format ,support: pcm/aac/mp3 ,use null arg to declare no audio \n");
    printf(" argv[7]: the first muxer format ,support: mp4/ts/aac/mp3 \n");
    printf(" argv[8]: the first encode resolution ,support: 480P/720P/1080P , use null arg to declare no video\n");
    printf(" argv[9]: the first recorder duration,the unit is second \n");
    printf(" argv[10]: the format of the first input yuv:support:yu12/yv12/nv21/nv12 \n");
    printf(" notice: if the argv[1] is equal 2 ,we should also input the second stream param \n");
    printf(" argv[11]: the second video yuv data absolute path and file, if not input yuv data,this arg set null,for example:/mnt/UDISK/720.yuv \n");
    printf(" argv[12]: the second audio pcm data absolute path and file, if not input pcm data,this arg set null,for example:/mnt/UDISK/audio.pcm \n");
    printf(" argv[13]: the second output absolute path ,for example:/mnt/SDCARD/ \n");
    printf(" argv[14]: the second video encoded format ,support: jpeg/h264 ,use null arg to declare no video\n");
    printf(" argv[15]: the second audio encoded format ,support: pcm/aac/mp3 ,use null arg to declare no audio \n");
    printf(" argv[16]: the second muxer format ,support: mp4/ts/aac/mp3 \n");
    printf(" argv[17]: the second encode resolution ,support: 480P/720P/1080P , use null arg to declare no video\n");
    printf(" argv[18]: the second recorder duration,the unit is second \n");
    printf(" argv[19]: the format of the second input yuv:support:yu12/yv12/nv21/nv12 \n");
    printf(" notice: if the argv[1] is equal 3 ,we should also input the third stream param \n");
    printf("******************************************************************************************\n");
}

//* the main method.
int main(int argc, char *argv[])
{

    if((argc == 1)||((argc == 2)&&(strcmp(argv[1],"--help")==0 || strcmp(argv[1],"-h")==0))){
	showHelp();
	return -1;
    }
    int recorderNum = 0;
    if(atoi(argv[1]) == 1 || atoi(argv[1]) == 2){
        recorderNum = atoi(argv[1]);
    }else{
        printf("err:the input recorder num is wrong,input recorder num = %d\n",atoi(argv[1]));
        return -1;
    }
    if(argc < (2+recorderNum*9))
    {
	printf("run failed , argc = %d,the argc is less than %d \n",argc,2+recorderNum*9);
	showHelp();
	return -1;
    }
    EncDataCallBackOps mEncDataCallBackOps;
    mEncDataCallBackOps.onAudioDataEnc = onAudioDataEnc;
    mEncDataCallBackOps.onVideoDataEnc = onVideoDataEnc;
    //* create the demoRecoder.
    DemoRecoderContext demoRecoder[recorderNum];
    for(int i = 0 ; i < recorderNum; i++){
        printf("it is the %d recorder stream\n",i);
        memset(&demoRecoder[i], 0, sizeof(DemoRecoderContext));
        demoRecoder[i].recorderIndex = i;
        //open the input yuv data
        printf("input yuv path = %s\n",argv[2+i*9]);
        demoRecoder[i].inputYUV = fopen(argv[2+i*9], "rb");
        printf("fopen inputYUV == %p\n", demoRecoder[i].inputYUV);
        if(demoRecoder[i].inputYUV == NULL)
        {
	    printf("open yuv file failed, errno(%d),strerror = %s\n", errno,strerror(errno));
        }

        //open the input pcm data
        printf("input pcm path = %s\n",argv[3+i*9]);
        demoRecoder[i].inputPCM = fopen(argv[3+i*9], "rb");
        printf("fopen inputPCM == %p\n", demoRecoder[i].inputPCM);
        if(demoRecoder[i].inputPCM == NULL)
        {
	    printf("open pcm file failed, errno(%d),strerror = %s\n", errno,strerror(errno));
        }

        //create the queue to store the encoded video data and encoded audio data
        demoRecoder[i].mAudioDataPool = AwPoolCreate(NULL);
        demoRecoder[i].mAudioDataQueue = CdxQueueCreate(demoRecoder[i].mAudioDataPool);
        demoRecoder[i].mVideoDataPool = AwPoolCreate(NULL);
        demoRecoder[i].mVideoDataQueue = CdxQueueCreate(demoRecoder[i].mVideoDataPool);

        //the total record duration
        demoRecoder[i].mRecordDuration = atoi(argv[9+i*9]);
        printf("record duration is %ds\n",demoRecoder[i].mRecordDuration);

        //muxer type
        if(strcmp(argv[7+i*9],"mp4")==0){
            demoRecoder[i].muxType       = CDX_MUXER_MOV;
        }else if(strcmp(argv[7+i*9],"ts")==0){
            demoRecoder[i].muxType       = CDX_MUXER_TS;
        }else if(strcmp(argv[7+i*9],"aac")==0){
            demoRecoder[i].muxType       = CDX_MUXER_AAC;
        }else if(strcmp(argv[7+i*9],"mp3")==0){
            demoRecoder[i].muxType       = CDX_MUXER_MP3;
        }else{
            printf("the input muxer type is not support,use the default:mp4\n");
            demoRecoder[i].muxType       = CDX_MUXER_MOV;
        }
        printf(" muxer type = %d\n",demoRecoder[i].muxType);

        //config VideoEncodeConfig
        memset(&demoRecoder[i].videoConfig, 0x00, sizeof(VideoEncodeConfig));
        if((demoRecoder[i].inputYUV != NULL) && (demoRecoder[i].muxType == CDX_MUXER_MOV || demoRecoder[i].muxType == CDX_MUXER_TS)){
            if(strcmp(argv[5+i*9],"jpeg")==0){
                demoRecoder[i].videoConfig.nType       = VIDEO_ENCODE_JPEG;
            }else if(strcmp(argv[5+i*9],"h264")==0){
                demoRecoder[i].videoConfig.nType       = VIDEO_ENCODE_H264;
            }else{
                    printf("the input video encode type is not support,use the default:h264\n");
                    demoRecoder[i].videoConfig.nType       = VIDEO_ENCODE_H264;
            }
            printf("video encode type = %d\n",demoRecoder[i].videoConfig.nType);

            if((strcmp(argv[8+i*9],"480P")==0) || (strcmp(argv[8+i*9],"480p")==0)){
                    demoRecoder[i].videoConfig.nOutHeight  = 480;
                    demoRecoder[i].videoConfig.nOutWidth   = 640;
                    demoRecoder[i].videoConfig.nSrcHeight  = 480;
                    demoRecoder[i].videoConfig.nSrcWidth   = 640;
                    demoRecoder[i].videoConfig.nBitRate    = 3*1000*1000;
            }else if((strcmp(argv[8+i*9],"720P")==0) || (strcmp(argv[8+i*9],"720p")==0)){
                    demoRecoder[i].videoConfig.nOutHeight  = 720;
                    demoRecoder[i].videoConfig.nOutWidth   = 1280;
                    demoRecoder[i].videoConfig.nSrcHeight  = 720;
                    demoRecoder[i].videoConfig.nSrcWidth   = 1280;
                    demoRecoder[i].videoConfig.nBitRate    = 6*1000*1000;
            }else if((strcmp(argv[8+i*9],"1080P")==0) || (strcmp(argv[8+i*9],"1080p")==0)){
                    demoRecoder[i].videoConfig.nOutHeight  = 1080;
                    demoRecoder[i].videoConfig.nOutWidth   = 1920;
                    demoRecoder[i].videoConfig.nSrcHeight  = 1080;
                    demoRecoder[i].videoConfig.nSrcWidth   = 1920;
                    demoRecoder[i].videoConfig.nBitRate    = 12*1000*1000;
            }else{
                    printf("err:the input resolution is not support");
                    return -1;
            }

            if((strcmp(argv[10+i*9],"yu12")==0)){
                demoRecoder[i].videoConfig.nInputYuvFormat = VIDEO_PIXEL_YUV420_YU12;
            }else if((strcmp(argv[10+i*9],"yv12")==0)){
                demoRecoder[i].videoConfig.nInputYuvFormat = VIDEO_PIXEL_YVU420_YV12;
            }else if((strcmp(argv[10+i*9],"nv21")==0)){
                demoRecoder[i].videoConfig.nInputYuvFormat = VIDEO_PIXEL_YUV420_NV21;
            }else if((strcmp(argv[10+i*9],"nv12")==0)){
                demoRecoder[i].videoConfig.nInputYuvFormat = VIDEO_PIXEL_YUV420_NV12;
            }else{
                printf("warnning : the input yuv format is not support,use the default format:VIDEO_PIXEL_YUV420_YU12\n");
                demoRecoder[i].videoConfig.nInputYuvFormat = VIDEO_PIXEL_YUV420_YU12;
            }

            //set the framerate
            demoRecoder[i].videoConfig.nFrameRate  = 30;

            //use the phy buf
            demoRecoder[i].videoConfig.bUsePhyBuf  = 1;
        }

        //config AudioEncodeConfig
        memset(&demoRecoder[i].audioConfig, 0x00, sizeof(AudioEncodeConfig));
        if(demoRecoder[i].inputPCM != NULL){
            if(strcmp(argv[6+i*9],"pcm")==0){
		demoRecoder[i].audioConfig.nType		= AUDIO_ENCODE_PCM_TYPE;
            }else if(strcmp(argv[6+i*9],"aac")==0){
		demoRecoder[i].audioConfig.nType		= AUDIO_ENCODE_AAC_TYPE;
            }else if(strcmp(argv[6+i*9],"mp3")==0){
		demoRecoder[i].audioConfig.nType		= AUDIO_ENCODE_MP3_TYPE;
            }else{
		printf("the input audio encode type is not support,use the default:pcm\n");
		demoRecoder[i].audioConfig.nType		= AUDIO_ENCODE_PCM_TYPE;
            }
            printf("audio encode type = %d\n",demoRecoder[i].audioConfig.nType);
            demoRecoder[i].audioConfig.nInChan = 2;
            demoRecoder[i].audioConfig.nInSamplerate = 44100;
            demoRecoder[i].audioConfig.nOutChan = 2;
            demoRecoder[i].audioConfig.nOutSamplerate = 44100;
            demoRecoder[i].audioConfig.nSamplerBits = 16;
        }

        if(demoRecoder[i].muxType == CDX_MUXER_TS && demoRecoder[i].audioConfig.nType == AUDIO_ENCODE_PCM_TYPE)
        {
            demoRecoder[i].audioConfig.nFrameStyle = 2;
        }

        if(demoRecoder[i].muxType == CDX_MUXER_TS && demoRecoder[i].audioConfig.nType == AUDIO_ENCODE_AAC_TYPE)
        {
            demoRecoder[i].audioConfig.nFrameStyle = 1;//not add head when encode aac,because use ts muxer
        }

        if(demoRecoder[i].muxType == CDX_MUXER_AAC)
        {
            demoRecoder[i].audioConfig.nType = AUDIO_ENCODE_AAC_TYPE;
            demoRecoder[i].audioConfig.nFrameStyle = 0;//add head when encode aac
        }

        if(demoRecoder[i].muxType == CDX_MUXER_MP3)
        {
            demoRecoder[i].audioConfig.nType = AUDIO_ENCODE_MP3_TYPE;
        }

        demoRecoder[i].mAwEncoder = AwEncoderCreate(&demoRecoder[i]);
        if(demoRecoder[i].mAwEncoder == NULL)
        {
            printf("can not create AwRecorder, quit.\n");
            exit(-1);
        }

        //* set callback to recoder,if the encoder has used the buf ,it will callback to app
        AwEncoderSetNotifyCallback(demoRecoder[i].mAwEncoder,NotifyCallbackForAwEncorder,&(demoRecoder[i]));
        if((demoRecoder[i].inputPCM != NULL) && (demoRecoder[i].muxType == CDX_MUXER_AAC || demoRecoder[i].muxType == CDX_MUXER_MP3)){
            //only encode  audio
            printf("only init audio encoder\n");
            AwEncoderInit(demoRecoder[i].mAwEncoder, NULL, &demoRecoder[i].audioConfig,&mEncDataCallBackOps);
            demoRecoder[i].videoEos = 1;
        }else if((demoRecoder[i].inputYUV!=NULL)&&(demoRecoder[i].inputPCM==NULL)&&(demoRecoder[i].muxType == CDX_MUXER_MOV || demoRecoder[i].muxType == CDX_MUXER_TS)){
	    //only encode video
            printf("only init video encoder\n");
            AwEncoderInit(demoRecoder[i].mAwEncoder, &demoRecoder[i].videoConfig, NULL,&mEncDataCallBackOps);
            demoRecoder[i].audioEos = 1;
        }else if((demoRecoder[i].inputYUV!=NULL)&&(demoRecoder[i].inputPCM!=NULL)&&(demoRecoder[i].muxType == CDX_MUXER_MOV || demoRecoder[i].muxType == CDX_MUXER_TS)){
            //encode audio and video
            printf("init audio and video encoder\n");
            AwEncoderInit(demoRecoder[i].mAwEncoder, &demoRecoder[i].videoConfig, &demoRecoder[i].audioConfig,&mEncDataCallBackOps);
        }

        //store the muxer file path
        strcpy(demoRecoder[i].mSavePath,argv[4+i*9]);
        printf("demoRecoder.mSavePath = %s\n",demoRecoder[i].mSavePath);

        //start encode
        AwEncoderStart(demoRecoder[i].mAwEncoder);

        AwEncoderGetExtradata(demoRecoder[i].mAwEncoder,&demoRecoder[i].extractDataBuff,&demoRecoder[i].extractDataLength);

        if(demoRecoder[i].inputPCM != NULL){
            printf("create audio input thread\n");
            pthread_create(&demoRecoder[i].audioDataThreadId, NULL, AudioInputThread, &demoRecoder[i]);
        }
        if((demoRecoder[i].inputYUV != NULL) && ((demoRecoder[i].muxType == CDX_MUXER_MOV) || (demoRecoder[i].muxType == CDX_MUXER_TS)))
        {
            printf("create video input thread\n");
            pthread_create(&demoRecoder[i].videoDataThreadId, NULL, VideoInputThread, &demoRecoder[i]);
        }
        pthread_create(&demoRecoder[i].muxerThreadId, NULL, MuxerThread, &demoRecoder[i]);
    }

    //the following is for exit the main thread when encoded and muxer finish
    for(int i = 0 ; i < recorderNum; i++){
        CdxMuxerPacketT *mPacket = NULL;
        if(demoRecoder[i].muxerThreadId)
            pthread_join(demoRecoder[i].muxerThreadId, NULL);
        if(demoRecoder[i].audioDataThreadId)
            pthread_join(demoRecoder[i].audioDataThreadId, NULL);
        if(demoRecoder[i].videoDataThreadId)
            pthread_join(demoRecoder[i].videoDataThreadId, NULL);

        printf("exit:it is %d recorder stream\n",i);
        printf("destroy AwRecorder.\n");
        while (!CdxQueueEmpty(demoRecoder[i].mAudioDataQueue))
        {
            printf("free a audio packet\n");
            mPacket = CdxQueuePop(demoRecoder[i].mAudioDataQueue);
            free(mPacket->buf);
            free(mPacket);
        }
        CdxQueueDestroy(demoRecoder[i].mAudioDataQueue);
        AwPoolDestroy(demoRecoder[i].mAudioDataPool);

        while (!CdxQueueEmpty(demoRecoder[i].mVideoDataQueue))
        {
            printf("free a video packet\n");
            mPacket = CdxQueuePop(demoRecoder[i].mVideoDataQueue);
            free(mPacket->buf);
            free(mPacket);
        }
        CdxQueueDestroy(demoRecoder[i].mVideoDataQueue);
        AwPoolDestroy(demoRecoder[i].mVideoDataPool);

        if(demoRecoder[i].mAwEncoder != NULL)
        {
	    AwEncoderStop(demoRecoder[i].mAwEncoder);
            AwEncoderDestory(demoRecoder[i].mAwEncoder);
            demoRecoder[i].mAwEncoder = NULL;
        }

        if (demoRecoder[i].inputYUV)
        {
            fclose(demoRecoder[i].inputYUV);
            demoRecoder[i].inputYUV = NULL;
        }

        if (demoRecoder[i].inputPCM)
        {
            fclose(demoRecoder[i].inputPCM);
            demoRecoder[i].inputPCM = NULL;
        }
    }

    printf("\n");
    printf("******************************************************************************************\n");
    printf("* Quit the program, goodbye!\n");
    printf("******************************************************************************************\n");
    printf("\n");
    return 0;
}
