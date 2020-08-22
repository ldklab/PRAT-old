/*
 * RV40 decoder motion compensation functions
 * Copyright (c) 2008 Konstantin Shishkov
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
 * RV40 decoder motion compensation functions
 */

#include "libavutil/common.h"
#include "libavutil/intreadwrite.h"
#include "avcodec.h"
#include "h264qpel.h"
#include "mathops.h"
#include "pixels.h"
#include "rnd_avg.h"
#include "rv34dsp.h"
#include "libavutil/avassert.h"

#define RV40_LOWPASS(OPNAME, OP) \
static void OPNAME ## rv40_qpel8_h_lowpass(uint8_t *dst, const uint8_t *src, int dstStride, int srcStride,\
                                                     const int h, const int C1, const int C2, const int SHIFT){\
    const uint8_t *cm = ff_crop_tab + MAX_NEG_CROP;\
    int i;\
    for(i = 0; i < h; i++)\
    {\
        OP(dst[0], (src[-2] + src[ 3] - 5*(src[-1]+src[2]) + src[0]*C1 + src[1]*C2 + (1<<(SHIFT-1))) >> SHIFT);\
        OP(dst[1], (src[-1] + src[ 4] - 5*(src[ 0]+src[3]) + src[1]*C1 + src[2]*C2 + (1<<(SHIFT-1))) >> SHIFT);\
        OP(dst[2], (src[ 0] + src[ 5] - 5*(src[ 1]+src[4]) + src[2]*C1 + src[3]*C2 + (1<<(SHIFT-1))) >> SHIFT);\
        OP(dst[3], (src[ 1] + src[ 6] - 5*(src[ 2]+src[5]) + src[3]*C1 + src[4]*C2 + (1<<(SHIFT-1))) >> SHIFT);\
        OP(dst[4], (src[ 2] + src[ 7] - 5*(src[ 3]+src[6]) + src[4]*C1 + src[5]*C2 + (1<<(SHIFT-1))) >> SHIFT);\
        OP(dst[5], (src[ 3] + src[ 8] - 5*(src[ 4]+src[7]) + src[5]*C1 + src[6]*C2 + (1<<(SHIFT-1))) >> SHIFT);\
        OP(dst[6], (src[ 4] + src[ 9] - 5*(src[ 5]+src[8]) + src[6]*C1 + src[7]*C2 + (1<<(SHIFT-1))) >> SHIFT);\
        OP(dst[7], (src[ 5] + src[10] - 5*(src[ 6]+src[9]) + src[7]*C1 + src[8]*C2 + (1<<(SHIFT-1))) >> SHIFT);\
        dst += dstStride;\
        src += srcStride;\
    }\
}\
\
static void OPNAME ## rv40_qpel8_v_lowpass(uint8_t *dst, const uint8_t *src, int dstStride, int srcStride,\
                                           const int w, const int C1, const int C2, const int SHIFT){\
    const uint8_t *cm = ff_crop_tab + MAX_NEG_CROP;\
    int i;\
    for(i = 0; i < w; i++)\
    {\
        const int srcB  = src[-2*srcStride];\
        const int srcA  = src[-1*srcStride];\
        const int src0  = src[0 *srcStride];\
        const int src1  = src[1 *srcStride];\
        const int src2  = src[2 *srcStride];\
        const int src3  = src[3 *srcStride];\
        const int src4  = src[4 *srcStride];\
        const int src5  = src[5 *srcStride];\
        const int src6  = src[6 *srcStride];\
        const int src7  = src[7 *srcStride];\
        const int src8  = src[8 *srcStride];\
        const int src9  = src[9 *srcStride];\
        const int src10 = src[10*srcStride];\
        OP(dst[0*dstStride], (srcB + src3  - 5*(srcA+src2) + src0*C1 + src1*C2 + (1<<(SHIFT-1))) >> SHIFT);\
        OP(dst[1*dstStride], (srcA + src4  - 5*(src0+src3) + src1*C1 + src2*C2 + (1<<(SHIFT-1))) >> SHIFT);\
        OP(dst[2*dstStride], (src0 + src5  - 5*(src1+src4) + src2*C1 + src3*C2 + (1<<(SHIFT-1))) >> SHIFT);\
        OP(dst[3*dstStride], (src1 + src6  - 5*(src2+src5) + src3*C1 + src4*C2 + (1<<(SHIFT-1))) >> SHIFT);\
        OP(dst[4*dstStride], (src2 + src7  - 5*(src3+src6) + src4*C1 + src5*C2 + (1<<(SHIFT-1))) >> SHIFT);\
        OP(dst[5*dstStride], (src3 + src8  - 5*(src4+src7) + src5*C1 + src6*C2 + (1<<(SHIFT-1))) >> SHIFT);\
        OP(dst[6*dstStride], (src4 + src9  - 5*(src5+src8) + src6*C1 + src7*C2 + (1<<(SHIFT-1))) >> SHIFT);\
        OP(dst[7*dstStride], (src5 + src10 - 5*(src6+src9) + src7*C1 + src8*C2 + (1<<(SHIFT-1))) >> SHIFT);\
        dst++;\
        src++;\
    }\
}\
\
static void OPNAME ## rv40_qpel16_v_lowpass(uint8_t *dst, const uint8_t *src, int dstStride, int srcStride,\
                                            const int w, const int C1, const int C2, const int SHIFT){\
    OPNAME ## rv40_qpel8_v_lowpass(dst  , src  , dstStride, srcStride, 8, C1, C2, SHIFT);\
    OPNAME ## rv40_qpel8_v_lowpass(dst+8, src+8, dstStride, srcStride, 8, C1, C2, SHIFT);\
    src += 8*srcStride;\
    dst += 8*dstStride;\
    OPNAME ## rv40_qpel8_v_lowpass(dst  , src  , dstStride, srcStride, w-8, C1, C2, SHIFT);\
    OPNAME ## rv40_qpel8_v_lowpass(dst+8, src+8, dstStride, srcStride, w-8, C1, C2, SHIFT);\
}\
\
static void OPNAME ## rv40_qpel16_h_lowpass(uint8_t *dst, const uint8_t *src, int dstStride, int srcStride,\
                                            const int h, const int C1, const int C2, const int SHIFT){\
    OPNAME ## rv40_qpel8_h_lowpass(dst  , src  , dstStride, srcStride, 8, C1, C2, SHIFT);\
    OPNAME ## rv40_qpel8_h_lowpass(dst+8, src+8, dstStride, srcStride, 8, C1, C2, SHIFT);\
    src += 8*srcStride;\
    dst += 8*dstStride;\
    OPNAME ## rv40_qpel8_h_lowpass(dst  , src  , dstStride, srcStride, h-8, C1, C2, SHIFT);\
    OPNAME ## rv40_qpel8_h_lowpass(dst+8, src+8, dstStride, srcStride, h-8, C1, C2, SHIFT);\
}\
\

