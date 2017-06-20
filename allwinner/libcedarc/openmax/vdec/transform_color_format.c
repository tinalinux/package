/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : transform_color_format.c
* Description :
* History :
*   Author  : AL3
*   Date    : 2013/05/05
*   Comment : complete the design code
*/

#include "log.h"

#include "transform_color_format.h"
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>

enum FORMAT_CONVERT_COLORFORMAT
{
    CONVERT_SRC_COLOR_FORMAT_NONE = 0,
    CONVERT_SRC_COLOR_FORMAT_YUV420PLANNER,
    CONVERT_SRC_COLOR_FORMAT_YUV422PLANNER,
    CONVERT_SRC_COLOR_FORMAT_YUV420MB,
    CONVERT_COLOR_FORMAT_YUV422MB,
};

#if 0
typedef struct ScalerParameter
{
    int nMode; //0: YV12 1:thumb yuv420p
    int nFormatIn;
    int nFormatOut;

    int nWidthIn;
    int nHeightIn;

    int nWidthOut;
    int nHeightOut;

    void *pAddr_in_y;
    void *pAddr_in_c;
    uintptr_t uAddr_out_y;
    uintptr_t uAddr_out_u;
    uintptr_t uAddr_out_v;
}ScalerParameter;
#endif


#if 0
#if 0 //Don't HAVE_NEON
static void map32x32_to_yuv_Y(unsigned char* pSrcY,
                              unsigned char* pTarY,
                              unsigned int uCodedWidth,
                              unsigned int uCodedHeight)
{
    unsigned int i,j,l,m,n;
    unsigned int uWidthMb,uHeightMb,uTwombLine,uReconWidth;
    unsigned long uOffset;
    unsigned char *pPtr;

    pPtr = pSrcY;
    uWidthMb = (uCodedWidth+15)>>4;
    uHeightMb = (uCodedHeight+15)>>4;
    uTwombLine = (uHeightMb+1)>>1;
    uReconWidth = (uWidthMb+1)&0xfffffffe;

    for(i=0;i<uTwombLine;i++)
    {
        for(j=0;j<uReconWidth;j+=2)
        {
            for(l=0;l<32;l++)
            {
                //first mb
                m=i*32 + l;
                n= j*16;
                if(m<uCodedHeight && n<uCodedWidth)
                {
                    uOffset = m*uCodedWidth + n;
                    memcpy(pTarY+uOffset,pPtr,16);
                    pPtr += 16;
                }
                else
                    pPtr += 16;

                //second mb
                n= j*16+16;
                if(m<uCodedHeight && n<uCodedWidth)
                {
                    uOffset = m*uCodedWidth + n;
                    memcpy(pTarY+uOffset,pPtr,16);
                    pPtr += 16;
                }
                else
                    pPtr += 16;
            }
        }
    }
}

static void map32x32_to_yuv_C(int nMode,unsigned char* pSrcC,
                              unsigned char* pTarCb,
                              unsigned char* pTarCr,
                              unsigned int uCodedWidth,
                              unsigned int uCodedHeight)
{
    unsigned int i,j,l,m,n,k;
    unsigned int uWidthMb,uHeightMb,uFourmbLine,uReconWidth;
    unsigned char ArrayLine[16];
    unsigned long uOffset;
    unsigned char *pPtr;

    pPtr = pSrcC;
    uWidthMb = (uCodedWidth+7)>>3;
    uHeightMb = (uCodedHeight+7)>>3;
    uFourmbLine = (uHeightMb+3)>>2;
    uReconWidth = (uWidthMb+1)&0xfffffffe;

    for(i=0;i<uFourmbLine;i++)
    {
        for(j=0;j<uReconWidth;j+=2)
        {
            for(l=0;l<32;l++)
            {
                //first mb
                m=i*32 + l;
                n= j*8;
                if(m<uCodedHeight && n<uCodedWidth)
                {
                    uOffset = m*uCodedWidth + n;
                    memcpy(ArrayLine,pPtr,16);
                    for(k=0;k<8;k++)
                    {
                        *(pTarCb + uOffset + k) = 0xaa;//ArrayLine[2*k];
                        *(pTarCr + uOffset + k) = 0x55; //ArrayLine[2*k+1];
                    }
                    pPtr += 16;
                }
                else
                    pPtr += 16;

                //second mb
                n= j*8+8;
                if(m<uCodedHeight && n<uCodedWidth)
                {
                    uOffset = m*uCodedWidth + n;
                    memcpy(ArrayLine,pPtr,16);
                    for(k=0;k<8;k++)
                    {
                        *(pTarCb + uOffset + k) = 0xaa;//ArrayLine[2*k];
                        *(pTarCr + uOffset + k) = 0x55;//ArrayLine[2*k+1];
                    }
                    pPtr += 16;
                }
                else
                    pPtr += 16;
            }
        }
    }
}

