/*
 * VP9 compatible video decoder
 *
 * Copyright (C) 2013 Ronald S. Bultje <rsbultje gmail com>
 * Copyright (C) 2013 Clément Bœsch <u pkh me>
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

#include "libavutil/common.h"
#include "bit_depth_template.c"
#include "vp9dsp.h"

#if BIT_DEPTH != 12

// FIXME see whether we can merge parts of this (perhaps at least 4x4 and 8x8)
// back with h264pred.[ch]

                       const uint8_t *left, const uint8_t *_top)
{


                       const uint8_t *left, const uint8_t *_top)
{

    }

                         const uint8_t *left, const uint8_t *_top)
{

    }

                         const uint8_t *left, const uint8_t *_top)
{

    }

                      const uint8_t *_left, const uint8_t *top)
{


                      const uint8_t *_left, const uint8_t *top)
{


    }

                        const uint8_t *_left, const uint8_t *top)
{


    }

                        const uint8_t *_left, const uint8_t *top)
{


    }

#endif /* BIT_DEPTH != 12 */

                     const uint8_t *_left, const uint8_t *_top)
{


    }

                     const uint8_t *_left, const uint8_t *_top)
{


    }

                       const uint8_t *_left, const uint8_t *_top)
{


    }

                       const uint8_t *_left, const uint8_t *_top)
{


    }

#if BIT_DEPTH != 12

                     const uint8_t *_left, const uint8_t *_top)
{
                                top[0] + top[1] + top[2] + top[3] + 4) >> 3);


                     const uint8_t *_left, const uint8_t *_top)
{
        ((left[0] + left[1] + left[2] + left[3] + left[4] + left[5] +
          left[6] + left[7] + top[0] + top[1] + top[2] + top[3] +
          top[4] + top[5] + top[6] + top[7] + 8) >> 4);

    }

                       const uint8_t *_left, const uint8_t *_top)
{
        ((left[0] + left[1] + left[2] + left[3] + left[4] + left[5] + left[6] +
          left[7] + left[8] + left[9] + left[10] + left[11] + left[12] +
          left[13] + left[14] + left[15] + top[0] + top[1] + top[2] + top[3] +
          top[4] + top[5] + top[6] + top[7] + top[8] + top[9] + top[10] +
          top[11] + top[12] + top[13] + top[14] + top[15] + 16) >> 5);

    }

                       const uint8_t *_left, const uint8_t *_top)
{
        ((left[0] + left[1] + left[2] + left[3] + left[4] + left[5] + left[6] +
          left[7] + left[8] + left[9] + left[10] + left[11] + left[12] +
          left[13] + left[14] + left[15] + left[16] + left[17] + left[18] +
          left[19] + left[20] + left[21] + left[22] + left[23] + left[24] +
          left[25] + left[26] + left[27] + left[28] + left[29] + left[30] +
          left[31] + top[0] + top[1] + top[2] + top[3] + top[4] + top[5] +
          top[6] + top[7] + top[8] + top[9] + top[10] + top[11] + top[12] +
          top[13] + top[14] + top[15] + top[16] + top[17] + top[18] + top[19] +
          top[20] + top[21] + top[22] + top[23] + top[24] + top[25] + top[26] +
          top[27] + top[28] + top[29] + top[30] + top[31] + 32) >> 6);

    }

                          const uint8_t *_left, const uint8_t *top)
{


                          const uint8_t *_left, const uint8_t *top)
{
        ((left[0] + left[1] + left[2] + left[3] +
          left[4] + left[5] + left[6] + left[7] + 4) >> 3);

    }

                            const uint8_t *_left, const uint8_t *top)
{
        ((left[0] + left[1] + left[2] + left[3] + left[4] + left[5] +
          left[6] + left[7] + left[8] + left[9] + left[10] + left[11] +
          left[12] + left[13] + left[14] + left[15] + 8) >> 4);

    }

                            const uint8_t *_left, const uint8_t *top)
{
        ((left[0] + left[1] + left[2] + left[3] + left[4] + left[5] +
          left[6] + left[7] + left[8] + left[9] + left[10] + left[11] +
          left[12] + left[13] + left[14] + left[15] + left[16] + left[17] +
          left[18] + left[19] + left[20] + left[21] + left[22] + left[23] +
          left[24] + left[25] + left[26] + left[27] + left[28] + left[29] +
          left[30] + left[31] + 16) >> 5);

    }

                         const uint8_t *left, const uint8_t *_top)
{


                         const uint8_t *left, const uint8_t *_top)
{
        ((top[0] + top[1] + top[2] + top[3] +
          top[4] + top[5] + top[6] + top[7] + 4) >> 3);

    }

                           const uint8_t *left, const uint8_t *_top)
{
        ((top[0] + top[1] + top[2] + top[3] + top[4] + top[5] +
          top[6] + top[7] + top[8] + top[9] + top[10] + top[11] +
          top[12] + top[13] + top[14] + top[15] + 8) >> 4);

    }

                           const uint8_t *left, const uint8_t *_top)
{
        ((top[0] + top[1] + top[2] + top[3] + top[4] + top[5] +
          top[6] + top[7] + top[8] + top[9] + top[10] + top[11] +
          top[12] + top[13] + top[14] + top[15] + top[16] + top[17] +
          top[18] + top[19] + top[20] + top[21] + top[22] + top[23] +
          top[24] + top[25] + top[26] + top[27] + top[28] + top[29] +
          top[30] + top[31] + 16) >> 5);

    }

