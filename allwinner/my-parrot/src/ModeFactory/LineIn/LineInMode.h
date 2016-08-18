#ifndef __LINE_IN_H__
#define __LINE_IN_H__

#include "EventWatcher.h"
#include "HandlerBase.h"
#include "AudioSetting.h"
#include "Mode.h"

namespace Parrot{

class LineInMode : public Mode, public AudioSetting::AudioLineInStatusListener
{
public:
    LineInMode(ref_EventWatcher watcher, ref_HandlerBase handlerbase);
    ~LineInMode();

    int onCreate();
    int onStart();
    int onStop();
    int onDestory();

    int start(){};
    int stop(){};

protected:
    void onAudioInStatusChannged(AudioSetting::tLineInStatus status, struct timeval time);
};

}/*namespace Parrot*/


#endif /*__LINE_IN_H__*/
