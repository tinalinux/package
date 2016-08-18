#define TAG "c_tinaplayer"
#include <tina_log.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <errno.h>

#include <allwinner/tinaplayer.h>

#ifndef CPP
#define CPP
#include "c_tinaplayer.h"
#include "c_bluetooth.h"
#include "main.h"
#include "voice.h"
#endif

static int isPlayingPromptTone = 0;

static int isContinuePlaying = 0;
static int isSeekCompleted = 0;
static char *last_song = 0;
static int last_song_seek_ms = 0;

using namespace aw;

static PlayerStatus tinaplayer_status = STATUS_STOPPED;
static pthread_t thread_start_tinaplayer;
static pthread_mutex_t mutex_start_tinaplayer;
static pthread_mutex_t mutex_set_status;
static pthread_mutex_t mutex_say_wait;
static pthread_mutex_t mutex_set_volume;
static TinaPlayer *pTinaPlayer;
static int volume = 40;
static int is_inited = 0;

/*==============================================================
* Function: Tinaplayer_Get_Status
* Description:
*   get value of tinaplayer_status
* Parameter:
*   none
* Return:
*   -1: have not initialized
*   0: STATUS_STOPPED
*   1: STATUS_PREPARING
*   2: STATUS_PREPARED
*   3: STATUS_PLAYING
*   4: STATUS_PAUSED
*   5: STATUS_COMPLETED
*   6: STATUS_SEEKING
*==============================================================*/
PlayerStatus Tinaplayer_Get_Status(void)
{
    return tinaplayer_status;
}

/*==============================================================
* Function: Tinaplayer_Set_Status
* Description:
*   set value of tinaplayer_status
* Parameter:
*   0: STATUS_STOPPED
*   1: STATUS_PREPARING
*   2: STATUS_PREPARED
*   3: STATUS_PLAYING
*   4: STATUS_PAUSED
*   5: STATUS_COMPLETED
*   6: STATUS_SEEKING
* Return:
*   -1: have not initialized
*   0: successfully
*==============================================================*/
int Tinaplayer_Set_Status(PlayerStatus status)
{
    if (is_inited == 0) {
        Tinaplayer_Init();
    }
    pthread_mutex_lock(&mutex_set_status);
    tinaplayer_status = status;
    pthread_mutex_unlock(&mutex_set_status);
    return 0;
}

/*==============================================================
* Function: Tinaplayer_Continue
* Description:
*   continue playing
* Parameter:
*   none
* Return:
*   0:successfully
*   -1:failed
*==============================================================*/
int Tinaplayer_Continue(void)
{
    isContinuePlaying = 1;
    return Tinaplayer_Play(last_song);
}

/*==============================================================
* Function: Tinaplayer_Stop
* Description:
*   stop playing. use it only when playing,pausing or complete
* Parameter:
*   none
* Return:
*   0:successfully
*   -1:failed
*==============================================================*/
int Tinaplayer_Stop(void)
{
    pTinaPlayer->getCurrentPosition(&last_song_seek_ms);
    return pTinaPlayer->stop();
}

/*==============================================================
* Function: Tinaplayer_Pause
* Description:
*   pause playing. use it only when playing
* Parameter:
*   none
* Return:
*   0:successfully
*   -1:failed
*==============================================================*/
int Tinaplayer_Pause(void)
{
    if (isPlayingPromptTone == 1) {
        printf("\tis playing prompt tone, can't stop\n");
        return 0;
    }
    return pTinaPlayer->pause();
}

/*==============================================================
* Function: Tinaplayer_IsPlaying
* Description:
*   determine whether or not playing
* Parameter:
*   none
* Return:
*   1:playing
*   0:not playing
*==============================================================*/
int Tinaplayer_IsPlaying(void)
{
    return pTinaPlayer->isPlaying();
}

