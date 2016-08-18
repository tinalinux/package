#ifndef __SUNXICODEC_H__
#define __SUNXICODEC_H__
#include "AudioSetting.h"

namespace Parrot {

class SunxiCodec : public AudioSetting
{
public:
    SunxiCodec();
    virtual ~SunxiCodec(){};

    /*relative volume by map*/
    virtual int setVolume(int value);
    virtual int getVolume();
    virtual int setVirtualVolume(int value);
    virtual int getVirtualVolume();

    virtual int downVolume();
    virtual int upVolume();
    virtual int setMute();
    virtual int setUnmute();
    virtual int setMaxVolume();
    virtual int setMinVolume();


};
}/*namespace softwinner*/
#endif /*__SUNXICODEC_H__*/