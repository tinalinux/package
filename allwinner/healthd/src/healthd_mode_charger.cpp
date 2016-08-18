#define TAG "healthd"
#include <tina_log.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <linux/input.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>
#include "minui.h"
#include <sys/socket.h>
#include <linux/netlink.h>

//#include <batteryservice/BatteryService.h>
#include <cutils/android_reboot.h>
#include <cutils/klog.h>
#include <cutils/misc.h>
#include <cutils/uevent.h>
#include <cutils/properties.h>

#include <pthread.h>
#include <sys/time.h>
#include "android_alarm.h"

/*
#ifdef CHARGER_ENABLE_SUSPEND
#include <suspend/autosuspend.h>
#endif
*/

//#include <minui/minui.h>
#include "android_alarm.h"
#include "healthd.h"

char *locale;

#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

#define ARRAY_SIZE(x)           (sizeof(x)/sizeof(x[0]))

#define MSEC_PER_SEC            (1000LL)
#define NSEC_PER_MSEC           (1000000LL)

#define BATTERY_UNKNOWN_TIME    (2 * MSEC_PER_SEC)
#define POWER_ON_KEY_TIME       (2 * MSEC_PER_SEC)
#define UNPLUGGED_SHUTDOWN_TIME (1 * MSEC_PER_SEC)

#define BATTERY_FULL_THRESH     95

#define LAST_KMSG_PATH          "/proc/last_kmsg"
#define LAST_KMSG_PSTORE_PATH   "/sys/fs/pstore/console-ramoops"
#define LAST_KMSG_MAX_SZ        (32 * 1024)

#define WAKEALARM_PATH        "/sys/class/rtc/rtc0/wakealarm"
#define ALARM_IN_BOOTING_PATH        "/sys/module/rtc_sunxi/parameters/alarm_in_booting"



struct key_state {
    bool pending;
    bool down;
    int64_t timestamp;
};

struct frame {
    int disp_time;
    int min_capacity;
    bool level_only;

    gr_surface surface;
};

struct animation {
    bool run;

    struct frame *frames;
    int cur_frame;
    int num_frames;

    int cur_cycle;
    int num_cycles;

    /* current capacity being animated */
    int capacity;
};

struct charger {
    bool have_battery_state;
    bool charger_connected;
    int64_t next_screen_transition;
    int64_t next_key_check;
    int64_t next_pwr_check;

    struct key_state keys[KEY_MAX + 1];

    struct animation *batt_anim;
    gr_surface surf_unknown;
};

static struct frame batt_anim_frames[] = {
    {
        .disp_time = 750,
        .min_capacity = 0,
        .level_only = false,
        .surface = NULL,
    },
    {
        .disp_time = 750,
        .min_capacity = 20,
        .level_only = false,
        .surface = NULL,
    },
    {
        .disp_time = 750,
        .min_capacity = 40,
        .level_only = false,
        .surface = NULL,
    },
    {
        .disp_time = 750,
        .min_capacity = 60,
        .level_only = false,
        .surface = NULL,
    },
    {
        .disp_time = 750,
        .min_capacity = 80,
        .level_only = true,
        .surface = NULL,
    },
    {
        .disp_time = 750,
        .min_capacity = BATTERY_FULL_THRESH,
        .level_only = false,
        .surface = NULL,
    },
};

static struct animation battery_animation = {
    .run = false,
    .frames = batt_anim_frames,
    .cur_frame = 0,
    .num_frames = ARRAY_SIZE(batt_anim_frames),
    .cur_cycle = 0,
    .num_cycles = 3,
    .capacity = 0,
};

static struct charger charger_state;
static struct healthd_config *healthd_config;
static struct BatteryProperties *batt_prop;
static int char_width;
static int char_height;
static bool minui_inited;
static bool suspend_flag;

static int alarm_fd = 0;
static pthread_t tid_alarm;

static int acquire_wake_lock_timeout(long long timeout);