static void map32x32_to_yuv_C_422(int nMode,unsigned char* pSrcC,
                                  unsigned char* pTarCb,
                                  unsigned char* pTarCr,
                                  unsigned int uCodedWidth,
                                  unsigned int uCodedHeight) {
    ;
}

#else


static void map32x32_to_yuv_Y(unsigned char* pSrcY,
                              unsigned char* pTarY,
                              unsigned int   uCodedWidth,
                              unsigned int   uCodedHeight)
{
    unsigned int i,j,l,m,n;
    unsigned int uWidthMb,uHeightMb,uTwombLine,uReconWidth;
    unsigned long uOffset;
    unsigned char *pPtr;
    unsigned char *pDstAsm,*pSrcAsm;

    pPtr = pSrcY;
    uWidthMb = (uCodedWidth+15)>>4;
    uHeightMb = (uCodedHeight+15)>>4;
    uTwombLine = (uHeightMb+1)>>1;
    uReconWidth = (uWidthMb+1)&0xfffffffe;

    for(i=0;i<uTwombLine;i++)
    {
        for(j=0;j<uWidthMb/2;j++)
        {
            for(l=0;l<32;l++)
            {
                //first mb
                m=i*32 + l;
                n= j*32;
                uOffset = m*uCodedWidth + n;
                //memcpy(pTarY+uOffset,pPtr,32);
                pDstAsm = pTarY+uOffset;
                pSrcAsm = pPtr;
                asm volatile (
                        "vld1.8         {d0 - d3}, [%[pSrcAsm]]              \n\t"
                        "vst1.8         {d0 - d3}, [%[pDstAsm]]              \n\t"
                        : [pDstAsm] "+r" (pDstAsm), [pSrcAsm] "+r" (pSrcAsm)
                        :  //[pSrcY] "r" (pSrcY)
                        : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6",
                         "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23",
                         "d24", "d28", "d29", "d30", "d31"
                        );

                pPtr += 32;
            }
        }

        //LOGV("uWidthMb:%d",uWidthMb);
        if(uWidthMb & 1)
        {
            j = uWidthMb-1;
            for(l=0;l<32;l++)
            {
                //first mb
                m=i*32 + l;
                n= j*16;
                if(m<uCodedHeight && n<uCodedWidth)
                {
                    uOffset = m*uCodedWidth + n;
                    //memcpy(pTarY+uOffset,pPtr,16);
                    pDstAsm = pTarY+uOffset;
                    pSrcAsm = pPtr;
                    asm volatile (
                            "vld1.8         {d0 - d1}, [%[pSrcAsm]]              \n\t"
                            "vst1.8         {d0 - d1}, [%[pDstAsm]]              \n\t"
                            : [pDstAsm] "+r" (pDstAsm), [pSrcAsm] "+r" (pSrcAsm)
                            :  //[pSrcY] "r" (pSrcY)
                            : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16",
                                "d17", "d18", "d19", "d20", "d21", "d22", "d23",
                                "d24", "d28", "d29", "d30", "d31"
                            );
                }

                pPtr += 16;
                pPtr += 16;
            }
        }
    }
}

