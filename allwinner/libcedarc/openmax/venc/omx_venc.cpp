/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : omx_venc.cpp
 *
 * Description : OpenMax Encoder Definition
 * History :
 *
 */

#define LOG_TAG "omx_venc"
#include "log.h"
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include "omx_venc.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memoryAdapter.h"
#include "CdcUtil.h"

#ifdef __ANDROID__
#include "MetadataBufferType.h"
#include <ion/ion.h>
#include <HardwareAPI.h>
#include <utils/CallStack.h>
#include <binder/IPCThreadState.h>
#include <ui/Rect.h>
#include <ui/GraphicBufferMapper.h>
#include <ui/GraphicBuffer.h>
#include <ui/Fence.h>
#include <media/IOMX.h>

#ifdef CONF_KITKAT_AND_NEWER
#include "hardware/hal_public.h"
#endif

#ifdef CONF_MARSHMALLOW_AND_NEWER
#include <OMX_IndexExt.h>
#endif

using namespace android;

#endif

#define SAVE_BITSTREAM 0

#if SAVE_BITSTREAM
static FILE *OutFile = NULL;
#endif

#ifdef CONF_ARMV7_A_NEON
extern "C" void ImgRGBA2YUV420SP_neon(unsigned char *pu8RgbBuffer,
                                      unsigned char** pu8SrcYUV,
                                      int *l32Width_stride,
                                      int l32Height);
#endif

#define DEFAULT_BITRATE 1024*1024*2

#define OPEN_STATISTICS 0
#define PRINT_FRAME_CNT (50)
#define HW_VIDEO_CALL_APK "com.huawei.iptv.stb.videotalk.activity"

#define BUF_ALIGN_SIZE 32
static unsigned int omx_venc_align(unsigned int x, int a)
{
    return (x + (a-1)) & (~(a-1));
}


#if (OPEN_STATISTICS)
int64_t nTimeUs1;
int64_t nTimeUs2;

static int64_t GetNowUs()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (int64_t)tv.tv_sec * 1000000ll + tv.tv_usec;
}
#endif


/* H.263 Supported Levels & profiles */
VIDEO_PROFILE_LEVEL_TYPE SupportedH263ProfileLevels[] = {
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level10},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level20},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level30},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level40},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level45},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level50},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level60},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level70},

  {-1, -1}
};

/* AVC Supported Levels & profiles  for cts*/
static VIDEO_PROFILE_LEVEL_TYPE CTSSupportedAVCProfileLevels[] = {
   {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel1},
   {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel1b},
   {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel11},
   {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel12},
   {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel13},
   {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel2},
   {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel21},
   {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel22},
   {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel3},
   {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel31},

   {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel1},
   {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel1b},
   {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel11},
   {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel12},
   {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel13},
   {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel2 },
   {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel21},
   {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel22},
   {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel3 },
   {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel31},
   {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel32},

   {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel1 },
   {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel1b},
   {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel11},
   {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel12},
   {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel13},
   {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel2 },
   {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel21},
   {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel22},
   {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel3 },
   {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel31},
   {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel32},

   {-1,-1}
};
/* AVC Supported Levels & profiles */
static VIDEO_PROFILE_LEVEL_TYPE SupportedAVCProfileLevels[] = {
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel1},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel1b},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel11},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel12},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel13},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel2},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel21},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel22},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel3},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel31},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel41},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel42},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel5},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel51},

  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel1},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel1b},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel11},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel12},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel13},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel2 },
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel21},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel22},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel3 },
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel31},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel32},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel4},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel41},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel42},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel5},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel51},

  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel1 },
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel1b},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel11},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel12},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel13},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel2 },
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel21},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel22},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel3 },
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel31},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel32},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel4},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel41},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel42},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel5},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel51},

  {-1,-1}
};

/*
 *     M A C R O S
 */

/*
 * Initializes a data structure using a pointer to the structure.
 * The initialization of OMX structures always sets up the nSize and nVersion fields
 *   of the structure.
 */
#define OMX_CONF_INIT_STRUCT_PTR(_variable_, _struct_name_) \
    memset((_variable_), 0x0, sizeof(_struct_name_)); \
    (_variable_)->nSize = sizeof(_struct_name_); \
    (_variable_)->nVersion.s.nVersionMajor = 0x1; \
    (_variable_)->nVersion.s.nVersionMinor = 0x1; \
    (_variable_)->nVersion.s.nRevision = 0x0; \
    (_variable_)->nVersion.s.nStep = 0x0


typedef enum VIDENC_CUSTOM_INDEX
{
    VideoEncodeCustomParamStoreMetaDataInBuffers = OMX_IndexVendorStartUnused,
    VideoEncodeCustomParamPrependSPSPPSToIDRFrames,
    VideoEncodeCustomParamEnableAndroidNativeBuffers,
    VideoEncodeCustomParamextendedVideo,
    VideoEncodeCustomParamextendedVideoSuperframe,
    VideoEncodeCustomParamextendedVideoSVCSkip,
    VideoEncodeCustomParamextendedVideoVBR,
    VideoEncodeCustomParamStoreANWBufferInMetadata,
    VideoEncodeCustomParamextendedVideoPSkip,
} VIDENC_CUSTOM_INDEX;


static VIDDEC_CUSTOM_PARAM sVideoEncCustomParams[] =
{
    {"OMX.google.android.index.storeMetaDataInBuffers",
        (OMX_INDEXTYPE)VideoEncodeCustomParamStoreMetaDataInBuffers},
    {"OMX.google.android.index.prependSPSPPSToIDRFrames",
        (OMX_INDEXTYPE)VideoEncodeCustomParamPrependSPSPPSToIDRFrames},
    {"OMX.google.android.index.enableAndroidNativeBuffers",
        (OMX_INDEXTYPE)VideoEncodeCustomParamEnableAndroidNativeBuffers},
    {"OMX.Topaz.index.param.extended_video",
        (OMX_INDEXTYPE)VideoEncodeCustomParamextendedVideo},
    {"OMX.aw.index.param.videoSuperFrameConfig",
        (OMX_INDEXTYPE)VideoEncodeCustomParamextendedVideoSuperframe},
    {"OMX.aw.index.param.videoSVCSkipConfig",
        (OMX_INDEXTYPE)VideoEncodeCustomParamextendedVideoSVCSkip},
    {"OMX.aw.index.param.videoVBRConfig",
        (OMX_INDEXTYPE)VideoEncodeCustomParamextendedVideoVBR},
    {"OMX.google.android.index.storeANWBufferInMetadata",
        (OMX_INDEXTYPE)VideoEncodeCustomParamStoreANWBufferInMetadata},
    {"OMX.aw.index.param.videoPSkipConfig",
        (OMX_INDEXTYPE)VideoEncodeCustomParamextendedVideoPSkip}
};

typedef enum OMX_VENC_COMMANDTYPE
{
    OMX_Venc_Cmd_Open,
    OMX_Venc_Cmd_Close,
    OMX_Venc_Cmd_Stop,
    OMX_Venc_Cmd_Enc_Idle,
    OMX_Venc_Cmd_ChangeBitrate,
    OMX_Venc_Cmd_ChangeColorFormat,
    OMX_Venc_Cmd_RequestIDRFrame,
    OMX_Venc_Cmd_ChangeFramerate,
} OMX_VENC_COMMANDTYPE;


typedef enum OMX_VENC_INPUTBUFFER_STEP
{
    OMX_VENC_STEP_GET_INPUTBUFFER,
    OMX_VENC_STEP_GET_ALLOCBUFFER,
    OMX_VENC_STEP_ADD_BUFFER_TO_ENC,
} OMX_VENC_INPUTBUFFER_STEP;


static void* ComponentThread(void* pThreadData);
static void* ComponentVencThread(void* pThreadData);


#ifdef __ANDROID__

#define GET_CALLING_PID    (IPCThreadState::self()->getCallingPid())
static void getCallingProcessName(char *name)
{
    char proc_node[128];

    if (name == 0)
    {
        loge("error in params");
        return;
    }

    memset(proc_node, 0, sizeof(proc_node));
    sprintf(proc_node, "/proc/%d/cmdline", GET_CALLING_PID);
    int fp = ::open(proc_node, O_RDONLY);
    if (fp > 0)
    {
        memset(name, 0, 128);
        ::read(fp, name, 128);
        ::close(fp);
        fp = 0;
        logd("Calling process is: %s", name);
    }
    else
    {
        loge("Obtain calling process failed");
    }
}

#endif

//* factory function executed by the core to create instances
void *get_omx_component_factory_fn(void)
{
    logd("----get_omx_component_factory_fn");
    return (new omx_venc);
}


void post_message_to_venc_and_wait(omx_venc *omx, OMX_S32 id)
{
      int ret_value;
      logv("omx_venc: post_message %d pipe out%d to venc\n", (int)id,omx->m_venc_cmdpipe[1]);
      ret_value = write(omx->m_venc_cmdpipe[1], &id, sizeof(OMX_S32));
      logv("post_message to pipe done %d\n",ret_value);
      omx_sem_down(&omx->m_msg_sem);
}

void post_message_to_venc(omx_venc *omx, OMX_S32 id)
{
      int ret_value;
      logv("omx_vdec: post_message %d pipe out%d to venc\n", (int)id,omx->m_venc_cmdpipe[1]);
      ret_value = write(omx->m_venc_cmdpipe[1], &id, sizeof(OMX_S32));
      logv("post_message to pipe done %d\n",ret_value);
}

omx_venc::omx_venc()
{
    m_state                = OMX_StateLoaded;
    m_cRole[0]             = 0;
    m_cName[0]             = 0;
    m_eCompressionFormat   = OMX_VIDEO_CodingUnused;
    m_pAppData             = NULL;
    m_thread_id            = 0;
    m_venc_thread_id       = 0;
    m_cmdpipe[0]           = 0;
    m_cmdpipe[1]           = 0;
    m_cmddatapipe[0]       = 0;
    m_cmddatapipe[1]       = 0;
    m_venc_cmdpipe[0]      = 0;
    m_venc_cmdpipe[1]      = 0;
    m_firstFrameFlag       = OMX_FALSE;
    m_framerate            = 30;
    mIsFromCts             = OMX_FALSE;
    mIsFromVideoeditor     = OMX_FALSE;
    m_useAllocInputBuffer  = OMX_FALSE;
    m_useAndroidBuffer     = OMX_FALSE;
    mFirstInputFrame       = OMX_TRUE;
    m_usePSkip             = OMX_FALSE;
    mEmptyBufCnt           = 0;
    mFillBufCnt            = 0;
    memops =  MemAdapterGetOpsS();
    CdcMemOpen(memops);

#if PRINTF_FRAME_SIZE
    mFrameCnt = 0;
    mAllFrameSize = 0;
    mTimeStart = 0;
    mTimeOut = 999;//ms
#endif
    memset(mCallingProcess,0,sizeof(mCallingProcess));

#ifdef __ANDROID__
    getCallingProcessName(mCallingProcess);
    if ((strcmp(mCallingProcess, "com.android.cts.media") == 0) ||
       (strcmp(mCallingProcess, "com.android.cts.videoperf") == 0) ||
       (strcmp(mCallingProcess, "com.android.pts.videoperf") == 0))
    {
        mIsFromCts           = OMX_TRUE;
    }
    else if (strcmp(mCallingProcess, "com.android.videoeditor") == 0)
    {
        mIsFromVideoeditor   = OMX_TRUE;
    }
#endif

    m_encoder = NULL;

    memset(&m_Callbacks, 0, sizeof(m_Callbacks));
    memset(&m_sInPortDefType, 0, sizeof(m_sInPortDefType));
    memset(&m_sOutPortDefType, 0, sizeof(m_sOutPortDefType));
    memset(&m_sInPortFormatType, 0, sizeof(m_sInPortFormatType));
    memset(&m_sOutPortFormatType, 0, sizeof(m_sOutPortFormatType));
    memset(&m_sPriorityMgmtType, 0, sizeof(m_sPriorityMgmtType));
    memset(&m_sInBufSupplierType, 0, sizeof(m_sInBufSupplierType));
    memset(&m_sOutBufSupplierType, 0, sizeof(m_sOutBufSupplierType));
    memset(&m_sInBufList, 0, sizeof(m_sInBufList));
    memset(&m_sOutBufList, 0, sizeof(m_sOutBufList));

    pthread_mutex_init(&m_inBufMutex, NULL);
    pthread_mutex_init(&m_outBufMutex, NULL);
    pthread_mutex_init(&m_pipeMutex, NULL);

    omx_sem_init(&m_msg_sem, 0);
    omx_sem_init(&m_input_sem, 0);

    logd("omx_enc Create!");
}


omx_venc::~omx_venc()
{
    OMX_S32 nIndex;
    // In case the client crashes, check for nAllocSize parameter.
    // If this is greater than zero, there are elements in the list that are not free'd.
    // In that case, free the elements.


    pthread_mutex_lock(&m_inBufMutex);

    for (nIndex=0; nIndex<m_sInBufList.nBufArrSize; nIndex++)
    {
        if (m_sInBufList.pBufArr[nIndex].pBuffer != NULL)
        {
            if (m_sInBufList.nAllocBySelfFlags & (1<<nIndex))
            {
                free(m_sInBufList.pBufArr[nIndex].pBuffer);
                m_sInBufList.pBufArr[nIndex].pBuffer = NULL;
            }
        }
    }

    if (m_sInBufList.pBufArr != NULL)
    {
        free(m_sInBufList.pBufArr);
    }

    if (m_sInBufList.pBufHdrList != NULL)
    {
        free(m_sInBufList.pBufHdrList);
    }

    memset(&m_sInBufList, 0, sizeof(struct _BufferList));
    m_sInBufList.nBufArrSize = m_sInPortDefType.nBufferCountActual;

    pthread_mutex_unlock(&m_inBufMutex);

    pthread_mutex_lock(&m_outBufMutex);

    for (nIndex=0; nIndex<m_sOutBufList.nBufArrSize; nIndex++)
    {
        if (m_sOutBufList.pBufArr[nIndex].pBuffer != NULL)
        {
            if (m_sOutBufList.nAllocBySelfFlags & (1<<nIndex))
            {
                free(m_sOutBufList.pBufArr[nIndex].pBuffer);
                m_sOutBufList.pBufArr[nIndex].pBuffer = NULL;
            }
        }
    }

    if (m_sOutBufList.pBufArr != NULL)
    {
        free(m_sOutBufList.pBufArr);
    }

    if (m_sOutBufList.pBufHdrList != NULL)
    {
        free(m_sOutBufList.pBufHdrList);
    }

    memset(&m_sOutBufList, 0, sizeof(struct _BufferList));
    m_sOutBufList.nBufArrSize = m_sOutPortDefType.nBufferCountActual;

    if(memops)
        CdcMemClose(memops);

    pthread_mutex_unlock(&m_outBufMutex);

    pthread_mutex_destroy(&m_inBufMutex);
    pthread_mutex_destroy(&m_outBufMutex);
    pthread_mutex_destroy(&m_pipeMutex);

    omx_sem_deinit(&m_msg_sem);
    omx_sem_deinit(&m_input_sem);

    logv("~omx_enc done!");
}

OMX_ERRORTYPE omx_venc::component_init(OMX_STRING pName)
{
    OMX_ERRORTYPE eRet = OMX_ErrorNone;
    int           err;
    OMX_U32       nIndex;
    char calling_process[256];

    logd(" COMPONENT_INIT, name = %s", pName);

    strncpy((char*)m_cName, pName, OMX_MAX_STRINGNAME_SIZE);

    if (!strncmp((char*)m_cName, "OMX.allwinner.video.encoder.avc", OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char*)m_cRole, "video_encoder.avc", OMX_MAX_STRINGNAME_SIZE);
        m_eCompressionFormat = OMX_VIDEO_CodingAVC;
    }
    else if (!strncmp((char*)m_cName, "OMX.allwinner.video.encoder.h263", OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char*)m_cRole, "video_encoder.avc", OMX_MAX_STRINGNAME_SIZE);
        m_eCompressionFormat = OMX_VIDEO_CodingH263;
    }
    else if (!strncmp((char*)m_cName, "OMX.allwinner.video.encoder.mpeg4", OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char*)m_cRole, "video_encoder.avc", OMX_MAX_STRINGNAME_SIZE);
        m_eCompressionFormat = OMX_VIDEO_CodingMPEG4;
    }
    else if (!strncmp((char*)m_cName, "OMX.allwinner.video.encoder.mjpeg", OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char*)m_cRole, "video_encoder.mjpeg", OMX_MAX_STRINGNAME_SIZE);
        m_eCompressionFormat = OMX_VIDEO_CodingMJPEG;
    }
    else
    {
        logv("\nERROR:Unknown Component\n");
        eRet = OMX_ErrorInvalidComponentName;
        return eRet;
    }

    // init codec type
    if (OMX_VIDEO_CodingAVC == m_eCompressionFormat)
    {
        m_vencCodecType = VENC_CODEC_H264;
    }
    else if (OMX_VIDEO_CodingVP8 == m_eCompressionFormat)
    {
        m_vencCodecType = VENC_CODEC_VP8;
    }
    else if (OMX_VIDEO_CodingMJPEG == m_eCompressionFormat)
    {
        m_vencCodecType = VENC_CODEC_JPEG;
    }
    else
    {
        logd("need check codec type");
    }

    // Initialize component data structures to default values
    OMX_CONF_INIT_STRUCT_PTR(&m_sPortParam, OMX_PORT_PARAM_TYPE);
    m_sPortParam.nPorts           = 0x2;
    m_sPortParam.nStartPortNumber = 0x0;

    // Initialize the video parameters for input port
    OMX_CONF_INIT_STRUCT_PTR(&m_sInPortDefType, OMX_PARAM_PORTDEFINITIONTYPE);
    m_sInPortDefType.nPortIndex                      = 0x0;
    m_sInPortDefType.bEnabled                        = OMX_TRUE;
    m_sInPortDefType.bPopulated                      = OMX_FALSE;
    m_sInPortDefType.eDomain                         = OMX_PortDomainVideo;
    m_sInPortDefType.format.video.nFrameWidth        = 0;
    m_sInPortDefType.format.video.nFrameHeight       = 0;
    m_sInPortDefType.eDir                            = OMX_DirInput;

//for 4k recorder @30fps
#ifdef CONF_SUPPORT_4K_30FPS_RECORDER
    m_sInPortDefType.nBufferCountMin                 = 4;
    m_sInPortDefType.nBufferCountActual              = 4;
#else
    m_sInPortDefType.nBufferCountMin                 = NUM_IN_BUFFERS;
    m_sInPortDefType.nBufferCountActual              = NUM_IN_BUFFERS;
#endif
#ifdef __ANDROID__
    getCallingProcessName(calling_process);
    if (strcmp(calling_process, HW_VIDEO_CALL_APK) == 0)
    {
        m_sInPortDefType.nBufferCountMin = 4;
        m_sInPortDefType.nBufferCountActual = 4;
        logd("HW_VIDOE_CALL:%s,F:%s,L:%d",HW_VIDEO_CALL_APK,__FUNCTION__,__LINE__);
    }
#endif
    m_sInPortDefType.nBufferSize = (OMX_U32)(m_sOutPortDefType.format.video.nFrameWidth *
                                             m_sOutPortDefType.format.video.nFrameHeight * 3 / 2);
    m_sInPortDefType.format.video.eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar; //fix it later

    // Initialize the video parameters for output port
    OMX_CONF_INIT_STRUCT_PTR(&m_sOutPortDefType, OMX_PARAM_PORTDEFINITIONTYPE);
    m_sOutPortDefType.nPortIndex                   = 0x1;
    m_sOutPortDefType.bEnabled                     = OMX_TRUE;
    m_sOutPortDefType.bPopulated                   = OMX_FALSE;
    m_sOutPortDefType.eDomain                      = OMX_PortDomainVideo;
    m_sOutPortDefType.format.video.cMIMEType       = (OMX_STRING)"YUV420";
    m_sOutPortDefType.format.video.nFrameWidth     = 176;
    m_sOutPortDefType.format.video.nFrameHeight    = 144;
    m_sOutPortDefType.eDir                         = OMX_DirOutput;

//for 4k recorder @30fps
#ifdef CONF_SUPPORT_4K_30FPS_RECORDER
    m_sOutPortDefType.nBufferCountMin              = 6;
    m_sOutPortDefType.nBufferCountActual           = 6;
#else
    m_sOutPortDefType.nBufferCountMin              = NUM_OUT_BUFFERS;
    m_sOutPortDefType.nBufferCountActual           = NUM_OUT_BUFFERS;
