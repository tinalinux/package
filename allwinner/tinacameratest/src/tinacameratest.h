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
#include "vdecoder.h"
#include "memoryAdapter.h"
#include <pthread.h>
#include <semaphore.h>
#include <ctype.h>
#include <errno.h>
#include <cdx_config.h>
#include <log.h>
#include <CdxQueue.h>
#include <AwPool.h>
#include <CdxBinary.h>
#include <CdxMuxer.h>
#include "awencoder.h"
#include "RecorderWriter.h"
#include "fifo.h"
#include "common.h"
#include "camera_detect.h"
#include "camera_capture.h"
#include "camera_decoder.h"
#include "camera_encoder.h"
#include "camera_mux.h"

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

typedef struct EncoderContext
{
	AwEncoder* mAwEncoder;

	VideoEncodeConfig videoConfig;
	AudioEncodeConfig audioConfig;

	unsigned char* extractDataBuff;
	unsigned int extractDataLength;

	AwPoolT *pool;
	CdxQueueT *dataQueue;

	int exitFlag;

	int saveframe_Flag;

	char *save_path;

	int frame_count;
}EncoderContext;

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

typedef struct _decoder
{
	int flag_decoder ;

	int SOURCE_FMT;

	int DECODE_FMT;

	VideoDecoder *pVideo;

	int fail_decoder;

	unsigned int frame_size;

	char *Vir_Y_Addr;
	char *Vir_U_Addr;

	size_addr Phy_Y_Addr;
	size_addr Phy_C_Addr;

	int frame_count;

}decoder_handle;

typedef struct _encoder
{

	int ENCODE_FMT;

	int SOURCE_FMT;

	int flag_UsePhyBuf;

	long long timestamp_now;
	long long timestamp_start;
	long long timestamp_interval;

	EncoderContext encoder_context;

	EncDataCallBackOps mEncDataCallBackOps;

}encoder_handle;

typedef struct _muxer
{
	int videoEos;
	int videoNum;

	int muxType;

	char pUrl[1024];

	CdxWriterT *pStream;

	CdxMuxerT *pMuxer;

	int exitFlag ;
}muxer_handle;

typedef struct _hawkview
{
	thread_handle	vid_thread;
	thread_handle	capture_thread;
	thread_handle	encoder_thread;
	thread_handle	muxer_thread;

	detect_handle	detect;

	capture_handle	capture;

	decoder_handle	decoder;

	encoder_handle	encoder;

	muxer_handle	muxer;
}hawkview_handle;
#endif
