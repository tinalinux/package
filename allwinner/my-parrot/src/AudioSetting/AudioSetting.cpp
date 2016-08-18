#define TAG "AudioSetting"
#include <tina_log.h>

#include <alsa/asoundlib.h>
#include <linux/input.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#include "parrot_common.h"
#include "AudioSetting.h"

#define PRESS_DATA_NAME    "audiocodec linein Jack"

#define LEVEL_BASIC     (1<<0)
#define LEVEL_INACTIVE  (1<<1)
#define LEVEL_ID        (1<<2)

static char card[64] = "default";
static int quiet = 0;
static int debugflag = 0;
static int no_check = 0;
static int smixer_level = 0;
static int ignore_error = 0;
static void print_spaces(unsigned int spaces)
{
    while (spaces-- > 0)
        putc(' ', stdout);
}

static void print_dB(long dB)
{
    if (dB < 0) {
        printf("-%li.%02lidB", -dB / 100, -dB % 100);
    } else {
        printf("%li.%02lidB", dB / 100, dB % 100);
    }
}
static const char *control_type(snd_ctl_elem_info_t *info)
{
    return snd_ctl_elem_type_name(snd_ctl_elem_info_get_type(info));
}
static void show_control_id(snd_ctl_elem_id_t *id)
{
    char *str;

    str = snd_ctl_ascii_elem_id_get(id);
    if (str)
        printf("%s", str);
    free(str);
}
static const char *control_access(snd_ctl_elem_info_t *info)
{
    static char result[10];
    char *res = result;

    *res++ = snd_ctl_elem_info_is_readable(info) ? 'r' : '-';
    *res++ = snd_ctl_elem_info_is_writable(info) ? 'w' : '-';
    *res++ = snd_ctl_elem_info_is_inactive(info) ? 'i' : '-';
    *res++ = snd_ctl_elem_info_is_volatile(info) ? 'v' : '-';
    *res++ = snd_ctl_elem_info_is_locked(info) ? 'l' : '-';
    *res++ = snd_ctl_elem_info_is_tlv_readable(info) ? 'R' : '-';
    *res++ = snd_ctl_elem_info_is_tlv_writable(info) ? 'W' : '-';
    *res++ = snd_ctl_elem_info_is_tlv_commandable(info) ? 'C' : '-';
    *res++ = '\0';
    return result;
}

