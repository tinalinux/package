/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : omx_core_cmp.cpp
* Description :
* History :
*   Author  : AL3
*   Date    : 2013/05/05
*   Comment : complete the design code
*/

/*============================================================================
                            O p e n M A X   w r a p p e r s
                             O p e n  M A X   C o r e

 This module contains the implementation of the OpenMAX core Macros which
 operate directly on the component.

*//*========================================================================*/

//////////////////////////////////////////////////////////////////////////////
//                             Include Files
//////////////////////////////////////////////////////////////////////////////

#include "aw_omx_common.h"
#include "omx_core_cmp.h"
#include "aw_omx_component.h"
#include <string.h>

#include "log.h"

void* aw_omx_create_component_wrapper(OMX_PTR pObj)
{
    aw_omx_component *pThis        = (aw_omx_component *)pObj;
    OMX_COMPONENTTYPE* pComponent   = &(pThis->mOmxCmp);
    memset(&pThis->mOmxCmp,0,sizeof(OMX_COMPONENTTYPE));

    pComponent->nSize               = sizeof(OMX_COMPONENTTYPE);
    pComponent->nVersion.nVersion   = OMX_SPEC_VERSION;
    pComponent->pApplicationPrivate = 0;
    pComponent->pComponentPrivate   = pObj;

    pComponent->AllocateBuffer      = &aw_omx_component_allocate_buffer;
    pComponent->FreeBuffer          = &aw_omx_component_free_buffer;
    pComponent->GetParameter        = &aw_omx_component_get_parameter;
    pComponent->SetParameter        = &aw_omx_component_set_parameter;
    pComponent->SendCommand         = &aw_omx_component_send_command;
    pComponent->FillThisBuffer      = &aw_omx_component_fill_this_buffer;
    pComponent->EmptyThisBuffer     = &aw_omx_component_empty_this_buffer;
    pComponent->GetState            = &aw_omx_component_get_state;
    pComponent->GetComponentVersion = &aw_omx_component_get_version;
    pComponent->GetConfig           = &aw_omx_component_get_config;
    pComponent->SetConfig           = &aw_omx_component_set_config;
    pComponent->GetExtensionIndex   = &aw_omx_component_get_extension_index;
    pComponent->ComponentTunnelRequest = &aw_omx_component_tunnel_request;
    pComponent->UseBuffer           = &aw_omx_component_use_buffer;
    pComponent->SetCallbacks        = &aw_omx_component_set_callbacks;
    pComponent->UseEGLImage         = &aw_omx_component_use_EGL_image;
    pComponent->ComponentRoleEnum   = &aw_omx_component_role_enum;
    pComponent->ComponentDeInit     = &aw_omx_component_deinit;
    return (void *)pComponent;
}

/************************************************************************/
/*               COMPONENT INTERFACE                                    */
/************************************************************************/

OMX_ERRORTYPE aw_omx_component_init(OMX_IN OMX_HANDLETYPE pHComp, OMX_IN OMX_STRING pComponentName)
{
    OMX_ERRORTYPE eReture = OMX_ErrorBadParameter;
    aw_omx_component *pThis
        = (pHComp)? (aw_omx_component *)(((OMX_COMPONENTTYPE *)pHComp)->pComponentPrivate):NULL;
    DEBUG_PRINT("OMXCORE: aw_omx_component_init %x\n",(unsigned)pHComp);

    if(pThis)
    {
        // call the init function
        eReture = pThis->component_init(pComponentName);

        if(eReture != OMX_ErrorNone)
        {
            //  in case of error, please destruct the component created
            delete pThis;
        }
    }

    return eReture;
}

