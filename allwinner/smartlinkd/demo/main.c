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

/*
    wifi connect test
    include <wifi_intf.h>
    add by Kirin 2016/11/22
    it should link to libwifimg.so
*/
#include <wifi_intf.h>


int debug_enable = 1;

/*
    param used to wifi connect test ,add by Kirin 2016/11/22
*/
const aw_wifi_interface_t *p_wifi_interface = NULL;
tKEY_MGMT key_mgmt;
int event_label = 0;
static int event = WIFIMG_NETWORK_DISCONNECTED;

static void wifi_event_handle(tWIFI_EVENT wifi_event, void *buf, int event_label)
{
    printf("event_label 0x%x\n", event_label);

    switch(wifi_event)
    {
        case WIFIMG_WIFI_ON_SUCCESS:
        {
            printf("WiFi on success!\n");
            event = WIFIMG_WIFI_ON_SUCCESS;
            break;
        }

        case WIFIMG_WIFI_ON_FAILED:
        {
            printf("WiFi on failed!\n");
            event = WIFIMG_WIFI_ON_FAILED;
            break;
        }

        case WIFIMG_WIFI_OFF_FAILED:
        {
            printf("wifi off failed!\n");
            event = WIFIMG_WIFI_OFF_FAILED;
            break;
        }

        case WIFIMG_WIFI_OFF_SUCCESS:
        {
            printf("wifi off success!\n");
            event = WIFIMG_WIFI_OFF_SUCCESS;
            break;
        }

        case WIFIMG_NETWORK_CONNECTED:
        {
            printf("WiFi connected ap!\n");
            event = WIFIMG_NETWORK_CONNECTED;
            break;
        }

        case WIFIMG_NETWORK_DISCONNECTED:
        {
            printf("WiFi disconnected!\n");
            event = WIFIMG_NETWORK_DISCONNECTED;
            break;
        }

        case WIFIMG_PASSWORD_FAILED:
        {
            printf("Password authentication failed!\n");
            event = WIFIMG_PASSWORD_FAILED;
            break;
        }

        case WIFIMG_CONNECT_TIMEOUT:
        {
            printf("Connected timeout!\n");
            event = WIFIMG_CONNECT_TIMEOUT;
            break;
        }

        case WIFIMG_NO_NETWORK_CONNECTING:
        {
            printf("It has no wifi auto connect when wifi on!\n");
            event = WIFIMG_NO_NETWORK_CONNECTING;
            break;
        }

        case WIFIMG_CMD_OR_PARAMS_ERROR:
        {
            printf("cmd or params error!\n");
            event = WIFIMG_CMD_OR_PARAMS_ERROR;
            break;
        }

        case WIFIMG_KEY_MGMT_NOT_SUPPORT:
        {
            printf("key mgmt is not supported!\n");
            event = WIFIMG_KEY_MGMT_NOT_SUPPORT;
            break;
        }

        case WIFIMG_OPT_NO_USE_EVENT:
        {
            printf("operation no use!\n");
            event = WIFIMG_OPT_NO_USE_EVENT;
            break;
        }

        case WIFIMG_NETWORK_NOT_EXIST:
        {
            printf("network not exist!\n");
            event = WIFIMG_NETWORK_NOT_EXIST;
            break;
        }

        case WIFIMG_DEV_BUSING_EVENT:
        {
            printf("wifi device busing!\n");
            event = WIFIMG_DEV_BUSING_EVENT;
            break;
        }

        default:
        {
            printf("Other event, no care!\n");
        }
    }
}

int onRead(char* buf,int length)
{
    if(length == THREAD_INIT){
        printf("if(length == THREAD_INIT)...\n");
    }
    else if(length == -1){
        printf("else if(length == -1)...\n");
    }else if(length == 0){
        printf("server close the connection...\n");
        //exit(0);
        return THREAD_EXIT;

    }else {
        printf("lenght: %d\n",length);
		printf("buf: %s\n",buf);
        //printf_info((struct _cmd *)buf);
        struct _cmd* c = (struct _cmd *)buf;
        if(c->cmd == AW_SMARTLINKD_FAILED){
            printf("response failed,c->cmd == AW_SMARTLINKD_FAILED\n");
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

#if 0
            printf("\n*********************************\n");
            printf("***Start wifi connect ap test!***\n");
            printf("*********************************\n");

            while(aw_wifi_get_wifi_state() == WIFIMG_WIFI_BUSING){
                printf("wifi state busing,waiting  11111\n");
                usleep(2000000);
            }

            //judge the encryption method of wifi
            if(c->info.base_info.security ==0)
			key_mgmt = 0;
		else if(c->info.base_info.security ==1)
			key_mgmt = 3;
		else if(c->info.base_info.security ==2)
			key_mgmt = 1;
		else if(c->info.base_info.security ==3)
                    key_mgmt = 2;


/*
WLAN_SECURITY_NONE = 0,
    WLAN_SECURITY_WEP,
    WLAN_SECURITY_WPA,
    WLAN_SECURITY_WPA2,

    typedef enum {
        WIFIMG_NONE = 0,
        WIFIMG_WPA_PSK,
        WIFIMG_WPA2_PSK,
        WIFIMG_WEP,
    }tKEY_MGMT;
*/
            event_label++;
            p_wifi_interface->add_network(c->info.base_info.ssid,key_mgmt,c->info.base_info.password,event_label);
    while(aw_wifi_get_wifi_state() == WIFIMG_WIFI_BUSING){
        printf("wifi state busing,waiting  22222\n");
        usleep(2000000);
    }

    printf("******************************\n");
    printf("Wifi connect ap test: Success!\n");
    printf("******************************\n");
#endif
}
    return THREAD_CONTINUE;
}

int main(int argc, char* argv[])
{
    int proto = 0;
    printf("#########start smartlinkd#########\n");

	printf("#########check wifi statue#########\n");
	event_label = rand();
				p_wifi_interface = aw_wifi_on(wifi_event_handle, event_label);
				if(p_wifi_interface == NULL){
					 printf("wifi on failed event 0x%x\n", event);
					 return -1;
				}

				while(aw_wifi_get_wifi_state() == WIFIMG_WIFI_BUSING){
						printf("wifi state busing,waiting  33333\n");
						usleep(2000000);
					}

				while(aw_wifi_get_wifi_state() == WIFIMG_WIFI_CONNECTED){
					 p_wifi_interface->disconnect_ap(0x7fffffff);
					 printf("aw_wifi_disconnect_ap\n");
					 usleep(1000000);
				}

	printf("#########check wifi statue ok#########\n");

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
