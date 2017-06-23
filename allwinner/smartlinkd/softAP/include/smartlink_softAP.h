#ifndef __SMARTLINK_SOFTAP_H
#define __SMARTLINK_SOFTAP_H

#if __cplusplus
extern "C" {
#endif



	int smartlink_softAP_connect_ap(char *ssid, char *password);
	int set_softAP_up();
	int set_softAP_down();

#if __cplusplus
};  // extern "C"
#endif

#endif /* __WPA_SUPPLICANT_CONF_H */