static long get_wakealarm_sec(void)
{
    int fd = 0, ret = 0;
    unsigned long wakealarm_time = 0;
    char buf[32] = { 0 };

    fd = open(WAKEALARM_PATH, O_RDWR);
    if (fd < 0) {
        printf("open %s failed, return=%d\n", WAKEALARM_PATH, fd);
        return fd;
    }

    ret = read(fd, buf, sizeof(buf));
    if (ret > 0) {
        wakealarm_time = strtoul(buf, NULL, 0);
        printf("%s, %d, read wakealarm_time=%lu\n", __func__, __LINE__, wakealarm_time);

        // Clean initial wakealarm.
        // We will set wakealarm again use ANDROID_ALARM_SET ioctl.
        // Initial wakealarm's time unit is second, have conflict with
        // nanosecond alarmtimer in alarmtimer_suspend().
        snprintf(buf, sizeof(buf), "0");
        write(fd, buf, strlen(buf) + 1);
        printf("%s, %d, write wakealarm_time=0\n", __func__, __LINE__);

        close(fd);
        return wakealarm_time;
    }

    close(fd);
    return ret;
}

static long is_alarm_in_booting(void)
{
    int fd = 0, ret = 0;
    unsigned long alarm_in_booting = 0;
    char buf[32] = { 0 };

    fd = open(ALARM_IN_BOOTING_PATH, O_RDONLY);
    if (fd < 0) {
        printf("open %s failed, return=%d\n", ALARM_IN_BOOTING_PATH, fd);
        return fd;
    }

    ret = read(fd, buf, sizeof(buf));
    if (ret > 0) {
        alarm_in_booting = strtoul(buf, NULL, 0);
        printf("%s, %d, read alarm_in_booting=%lu\n", __func__, __LINE__, alarm_in_booting);
    }

    close(fd);
    return alarm_in_booting;
}

void *alarm_thread_handler(void *arg)
{
    struct timespec *ts = (struct timespec *)arg;
    int ret = 0;
    if (alarm_fd <= 0) {
        printf("%s, %d, alarm_fd=%d and exit\n", __func__, __LINE__, alarm_fd);
        return NULL;
    }

    while (true) {
        ret = ioctl(alarm_fd, ANDROID_ALARM_WAIT);
        if (ret & ANDROID_ALARM_RTC_SHUTDOWN_WAKEUP_MASK) {
            printf("%s, %d, alarm wakeup rebooting\n", __func__, __LINE__);
            acquire_wake_lock_timeout(UNPLUGGED_SHUTDOWN_TIME);
            android_reboot(ANDROID_RB_RESTART, 0, 0);
        } else {
            printf("%s, %d, alarm wait wakeup by %d\n", __func__, __LINE__, ret);
        }
    }

    return NULL;
}
/* current time in milliseconds */
static int64_t curr_time_ms(void)
{
    struct timespec tm;
    clock_gettime(CLOCK_MONOTONIC, &tm);
    return tm.tv_sec * MSEC_PER_SEC + (tm.tv_nsec / NSEC_PER_MSEC);
}
static const char *pwr_state_mem = "mem";
static const char *pwr_state_on = "on";

static int autosuspend_enable(void){
    const int SIZE = 256;
    char file[256];
   int ret;
    sprintf(file, "sys/power/%s", "state");
    int fd = open(file,O_RDWR,0);
    if(fd == -1){
		printf("Could not open '%s'\n",file);
		return -1;
    }
     ret = write(fd, pwr_state_mem, strlen(pwr_state_mem));
     if (ret < 0) {
		printf("set_power_state_mem err\n");
	}
    close(fd);
    return 0;
}


static int autosuspend_disable(void){
    const int SIZE = 256;
    char file[256];
   int ret;
    sprintf(file, "sys/power/%s", "state");
    int fd = open(file,O_RDWR,0);
    if(fd == -1){
		printf("Could not open '%s'\n",file);
		return -1;
    }
     ret = write(fd, pwr_state_on, strlen(pwr_state_on));
     if (ret < 0) {
		printf("set_power_state_on err\n");
	}
    close(fd);
    return 0;
}

static int request_suspend(bool enable)
{
	//printf("request_suspend enable=%d\n",enable);
    if (enable)
        return autosuspend_enable();
    else
        return autosuspend_disable();
}


