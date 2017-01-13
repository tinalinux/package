#define TAG "smartlinkd-client"
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stddef.h>
#include "aw_smartlinkd_connect.h"
#include "log.h"

#ifdef __cplusplus
extern "C" {
#endif

static int(*func)(char*,int) = NULL;

#ifndef ANDROID_ENV
#define UNIX_DOMAIN "/tmp/UNIX.domain"
#else
#define UNIX_DOMAIN "/data/misc/wifi/UNIX.domain"
#endif
static int sockfd = -1;
static int initialized = 0;

void printf_info(struct _cmd *c){
	LOGD("cmd: %d\n",c->cmd);
	if(c->cmd == AW_SMARTLINKD_FAILED){
		LOGD("response failed\n");
		return;
	}
	LOGD("ssid: %s\n",c->info.base_info.ssid);
	LOGD("pasd: %s\n",c->info.base_info.password);
	LOGD("pcol: %d\n",c->info.protocol);
	if(c->info.protocol == AW_SMARTLINKD_PROTO_AKISS)
		LOGD("radm: %d\n",c->info.airkiss_random);
	if(c->info.protocol == AW_SMARTLINKD_PROTO_COOEE){
		LOGD("ip: %s\n",c->info.ip_info.ip);
		LOGD("port: %d\n",c->info.ip_info.port);
	}
}

static int init_socket(){
	int ret;
	struct sockaddr_un srv_addr;
	//creat unix socket
	if(sockfd != -1){
		LOGE("another smartlinkd client exsit, please wait!!");
		return -1;
	}
	sockfd=socket(PF_UNIX,SOCK_STREAM,0);
	if(sockfd<0)
	{
		LOGE("cannot create communication socket");
		return -1;
	}
	srv_addr.sun_family=AF_UNIX;
	//strcpy(srv_addr.sun_path,UNIX_DOMAIN);
	strcpy(srv_addr.sun_path+1,UNIX_DOMAIN);
	srv_addr.sun_path[0] = '\0';
	int size = offsetof(struct sockaddr_un,sun_path) + strlen(UNIX_DOMAIN)+1;
	//connect server
	ret=connect(sockfd,(struct sockaddr*)&srv_addr,size);
	if(ret==-1)
	{
		LOGE("cannot connect to the server: %s",strerror(errno));
		close(sockfd);
        sockfd = -1;
		return -1;
	}
	return 0;
}

void aw_smartlinkd_prepare(){
	//stop wpa_supplicant
	system("/etc/wifi/wifi stop");
	//up the wlan
	system("ifconfig wlan0 up");
}
void aw_release(){
	close(sockfd);
    sockfd = -1;
}

static void* readthread(void* arg){
	char buf[1024];
	if(func != NULL && func(buf,THREAD_INIT) == THREAD_EXIT);//test
	while(1){
		memset(buf,0,sizeof(buf));
		int bytes_read = read(sockfd, buf, sizeof(buf)); //read once only
		LOGD("sockfd->buf:read byte %d\n",bytes_read);
		if ( bytes_read == -1 )	{
			if( errno == EAGAIN || errno == EWOULDBLOCK ){
				continue;
			}
			LOGE("recv failed(%s)\n",strerror(errno));
			if(func != NULL) func(buf,bytes_read);
			break;
		}
		else if ( bytes_read == 0 ){
            //LOGD("server close the connection\n");
			if(func != NULL) func(buf,bytes_read);
			break;
		}

		if(func != NULL && func(buf,bytes_read) == THREAD_EXIT)
			break;
	}
    initialized = 0;
    LOGD("thread exit....");
	close(sockfd);
    sockfd = -1;
	return (void*)0;
}
static int init_thread(){
	pthread_t tid;
	if(pthread_create( &tid, NULL, readthread, (void*)0 ) != 0){
		LOGE("create read thread failed!\n");
		return -1;
	}
	if( pthread_detach( tid ) ){
		LOGE("detach read thread failed!\n");
		return -1;
	}
	return 0;
}
int aw_smartlinkd_init(int fd,int(*f)(char*,int)){
	func = f;

    if(init_socket() == 0 && init_thread() == 0){
        initialized = 1;
        return 0;
    }
    return -1;
}
static int startprotocol(int protocol){
	struct _cmd c;
	c.cmd = AW_SMARTLINKD_START;
	c.info.protocol = protocol;
	LOGD("protocol = %d\n",protocol);
    if(initialized == 0)
        aw_smartlinkd_init(0,func);
	return send(sockfd,(char*)&c,sizeof(c),0);
}
static int stopprotocol(){
	struct _cmd c;
	c.cmd = AW_SMARTLINKD_STOP;
	return send(sockfd,(char*)&c,sizeof(c),0);
}
int aw_startairkiss(){
	return startprotocol(AW_SMARTLINKD_PROTO_AKISS);
}
int aw_stopairkiss(){
    return stopprotocol();
}

int aw_startcooee(){
	return startprotocol(AW_SMARTLINKD_PROTO_COOEE);
}
int aw_stopcooee(){
    return stopprotocol();
}

int aw_startadt(){
	return startprotocol(AW_SMARTLINKD_PROTO_ADT);
}
int aw_stopadt(){
    return stopprotocol();
}
int aw_startcomposite(int proto_mask){
    return startprotocol(proto_mask);
}
int aw_stopcomposite(){
    return stopprotocol();
}

int aw_easysetupfinish(struct _cmd *c){
	//printf_info(c);
	c->cmd = AW_SMARTLINKD_FINISHED;
	return send(sockfd,(char*)c,sizeof(struct _cmd),0);
}
#ifdef __cplusplus
}
#endif
