#include <stdio.h>
#include <string.h>

#include "ControlorBase.h"

#include "parrot_common.h"
#include "EventWatcher.h"
#include "HandlerBase.h"
#include "Speech.h"
#include "BTMode.h"
#include "LineInMode.h"

#include "UI.h"

namespace Parrot{

void ControlorBase::first_start()
{
    //make sure app first start access /etc/wifi/wpa_supplicant.conf
    //UI Hi我是小乐请通过手机APP帮助我联网吧

    const char *file = "/etc/my-parrot";
    if(access(file, F_OK) != 0){
        //config file is no exist, make it!
        FILE *fp = fopen(file, "wb+");
        if(fp == NULL) {
            ploge("first_start open file failed! %s",strerror(errno));
            return;
        }
        const char *done = "done\n";
        fwrite(done, strlen(done)+1, 1, fp);
        fflush(fp);
        fclose(fp);
        UI::FirstStart(true);
    }else{
        UI::CommonSay(true);
    }
}

int ControlorBase::init(ref_EventWatcher watcher, ref_HandlerBase base)
{

    //init wifi
    mNetworkConnection = NetworkConnection::getInstance();
    mNetworkConnection->addListener(this);

    //init all support mode
    Mode *mode;

    mode = new BTMode(watcher, base);
    CHECK_POINTER(mode);
    mModeList.push_back(mode);
    mode->setPlayer(mPlayer);
    mode->CreateAsync();

    mode = new LineInMode(watcher, base);
    CHECK_POINTER(mode);
    mModeList.push_back(mode);
    mode->setPlayer(mPlayer);
    mode->Create();

    mode = new Speech(watcher, base);
    CHECK_POINTER(mode);
    mModeList.push_back(mode);
    mode->setPlayer(mPlayer);
    mode->Create();

    mCurMode = NULL;

    first_start();

    return SUCCESS;
}

int ControlorBase::release()
{

}

int ControlorBase::start()
{
    //if(mNetworkConnection->init() == FAILURE) exit(-1);
}

int ControlorBase::stop()
{

}

NetworkConnection *ControlorBase::getNetworkConnection()
{
    return mNetworkConnection;
}

Mode* ControlorBase::getCurMode()
{
    return mCurMode;
}

int ControlorBase::setPlayer(ref_PPlayer player)
{
    mPlayer = player;
    return SUCCESS;
}

int ControlorBase::doControlPower()
{

}

int ControlorBase::doControlVolume(int up_down)
{
    int ret;
    if(up_down > 0){
        locker::autolock l(mVolumeLocker);
        ret = mPlayer->getAudioSetting()->upVolume();
        //UI::CommonSay(true);
    }
    else{
        locker::autolock l(mVolumeLocker);
        ret = mPlayer->getAudioSetting()->downVolume();
        //UI::CommonSay(true);
    }
    if (mControlorStatusListener != NULL) {
        mControlorStatusListener->onControlVolumeChanged(up_down);
    }

    return ret;
}

int ControlorBase::doControlPlay()
{
    if(mCurMode != NULL){
        int ret = mCurMode->play();
        if (mControlorStatusListener != NULL) {
            mControlorStatusListener->onControlPlayChanged();
        }
        return ret;
    }
    return SUCCESS;
}

int ControlorBase::doControlPause()
{
    if(mCurMode != NULL){
        int ret =mCurMode->pause();
        if (mControlorStatusListener != NULL) {
            mControlorStatusListener->onControlPlayChanged();
        }
        return ret;
    }
    return SUCCESS;
}

int ControlorBase::doControlSwitchPlay()
{
    if(mCurMode != NULL){
        int ret = mCurMode->switchplay();
        if (mControlorStatusListener != NULL) {
            mControlorStatusListener->onControlPlayChanged();
        }
        return ret;
    }
    return SUCCESS;

}

int ControlorBase::doControlEnableMode(Mode::tMode mode)
{
    // Speech <-> BT OK
    // BT <-> LineIn OK
    //LineIn -> Speech 有问题
    //mModeLocker.lock();
    if(mCurMode != NULL){
        if(mCurMode->getModeIndex() == mode){
            //mModeLocker.unlock();
            return SUCCESS;
        }
        mCurMode->StopAsync();
        /*
        if( mCurMode->Stop() != SUCCESS){
            ploge("Mode: %d stop failed!",mCurMode->getModeIndex());
            return FAILURE;
        }
        */
    }

    Mode *m = get_mode(mode);
    if(m != NULL){
        mCurMode = m;
        mCurMode->StartAsync();
    }
    if (mControlorStatusListener != NULL) {
        mControlorStatusListener->onControlModeChanged();
    }
    //mModeLocker.unlock();
    return SUCCESS;
}

int ControlorBase::doControlNetwork()
{
    if(mCurMode != NULL){
        mCurMode->Stop();
        mCurMode = NULL;
    }
    UI::StartSmartLink(true);
    mNetworkConnection->startSmartlink();
    return SUCCESS;
}

int ControlorBase::doControlPlayNext()
{
    if(mCurMode != NULL)
        return mCurMode->next();
}

int ControlorBase::doControlPlayPrev()
{
    if(mCurMode != NULL)
        return mCurMode->prev();
}

int ControlorBase::doControlPlaylist(int pos, int seek)
{
    if(mCurMode != NULL){
        if(mCurMode->getModeIndex() == Mode::MODE_SPEECH){
            mPlayer->playlist(pos, seek);
        }
    }
}

int ControlorBase::doControlSeek(int seek)
{
    if(mCurMode != NULL){
        if(mCurMode->getModeIndex() == Mode::MODE_SPEECH){
            mPlayer->seekTo(seek);
        }
    }
}

void ControlorBase::onNetworkSmartlinkdDetected()
{
    //UI: 声波已接收,正在联网
    //绿色，Double快闪
    UI::SmartLinkEnd(true);
}

void ControlorBase::onNetworkSmartlinkdFailed()
{
    UI::NetWorkConnectFailed(true);
}

void ControlorBase::onNetworkConnected()
{
    //绿色，常亮
    //mModeLocker.lock();
    if (mCurMode == NULL){
        //mModeLocker.unlock();
        doControlEnableMode(Mode::MODE_SPEECH);
        //UI: 联网成功,来听听我的推荐音乐吧
        UI::NetWorkConnected(true);

    }else{
        //mModeLocker.unlock();
        //播放短的联网提示音
        UI::NetWorkConnected(false);
        UI::CommonSay(true);
    }

}

void ControlorBase::onNetworkConnectFailed()
{
    //联网失败,请重试
    //按照手机提示操作smartlink
    //mModeLocker.lock();
    if (mCurMode == NULL){
        //mModeLocker.unlock();
        UI::NetWorkConnectFailed(true);
    }else{
        //mModeLocker.unlock();
        //播放短的联网提示音
        UI::NetWorkConnectFailed(false);
        UI::CommonSay(true);
    }
}

void ControlorBase::onNetworkDisconnected()
{
    //网络未连接
    //按照手机提示操作smartlink
}

Mode *ControlorBase::get_mode(Mode::tMode mode)
{
    ploge("ControlorBase::get_mode: %d",mode);
    //mModeLocker.lock();
    list<Mode*>::iterator it;
    for(it = mModeList.begin(); it != mModeList.end();it++) {
        if((*it)->getModeIndex() == mode){
            //mModeLocker.unlock();
            return *it;
        }
    }
    //mModeLocker.unlock();
    return NULL;
}

}/*namespace Parrot*/