static void map32x32_to_yuv_C(int nMode,
                              unsigned char* pSrcC,
                              unsigned char* pTarCb,
                              unsigned char* pTarCr,
                              unsigned int uCodedWidth,
                              unsigned int uCodedHeight)
{
    unsigned int i,j,l,m,n,k;
    unsigned int uWidthMb,uHeightMb,uFourmbLine,uReconWidth;
    unsigned long uOffset;
    unsigned char *pPtr;
    unsigned char *pDstAsm0,*pDstAsm1,*pSrcAsm;
    unsigned char ArrayLine[16];
    int nDstStride = nMode==0 ? (uCodedWidth + 15) & (~15) : uCodedWidth;

    pPtr = pSrcC;
    uWidthMb = (uCodedWidth+7)>>3;
    uHeightMb = (uCodedHeight+7)>>3;
    uFourmbLine = (uHeightMb+3)>>2;
    uReconWidth = (uWidthMb+1)&0xfffffffe;

    for(i=0;i<uFourmbLine;i++)
    {
        for(j=0;j<uWidthMb/2;j++)
        {
            for(l=0;l<32;l++)
            {
                //first mb
                m=i*32 + l;
                n= j*16;
                if(m<uCodedHeight && n<uCodedWidth)
                {
                    uOffset = m*nDstStride + n;

                    pDstAsm0 = pTarCb + uOffset;
                    pDstAsm1 = pTarCr+uOffset;
                    pSrcAsm = pPtr;
//                    for(k=0;k<16;k++)
//                    {
//                        pDstAsm0[k] = pSrcAsm[2*k];
//                        pDstAsm1[k] = pSrcAsm[2*k+1];
//                    }
                    asm volatile (
                            "vld1.8         {d0 - d3}, [%[pSrcAsm]]              \n\t"
                            "vuzp.8         d0, d1              \n\t"
                            "vuzp.8         d2, d3              \n\t"
                            "vst1.8         {d0}, [%[pDstAsm0]]!              \n\t"
                            "vst1.8         {d2}, [%[pDstAsm0]]!              \n\t"
                            "vst1.8         {d1}, [%[pDstAsm1]]!              \n\t"
                            "vst1.8         {d3}, [%[pDstAsm1]]!              \n\t"
                             : [pDstAsm0] "+r" (pDstAsm0), [pDstAsm1] "+r" (pDstAsm1),
                               [pSrcAsm] "+r" (pSrcAsm)
                             :  //[pSrcY] "r" (pSrcY)
                             : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16",
                                "d17", "d18", "d19", "d20", "d21", "d22", "d23",
                                "d24", "d28", "d29", "d30", "d31"
                             );
                }

                pPtr += 32;
            }
        }

        if(uWidthMb & 1)
        {
            j= uWidthMb-1;
            for(l=0;l<32;l++)
            {
                m=i*32 + l;
                n= j*8;

                if(m<uCodedHeight && n<uCodedWidth)
                {
                    uOffset = m*nDstStride + n;
                    memcpy(ArrayLine,pPtr,16);
                    for(k=0;k<8;k++)
                    {
                        *(pTarCb + uOffset + k) = ArrayLine[2*k];
                        *(pTarCr + uOffset + k) = ArrayLine[2*k+1];
                    }
                }

                pPtr += 16;
                pPtr += 16;
            }
        }
    }
}

