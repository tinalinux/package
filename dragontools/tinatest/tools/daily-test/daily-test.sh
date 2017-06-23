#!/bin/bash
# 说明: 此脚本给主机(PC)使用,用于执行每日自动化测试
#       主机通过adb控制所有USB连接的开发板,执行tinatest
#       收集markdwon格式的report,并用pandoc转化为html(含css)
# 执行: ./daily-test.sh [输出文件夹]]

# ============================================================================
# GLOBAL VARIABLES
# ============================================================================
CSS="$(dirname $0)/daily-test.css.in"
REPORT="/mnt/UDISK/md/report.md"
MJSON="/etc/tinatest.json"
WAIT_TT_END_HOUR="1"
WAIT_TT_END_MIN="30"
WAIT_TT_END_SEC="0"

# ============================================================================
# GLOBAL FUNCTIONS
# ============================================================================
adb_shell() {
    local serial cmd
    serial="$1"
    shift
    eval "cmd='$@'"
    eval "adb -s ${serial} shell '${cmd}' | sed 's/\\r//g'"
}

check_depend_host_do() {
    echo -n "Checkinng $1 ... "
    which $1 &>/dev/null && echo yes || {
        echo -e "\r\033[31mChecking $1 ... no\033[0m"
        exit 1
    }
}

check_depend_device_do() {
    echo -n "Checking $2 ($1) ... "
    [ -n "$(adb_shell "$1" "which $2" 2>/dev/null)" ] && echo yes || {
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
    if [ -n "$(adb_shell $1 "[ -f \"${MJSON}\" ] && echo yes" 2>/dev/null)" ]; then
        if [ -n "$(adb_shell $1 "grep outlog_markdown -A 2 ${MJSON} | grep true && echo yes")" ]; then
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
        TARGETS=(${TARGETS[@]} $(adb_shell ${USB_SERIAL[$num]} get_target | sed 's/\r//g'))
    done
    USB_SERIAL=(${USB_SERIAL[@]})
    PORTS=(${PORTS[@]})
}

check_usb_devices() {
    echo -e "\033[36m##### check usb device #####\033[0m"
    PORTS=($(adb devices -l | awk '$2 == "device"{print $3}' | sed 's/:/-/g'))
    USB_SERIAL=($(adb devices -l | awk '$2 == "device"{print $1}'))
    adb devices -l | awk '$2 == "device"{printf "port: %s && serial: %s\n", $3, $1}'
}

wait_tt_end() {
    echo -n "Waiting tinatest ... "
    now=$(date +%s)
    max=$(( ${now} + ${WAIT_TT_END_SEC} + 60 * ${WAIT_TT_END_MIN} + 60 * 60 * ${WAIT_TT_END_HOUR} ))
    while [ "${now}" -lt "${max}" ]
    do
        adb -s $1 wait-for-device
        adb -s $1 shell "ps 2>/dev/null" 2>/dev/null | grep "tinatest" &>/dev/null || {
            sleep 5
            adb devices | grep -w "^$1" &>/dev/null || continue
            adb -s $1 shell "ps 2>/dev/null" 2>/dev/null | grep "tinatest" &>/dev/null || break
        }
        now=$(date +%s)
    done
    [ "${now}" -eq "${max}" ] \
        && {
            echo -e "\r\003[31mWaiting tinatest ... failed - timeout\003[0m"
            return 1
        } || echo "yes"
}

clean_old() {
    echo -n "Cleaning old ... "
    adb_shell $1 rm -rf ${REPORT}
    [ "$(adb_shell $1 "[ -f ${REPORT} ] && echo yes")" != "yes" ] \
        && echo yes \
        || {
            echo -e "\r\033[31mCleaning old ... failed\033[0m\n"
            return 1
        }
}

run_tt() {
    echo -n "Running tinatest ... "
    adb_shell $1 "tinatest ${TASKS}" &>/dev/null
    echo yes
}

get_report() {
    echo -n "Getting markdown ... "
    adb -s $1 pull ${REPORT} ${out_md} &>/dev/null
    [ -f "${out_md}" ] && echo yes || {
        echo -e "\r\033[31mGetting markdown ... failed\033[0m"
        return 1
    }
}

convert_html() {
    echo -n "Converting markdown ... "
    # transform to html
    [ -f "${CSS}" ] \
        && pandoc -o ${out_html} ${out_md} -H ${CSS} &>/dev/null \
        || pandoc -o ${out_html} ${out_md} &>/dev/null

    # check
    [ -f "${out_html}" ] && echo yes || echo -e "\r\033[31mConverting markdown ... failed\033[0m"
}

# begin_do target port usb_serial
begin_do() {
    ftmp="/tmp/tinatest-$1_$2_$3"
    out_md="${OUTDIR}$1_$2_$3.md"
    out_html="${OUTDIR}$1_$2_$3.html"

    echo "** $1 : $2 : $3 **" > ${ftmp}
    clean_old "$3" >> ${ftmp} \
        && run_tt "$3" >> ${ftmp} \
        && wait_tt_end "$3" >> ${ftmp} \
        && get_report "$3" >> ${ftmp} \
        && convert_html >> ${ftmp}

    cat ${ftmp}
    rm ${ftmp}
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
    [ -z "$(echo ${USB_SERIAL[@]})" ] && echo -e "\033[31m *** None ***\033[0m" && exit 1
    for num in `seq 0 $(( ${#PORTS[@]} - 1 ))`
    do
        echo "${TARGETS[$num]} : ${PORTS[$num]} : ${USB_SERIAL[$num]}"
    done

    rm -f ${OUTDIR}*.md ${OUTDIR}*.html
    echo -e "\033[36m##### begin #####\033[0m"
    for num in `seq 0 $(( ${#PORTS[@]} - 1 ))`
    do
        echo "** ${TARGETS[$num]}:${PORTS[$num]}:${USB_SERIAL[$num]} **"
        begin_do ${TARGETS[$num]} ${PORTS[$num]} ${USB_SERIAL[$num]} &
    done
    echo -e "\033[36m##### wait #####\033[0m"
    wait
    echo -e "\033[36m##### end #####\033[0m"
}

check() {
    check_depend_host
    check_usb_devices
    check_depend_device
}

# ============================================================================
# MAIN
# ============================================================================
begin $@