#define RV40_MC(OPNAME, SIZE) \
static void OPNAME ## rv40_qpel ## SIZE ## _mc10_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride)\
{\
    OPNAME ## rv40_qpel ## SIZE ## _h_lowpass(dst, src, stride, stride, SIZE, 52, 20, 6);\
}\
\
static void OPNAME ## rv40_qpel ## SIZE ## _mc30_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride)\
{\
    OPNAME ## rv40_qpel ## SIZE ## _h_lowpass(dst, src, stride, stride, SIZE, 20, 52, 6);\
}\
\
static void OPNAME ## rv40_qpel ## SIZE ## _mc01_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride)\
{\
    OPNAME ## rv40_qpel ## SIZE ## _v_lowpass(dst, src, stride, stride, SIZE, 52, 20, 6);\
}\
\
static void OPNAME ## rv40_qpel ## SIZE ## _mc11_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride)\
{\
    uint8_t full[SIZE*(SIZE+5)];\
    uint8_t * const full_mid = full + SIZE*2;\
    put_rv40_qpel ## SIZE ## _h_lowpass(full, src - 2*stride, SIZE, stride, SIZE+5, 52, 20, 6);\
    OPNAME ## rv40_qpel ## SIZE ## _v_lowpass(dst, full_mid, stride, SIZE, SIZE, 52, 20, 6);\
}\
\
static void OPNAME ## rv40_qpel ## SIZE ## _mc21_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride)\
{\
    uint8_t full[SIZE*(SIZE+5)];\
    uint8_t * const full_mid = full + SIZE*2;\
    put_rv40_qpel ## SIZE ## _h_lowpass(full, src - 2*stride, SIZE, stride, SIZE+5, 20, 20, 5);\
    OPNAME ## rv40_qpel ## SIZE ## _v_lowpass(dst, full_mid, stride, SIZE, SIZE, 52, 20, 6);\
}\
\
static void OPNAME ## rv40_qpel ## SIZE ## _mc31_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride)\
{\
    uint8_t full[SIZE*(SIZE+5)];\
    uint8_t * const full_mid = full + SIZE*2;\
    put_rv40_qpel ## SIZE ## _h_lowpass(full, src - 2*stride, SIZE, stride, SIZE+5, 20, 52, 6);\
    OPNAME ## rv40_qpel ## SIZE ## _v_lowpass(dst, full_mid, stride, SIZE, SIZE, 52, 20, 6);\
}\
\
static void OPNAME ## rv40_qpel ## SIZE ## _mc12_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride)\
{\
    uint8_t full[SIZE*(SIZE+5)];\
    uint8_t * const full_mid = full + SIZE*2;\
    put_rv40_qpel ## SIZE ## _h_lowpass(full, src - 2*stride, SIZE, stride, SIZE+5, 52, 20, 6);\
    OPNAME ## rv40_qpel ## SIZE ## _v_lowpass(dst, full_mid, stride, SIZE, SIZE, 20, 20, 5);\
}\
\
static void OPNAME ## rv40_qpel ## SIZE ## _mc22_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride)\
{\
    uint8_t full[SIZE*(SIZE+5)];\
    uint8_t * const full_mid = full + SIZE*2;\
    put_rv40_qpel ## SIZE ## _h_lowpass(full, src - 2*stride, SIZE, stride, SIZE+5, 20, 20, 5);\
    OPNAME ## rv40_qpel ## SIZE ## _v_lowpass(dst, full_mid, stride, SIZE, SIZE, 20, 20, 5);\
}\
\
static void OPNAME ## rv40_qpel ## SIZE ## _mc32_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride)\
{\
    uint8_t full[SIZE*(SIZE+5)];\
    uint8_t * const full_mid = full + SIZE*2;\
    put_rv40_qpel ## SIZE ## _h_lowpass(full, src - 2*stride, SIZE, stride, SIZE+5, 20, 52, 6);\
    OPNAME ## rv40_qpel ## SIZE ## _v_lowpass(dst, full_mid, stride, SIZE, SIZE, 20, 20, 5);\
}\
\
static void OPNAME ## rv40_qpel ## SIZE ## _mc03_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride)\
{\
    OPNAME ## rv40_qpel ## SIZE ## _v_lowpass(dst, src, stride, stride, SIZE, 20, 52, 6);\
}\
\
static void OPNAME ## rv40_qpel ## SIZE ## _mc13_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride)\
{\
    uint8_t full[SIZE*(SIZE+5)];\
    uint8_t * const full_mid = full + SIZE*2;\
    put_rv40_qpel ## SIZE ## _h_lowpass(full, src - 2*stride, SIZE, stride, SIZE+5, 52, 20, 6);\
    OPNAME ## rv40_qpel ## SIZE ## _v_lowpass(dst, full_mid, stride, SIZE, SIZE, 20, 52, 6);\
}\
\
static void OPNAME ## rv40_qpel ## SIZE ## _mc23_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride)\
{\
    uint8_t full[SIZE*(SIZE+5)];\
    uint8_t * const full_mid = full + SIZE*2;\
    put_rv40_qpel ## SIZE ## _h_lowpass(full, src - 2*stride, SIZE, stride, SIZE+5, 20, 20, 5);\
    OPNAME ## rv40_qpel ## SIZE ## _v_lowpass(dst, full_mid, stride, SIZE, SIZE, 20, 52, 6);\
}\
\

