/*
 * VP7/VP8 compatible video decoder
 *
 * Copyright (C) 2010 David Conrad
 * Copyright (C) 2010 Ronald S. Bultje
 * Copyright (C) 2010 Fiona Glaser
 * Copyright (C) 2012 Daniel Kang
 * Copyright (C) 2014 Peter Ross
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

#include "libavutil/imgutils.h"

#include "avcodec.h"
#include "hwconfig.h"
#include "internal.h"
#include "mathops.h"
#include "rectangle.h"
#include "thread.h"
#include "vp8.h"
#include "vp8data.h"

#if ARCH_ARM
#   include "arm/vp8.h"
#endif

#if CONFIG_VP7_DECODER && CONFIG_VP8_DECODER
#define VPX(vp7, f) (vp7 ? vp7_ ## f : vp8_ ## f)
#elif CONFIG_VP7_DECODER
#define VPX(vp7, f) vp7_ ## f
#else // CONFIG_VP8_DECODER
#define VPX(vp7, f) vp8_ ## f
#endif

{
#if HAVE_THREADS
#endif
        }


{
                                    ref ? AV_GET_BUFFER_FLAG_REF : 0)) < 0)
        return ret;
        goto fail;
        const AVHWAccel *hwaccel = s->avctx->hwaccel;
        if (hwaccel->frame_priv_data_size) {
            f->hwaccel_priv_buf = av_buffer_allocz(hwaccel->frame_priv_data_size);
            if (!f->hwaccel_priv_buf)
                goto fail;
            f->hwaccel_picture_private = f->hwaccel_priv_buf->data;
        }
    }
    return 0;

fail:
    av_buffer_unref(&f->seg_map);
    ff_thread_release_buffer(s->avctx, &f->tf);
    return AVERROR(ENOMEM);
}

static void vp8_release_frame(VP8Context *s, VP8Frame *f)
{
    av_buffer_unref(&f->seg_map);
    av_buffer_unref(&f->hwaccel_priv_buf);
    f->hwaccel_picture_private = NULL;
    ff_thread_release_buffer(s->avctx, &f->tf);
}

#if CONFIG_VP8_DECODER
static int vp8_ref_frame(VP8Context *s, VP8Frame *dst, VP8Frame *src)
{
    int ret;

    vp8_release_frame(s, dst);

    if ((ret = ff_thread_ref_frame(&dst->tf, &src->tf)) < 0)
        return ret;
    if (src->seg_map &&
        !(dst->seg_map = av_buffer_ref(src->seg_map))) {
        vp8_release_frame(s, dst);
        return AVERROR(ENOMEM);
    }
    if (src->hwaccel_picture_private) {
        dst->hwaccel_priv_buf = av_buffer_ref(src->hwaccel_priv_buf);
        if (!dst->hwaccel_priv_buf)
            return AVERROR(ENOMEM);
        dst->hwaccel_picture_private = dst->hwaccel_priv_buf->data;
    }

    return 0;
}
#endif /* CONFIG_VP8_DECODER */

static void vp8_decode_flush_impl(AVCodecContext *avctx, int free_mem)
{
    VP8Context *s = avctx->priv_data;
    int i;

    for (i = 0; i < FF_ARRAY_ELEMS(s->frames); i++)
        vp8_release_frame(s, &s->frames[i]);
    memset(s->framep, 0, sizeof(s->framep));

    if (free_mem)
        free_buffers(s);
}

static void vp8_decode_flush(AVCodecContext *avctx)
{
    vp8_decode_flush_impl(avctx, 0);
}

{

    // find a free buffer
            frame = &s->frames[i];
            break;
        }
        av_log(s->avctx, AV_LOG_FATAL, "Ran out of free frames!\n");
        abort();
    }
        vp8_release_frame(s, frame);

}

static enum AVPixelFormat get_pixel_format(VP8Context *s)
{
    enum AVPixelFormat pix_fmts[] = {
#if CONFIG_VP8_VAAPI_HWACCEL
        AV_PIX_FMT_VAAPI,
#endif
#if CONFIG_VP8_NVDEC_HWACCEL
        AV_PIX_FMT_CUDA,
#endif
        AV_PIX_FMT_YUV420P,
        AV_PIX_FMT_NONE,
    };

    return ff_get_format(s->avctx, pix_fmts);
}

static av_always_inline
{


            return ret;
    }

            return AVERROR(EINVAL);
    }


                   avctx->thread_count > 1;
                                               sizeof(*s->macroblocks));
    } else // Sliced threading
                                         sizeof(*s->macroblocks));

        free_buffers(s);
        return AVERROR(ENOMEM);
    }

            free_buffers(s);
            return AVERROR(ENOMEM);
        }
#if HAVE_THREADS
#endif
    }


}

{
}

{
}


{




    }

{


        }
    }


        }
    }

{


        return -1;

            return -1;

            return ret;
    }


}

{



{


        } else

        /* 101581>>16 is equivalent to 155/100 */

    }

/**
 * Determine which buffers golden and altref should be updated with after this frame.
 * The spec isn't clear here, so I'm going by my understanding of what libvpx does
 *
 * Intra frames update all 3 references
 * Inter frames update VP56_FRAME_PREVIOUS if the update_last flag is set
 * If the update (golden|altref) flag is set, it's updated with the current frame
 *      if update_last is set, and VP56_FRAME_PREVIOUS otherwise.
 * If the flag is not set, the number read means:
 *      0: no update
 *      1: VP56_FRAME_PREVIOUS
 *      2: update golden with altref, or update altref with golden
 */
{

        return VP56_FRAME_CURRENT;

    case 1:
        return VP56_FRAME_PREVIOUS;
    }
}

{
                   sizeof(s->prob->token[i][j]));

{

                    }

