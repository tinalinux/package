#include <json-c/json.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#include "parrot_common.h"
#include "RemoteControlor.h"
#include "ConfigUtils.h"

//extent int get_mac_addr(const char* name,char* hwaddr);

static int stringToLength(const char * cLen)
{

    int value;
    value = (int) (((cLen[0] & 0xFF) << 24) | ((cLen[1] & 0xFF) << 16) | ((cLen[2] & 0xFF) << 8) | (cLen[3] & 0xFF));
    return value;
}

static void lengthToString(int length, char * cLen)
{
    cLen[3] = (char) (length & 0xff);
    cLen[2] = (char) (length >> 8 & 0xff);
    cLen[1] = (char) (length >> 16 & 0xff);
    cLen[0] = (char) (length >> 24 & 0xff);
}

static int getHostIp(const char *ifname, char *ip){
    struct ifaddrs * ifAddrStruct=NULL;
    void * tmpAddrPtr=NULL;

    getifaddrs(&ifAddrStruct);

    while (ifAddrStruct != NULL) {
        if (ifAddrStruct->ifa_addr->sa_family == AF_INET) { // check it is IP4
            //is a valid IP4 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            if(!strcmp(ifAddrStruct->ifa_name, ifname)){
                memcpy(ip, addressBuffer, INET_ADDRSTRLEN);
                break;
            }
        }
        ifAddrStruct=ifAddrStruct->ifa_next;
    }
    freeifaddrs(ifAddrStruct);
}

