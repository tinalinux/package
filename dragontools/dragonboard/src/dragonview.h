#ifndef __DRAGONVIEW_H__
#define __DRAGONVIEW_H__

#include <libdragon/dragonboard.h>
#include <libdragon/test_case.h>
#include <libdragon/core.h>
#include <pthread.h>

struct dragonview {
    int manual_xstart;
    int manual_ystart;
    int manual_xlength;
    int auto_xstart;
    int auto_ystart;
    int auto_xlength;
    int ylength;
};

struct dragon_data {
    int lcd_width;
    int lcd_height;
    int shm;
    int test_auto;
    int test_manual;
    int test_count;
    HWND *hwnd;
#define TEST_STATUS_INIT 0
#define TEST_STATUS_WAIT 1
#define TEST_STATUS_PASS 2
#define TEST_STATUS_FAIL 3
    int *test_status;
    char exdata[64][64];
    struct testcase_base_info *info;
    pthread_t tid_run_testcase;
    pthread_t tid_finish_testcase;
};

#undef LOG_TAG
#define LOG_TAG "DragonBoard"

// whnd ID define
#define ID_MWND   100
#define ID_CAM1   101
#define ID_CAM2   102
#define ID_WIFI   103
#define ID_HAND   104
#define ID_AUTO   105
#define ID_MIC    106
#define ID_TIMER  107
#define ID_BASE   115

void dragonview_create(HWND hWnd, struct dragon_data *data);
void CameraWin (HWND hwnd, int x_start, int y_start, int x_end, int y_end);
#endif
