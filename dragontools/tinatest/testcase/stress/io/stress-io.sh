#!/bin/sh

tt_path="/stress/io"
workers=$(mjson_fetch ${tt_path}/workers)
timeout=$(mjson_fetch ${tt_path}/timeout)
size=$(mjson_fetch ${tt_path}/size)
path=$(mjson_fetch ${tt_path}/path)

echo "workers=${workers}"
echo "timeout=${timeout}"
echo "size=${size}"
echo "path=${path}"

if [ -n ${path} ]; then
    echo "cd to ${path}"
    cd ${path}
fi

if [ -z "${workers}" ]; then
    echo "run failed: miss workers"
    exit 1
else
    args="--hdd ${workers}"
fi

if [ -n "${timeout}" ]; then
    args="${args} --timeout ${timeout}"
fi

if [ -n "${size}" ]; then
    args="${args} --hdd-bytes ${size}"
fi

stress ${args}
