#define LOG_TAG "tplayer"
#include "tlog.h"
#include "tplayer.h"
#include "tsound_ctrl.h"
#include "tdisp_ctrl.h"

int CallbackFromXPlayer(void* pUserData, int msg, int ext1, void* param){
    TPlayer* tplayer = (TPlayer*)pUserData;
    TP_CHECK(tplayer);
    int appMsg = -1;
     switch(msg)
    {
        case AWPLAYER_MEDIA_PREPARED:
        {
            appMsg = TPLAYER_NOTIFY_PREPARED;
            break;
        }

        case AWPLAYER_MEDIA_PLAYBACK_COMPLETE:
        {
            appMsg = TPLAYER_NOTIFY_PLAYBACK_COMPLETE;
            break;
        }

        case AWPLAYER_MEDIA_SEEK_COMPLETE:
        {
            appMsg = TPLAYER_NOTIFY_SEEK_COMPLETE;
            break;
        }

        case AWPLAYER_MEDIA_ERROR:
        {
            appMsg = TPLAYER_NOTIFY_MEDIA_ERROR;
            switch(ext1)
            {
                case AW_MEDIA_ERROR_UNKNOWN:
                {
                    ext1 = TPLAYER_MEDIA_ERROR_UNKNOWN;
                    break;
                }
                case AW_MEDIA_ERROR_UNSUPPORTED:
                {
                    ext1 = TPLAYER_MEDIA_ERROR_UNSUPPORTED;
                    break;
                }
                case AW_MEDIA_ERROR_IO:
                {
                    ext1 = TPLAYER_MEDIA_ERROR_IO;
                    break;
                }
            }
            break;
        }
        case AWPLAYER_MEDIA_INFO:
        {
            switch(ext1)
            {
                case AW_MEDIA_INFO_NOT_SEEKABLE:
                {
                    appMsg = TPLAYER_NOTIFY_NOT_SEEKABLE;
                    break;
                }
                case AW_MEDIA_INFO_BUFFERING_START:
                {
                    appMsg = TPLAYER_NOTIFY_BUFFER_START;
                    break;
                }
                case AW_MEDIA_INFO_BUFFERING_END:
                {
                    appMsg = TPLAYER_NOTIFY_BUFFER_END;
                    break;
                }
            }
            break;
        }

        case AWPLAYER_MEDIA_SET_VIDEO_SIZE:
        {
            int w, h;
            w   = ((int*)param)[0];   //* read positon to total file size.
            h  = ((int*)param)[1];   //* cache buffer fullness.
            TLOGD("video width = %d,height = %d",w,h);
            appMsg = TPLAYER_NOTIFY_MEDIA_VIDEO_SIZE;
            break;
        }

        default:
        {
            //printf("warning: unknown callback from xplayer.\n");
            break;
        }
    }
    if(appMsg != -1){
        tplayer->mNotifier(tplayer->mUserData,appMsg,ext1,param);
    }
    return 0;
}

static int SubCallbackProcess(void* pUser, int eMessageId, void* param)
{
    TPlayer* tplayer = (TPlayer*)pUser;
    TP_CHECK(tplayer);
    TP_CHECK(param);
    //TLOGD("SubCallbackProcess   ");
    int msg = 1;
    switch(eMessageId)
    {
        case SUBCTRL_SUBTITLE_AVAILABLE:
            msg = TPLAYER_NOTIFY_SUBTITLE_FRAME;
            break;
        case SUBCTRL_SUBTITLE_EXPIRED:

            break;
        default:
            TLOGE("the msg is not support");
            break;
    }

    if(tplayer->mNotifier){
        if(msg != -1)
            tplayer->mNotifier(tplayer->mUserData,msg,0,param);
    }
    return 0;

}

