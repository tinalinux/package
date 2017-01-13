#include <string.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/ctrl/edit.h>

#include "dragonview.h"

static struct dragonview view;

static void dragonview_init(struct dragon_data *data)
{
    struct dragonview *v = &view;
    v->ylength = 20;
    v->manual_xstart = 0;
    v->manual_ystart = 0;
    v->manual_xlength = data->lcd_width / 2 - 10;
    v->auto_xstart = data->lcd_width /2 + 10;
    v->auto_ystart = 0;
    v->auto_xlength = v->manual_xlength;

    data->hwnd = (HWND *)malloc(sizeof(HWND) * data->test_count);
    if (data->hwnd == NULL)
	db_error("data->hwnd malloc failed");

    data->test_status = (int *)malloc(sizeof(int) * data->test_count);
    if (data->test_status == NULL)
	db_error("data->test_status malloc failed");

}

void dragonview_create(HWND hWnd, struct dragon_data *data)
{
    struct dragonview *v = &view;
    HWND hHandTester, hAutoTester, hMicBar;
    dragonview_init(data);

    hHandTester = CreateWindow("static", "HandTester",
	    WS_CHILD | WS_VISIBLE, ID_HAND,
	    v->manual_xstart, v->manual_ystart, v->manual_xlength + v->ylength, v->ylength,
	    hWnd, 0);
    SetWindowBkColor(hHandTester, COLOR_yellow);

    hAutoTester = CreateWindow("static", "AutoTester",
	    WS_CHILD | WS_VISIBLE, ID_AUTO,
	    v->auto_xstart, v->auto_ystart, v->auto_xlength, v->ylength,
	    hWnd, 0);
    SetWindowBkColor(hAutoTester, COLOR_yellow);

    hMicBar = CreateWindowEx("progressbar", NULL,
	    WS_CHILD | WS_VISIBLE | PBS_VERTICAL,
	    WS_EX_NONE, ID_MIC,
	    v->manual_xlength, v->ylength, v->ylength, data->lcd_height - v->ylength,
	    hWnd, 0);
    SendDlgItemMessage (hWnd, ID_MIC, PBM_SETRANGE, 0, 200);
    SetWindowBkColor(hMicBar, COLOR_black);

    int count = 0;
    for (int i = 0; i < data->test_count; i++) {
	    if (!strcmp(data->info[i].name, "camera")) {
            CameraWin(hWnd, v->manual_xstart, v->manual_ystart + 21, v->manual_xlength, data->lcd_height / 2);//建立二级camera窗口
            v->manual_ystart = v->manual_ystart + data->lcd_height / 2;
        }
    }
    for (int i = 0; i < data->test_count; i++) {
	if (data->info[i].category == CATEGORY_MANUAL && strcmp(data->info[i].name, "camera")) {
		data->hwnd[i] = CreateWindow("static", data->info[i].display_name,
			WS_CHILD | WS_VISIBLE, ID_BASE + data->info[i].id,
			v->manual_xstart, v->manual_ystart + 21 + 21 * count, v->manual_xlength, v->ylength,
			hWnd, 0);
		count ++;
		data->test_status[i] = TEST_STATUS_INIT;
		SetWindowBkColor(data->hwnd[i], COLOR_darkblue);
	}
    }

    count = 0;
    for (int i = 0; i < data->test_count; i++) {
	if (data->info[i].category == CATEGORY_AUTO) {
		data->hwnd[i] = CreateWindow("static", data->info[i].display_name,
			WS_CHILD | WS_VISIBLE, ID_BASE + data->info[i].id,
			v->auto_xstart, v->auto_ystart + 21 + 21 * count, v->auto_xlength, v->ylength,
			hWnd, 0);
		count ++;
		data->test_status[i] = TEST_STATUS_INIT;
		SetWindowBkColor(data->hwnd[i], COLOR_darkblue);
	}
    }
    db_msg("dragonboard view create");
}