#define VP7_MVC_SIZE 17
#define VP8_MVC_SIZE 19

                                                            int mvc_size)
{


    // 17.2 MV probability update

{



static void copy_chroma(AVFrame *dst, AVFrame *src, int width, int height)
{
    int i, j;

    for (j = 1; j < 3; j++) {
        for (i = 0; i < height / 2; i++)
            memcpy(dst->data[j] + i * dst->linesize[j],
                   src->data[j] + i * src->linesize[j], width / 2);
    }
}

static void fade(uint8_t *dst, ptrdiff_t dst_linesize,
                 const uint8_t *src, ptrdiff_t src_linesize,
                 int width, int height,
                 int alpha, int beta)
{
    int i, j;
    for (j = 0; j < height; j++) {
        const uint8_t *src2 = src + j * src_linesize;
        uint8_t *dst2 = dst + j * dst_linesize;
        for (i = 0; i < width; i++) {
            uint8_t y = src2[i];
            dst2[i] = av_clip_uint8(y + ((y * beta) >> 8) + alpha);
        }
    }
}

{

        int width  = s->mb_width * 16;
        int height = s->mb_height * 16;
        AVFrame *src, *dst;

        if (!s->framep[VP56_FRAME_PREVIOUS] ||
            !s->framep[VP56_FRAME_GOLDEN]) {
            av_log(s->avctx, AV_LOG_WARNING, "Discarding interframe without a prior keyframe!\n");
            return AVERROR_INVALIDDATA;
        }

        dst =
        src = s->framep[VP56_FRAME_PREVIOUS]->tf.f;

        /* preserve the golden frame, write a new previous frame */
        if (s->framep[VP56_FRAME_GOLDEN] == s->framep[VP56_FRAME_PREVIOUS]) {
            s->framep[VP56_FRAME_PREVIOUS] = vp8_find_free_buffer(s);
            if ((ret = vp8_alloc_frame(s, s->framep[VP56_FRAME_PREVIOUS], 1)) < 0)
                return ret;

            dst = s->framep[VP56_FRAME_PREVIOUS]->tf.f;

            copy_chroma(dst, src, width, height);
        }

             src->data[0], src->linesize[0],
             width, height, alpha, beta);
    }

    return 0;
}

{

        return AVERROR_INVALIDDATA;
    }

        avpriv_request_sample(s->avctx, "Unknown profile %d", s->profile);
        return AVERROR_INVALIDDATA;
    }


        av_log(s->avctx, AV_LOG_ERROR, "Buffer size %d is too small, needed : %d\n", buf_size, 4 - s->profile + part1_size);
        return AVERROR_INVALIDDATA;
    }



        return ret;

    /* A. Dimension information (keyframes only) */
            avpriv_request_sample(s->avctx, "Upscaling");

               sizeof(s->prob->pred16x16));
               sizeof(s->prob->pred8x8c));
                   sizeof(vp7_mv_default_prob[i]));
    }


    /* B. Decoding information for all four macroblock-level features */
             s->feature_present_prob[i] = vp8_rac_get_uint(c, 8);

             for (j = 0; j < 3; j++)
                 s->feature_index_prob[i][j] =
                     vp8_rac_get(c) ? vp8_rac_get_uint(c, 8) : 255;

             if (vp7_feature_value_size[s->profile][i])
                 for (j = 0; j < 4; j++)
                     s->feature_value[i][j] =
                        vp8_rac_get(c) ? vp8_rac_get_uint(c, vp7_feature_value_size[s->profile][i]) : 0;
        }
    }


        return ret;

            return ret;
    }

    /* C. Dequantization indices */

    /* D. Golden frame update flag (a Flag) for interframes only */
    }


        s->update_probabilities = vp8_rac_get(c);
        if (!s->update_probabilities)
            s->prob[1] = s->prob[0];

        if (!s->keyframe)
            s->fade_present = vp8_rac_get(c);
    }

        return AVERROR_INVALIDDATA;
    /* E. Fading information for previous frame */
        alpha = (int8_t) vp8_rac_get_uint(c, 8);
        beta  = (int8_t) vp8_rac_get_uint(c, 8);
    }

    /* F. Loop filter type */

    /* G. DCT coefficient ordering specification */

    /* H. Loop filter levels  */
        s->filter.simple = vp8_rac_get(c);

    /* I. DCT coefficient probability update; 13.3 Token Probability Updates */


    /* J. The remaining frame header data occurs ONLY FOR INTERFRAMES */
    }

        return AVERROR_INVALIDDATA;

        return ret;

    return 0;
}

{

        av_log(s->avctx, AV_LOG_ERROR, "Insufficent data (%d) for header\n", buf_size);
        return AVERROR_INVALIDDATA;
    }



        av_log(s->avctx, AV_LOG_WARNING, "Unknown profile %d\n", s->profile);

               sizeof(s->put_pixels_tab));
    else    // profile 1-3 use bilinear, 4+ aren't defined so whatever
               sizeof(s->put_pixels_tab));

        av_log(s->avctx, AV_LOG_ERROR, "Header size larger than data provided\n");
        return AVERROR_INVALIDDATA;
    }

            av_log(s->avctx, AV_LOG_ERROR,
                   "Invalid start code 0x%x\n", AV_RL24(buf));
            return AVERROR_INVALIDDATA;
        }

            avpriv_request_sample(s->avctx, "Upscaling");

               sizeof(s->prob->pred16x16));
               sizeof(s->prob->pred8x8c));
               sizeof(s->prob->mvc));
    }

        return ret;

            av_log(s->avctx, AV_LOG_WARNING, "Unspecified colorspace\n");
    }

    else


    }

        av_log(s->avctx, AV_LOG_ERROR, "Invalid partitions\n");
        return AVERROR_INVALIDDATA;
    }

            return ret;


    }

    // if we aren't saving this frame's probabilities for future frames,
    // make a copy of the current probabilities




    }

    // Record the entropy coder state here so that hwaccels can use it.

}

static av_always_inline
{
                             av_clip(s->mv_max.x, INT16_MIN, INT16_MAX));
                             av_clip(s->mv_max.y, INT16_MIN, INT16_MAX));
}

/**
 * Motion vector coding, 17.1.
 */
{

        int i;

    } else {
        // small_mvtree
    }

}

static int vp7_read_mv_component(VP56RangeCoder *c, const uint8_t *p)
{
    return read_mv_component(c, p, 1);
}

