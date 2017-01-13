#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "power_manager_client.h"

#define SEL_SUSPEND	"suspend"
#define SEL_SHUTDOWN	"shutdown"
#define SEL_REBOOT		"reboot"
#define SEL_ACQUIRE	"acquire_wakelock"
#define SEL_RELEASE	"release_wakelock"
#define SEL_USRACTIVITY "useractivity"
#define SEL_SETWAKETIME "setwaketime"
#define SEL_SETSCENE	"setscene"

void usage(char *name)
{
	printf("%s usage:\n", name);
	printf("%s sel [arg]\n", name);
	printf("sel:\n%-20s%-20s%-20s%-20s\n", SEL_SUSPEND, SEL_SHUTDOWN, SEL_REBOOT, SEL_ACQUIRE);
	printf("%-20s%-20s%-20s%-20s\n", SEL_RELEASE, SEL_USRACTIVITY, SEL_SETWAKETIME, SEL_SETSCENE);
	exit(1);
}

int main(int argc, char **argv)
{
	int ret;
	unsigned int sel=0;

	if(argc == 1)
		usage(argv[0]);
	else if(argc > 1) {
		if(!strcmp(SEL_SUSPEND, argv[1]))
			sel = 0;
		else if(!strcmp(SEL_SHUTDOWN, argv[1]))
			sel = 1;
		else if(!strcmp(SEL_REBOOT, argv[1]))
			sel = 2;
		else if(!strcmp(SEL_ACQUIRE, argv[1]) && argc == 3)
			sel = 3;
		else if(!strcmp(SEL_RELEASE, argv[1]) && argc == 3)
			sel = 4;
		else if(!strcmp(SEL_USRACTIVITY, argv[1]))
			sel = 5;
		else if(!strcmp(SEL_SETWAKETIME, argv[1]) && argc == 3)
			sel = 6;
		else if(!strcmp(SEL_SETSCENE, argv[1]))
			sel = 7;
		else
			usage(argv[0]);
	}

	switch(sel) {
		case 0:
			ret = PowerManagerSuspend(0, SUSPEND_APPLICATION);
			break;
		case 1:
			ret = PowerManagerShutDown(SHUTDOWN_DEFAULT);
			break;
		case 2:
			ret = PowerManagerReboot(REBOOT_DEFAULT);
			break;
		case 3:
			ret = PowerManagerAcquireWakeLock(argv[2]);
			break;
		case 4:
			ret = PowerManagerReleaseWakeLock(argv[2]);
			break;
		case 5:
			ret = PowerManagerUserActivity(USERACTIVITY_DEFAULT);
			break;
		case 6:
			ret = PowerManagerSetAwakeTimeout(atoi(argv[2]));
			break;
		case 7:
			ret = PowerManagerSetScene(NATIVEPOWER_SCENE_BOOT_COMPLETE);
			break;
	}
	printf("return[%d] by sending cmd: %s\n", ret, argv[1]);
}
