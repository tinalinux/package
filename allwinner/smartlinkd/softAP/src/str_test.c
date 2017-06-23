#include <string.h>
#include <stdio.h>
#define MAX_BUF 128

int main(){
	char *str = "::SSID::123456781::Password::12345678::End::";
	char *smartlink_softAP_ssid;// = "12345678";
	char *smartlink_softAP_password;// = "12345678";

	smartlink_softAP_ssid = (char *)malloc(20);
	memset((void*)smartlink_softAP_ssid,'\0',20);

	smartlink_softAP_password = (char *)malloc(20);
	memset((void*)smartlink_softAP_password,'\0',20);

	smartlink_softAP_get_info(str, smartlink_softAP_ssid, smartlink_softAP_password);

	printf("++++++++++smartlink_softAP_ssid  %s++++++++++\n", smartlink_softAP_ssid);
	printf("++++++++++smartlink_softAP_password  %s++++++++++\n", smartlink_softAP_password);

	/*
	   char *p =NULL;


	   char *ssid = NULL;
	   char *password = NULL;


	   int ssid_len = strstr(str, "::Password::")-str-8;
	   printf("ssid_len  %d\n",ssid_len);
	   ssid = (char *)malloc(ssid_len + 0);

	   if((p = strstr(str, "::SSID::")) != NULL){
	   p += strlen("::SSID::");

	   if(*p){
	   if(strstr(p, "::Password::") != NULL){

	   strncpy(ssid, p, ssid_len);
	   }
	   }

	   }

	   printf("%s\n",ssid);

	   int password_len = strstr(str, "::End::") - strstr(str, "::Password::") - 12;
	   printf("password_len  %d\n",password_len);

	   password = (char *)malloc(password_len + 0);

	   if((p = strstr(str, "::Password::")) != NULL){
	   p += strlen("::Password::");//跳过前缀

	   if(*p){
	   if(strstr(p, "::End::") != NULL){

	   strncpy(password, p, password_len);
	   }
	   }

	   }
	   printf("%s\n",password);
	 */
	return 0;
}
//buff:   ::SSID::12345678::Password::12345678::End::\0
int smartlink_softAP_get_info(char *str, char *smartlink_softAP_ssid, char *smartlink_softAP_password){
	char *p =NULL;

	int ssid_len = strstr(str, "::Password::")-str-8;
	printf("ssid_len  %d\n",ssid_len);
	//smartlink_softAP_ssid = (char *)malloc(ssid_len+1);

	if((p = strstr(str, "::SSID::")) != NULL){
		p += strlen("::SSID::");//跳过前缀

		if(*p){
			if(strstr(p, "::Password::") != NULL){
				//memset((void*)smartlink_softAP_ssid,'\0',ssid_len+1);
				strncpy(smartlink_softAP_ssid, p, ssid_len);
			}
		}

	}

	printf("%s\n",smartlink_softAP_ssid);

	int password_len = strstr(str, "::End::") - strstr(str, "::Password::") - 12;
	printf("password_len  %d\n",password_len);

	//smartlink_softAP_password = (char *)malloc(password_len+1);

	if((p = strstr(str, "::Password::")) != NULL){
		p += strlen("::Password::");//跳过前缀

		if(*p){
			if(strstr(p, "::End::") != NULL){
				//memset((void*)smartlink_softAP_password,'\0',password_len+1);
				strncpy(smartlink_softAP_password, p, password_len);
			}
		}

	}
	printf("%s\n",smartlink_softAP_password);


	return 0;
}