static void map32x32_to_yuv_C_422(int nMode,
                                  unsigned char* pSrcC,
                                  unsigned char* pTarCb,
                                  unsigned char* pTarCr,
                                  unsigned int uCodedWidth,
                                  unsigned int uCodedHeight)
{
    unsigned int i,j,l,m,n,k;
    unsigned int uWidthMb,uHeightMb,uTwombLine,uReconWidth;
    unsigned long uOffset;
    unsigned char *pPtr;
    unsigned char *pDstAsm0,*pDstAsm1,*pSrcAsm;
    unsigned char ArrayLine[16];

    CEDARC_UNUSE(nMode);

    pPtr = pSrcC;
    uWidthMb = (uCodedWidth+7)>>3;
    uHeightMb = (uCodedHeight+7)>>3;
    uTwombLine = (uHeightMb+1)>>1;
    uReconWidth = (uWidthMb+1)&0xfffffffe;

    for(i=0;i<uTwombLine;i++)
    {
        for(j=0;j<uWidthMb/2;j++)
        {
            for(l=0;l<16;l++)
            {
                //first mb
                m=i*16 + l;
                n= j*16;
                if(m<uCodedHeight && n<uCodedWidth)
                {
                    uOffset = m*uCodedWidth + n;

                    pDstAsm0 = pTarCb + uOffset;
                    pDstAsm1 = pTarCr+uOffset;
                    pSrcAsm = pPtr;
//                    for(k=0;k<16;k++)
//                    {
//                        pDstAsm0[k] = pSrcAsm[2*k];
//                        pDstAsm1[k] = pSrcAsm[2*k+1];
//                    }
                    asm volatile (
                            "vld1.8         {d0 - d3}, [%[pSrcAsm]]              \n\t"
                            "vuzp.8         d0, d1              \n\t"
                            "vuzp.8         d2, d3              \n\t"
                            "vst1.8         {d0}, [%[pDstAsm0]]!              \n\t"
                            "vst1.8         {d2}, [%[pDstAsm0]]!              \n\t"
                            "vst1.8         {d1}, [%[pDstAsm1]]!              \n\t"
                            "vst1.8         {d3}, [%[pDstAsm1]]!              \n\t"
                             : [pDstAsm0] "+r" (pDstAsm0), [pDstAsm1] "+r" (pDstAsm1),
                             [pSrcAsm] "+r" (pSrcAsm)
                             :  //[pSrcY] "r" (pSrcY)
                             : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6",
                                "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23",
                                "d24", "d28", "d29", "d30", "d31"
                             );
                }

                pPtr += 32;
                pPtr += 32;
            }
        }

        if(uWidthMb & 1)
        {
            j= uWidthMb-1;
            for(l=0;l<16;l++)
            {
                m=i*32 + l;
                n= j*8;

                if(m<uCodedHeight && n<uCodedWidth)
                {
                    uOffset = m*uCodedWidth + n;
                    memcpy(ArrayLine,pPtr,16);
                    for(k=0;k<8;k++)
                    {
                        *(pTarCb + uOffset + k) = ArrayLine[2*k];
                        *(pTarCr + uOffset + k) = ArrayLine[2*k+1];
                    }
                }

                pPtr += 32;
                pPtr += 32;
            }
        }
    }
}
#endif


static void SoftwarePictureScaler(ScalerParameter *pScalerPara)
{
    map32x32_to_yuv_Y((unsigned char*)pScalerPara->pAddr_in_y,
                      (unsigned char*)pScalerPara->uAddr_out_y,
                      (unsigned int)pScalerPara->nWidthOut,
                      (unsigned int)pScalerPara->nHeightOut);

    if (pScalerPara->nFormatIn == CONVERT_COLOR_FORMAT_YUV422MB)
        map32x32_to_yuv_C_422(pScalerPara->nMode,
                              (unsigned char*)pScalerPara->pAddr_in_c,
                              (unsigned char*)pScalerPara->uAddr_out_u,
                              (unsigned char*)pScalerPara->uAddr_out_v,
                              (unsigned int)pScalerPara->nWidthOut / 2,
                              (unsigned int)pScalerPara->nHeightOut / 2);
    else
        map32x32_to_yuv_C(pScalerPara->nMode,
                          (unsigned char*)pScalerPara->pAddr_in_c,
                          (unsigned char*)pScalerPara->uAddr_out_u,
                          (unsigned char*)pScalerPara->uAddr_out_v,
                          (unsigned int)pScalerPara->nWidthOut / 2,
                          (unsigned int)pScalerPara->nHeightOut / 2);

    return;
}