{
}

static av_always_inline
{
        return vp7_submv_prob;

        return vp8_submv_prob[2];
}

/**
 * Split motion vector prediction, 16.4.
 * @returns the number of motion vectors parsed (2, 4 or 16)
 */
static av_always_inline
                    int layout, int is_vp7)
{

    else
        top_mb = &mb[-s->mb_width - 1];

        else
            part_idx = VP8_SPLITMVMODE_8x8;
    } else {
        part_idx = VP8_SPLITMVMODE_4x4;
    }



        else
        else


                } else {
                }
            } else {
            }
        } else {
        }
    }

}

/**
 * The vp7 reference decoder uses a padding macroblock column (added to right
 * edge of the frame) to guard against illegal macroblock offsets. The
 * algorithm has bugs that permit offsets to straddle the padding column.
 * This function replicates those bugs.
 *
 * @param[out] edge_x macroblock x address
 * @param[out] edge_y macroblock y address
 *
 * @return macroblock offset legal (boolean)
 */
static int vp7_calculate_mb_offset(int mb_x, int mb_y, int mb_width,
                                   int xoffset, int yoffset, int boundary,
                                   int *edge_x, int *edge_y)
{
    int vwidth = mb_width + 1;
    int new = (mb_y + yoffset) * vwidth + mb_x + xoffset;
    if (new < boundary || new % vwidth == vwidth - 1)
        return 0;
    *edge_y = new / vwidth;
    *edge_x = new % vwidth;
    return 1;
}

static const VP56mv *get_bmv_ptr(const VP8Macroblock *mb, int subblock)
{
    return &mb->bmv[mb->mode == VP8_MVMODE_SPLIT ? vp8_mbsplits[mb->partitioning][subblock] : 0];
}

static av_always_inline
void vp7_decode_mvs(VP8Context *s, VP8Macroblock *mb,
                    int mb_x, int mb_y, int layout)
{
    VP8Macroblock *mb_edge[12];
    enum { CNT_ZERO, CNT_NEAREST, CNT_NEAR };
    enum { VP8_EDGE_TOP, VP8_EDGE_LEFT, VP8_EDGE_TOPLEFT };
    int idx = CNT_ZERO;
    VP56mv near_mv[3];
    uint8_t cnt[3] = { 0 };
    VP56RangeCoder *c = &s->c;
    int i;

    AV_ZERO32(&near_mv[0]);
    AV_ZERO32(&near_mv[1]);
    AV_ZERO32(&near_mv[2]);

    for (i = 0; i < VP7_MV_PRED_COUNT; i++) {
        const VP7MVPred * pred = &vp7_mv_pred[i];
        int edge_x, edge_y;

        if (vp7_calculate_mb_offset(mb_x, mb_y, s->mb_width, pred->xoffset,
                                    pred->yoffset, !s->profile, &edge_x, &edge_y)) {
            VP8Macroblock *edge = mb_edge[i] = (s->mb_layout == 1)
                                             ? s->macroblocks_base + 1 + edge_x +
                                               (s->mb_width + 1) * (edge_y + 1)
                                             : s->macroblocks + edge_x +
                                               (s->mb_height - edge_y - 1) * 2;
            uint32_t mv = AV_RN32A(get_bmv_ptr(edge, vp7_mv_pred[i].subblock));
            if (mv) {
                if (AV_RN32A(&near_mv[CNT_NEAREST])) {
                    if (mv == AV_RN32A(&near_mv[CNT_NEAREST])) {
                        idx = CNT_NEAREST;
                    } else if (AV_RN32A(&near_mv[CNT_NEAR])) {
                        if (mv != AV_RN32A(&near_mv[CNT_NEAR]))
                            continue;
                        idx = CNT_NEAR;
                    } else {
                        AV_WN32A(&near_mv[CNT_NEAR], mv);
                        idx = CNT_NEAR;
                    }
                } else {
                    AV_WN32A(&near_mv[CNT_NEAREST], mv);
                    idx = CNT_NEAREST;
                }
            } else {
                idx = CNT_ZERO;
            }
        } else {
            idx = CNT_ZERO;
        }
        cnt[idx] += vp7_mv_pred[i].score;
    }

    mb->partitioning = VP8_SPLITMVMODE_NONE;

    if (vp56_rac_get_prob_branchy(c, vp7_mode_contexts[cnt[CNT_ZERO]][0])) {
        mb->mode = VP8_MVMODE_MV;

        if (vp56_rac_get_prob_branchy(c, vp7_mode_contexts[cnt[CNT_NEAREST]][1])) {

            if (vp56_rac_get_prob_branchy(c, vp7_mode_contexts[cnt[CNT_NEAR]][2])) {

                if (cnt[CNT_NEAREST] > cnt[CNT_NEAR])
                    AV_WN32A(&mb->mv, cnt[CNT_ZERO] > cnt[CNT_NEAREST] ? 0 : AV_RN32A(&near_mv[CNT_NEAREST]));
                else
                    AV_WN32A(&mb->mv, cnt[CNT_ZERO] > cnt[CNT_NEAR]    ? 0 : AV_RN32A(&near_mv[CNT_NEAR]));

                if (vp56_rac_get_prob_branchy(c, vp7_mode_contexts[cnt[CNT_NEAR]][3])) {
                    mb->mode = VP8_MVMODE_SPLIT;
                    mb->mv = mb->bmv[decode_splitmvs(s, c, mb, layout, IS_VP7) - 1];
                } else {
                    mb->mv.y += vp7_read_mv_component(c, s->prob->mvc[0]);
                    mb->mv.x += vp7_read_mv_component(c, s->prob->mvc[1]);
                    mb->bmv[0] = mb->mv;
                }
            } else {
                mb->mv = near_mv[CNT_NEAR];
                mb->bmv[0] = mb->mv;
            }
        } else {
            mb->mv = near_mv[CNT_NEAREST];
            mb->bmv[0] = mb->mv;
        }
    } else {
        mb->mode = VP8_MVMODE_ZERO;
        AV_ZERO32(&mb->mv);
        mb->bmv[0] = mb->mv;
    }
}

