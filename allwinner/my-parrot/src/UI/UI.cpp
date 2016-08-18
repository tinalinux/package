
#include <mp3player.h>

#include "parrot_common.h"
#include "UI.h"

static const char media_path[] = "/usr/share/my-parrot";

static const char first_boot[] = "first_boot.mp3";
static const char bluetooth_connected[] = "bluetooth_connected.mp3";
static const char bluetooth_failed[] = "bluetooth_failed.mp3";
static const char bluetooth_pairing[] = "bluetooth_pairing.mp3";
static const char chargging[] = "chargging.mp3";
static const char extenal_media[] = "extenal_media.mp3";
static const char extenal_media_in[] = "extenal_media_in.mp3";
static const char max_volume[] = "max_volume.mp3";
static const char min_volume[] = "min_volume.mp3";
static const char network_connect_fail_retry[] = "network_connect_fail_retry.mp3";
static const char network_connected_music[] = "network_connected_music.mp3";
static const char see_you[] = "see_you.mp3.mp3";
static const char see_you_again[] = "see_you_again.mp3";
static const char sound_received_connectting[] = "sound_received_connectting.mp3";
static const char start_bluetooth_mode[] = "start_bluetooth_mode.mp3";
static const char start_smartlink[] = "start_smartlink.mp3";
static const char take_a_break[] = "take_a_break.mp3";
static const char common[] = "common.mp3";


namespace Parrot{

Led *UI::mWifiLed = new Led(1);
Led *UI::mBTLed = new Led(2);
Led *UI::mLineInLed =  mWifiLed;
Led *UI::mPowerLed =   mWifiLed;
Led *UI::mBatteryLed = mWifiLed;
int UI::mIsPlay = 0;

locker *UI::mPlayerLocker = new locker();
sem *UI::mPlaySem = new sem();

UI::UI(){}

UI::~UI(){}

int UI::init()
{

}

int UI::release()
{

}

void UI::FirstStart(bool should_say)
{
    if(should_say)
        say(first_boot);
}

void UI::NetWorkConnected(bool should_say)
{
    flash(mWifiLed, Led::ON);
    if(should_say)
        say(network_connected_music);
}

void UI::NetWorkConnectFailed(bool should_say)
{
    flash(mWifiLed, Led::FLASH);
    if(should_say)
        say(network_connect_fail_retry);
}

void UI::StartSmartLink(bool should_say)
{
    flash(mWifiLed, Led::DOUBLEFLASH);
    if(should_say)
        say(start_smartlink);
}
void UI::SmartLinkEnd(bool should_say)
{
    if(should_say)
        say(sound_received_connectting);
}

void UI::BluetoothModeStart(bool should_say)
{
    if(should_say)
        say(start_bluetooth_mode);
}

void UI::BluetoothConnected(bool should_say)
{
    flash(mBTLed, Led::ON);
    if(should_say)
        say(bluetooth_connected);
}

void UI::BluetoothConnectfailed(bool should_say)
{
    flash(mBTLed, Led::FLASH);
    if(should_say)
        say(bluetooth_failed);
}

void UI::BluetoothParing(bool should_say)
{
    flash(mBTLed, Led::DOUBLEFLASH);
    if(should_say)
        say(bluetooth_pairing);
}


void UI::LineInModeStart(bool should_say)
{
    if(should_say)
        say(extenal_media);
}

void UI::LineInMediaIn(bool should_say)
{
    flash(mLineInLed, Led::ON);
    if(should_say)
        say(extenal_media_in);
}

void UI::LineInMediaOut(bool should_say)
{
    flash(mLineInLed, Led::OFF);
    if(should_say)
        say(common);
}

void UI::CommonSay(bool should_say)
{
    if(should_say)
        say(common);
}

void UI::say(const char *target)
{

    static bool should_post = false;
    char file[100];
    snprintf(file, sizeof(file), "%s/%s", media_path, target);
    plogd("playing  -->>  %s  play flag: %d",file, mIsPlay);
    if(mIsPlay == 1){
        tinymp3_stop();
        should_post = true; //locker this??
        mPlaySem->wait();
    }
    mIsPlay = 1;
    tinymp3_play(file);
    mIsPlay = 0;
    if(should_post){
        mPlaySem->post();
        should_post = false;
    }

}

void UI::flash(Led *led, Led::tLedStatus status)
{
    //all_led_off();
    //led->set(status);
}

void UI::all_led_off()
{
    mWifiLed->off();
    mBTLed->off();
    mLineInLed->off();
    mPowerLed->off();
    mBatteryLed->off();
}

}