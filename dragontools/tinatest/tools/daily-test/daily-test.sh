#!/bin/bash
# 说明: 此脚本给主机(PC)使用,用于执行每日自动化测试
#       主机通过adb控制所有USB连接的开发板,执行tinatest
#       收集markdwon格式的report,并用pandoc转化为html(含css)
# 执行: ./daily-test.sh [输出文件夹]]

check_depend_host_do() {
    echo -n "Checkinng $1 ... "
    which $1 &>/dev/null && echo yes || {
        echo -e "\r\033[31mChecking $1 ... no\033[0m"
        exit
    }
}

check_depend_device_do() {
    echo -n "Checking $2 ($1) ... "
    [ -n "$(adb -s $1 shell which $2 2>/dev/null)" ] && echo yes || {
        echo -e "\r\033[31mChecking $2 ($1) ... no - ignore this device\033[0m"
        return 1
    }
}

check_depend_host() {
    echo -e "\033[36m##### check host depend #####\033[0m"
    check_depend_host_do "echo"
    check_depend_host_do "egrep"
    check_depend_host_do "find"
    check_depend_host_do "awk"
    check_depend_host_do "sed"
    check_depend_host_do "adb"
    check_depend_host_do "pandoc"
}

check_enable_markdown() {
    echo -n "Checkinng tinatest.json ($1) ... "
    if [ -n "$(adb -s $1 shell "[ -f \"${MJSON}\" ] && echo yes" 2>/dev/null)" ]; then
        if [ -n "$(adb -s $1 shell "grep outlog_markdown -A 2 ${MJSON} | grep true && echo yes")" ]; then
            echo yes
            return 0
        else
            echo -e "\r\033[31mChecking tinatest.json ($1) ... unset markdown - ignore this device\033[0m"
            return 1
        fi
    else
        echo -e "\r\033[31mChecking tinatest.json ($1) ... no existed - ignore this device\033[0m"
        return 1
    fi
}

check_depend_device() {
    echo -e "\033[36m##### check device depend #####\033[0m"
    local num
    for num in `seq 0 $(( ${#PORTS[@]} - 1 ))`
    do
        check_enable_markdown "${USB_SERIAL[$num]}" \
            &&check_depend_device_do "${USB_SERIAL[$num]}" "tinatest" \
            && check_depend_device_do "${USB_SERIAL[$num]}" "get_target" \
            && check_depend_device_do "${USB_SERIAL[$num]}" "ubus" \
            || {
                unset USB_SERIAL[$num]
                unset PORTS[$num]
                continue
            }
        TARGETS=(${TARGETS[@]} $(adb -s ${USB_SERIAL[$num]} shell get_target | sed 's/\r//g'))
    done
    USB_SERIAL=(${USB_SERIAL[@]})
    PORTS=(${PORTS[@]})
}

check_usb_devices() {
    echo -e "\033[36m##### check usb device #####\033[0m"
    local one tmp port serial
    for one in $(find /sys/devices -name "usb*" -type d | egrep "usb[[:digit:]]+$")
    do
        for tmp in $(find ${one}/*/ -name "serial" -type f 2>/dev/null)
        do
            port="$(awk -F/ '{print $(NF-1)}' <<< ${tmp})"
            serial="$(cat ${tmp})"
            if [ -n "${SET_SERIAL}" ]; then
                grep -w "${serial}" <<< ${SET_SERIAL} &>/dev/null || continue
            fi

            PORTS=(${PORTS[@]} ${port})
            USB_SERIAL=(${USB_SERIAL[@]} ${serial})
            echo "bus-port: ${port} && serial: ${serial}"
        done
    done
}

# begin_do target port usb_serial
begin_do() {
    out_md="${OUTDIR}$1_$2_$3.md"
    out_html="${OUTDIR}$1_$2_$3.html"
    # clear old file
    adb -s $3 shell rm -rf ${REPORT}
    rm -rf ${out_md} ${out_html}
    # run tinatest
    adb -s $3 shell tinatest ${TASKS} >/dev/null
    # get report
    adb -s $3 pull ${REPORT} ${out_md} 2>/dev/null
    if [ -f "${out_md}" ]; then
        # transform to html
        [ -f "${CSS}" ] \
            && pandoc -o ${out_html} ${out_md} -H ${CSS} \
            || pandoc -o ${out_html} ${out_md}

        # check
        [ -f "${out_html}" ] \
            && echo -e "$1 : $2 : $3 - Success" \
            && return 0
    fi
    echo -e "\033[31m$1 : $2 : $3 - Failed\033[0m"
}

show_help() {
    echo "功能:"
    echo "    对所有连接上PC的Tina设备(必须已安装tinatest且使能outlog_markdown)"
    echo "    执行tinatest, 导出markdwon文件,并用pandoc转换为html."
    echo "    转换的html会附加上css,可直接在浏览器中打开"
    echo ""
    echo "使用:"
    echo "      daily-test.sh [-h] [-o <输出文件夹>] [-s <串号1>[,<串号2>,...]] [测试用例路径1] [测试用例路径2] ..."
    echo ""
    echo "参数说明:"
    echo "    -o <输出文件夹> : 指定输出文件夹"
    echo "    -s <设备串号> : 指定测试测试的串口,多个设备以逗号分割"
    echo ""
    echo "例子:"
    echo "    ./daily-test.sh -o ~/out -s "20080411" /demo/demo-c /base"
    echo "    执行 /demo 和 /base 的测试用例, 且输出文件到 ~/out 目录"
}

begin() {
    local opts="$(getopt -o "hs:o:" -- $@)" || return 1
    eval set -- "${opts}"
    while true
    do
        case "$1" in
            -h)
                show_help
                exit 0
                ;;
            -o)
                shift
                OUTDIR="$(sed 's#/$##' <<< $1)/"
                [ ! -d "${OUTDIR}" ] && echo "Not found dir ${OUTDIR}" && exit
                shift
                ;;
            -s)
                shift
                SET_SERIAL="$1"
                shift
                ;;
            --)
                shift
                break
                ;;
            *)
                shift
                ;;
        esac
    done

    TASKS="$@"

    check
    echo -e "\033[36m##### valid devices #####\033[0m"
    [ -z "$(echo ${USB_SERIAL[@]})" ] && echo -e "\033[31m *** None ***\033[0m" && exit
    for num in `seq 0 $(( ${#PORTS[@]} - 1 ))`
    do
        echo "${TARGETS[$num]} : ${PORTS[$num]} : ${USB_SERIAL[$num]}"
    done


    echo -e "\033[36m##### begin #####\033[0m"
    for num in `seq 0 $(( ${#PORTS[@]} - 1 ))`
    do
        begin_do ${TARGETS[$num]} ${PORTS[$num]} ${USB_SERIAL[$num]} &
    done
    wait
    echo -e "\033[36m##### end #####\033[0m"
}

check() {
    check_depend_host
    check_usb_devices
    check_depend_device
}

CSS="$(dirname $0)/daily-test.css.in"
REPORT="/mnt/UDISK/md/report.md"
MJSON="/etc/tinatest.json"
begin $@
