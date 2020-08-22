/*
 * MPEG-4 decoder
 * Copyright (c) 2000,2001 Fabrice Bellard
 * Copyright (c) 2002-2010 Michael Niedermayer <michaelni@gmx.at>
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

#define UNCHECKED_BITSTREAM_READER 1

#include "libavutil/internal.h"
#include "libavutil/opt.h"
#include "libavutil/pixdesc.h"
#include "error_resilience.h"
#include "hwconfig.h"
#include "idctdsp.h"
#include "internal.h"
#include "mpegutils.h"
#include "mpegvideo.h"
#include "mpegvideodata.h"
#include "mpeg4video.h"
#include "h263.h"
#include "profiles.h"
#include "thread.h"
#include "xvididct.h"
#include "unary.h"

/* The defines below define the number of bits that are read at once for
 * reading vlc values. Changing these may improve speed and data cache needs
 * be aware though that decreasing them may need the number of stages that is
 * passed to get_vlc* to be increased. */
#define SPRITE_TRAJ_VLC_BITS 6
#define DC_VLC_BITS 9
#define MB_TYPE_B_VLC_BITS 4
#define STUDIO_INTRA_BITS 9

static int decode_studio_vol_header(Mpeg4DecContext *ctx, GetBitContext *gb);

static VLC dc_lum, dc_chrom;
static VLC sprite_trajectory;
static VLC mb_type_b_vlc;

static const int mb_type_b_map[4] = {
    MB_TYPE_DIRECT2 | MB_TYPE_L0L1,
    MB_TYPE_L0L1    | MB_TYPE_16x16,
    MB_TYPE_L1      | MB_TYPE_16x16,
    MB_TYPE_L0      | MB_TYPE_16x16,
};

/**
 * Predict the ac.
 * @param n block index (0-3 are luma, 4-5 are chroma)
 * @param dir the ac prediction direction
 */
{

    /* find prediction */
            /* left prediction */

                n == 1 || n == 3) {
                /* same qscale */
            } else {
                /* different qscale, we must rescale */
                for (i = 1; i < 8; i++)
                    block[s->idsp.idct_permutation[i << 3]] += ROUNDED_DIV(ac_val[i] * qscale_table[xy], s->qscale);
            }
        } else {
            /* top prediction */

                /* same qscale */
            } else {
                /* different qscale, we must rescale */
                for (i = 1; i < 8; i++)
                    block[s->idsp.idct_permutation[i]] += ROUNDED_DIV(ac_val[i + 8] * qscale_table[xy], s->qscale);
            }
        }
    }
    /* left copy */

    /* top copy */

/**
 * check if the next stuff is a resync marker or the end.
 * @return 0 if not
 */
{

        return 0;

            break;
        skip_bits(&s->gb, 8 + s->pict_type);
        bits_count += 8 + s->pict_type;
        v = show_bits(&s->gb, 16);
    }


    } else {


                    break;

                mb_num= -1;


        }
    }
    return 0;
}

static int mpeg4_decode_sprite_trajectory(Mpeg4DecContext *ctx, GetBitContext *gb)
{
    MpegEncContext *s = &ctx->m;
    int a     = 2 << s->sprite_warping_accuracy;
    int rho   = 3  - s->sprite_warping_accuracy;
    int r     = 16 / a;
    int alpha = 1;
    int beta  = 0;
    int w     = s->width;
    int h     = s->height;
    int min_ab, i, w2, h2, w3, h3;
    int sprite_ref[4][2];
    int virtual_ref[2][2];
    int64_t sprite_offset[2][2];
    int64_t sprite_delta[2][2];

    // only true for rectangle shapes
    const int vop_ref[4][2] = { { 0, 0 },         { s->width, 0 },
                                { 0, s->height }, { s->width, s->height } };
    int d[4][2]             = { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } };

    if (w <= 0 || h <= 0)
        return AVERROR_INVALIDDATA;

    /* the decoder was not properly initialized and we cannot continue */
    if (sprite_trajectory.table == NULL)
        return AVERROR_INVALIDDATA;

    for (i = 0; i < ctx->num_sprite_warping_points; i++) {
        int length;
        int x = 0, y = 0;

        length = get_vlc2(gb, sprite_trajectory.table, SPRITE_TRAJ_VLC_BITS, 3);
        if (length > 0)
            x = get_xbits(gb, length);

        if (!(ctx->divx_version == 500 && ctx->divx_build == 413))
            check_marker(s->avctx, gb, "before sprite_trajectory");

        length = get_vlc2(gb, sprite_trajectory.table, SPRITE_TRAJ_VLC_BITS, 3);
        if (length > 0)
            y = get_xbits(gb, length);

        check_marker(s->avctx, gb, "after sprite_trajectory");
        ctx->sprite_traj[i][0] = d[i][0] = x;
        ctx->sprite_traj[i][1] = d[i][1] = y;
    }
    for (; i < 4; i++)
        ctx->sprite_traj[i][0] = ctx->sprite_traj[i][1] = 0;

    while ((1 << alpha) < w)
        alpha++;
    while ((1 << beta) < h)
        beta++;  /* typo in the MPEG-4 std for the definition of w' and h' */
    w2 = 1 << alpha;
    h2 = 1 << beta;

    // Note, the 4th point isn't used for GMC
    if (ctx->divx_version == 500 && ctx->divx_build == 413) {
        sprite_ref[0][0] = a * vop_ref[0][0] + d[0][0];
        sprite_ref[0][1] = a * vop_ref[0][1] + d[0][1];
        sprite_ref[1][0] = a * vop_ref[1][0] + d[0][0] + d[1][0];
        sprite_ref[1][1] = a * vop_ref[1][1] + d[0][1] + d[1][1];
        sprite_ref[2][0] = a * vop_ref[2][0] + d[0][0] + d[2][0];
        sprite_ref[2][1] = a * vop_ref[2][1] + d[0][1] + d[2][1];
    } else {
        sprite_ref[0][0] = (a >> 1) * (2 * vop_ref[0][0] + d[0][0]);
        sprite_ref[0][1] = (a >> 1) * (2 * vop_ref[0][1] + d[0][1]);
        sprite_ref[1][0] = (a >> 1) * (2 * vop_ref[1][0] + d[0][0] + d[1][0]);
        sprite_ref[1][1] = (a >> 1) * (2 * vop_ref[1][1] + d[0][1] + d[1][1]);
        sprite_ref[2][0] = (a >> 1) * (2 * vop_ref[2][0] + d[0][0] + d[2][0]);
        sprite_ref[2][1] = (a >> 1) * (2 * vop_ref[2][1] + d[0][1] + d[2][1]);
    }
    /* sprite_ref[3][0] = (a >> 1) * (2 * vop_ref[3][0] + d[0][0] + d[1][0] + d[2][0] + d[3][0]);
     * sprite_ref[3][1] = (a >> 1) * (2 * vop_ref[3][1] + d[0][1] + d[1][1] + d[2][1] + d[3][1]); */

    /* This is mostly identical to the MPEG-4 std (and is totally unreadable
     * because of that...). Perhaps it should be reordered to be more readable.
     * The idea behind this virtual_ref mess is to be able to use shifts later
     * per pixel instead of divides so the distance between points is converted
     * from w&h based to w2&h2 based which are of the 2^x form. */
    virtual_ref[0][0] = 16 * (vop_ref[0][0] + w2) +
                         ROUNDED_DIV(((w - w2) *
                                           (r * sprite_ref[0][0] - 16LL * vop_ref[0][0]) +
                                      w2 * (r * sprite_ref[1][0] - 16LL * vop_ref[1][0])), w);
    virtual_ref[0][1] = 16 * vop_ref[0][1] +
                        ROUNDED_DIV(((w - w2) *
                                          (r * sprite_ref[0][1] - 16LL * vop_ref[0][1]) +
                                     w2 * (r * sprite_ref[1][1] - 16LL * vop_ref[1][1])), w);
    virtual_ref[1][0] = 16 * vop_ref[0][0] +
                        ROUNDED_DIV(((h - h2) * (r * sprite_ref[0][0] - 16LL * vop_ref[0][0]) +
                                           h2 * (r * sprite_ref[2][0] - 16LL * vop_ref[2][0])), h);
    virtual_ref[1][1] = 16 * (vop_ref[0][1] + h2) +
                        ROUNDED_DIV(((h - h2) * (r * sprite_ref[0][1] - 16LL * vop_ref[0][1]) +
                                           h2 * (r * sprite_ref[2][1] - 16LL * vop_ref[2][1])), h);

    switch (ctx->num_sprite_warping_points) {
    case 0:
        sprite_offset[0][0]    =
        sprite_offset[0][1]    =
        sprite_offset[1][0]    =
        sprite_offset[1][1]    = 0;
        sprite_delta[0][0]     = a;
        sprite_delta[0][1]     =
        sprite_delta[1][0]     = 0;
        sprite_delta[1][1]     = a;
        ctx->sprite_shift[0]   =
        ctx->sprite_shift[1]   = 0;
        break;
    case 1:     // GMC only
        sprite_offset[0][0]    = sprite_ref[0][0] - a * vop_ref[0][0];
        sprite_offset[0][1]    = sprite_ref[0][1] - a * vop_ref[0][1];
        sprite_offset[1][0]    = ((sprite_ref[0][0] >> 1) | (sprite_ref[0][0] & 1)) -
                                 a * (vop_ref[0][0] / 2);
        sprite_offset[1][1]    = ((sprite_ref[0][1] >> 1) | (sprite_ref[0][1] & 1)) -
                                 a * (vop_ref[0][1] / 2);
        sprite_delta[0][0]     = a;
        sprite_delta[0][1]     =
        sprite_delta[1][0]     = 0;
        sprite_delta[1][1]     = a;
        ctx->sprite_shift[0]   =
        ctx->sprite_shift[1]   = 0;
        break;
    case 2:
        sprite_offset[0][0]    = ((int64_t)      sprite_ref[0][0] * (1 << alpha + rho)) +
                                 ((int64_t) -r * sprite_ref[0][0] + virtual_ref[0][0]) *
                                 ((int64_t)        -vop_ref[0][0]) +
                                 ((int64_t)  r * sprite_ref[0][1] - virtual_ref[0][1]) *
                                 ((int64_t)        -vop_ref[0][1]) + (1 << (alpha + rho - 1));
        sprite_offset[0][1]    = ((int64_t)      sprite_ref[0][1] * (1 << alpha + rho)) +
                                 ((int64_t) -r * sprite_ref[0][1] + virtual_ref[0][1]) *
                                 ((int64_t)        -vop_ref[0][0]) +
                                 ((int64_t) -r * sprite_ref[0][0] + virtual_ref[0][0]) *
                                 ((int64_t)        -vop_ref[0][1]) + (1 << (alpha + rho - 1));
        sprite_offset[1][0]    = (((int64_t)-r * sprite_ref[0][0] + virtual_ref[0][0]) *
                                  ((int64_t)-2 *    vop_ref[0][0] + 1) +
                                  ((int64_t) r * sprite_ref[0][1] - virtual_ref[0][1]) *
                                  ((int64_t)-2 *    vop_ref[0][1] + 1) + 2 * w2 * r *
                                   (int64_t)     sprite_ref[0][0] - 16 * w2 + (1 << (alpha + rho + 1)));
        sprite_offset[1][1]    = (((int64_t)-r * sprite_ref[0][1] + virtual_ref[0][1]) *
                                  ((int64_t)-2 *    vop_ref[0][0] + 1) +
                                  ((int64_t)-r * sprite_ref[0][0] + virtual_ref[0][0]) *
                                  ((int64_t)-2 *    vop_ref[0][1] + 1) + 2 * w2 * r *
                                  (int64_t)      sprite_ref[0][1] - 16 * w2 + (1 << (alpha + rho + 1)));
        sprite_delta[0][0] = (-r * sprite_ref[0][0] + virtual_ref[0][0]);
        sprite_delta[0][1] = (+r * sprite_ref[0][1] - virtual_ref[0][1]);
        sprite_delta[1][0] = (-r * sprite_ref[0][1] + virtual_ref[0][1]);
        sprite_delta[1][1] = (-r * sprite_ref[0][0] + virtual_ref[0][0]);

        ctx->sprite_shift[0]  = alpha + rho;
        ctx->sprite_shift[1]  = alpha + rho + 2;
        break;
    case 3:
        min_ab = FFMIN(alpha, beta);
        w3     = w2 >> min_ab;
        h3     = h2 >> min_ab;
        sprite_offset[0][0]    = ((int64_t)sprite_ref[0][0] * (1 << (alpha + beta + rho - min_ab))) +
                                 ((int64_t)-r * sprite_ref[0][0] + virtual_ref[0][0]) * h3 * (-vop_ref[0][0]) +
                                 ((int64_t)-r * sprite_ref[0][0] + virtual_ref[1][0]) * w3 * (-vop_ref[0][1]) +
                                 ((int64_t)1 << (alpha + beta + rho - min_ab - 1));
        sprite_offset[0][1]    = ((int64_t)sprite_ref[0][1] * (1 << (alpha + beta + rho - min_ab))) +
                                 ((int64_t)-r * sprite_ref[0][1] + virtual_ref[0][1]) * h3 * (-vop_ref[0][0]) +
                                 ((int64_t)-r * sprite_ref[0][1] + virtual_ref[1][1]) * w3 * (-vop_ref[0][1]) +
                                 ((int64_t)1 << (alpha + beta + rho - min_ab - 1));
        sprite_offset[1][0]    = ((int64_t)-r * sprite_ref[0][0] + virtual_ref[0][0]) * h3 * (-2 * vop_ref[0][0] + 1) +
                                 ((int64_t)-r * sprite_ref[0][0] + virtual_ref[1][0]) * w3 * (-2 * vop_ref[0][1] + 1) +
                                  (int64_t)2 * w2 * h3 * r * sprite_ref[0][0] - 16 * w2 * h3 +
                                 ((int64_t)1 << (alpha + beta + rho - min_ab + 1));
        sprite_offset[1][1]    = ((int64_t)-r * sprite_ref[0][1] + virtual_ref[0][1]) * h3 * (-2 * vop_ref[0][0] + 1) +
                                 ((int64_t)-r * sprite_ref[0][1] + virtual_ref[1][1]) * w3 * (-2 * vop_ref[0][1] + 1) +
                                  (int64_t)2 * w2 * h3 * r * sprite_ref[0][1] - 16 * w2 * h3 +
                                 ((int64_t)1 << (alpha + beta + rho - min_ab + 1));
        sprite_delta[0][0] = (-r * (int64_t)sprite_ref[0][0] + virtual_ref[0][0]) * h3;
        sprite_delta[0][1] = (-r * (int64_t)sprite_ref[0][0] + virtual_ref[1][0]) * w3;
        sprite_delta[1][0] = (-r * (int64_t)sprite_ref[0][1] + virtual_ref[0][1]) * h3;
        sprite_delta[1][1] = (-r * (int64_t)sprite_ref[0][1] + virtual_ref[1][1]) * w3;

        ctx->sprite_shift[0]  = alpha + beta + rho - min_ab;
        ctx->sprite_shift[1]  = alpha + beta + rho - min_ab + 2;
        break;
    }
    /* try to simplify the situation */
    if (sprite_delta[0][0] == a << ctx->sprite_shift[0] &&
        sprite_delta[0][1] == 0 &&
        sprite_delta[1][0] == 0 &&
        sprite_delta[1][1] == a << ctx->sprite_shift[0]) {
        sprite_offset[0][0] >>= ctx->sprite_shift[0];
        sprite_offset[0][1] >>= ctx->sprite_shift[0];
        sprite_offset[1][0] >>= ctx->sprite_shift[1];
        sprite_offset[1][1] >>= ctx->sprite_shift[1];
        sprite_delta[0][0] = a;
        sprite_delta[0][1] = 0;
        sprite_delta[1][0] = 0;
        sprite_delta[1][1] = a;
        ctx->sprite_shift[0] = 0;
        ctx->sprite_shift[1] = 0;
        s->real_sprite_warping_points = 1;
    } else {
        int shift_y = 16 - ctx->sprite_shift[0];
        int shift_c = 16 - ctx->sprite_shift[1];

        for (i = 0; i < 2; i++) {
            if (shift_c < 0 || shift_y < 0 ||
                FFABS(  sprite_offset[0][i]) >= INT_MAX >> shift_y  ||
                FFABS(  sprite_offset[1][i]) >= INT_MAX >> shift_c  ||
                FFABS(   sprite_delta[0][i]) >= INT_MAX >> shift_y  ||
                FFABS(   sprite_delta[1][i]) >= INT_MAX >> shift_y
            ) {
                avpriv_request_sample(s->avctx, "Too large sprite shift, delta or offset");
                goto overflow;
            }
        }

        for (i = 0; i < 2; i++) {
            sprite_offset[0][i]    *= 1 << shift_y;
            sprite_offset[1][i]    *= 1 << shift_c;
            sprite_delta[0][i]     *= 1 << shift_y;
            sprite_delta[1][i]     *= 1 << shift_y;
            ctx->sprite_shift[i]     = 16;

        }
        for (i = 0; i < 2; i++) {
            int64_t sd[2] = {
                sprite_delta[i][0] - a * (1LL<<16),
                sprite_delta[i][1] - a * (1LL<<16)
            };

            if (llabs(sprite_offset[0][i] + sprite_delta[i][0] * (w+16LL)) >= INT_MAX ||
                llabs(sprite_offset[0][i] + sprite_delta[i][1] * (h+16LL)) >= INT_MAX ||
                llabs(sprite_offset[0][i] + sprite_delta[i][0] * (w+16LL) + sprite_delta[i][1] * (h+16LL)) >= INT_MAX ||
                llabs(sprite_delta[i][0] * (w+16LL)) >= INT_MAX ||
                llabs(sprite_delta[i][1] * (h+16LL)) >= INT_MAX ||
                llabs(sd[0]) >= INT_MAX ||
                llabs(sd[1]) >= INT_MAX ||
                llabs(sprite_offset[0][i] + sd[0] * (w+16LL)) >= INT_MAX ||
                llabs(sprite_offset[0][i] + sd[1] * (h+16LL)) >= INT_MAX ||
                llabs(sprite_offset[0][i] + sd[0] * (w+16LL) + sd[1] * (h+16LL)) >= INT_MAX
            ) {
                avpriv_request_sample(s->avctx, "Overflow on sprite points");
                goto overflow;
            }
        }
        s->real_sprite_warping_points = ctx->num_sprite_warping_points;
    }

    for (i = 0; i < 4; i++) {
        s->sprite_offset[i&1][i>>1] = sprite_offset[i&1][i>>1];
        s->sprite_delta [i&1][i>>1] = sprite_delta [i&1][i>>1];
    }

    return 0;