static void clear_screen(void)
{
    gr_color(0, 0, 0, 255);
    gr_clear();
}

static int acquire_wake_lock_timeout(long long timeout)
{
    int fd,ret;
    char str[64];
    fd = open("/sys/power/wake_lock", O_WRONLY, 0);
    if (fd < 0)
        return -1;

    sprintf(str,"charge %lld",timeout);
    ret = write(fd, str, strlen(str));
    close(fd);
    return ret;
}

static int draw_text(const char *str, int x, int y)
{
    int str_len_px = gr_measure(str);

    if (x < 0)
        x = (gr_fb_width() - str_len_px) / 2;
    if (y < 0)
        y = (gr_fb_height() - char_height) / 2;
    gr_text(x, y, str, 0);

    return y + char_height;
}
static void android_green(void)
{
    gr_color(0xa4, 0xc6, 0x39, 255);
}
static int draw_surface_centered(struct charger* /*charger*/, gr_surface surface)
{
    int w;
    int h;
    int x;
    int y;
    w = gr_get_width(surface);
    h = gr_get_height(surface);
    x = (gr_fb_width() - w) / 2 ;
    y = (gr_fb_height() - h) / 2 ;

    printf("drawing surface %dx%d+%d+%d\n", w, h, x, y);
    gr_blit(surface, 0, 0, w, h, x, y);
    return y + h;
}
static void draw_unknown(struct charger *charger)
{
    int y;
    if (charger->surf_unknown) {
        draw_surface_centered(charger, charger->surf_unknown);
    } else {
        android_green();
        y = draw_text("Charging!", -1, -1);
        draw_text("?\?/100", -1, y + 25);
    }
}

static void draw_battery(struct charger *charger)
{
    struct animation *batt_anim = charger->batt_anim;
    struct frame *frame = &batt_anim->frames[batt_anim->cur_frame];

    if (batt_anim->num_frames != 0) {
        draw_surface_centered(charger, frame->surface);
        printf("drawing frame #%d min_cap=%d time=%d\n",
             batt_anim->cur_frame, frame->min_capacity,
             frame->disp_time);
    }
}

static void redraw_screen(struct charger *charger)
{
    struct animation *batt_anim = charger->batt_anim;

    clear_screen();

    /* try to display *something* */
    if (batt_anim->capacity < 0 || batt_anim->num_frames == 0)
	{
        draw_unknown(charger);
    }
	else
	{
        draw_battery(charger);
	}
    gr_flip();
}

static void kick_animation(struct animation *anim)
{
    anim->run = true;
}

static void reset_animation(struct animation *anim)
{
    anim->cur_cycle = 0;
    anim->cur_frame = 0;
    anim->run = false;
}

