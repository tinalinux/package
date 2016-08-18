#ifndef __KET_EVENT_WATCHER_H__
#define __KET_EVENT_WATCHER_H__

#include <linux/input.h>

#include "InputEventWatcher.h"
#include "EventWatcher.h"
#include "Handler.h"

#include <map>
#include <list>

namespace Parrot{

class KeyEventWatcher : public EventWatcher::WatcherListener, public Handler{
public:
    class KeyEventListener{
    public:
        virtual void onKeyDown(int fdindex, int keycode){};
        virtual void onKeyUp(int fdindex, int keycode){};
        virtual void onkeyLongClick(int fdindex, int keycode, int longclicktime){};
    };

    KeyEventWatcher(ref_EventWatcher watcher, ref_HandlerBase base, const char *name, KeyEventListener* listener);
    ~KeyEventWatcher();

    int addLongClickTime(int code, int longclicktime/*ms*/);

    virtual void handleMessage(Message *msg);

protected:
    void onEvent(EventWatcher::tEventType type,int index, void *data, int len, void* args);

private:
    enum _WhatMessage
    {
        MSG_KEY_DOWN = 0xfa,
        MSG_KEY_UP,
        MSG_KEY_LONGCLICK
    };

    typedef struct _KeyMessage
    {
        int code;
        int fd;
        int longclicktime;
    }tKeyMessage;

    typedef struct _LongClick
    {
        int code;
        int longclicktime;
    }tLongClick;

    typedef struct _KeyStatus
    {
        int status;
        int code;
    }tKeyStatus;

    enum LongClickAction
    {
        LongClickStart,
        LongClickEnd
    };
    void sendKeyMessage(int what, int fd ,int keycode, int longclicktime);
private:
    ref_EventWatcher mWatcher;
    KeyEventListener* mListener;
    list<tLongClick> mLongClickTimeList;

    InputEventWatcher *mInputEventWatcher;
    map<int,struct input_event> mKeyStatusMap;

    map<int,tKeyStatus> mKeyTimerMap;
};
}/*namespace Parrot*/
#endif /*__KET_EVENT_WATCHER_H__*/