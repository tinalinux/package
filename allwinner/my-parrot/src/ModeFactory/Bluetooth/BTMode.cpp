#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "parrot_common.h"
#include "ConfigUtils.h"
#include "BTMode.h"
#include "UI.h"

namespace Parrot {

static Parrot::BTMode* cur = NULL;
void bt_event_f(BT_EVENT event)
{
    cur->onBTEvent(event);
}
static tStateString bt_state_string[] =
{
  { BT_AVK_CONNECTED_EVT, "BT_AVK_CONNECTED_EVT" },
  { BT_AVK_DISCONNECTED_EVT, "BT_AVK_DISCONNECTED_EVT" },
  { BT_AVK_START_EVT, "BT_AVK_START_EVT" },
  { BT_AVK_STOP_EVT, "BT_AVK_STOP_EVT" },
};

//const char* BTMode::CONFIG_FILE = "/etc/bluetooth/mac_config";

BTMode::BTMode(ref_EventWatcher watcher, ref_HandlerBase handlerbase):
    Mode(watcher,handlerbase),
    mBTStatus(10,"bt",bt_state_string,sizeof(bt_state_string)/sizeof(tStateString))
{
    if(cur != NULL){
        ploge("BTMode cannot be construct!");
        throw;
    }
    mMode = MODE_BT;

    cur = this;

}

BTMode::~BTMode(){}

int BTMode::onCreate()
{
    plogd("BT Mode onCreate!");
    //fetch WIFI MAC to BT config file

    mCBT = new c_bt();
    CHECK_POINTER(mCBT);
    //bt on
    mCBT->set_callback(bt_event_f);
    mCBT->bt_off();
    mCBT->bt_on(ConfigUtils::getBTMacConfig());
    mCBT->set_bt_name(ConfigUtils::getBTNameConfig());

    //
    mCBT->set_dev_discoverable(0);
    mCBT->set_dev_connectable(0);

    Mode::onCreate();
    return SUCCESS;
}

int BTMode::onStart()
{
    plogd("BT Mode onStart!");
    UI::BluetoothModeStart(true);

    mCBT->set_dev_connectable(1);

    if( mCBT->connect_auto() < 0) //block call
        UI::BluetoothConnectfailed(true);

    Mode::onStart();
    return SUCCESS;
}

int BTMode::onStop()
{
    plogd("BT Mode onStop!");

    mCBT->set_dev_discoverable(0);
    mCBT->set_dev_connectable(0);
    mCBT->disconnect();

    plogd("BT Mode onStop end!");
    Mode::onStop();
    return SUCCESS;

}

int BTMode::onDestory()
{
    plogd("BT Mode onDestory!");
    mCBT->bt_off();
    delete mCBT;

    Mode::onDestory();
    return SUCCESS;
}

int BTMode::enableConnection()
{
    UI::BluetoothParing(true);
    mCBT->set_dev_discoverable(1);
    mCBT->set_dev_connectable(1);
    mCBT->disconnect();
    return SUCCESS;
}

int BTMode::play()
{
    if(mBTStatus.get(StateManager::CUR) == BT_AVK_START_EVT){
        return FAILURE;
    }
    return mCBT->avk_play();
}

int BTMode::pause()
{
    if(mBTStatus.get(StateManager::CUR) == BT_AVK_STOP_EVT){
        return FAILURE;
    }
    return mCBT->avk_pause();
}

int BTMode::next()
{
    if(mBTStatus.get(StateManager::CUR) == BT_AVK_DISCONNECTED_EVT){
        return FAILURE;
    }
    return mCBT->avk_next();
}

int BTMode::prev()
{
    if(mBTStatus.get(StateManager::CUR) == BT_AVK_DISCONNECTED_EVT){
        return FAILURE;
    }
    return mCBT->avk_previous();
}

int BTMode::switchplay()
{
    if(mBTStatus.get(StateManager::CUR) == BT_AVK_STOP_EVT){
        return play();
    }else
        return pause();
}

void BTMode::onBTEvent(BT_EVENT event)
{
    switch(event) {
    case BT_AVK_CONNECTED_EVT: {
        mBTStatus.update(BT_AVK_CONNECTED_EVT);
        UI::BluetoothConnected(true);
        break;
    }

    case BT_AVK_DISCONNECTED_EVT: {
        mBTStatus.update(BT_AVK_DISCONNECTED_EVT);
        UI::CommonSay(true);
        break;
    }

    case BT_AVK_START_EVT: {
        mBTStatus.update(BT_AVK_START_EVT);
        break;
    }

    case BT_AVK_STOP_EVT: {
        mBTStatus.update(BT_AVK_STOP_EVT);

        break;
    }

    default: ;
    }
}
}
