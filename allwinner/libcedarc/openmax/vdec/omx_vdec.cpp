/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : omx_vdec.cpp
* Description :
* History :
*   Author  : AL3
*   Date    : 2013/05/05
*   Comment : complete the design code
*/

/*============================================================================
                            O p e n M A X   w r a p p e r s
                             O p e n  M A X   C o r e

*//** @file omx_vdec.cpp
  This module contains the implementation of the OpenMAX core & component.

*//*========================================================================*/

//////////////////////////////////////////////////////////////////////////////
//                             Include Files
//////////////////////////////////////////////////////////////////////////////

#define LOG_TAG "omx_vdec"
#include "log.h"

#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include "omx_vdec.h"
#include <fcntl.h>
#include "AWOMX_VideoIndexExtension.h"
#include "transform_color_format.h"

#include "memoryAdapter.h"
#include "vdecoder.h"
//#include "ve.h"
#include <CdcSysinfo.h>
#include <dlfcn.h>

#define SCALEDOWN_WHEN_RESOLUTION_MOER_THAN_1080P (1)

#ifdef __ANDROID__
    #include <binder/IPCThreadState.h>
    #include <media/stagefright/foundation/ADebug.h>
    #include <ui/GraphicBufferMapper.h>
    #include <ui/Rect.h>
    #include <HardwareAPI.h>
#endif

#define debug logi("LINE %d, FUNCTION %s", __LINE__, __FUNCTION__);

#define OPEN_STATISTICS (0)
#define PRINT_FRAME_CNT (50)
#define HW_VIDEO_CALL_APK "com.huawei.iptv.stb.videotalk.activity"

#define OMX_RESULT_OK    (0)
#define OMX_RESULT_ERROR (-1)
#define OMX_RESOLUTION_CHANGE (1)
#define OMX_RESULT_EXIT  (2)

#define ANDROID_SHMEM_ALIGN 32

#ifdef __ANDROID__
    using namespace android;
#endif

/*
 * Enumeration for the commands processed by the component
 */
typedef enum ThrCmdType
{
    MAIN_THREAD_CMD_SET_STATE    = 0,
    MAIN_THREAD_CMD_FLUSH        = 1,
    MAIN_THREAD_CMD_STOP_PORT    = 2,
    MAIN_THREAD_CMD_RESTART_PORT = 3,
    MAIN_THREAD_CMD_MARK_BUF     = 4,
    MAIN_THREAD_CMD_STOP         = 5,
    MAIN_THREAD_CMD_FILL_BUF     = 6,
    MAIN_THREAD_CMD_EMPTY_BUF    = 7,

    MAIN_THREAD_CMD_VDRV_NOTIFY_EOS        = 8,
    MAIN_THREAD_CMD_VDRV_RESOLUTION_CHANGE = 9,
} ThrCmdType;

typedef enum OMX_VDRV_COMMANDTYPE
{
    VDRV_THREAD_CMD_PREPARE_VDECODER = 0,
    VDRV_THREAD_CMD_CLOSE_VDECODER   = 1,
    VDRV_THREAD_CMD_FLUSH_VDECODER   = 2,
    VDRV_THREAD_CMD_STOP_VDECODER    = 3,
    VDRV_THREAD_CMD_EndOfStream      = 4,
} OMX_VDRV_COMMANDTYPE;

/* H.263 Supported Levels & profiles */
static VIDEO_PROFILE_LEVEL_TYPE SupportedH263ProfileLevels[] = {
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level10},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level20},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level30},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level40},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level45},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level50},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level60},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level70},
  {-1, -1}};

/* MPEG4 Supported Levels & profiles */
static VIDEO_PROFILE_LEVEL_TYPE SupportedMPEG4ProfileLevels[] ={
  {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level0},
  {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level0b},
  {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level1},
  {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level2},
  {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level3},
  {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level4},
  {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level4a},
  {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level5},
  {OMX_VIDEO_MPEG4ProfileAdvancedSimple, OMX_VIDEO_MPEG4Level0},
  {OMX_VIDEO_MPEG4ProfileAdvancedSimple, OMX_VIDEO_MPEG4Level0b},
  {OMX_VIDEO_MPEG4ProfileAdvancedSimple, OMX_VIDEO_MPEG4Level1},
  {OMX_VIDEO_MPEG4ProfileAdvancedSimple, OMX_VIDEO_MPEG4Level2},
  {OMX_VIDEO_MPEG4ProfileAdvancedSimple, OMX_VIDEO_MPEG4Level3},
  {OMX_VIDEO_MPEG4ProfileAdvancedSimple, OMX_VIDEO_MPEG4Level4},
  {OMX_VIDEO_MPEG4ProfileAdvancedSimple, OMX_VIDEO_MPEG4Level5},
  {-1,-1}};

/* AVC Supported Levels & profiles for cts */
static VIDEO_PROFILE_LEVEL_TYPE CTSSupportedAVCProfileLevels[] ={
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
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel32},

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

  {-1,-1}};

/* AVC Supported Levels & profiles */
static VIDEO_PROFILE_LEVEL_TYPE SupportedAVCProfileLevels[] ={
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
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel32},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel4},
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

  {-1,-1}};
/*
 *     M A C R O S
 */

/*
 * Initializes a data structure using a pointer to the structure.
 * The initialization of OMX structures always sets up the nSize and nVersion fields
 *   of the structure.
 */
#define OMX_CONF_INIT_STRUCT_PTR(_variable_, _struct_name_)    \
    memset((_variable_), 0x0, sizeof(_struct_name_));    \
    (_variable_)->nSize = sizeof(_struct_name_);        \
    (_variable_)->nVersion.s.nVersionMajor = 0x1;    \
    (_variable_)->nVersion.s.nVersionMinor = 0x1;    \
    (_variable_)->nVersion.s.nRevision = 0x0;        \
    (_variable_)->nVersion.s.nStep = 0x0

static VIDDEC_CUSTOM_PARAM sVideoDecCustomParams[] =
{
    {VIDDEC_CUSTOMPARAM_ENABLE_ANDROID_NATIVE_BUFFER,
        (OMX_INDEXTYPE)AWOMX_IndexParamVideoEnableAndroidNativeBuffers},
    {VIDDEC_CUSTOMPARAM_GET_ANDROID_NATIVE_BUFFER_USAGE,
        (OMX_INDEXTYPE)AWOMX_IndexParamVideoGetAndroidNativeBufferUsage},
    {VIDDEC_CUSTOMPARAM_USE_ANDROID_NATIVE_BUFFER2,
        (OMX_INDEXTYPE)AWOMX_IndexParamVideoUseAndroidNativeBuffer2},
    {VIDDEC_CUSTOMPARAM_STORE_META_DATA_IN_BUFFER,
        (OMX_INDEXTYPE)AWOMX_IndexParamVideoUseStoreMetaDataInBuffer},
    {VIDDEC_CUSTOMPARAM_PREPARE_FOR_ADAPTIVE_PLAYBACK,
        (OMX_INDEXTYPE)AWOMX_IndexParamVideoUsePrepareForAdaptivePlayback}
};

static void* RenderThread(void* pThreadData);
static void* ComponentThread(void* pThreadData);
static void* ComponentVdrvThread(void* pThreadData);
static inline void controlFillBuf(omx_vdec* pSelf, OMX_U32 cmddata);
static inline void controlEmptyBuf(omx_vdec* pSelf, OMX_U32 cmddata);

#ifdef __ANDROID__
    #define GET_CALLING_PID    (IPCThreadState::self()->getCallingPid())
    static void getCallingProcessName(char *name)
    {
        char proc_node[128];

        if (name == 0)
        {
            logd("error in params");
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
            logd("Obtain calling process failed");
        }
    }
#endif

//* factory function executed by the core to create instances
void *get_omx_component_factory_fn(void)
{
    return (new omx_vdec);
}

#if (OPEN_STATISTICS)
static int64_t OMX_GetNowUs() {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (int64_t)tv.tv_sec * 1000000ll + tv.tv_usec;
}
#endif

static inline void tryPostSem(sem_t* pSem)
{
    int nSemVal;
    int nRetSemGetValue;
    nRetSemGetValue=sem_getvalue(pSem, &nSemVal);
    if(0 == nRetSemGetValue)
    {
        if(0 == nSemVal)
        {
            sem_post(pSem);
        }
        else
        {
            logi("Be careful, sema frame_output[%d]!=0", nSemVal);
        }
    }
    else
    {
        logw("fatal error, why sem getvalue of m_sem_frame_output[%d] fail!",nRetSemGetValue);
    }
}

static OMX_U32 omx_vdec_align(unsigned int nOriginValue, int nAlign)
{
    return (nOriginValue + (nAlign-1)) & (~(nAlign-1));
}

omx_vdec::omx_vdec()
{
    logd("(f:%s, l:%d) ", __FUNCTION__, __LINE__);
    m_state              = OMX_StateLoaded;
    m_cRole[0]           = 0;
    m_cName[0]           = 0;
    m_eCompressionFormat = OMX_VIDEO_CodingUnused;
    m_pAppData           = NULL;
    m_thread_id          = 0;
    m_vdrv_thread_id     = 0;
    m_InputNum           = 0;
    m_OutputNum          = 0;
    m_maxWidth           = 0;
    m_maxHeight          = 0;
    m_decoder            = NULL;
    exitRender           = 0;

    memops =  MemAdapterGetOpsS();
    CdcMemOpen(memops);
    m_storeOutputMetaDataFlag = OMX_FALSE;
    m_useAndroidBuffer        = OMX_FALSE;
    m_JumpFlag                = OMX_FALSE;
    mIsFromCts                = OMX_FALSE;
    m_nInportPrevTimeStamp    = 0;
    mFirstInputDataFlag       = OMX_TRUE;
    mVp9orH265SoftDecodeFlag  = OMX_FALSE;
    mResolutionChangeFlag     = OMX_FALSE;
    nResolutionChangeNativeThreadFlag = OMX_FALSE;
    nResolutionChangeVdrvThreadFlag = OMX_FALSE;
    mIsCyber = false;
    mEmptyBufCnt = 0;
    mFillBufCnt = 0;
    mCropEnable = true;
    mGpuAlignStride = 32;
    nInBufEos       = OMX_FALSE;
    pMarkBuf             = NULL;
    pMarkData            = NULL;
    hMarkTargetComponent = NULL;
    port_setting_match   = OMX_TRUE;
    nVdrvNotifyEosFlag   = OMX_FALSE;
    nInputEosFlag        = OMX_FALSE;
    nVdrvResolutionChangeFlag = OMX_FALSE;

    memset(mCallingProcess, 0, sizeof(mCallingProcess));


#ifdef __ANDROID__
    getCallingProcessName(mCallingProcess);
    if((strcmp(mCallingProcess, "com.android.cts.media") == 0)
        || (strcmp(mCallingProcess, "com.android.cts.videoperf") == 0)
        || (strcmp(mCallingProcess, "com.android.pts.videoperf") == 0))
    {
        mIsFromCts           = true;
    }

    if((strcmp(mCallingProcess, "com.cloud.cyber") == 0))
    {
        mIsCyber = true;
        logd("+++ cyber apk");
    }
    if((strcmp(mCallingProcess, HW_VIDEO_CALL_APK) == 0))
    {
        mCropEnable = false;
        logd("mCropEnable = false");
    }
#endif

    mIsFirstGetOffset  = true;

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
    memset(&m_streamInfo, 0, sizeof(m_streamInfo));
    memset(&mVideoRect,0,sizeof(OMX_CONFIG_RECTTYPE));


    pthread_mutex_init(&m_inBufMutex, NULL);
    pthread_mutex_init(&m_outBufMutex, NULL);
    pthread_mutex_init(&m_pipeMutex, NULL);

    sem_init(&m_vdrv_cmd_lock,0,0);
    sem_init(&m_sem_vbs_input,0,0);
    sem_init(&m_sem_frame_output,0,0);

    mqMainThread = CdcMessageQueueCreate(64, "omx_vdec_new_mainThread");
    mqVdrvThread = CdcMessageQueueCreate(64, "omx_vdec_new_VdrvThread");
    if(mqMainThread == NULL || mqVdrvThread == NULL)
    {
        loge(" create mqMainThread[%p] or mqVdrvThread[%p] failed",mqMainThread, mqVdrvThread);
    }
}


omx_vdec::~omx_vdec()
{
    OMX_S32 nIndex;
    // In case the client crashes, check for nAllocSize parameter.
    // If this is greater than zero, there are elements in the list that are not free'd.
    // In that case, free the elements.
    logi("(f:%s, l:%d) ", __FUNCTION__, __LINE__);
    pthread_mutex_lock(&m_inBufMutex);

    for(nIndex=0; nIndex<m_sInBufList.nBufArrSize; nIndex++)
    {
        if(m_sInBufList.pBufArr==NULL)
        {
            break;
        }

        if(m_sInBufList.pBufArr[nIndex].pBuffer != NULL)
        {
            if(m_sInBufList.nAllocBySelfFlags & (1<<nIndex))
            {
                free(m_sInBufList.pBufArr[nIndex].pBuffer);
                m_sInBufList.pBufArr[nIndex].pBuffer = NULL;
            }
        }
    }

    if (m_sInBufList.pBufArr != NULL)
        free(m_sInBufList.pBufArr);

    if (m_sInBufList.pBufHdrList != NULL)
        free(m_sInBufList.pBufHdrList);

    memset(&m_sInBufList, 0, sizeof(struct _BufferList));
    m_sInBufList.nBufArrSize = m_sInPortDefType.nBufferCountActual;

    pthread_mutex_unlock(&m_inBufMutex);


    pthread_mutex_lock(&m_outBufMutex);

    for(nIndex=0; nIndex<m_sOutBufList.nBufArrSize; nIndex++)
    {
        if(m_sOutBufList.pBufArr[nIndex].pBuffer != NULL)
        {
            if(m_sOutBufList.nAllocBySelfFlags & (1<<nIndex))
            {
                free(m_sOutBufList.pBufArr[nIndex].pBuffer);
                m_sOutBufList.pBufArr[nIndex].pBuffer = NULL;
            }
        }
    }

    if (m_sOutBufList.pBufArr != NULL)
        free(m_sOutBufList.pBufArr);

    if (m_sOutBufList.pBufHdrList != NULL)
        free(m_sOutBufList.pBufHdrList);

    memset(&m_sOutBufList, 0, sizeof(struct _BufferList));
    m_sOutBufList.nBufArrSize = m_sOutPortDefType.nBufferCountActual;

    pthread_mutex_unlock(&m_outBufMutex);

    if(m_decoder != NULL)
    {
        DestroyVideoDecoder(m_decoder);
        m_decoder = NULL;
    }

    pthread_mutex_destroy(&m_inBufMutex);
    pthread_mutex_destroy(&m_outBufMutex);

    pthread_mutex_destroy(&m_pipeMutex);
    sem_destroy(&m_vdrv_cmd_lock);
    sem_destroy(&m_sem_vbs_input);
    sem_destroy(&m_sem_frame_output);

    if(m_streamInfo.pCodecSpecificData)
        free(m_streamInfo.pCodecSpecificData);
     if(memops)
        CdcMemClose(memops);

    if(mqMainThread)
        CdcMessageQueueDestroy(mqMainThread);

    if(mqVdrvThread)
        CdcMessageQueueDestroy(mqVdrvThread);

#if (OPEN_STATISTICS)
    if(mDecodeFrameTotalCount!=0 && mConvertTotalCount!=0)
    {
        mDecodeFrameSmallAverageDuration
            = (mDecodeFrameTotalDuration + mDecodeOKTotalDuration)/(mDecodeFrameTotalCount);
        mDecodeFrameBigAverageDuration
            = (mDecodeFrameTotalDuration + mDecodeOKTotalDuration \
               + mDecodeNoFrameTotalDuration + mDecodeNoBitstreamTotalDuration \
               + mDecodeOtherTotalDuration)/(mDecodeFrameTotalCount);
        if(mDecodeNoFrameTotalCount > 0)
        {
            mDecodeNoFrameAverageDuration
                = (mDecodeNoFrameTotalDuration)/(mDecodeNoFrameTotalCount);
        }
        else
        {
            mDecodeNoFrameAverageDuration = 0;
        }
        if(mDecodeNoBitstreamTotalCount > 0)
        {
            mDecodeNoBitstreamAverageDuration
                = (mDecodeNoBitstreamTotalDuration)/(mDecodeNoBitstreamTotalCount);
        }
        else
        {
            mDecodeNoBitstreamAverageDuration = 0;
        }
        mConvertAverageDuration = mConvertTotalDuration/mConvertTotalCount;
        logd("decode and convert statistics: \
            \n mDecodeFrameTotalDuration[%lld]ms, mDecodeOKTotalDuration[%lld]ms, \
               mDecodeNoFrameTotalDuration[%lld]ms, mDecodeNoBitstreamTotalDuration[%lld]ms, \
               mDecodeOtherTotalDuration[%lld]ms, \
            \n mDecodeFrameTotalCount[%lld], mDecodeOKTotalCount[%lld], \
               mDecodeNoFrameTotalCount[%lld], mDecodeNoBitstreamTotalCount[%lld], \
               mDecodeOtherTotalCount[%lld],\
            \n mDecodeFrameSmallAverageDuration[%lld]ms, mDecodeFrameBigAverageDuration[%lld]ms,\
               mDecodeNoFrameAverageDuration[%lld]ms, mDecodeNoBitstreamAverageDuration[%lld]ms\
            \n mConvertTotalDuration[%lld]ms, mConvertTotalCount[%lld],\
               mConvertAverageDuration[%lld]ms",
            mDecodeFrameTotalDuration/1000, mDecodeOKTotalDuration/1000,
            mDecodeNoFrameTotalDuration/1000, mDecodeNoBitstreamTotalDuration/1000,
            mDecodeOtherTotalDuration/1000,mDecodeFrameTotalCount,
            mDecodeOKTotalCount, mDecodeNoFrameTotalCount,
            mDecodeNoBitstreamTotalCount, mDecodeOtherTotalCount,
            mDecodeFrameSmallAverageDuration/1000, mDecodeFrameBigAverageDuration/1000,
            mDecodeNoFrameAverageDuration/1000, mDecodeNoBitstreamAverageDuration/1000,
            mConvertTotalDuration/1000, mConvertTotalCount, mConvertAverageDuration/1000);
    }
#endif

    logd("~omx_dec done!");
}

typedef void PluginInitFunc(void);

void InitPlugin(char *pName)
{
	PluginInitFunc *PluginInit = NULL;
	void *libFd = dlopen(pName,RTLD_NOW);
	if(libFd == NULL)
	{
		logd("load library %s fail!\n",pName);
	}
	else
	{
		logd("load library %s success!\n",pName);
	}

	PluginInit = (PluginInitFunc *)dlsym(libFd,"CedarPluginVDInit");
	if(PluginInit != NULL)
	{
		PluginInit();
	}
	else
	{
		logd("no CedarPluginVDInit in %s\n",pName);
	}
}

