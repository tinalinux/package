#define TAG "c_wifimanager"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <wifi_intf.h>
#include <tina_log.h>

#ifndef CPP
#define CPP
#include "aw_smartlinkd_connect.h"
#include "c_tinaplayer.h"
#include "c_wifimanager.h"
#include "voice.h"
#include "main.h"
#endif

static int event = WIFIMG_NETWORK_DISCONNECTED;
const aw_wifi_interface_t *p_wifi_interface = NULL;

/*================================================================
* Function: Wifi_Auto_Connect
* Description:
*   Wifi auto connect
* Parameter:
*   none
* Return:
*   0: successfully
*   -1: failed
*===============================================================*/
int Wifi_Auto_Connect(void)
{
    return p_wifi_interface->connect_ap_auto(0);
}

/*================================================================
* Function: Wifi_Init
* Description:
*   initial wifi
* Parameter:
*   none
* Return:
*   0: successfully
*   -1: failed
*===============================================================*/
int Wifi_Init(void)
{
    int res = 0;
    int timecnt = 4;
    while(Wifi_On() != 0 && timecnt > 0){
        printf("\tturn on wifi failed, try again for %d times\n", timecnt);
        timecnt--;
        sleep(1);
    }
    if (timecnt <= 0 && Wifi_Get_State() == WIFIMG_WIFI_DISABLED) {
        printf("\tturn on wifi failed, have tryed for 4 times , exit!!\n");
        return -1;
    }
    if (Wifi_Get_State() == WIFIMG_WIFI_DISCONNECTED) {
        Wifi_Auto_Connect();
    }
    return 0;
}

/*================================================================
* Function: Wifi_State
* Description:
*   this function is runned while switch state to wifi
* Parameter:
*   none
* Return:
*   none
*===============================================================*/
void Wifi_State(void)
{
    int time_cnt = 0;
    int rand_num = 0;
    time_t ti;
    char music_path[100] = {0};
    printf("\tnow in wifi state\n");
    if (Wifi_Get_State() == WIFIMG_WIFI_DISCONNECTED && Get_LastMainState() == STATE_SMARTLINKD) {
        say(VOICE_WIFI_AUTO_CONNETC_AFTER_SMARTLINKD);
        printf("\ttry to connect automatically\n");
        Wifi_Auto_Connect();
        for(time_cnt = 0; Wifi_Get_State() != WIFIMG_WIFI_CONNECTED && time_cnt <= 12; time_cnt++) {
            printf("\twait for wifi connecting automatically : %ds\n", 12-time_cnt);
            sleep(1);
        }
    }
    srand((unsigned)time(&ti));
    while(1){
        if (Wifi_Get_State() == WIFIMG_WIFI_CONNECTED) {
            say(VOICE_WIFI_NETWORK_CONNECTED);
            sleep(4);
            say(VOICE_WIFI_RECOMMENDED_MUSIC);
            sleep(4);
            while(1) {
                rand_num = rand() % 6 + 1;
                sprintf(music_path, "http://we.china-ota.com/WeChatSurvey/%d%d.mp3", rand_num, rand_num);
                Tinaplayer_Play(music_path);
                while(Tinaplayer_Get_Status() != STATUS_COMPLETED){
                    sleep(1);
                }
            }
        }
        else if (Wifi_Get_State() == WIFIMG_WIFI_DISCONNECTED) {
            say(VOICE_WIFI_NETWORK_NOT_CONNECTED);
            sleep(4);
            Set_MainState(STATE_SMARTLINKD);
            while(1) {
                sleep(1);
                printf("\twait for smartlink state\n");
            }
        }
        else if (Wifi_Get_State() == WIFIMG_WIFI_DISABLED) {
            say(VOICE_WIFI_NOT_ON);
            printf("\twifi is not on!\n");
            sleep(5);
        }
    }
}

