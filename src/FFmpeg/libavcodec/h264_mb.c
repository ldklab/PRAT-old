/*
 * H.26L/H.264/AVC/JVT/14496-10/... decoder
 * Copyright (c) 2003 Michael Niedermayer <michaelni@gmx.at>
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
 * H.264 / AVC / MPEG-4 part10 macroblock decoding
 */

#include <stdint.h>

#include "config.h"

#include "libavutil/common.h"
#include "libavutil/intreadwrite.h"
#include "avcodec.h"
#include "h264dec.h"
#include "h264_ps.h"
#include "qpeldsp.h"
#include "thread.h"

static inline int get_lowest_part_list_y(H264SliceContext *sl,
                                         int n, int height, int y_offset, int list)
{
    int raw_my             = sl->mv_cache[list][scan8[n]][1];
    int filter_height_down = (raw_my & 3) ? 3 : 0;
    int full_my            = (raw_my >> 2) + y_offset;
    int bottom             = full_my + filter_height_down + height;

    av_assert2(height >= 0);

    return FFMAX(0, bottom);
}

static inline void get_lowest_part_y(const H264Context *h, H264SliceContext *sl,
                                     int16_t refs[2][48], int n,
                                     int height, int y_offset, int list0,
                                     int list1, int *nrefs)
{
    int my;

    y_offset += 16 * (sl->mb_y >> MB_FIELD(sl));

    if (list0) {
        int ref_n = sl->ref_cache[0][scan8[n]];
        H264Ref *ref = &sl->ref_list[0][ref_n];

        // Error resilience puts the current picture in the ref list.
        // Don't try to wait on these as it will cause a deadlock.
        // Fields can wait on each other, though.
        if (ref->parent->tf.progress->data != h->cur_pic.tf.progress->data ||
            (ref->reference & 3) != h->picture_structure) {
            my = get_lowest_part_list_y(sl, n, height, y_offset, 0);
            if (refs[0][ref_n] < 0)
                nrefs[0] += 1;
            refs[0][ref_n] = FFMAX(refs[0][ref_n], my);
        }
    }

    if (list1) {
        int ref_n    = sl->ref_cache[1][scan8[n]];
        H264Ref *ref = &sl->ref_list[1][ref_n];

        if (ref->parent->tf.progress->data != h->cur_pic.tf.progress->data ||
            (ref->reference & 3) != h->picture_structure) {
            my = get_lowest_part_list_y(sl, n, height, y_offset, 1);
            if (refs[1][ref_n] < 0)
                nrefs[1] += 1;
            refs[1][ref_n] = FFMAX(refs[1][ref_n], my);
        }
    }
}

/**
 * Wait until all reference frames are available for MC operations.
 *
 * @param h the H.264 context
 */
