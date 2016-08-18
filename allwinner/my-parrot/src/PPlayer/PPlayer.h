#ifndef __CEDARX_PLAYER_ADAPTER_H__
#define __CEDARX_PLAYER_ADAPTER_H__

#include <string>
#include <list>
#include <map>

#include <allwinner/tinaplayer.h>
#include <locker.h>

#include "Handler.h"
#include "StateManager.h"
#include "T_Reference.h"
#include "AudioSetting.h"

using namespace aw;
using namespace std;

namespace Parrot{
class MediaElem
{
public:
    enum _SrcType
    {
        MEDIA_XIMALAYA = 1,
        MEDIA_XIAMI = 2,
        MEDIA_SPEECH = 3,
        MEDIA_LOCAL = 4
    };

    MediaElem():fail_count(0),seek_point(0){};

    int index;

    enum _SrcType type;
    int musictype;

    int id;

    string img;
    string artist;
    string title;
    string imgSize;
    string url;
    string lrcurl;
    string album;



    //useless now
    string size;
    int lrcSize;

    int seek_point; //0~99
    int fail_count;
};
/*
class ximalaya : public MediaElem{
    int id;
};
class xiami : public MediaElem{
    int id;
};
*/
class PPlayer : public Handler {
public:

    typedef enum _Playmode
    {
        PLAYMODE_REPEAT_ALL = 0,    //循环播放 all repeat
        PLAYMODE_ORDER,             //顺序播放 order
        PLAYMODE_SHUFFLE,           //随机播放 shuffle
        PLAYMODE_REPEAT_ONCE,       //单曲循环 repeat once
        PLAYMODE_ONCE               //单曲播放 once
    }tPlaymode;

    typedef enum _PlayKeep
    {
        PLAY_KEEP = 0,    //继续播放列表
        PLAY_STOP,        //停止
    }tPlaKeep;

    enum _HandleCmd
    {
        CMD_SEEK_COMPLETE,
        CMD_PREPARED,
        CMD_PLAYBACK_COMPLETE,
        CMD_PLAY_LIST
    };

    typedef enum _PlayerStatus
    {
        STATUS_IDLE = 0xf0,
        STATUS_PREPARING,
        STATUS_PREPARED ,
        STATUS_PLAYING,
        STATUS_PAUSED,
        STATUS_SEEKING,
        STATUS_STOPPED,
        STATUS_COMPELTE,
    }tPlayerStatus;

public:
    PPlayer(ref_HandlerBase base);
	~PPlayer();

    class PPlayerListener {
    public:
        virtual void onPrepared() = 0;
        virtual void onPlaybackComplete() = 0;
        virtual void onSeekComplete() = 0;
    };

    virtual int setDataSource(const char* pUrl);
    virtual int prepare();
    virtual int prepareAsync();
    virtual int start();
    virtual int stop();
    virtual int pause();
    virtual int reset();
    virtual int isPlaying();
    virtual int seekTo(int msec);

    virtual int getCurrentPosition(int* msec);
    virtual int getDuration(int* msec);

    virtual int setLooping(int bLoop);

    int init();
    int release();

    void setPlayerListener(PPlayerListener *l);

    int setAudioSetting(AudioSetting *as);
    AudioSetting* getAudioSetting();

    class PPlayerPlaylistListener {
    public:
        //播放状态
        virtual void onPPlayerStatusChanged(tPlayerStatus status){}; //播放状态更新： play pause stop reset
        virtual void onPPlayerPlayChanged(){};  //列表播放状态更新：上下首播放

        //列表状态
        virtual void onPPlayerPlaylistChanged(int index){}; //列表某一项变化
        virtual void onPPlayerPlaylistupdated(){}; //列表整体刷写
        virtual void onPPlayerPlaylistReset(){}; //列表重置
        virtual void onPPlayerFetchPlayListElem(MediaElem *elem){}; //列表某项不能播放，请求填充细节 如url
    };

    void setPlaylistListener(PPlayerPlaylistListener *l);

    void enablePlayList(bool enable);
    void setPlaymode(tPlaymode mode);

    void addPlayList(MediaElem *elem);
    void updatePlayList();
    void resetPlayList();
    list<MediaElem*> getPlayList();
    MediaElem *getCurPlayListElem();
    int getCurPlayListElemIndex();
    MediaElem *getPlayListElem(int index);

    int getPlayListSize();

/*
    void updatePlayMap(int index, MediaElem *elem);
    void resetPlayMap();
    MediaElem *getCurPlayMapElem();
    MediaElem *getPlayMapElem(int index);
    int getPlayMapSize();
*/

    int playlist();
    int playlist(int index, int seek);
    int next();
    int prev();

    //callback by Tinaplayer
    void onNotifyCallback(void* pUserData, int msg, int param0, void* param1);

    //virtual function in Handler
    virtual void handleMessage(Message *msg);

private:
    tPlaKeep fetch_index_by_playmode();  //return 0: keep playing 1: stop play
private:
    TinaPlayer      *mAwPlayer;
    int             mSeekable;
    locker          mPlayerLock;
    StateManager    mPlayerStatus;
    PPlayerListener *mListener;
    AudioSetting    *mAudioSetting;

    bool            mPlayListEnable;
    list<MediaElem*> mPlayList;
    locker          mPlayListLock;
    int             mCurPlayListIndex;
    PPlayerPlaylistListener *mPlaylistListener;
    tPlaymode       mPlaymode;


    map<int,MediaElem*> mPlayMap;
    locker          mPlayMapLock;
};
typedef T_Reference<PPlayer> ref_PPlayer;
}/*namespace Parrot*/
#endif /*__CEDARX_PLAYER_ADAPTER_H__*/