overflow:
    memset(s->sprite_offset, 0, sizeof(s->sprite_offset));
    memset(s->sprite_delta, 0, sizeof(s->sprite_delta));
    return AVERROR_PATCHWELCOME;
}

static int decode_new_pred(Mpeg4DecContext *ctx, GetBitContext *gb) {
    MpegEncContext *s = &ctx->m;
    int len = FFMIN(ctx->time_increment_bits + 3, 15);

    get_bits(gb, len);
    if (get_bits1(gb))
        get_bits(gb, len);
    check_marker(s->avctx, gb, "after new_pred");

    return 0;
}

/**
 * Decode the next video packet.
 * @return <0 if something went wrong
 */
{


    /* is there enough space left for a video packet + header */
        return AVERROR_INVALIDDATA;

            break;

        av_log(s->avctx, AV_LOG_ERROR, "marker does not match f_code\n");
        return AVERROR_INVALIDDATA;
    }

        header_extension = get_bits1(&s->gb);
        // FIXME more stuff here
    }

        av_log(s->avctx, AV_LOG_ERROR,
               "illegal mb_num in video packet (%d %d) \n", mb_num, s->mb_num);
        return AVERROR_INVALIDDATA;
    }


    }


        int time_incr = 0;

        while (get_bits1(&s->gb) != 0)
            time_incr++;

        check_marker(s->avctx, &s->gb, "before time_increment in video packed header");
        skip_bits(&s->gb, ctx->time_increment_bits);      /* time_increment */
        check_marker(s->avctx, &s->gb, "before vop_coding_type in video packed header");

        skip_bits(&s->gb, 2); /* vop coding type */
        // FIXME not rect stuff here

        if (ctx->shape != BIN_ONLY_SHAPE) {
            skip_bits(&s->gb, 3); /* intra dc vlc threshold */
            // FIXME don't just ignore everything
            if (s->pict_type == AV_PICTURE_TYPE_S &&
                ctx->vol_sprite_usage == GMC_SPRITE) {
                if (mpeg4_decode_sprite_trajectory(ctx, &s->gb) < 0)
                    return AVERROR_INVALIDDATA;
                av_log(s->avctx, AV_LOG_ERROR, "untested\n");
            }

            // FIXME reduced res stuff here

            if (s->pict_type != AV_PICTURE_TYPE_I) {
                int f_code = get_bits(&s->gb, 3);       /* fcode_for */
                if (f_code == 0)
                    av_log(s->avctx, AV_LOG_ERROR,
                           "Error, video packet header damaged (f_code=0)\n");
            }
            if (s->pict_type == AV_PICTURE_TYPE_B) {
                int b_code = get_bits(&s->gb, 3);
                if (b_code == 0)
                    av_log(s->avctx, AV_LOG_ERROR,
                           "Error, video packet header damaged (b_code=0)\n");
            }
        }
    }
        decode_new_pred(ctx, &s->gb);

    return 0;
}

static void reset_studio_dc_predictors(MpegEncContext *s)
{
    /* Reset DC Predictors */
    s->last_dc[0] =
    s->last_dc[1] =
    s->last_dc[2] = 1 << (s->avctx->bits_per_raw_sample + s->dct_precision + s->intra_dc_precision - 1);
}

/**
 * Decode the next video packet.
 * @return <0 if something went wrong
 */
int ff_mpeg4_decode_studio_slice_header(Mpeg4DecContext *ctx)
{
    MpegEncContext *s = &ctx->m;
    GetBitContext *gb = &s->gb;
    unsigned vlc_len;
    uint16_t mb_num;

    if (get_bits_left(gb) >= 32 && get_bits_long(gb, 32) == SLICE_START_CODE) {
        vlc_len = av_log2(s->mb_width * s->mb_height) + 1;
        mb_num = get_bits(gb, vlc_len);

        if (mb_num >= s->mb_num)
            return AVERROR_INVALIDDATA;

        s->mb_x = mb_num % s->mb_width;
        s->mb_y = mb_num / s->mb_width;

        if (ctx->shape != BIN_ONLY_SHAPE)
            s->qscale = mpeg_get_qscale(s);

        if (get_bits1(gb)) {  /* slice_extension_flag */
            skip_bits1(gb);   /* intra_slice */
            skip_bits1(gb);   /* slice_VOP_id_enable */
            skip_bits(gb, 6); /* slice_VOP_id */
            while (get_bits1(gb)) /* extra_bit_slice */
                skip_bits(gb, 8); /* extra_information_slice */
        }

        reset_studio_dc_predictors(s);
    }
    else {
        return AVERROR_INVALIDDATA;
    }

    return 0;
}

/**
 * Get the average motion vector for a GMC MB.
 * @param n either 0 for the x component or 1 for y
 * @return the average MV for a GMC MB
 */
static inline int get_amv(Mpeg4DecContext *ctx, int n)
{
    MpegEncContext *s = &ctx->m;
    int x, y, mb_v, sum, dx, dy, shift;
    int len     = 1 << (s->f_code + 4);
    const int a = s->sprite_warping_accuracy;

    if (s->workaround_bugs & FF_BUG_AMV)
        len >>= s->quarter_sample;

    if (s->real_sprite_warping_points == 1) {
        if (ctx->divx_version == 500 && ctx->divx_build == 413 && a >= s->quarter_sample)
            sum = s->sprite_offset[0][n] / (1 << (a - s->quarter_sample));
        else
            sum = RSHIFT(s->sprite_offset[0][n] * (1 << s->quarter_sample), a);
    } else {
        dx    = s->sprite_delta[n][0];
        dy    = s->sprite_delta[n][1];
        shift = ctx->sprite_shift[0];
        if (n)
            dy -= 1 << (shift + a + 1);
        else
            dx -= 1 << (shift + a + 1);
        mb_v = s->sprite_offset[0][n] + dx * s->mb_x * 16U + dy * s->mb_y * 16U;

        sum = 0;
        for (y = 0; y < 16; y++) {
            int v;

            v = mb_v + dy * y;
            // FIXME optimize
            for (x = 0; x < 16; x++) {
                sum += v >> shift;
                v   += dx;
            }
        }
        sum = RSHIFT(sum, a + 8 - s->quarter_sample);
    }

    if (sum < -len)
        sum = -len;
    else if (sum >= len)
        sum = len - 1;

    return sum;
}

