#define TAG "c_bluetooth"

#include <stdio.h>
#include <stdlib.h>
#include <bluetooth_socket.h>
#include <tina_log.h>

#ifndef CPP

#define CPP
#include "c_bluetooth.h"
#include "c_tinaplayer.h"
extern "C"
{
#include "main.h"
#include "voice.h"
}

#endif

c_bt c;

typedef void (tBtCallback)(BT_EVENT event, void *reply, int *len);
static struct {
    int isConnected;
    int isPlaying;
    int isOn;
    int isInited;
}bluetooth_status;

/*=====================================================
* Function: Bluetooth_isConnected
* Description:
*   determine bt connectting or not
* Parameter:
*   none
* Return:
*   0: not connected
*   1: connected
*====================================================*/
int Bluetooth_isConnected(void)
{
    return bluetooth_status.isConnected;
}

/*=====================================================
* Function: Bluetooth_isOn
* Description:
*   determine bt on or not
* Parameter:
*   none
* Return:
*   0: not on
*   1: have turned on
*====================================================*/
int Bluetooth_isOn(void)
{
    return bluetooth_status.isOn;
}

/*=====================================================
* Function: Bluetooth_isPlaying
* Description:
*   determine bt playing or not
* Parameter:
*   none
* Return:
*   0: not playing
*   1: playing
*====================================================*/
int Bluetooth_isPlaying(void)
{
    return bluetooth_status.isPlaying;
}

/*=====================================================
* Function: Bluetooth_Reset_Avk_Status
* Description:
*   ?????????????????????????
* Parameter:
*   none
* Return:
*   none
*====================================================*/
int Bluetooth_Reset_Avk_Status(void)
{
    return c.reset_avk_status();
}

/*=====================================================
* Function: Bluetooth_Disconnect
* Description:
*   disconnect
* Parameter:
*   none
* Return:
*   0: successfully
*   else: faild
*====================================================*/
int Bluetooth_Disconnect(void)
{
    return c.disconnect();
}

/*=====================================================
* Function: Bluetooth_On
* Description:
*   turn on bluetooth
* Parameter:
*   bt_addr: the file recording bt's MAC address,NULL means using random address
* Return:
*   0: successfully
*   else: faild
*====================================================*/
int Bluetooth_On(void)
{
    bluetooth_status.isOn = 1;
    return c.bt_on(NULL);
}

/*=====================================================
* Function: Bluetooth_Off
* Description:
*   turn off bluetooth
* Parameter:
*   none
* Return:
*   0: successfully
*   else: faild
*====================================================*/
int Bluetooth_Off(void)
{
    bluetooth_status.isOn = 0;
    bluetooth_status.isPlaying = 0;
    bluetooth_status.isConnected = 0;
    return c.bt_off();
}

/*=====================================================
* Function: Bluetooth_Set_Name
* Description:
*   set bt's name which will known by scaner
* Parameter:
*   the pointer to string of bt'name
* Return:
*   0: successfully
*   else: faild
*====================================================*/
int Bluetooth_Set_Name(const char *bt_name)
{
    c.set_bt_name(bt_name);
}

/*=====================================================
* Function: Bluetooth_Set_Device_Discoverable
* Description:
*   determine that whether bt can be discovered
* Parameter:
*   0: can not be discovered
*   1: can be discovered
* Return:
*   0: successfully
*   else: faild
*====================================================*/
int Bluetooth_Set_Device_Discoverable(int enable)
{
    return c.set_dev_discoverable(enable);
}

/*=====================================================
* Function: Bluetooth_Set_Device_Connectable
* Description:
*   determine that whether bt can be connected
* Parameter:
*   0: can not be connected
*   1: can be connected
* Return:
*   0: successfully
*   else: faild
*====================================================*/
int Bluetooth_Set_Device_Connectable(int enable)
{
    return c.set_dev_connectable(enable);
}

/*=====================================================
* Function: Bluetooth_Auto_Connect
* Description:
*   make bt to connect automatically
* Parameter:
*   none
* Return:
*   0: successfully
*   else: faild
*====================================================*/
int Bluetooth_Auto_Connect(void)
{
    return c.connect_auto();
}

