#ifndef __WIFICONNECTION_H__
#define __WIFICONNECTION_H__

#include <list>
#include <string>
#include <stdio.h>

#include <wifi_intf.h>
#include <locker.h>

#include "StateManager.h"
#include "Handler.h"
#include "BroadcastSender.h"

namespace Parrot{
class NetworkTask
{
public:
    NetworkTask():data(NULL){};
    int cmd;
    void* data;
};
class NetworkConnection : public Handler
{
public:
    class NetworkConnectionLinstener
    {
    public:
        typedef enum _NotifyId
        {
            SmartlinkdDetected,
            SmartlinkdFailed,
            Connected,
            ConnectFailed,
            Disconnected
        }tNotifyId;
        virtual void onNetworkSmartlinkdFailed(){};
        virtual void onNetworkSmartlinkdDetected(){};
        virtual void onNetworkConnected(){};
        virtual void onNetworkConnectFailed(){};
        virtual void onNetworkDisconnected(){};
    };
public:
    static NetworkConnection* createInstance(ref_HandlerBase base);
    static NetworkConnection* getInstance();
    static void realeseInstance();

private:
    NetworkConnection(ref_HandlerBase base);
    ~NetworkConnection();
    static NetworkConnection *mInstance;
    static locker *mInstanceLock;

public:
    //specific Wifi Msg lebel (<100);
    enum WifiMsgID
    {
        SMARTLINKD_CONNECT = 0x01,
    };
    typedef enum Status
    {
        NETWORK_CONNECTED = 0,
        NETWORK_DISCONNECTED,
    }tNetworkStatus;

    typedef enum NetworkTaskCmd
    {
        START_WIFI_MANAGER = 0xf0,
        START_SMARTLINK,
        RESPONE_SMARTLINK,
    }tNetworkTaskCmd;

    int init();
    int release();

    void setListener(NetworkConnectionLinstener *l){
        mListener = l;
    };

    void addListener(NetworkConnectionLinstener *l);


    int startSmartlink();
    int stopSmartlink();

    //callback form WifiManager
    void onWifiManagerEvent(tWIFI_EVENT wifi_event,void *buf, int msgid);
    //callback form Smartlinkd
    void onSmartlinkdEvent(char *buf, int length);

    tNetworkStatus getWifiStatus();

    virtual void handleMessage(Message *msg);

protected:
    void onEvent(EventWatcher::tEventType type,int fd,void* args);

private:
    void respone_smartlink();
    void notify_all_listener(NetworkConnectionLinstener::tNotifyId id);

private:
    const aw_wifi_interface_t *mWifiInterface;
    sem mWifiInterfaceSem;
    NetworkConnectionLinstener *mListener;
    tNetworkStatus mStatus;
    StateManager mWifiStatus;

    string mSmartlinkResponeIp;

    BroadcastSender *mBroadcastSender;

    list<NetworkConnectionLinstener*> mListenerList;
};
}

#endif /*__NetworkCONNECTION_H__*/
