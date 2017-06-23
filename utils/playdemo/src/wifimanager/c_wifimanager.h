#ifndef _C_WIFIMANAGER_H
#define _C_WIFIMANAGER_H

#include <wifi_intf.h>

#ifdef CPP
extern "C"
{
#endif

int Wifi_On(void);
int Wifi_Off(void);
int Wifi_Get_Connected_AP_Info(char *ssid, int *len);
int Wifi_Scan(char *result, int *len);
int Wifi_Connect_AP(const char *ssid, int type, const char *passwd);
int Wifi_Disconnect_AP(void);
int Wifi_Disconnect_AP(void);
int Wifi_Init(void);
int Wifi_Get_State(void);
void Wifi_State(void);

#ifdef CPP
}
#endif

#endif
