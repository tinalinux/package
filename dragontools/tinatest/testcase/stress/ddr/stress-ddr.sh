#!/bin/sh

debug_node=/sys/class/sunxi_dump/dump
#sdram config registe
platform=`get_platform`
case ${platform} in
    a64|A64)
        SDCR="0x01c62000"
        ;;
    *)
        SDCR="0xf1c62000"
        ;;
esac

reg_read()
{
 echo $1 > ${debug_node}
 cat ${debug_node}

}

get_dram_size()
{
    local dram_freq
    sdcr_value=`reg_read $SDCR`
    let "page_size=(${sdcr_value}>>8)&0xf"
    if [ ${page_size} -eq 7 ]; then
        dram_size=1
    elif [ ${page_size} -eq 8 ]; then
        dram_size=2
    elif [ ${page_size} -eq 9 ]; then
        dram_size=4
    elif [ ${page_size} -eq 10 ]; then
        dram_size=8
    else
        dram_size=0
    fi

    let "row_addr_width=(${sdcr_value}>>4)&0xf"
    let "dram_size *=(1<<(${row_addr_width}-9))"
    let "bank_addr_width=(${sdcr_value}>>2)&0x3"
    let "dram_size *=(4<<${bank_addr_width})"
    let "dual_channel_enable=(${sdcr_value}>>19)&0x1"
    let "dram_size *=(${dual_channel_enable}+1)"
    let "rank_addr_width=(${sdcr_value}>>0)&0x3"
    let "dram_size *=(${rank_addr_width}+1)"
    echo ${dram_size}

}

workers=`mjson_fetch /stress/ddr/workers`
dram_size=`mjson_fetch /stress/ddr/dram_size`
test_size=`mjson_fetch /stress/ddr/test_size`
loop_times=`mjson_fetch /stress/ddr/loop_times`
actual_size=`get_dram_size`

case "${platform}" in
    a64|A64)
        dram_freq=`cat /sys/class/devfreq/dramfreq/cur_freq | awk -F: '{print $1}'`
        ;;
    r40|R40)
        dram_freq=`cat /sys/devices/1c62000.dramfreq/devfreq/dramfreq/cur_freq \
            | awk -F: '{print $1}'`
        ;;
    *)
        dram_freq=`cat /sys/devices/platform/sunxi-ddrfreq/devfreq/sunxi-ddrfreq/cur_freq \
            | awk -F: '{print $1}'`
        ;;
esac

let "dram_freq=${dram_freq}/1000"

echo "dram_freq=${dram_freq}"
echo "config dram_size=${dram_size}"
echo "actual_size=${actual_size}m"
echo "test_size=${test_size}"
echo "workers=${workers}"

if [ ${actual_size} -lt ${dram_size} ]; then
   echo ERROR: "size ${actual_size}m error"
   exit 1
fi

memtester ${test_size} ${loop_times}
