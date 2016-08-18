#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <json-c/json.h>

#include "parrot_common.h"
#include "Speech.h"

namespace Parrot{

const char* Speech::IP = "127.0.0.1";
const int Speech::PORT = 50002;

Speech::Speech(ref_EventWatcher watcher, ref_HandlerBase handlerbase):
    Mode(watcher,handlerbase),
    mSocketClientWatcher(watcher,handlerbase)
{
    mMode = MODE_SPEECH;
    mShouldStart = false;
}

Speech::~Speech()
{

}

int Speech::onCreate()
{
    plogd("Specch Mode onCreate!");
    //建立连接,可以推荐列表歌曲
    CHECK( mSocketClientWatcher.connectServer(IP,PORT) );
    mSocketClientWatcher.setListener(this);
    //enableSmartVoice(false); // default setVoice false
    Mode::onCreate();
    return SUCCESS;
}

int Speech::onStart()
{
    plogd("Specch Mode onStart!");
    //使能语音
    if(!existPlayer()){
        ploge("Speech mode need player, please setPlayer!");
        return FAILURE;
    }
    //mPlayer->reset();
    //mPlayer->resetPlayList();
    mPlayer->setPlayerListener(this);
    setNetworkStatus("connected");

    CHECK( enableSmartVoice(true) );

    if(mPlayer->getPlayListSize() > 0){
        mPlayer->playlist();
        setPlaybackState("playing");
    }else{
        setPlaybackState("stop");
        playDefaultList();
    }

    Mode::onStart();
    return SUCCESS;
}

int Speech::onStop()
{
    plogd("Specch Mode onStop!");
    //关闭语音
    setPlaybackState("stop");
    enableSmartVoice(false);
    //mPlayer->pause();
    mPlayer->reset();
    //mPlayer->resetPlayList();

    Mode::onStop();
    return SUCCESS;
}

int Speech::onDestory()
{
    plogd("Specch Mode onDestory!");
    mPlayer->reset();
    mPlayer->setPlayerListener(NULL);
    mPlayer->resetPlayList();
    mSocketClientWatcher.disconnectServer();
    Mode::onDestory();
    return SUCCESS;
}

int Speech::play()
{
    if(mPlayer->start() > 0)
        return setPlaybackState("playing");
}

int Speech::pause()
{
    if(mPlayer->pause() > 0)
        return setPlaybackState("paused");
}

int Speech::next()
{
    return mPlayer->next();
}

int Speech::prev()
{
    return mPlayer->prev();
}

int Speech::switchplay()
{
    if(mPlayer->isPlaying() == 1)
        return pause();
    else
        return play();
}

int Speech::setNetworkStatus(const char* state)
{
    return publish_to_server("wifi",state);
}
int Speech::setPlaybackControls(const char *state)
{
    return publish_to_server("playback_controls",state);
}

int Speech::setPlaybackState(const char *state)
{
    return publish_to_server("playback_states",state);
}

int Speech::enableSmartVoice(bool enable)
{
    const char* msgid = "switchvoice";
    int ret = request_to_server(enable?"turnon":"turnoff",msgid);
    if(ret < 0) return FAILURE;

    mRequestCond.wait();

    return mRequestRet;
}

int Speech::playDefaultList()
{
    //getmusiclist
    const char* msgid = "getmusiclist";
    return request_to_server("getmusiclist", msgid);
}

char *Speech::get_json_string(char *string,int len)
{
    char *data = string;
    if(data[0] = 'A'){
        char json_len[10];
        plogd("here is speech TLV protocol!");
        int j = 0;
        for(int i = 3; i < len; i++){
            if(data[i] == C1 && data[i+1] == C2){
                data = data + i + 2;
                break;
            }
            json_len[j++] = data[i];
        }
        json_len[j] = '\0';
        if(atoi(json_len) > strlen(data)){
            ploge("string len dismatch!");
            return NULL;
        }
    }
    return data;
}

int Speech::request_to_server(const char *method, const char *msgid)
{
    json_object *json=json_object_new_object();
    json_object_object_add(json,"msgtype",json_object_new_string("request"));
    json_object_object_add(json,"msgid",json_object_new_string(msgid));
    json_object_object_add(json,"method",json_object_new_string(method));

    const char *str=json_object_to_json_string(json);

    //printf("%s\n",str);
    int ret = send_to_server(str);
    json_object_put(json);

    return ret;
}

int Speech::publish_to_server(const char *state_name, const char *state)
{
    json_object *json=json_object_new_object();
    json_object_object_add(json, "msgtype", json_object_new_string("publish"));
    json_object_object_add(json, "state_name", json_object_new_string(state_name));

    json_object *params=json_object_new_object();
    json_object_object_add(params,"state", json_object_new_string(state));

    json_object_object_add(json, "params", params);

    const char *str=json_object_to_json_string(json);

    //printf("%s\n",str);
    int ret = send_to_server(str);
    json_object_put(json);

    return ret;
}

int Speech::response_to_server(const char *msgid, int state)
{
    if(state >= 0)
        return response_to_server(msgid, "success");
    return response_to_server(msgid, "failed");

}
int Speech::response_to_server(const char *msgid, const char *state)
{
    json_object *json = json_object_new_object();
    json_object_object_add(json, "msgtype", json_object_new_string("response"));
    json_object_object_add(json, "msgid", json_object_new_string(msgid));

    json_object *params = json_object_new_object();
    json_object_object_add(params, "state", json_object_new_string(state));

    json_object_object_add(json, "params", params);

    const char *str = json_object_to_json_string(json);

    //printf("%s\n",str);
    int ret = send_to_server(str);
    json_object_put(json);

    return ret;
}

int Speech::send_to_server(const char* response)
{
    //plogd("send to server:  %s",response);
    int json_len = strlen(response);

    char json_len_string[25];
    bzero(json_len_string,sizeof(json_len_string));
    snprintf(json_len_string,sizeof(json_len_string),"%d",json_len);

    int separate_len = 2;
    int len1 = 1; // A protocal data type
    int len2 = strlen(json_len_string);

    int len_total = len1 + 2 * separate_len + len2 + json_len;
    char *out = (char*)malloc(len_total+1);
    CHECK_POINTER(out);
    bzero(out,len_total);
    int index = 0;
    out[index++] = 'A';
    out[index++] = C1;
    out[index++] = C2;
    // A\r\n
    strncpy(out + index,json_len_string,len2);
    index = index + len2;
    // A\r\nxxx
    out[index++] = C1;
    out[index++] = C2;
    // A\r\nxxx\r\n
    strncpy(out + index, response, json_len);
    out[len_total] = '\0';
    plogd("send: %s",out);

    int ret = mSocketClientWatcher.sendtoServer(out,len_total);
    free(out);
    return ret;
}

int Speech::do_play(const char *msgid)
{
    int state = play();
    return response_to_server(msgid,state);
}

int Speech::do_pause(const char *msgid)
{
    int state = pause();
    if(state < 0 && (-state  == PPlayer::STATUS_PLAYING || -state == PPlayer::STATUS_SEEKING))
        return response_to_server(msgid,-1);
    return response_to_server(msgid,0);
}

int Speech::do_stop(const char *msgid)
{
    int state = mPlayer->reset();
    return response_to_server(msgid,0);
}

int Speech::do_seturl(const char *url,const char *msgid)
{
    mPlayer->reset();
    int state = mPlayer->setDataSource(url);
    if(state == FAILURE)
        return response_to_server(msgid,state);
    state = mPlayer->prepare();
    return response_to_server(msgid,state);
}
int Speech::do_seturl(struct json_object *abslist,const char *msgid)
{
    //new playlist is comming
    mPlayer->resetPlayList();
    mPlayer->enablePlayList(true);

    int len = json_object_array_length(abslist);
    for(int i = 0; i < len; i++){

        struct json_object *item = json_object_array_get_idx(abslist, i);

        MediaElem *elem = new MediaElem();

        struct json_object *data;

        json_object_object_get_ex(item, "size", &data);
        if(data != NULL) elem->size = json_object_get_string(data);

        json_object_object_get_ex(item, "id", &data);
        if(data != NULL) elem->id = atoi(json_object_get_string(data));

        json_object_object_get_ex(item, "lrcSize", &data);
        if(data != NULL) elem->lrcSize = json_object_get_int(data);

        json_object_object_get_ex(item, "img", &data);
        if(data != NULL) elem->img = json_object_get_string(data);

        json_object_object_get_ex(item, "artist", &data);
        if(data != NULL) elem->artist = json_object_get_string(data);

        json_object_object_get_ex(item, "title", &data);
        if(data != NULL) elem->title = json_object_get_string(data);

        json_object_object_get_ex(item, "imgSize", &data);
        if(data != NULL) elem->imgSize = json_object_get_string(data);

        json_object_object_get_ex(item, "url", &data);
        if(data != NULL) elem->url = json_object_get_string(data);

        json_object_object_get_ex(item, "lrcUrl", &data);
        if(data != NULL) elem->lrcurl = json_object_get_string(data);

        elem->type = MediaElem::MEDIA_SPEECH;
        elem->index = i;

        mPlayer->addPlayList(elem);

        plogd("--> index = %d",elem->index);
        plogd("--> size = %s",      elem->size.data());
        plogd("--> id = %d",        elem->id);
        plogd("--> lrcSize = %d",   elem->lrcSize);
        plogd("--> img = %s",       elem->img.data());
        plogd("--> artist = %s",    elem->artist.data());
        plogd("--> title = %s",     elem->title.data());
        plogd("--> imgSize = %s",   elem->imgSize.data());
        plogd("--> url = %s",       elem->url.data());
        plogd("--> lrcurl = %s\n",  elem->lrcurl.data());

    }

    mPlayer->updatePlayList();
    MediaElem *elem = mPlayer->getCurPlayListElem();

    mPlayer->reset();
    int state = mPlayer->setDataSource(elem->url.data());
    if(state == FAILURE)
        return response_to_server(msgid,state);
    state = mPlayer->prepare();
    if(!strcmp(msgid,"getmusiclist")){
        play();
        return SUCCESS;
    }
    return response_to_server(msgid,state);
}

int Speech::do_set_volume(const char *operation,const char *msgid)
{
    int state = 0;
    if(!strcmp(operation,"up")){
        state = mPlayer->getAudioSetting()->upVolume();
    }else if(!strcmp(operation,"down")){
        state = mPlayer->getAudioSetting()->downVolume();
    }else if(!strcmp(operation,"mute")){
        state = mPlayer->getAudioSetting()->setMute();
    }else if(!strcmp(operation,"unmute")){
        state = mPlayer->getAudioSetting()->setUnmute();
    }else if(!strcmp(operation,"max")){
        state = mPlayer->getAudioSetting()->setMaxVolume();
    }else if(!strcmp(operation,"min")){
        state = mPlayer->getAudioSetting()->setMinVolume();
    }
    return response_to_server(msgid,state);
}

int Speech::do_switch_songs(const char *operation,const char *msgid)
{
    int state = 0;
    if(!strcmp(operation,"prev")){
        state = mPlayer->prev();
    }else if(!strcmp(operation,"next")){
        state = mPlayer->next();
    }
    return response_to_server(msgid, state);
}

int Speech::handle_request(const char *method, const char *msgid, struct json_object *params)
{
    plogd("%s %d %s",__func__,__LINE__,method);
    if(!strcmp(method, "seturl")){
        plogd("%s %d",__func__,__LINE__);
        struct json_object *abslist;
        if(json_object_object_get_ex(params, "abslist", &abslist))
            return do_seturl(abslist,msgid);
        return FAILURE;
    }else if(!strcmp(method, "play")){
        return do_play(msgid);
    }else if(!strcmp(method, "pause")){
        return do_pause(msgid);
    }else if(!strcmp(method, "stop")){
        return do_stop(msgid);
    }else if(!strcmp(method, "set_volume")){
        struct json_object *operation;
        if(json_object_object_get_ex(params, "operation", &operation)){
            return do_set_volume(json_object_get_string(operation), msgid);
        }
        return FAILURE;
    }else if(!strcmp(method, "switch_songs")){
        struct json_object *operation;
        if(json_object_object_get_ex(params, "operation", &operation)){
            return do_switch_songs(json_object_get_string(operation), msgid);
        }
        return FAILURE;
    }
}
int Speech::onSocketClientRead(int socketfd, char *buf, int len)
{
    if(len == 0){
        plogd("Server close the connection");
        return 0;
    }
    char *data = (char*)buf;

    char *json_string = get_json_string(data,len);
    if(json_string == NULL) return FAILURE;

    struct json_object *main_json = json_tokener_parse(json_string);
    if(is_error(main_json)) {
        plogd("speech read json err!!");
        return FAILURE;
    }

    struct json_object *msgtype = NULL;
    struct json_object *msgid = NULL;
    struct json_object *params = NULL;

    json_object_object_get_ex(main_json, "msgtype", &msgtype);
    json_object_object_get_ex(main_json, "msgid", &msgid);
    json_object_object_get_ex(main_json, "params", &params);

    if( msgtype == NULL || msgid == NULL){
        json_object_put(main_json);
        ploge("speech get json object err!!");
        return FAILURE;
    }

    const char *s_msgtype = json_object_get_string(msgtype);
    const char *s_msgid = json_object_get_string(msgid);
    if(!strcmp(s_msgtype, "request")){
        struct json_object *method;
        json_object_object_get_ex(main_json, "method", &method);
        const char *s_method = json_object_get_string(method);

        if (s_method != NULL && handle_request(s_method, s_msgid, params) == FAILURE){
            response_to_server(s_msgid,-1);
        }

    }else if(!strcmp(s_msgtype, "response")){
        struct json_object *state;
        json_object_object_get_ex(params, "state", &state);
        if(!strcmp(json_object_get_string(state), "success")){
            mRequestRet = 0;
        }else{
            mRequestRet = -1;
        }
        const char *s_msgid = json_object_get_string(msgid);
        if(!strcmp(s_msgid, "getmusiclist")){
            struct json_object *abslist;
            if(json_object_object_get_ex(params, "abslist", &abslist))
                do_seturl(abslist, s_msgid);
            mRequestCond.signal();
        }else if(!strcmp(s_msgid, "switchvoice")){
            mRequestCond.signal();
        }

    }

    json_object_put(main_json);

}

void Speech::onPrepared()
{

}
void Speech::onPlaybackComplete()
{

}
void Speech::onSeekComplete()
{

}

}