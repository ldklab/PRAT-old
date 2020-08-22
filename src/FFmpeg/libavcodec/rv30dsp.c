/*
 * RV30 decoder motion compensation functions
 * Copyright (c) 2007 Konstantin Shishkov
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * RV30 decoder motion compensation functions
 */

#include "avcodec.h"
#include "h264chroma.h"
#include "h264qpel.h"
#include "mathops.h"
#include "rv34dsp.h"

#define RV30_LOWPASS(OPNAME, OP) \
static void OPNAME ## rv30_tpel8_h_lowpass(uint8_t *dst, const uint8_t *src, int dstStride, int srcStride, const int C1, const int C2){\
    const int h = 8;\
    const uint8_t *cm = ff_crop_tab + MAX_NEG_CROP;\
    int i;\
    for(i = 0; i < h; i++)\
    {\
        OP(dst[0], (-(src[-1]+src[2]) + src[0]*C1 + src[1]*C2 + 8)>>4);\
        OP(dst[1], (-(src[ 0]+src[3]) + src[1]*C1 + src[2]*C2 + 8)>>4);\
        OP(dst[2], (-(src[ 1]+src[4]) + src[2]*C1 + src[3]*C2 + 8)>>4);\
        OP(dst[3], (-(src[ 2]+src[5]) + src[3]*C1 + src[4]*C2 + 8)>>4);\
        OP(dst[4], (-(src[ 3]+src[6]) + src[4]*C1 + src[5]*C2 + 8)>>4);\
        OP(dst[5], (-(src[ 4]+src[7]) + src[5]*C1 + src[6]*C2 + 8)>>4);\
        OP(dst[6], (-(src[ 5]+src[8]) + src[6]*C1 + src[7]*C2 + 8)>>4);\
        OP(dst[7], (-(src[ 6]+src[9]) + src[7]*C1 + src[8]*C2 + 8)>>4);\
        dst += dstStride;\
        src += srcStride;\
    }\
}\
\
static void OPNAME ## rv30_tpel8_v_lowpass(uint8_t *dst, const uint8_t *src, int dstStride, int srcStride, const int C1, const int C2){\
    const int w = 8;\
    const uint8_t *cm = ff_crop_tab + MAX_NEG_CROP;\
    int i;\
    for(i = 0; i < w; i++)\
    {\
        const int srcA = src[-1*srcStride];\
        const int src0 = src[0 *srcStride];\
        const int src1 = src[1 *srcStride];\
        const int src2 = src[2 *srcStride];\
        const int src3 = src[3 *srcStride];\
        const int src4 = src[4 *srcStride];\
        const int src5 = src[5 *srcStride];\
        const int src6 = src[6 *srcStride];\
        const int src7 = src[7 *srcStride];\
        const int src8 = src[8 *srcStride];\
        const int src9 = src[9 *srcStride];\
        OP(dst[0*dstStride], (-(srcA+src2) + src0*C1 + src1*C2 + 8)>>4);\
        OP(dst[1*dstStride], (-(src0+src3) + src1*C1 + src2*C2 + 8)>>4);\
        OP(dst[2*dstStride], (-(src1+src4) + src2*C1 + src3*C2 + 8)>>4);\
        OP(dst[3*dstStride], (-(src2+src5) + src3*C1 + src4*C2 + 8)>>4);\
        OP(dst[4*dstStride], (-(src3+src6) + src4*C1 + src5*C2 + 8)>>4);\
        OP(dst[5*dstStride], (-(src4+src7) + src5*C1 + src6*C2 + 8)>>4);\
        OP(dst[6*dstStride], (-(src5+src8) + src6*C1 + src7*C2 + 8)>>4);\
        OP(dst[7*dstStride], (-(src6+src9) + src7*C1 + src8*C2 + 8)>>4);\
        dst++;\
        src++;\
    }\
}\
\
static void OPNAME ## rv30_tpel8_hv_lowpass(uint8_t *dst, const uint8_t *src, int dstStride, int srcStride){\
    const int w = 8;\
    const int h = 8;\
    const uint8_t *cm = ff_crop_tab + MAX_NEG_CROP;\
    int i, j;\
    for(j = 0; j < h; j++){\
        for(i = 0; i < w; i++){\
            OP(dst[i], (\
                  src[srcStride*-1+i-1]  -12*src[srcStride*-1+i]  -6*src[srcStride*-1+i+1]    +src[srcStride*-1+i+2]+\
              -12*src[srcStride* 0+i-1] +144*src[srcStride* 0+i] +72*src[srcStride* 0+i+1] -12*src[srcStride* 0+i+2] +\
               -6*src[srcStride* 1+i-1]  +72*src[srcStride* 1+i] +36*src[srcStride* 1+i+1]  -6*src[srcStride* 1+i+2] +\
                  src[srcStride* 2+i-1]  -12*src[srcStride* 2+i]  -6*src[srcStride* 2+i+1]    +src[srcStride* 2+i+2] +\
                  128)>>8);\
        }\
        src += srcStride;\
        dst += dstStride;\
    }\
}\
\
static void OPNAME ## rv30_tpel8_hhv_lowpass(uint8_t *dst, const uint8_t *src, int dstStride, int srcStride){\
    const int w = 8;\
    const int h = 8;\
    const uint8_t *cm = ff_crop_tab + MAX_NEG_CROP;\
    int i, j;\
    for(j = 0; j < h; j++){\
        for(i = 0; i < w; i++){\
            OP(dst[i], (\
                  src[srcStride*-1+i-1]  -12*src[srcStride*-1+i+1]  -6*src[srcStride*-1+i]    +src[srcStride*-1+i+2]+\
              -12*src[srcStride* 0+i-1] +144*src[srcStride* 0+i+1] +72*src[srcStride* 0+i] -12*src[srcStride* 0+i+2]+\
               -6*src[srcStride* 1+i-1]  +72*src[srcStride* 1+i+1] +36*src[srcStride* 1+i]  -6*src[srcStride* 1+i+2]+\
                  src[srcStride* 2+i-1]  -12*src[srcStride* 2+i+1]  -6*src[srcStride* 2+i]    +src[srcStride* 2+i+2]+\
                  128)>>8);\
        }\
        src += srcStride;\
        dst += dstStride;\
    }\
}\
\
static void OPNAME ## rv30_tpel8_hvv_lowpass(uint8_t *dst, const uint8_t *src, int dstStride, int srcStride){\
    const int w = 8;\
    const int h = 8;\
    const uint8_t *cm = ff_crop_tab + MAX_NEG_CROP;\
    int i, j;\
    for(j = 0; j < h; j++){\
        for(i = 0; i < w; i++){\
            OP(dst[i], (\
                  src[srcStride*-1+i-1]  -12*src[srcStride*-1+i]  -6*src[srcStride*-1+i+1]    +src[srcStride*-1+i+2]+\
               -6*src[srcStride* 0+i-1]  +72*src[srcStride* 0+i] +36*src[srcStride* 0+i+1]  -6*src[srcStride* 0+i+2]+\
              -12*src[srcStride* 1+i-1] +144*src[srcStride* 1+i] +72*src[srcStride* 1+i+1] -12*src[srcStride* 1+i+2]+\
                  src[srcStride* 2+i-1]  -12*src[srcStride* 2+i]  -6*src[srcStride* 2+i+1]    +src[srcStride* 2+i+2]+\
                  128)>>8);\
        }\
        src += srcStride;\
        dst += dstStride;\
    }\
}\
\
static void OPNAME ## rv30_tpel8_hhvv_lowpass(uint8_t *dst, const uint8_t *src, int dstStride, int srcStride){\
    const int w = 8;\
    const int h = 8;\
    const uint8_t *cm = ff_crop_tab + MAX_NEG_CROP;\
    int i, j;\
    for(j = 0; j < h; j++){\
        for(i = 0; i < w; i++){\
            OP(dst[i], (\
               36*src[i+srcStride*0] +54*src[i+1+srcStride*0] +6*src[i+2+srcStride*0]+\
               54*src[i+srcStride*1] +81*src[i+1+srcStride*1] +9*src[i+2+srcStride*1]+\
                6*src[i+srcStride*2] + 9*src[i+1+srcStride*2] +  src[i+2+srcStride*2]+\
               128)>>8);\
        }\
        src += srcStride;\
        dst += dstStride;\
    }\
}\
\
static void OPNAME ## rv30_tpel16_v_lowpass(uint8_t *dst, const uint8_t *src, int dstStride, int srcStride, const int C1, const int C2){\
    OPNAME ## rv30_tpel8_v_lowpass(dst  , src  , dstStride, srcStride, C1, C2);\
    OPNAME ## rv30_tpel8_v_lowpass(dst+8, src+8, dstStride, srcStride, C1, C2);\
    src += 8*srcStride;\
    dst += 8*dstStride;\
    OPNAME ## rv30_tpel8_v_lowpass(dst  , src  , dstStride, srcStride, C1, C2);\
    OPNAME ## rv30_tpel8_v_lowpass(dst+8, src+8, dstStride, srcStride, C1, C2);\
}\
\
static void OPNAME ## rv30_tpel16_h_lowpass(uint8_t *dst, const uint8_t *src, int dstStride, int srcStride, const int C1, const int C2){\
    OPNAME ## rv30_tpel8_h_lowpass(dst  , src  , dstStride, srcStride, C1, C2);\
    OPNAME ## rv30_tpel8_h_lowpass(dst+8, src+8, dstStride, srcStride, C1, C2);\
    src += 8*srcStride;\
    dst += 8*dstStride;\
    OPNAME ## rv30_tpel8_h_lowpass(dst  , src  , dstStride, srcStride, C1, C2);\
    OPNAME ## rv30_tpel8_h_lowpass(dst+8, src+8, dstStride, srcStride, C1, C2);\
}\
\
static void OPNAME ## rv30_tpel16_hv_lowpass(uint8_t *dst, const uint8_t *src, int dstStride, int srcStride){\
    OPNAME ## rv30_tpel8_hv_lowpass(dst  , src  , dstStride, srcStride);\
    OPNAME ## rv30_tpel8_hv_lowpass(dst+8, src+8, dstStride, srcStride);\
    src += 8*srcStride;\
    dst += 8*dstStride;\
    OPNAME ## rv30_tpel8_hv_lowpass(dst  , src  , dstStride, srcStride);\
    OPNAME ## rv30_tpel8_hv_lowpass(dst+8, src+8, dstStride, srcStride);\
}\
\
static void OPNAME ## rv30_tpel16_hhv_lowpass(uint8_t *dst, const uint8_t *src, int dstStride, int srcStride){\
    OPNAME ## rv30_tpel8_hhv_lowpass(dst  , src  , dstStride, srcStride);\
    OPNAME ## rv30_tpel8_hhv_lowpass(dst+8, src+8, dstStride, srcStride);\
    src += 8*srcStride;\
    dst += 8*dstStride;\
    OPNAME ## rv30_tpel8_hhv_lowpass(dst  , src  , dstStride, srcStride);\
    OPNAME ## rv30_tpel8_hhv_lowpass(dst+8, src+8, dstStride, srcStride);\
}\
\
static void OPNAME ## rv30_tpel16_hvv_lowpass(uint8_t *dst, const uint8_t *src, int dstStride, int srcStride){\
    OPNAME ## rv30_tpel8_hvv_lowpass(dst  , src  , dstStride, srcStride);\
    OPNAME ## rv30_tpel8_hvv_lowpass(dst+8, src+8, dstStride, srcStride);\
    src += 8*srcStride;\
    dst += 8*dstStride;\
    OPNAME ## rv30_tpel8_hvv_lowpass(dst  , src  , dstStride, srcStride);\
    OPNAME ## rv30_tpel8_hvv_lowpass(dst+8, src+8, dstStride, srcStride);\
}\
\
static void OPNAME ## rv30_tpel16_hhvv_lowpass(uint8_t *dst, const uint8_t *src, int dstStride, int srcStride){\
    OPNAME ## rv30_tpel8_hhvv_lowpass(dst  , src  , dstStride, srcStride);\
    OPNAME ## rv30_tpel8_hhvv_lowpass(dst+8, src+8, dstStride, srcStride);\
    src += 8*srcStride;\
    dst += 8*dstStride;\
    OPNAME ## rv30_tpel8_hhvv_lowpass(dst  , src  , dstStride, srcStride);\
    OPNAME ## rv30_tpel8_hhvv_lowpass(dst+8, src+8, dstStride, srcStride);\
}\
\