static void update_screen_state(struct charger *charger, int64_t now)
{
    struct animation *batt_anim = charger->batt_anim;
    int cur_frame;
    int disp_time;

    if (!batt_anim->run || now < charger->next_screen_transition)
        return;

    if (!minui_inited) {

        if (healthd_config && healthd_config->screen_on) {
            if (!healthd_config->screen_on(batt_prop)) {
                printf("[%" PRId64 "] leave screen off\n", now);
                batt_anim->run = false;
                charger->next_screen_transition = -1;
                if (charger->charger_connected)
                    request_suspend(true);
                return;
            }
        }

        gr_init();
        gr_font_size(&char_width, &char_height);

#ifndef CHARGER_DISABLE_INIT_BLANK
        gr_fb_blank(true);
#endif
        minui_inited = true;
    }

    /* animation is over, blank screen and leave */
    if (batt_anim->cur_cycle == batt_anim->num_cycles) {
        reset_animation(batt_anim);
        charger->next_screen_transition = -1;
        gr_fb_blank(true);
        printf("[%" PRId64 "] animation done\n", now);

        if (charger->charger_connected == 1){
			printf("[%" PRId64 "] request_suspend(true)---update_screen_state\n", now);
			request_suspend(true);
		}
        return;
    }
/*
    disp_time = batt_anim->frames[batt_anim->cur_frame].disp_time;


    if (batt_anim->cur_frame == 0) {
        int ret;

        printf("[%" PRId64 "] animation starting\n", now);
        if (batt_prop && batt_prop->batteryLevel >= 0 && batt_anim->num_frames != 0) {
            int i;

            for (i = 1; i < batt_anim->num_frames; i++) {
                if (batt_prop->batteryLevel < batt_anim->frames[i].min_capacity)
                    break;
            }
            batt_anim->cur_frame = i - 1;


            disp_time = batt_anim->frames[batt_anim->cur_frame].disp_time * 2;
        }
        if (batt_prop)
            batt_anim->capacity = batt_prop->batteryLevel;
    }


    if (batt_anim->cur_cycle == 0)
        //gr_fb_blank(false);


    redraw_screen(charger);


    if (batt_anim->num_frames == 0 || batt_anim->capacity < 0) {
        printf("[%" PRId64 "] animation missing or unknown battery status\n", now);
        charger->next_screen_transition = now + BATTERY_UNKNOWN_TIME;
        batt_anim->cur_cycle++;
        return;
    }


    charger->next_screen_transition = now + disp_time;


    if (charger->charger_connected) {
        batt_anim->cur_frame++;


        while (batt_anim->cur_frame < batt_anim->num_frames &&
               batt_anim->frames[batt_anim->cur_frame].level_only)
            batt_anim->cur_frame++;
        if (batt_anim->cur_frame >= batt_anim->num_frames) {
            batt_anim->cur_cycle++;
            batt_anim->cur_frame = 0;


        }
    } else {

        batt_anim->cur_frame = 0;
        batt_anim->cur_cycle++;
    }
*/
}

static int set_key_callback(int code, int value, void *data)
{
    struct charger *charger = (struct charger *)data;
    int64_t now = curr_time_ms();
    int down = !!value;
	//printf("set_key_callback\n");
    if (code > KEY_MAX)
        return -1;

    /* ignore events that don't modify our state */
    if (charger->keys[code].down == down)
        return 0;

    /* only record the down even timestamp, as the amount
     * of time the key spent not being pressed is not useful */
    if (down)
        charger->keys[code].timestamp = now;
    charger->keys[code].down = down;
    charger->keys[code].pending = true;
    if (down) {
        //printf("[%" PRId64 "] key[%d] down\n", now, code);
    } else {
        int64_t duration = now - charger->keys[code].timestamp;
        int64_t secs = duration / 1000;
        int64_t msecs = duration - secs * 1000;
        //printf("[%" PRId64 "] key[%d] up (was down for %" PRId64 ".%" PRId64 "sec)\n",
         //    now, code, secs, msecs);
    }

    return 0;
}

static void update_input_state(struct charger *charger,
                               struct input_event *ev)
{
    if (ev->type != EV_KEY)
        return;
	//printf("update_input_state\n");
    set_key_callback(ev->code, ev->value, charger);
}

static void set_next_key_check(struct charger *charger,
                               struct key_state *key,
                               int64_t timeout)
{
    int64_t then = key->timestamp + timeout;
	//printf("set_next_key_check\n");
    if (charger->next_key_check == -1 || then < charger->next_key_check)
        charger->next_key_check = then;
}

static void process_key(struct charger *charger, int code, int64_t now)
{
    struct key_state *key = &charger->keys[code];
    int64_t next_key_check;
	//printf("process_key\n");
    if (code == KEY_POWER) {
        if (key->down) {
            int64_t reboot_timeout = key->timestamp + POWER_ON_KEY_TIME;
            if (now >= reboot_timeout) {
                /* We do not currently support booting from charger mode on
                   all devices. Check the property and continue booting or reboot
                   accordingly. */
                if (property_get_bool("ro.enable_boot_charger_mode", false)) {
                    printf("[%" PRId64 "] booting from charger mode\n", now);
                    property_set("sys.boot_from_charger_mode", "1");
                } else {
                    printf("[%" PRId64 "] rebooting\n", now);
                    android_reboot(ANDROID_RB_RESTART, 0, 0);
                }
            } else {
                /* if the key is pressed but timeout hasn't expired,
                 * make sure we wake up at the right-ish time to check
                 */
				printf("process_key  request_suspend(false)\n");
                request_suspend(false);
				suspend_flag=false;
                set_next_key_check(charger, key, POWER_ON_KEY_TIME);
            }
        } else {
            /* if the power key got released, force screen state cycle */
			//printf("process_key    power key got released\n");
            if (key->pending) {
                //request_suspend(false);
                //kick_animation(charger->batt_anim);
            }
        }
    }

    key->pending = false;
}

