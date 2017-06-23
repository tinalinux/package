#!/bin/sh

testcase_path="/stress/rw/rw-loop"
begin_size=`mjson_fetch ${testcase_path}/begin_size`
end_size=`mjson_fetch ${testcase_path}/end_size`
check_dir=`mjson_fetch ${testcase_path}/check_directory`

[ -n "${begin_size}" ] && args="${args} -b ${begin_size}"
[ -n "${end_size}" ] && args="${args} -e ${end_size}"
[ -n "${check_dir}" ] && args="${args} -d ${check_dir}"


echo -e "\n\trun command: rwcheck -l ${args}"
rwcheck -l ${args} &
sleep 3
rm /etc/init.d/rwcheck /etc/rc.d/S99rwcheck
wait
