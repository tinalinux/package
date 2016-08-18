#ifndef __AUDIO_SETTING_H__
#define __AUDIO_SETTING_H__
#include <stdlib.h>
#include <stdio.h>

#include "EventWatcher.h"
#include "InputEventWatcher.h"

#define NAME_MAX_SIZE 100

namespace Parrot {
typedef struct _volume_elem {
    int soft_volume;
    int hw_volume;
}tVolumeElem;

class AudioSetting : public EventWatcher::WatcherListener{
public:
    AudioSetting();
    virtual ~AudioSetting();

    /**/
    /* hardware volume setting */
    virtual int setVolume(int value);
    virtual int getVolume();

    // virtual volume 0~99
    virtual int setVirtualVolume(int value);
    virtual int getVirtualVolume();

    virtual int downVolume();
    virtual int upVolume();
    virtual int setMute(){};
    virtual int setUnmute(){};
    virtual int setMaxVolume(){};
    virtual int setMinVolume(){};

    virtual int setChannel(int channel){};

    /* Line In setting*/
    typedef enum _status
    {
        STATUS_OUT = 0,
        STATUS_IN = 32,
        STATUS_SIGNAL = 64,
    }tLineInStatus;

    class AudioLineInStatusListener{
    public:
        virtual void onAudioInStatusChannged(tLineInStatus status, struct timeval time) = 0;
    };

    int fetchAudioLineInStatus();
    int initAudioLineInDetect(AudioLineInStatusListener* l);
    void destoryAudioLineInDetect();
    int switchtoLineInChannel(bool is_line_in);
    int setListener(AudioLineInStatusListener* l);

    int configureCardName(const char *card);
    int configureVolumeName(const char *name);
    int configureVolumeMap(const tVolumeElem *map);
    int configureLineInJackName(const char *name);
    int configureLineInStatusPath(const char *path);
    int configureLinInStatusSwitch(const char *name);
    int configureWatcher(ref_EventWatcher watcher);

protected:

    void onEvent(EventWatcher::tEventType type,int index, void *data, int len, void* args);

    int amixer_set(const char *name, const char *value);
    int amixer_get(const char *name);
    int amixer_opreation(const char *name, const char *value, int roflag, int *get);

    //need lock??
    bool mIsMute;
    int mMuteVolume;
    int mCurVolume;

    char Card[NAME_MAX_SIZE];
    char VolumeName[NAME_MAX_SIZE];
    char LineInJack[NAME_MAX_SIZE];
    char LineInStatusPath[NAME_MAX_SIZE];
    char LineInSwitchName[NAME_MAX_SIZE];

private:
    AudioLineInStatusListener *mAudioLineInStatusListener;
    ref_EventWatcher mWatcher;

    tLineInStatus mLineInStatus;

    InputEventWatcher *mInputEventWatcher;
};
}/*namespace softwinner*/
#endif /*__AUDIO_SETTING_H__*/
