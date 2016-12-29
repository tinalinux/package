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

#include "tinacameratest.h"

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
	pthread_mutex_init(&hawkview->encoder_thread.mutex, NULL);
	pthread_mutex_init(&hawkview->muxer_thread.mutex, NULL);

	return 0;
}

int hawkview_detect(hawkview_handle* hawkview, int argc, char *argv[])
{
	camera_detect(hawkview);
	/* check and explain instructions */
	if (argc != 7) {
		hv_msg("******************************************************************************************\n");
		hv_msg(" Please enter following instructions!\n");
		hv_msg(" tinacameratest argv[1] argv[2] argv[3] argv[4] argv[5] argv[6]\n");
		hv_msg(" argv[1]: video source width\n");
		hv_msg(" argv[2]: video source heitht\n");
		hv_msg(" argv[3]: capture photo or video\n");
		hv_msg(" argv[4]: the number of frame\n");
		hv_msg(" argv[5]: file save path, support:/tmp\n");
		hv_msg(" argv[6]: capture pixelformat\n");
		hv_msg("******************************************************************************************\n");
		hv_msg(" For instance : tinacameratest 640 480 photo 50 /tmp NV21\n");
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

int start_capture_thread(hawkview_handle* hv)
{
	int ret;
	hv->capture_thread.tid = 0;
	ret = pthread_create(&hv->capture_thread.tid, NULL, Capture_Thread, (void *)hv);
	hv_dbg("capture pthread_create ret:%d\n",ret);
	if (ret == -1) {
	hv_err("camera: can't create capture thread(%s)\n", strerror(errno));
	return -1;
	}
	return ret;
}

int start_encoder_thread(hawkview_handle* hv)
{
	int ret;
	hv->encoder_thread.tid = 0;
	ret = pthread_create(&hv->encoder_thread.tid, NULL, Encoder_Thread, (void *)hv);
	hv_dbg("encoder pthread_create ret:%d\n",ret);
	if (ret == -1) {
	hv_err("camera: can't create encoder thread(%s)\n", strerror(errno));
	return -1;
	}
	return ret;
}

int start_muxer_thread(hawkview_handle* hv)
{
	int ret;
	hv->muxer_thread.tid = 0;
	ret = pthread_create(&hv->muxer_thread.tid, NULL, Muxer_Thread, (void *)hv);
	hv_dbg("muxer pthread_create ret:%d\n",ret);
	if (ret == -1) {
	hv_err("camera: can't create muxer thread(%s)\n", strerror(errno));
	return -1;
	}
	return ret;
}

int start_capture_test_thread(hawkview_handle* hv)
{
	int ret;
	hv->capture_thread.tid = 0;
	ret = pthread_create(&hv->capture_thread.tid, NULL, Capture_Test_Thread, (void *)hv);
	hv_dbg("capture pthread_create ret:%d\n",ret);
	if (ret == -1) {
	hv_err("camera: can't create capture test thread(%s)\n", strerror(errno));
	return -1;
	}
	return ret;
}

void hawkview_start(hawkview_handle *hv)
{
	if (strcmp(hv->detect.media_type,"photo") == 0) {
		hv->encoder.ENCODE_FMT = VIDEO_ENCODE_JPEG;
		/* init camera */
		init_capture(hv);
		init_decoder(&hv->decoder);
		init_encoder(hv);
		//init_muxer(hawkview);
		start_capture_thread(hv);
		start_encoder_thread(hv);

		pthread_join(hv->encoder_thread.tid, &hv->encoder_thread.status);
		pthread_join(hv->capture_thread.tid, &hv->capture_thread.status);

		capture_quit(hv);
	} else if (strcmp(hv->detect.media_type,"video") == 0) {

		/* init camera */
		init_capture(hv);
		init_decoder(&hv->decoder);
		init_encoder(hv);
		init_muxer(hv);

		start_capture_thread(hv);
		start_encoder_thread(hv);
		start_muxer_thread(hv);

		pthread_join(hv->capture_thread.tid, &hv->capture_thread.status);
		pthread_join(hv->encoder_thread.tid, &hv->encoder_thread.status);
		pthread_join(hv->muxer_thread.tid, &hv->muxer_thread.status);
		while (hv->muxer.exitFlag == 0) {
			printf("####thread_mux######wait MuxerThread finish!\n");
			usleep(1000*1000);
		}
		capture_quit(hv);
		/**/
		hv->muxer.videoEos = 0;
	} else if (strcmp(hv->detect.media_type,"test") == 0) {
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
	hawkview->encoder.flag_UsePhyBuf = 0;
	hawkview->muxer.videoNum = 0;

	if (atoi(argv[4]) < SAVE_HOUR) {
		/* detect camera */
		if (hawkview_detect(hawkview, argc, argv) < 0)
			return -1;

		/* start hawkview */
		hawkview_start(hawkview);
	} else {
		int save_argv4;
		int argv4_tmp;

		char buf[16];
		char *new = "108000";
		char *argv_tmp[7];
		memcpy(argv_tmp, argv, sizeof(argv_tmp));
		save_argv4 = atoi(argv[4]);
		argv4_tmp = save_argv4;

		while (hawkview->muxer.videoNum <= (save_argv4/SAVE_HOUR))
		{
			if ((argv4_tmp/SAVE_HOUR) > 0)
				argv_tmp[4] = new;
			else {
				snprintf(buf, sizeof(buf), "%d", argv4_tmp);
				argv_tmp[4] = buf;
			}

			argv4_tmp = argv4_tmp - atoi(argv_tmp[4]);
			hv_msg("save[%d].mp4 is doing and left frames is %d\n", hawkview->muxer.videoNum, argv4_tmp);
			/* detect camera */
			if (hawkview_detect(hawkview, argc, argv_tmp) < 0)
				return -1;
			/* init camera */
			/* init_capture(hawkview);
			//init_decoder(&hawkview->decoder);
			init_encoder(hawkview);
			init_muxer(hawkview); */

			/* start hawkview */
			hawkview_start(hawkview);

			hawkview->muxer.videoNum++;
		}
	}


	return 0;
}