OMX_ERRORTYPE omx_vdec::component_init(OMX_STRING pName)
{
    OMX_ERRORTYPE eRet = OMX_ErrorNone;
    int           err;
    OMX_U32       nIndex;
    char calling_process[256];

    logi("(f:%s, l:%d) name = %s", __FUNCTION__, __LINE__, pName);
	AddVDPlugin();
    strncpy((char*)m_cName, pName, OMX_MAX_STRINGNAME_SIZE);


    if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.mjpeg",
        OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char *)m_cRole, "video_decoder.mjpeg", OMX_MAX_STRINGNAME_SIZE);
        m_eCompressionFormat      = OMX_VIDEO_CodingMJPEG;
        m_streamInfo.eCodecFormat = VIDEO_CODEC_FORMAT_MJPEG;
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.mpeg1",
             OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char *)m_cRole, "video_decoder.mpeg1", OMX_MAX_STRINGNAME_SIZE);
        m_eCompressionFormat      = OMX_VIDEO_CodingMPEG1;
        m_streamInfo.eCodecFormat = VIDEO_CODEC_FORMAT_MPEG1;
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.mpeg2",
             OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char *)m_cRole, "video_decoder.mpeg2", OMX_MAX_STRINGNAME_SIZE);
        m_eCompressionFormat      = OMX_VIDEO_CodingMPEG2;
        m_streamInfo.eCodecFormat = VIDEO_CODEC_FORMAT_MPEG2;
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.mpeg4",
             OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char*)m_cRole, "video_decoder.mpeg4", OMX_MAX_STRINGNAME_SIZE);
        m_eCompressionFormat      = OMX_VIDEO_CodingMPEG4;
        m_streamInfo.eCodecFormat = VIDEO_CODEC_FORMAT_XVID;
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.msmpeg4v1",
             OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char*)m_cRole, "video_decoder.msmpeg4v1", OMX_MAX_STRINGNAME_SIZE);
        m_eCompressionFormat      = OMX_VIDEO_CodingMSMPEG4V1;
        m_streamInfo.eCodecFormat = VIDEO_CODEC_FORMAT_MSMPEG4V1;
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.msmpeg4v2",
             OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char*)m_cRole, "video_decoder.msmpeg4v2", OMX_MAX_STRINGNAME_SIZE);
        m_eCompressionFormat      = OMX_VIDEO_CodingMSMPEG4V2;
        m_streamInfo.eCodecFormat = VIDEO_CODEC_FORMAT_MSMPEG4V2;
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.msmpeg4v3",
             OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char*)m_cRole, "video_decoder.msmpeg4v3", OMX_MAX_STRINGNAME_SIZE);
        m_eCompressionFormat      = OMX_VIDEO_CodingMSMPEG4V2;
        m_streamInfo.eCodecFormat = VIDEO_CODEC_FORMAT_DIVX3;
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.divx4",
             OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char*)m_cRole, "video_decoder.divx4", OMX_MAX_STRINGNAME_SIZE);
        m_eCompressionFormat      = OMX_VIDEO_CodingDIVX4;
        m_streamInfo.eCodecFormat = VIDEO_CODEC_FORMAT_DIVX4;
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.rx",
             OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char*)m_cRole, "video_decoder.rx", OMX_MAX_STRINGNAME_SIZE);
        m_eCompressionFormat      = OMX_VIDEO_CodingRX;
        m_streamInfo.eCodecFormat = VIDEO_CODEC_FORMAT_RX;
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.avs",
             OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char*)m_cRole, "video_decoder.avs", OMX_MAX_STRINGNAME_SIZE);
        m_eCompressionFormat      = OMX_VIDEO_CodingAVS;
        m_streamInfo.eCodecFormat = VIDEO_CODEC_FORMAT_AVS;
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.divx",
             OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char*)m_cRole, "video_decoder.divx", OMX_MAX_STRINGNAME_SIZE);
        m_eCompressionFormat      = OMX_VIDEO_CodingDIVX;
        m_streamInfo.eCodecFormat = VIDEO_CODEC_FORMAT_DIVX5;
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.xvid",
             OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char*)m_cRole, "video_decoder.xvid", OMX_MAX_STRINGNAME_SIZE);
        m_eCompressionFormat      = OMX_VIDEO_CodingXVID;
        m_streamInfo.eCodecFormat = VIDEO_CODEC_FORMAT_XVID;
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.h263",
             OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char *)m_cRole, "video_decoder.h263", OMX_MAX_STRINGNAME_SIZE);
        logi("\n H263 Decoder selected");
        m_eCompressionFormat      = OMX_VIDEO_CodingH263;
        m_streamInfo.eCodecFormat = VIDEO_CODEC_FORMAT_H263;
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.s263",
             OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char *)m_cRole, "video_decoder.s263", OMX_MAX_STRINGNAME_SIZE);
        logi("\n H263 Decoder selected");
        m_eCompressionFormat      = OMX_VIDEO_CodingS263;
        m_streamInfo.eCodecFormat = VIDEO_CODEC_FORMAT_SORENSSON_H263;
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.rxg2",
             OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char *)m_cRole, "video_decoder.rxg2", OMX_MAX_STRINGNAME_SIZE);
        logi("\n H263 Decoder selected");
        m_eCompressionFormat      = OMX_VIDEO_CodingRXG2;
        m_streamInfo.eCodecFormat = VIDEO_CODEC_FORMAT_RXG2;
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.wmv1",
             OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char *)m_cRole, "video_decoder.wmv1", OMX_MAX_STRINGNAME_SIZE);
        m_eCompressionFormat      = OMX_VIDEO_CodingWMV1;
        m_streamInfo.eCodecFormat = VIDEO_CODEC_FORMAT_WMV1;
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.wmv2",
             OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char *)m_cRole, "video_decoder.wmv2", OMX_MAX_STRINGNAME_SIZE);
        m_eCompressionFormat      = OMX_VIDEO_CodingWMV2;
        m_streamInfo.eCodecFormat = VIDEO_CODEC_FORMAT_WMV2;
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.vc1",
             OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char *)m_cRole, "video_decoder.vc1", OMX_MAX_STRINGNAME_SIZE);
        m_eCompressionFormat      = OMX_VIDEO_CodingWMV;
        m_streamInfo.eCodecFormat = VIDEO_CODEC_FORMAT_WMV3;
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.vp6",
             OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char *)m_cRole, "video_decoder.vp6", OMX_MAX_STRINGNAME_SIZE);
        m_eCompressionFormat      = OMX_VIDEO_CodingVP6; //OMX_VIDEO_CodingVPX
        m_streamInfo.eCodecFormat = VIDEO_CODEC_FORMAT_VP6;
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.vp8",
             OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char *)m_cRole, "video_decoder.vp8", OMX_MAX_STRINGNAME_SIZE);
        m_eCompressionFormat      = OMX_VIDEO_CodingVP8; //OMX_VIDEO_CodingVPX
        m_streamInfo.eCodecFormat = VIDEO_CODEC_FORMAT_VP8;
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.vp9",
             OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char *)m_cRole, "video_decoder.vp9", OMX_MAX_STRINGNAME_SIZE);
        m_eCompressionFormat      = OMX_VIDEO_CodingVP9; //OMX_VIDEO_CodingVPX
        m_streamInfo.eCodecFormat = VIDEO_CODEC_FORMAT_VP9;
        mVp9orH265SoftDecodeFlag  = OMX_TRUE;
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.avc",
             OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char *)m_cRole, "video_decoder.avc", OMX_MAX_STRINGNAME_SIZE);
        m_eCompressionFormat      = OMX_VIDEO_CodingAVC;
        m_streamInfo.eCodecFormat = VIDEO_CODEC_FORMAT_H264;
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.hevc",
             OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char *)m_cRole, "video_decoder.hevc", OMX_MAX_STRINGNAME_SIZE);
        logi("\n H263 Decoder selected");
        m_eCompressionFormat      = OMX_VIDEO_CodingHEVC;
        m_streamInfo.eCodecFormat = VIDEO_CODEC_FORMAT_H265;
        mVp9orH265SoftDecodeFlag  = OMX_TRUE;
    }
    else
    {
        logi("\nERROR:Unknown Component\n");
        eRet = OMX_ErrorInvalidComponentName;
        return eRet;
    }

    // Initialize component data structures to default values
    OMX_CONF_INIT_STRUCT_PTR(&m_sPortParam, OMX_PORT_PARAM_TYPE);
    m_sPortParam.nPorts           = 0x2;
    m_sPortParam.nStartPortNumber = 0x0;

    // Initialize the video parameters for input port
    OMX_CONF_INIT_STRUCT_PTR(&m_sInPortDefType, OMX_PARAM_PORTDEFINITIONTYPE);
    m_sInPortDefType.nPortIndex                      = 0x0;
    m_sInPortDefType.bEnabled                          = OMX_TRUE;
    m_sInPortDefType.bPopulated                      = OMX_FALSE;
    m_sInPortDefType.eDomain                          = OMX_PortDomainVideo;
    m_sInPortDefType.format.video.nFrameWidth          = 0;
    m_sInPortDefType.format.video.nFrameHeight          = 0;
    m_sInPortDefType.eDir                              = OMX_DirInput;
    m_sInPortDefType.nBufferCountMin                  = NUM_IN_BUFFERS;
    m_sInPortDefType.nBufferCountActual              = NUM_IN_BUFFERS;
#ifdef __ANDROID__
    getCallingProcessName(calling_process);
    if (strcmp(calling_process, HW_VIDEO_CALL_APK) == 0)
    {
        m_sInPortDefType.nBufferCountMin     = 8;
        m_sInPortDefType.nBufferCountActual = 8;
        logi("HW_VIDOE_CALL:%s,F:%s,L:%d",HW_VIDEO_CALL_APK,__FUNCTION__,__LINE__);
    }