/*================================================================
* Function: Wifi_Callback
* Description:
*   wifi call back function
* Parameter:
*   wifi_state: wifi status
*   buf: other info of wifi
* Return:
*   none
*===============================================================*/
static void wifi_Callback(tWIFI_EVENT wifi_event, void *buf, int event_label)
{
    switch(wifi_event)
    {
        case WIFIMG_WIFI_ON_SUCCESS:
        {
            printf("\tCallback note: WiFi on success!\n");
            event = WIFIMG_WIFI_ON_SUCCESS;
            break;
        }

        case WIFIMG_WIFI_ON_FAILED:
        {
            printf("\tCallback note: WiFi on failed!\n");
            event = WIFIMG_WIFI_ON_FAILED;
            break;
        }

        case WIFIMG_WIFI_OFF_FAILED:
        {
            printf("\tCallback note: wifi off failed!\n");
            event = WIFIMG_WIFI_OFF_FAILED;
            break;
        }

        case WIFIMG_WIFI_OFF_SUCCESS:
        {
            printf("\tCallback note: wifi off success!\n");
            event = WIFIMG_WIFI_OFF_SUCCESS;
            break;
        }

        case WIFIMG_NETWORK_CONNECTED:
        {
            printf("\tCallback note: WiFi connected ap!\n");
            event = WIFIMG_NETWORK_CONNECTED;
            break;
        }

        case WIFIMG_NETWORK_DISCONNECTED:
        {
            printf("\tCallback note: WiFi disconnected!\n");
            event = WIFIMG_NETWORK_DISCONNECTED;
            break;
        }

        case WIFIMG_PASSWORD_FAILED:
        {
            printf("\tCallback note: Password authentication failed!\n");
            event = WIFIMG_PASSWORD_FAILED;
            break;
        }

        case WIFIMG_CONNECT_TIMEOUT:
        {
            printf("\tCallback note: Connected timeout!\n");
            event = WIFIMG_CONNECT_TIMEOUT;
            break;
        }

        case WIFIMG_NO_NETWORK_CONNECTING:
        {
		  printf("\tCallback note: It has no wifi auto connect when wifi on!\n");
		  event = WIFIMG_NO_NETWORK_CONNECTING;
		  break;
        }

        case WIFIMG_CMD_OR_PARAMS_ERROR:
        {
            printf("\tCallback note: cmd or params error!\n");
            event = WIFIMG_CMD_OR_PARAMS_ERROR;
            break;
        }

        case WIFIMG_KEY_MGMT_NOT_SUPPORT:
        {
            printf("\tCallback note: key mgmt is not supported!\n");
            event = WIFIMG_KEY_MGMT_NOT_SUPPORT;
            break;
        }

        case WIFIMG_OPT_NO_USE_EVENT:
        {
            printf("\tCallback note: operation no use!\n");
            event = WIFIMG_OPT_NO_USE_EVENT;
            break;
        }

        case WIFIMG_NETWORK_NOT_EXIST:
        {
            printf("\tCallback note: network not exist!\n");
            event = WIFIMG_NETWORK_NOT_EXIST;
            break;
        }

        case WIFIMG_DEV_BUSING_EVENT:
        {
            printf("\tCallback note: wifi device busing!\n");
            event = WIFIMG_DEV_BUSING_EVENT;
            break;
        }

        default:
        {
            printf("\tCallback note: Other event, no care!\n");
        }
    }
}

/*================================================================
* Function: Wifi_On
* Description:
*   turn on wifi
* Parameter:
*   none
* Return:
*   0: successfully
*   else: failed
*===============================================================*/
int Wifi_On(void)
{
    p_wifi_interface = aw_wifi_on((tWifi_event_callback)wifi_Callback, 0);
    if (p_wifi_interface == NULL) {
        printf("\t%s: turn on wifi failed\n", __func__);
        return -1;
    }
    return 0;
}

/*================================================================
* Function: Wifi_Off
* Description:
*   turn off wifi
* Parameter:
*   none
* Return:
*   0: successfully
*   else: failed
*===============================================================*/
int Wifi_Off(void)
{
    while(Wifi_Get_State() == WIFIMG_WIFI_BUSING) {
        printf("\t%s: wifi is busy, wait..\n", __func__);
        usleep(1000 * 500);
    }
    return aw_wifi_off((aw_wifi_interface_t *)p_wifi_interface);
}

