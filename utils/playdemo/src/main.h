#ifndef _MAIN_H
#define _MAIN_H

#define TRUE 1
#define FALSE 0

#ifdef CPP
extern "C"
{
#endif

struct APINFO{
    char isEnable;
    char ssid[64];
    char password[100];
    int  security;
};
extern struct APINFO my_ap_info;

enum MAINSTATE{
    STATE_WIFI = 0,
    STATE_LINEIN,
    STATE_BLUETOOTH,
    STATE_SMARTLINKD,
    STATE_LOCALMUSIC,
};

extern int begin;

int Set_MainState(enum MAINSTATE state);
enum MAINSTATE Get_LastMainState(void);
enum MAINSTATE Get_MainState(void);

#ifdef CPP
}
#endif

#endif
