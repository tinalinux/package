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

#ifndef __CAMERATEST_H__
#define __CAMERATEST_H__

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include "videodev.h"
#include <pthread.h>
#include <semaphore.h>
#include <ctype.h>
#include <errno.h>
//#include <log.h>
#include "fifo.h"
#include "common.h"
#include "camera_detect.h"
#include "camera_capture.h"

#define HAWKVIEW_DBG 1

#define SAVE_SOURCE_DATA 0

#if HAWKVIEW_DBG
#define hv_warn(x,arg...) printf("[hawkview_warn]"x,##arg)
#define hv_dbg(x,arg...) printf("[hawkview_dbg]"x,##arg)
#else
#define hv_warn(x,arg...)
#define hv_dbg(x,arg...)
#endif
#define hv_err(x,arg...) printf("[hawkview_err]xxxx"x,##arg)
#define hv_msg(x,arg...) printf("[hawkview_msg]----"x,##arg)

#ifdef ANDROID_ENV
#include "log.h"
#undef hv_warn
#undef hv_dbg
#undef hv_err
#undef hv_msg
#define hv_err log_err
#define hv_dbg log_dbg
#define hv_msg log_dbg
#define hv_warn log_dbg

#endif

#define ALIGN_4K(x) (((x) + (4095)) & ~(4095))
#define ALIGN_32B(x) (((x) + (31)) & ~(31))
#define ALIGN_16B(x) (((x) + (15)) & ~(15))

#define csi_camera 0
#define usb_camera 1
#define AUDIO_INPUT (0)
#define VIDEO_INPUT (1)

#define SAVE_HOUR 108000
#define SAVE_MIN  1800


///////////////////////////////////////////////////////////////////////////////
//prepare
struct buffer
{
	void * start;
	size_t length;
	unsigned int phy_addr;
};

typedef struct _thread
{
	pthread_mutex_t		mutex;
	pthread_cond_t		cond;
	pthread_t			tid;
	sem_t				vid_sem;
	void*				status;
}thread_handle;

typedef struct _detect
{
	int dev_id;			// v4l2 subdevices id

	int videofd;		// open videox

	int camera_type;	// csi camera or usb camera

	unsigned int min_width;
	unsigned int min_height;

	unsigned int max_width;
	unsigned int max_height;

	int req_w;			//request target capture size
	int req_h;
	char *media_type;	//request photo or video
	int frame_num;		//request frame numbers
	char *path;

	char *req_fmt;

}detect_handle;

typedef struct _capture
{
	int sensor_type;	// yuv or raw sensor

	int cap_w;			//supported capture size
	int cap_h;

	int cap_fps;		//capture framerate
	int cap_fmt;		//capture format

	int buf_count;		//request buffers count

	struct buffer *buffers;

	P_FIFO_T fifo;
	/* capture_status status;
	capture_status save_status; */
}capture_handle;

typedef struct _hawkview
{
	thread_handle	vid_thread;
	thread_handle	capture_thread;

	detect_handle	detect;

	capture_handle	capture;

}hawkview_handle;
#endif
