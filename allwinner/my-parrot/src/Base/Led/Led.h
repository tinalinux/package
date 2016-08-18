#ifndef __LED_H__
#define __LED_H__

namespace Parrot{

class Led
{
public:
    Led(int index):mIndex(index){};
    ~Led(){};
    typedef enum _ledStatus
    {
        ON,
        OFF,
        FLASH,
        DOUBLEFLASH
    }tLedStatus;

    int set(tLedStatus status);
    int on();
    int off();
    int flash();
    int doubleflash();
    int flash(int delay_on/*ms*/, int delay_off/*ms*/);
    int doubleflash(int delay_on/*ms*/, int first_delay_off/*ms*/, int second_delay_off/*ms*/);

private:
    int set_led_class(const char *file,int value);
    int set_led_class(const char *file,const char *value);
private:
    int mIndex;

};/*class Led*/
}/*namespace Parrot*/
#endif /*__LED_H__*/