#endif
    m_sInPortDefType.nBufferSize                      = OMX_VIDEO_DEC_INPUT_BUFFER_SIZE;
    m_sInPortDefType.format.video.eCompressionFormat = m_eCompressionFormat;
    m_sInPortDefType.format.video.cMIMEType          = (OMX_STRING)"";

    m_sInPortDefType.nBufferSize \
        = omx_vdec_align(m_sInPortDefType.nBufferSize, ANDROID_SHMEM_ALIGN);

    // Initialize the video parameters for output port
    OMX_CONF_INIT_STRUCT_PTR(&m_sOutPortDefType, OMX_PARAM_PORTDEFINITIONTYPE);
    m_sOutPortDefType.nPortIndex                      = 0x1;
    m_sOutPortDefType.bEnabled                          = OMX_TRUE;
    m_sOutPortDefType.bPopulated                      = OMX_FALSE;
    m_sOutPortDefType.eDomain                          = OMX_PortDomainVideo;
    m_sOutPortDefType.format.video.cMIMEType          = (OMX_STRING)"YUV420";
    m_sOutPortDefType.format.video.nFrameWidth          = 720;
    m_sOutPortDefType.format.video.nFrameHeight      = 480;
    m_sOutPortDefType.format.video.nStride = 720;
    m_sOutPortDefType.format.video.nSliceHeight = 480;
    m_sOutPortDefType.eDir                              = OMX_DirOutput;
    m_sOutPortDefType.nBufferCountMin                  = NUM_OUT_BUFFERS;
    m_sOutPortDefType.nBufferCountActual              = NUM_OUT_BUFFERS;
    if (strcmp(calling_process, HW_VIDEO_CALL_APK) == 0)
    {
        m_sOutPortDefType.nBufferCountMin = 6;
        m_sOutPortDefType.nBufferCountActual  = 6;
        logi("HW_VIDOE_CALL:%s,F:%s,L:%d",HW_VIDEO_CALL_APK,__FUNCTION__,__LINE__);
    }
    m_sOutPortDefType.nBufferSize
        = (OMX_U32)(m_sOutPortDefType.format.video.nFrameWidth*
                    m_sOutPortDefType.format.video.nFrameHeight*3/2);
    m_sOutPortDefType.format.video.eColorFormat      = OMX_COLOR_FormatYUV420Planar;

    // Initialize the video compression format for input port
    OMX_CONF_INIT_STRUCT_PTR(&m_sInPortFormatType, OMX_VIDEO_PARAM_PORTFORMATTYPE);
    m_sInPortFormatType.nPortIndex         = 0x0;
    m_sInPortFormatType.nIndex             = 0x0;
    m_sInPortFormatType.eCompressionFormat = m_eCompressionFormat;

    // Initialize the compression format for output port
    OMX_CONF_INIT_STRUCT_PTR(&m_sOutPortFormatType, OMX_VIDEO_PARAM_PORTFORMATTYPE);
    m_sOutPortFormatType.nPortIndex        = 0x1;
    m_sOutPortFormatType.nIndex            = 0x0;
    m_sOutPortFormatType.eColorFormat      = OMX_COLOR_FormatYUV420Planar;

    OMX_CONF_INIT_STRUCT_PTR(&m_sPriorityMgmtType, OMX_PRIORITYMGMTTYPE);

    OMX_CONF_INIT_STRUCT_PTR(&m_sInBufSupplierType, OMX_PARAM_BUFFERSUPPLIERTYPE );
    m_sInBufSupplierType.nPortIndex  = 0x0;

    OMX_CONF_INIT_STRUCT_PTR(&m_sOutBufSupplierType, OMX_PARAM_BUFFERSUPPLIERTYPE );
    m_sOutBufSupplierType.nPortIndex = 0x1;

    // Initialize the input buffer list
    memset(&(m_sInBufList), 0x0, sizeof(BufferList));

    m_sInBufList.pBufArr
        = (OMX_BUFFERHEADERTYPE*)malloc(sizeof(OMX_BUFFERHEADERTYPE) *
                                        m_sInPortDefType.nBufferCountActual);
    if(m_sInBufList.pBufArr == NULL)
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


    m_sInBufList.pBufHdrList
        = (OMX_BUFFERHEADERTYPE**)malloc(sizeof(OMX_BUFFERHEADERTYPE*) *
                                         m_sInPortDefType.nBufferCountActual);
    if(m_sInBufList.pBufHdrList == NULL)
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

    m_sOutBufList.pBufArr
        = (OMX_BUFFERHEADERTYPE*)malloc(sizeof(OMX_BUFFERHEADERTYPE) *
                                        m_sOutPortDefType.nBufferCountActual);
    if(m_sOutBufList.pBufArr == NULL)
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

    m_sOutBufList.pBufHdrList
        = (OMX_BUFFERHEADERTYPE**)malloc(sizeof(OMX_BUFFERHEADERTYPE*) *
                                         m_sOutPortDefType.nBufferCountActual);
    if(m_sOutBufList.pBufHdrList == NULL)
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

    //* create a decoder.

    /*
    m_decoder = CreateVideoDecoder();
    if(m_decoder == NULL)
    {
        logi(" can not create video decoder.");
        eRet = OMX_ErrorInsufficientResources;
        goto EXIT;
    }
    */

    //*set omx cts flag to flush the last frame in h264
    //m_decoder->ioctrl(m_decoder, CEDARV_COMMAND_SET_OMXCTS_DECODER, 1);

    // Create the component thread
    err = pthread_create(&m_thread_id, NULL, ComponentThread, this);
    if( err || !m_thread_id )
    {
        eRet = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    // Create vdrv thread
    err = pthread_create(&m_vdrv_thread_id, NULL, ComponentVdrvThread, this);
    if( err || !m_vdrv_thread_id )
    {
        eRet = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    err = pthread_create(&m_render_thread_id, NULL, RenderThread, this);
    if( err || !m_render_thread_id )
    {
        eRet = OMX_ErrorInsufficientResources;
        goto EXIT;
    }


    mDecodeFrameTotalDuration           = 0;
    mDecodeOKTotalDuration              = 0;
    mDecodeNoFrameTotalDuration         = 0;
    mDecodeNoBitstreamTotalDuration     = 0;
    mDecodeOtherTotalDuration           = 0;
    mDecodeFrameTotalCount              = 0;
    mDecodeOKTotalCount                 = 0;
    mDecodeNoFrameTotalCount            = 0;
    mDecodeNoBitstreamTotalCount        = 0;
    mDecodeOtherTotalCount              = 0;
    mDecodeFrameSmallAverageDuration    = 0;
    mDecodeFrameBigAverageDuration      = 0;
    mDecodeNoFrameAverageDuration       = 0;
    mDecodeNoBitstreamAverageDuration   = 0;

    mConvertTotalDuration               = 0;
    mConvertTotalCount                  = 0;
    mConvertAverageDuration             = 0;
EXIT:
    return eRet;
}


OMX_ERRORTYPE  omx_vdec::get_component_version(OMX_IN OMX_HANDLETYPE pHComp,
                                               OMX_OUT OMX_STRING pComponentName,
                                               OMX_OUT OMX_VERSIONTYPE* pComponentVersion,
                                               OMX_OUT OMX_VERSIONTYPE* pSpecVersion,
                                               OMX_OUT OMX_UUIDTYPE* pComponentUUID)
{
    CEDARC_UNUSE(pComponentUUID);
    logi("(f:%s, l:%d) ", __FUNCTION__, __LINE__);
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


OMX_ERRORTYPE  omx_vdec::send_command(OMX_IN OMX_HANDLETYPE  pHComp,
                                      OMX_IN OMX_COMMANDTYPE eCmd,
                                      OMX_IN OMX_U32         uParam1,
                                      OMX_IN OMX_PTR         pCmdData)
{
    CEDARC_UNUSE(pHComp);
    ThrCmdType    eCmdNative;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    logi("(f:%s, l:%d) ", __FUNCTION__, __LINE__);

    if(m_state == OMX_StateInvalid)
    {
        logd("ERROR: Send Command in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    if (eCmd == OMX_CommandMarkBuffer && pCmdData == NULL)
    {
        logd("ERROR: Send OMX_CommandMarkBuffer command but pCmdData invalid.");
        return OMX_ErrorBadParameter;
    }

    switch (eCmd)
    {
        case OMX_CommandStateSet:
            logi(" COMPONENT_SEND_COMMAND: OMX_CommandStateSet");
            eCmdNative = MAIN_THREAD_CMD_SET_STATE;
            break;

        case OMX_CommandFlush:
            logi(" COMPONENT_SEND_COMMAND: OMX_CommandFlush");
            eCmdNative = MAIN_THREAD_CMD_FLUSH;
            if ((int)uParam1 > 1 && (int)uParam1 != -1)
            {
                logd("Error: Send OMX_CommandFlush command but uParam1 invalid.");
                return OMX_ErrorBadPortIndex;
            }
            break;

        case OMX_CommandPortDisable:
            logi(" COMPONENT_SEND_COMMAND: OMX_CommandPortDisable");
            eCmdNative = MAIN_THREAD_CMD_STOP_PORT;
            break;

        case OMX_CommandPortEnable:
            logi(" COMPONENT_SEND_COMMAND: OMX_CommandPortEnable");
            eCmdNative = MAIN_THREAD_CMD_RESTART_PORT;
            break;

        case OMX_CommandMarkBuffer:
            logi(" COMPONENT_SEND_COMMAND: OMX_CommandMarkBuffer");
            eCmdNative = MAIN_THREAD_CMD_MARK_BUF;
             if (uParam1 > 0)
            {
                logd("Error: Send OMX_CommandMarkBuffer command but uParam1 invalid.");
                return OMX_ErrorBadPortIndex;
            }
            break;

        default:
            logw("(f:%s, l:%d) ignore other command[0x%x]", __FUNCTION__, __LINE__, eCmd);
            return OMX_ErrorBadParameter;
    }

    CdcMessage msg;
    memset(&msg, 0, sizeof(CdcMessage));
    msg.messageId = eCmdNative;
    msg.params[0] = (uintptr_t)uParam1;

    CdcMessageQueuePostMessage(mqMainThread, &msg);

    return eError;
}


OMX_ERRORTYPE  omx_vdec::get_parameter(OMX_IN OMX_HANDLETYPE pHComp,
                                       OMX_IN OMX_INDEXTYPE  eParamIndex,
                                       OMX_INOUT OMX_PTR     pParamData)
{
    CEDARC_UNUSE(pHComp);
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    logi("(f:%s, l:%d) eParamIndex = 0x%x", __FUNCTION__, __LINE__, eParamIndex);
    if(m_state == OMX_StateInvalid)
    {
        logi("Get Param in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    if(pParamData == NULL)
    {
        logi("Get Param in Invalid pParamData \n");
        return OMX_ErrorBadParameter;
    }

    switch(eParamIndex)
    {
        case OMX_IndexParamVideoInit:
        {
            logi(" COMPONENT_GET_PARAMETER: OMX_IndexParamVideoInit");
            memcpy(pParamData, &m_sPortParam, sizeof(OMX_PORT_PARAM_TYPE));
            break;
        }

        case OMX_IndexParamPortDefinition:
        {
            logi(" COMPONENT_GET_PARAMETER: OMX_IndexParamPortDefinition");
            if (((OMX_PARAM_PORTDEFINITIONTYPE *)(pParamData))->nPortIndex
                == m_sInPortDefType.nPortIndex)
            {
                memcpy(pParamData, &m_sInPortDefType, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
                logi(" get_OMX_IndexParamPortDefinition: m_sInPortDefType.nPortIndex[%d]",
                    (int)m_sInPortDefType.nPortIndex);
            }
            else if (((OMX_PARAM_PORTDEFINITIONTYPE*)(pParamData))->nPortIndex
                     == m_sOutPortDefType.nPortIndex)
            {
                memcpy(pParamData, &m_sOutPortDefType, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
                logi("(omx_vdec, f:%s, l:%d) OMX_IndexParamPortDefinition, width = %d, \
                      height = %d, nPortIndex[%d], nBufferCountActual[%d], \
                      nBufferCountMin[%d], nBufferSize[%d]", __FUNCTION__, __LINE__,
                    (int)m_sOutPortDefType.format.video.nFrameWidth,
                    (int)m_sOutPortDefType.format.video.nFrameHeight,
                    (int)m_sOutPortDefType.nPortIndex,
                    (int)m_sOutPortDefType.nBufferCountActual,
                    (int)m_sOutPortDefType.nBufferCountMin, (int)m_sOutPortDefType.nBufferSize);
            }
            else
            {
                eError = OMX_ErrorBadPortIndex;
                logw(" get_OMX_IndexParamPortDefinition: error. pParamData->nPortIndex=[%d]",
                     (int)((OMX_PARAM_PORTDEFINITIONTYPE*)(pParamData))->nPortIndex);
            }

            break;
        }

        case OMX_IndexParamVideoPortFormat:
        {
            logi(" COMPONENT_GET_PARAMETER: OMX_IndexParamVideoPortFormat");
             if (((OMX_VIDEO_PARAM_PORTFORMATTYPE *)(pParamData))->nPortIndex
                 == m_sInPortFormatType.nPortIndex)
             {
                if (((OMX_VIDEO_PARAM_PORTFORMATTYPE*)(pParamData))->nIndex
                     > m_sInPortFormatType.nIndex)
                {
                    eError = OMX_ErrorNoMore;
                }
                else
                {
                    memcpy(pParamData, &m_sInPortFormatType,
                           sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
                }
            }
            else if (((OMX_VIDEO_PARAM_PORTFORMATTYPE*)(pParamData))->nPortIndex
                    == m_sOutPortFormatType.nPortIndex)
            {
                if (((OMX_VIDEO_PARAM_PORTFORMATTYPE*)(pParamData))->nIndex
                    > m_sOutPortFormatType.nIndex)
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
                eError = OMX_ErrorBadPortIndex;

            logi("OMX_IndexParamVideoPortFormat, eError[0x%x]", eError);
            break;
        }

        case OMX_IndexParamStandardComponentRole:
        {
            logi(" COMPONENT_GET_PARAMETER: OMX_IndexParamStandardComponentRole");
            OMX_PARAM_COMPONENTROLETYPE* comp_role;

            comp_role                    = (OMX_PARAM_COMPONENTROLETYPE *) pParamData;
            comp_role->nVersion.nVersion = OMX_SPEC_VERSION;
            comp_role->nSize             = sizeof(*comp_role);

            strncpy((char*)comp_role->cRole, (const char*)m_cRole, OMX_MAX_STRINGNAME_SIZE);
            break;
        }

        case OMX_IndexParamPriorityMgmt:
        {
            logi(" COMPONENT_GET_PARAMETER: OMX_IndexParamPriorityMgmt");
            memcpy(pParamData, &m_sPriorityMgmtType, sizeof(OMX_PRIORITYMGMTTYPE));
            break;
        }

        case OMX_IndexParamCompBufferSupplier:
        {
            logi(" COMPONENT_GET_PARAMETER: OMX_IndexParamCompBufferSupplier");
            OMX_PARAM_BUFFERSUPPLIERTYPE* pBuffSupplierParam
                = (OMX_PARAM_BUFFERSUPPLIERTYPE*)pParamData;

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

        case OMX_IndexParamAudioInit:
        {
            logi(" COMPONENT_GET_PARAMETER: OMX_IndexParamAudioInit");
            OMX_PORT_PARAM_TYPE *audioPortParamType = (OMX_PORT_PARAM_TYPE *) pParamData;

            audioPortParamType->nVersion.nVersion = OMX_SPEC_VERSION;
            audioPortParamType->nSize             = sizeof(OMX_PORT_PARAM_TYPE);
            audioPortParamType->nPorts            = 0;
            audioPortParamType->nStartPortNumber  = 0;

            break;
        }

        case OMX_IndexParamImageInit:
        {
            logi(" COMPONENT_GET_PARAMETER: OMX_IndexParamImageInit");
            OMX_PORT_PARAM_TYPE *imagePortParamType = (OMX_PORT_PARAM_TYPE *) pParamData;

            imagePortParamType->nVersion.nVersion = OMX_SPEC_VERSION;
            imagePortParamType->nSize             = sizeof(OMX_PORT_PARAM_TYPE);
            imagePortParamType->nPorts            = 0;
            imagePortParamType->nStartPortNumber  = 0;

            break;
        }

        case OMX_IndexParamOtherInit:
        {
            logi(" COMPONENT_GET_PARAMETER: OMX_IndexParamOtherInit");
            OMX_PORT_PARAM_TYPE *otherPortParamType = (OMX_PORT_PARAM_TYPE *) pParamData;

            otherPortParamType->nVersion.nVersion = OMX_SPEC_VERSION;
            otherPortParamType->nSize             = sizeof(OMX_PORT_PARAM_TYPE);
            otherPortParamType->nPorts            = 0;
            otherPortParamType->nStartPortNumber  = 0;

            break;
        }

        case OMX_IndexParamVideoAvc:
        {
            logi(" COMPONENT_GET_PARAMETER: OMX_IndexParamVideoAvc");
            logi("get_parameter: OMX_IndexParamVideoAvc, do nothing.\n");
            break;
        }

        case OMX_IndexParamVideoH263:
        {
            logi(" COMPONENT_GET_PARAMETER: OMX_IndexParamVideoH263");
            logi("get_parameter: OMX_IndexParamVideoH263, do nothing.\n");
            break;
        }

        case OMX_IndexParamVideoMpeg4:
        {
            logi(" COMPONENT_GET_PARAMETER: OMX_IndexParamVideoMpeg4");
            logi("get_parameter: OMX_IndexParamVideoMpeg4, do nothing.\n");
            break;
        }
        case OMX_IndexParamVideoProfileLevelQuerySupported:
        {
            VIDEO_PROFILE_LEVEL_TYPE* pProfileLevel = NULL;
            OMX_U32 nNumberOfProfiles = 0;
            OMX_VIDEO_PARAM_PROFILELEVELTYPE *pParamProfileLevel
                = (OMX_VIDEO_PARAM_PROFILELEVELTYPE *)pParamData;

            pParamProfileLevel->nPortIndex = m_sInPortDefType.nPortIndex;

            /* Choose table based on compression format */
            switch(m_sInPortDefType.format.video.eCompressionFormat)
            {
            case OMX_VIDEO_CodingH263:
                pProfileLevel = SupportedH263ProfileLevels;
                nNumberOfProfiles
                    = sizeof(SupportedH263ProfileLevels) / sizeof (VIDEO_PROFILE_LEVEL_TYPE);
                break;

            case OMX_VIDEO_CodingMPEG4:
                pProfileLevel = SupportedMPEG4ProfileLevels;
                nNumberOfProfiles
                    = sizeof(SupportedMPEG4ProfileLevels) / sizeof (VIDEO_PROFILE_LEVEL_TYPE);
                break;

            case OMX_VIDEO_CodingAVC:
            if (mIsFromCts == true){
                pProfileLevel = CTSSupportedAVCProfileLevels;
                nNumberOfProfiles
                    = sizeof(CTSSupportedAVCProfileLevels) / sizeof (VIDEO_PROFILE_LEVEL_TYPE);
            }else{
                pProfileLevel = SupportedAVCProfileLevels;
                nNumberOfProfiles
                    = sizeof(SupportedAVCProfileLevels) / sizeof (VIDEO_PROFILE_LEVEL_TYPE);
            }
            break;

            default:
                logw("OMX_IndexParamVideoProfileLevelQuerySupported, Format[0x%x] not support",
                     m_sInPortDefType.format.video.eCompressionFormat);
                return OMX_ErrorBadParameter;
            }

            if(((int)pParamProfileLevel->nProfileIndex < 0)
                || (pParamProfileLevel->nProfileIndex >= (nNumberOfProfiles - 1)))
            {
                logw("pParamProfileLevel->nProfileIndex[0x%x] error!",
                     (unsigned int)pParamProfileLevel->nProfileIndex);
                return OMX_ErrorBadParameter;
            }

            /* Point to table entry based on uIndex */
            pProfileLevel += pParamProfileLevel->nProfileIndex;

            /* -1 indicates end of table */
            if(pProfileLevel->nProfile != -1)
            {
                pParamProfileLevel->eProfile = pProfileLevel->nProfile;
                pParamProfileLevel->eLevel = pProfileLevel->nLevel;
                eError = OMX_ErrorNone;
            }
            else
            {
                logw("pProfileLevel->nProfile error!");
                eError = OMX_ErrorNoMore;
            }

            break;
        }
        default:
        {
            if((AW_VIDEO_EXTENSIONS_INDEXTYPE)eParamIndex
                == AWOMX_IndexParamVideoGetAndroidNativeBufferUsage)
            {
                logi(" COMPONENT_GET_PARAMETER: AWOMX_IndexParamVideoGetAndroidNativeBufferUsage");
                break;
            }
            else
            {
                logi("get_parameter: unknown param %08x\n", eParamIndex);
                eError =OMX_ErrorUnsupportedIndex;
                break;
            }
        }
    }

    return eError;
}

OMX_ERRORTYPE  omx_vdec::set_parameter(OMX_IN OMX_HANDLETYPE pHComp,
                                       OMX_IN OMX_INDEXTYPE eParamIndex,
                                       OMX_IN OMX_PTR pParamData)
{
    CEDARC_UNUSE(pHComp);
    OMX_ERRORTYPE         eError      = OMX_ErrorNone;

    logi("(f:%s, l:%d) eParamIndex = 0x%x", __FUNCTION__, __LINE__, eParamIndex);
    if(m_state == OMX_StateInvalid)
    {
        logi("Set Param in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    if(pParamData == NULL)
    {
        logi("Get Param in Invalid pParamData \n");
        return OMX_ErrorBadParameter;
    }

    switch(eParamIndex)
    {
        case OMX_IndexParamPortDefinition:
        {
            logi(" COMPONENT_SET_PARAMETER: OMX_IndexParamPortDefinition");
            if (((OMX_PARAM_PORTDEFINITIONTYPE *)(pParamData))->nPortIndex
                 == m_sInPortDefType.nPortIndex)
            {
                logi("set_OMX_IndexParamPortDefinition, m_sInPortDefType.nPortIndex=%d",
                     (int)m_sInPortDefType.nPortIndex);

                //* TODO for any new IC
                OMX_PARAM_PORTDEFINITIONTYPE * para = (OMX_PARAM_PORTDEFINITIONTYPE *)pParamData;
                //* now only h3 support H265 4k decode
                if ((CdcGetSysinfoChipId() != SI_CHIP_H3)
                    && (para->format.video.eCompressionFormat == OMX_VIDEO_CodingHEVC)
                    && ((para->format.video.nFrameWidth > 1920)
                        || (para->format.video.nFrameHeight > 1088)))
                {
                    logw("ChipId: %d, H265 decode do not support over 1080p,\
                          but this video is %lux%lu",
                        CdcGetSysinfoChipId(), para->format.video.nFrameWidth,
                        para->format.video.nFrameHeight);
                    return OMX_ErrorUnsupportedSetting;
                }

                //* H3s/H2 do not support 4k decode
                if ((CdcGetSysinfoChipId() == SI_CHIP_H3s || CdcGetSysinfoChipId() == SI_CHIP_H2)
                    && ((para->format.video.nFrameWidth > 1920)
                        || (para->format.video.nFrameHeight > 1088)))
                {
                    logw("ChipId: %d, decode do not support over 1080p, but this video is %lux%lu",
                        CdcGetSysinfoChipId(), para->format.video.nFrameWidth,
                        para->format.video.nFrameHeight);
                    return OMX_ErrorUnsupportedSetting;
                }

                if(((OMX_PARAM_PORTDEFINITIONTYPE *)(pParamData))->nBufferCountActual
                    != m_sInPortDefType.nBufferCountActual)
                {
                    int nBufCnt;
                    int nIndex;


                    pthread_mutex_lock(&m_inBufMutex);

                    if(m_sInBufList.pBufArr != NULL)
                        free(m_sInBufList.pBufArr);

                    if(m_sInBufList.pBufHdrList != NULL)
                        free(m_sInBufList.pBufHdrList);

                    nBufCnt = ((OMX_PARAM_PORTDEFINITIONTYPE *)(pParamData))->nBufferCountActual;
                    logi("x allocate %d buffers.", nBufCnt);

                    m_sInBufList.pBufArr
                        = (OMX_BUFFERHEADERTYPE*)malloc(sizeof(OMX_BUFFERHEADERTYPE)* nBufCnt);
                    m_sInBufList.pBufHdrList
                        = (OMX_BUFFERHEADERTYPE**)malloc(sizeof(OMX_BUFFERHEADERTYPE*)* nBufCnt);
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

                m_streamInfo.nWidth  = m_sInPortDefType.format.video.nFrameWidth;
                m_streamInfo.nHeight = m_sInPortDefType.format.video.nFrameHeight;

                // the folllowing values are not right,
                // we should get offset in first time getVideoPicture
                mVideoRect.nTop    = 0;
                mVideoRect.nLeft   = 0;
                mVideoRect.nWidth  = m_sInPortDefType.format.video.nFrameWidth;
                mVideoRect.nHeight = m_sInPortDefType.format.video.nFrameHeight;

                // init vdecoder here
                if(mIsCyber)
                {
                    //*if mdecoder had closed before, we should create it
                    if(m_decoder==NULL)
                    {
                        //AddVDPlugin();
                        m_decoder = CreateVideoDecoder();
                    }

                    //* set video stream info.
                    VConfig m_videoConfig;
                    memset(&m_videoConfig,0,sizeof(VConfig));

                    //*set yv12
                    m_videoConfig.eOutputPixelFormat = PIXEL_FORMAT_YV12;

                    m_videoConfig.memops = MemAdapterGetOpsS();
                    //* palloc fbm in InitializeVideoDecode,
                    //  shorten the time of first frame decoding
                    m_videoConfig.bSupportPallocBufBeforeDecode = 1;
                    m_streamInfo.nWidth = m_sInPortDefType.format.video.nFrameWidth;
                    m_streamInfo.nHeight = m_sInPortDefType.format.video.nFrameHeight;
                    logd("input w: %d, m_streamInfo.nHeight: %d",
                         m_streamInfo.nWidth, m_streamInfo.nHeight);

                    //FIXME :m_useAndroidBuffer not set in this time
                    //if(m_useAndroidBuffer)
                    {
                        m_videoConfig.nAlignStride = mGpuAlignStride;
                    }

                    // omx decoder make out buffer no more than 1920x1080
                    if (m_streamInfo.nWidth > 1920
                        && m_streamInfo.nHeight > 1088)
                    {
                        m_videoConfig.bScaleDownEn = 1;
                        m_videoConfig.nHorizonScaleDownRatio = 1;
                        m_videoConfig.nVerticalScaleDownRatio = 1;
                    }

                    if(m_streamInfo.eCodecFormat == VIDEO_CODEC_FORMAT_WMV3
                        || m_streamInfo.eCodecFormat == VIDEO_CODEC_FORMAT_H264)
                    {
                        logd("*** pSelf->m_streamInfo.bIsFramePackage to 1 when it is vc1");
                        m_streamInfo.bIsFramePackage = 1;
                    }

                    // display error frame for cyber
                    m_videoConfig.bDispErrorFrame = 1;
                    logd("++++++++ m_videoConfig.bDispErrorFrame: %d",
                         m_videoConfig.bDispErrorFrame);
                    //*not use deinterlace
                    m_videoConfig.nDeInterlaceHoldingFrameBufferNum = 0;
                    //*gpu and decoder not share buffer
                    m_videoConfig.nDisplayHoldingFrameBufferNum     = 0;
                    m_videoConfig.nRotateHoldingFrameBufferNum
                        = NUM_OF_PICTURES_KEEPPED_BY_ROTATE;
                    m_videoConfig.nDecodeSmoothFrameBufferNum
                        = NUM_OF_PICTURES_FOR_EXTRA_SMOOTH;

                    if(InitializeVideoDecoder(m_decoder, &(m_streamInfo),&m_videoConfig) != 0)
                    {
                        DestroyVideoDecoder(m_decoder);
                        m_decoder           = NULL;
                        loge("Idle transition failed, set_vstream_info() return fail.\n");
                    }
                }
            }
            else if (((OMX_PARAM_PORTDEFINITIONTYPE *)(pParamData))->nPortIndex
                     == m_sOutPortDefType.nPortIndex)
            {
                logi("set_OMX_IndexParamPortDefinition, m_sOutPortDefType.nPortIndex=%d",
                     (int)m_sOutPortDefType.nPortIndex);
                if(((OMX_PARAM_PORTDEFINITIONTYPE *)(pParamData))->nBufferCountActual
                    != m_sOutPortDefType.nBufferCountActual)
                {
                    int nBufCnt;
                    int nIndex;

                    pthread_mutex_lock(&m_outBufMutex);

                    if(m_sOutBufList.pBufArr != NULL)
                        free(m_sOutBufList.pBufArr);

                    if(m_sOutBufList.pBufHdrList != NULL)
                        free(m_sOutBufList.pBufHdrList);

                    nBufCnt = ((OMX_PARAM_PORTDEFINITIONTYPE *)(pParamData))->nBufferCountActual;
                    logi("x allocate %d buffers.", nBufCnt);

                    //*Initialize the output buffer list
                    m_sOutBufList.pBufArr
                        = (OMX_BUFFERHEADERTYPE*) malloc(sizeof(OMX_BUFFERHEADERTYPE) * nBufCnt);
                    m_sOutBufList.pBufHdrList
                        = (OMX_BUFFERHEADERTYPE**) malloc(sizeof(OMX_BUFFERHEADERTYPE*) * nBufCnt);
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

                // set video width and height to 32 align
                if(mIsCyber)
                {
                    m_sOutPortDefType.format.video.nFrameWidth
                        = (m_sOutPortDefType.format.video.nFrameWidth+31) & ~31;
                    m_sOutPortDefType.format.video.nFrameHeight
                        = (m_sOutPortDefType.format.video.nFrameHeight+31) & ~31;
                }

                //check some key parameter
                OMX_U32 buffer_size = (m_sOutPortDefType.format.video.nFrameWidth *
                                       m_sOutPortDefType.format.video.nFrameHeight) * 3 / 2;
                if(buffer_size != m_sOutPortDefType.nBufferSize)
                {
                    logw("set_parameter, OMX_IndexParamPortDefinition, OutPortDef : change \
                          nBufferSize[%d] to [%d] to suit frame width[%d] and height[%d]",
                        (int)m_sOutPortDefType.nBufferSize, (int)buffer_size,
                        (int)m_sOutPortDefType.format.video.nFrameWidth,
                        (int)m_sOutPortDefType.format.video.nFrameHeight);
                    m_sOutPortDefType.nBufferSize = buffer_size;
                }

                logi("(omx_vdec, f:%s, l:%d) OMX_IndexParamPortDefinition, width = %d, \
                      height = %d, nPortIndex[%d], nBufferCountActual[%d], nBufferCountMin[%d], \
                      nBufferSize[%d]", __FUNCTION__, __LINE__,
                    (int)m_sOutPortDefType.format.video.nFrameWidth,
                    (int)m_sOutPortDefType.format.video.nFrameHeight,
                    (int)m_sOutPortDefType.nPortIndex,
                    (int)m_sOutPortDefType.nBufferCountActual,
                    (int)m_sOutPortDefType.nBufferCountMin, (int)m_sOutPortDefType.nBufferSize);
            }
            else
            {
                logw("set_OMX_IndexParamPortDefinition, error, paramPortIndex=%d",
                     (int)((OMX_PARAM_PORTDEFINITIONTYPE *)(pParamData))->nPortIndex);
                eError = OMX_ErrorBadPortIndex;
            }

           break;
        }

        case OMX_IndexParamVideoPortFormat:
        {
            logi(" COMPONENT_SET_PARAMETER: OMX_IndexParamVideoPortFormat");

            if (((OMX_VIDEO_PARAM_PORTFORMATTYPE *)(pParamData))->nPortIndex
                == m_sInPortFormatType.nPortIndex)
            {
                if (((OMX_VIDEO_PARAM_PORTFORMATTYPE *)(pParamData))->nIndex
                    > m_sInPortFormatType.nIndex)
                {
                    eError = OMX_ErrorNoMore;
                }
                else
                {
                    memcpy(&m_sInPortFormatType, pParamData,
                           sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
                }
            }
            else if (((OMX_VIDEO_PARAM_PORTFORMATTYPE*)(pParamData))->nPortIndex
                     == m_sOutPortFormatType.nPortIndex)
            {
                if (((OMX_VIDEO_PARAM_PORTFORMATTYPE*)(pParamData))->nIndex
                    > m_sOutPortFormatType.nIndex)
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
                eError = OMX_ErrorBadPortIndex;
            break;
        }

        case OMX_IndexParamStandardComponentRole:
        {
            logi(" COMPONENT_SET_PARAMETER: OMX_IndexParamStandardComponentRole");
            OMX_PARAM_COMPONENTROLETYPE *comp_role;
            comp_role = (OMX_PARAM_COMPONENTROLETYPE *) pParamData;
            logi("set_parameter: OMX_IndexParamStandardComponentRole %s\n", comp_role->cRole);

            if((m_state == OMX_StateLoaded)
                /* && !BITMASK_PRESENT(&m_flags,OMX_COMPONENT_IDLE_PENDING)*/)
            {
                logi("Set Parameter called in valid state");
            }
            else
            {
                logi("Set Parameter called in Invalid State\n");
                return OMX_ErrorIncorrectStateOperation;
            }

            if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.mjpeg",
                OMX_MAX_STRINGNAME_SIZE))
            {
                if(!strncmp((const char*)comp_role->cRole,"video_decoder.mjpeg",
                    OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"video_decoder.mjpeg",OMX_MAX_STRINGNAME_SIZE);
                }
                else
                {
                    logi("Setparameter: unknown uIndex %s\n", comp_role->cRole);
                    eError =OMX_ErrorUnsupportedSetting;
                }
            }
            else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.mpeg1",
                     OMX_MAX_STRINGNAME_SIZE))
            {
                if(!strncmp((char*)comp_role->cRole, "video_decoder.mpeg1",
                    OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"video_decoder.mpeg1", OMX_MAX_STRINGNAME_SIZE);
                }
                else
                {
                      logi("Setparameter: unknown uIndex %s\n", comp_role->cRole);
                      eError =OMX_ErrorUnsupportedSetting;
                }
            }
            else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.mpeg2",
                     OMX_MAX_STRINGNAME_SIZE))
            {
                if(!strncmp((char*)comp_role->cRole, "video_decoder.mpeg2",
                    OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"video_decoder.mpeg2", OMX_MAX_STRINGNAME_SIZE);
                }
                else
                {
                      logi("Setparameter: unknown uIndex %s\n", comp_role->cRole);
                      eError =OMX_ErrorUnsupportedSetting;
                }
            }
            else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.mpeg4",
                     OMX_MAX_STRINGNAME_SIZE))
            {
                if(!strncmp((const char*)comp_role->cRole,"video_decoder.mpeg4",
                    OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"video_decoder.mpeg4",OMX_MAX_STRINGNAME_SIZE);
                }
                else
                {
                    logi("Setparameter: unknown uIndex %s\n", comp_role->cRole);
                    eError = OMX_ErrorUnsupportedSetting;
                }
            }
            else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.msmpeg4v1",
                     OMX_MAX_STRINGNAME_SIZE))
            {
                if(!strncmp((const char*)comp_role->cRole,"video_decoder.msmpeg4v1",
                    OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"video_decoder.msmpeg4v1",OMX_MAX_STRINGNAME_SIZE);
                }
                else
                {
                    logi("Setparameter: unknown uIndex %s\n", comp_role->cRole);
                    eError = OMX_ErrorUnsupportedSetting;
                }
            }
            else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.msmpeg4v2",
                    OMX_MAX_STRINGNAME_SIZE))
            {
                if(!strncmp((const char*)comp_role->cRole,"video_decoder.msmpeg4v2",
                    OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"video_decoder.msmpeg4v2",OMX_MAX_STRINGNAME_SIZE);
                }
                else
                {
                    logi("Setparameter: unknown uIndex %s\n", comp_role->cRole);
                    eError = OMX_ErrorUnsupportedSetting;
                }
            }
            else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.msmpeg4v3",
                    OMX_MAX_STRINGNAME_SIZE))
            {
                if(!strncmp((const char*)comp_role->cRole,"video_decoder.msmpeg4v3",
                    OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"video_decoder.msmpeg4v3",OMX_MAX_STRINGNAME_SIZE);
                }
                else
                {
                    logi("Setparameter: unknown uIndex %s\n", comp_role->cRole);
                    eError = OMX_ErrorUnsupportedSetting;
                }
            }
            else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.divx4",
                    OMX_MAX_STRINGNAME_SIZE))
            {
                if(!strncmp((const char*)comp_role->cRole,"video_decoder.divx4",
                    OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"video_decoder.divx4",OMX_MAX_STRINGNAME_SIZE);
                }
                else
                {
                    logi("Setparameter: unknown uIndex %s\n", comp_role->cRole);
                    eError = OMX_ErrorUnsupportedSetting;
                }
            }
            else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.rx",
                    OMX_MAX_STRINGNAME_SIZE))
            {
                if(!strncmp((const char*)comp_role->cRole,"video_decoder.rx",
                    OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"video_decoder.rx",OMX_MAX_STRINGNAME_SIZE);
                }
                else
                {
                    logi("Setparameter: unknown uIndex %s\n", comp_role->cRole);
                    eError = OMX_ErrorUnsupportedSetting;
                }
            }
            else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.avs",
                    OMX_MAX_STRINGNAME_SIZE))
            {
                if(!strncmp((const char*)comp_role->cRole,"video_decoder.avs",
                    OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"video_decoder.avs",OMX_MAX_STRINGNAME_SIZE);
                }
                else
                {
                    logi("Setparameter: unknown uIndex %s\n", comp_role->cRole);
                    eError = OMX_ErrorUnsupportedSetting;
                }
            }
            else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.divx",
                    OMX_MAX_STRINGNAME_SIZE))
            {
                if(!strncmp((const char*)comp_role->cRole,"video_decoder.divx",
                    OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"video_decoder.divx",OMX_MAX_STRINGNAME_SIZE);
                }
                else
                {
                    logi("Setparameter: unknown uIndex %s\n", comp_role->cRole);
                    eError = OMX_ErrorUnsupportedSetting;
                }
            }
            else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.xvid",
                    OMX_MAX_STRINGNAME_SIZE))
            {
                if(!strncmp((const char*)comp_role->cRole,"video_decoder.xvid",
                    OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"video_decoder.xvid",OMX_MAX_STRINGNAME_SIZE);
                }
                else
                {
                    logi("Setparameter: unknown uIndex %s\n", comp_role->cRole);
                    eError = OMX_ErrorUnsupportedSetting;
                }
            }
            else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.h263",
                    OMX_MAX_STRINGNAME_SIZE))
            {
                if(!strncmp((const char*)comp_role->cRole,"video_decoder.h263",
                    OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"video_decoder.h263", OMX_MAX_STRINGNAME_SIZE);
                }
                else
                {
                    logi("Setparameter: unknown uIndex %s\n", comp_role->cRole);
                    eError =OMX_ErrorUnsupportedSetting;
                }
            }
            else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.s263",
                    OMX_MAX_STRINGNAME_SIZE))
            {
                if(!strncmp((const char*)comp_role->cRole,"video_decoder.s263",
                    OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"video_decoder.s263", OMX_MAX_STRINGNAME_SIZE);
                }
                else
                {
                    logi("Setparameter: unknown uIndex %s\n", comp_role->cRole);
                    eError =OMX_ErrorUnsupportedSetting;
                }
            }
            else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.rxg2",
                     OMX_MAX_STRINGNAME_SIZE))
            {
                if(!strncmp((const char*)comp_role->cRole,"video_decoder.rxg2",
                    OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"video_decoder.rxg2", OMX_MAX_STRINGNAME_SIZE);
                }
                else
                {
                    logi("Setparameter: unknown uIndex %s\n", comp_role->cRole);
                    eError =OMX_ErrorUnsupportedSetting;
                }
            }
            else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.wmv1",
                     OMX_MAX_STRINGNAME_SIZE))
            {
                if(!strncmp((const char*)comp_role->cRole,"video_decoder.wmv1",
                    OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"video_decoder.wmv1",OMX_MAX_STRINGNAME_SIZE);
                }
                else
                {
                    logi("Setparameter: unknown uIndex %s\n", comp_role->cRole);
                    eError =OMX_ErrorUnsupportedSetting;
                }
            }
            else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.wmv2",
                     OMX_MAX_STRINGNAME_SIZE))
            {
                if(!strncmp((const char*)comp_role->cRole,"video_decoder.wmv2",
                    OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"video_decoder.wmv2",OMX_MAX_STRINGNAME_SIZE);
                }
                else
                {
                    logi("Setparameter: unknown uIndex %s\n", comp_role->cRole);
                    eError =OMX_ErrorUnsupportedSetting;
                }
            }
            else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.vc1",
                     OMX_MAX_STRINGNAME_SIZE))
            {
                if(!strncmp((const char*)comp_role->cRole,"video_decoder.vc1",
                    OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"video_decoder.vc1",OMX_MAX_STRINGNAME_SIZE);
                }
                else
                {
                    logi("Setparameter: unknown uIndex %s\n", comp_role->cRole);
                    eError =OMX_ErrorUnsupportedSetting;
                }
            }
            else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.vp6",
                    OMX_MAX_STRINGNAME_SIZE))
            {
                if(!strncmp((char*)comp_role->cRole, "video_decoder.vp6",
                    OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"video_decoder.vp6", OMX_MAX_STRINGNAME_SIZE);
                }
                else
                {
                      logi("Setparameter: unknown uIndex %s\n", comp_role->cRole);
                      eError =OMX_ErrorUnsupportedSetting;
                }
            }
            else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.vp8",
                     OMX_MAX_STRINGNAME_SIZE))
            {
                if(!strncmp((char*)comp_role->cRole, "video_decoder.vp8",
                    OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"video_decoder.vp8", OMX_MAX_STRINGNAME_SIZE);
                }
                else
                {
                      logi("Setparameter: unknown uIndex %s\n", comp_role->cRole);
                      eError =OMX_ErrorUnsupportedSetting;
                }
            }
            else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.vp9",
                     OMX_MAX_STRINGNAME_SIZE))
            {
                if(!strncmp((char*)comp_role->cRole, "video_decoder.vp9",
                    OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"video_decoder.vp9", OMX_MAX_STRINGNAME_SIZE);
                }
                else
                {
                      logi("Setparameter: unknown uIndex %s\n", comp_role->cRole);
                      eError =OMX_ErrorUnsupportedSetting;
                }
            }
            else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.avc",
                    OMX_MAX_STRINGNAME_SIZE))
            {
                if(!strncmp((char*)comp_role->cRole, "video_decoder.avc",
                    OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"video_decoder.avc", OMX_MAX_STRINGNAME_SIZE);
                }
                else
                {
                      logi("Setparameter: unknown uIndex %s\n", comp_role->cRole);
                      eError =OMX_ErrorUnsupportedSetting;
                }
            }
            else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.hevc",
                    OMX_MAX_STRINGNAME_SIZE))
            {
                if(!strncmp((const char*)comp_role->cRole,"video_decoder.hevc",
                    OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)m_cRole,"video_decoder.hevc", OMX_MAX_STRINGNAME_SIZE);
                }
                else
                {
                    logi("Setparameter: unknown uIndex %s\n", comp_role->cRole);
                    eError =OMX_ErrorUnsupportedSetting;
                }
            }
            else
            {
                logi("Setparameter: unknown param %s\n", m_cName);
                eError = OMX_ErrorInvalidComponentName;
            }

            break;
        }

        case OMX_IndexParamPriorityMgmt:
        {
            logi(" COMPONENT_SET_PARAMETER: OMX_IndexParamPriorityMgmt");
            if(m_state != OMX_StateLoaded)
            {
                logi("Set Parameter called in Invalid State\n");
                return OMX_ErrorIncorrectStateOperation;
            }

            OMX_PRIORITYMGMTTYPE *priorityMgmtype = (OMX_PRIORITYMGMTTYPE*) pParamData;

            m_sPriorityMgmtType.nGroupID = priorityMgmtype->nGroupID;
            m_sPriorityMgmtType.nGroupPriority = priorityMgmtype->nGroupPriority;

            break;
        }

        case OMX_IndexParamCompBufferSupplier:
        {
            logi(" COMPONENT_SET_PARAMETER: OMX_IndexParamCompBufferSupplier");
            OMX_PARAM_BUFFERSUPPLIERTYPE *bufferSupplierType
                = (OMX_PARAM_BUFFERSUPPLIERTYPE*) pParamData;

            logi("set_parameter: OMX_IndexParamCompBufferSupplier %d\n",
                bufferSupplierType->eBufferSupplier);
            if(bufferSupplierType->nPortIndex == 0)
                m_sInBufSupplierType.eBufferSupplier = bufferSupplierType->eBufferSupplier;
            else if(bufferSupplierType->nPortIndex == 1)
                m_sOutBufSupplierType.eBufferSupplier = bufferSupplierType->eBufferSupplier;
            else
                eError = OMX_ErrorBadPortIndex;

            break;
        }

        case OMX_IndexParamVideoAvc:
        {
            logi(" COMPONENT_SET_PARAMETER: OMX_IndexParamVideoAvc");
              logi("set_parameter: OMX_IndexParamVideoAvc, do nothing.\n");
            break;
        }

        case OMX_IndexParamVideoH263:
        {
            logi(" COMPONENT_SET_PARAMETER: OMX_IndexParamVideoH263");
             logi("set_parameter: OMX_IndexParamVideoH263, do nothing.\n");
            break;
        }

        case OMX_IndexParamVideoMpeg4:
        {
            logi(" COMPONENT_SET_PARAMETER: OMX_IndexParamVideoMpeg4");
             logi("set_parameter: OMX_IndexParamVideoMpeg4, do nothing.\n");
            break;
        }

        default:
        {
#ifdef __ANDROID__
            if((AW_VIDEO_EXTENSIONS_INDEXTYPE)eParamIndex
                == AWOMX_IndexParamVideoUseAndroidNativeBuffer2)
            {
                logi(" COMPONENT_SET_PARAMETER: AWOMX_IndexParamVideoUseAndroidNativeBuffer2");
                logi("set_parameter: AWOMX_IndexParamVideoUseAndroidNativeBuffer2, do nothing.\n");
                m_useAndroidBuffer = OMX_TRUE;
                break;
            }
            else if((AW_VIDEO_EXTENSIONS_INDEXTYPE)eParamIndex
                    == AWOMX_IndexParamVideoEnableAndroidNativeBuffers)
            {
                logi(" COMPONENT_SET_PARAMETER: AWOMX_IndexParamVideoEnableAndroidNativeBuffers");
                logi("set_parameter: AWOMX_IndexParamVideoEnableAndroidNativeBuffers,\
                  set m_useAndroidBuffer to OMX_TRUE\n");

                EnableAndroidNativeBuffersParams *EnableAndroidBufferParams
                    =  (EnableAndroidNativeBuffersParams*) pParamData;
                logi(" enbleParam = %d\n",EnableAndroidBufferParams->enable);
                if(1==EnableAndroidBufferParams->enable)
                {
                    m_useAndroidBuffer = OMX_TRUE;
                }
                break;
            }
            else if((AW_VIDEO_EXTENSIONS_INDEXTYPE)eParamIndex
                    == AWOMX_IndexParamVideoUseStoreMetaDataInBuffer)
            {
                logi(" COMPONENT_SET_PARAMETER: AWOMX_IndexParamVideoUseStoreMetaDataInBuffer");

                StoreMetaDataInBuffersParams *pStoreMetaData
                    = (StoreMetaDataInBuffersParams*)pParamData;
                if(pStoreMetaData->nPortIndex != 1)
                {
                    logd("error: not support set AWOMX_IndexParamVideoUseStoreMetaDataInBuffer \
                         for inputPort");
                    eError = OMX_ErrorUnsupportedIndex;
                }
                if(pStoreMetaData->nPortIndex==1 && pStoreMetaData->bStoreMetaData==OMX_TRUE)
                {
                    logi("***set m_storeOutputMetaDataFlag to TRUE");
                    m_storeOutputMetaDataFlag = OMX_TRUE;
                }
            }
#ifdef CONF_KITKAT_AND_NEWER
            else if((AW_VIDEO_EXTENSIONS_INDEXTYPE)eParamIndex
                    ==AWOMX_IndexParamVideoUsePrepareForAdaptivePlayback)
            {
                logi(" COMPONENT_SET_PARAMETER: \
                     AWOMX_IndexParamVideoUsePrepareForAdaptivePlayback");

                PrepareForAdaptivePlaybackParams *pPlaybackParams;
                pPlaybackParams = (PrepareForAdaptivePlaybackParams *)pParamData;

                if(pPlaybackParams->nPortIndex==1 && pPlaybackParams->bEnable==OMX_TRUE)
                {
                    logi("set adaptive playback ,maxWidth = %d, maxHeight = %d",
                           (int)pPlaybackParams->nMaxFrameWidth,
                           (int)pPlaybackParams->nMaxFrameHeight);

                    m_maxWidth  = pPlaybackParams->nMaxFrameWidth;
                    m_maxHeight = pPlaybackParams->nMaxFrameHeight;
                }
            }
            #endif
            else
            {
                logi("Setparameter: unknown param %d\n", eParamIndex);
                eError = OMX_ErrorUnsupportedIndex;
                break;
            }
#else
            logi("Setparameter: unknown param %d\n", eParamIndex);
            eError = OMX_ErrorUnsupportedIndex;
            break;
#endif
        }
    }

    return eError;
}