/**
 * Decode the dc value.
 * @param n block index (0-3 are luma, 4-5 are chroma)
 * @param dir_ptr the prediction direction will be stored here
 * @return the quantized dc
 */
{

    else

        av_log(s->avctx, AV_LOG_ERROR, "illegal dc vlc\n");
        return AVERROR_INVALIDDATA;
    }

        level = 0;
    } else {
            if (code == 1)
                level = 2 * get_bits1(&s->gb) - 1;
            else {
                if (get_bits1(&s->gb))
                    level = get_bits(&s->gb, code - 1) + (1 << (code - 1));
                else
                    level = -get_bits(&s->gb, code - 1) - (1 << (code - 1));
            }
        } else {
        }

            if (get_bits1(&s->gb) == 0) { /* marker */
                if (s->avctx->err_recognition & (AV_EF_BITSTREAM|AV_EF_COMPLIANT)) {
                    av_log(s->avctx, AV_LOG_ERROR, "dc marker bit missing\n");
                    return AVERROR_INVALIDDATA;
                }
            }
        }
    }

}

/**
 * Decode first partition.
 * @return number of MBs decoded or <0 if an error occurred
 */
{

    /* decode first partition */




                        av_log(s->avctx, AV_LOG_ERROR,
                               "mcbpc corrupted at %d %d\n", s->mb_x, s->mb_y);
                        return AVERROR_INVALIDDATA;
                    }


                    ff_set_qscale(s, s->qscale + quant_tab[get_bits(&s->gb, 2)]);


                        av_log(s->avctx, AV_LOG_ERROR,
                               "DC corrupted at %d %d\n", s->mb_x, s->mb_y);
                        return dc;
                    }
                }
            } else { /* P/S_TYPE */


                    /* skip mb */
                        ctx->vol_sprite_usage == GMC_SPRITE) {
                        s->current_picture.mb_type[xy] = MB_TYPE_SKIP  |
                                                         MB_TYPE_16x16 |
                                                         MB_TYPE_GMC   |
                                                         MB_TYPE_L0;
                        mx = get_amv(ctx, 0);
                        my = get_amv(ctx, 1);
                    } else {
                                                         MB_TYPE_16x16 |
                                                         MB_TYPE_L0;
                    }

                }

                    av_log(s->avctx, AV_LOG_ERROR,
                           "mcbpc corrupted at %d %d\n", s->mb_x, s->mb_y);
                    return AVERROR_INVALIDDATA;
                }
                    goto try_again;



                } else {

                        ctx->vol_sprite_usage == GMC_SPRITE &&
                        (cbpc & 16) == 0)
                        s->mcsel = get_bits1(&s->gb);
                    else

                        /* 16x16 motion prediction */

                                return AVERROR_INVALIDDATA;

                                return AVERROR_INVALIDDATA;
                                                             MB_TYPE_L0;
                        } else {
                            mx = get_amv(ctx, 0);
                            my = get_amv(ctx, 1);
                            s->current_picture.mb_type[xy] = MB_TYPE_16x16 |
                                                             MB_TYPE_GMC   |
                                                             MB_TYPE_L0;
                        }

                    } else {
                                                         MB_TYPE_L0;
                                return AVERROR_INVALIDDATA;

                                return AVERROR_INVALIDDATA;
                        }
                    }
                }
            }
        }
    }

    return mb_num;
}

/**
 * decode second partition.
 * @return <0 if an error occurred
 */
{



                    av_log(s->avctx, AV_LOG_ERROR,
                           "cbpy corrupted at %d %d\n", s->mb_x, s->mb_y);
                    return AVERROR_INVALIDDATA;
                }

            } else { /* P || S_TYPE */

                        av_log(s->avctx, AV_LOG_ERROR,
                               "I cbpy corrupted at %d %d\n", s->mb_x, s->mb_y);
                        return AVERROR_INVALIDDATA;
                    }

                        ff_set_qscale(s, s->qscale + quant_tab[get_bits(&s->gb, 2)]);

                            av_log(s->avctx, AV_LOG_ERROR,
                                   "DC corrupted at %d %d\n", s->mb_x, s->mb_y);
                            return dc;
                        }
                    }
                } else {

                        av_log(s->avctx, AV_LOG_ERROR,
                               "P cbpy corrupted at %d %d\n", s->mb_x, s->mb_y);
                        return AVERROR_INVALIDDATA;
                    }

                        ff_set_qscale(s, s->qscale + quant_tab[get_bits(&s->gb, 2)]);

                }
            }
        }
            return 0;
    }
    return 0;
}

/**
 * Decode the first and second partition.
 * @return <0 if error (and sets error type in the error_status_table)
 */
{

        ff_er_add_slice(&s->er, s->resync_mb_x, s->resync_mb_y,
                        s->mb_x, s->mb_y, part_a_error);
        return mb_num ? mb_num : AVERROR_INVALIDDATA;
    }

        av_log(s->avctx, AV_LOG_ERROR, "slice below monitor ...\n");
        ff_er_add_slice(&s->er, s->resync_mb_x, s->resync_mb_y,
                        s->mb_x, s->mb_y, part_a_error);
        return AVERROR_INVALIDDATA;
    }


            av_log(s->avctx, AV_LOG_ERROR,
                   "marker missing after first I partition at %d %d\n",
                   s->mb_x, s->mb_y);
            return AVERROR_INVALIDDATA;
        }
    } else {
            av_log(s->avctx, AV_LOG_ERROR,
                   "marker missing after first P partition at %d %d\n",
                   s->mb_x, s->mb_y);
            return AVERROR_INVALIDDATA;
        }
    }

        if (s->pict_type == AV_PICTURE_TYPE_P)
            ff_er_add_slice(&s->er, s->resync_mb_x, s->resync_mb_y,
                            s->mb_x, s->mb_y, ER_DC_ERROR);
        return ret;
    } else {
    }

    return 0;
}

/**
 * Decode a block.
 * @return <0 if an error occurred
 */
                                     int n, int coded, int intra, int rvlc)
{

    // Note intra & rvlc should be optimized away if this is inlined

            /* DC coef */
                else
            } else {
                    return level;
            }
        } else {
            i = -1;
            ff_mpeg4_pred_dc(s, n, 0, &dc_pred_dir, 0);
        }

            rl     = &ff_rvlc_rl_intra;
            rl_vlc = ff_rvlc_rl_intra.rl_vlc[0];
        } else {
        }
            else
        } else {
        }
        qmul = 1;
        qadd = 0;
    } else {
        }
            rl = &ff_rvlc_rl_inter;
        else


                rl_vlc = ff_rvlc_rl_inter.rl_vlc[0];
            else
        } else {
                rl_vlc = ff_rvlc_rl_inter.rl_vlc[s->qscale];
            else
        }
    }
    {
                /* escape */
                    if (SHOW_UBITS(re, &s->gb, 1) == 0) {
                        av_log(s->avctx, AV_LOG_ERROR,
                               "1. marker bit missing in rvlc esc\n");
                        return AVERROR_INVALIDDATA;
                    }
                    SKIP_CACHE(re, &s->gb, 1);

                    last = SHOW_UBITS(re, &s->gb, 1);
                    SKIP_CACHE(re, &s->gb, 1);
                    run = SHOW_UBITS(re, &s->gb, 6);
                    SKIP_COUNTER(re, &s->gb, 1 + 1 + 6);
                    UPDATE_CACHE(re, &s->gb);

                    if (SHOW_UBITS(re, &s->gb, 1) == 0) {
                        av_log(s->avctx, AV_LOG_ERROR,
                               "2. marker bit missing in rvlc esc\n");
                        return AVERROR_INVALIDDATA;
                    }
                    SKIP_CACHE(re, &s->gb, 1);

                    level = SHOW_UBITS(re, &s->gb, 11);
                    SKIP_CACHE(re, &s->gb, 11);

                    if (SHOW_UBITS(re, &s->gb, 5) != 0x10) {
                        av_log(s->avctx, AV_LOG_ERROR, "reverse esc missing\n");
                        return AVERROR_INVALIDDATA;
                    }
                    SKIP_CACHE(re, &s->gb, 5);

                    level = level * qmul + qadd;
                    level = (level ^ SHOW_SBITS(re, &s->gb, 1)) - SHOW_SBITS(re, &s->gb, 1);
                    SKIP_COUNTER(re, &s->gb, 1 + 11 + 5 + 1);

                    i += run + 1;
                    if (last)
                        i += 192;
                } else {

                        cache ^= 0xC0000000;

                            /* third escape */

                                level = SHOW_SBITS(re, &s->gb, 12);
                                LAST_SKIP_BITS(re, &s->gb, 12);
                            } else {
                                    av_log(s->avctx, AV_LOG_ERROR,
                                           "1. marker bit missing in 3. esc\n");
                                    if (!(s->avctx->err_recognition & AV_EF_IGNORE_ERR))
                                        return AVERROR_INVALIDDATA;
                                }


                                    av_log(s->avctx, AV_LOG_ERROR,
                                           "2. marker bit missing in 3. esc\n");
                                    if (!(s->avctx->err_recognition & AV_EF_IGNORE_ERR))
                                        return AVERROR_INVALIDDATA;
                                }

                            }

#if 0
                            if (s->error_recognition >= FF_ER_COMPLIANT) {
                                const int abs_level= FFABS(level);
                                if (abs_level<=MAX_LEVEL && run<=MAX_RUN) {
                                    const int run1= run - rl->max_run[last][abs_level] - 1;
                                    if (abs_level <= rl->max_level[last][run]) {
                                        av_log(s->avctx, AV_LOG_ERROR, "illegal 3. esc, vlc encoding possible\n");
                                        return AVERROR_INVALIDDATA;
                                    }
                                    if (s->error_recognition > FF_ER_COMPLIANT) {
                                        if (abs_level <= rl->max_level[last][run]*2) {
                                            av_log(s->avctx, AV_LOG_ERROR, "illegal 3. esc, esc 1 encoding possible\n");
                                            return AVERROR_INVALIDDATA;
                                        }
                                        if (run1 >= 0 && abs_level <= rl->max_level[last][run1]) {
                                            av_log(s->avctx, AV_LOG_ERROR, "illegal 3. esc, esc 2 encoding possible\n");
                                            return AVERROR_INVALIDDATA;
                                        }
                                    }
                                }
                            }
#endif
                            else

                                if (s->avctx->err_recognition & (AV_EF_BITSTREAM|AV_EF_AGGRESSIVE)) {
                                    if (level > 2560 || level < -2560) {
                                        av_log(s->avctx, AV_LOG_ERROR,
                                               "|level| overflow in 3. esc, qp=%d\n",
                                               s->qscale);
                                        return AVERROR_INVALIDDATA;
                                    }
                                }
                                level = level < 0 ? -2048 : 2047;
                            }

                        } else {
                            /* second escape */
                        }
                    } else {
                        /* first escape */
                    }
                }
            } else {
            }
                    av_log(s->avctx, AV_LOG_ERROR,
                           "ac-tex damaged at %d %d\n", s->mb_x, s->mb_y);
                    return AVERROR_INVALIDDATA;
                }

            }

        }
    }

            block[0] = ff_mpeg4_pred_dc(s, n, block[0], &dc_pred_dir, 0);

            i -= i >> 31;  // if (i == -1) i = 0;
        }

    }
}

/**
 * decode partition C of one MB.
 * @return <0 if an error occurred
 */
{




        ff_set_qscale(s, s->current_picture.qscale_table[xy]);

        s->pict_type == AV_PICTURE_TYPE_S) {
        int i;
        }

            /* skip mb */
                && ctx->vol_sprite_usage == GMC_SPRITE) {
                s->mcsel      = 1;
                s->mb_skipped = 0;
            } else {
            }
            // s->mcsel = 0;  // FIXME do we need to init that?

            } else {
            }
        }
    } else { /* I-Frame */
    }

        /* decode each block */
                av_log(s->avctx, AV_LOG_ERROR,
                       "texture corrupted at %d %d %d\n",
                       s->mb_x, s->mb_y, s->mb_intra);
                return AVERROR_INVALIDDATA;
            }
        }
    }

    /* per-MB end of slice check */
            return SLICE_END;
        else
            return SLICE_NOEND;
    } else {
                return SLICE_END;
        }
    }
}

