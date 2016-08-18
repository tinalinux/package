#ifndef __ACTION_KEEPER_H__
#define __ACTION_KEEPER_H__

#include "locker.h"
#include "Handler.h"

namespace Parrot{

class ActionKeeper : public Handler
{
public:
    ActionKeeper(ref_HandlerBase base);
    ~ActionKeeper();

    class KeeperAction
    {
    public:
        virtual int onKeeperAction(ActionKeeper *keeper, int key, int *interval){};
    };

    int init(int key, KeeperAction *action, int interval/*ms*/);
    int release();

    int action();
    int action(int interval);
    int stop();

    virtual void handleMessage(Message *msg);

private:
    //locker mLocker;
    int mIndex;
    int mKey;
    KeeperAction *mAction;
    int mInterval;
    bool mKeepFlag;
    Message mCurMessage;

};
}/*namespace Parrot*/

#endif /*__ACTION_KEEPER_H__*/