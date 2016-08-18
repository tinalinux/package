#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stddef.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include "parrot_common.h"
#include "SocketClientWatcher.h"

namespace Parrot{

SocketClientWatcher::SocketClientWatcher(ref_EventWatcher watcher, ref_HandlerBase base):
    mWatcher(watcher),
    Handler(base),
    mSocketfd(-1),
    mSocketListener(NULL)
{

}

int SocketClientWatcher::getFd()
{
    return mSocketfd;
}

int SocketClientWatcher::connectServer(const char *ip, int port)
{
    int ret;
    struct sockaddr_in s_add;
    mSocketfd = socket(AF_INET, SOCK_STREAM, 0);
    if(mSocketfd < 0) {
        ploge("cannot create socket (%s)",strerror(errno));
        return FAILURE;
    }

    bzero(&s_add,sizeof(struct sockaddr_in));
    s_add.sin_family = AF_INET;
    s_add.sin_addr.s_addr = inet_addr(ip);
    s_add.sin_port = htons(port);
    plogd("connect %s:%d, s_addr = %#x ,port : %#x\r\n",ip,port,s_add.sin_addr.s_addr,s_add.sin_port);

    //int ireuseadd_on = 1;//支持端口复用
    //setsockopt(mSocketfd, SOL_SOCKET, SO_REUSEADDR, &ireuseadd_on, sizeof(ireuseadd_on));

    //unsigned long ul = 1;
    //ioctl(mSocketfd, FIONBIO, &ul); //设置为非阻塞模式

    int keepalive = 1;
    int keepidle = 5;
    int keepinterval = 3;
    int keepcount = 2;
    if(setsockopt(mSocketfd,SOL_SOCKET, SO_KEEPALIVE,   &keepalive,     sizeof(keepalive))<0)    return -3;
    if(setsockopt(mSocketfd,SOL_TCP,    TCP_KEEPIDLE,   &keepidle,      sizeof(keepidle))<0)     return -4;
    if(setsockopt(mSocketfd,SOL_TCP,    TCP_KEEPINTVL,  &keepinterval,  sizeof(keepinterval))<0) return -5;
    if(setsockopt(mSocketfd,SOL_TCP,    TCP_KEEPCNT,    &keepcount,     sizeof(keepcount))<0)    return -6;


    ret = connect(mSocketfd, (struct sockaddr *)(&s_add), sizeof(struct sockaddr));

    if(ret == -1) {
        ploge("cannot connect to the server %s:%d (%s)",ip, port,strerror(errno));
        close(mSocketfd);
        mSocketfd = -1;
        return FAILURE;
    }

    //test
    plogd("connect %s %d success, fd: %d",ip, port, mSocketfd);
    mWatcher->addIOEventWatcher(mSocketfd, this, EventWatcher::IO_READ, NULL);
    //run();
    return SUCCESS;

}
int SocketClientWatcher::connectServer(const char *unix_domain)
{
    int ret;
    struct sockaddr_un srv_addr;
    //creat unix socket
    mSocketfd = socket(PF_UNIX, SOCK_STREAM, 0);
    if(mSocketfd < 0) {
        ploge("cannot create socket (%s)",strerror(errno));
        return FAILURE;
    }
    srv_addr.sun_family=AF_UNIX;

    strcpy(srv_addr.sun_path+1,unix_domain);
    srv_addr.sun_path[0] = '\0';
    int size = offsetof(struct sockaddr_un,sun_path) + strlen(unix_domain)+1;

    //connect server
    ret = connect(mSocketfd,(struct sockaddr*)&srv_addr,size);
    if(ret == -1) {
        ploge("cannot connect to the server %s: %s",unix_domain, strerror(errno));
        close(mSocketfd);
        mSocketfd = -1;
        return FAILURE;
    }

    //run();
    return SUCCESS;
}
int SocketClientWatcher::disconnectServer()
{
    if(mSocketfd != -1){
        mWatcher->deleteIOEventWatcher(mSocketfd);
        shutdown(mSocketfd, SHUT_RDWR);
    }

    //plogd("close socket fd");
    //shutdown(mSocketfd, SHUT_RDWR);
    mSocketfd = -1;
}
int SocketClientWatcher::sendtoServer(const char *buf, int len)
{
    int ret = send(mSocketfd, buf, len, 0);
    if(ret != len){
        ploge("sendtoServer failed! (%s)",strerror(errno));
    }
    return ret;
}
int SocketClientWatcher::setListener(SocketListener *l)
{
    mSocketListener = l;
    return SUCCESS;
}

void SocketClientWatcher::handleMessage(Message *msg)
{
    switch(msg->what){
    case SOCKET_DISCONNECTED:
        mWatcher->deleteIOEventWatcher(mSocketfd);
        if(mSocketListener != NULL)
            mSocketListener->onSocketClientRead(mSocketfd, NULL, 0);
        break;
    case SOCKET_DATA:
        if(mSocketListener != NULL){
            mSocketListener->onSocketClientRead(mSocketfd, (char*)msg->data->data, msg->data->len);
        }
        break;
    }
}

void SocketClientWatcher::onEvent(EventWatcher::tEventType type,int index, void *data, int len,void* args)
{
    //todo:  test
    switch(type){
    case EventWatcher::EVENT_IO_DATA:{
        Message msg;
        msg.what = SOCKET_DATA;
        msg.setData(data, len);
        sendMessage(msg);
        break;
    }
    case EventWatcher::EVENT_IO_DISCONNECTED:{
        sendMessage(SOCKET_DISCONNECTED);
    }
    break;
    }
}

int SocketClientWatcher::loop()
{
    //todo: exit this loop when blocking in recv ??
    char buf[10000]; // buf: 10k
    bzero(buf, sizeof(buf));
    int recv_bytes = recv(mSocketfd,buf,sizeof(buf),0);
    if ( recv_bytes == -1 ) {
        if( errno == EAGAIN || errno == EWOULDBLOCK ){
            return Thread::THREAD_CONTINUE;
        }
        ploge("recv failed(%s)\n",strerror(errno));

        return Thread::THREAD_EXIT;
        //need callback event?
    }

    if (recv_bytes == 0) {
        plogd("socekt client disconnected.");
        sendMessage(SOCKET_DISCONNECTED);
        return Thread::THREAD_EXIT;
    }
    else {
        plogd("handler Message: %s",(char*)buf);
        Message msg;
        msg.what = SOCKET_DATA;
        msg.setData(buf, recv_bytes);
        sendMessage(msg);
    }
    return Thread::THREAD_CONTINUE;
}

}