{


        s->pict_type == AV_PICTURE_TYPE_S) {
                /* skip mb */
                    ctx->vol_sprite_usage == GMC_SPRITE) {
                    s->current_picture.mb_type[xy] = MB_TYPE_SKIP  |
                                                     MB_TYPE_GMC   |
                                                     MB_TYPE_16x16 |
                                                     MB_TYPE_L0;
                    s->mcsel       = 1;
                    s->mv[0][0][0] = get_amv(ctx, 0);
                    s->mv[0][0][1] = get_amv(ctx, 1);
                    s->mb_skipped  = 0;
                } else {
                                                     MB_TYPE_16x16 |
                                                     MB_TYPE_L0;
                }
            }
                av_log(s->avctx, AV_LOG_ERROR,
                       "mcbpc damaged at %d %d\n", s->mb_x, s->mb_y);
                return AVERROR_INVALIDDATA;
            }


            ctx->vol_sprite_usage == GMC_SPRITE && (cbpc & 16) == 0)
            s->mcsel = get_bits1(&s->gb);
        else
            av_log(s->avctx, AV_LOG_ERROR,
                   "P cbpy damaged at %d %d\n", s->mb_x, s->mb_y);
            return AVERROR_INVALIDDATA;
        }

            (cbp || (s->workaround_bugs & FF_BUG_XVID_ILACE)))
            s->interlaced_dct = get_bits1(&s->gb);

                s->current_picture.mb_type[xy] = MB_TYPE_GMC   |
                                                 MB_TYPE_16x16 |
                                                 MB_TYPE_L0;
                /* 16x16 global motion prediction */
                s->mv_type     = MV_TYPE_16X16;
                mx             = get_amv(ctx, 0);
                my             = get_amv(ctx, 1);
                s->mv[0][0][0] = mx;
                s->mv[0][0][1] = my;
                s->current_picture.mb_type[xy] = MB_TYPE_16x8 |
                                                 MB_TYPE_L0   |
                                                 MB_TYPE_INTERLACED;
                /* 16x8 field motion prediction */
                s->mv_type = MV_TYPE_FIELD;

                s->field_select[0][0] = get_bits1(&s->gb);
                s->field_select[0][1] = get_bits1(&s->gb);

                ff_h263_pred_motion(s, 0, 0, &pred_x, &pred_y);

                for (i = 0; i < 2; i++) {
                    mx = ff_h263_decode_motion(s, pred_x, s->f_code);
                    if (mx >= 0xffff)
                        return AVERROR_INVALIDDATA;

                    my = ff_h263_decode_motion(s, pred_y / 2, s->f_code);
                    if (my >= 0xffff)
                        return AVERROR_INVALIDDATA;

                    s->mv[0][i][0] = mx;
                    s->mv[0][i][1] = my;
                }
            } else {
                /* 16x16 motion prediction */

                    return AVERROR_INVALIDDATA;


                    return AVERROR_INVALIDDATA;
            }
        } else {
                    return AVERROR_INVALIDDATA;

                    return AVERROR_INVALIDDATA;
            }
        }


            }

        }

        /* if we skipped it in the future P-frame than skip it now too */

            /* skip mb */

                                             MB_TYPE_16x16 |
                                             MB_TYPE_L0;
        }

            // like MB_TYPE_B_DIRECT but no vectors coded
            mb_type = MB_TYPE_DIRECT2 | MB_TYPE_SKIP | MB_TYPE_L0L1;
            cbp     = 0;
        } else {
                av_log(s->avctx, AV_LOG_ERROR, "illegal MB_type\n");
                return AVERROR_INVALIDDATA;
            }
                cbp = 0;
            } else {
            }

            }

                if (cbp)
                    s->interlaced_dct = get_bits1(&s->gb);

                if (!IS_DIRECT(mb_type) && get_bits1(&s->gb)) {
                    mb_type |= MB_TYPE_16x8 | MB_TYPE_INTERLACED;
                    mb_type &= ~MB_TYPE_16x16;

                    if (USES_LIST(mb_type, 0)) {
                        s->field_select[0][0] = get_bits1(&s->gb);
                        s->field_select[0][1] = get_bits1(&s->gb);
                    }
                    if (USES_LIST(mb_type, 1)) {
                        s->field_select[1][0] = get_bits1(&s->gb);
                        s->field_select[1][1] = get_bits1(&s->gb);
                    }
                }
            }



                }


                }
                s->mv_type = MV_TYPE_FIELD;

                if (USES_LIST(mb_type, 0)) {
                    s->mv_dir = MV_DIR_FORWARD;

                    for (i = 0; i < 2; i++) {
                        mx = ff_h263_decode_motion(s, s->last_mv[0][i][0], s->f_code);
                        my = ff_h263_decode_motion(s, s->last_mv[0][i][1] / 2, s->f_code);
                        s->last_mv[0][i][0] =
                        s->mv[0][i][0]      = mx;
                        s->last_mv[0][i][1] = (s->mv[0][i][1] = my) * 2;
                    }
                }

                if (USES_LIST(mb_type, 1)) {
                    s->mv_dir |= MV_DIR_BACKWARD;

                    for (i = 0; i < 2; i++) {
                        mx = ff_h263_decode_motion(s, s->last_mv[1][i][0], s->b_code);
                        my = ff_h263_decode_motion(s, s->last_mv[1][i][1] / 2, s->b_code);
                        s->last_mv[1][i][0] =
                        s->mv[1][i][0]      = mx;
                        s->last_mv[1][i][1] = (s->mv[1][i][1] = my) * 2;
                    }
                }
            }
        }

                mx =
                my = 0;
            } else {
            }

        }
    } else { /* I-Frame */
                av_log(s->avctx, AV_LOG_ERROR,
                       "I cbpc damaged at %d %d\n", s->mb_x, s->mb_y);
                return AVERROR_INVALIDDATA;
            }


        else

            av_log(s->avctx, AV_LOG_ERROR,
                   "I cbpy damaged at %d %d\n", s->mb_x, s->mb_y);
            return AVERROR_INVALIDDATA;
        }



            s->interlaced_dct = get_bits1(&s->gb);

        /* decode each block */
                return AVERROR_INVALIDDATA;
        }
    }

    /* decode each block */
            return AVERROR_INVALIDDATA;
    }

    /* per-MB end of slice check */
                return AVERROR_INVALIDDATA;
                return SLICE_END;

                                         : s->mb_y, 0);
                    return SLICE_OK;
            }

            return SLICE_END;
        }
    }

    return SLICE_OK;
}

/* As per spec, studio start code search isn't the same as the old type of start code */
static void next_start_code_studio(GetBitContext *gb)
{
    align_get_bits(gb);

    while (get_bits_left(gb) >= 24 && show_bits(gb, 24) != 0x1) {
        get_bits(gb, 8);
    }
}

/* additional_code, vlc index */
static const uint8_t ac_state_tab[22][2] =
{
    {0, 0},
    {0, 1},
    {1, 1},
    {2, 1},
    {3, 1},
    {4, 1},
    {5, 1},
    {1, 2},
    {2, 2},
    {3, 2},
    {4, 2},
    {5, 2},
    {6, 2},
    {1, 3},
    {2, 4},
    {3, 5},
    {4, 6},
    {5, 7},
    {6, 8},
    {7, 9},
    {8, 10},
    {0, 11}
};

static int mpeg4_decode_studio_block(MpegEncContext *s, int32_t block[64], int n)
{
    Mpeg4DecContext *ctx = s->avctx->priv_data;

    int cc, dct_dc_size, dct_diff, code, j, idx = 1, group = 0, run = 0,
        additional_code_len, sign, mismatch;
    VLC *cur_vlc = &ctx->studio_intra_tab[0];
    uint8_t *const scantable = s->intra_scantable.permutated;
    const uint16_t *quant_matrix;
    uint32_t flc;
    const int min = -1 *  (1 << (s->avctx->bits_per_raw_sample + 6));
    const int max =      ((1 << (s->avctx->bits_per_raw_sample + 6)) - 1);
    int shift =  3 - s->dct_precision;

    mismatch = 1;

    memset(block, 0, 64 * sizeof(int32_t));

    if (n < 4) {
        cc = 0;
        dct_dc_size = get_vlc2(&s->gb, ctx->studio_luma_dc.table, STUDIO_INTRA_BITS, 2);
        quant_matrix = s->intra_matrix;
    } else {
        cc = (n & 1) + 1;
        if (ctx->rgb)
            dct_dc_size = get_vlc2(&s->gb, ctx->studio_luma_dc.table, STUDIO_INTRA_BITS, 2);
        else
            dct_dc_size = get_vlc2(&s->gb, ctx->studio_chroma_dc.table, STUDIO_INTRA_BITS, 2);
        quant_matrix = s->chroma_intra_matrix;
    }

    if (dct_dc_size < 0) {
        av_log(s->avctx, AV_LOG_ERROR, "illegal dct_dc_size vlc\n");
        return AVERROR_INVALIDDATA;
    } else if (dct_dc_size == 0) {
        dct_diff = 0;
    } else {
        dct_diff = get_xbits(&s->gb, dct_dc_size);

        if (dct_dc_size > 8) {
            if(!check_marker(s->avctx, &s->gb, "dct_dc_size > 8"))
                return AVERROR_INVALIDDATA;
        }

    }

    s->last_dc[cc] += dct_diff;

    if (s->mpeg_quant)
        block[0] = s->last_dc[cc] * (8 >> s->intra_dc_precision);
    else
        block[0] = s->last_dc[cc] * (8 >> s->intra_dc_precision) * (8 >> s->dct_precision);
    /* TODO: support mpeg_quant for AC coefficients */

    block[0] = av_clip(block[0], min, max);
    mismatch ^= block[0];

    /* AC Coefficients */
    while (1) {
        group = get_vlc2(&s->gb, cur_vlc->table, STUDIO_INTRA_BITS, 2);

        if (group < 0) {
            av_log(s->avctx, AV_LOG_ERROR, "illegal ac coefficient group vlc\n");
            return AVERROR_INVALIDDATA;
        }

        additional_code_len = ac_state_tab[group][0];
        cur_vlc = &ctx->studio_intra_tab[ac_state_tab[group][1]];

        if (group == 0) {
            /* End of Block */
            break;
        } else if (group >= 1 && group <= 6) {
            /* Zero run length (Table B.47) */
            run = 1 << additional_code_len;
            if (additional_code_len)
                run += get_bits(&s->gb, additional_code_len);
            idx += run;
            continue;
        } else if (group >= 7 && group <= 12) {
            /* Zero run length and +/-1 level (Table B.48) */
            code = get_bits(&s->gb, additional_code_len);
            sign = code & 1;
            code >>= 1;
            run = (1 << (additional_code_len - 1)) + code;
            idx += run;
            if (idx > 63)
                return AVERROR_INVALIDDATA;
            j = scantable[idx++];
            block[j] = sign ? 1 : -1;
        } else if (group >= 13 && group <= 20) {
            /* Level value (Table B.49) */
            if (idx > 63)
                return AVERROR_INVALIDDATA;
            j = scantable[idx++];
            block[j] = get_xbits(&s->gb, additional_code_len);
        } else if (group == 21) {
            /* Escape */
            if (idx > 63)
                return AVERROR_INVALIDDATA;
            j = scantable[idx++];
            additional_code_len = s->avctx->bits_per_raw_sample + s->dct_precision + 4;
            flc = get_bits(&s->gb, additional_code_len);
            if (flc >> (additional_code_len-1))
                block[j] = -1 * (( flc ^ ((1 << additional_code_len) -1)) + 1);
            else
                block[j] = flc;
        }
        block[j] = ((block[j] * quant_matrix[j] * s->qscale) * (1 << shift)) / 16;
        block[j] = av_clip(block[j], min, max);
        mismatch ^= block[j];
    }

    block[63] ^= mismatch & 1;

    return 0;
}