static av_always_inline
                    int mb_x, int mb_y, int layout)
{
                                  0      /* top-left */ };

    } else {
        mb_edge[0] = mb - s->mb_width - 1;
        mb_edge[2] = mb - s->mb_width - 2;
    }


    /* Process MB on top, left and top-left */
#define MV_EDGE_CHECK(n)                                                      \
    {                                                                         \
        VP8Macroblock *edge = mb_edge[n];                                     \
        int edge_ref = edge->ref_frame;                                       \
        if (edge_ref != VP56_FRAME_CURRENT) {                                 \
            uint32_t mv = AV_RN32A(&edge->mv);                                \
            if (mv) {                                                         \
                if (cur_sign_bias != sign_bias[edge_ref]) {                   \
                    /* SWAR negate of the values in mv. */                    \
                    mv = ~mv;                                                 \
                    mv = ((mv & 0x7fff7fff) +                                 \
                          0x00010001) ^ (mv & 0x80008000);                    \
                }                                                             \
                if (!n || mv != AV_RN32A(&near_mv[idx]))                      \
                    AV_WN32A(&near_mv[++idx], mv);                            \
                cnt[idx] += 1 + (n != 2);                                     \
            } else                                                            \
                cnt[CNT_ZERO] += 1 + (n != 2);                                \
        }                                                                     \
    }



        /* If we have three distinct MVs, merge first and last if they're the same */

        /* Swap near and nearest if necessary */
        }

                /* Choose the best mv out of 0,0 and the nearest mv */

                } else {
                }
            } else {
            }
        } else {
        }
    } else {
    }

static av_always_inline
                           int mb_x, int keyframe, int layout)
{

        VP8Macroblock *mb_top = mb - s->mb_width - 1;
    }
            top = mb->intra4x4_pred_mode_top;
        else
            }
        }
    } else {
        int i;
                                           vp8_pred4x4_prob_inter);
    }
}

static av_always_inline
                    VP8Macroblock *mb, int mb_x, int mb_y,
                    uint8_t *segment, uint8_t *ref, int layout, int is_vp7)
{
                                                     "lf-delta",
                                                     "partial-golden-update",
                                                     "blit-pitch" };
        int i;
        *segment = 0;
        for (i = 0; i < 4; i++) {
            if (s->feature_enabled[i]) {
                if (vp56_rac_get_prob_branchy(c, s->feature_present_prob[i])) {
                      int index = vp8_rac_get_tree(c, vp7_feature_index_tree,
                                                   s->feature_index_prob[i]);
                      av_log(s->avctx, AV_LOG_WARNING,
                             "Feature %s present in macroblock (value 0x%x)\n",
                             vp7_feature_name[i], s->feature_value[i][index]);
                }
           }
        }


                                    vp8_pred16x16_prob_intra);

        } else {
                AV_WN32A(mb->intra4x4_pred_mode_top, modes);
            else
        }

                                                vp8_pred8x8c_prob_intra);
        // inter MB, 16.2
                                                                   : VP56_FRAME_GOLDEN;
        else

        // motion vectors, 16.3
            vp7_decode_mvs(s, mb, mb_x, mb_y, layout);
        else
    } else {
        // intra MB, 16.1


    }
}

/**
 * @param r     arithmetic bitstream reader context
 * @param block destination for block coefficients
 * @param probs probabilities to use when reading trees from the bitstream
 * @param i     initial coeff index, 0 unless a separate DC block is coded
 * @param qmul  array holding the dc/ac dequant factor at position 0/1
 *
 * @return 0 if no coeffs were decoded
 *         otherwise, the index of the last coeff decoded plus one
 */
static av_always_inline
                                 uint8_t probs[16][3][NUM_DCT_TOKENS - 1],
                                 int i, uint8_t *token_prob, int16_t qmul[2],
                                 const uint8_t scan[16], int vp7)
{
            break;

skip_eob:
                break; // invalid input; blocks should end with EOB
        }

        } else {
            } else {
                // DCT_CAT*
                    } else {                                    // DCT_CAT2
                    }
                } else {    // DCT_CAT3 and up
                }
            }
        }

}

static av_always_inline
{

        dc += pred[0];
        ret = 1;
    }

    } else {
        if (pred[0] == dc)
            pred[1]++;
        block[0] = pred[0] = dc;
    }

}

                                            int16_t block[16],
                                            uint8_t probs[16][3][NUM_DCT_TOKENS - 1],
                                            int i, uint8_t *token_prob,
                                            int16_t qmul[2],
                                            const uint8_t scan[16])
{
                                        token_prob, qmul, scan, IS_VP7);
}

#ifndef vp8_decode_block_coeffs_internal
                                            int16_t block[16],
                                            uint8_t probs[16][3][NUM_DCT_TOKENS - 1],
                                            int i, uint8_t *token_prob,
                                            int16_t qmul[2])
{
                                        token_prob, qmul, ff_zigzag_scan, IS_VP8);
}
#endif

/**
 * @param c          arithmetic bitstream reader context
 * @param block      destination for block coefficients
 * @param probs      probabilities to use when reading trees from the bitstream
 * @param i          initial coeff index, 0 unless a separate DC block is coded
 * @param zero_nhood the initial prediction context for number of surrounding
 *                   all-zero blocks (only left/top, so 0-2)
 * @param qmul       array holding the dc/ac dequant factor at position 0/1
 * @param scan       scan pattern (VP7 only)
 *
 * @return 0 if no coeffs were decoded
 *         otherwise, the index of the last coeff decoded plus one
 */
static av_always_inline
                        uint8_t probs[16][3][NUM_DCT_TOKENS - 1],
                        int i, int zero_nhood, int16_t qmul[2],
                        const uint8_t scan[16], int vp7)
{
                                                  token_prob, qmul, scan)
                                                  token_prob, qmul);
}

