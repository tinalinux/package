#ifndef __SOCKET_CLIENT_WATCHER_H__
#define __SOCKET_CLIENT_WATCHER_H__

#include <thread.h>
#include "EventWatcher.h"
#include "Handler.h"

namespace Parrot{

class SocketClientWatcher : public EventWatcher::WatcherListener, public Handler, public Thread
{
public:
    class SocketListener
    {
    public:
        virtual int onSocketClientRead(int socketfd, char *buf, int len) = 0;
    };
    SocketClientWatcher(ref_EventWatcher watcher, ref_HandlerBase base);
    ~SocketClientWatcher(){};

    int connectServer(const char *ip, int port);
    int connectServer(const char *unix_domain);
    int disconnectServer();
    int sendtoServer(const char *buf, int len);
    int setListener(SocketListener *l);
    int getFd();

    typedef enum SocketCmd
    {
        SOCKET_DISCONNECTED = 0xf0,
        SOCKET_DATA,
    }tSocketCmd;

    virtual void handleMessage(Message *msg);
protected:
    //todo TEST
    void onEvent(EventWatcher::tEventType type,int index,void *data, int len, void* args);
    int loop();

private:
    ref_EventWatcher mWatcher;

    SocketListener *mSocketListener;
    int mSocketfd;
};

}/*namespace Parrot*/
#endif /*__SOCKET_CLIENT_WATCHER_H__*/