#define op_avg(a, b)  a = (((a)+cm[b]+1)>>1)
#define op_put(a, b)  a = cm[b]

RV40_LOWPASS(put_       , op_put)
RV40_LOWPASS(avg_       , op_avg)

#undef op_avg
#undef op_put

RV40_MC(put_, 8)
RV40_MC(put_, 16)
RV40_MC(avg_, 8)
RV40_MC(avg_, 16)

#define PIXOP2(OPNAME, OP)                                              \
static inline void OPNAME ## _pixels8_xy2_8_c(uint8_t *block,           \
                                              const uint8_t *pixels,    \
                                              ptrdiff_t line_size,      \
                                              int h)                    \
{                                                                       \
    /* FIXME HIGH BIT DEPTH */                                          \
    int j;                                                              \
                                                                        \
    for (j = 0; j < 2; j++) {                                           \
        int i;                                                          \
        const uint32_t a = AV_RN32(pixels);                             \
        const uint32_t b = AV_RN32(pixels + 1);                         \
        uint32_t l0 = (a & 0x03030303UL) +                              \
                      (b & 0x03030303UL) +                              \
                           0x02020202UL;                                \
        uint32_t h0 = ((a & 0xFCFCFCFCUL) >> 2) +                       \
                      ((b & 0xFCFCFCFCUL) >> 2);                        \
        uint32_t l1, h1;                                                \
                                                                        \
        pixels += line_size;                                            \
        for (i = 0; i < h; i += 2) {                                    \
            uint32_t a = AV_RN32(pixels);                               \
            uint32_t b = AV_RN32(pixels + 1);                           \
            l1 = (a & 0x03030303UL) +                                   \
                 (b & 0x03030303UL);                                    \
            h1 = ((a & 0xFCFCFCFCUL) >> 2) +                            \
                 ((b & 0xFCFCFCFCUL) >> 2);                             \
            OP(*((uint32_t *) block),                                   \
               h0 + h1 + (((l0 + l1) >> 2) & 0x0F0F0F0FUL));            \
            pixels += line_size;                                        \
            block  += line_size;                                        \
            a = AV_RN32(pixels);                                        \
            b = AV_RN32(pixels + 1);                                    \
            l0 = (a & 0x03030303UL) +                                   \
                 (b & 0x03030303UL) +                                   \
                      0x02020202UL;                                     \
            h0 = ((a & 0xFCFCFCFCUL) >> 2) +                            \
                 ((b & 0xFCFCFCFCUL) >> 2);                             \
            OP(*((uint32_t *) block),                                   \
               h0 + h1 + (((l0 + l1) >> 2) & 0x0F0F0F0FUL));            \
            pixels += line_size;                                        \
            block  += line_size;                                        \
        }                                                               \
        pixels += 4 - line_size * (h + 1);                              \
        block  += 4 - line_size * h;                                    \
    }                                                                   \
}                                                                       \
                                                                        \