OMX_ERRORTYPE aw_omx_component_get_version(OMX_IN OMX_HANDLETYPE               pHComp,
                                           OMX_OUT OMX_STRING          pComponentName,
                                           OMX_OUT OMX_VERSIONTYPE* pComponentVersion,
                                           OMX_OUT OMX_VERSIONTYPE*      pSpecVersion,
                                           OMX_OUT OMX_UUIDTYPE*       pComponentUUID)
{
    OMX_ERRORTYPE eReturn = OMX_ErrorBadParameter;
    aw_omx_component *pThis
        = (pHComp)? (aw_omx_component *)(((OMX_COMPONENTTYPE *)pHComp)->pComponentPrivate):NULL;
    DEBUG_PRINT("OMXCORE: aw_omx_component_get_version %x, %s , %x\n",
                 (unsigned)pHComp,pComponentName,(unsigned)pComponentVersion);
    if(pThis)
    {
        eReturn = pThis->get_component_version(pHComp,pComponentName,
                                               pComponentVersion,pSpecVersion,pComponentUUID);
    }

    return eReturn;
}

OMX_ERRORTYPE aw_omx_component_send_command(OMX_IN OMX_HANDLETYPE pHComp,
                                            OMX_IN OMX_COMMANDTYPE  eCmd,
                                            OMX_IN OMX_U32       uParam1,
                                            OMX_IN OMX_PTR      pCmdData)
{
    OMX_ERRORTYPE eReturn = OMX_ErrorBadParameter;
    aw_omx_component *pThis
        = (pHComp)? (aw_omx_component *)(((OMX_COMPONENTTYPE *)pHComp)->pComponentPrivate):NULL;
    DEBUG_PRINT("OMXCORE: aw_omx_component_send_command %x, %d , %d\n",
                 (unsigned)pHComp,(unsigned)eCmd,(unsigned)uParam1);

    if(pThis)
    {
        eReturn = pThis->send_command(pHComp,eCmd,uParam1,pCmdData);
    }

    return eReturn;
}

OMX_ERRORTYPE aw_omx_component_get_parameter(OMX_IN OMX_HANDLETYPE     pHComp,
                                             OMX_IN OMX_INDEXTYPE eParamIndex,
                                             OMX_INOUT OMX_PTR     pParamData)
{
    OMX_ERRORTYPE eReturn = OMX_ErrorBadParameter;
    aw_omx_component *pThis
        = (pHComp)? (aw_omx_component *)(((OMX_COMPONENTTYPE *)pHComp)->pComponentPrivate):NULL;
    DEBUG_PRINT("OMXCORE: aw_omx_component_get_parameter %x, %x , %d\n",
                (unsigned)pHComp,(unsigned)pParamData,eParamIndex);

    if(pThis)
    {
        eReturn = pThis->get_parameter(pHComp,eParamIndex,pParamData);
    }

    return eReturn;
}

OMX_ERRORTYPE aw_omx_component_set_parameter(OMX_IN OMX_HANDLETYPE     pHComp,
                                             OMX_IN OMX_INDEXTYPE eParamIndex,
                                             OMX_IN OMX_PTR        pParamData)
{
    OMX_ERRORTYPE eReturn = OMX_ErrorBadParameter;
    aw_omx_component *pThis
        = (pHComp)? (aw_omx_component *)(((OMX_COMPONENTTYPE *)pHComp)->pComponentPrivate):NULL;
    DEBUG_PRINT("OMXCORE: aw_omx_component_set_parameter %x, %x , %d\n",
        (unsigned)pHComp,(unsigned)pParamData,eParamIndex);

    if(pThis)
    {
        eReturn = pThis->set_parameter(pHComp,eParamIndex,pParamData);
    }
    return eReturn;
}

OMX_ERRORTYPE aw_omx_component_get_config(OMX_IN OMX_HANDLETYPE      pHComp,
                                          OMX_IN OMX_INDEXTYPE eConfigIndex,
                                          OMX_INOUT OMX_PTR     pConfigData)
{
    OMX_ERRORTYPE eReturn = OMX_ErrorBadParameter;
    aw_omx_component *pThis
        = (pHComp)? (aw_omx_component *)(((OMX_COMPONENTTYPE *)pHComp)->pComponentPrivate):NULL;
    DEBUG_PRINT("OMXCORE: aw_omx_component_get_config %x\n",(unsigned)pHComp);

    if(pThis)
    {
        eReturn = pThis->get_config(pHComp, eConfigIndex, pConfigData);
    }

    return eReturn;
}