static void decode_tlv(unsigned int spaces, unsigned int *tlv, unsigned int tlv_size)
{
    unsigned int type = tlv[0];
    unsigned int size;
    unsigned int idx = 0;
    const char *chmap_type = NULL;

    if (tlv_size < 2 * sizeof(unsigned int)) {
        printf("TLV size error!\n");
        return;
    }
    print_spaces(spaces);
    printf("| ");
    type = tlv[idx++];
    size = tlv[idx++];
    tlv_size -= 2 * sizeof(unsigned int);
    if (size > tlv_size) {
        printf("TLV size error (%u, %u, %u)!\n", type, size, tlv_size);
        return;
    }
    switch (type) {
    case SND_CTL_TLVT_CONTAINER:
        printf("container\n");
        size += sizeof(unsigned int) -1;
        size /= sizeof(unsigned int);
        while (idx < size) {
            if (tlv[idx+1] > (size - idx) * sizeof(unsigned int)) {
                printf("TLV size error in compound!\n");
                return;
            }
            decode_tlv(spaces + 2, tlv + idx, tlv[idx+1] + 8);
            idx += 2 + (tlv[idx+1] + sizeof(unsigned int) - 1) / sizeof(unsigned int);
        }
        break;
    case SND_CTL_TLVT_DB_SCALE:
        printf("dBscale-");
        if (size != 2 * sizeof(unsigned int)) {
            while (size > 0) {
                printf("0x%08x,", tlv[idx++]);
                size -= sizeof(unsigned int);
            }
        } else {
            printf("min=");
            print_dB((int)tlv[2]);
            printf(",step=");
            print_dB(tlv[3] & 0xffff);
            printf(",mute=%i", (tlv[3] >> 16) & 1);
        }
        break;
    default:
        printf("unk-%u-", type);
        while (size > 0) {
            printf("0x%08x,", tlv[idx++]);
            size -= sizeof(unsigned int);
        }
        break;
    }
    putc('\n', stdout);
}
static int show_control(const char *space, snd_hctl_elem_t *elem,
            int level)
{
    int err;
    unsigned int item, idx, count, *tlv;
    snd_ctl_elem_type_t type;
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_info_t *info;
    snd_ctl_elem_value_t *control;
    snd_aes_iec958_t iec958;
    snd_ctl_elem_id_alloca(&id);
    snd_ctl_elem_info_alloca(&info);
    snd_ctl_elem_value_alloca(&control);
    if ((err = snd_hctl_elem_info(elem, info)) < 0) {
        printf("Control %s snd_hctl_elem_info error: %s\n", card, snd_strerror(err));
        return err;
    }
    if (level & LEVEL_ID) {
        snd_hctl_elem_get_id(elem, id);
        show_control_id(id);
        printf("\n");
    }
    count = snd_ctl_elem_info_get_count(info);
    type = snd_ctl_elem_info_get_type(info);
    printf("%s; type=%s,access=%s,values=%u", space, control_type(info), control_access(info), count);
    switch (type) {
    case SND_CTL_ELEM_TYPE_INTEGER:
        printf(",min=%li,max=%li,step=%li\n",
               snd_ctl_elem_info_get_min(info),
               snd_ctl_elem_info_get_max(info),
               snd_ctl_elem_info_get_step(info));
        break;
    case SND_CTL_ELEM_TYPE_INTEGER64:
        printf(",min=%Li,max=%Li,step=%Li\n",
               snd_ctl_elem_info_get_min64(info),
               snd_ctl_elem_info_get_max64(info),
               snd_ctl_elem_info_get_step64(info));
        break;
    case SND_CTL_ELEM_TYPE_ENUMERATED:
    {
        unsigned int items = snd_ctl_elem_info_get_items(info);
        printf(",items=%u\n", items);
        for (item = 0; item < items; item++) {
            snd_ctl_elem_info_set_item(info, item);
            if ((err = snd_hctl_elem_info(elem, info)) < 0) {
                printf("Control %s element info error: %s\n", card, snd_strerror(err));
                return err;
            }
            printf("%s; Item #%u '%s'\n", space, item, snd_ctl_elem_info_get_item_name(info));
        }
        break;
    }
    default:
        printf("\n");
        break;
    }
    if (level & LEVEL_BASIC) {
        if (!snd_ctl_elem_info_is_readable(info))
            goto __skip_read;
        if ((err = snd_hctl_elem_read(elem, control)) < 0) {
            printf("Control %s element read error: %s\n", card, snd_strerror(err));
            return err;
        }
        printf("%s: values=", space);
        for (idx = 0; idx < count; idx++) {
            if (idx > 0)
                printf(",");
            switch (type) {
            case SND_CTL_ELEM_TYPE_BOOLEAN:
                printf("%s", snd_ctl_elem_value_get_boolean(control, idx) ? "on" : "off");
                break;
            case SND_CTL_ELEM_TYPE_INTEGER:
                printf("%li", snd_ctl_elem_value_get_integer(control, idx));
                break;
            case SND_CTL_ELEM_TYPE_INTEGER64:
                printf("%Li", snd_ctl_elem_value_get_integer64(control, idx));
                break;
            case SND_CTL_ELEM_TYPE_ENUMERATED:
                printf("%u", snd_ctl_elem_value_get_enumerated(control, idx));
                break;
            case SND_CTL_ELEM_TYPE_BYTES:
                printf("0x%02x", snd_ctl_elem_value_get_byte(control, idx));
                break;
            case SND_CTL_ELEM_TYPE_IEC958:
                snd_ctl_elem_value_get_iec958(control, &iec958);
                printf("[AES0=0x%02x AES1=0x%02x AES2=0x%02x AES3=0x%02x]",
                       iec958.status[0], iec958.status[1],
                       iec958.status[2], iec958.status[3]);
                break;
            default:
                printf("?");
                break;
            }
        }
        printf("\n");
          __skip_read:
        if (!snd_ctl_elem_info_is_tlv_readable(info))
            goto __skip_tlv;
        tlv = (unsigned int*)malloc(4096);
        if ((err = snd_hctl_elem_tlv_read(elem, tlv, 4096)) < 0) {
            printf("Control %s element TLV read error: %s\n", card, snd_strerror(err));
            free(tlv);
            return err;
        }
        decode_tlv(strlen(space), tlv, 4096);
        free(tlv);
    }
      __skip_tlv:
    return 0;
}

/*-------------------------------------------------------------*/