//this function is callback by layer
static int LayerCallbackVideoframe(void* pUser,void* param){
    TPlayer* tplayer = (TPlayer*)pUser;
    VideoPicture *pic = (VideoPicture*)param;
    TP_CHECK(tplayer);
    TP_CHECK(param);
    //TLOGD("LayerCallbackVideoframe");
    VideoPicData data;
    memset(&data, 0x00, sizeof(VideoPicData));
    data.nPts = pic->nPts;
    data.nWidth = pic->nWidth;
    data.nHeight = pic->nHeight;
    data.ePixelFormat = pic->ePixelFormat;
    data.nBottomOffset = pic->nBottomOffset;
    data.nLeftOffset = pic->nLeftOffset;
    data.nRightOffset = pic->nRightOffset;
    data.nTopOffset = pic->nTopOffset;
    data.pData0 = pic->pData0;
    data.pData1 = pic->pData1;
    data.pData2 = pic->pData2;
    data.phyCBufAddr = pic->phyCBufAddr;
    data.phyYBufAddr = pic->phyYBufAddr;
    if(tplayer->mNotifier)
        tplayer->mNotifier(tplayer->mUserData,TPLAYER_NOTIFY_VIDEO_FRAME,0,&data);
    return 0;
}

//this function is callback by soundctrl
static int SoundctrlCallbackAudioframe(void* pUser,void* param){
    TPlayer* tplayer = (TPlayer*)pUser;
    SoundPcmData *pcmData = (SoundPcmData*)param;
    TP_CHECK(tplayer);
    TP_CHECK(param);
    //TLOGD("SoundctrlCallbackAudioframe");
    AudioPcmData data;
    memset(&data, 0x00, sizeof(AudioPcmData));
    data.pData = pcmData->pData;
    data.nSize = pcmData->nSize;
    data.samplerate = pcmData->samplerate;
    data.channels = pcmData->channels;
    data.accuracy = pcmData->accuracy;
    if(tplayer->mNotifier)
        tplayer->mNotifier(tplayer->mUserData,TPLAYER_NOTIFY_AUDIO_FRAME,0,&data);
    return 0;
}

TPlayer* TPlayerCreate(TplayerType type){
    TPlayer* mPrivateData = (TPlayer*)malloc(sizeof(TPlayer));
    if(mPrivateData == NULL){
        TLOGE("malloc TPlayer fail");
        return NULL;
    }
    TLOGD("TPlayerCreate");
    memset(mPrivateData,0x00,sizeof(TPlayer));
    mPrivateData->mPlayerType = type;//default is CEDARX_PLAYER
    mPrivateData->mUserData = NULL;
    mPrivateData->mNotifier = NULL;
    mPrivateData->mVolume = 20;//default is 20,which is the origin pcm
    mPrivateData->mMuteFlag = 0;
    mPrivateData->mHideSubFlag = 0;
    mPrivateData->mDebugFlag = 0;
    if(type == CEDARX_PLAYER){
        mPrivateData->mXPlayer = XPlayerCreate();
        if(mPrivateData->mXPlayer == NULL){
            TLOGE("XPlayerCreate fail");
            free(mPrivateData);
            mPrivateData = NULL;
            return NULL;
        }
        int checkRet = XPlayerInitCheck(mPrivateData->mXPlayer);
        if(checkRet == -1){
            TLOGE("the player init check fail");
            free(mPrivateData->mXPlayer);
            mPrivateData->mXPlayer = NULL;
            free(mPrivateData);
            mPrivateData = NULL;
            return NULL;
        }
        mPrivateData->mMediaInfo = NULL;
        mPrivateData->mSoundCtrl = TSoundDeviceCreate(SoundctrlCallbackAudioframe,mPrivateData);
        if(mPrivateData->mSoundCtrl == NULL){
            //if the mSoundCtrl is null,do not set it to xplayer
            TLOGE("sound device create fail,mPrivateData->mSoundCtrl is null");
        }else{
            XPlayerSetAudioSink(mPrivateData->mXPlayer,mPrivateData->mSoundCtrl);
        }
        mPrivateData->mLayerCtrl = LayerCreate(LayerCallbackVideoframe,mPrivateData);
        TP_LOG_CHECK(mPrivateData->mXPlayer,"TPlayer create layer failed!\n");

        mPrivateData->mSubCtrl = SubtitleCreate(SubCallbackProcess, mPrivateData);
        TP_LOG_CHECK(mPrivateData->mXPlayer,"TPlayer create subtitle ctrl failed!\n");

        mPrivateData->mDeinterlace = DeinterlaceCreate();
        TP_LOG_CHECK(mPrivateData->mXPlayer,"TPlayer create deinterlace ctrl failed!\n");

        XPlayerSetVideoSurfaceTexture(mPrivateData->mXPlayer, (void*)mPrivateData->mLayerCtrl);
        XPlayerSetSubCtrl(mPrivateData->mXPlayer, (void*)mPrivateData->mSubCtrl);
        XPlayerSetDeinterlace(mPrivateData->mXPlayer, (void*)mPrivateData->mDeinterlace);
    }
    return mPrivateData;
}

