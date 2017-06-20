#define TAG "smartlink_softAP"
#include <tina_log.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "include/response_code.h"
#include <command_listener.h>

#include "include/smartlink_softAP.h"

#define MSG_LEN_MX 256
#define CMD_LEN_MX 128

int set_softAP_up(){
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
	agv[4] = (char *)"broadcast";
	agv[5] = (char *)"6";
	agv[6] = (char *)"wpa2-psk";
	agv[7] = (char *)"wifi1111";

	printf("***************************\n");
	printf("Start hostapd test!\n");
	printf("***************************\n");

	system("ifconfig wlan0 down");
	system("killall -9 wpa_supplicant");
	usleep(4000000);

	printf("Start to reload firmware!\n");

	agv[3] = "AP";
	sprintf(cmd,"fwreload");
	len = sizeof(msg);
	run_softap_command(&ret_code,msg,cmd,&len,agc,agv);
	if(ret_code != SOFTAP_STATUS_RESULT)
	{
		printf("Reload firmware failed! Code is %d\n",ret_code);
		free(agv);
		return -1;
	}
	printf("Message is: %s\n",msg);
	printf("Reload firmware finished!\n");

	printf("Start to set softap!\n");
	agv[3] = (char *)"Smart-AW-HOSTAPD";
	sprintf(cmd,"set");
	len = MSG_LEN_MX;
	run_softap_command(&ret_code,msg,cmd,&len,agc,agv);
	if(ret_code != SOFTAP_STATUS_RESULT)
	{
		printf("Set softap failed! Code is %d\n",ret_code);
		free(agv);
		return -1;
	}
	printf("Message is: %s\n",msg);
	printf("Set softap finished!\n");

	system("ifconfig wlan0 up");
	usleep(4000000);


	printf("Start to start softap!\n");
	sprintf(cmd,"startap");
	len = MSG_LEN_MX;
	run_softap_command(&ret_code,msg,cmd,&len,agc,agv);
	if(ret_code != SOFTAP_STATUS_RESULT)
	{
		printf("Start softap failed! Code is %d\n",ret_code);
		free(agv);
		return -1;
	}
	printf("Message is: %s\n",msg);
	printf("Start softap finished!\n");

	system("ifconfig wlan0 192.168.5.1 netmask 255.255.255.0");
	//	system("udhcpd /etc/wifi/udhcpd.conf");
	usleep(500000);
	system("/etc/init.d/dnsmasq restart");
	system("echo 1 > /proc/sys/net/ipv4/ip_forward");
	system("iptables -A FORWARD -i wlan0 -o eth0 -m state --state ESTABLISHED,RELATED -j ACCEPT");
	system("iptables -A FORWARD -i wlan0 -o eth0 -j ACCEPT");
	system("iptables -A FORWARD -i eth0 -o wlan0 -j ACCEPT");
	system("iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE");

	printf("***************************\n");
	printf("Hostapd test successed!\n");
	printf("***************************\n");

	free(agv);
	return 0;
}

int set_softAP_down(){
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