#endif /* BIT_DEPTH != 12 */

                         const uint8_t *left, const uint8_t *top)
{


                         const uint8_t *left, const uint8_t *top)
{

    }

                           const uint8_t *left, const uint8_t *top)
{

    }

                           const uint8_t *left, const uint8_t *top)
{

    }

                         const uint8_t *left, const uint8_t *top)
{


                         const uint8_t *left, const uint8_t *top)
{

    }

                           const uint8_t *left, const uint8_t *top)
{

    }

                           const uint8_t *left, const uint8_t *top)
{

    }

                         const uint8_t *left, const uint8_t *top)
{


                         const uint8_t *left, const uint8_t *top)
{

    }

                           const uint8_t *left, const uint8_t *top)
{

    }

                           const uint8_t *left, const uint8_t *top)
{

    }

#if BIT_DEPTH != 12

#if BIT_DEPTH == 8
#define memset_bpc memset
#else
    int n;
    }
}
#endif

#define DST(x, y) dst[(x) + (y) * stride]

                                const uint8_t *left, const uint8_t *_top)
{


#define def_diag_downleft(size) \
static void diag_downleft_##size##x##size##_c(uint8_t *_dst, ptrdiff_t stride, \
                                              const uint8_t *left, const uint8_t *_top) \
{ \
    pixel *dst = (pixel *) _dst; \
    const pixel *top = (const pixel *) _top; \
    int i, j; \
    pixel v[size - 1]; \
\
    stride /= sizeof(pixel); \
    for (i = 0; i < size - 2; i++) \
        v[i] = (top[i] + top[i + 1] * 2 + top[i + 2] + 2) >> 2; \
    v[size - 2] = (top[size - 2] + top[size - 1] * 3 + 2) >> 2; \
\
    for (j = 0; j < size; j++) { \
        memcpy(dst + j*stride, v + j, (size - 1 - j) * sizeof(pixel)); \
        memset_bpc(dst + j*stride + size - 1 - j, top[size - 1], j + 1); \
    } \
}


                                 const uint8_t *_left, const uint8_t *_top)
{


#define def_diag_downright(size) \
static void diag_downright_##size##x##size##_c(uint8_t *_dst, ptrdiff_t stride, \
                                               const uint8_t *_left, const uint8_t *_top) \
{ \
    pixel *dst = (pixel *) _dst; \
    const pixel *top = (const pixel *) _top; \
    const pixel *left = (const pixel *) _left; \
    int i, j; \
    pixel v[size + size - 1]; \
\
    stride /= sizeof(pixel); \
    for (i = 0; i < size - 2; i++) { \
        v[i           ] = (left[i] + left[i + 1] * 2 + left[i + 2] + 2) >> 2; \
        v[size + 1 + i] = (top[i]  + top[i + 1]  * 2 + top[i + 2]  + 2) >> 2; \
    } \
    v[size - 2] = (left[size - 2] + left[size - 1] * 2 + top[-1] + 2) >> 2; \
    v[size - 1] = (left[size - 1] + top[-1] * 2 + top[ 0] + 2) >> 2; \
    v[size    ] = (top[-1] + top[0]  * 2 + top[ 1] + 2) >> 2; \
\
    for (j = 0; j < size; j++) \
        memcpy(dst + j*stride, v + size - 1 - j, size * sizeof(pixel)); \
}


                             const uint8_t *_left, const uint8_t *_top)
{


#define def_vert_right(size) \
static void vert_right_##size##x##size##_c(uint8_t *_dst, ptrdiff_t stride, \
                                           const uint8_t *_left, const uint8_t *_top) \
{ \
    pixel *dst = (pixel *) _dst; \
    const pixel *top = (const pixel *) _top; \
    const pixel *left = (const pixel *) _left; \
    int i, j; \
    pixel ve[size + size/2 - 1], vo[size + size/2 - 1]; \
\
    stride /= sizeof(pixel); \
    for (i = 0; i < size/2 - 2; i++) { \
        vo[i] = (left[i*2 + 3] + left[i*2 + 2] * 2 + left[i*2 + 1] + 2) >> 2; \
        ve[i] = (left[i*2 + 4] + left[i*2 + 3] * 2 + left[i*2 + 2] + 2) >> 2; \
    } \
    vo[size/2 - 2] = (left[size - 1] + left[size - 2] * 2 + left[size - 3] + 2) >> 2; \
    ve[size/2 - 2] = (top[-1] + left[size - 1] * 2 + left[size - 2] + 2) >> 2; \
\
    ve[size/2 - 1] = (top[-1] + top[0] + 1) >> 1; \
    vo[size/2 - 1] = (left[size - 1] + top[-1] * 2 + top[0] + 2) >> 2; \
    for (i = 0; i < size - 1; i++) { \
        ve[size/2 + i] = (top[i] + top[i + 1] + 1) >> 1; \
        vo[size/2 + i] = (top[i - 1] + top[i] * 2 + top[i + 1] + 2) >> 2; \
    } \
\
    for (j = 0; j < size / 2; j++) { \
        memcpy(dst +  j*2     *stride, ve + size/2 - 1 - j, size * sizeof(pixel)); \
        memcpy(dst + (j*2 + 1)*stride, vo + size/2 - 1 - j, size * sizeof(pixel)); \
    } \
}


                           const uint8_t *_left, const uint8_t *_top)
{


#define def_hor_down(size) \
static void hor_down_##size##x##size##_c(uint8_t *_dst, ptrdiff_t stride, \
                                         const uint8_t *_left, const uint8_t *_top) \
{ \
    pixel *dst = (pixel *) _dst; \
    const pixel *top = (const pixel *) _top; \
    const pixel *left = (const pixel *) _left; \
    int i, j; \
    pixel v[size * 3 - 2]; \
\
    stride /= sizeof(pixel); \
    for (i = 0; i < size - 2; i++) { \
        v[i*2       ] = (left[i + 1] + left[i + 0] + 1) >> 1; \
        v[i*2    + 1] = (left[i + 2] + left[i + 1] * 2 + left[i + 0] + 2) >> 2; \
        v[size*2 + i] = (top[i - 1] + top[i] * 2 + top[i + 1] + 2) >> 2; \
    } \
    v[size*2 - 2] = (top[-1] + left[size - 1] + 1) >> 1; \
    v[size*2 - 4] = (left[size - 1] + left[size - 2] + 1) >> 1; \
    v[size*2 - 1] = (top[0]  + top[-1] * 2 + left[size - 1] + 2) >> 2; \
    v[size*2 - 3] = (top[-1] + left[size - 1] * 2 + left[size - 2] + 2) >> 2; \
\
    for (j = 0; j < size; j++) \
        memcpy(dst + j*stride, v + size*2 - 2 - j*2, size * sizeof(pixel)); \
}


                            const uint8_t *left, const uint8_t *_top)
{


#define def_vert_left(size) \
static void vert_left_##size##x##size##_c(uint8_t *_dst, ptrdiff_t stride, \
                                          const uint8_t *left, const uint8_t *_top) \
{ \
    pixel *dst = (pixel *) _dst; \
    const pixel *top = (const pixel *) _top; \
    int i, j; \
    pixel ve[size - 1], vo[size - 1]; \
\
    stride /= sizeof(pixel); \
    for (i = 0; i < size - 2; i++) { \
        ve[i] = (top[i] + top[i + 1] + 1) >> 1; \
        vo[i] = (top[i] + top[i + 1] * 2 + top[i + 2] + 2) >> 2; \
    } \
    ve[size - 2] = (top[size - 2] + top[size - 1] + 1) >> 1; \
    vo[size - 2] = (top[size - 2] + top[size - 1] * 3 + 2) >> 2; \
\
    for (j = 0; j < size / 2; j++) { \
        memcpy(dst +  j*2      * stride, ve + j, (size - j - 1) * sizeof(pixel)); \
        memset_bpc(dst +  j*2      * stride + size - j - 1, top[size - 1], j + 1); \
        memcpy(dst + (j*2 + 1) * stride, vo + j, (size - j - 1) * sizeof(pixel)); \
        memset_bpc(dst + (j*2 + 1) * stride + size - j - 1, top[size - 1], j + 1); \
    } \
}


                         const uint8_t *_left, const uint8_t *top)
{


#define def_hor_up(size) \
static void hor_up_##size##x##size##_c(uint8_t *_dst, ptrdiff_t stride, \
                                       const uint8_t *_left, const uint8_t *top) \
{ \
    pixel *dst = (pixel *) _dst; \
    const pixel *left = (const pixel *) _left; \
    int i, j; \
    pixel v[size*2 - 2]; \
\
    stride /= sizeof(pixel); \
    for (i = 0; i < size - 2; i++) { \
        v[i*2    ] = (left[i] + left[i + 1] + 1) >> 1; \
        v[i*2 + 1] = (left[i] + left[i + 1] * 2 + left[i + 2] + 2) >> 2; \
    } \
    v[size*2 - 4] = (left[size - 2] + left[size - 1] + 1) >> 1; \
    v[size*2 - 3] = (left[size - 2] + left[size - 1] * 3 + 2) >> 2; \
\
    for (j = 0; j < size / 2; j++) \
        memcpy(dst + j*stride, v + j*2, size * sizeof(pixel)); \
    for (j = size / 2; j < size; j++) { \
        memcpy(dst + j*stride, v + j*2, (size*2 - 2 - j*2) * sizeof(pixel)); \
        memset_bpc(dst + j*stride + size*2 - 2 - j*2, left[size - 1], \
                   2 + j*2 - size); \
    } \
}