void TPlayerDestroy(TPlayer* p){
    TP_CHECK(p);
    if(p->mXPlayer != NULL){
	XPlayerDestroy(p->mXPlayer);
        p->mXPlayer = NULL;
    }
    free(p);
}

int TPlayerSetDebugFlag(TPlayer* p, bool debugFlag){
    TP_CHECK(p);
    p->mDebugFlag = debugFlag;
    return 0;
}

int TPlayerSetNotifyCallback(TPlayer* p,
                                    TPlayerNotifyCallback notifier,
                                    void* pUserData){
    int ret = -1;
    TP_CHECK(p);
    p->mNotifier = notifier;
    p->mUserData = pUserData;
    ret = XPlayerSetNotifyCallback(p->mXPlayer, CallbackFromXPlayer, (void*)p);
    return ret;
}

int TPlayerSetDataSource(TPlayer* p, const char* pUrl,
                         void* httpService, const CdxKeyedVectorT* pHeaders){
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    return XPlayerSetDataSourceUrl(p->mXPlayer,pUrl,httpService,pHeaders);
}

/* int TPlayerSetDataSourceFd(TPlayer* p, int fd, int64_t nOffset, int64_t nLength);*/

int TPlayerPrepare(TPlayer* p){
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    return XPlayerPrepare(p->mXPlayer);
}

int TPlayerPrepareAsync(TPlayer* p){
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    return XPlayerPrepareAsync(p->mXPlayer);
}

int TPlayerStart(TPlayer* p){
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    return XPlayerStart(p->mXPlayer);
}

int TPlayerPause(TPlayer* p){
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    return XPlayerPause(p->mXPlayer);
}

int TPlayerStop(TPlayer* p){
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    return XPlayerStop(p->mXPlayer);
}

int TPlayerReset(TPlayer* p){
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    return XPlayerReset(p->mXPlayer);
}

int TPlayerSeekTo(TPlayer* p, int nSeekTimeMs){
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    return XPlayerSeekTo(p->mXPlayer,nSeekTimeMs);
}

bool TPlayerIsPlaying(TPlayer* p){
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    return (bool)XPlayerIsPlaying(p->mXPlayer);
}

int TPlayerGetCurrentPosition(TPlayer* p, int* msec){
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    return XPlayerGetCurrentPosition(p->mXPlayer,msec);
}

int TPlayerGetDuration(TPlayer* p, int* msec){
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    return XPlayerGetDuration(p->mXPlayer,msec);
}

MediaInfo*  TPlayerGetMediaInfo(TPlayer* p){
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
     MediaInfo* mi = XPlayerGetMediaInfo(p->mXPlayer);
     p->mMediaInfo = mi;
     return mi;
}

int TPlayerSetLooping(TPlayer* p, bool bLoop){
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    if(bLoop)
        return XPlayerSetLooping(p->mXPlayer,1);
    else
        return XPlayerSetLooping(p->mXPlayer,0);
}

//this interface should be called before prepare
int TPlayerSetScaleDownRatio(TPlayer* p, TplayerVideoScaleDownType nHorizonScaleDown, TplayerVideoScaleDownType nVerticalScaleDown){
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    int horizonScaleDown = 0;
    int verticalScaleDown = 0;
    switch (nHorizonScaleDown)
    {
        case VIDEO_SCALE_DOWN_1:
        {
            horizonScaleDown = 0;
            break;
        }
        case VIDEO_SCALE_DOWN_2:
        {
            horizonScaleDown = 1;
            break;
        }
        case VIDEO_SCALE_DOWN_4:
        {
            horizonScaleDown = 2;
            break;
        }
        case VIDEO_SCALE_DOWN_8:
        {
            horizonScaleDown = 3;
            break;
        }
        default:
        {
            TLOGE("the nHorizonScaleDown value is wrong,nHorizonScaleDown = %d",nHorizonScaleDown);
            break;
        }
    }
    switch (nVerticalScaleDown)
    {
        case VIDEO_SCALE_DOWN_1:
        {
            verticalScaleDown = 0;
            break;
        }
        case VIDEO_SCALE_DOWN_2:
        {
            verticalScaleDown = 1;
            break;
        }
        case VIDEO_SCALE_DOWN_4:
        {
            verticalScaleDown = 2;
            break;
        }
        case VIDEO_SCALE_DOWN_8:
        {
            verticalScaleDown = 3;
            break;
        }
        default:
        {
            TLOGE("the nVerticalScaleDown value is wrong,nVerticalScaleDown = %d",nVerticalScaleDown);
            break;
        }
    }
    return XPlayerSetScaleDownRatio(p->mXPlayer,horizonScaleDown,verticalScaleDown);
}

