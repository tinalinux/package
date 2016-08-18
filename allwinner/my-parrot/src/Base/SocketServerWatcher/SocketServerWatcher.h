#ifndef __SOCKET_SERVER_WATCHER_H__
#define __SOCKET_SERVER_WATCHER_H__

#include <list>

#include "EventWatcher.h"
#include "Handler.h"
#include "locker.h"

namespace Parrot{

class SocketServerWatcher : public EventWatcher::WatcherListener, public Handler
{
public:
    SocketServerWatcher(ref_EventWatcher watcher, ref_HandlerBase base);
    ~SocketServerWatcher();

    typedef enum SocketCmd
    {
        SOCKET_DISCONNECTED = 0xf0,
        SOCKET_DATA,
    }tSocketCmd;

    class SocketListener
    {
    public:
        virtual int onSocketServerAccept(int client_fd) {};
        virtual int onSocketServerRead(int client_fd, char *buf, int len) {};
    };

    int startServer(int port);
    int startServer(const char *unix_domain);

    int stopServer();

    int disconnectClient(int client_fd);
    int disconnectAllClient();

    int sendtoClient(int client_fd, const char *buf, int len);
    int sendtoAllClient(const char *buf, int len);

    int setListener(SocketListener *l);

    bool isServerWork();
    int exsitClient();
    int getPort();

    virtual void handleMessage(Message *msg);

protected:
    void onEvent(EventWatcher::tEventType type, int index, void *data, int len, void* args);


private:
    ref_EventWatcher mWatcher;
    SocketListener *mSocketListener;
    int mServerfd;
    int mPort;
    list<int> mClientfdList;
    locker mClientfdListLock;
};
}/*namespace Parrot*/
#endif /*__SOCKET_SERVER_WATCHER_H__*/