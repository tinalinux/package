#ifndef __REMOTE_CONTROLOR_H__
#define __REMOTE_CONTROLOR_H__

#include <map>
#include <locker.h>
#include <json-c/json.h>

#include "Handler.h"
#include "ControlorBase.h"
#include "SocketClientWatcher.h"
#include "PPlayer.h"
#include "XimalayaApi.h"
#include "NetworkConnection.h"
#include "BroadcastSender.h"
#include "SocketServerWatcher.h"

#include "ActionKeeper.h"

namespace Parrot{

class RemoteControlor : public Handler,
                        public SocketClientWatcher::SocketListener,
                        public SocketServerWatcher::SocketListener,
                        public PPlayer::PPlayerPlaylistListener,
                        public NetworkConnection::NetworkConnectionLinstener,
                        public ControlorBase::ControlorStatusListener,
                        public ActionKeeper::KeeperAction
{
public:
    RemoteControlor(ref_EventWatcher watcher, ref_HandlerBase base);
    ~RemoteControlor();

    static const char *IP; //??
    static const int PORT;
    static const char *DeviceID;
    static const int HEAD_LEN = 4;
    int init(ref_EventWatcher watcher, ref_HandlerBase base, ControlorBase *controlorbase);
    int release();

    int start();

    int setPlayer(ref_PPlayer player){
        mPlayer = player;
        mPlayer->setPlaylistListener(this);
    };

    virtual void handleMessage(Message *msg);

protected:
    int onSocketServerRead(int clientfd, char *buf, int len);
    int onSocketClientRead(int socketfd, char *buf, int len);
    void onPPlayerPlayChanged();
    void onPPlayerPlaylistChanged(int index);
    void onPPlayerPlaylistupdated();
    void onPPlayerPlaylistReset();
    void onPPlayerFetchPlayListElem(MediaElem *elem);

    void onNetworkConnected();
    void onNetworkDisconnected();
    void onNetworkSmartlinkdDetected();

    void onControlVolumeChanged(int volume);
    void onControlPlayChanged();
    void onControlModeChanged();

    int onKeeperAction(ActionKeeper *keeper, int key, int *interval);

private:
    typedef struct _UserInfo
    {
        int bindtype;
        int is_lan;
        int lan_fd;
    }tUserInfo;

    typedef enum _CtlCmd
    {
        CHECK_NETWORK,
        PING,
        DECOVERY,
        SYNC_ALL,
        SYNC_LIST,
        SYNC_STATE,
        SYNC_STATE_TIMER,
    }tCtlCmd;

    int login(const char *device_id, int type);
    int syncstate();
    int sync_to_server(int cmdid, int subcmd);

    int cmd_respone_to_server(const char *cmd, int cmdid, int ret);

    int do_proto_data(const char *buf, int len);
    /*handler remote msg cmd*/

    //login online offline
    int do_cmd_login(struct json_object *json);
    int do_cmd_online(struct json_object *json);
    int do_cmd_offline(struct json_object *json);

    int do_cmd_bind(struct json_object *json);
    int do_cmd_unbind(struct json_object *json);

    //play
    int do_cmd_audio(struct json_object *json);
    int do_cmd_updatelist(struct json_object *json);
    int do_cmd_volume(struct json_object *json);
    int do_cmd_getstatelist(struct json_object *json);

    int do_cmd_directconn(struct json_object *json);

    //settting
    int do_cmd_deletelocalfiles(struct json_object *json);
    int do_cmd_version(struct json_object *json);
    int do_cmd_getstorage(struct json_object *json);
    int do_cmd_shutdown(struct json_object *json);
    int do_cmd_upgrade(struct json_object *json);
    int do_cmd_modifyname(struct json_object *json);
    int do_cmd_networkconnect(struct json_object *json);
    int do_cmd_devicereset(struct json_object *json);

    //ximalaya
    int do_cmd_ximalaya(struct json_object *json);

    //state
    int set_state(struct json_object *json);

    //list

    //
    int send_to_server(const char *buf, int len, bool force_send);

private:
    ref_EventWatcher mWatcher;
    ref_HandlerBase mBase;
    ref_PPlayer mPlayer;

    ControlorBase *mControlor;

    SocketServerWatcher *mLanServerWatcher;
    SocketClientWatcher *mWanSocketWatcher;
    SocketClientWatcher *mLanSocketWatcher;
    int mWanFd;
    int mLanFd;

    bool mCheckNetworkflag;
    locker mCheckNetworkLock;

    ActionKeeper *mCheckNetworkKeeper;
    ActionKeeper *mPingKeeper;
    ActionKeeper *mWanSyncKeeper;
    ActionKeeper *mDecoveryKeeper;
    //ActionKeeper *mLanSyncKeeper;

    allwinner::XimalayaApi mXimalaya;

    int mUserOnlineCount;
    map<string, tUserInfo> mOnlineUserMap;
    //string mUser;

    int mDecoveryCount;

    BroadcastSender mBroadcastSender;

    cond mCond;
};/*class RemoteControlor*/

}/*namespace Parrot*/

#endif /*__REMOTE_CONTROLOR_H__*/