void TransformMB32ToYV12(VideoPicture* pPicture, OutPutBufferInfo* pOutPutBufferInfo)
{
    ScalerParameter mScalerPara;
    int             nDisplayHeightAlign;
    int             nDisplayWidthAlign;
    int             nDstStrideC;
    int             nDstSizeY;
    int             nDstSizeC;
    int             nAllocSize;

    if(pPicture == NULL)
        return;

    pPicture->nHeight = (pPicture->nHeight + 7) & (~7);
    nDisplayHeightAlign = (pPicture->nHeight + 1) & (~1);

#if(CONFIG_CHIP == OPTION_CHIP_1673)
    nDisplayWidthAlign  = (pPicture->nWidth+ 31) & (~31);//on chip-1673, gpu is 32pixel align
#else
    nDisplayWidthAlign  = (pPicture->nWidth+ 15) & (~15);//other chip, gpu buffer is 16 align
#endif

    nDstSizeY           = nDisplayWidthAlign * nDisplayHeightAlign;
    nDstStrideC         = (pPicture->nWidth/2 + 15) & (~15);
    nDstSizeC           = nDstStrideC * (nDisplayHeightAlign/2);
    nAllocSize           = nDstSizeY + nDstSizeC * 2;

    mScalerPara.nMode       = 0;
    mScalerPara.nFormatIn  = (pPicture->ePixelFormat == PIXEL_FORMAT_YUV_MB32_422) ?  \
                                 CONVERT_COLOR_FORMAT_YUV422MB : CONVERT_SRC_COLOR_FORMAT_YUV420MB;
    mScalerPara.nFormatOut = CONVERT_SRC_COLOR_FORMAT_YUV420PLANNER;
    mScalerPara.nWidthIn   = pPicture->nWidth;
    mScalerPara.nHeightIn  = pPicture->nHeight;
    mScalerPara.pAddr_in_y  = (void*)pPicture->pData0;
    mScalerPara.pAddr_in_c  = (void*)pPicture->pData1;
#if 0
    cedarx_cache_op(mScalerPara.pAddr_in_y, mScalerPara.pAddr_in_y+pPicture->size_y,
                    CEDARX_DCACHE_FLUSH);
    cedarx_cache_op(mScalerPara.pAddr_in_c, mScalerPara.pAddr_in_c+pPicture->size_u,
                    CEDARX_DCACHE_FLUSH);
#endif
    mScalerPara.nWidthOut  = nDisplayWidthAlign;
    mScalerPara.nHeightOut = nDisplayHeightAlign;

    mScalerPara.uAddr_out_y = (uintptr_t)pOutPutBufferInfo->pAddr;
    mScalerPara.uAddr_out_v = mScalerPara.uAddr_out_y + nDstSizeY;
    mScalerPara.uAddr_out_u = mScalerPara.uAddr_out_v + nDstSizeC;

    //* use neon accelarator instruction to transform the pixel format,
    //* slow if buffer is not cached(DMA nMode).
    SoftwarePictureScaler(&mScalerPara);

    return;
}

