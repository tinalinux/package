#define TAG "healthd"
#include <tina_log.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <linux/input.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>
#include "minui.h"
#include <sys/socket.h>
#include <linux/netlink.h>

//#include <batteryservice/BatteryService.h>
#include <cutils/android_reboot.h>
#include <cutils/klog.h>
#include <cutils/misc.h>
#include <cutils/uevent.h>
#include <cutils/properties.h>

#include <pthread.h>
#include <sys/time.h>
#include "android_alarm.h"

/*
#ifdef CHARGER_ENABLE_SUSPEND
#include <suspend/autosuspend.h>
#endif
*/

//#include <minui/minui.h>
#include "android_alarm.h"
#include "healthd.h"

void healthd_mode_tina_battery_update(struct BatteryProperties *props)
{
	//printf("healthd_mode_tina_battery_update\n");
}
int healthd_mode_tina_preparetowait(void) {
	//printf("healthd_mode_tina_preparetowait\n");
    return -1;
}
void healthd_mode_tina_init(struct healthd_config* /*config*/) {
	//printf("healthd_mode_tina_init\n");
}
