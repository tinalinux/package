#define TAG "EventWatcher"
#include <tina_log.h>

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <dirent.h>
#include <dlfcn.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <signal.h>
#include <arpa/inet.h>
#include <stddef.h>

#include <ev.h>

#include <iostream>

#include "EventWatcher.h"
#include "parrot_common.h"
#include "SocketClientWatcher.h"

#define PRESS_SYSFS_PATH   "/sys/class/input"

static int g_timer_id = 0;
#ifndef _LIBEV_CPP_
void io_callback(struct ev_loop *loop, struct ev_io *watcher, int revents) {
    if (EV_ERROR & revents) {
        ploge("error event");
        return;
    }
    //plogd("io revents: %d data: %p",revents,watcher->data);
    Parrot::EventWatcher *main_class = (Parrot::EventWatcher *)ev_userdata(loop);
    main_class->ev_io_cb(loop, watcher, revents);
}

void timer_callback(struct ev_loop *loop, struct ev_timer *watcher, int revents) {
    if (EV_ERROR & revents) {
        ploge("error event");
        return;
    }
    plogd("io revents: %d data: %p",revents,watcher->data);
    Parrot::EventWatcher *main_class = (Parrot::EventWatcher *)ev_userdata(loop);
    //main_class->ev_io_cb(loop, watcher, revents);
}

void stae_callback(struct ev_loop *loop, struct ev_stat *watcher, int revents) {
    if (EV_ERROR & revents) {
        ploge("error event");
        return;
    }
    //plogd("io revents: %d data: %p",revents,watcher->data);
    Parrot::EventWatcher *main_class = (Parrot::EventWatcher *)ev_userdata(loop);
    //main_class->ev_io_cb(loop, watcher, revents);
}
#endif
namespace Parrot{

EventWatcher::EventWatcher():mTimerCount(0)
{
    //main_loop = ev_loop_new (EVBACKEND_EPOLL | EVFLAG_NOENV);
    main_loop = ev_default_loop(0);

    if (!main_loop){
        perror ("no epoll found here, maybe it hides under your chair");
        throw;
    }
    ev_set_userdata(main_loop,this);
}

EventWatcher::~EventWatcher()
{
    //ev_loop_destory(main_loop); //TODO
}

int EventWatcher::startWatcher()
{

    if( socketpair(AF_UNIX, SOCK_STREAM, 0, mHandlerfd) == -1 ){
        ploge("create unnamed socket pair failed:%s\n",strerror(errno) );
        exit(-1);
    }

    add_io_eventwatcher(mHandlerfd[0], NULL, IO_READ);
    //mHandlerfd[0] read and wait
    //mHandlerfd[1] send

    int init_loop = LOOP_INIT;
    int init_timeout = 10000; // 10ms
    addTimerEventWatcher(0, init_timeout, NULL, (void*)&init_loop);

    run();

    //wait loop run
    mLoopCond.wait();
    usleep(init_timeout);

    plogd("startWatcher finished!");

    return SUCCESS;
}

int EventWatcher::stopWatcher()
{
    //TODO: after ev_run return, release all the fd
    //add a new timer to break the ev_run

    if(mEventMap.size() > 1 || mTimerCount != 0){
        ploge("stop failed: there are %d io watcher pending and \
                %d timer watcher pending in the loop",mEventMap.size()-1,mTimerCount);
        return FAILURE;
    }

    //deleteIOEventWatcher(mEvLoopHandIO);

    int destroy_loop = LOOP_DESTROY;
    //addTimerEventWatcher(0,111,NULL,&destroy_loop);// 100ms later break the loop

    mLoopCond.wait();
    plogd("stopWatcher finished!");
}

long int EventWatcher::get_cur_time()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec; //ms
}

int EventWatcher::addIOEventWatcher(int fd, WatcherListener* listener, tIOEventWatcherType type, void *usrdata)
{
    /*        tEventType event;
        int fd;
        int sec;
        int usec;
        void *listener;
        void *usrdata;     */
    tEventWatcher w;
    w.event = EVENT_START_IO;
    w.fd = fd;
    w.listener = (void*)listener;
    w.usrdata = usrdata;
    w.io_type = type;
    int ret = send(mHandlerfd[1], &w, sizeof(w), 0);
    if(ret == sizeof(w))
        return fd;
    return FAILURE;

    /*
    IOEventWatcher* watcher = new IOEventWatcher();
    {
        locker::autolock l(mEventMapLocker);
        mEventMap.insert(pair<int, IOEventWatcher*>(fd, watcher));
    }
    watcher->start(fd, this, event, usrdata);
    //addTimerEventWatcher(0,100,NULL);
    //debug_event_map();

    return fd;
    */
}

int EventWatcher::deleteIOEventWatcher(int io_id)
{
    //TODO
    //it seems to not need in my-parrot
    //should lock mEventMap??

    tEventWatcher w;
    w.event = EVENT_STOP_IO;
    w.fd = io_id;

    int ret = send(mHandlerfd[1], &w, sizeof(w), 0);
    return SUCCESS;
}