#endif
void TransformYV12ToYUV420(VideoPicture* pPicture, OutPutBufferInfo* pOutPutBufferInfo)
{
    int i;
    int nPicRealWidth;
    int nPicRealHeight;
    int nSrcBufWidth;
    int nSrcBufHeight;
    int nDstBufWidth;
    int nDstBufHeight;
    int nCopyDataWidth;
    int nCopyDataHeight;
    unsigned char* dstPtr;
    unsigned char* srcPtr;
    dstPtr      = (unsigned char*)pOutPutBufferInfo->pAddr;
    srcPtr      = (unsigned char*)pPicture->pData0;

    nPicRealWidth  = pPicture->nRightOffset - pPicture->nLeftOffset;
    nPicRealHeight = pPicture->nBottomOffset - pPicture->nTopOffset;

    //* if the uOffset is not right, we should not use them to compute width and height
    if(nPicRealWidth <= 0 || nPicRealHeight <= 0)
    {
        nPicRealWidth  = pPicture->nWidth;
        nPicRealHeight = pPicture->nHeight;
    }

    nSrcBufWidth  = (pPicture->nWidth + 15) & ~15;
    nSrcBufHeight = (pPicture->nHeight + 15) & ~15;

    //* On chip-1673, the gpu is 32 align ,but here is not copy to gpu, so also 16 align.
    //* On other chip, gpu buffer is 16 align.
    //nDstBufWidth = (pOutPutBufferInfo->nWidth+ 15)&~15;
    nDstBufWidth  = pOutPutBufferInfo->nWidth;

    nDstBufHeight = pOutPutBufferInfo->nHeight;

    nCopyDataWidth  = nPicRealWidth;
    nCopyDataHeight = nPicRealHeight;

    logv("nPicRealWidth & H = %d, %d, nSrcBufWidth & H = %d, %d, nDstBufWidth & H = %d, %d,\
          nCopyDataWidth & H = %d, %d",
          nPicRealWidth,nPicRealHeight,
          nSrcBufWidth,nSrcBufHeight,
          nDstBufWidth,nDstBufHeight,
          nCopyDataWidth,nCopyDataHeight);

    //*copy y
    for(i=0; i < nCopyDataHeight; i++)
    {
        memcpy(dstPtr, srcPtr, nCopyDataWidth);
        dstPtr += nDstBufWidth;
        srcPtr += nSrcBufWidth;
    }

    //*copy u
    srcPtr = ((unsigned char*)pPicture->pData0) + nSrcBufWidth*nSrcBufHeight*5/4;
    nCopyDataWidth  = (nCopyDataWidth+1)/2;
    nCopyDataHeight = (nCopyDataHeight+1)/2;
    for(i=0; i < nCopyDataHeight; i++)
    {
        memcpy(dstPtr, srcPtr, nCopyDataWidth);
        dstPtr += nDstBufWidth/2;
        srcPtr += nSrcBufWidth/2;
    }

    //*copy v
    srcPtr = ((unsigned char*)pPicture->pData0) + nSrcBufWidth*nSrcBufHeight;
    for(i=0; i<nCopyDataHeight; i++)
    {
        memcpy(dstPtr, srcPtr, nCopyDataWidth);
        dstPtr += nDstBufWidth/2;    //a31 gpu, uv is half of y
        srcPtr += nSrcBufWidth/2;
    }

    return;
}

void TransformYV12ToYUV420Soft(VideoPicture* pPicture, OutPutBufferInfo* pOutPutBufferInfo)
{
    int i;
    unsigned char* dstPtr;
    unsigned char* srcPtr;
    int copyHeight;
    int copyWidth;
    int dstWidth;
    int dstHeight;

    dstPtr = (unsigned char*)pOutPutBufferInfo->pAddr;
    srcPtr = (unsigned char*)pPicture->pData0 +
             (pPicture->nWidth * pPicture->nTopOffset + pPicture->nLeftOffset);
    copyHeight = pPicture->nBottomOffset - pPicture->nTopOffset;
    copyWidth  = pPicture->nRightOffset - pPicture->nLeftOffset;

    //* On chip-1673, the gpu is 32 align ,but here is not copy to gpu, so also 16 align.
    //* On other chip, gpu buffer is 16 align.
    //dstWidth   = (pOutPutBufferInfo->nWidth+15)&~15;
    dstWidth   = pOutPutBufferInfo->nWidth;
    dstHeight  = pOutPutBufferInfo->nHeight;

    logv("c dst w: %d,  dst h: %d;;   src w: %d, src h: %d; top: %d, left: %d, b:%d, r:%d",
          dstWidth, dstHeight, copyWidth, copyHeight,
          pPicture->nTopOffset, pPicture->nLeftOffset,
          pPicture->nBottomOffset, pPicture->nRightOffset);

    //*copy y
    for(i = 0; i < dstHeight; i++)
    {
        memcpy(dstPtr, srcPtr, copyWidth);
        srcPtr += pPicture->nWidth;
        dstPtr += dstWidth;
    }

    //*copy v
    srcPtr = ((unsigned char*)pPicture->pData0) + pPicture->nWidth * pPicture->nHeight * 5/4 + \
               pPicture->nWidth * pPicture->nTopOffset / 4 + pPicture->nLeftOffset / 2 ;

    dstHeight = (dstHeight+1)/2;
    dstWidth  = dstWidth/2;
    copyWidth = copyWidth/2;

    for(i = 0; i < dstHeight; i++)
    {
        memcpy(dstPtr, srcPtr, copyWidth);
        srcPtr += pPicture->nWidth/2;
        dstPtr += dstWidth;
    }

    //copy u
    srcPtr =((unsigned char*)pPicture->pData0) + pPicture->nWidth * pPicture->nHeight + \
              pPicture->nWidth * pPicture->nTopOffset / 4 + pPicture->nLeftOffset / 2 ;

    for(i = 0; i < dstHeight; i++)
    {
        memcpy(dstPtr, srcPtr, copyWidth);
        srcPtr += pPicture->nWidth/2;
        dstPtr += dstWidth;
    }

    return;
}