static int mpeg4_decode_dpcm_macroblock(MpegEncContext *s, int16_t macroblock[256], int n)
{
    int i, j, w, h, idx = 0;
    int block_mean, rice_parameter, rice_prefix_code, rice_suffix_code,
        dpcm_residual, left, top, topleft, min_left_top, max_left_top, p, p2, output;
    h = 16 >> (n ? s->chroma_y_shift : 0);
    w = 16 >> (n ? s->chroma_x_shift : 0);

    block_mean = get_bits(&s->gb, s->avctx->bits_per_raw_sample);
    if (block_mean == 0){
        av_log(s->avctx, AV_LOG_ERROR, "Forbidden block_mean\n");
        return AVERROR_INVALIDDATA;
    }
    s->last_dc[n] = block_mean * (1 << (s->dct_precision + s->intra_dc_precision));

    rice_parameter = get_bits(&s->gb, 4);
    if (rice_parameter == 0) {
        av_log(s->avctx, AV_LOG_ERROR, "Forbidden rice_parameter\n");
        return AVERROR_INVALIDDATA;
    }

    if (rice_parameter == 15)
        rice_parameter = 0;

    if (rice_parameter > 11) {
        av_log(s->avctx, AV_LOG_ERROR, "Forbidden rice_parameter\n");
        return AVERROR_INVALIDDATA;
    }

    for (i = 0; i < h; i++) {
        output = 1 << (s->avctx->bits_per_raw_sample - 1);
        top = 1 << (s->avctx->bits_per_raw_sample - 1);

        for (j = 0; j < w; j++) {
            left = output;
            topleft = top;

            rice_prefix_code = get_unary(&s->gb, 1, 12);

            /* Escape */
            if (rice_prefix_code == 11)
                dpcm_residual = get_bits(&s->gb, s->avctx->bits_per_raw_sample);
            else {
                if (rice_prefix_code == 12) {
                    av_log(s->avctx, AV_LOG_ERROR, "Forbidden rice_prefix_code\n");
                    return AVERROR_INVALIDDATA;
                }
                rice_suffix_code = get_bitsz(&s->gb, rice_parameter);
                dpcm_residual = (rice_prefix_code << rice_parameter) + rice_suffix_code;
            }

            /* Map to a signed residual */
            if (dpcm_residual & 1)
                dpcm_residual = (-1 * dpcm_residual) >> 1;
            else
                dpcm_residual = (dpcm_residual >> 1);

            if (i != 0)
                top = macroblock[idx-w];

            p = left + top - topleft;
            min_left_top = FFMIN(left, top);
            if (p < min_left_top)
                p = min_left_top;

            max_left_top = FFMAX(left, top);
            if (p > max_left_top)
                p = max_left_top;

            p2 = (FFMIN(min_left_top, topleft) + FFMAX(max_left_top, topleft)) >> 1;
            if (p2 == p)
                p2 = block_mean;

            if (p2 > p)
                dpcm_residual *= -1;

            macroblock[idx++] = output = (dpcm_residual + p) & ((1 << s->avctx->bits_per_raw_sample) - 1);
        }
    }

    return 0;
}

static int mpeg4_decode_studio_mb(MpegEncContext *s, int16_t block_[12][64])
{
    int i;

    s->dpcm_direction = 0;

    /* StudioMacroblock */
    /* Assumes I-VOP */
    s->mb_intra = 1;
    if (get_bits1(&s->gb)) { /* compression_mode */
        /* DCT */
        /* macroblock_type, 1 or 2-bit VLC */
        if (!get_bits1(&s->gb)) {
            skip_bits1(&s->gb);
            s->qscale = mpeg_get_qscale(s);
        }

        for (i = 0; i < mpeg4_block_count[s->chroma_format]; i++) {
            if (mpeg4_decode_studio_block(s, (*s->block32)[i], i) < 0)
                return AVERROR_INVALIDDATA;
        }
    } else {
        /* DPCM */
        check_marker(s->avctx, &s->gb, "DPCM block start");
        s->dpcm_direction = get_bits1(&s->gb) ? -1 : 1;
        for (i = 0; i < 3; i++) {
            if (mpeg4_decode_dpcm_macroblock(s, (*s->dpcm_macroblock)[i], i) < 0)
                return AVERROR_INVALIDDATA;
        }
    }

    if (get_bits_left(&s->gb) >= 24 && show_bits(&s->gb, 23) == 0) {
        next_start_code_studio(&s->gb);
        return SLICE_END;
    }

    //vcon-stp9L1.bits (first frame)
    if (get_bits_left(&s->gb) == 0)
        return SLICE_END;

    //vcon-stp2L1.bits, vcon-stp3L1.bits, vcon-stp6L1.bits, vcon-stp7L1.bits, vcon-stp8L1.bits, vcon-stp10L1.bits (first frame)
    if (get_bits_left(&s->gb) < 8U && show_bits(&s->gb, get_bits_left(&s->gb)) == 0)
        return SLICE_END;

    return SLICE_OK;
}

static int mpeg4_decode_gop_header(MpegEncContext *s, GetBitContext *gb)
{
    int hours, minutes, seconds;

    if (!show_bits(gb, 23)) {
        av_log(s->avctx, AV_LOG_WARNING, "GOP header invalid\n");
        return AVERROR_INVALIDDATA;
    }

    hours   = get_bits(gb, 5);
    minutes = get_bits(gb, 6);
    check_marker(s->avctx, gb, "in gop_header");
    seconds = get_bits(gb, 6);

    s->time_base = seconds + 60*(minutes + 60*hours);

    skip_bits1(gb);
    skip_bits1(gb);

    return 0;
}

static int mpeg4_decode_profile_level(MpegEncContext *s, GetBitContext *gb, int *profile, int *level)
{

    *profile = get_bits(gb, 4);
    *level   = get_bits(gb, 4);

    // for Simple profile, level 0
    if (*profile == 0 && *level == 8) {
        *level = 0;
    }

    return 0;
}

static int mpeg4_decode_visual_object(MpegEncContext *s, GetBitContext *gb)
{
    int visual_object_type;
    int is_visual_object_identifier = get_bits1(gb);

    if (is_visual_object_identifier) {
        skip_bits(gb, 4+3);
    }
    visual_object_type = get_bits(gb, 4);

    if (visual_object_type == VOT_VIDEO_ID ||
        visual_object_type == VOT_STILL_TEXTURE_ID) {
        int video_signal_type = get_bits1(gb);
        if (video_signal_type) {
            int video_range, color_description;
            skip_bits(gb, 3); // video_format
            video_range = get_bits1(gb);
            color_description = get_bits1(gb);

            s->avctx->color_range = video_range ? AVCOL_RANGE_JPEG : AVCOL_RANGE_MPEG;

            if (color_description) {
                s->avctx->color_primaries = get_bits(gb, 8);
                s->avctx->color_trc       = get_bits(gb, 8);
                s->avctx->colorspace      = get_bits(gb, 8);
            }
        }
    }

    return 0;
}

static void mpeg4_load_default_matrices(MpegEncContext *s)
{
    int i, v;

    /* load default matrices */

    }
}

{

    /* vol header */

    /* If we are in studio profile (per vo_type), check if its all consistent
     * and if so continue pass control to decode_studio_vol_header().
     * elIf something is inconsistent, error out
     * else continue with (non studio) vol header decpoding.
     */
        s->vo_type == SIMPLE_STUDIO_VO_TYPE) {
        if (s->avctx->profile != FF_PROFILE_UNKNOWN && s->avctx->profile != FF_PROFILE_MPEG4_SIMPLE_STUDIO)
            return AVERROR_INVALIDDATA;
        s->studio_profile = 1;
        s->avctx->profile = FF_PROFILE_MPEG4_SIMPLE_STUDIO;
        return decode_studio_vol_header(ctx, gb);
        return AVERROR_PATCHWELCOME;
    }

    } else {
        vo_ver_id = 1;
    }
        s->avctx->sample_aspect_ratio.num = get_bits(gb, 8);  // par_width
        s->avctx->sample_aspect_ratio.den = get_bits(gb, 8);  // par_height
    } else {
    }

            av_log(s->avctx, AV_LOG_ERROR, "illegal chroma format\n");

            get_bits(gb, 15);   /* first_half_bitrate */
            check_marker(s->avctx, gb, "after first_half_bitrate");
            get_bits(gb, 15);   /* latter_half_bitrate */
            check_marker(s->avctx, gb, "after latter_half_bitrate");
            get_bits(gb, 15);   /* first_half_vbv_buffer_size */
            check_marker(s->avctx, gb, "after first_half_vbv_buffer_size");
            get_bits(gb, 3);    /* latter_half_vbv_buffer_size */
            get_bits(gb, 11);   /* first_half_vbv_occupancy */
            check_marker(s->avctx, gb, "after first_half_vbv_occupancy");
            get_bits(gb, 15);   /* latter_half_vbv_occupancy */
            check_marker(s->avctx, gb, "after latter_half_vbv_occupancy");
        }
    } else {
        /* is setting low delay flag only once the smartest thing to do?
         * low delay detection will not be overridden. */
            case ADV_SIMPLE_VO_TYPE:
            }
    }

        av_log(s->avctx, AV_LOG_ERROR, "only rectangular vol supported\n");
        av_log(s->avctx, AV_LOG_ERROR, "Gray shape not supported\n");
        skip_bits(gb, 4);  /* video_object_layer_shape_extension */
    }


        av_log(s->avctx, AV_LOG_ERROR, "framerate==0\n");
        return AVERROR_INVALIDDATA;
    }

        ctx->time_increment_bits = 1;


    else



            }
        }

            av_log(s->avctx, AV_LOG_INFO,           /* OBMC Disable */
                   "MPEG-4 OBMC not supported (very likely buggy encoder)\n");
        else

            av_log(s->avctx, AV_LOG_ERROR, "Static Sprites not supported\n");
            ctx->vol_sprite_usage == GMC_SPRITE) {
            if (ctx->vol_sprite_usage == STATIC_SPRITE) {
                skip_bits(gb, 13); // sprite_width
                check_marker(s->avctx, gb, "after sprite_width");
                skip_bits(gb, 13); // sprite_height
                check_marker(s->avctx, gb, "after sprite_height");
                skip_bits(gb, 13); // sprite_left
                check_marker(s->avctx, gb, "after sprite_left");
                skip_bits(gb, 13); // sprite_top
                check_marker(s->avctx, gb, "after sprite_top");
            }
            ctx->num_sprite_warping_points = get_bits(gb, 6);
            if (ctx->num_sprite_warping_points > 3) {
                av_log(s->avctx, AV_LOG_ERROR,
                       "%d sprite_warping_points\n",
                       ctx->num_sprite_warping_points);
                ctx->num_sprite_warping_points = 0;
                return AVERROR_INVALIDDATA;
            }
            s->sprite_warping_accuracy  = get_bits(gb, 2);
            ctx->sprite_brightness_change = get_bits1(gb);
            if (ctx->vol_sprite_usage == STATIC_SPRITE)
                skip_bits1(gb); // low_latency_sprite
        }
        // FIXME sadct disable bit if verid!=1 && shape not rect

            s->quant_precision = get_bits(gb, 4);   /* quant_precision */
            if (get_bits(gb, 4) != 8)               /* bits_per_pixel */
                av_log(s->avctx, AV_LOG_ERROR, "N-bit not supported\n");
            if (s->quant_precision != 5)
                av_log(s->avctx, AV_LOG_ERROR,
                       "quant precision %d\n", s->quant_precision);
            if (s->quant_precision<3 || s->quant_precision>9) {
                s->quant_precision = 5;
            }
        } else {
        }

        // FIXME a bunch of grayscale shape things

            int i, v;


            /* load custom intra matrix */
                int last = 0;
                        av_log(s->avctx, AV_LOG_ERROR, "insufficient data for custom matrix\n");
                        return AVERROR_INVALIDDATA;
                    }
                        break;

                }

                /* replicate last value */
                }
            }

            /* load custom non intra matrix */
                int last = 0;
                        av_log(s->avctx, AV_LOG_ERROR, "insufficient data for custom matrix\n");
                        return AVERROR_INVALIDDATA;
                    }
                        break;

                }

                /* replicate last value */
                }
            }

            // FIXME a bunch of grayscale shape things
        }

        else

            av_log(s->avctx, AV_LOG_ERROR, "VOL Header truncated\n");
            return AVERROR_INVALIDDATA;
        }

            int pos               = get_bits_count(gb);
            int estimation_method = get_bits(gb, 2);
            if (estimation_method < 2) {
                if (!get_bits1(gb)) {
                    ctx->cplx_estimation_trash_i += 8 * get_bits1(gb);  /* opaque */
                    ctx->cplx_estimation_trash_i += 8 * get_bits1(gb);  /* transparent */
                    ctx->cplx_estimation_trash_i += 8 * get_bits1(gb);  /* intra_cae */
                    ctx->cplx_estimation_trash_i += 8 * get_bits1(gb);  /* inter_cae */
                    ctx->cplx_estimation_trash_i += 8 * get_bits1(gb);  /* no_update */
                    ctx->cplx_estimation_trash_i += 8 * get_bits1(gb);  /* upsampling */
                }
                if (!get_bits1(gb)) {
                    ctx->cplx_estimation_trash_i += 8 * get_bits1(gb);  /* intra_blocks */
                    ctx->cplx_estimation_trash_p += 8 * get_bits1(gb);  /* inter_blocks */
                    ctx->cplx_estimation_trash_p += 8 * get_bits1(gb);  /* inter4v_blocks */
                    ctx->cplx_estimation_trash_i += 8 * get_bits1(gb);  /* not coded blocks */
                }
                if (!check_marker(s->avctx, gb, "in complexity estimation part 1")) {
                    skip_bits_long(gb, pos - get_bits_count(gb));
                    goto no_cplx_est;
                }
                if (!get_bits1(gb)) {
                    ctx->cplx_estimation_trash_i += 8 * get_bits1(gb);  /* dct_coeffs */
                    ctx->cplx_estimation_trash_i += 8 * get_bits1(gb);  /* dct_lines */
                    ctx->cplx_estimation_trash_i += 8 * get_bits1(gb);  /* vlc_syms */
                    ctx->cplx_estimation_trash_i += 4 * get_bits1(gb);  /* vlc_bits */
                }
                if (!get_bits1(gb)) {
                    ctx->cplx_estimation_trash_p += 8 * get_bits1(gb);  /* apm */
                    ctx->cplx_estimation_trash_p += 8 * get_bits1(gb);  /* npm */
                    ctx->cplx_estimation_trash_b += 8 * get_bits1(gb);  /* interpolate_mc_q */
                    ctx->cplx_estimation_trash_p += 8 * get_bits1(gb);  /* forwback_mc_q */
                    ctx->cplx_estimation_trash_p += 8 * get_bits1(gb);  /* halfpel2 */
                    ctx->cplx_estimation_trash_p += 8 * get_bits1(gb);  /* halfpel4 */
                }
                if (!check_marker(s->avctx, gb, "in complexity estimation part 2")) {
                    skip_bits_long(gb, pos - get_bits_count(gb));
                    goto no_cplx_est;
                }
                if (estimation_method == 1) {
                    ctx->cplx_estimation_trash_i += 8 * get_bits1(gb);  /* sadct */
                    ctx->cplx_estimation_trash_p += 8 * get_bits1(gb);  /* qpel */
                }
            } else
                av_log(s->avctx, AV_LOG_ERROR,
                       "Invalid Complexity estimation method %d\n",
                       estimation_method);
        } else {

        }



                av_log(s->avctx, AV_LOG_ERROR, "new pred not supported\n");
                skip_bits(gb, 2); /* requested upstream message type */
                skip_bits1(gb);   /* newpred segment type */
            }
                av_log(s->avctx, AV_LOG_ERROR,
                       "reduced resolution VOP not supported\n");
        } else {
        }


            GetBitContext bak = *gb;
            int h_sampling_factor_n;
            int h_sampling_factor_m;
            int v_sampling_factor_n;
            int v_sampling_factor_m;

            skip_bits1(gb);    // hierarchy_type
            skip_bits(gb, 4);  /* ref_layer_id */
            skip_bits1(gb);    /* ref_layer_sampling_dir */
            h_sampling_factor_n = get_bits(gb, 5);
            h_sampling_factor_m = get_bits(gb, 5);
            v_sampling_factor_n = get_bits(gb, 5);
            v_sampling_factor_m = get_bits(gb, 5);
            ctx->enhancement_type = get_bits1(gb);

            if (h_sampling_factor_n == 0 || h_sampling_factor_m == 0 ||
                v_sampling_factor_n == 0 || v_sampling_factor_m == 0) {
                /* illegal scalability header (VERY broken encoder),
                 * trying to workaround */
                ctx->scalability = 0;
                *gb            = bak;
            } else
                av_log(s->avctx, AV_LOG_ERROR, "scalability not supported\n");

            // bin shape stuff FIXME
        }
    }

        av_log(s->avctx, AV_LOG_DEBUG, "tb %d/%d, tincrbits:%d, qp_prec:%d, ps:%d, low_delay:%d  %s%s%s%s\n",
               s->avctx->framerate.den, s->avctx->framerate.num,
               ctx->time_increment_bits,
               s->quant_precision,
               s->progressive_sequence,
               s->low_delay,
               ctx->scalability ? "scalability " :"" , s->quarter_sample ? "qpel " : "",
               s->data_partitioning ? "partition " : "", ctx->rvlc ? "rvlc " : ""
        );
    }

    return 0;
}

