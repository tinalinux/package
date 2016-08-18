#ifndef __MODE_H__
#define __MODE_H__

#include <list>
#include <locker.h>

#include "parrot_common.h"
#include "EventWatcher.h"
#include "HandlerBase.h"
#include "Handler.h"
#include "PPlayer.h"

namespace Parrot{
static const char *ModeString[] = {"NONE", "BT", "LINEIN", "DLNA", "SPEECH"};
static const char *StatusString[] = {"none","creating","created","create_fail","starting","started","start_failed","stopping","stopped","stop_fail"};
class Mode : public Handler {
public:
    static const int STATUS_BASE = 0xf0;

    typedef enum _Cmd
    {
        CMD_MODE_NONE = 0,
        CMD_MODE_CREATE,
        CMD_MODE_START,
        CMD_MODE_STOP,
        CMD_MODE_DESTORY
    }tCMD;

    typedef enum _Status
    {
        STATUS_NONE                = 0,
        STATUS_CREATING            = 1,
        STATUS_CREATED             = 2,
        STATUS_CREATE_FAILED       = 3,

        STATUS_STARTING            = 4,
        STATUS_STARTED             = 5,
        STATUS_START_FAILED        = 6,

        STATUS_STOPPING            = 7,
        STATUS_STOPPED             = 8,
        STATUS_STOP_FAILED         = 9,

        STATUS_DESTORING           = 10,
        STATUS_DESTORYED           = 11,
        STATUS_DESTORY_FAILED      = 12
    }tStatus;

    typedef enum _Mode
    {
        MODE_NONE = 0,
        MODE_BT,
        MODE_LINEIN,
        MODE_DLNA,
        MODE_SPEECH,
        MODE_PHONE,
    }tMode;

    Mode(ref_EventWatcher watcher, ref_HandlerBase handlerbase):
        Handler(handlerbase),
        mMode(MODE_NONE),
        mStatus(STATUS_NONE),
        mWatcher(watcher),
        mHandlerBase(handlerbase),
        mShouldPrevCmd(CMD_MODE_NONE){};

    void handleMessage(Message *msg) {

        switch(msg->what) {
        case CMD_MODE_CREATE:{
            Create();
            break;
        }
        case CMD_MODE_START:{
            plogd("--->%s start cmd ",ModeString[mMode]);
            if(getStatus() == STATUS_STOPPING){
                //wait stop end, the start
                mNextCmdLocker.lock();
                mShouldPrevCmd = CMD_MODE_START;
                plogd("%s mode is stopping, waiting..... cmd: %d", ModeString[mMode],mShouldPrevCmd);
                mNextCmdLocker.unlock();
                break;
            }

            int ret = Start();
            plogd("%s mode status: %s cmd: %d",ModeString[mMode], StatusString[getStatus()], mShouldPrevCmd);
            mNextCmdLocker.lock();
            if(ret == SUCCESS && mShouldPrevCmd == CMD_MODE_STOP){
                plogd("do stop now .....");
                sendMessage(CMD_MODE_STOP);
                mShouldPrevCmd = CMD_MODE_NONE;
            }
            mNextCmdLocker.unlock();
            break;
        }
        case CMD_MODE_STOP:{
            plogd("--->%s stop cmd ",ModeString[mMode]);
            if(getStatus() == STATUS_STARTING){
                //wait stop end, the start
                mNextCmdLocker.lock();
                mShouldPrevCmd = CMD_MODE_STOP;
                plogd("%s mode is starting, waiting..... %d",ModeString[mMode],mShouldPrevCmd);
                mNextCmdLocker.unlock();
                break;
            }
            int ret = Stop();
            mNextCmdLocker.lock();
            if(ret == SUCCESS && mShouldPrevCmd == CMD_MODE_START){
                plogd("do start now .....");
                sendMessage(CMD_MODE_START);
                mShouldPrevCmd = CMD_MODE_NONE;
            }
            mNextCmdLocker.unlock();
            break;
        }
        case CMD_MODE_DESTORY:{
            Destory();
            break;
        }
        default:
            ;
        }
    }

    int CreateAsync() {
        return sendMessage(CMD_MODE_CREATE);
    }
    int StartAsync() {
        return sendMessage(CMD_MODE_START);
    }
    int StopAsync() {
        return sendMessage(CMD_MODE_STOP);
    }
    int DestoryAsync() {
        return sendMessage(CMD_MODE_DESTORY);
    }

