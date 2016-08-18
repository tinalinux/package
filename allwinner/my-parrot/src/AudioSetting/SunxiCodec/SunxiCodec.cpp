#include "parrot_common.h"
#include "SunxiCodec.h"

namespace Parrot {
/*
static tVolumeElem SunxiCodecVolumeMap[] =
{
    { 0, 00 },
    { 1, 06 },
    { 2, 12 },
    { 3, 19 },
    { 4, 25 },
    { 5, 32 },
    { 6, 38 },
    { 7, 42 },
    { 8, 50 },
    { 9, 56 },
};
*/
static tVolumeElem SunxiCodecVolumeMap[] =
{
    { 0, 20 },
    { 1, 24 },
    { 2, 28 },
    { 3, 32 },
    { 4, 36 },
    { 5, 40 },
    { 6, 45 },
    { 7, 50 },
    { 8, 54 },
    { 9, 60 },
};
SunxiCodec::SunxiCodec()
{
    configureCardName("default");
    configureVolumeName("numid=1");
    configureLineInJackName("audiocodec linein Jack");
    configureLineInStatusPath("/sys/module/sun8iw5_sndcodec/parameters/AUX_JACK_DETECT");
    configureLinInStatusSwitch("numid=26");
    mCurVolume = 2;
    setVolume(mCurVolume);
}

int SunxiCodec::setVolume(int value)
{
    int len = sizeof(SunxiCodecVolumeMap)/sizeof(tVolumeElem);
    for(int i = 0 ; i < len; i++){
            if(SunxiCodecVolumeMap[i].soft_volume == value){
                if(AudioSetting::setVolume(SunxiCodecVolumeMap[i].hw_volume) < 0)
                    return FAILURE;
                mCurVolume = value;
                return mCurVolume;
        }
    }
    return FAILURE;
}

int SunxiCodec::getVolume()
{
    return mCurVolume;
    int i = 0;
    int len = sizeof(SunxiCodecVolumeMap)/sizeof(tVolumeElem);
    int hw_volume = AudioSetting::getVolume();
    for(i = 0 ; i < len; i++){
        if(SunxiCodecVolumeMap[i].hw_volume < hw_volume){
           continue;
        }else break;
    }
    return SunxiCodecVolumeMap[i].soft_volume;
}

int SunxiCodec::setVirtualVolume(int value)
{
    return setVolume(value/10 - 1);
}
int SunxiCodec::getVirtualVolume()
{
    return (getVolume() + 1) * 10;
}

int SunxiCodec::downVolume()
{
    int ret = getVolume();
    if(ret < 0)
        return ret;
    return setVolume(ret - 1);
}
int SunxiCodec::upVolume()
{
    int ret = getVolume();
    if(ret < 0)
        return ret;
    return setVolume(ret + 1);
}
int SunxiCodec::setMute()
{
    //should lock??
    mIsMute = true;
    mMuteVolume = getVolume();
    return setVolume(0);
}
int SunxiCodec::setUnmute()
{
    mIsMute = false;
    return setVolume(mMuteVolume);
}
int SunxiCodec::setMaxVolume()
{
    return setVolume(9);
}
int SunxiCodec::setMinVolume()
{
    return setMute();
}
}