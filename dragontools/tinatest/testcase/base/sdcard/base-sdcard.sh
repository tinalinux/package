#!/bin/sh

count=0
boot_type=`get_boot_media`
echo "boot from ${boot_type}"
case  "${boot_type}" in
    emmc)
        while true; do
            let count=${count}+1
            if [ -b "/dev/mmcblk1" ]; then
                echo "Found sdcard in /dev/mmcblk1"
                echo "TF-card test successfully!!"
                exit 0
            fi
            if [ "${count}" -gt 60 ]; then
                echo "NOT Fount sdcard"
                echo "TF-card test fail!!"
                exit 1
            fi
            sleep 1
        done
        ;;
    *)
        while true; do
            let count=$count+1
            if [ -b "/dev/mmcblk0" ]; then
                echo "Found sdcard in /dev/mmcblk0"
                echo "TF-card test successfully!!"
                exit 0
            fi
            if [ $count -gt 60 ]; then
                echo "NOT Fount sdcard"
                echo "TF-card test fail!!"
                exit 1
            fi
            sleep 1
        done
        ;;
esac