#undef DST

#endif /* BIT_DEPTH != 12 */

#if BIT_DEPTH != 8
void ff_vp9dsp_intrapred_init_10(VP9DSPContext *dsp);
#endif
#if BIT_DEPTH != 10
static
#endif
{
#define init_intra_pred_bd_aware(tx, sz) \
    dsp->intra_pred[tx][TM_VP8_PRED]          = tm_##sz##_c; \
    dsp->intra_pred[tx][DC_128_PRED]          = dc_128_##sz##_c; \
    dsp->intra_pred[tx][DC_127_PRED]          = dc_127_##sz##_c; \
    dsp->intra_pred[tx][DC_129_PRED]          = dc_129_##sz##_c

#if BIT_DEPTH == 12
#define init_intra_pred(tx, sz) \
    init_intra_pred_bd_aware(tx, sz)
#else
    #define init_intra_pred(tx, sz) \
    dsp->intra_pred[tx][VERT_PRED]            = vert_##sz##_c; \
    dsp->intra_pred[tx][HOR_PRED]             = hor_##sz##_c; \
    dsp->intra_pred[tx][DC_PRED]              = dc_##sz##_c; \
    dsp->intra_pred[tx][DIAG_DOWN_LEFT_PRED]  = diag_downleft_##sz##_c; \
    dsp->intra_pred[tx][DIAG_DOWN_RIGHT_PRED] = diag_downright_##sz##_c; \
    dsp->intra_pred[tx][VERT_RIGHT_PRED]      = vert_right_##sz##_c; \
    dsp->intra_pred[tx][HOR_DOWN_PRED]        = hor_down_##sz##_c; \
    dsp->intra_pred[tx][VERT_LEFT_PRED]       = vert_left_##sz##_c; \
    dsp->intra_pred[tx][HOR_UP_PRED]          = hor_up_##sz##_c; \
    dsp->intra_pred[tx][LEFT_DC_PRED]         = dc_left_##sz##_c; \
    dsp->intra_pred[tx][TOP_DC_PRED]          = dc_top_##sz##_c; \
    init_intra_pred_bd_aware(tx, sz)
#endif


#undef init_intra_pred
#undef init_intra_pred_bd_aware

#define itxfm_wrapper(type_a, type_b, sz, bits, has_dconly) \
static void type_a##_##type_b##_##sz##x##sz##_add_c(uint8_t *_dst, \
                                                    ptrdiff_t stride, \
                                                    int16_t *_block, int eob) \
{ \
    int i, j; \
    pixel *dst = (pixel *) _dst; \
    dctcoef *block = (dctcoef *) _block, tmp[sz * sz], out[sz]; \
\
    stride /= sizeof(pixel); \
    if (has_dconly && eob == 1) { \
        const int t  = ((((dctint) block[0] * 11585 + (1 << 13)) >> 14) \
                                            * 11585 + (1 << 13)) >> 14; \
        block[0] = 0; \
        for (i = 0; i < sz; i++) { \
            for (j = 0; j < sz; j++) \
                dst[j * stride] = av_clip_pixel(dst[j * stride] + \
                                                (bits ? \
                                                 (t + (1 << (bits - 1))) >> bits : \
                                                 t)); \
            dst++; \
        } \
        return; \
    } \
\
    for (i = 0; i < sz; i++) \
        type_a##sz##_1d(block + i, sz, tmp + i * sz, 0); \
    memset(block, 0, sz * sz * sizeof(*block)); \
    for (i = 0; i < sz; i++) { \
        type_b##sz##_1d(tmp + i, sz, out, 1); \
        for (j = 0; j < sz; j++) \
            dst[j * stride] = av_clip_pixel(dst[j * stride] + \
                                            (bits ? \
                                             (out[j] + (1 << (bits - 1))) >> bits : \
                                             out[j])); \
        dst++; \
    } \
}

