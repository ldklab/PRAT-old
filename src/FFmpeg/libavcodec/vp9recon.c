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

#include "libavutil/avassert.h"

#include "avcodec.h"
#include "internal.h"
#include "videodsp.h"
#include "vp9data.h"
#include "vp9dec.h"

                                             uint8_t *dst_edge, ptrdiff_t stride_edge,
                                             uint8_t *dst_inner, ptrdiff_t stride_inner,
                                             uint8_t *l, int col, int x, int w,
                                             int row, int y, enum TxfmMode tx,
                                             int p, int ss_h, int ss_v, int bytesperpixel)
{
        [VERT_PRED]            = { { DC_127_PRED,          VERT_PRED            },
                                   { DC_127_PRED,          VERT_PRED            } },
        [HOR_PRED]             = { { DC_129_PRED,          DC_129_PRED          },
                                   { HOR_PRED,             HOR_PRED             } },
        [DC_PRED]              = { { DC_128_PRED,          TOP_DC_PRED          },
                                   { LEFT_DC_PRED,         DC_PRED              } },
        [DIAG_DOWN_LEFT_PRED]  = { { DC_127_PRED,          DIAG_DOWN_LEFT_PRED  },
                                   { DC_127_PRED,          DIAG_DOWN_LEFT_PRED  } },
        [DIAG_DOWN_RIGHT_PRED] = { { DIAG_DOWN_RIGHT_PRED, DIAG_DOWN_RIGHT_PRED },
                                   { DIAG_DOWN_RIGHT_PRED, DIAG_DOWN_RIGHT_PRED } },
        [VERT_RIGHT_PRED]      = { { VERT_RIGHT_PRED,      VERT_RIGHT_PRED      },
                                   { VERT_RIGHT_PRED,      VERT_RIGHT_PRED      } },
        [HOR_DOWN_PRED]        = { { HOR_DOWN_PRED,        HOR_DOWN_PRED        },
                                   { HOR_DOWN_PRED,        HOR_DOWN_PRED        } },
        [VERT_LEFT_PRED]       = { { DC_127_PRED,          VERT_LEFT_PRED       },
                                   { DC_127_PRED,          VERT_LEFT_PRED       } },
        [HOR_UP_PRED]          = { { DC_129_PRED,          DC_129_PRED          },
                                   { HOR_UP_PRED,          HOR_UP_PRED          } },
        [TM_VP8_PRED]          = { { DC_129_PRED,          VERT_PRED            },
                                   { HOR_PRED,             TM_VP8_PRED          } },
    };
        uint8_t needs_left:1;
        uint8_t needs_top:1;
        uint8_t needs_topleft:1;
        uint8_t needs_topright:1;
        uint8_t invert_left:1;
    } edges[N_INTRA_PRED_MODES] = {
        [VERT_PRED]            = { .needs_top  = 1 },
        [HOR_PRED]             = { .needs_left = 1 },
        [DC_PRED]              = { .needs_top  = 1, .needs_left = 1 },
        [DIAG_DOWN_LEFT_PRED]  = { .needs_top  = 1, .needs_topright = 1 },
        [DIAG_DOWN_RIGHT_PRED] = { .needs_left = 1, .needs_top = 1,
                                   .needs_topleft = 1 },
        [VERT_RIGHT_PRED]      = { .needs_left = 1, .needs_top = 1,
                                   .needs_topleft = 1 },
        [HOR_DOWN_PRED]        = { .needs_left = 1, .needs_top = 1,
                                   .needs_topleft = 1 },
        [VERT_LEFT_PRED]       = { .needs_top  = 1, .needs_topright = 1 },
        [HOR_UP_PRED]          = { .needs_left = 1, .invert_left = 1 },
        [TM_VP8_PRED]          = { .needs_left = 1, .needs_top = 1,
                                   .needs_topleft = 1 },
        [LEFT_DC_PRED]         = { .needs_left = 1 },
        [TOP_DC_PRED]          = { .needs_top  = 1 },
        [DC_128_PRED]          = { 0 },
        [DC_127_PRED]          = { 0 },
        [DC_129_PRED]          = { 0 }
    };



        // if top of sb64-row, use s->intra_pred_data[] instead of
        // dst[-stride] for intra prediction (it contains pre- instead of
        // post-loopfilter data)
        }

            *a = top;
        } else {
                } else {
#define memset_bpp(c, i1, v, i2, num) do { \
    if (bytesperpixel == 1) { \
        memset(&(c)[(i1)], (v)[(i2)], (num)); \
    } else { \
        int n, val = AV_RN16A(&(v)[(i2) * 2]); \
        for (n = 0; n < (num); n++) { \
            AV_WN16A(&(c)[((i1) + n) * 2], val); \
        } \
    } \
} while (0)
                }
            } else {
#define memset_val(c, val, num) do { \
    if (bytesperpixel == 1) { \
        memset((c), (val), (num)); \
    } else { \
        int n; \
        for (n = 0; n < (num); n++) { \
            AV_WN16A(&(c)[n * 2], (val)); \
        } \
    } \
} while (0)
            }
