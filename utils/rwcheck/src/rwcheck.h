#ifndef __RWCHECK_H
#define __RWCHECK_H

#include <pthread.h>
#include <stdio.h>
//notice that the basic unit is KB
#define KB ( 1 )
#define MB ( KB * 1024 )
#define GB ( MB * 1024 )

//to specify the rate of growth check file size
#define FILE_SIZE_UP_RATE 4
#define SIZE_PER_READ_MB 8

#define boot_script (char *)"/etc/init.d/rwcheck"
#define boot_script_link (char *)"/etc/rc.d/S100rwcheck"
#define LOOP_MODE_DEFAULT_OUTPUT_FILE "/mnt/UDISK/rwcheck.result"
#define LOOP_MODE_DEFAULT_CHECK_PATH "/mnt/UDISK"  //no '/' at end
#define INITIAL_FILE_NAME "rwcheck.tmp"
#define CRC_RANDOM_DATA_MAX_SIZE_BYTES (unsigned long)( 512 * MB * 1204 )

enum check_mode {
    nornal_mode = 0,
    auto_mode,
    loop_mode,
};

struct size_unit {
    unsigned long filesize_kb;
    unsigned long size;  //size match units, for the most humanized view, in KB|MB or GB depend on units
    char units[5];    //KB,MB or GB
};

struct once {
    struct size_unit filesize;
    pthread_mutex_t lock_refresh_status;

    struct obj *head;
    struct obj *middle;
    struct obj *tail;
    struct obj *whole;
};

struct docheck {
    struct once *checking;

    enum check_mode mode;
    struct size_unit begin_file_size;
    struct size_unit end_file_size;
    char *output_file;
    char *check_file_path;
    unsigned int check_times;
};

#endif