/**
 * Decode the user data stuff in the header.
 * Also initializes divx/xvid/lavc_version/build.
 */
{

            break;
    }

    /* divx detection */
    }

    /* libavcodec detection */
                av_log(s->avctx, AV_LOG_WARNING,
                     "Unknown Lavc version string encountered, %d.%d.%d; "
                     "clamping sub-version values to 8-bits.\n",
                     ver, ver2, ver3);
            }
        }
    }
            ctx->lavc_build = 4600;
    }

    /* Xvid detection */

}

{

            s->codec_tag        == AV_RL32("SIPP"))
            ctx->xvid_build = 0;
    }

            ctx->vol_control_parameters == 0)
            ctx->divx_version = 400;  // divx 4

    }

            s->workaround_bugs |= FF_BUG_XVID_ILACE;

            s->workaround_bugs |= FF_BUG_UMP4;

            s->workaround_bugs |= FF_BUG_QPEL_CHROMA;

            s->workaround_bugs |= FF_BUG_QPEL_CHROMA2;

            s->padding_bug_score = 256 * 256 * 256 * 64;

            s->workaround_bugs |= FF_BUG_QPEL_CHROMA;

            s->workaround_bugs |= FF_BUG_EDGE;

            s->workaround_bugs |= FF_BUG_DC_CLIP;

#define SET_QPEL_FUNC(postfix1, postfix2)                           \
    s->qdsp.put_        ## postfix1 = ff_put_        ## postfix2;   \
    s->qdsp.put_no_rnd_ ## postfix1 = ff_put_no_rnd_ ## postfix2;   \
    s->qdsp.avg_        ## postfix1 = ff_avg_        ## postfix2;

            s->workaround_bugs |= FF_BUG_STD_QPEL;

            s->workaround_bugs |= FF_BUG_DIRECT_BLOCKSIZE;

            s->workaround_bugs |= FF_BUG_EDGE;

            s->workaround_bugs |= FF_BUG_DC_CLIP;

               (ctx->lavc_build < 3752037 || ctx->lavc_build > 3752191) // 3.2.1+
            )
                s->workaround_bugs |= FF_BUG_IEDGE;
        }

            s->workaround_bugs |= FF_BUG_DIRECT_BLOCKSIZE;
            s->padding_bug_score = 256 * 256 * 256 * 64;

            s->workaround_bugs |= FF_BUG_EDGE;

            s->workaround_bugs |= FF_BUG_HPEL_CHROMA;
    }

        SET_QPEL_FUNC(qpel_pixels_tab[0][5], qpel16_mc11_old_c)
        SET_QPEL_FUNC(qpel_pixels_tab[0][7], qpel16_mc31_old_c)
        SET_QPEL_FUNC(qpel_pixels_tab[0][9], qpel16_mc12_old_c)
        SET_QPEL_FUNC(qpel_pixels_tab[0][11], qpel16_mc32_old_c)
        SET_QPEL_FUNC(qpel_pixels_tab[0][13], qpel16_mc13_old_c)
        SET_QPEL_FUNC(qpel_pixels_tab[0][15], qpel16_mc33_old_c)

        SET_QPEL_FUNC(qpel_pixels_tab[1][5], qpel8_mc11_old_c)
        SET_QPEL_FUNC(qpel_pixels_tab[1][7], qpel8_mc31_old_c)
        SET_QPEL_FUNC(qpel_pixels_tab[1][9], qpel8_mc12_old_c)
        SET_QPEL_FUNC(qpel_pixels_tab[1][11], qpel8_mc32_old_c)
        SET_QPEL_FUNC(qpel_pixels_tab[1][13], qpel8_mc13_old_c)
        SET_QPEL_FUNC(qpel_pixels_tab[1][15], qpel8_mc33_old_c)
    }

        av_log(s->avctx, AV_LOG_DEBUG,
               "bugs: %X lavc_build:%d xvid_build:%d divx_version:%d divx_build:%d %s\n",
               s->workaround_bugs, ctx->lavc_build, ctx->xvid_build,
               ctx->divx_version, ctx->divx_build, s->divx_packed ? "p" : "");

    }

    return 0;
}

{

        ctx->vol_control_parameters == 0 && !(s->avctx->flags & AV_CODEC_FLAG_LOW_DELAY)) {
        av_log(s->avctx, AV_LOG_ERROR, "low_delay flag set incorrectly, clearing it\n");
        s->low_delay = 0;
    }

    else

    time_incr = 0;


               "time_increment_bits %d is invalid in relation to the current bitstream, this is likely caused by a missing VOL header\n", ctx->time_increment_bits);

                (s->pict_type == AV_PICTURE_TYPE_S &&
                 ctx->vol_sprite_usage == GMC_SPRITE)) {
                if ((show_bits(gb, ctx->time_increment_bits + 6) & 0x37) == 0x30)
                    break;
                break;
        }

               "time_increment_bits set to %d bits, based on bitstream analysis\n", ctx->time_increment_bits);
        }
    }

        time_increment = get_bits1(gb);        // FIXME investigate further
    else

            if (s->time < s->last_non_b_time) {
                /* header is not mpeg-4-compatible, broken encoder,
                 * trying to workaround */
                s->time_base++;
                s->time += s->avctx->framerate.num;
            }
        }
    } else {
            s->pp_time <= s->pp_time - s->pb_time ||
            s->pp_time <= 0) {
            /* messed up order, maybe after seeking? skipping current B-frame */
            return FRAME_SKIPPED;
        }

            ctx->t_frame = 1;  // 1/0 protection
                            ROUNDED_DIV(s->last_non_b_time - s->pp_time, ctx->t_frame)) * 2;
                return FRAME_SKIPPED;
        }
    }

        pts = ROUNDED_DIV(s->time, s->avctx->framerate.den);
    else


    /* vop coded */
            av_log(s->avctx, AV_LOG_ERROR, "vop not coded\n");
    }
        decode_new_pred(ctx, gb);

                     (s->pict_type == AV_PICTURE_TYPE_S &&
                      ctx->vol_sprite_usage == GMC_SPRITE))) {
        /* rounding type for motion estimation */
    } else {
    }
    // FIXME reduced res stuff

        if (ctx->vol_sprite_usage != 1 || s->pict_type != AV_PICTURE_TYPE_I) {
            skip_bits(gb, 13);  /* width */
            check_marker(s->avctx, gb, "after width");
            skip_bits(gb, 13);  /* height */
            check_marker(s->avctx, gb, "after height");
            skip_bits(gb, 13);  /* hor_spat_ref */
            check_marker(s->avctx, gb, "after hor_spat_ref");
            skip_bits(gb, 13);  /* ver_spat_ref */
        }
        skip_bits1(gb);         /* change_CR_disable */

        if (get_bits1(gb) != 0)
            skip_bits(gb, 8);   /* constant_alpha_value */
    }

    // FIXME complexity estimation stuff


            av_log(s->avctx, AV_LOG_ERROR, "Header truncated\n");
            return AVERROR_INVALIDDATA;
        }
        } else
    }

    } else {
    }

        if((ctx->vol_sprite_usage == STATIC_SPRITE ||
            ctx->vol_sprite_usage == GMC_SPRITE)) {
            if (mpeg4_decode_sprite_trajectory(ctx, gb) < 0)
                return AVERROR_INVALIDDATA;
            if (ctx->sprite_brightness_change)
                av_log(s->avctx, AV_LOG_ERROR,
                    "sprite_brightness_change not supported\n");
            if (ctx->vol_sprite_usage == STATIC_SPRITE)
                av_log(s->avctx, AV_LOG_ERROR, "static sprite not supported\n");
        } else {
            memset(s->sprite_offset, 0, sizeof(s->sprite_offset));
            memset(s->sprite_delta, 0, sizeof(s->sprite_delta));
        }
    }

            av_log(s->avctx, AV_LOG_ERROR,
                   "Error, header damaged or not MPEG-4 header (qscale=0)\n");
            return AVERROR_INVALIDDATA;  // makes no sense to continue, as there is nothing left from the image then
        }

                       "Error, header damaged or not MPEG-4 header (f_code=0)\n");
            }
        } else

                av_log(s->avctx, AV_LOG_ERROR,
                       "Error, header damaged or not MPEG4 header (b_code=0)\n");
                s->b_code=1;
                return AVERROR_INVALIDDATA; // makes no sense to continue, as the MV decoding will break very quickly
            }
        } else

            av_log(s->avctx, AV_LOG_DEBUG,
                   "qp:%d fc:%d,%d %s size:%d pro:%d alt:%d top:%d %spel part:%d resync:%d w:%d a:%d rnd:%d vot:%d%s dc:%d ce:%d/%d/%d time:%"PRId64" tincr:%d\n",
                   s->qscale, s->f_code, s->b_code,
                   s->pict_type == AV_PICTURE_TYPE_I ? "I" : (s->pict_type == AV_PICTURE_TYPE_P ? "P" : (s->pict_type == AV_PICTURE_TYPE_B ? "B" : "S")),
                   gb->size_in_bits,s->progressive_sequence, s->alternate_scan,
                   s->top_field_first, s->quarter_sample ? "q" : "h",
                   s->data_partitioning, ctx->resync_marker,
                   ctx->num_sprite_warping_points, s->sprite_warping_accuracy,
                   1 - s->no_rounding, s->vo_type,
                   ctx->vol_control_parameters ? " VOLC" : " ", ctx->intra_dc_threshold,
                   ctx->cplx_estimation_trash_i, ctx->cplx_estimation_trash_p,
                   ctx->cplx_estimation_trash_b,
                   s->time,
                   time_increment
                  );
        }

                skip_bits1(gb);  // vop shape coding type
        } else {
            if (ctx->enhancement_type) {
                int load_backward_shape = get_bits1(gb);
                if (load_backward_shape)
                    av_log(s->avctx, AV_LOG_ERROR,
                           "load backward shape isn't supported\n");
            }
            skip_bits(gb, 2);  // ref_select_code
        }
    }
    /* detect buggy encoders which don't set the low_delay flag
     * (divx4/xvid/opendivx). Note we cannot detect divx5 without B-frames
     * easily (although it's buggy too) */
               "looks like this file was encoded with (divx4/(old)xvid/opendivx) -> forcing low_delay flag\n");
    }


    // FIXME add short header support

        s->h_edge_pos = s->width;
        s->v_edge_pos = s->height;
    }
    return 0;
}