namespace Parrot{

//TODO: get this config from setting file
const char *RemoteControlor::IP = "114.215.158.131";
const int RemoteControlor::PORT = 8081;
//const char *RemoteControlor::DeviceID = "asdfghjkl";
const char *RemoteControlor::DeviceID = "zxcvbnm";

RemoteControlor::RemoteControlor(ref_EventWatcher watcher, ref_HandlerBase base):
    Handler(base)
{
    mWatcher = watcher;
    mBase = base;

    mUserOnlineCount = 0;

    mDecoveryCount = 0;

    mWanFd = -1;
    mLanFd = -1;

    mCheckNetworkflag = false;

    mWanSocketWatcher = new SocketClientWatcher(watcher, base);
    mLanSocketWatcher = new SocketClientWatcher(watcher, base);

    mLanServerWatcher = new SocketServerWatcher(watcher, base);


    mCheckNetworkKeeper = new ActionKeeper(base);
    mPingKeeper = new ActionKeeper(base);
    mWanSyncKeeper = new ActionKeeper(base);
    mDecoveryKeeper = new ActionKeeper(base);


}

RemoteControlor::~RemoteControlor(){
    delete mWanSocketWatcher;
    delete mLanSocketWatcher;
}

int RemoteControlor::init(ref_EventWatcher watcher, ref_HandlerBase base, ControlorBase *controlorbase)
{
    mControlor = controlorbase;
    mWanSocketWatcher->setListener(this);
    mLanSocketWatcher->setListener(this);
    mLanServerWatcher->setListener(this);

    mCheckNetworkKeeper->init(CHECK_NETWORK, this, 2000/*ms*/);
    mPingKeeper->init(PING, this, 8000/*ms*/);
    mWanSyncKeeper->init(SYNC_STATE_TIMER, this, 4000/*ms*/);
    mDecoveryKeeper->init(DECOVERY, this, 1000/*ms*/);
    return SUCCESS;
}

int RemoteControlor::release()
{
    mWanSocketWatcher->setListener(NULL);
    mLanSocketWatcher->setListener(NULL);
    mLanServerWatcher->setListener(NULL);

    mCheckNetworkKeeper->release();
    mPingKeeper->release();
    mWanSyncKeeper->release();
    mDecoveryKeeper->release();
}

int RemoteControlor::start()
{
    //check network first
    //114.215.158.131
    mControlor->getNetworkConnection()->addListener(this);
    mControlor->setControlorStatusListener(this);
    //return sendMessage(CHECK_NETWORK);
    mBroadcastSender.init(ConfigUtils::DecoveryBroadcastPort);
}

int RemoteControlor::send_to_server(const char *buf, int len, bool force_send)
{
    //make package
    if(mUserOnlineCount == 0 && force_send == false){
        ploge("no user online: %d!!",mUserOnlineCount);
        return SUCCESS;
    }

    char *data = (char*)malloc(len + HEAD_LEN + 1);
    memset(data, 0, len + HEAD_LEN + 1);
    lengthToString(len, data);
    memcpy(data + HEAD_LEN, buf, len);

    //if(strcmp(buf, "ping"))
        //ploge("current online user: %d ,send head: 0x%x  %s, len : %d",mUserOnlineCount, *(int*)data,data + HEAD_LEN, len + HEAD_LEN);
    int ret;

    //todo 要考虑有独立的WAN用户的时候
    if(force_send == false && mLanServerWatcher->exsitClient() > 0){
        ploge("LAN send current online user: %d , %s, len : %d",mUserOnlineCount,buf, len);
        ret = mLanServerWatcher->sendtoAllClient(data, len + HEAD_LEN);
        //how to do when send fail
        if(ret < 0) mLanServerWatcher->disconnectAllClient();
    }
    else{
        ploge("WAN send current online user: %d ,send head: 0x%x  %s, len : %d", mUserOnlineCount, *(int*)data,data + HEAD_LEN, len + HEAD_LEN);
        ret = mWanSocketWatcher->sendtoServer(data, len + HEAD_LEN);
        if(ret < 0){
            mPingKeeper->stop();
            mWanSyncKeeper->stop();
            mCheckNetworkKeeper->action();
        }
    }
    free(data);

    return ret;
}

void RemoteControlor::onPPlayerPlayChanged()
{
    //播放状态改变
    if(mWanSocketWatcher->getFd() != -1)
        sendMessage(SYNC_STATE);
}

void RemoteControlor::onPPlayerPlaylistChanged(int index)
{
    //播放列表改变
    //sync_to_server(2);
    //sendMessage(SYNC_LIST);
}

void RemoteControlor::onPPlayerPlaylistupdated()
{
    //播放列表更新
    if(mWanSocketWatcher->getFd() != -1)
        sendMessage(SYNC_LIST);
}

void RemoteControlor::onPPlayerPlaylistReset()
{
    //sendMessage(SYNC_LIST);
}

void RemoteControlor::onControlVolumeChanged(int volume)
{
    if(mWanSocketWatcher->getFd() != -1)
        sendMessage(SYNC_STATE);
}

void RemoteControlor::onControlPlayChanged()
{
    if(mWanSocketWatcher->getFd() != -1)
        sendMessage(SYNC_STATE);
}

void RemoteControlor::onControlModeChanged()
{
    if(mWanSocketWatcher->getFd() != -1)
        sendMessage(SYNC_STATE);
}

void RemoteControlor::onNetworkSmartlinkdDetected()
{
    mDecoveryCount = 50;
}

void RemoteControlor::onNetworkConnected()
{
    mDecoveryKeeper->action();
    //if(mCheckNetworkflag == false) sendMessage(CHECK_NETWORK);
    mCheckNetworkKeeper->action();
}

void RemoteControlor::onNetworkDisconnected()
{
    mDecoveryKeeper->stop();
    mCheckNetworkKeeper->stop();
    mWanSocketWatcher->disconnectServer();

    mUserOnlineCount = 0;

}

int RemoteControlor::onKeeperAction(ActionKeeper *keeper, int key, int *interval)
{
    switch(key){
    case CHECK_NETWORK:{
        if(mControlor->getNetworkConnection()->getWifiStatus() == NetworkConnection::NETWORK_DISCONNECTED){
            //retry
            plogd("NETWORK_DISCONNECTED wait...");
            mCheckNetworkKeeper->stop();
            break;
        }
        ploge("start to connect!!!!!");

        mWanSocketWatcher->disconnectServer();
        if(mWanSocketWatcher->connectServer(IP, PORT) == SUCCESS){
            mWanFd = mWanSocketWatcher->getFd();
            if(login(ConfigUtils::getUUID(), 0) < 0){
                //if fail, retry
                break;
            }
        }

        //todo check errorCode!! ----------------------------------------
        int errorCode;
        mXimalaya.SecureAccessToken(errorCode);
        //---------------------------------------------------------------
        mCheckNetworkKeeper->stop();
        break;
    }
    case PING:{
        //TODO 断网后，ping要退出，等到再一次连上网在ping
        send_to_server("ping", strlen("ping"), true);
        break;
    }
    case DECOVERY:{
        struct json_object *json = json_object_new_object();

        if (!is_error(json)) {
            json_object_object_add(json, "cmd", json_object_new_string("decovery"));
            json_object_object_add(json, "id", json_object_new_string(ConfigUtils::getUUID()));
            json_object_object_add(json, "type", json_object_new_int(mDecoveryCount==0?1:0));
            const char *str = json_object_to_json_string(json);
            ploge("str: %s",str);
            mBroadcastSender.send(str, strlen(str));
            json_object_put(json);
        }

        if(mDecoveryCount > 0){
            *interval = 200;
            mDecoveryCount--;
        }
        else{
            //modify interval
            *interval = 2000;
        }
        break;
    }

    case SYNC_STATE_TIMER:{
        if(mPlayer->isPlaying() == 1) sync_to_server(20000, 1);
        break;
    }

    }
}

void RemoteControlor::onPPlayerFetchPlayListElem(MediaElem *elem)
{
    int ids[1];
    ids[0] = elem->id;
    string result;
    int errorCode;
    mXimalaya.TracksGetBatch(result, errorCode, ids, 1);

    struct json_object *j_result = json_tokener_parse(result.data());

    const char *url_key = "play_url_24_m4a";
    //const char *url_key = "play_url_32";
    //const char *url_key = "play_url_64_m4a";

    struct json_object *j_track;
    struct json_object *item;
    struct json_object *data;
    if(json_object_object_get_ex(j_result, "tracks", &j_track)){
        item = json_object_array_get_idx(j_track, 0);
        if(json_object_object_get_ex(item, url_key, &data)){
            elem->url = json_object_get_string(data);
        }
        if(json_object_object_get_ex(item, "track_title", &data)){
            elem->title = json_object_get_string(data);
        }
        ploge("url: %s", elem->url.data());

    }
}

int RemoteControlor::onSocketServerRead(int clientfd, char *buf, int len)
{
    if(len == 0){
        plogd("Client close the connection fd: %d", clientfd);
        return 0;
    }
    int data_len = stringToLength(buf);
    if(data_len != len - HEAD_LEN){
        ploge("get WAN data error");
        return FAILURE;
    }
    ploge("get LAN data: head 0x%x, %s, len %d", *(int*)buf, buf + HEAD_LEN, len);
    return do_proto_data(buf + HEAD_LEN, len);
    //return onSocketClientRead(mWanSocketWatcher->getFd(), buf, len);
}

int RemoteControlor::onSocketClientRead(int socketfd, char *buf, int len)
{
    if(len == 0){
        plogd("RemoteControlor Server close the connection");
        return 0;
    }
    char *data;
    if(socketfd == mLanSocketWatcher->getFd()){
        int data_len = stringToLength(buf);
        if(data_len != len - HEAD_LEN){
            ploge("get WAN data error");
            return FAILURE;
        }
        data = buf + HEAD_LEN;
        ploge("get LAN data: head 0x%x, %s, len %d", *(int*)buf, buf + HEAD_LEN, len);
    }else if(socketfd == mWanSocketWatcher->getFd()){
        int data_len = stringToLength(buf);
        if(data_len != len - HEAD_LEN){
            ploge("get WAN data error");
            return FAILURE;
        }
        data = buf + HEAD_LEN;
        ploge("get WAN data: head 0x%x, %s, len %d", *(int*)buf, buf + HEAD_LEN, len);
    }

    return do_proto_data(data, len);
}
int RemoteControlor::do_proto_data(const char *buf, int len)
{
    struct json_object *json = json_tokener_parse(buf);

    if (!is_error(json)) {
        struct json_object *json_cmd;
        if (json_object_object_get_ex(json, "cmd", &json_cmd)) {
            const char* cmd = json_object_get_string(json_cmd);
            if (!strcmp(cmd, "devlogin")) {
                do_cmd_login(json);
            } else if(!strcmp(cmd, "online")) {
                do_cmd_online(json);
            } else if(!strcmp(cmd, "offline")){
                do_cmd_offline(json);
            } else if(!strcmp(cmd, "bind")){
                do_cmd_bind(json);
            } else if(!strcmp(cmd, "unbind")){
                do_cmd_unbind(json);
            } else if(!strcmp(cmd, "audio")){
                do_cmd_audio(json);
            } else if(!strcmp(cmd, "updatelist")){
                do_cmd_updatelist(json);
            } else if(!strcmp(cmd, "volume")){
                do_cmd_volume(json);
            } else if(!strcmp(cmd, "getstatelist")){
                do_cmd_getstatelist(json);
            } else if(!strcmp(cmd, "deletelocalfiles")){
                do_cmd_deletelocalfiles(json);
            } else if(!strcmp(cmd, "version")){
                do_cmd_version(json);
            } else if(!strcmp(cmd, "getstorage")){
                do_cmd_getstorage(json);
            } else if(!strcmp(cmd, "shutdown")){
                do_cmd_shutdown(json);
            } else if(!strcmp(cmd, "upgrade")){
                do_cmd_upgrade(json);
            } else if(!strcmp(cmd, "modifyname")){
                do_cmd_modifyname(json);
            } else if(!strcmp(cmd, "networkconnect")){
                do_cmd_networkconnect(json);
            } else if(!strcmp(cmd, "devicereset")){
                do_cmd_devicereset(json);
            } else if(!strcmp(cmd, "ximalaya")){
                do_cmd_ximalaya(json);
            } else if(!strcmp(cmd, "directconn")){
                do_cmd_directconn(json);
            }
        }
    }

    json_object_put(json);
}
void RemoteControlor::handleMessage(Message *msg)
{
    switch(msg->what){
    case SYNC_ALL:{
        sync_to_server(10,3);
        break;
    }
    case SYNC_LIST:{
        sync_to_server(10,2);
        break;
    }
    case SYNC_STATE:{
        sync_to_server(10,1);
        break;
    }
    }
}

int RemoteControlor::login(const char *device_id, int type)
{
    int ret = SUCCESS;
    struct json_object *json = json_object_new_object();

    if (!is_error(json)) {
        json_object_object_add(json, "cmd", json_object_new_string("devlogin"));
        json_object_object_add(json, "type", json_object_new_int(type));
        json_object_object_add(json, "id", json_object_new_string(device_id));
        json_object_object_add(json, "cmdid", json_object_new_int(10086));
        const char *str = json_object_to_json_string(json);

        ret  = send_to_server(str,strlen(str), true);
    }

    json_object_put(json);

    return ret;
}

int RemoteControlor::syncstate()
{

}
/*
    　　‘index’:[int] //主键，所在播放列表的位置，必选
    　　‘srctype’:[int] //音乐资源类型 1喜马拉雅 2虾米 3音箱本地音乐，必选
    　　‘id’:[int], //音乐ID，srctype=1或2时必选
    　　‘musictype’:[int], //音乐类型， 1点播 2直播电台， srctype=1时必选
    　　‘url’:[string], //歌曲url，APP->DEVICE且srctype=2时必选
    　　DEVICE->APP无此项
    　　‘title’:[string] //音乐名，选填 APP->DEVICE无此项
    　　‘singer’:[string] // 演唱者，选填 APP->DEVICE无此项
    　　‘album’:[string] //所属专辑，选填 APP->DEVICE无此项
    　　‘image’:[string]//专辑封面，选填 APP->DEVICE无此项
    　　‘lrc’:[string]//歌词链接，选填 APP->DEVICE无此项

        int index;

        enum _SrcType type;

        string img;
        string artist;
        string title;
        string imgSize;
        string url;
        string lrcurl;
        string album;

        string id;
*/

int RemoteControlor::sync_to_server(int cmdid, int subcmd)
{
    list<MediaElem*> playlist = mPlayer->getPlayList();
    int list_size = playlist.size();
    int ret = SUCCESS;
    //ploge("sync to server list size: %d",list_size);
    struct json_object *json = json_object_new_object();

    if (is_error(json)) {
        ploge("error json object!");
        return FAILURE;
    }

    json_object_object_add(json, "cmd", json_object_new_string("devupdate"));
    json_object_object_add(json, "subcmd", json_object_new_int(subcmd));
    json_object_object_add(json, "cmdid", json_object_new_int(cmdid));

    if(subcmd == 2 || subcmd == 3){
        struct json_object *j_list = json_object_new_array();

        //list
        list<MediaElem*>::iterator it;
        for(it = playlist.begin(); it != playlist.end(); it++){
            struct json_object *j_elem = json_object_new_object();

            MediaElem *elem = *it;
            json_object_object_add(j_elem, "index", json_object_new_int(elem->index));
            json_object_object_add(j_elem, "srctype", json_object_new_int(elem->type));
            if(elem->type == MediaElem::MEDIA_XIMALAYA || elem->type == MediaElem::MEDIA_XIAMI){
                json_object_object_add(j_elem, "id", json_object_new_int(elem->id));
            }
            if(elem->title.size() > 0)
                json_object_object_add(j_elem, "title", json_object_new_string(elem->title.data()));
            if(elem->artist.size() > 0)
                json_object_object_add(j_elem, "singer", json_object_new_string(elem->artist.data()));
            if(elem->artist.size() > 0)
                json_object_object_add(j_elem, "image", json_object_new_string(elem->img.data()));
            if(elem->lrcurl.size() > 0)
                json_object_object_add(j_elem, "lrc", json_object_new_string(elem->lrcurl.data()));

            json_object_array_add(j_list, j_elem);
        }
        json_object_object_add(json, "list", j_list);

    }
    if(subcmd == 1 || subcmd == 3){
/*
    　　‘devmode’:[int], //音箱模式1 wifi在线音乐2 bt 3 line in 4音箱本地，必选
    　　‘volume’:[int], //音量值百分比，必选
    　　‘state’:[int],//1 播放 2暂停 3停止，必选
    　　‘pos’:[int], //当前播放音乐在列表中的位置，state=1或2时必选
    　　‘current’:[int], //当前播放到哪里， 单位秒 ，state=1或2时必选
    　　‘duration’:[int] //当前播放音乐的总时长，单位秒，DEVICE->APP且state=1或2时必选
*/
        struct json_object *j_state = json_object_new_object();

        int devmode = 1; //apk push
        Mode *curmode = mControlor->getCurMode();
        if(curmode != NULL){
            if(curmode->getModeIndex() == Mode::MODE_SPEECH) //local music
                devmode = 4;
            else if(curmode->getModeIndex() == Mode::MODE_LINEIN)
                devmode = 3;
            else if(curmode->getModeIndex() == Mode::MODE_BT)
                devmode = 2;
        }
        json_object_object_add(j_state, "devmode", json_object_new_int( devmode ));
        json_object_object_add(j_state, "volume",  json_object_new_int( mPlayer->getAudioSetting()->getVirtualVolume() ));
        json_object_object_add(j_state, "state",   json_object_new_int( mPlayer->isPlaying()==1?1:2 ));
        json_object_object_add(j_state, "pos",     json_object_new_int( mPlayer->getCurPlayListElemIndex() ));

        int current;
        mPlayer->getCurrentPosition(&current); //ms
        json_object_object_add(j_state, "current", json_object_new_int(current/1000));
        mPlayer->getDuration(&current);
        json_object_object_add(j_state, "duration", json_object_new_int(current/1000));
        json_object_object_add(json, "state", j_state);
    }
    const char *str = json_object_to_json_string(json);

    ret  = send_to_server(str, strlen(str), false);

    json_object_put(json);

    return ret;

}

int RemoteControlor::do_cmd_login(struct json_object *json)
{
    /*
    {
    　　‘cmd’:’devlogin’,
    　　‘cmdid’:[int],
    　　‘ret’:’[int]’//操作结果， 0登录成功， 1 登录失败
　　}
    */
    struct json_object *j_ret;
    if (json_object_object_get_ex(json, "ret", &j_ret)) {
        int i_ret = json_object_get_int(j_ret);
        if (i_ret == 0) {
            // i_ret==0表示操作成功
            plogd("login success! start keep ping");

            mPingKeeper->action();
            mWanSyncKeeper->action();
            sendMessage(SYNC_ALL);

        }
        else {
            // i_ret!=0表示操作失败
            plogd("login failed retry!");
            mCheckNetworkKeeper->action();
        }
    }

    return SUCCESS;
}

int RemoteControlor::do_cmd_online(struct json_object *json)
{
    /*
    　　{
        　　‘cmd’:’online’,
        　　‘user’:’[string]’//设备的MAC地址，或app用户名
    　　}
    */

    struct json_object *j_bindtype;
    struct json_object *j_user;
    int i_bindtype = 1;
    if(json_object_object_get_ex(json, "bindtype", &j_bindtype)){
        i_bindtype = json_object_get_int(j_bindtype);
    }
    if (json_object_object_get_ex(json, "user", &j_user)) {
        const char *s_user = json_object_get_string(j_user);
        plogd("online : %s", s_user);

        mOnlineUserMap[s_user].bindtype = i_bindtype;
        mUserOnlineCount = mOnlineUserMap.size();
    }
    return SUCCESS;

}

int RemoteControlor::do_cmd_offline(struct json_object *json)
{
    /*
    　　{
        　　‘cmd’:’online’,
        　　‘user’:’[string]’//设备的MAC地址，或app用户名
    　　}
    */
    /*
    struct json_object *j_user;
    if (json_object_object_get_ex(json, "user", &j_user)) {
        const char *s_user = json_object_get_string(j_user);
        plogd("offline : %s", s_user);

        mOnlineUserMap.erase(s_user);
        mUserOnlineCount = mOnlineUserMap.size();
    }
    return SUCCESS;
    */
}

int RemoteControlor::do_cmd_bind(struct json_object *json)
{
    struct json_object *j_bindtype;
    struct json_object *j_user;
    int i_bindtype = 1;
    if(json_object_object_get_ex(json, "bindtype", &j_bindtype)){
        i_bindtype = json_object_get_int(j_bindtype);
    }
    if (json_object_object_get_ex(json, "user", &j_user)) {
        const char *s_user = json_object_get_string(j_user);
        plogd("online : %s", s_user);

        mOnlineUserMap[s_user].bindtype = i_bindtype;
        mUserOnlineCount = mOnlineUserMap.size();
    }
    return SUCCESS;
}
int RemoteControlor::do_cmd_unbind(struct json_object *json)
{
    struct json_object *j_user;
    if (json_object_object_get_ex(json, "user", &j_user)) {
        const char *s_user = json_object_get_string(j_user);
        plogd("offline : %s", s_user);

        mOnlineUserMap.erase(s_user);
        mUserOnlineCount = mOnlineUserMap.size();
    }
    return SUCCESS;
}

int RemoteControlor::do_cmd_directconn(struct json_object *json)
{
    struct json_object *j_cmdid;
    int i_cmdid = 0;

    if(json_object_object_get_ex(json, "cmdid", &j_cmdid)){
        i_cmdid = json_object_get_int(j_cmdid);
    }

    int retry = 10;
    int port = 20000;
    if(mLanServerWatcher->isServerWork()){
        port = mLanServerWatcher->getPort();
        plogd("server is already exsit! port: %d", port);
    }else{
        while(mLanServerWatcher->startServer(port) != SUCCESS && retry-- > 0){
            port++;
        }
        if(retry == 0){
            ploge("startServer failed! port: %d",port);
            //return FAILURE;
        }
    }
    char addressBuffer[INET_ADDRSTRLEN];
    getHostIp("wlan0", addressBuffer);
    struct json_object *resq = json_object_new_object();
    json_object_object_add(resq, "cmd", json_object_new_string("directconn"));
    json_object_object_add(resq, "ret", json_object_new_int(retry!=0?0:1));
    json_object_object_add(resq, "cmdid", json_object_new_int(i_cmdid));
    json_object_object_add(resq, "port", json_object_new_int(port));
    json_object_object_add(resq, "ip", json_object_new_string(addressBuffer));
    const char *str = json_object_to_json_string(resq);
    send_to_server(str, strlen(str), false);
    json_object_put(resq);

    return SUCCESS;

/*

    struct json_object *j_ip;
    struct json_object *j_port;

    int i_cmdid;
    const char *s_ip;
    int i_port;

    if(json_object_object_get_ex(json, "cmdid", &j_cmdid)){
        i_cmdid = json_object_get_int(j_cmdid);
    }
    if(json_object_object_get_ex(json, "ip", &j_ip)){
        s_ip = json_object_get_string(j_ip);
    }
    if(json_object_object_get_ex(json, "port", &j_port)){
        i_port = json_object_get_int(j_port);
    }
    if(mLanSocketWatcher->getFd() != -1) {
        plogd("Lan socket is already exsit!!");
        mLanSocketWatcher->disconnectServer();
    }
    if(mLanSocketWatcher->connectServer(s_ip, i_port) == SUCCESS){
        struct json_object *resq = json_object_new_object();
        json_object_object_add(resq, "cmd", json_object_new_string("directconn"));
        json_object_object_add(resq, "cmdid", json_object_new_int(i_cmdid));
        json_object_object_add(resq, "id", json_object_new_string(ConfigUtils::getUUID()));
        const char *str = json_object_to_json_string(resq);
        send_to_server(str, strlen(str), false);
        json_object_put(resq);
    }
*/
}

/*

音乐列表
　　{
    　　‘index’:[int] //主键，所在播放列表的位置，必选
    　　‘srctype’:[int] //音乐资源类型 1喜马拉雅 2虾米 3音箱本地音乐，必选
    　　‘id’:[int], //音乐ID，srctype=1或2时必选
    　　‘musictype’:[int], //音乐类型， 1点播 2直播电台， srctype=1时必选
    　　‘url’:[string], //歌曲url，APP->DEVICE且srctype=2时必选
    　　DEVICE->APP无此项
    　　‘title’:[string] //音乐名，选填 APP->DEVICE无此项
    　　‘singer’:[string] // 演唱者，选填 APP->DEVICE无此项
    　　‘album’:[string] //所属专辑，选填 APP->DEVICE无此项
    　　‘image’:[string]//专辑封面，选填 APP->DEVICE无此项
    　　‘lrc’:[string]//歌词链接，选填 APP->DEVICE无此项
　　}
*/
/*
控制、状态列表
　　{
    　　‘devmode’:[int], //音箱模式1 wifi在线音乐2 bt 3 line in 4音箱本地，必选
    　　‘volume’:[int], //音量值百分比，必选
    　　‘state’:[int],//1 播放 2暂停 3停止，必选
    　　‘pos’:[int], //当前播放音乐在列表中的位置，state=1或2时必选
    　　‘current’:[int], //当前播放到哪里， 单位秒 ，state=1或2时必选
    　　‘duration’:[int] //当前播放音乐的总时长，单位秒，DEVICE->APP且state=1或2时必选
　　}
*/
int RemoteControlor::set_state(struct json_object *json)
{

}

int RemoteControlor::cmd_respone_to_server(const char *cmd, int cmdid, int ret)
{
    struct json_object *resq = json_object_new_object();
    json_object_object_add(resq, "cmd", json_object_new_string(cmd));
    json_object_object_add(resq, "cmdid", json_object_new_int(cmdid));
    json_object_object_add(resq, "ret", json_object_new_int(ret));
    const char *str = json_object_to_json_string(resq);
    send_to_server(str, strlen(str), false);
    json_object_put(resq);
}

int RemoteControlor::do_cmd_audio(struct json_object *json)
{
    /*
　　请求
　　{
    　　‘cmd’:’audio’,
    　　‘cmdid’:[int],
    　　‘subcmd’:[int],//控制命令 1播放 2暂停 3上一曲 4下一曲 5 volume 6 playmode
    　　Position[int] ：   //当subcmd为1（播放），携带position和seek
    　　Seek[int]：        //百分比，-1表示进度不变
    　　Volume[int]：  //百分比
    　　Playmode[int]：    //1顺序播放；2随机播放 ；3 单曲播放
　　}
　　返回
　　{
    　　‘cmd’:’audio’,
    　　‘cmdid’:[int],
    　　‘ret’:’[int]’//操作结果， 0成功， 1 失败
　　}
    */
    //just mode speech
    if(mControlor->getCurMode()->getModeIndex() != Mode::MODE_SPEECH){
        return SUCCESS;
    }
    struct json_object *j_cmdid;
    int i_cmdid;
    if(json_object_object_get_ex(json, "cmdid", &j_cmdid)){
        i_cmdid = json_object_get_int(j_cmdid);
    }

    struct json_object *j_subcmd;
    if (json_object_object_get_ex(json, "subcmd", &j_subcmd)) {
        int i_subcmd = json_object_get_int(j_subcmd);
        switch(i_subcmd){
        case 1:{

            cmd_respone_to_server("audio", i_cmdid, 0);

            struct json_object *j_Position;
            struct json_object *j_Seek;
            int position = -1;
            int seek = -1;
            if(json_object_object_get_ex(json, "Position", &j_Position)){

                position = json_object_get_int(j_Position);
            }
            if(json_object_object_get_ex(json, "Seek", &j_Seek)){
                ploge("do seek and play");
                seek = json_object_get_int(j_Seek);
            }

            if(position == -1 && seek > 0){
                int duration;
                mPlayer->getDuration(&duration);
                duration = seek*duration/100;
                ploge("do seek %d",duration);
                //mPlayer->seekTo(duration);
                mControlor->doControlSeek(duration);
            } else if(position != -1){
                ploge("do select and play position: %d",position);
                //mPlayer->playlist(position, 0);
                mControlor->doControlPlaylist(position, 0);
            } else{
                //mPlayer->start();
                mControlor->doControlPlay();
            }


/*
            if(position != -1){
                ploge("do select and play position: %d",position);
                mPlayer->playlist(position, 0);
            }else if(seek > 0){
                int duration;
                mPlayer->getDuration(&duration);
                duration = seek*duration/100;
                ploge("do seek %d",duration);
                mPlayer->seekTo(duration);

            }else{
                mPlayer->start();
            }
            //just play
            //mControlor->doControlPlay();
*/
            //cmd_respone_to_server("audio", i_cmdid, 0);
            //sendMessage(SYNC_STATE);
            break;
        }
        case 2:{
            //mPlayer->pause();
            cmd_respone_to_server("audio", i_cmdid, 0);
            mControlor->doControlPause();
            //sendMessage(SYNC_STATE);
            break;
        }
        case 3:{
            cmd_respone_to_server("audio", i_cmdid, 0);
            mControlor->doControlPlayPrev();
            //sendMessage(SYNC_STATE);
            break;
        }
        case 4:{
            cmd_respone_to_server("audio", i_cmdid, 0);
            mControlor->doControlPlayNext();
            //sendMessage(SYNC_STATE);
            break;
        }
        case 5:{
            //volume
            struct json_object *j_volume;
            if(json_object_object_get_ex(json, "Volume", &j_volume)){
                int i_volume = json_object_get_int(j_volume);
                if(i_volume < 0 || i_volume > 100){
                    cmd_respone_to_server("audio", i_cmdid, 1);
                    break;
                }
                mPlayer->getAudioSetting()->setVirtualVolume(i_volume);
                cmd_respone_to_server("audio", i_cmdid, 0);
                sendMessage(SYNC_STATE);
            }

            break;
        }
        case 6:{
            //playmode
            break;
        }

        default:
            ;
        }
    }


    return SUCCESS;

}

int RemoteControlor::do_cmd_updatelist(struct json_object *json)
{
/*
　　请求
　　{
    　　‘cmd’:’updatelist’,
    　　‘cmdid’:[int],
    　　‘list’:[
    　　{
        　　‘index’:[int] //主键，所在播放列表的位置，必选
        　　‘srctype’:[int] //音乐资源类型 1喜马拉雅 2虾米 3音箱本地音乐，必选
        　　‘id’:[int], //音乐ID，srctype=1或2时必选
        　　‘musictype’:[int], //音乐类型， 1点播 2直播电台， srctype=1时必选
        　　‘url’:[string], //歌曲url，APP->DEVICE且srctype=2时必选DEVICE->APP无此项
    　　},
    　　...
    　　]
　　}
　　返回
　　{
    　　‘cmd’:’updatelist’,
    　　‘cmdid’:[int],
    　　‘ret’:’[int]’
　　}
*/
    struct json_object *j_cmdid;
    int i_cmdid = -1;
    if(json_object_object_get_ex(json, "cmdid", &j_cmdid)){
        i_cmdid = json_object_get_int(j_cmdid);
    }
    struct json_object *j_list;
    if(json_object_object_get_ex(json, "list", &j_list)){
        int len = json_object_array_length(j_list);
        ploge("update  list size: %d", len);

        mPlayer->resetPlayList();

        struct json_object *data;
        struct json_object *item;
        for(int i = 0; i < len; i++){

            item = json_object_array_get_idx(j_list, i);

            MediaElem *elem = new MediaElem();

            if(json_object_object_get_ex(item, "index", &data)){
                int index = json_object_get_int(data);
                ploge("update index : %d", index);
                elem->index = index;
            }else{
                ploge("no index !!");
                return FAILURE;
            }

            if(json_object_object_get_ex(item, "srctype", &data)){
                int srctype = json_object_get_int(data);
                if(srctype == 1)
                    elem->type = MediaElem::MEDIA_XIMALAYA;
                else
                    elem->type = MediaElem::MEDIA_XIAMI;
                if(srctype == 1){ //ximalaya
                    if(json_object_object_get_ex(item, "musictype", &data))
                        elem->musictype = json_object_get_int(data);
                    if(json_object_object_get_ex(item, "id", &data))
                        elem->id = json_object_get_int(data);
                    if(json_object_object_get_ex(item, "title", &data))
                        elem->title = json_object_get_string(data);
                    if(json_object_object_get_ex(item, "singer", &data))
                        elem->artist = json_object_get_string(data);

                    mPlayer->addPlayList(elem);
                }else if(srctype == 2){ //xiami
                    if(json_object_object_get_ex(item, "url", &data)){
                        const char *url = json_object_get_string(data);
                        //do
                        mPlayer->addPlayList(elem);
                    }
                }
            }
        }
    }
    cmd_respone_to_server("updatelist", i_cmdid, 0);
    mPlayer->updatePlayList();
}
int RemoteControlor::do_cmd_volume(struct json_object *json)
{
/*
　　请求
　　{
    　　‘cmd’:’volume’,
    　　‘cmdid’:[int],
    　　‘value’:[int]//要设置的音量的百分比
　　}
　　返回
　　{
    　　‘cmd’:’volume’,
    　　‘cmdid’:[int],
    　　‘state’:{
    　　//详见 数据结构-》控制状态列表
    　　}
　　}
*/
    struct json_object *j_value;
    if(json_object_object_get_ex(json, "value", &j_value)){
        int i_value = json_object_get_int(j_value);
    }
}

int RemoteControlor::do_cmd_getstatelist(struct json_object *json)
{
/*
　　请求
　　{
    　　‘cmd’:’getstatelist’,
    　　‘cmdid’:[int],
    　　‘subcmd’:[int] , // 1获取状态 2 获取列表 3同时获取状态和列表
　　}
　　返回
　　{
    　　‘cmd’:’getstatelist’,
    　　‘cmdid’:[int],
    　　‘subcmd’:[int] ,  // 1获取状态 2 获取列表 3同时获取状态和列表
    　　‘state’:
    　　{
        　　//详见 数据结构-》控制状态列表
    　　}
    　　‘list’:[
        　　{
        　　‘list’//详见 数据结构-》音乐列表 章节
        　　},
        　　...
    　　]
　　}
*/
    struct json_object *j_subcmd;
    struct json_object *j_cmdid;
    int i_cmdid = -1;
    if(json_object_object_get_ex(json, "cmdid", &j_cmdid)){
        i_cmdid = json_object_get_int(j_cmdid);
    }
    if(json_object_object_get_ex(json, "subcmd", &j_subcmd)){
        int i_subcmd = json_object_get_int(j_subcmd);
        cmd_respone_to_server("getstatelist", i_cmdid, 0);
        if(i_subcmd == 1) {
            //return state
            sendMessage(SYNC_STATE);
        }else if (i_subcmd == 2) {
            //return playlist
            sendMessage(SYNC_LIST);
        }else if (i_subcmd == 3) {
            // return state playlist
            sendMessage(SYNC_ALL);
        }
    }

}

int RemoteControlor::do_cmd_deletelocalfiles(struct json_object *json)
{

}

int RemoteControlor::do_cmd_version(struct json_object *json)
{

}

int RemoteControlor::do_cmd_getstorage(struct json_object *json)
{

}

int RemoteControlor::do_cmd_shutdown(struct json_object *json)
{

}

int RemoteControlor::do_cmd_upgrade(struct json_object *json)
{

}

int RemoteControlor::do_cmd_modifyname(struct json_object *json)
{

}

int RemoteControlor::do_cmd_networkconnect(struct json_object *json)
{

}

int RemoteControlor::do_cmd_devicereset(struct json_object *json)
{

}

int RemoteControlor::do_cmd_ximalaya(struct json_object *json)
{

}


}