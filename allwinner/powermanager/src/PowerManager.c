#define TAG "PowerManager"
#include <tina_log.h>

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <dirent.h>
#include <sys/select.h>
#include <dlfcn.h>
#include <sys/epoll.h>
#include <stdbool.h>

#include "PowerManager.h"

#define SUSPEND_SYS_POWER_STATE	"/sys/power/state"
#define SUSPEND_SYS_POWER_WAKE_LOCK		"/sys/power/wake_lock"
#define SUSPEND_SYS_POWER_WAKE_UNLOCK		"/sys/power/wake_unlock"

static int sPowerStatefd;
static int sWakeLockfd;
static int sWakeLock_fd_init = 0;
static int sWakeUnlockfd;

//static struct autosuspend_ops *autosuspend_ops;
static const char *pwr_state_mem = "mem";
static const char *pwr_state_on = "on";
#define LENGTH 100
#define MAX 10
static char *lock[LENGTH];
static int lock_count = 0;

//#define MSEC_PER_SEC            (10LL)
#define MSEC_PER_SEC            (1000LL)
#define NSEC_PER_MSEC           (1000000LL)
#define WAKE_LOCK_TIMEOUT       (60 * NSEC_PER_MSEC * MSEC_PER_SEC)
//60sec

static bool suspend_inited;
static int g_initialized ;

int suspend_init(void){
    char buf[80];
    int ret;

    int cnt = 0;

    lock_count = 0;
    if (suspend_inited) {
         return 0;
    }

    sPowerStatefd = open(SUSPEND_SYS_POWER_STATE, O_RDWR);
    if(sPowerStatefd < 0){
        strerror_r(errno, buf, sizeof(buf));
        printf("Error opening %s: %s\n",SUSPEND_SYS_POWER_STATE, buf);
        return -1;
     }

    ret = write(sPowerStatefd, "on", 2);
    if(ret < 0){
        strerror_r(errno, buf, sizeof(buf));
        printf("Error writing 'on' to %s: %s\n",SUSPEND_SYS_POWER_STATE, buf);
        goto err_write;
    }

    suspend_inited = true;

    for (cnt = 0; cnt < LENGTH; cnt++) {
        lock[cnt] = (char *)malloc(40 * sizeof(char));
        memset(lock[cnt], 0 , 40);
    }
    sWakeLockfd = open(SUSPEND_SYS_POWER_WAKE_LOCK, O_RDWR);
    if(sWakeLockfd < 0){
        strerror_r(errno, buf, sizeof(buf));
        TLOGE("Error opening %s: %s\n",SUSPEND_SYS_POWER_WAKE_LOCK, buf);
        return -1;
    }
    sWakeLock_fd_init = 1;

    sWakeUnlockfd = open(SUSPEND_SYS_POWER_WAKE_UNLOCK, O_RDWR);
    if(sWakeUnlockfd < 0){
        strerror_r(errno, buf, sizeof(buf));
        TLOGE("Error opening %s: %s\n",SUSPEND_SYS_POWER_WAKE_UNLOCK, buf);
        return -1;
    }

    return 0;

err_write:
    close(sPowerStatefd);
    return -1;
}



int acquire_wake_lock(const char* id)
{
    int ret;
    char buf[80];
    int i=0;


    if(lock_count == 0){
        strncpy(lock[lock_count], id, strlen(id) > 99 ? 99 : strlen(id));
        lock_count++;
    }else{
        while(i<lock_count){
            if (!strcmp(lock[i], id)) {
                TLOGI("lock %s is already here\n", id);
                return -1;   //already wrote a same id in to wake_lock
            }

		i++;
	}

        strncpy(lock[lock_count], id, strlen(id) > 99 ? 99 : strlen(id));
        lock_count++;
    }


    return write(sWakeLockfd, lock[lock_count-1], strlen(lock[lock_count-1]));

}


int release_wake_lock(const char* id)
{

    int ret = 0;

    char buf[80];
    int j=0;


    if(lock_count==0){
        TLOGE("there has no wake_lock,u don't have to write wake_unlock\n");
        return -1;
    }else{
        while(j < MAX){
            if(lock[j] != NULL && !strcmp(lock[j],id)){
                memset(lock[j], 0 , strlen(lock[j]));
                lock_count=lock_count - 1;

                ret=write(sWakeUnlockfd, id, strlen(id));
                if(ret < 0){
                    TLOGE("Error in release_wake_lock to write wake_unlock\n");
                    return -1;
                }

                break;
            }
            j++;
        }

        if (j == MAX) {
            printf("no match string\n");
            return -1;
        }
    }

    return 0;
}

int get_lock_count(void){
	TLOGI("get_lock_count lock_count=%d\n",lock_count);
	return lock_count;
}

static int check_wake_lock(void){

	char buf[256];
	int ret;

	if ((ret = read(sWakeLockfd, buf, sizeof(buf))) < 0) {
        TLOGE("err:read %s  fail in getvalue\n",buf);
        return -1;
    }
    if (!strcmp(buf, "0")){
        return 0;
    }

    return -1;
}


int acquire_wake_lock_timeout(const char* id){
    int fd,ret;
    char str[64];
    char buf[80];
    if(sWakeLock_fd_init != 1){
        sWakeLockfd = open(SUSPEND_SYS_POWER_WAKE_LOCK, O_RDWR);
        if(sWakeLockfd < 0){
            strerror_r(errno, buf, sizeof(buf));
            TLOGE("Error opening %s: %s\n",SUSPEND_SYS_POWER_WAKE_LOCK, buf);
            return -1;
        }
    }

    sprintf(str,"%s_timeout %lld",id,WAKE_LOCK_TIMEOUT);
    TLOGI("acquire_wake_lock_timeout = %s\n",str);
    ret = write(sWakeLockfd, str, strlen(str));
    return ret;
}

int suspend_enable(void){
    char buf[80];
    int ret;

    ret = write(sPowerStatefd, pwr_state_mem, strlen(pwr_state_mem));
    if(ret < 0){
        strerror_r(errno, buf, sizeof(buf));
        TLOGE("Error writing to %s: %s\n",SUSPEND_SYS_POWER_STATE,buf);
        return -3;
    }

    return 0;
}

int suspend_disable(void){
    char buf[80];
    int ret;

    ret = write(sPowerStatefd, pwr_state_on, strlen(pwr_state_on));
    if(ret < 0){
        strerror_r(errno, buf, sizeof(buf));
        TLOGE("Error writing to %s: %s\n",SUSPEND_SYS_POWER_STATE,buf);
        return -1;
    }

    return 0;
}
