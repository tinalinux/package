#ifndef __BROADCAST_SENDER_H__
#define __BROADCAST_SENDER_H__

namespace Parrot{

class BroadcastSender
{
public:
    int init(int port);
    int release();

    int send(const char *buf, int len);

private:
    int mSendfd;
    void *mDest;
};
}/*namesapce Parrot*/
#endif/*__BROADCAST_SENDER_H__*/