#define itxfm_wrap(sz, bits) \
itxfm_wrapper(idct,  idct,  sz, bits, 1) \
itxfm_wrapper(iadst, idct,  sz, bits, 0) \
itxfm_wrapper(idct,  iadst, sz, bits, 0) \
itxfm_wrapper(iadst, iadst, sz, bits, 0)

#define IN(x) ((dctint) in[(x) * stride])

                                      dctcoef *out, int pass)
{


}

                                       dctcoef *out, int pass)
{


}


                                      dctcoef *out, int pass)
{




}

                                       dctcoef *out, int pass)
{






}


                                       dctcoef *out, int pass)
{






}

                                        dctcoef *out, int pass)
{







}


                                       dctcoef *out, int pass)
{







}


                                      dctcoef *out, int pass)
{

    } else {
    }


}


#undef IN
#undef itxfm_wrapper
#undef itxfm_wrap

{
#define init_itxfm(tx, sz) \
    dsp->itxfm_add[tx][DCT_DCT]   = idct_idct_##sz##_add_c; \
    dsp->itxfm_add[tx][DCT_ADST]  = iadst_idct_##sz##_add_c; \
    dsp->itxfm_add[tx][ADST_DCT]  = idct_iadst_##sz##_add_c; \
    dsp->itxfm_add[tx][ADST_ADST] = iadst_iadst_##sz##_add_c

#define init_idct(tx, nm) \
    dsp->itxfm_add[tx][DCT_DCT]   = \
    dsp->itxfm_add[tx][ADST_DCT]  = \
    dsp->itxfm_add[tx][DCT_ADST]  = \
    dsp->itxfm_add[tx][ADST_ADST] = nm##_add_c


#undef init_itxfm
#undef init_idct

                                         ptrdiff_t stridea, ptrdiff_t strideb,
                                         int wd)
{




        }


        } else {



            } else {



            }
        }
    }
}

