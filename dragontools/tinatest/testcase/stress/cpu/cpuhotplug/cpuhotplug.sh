#!/bin/ash

if [ ! $1 ]; then
	echo "please input test counter"
	exit
fi

if [ "x$2" != "xlinux" ] && [ "x$2" != "xandroid" ]; then
	echo "please input test platform: linux or android"
	exit
fi

TARGET=$(get_target)
if [ "x$TARGET" == "x" ]
then
	echo "TARGET is NULL! exit."
	exit
fi

if [ "x$TARGET" == "xsitar-evb" ]
then
	echo "$TARGET have only one CPU, and can not hotplug it."
	exit
fi

echo ""
echo "$0 start"

autohotplug_file="/sys/kernel/autohotplug/enable"
if [ -f "$autohotplug_file" ]; then
	hotplug_enable=`cat /sys/kernel/autohotplug/enable`
	if [ $hotplug_enable -eq 1 ]; then
		echo 0 > /sys/kernel/autohotplug/enable
	fi
fi

all_cpu=`cat /sys/devices/system/cpu/kernel_max`
let "all_cpu += 1"

#disble all non-boot cpus
tmp_count=1
while [ $tmp_count -lt $all_cpu ]
do
        cpux_on=`cat /sys/devices/system/cpu/cpu$tmp_count/online`
	cpus_status="$cpus_status $cpux_on"
        if [ $cpux_on -eq 1 ]; then
		echo 0 > /sys/devices/system/cpu/cpu$tmp_count/online
	fi
        let tmp_count++
done

#resotre cpu hotplug env
cpuhotplug_env_restore()
{
	tmp_count=1
	while [ $tmp_count -lt $all_cpu ]
	do
		cpux_on=`cat /sys/devices/system/cpu/cpu$tmp_count/online`
		cpux_saved=`echo $cpus_status | awk '{print $num}' num="$tmp_count"`
		if [ "x$cpux_on" != "x$cpux_saved" ]; then
			echo $cpux_saved > /sys/devices/system/cpu/cpu$tmp_count/online
		fi
		let tmp_count++
	done

	if [ $hotplug_enable -eq 1 ]; then
		echo 1 > /sys/kernel/autohotplug/enable
	fi
}

cur_count=1
while [ $cur_count -le $1 ]
do
	echo "test time $cur_count"

	number=$(</dev/urandom tr -dc 0-9 | head -c 5 | sed 's/^0\+//g')
	let "number %= ($all_cpu - 1)"
	let "number += 1"
	kill_cpu=$number
	result=`cat /sys/devices/system/cpu/cpu$kill_cpu/online`

	if [ $result -eq 1 ]; then
		echo "kill cpu$kill_cpu"
		echo "-----------------"
		echo 0 > /sys/devices/system/cpu/cpu${kill_cpu}/online
	else
		echo "bringup cpu$kill_cpu"
		echo "-----------------"
		echo 1 > /sys/devices/system/cpu/cpu${kill_cpu}/online
	fi

	if [ $2 == "linux" ]; then
	#	usleep 30000
		sleep 1
	elif [ $2 == "android" ]; then
		sleep 0.03
	fi

	online=`cat /sys/devices/system/cpu/online`
	echo "online: [$online]"
	cur_temp=`cat /sys/class/thermal/thermal_zone0/temp`
	echo temperature=$cur_temp C
	echo ""

	let cur_count++
done

cpuhotplug_env_restore

echo "$0 success"
