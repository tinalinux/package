/*
 * Copyright (C) 2010 The Android Open Source Project
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

#include "camera_capture.h"

int init_capture(void *arg)
{
	struct v4l2_input inp;
	struct v4l2_streamparm parms;
	struct v4l2_format fmt;
	struct v4l2_requestbuffers req;
	struct v4l2_buffer buf;
	int n_buffers;

	hawkview_handle *hv = (hawkview_handle*)arg;
	hv->capture.cap_fps = 30;
	hv->capture.buf_count = 10;

	/* set capture input */
	inp.index = 0;
	inp.type = V4L2_INPUT_TYPE_CAMERA;
	if (ioctl(hv->detect.videofd, VIDIOC_S_INPUT, &inp) == -1) {
		hv_err("VIDIOC_S_INPUT failed! s_input: %d\n",inp.index);
		return -1;
	}

	/* set capture parms */
	parms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	parms.parm.capture.timeperframe.numerator = 1;
	parms.parm.capture.timeperframe.denominator = hv->capture.cap_fps;
	if (ioctl(hv->detect.videofd, VIDIOC_S_PARM, &parms) == -1) {
		hv_err("VIDIOC_S_PARM failed!\n");
		return -1;
	}

	/* get and print capture parms */
	memset(&parms, 0, sizeof(struct v4l2_streamparm));
	parms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (ioctl(hv->detect.videofd, VIDIOC_G_PARM, &parms) == 0) {
		hv_msg(" Camera capture framerate is %u/%u\n",
				parms.parm.capture.timeperframe.denominator, \
				parms.parm.capture.timeperframe.numerator);
	} else {
		hv_err("VIDIOC_G_PARM failed!\n");
	}

	/* set image format */
	memset(&fmt, 0, sizeof(struct v4l2_format));
	fmt.type					= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width			= hv->capture.cap_w;
	fmt.fmt.pix.height			= hv->capture.cap_h;
	fmt.fmt.pix.pixelformat		= hv->capture.cap_fmt;
	fmt.fmt.pix.field			= V4L2_FIELD_NONE;
	if (ioctl(hv->detect.videofd, VIDIOC_S_FMT, &fmt) < 0) {
		hv_err("VIDIOC_S_FMT failed!\n");
		return -1;
	}

	/* reqbufs */
	memset(&req, 0, sizeof(struct v4l2_requestbuffers));
	req.count  = hv->capture.buf_count;
	req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	if (ioctl(hv->detect.videofd, VIDIOC_REQBUFS, &req) < 0) {
		hv_err("VIDIOC_REQBUFS failed\n");
		return -1;
	}

	/* mmap hv->capture.buffers */
	hv->capture.buffers = calloc(req.count, sizeof(struct buffer));
	for (n_buffers= 0; n_buffers < req.count; ++n_buffers) {
		memset(&buf, 0, sizeof(struct v4l2_buffer));
		buf.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory	= V4L2_MEMORY_MMAP;
		buf.index	= n_buffers;

		if (ioctl(hv->detect.videofd, VIDIOC_QUERYBUF, &buf) == -1) {
			hv_err("VIDIOC_QUERYBUF error\n");
			return -1;
		}

		hv->capture.buffers[n_buffers].length= buf.length;
		hv->capture.buffers[n_buffers].start = mmap(NULL , buf.length,
										PROT_READ | PROT_WRITE, \
										MAP_SHARED , hv->detect.videofd, \
										buf.m.offset);
		hv_msg("map buffer index: %d, mem: 0x%x, len: %x, offset: %x\n",
				n_buffers,(int)hv->capture.buffers[n_buffers].start,buf.length,buf.m.offset);
	}

	/* qbuf */
	for(int n_buffers = 0; n_buffers < req.count; n_buffers++) {
		memset(&buf, 0, sizeof(struct v4l2_buffer));
		buf.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory	= V4L2_MEMORY_MMAP;
		buf.index	= n_buffers;
		if (ioctl(hv->detect.videofd, VIDIOC_QBUF, &buf) == -1) {
			hv_err("VIDIOC_QBUF error\n");
			return -1;
		}
	}

	/* create fifo */
	hv->capture.fifo = FIFO_Creat(req.count*2, sizeof(struct v4l2_buffer));
}

int capture_quit(void *arg)
{
	int i;
	enum v4l2_buf_type type;
	hawkview_handle *hv = (hawkview_handle*)arg;

	hv_msg("capture quit!\n");

	/* streamoff */
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(ioctl(hv->detect.videofd, VIDIOC_STREAMOFF, &type) == -1) {
		hv_err("VIDIOC_STREAMOFF error! %s\n",strerror(errno));
		return -1;
	}

	/* munmap hv->capture.buffers */
	for (i = 0; i < hv->capture.buf_count; i++) {
		hv_dbg("ummap index: %d, mem: %x, len: %x\n",
				i,(unsigned int)hv->capture.buffers[i].start,(unsigned int)hv->capture.buffers[i].length);
		munmap(hv->capture.buffers[i].start, hv->capture.buffers[i].length);
	}

	/* free hv->capture.buffers and close hv->detect.videofd */
	free(hv->capture.buffers);
	close(hv->detect.videofd);

	return 0;
}

void *Capture_Test_Thread(void *arg)
{
	enum v4l2_buf_type type;
	struct timeval tv;
	fd_set fds;
	struct v4l2_buffer buf;
	int ret;
	int np = 0;
	long long timestamp_now, timestamp_save;
	char source_data_path[30];

	hawkview_handle *hv = (hawkview_handle*)arg;

	/* streamon */
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (ioctl(hv->detect.videofd, VIDIOC_STREAMON, &type) == -1) {
		hv_err("VIDIOC_STREAMON error! %s\n",strerror(errno));
		return (void*)0;
	}

	FD_ZERO(&fds);
	FD_SET(hv->detect.videofd, &fds);

	memset(&buf, 0, sizeof(struct v4l2_buffer));
	buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	while (np < hv->detect.frame_num) {
		hv_msg("capture num is [%d]\n",np);
		/* wait for sensor capture data */
		tv.tv_sec  = 1;
		tv.tv_usec = 0;
		ret = select(hv->detect.videofd + 1, &fds, NULL, NULL, &tv);
		if (ret == -1)
			hv_err("select error\n");
		else if (ret == 0)
			hv_err("select timeout\n");

		/* dqbuf */
		ret = ioctl(hv->detect.videofd, VIDIOC_DQBUF, &buf);
		if (ret == 0)
			hv_msg("\n*****DQBUF[%d] FINISH*****\n",buf.index);
		else
			hv_err("****DQBUF FAIL*****\n");

		timestamp_now = secs_to_msecs((long long)(buf.timestamp.tv_sec), (long long)(buf.timestamp.tv_usec));
		if (np == 0) {
			timestamp_save = timestamp_now;
		}
		hv_msg("the interval of two frames is %lld ms\n", timestamp_now - timestamp_save);
		timestamp_save = timestamp_now;

		sprintf(source_data_path, "/tmp/%s%d%s", "source_data", np+1, ".yuv");
        save_frame_to_file(source_data_path, hv->capture.buffers[buf.index].start, buf.bytesused);

		/* qbuf */
		if (ioctl(hv->detect.videofd, VIDIOC_QBUF, &buf) == 0)
			hv_msg("************QBUF[%d] FINISH**************\n",buf.index);

		np++;
	}
	hv_msg("capture thread finish !\n");
	//capture_quit(hv);
	return (void*)0;
}
