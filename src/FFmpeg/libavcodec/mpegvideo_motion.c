/*
 * Copyright (c) 2000,2001 Fabrice Bellard
 * Copyright (c) 2002-2004 Michael Niedermayer <michaelni@gmx.at>
 *
 * 4MV & hq & B-frame encoding stuff by Michael Niedermayer <michaelni@gmx.at>
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

#include <string.h>

#include "libavutil/avassert.h"
#include "libavutil/internal.h"
#include "avcodec.h"
#include "h261.h"
#include "mpegutils.h"
#include "mpegvideo.h"
#include "mjpegenc.h"
#include "msmpeg4.h"
#include "qpeldsp.h"
#include "wmv2.h"
#include <limits.h>

static void gmc1_motion(MpegEncContext *s,
                        uint8_t *dest_y, uint8_t *dest_cb, uint8_t *dest_cr,
                        uint8_t **ref_picture)
{
    uint8_t *ptr;
    int src_x, src_y, motion_x, motion_y;
    ptrdiff_t offset, linesize, uvlinesize;
    int emu = 0;

    motion_x   = s->sprite_offset[0][0];
    motion_y   = s->sprite_offset[0][1];
    src_x      = s->mb_x * 16 + (motion_x >> (s->sprite_warping_accuracy + 1));
    src_y      = s->mb_y * 16 + (motion_y >> (s->sprite_warping_accuracy + 1));
    motion_x *= 1 << (3 - s->sprite_warping_accuracy);
    motion_y *= 1 << (3 - s->sprite_warping_accuracy);
    src_x      = av_clip(src_x, -16, s->width);
    if (src_x == s->width)
        motion_x = 0;
    src_y = av_clip(src_y, -16, s->height);
    if (src_y == s->height)
        motion_y = 0;

    linesize   = s->linesize;
    uvlinesize = s->uvlinesize;

    ptr = ref_picture[0] + src_y * linesize + src_x;

    if ((unsigned)src_x >= FFMAX(s->h_edge_pos - 17, 0) ||
        (unsigned)src_y >= FFMAX(s->v_edge_pos - 17, 0)) {
        s->vdsp.emulated_edge_mc(s->sc.edge_emu_buffer, ptr,
                                 linesize, linesize,
                                 17, 17,
                                 src_x, src_y,
                                 s->h_edge_pos, s->v_edge_pos);
        ptr = s->sc.edge_emu_buffer;
    }

    if ((motion_x | motion_y) & 7) {
        s->mdsp.gmc1(dest_y, ptr, linesize, 16,
                     motion_x & 15, motion_y & 15, 128 - s->no_rounding);
        s->mdsp.gmc1(dest_y + 8, ptr + 8, linesize, 16,
                     motion_x & 15, motion_y & 15, 128 - s->no_rounding);
    } else {
        int dxy;

        dxy = ((motion_x >> 3) & 1) | ((motion_y >> 2) & 2);
        if (s->no_rounding) {
            s->hdsp.put_no_rnd_pixels_tab[0][dxy](dest_y, ptr, linesize, 16);
        } else {
            s->hdsp.put_pixels_tab[0][dxy](dest_y, ptr, linesize, 16);
        }
    }

    if (CONFIG_GRAY && s->avctx->flags & AV_CODEC_FLAG_GRAY)
        return;

    motion_x   = s->sprite_offset[1][0];
    motion_y   = s->sprite_offset[1][1];
    src_x      = s->mb_x * 8 + (motion_x >> (s->sprite_warping_accuracy + 1));
    src_y      = s->mb_y * 8 + (motion_y >> (s->sprite_warping_accuracy + 1));
    motion_x  *= 1 << (3 - s->sprite_warping_accuracy);
    motion_y  *= 1 << (3 - s->sprite_warping_accuracy);
    src_x      = av_clip(src_x, -8, s->width >> 1);
    if (src_x == s->width >> 1)
        motion_x = 0;
    src_y = av_clip(src_y, -8, s->height >> 1);
    if (src_y == s->height >> 1)
        motion_y = 0;

    offset = (src_y * uvlinesize) + src_x;
    ptr    = ref_picture[1] + offset;
    if ((unsigned)src_x >= FFMAX((s->h_edge_pos >> 1) - 9, 0) ||
        (unsigned)src_y >= FFMAX((s->v_edge_pos >> 1) - 9, 0)) {
        s->vdsp.emulated_edge_mc(s->sc.edge_emu_buffer, ptr,
                                 uvlinesize, uvlinesize,
                                 9, 9,
                                 src_x, src_y,
                                 s->h_edge_pos >> 1, s->v_edge_pos >> 1);
        ptr = s->sc.edge_emu_buffer;
        emu = 1;
    }
    s->mdsp.gmc1(dest_cb, ptr, uvlinesize, 8,
                 motion_x & 15, motion_y & 15, 128 - s->no_rounding);

    ptr = ref_picture[2] + offset;
    if (emu) {
        s->vdsp.emulated_edge_mc(s->sc.edge_emu_buffer, ptr,
                                 uvlinesize, uvlinesize,
                                 9, 9,
                                 src_x, src_y,
                                 s->h_edge_pos >> 1, s->v_edge_pos >> 1);
        ptr = s->sc.edge_emu_buffer;
    }
    s->mdsp.gmc1(dest_cr, ptr, uvlinesize, 8,
                 motion_x & 15, motion_y & 15, 128 - s->no_rounding);
}

static void gmc_motion(MpegEncContext *s,
                       uint8_t *dest_y, uint8_t *dest_cb, uint8_t *dest_cr,
                       uint8_t **ref_picture)
{
    uint8_t *ptr;
    int linesize, uvlinesize;
    const int a = s->sprite_warping_accuracy;
    int ox, oy;

    linesize   = s->linesize;
    uvlinesize = s->uvlinesize;

    ptr = ref_picture[0];

    ox = s->sprite_offset[0][0] + s->sprite_delta[0][0] * s->mb_x * 16 +
         s->sprite_delta[0][1] * s->mb_y * 16;
    oy = s->sprite_offset[0][1] + s->sprite_delta[1][0] * s->mb_x * 16 +
         s->sprite_delta[1][1] * s->mb_y * 16;

    s->mdsp.gmc(dest_y, ptr, linesize, 16,
                ox, oy,
                s->sprite_delta[0][0], s->sprite_delta[0][1],
                s->sprite_delta[1][0], s->sprite_delta[1][1],
                a + 1, (1 << (2 * a + 1)) - s->no_rounding,
                s->h_edge_pos, s->v_edge_pos);
    s->mdsp.gmc(dest_y + 8, ptr, linesize, 16,
                ox + s->sprite_delta[0][0] * 8,
                oy + s->sprite_delta[1][0] * 8,
                s->sprite_delta[0][0], s->sprite_delta[0][1],
                s->sprite_delta[1][0], s->sprite_delta[1][1],
                a + 1, (1 << (2 * a + 1)) - s->no_rounding,
                s->h_edge_pos, s->v_edge_pos);

    if (CONFIG_GRAY && s->avctx->flags & AV_CODEC_FLAG_GRAY)
        return;

    ox = s->sprite_offset[1][0] + s->sprite_delta[0][0] * s->mb_x * 8 +
         s->sprite_delta[0][1] * s->mb_y * 8;
    oy = s->sprite_offset[1][1] + s->sprite_delta[1][0] * s->mb_x * 8 +
         s->sprite_delta[1][1] * s->mb_y * 8;

    ptr = ref_picture[1];
    s->mdsp.gmc(dest_cb, ptr, uvlinesize, 8,
                ox, oy,
                s->sprite_delta[0][0], s->sprite_delta[0][1],
                s->sprite_delta[1][0], s->sprite_delta[1][1],
                a + 1, (1 << (2 * a + 1)) - s->no_rounding,
                (s->h_edge_pos + 1) >> 1, (s->v_edge_pos + 1) >> 1);

    ptr = ref_picture[2];
    s->mdsp.gmc(dest_cr, ptr, uvlinesize, 8,
                ox, oy,
                s->sprite_delta[0][0], s->sprite_delta[0][1],
                s->sprite_delta[1][0], s->sprite_delta[1][1],
                a + 1, (1 << (2 * a + 1)) - s->no_rounding,
                (s->h_edge_pos + 1) >> 1, (s->v_edge_pos + 1) >> 1);
}

                              uint8_t *dest, uint8_t *src,
                              int src_x, int src_y,
                              op_pixels_func *pix_op,
                              int motion_x, int motion_y)
{


    /* WARNING: do no forget half pels */

                                     s->linesize, s->linesize,
                                     9, 9,
                                     src_x, src_y,
                                     s->h_edge_pos, s->v_edge_pos);
        }
}

