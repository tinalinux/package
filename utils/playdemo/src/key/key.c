#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <tina_log.h>

#include "key.h"
#include "main.h"
#include "linein.h"
#include "localmusic_player.h"
#include "c_tinaplayer.h"
#include "c_powermanager.h"
#include "c_bluetooth.h"

#define KEYHOLDTIME 2

static int key_power_is_pressed = 0;
static int key_volume_down_is_pressed = 0;
static int key_volume_up_is_pressed = 0;
static int timecnt_250ms_key_power = 0;
static int timecnt_250ms_key_volume_up = 0;
static int timecnt_250ms_key_volume_down = 0;
static pthread_t thread_scan_key_power;
static pthread_t thread_scan_key_volume;
static pthread_t thread_do_key_power;
static pthread_t thread_do_key_volume_down;
static pthread_t thread_do_key_volume_up;
static pthread_mutex_t mutex_key_power;
static pthread_mutex_t mutex_key_volume_down;
static pthread_mutex_t mutex_key_volume_up;

/*================================================================
* Function: scan_key_power
* Decription:
*   scan key power, change flag while key is pressed or released
* Parameter:
*   none
* Return:
*   none
*===============================================================*/
static void scan_key_power(void)
{
    char *path = "/dev/input/event0";
    struct input_event key_event;
    int key_fd = 0;

    key_fd = open(path, O_RDONLY);
    if (key_fd < 0) {
        printf("\topen %s failed!\n",path);
        pthread_exit(0);
    }
    while(1){
        if (read(key_fd, &key_event, sizeof(key_event)) == sizeof(key_event) && key_event.type == EV_KEY) {
            if (key_event.value == 1) {
                printf("\tkey power is pressed\n");
                key_power_is_pressed = 1;
                printf("\tthe flag key_power_is_pressed is %d\n", key_power_is_pressed);
                pthread_mutex_unlock(&mutex_key_power);
                usleep(1000 * 500);
            }
            else if (key_event.value == 0) {
                printf("\tkey power is released\n");
                key_power_is_pressed = 0;
                printf("\tthe flag key_power_is_pressed is %d\n", key_power_is_pressed);
            }
        }
    }
}

/*================================================================
* Function: scan_key_volume
* Decription:
*   scan key volume,including volume+ and volume-, change flag while key is pressed or released
* Parameter:
*   none
* Return:
*   none
*===============================================================*/
static void scan_key_volume(void)
{
    char *path = "/dev/input/event1";
    struct input_event key_event;
    int key_fd = 0;

    key_fd = open(path, O_RDONLY);
    if (key_fd < 0) {
        printf("\topen %s failed!\n",path);
        pthread_exit(0);
    }
    while(1){
        if (read(key_fd, &key_event, sizeof(key_event)) == sizeof(key_event) && key_event.type == EV_KEY) {
            if (key_event.code == 0x72) {
                if (key_event.value == 1) {
                    printf("\tkey volume down is pressed\n");
                    key_volume_down_is_pressed = 1;
                    pthread_mutex_unlock(&mutex_key_volume_down);
                    printf("\tthe flag key_volume_down_is_pressed is %d\n", key_volume_down_is_pressed);
                    usleep(1000 * 500);
                }
                else if(key_event.value == 0) {
                    printf("\tkey volume down is released\n");
                    key_volume_down_is_pressed = 0;
                    printf("\tthe flag key_volume_down_is_pressed is %d\n", key_volume_down_is_pressed);
                }
            }
            else if (key_event.code == 0x73) {
                if (key_event.value == 1) {
                    printf("\tkey volume up is pressed\n");
                    key_volume_up_is_pressed = 1;
                    pthread_mutex_unlock(&mutex_key_volume_up);
                    printf("\tthe flag key_volume_up_is_pressed is %d\n", key_volume_up_is_pressed);
                    usleep(1000 * 500);
                }
                else if (key_event.value == 0) {
                    printf("\tkey volume up is released\n");
                    key_volume_up_is_pressed = 0;
                    printf("\tthe flag key_volume_up_is_pressed is %d\n", key_volume_up_is_pressed);
                }
            }
        }
    }
}