#define assign_bpp(c, i1, v, i2) do { \
    if (bytesperpixel == 1) { \
        (c)[(i1)] = (v)[(i2)]; \
    } else { \
        AV_COPY16(&(c)[(i1) * 2], &(v)[(i2) * 2]); \
    } \
} while (0)
                } else {
#define assign_val(c, i, v) do { \
    if (bytesperpixel == 1) { \
        (c)[(i)] = (v); \
    } else { \
        AV_WN16A(&(c)[(i) * 2], (v)); \
    } \
} while (0)
                }
            }
                    memcpy(&(*a)[4 * bytesperpixel], &top[4 * bytesperpixel], 4 * bytesperpixel);
                } else {
                }
            }
        }
    }

                } else {
                }
            } else {
                } else {
                }
            }
        } else {
        }
    }

}

                                         ptrdiff_t uv_off, int bytesperpixel)
{

        uint8_t *ptr = dst, *ptr_r = dst_r;

                                    ptr, td->y_stride, l,
                                    col, x, w4, row, y, b->tx, 0, 0, 0, bytesperpixel);
        }
    }

    // U/V
            uint8_t *ptr = dst, *ptr_r = dst_r;

                                        ptr, td->uv_stride, l, col, x, w4, row, y,
            }
        }
    }
}

{

{

                                              uint8_t *dst, ptrdiff_t dst_stride,
                                              const uint8_t *ref, ptrdiff_t ref_stride,
                                              ThreadFrame *ref_frame,
                                              ptrdiff_t y, ptrdiff_t x, const VP56mv *mv,
                                              int bw, int bh, int w, int h, int bytesperpixel)
{

    // FIXME bilinear filter only needs 0/1 pixels, not 3/4
    // we use +7 because the last 7 pixels of each sbrow can be changed in
    // the longest loopfilter of the next sbrow
    // The arm/aarch64 _hv filters read one more row than what actually is
    // needed, so switch to emulated edge one pixel sooner vertically
    // (!!my * 5) than horizontally (!!mx * 4).
                                 160, ref_stride,
    }
}

                                                uint8_t *dst_u, uint8_t *dst_v,
                                                ptrdiff_t dst_stride,
                                                const uint8_t *ref_u, ptrdiff_t src_stride_u,
                                                const uint8_t *ref_v, ptrdiff_t src_stride_v,
                                                ThreadFrame *ref_frame,
                                                ptrdiff_t y, ptrdiff_t x, const VP56mv *mv,
                                                int bw, int bh, int w, int h, int bytesperpixel)
{

    // FIXME bilinear filter only needs 0/1 pixels, not 3/4
    // we use +7 because the last 7 pixels of each sbrow can be changed in
    // the longest loopfilter of the next sbrow
    // The arm/aarch64 _hv filters read one more row than what actually is
    // needed, so switch to emulated edge one pixel sooner vertically
    // (!!my * 5) than horizontally (!!mx * 4).
                                 160, src_stride_u,

                                 160, src_stride_v,
                                 bw + !!mx * 7, bh + !!my * 7,
                                 x - !!mx * 3, y - !!my * 3, w, h);
    } else {
    }
}

#define mc_luma_dir(td, mc, dst, dst_ls, src, src_ls, tref, row, col, mv, \
                    px, py, pw, ph, bw, bh, w, h, i) \
    mc_luma_unscaled(td, s->dsp.mc, dst, dst_ls, src, src_ls, tref, row, col, \
                     mv, bw, bh, w, h, bytesperpixel)
#define mc_chroma_dir(td, mc, dstu, dstv, dst_ls, srcu, srcu_ls, srcv, srcv_ls, tref, \
                      row, col, mv, px, py, pw, ph, bw, bh, w, h, i) \
    mc_chroma_unscaled(td, s->dsp.mc, dstu, dstv, dst_ls, srcu, srcu_ls, srcv, srcv_ls, tref, \
                       row, col, mv, bw, bh, w, h, bytesperpixel)