#define lf_8_fn(dir, wd, stridea, strideb) \
static void loop_filter_##dir##_##wd##_8_c(uint8_t *_dst, \
                                           ptrdiff_t stride, \
                                           int E, int I, int H) \
{ \
    pixel *dst = (pixel *) _dst; \
    stride /= sizeof(pixel); \
    loop_filter(dst, E, I, H, stridea, strideb, wd); \
}

#define lf_8_fns(wd) \
lf_8_fn(h, wd, stride, 1) \
lf_8_fn(v, wd, 1, stride)


#undef lf_8_fn
#undef lf_8_fns

#define lf_16_fn(dir, stridea) \
static void loop_filter_##dir##_16_16_c(uint8_t *dst, \
                                        ptrdiff_t stride, \
                                        int E, int I, int H) \
{ \
    loop_filter_##dir##_16_8_c(dst, stride, E, I, H); \
    loop_filter_##dir##_16_8_c(dst + 8 * stridea, stride, E, I, H); \
}


#undef lf_16_fn

#define lf_mix_fn(dir, wd1, wd2, stridea) \
static void loop_filter_##dir##_##wd1##wd2##_16_c(uint8_t *dst, \
                                                  ptrdiff_t stride, \
                                                  int E, int I, int H) \
{ \
    loop_filter_##dir##_##wd1##_8_c(dst, stride, E & 0xff, I & 0xff, H & 0xff); \
    loop_filter_##dir##_##wd2##_8_c(dst + 8 * stridea, stride, E >> 8, I >> 8, H >> 8); \
}

#define lf_mix_fns(wd1, wd2) \
lf_mix_fn(h, wd1, wd2, stride) \
lf_mix_fn(v, wd1, wd2, sizeof(pixel))


#undef lf_mix_fn
#undef lf_mix_fns

{



#if BIT_DEPTH != 12

                                    const uint8_t *src, ptrdiff_t src_stride,
                                    int w, int h)
{

}

                                   const uint8_t *_src, ptrdiff_t src_stride,
                                   int w, int h)
{



}

#define fpel_fn(type, sz) \
static void type##sz##_c(uint8_t *dst, ptrdiff_t dst_stride, \
                         const uint8_t *src, ptrdiff_t src_stride, \
                         int h, int mx, int my) \
{ \
    type##_c(dst, dst_stride, src, src_stride, sz, h); \
}

#define copy_avg_fn(sz) \
fpel_fn(copy, sz) \
fpel_fn(avg,  sz)


#undef fpel_fn
#undef copy_avg_fn

#endif /* BIT_DEPTH != 12 */

#define FILTER_8TAP(src, x, F, stride) \
    av_clip_pixel((F[0] * src[x + -3 * stride] + \
                   F[1] * src[x + -2 * stride] + \
                   F[2] * src[x + -1 * stride] + \
                   F[3] * src[x + +0 * stride] + \
                   F[4] * src[x + +1 * stride] + \
                   F[5] * src[x + +2 * stride] + \
                   F[6] * src[x + +3 * stride] + \
                   F[7] * src[x + +4 * stride] + 64) >> 7)

                                          const uint8_t *_src, ptrdiff_t src_stride,
                                          int w, int h, ptrdiff_t ds,
                                          const int16_t *filter, int avg)
{


            } else {
            }

}

#define filter_8tap_1d_fn(opn, opa, dir, ds) \
static av_noinline void opn##_8tap_1d_##dir##_c(uint8_t *dst, ptrdiff_t dst_stride, \
                                                const uint8_t *src, ptrdiff_t src_stride, \
                                                int w, int h, const int16_t *filter) \
{ \
    do_8tap_1d_c(dst, dst_stride, src, src_stride, w, h, ds, filter, opa); \
}


