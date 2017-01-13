#define TAG "playdemo"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <tina_log.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "main.h"
#include "key.h"
#include "voice.h"
#include "linein.h"
#include "smartlinkd.h"
#include "localmusic_player.h"
#include "c_wifimanager.h"
#include "c_powermanager.h"
#include "c_tinaplayer.h"
#include "c_bluetooth.h"

extern int aw_stopcooee(void);
struct APINFO my_ap_info;
static enum MAINSTATE main_state =  STATE_WIFI;
static enum MAINSTATE old_main_state = STATE_WIFI;
static pthread_mutex_t mutex_change_main_state;
static pthread_mutex_t mutex_switch_state_enable;
pthread_t thread_linein;
pthread_t thread_bluetooth;
pthread_t thread_wifi;
pthread_t thread_smartlinkd;
pthread_t thread_localmusic;

#ifndef TEMP_FAILURE_RETRY
/* Used to retry syscalls that can return EINTR. */
#define TEMP_FAILURE_RETRY(exp) ({         \
    typeof (exp) _rc;                      \
    do {                                   \
        _rc = (exp);                       \
    } while (_rc == -1 && errno == EINTR); \
    _rc; })
#endif

/* waring for boot compelted */
int waiting_for_bootcompeled(int timeout)
{
    int fd = -1;
    char buf[16] = {0};
    int count = 0;
    while (timeout--) {
        fd = TEMP_FAILURE_RETRY(open("/tmp/booting_state", O_RDONLY));
        count = read(fd, buf, 16);
        if (count > 0 && !strncmp(buf, "done", 4))
            break;
        sleep(1);
    };
    return timeout;
}

/*===========================================================================
* Function: create_thread
* Description:
*   create a thread when switch main state according to state
* Parameter:
*   state:
*       STATE_SWITCH: in state of switching state
*       STATE_WIFI: in state of wifi
*       STATE_LINEIN: in state of linein
*       STATE_BLUETOOTH: in state of bluetooth
*       STATE_SMARTLINKD: in state of smartlinkd
*       STATE_LOCALMUSIC: in state of localmusic
* Return:
*   -1: create failed
*   0: create successfully
*===========================================================================*/
inline int create_thread(enum MAINSTATE state)
{
    int res;
    switch (state) {
        case STATE_LINEIN:
        {
            res = pthread_create(&thread_linein, NULL, (void *)Linein_State, NULL);
            if(res != 0){
                printf("\tCreate thread of Linein_State failed\n");
                return -1;
            }
            res = pthread_detach(thread_linein);
            if(res != 0){
                printf("\tSet thread of Linein_State detached failed\n");
                return -1;
            }
            printf("\tCreate thread of linein successfully\n");
            break;
        }
        case STATE_WIFI:
        {
            res = pthread_create(&thread_wifi, NULL, (void *)Wifi_State, NULL);
            if(res != 0){
                printf("\tCreate thread of Wifi_State failed\n");
                return -1;
            }
            res = pthread_detach(thread_wifi);
            if(res != 0){
                printf("\tSet thread of Wifi_State detached failed\n");
                return -1;
            }
            printf("\tCreate thread of wifi successfully\n");
            break;
        }
        case STATE_SMARTLINKD:
        {
            res = pthread_create(&thread_smartlinkd, NULL, (void *)Smartlinkd_State, NULL);
            if(res != 0){
                printf("\tCreate thread of Smartlinkd_State failed\n");
                return -1;
            }
            res = pthread_detach(thread_smartlinkd);
            if(res != 0){
                printf("\tSet thread of Smartlinkd_State detached failed\n");
                return -1;
            }
            printf("\tCreate thread of smartlinkd successfully\n");
            break;
        }
        case STATE_BLUETOOTH:
        {
            res = pthread_create(&thread_bluetooth, NULL, (void *)Bluetooth_State, NULL);
            if(res != 0){
                printf("\tCreate thread of Bluetooth_State failed\n");
                return -1;
            }
            res = pthread_detach(thread_bluetooth);
            if(res != 0){
                printf("\tSet thread of Bluetooth_State detached failed\n");
                return -1;
            }
            printf("\tCreate thread of bluetooth successfully\n");
            break;
        }
        case STATE_LOCALMUSIC:
        {
            res = pthread_create(&thread_localmusic, NULL, (void *)LocalMusic_State, NULL);
            if(res != 0){
                printf("\tCreate thread of LocalMusic_State failed\n");
                return -1;
            }
            res = pthread_detach(thread_localmusic);
            if(res != 0){
                printf("\tSet thread of LocalMusic_State detached failed\n");
                return -1;
            }
            printf("\tCreate thread of localmusic successfully\n");
            break;
        }
        default:
        {
            printf("\terror state\n");
            return -1;
        }
    }
    return 0;
}