//the xplayer do not supply this interface,and this interface should be called before prepare status
int TPlayerSetRotate(TPlayer* p, TplayerVideoRotateType rotate){
    return 0;
}

//this interface may have bug
int TPlayerSetSpeed(TPlayer* p, TplayerPlaySpeedType nSpeed){
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    int speed = 1;
    switch (nSpeed)
    {
        case PLAY_SPEED_FAST_FORWARD_16:
        {
            speed = 16;
            break;
        }
        case PLAY_SPEED_FAST_FORWARD_8:
        {
            speed = 8;
            break;
        }
        case PLAY_SPEED_FAST_FORWARD_4:
        {
            speed = 4;
            break;
        }
        case PLAY_SPEED_FAST_FORWARD_2:
        {
            speed = 2;
            break;
        }
        case PLAY_SPEED_1:
        {
            speed = 1;
            break;
        }
        case PLAY_SPEED_FAST_BACKWARD_2:
        {
            speed = -2;
            break;
        }
        case PLAY_SPEED_FAST_BACKWARD_4:
        {
            speed = -4;
            break;
        }
        case PLAY_SPEED_FAST_BACKWARD_8:
        {
            speed = -8;
            break;
        }
        case PLAY_SPEED_FAST_BACKWARD_16:
        {
            speed = -16;
            break;
        }
        default:
        {
            TLOGE("the nSpeed value is wrong,nSpeed = %d",nSpeed);
            break;
        }
    }
    return XPlayerSetSpeed(p->mXPlayer,speed);
}

/* the following  reference to mediaplayer */
//int TPlayerGetVideoWidth(TPlayer* p, int* width); /* media info has include, no need*/
//int TPlayerGetVideoHeight(TPlayer* p, int* height); /*media info has include, no need*/
//int TPlayerGetSubtitleStreamNum(TPlayer* p, int* nStreamNum);/* media info has include, no need*/
//int TPlayerGetAudioStreamNum(TPlayer* p, int* nStreamNum);/* media info has include, no need*/

/**
 *  Set volume with specified value.
 *  @param[in] volume Specified volume value to be set.
 *  @return TRUE if success. Otherwise, return FALSE.
 */
int TPlayerSetVolume(TPlayer* p, int volume){
    TP_CHECK(p);

    if(p->mSoundCtrl){
        if(volume > 40){
            TLOGE("the volume(%d) is larger than the largest volume(40),set it to 40",volume);
            volume = 40;
        }else if(volume < 0){
            TLOGE("the volume(%d) is smaller than 0,set it to 0",volume);
            volume =0;
        }
        p->mVolume = volume;
        volume -= 20;
        return TSoundDeviceSetVolume(p->mSoundCtrl,volume);
    }else{
        TLOGE("p->mSoundCtrl is NULL,can not set the volume");
        return -1;
    }
}

int TPlayerGetVolume(TPlayer* p){
    TP_CHECK(p);
    return p->mVolume;
}

int TPlayerSetAudioMute(TPlayer* p ,bool mute){
    TP_CHECK(p);
    if(p->mSoundCtrl){
        if(mute){
            int muteVolume = -20;
            return TSoundDeviceSetVolume(p->mSoundCtrl,muteVolume);
        }else{
            return TSoundDeviceSetVolume(p->mSoundCtrl,p->mVolume);
        }
    }else{
        TLOGE("p->mSoundCtrl is NULL,can not set the volume mute");
        return -1;
    }
}


int TPlayerSetExternalSubUrl(TPlayer* p, const char* filePath){
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    TP_CHECK(filePath);
    return XPlayerSetExternalSubUrl(p->mXPlayer,filePath);
}

int TPlayerSetExternalSubFd(TPlayer* p, int fd, int64_t offset, int64_t len, int fdSub){
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    return XPlayerSetExternalSubFd(p->mXPlayer,fd,offset,len,fdSub);
}