#endif
    if (strcmp(calling_process, HW_VIDEO_CALL_APK) == 0)
    {
        m_sOutPortDefType.nBufferCountMin = 4;
        m_sOutPortDefType.nBufferCountActual = 4;
        logd("HW_VIDOE_CALL:%s,F:%s,L:%d",HW_VIDEO_CALL_APK,__FUNCTION__,__LINE__);
    }
    m_sOutPortDefType.nBufferSize                 = OMX_VIDEO_ENC_INPUT_BUFFER_SIZE;

    m_sOutPortDefType.format.video.eCompressionFormat = m_eCompressionFormat;
    m_sOutPortDefType.format.video.cMIMEType          = (OMX_STRING)"";

    // Initialize the video compression format for input port
    OMX_CONF_INIT_STRUCT_PTR(&m_sInPortFormatType, OMX_VIDEO_PARAM_PORTFORMATTYPE);
    m_sInPortFormatType.nPortIndex         = 0x0;
    m_sInPortFormatType.nIndex             = 0x2;
    m_sInPortFormatType.eColorFormat       = OMX_COLOR_FormatYUV420SemiPlanar;

    m_inputcolorFormats[0] = OMX_COLOR_FormatYUV420SemiPlanar;
    m_inputcolorFormats[1] = OMX_COLOR_FormatAndroidOpaque;
    m_inputcolorFormats[2] = OMX_COLOR_FormatYVU420SemiPlanar;

    // Initialize the compression format for output port
    OMX_CONF_INIT_STRUCT_PTR(&m_sOutPortFormatType, OMX_VIDEO_PARAM_PORTFORMATTYPE);
    m_sOutPortFormatType.nPortIndex   = 0x1;
    m_sOutPortFormatType.nIndex       = 0x0;
    m_sOutPortFormatType.eCompressionFormat = m_eCompressionFormat;

    OMX_CONF_INIT_STRUCT_PTR(&m_sPriorityMgmtType, OMX_PRIORITYMGMTTYPE);

    OMX_CONF_INIT_STRUCT_PTR(&m_sInBufSupplierType, OMX_PARAM_BUFFERSUPPLIERTYPE );
    m_sInBufSupplierType.nPortIndex = 0x0;

    OMX_CONF_INIT_STRUCT_PTR(&m_sOutBufSupplierType, OMX_PARAM_BUFFERSUPPLIERTYPE );
    m_sOutBufSupplierType.nPortIndex = 0x1;

    // Initalize the output bitrate
    OMX_CONF_INIT_STRUCT_PTR(&m_sOutPutBitRateType, OMX_VIDEO_PARAM_BITRATETYPE);
    m_sOutPutBitRateType.nPortIndex = 0x01;
    m_sOutPutBitRateType.eControlRate = OMX_Video_ControlRateDisable;
    m_sOutPutBitRateType.nTargetBitrate = DEFAULT_BITRATE; //default bitrate

    // Initalize the output h264param
    OMX_CONF_INIT_STRUCT_PTR(&m_sH264Type, OMX_VIDEO_PARAM_AVCTYPE);
    m_sH264Type.nPortIndex                = 0x01;
    m_sH264Type.nSliceHeaderSpacing       = 0;
    m_sH264Type.nPFrames                  = -1;
    m_sH264Type.nBFrames                  = -1;
    m_sH264Type.bUseHadamard              = OMX_TRUE; /*OMX_FALSE*/
    m_sH264Type.nRefFrames                = 1; /*-1;  */
    m_sH264Type.nRefIdx10ActiveMinus1     = -1;
    m_sH264Type.nRefIdx11ActiveMinus1     = -1;
    m_sH264Type.bEnableUEP                = OMX_FALSE;
    m_sH264Type.bEnableFMO                = OMX_FALSE;
    m_sH264Type.bEnableASO                = OMX_FALSE;
    m_sH264Type.bEnableRS                 = OMX_FALSE;
    m_sH264Type.eProfile                  = OMX_VIDEO_AVCProfileBaseline; /*0x01;*/
    m_sH264Type.eLevel                    = OMX_VIDEO_AVCLevel1; /*OMX_VIDEO_AVCLevel11;*/
    m_sH264Type.nAllowedPictureTypes      = -1;
    m_sH264Type.bFrameMBsOnly             = OMX_FALSE;
    m_sH264Type.bMBAFF                    = OMX_FALSE;
    m_sH264Type.bEntropyCodingCABAC       = OMX_FALSE;
    m_sH264Type.bWeightedPPrediction      = OMX_FALSE;
    m_sH264Type.nWeightedBipredicitonMode = -1;
    m_sH264Type.bconstIpred               = OMX_FALSE;
    m_sH264Type.bDirect8x8Inference       = OMX_FALSE;
    m_sH264Type.bDirectSpatialTemporal    = OMX_FALSE;
    m_sH264Type.nCabacInitIdc             = -1;
    m_sH264Type.eLoopFilterMode           = OMX_VIDEO_AVCLoopFilterDisable;


    // Initialize the input buffer list
    memset(&(m_sInBufList), 0x0, sizeof(BufferList));

    m_sInBufList.pBufArr = (OMX_BUFFERHEADERTYPE*)malloc(sizeof(OMX_BUFFERHEADERTYPE) *
                                                         m_sInPortDefType.nBufferCountActual);
    if (m_sInBufList.pBufArr == NULL)
    {
        eRet = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    memset(m_sInBufList.pBufArr, 0,
           sizeof(OMX_BUFFERHEADERTYPE) * m_sInPortDefType.nBufferCountActual);
    for (nIndex = 0; nIndex < m_sInPortDefType.nBufferCountActual; nIndex++)
    {
        OMX_CONF_INIT_STRUCT_PTR (&m_sInBufList.pBufArr[nIndex], OMX_BUFFERHEADERTYPE);
    }

    m_sInBufList.pBufHdrList = (OMX_BUFFERHEADERTYPE**)malloc(sizeof(OMX_BUFFERHEADERTYPE*) *
                                                              m_sInPortDefType.nBufferCountActual);
    if (m_sInBufList.pBufHdrList == NULL)
    {
        eRet = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    m_sInBufList.nSizeOfList       = 0;
    m_sInBufList.nAllocSize        = 0;
    m_sInBufList.nWritePos         = 0;
    m_sInBufList.nReadPos          = 0;
    m_sInBufList.nAllocBySelfFlags = 0;
    m_sInBufList.nSizeOfList       = 0;
    m_sInBufList.nBufArrSize       = m_sInPortDefType.nBufferCountActual;
    m_sInBufList.eDir              = OMX_DirInput;

    // Initialize the output buffer list
    memset(&m_sOutBufList, 0x0, sizeof(BufferList));

    m_sOutBufList.pBufArr = (OMX_BUFFERHEADERTYPE*)malloc(sizeof(OMX_BUFFERHEADERTYPE) *
                                                          m_sOutPortDefType.nBufferCountActual);
    if (m_sOutBufList.pBufArr == NULL)
    {
        eRet = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    memset(m_sOutBufList.pBufArr, 0,
           sizeof(OMX_BUFFERHEADERTYPE) * m_sOutPortDefType.nBufferCountActual);
    for (nIndex = 0; nIndex < m_sOutPortDefType.nBufferCountActual; nIndex++)
    {
        OMX_CONF_INIT_STRUCT_PTR(&m_sOutBufList.pBufArr[nIndex], OMX_BUFFERHEADERTYPE);
    }

    m_sOutBufList.pBufHdrList = (OMX_BUFFERHEADERTYPE**)malloc(sizeof(OMX_BUFFERHEADERTYPE*) *
                                 m_sOutPortDefType.nBufferCountActual);
    if (m_sOutBufList.pBufHdrList == NULL)
    {
        eRet = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    m_sOutBufList.nSizeOfList       = 0;
    m_sOutBufList.nAllocSize        = 0;
    m_sOutBufList.nWritePos         = 0;
    m_sOutBufList.nReadPos          = 0;
    m_sOutBufList.nAllocBySelfFlags = 0;
    m_sOutBufList.nSizeOfList       = 0;
    m_sOutBufList.nBufArrSize       = m_sOutPortDefType.nBufferCountActual;
    m_sOutBufList.eDir              = OMX_DirOutput;


    // Create the pipe used to send commands to the thread
    err = pipe(m_cmdpipe);
    if (err)
    {
        eRet = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    // Create the pipe used to send commands to the venc drv thread
    err = pipe(m_venc_cmdpipe);
    if (err)
    {
        eRet = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    // Create the pipe used to send command data to the thread
    err = pipe(m_cmddatapipe);
    if (err)
    {
        eRet = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    // Create the component thread
    err = pthread_create(&m_thread_id, NULL, ComponentThread, this);
    if ( err || !m_thread_id )
    {
        loge("create ComponentThread error!!!");
        eRet = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    // Create venc thread
    err = pthread_create(&m_venc_thread_id, NULL, ComponentVencThread, this);
    if ( err || !m_venc_thread_id )
    {
        loge("create ComponentVencThread error!!!");
        eRet = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    // init some param
    m_useAndroidBuffer = OMX_FALSE;
    m_useMetaDataInBuffers = OMX_FALSE;
    m_prependSPSPPSToIDRFrames = OMX_FALSE;

    memset(&mVideoExtParams, 0, sizeof(OMX_VIDEO_PARAMS_EXTENDED));
    memset(&mVideoSuperFrame, 0, sizeof(OMX_VIDEO_PARAMS_SUPER_FRAME));
    memset(&mVideoSVCSkip, 0, sizeof(OMX_VIDEO_PARAMS_SVC));
    memset(&mVideoVBR, 0, sizeof(OMX_VIDEO_PARAMS_VBR));
    if (strcmp(calling_process, HW_VIDEO_CALL_APK) == 0)
    {
        mVideoSuperFrame.bEnable = OMX_TRUE;
        logd("VideoSuperFrame Enable HW_VIDOE_CALL:%s,F:%s,L:%d",
             HW_VIDEO_CALL_APK,__FUNCTION__,__LINE__);
    }

#if SAVE_BITSTREAM
    OutFile = fopen("/data/camera/bitstream.dat", "wb");
#endif

EXIT:
    return eRet;
}


OMX_ERRORTYPE  omx_venc::get_component_version(OMX_IN OMX_HANDLETYPE pHComp,
                                               OMX_OUT OMX_STRING pComponentName,
                                               OMX_OUT OMX_VERSIONTYPE* pComponentVersion,
                                               OMX_OUT OMX_VERSIONTYPE* pSpecVersion,
                                               OMX_OUT OMX_UUIDTYPE* pComponentUUID)
{

    logv(" COMPONENT_GET_VERSION");
    CEDARC_UNUSE(pComponentUUID);

    if (!pHComp || !pComponentName || !pComponentVersion || !pSpecVersion)
    {
        return OMX_ErrorBadParameter;
    }

    strcpy((char*)pComponentName, (char*)m_cName);

    pComponentVersion->s.nVersionMajor = 1;
    pComponentVersion->s.nVersionMinor = 1;
    pComponentVersion->s.nRevision     = 0;
    pComponentVersion->s.nStep         = 0;

    pSpecVersion->s.nVersionMajor = 1;
    pSpecVersion->s.nVersionMinor = 1;
    pSpecVersion->s.nRevision     = 0;
    pSpecVersion->s.nStep         = 0;

    return OMX_ErrorNone;
}


OMX_ERRORTYPE  omx_venc::send_command(OMX_IN OMX_HANDLETYPE  pHComp,
                                      OMX_IN OMX_COMMANDTYPE eCmd,
                                      OMX_IN OMX_U32         uParam1,
                                      OMX_IN OMX_PTR         pCmdData)
{
    ThrCmdType    eCmdNative;
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    CEDARC_UNUSE(pHComp);

    if (m_state == OMX_StateInvalid)
    {
        loge("ERROR: Send Command in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    if (eCmd == OMX_CommandMarkBuffer && pCmdData == NULL)
    {
        loge("ERROR: Send OMX_CommandMarkBuffer command but pCmdData invalid.");
        return OMX_ErrorBadParameter;
    }

    switch (eCmd)
    {
        case OMX_CommandStateSet:
        {
            logv(" COMPONENT_SEND_COMMAND: OMX_CommandStateSet");
            eCmdNative = SetState;
            break;
        }

        case OMX_CommandFlush:
        {
            logv(" COMPONENT_SEND_COMMAND: OMX_CommandFlush");
            eCmdNative = Flush;
            if ((int)uParam1 > 1 && (int)uParam1 != -1)
            {
                loge("Error: Send OMX_CommandFlush command but uParam1 invalid.");
                return OMX_ErrorBadPortIndex;
            }
            break;
         }

        case OMX_CommandPortDisable:
        {
            logv(" COMPONENT_SEND_COMMAND: OMX_CommandPortDisable");
            eCmdNative = StopPort;
            break;
        }

        case OMX_CommandPortEnable:
        {
            logv(" COMPONENT_SEND_COMMAND: OMX_CommandPortEnable");
            eCmdNative = RestartPort;
            break;
        }

        case OMX_CommandMarkBuffer:
        {
            logv(" COMPONENT_SEND_COMMAND: OMX_CommandMarkBuffer");
            eCmdNative = MarkBuf;
             if (uParam1 > 0)
            {
                loge("Error: Send OMX_CommandMarkBuffer command but uParam1 invalid.");
                return OMX_ErrorBadPortIndex;
            }
            break;
        }

        default:
        {
            return OMX_ErrorBadPortIndex;
        }
    }

    post_event(eCmdNative, uParam1, pCmdData);

    return eError;
}


OMX_ERRORTYPE  omx_venc::get_parameter(OMX_IN OMX_HANDLETYPE pHComp,
                                       OMX_IN OMX_INDEXTYPE  eParamIndex,
                                       OMX_INOUT OMX_PTR     pParamData)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    CEDARC_UNUSE(pHComp);

    if (m_state == OMX_StateInvalid)
    {
        logv("Get Param in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    if (pParamData == NULL)
    {
        logv("Get Param in Invalid pParamData \n");
        return OMX_ErrorBadParameter;
    }

    switch (eParamIndex)
    {
        case OMX_IndexParamVideoInit:
        {
            logv(" COMPONENT_GET_PARAMETER: OMX_IndexParamVideoInit");
            memcpy(pParamData, &m_sPortParam, sizeof(OMX_PORT_PARAM_TYPE));
            break;
        }

        case OMX_IndexParamPortDefinition:
        {
            // android::CallStack stack;
            // stack.update(1, 100);
            // stack.dump("get_parameter");
            logv(" COMPONENT_GET_PARAMETER: OMX_IndexParamPortDefinition");
            if (((OMX_PARAM_PORTDEFINITIONTYPE *)(pParamData))->nPortIndex ==
                 m_sInPortDefType.nPortIndex)
            {
                memcpy(pParamData, &m_sInPortDefType, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
            }
            else if (((OMX_PARAM_PORTDEFINITIONTYPE*)(pParamData))->nPortIndex ==
                     m_sOutPortDefType.nPortIndex)
            {
                memcpy(pParamData, &m_sOutPortDefType, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
                logv(" width = %d, height = %d", (int)m_sOutPortDefType.format.video.nFrameWidth,
                     (int)m_sOutPortDefType.format.video.nFrameHeight);
            }
            else
            {
                eError = OMX_ErrorBadPortIndex;
            }
            break;
        }

        case OMX_IndexParamVideoPortFormat:
        {
            logv(" COMPONENT_GET_PARAMETER: OMX_IndexParamVideoPortFormat");

            OMX_VIDEO_PARAM_PORTFORMATTYPE * param_portformat_type =
                (OMX_VIDEO_PARAM_PORTFORMATTYPE *)(pParamData);
            if (param_portformat_type->nPortIndex == m_sInPortFormatType.nPortIndex)
            {
                if (param_portformat_type->nIndex > m_sInPortFormatType.nIndex)
                {
                    eError = OMX_ErrorNoMore;
                }
                else
                {
                    param_portformat_type->eCompressionFormat = (OMX_VIDEO_CODINGTYPE)0;
                    param_portformat_type->eColorFormat =
                        m_inputcolorFormats[param_portformat_type->nIndex];
                }
            }
            else if (((OMX_VIDEO_PARAM_PORTFORMATTYPE*)(pParamData))->nPortIndex ==
                     m_sOutPortFormatType.nPortIndex)
            {
                if (((OMX_VIDEO_PARAM_PORTFORMATTYPE*)(pParamData))->nIndex >
                    m_sOutPortFormatType.nIndex)
                {
                    eError = OMX_ErrorNoMore;
                }
                else
                {
                    memcpy(pParamData, &m_sOutPortFormatType,
                           sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
                }
            }
            else
            {
                eError = OMX_ErrorBadPortIndex;
            }
            break;
        }

        case OMX_IndexParamStandardComponentRole:
        {
            logv(" COMPONENT_GET_PARAMETER: OMX_IndexParamStandardComponentRole");
            OMX_PARAM_COMPONENTROLETYPE* comp_role;
            comp_role                    = (OMX_PARAM_COMPONENTROLETYPE *) pParamData;
            comp_role->nVersion.nVersion = OMX_SPEC_VERSION;
            comp_role->nSize             = sizeof(*comp_role);
            strncpy((char*)comp_role->cRole, (const char*)m_cRole, OMX_MAX_STRINGNAME_SIZE);
            break;
        }

        case OMX_IndexParamPriorityMgmt:
        {
            logv(" COMPONENT_GET_PARAMETER: OMX_IndexParamPriorityMgmt");
            memcpy(pParamData, &m_sPriorityMgmtType, sizeof(OMX_PRIORITYMGMTTYPE));
            break;
        }

        case OMX_IndexParamCompBufferSupplier:
        {
            logv(" COMPONENT_GET_PARAMETER: OMX_IndexParamCompBufferSupplier");
            OMX_PARAM_BUFFERSUPPLIERTYPE* pBuffSupplierParam =
                (OMX_PARAM_BUFFERSUPPLIERTYPE*)pParamData;
            if (pBuffSupplierParam->nPortIndex == 1)
            {
                pBuffSupplierParam->eBufferSupplier = m_sOutBufSupplierType.eBufferSupplier;
            }
            else if (pBuffSupplierParam->nPortIndex == 0)
            {
                pBuffSupplierParam->eBufferSupplier = m_sInBufSupplierType.eBufferSupplier;
            }
            else
            {
                eError = OMX_ErrorBadPortIndex;
            }
            break;
        }

        case OMX_IndexParamVideoBitrate:
        {
            logv(" COMPONENT_GET_PARAMETER: OMX_IndexParamVideoBitrate");
            if (((OMX_VIDEO_PARAM_BITRATETYPE*)(pParamData))->nPortIndex ==
                m_sOutPutBitRateType.nPortIndex)
            {
                memcpy(pParamData,&m_sOutPutBitRateType, sizeof(OMX_VIDEO_PARAM_BITRATETYPE));
            }
            else
            {
                eError = OMX_ErrorBadPortIndex;
            }
            break;
        }

        case OMX_IndexParamVideoAvc:
        {
            logv(" COMPONENT_GET_PARAMETER: OMX_IndexParamVideoAvc");
            OMX_VIDEO_PARAM_AVCTYPE* pComponentParam = (OMX_VIDEO_PARAM_AVCTYPE*)pParamData;

            if (pComponentParam->nPortIndex == m_sH264Type.nPortIndex)
            {
                memcpy(pComponentParam, &m_sH264Type, sizeof(OMX_VIDEO_PARAM_AVCTYPE));
            }
            else
            {
                eError = OMX_ErrorBadPortIndex;
            }
            break;
        }


        case OMX_IndexParamAudioInit:
        {
            logv(" COMPONENT_GET_PARAMETER: OMX_IndexParamAudioInit");
            OMX_PORT_PARAM_TYPE *audioPortParamType = (OMX_PORT_PARAM_TYPE *) pParamData;

            audioPortParamType->nVersion.nVersion = OMX_SPEC_VERSION;
            audioPortParamType->nSize             = sizeof(OMX_PORT_PARAM_TYPE);
            audioPortParamType->nPorts            = 0;
            audioPortParamType->nStartPortNumber  = 0;

            break;
        }

        case OMX_IndexParamImageInit:
        {
            logv(" COMPONENT_GET_PARAMETER: OMX_IndexParamImageInit");
            OMX_PORT_PARAM_TYPE *imagePortParamType = (OMX_PORT_PARAM_TYPE *) pParamData;

            imagePortParamType->nVersion.nVersion = OMX_SPEC_VERSION;
            imagePortParamType->nSize             = sizeof(OMX_PORT_PARAM_TYPE);
            imagePortParamType->nPorts            = 0;
            imagePortParamType->nStartPortNumber  = 0;
            break;
        }

        case OMX_IndexParamOtherInit:
        {
            logv(" COMPONENT_GET_PARAMETER: OMX_IndexParamOtherInit");
            OMX_PORT_PARAM_TYPE *otherPortParamType = (OMX_PORT_PARAM_TYPE *) pParamData;

            otherPortParamType->nVersion.nVersion = OMX_SPEC_VERSION;
            otherPortParamType->nSize             = sizeof(OMX_PORT_PARAM_TYPE);
            otherPortParamType->nPorts            = 0;
            otherPortParamType->nStartPortNumber  = 0;
            break;
        }

        case OMX_IndexParamVideoH263:
        {
            logv(" COMPONENT_GET_PARAMETER: OMX_IndexParamVideoH263");
            logv("get_parameter: OMX_IndexParamVideoH263, do nothing.\n");
            break;
        }

        case OMX_IndexParamVideoMpeg4:
        {
            logv(" COMPONENT_GET_PARAMETER: OMX_IndexParamVideoMpeg4");
            logv("get_parameter: OMX_IndexParamVideoMpeg4, do nothing.\n");
            break;
        }
        case OMX_IndexParamVideoProfileLevelQuerySupported:
        {
            VIDEO_PROFILE_LEVEL_TYPE* pProfileLevel = NULL;
            OMX_U32 nNumberOfProfiles = 0;
            OMX_VIDEO_PARAM_PROFILELEVELTYPE *pParamProfileLevel =
                (OMX_VIDEO_PARAM_PROFILELEVELTYPE *)pParamData;

            pParamProfileLevel->nPortIndex = m_sOutPortDefType.nPortIndex;

            /* Choose table based on compression format */
            switch (m_sOutPortDefType.format.video.eCompressionFormat)
            {
                case OMX_VIDEO_CodingAVC:
                {
                    pProfileLevel = SupportedAVCProfileLevels;
                    nNumberOfProfiles = sizeof(SupportedAVCProfileLevels) /
                                        sizeof (VIDEO_PROFILE_LEVEL_TYPE);
                    break;
                }
                case OMX_VIDEO_CodingH263:
                {
                    pProfileLevel = SupportedH263ProfileLevels;
                    nNumberOfProfiles = sizeof(SupportedH263ProfileLevels) /
                                        sizeof (VIDEO_PROFILE_LEVEL_TYPE);
                    break;
                }
                default:
                {
                    return OMX_ErrorBadParameter;
                }
            }

            if (pParamProfileLevel->nProfileIndex >= (nNumberOfProfiles - 1))
            {
                return OMX_ErrorBadParameter;
            }

            /* Point to table entry based on index */
            pProfileLevel += pParamProfileLevel->nProfileIndex;

            /* -1 indicates end of table */
            if (pProfileLevel->nProfile != -1)
            {
                pParamProfileLevel->eProfile = pProfileLevel->nProfile;
                pParamProfileLevel->eLevel = pProfileLevel->nLevel;
                eError = OMX_ErrorNone;
            }
            else
            {
                eError = OMX_ErrorNoMore;
            }
            break;
        }

#ifdef CONF_MARSHMALLOW_AND_NEWER
        case OMX_IndexParamConsumerUsageBits:
        {
            OMX_U32 *usageBits = (OMX_U32 *)pParamData;
            *usageBits = GRALLOC_USAGE_HW_2D;
            break;
        }
#endif

        default:
        {
            switch ((VIDENC_CUSTOM_INDEX)eParamIndex)
            {
                 case VideoEncodeCustomParamextendedVideo:
                {
                    logd("get VideoEncodeCustomParamextendedVideo");
                    memcpy(pParamData,&mVideoExtParams,  sizeof(OMX_VIDEO_PARAMS_EXTENDED));
                    break;
                }
                case VideoEncodeCustomParamextendedVideoSuperframe:
                {
                    logd("get VideoEncodeCustomParamextendedVideoSuperframe");
                    memcpy(pParamData,&mVideoSuperFrame,  sizeof(OMX_VIDEO_PARAMS_SUPER_FRAME));
                    break;
                }
                case VideoEncodeCustomParamextendedVideoSVCSkip:
                {
                    logd("get VideoEncodeCustomParamextendedVideoSVCSkip");
                    memcpy(pParamData, &mVideoSVCSkip,  sizeof(OMX_VIDEO_PARAMS_SVC));
                    break;
                }
                case VideoEncodeCustomParamextendedVideoVBR:
                {
                    logd("get VideoEncodeCustomParamextendedVideoVBR\n");
                    memcpy(pParamData, &mVideoVBR, sizeof(OMX_VIDEO_PARAMS_VBR));
                    break;
                }
                case VideoEncodeCustomParamStoreANWBufferInMetadata:
                {
                    break;
                }
                case VideoEncodeCustomParamextendedVideoPSkip:
                {
                    logd("get VideoEncodeCustomParamextendedVideoPSkip\n");
                    break;
                }
                default:
                {
                    loge("getparameter: unknown param %p\n", pParamData);
                    eError = OMX_ErrorUnsupportedIndex;
                    break;
                }
            }
            break;
        }
    }

    return eError;
}


OMX_ERRORTYPE  omx_venc::set_parameter(OMX_IN OMX_HANDLETYPE pHComp,
                                       OMX_IN OMX_INDEXTYPE eParamIndex,
                                       OMX_IN OMX_PTR pParamData)
{
    OMX_ERRORTYPE         eError      = OMX_ErrorNone;

    CEDARC_UNUSE(pHComp);

    if (m_state == OMX_StateInvalid)
    {
        logv("Set Param in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    if (pParamData == NULL)
    {
        logv("Get Param in Invalid pParamData \n");
        return OMX_ErrorBadParameter;
    }

   logv("set_parameter, eParamIndex: %x", eParamIndex);

    switch (eParamIndex)
    {
        case OMX_IndexParamPortDefinition:
        {
            logv(" COMPONENT_SET_PARAMETER: OMX_IndexParamPortDefinition");

            if (((OMX_PARAM_PORTDEFINITIONTYPE *)(pParamData))->nPortIndex ==
                m_sInPortDefType.nPortIndex)
            {
                logv("in, nPortIndex: %d",
                     (int)(((OMX_PARAM_PORTDEFINITIONTYPE *)(pParamData))->nBufferCountActual));
                if (((OMX_PARAM_PORTDEFINITIONTYPE *)(pParamData))->nBufferCountActual !=
                   m_sInPortDefType.nBufferCountActual)
                {
                    int nBufCnt;
                    int nIndex;


                    pthread_mutex_lock(&m_inBufMutex);

                    if (m_sInBufList.pBufArr != NULL)
                        free(m_sInBufList.pBufArr);

                    if (m_sInBufList.pBufHdrList != NULL)
                        free(m_sInBufList.pBufHdrList);

                    nBufCnt = ((OMX_PARAM_PORTDEFINITIONTYPE *)(pParamData))->nBufferCountActual;
                    logv(" allocate %d buffers.", (int)nBufCnt);

                    m_sInBufList.pBufArr =
                        (OMX_BUFFERHEADERTYPE*)malloc(sizeof(OMX_BUFFERHEADERTYPE)* nBufCnt);
                    m_sInBufList.pBufHdrList =
                        (OMX_BUFFERHEADERTYPE**)malloc(sizeof(OMX_BUFFERHEADERTYPE*)* nBufCnt);
                    for (nIndex = 0; nIndex < nBufCnt; nIndex++)
                    {
                        OMX_CONF_INIT_STRUCT_PTR (&m_sInBufList.pBufArr[nIndex],
                                                  OMX_BUFFERHEADERTYPE);
                    }

                    m_sInBufList.nSizeOfList       = 0;
                    m_sInBufList.nAllocSize        = 0;
                    m_sInBufList.nWritePos         = 0;
                    m_sInBufList.nReadPos          = 0;
                    m_sInBufList.nAllocBySelfFlags = 0;
                    m_sInBufList.nSizeOfList       = 0;
                    m_sInBufList.nBufArrSize       = nBufCnt;
                    m_sInBufList.eDir              = OMX_DirInput;

                    pthread_mutex_unlock(&m_inBufMutex);
                }
                memcpy(&m_sInPortDefType, pParamData, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
                logd("init_input_port: stride = %d, width = %d, height = %d",
                     (int)m_sInPortDefType.format.video.nStride,
                     (int)m_sInPortDefType.format.video.nFrameWidth,
                     (int)m_sInPortDefType.format.video.nFrameHeight);
            }
            else if (((OMX_PARAM_PORTDEFINITIONTYPE *)(pParamData))->nPortIndex ==
                     m_sOutPortDefType.nPortIndex)
            {
                logv("out, nPortIndex: %d",
                     (int)((OMX_PARAM_PORTDEFINITIONTYPE *)(pParamData))->nBufferCountActual);
                if (((OMX_PARAM_PORTDEFINITIONTYPE *)(pParamData))->nBufferCountActual !=
                    m_sOutPortDefType.nBufferCountActual)
                {
                    int nBufCnt;
                    int nIndex;

                    pthread_mutex_lock(&m_outBufMutex);

                    if (m_sOutBufList.pBufArr != NULL)
                    {
                        free(m_sOutBufList.pBufArr);
                    }

                    if (m_sOutBufList.pBufHdrList != NULL)
                    {
                        free(m_sOutBufList.pBufHdrList);
                    }

                    nBufCnt = ((OMX_PARAM_PORTDEFINITIONTYPE *)(pParamData))->nBufferCountActual;
                    logv(" allocate %d buffers.", (int)nBufCnt);
                    // Initialize the output buffer list
                    m_sOutBufList.pBufArr =
                        (OMX_BUFFERHEADERTYPE*)malloc(sizeof(OMX_BUFFERHEADERTYPE) * nBufCnt);
                    m_sOutBufList.pBufHdrList =
                        (OMX_BUFFERHEADERTYPE**) malloc(sizeof(OMX_BUFFERHEADERTYPE*) * nBufCnt);
                    for (nIndex = 0; nIndex < nBufCnt; nIndex++)
                    {
                        OMX_CONF_INIT_STRUCT_PTR (&m_sOutBufList.pBufArr[nIndex],
                                                  OMX_BUFFERHEADERTYPE);
                    }

                    m_sOutBufList.nSizeOfList       = 0;
                    m_sOutBufList.nAllocSize        = 0;
                    m_sOutBufList.nWritePos         = 0;
                    m_sOutBufList.nReadPos          = 0;
                    m_sOutBufList.nAllocBySelfFlags = 0;
                    m_sOutBufList.nSizeOfList       = 0;
                    m_sOutBufList.nBufArrSize       = nBufCnt;
                    m_sOutBufList.eDir              = OMX_DirOutput;

                    pthread_mutex_unlock(&m_outBufMutex);
                }
                memcpy(&m_sOutPortDefType, pParamData, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
                m_sOutPortDefType.nBufferSize =
                    m_sOutPortDefType.format.video.nFrameWidth *
                    m_sOutPortDefType.format.video.nFrameHeight * 3 / 2;
                m_sOutPortDefType.nBufferSize = \
                    omx_venc_align(m_sOutPortDefType.nBufferSize, BUF_ALIGN_SIZE);

                m_framerate = (m_sInPortDefType.format.video.xFramerate);
                logd("m_framerate: %d", (int)m_framerate);
            }
            else
                eError = OMX_ErrorBadPortIndex;
            break;
        }

        case OMX_IndexParamVideoPortFormat:
        {
            logv(" COMPONENT_SET_PARAMETER: OMX_IndexParamVideoPortFormat");

            OMX_VIDEO_PARAM_PORTFORMATTYPE * param_portformat_type =
                (OMX_VIDEO_PARAM_PORTFORMATTYPE *)(pParamData);
             if (param_portformat_type->nPortIndex == m_sInPortFormatType.nPortIndex)
             {
                 if (param_portformat_type->nIndex > m_sInPortFormatType.nIndex)
                 {
                       eError = OMX_ErrorNoMore;
                 }
                else
                {
                    m_sInPortFormatType.eColorFormat = param_portformat_type->eColorFormat;
                    m_sInPortDefType.format.video.eColorFormat = m_sInPortFormatType.eColorFormat;
                }
            }
            else if (((OMX_VIDEO_PARAM_PORTFORMATTYPE*)(pParamData))->nPortIndex ==
                     m_sOutPortFormatType.nPortIndex)
            {
                if (((OMX_VIDEO_PARAM_PORTFORMATTYPE*)(pParamData))->nIndex >
                    m_sOutPortFormatType.nIndex)
                {
                    eError = OMX_ErrorNoMore;
                }
                else
                {
                    memcpy(&m_sOutPortFormatType, pParamData,
                           sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
                }
            }
            else
            {
                eError = OMX_ErrorBadPortIndex;
            }
            break;
        }

        case OMX_IndexParamStandardComponentRole:
        {
            logv(" COMPONENT_SET_PARAMETER: OMX_IndexParamStandardComponentRole");
            OMX_PARAM_COMPONENTROLETYPE *comp_role;
            comp_role = (OMX_PARAM_COMPONENTROLETYPE *) pParamData;
            logv("set_parameter: OMX_IndexParamStandardComponentRole %s\n", comp_role->cRole);

            if ((m_state == OMX_StateLoaded)
               /* && !BITMASK_PRESENT(&m_flags,OMX_COMPONENT_IDLE_PENDING)*/)
            {
                logv("Set Parameter called in valid state");
            }
            else
            {
                logv("Set Parameter called in Invalid State\n");
                return OMX_ErrorIncorrectStateOperation;
            }

            if (!strncmp((char*)m_cName, "OMX.allwinner.video.encoder.avc",
                         OMX_MAX_STRINGNAME_SIZE))
            {
                if (!strncmp((char*)comp_role->cRole, "video_encoder.avc",
                             OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"video_encoder.avc", OMX_MAX_STRINGNAME_SIZE);
                }
                else
                {
                    logv("Setparameter: unknown Index %s\n", comp_role->cRole);
                    eError =OMX_ErrorUnsupportedSetting;
                }
            }
            else if (!strncmp((char*)m_cName, "OMX.allwinner.video.encoder.mjpeg",
                              OMX_MAX_STRINGNAME_SIZE))
            {
                if (!strncmp((char*)comp_role->cRole, "video_encoder.mjpeg",
                             OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"video_encoder.mjpeg", OMX_MAX_STRINGNAME_SIZE);
                }
                else
                {
                    logv("Setparameter: unknown Index %s\n", comp_role->cRole);
                    eError =OMX_ErrorUnsupportedSetting;
                }
            }
            else
            {
                logv("Setparameter: unknown param %s\n", m_cName);
                eError = OMX_ErrorInvalidComponentName;
            }
            break;
        }

        case OMX_IndexParamPriorityMgmt:
        {
            logv(" COMPONENT_SET_PARAMETER: OMX_IndexParamPriorityMgmt");
            if (m_state != OMX_StateLoaded)
            {
                logv("Set Parameter called in Invalid State\n");
                return OMX_ErrorIncorrectStateOperation;
            }
            OMX_PRIORITYMGMTTYPE *priorityMgmtype = (OMX_PRIORITYMGMTTYPE*) pParamData;
            m_sPriorityMgmtType.nGroupID = priorityMgmtype->nGroupID;
            m_sPriorityMgmtType.nGroupPriority = priorityMgmtype->nGroupPriority;
            break;
        }

        case OMX_IndexParamCompBufferSupplier:
        {
            logv(" COMPONENT_SET_PARAMETER: OMX_IndexParamCompBufferSupplier");
            OMX_PARAM_BUFFERSUPPLIERTYPE *bufferSupplierType =
                (OMX_PARAM_BUFFERSUPPLIERTYPE*) pParamData;
            logv("set_parameter: OMX_IndexParamCompBufferSupplier %d\n",
                 bufferSupplierType->eBufferSupplier);
            if (bufferSupplierType->nPortIndex == 0)
                m_sInBufSupplierType.eBufferSupplier = bufferSupplierType->eBufferSupplier;
            else if (bufferSupplierType->nPortIndex == 1)
                m_sOutBufSupplierType.eBufferSupplier = bufferSupplierType->eBufferSupplier;
            else
                eError = OMX_ErrorBadPortIndex;
            break;
        }

        case OMX_IndexParamVideoBitrate:
        {
            logv(" COMPONENT_SET_PARAMETER: OMX_IndexParamVideoBitrate");
            OMX_VIDEO_PARAM_BITRATETYPE* pComponentParam =
                (OMX_VIDEO_PARAM_BITRATETYPE*)pParamData;
            if (pComponentParam->nPortIndex == m_sOutPutBitRateType.nPortIndex)
            {
                memcpy(&m_sOutPutBitRateType,pComponentParam, sizeof(OMX_VIDEO_PARAM_BITRATETYPE));

                if (!m_sOutPutBitRateType.nTargetBitrate)
                {
                    m_sOutPutBitRateType.nTargetBitrate = DEFAULT_BITRATE;
                }

                m_sOutPortDefType.format.video.nBitrate = m_sOutPutBitRateType.nTargetBitrate;

                if (m_state == OMX_StateExecuting && m_encoder)
                {
                    post_message_to_venc(this, OMX_Venc_Cmd_ChangeBitrate);
                }
            }
            else
            {
                eError = OMX_ErrorBadPortIndex;
            }
            break;
        }

        case OMX_IndexParamVideoAvc:
        {
            logv(" COMPONENT_SET_PARAMETER: OMX_IndexParamVideoAvc");
            OMX_VIDEO_PARAM_AVCTYPE* pComponentParam = (OMX_VIDEO_PARAM_AVCTYPE*)pParamData;

            if (pComponentParam->nPortIndex == m_sH264Type.nPortIndex)
            {
                memcpy(&m_sH264Type,pComponentParam, sizeof(OMX_VIDEO_PARAM_AVCTYPE));
                //CalculateBufferSize(pCompPortOut->pPortDef, pComponentPrivate);
            }
            else
            {
                eError = OMX_ErrorBadPortIndex;
            }
            break;
        }

        case OMX_IndexParamVideoH263:
        {
            logv(" COMPONENT_SET_PARAMETER: OMX_IndexParamVideoH263");
            logv("set_parameter: OMX_IndexParamVideoH263, do nothing.\n");
            break;
        }

        case OMX_IndexParamVideoMpeg4:
        {
            logv(" COMPONENT_SET_PARAMETER: OMX_IndexParamVideoMpeg4");
             logv("set_parameter: OMX_IndexParamVideoMpeg4, do nothing.\n");
            break;
        }
        case OMX_IndexParamVideoIntraRefresh:
        {
            OMX_VIDEO_PARAM_INTRAREFRESHTYPE* pComponentParam =
                (OMX_VIDEO_PARAM_INTRAREFRESHTYPE*)pParamData;
            if (pComponentParam->nPortIndex == 1 &&
               pComponentParam->eRefreshMode == OMX_VIDEO_IntraRefreshCyclic)
            {
                int mbWidth, mbHeight;
                mbWidth = (m_sInPortDefType.format.video.nFrameWidth + 15)/16;
                mbHeight = (m_sInPortDefType.format.video.nFrameHeight + 15)/16;
                m_vencCyclicIntraRefresh.bEnable = 1;
                m_vencCyclicIntraRefresh.nBlockNumber = mbWidth*mbHeight/pComponentParam->nCirMBs;
                logd("m_vencCyclicIntraRefresh.nBlockNumber: %d",
                     m_vencCyclicIntraRefresh.nBlockNumber);
            }
            break;
        }

        default:
        {
#ifdef __ANDROID__

            switch ((VIDENC_CUSTOM_INDEX)eParamIndex)
            {
                case VideoEncodeCustomParamStoreMetaDataInBuffers:
                {
                    OMX_BOOL bFlag = ((StoreMetaDataInBuffersParams*)pParamData)->bStoreMetaData;;
                    if (((StoreMetaDataInBuffersParams*)pParamData)->nPortIndex == 0)
                    {
                        m_useMetaDataInBuffers = bFlag;
                    }
                    else
                    {
                        if (bFlag)
                        {
                            eError = OMX_ErrorUnsupportedSetting;
                        }
                    }
                    logd(" COMPONENT_SET_PARAMETER: \
                             VideoEncodeCustomParamStoreMetaDataInBuffers %d",
                         m_useMetaDataInBuffers);
                    break;
                }

                case VideoEncodeCustomParamPrependSPSPPSToIDRFrames:
                {
                    m_prependSPSPPSToIDRFrames =
                        ((PrependSPSPPSToIDRFramesParams*)pParamData)->bEnable;
                    logd(" COMPONENT_SET_PARAMETER: \
                             VideoEncodeCustomParamPrependSPSPPSToIDRFrames %d",
                         m_prependSPSPPSToIDRFrames);
                    break;
                }

                case VideoEncodeCustomParamEnableAndroidNativeBuffers:
                {
                    OMX_BOOL bFlag = ((EnableAndroidNativeBuffersParams*)pParamData)->enable;
                    OMX_U32  index = ((EnableAndroidNativeBuffersParams*)pParamData)->nPortIndex;

                    logd("COMPONENT_SET_PARAMETER: \
                         VideoEncodeCustomParamEnableAndroidNativeBuffers, \
                         nPortIndex: %d,enable:%d", (int)index, (int)bFlag);
                    if(index == 0)
                        m_useAndroidBuffer = bFlag;

                    break;
                }
                case VideoEncodeCustomParamextendedVideo:
                {
                    logd("set VideoEncodeCustomParamextendedVideo");
                    memcpy(&mVideoExtParams, pParamData, sizeof(OMX_VIDEO_PARAMS_EXTENDED));
                    break;
                }
                case VideoEncodeCustomParamextendedVideoSuperframe:
                {
                    logd("set VideoEncodeCustomParamextendedVideoSuperframe");
                    memcpy(&mVideoSuperFrame, pParamData, sizeof(OMX_VIDEO_PARAMS_SUPER_FRAME));
                    break;
                }
                case VideoEncodeCustomParamextendedVideoSVCSkip:
                {
                    logd("set VideoEncodeCustomParamextendedVideoSVCSkip\n");
                    memcpy(&mVideoSVCSkip, pParamData, sizeof(OMX_VIDEO_PARAMS_SVC));
                    break;
                }
                case VideoEncodeCustomParamextendedVideoVBR:
                {
                    logd("set VideoEncodeCustomParamextendedVideoVBR\n");
                    memcpy(&mVideoVBR, pParamData, sizeof(OMX_VIDEO_PARAMS_VBR));
                    break;
                }
                case VideoEncodeCustomParamStoreANWBufferInMetadata:
                {
                    OMX_BOOL bFlag = ((StoreMetaDataInBuffersParams*)pParamData)->bStoreMetaData;
                    OMX_U32  index = ((StoreMetaDataInBuffersParams*)pParamData)->nPortIndex;
                    logd("=== VideoEncodeCustomParamStoreANWBufferInMetadata, flag: %d", bFlag);
                    if(index == 0)
                        m_useAndroidBuffer = bFlag;
                    break;
                }
                case VideoEncodeCustomParamextendedVideoPSkip:
                {
                    logd("set VideoEncodeCustomParamextendedVideoPSkip\n");
                    m_usePSkip = (*(OMX_BOOL *)pParamData);
                    break;
                }
                default:
                {
                    loge("Setparameter: unknown param %x\n", eParamIndex);
                    eError = OMX_ErrorUnsupportedIndex;
                    break;
                }
            }
#else
            eError = OMX_ErrorUnsupportedIndex;
#endif
        }
    }

    return eError;
}

OMX_ERRORTYPE  omx_venc::get_config(OMX_IN OMX_HANDLETYPE pHComp,
                                    OMX_IN OMX_INDEXTYPE eConfigIndex,
                                    OMX_INOUT OMX_PTR pConfigData)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    logv(" COMPONENT_GET_CONFIG: index = %d", eConfigIndex);
    CEDARC_UNUSE(pHComp);
    CEDARC_UNUSE(pConfigData);

    if (m_state == OMX_StateInvalid)
    {
        logv("get_config in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    switch (eConfigIndex)
    {
        default:
        {
            logv("get_config: unknown param %d\n",eConfigIndex);
            eError = OMX_ErrorUnsupportedIndex;
        }
    }

    return eError;
}

OMX_ERRORTYPE omx_venc::set_config(OMX_IN OMX_HANDLETYPE pHComp,
                                   OMX_IN OMX_INDEXTYPE eConfigIndex,
                                   OMX_IN OMX_PTR pConfigData)
{
    logv(" COMPONENT_SET_CONFIG: index = %d", eConfigIndex);

    CEDARC_UNUSE(pHComp);

    if (m_state == OMX_StateInvalid)
    {
        logv("set_config in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    OMX_ERRORTYPE eError = OMX_ErrorNone;

    switch (eConfigIndex)
    {
        case OMX_IndexConfigVideoIntraVOPRefresh:
        {
            if (m_state == OMX_StateExecuting && m_encoder)
            {
                post_message_to_venc(this, OMX_Venc_Cmd_RequestIDRFrame);
                logd("venc, OMX_Venc_Cmd_RequestIDRFrame");
            }
            break;
        }
        case OMX_IndexConfigVideoBitrate:
        {
            OMX_VIDEO_CONFIG_BITRATETYPE* pData = (OMX_VIDEO_CONFIG_BITRATETYPE*)(pConfigData);

            if (pData->nPortIndex == 1)
            {
                if (m_state == OMX_StateExecuting && m_encoder)
                {
                    m_sOutPortDefType.format.video.nBitrate = pData->nEncodeBitrate;
                    post_message_to_venc(this, OMX_Venc_Cmd_ChangeBitrate);
                    logv("FUNC:%s, LINE:%d , pData->nEncodeBitrate = %d",
                         __FUNCTION__,__LINE__,pData->nEncodeBitrate);
                }
            }
            break;
        }
        case OMX_IndexConfigVideoFramerate:
        {
            OMX_CONFIG_FRAMERATETYPE* pData = (OMX_CONFIG_FRAMERATETYPE*)(pConfigData);
            logd("Microphone, FUNC:%s, LINE:%d", __FUNCTION__,__LINE__);
            if (pData->nPortIndex == 0)
            {
                if (m_state == OMX_StateExecuting && m_encoder)
                {
                    m_sInPortDefType.format.video.xFramerate = pData->xEncodeFramerate;
                    m_framerate = (m_sInPortDefType.format.video.xFramerate >> 16);
                    post_message_to_venc(this, OMX_Venc_Cmd_ChangeFramerate);
                    logv("FUNC:%s, LINE:%d , pData->xEncodeFramerate = %d",
                         __FUNCTION__,__LINE__,m_framerate);
                }
            }
            break;
        }

        default:
        {
            logv("get_config: unknown param %d\n",eConfigIndex);
            eError = OMX_ErrorUnsupportedIndex;
            break;
        }
    }

    return eError;
}

OMX_ERRORTYPE  omx_venc::get_extension_index(OMX_IN OMX_HANDLETYPE pHComp,
                                             OMX_IN OMX_STRING pParamName,
                                             OMX_OUT OMX_INDEXTYPE* pIndexType)
{
    unsigned int  nIndex;
    OMX_ERRORTYPE eError = OMX_ErrorUndefined;

    logv(" COMPONENT_GET_EXTENSION_INDEX: param name = %s", pParamName);
    if (m_state == OMX_StateInvalid)
    {
        logv("Get Extension Index in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    if (pHComp == NULL)
    {
        return OMX_ErrorBadParameter;
    }

	if(strcmp((char *)pParamName, "OMX.google.android.index.storeANWBufferInMetadata")
            == 0)
	{
        logw("do not support OMX.google.android.index.storeANWBufferInMetadata\n");
        return OMX_ErrorUnsupportedIndex;
	}

    for (nIndex = 0; nIndex < sizeof(sVideoEncCustomParams)/sizeof(VIDDEC_CUSTOM_PARAM); nIndex++)
    {
        if (strcmp((char *)pParamName, (char *)&(sVideoEncCustomParams[nIndex].cCustomParamName))
            == 0)
        {
            *pIndexType = sVideoEncCustomParams[nIndex].nCustomParamIndex;
            eError = OMX_ErrorNone;
            break;
        }
    }
    return eError;
}

OMX_ERRORTYPE omx_venc::get_state(OMX_IN OMX_HANDLETYPE pHComp, OMX_OUT OMX_STATETYPE* pState)
{
    logv(" COMPONENT_GET_STATE");

    if (pHComp == NULL || pState == NULL)
    {
        return OMX_ErrorBadParameter;
    }

    *pState = m_state;
    return OMX_ErrorNone;
}


OMX_ERRORTYPE omx_venc::component_tunnel_request(OMX_IN    OMX_HANDLETYPE       pHComp,
                                                 OMX_IN    OMX_U32              uPort,
                                                 OMX_IN    OMX_HANDLETYPE       pPeerComponent,
                                                 OMX_IN    OMX_U32              uPeerPort,
                                                 OMX_INOUT OMX_TUNNELSETUPTYPE* pTunnelSetup)
{
    logv(" COMPONENT_TUNNEL_REQUEST");

    CEDARC_UNUSE(pHComp);
    CEDARC_UNUSE(uPort);
    CEDARC_UNUSE(pPeerComponent);
    CEDARC_UNUSE(uPeerPort);
    CEDARC_UNUSE(pTunnelSetup);

    logv("Error: component_tunnel_request Not Implemented\n");
    return OMX_ErrorNotImplemented;
}

OMX_ERRORTYPE  omx_venc::use_buffer(OMX_IN    OMX_HANDLETYPE          hComponent,
                                    OMX_INOUT OMX_BUFFERHEADERTYPE**  ppBufferHdr,
                                    OMX_IN    OMX_U32                 nPortIndex,
                                    OMX_IN    OMX_PTR                 pAppPrivate,
                                    OMX_IN    OMX_U32                 nSizeBytes,
                                    OMX_IN    OMX_U8*                 pBuffer)
{
    OMX_PARAM_PORTDEFINITIONTYPE*   pPortDef;
    OMX_U32                         nIndex = 0x0;
    logv(" COMPONENT_USE_BUFFER");
    // logd("-------nPortIndex: %d, nSizeBytes: %d", (int)nPortIndex, (int)nSizeBytes);

    if (hComponent == NULL || ppBufferHdr == NULL || pBuffer == NULL)
    {
        return OMX_ErrorBadParameter;
    }

    if (nPortIndex == m_sInPortDefType.nPortIndex)
        pPortDef = &m_sInPortDefType;
    else if (nPortIndex == m_sOutPortDefType.nPortIndex)
        pPortDef = &m_sOutPortDefType;
    else
        return OMX_ErrorBadParameter;

    if (!pPortDef->bEnabled)
        return OMX_ErrorIncorrectStateOperation;

    if (pPortDef->bPopulated)
        return OMX_ErrorBadParameter;

    // Find an empty position in the BufferList and allocate memory for the buffer header.
    // Use the buffer passed by the client to initialize the actual buffer
    // inside the buffer header.
    if (nPortIndex == m_sInPortDefType.nPortIndex)
    {
        pthread_mutex_lock(&m_inBufMutex);
        logv("vencInPort: use_buffer");

        if ((int)m_sInBufList.nAllocSize >= m_sInBufList.nBufArrSize)
        {
            pthread_mutex_unlock(&m_inBufMutex);
            return OMX_ErrorInsufficientResources;
        }

        nIndex = m_sInBufList.nAllocSize;
        m_sInBufList.nAllocSize++;

        m_sInBufList.pBufArr[nIndex].pBuffer = pBuffer;
        m_sInBufList.pBufArr[nIndex].nAllocLen   = nSizeBytes;
        m_sInBufList.pBufArr[nIndex].pAppPrivate = pAppPrivate;
        m_sInBufList.pBufArr[nIndex].nInputPortIndex = nPortIndex;
        m_sInBufList.pBufArr[nIndex].nOutputPortIndex = 0xFFFFFFFE;
        *ppBufferHdr = &m_sInBufList.pBufArr[nIndex];
        if (m_sInBufList.nAllocSize == pPortDef->nBufferCountActual)
        {
            pPortDef->bPopulated = OMX_TRUE;
        }

        pthread_mutex_unlock(&m_inBufMutex);
    }
    else
    {
        pthread_mutex_lock(&m_outBufMutex);
        logv("vencOutPort: use_buffer");

        if ((int)m_sOutBufList.nAllocSize >= m_sOutBufList.nBufArrSize)
        {
            pthread_mutex_unlock(&m_outBufMutex);
            return OMX_ErrorInsufficientResources;
        }

        nIndex = m_sOutBufList.nAllocSize;
        m_sOutBufList.nAllocSize++;
        m_sOutBufList.pBufArr[nIndex].pBuffer = pBuffer;
        m_sOutBufList.pBufArr[nIndex].nAllocLen   = nSizeBytes;
        m_sOutBufList.pBufArr[nIndex].pAppPrivate = pAppPrivate;
        m_sOutBufList.pBufArr[nIndex].nInputPortIndex = 0xFFFFFFFE;
        m_sOutBufList.pBufArr[nIndex].nOutputPortIndex = nPortIndex;
        *ppBufferHdr = &m_sOutBufList.pBufArr[nIndex];
        if (m_sOutBufList.nAllocSize == pPortDef->nBufferCountActual)
        {
            pPortDef->bPopulated = OMX_TRUE;
        }

        pthread_mutex_unlock(&m_outBufMutex);
    }

    return OMX_ErrorNone;
}


OMX_ERRORTYPE omx_venc::allocate_buffer(OMX_IN    OMX_HANDLETYPE         hComponent,
                                        OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                                        OMX_IN    OMX_U32                nPortIndex,
                                        OMX_IN    OMX_PTR                pAppPrivate,
                                        OMX_IN    OMX_U32                nSizeBytes)
{
    OMX_S8                        nIndex = 0x0;
    OMX_PARAM_PORTDEFINITIONTYPE* pPortDef;
    logv(" COMPONENT_ALLOCATE_BUFFER");

    if (hComponent == NULL || ppBufferHdr == NULL)
        return OMX_ErrorBadParameter;

    if (nPortIndex == m_sInPortDefType.nPortIndex)
    {
        logv("port_in: nPortIndex=%d", (int)nPortIndex);
        pPortDef = &m_sInPortDefType;
    }
    else
    {
        logv("port_out: nPortIndex=%d",(int)nPortIndex);
        if (nPortIndex == m_sOutPortDefType.nPortIndex)
        {
            logv("port_out: nPortIndex=%d", (int)nPortIndex);
            pPortDef = &m_sOutPortDefType;
        }
        else
        {
            logv("allocate_buffer fatal error! nPortIndex=%d", (int)nPortIndex);
            return OMX_ErrorBadParameter;
        }
    }

    if (!pPortDef->bEnabled)
        return OMX_ErrorIncorrectStateOperation;

    if (pPortDef->bPopulated)
        return OMX_ErrorBadParameter;

    // Find an empty position in the BufferList and allocate memory for the buffer header
    // and the actual buffer
    if (nPortIndex == m_sInPortDefType.nPortIndex)
    {
        pthread_mutex_lock(&m_inBufMutex);
        logv("vencInPort: malloc vbs");

        if ((int)m_sInBufList.nAllocSize >= m_sInBufList.nBufArrSize)
        {
            pthread_mutex_unlock(&m_inBufMutex);
            return OMX_ErrorInsufficientResources;
        }

        nIndex = m_sInBufList.nAllocSize;

        m_sInBufList.pBufArr[nIndex].pBuffer = (OMX_U8*)malloc(nSizeBytes);

        if (!m_sInBufList.pBufArr[nIndex].pBuffer)
        {
            pthread_mutex_unlock(&m_inBufMutex);
            return OMX_ErrorInsufficientResources;
        }

        m_sInBufList.nAllocBySelfFlags |= (1<<nIndex);

        m_sInBufList.pBufArr[nIndex].nAllocLen   = nSizeBytes;
        m_sInBufList.pBufArr[nIndex].pAppPrivate = pAppPrivate;
        m_sInBufList.pBufArr[nIndex].nInputPortIndex = nPortIndex;
        m_sInBufList.pBufArr[nIndex].nOutputPortIndex = 0xFFFFFFFE;
        *ppBufferHdr = &m_sInBufList.pBufArr[nIndex];

        m_sInBufList.nAllocSize++;
        if (m_sInBufList.nAllocSize == pPortDef->nBufferCountActual)
        {
            pPortDef->bPopulated = OMX_TRUE;
        }

        pthread_mutex_unlock(&m_inBufMutex);
    }
    else
    {
        logv("vencOutPort: malloc frame");
//        android::CallStack stack;
//        stack.update(1, 100);
//        stack.dump("allocate_buffer_for_frame");
        pthread_mutex_lock(&m_outBufMutex);

        if ((int)m_sOutBufList.nAllocSize >= m_sOutBufList.nBufArrSize)
        {
            pthread_mutex_unlock(&m_outBufMutex);
            return OMX_ErrorInsufficientResources;
        }

        nIndex = m_sOutBufList.nAllocSize;

        m_sOutBufList.pBufArr[nIndex].pBuffer = (OMX_U8*)malloc(nSizeBytes);

        if (!m_sOutBufList.pBufArr[nIndex].pBuffer)
        {
            pthread_mutex_unlock(&m_outBufMutex);
            return OMX_ErrorInsufficientResources;
        }

        m_sOutBufList.nAllocBySelfFlags |= (1<<nIndex);

        m_sOutBufList.pBufArr[nIndex].nAllocLen        = nSizeBytes;
        m_sOutBufList.pBufArr[nIndex].pAppPrivate      = pAppPrivate;
        m_sOutBufList.pBufArr[nIndex].nInputPortIndex  = 0xFFFFFFFE;
        m_sOutBufList.pBufArr[nIndex].nOutputPortIndex = nPortIndex;
        *ppBufferHdr = &m_sOutBufList.pBufArr[nIndex];

        m_sOutBufList.nAllocSize++;
        if (m_sOutBufList.nAllocSize == pPortDef->nBufferCountActual)
        {
            pPortDef->bPopulated = OMX_TRUE;
        }

        pthread_mutex_unlock(&m_outBufMutex);
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE omx_venc::free_buffer(OMX_IN  OMX_HANDLETYPE        hComponent,
                                    OMX_IN  OMX_U32               nPortIndex,
                                    OMX_IN  OMX_BUFFERHEADERTYPE* pBufferHdr)
{
    OMX_PARAM_PORTDEFINITIONTYPE* pPortDef;
    OMX_S32                       nIndex;

    logv(" COMPONENT_FREE_BUFFER, nPortIndex = %d, pBufferHdr = %p",
         (int)nPortIndex, pBufferHdr);

    if (hComponent == NULL || pBufferHdr == NULL)
    {
        return OMX_ErrorBadParameter;
    }

    // Match the pBufferHdr to the appropriate entry in the BufferList
    // and free the allocated memory
    if (nPortIndex == m_sInPortDefType.nPortIndex)
    {
        pPortDef = &m_sInPortDefType;

        pthread_mutex_lock(&m_inBufMutex);
        logv("vencInPort: free_buffer");

        for (nIndex = 0; nIndex < m_sInBufList.nBufArrSize; nIndex++)
        {
            if (pBufferHdr == &m_sInBufList.pBufArr[nIndex])
            {
                break;
            }
        }

        if (nIndex == m_sInBufList.nBufArrSize)
        {
            pthread_mutex_unlock(&m_inBufMutex);
            return OMX_ErrorBadParameter;
        }

        if (m_sInBufList.nAllocBySelfFlags & (1<<nIndex))
        {
            free(m_sInBufList.pBufArr[nIndex].pBuffer);
            m_sInBufList.pBufArr[nIndex].pBuffer = NULL;
            m_sInBufList.nAllocBySelfFlags &= ~(1<<nIndex);
        }

        m_sInBufList.nAllocSize--;
        if (m_sInBufList.nAllocSize == 0)
        {
            pPortDef->bPopulated = OMX_FALSE;
        }

        pthread_mutex_unlock(&m_inBufMutex);
    }
    else if (nPortIndex == m_sOutPortDefType.nPortIndex)
    {
        pPortDef = &m_sOutPortDefType;

        pthread_mutex_lock(&m_outBufMutex);
        logv("vencOutPort: free_buffer");

        for (nIndex = 0; nIndex < m_sOutBufList.nBufArrSize; nIndex++)
        {
            logv("pBufferHdr = %p, &m_sOutBufList.pBufArr[%d] = %p", pBufferHdr,
                 (int)nIndex, &m_sOutBufList.pBufArr[nIndex]);
            if (pBufferHdr == &m_sOutBufList.pBufArr[nIndex])
                break;
        }

        logv("index = %d", (int)nIndex);

        if (nIndex == m_sOutBufList.nBufArrSize)
        {
            pthread_mutex_unlock(&m_outBufMutex);
            return OMX_ErrorBadParameter;
        }

        if (m_sOutBufList.nAllocBySelfFlags & (1<<nIndex))
        {
            free(m_sOutBufList.pBufArr[nIndex].pBuffer);
            m_sOutBufList.pBufArr[nIndex].pBuffer = NULL;
            m_sOutBufList.nAllocBySelfFlags &= ~(1<<nIndex);
        }

        m_sOutBufList.nAllocSize--;
        if (m_sOutBufList.nAllocSize == 0)
        {
            pPortDef->bPopulated = OMX_FALSE;
        }

        pthread_mutex_unlock(&m_outBufMutex);
    }
    else
    {
        return OMX_ErrorBadParameter;
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE  omx_venc::empty_this_buffer(OMX_IN OMX_HANDLETYPE hComponent,
                                           OMX_IN OMX_BUFFERHEADERTYPE* pBufferHdr)
{
    ThrCmdType eCmdNative   = EmptyBuf;

    mEmptyBufCnt++;
    if (mEmptyBufCnt >= PRINT_FRAME_CNT)
    {
        logd("venc, empty_this_buffer %d times",PRINT_FRAME_CNT);
        mEmptyBufCnt = 0;
    }

    logv(" venc , FUNC:%s, LINE: %d",__FUNCTION__,__LINE__);

    if (hComponent == NULL || pBufferHdr == NULL)
    {
        return OMX_ErrorBadParameter;
    }

    if (!m_sInPortDefType.bEnabled)
    {
        return OMX_ErrorIncorrectStateOperation;
    }

    if (pBufferHdr->nInputPortIndex != 0x0  || pBufferHdr->nOutputPortIndex != OMX_NOPORT)
    {
        return OMX_ErrorBadPortIndex;
    }

    if (m_state != OMX_StateExecuting && m_state != OMX_StatePause)
    {
        return OMX_ErrorIncorrectStateOperation;
    }


    //fwrite(pBufferHdr->pBuffer, 1, pBufferHdr->nFilledLen, ph264File);
    //logv("BHF[0x%x],len[%d]", pBufferHdr->nFlags, pBufferHdr->nFilledLen);
    // Put the command and data in the pipe
    post_event(eCmdNative, 0, (OMX_PTR)pBufferHdr);

    return OMX_ErrorNone;
}

OMX_ERRORTYPE  omx_venc::fill_this_buffer(OMX_IN OMX_HANDLETYPE hComponent,
                                          OMX_IN OMX_BUFFERHEADERTYPE* pBufferHdr)
{
    ThrCmdType eCmdNative = FillBuf;

    mFillBufCnt++;
    if (mFillBufCnt >= PRINT_FRAME_CNT)
    {
        logd("venc, fill_this_buffer %d times",PRINT_FRAME_CNT);
        mFillBufCnt = 0;
    }
    // logv(" COMPONENT_FILL_THIS_BUFFER");
    logv("vencOutPort: fill_this_buffer");

    if (hComponent == NULL || pBufferHdr == NULL)
    {
        return OMX_ErrorBadParameter;
    }

    if (!m_sOutPortDefType.bEnabled)
    {
        return OMX_ErrorIncorrectStateOperation;
    }

    if (pBufferHdr->nOutputPortIndex != 0x1 || pBufferHdr->nInputPortIndex != OMX_NOPORT)
    {
        return OMX_ErrorBadPortIndex;
    }

    if (m_state != OMX_StateExecuting && m_state != OMX_StatePause)
    {
        return OMX_ErrorIncorrectStateOperation;
    }

    // Put the command and data in the pipe
    post_event(eCmdNative, 0, (OMX_PTR)pBufferHdr);

    return OMX_ErrorNone;
}

OMX_ERRORTYPE  omx_venc::set_callbacks(OMX_IN OMX_HANDLETYPE        pHComp,
                                           OMX_IN OMX_CALLBACKTYPE* pCallbacks,
                                           OMX_IN OMX_PTR           pAppData)
{
    logv(" COMPONENT_SET_CALLBACKS");

    if (pHComp == NULL || pCallbacks == NULL || pAppData == NULL)
    {
        return OMX_ErrorBadParameter;
    }
    memcpy(&m_Callbacks, pCallbacks, sizeof(OMX_CALLBACKTYPE));
    m_pAppData = pAppData;

    return OMX_ErrorNone;
}


OMX_ERRORTYPE  omx_venc::component_deinit(OMX_IN OMX_HANDLETYPE pHComp)
{
    logv(" COMPONENT_DEINIT");

    OMX_ERRORTYPE eError = OMX_ErrorNone;
    ThrCmdType    eCmdNative   = Stop;
    OMX_S32       nIndex = 0;

    logd("component_deinit");

    CEDARC_UNUSE(pHComp);

    // In case the client crashes, check for nAllocSize parameter.
    // If this is greater than zero, there are elements in the list that are not free'd.
    // In that case, free the elements.
    if (m_sInBufList.nAllocSize > 0)
    {
        for (nIndex=0; nIndex<m_sInBufList.nBufArrSize; nIndex++)
        {
            if (m_sInBufList.pBufArr[nIndex].pBuffer != NULL)
            {
                if (m_sInBufList.nAllocBySelfFlags & (1<<nIndex))
                {
                    free(m_sInBufList.pBufArr[nIndex].pBuffer);
                    m_sInBufList.pBufArr[nIndex].pBuffer = NULL;
                }
            }
        }

        if (m_sInBufList.pBufArr != NULL)
        {
            free(m_sInBufList.pBufArr);
        }

        if (m_sInBufList.pBufHdrList != NULL)
        {
            free(m_sInBufList.pBufHdrList);
        }

        memset(&m_sInBufList, 0, sizeof(struct _BufferList));
        m_sInBufList.nBufArrSize = m_sInPortDefType.nBufferCountActual;
    }

    if (m_sOutBufList.nAllocSize > 0)
    {
        for (nIndex=0; nIndex<m_sOutBufList.nBufArrSize; nIndex++)
        {
            if (m_sOutBufList.pBufArr[nIndex].pBuffer != NULL)
            {
                if (m_sOutBufList.nAllocBySelfFlags & (1<<nIndex))
                {
                    free(m_sOutBufList.pBufArr[nIndex].pBuffer);
                    m_sOutBufList.pBufArr[nIndex].pBuffer = NULL;
                }
            }
        }

        if (m_sOutBufList.pBufArr != NULL)
        {
            free(m_sOutBufList.pBufArr);
        }

        if (m_sOutBufList.pBufHdrList != NULL)
        {
            free(m_sOutBufList.pBufHdrList);
        }

        memset(&m_sOutBufList, 0, sizeof(struct _BufferList));
        m_sOutBufList.nBufArrSize = m_sOutPortDefType.nBufferCountActual;
    }

    // Put the command and data in the pipe
    //write(m_cmdpipe[1], &eCmdNative, sizeof(eCmdNative));
    //write(m_cmddatapipe[1], &eCmdNative, sizeof(eCmdNative));
    post_event(eCmdNative, eCmdNative, NULL);

    // Wait for thread to exit so we can get the status into "error"
    pthread_join(m_thread_id, (void**)&eError);
    pthread_join(m_venc_thread_id, (void**)&eError);

    // close the pipe handles
    close(m_cmdpipe[0]);
    close(m_cmdpipe[1]);
    close(m_cmddatapipe[0]);
    close(m_cmddatapipe[1]);
    close(m_venc_cmdpipe[0]);
    close(m_venc_cmdpipe[1]);

#if SAVE_BITSTREAM
    if (OutFile)
    {
        fclose(OutFile);
        OutFile = NULL;
    }
#endif

    return eError;
}


OMX_ERRORTYPE  omx_venc::use_EGL_image(OMX_IN OMX_HANDLETYPE               pHComp,
                                          OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                                          OMX_IN OMX_U32                   uPort,
                                          OMX_IN OMX_PTR                   pAppData,
                                          OMX_IN void*                     pEglImage)
{
    logv("Error : use_EGL_image:  Not Implemented \n");

    CEDARC_UNUSE(pHComp);
    CEDARC_UNUSE(ppBufferHdr);
    CEDARC_UNUSE(uPort);
    CEDARC_UNUSE(pAppData);
    CEDARC_UNUSE(pEglImage);

    return OMX_ErrorNotImplemented;
}


OMX_ERRORTYPE  omx_venc::component_role_enum(OMX_IN  OMX_HANDLETYPE pHComp,
                                             OMX_OUT OMX_U8*        pRole,
                                             OMX_IN  OMX_U32        uIndex)
{
    logv(" COMPONENT_ROLE_ENUM");

    OMX_ERRORTYPE eRet = OMX_ErrorNone;

    CEDARC_UNUSE(pHComp);

    if (!strncmp((char*)m_cName, "OMX.allwinner.video.encoder.avc", OMX_MAX_STRINGNAME_SIZE))
    {
        if ((0 == uIndex) && pRole)
        {
            strncpy((char *)pRole, "video_encoder.avc", OMX_MAX_STRINGNAME_SIZE);
            logv("component_role_enum: pRole %s\n", pRole);
        }
        else
        {
            eRet = OMX_ErrorNoMore;
        }
    }
    else
    {
        logd("\nERROR:Querying pRole on Unknown Component\n");
        eRet = OMX_ErrorInvalidComponentName;
    }

    return eRet;
}

OMX_ERRORTYPE omx_venc::post_event(OMX_IN ThrCmdType eCmdNative,
                                   OMX_IN OMX_U32         uParam1,
                                   OMX_IN OMX_PTR         pCmdData)
{
    pthread_mutex_lock(&m_pipeMutex);
    write(m_cmdpipe[1], &eCmdNative, sizeof(eCmdNative));

    // In case of MarkBuf, the pCmdData parameter is used to carry the data.
    // In other cases, the nParam1 parameter carries the data.
    if (eCmdNative == MarkBuf || eCmdNative == EmptyBuf || eCmdNative == FillBuf)
        write(m_cmddatapipe[1], &pCmdData, sizeof(OMX_PTR));
    else
        write(m_cmddatapipe[1], &uParam1, sizeof(uParam1));

    pthread_mutex_unlock(&m_pipeMutex);

    return OMX_ErrorNone;
}

int waitPipeDataToRead(int nPipeFd, int nTimeUs)
{
    fd_set                  rfds;
    struct timeval          timeout;
    FD_ZERO(&rfds);
    FD_SET(nPipeFd, &rfds);

    // Check for new command
    timeout.tv_sec  = 0;
    timeout.tv_usec = nTimeUs;

    select(nPipeFd+1, &rfds, NULL, NULL, &timeout);

    if (FD_ISSET(nPipeFd, &rfds))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


void setSuperFrameCfg(omx_venc* pVenc)
{
    VencSuperFrameConfig sSuperFrameCfg;

    VideoEncSetParameter(pVenc->m_encoder, VENC_IndexParamFramerate, &pVenc->m_framerate);

    switch (pVenc->mVideoSuperFrame.eSuperFrameMode)
    {
        case OMX_VIDEO_SUPERFRAME_NONE:
            sSuperFrameCfg.eSuperFrameMode = VENC_SUPERFRAME_NONE;
            break;

        case OMX_VIDEO_SUPERFRAME_REENCODE:
            sSuperFrameCfg.eSuperFrameMode = VENC_SUPERFRAME_REENCODE;
            break;

        case OMX_VIDEO_SUPERFRAME_DISCARD:
            sSuperFrameCfg.eSuperFrameMode = VENC_SUPERFRAME_DISCARD;
            break;

        default:
            logd("the omx venc do not support the superFrame mode:%d\n", \
                pVenc->mVideoSuperFrame.eSuperFrameMode);
            sSuperFrameCfg.eSuperFrameMode = VENC_SUPERFRAME_NONE;
            break;
    }

    if(!pVenc->mVideoSuperFrame.nMaxIFrameBits && !pVenc->mVideoSuperFrame.nMaxPFrameBits)
    {
        sSuperFrameCfg.nMaxIFrameBits = pVenc->m_sOutPortDefType.format.video.nBitrate /
            pVenc->m_framerate*3;
        sSuperFrameCfg.nMaxPFrameBits = pVenc->m_sOutPortDefType.format.video.nBitrate /
            pVenc->m_framerate*2;
        if (pVenc->m_framerate <= 10)
        {
            sSuperFrameCfg.nMaxIFrameBits = pVenc->m_sOutPortDefType.format.video.nBitrate /
                pVenc->m_framerate * 2;
            sSuperFrameCfg.nMaxPFrameBits = pVenc->m_sOutPortDefType.format.video.nBitrate /
                pVenc->m_framerate*3/2;
        }
    }
    else
    {
        sSuperFrameCfg.nMaxIFrameBits = pVenc->mVideoSuperFrame.nMaxIFrameBits;
        sSuperFrameCfg.nMaxPFrameBits = pVenc->mVideoSuperFrame.nMaxPFrameBits;
    }

    logd("setSuperFrameCfg, pSelf->m_framerate: %d, bitrate: %d\n",
         (int)pVenc->m_framerate, (int)pVenc->m_sOutPortDefType.format.video.nBitrate);
    logd("bitRate/frameRate:%d, nMaxIFrameBits:%d, nMaxPFrameBits:%d\n",\
    (int)pVenc->m_sOutPortDefType.format.video.nBitrate/(int)pVenc->m_framerate,\
    sSuperFrameCfg.nMaxIFrameBits, sSuperFrameCfg.nMaxPFrameBits);
    VideoEncSetParameter(pVenc->m_encoder, VENC_IndexParamSuperFrameConfig, &sSuperFrameCfg);
}

void setSVCSkipCfg(omx_venc* pVenc)
{
    VencH264SVCSkip SVCSkip;
    unsigned int total_ratio;
    memset(&SVCSkip, 0, sizeof(VencH264SVCSkip));

    logd("setSVCSkipCfg, pSelf->mVideoSVCSkip.uTemporalSVC: %d, LayerRatio[0-3]: [%d %d %d %d]\n",\
    (int)pVenc->mVideoSVCSkip.eSvcLayer, (int)pVenc->mVideoSVCSkip.uLayerRatio[0],\
    (int)pVenc->mVideoSVCSkip.uLayerRatio[1], (int)pVenc->mVideoSVCSkip.uLayerRatio[2],\
    (int)pVenc->mVideoSVCSkip.uLayerRatio[3]);

    SVCSkip.nTemporalSVC = NO_T_SVC;
    SVCSkip.nSkipFrame = NO_SKIP;
    switch(pVenc->mVideoSVCSkip.eSvcLayer)
    {
        case OMX_VIDEO_NO_SVC:
            SVCSkip.nTemporalSVC = NO_T_SVC;
            SVCSkip.nSkipFrame = NO_SKIP;
            break;
       case OMX_VIDEO_LAYER_2:
            SVCSkip.nTemporalSVC = T_LAYER_2;
            SVCSkip.nSkipFrame = SKIP_2;
            break;
       case OMX_VIDEO_LAYER_3:
            SVCSkip.nTemporalSVC = T_LAYER_3;
            SVCSkip.nSkipFrame = SKIP_4;
            break;
       case OMX_VIDEO_LAYER_4:
            SVCSkip.nTemporalSVC = T_LAYER_4;
            SVCSkip.nSkipFrame = SKIP_8;
            break;
       default:
            SVCSkip.nTemporalSVC = NO_T_SVC;
            SVCSkip.nSkipFrame = NO_SKIP;
            break;
    }
    total_ratio = pVenc->mVideoSVCSkip.uLayerRatio[0] + pVenc->mVideoSVCSkip.uLayerRatio[1] +\
    pVenc->mVideoSVCSkip.uLayerRatio[2] + pVenc->mVideoSVCSkip.uLayerRatio[3];
    if(total_ratio != 100)
    {
        logd("the set layer ratio sum is %d, not 100, so use the encoder default layer ratio\n",\
        total_ratio);
        SVCSkip.bEnableLayerRatio = 0;
    }
    else
    {
        SVCSkip.bEnableLayerRatio = 1;

        for(int i=0; i<4; i++)
            SVCSkip.nLayerRatio[i] = pVenc->mVideoSVCSkip.uLayerRatio[i];
    }

    VideoEncSetParameter(pVenc->m_encoder, VENC_IndexParamH264SVCSkip, &SVCSkip);
}


int openVencDriver(omx_venc* pVenc)
{
    VencBaseConfig sBaseConfig;
    int result;
    memset(&sBaseConfig, 0, sizeof(VencBaseConfig));
    int size_y;
    int size_c;
    char calling_process[256];

    pVenc->m_encoder = VideoEncCreate(pVenc->m_vencCodecType);
    if (pVenc->m_encoder == NULL)
    {
        pVenc->m_Callbacks.EventHandler(&pVenc->mOmxCmp, pVenc->m_pAppData,
                                        OMX_EventError, OMX_ErrorHardware, 0 , NULL);
        logw("VideoEncCreate fail");
        return -1;
    }

    if (pVenc->m_vencCodecType == VENC_CODEC_H264)
    {
        //* h264 param
        pVenc->m_vencH264Param.bEntropyCodingCABAC = 1;
        pVenc->m_vencH264Param.nBitrate = pVenc->m_sOutPutBitRateType.nTargetBitrate; /* bps */
        pVenc->m_vencH264Param.nFramerate = pVenc->m_framerate; /* fps */
        pVenc->m_vencH264Param.nCodingMode = VENC_FRAME_CODING;
        pVenc->m_vencH264Param.nMaxKeyInterval = (pVenc->m_sH264Type.nPFrames + 1);
        if (pVenc->m_vencH264Param.nMaxKeyInterval < 0)
        {
            pVenc->m_vencH264Param.nMaxKeyInterval = 30;
        }
        logd ("venc,pVenc->m_vencH264Param.nMaxKeyInterval = %d",
              pVenc->m_vencH264Param.nMaxKeyInterval);
        logd ("venc,pVenc->m_vencH264Param.nBitrate = %d, nFramerate:%d ",
              pVenc->m_vencH264Param.nBitrate, pVenc->m_vencH264Param.nFramerate);

        //set profile
        switch (pVenc->m_sH264Type.eProfile)
        {
            case 1:
                pVenc->m_vencH264Param.sProfileLevel.nProfile = VENC_H264ProfileBaseline;
                break;
            case 2:
                pVenc->m_vencH264Param.sProfileLevel.nProfile = VENC_H264ProfileMain;
                break;
            case 8:
                pVenc->m_vencH264Param.sProfileLevel.nProfile = VENC_H264ProfileHigh;
                break;

            default:
                pVenc->m_vencH264Param.sProfileLevel.nProfile = VENC_H264ProfileBaseline;
                break;
        }

        if (pVenc->mIsFromVideoeditor)
        {
            logd ("Called from Videoeditor,set VENC_H264ProfileBaseline");
            pVenc->m_vencH264Param.sProfileLevel.nProfile = VENC_H264ProfileBaseline;
        }

        logd("profile-venc=%d, profile-omx=%d, frame_rate:%d, bit_rate:%d, eColorFormat:%08x\n",
             pVenc->m_vencH264Param.sProfileLevel.nProfile,
             pVenc->m_sH264Type.eProfile,pVenc->m_vencH264Param.nFramerate,
             pVenc->m_vencH264Param.nBitrate,pVenc->m_sInPortFormatType.eColorFormat);

        //set level
        switch (pVenc->m_sH264Type.eLevel)
        {
            case 0x100:
            {
                pVenc->m_vencH264Param.sProfileLevel.nLevel = VENC_H264Level3;
                break;
            }
            case 0x200:
            {
                pVenc->m_vencH264Param.sProfileLevel.nLevel = VENC_H264Level31;
                break;
            }
            case 0x400:
            {
                pVenc->m_vencH264Param.sProfileLevel.nLevel = VENC_H264Level32;
                break;
            }
            case 0x800:
            {
                pVenc->m_vencH264Param.sProfileLevel.nLevel = VENC_H264Level4;
                break;
            }
            case 0x1000:
            {
                pVenc->m_vencH264Param.sProfileLevel.nLevel = VENC_H264Level41;
                break;
            }
            case 0x2000:
            {
                pVenc->m_vencH264Param.sProfileLevel.nLevel = VENC_H264Level42;
                break;
            }
            case 0x4000:
            {
                pVenc->m_vencH264Param.sProfileLevel.nLevel = VENC_H264Level5;
                break;
            }
            case 0x8000:
            {
                pVenc->m_vencH264Param.sProfileLevel.nLevel = VENC_H264Level51;
                break;
            }
            default:
            {
                pVenc->m_vencH264Param.sProfileLevel.nLevel = VENC_H264Level41;
                break;
            }
        }

        pVenc->m_vencH264Param.sQPRange.nMinqp = 6;
        pVenc->m_vencH264Param.sQPRange.nMaxqp = 45;

        if (pVenc->mVideoVBR.bEnable)
        {
            pVenc->m_vencH264Param.sQPRange.nMinqp = pVenc->mVideoVBR.uQpMin;
            pVenc->m_vencH264Param.sQPRange.nMaxqp = pVenc->mVideoVBR.uQpMax;
            pVenc->m_vencH264Param.nBitrate = pVenc->mVideoVBR.uBitRate;

            if(pVenc->mVideoVBR.sMdParam.bMotionDetectEnable)
            {
                OMX_VIDEO_PARAMS_MD sMD_param;
                sMD_param.bMotionDetectEnable = pVenc->mVideoVBR.sMdParam.bMotionDetectEnable;
                sMD_param.nMotionDetectRatio = pVenc->mVideoVBR.sMdParam.nMotionDetectRatio;
                sMD_param.nStaticDetectRatio = pVenc->mVideoVBR.sMdParam.nStaticDetectRatio;
                sMD_param.nMaxNumStaticFrame = pVenc->mVideoVBR.sMdParam.nMaxNumStaticFrame;
                sMD_param.nStaticBitsRatio = pVenc->mVideoVBR.sMdParam.nStaticBitsRatio;
                sMD_param.dMV64x64Ratio = pVenc->mVideoVBR.sMdParam.dMV64x64Ratio;
                sMD_param.sMVXTh = pVenc->mVideoVBR.sMdParam.sMVXTh;
                sMD_param.sMVYTh = pVenc->mVideoVBR.sMdParam.sMVYTh;

                VideoEncSetParameter(pVenc->m_encoder,\
                VENC_IndexParamMotionDetectEnable, &sMD_param);
            }
            logd("use VBR\n");
        }
        VideoEncSetParameter(pVenc->m_encoder, VENC_IndexParamH264Param, &pVenc->m_vencH264Param);

        if (pVenc->mVideoSuperFrame.bEnable)
        {
            setSuperFrameCfg(pVenc);
            logd("use setSuperFrameCfg");
        }

        if (pVenc->mVideoSVCSkip.bEnable)
        {
            setSVCSkipCfg(pVenc);
        }

#if 0 //* do not use this function right now
        if (pVenc->m_vencCyclicIntraRefresh.bEnable)
        {
            VideoEncSetParameter(pVenc->m_encoder, VENC_IndexParamH264CyclicIntraRefresh,
                                 &pVenc->m_vencCyclicIntraRefresh);
        }
#endif

    }

    switch (pVenc->m_sInPortFormatType.eColorFormat)
    {
#ifdef CONF_NOUGAT_AND_NEWER
        //* android 7.0 cannot get mIsFromCts, so we extend OMX_COLOR_FormatYVU420SemiPlanar
        case OMX_COLOR_FormatYUV420SemiPlanar:
        {
            pVenc->m_vencColorFormat = VENC_PIXEL_YUV420SP;
            break;
        }
        case OMX_COLOR_FormatYVU420SemiPlanar:
        {
            pVenc->m_vencColorFormat = VENC_PIXEL_YVU420SP;
            break;
        }
#else
        case OMX_COLOR_FormatYUV420SemiPlanar:
        {
            if(pVenc->mIsFromCts)
            {
                pVenc->m_vencColorFormat = VENC_PIXEL_YUV420SP;
            }
            else//* from camera
            {
                pVenc->m_vencColorFormat = VENC_PIXEL_YVU420SP;
            }
            break;
        }
#endif
        case OMX_COLOR_FormatAndroidOpaque:
        {
            pVenc->m_vencColorFormat = VENC_PIXEL_ARGB; //maybe change this later;
            break;
        }
        default:
        {
            break;
        }
    }

    if (!pVenc->m_useMetaDataInBuffers && !pVenc->m_useAndroidBuffer)
    {
        pVenc->m_useAllocInputBuffer = OMX_TRUE;
    }
    else
    {
        #if 0
        //* just work on chip-a80 and chip-a23, remove now
        if (pVenc->m_sInPortFormatType.eColorFormat ==
            OMX_COLOR_FormatAndroidOpaque && pVenc->mIsFromCts)
        {
            VENC_COLOR_SPACE colorSpace = VENC_BT601;
            if (VideoEncSetParameter(pVenc->m_encoder, VENC_IndexParamRgb2Yuv, &colorSpace) != 0)
            {
                pVenc->m_useAllocInputBuffer = OMX_TRUE;
                pVenc->m_vencColorFormat = VENC_PIXEL_YUV420SP;
                //use ImgRGBA2YUV420SP_neon convert argb to yuv420sp
            }
            else
            {
                logd("use bt601 ok");
                pVenc->m_useAllocInputBuffer = OMX_FALSE;
            }
        }
        else
        #endif
        {
            pVenc->m_useAllocInputBuffer = OMX_FALSE;
        }
    }

    logd("omx_venc base_config info: src_wxh:%dx%d, dis_wxh:%dx%d, stride:%d\n",
         (int)pVenc->m_sInPortDefType.format.video.nFrameWidth,
         (int)pVenc->m_sInPortDefType.format.video.nFrameHeight,
         (int)pVenc->m_sOutPortDefType.format.video.nFrameWidth,
         (int)pVenc->m_sOutPortDefType.format.video.nFrameHeight,
         (int)pVenc->m_sInPortDefType.format.video.nStride);

    sBaseConfig.nInputWidth= pVenc->m_sInPortDefType.format.video.nFrameWidth;
    sBaseConfig.nInputHeight = pVenc->m_sInPortDefType.format.video.nFrameHeight;
    sBaseConfig.nStride = pVenc->m_sInPortDefType.format.video.nStride;

    if (pVenc->mVideoExtParams.bEnableScaling)
    {
       sBaseConfig.nDstWidth = pVenc->mVideoExtParams.ui16ScaledWidth;
       sBaseConfig.nDstHeight = pVenc->mVideoExtParams.ui16ScaledHeight;
    }else
    {
       sBaseConfig.nDstWidth = pVenc->m_sOutPortDefType.format.video.nFrameWidth;
       sBaseConfig.nDstHeight = pVenc->m_sOutPortDefType.format.video.nFrameHeight;
    }

    sBaseConfig.eInputFormat = pVenc->m_vencColorFormat;
    sBaseConfig.memops = MemAdapterGetOpsS();

#ifdef CONF_IMG_GPU
    if (pVenc->m_sInPortFormatType.eColorFormat == OMX_COLOR_FormatAndroidOpaque)
        sBaseConfig.nStride = (sBaseConfig.nStride + 31) & (~31);
#endif

    if(pVenc->m_usePSkip)
        VideoEncSetParameter(pVenc->m_encoder, VENC_IndexParamSetPSkip, &pVenc->m_usePSkip);

    result = VideoEncInit(pVenc->m_encoder, &sBaseConfig);

    if (result != 0)
    {
        VideoEncDestroy(pVenc->m_encoder);
        pVenc->m_encoder = NULL;
        pVenc->m_Callbacks.EventHandler(&pVenc->mOmxCmp, pVenc->m_pAppData,
                                        OMX_EventError, OMX_ErrorHardware, 0 , NULL);
        logw("VideoEncInit, failed, result: %d\n", result);
        return -1;
    }

    if (pVenc->m_vencCodecType == VENC_CODEC_H264)
    {
        VideoEncGetParameter(pVenc->m_encoder, VENC_IndexParamH264SPSPPS, &pVenc->m_headdata);
    }

    size_y = sBaseConfig.nStride*sBaseConfig.nInputHeight;
    size_c = size_y>>1;

    pVenc->m_vencAllocBufferParam.nBufferNum = pVenc->m_sOutPortDefType.nBufferCountActual;
    pVenc->m_vencAllocBufferParam.nSizeY = size_y;
    pVenc->m_vencAllocBufferParam.nSizeC = size_c;

    pVenc->m_firstFrameFlag = OMX_TRUE;
    pVenc->mFirstInputFrame = OMX_TRUE;

    if (pVenc->m_useAllocInputBuffer)
    {
        result = AllocInputBuffer(pVenc->m_encoder, &pVenc->m_vencAllocBufferParam);
        if (result !=0 )
        {
            VideoEncUnInit(pVenc->m_encoder);
            VideoEncDestroy(pVenc->m_encoder);
            loge("AllocInputBuffer,error");
            pVenc->m_Callbacks.EventHandler(&pVenc->mOmxCmp, pVenc->m_pAppData,
                                            OMX_EventError, OMX_ErrorHardware, 0 , NULL);
            return -1;
        }
    }

#if PRINTF_FRAME_SIZE
    timeval cur_time1;
    gettimeofday(&cur_time1, NULL);
    pVenc->mTimeStart = cur_time1.tv_sec * 1000000LL + cur_time1.tv_usec;
#endif
    return 0;
}

void closeVencDriver(omx_venc* pVenc)
{
    if (pVenc->m_useAllocInputBuffer)
    {
        // ReleaseAllocInputBuffer(pVenc->m_encoder);
        pVenc->m_useAllocInputBuffer = OMX_FALSE;
    }

    VideoEncUnInit(pVenc->m_encoder);
    VideoEncDestroy(pVenc->m_encoder);
    pVenc->m_encoder = NULL;
}

/*
 *  Component Thread
 *    The ComponentThread function is exeuted in a separate pThread and
 *    is used to implement the actual component functions.
 */
 /*****************************************************************************/
static void* ComponentThread(void* pThreadData)
{
    int                     result;

    int                     i;
    int                     fd1;
    fd_set                  rfds;
    OMX_U32                 pCmdData;
    ThrCmdType              cmd;

    // Variables related to encoder buffer handling
    OMX_BUFFERHEADERTYPE*   pInBufHdr   = NULL;
    OMX_BUFFERHEADERTYPE*   pOutBufHdr  = NULL;
    OMX_MARKTYPE*           pMarkBuf    = NULL;
    OMX_U8*                 pInBuf      = NULL;
    OMX_U32                 nInBufSize;
    OMX_BOOL                bNoNeedSleep;
    int                     nInputBufferStep = OMX_VENC_STEP_GET_INPUTBUFFER;

    // Variables related to encoder timeouts
    OMX_U32                 nTimeout;
    OMX_BOOL                port_setting_match;
    OMX_BOOL                nInBufEos;
    OMX_BOOL                nVencNotifyEosFlag;

    OMX_PTR                 pMarkData;
    OMX_HANDLETYPE          hMarkTargetComponent;

    struct timeval          timeout;

    port_setting_match   = OMX_TRUE;
    nInBufEos            = OMX_FALSE;
    nVencNotifyEosFlag   = OMX_FALSE;
    pMarkData            = NULL;
    hMarkTargetComponent = NULL;

    VencInputBuffer       sInputBuffer;
    VencInputBuffer       sInputBuffer_return;
    VencOutputBuffer      sOutputBuffer;

    // Recover the pointer to my component specific data
    omx_venc* pSelf = static_cast<omx_venc*>(pThreadData);


    while (1)
    {
        fd1 = pSelf->m_cmdpipe[0];
        FD_ZERO(&rfds);
        FD_SET(fd1, &rfds);

        // Check for new command
        timeout.tv_sec  = 0;
        timeout.tv_usec = 0;

        i = select(pSelf->m_cmdpipe[0]+1, &rfds, NULL, NULL, &timeout);

        if (FD_ISSET(pSelf->m_cmdpipe[0], &rfds))
        {
            // retrieve command and data from pipe
            read(pSelf->m_cmdpipe[0], &cmd, sizeof(cmd));
            read(pSelf->m_cmddatapipe[0], &pCmdData, sizeof(pCmdData));

            // State transition command
            if (cmd == SetState)
            {
                logd("x set state command, cmd = %d, pCmdData = %d.", (int)cmd, (int)pCmdData);
                // If the parameter states a transition to the same state
                // raise a same state transition error.
                if (pSelf->m_state == (OMX_STATETYPE)(pCmdData))
                {
                    pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                    OMX_EventError, OMX_ErrorSameState, 0 , NULL);
                }
                else
                {
                    // transitions/callbacks made based on state transition table
                    // pCmdData contains the target state
                    switch ((OMX_STATETYPE)(pCmdData))
                    {
                         case OMX_StateInvalid:
                         {
                             pSelf->m_state = OMX_StateInvalid;
                             pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                             OMX_EventError, OMX_ErrorInvalidState,
                                                             0 , NULL);
                             pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                             OMX_EventCmdComplete,
                                                             OMX_CommandStateSet,
                                                             pSelf->m_state, NULL);
                            break;
                         }

                        case OMX_StateLoaded:
                        {
                            if (pSelf->m_state == OMX_StateIdle ||
                                pSelf->m_state == OMX_StateWaitForResources)
                            {
                                nTimeout = 0x0;
                                while (1)
                                {
                                    // Transition happens only when the ports are unpopulated
                                    if (!pSelf->m_sInPortDefType.bPopulated &&
                                        !pSelf->m_sOutPortDefType.bPopulated)
                                    {
                                        pSelf->m_state = OMX_StateLoaded;
                                        pSelf->m_Callbacks.EventHandler(
                                            &pSelf->mOmxCmp, pSelf->m_pAppData,
                                            OMX_EventCmdComplete,  OMX_CommandStateSet,
                                            pSelf->m_state, NULL);

                                        break;
                                     }
                                    else if (nTimeout++ > OMX_MAX_TIMEOUTS)
                                    {
                                        pSelf->m_Callbacks.EventHandler(
                                            &pSelf->mOmxCmp, pSelf->m_pAppData,
                                            OMX_EventError, OMX_ErrorInsufficientResources,
                                            0 , NULL);
                                        logw("Transition to loaded failed\n");
                                        break;
                                    }
                                    usleep(OMX_TIMEOUT*1000);
                                }
                            }
                            else
                            {
                                pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp,
                                                                pSelf->m_pAppData,
                                                                OMX_EventError,
                                                                OMX_ErrorIncorrectStateTransition,
                                                                0 , NULL);
                            }
                            break;
                        }

                        case OMX_StateIdle:
                        {
                            if (pSelf->m_state == OMX_StateInvalid)
                                pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                                OMX_EventError,
                                                                OMX_ErrorIncorrectStateTransition,
                                                                0 , NULL);
                            else
                            {
                                // Return buffers if currently in pause and executing
                                if (pSelf->m_state == OMX_StatePause ||
                                    pSelf->m_state == OMX_StateExecuting)
                                {
                                    // venc should in idle state before stop
                                    post_message_to_venc_and_wait(pSelf, OMX_Venc_Cmd_Enc_Idle);

                                    if (pSelf->m_useAllocInputBuffer)
                                    {
                                        if (nInputBufferStep == OMX_VENC_STEP_GET_ALLOCBUFFER)
                                        {
                                            pSelf->m_Callbacks.EmptyBufferDone(&pSelf->mOmxCmp,
                                                                               pSelf->m_pAppData,
                                                                               pInBufHdr);
                                        }
                                    }
                                    else
                                    {
                                        if (nInputBufferStep == OMX_VENC_STEP_ADD_BUFFER_TO_ENC)
                                        {
                                            pSelf->m_Callbacks.EmptyBufferDone(&pSelf->mOmxCmp,
                                                                               pSelf->m_pAppData,
                                                                               pInBufHdr);
                                        }
                                        //* check used buffer
                                        while (0 == AlreadyUsedInputBuffer(pSelf->m_encoder,
                                                                           &sInputBuffer_return))
                                        {
                                            pInBufHdr =
                                                (OMX_BUFFERHEADERTYPE*)sInputBuffer_return.nID;
                                            pSelf->m_Callbacks.EmptyBufferDone(&pSelf->mOmxCmp,
                                                                               pSelf->m_pAppData,
                                                                               pInBufHdr);
                                        }
                                    }

                                    pthread_mutex_lock(&pSelf->m_inBufMutex);

                                    while (pSelf->m_sInBufList.nSizeOfList > 0)
                                    {
                                        OMX_S32 in_pos = pSelf->m_sInBufList.nReadPos++;
                                        pSelf->m_sInBufList.nSizeOfList--;
                                        pSelf->m_Callbacks.EmptyBufferDone(
                                            &pSelf->mOmxCmp, pSelf->m_pAppData,
                                            pSelf->m_sInBufList.pBufHdrList[in_pos]);

                                        if (pSelf->m_sInBufList.nReadPos >=
                                            pSelf->m_sInBufList.nBufArrSize)
                                        {
                                            pSelf->m_sInBufList.nReadPos = 0;
                                        }
                                    }

                                    pthread_mutex_unlock(&pSelf->m_inBufMutex);

                                    pthread_mutex_lock(&pSelf->m_outBufMutex);

                                    while (pSelf->m_sOutBufList.nSizeOfList > 0)
                                    {
                                        OMX_S32 out_pos = pSelf->m_sOutBufList.nReadPos++;
                                        pSelf->m_sOutBufList.nSizeOfList--;
                                        pSelf->m_Callbacks.FillBufferDone(
                                            &pSelf->mOmxCmp, pSelf->m_pAppData,
                                            pSelf->m_sOutBufList.pBufHdrList[out_pos]);

                                        if (pSelf->m_sOutBufList.nReadPos >=
                                            pSelf->m_sOutBufList.nBufArrSize)
                                        {
                                            pSelf->m_sOutBufList.nReadPos = 0;
                                        }
                                    }

                                    pthread_mutex_unlock(&pSelf->m_outBufMutex);

                                    post_message_to_venc_and_wait(pSelf, OMX_Venc_Cmd_Close);
                                }
                                else
                                {
                                    //* init encoder
                                    //post_message_to_venc_and_wait(pSelf, OMX_Venc_Cmd_Open);
                                }

                                nTimeout = 0x0;
                                while (1)
                                {
                                    logv("pSelf->m_sInPortDefType.bPopulated:%d, \
                                         pSelf->m_sOutPortDefType.bPopulated: %d",
                                         pSelf->m_sInPortDefType.bPopulated,
                                         pSelf->m_sOutPortDefType.bPopulated);
                                    // Ports have to be populated before transition completes
                                    if ((!pSelf->m_sInPortDefType.bEnabled &&
                                        !pSelf->m_sOutPortDefType.bEnabled) ||
                                        (pSelf->m_sInPortDefType.bPopulated &&
                                        pSelf->m_sOutPortDefType.bPopulated))
                                    {
                                        pSelf->m_state = OMX_StateIdle;
                                        pSelf->m_Callbacks.EventHandler(
                                            &pSelf->mOmxCmp, pSelf->m_pAppData,
                                            OMX_EventCmdComplete, OMX_CommandStateSet,
                                            pSelf->m_state, NULL);

                                         break;
                                    }
                                    else if (nTimeout++ > OMX_MAX_TIMEOUTS)
                                    {
                                        pSelf->m_Callbacks.EventHandler(
                                            &pSelf->mOmxCmp, pSelf->m_pAppData,
                                            OMX_EventError, OMX_ErrorInsufficientResources,
                                            0 , NULL);
                                        logw("Idle transition failed\n");
                                        break;
                                    }
                                    usleep(OMX_TIMEOUT*1000);
                                }
                            }
                            break;
                        }

                        case OMX_StateExecuting:
                        {
                            // Transition can only happen from pause or idle state
                            if (pSelf->m_state == OMX_StateIdle || pSelf->m_state == OMX_StatePause)
                            {
                                // Return buffers if currently in pause
                                if (pSelf->m_state == OMX_StatePause)
                                {
                                    loge("encode component do not support OMX_StatePause");

                                    pthread_mutex_lock(&pSelf->m_inBufMutex);

                                    while (pSelf->m_sInBufList.nSizeOfList > 0)
                                    {
                                        OMX_S32 in_pos = pSelf->m_sInBufList.nReadPos++;
                                        pSelf->m_sInBufList.nSizeOfList--;
                                        pSelf->m_Callbacks.EmptyBufferDone(
                                            &pSelf->mOmxCmp, pSelf->m_pAppData,
                                            pSelf->m_sInBufList.pBufHdrList[in_pos]);

                                        if (pSelf->m_sInBufList.nReadPos >=
                                            pSelf->m_sInBufList.nBufArrSize)
                                        {
                                            pSelf->m_sInBufList.nReadPos = 0;
                                        }
                                    }

                                    pthread_mutex_unlock(&pSelf->m_inBufMutex);

                                    pthread_mutex_lock(&pSelf->m_outBufMutex);

                                    while (pSelf->m_sOutBufList.nSizeOfList > 0)
                                    {
                                        OMX_S32 out_pos = pSelf->m_sOutBufList.nReadPos++;
                                        pSelf->m_sOutBufList.nSizeOfList--;
                                        pSelf->m_Callbacks.FillBufferDone(
                                            &pSelf->mOmxCmp, pSelf->m_pAppData,
                                            pSelf->m_sOutBufList.pBufHdrList[out_pos]);

                                        if (pSelf->m_sOutBufList.nReadPos >=
                                            pSelf->m_sOutBufList.nBufArrSize)
                                        {
                                            pSelf->m_sOutBufList.nReadPos = 0;
                                        }
                                    }

                                    pthread_mutex_unlock(&pSelf->m_outBufMutex);
                                }

                                pSelf->m_state = OMX_StateExecuting;
                                pSelf->m_Callbacks.EventHandler(
                                    &pSelf->mOmxCmp, pSelf->m_pAppData,
                                    OMX_EventCmdComplete, OMX_CommandStateSet,
                                    pSelf->m_state, NULL);

                                post_message_to_venc_and_wait(pSelf, OMX_Venc_Cmd_Open);
                            }
                            else
                            {
                                pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                                OMX_EventError,
                                                                OMX_ErrorIncorrectStateTransition,
                                                                0 , NULL);
                            }
                            nInBufEos            = OMX_FALSE; //if need it
                            pMarkData            = NULL;
                            hMarkTargetComponent = NULL;
                            break;
                        }

                        case OMX_StatePause:
                        {
                            loge("encode component do not support OMX_StatePause");
                            // Transition can only happen from idle or executing state
                            if (pSelf->m_state == OMX_StateIdle ||
                                pSelf->m_state == OMX_StateExecuting)
                            {
                                pSelf->m_state = OMX_StatePause;
                                pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                                OMX_EventCmdComplete,
                                                                OMX_CommandStateSet,
                                                                pSelf->m_state, NULL);
                            }
                            else
                            {
                                pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                                OMX_EventError,
                                                                OMX_ErrorIncorrectStateTransition,
                                                                0 , NULL);
                            }
                            break;
                        }

                        case OMX_StateWaitForResources:
                        {
                            if (pSelf->m_state == OMX_StateLoaded)
                            {
                                pSelf->m_state = OMX_StateWaitForResources;
                                pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                                OMX_EventCmdComplete,
                                                                OMX_CommandStateSet,
                                                                pSelf->m_state, NULL);
                            }
                            else
                            {
                                pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                                OMX_EventError,
                                                                OMX_ErrorIncorrectStateTransition,
                                                                0 , NULL);
                            }
                            break;
                        }
                        default:
                        {
                            loge("unknowed OMX_State");
                            pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                            OMX_EventError,
                                                            OMX_ErrorIncorrectStateTransition,
                                                            0 , NULL);
                            break;
                        }
                    }
                }
            }
            else if (cmd == StopPort)
            {
                logd("x stop port command, pCmdData = %d.", (int)pCmdData);
                // Stop Port(s)
                // pCmdData contains the port index to be stopped.
                // It is assumed that 0 is input and 1 is output port for this component
                // The pCmdData value -1 means that both input and output ports will be stopped.
                if (pCmdData == 0x0 || (int)pCmdData == -1)
                {
                    // Return all input buffers
                    pthread_mutex_lock(&pSelf->m_inBufMutex);

                    while (pSelf->m_sInBufList.nSizeOfList > 0)
                    {
                        pSelf->m_sInBufList.nSizeOfList--;
                        pSelf->m_Callbacks.EmptyBufferDone(
                            &pSelf->mOmxCmp, pSelf->m_pAppData,
                            pSelf->m_sInBufList.pBufHdrList[pSelf->m_sInBufList.nReadPos++]);

                        if (pSelf->m_sInBufList.nReadPos >= pSelf->m_sInBufList.nBufArrSize)
                            pSelf->m_sInBufList.nReadPos = 0;
                    }

                    pthread_mutex_unlock(&pSelf->m_inBufMutex);

                     // Disable port
                    pSelf->m_sInPortDefType.bEnabled = OMX_FALSE;
                }

                if (pCmdData == 0x1 || (int)pCmdData == -1)
                {
                    // Return all output buffers
                    pthread_mutex_lock(&pSelf->m_outBufMutex);

                    while (pSelf->m_sOutBufList.nSizeOfList > 0)
                    {
                        pSelf->m_sOutBufList.nSizeOfList--;
                        pSelf->m_Callbacks.FillBufferDone(
                            &pSelf->mOmxCmp, pSelf->m_pAppData,
                            pSelf->m_sOutBufList.pBufHdrList[pSelf->m_sOutBufList.nReadPos++]);

                        if (pSelf->m_sOutBufList.nReadPos >= pSelf->m_sOutBufList.nBufArrSize)
                            pSelf->m_sOutBufList.nReadPos = 0;
                    }

                    pthread_mutex_unlock(&pSelf->m_outBufMutex);

                       // Disable port
                    pSelf->m_sOutPortDefType.bEnabled = OMX_FALSE;
                }

                // Wait for all buffers to be freed
                nTimeout = 0x0;
                while (1)
                {
                    if (pCmdData == 0x0 && !pSelf->m_sInPortDefType.bPopulated)
                    {
                        // Return cmdcomplete event if input unpopulated
                        pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                        OMX_EventCmdComplete,
                                                        OMX_CommandPortDisable, 0x0, NULL);
                        break;
                    }

                    if (pCmdData == 0x1 && !pSelf->m_sOutPortDefType.bPopulated)
                    {
                        // Return cmdcomplete event if output unpopulated
                        pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                        OMX_EventCmdComplete,
                                                        OMX_CommandPortDisable, 0x1, NULL);
                        break;
                    }

                    if ((int)pCmdData == -1 && !pSelf->m_sInPortDefType.bPopulated &&
                        !pSelf->m_sOutPortDefType.bPopulated)
                    {
                        // Return cmdcomplete event if inout & output unpopulated
                        pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                        OMX_EventCmdComplete,
                                                        OMX_CommandPortDisable, 0x0, NULL);
                        pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                        OMX_EventCmdComplete,
                                                        OMX_CommandPortDisable, 0x1, NULL);
                        break;
                    }

                    if (nTimeout++ > OMX_MAX_TIMEOUTS)
                    {
                        pSelf->m_Callbacks.EventHandler(
                            &pSelf->mOmxCmp, pSelf->m_pAppData, OMX_EventError,
                            OMX_ErrorPortUnresponsiveDuringDeallocation, 0, NULL);
                        break;
                    }

                    usleep(OMX_TIMEOUT*1000);
                }
            }
            else if (cmd == RestartPort)
            {
                logd("x restart port command.");
                // Restart Port(s)
                // pCmdData contains the port index to be restarted.
                // It is assumed that 0 is input and 1 is output port for this component.
                // The pCmdData value -1 means both input and output ports will be restarted.
                if (pCmdData == 0x0 || (int)pCmdData == -1)
                    pSelf->m_sInPortDefType.bEnabled = OMX_TRUE;

                if (pCmdData == 0x1 || (int)pCmdData == -1)
                    pSelf->m_sOutPortDefType.bEnabled = OMX_TRUE;

                 // Wait for port to be populated
                nTimeout = 0x0;
                while (1)
                {
                    // Return cmdcomplete event if input port populated
                    if (pCmdData == 0x0 && (pSelf->m_state == OMX_StateLoaded ||
                        pSelf->m_sInPortDefType.bPopulated))
                    {
                        pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                        OMX_EventCmdComplete,
                                                        OMX_CommandPortEnable, 0x0, NULL);
                        break;
                    }
                    // Return cmdcomplete event if output port populated
                    else if (pCmdData == 0x1 && (pSelf->m_state == OMX_StateLoaded ||
                             pSelf->m_sOutPortDefType.bPopulated))
                    {
                        pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                        OMX_EventCmdComplete,
                                                        OMX_CommandPortEnable, 0x1, NULL);
                        break;
                    }
                    // Return cmdcomplete event if input and output ports populated
                    else if ((int)pCmdData == -1 && (pSelf->m_state == OMX_StateLoaded ||
                             (pSelf->m_sInPortDefType.bPopulated &&
                              pSelf->m_sOutPortDefType.bPopulated)))
                    {
                        pSelf->m_Callbacks.EventHandler(
                            &pSelf->mOmxCmp, pSelf->m_pAppData,
                            OMX_EventCmdComplete, OMX_CommandPortEnable, 0x0, NULL);
                        pSelf->m_Callbacks.EventHandler(
                            &pSelf->mOmxCmp, pSelf->m_pAppData,
                            OMX_EventCmdComplete, OMX_CommandPortEnable, 0x1, NULL);
                        break;
                    }
                    else if (nTimeout++ > OMX_MAX_TIMEOUTS)
                    {
                        pSelf->m_Callbacks.EventHandler(
                            &pSelf->mOmxCmp, pSelf->m_pAppData, OMX_EventError,
                            OMX_ErrorPortUnresponsiveDuringAllocation, 0, NULL);
                        break;
                    }

                    usleep(OMX_TIMEOUT*1000);
                }

                if (port_setting_match == OMX_FALSE)
                    port_setting_match = OMX_TRUE;
            }
            else if (cmd == Flush)
            {
                logd("x flush command.");
                // Flush port(s)
                // pCmdData contains the port index to be flushed.
                // It is assumed that 0 is input and 1 is output port for this component
                // The pCmdData value -1 means that both input and output ports will be flushed.
                if (pCmdData == 0x0 || (int)pCmdData == -1)
                {
                    // Return all input buffers and send cmdcomplete
                    pthread_mutex_lock(&pSelf->m_inBufMutex);

                    while (pSelf->m_sInBufList.nSizeOfList > 0)
                    {
                        pSelf->m_sInBufList.nSizeOfList--;
                        pSelf->m_Callbacks.EmptyBufferDone(
                            &pSelf->mOmxCmp, pSelf->m_pAppData,
                            pSelf->m_sInBufList.pBufHdrList[pSelf->m_sInBufList.nReadPos++]);

                        if (pSelf->m_sInBufList.nReadPos >= pSelf->m_sInBufList.nBufArrSize)
                            pSelf->m_sInBufList.nReadPos = 0;
                    }

                    pthread_mutex_unlock(&pSelf->m_inBufMutex);

                    pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                    OMX_EventCmdComplete,
                                                    OMX_CommandFlush, 0x0, NULL);
                }

                if (pCmdData == 0x1 || (int)pCmdData == -1)
                {
                    // Return all output buffers and send cmdcomplete
                    pthread_mutex_lock(&pSelf->m_outBufMutex);

                    while (pSelf->m_sOutBufList.nSizeOfList > 0)
                    {
                        pSelf->m_sOutBufList.nSizeOfList--;
                        pSelf->m_Callbacks.FillBufferDone(
                            &pSelf->mOmxCmp, pSelf->m_pAppData,
                            pSelf->m_sOutBufList.pBufHdrList[pSelf->m_sOutBufList.nReadPos++]);

                        if (pSelf->m_sOutBufList.nReadPos >= pSelf->m_sOutBufList.nBufArrSize)
                            pSelf->m_sOutBufList.nReadPos = 0;
                    }

                    pthread_mutex_unlock(&pSelf->m_outBufMutex);

                    pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                    OMX_EventCmdComplete,
                                                    OMX_CommandFlush, 0x1, NULL);
                }
            }
            else if (cmd == Stop)
            {
                logd("x stop command.");
                post_message_to_venc_and_wait(pSelf, OMX_Venc_Cmd_Stop);
                // Kill thread
                goto EXIT;
            }
            else if (cmd == FillBuf)
            {
                logv("LINE %d", __LINE__);
                OMX_BUFFERHEADERTYPE*   OutBufHdr  = NULL;
                // Fill buffer
                pthread_mutex_lock(&pSelf->m_outBufMutex);
                if (pSelf->m_sOutBufList.nSizeOfList <= pSelf->m_sOutBufList.nAllocSize)
                {
                    pSelf->m_sOutBufList.nSizeOfList++;
                    OutBufHdr = pSelf->m_sOutBufList.pBufHdrList[pSelf->m_sOutBufList.nWritePos++]=
                        ((OMX_BUFFERHEADERTYPE*) pCmdData);

                    if (pSelf->m_sOutBufList.nWritePos >= (int)pSelf->m_sOutBufList.nAllocSize)
                    {
                        pSelf->m_sOutBufList.nWritePos = 0;
                    }
                }
                logv("###[FillBuf ] OutBufHdr=%p", OutBufHdr);
                pthread_mutex_unlock(&pSelf->m_outBufMutex);
            }
            else if (cmd == EmptyBuf)
            {
                logv("LINE %d", __LINE__);
                OMX_BUFFERHEADERTYPE*   inBufHdr  = NULL;
                // Empty buffer
                pthread_mutex_lock(&pSelf->m_inBufMutex);

                if (pSelf->m_sInBufList.nSizeOfList <= pSelf->m_sInBufList.nAllocSize)
                {
                    pSelf->m_sInBufList.nSizeOfList++;
                    inBufHdr = pSelf->m_sInBufList.pBufHdrList[pSelf->m_sInBufList.nWritePos++] =
                        ((OMX_BUFFERHEADERTYPE*) pCmdData);

                    if (pSelf->m_sInBufList.nWritePos >= (int)pSelf->m_sInBufList.nAllocSize)
                        pSelf->m_sInBufList.nWritePos = 0;
                }
                logv("###[EmptyBuf ] inBufHdr=%p", inBufHdr);
                pthread_mutex_unlock(&pSelf->m_inBufMutex);

                // Mark current buffer if there is outstanding command
                if (pMarkBuf)
                {
                    ((OMX_BUFFERHEADERTYPE *)(pCmdData))->hMarkTargetComponent = &pSelf->mOmxCmp;
                    ((OMX_BUFFERHEADERTYPE *)(pCmdData))->pMarkData = pMarkBuf->pMarkData;
                    pMarkBuf = NULL;
                }
            }
            else if (cmd == MarkBuf)
            {
                if (!pMarkBuf)
                    pMarkBuf = (OMX_MARKTYPE *)(pCmdData);
            }
        }


        // Buffer processing
        // Only happens when the component is in executing state.
        if (pSelf->m_state == OMX_StateExecuting  &&
            pSelf->m_sInPortDefType.bEnabled      &&
            pSelf->m_sOutPortDefType.bEnabled     &&
            port_setting_match)
        {
            //* check input buffer.
            bNoNeedSleep = OMX_FALSE;

            if (nInBufEos && (nInputBufferStep == OMX_VENC_STEP_GET_INPUTBUFFER))
            {
                post_message_to_venc_and_wait(pSelf, OMX_Venc_Cmd_Enc_Idle);

                if (ValidBitstreamFrameNum(pSelf->m_encoder) <= 0)
                {
                    pthread_mutex_lock(&pSelf->m_outBufMutex);

                    if (pSelf->m_sOutBufList.nSizeOfList > 0)
                    {
                        pSelf->m_sOutBufList.nSizeOfList--;
                        pOutBufHdr =
                            pSelf->m_sOutBufList.pBufHdrList[pSelf->m_sOutBufList.nReadPos++];
                        if (pSelf->m_sOutBufList.nReadPos >= (int)pSelf->m_sOutBufList.nAllocSize)
                            pSelf->m_sOutBufList.nReadPos = 0;
                    }
                    else
                    {
                        pOutBufHdr = NULL;
                    }

                    pthread_mutex_unlock(&pSelf->m_outBufMutex);

                    //* if no output buffer, wait for some time.
                    if (pOutBufHdr == NULL)
                    {
                    }
                    else
                    {
                        pOutBufHdr->nFlags &= ~OMX_BUFFERFLAG_CODECCONFIG;
                        pOutBufHdr->nFlags &= ~OMX_BUFFERFLAG_SYNCFRAME;

                        pOutBufHdr->nFlags |= OMX_BUFFERFLAG_EOS;
                        pOutBufHdr->nFilledLen = 0;

                        pSelf->m_Callbacks.FillBufferDone(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                          pOutBufHdr);
                        pOutBufHdr = NULL;
                        nInBufEos = OMX_FALSE;
                    }
                }
            }

            if (nInputBufferStep == OMX_VENC_STEP_GET_INPUTBUFFER)
            {
                pthread_mutex_lock(&pSelf->m_inBufMutex);

                if (pSelf->m_sInBufList.nSizeOfList > 0)
                {
                    pSelf->m_sInBufList.nSizeOfList--;
                    pInBufHdr = pSelf->m_sInBufList.pBufHdrList[pSelf->m_sInBufList.nReadPos++];
                    if (pSelf->m_sInBufList.nReadPos >= (int)pSelf->m_sInBufList.nAllocSize)
                        pSelf->m_sInBufList.nReadPos = 0;
                }
                else
                {
                    pInBufHdr = NULL;
                }

                pthread_mutex_unlock(&pSelf->m_inBufMutex);

                if (pInBufHdr)
                {
                    bNoNeedSleep = OMX_TRUE;

                    if (pInBufHdr->nFlags & OMX_BUFFERFLAG_EOS)
                    {
                        // Copy flag to output buffer header
                        nInBufEos = OMX_TRUE;
                        logd(" set up nInBufEos flag.: %p", pInBufHdr);
                        // Trigger event handler
                        pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                        OMX_EventBufferFlag, 0x1,
                                                        pInBufHdr->nFlags, NULL);
                        // Clear flag
                        pInBufHdr->nFlags = 0;
                    }

                    // Check for mark buffers
                    if (pInBufHdr->pMarkData)
                    {
                        // Copy mark to output buffer header
                        if (pOutBufHdr)
                        {
                            pMarkData = pInBufHdr->pMarkData;
                            // Copy handle to output buffer header
                            hMarkTargetComponent = pInBufHdr->hMarkTargetComponent;
                        }
                    }

                    // Trigger event handler
                    if (pInBufHdr->hMarkTargetComponent == &pSelf->mOmxCmp && pInBufHdr->pMarkData)
                    {
                        pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                        OMX_EventMark, 0, 0, pInBufHdr->pMarkData);
                    }

                    if (!pSelf->m_useAllocInputBuffer)
                    {
                        if (pInBufHdr->nFilledLen <= 0)
                        {
                            logw("skip this input buffer, pInBufHdr->nTimeStamp %lld",
                                 pInBufHdr->nTimeStamp);
                            pSelf->m_Callbacks.EmptyBufferDone(&pSelf->mOmxCmp,
                                                               pSelf->m_pAppData, pInBufHdr);
                            pInBufHdr = NULL;
                        }
                        else
                        {
#ifdef __ANDROID__
                            int buffer_type =  *(int*)(pInBufHdr->pBuffer+pInBufHdr->nOffset);
                            buffer_handle_t bufferHanle;

#ifdef CONF_NOUGAT_AND_NEWER
                            if((buffer_type == kMetadataBufferTypeGrallocSource) || \
                                (buffer_type == kMetadataBufferTypeANWBuffer))
#else
                            if(buffer_type == kMetadataBufferTypeGrallocSource)
#endif
                            {
                                unsigned long phyaddress = 0;
                                if (buffer_type == kMetadataBufferTypeGrallocSource)
                                {
                                    bufferHanle = *(buffer_handle_t*)(pInBufHdr->pBuffer +
                                                                      pInBufHdr->nOffset + 4);
                                }
#ifdef CONF_NOUGAT_AND_NEWER
                                else if(buffer_type == kMetadataBufferTypeANWBuffer)
                                {
                                    VideoNativeMetadata &nativeMeta = \
                                    *(VideoNativeMetadata *)(pInBufHdr->pBuffer + \
                                    pInBufHdr->nOffset);
                                    ANativeWindowBuffer *buffer = \
                                    (ANativeWindowBuffer *)nativeMeta.pBuffer;

                                    bufferHanle = buffer->handle;

                                    if (nativeMeta.nFenceFd >= 0) {
                                        sp<Fence> fence = new Fence(nativeMeta.nFenceFd);
                                        nativeMeta.nFenceFd = -1;
                                        status_t err = fence->wait(IOMX::kFenceTimeoutMs);
                                        if (err != OK) {
                                            ALOGE("Timed out waiting on input fence");
                                            return NULL;
                                        }
                                    }
                                }
#endif
                                if (pSelf->m_sInPortFormatType.eColorFormat !=
                                    OMX_COLOR_FormatAndroidOpaque)
                                {
                                    logw("do not support this format: %d",
                                         pSelf->m_sInPortFormatType.eColorFormat);
                                }

                                if (bufferHanle)
                                {
                                    int colorFormat;
                                    //for mali GPU
#ifdef CONF_MALI_GPU
                                    private_handle_t* hnd = (private_handle_t *)(bufferHanle);
                                    colorFormat = hnd->format;
#else
                                    IMG_native_handle_t* hnd = (IMG_native_handle_t*)(bufferHanle);
                                    colorFormat = hnd->iFormat;
#endif
                                    switch (colorFormat)
                                    {
                                        case HAL_PIXEL_FORMAT_RGBA_8888:
                                        case HAL_PIXEL_FORMAT_RGBX_8888:
                                        {
                                            if (pSelf->mFirstInputFrame)
                                            {
                                                pSelf->m_vencColorFormat = VENC_PIXEL_ABGR;
                                                post_message_to_venc_and_wait(
                                                    pSelf, OMX_Venc_Cmd_ChangeColorFormat);
                                                pSelf->mFirstInputFrame = OMX_FALSE;
                                                logd("set color format to ABGR");
                                            }
                                            break;
                                        }
                                        case HAL_PIXEL_FORMAT_BGRA_8888:
                                        {
                                            logv("do nothing, defalt is ARGB");
                                            break;
                                        }
                                        default:
                                        {
                                            logw("do not support this format: %d", colorFormat);
                                            break;
                                        }
                                    }

                                    int fd = ion_open();
#ifdef CONF_KERNEL_VERSION_3_10
                                    ion_user_handle_t handle_ion;
#else
                                    struct ion_handle *handle_ion;
#endif
                                    if (fd != -1)
                                    {
#ifdef CONF_MALI_GPU
                                        ion_import(fd, hnd->share_fd, &handle_ion);
#else
                                        ion_import(fd, hnd->fd[0], &handle_ion);
#endif
                                        phyaddress= CdcIonGetPhyAdr(fd, (uintptr_t)handle_ion);
                                        ion_close(fd);
                                    }
                                    else
                                    {
                                        loge("ion_open fail");
                                    }

                                }
                                else
                                {
                                    loge("bufferHanle is null");
                                }

                                logv("phyaddress: %x", phyaddress);
                                // only support ARGB now
                                sInputBuffer.pAddrPhyY= (unsigned char *)phyaddress;
                                sInputBuffer.pAddrPhyC = 0;

                            }
                            else
                            {
                                if (buffer_type != kMetadataBufferTypeCameraSource)
                                {
                                    logw("skip this input buffer, error buffer type: %d",
                                         buffer_type);
                                    pSelf->m_Callbacks.EmptyBufferDone(&pSelf->mOmxCmp,
                                                                       pSelf->m_pAppData,
                                                                       pInBufHdr);
                                    //pInBufHdr = NULL;
                                }
                                else
                                {
                                    memcpy(&sInputBuffer,
                                           (pInBufHdr->pBuffer+pInBufHdr->nOffset + 4),
                                           sizeof(VencInputBuffer));
                                    sInputBuffer.pAddrPhyC= sInputBuffer.pAddrPhyY +
                                        pSelf->m_sInPortDefType.format.video.nStride *
                                        pSelf->m_sInPortDefType.format.video.nFrameHeight;
                                }
                            }

                            //* clear flag
                            sInputBuffer.nFlag = 0;
                            sInputBuffer.nPts = pInBufHdr->nTimeStamp;
                            sInputBuffer.nID = (unsigned long)pInBufHdr;

                            if (pInBufHdr->nFlags & OMX_BUFFERFLAG_EOS)
                            {
                                sInputBuffer.nFlag |= VENC_BUFFERFLAG_EOS;
                            }

                            if (pSelf->mVideoExtParams.bEnableCropping)
                            {
                                if (pSelf->mVideoExtParams.ui16CropLeft ||
                                     pSelf->mVideoExtParams.ui16CropTop)
                                {
                                    sInputBuffer.bEnableCorp = 1;
                                    sInputBuffer.sCropInfo.nLeft =
                                        pSelf->mVideoExtParams.ui16CropLeft;
                                    sInputBuffer.sCropInfo.nWidth =
                                        pSelf->m_sOutPortDefType.format.video.nFrameWidth -
                                        pSelf->mVideoExtParams.ui16CropLeft -
                                        pSelf->mVideoExtParams.ui16CropRight;
                                    sInputBuffer.sCropInfo.nTop =
                                        pSelf->mVideoExtParams.ui16CropTop;
                                    sInputBuffer.sCropInfo.nHeight =
                                        pSelf->m_sOutPortDefType.format.video.nFrameHeight -
                                        pSelf->mVideoExtParams.ui16CropTop -
                                        pSelf->mVideoExtParams.ui16CropBottom;
                                }
                            }

                            result = AddOneInputBuffer(pSelf->m_encoder, &sInputBuffer);
                            if (result!=0)
                            {
                                nInputBufferStep = OMX_VENC_STEP_ADD_BUFFER_TO_ENC;
                            }
                            else
                            {
                                nInputBufferStep = OMX_VENC_STEP_GET_INPUTBUFFER;
                            }
#else
                            loge("do not support metadata input buffer");
#endif
                        }
                    }
                    else
                    {
                        int buffer_type =  *(int*)(pInBufHdr->pBuffer+pInBufHdr->nOffset);

                        //if (pSelf->m_useMetaDataInBuffers && buffer_type !=
                        //    kMetadataBufferTypeGrallocSource)
                        if (pInBufHdr->nFilledLen <= 0)
                        {
                            logw("skip this input buffer, pInBufHd:%p, buffer_type=%08x, \
                                     buf_size=%d",
                                 pInBufHdr,buffer_type,(int)pInBufHdr->nFilledLen);
                            pSelf->m_Callbacks.EmptyBufferDone(&pSelf->mOmxCmp,
                                                               pSelf->m_pAppData, pInBufHdr);
                            pInBufHdr = NULL;
                        }
                        else
                        {
                            result = GetOneAllocInputBuffer(pSelf->m_encoder, &sInputBuffer);

                            if (result !=0)
                            {
                                nInputBufferStep = OMX_VENC_STEP_GET_ALLOCBUFFER;
                            }
                            else
                            {
                                int size_y;
                                int size_c;
                                switch (pSelf->m_sInPortFormatType.eColorFormat)
                                {

                                    case OMX_COLOR_FormatYUV420SemiPlanar:
                                    {
                                        size_y = pSelf->m_sInPortDefType.format.video.nStride *
                                            pSelf->m_sInPortDefType.format.video.nFrameHeight;
                                        size_c = size_y>>1;
                                        break;
                                    }
                                    default:
                                    {
                                        size_y = pSelf->m_sInPortDefType.format.video.nStride *
                                            pSelf->m_sInPortDefType.format.video.nFrameHeight;
                                        size_c = size_y>>1;
                                        break;
                                    }
                                }


                                //* clear flag
                                sInputBuffer.nFlag = 0;
                                if (pInBufHdr->nFlags & OMX_BUFFERFLAG_EOS)
                                {
                                    sInputBuffer.nFlag |= VENC_BUFFERFLAG_EOS;
                                }

                                sInputBuffer.nPts = pInBufHdr->nTimeStamp;
                                sInputBuffer.bEnableCorp = 0;

                                if (pSelf->m_sInPortFormatType.eColorFormat ==
                                    OMX_COLOR_FormatAndroidOpaque)
                                {
#ifdef __ANDROID__
                                    void* bufAddr;
                                    buffer_handle_t bufferHanle;

                                    int width;
                                    int height;

                                    //* get the argb buffer.
                                    bufferHanle = *(buffer_handle_t*)(pInBufHdr->pBuffer +
                                                                      pInBufHdr->nOffset + 4);
                                    android::Rect rect(
                                        (int)pSelf->m_sInPortDefType.format.video.nStride,
                                        (int)pSelf->m_sInPortDefType.format.video.nFrameHeight);
                                    GraphicBufferMapper::get().lock(
                                         bufferHanle,
                                         (GRALLOC_USAGE_HW_VIDEO_ENCODER |
                                              GRALLOC_USAGE_SW_WRITE_OFTEN),
                                          rect, &bufAddr);

                                    width = pSelf->m_sInPortDefType.format.video.nStride;
                                    height = pSelf->m_sInPortDefType.format.video.nFrameHeight;

#ifdef CONF_ARMV7_A_NEON
                                    if (width % 32 == 0)
                                    {
                                        int widthandstride[2];
                                        unsigned char* addr[2];

                                        widthandstride[0] = width;
                                        widthandstride[1] = width;

                                        addr[0] = sInputBuffer.pAddrVirY;
                                        addr[1] = sInputBuffer.pAddrVirC;

                                        ImgRGBA2YUV420SP_neon((unsigned char *)bufAddr,
                                                              addr, widthandstride, height);
                                    }
                                    else
                                    {
                                        int widthandstride[2];
                                        unsigned char* addr[2];

                                        widthandstride[0] = width;
                                        widthandstride[1] = (width + 31) & (~31);

                                        addr[0] = sInputBuffer.pAddrVirY;
                                        addr[1] = sInputBuffer.pAddrVirC;

                                        ImgRGBA2YUV420SP_neon((unsigned char *)bufAddr, addr,
                                                              widthandstride, height);
                                    }
#endif

                                    CdcMemFlushCache(pSelf->memops, sInputBuffer.pAddrVirY,
                                                     width * height);
                                    CdcMemFlushCache(pSelf->memops, sInputBuffer.pAddrVirC,
                                                     width * height / 2);

                                    GraphicBufferMapper::get().unlock(bufferHanle);
#endif
                                }
                                else
                                {
                                    if (pSelf->mVideoExtParams.bEnableCropping)
                                    {
                                        if (pSelf->mVideoExtParams.ui16CropLeft ||
                                            pSelf->mVideoExtParams.ui16CropTop)
                                        {
                                            sInputBuffer.bEnableCorp = 1;
                                            sInputBuffer.sCropInfo.nLeft =
                                                pSelf->mVideoExtParams.ui16CropLeft;
                                            sInputBuffer.sCropInfo.nWidth =
                                                pSelf->m_sOutPortDefType.format.video.nFrameWidth -
                                                pSelf->mVideoExtParams.ui16CropLeft -
                                                pSelf->mVideoExtParams.ui16CropRight;
                                            sInputBuffer.sCropInfo.nTop =
                                                pSelf->mVideoExtParams.ui16CropTop;
                                            sInputBuffer.sCropInfo.nHeight =
                                                pSelf->m_sOutPortDefType.format.video.nFrameHeight -
                                                pSelf->mVideoExtParams.ui16CropTop -
                                                pSelf->mVideoExtParams.ui16CropBottom;
                                        }
                                    }
                                    memcpy(sInputBuffer.pAddrVirY,
                                           pInBufHdr->pBuffer + pInBufHdr->nOffset, size_y);
                                    memcpy(sInputBuffer.pAddrVirC,
                                           pInBufHdr->pBuffer + pInBufHdr->nOffset + size_y,
                                           size_c);
                                }

                                pSelf->m_Callbacks.EmptyBufferDone(&pSelf->mOmxCmp,
                                                                   pSelf->m_pAppData, pInBufHdr);

                                FlushCacheAllocInputBuffer(pSelf->m_encoder, &sInputBuffer);

                                result = AddOneInputBuffer(pSelf->m_encoder, &sInputBuffer);
                                if (result!=0)
                                {
                                    nInputBufferStep = OMX_VENC_STEP_ADD_BUFFER_TO_ENC;
                                }
                                else
                                {
                                    nInputBufferStep = OMX_VENC_STEP_GET_INPUTBUFFER;
                                }
                            }
                        }
                    }
                }
                else
                {
                    //* do nothing
                }
            }
            else if (nInputBufferStep == OMX_VENC_STEP_GET_ALLOCBUFFER)
            {
                result = GetOneAllocInputBuffer(pSelf->m_encoder, &sInputBuffer);

                if (result !=0)
                {
                    nInputBufferStep = OMX_VENC_STEP_GET_ALLOCBUFFER;
                }

                else
                {
                    int size_y;
                    int size_c;
                    switch (pSelf->m_sInPortFormatType.eColorFormat)
                    {
                        case OMX_COLOR_FormatYUV420SemiPlanar:
                        {
                            size_y = pSelf->m_sInPortDefType.format.video.nStride *
                                pSelf->m_sInPortDefType.format.video.nFrameHeight;
                            size_c = size_y>>1;
                            break;
                        }
                        default:
                        {
                            size_y = pSelf->m_sInPortDefType.format.video.nStride *
                                pSelf->m_sInPortDefType.format.video.nFrameHeight;
                            size_c = size_y>>1;
                            break;
                        }
                    }

                    //* clear flag
                    sInputBuffer.nFlag = 0;
                    if (pInBufHdr->nFlags & OMX_BUFFERFLAG_EOS)
                    {
                        sInputBuffer.nFlag |= VENC_BUFFERFLAG_EOS;
                    }

                    sInputBuffer.nPts = pInBufHdr->nTimeStamp;
                    sInputBuffer.bEnableCorp = 0;

                    if (pSelf->m_sInPortFormatType.eColorFormat == OMX_COLOR_FormatAndroidOpaque)
                    {
#ifdef __ANDROID__
                        void* bufAddr;
                        buffer_handle_t bufferHanle;

                        int width;
                        int height;

                        //* get the argb buffer.
                        bufferHanle = *(buffer_handle_t*)(pInBufHdr->pBuffer+pInBufHdr->nOffset
                                                          + 4);
                        android::Rect rect((int)pSelf->m_sInPortDefType.format.video.nStride,
                                          (int)pSelf->m_sInPortDefType.format.video.nFrameHeight);
                        GraphicBufferMapper::get().lock(bufferHanle,
                                                        (GRALLOC_USAGE_HW_VIDEO_ENCODER |
                                                         GRALLOC_USAGE_SW_WRITE_OFTEN),
                                                         rect, &bufAddr);

                        width = pSelf->m_sInPortDefType.format.video.nStride;
                        height = pSelf->m_sInPortDefType.format.video.nFrameHeight;

#ifdef CONF_ARMV7_A_NEON
                        if (width % 32 == 0)
                        {
                            int widthandstride[2];
                            unsigned char* addr[2];

                            widthandstride[0] = width;
                            widthandstride[1] = width;

                            addr[0] = sInputBuffer.pAddrVirY;
                            addr[1] = sInputBuffer.pAddrVirC;
                            ImgRGBA2YUV420SP_neon((unsigned char *)bufAddr, addr,
                                                  widthandstride, height);
                        }
                        else
                        {
                            int widthandstride[2];
                            unsigned char* addr[2];

                            widthandstride[0] = width;
                            widthandstride[1] = (width + 31) & (~31);

                            addr[0] = sInputBuffer.pAddrVirY;
                            addr[1] = sInputBuffer.pAddrVirC;

                            ImgRGBA2YUV420SP_neon((unsigned char *)bufAddr, addr,
                                                  widthandstride, height);
                        }
#endif

                        CdcMemFlushCache(pSelf->memops, sInputBuffer.pAddrVirY, width*height);
                        CdcMemFlushCache(pSelf->memops, sInputBuffer.pAddrVirC, width*height/2);

                        GraphicBufferMapper::get().unlock(bufferHanle);
#endif
                    }
                    else
                    {
                        memcpy(sInputBuffer.pAddrVirY,
                               pInBufHdr->pBuffer + pInBufHdr->nOffset, size_y);
                        memcpy(sInputBuffer.pAddrVirC,
                               pInBufHdr->pBuffer + pInBufHdr->nOffset + size_y, size_c);
                    }

                    pSelf->m_Callbacks.EmptyBufferDone(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                       pInBufHdr);

                    result = AddOneInputBuffer(pSelf->m_encoder, &sInputBuffer);
                    if (result!=0)
                    {
                        nInputBufferStep = OMX_VENC_STEP_ADD_BUFFER_TO_ENC;
                    }
                    else
                    {
                        nInputBufferStep = OMX_VENC_STEP_GET_INPUTBUFFER;
                    }
                }

                bNoNeedSleep = OMX_TRUE;
            }
            else if (nInputBufferStep == OMX_VENC_STEP_ADD_BUFFER_TO_ENC)
            {
                result = AddOneInputBuffer(pSelf->m_encoder, &sInputBuffer);
                if (result!=0)
                {
                    nInputBufferStep = OMX_VENC_STEP_ADD_BUFFER_TO_ENC;
                }
                else
                {
                    nInputBufferStep = OMX_VENC_STEP_GET_INPUTBUFFER;
                    bNoNeedSleep = OMX_TRUE;
                }
            }


            //* check used buffer
            if (0==AlreadyUsedInputBuffer(pSelf->m_encoder, &sInputBuffer_return))
            {
                bNoNeedSleep = OMX_TRUE;

                if (pSelf->m_useAllocInputBuffer)
                {
                    ReturnOneAllocInputBuffer(pSelf->m_encoder, &sInputBuffer_return);
                }
                else
                {
                    pInBufHdr = (OMX_BUFFERHEADERTYPE*)sInputBuffer_return.nID;
                    pSelf->m_Callbacks.EmptyBufferDone(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                       pInBufHdr);
                }
            }

            if (ValidBitstreamFrameNum(pSelf->m_encoder) > 0)
            {
                //* check output buffer
                pthread_mutex_lock(&pSelf->m_outBufMutex);

                if (pSelf->m_sOutBufList.nSizeOfList > 0)
                {
                    pSelf->m_sOutBufList.nSizeOfList--;
                    pOutBufHdr = pSelf->m_sOutBufList.pBufHdrList[pSelf->m_sOutBufList.nReadPos++];
                    if (pSelf->m_sOutBufList.nReadPos >= (int)pSelf->m_sOutBufList.nAllocSize)
                        pSelf->m_sOutBufList.nReadPos = 0;
                }
                else
                {
                    pOutBufHdr = NULL;
                }

                pthread_mutex_unlock(&pSelf->m_outBufMutex);

                if (pOutBufHdr)
                {
                    pOutBufHdr->nFlags &= ~OMX_BUFFERFLAG_CODECCONFIG;
                    pOutBufHdr->nFlags &= ~OMX_BUFFERFLAG_SYNCFRAME;

                    if (pSelf->m_firstFrameFlag && pSelf->m_vencCodecType == VENC_CODEC_H264)
                    {
                        pOutBufHdr->nTimeStamp = 0; //fixed it later;
                        pOutBufHdr->nFilledLen = pSelf->m_headdata.nLength;
                        pOutBufHdr->nOffset = 0;

                        memcpy(pOutBufHdr->pBuffer, pSelf->m_headdata.pBuffer,
                               pOutBufHdr->nFilledLen);
                        pSelf->m_firstFrameFlag = OMX_FALSE;
                        pOutBufHdr->nFlags |= OMX_BUFFERFLAG_CODECCONFIG;

#if SAVE_BITSTREAM
                        if (OutFile)
                        {
                            fwrite(pOutBufHdr->pBuffer, 1, pOutBufHdr->nFilledLen, OutFile);
                        }
                        else
                        {
                            logw("open outfile failed");
                        }
#endif

                        pSelf->m_Callbacks.FillBufferDone(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                          pOutBufHdr);
                        pOutBufHdr = NULL;
                    }
                    else
                    {
                        GetOneBitstreamFrame(pSelf->m_encoder, &sOutputBuffer);

                        pOutBufHdr->nTimeStamp = sOutputBuffer.nPts;
                        pOutBufHdr->nFilledLen = sOutputBuffer.nSize0 + sOutputBuffer.nSize1;
                        pOutBufHdr->nOffset = 0;
#if PRINTF_FRAME_SIZE
                        pSelf->mFrameCnt++;
                        pSelf->mAllFrameSize += pOutBufHdr->nFilledLen;

                        timeval cur_time;
                        gettimeofday(&cur_time, NULL);
                        const uint64_t now_time = cur_time.tv_sec * 1000000LL + cur_time.tv_usec;
                        if (((now_time-pSelf->mTimeStart)/1000) >= pSelf->mTimeOut)
                        {
                            int bitrate_real = (pSelf->mAllFrameSize) /
                                ((float)(now_time-pSelf->mTimeStart) / 1000000) * 8;
                            if (bitrate_real)
                            {
                                logd("venc bitrate real:%d,set:%ld , framerate real:%d,set:%ld , \
                                         avg framesize real:%d,set:%ld    ",
                                     bitrate_real,
                                     pSelf->m_sOutPortDefType.format.video.nBitrate,
                                     pSelf->mFrameCnt,
                                     pSelf->m_sInPortDefType.format.video.xFramerate >> 16,
                                     bitrate_real/pSelf->mFrameCnt,
                                     pSelf->m_sOutPortDefType.format.video.nBitrate /
                                         (pSelf->m_sInPortDefType.format.video.xFramerate));
                            }
                            pSelf->mTimeStart = now_time;
                            pSelf->mFrameCnt = 0;
                            pSelf->mAllFrameSize = 0;
                        }
#endif
                        pOutBufHdr->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;

                        if (sOutputBuffer.nFlag & VENC_BUFFERFLAG_KEYFRAME)
                        {
                            pOutBufHdr->nFlags |= OMX_BUFFERFLAG_SYNCFRAME;
                        }

                        if (sOutputBuffer.nFlag & VENC_BUFFERFLAG_EOS)
                        {
                            pOutBufHdr->nFlags |= OMX_BUFFERFLAG_EOS;
                        }

                        if (pSelf->m_prependSPSPPSToIDRFrames ==
                            OMX_TRUE && (sOutputBuffer.nFlag & VENC_BUFFERFLAG_KEYFRAME))
                        {
                            memcpy(pOutBufHdr->pBuffer, pSelf->m_headdata.pBuffer,
                                   pSelf->m_headdata.nLength);
                            memcpy(pOutBufHdr->pBuffer + pSelf->m_headdata.nLength,
                                   sOutputBuffer.pData0, sOutputBuffer.nSize0);
                            if (sOutputBuffer.nSize1)
                            {
                                memcpy((pOutBufHdr->pBuffer + pSelf->m_headdata.nLength +
                                        sOutputBuffer.nSize0),
                                       sOutputBuffer.pData1, sOutputBuffer.nSize1);
                            }

                            pOutBufHdr->nFilledLen += pSelf->m_headdata.nLength;
                        }
                        else
                        {
                            memcpy(pOutBufHdr->pBuffer, sOutputBuffer.pData0,
                                   sOutputBuffer.nSize0);
                            if (sOutputBuffer.nSize1)
                            {
                                memcpy(pOutBufHdr->pBuffer + sOutputBuffer.nSize0,
                                       sOutputBuffer.pData1, sOutputBuffer.nSize1);
                            }
                        }

#if SAVE_BITSTREAM
                        if (OutFile)
                        {
                            fwrite(pOutBufHdr->pBuffer, 1, pOutBufHdr->nFilledLen, OutFile);
                        }
                        else
                        {
                            logw("open outfile failed");
                        }
#endif

                        FreeOneBitStreamFrame(pSelf->m_encoder, &sOutputBuffer);

                        // Check for mark buffers
                        if (pMarkData != NULL && hMarkTargetComponent != NULL)
                        {
                            if (ValidBitstreamFrameNum(pSelf->m_encoder) == 0)
                            {
                                // Copy mark to output buffer header
                                pOutBufHdr->pMarkData = pInBufHdr->pMarkData;
                                // Copy handle to output buffer header
                                pOutBufHdr->hMarkTargetComponent = pInBufHdr->hMarkTargetComponent;

                                pMarkData = NULL;
                                hMarkTargetComponent = NULL;
                            }
                         }

                        pSelf->m_Callbacks.FillBufferDone(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                          pOutBufHdr);
                        pOutBufHdr = NULL;
                    }
                }
                else
                {
                    //* do nothing
                }
            }

            if (!bNoNeedSleep)
            {
                logv("need sleep");
                usleep(10*1000);
            }
            else
            {
                logv("no need sleep");
            }
        }
    }

EXIT:

    return (void*)OMX_ErrorNone;
}


static void* ComponentVencThread(void* pThreadData)
{
    int                     result = 0;
    int                     i;
    int                     fd1;
    fd_set                  rfds;
    OMX_VENC_COMMANDTYPE    cmd;
    OMX_BOOL                nEosFlag = OMX_FALSE;

    struct timeval          timeout;

    int nSemVal;
    int nRetSemGetValue;
    int nStopFlag = 0;
    int nWaitIdle = 0;

    // Recover the pointer to my component specific data
    omx_venc* pSelf = static_cast<omx_venc*>(pThreadData);

    while (1)
    {
        fd1 = pSelf->m_venc_cmdpipe[0];
        FD_ZERO(&rfds);
        FD_SET(fd1, &rfds);
        // Check for new command
        timeout.tv_sec  = 0;
        timeout.tv_usec = 0;

        i = select(pSelf->m_venc_cmdpipe[0]+1, &rfds, NULL, NULL, &timeout);

        if (FD_ISSET(pSelf->m_venc_cmdpipe[0], &rfds))
        {
            // retrieve command and data from pipe
            read(pSelf->m_venc_cmdpipe[0], &cmd, sizeof(cmd));
            logv("(f:%s, l:%d) vdrvThread receive cmd[0x%x]", __FUNCTION__, __LINE__, cmd);
            // State transition command

            switch (cmd)
            {
                case OMX_Venc_Cmd_Open:
                {
                    logv("(f:%s, l:%d) vencThread receive cmd[0x%x]", __FUNCTION__, __LINE__, cmd);
                    if (!pSelf->m_encoder)
                    {
                        openVencDriver(pSelf);
                    }
                    omx_sem_up(&pSelf->m_msg_sem);
                    logv("(f:%s, l:%d) vencThread receive cmd[0x%x]", __FUNCTION__, __LINE__, cmd);

                    break;
                }

                case OMX_Venc_Cmd_Close:
                {
                    logv("(f:%s, l:%d) vencThread receive cmd[0x%x]", __FUNCTION__, __LINE__, cmd);
                    if (pSelf->m_encoder)
                    {
                        closeVencDriver(pSelf);
                    }
                    omx_sem_up(&pSelf->m_msg_sem);
                    logv("(f:%s, l:%d) vencThread receive cmd[0x%x]", __FUNCTION__, __LINE__, cmd);
                    break;
                }

                case OMX_Venc_Cmd_Stop:
                {
                    logv("(f:%s, l:%d) vencThread receive cmd[0x%x]", __FUNCTION__, __LINE__, cmd);
                    nStopFlag = 1;
                    omx_sem_up(&pSelf->m_msg_sem);
                    logv("(f:%s, l:%d) vencThread receive cmd[0x%x]", __FUNCTION__, __LINE__, cmd);
                    break;
                }

                case OMX_Venc_Cmd_Enc_Idle:
                {
                    logv("(f:%s, l:%d) vencThread receive cmd[0x%x]", __FUNCTION__, __LINE__, cmd);
                    nWaitIdle = 1;
                    logv("(f:%s, l:%d) vencThread receive cmd[0x%x]", __FUNCTION__, __LINE__, cmd);
                    break;
                }

                case OMX_Venc_Cmd_ChangeBitrate:
                {
                    logd("pSelf->m_framerate: %d, bitrate: %d", (int)pSelf->m_framerate,
                         (int) pSelf->m_sOutPortDefType.format.video.nBitrate);
                    VideoEncSetParameter(pSelf->m_encoder, VENC_IndexParamBitrate,
                                         &pSelf->m_sOutPortDefType.format.video.nBitrate);
                    if (pSelf->mVideoSuperFrame.bEnable)
                    {
                        setSuperFrameCfg(pSelf);
                    }
                    break;
                }

                case OMX_Venc_Cmd_ChangeColorFormat:
                {
                    VideoEncSetParameter(pSelf->m_encoder, VENC_IndexParamColorFormat,
                                         &pSelf->m_vencColorFormat);
                    omx_sem_up(&pSelf->m_msg_sem);
                    break;
                }

                case OMX_Venc_Cmd_RequestIDRFrame:
                {
                    int value = 1;
                    VideoEncSetParameter(pSelf->m_encoder,VENC_IndexParamForceKeyFrame, &value);
                    logd("(f:%s, l:%d) OMX_Venc_Cmd_RequestIDRFrame[0x%x]",
                         __FUNCTION__, __LINE__, cmd);
                    break;
                }

                case OMX_Venc_Cmd_ChangeFramerate:
                {
                    logd("pSelf->m_framerate: %d, bitrate: %d", (int)pSelf->m_framerate,
                         (int)pSelf->m_sOutPortDefType.format.video.nBitrate);
                    VideoEncSetParameter(pSelf->m_encoder, VENC_IndexParamFramerate,
                                         &pSelf->m_framerate);
                    if (pSelf->mVideoSuperFrame.bEnable)
                        setSuperFrameCfg(pSelf);
                    setSVCSkipCfg(pSelf);
                    break;
                }
                default:
                {
                    logw("unknown cmd: %d", cmd);
                    break;
                }
            }
        }

        if (nStopFlag)
        {
            logd("vencThread detect nStopFlag[%d], exit!", (int)nStopFlag);
            goto EXIT;
        }

        if (pSelf->m_state == OMX_StateExecuting && pSelf->m_encoder)
        {
            #if (OPEN_STATISTICS)
            nTimeUs1 = GetNowUs();
            #endif
            result = VideoEncodeOneFrame(pSelf->m_encoder);
            #if (OPEN_STATISTICS)
            nTimeUs2 = GetNowUs();
            logw("MicH264Enc, VideoEncodeOneFrame time %lld",(nTimeUs2-nTimeUs1));
            #endif

            if (result == VENC_RESULT_ERROR)
            {
                pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                OMX_EventError, OMX_ErrorHardware,
                                                0 , NULL);
                loge("VideoEncodeOneFrame, failed, result: %d\n", result);
            }

            if (nWaitIdle && result == VENC_RESULT_NO_FRAME_BUFFER)
            {
                logv("input buffer idle \n");
                omx_sem_up(&pSelf->m_msg_sem);
                nWaitIdle = 0;
            }

            if (result != VENC_RESULT_OK)
            {
                waitPipeDataToRead(pSelf->m_venc_cmdpipe[0], 10 * 1000);
            }
        }
        else
        {
            waitPipeDataToRead(pSelf->m_venc_cmdpipe[0], 10 * 1000);
        }
    }

EXIT:
    return (void*)OMX_ErrorNone;
}