int EventWatcher::addTimerEventWatcher(int sec, int usec, WatcherListener *listener, void *usrdata)
{
    int index = g_timer_id++;
    g_timer_id %= 10000;

    tEventWatcher w;
    w.event = EVENT_TIMER;
    w.timer_index = index;
    w.sec = sec;
    w.usec = usec;

    w.listener = (void*)listener;
    w.usrdata = usrdata;
    int ret = send(mHandlerfd[1], &w, sizeof(w), 0);
    if(ret == sizeof(w))
        return index;
    return FAILURE;
}

#if 0
int EventWatcher::deleteTimerEventWatcher(int timer_id)
{

}
#endif
int EventWatcher::addStatEventWatcher(const char *path, WatcherListener* listener, void *usrdata)
{
    StatEventWatcher *watcher = new StatEventWatcher();
    watcher->start(path,0,this,listener,usrdata);
}
int EventWatcher::loop()
{
    plogd("loop......");
    mLoopCond.signal();

    ev_run(main_loop,0);

    plogd("loop end......");
    mLoopCond.signal();

    return Thread::THREAD_EXIT;
}

void EventWatcher::debug_event_map()
{
    map<int, IOEventWatcher*>::iterator iter;
    for (iter = mEventMap.begin(); iter != mEventMap.end(); iter++){
        plogd("EventMap: %d, %p",iter->first,iter->second);
    }
    if(mEventMap.size() == 0)
        plogd("EventMap is empty");
}
void EventWatcher::debug_timer_map()
{
    map<int, TimerEventWatcher*>::iterator iter;
    for (iter = mTimerMap.begin(); iter != mTimerMap.end(); iter++){
        plogd("TimerMap: %d, %p",iter->first,iter->second);
    }
    if(mTimerMap.size() == 0)
        plogd("TimerMap is empty");
}

int EventWatcher::add_io_eventwatcher(int fd, WatcherListener* listener, tIOEventWatcherType type, void *usrdata)
{
    IOEventWatcher* watcher = new IOEventWatcher();
    {
        locker::autolock l(mEventMapLocker);
        mEventMap.insert(pair<int, IOEventWatcher*>(fd, watcher));
    }
    watcher->type = type;
    watcher->start(fd, this, listener, usrdata);
    //addTimerEventWatcher(0,100,NULL);
    //debug_event_map();

    return fd;
}

int EventWatcher::delete_io_eventwatcher(int io_id)
{
    //TODO
    //it seems to not need in my-parrot
    //should lock mEventMap??

    if(mEventMap[io_id] != NULL){
        ploge("delete io watcher: %d", io_id);
        IOEventWatcher *watcher = mEventMap.find(io_id)->second;
        watcher->stop();
        mEventMap.erase(io_id);
        delete watcher;
    }
    //ev_break (main_loop);
    return 0;
}

int EventWatcher::add_timer_eventwatcher(int timer_index, int sec,int usec,WatcherListener* listener,void *usrdata)
{
    TimerEventWatcher *watcher = new TimerEventWatcher(&mTimerCount, &mTimerCountlocker);
    //TimerEventWatcher *watcher = new TimerEventWatcher(NULL, NULL);
    watcher->start(sec, usec, timer_index, this, listener, usrdata);
    return timer_index;
}

void EventWatcher::_ev_io_cb(int fd, void *data)
{
    IOEventWatcher *watcher = (IOEventWatcher*)(data);

    if(fd == mHandlerfd[0]){
        tEventWatcher w;
        read(fd, &w, sizeof(w));
        if(w.event == EVENT_START_IO)
            add_io_eventwatcher(w.fd, (WatcherListener*)w.listener, w.io_type, w.usrdata);
        else if(w.event == EVENT_TIMER)
            add_timer_eventwatcher(w.timer_index, w.sec, w.usec, (WatcherListener*)w.listener, w.usrdata);
        else if(w.event == EVENT_STOP_IO)
            delete_io_eventwatcher(w.fd);

        //
        return;
    }

    if(watcher->type == IO_ACCEPT){
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_sd;
        client_sd = accept(fd, (struct sockaddr*) &client_addr, &client_len);
        if (client_sd < 0) {
           ploge("accept error");
           return;
        }
        if(watcher->listener != NULL)
            watcher->listener->onEvent(EVENT_IO_ACCEPT, fd, (void*)client_sd, 1, watcher->usr_data);
        return;
    }

    if(watcher->type != IO_READ) return;

    char buf[IO_BUFF_SIZE];
    memset(buf, 0, sizeof(IO_BUFF_SIZE)); //?? //can not work

    int recv_bytes = read(fd,buf,sizeof(buf));

    //plogd("read form io bytes:%d ,msg: %s",recv_bytes,buf);

    if ( recv_bytes == -1 ) {
        if( errno == EAGAIN || errno == EWOULDBLOCK ){
            return;
        }
        ploge("recv failed(%s)\n",strerror(errno));
        if(watcher->listener != NULL)
            watcher->listener->onEvent(EVENT_IO_DISCONNECTED, fd, buf, recv_bytes, watcher->usr_data);
        watcher->stop();
        return;
    }

    if (recv_bytes == 0) {
        plogd("fd: %d, disconnected.",fd);
        if(watcher->listener != NULL)
            watcher->listener->onEvent(EVENT_IO_DISCONNECTED, fd, buf, recv_bytes, watcher->usr_data);
        watcher->stop();
        //callback
        return;
    }
    else {
        if(watcher->listener != NULL)
            watcher->listener->onEvent(EVENT_IO_DATA, fd, buf, recv_bytes, watcher->usr_data);
    }
}

