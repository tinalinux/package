#!/bin/sh

source send_cmd_pipe.sh
source script_parser.sh

ROOT_DEVICE=/dev/nandd
for parm in $(cat /proc/cmdline); do
    case $parm in
        root=*)
            ROOT_DEVICE=`echo $parm | awk -F\= '{print $2}'`
            ;;
    esac
done

BOOT_TYPE=-1
for parm in $(cat /proc/cmdline); do
    case $parm in
        boot_type=*)
            BOOT_TYPE=`echo $parm | awk -F\= '{print $2}'`
            ;;
    esac
done

case $ROOT_DEVICE in
    /dev/nand*)
        echo "nand boot"
        mount /dev/nanda /boot
        ;;
    /dev/mmc*)
        case $BOOT_TYPE in
            2)
                echo "emmc boot,boot_type = $BOOT_TYPE"
                mount /dev/mmcblk0p2 /boot
                SEND_CMD_PIPE_OK_EX $3
                exit 1
                ;;
            *)
                echo "card boot,boot_type = $BOOT_TYPE"
                ;;
        esac
        ;;
    *)
        echo "default boot type"
        mount /dev/nanda /boot
        ;;
esac

#only in card boot mode,it will run here
if [ ! -b "/dev/mmcblk1" ]; then
	#/dev/mmcblk1 is not exist,maybe emmc driver hasn't been insmod
	SEND_CMD_PIPE_FAIL_EX $3 "11"
	exit 1
fi

flashdev="/dev/mmcblk1"
echo "flashdev=$flashdev"

mkfs.vfat $flashdev
if [ $? -ne 0 ]; then
    SEND_CMD_PIPE_FAIL_EX $3 "22"
    exit 1
fi

test_size=`script_fetch "emmc" "test_size"`
if [ -z "$test_size" -o $test_size -le 0 -o $test_size -gt $total_size ]; then
    test_size=64
fi

echo "test_size=$test_size"
echo "emmc test read and write"

emmcrw "$flashdev" "$test_size"
if [ $? -ne 0 ]; then
    SEND_CMD_PIPE_FAIL_EX $3 "33"
    echo "emmc test fail!!"
else
    SEND_CMD_PIPE_OK_EX $3
    echo "emmc test ok!!"
fi
