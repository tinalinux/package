#ifndef __EVENT_WATCHER_H__
#define __EVENT_WATCHER_H__

#include <locker.h>
#include <thread.h>
#include <map>
#include <string>
#include <stdio.h>

#include <ev++.h>

#include "T_Reference.h"

//#define _LIBEV_CPP_
using namespace std;

namespace Parrot{
class EventWatcher : public Thread{
public:
    EventWatcher();
    virtual ~EventWatcher();
    enum LoopStatus
    {
        LOOP_INIT = 0xaa,
        LOOP_DESTROY,
    };

    typedef enum Eventtype
    {
        EVENT_START_IO,
        EVENT_STOP_IO,
        EVENT_IO_ACCEPT,
        EVENT_IO_DATA,
        EVENT_IO_DISCONNECTED,
        EVENT_TIMER,
    }tEventType;

    typedef enum _IOEventWatcherType
    {
        IO_READ,
        IO_ACCEPT
    }tIOEventWatcherType;

    typedef struct _EventWatcher
    {
        tEventType event;
        int fd;
        tIOEventWatcherType io_type;
        int timer_index;
        int sec;
        int usec;
        void *listener;
        void *usrdata;
    }tEventWatcher;



    class WatcherListener{
    public:
        virtual void onEvent(tEventType type, int index, void *data, int len, void* args) = 0;
    };

    class IOEventWatcher{

    public:
        ~IOEventWatcher(){printf("~IOEventWatcher\n");};
        void start(int fd, EventWatcher *base, WatcherListener* listener, void *usrdata = NULL);
        void stop();
#ifdef _LIBEV_CPP_
        ev::io io;
#else
        ev_io io;
        struct ev_loop *main_loop;
#endif
        WatcherListener* listener;
        int io_index;
        tIOEventWatcherType type;
        bool should_stop;
        void *usr_data;
    };

    class TimerEventWatcher{
    public:
        TimerEventWatcher(int *count = NULL, locker *count_locker = NULL);
        ~TimerEventWatcher();
        void start(int sec,int usec,int timer_index, EventWatcher *base,WatcherListener* listener,void *usrdata = NULL);
        void stop();
        ev::timer timer;
        WatcherListener* listener;
        int timer_index;
        bool should_stop;
        void *usr_data;

        int *count;
        locker *count_locker;
    };

    class StatEventWatcher //TODO: test stop
    {
    public:
        void start(const char *path,int stat_index, EventWatcher *base,WatcherListener* listener,void *usrdata = NULL);
        void stop();
        ev::stat stat;
        WatcherListener* listener;
        int stat_index;
        void *usr_data;

    };
    int startWatcher();
    int stopWatcher();

    /* return inputevent watcher id */
    int addIOEventWatcher(int fd, WatcherListener* listener, tIOEventWatcherType type, void *usrdata = NULL);
    int deleteIOEventWatcher(int io_id);

    /* return timerevent watcher id */
    int addTimerEventWatcher(int sec,int usec,WatcherListener* listener,void *usrdata = NULL);
    int deleteTimerEventWatcher(int timer_id){};

    /* return timerevent watcher id */
    int addStatEventWatcher(const char *path, WatcherListener* listener, void *usrdata = NULL);
    int deleteStatEventWatcher(int timer_id){};

    struct ev_loop *main_loop;
private:
    virtual int loop();

    void debug_event_map();
    void debug_timer_map();

    long int get_cur_time();

    /* return inputevent watcher id */
    int add_io_eventwatcher(int fd, WatcherListener* listener, tIOEventWatcherType type, void *usrdata = NULL);
    int delete_io_eventwatcher(int io_id);

    /* return timerevent watcher id */
    int add_timer_eventwatcher(int timer_index, int sec, int usec, WatcherListener* listener, void *usrdata = NULL);

public:
    // libev++
#ifdef _LIBEV_CPP_
    void ev_io_cb(ev::io &w, int revents);
#else
    void ev_io_cb(struct ev_loop *loop, struct ev_io *watcher, int revents);
#endif

    void ev_timer_cb(ev::timer &w, int revents);
    void ev_stat_cb(ev::stat &w, int revents);

private:
    void _ev_io_cb(int fd, void *data);
private:
    int mHandlerfd[2];
    int mEvLoopHandIO;
    map<int,IOEventWatcher*> mEventMap;
    map<int,TimerEventWatcher*> mTimerMap;

    locker mEventMapLocker;

    int mTimerCount;
    locker mTimerCountlocker;

    cond mLoopCond;

};/*class EventWatcher*/
typedef T_Reference<EventWatcher> ref_EventWatcher;
}/*namespace Parrot*/
#endif /*__EVENT_WATCHER_H__*/
