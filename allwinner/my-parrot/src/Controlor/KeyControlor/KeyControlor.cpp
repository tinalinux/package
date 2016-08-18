#include "parrot_common.h"
#include "KeyControlor.h"
#include "BTMode.h"

namespace Parrot{
#define LONG_CLICK_TIME 1500 //ms
int KeyControlor::init(ref_EventWatcher watcher, ref_HandlerBase base, ControlorBase *controlorbase)
{
    mControlor = controlorbase;
    //init key
    KeyEventWatcher *key = new KeyEventWatcher(watcher, base, "sunxi-keyboard", this);
    CHECK_POINTER(key);
    key->addLongClickTime(KEY_CTL_WIFI, LONG_CLICK_TIME);
    key->addLongClickTime(KEY_CTL_UP_NEXT, LONG_CLICK_TIME);
    key->addLongClickTime(KEY_CTL_DOWN_PREV, LONG_CLICK_TIME);
    key->addLongClickTime(KEY_CTL_PLAY, LONG_CLICK_TIME);
    key->addLongClickTime(KEY_CTL_BT_LINEIN, LONG_CLICK_TIME);
    mKeylist.push_back(key);

    key = new KeyEventWatcher(watcher, base, "axp22-supplyer", this);
    CHECK_POINTER(key);
    key->addLongClickTime(KEY_CTL_POWER, 1000);
    mKeylist.push_back(key);

    return SUCCESS;
}

int KeyControlor::release()
{
    while(mKeylist.size() > 0){
        KeyEventWatcher *key = mKeylist.back();
        delete key;
        mKeylist.pop_back();
    }

    return SUCCESS;
}

void KeyControlor::onKeyDown(int fdindex, int keycode)
{
    plogd("key: %d down",keycode);
}

void KeyControlor::onKeyUp(int fdindex, int keycode)
{
    plogd("key: %d up",keycode);
    switch(keycode){
        case KEY_CTL_POWER:
            //mControlor->doControlPower();
            mControlor->doControlSwitchPlay();
            break;
        case KEY_CTL_UP_NEXT:
            mControlor->doControlVolume(1);
            break;
        case KEY_CTL_DOWN_PREV:
            mControlor->doControlVolume(0);
            break;
        case KEY_CTL_PLAY:{
            mControlor->doControlSwitchPlay();
            break;
        }
        case KEY_CTL_BT_LINEIN:{
            Mode *mode = mControlor->getCurMode();
            if(mode != NULL && mode->getModeIndex() == Mode::MODE_BT)
                mControlor->doControlEnableMode(Mode::MODE_LINEIN);
            else
                mControlor->doControlEnableMode(Mode::MODE_BT);
            break;
        }

        case KEY_CTL_WIFI:
            //mControlor->doControlWifi();
            mControlor->doControlEnableMode(Mode::MODE_SPEECH);
            break;
        default:
        ;
    }
}
void KeyControlor::onkeyLongClick(int fdindex, int keycode, int longclicktime)
{
    plogd("key: %d longclick: %d",keycode,longclicktime);
    switch(keycode){
    case KEY_CTL_POWER:
        mControlor->doControlNetwork();
        break;
    case KEY_CTL_UP_NEXT:
        mControlor->doControlPlayNext();
        break;
    case KEY_CTL_DOWN_PREV:
        mControlor->doControlPlayPrev();
        break;
    case KEY_CTL_BT_LINEIN: {
        if(longclicktime == LONG_CLICK_TIME){
            Mode *mode = mControlor->getCurMode();

            if(mode != NULL && mode->getModeIndex() == Mode::MODE_BT){
               BTMode* btmode = (BTMode*)mode;
               btmode->enableConnection();
            }
        }else if(longclicktime == 5000){
            //do clear BT config
        }
        break;
    }

    case KEY_CTL_WIFI:
        mControlor->doControlNetwork();
        break;
    default:
        ;
    }
}

}/*namespace Parrot*/