static void await_references(const H264Context *h, H264SliceContext *sl)
{
    const int mb_xy   = sl->mb_xy;
    const int mb_type = h->cur_pic.mb_type[mb_xy];
    int16_t refs[2][48];
    int nrefs[2] = { 0 };
    int ref, list;

    memset(refs, -1, sizeof(refs));

    if (IS_16X16(mb_type)) {
        get_lowest_part_y(h, sl, refs, 0, 16, 0,
                          IS_DIR(mb_type, 0, 0), IS_DIR(mb_type, 0, 1), nrefs);
    } else if (IS_16X8(mb_type)) {
        get_lowest_part_y(h, sl, refs, 0, 8, 0,
                          IS_DIR(mb_type, 0, 0), IS_DIR(mb_type, 0, 1), nrefs);
        get_lowest_part_y(h, sl, refs, 8, 8, 8,
                          IS_DIR(mb_type, 1, 0), IS_DIR(mb_type, 1, 1), nrefs);
    } else if (IS_8X16(mb_type)) {
        get_lowest_part_y(h, sl, refs, 0, 16, 0,
                          IS_DIR(mb_type, 0, 0), IS_DIR(mb_type, 0, 1), nrefs);
        get_lowest_part_y(h, sl, refs, 4, 16, 0,
                          IS_DIR(mb_type, 1, 0), IS_DIR(mb_type, 1, 1), nrefs);
    } else {
        int i;

        av_assert2(IS_8X8(mb_type));

        for (i = 0; i < 4; i++) {
            const int sub_mb_type = sl->sub_mb_type[i];
            const int n           = 4 * i;
            int y_offset          = (i & 2) << 2;

            if (IS_SUB_8X8(sub_mb_type)) {
                get_lowest_part_y(h, sl, refs, n, 8, y_offset,
                                  IS_DIR(sub_mb_type, 0, 0),
                                  IS_DIR(sub_mb_type, 0, 1),
                                  nrefs);
            } else if (IS_SUB_8X4(sub_mb_type)) {
                get_lowest_part_y(h, sl, refs, n, 4, y_offset,
                                  IS_DIR(sub_mb_type, 0, 0),
                                  IS_DIR(sub_mb_type, 0, 1),
                                  nrefs);
                get_lowest_part_y(h, sl, refs, n + 2, 4, y_offset + 4,
                                  IS_DIR(sub_mb_type, 0, 0),
                                  IS_DIR(sub_mb_type, 0, 1),
                                  nrefs);
            } else if (IS_SUB_4X8(sub_mb_type)) {
                get_lowest_part_y(h, sl, refs, n, 8, y_offset,
                                  IS_DIR(sub_mb_type, 0, 0),
                                  IS_DIR(sub_mb_type, 0, 1),
                                  nrefs);
                get_lowest_part_y(h, sl, refs, n + 1, 8, y_offset,
                                  IS_DIR(sub_mb_type, 0, 0),
                                  IS_DIR(sub_mb_type, 0, 1),
                                  nrefs);
            } else {
                int j;
                av_assert2(IS_SUB_4X4(sub_mb_type));
                for (j = 0; j < 4; j++) {
                    int sub_y_offset = y_offset + 2 * (j & 2);
                    get_lowest_part_y(h, sl, refs, n + j, 4, sub_y_offset,
                                      IS_DIR(sub_mb_type, 0, 0),
                                      IS_DIR(sub_mb_type, 0, 1),
                                      nrefs);
                }
            }
        }
    }

    for (list = sl->list_count - 1; list >= 0; list--)
        for (ref = 0; ref < 48 && nrefs[list]; ref++) {
            int row = refs[list][ref];
            if (row >= 0) {
                H264Ref *ref_pic  = &sl->ref_list[list][ref];
                int ref_field         = ref_pic->reference - 1;
                int ref_field_picture = ref_pic->parent->field_picture;
                int pic_height        = 16 * h->mb_height >> ref_field_picture;

                row <<= MB_MBAFF(sl);
                nrefs[list]--;

                if (!FIELD_PICTURE(h) && ref_field_picture) { // frame referencing two fields
                    av_assert2((ref_pic->parent->reference & 3) == 3);
                    ff_thread_await_progress(&ref_pic->parent->tf,
                                             FFMIN((row >> 1) - !(row & 1),
                                                   pic_height - 1),
                                             1);
                    ff_thread_await_progress(&ref_pic->parent->tf,
                                             FFMIN((row >> 1), pic_height - 1),
                                             0);
                } else if (FIELD_PICTURE(h) && !ref_field_picture) { // field referencing one field of a frame
                    ff_thread_await_progress(&ref_pic->parent->tf,
                                             FFMIN(row * 2 + ref_field,
                                                   pic_height - 1),
                                             0);
                } else if (FIELD_PICTURE(h)) {
                    ff_thread_await_progress(&ref_pic->parent->tf,
                                             FFMIN(row, pic_height - 1),
                                             ref_field);
                } else {
                    ff_thread_await_progress(&ref_pic->parent->tf,
                                             FFMIN(row, pic_height - 1),
                                             0);
                }
            }
        }
}

                                         H264Ref *pic,
                                         int n, int square, int height,
                                         int delta, int list,
                                         uint8_t *dest_y, uint8_t *dest_cb,
                                         uint8_t *dest_cr,
                                         int src_x_offset, int src_y_offset,
                                         const qpel_mc_func *qpix_op,
                                         h264_chroma_mc_func chroma_op,
                                         int pixel_shift, int chroma_idc)
{


                                 sl->mb_linesize, sl->mb_linesize,
                                 16 + 5, 16 + 5 /*FIXME*/, full_mx - 2,
                                 full_my - 2, pic_width, pic_height);
    }


        return;

        src_cb = pic->data[1] + offset;
        if (emu) {
            h->vdsp.emulated_edge_mc(sl->edge_emu_buffer,
                                     src_cb - (2 << pixel_shift) - 2 * sl->mb_linesize,
                                     sl->mb_linesize, sl->mb_linesize,
                                     16 + 5, 16 + 5 /*FIXME*/,
                                     full_mx - 2, full_my - 2,
                                     pic_width, pic_height);
            src_cb = sl->edge_emu_buffer + (2 << pixel_shift) + 2 * sl->mb_linesize;
        }
        qpix_op[luma_xy](dest_cb, src_cb, sl->mb_linesize); // FIXME try variable height perhaps?
        if (!square)
            qpix_op[luma_xy](dest_cb + delta, src_cb + delta, sl->mb_linesize);

        src_cr = pic->data[2] + offset;
        if (emu) {
            h->vdsp.emulated_edge_mc(sl->edge_emu_buffer,
                                     src_cr - (2 << pixel_shift) - 2 * sl->mb_linesize,
                                     sl->mb_linesize, sl->mb_linesize,
                                     16 + 5, 16 + 5 /*FIXME*/,
                                     full_mx - 2, full_my - 2,
                                     pic_width, pic_height);
            src_cr = sl->edge_emu_buffer + (2 << pixel_shift) + 2 * sl->mb_linesize;
        }
        qpix_op[luma_xy](dest_cr, src_cr, sl->mb_linesize); // FIXME try variable height perhaps?
        if (!square)
            qpix_op[luma_xy](dest_cr + delta, src_cr + delta, sl->mb_linesize);
        return;
    }

        // chroma offset when predicting from a field of opposite parity
    }

             (my >> ysh) * sl->mb_uvlinesize;

                                 sl->mb_uvlinesize, sl->mb_uvlinesize,
                                 9, 8 * chroma_idc + 1, (mx >> 3), (my >> ysh),
                                 pic_width >> 1, pic_height >> (chroma_idc == 1 /* yuv420 */));
    }
              height >> (chroma_idc == 1 /* yuv420 */),

                                 sl->mb_uvlinesize, sl->mb_uvlinesize,
                                 9, 8 * chroma_idc + 1, (mx >> 3), (my >> ysh),
                                 pic_width >> 1, pic_height >> (chroma_idc == 1 /* yuv420 */));
    }
              mx & 7, ((unsigned)my << (chroma_idc == 2 /* yuv422 */)) & 7);
}

                                         int n, int square,
                                         int height, int delta,
                                         uint8_t *dest_y, uint8_t *dest_cb,
                                         uint8_t *dest_cr,
                                         int x_offset, int y_offset,
                                         const qpel_mc_func *qpix_put,
                                         h264_chroma_mc_func chroma_put,
                                         const qpel_mc_func *qpix_avg,
                                         h264_chroma_mc_func chroma_avg,
                                         int list0, int list1,
                                         int pixel_shift, int chroma_idc)
{

        dest_cb += (2 * x_offset << pixel_shift) + 2 * y_offset * sl->mb_linesize;
        dest_cr += (2 * x_offset << pixel_shift) + 2 * y_offset * sl->mb_linesize;
    } else { /* yuv420 */
    }

                    dest_y, dest_cb, dest_cr, x_offset, y_offset,
                    qpix_op, chroma_op, pixel_shift, chroma_idc);

        qpix_op   = qpix_avg;
        chroma_op = chroma_avg;
    }

                    dest_y, dest_cb, dest_cr, x_offset, y_offset,
                    qpix_op, chroma_op, pixel_shift, chroma_idc);
    }
}

                                              int n, int square,
                                              int height, int delta,
                                              uint8_t *dest_y, uint8_t *dest_cb,
                                              uint8_t *dest_cr,
                                              int x_offset, int y_offset,
                                              const qpel_mc_func *qpix_put,
                                              h264_chroma_mc_func chroma_put,
                                              h264_weight_func luma_weight_op,
                                              h264_weight_func chroma_weight_op,
                                              h264_biweight_func luma_weight_avg,
                                              h264_biweight_func chroma_weight_avg,
                                              int list0, int list1,
                                              int pixel_shift, int chroma_idc)
{

        chroma_height     = height;
        chroma_weight_avg = luma_weight_avg;
        chroma_weight_op  = luma_weight_op;
        dest_cb += (2 * x_offset << pixel_shift) + 2 * y_offset * sl->mb_linesize;
        dest_cr += (2 * x_offset << pixel_shift) + 2 * y_offset * sl->mb_linesize;
    } else { /* yuv420 */
    }

        /* don't optimize for luma-only case, since B-frames usually
         * use implicit weights => chroma too. */

                    dest_y, dest_cb, dest_cr,
                    x_offset, y_offset, qpix_put, chroma_put,
                    pixel_shift, chroma_idc);
                    tmp_y, tmp_cb, tmp_cr,
                    x_offset, y_offset, qpix_put, chroma_put,
                    pixel_shift, chroma_idc);

                            height, 5, weight0, weight1, 0);
                                  chroma_height, 5, weight0, weight1, 0);
                                  chroma_height, 5, weight0, weight1, 0);
            }
        } else {
                            sl->pwt.luma_log2_weight_denom,
                            sl->pwt.luma_weight[refn0][0][0],
                            sl->pwt.luma_weight[refn1][1][0],
                                  sl->pwt.chroma_log2_weight_denom,
                                  sl->pwt.chroma_weight[refn0][0][0][0],
                                  sl->pwt.chroma_weight[refn1][1][0][0],
                                  sl->pwt.chroma_log2_weight_denom,
                                  sl->pwt.chroma_weight[refn0][0][1][0],
                                  sl->pwt.chroma_weight[refn1][1][1][0],
            }
        }
    } else {
                    dest_y, dest_cb, dest_cr, x_offset, y_offset,
                    qpix_put, chroma_put, pixel_shift, chroma_idc);

                       sl->pwt.luma_log2_weight_denom,
                       sl->pwt.luma_weight[refn][list][0],
                       sl->pwt.luma_weight[refn][list][1]);
                                 sl->pwt.chroma_log2_weight_denom,
                                 sl->pwt.chroma_weight[refn][list][0][0],
                                 sl->pwt.chroma_weight[refn][list][0][1]);
                                 sl->pwt.chroma_log2_weight_denom,
                                 sl->pwt.chroma_weight[refn][list][1][0],
                                 sl->pwt.chroma_weight[refn][list][1][1]);
            }
        }
    }
}

                                             int list, int pixel_shift,
                                             int chroma_idc)
{
    /* fetch pixels for estimated mv 4 macroblocks ahead
     * optimized for 64byte cache lines */
            h->vdsp.prefetch(src[1] + off, sl->linesize, 4);
            h->vdsp.prefetch(src[2] + off, sl->linesize, 4);
        } else {
        }
    }
}

                                            uint8_t *src_y,
                                            uint8_t *src_cb, uint8_t *src_cr,
                                            int linesize, int uvlinesize,
                                            int xchg, int chroma444,
                                            int simple, int pixel_shift)
{

                return;
        } else {
        }
    }

    } else {
    }



