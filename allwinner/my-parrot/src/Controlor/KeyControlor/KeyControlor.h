#ifndef __KEY_CONTROLOR_H__
#define __KEY_CONTROLOR_H__

#include "KeyEventWatcher.h"
#include "ControlorBase.h"

namespace Parrot{

class KeyControlor : public KeyEventWatcher::KeyEventListener
{
public:
    KeyControlor(){};
    ~KeyControlor(){};
#if 1
    const static int KEY_CTL_POWER = 116;      /*"axp22-supplyer"*/
    const static int KEY_CTL_UP_NEXT = 114;//114;    /*"sunxi-keyboard"*/
    const static int KEY_CTL_DOWN_PREV = 139;//115;  /*"sunxi-keyboard"*/
    const static int KEY_CTL_PLAY = 102;//102;       /*"sunxi-keyboard"*/
    const static int KEY_CTL_BT_LINEIN = 28;//139;  /*"sunxi-keyboard"*/
    const static int KEY_CTL_WIFI = 115;//28;        /*"sunxi-keyboard"*/
#else
    const static int KEY_CTL_POWER = 116;      /*"axp22-supplyer"*/
    const static int KEY_CTL_UP_NEXT = 114;    /*"sunxi-keyboard"*/
    const static int KEY_CTL_DOWN_PREV = 115;  /*"sunxi-keyboard"*/
    const static int KEY_CTL_PLAY = 102;       /*"sunxi-keyboard"*/
    const static int KEY_CTL_BT_LINEIN = 139;  /*"sunxi-keyboard"*/
    const static int KEY_CTL_WIFI = 28;        /*"sunxi-keyboard"*/
#endif

    int init(ref_EventWatcher watcher, ref_HandlerBase base, ControlorBase *controlorbase);
    int release();

protected:
    void onKeyDown(int fdindex, int keycode);
    void onKeyUp(int fdindex, int keycode);
    void onkeyLongClick(int fdindex, int keycode, int longclicktime);

private:
    list<KeyEventWatcher*> mKeylist;
    ControlorBase *mControlor;

};/*class KeyControlor*/
}/*namespace Parrot*/
#endif/*__KEY_CONTROLOR_H__*/