#define SCALED 0
#define FN(x) x##_8bpp
#define BYTES_PER_PIXEL 1
#include "vp9_mc_template.c"
#undef FN
#undef BYTES_PER_PIXEL
#define FN(x) x##_16bpp
#define BYTES_PER_PIXEL 2
#include "vp9_mc_template.c"
#undef mc_luma_dir
#undef mc_chroma_dir
#undef FN
#undef BYTES_PER_PIXEL
#undef SCALED

                                            vp9_mc_func (*mc)[2],
                                            uint8_t *dst, ptrdiff_t dst_stride,
                                            const uint8_t *ref, ptrdiff_t ref_stride,
                                            ThreadFrame *ref_frame,
                                            ptrdiff_t y, ptrdiff_t x, const VP56mv *in_mv,
                                            int px, int py, int pw, int ph,
                                            int bw, int bh, int w, int h, int bytesperpixel,
                                            const uint16_t *scale, const uint8_t *step)
{
        s->s.frames[CUR_FRAME].tf.f->height == ref_frame->f->height) {
                         y, x, in_mv, bw, bh, w, h, bytesperpixel);
    } else {
#define scale_mv(n, dim) (((int64_t)(n) * scale[dim]) >> 14)

    // BUG libvpx seems to scale the two components separately. This introduces
    // rounding errors but we have to reproduce them to be exactly compatible
    // with the output from libvpx...

    // FIXME bilinear filter only needs 0/1 pixels, not 3/4
    // we use +7 because the last 7 pixels of each sbrow can be changed in
    // the longest loopfilter of the next sbrow
    // The arm/aarch64 _hv filters read one more row than what actually is
    // needed, so switch to emulated edge one pixel sooner vertically
    // (y + 5 >= h - refbh_m1) than horizontally (x + 4 >= w - refbw_m1).
                                 288, ref_stride,
                                 refbw_m1 + 8, refbh_m1 + 8,
    }
    }
}

                                              vp9_mc_func (*mc)[2],
                                              uint8_t *dst_u, uint8_t *dst_v,
                                              ptrdiff_t dst_stride,
                                              const uint8_t *ref_u, ptrdiff_t src_stride_u,
                                              const uint8_t *ref_v, ptrdiff_t src_stride_v,
                                              ThreadFrame *ref_frame,
                                              ptrdiff_t y, ptrdiff_t x, const VP56mv *in_mv,
                                              int px, int py, int pw, int ph,
                                              int bw, int bh, int w, int h, int bytesperpixel,
                                              const uint16_t *scale, const uint8_t *step)
{
        s->s.frames[CUR_FRAME].tf.f->height == ref_frame->f->height) {
                           ref_v, src_stride_v, ref_frame,
                           y, x, in_mv, bw, bh, w, h, bytesperpixel);
    } else {

        // BUG https://code.google.com/p/webm/issues/detail?id=820
    } else {
        mv.x = av_clip(in_mv->x, -(x + pw - px + 4) * 8, (s->cols * 8 - x + px + 3) * 8);
        mx = scale_mv(mv.x * 2, 0) + scale_mv(x * 16, 0);
    }
        // BUG https://code.google.com/p/webm/issues/detail?id=820
    } else {
        mv.y = av_clip(in_mv->y, -(y + ph - py + 4) * 8, (s->rows * 8 - y + py + 3) * 8);
        my = scale_mv(mv.y * 2, 1) + scale_mv(y * 16, 1);
    }
#undef scale_mv
    // FIXME bilinear filter only needs 0/1 pixels, not 3/4
    // we use +7 because the last 7 pixels of each sbrow can be changed in
    // the longest loopfilter of the next sbrow
    // The arm/aarch64 _hv filters read one more row than what actually is
    // needed, so switch to emulated edge one pixel sooner vertically
    // (y + 5 >= h - refbh_m1) than horizontally (x + 4 >= w - refbw_m1).
                                 288, src_stride_u,
                                 refbw_m1 + 8, refbh_m1 + 8,

                                 288, src_stride_v,
                                 refbw_m1 + 8, refbh_m1 + 8,
                                 x - 3, y - 3, w, h);
    } else {
    }
    }
}

#define mc_luma_dir(td, mc, dst, dst_ls, src, src_ls, tref, row, col, mv, \
                    px, py, pw, ph, bw, bh, w, h, i) \
    mc_luma_scaled(td, s->dsp.s##mc, s->dsp.mc, dst, dst_ls, src, src_ls, tref, row, col, \
                   mv, px, py, pw, ph, bw, bh, w, h, bytesperpixel, \
                   s->mvscale[b->ref[i]], s->mvstep[b->ref[i]])
#define mc_chroma_dir(td, mc, dstu, dstv, dst_ls, srcu, srcu_ls, srcv, srcv_ls, tref, \
                      row, col, mv, px, py, pw, ph, bw, bh, w, h, i) \
    mc_chroma_scaled(td, s->dsp.s##mc, s->dsp.mc, dstu, dstv, dst_ls, srcu, srcu_ls, srcv, srcv_ls, tref, \
                     row, col, mv, px, py, pw, ph, bw, bh, w, h, bytesperpixel, \
                     s->mvscale[b->ref[i]], s->mvstep[b->ref[i]])
#define SCALED 1
#define FN(x) x##_scaled_8bpp
#define BYTES_PER_PIXEL 1
#include "vp9_mc_template.c"
#undef FN
#undef BYTES_PER_PIXEL
#define FN(x) x##_scaled_16bpp
#define BYTES_PER_PIXEL 2
#include "vp9_mc_template.c"
#undef mc_luma_dir
#undef mc_chroma_dir
#undef FN
#undef BYTES_PER_PIXEL
#undef SCALED

{

        if (!s->td->error_info) {
            s->td->error_info = AVERROR_INVALIDDATA;
            av_log(NULL, AV_LOG_ERROR, "Bitstream not supported, "
                                       "reference frame has invalid dimensions\n");
        }
        return;
    }

        } else {
            inter_pred_scaled_16bpp(td);
        }
    } else {
        } else {
        }
    }

        /* mostly copied intra_recon() */


        // y itxfm add
            uint8_t *ptr = dst;

            }
        }

        // uv itxfm add
                uint8_t *ptr = dst;

                }
            }
        }
    }
}

{

{
