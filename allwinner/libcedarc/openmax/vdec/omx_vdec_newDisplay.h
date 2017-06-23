/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : omx_vdec_newDisplay.h
* Description :
* History :
*   Author  : AL3
*   Date    : 2013/05/05
*   Comment : complete the design code
*/

#ifndef __OMX_VDEC_H__
#define __OMX_VDEC_H__
/*============================================================================
                            O p e n M A X   Component
                                Video Decoder

*//** @file omx_vdec.h
  This module contains the class definition for openMAX decoder component.

*//*========================================================================*/

//////////////////////////////////////////////////////////////////////////////
//                             Include Files
//////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>

#include <pthread.h>
#include <semaphore.h>

#include "OMX_Core.h"
#include "aw_omx_component.h"

#include "vdecoder.h"
#include "CdcMessageQueue.h"

#include <hardware/hal_public.h>
#include <linux/ion.h>
#include <ion/ion.h>

extern "C"
{
    OMX_API void* get_omx_component_factory_fn(void);
}

//////////////////////////////////////////////////////////////////////////////
//                       Module specific globals
//////////////////////////////////////////////////////////////////////////////
#define OMX_SPEC_VERSION  0x00000101

/*
 *  D E C L A R A T I O N S
 */
#define OMX_NOPORT                      0xFFFFFFFE
#define NUM_IN_BUFFERS                  2              // Input Buffers
#define NUM_IN_BUFFERS_SECURE           1              // Input Buffers(secure)
#define NUM_OUT_BUFFERS                 2              // Output Buffers
#define MAX_NUM_OUT_BUFFERS          15                // max Output Buffers
#define OMX_TIMEOUT                     10            // Timeout value in milisecond
// Count of Maximum number of times the component can time out
#define OMX_MAX_TIMEOUTS                160
#define OMX_VIDEO_DEC_INPUT_BUFFER_SIZE (2*1024*1024)     // 2 MB
#define OMX_VIDEO_DEC_INPUT_BUFFER_SIZE_SECURE (256*1024) // 256 kB

#define AWOMX_PTS_JUMP_THRESH             (2000000)   //pts jump threshold to judge, unit:us

#define OMX_MAX_VIDEO_BUFFER_NUM 32
#define CODEC_SPECIFIC_DATA_LENGTH 20*1024
/*
 *     D E F I N I T I O N S
 */

typedef struct _BufferList BufferList;

/*
 * The main structure for buffer management.
 *
 *   pBufHdr     - An array of pointers to buffer headers.
 *                 The size of the array is set dynamically using the nBufferCountActual value
 *                   send by the client.
 *   nListEnd    - Marker to the boundary of the array. This points to the last index of the
 *                   pBufHdr array.
 *   nSizeOfList - Count of valid data in the list.
 *   nAllocSize  - Size of the allocated list. This is equal to (nListEnd + 1) in most of
 *                   the times. When the list is freed this is decremented and at that
 *                   time the value is not equal to (nListEnd + 1). This is because
 *                   the list is not freed from the end and hence we cannot decrement
 *                   nListEnd each time we free an element in the list. When nAllocSize is zero,
 *                   the list is completely freed and the other paramaters of the list are
 *                   initialized.
 *                 If the client crashes before freeing up the buffers, this parameter is
 *                   checked (for nonzero value) to see if there are still elements on the list.
 *                   If yes, then the remaining elements are freed.
 *    nWritePos  - The position where the next buffer would be written. The value is incremented
 *                   after the write. It is wrapped around when it is greater than nListEnd.
 *    nReadPos   - The position from where the next buffer would be read. The value is incremented
 *                   after the read. It is wrapped around when it is greater than nListEnd.
 *    eDir       - Type of BufferList.
 *                            OMX_DirInput  =  Input  Buffer List
 *                           OMX_DirOutput  =  Output Buffer List
 */

struct _BufferList
{
   OMX_BUFFERHEADERTYPE**   pBufHdrList;
   OMX_U32                  nSizeOfList;
   OMX_S32                  nWritePos;
   OMX_S32                  nReadPos;