/*===========================================================================
* Function: cancel_thread
* Description:
*   cancel a thread when switch main state
* Parameter:
*   state:
*       STATE_SWITCH: in state of switching state
*       STATE_WIFI: in state of wifi
*       STATE_LINEIN: in state of linein
*       STATE_BLUETOOTH: in state of bluetooth
*       STATE_SMARTLINKD: in state of smartlinkd
*       STATE_LOCALMUSIC: in state of localmusic
* Return:
*   -1: state is not STATE_LINEIN,STATE_WIFI,STATE_SMARTLINKD,STATE_BLUETOOTH
*   0: cancel successfully
*   else: pthread_cancel error
*===========================================================================*/
inline int cancel_thread(enum MAINSTATE state)
{
    switch (state) {
        case STATE_LINEIN:
        {
            printf("\tcancel thread of linein\n");
            return pthread_cancel(thread_linein);
        }
        case STATE_WIFI:
        {
            printf("\tcancel thread of wifi\n");
            return pthread_cancel(thread_wifi);
        }
        case STATE_SMARTLINKD:
        {
            printf("\tcancel thread of smartlinkd\n");
            return pthread_cancel(thread_smartlinkd);
        }
        case STATE_BLUETOOTH:
        {
            printf("\tcancel thread of bluetooth\n");
            return pthread_cancel(thread_bluetooth);
        }
        case STATE_LOCALMUSIC:
        {
            printf("\tcancel thread of localmusic\n");
            return pthread_cancel(thread_localmusic);
        }
        default:
        {
            printf("\twrong parameter\n");
            return -1;
        }
    }
}

/*===========================================================================
* Function: Set_MainState
* Description:
*   Set a variable's value which determine the state in
*   note that this function also achieves priority
*   after changing main_state, this funciton will call switch_switch by unlock mutex
* Parameter:
*   state:
*       STATE_SWITCH: in state of switching state
*       STATE_WIFI: in state of wifi
*       STATE_LINEIN: in state of linein
*       STATE_BLUETOOTH: in state of bluetooth
*       STATE_SMARTLINKD: in state of smartlinkd
* Return:
*   -1: set failed
*   0: set successfully
*===========================================================================*/
int Set_MainState(enum MAINSTATE state)
{
    if (main_state == STATE_SMARTLINKD && state != STATE_SMARTLINKD) {
        printf("\tstop cooee of smartlinkd\n");
        aw_stopcooee();
    }
    if (isPlugIn() == 1) {
        if (main_state != STATE_LINEIN) {
            pthread_mutex_lock(&mutex_change_main_state);
            old_main_state = main_state;
            main_state = STATE_LINEIN;
            printf("\tSet main_state: STATE_LINEIN\n");
            pthread_mutex_unlock(&mutex_switch_state_enable);
            return 0;
        }
    }
    else{
        if (Bluetooth_isConnected() == 1) {
            if (main_state != STATE_BLUETOOTH) {
                pthread_mutex_lock(&mutex_change_main_state);
                old_main_state = main_state;
                main_state = STATE_BLUETOOTH;
                printf("\tSet main_state: STATE_BLUETOOTH\n");
                pthread_mutex_unlock(&mutex_switch_state_enable);
                return 0;
            }
        }
        else if (state != main_state) {
            pthread_mutex_lock(&mutex_change_main_state);
            if (state == STATE_WIFI) {
                printf("\tSet main_state: STATE_WIFI\n");
            }
            else if (state == STATE_SMARTLINKD) {
                printf("\tSet main_state: STATE_SMARTLINKD\n");
            }
            else if (state == STATE_LOCALMUSIC) {
                printf("\tSet main_state: STATE_LOCALMUSIC\n");
            }
            else {
                return -1;
            }
            old_main_state = main_state;
            main_state = state;
            pthread_mutex_unlock(&mutex_switch_state_enable);
            return 0;
        }
    }
    return -1;
}

