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

#include "camera_mux.h"

int init_muxer(void *arg)
{
	hawkview_handle *hv = (hawkview_handle*)arg;
	CdxMuxerMediaInfoT mediainfo;
	CdxFsCacheMemInfo fs_cache_mem;
	RecoderWriterT *rw = NULL;
	int fs_mode = FSWRITEMODE_DIRECT;
	int fs_cache_size = 1024 * 1024;

	/* should set by usr */
	hv->muxer.muxType = CDX_MUXER_MOV;
	/* set mux purl */
	switch (hv->muxer.muxType) {
		case CDX_MUXER_AAC:
			sprintf(hv->muxer.pUrl,"%s/%s",hv->detect.path,"save.aac");
			break;

		case CDX_MUXER_MP3:
			sprintf(hv->muxer.pUrl,"%s/%s",hv->detect.path,"save.mp3");
			break;

		case CDX_MUXER_TS:
			sprintf(hv->muxer.pUrl,"%s/%s",hv->detect.path,"save.ts");
			break;

		default:
			sprintf(hv->muxer.pUrl, "%s/%s%d_%d%s", hv->detect.path, "save", hv->detect.dev_id, hv->muxer.videoNum + 1, ".mp4");
			break;
	}
	/* CdxMuxerCreate */
	if (hv->muxer.pUrl) {
		/* CdxWriterCreat */
#ifdef TARGET_BOARD
		if ((hv->muxer.pStream = CdxWriterCreat(hv->muxer.pUrl)) == NULL) {
#else
		if ((hv->muxer.pStream = CdxWriterCreat()) == NULL) {
#endif
			hv_err("CdxWriterCreat failed\n");
			return -1;
		}
		/* RWOpen pUrl */
		rw = (RecoderWriterT*)hv->muxer.pStream;
		rw->file_mode = FD_FILE_MODE;
		strcpy(rw->file_path, hv->muxer.pUrl);
		RWOpen(hv->muxer.pStream);
		/* CdxMuxerCreate */
		hv->muxer.pMuxer = CdxMuxerCreate(hv->muxer.muxType, hv->muxer.pStream);
		if(hv->muxer.pMuxer == NULL) {
			hv_err("CdxMuxerCreate failed\n");
			return -1;
		}
	}

	/* Set MediaInfo */
	memset(&mediainfo, 0, sizeof(CdxMuxerMediaInfoT));
	/* set audio of Mediainfo */
	switch (hv->encoder.encoder_context.audioConfig.nType) {
		case AUDIO_ENCODE_PCM_TYPE:
			mediainfo.audio.eCodecFormat = AUDIO_ENCODER_PCM_TYPE;
			break;

		case AUDIO_ENCODE_AAC_TYPE:
			mediainfo.audio.eCodecFormat = AUDIO_ENCODER_AAC_TYPE;
			break;

		case AUDIO_ENCODE_MP3_TYPE:
			mediainfo.audio.eCodecFormat = AUDIO_ENCODER_MP3_TYPE;
			break;

		case AUDIO_ENCODE_LPCM_TYPE:
			mediainfo.audio.eCodecFormat = AUDIO_ENCODER_LPCM_TYPE;
			break;

		default:
			hv_msg("unknown audio type(%d)\n", hv->encoder.encoder_context.audioConfig.nType);
			break;
	}
#if AUDIO_INPUT
	mediainfo.audioNum = 1;
#endif
	mediainfo.audio.nAvgBitrate		= hv->encoder.encoder_context.audioConfig.nBitrate;
	mediainfo.audio.nBitsPerSample		= hv->encoder.encoder_context.audioConfig.nSamplerBits;
	mediainfo.audio.nChannelNum			= hv->encoder.encoder_context.audioConfig.nOutChan;
	mediainfo.audio.nMaxBitRate		= hv->encoder.encoder_context.audioConfig.nBitrate;
	mediainfo.audio.nSampleRate		= hv->encoder.encoder_context.audioConfig.nOutSamplerate;
	mediainfo.audio.nSampleCntPerFrame	= 1024; // aac
	/* set video of Mediainfo */
	switch (hv->encoder.encoder_context.videoConfig.nType) {
		case VIDEO_ENCODE_H264:
			mediainfo.video.eCodeType = VENC_CODEC_H264;
			break;

		case VENC_CODEC_JPEG:
			mediainfo.video.eCodeType = VENC_CODEC_JPEG;
			break;

		default:
			hv_msg("unknown video type(%d)\n", hv->encoder.encoder_context.videoConfig.nType);
			break;
	}
#if VIDEO_INPUT
	mediainfo.videoNum = 1;
#endif
	mediainfo.video.nWidth			= hv->encoder.encoder_context.videoConfig.nOutWidth;
	mediainfo.video.nHeight			= hv->encoder.encoder_context.videoConfig.nOutHeight;
	mediainfo.video.nFrameRate		= hv->encoder.encoder_context.videoConfig.nFrameRate;
	/* print mux mediainfo */
	printf("####thread_mux######******************* mux mediainfo *****************************\n");
	printf("####thread_mux######videoNum                   : %d\n", mediainfo.videoNum);
	printf("####thread_mux######audioNum                   : %d\n", mediainfo.audioNum);
	printf("####thread_mux######videoTYpe                  : %d\n", mediainfo.video.eCodeType);
	printf("####thread_mux######framerate                  : %d\n", mediainfo.video.nFrameRate);
	printf("####thread_mux######width                      : %d\n", mediainfo.video.nWidth);
	printf("####thread_mux######height                     : %d\n", mediainfo.video.nHeight);
	printf("####thread_mux######**************************************************************\n");
	/* CdxMuxerSetMediaInfo */
	if (CdxMuxerSetMediaInfo(hv->muxer.pMuxer, &mediainfo) < 0) {
		hv_err("CdxMuxerSetMediaInfo failed\n");
		return -1;
	}

#if FS_WRITER
	memset(&fs_cache_mem, 0, sizeof(CdxFsCacheMemInfo));
	/* FSWRITEMODE_CACHETHREAD */
	/* fs_cache_mem.m_cache_size = 512 * 1024;
	fs_cache_mem.mp_cache = (cdx_int8*)malloc(fs_cache_mem.m_cache_size);
	if (fs_cache_mem.mp_cache == NULL) {
		printf("####thread_mux######fs_cache_mem.mp_cache malloc failed\n");
		return NULL;
	}
	CdxMuxerControl(hv->muxer.pMuxer, SET_CACHE_MEM, &fs_cache_mem);
	fs_mode = FSWRITEMODE_CACHETHREAD; */

	/* FSWRITEMODE_SIMPLECACHE */
	fs_cache_size = 512 * 1024;
	CdxMuxerControl(hv->muxer.pMuxer, SET_FS_SIMPLE_CACHE_SIZE, &fs_cache_size);
	fs_mode = FSWRITEMODE_SIMPLECACHE;
	/* FSWRITEMODE_SIMPLECACHE */
	/* fs_mode = FSWRITEMODE_DIRECT; */
	CdxMuxerControl(hv->muxer.pMuxer, SET_FS_WRITE_MODE, &fs_mode);
#endif

}

void *Muxer_Thread(void *arg)
{
	hawkview_handle *hv = (hawkview_handle*)arg;
	CdxMuxerPacketT *mPacket = NULL;

	if (hv->encoder.encoder_context.extractDataLength > 0 && hv->muxer.pMuxer) {
		hv_msg("demo WriteExtraData\n");
		if(hv->muxer.pMuxer)
			CdxMuxerWriteExtraData(hv->muxer.pMuxer, hv->encoder.encoder_context.extractDataBuff, \
									hv->encoder.encoder_context.extractDataLength, 0);
	}

	/* muxer write head */
	if (hv->muxer.pMuxer) {
		CdxMuxerWriteHeader(hv->muxer.pMuxer);
		hv_msg("muxer write head\n");
	}

#if AUDIO_INPUT && VIDEO_INPUT
	while ((audioEos==0) || (videoEos==0))
#elif VIDEO_INPUT
	while(hv->muxer.videoEos == 0)
#elif AUDIO_INPUT
	while(audioEos==0)
#endif
	{
		while (!CdxQueueEmpty(hv->encoder.encoder_context.dataQueue))
		{
			mPacket = CdxQueuePop(hv->encoder.encoder_context.dataQueue);
			if (hv->muxer.pMuxer) {
				if(CdxMuxerWritePacket(hv->muxer.pMuxer, mPacket) < 0) {
					hv_err("CdxMuxerWritePacket failed\n");
				}
			}
			free(mPacket->buf);
			free(mPacket);
		}
	}
	/* muxer write trailer */
	if (hv->muxer.pMuxer) {
		CdxMuxerWriteTrailer(hv->muxer.pMuxer);
		hv_msg("muxer write trailer\n");
	}
	/* close pMuxer and destroy pStreeam*/
	hv_msg("CdxMuxerClose\n");
	if (hv->muxer.pMuxer) {
		CdxMuxerClose(hv->muxer.pMuxer);
		hv->muxer.pMuxer = NULL;
	}
	if (hv->muxer.pStream) {
		RWClose(hv->muxer.pStream);
		CdxWriterDestroy(hv->muxer.pStream);
		hv->muxer.pStream = NULL;
	}

/* #if FS_WRITER
	if (fs_cache_mem.mp_cache) {
		free(fs_cache_mem.mp_cache);
		fs_cache_mem.mp_cache = NULL;
	}
#endif */
	hv_msg("MuxerThread has finish!\n");
	hv->muxer.exitFlag  = 1;

	return (void*)0;
}
