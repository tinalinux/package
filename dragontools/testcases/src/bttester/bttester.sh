#!/bin/sh
##############################################################################
# \version     1.0.0
# \date        2012年05月31日
# \author      James Deng <csjamesdeng@allwinnertech.com>
# \Descriptions:
#			create the inital version

# \version     1.1.0
# \date        2012年09月26日
# \author      Martin <zhengjiewen@allwinnertech.com>
# \Descriptions:
#			add some new features:
#			1.wifi hotpoint ssid and single strongth san
#			2.sort the hotpoint by single strongth quickly
##############################################################################
source send_cmd_pipe.sh
source script_parser.sh
module_path=`script_fetch "bluetooth" "module_path"`
loop_time=`script_fetch "bluetooth" "test_time"`
destination_bt=`script_fetch "bluetooth" "dst_bt"`
device_node=`script_fetch "bluetooth" "device_node"`
baud_rate=`script_fetch "bluetooth" "baud_rate"`

echo $module_path
echo $loop_time
echo "${destination_bt}"
echo $device_node

echo 0 > /sys/class/rfkill/rfkill0/state
sleep 2
echo 1 > /sys/class/rfkill/rfkill0/state
sleep 2

#bluetooltester --tosleep=50000 --no2bytes --enable_hci --scopcm=0,2,0,0,0,0,0,0,0,0  --baudrate 1500000 --use_baudrate_for_download --enable_lpm --patchram ${module_path}  $device_node &
brcm_patchram_plus  --tosleep=200000 --no2bytes --enable_hci --scopcm=0,2,0,0,0,0,0,0,0,0  --baudrate ${baud_rate} --patchram ${module_path}  $device_node &
sleep 10

i=0
while true;
do
	let i=$i+1
	if [ $i -gt 20 ]; then
		break;
	fi
	rfkillpath="/sys/class/rfkill/rfkill"${i}
	if [ -d "$rfkillpath" ]; then
		if cat $rfkillpath"/type" | grep bluetooth; then
			statepath=${rfkillpath}"/state"
			echo $statepath
			echo 1 > $statepath
			sleep 1
			break
		fi
	fi
done


hciconfig hci0 up
sleep 1

count=0
while true;  do
	let count=$count+1
	if [ $count -gt 20 ]; then
		hciconfig hci0 down
		SEND_CMD_PIPE_FAIL $3
		break;
	fi
	#result=`hcitool scan | grep "${destination_bt}" | awk '{print $2}'`
	result=`cat /sys/devices/soc/uart3/tty/ttyS3/hci0/name`
	if [ -z $result ]; then
		echo "no reslut"
	else
		count=0
		SEND_CMD_PIPE_OK_EX $3 "$result"
		exit 0
	fi
	sleep 3
done
