/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#define TAG "healthd"
#include <tina_log.h>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
//#include <batteryservice/BatteryService.h>
#include <cutils/klog.h>
#include <cutils/properties.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <dirent.h>
#include <sys/epoll.h>
#include <utils/Vector.h>
#include "healthd.h"
#include "BatteryMonitor.h"
//#include "String8.h"


#define POWER_SUPPLY_AC "ac"
#define POWER_SUPPLY_AC "ac"
#define POWER_SUPPLY_SUBSYSTEM "power_supply"
#define POWER_SUPPLY_SYSFS_PATH "/sys/class/" POWER_SUPPLY_SUBSYSTEM
#define POWER_SUPPLY_SYSFS_PATH_AC "/sys/class/" POWER_SUPPLY_SUBSYSTEM/POWER_SUPPLY_AC
#define POWER_SUPPLY_SYSFS_PATH_AC_ONLINE "/sys/class/" POWER_SUPPLY_SUBSYSTEM/POWER_SUPPLY_AC/"online"

#define MSEC_PER_SEC            (1000LL)
#define NSEC_PER_MSEC           (1000000LL)


#define BATTERY_UNKNOWN_TIME    (2 * MSEC_PER_SEC)
#define POWER_ON_KEY_TIME       (2 * MSEC_PER_SEC)
#define UNPLUGGED_SHUTDOWN_TIME (3 * MSEC_PER_SEC)
#define WAKEUP_TIME (2 * NSEC_PER_MSEC * MSEC_PER_SEC)

#define WAKEUP_TIME_MORE (60 * NSEC_PER_MSEC * MSEC_PER_SEC)

namespace softwinner {


 int LastAcOnline = 0;
 int  LastUsbOnline = 0;
  int LastPowerKey = 0;
 int AcOnline = 0;
 int UsbOnline = 0;
 int PowerKey = 0;
// int suspend_key = 0;

static const char *pwr_state_mem = "mem";
static const char *pwr_state_on = "on";

struct sysfsStringEnumMap {
    char* s;
    int val;
};

static int mapSysfsString(const char* str,
			struct sysfsStringEnumMap map[]){
	for(int i = 0; map[i].s; i++)
		if(!strcmp(str, map[i].s))
			return map[i].val;

