#include <sys/ioctl.h>
#include <net/if.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <errno.h>
#include <aw_smartlinkd_connect.h>

#include <json-c/json.h>

#include "ConfigUtils.h"
#include "NetworkConnection.h"

#include "parrot_common.h"

static int g_event;
static int local_msgid = 100;

namespace Parrot{
NetworkConnection* NetworkConnection::mInstance = NULL;
locker* NetworkConnection::mInstanceLock = new locker();
static tStateString wifi_state_string[] =
{
  { WIFIMG_WIFI_ON_SUCCESS, "WIFIMG_WIFI_ON_SUCCESS" },
  { WIFIMG_WIFI_ON_FAILED, "WIFIMG_WIFI_ON_FAILED" },
  { WIFIMG_WIFI_OFF_FAILED, "WIFIMG_WIFI_OFF_FAILED" },
  { WIFIMG_WIFI_OFF_SUCCESS, "WIFIMG_WIFI_OFF_SUCCESS" },
  { WIFIMG_NETWORK_CONNECTED, "WIFIMG_NETWORK_CONNECTED" },
  { WIFIMG_NETWORK_DISCONNECTED, "WIFIMG_NETWORK_DISCONNECTED" },
  { WIFIMG_PASSWORD_FAILED, "WIFIMG_PASSWORD_FAILED" },
  { WIFIMG_CONNECT_TIMEOUT, "WIFIMG_CONNECT_TIMEOUT" },
  { WIFIMG_NO_NETWORK_CONNECTING, "WIFIMG_NO_NETWORK_CONNECTING" },
  { WIFIMG_CMD_OR_PARAMS_ERROR, "WIFIMG_CMD_OR_PARAMS_ERROR" },
  { WIFIMG_KEY_MGMT_NOT_SUPPORT, "WIFIMG_KEY_MGMT_NOT_SUPPORT" },
  { WIFIMG_OPT_NO_USE_EVENT, "WIFIMG_OPT_NO_USE_EVENT" },
  { WIFIMG_NETWORK_NOT_EXIST, "WIFIMG_NETWORK_NOT_EXIST" },
  { WIFIMG_DEV_BUSING_EVENT, "WIFIMG_DEV_BUSING_EVENT" },
};

static void wifi_event_handle(tWIFI_EVENT wifi_event, void *buf, int event_label)
{
    plogd("event_label 0x%x\n", event_label);
    NetworkConnection::getInstance()->onWifiManagerEvent(wifi_event, buf, event_label);
}

static int smartlinkd_event_handle(char* buf,int length){
    if(length == THREAD_INIT){

    }
    else if(length < 0){
        ploge("error");
        return THREAD_EXIT;
    }
    else if(length == 0){
        plogd("server close the connection...\n");
        return THREAD_EXIT;
    }
    else if(length > 0)
        NetworkConnection::getInstance()->onSmartlinkdEvent(buf, length);
    return THREAD_CONTINUE;
}

int get_mac_addr(const char* name,char* hwaddr)
{
    struct ifreq ifreq;
    int sock;

    if((sock=socket(AF_INET,SOCK_STREAM,0)) <0)
    {
        perror( "socket ");
        return 2;
    }
    strcpy(ifreq.ifr_name,name);
    if(ioctl(sock,SIOCGIFHWADDR,&ifreq) <0)
    {
        perror( "ioctl ");
        return 3;
    }
    sprintf(hwaddr,"%02x:%02x:%02x:%02x:%02x:%02x",
            (unsigned char)ifreq.ifr_hwaddr.sa_data[0],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[1],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[2],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[3],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[4],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[5]);

    return   0;
}

/* ########################################################## */
NetworkConnection* NetworkConnection::createInstance(ref_HandlerBase base)
{
    locker::autolock l(*mInstanceLock);
    if(mInstance == NULL){
        mInstance = new NetworkConnection(base);
    }
    return mInstance;
}
NetworkConnection* NetworkConnection::getInstance()
{
    locker::autolock l(*mInstanceLock);
    return mInstance;
}

NetworkConnection::NetworkConnection(ref_HandlerBase base):
    mWifiStatus(10,"network",wifi_state_string,sizeof(wifi_state_string)/sizeof(tStateString)),
    mListener(NULL),
    mWifiInterface(NULL),
    Handler(base)
{
    //run();
}
NetworkConnection::~NetworkConnection()
{
    aw_wifi_off(mWifiInterface);
}
int NetworkConnection::init()
{
    sendMessage(START_WIFI_MANAGER);
    mWifiInterfaceSem.wait();
    if(mWifiStatus.get(StateManager::CUR) == WIFIMG_WIFI_ON_FAILED){
        ploge("NetworkConnection::init failed!");
        return FAILURE;
    }
    return SUCCESS;
}
int NetworkConnection::release()
{

}

void NetworkConnection::realeseInstance()
{
    if(mInstance != NULL)
        delete mInstance;
}

NetworkConnection::tNetworkStatus NetworkConnection::getWifiStatus()
{
    if(aw_wifi_get_wifi_state() == WIFIMG_WIFI_CONNECTED)
        return NETWORK_CONNECTED;
    return NETWORK_DISCONNECTED;
}
void NetworkConnection::onSmartlinkdEvent(char *buf, int length)
{
    struct _cmd* c = (struct _cmd *)buf;
    if(c->cmd == AW_SMARTLINKD_FAILED){
        plogd("response failed\n");
        return ;
    }
    if(c->info.protocol == AW_SMARTLINKD_PROTO_FAIL){
        plogd("proto scan fail");
        notify_all_listener(NetworkConnectionLinstener::SmartlinkdFailed);
        return ;
    }

    plogd("cmd: %d\n",c->cmd);
    plogd("pcol: %d\n",c->info.protocol);
    plogd("ssid: %s\n",c->info.base_info.ssid);
    plogd("pasd: %s\n",c->info.base_info.password);
    plogd("security: %d\n",c->info.base_info.security);

    if(mListener != NULL)
        mListener->onNetworkSmartlinkdDetected();
    notify_all_listener(NetworkConnectionLinstener::SmartlinkdDetected);

    if(c->info.protocol == AW_SMARTLINKD_PROTO_AKISS)
        plogd("radm: %d\n",c->info.airkiss_random);

    if(c->info.protocol == AW_SMARTLINKD_PROTO_COOEE){
        plogd("ip: %s port: %d",c->info.ip_info.ip,c->info.ip_info.port);
        mSmartlinkResponeIp = string(c->info.ip_info.ip);
        mWifiInterface->connect_ap(c->info.base_info.ssid, c->info.base_info.password, SMARTLINKD_CONNECT);
    }
    if(c->info.protocol == AW_SMARTLINKD_PROTO_ADT){
        plogd("adt get: %s\n",c->info.adt_str);

        struct json_object *main_json = json_tokener_parse(c->info.adt_str);
        if(is_error(main_json)) {
            plogd("adt read json err!!");
            return;
        }
        struct json_object *j_ssid = NULL;
        struct json_object *j_psk = NULL;
        struct json_object *j_ip = NULL;
        const char *s_ssid, *s_psk, *s_ip;

        if(json_object_object_get_ex(main_json, "ssid", &j_ssid)){
            s_ssid = json_object_get_string(j_ssid);
        }
        if(json_object_object_get_ex(main_json, "psk", &j_psk)){
            s_psk = json_object_get_string(j_psk);
        }
        if(json_object_object_get_ex(main_json, "ip", &j_ip)){
            s_ip = json_object_get_string(j_ip);
        }
        mSmartlinkResponeIp = string(s_ip);
        mWifiInterface->connect_ap(s_ssid, s_psk, SMARTLINKD_CONNECT);
        json_object_put(main_json);

        /*
        cJSON *json;
        json = cJSON_Parse(c->info.adt_str);
        if (!json) {
            ploge("Error before: [%s]\n",cJSON_GetErrorPtr());
            return;
        }
        char *ssid = cJSON_GetObjectItem(json,"ssid")->valuestring;
        char *psk  = cJSON_GetObjectItem(json,"psk")->valuestring;
        char *ip = cJSON_GetObjectItem(json,"ip")->valuestring;
        mSmartlinkResponeIp = string(ip);
        mWifiInterface->connect_ap(ssid, psk, SMARTLINKD_CONNECT);

        cJSON_Delete(json);
        */
    }
}
void NetworkConnection::onWifiManagerEvent(tWIFI_EVENT wifi_event, void *buf, int msgid)
{
    switch(wifi_event)
    {
        case WIFIMG_WIFI_ON_SUCCESS:
        {
            plogd("WiFi on success!\n");
            mWifiStatus.update(wifi_event);
            mWifiInterfaceSem.post();
            break;
        }

        case WIFIMG_WIFI_ON_FAILED:
        {
            plogd("WiFi on failed!\n");
            mWifiStatus.update(wifi_event);
            mWifiInterfaceSem.post();
            break;
        }

        case WIFIMG_WIFI_OFF_FAILED:
        {
            plogd("wifi off failed!\n");
            mWifiStatus.update(wifi_event);
            break;
        }

        case WIFIMG_WIFI_OFF_SUCCESS:
        {
            plogd("wifi off success!\n");
            mWifiStatus.update(wifi_event);
            break;
        }

        case WIFIMG_NETWORK_CONNECTED:
        {
            plogd("WiFi connected ap!\n");
            mWifiStatus.update(wifi_event);
            //if(SMARTLINKD_CONNECT == msgid){
            //    sendMessage(RESPONE_SMARTLINK);
            //}
            if(mListener != NULL)
                mListener->onNetworkConnected();
            notify_all_listener(NetworkConnectionLinstener::Connected);

            //UI

            break;
        }

        case WIFIMG_NETWORK_DISCONNECTED:
        {
            plogd("WiFi disconnected!\n");
            mWifiStatus.update(wifi_event);
            if(mListener != NULL)
                mListener->onNetworkDisconnected();
            notify_all_listener(NetworkConnectionLinstener::Disconnected);
            break;
        }

        case WIFIMG_PASSWORD_FAILED:
        {
            plogd("Password authentication failed!\n");
            mWifiStatus.update(wifi_event);
            if(mListener != NULL)
                mListener->onNetworkConnectFailed();
            notify_all_listener(NetworkConnectionLinstener::ConnectFailed);
            break;
        }

        case WIFIMG_CONNECT_TIMEOUT:
        {
            plogd("Connected timeout!\n");
            mWifiStatus.update(wifi_event);
            if(mListener != NULL)
                mListener->onNetworkConnectFailed();
            notify_all_listener(NetworkConnectionLinstener::ConnectFailed);
            break;
        }

        case WIFIMG_NO_NETWORK_CONNECTING:
        {
            plogd("It has no wifi auto connect when wifi on!\n");
            mWifiStatus.update(wifi_event);
            if(mListener != NULL)
                mListener->onNetworkConnectFailed();
            notify_all_listener(NetworkConnectionLinstener::ConnectFailed);
            break;
        }

        case WIFIMG_CMD_OR_PARAMS_ERROR:
        {
            plogd("cmd or params error!\n");
            mWifiStatus.update(wifi_event);
            break;
        }

        case WIFIMG_KEY_MGMT_NOT_SUPPORT:
        {
            plogd("key mgmt is not supported!\n");
            mWifiStatus.update(wifi_event);
            break;
        }

        case WIFIMG_OPT_NO_USE_EVENT:
        {
            plogd("operation no use!\n");
            mWifiStatus.update(wifi_event);
            break;
        }

        case WIFIMG_NETWORK_NOT_EXIST:
        {
            plogd("network not exist!\n");
            mWifiStatus.update(wifi_event);
            break;
        }

        case WIFIMG_DEV_BUSING_EVENT:
        {
            plogd("wifi device busing!\n");
            mWifiStatus.update(wifi_event);
            break;
        }

        default:
        {
            plogd("Other event, no care!\n");
        }
    }
}

void NetworkConnection::addListener(NetworkConnectionLinstener *l)
{
    mListenerList.push_back(l);
}
void NetworkConnection::notify_all_listener(NetworkConnectionLinstener::tNotifyId id)
{
/*
        typedef enum _NotifyId
        {
            SmartlinkdDetected,
            Connected,
            ConnectFailed,
            Disconnected
        }tNotifyId;
        virtual void onNetworkSmartlinkdDetected(){};
        virtual void onNetworkConnected(){};
        virtual void onNetworkConnectFailed(){};
        virtual void onNetworkDisconnected(){};
*/
    list<NetworkConnectionLinstener*>::iterator it;
    for(it = mListenerList.begin(); it != mListenerList.end(); it++){
        switch(id){
            case NetworkConnectionLinstener::SmartlinkdDetected:
                plogd("NetworkConnectionLinstener::SmartlinkdDetected");
                (*it)->onNetworkSmartlinkdDetected();
                break;
            case NetworkConnectionLinstener::SmartlinkdFailed:
                plogd("NetworkConnectionLinstener::SmartlinkdFailed");
                (*it)->onNetworkSmartlinkdFailed();
                break;
            case NetworkConnectionLinstener::Connected:
                plogd("NetworkConnectionLinstener::Connected");
                (*it)->onNetworkConnected();
                break;
            case NetworkConnectionLinstener::ConnectFailed:
                plogd("NetworkConnectionLinstener::ConnectFailed");
                (*it)->onNetworkConnectFailed();
                break;
            case NetworkConnectionLinstener::Disconnected:
                plogd("NetworkConnectionLinstener::Disconnected");
                (*it)->onNetworkDisconnected();
                break;
        }
    }
}

int NetworkConnection::startSmartlink()
{
    return sendMessage(START_SMARTLINK);
}

int NetworkConnection::stopSmartlink()
{
    return aw_stopcomposite();
}

void NetworkConnection::respone_smartlink()
{
    //TODO  UDP port 20000
    plogd("respone_smartlink !!! %s",mSmartlinkResponeIp.data());
    //sleep(4);//wait for ipv4 addr
    int sock_cli = socket(AF_INET,SOCK_STREAM, 0);

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(20000);
    servaddr.sin_addr.s_addr = inet_addr(mSmartlinkResponeIp.data());

    if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
        ploge("connect failed(%s)",strerror(errno));
        return;
    }
    //to do
    char addr[30];
    memset(addr, 0, sizeof(addr));
    get_mac_addr("wlan0",addr);    //{"ok":"00:00:00:00:00:00"}

