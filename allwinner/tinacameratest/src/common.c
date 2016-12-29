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

#include "common.h"


void NV12ToYUV420P(void *NV12, int width, int height) {
	int len = width * height * 3 >>1;
	char *save_NV12 = (char*)malloc(len);
	memcpy(save_NV12, NV12, len);
	char *src_u	= (char *)save_NV12 + width * height;
	char *src_v	= (char *)save_NV12 + width * height+1;
	char *dst_uv= (char *)NV12+ width * height;
	//memcpy(nv21, yv12, width * height);//copy Y
	for(int i = 0; i < width * height/4; i ++) {
		*(dst_uv++) = *(src_u+2*i);
	}
	for(int j = 0; j < width * height/4; j ++) {
		*(dst_uv++) = *(src_v+2*j);
	}
	free(save_NV12);
}

void YUYVToUYVY(void *YUYV, int len) {
	char *save_YUYV = (char *)malloc(len);
	char *dst_YUYV	= (char *)YUYV;
	memcpy(save_YUYV, YUYV, len);
	for(int i = 1; i < len; i = i + 2 ) {
		*(dst_YUYV++) = *(save_YUYV + i);
		*(dst_YUYV++) = *(save_YUYV + i -1);
	}
	free(save_YUYV);
}

int save_frame_to_file(void *str,void *start,int length) {
	FILE *fp = NULL;
	fp = fopen(str, "ab+");		//save more frames
	if(!fp) {
		//hv_err("Open file error\n");
		return -1;
	}
	if(fwrite(start, length, 1, fp)) {
		printf("write finish!\n");
		fclose(fp);
		return 0;
	}
	else {
		//hv_err("Write file fail (%s)\n",strerror(errno));
		fclose(fp);
		return -1;
	}
	return 0;
}

long long secs_to_msecs(long long secs,long long usecs) {
	long long msecs;
	msecs = usecs/1000 + secs* 1000;
	return msecs;
}