namespace Parrot {
AudioSetting::AudioSetting(){
    mAudioLineInStatusListener = NULL;

    bzero(Card,sizeof(Card));
    bzero(VolumeName,sizeof(VolumeName));
    bzero(LineInJack,sizeof(LineInJack));

    mLineInStatus = STATUS_OUT;
}

AudioSetting::~AudioSetting(){

    //destoryAudioLineInDetect();

}
#if 0
int AudioSetting::mixer_getcurvolume(const char* name){
    const char* shell = "volume=`amixer cget name='%s' | grep ': values='`;volume=${volume#*=};echo $volume";
    return mixer_get(shell,name);
}

int AudioSetting::mixer_getmaxvolume(const char* name){
    const char* shell = "volume=`amixer cget name='%s' | grep max `; volume=${volume#*max=}; volume=${volume%%,*};echo $volume";
    return mixer_get(shell,name);
}
#endif
int AudioSetting::fetchAudioLineInStatus()
{
    int res;

    int fd = open(LineInStatusPath, O_RDONLY);
    if(fd < 0){
        ploge("Linein_Init: open %s error!", LineInStatusPath);
        return FAILURE;
    }

    char tmp;
    lseek(fd, 0, SEEK_SET);
    read(fd, &tmp, sizeof(tmp));
    if(tmp == 'Y'){
        mLineInStatus = STATUS_IN;
    }
    else if(tmp == 'N'){
        mLineInStatus = STATUS_OUT;
    }
    close(fd);
    plogd("mLineInStatus = %d", mLineInStatus);
    return FAILURE;
}

int AudioSetting::setListener(AudioLineInStatusListener* l)
{
    mAudioLineInStatusListener = l;
}

int AudioSetting::initAudioLineInDetect(AudioLineInStatusListener *l){

    if(mWatcher.AsPointer() == NULL){
        ploge("initAudioLineInDetect error, please configure Watcher first!");
        return FAILURE;
    }
    fetchAudioLineInStatus();
    mAudioLineInStatusListener = l;
    if(mInputEventWatcher == NULL){
        mInputEventWatcher = new InputEventWatcher(mWatcher,LineInJack,this);
        CHECK_POINTER(mInputEventWatcher);
    }
    return SUCCESS;
}

void AudioSetting::destoryAudioLineInDetect()
{
    if(mInputEventWatcher != NULL){
        ploge("%s %d",__func__,__LINE__);
        delete mInputEventWatcher;
        ploge("%s %d",__func__,__LINE__);
        mAudioLineInStatusListener = NULL;
    }
}

int AudioSetting::switchtoLineInChannel(bool is_line_in)
{
    char set[20];

    if(is_line_in){
        snprintf(set,sizeof(set),"%d",1);
    }
    else{
        snprintf(set,sizeof(set),"%d",0);
    }
    return amixer_opreation(LineInSwitchName,set,0,NULL);
}

void AudioSetting::onEvent(EventWatcher::tEventType type,int index, void *data, int len, void* args)
{
    struct input_event *buff = (struct input_event *)data;
    ploge("code: %d value %d", buff->code, buff->value);
    if(buff->code != 0 && buff->value == STATUS_IN) {
        mLineInStatus = STATUS_IN;
        //switchtoLineInChannel(true);
    } else if (buff->code != 0 && buff->value == STATUS_OUT) {
        mLineInStatus = STATUS_OUT;
        //switchtoLineInChannel(false);
    }
    if(buff->code != 0 && mAudioLineInStatusListener != NULL){
        mAudioLineInStatusListener->onAudioInStatusChannged(mLineInStatus, buff->time);
    }
    plogd("mLineInStatus = %d", mLineInStatus);
}

int AudioSetting::setVolume(int value)
{
    char set[20];
    snprintf(set, sizeof(set), "%d", value);
    amixer_set(VolumeName, set);
}

int AudioSetting::getVolume()
{
    return amixer_get(VolumeName);
}

int AudioSetting::setVirtualVolume(int value)
{

}

int AudioSetting::getVirtualVolume()
{


}
int AudioSetting::downVolume()
{
    return setVolume(getVolume() - 1);
}

int AudioSetting::upVolume()
{
    return setVolume(getVolume() + 1);
}

int AudioSetting::configureCardName(const char *card)
{
    if(strlen(card) > sizeof(Card)){
        ploge("configureCardName error, Card need more space!");
        return FAILURE;
    }
    strncpy(Card,card,sizeof(Card));
    return SUCCESS;
}

int AudioSetting::configureVolumeName(const char *name)
{
    if(strlen(name) > sizeof(VolumeName)){
        ploge("configureVolumeName error, VolumeName need more space!");
        return FAILURE;
    }
    strncpy(VolumeName,name,sizeof(VolumeName));
    return SUCCESS;
}

int AudioSetting::configureLineInJackName(const char *name)
{
    if(strlen(name) > sizeof(LineInJack)){
        ploge("configureLineInJackName error, LineInJack need more space!");
        return FAILURE;
    }
    strncpy(LineInJack,name,sizeof(LineInJack));
    return SUCCESS;
}

int AudioSetting::configureLineInStatusPath(const char *path)
{
    if(strlen(path) > sizeof(LineInStatusPath)){
        ploge("configureLineInStatusPath error, LineInJack need more space!");
        return FAILURE;
    }
    strncpy(LineInStatusPath,path,sizeof(LineInStatusPath));
    return SUCCESS;
}

int AudioSetting::configureLinInStatusSwitch(const char *name)
{
    if(strlen(name) > sizeof(LineInSwitchName)){
        ploge("configureLineInSwitchName error, LineInJack need more space!");
        return FAILURE;
    }
    strncpy(LineInSwitchName,name,sizeof(LineInSwitchName));
    return SUCCESS;
}

int AudioSetting::configureWatcher(ref_EventWatcher watcher)
{
    mWatcher = watcher;
}

int AudioSetting::amixer_set(const char *name, const char *value)
{
    ploge("%s %d",__func__,__LINE__);
    return amixer_opreation(name,value,0,NULL);
}
int AudioSetting::amixer_get(const char *name)
{
    int get;
    int ret = amixer_opreation(name,NULL,1,&get);
    if(ret < 0){
        ploge("amixer_opreation get %s error",name);
        return ret;
    }
    return get;
}
//memory leak here
int AudioSetting::amixer_opreation(const char *name, const char *value, int roflag, int *get)
{
    ploge("%s %d",__func__,__LINE__);
    int err;
    static snd_ctl_t *handle = NULL;
    snd_ctl_elem_info_t *info;
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_value_t *control;

    snd_ctl_elem_info_alloca(&info);
    snd_ctl_elem_id_alloca(&id);
    snd_ctl_elem_value_alloca(&control);

    if (snd_ctl_ascii_elem_id_parse(id, name)) {
        fprintf(stderr, "Wrong control identifier: %s\n", name);
        return -EINVAL;
    }

    if (handle == NULL &&
        (err = snd_ctl_open(&handle, Card, 0)) < 0) {
        ploge("Control %s open error: %s\n", Card, snd_strerror(err));
        return err;
    }

    snd_ctl_elem_info_set_id(info, id);

    if ((err = snd_ctl_elem_info(handle, info)) < 0) {
        ploge("Cannot find the given element from control %s\n", Card);
        snd_ctl_close(handle);
        handle = NULL;
        return err;
    }

    snd_ctl_elem_info_get_id(info, id);     /* FIXME: Remove it when hctl find works ok !!! */

    snd_ctl_elem_value_set_id(control, id);
    if ((err = snd_ctl_elem_read(handle, control)) < 0) {
        ploge("Cannot read the given element from control %s\n", Card);
        snd_ctl_close(handle);
        handle = NULL;
        return err;
    }
    if (!roflag) {
        //set
        err = snd_ctl_ascii_value_parse(handle, control, info, value);
        if (err < 0) {
            ploge("Control %s parse error: %s\n", Card, snd_strerror(err));
            snd_ctl_close(handle);
            handle = NULL;
            return err;
        }

        if ((err = snd_ctl_elem_write(handle, control)) < 0) {
            ploge("Control %s element %s %s write error: %s\n", Card, name, value, snd_strerror(err));
            snd_ctl_close(handle);
            handle = NULL;
            return err;
        }
    }else{
        //get
        unsigned int count;
        snd_ctl_elem_type_t type;

        count = snd_ctl_elem_info_get_count(info);
        type  = snd_ctl_elem_info_get_type(info);

        if(type != SND_CTL_ELEM_TYPE_INTEGER || 1 != count) {
            ploge("Cannot find the given element from control %s\n", card);
            snd_ctl_close(handle);
            handle = NULL;
            return FAILURE;
        }

        snd_ctl_elem_value_set_id(control, id);

        if (!snd_ctl_elem_read(handle, control)) {
            *get = snd_ctl_elem_value_get_integer(control, 0);
        }
    }

    snd_ctl_close(handle);
    handle = NULL;
    quiet = 1;
    if (!quiet) {
        snd_hctl_t *hctl;
        snd_hctl_elem_t *elem;
        if ((err = snd_hctl_open(&hctl, Card, 0)) < 0) {
            printf("Control %s open error: %s\n", Card, snd_strerror(err));
            return err;
        }
        if ((err = snd_hctl_load(hctl)) < 0) {
            printf("Control %s load error: %s\n", Card, snd_strerror(err));
            return err;
        }
        elem = snd_hctl_find_elem(hctl, id);
        if (elem)
            show_control("  ", elem, LEVEL_BASIC | LEVEL_ID);
        else
            printf("Could not find the specified element\n");
        snd_hctl_close(hctl);
    }

    return SUCCESS;
}

}
