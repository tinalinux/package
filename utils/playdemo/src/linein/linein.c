#define TAG "linein"
#include <tina_log.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>
#include <string.h>
#include <pthread.h>
#include <dlfcn.h>

#include "main.h"
#include "linein.h"
#include "c_bluetooth.h"
#include "c_tinaplayer.h"
#include "c_wifimanager.h"
#include "voice.h"

static char *cmd[2] = {"amixer -c 0 cset iface=MIXER,name='Audio linein in' 0 &>/dev/null", "amixer -c 0 cset iface=MIXER,name='Audio linein in' 1 &>/dev/null"};
static char *isPlug_Path = "/sys/module/sun8iw5_sndcodec/parameters/AUX_JACK_DETECT";
static int fd = 0;
static pthread_t thread_linein_check;
static int is_linein_playing = 0;

/*=====================================================
* Function:mixer_set
* Description:set linein on or off
* Parameter:
*   1:set linein on
*   0:set linein off
* Return:
*   none;
*====================================================*/
void mixer_set(int mode)
{
    if(mode == 0 || mode ==1){
       is_linein_playing = mode;
       system(cmd[mode]);
       Tinaplayer_SetVolume(KEEP_VOLUME);
    }
}

/*=====================================================
* Function: Linein_isPlaying
* Description:
*   is linein playing
* Parameter:
*   none
* Return:
*   0: not playing
*   1: playing
*====================================================*/
int Linein_isPlaying(void)
{
    return is_linein_playing;
}

/*=====================================================
* Function: Linein_State
* Description:
*   linein state, run when
* Parameter:
*   none
* Return:
*   none;
*====================================================*/
void Linein_State(void)
{
    printf("\tnow in linein state\n");
    if (Get_MainState() == STATE_LINEIN) {
        say(VOICE_LINEIN_IN);//噔咚
        sleep(1);
        mixer_set(1);
    }
    while(1){
        sleep(1);
    }
}

/*====================================================
* Function: linein_Check
* Description:
*   check linein plug
* Parameter:
*   none
* Return:
*   none
*===================================================*/
static void linein_Check(void)
{
    int res;

    while(1){
        res = isPlugIn();
        if (res == 1 && Get_MainState() != STATE_LINEIN) {
            printf("\tlinein plug in\n");
            Set_MainState(STATE_LINEIN);
        }
        else if (res == 0 && Get_MainState() == STATE_LINEIN) {
            printf("\t\tlinein plug out\n");
            mixer_set(0);
            say(VOICE_LINEIN_OUT);//正在恢复联网，请稍等
            Set_MainState(STATE_WIFI);
        }
    }
}

/*====================================================
* Function: Linein_Init
* Description:
*   initialize linein and create linein pthread
* Parameter:
*   none
* Return:
*   -1: failed
*   0: successfully
*===================================================*/
int Linein_Init(void)
{
    int res;

    fd = open(isPlug_Path, O_RDONLY);
    if(fd < 0){
        printf("\tLinein_Init: open %s error!", isPlug_Path);
        return -1;
    }
    res = pthread_create(&thread_linein_check, NULL, (void *)linein_Check, NULL);
    if(res != 0){
        printf("\tcreate linein check thread failed\n");
        return -1;
    }
    return 0;
}


/*====================================================
* Function: isPlugIn
* Description:
*   to determine whether the plug in or not
* Parameter:
* Return:
*   -1: read nothing
*   0: linein out
*   1: linein in
*===================================================*/
int isPlugIn(void)
{
    char tmp;

    lseek(fd, 0, SEEK_SET);
    read(fd, &tmp, sizeof(tmp));
    if(tmp == 'Y'){
        return 1;
    }
    else if(tmp == 'N'){
        return 0;
    }
    return -1;
}