    struct json_object *json = json_object_new_object();

    if (!is_error(json)) {
        json_object_object_add(json, "ok", json_object_new_string(addr));
        const char *str = json_object_to_json_string(json);
        ploge("str %s", str);
        send(sock_cli, str, strlen(str), 0);
        json_object_put(json);
    }

    shutdown(sock_cli,SHUT_RDWR);

}

void NetworkConnection::handleMessage(Message *msg)
{
    switch(msg->what){
    case START_WIFI_MANAGER:
        plogd("wifi on now!!");
        mWifiInterface = aw_wifi_on(wifi_event_handle, local_msgid++);
        sleep(1);
        mWifiInterface->connect_ap_auto(1);
        break;

    case START_SMARTLINK:
        if(aw_wifi_get_wifi_state() == WIFIMG_WIFI_BUSING){
            plogd("wifi state busing,waiting");
            sendMessageDelayed(START_SMARTLINK,1,0); //Delay 1s retry
            return;
        }
        mWifiInterface->disconnect_ap(local_msgid++);
        if(aw_smartlinkd_init(0,smartlinkd_event_handle) == 0){
            //aw_startcomposite(AW_SMARTLINKD_PROTO_COOEE | AW_SMARTLINKD_PROTO_ADT);
            aw_startcomposite(AW_SMARTLINKD_PROTO_COOEE);
            //aw_startcomposite(AW_SMARTLINKD_PROTO_ADT);
        }

        break;
    case RESPONE_SMARTLINK:{
        static int count = 0;
        if(count != 0) break;
        if(count == 0){
            mBroadcastSender = new BroadcastSender();
            mBroadcastSender->init(ConfigUtils::SmartlinkdRespBroadcastPort);
            //respone_smartlink();
        }

        while(count++ < 20){
            struct json_object *json = json_object_new_object();

            if (!is_error(json)) {
                json_object_object_add(json, "cmd", json_object_new_string("decovery"));
                json_object_object_add(json, "id", json_object_new_string(ConfigUtils::getUUID()));
                json_object_object_add(json, "type", json_object_new_int(0));
                const char *str = json_object_to_json_string(json);
                ploge("str %s", str);
                mBroadcastSender->send(str, strlen(str));
                json_object_put(json);
            }
            else
                break;
        }

        count = 0;
        mBroadcastSender->release();
        delete mBroadcastSender;

        break;
    }
    default:
        ;
    }
    return;
}

}