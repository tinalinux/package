#include <stdio.h>

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <dlfcn.h>
#include <stdio.h>
#include <linux/input.h>

#include <vector>
#include <algorithm>
#include <functional>

#include "parrot_common.h"
#include "StateManager.h"
#include "EventWatcher.h"
#include "KeyEventWatcher.h"
#include "NetworkConnection.h"
#include "HandlerBase.h"
#include "Handler.h"
#include "SocketClientWatcher.h"
#include "Speech.h"
#include "PPlayer.h"
#include "AudioSetting.h"
#include "SunxiCodec.h"

#include "BTMode.h"

#include "Led.h"

#include "PostMsgPackage.h"
#include "CHttpClient.h"
#include "XimalayaApi.h"

#include "ConfigUtils.h"

#include "ControlorBase.h"
#include "KeyControlor.h"
#include "RemoteControlor.h"

#include "UI.h"


#include "BroadcastSender.h"

#include "ActionKeeper.h"

using namespace std;
using namespace Parrot;

void dump(int signo)
{
    void *buffer[30] = {0};
    size_t size;
    char **strings = NULL;
    size_t i = 0;
    plogd("dump~!!!!!!");
/*
    size = backtrace(buffer, 30);
    fprintf(stdout, "Obtained %zd stack frames.nm\n", size);
    strings = backtrace_symbols(buffer, size);
    if (strings == NULL)
    {
        perror("backtrace_symbols.");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < size; i++)
    {
        fprintf(stdout, "%s\n", strings[i]);
    }
    free(strings);
    strings = NULL;
*/
    exit(0);
}

class Keeper : public ActionKeeper::KeeperAction
{
public:
    Keeper(ref_HandlerBase base): mActionKeeper(base){};
    ~Keeper(){};

    int init(){
        mActionKeeper.init(10, this, 2000);
        mActionKeeper.action();
        mActionKeeper.action();
        mActionKeeper.action();
        //mActionKeeper.keep();
    }
    int onKeeperAction(int key, int *interval){
        ploge("111111 %d",key);mActionKeeper.stop();
    }
private:
    ActionKeeper mActionKeeper;
};

class myParrot
{
public:

    void do_control_power()
    {
        //
    }

    void do_control_volume(int up_down)
    {

    }

    void do_control_play()
    {
        //mode play
    }

    void do_control_change_mode(Mode::tMode mode)
    {

    }

    void do_control_wifi()
    {

    }


    int init()
    {
        //init UI settting

        ConfigUtils::init();
        UI::init();
        ploge("%s %d",__func__,__LINE__);
        //int watcher
        mWatcher = new EventWatcher();
        CHECK_POINTER( mWatcher.AsPointer() );
        mWatcher->startWatcher();
        ploge("%s %d",__func__,__LINE__);
        //init handler
        mHandlerbase = new HandlerBase(mWatcher, 5);
        CHECK_POINTER( mHandlerbase.AsPointer() );

        //Keeper keep(mHandlerbase);
        //keep.init();

        //while(1);
        ploge("%s %d",__func__,__LINE__);
        mAudioSetting = new SunxiCodec();
        ploge("%s %d",__func__,__LINE__);
        CHECK_POINTER( mAudioSetting );
        ploge("%s %d",__func__,__LINE__);
        mAudioSetting->configureWatcher(mWatcher);
        ploge("%s %d",__func__,__LINE__);
        //init player
        mPlayer = new PPlayer(mHandlerbase);
        CHECK_POINTER( mPlayer.AsPointer() );
        CHECK( mPlayer->init());
        CHECK( mPlayer->setAudioSetting(mAudioSetting) );

        mNetworkConnection = NetworkConnection::createInstance(mHandlerbase);

        mControlorBase = new ControlorBase(mPlayer);
        CHECK_POINTER(mControlorBase);
        CHECK( mControlorBase->init(mWatcher, mHandlerbase) );

        mKeyControlor = new KeyControlor();
        CHECK_POINTER(mKeyControlor);
        CHECK( mKeyControlor->init(mWatcher, mHandlerbase, mControlorBase) );

        mRemoteControlor = new RemoteControlor(mWatcher, mHandlerbase);
        CHECK_POINTER(mRemoteControlor);
        CHECK( mRemoteControlor->init(mWatcher, mHandlerbase, mControlorBase) );
        mRemoteControlor->setPlayer(mPlayer);

        mControlorBase->start();
        mRemoteControlor->start();

        if(mNetworkConnection->init() == FAILURE) exit(-1);
        //init wifi

        //wifi ok -> enable speech mode

        //wifi failed -> notify to start smartlink to set wifi

        //wait mode selector

        //mControlorBase->doControlEnableMode(Mode::MODE_SPEECH);

    }
    int release()
    {
        mWatcher->stopWatcher();
    }

private:
    void init_audio_settting()
    {
        mAudioSetting = new SunxiCodec();
        mAudioSetting->configureWatcher(mWatcher);
        mAudioSetting->initAudioLineInDetect(NULL);
    }

