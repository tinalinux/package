#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <response_code.h>
#include <command_listener.h>


#define MSG_LEN_MX 256
#define CMD_LEN_MX 128
int main(int argc, char **argv)
{
	tRESPONSE_CODE ret_code = SOFTAP_STATUS_RESULT;
	char msg[MSG_LEN_MX] = {0};
	char cmd[CMD_LEN_MX] = {0};
	int i = 0;
	int agc = 8;
	int len = 0;
	char **agv = (char **)malloc(sizeof(char *) * agc);

	agv[agc-1] = NULL;
    agv[0] = (char *)"setsoftap";
    agv[1] = (char *)"set";
    agv[2] = (char *)"wlan0";
    agv[3] = (char *)"Smart-AW-HOSTAPD";
	agv[4] = (char *)"broadcast"; /*broadcast or hidden*/
    agv[5] = (char *)"6";
    agv[6] = (char *)"wpa2-psk";
	agv[7] = (char *)"wifi1111";

	/*No argruments or only the one who is for help*/
	if((argc == 1) || (argc == 2 && !strcmp(argv[1],"help")))
	{
		printf("******************************\n");
		printf("softap_up help: for usage details\n");
		printf("Usage:\n");
		printf("softap_up \"ssid\" \"psk/open\" \"broadcast/hidden\"\n");
		printf("******************************\n");
		free(agv);
		return 0;
	}
	/*If there is no passwords, set it as an open ap*/
	if((argv[2] != NULL) && strcmp(argv[2],"open"))
	{
		if(strlen(argv[2]) >= 8)
			agv[7] = (char *)argv[2];
		else
		{
			printf("Error: the length of password must be larger than 8!\n");
			free(agv);
			return -1;
		}
	}
	else
		agv[6] = (char *)"open";

	/*If the ap is hidden*/
	if(argc >= 4)
	{
		if(!strcmp(argv[3],"hidden") || !strcmp(argv[3],"broadcast"))
			agv[4] = (char *)argv[3];
		else
		{
			printf("The third parameter error! Please use \"broadcast\" or \"hidden\"!\n");
			free(agv);
			return -1;
		}
	}


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
	agv[3] = (char *)argv[1];
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
//	usleep(500000);
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