/*=====================================================
* Function: Bluetooth_Avk_Play
* Description:
*   to play music
* Parameter:
*   none
* Return:
*   0: successfully
*   else: faild
*====================================================*/
int Bluetooth_Avk_Play(void)
{
    return c.avk_play();
}

/*=====================================================
* Function: Bluetooth_Avk_Pause
* Description:
*   to pause music
* Parameter:
*   none
* Return:
*   0: successfully
*   else: faild
*====================================================*/
int Bluetooth_Avk_Pause(void)
{
    return c.avk_pause();
}

/*=====================================================
* Function: Bluetooth_Avk_Previous
* Description:
*   to play prevoius music
* Parameter:
*   none
* Return:
*   0: successfully
*   else: faild
*====================================================*/
int Bluetooth_Avk_Previous(void)
{
    return c.avk_previous();
}

/*=====================================================
* Function: Bluetooth_Avk_Next
* Description:
*   to play next music
* Parameter:
*   none
* Return:
*   0: successfully
*   else: faild
*====================================================*/
int Bluetooth_Avk_Next(void)
{
    return c.avk_next();
}

/*=====================================================
* Function: bluetooth_Set_Callback
* Description:
*   set callback function
* Parameter:
*   the pointer to callback function
* Return:
*   none
*====================================================*/
static void bluetooth_Set_Callback(tBtCallback *pCb)
{
    c.set_callback(pCb);
}

/*=====================================================
* Function: bluetooth_Callback
* Description:
*   the bt's callback function
* Parameter:
*   the event of bt
* Return:
*   none
*====================================================*/
static void bluetooth_Callback(BT_EVENT event, void *reply, int *len)
{
    char tmp[100];
    switch(event)
    {
        case BT_AVK_CONNECTED_EVT:
        {
            printf("\tCallback Note: Bluetooth connected!\n");
            c.set_dev_discoverable(0);
            c.set_dev_connectable(0);
            bluetooth_status.isConnected = 1;
            Set_MainState(STATE_BLUETOOTH);
            break;
        }

        case BT_AVK_DISCONNECTED_EVT:
        {
            printf("\tCallback Note: Bluetooth disconnected\n");
            c.set_dev_connectable(1);
            c.set_dev_discoverable(1);
            bluetooth_status.isConnected = 0;
            bluetooth_status.isPlaying = 0;
            Set_MainState(STATE_WIFI);
            break;
        }

        case BT_AVK_START_EVT:
        {
            Tinaplayer_SetVolume(KEEP_VOLUME);
            printf("\tCallback Note: Bluetooth start playing!\n");
            bluetooth_status.isPlaying = 1;
            break;
        }

        case BT_AVK_STOP_EVT:
        {
            printf("\tCallback Note: Bluetooth stop playing!\n");
            bluetooth_status.isPlaying = 0;
            break;
        }

        default:
            break;
    }
}

/*=====================================================
* Function: Bluetooth_Init
* Description:
*   set callback function and initialize bt status
* Parameter:
*   none
* Return:
*   0: successfully
*   else: faild
*====================================================*/
int Bluetooth_Init(void)
{
    int res = 0;
    bluetooth_status.isConnected = 0;
    bluetooth_status.isPlaying = 0;
    bluetooth_status.isOn = 0;
    bluetooth_Set_Callback(bluetooth_Callback);

    res = Bluetooth_On();
    if (res != 0){
        printf("\tTurn on bluetooth failed\n");
        return res;
    }
    printf("\tTrun on bluetooth sucessfully\n");

    res = Bluetooth_Set_Name("bt-test-allwinnertech");
    if (res != 0){
        printf("\tSet bluetooth name failed\n");
        return res;
    }
    printf("\tset bluetooth name sucessfully\n");

    bluetooth_status.isInited == 1;
    return 0;
}

/*=====================================================
* Function: Bluetooth_State
* Description:
*   this function is runned while in bleutooth state and called by swtich_state
* Parameter:
*   none
* Return:
*   none
*====================================================*/
void Bluetooth_State(void)
{
    printf("\tnow in bluetooth state\n");
    say(VOICE_BLUETOOTH_IN);
    sleep(2);
    while(1){
        sleep(2);
    }
}