/*===========================================================================
* Function: Get_MainState
* Description:
*   Get a variable's value which determine the state in
* Parameter:
*   none
* Return:
*   STATE_SWITCH: in state of switching state
*   STATE_WIFI: in state of wifi
*   STATE_LINEIN: in state of linein
*   STATE_BLUETOOTH: in state of bluetooth
*   STATE_SMARTLINKD: in state of smartlinkd
*===========================================================================*/
enum MAINSTATE Get_MainState(void)
{
    return main_state;
}

/*===========================================================================
* Function: Get_LastMainState
* Description:
*   Get a variable's value which determine the last state in
* Parameter:
*   none
* Return:
*   STATE_SWITCH: in state of switching state
*   STATE_WIFI: in state of wifi
*   STATE_LINEIN: in state of linein
*   STATE_BLUETOOTH: in state of bluetooth
*   STATE_SMARTLINKD: in state of smartlinkd
*===========================================================================*/
enum MAINSTATE Get_LastMainState(void)
{
    return old_main_state;
}

/*============================================================================
* Function: switch_state
* Description:
*   achieve switch state according to variable main_state
*   mutex is unlocked by function Set_MainState
* Parameter:
*   none
* Return:
*   -1: switch failed
*   0: switch successfully
*===========================================================================*/
static int switch_state(void)
{
    int res = 0;
    create_thread(STATE_WIFI);
    while(1){
        res = pthread_mutex_lock(&mutex_switch_state_enable);
        if(res != 0){
            printf("\tpthread lock mutex_switch_state_enable failed\n");
            return -1;
        }
        cancel_thread(old_main_state);
        create_thread(main_state);
        pthread_mutex_unlock(&mutex_change_main_state);
    }
}

int main(int argc, char *argv[])
{
    int res = 0;
    my_ap_info.ssid[0] = '\0';
    my_ap_info.password[0] = '\0';
    my_ap_info.security = 0;

    res = waiting_for_bootcompeled(10);
    if (res < 0)
        return -1;

    res = pthread_mutex_init(&mutex_switch_state_enable, NULL);
    if (res != 0) {
        printf("\tInitial mutex_switch_state_enable failed\n");
        return -1;
    }
    pthread_mutex_lock(&mutex_switch_state_enable);

    res = pthread_mutex_init(&mutex_change_main_state, NULL);
    if (res != 0) {
        printf("\tInitial mutex_change_main_state failed\n");
        return -1;
    }

    res = Tinaplayer_Init();
    if (res != 0) {
        printf("\tInit Tinaplayer Failed\n");
        return -1;
    }
    printf("\tInit Tianplayer Successfully\n");

    res = Power_Init();
    if (res != 0) {
        printf("\tInit Power Manager failed\n");
        return -1;
    }
    else {
        printf("\tInit Power Successfully\n");
    }

    res = InitKey();
    if (res != 0) {
        printf("\tInit Key Failed\n");
    }
    else {
        printf("\tInit Key Successfully\n");
    }

    say(VOICE_TURN_ON);

    res = Linein_Init();
    if (res != 0) {
        printf("\tInit Linein Failed\n");
    }
    else {
        printf("\tInit Linein Successfully\n");
    }

    res = Bluetooth_Init();
    if (res != 0) {
        printf("\tInit Bluetooth Failed\n");
    }
    else {
        printf("\tInit Bluetooth Successfully\n");
    }

    res = Wifi_Init();
    if (res != 0) {
        printf("\tInit Wifi Failed\n");
    }
    else {
        printf("\tInit Wifi Successfully\n");
    }

    res = Smartlinkd_Init();
    if (res != 0) {
        printf("\tInit Smartlinkd Failed\n");
    }
    else {
        printf("\tInit Smartlinkd Successfully\n");
    }

    res = 0;
    sleep(2);
    printf("\twait for bluetooth\\wifi\\linein connect\n");
    while (res <= 8 && (Wifi_Get_State() != WIFIMG_WIFI_CONNECTED && Bluetooth_isConnected() != 1 && isPlugIn() != 1)) {
        sleep(1);
        res ++;
    }
    switch_state();
}
