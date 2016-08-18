#include <string.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <errno.h>

#include "parrot_common.h"
#include "ConfigUtils.h"

static int get_mac_addr(const char* name,char* hwaddr)
{
    struct ifreq ifreq;
    int sock;

    if((sock=socket(AF_INET,SOCK_STREAM,0)) <0)
    {
        perror( "socket ");
        return 2;
    }
    strcpy(ifreq.ifr_name,name);
    if(ioctl(sock,SIOCGIFHWADDR,&ifreq) <0)
    {
        perror( "ioctl ");
        return 3;
    }
    sprintf(hwaddr,"%02x:%02x:%02x:%02x:%02x:%02x",
            (unsigned char)ifreq.ifr_hwaddr.sa_data[0],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[1],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[2],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[3],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[4],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[5]);

    return   0;
}

namespace Parrot{

#define CONFIG_STR_LEN 100

static char BtMacConfig[CONFIG_STR_LEN];
static char BtNameConfig[CONFIG_STR_LEN];

static char Mac[CONFIG_STR_LEN];
static char UUID[CONFIG_STR_LEN];




void ConfigUtils::init()
{
    memset(BtMacConfig, 0 , sizeof(BtMacConfig));
    memset(BtNameConfig, 0 , sizeof(BtNameConfig));
    memset(Mac, 0, sizeof(Mac));
    memset(UUID, 0, sizeof(UUID));

    //get Mac
    get_mac_addr("wlan0", Mac);
    get_mac_addr("wlan0", UUID);


    //set BT config
    strncpy(BtNameConfig,"Henrisk's Parrot",sizeof(BtNameConfig));
    strncpy(BtMacConfig,"/etc/bluetooth/mac_config",sizeof(BtMacConfig));
    if(access(BtMacConfig,F_OK) != 0){
        //config file is no exist, make it!
        FILE *fp = fopen(BtMacConfig,"wb+");
        if(fp == NULL){
            ploge("fopen %s error(%s)",BtMacConfig, strerror(errno));
            return;
        }
        fwrite(Mac,strlen(Mac)+1,1,fp);
        fflush(fp);
        fclose(fp);
    }else{
        plogd("%s is exist!",BtMacConfig);
    }
}

char *ConfigUtils::getMac()
{
    return Mac;
}

char *ConfigUtils::getUUID()
{
    return UUID;
}

char *ConfigUtils::getBTMacConfig()
{
    return BtMacConfig;
}

char *ConfigUtils::getBTNameConfig()
{
    return BtNameConfig;
}
}