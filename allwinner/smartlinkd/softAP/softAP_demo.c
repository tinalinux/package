#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <net/if.h>
#include <errno.h>
#include <sys/ioctl.h>

#include "include/response_code.h"
#include "include/command_listener.h"

#include "include/smartlink_softAP.h"

#define IP_SIZE 16

#define MSG_LEN_MX 256
#define CMD_LEN_MX 128

#define PORT_DEFAULT 8066

//server
int main(int argc, char **argv){

	int port = PORT_DEFAULT;
	if(argc >=2){
		port = atoi(argv[1]);
	}
	printf("*****softAP demo*****\n");

	//先把设备设置为softAP模式
	if(set_softAP_up()==0){
		printf("start softAP success.\n");
	}


	/*创建socket接收手机发来的ssid信息，并返回确认信息*/

	//创建socket
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	printf("*****port:%d *****\n", port);


	//获取本设备的ip地址
	/*
	   char ip[IP_SIZE];
	   const char *test_eth = "wlan0";
	   get_local_ip(test_eth, ip);

	   printf("%s ip: %s\n",test_eth, ip);
	 */

	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	int sock;
	if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		printf("*****socket error!*****\n");
		printf("*****%s *****\n",strerror(errno));
		exit(1);
	}else{
		printf("*****socket success.*****\n");
	}
	//printf("*****addr:%s *****\n",addr.sin_addr.s_addr);
	if(bind(sock, (struct sockaddr *)&addr, sizeof(addr))< 0){
		printf("*****bind error!*****\n");
		printf("*****%s *****\n", strerror(errno));
		exit(1);
	}else{
		printf("bind success.\n");
	}

	//接收手机发来的ssid信息
	char buff[256];
	struct sockaddr_in clientAddr;
	int len = sizeof(clientAddr);
	int n;
	while(1){
		printf("recvfrom...\n");
		n = recvfrom(sock, buff, 511, 0, (struct sockaddr*)&clientAddr, &len);
		printf("*****addr: %s -port: %u says: %s *****\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), buff);
		if(n>0){
			printf("recvfrom success.\n");
			break;
		}
	}

	//往手机返回确认信息
	char buff_confirm[3];
	strcpy(buff_confirm,"OK");
	while(1){
		n = sendto(sock, buff_confirm, n, 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
		printf("*****addr: %s -port: %u says: %s *****\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), buff_confirm);
		if(n>0){
			printf("sendto success.\n");
			//usleep(10*1000000);
			break;
		}
	}

	usleep(5*1000000);
	close(sock);

	//设备退出softAP模式，退出后将自动恢复到station模式
	set_softAP_down();
	usleep(5*1000000);

	//解析设备端发过来的ssid信息
	char* smartlink_softAP_ssid;// = "12345678";
	char* smartlink_softAP_password;// = "12345678";

	char *p =NULL;

	int ssid_len = strstr(buff, "::Password::")-buff-8;
	printf("ssid_len  %d\n",ssid_len);
	smartlink_softAP_ssid = (char *)malloc(ssid_len+1);

	if((p = strstr(buff, "::SSID::")) != NULL){
		p += strlen("::SSID::");//跳过前缀

		if(*p){
			if(strstr(p, "::Password::") != NULL){
				memset((void*)smartlink_softAP_ssid,'\0',ssid_len+1);
				strncpy(smartlink_softAP_ssid, p, ssid_len);
			}
		}

	}

	printf("%s\n",smartlink_softAP_ssid);

	int password_len = strstr(buff, "::End::") - strstr(buff, "::Password::") - 12;
	printf("password_len  %d\n",password_len);

	smartlink_softAP_password = (char *)malloc(password_len+1);

	if((p = strstr(buff, "::Password::")) != NULL){
		p += strlen("::Password::");//跳过前缀

		if(*p){
			if(strstr(p, "::End::") != NULL){
				memset((void*)smartlink_softAP_password,'\0',password_len+1);
				strncpy(smartlink_softAP_password, p, password_len);
			}
		}

	}
	printf("%s\n",smartlink_softAP_password);


	//使用解析到的ssid信息连接wifi
	smartlink_softAP_connect_ap(smartlink_softAP_ssid, smartlink_softAP_password);

	free(buff);
	free(smartlink_softAP_ssid);
	free(smartlink_softAP_password);
	return 0;
}



void get_local_ip(const char *eth_inf, char *ip){
	int sd;
	struct sockaddr_in sin;
	struct ifreq ifr;

	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if(-1 == sd){
		printf("*****socket error: %s *****\n", strerror(errno));
		return -1;
	}

	strncpy(ifr.ifr_name, eth_inf, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;

	// if error: No such device
	if(ioctl(sd, SIOCGIFADDR, &ifr) < 0){
		printf("*****ioctl error: %s *****\n", strerror(errno));
		close(sd);
		return -1;
	}

	memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
	snprintf(ip, IP_SIZE, "%s", inet_ntoa(sin.sin_addr));

	close(sd);
	return 0;
}