static av_always_inline
                          uint8_t *dest_y,
                          uint8_t *dest_cb,
                          uint8_t *dest_cr,
                          int field_based,
                          int bottom_field,
                          int field_select,
                          uint8_t **ref_picture,
                          op_pixels_func (*pix_op)[4],
                          int motion_x,
                          int motion_y,
                          int h,
                          int is_mpeg12,
                          int is_16x8,
                          int mb_y)
{
        uvsrc_x, uvsrc_y, v_edge_pos, block_y_half;



        if ((s->workaround_bugs & FF_BUG_HPEL_CHROMA) && field_based) {
            mx      = (motion_x >> 1) | (motion_x & 1);
            my      = motion_y >> 1;
            uvdxy   = ((my & 1) << 1) | (mx & 1);
            uvsrc_x = s->mb_x * 8 + (mx >> 1);
            uvsrc_y = (mb_y << (3 - block_y_half)) + (my >> 1);
        } else {
        }
    // Even chroma mv's are full pel in H261
    } else {
        if (s->chroma_y_shift) {
        } else {
                // Chroma422
            } else {
                // Chroma444
                uvdxy   = dxy;
                uvsrc_x = src_x;
                uvsrc_y = src_y;
            }
        }
    }


            s->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
            av_log(s->avctx, AV_LOG_DEBUG,
                   "MPEG motion vector out of boundary (%d %d)\n", src_x,
                   src_y);
            return;
        }
                                 s->linesize, s->linesize,
                                 17, 17 + field_based,
                                 src_x, src_y,
                                 s->h_edge_pos, s->v_edge_pos);
                vbuf -= s->uvlinesize;
                                     s->uvlinesize, s->uvlinesize,
                                     9, 9 + field_based,
                                     uvsrc_x, uvsrc_y,
                                     s->uvlinesize, s->uvlinesize,
                                     9, 9 + field_based,
                                     uvsrc_x, uvsrc_y,
        }
    }

    /* FIXME use this for field pix too instead of the obnoxious hack which
     * changes picture.data */
    }

    }


    }
    }
}
/* apply one mpeg motion vector to the three components */
                        uint8_t *dest_y, uint8_t *dest_cb, uint8_t *dest_cr,
                        int field_select, uint8_t **ref_picture,
                        op_pixels_func (*pix_op)[4],
                        int motion_x, int motion_y, int h, int is_16x8, int mb_y)
{
#if !CONFIG_SMALL
                             field_select, ref_picture, pix_op,
                             motion_x, motion_y, h, 1, is_16x8, mb_y);
    else
