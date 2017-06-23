#include <stdio.h>
#include <PowerManager.h>

#define CPP
#include "c_powermanager.h"

#define LOCKNAME "playdemo"

/*===================================================
* Function: Power_Init
* Descritpion:
*   Initialize power manager
* Parameter:
*   none
* Return:
*   -1: failed
*   0: successfully
*==================================================*/
int Power_Init(void)
{
    int res = 0;
    res = suspend_init();
    if (res != 0) {
        printf("\tpower manager, suspend init falied\n");
        return -1;
    }

    res = Power_Lock();
    if (res == -1) {
        printf("\tpower lock: playdemo failed\n");
        return -1;
    }

    res = suspend_enable();
    if (res != 0) {
        printf("\tenable power manager failed\n");
        return -1;
    }
    return 0;
}

/*===================================================
* Function: Power_Lock_TimeCnt
* Descritpion:
*   Initialize power manager
* Parameter:
*   none
* Return:
*   -1: failed
*   0: successfully
*==================================================*/
int Power_Lock_TimeCnt(void)
{
    return acquire_wake_lock_timeout(LOCKNAME);
}

/*===================================================
* Function: Power_Lock
* Descritpion:
*   make lock
* Parameter:
*   none
* Return:
*   -1: failed
*   0: successfully
*==================================================*/
int Power_Lock(void)
{
    return acquire_wake_lock(LOCKNAME);
}

/*===================================================
* Function: Power_Get_LockCnt
* Descritpion:
*   get the count of lock but not timecnt lock
* Parameter:
*   none
* Return:
*   the count of lock
*==================================================*/
int Power_Get_LockCnt(void)
{
    return get_lock_count();
}

/*===================================================
* Function: Power_Unlock
* Descritpion:
*   unlock power
* Parameter:
*   none
* Return:
*   -1: failed
*   0: successfully
*==================================================*/
int Power_Unlock(void)
{
    return release_wake_lock(LOCKNAME);
}