OMX_ERRORTYPE aw_omx_component_set_config(OMX_IN OMX_HANDLETYPE      pHComp,
                                          OMX_IN OMX_INDEXTYPE eConfigIndex,
                                          OMX_IN OMX_PTR        pConfigData)
{
    OMX_ERRORTYPE eReturn = OMX_ErrorBadParameter;
    aw_omx_component *pThis
        = (pHComp)? (aw_omx_component *)(((OMX_COMPONENTTYPE *)pHComp)->pComponentPrivate):NULL;
    DEBUG_PRINT("OMXCORE: aw_omx_component_set_config %x\n",(unsigned)pHComp);

    if(pThis)
    {
        eReturn = pThis->set_config(pHComp, eConfigIndex, pConfigData);
    }

    return eReturn;
}

OMX_ERRORTYPE aw_omx_component_get_extension_index(OMX_IN OMX_HANDLETYPE      pHComp,
                                                   OMX_IN OMX_STRING      pParamName,
                                                   OMX_OUT OMX_INDEXTYPE* pIndexType)
{
    OMX_ERRORTYPE eReturn = OMX_ErrorBadParameter;
    aw_omx_component *pThis
        = (pHComp)? (aw_omx_component *)(((OMX_COMPONENTTYPE *)pHComp)->pComponentPrivate):NULL;
    if(pThis)
    {
        eReturn = pThis->get_extension_index(pHComp,pParamName,pIndexType);
    }

    return eReturn;
}

OMX_ERRORTYPE aw_omx_component_get_state(OMX_IN OMX_HANDLETYPE  pHComp,
                                         OMX_OUT OMX_STATETYPE* pState)
{
    OMX_ERRORTYPE eReturn = OMX_ErrorBadParameter;
    aw_omx_component *pThis
        = (pHComp)? (aw_omx_component *)(((OMX_COMPONENTTYPE *)pHComp)->pComponentPrivate):NULL;
    DEBUG_PRINT("OMXCORE: aw_omx_component_get_state %x\n",(unsigned)pHComp);

    if(pThis)
    {
        eReturn = pThis->get_state(pHComp,pState);
    }

    return eReturn;
}

OMX_ERRORTYPE aw_omx_component_tunnel_request(OMX_IN OMX_HANDLETYPE                pHComp,
                                              OMX_IN OMX_U32                        uPort,
                                              OMX_IN OMX_HANDLETYPE        pPeerComponent,
                                              OMX_IN OMX_U32                    uPeerPort,
                                              OMX_INOUT OMX_TUNNELSETUPTYPE* pTunnelSetup)
{
    DEBUG_PRINT("Error: aw_omx_component_tunnel_request Not Implemented\n");

    CEDARC_UNUSE(pHComp);
    CEDARC_UNUSE(uPort);
    CEDARC_UNUSE(pPeerComponent);
    CEDARC_UNUSE(uPeerPort);
    CEDARC_UNUSE(pTunnelSetup);

    return OMX_ErrorNotImplemented;
}

OMX_ERRORTYPE aw_omx_component_use_buffer(OMX_IN OMX_HANDLETYPE                pHComp,
                                          OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                                          OMX_IN OMX_U32                        uPort,
                                          OMX_IN OMX_PTR                     pAppData,
                                          OMX_IN OMX_U32                       uBytes,
                                          OMX_IN OMX_U8*                      pBuffer)
{
    OMX_ERRORTYPE eReturn = OMX_ErrorBadParameter;
    aw_omx_component *pThis
        = (pHComp)? (aw_omx_component *)(((OMX_COMPONENTTYPE *)pHComp)->pComponentPrivate):NULL;
    DEBUG_PRINT("OMXCORE: aw_omx_component_use_buffer %x\n",
                (unsigned)pHComp);

    if(pThis)
    {
        eReturn = pThis->use_buffer(pHComp, ppBufferHdr, uPort, pAppData, uBytes, pBuffer);
    }

    return eReturn;
}

