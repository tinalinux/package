#define TAG "PPlayer"
#define CONFIG_TLOG_LEVEL OPTION_TLOG_LEVEL_DETAIL
#include <tina_log.h>

#include "parrot_common.h"
#include "PPlayer.h"
#define F_LOG plogd("%s, line: %d", __FUNCTION__, static);
/*
static const char* status_str[] = {
    "PLAYER_STATUS_IDLE",
    "PLAYER_STATUS_PLAYING",
    "PLAYER_STATUS_PLAYING",
    "PLAYER_STATUS_PAUSED",
    "PLAYER_STATUS_SEEKING",
    "PLAYER_STATUS_COMPELTE"
};
*/
namespace Parrot{

static tStateString player_state_string[] =
{
  { PPlayer::STATUS_IDLE,      "PLAYER_STATUS_IDLE"      },
  { PPlayer::STATUS_PREPARING, "PLAYER_STATUS_PREPARING" },
  { PPlayer::STATUS_PREPARED,  "PLAYER_STATUS_PREPARED"  },
  { PPlayer::STATUS_PLAYING,   "PLAYER_STATUS_PLAYING"   },
  { PPlayer::STATUS_PAUSED,    "PLAYER_STATUS_PAUSED"    },
  { PPlayer::STATUS_SEEKING,   "PLAYER_STATUS_SEEKING"   },
  { PPlayer::STATUS_COMPELTE,  "PLAYER_STATUS_COMPELTE"  },
  { PPlayer::STATUS_STOPPED,   "PLAYER_STATUS_STOPPED"   },
};

//* a callback for awplayer.
void CallbackForTinaPlayer(void* pUserData, int msg, int param0, void* param1)
{
    PPlayer* pPlayer = (PPlayer*)pUserData;

    pPlayer->onNotifyCallback(pUserData,msg,param0,param1);
}

PPlayer::PPlayer(ref_HandlerBase base):
    Handler(base),
    mListener(NULL),
    mPlayerStatus(10,"PPlayer",player_state_string,sizeof(player_state_string)/sizeof(tStateString))
{
    mAwPlayer = NULL;
    mPlayerStatus.reset(STATUS_IDLE);
    mPlayListEnable = true;
    mPlaymode = PLAYMODE_REPEAT_ALL;
    mPlaylistListener = NULL;
    mListener = NULL;
}
PPlayer::~PPlayer(){

}
/*
reset()
setDefaultNotify(bool mode){
    if("reset")
        mAwPlayer->setNotifyCallback(NULL, NULL);
    else
        mAwPlayer->setNotifyCallback(CallbackForTinaPlayer, (void*)this);

}
getTinaPlayer(){
    return mAwPlayer;
}
setPlayDLNA()
*/
int PPlayer::init(){

    mAwPlayer = new TinaPlayer();
    CHECK_POINTER(mAwPlayer);

    mAwPlayer->setNotifyCallback(CallbackForTinaPlayer, (void*)this);

    if(mAwPlayer->initCheck() != 0){
        ploge("initCheck of the player fail, quit.\n");
        delete mAwPlayer;
        return FAILURE;
    }

    mPlayerStatus.update(STATUS_IDLE);
    return SUCCESS;
}
int PPlayer::release(){
    DELETE_POINTER(mAwPlayer);
    return SUCCESS;
}

int PPlayer::setDataSource(const char* pUrl){

    locker::autolock l(mPlayerLock);

    int status =  mPlayerStatus.get(StateManager::CUR);

    if(status != STATUS_IDLE)
    {
        ploge("invalid command:\n");
        ploge("    play is not in stopped status.\n");
        return -status;
    }

    int ret = mAwPlayer->setDataSource((const char*)pUrl, NULL);
    if(ret != 0){
        ploge("PPlayer setDataSource failed\n");
        return -status;
    }
    mSeekable = 1;   //* if the media source is not seekable, this flag will be
                     //* clear at the NOTIFY_NOT_SEEKABLE callback.
    return status;
}
int PPlayer::prepare(){

    locker::autolock l(mPlayerLock);

    int status =  mPlayerStatus.get(StateManager::CUR);
    //prepare will block,until prepare finish
    if(mAwPlayer->prepare() != 0){
        ploge("error:\n");
        ploge("    AwPlayer::prepare() return fail.\n");
        return -status;
    }
    //mPreStatus = STATUS_IDLE;
    //mStatus    = STATUS_PREPARED;
    mPlayerStatus.update(STATUS_PREPARED);
    plogd("prepared...\n");
    return STATUS_PREPARED;
}
int PPlayer::prepareAsync(){

    locker::autolock l(mPlayerLock);

    int status =  mPlayerStatus.get(StateManager::CUR);
    //prepareAsync no block
    if(mAwPlayer->prepareAsync() != 0){
        ploge("error:\n");
        ploge("    AwPlayer::prepareAsync() return fail.\n");
        return -status;
    }
    //mPreStatus = STATUS_IDLE;
    //mStatus    = STATUS_PREPARING;
    mPlayerStatus.update(STATUS_PREPARING);
    plogd("preparing...\n");
    return STATUS_PREPARING;
}
int PPlayer::start(){

    locker::autolock l(mPlayerLock);
    int status = mPlayerStatus.get(StateManager::CUR);
    if(status != STATUS_PREPARED &&
       status != STATUS_SEEKING &&
       status != STATUS_PAUSED &&
       status != STATUS_STOPPED &&
       status != STATUS_COMPELTE) {
        ploge("invalid command:\n");
        ploge("    player is neither in prepared status nor in seeking.\n");
        return -status;
    }

    if(status == STATUS_STOPPED)
    {
        if(mPlayerStatus.get(StateManager::PRE) == STATUS_STOPPED)
        {
            ploge("error:\n");
            ploge("    player is stop, the prestatus also is stop\n");
            return -status;
        }
        else
        {
            //play(pause,complete) -> stop -> play
            mAwPlayer->prepare();//?? or prepareAsync
            mPlayerStatus.update(STATUS_PREPARED);
        }
    }

    //* start the playback

    plogd("start");
    if(mAwPlayer->start() != 0){
        ploge("error:\n");
        ploge("    AwPlayer::start() return fail.\n");
        return -status;
    }
    //mPreStatus = mStatus;
    //mStatus    = STATUS_PLAYING;
    mPlayerStatus.update(STATUS_PLAYING);

    if(mPlaylistListener != NULL)
        mPlaylistListener->onPPlayerStatusChanged(STATUS_PLAYING);

    return STATUS_PLAYING;
}

int PPlayer::stop(){

    locker::autolock l(mPlayerLock);

    int status = mPlayerStatus.get(StateManager::CUR);
    if(status != STATUS_PLAYING &&
       status != STATUS_SEEKING &&
       status != STATUS_PREPARED &&
       status != STATUS_COMPELTE){
        ploge("invalid command:\n");
        ploge("    player status %d can not stop.\n",status);
        return -status;
    }

    if(mAwPlayer->stop() != 0) {
        ploge("error:\n");
        ploge("    AwPlayer::stop() return fail.\n");
        return -status;
    }
    //mPreStatus = mStatus;
    //mStatus    = STATUS_IDLE;
    mPlayerStatus.update(STATUS_STOPPED);
    if(mPlaylistListener != NULL)
        mPlaylistListener->onPPlayerStatusChanged(STATUS_STOPPED);
    return STATUS_STOPPED;
}
int PPlayer::reset(){

    locker::autolock l(mPlayerLock);
/*
    if(mPlayListEnable && mPlayList.size() > 0 && isPlaying() == 1){
        MediaElem *elem = getCurPlayListElem();
        getCurrentPosition(&elem->seek_point);
        ploge("-------------------------------setting seek to %d ",elem->seek_point);
    }
*/
    int status = mPlayerStatus.get(StateManager::CUR);
    if(mAwPlayer->reset() != 0) {
        ploge("error:\n");
        ploge("    AwPlayer::reset() return fail.\n");
        return -status;
    }
    mPlayerStatus.update(STATUS_IDLE);

    if(mPlaylistListener != NULL)
        mPlaylistListener->onPPlayerStatusChanged(STATUS_IDLE);

    return STATUS_IDLE;
}
int PPlayer::pause(){

    locker::autolock l(mPlayerLock);

    int status = mPlayerStatus.get(StateManager::CUR);

    if(status != STATUS_PLAYING &&
       status != STATUS_SEEKING){
        ploge("invalid command:\n");
        ploge("    player is neither in playing status nor in seeking status.\n");
        return -status;
    }

    //* pause the playback
    if(mAwPlayer->pause() != 0) {
        ploge("error:\n");
        ploge("    AwPlayer::pause() return fail.\n");
        return -status;
    }
    //mPreStatus = mStatus;
    //mStatus    = STATUS_PAUSED;
    mPlayerStatus.update(STATUS_PAUSED);

    if(mPlaylistListener != NULL)
        mPlaylistListener->onPPlayerStatusChanged(STATUS_PAUSED);

    plogd("paused.\n");
    return STATUS_PAUSED;
}
int PPlayer::isPlaying(){
    return mAwPlayer->isPlaying();
}
int PPlayer::seekTo(int msec){
    int nSeekTimeMs;
    int nDuration;

    nSeekTimeMs = msec;

    int status = mPlayerStatus.get(StateManager::CUR);

    if(status != STATUS_PLAYING &&
       status != STATUS_SEEKING &&
       status != STATUS_PAUSED  &&
       status != STATUS_PREPARED &&
       status != STATUS_COMPELTE) {
        ploge("invalid command:\n");
        ploge("    player is not in playing/seeking/paused/prepared status.\n");
        return -status;
    }

    if(getDuration(&nDuration) == 0)
    {
        if(nSeekTimeMs > nDuration){
            ploge("seek time out of range, media duration = %u seconds.\n", nDuration/1000);
            return -status;
        }
    }

    if(mSeekable == 0)
    {
        ploge("media source is unseekable.\n");
        return -status;
    }

    //* the player will keep the pauded status and pause the playback after seek finish.
    //* sync with the seek finish callback.

    locker::autolock l(mPlayerLock);

    mAwPlayer->seekTo(nSeekTimeMs);
    if(status != STATUS_SEEKING){
        mPlayerStatus.update(STATUS_SEEKING);
    }else
        plogd("the status already is SEEKING, keep!");
    return STATUS_SEEKING;
}

int PPlayer::getCurrentPosition(int* msec){
    return mAwPlayer->getCurrentPosition(msec);
}
int PPlayer::getDuration(int* msec){
    return mAwPlayer->getDuration(msec);
}

int PPlayer::setLooping(int bLoop){
    return mAwPlayer->setLooping(bLoop);
}

void PPlayer::onNotifyCallback(void* pUserData, int msg, int param0, void* param1){
    switch(msg)
    {
        case TINA_NOTIFY_NOT_SEEKABLE:
        {
            mSeekable = 0;
            plogd("info: media source is unseekable.\n");
            break;
        }

        case TINA_NOTIFY_ERROR:
        {
            locker::autolock l(mPlayerLock);
            //mStatus = STATUS_IDLE;
            //mPreStatus = STATUS_IDLE;
            mPlayerStatus.update(STATUS_IDLE);
            mPlayerStatus.update(STATUS_IDLE);
            plogd("error: open media source fail.\n");
            break;
        }

        case TINA_NOTIFY_PREPARED:
        {
            plogd("info: prepare ok.\n");
            {
                locker::autolock l(mPlayerLock);
                //mPreStatus = mStatus;
                //mStatus = STATUS_PREPARED;
                //updateStatus(STATUS_PREPARED,mStatus);

                mPlayerStatus.update(STATUS_PREPARED);
                sendMessage(CMD_PREPARED);
            }

            break;
        }

        case TINA_NOTIFY_BUFFERRING_UPDATE:
        {
            int nBufferedFilePos;
            int nBufferFullness;

            nBufferedFilePos = param0 & 0x0000ffff;
            nBufferFullness  = (param0>>16) & 0x0000ffff;
            plogd("info: buffer %d percent of the media file, buffer fullness = %d percent.\n",
                nBufferedFilePos, nBufferFullness);

            break;
        }

        case TINA_NOTIFY_PLAYBACK_COMPLETE:
        {
            //* stop the player.
            //* TODO
            {
                locker::autolock l(mPlayerLock);
                //mPreStatus = mStatus;
                //mStatus = STATUS_PREPARED;
                mPlayerStatus.update(STATUS_COMPELTE);
                sendMessage(CMD_PLAYBACK_COMPLETE);
            }

            plogd("playback completed.\n");
            break;
        }

        case TINA_NOTIFY_SEEK_COMPLETE:
        {
            {
                locker::autolock l(mPlayerLock);
                //mStatus = mPreStatus;
                //updateStatus(mPreStatus,mPreStatus);??
                sendMessage(CMD_SEEK_COMPLETE);
            }

            plogd("info: seek ok.\n");
            break;
        }

        default:
        {
            ploge("warning: unknown callback from AwPlayer.\n");
            break;
        }
    }
}

void PPlayer::setPlayerListener(PPlayerListener *l)
{
    mListener = l;
}
void PPlayer::handleMessage(Message *msg)
{
    switch(msg->what){
    case CMD_SEEK_COMPLETE:
        if(mListener != NULL)
            mListener->onSeekComplete();
        //start();
        break;
    case CMD_PREPARED:
        if(mListener != NULL)
            mListener->onPrepared();
        break;
    case CMD_PLAYBACK_COMPLETE:
        //play once?
        if(mPlayListEnable &&  mPlayList.size() > 0){
            if(fetch_index_by_playmode() == PLAY_KEEP)
                sendMessage(CMD_PLAY_LIST); //fail, try next playlist elem
        }
        else {
            if(mListener != NULL)
                mListener->onPlaybackComplete();
        }
        break;
    case CMD_PLAY_LIST:{
            ploge("===CMD_PLAY_LIST===");
            if(mPlayListEnable &&  mPlayList.size() > 0){

                MediaElem *elem = getCurPlayListElem();
                if(elem == NULL) break;
                if(elem->url.size() == 0){
                    if(mPlaylistListener != NULL)
                        mPlaylistListener->onPPlayerFetchPlayListElem(elem);
                    else
                        break;
                }

                if((reset() < 0 ) || \
                    (setDataSource(elem->url.data()) < 0) || \
                    (prepare() < 0)) {
                     sendMessage(CMD_PLAY_LIST); //fail, play next
                     //fail, do some about this elem
                     elem->fail_count++;
                }
                else{
                     //success, do some about this elem
                    int ret = -1;
                    /*
                    list<MediaElem*>::iterator iter = mPlayList.begin();
                    for(iter = mPlayList.begin(); iter != mPlayList.end(); iter++)
                        ploge(" ------- %d",(*iter)->seek_point);
                    */
                    if(elem->seek_point != 0){
                        int duration;
                        getDuration(&duration);
                        ploge("-------------------seek to %d",elem->seek_point);
                        ret = seekTo(elem->seek_point*duration/100);
                        elem->seek_point = 0; //reset seek point
                    }
                    if(ret < 0){
                        ret = start();
                        //linstener play change
                        if(mPlaylistListener != NULL)
                            mPlaylistListener->onPPlayerPlayChanged();
                    }
                    if(ret < 0){
                        sendMessage(CMD_PLAY_LIST); //fail, play next
                        //fail, do some about this elem
                        elem->fail_count++;
                    }else
                        elem->fail_count = 0;
                }
            }
            break;
        }
    }
}

int PPlayer::setAudioSetting(AudioSetting *as)
{
    mAudioSetting = as;
    return SUCCESS;
}
AudioSetting* PPlayer::getAudioSetting()
{
    return mAudioSetting;
}

void PPlayer::enablePlayList(bool enable)
{
    mPlayListEnable = enable;
}

void PPlayer::setPlaymode(PPlayer::tPlaymode mode)
{
    mPlaymode = mode;
}

/*
void PPlayer::updatePlayMap(int index, MediaElem *elem)
{
    locker::autolock l(mPlayMapLock);
    mPlayMap[index] = elem;
}

void PPlayer::resetPlayMap()
{
    locker::autolock l(mPlayMapLock);
    map<int, MediaElem*>::iterator it;
    for (it = mPlayMap.begin(); it != mPlayMap.end();){
        MediaElem *elem = it->second;
        delete elem;
        mPlayMap.erase(it++);
    }
    ploge("resetPlayMap size %d", mPlayMap.size());
    mPlayMap.clear();
    mCurPlayListIndex = 0;
}

MediaElem *PPlayer::getPlayMapElem(int index)
{
    return mPlayMap[index];
}

MediaElem *PPlayer::getCurPlayMapElem()
{
    return getPlayMapElem(mCurPlayListIndex);
}
*/

void PPlayer::setPlaylistListener(PPlayerPlaylistListener *l)
{
    mPlaylistListener = l;
}

void PPlayer::updatePlayList()
{
    //do notify
    if(mPlaylistListener != NULL)
        mPlaylistListener->onPPlayerPlaylistupdated();
}

void PPlayer::addPlayList(MediaElem *elem)
{
    locker::autolock l(mPlayListLock);
    mPlayList.push_back(elem);
}

void PPlayer::resetPlayList()
{
    locker::autolock l(mPlayListLock);
    while(mPlayList.size() > 0){
        MediaElem* elem = mPlayList.front();
        mPlayList.pop_front();
        delete elem;
    }
    mPlayList.clear();
    mCurPlayListIndex = 0;
    plogd("restPlayList end!!!");
}

list<MediaElem*> PPlayer::getPlayList()
{
    return mPlayList;
}

MediaElem *PPlayer::getPlayListElem(int index)
{
    locker::autolock l(mPlayListLock);
    int size = mPlayList.size();
    if(size == 0) return NULL;

    list<MediaElem*>::iterator iter = mPlayList.begin();

    index = (index + size) % size;

    for (iter = mPlayList.begin(); iter != mPlayList.end(); iter++){
        if((*iter)->index == index)
            break;
    }
    plogd("get --> index = %d",     (*iter)->index);
    plogd("get --> size = %s",      (*iter)->size.data());
    plogd("get --> id = %d",        (*iter)->id);
    plogd("get --> lrcSize = %d",   (*iter)->lrcSize);
    plogd("get --> img = %s",       (*iter)->img.data());
    plogd("get --> artist = %s",    (*iter)->artist.data());
    plogd("get --> title = %s",     (*iter)->title.data());
    plogd("get --> imgSize = %s",   (*iter)->imgSize.data());
    plogd("get --> url = %s",       (*iter)->url.data());
    plogd("get --> lrcurl = %s\n",  (*iter)->lrcurl.data());

    return *iter;
}

int PPlayer::getCurPlayListElemIndex()
{
    locker::autolock l(mPlayListLock);
    return mCurPlayListIndex;
}

MediaElem* PPlayer::getCurPlayListElem()
{
    ploge("getCurPlayListElem %d",mCurPlayListIndex);
    return getPlayListElem(mCurPlayListIndex);
}

int PPlayer::getPlayListSize()
{
    locker::autolock l(mPlayListLock);
    return mPlayList.size();
}

int PPlayer::playlist()
{
    //pause -> playlist
    //stop(reset) -> playlist
    if(mPlayList.size() > 0){
        if(mPlayerStatus.get(StateManager::CUR) == STATUS_PAUSED)
            start();
        else
            sendMessage(CMD_PLAY_LIST);
        return SUCCESS;
    }
    else
        return FAILURE;
}

int PPlayer::playlist(int index, int seek)
{
    locker::autolock l(mPlayListLock);
    mCurPlayListIndex = index;
    if(seek != 0){
        getCurPlayListElem()->seek_point = seek;
    }
    return playlist();
}

int PPlayer::next()
{
    locker::autolock l(mPlayListLock);
    mCurPlayListIndex++;
    sendMessage(CMD_PLAY_LIST);
    return SUCCESS;
}

int PPlayer::prev()
{
    locker::autolock l(mPlayListLock);
    mCurPlayListIndex--;
    sendMessage(CMD_PLAY_LIST);
    return SUCCESS;
}

PPlayer::tPlaKeep PPlayer::fetch_index_by_playmode()
{
    locker::autolock l(mPlayListLock);
    tPlaKeep ret = PLAY_KEEP;
    switch(mPlaymode){
    case PLAYMODE_REPEAT_ALL:{
        //循环播放 all repeat
        mCurPlayListIndex++;
        break;
    }
    case PLAYMODE_ORDER:{
        //顺序播放 order //todo: test
        if(mCurPlayListIndex + 1 == mPlayList.size())
            ret = PLAY_STOP;
        else{
            mCurPlayListIndex++;
        }
        break;
    }
    case PLAYMODE_SHUFFLE:{
        //随机播放 shuffle //todo: test
        mCurPlayListIndex = random()%mPlayList.size();
        break;
    }
    case PLAYMODE_REPEAT_ONCE:{
        //单曲循环 repeat once //todo: test
        break;
    }
    case PLAYMODE_ONCE:{
        //单曲播放 once //todo: test
        ret = PLAY_STOP;
        break;
    }
    }
    return ret;
}
}/*namespace softwinner*/
