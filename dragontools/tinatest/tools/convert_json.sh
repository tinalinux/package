#!/bin/bash

outlog_list=(
    #config-name:config-type
    "@outlog_serial:bool"
    "@outlog_markdown/enable:bool"
    "@outlog_markdown/outdir:string"
)

collectd_list=(
    #config-name:config-type
    "@collectd_interval_sec:int"
    "@collectd_csv/enable:bool"
    "@collectd_csv/outdir:string"
    "@collectd_rrdtool/enable:bool"
    "@collectd_rrdtool/outdir:string"
    "@collectd_cpu/enable:bool"
    "@collectd_cpu/report_to_percentage:bool"
    "@collectd_memory/enable:bool"
    "@collectd_memory/report_to_percentage:bool"
    "@collectd_memory/report_to_absolute:bool"
    "@collectd_df/enable:bool"
    "@collectd_df/report_to_percentage:bool"
    "@collectd_df/report_to_absolute:bool"
    "@collectd_df/match/sellect_or_ignore:string"
    "@collectd_df/match/device:string"
    "@collectd_df/match/mountpoint:string"
    "@collectd_df/match/fstype:string"
    "@collectd_disk/enable:bool"
    "@collectd_disk/match/sellect_or_ignore:string"
    "@collectd_disk/match/disk_regular_expression:string"
    "@collectd_filecount/enable:bool"
    "@collectd_filecount/directory:string"
    "@collectd_filecount/include_hidden:bool"
    "@collectd_filecount/include_subdir:bool"
    "@collectd_filecount/match/name:string"
    "@collectd_filecount/match/size:string"
    "@collectd_filecount/match/mtime:string"
    "@collectd_ping/enable:bool"
    "@collectd_ping/host:string"
    "@collectd_ping/send_interval_sec:string"
    "@collectd_ping/timeout_sec:string"
    "@collectd_ping/max_ttl:int"
)

