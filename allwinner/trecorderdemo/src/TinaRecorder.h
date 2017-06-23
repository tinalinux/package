#ifndef _TINA_RECORDER_H_
#define _TINA_RECORDER_H_

#define MAX_URL_LEN 1024

typedef enum
{
	TR_OUTPUT_TS,
	TR_OUTPUT_MOV,
	TR_OUTPUT_JPG,
	TR_OUTPUT_AAC,
	TR_OUTPUT_MP3,
	TR_OUTPUT_END
}outputFormat;

typedef enum
{
	TR_VIDEO_H264,
	TR_VIDEO_MJPEG,
	TR_VIDEO_END
}videoEncodeFormat;

typedef enum
{
	TR_AUDIO_PCM,
	TR_AUDIO_AAC,
	TR_AUDIO_MP3,
	TR_AUDIO_LPCM,
	TR_AUDIO_END
}audioEncodeFormat;

typedef enum
{
	TR_CAPTURE_JPG,
	TR_CAPTURE_END
}captureFormat;

typedef enum
{
    TR_PIXEL_YUV420SP,//NV12
    TR_PIXEL_YVU420SP,//NV21
    TR_PIXEL_YUV420P,//YU12
    TR_PIXEL_YVU420P,//YV12
    TR_PIXEL_YUV422SP,
    TR_PIXEL_YVU422SP,
    TR_PIXEL_YUV422P,
    TR_PIXEL_YVU422P,
    TR_PIXEL_YUYV422,
    TR_PIXEL_UYVY422,
    TR_PIXEL_YVYU422,
    TR_PIXEL_VYUY422,
    TR_PIXEL_ARGB,
    TR_PIXEL_RGBA,
    TR_PIXEL_ABGR,
    TR_PIXEL_BGRA,
    TR_PIXEL_TILE_32X32,//MB32_420
    TR_PIXEL_TILE_128X32,
}pixelFormat;

typedef struct
{
	int isPhy;
	int bufferId;
	long long pts;
}dataParam;

typedef struct
{
	int topLeft;
	int topRight;
	int bottomLeft;
	int bottomRight;
}dispRect;

typedef int (*readCallback)(void **,int,dataParam);
typedef int (*writeCallback)(void **,int,dataParam);

typedef struct
{
	int (*init)(int enable,int format,int framerate,int width,int height,readCallback callback);
	int (*readData)(void *data,int size);
	int (*dequeue)(void **data,int *size,dataParam *param);
	int (*queue)(dataParam *param);
	int (*getEnable)();
	int (*getFrameRate)();
	int (*getFormat)();
	int (*getSize)(int *width,int *height);

	int (*setEnable)(int enable);
	int (*setFrameRate)(int framerate);
	int (*setFormat)(int format);
	int (*setSize)(int width,int height);

	int (*setparameter)(int cmd,int param);

	int enable;
	int framerate;
	int width,height;
	int format;
}vInPort;

typedef struct
{
	int (*init)(int enable,int format,int samplerate,int channels,int bitrate,readCallback callback);
	int (*readData)(void *data,int size);

	int (*getEnable)();
	int (*getFormat)();
	int (*getAudioSampleRate)();
	int (*getAudioChannels)();
	int (*getAudioBitRate)();

	int (*setEnable)(int enable);
	int (*setFormat)(int format);
	int (*setAudioSampleRate)(int samplerate);
	int (*setAudioChannels)(int channels);
	int (*setAudioBitRate)(int bitrate);

	int (*setparameter)(int cmd,int param);
	long long (*getpts)();

	int enable;
	int format;
	int samplerate;
	int channels;
	int bitRate;
	long long pts;
}aInPort;

typedef struct
{
	int (*init)(int enable,int rotate,dispRect *rect,writeCallback callback);
	int (*writeData)(void **data,int size);

	int (*dequeue)(void **data,int *size,dataParam *param);
	int (*queueToDisplay)(void *data,int size,dataParam *param);

	int (*setEnable)(int enable);
	int (*setRect)(dispRect *rect);
	int (*setRotateAngel)(int retancle);

	int (*getEnable)();
	int (*getRect)(dispRect *rect);
	int (*getRotateAngel)();

	int enable;
	int rotate;
	dispRect rect;
}dispOutPort;

typedef struct
{
	int (*init)(char *url,writeCallback callback);
	int (*writeData)(void **data,int size,dataParam *para);
	int (*getUrl)(char *url);
	int (*setUrl)(char *url);
	int (*setEnable)(int enable);
	int (*getEnable)();

	int enable;
	char url[MAX_URL_LEN];
}rOutPort;

typedef int (*Rcallback)(int msg,int userData);

typedef struct
{
	int (*setVideoInPort)(void *hdl,vInPort *vPort);
	int (*setAudioInPort)(void *hdl,aInPort *aPort);
	int (*setDispOutPort)(void *hdl,dispOutPort *dispPort);
	int (*setOutput)(void *hdl,rOutPort *output);

	int (*start)(void *hdl);
	int (*stop)(void *hdl);
	int (*release)(void *hdl);
	int (*prepare)(void *hdl);
	int (*reset)(void *hdl);

	int (*setEncodeSize)(void *hdl,int width,int height);
	int (*setEncodeBitRate)(void *hdl,int bitrate);
	int (*setOutputFormat)(void *hdl,int format);
	int (*setCallback)(void *hdl,Rcallback callback);
	int (*setEncodeVideoFormat)(void *hdl,int format);
	int (*setEncodeAudioFormat)(void *hdl,int format);
	int (*setEncodeFramerate)(void *hdl,int framerate);

	int (*getRecorderTime)(void *hdl);
	int (*captureCurrent)(void *hdl,char *path,int format);
	int (*setMaxRecordTime)(void *hdl,int ms);
	int (*setParameter)(void *hdl,int cmd,int parameter);

	int encodeWidth;
	int encodeHeight;
	int encodeBitrate;
	int encodeFramerate;
	outputFormat outputFormat;
	Rcallback callback;
	videoEncodeFormat encodeVFormat;
	audioEncodeFormat encodeAFormat;
	int maxRecordTime;
	vInPort vport;
	aInPort aport;
	dispOutPort dispport;
	rOutPort outport;
	void *EncoderContext;
}MediaRecorder;

MediaRecorder *CreateTinaRecorder();
int DestroyTinaRecorder(MediaRecorder *hdl);
#endif
