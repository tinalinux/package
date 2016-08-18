#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>

#include "parrot_common.h"
#include "SocketServerWatcher.h"

namespace Parrot{

SocketServerWatcher::SocketServerWatcher(ref_EventWatcher watcher, ref_HandlerBase base):
    Handler(base)
{
    mWatcher = watcher;
    mServerfd = -1;
    mSocketListener = NULL;
    mPort = -1;
}

SocketServerWatcher::~SocketServerWatcher()
{

}

int SocketServerWatcher::startServer(int port)
{
    struct sockaddr_in addr;
    int addr_len = sizeof(addr);

    mServerfd = socket(PF_INET, SOCK_STREAM, 0);
    if (mServerfd < 0) {
        ploge("socket error\n");
        return FAILURE;
    }

    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    // bind
    if (bind(mServerfd, (struct sockaddr*) &addr, sizeof(addr)) != 0) {
        ploge("bind error\n");
        return FAILURE;
    }
    // listen
    if (listen(mServerfd, 5) < 0) {
        ploge("listen error\n");
        return FAILURE;
    }
    // set mServerfd reuseful
    int bReuseaddr = 1;
    if (setsockopt(mServerfd, SOL_SOCKET, SO_REUSEADDR, (const char*) &bReuseaddr, sizeof(bReuseaddr)) != 0) {
        ploge("setsockopt error in reuseaddr[%d]\n", mServerfd);
        return FAILURE;
    }
    mPort = port;
    mWatcher->addIOEventWatcher(mServerfd, this, EventWatcher::IO_ACCEPT, NULL);
    return SUCCESS;
}

int SocketServerWatcher::startServer(const char *unix_domain)
{

}

int SocketServerWatcher::stopServer()
{
    if(mServerfd != -1){
        disconnectAllClient();
        mWatcher->deleteIOEventWatcher(mServerfd);
        shutdown(mServerfd, SHUT_RDWR);
        mServerfd = -1;
    }
    return SUCCESS;
}

int SocketServerWatcher::disconnectClient(int client_fd)
{
    mWatcher->deleteIOEventWatcher(client_fd);
    shutdown(client_fd, SHUT_RDWR);
    locker::autolock l(mClientfdListLock);
    list<int>::iterator iter;
    for(iter = mClientfdList.begin(); iter != mClientfdList.end(); iter++){
        if(*iter == client_fd){
            mClientfdList.erase(iter);
            break;
        }
    }
    return SUCCESS;
}

int SocketServerWatcher::disconnectAllClient()
{
    locker::autolock l(mClientfdListLock);
    list<int>::iterator iter;
    for(iter = mClientfdList.begin(); iter != mClientfdList.end(); iter++){
        mWatcher->deleteIOEventWatcher(*iter);
        shutdown(*iter, SHUT_RDWR);
        mClientfdList.erase(iter);
    }
    return SUCCESS;
}

int SocketServerWatcher::sendtoClient(int client_fd, const char *buf, int len)
{
    int ret = send(client_fd, buf, len, 0);
    if(ret != len){
        ploge("sendtoClient failed! (%s)",strerror(errno));
    }
    return ret;
}

int SocketServerWatcher::sendtoAllClient(const char *buf, int len)
{
    list<int>::iterator iter;
    for(iter = mClientfdList.begin(); iter != mClientfdList.end(); iter++){
        if( sendtoClient(*iter, buf, len) < 0)
            return FAILURE;
    }
    return SUCCESS;
}

int SocketServerWatcher::setListener(SocketListener *l)
{
    mSocketListener = l;
    return SUCCESS;
}

void SocketServerWatcher::handleMessage(Message *msg)
{
    int client_fd = *(int*)msg->data->data;
    char *data = (char*)msg->data->data + 4;
    int len = msg->data->len - 4;
    switch(msg->what){
    case SOCKET_DISCONNECTED:{
        disconnectClient(client_fd);
        if(mSocketListener != NULL)
            mSocketListener->onSocketServerRead(client_fd, NULL, 0);
        break;
    }
    case SOCKET_DATA:
        if(mSocketListener != NULL){
            mSocketListener->onSocketServerRead(client_fd, (char*)data, len);
        }
        break;
    }
}


void SocketServerWatcher::onEvent(EventWatcher::tEventType type, int index, void *data, int len, void* args)
{
    switch(type){
    case EventWatcher::EVENT_IO_DATA:{
        Message msg;
        msg.what = SOCKET_DATA;
        msg.setData((void*)&index, sizeof(int));
        msg.setData(data, len);
        sendMessage(msg);
        break;
    }
    case EventWatcher::EVENT_IO_DISCONNECTED:{
        Message msg;
        msg.what = SOCKET_DISCONNECTED;
        msg.setData((void*)&index, sizeof(int));
        sendMessage(msg);
        break;
    }
    case EventWatcher::EVENT_IO_ACCEPT:{

        int client_fd = (int)data;
        mWatcher->addIOEventWatcher(client_fd, this, EventWatcher::IO_READ, NULL);
        {
            locker::autolock l(mClientfdListLock);
            mClientfdList.push_back(client_fd);
        }
        if(mSocketListener != NULL)
            mSocketListener->onSocketServerAccept(client_fd);
        break;
    }
    }

}

int SocketServerWatcher::exsitClient()
{
    return mClientfdList.size();
}

bool SocketServerWatcher::isServerWork()
{
    return mServerfd != -1?true:false;
}

int SocketServerWatcher::getPort()
{
    return mPort;
}

}