#define XCHG(a, b, xchg)                        \
    if (pixel_shift) {                          \
        if (xchg) {                             \
            AV_SWAP64(b + 0, a + 0);            \
            AV_SWAP64(b + 8, a + 8);            \
        } else {                                \
            AV_COPY128(b, a);                   \
        }                                       \
    } else if (xchg)                            \
        AV_SWAP64(b, a);                        \
    else                                        \
        AV_COPY64(b, a);

        }
        }
                }
                }
            } else {
                }
            }
        }
    }
}

                                        int index)
{
    } else
}

                                         int index, int value)
{
        AV_WN32A(((int32_t *)mb) + index, value);
    } else
}

                                                       H264SliceContext *sl,
                                                       int mb_type, int simple,
                                                       int transform_bypass,
                                                       int pixel_shift,
                                                       const int *block_offset,
                                                       int linesize,
                                                       uint8_t *dest_y, int p)
{
            } else {
            }
                    } else
                        h->hpc.pred8x8l_filter_add[dir](ptr, sl->mb + (i * 16 + p * 256 << pixel_shift),
                                                        (sl-> topleft_samples_available << i) & 0x8000,
                                                        (sl->topright_samples_available << i) & 0x4000, linesize);
                } else {
                        else
                    }
                }
            }
        } else {
            } else {
            }

                } else {
                            } else {
                            }
                        } else
                    } else
                        topright = NULL;

                        else
                    }
                }
            }
        }
    } else {
            else {
                static const uint8_t dc_mapping[16] = {
                     0 * 16,  1 * 16,  4 * 16,  5 * 16,
                     2 * 16,  3 * 16,  6 * 16,  7 * 16,
                     8 * 16,  9 * 16, 12 * 16, 13 * 16,
                    10 * 16, 11 * 16, 14 * 16, 15 * 16
                };
                                            pixel_shift, i));
            }
        }
    }
}

                                                    int mb_type, int simple,
                                                    int transform_bypass,
                                                    int pixel_shift,
                                                    const int *block_offset,
                                                    int linesize,
                                                    uint8_t *dest_y, int p)
{
                     sl->intra16x16_pred_mode == HOR_PRED8x8)) {
                                                                   linesize);
                } else {
                                                              linesize);
                }
            } else {
                                                linesize,
            }
                                 linesize);
            } else {
                                               linesize,
                else
                                               linesize,
            }
        }
    }
}

#define BITS   8
#define SIMPLE 1
#include "h264_mb_template.c"

#undef  BITS
#define BITS   16
#include "h264_mb_template.c"

#undef  SIMPLE
#define SIMPLE 0
#include "h264_mb_template.c"

{

        else
    } else
