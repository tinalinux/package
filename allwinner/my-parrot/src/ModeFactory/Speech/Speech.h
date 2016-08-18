#ifndef __SPEECH_H__
#define __SPEECH_H__
#include <locker.h>
#include <json-c/json.h>

#include "Mode.h"
#include "SocketClientWatcher.h"

namespace Parrot{

class Speech : public Mode, public SocketClientWatcher::SocketListener, public PPlayer::PPlayerListener
{
public:
    static const char *IP; //??
    static const int PORT;
    static const int C1 = 13;
    static const int C2 = 10;
    Speech(ref_EventWatcher watcher, ref_HandlerBase handlerbase);
    ~Speech();

    int onCreate();
    int onStart();
    int onStop();
    int onDestory();

    int play();
    int pause();
    int next();
    int prev();
    int switchplay();

    int setNetworkStatus(const char *state);
    int setPlaybackControls(const char *state);
    int setPlaybackState(const char *state);
    int enableSmartVoice(bool enable);
    int playDefaultList();

protected:
    int onSocketClientRead(int socketfd, char *buf, int len);
    void onPrepared();
    void onPlaybackComplete();
    void onSeekComplete();

private:
    char *get_json_string(char *string, int len);
    int handle_request(const char *medthod, const char *msgid, struct json_object *params);
    int do_seturl(const char *url, const char *msgid);
    int do_seturl(struct json_object *abslist,const char *msgid);
    int do_set_volume(const char *operation,const char *msgid);
    int do_switch_songs(const char *operation, const char *msgid);
    int do_play(const char *msgid);
    int do_pause(const char *msgid);
    int do_stop(const char *msgid);

    int response_to_server(const char *msgpid, int state);
    int response_to_server(const char *msgid, const char *state);
    int send_to_server(const char* respone);

    int publish_to_server(const char *state_name, const char *state);
    int request_to_server(const char *medthod, const char *msgid);

    int handle_publish(){};
private:
   SocketClientWatcher mSocketClientWatcher;
   bool mShouldStart;

   int  mRequestRet;
   cond mRequestCond;
};

}
#endif/*__SPEECH_H__*/