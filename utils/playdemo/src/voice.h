#ifndef _VOICE_H
#define _VOICE_H

#define VOICE_PATH "/usr/share/playdemo_data/voice"
//for all
#define VOICE_TURN_ON "recover.mp3" //有见面了，正在联网，请稍等
//for linein
#define VOICE_LINEIN_OUT "start_network_recover.mp3" //正在恢复联网，请稍等
#define VOICE_LINEIN_IN "linein_out.mp3" //噔咚
//for bluetooth
#define VOICE_BLUETOOTH_IN "bluetoothon.mp3" //进入蓝牙模式
//for wifi
#define VOICE_WIFI_NETWORK_ERROR "play_net_err_wait_retry.mp3" //网络不给力，稍等一会再听把
#define VOICE_WIFI_NETWORK_NOT_CONNECTED "start_network_failed.mp3" //网络未连接
#define VOICE_WIFI_NETWORK_CONNECTED "start_network_success.mp3" //联网成功
#define VOICE_WIFI_RECOMMENDED_MUSIC "recommended_musiclist.mp3" //来听听我的推荐音乐把
#define VOICE_WIFI_NOT_ON "wifi_stopped.mp3" //wifi未打开
#define VOICE_WIFI_AUTO_CONNETC_AFTER_SMARTLINKD "start_network_recover.mp3" //正在恢复联网,请稍等
//for smartlinkd
#define VOICE_SMARTLINKD_IN "ready.mp3" //音响准备联网，请根据手机APP指示操作
#define VOICE_SMARTLINKD_GET_AP "bluetoothoff.mp3" //咚
//for localmusic
#define VOICE_LOCALMUSIC_IN "bluetoothsuccess.mp3"  //叮咚

#endif
