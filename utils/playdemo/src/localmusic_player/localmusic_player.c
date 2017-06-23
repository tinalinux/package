#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "c_tinaplayer.h"
#include "localmusic_player.h"
#include "voice.h"

#define SCAN_MUSIC_SCRIPT_PATH "/usr/share/playdemo_data/scan_music.sh"
#define GET_MUSIC_SCRIPT_PATH "/usr/share/playdemo_data/get_music.sh"
#define MUSIC_LIST_PATH "/usr/share/playdemo_data/music/music_list"
#define MUSIC_SUM_PATH "/usr/share/playdemo_data/music/music_sum"

#define MUSIC_PATH_LENGTH 200
static char g_music_path[MUSIC_PATH_LENGTH] = {0};
static int g_music_index = 0; //count start from 1, the index records the playing num of songs
static long g_music_sum = 0;
static pthread_mutex_t mutex_get_music;

/*==============================================================
* Function: LocalMusic_State
* Description:
*   play local music
* Parameter:
*   none
* Return:
*   none
*==============================================================*/
void LocalMusic_State(void)
{
    int res = 0;

    printf("\tnow in local_music state\n");
    say(VOICE_LOCALMUSIC_IN);
    sleep(1);
    res = LocalMusic_Scan();
    if (res == -1) {
        printf("\t%s: scan local music failed\n", __func__);
    }

    while(1){
        if (g_music_sum == 0) {
            printf("\tthere is no local music\n");
            sleep(1);
            res = LocalMusic_Scan();
            if (res == -1) {
                printf("\t%s: scan local music failed\n", __func__);
            }
        }
        else {
            g_music_index++;
            if (g_music_index > g_music_sum) {
                g_music_index = 1;
            }
            LocalMusic_Play(g_music_index);
            while(Tinaplayer_Get_Status() != STATUS_COMPLETED){
                sleep(1);
            }
        }
    }
}


/*==============================================================
* Function: LocalMusic_Init
* Description:
*   Initial local music player
* Parameter:
*   none
* Return:
*   -1: failed
*   0: successfully
*==============================================================*/
int LocalMusic_Init(void)
{
    g_music_index = 0;
    pthread_mutex_init(&mutex_get_music, NULL);
    return 0;
}

/*==============================================================
* Function: LocalMusic_Play
* Description:
*   start to play local music
* Parameter:
*   the num of local music to play in music_list
* Return:
*   -1: failed
*   0: successfully
*==============================================================*/
int LocalMusic_Play(const long num)
{
    int res = 0;

    res = LocalMusic_GetPath(g_music_index);
    if (res == -1) {
        return -1;
    }

    printf("\t>>>>>>>>>>>>>>>>>>play local music: %s>>>>>>>>>>>>>>>>>>\n", g_music_path);
    return Tinaplayer_Play(g_music_path);
}

/*==============================================================
* Function: LocalMusic_Stop
* Description:
*   stop local music
* Parameter:
*   none
* Return:
*   -1: failed
*   0: successfully
*==============================================================*/
int LocalMusic_Stop(void)
{
    return Tinaplayer_Stop();
}

/*==============================================================
* Function: LocalMusic_Pause
* Description:
*   pause local music
* Parameter:
*   none
* Return:
*   -1: failed
*   0: successfully
*==============================================================*/
int LocalMusic_Pause(void)
{
    return Tinaplayer_Pause();
}

/*==============================================================
* Function: LocalMusic_GetPath
* Description:
*   get specified row in music_list which is music path
*   the result is save in global variable music_path
* Parameter:
*   the specified row
* Return:
*   -1: failed
*   0: successfully
*==============================================================*/
int LocalMusic_GetPath(long row)
{
    FILE *file_tmp = NULL;
    char commond_tmp[100] = {0};
    int res = 0;

    pthread_mutex_lock(&mutex_get_music);
    sprintf(commond_tmp, "%s %ld", GET_MUSIC_SCRIPT_PATH, row);
    file_tmp = popen(commond_tmp, "r");
    if (file_tmp == NULL) {
        printf("\tpopen %s failed\n", GET_MUSIC_SCRIPT_PATH);
        pthread_mutex_unlock(&mutex_get_music);
        return -1;
    }

    memset(g_music_path, 0 , MUSIC_PATH_LENGTH);
    res = fscanf(file_tmp, "%s", g_music_path);
    if (res == -1) {
        printf("\t%s: read pope file failed\n", __func__);
        pthread_mutex_unlock(&mutex_get_music);
        return -1;
    }

    res = pclose(file_tmp);
    if (res == -1) {
        printf("\t%sclose pope file failed\n", __func__);
        pthread_mutex_unlock(&mutex_get_music);
        return -1;
    }

    pthread_mutex_unlock(&mutex_get_music);
    return 0;
}

/*==============================================================
* Function: LocalMusic_Next
* Description:
*   play next song
* Parameter:
*   none
* Return:
*   -1: failed
*   0: successfully
*==============================================================*/
int LocalMusic_Next(void)
{
    int res = 0;

    g_music_index++;
    if (g_music_index > g_music_sum) {
        g_music_index = 1;
    }

    res = LocalMusic_Play(g_music_index);
    if (res == -1) {
        printf("\tplay song failed\n");
        return -1;
    }

    return 0;
}

/*==============================================================
* Function: LocalMusic_Previous
* Description:
*   play prevoius song
* Parameter:
*   none
* Return:
*   -1: failed
*   0: successfully
*==============================================================*/
int LocalMusic_Previous(void)
{
    int res = 0;

    g_music_index--;
    if (g_music_index <= 0) {
        g_music_index = g_music_sum;
    }

    res = LocalMusic_Play(g_music_index);
    if (res == -1) {
        printf("\tplay song failed\n");
        return -1;
    }

    return 0;
}

/*==============================================================
* Function: LocalMusic_Scan
* Description:
*   scan songs files by shell script
* Parameter:
*   none
* Return:
*   -1: failed
*   0: successfully
*==============================================================*/
int LocalMusic_Scan(void)
{
    int res = 0;
    FILE *ftemp = NULL;
    res = system(SCAN_MUSIC_SCRIPT_PATH);
    if (res == -1) {
        printf("\t%s: execute scan_music.sh failed\n", __func__);
        return -1;
    }

    ftemp = fopen(MUSIC_SUM_PATH, "r");
    fscanf(ftemp, "%ld", &g_music_sum);
    fclose(ftemp);

    return 0;
}
