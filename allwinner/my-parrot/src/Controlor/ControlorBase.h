#ifndef __CONTROLOER_H__
#define __CONTROLOER_H__

#include <list>
#include <locker.h>

#include "Mode.h"
#include "PPlayer.h"
#include "NetworkConnection.h"

namespace Parrot{

class ControlorBase : public NetworkConnection::NetworkConnectionLinstener
{
public:
    ControlorBase(ref_PPlayer player):mPlayer(player){};
    ~ControlorBase(){};

public:

    class ControlorStatusListener
    {
    public:
        virtual void onControlVolumeChanged(int volume){};
        virtual void onControlPlayChanged(){};
        virtual void onControlModeChanged(){};

    };
    int init(ref_EventWatcher watcher, ref_HandlerBase base);
    int release();
    int start();
    int stop();

    int setPlayer(ref_PPlayer player);
    int doControlPower();
    int doControlVolume(int up_down);
    int doControlSwitchPlay();
    int doControlPlay();
    int doControlPlaylist(int pos, int seek);
    int doControlSeek(int seek);
    int doControlPause();
    int doControlPlayNext();
    int doControlPlayPrev();

    int doControlEnableMode(Mode::tMode mode);
    int doControlNetwork();

    NetworkConnection *getNetworkConnection();
    Mode *getCurMode();

    void setControlorStatusListener(ControlorStatusListener *l){
        mControlorStatusListener = l;
    }

private:
    void first_start();

    Mode *get_mode(Mode::tMode mode);
    void onNetworkSmartlinkdFailed();
    void onNetworkSmartlinkdDetected();
    void onNetworkConnected();
    void onNetworkConnectFailed();
    void onNetworkDisconnected();

private:
    NetworkConnection *mNetworkConnection;
    Mode * mCurMode;
    list<Mode*> mModeList;
    ref_PPlayer mPlayer;

    locker mModeLocker;
    locker mVolumeLocker;

    ControlorStatusListener *mControlorStatusListener;

};/*class ControlorBase*/

}/*namespace Parrot*/

#endif/*__CONTROLOER_H__*/