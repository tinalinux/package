#ifndef __HANDLER_BASE_H__
#define __HANDLER_BASE_H__

#include "threadpool.h"
#include "EventWatcher.h"
#include "T_Reference.h"

namespace Parrot{
class HandlerBaseMessage
{
public:
    virtual void process(){};

};
class HandlerBase : public EventWatcher::WatcherListener
{
public:
    static const int ThreadNumber = 2;
    static const int MaxRequests = 1000;
    HandlerBase(ref_EventWatcher watcher,int thread_number = ThreadNumber, int max_requests = MaxRequests);
    virtual ~HandlerBase();

    int sendMessage(HandlerBaseMessage *msg,int sec,int usec);

protected:
    virtual void onEvent(EventWatcher::tEventType type, int index, void *data, int len, void* args);
private:
    ref_EventWatcher m_watcher;
    threadpool<HandlerBaseMessage> *m_pool;
    //std::map<int,EventWatcher::WatcherListener*> m_owner_map;
};
typedef T_Reference<HandlerBase> ref_HandlerBase;
}/*namespace Parrot*/

#endif /*__HANDLER_BASE_H__*/