static int read_quant_matrix_ext(MpegEncContext *s, GetBitContext *gb)
{
    int i, j, v;

    if (get_bits1(gb)) {
        if (get_bits_left(gb) < 64*8)
            return AVERROR_INVALIDDATA;
        /* intra_quantiser_matrix */
        for (i = 0; i < 64; i++) {
            v = get_bits(gb, 8);
            j = s->idsp.idct_permutation[ff_zigzag_direct[i]];
            s->intra_matrix[j]        = v;
            s->chroma_intra_matrix[j] = v;
        }
    }

    if (get_bits1(gb)) {
        if (get_bits_left(gb) < 64*8)
            return AVERROR_INVALIDDATA;
        /* non_intra_quantiser_matrix */
        for (i = 0; i < 64; i++) {
            get_bits(gb, 8);
        }
    }

    if (get_bits1(gb)) {
        if (get_bits_left(gb) < 64*8)
            return AVERROR_INVALIDDATA;
        /* chroma_intra_quantiser_matrix */
        for (i = 0; i < 64; i++) {
            v = get_bits(gb, 8);
            j = s->idsp.idct_permutation[ff_zigzag_direct[i]];
            s->chroma_intra_matrix[j] = v;
        }
    }

    if (get_bits1(gb)) {
        if (get_bits_left(gb) < 64*8)
            return AVERROR_INVALIDDATA;
        /* chroma_non_intra_quantiser_matrix */
        for (i = 0; i < 64; i++) {
            get_bits(gb, 8);
        }
    }

    next_start_code_studio(gb);
    return 0;
}

static void extension_and_user_data(MpegEncContext *s, GetBitContext *gb, int id)
{
    uint32_t startcode;
    uint8_t extension_type;

    startcode = show_bits_long(gb, 32);
    if (startcode == USER_DATA_STARTCODE || startcode == EXT_STARTCODE) {

        if ((id == 2 || id == 4) && startcode == EXT_STARTCODE) {
            skip_bits_long(gb, 32);
            extension_type = get_bits(gb, 4);
            if (extension_type == QUANT_MATRIX_EXT_ID)
                read_quant_matrix_ext(s, gb);
        }
    }
}

static void decode_smpte_tc(Mpeg4DecContext *ctx, GetBitContext *gb)
{
    MpegEncContext *s = &ctx->m;

    skip_bits(gb, 16); /* Time_code[63..48] */
    check_marker(s->avctx, gb, "after Time_code[63..48]");
    skip_bits(gb, 16); /* Time_code[47..32] */
    check_marker(s->avctx, gb, "after Time_code[47..32]");
    skip_bits(gb, 16); /* Time_code[31..16] */
    check_marker(s->avctx, gb, "after Time_code[31..16]");
    skip_bits(gb, 16); /* Time_code[15..0] */
    check_marker(s->avctx, gb, "after Time_code[15..0]");
    skip_bits(gb, 4); /* reserved_bits */
}

/**
 * Decode the next studio vop header.
 * @return <0 if something went wrong
 */
static int decode_studio_vop_header(Mpeg4DecContext *ctx, GetBitContext *gb)
{
    MpegEncContext *s = &ctx->m;

    if (get_bits_left(gb) <= 32)
        return 0;

    s->partitioned_frame = 0;
    s->interlaced_dct = 0;
    s->decode_mb = mpeg4_decode_studio_mb;

    decode_smpte_tc(ctx, gb);

    skip_bits(gb, 10); /* temporal_reference */
    skip_bits(gb, 2); /* vop_structure */
    s->pict_type = get_bits(gb, 2) + AV_PICTURE_TYPE_I; /* vop_coding_type */
    if (get_bits1(gb)) { /* vop_coded */
        skip_bits1(gb); /* top_field_first */
        skip_bits1(gb); /* repeat_first_field */
        s->progressive_frame = get_bits1(gb) ^ 1; /* progressive_frame */
    }

    if (s->pict_type == AV_PICTURE_TYPE_I) {
        if (get_bits1(gb))
            reset_studio_dc_predictors(s);
    }

    if (ctx->shape != BIN_ONLY_SHAPE) {
        s->alternate_scan = get_bits1(gb);
        s->frame_pred_frame_dct = get_bits1(gb);
        s->dct_precision = get_bits(gb, 2);
        s->intra_dc_precision = get_bits(gb, 2);
        s->q_scale_type = get_bits1(gb);
    }

    if (s->alternate_scan) {
        ff_init_scantable(s->idsp.idct_permutation, &s->inter_scantable,   ff_alternate_vertical_scan);
        ff_init_scantable(s->idsp.idct_permutation, &s->intra_scantable,   ff_alternate_vertical_scan);
        ff_init_scantable(s->idsp.idct_permutation, &s->intra_h_scantable, ff_alternate_vertical_scan);
        ff_init_scantable(s->idsp.idct_permutation, &s->intra_v_scantable, ff_alternate_vertical_scan);
    } else {
        ff_init_scantable(s->idsp.idct_permutation, &s->inter_scantable,   ff_zigzag_direct);
        ff_init_scantable(s->idsp.idct_permutation, &s->intra_scantable,   ff_zigzag_direct);
        ff_init_scantable(s->idsp.idct_permutation, &s->intra_h_scantable, ff_alternate_horizontal_scan);
        ff_init_scantable(s->idsp.idct_permutation, &s->intra_v_scantable, ff_alternate_vertical_scan);
    }

    mpeg4_load_default_matrices(s);

    next_start_code_studio(gb);
    extension_and_user_data(s, gb, 4);

    return 0;
}

static int decode_studiovisualobject(Mpeg4DecContext *ctx, GetBitContext *gb)
{
    MpegEncContext *s = &ctx->m;
    int visual_object_type;

        skip_bits(gb, 4); /* visual_object_verid */
        visual_object_type = get_bits(gb, 4);
        if (visual_object_type != VOT_VIDEO_ID) {
            avpriv_request_sample(s->avctx, "VO type %u", visual_object_type);
            return AVERROR_PATCHWELCOME;
        }

        next_start_code_studio(gb);
        extension_and_user_data(s, gb, 1);

    return 0;
}

static int decode_studio_vol_header(Mpeg4DecContext *ctx, GetBitContext *gb)
{
    MpegEncContext *s = &ctx->m;
    int width, height;
    int bits_per_raw_sample;
    int rgb, chroma_format;

            // random_accessible_vol and video_object_type_indication have already
            // been read by the caller decode_vol_header()
            skip_bits(gb, 4); /* video_object_layer_verid */
            ctx->shape = get_bits(gb, 2); /* video_object_layer_shape */
            skip_bits(gb, 4); /* video_object_layer_shape_extension */
            skip_bits1(gb); /* progressive_sequence */
            if (ctx->shape != RECT_SHAPE) {
                avpriv_request_sample(s->avctx, "MPEG-4 Studio profile non rectangular shape");
                return AVERROR_PATCHWELCOME;
            }
            if (ctx->shape != BIN_ONLY_SHAPE) {
                rgb = get_bits1(gb); /* rgb_components */
                chroma_format = get_bits(gb, 2); /* chroma_format */
                if (!chroma_format || chroma_format == CHROMA_420 || (rgb && chroma_format == CHROMA_422)) {
                    av_log(s->avctx, AV_LOG_ERROR, "illegal chroma format\n");
                    return AVERROR_INVALIDDATA;
                }

                bits_per_raw_sample = get_bits(gb, 4); /* bit_depth */
                if (bits_per_raw_sample == 10) {
                    if (rgb) {
                        s->avctx->pix_fmt = AV_PIX_FMT_GBRP10;
                    }
                    else {
                        s->avctx->pix_fmt = chroma_format == CHROMA_422 ? AV_PIX_FMT_YUV422P10 : AV_PIX_FMT_YUV444P10;
                    }
                }
                else {
                    avpriv_request_sample(s->avctx, "MPEG-4 Studio profile bit-depth %u", bits_per_raw_sample);
                    return AVERROR_PATCHWELCOME;
                }
                if (rgb != ctx->rgb || s->chroma_format != chroma_format)
                    s->context_reinit = 1;
                s->avctx->bits_per_raw_sample = bits_per_raw_sample;
                ctx->rgb = rgb;
                s->chroma_format = chroma_format;
            }
            if (ctx->shape == RECT_SHAPE) {
                check_marker(s->avctx, gb, "before video_object_layer_width");
                width = get_bits(gb, 14); /* video_object_layer_width */
                check_marker(s->avctx, gb, "before video_object_layer_height");
                height = get_bits(gb, 14); /* video_object_layer_height */
                check_marker(s->avctx, gb, "after video_object_layer_height");

                /* Do the same check as non-studio profile */
                if (width && height) {
                    if (s->width && s->height &&
                        (s->width != width || s->height != height))
                        s->context_reinit = 1;
                    s->width  = width;
                    s->height = height;
                }
            }
            s->aspect_ratio_info = get_bits(gb, 4);
            if (s->aspect_ratio_info == FF_ASPECT_EXTENDED) {
                s->avctx->sample_aspect_ratio.num = get_bits(gb, 8);  // par_width
                s->avctx->sample_aspect_ratio.den = get_bits(gb, 8);  // par_height
            } else {
                s->avctx->sample_aspect_ratio = ff_h263_pixel_aspect[s->aspect_ratio_info];
            }
            skip_bits(gb, 4); /* frame_rate_code */
            skip_bits(gb, 15); /* first_half_bit_rate */
            check_marker(s->avctx, gb, "after first_half_bit_rate");
            skip_bits(gb, 15); /* latter_half_bit_rate */
            check_marker(s->avctx, gb, "after latter_half_bit_rate");
            skip_bits(gb, 15); /* first_half_vbv_buffer_size */
            check_marker(s->avctx, gb, "after first_half_vbv_buffer_size");
            skip_bits(gb, 3); /* latter_half_vbv_buffer_size */
            skip_bits(gb, 11); /* first_half_vbv_buffer_size */
            check_marker(s->avctx, gb, "after first_half_vbv_buffer_size");
            skip_bits(gb, 15); /* latter_half_vbv_occupancy */
            check_marker(s->avctx, gb, "after latter_half_vbv_occupancy");
            s->low_delay = get_bits1(gb);
            s->mpeg_quant = get_bits1(gb); /* mpeg2_stream */

            next_start_code_studio(gb);
            extension_and_user_data(s, gb, 2);

    return 0;
}

