/*
 * Copyright (C) 2016 The AllWinner Project
 */

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "power.h"
#include "power_manager.h"
#include "np_uci_config.h"
#include "suspend/autosuspend.h"

#define PROP_SECTION_AWKE_TIMEOUT "awake_timeout"
#define AWAKE_TIMEOUT_TIME "time"

static pthread_mutex_t mLock = PTHREAD_MUTEX_INITIALIZER;
static time_t mLastUserActivityTime = -1;
static time_t mKeepAwakeTime = -1;

extern int set_sleep_state(unsigned int state);

int releaseWakeLock(const char* id)
{
    return release_wake_lock(id);
}

int acquireWakeLock(int lock, const char* id)
{
    return acquire_wake_lock(lock, id);
}


int goToSleep(long event_time_ms, int reason, int flags)
{
/*
*		EARLYSUSPEND_ON  = 0
*		EARLYSUSPEND_MEM = 1
*/
    if (get_wake_lock_count() <= 1)
    {
		set_sleep_state(1);
        releaseWakeLock(NATIVE_POWER_DISPLAY_LOCK);
#ifdef USE_TINA_SUSPEND
		tinasuspend_set_state(1);
		tinasuspend_wait_resume();
#endif
		return 0;
    }

    return -1;
}

int shutdown()
{
    int pid = fork();
    int ret = -1;

    if (pid == 0)
    {
        execlp("/sbin/poweroff", "poweroff", NULL);
    }

    return 0;
}

int reboot()
{
    int pid = fork();
    int ret = -1;

    if (pid == 0)
    {
        execlp("/sbin/reboot", "reboot", NULL);
    }

    return 0;
}

int userActivity()
{
    pthread_mutex_lock(&mLock);
    mLastUserActivityTime = time(NULL);
    pthread_mutex_unlock(&mLock);

    return 0;
}

int isActivityTimeValid()
{
    time_t timenow = time(NULL);

    if(mKeepAwakeTime < 0)
    {
          return 1;
    }

    if (timenow < mLastUserActivityTime || mLastUserActivityTime <= 0)
    {
        mLastUserActivityTime = timenow;
    }

	if((timenow - mLastUserActivityTime) < mKeepAwakeTime) {
		return 1;
	} else {
		mLastUserActivityTime = 0;
		return 0;
	}
}

int invalidateActivityTime()
{
    mLastUserActivityTime = -1;

    return 0;
}

int init_awake_timeout()
{
    char cAwakeTimeout[256];
    time_t awakeTimeout = -1;
    int ret = -1;

    NP_UCI *uci = np_uci_open(NATIVE_POWER_CONFIG_PATH);

    memset(cAwakeTimeout, 0 , sizeof(cAwakeTimeout));

    ret = np_uci_read_config(uci, PROP_SECTION_AWKE_TIMEOUT, AWAKE_TIMEOUT_TIME, cAwakeTimeout);
    if (ret < 0)
    {
        goto end;
    }

    awakeTimeout = atoll(cAwakeTimeout);

    if (awakeTimeout > 0)
    {
        pthread_mutex_lock(&mLock);
        mKeepAwakeTime = awakeTimeout;
        pthread_mutex_unlock(&mLock);
        ret =0;
    }

end:
    np_uci_close(uci);
    return ret;
}

int set_awake_timeout(long timeout_s)
{
    char cAwakeTimeout[256];
    int ret = -1;

	pthread_mutex_lock(&mLock);
	mKeepAwakeTime = (time_t) timeout_s;
	pthread_mutex_unlock(&mLock);
	sprintf(cAwakeTimeout, "%ld", timeout_s);
	NP_UCI *uci = np_uci_open(NATIVE_POWER_CONFIG_PATH);
	ret = np_uci_write_config(uci, PROP_SECTION_AWKE_TIMEOUT, AWAKE_TIMEOUT_TIME, cAwakeTimeout);
	np_uci_close(uci);

    return ret;
}
