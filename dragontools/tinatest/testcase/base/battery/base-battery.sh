#!/bin/sh

#Battery Events
#ac_present #usb_present
#battery_present
#status
#health
#capacity
#vol_now
#current_now
#temp
#bat_low_capacity
#ic_over_temp

get_bat_status()
{
    local status=`cat /sys/class/power_supply/battery/status`
    case $status in
    0) echo Unknown;;
    1) echo Charging;;
    2) echo Discharging;;
    3) echo Not Charging;;
    4) echo Full;;
    *) echo ErrorStatus;;
    esac
}

parse_battery_event()
{
    case $1 in
    "ac_present")
        echo "------------------------------------------------------------"
        echo -n "ac present change! ac_present=";
        cat /sys/class/power_supply/ac/present;;
    "usb_present")
        echo "------------------------------------------------------------"
        echo -n "usb present change! usb_present=";
        cat /sys/class/power_supply/usb/present;;
    "battery_present")
        echo "------------------------------------------------------------"
        echo -n "battery present change! battery_present=";
        cat /sys/class/power_supply/battery/present;;
    "battery_present")
        echo "------------------------------------------------------------"
        echo -n "usb present change! usb_present=";
        cat /sys/class/power_supply/usb/present;;
    "status")
        echo "------------------------------------------------------------"
        echo -n "bat status change! bat status=";
	get_bat_status;;
    "health")
        echo "------------------------------------------------------------"
        echo -n "bat health change! bat health=";
        cat /sys/class/power_supply/battery/health;;
    "capacity")
        echo "------------------------------------------------------------"
        echo -n "bat capacity change! bat capacity=";
        cat /sys/class/power_supply/battery/capacity;;
    "vol_now")
        echo "------------------------------------------------------------"
        echo -n "bat voltage change! bat voltage=";
        cat /sys/class/power_supply/battery/voltage_now;;
    "current_now")
        echo "------------------------------------------------------------"
        echo -n "bat current change! bat current=";
        cat /sys/class/power_supply/battery/current_now;;
    "temp")
        echo "------------------------------------------------------------"
        echo -n "bat temp change! bat temp=";
        cat /sys/class/power_supply/battery/temp;;
    "bat_low_capacity")
        echo "------------------------------------------------------------"
        echo -n "bat low capacity warning! bat capacity=";
        cat /sys/class/power_supply/battery/capacity;;
    "ic_over_temp")
        echo "------------------------------------------------------------"
        echo -n "pmic over temp warning! pmic temp=";
        cat /sys/class/power_supply/battery/temp_ambient;;
    esac
}

BatteryListenEvent()
{
    local count=0
    echo "Entry Battery Test"
    echo "Start to listen battery event..."

    dbus-monitor --system --profile "type='signal',interface='healthd.signal.interface'" | awk '{print $8}' |\
    while read line
    do
        count=$(($count+1))
        parse_battery_event $line
    done
}

BatteryCollectInfo()
{
    local internal;

    echo "Entry Battery Test"
    echo "Start to collect battery info..."
    echo
    printf 'time\tcapacity\tstatus\n'
    while true
    do
        local time=`date "+%H:%M:%S"`;
	local capacity=`cat /sys/class/power_supply/battery/capacity 2>/dev/null || echo 100`;
	local status=`cat /sys/class/power_supply/battery/status 2>/dev/null || echo 0`
	printf '%s\t%s\t%s\n' $time $capacity $status
	if [ $capacity -lt 20 ]; then
	    internal=10;
	elif [ $capacity -lt 80 ]; then
	    internal=30;
	else
	    internal=60;
	fi
	sleep $internal
    done
}

usage()
{
   local name=`basename $0`
   echo
   echo "Usage: $name func"
   echo "func: 1-listen battery event; 2-collect battery information"
   echo
}

if [ $# != 1 ]; then
    usage;
    exit;
fi

case $1 in
1)BatteryListenEvent;;
2)BatteryCollectInfo;;
*)usage;;
esac

exit