#define RV30_MC(OPNAME, SIZE) \
static void OPNAME ## rv30_tpel ## SIZE ## _mc10_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride)\
{\
    OPNAME ## rv30_tpel ## SIZE ## _h_lowpass(dst, src, stride, stride, 12, 6);\
}\
\
static void OPNAME ## rv30_tpel ## SIZE ## _mc20_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride)\
{\
    OPNAME ## rv30_tpel ## SIZE ## _h_lowpass(dst, src, stride, stride, 6, 12);\
}\
\
static void OPNAME ## rv30_tpel ## SIZE ## _mc01_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride)\
{\
    OPNAME ## rv30_tpel ## SIZE ## _v_lowpass(dst, src, stride, stride, 12, 6);\
}\
\
static void OPNAME ## rv30_tpel ## SIZE ## _mc02_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride)\
{\
    OPNAME ## rv30_tpel ## SIZE ## _v_lowpass(dst, src, stride, stride, 6, 12);\
}\
\
static void OPNAME ## rv30_tpel ## SIZE ## _mc11_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride)\
{\
    OPNAME ## rv30_tpel ## SIZE ## _hv_lowpass(dst, src, stride, stride);\
}\
\
static void OPNAME ## rv30_tpel ## SIZE ## _mc12_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride)\
{\
    OPNAME ## rv30_tpel ## SIZE ## _hvv_lowpass(dst, src, stride, stride);\
}\
\
static void OPNAME ## rv30_tpel ## SIZE ## _mc21_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride)\
{\
    OPNAME ## rv30_tpel ## SIZE ## _hhv_lowpass(dst, src, stride, stride);\
}\
\
static void OPNAME ## rv30_tpel ## SIZE ## _mc22_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride)\
{\
    OPNAME ## rv30_tpel ## SIZE ## _hhvv_lowpass(dst, src, stride, stride);\
}\
\

#define op_avg(a, b)  a = (((a)+cm[b]+1)>>1)
#define op_put(a, b)  a = cm[b]


{



