#ifndef __UI_H__
#define __UI_H__

#include "Led.h"
#include <locker.h>
namespace Parrot {

class UI
{
public:
    UI();
    ~UI();

    static int init();
    static int release();

    static void FirstStart(bool should_say);

    static void NetWorkConnectFailed(bool should_say);
    static void NetWorkConnected(bool should_say);

    static void StartSmartLink(bool should_say);
    static void SmartLinkEnd(bool should_say);

    static void BluetoothModeStart(bool should_say);
    static void BluetoothConnected(bool should_say);
    static void BluetoothConnectfailed(bool should_say);
    static void BluetoothParing(bool should_say);

    static void LineInModeStart(bool should_say);
    static void LineInMediaIn(bool should_say);
    static void LineInMediaOut(bool should_say);

    static void CommonSay(bool should_say);

private:
    static void all_led_off();
    static void say(const char *target);
    static void flash(Led *led, Led::tLedStatus status);

private:
    static Led *mWifiLed;
    static Led *mBTLed;
    static Led *mLineInLed;
    static Led *mPowerLed;
    static Led *mBatteryLed;

    static locker *mPlayerLocker;
    static int mIsPlay;
    static sem *mPlaySem;

};/*class Parrot*/
}/*namespace Parrot*/
#endif/*__UI_H__*/