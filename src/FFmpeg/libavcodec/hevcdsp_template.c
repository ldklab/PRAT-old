/*
 * HEVC video decoder
 *
 * Copyright (C) 2012 - 2013 Guillaume Martres
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

#include "get_bits.h"
#include "hevcdec.h"

#include "bit_depth_template.c"
#include "hevcdsp.h"

                          GetBitContext *gb, int pcm_bit_depth)
{


    }

                                                ptrdiff_t stride, int size)
{


        }
    }
}

                                  ptrdiff_t stride)
{

                                  ptrdiff_t stride)
{

                                    ptrdiff_t stride)
{

                                    ptrdiff_t stride)
{

{

        }
    } else {
        }
    }

{

            }
        }
    } else {
            }
        }
    }

#define SET(dst, x)   (dst) = (x)
#define SCALE(dst, x) (dst) = av_clip_int16(((x) + add) >> shift)

#define TR_4x4_LUMA(dst, src, step, assign)                             \
    do {                                                                \
        int c0 = src[0 * step] + src[2 * step];                         \
        int c1 = src[2 * step] + src[3 * step];                         \
        int c2 = src[0 * step] - src[3 * step];                         \
        int c3 = 74 * src[1 * step];                                    \
                                                                        \
        assign(dst[2 * step], 74 * (src[0 * step] -                     \
                                    src[2 * step] +                     \
                                    src[3 * step]));                    \
        assign(dst[0 * step], 29 * c0 + 55 * c1 + c3);                  \
        assign(dst[1 * step], 55 * c2 - 29 * c1 + c3);                  \
        assign(dst[3 * step], 55 * c0 + 29 * c2 - c3);                  \
    } while (0)

{

    }

    }

#undef TR_4x4_LUMA

#define TR_4(dst, src, dstep, sstep, assign, end)                 \
    do {                                                          \
        const int e0 = 64 * src[0 * sstep] + 64 * src[2 * sstep]; \
        const int e1 = 64 * src[0 * sstep] - 64 * src[2 * sstep]; \
        const int o0 = 83 * src[1 * sstep] + 36 * src[3 * sstep]; \
        const int o1 = 36 * src[1 * sstep] - 83 * src[3 * sstep]; \
                                                                  \
        assign(dst[0 * dstep], e0 + o0);                          \
        assign(dst[1 * dstep], e1 + o1);                          \
        assign(dst[2 * dstep], e1 - o1);                          \
        assign(dst[3 * dstep], e0 - o0);                          \
    } while (0)

#define TR_8(dst, src, dstep, sstep, assign, end)                 \
    do {                                                          \
        int i, j;                                                 \
        int e_8[4];                                               \
        int o_8[4] = { 0 };                                       \
        for (i = 0; i < 4; i++)                                   \
            for (j = 1; j < end; j += 2)                          \
                o_8[i] += transform[4 * j][i] * src[j * sstep];   \
        TR_4(e_8, src, 1, 2 * sstep, SET, 4);                     \
                                                                  \
        for (i = 0; i < 4; i++) {                                 \
            assign(dst[i * dstep], e_8[i] + o_8[i]);              \
            assign(dst[(7 - i) * dstep], e_8[i] - o_8[i]);        \
        }                                                         \
    } while (0)

#define TR_16(dst, src, dstep, sstep, assign, end)                \
    do {                                                          \
        int i, j;                                                 \
        int e_16[8];                                              \
        int o_16[8] = { 0 };                                      \
        for (i = 0; i < 8; i++)                                   \
            for (j = 1; j < end; j += 2)                          \
                o_16[i] += transform[2 * j][i] * src[j * sstep];  \
        TR_8(e_16, src, 1, 2 * sstep, SET, 8);                    \
                                                                  \
        for (i = 0; i < 8; i++) {                                 \
            assign(dst[i * dstep], e_16[i] + o_16[i]);            \
            assign(dst[(15 - i) * dstep], e_16[i] - o_16[i]);     \
        }                                                         \
    } while (0)

#define TR_32(dst, src, dstep, sstep, assign, end)                \
    do {                                                          \
        int i, j;                                                 \
        int e_32[16];                                             \
        int o_32[16] = { 0 };                                     \
        for (i = 0; i < 16; i++)                                  \
            for (j = 1; j < end; j += 2)                          \
                o_32[i] += transform[j][i] * src[j * sstep];      \
        TR_16(e_32, src, 1, 2 * sstep, SET, end / 2);             \
                                                                  \
        for (i = 0; i < 16; i++) {                                \
            assign(dst[i * dstep], e_32[i] + o_32[i]);            \
            assign(dst[(31 - i) * dstep], e_32[i] - o_32[i]);     \
        }                                                         \
    } while (0)

#define IDCT_VAR4(H)                                              \
    int limit2 = FFMIN(col_limit + 4, H)
#define IDCT_VAR8(H)                                              \
    int limit  = FFMIN(col_limit, H);                             \
    int limit2 = FFMIN(col_limit + 4, H)
#define IDCT_VAR16(H)   IDCT_VAR8(H)
#define IDCT_VAR32(H)   IDCT_VAR8(H)

#define IDCT(H)                                                   \
static void FUNC(idct_ ## H ## x ## H )(int16_t *coeffs,          \
                                        int col_limit)            \
{                                                                 \
    int i;                                                        \
    int      shift = 7;                                           \
    int      add   = 1 << (shift - 1);                            \
    int16_t *src   = coeffs;                                      \
    IDCT_VAR ## H(H);                                             \
                                                                  \
    for (i = 0; i < H; i++) {                                     \
        TR_ ## H(src, src, H, H, SCALE, limit2);                  \
        if (limit2 < H && i%4 == 0 && !!i)                        \
            limit2 -= 4;                                          \
        src++;                                                    \
    }                                                             \
                                                                  \
    shift = 20 - BIT_DEPTH;                                       \
    add   = 1 << (shift - 1);                                     \
    for (i = 0; i < H; i++) {                                     \
        TR_ ## H(coeffs, coeffs, 1, 1, SCALE, limit);             \
        coeffs += H;                                              \
    }                                                             \
}

#define IDCT_DC(H)                                                \
static void FUNC(idct_ ## H ## x ## H ## _dc)(int16_t *coeffs)    \
{                                                                 \
    int i, j;                                                     \
    int shift = 14 - BIT_DEPTH;                                   \
    int add   = 1 << (shift - 1);                                 \
    int coeff = (((coeffs[0] + 1) >> 1) + add) >> shift;          \
                                                                  \
    for (j = 0; j < H; j++) {                                     \
        for (i = 0; i < H; i++) {                                 \
            coeffs[i + j * H] = coeff;                            \
        }                                                         \
    }                                                             \
}



#undef TR_4
#undef TR_8
#undef TR_16
#undef TR_32

#undef SET
#undef SCALE

                                  ptrdiff_t stride_dst, ptrdiff_t stride_src,
                                  int16_t *sao_offset_val, int sao_left_class,
                                  int width, int height)
{


    }

#define CMP(a, b) (((a) > (b)) - ((a) < (b)))

                                  int eo, int width, int height) {

        { { -1,  0 }, {  1, 0 } }, // horizontal
        { {  0, -1 }, {  0, 1 } }, // vertical
        { { -1, -1 }, {  1, 1 } }, // 45 degree
        { {  1, -1 }, { -1, 1 } }, // 135 degree
    };

        }
    }

                                    ptrdiff_t stride_dst, ptrdiff_t stride_src, SAOParams *sao,
                                    int *borders, int _width, int _height,
                                    int c_idx, uint8_t *vert_edge,
                                    uint8_t *horiz_edge, uint8_t *diag_edge)
{


            }
            init_x = 1;
        }
            }
            width--;
        }
    }
        }
        }
    }

                                    ptrdiff_t stride_dst, ptrdiff_t stride_src, SAOParams *sao,
                                    int *borders, int _width, int _height,
                                    int c_idx, uint8_t *vert_edge,
                                    uint8_t *horiz_edge, uint8_t *diag_edge)
{


            }
            init_x = 1;
        }
            }
            width--;
        }
    }
            init_y = 1;
        }
            height--;
        }
    }

    {

        // Restore pixels that can't be modified
        }
        }

        }
        }

    }

#undef CMP

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
                                      uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, intptr_t mx, intptr_t my, int width)
{

    }

                                          int height, intptr_t mx, intptr_t my, int width)
{

    }

                                         int16_t *src2,
                                         int height, intptr_t mx, intptr_t my, int width)
{

#if BIT_DEPTH < 14
#else
    int offset = 0;
#endif

    }

                                            int height, int denom, int wx, int ox, intptr_t mx, intptr_t my, int width)
{
#if BIT_DEPTH < 14
#else
    int offset = 0;
#endif

    }

                                           int16_t *src2,
                                           int height, int denom, int wx0, int wx1,
                                           int ox0, int ox1, intptr_t mx, intptr_t my, int width)
{


        }
    }

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
#define QPEL_FILTER(src, stride)                                               \
    (filter[0] * src[x - 3 * stride] +                                         \
     filter[1] * src[x - 2 * stride] +                                         \
     filter[2] * src[x -     stride] +                                         \
     filter[3] * src[x             ] +                                         \
     filter[4] * src[x +     stride] +                                         \
     filter[5] * src[x + 2 * stride] +                                         \
     filter[6] * src[x + 3 * stride] +                                         \
     filter[7] * src[x + 4 * stride])

static void FUNC(put_hevc_qpel_h)(int16_t *dst,
                                  uint8_t *_src, ptrdiff_t _srcstride,
                                  int height, intptr_t mx, intptr_t my, int width)
{
    int x, y;
    pixel        *src       = (pixel*)_src;
    ptrdiff_t     srcstride = _srcstride / sizeof(pixel);
    const int8_t *filter    = ff_hevc_qpel_filters[mx - 1];
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++)
            dst[x] = QPEL_FILTER(src, 1) >> (BIT_DEPTH - 8);
        src += srcstride;
        dst += MAX_PB_SIZE;
    }
}

static void FUNC(put_hevc_qpel_v)(int16_t *dst,
                                  uint8_t *_src, ptrdiff_t _srcstride,
                                  int height, intptr_t mx, intptr_t my, int width)
{
    int x, y;
    pixel        *src       = (pixel*)_src;
    ptrdiff_t     srcstride = _srcstride / sizeof(pixel);
    const int8_t *filter    = ff_hevc_qpel_filters[my - 1];
    for (y = 0; y < height; y++)  {
        for (x = 0; x < width; x++)
            dst[x] = QPEL_FILTER(src, srcstride) >> (BIT_DEPTH - 8);
        src += srcstride;
        dst += MAX_PB_SIZE;
    }
}

static void FUNC(put_hevc_qpel_hv)(int16_t *dst,
                                   uint8_t *_src,
                                   ptrdiff_t _srcstride,
                                   int height, intptr_t mx,
                                   intptr_t my, int width)
{
    int x, y;
    const int8_t *filter;
    pixel *src = (pixel*)_src;
    ptrdiff_t srcstride = _srcstride / sizeof(pixel);
    int16_t tmp_array[(MAX_PB_SIZE + QPEL_EXTRA) * MAX_PB_SIZE];
    int16_t *tmp = tmp_array;

    src   -= QPEL_EXTRA_BEFORE * srcstride;
    filter = ff_hevc_qpel_filters[mx - 1];
    for (y = 0; y < height + QPEL_EXTRA; y++) {
        for (x = 0; x < width; x++)
            tmp[x] = QPEL_FILTER(src, 1) >> (BIT_DEPTH - 8);
        src += srcstride;
        tmp += MAX_PB_SIZE;
    }

    tmp    = tmp_array + QPEL_EXTRA_BEFORE * MAX_PB_SIZE;
    filter = ff_hevc_qpel_filters[my - 1];
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++)
            dst[x] = QPEL_FILTER(tmp, MAX_PB_SIZE) >> 6;
        tmp += MAX_PB_SIZE;
        dst += MAX_PB_SIZE;
    }
}

static void FUNC(put_hevc_qpel_uni_h)(uint8_t *_dst,  ptrdiff_t _dststride,
                                      uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, intptr_t mx, intptr_t my, int width)
{
    int x, y;
    pixel        *src       = (pixel*)_src;
    ptrdiff_t     srcstride = _srcstride / sizeof(pixel);
    pixel *dst          = (pixel *)_dst;
    ptrdiff_t dststride = _dststride / sizeof(pixel);
    const int8_t *filter    = ff_hevc_qpel_filters[mx - 1];
    int shift = 14 - BIT_DEPTH;

#if BIT_DEPTH < 14
    int offset = 1 << (shift - 1);
#else
    int offset = 0;
#endif

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++)
            dst[x] = av_clip_pixel(((QPEL_FILTER(src, 1) >> (BIT_DEPTH - 8)) + offset) >> shift);
        src += srcstride;
        dst += dststride;
    }
}

static void FUNC(put_hevc_qpel_bi_h)(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                                     int16_t *src2,
                                     int height, intptr_t mx, intptr_t my, int width)
{
    int x, y;
    pixel        *src       = (pixel*)_src;
    ptrdiff_t     srcstride = _srcstride / sizeof(pixel);
    pixel *dst          = (pixel *)_dst;
    ptrdiff_t dststride = _dststride / sizeof(pixel);

    const int8_t *filter    = ff_hevc_qpel_filters[mx - 1];

    int shift = 14  + 1 - BIT_DEPTH;
#if BIT_DEPTH < 14
    int offset = 1 << (shift - 1);
#else
    int offset = 0;
#endif

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++)
            dst[x] = av_clip_pixel(((QPEL_FILTER(src, 1) >> (BIT_DEPTH - 8)) + src2[x] + offset) >> shift);
        src  += srcstride;
        dst  += dststride;
        src2 += MAX_PB_SIZE;
    }
}

static void FUNC(put_hevc_qpel_uni_v)(uint8_t *_dst,  ptrdiff_t _dststride,
                                     uint8_t *_src, ptrdiff_t _srcstride,
                                     int height, intptr_t mx, intptr_t my, int width)
{
    int x, y;
    pixel        *src       = (pixel*)_src;
    ptrdiff_t     srcstride = _srcstride / sizeof(pixel);
    pixel *dst          = (pixel *)_dst;
    ptrdiff_t dststride = _dststride / sizeof(pixel);
    const int8_t *filter    = ff_hevc_qpel_filters[my - 1];
    int shift = 14 - BIT_DEPTH;

#if BIT_DEPTH < 14
    int offset = 1 << (shift - 1);
#else
    int offset = 0;
#endif

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++)
            dst[x] = av_clip_pixel(((QPEL_FILTER(src, srcstride) >> (BIT_DEPTH - 8)) + offset) >> shift);
        src += srcstride;
        dst += dststride;
    }
}


static void FUNC(put_hevc_qpel_bi_v)(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                                     int16_t *src2,
                                     int height, intptr_t mx, intptr_t my, int width)
{
    int x, y;
    pixel        *src       = (pixel*)_src;
    ptrdiff_t     srcstride = _srcstride / sizeof(pixel);
    pixel *dst          = (pixel *)_dst;
    ptrdiff_t dststride = _dststride / sizeof(pixel);

    const int8_t *filter    = ff_hevc_qpel_filters[my - 1];

    int shift = 14 + 1 - BIT_DEPTH;
#if BIT_DEPTH < 14
    int offset = 1 << (shift - 1);
#else
    int offset = 0;
#endif

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++)
            dst[x] = av_clip_pixel(((QPEL_FILTER(src, srcstride) >> (BIT_DEPTH - 8)) + src2[x] + offset) >> shift);
        src  += srcstride;
        dst  += dststride;
        src2 += MAX_PB_SIZE;
    }
}

static void FUNC(put_hevc_qpel_uni_hv)(uint8_t *_dst,  ptrdiff_t _dststride,
                                       uint8_t *_src, ptrdiff_t _srcstride,
                                       int height, intptr_t mx, intptr_t my, int width)
{
    int x, y;
    const int8_t *filter;
    pixel *src = (pixel*)_src;
    ptrdiff_t srcstride = _srcstride / sizeof(pixel);
    pixel *dst          = (pixel *)_dst;
    ptrdiff_t dststride = _dststride / sizeof(pixel);
    int16_t tmp_array[(MAX_PB_SIZE + QPEL_EXTRA) * MAX_PB_SIZE];
    int16_t *tmp = tmp_array;
    int shift =  14 - BIT_DEPTH;

#if BIT_DEPTH < 14
    int offset = 1 << (shift - 1);
#else
    int offset = 0;
#endif

    src   -= QPEL_EXTRA_BEFORE * srcstride;
    filter = ff_hevc_qpel_filters[mx - 1];
    for (y = 0; y < height + QPEL_EXTRA; y++) {
        for (x = 0; x < width; x++)
            tmp[x] = QPEL_FILTER(src, 1) >> (BIT_DEPTH - 8);
        src += srcstride;
        tmp += MAX_PB_SIZE;
    }

    tmp    = tmp_array + QPEL_EXTRA_BEFORE * MAX_PB_SIZE;
    filter = ff_hevc_qpel_filters[my - 1];

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++)
            dst[x] = av_clip_pixel(((QPEL_FILTER(tmp, MAX_PB_SIZE) >> 6) + offset) >> shift);
        tmp += MAX_PB_SIZE;
        dst += dststride;
    }
}

static void FUNC(put_hevc_qpel_bi_hv)(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                                      int16_t *src2,
                                      int height, intptr_t mx, intptr_t my, int width)
{
    int x, y;
    const int8_t *filter;
    pixel *src = (pixel*)_src;
    ptrdiff_t srcstride = _srcstride / sizeof(pixel);
    pixel *dst          = (pixel *)_dst;
    ptrdiff_t dststride = _dststride / sizeof(pixel);
    int16_t tmp_array[(MAX_PB_SIZE + QPEL_EXTRA) * MAX_PB_SIZE];
    int16_t *tmp = tmp_array;
    int shift = 14 + 1 - BIT_DEPTH;
#if BIT_DEPTH < 14
    int offset = 1 << (shift - 1);
#else
    int offset = 0;
#endif

    src   -= QPEL_EXTRA_BEFORE * srcstride;
    filter = ff_hevc_qpel_filters[mx - 1];
    for (y = 0; y < height + QPEL_EXTRA; y++) {
        for (x = 0; x < width; x++)
            tmp[x] = QPEL_FILTER(src, 1) >> (BIT_DEPTH - 8);
        src += srcstride;
        tmp += MAX_PB_SIZE;
    }

    tmp    = tmp_array + QPEL_EXTRA_BEFORE * MAX_PB_SIZE;
    filter = ff_hevc_qpel_filters[my - 1];

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++)
            dst[x] = av_clip_pixel(((QPEL_FILTER(tmp, MAX_PB_SIZE) >> 6) + src2[x] + offset) >> shift);
        tmp  += MAX_PB_SIZE;
        dst  += dststride;
        src2 += MAX_PB_SIZE;
    }
}

static void FUNC(put_hevc_qpel_uni_w_h)(uint8_t *_dst,  ptrdiff_t _dststride,
                                        uint8_t *_src, ptrdiff_t _srcstride,
                                        int height, int denom, int wx, int ox,
                                        intptr_t mx, intptr_t my, int width)
{
    int x, y;
    pixel        *src       = (pixel*)_src;
    ptrdiff_t     srcstride = _srcstride / sizeof(pixel);
    pixel *dst          = (pixel *)_dst;
    ptrdiff_t dststride = _dststride / sizeof(pixel);
    const int8_t *filter    = ff_hevc_qpel_filters[mx - 1];
    int shift = denom + 14 - BIT_DEPTH;
#if BIT_DEPTH < 14
    int offset = 1 << (shift - 1);
#else
    int offset = 0;
#endif

    ox = ox * (1 << (BIT_DEPTH - 8));
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++)
            dst[x] = av_clip_pixel((((QPEL_FILTER(src, 1) >> (BIT_DEPTH - 8)) * wx + offset) >> shift) + ox);
        src += srcstride;
        dst += dststride;
    }
}

static void FUNC(put_hevc_qpel_bi_w_h)(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                                       int16_t *src2,
                                       int height, int denom, int wx0, int wx1,
                                       int ox0, int ox1, intptr_t mx, intptr_t my, int width)
{
    int x, y;
    pixel        *src       = (pixel*)_src;
    ptrdiff_t     srcstride = _srcstride / sizeof(pixel);
    pixel *dst          = (pixel *)_dst;
    ptrdiff_t dststride = _dststride / sizeof(pixel);

    const int8_t *filter    = ff_hevc_qpel_filters[mx - 1];

    int shift = 14  + 1 - BIT_DEPTH;
    int log2Wd = denom + shift - 1;

    ox0     = ox0 * (1 << (BIT_DEPTH - 8));
    ox1     = ox1 * (1 << (BIT_DEPTH - 8));
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++)
            dst[x] = av_clip_pixel(((QPEL_FILTER(src, 1) >> (BIT_DEPTH - 8)) * wx1 + src2[x] * wx0 +
                                    ((ox0 + ox1 + 1) * (1 << log2Wd))) >> (log2Wd + 1));
        src  += srcstride;
        dst  += dststride;
        src2 += MAX_PB_SIZE;
    }
}

static void FUNC(put_hevc_qpel_uni_w_v)(uint8_t *_dst,  ptrdiff_t _dststride,
                                        uint8_t *_src, ptrdiff_t _srcstride,
                                        int height, int denom, int wx, int ox,
                                        intptr_t mx, intptr_t my, int width)
{
    int x, y;
    pixel        *src       = (pixel*)_src;
    ptrdiff_t     srcstride = _srcstride / sizeof(pixel);
    pixel *dst          = (pixel *)_dst;
    ptrdiff_t dststride = _dststride / sizeof(pixel);
    const int8_t *filter    = ff_hevc_qpel_filters[my - 1];
    int shift = denom + 14 - BIT_DEPTH;
#if BIT_DEPTH < 14
    int offset = 1 << (shift - 1);
#else
    int offset = 0;
#endif

    ox = ox * (1 << (BIT_DEPTH - 8));
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++)
            dst[x] = av_clip_pixel((((QPEL_FILTER(src, srcstride) >> (BIT_DEPTH - 8)) * wx + offset) >> shift) + ox);
        src += srcstride;
        dst += dststride;
    }
}

static void FUNC(put_hevc_qpel_bi_w_v)(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                                       int16_t *src2,
                                       int height, int denom, int wx0, int wx1,
                                       int ox0, int ox1, intptr_t mx, intptr_t my, int width)
{
    int x, y;
    pixel        *src       = (pixel*)_src;
    ptrdiff_t     srcstride = _srcstride / sizeof(pixel);
    pixel *dst          = (pixel *)_dst;
    ptrdiff_t dststride = _dststride / sizeof(pixel);

    const int8_t *filter    = ff_hevc_qpel_filters[my - 1];

    int shift = 14 + 1 - BIT_DEPTH;
    int log2Wd = denom + shift - 1;

    ox0     = ox0 * (1 << (BIT_DEPTH - 8));
    ox1     = ox1 * (1 << (BIT_DEPTH - 8));
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++)
            dst[x] = av_clip_pixel(((QPEL_FILTER(src, srcstride) >> (BIT_DEPTH - 8)) * wx1 + src2[x] * wx0 +
                                    ((ox0 + ox1 + 1) * (1 << log2Wd))) >> (log2Wd + 1));
        src  += srcstride;
        dst  += dststride;
        src2 += MAX_PB_SIZE;
    }
}

static void FUNC(put_hevc_qpel_uni_w_hv)(uint8_t *_dst,  ptrdiff_t _dststride,
                                         uint8_t *_src, ptrdiff_t _srcstride,
                                         int height, int denom, int wx, int ox,
                                         intptr_t mx, intptr_t my, int width)
{
    int x, y;
    const int8_t *filter;
    pixel *src = (pixel*)_src;
    ptrdiff_t srcstride = _srcstride / sizeof(pixel);
    pixel *dst          = (pixel *)_dst;
    ptrdiff_t dststride = _dststride / sizeof(pixel);
    int16_t tmp_array[(MAX_PB_SIZE + QPEL_EXTRA) * MAX_PB_SIZE];
    int16_t *tmp = tmp_array;
    int shift = denom + 14 - BIT_DEPTH;
#if BIT_DEPTH < 14
    int offset = 1 << (shift - 1);
#else
    int offset = 0;
#endif

    src   -= QPEL_EXTRA_BEFORE * srcstride;
    filter = ff_hevc_qpel_filters[mx - 1];
    for (y = 0; y < height + QPEL_EXTRA; y++) {
        for (x = 0; x < width; x++)
            tmp[x] = QPEL_FILTER(src, 1) >> (BIT_DEPTH - 8);
        src += srcstride;
        tmp += MAX_PB_SIZE;
    }

    tmp    = tmp_array + QPEL_EXTRA_BEFORE * MAX_PB_SIZE;
    filter = ff_hevc_qpel_filters[my - 1];

    ox = ox * (1 << (BIT_DEPTH - 8));
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++)
            dst[x] = av_clip_pixel((((QPEL_FILTER(tmp, MAX_PB_SIZE) >> 6) * wx + offset) >> shift) + ox);
        tmp += MAX_PB_SIZE;
        dst += dststride;
    }
}

static void FUNC(put_hevc_qpel_bi_w_hv)(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                                        int16_t *src2,
                                        int height, int denom, int wx0, int wx1,
                                        int ox0, int ox1, intptr_t mx, intptr_t my, int width)
{
    int x, y;
    const int8_t *filter;
    pixel *src = (pixel*)_src;
    ptrdiff_t srcstride = _srcstride / sizeof(pixel);
    pixel *dst          = (pixel *)_dst;
    ptrdiff_t dststride = _dststride / sizeof(pixel);
    int16_t tmp_array[(MAX_PB_SIZE + QPEL_EXTRA) * MAX_PB_SIZE];
    int16_t *tmp = tmp_array;
    int shift = 14 + 1 - BIT_DEPTH;
    int log2Wd = denom + shift - 1;

    src   -= QPEL_EXTRA_BEFORE * srcstride;
    filter = ff_hevc_qpel_filters[mx - 1];
    for (y = 0; y < height + QPEL_EXTRA; y++) {
        for (x = 0; x < width; x++)
            tmp[x] = QPEL_FILTER(src, 1) >> (BIT_DEPTH - 8);
        src += srcstride;
        tmp += MAX_PB_SIZE;
    }

    tmp    = tmp_array + QPEL_EXTRA_BEFORE * MAX_PB_SIZE;
    filter = ff_hevc_qpel_filters[my - 1];

    ox0     = ox0 * (1 << (BIT_DEPTH - 8));
    ox1     = ox1 * (1 << (BIT_DEPTH - 8));
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++)
            dst[x] = av_clip_pixel(((QPEL_FILTER(tmp, MAX_PB_SIZE) >> 6) * wx1 + src2[x] * wx0 +
                                    ((ox0 + ox1 + 1) * (1 << log2Wd))) >> (log2Wd + 1));
        tmp  += MAX_PB_SIZE;
        dst  += dststride;
        src2 += MAX_PB_SIZE;
    }
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
#define EPEL_FILTER(src, stride)                                               \
    (filter[0] * src[x - stride] +                                             \
     filter[1] * src[x]          +                                             \
     filter[2] * src[x + stride] +                                             \
     filter[3] * src[x + 2 * stride])

                                  uint8_t *_src, ptrdiff_t _srcstride,
                                  int height, intptr_t mx, intptr_t my, int width)
{
    }

                                  uint8_t *_src, ptrdiff_t _srcstride,
                                  int height, intptr_t mx, intptr_t my, int width)
{

    }

                                   uint8_t *_src, ptrdiff_t _srcstride,
                                   int height, intptr_t mx, intptr_t my, int width)
{


    }


    }

                                      int height, intptr_t mx, intptr_t my, int width)
{
#if BIT_DEPTH < 14
#else
    int offset = 0;
#endif

    }

                                     int16_t *src2,
                                     int height, intptr_t mx, intptr_t my, int width)
{
#if BIT_DEPTH < 14
#else
    int offset = 0;
#endif

        }
    }

                                      int height, intptr_t mx, intptr_t my, int width)
{
#if BIT_DEPTH < 14
#else
    int offset = 0;
#endif

    }

                                     int16_t *src2,
                                     int height, intptr_t mx, intptr_t my, int width)
{
#if BIT_DEPTH < 14
#else
    int offset = 0;
#endif

    }

                                       int height, intptr_t mx, intptr_t my, int width)
{
#if BIT_DEPTH < 14
#else
    int offset = 0;
#endif


    }


    }

                                      int16_t *src2,
                                      int height, intptr_t mx, intptr_t my, int width)
{
#if BIT_DEPTH < 14
#else
    int offset = 0;
#endif


    }


    }

                                        int height, int denom, int wx, int ox, intptr_t mx, intptr_t my, int width)
{
#if BIT_DEPTH < 14
#else
    int offset = 0;
#endif

        }
    }

                                       int16_t *src2,
                                       int height, int denom, int wx0, int wx1,
                                       int ox0, int ox1, intptr_t mx, intptr_t my, int width)
{

                                    ((ox0 + ox1 + 1) * (1 << log2Wd))) >> (log2Wd + 1));
    }

                                        int height, int denom, int wx, int ox, intptr_t mx, intptr_t my, int width)
{
#if BIT_DEPTH < 14
#else
    int offset = 0;
#endif

        }
    }

                                       int16_t *src2,
                                       int height, int denom, int wx0, int wx1,
                                       int ox0, int ox1, intptr_t mx, intptr_t my, int width)
{

                                    ((ox0 + ox1 + 1) * (1 << log2Wd))) >> (log2Wd + 1));
    }

                                         int height, int denom, int wx, int ox, intptr_t mx, intptr_t my, int width)
{
#if BIT_DEPTH < 14
#else
    int offset = 0;
#endif


    }


    }

                                        int16_t *src2,
                                        int height, int denom, int wx0, int wx1,
                                        int ox0, int ox1, intptr_t mx, intptr_t my, int width)
{


    }


                                    ((ox0 + ox1 + 1) * (1 << log2Wd))) >> (log2Wd + 1));
    }

// line zero
#define P3 pix[-4 * xstride]
#define P2 pix[-3 * xstride]
#define P1 pix[-2 * xstride]
#define P0 pix[-1 * xstride]
#define Q0 pix[0 * xstride]
#define Q1 pix[1 * xstride]
#define Q2 pix[2 * xstride]
#define Q3 pix[3 * xstride]

// line three. used only for deblocking decision
#define TP3 pix[-4 * xstride + 3 * ystride]
#define TP2 pix[-3 * xstride + 3 * ystride]
#define TP1 pix[-2 * xstride + 3 * ystride]
#define TP0 pix[-1 * xstride + 3 * ystride]
#define TQ0 pix[0  * xstride + 3 * ystride]
#define TQ1 pix[1  * xstride + 3 * ystride]
#define TQ2 pix[2  * xstride + 3 * ystride]
#define TQ3 pix[3  * xstride + 3 * ystride]

                                        ptrdiff_t _xstride, ptrdiff_t _ystride,
                                        int beta, int *_tc,
                                        uint8_t *_no_p, uint8_t *_no_q)
{



        } else {

                // strong filtering
                    }
                    }
                }
            } else { // normal filtering

                        }
                        }
                    }
                }
            }
        }
    }

                                          ptrdiff_t _ystride, int *_tc,
                                          uint8_t *_no_p, uint8_t *_no_q)
{

        }

        }
    }

                                            int32_t *tc, uint8_t *no_p,
                                            uint8_t *no_q)
{

                                            int32_t *tc, uint8_t *no_p,
                                            uint8_t *no_q)
{

                                          int beta, int32_t *tc, uint8_t *no_p,
                                          uint8_t *no_q)
{
                                beta, tc, no_p, no_q);

                                          int beta, int32_t *tc, uint8_t *no_p,
                                          uint8_t *no_q)
{
                                beta, tc, no_p, no_q);

#undef P3
#undef P2
#undef P1
#undef P0
#undef Q0
#undef Q1
#undef Q2
#undef Q3

#undef TP3
#undef TP2
#undef TP1
#undef TP0
#undef TQ0
#undef TQ1
#undef TQ2
#undef TQ3
