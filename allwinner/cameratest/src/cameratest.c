/*
 * Copyright (C) 2016 Allwinnertech
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cameratest.h"

int hawkview_init(hawkview_handle **hv)
{
	int ret;
	hawkview_handle *hawkview;

	hawkview = malloc(sizeof(hawkview_handle));
	if (hawkview == NULL) {
		hv_err("malloc hawkview faided!\n");
		return -1;
	}
	memset(hawkview, 0, sizeof(hawkview_handle));
	*hv = hawkview;

	pthread_mutex_init(&hawkview->vid_thread.mutex, NULL);
	pthread_cond_init(&hawkview->vid_thread.cond, NULL);

	sem_init(&hawkview->vid_thread.vid_sem, 0, 0);

	pthread_mutex_init(&hawkview->capture_thread.mutex, NULL);

	return 0;
}

int hawkview_detect(hawkview_handle* hawkview, int argc, char *argv[])
{

	if (camera_detect(hawkview) < 0)
		return -1;
	/* check and explain instructions */
	if (argc != 7) {
		hv_msg("******************************************************************************************\n");
		hv_msg(" Please enter following instructions!\n");
		hv_msg(" cameratest argv[1] argv[2] argv[3] argv[4] argv[5] argv[6]\n");
		hv_msg(" argv[1]: video source width\n");
		hv_msg(" argv[2]: video source heitht\n");
		hv_msg(" argv[3]: capture photo\n");
		hv_msg(" argv[4]: the number of frame\n");
		hv_msg(" argv[5]: file save path, support:/tmp\n");
		hv_msg(" argv[6]: capture pixelformat\n");
		hv_msg("******************************************************************************************\n");
		hv_msg(" For instance : cameratest 640 480 photo 50 /tmp NV21\n");
		hv_msg("******************************************************************************************\n");
		return -1;
	}
	/* get and set print params */
	hawkview->detect.req_w		= atoi(argv[1]);
	hawkview->detect.req_h		= atoi(argv[2]);
	hawkview->detect.media_type	= argv[3];
	hawkview->detect.frame_num	= atoi(argv[4]);
	hawkview->detect.path		= argv[5];
	hawkview->detect.req_fmt	= argv[6];
	/* set_camera_format */
	set_camera_format(hawkview);
	/* detect_supported_format_frmsize */
	detect_supported_format_frmsize(hawkview);
	/* correct_require_size */
	correct_require_size(hawkview);

	return 0;
}

int start_capture_test_thread(hawkview_handle* hv)
{
	int ret;
	hv->capture_thread.tid = 0;
	ret = pthread_create(&hv->capture_thread.tid, NULL, Capture_Test_Thread, (void *)hv);
	if (ret == -1) {
		hv_err("camera: can't create capture test thread(%s)\n", strerror(errno));
		return -1;
	}
	return ret;
}

void hawkview_start(hawkview_handle *hv)
{
	if (strcmp(hv->detect.media_type,"photo") == 0) {
		init_capture(hv);
		start_capture_test_thread(hv);
		pthread_join(hv->capture_thread.tid, &hv->capture_thread.status);
		capture_quit(hv);
	}
}

int main(int argc, char *argv[])
{
	hawkview_handle *hawkview;

	/* init hawkview */
	hawkview_init(&hawkview);
	hawkview->detect.dev_id = 0;
	/* detect camera */
	if (hawkview_detect(hawkview, argc, argv) < 0)
		return -1;

	/* start hawkview */
	hawkview_start(hawkview);



	return 0;
}
