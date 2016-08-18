#include "parrot_common.h"
#include "ActionKeeper.h"

namespace Parrot{

ActionKeeper::ActionKeeper(ref_HandlerBase base):
    Handler(base)
{
    mKeepFlag = false;
}

ActionKeeper::~ActionKeeper()
{

}

int ActionKeeper::init(int key, KeeperAction *action, int interval/*ms*/)
{
    mIndex = 0;
    mKeepFlag = false;
    mKey = key;
    mAction = action;
    mInterval = interval;

    return SUCCESS;
}

int ActionKeeper::release()
{
    mKeepFlag = false;
    mAction = NULL;
}

int ActionKeeper::action()
{
    //locker::autolock l(mLocker);
    mIndex = (mIndex % 10000) + 1;
    mKeepFlag = true;

    sendMessage(mIndex);

    return SUCCESS;
}

int ActionKeeper::action(int interval)
{
    mInterval = interval;
    return action();
}

int ActionKeeper::stop()
{
    mKeepFlag = false;
}

void ActionKeeper::handleMessage(Message *msg)
{
    int index;
    {
        //locker::autolock l(mLocker);
        index = mIndex;
    }
    //ploge("handleMessage len: %d mindex %d", msg->what, mIndex);
    if(msg->what == index){
        if(mAction != NULL && mKeepFlag == true){
            mAction->onKeeperAction(this, mKey, &mInterval);
        }
        if(mKeepFlag)
            sendMessageDelayed(msg->what, 0, mInterval*1000);

    }else
        ploge("this msg is no use!!! key: %d mindex %d", mKey, mIndex);
}

}