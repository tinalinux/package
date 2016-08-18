#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <tina_log.h>

#include "main.h"
#include "voice.h"
#include "smartlinkd.h"
#include "c_wifimanager.h"
#include "c_tinaplayer.h"
#include "aw_smartlinkd_connect.h"

static int is_get_ap_info = 0;
static int is_smartlinkd_scan_timeout = 0;

/*=================================================== * Function: smartlinkd_callback
* Description:
*   call back function
* Parameter:
*   buf: ap infomation from device
*   length: the length of buf
* Return:
*   THREAD_EXIT: exit the pthread
*   THREAD_CONTINUE: continue the pthread
*==================================================*/
static int smartlinkd_callback(char* buf,int length)
{
    if (length == THREAD_INIT) {
        return THREAD_CONTINUE;
    }
    else if (length == -1) {
        return THREAD_CONTINUE;
    }
    else if (length == 0) {
        printf("\tCallback note: server close the connection...\n");
        is_get_ap_info = 1;
        return THREAD_EXIT;
    }
    else {
        struct _cmd* c = (struct _cmd *)buf;
        if (c->cmd == AW_SMARTLINKD_FAILED) {
            printf("\tCallback note: response failed\n");
            return THREAD_CONTINUE;
        }
        if(c->info.protocol == AW_SMARTLINKD_PROTO_FAIL){
            printf("\tCallback note: timeout or proto scan fail\n");
            is_smartlinkd_scan_timeout = 1;
            return THREAD_EXIT;
        }
        strncpy(my_ap_info.ssid, c->info.base_info.ssid, strlen(c->info.base_info.ssid) >= 63 ? 63 : strlen(c->info.base_info.ssid));
        my_ap_info.ssid[strlen(c->info.base_info.ssid)] = '\0';
        strncpy(my_ap_info.password,c->info.base_info.password,strlen(c->info.base_info.ssid) >= 99 ? 99 : strlen(c->info.base_info.password));
        my_ap_info.password[strlen(c->info.base_info.password)] = '\0';
        my_ap_info.security = c->info.base_info.security;
        printf("\tCallback note: ssid: %s\n",my_ap_info.ssid);
        printf("\tCallback note: pasd: %s\n",my_ap_info.password);
        printf("\tCallback note: security: %d\n",my_ap_info.security);
        printf("\tCallback note: pcol: %d\n",c->info.protocol);
        if(c->info.protocol == AW_SMARTLINKD_PROTO_AKISS)
            printf("\tCallback note: radm: %d\n",c->info.airkiss_random);
        if(c->info.protocol == AW_SMARTLINKD_PROTO_COOEE){
            printf("\tCallback note: ip: %s\n",c->info.ip_info.ip);
            printf("\tCallback note: port: %d\n",c->info.ip_info.port);
        }
        return THREAD_CONTINUE;
    }
}

/*===================================================
* Function: Smartlinkd_Init
* Description:
*   Initial samrtlinkd
* Parameter:
*   none
* Return:
*   -1: failed
*   0: successfully
*==================================================*/
int Smartlinkd_Init(void)
{
    is_get_ap_info = 0;
    is_smartlinkd_scan_timeout = 0;
    return 0;
}


/*===================================================
* Function: Smartlinkd_State
* Description:
*   this function is runned while main_state is STATE_SMARTLINKD
* Parameter:
*   none
* Return:
*   none
*==================================================*/
void Smartlinkd_State(void)
{
    printf("\tnow in smartlinkd state\n");
    int res = 0;
    if (Wifi_Get_State() ==  WIFIMG_WIFI_CONNECTED) {
        printf("\tWifi is connected, now to disconnected\n");
        Wifi_Disconnect_AP();
    }
    while(1){
        is_get_ap_info = 0;
        is_smartlinkd_scan_timeout = 0;
        say(VOICE_SMARTLINKD_IN);
        sleep(2);
        printf("\tStart to run smartlinkd\n");
        res = aw_smartlinkd_init(0,smartlinkd_callback);
        if (res != 0) {
            printf("\tsmartlinkd init failed! but go ahead\n");
        }
        aw_startcooee();
        while (is_get_ap_info == 0 && is_smartlinkd_scan_timeout == 0) {
            printf("\twait for ap_info\n");
            sleep(1);
        }
        if (is_smartlinkd_scan_timeout == 1) {
            printf("\tsmartlinkd scan time out\n");
            continue;
        }
        aw_stopcooee();
        say(VOICE_SMARTLINKD_GET_AP);
        while(Wifi_Get_State() == WIFIMG_WIFI_BUSING){
            printf("\twifi is busy, wait\n");
            sleep(1);
        }
        printf("\tnow to connect ap with ap info from smartlinkd\n");
        Wifi_Connect_AP(my_ap_info.ssid, my_ap_info.security, my_ap_info.password);
        while(Wifi_Get_State() == WIFIMG_WIFI_BUSING){
            printf("\twait for wifi connect ap\n");
            sleep(1);
        }
        if (Wifi_Get_State() == WIFIMG_WIFI_CONNECTED) {
            Set_MainState(STATE_WIFI);
            while(1){
                sleep(1);
                printf("\twait for wifi state\n");
            }
        }
    }
}
