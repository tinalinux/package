#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <memory.h>

#include <netinet/ether.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <net/if_arp.h>

#include "parrot_common.h"
#include "BroadcastSender.h"

namespace Parrot{

int BroadcastSender::init(int port)
{
    int addr_len, broadcast = 1;
    unsigned int self_ipaddr = 0, i;
    unsigned char payload = 0;

    //sleep(10);
    plogd("Broadcast init");
    if ((mSendfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        ploge("socket failed");
        return FAILURE;
    }

    // this call is what allows broadcast packets to be sent:
    if (setsockopt(mSendfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast) == -1) {
        ploge("setsockopt (SO_BROADCAST) failed");
        return FAILURE;
    }

    mDest = (void*)malloc(sizeof(struct sockaddr_in));
    struct sockaddr_in *s = (struct sockaddr_in*)mDest;
    s->sin_family = AF_INET;
    s->sin_port = htons(port);
    s->sin_addr.s_addr = htonl(INADDR_BROADCAST);//inet_addr("255.255.255.255");//self_ipaddr;   //htonl(INADDR_BROADCAST);
    return SUCCESS;

}
int BroadcastSender::release()
{
    shutdown(mSendfd, SHUT_RDWR);
    free(mDest);
    mSendfd = -1;
    return SUCCESS;
}

int BroadcastSender::send(const char *buf, int len)
{
    if(mSendfd == -1) return FAILURE;

    int ret = sendto(mSendfd, buf, len, 0, (struct sockaddr *)mDest, sizeof(struct sockaddr));
    if(ret == -1)
        ploge("BroadcastSender::send ret = %d,%s\n",ret,strerror(errno));

    return ret;
}

}
