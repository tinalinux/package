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

//#include "tinacameratest.h"
#include "camera_encoder.h"

/* EncoderContext hv->encoder.encoder_context; */

#ifdef DISPLAY_SOURCE_DATA
void disp_mgr_init()
{
    int fdDisp;
    disp_init();
    fdDisp = disp_get_cur_fd();
    if(fdDisp < 1) {
        printf("open disp failed");
        return;
    }
    disp_lcd_on(fdDisp);
    disp_open_backlight();
}

void release_de(int signb)
{
        disp_uninit();
        exit(1);
}
#endif

int onVideoDataEnc(void *app, CdxMuxerPacketT *buff)
{
	char image_name_path[30];
	FILE* fpSaveImage = NULL;
	CdxMuxerPacketT *packet = NULL;

	EncoderContext *encoder_context = (EncoderContext*)app;

	if (encoder_context->saveframe_Flag == 1) {
		sprintf(image_name_path, "%s/%s%d%s", encoder_context->save_path, "IMAGE", encoder_context->frame_count, ".jpeg");
		fpSaveImage= fopen(image_name_path, "wb+");
		if (fpSaveImage == NULL) {
			hv_err("open file SaveImage failed, errno(%d)\n", errno);
			return -1;
		}
		fwrite(buff->buf, 1, buff->buflen, fpSaveImage);
		hv_msg("IMAGE[%d] has finish!\n", encoder_context->frame_count);
		fclose(fpSaveImage);
	} else {
		if (!buff) {
			hv_err("onVideoDataEnc: buff = NULL\n");
			return -1;
		}
		/* malloc and copy encode data to packet */
		packet = (CdxMuxerPacketT*)malloc(sizeof(CdxMuxerPacketT));
		packet->buflen		= buff->buflen;
		packet->length		= buff->length;
		packet->pts			= buff->pts;
		packet->type		= buff->type;
		packet->streamIndex	= buff->streamIndex;
		packet->duration	= buff->duration;
		packet->buf			= malloc(buff->buflen);
		memcpy(packet->buf, buff->buf, packet->buflen);
		hv_msg("before mux the Pts is = %lld ms\n",packet->pts);
		/* push packet to cdxqueue */
		CdxQueuePush(encoder_context->dataQueue,packet);
		hv_msg("CdxQueuePush Finish\n");
	}
	encoder_context->frame_count++;
	return 0;
}

/* TO DO */
int onAudioDataEnc(void *app,CdxMuxerPacketT *buff)
{
	CdxMuxerPacketT *packet = NULL;
	EncoderContext* encoder_context = (EncoderContext*)app;
	if (!buff)
		return 0;
	packet = (CdxMuxerPacketT*)malloc(sizeof(CdxMuxerPacketT));
	packet->buflen = buff->buflen;
	packet->length = buff->length;
	packet->buf = malloc(buff->buflen);
	memcpy(packet->buf, buff->buf, packet->buflen);
	packet->pts = buff->pts;
	packet->type = buff->type;
	packet->streamIndex = buff->streamIndex;

	CdxQueuePush(encoder_context->dataQueue,packet);
	return 0;
}