OMX_ERRORTYPE  omx_vdec::get_config(OMX_IN OMX_HANDLETYPE pHComp,
                                    OMX_IN OMX_INDEXTYPE eConfigIndex,
                                    OMX_INOUT OMX_PTR pConfigData)
{
    CEDARC_UNUSE(pHComp);
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    logi("(f:%s, l:%d) uIndex = %d", __FUNCTION__, __LINE__, eConfigIndex);
    if (m_state == OMX_StateInvalid)
    {
        logi("get_config in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    switch (eConfigIndex)
    {
        case OMX_IndexConfigCommonOutputCrop:
        {
            if(!mCropEnable)
            {
                logi("get_config: unknown param %d\n",eConfigIndex);
                eError = OMX_ErrorUnsupportedIndex;
                break;
            }
            OMX_CONFIG_RECTTYPE *pRect = (OMX_CONFIG_RECTTYPE *)pConfigData;

            logw("+++++ get display crop: top[%d],left[%d],width[%d],height[%d]",
                  (int)mVideoRect.nTop,(int)mVideoRect.nLeft,
                  (int)mVideoRect.nWidth,(int)mVideoRect.nHeight);

            OMX_CONFIG_RECTTYPE    mCallbackVideoRect;
            memcpy(&mCallbackVideoRect, &mVideoRect, sizeof(OMX_CONFIG_RECTTYPE));

            //* on GPU_IMG, gpu will display (1*Height and 1*widht ) invaild data more.
            //* we avoid it here.
#ifdef CONFIG_IMG_GPU
            if(mVideoRect.nTop != 0)
            {
                mCallbackVideoRect.nTop    = mVideoRect.nTop    + 1;
                mCallbackVideoRect.nHeight = mVideoRect.nHeight - 1;
            }

            if(mVideoRect.nLeft != 0)
            {
                mCallbackVideoRect.nLeft    = mVideoRect.nLeft  + 1;
                mCallbackVideoRect.nWidth   = mVideoRect.nWidth - 1;
            }
            #endif

            if(mCallbackVideoRect.nWidth != 0 && mCallbackVideoRect.nHeight != 0)
            {
                memcpy(pRect,&mCallbackVideoRect,sizeof(OMX_CONFIG_RECTTYPE));
            }
            else
            {
                logw("the crop is invalid!");
                eError = OMX_ErrorUnsupportedIndex;
            }
            break;
        }

        default:
        {
            logi("get_config: unknown param %d\n",eConfigIndex);
            eError = OMX_ErrorUnsupportedIndex;
        }
    }

    return eError;
}

OMX_ERRORTYPE omx_vdec::set_config(OMX_IN OMX_HANDLETYPE pHComp,
                                    OMX_IN OMX_INDEXTYPE eConfigIndex,
                                    OMX_IN OMX_PTR pConfigData)
{
    CEDARC_UNUSE(pHComp);
    CEDARC_UNUSE(pConfigData);
    logi("(f:%s, l:%d) uIndex = %d", __FUNCTION__, __LINE__, eConfigIndex);

    if(m_state == OMX_StateInvalid)
    {
        logi("set_config in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    OMX_ERRORTYPE eError = OMX_ErrorNone;

    if (m_state == OMX_StateExecuting)
    {
        logi("set_config: Ignore in Executing state\n");
        return eError;
    }

    switch(eConfigIndex)
    {
        default:
        {
            eError = OMX_ErrorUnsupportedIndex;
        }
    }

    return eError;
}


OMX_ERRORTYPE  omx_vdec::get_extension_index(OMX_IN OMX_HANDLETYPE pHComp,
                                             OMX_IN OMX_STRING pParamName,
                                             OMX_OUT OMX_INDEXTYPE* pIndexType)
{
    unsigned int  nIndex;
    OMX_ERRORTYPE eError = OMX_ErrorUndefined;

    logi("(f:%s, l:%d) param name = %s", __FUNCTION__, __LINE__, pParamName);
    if(m_state == OMX_StateInvalid)
    {
        logi("Get Extension uIndex in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    if(pHComp == NULL)
        return OMX_ErrorBadParameter;

    for(nIndex = 0; nIndex < sizeof(sVideoDecCustomParams)/sizeof(VIDDEC_CUSTOM_PARAM); nIndex++)
    {
        if(strcmp((char *)pParamName, (char *)&(sVideoDecCustomParams[nIndex].cCustomParamName))
           == 0)
        {
            *pIndexType = sVideoDecCustomParams[nIndex].nCustomParamIndex;
            eError = OMX_ErrorNone;
            break;
        }
    }

    return eError;
}

OMX_ERRORTYPE omx_vdec::get_state(OMX_IN OMX_HANDLETYPE pHComp, OMX_OUT OMX_STATETYPE* pState)
{
    CEDARC_UNUSE(pHComp);
    logi("(f:%s, l:%d) ", __FUNCTION__, __LINE__);

    if(pHComp == NULL || pState == NULL)
        return OMX_ErrorBadParameter;

    *pState = m_state;
    logi("COMPONENT_GET_STATE, state[0x%x]", m_state);
    return OMX_ErrorNone;
}

OMX_ERRORTYPE omx_vdec::component_tunnel_request(OMX_IN    OMX_HANDLETYPE       pHComp,
                                                 OMX_IN    OMX_U32              uPort,
                                                 OMX_IN    OMX_HANDLETYPE       pPeerComponent,
                                                 OMX_IN    OMX_U32              uPeerPort,
                                                 OMX_INOUT OMX_TUNNELSETUPTYPE* pTunnelSetup)
{
    CEDARC_UNUSE(pHComp);
    CEDARC_UNUSE(uPort);
    CEDARC_UNUSE(pPeerComponent);
    CEDARC_UNUSE(uPeerPort);
    CEDARC_UNUSE(pTunnelSetup);
    logi(" COMPONENT_TUNNEL_REQUEST");

    logw("Error: component_tunnel_request Not Implemented\n");
    return OMX_ErrorNotImplemented;
}

OMX_ERRORTYPE  omx_vdec::use_buffer(OMX_IN    OMX_HANDLETYPE          pHComponent,
                                      OMX_INOUT OMX_BUFFERHEADERTYPE**  ppBufferHdr,
                                      OMX_IN    OMX_U32                 nPortIndex,
                                      OMX_IN    OMX_PTR                 pAppPrivate,
                                      OMX_IN    OMX_U32                 nSizeBytes,
                                      OMX_IN    OMX_U8*                 pBuffer)
{
    OMX_PARAM_PORTDEFINITIONTYPE*   pPortDef;
    OMX_U32                         nIndex = 0x0;

    logi("(f:%s, l:%d) PortIndex[%d], nSizeBytes[%d], pBuffer[%p]",
         __FUNCTION__, __LINE__, (int)nPortIndex, (int)nSizeBytes, pBuffer);
    if(pHComponent == NULL || ppBufferHdr == NULL || pBuffer == NULL)
    {
        return OMX_ErrorBadParameter;
    }

    if (nPortIndex == m_sInPortDefType.nPortIndex)
        pPortDef = &m_sInPortDefType;
    else if (nPortIndex == m_sOutPortDefType.nPortIndex)
        pPortDef = &m_sOutPortDefType;
    else
        return OMX_ErrorBadParameter;

    if (m_state!=OMX_StateLoaded
        && m_state!=OMX_StateWaitForResources
        && pPortDef->bEnabled!=OMX_FALSE)
    {
        logw("pPortDef[%d]->bEnabled=%d, m_state=0x%x, Can't use_buffer!",
              (int)nPortIndex, pPortDef->bEnabled, m_state);
        return OMX_ErrorIncorrectStateOperation;
    }
    logi("pPortDef[%d]->bEnabled=%d, m_state=0x%x, can use_buffer.",
          (int)nPortIndex, pPortDef->bEnabled, m_state);

    if(pPortDef->bPopulated)
        return OMX_ErrorBadParameter;

    // Find an empty position in the BufferList and allocate memory for the buffer header.
    // Use the buffer passed by the client to initialize the actual buffer
    // inside the buffer header.
    if (nPortIndex == m_sInPortDefType.nPortIndex)
    {
        if (nSizeBytes != pPortDef->nBufferSize)
              return OMX_ErrorBadParameter;

        logi("use_buffer, m_sInPortDefType.nPortIndex=[%d]", (int)m_sInPortDefType.nPortIndex);
        pthread_mutex_lock(&m_inBufMutex);

        if((OMX_S32)m_sInBufList.nAllocSize >= m_sInBufList.nBufArrSize)
        {
            pthread_mutex_unlock(&m_inBufMutex);
            return OMX_ErrorInsufficientResources;
        }

        nIndex = m_sInBufList.nAllocSize;
        m_sInBufList.nAllocSize++;

        m_sInBufList.pBufArr[nIndex].pBuffer          = pBuffer;
        m_sInBufList.pBufArr[nIndex].nAllocLen        = nSizeBytes;
        m_sInBufList.pBufArr[nIndex].pAppPrivate      = pAppPrivate;
        m_sInBufList.pBufArr[nIndex].nInputPortIndex  = nPortIndex;
        m_sInBufList.pBufArr[nIndex].nOutputPortIndex = 0xFFFFFFFE;
        *ppBufferHdr = &m_sInBufList.pBufArr[nIndex];
        if (m_sInBufList.nAllocSize == pPortDef->nBufferCountActual)
            pPortDef->bPopulated = OMX_TRUE;

        pthread_mutex_unlock(&m_inBufMutex);
    }
    else
    {
#ifdef CONF_KITKAT_AND_NEWER
        if(m_storeOutputMetaDataFlag==OMX_TRUE)
        {
            if(nSizeBytes != sizeof(VideoDecoderOutputMetaData))
                return OMX_ErrorBadParameter;
        }
        else
        {
            if(nSizeBytes != pPortDef->nBufferSize)
                  return OMX_ErrorBadParameter;
        }
#else
        if(nSizeBytes != pPortDef->nBufferSize)
                  return OMX_ErrorBadParameter;
#endif

        logi("use_buffer, m_sOutPortDefType.nPortIndex=[%d]",
              (int)m_sOutPortDefType.nPortIndex);
        pthread_mutex_lock(&m_outBufMutex);

        if((OMX_S32)m_sOutBufList.nAllocSize >= m_sOutBufList.nBufArrSize)
        {
            pthread_mutex_unlock(&m_outBufMutex);
            return OMX_ErrorInsufficientResources;
        }
        pPortDef->nBufferSize = nSizeBytes;
        nIndex = m_sOutBufList.nAllocSize;
        m_sOutBufList.nAllocSize++;

        m_sOutBufList.pBufArr[nIndex].pBuffer          = pBuffer;
        m_sOutBufList.pBufArr[nIndex].nAllocLen        = nSizeBytes;
        m_sOutBufList.pBufArr[nIndex].pAppPrivate      = pAppPrivate;
        m_sOutBufList.pBufArr[nIndex].nInputPortIndex  = 0xFFFFFFFE;
        m_sOutBufList.pBufArr[nIndex].nOutputPortIndex = nPortIndex;
        *ppBufferHdr = &m_sOutBufList.pBufArr[nIndex];
        if (m_sOutBufList.nAllocSize == pPortDef->nBufferCountActual)
            pPortDef->bPopulated = OMX_TRUE;

        pthread_mutex_unlock(&m_outBufMutex);
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE omx_vdec::allocate_buffer(OMX_IN    OMX_HANDLETYPE         pHComponent,
                                        OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                                        OMX_IN    OMX_U32                nPortIndex,
                                        OMX_IN    OMX_PTR                pAppPrivate,
                                        OMX_IN    OMX_U32                nSizeBytes)
{
    OMX_S8                        nIndex = 0x0;
    OMX_PARAM_PORTDEFINITIONTYPE* pPortDef;

    //logi(" COMPONENT_ALLOCATE_BUFFER");
    logi("(f:%s, l:%d) nPortIndex[%d], nSizeBytes[%d]",
           __FUNCTION__, __LINE__, (int)nPortIndex, (int)nSizeBytes);
    if(pHComponent == NULL || ppBufferHdr == NULL)
        return OMX_ErrorBadParameter;

    if (nPortIndex == m_sInPortDefType.nPortIndex)
        pPortDef = &m_sInPortDefType;
    else
    {
        if (nPortIndex == m_sOutPortDefType.nPortIndex)
            pPortDef = &m_sOutPortDefType;
        else
            return OMX_ErrorBadParameter;
    }

//    if (!pPortDef->bEnabled)
//        return OMX_ErrorIncorrectStateOperation;

    if (m_state != OMX_StateLoaded
        && m_state!=OMX_StateWaitForResources
        && pPortDef->bEnabled!=OMX_FALSE)
    {
        logw("pPortDef[%d]->bEnabled=%d, m_state=0x%x, Can't allocate_buffer!",
             (int)nPortIndex, pPortDef->bEnabled, m_state);
        return OMX_ErrorIncorrectStateOperation;
    }
    logi("pPortDef[%d]->bEnabled=%d, m_state=0x%x, can allocate_buffer.",
         (int)nPortIndex, pPortDef->bEnabled, m_state);

    if (nSizeBytes != pPortDef->nBufferSize || pPortDef->bPopulated)
        return OMX_ErrorBadParameter;

    // Find an empty position in the BufferList and allocate memory for the buffer header
    // and the actual buffer
    if (nPortIndex == m_sInPortDefType.nPortIndex)
    {
        logi("allocate_buffer, m_sInPortDefType.nPortIndex[%d]",
              (int)m_sInPortDefType.nPortIndex);
        pthread_mutex_lock(&m_inBufMutex);

        if((OMX_S32)m_sInBufList.nAllocSize >= m_sInBufList.nBufArrSize)
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

        m_sInBufList.pBufArr[nIndex].nAllocLen        = nSizeBytes;
        m_sInBufList.pBufArr[nIndex].pAppPrivate      = pAppPrivate;
        m_sInBufList.pBufArr[nIndex].nInputPortIndex  = nPortIndex;
        m_sInBufList.pBufArr[nIndex].nOutputPortIndex = 0xFFFFFFFE;
        *ppBufferHdr = &m_sInBufList.pBufArr[nIndex];

        m_sInBufList.nAllocSize++;

        if (m_sInBufList.nAllocSize == pPortDef->nBufferCountActual)
            pPortDef->bPopulated = OMX_TRUE;

        pthread_mutex_unlock(&m_inBufMutex);
    }
    else
    {
        logi("allocate_buffer, m_sOutPortDefType.nPortIndex[%d]",
              (int)m_sOutPortDefType.nPortIndex);
        pthread_mutex_lock(&m_outBufMutex);

        if((OMX_S32)m_sOutBufList.nAllocSize >= m_sOutBufList.nBufArrSize)
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
            pPortDef->bPopulated = OMX_TRUE;

        pthread_mutex_unlock(&m_outBufMutex);
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE omx_vdec::free_buffer(OMX_IN  OMX_HANDLETYPE        pHComponent,
                                    OMX_IN  OMX_U32               nPortIndex,
                                    OMX_IN  OMX_BUFFERHEADERTYPE* pBufferHdr)
{
    OMX_PARAM_PORTDEFINITIONTYPE* pPortDef;
    OMX_S32                       nIndex;

    logi("(f:%s, l:%d) nPortIndex = %d, pBufferHdr = %p, m_state=0x%x",
          __FUNCTION__, __LINE__, (int)nPortIndex, pBufferHdr, m_state);
    if(pHComponent == NULL || pBufferHdr == NULL)
        return OMX_ErrorBadParameter;

    // Match the pBufferHdr to the appropriate entry in the BufferList
    // and free the allocated memory
    if (nPortIndex == m_sInPortDefType.nPortIndex)
    {
        pPortDef = &m_sInPortDefType;

        pthread_mutex_lock(&m_inBufMutex);

        for(nIndex = 0; nIndex < m_sInBufList.nBufArrSize; nIndex++)
        {
            if(pBufferHdr == &m_sInBufList.pBufArr[nIndex])
                break;
        }

        if(nIndex == m_sInBufList.nBufArrSize)
        {
            pthread_mutex_unlock(&m_inBufMutex);
            return OMX_ErrorBadParameter;
        }

        if(m_sInBufList.nAllocBySelfFlags & (1<<nIndex))
        {
            free(m_sInBufList.pBufArr[nIndex].pBuffer);
            m_sInBufList.pBufArr[nIndex].pBuffer = NULL;
            m_sInBufList.nAllocBySelfFlags &= ~(1<<nIndex);
        }

        m_sInBufList.nAllocSize--;

        if(m_sInBufList.nAllocSize == 0)
            pPortDef->bPopulated = OMX_FALSE;

        pthread_mutex_unlock(&m_inBufMutex);
    }
    else if (nPortIndex == m_sOutPortDefType.nPortIndex)
    {
        pPortDef = &m_sOutPortDefType;

        pthread_mutex_lock(&m_outBufMutex);

        for(nIndex = 0; nIndex < m_sOutBufList.nBufArrSize; nIndex++)
        {
            logi("pBufferHdr = %p, &m_sOutBufList.pBufArr[%d] = %p",
                  pBufferHdr, (int)nIndex, &m_sOutBufList.pBufArr[nIndex]);
            if(pBufferHdr == &m_sOutBufList.pBufArr[nIndex])
                break;
        }

        logi("uIndex = %d", (int)nIndex);

        if(nIndex == m_sOutBufList.nBufArrSize)
        {
            pthread_mutex_unlock(&m_outBufMutex);
            return OMX_ErrorBadParameter;
        }

        if(m_sOutBufList.nAllocBySelfFlags & (1<<nIndex))
        {
            free(m_sOutBufList.pBufArr[nIndex].pBuffer);
            m_sOutBufList.pBufArr[nIndex].pBuffer = NULL;
            m_sOutBufList.nAllocBySelfFlags &= ~(1<<nIndex);
        }

        m_sOutBufList.nAllocSize--;

        if(m_sOutBufList.nAllocSize == 0)
            pPortDef->bPopulated = OMX_FALSE;

        pthread_mutex_unlock(&m_outBufMutex);
    }
    else
        return OMX_ErrorBadParameter;

    return OMX_ErrorNone;
}

OMX_ERRORTYPE  omx_vdec::empty_this_buffer(OMX_IN OMX_HANDLETYPE pHComponent,
                                           OMX_IN OMX_BUFFERHEADERTYPE* pBufferHdr)
{
    mEmptyBufCnt++;
    if(mEmptyBufCnt >= PRINT_FRAME_CNT)
    {
        logd("vdec, empty_this_buffer %d times",PRINT_FRAME_CNT);
        mEmptyBufCnt = 0;
    }
    logv("vdec ***emptyThisBuffer: pts = %lld , videoFormat = %d",
         pBufferHdr->nTimeStamp,
         m_eCompressionFormat);

    if(pHComponent == NULL || pBufferHdr == NULL)
        return OMX_ErrorBadParameter;

    if (!m_sInPortDefType.bEnabled)
        return OMX_ErrorIncorrectStateOperation;

    if (pBufferHdr->nInputPortIndex != 0x0  || pBufferHdr->nOutputPortIndex != OMX_NOPORT)
        return OMX_ErrorBadPortIndex;

    if (m_state != OMX_StateExecuting && m_state != OMX_StatePause)
        return OMX_ErrorIncorrectStateOperation;

    controlEmptyBuf(this,(OMX_U32)pBufferHdr);

    return OMX_ErrorNone;
}

OMX_ERRORTYPE  omx_vdec::fill_this_buffer(OMX_IN OMX_HANDLETYPE pHComponent,
                                          OMX_IN OMX_BUFFERHEADERTYPE* pBufferHdr)
{
    mFillBufCnt++;
    if(mFillBufCnt >= PRINT_FRAME_CNT)
    {
        logd("vdec, fill_this_buffer %d times",PRINT_FRAME_CNT);
        mFillBufCnt = 0;
    }
    logv("vdec (f:%s, l:%d) ", __FUNCTION__, __LINE__);
    if(pHComponent == NULL || pBufferHdr == NULL)
        return OMX_ErrorBadParameter;

    if (!m_sOutPortDefType.bEnabled)
        return OMX_ErrorIncorrectStateOperation;

    if (pBufferHdr->nOutputPortIndex != 0x1 || pBufferHdr->nInputPortIndex != OMX_NOPORT)
        return OMX_ErrorBadPortIndex;

    if (m_state != OMX_StateExecuting && m_state != OMX_StatePause)
        return OMX_ErrorIncorrectStateOperation;

    controlFillBuf(this,(OMX_U32)pBufferHdr);
    return OMX_ErrorNone;
}

OMX_ERRORTYPE  omx_vdec::set_callbacks(OMX_IN OMX_HANDLETYPE        pHComp,
                                           OMX_IN OMX_CALLBACKTYPE* pCallbacks,
                                           OMX_IN OMX_PTR           pAppData)
{
    logi("(f:%s, l:%d) ", __FUNCTION__, __LINE__);

    if(pHComp == NULL || pCallbacks == NULL || pAppData == NULL)
        return OMX_ErrorBadParameter;
    memcpy(&m_Callbacks, pCallbacks, sizeof(OMX_CALLBACKTYPE));
    m_pAppData = pAppData;

    return OMX_ErrorNone;
}

OMX_ERRORTYPE  omx_vdec::component_deinit(OMX_IN OMX_HANDLETYPE pHComp)
{
    //logi(" COMPONENT_DEINIT");
    CEDARC_UNUSE(pHComp);
    logi("(f:%s, l:%d) ", __FUNCTION__, __LINE__);
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_S32       nIndex = 0;

    // In case the client crashes, check for nAllocSize parameter.
    // If this is greater than zero, there are elements in the list that are not free'd.
    // In that case, free the elements.
    if (m_sInBufList.nAllocSize > 0)
    {
        for(nIndex=0; nIndex<m_sInBufList.nBufArrSize; nIndex++)
        {
            if(m_sInBufList.pBufArr[nIndex].pBuffer != NULL)
            {
                if(m_sInBufList.nAllocBySelfFlags & (1<<nIndex))
                {
                    free(m_sInBufList.pBufArr[nIndex].pBuffer);
                    m_sInBufList.pBufArr[nIndex].pBuffer = NULL;
                }
            }
        }

        if (m_sInBufList.pBufArr != NULL)
            free(m_sInBufList.pBufArr);

        if (m_sInBufList.pBufHdrList != NULL)
            free(m_sInBufList.pBufHdrList);

        memset(&m_sInBufList, 0, sizeof(struct _BufferList));
        m_sInBufList.nBufArrSize = m_sInPortDefType.nBufferCountActual;
    }

    if (m_sOutBufList.nAllocSize > 0)
    {
        for(nIndex=0; nIndex<m_sOutBufList.nBufArrSize; nIndex++)
        {
            if(m_sOutBufList.pBufArr[nIndex].pBuffer != NULL)
            {
                if(m_sOutBufList.nAllocBySelfFlags & (1<<nIndex))
                {
                    free(m_sOutBufList.pBufArr[nIndex].pBuffer);
                    m_sOutBufList.pBufArr[nIndex].pBuffer = NULL;
                }
            }
        }

        if (m_sOutBufList.pBufArr != NULL)
            free(m_sOutBufList.pBufArr);

        if (m_sOutBufList.pBufHdrList != NULL)
            free(m_sOutBufList.pBufHdrList);

        memset(&m_sOutBufList, 0, sizeof(struct _BufferList));
        m_sOutBufList.nBufArrSize = m_sOutPortDefType.nBufferCountActual;
    }

    CdcMessage msg;
    memset(&msg, 0, sizeof(CdcMessage));
    msg.messageId = MAIN_THREAD_CMD_STOP;

    CdcMessageQueuePostMessage(mqMainThread, &msg);

    // Wait for thread to exit so we can get the status into "error"
    pthread_join(m_thread_id, (void**)&eError);
    pthread_join(m_vdrv_thread_id, (void**)&eError);

    logd("(f:%s, l:%d) two threads exit!", __FUNCTION__, __LINE__);

    if(m_decoder != NULL)
    {
        DestroyVideoDecoder(m_decoder);
        m_decoder = NULL;
    }

    return eError;
}


OMX_ERRORTYPE  omx_vdec::use_EGL_image(OMX_IN OMX_HANDLETYPE               pHComp,
                                          OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                                          OMX_IN OMX_U32                   uPort,
                                          OMX_IN OMX_PTR                   pAppData,
                                          OMX_IN void*                     pEglImage)
{
    CEDARC_UNUSE(pHComp);
    CEDARC_UNUSE(ppBufferHdr);
    CEDARC_UNUSE(uPort);
    CEDARC_UNUSE(pAppData);
    CEDARC_UNUSE(pEglImage);
    logw("Error : use_EGL_image:  Not Implemented \n");
    return OMX_ErrorNotImplemented;
}


OMX_ERRORTYPE  omx_vdec::component_role_enum(OMX_IN  OMX_HANDLETYPE pHComp,
                                             OMX_OUT OMX_U8*        pRole,
                                             OMX_IN  OMX_U32        uIndex)
{
    CEDARC_UNUSE(pHComp);
    //logi(" COMPONENT_ROLE_ENUM");
    logi("(f:%s, l:%d) ", __FUNCTION__, __LINE__);
    OMX_ERRORTYPE eRet = OMX_ErrorNone;

    if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.mpeg4", OMX_MAX_STRINGNAME_SIZE))
    {
        if((0 == uIndex) && pRole)
        {
            strncpy((char *)pRole, "video_decoder.mpeg4", OMX_MAX_STRINGNAME_SIZE);
            logi("component_role_enum: pRole %s\n", pRole);
        }
        else
        {
            eRet = OMX_ErrorNoMore;
        }
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.h263", OMX_MAX_STRINGNAME_SIZE))
    {
        if((0 == uIndex) && pRole)
        {
            strncpy((char *)pRole, "video_decoder.h263",OMX_MAX_STRINGNAME_SIZE);
            logi("component_role_enum: pRole %s\n",pRole);
        }
        else
        {
            logi("\n No more roles \n");
            eRet = OMX_ErrorNoMore;
        }
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.avc", OMX_MAX_STRINGNAME_SIZE))
    {
        if((0 == uIndex) && pRole)
        {
            strncpy((char *)pRole, "video_decoder.avc",OMX_MAX_STRINGNAME_SIZE);
            logi("component_role_enum: pRole %s\n",pRole);
        }
        else
        {
            logi("\n No more roles \n");
            eRet = OMX_ErrorNoMore;
        }
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.vc1", OMX_MAX_STRINGNAME_SIZE))
    {
        if((0 == uIndex) && pRole)
        {
            strncpy((char *)pRole, "video_decoder.vc1",OMX_MAX_STRINGNAME_SIZE);
            logi("component_role_enum: pRole %s\n",pRole);
        }
        else
        {
            logi("\n No more roles \n");
            eRet = OMX_ErrorNoMore;
        }
    }
    else if(!strncmp((char*)m_cName, "OMX.allwinner.video.decoder.vp8", OMX_MAX_STRINGNAME_SIZE))
    {
        if((0 == uIndex) && pRole)
        {
            strncpy((char *)pRole, "video_decoder.vp8",OMX_MAX_STRINGNAME_SIZE);
            logi("component_role_enum: pRole %s\n",pRole);
        }
        else
        {
            logi("\n No more roles \n");
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

static inline void setStateLoaded(omx_vdec* pSelf)
{
    OMX_U32 nTimeout = 0x0;
    if (pSelf->m_state == OMX_StateIdle || pSelf->m_state == OMX_StateWaitForResources)
    {
        while (1)
        {
            //*Transition happens only when the ports are unpopulated
            if (!pSelf->m_sInPortDefType.bPopulated && !pSelf->m_sOutPortDefType.bPopulated)
            {
                pSelf->m_state = OMX_StateLoaded;
                pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                OMX_EventCmdComplete, OMX_CommandStateSet,
                                                pSelf->m_state, NULL);
                //* close decoder
                //* TODO.
                break;
             }
            else if (nTimeout++ > OMX_MAX_TIMEOUTS)
            {
                pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                OMX_EventError, OMX_ErrorInsufficientResources,
                                                0 , NULL);
                logw("Transition to loaded failed\n");
                break;
            }
            usleep(OMX_TIMEOUT*1000);
        }

        CdcMessage msg;
        memset(&msg, 0, sizeof(CdcMessage));
        msg.messageId = VDRV_THREAD_CMD_CLOSE_VDECODER;
        CdcMessageQueuePostMessage(pSelf->mqVdrvThread, &msg);
        sem_post(&pSelf->m_sem_vbs_input);
        sem_post(&pSelf->m_sem_frame_output);
        logd("(f:%s, l:%d) wait for VDRV_THREAD_CMD_CLOSE_VDECODER", __FUNCTION__, __LINE__);
        SemTimedWait(&pSelf->m_vdrv_cmd_lock, -1);
        logd("(f:%s, l:%d) wait for VDRV_THREAD_CMD_CLOSE_VDECODER done!", __FUNCTION__, __LINE__);
    }
    else
    {
        pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                        OMX_EventError, OMX_ErrorIncorrectStateTransition,
                                        0 , NULL);
    }

}
static inline void setStateIdle(omx_vdec* pSelf)
{
    OMX_U32 nTimeout = 0x0;

    if (pSelf->m_state == OMX_StateInvalid)
    {
        pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                        OMX_EventError, OMX_ErrorIncorrectStateTransition,
                                        0 , NULL);
    }
    else
    {
        //*Return buffers if currently in pause and executing
        if (pSelf->m_state == OMX_StatePause || pSelf->m_state == OMX_StateExecuting)
        {
            pthread_mutex_lock(&pSelf->m_inBufMutex);

            while (pSelf->m_sInBufList.nSizeOfList > 0)
            {
                pSelf->m_sInBufList.nSizeOfList--;
                pSelf->m_Callbacks.EmptyBufferDone(&pSelf->mOmxCmp,
                           pSelf->m_pAppData,
                           pSelf->m_sInBufList.pBufHdrList[pSelf->m_sInBufList.nReadPos++]);

                if (pSelf->m_sInBufList.nReadPos >= pSelf->m_sInBufList.nBufArrSize)
                    pSelf->m_sInBufList.nReadPos = 0;
            }

            pSelf->m_sInBufList.nReadPos  = 0;
            pSelf->m_sInBufList.nWritePos = 0;

            pthread_mutex_unlock(&pSelf->m_inBufMutex);


            pthread_mutex_lock(&pSelf->m_outBufMutex);

            while (pSelf->m_sOutBufList.nSizeOfList > 0)
            {
                pSelf->m_sOutBufList.nSizeOfList--;
                pSelf->m_Callbacks.FillBufferDone(&pSelf->mOmxCmp,
                           pSelf->m_pAppData,
                           pSelf->m_sOutBufList.pBufHdrList[pSelf->m_sOutBufList.nReadPos++]);

                if (pSelf->m_sOutBufList.nReadPos >= pSelf->m_sOutBufList.nBufArrSize)
                    pSelf->m_sOutBufList.nReadPos = 0;
            }

            pSelf->m_sOutBufList.nReadPos  = 0;
            pSelf->m_sOutBufList.nWritePos = 0;

            pthread_mutex_unlock(&pSelf->m_outBufMutex);
        }
        else    //OMX_StateLoaded -> OMX_StateIdle
        {
            //*we not create decoder here
            /*
            CdcMessage msg;
            memset(&msg, 0, sizeof(CdcMessage));
            msg.messageId = VDRV_THREAD_CMD_PREPARE_VDECODER;
            CdcMessageQueuePostMessage(pSelf->mqVdrvThread, &msg);
            sem_post(&pSelf->m_sem_vbs_input);
            sem_post(&pSelf->m_sem_frame_output);
            logd("(f:%s, l:%d) wait for VDRV_THREAD_CMD_PREPARE_VDECODER",
                  __FUNCTION__, __LINE__);
            SemTimedWait(&pSelf->m_vdrv_cmd_lock, -1);
            logd("(f:%s, l:%d) wait for VDRV_THREAD_CMD_PREPARE_VDECODER done!",
                  __FUNCTION__, __LINE__);
            */
        }

        while (1)
        {
            //*Ports have to be populated before transition completes
            if ((!pSelf->m_sInPortDefType.bEnabled && !pSelf->m_sOutPortDefType.bEnabled)   ||
                (pSelf->m_sInPortDefType.bPopulated && pSelf->m_sOutPortDefType.bPopulated))
            {
                pSelf->m_state = OMX_StateIdle;
                //*wake up vdrvThread
                sem_post(&pSelf->m_sem_vbs_input);
                sem_post(&pSelf->m_sem_frame_output);
                pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                OMX_EventCmdComplete, OMX_CommandStateSet,
                                                pSelf->m_state, NULL);

                //* Open decoder
                //* TODO.
                 break;
            }
            else if (nTimeout++ > OMX_MAX_TIMEOUTS)
            {
                pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                OMX_EventError, OMX_ErrorInsufficientResources,
                                                0 , NULL);
                logw("Idle transition failed\n");
                break;
            }

            usleep(OMX_TIMEOUT*1000);
        }
    }

}

static inline void setStateExecuting(omx_vdec* pSelf)
{
    //*Transition can only happen from pause or idle state
    if (pSelf->m_state == OMX_StateIdle || pSelf->m_state == OMX_StatePause)
    {
        //*Return buffers if currently in pause
        if (pSelf->m_state == OMX_StatePause)
        {
            pthread_mutex_lock(&pSelf->m_inBufMutex);

            while (pSelf->m_sInBufList.nSizeOfList > 0)
            {
                pSelf->m_sInBufList.nSizeOfList--;
                pSelf->m_Callbacks.EmptyBufferDone(&pSelf->mOmxCmp,
                           pSelf->m_pAppData,
                           pSelf->m_sInBufList.pBufHdrList[pSelf->m_sInBufList.nReadPos++]);

                if (pSelf->m_sInBufList.nReadPos >= pSelf->m_sInBufList.nBufArrSize)
                    pSelf->m_sInBufList.nReadPos = 0;
            }

            pSelf->m_sInBufList.nReadPos  = 0;
            pSelf->m_sInBufList.nWritePos = 0;

            pthread_mutex_unlock(&pSelf->m_inBufMutex);

            pthread_mutex_lock(&pSelf->m_outBufMutex);

            while (pSelf->m_sOutBufList.nSizeOfList > 0)
            {
                pSelf->m_sOutBufList.nSizeOfList--;
                pSelf->m_Callbacks.FillBufferDone(&pSelf->mOmxCmp,
                           pSelf->m_pAppData,
                           pSelf->m_sOutBufList.pBufHdrList[pSelf->m_sOutBufList.nReadPos++]);

                if (pSelf->m_sOutBufList.nReadPos >= pSelf->m_sOutBufList.nBufArrSize)
                    pSelf->m_sOutBufList.nReadPos = 0;
            }

            pSelf->m_sOutBufList.nReadPos  = 0;
            pSelf->m_sOutBufList.nWritePos = 0;

            pthread_mutex_unlock(&pSelf->m_outBufMutex);
        }

        pSelf->m_state = OMX_StateExecuting;
        pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                        OMX_EventCmdComplete, OMX_CommandStateSet,
                                        pSelf->m_state, NULL);
    }
    else
    {
        pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                        OMX_EventError, OMX_ErrorIncorrectStateTransition,
                                        0 , NULL);
    }

    pSelf->nInBufEos            = OMX_FALSE;
    pSelf->pMarkData            = NULL;
    pSelf->hMarkTargetComponent = NULL;
}

static inline void controlSetState(omx_vdec* pSelf, OMX_U32 cmddata)
{
    //*transitions/pCallbacks made based on state transition table
    // cmddata contains the target state
    switch ((OMX_STATETYPE)(cmddata))
    {
        case OMX_StateInvalid:
            pSelf->m_state = OMX_StateInvalid;
            pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                            OMX_EventError, OMX_ErrorInvalidState,
                                            0 , NULL);
            pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                            OMX_EventCmdComplete, OMX_CommandStateSet,
                                            pSelf->m_state, NULL);
            break;

        case OMX_StateLoaded:
            setStateLoaded(pSelf);
            break;

        case OMX_StateIdle:
            setStateIdle(pSelf);
            break;

        case OMX_StateExecuting:
            setStateExecuting(pSelf);
            break;

        case OMX_StatePause:
            // Transition can only happen from idle or executing state
            if (pSelf->m_state == OMX_StateIdle || pSelf->m_state == OMX_StateExecuting)
            {
                pSelf->m_state = OMX_StatePause;
                pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                OMX_EventCmdComplete, OMX_CommandStateSet,
                                                pSelf->m_state, NULL);
            }
            else
            {
                pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                            OMX_EventError, OMX_ErrorIncorrectStateTransition,
                                            0 , NULL);
            }

            break;

        case OMX_StateWaitForResources:
            if (pSelf->m_state == OMX_StateLoaded)
            {
                pSelf->m_state = OMX_StateWaitForResources;
                pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                            OMX_EventCmdComplete, OMX_CommandStateSet,
                                            pSelf->m_state, NULL);
            }
            else
            {
                pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                            OMX_EventError, OMX_ErrorIncorrectStateTransition,
                                            0 , NULL);
            }

            break;

        default:
            pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                            OMX_EventError,
                                            OMX_ErrorIncorrectStateTransition, 0 , NULL);
            break;

    }
}

static inline void controlStopPort(omx_vdec* pSelf, OMX_U32 cmddata)
{
    logd(" stop port command, cmddata = %d.", (int)cmddata);
    OMX_U32 nTimeout = 0x0;
    //*Stop Port(s)
    // cmddata contains the port index to be stopped.
    // It is assumed that 0 is input and 1 is output port for this component
    // The cmddata value -1 means that both input and output ports will be stopped.
    if (cmddata == 0x0 || (OMX_S32)cmddata == -1)
    {
        //*Return all input buffers
        pthread_mutex_lock(&pSelf->m_inBufMutex);

        while (pSelf->m_sInBufList.nSizeOfList > 0)
        {
            pSelf->m_sInBufList.nSizeOfList--;
            pSelf->m_Callbacks.EmptyBufferDone(&pSelf->mOmxCmp,
                           pSelf->m_pAppData,
                           pSelf->m_sInBufList.pBufHdrList[pSelf->m_sInBufList.nReadPos++]);

            if (pSelf->m_sInBufList.nReadPos >= pSelf->m_sInBufList.nBufArrSize)
                pSelf->m_sInBufList.nReadPos = 0;
        }

        pSelf->m_sInBufList.nReadPos  = 0;
        pSelf->m_sInBufList.nWritePos = 0;

        pthread_mutex_unlock(&pSelf->m_inBufMutex);

         //*Disable port
        pSelf->m_sInPortDefType.bEnabled = OMX_FALSE;
    }

    if (cmddata == 0x1 || (OMX_S32)cmddata == -1)
    {
        //*Return all output buffers
        pthread_mutex_lock(&pSelf->m_outBufMutex);

        while (pSelf->m_sOutBufList.nSizeOfList > 0)
        {
            pSelf->m_sOutBufList.nSizeOfList--;
            pSelf->m_Callbacks.FillBufferDone(&pSelf->mOmxCmp,
                           pSelf->m_pAppData,
                           pSelf->m_sOutBufList.pBufHdrList[pSelf->m_sOutBufList.nReadPos++]);

            if (pSelf->m_sOutBufList.nReadPos >= pSelf->m_sOutBufList.nBufArrSize)
                pSelf->m_sOutBufList.nReadPos = 0;
        }

        pSelf->m_sOutBufList.nReadPos  = 0;
        pSelf->m_sOutBufList.nWritePos = 0;
        pthread_mutex_unlock(&pSelf->m_outBufMutex);

           // Disable port
        pSelf->m_sOutPortDefType.bEnabled = OMX_FALSE;
    }

    // Wait for all buffers to be freed
    while (1)
    {
        if (cmddata == 0x0 && !pSelf->m_sInPortDefType.bPopulated)
        {
            //*Return cmdcomplete event if input unpopulated
            pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                            OMX_EventCmdComplete, OMX_CommandPortDisable,
                                            0x0, NULL);
            break;
        }

        if (cmddata == 0x1 && !pSelf->m_sOutPortDefType.bPopulated)
        {
            //*Return cmdcomplete event if output unpopulated
            pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                            OMX_EventCmdComplete, OMX_CommandPortDisable,
                                            0x1, NULL);
            break;
        }

        if ((OMX_S32)cmddata == -1
            && !pSelf->m_sInPortDefType.bPopulated
            && !pSelf->m_sOutPortDefType.bPopulated)
        {
            //*Return cmdcomplete event if inout & output unpopulated
            pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                            OMX_EventCmdComplete, OMX_CommandPortDisable,
                                            0x0, NULL);
            pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                            OMX_EventCmdComplete, OMX_CommandPortDisable,
                                            0x1, NULL);
            break;
        }

        //* Here , we sleep 500 ms.
        //* In the duration , if not all buffers be freed, we also return OMX_EventCmdComplete
        if (nTimeout++ > OMX_MAX_TIMEOUTS_STOP_PORT)
        {
            logw("timeOut when wait for all buffer to be freed in stopPort command! \
                  But we also return OMX_EventCmdComplete!");
            if(cmddata == 0x0)
            {
                pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                OMX_EventCmdComplete, OMX_CommandPortDisable,
                                                0x0, NULL);
                break;
            }
            else if(cmddata == 0x1)
            {
                pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                OMX_EventCmdComplete, OMX_CommandPortDisable,
                                                0x1, NULL);
                break;
            }
            else if((OMX_S32)cmddata == -1)
            {
                pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                OMX_EventCmdComplete, OMX_CommandPortDisable,
                                                0x0, NULL);
                pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                                OMX_EventCmdComplete, OMX_CommandPortDisable,
                                                0x1, NULL);
                break;
            }

            //pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
            //                                OMX_EventError,
            //                                OMX_ErrorPortUnresponsiveDuringDeallocation,
            //                                0 , NULL);
            //break;
        }

        usleep(OMX_TIMEOUT*1000);
    }
}

