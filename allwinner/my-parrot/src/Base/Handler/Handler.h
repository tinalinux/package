#ifndef __HANDLER_H__
#define __HANDLER_H__
#include "T_Reference.h"
#include "HandlerBase.h"

namespace Parrot{
class MessageData
{
public:
    MessageData();
    ~MessageData();
    int setData(void *data, int len);
    void *data;
    int len;
};
typedef T_Reference<MessageData> ref_Data;

class Message : public HandlerBaseMessage
{
public:
    Message();
    ~Message();
    int what;
    ref_Data data;
    void *handler;
    void process();
    int setData(void *data, int len);
    void * getData();
    int getDataLen();
};

class MessageHandler
{
public:
    virtual void handleMessage(Message *msg) = 0;
};
class Handler
{
public:
    Handler(ref_HandlerBase base):mBase(base),mMessageHandler(NULL){};
    virtual ~Handler(){};

    int sendMessage(int what, int sec = 0, int usec = 0);
    int sendMessageDelayed(int what, int sec, int usec);
    int sendMessage(Message &msg, int sec = 0, int usec = 0);
    int sendMessageDelayed(Message &msg, int sec, int usec);

    //callback from Message
    virtual void handleMessage(Message *msg);

    void setMessageHandler(MessageHandler *handler){
        mMessageHandler = handler;
    }

private:
    int send_message(Message &msg,int sec,int usec);

private:
    ref_HandlerBase mBase;
    MessageHandler *mMessageHandler;
};
}/*namespace Parrot*/

#endif/*__HANDLER_H__*/