static av_always_inline
                      VP8Macroblock *mb, uint8_t t_nnz[9], uint8_t l_nnz[9],
                      int is_vp7)
{


        // decode DC values and do hadamard
                                  ff_zigzag_scan, is_vp7);

        }

            else
        }
        luma_start = 1;
        luma_ctx   = 0;
    }

    // luma blocks
                                      luma_start, nnz_pred,
            /* nnz+block_dc may be one more than the actual last index,
             * but we don't care */
        }

    // chroma blocks
    // TODO: what to do about dimensions? 2nd dim for luma is x,
    // but for chroma it's (y<<1)|x
            }

    // if there were no coded coeffs despite the macroblock not being marked skip,
    // we MUST not do the inner loop filter and should not do IDCT
    // Since skip isn't used for bitstream prediction, just manually set it.
}

static av_always_inline
                      uint8_t *src_cb, uint8_t *src_cr,
                      ptrdiff_t linesize, ptrdiff_t uvlinesize, int simple)
{
    }
}

static av_always_inline
                    uint8_t *src_cr, ptrdiff_t linesize, ptrdiff_t uvlinesize, int mb_x,
                    int mb_y, int mb_width, int simple, int xchg)
{

#define XCHG(a, b, xchg)                                                      \
    do {                                                                      \
        if (xchg)                                                             \
            AV_SWAP64(b, a);                                                  \
        else                                                                  \
            AV_COPY64(b, a);                                                  \
    } while (0)


    // only copy chroma for normal loop filter
    // or to initialize the top row to 127
    }
}

static av_always_inline
{
    else
}

static av_always_inline
{
    else
}

static av_always_inline
{
    case DC_PRED8x8:
    case PLANE_PRED8x8: /* TM */
    }
    return mode;
}

static av_always_inline
{
    } else {
    }
}

static av_always_inline
                                     int *copy_buf, int vp7)
{
            *copy_buf = 1;
            return mode;
        }
        /* fall-through */
    case DIAG_DOWN_LEFT_PRED:
    case VERT_LEFT_PRED:
            *copy_buf = 1;
            return mode;
        }
        /* fall-through */
    case HOR_UP_PRED:
    case TM_VP8_PRED:
                   * as 16x16/8x8 DC */
    case DIAG_DOWN_RIGHT_PRED:
    case VERT_RIGHT_PRED:
    case HOR_DOWN_PRED:
            *copy_buf = 1;
        return mode;
    }
    return mode;
}

static av_always_inline
                   VP8Macroblock *mb, int mb_x, int mb_y, int is_vp7)
{

    /* for the first row, we need to run xchg_mb_border to init the top edge
     * to 127 otherwise, skip it if we aren't going to deblock */

    } else {

        // all blocks on the right edge of the macroblock use bottom edge
        // the top macroblock for their topright edge

        // if we're on the right edge of the frame, said edge is extended
        // from the top macroblock
        }



                    topright = tr_top;

                                                        mb_y + y, &copy, is_vp7);
                if (copy) {
                    } else {
                        } else {
                            copy_dst[3] = ptr[4 * x - s->linesize - 1];
                        }
                    }
                    } else {
                    }
                }
                }

                    else
                }
            }

        }
    }

                                            mb_x, mb_y, is_vp7);


static const uint8_t subpel_idx[3][8] = {
    { 0, 1, 2, 1, 2, 1, 2, 1 }, // nr. of left extra pixels,
                                // also function pointer index
    { 0, 3, 5, 3, 5, 3, 5, 3 }, // nr. of extra pixels required
    { 0, 2, 3, 2, 3, 2, 3, 2 }, // nr. of right extra pixels
};

/**
 * luma MC function
 *
 * @param s        VP8 decoding context
 * @param dst      target buffer for block data at block position
 * @param ref      reference picture buffer at origin (0, 0)
 * @param mv       motion vector (relative to block position) to get pixel data from
 * @param x_off    horizontal position of block from origin (0, 0)
 * @param y_off    vertical position of block from origin (0, 0)
 * @param block_w  width of block (16, 8 or 4)
 * @param block_h  height of block (always same as block_w)
 * @param width    width of src/dst plane data
 * @param height   height of src/dst plane data
 * @param linesize size of a single line of plane data, including padding
 * @param mc_func  motion compensation function pointers (bilinear or sixtap MC)
 */
static av_always_inline
                 ThreadFrame *ref, const VP56mv *mv,
                 int x_off, int y_off, int block_w, int block_h,
                 int width, int height, ptrdiff_t linesize,
                 vp8_mc_func mc_func[3][3])
{




        // edge emulation
                                     EDGE_EMU_LINESIZE, linesize,
                                     x_off - mx_idx, y_off - my_idx,
                                     width, height);
        }
    } else {
                      linesize, block_h, 0, 0);
    }
}

/**
 * chroma MC function
 *
 * @param s        VP8 decoding context
 * @param dst1     target buffer for block data at block position (U plane)
 * @param dst2     target buffer for block data at block position (V plane)
 * @param ref      reference picture buffer at origin (0, 0)
 * @param mv       motion vector (relative to block position) to get pixel data from
 * @param x_off    horizontal position of block from origin (0, 0)
 * @param y_off    vertical position of block from origin (0, 0)
 * @param block_w  width of block (16, 8 or 4)
 * @param block_h  height of block (always same as block_w)
 * @param width    width of src/dst plane data
 * @param height   height of src/dst plane data
 * @param linesize size of a single line of plane data, including padding
 * @param mc_func  motion compensation function pointers (bilinear or sixtap MC)
 */
static av_always_inline
                   uint8_t *dst2, ThreadFrame *ref, const VP56mv *mv,
                   int x_off, int y_off, int block_w, int block_h,
                   int width, int height, ptrdiff_t linesize,
                   vp8_mc_func mc_func[3][3])
{



        // edge emulation
                                     EDGE_EMU_LINESIZE, linesize,
                                     x_off - mx_idx, y_off - my_idx, width, height);

                                     EDGE_EMU_LINESIZE, linesize,
                                     block_w + subpel_idx[1][mx],
                                     block_h + subpel_idx[1][my],
                                     x_off - mx_idx, y_off - my_idx, width, height);
        } else {
        }
    } else {
    }
}

