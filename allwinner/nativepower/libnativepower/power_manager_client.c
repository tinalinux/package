/*
 * Copyright (C) 2016 The AllWinner Project
 */
#include <stdio.h>
#include <stdlib.h>

#include "power_manager_client.h"
#include "dbus_client.h"

int PowerManagerSuspend(long event_uptime, SuspendReason reason)
{
	return dbus_client_invoke(dbus_client_open(), NATIVEPOWER_DEAMON_GOTOSLEEP, NULL, reason);
}

int PowerManagerShutDown(ShutdownReason reason)
{
	return dbus_client_invoke(dbus_client_open(), NATIVEPOWER_DEAMON_SHUTDOWN, NULL, reason);
}
int PowerManagerReboot(RebootReason reason)
{
	return dbus_client_invoke(dbus_client_open(), NATIVEPOWER_DEAMON_REBOOT, NULL, reason);
}

int PowerManagerAcquireWakeLock(const char * id)
{
	return dbus_client_invoke(dbus_client_open(), NATIVEPOWER_DEAMON_ACQUIRE_WAKELOCK, id, 0);
}

int PowerManagerReleaseWakeLock(const char *id)
{
	return dbus_client_invoke(dbus_client_open(), NATIVEPOWER_DEAMON_RELEASE_WAKELOCK, id, 0);
}

int PowerManagerUserActivity(UserActivityReason reason)
{
	return dbus_client_invoke(dbus_client_open(), NATIVEPOWER_DEAMON_USERACTIVITY, NULL, reason);
}

int PowerManagerSetAwakeTimeout(long timeout_s)
{
	char cAwakeTimeout[256];
	if(timeout_s > 0 && timeout_s < 5) {
		fprintf(stderr, "awake timeout must be larger than 5s!\n");
		return -1;
	}
    sprintf(cAwakeTimeout, "%ld", timeout_s);
	return dbus_client_invoke(dbus_client_open(), NATIVEPOWER_DEAMON_SET_AWAKE_TIMEOUT, cAwakeTimeout, 0);
}

int PowerManagerSetScene(NativePowerScene scene)
{
	return dbus_client_invoke(dbus_client_open(), NATIVEPOWER_DEAMON_SET_SCENE, NULL, scene);
}

