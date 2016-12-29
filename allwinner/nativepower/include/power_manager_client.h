#ifndef _NATIVEPOWER_DAEMON_POWER_MANAGER_H_
#define _NATIVEPOWER_DAEMON_POWER_MANAGER_H_

#include "np_utils.h"

#if __cplusplus
extern "C"
{
#endif

int PowerManagerSuspend(long event_uptime, SuspendReason reason);
int PowerManagerShutDown(ShutdownReason reason);
int PowerManagerReboot(RebootReason reason);
int PowerManagerAcquireWakeLock(const char * id);
int PowerManagerReleaseWakeLock(const char *id);
int PowerManagerUserActivity(UserActivityReason reason);
int PowerManagerSetAwakeTimeout(long timeout_s);
int PowerManagerSetScene(NativePowerScene scene);


#if __cplusplus
} // extern "C"
#endif

#endif

