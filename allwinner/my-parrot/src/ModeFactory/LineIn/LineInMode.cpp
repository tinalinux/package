#include "parrot_common.h"
#include "LineInMode.h"
#include "UI.h"
namespace Parrot{

LineInMode::LineInMode(ref_EventWatcher watcher, ref_HandlerBase handlerbase):
    Mode(watcher, handlerbase)
{
    mMode = MODE_LINEIN;
}

LineInMode::~LineInMode()
{

}

int LineInMode::onCreate()
{
    plogd("LineIn Mode onCreate!");
    CHECK(mPlayer->getAudioSetting()->initAudioLineInDetect(NULL));
    Mode::onCreate();
    return SUCCESS;
}

int LineInMode::onStart()
{
    UI::LineInModeStart(true);
    plogd("LineIn Mode onStart!");
    mPlayer->getAudioSetting()->setListener(this);
    CHECK(mPlayer->getAudioSetting()->switchtoLineInChannel(true));
    Mode::onStart();
    return SUCCESS;
}

int LineInMode::onStop()
{
    plogd("LineIn Mode onStop!");
    //mPlayer->getAudioSetting()->destoryAudioLineInDetect();
    mPlayer->getAudioSetting()->setListener(NULL);
    plogd("LineIn Mode onStop111!");
    CHECK(mPlayer->getAudioSetting()->switchtoLineInChannel(false));
    plogd("LineIn Mode onStop222!");
    Mode::onStop();
    return SUCCESS;
}

int LineInMode::onDestory()
{
    mPlayer->getAudioSetting()->destoryAudioLineInDetect();
    plogd("LineIn Mode onDestory!");
    Mode::onDestory();
    return SUCCESS;
}

void LineInMode::onAudioInStatusChannged(AudioSetting::tLineInStatus status, struct timeval time)
{
    if(status == AudioSetting::STATUS_IN){
        UI::LineInMediaIn(true);
    }
    else{
        UI::LineInMediaOut(true);
    }
}
}/*namespace Parrot*/
