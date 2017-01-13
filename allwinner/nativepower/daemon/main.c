/*
 * Copyright (C) 2016 The AllWinner Project
 */



#define LOG_TAG "nativepower"

#include <tina_log.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#include "dbus_server.h"
#include "suspend/autosuspend.h"
#include "power.h"
#include "power_manager.h"
#include "np_input.h"
#include "scene_manager.h"

static pthread_t dbus_thread;

extern int set_sleep_state(unsigned int state);

void wakeup_callback(bool sucess) {
//	if (sucess) {
		set_sleep_state(0);
		acquireWakeLock(PARTIAL_WAKE_LOCK, NATIVE_POWER_DISPLAY_LOCK);
		invalidateActivityTime();
//	}
}



int main(int argc, char **argv) {
	int ret;
	char buf[80];

	acquireWakeLock(PARTIAL_WAKE_LOCK, NATIVE_POWER_DISPLAY_LOCK);
	autosuspend_enable();
    set_wakeup_callback(wakeup_callback);
    init_awake_timeout();

    np_input_init();

//    np_scene_change("boot_complete");

    ret = pthread_create(&dbus_thread, NULL, dbus_server_thread_func, NULL);
    if (ret)
    {
        strerror_r(ret, buf, sizeof(buf));
        goto end;
    }

    while (1)
    {
        if (!isActivityTimeValid())
        {
			goToSleep(100, 1, 0);
        }

        usleep(100000);
    }

end:
	autosuspend_disable();
	TLOGE(LOG_TAG, "EXIT ERROR!!!!");
}

