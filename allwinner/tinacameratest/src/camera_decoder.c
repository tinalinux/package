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

#include "camera_decoder.h"

int init_decoder(void *decoder)
{
	struct ScMemOpsS* memops;
	VideoStreamInfo videoInfo;
	VConfig vConfig;

	decoder_handle* dec = (decoder_handle*)decoder;

	/* create a videodecoder */
	dec->pVideo = CreateVideoDecoder();
	if (dec->pVideo == NULL) {
		hv_err("create video decoder failed\n");
		return -1;
	}

	/* set the videoInfo */
	memset(&videoInfo, 0x00, sizeof(VideoStreamInfo));
	videoInfo.eCodecFormat = dec->SOURCE_FMT;

	/* allocate mem */
	memops = MemAdapterGetOpsS();
	if (memops == NULL) {
		hv_err("MemAdapterGetOpsS failed\n");
		return -1;
	}
	CdcMemOpen(memops);

	/* set the vConfig */
	memset(&vConfig, 0x00, sizeof(VConfig));
	vConfig.bDisable3D = 0;
	vConfig.bDispErrorFrame = 0;
	vConfig.bNoBFrames = 0;
	vConfig.bRotationEn = 0;
	vConfig.bScaleDownEn = 0;
	vConfig.nHorizonScaleDownRatio = 0;
	vConfig.nVerticalScaleDownRatio = 0;
	vConfig.eOutputPixelFormat = dec->DECODE_FMT;
	vConfig.nDeInterlaceHoldingFrameBufferNum = 0;
	vConfig.nDisplayHoldingFrameBufferNum = 0;
	vConfig.nRotateHoldingFrameBufferNum = 0;
	vConfig.nDecodeSmoothFrameBufferNum = 1;
	vConfig.nVbvBufferSize = 2*1024*1024;
	vConfig.bThumbnailMode = 0;
	vConfig.memops = memops;

	/* Load video decoder libs */

#ifdef TARGET_BOARD
	//AddVDPlugin();
#else
	AddVDPlugin();
#endif

	if ((InitializeVideoDecoder(dec->pVideo, &videoInfo, &vConfig)) != 0) {
		hv_err("Initialize Video Decoder failed\n");
		return -1;
	}
	hv_msg("Initialize Video Decoder ok\n");
}

int Decoder_Frame(void *arg, void *buf_pop)
{
	int RequireSize = 0;
	char *Buf, *RingBuf;
	int BufSize, RingBufSize;
	int ret, num;
	char *Data = NULL;
	VideoPicture *videoPicture = NULL;
	VideoStreamDataInfo DataInfo;

	hawkview_handle *hv = (hawkview_handle*)arg;
	struct v4l2_buffer *param = (struct v4l2_buffer *)buf_pop;

	RequireSize = param->bytesused;
	hv_msg("Decode: RequireSize is %d\n",RequireSize);
	Data = hv->capture.buffers[param->index].start;

	hv_msg("Decode: before RequestVideoStreamBuffer\n");
	/* Request Video Stream Buffer */
	if (RequestVideoStreamBuffer(hv->decoder.pVideo, RequireSize, (char**)&Buf, &BufSize,
								(char**)&RingBuf, &RingBufSize, 0) != 0) {
		hv_err("RequestVideoStreamBuffer failed\n");
		return -1;
	}
	hv_msg("Decode: before memcpy(Buf, Data, RequireSize)\n");
	/* Copy Video Stream Data to Buf and RingBuf */
	if (BufSize + RingBufSize < RequireSize) {
		hv_err("request buffer failed, buffer is not enough\n");
		return -1;
	}
	if (BufSize >= RequireSize) {
		memcpy(Buf, Data, RequireSize);
	}
	else {
		memcpy(Buf, Data, BufSize);
		memcpy(RingBuf, Data+BufSize, RequireSize-BufSize);
	}

	/* Submit Video Stream Data */
	memset(&DataInfo, 0, sizeof(DataInfo));
	DataInfo.pData = Buf;
	DataInfo.nLength = RequireSize;
	DataInfo.bIsFirstPart = 1;
	DataInfo.bIsLastPart = 1;
	hv_msg("Decode: before SubmitVideoStreamData\n");
	if (SubmitVideoStreamData(hv->decoder.pVideo, &DataInfo, 0)) {
		hv_err("Submit Video Stream Data failed!\n");
		return -1;
	}

	/* decode stream */
	int endofstream = 0;
	int decodekeyframeonly = 0;
	int dropBFrameifdelay = 0;
	int64_t currenttimeus = 0;

	hv_msg("Decode: before DecodeVideoStream\n");
	ret = DecodeVideoStream(hv->decoder.pVideo, endofstream, decodekeyframeonly,dropBFrameifdelay, currenttimeus);
	hv_msg("Decode: ret of DecodeVideoStream is %d\n",ret);
	switch (ret) {
		case VDECODE_RESULT_KEYFRAME_DECODED:
		case VDECODE_RESULT_FRAME_DECODED:
		{
			num = ValidPictureNum(hv->decoder.pVideo, 0);
			if (num > 0) {
				videoPicture = RequestPicture(hv->decoder.pVideo, 0);
				if (videoPicture == NULL) {
					hv_err("RequestPicture fail\n");
					break;
				}
				/* get Vir_Addr and Phy_Addr */
				//Buf_Len = videoPicture->nWidth*videoPicture->nHeight*3/2;
				hv->decoder.frame_size = videoPicture->nWidth*videoPicture->nHeight*3/2;
				hv->decoder.Vir_Y_Addr = videoPicture->pData0;
				hv->decoder.Vir_U_Addr = videoPicture->pData1;

				hv->decoder.Phy_Y_Addr = videoPicture->phyYBufAddr;
				hv->decoder.Phy_C_Addr = videoPicture->phyCBufAddr;
				hv_msg("Decode Video Frame Finish\n");

				ReturnPicture(hv->decoder.pVideo, videoPicture);
				hv->decoder.fail_decoder = 0;
				return 0;
			}
			else {
				hv_err("ValidPictureNum error and num is %d\n", num);
			}
			break;
		}
		case VDECODE_RESULT_OK:
		case VDECODE_RESULT_CONTINUE:
		case VDECODE_RESULT_NO_FRAME_BUFFER:
		case VDECODE_RESULT_NO_BITSTREAM:
		case VDECODE_RESULT_RESOLUTION_CHANGE:
		case VDECODE_RESULT_UNSUPPORTED:
		default:
			hv_err("video decode stream error, ret is %d\n", ret);
	}

	hv->decoder.fail_decoder = 1;
	return -1;
}