#undef filter_8tap_1d_fn

                                          const uint8_t *_src, ptrdiff_t src_stride,
                                          int w, int h, const int16_t *filterx,
                                          const int16_t *filtery, int avg)
{




    tmp_ptr = tmp + 64 * 3;

            } else {
            }

}

#define filter_8tap_2d_fn(opn, opa) \
static av_noinline void opn##_8tap_2d_hv_c(uint8_t *dst, ptrdiff_t dst_stride, \
                                           const uint8_t *src, ptrdiff_t src_stride, \
                                           int w, int h, const int16_t *filterx, \
                                           const int16_t *filtery) \
{ \
    do_8tap_2d_c(dst, dst_stride, src, src_stride, w, h, filterx, filtery, opa); \
}


#undef filter_8tap_2d_fn

#define filter_fn_1d(sz, dir, dir_m, type, type_idx, avg) \
static void avg##_8tap_##type##_##sz##dir##_c(uint8_t *dst, ptrdiff_t dst_stride, \
                                              const uint8_t *src, ptrdiff_t src_stride, \
                                              int h, int mx, int my) \
{ \
    avg##_8tap_1d_##dir##_c(dst, dst_stride, src, src_stride, sz, h, \
                            ff_vp9_subpel_filters[type_idx][dir_m]); \
}

#define filter_fn_2d(sz, type, type_idx, avg) \
static void avg##_8tap_##type##_##sz##hv_c(uint8_t *dst, ptrdiff_t dst_stride, \
                                           const uint8_t *src, ptrdiff_t src_stride, \
                                           int h, int mx, int my) \
{ \
    avg##_8tap_2d_hv_c(dst, dst_stride, src, src_stride, sz, h, \
                       ff_vp9_subpel_filters[type_idx][mx], \
                       ff_vp9_subpel_filters[type_idx][my]); \
}

#if BIT_DEPTH != 12

#define FILTER_BILIN(src, x, mxy, stride) \
    (src[x] + ((mxy * (src[x + stride] - src[x]) + 8) >> 4))

                                           const uint8_t *_src, ptrdiff_t src_stride,
                                           int w, int h, ptrdiff_t ds, int mxy, int avg)
{


            } else {
            }

}

#define bilin_1d_fn(opn, opa, dir, ds) \
static av_noinline void opn##_bilin_1d_##dir##_c(uint8_t *dst, ptrdiff_t dst_stride, \
                                                 const uint8_t *src, ptrdiff_t src_stride, \
                                                 int w, int h, int mxy) \
{ \
    do_bilin_1d_c(dst, dst_stride, src, src_stride, w, h, ds, mxy, opa); \
}


#undef bilin_1d_fn

                                           const uint8_t *_src, ptrdiff_t src_stride,
                                           int w, int h, int mx, int my, int avg)
{




    tmp_ptr = tmp;

            } else {
            }

}

#define bilin_2d_fn(opn, opa) \
static av_noinline void opn##_bilin_2d_hv_c(uint8_t *dst, ptrdiff_t dst_stride, \
                                            const uint8_t *src, ptrdiff_t src_stride, \
                                            int w, int h, int mx, int my) \
{ \
    do_bilin_2d_c(dst, dst_stride, src, src_stride, w, h, mx, my, opa); \
}


#undef bilin_2d_fn

#define bilinf_fn_1d(sz, dir, dir_m, avg) \
static void avg##_bilin_##sz##dir##_c(uint8_t *dst, ptrdiff_t dst_stride, \
                                      const uint8_t *src, ptrdiff_t src_stride, \
                                      int h, int mx, int my) \
{ \
    avg##_bilin_1d_##dir##_c(dst, dst_stride, src, src_stride, sz, h, dir_m); \
}

#define bilinf_fn_2d(sz, avg) \
static void avg##_bilin_##sz##hv_c(uint8_t *dst, ptrdiff_t dst_stride, \
                                   const uint8_t *src, ptrdiff_t src_stride, \
                                   int h, int mx, int my) \
{ \
    avg##_bilin_2d_hv_c(dst, dst_stride, src, src_stride, sz, h, mx, my); \
}

#else

#define bilinf_fn_1d(a, b, c, d)
#define bilinf_fn_2d(a, b)

#endif

#define filter_fn(sz, avg) \
filter_fn_1d(sz, h, mx, regular, FILTER_8TAP_REGULAR, avg) \
filter_fn_1d(sz, v, my, regular, FILTER_8TAP_REGULAR, avg) \
filter_fn_2d(sz,        regular, FILTER_8TAP_REGULAR, avg) \
filter_fn_1d(sz, h, mx, smooth,  FILTER_8TAP_SMOOTH,  avg) \
filter_fn_1d(sz, v, my, smooth,  FILTER_8TAP_SMOOTH,  avg) \
filter_fn_2d(sz,        smooth,  FILTER_8TAP_SMOOTH,  avg) \
filter_fn_1d(sz, h, mx, sharp,   FILTER_8TAP_SHARP,   avg) \
filter_fn_1d(sz, v, my, sharp,   FILTER_8TAP_SHARP,   avg) \
filter_fn_2d(sz,        sharp,   FILTER_8TAP_SHARP,   avg) \
bilinf_fn_1d(sz, h, mx,                               avg) \
bilinf_fn_1d(sz, v, my,                               avg) \
bilinf_fn_2d(sz,                                      avg)