/*================================================================
* Function: do_key_power
* Decription:
*   do while key_power is pressed
* Parameter:
*   none
* Return:
*   none
*===============================================================*/
void do_key_power(void){
    pthread_mutex_lock(&mutex_key_power);
    while(1){
        pthread_mutex_lock(&mutex_key_power);
        timecnt_250ms_key_power = 0;
        printf("\tin function :%s\n", __func__);
        while(1){
            if (key_power_is_pressed == 1 && Power_Get_LockCnt() == 0) {
                printf("\twake up, lock power\n");
                Power_Lock();
                usleep(1000 * 250);
                printf("\tnow to continue tinaplayer\n");
                Tinaplayer_Continue();
                break;
            }
            if (key_power_is_pressed == 1 && key_volume_down_is_pressed != 1 && key_volume_up_is_pressed != 1) {
                timecnt_250ms_key_power++;
                usleep(1000 * 250);
                if (timecnt_250ms_key_power >= KEYHOLDTIME * 4) {
                    printf("\tkey power is longly pressed\n");
                    if (Tinaplayer_IsPlaying() == 1) {
                        printf("\tnow to stop tinaplayer\n");
                        Tinaplayer_Stop();
                        sleep(3);
                    }
                    printf("\tnow to unlock power\n");
                    Power_Unlock();
                    break;
                }
            }
            else if (key_power_is_pressed == 1 && (key_volume_down_is_pressed == 1 || key_volume_up_is_pressed == 1)) {
                if (key_volume_down_is_pressed == 1) {//v- and power are both pressed
                    key_volume_down_is_pressed = 2;
                    timecnt_250ms_key_volume_down = 0;
                    printf("\tpower and volume down are both pressed\n");
                    if (Get_MainState() == STATE_WIFI) {
                        Set_MainState(STATE_SMARTLINKD);
                    }
                    else if (Get_MainState() != STATE_WIFI) {
                        Set_MainState(STATE_WIFI);
                    }
                }
                else if (key_volume_up_is_pressed == 1) {//v+ and power are both pressed
                    key_volume_up_is_pressed = 2;
                    timecnt_250ms_key_volume_up = 0;
                    printf("\tpower and volume up are both pressed\n");
                    if (Get_MainState() == STATE_LOCALMUSIC) {
                        Set_MainState(STATE_WIFI);
                    }
                    else if (Get_MainState() != STATE_LOCALMUSIC) {
                        Set_MainState(STATE_LOCALMUSIC);
                    }
                }
                break;
            }
            else if (key_power_is_pressed == 0 && timecnt_250ms_key_power < KEYHOLDTIME * 4 && timecnt_250ms_key_power > 0) {
                printf("\tkey power is shortly pressed\n");
                printf("\tonly press key power\n");
                if (Get_MainState() == STATE_WIFI || Get_MainState() == STATE_LOCALMUSIC) {
                    if (Tinaplayer_IsPlaying() == 1) {
                        Tinaplayer_Pause();
                    }
                    else{
                        Tinaplayer_Start();
                    }
                }
                else if (Get_MainState() == STATE_BLUETOOTH) {
                    if (Bluetooth_isPlaying() == 1) {
                        Bluetooth_Avk_Pause();
                    }
                    else{
                        Bluetooth_Avk_Play();
                    }
                }
                else if (Get_MainState() == STATE_LINEIN) {
                    if (Linein_isPlaying() == 1) {
                        mixer_set(0);
                    }
                    else if (Linein_isPlaying() == 0) {
                        mixer_set(1);
                    }
                }
                break;
            }
            else {
                break;
            }
        }
    }
}