/**
 * Decode MPEG-4 headers.
 *
 * @param  header If set the absence of a VOP is not treated as error; otherwise, it is treated as such.
 * @return <0 if an error occurred
 *         FRAME_SKIPPED if a not coded VOP is found
 *         0 else
 */
{

    /* search next start code */

    // If we have not switched to studio profile than we also did not switch bps
    // that means something else (like a previous instance) outside set bps which
    // would be inconsistant with the currect state, thus reset it

        skip_bits(gb, 24);
        if (get_bits(gb, 8) == 0xF0)
            goto end;
    }

    startcode = 0xff;
                av_log(s->avctx, AV_LOG_VERBOSE, "frame skip %d\n", gb->size_in_bits);
                return FRAME_SKIPPED;  // divx bug
                return 0; // ordinary return value for parsing of extradata
            } else
        }

        /* use the bits after the test */


            av_log(s->avctx, AV_LOG_DEBUG, "startcode: %3X ", startcode);
            if (startcode <= 0x11F)
                av_log(s->avctx, AV_LOG_DEBUG, "Video Object Start");
            else if (startcode <= 0x12F)
                av_log(s->avctx, AV_LOG_DEBUG, "Video Object Layer Start");
            else if (startcode <= 0x13F)
                av_log(s->avctx, AV_LOG_DEBUG, "Reserved");
            else if (startcode <= 0x15F)
                av_log(s->avctx, AV_LOG_DEBUG, "FGS bp start");
            else if (startcode <= 0x1AF)
                av_log(s->avctx, AV_LOG_DEBUG, "Reserved");
            else if (startcode == 0x1B0)
                av_log(s->avctx, AV_LOG_DEBUG, "Visual Object Seq Start");
            else if (startcode == 0x1B1)
                av_log(s->avctx, AV_LOG_DEBUG, "Visual Object Seq End");
            else if (startcode == 0x1B2)
                av_log(s->avctx, AV_LOG_DEBUG, "User Data");
            else if (startcode == 0x1B3)
                av_log(s->avctx, AV_LOG_DEBUG, "Group of VOP start");
            else if (startcode == 0x1B4)
                av_log(s->avctx, AV_LOG_DEBUG, "Video Session Error");
            else if (startcode == 0x1B5)
                av_log(s->avctx, AV_LOG_DEBUG, "Visual Object Start");
            else if (startcode == 0x1B6)
                av_log(s->avctx, AV_LOG_DEBUG, "Video Object Plane start");
            else if (startcode == 0x1B7)
                av_log(s->avctx, AV_LOG_DEBUG, "slice start");
            else if (startcode == 0x1B8)
                av_log(s->avctx, AV_LOG_DEBUG, "extension start");
            else if (startcode == 0x1B9)
                av_log(s->avctx, AV_LOG_DEBUG, "fgs start");
            else if (startcode == 0x1BA)
                av_log(s->avctx, AV_LOG_DEBUG, "FBA Object start");
            else if (startcode == 0x1BB)
                av_log(s->avctx, AV_LOG_DEBUG, "FBA Object Plane start");
            else if (startcode == 0x1BC)
                av_log(s->avctx, AV_LOG_DEBUG, "Mesh Object start");
            else if (startcode == 0x1BD)
                av_log(s->avctx, AV_LOG_DEBUG, "Mesh Object Plane start");
            else if (startcode == 0x1BE)
                av_log(s->avctx, AV_LOG_DEBUG, "Still Texture Object start");
            else if (startcode == 0x1BF)
                av_log(s->avctx, AV_LOG_DEBUG, "Texture Spatial Layer start");
            else if (startcode == 0x1C0)
                av_log(s->avctx, AV_LOG_DEBUG, "Texture SNR Layer start");
            else if (startcode == 0x1C1)
                av_log(s->avctx, AV_LOG_DEBUG, "Texture Tile start");
            else if (startcode == 0x1C2)
                av_log(s->avctx, AV_LOG_DEBUG, "Texture Shape Layer start");
            else if (startcode == 0x1C3)
                av_log(s->avctx, AV_LOG_DEBUG, "stuffing start");
            else if (startcode <= 0x1C5)
                av_log(s->avctx, AV_LOG_DEBUG, "reserved");
            else if (startcode <= 0x1FF)
                av_log(s->avctx, AV_LOG_DEBUG, "System start");
            av_log(s->avctx, AV_LOG_DEBUG, " at %d\n", get_bits_count(gb));
        }

                av_log(s->avctx, AV_LOG_WARNING, "Ignoring multiple VOL headers\n");
                continue;
            }
                return ret;
                (level > 0 && level < 9)) {
                s->studio_profile = 1;
                next_start_code_studio(gb);
                extension_and_user_data(s, gb, 0);
                avpriv_request_sample(s->avctx, "Mixes studio and non studio profile\n");
                return AVERROR_PATCHWELCOME;
            }
                if ((ret = decode_studiovisualobject(ctx, gb)) < 0)
                    return ret;
            } else
            break;
        }

    }

        s->low_delay = 1;

        if (!s->avctx->bits_per_raw_sample) {
            av_log(s->avctx, AV_LOG_ERROR, "Missing VOL header\n");
            return AVERROR_INVALIDDATA;
        }
        return decode_studio_vop_header(ctx, gb);
    } else
}


                        &ff_mpeg4_DCtab_lum[0][1], 2, 1,
                        &ff_mpeg4_DCtab_lum[0][0], 2, 1, 512);
                        &ff_mpeg4_DCtab_chrom[0][1], 2, 1,
                        &ff_mpeg4_DCtab_chrom[0][0], 2, 1, 512);
                        &ff_sprite_trajectory_tab[0][1], 4, 2,
                        &ff_sprite_trajectory_tab[0][0], 4, 2, 128);
                        &ff_mb_type_b_tab[0][1], 2, 1,
                        &ff_mb_type_b_tab[0][0], 2, 1, 16);
    }

{

    /* divx 5.01+ bitstream reorder stuff */
    /* Since this clobbers the input buffer and hwaccel codecs still need the
     * data during hwaccel->end_frame we should not do this any earlier */


            int i;

                }
        }

                       "wasteful way to store B-frames ('packed B-frames'). "
                       "Consider using the mpeg4_unpack_bframes bitstream filter without encoding but stream copy to fix it.\n");
            }
                           &s->allocated_bitstream_buffer_size,
                           buf_size - current_pos);
                s->bitstream_buffer_size = 0;
                return AVERROR(ENOMEM);
            }
                   buf_size - current_pos);
        }
    }

    return 0;
}

#if HAVE_THREADS
static int mpeg4_update_thread_context(AVCodecContext *dst,
                                       const AVCodecContext *src)
{
    Mpeg4DecContext *s = dst->priv_data;
    const Mpeg4DecContext *s1 = src->priv_data;
    int init = s->m.context_initialized;

    int ret = ff_mpeg_update_thread_context(dst, src);

    if (ret < 0)
        return ret;

    // copy all the necessary fields explicitly
    s->time_increment_bits       = s1->time_increment_bits;
    s->shape                     = s1->shape;
    s->vol_sprite_usage          = s1->vol_sprite_usage;
    s->sprite_brightness_change  = s1->sprite_brightness_change;
    s->num_sprite_warping_points = s1->num_sprite_warping_points;
    s->rvlc                      = s1->rvlc;
    s->resync_marker             = s1->resync_marker;
    s->t_frame                   = s1->t_frame;
    s->new_pred                  = s1->new_pred;
    s->enhancement_type          = s1->enhancement_type;
    s->scalability               = s1->scalability;
    s->use_intra_dc_vlc          = s1->use_intra_dc_vlc;
    s->intra_dc_threshold        = s1->intra_dc_threshold;
    s->divx_version              = s1->divx_version;
    s->divx_build                = s1->divx_build;
    s->xvid_build                = s1->xvid_build;
    s->lavc_build                = s1->lavc_build;
    s->showed_packed_warning     = s1->showed_packed_warning;
    s->vol_control_parameters    = s1->vol_control_parameters;
    s->cplx_estimation_trash_i   = s1->cplx_estimation_trash_i;
    s->cplx_estimation_trash_p   = s1->cplx_estimation_trash_p;
    s->cplx_estimation_trash_b   = s1->cplx_estimation_trash_b;
    s->rgb                       = s1->rgb;

    memcpy(s->sprite_shift, s1->sprite_shift, sizeof(s1->sprite_shift));
    memcpy(s->sprite_traj,  s1->sprite_traj,  sizeof(s1->sprite_traj));

    if (CONFIG_MPEG4_DECODER && !init && s1->xvid_build >= 0)
        ff_xvid_idct_init(&s->m.idsp, dst);

    return 0;
}
#endif

{

                       &ff_mpeg4_studio_intra[i][0][1], 4, 2,
                       &ff_mpeg4_studio_intra[i][0][0], 4, 2,
                       0);

            return ret;
    }

                   &ff_mpeg4_studio_dc_luma[0][1], 4, 2,
                   &ff_mpeg4_studio_dc_luma[0][0], 4, 2,
                   0);
        return ret;

                   &ff_mpeg4_studio_dc_chroma[0][1], 4, 2,
                   &ff_mpeg4_studio_dc_chroma[0][0], 4, 2,
                   0);
        return ret;

    return 0;
}

{


        return ret;

        return ret;



}

{



}

static const AVOption mpeg4_options[] = {
    {"quarter_sample", "1/4 subpel MC", offsetof(MpegEncContext, quarter_sample), AV_OPT_TYPE_BOOL, {.i64 = 0}, 0, 1, 0},
    {"divx_packed", "divx style packed b frames", offsetof(MpegEncContext, divx_packed), AV_OPT_TYPE_BOOL, {.i64 = 0}, 0, 1, 0},
    {NULL}
};

static const AVClass mpeg4_class = {
    .class_name = "MPEG4 Video Decoder",
    .item_name  = av_default_item_name,
    .option     = mpeg4_options,
    .version    = LIBAVUTIL_VERSION_INT,
};

AVCodec ff_mpeg4_decoder = {
    .name                  = "mpeg4",
    .long_name             = NULL_IF_CONFIG_SMALL("MPEG-4 part 2"),
    .type                  = AVMEDIA_TYPE_VIDEO,
    .id                    = AV_CODEC_ID_MPEG4,
    .priv_data_size        = sizeof(Mpeg4DecContext),
    .init                  = decode_init,
    .close                 = decode_end,
    .decode                = ff_h263_decode_frame,
    .capabilities          = AV_CODEC_CAP_DRAW_HORIZ_BAND | AV_CODEC_CAP_DR1 |
                             AV_CODEC_CAP_TRUNCATED | AV_CODEC_CAP_DELAY |
                             AV_CODEC_CAP_FRAME_THREADS,
    .caps_internal         = FF_CODEC_CAP_SKIP_FRAME_FILL_PARAM |
                             FF_CODEC_CAP_ALLOCATE_PROGRESS |
                             FF_CODEC_CAP_INIT_CLEANUP,
    .flush                 = ff_mpeg_flush,
    .max_lowres            = 3,
    .pix_fmts              = ff_h263_hwaccel_pixfmt_list_420,
    .profiles              = NULL_IF_CONFIG_SMALL(ff_mpeg4_video_profiles),
    .update_thread_context = ONLY_IF_THREADS_ENABLED(mpeg4_update_thread_context),
    .priv_class = &mpeg4_class,
    .hw_configs            = (const AVCodecHWConfigInternal*[]) {
#if CONFIG_MPEG4_NVDEC_HWACCEL
                               HWACCEL_NVDEC(mpeg4),
#endif
#if CONFIG_MPEG4_VAAPI_HWACCEL
                               HWACCEL_VAAPI(mpeg4),
#endif
#if CONFIG_MPEG4_VDPAU_HWACCEL
                               HWACCEL_VDPAU(mpeg4),
#endif
#if CONFIG_MPEG4_VIDEOTOOLBOX_HWACCEL
                               HWACCEL_VIDEOTOOLBOX(mpeg4),
#endif
                               NULL
                           },
};