   OMX_BUFFERHEADERTYPE*    pBufArr;
   OMX_S32                  nAllocBySelfFlags;
   OMX_S32                  nBufArrSize;
   OMX_U32                  nAllocSize;
   OMX_DIRTYPE              eDir;
};

typedef struct VIDDEC_CUSTOM_PARAM
{
    unsigned char cCustomParamName[128];
    OMX_INDEXTYPE nCustomParamIndex;
} VIDDEC_CUSTOM_PARAM;

typedef struct VIDEO_PROFILE_LEVEL
{
    OMX_S32  nProfile;
    OMX_S32  nLevel;
} VIDEO_PROFILE_LEVEL_TYPE;

typedef enum OMX_VDRV_FEEDBACK_MSGTYPE
{
    //vdeclib has drained all input data.
    OMX_VdrvFeedbackMsg_NotifyEos        = OMX_CommandVendorStartUnused,
    OMX_VdrvFeedbackMsg_ResolutionChange,
    OMX_VdrvFeedbackMsg_Max              = 0X7FFFFFFF,
} OMX_VDRV_FEEDBACK_MSGTYPE;

//* Out Buffer can be owned by twos at the same time
enum OutBufferStatus {
    OWNED_BY_NONE     = 0,
    OWNED_BY_RENDER   = 1,  //*means owned by ACodec.cpp or OMXCodec
    OWNED_BY_DECODER  = 2,  //*means owned by decoder.cpp
};

//* describe the output buffer (malloc by gpu), share by gpu and decoder
typedef struct OMXOutputBufferInfoS
{
    ANativeWindowBuffer*  pWindowBuf;

#ifdef CONF_KERNEL_VERSION_3_10
                    ion_user_handle_t handle_ion;
#else
                    struct ion_handle *handle_ion;
#endif


    OMX_BUFFERHEADERTYPE* pHeadBufInfo;
    char*                 pBufPhyAddr;
    char*                 pBufVirAddr;
    VideoPicture*           pVideoPicture;
    OutBufferStatus       mStatus;
    OMX_BOOL              mUseDecoderBufferToSetEosFlag;
}OMXOutputBufferInfoT;

// OMX video decoder class
class omx_vdec: public aw_omx_component
{
public:
    omx_vdec();           // constructor
    virtual ~omx_vdec();  // destructor

    OMX_ERRORTYPE allocate_buffer(OMX_HANDLETYPE         pHComp,
                                  OMX_BUFFERHEADERTYPE** ppBufferHdr,
                                  OMX_U32                uPort,
                                  OMX_PTR                pAppData,
                                  OMX_U32                uBytes
                                  );


    OMX_ERRORTYPE component_deinit(OMX_HANDLETYPE pHComp);

    OMX_ERRORTYPE component_init(OMX_STRING pName);

    OMX_ERRORTYPE component_role_enum(OMX_HANDLETYPE pHComp,
                                      OMX_U8*        pRole,
                                      OMX_U32        uIndex
                                      );

    OMX_ERRORTYPE component_tunnel_request(OMX_HANDLETYPE       pHComp,
                                           OMX_U32              uPort,
                                           OMX_HANDLETYPE       pPeerComponent,
                                           OMX_U32              uPeerPort,
                                           OMX_TUNNELSETUPTYPE* pTunnelSetup
                                           );

    OMX_ERRORTYPE empty_this_buffer(OMX_HANDLETYPE        pHComp,
                                    OMX_BUFFERHEADERTYPE* pBuffer
                                    );

    OMX_ERRORTYPE fill_this_buffer(OMX_HANDLETYPE        pHComp,
                                   OMX_BUFFERHEADERTYPE* pBuffer
                                   );

    OMX_ERRORTYPE free_buffer(OMX_HANDLETYPE        pHComp,
                              OMX_U32               uPort,
                              OMX_BUFFERHEADERTYPE* pBuffer
                              );