/*================================================================
* Function: do_key_volume_down
* Decription:
*   do while key_volume_down is pressed
* Parameter:
*   none
* Return:
*   none
*===============================================================*/
void do_key_volume_down(void){
    pthread_mutex_lock(&mutex_key_volume_down);
    while(1){
        pthread_mutex_lock(&mutex_key_volume_down);
        timecnt_250ms_key_volume_down = 0;
        printf("\tin function :%s\n", __func__);
        while(1){
            if (key_volume_down_is_pressed == 1) {
                timecnt_250ms_key_volume_down++;
                usleep(1000 * 250);
                if (timecnt_250ms_key_volume_down >= KEYHOLDTIME * 4) {
                    printf("\tkey volume down is longly pressed\n");
                    if (Get_MainState() == STATE_BLUETOOTH) {
                        printf("\tbluetooth: play previous song\n");
                        Bluetooth_Avk_Previous();
                    }
                    else if (Get_MainState() == STATE_LOCALMUSIC) {
                        printf("\tlocalmusic: play previous song\n");
                        LocalMusic_Previous();
                    }
                    break;
                }
            }
            else if (key_volume_down_is_pressed == 0 && timecnt_250ms_key_volume_down < KEYHOLDTIME * 4 && timecnt_250ms_key_volume_down > 0) {
                printf("\tkey volume down is shortly pressed\n");
                Tinaplayer_SetVolume(DOWN_VOLUME);
                break;
            }
            else {
                break;
            }
        }
    }
}

/*================================================================
* Function: do_key_volume_up
* Decription:
*   do while key_volume_up is pressed
* Parameter:
*   none
* Return:
*   none
*===============================================================*/
void do_key_volume_up(void){
    pthread_mutex_lock(&mutex_key_volume_up);
    while(1){
        pthread_mutex_lock(&mutex_key_volume_up);
        timecnt_250ms_key_volume_up = 0;
        printf("\tin function :%s\n", __func__);
        while(1){
            if (key_volume_up_is_pressed == 1) {
                timecnt_250ms_key_volume_up++;
                usleep(1000 * 250);
                if (timecnt_250ms_key_volume_up >= KEYHOLDTIME * 4) {
                    printf("\tkey volume up is longly pressed\n");
                    if (Get_MainState() == STATE_BLUETOOTH) {
                        printf("\tbluetooth: play next song\n");
                        Bluetooth_Avk_Next();
                    }
                    else if (Get_MainState() == STATE_LOCALMUSIC) {
                        printf("\tlocalmusic: play next song\n");
                        LocalMusic_Next();
                    }
                    break;
                }
            }
            else if (key_volume_up_is_pressed == 0 && timecnt_250ms_key_volume_up < KEYHOLDTIME * 4 && timecnt_250ms_key_volume_up > 0) {
                printf("\tkey volume up is shortly pressed\n");
                Tinaplayer_SetVolume(UP_VOLUME);
                break;
            }
            else {
                break;
            }
        }
    }
}

/*================================================================
* Function: initKey
* Decription:
*   initialize key variable and create pthreadsof keys
* Parameter:
*   none
* Return:
*   -1: create pthread failed
*   0: successfully
*===============================================================*/
int InitKey(void)
{
    int res = 0;
    res = pthread_mutex_init(&mutex_key_power, NULL);
    if (res != 0) {
        printf("\tinitial mutex_key_power failed\n");
        return -1;
    }
    res = pthread_mutex_init(&mutex_key_volume_down, NULL);
    if (res != 0) {
        printf("\tinitial mutex_key_volume_down failed\n");
        return -1;
    }
    res = pthread_mutex_init(&mutex_key_volume_up, NULL);
    if (res != 0) {
        printf("\tinitial mutex_key_volume_up failed\n");
        return -1;
    }
    res = pthread_create(&thread_scan_key_power, NULL, (void *)scan_key_power, NULL);
    if (res != 0) {
        printf("\tcreate thread : scan_key_power failed\n");
        return -1;
    }
    res = pthread_create(&thread_scan_key_volume, NULL, (void *)scan_key_volume, NULL);
    if (res != 0) {
        printf("\tcreate thread : scan_key_volume failed\n");
        return -1;
    }
    res = pthread_create(&thread_do_key_power, NULL, (void *)do_key_power, NULL);
    if (res != 0) {
        printf("\tcreate thread : do_key_power failed\n");
        return -1;
    }
    res = pthread_create(&thread_do_key_volume_down, NULL, (void *)do_key_volume_down, NULL);
    if (res != 0) {
        printf("\tcreate thread : do_key_volume_down failed\n");
        return -1;
    }
    res = pthread_create(&thread_do_key_volume_up, NULL, (void *)do_key_volume_up, NULL);
    if (res != 0) {
        printf("\tcreate thread : do_key_volume_up failed\n");
        return -1;
    }
    return 0;
}