#ifdef _LIBEV_CPP_
void EventWatcher::ev_io_cb(ev::io &w, int revents)
{
    int fd = w.fd;
    void *data = w.data;
    _ev_io_cb(fd, data);
}
#else
void EventWatcher::ev_io_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{
    int fd = w->fd;
    void *data = w->data;
    _ev_io_cb(fd, data);
}
#endif

void EventWatcher::ev_timer_cb(ev::timer &w, int revents)
{
    if (EV_ERROR & revents) {
        ploge("error event in read");
        return;
    }
    //todo
    TimerEventWatcher *watcher = (TimerEventWatcher*)(w.data);
    //plogd("timer revents: %d index : %d",revents,watcher->timer_index);
    if(watcher != NULL && watcher->listener != NULL)
        watcher->listener->onEvent(EVENT_TIMER,watcher->timer_index,NULL,0,watcher->usr_data);
    else{
        plogd("timer errno....");
    }
    watcher->stop();
    delete watcher;
    return;
}

void EventWatcher::ev_stat_cb(ev::stat &w, int revents)
{
    if (EV_ERROR & revents) {
        ploge("error event in read");
        return;
    }
     plogd("stat revents: %d data: %p",revents,w.data);
}
/* ----------------------------------------------------- */
void EventWatcher::IOEventWatcher::start(int fd, EventWatcher *base, WatcherListener* listener,void *usrdata)
{
    this->listener = listener;
    this->io_index = fd;
    this->usr_data = usrdata;

#ifdef _LIBEV_CPP_
    io.set(base->main_loop);
    io.set<EventWatcher, &EventWatcher::ev_io_cb>(base);
    io.data = this; //must after set<>()....   see: ev++.h(422~466) function set() _set()
    ploge("fd: %d start to watcher....", fd);
    io.start(fd, ev::READ);  // set + start in one call
#else
    main_loop = base->main_loop;
    io.data = this;
    ploge("fd: %d start to watcher....", fd);
    ev_io_init(&io, io_callback, fd, EV_READ);
    ev_io_start(main_loop, &io);
#endif
}
void EventWatcher::IOEventWatcher::stop()
{
    //TODO
#ifdef _LIBEV_CPP_
    io.stop();
#else
    ploge("stop io watcher!! %d ",io_index);
    ev_io_stop(main_loop, &io);

#endif
}

EventWatcher::TimerEventWatcher::TimerEventWatcher(int *count, locker *count_locker)
{
    this->count = count;
    this->count_locker = count_locker;
    if(this->count != NULL && this->count_locker != NULL){
        locker::autolock l(*count_locker);
        (*count)++;
        //printf("TimerEventWatcher count %d\n",*count);
    }

}

EventWatcher::TimerEventWatcher::~TimerEventWatcher()
{
    if(this->count != NULL && this->count_locker != NULL){
        locker::autolock l(*count_locker);
        (*count)--;
        //printf("~TimerEventWatcher %d\n",*count);
    }
}

void EventWatcher::TimerEventWatcher::start(int sec, int usec, int timer_index, EventWatcher *base, WatcherListener* listener,void *usrdata)
{
    this->listener = listener;
    this->timer_index = timer_index;
    this->usr_data = usrdata;

    timer.set(base->main_loop);
    timer.set<EventWatcher, &EventWatcher::ev_timer_cb>(base);
    timer.data = this; //must after set<>()....   see: ev++.h(422~466) function set _set

    float timeout = sec + (float)usec/1000000;
    //plogd("start timer %d: %f(s) later!",timer_index,timeout);
    timer.start(timeout,0);
}
void EventWatcher::TimerEventWatcher::stop()
{
    //TODO
    timer.stop();
}

void EventWatcher::StatEventWatcher::start(const char *path,int stat_index, EventWatcher *base,WatcherListener* listener,void *usrdata)
{
    this->listener = listener;
    this->stat_index = stat_index;
    this->usr_data = usrdata;

    stat.set(base->main_loop);
    stat.set<EventWatcher, &EventWatcher::ev_stat_cb>(base);
    stat.data = this; //must after set<>()....   see: ev++.h(422~466) function set _set

    stat.start(path, 0);
}
void EventWatcher::StatEventWatcher::stop()
{
    stat.stop();
}
} /*namesapce Parrot*/