static void handle_input_state(struct charger *charger, int64_t now)
{
    process_key(charger, KEY_POWER, now);
	//printf("handle_input_state\n");
    if (charger->next_key_check != -1 && now > charger->next_key_check)
        charger->next_key_check = -1;
}

static void handle_power_supply_state(struct charger *charger, int64_t now)
{
	//printf("handle_power_supply_state charger->charger_connected=%d\n",charger->charger_connected);

    if (!charger->have_battery_state)
        return;
    if (!charger->charger_connected) {
		printf("handle_power_supply_state  request_suspend(false)\n");
        request_suspend(false);
        if (charger->next_pwr_check == -1) {
            charger->next_pwr_check = now + UNPLUGGED_SHUTDOWN_TIME;
            printf("[%" PRId64 "] device unplugged: shutting down in %" PRId64 " (@ %" PRId64 ")\n",
                 now, (int64_t)UNPLUGGED_SHUTDOWN_TIME, charger->next_pwr_check);
        } else if (now >= charger->next_pwr_check) {
            printf("[%" PRId64 "] shutting down\n", now);
            android_reboot(ANDROID_RB_POWEROFF, 0, 0);
        } else {
            /* otherwise we already have a shutdown timer scheduled */
        }
    } else {
        /* online supply present, reset shutdown timer if set */
        if (charger->next_pwr_check != -1) {
            //printf("[%" PRId64 "] device plugged in: shutdown cancelled\n", now);
            //kick_animation(charger->batt_anim);
        }

		if(suspend_flag == false){
			suspend_flag=true;
			printf("request_suspend(true) charger->next_pwr_check=%d\n",charger->next_pwr_check);
			request_suspend(true);
		}

        charger->next_pwr_check = -1;
    }
}

void healthd_mode_charger_heartbeat()
{
    struct charger *charger = &charger_state;
    int64_t now = curr_time_ms();
    int ret;
	//printf("healthd_mode_charger_heartbeat\n");
    handle_input_state(charger, now);
    handle_power_supply_state(charger, now);

    /* do screen update last in case any of the above want to start
     * screen transitions (animations, etc)
     */
    update_screen_state(charger, now);
}

void healthd_mode_charger_battery_update(
    struct BatteryProperties *props)
{
    struct charger *charger = &charger_state;

    charger->charger_connected =
        props->chargerAcOnline || props->chargerUsbOnline;
	//printf("healthd_mode_charger_battery_update charger_connected=%d\n",charger->charger_connected);
	//printf("chargerAcOnline=%d\n",props->chargerAcOnline);
	//printf("chargerUsbOnline=%d\n",props->chargerUsbOnline);
    if (!charger->have_battery_state) {
        charger->have_battery_state = true;
        charger->next_screen_transition = curr_time_ms() - 1;
        reset_animation(charger->batt_anim);
        kick_animation(charger->batt_anim);
    }
    batt_prop = props;
}

int healthd_mode_charger_preparetowait(void)
{
    struct charger *charger = &charger_state;
    int64_t now = curr_time_ms();
    int64_t next_event = INT64_MAX;
    int64_t timeout;
    struct input_event ev;
    int ret;
/*
    printf("[%" PRId64 "] next key: %" PRId64 " next pwr: %" PRId64 "\n", now,
         charger->next_key_check,charger->next_pwr_check);
*/
    if (charger->next_screen_transition != -1)
        next_event = charger->next_screen_transition;
    if (charger->next_key_check != -1 && charger->next_key_check < next_event)
        next_event = charger->next_key_check;
    if (charger->next_pwr_check != -1 && charger->next_pwr_check < next_event)
        next_event = charger->next_pwr_check;

    if (next_event != -1 && next_event != INT64_MAX)
        timeout = max(0, next_event - now);
    else
        timeout = -1;

   return (int)timeout;
}