int init_encoder(void *arg)
{
	hawkview_handle *hv = (hawkview_handle*)arg;

#ifdef DISPLAY_SOURCE_DATA
        disp_mgr_init();
        disp_video_chanel_open();
#endif

	hv->encoder.mEncDataCallBackOps.onAudioDataEnc = onAudioDataEnc;
	hv->encoder.mEncDataCallBackOps.onVideoDataEnc = onVideoDataEnc;

#ifdef DISPLAY_SOURCE_DATA
        signal(SIGINT, release_de);
#endif

	memset(&hv->encoder.encoder_context, 0, sizeof(EncoderContext));
	hv->encoder.encoder_context.pool = AwPoolCreate(NULL);
	hv->encoder.encoder_context.dataQueue = CdxQueueCreate(hv->encoder.encoder_context.pool);
	hv->encoder.encoder_context.save_path = hv->detect.path;

	/* photo or video */
	if (strcmp(hv->detect.media_type, "photo") == 0) {
		hv->encoder.encoder_context.saveframe_Flag = 1;
	} else if (strcmp(hv->detect.media_type, "video") == 0) {
		hv->encoder.encoder_context.saveframe_Flag = 0;
	}
	/* set videoConfig */
	memset(&hv->encoder.encoder_context.videoConfig, 0x00, sizeof(VideoEncodeConfig));
	hv->encoder.encoder_context.videoConfig.nFrameRate		= hv->capture.cap_fps;
	hv->encoder.encoder_context.videoConfig.nOutHeight		= hv->capture.cap_h;
	hv->encoder.encoder_context.videoConfig.nOutWidth		= hv->capture.cap_w;
	hv->encoder.encoder_context.videoConfig.nSrcHeight		= hv->capture.cap_h;
	hv->encoder.encoder_context.videoConfig.nSrcWidth		= hv->capture.cap_w;
	hv->encoder.encoder_context.videoConfig.nBitRate		= 3*1024*1024;
	hv->encoder.encoder_context.videoConfig.bUsePhyBuf		= hv->encoder.flag_UsePhyBuf;
#ifdef TARGET_BOARD
	//hv->encoder.encoder_context.videoConfig.nInputYuvFormat	= hv->encoder.SOURCE_FMT;
#else
	hv->encoder.encoder_context.videoConfig.nInputYuvFormat	= hv->encoder.SOURCE_FMT;
#endif
	hv->encoder.encoder_context.videoConfig.nType			= hv->encoder.ENCODE_FMT;
	/* AwEncoderCreate */
	hv->encoder.encoder_context.mAwEncoder = AwEncoderCreate(&hv->encoder.encoder_context);
	if (hv->encoder.encoder_context.mAwEncoder == NULL) {
		hv_err("can not create AwEncoder\n");
		return -1;
	}

	AwEncoderInit(hv->encoder.encoder_context.mAwEncoder, &hv->encoder.encoder_context.videoConfig, NULL, &hv->encoder.mEncDataCallBackOps);
	AwEncoderStart(hv->encoder.encoder_context.mAwEncoder);
	AwEncoderGetExtradata(hv->encoder.encoder_context.mAwEncoder, &hv->encoder.encoder_context.extractDataBuff, &hv->encoder.encoder_context.extractDataLength);

	return 0;
}

int quit_encoder(void *arg)
{
	hawkview_handle *hv = (hawkview_handle*)arg;

	/* AwEncoderDestory */
	AwEncoderDestory(hv->encoder.encoder_context.mAwEncoder);

	return 0;
}

