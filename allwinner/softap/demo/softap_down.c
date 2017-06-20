#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <response_code.h>
#include <command_listener.h>



#define MSG_LEN_MX 256
#define CMD_LEN_MX 128
int main()
{
	tRESPONSE_CODE ret_code = SOFTAP_STATUS_RESULT;
	char msg[MSG_LEN_MX] = {0};
	char cmd[CMD_LEN_MX] = {0};
	int agc = 8;
	int len = 0;
	char **agv = (char **)malloc(sizeof(char *) * agc);
	agv[agc-1] = NULL;
        agv[0] = (char *)"setsoftap";
        agv[1] = (char *)"set";
        agv[2] = (char *)"wlan0";
        agv[3] = (char *)"Smart-AW-HOSTAPD";
		/*if(password == NULL)
		 agv[4] = (char *)"open";
		 else
			*/
        agv[4] = (char *)"broadcast";
        agv[5] = (char *)"6";
        agv[6] = (char *)"wpa2-psk";
		agv[7] = (char *)"wifi1111";

	printf("***************************\n");
	printf("Start to shutdown hostapd!\n");
	printf("***************************\n");

	system("ifconfig wlan0 down");
	usleep(4000000);

	printf("Start to reload firmware!\n");

	agv[3] = "STA";
	sprintf(cmd,"fwreload");
	len = sizeof(msg);
	run_softap_command(&ret_code,msg,cmd,&len,agc,agv);
	if(ret_code != SOFTAP_STATUS_RESULT)
	{
		printf("Reload firmware failed! Code is %d\n",ret_code);
		free(agv);
		return -1;
	}

	system("killall -9 hostapd");
/*
	system("ifconfig wlan0 up");
*/
	usleep(4000000);


	printf("***************************\n");
	printf("Shutdown Hostapd successed!\n");
	printf("***************************\n");

	free(agv);
	return 0;
}
