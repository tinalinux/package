#ifndef __BT_MODE_H__
#define __BT_MODE_H__

#include <bluetooth_socket.h>

#include "Mode.h"
#include "EventWatcher.h"
#include "HandlerBase.h"
#include "StateManager.h"

namespace Parrot{

class BTMode : public Mode
{
public:
    BTMode(ref_EventWatcher watcher, ref_HandlerBase handlerbase);
    ~BTMode();

    static const char *CONFIG_FILE;

    int onCreate();
    int onStart();
    int onStop();
    int onDestory();

    int play();
    int pause();
    int next();
    int prev();
    int switchplay();

    void onBTEvent(BT_EVENT event);

    int enableConnection();
private:
    enum _btstatus
    {

    };
private:
    char mConfigFile[50];
    c_bt *mCBT;
    StateManager mBTStatus;

};

}
#endif