static inline void controlRestartPort(omx_vdec* pSelf, OMX_U32 cmddata)
{
    logd(" restart port command.pSelf->m_state[%d]", pSelf->m_state);

    //*Restart Port(s)
    // cmddata contains the port uIndex to be restarted.
    // It is assumed that 0 is input and 1 is output port for this component.
    // The cmddata value -1 means both input and output ports will be restarted.

    /*
    if (cmddata == 0x0 || (OMX_S32)cmddata == -1)
        pSelf->m_sInPortDefType.bEnabled = OMX_TRUE;

    if (cmddata == 0x1 || (OMX_S32)cmddata == -1)
        pSelf->m_sOutPortDefType.bEnabled = OMX_TRUE;
    */

    // Wait for port to be populated
    OMX_U32 nTimeout = 0x0;
    while (1)
    {
        // Return cmdcomplete event if input port populated
        if (cmddata == 0x0
            && (pSelf->m_state == OMX_StateLoaded || pSelf->m_sInPortDefType.bPopulated))
        {
            pSelf->m_sInPortDefType.bEnabled = OMX_TRUE;
            pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                            OMX_EventCmdComplete, OMX_CommandPortEnable,
                                            0x0, NULL);
            break;
        }
        // Return cmdcomplete event if output port populated
        else if (cmddata == 0x1
                && (pSelf->m_state == OMX_StateLoaded || pSelf->m_sOutPortDefType.bPopulated))
        {
            pSelf->m_sOutPortDefType.bEnabled = OMX_TRUE;
            pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                            OMX_EventCmdComplete, OMX_CommandPortEnable,
                                            0x1, NULL);
            break;
        }
        // Return cmdcomplete event if input and output ports populated
        else if ((OMX_S32)cmddata == -1
                && (pSelf->m_state == OMX_StateLoaded
                    || (pSelf->m_sInPortDefType.bPopulated
                        && pSelf->m_sOutPortDefType.bPopulated)))
        {
            pSelf->m_sInPortDefType.bEnabled = OMX_TRUE;
            pSelf->m_sOutPortDefType.bEnabled = OMX_TRUE;
            pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                            OMX_EventCmdComplete, OMX_CommandPortEnable,
                                            0x0, NULL);
            pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                            OMX_EventCmdComplete, OMX_CommandPortEnable,
                                            0x1, NULL);
            break;
        }
        else if (nTimeout++ > OMX_MAX_TIMEOUTS)
        {
            pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                        OMX_EventError,
                                        OMX_ErrorPortUnresponsiveDuringAllocation, 0, NULL);
            break;
        }

        usleep(OMX_TIMEOUT*1000);
    }

    if(pSelf->port_setting_match == OMX_FALSE)
        pSelf->port_setting_match = OMX_TRUE;
}