void *Encoder_Thread(void *arg)
{
	struct v4l2_buffer buf_pop;
	VideoInputBuffer buf;
	int ret;
	char source_data_path[30];
	unsigned int layer_id;
	hawkview_handle *hv = (hawkview_handle*)arg;

#ifdef DISPLAY_SOURCE_DATA
	disp_rect info;
	struct win_info win;
	unsigned int fbAddr[3];

	memset(&info,0,sizeof(disp_rect));

	info.width	= hv->capture.cap_w;//disp_screenwidth_get();
	info.height	= hv->capture.cap_h;//disp_screenheight_get();

	memset(&win, 0, sizeof(struct win_info));

	layer_id = hv->detect.dev_id + 1;

	if (hv->detect.dev_id == 0) {
		win.scn_win.x = 0;
        win.scn_win.y = 0;
		win.scn_win.w = disp_screenwidth_get();
        win.scn_win.h = disp_screenheight_get();
	} else {
		win.scn_win.x = 480;
		win.scn_win.y = 0;
		win.scn_win.w = info.width/2;
        win.scn_win.h = info.height/2;
	}
	win.src_win.nWidth = info.width;
	win.src_win.nHeight= info.height;
	win.src_win.crop_x = 0;
	win.src_win.crop_y= 0;
	win.src_win.crop_w= info.width;
	win.src_win.crop_h= info.height;
	win.nScreenHeight = 480;
	win.nScreenWidth = 800;
#endif
	memset(&buf, 0x00, sizeof(VideoInputBuffer));
	hv->encoder.encoder_context.frame_count = 0;
	int num =0 ;

	//while (hv->encoder.encoder_context.frame_count < hv->detect.frame_num)
	while (hv->encoder.encoder_context.frame_count <= hv->detect.frame_num)
	{
		hv_msg("hv->encoder.encoder_context.frame_count is [%d]\n",hv->encoder.encoder_context.frame_count);

		hv_msg("Before POP!\n");
		/* get buf_pop */
		sem_wait(&hv->vid_thread.vid_sem);
		pthread_mutex_lock(&hv->vid_thread.mutex);
		//pthread_cond_wait(&hv->vid_thread.cond, &hv->vid_thread.mutex);
		num = FIFO_Pop(hv->capture.fifo, &buf_pop);
		pthread_mutex_unlock(&hv->vid_thread.mutex);
		hv_msg("After POP! and ret of POP is %d\n", num);

		buf.nID = buf_pop.index;
		buf.nPts = secs_to_msecs((long long)(buf_pop.timestamp.tv_sec), (long long)(buf_pop.timestamp.tv_usec));
		hv_msg("the pop_buf.nID is = %ld\n", buf.nID);

#if SAVE_SOURCE_DATA
		sprintf(source_data_path, "%s/%s%d%s", hv->encoder.encoder_context.save_path, \
				"source_data", hv->encoder.encoder_context.frame_count, ".jpeg");
		save_frame_to_file(source_data_path, hv->capture.buffers[buf_pop.index].start, buf_pop.bytesused);
#endif

#ifdef DISPLAY_SOURCE_DATA

                fbAddr[0] = (unsigned int)buf_pop.m.offset;
                fbAddr[1] = (unsigned int)(fbAddr[0] + ALIGN_16B(hv->capture.cap_w) * ALIGN_16B(hv->capture.cap_h));
		ret = disp_video_layer_param_set(fbAddr, VIDEO_PIXEL_FORMAT_NV21, &win, layer_id);
#endif
		/* some usb camera first frame is error */
		if (hv->encoder.encoder_context.frame_count == 0 ) {
			/* VIDIOC_QBUF qbuf */
			if (ioctl(hv->detect.videofd, VIDIOC_QBUF, &buf_pop) == 0)
				hv_msg("************QBUF[%d] FINISH**************\n",buf_pop.index);
			hv->encoder.encoder_context.frame_count ++;
			continue;
		} else {
		if (hv->decoder.flag_decoder == 0) {
			if (hv->encoder.flag_UsePhyBuf == 0) {
			buf.nLen = buf_pop.bytesused;
			buf.pData = (unsigned char*)malloc(buf.nLen);
			memcpy(buf.pData, hv->capture.buffers[buf_pop.index].start, buf.nLen);
			} else if (hv->encoder.flag_UsePhyBuf == 1) {
				/* encoder use phy_addr */
				buf.pAddrPhyY = (unsigned char*)buf_pop.m.offset;
				buf.pAddrPhyC = buf.pAddrPhyY + ALIGN_16B(hv->capture.cap_w) * hv->capture.cap_h;
			}
			/* VIDIOC_QBUF qbuf */
			if (ioctl(hv->detect.videofd, VIDIOC_QBUF, &buf_pop) == 0)
				hv_msg("191************QBUF[%d] FINISH**************\n",buf_pop.index);
		} else if (hv->decoder.flag_decoder == 1) {
			/* Decoder_Frame */
			Decoder_Frame(hv, &buf_pop);
			/* VIDIOC_QBUF qbuf */
			if (ioctl(hv->detect.videofd, VIDIOC_QBUF, &buf_pop) == 0)
				hv_msg("197************QBUF[%d] FINISH**************\n",buf_pop.index);
			/* encoder when decoder success */
			if (hv->decoder.fail_decoder == 0) {
				if (hv->encoder.flag_UsePhyBuf == 0) {
					/* encoder use vir_addr */
					buf.nLen = hv->decoder.frame_size;
					buf.pData = (unsigned char*)malloc(buf.nLen);
					memcpy(buf.pData, hv->decoder.Vir_Y_Addr, buf.nLen*2/3);
					memcpy(buf.pData + buf.nLen*2/3, hv->decoder.Vir_U_Addr, buf.nLen/3);
				} else if (hv->encoder.flag_UsePhyBuf == 1) {
					/* encoder use phy_addr */
					buf.pAddrPhyY = (unsigned char*)hv->decoder.Phy_Y_Addr;
					buf.pAddrPhyC = (unsigned char*)hv->decoder.Phy_C_Addr;
				}
			} else if (hv->decoder.fail_decoder == 1) {
				/* free buf.pData */
				if (buf.pData != NULL) {
					free(buf.pData);
					buf.pData = NULL;
					hv_msg("free buf.pData finish\n");
				}
				//num++;
				continue;
			}
		}
		/* get timestamp_start */
		if (hv->encoder.encoder_context.frame_count == 1) {
			hv->encoder.timestamp_start = buf.nPts;
			hv->encoder.timestamp_now = hv->encoder.timestamp_start;
		}

		/* AwEncoderWriteYUVdata */
		//if (buf.nPts >= hv->encoder.timestamp_now) {
			hv_msg("the interval between two frames is %lld ms\n", buf.nPts - hv->encoder.timestamp_now);
			ret = AwEncoderWriteYUVdata(hv->encoder.encoder_context.mAwEncoder, &buf);
			if (ret == 0) {
				hv_msg("success to write YUVdata\n");
				hv->encoder.timestamp_now = buf.nPts;
			}
		//}
		/* free buf.pData */
		if (buf.pData != NULL) {
			free(buf.pData);
			buf.pData = NULL;
			hv_msg("free buf.pData finish\n");
		}
		//hv->encoder.encoder_context.frame_count++;
		}
	}
	AwEncoderStop(hv->encoder.encoder_context.mAwEncoder);
	quit_encoder(hv);
	/*  */
	hv->muxer.videoEos = 1;
	return (void*)0;
}