static av_always_inline
                 ThreadFrame *ref_frame, int x_off, int y_off,
                 int bx_off, int by_off, int block_w, int block_h,
                 int width, int height, VP56mv *mv)
{

    /* Y */
                ref_frame, mv, x_off + bx_off, y_off + by_off,
                block_w, block_h, width, height, s->linesize,

    /* U/V */
        /* this block only applies VP8; it is safe to check
         * only the profile, as VP7 profile <= 1 */
    }
                  &uvmv, x_off + bx_off, y_off + by_off,
                  block_w, block_h, width, height, s->uvlinesize,

/* Fetch pixels for estimated mv 4 macroblocks ahead.
 * Optimized for 64-byte cache lines. Inspired by ffh264 prefetch_motion. */
static av_always_inline
                     int mb_xy, int ref)
{
    /* Don't prefetch refs that haven't been used very often this frame. */
        /* For threading, a ff_thread_await_progress here might be useful, but
         * it actually slows down the decoder. Since a bad prefetch doesn't
         * generate bad decoder output, we don't run it here. */
    }
}

/**
 * Apply motion vectors to prediction buffer, chapter 18.
 */
static av_always_inline
                   VP8Macroblock *mb, int mb_x, int mb_y)
{

                    0, 0, 16, 16, width, height, &mb->mv);
        break;
    case VP8_SPLITMVMODE_4x4: {
        int x, y;
        VP56mv uvmv;

        /* Y */
                            width, height, s->linesize,
            }
        }

        /* U/V */
                }
                              width, height, s->uvlinesize,
            }
        }
    }
    case VP8_SPLITMVMODE_16x8:
                    0, 0, 16, 8, width, height, &bmv[0]);
                    0, 8, 16, 8, width, height, &bmv[1]);
        break;
    case VP8_SPLITMVMODE_8x16:
                    0, 0, 8, 16, width, height, &bmv[0]);
                    8, 0, 8, 16, width, height, &bmv[1]);
        break;
    case VP8_SPLITMVMODE_8x8:
                    0, 0, 8, 8, width, height, &bmv[0]);
                    8, 0, 8, 8, width, height, &bmv[1]);
                    0, 8, 8, 8, width, height, &bmv[2]);
                    8, 8, 8, 8, width, height, &bmv[3]);
        break;
    }
}

static av_always_inline
{

                                                      s->linesize);
                                                   s->linesize);
                            break;
                    }
                } else {
                }
            }
        }
    }

                                                      s->uvlinesize);
                                                   s->uvlinesize);
                    }
                }
            } else {
            }
        }
    }
}

static av_always_inline
                         VP8FilterStrength *f, int is_vp7)
{

    } else

    }


    }

                      mb->mode == VP8_MVMODE_SPLIT;

static av_always_inline
               int mb_x, int mb_y, int is_vp7)
{
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
          2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
          3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
          3, 3, 3, 3 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
          2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
          2, 2, 2, 2 }
    };

        return;

    } else {
    }


                                       mbedge_lim, inner_limit, hev_thresh);
                                       mbedge_lim, inner_limit, hev_thresh);
    }

#define H_LOOP_FILTER_16Y_INNER(cond)                                         \
    if (cond && inner_filter) {                                               \
        s->vp8dsp.vp8_h_loop_filter16y_inner(dst[0] +  4, linesize,           \
                                             bedge_lim_y, inner_limit,        \
                                             hev_thresh);                     \
        s->vp8dsp.vp8_h_loop_filter16y_inner(dst[0] +  8, linesize,           \
                                             bedge_lim_y, inner_limit,        \
                                             hev_thresh);                     \
        s->vp8dsp.vp8_h_loop_filter16y_inner(dst[0] + 12, linesize,           \
                                             bedge_lim_y, inner_limit,        \
                                             hev_thresh);                     \
        s->vp8dsp.vp8_h_loop_filter8uv_inner(dst[1] +  4, dst[2] + 4,         \
                                             uvlinesize,  bedge_lim_uv,       \
                                             inner_limit, hev_thresh);        \
    }


                                       mbedge_lim, inner_limit, hev_thresh);
                                       mbedge_lim, inner_limit, hev_thresh);
    }

                                             linesize, bedge_lim_y,
                                             inner_limit, hev_thresh);
                                             linesize, bedge_lim_y,
                                             inner_limit, hev_thresh);
                                             linesize, bedge_lim_y,
                                             inner_limit, hev_thresh);
                                             uvlinesize, bedge_lim_uv,
                                             inner_limit, hev_thresh);
    }

}

static av_always_inline
                      int mb_x, int mb_y)
{

        return;


    }

    }
}

#define MARGIN (16 << 2)
static av_always_inline
int vp78_decode_mv_mb_modes(AVCodecContext *avctx, VP8Frame *curframe,
                                    VP8Frame *prev_frame, int is_vp7)
{
    VP8Context *s = avctx->priv_data;
    int mb_x, mb_y;

    s->mv_bounds.mv_min.y = -MARGIN;
    s->mv_bounds.mv_max.y = ((s->mb_height - 1) << 6) + MARGIN;
    for (mb_y = 0; mb_y < s->mb_height; mb_y++) {
        VP8Macroblock *mb = s->macroblocks_base +
                            ((s->mb_width + 1) * (mb_y + 1) + 1);
        int mb_xy = mb_y * s->mb_width;

        AV_WN32A(s->intra4x4_pred_mode_left, DC_PRED * 0x01010101);

        s->mv_bounds.mv_min.x = -MARGIN;
        s->mv_bounds.mv_max.x = ((s->mb_width - 1) << 6) + MARGIN;

        if (vpX_rac_is_end(&s->c)) {
            return AVERROR_INVALIDDATA;
        }
        for (mb_x = 0; mb_x < s->mb_width; mb_x++, mb_xy++, mb++) {
            if (mb_y == 0)
                AV_WN32A((mb - s->mb_width - 1)->intra4x4_pred_mode_top,
                         DC_PRED * 0x01010101);
            decode_mb_mode(s, &s->mv_bounds, mb, mb_x, mb_y, curframe->seg_map->data + mb_xy,
                           prev_frame && prev_frame->seg_map ?
                           prev_frame->seg_map->data + mb_xy : NULL, 1, is_vp7);
            s->mv_bounds.mv_min.x -= 64;
            s->mv_bounds.mv_max.x -= 64;
        }
        s->mv_bounds.mv_min.y -= 64;
        s->mv_bounds.mv_max.y -= 64;
    }
    return 0;
}