static inline void controlFlush(omx_vdec* pSelf, OMX_U32 cmddata)
{
    logd(" flush command.");

    //*Flush port(s)
    // cmddata contains the port uIndex to be flushed.
    // It is assumed that 0 is input and 1 is output port for this component
    // The cmddata value -1 means that both input and output ports will be flushed.

    //if request flush input and output port, we reset decoder!
    if(cmddata == OMX_ALL || cmddata == 0x1 || (OMX_S32)cmddata == -1)
    {
        logd(" flush all port! we reset decoder!");
        CdcMessage msg;
        memset(&msg, 0, sizeof(CdcMessage));
        msg.messageId = VDRV_THREAD_CMD_FLUSH_VDECODER;

        CdcMessageQueuePostMessage(pSelf->mqVdrvThread, &msg);
        sem_post(&pSelf->m_sem_vbs_input);
        sem_post(&pSelf->m_sem_frame_output);
        logd("(f:%s, l:%d) wait for VDRV_THREAD_CMD_FLUSH_VDECODER",
             __FUNCTION__, __LINE__);
        SemTimedWait(&pSelf->m_vdrv_cmd_lock, -1);
        logd("(f:%s, l:%d) wait for VDRV_THREAD_CMD_FLUSH_VDECODER done!",
              __FUNCTION__, __LINE__);
    }
    if (cmddata == 0x0 || (OMX_S32)cmddata == -1)
    {
        // Return all input buffers and send cmdcomplete
        pthread_mutex_lock(&pSelf->m_inBufMutex);

        while (pSelf->m_sInBufList.nSizeOfList > 0)
        {
            pSelf->m_sInBufList.nSizeOfList--;
            pSelf->m_Callbacks.EmptyBufferDone(&pSelf->mOmxCmp,
                       pSelf->m_pAppData,
                       pSelf->m_sInBufList.pBufHdrList[pSelf->m_sInBufList.nReadPos++]);

            if (pSelf->m_sInBufList.nReadPos >= pSelf->m_sInBufList.nBufArrSize)
                pSelf->m_sInBufList.nReadPos = 0;
        }

        pSelf->m_sInBufList.nReadPos  = 0;
        pSelf->m_sInBufList.nWritePos = 0;

        pthread_mutex_unlock(&pSelf->m_inBufMutex);

        pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                        OMX_EventCmdComplete, OMX_CommandFlush,
                                        0x0, NULL);
    }

    if (cmddata == 0x1 || (OMX_S32)cmddata == -1)
    {
        // Return all output buffers and send cmdcomplete
        pthread_mutex_lock(&pSelf->m_outBufMutex);

        while (pSelf->m_sOutBufList.nSizeOfList > 0)
        {
            pSelf->m_sOutBufList.nSizeOfList--;
            pSelf->m_Callbacks.FillBufferDone(&pSelf->mOmxCmp,
                       pSelf->m_pAppData,
                       pSelf->m_sOutBufList.pBufHdrList[pSelf->m_sOutBufList.nReadPos++]);

            if (pSelf->m_sOutBufList.nReadPos >= pSelf->m_sOutBufList.nBufArrSize)
                pSelf->m_sOutBufList.nReadPos = 0;
        }

        pSelf->m_sOutBufList.nReadPos  = 0;
        pSelf->m_sOutBufList.nWritePos = 0;

        pthread_mutex_unlock(&pSelf->m_outBufMutex);

        pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                        OMX_EventCmdComplete, OMX_CommandFlush,
                                        0x1, NULL);
    }
}

static inline void controlFillBuf(omx_vdec* pSelf, OMX_U32 cmddata)
{
    //*Fill buffer
    pthread_mutex_lock(&pSelf->m_outBufMutex);
    if (pSelf->m_sOutBufList.nSizeOfList < pSelf->m_sOutBufList.nAllocSize)
    {
        pSelf->m_sOutBufList.nSizeOfList++;
        pSelf->m_sOutBufList.pBufHdrList[pSelf->m_sOutBufList.nWritePos++]
            = ((OMX_BUFFERHEADERTYPE*) cmddata);

        if (pSelf->m_sOutBufList.nWritePos >= (OMX_S32)pSelf->m_sOutBufList.nAllocSize)
            pSelf->m_sOutBufList.nWritePos = 0;
    }
    pthread_mutex_unlock(&pSelf->m_outBufMutex);
}

static inline void controlEmptyBuf(omx_vdec* pSelf, OMX_U32 cmddata)
{
    OMX_BUFFERHEADERTYPE* pTmpInBufHeader = (OMX_BUFFERHEADERTYPE*) cmddata;
    OMX_TICKS   nInterval;

    //*Empty buffer
    pthread_mutex_lock(&pSelf->m_inBufMutex);
    if(0 == pTmpInBufHeader->nTimeStamp)
    {
        logi("why cur vbs input nTimeStamp=0? prevPts=%lld", pSelf->m_nInportPrevTimeStamp);
        if(pSelf->m_nInportPrevTimeStamp > 0)
        {
            logi("fix cur vbs input nTimeStamp from 0 to -1");
            pTmpInBufHeader->nTimeStamp = -1;
        }
    }
    else
    {
        if(pSelf->m_nInportPrevTimeStamp)
        {
            //*compare pts to decide if jump
            nInterval = pTmpInBufHeader->nTimeStamp - pSelf->m_nInportPrevTimeStamp;
            if(nInterval < -AWOMX_PTS_JUMP_THRESH || nInterval > AWOMX_PTS_JUMP_THRESH)
            {
                logd("pts jump [%lld]us, prev[%lld],cur[%lld],need reset vdeclib",
                     nInterval, pSelf->m_nInportPrevTimeStamp, pTmpInBufHeader->nTimeStamp);
                pSelf->m_JumpFlag = OMX_TRUE;
            }

            pSelf->m_nInportPrevTimeStamp = pTmpInBufHeader->nTimeStamp;
        }
        else //*first InBuf data
        {
            pSelf->m_nInportPrevTimeStamp = pTmpInBufHeader->nTimeStamp;
        }
    }

    if (pSelf->m_sInBufList.nSizeOfList < pSelf->m_sInBufList.nAllocSize)
    {
        pSelf->m_sInBufList.nSizeOfList++;
        pSelf->m_sInBufList.pBufHdrList[pSelf->m_sInBufList.nWritePos++]
            = ((OMX_BUFFERHEADERTYPE*) cmddata);

        if (pSelf->m_sInBufList.nWritePos >= (OMX_S32)pSelf->m_sInBufList.nAllocSize)
            pSelf->m_sInBufList.nWritePos = 0;
    }

    logi("(omx_vdec, f:%s, l:%d) nTimeStamp[%lld], nAllocLen[%d], nFilledLen[%d],\
          nOffset[%d], nFlags[0x%x], nOutputPortIndex[%d], nInputPortIndex[%d]",
          __FUNCTION__, __LINE__,
          pTmpInBufHeader->nTimeStamp,
          (int)pTmpInBufHeader->nAllocLen,
          (int)pTmpInBufHeader->nFilledLen,
          (int)pTmpInBufHeader->nOffset,
          (int)pTmpInBufHeader->nFlags,
          (int)pTmpInBufHeader->nOutputPortIndex,
          (int)pTmpInBufHeader->nInputPortIndex);

    pthread_mutex_unlock(&pSelf->m_inBufMutex);

    //*Mark current buffer if there is outstanding command
    if (pSelf->pMarkBuf)
    {
        ((OMX_BUFFERHEADERTYPE *)(cmddata))->hMarkTargetComponent = &pSelf->mOmxCmp;
        ((OMX_BUFFERHEADERTYPE *)(cmddata))->pMarkData = pSelf->pMarkBuf->pMarkData;
        pSelf->pMarkBuf = NULL;
    }
}