    void test_handler()
    {
        Handler *handler = new Handler(mHandlerbase);
        for(int i = 0; i < 2; i++){
            handler->sendMessageDelayed(i,0,i*10000);
        }
        while(1);
    }
    void test_speech()
    {
        Speech s(mWatcher,mHandlerbase);
        s.setPlayer(mPlayer);
        s.onCreate();
        while(1);
    }
    void test_audiosettting()
    {
        mAudioSetting->configureWatcher(mWatcher);
        mAudioSetting->initAudioLineInDetect(NULL);
        int i = 15;
        while(i--){
            mAudioSetting->upVolume();
            sleep(1);
            system("amixer cget numid=1");
        }
        i =15;
        while(i--){
            mAudioSetting->downVolume();
            sleep(1);
            system("amixer cget numid=1");
        }
    }

private:
    ref_EventWatcher mWatcher;
    ref_HandlerBase  mHandlerbase;
    ref_PPlayer mPlayer;

    NetworkConnection *mNetworkConnection;
    Speech *mSpeechMode;
    BTMode *mBTMode;
    AudioSetting *mAudioSetting;

    KeyControlor *mKeyControlor;
    ControlorBase *mControlorBase;
    RemoteControlor *mRemoteControlor;

    //tMode mCurMode;


};
void test_ximalaya()
    {
        allwinner::XimalayaApi api;
        int errorCode = -1;

        if (api.SecureAccessToken(errorCode))
        {
            int ids[1] =
            { 15967459 };
            std::string result;
#if 0
            if (api.TracksHot(result, errorCode))
            {
                cout << result << " errorCode:" + errorCode << endl;
                /*
                Json::Reader trackRespReader;
                Json::Value trackRespValue;
                if (trackRespReader.parse(result, trackRespValue))
                {
                    Json::Value tracksArray = trackRespValue["tracks"];
                    if (tracksArray.size() >= 1)
                    {
                        Json::Value track = tracksArray[0];

                        ids[0] = track["id"].asInt();
                    }
                }
                */
            }

            if (api.TracksGetBatch(result, errorCode, &ids[0], 1))
            {
                /*
                Json::Reader trackRespReader;
                Json::Value trackRespValue;
                if (trackRespReader.parse(result, trackRespValue))
                {
                    Json::Value tracksArray = trackRespValue["tracks"];
                    if (tracksArray.size() == 1)
                    {
                        Json::Value track = tracksArray[0];

                        if (track["id"].asInt() == ids[0])
                        {
                            std::string url = track["play_url_32"].asString();
                            std::cout << "play_url_32 ----" << url << std::endl;
                        }
                    }
                }
                */
            }
#endif
            //api.LiveRadios(result, errorCode, 1);
            //string & result, int & errorCode, const int * ids, const int idLen
            ids[0] = 13453208;
            api.TracksGetBatch(result, errorCode, ids, 1);
            cout << result << endl;

            //api.RanksRadios(result, errorCode, 10);
            //cout << result << endl;
//
            //api.SearchRadios(result, errorCode, "音乐");
            //cout << result << endl;
//
            //api.SearchAlbums(result, errorCode, "刘德华");
            //cout << result << endl;
//
            //api.AlbumsBrowse(result, errorCode, 1);
            //cout << result << endl;
//
            //api.SearchTracks(result, errorCode, "刘德华");
            //cout << result << endl;
//
            //api.SecureAccessToken(errorCode);
        }
    }
void sysUsecTime()
{
    struct timeval    tv;
    struct timezone tz;

    struct tm *p;

    gettimeofday(&tv, &tz);

    p = localtime(&tv.tv_sec);
    printf("time_now:%d-%d-%d-%d-%d-%d.%ld\n", 1900+p->tm_year, 1+p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, tv.tv_usec);
}

int main(){
    //test_ximalaya();
    //while(1);

    sysUsecTime();


    map<int,int*> m;

    ploge("%p",m[10]);

    //test_ximalaya();
    //Led led1(1);
    //led1.on();
    //sleep(4);
    //led1.off();
    //led1.flash();
    //sleep(10);
    //led1.doubleflash();

    printf("%d\n",(-1)%5);
    //signal(SIGSEGV, dump);
    myParrot mp;
    mp.init();

    sleep(1);
    //mp.release();
    printf("aaa\n");
    while(1);
    return 0;
}