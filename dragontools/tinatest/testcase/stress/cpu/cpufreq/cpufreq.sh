#!/bin/ash

if [ ! $1 ]; then
	echo "please input test counter"
	exit
fi

if [ ! $2 ]; then
	echo "please input online cpu sum"
	exit
fi

if [ "x$3" != "xlinux" ] && [ "x$3" != "xandroid" ]; then
	echo "please input test platform: linux or android"
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
if [ $2 -eq 1 ]; then
	echo "cpu0 already online"
elif [ $2 -gt $all_cpu ]; then
	echo "online cpu number set error $2 $all_cpu"
	exit
fi

# disable all non-boot cpus
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

# enable
tmp_count=1
while [ $tmp_count -lt $2 ]
do
	echo 1 > /sys/devices/system/cpu/cpu$tmp_count/online
	let tmp_count++
done

online=`cat /sys/devices/system/cpu/online`
echo "online cpu [$online]"

# ash does not support array, so a substitute way.
#CPUFREQ_ARRAY=(
#	240000
#	480000
#	600000
#	720000
#	816000
#	912000
#	1008000
#)
CPUFREQ_ARRAY="240000 480000 600000 720000 816000 912000 1008000"

#cpufreq env save
cpufreq_env_save()
{
	governor_save=`cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor`
	freq_save=`cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq`
}

#cpufreq env restore
cpufreq_env_restore()
{
	echo $freq_save > /sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed
	echo restore $freq_target done!

	cpuinfo_cur_freq=`cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq`
	echo Freq[$cpuinfo_cur_freq] restore OK

	echo $governor_save > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor

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

cpufreq_env_save
echo userspace > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
sleep 1

size=`echo $CPUFREQ_ARRAY | awk '{print NF}'`
echo cpufreq_array_size=$size
echo ""

max_scaling_freq=`cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq`
min_scaling_freq=`cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq`
echo "====================================="
echo "min_scaling_freq is $min_scaling_freq"
echo "max_scaling_freq is $max_scaling_freq"
echo "====================================="

cpufreq_scaling()
{
	cur_count=1
	while [ $cur_count -le $1 ]
	do
		echo No.$cur_count
		case "$3" in
			random)
				# ash does not support $RANDOM, so ...
				#number=$RANDOM
				# srand()'s seed, by default, keys off of current date+time.
				# if awk is called multiple times within the same second,
				# you almost certainly will get the same value.
				#number=`awk 'BEGIN { srand(); print int(rand() * 32767); }'`
				number=$(</dev/urandom tr -dc 0-9 | head -c 5 | sed 's/^0\+//g')
				let "number %= $size"
				let "number += 1"
				freq_target=`echo $CPUFREQ_ARRAY | awk '{print $num}' num="$number"`
				;;
			seq)
				number=$cur_count
				let "number %= $size"
				let "number += 1"
				freq_target=`echo $CPUFREQ_ARRAY | awk '{print $num}' num="$number"`
				;;
			maxmin)
				number=$cur_count
				let "number %= 2"
				if [ $number -eq 1 ];then
					min_freq=`cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_min_freq`
					freq_target=$min_freq
				else
					max_freq=`cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq`
					freq_target=$max_freq
				fi
				;;
			*)
				return
				;;
		esac

		echo freq_target=$freq_target
		echo $freq_target > /sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed
		echo set $freq_target done!

		cur_freq=`cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq`
		echo Freq[$cur_freq] OK
		cur_temp=`cat /sys/class/thermal/thermal_zone0/temp`
		echo temperature=$cur_temp C

		echo ""
		let cur_count++

		if [ $2 == "linux" ]; then
			#usleep 30000
			sleep 1
		elif [ $2 == "android" ]; then
			sleep 0.03
		fi
	done
}

cpufreq_scaling $1 $3 random
cpufreq_scaling $1 $3 seq
cpufreq_scaling $1 $3 maxmin

cpufreq_env_restore

echo "$0 success"