static int vp7_decode_mv_mb_modes(AVCodecContext *avctx, VP8Frame *cur_frame,
                                   VP8Frame *prev_frame)
{
    return vp78_decode_mv_mb_modes(avctx, cur_frame, prev_frame, IS_VP7);
}

static int vp8_decode_mv_mb_modes(AVCodecContext *avctx, VP8Frame *cur_frame,
                                   VP8Frame *prev_frame)
{
    return vp78_decode_mv_mb_modes(avctx, cur_frame, prev_frame, IS_VP8);
}

#if HAVE_THREADS
#define check_thread_pos(td, otd, mb_x_check, mb_y_check)                     \
    do {                                                                      \
        int tmp = (mb_y_check << 16) | (mb_x_check & 0xFFFF);                 \
        if (atomic_load(&otd->thread_mb_pos) < tmp) {                         \
            pthread_mutex_lock(&otd->lock);                                   \
            atomic_store(&td->wait_mb_pos, tmp);                              \
            do {                                                              \
                if (atomic_load(&otd->thread_mb_pos) >= tmp)                  \
                    break;                                                    \
                pthread_cond_wait(&otd->cond, &otd->lock);                    \
            } while (1);                                                      \
            atomic_store(&td->wait_mb_pos, INT_MAX);                          \
            pthread_mutex_unlock(&otd->lock);                                 \
        }                                                                     \
    } while (0)

#define update_pos(td, mb_y, mb_x)                                            \
    do {                                                                      \
        int pos              = (mb_y << 16) | (mb_x & 0xFFFF);                \
        int sliced_threading = (avctx->active_thread_type == FF_THREAD_SLICE) && \
                               (num_jobs > 1);                                \
        int is_null          = !next_td || !prev_td;                          \
        int pos_check        = (is_null) ? 1 :                                \
            (next_td != td && pos >= atomic_load(&next_td->wait_mb_pos)) ||   \
            (prev_td != td && pos >= atomic_load(&prev_td->wait_mb_pos));     \
        atomic_store(&td->thread_mb_pos, pos);                                \
        if (sliced_threading && pos_check) {                                  \
            pthread_mutex_lock(&td->lock);                                    \
            pthread_cond_broadcast(&td->cond);                                \
            pthread_mutex_unlock(&td->lock);                                  \
        }                                                                     \
    } while (0)
#else
#define check_thread_pos(td, otd, mb_x_check, mb_y_check) while(0)
#define update_pos(td, mb_y, mb_x) while(0)
#endif

                                        int jobnr, int threadnr, int is_vp7)
{
    };

         return AVERROR_INVALIDDATA;

        prev_td = td;
    else
        next_td = td;
    else
    else {
        // Make sure the previous frame has read its segmentation map,
        // if we re-use the same map.
    }



            return AVERROR_INVALIDDATA;
        // Wait for previous thread to read mb_x+2, and reach mb_y-1.
            if (threadnr != 0) {
                check_thread_pos(td, prev_td,
                                 mb_x + (is_vp7 ? 2 : 1),
                                 mb_y - (is_vp7 ? 2 : 1));
            } else {
                check_thread_pos(td, prev_td,
                                 mb_x + (is_vp7 ? 2 : 1) + s->mb_width + 3,
                                 mb_y - (is_vp7 ? 2 : 1));
            }
        }

                         s->linesize, 4);




        else


        } else {

            /* Reset DC block predictors if they would exist
             * if the mb had coefficients */
            }
        }


            if (s->filter.simple)
                backup_mb_border(s->top_border[mb_x + 1], dst[0],
                                 NULL, NULL, s->linesize, 0, 1);
            else
                backup_mb_border(s->top_border[mb_x + 1], dst[0],
                                 dst[1], dst[2], s->linesize, s->uvlinesize, 0);
        }



            update_pos(td, mb_y, s->mb_width + 3);
        } else {
        }
    }
    return 0;
}

                                        int jobnr, int threadnr)
{
}

                                        int jobnr, int threadnr)
{
}

                              int jobnr, int threadnr, int is_vp7)
{
    };

        mb = s->macroblocks_base + ((s->mb_width + 1) * (mb_y + 1) + 1);
    else

        prev_td = td;
    else
        next_td = td;
    else

            check_thread_pos(td, prev_td,
                             (mb_x + 1) + (s->mb_width + 3), mb_y - 1);
            if (next_td != &s->thread_data[0])
                check_thread_pos(td, next_td, mb_x + 1, mb_y + 1);

                                 NULL, NULL, s->linesize, 0, 1);
            else
                                 dst[1], dst[2], s->linesize, s->uvlinesize, 0);
        }

        else

    }
}

                              int jobnr, int threadnr)
{

                              int jobnr, int threadnr)
{

static av_always_inline
                              int threadnr, int is_vp7)
{

            update_pos(td, s->mb_height, INT_MAX & 0xFFFF);
            return ret;
        }


            ff_thread_report_progress(&curframe->tf, mb_y, 0);
    }

    return 0;
}

                                    int jobnr, int threadnr)
{
}

                                    int jobnr, int threadnr)
{
}

