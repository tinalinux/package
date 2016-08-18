#include <linux/input.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#include "KeyEventWatcher.h"
#include "parrot_common.h"

using namespace std;

namespace Parrot {

KeyEventWatcher::KeyEventWatcher(ref_EventWatcher watcher, ref_HandlerBase base, const char *name, KeyEventListener* listener):
    Handler(base)
{
    mWatcher = watcher;
    mListener = listener;
    mInputEventWatcher = new InputEventWatcher(watcher, name, this);
}

KeyEventWatcher::~KeyEventWatcher()
{
    delete mInputEventWatcher;
}

int KeyEventWatcher::addLongClickTime(int code, int longclicktime)
{
    tLongClick longclick;
    longclick.code = code;
    longclick.longclicktime = longclicktime;
    mLongClickTimeList.push_back(longclick);

    return SUCCESS;
}

void KeyEventWatcher::handleMessage(Message *msg)
{
    switch(msg->what) {
    case MSG_KEY_DOWN:{
        tKeyMessage *info = (tKeyMessage *)msg->getData();
        mListener->onKeyDown(info->fd,info->code);

        break;
    }
    case MSG_KEY_UP:{
        tKeyMessage *info = (tKeyMessage *)msg->getData();
        mListener->onKeyUp(info->fd,info->code);
        break;
    }
    case MSG_KEY_LONGCLICK:{
        tKeyMessage *info = (tKeyMessage *)msg->getData();
        mListener->onkeyLongClick(info->fd,info->code,info->longclicktime);
        break;
    }
    default:
        break;
    }
}

void KeyEventWatcher::sendKeyMessage(int what, int fd ,int keycode, int longclicktime)
{
    tKeyMessage info;
    info.code = keycode;
    info.fd = fd;
    info.longclicktime = longclicktime;
    Message msg;
    msg.what = what;
    msg.setData(&info, sizeof(tKeyMessage));
    sendMessage(msg);
}

void KeyEventWatcher::onEvent(EventWatcher::tEventType type,int fd,void *data, int len, void* args)
{

    if(type == EventWatcher::EVENT_IO_DATA){
        //input event
        struct input_event *buff = (struct input_event*)data;

        if(buff->code > 0 && buff->value == 1){

            list<tLongClick>::iterator it;
            for(it = mLongClickTimeList.begin(); it != mLongClickTimeList.end();it++)  {
                if((*it).code == buff->code){
                    int index = mWatcher->addTimerEventWatcher(0, (*it).longclicktime*1000, this, (void*)(*it).longclicktime);
                    mKeyTimerMap[index].status = LongClickStart;
                    mKeyTimerMap[index].code = buff->code;
                }
            }

            sendKeyMessage(MSG_KEY_DOWN, fd, buff->code, 0);
        }
        else if(buff->code > 0 && buff->value == 0){
            bool should_key_up = true;
            if(mLongClickTimeList.size() > 0){
                //默认从小到大排序 front 小
                map<int, tKeyStatus>::iterator it;

                for (it = mKeyTimerMap.begin(); it != mKeyTimerMap.end();){
                    if (it->second.code == buff->code){
                        if(it->second.status == LongClickEnd)
                            should_key_up = false;
                        mKeyTimerMap.erase(it++);
                    }else
                        it++;
                }

            }
            if(should_key_up)
                sendKeyMessage(MSG_KEY_UP, fd, buff->code, 0);

        }
    }
    else if(type == EventWatcher::EVENT_TIMER){
        //timer event
        map<int, tKeyStatus>::iterator it = mKeyTimerMap.find(fd);

        if(it != mKeyTimerMap.end()){
            sendKeyMessage(MSG_KEY_LONGCLICK, fd, it->second.code, (int)args);
            it->second.status = LongClickEnd;
            //mKeyTimerMap.erase(it);
        }
    }

}

}/*namespace Parrot*/