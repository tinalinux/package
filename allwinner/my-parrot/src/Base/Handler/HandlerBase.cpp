#include "HandlerBase.h"

namespace Parrot{

HandlerBase::HandlerBase(ref_EventWatcher watcher,int thread_number, int max_requests)
{
    m_watcher = watcher;
    m_pool = new threadpool<HandlerBaseMessage>(thread_number,max_requests);
}

HandlerBase::~HandlerBase()
{
    //todo
    delete m_pool;
}

int HandlerBase::sendMessage(HandlerBaseMessage *msg,int sec,int usec)
{
    if(sec > 0 || usec > 0){
        m_watcher->addTimerEventWatcher(sec, usec, this, (void*)msg);
    }
    else
        m_pool->append(msg);
}

void HandlerBase::onEvent(EventWatcher::tEventType type, int index, void *data, int len, void* args)
{
    m_pool->append((HandlerBaseMessage *)args);
}

}