int TPlayerGetSubDelay(TPlayer* p){
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    return XPlayerGetSubDelay(p->mXPlayer);
}

int TPlayerSwitchAudio(TPlayer* p, int nStreamIndex){
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
    if(p->mMediaInfo != NULL){
        if(nStreamIndex >= 0 && nStreamIndex<p->mMediaInfo->nAudioStreamNum){
            return XPlayerSwitchAudio(p->mXPlayer,nStreamIndex);
        }else{
            TLOGE("the nStreamIndex is wrong,can not switch audio.  nStreamIndex = %d",nStreamIndex);
        }
    }
    return -1;
}

int TPlayerSetSubDelay(TPlayer* p, int nTimeMs)
{
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
	return XPlayerSetSubDelay(p->mXPlayer, nTimeMs);
}

int TPlayerGetSubCharset(TPlayer* p, char *charset)
{
    TP_CHECK(p);
    TP_CHECK(p->mXPlayer);
	if(NULL == charset) {
		TLOGE("charset invalid!\n");
		return -1;
	}
	return XPlayerGetSubCharset(p->mXPlayer, charset);
}

int TPlayerSetSubCharset(TPlayer* p, const char* strFormat)
{
	TP_CHECK(p);
	TP_CHECK(p->mXPlayer);

	if(NULL == strFormat) {
		TLOGE("strFormat invalid!\n");
		return -1;
	}
	return XPlayerGetSubCharset(p->mXPlayer, strFormat);
}

int TPlayerSwitchSubtitle(TPlayer* p, int nStreamIndex)
{
	TP_CHECK(p);
	TP_CHECK(p->mXPlayer);

	if(nStreamIndex < 0)
		return -1;

	return XPlayerSwitchSubtitle(p->mXPlayer, nStreamIndex);
}

void TPlayerSetSubtitleDisplay(TPlayer* p, bool onoff)
{
	TP_CHECK(p);
	TP_CHECK(p->mXPlayer);
	TP_CHECK(p->mSubCtrl);

	SubtitleDisplayOnoff(p->mSubCtrl, onoff);
}

void TPlayerSetVideoDisplay(TPlayer* p, bool onoff)
{
	TP_CHECK(p);
	TP_CHECK(p->mXPlayer);
	TP_CHECK(p->mLayerCtrl);

	return LayerDisplayOnoff(p->mLayerCtrl, onoff);
}

void TPlayerSetDisplayRect(TPlayer* p, int x, int y, unsigned int width, unsigned int height)
{
	TP_CHECK(p);
	TP_CHECK(p->mXPlayer);
	TP_CHECK(p->mLayerCtrl);

	return LayerSetDisplayRect(p->mLayerCtrl, x, y, width, height);
}

void TPlayerSetBrightness(TPlayer* p, unsigned int grade)
{
	TP_CHECK(p);
	TP_CHECK(p->mXPlayer);
	TP_CHECK(p->mLayerCtrl);

	LayerSetControl(p->mLayerCtrl, LAYER_CMD_SET_BRIGHTNESS, grade);
}

void TPlayerSetContrast(TPlayer* p, unsigned int grade)
{
	TP_CHECK(p);
	TP_CHECK(p->mXPlayer);
	TP_CHECK(p->mLayerCtrl);

	LayerSetControl(p->mLayerCtrl, LAYER_CMD_SET_CONTRAST, grade);
}
void TPlayerSetHue(TPlayer* p, unsigned int grade)
{
	TP_CHECK(p);
	TP_CHECK(p->mXPlayer);
	TP_CHECK(p->mLayerCtrl);

	LayerSetControl(p->mLayerCtrl, LAYER_CMD_SET_HUE, grade);
}

void TPlayerSetSaturation(TPlayer* p, unsigned int grade)
{
	TP_CHECK(p);
	TP_CHECK(p->mXPlayer);
	TP_CHECK(p->mLayerCtrl);

	LayerSetControl(p->mLayerCtrl, LAYER_CMD_SET_SATURATION, grade);
}

void TPlayerSetEnhanceDefault(TPlayer* p)

{
	TP_CHECK(p);
	TP_CHECK(p->mXPlayer);
	TP_CHECK(p->mLayerCtrl);

	LayerSetControl(p->mLayerCtrl, LAYER_CMD_SET_VEDIO_ENHANCE_DEFAULT, 0);
}