void TransformYV12ToYV12Hw(VideoPicture* pPicture, OutPutBufferInfo* pOutPutBufferInfo)
{
    int i;
    int nPicRealWidth;
    int nPicRealHeight;
    int nSrcBufWidth;
    int nSrcBufHeight;
    int nDstBufWidth;
    int nDstBufHeight;
    int nCopyDataWidth;
    int nCopyDataHeight;
    unsigned char* dstPtr;
    unsigned char* srcPtr;
    dstPtr      = (unsigned char*)pOutPutBufferInfo->pAddr;
    srcPtr      = (unsigned char*)pPicture->pData0;

    nPicRealWidth  = pPicture->nRightOffset - pPicture->nLeftOffset;
    nPicRealHeight = pPicture->nBottomOffset - pPicture->nTopOffset;

    //* if the uOffset is not right, we should use them to compute width and height
    if(nPicRealWidth <= 0 || nPicRealHeight <= 0)
    {
        nPicRealWidth  = pPicture->nWidth;
        nPicRealHeight = pPicture->nHeight;
    }

    nSrcBufWidth  = (pPicture->nWidth + 15) & ~15;
    nSrcBufHeight = (pPicture->nHeight + 15) & ~15;

    nDstBufWidth = (pOutPutBufferInfo->nWidth + 31)&~31;   //gpu is 32pixel align

    nDstBufHeight = pOutPutBufferInfo->nHeight;

    nCopyDataWidth  = nPicRealWidth;
    nCopyDataHeight = nPicRealHeight;

    logv("nPicRealWidth & H = %d, %d, nSrcBufWidth & H = %d, %d, nDstBufWidth & H = %d, %d,\
          nCopyDataWidth & H = %d, %d",
          nPicRealWidth,nPicRealHeight,
          nSrcBufWidth,nSrcBufHeight,
          nDstBufWidth,nDstBufHeight,
          nCopyDataWidth,nCopyDataHeight);

    //*copy y
    for(i=0; i < nCopyDataHeight; i++)
    {
        memcpy(dstPtr, srcPtr, nCopyDataWidth);
        dstPtr += nDstBufWidth;
        srcPtr += nSrcBufWidth;
    }

    nCopyDataWidth  = (nCopyDataWidth+1)/2;
    nCopyDataHeight = (nCopyDataHeight+1)/2;

    //* the uv is 16 align
    nDstBufWidth = (nDstBufWidth/2 + 15)&~15;
    //*copy v
    srcPtr = ((unsigned char*)pPicture->pData0) + nSrcBufWidth*nSrcBufHeight;
    for(i=0; i < nCopyDataHeight; i++)
    {
        memcpy(dstPtr, srcPtr, nCopyDataWidth);
        dstPtr += nDstBufWidth;
        srcPtr += nSrcBufWidth/2;
    }

    //*copy u
    srcPtr = ((unsigned char*)pPicture->pData0) + nSrcBufWidth*nSrcBufHeight*5/4;
    for(i=0; i<nCopyDataHeight; i++)
    {
        memcpy(dstPtr, srcPtr, nCopyDataWidth);
        dstPtr += nDstBufWidth;
        srcPtr += nSrcBufWidth/2;
    }

    return;
}

