#!/bin/bash

## cpath: config path, such as /stress/reboot
## pconf: path of private.conf, such as ../testcase/stress/io/private.conf
## top: the root path of tinatest, such as tina/pacakge/dragontools/tinatest

show_help() {
    echo -e "使用说明:"
    echo -e "\033[33m    del_testcase.sh <配置文件1> [配置文件2] ...\033[0m\n"
    echo -e "配置文件:"
    echo -e "    记录新用例的路径以及默认配置值,一行一条键值对,格式为:"
    echo -e "\033[33m              <配置项>[:类型] = <配置值>\033[0m"
    echo -e "    \033[33m[配置项]\033[0m: 包含PATH/ENABLE/INFO/LIMIT/DEPENDS和测试用例的所有配置项(例如:command,run_times,run_alone等)"
    echo -e "    其中:"
    echo -e "        PATH: 测试用例在配置树中的绝对路径(字符串)"
    echo -e "        ENABLE: 默认是否使能此用例(bool)"
    echo -e "        INFO: 默认是否使能所有的 局部信息 配置项(bool)"
    echo -e "        LIMIT: 默认是否使能所有的 局部限制 配置项(bool)"
    echo -e "        DEPENDS: 测试用例依赖的第三方应用包(string)"
    echo -e "                 格式: +TINATEST_<测试用例路径>_ENABLE:<依赖的软件包>"
    echo -e "                 例如 /stress/rw/rw-auto 依赖 rwcheck 软件包, 则 DEPENDS=\"+TINATEST_STRESS_RW_RW_AUTO_ENABLE:rwcheck\""
    echo -e "        \033[32m[大写字母为特定配置项][无指定类型的小写字母为用例属性项][指定类型的小写字母为私有配置项]\033[0m"
    echo -e "    \033[33m[类型]\033[0m: \033[31m(私有配置项才需要)\033[0m为mjson支持的数据类型,包括:int/double/true/false/string/array"
    echo -e "    \033[33m[配置值]\033[0m: 支持字符串/字符串数组/整数/浮点数/bool(见示例)"
    echo -e "    其中:"
    echo -e "        1) \033[31m字符串必须用双引号括起来\033[0m"
    echo -e "        2) 字符串数组以字符串形式表示,字符串之间以空格间隔"
    echo -e "        4) 字符串内若双引号\\转义字符等,需要有双重转义, 例如: command = \"echo \\\\\\\\\\\\\"test\\\\\\\\\\\\\"\" 表示echo \"test\""
    echo -e "        4) 每一行开头不能有空格/Tab等"
    echo -e "\n示例如下:"
    echo -e "    |PATH = /demo/demo-c"
    echo -e "    |ENABLE = false"
    echo -e "    |INFO = true"
    echo -e "    |command = \"demo-c\""
    echo -e "    |run_times = 10"
    echo -e "    |run_alone = false"
    echo -e "    |workers:int = 2"
    echo -e "    |words:string = \"test\""
    echo -e "    |right:bool = true"
    echo -e "    |str_array:array = \"one two three\""
}

#get_value <config-name>
# From 配置文件
get_value() {
    echo ${oPWD}/${pconf} | xargs grep "^$1 *=.*$" | cut -d '=' -f 2- | sed -r 's/^ +//g'
}

del_list() {
    local tmp=$(sed 's#/#\\/#g' <<< ${cpath})
    sed -i "/^${tmp}$/d" ${top}/testcase/${class}/list
}

#del_config_in
del_config_in() {
    local tmp

    sed -i "/source ${confile}/d" ./Config.in

    while [ ! "`pwd`" = "${top}/config/${class}" ]
    do
        [ -f "./Config.in" ] || return
        grep "source" ./Config.in &>/dev/null \
            && break \
            || {
                tmp=${PWD##*/}
                echo tmp is $tmp
                cd ..
                rm -rf ${tmp}
            }
    done

    [ "`pwd`" = "${top}/config/${class}" -a -n "${tmp}" ] && sed -i "/source ${tmp}\/Config.in/d" ./Config.in
}

del_config() {
    testcase=$(awk -F '/' '{print $NF}' <<< ${cpath})
    cpath_rest="$(cut -d '/' -f 3- <<< ${cpath})" && cpath_rest="${cpath_rest%/*}"
    [ "${cpath_rest}" = "${testcase}" ] && unset cpath_rest
    confile="Config-$(sed 's#/#-#g' <<< ${testcase}).in"
    condir="${top}/config/${class}$([ -n "${cpath_rest}" ] && echo /${cpath_rest})"
    conpath="${condir}/${confile}"

    # delete Config-<testcase>.in
    [ -f "${conpath}" ] && rm ${conpath}

    # Config.in
    [ -d "${top}/config/${class}/${cpath_rest}" ] \
        && cd ${top}/config/${class}/${cpath_rest} \
        && del_config_in
}

del_depend() {
    local depends=$(get_value "DEPENDS")
    [ -z "${depends}" ] && return

    local one
    for one in ${depends}
    do
        one=$(awk -F \" '{print $2}' <<< ${one})
        sed -i "/    ${one} /d" ${top}/Depends.mk
    done
}

del_main() {
    oPWD=`pwd`
    for pconf in $@
    do
        cd ${oPWD}
        [ -f "${pconf}" ] || {
            echo "Not find \"${pconf}\""
            continue
        }

        cpath=$(get_value "PATH")
        [ -z "${cpath}" ] && echo "Not get path in ${pconf}" && continue
        cpath=$(awk -F \" '{print $2}' <<< ${cpath})
        command=$(get_value "command")
        [ -z "${command}" ] && echo "Not get command in ${pconf}" && continue
        class=$(awk -F '/' '{print $2}' <<< ${cpath})

        del_list
        del_config
        del_depend
    done
}

# check usage
[ "$#" -lt 1 ] && show_help

# get top path
top=`pwd`
while [ ! "${top}" = "/" ]
do
    [ -f "${top}/Config.in" -a -f "${top}/Makefile" -a -f "${top}/Depends.mk" ] && break
    top=$(dirname ${top})
done
[ "${top}" = "/" ] && echo "Get top failed" && exit

del_main $@
touch ${top}/Makefile
echo "All Done!"
