/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : omx_core_cmp.h
* Description :
* History :
*   Author  : AL3
*   Date    : 2013/05/05
*   Comment : complete the design code
*/

/*============================================================================
                            O p e n M A X   w r a p p e r s
                             O p e n  M A X   C o r e

 OpenMAX Core Macros interface.

============================================================================*/

//////////////////////////////////////////////////////////////////////////////
//                             Include Files
//////////////////////////////////////////////////////////////////////////////
#ifndef OMX_CORE_CMP_H
#define OMX_CORE_CMP_H

#include "OMX_Core.h"

//#define printf ALOGV
#if 0
#define DEBUG_PRINT_ERROR printf
#define DEBUG_PRINT       printf
#define DEBUG_DETAIL      printf

#else
#define DEBUG_PRINT_ERROR(...) ((void)0)
#define DEBUG_PRINT(...)       ((void)0)
#define DEBUG_DETAIL(...)      ((void)0)

#endif

#ifdef __cplusplus
extern "C" {
#endif

void* aw_omx_create_component_wrapper(OMX_PTR obj_ptr);

OMX_ERRORTYPE aw_omx_component_init(OMX_IN OMX_HANDLETYPE pHComp,
                                    OMX_IN OMX_STRING pComponentName);

OMX_ERRORTYPE aw_omx_component_get_version(OMX_IN OMX_HANDLETYPE               pHComp,
                                           OMX_OUT OMX_STRING          pComponentName,
                                           OMX_OUT OMX_VERSIONTYPE* pComponentVersion,
                                           OMX_OUT OMX_VERSIONTYPE*      pSpecVersion,
                                           OMX_OUT OMX_UUIDTYPE*       pComponentUUID);

OMX_ERRORTYPE aw_omx_component_send_command(OMX_IN OMX_HANDLETYPE pHComp,
                                            OMX_IN OMX_COMMANDTYPE  eCmd,
                                            OMX_IN OMX_U32       uParam1,
                                            OMX_IN OMX_PTR      pCmdData);

OMX_ERRORTYPE aw_omx_component_get_parameter(OMX_IN OMX_HANDLETYPE     pHComp,
                                             OMX_IN OMX_INDEXTYPE eParamIndex,
                                             OMX_INOUT OMX_PTR     pParamData);

OMX_ERRORTYPE aw_omx_component_set_parameter(OMX_IN OMX_HANDLETYPE     pHComp,
                                             OMX_IN OMX_INDEXTYPE eParamIndex,
                                             OMX_IN OMX_PTR        pParamData);

OMX_ERRORTYPE aw_omx_component_get_config(OMX_IN OMX_HANDLETYPE      pHComp,
                                          OMX_IN OMX_INDEXTYPE eConfigIndex,
                                          OMX_INOUT OMX_PTR     pConfigData);

OMX_ERRORTYPE aw_omx_component_set_config(OMX_IN OMX_HANDLETYPE      pHComp,
                                          OMX_IN OMX_INDEXTYPE eConfigIndex,
                                          OMX_IN OMX_PTR        pConfigData);

OMX_ERRORTYPE aw_omx_component_get_extension_index(OMX_IN OMX_HANDLETYPE      pHComp,
                                                   OMX_IN OMX_STRING      pParamName,
                                                   OMX_OUT OMX_INDEXTYPE* pIndexType);

OMX_ERRORTYPE aw_omx_component_get_state(OMX_IN OMX_HANDLETYPE  pHComp,
                                         OMX_OUT OMX_STATETYPE* pState);

OMX_ERRORTYPE aw_omx_component_tunnel_request(OMX_IN OMX_HANDLETYPE                pHComp,
                                              OMX_IN OMX_U32                        uPort,
                                              OMX_IN OMX_HANDLETYPE        pPeerComponent,
                                              OMX_IN OMX_U32                    uPeerPort,
                                              OMX_INOUT OMX_TUNNELSETUPTYPE* pTunnelSetup);

OMX_ERRORTYPE aw_omx_component_use_buffer(OMX_IN OMX_HANDLETYPE                pHComp,
                                          OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                                          OMX_IN OMX_U32                        uPort,
                                          OMX_IN OMX_PTR                     pAppData,
                                          OMX_IN OMX_U32                       uBytes,
                                          OMX_IN OMX_U8*                      pBuffer);

// aw_omx_component_allocate_buffer  -- API Call
OMX_ERRORTYPE aw_omx_component_allocate_buffer(OMX_IN OMX_HANDLETYPE                pHComp,
                                               OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                                               OMX_IN OMX_U32                        uPort,
                                               OMX_IN OMX_PTR                     pAppData,
                                               OMX_IN OMX_U32                       uBytes);

OMX_ERRORTYPE aw_omx_component_free_buffer(OMX_IN OMX_HANDLETYPE         pHComp,
                                           OMX_IN OMX_U32                 uPort,
                                           OMX_IN OMX_BUFFERHEADERTYPE* pBuffer);

OMX_ERRORTYPE aw_omx_component_empty_this_buffer(OMX_IN OMX_HANDLETYPE         pHComp,
                                                 OMX_IN OMX_BUFFERHEADERTYPE* pBuffer);

OMX_ERRORTYPE aw_omx_component_fill_this_buffer(OMX_IN OMX_HANDLETYPE         pHComp,
                                                OMX_IN OMX_BUFFERHEADERTYPE* pBuffer);

OMX_ERRORTYPE aw_omx_component_set_callbacks(OMX_IN OMX_HANDLETYPE        pHComp,
                                             OMX_IN OMX_CALLBACKTYPE* pCallbacks,
                                             OMX_IN OMX_PTR             pAppData);

OMX_ERRORTYPE aw_omx_component_deinit(OMX_IN OMX_HANDLETYPE pHComp);

OMX_ERRORTYPE aw_omx_component_use_EGL_image(OMX_IN OMX_HANDLETYPE                pHComp,
                                             OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                                             OMX_IN OMX_U32                        uPort,
                                             OMX_IN OMX_PTR                     pAppData,
                                             OMX_IN void*                      pEglImage);

OMX_ERRORTYPE aw_omx_component_role_enum(OMX_IN OMX_HANDLETYPE pHComp,
                                         OMX_OUT OMX_U8*        pRole,
                                         OMX_IN OMX_U32        uIndex);

#ifdef __cplusplus
}
#endif

#endif
