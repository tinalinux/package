#ifndef POWERMANAGER_H
#define POWERMANAGER_H



 int suspend_init(void);
int acquire_wake_lock(const char* id);
int release_wake_lock(const char* id);
int acquire_wake_lock_timeout(const char* id);
int get_lock_count(void);
/*
 * autosuspend_enable
 *
 * Turn on autosuspend in the kernel, allowing it to enter suspend if no
 * wakelocks/wakeup_sources are held.
 *
 *
 *
 * Returns 0 on success, -1 if autosuspend was not enabled.
 */
int suspend_enable(void);

/*
 * autosuspend_disable
 *
 * Turn off autosuspend in the kernel, preventing suspend and synchronizing
 * with any in-progress resume.
 *
 * Returns 0 on success, -1 if autosuspend was not disabled.
 */
int suspend_disable(void);


//int autosuspend_bootfast(void);
#endif // POWERMANAGER_H