#endif
                             field_select, ref_picture, pix_op,
                             motion_x, motion_y, h, 0, is_16x8, mb_y);

                              uint8_t *dest_cb, uint8_t *dest_cr,
                              int bottom_field, int field_select,
                              uint8_t **ref_picture,
                              op_pixels_func (*pix_op)[4],
                              int motion_x, int motion_y, int h, int mb_y)
{
#if !CONFIG_SMALL
                             bottom_field, field_select, ref_picture, pix_op,
                             motion_x, motion_y, h, 1, 0, mb_y);
    else
#endif
        mpeg_motion_internal(s, dest_y, dest_cb, dest_cr, 1,
                             bottom_field, field_select, ref_picture, pix_op,
                             motion_x, motion_y, h, 0, 0, mb_y);

// FIXME: SIMDify, avg variant, 16x16 version
{
#define OBMC_FILTER(x, t, l, m, r, b)\
    dst[x]= (t*top[x] + l*left[x] + m*mid[x] + r*right[x] + b*bottom[x] + 4)>>3
#define OBMC_FILTER4(x, t, l, m, r, b)\
    OBMC_FILTER(x         , t, l, m, r, b);\
    OBMC_FILTER(x+1       , t, l, m, r, b);\
    OBMC_FILTER(x  +stride, t, l, m, r, b);\
    OBMC_FILTER(x+1+stride, t, l, m, r, b);


/* obmc for 1 8x8 luma block */
                               uint8_t *dest, uint8_t *src,
                               int src_x, int src_y,
                               op_pixels_func *pix_op,
                               int16_t mv[5][2] /* mid top left right bottom */)