    OMX_ERRORTYPE get_component_version(OMX_HANDLETYPE   pHComp,
                                        OMX_STRING       pComponentName,
                                        OMX_VERSIONTYPE* pComponentVersion,
                                        OMX_VERSIONTYPE* pSpecVersion,
                                        OMX_UUIDTYPE*    pComponentUUID
                                        );

    OMX_ERRORTYPE get_config(OMX_HANDLETYPE pHComp,
                             OMX_INDEXTYPE  eConfigIndex,
                             OMX_PTR        pConfigData
                             );

    OMX_ERRORTYPE get_extension_index(OMX_HANDLETYPE pHComp,
                                      OMX_STRING     pParamName,
                                      OMX_INDEXTYPE* pIndexType
                                      );

    OMX_ERRORTYPE get_parameter(OMX_HANDLETYPE pHComp,
                                OMX_INDEXTYPE  pParamIndex,
                                OMX_PTR        pParamData);

    OMX_ERRORTYPE get_state(OMX_HANDLETYPE pHComp,
                            OMX_STATETYPE *pState);


    OMX_ERRORTYPE send_command(OMX_HANDLETYPE  pHComp,
                               OMX_COMMANDTYPE eCmd,
                               OMX_U32         uParam1,
                               OMX_PTR         pCmdData);


    OMX_ERRORTYPE set_callbacks(OMX_HANDLETYPE    pHComp,
                                OMX_CALLBACKTYPE* pCallbacks,
                                OMX_PTR           pAppData);

    OMX_ERRORTYPE set_config(OMX_HANDLETYPE pHComp,
                             OMX_INDEXTYPE  eConfigIndex,
                             OMX_PTR        pConfigData);

    OMX_ERRORTYPE set_parameter(OMX_HANDLETYPE pHComp,
                                OMX_INDEXTYPE  eCaramIndex,
                                OMX_PTR        pParamData);

    OMX_ERRORTYPE use_buffer(OMX_HANDLETYPE         pHComp,
                             OMX_BUFFERHEADERTYPE** ppBufferHdr,
                             OMX_U32                uPort,
                             OMX_PTR                pAppData,
                             OMX_U32                uBytes,
                             OMX_U8*                pBuffer);


    OMX_ERRORTYPE use_EGL_image(OMX_HANDLETYPE         pHComp,
                                OMX_BUFFERHEADERTYPE** ppBufferHdr,
                                OMX_U32                uPort,
                                OMX_PTR                pAppData,
                                void *                 pEglImage);
public:
    OMX_STATETYPE                   m_state;
    OMX_U8                           m_cRole[OMX_MAX_STRINGNAME_SIZE];
    OMX_U8                           m_cName[OMX_MAX_STRINGNAME_SIZE];
    OMX_VIDEO_CODINGTYPE             m_eCompressionFormat;
    OMX_CALLBACKTYPE                m_Callbacks;
    OMX_PTR                         m_pAppData;
    OMX_PORT_PARAM_TYPE             m_sPortParam;
    OMX_PARAM_PORTDEFINITIONTYPE    m_sInPortDefType;
    OMX_PARAM_PORTDEFINITIONTYPE    m_sOutPortDefType;
    OMX_VIDEO_PARAM_PORTFORMATTYPE  m_sInPortFormatType;
    OMX_VIDEO_PARAM_PORTFORMATTYPE  m_sOutPortFormatType;
    OMX_PRIORITYMGMTTYPE            m_sPriorityMgmtType;
    OMX_PARAM_BUFFERSUPPLIERTYPE    m_sInBufSupplierType;
    OMX_PARAM_BUFFERSUPPLIERTYPE    m_sOutBufSupplierType;
    pthread_t                       m_thread_id;
    pthread_t                       m_vdrv_thread_id;
    BufferList                      m_sInBufList;
    BufferList                      m_sOutBufList;
    pthread_mutex_t                    m_inBufMutex;
    pthread_mutex_t                 m_outBufMutex;

