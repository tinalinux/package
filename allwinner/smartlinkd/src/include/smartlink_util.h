#ifndef __UTIL_H__
#define __UTIL_H__

#define LEN 65
#define KEY_LEN 16
#define ADT_STR 129

#define IFNAME "wlan0"
#define CHECK_WIFI_SHELL "lsmod | grep bcmdhd"
#define CHECK_WLAN_SHELL "ifconfig | grep "IFNAME
#define Broadcom_Setup "/setup"
#define Realtek_Setup "/arikiss"

struct ap_info{
	char ssid[LEN];
	char password[LEN];
	int security;
};
struct sender_info{
	char ip[16];/* ipv4 max length */
	int port;
};
struct _info{
	struct ap_info base_info;
	int protocol;
	int airkiss_random;
	struct sender_info ip_info;
    char adt_str[ADT_STR];
};

/* wifi modules */
enum {
	AW_SMARTLINKD_BROADCOM = 0,
	AW_SMARTLINKD_REALTEK,
	AW_SMARTLINKD_ALLWINNERTCH,
};

/* wlan security */
enum {
	AW_SMARTLINKD_SECURITY_NONE = 0,
	AW_SMARTLINKD_SECURITY_WEP,
	AW_SMARTLINKD_SECURITY_WPA,
	AW_SMARTLINKD_SECURITY_WPA2,
};

/* protocols */
enum {
	AW_SMARTLINKD_PROTO_COOEE = 0,
	AW_SMARTLINKD_PROTO_NEEZE,
	AW_SMARTLINKD_PROTO_AKISS,
	AW_SMARTLINKD_PROTO_ADT,
	AW_SMARTLINKD_PROTO_XIAOMI,
	AW_SMARTLINKD_PROTO_CHANGHONG,
	AW_SMARTLINKD_PROTO_MAX,
	AW_SMARTLINKD_PROTO_FAIL = 0xff,
};

//cmd for processes
enum {
	AW_SMARTLINKD_START = 0,
	AW_SMARTLINKD_STOP,
	AW_SMARTLINKD_FINISHED,
	AW_SMARTLINKD_FAILED,
};

struct _cmd{
	int cmd;
	char usekey;
	char AESKey[KEY_LEN];
	struct _info info;
};
#endif /* __UTIL_H__ */