// aw_omx_component_allocate_buffer  -- API Call
OMX_ERRORTYPE aw_omx_component_allocate_buffer(OMX_IN OMX_HANDLETYPE                pHComp,
                                               OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                                               OMX_IN OMX_U32                        uPort,
                                               OMX_IN OMX_PTR                     pAppData,
                                               OMX_IN OMX_U32                       uBytes)
{
    OMX_ERRORTYPE eReturn = OMX_ErrorBadParameter;
    aw_omx_component *pThis
        = (pHComp)? (aw_omx_component *)(((OMX_COMPONENTTYPE *)pHComp)->pComponentPrivate):NULL;
    DEBUG_PRINT("OMXCORE: aw_omx_component_allocate_buffer %x, %x , %d\n",
                (unsigned)pHComp,(unsigned)ppBufferHdr,(unsigned)uPort);

    if(pThis)
    {
        eReturn = pThis->allocate_buffer(pHComp,ppBufferHdr,uPort,pAppData,uBytes);
    }

    return eReturn;
}

OMX_ERRORTYPE aw_omx_component_free_buffer(OMX_IN OMX_HANDLETYPE         pHComp,
                                           OMX_IN OMX_U32                 uPort,
                                           OMX_IN OMX_BUFFERHEADERTYPE* pBuffer)
{
    OMX_ERRORTYPE eReturn = OMX_ErrorBadParameter;
    aw_omx_component *pThis
        = (pHComp)? (aw_omx_component *)(((OMX_COMPONENTTYPE *)pHComp)->pComponentPrivate):NULL;
    DEBUG_PRINT("OMXCORE: aw_omx_component_free_buffer[%d] %x, %x\n",
                (unsigned)uPort, (unsigned)pHComp, (unsigned)pBuffer);

    if(pThis)
    {
        eReturn = pThis->free_buffer(pHComp,uPort,pBuffer);
    }

    return eReturn;
}

OMX_ERRORTYPE aw_omx_component_empty_this_buffer(OMX_IN OMX_HANDLETYPE         pHComp,
                                                 OMX_IN OMX_BUFFERHEADERTYPE* pBuffer)
{
    OMX_ERRORTYPE eReturn = OMX_ErrorBadParameter;
    aw_omx_component *pThis
        = (pHComp)? (aw_omx_component *)(((OMX_COMPONENTTYPE *)pHComp)->pComponentPrivate):NULL;

    if(pThis)
    {
        eReturn = pThis->empty_this_buffer(pHComp,pBuffer);
    }

    return eReturn;
}

OMX_ERRORTYPE aw_omx_component_fill_this_buffer(OMX_IN OMX_HANDLETYPE         pHComp,
                                                OMX_IN OMX_BUFFERHEADERTYPE* pBuffer)
{
    OMX_ERRORTYPE eReturn = OMX_ErrorBadParameter;
    aw_omx_component *pThis
        = (pHComp)? (aw_omx_component *)(((OMX_COMPONENTTYPE *)pHComp)->pComponentPrivate):NULL;

    if(pThis)
    {
        eReturn = pThis->fill_this_buffer(pHComp,pBuffer);
    }

    return eReturn;
}

OMX_ERRORTYPE aw_omx_component_set_callbacks(OMX_IN OMX_HANDLETYPE        pHComp,
                                             OMX_IN OMX_CALLBACKTYPE* pCallbacks,
                                             OMX_IN OMX_PTR             pAppData)
{
    OMX_ERRORTYPE eReturn = OMX_ErrorBadParameter;
    aw_omx_component *pThis
        = (pHComp)? (aw_omx_component *)(((OMX_COMPONENTTYPE *)pHComp)->pComponentPrivate):NULL;
    DEBUG_PRINT("OMXCORE: aw_omx_component_set_callbacks %x, %x , %x\n",
                 (unsigned)pHComp,(unsigned)pCallbacks,(unsigned)pAppData);

    if(pThis)
    {
        eReturn = pThis->set_callbacks(pHComp,pCallbacks,pAppData);
    }

    return eReturn;
}

