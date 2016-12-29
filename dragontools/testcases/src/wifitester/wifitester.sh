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

if ifconfig -a | grep wlan0; then
	# enable wlan0
	for i in `seq 3`; do
		ifconfig wlan0 up > /dev/null
		if [ $? -ne 0 -a $i -eq 3 ]; then
			echo "ifconfig wlan0 up failed, no more try"
			SEND_CMD_PIPE_FAIL $3
			exit 1
		fi
		if [ $? -ne 0 ]; then
			echo "ifconfig wlan0 up failed, try again 1s later"
			sleep 1
		else
			echo "ifconfig wlan0 up done"
			break
		fi
	done

else
	echo "wlan0 not found, fail"
	SEND_CMD_PIPE_FAIL $3
	exit 1
fi

rm -rf /var/run/wpa_supplicant/wlan0
echo ctrl_interface=/var/run/wpa_supplicant > /etc/wpa_supplicant.conf
wpa_supplicant  -B -i wlan0  -D nl80211 -c /etc/wpa_supplicant.conf

count=0
while true ; do
	flag=`wpa_cli  -iwlan0 scan`
	if [ "$flag" = "OK" ]; then
		count=0
		wifi=`wpa_cli -i wlan0 scan_results | awk '{print $5}'`
		wifi=`echo $wifi | awk '{printf ("SSID: %s %s %s", $2, $3, $4)}'`
		echo $wifi
		# test done
		SEND_CMD_PIPE_OK_EX $3 "$wifi"
	else
		let count=$count+1
		sleep 3
		continue
	fi

	if [ $count -gt 10 ]; then
		SEND_CMD_PIPE_FAIL $3
		break
	fi
	sleep 3
done

# test failed
echo "wlan0 not found, no more try"
SEND_CMD_PIPE_FAIL $3
exit 1
