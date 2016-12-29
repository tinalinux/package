#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/ctrl/edit.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "videodev.h"
#include "dragonview.h"

using namespace std;

struct dragon_data data;

void *run_testcases(void *data)
{
    struct dragon_data *dt = (struct dragon_data *)data;
    struct testcase_base_info *info = dt->info;
    for (int i = 0; i < dt->test_count; i++) {
        if (info[i].binary[0] == '\0') {
            db_msg("can not exec testcase: %s", info[i].display_name);
            continue;
        }

        run_one_testcase(DRAGONBOARD_VERSION, NULL,  dt->shm, &info[i]);
        dt->test_status[i] = TEST_STATUS_WAIT;
    }

    wait_for_test_completion();
}

void *finish_testcases(void *data)
{
    struct dragon_data *dt = (struct dragon_data *)data;
    struct testcase_base_info *info = dt->info;
    FILE *from_child;
    char *exdata;
    int exdata_len;
    char buffer[128];
    char *test_case_id_s;
    int test_case_id;
    char *result_s;
    int result;
    /* listening to child process */
    db_msg("listening to child process, starting...");

    while (1) {
        from_child = fopen(CMD_PIPE_NAME, "r");
        if (from_child == NULL){
            db_error("finish cmd pipe open failed");
        }
        if (fgets(buffer, sizeof(buffer), from_child) == NULL) {
            fclose(from_child);
            continue;
        }

        db_dump("command from child: %s", buffer);

        /* test completion */
        test_case_id_s = strtok(buffer, " \n");
        db_dump("test case id #%s", test_case_id_s);
        if (strcmp(buffer, TEST_COMPLETION) == 0) {
            fclose(from_child);
            break;
        }

        if (test_case_id_s == NULL)
            continue;
        test_case_id = atoi(test_case_id_s);

        result_s = strtok(NULL, " \n");
        db_dump("result: %s", result_s);
        if (result_s == NULL) {
            fclose(from_child);
            continue;
        }
        result = atoi(result_s);

        exdata = strtok(NULL, "\n");

        if (exdata != NULL) {
            db_msg("exdata: %s,%s", exdata);
            strcpy(dt->exdata[test_case_id], exdata);
        }
        db_dump("%s TEST %s", info[test_case_id].name,
                (result == 1) ? "OK" : "Fail");
        dt->test_status[test_case_id] = (result == 1 ? TEST_STATUS_PASS : TEST_STATUS_FAIL);
        fclose(from_child);
    }

}

int DragonWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    char buff[20];
    static int keyVal = 0;
    static clock_t timestamp1, timestamp2;
    struct dragon_data *dt = &data;
    struct testcase_base_info *info = dt->info;

    switch (message) {
        case MSG_CREATE:
            dragonview_create(hWnd, &data);
            db_debug("create testcases run thread");
            if (pthread_create(&dt->tid_run_testcase, NULL, run_testcases, (void *)dt) < 0){
                db_error("run testcase thread create failed");
            }
            db_debug("create testcases finish thread");
            if (pthread_create(&dt->tid_finish_testcase,NULL, finish_testcases, (void *)dt) < 0){
                db_error("finish testcase thread create failed");
            }
            SetTimer(hWnd, ID_TIMER, 50);       // set timer 0.2s(20), 1s(100)
            break;

        case MSG_TIMER:
            for (int i = 0; i < dt->test_count; i++) {
                char str[128] = {0};
                if (dt->test_status[i] == TEST_STATUS_WAIT) {
                    sprintf(str, "%s%s", data.info[i].display_name, ": testing...");
                    SetDlgItemText(hWnd, data.info[i].id + ID_BASE, str);
                } else if (dt->test_status[i] == TEST_STATUS_PASS) {
                    sprintf(str, "%s%s%s", data.info[i].display_name, ": PASS    ", data.exdata[i]);
                    SetDlgItemText(hWnd, data.info[i].id + ID_BASE, str);
                    SetWindowBkColor(data.hwnd[i], COLOR_green);
                } else if (dt->test_status[i] == TEST_STATUS_FAIL) {
                    sprintf(str, "%s%s%s", data.info[i].display_name, ": FAIL    ", data.exdata[i]);
                    SetDlgItemText(hWnd, data.info[i].id + ID_BASE, str);
                    SetWindowBkColor(data.hwnd[i], COLOR_red);
                }
            }
            SendDlgItemMessage (hWnd, ID_MIC, PBM_SETPOS, 0, 0L);
            break;
        case MSG_KEYDOWN:
            keyVal = LOWORD(wParam);
            break;
        case MSG_KEYUP:
            keyVal = LOWORD(wParam);
            break;
        case MSG_CLOSE:
            KillTimer(hWnd, ID_TIMER);
            DestroyAllControls(hWnd);
            DestroyMainWindow(hWnd);
            PostQuitMessage(hWnd);
            return 0;
    }

    return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

int MiniGUIMain(int argc, const char **argv)
{
    char exdate[64];
    double length;
    int retVal;
    HWND hMainWnd;
    MSG Msg;

    data.shm = init_script("/boot-res/test_config.fex");
    data.info = init_testcases();
    data.test_manual = get_manual_testcases_number();
    data.test_auto = get_auto_testcases_number();
    data.test_count = get_testcases_number();

    db_debug("manual:%d, auto:%d, count%d", data.test_manual, data.test_auto, data.test_count);

    // save LCD's width and height
    data.lcd_width = GetGDCapability(HDC_SCREEN, GDCAP_MAXX) + 1;
    data.lcd_height = GetGDCapability(HDC_SCREEN, GDCAP_MAXY) + 1;

    db_debug("LCD width & height: (%d, %d)", data.lcd_width, data.lcd_height);

    MAINWINCREATE CreateInfo = {
        WS_VISIBLE,
        WS_EX_NOCLOSEBOX,
        DRAGONBOARD_COPYRIGHT,
        0,
        GetSystemCursor(0),
        0,
        HWND_DESKTOP,
        DragonWinProc,
        0, 0, data.lcd_width, data.lcd_height,
        COLOR_black,
        0, 0};

    hMainWnd = CreateMainWindow(&CreateInfo);
    HDC hdc = GetDC(hMainWnd);
    SetWindowBkColor(hMainWnd, RGBA2Pixel(hdc, 0x00, 0x00, 0x00, 0x00));
    if (hMainWnd == HWND_INVALID) {
        db_msg("CreateMainWindow error");
        return -1;
    }

    while (GetMessage(&Msg, hMainWnd)) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    MainWindowCleanup(hMainWnd);

    return 0;
}

#ifndef _MGRM_PROCESSES
#include <minigui/dti.c>
#endif
