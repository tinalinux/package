#include <stdio.h>
#include <unistd.h>
#include "Handler.h"
#include "parrot_common.h"
namespace Parrot{
MessageData::MessageData():data(NULL),len(0)
{

}
MessageData::~MessageData()
{
    if(data != NULL) {
        delete (char*)data;
        data = NULL;
    }
}
int MessageData::setData(void *data, int len)
{
    if(this->len == 0){
        this->data = new char[len + 1];
        CHECK_POINTER(this->data);
        memset(this->data, 0, len + 1);
        memcpy(this->data, data, len);
        this->len = len;
    }else if(this->len > 0){
        int len_new = this->len + len;
        void *data_new = new char[ len_new + 1 ];
        CHECK_POINTER(data_new);
        memset(data_new, 0, len_new + 1);
        memcpy(data_new, this->data, this->len);
        memcpy((char*)data_new + this->len, data, len);
        delete (char*)(this->data);
        this->data = data_new;
        this->len = len_new;
    }
    return SUCCESS;
}

Message::Message():what(0),handler(NULL)
{
    data = new MessageData();//??
}

Message::~Message()
{

}

void Message::process()
{
    Handler *h = (Handler*)handler;
    h->handleMessage(this);
    delete this;
}

int Message::setData(void *data, int len)
{
    return this->data->setData(data, len);
}

void * Message::getData()
{
    return this->data->data;
}
int Message::getDataLen()
{
    return this->data->len;
}
int Handler::sendMessage(int what,int sec, int usec)
{
    Message msg;
    msg.what = what;
    return send_message(msg,sec,usec);
}

int Handler::sendMessage(Message &msg, int sec, int usec)
{
    return send_message(msg,sec,usec);
}
int Handler::sendMessageDelayed(int what,int sec, int usec)
{
    return sendMessage(what,sec,usec);
}

int Handler::sendMessageDelayed(Message &msg, int sec, int usec)
{
    return sendMessage(msg,sec,usec);
}

void Handler::handleMessage(Message *msg)
{
    if(mMessageHandler != NULL)
        mMessageHandler->handleMessage(msg);
}

int Handler::send_message(Message &msg,int sec,int usec)
{
    Message *m = new Message();
    m->what = msg.what;
    m->data = msg.data;
    m->handler = this;
    mBase->sendMessage((HandlerBaseMessage*)m,sec,usec);
}

}/*namespace Parrot*/