static int input_callback(int fd, unsigned int epevents, void *data)
{
    struct charger *charger = (struct charger *)data;
    struct input_event ev;
    int ret;
	//printf("input_callback\n");
    ret = ev_get_input(fd, epevents, &ev);
    if (ret)
        return -1;
    update_input_state(charger, &ev);
    return 0;
}

static void charger_event_handler(uint32_tt /*epevents*/)
{
    int ret;
	//printf("charger_event_handler\n");
    ret = ev_wait(-1);
    if (!ret)
        ev_dispatch();
}

static void init_shutdown_alarm(void)
{
    long alarm_secs, alarm_in_booting = 0;
    struct timeval now_tv = { 0, 0 };
    struct timespec ts;
    alarm_fd = open("/dev/alarm", O_RDWR);
    if (alarm_fd < 0) {
        printf("open /dev/alarm failed, ret=%d\n", alarm_fd);
        return;
    }

    alarm_secs = get_wakealarm_sec();
    // have alarm irq in booting ?
    alarm_in_booting = is_alarm_in_booting();
    gettimeofday(&now_tv, NULL);

    printf("gettimeofday, sec=%ld, microsec=%ld, alarm_secs=%ld, alarm_in_booting=%ld\n",
         (long)now_tv.tv_sec, (long)now_tv.tv_usec, alarm_secs, alarm_in_booting);

    // alarm interval time == 0 and have no alarm irq in booting
    if (alarm_secs <= 0 && (alarm_in_booting != 1))
        return;

    if (alarm_secs)
        ts.tv_sec = alarm_secs;
    else
        ts.tv_sec = (long)now_tv.tv_sec + 1;

    ts.tv_nsec = 0;

    ioctl(alarm_fd, ANDROID_ALARM_SET(ANDROID_ALARM_RTC_SHUTDOWN_WAKEUP), &ts);

    pthread_create(&tid_alarm, NULL, alarm_thread_handler, NULL);

    return;
}

void healthd_mode_charger_init(struct healthd_config* config)
{
    int ret;
    struct charger *charger = &charger_state;

        int i;
    int epollfd;
	printf("--------------- STARTING CHARGER MODE ---------------\n");
    printf("\n");
    printf("*************** LAST KMSG ***************\n");
    printf("\n");
	init_shutdown_alarm();

    ret = ev_init(input_callback, charger);
    if (!ret) {
        epollfd = ev_get_epollfd();
        healthd_register_event(epollfd, charger_event_handler);
    }

    ret = res_create_display_surface("charger/battery_fail", &charger->surf_unknown);
    if (ret < 0) {
        printf("Cannot load battery_fail image\n");
        charger->surf_unknown = NULL;
    }

    charger->batt_anim = &battery_animation;

    gr_surface* scale_frames;
    int scale_count;
    ret = res_create_multi_display_surface("charger/battery_scale", &scale_count, &scale_frames);
    if (ret < 0) {
        printf("Cannot load battery_scale image\n");
        charger->batt_anim->num_frames = 0;
        charger->batt_anim->num_cycles = 1;
    } else if (scale_count != charger->batt_anim->num_frames) {
        printf("battery_scale image has unexpected frame count (%d, expected %d)\n",
             scale_count, charger->batt_anim->num_frames);
        charger->batt_anim->num_frames = 0;
        charger->batt_anim->num_cycles = 1;
    } else {
        for (i = 0; i < charger->batt_anim->num_frames; i++) {
            charger->batt_anim->frames[i].surface = scale_frames[i];
        }
    }

    ev_sync_key_state(set_key_callback, charger);

    charger->next_screen_transition = -1;
    charger->next_key_check = -1;
    charger->next_pwr_check = -1;
    healthd_config = config;
}