static inline int vdecoderPrepare(omx_vdec* pSelf)
{
    logd("(f:%s, l:%d)(VDRV_THREAD_CMD_PREPARE_VDECODER)", __FUNCTION__, __LINE__);

    // If the parameter states a transition to the same state
    // raise a same state transition error.
    if (pSelf->m_state != OMX_StateExecuting)
    {
        logw("fatal error! when vdrv VDRV_THREAD_CMD_PREPARE_VDECODER, m_state[0x%x] \
              should be OMX_StateExecuting", pSelf->m_state);
    }

    //*if mdecoder had closed before, we should create it
    if(pSelf->m_decoder==NULL)
    {
        //AddVDPlugin();
        pSelf->m_decoder = CreateVideoDecoder();
    }

    //* set video stream info.
    VConfig m_videoConfig;
    memset(&m_videoConfig,0,sizeof(VConfig));

    //*set yv12
    m_videoConfig.eOutputPixelFormat = PIXEL_FORMAT_YV12;

    m_videoConfig.memops = MemAdapterGetOpsS();
    if(pSelf->m_useAndroidBuffer)
    {
        m_videoConfig.nAlignStride = pSelf->mGpuAlignStride;
    }
#if (SCALEDOWN_WHEN_RESOLUTION_MOER_THAN_1080P)
    // omx decoder make out buffer no more than 1920x1080
    if (pSelf->m_streamInfo.nWidth > 1920
        && pSelf->m_streamInfo.nHeight > 1088)
    {
        m_videoConfig.bScaleDownEn = 1;
        m_videoConfig.nHorizonScaleDownRatio = 1;
        m_videoConfig.nVerticalScaleDownRatio = 1;
    }
#endif
    if(pSelf->m_streamInfo.eCodecFormat == VIDEO_CODEC_FORMAT_WMV3
        || pSelf->m_streamInfo.eCodecFormat == VIDEO_CODEC_FORMAT_H264)
    {
        logd("*** pSelf->m_streamInfo.bIsFramePackage to 1 when it is vc1");
        pSelf->m_streamInfo.bIsFramePackage = 1;
    }

    m_videoConfig.nDeInterlaceHoldingFrameBufferNum = 0;//*not use deinterlace
    m_videoConfig.nDisplayHoldingFrameBufferNum     = 0;//*gpu and decoder not share buffer
    m_videoConfig.nRotateHoldingFrameBufferNum      = NUM_OF_PICTURES_KEEPPED_BY_ROTATE;
    m_videoConfig.nDecodeSmoothFrameBufferNum       = NUM_OF_PICTURES_FOR_EXTRA_SMOOTH;
    if(InitializeVideoDecoder(pSelf->m_decoder, &(pSelf->m_streamInfo),&m_videoConfig) != 0)
    {
        DestroyVideoDecoder(pSelf->m_decoder);
        pSelf->m_decoder           = NULL;
        pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                        OMX_EventError, OMX_ErrorHardware,
                                        0 , NULL);
        loge("Idle transition failed, set_vstream_info() return fail.\n");
        return OMX_RESULT_ERROR;
    }
    return OMX_RESULT_OK;
}

static inline void vdecoderDecodeStream(omx_vdec* pSelf)
{
    int decodeResult;
    CdcMessage msg;
    memset(&msg, 0, sizeof(CdcMessage));
#if (OPEN_STATISTICS)
    nTimeUs1 = OMX_GetNowUs();
#endif

    decodeResult = DecodeVideoStream(pSelf->m_decoder,0,0,0,0);
    logi("***decodeResult = %d",decodeResult);

#if (OPEN_STATISTICS)
    nTimeUs2 = OMX_GetNowUs();

    if(decodeResult == CEDARV_RESULT_FRAME_DECODED
       || decodeResult == CEDARV_RESULT_KEYFRAME_DECODED)
    {
        pSelf->mDecodeFrameTotalDuration += (nTimeUs2-nTimeUs1);
        pSelf->mDecodeFrameTotalCount++;
    }
    else if(decodeResult == CEDARV_RESULT_OK)
    {
        pSelf->mDecodeOKTotalDuration += (nTimeUs2-nTimeUs1);
        pSelf->mDecodeOKTotalCount++;
    }
    else if(decodeResult == CEDARV_RESULT_NO_FRAME_BUFFER)
    {
        pSelf->mDecodeNoFrameTotalDuration += (nTimeUs2-nTimeUs1);
        pSelf->mDecodeNoFrameTotalCount++;
    }
    else if(decodeResult == CEDARV_RESULT_NO_BITSTREAM)
    {
        pSelf->mDecodeNoBitstreamTotalDuration += (nTimeUs2-nTimeUs1);
        pSelf->mDecodeNoBitstreamTotalCount++;
    }
    else
    {
        pSelf->mDecodeOtherTotalDuration += (nTimeUs2-nTimeUs1);
        pSelf->mDecodeOtherTotalCount++;
    }
#endif

    if(decodeResult == VDECODE_RESULT_KEYFRAME_DECODED ||
       decodeResult == VDECODE_RESULT_FRAME_DECODED ||
       decodeResult == VDECODE_RESULT_OK)
    {
        //*do nothing
    }
    else if(decodeResult == VDECODE_RESULT_NO_FRAME_BUFFER)
    {
        logi("(f:%s, l:%d) no_frame_buffer, wait!", __FUNCTION__, __LINE__);
        SemTimedWait(&pSelf->m_sem_frame_output, 20);
        logi("(f:%s, l:%d) no_frame_buffer, wait_done!", __FUNCTION__, __LINE__);
    }
    else if(decodeResult == VDECODE_RESULT_NO_BITSTREAM)
    {
        if(pSelf->nInputEosFlag)
        {
            pSelf->nInputEosFlag = OMX_FALSE;

            //*set eos to decoder ,decoder should flush all fbm
            int mRet = 0;
            int mDecodeCount = 0;
            while(mRet != VDECODE_RESULT_NO_BITSTREAM)
            {
                mRet = DecodeVideoStream(pSelf->m_decoder,1,0,0,0);
                usleep(5*1000);
                mDecodeCount++;
                if(mDecodeCount > 1000)
                {
                    logw(" decoder timeOut when set eos to decoder!");
                    break;
                }
            }

            //*set eof flag, MediaCodec use this flag
            // to determine whether a playback is finished.

            logi("(f:%s, l:%d) when eos, vdrvtask meet no_bitstream, \
                  all frames have decoded, notify ComponentThread eos!",
                  __FUNCTION__, __LINE__);
            memset(&msg, 0, sizeof(CdcMessage));
            msg.messageId = MAIN_THREAD_CMD_VDRV_NOTIFY_EOS;

            CdcMessageQueuePostMessage(pSelf->mqMainThread, &msg);
            logi("(f:%s, l:%d) no_vbsinput_buffer, eos wait!", __FUNCTION__, __LINE__);
            SemTimedWait(&pSelf->m_sem_vbs_input, 20);
            logi("(f:%s, l:%d) no_vbsinput_buffer, eos wait_done!", __FUNCTION__, __LINE__);
        }
        else
        {
            logi("(f:%s, l:%d) no_vbsinput_buffer, wait!", __FUNCTION__, __LINE__);
            SemTimedWait(&pSelf->m_sem_vbs_input, 20);
            logi("(f:%s, l:%d) no_vbsinput_buffer, wait_done!", __FUNCTION__, __LINE__);
        }
    }
    else if(decodeResult == VDECODE_RESULT_RESOLUTION_CHANGE)
    {
        logd("set nResolutionChangeVdrvThreadFlag to 1, decode detect resolution change");
        pSelf->nResolutionChangeVdrvThreadFlag = OMX_TRUE;
        memset(&msg, 0, sizeof(CdcMessage));
        msg.messageId = MAIN_THREAD_CMD_VDRV_RESOLUTION_CHANGE;
        msg.params[0] = 1;
        CdcMessageQueuePostMessage(pSelf->mqMainThread, &msg);
    }
    else if(decodeResult < 0)
    {
        logw("decode fatal error[%d]", decodeResult);
        //* report a fatal error.
        pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                        OMX_EventError, OMX_ErrorHardware,
                                        0, NULL);
    }
    else
    {
        logd("decode ret[%d], ignore? why?", decodeResult);
    }
}

static inline void decoderResolutionChange(omx_vdec* pSelf)
{
    logv("***ReopenVideoEngine!");
    VConfig m_videoConfig;
    memset(&m_videoConfig,0,sizeof(VConfig));

    m_videoConfig.memops = MemAdapterGetOpsS();
    //*set yv12
    m_videoConfig.eOutputPixelFormat = PIXEL_FORMAT_YV12;

    m_videoConfig.nDeInterlaceHoldingFrameBufferNum = 0;//*not use deinterlace
    m_videoConfig.nDisplayHoldingFrameBufferNum     = 0;//*gpu and decoder not share buffer
    m_videoConfig.nRotateHoldingFrameBufferNum      = NUM_OF_PICTURES_KEEPPED_BY_ROTATE;
    m_videoConfig.nDecodeSmoothFrameBufferNum       = NUM_OF_PICTURES_FOR_EXTRA_SMOOTH;

    if(pSelf->m_useAndroidBuffer)
    {
        m_videoConfig.nAlignStride = pSelf->mGpuAlignStride;
    }
#if (SCALEDOWN_WHEN_RESOLUTION_MOER_THAN_1080P)
    // omx decoder make out buffer no more than 1920x1080
    if (pSelf->m_streamInfo.nWidth > 1920
        && pSelf->m_streamInfo.nHeight > 1088)
    {
        m_videoConfig.bScaleDownEn = 1;
        m_videoConfig.nHorizonScaleDownRatio = 1;
        m_videoConfig.nVerticalScaleDownRatio = 1;
    }
#endif
    if(pSelf->m_streamInfo.pCodecSpecificData != NULL)
    {
        free(pSelf->m_streamInfo.pCodecSpecificData);
        pSelf->m_streamInfo.pCodecSpecificData  = NULL;
        pSelf->m_streamInfo.nCodecSpecificDataLen = 0;
    }
    ReopenVideoEngine(pSelf->m_decoder, &m_videoConfig, &(pSelf->m_streamInfo));
    pSelf->mResolutionChangeFlag = OMX_FALSE;
    pSelf->nVdrvResolutionChangeFlag = OMX_FALSE;
    return ;
}


static inline int detectFirstStreamData(omx_vdec* pSelf)
{
    OMX_BUFFERHEADERTYPE*   pInBufHdr   = NULL;
    pInBufHdr = pSelf->m_sInBufList.pBufHdrList[pSelf->m_sInBufList.nReadPos];

    if(pInBufHdr == NULL)
    {
        logd("(f:%s, l:%d) fatal error! pInBufHdr is NULL, check code!",
             __FUNCTION__, __LINE__);
        return OMX_RESULT_ERROR;
    }

    pSelf->mFirstInputDataFlag = OMX_FALSE;
    logv("*** the first pInBufHdr->nFlags = 0x%x",(int)pInBufHdr->nFlags);
    if (pInBufHdr->nFlags & OMX_BUFFERFLAG_CODECCONFIG)
    {
        if(pSelf->m_streamInfo.pCodecSpecificData)
            free(pSelf->m_streamInfo.pCodecSpecificData);
        pSelf->m_streamInfo.nCodecSpecificDataLen = pInBufHdr->nFilledLen;
        pSelf->m_streamInfo.pCodecSpecificData
            = (char*)malloc(pSelf->m_streamInfo.nCodecSpecificDataLen);
        memset(pSelf->m_streamInfo.pCodecSpecificData,0,
               pSelf->m_streamInfo.nCodecSpecificDataLen);
        memcpy(pSelf->m_streamInfo.pCodecSpecificData,
               pInBufHdr->pBuffer,pSelf->m_streamInfo.nCodecSpecificDataLen);

        CdcMessage msg;
        memset(&msg, 0, sizeof(CdcMessage));
        msg.messageId = VDRV_THREAD_CMD_PREPARE_VDECODER;

        CdcMessageQueuePostMessage(pSelf->mqVdrvThread, &msg);
        sem_post(&pSelf->m_sem_vbs_input);
        sem_post(&pSelf->m_sem_frame_output);
        logd("(f:%s, l:%d) wait for VDRV_THREAD_CMD_PREPARE_VDECODER",
              __FUNCTION__, __LINE__);
        SemTimedWait(&pSelf->m_vdrv_cmd_lock, -1);
        logd("(f:%s, l:%d) wait for VDRV_THREAD_CMD_PREPARE_VDECODER done!",
              __FUNCTION__, __LINE__);

        pSelf->m_sInBufList.nSizeOfList--;
        pSelf->m_sInBufList.nReadPos++;
        if (pSelf->m_sInBufList.nReadPos >= (OMX_S32)pSelf->m_sInBufList.nAllocSize)
            pSelf->m_sInBufList.nReadPos = 0;

        pSelf->m_Callbacks.EmptyBufferDone(&pSelf->mOmxCmp, pSelf->m_pAppData, pInBufHdr);
    }
    else
    {
        if(!pSelf->mIsCyber)
        {
            CdcMessage msg;
            memset(&msg, 0, sizeof(CdcMessage));
            msg.messageId = VDRV_THREAD_CMD_PREPARE_VDECODER;

            CdcMessageQueuePostMessage(pSelf->mqVdrvThread, &msg);
            sem_post(&pSelf->m_sem_vbs_input);
            sem_post(&pSelf->m_sem_frame_output);
            logd("(f:%s, l:%d) wait for VDRV_THREAD_CMD_PREPARE_VDECODER",
                 __FUNCTION__, __LINE__);
            SemTimedWait(&pSelf->m_vdrv_cmd_lock, -1);
            logd("(f:%s, l:%d) wait for VDRV_THREAD_CMD_PREPARE_VDECODER done!",
                 __FUNCTION__, __LINE__);
        }
    }
    return OMX_RESULT_OK;
}

static inline int checkResolutionChange(omx_vdec* pSelf, VideoPicture* picture)
{
    //*here, we callback buffer_widht and buffer_height to ACodec

    int width_align  = 0;
    int height_align = 0;
    OMX_CONFIG_RECTTYPE mCurVideoRect;
    memset(&mCurVideoRect, 0, sizeof(OMX_CONFIG_RECTTYPE));

    if(pSelf->mIsFirstGetOffset)
    {
        pSelf->mVideoRect.nLeft   = picture->nLeftOffset;
        pSelf->mVideoRect.nTop    = picture->nTopOffset;
        pSelf->mVideoRect.nWidth  = picture->nRightOffset - picture->nLeftOffset;
        pSelf->mVideoRect.nHeight = picture->nBottomOffset - picture->nTopOffset;
        pSelf->mIsFirstGetOffset = false;
    }

    mCurVideoRect.nLeft   = picture->nLeftOffset;
    mCurVideoRect.nTop    = picture->nTopOffset;
    mCurVideoRect.nWidth  = picture->nRightOffset - picture->nLeftOffset;
    mCurVideoRect.nHeight = picture->nBottomOffset - picture->nTopOffset;

    if(pSelf->mCropEnable)
    {
        width_align  = picture->nWidth;
        height_align = picture->nHeight;
    }
    else
    {
        width_align  = picture->nRightOffset - picture->nLeftOffset;
        height_align = picture->nBottomOffset - picture->nTopOffset;
        if(width_align <= 0 || height_align <= 0)
        {
            width_align  = picture->nWidth;
            height_align = picture->nHeight;
        }
    }

    //* make the height to 2 align
    height_align = (height_align + 1) & ~1;

    if((OMX_U32)width_align != pSelf->m_sOutPortDefType.format.video.nFrameWidth
        || (OMX_U32)height_align  != pSelf->m_sOutPortDefType.format.video.nFrameHeight
        || (mCurVideoRect.nLeft   != pSelf->mVideoRect.nLeft)
        || (mCurVideoRect.nTop    != pSelf->mVideoRect.nTop)
        || (mCurVideoRect.nWidth  != pSelf->mVideoRect.nWidth)
        || (mCurVideoRect.nHeight != pSelf->mVideoRect.nHeight))
    {
        logw(" port setting changed -- old info : widht = %d, height = %d, \
               mVideoRect: %d, %d, %d, %d ",
                (int)pSelf->m_sOutPortDefType.format.video.nFrameWidth,
                (int)pSelf->m_sOutPortDefType.format.video.nFrameHeight,
                (int)pSelf->mVideoRect.nTop,(int)pSelf->mVideoRect.nLeft,
                (int)pSelf->mVideoRect.nWidth,(int)pSelf->mVideoRect.nHeight);
        logw(" port setting changed -- new info : widht = %d, height = %d, \
               mVideoRect: %d, %d, %d, %d ",
                (int)width_align, (int)height_align,
                (int)mCurVideoRect.nTop,(int)mCurVideoRect.nLeft,
                (int)mCurVideoRect.nWidth,(int)mCurVideoRect.nHeight);

        memcpy(&pSelf->mVideoRect, &mCurVideoRect, sizeof(OMX_CONFIG_RECTTYPE));
        pSelf->m_sOutPortDefType.format.video.nFrameHeight = height_align;
        pSelf->m_sOutPortDefType.format.video.nFrameWidth = width_align;
		pSelf->m_sOutPortDefType.format.video.nStride = width_align;
		pSelf->m_sOutPortDefType.format.video.nSliceHeight = height_align;
        pSelf->m_sOutPortDefType.nBufferSize = height_align*width_align *3/2;
        pSelf->port_setting_match = OMX_FALSE;
        pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                        OMX_EventPortSettingsChanged, 0x01, 0, NULL);
        return OMX_RESOLUTION_CHANGE;
    }
    return OMX_RESULT_OK;
}

static inline void copyPictureDataToAndroidBuffer(omx_vdec* pSelf,
                                                  VideoPicture* picture,
                                                  OMX_BUFFERHEADERTYPE* pOutBufHdr)
{
    int64_t         nTimeUs1 = 0;
    int64_t         nTimeUs2 = 0;
    OutPutBufferInfo        mOutPutBufferInfo;
    memset(&mOutPutBufferInfo, 0 ,sizeof(OutPutBufferInfo));

#ifdef __ANDROID__

#if (OPEN_STATISTICS)
    nTimeUs1 = OMX_GetNowUs();
#endif

    void* dst;
    buffer_handle_t         pBufferHandle = NULL;

    android::GraphicBufferMapper &mapper = android::GraphicBufferMapper::get();

    //*when lock gui buffer, we must according gui buffer's width and height, not by decoder!
    android::Rect bounds(pSelf->m_sOutPortDefType.format.video.nFrameWidth,
                         pSelf->m_sOutPortDefType.format.video.nFrameHeight);

#ifdef CONF_KITKAT_AND_NEWER
        if(pSelf->m_storeOutputMetaDataFlag ==OMX_TRUE)
        {
            VideoDecoderOutputMetaData *pMetaData
                = (VideoDecoderOutputMetaData*)pOutBufHdr->pBuffer;
            pBufferHandle = pMetaData->pHandle;
        }
        else
            pBufferHandle = (buffer_handle_t)pOutBufHdr->pBuffer;
#else
        pBufferHandle = (buffer_handle_t)pOutBufHdr->pBuffer;
#endif

    if(0 != mapper.lock(pBufferHandle, GRALLOC_USAGE_SW_WRITE_OFTEN, bounds, &dst))
    {
        logw("Lock GUIBuf fail!");
    }

    //TransformMB32ToYV12(&picture, dst);

    if(pSelf->mCropEnable)
    {
        memcpy(dst,picture->pData0,picture->nWidth*picture->nHeight*3/2);
    }
    else
    {
        mOutPutBufferInfo.pAddr   = dst;
        mOutPutBufferInfo.nWidth  = pSelf->m_sOutPortDefType.format.video.nFrameWidth;
        mOutPutBufferInfo.nHeight = pSelf->m_sOutPortDefType.format.video.nFrameHeight;
        if(pSelf->mVp9orH265SoftDecodeFlag==OMX_TRUE)
            TransformYV12ToYV12Soft(picture, &mOutPutBufferInfo);
        else
            TransformYV12ToYV12Hw(picture, &mOutPutBufferInfo);
    }

    if(0 != mapper.unlock(pBufferHandle))
    {
        logw("Unlock GUIBuf fail!");
    }

#if (OPEN_STATISTICS)
    nTimeUs2 = OMX_GetNowUs();
    pSelf->mConvertTotalDuration += (nTimeUs2-nTimeUs1);
    pSelf->mConvertTotalCount++;
#endif
#endif
    return ;
}