/*================================================================
* Function: Wifi_Get_Connected_AP_Info
* Description:
*   this funciton is to get the information connecting ap
*   if you want to determine whether connected, can use function aw_wifi_get_wifi_state()
* Parameter:
*   ssid: the pointer to which to save connecting ap's ssid
*   len: the length of the ssid buf when call the funciton, but the length of the connecting ap's ssid when return
*   both of those two parameter are enable only when returns is greater than 0
* Return:
*   >0: is connected
*   <=0: failed,not connectedn
*===============================================================*/
int Wifi_Get_Connected_AP_Info(char *ssid, int *len)
{
    while(Wifi_Get_State() == WIFIMG_WIFI_BUSING) {
        printf("\t%s: wifi is busy, wait..\n", __func__);
        usleep(1000 * 500);
    }
    return p_wifi_interface->is_ap_connected(ssid, len);
}

/*================================================================
* Function: Wifi_Scan
* Description:
*   scan wifi, and get scan result
* Parameter:
*   resutl: the pointer to which to save scan resutl
*   len: the length of the ssid buf when call the funciton, but the length of the connecting ap's ssid when return
*   both of those two parameter are enable only when returns is greater than 0
* Return:
*   -2: get scan result failed
*   -1: scan failed
*   0: successfully
*===============================================================*/
int Wifi_Scan(char *result, int *len)
{
    int res;
    while(Wifi_Get_State() == WIFIMG_WIFI_BUSING) {
        printf("\t%s: wifi is busy, wait..\n", __func__);
        usleep(1000 * 500);
    }

    printf("\tnow to scan wifi ap\n");
    res = p_wifi_interface->start_scan(0);
    if(res != 0){
        printf("\tscan failed!\n");
        return -1;
    }

    while(Wifi_Get_State() == WIFIMG_WIFI_BUSING) {
        printf("\t%s: wifi is busy, wait..\n", __func__);
        usleep(1000 * 500);
    }

    printf("\tdisplay wifi scan results\n");
    res = p_wifi_interface->get_scan_results(result, len);
    if(res != 0){
        printf("\tget scan result failed\n");
        return -2;
    }

    return 0;
}

/*================================================================
* Function: Wifi_Connect_AP
* Description:
*   connect wifi ap. get connected result by function:Get_Wifi_Status or put yourself code on callback function
*   becasue the value of security type is deffirent between smartlinkd and wifimanager,so adapt in here
* Parameter:
*   ssid: ap's ssid to connect
*   type: kind of security of the ap
*   passwd: password of the ap
* Return:
*   0: successfully
*   else: failed
*===============================================================*/
int Wifi_Connect_AP(const char *ssid, int type, const char *passwd)
{
    tKEY_MGMT tmp;
    if(type ==  AW_SMARTLINKD_SECURITY_WEP){
        tmp = WIFIMG_WEP;
    }
    else if(type == AW_SMARTLINKD_SECURITY_NONE){
        tmp = WIFIMG_NONE;
    }
    else if(type == AW_SMARTLINKD_SECURITY_WPA){
        tmp = WIFIMG_WPA_PSK;
    }
    else if(type == AW_SMARTLINKD_SECURITY_WPA2){
        tmp = WIFIMG_WPA2_PSK;
    }
    else {
        return -1;
    }

    while(Wifi_Get_State() == WIFIMG_WIFI_BUSING) {
        printf("\t%s: wifi is busy, wait..\n", __func__);
        usleep(1000 * 500);
    }
    return p_wifi_interface->connect_ap_key_mgmt(ssid, tmp, passwd, 0);
}

/*================================================================
* Function: Wifi_Disconnect_AP
* Description:
*   disconnect wifi ap. get connected result by function:Get_Wifi_Status or put yourself code on callback function
* Parameter:
*   none
* Return:
*   0: successfully
*   else: failed
*===============================================================*/
int Wifi_Disconnect_AP(void)
{
    while(Wifi_Get_State() == WIFIMG_WIFI_BUSING) {
        printf("\t%s: wifi is busy, wait..\n", __func__);
        usleep(1000 * 500);
    }
    return p_wifi_interface->disconnect_ap(0);
}

/*================================================================
* Function: Wifi_Get_State
* Description:
*   get wifi state
* Parameter:
*   none
* Return:
*   WIFIMG_WIFI_ENABLE = 0x1000
*   WIFIMG_WIFI_DISCONNECTED
*   WIFIMG_WIFI_BUSING
*   WIFIMG_WIFI_CONNECTED
*   WIFIMG_WIFI_DISABLED
*===============================================================*/
int Wifi_Get_State(void)
{
    return aw_wifi_get_wifi_state();
}