#define MID    0
{


        } else {
        }
    }


                               uint8_t *dest_y,
                               uint8_t *dest_cb,
                               uint8_t *dest_cr,
                               int field_based, int bottom_field,
                               int field_select, uint8_t **ref_picture,
                               op_pixels_func (*pix_op)[4],
                               qpel_mc_func (*qpix_op)[16],
                               int motion_x, int motion_y, int h)
{




        mx = motion_x / 2;
        my = motion_y >> 1;
        static const int rtab[8] = { 0, 0, 1, 1, 0, 0, 0, 1 };
        mx = (motion_x >> 1) + rtab[motion_x & 7];
        my = (motion_y >> 1) + rtab[motion_y & 7];
        mx = (motion_x >> 1) | (motion_x & 1);
        my = (motion_y >> 1) | (motion_y & 1);
    } else {
    }




                                 s->linesize, s->linesize,
                                 17, 17 + field_based,
                                 src_x, src_y * (1 << field_based),
                                 s->h_edge_pos, s->v_edge_pos);
                vbuf -= s->uvlinesize;
                                     s->uvlinesize, s->uvlinesize,
                                     9, 9 + field_based,
                                     uvsrc_x, uvsrc_y * (1 << field_based),
                                     s->uvlinesize, s->uvlinesize,
                                     9, 9 + field_based,
                                     uvsrc_x, uvsrc_y * (1 << field_based),
        }
    }

    else {
        if (bottom_field) {
            dest_y  += s->linesize;
            dest_cb += s->uvlinesize;
            dest_cr += s->uvlinesize;
        }

        if (field_select) {
            ptr_y  += s->linesize;
            ptr_cb += s->uvlinesize;
            ptr_cr += s->uvlinesize;
        }
        // damn interlaced mode
        // FIXME boundary mirroring is not exactly correct here
        qpix_op[1][dxy](dest_y, ptr_y, linesize);
        qpix_op[1][dxy](dest_y + 8, ptr_y + 8, linesize);
    }
    }

/**
 * H.263 chroma 4mv motion compensation.
 */
                              uint8_t *dest_cb, uint8_t *dest_cr,
                              uint8_t **ref_picture,
                              op_pixels_func *pix_op,
                              int mx, int my)
{

    /* In case of 8X8, we construct a single chroma motion vector
     * with a special rounding */



                                 s->uvlinesize, s->uvlinesize,
                                 9, 9, src_x, src_y,
    }

                                 s->uvlinesize, s->uvlinesize,
                                 9, 9, src_x, src_y,
    }

{
    /* fetch pixels for estimated mv 4 macroblocks ahead
     * optimized for 64byte cache lines */


                              uint8_t *dest_y,
                              uint8_t *dest_cb,
                              uint8_t *dest_cr,
                              uint8_t **ref_picture,
                              op_pixels_func (*pix_op)[4])
{



              cur_frame->motion_val[0][mot_xy + mot_stride]);
              cur_frame->motion_val[0][mot_xy + mot_stride + 1]);

              cur_frame->motion_val[0][mot_xy + mot_stride]);
              cur_frame->motion_val[0][mot_xy + mot_stride + 1]);

    } else {
                  cur_frame->motion_val[0][mot_xy - mot_stride]);
                  cur_frame->motion_val[0][mot_xy - mot_stride + 1]);
    }

    } else {
                  cur_frame->motion_val[0][mot_xy - 1 + mot_stride]);
    }

    } else {
                  cur_frame->motion_val[0][mot_xy + 2 + mot_stride]);
    }

    mx = 0;
    my = 0;
        };
        // FIXME cleanup
                    ref_picture[0],
                    mv);

    }
                          mx, my);

                             uint8_t *dest_y,
                             uint8_t *dest_cb,
                             uint8_t *dest_cr,
                             int dir,
                             uint8_t **ref_picture,
                             qpel_mc_func (*qpix_op)[16],
                             op_pixels_func (*pix_op)[4])
{



            /* WARNING: do no forget half pels */

                                         s->linesize, s->linesize,
                                         9, 9,
                                         src_x, src_y,
                                         s->h_edge_pos,
                                         s->v_edge_pos);
            }

        }
    } else {
                        ref_picture[0],
                        s->mv[dir][i][0],
                        s->mv[dir][i][1]);

        }
    }


