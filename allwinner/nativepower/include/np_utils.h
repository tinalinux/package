#ifndef _NP_UTILS_H_
#define _NP_UTILS_H_

#if __cplusplus
extern "C"
{
#endif

typedef enum
{
    SUSPEND_APPLICATION,
    SUSPEND_DEVICE_ADMIN,
    SUSPEND_TIMEOUT,
    SUSPEND_LID_SWITCH,
    SUSPEND_POWER_BUTTON,
    SUSPEND_HDMI,
    SUSPEND_SLEEP_BUTTON,
} SuspendReason;

typedef enum
{
    SHUTDOWN_DEFAULT, SHUTDOWN_USER_REQUESTED,
} ShutdownReason;

typedef enum
{
    REBOOT_DEFAULT, SHUTDOWN_RECOVERY,
} RebootReason;

typedef enum
{
    USERACTIVITY_DEFAULT,
} UserActivityReason;

typedef enum
{
    CALLBACK_SUSPEND,
} CallbackReason;

typedef enum
{
    NATIVEPOWER_SCENE_BOOT_COMPLETE,
    NATIVEPOWER_SCENE_MAX,
} NativePowerScene;

#if __cplusplus
} // extern "C"
#endif

#endif
