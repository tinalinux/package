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

#include "camera_detect.h"

int openDevice(void *arg)
{
	char dev_name[32];

	hawkview_handle *hv = (hawkview_handle*)arg;

	snprintf(dev_name, sizeof(dev_name), "/dev/video%d", hv->detect.dev_id);
	hv_msg("******************************************************************************************\n");
	hv_msg(" Open %s\n", dev_name);

	if ((hv->detect.videofd = open(dev_name, O_RDWR,0)) < 0) {
		hv_err(" can't open %s(%s)\n", dev_name, strerror(errno));
		return -1;
	}

	//fcntl(videofh, F_SETFD, FD_CLOEXEC);
	return 0;
}

char *get_format_ShortName(char *description)
{
	if (strcmp(description, "planar YUV 420") == 0) {
		return "YU12";
	} else if (strcmp(description, "planar YVU 420") == 0) {
		return "YV12";
	} else if (strcmp(description, "planar YUV 420 UV combined") == 0) {
		return "NV12";
	} else if (strcmp(description, "planar YUV 420 VU combined") == 0) {
		return "NV21";
	} else if (strcmp(description, "planar YUV 422 UV combined") == 0) {
		return "NV16";
	} else if (strcmp(description, "planar YUV 422 VU combined") == 0) {
		return "NV61";
	} else {
		return NULL;
	}
}

int detect_csi_camera_supported_format(void *detect)
{
	struct v4l2_fmtdesc fmtdesc;
	int index = 1;
	int fd;

	detect_handle* det = (detect_handle*)detect;

	fd = det->videofd;
	fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	hv_msg("******************************************************************************************\n");
	hv_msg(" This Camera support following capture format:\n");
	for (int i = 0; i < 12; i++) {
		fmtdesc.index = i;
		if (ioctl (fd, VIDIOC_ENUM_FMT, &fmtdesc) == -1) {
			break;
		}

		if (get_format_ShortName(fmtdesc.description)) {
			hv_msg(" capture format : index = %d, name = %s, description = %s\n",
					index++, get_format_ShortName(fmtdesc.description), fmtdesc.description);
		}
	}
	return 0;
}

int detect_usb_camera_supported_format(void *detect)
{
	struct v4l2_fmtdesc fmtdesc;
	int index = 1;
	int fd;

	detect_handle* det = (detect_handle*)detect;

	fd = det->videofd;
	fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	while(fmtdesc.type < 11){
	hv_msg("******************************************************************************************\n");
	hv_msg(" This Camera support following capture format:\n");
	for (int i = 0; i < 12; i++) {
		fmtdesc.index = i;
		if (ioctl (fd, VIDIOC_ENUM_FMT, &fmtdesc) == -1) {
			break;
		}
		hv_msg(" capture format : index = %d, name = %s\n",
				index++, fmtdesc.description);
	}
	fmtdesc.type += 1;
	}
	return 0;
}

int detect_camera_type(void *arg)
{
	struct v4l2_capability cap;
	hawkview_handle *hv = (hawkview_handle*)arg;

	/* querycap */
	if (ioctl(hv->detect.videofd, VIDIOC_QUERYCAP, &cap) < 0) {
		hv_err("VIDIOC_QUERYCAP failed!\n");
		return -1;
	}
	hv_msg("******************************************************************************************\n");
	hv_msg(" The driver name is [%s], the card name is [%s]\n", cap.driver, cap.card);


	if (strcmp(cap.driver , "sunxi-vfe") == 0) {
		hv->detect.camera_type = csi_camera;
		detect_csi_camera_supported_format(&hv->detect);
		return 0;
	} else if (strcmp(cap.driver, "uvcvideo") == 0) {
		hv->detect.camera_type = usb_camera;
		detect_usb_camera_supported_format(&hv->detect);
		return 0;
	} else {
		hv_err("can not detect the camera type!\n");
		return -1;
	}
}

int detect_supported_format_frmsize(void *arg)
{
	struct v4l2_frmsizeenum frmsize;
	int ret;
	int fd;

	hawkview_handle *hv = (hawkview_handle*)arg;

	fd = hv->detect.videofd;
	memset(&frmsize, 0, sizeof(frmsize));
	frmsize.index = 0;
	frmsize.pixel_format = hv->capture.cap_fmt;

	hv_msg("******************************************************************************************\n");
	hv_msg(" Supported following framesize with choosed format :\n");
	//ret = ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize);
	while ((ret = ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize)) == 0) {
		if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
			hv_msg(" discrete: width = %u, height = %u\n",
					frmsize.discrete.width, frmsize.discrete.height);
		} else if (frmsize.type == V4L2_FRMSIZE_TYPE_CONTINUOUS) {
			hv_msg(" continuous: min [ width = %u, height = %u ] .. "
					"max [ width = %u, height = %u ]\n",
					frmsize.stepwise.min_width, frmsize.stepwise.min_height,
					frmsize.stepwise.max_width, frmsize.stepwise.max_height);
			break;
		} else if (frmsize.type == V4L2_FRMSIZE_TYPE_STEPWISE) {
			hv_msg(" stepwise: min [ width = %u, height = %u ] .. "
					"max [ width = %u, height = %u ] / "
					"stepsize [ width = %u, height = %u ]\n",
					frmsize.stepwise.min_width, frmsize.stepwise.min_height,
					frmsize.stepwise.max_width, frmsize.stepwise.max_height,
					frmsize.stepwise.step_width, frmsize.stepwise.step_height);
			break;
		}
		frmsize.index++;
	}
	hv_msg("******************************************************************************************\n");

	if (ret != 0 && errno != EINVAL) {
		hv_msg("ERROR enumerating sizes: %d\n", errno);
		return -1;
	}

	return 0;
}