/**
 * motion compensation of a single macroblock
 * @param s context
 * @param dest_y luma destination pointer
 * @param dest_cb chroma cb/u destination pointer
 * @param dest_cr chroma cr/v destination pointer
 * @param dir direction (0->forward, 1->backward)
 * @param ref_picture array[3] of pointers to the 3 planes of the reference picture
 * @param pix_op halfpel motion compensation function (average or put normally)
 * @param qpix_op qpel motion compensation function (average or put normally)
 * the motion vectors are taken from s->mv and the MV type from s->mv_type
 */
                                                 uint8_t *dest_y,
                                                 uint8_t *dest_cb,
                                                 uint8_t *dest_cr,
                                                 int dir,
                                                 uint8_t **ref_picture,
                                                 op_pixels_func (*pix_op)[4],
                                                 qpel_mc_func (*qpix_op)[16],
                                                 int is_mpeg12)
{


    }

            if (s->real_sprite_warping_points == 1) {
                gmc1_motion(s, dest_y, dest_cb, dest_cr,
                            ref_picture);
            } else {
                gmc_motion(s, dest_y, dest_cb, dest_cr,
                           ref_picture);
            }
                        0, 0, 0,
                        ref_picture, pix_op, qpix_op,
                        s->mv[dir][0][0], s->mv[dir][0][1], 16);
                            ref_picture, pix_op,
                            s->mv[dir][0][0], s->mv[dir][0][1], 16);
        } else {
                        ref_picture, pix_op,
                        s->mv[dir][0][0], s->mv[dir][0][1], 16, 0, mb_y);
        }
        break;
    case MV_TYPE_8X8:
                      dir, ref_picture, qpix_op, pix_op);
        break;
            if (!is_mpeg12 && s->quarter_sample) {
                for (i = 0; i < 2; i++)
                    qpel_motion(s, dest_y, dest_cb, dest_cr,
                                1, i, s->field_select[dir][i],
                                ref_picture, pix_op, qpix_op,
                                s->mv[dir][i][0], s->mv[dir][i][1], 8);
            } else {
                /* top field */
                                  0, s->field_select[dir][0],
                                  ref_picture, pix_op,
                                  s->mv[dir][0][0], s->mv[dir][0][1], 8, mb_y);
                /* bottom field */
                                  1, s->field_select[dir][1],
                                  ref_picture, pix_op,
                                  s->mv[dir][1][0], s->mv[dir][1][1], 8, mb_y);
            }
        } else {
            }

                        s->field_select[dir][0],
                        ref_picture, pix_op,
                        s->mv[dir][0][0], s->mv[dir][0][1], 16, 0, mb_y >> 1);
        }
        break;
    case MV_TYPE_16X8:

                ref2picture = ref_picture;
            } else {
            }

                        s->field_select[dir][i],
                        ref2picture, pix_op,
                        s->mv[dir][i][0], s->mv[dir][i][1],

        }
        break;
            for (i = 0; i < 2; i++) {
                int j;
                for (j = 0; j < 2; j++)
                    mpeg_motion_field(s, dest_y, dest_cb, dest_cr,
                                      j, j ^ i, ref_picture, pix_op,
                                      s->mv[dir][2 * i + j][0],
                                      s->mv[dir][2 * i + j][1], 8, mb_y);
                pix_op = s->hdsp.avg_pixels_tab;
            }
        } else {
                ref_picture = s->current_picture_ptr->f->data;
            }
                            ref_picture, pix_op,
                            16, 0, mb_y >> 1);

                // after put we make avg of the same block

                /* opposite parity is always in the same frame if this is
                 * second field */
                }
            }
        }
        break;
    default: av_assert2(0);
    }
}

                   uint8_t *dest_y, uint8_t *dest_cb,
                   uint8_t *dest_cr, int dir,
                   uint8_t **ref_picture,
                   op_pixels_func (*pix_op)[4],
                   qpel_mc_func (*qpix_op)[16])
{
#if !CONFIG_SMALL
                            ref_picture, pix_op, qpix_op, 1);
    else
#endif
                            ref_picture, pix_op, qpix_op, 0);