	return -1;
}

int BatteryMonitor::getBatteryStatus(const char* status){
	int ret;
	struct sysfsStringEnumMap batteryStatusMap[]={
		{"Unknown", BATTERY_STATUS_UNKNOWN},
		{"Charging", BATTERY_STATUS_CHARGING},
		{"Discharging", BATTERY_STATUS_DISCHARGING},
		{"Not charging", BATTERY_STATUS_NOT_CHARGING},
		{"Full", BATTERY_STATUS_FULL},
		{NULL, 0},
	};

	ret = mapSysfsString(status, batteryStatusMap);
	if(ret < 0){
		printf("Unknown battery status '%s'\n", status);
		ret = BATTERY_STATUS_UNKNOWN;
	}

	return ret;
}

int BatteryMonitor::getBatteryHealth(const char* status) {
    int ret;
    struct sysfsStringEnumMap batteryHealthMap[] = {
        { "Unknown", BATTERY_HEALTH_UNKNOWN },
        { "Good", BATTERY_HEALTH_GOOD },
        { "Overheat", BATTERY_HEALTH_OVERHEAT },
        { "Dead", BATTERY_HEALTH_DEAD },
        { "Over voltage", BATTERY_HEALTH_OVER_VOLTAGE },
        { "Unspecified failure", BATTERY_HEALTH_UNSPECIFIED_FAILURE },
        { "Cold", BATTERY_HEALTH_COLD },
        { NULL, 0 },
    };

    ret = mapSysfsString(status, batteryHealthMap);
    if (ret < 0) {
        printf("Unknown battery health '%s'\n", status);
        ret = BATTERY_HEALTH_UNKNOWN;
    }

    return ret;
}



int BatteryMonitor::getvalue(char* buf)
{
	int fd = -1;
	int res;
	int ret;
	fd = open(buf, O_RDONLY);
	if (fd < 0) {
		TLOGE("err:open %s  fail in getvalue\n",buf);
    }
	if ((res = read(fd, buf, sizeof(buf))) < 0) {
		TLOGE("err:read %s  fail in getvalue\n",buf);
		close(fd);
	}
	close(fd);
	ret = atoi(buf);
	return ret;
}

int BatteryMonitor::readFromFile(const char * file, char * buf, size_t size){
	char *cp = NULL;
	char err_buf[80];
	if(file == NULL)
		return -1;
	int fd = open(file,O_RDONLY,0);
	if(fd == -1){
		strerror_r(errno, err_buf, sizeof(err_buf));
		TLOGE("Could not open '%s' : %s\n",file,err_buf);
		return -1;
	}

	//ssize_t count = TEMP_FAILURE_RETRY(read(fd,buf,size));
	ssize_t count = read(fd,buf,size);
	if(count > 0)
		cp = (char *)memrchr(buf, '\n',count);

	if(cp)
	    *cp = '\0';
	else
	    buf[0] = '\0';
	close(fd);

	return count;
}

bool BatteryMonitor::getBooleanField(const char *file) {
    const int SIZE = 16;
    char buf[SIZE];

    bool value = false;
    if (readFromFile(file, buf, SIZE) > 0) {
        if (buf[0] != '0') {
            value = true;
        }
    }

    return value;
}

int BatteryMonitor::getIntField(const char *file) {
    const int SIZE = 128;
    char buf[SIZE];

    int value = 0;
    if (readFromFile(file, buf, SIZE) > 0) {
        value = strtol(buf, NULL, 0);
    }
    return value;
}

static int acquire_wake_lock_timeout(long long timeout)
{
    int fd,ret;
    char str[64];
    fd = open("/sys/power/wake_lock", O_WRONLY, 0);
    if (fd < 0)
        return -1;

    sprintf(str,"charge %lld",timeout);
    ret = write(fd, str, strlen(str));
    close(fd);
    return ret;
}


int BatteryMonitor::set_power_state_mem(void){
    const int SIZE = 256;
    char file[256];
   int ret;
    sprintf(file, "sys/power/%s", "state");
    int fd = open(file,O_RDWR,0);
    if(fd == -1){
		printf("Could not open '%s'\n",file);
		return -1;
    }
     ret = write(fd, pwr_state_mem, strlen(pwr_state_mem));
     if (ret < 0) {
	TLOGE("set_power_state_mem err\n");
	}
    close(fd);
    return 0;
}


int BatteryMonitor::set_power_state_on(void){
    const int SIZE = 256;
    char file[256];
   int ret;
    sprintf(file, "sys/power/%s", "state");
    int fd = open(file,O_RDWR,0);
    if(fd == -1){
		printf("Could not open '%s'\n",file);
		return -1;
    }
     ret = write(fd, pwr_state_on, strlen(pwr_state_on));
     if (ret < 0) {
	TLOGE("set_power_state_on err\n");
	}
    close(fd);
    return 0;
}



bool BatteryMonitor::update(void) {
	//printf("14:05 BatteryMonitor::updiate()\n");

	const int SIZE = 256;
	char buf[SIZE];
	char file[256];
	int ret;

    props.chargerAcOnline = false;
    props.chargerUsbOnline = false;
    //props.chargerWirelessOnline = false;
    props.batteryStatus = BATTERY_STATUS_UNKNOWN;
    props.batteryHealth = BATTERY_HEALTH_UNKNOWN;

/*
	sprintf(buf, "sys/class/power_supply/%s/%s", "usb", "suspend_key");
	suspend_key= getBooleanField(buf);
	TLOGI("update-suspend_key=%d\n",suspend_key);
*/

	//present
	sprintf(buf, "sys/class/power_supply/%s/%s", "battery", "present");
	mHealthdConfig->batteryPresent= getBooleanField(buf);
	//TLOGI("update-mHealthdConfig->batteryPresent=%d\n",mHealthdConfig->batteryPresent);
	//capacity

	props.batteryPresent = mHealthdConfig->batteryPresent;
	sprintf(buf, "sys/class/power_supply/%s/%s", "battery", "capacity");
	mHealthdConfig->batteryCapacity= getIntField(buf);
	props.batteryLevel=mHealthdConfig->batteryCapacity;
	//TLOGI("update-mHealthdConfig->batteryCapacity=%d\n",mHealthdConfig->batteryCapacity);
	//voltage_now
	sprintf(buf, "sys/class/power_supply/%s/%s", "battery", "voltage_now");
	mHealthdConfig->batteryVoltage= getIntField(buf)/ 1000;
	//TLOGI("update-mHealthdConfig->batteryVoltage=%d\n",mHealthdConfig->batteryVoltage);
	//temp
	sprintf(buf, "sys/class/power_supply/%s/%s", "battery", "temp");
	mHealthdConfig->batteryTemperature= getIntField(buf);
	//TLOGI("update-mHealthdConfig->batteryTemperature=%d\n",mHealthdConfig->batteryTemperature);
	//current_now
	sprintf(buf, "sys/class/power_supply/%s/%s", "battery", "current_now");
	mHealthdConfig->batteryCurrentNow= getIntField(buf)/1000;
	//TLOGI("update-mHealthdConfig->batteryCurrentNow=%d\n",mHealthdConfig->batteryCurrentNow);

	//status
	sprintf(file, "sys/class/power_supply/%s/%s", "battery", "status");
	if(readFromFile(file, buf, SIZE) > 0)
		mHealthdConfig->batteryStatus = getBatteryStatus(buf);
	//TLOGI("update-mHealthdConfig->batteryStatus=%d\n",mHealthdConfig->batteryStatus);
	//health
	sprintf(file, "sys/class/power_supply/%s/%s", "battery", "health");
	if(readFromFile(file, buf, SIZE) > 0)
		mHealthdConfig->batteryHealth=getBatteryHealth(buf);
	//TLOGI("update-mHealthdConfig->batteryHealth=%d\n",mHealthdConfig->batteryHealth);
	//ac online
	sprintf(buf, "sys/class/power_supply/%s/%s", "ac", "online");
	AcOnline= getBooleanField(buf);
	props.chargerAcOnline = AcOnline;
	//TLOGI("update-ac online=%d\n",AcOnline);
	//usb online
	sprintf(buf, "sys/class/power_supply/%s/%s", "usb", "online");
	UsbOnline= getBooleanField(buf);
	props.chargerUsbOnline = UsbOnline;
	//TLOGI("update-usb online=%d\n",UsbOnline);



	healthd_mode_ops->battery_update(&props);



/*
	//power_key
	sprintf(buf, "sys/class/power_supply/%s/%s", "usb", "power_key");
	PowerKey= getBooleanField(buf);
	TLOGI("update-PowerKey=%d\n",PowerKey);

	if(PowerKey == 1){
		if(is_charge_bm == 1)
		{
			printf("charge mode and  wake lock  timeout\n");
			acquire_wake_lock_timeout(WAKEUP_TIME);
			if(UsbOnline == 0 && AcOnline == 0){
				printf("No charging then should poweroff!\n");
				system("poweroff");

			}
		}else{
			printf("Tina wake up \n");
			//ret=set_power_state_on();
		//	ret=set_power_state_mem();
			acquire_wake_lock_timeout(WAKEUP_TIME_MORE);
			ret=set_power_state_mem();
		}
	}
*/
	printf("healthd:l-%d c-%d u-%d A-%d v-%d h-%d p-%d\n",mHealthdConfig->batteryCapacity,mHealthdConfig->batteryCurrentNow,UsbOnline,AcOnline,mHealthdConfig->batteryVoltage,mHealthdConfig->batteryHealth,mHealthdConfig->batteryPresent);

	LastAcOnline = AcOnline;
	LastUsbOnline = UsbOnline;
	return AcOnline | UsbOnline;
}


void BatteryMonitor::init(struct healthd_config *hc, int * charger)
{
	mHealthdConfig = hc;
	is_charge_bm = *charger;
//	printf("is_charge_bm=%d\n",is_charge_bm);
	DIR* dir = opendir(POWER_SUPPLY_SYSFS_PATH);  //open /sys/class/power_supply
	char buf[256];
	char file[256];
	int ret;
	int batteryStatus ;
	const int SIZE = 128;
	 if(dir == NULL){
		TLOGE("Could not open %s\n",POWER_SUPPLY_SYSFS_PATH);
	 }else{
		struct dirent* entry;
		while((entry = readdir(dir))){
			const char* name = entry->d_name;

			if(!strcmp(name, ".") || !strcmp(name, ".."))
				continue;

			if(!strcmp(name,"usb")){

				sprintf(buf, "sys/class/power_supply/%s/%s", "usb", "online");
				LastUsbOnline = getvalue(buf);
				//TLOGI("Init usb_online=%d\n",LastUsbOnline);

				/*
				sprintf(buf, "sys/class/power_supply/%s/%s", "usb", "power_key");
				LastPowerKey = getvalue(buf);
				TLOGI("Init LastPowerKey=%d\n",LastPowerKey);
				*/

			}else if(!strcmp(name,"ac")){
				sprintf(buf, "sys/class/power_supply/%s/%s", "ac", "online");
			        LastAcOnline = getvalue(buf);
				//TLOGI("Init ac_online=%d\n",LastAcOnline);
			}else if(!strcmp(name,"battery")){
				mBatteryDevicePresent = true;

				//status
				sprintf(file, "sys/class/power_supply/%s/%s", "battery", "status");
				if(readFromFile(file, buf, SIZE) > 0)
					mHealthdConfig->batteryStatus = getBatteryStatus(buf);
				//TLOGI("Init batteryStatus=%d\n",mHealthdConfig->batteryStatus);

				//health
				sprintf(file, "sys/class/power_supply/%s/%s", "battery", "health");
				if(readFromFile(file, buf, SIZE) > 0)
					mHealthdConfig->batteryHealth=getBatteryHealth(buf);
				//TLOGI("Init batteryHealth=%d\n",mHealthdConfig->batteryHealth);

				//present
				sprintf(buf, "sys/class/power_supply/%s/%s", "battery", "present");
				mHealthdConfig->batteryPresent= getBooleanField(buf);
				//TLOGI("Init batteryPresen=%d\n",mHealthdConfig->batteryPresent);
				//capacity
				sprintf(buf, "sys/class/power_supply/%s/%s", "battery", "capacity");
				mHealthdConfig->batteryCapacity= getIntField(buf);
				//TLOGI("Init batteryCapacity=%d\n",mHealthdConfig->batteryCapacity);
				//voltage_now
				sprintf(buf, "sys/class/power_supply/%s/%s", "battery", "voltage_now");
				mHealthdConfig->batteryVoltage= getIntField(buf)/1000;
				//TLOGI("Init batteryVoltage=%d\n",mHealthdConfig->batteryVoltage);
				//temp
				sprintf(buf, "sys/class/power_supply/%s/%s", "battery", "temp");
				mHealthdConfig->batteryTemperature= getIntField(buf);
				//TLOGI("Init batteryTemperature=%d\n",mHealthdConfig->batteryTemperature);
				//current_now
				sprintf(buf, "sys/class/power_supply/%s/%s", "battery", "current_now");
				mHealthdConfig->batteryCurrentNow= getIntField(buf);
				//TLOGI("Init batteryCurrentNow=%d\n",mHealthdConfig->batteryCurrentNow);
			}
		}



	 }

	 closedir(dir);

    if (!mBatteryDevicePresent) {
        TLOGE("No battery devices found\n");
        hc->periodic_chores_interval_fast = -1;
        hc->periodic_chores_interval_slow = -1;
    }


}



}