CALL_2X_PIXELS(OPNAME ## _pixels16_xy2_8_c,                             \
               OPNAME ## _pixels8_xy2_8_c,                              \
               8)                                                       \

#define op_avg(a, b) a = rnd_avg32(a, b)
#define op_put(a, b) a = b
PIXOP2(avg, op_avg)
PIXOP2(put, op_put)
#undef op_avg
#undef op_put

static void put_rv40_qpel16_mc33_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride)
{
    put_pixels16_xy2_8_c(dst, src, stride, 16);
}
static void avg_rv40_qpel16_mc33_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride)
{
    avg_pixels16_xy2_8_c(dst, src, stride, 16);
}
static void put_rv40_qpel8_mc33_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride)
{
    put_pixels8_xy2_8_c(dst, src, stride, 8);
}
static void avg_rv40_qpel8_mc33_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride)
{
    avg_pixels8_xy2_8_c(dst, src, stride, 8);
}

static const int rv40_bias[4][4] = {
    {  0, 16, 32, 16 },
    { 32, 28, 32, 28 },
    {  0, 32, 16, 32 },
    { 32, 28, 32, 28 }
};

#define RV40_CHROMA_MC(OPNAME, OP)\
static void OPNAME ## rv40_chroma_mc4_c(uint8_t *dst /*align 8*/,\
                                        uint8_t *src /*align 1*/,\
                                        ptrdiff_t stride, int h, int x, int y)\
{\
    const int A = (8-x) * (8-y);\
    const int B = (  x) * (8-y);\
    const int C = (8-x) * (  y);\
    const int D = (  x) * (  y);\
    int i;\
    int bias = rv40_bias[y>>1][x>>1];\
    \
    av_assert2(x<8 && y<8 && x>=0 && y>=0);\
\
    if(D){\
        for(i = 0; i < h; i++){\
            OP(dst[0], (A*src[0] + B*src[1] + C*src[stride+0] + D*src[stride+1] + bias));\
            OP(dst[1], (A*src[1] + B*src[2] + C*src[stride+1] + D*src[stride+2] + bias));\
            OP(dst[2], (A*src[2] + B*src[3] + C*src[stride+2] + D*src[stride+3] + bias));\
            OP(dst[3], (A*src[3] + B*src[4] + C*src[stride+3] + D*src[stride+4] + bias));\
            dst += stride;\
            src += stride;\
        }\
    }else{\
        const int E = B + C;\
        const ptrdiff_t step = C ? stride : 1;\
        for(i = 0; i < h; i++){\
            OP(dst[0], (A*src[0] + E*src[step+0] + bias));\
            OP(dst[1], (A*src[1] + E*src[step+1] + bias));\
            OP(dst[2], (A*src[2] + E*src[step+2] + bias));\
            OP(dst[3], (A*src[3] + E*src[step+3] + bias));\
            dst += stride;\
            src += stride;\
        }\
    }\
}\
\
static void OPNAME ## rv40_chroma_mc8_c(uint8_t *dst/*align 8*/,\
                                        uint8_t *src/*align 1*/,\
                                        ptrdiff_t stride, int h, int x, int y)\
{\
    const int A = (8-x) * (8-y);\
    const int B = (  x) * (8-y);\
    const int C = (8-x) * (  y);\
    const int D = (  x) * (  y);\
    int i;\
    int bias = rv40_bias[y>>1][x>>1];\
    \
    av_assert2(x<8 && y<8 && x>=0 && y>=0);\
\
    if(D){\
        for(i = 0; i < h; i++){\
            OP(dst[0], (A*src[0] + B*src[1] + C*src[stride+0] + D*src[stride+1] + bias));\
            OP(dst[1], (A*src[1] + B*src[2] + C*src[stride+1] + D*src[stride+2] + bias));\
            OP(dst[2], (A*src[2] + B*src[3] + C*src[stride+2] + D*src[stride+3] + bias));\
            OP(dst[3], (A*src[3] + B*src[4] + C*src[stride+3] + D*src[stride+4] + bias));\
            OP(dst[4], (A*src[4] + B*src[5] + C*src[stride+4] + D*src[stride+5] + bias));\
            OP(dst[5], (A*src[5] + B*src[6] + C*src[stride+5] + D*src[stride+6] + bias));\
            OP(dst[6], (A*src[6] + B*src[7] + C*src[stride+6] + D*src[stride+7] + bias));\
            OP(dst[7], (A*src[7] + B*src[8] + C*src[stride+7] + D*src[stride+8] + bias));\
            dst += stride;\
            src += stride;\
        }\
    }else{\
        const int E = B + C;\
        const ptrdiff_t step = C ? stride : 1;\
        for(i = 0; i < h; i++){\
            OP(dst[0], (A*src[0] + E*src[step+0] + bias));\
            OP(dst[1], (A*src[1] + E*src[step+1] + bias));\
            OP(dst[2], (A*src[2] + E*src[step+2] + bias));\
            OP(dst[3], (A*src[3] + E*src[step+3] + bias));\
            OP(dst[4], (A*src[4] + E*src[step+4] + bias));\
            OP(dst[5], (A*src[5] + E*src[step+5] + bias));\
            OP(dst[6], (A*src[6] + E*src[step+6] + bias));\
            OP(dst[7], (A*src[7] + E*src[step+7] + bias));\
            dst += stride;\
            src += stride;\
        }\
    }\
}