    pthread_mutex_t                 m_pipeMutex;
    sem_t                           m_vdrv_cmd_lock;       //for synchronise cmd.
    sem_t                           m_sem_vbs_input; //for notify vdrv_task vbs input.
    sem_t                           m_sem_frame_output; //for notify vdrv_task free frame.
    sem_t                           m_semExtraOutBufferNum;
    //* for cedarv decoder.
    VideoDecoder*                   m_decoder;
    VideoStreamInfo                 m_streamInfo;
    VConfig                         m_videoConfig;

    OMX_BOOL                        m_useAndroidBuffer;

    //* for detect pts jump
    OMX_TICKS                       m_nInportPrevTimeStamp;
    OMX_BOOL                        m_storeOutputMetaDataFlag;
    OMX_S32                         m_maxWidth;
    OMX_S32                         m_maxHeight;

    OMX_S32                            m_OutputNum;
    //for statistics
    int64_t     mDecodeFrameTotalDuration;
    int64_t     mDecodeOKTotalDuration;
    int64_t     mDecodeNoFrameTotalDuration;
    int64_t     mDecodeNoBitstreamTotalDuration;
    int64_t     mDecodeOtherTotalDuration;
    int64_t     mDecodeFrameTotalCount;
    int64_t     mDecodeOKTotalCount;
    int64_t     mDecodeNoFrameTotalCount;
    int64_t     mDecodeNoBitstreamTotalCount;
    int64_t     mDecodeOtherTotalCount;
    int64_t     mDecodeFrameSmallAverageDuration;
    int64_t     mDecodeFrameBigAverageDuration;
    int64_t     mDecodeNoFrameAverageDuration;
    int64_t     mDecodeNoBitstreamAverageDuration;
    int64_t     mConvertTotalDuration;
    int64_t     mConvertTotalCount;
    int64_t     mConvertAverageDuration;
    char        mCallingProcess[256];  //add by fuqiang for cts
    OMX_BOOL    mVp9orH265SoftDecodeFlag;
    OMX_BOOL    mResolutionChangeFlag;

    //* for new display
    OMX_MARKTYPE*          pMarkBuf;
    OMX_PTR                pMarkData;
    OMX_HANDLETYPE         hMarkTargetComponent;
    OMXOutputBufferInfoT   mOutputBufferInfo[OMX_MAX_VIDEO_BUFFER_NUM];
    OMX_S32                mIonFd;
    OMX_CONFIG_RECTTYPE    mVideoRect; //* for display crop
    OMX_S32                mPicNum;    //* just for debug
    OMX_S32                mGpuAlignStride;
    OMX_U8                 mCodecSpecificData[CODEC_SPECIFIC_DATA_LENGTH];
    OMX_U32                mCodecSpecificDataLen;

    OMX_BOOL               mIsSecureVideoFlag;
    OMX_BOOL               bPortSettingMatchFlag;
    OMX_BOOL               mIsFlushingFlag;
    OMX_BOOL               mIsSoftwareDecoderFlag;
    OMX_BOOL               mIs4KAlignFlag;
    OMX_BOOL               mHadInitDecoderFlag;
    OMX_BOOL               mInputEosFlag;
    OMX_BOOL               mFirstPictureFlag;
    OMX_BOOL               mHadGetVideoFbmBufInfoFlag;
    OMX_BOOL               mVdrvNotifyEosFlag;
    OMX_BOOL               nResolutionChangeVdrvThreadFlag;
    OMX_BOOL               nResolutionChangeNativeThreadFlag;

    OMX_U32                   mSetToDecoderBufferNum;
    OMX_U32                   mNeedReSetToDecoderBufferNum;

    OMX_U32     mExtraOutBufferNum;    //need by native window
    struct ScMemOpsS* omxMemOps;
    struct ScMemOpsS* decMemOps;
    OMX_BUFFERHEADERTYPE* mOutBufferKeepInOmx[OMX_MAX_VIDEO_BUFFER_NUM];
    OMX_S32 mOutBufferKeepInOmxNum;
    OMX_S32 mOutBufferNumDecoderNeed;

    CdcMessageQueue* mqMainThread;
    CdcMessageQueue* mqVdrvThread;

};

#endif // __OMX_VDEC_H__