    int Create() {
        int ret = SUCCESS;
        if(getStatus() == STATUS_NONE || getStatus() == STATUS_DESTORYED || getStatus() == STATUS_CREATE_FAILED){
            setStatus(STATUS_CREATING);
            plogd("--------------------------------------------------------------->   %s Create", ModeString[mMode]);
            int ret = onCreate();
            plogd("--------------------------------------------------------------->   %s Create end",  ModeString[mMode]);
            if(ret == SUCCESS)
                setStatus(STATUS_CREATED);
            else
                setStatus(STATUS_CREATE_FAILED);
        }else{
            ploge("%s Mode Create failed, status: %s",ModeString[mMode], StatusString[getStatus()]);
        }

        return ret;
    };

    int Start(){
        int ret = SUCCESS;
        if(getStatus() == STATUS_CREATED || getStatus() == STATUS_STOPPED || getStatus() == STATUS_START_FAILED){
            setStatus(STATUS_STARTING);
            plogd("--------------------------------------------------------------->   %s Start",  ModeString[mMode]);
            int ret = onStart();
            plogd("--------------------------------------------------------------->   %s Start end",  ModeString[mMode]);
            if(ret == SUCCESS)
                setStatus(STATUS_STARTED);
            else
                setStatus(STATUS_START_FAILED);
        }else{
            ploge("%s Mode Start failed, status: %s", ModeString[mMode], StatusString[getStatus()]);
        }

        return ret;
    };

    int Stop(){
        int ret = FAILURE;
        if(getStatus() == STATUS_STARTED || getStatus() == STATUS_STOP_FAILED){
            setStatus(STATUS_STOPPING);
            plogd("--------------------------------------------------------------->   %s Stop",  ModeString[mMode]);
            ret = onStop();
            plogd("--------------------------------------------------------------->   %s Stop end",  ModeString[mMode]);
            if(ret == SUCCESS)
                setStatus(STATUS_STOPPED);
            else
                setStatus(STATUS_STOP_FAILED);
        }else{
            ploge("%s Mode Stop failed, status: %s", ModeString[mMode], StatusString[getStatus()]);
        }

        return ret;
    };

    int Destory(){
        int ret = FAILURE;
        if(getStatus() == STATUS_STOPPED || getStatus() == STATUS_CREATED || getStatus() == STATUS_DESTORY_FAILED){
            setStatus(STATUS_DESTORING);
            plogd("--------------------------------------------------------------->   %s Destory",  ModeString[mMode]);
            int ret = onDestory();
            plogd("--------------------------------------------------------------->   %s Destory end",  ModeString[mMode]);
            if(ret == SUCCESS)
                setStatus(STATUS_DESTORYED);
            else
                setStatus(STATUS_DESTORY_FAILED);
        }else{
            ploge("%s Mode Destory failed, status: %d", ModeString[mMode], getStatus());
        }

        return ret;
    };

    /*  模式初始化

        资源申请
        状态初始化
        启动后台不需要关闭的服务
    */
    virtual int onCreate(){};

    /*  模式使能，正常功能启用，休眠退出后使能模式

        服务连接
        线程开启
        需要考虑休眠进出问题
    */
    virtual int onStart(){};

    /*  模式停止，正常功能停止，休眠前停止模式

        断开远程连接，如socket
        停止onStart启动的线程
        需要考虑休眠进出问题
    */
    virtual int onStop(){};

    /*  模式销毁

        停止后台服务
        释放资源
        重置状态
    */
    virtual int onDestory(){};

    /*
        调用顺序为 onCreate -> onStart -> onStop -> onDestory
        onStart,onStop定义为快速启停模式
        费时操作在onCreate,onDestory中实现
    */


    //virtual int start(){};
    //virtual int stop(){};

    virtual int play(){};
    virtual int pause(){};
    virtual int next(){};
    virtual int prev(){};
    virtual int switchplay(){};

    virtual void setPlayer(ref_PPlayer player){
        mPlayer = player;
    }

    bool existPlayer(){
        if(mPlayer.AsPointer() != NULL)
            return true;
        return false;
    }

    tStatus getStatus(){
        locker::autolock l(mStatusLocker);
        return mStatus;
    };

    void setStatus(tStatus status){
        locker::autolock l(mStatusLocker);
        mStatus = status;
    }

    tMode getModeIndex(){
        return mMode;
    }

protected:
    int active_async(tCMD cmd){
        sendMessage(cmd);
        return SUCCESS;
    }

protected:
    tMode mMode;
    tStatus mStatus;

    tCMD mShouldPrevCmd;
    locker mNextCmdLocker;

    locker mStatusLocker;

    ref_EventWatcher mWatcher;
    ref_HandlerBase mHandlerBase;
    ref_PPlayer mPlayer;
};

}/*namespace Parrot*/
#endif /*__MODE_H__*/