static inline void sendStreamDataToDecoder(omx_vdec* pSelf)
{
    char* pBuf0;
    char* pBuf1;
    int   size0;
    int   size1;
    int   require_size;
    unsigned char*   pData;
    VideoStreamDataInfo DataInfo;
    OMX_BUFFERHEADERTYPE*   pInBufHdr   = NULL;
    int nSemVal;
    int nRetSemGetValue;
    int ret = -1;

    memset(&DataInfo, 0, sizeof(VideoStreamDataInfo));
    pInBufHdr = pSelf->m_sInBufList.pBufHdrList[pSelf->m_sInBufList.nReadPos];

    if(pInBufHdr == NULL)
    {
        logd("(f:%s, l:%d) fatal error! pInBufHdr is NULL, check code!",
             __FUNCTION__, __LINE__);
        return ;
    }

    //*add stream to decoder.
    require_size = pInBufHdr->nFilledLen;
    ret = RequestVideoStreamBuffer(pSelf->m_decoder, require_size,
                                   &pBuf0, &size0, &pBuf1, &size1,0);
    if(ret != 0)
    {
        logi("(f:%s, l:%d)req vbs fail! maybe vbs buffer is full! require_size[%d]",
             __FUNCTION__, __LINE__, require_size);
        return ;
    }

    DataInfo.nLength      = require_size;
    DataInfo.bIsFirstPart = 1;
    DataInfo.bIsLastPart  = 1;
    DataInfo.pData        = pBuf0;
    if(pInBufHdr->nTimeStamp >= 0)
    {
        DataInfo.nPts   = pInBufHdr->nTimeStamp;
        DataInfo.bValid = 1;
    }
    else
    {
        DataInfo.nPts   = -1;
        DataInfo.bValid = 0;
    }

    if(require_size <= size0)
    {
        pData = pInBufHdr->pBuffer + pInBufHdr->nOffset;
        memcpy(pBuf0, pData, require_size);
    }
    else
    {
        pData = pInBufHdr->pBuffer + pInBufHdr->nOffset;
        memcpy(pBuf0, pData, size0);
        pData += size0;
        memcpy(pBuf1, pData, require_size - size0);
    }
    SubmitVideoStreamData(pSelf->m_decoder, &DataInfo,0);

    //*Check for EOS flag
    if (pInBufHdr)
    {
        if (pInBufHdr->nFlags & OMX_BUFFERFLAG_EOS)
        {
            //*Copy flag to output buffer header
            pSelf->nInBufEos = OMX_TRUE;
            usleep(5*1000);
            CdcMessage msg;
            memset(&msg, 0, sizeof(CdcMessage));
            msg.messageId = VDRV_THREAD_CMD_EndOfStream;

            CdcMessageQueuePostMessage(pSelf->mqVdrvThread, &msg);
            sem_post(&pSelf->m_sem_vbs_input);
            sem_post(&pSelf->m_sem_frame_output);
            logd("(f:%s) wait for VDRV_THREAD_CMD_EndOfStream", __FUNCTION__);
            SemTimedWait(&pSelf->m_vdrv_cmd_lock, -1);
            logd("(f:%s) wait for VDRV_THREAD_CMD_EndOfStream done!", __FUNCTION__);
            logd(" set up nInBufEos flag.");
             //*Trigger event handler
            pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                            OMX_EventBufferFlag, 0x1,
                                            pInBufHdr->nFlags, NULL);
             //*Clear flag
            pInBufHdr->nFlags = 0;
        }
        //*Check for mark buffers
        if (pInBufHdr->pMarkData)
        {
            if (pSelf->pMarkData == NULL && pSelf->hMarkTargetComponent == NULL)
            {
                pSelf->pMarkData = pInBufHdr->pMarkData;
                pSelf->hMarkTargetComponent = pInBufHdr->hMarkTargetComponent;
            }
        }
        //*Trigger event handler
        if (pInBufHdr->hMarkTargetComponent == &pSelf->mOmxCmp && pInBufHdr->pMarkData)
        {
            pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                            OMX_EventMark, 0, 0, pInBufHdr->pMarkData);
        }
    }

    pSelf->m_Callbacks.EmptyBufferDone(&pSelf->mOmxCmp, pSelf->m_pAppData, pInBufHdr);
    //*notify vdrvThread vbs input
    tryPostSem(&pSelf->m_sem_vbs_input);

    //* check if there is a input bit stream.
    pthread_mutex_lock(&pSelf->m_inBufMutex);

    pSelf->m_sInBufList.nSizeOfList--;
    pSelf->m_sInBufList.nReadPos++;
    if (pSelf->m_sInBufList.nReadPos >= (OMX_S32)pSelf->m_sInBufList.nAllocSize)
        pSelf->m_sInBufList.nReadPos = 0;

    /*for cts, using for synchronization between inputbuffer and outputbuffer*/
    pSelf->m_InputNum ++;
    if ((pSelf->mIsFromCts == true)/* && ((pSelf->m_InputNum - pSelf->m_OutputNum) > 3)*/)
    {
        usleep(10000);
    }
    pthread_mutex_unlock(&pSelf->m_inBufMutex);

}

static inline void drainPicture(omx_vdec* pSelf)
{
    OMX_BUFFERHEADERTYPE*   pOutBufHdr  = NULL;
    VideoPicture*           picture = NULL;
    OutPutBufferInfo        mOutPutBufferInfo;
    int nSemVal;
    int nRetSemGetValue;

    memset(&mOutPutBufferInfo, 0 ,sizeof(OutPutBufferInfo));
    //* check if the picture size changed.
    picture = NextPictureInfo(pSelf->m_decoder,0);

    if(checkResolutionChange(pSelf, picture) == OMX_RESOLUTION_CHANGE)
    {
        return ;
    }

    pthread_mutex_lock(&pSelf->m_outBufMutex);
    if(pSelf->m_sOutBufList.nSizeOfList > 0)
    {
        pSelf->m_sOutBufList.nSizeOfList--;
        pOutBufHdr = pSelf->m_sOutBufList.pBufHdrList[pSelf->m_sOutBufList.nReadPos++];
        if (pSelf->m_sOutBufList.nReadPos >= (OMX_S32)pSelf->m_sOutBufList.nAllocSize)
            pSelf->m_sOutBufList.nReadPos = 0;
    }
    else
        pOutBufHdr = NULL;
    pthread_mutex_unlock(&pSelf->m_outBufMutex);

    //* if no output buffer, wait for some time.
    if(pOutBufHdr == NULL)
    {
        usleep(1000);
        return ;
    }

    picture = RequestPicture(pSelf->m_decoder, 0);

    //* should flush cache(ve buffer) befor copy ve buffer to ouput buffer
    CdcMemFlushCache(pSelf->memops, (void*)picture->pData0,
                     picture->nLineStride * picture->nHeight*3/2);
    //*transform picture color format.
    logv("(omx_vdec, f:%s, l:%d) m_useAndroidBuffer = %d",
          __FUNCTION__, __LINE__,(int)pSelf->m_useAndroidBuffer);
    if(pSelf->m_useAndroidBuffer == OMX_FALSE)
    {
        mOutPutBufferInfo.pAddr   = pOutBufHdr->pBuffer;
        mOutPutBufferInfo.nWidth  = pSelf->m_sOutPortDefType.format.video.nFrameWidth;
        mOutPutBufferInfo.nHeight = pSelf->m_sOutPortDefType.format.video.nFrameHeight;
        if(pSelf->mVp9orH265SoftDecodeFlag==OMX_TRUE)
            TransformYV12ToYUV420Soft(picture, &mOutPutBufferInfo);
        else
            TransformYV12ToYUV420(picture, &mOutPutBufferInfo);    // YUV420 planar
    }
    else
    {
        copyPictureDataToAndroidBuffer(pSelf, picture, pOutBufHdr);
    }

    pOutBufHdr->nTimeStamp = picture->nPts;
    pOutBufHdr->nOffset    = 0;

#ifdef CONF_KITKAT_AND_NEWER
    if(pSelf->m_storeOutputMetaDataFlag==OMX_TRUE)
        pOutBufHdr->nFilledLen = sizeof(VideoDecoderOutputMetaData);
    else
        pOutBufHdr->nFilledLen = (pSelf->m_sOutPortDefType.format.video.nFrameWidth *
                                  pSelf->m_sOutPortDefType.format.video.nFrameHeight)*3/2;
#else
    pOutBufHdr->nFilledLen = (pSelf->m_sOutPortDefType.format.video.nFrameWidth *
                              pSelf->m_sOutPortDefType.format.video.nFrameHeight)*3/2;
#endif
    ReturnPicture(pSelf->m_decoder, picture);

    //*notify vdrvThread free frame
    tryPostSem(&pSelf->m_sem_frame_output);

    //*Check for mark buffers
    if (pSelf->pMarkData != NULL && pSelf->hMarkTargetComponent != NULL)
    {
        if(!ValidPictureNum(pSelf->m_decoder,0))
        {
            pOutBufHdr->pMarkData = pSelf->pMarkData;
            pOutBufHdr->hMarkTargetComponent = pSelf->hMarkTargetComponent;
            pSelf->pMarkData = NULL;
            pSelf->hMarkTargetComponent = NULL;
        }
    }

    logi("****FillBufferDone is called, nSizeOfList[%d], pts[%lld],\
          nAllocLen[%d], nFilledLen[%d], nOffset[%d], nFlags[0x%x], \
          nOutputPortIndex[%d], nInputPortIndex[%d]",
        (int)pSelf->m_sOutBufList.nSizeOfList,pOutBufHdr->nTimeStamp,
        (int)pOutBufHdr->nAllocLen,(int)pOutBufHdr->nFilledLen,
        (int)pOutBufHdr->nOffset,(int)pOutBufHdr->nFlags,
        (int)pOutBufHdr->nOutputPortIndex,(int)pOutBufHdr->nInputPortIndex);

    pSelf->m_OutputNum ++;

    pSelf->m_Callbacks.FillBufferDone(&pSelf->mOmxCmp, pSelf->m_pAppData, pOutBufHdr);
    pOutBufHdr = NULL;
    return ;
}

static inline void processOutputEosFlag(omx_vdec* pSelf)
{
    OMX_BUFFERHEADERTYPE*   pOutBufHdr  = NULL;
    //*set eos flag, MediaCodec use this flag
    // to determine whether a playback is finished.
    pthread_mutex_lock(&pSelf->m_outBufMutex);
    while (pSelf->m_sOutBufList.nSizeOfList > 0)
    {
        pSelf->m_sOutBufList.nSizeOfList--;
        pOutBufHdr = pSelf->m_sOutBufList.pBufHdrList[pSelf->m_sOutBufList.nReadPos++];
        if (pSelf->m_sOutBufList.nReadPos >= (OMX_S32)pSelf->m_sOutBufList.nAllocSize)
        {
            pSelf->m_sOutBufList.nReadPos = 0;
        }

        if(pOutBufHdr==NULL)
            continue;

        if (pOutBufHdr->nFilledLen != 0)
        {
            pSelf->m_Callbacks.FillBufferDone(&pSelf->mOmxCmp, pSelf->m_pAppData, pOutBufHdr);
            pOutBufHdr = NULL;
        }
        else
        {
            pOutBufHdr->nFlags |= OMX_BUFFERFLAG_EOS;
            pSelf->m_Callbacks.FillBufferDone(&pSelf->mOmxCmp, pSelf->m_pAppData, pOutBufHdr);
            pOutBufHdr = NULL;
            pSelf->nInBufEos = OMX_FALSE;
            pSelf->nVdrvNotifyEosFlag = OMX_FALSE;
            break;
        }
    }
    pthread_mutex_unlock(&pSelf->m_outBufMutex);
    return ;
}

static inline int processThreadCommand(omx_vdec* pSelf, CdcMessage* pMsg)
{
    int cmd = pMsg->messageId;
    uintptr_t  cmddata = pMsg->params[0];
    logv("processThreadCommand cmd = %d", cmd);

    //*State transition command
    if (cmd == MAIN_THREAD_CMD_SET_STATE)
    {
        logd(" set state command, cmd = %d, cmddata = %d.", (int)cmd, (int)cmddata);
        //*If the parameter states a transition to the same state
        // raise a same state transition error.
        if (pSelf->m_state == (OMX_STATETYPE)(cmddata))
        {
            pSelf->m_Callbacks.EventHandler(&pSelf->mOmxCmp, pSelf->m_pAppData,
                                            OMX_EventError, OMX_ErrorSameState, 0 , NULL);
        }
        else
        {
            controlSetState(pSelf, (OMX_U32)cmddata);
        }
    }
    else if (cmd == MAIN_THREAD_CMD_STOP_PORT)
    {
        controlStopPort(pSelf, (OMX_U32)cmddata);
    }
    else if (cmd == MAIN_THREAD_CMD_RESTART_PORT)
    {
        controlRestartPort(pSelf, (OMX_U32)cmddata);
    }
    else if (cmd == MAIN_THREAD_CMD_FLUSH)
    {
        controlFlush(pSelf, (OMX_U32)cmddata);
    }
    else if (cmd == MAIN_THREAD_CMD_STOP)
    {
        logd(" stop command.");
        CdcMessage msg;
        memset(&msg, 0, sizeof(CdcMessage));
        msg.messageId = VDRV_THREAD_CMD_STOP_VDECODER;

        CdcMessageQueuePostMessage(pSelf->mqVdrvThread, &msg);
        sem_post(&pSelf->m_sem_vbs_input);
        sem_post(&pSelf->m_sem_frame_output);
        logd("(f:%s, l:%d) wait for VDRV_THREAD_CMD_STOP_VDECODER", __FUNCTION__, __LINE__);
        SemTimedWait(&pSelf->m_vdrv_cmd_lock, -1);
        logd("(f:%s, l:%d) wait for VDRV_THREAD_CMD_STOP_VDECODER done!", __FUNCTION__, __LINE__);
          //*Kill thread
         return OMX_RESULT_EXIT;
    }
    else if (cmd == MAIN_THREAD_CMD_FILL_BUF)
    {
        controlFillBuf(pSelf, (OMX_U32)cmddata);
    }
    else if (cmd == MAIN_THREAD_CMD_EMPTY_BUF)
    {
        controlEmptyBuf(pSelf, (OMX_U32)cmddata);
    }
    else if (cmd == MAIN_THREAD_CMD_MARK_BUF)
    {
        if (!pSelf->pMarkBuf)
            pSelf->pMarkBuf = (OMX_MARKTYPE *)(cmddata);
    }
    else if(cmd == MAIN_THREAD_CMD_VDRV_NOTIFY_EOS)
    {
        pSelf->nVdrvNotifyEosFlag = OMX_TRUE;
    }
    else if(cmd == MAIN_THREAD_CMD_VDRV_RESOLUTION_CHANGE)
    {
        //pSelf->mResolutionChangeFlag = (OMX_BOOL)cmddata;
        pSelf->nResolutionChangeNativeThreadFlag = (OMX_BOOL)cmddata;
        logd(" set nResolutionChangeNativeThreadFlag to %d",
             (int)pSelf->nResolutionChangeNativeThreadFlag);
    }
    else
    {
        logw("(f:%s, l:%d) ", __FUNCTION__, __LINE__);
    }
    return OMX_RESULT_OK;
}

static inline int processVdrvThreadCommand(omx_vdec* pSelf, CdcMessage* pMsg)
{
    int cmd = pMsg->messageId;
    logd("(f:%s, l:%d) vdrvThread receive cmd[0x%x]", __FUNCTION__, __LINE__, cmd);
    // State transition command
    //now omx_vdec's m_state = OMX_StateLoaded, OMX_StateLoaded->OMX_StateIdle
    if (cmd == VDRV_THREAD_CMD_PREPARE_VDECODER)
    {
        vdecoderPrepare(pSelf);
    }
    // now omx_vdec's m_state = OMX_StateLoaded, OMX_StateIdle->OMX_StateLoaded
    else if (cmd == VDRV_THREAD_CMD_CLOSE_VDECODER)
    {
        logd("(f:%s, l:%d)(VDRV_THREAD_CMD_CLOSE_VDECODER)", __FUNCTION__, __LINE__);
        //* stop and close decoder.
        if(pSelf->m_decoder != NULL)
        {
            DestroyVideoDecoder(pSelf->m_decoder);
            pSelf->m_decoder           = NULL;
            pSelf->mFirstInputDataFlag = OMX_TRUE;
        }
    }
    else if (cmd == VDRV_THREAD_CMD_FLUSH_VDECODER)
    {
        logd("(f:%s, l:%d)(VDRV_THREAD_CMD_FLUSH_VDECODER)", __FUNCTION__, __LINE__);
        if(pSelf->m_decoder)
        {
            ResetVideoDecoder(pSelf->m_decoder);
        }
        else
        {
            logw(" fatal error, m_decoder is not malloc when flush all ports!");
        }
    }
    else if (cmd == VDRV_THREAD_CMD_STOP_VDECODER)
    {
        logd("(f:%s, l:%d)(VDRV_THREAD_CMD_STOP_VDECODER)", __FUNCTION__, __LINE__);
        return OMX_RESULT_EXIT;
    }
    else if (cmd == VDRV_THREAD_CMD_EndOfStream)
    {
        logd("(f:%s, l:%d)(VDRV_THREAD_CMD_EndOfStream)", __FUNCTION__, __LINE__);
        pSelf->nInputEosFlag = OMX_TRUE;
    }
    else
    {
        logw("(f:%s, l:%d)fatal error! unknown OMX_VDRV_COMMANDTYPE[0x%x]",
              __FUNCTION__, __LINE__, cmd);
    }
    return OMX_RESULT_OK;
}

static void* RenderThread(void* pThreadData)
{
    int                     decodeResult;

    // Variables related to decoder buffer handling
    OMX_BUFFERHEADERTYPE*   pInBufHdr   = NULL;
    OMX_BUFFERHEADERTYPE*   pOutBufHdr  = NULL;
    OMX_U8*                 pInBuf      = NULL;
    OMX_U32                 nInBufSize;

    int nSemVal;
    int nRetSemGetValue;
    CdcMessage msg;
    memset(&msg, 0, sizeof(CdcMessage));

    // Recover the pointer to my component specific data
    omx_vdec* pSelf = static_cast<omx_vdec*>(pThreadData);

    char calling_process[256];
    int validNumber;

    while (1)
    {
        if(pSelf->exitRender)
            goto EXIT;

        //*Buffer processing
        // Only happens when the component is in executing state.
        if (pSelf->m_state == OMX_StateExecuting  &&
            pSelf->m_sInPortDefType.bEnabled          &&
            pSelf->m_sOutPortDefType.bEnabled         &&
            pSelf->port_setting_match                    &&
            (pSelf->mResolutionChangeFlag == OMX_FALSE))
        {
            if(pSelf->m_decoder == NULL)
                continue ;
            //*check if there is a picture to output.
            if((validNumber = ValidPictureNum(pSelf->m_decoder,0)) != 0)
            {
                drainPicture(pSelf);
                continue;
            }
            else
            {
                logi("LINE %d, nInBufEos %d, nVdrvNotifyEosFlag %d",
                     __LINE__,  pSelf->nInBufEos, pSelf->nVdrvNotifyEosFlag);

                if (pSelf->nVdrvNotifyEosFlag)
                {
                    processOutputEosFlag(pSelf);
                }

                if(pSelf->nResolutionChangeNativeThreadFlag == OMX_TRUE)
                {
                    logd("set pSelf->mResolutionChangeFlag to 1");
                    pSelf->mResolutionChangeFlag = OMX_TRUE;
                    pSelf->nResolutionChangeNativeThreadFlag = OMX_FALSE;
                }
            }
        }
        else
        {
            usleep(2000);
        }
    }

EXIT:
    loge("exit render!\n");
    return (void*)OMX_ErrorNone;
}


/*
 *  Component Thread
 *    The ComponentThread function is exeuted in a separate pThread and
 *    is used to implement the actual component functions.
 */
 /*****************************************************************************/
static void* ComponentThread(void* pThreadData)
{
    int                     decodeResult;

    // Variables related to decoder buffer handling
    OMX_BUFFERHEADERTYPE*   pInBufHdr   = NULL;
    OMX_BUFFERHEADERTYPE*   pOutBufHdr  = NULL;
    OMX_U8*                 pInBuf      = NULL;
    OMX_U32                 nInBufSize;

    int nSemVal;
    int nRetSemGetValue;
    CdcMessage msg;
    memset(&msg, 0, sizeof(CdcMessage));

    // Recover the pointer to my component specific data
    omx_vdec* pSelf = static_cast<omx_vdec*>(pThreadData);

    char calling_process[256];
    int validNumber;

    while (1)
    {
        if(CdcMessageQueueTryGetMessage(pSelf->mqMainThread, &msg, 5) == 0)
        {
            if(processThreadCommand(pSelf, &msg) == OMX_RESULT_EXIT)
            {
                pSelf->exitRender = 1;
                goto EXIT;
            }
        }

        //*Buffer processing
        // Only happens when the component is in executing state.
        if (pSelf->m_state == OMX_StateExecuting  &&
            pSelf->m_sInPortDefType.bEnabled          &&
            pSelf->m_sOutPortDefType.bEnabled         &&
            pSelf->port_setting_match                    &&
            (pSelf->mResolutionChangeFlag == OMX_FALSE))
        {
            if(OMX_TRUE == pSelf->m_JumpFlag)
            {
                logd("reset vdeclib for jump!");
                //pSelf->m_decoder->ioctrl(pSelf->m_decoder, CEDARV_COMMAND_JUMP, 0);
                pSelf->m_JumpFlag = OMX_FALSE;
            }

            //*if the first-input-data is configure-data, we should cpy to decoder
            // as init-data. here we send commad to VdrvThread to create decoder
            if(pSelf->mFirstInputDataFlag==OMX_TRUE && pSelf->m_sInBufList.nSizeOfList>0)
            {
                if(detectFirstStreamData(pSelf) == OMX_RESULT_ERROR)
                    goto EXIT;
            }

            if(pSelf->m_decoder == NULL)
                continue ;
            //*fill buffer first
            //while(0==ValidPictureNum(pSelf->m_decoder,0) && pSelf->m_sInBufList.nSizeOfList>0)
            while(ValidPictureNum(pSelf->m_decoder,0) < 2 && pSelf->m_sInBufList.nSizeOfList>0)
            {
                sendStreamDataToDecoder(pSelf);
            }
        }
    }

EXIT:
    return (void*)OMX_ErrorNone;
}

static void* ComponentVdrvThread(void* pThreadData)
{
    int nSemVal;
    int nRetSemGetValue;
    int nStopFlag = 0;
    CdcMessage msg;
    memset(&msg, 0, sizeof(CdcMessage));

    // Recover the pointer to my component specific data
    omx_vdec* pSelf = static_cast<omx_vdec*>(pThreadData);

    while (1)
    {
        if(CdcMessageQueueTryGetMessage(pSelf->mqVdrvThread, &msg, 0) == 0)
        {
process_vdrv_cmd:
            if(processVdrvThreadCommand(pSelf, &msg) == OMX_RESULT_EXIT)
                nStopFlag = 1;

            tryPostSem(&pSelf->m_vdrv_cmd_lock);
        }

        if(nStopFlag)
        {
            logd("vdrvThread detect nStopFlag[%d], exit!", nStopFlag);
            goto EXIT;
        }

        //*Buffer processing
        // Only happens when the component is in executing state.
        if ((pSelf->m_state == OMX_StateExecuting || pSelf->m_state == OMX_StatePause)
            &&(pSelf->m_decoder!=NULL)
            &&(pSelf->nResolutionChangeVdrvThreadFlag == OMX_FALSE))
        {
            vdecoderDecodeStream(pSelf);
        }
        else
        {
            if(pSelf->mResolutionChangeFlag == OMX_TRUE)
            {
                decoderResolutionChange(pSelf);
                pSelf->nResolutionChangeVdrvThreadFlag   = OMX_FALSE;
            }
            if(CdcMessageQueueTryGetMessage(pSelf->mqVdrvThread, &msg, 10) == 0)
                goto process_vdrv_cmd;
        }
    }

EXIT:
    logd("***notify: exit the componentVdrvThread!");
    return (void*)OMX_ErrorNone;
}
