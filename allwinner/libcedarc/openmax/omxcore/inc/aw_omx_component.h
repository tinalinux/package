/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : aw_omx_component.h
* Description :
* History :
*   Author  : AL3
*   Date    : 2013/05/05
*   Comment : complete the design code
*/

/*============================================================================
                            O p e n M A X   w r a p p e r s
                O p e n  M A X   C o m p o n e n t  I n t e r f a c e

*//** @file aw_omx_component.h
  This module contains the abstract interface for the OpenMAX components.

*//*========================================================================*/

#ifndef AW_OMX_COMPONENT_H
#define AW_OMX_COMPONENT_H
//////////////////////////////////////////////////////////////////////////////
//                             Include Files
//////////////////////////////////////////////////////////////////////////////
#include "OMX_Core.h"
#include "OMX_Component.h"

class aw_omx_component
{

public:
  /* single member to hold the vtable */
  OMX_COMPONENTTYPE mOmxCmp;

public:

  // this is critical, otherwise, sub class destructor will not be called
  virtual ~aw_omx_component(){}

  // Initialize the component after creation
  virtual OMX_ERRORTYPE component_init(OMX_IN OMX_STRING pComponentName)=0;

  /*******************************************************************/
  /*           Standard OpenMAX Methods                              */
  /*******************************************************************/

  // Query the component for its information
  virtual
  OMX_ERRORTYPE  get_component_version(OMX_HANDLETYPE       pCmp_handle,
                                       OMX_STRING             pCmp_name,
                                       OMX_VERSIONTYPE*    pCmp_version,
                                       OMX_VERSIONTYPE*   pSpec_version,
                                       OMX_UUIDTYPE*          pCmp_UUID)=0;

  // Invoke a command on the component
  virtual
  OMX_ERRORTYPE  send_command(OMX_HANDLETYPE pCmp_handle,
                              OMX_COMMANDTYPE       eCmd,
                              OMX_U32            uParam1,
                              OMX_PTR          pCmd_data)=0;

  // Get a Parameter setting from the component
  virtual
  OMX_ERRORTYPE  get_parameter(OMX_HANDLETYPE     pCmp_handle,
                               OMX_INDEXTYPE     eParam_index,
                               OMX_PTR            pParam_data)=0;

  // Send a parameter structure to the component
  virtual
  OMX_ERRORTYPE  set_parameter(OMX_HANDLETYPE     pCmp_handle,
                               OMX_INDEXTYPE     eParam_index,
                               OMX_PTR            pParam_data)=0;

  // Get a configuration structure from the component
  virtual
  OMX_ERRORTYPE  get_config(OMX_HANDLETYPE      pCmp_handle,
                            OMX_INDEXTYPE     eConfig_index,
                            OMX_PTR            pConfig_data)=0;

  // Set a component configuration value
  virtual
  OMX_ERRORTYPE  set_config(OMX_HANDLETYPE      pCmp_handle,
                            OMX_INDEXTYPE     eConfig_index,
                            OMX_PTR            pConfig_data)=0;

  // Translate the vendor specific extension string to
  // standardized index type
  virtual
  OMX_ERRORTYPE  get_extension_index(OMX_HANDLETYPE  pCmp_handle,
                                     OMX_STRING       pParamName,
                                     OMX_INDEXTYPE*   pIndexType)=0;

  // Get Current state information
  virtual
  OMX_ERRORTYPE  get_state(OMX_HANDLETYPE  pCmp_handle,
                           OMX_STATETYPE*       pState)=0;

  // Component Tunnel Request
  virtual
  OMX_ERRORTYPE  component_tunnel_request(OMX_HANDLETYPE           pCmp_handle,
                                          OMX_U32                        uPort,
                                          OMX_HANDLETYPE       pPeer_component,
                                          OMX_U32                   uPeer_port,
                                          OMX_TUNNELSETUPTYPE*   pTunnel_setup)=0;

  // Use a buffer already allocated by the IL client
  // or a buffer already supplied by a tunneled component
  virtual
  OMX_ERRORTYPE  use_buffer(OMX_HANDLETYPE                pCmp_handle,
                            OMX_BUFFERHEADERTYPE**        ppBuffer_hdr,
                            OMX_U32                             uPort,
                            OMX_PTR                         pApp_data,
                            OMX_U32                            uBytes,
                            OMX_U8*                           pBuffer)=0;

  // Request that the component allocate new buffer and associated header
  virtual
  OMX_ERRORTYPE  allocate_buffer(OMX_HANDLETYPE                pCmp_handle,
                                 OMX_BUFFERHEADERTYPE**        ppBuffer_hdr,
                                 OMX_U32                             uPort,
                                 OMX_PTR                         pApp_data,
                                 OMX_U32                            uBytes)=0;

  // Release the buffer and associated header from the component
  virtual
  OMX_ERRORTYPE  free_buffer(OMX_HANDLETYPE         pCmp_handle,
                             OMX_U32                      uPort,
                             OMX_BUFFERHEADERTYPE*      pBuffer)=0;

  // Send a filled buffer to an input uPort of a component
  virtual
  OMX_ERRORTYPE  empty_this_buffer(OMX_HANDLETYPE         pCmp_handle,
                                   OMX_BUFFERHEADERTYPE*      pBuffer)=0;

  // Send an empty buffer to an output uPort of a component
  virtual
  OMX_ERRORTYPE  fill_this_buffer(OMX_HANDLETYPE         pCmp_handle,
                                  OMX_BUFFERHEADERTYPE*      pBuffer)=0;

  // Set callbacks
  virtual
  OMX_ERRORTYPE  set_callbacks( OMX_HANDLETYPE        pCmp_handle,
                                OMX_CALLBACKTYPE*      pCallbacks,
                                OMX_PTR                 pApp_data)=0;

  // Component De-Initialize
  virtual
  OMX_ERRORTYPE  component_deinit( OMX_HANDLETYPE pCmp_handle)=0;

  // Use the Image already allocated via EGL
  virtual
  OMX_ERRORTYPE  use_EGL_image(OMX_HANDLETYPE                pCmp_handle,
                               OMX_BUFFERHEADERTYPE**        ppBuffer_hdr,
                               OMX_U32                             uPort,
                               OMX_PTR                         pApp_data,
                               void*                          pEgl_image)=0;

  // Component Role enum
  virtual
  OMX_ERRORTYPE  component_role_enum( OMX_HANDLETYPE pCmp_handle,
                                      OMX_U8*              pRole,
                                      OMX_U32             uIndex)=0;

};
#endif /* AW_OMX_COMPONENT_H */
