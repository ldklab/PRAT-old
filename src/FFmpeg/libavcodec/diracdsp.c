/*
 * Copyright (C) 2009 David Conrad
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

#include "avcodec.h"
#include "diracdsp.h"

#define FILTER(src, stride)                                     \
    ((21*((src)[ 0*stride] + (src)[1*stride])                   \
      -7*((src)[-1*stride] + (src)[2*stride])                   \
      +3*((src)[-2*stride] + (src)[3*stride])                   \
      -1*((src)[-3*stride] + (src)[4*stride]) + 16) >> 5)

static void dirac_hpel_filter(uint8_t *dsth, uint8_t *dstv, uint8_t *dstc, const uint8_t *src,
                              int stride, int width, int height)
{
    int x, y;

    for (y = 0; y < height; y++) {
        for (x = -3; x < width+5; x++)
            dstv[x] = av_clip_uint8(FILTER(src+x, stride));

        for (x = 0; x < width; x++)
            dstc[x] = av_clip_uint8(FILTER(dstv+x, 1));

        for (x = 0; x < width; x++)
            dsth[x] = av_clip_uint8(FILTER(src+x, 1));

        src  += stride;
        dsth += stride;
        dstv += stride;
        dstc += stride;
    }
}

#define PIXOP_BILINEAR(PFX, OP, WIDTH)                                  \
    static void ff_ ## PFX ## _dirac_pixels ## WIDTH ## _bilinear_c(uint8_t *dst, const uint8_t *src[5], int stride, int h) \
    {                                                                   \
        int x;                                                          \
        const uint8_t *s0 = src[0];                                     \
        const uint8_t *s1 = src[1];                                     \
        const uint8_t *s2 = src[2];                                     \
        const uint8_t *s3 = src[3];                                     \
        const uint8_t *w  = src[4];                                     \
                                                                        \
        while (h--) {                                                   \
            for (x = 0; x < WIDTH; x++) {                               \
                OP(dst[x], (s0[x]*w[0] + s1[x]*w[1] + s2[x]*w[2] + s3[x]*w[3] + 8) >> 4); \
            }                                                           \
                                                                        \
            dst += stride;                                              \
            s0 += stride;                                               \
            s1 += stride;                                               \
            s2 += stride;                                               \
            s3 += stride;                                               \
        }                                                               \
    }

#define OP_PUT(dst, val) (dst) = (val)
#define OP_AVG(dst, val) (dst) = (((dst) + (val) + 1)>>1)

PIXOP_BILINEAR(put, OP_PUT, 8)
PIXOP_BILINEAR(put, OP_PUT, 16)
PIXOP_BILINEAR(put, OP_PUT, 32)
PIXOP_BILINEAR(avg, OP_AVG, 8)
PIXOP_BILINEAR(avg, OP_AVG, 16)
PIXOP_BILINEAR(avg, OP_AVG, 32)

#define op_scale1(x)  block[x] = av_clip_uint8( (block[x]*weight + (1<<(log2_denom-1))) >> log2_denom)
#define op_scale2(x)  dst[x] = av_clip_uint8( (src[x]*weights + dst[x]*weightd + (1<<(log2_denom-1))) >> log2_denom)

#define DIRAC_WEIGHT(W)                                                 \
    static void weight_dirac_pixels ## W ## _c(uint8_t *block, int stride, int log2_denom, \
                                               int weight, int h) {     \
        int x;                                                          \
        while (h--) {                                                   \
            for (x = 0; x < W; x++) {                                   \
                op_scale1(x);                                           \
                op_scale1(x+1);                                         \
            }                                                           \
            block += stride;                                            \
        }                                                               \
    }                                                                   \
    static void biweight_dirac_pixels ## W ## _c(uint8_t *dst, const uint8_t *src, int stride, int log2_denom, \
                                                 int weightd, int weights, int h) { \
        int x;                                                          \
        while (h--) {                                                   \
            for (x = 0; x < W; x++) {                                   \
                op_scale2(x);                                           \
                op_scale2(x+1);                                         \
            }                                                           \
            dst += stride;                                              \
            src += stride;                                              \
        }                                                               \
    }

DIRAC_WEIGHT(8)
DIRAC_WEIGHT(16)
DIRAC_WEIGHT(32)

#define ADD_OBMC(xblen)                                                 \
    static void add_obmc ## xblen ## _c(uint16_t *dst, const uint8_t *src, int stride, \
                                        const uint8_t *obmc_weight, int yblen) \
    {                                                                   \
        int x;                                                          \
        while (yblen--) {                                               \
            for (x = 0; x < xblen; x += 2) {                            \
                dst[x  ] += src[x  ] * obmc_weight[x  ];                \
                dst[x+1] += src[x+1] * obmc_weight[x+1];                \
            }                                                           \
            dst += stride;                                              \
            src += stride;                                              \
            obmc_weight += 32;                                          \
        }                                                               \
    }

ADD_OBMC(8)
ADD_OBMC(16)
ADD_OBMC(32)

static void put_signed_rect_clamped_8bit_c(uint8_t *dst, int dst_stride, const uint8_t *_src, int src_stride, int width, int height)
{
    int x, y;
    int16_t *src = (int16_t *)_src;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x+=4) {
            dst[x  ] = av_clip_uint8(src[x  ] + 128);
            dst[x+1] = av_clip_uint8(src[x+1] + 128);
            dst[x+2] = av_clip_uint8(src[x+2] + 128);
            dst[x+3] = av_clip_uint8(src[x+3] + 128);
        }
        dst += dst_stride;
        src += src_stride >> 1;
    }
}

#define PUT_SIGNED_RECT_CLAMPED(PX)                                                                     \
static void put_signed_rect_clamped_ ## PX ## bit_c(uint8_t *_dst, int dst_stride, const uint8_t *_src, \
                                                  int src_stride, int width, int height)                \
{                                                                                                       \
    int x, y;                                                                                           \
    uint16_t *dst = (uint16_t *)_dst;                                                                   \
    int32_t *src = (int32_t *)_src;                                                                     \
    for (y = 0; y < height; y++) {                                                                      \
        for (x = 0; x < width; x+=4) {                                                                  \
            dst[x  ] = av_clip_uintp2(src[x  ] + (1U << (PX - 1)), PX);                                  \
            dst[x+1] = av_clip_uintp2(src[x+1] + (1U << (PX - 1)), PX);                                  \
            dst[x+2] = av_clip_uintp2(src[x+2] + (1U << (PX - 1)), PX);                                  \
            dst[x+3] = av_clip_uintp2(src[x+3] + (1U << (PX - 1)), PX);                                  \
        }                                                                                               \
        dst += dst_stride >> 1;                                                                         \
        src += src_stride >> 2;                                                                         \
    }                                                                                                   \
}

PUT_SIGNED_RECT_CLAMPED(10)

static void add_rect_clamped_c(uint8_t *dst, const uint16_t *src, int stride,
                               const int16_t *idwt, int idwt_stride,
                               int width, int height)
{
    int x, y;

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x+=2) {
            dst[x  ] = av_clip_uint8(((src[x  ]+32)>>6) + idwt[x  ]);
            dst[x+1] = av_clip_uint8(((src[x+1]+32)>>6) + idwt[x+1]);
        }
        dst += stride;
        src += stride;
        idwt += idwt_stride;
    }
}

#define DEQUANT_SUBBAND(PX)                                                                \
static void dequant_subband_ ## PX ## _c(uint8_t *src, uint8_t *dst, ptrdiff_t stride,     \
                                         const int qf, const int qs, int tot_v, int tot_h) \
{                                                                                          \
    int i, y;                                                                              \
    for (y = 0; y < tot_v; y++) {                                                          \
        PX c, sign, *src_r = (PX *)src, *dst_r = (PX *)dst;                                \
        for (i = 0; i < tot_h; i++) {                                                      \
            c = *src_r++;                                                                  \
            sign = FFSIGN(c)*(!!c);                                                        \
            c = (FFABS(c)*(unsigned)qf + qs) >> 2;                                                   \
            *dst_r++ = c*sign;                                                             \
        }                                                                                  \
        src += tot_h << (sizeof(PX) >> 1);                                                 \
        dst += stride;                                                                     \
    }                                                                                      \
}


#define PIXFUNC(PFX, WIDTH)                                             \
    c->PFX ## _dirac_pixels_tab[WIDTH>>4][0] = ff_ ## PFX ## _dirac_pixels ## WIDTH ## _c; \
    c->PFX ## _dirac_pixels_tab[WIDTH>>4][1] = ff_ ## PFX ## _dirac_pixels ## WIDTH ## _l2_c; \
    c->PFX ## _dirac_pixels_tab[WIDTH>>4][2] = ff_ ## PFX ## _dirac_pixels ## WIDTH ## _l4_c; \
    c->PFX ## _dirac_pixels_tab[WIDTH>>4][3] = ff_ ## PFX ## _dirac_pixels ## WIDTH ## _bilinear_c

{