OMX_ERRORTYPE aw_omx_component_deinit(OMX_IN OMX_HANDLETYPE pHComp)
{
    OMX_ERRORTYPE eReturn = OMX_ErrorBadParameter;
    aw_omx_component *pThis
        = (pHComp)? (aw_omx_component *)(((OMX_COMPONENTTYPE *)pHComp)->pComponentPrivate):NULL;
    DEBUG_PRINT("OMXCORE: aw_omx_component_deinit %x\n",(unsigned)pHComp);

    if(pThis)
    {
        // call the deinit fuction first
        OMX_STATETYPE eState;
        pThis->get_state(pHComp,&eState);
        DEBUG_PRINT("Calling FreeHandle in eState %d \n", eState);
        eReturn = pThis->component_deinit(pHComp);
        // destroy the component.
        delete pThis;
        ((OMX_COMPONENTTYPE *)pHComp)->pComponentPrivate = NULL;
    }

    return eReturn;
}

OMX_ERRORTYPE aw_omx_component_use_EGL_image(OMX_IN OMX_HANDLETYPE                pHComp,
                                             OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                                             OMX_IN OMX_U32                        uPort,
                                             OMX_IN OMX_PTR                     pAppData,
                                             OMX_IN void*                      pEglImage)
{
    OMX_ERRORTYPE eReturn = OMX_ErrorBadParameter;
    aw_omx_component *pThis
        = (pHComp)? (aw_omx_component *)(((OMX_COMPONENTTYPE *)pHComp)->pComponentPrivate):NULL;
    DEBUG_PRINT("OMXCORE: aw_omx_component_use_EGL_image %x, %x , %d\n",
                (unsigned)pHComp,(unsigned)ppBufferHdr,(unsigned)uPort);
    if(pThis)
    {
        eReturn = pThis->use_EGL_image(pHComp,ppBufferHdr,uPort,pAppData,pEglImage);
    }

    return eReturn;
}

OMX_ERRORTYPE aw_omx_component_role_enum(OMX_IN OMX_HANDLETYPE pHComp,
                                         OMX_OUT OMX_U8*        pRole,
                                         OMX_IN OMX_U32        uIndex)
{
    OMX_ERRORTYPE eReturn = OMX_ErrorBadParameter;
    aw_omx_component *pThis
        = (pHComp)? (aw_omx_component *)(((OMX_COMPONENTTYPE *)pHComp)->pComponentPrivate):NULL;
    DEBUG_PRINT("OMXCORE: aw_omx_component_role_enum %x, %x , %d\n",
                (unsigned)pHComp,(unsigned)pRole,(unsigned)uIndex);

    if(pThis)
      {
        eReturn = pThis->component_role_enum(pHComp,pRole,uIndex);
      }

    return eReturn;
}

static void fixStaticCheckWarning()
{
    CEDARC_UNUSE(fixStaticCheckWarning);
    CEDARC_UNUSE(aw_omx_component_init);
    CEDARC_UNUSE(aw_omx_component_get_version);
    CEDARC_UNUSE(aw_omx_component_send_command);
    CEDARC_UNUSE(aw_omx_component_get_parameter);
    CEDARC_UNUSE(aw_omx_component_set_parameter);
    CEDARC_UNUSE(aw_omx_component_get_config);
    CEDARC_UNUSE(aw_omx_component_set_config);
    CEDARC_UNUSE(aw_omx_component_get_extension_index    );
    CEDARC_UNUSE(aw_omx_component_get_state);
    CEDARC_UNUSE(aw_omx_component_tunnel_request);
    CEDARC_UNUSE(aw_omx_component_use_buffer);
    CEDARC_UNUSE(aw_omx_component_allocate_buffer);
    CEDARC_UNUSE(aw_omx_component_free_buffer);
    CEDARC_UNUSE(aw_omx_component_empty_this_buffer);
    CEDARC_UNUSE(aw_omx_component_fill_this_buffer);
    CEDARC_UNUSE(aw_omx_component_set_callbacks);
    CEDARC_UNUSE(aw_omx_component_deinit);
    CEDARC_UNUSE(aw_omx_component_use_EGL_image);
    CEDARC_UNUSE(aw_omx_component_role_enum);

}