/*==============================================================
* Function: Tinaplayer_Play
* Description:
*   play music
* Parameter:
*   the music file path.it can also be a url
* Return:
*   0:successfully
*   -1:failed
*==============================================================*/
int Tinaplayer_Play(const char *path)
{
    int res = 0;
    if(is_inited == 0){
        printf("\t>>>>>>>>>>>>>>>>>>>>>>init tinaplayer now>>>>>>>>>>>>>>>>>>>>>>>>\n");
        res = Tinaplayer_Init();
        if(res != 0){
            printf("\tinit tinaplayer failed\n");
            return -1;
        }
    }
    if (Bluetooth_isConnected() == 1 && Get_MainState() != STATE_BLUETOOTH) {
        if (Bluetooth_isPlaying() == 1) {
            printf("\tbluetooth is playing , pause!!\n");
            Bluetooth_Avk_Pause();
        }
        printf("\tbluetooth is connecting, stop!\n");
        res = Bluetooth_Disconnect();
        if (res != 0) {
            printf("\tin %s-%s : disconnect bluetooth failed\n", __FILE__, __func__);
            return -1;
        }
        for (res = 0; res < 8 && Bluetooth_isConnected() == 1; res++) {
            printf("\twait for disconnecting bluetooth : %d\n", res);
            sleep(1);
        }
    }
    printf("\t>>>>>>>>>>>>>>>>>>>>>>reset tinaplayer now>>>>>>>>>>>>>>>>>>>>>>>>\n");
    res = pTinaPlayer->reset();
    if(res == -1){
        printf("\treset tinaplayer failed\n");
        return -1;
    }
    if (isPlayingPromptTone != 0) {
        isPlayingPromptTone--;
    }

    Tinaplayer_Set_Status(STATUS_STOPPED);

    printf("\t>>>>>>>>>>>>>>>>>>>>>>set tinaplayer data source now>>>>>>>>>>>>>>>>>>>>>>>>\n");
    res =  pTinaPlayer->setDataSource(path, NULL);
    if(res == -1){
        printf("\tset tinaplayer data source failed\n");
        return -1;
    }

    printf("\t>>>>>>>>>>>>>>>>>>>>>>prepare tinaplayer Asynchronously now>>>>>>>>>>>>>>>>>>>>>>>>\n");
    res = pTinaPlayer->prepareAsync();
    if(res == -1){
        printf("\tprepare tinaplayer Asynchronously failed\n");
        return -1;
    }

    last_song = (char *)path;
    return 0;
}

/*==============================================================
* Function: Tinaplayer_SetVolume
* Description:
*   Set volume of tinaplayer
* Parameter:
*   UP_VOLUME(2): volume ++
*   KEEP_VOLUME(1): keep volume
*   DOWN_VOLUME(0): volume --
* Return:
*   0: set successfully
*   -1: set failed
*=============================================================*/
int Tinaplayer_SetVolume(int mode)
{
    char tmp[100];
    switch(mode){
        case UP_VOLUME:
        {
            if(volume < MAX_VOLUME){
                pthread_mutex_lock(&mutex_set_volume);
                volume+=2;
                pthread_mutex_unlock(&mutex_set_volume);
            }
            break;
        }
        case KEEP_VOLUME:
        {
            break;
        }
        case DOWN_VOLUME:
        {
            if(volume > MIN_VOLUME){
                pthread_mutex_lock(&mutex_set_volume);
                volume-=2;
                pthread_mutex_unlock(&mutex_set_volume);
            }
            break;
        }
    }
    printf("\tSet volume to %d\n", volume);
    sprintf(tmp, "amixer cset numid=3 %d", volume);
    system(tmp);
    return 0;
}

/*==============================================================
* Function: Tinaplayer_GetVolume
* Description:
*   get volume of tinaplayer
* Parameter:
*   none
* Return:
*   volume value
*=============================================================*/
int Tinaplayer_GetVolume(void)
{
    return volume;
}

/*==============================================================
* Function: Tinaplayer_Start
* Description:
*   to unlock the mutex to start to play
*   use this function only after pausing or stop tinaplayer
* Parameter:
*   none
* Return:
*   none
*=============================================================*/
void Tinaplayer_Start(void)
{
    pthread_mutex_unlock(&mutex_start_tinaplayer);
}

/*==============================================================
* Function: tinaplayer_Start
* Description:
*   to start play after prepared
*   this function will run in a new thread which to achive start to play after prepared
*   be carefully, this function is diffrent from Tinaplayer_Start which is only to unlock mutex to run function tinaplayer_Start
* Parameter:
*   none
* Return:
*   none
*=============================================================*/
static void *tinaplayer_Start(void *para)
{
    int res = 0;
    int timecnt_250ms = 0;
    pthread_mutex_lock(&mutex_start_tinaplayer);
    while(1){
        pthread_mutex_lock(&mutex_start_tinaplayer);
        //system("amixer cset numid=27 2");
        printf("\t>>>>>>>>>>>>>>>>>>>>>>start tinaplayer now>>>>>>>>>>>>>>>>>>>>>>>>\n");
        if (isContinuePlaying == 1) {
//            printf("\tnow to seek to last song time: %dms\n", last_song_seek_ms);
//            pTinaPlayer->seekTo(last_song_seek_ms);
//            isContinuePlaying = 0;
//            timecnt_250ms = 0;
//            while (isSeekCompleted == 0 && timecnt_250ms <= 3 * 4){
//                timecnt_250ms++;
//                printf("\twait for seek completed\n");
//                usleep(1000 * 250);
//            }
//            if (timecnt_250ms > 3 * 4) {
//                printf("\tseek failed\n");
//            }
        }
        res = pTinaPlayer->start();
        if(res == -1){
            printf("\tstart tinaplayer failed\n");
            continue;
        }
        Tinaplayer_Set_Status(STATUS_PLAYING);
    }
}