#define filter_fn_set(avg) \
filter_fn(64, avg) \
filter_fn(32, avg) \
filter_fn(16, avg) \
filter_fn(8,  avg) \
filter_fn(4,  avg)


#undef filter_fn
#undef filter_fn_set
#undef filter_fn_1d
#undef filter_fn_2d
#undef bilinf_fn_1d
#undef bilinf_fn_2d

#if BIT_DEPTH != 8
void ff_vp9dsp_mc_init_10(VP9DSPContext *dsp);
#endif
#if BIT_DEPTH != 10
static
#endif
{
#if BIT_DEPTH == 12
#else /* BIT_DEPTH == 12 */

#define init_fpel(idx1, idx2, sz, type) \
    dsp->mc[idx1][FILTER_8TAP_SMOOTH ][idx2][0][0] = type##sz##_c; \
    dsp->mc[idx1][FILTER_8TAP_REGULAR][idx2][0][0] = type##sz##_c; \
    dsp->mc[idx1][FILTER_8TAP_SHARP  ][idx2][0][0] = type##sz##_c; \
    dsp->mc[idx1][FILTER_BILINEAR    ][idx2][0][0] = type##sz##_c

#define init_copy_avg(idx, sz) \
    init_fpel(idx, 0, sz, copy); \
    init_fpel(idx, 1, sz, avg)


#undef init_copy_avg
#undef init_fpel

#endif /* BIT_DEPTH == 12 */

#define init_subpel1_bd_aware(idx1, idx2, idxh, idxv, sz, dir, type) \
    dsp->mc[idx1][FILTER_8TAP_SMOOTH ][idx2][idxh][idxv] = type##_8tap_smooth_##sz##dir##_c; \
    dsp->mc[idx1][FILTER_8TAP_REGULAR][idx2][idxh][idxv] = type##_8tap_regular_##sz##dir##_c; \
    dsp->mc[idx1][FILTER_8TAP_SHARP  ][idx2][idxh][idxv] = type##_8tap_sharp_##sz##dir##_c

#if BIT_DEPTH == 12
#define init_subpel1 init_subpel1_bd_aware
#else
#define init_subpel1(idx1, idx2, idxh, idxv, sz, dir, type) \
    init_subpel1_bd_aware(idx1, idx2, idxh, idxv, sz, dir, type); \
    dsp->mc[idx1][FILTER_BILINEAR    ][idx2][idxh][idxv] = type##_bilin_##sz##dir##_c
#endif

#define init_subpel2(idx, idxh, idxv, dir, type) \
    init_subpel1(0, idx, idxh, idxv, 64, dir, type); \
    init_subpel1(1, idx, idxh, idxv, 32, dir, type); \
    init_subpel1(2, idx, idxh, idxv, 16, dir, type); \
    init_subpel1(3, idx, idxh, idxv,  8, dir, type); \
    init_subpel1(4, idx, idxh, idxv,  4, dir, type)

#define init_subpel3(idx, type) \
    init_subpel2(idx, 1, 1, hv, type); \
    init_subpel2(idx, 0, 1, v, type); \
    init_subpel2(idx, 1, 0, h, type)


#undef init_subpel1
#undef init_subpel2
#undef init_subpel3
#undef init_subpel1_bd_aware

                                              const uint8_t *_src, ptrdiff_t src_stride,
                                              int w, int h, int mx, int my,
                                              int dx, int dy, int avg,
                                              const int16_t (*filters)[8])
{


        }


    tmp_ptr = tmp + 64 * 3;

                dst[x] = (dst[x] + FILTER_8TAP(tmp_ptr, x, filter, 64) + 1) >> 1;
            } else {
            }

}

#define scaled_filter_8tap_fn(opn, opa) \
static av_noinline void opn##_scaled_8tap_c(uint8_t *dst, ptrdiff_t dst_stride, \
                                            const uint8_t *src, ptrdiff_t src_stride, \
                                            int w, int h, int mx, int my, int dx, int dy, \
                                            const int16_t (*filters)[8]) \
{ \
    do_scaled_8tap_c(dst, dst_stride, src, src_stride, w, h, mx, my, dx, dy, \
                     opa, filters); \
}

scaled_filter_8tap_fn(avg, 1)

#undef scaled_filter_8tap_fn

#undef FILTER_8TAP

#define scaled_filter_fn(sz, type, type_idx, avg) \
static void avg##_scaled_##type##_##sz##_c(uint8_t *dst, ptrdiff_t dst_stride, \
                                           const uint8_t *src, ptrdiff_t src_stride, \
                                           int h, int mx, int my, int dx, int dy) \
{ \
    avg##_scaled_8tap_c(dst, dst_stride, src, src_stride, sz, h, mx, my, dx, dy, \
                        ff_vp9_subpel_filters[type_idx]); \
}