static av_always_inline
                      AVPacket *avpkt, int is_vp7)
{

    else

        goto err;

        // avctx->pix_fmt already set in caller.
        s->pix_fmt = get_pixel_format(s);
        if (s->pix_fmt < 0) {
            ret = AVERROR(EINVAL);
            goto err;
        }
        avctx->pix_fmt = s->pix_fmt;
    }



                                             : AVDISCARD_ALL;

        s->invisible = 1;
        memcpy(&s->next_framep[0], &s->framep[0], sizeof(s->framep[0]) * 4);
        goto skip_decode;
    }

    // release no longer referenced frames


        avctx->color_range = AVCOL_RANGE_JPEG;
    else

    /* Given that arithmetic probabilities are updated every frame, it's quite
     * likely that the values we have on a random interframe are complete
     * junk if we didn't start decode on a keyframe. So just don't display
     * anything rather than junk. */
        av_log(avctx, AV_LOG_WARNING,
               "Discarding interframe without a prior keyframe!\n");
        ret = AVERROR_INVALIDDATA;
        goto err;
    }

        goto err;

    // check if golden and altref are swapped
    else

    else

    else



        ret = avctx->hwaccel->start_frame(avctx, avpkt->data, avpkt->size);
        if (ret < 0)
            goto err;

        ret = avctx->hwaccel->decode_slice(avctx, avpkt->data, avpkt->size);
        if (ret < 0)
            goto err;

        ret = avctx->hwaccel->end_frame(avctx);
        if (ret < 0)
            goto err;

    } else {

        /* Zero macroblock structures for top/top-left prediction
         * from outside the frame. */


            // Make sure the previous frame has read its segmentation map,
            // if we re-use the same map.
                !s->segmentation.update_map)
                ff_thread_await_progress(&prev_frame->tf, 1, 0);
            else
                ret = vp8_decode_mv_mb_modes(avctx, curframe, prev_frame);
                goto err;
        }

            num_jobs = 1;
        else
        }
                            num_jobs);
        else
                            num_jobs);
    }


    // if future frames don't use the updated probabilities,
    // reset them to the values we saved

            return ret;
    }

err:
    memcpy(&s->next_framep[0], &s->framep[0], sizeof(s->framep[0]) * 4);
    return ret;
}

                        AVPacket *avpkt)
{
}

#if CONFIG_VP7_DECODER
                            AVPacket *avpkt)
{
}
#endif /* CONFIG_VP7_DECODER */

{

        return 0;


    return 0;
}

{
            return AVERROR(ENOMEM);
    }
    return 0;
}

static av_always_inline
{



    }

    /* does not change for VP8 */

        ff_vp8_decode_free(avctx);
        return ret;
    }

    return 0;
}

#if CONFIG_VP7_DECODER
{
}
#endif /* CONFIG_VP7_DECODER */

{
}

#if CONFIG_VP8_DECODER
#if HAVE_THREADS
#define REBASE(pic) ((pic) ? (pic) - &s_src->frames[0] + &s->frames[0] : NULL)

static int vp8_decode_update_thread_context(AVCodecContext *dst,
                                            const AVCodecContext *src)
{
    VP8Context *s = dst->priv_data, *s_src = src->priv_data;
    int i;

    if (s->macroblocks_base &&
        (s_src->mb_width != s->mb_width || s_src->mb_height != s->mb_height)) {
        free_buffers(s);
        s->mb_width  = s_src->mb_width;
        s->mb_height = s_src->mb_height;
    }

    s->pix_fmt      = s_src->pix_fmt;
    s->prob[0]      = s_src->prob[!s_src->update_probabilities];
    s->segmentation = s_src->segmentation;
    s->lf_delta     = s_src->lf_delta;
    memcpy(s->sign_bias, s_src->sign_bias, sizeof(s->sign_bias));

    for (i = 0; i < FF_ARRAY_ELEMS(s_src->frames); i++) {
        if (s_src->frames[i].tf.f->buf[0]) {
            int ret = vp8_ref_frame(s, &s->frames[i], &s_src->frames[i]);
            if (ret < 0)
                return ret;
        }
    }

    s->framep[0] = REBASE(s_src->next_framep[0]);
    s->framep[1] = REBASE(s_src->next_framep[1]);
    s->framep[2] = REBASE(s_src->next_framep[2]);
    s->framep[3] = REBASE(s_src->next_framep[3]);

    return 0;
}
#endif /* HAVE_THREADS */
#endif /* CONFIG_VP8_DECODER */

#if CONFIG_VP7_DECODER
AVCodec ff_vp7_decoder = {
    .name                  = "vp7",
    .long_name             = NULL_IF_CONFIG_SMALL("On2 VP7"),
    .type                  = AVMEDIA_TYPE_VIDEO,
    .id                    = AV_CODEC_ID_VP7,
    .priv_data_size        = sizeof(VP8Context),
    .init                  = vp7_decode_init,
    .close                 = ff_vp8_decode_free,
    .decode                = vp7_decode_frame,
    .capabilities          = AV_CODEC_CAP_DR1,
    .flush                 = vp8_decode_flush,
};
#endif /* CONFIG_VP7_DECODER */

#if CONFIG_VP8_DECODER
AVCodec ff_vp8_decoder = {
    .name                  = "vp8",
    .long_name             = NULL_IF_CONFIG_SMALL("On2 VP8"),
    .type                  = AVMEDIA_TYPE_VIDEO,
    .id                    = AV_CODEC_ID_VP8,
    .priv_data_size        = sizeof(VP8Context),
    .init                  = ff_vp8_decode_init,
    .close                 = ff_vp8_decode_free,
    .decode                = ff_vp8_decode_frame,
    .capabilities          = AV_CODEC_CAP_DR1 | AV_CODEC_CAP_FRAME_THREADS |
                             AV_CODEC_CAP_SLICE_THREADS,
    .flush                 = vp8_decode_flush,
    .update_thread_context = ONLY_IF_THREADS_ENABLED(vp8_decode_update_thread_context),
    .hw_configs            = (const AVCodecHWConfigInternal*[]) {
#if CONFIG_VP8_VAAPI_HWACCEL
                               HWACCEL_VAAPI(vp8),
#endif
#if CONFIG_VP8_NVDEC_HWACCEL
                               HWACCEL_NVDEC(vp8),
#endif
                               NULL
                           },
    .caps_internal         = FF_CODEC_CAP_ALLOCATE_PROGRESS,
};
#endif /* CONFIG_VP7_DECODER */