/*==============================================================
* Function: tinaplayer_Callback
* Description:
*   callback function of tinaplayer
* Parameter:
*   pUserData:....
*   msg:event flag of tinaplayer
*   param0:....
*   param1:....
* Return:
*   0:successfully
*   -1:failed
*==============================================================*/
static void tinaplayer_Callback(void* pUserData, int msg, int param0, void* param1)
{
    switch(msg){
        case TINA_NOTIFY_NOT_SEEKABLE:
        {
            printf("\tmedia source is unseekable.\n");
            break;
        }

        case TINA_NOTIFY_ERROR:
        {
            printf("\topen media source fail.\n");
            Tinaplayer_Set_Status(STATUS_STOPPED);
            break;
        }

        case TINA_NOTIFY_PREPARED:
        {
            printf("\tmedia is prepared\n");
            Tinaplayer_Set_Status(STATUS_PREPARED);
            pthread_mutex_unlock(&mutex_start_tinaplayer);
            break;
        }

        case TINA_NOTIFY_BUFFERRING_UPDATE:
        {
            break;
        }

        case TINA_NOTIFY_PLAYBACK_COMPLETE:
        {
            printf("\tmedia playing completed\n");
            if (isPlayingPromptTone != 0) {
                isPlayingPromptTone = 0;
            }
            Tinaplayer_Set_Status(STATUS_COMPLETED);
            break;
        }

        case TINA_NOTIFY_RENDERING_START:
        {
            break;
        }

        case TINA_NOTIFY_SEEK_COMPLETE:
        {
            printf("\tmedia seed completed\n");
            isSeekCompleted = 1;
            break;
        }

        case TINA_NOTIFY_VIDEO_PACKET:
        {
		break;
        }

        case TINA_NOTIFY_AUDIO_PACKET:
        {
            break;
        }

        default:
        {
            printf("\twarning: unknown callback from Tinaplayer.\n");
            break;
        }
    }
}

/*==============================================================
* Function: Tinaplayer_Init
* Description:
*   use it to set callbackfunc,tina speak
* Parameter:
*   none
* Return:
*   0:successfully
*   -1:failed
*==============================================================*/
int Tinaplayer_Init(void)
{
    int res;
    pTinaPlayer = new TinaPlayer();
    is_inited = 1;
    res = pthread_mutex_init(&mutex_say_wait, NULL);
    if (res != 0) {
        printf("\tinitial mutex_say_wait failed\n");
        return -1;
    }
    res = pthread_mutex_init(&mutex_start_tinaplayer, NULL);
    if (res != 0) {
        printf("\tinitial mutex_start_tinaplayer failed\n");
        return -1;
    }
    res = pthread_mutex_init(&mutex_set_volume, NULL);
    if (res != 0) {
        printf("\tinitial mutex_set_volume failed\n");
        return -1;
    }
    res = pthread_create(&thread_start_tinaplayer, NULL, tinaplayer_Start, NULL);
    if (res != 0) {
        printf("\tcreate thread of tinaplayer_Start failed\n");
        return -1;
    }
    Tinaplayer_SetVolume(KEEP_VOLUME);
    return pTinaPlayer->setNotifyCallback(tinaplayer_Callback, NULL);
}

/*===========================================================================
* Function: say
* Description:
*   play music file, just need filename, and the path default is VOICE_PATH
*   use this function only when to play prompt tone
* Parameter:
*   filename: the music file name
* Return:
*   none
*===========================================================================*/
void say(const char *fname)
{
    char ftmp[100] = {0};
    isPlayingPromptTone = 2;
    sprintf(ftmp, "%s/%s", VOICE_PATH, fname);
    Tinaplayer_Play(ftmp);
}