int set_camera_format(void *arg)
{
	hawkview_handle *hv = (hawkview_handle*)arg;

/* YUV 4:2:0 */
	if (strcmp(hv->detect.req_fmt, "YU12") == 0) {
		hv->capture.cap_fmt = V4L2_PIX_FMT_YUV420;
		return 0;
	} else if (strcmp(hv->detect.req_fmt, "YV12") == 0) {
		hv->capture.cap_fmt = V4L2_PIX_FMT_YVU420;
		return 0;
	} else if (strcmp(hv->detect.req_fmt, "NV12") == 0) {
		hv->capture.cap_fmt = V4L2_PIX_FMT_NV12;
		return 0;
	} else if (strcmp(hv->detect.req_fmt, "NV21") == 0) {
		hv->capture.cap_fmt = V4L2_PIX_FMT_NV21;
		return 0;
	}
/* YUV 4:2:2 */
	else if (strcmp(hv->detect.req_fmt, "YUV422P") == 0) {
		hv->capture.cap_fmt = V4L2_PIX_FMT_YUV422P;
		return 0;
	} /* else if (strcmp(hv->detect.req_fmt, "YVU422P") == 0) {
		hv->capture.cap_fmt = V4L2_PIX_FMT_YVU422P;
		hv->encoder.SOURCE_FMT = VENC_PIXEL_YVU422P;
		return 0;
		}*/
	else if (strcmp(hv->detect.req_fmt, "NV16") == 0) {
		hv->capture.cap_fmt = V4L2_PIX_FMT_NV16;
		return 0;
	} else if (strcmp(hv->detect.req_fmt, "NV61") == 0) {
		hv->capture.cap_fmt = V4L2_PIX_FMT_NV61;
		return 0;
	}
/* YUYV YVYU UYVY VYUY */
	else if (strcmp(hv->detect.req_fmt, "YUYV") == 0) {
		hv->capture.cap_fmt = V4L2_PIX_FMT_YUYV;
		return 0;
	} else if (strcmp(hv->detect.req_fmt, "YVYU") == 0) {
		hv->capture.cap_fmt = V4L2_PIX_FMT_YVYU;
		return 0;
	} else if (strcmp(hv->detect.req_fmt, "UYVY") == 0) {
		hv->capture.cap_fmt = V4L2_PIX_FMT_UYVY;
		return 0;
	} else if (strcmp(hv->detect.req_fmt, "VYUY") == 0) {
		hv->capture.cap_fmt = V4L2_PIX_FMT_VYUY;
		return 0;
	} else {
		hv_err("Can not find camera format!\n");
		return -1;
	}
}

int correct_require_size(void *arg)
{
	struct v4l2_format fmt;
	int fd;

	hawkview_handle *hv = (hawkview_handle*)arg;

	//fd = hv->detect.videofd;
	memset(&fmt, 0, sizeof(fmt));
	fmt.type					= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width			= hv->detect.req_w;
	fmt.fmt.pix.height			= hv->detect.req_h;
	fmt.fmt.pix.pixelformat		= hv->capture.cap_fmt;
	fmt.fmt.pix.field			= V4L2_FIELD_NONE;

	if (ioctl(hv->detect.videofd, VIDIOC_TRY_FMT, &fmt) < 0) {
		hv_err("VIDIOC_TRY_FMT Failed: %s", strerror(errno));
		return -1;
	}

	/* get correct size */
	hv->capture.cap_w = fmt.fmt.pix.width;
	hv->capture.cap_h = fmt.fmt.pix.height;

	hv_msg(" After correct Size: w: %d, h: %d\n", \
			hv->capture.cap_w, hv->capture.cap_h = fmt.fmt.pix.height);
	return 0;
}


int camera_detect(void *arg)
{
	hawkview_handle *hv = (hawkview_handle*)arg;
	int ret;

	if (ret = openDevice(hv) < 0)
		return ret;
	if (ret = detect_camera_type(hv) < 0)
		return ret;
	return 0;
}
