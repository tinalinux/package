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
		kill $(ps | awk '($5~/^brcm_patchram_plus/){print $1}')
		SEND_CMD_PIPE_FAIL $3
		echo "bt check failed"
		exit -1
	fi
	#result=`hcitool scan | grep "${destination_bt}" | awk '{print $2}' | head -n 1`
	result=`cat $(find /sys/devices/ -name hci0 -type d | head -n 1)/name | awk '{print $1}'`
	if [ -z "$result" ]; then
		echo "no reslut"
	else
		count=0
		kill $(ps | awk '($5~/^brcm_patchram_plus/){print $1}')
		SEND_CMD_PIPE_OK_EX $3 "$result"
		exit 0
	fi
	sleep 3
done
