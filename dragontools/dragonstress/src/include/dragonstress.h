#ifndef __DRAGONSTRESS_H__
#define __DRAGONSTRESS_H__

#include <libdragon/dragonboard.h>
#include <libdragon/test_case.h>
#include <libdragon/core.h>
#include <libdragon/script.h>
#include <pthread.h>

#include <tina_log.h>
#define DRAGONSTRESS_VERSION			0x161020
#define DRAGONSTRESS_COPYRIGHT			"DragonStress V1.0 Copyright (C) 2016 Allwinner"

#undef LOG_TAG
#define LOG_TAG "dragonstress"

#define CMD_PIPE_NAME                   "/tmp/cmd_pipe"
#define VOLUME_PIPE_NAME                "/tmp/volume_pipe"
#define CAMERA_PIPE_NAME                "/tmp/camera_pipe"
#define MIC_PIPE_NAME                   "/tmp/mic_pipe"
#define WIFI_PIPE_NAME                  "/tmp/wifi_pipe"
#define TEST_COMPLETION                 "done"

#define ds_error(fmt, arg...)   TLOGE(fmt, ##arg)
#define ds_warn(fmt, arg...)    TLOGW(fmt, ##arg)
#define ds_msg(fmt, arg...)     TLOGI(fmt, ##arg)
//#define ds_debug(fmt, arg...)   TLOGD(fmt, ##arg)
#define ds_debug(fmt, arg...)   printf(fmt, ##arg)
#define ds_dump(fmt, arg...)    TLOGI(fmt, ##arg)

enum {
	TESTCASES_INIT = 0,
	TESTCASES_RUNNING,
	TESTCASES_FINISH,
	TESTCASES_MAX,
};

struct dragon_stress_data {
	int shm;
	struct testcase_base_info *info;
	int test_auto;
	int test_manual;
	int test_count;
#define TEST_STATUS_INIT 0
#define TEST_STATUS_WAIT 1
#define TEST_STATUS_PASS 2
#define TEST_STATUS_FAIL 3
	int *test_status;
	int *fail_module;
	pthread_t tid_running;
	int run_state;
};

#endif