void TransformYV12ToYV12Soft(VideoPicture* pPicture, OutPutBufferInfo* pOutPutBufferInfo)
{
    int i;
    unsigned char* dstPtr;
    unsigned char* srcPtr;
    int copyHeight;
    int copyWidth;
    int dstWidth;
    int dstHeight;

    dstPtr = (unsigned char*)pOutPutBufferInfo->pAddr;
    srcPtr = (unsigned char*)pPicture->pData0 +
             (pPicture->nWidth * pPicture->nTopOffset + pPicture->nLeftOffset);
    copyHeight = pPicture->nBottomOffset - pPicture->nTopOffset;
    copyWidth  = pPicture->nRightOffset - pPicture->nLeftOffset;
    dstWidth   = (pOutPutBufferInfo->nWidth + 31)&~31;   //gpu is 32pixel align
    dstHeight  = pOutPutBufferInfo->nHeight;

    logv("c dst w: %d,  dst h: %d;;   src w: %d, src h: %d; top: %d, left: %d, b:%d, r:%d",
          dstWidth, dstHeight, copyWidth, copyHeight,
          pPicture->nTopOffset, pPicture->nLeftOffset,
          pPicture->nBottomOffset, pPicture->nRightOffset);

    //*copy y
    for(i = 0; i < dstHeight; i++)
    {
        memcpy(dstPtr, srcPtr, copyWidth);
        srcPtr += pPicture->nWidth;
        dstPtr += dstWidth;
    }

    //*copy v
    srcPtr =((unsigned char*)pPicture->pData0) + pPicture->nWidth * pPicture->nHeight + \
              pPicture->nWidth * pPicture->nTopOffset / 4 + pPicture->nLeftOffset / 2 ;

    dstHeight = (dstHeight+1)/2;

    //* the uv is 16 align
    dstWidth  = (dstWidth/2+15)&~15;

    copyWidth = copyWidth/2;

    for(i = 0; i < dstHeight; i++)
    {
        memcpy(dstPtr, srcPtr, copyWidth);
        srcPtr += pPicture->nWidth/2;
        dstPtr += dstWidth;
    }

    //copy u
    srcPtr = ((unsigned char*)pPicture->pData0) + pPicture->nWidth * pPicture->nHeight * 5/4 + \
               pPicture->nWidth * pPicture->nTopOffset / 4 + pPicture->nLeftOffset / 2 ;

    for(i = 0; i < dstHeight; i++)
    {
        memcpy(dstPtr, srcPtr, copyWidth);
        srcPtr += pPicture->nWidth/2;
        dstPtr += dstWidth;
    }

    return;
}

#if 0
void TransformToGPUBuffer0(cedarv_picture_t* pPicture, void* ybuf)
{
    ScalerParameter mScalerPara;
    int             nDisplayHeightAlign;
    int             nDisplayWidthAlign;
    int             nDstStrideC;
    int             nDstSizeY;
    int             nDstSizeC;
    int             nAllocSize;

    //memcpy(ybuf, pPicture->y, pPicture->size_y + pPicture->size_u);

    {
        int i;
        int widthAlign;
        int heightAlign;
        int cHeight;
        int cWidth;
        int dstCStride;
        int GPUFBStride;
        unsigned char* dstPtr;
        unsigned char* srcPtr;
        dstPtr = (unsigned char*)ybuf;
        //srcPtr = (unsigned char*)cedarv_address_phy2vir((void*)pOverlayParam->addr[0]);
        srcPtr = pPicture->y;
        //widthAlign = (mWidth + 15) & ~15;
        //heightAlign = (mHeight + 15) & ~15;
        widthAlign = (pPicture->display_width+15)&~15;  //hw_decoder is 16pixel align
        heightAlign = (pPicture->display_height+15)&~15;
        GPUFBStride = (pPicture->display_width + 31)&~31;   //gpu is 32pixel align
        for(i=0; i<heightAlign; i++)
        {
            memcpy(dstPtr, srcPtr, widthAlign);
            dstPtr += GPUFBStride;
            srcPtr += widthAlign;
        }
        //cWidth = (mWidth/2 + 15) & ~15;
        //equal to GPUFBStride/2. hw_decoder's uv is 16pixel align
        cWidth = (pPicture->display_width/2 + 15) & ~15;
        cHeight = heightAlign;
        for(i=0; i<cHeight; i++)
        {
            memcpy(dstPtr, srcPtr, cWidth);
            dstPtr += cWidth;
            srcPtr += cWidth;
        }
    }
    return;
}
#endif
