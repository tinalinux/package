#define TAG "smartlinkd-demo"
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

#include <tina_log.h>
#include "aw_smartlinkd_connect.h"

int debug_enable = 1;

int onRead(char* buf,int length)
{
    if(length == THREAD_INIT){

    }
    else if(length == -1){

    }else if(length == 0){
        printf("server close the connection...\n");
        //exit(0);
        return THREAD_EXIT;

    }else {
        printf("lenght: %d\n",length);
        //printf_info((struct _cmd *)buf);
        struct _cmd* c = (struct _cmd *)buf;
        if(c->cmd == AW_SMARTLINKD_FAILED){
            printf("response failed\n");
            return THREAD_CONTINUE;
        }
        if(c->info.protocol == AW_SMARTLINKD_PROTO_FAIL){
            printf("proto scan fail");
            return THREAD_CONTINUE;
        }
        printf("cmd: %d\n",c->cmd);
        printf("pcol: %d\n",c->info.protocol);
        printf("ssid: %s\n",c->info.base_info.ssid);
        printf("pasd: %s\n",c->info.base_info.password);
        printf("security: %d\n",c->info.base_info.security);

        if(c->info.protocol == AW_SMARTLINKD_PROTO_AKISS)
            printf("radm: %d\n",c->info.airkiss_random);
        if(c->info.protocol == AW_SMARTLINKD_PROTO_COOEE){
            printf("ip: %s\n",c->info.ip_info.ip);
            printf("port: %d\n",c->info.ip_info.port);
        }
        if(c->info.protocol == AW_SMARTLINKD_PROTO_ADT){
            printf("adt get: %s\n",c->info.adt_str);
        }
    }
    return THREAD_CONTINUE;
}

int main(int argc, char* argv[])
{
    int proto = 0;
    if(argc > 1){
        proto = atoi(argv[1]);
    }

    //aw_smartlinkd_prepare();
    if(aw_smartlinkd_init(0,onRead) == 0){
        if(proto == 0){
            printf("start airkiss\n");
            aw_startairkiss();
        }
        else if(proto == 1){
            printf("start cooee\n");
            aw_startcooee();
        }
        else if(proto == 2){
            printf("start adt\n");
            aw_startadt();
        }
        else if(proto ==3){
            printf("start composite\n");
            aw_startcomposite(AW_SMARTLINKD_PROTO_COOEE|AW_SMARTLINKD_PROTO_ADT);
        }
    }
    /*
    sleep(1);
    aw_stopcooee();
    sleep(2);
    aw_startairkiss();
    sleep(1);
    aw_stopairkiss();
    sleep(1);
    aw_startcooee();
    sleep(1);
    aw_stopcooee();
    sleep(1);
    aw_startadt();
    sleep(30);
    aw_startadt();
    */
    while(1);

    return 0;
}