#if BIT_DEPTH != 12

static av_always_inline void do_scaled_bilin_c(uint8_t *_dst, ptrdiff_t dst_stride,
                                               const uint8_t *_src, ptrdiff_t src_stride,
                                               int w, int h, int mx, int my,
                                               int dx, int dy, int avg)
{
    pixel tmp[64 * 129], *tmp_ptr = tmp;
    int tmp_h = (((h - 1) * dy + my) >> 4) + 2;
    pixel *dst = (pixel *) _dst;
    const pixel *src = (const pixel *) _src;

    dst_stride /= sizeof(pixel);
    src_stride /= sizeof(pixel);
    do {
        int x;
        int imx = mx, ioff = 0;

        for (x = 0; x < w; x++) {
            tmp_ptr[x] = FILTER_BILIN(src, ioff, imx, 1);
            imx += dx;
            ioff += imx >> 4;
            imx &= 0xf;
        }

        tmp_ptr += 64;
        src += src_stride;
    } while (--tmp_h);

    tmp_ptr = tmp;
    do {
        int x;

        for (x = 0; x < w; x++)
            if (avg) {
                dst[x] = (dst[x] + FILTER_BILIN(tmp_ptr, x, my, 64) + 1) >> 1;
            } else {
                dst[x] = FILTER_BILIN(tmp_ptr, x, my, 64);
            }

        my += dy;
        tmp_ptr += (my >> 4) * 64;
        my &= 0xf;
        dst += dst_stride;
    } while (--h);
}

#define scaled_bilin_fn(opn, opa) \
static av_noinline void opn##_scaled_bilin_c(uint8_t *dst, ptrdiff_t dst_stride, \
                                             const uint8_t *src, ptrdiff_t src_stride, \
                                             int w, int h, int mx, int my, int dx, int dy) \
{ \
    do_scaled_bilin_c(dst, dst_stride, src, src_stride, w, h, mx, my, dx, dy, opa); \
}

scaled_bilin_fn(put, 0)
scaled_bilin_fn(avg, 1)

#undef scaled_bilin_fn

#undef FILTER_BILIN

#define scaled_bilinf_fn(sz, avg) \
static void avg##_scaled_bilin_##sz##_c(uint8_t *dst, ptrdiff_t dst_stride, \
                                        const uint8_t *src, ptrdiff_t src_stride, \
                                        int h, int mx, int my, int dx, int dy) \
{ \
    avg##_scaled_bilin_c(dst, dst_stride, src, src_stride, sz, h, mx, my, dx, dy); \
}

#else

#define scaled_bilinf_fn(a, b)

#endif

#define scaled_filter_fns(sz, avg) \
scaled_filter_fn(sz,        regular, FILTER_8TAP_REGULAR, avg) \
scaled_filter_fn(sz,        smooth,  FILTER_8TAP_SMOOTH,  avg) \
scaled_filter_fn(sz,        sharp,   FILTER_8TAP_SHARP,   avg) \
scaled_bilinf_fn(sz,                                      avg)

#define scaled_filter_fn_set(avg) \
scaled_filter_fns(64, avg) \
scaled_filter_fns(32, avg) \
scaled_filter_fns(16, avg) \
scaled_filter_fns(8,  avg) \
scaled_filter_fns(4,  avg)

scaled_filter_fn_set(avg)

#undef scaled_filter_fns
#undef scaled_filter_fn_set
#undef scaled_filter_fn
#undef scaled_bilinf_fn

#if BIT_DEPTH != 8
void ff_vp9dsp_scaled_mc_init_10(VP9DSPContext *dsp);
#endif
#if BIT_DEPTH != 10
static
#endif
{
#define init_scaled_bd_aware(idx1, idx2, sz, type) \
    dsp->smc[idx1][FILTER_8TAP_SMOOTH ][idx2] = type##_scaled_smooth_##sz##_c; \
    dsp->smc[idx1][FILTER_8TAP_REGULAR][idx2] = type##_scaled_regular_##sz##_c; \
    dsp->smc[idx1][FILTER_8TAP_SHARP  ][idx2] = type##_scaled_sharp_##sz##_c

#if BIT_DEPTH == 12
#define init_scaled(a,b,c,d) init_scaled_bd_aware(a,b,c,d)
#else
#define init_scaled(idx1, idx2, sz, type) \
    init_scaled_bd_aware(idx1, idx2, sz, type); \
    dsp->smc[idx1][FILTER_BILINEAR    ][idx2] = type##_scaled_bilin_##sz##_c
#endif

#define init_scaled_put_avg(idx, sz) \
    init_scaled(idx, 0, sz, put); \
    init_scaled(idx, 1, sz, avg)


#undef init_scaled_put_avg
#undef init_scaled
#undef init_scaled_bd_aware

{
