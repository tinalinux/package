/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : transform_color_format.h
* Description :
* History :
*   Author  : AL3
*   Date    : 2013/05/05
*   Comment : complete the design code
*/

#ifndef TRANSFORM_COLOR_FORMAT_H
#define TRANSFORM_COLOR_FORMAT_H

//#include "libcedarv.h"

#include <vdecoder.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OutPutBufferInfo
{
    void* pAddr;
    int  nWidth;
    int  nHeight;
}OutPutBufferInfo;


void TransformMB32ToYV12(VideoPicture* pict,  OutPutBufferInfo* pOutPutBufferInfo);
void TransformYV12ToYUV420(VideoPicture* pict, OutPutBufferInfo* pOutPutBufferInfo);
void TransformYV12ToYUV420Soft(VideoPicture* pict, OutPutBufferInfo* pOutPutBufferInfo);
void TransformYV12ToYV12Hw(VideoPicture* pict,  OutPutBufferInfo* pOutPutBufferInfo);
void TransformYV12ToYV12Soft(VideoPicture* pict,  OutPutBufferInfo* pOutPutBufferInfo);

#ifdef __cplusplus
}
#endif

#endif
