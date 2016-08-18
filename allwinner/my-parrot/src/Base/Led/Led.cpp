#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "parrot_common.h"
#include "Led.h"

namespace Parrot{

static const char *led_class = "/sys/devices/platform/sunxi-led/leds/led%d/%s";
int Led::set(tLedStatus status)
{
    int ret = SUCCESS;
    switch(status){
    case ON:
        ret = on();
        break;
    case OFF:
        ret = off();
        break;
    case FLASH:
        ret = flash();
        break;
    case DOUBLEFLASH:
        ret = doubleflash();
        break;
    }
    return ret;
}

int Led::on()
{
    CHECK( set_led_class("trigger", "none"));
    CHECK( set_led_class("brightness","100"));
    return SUCCESS;
}

int Led::off()
{
    CHECK( set_led_class("trigger", "none"));
    CHECK( set_led_class("brightness","0"));
    return SUCCESS;
}

int Led::flash()
{
    return flash(500, 500);
}

int Led::doubleflash()
{
    return doubleflash(200, 200, 600);
}

int Led::flash(int delay_on/*ms*/, int delay_off/*ms*/)
{
    CHECK(set_led_class("trigger","timer"));
    usleep(1000);
    CHECK(set_led_class("delay_on",delay_on));
    CHECK(set_led_class("delay_off",delay_off));
    return SUCCESS;
}

int Led::doubleflash(int delay_on/*ms*/, int first_delay_off/*ms*/, int second_delay_off/*ms*/)
{
    CHECK(set_led_class("trigger","doubleflash"));
    usleep(1000);
    CHECK(set_led_class("delay_on",delay_on));
    CHECK(set_led_class("first_delay_off",first_delay_off));
    CHECK(set_led_class("second_delay_off",second_delay_off));
    return SUCCESS;
}
int Led::set_led_class(const char *file,int value)
{
    char set[20];
    snprintf(set, sizeof(set), "%d", value);
    return set_led_class(file, set);
}
int Led::set_led_class(const char *file, const char *value)
{
    char path[300];
    snprintf(path, sizeof(path), led_class, mIndex, file);

    //ploge("%s %s %s ","set_led_class", path, value);
    char cmd[300];
    snprintf(cmd, sizeof(cmd), "echo %s > %s", value, path);
    ploge("%s", cmd);
    system(cmd);
    /*
    FILE *fp = fopen(path, "wb+");
    if(fp == NULL){
        ploge("open file: %s failed!",path);
        return FAILURE;
    }
    int ret;
    //??
    if(!strcmp(value,"doubleflash"))
        ret = fwrite(value, strlen(value) + 1, 1, fp);
    else
        ret = fwrite(value, strlen(value) + 1, 1, fp);
    if(ret < 0){
        ploge("write %s -> %s failed!",value, path);
        fclose(fp);
        return FAILURE;
    }
    fflush(fp);
    fclose(fp);
    */
    return SUCCESS;
}
}/*namespace Parrot*/