#define op_avg(a, b) a = (((a)+((b)>>6)+1)>>1)
#define op_put(a, b) a = ((b)>>6)

RV40_CHROMA_MC(put_, op_put)
RV40_CHROMA_MC(avg_, op_avg)

#define RV40_WEIGHT_FUNC(size) \
static void rv40_weight_func_rnd_ ## size (uint8_t *dst, uint8_t *src1, uint8_t *src2, int w1, int w2, ptrdiff_t stride)\
{\
    int i, j;\
\
    for (j = 0; j < size; j++) {\
        for (i = 0; i < size; i++)\
            dst[i] = ((((unsigned)w2 * src1[i]) >> 9) + (((unsigned)w1 * src2[i]) >> 9) + 0x10) >> 5;\
        src1 += stride;\
        src2 += stride;\
        dst  += stride;\
    }\
}\
static void rv40_weight_func_nornd_ ## size (uint8_t *dst, uint8_t *src1, uint8_t *src2, int w1, int w2, ptrdiff_t stride)\
{\
    int i, j;\
\
    for (j = 0; j < size; j++) {\
        for (i = 0; i < size; i++)\
            dst[i] = ((unsigned)w2 * src1[i] + (unsigned)w1 * src2[i] + 0x10) >> 5;\
        src1 += stride;\
        src2 += stride;\
        dst  += stride;\
    }\
}

