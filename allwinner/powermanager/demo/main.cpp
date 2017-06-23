#include <PowerManager.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <linux/input.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TARGET "power-lock"
#define POWER_KEY_PATH "/dev/input/event0"
#define VOLUME_KEY_PATH "/dev/input/event1"

static pthread_t thread_key_power;
static pthread_t thread_key_volume;
static int key_power_fd = 0;
static int key_volume_fd = 0;
static struct input_event key_power_event;
static struct input_event key_volume_event;
static int simple_lock_cnt = 0;
static int delay_lock_cnt = 0;

void *scanPowerKey(void *arg);
void *scanVolumeKey(void *arg);
int Lock_timeout(void);
int Lock(void);
int Unlock(void);

/*=============================================================
* Function: Power_Init
* Descriptions:
*   create thread of scaning power and set app lock
* Parameters:
*   none
* Return:
*   -1: failed
*   0: successfully
*============================================================*/
static int Power_Init(void)
{
    int res = 0;

    key_power_fd = open(POWER_KEY_PATH, O_RDONLY);
    if (key_power_fd < 0) {
        printf("\topen %s failed\n", POWER_KEY_PATH);
        return -1;
    }

    key_volume_fd = open(VOLUME_KEY_PATH, O_RDONLY);
    if (key_volume_fd < 0) {
        printf("\topen %s failed\n", VOLUME_KEY_PATH);
        return -1;
    }

    res = pthread_create(&thread_key_power, NULL, scanPowerKey, NULL);
    if (res < 0) {
        printf("\tcreate thread of scanPowerKey failed\n");
        return -1;
    }

    res = pthread_create(&thread_key_volume, NULL, scanVolumeKey, NULL);
    if (res < 0) {
        printf("\tcreate thread of scanVolumeKey failed\n");
        return -1;
    }

    res = pthread_detach(thread_key_power);
    if (res < 0) {
        printf("\tset scanPowerKey detached failed\n");
        return -1;
    }

    res = pthread_detach(thread_key_volume);
    if (res < 0) {
        printf("\tset scanVolumeKey detached failed\n");
        return -1;
    }

	res = suspend_init();
	if (res < 0) {
		printf("\tError on autosuspend_init\n");
		return -1;
	}

    res = Lock();
    if (res < 0) {
        printf("\tset app lock failed\n");
        return -1;
    }

    res = suspend_enable();
    if (res < 0) {
        printf("\tRelease main wake lock failed\n");
        return -1;
    }

    sleep(1);
    return 0;
}

/*=============================================================
* Function: Lock
* Descriptions:
*   add lock
* Parameters:
*   none
* Return:
*   -1: failed
*   0: successfully
*============================================================*/
int Lock(void)
{
    int res = 0;
    char tmp[100] = {0};

    simple_lock_cnt++;
    sprintf(tmp, "%s-%d", TARGET, simple_lock_cnt);
    res = acquire_wake_lock(tmp);
    if (res < 0) {
        printf("\tset app lock failed\n");
        simple_lock_cnt--;
        return -1;
    }
    printf("\tlock successfully: %s\n", tmp);

    return 0;
}

/*=============================================================
* Function: Lock_timeout
* Descriptions:
*   add delay lock
* Parameters:
*   none
* Return:
*   -1: failed
*   0: successfully
*============================================================*/
int Lock_timeout(void)
{
    int res = 0;
    char tmp[100] = {0};

    delay_lock_cnt++;
    sprintf(tmp, "%s-%d", TARGET, delay_lock_cnt);
    res = acquire_wake_lock_timeout(tmp);
    if (res < 0) {
        printf("\tset app delay lock failed\n");
        delay_lock_cnt--;
        return -1;
    }
    printf("\tdelay lock successfully\n");

    return 0;
}

/*=============================================================
* Function: Unlock
* Descriptions:
*   delete lock
* Parameters:
*   none
* Return:
*   -1: failed
*   0: successfully
*============================================================*/
int Unlock(void)
{
    int res = 0;
    char tmp[100] = {0};

    sprintf(tmp, "%s-%d", TARGET, simple_lock_cnt);
    res = release_wake_lock(tmp);
    if (res < 0) {
        printf("\tdelete lock failed\n");
        return -1;
    }
    printf("\tunlock successfully: %s\n", tmp);
    simple_lock_cnt--;

    return 0;
}


inline void dis_info(void)
{
    printf("\n\t>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    printf("\tsimple_lock_cnt: %d\n", simple_lock_cnt);
    printf("\tdelay_lock_cnt: %d\n", delay_lock_cnt);
    printf("\tthe sum: %d\n", get_lock_count());
}

/*=============================================================
* Function: scanVolumeKey
* Descriptions:
*   the thread to scan volume up and volume down
* Parameters:
*   none
* Return:
*   none
*============================================================*/
void *scanVolumeKey(void *arg)
{
    while(1){
		if (read(key_volume_fd, &key_volume_event, sizeof(input_event) ) == sizeof(input_event) && key_volume_event.type == EV_KEY) {
            if (key_volume_event.code == 0x72) {
                if (key_volume_event.value == 1) {
                    printf("\t>>>> Unlock >>>>\n");
                    Unlock();
                    dis_info();
                }
                else if (key_volume_event.value == 0) {
                }
            }
            else if (key_volume_event.code == 0x73) {
                if (key_volume_event.value == 1) {
                    printf("\t>>>> Lock >>>>\n");
                    Lock();
                    dis_info();
                }
                else if (key_volume_event.value == 0) {
                }
            }
        }
    }
}

/*=============================================================
* Function: scanPowerKey
* Descriptions:
*   the thread to scan power
* Parameters:
*   none
* Return:
*   none
*============================================================*/
void *scanPowerKey(void *arg)
{
    while(1){
		if (read(key_power_fd, &key_power_event, sizeof(input_event) ) == sizeof(input_event) && key_power_event.type == EV_KEY) {
            if (key_power_event.value == 1) {
                if (get_lock_count() == 0) {
                    Lock();
                }
                else {
                    printf("\t>>>> Delay Lock >>>>\n");
                    Lock_timeout();
                    dis_info();
                }
            }
            else if (key_power_event.value == 0) {
            }
        }
    }
}

int main(int argc, char *argv[])
{
    int res = 0;

    res = Power_Init();
    if (res < 0) {
        printf("\tInitial power failed\n");
        return -1;
    }

    printf("\n\tNote:\n");
    printf("\tpress key power to add delay lock\n");
    printf("\tpress key volume down to delete simple lock\n");
    printf("\tpress key volume up to add simple lock\n");

    dis_info();

    while(1){
        sleep(2);
    }
}
