# get kernel_version,target board,boot type
#
# echo:
#	TinaLinux <kernel_version> <RXX> <TARGET> <Boot from>
# echo-example:
#	TinaLinux kernel-3.4.39 R58 octopus-sch sdcard
# example:
#   var=`get_sysinfo`
get_sysinfo()
{
	echo "TinaLinux $(get_kernel_version) $(get_target_board) $(get_boot_type)"
}

# get kernel_version
#
# echo-example:
#	kernel-3.4.39
# example:
#   var=`get_kernel_version`
get_kernel_version()
{
	echo kernel-`uname -r`
}

# get target_board
#
# echo-example:
#	R58 octopus-sch
# example:
#   var=`get_target_board`
get_target_board()
{
	local target=`ubus call system board | grep "\"target\"" | sed 's/\"\|\\\\/ /g' | awk '{print $3}'`
	case "${target}" in
		azalea-*)
			echo "R40 $target"
			;;
		octopus-*)
			echo "R58 $target"
			;;
		astar-*)
			echo "R16 $target"
			;;
		nuclear-*)
			echo "R8 $target"
			;;
		tulip-*)
			echo "R18 $target"
			;;
		*)
			echo "(NULL) $target"
			;;
	esac
}

# get boot_type
#
# echo:
#	"sdcard" or "emmc" or "nand" or "nor-flash"
# example:
#   var=`get_boot_type`
get_boot_type()
{
	local mem=`echo "$partitions" | sed 's/\:/\n/g' | grep "rootfs@" | awk 'BEGIN{FS="@"} {print $2}'`
	case "$mem" in
		mmcblk*)
			local kversion=$(get_kernel_version)
			case "$kversion" in
				kernel-3.4*)
					find /sys/devices -name "mmc0" -type d | grep "sunxi-mmc\.2" &>/dev/null \
						&& echo "emmc" \
						|| echo "sdcard"
					;;
				kernel-3.10*)
					find /sys/devices -name "mmc0" -type d | grep "sdc2" &>/dev/null \
						&& echo "emmc" \
						|| echo "sdcard"
					;;
				kernel-4.4*)
					;;
			esac
			;;
		nand*)
			echo nand
			;;
		mtdblk*)
			echo nor-flash
			;;
	esac
}