RV40_WEIGHT_FUNC(16)
RV40_WEIGHT_FUNC(8)

/**
 * dither values for deblocking filter - left/top values
 */
static const uint8_t rv40_dither_l[16] = {
    0x40, 0x50, 0x20, 0x60, 0x30, 0x50, 0x40, 0x30,
    0x50, 0x40, 0x50, 0x30, 0x60, 0x20, 0x50, 0x40
};

/**
 * dither values for deblocking filter - right/bottom values
 */
static const uint8_t rv40_dither_r[16] = {
    0x40, 0x30, 0x60, 0x20, 0x50, 0x30, 0x30, 0x40,
    0x40, 0x40, 0x50, 0x30, 0x20, 0x60, 0x30, 0x40
};

#define CLIP_SYMM(a, b) av_clip(a, -(b), b)
/**
 * weaker deblocking very similar to the one described in 4.4.2 of JVT-A003r1
 */
                                                   const int step,
                                                   const ptrdiff_t stride,
                                                   const int filter_p1,
                                                   const int filter_q1,
                                                   const int alpha,
                                                   const int beta,
                                                   const int lim_p0q0,
                                                   const int lim_q1,
                                                   const int lim_p1)
{






        }

        }
    }
}

                                    const int filter_p1, const int filter_q1,
                                    const int alpha, const int beta,
                                    const int lim_p0q0, const int lim_q1,
                                    const int lim_p1)
{
                          alpha, beta, lim_p0q0, lim_q1, lim_p1);

                                    const int filter_p1, const int filter_q1,
                                    const int alpha, const int beta,
                                    const int lim_p0q0, const int lim_q1,
                                    const int lim_p1)
{
                          alpha, beta, lim_p0q0, lim_q1, lim_p1);

                                                     const int step,
                                                     const ptrdiff_t stride,
                                                     const int alpha,
                                                     const int lims,
                                                     const int dmode,
                                                     const int chroma)
{






        }


        }


        }
    }
}

                                      const int alpha, const int lims,
                                      const int dmode, const int chroma)
{

                                      const int alpha, const int lims,
                                      const int dmode, const int chroma)
{

                                                      int step, ptrdiff_t stride,
                                                      int beta, int beta2,
                                                      int edge,
                                                      int *p1, int *q1)
{

    }


        return 0;

        return 0;

    }


}

                                       int beta, int beta2, int edge,
                                       int *p1, int *q1)
{
}

                                       int beta, int beta2, int edge,
                                       int *p1, int *q1)
{
}

{






        ff_rv40dsp_init_aarch64(c);
        ff_rv40dsp_init_arm(c);