testcase_config_list=(
    #config-name:config-type
    "enable:bool"   #the first config must be enable
    "command:string"
    "stdin:array"
    "fstdin:string"
    #info
    "date:bool"
    "resource:bool"
    #limit
    "run_times:int"
    "run_alone:bool"
    "run_parallel:bool"
    "may_reboot:bool"
    "run_time_sec:int"
    "run_time_min:int"
    "run_time_hour:int"
)
testcase_config_cnt=${#testcase_config_list[@]}

#draw_indent <tab_cnt>
draw_indent() {
    local cnt
    for cnt in `seq $1`
    do
        echo -n "    "
    done
}

#testcase_convert <depth> <TREE_BASE @ tree1> <TREE_BASE @ tree2> ...
testcase_convert() {
    local depth bases forest trunks branches
    local base trunk branch num tmp marks mark list
    depth="$1" && shift
    bases=(${@%%@*})
    forest=($(echo ${@#*@} | sed 's/ /\/$ /g' | sed 's/\(.*\)/\1\/$/'))
    trunks=(${forest[@]%%/*})
    branches=(${forest[@]#*/})

    for num in `seq 0 $(( ${#forest[@]} - 1 ))`
    do
        trunk="${trunks[${num}]}"
        grep -w "${trunk}" <<< ${marks[@]} &> /dev/null || \
            marks=(${marks[@]} ${trunk})
        tmp="$(sed 's/[^ [:alnum:]]/_/g' <<< ${trunk})"
        base="${bases[${num}]}_$(sed 's/[a-z]/\u&/g' <<< ${tmp})"
        eval "local mark_${tmp}=\"\${mark_${tmp}} ${base}@${branches[${num}]}\""
    done

    num=0
    for mark in ${marks[@]}
    do
        branch=$(eval "echo \${mark_$(sed 's/[^ [:alnum:]]/_/g' <<< ${mark})}")
        draw_indent ${depth} && echo "\"${mark}\" : {"
        for tmp in ${branch}
        do
            if [ "${tmp#*@}" = "$" ]; then
                testcase_common $(( ${depth} + 1 )) ${tmp%%@*} ${mark}
            else
                list="${list} ${tmp%/*}"
            fi
        done
        [ -n "${list}" ] && testcase_convert $(( ${depth} + 1 )) ${list}
        draw_indent ${depth} && echo -n "}"
        num=$(( ${num} + 1 ))
        [ "${num}" -lt "${#marks[@]}" ] && echo "," || echo
        unset list
    done
}

#testcase_common <depth> <base> <name>
testcase_common() {
    local config_type config_name config_value
    local line cnt tmp class private_conf private_cnt cnt_max

    class=$(awk -F '_' '{print $3}' <<< $2 | sed 's/[A-Z]/\l&/g')
    tmp="${top}/testcase$(grep "$3" ${top}/testcase/${class}/list | head -n 1)/private.conf"
    [ -f "${tmp}" ] && {
        private_conf=($(egrep "^.*:.*=.*$" ${tmp} | awk -F ' ' '{print $1}'))
        private_cnt=${#private_conf[@]}
    }

    cnt_max=$(( ${private_cnt} + ${testcase_config_cnt} ))

    for line in ${testcase_config_list[@]} ${private_conf[@]}
    do
        config_type=${line##*:}
        config_name=${line%%:*}
        tmp=$(echo ${config_name} | sed 's/[a-z]/\u&/g')
        config_value=$(eval "echo \${$2_${tmp}}")

        [ -z "${config_value}" ] && {
            [ "${config_name}" = "enable" ] && {
                return
            }
            continue
        }

        ## Default should ignore
        grep -iw "^default$" <<< ${config_value} &>/dev/null && continue

        ## escape double quotes
        config_value=$(sed 's#"#\\"#g' <<< ${config_value})

        ## only the last line should not trail with ,
        cnt=$(( ${cnt} + 1 ))
        [ "${cnt}" -lt "${cnt_max}" -a "${cnt}" -ne "1" ] && echo ","

        ## echo config
        draw_indent $1 && echo -n "\"${config_name}\" : "
        case "${config_type}" in
            "bool")
                [ "${config_value}" = "y" ] && echo -n "true" || echo -n "false"
                ;;
            "int")
                echo -n "${config_value}"
                ;;
            "array")
                local tmp num
                echo -n "["
                tmp=(${config_value})
                for num in $(seq 0 $(( ${#tmp[@]} - 1 )))
                do
                    echo -n " \"${tmp[${num}]}\""
                    [ "${num}" -ne $(( ${#tmp[@]} - 1 )) ] && echo -n ","
                done
                echo -n " ]"
                ;;
            "string")
                echo -n "\"${config_value}\""
                ;;
        esac

    done
    echo
}

convert_sys() {
    # /sys
    draw_indent 1 && echo "\"sys\" : {"
    # /sys/global
    draw_indent 2 && echo "\"global\" : {"
    # /sys/global/info
    draw_indent 3 && echo "\"info\" : {"
    # /sys/global/info/collectd
    draw_indent 4 && echo "\"collectd\" : {"
    [ "${CONFIG_TINATEST_SYS_GLOBAL_INFO_COLLECTD}" = "y" ] && {
        draw_indent 5 && echo "\"enable\" : true,"
        module_convert 5 ${collectd_list[@]}
    } || {
        draw_indent 5 && echo "\"enable\" : false"
    }
    draw_indent 4 && echo "},"
    draw_indent 4 && echo "\"outlog\" : {"
    module_convert 5 ${outlog_list[@]}
    draw_indent 4 && echo "}"
    draw_indent 3 && echo "},"
    # /sys/global/limit
    draw_indent 3 && echo "\"limit\" : {"
    [ -n "${CONFIG_TINATEST_SYS_GLOBAL_LIMIT_RUN_CNT_UP_TO}" ] && {
        draw_indent 4 && echo -n "\"run_cnt_up_to\" : " && \
            echo ${CONFIG_TINATEST_SYS_GLOBAL_LIMIT_RUN_CNT_UP_TO}
    }
    draw_indent 3 && echo "}"
    draw_indent 2 && echo "},"
    # /sys/local
    draw_indent 2 && echo "\"local\" : {"
    # /sys/local/info
    draw_indent 3 && echo "\"info\" : {"
    [ "${CONFIG_TINATEST_SYS_LOCAL_INFO_DATE}" = "y" ] && {
        draw_indent 4 && echo "\"date\" : true,"
    } || {
        draw_indent 4 && echo "\"date\" : false,"
    }
    [ "${CONFIG_TINATEST_SYS_LOCAL_INFO_RESOURCE}" = "y" ] && {
        draw_indent 4 && echo "\"resource\" : true"
    } || {
        draw_indent 4 && echo "\"resource\" : false"
    }
    draw_indent 3 && echo "},"
    # /sys/local/limit
    draw_indent 3 && echo "\"limit\" : {"
    [ -n "${CONFIG_TINATEST_SYS_LOCAL_LIMIT_RUN_TIMES}" ] && {
        draw_indent 4 && echo -n "\"run_times\" : " && \
            echo -n ${CONFIG_TINATEST_SYS_LOCAL_LIMIT_RUN_TIMES}
    }
    [ -n "${CONFIG_TINATEST_SYS_LOCAL_LIMIT_RUN_TIME_SEC}" ] && {
        echo "," && draw_indent 4 && echo -n "\"run_time_sec\" : " && \
            echo -n ${CONFIG_TINATEST_SYS_LOCAL_LIMIT_RUN_TIME_SEC}
    }
    [ -n "${CONFIG_TINATEST_SYS_LOCAL_LIMIT_RUN_TIME_MIN}" ] && {
        echo "," && draw_indent 4 && echo -n "\"run_time_min\" : " && \
            echo -n ${CONFIG_TINATEST_SYS_LOCAL_LIMIT_RUN_TIME_MIN}
    }
    [ -n "${CONFIG_TINATEST_SYS_LOCAL_LIMIT_RUN_TIME_HOUR}" ] && {
        echo "," && draw_indent 4 && echo -n "\"run_time_hour\" : " && \
            echo -n ${CONFIG_TINATEST_SYS_LOCAL_LIMIT_RUN_TIME_HOUR}
    }
    echo
    draw_indent 3 && echo "}"
    draw_indent 2 && echo "}"
    draw_indent 1 && echo "},"
}

#module_convert <depth> <TREE_BASE @ tree1> <TREE_BASE @ tree2> ...
module_convert() {
    local depth bases forest trunks branches config_type flag
    local base trunk branch num tmp marks mark mark_conf list
    depth="$1" && shift
    bases=(${@%%@*})
    ## --- DEBUG: tmp = CONFIG_...INFO@collectd_df/enable
    tmp=(${@%:*})
    ## --- DEBUG: config_type = bool
    config_type=(${@##*:})
    ## --- DEBUG: forest = collectd_df/enable/$
    forest=($(echo ${tmp[@]#*@} | sed 's/ /\/$ /g' | sed 's/\(.*\)/\1\/$/'))
    ## --- DEBUG: trunks = collectd_df
    trunks=(${forest[@]%%/*})
    ## --- DEBUG: branches = enable/$
    branches=(${forest[@]#*/})
    unset tmp

    for num in `seq 0 $(( ${#forest[@]} - 1 ))`
    do
        ## --- DEBUG: trunk = collectd_df
        trunk="${trunks[${num}]}"
        grep -w "${trunk}" <<< ${marks[@]} &> /dev/null || \
            marks=(${marks[@]} ${trunk})
        tmp="$(sed 's/[^ [:alnum:]]/_/g' <<< ${trunk})"
        base="${bases[${num}]}_$(sed 's/[a-z]/\u&/g' <<< ${tmp})"
        eval "local mark_${tmp}=\"\${mark_${tmp}} ${base}@${branches[${num}]}:${config_type[${num}]}\""
    done

    num=0
    flag=0
    ## --- DEBUG: marks = collectd_df
    for mark in ${marks[@]}
    do
        mark_conf=($(eval "echo \${mark_$(sed 's/[^ [:alnum:]]/_/g' <<< ${mark})}"))
        ## --- DEBUG: mark_conf = $CONFIG..INFO_COLLECTD_CSV@enable/$:bool
        num=$(( ${num} + 1 ))
        for branch in ${mark_conf[@]}
        do
            ## --- DEBUG: branch = $CONFIG..INFO_COLLECTD_CSV@enable/$:bool
            tmp=${branch%:*}
            ## --- DEBUG: tmp = $CONFIG..INFO_COLLECTD_CSV@enable/$
            if [ "${tmp#*@}" = "$" ]; then
                module_common ${depth} ${tmp%%@*} "${mark}:${branch##*:}" ${flag} \
                    && flag=1
                [ "${num}" -eq "${#marks[@]}" -a "${flag}" -eq "1" ] && echo
            else
                ## --- DEBUG: list = $CONFIG..INFO_COLLECTD_CSV@enable:bool
                list="${list} ${tmp%/*}:${branch##*:}"
            fi
        done
        [ -n "${list}" ] && {
            [ "${flag}" -eq "1" ] && echo "," && flag=0
            draw_indent ${depth} && echo "\"${mark%%:*}\" : {"
            module_convert $(( ${depth} + 1 )) ${list}
            draw_indent ${depth} && echo -n "}"
            [ "${num}" -lt "${#marks[@]}" ] && echo "," || echo
        }
        unset list
    done
}

#module_common <depth> <variable> <config:type> <comma_flag>
module_common() {
    local config_type=${3##*:}
    local config_name=${3%:*}
    local config_value=$(eval "echo \${$2}")

    [ -n "${config_value}" ] && {
        [ "$4" -eq "1" ] && echo ","
        draw_indent $1 && echo -n "\"${config_name}\" : "
        case "${config_type}" in
            "int")
                echo -n "${config_value}"
                ;;
            "string")
                echo -n "\"${config_value}\""
                ;;
            "array")
                local tmp num
                echo -n "["
                tmp=(${config_value})
                for num in $(seq 0 $(( ${#tmp[@]} - 1 )))
                do
                    echo -n " \"${tmp[${num}]}\""
                    [ "${num}" -ne $(( ${#tmp[@]} - 1 )) ] && echo -n ","
                done
                echo -n " ]"
                ;;
            "bool")
                [ "${config_value}" = "y" ] && echo -n "true" || echo -n "false"
                ;;
        esac
        return 0
    } || return 1
}

convert_main() {
    testcase_list=(${testcase_list[@]/#\//CONFIG_TINATEST@})
    collectd_list=(${collectd_list[@]/#@/CONFIG_TINATEST_SYS_GLOBAL_INFO@})
    outlog_list=(${outlog_list[@]/#@/CONFIG_TINATEST_SYS_GLOBAL_INFO@})

    echo "\"/\" : {"
    convert_sys
    testcase_convert 1 ${testcase_list[@]}
    echo "}"
}

# check usage
[ "$#" -ne 1 ] && echo "No config file" && exit
source $1/.config

# get top path
top=`pwd`
while [ ! "${top}" = "/" ]
do
    [ -f "${top}/Config.in" -a -f "${top}/Makefile" -a -f "${top}/Depends.mk" ] && break
    top=$(dirname ${top})
done
[ "${top}" = "/" ] && echo "Get top failed"

testcase_list=($(find ${top}/testcase -name "list" 2>/dev/null | xargs cat 2>/dev/null))

convert_main
