/*
 * MPEG-1/2 decoder
 * Copyright (c) 2000, 2001 Fabrice Bellard
 * Copyright (c) 2002-2013 Michael Niedermayer <michaelni@gmx.at>
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
 * MPEG-1/2 decoder
 */

#define UNCHECKED_BITSTREAM_READER 1
#include <inttypes.h>

#include "libavutil/attributes.h"
#include "libavutil/imgutils.h"
#include "libavutil/internal.h"
#include "libavutil/stereo3d.h"

#include "avcodec.h"
#include "bytestream.h"
#include "error_resilience.h"
#include "hwconfig.h"
#include "idctdsp.h"
#include "internal.h"
#include "mpeg_er.h"
#include "mpeg12.h"
#include "mpeg12data.h"
#include "mpegutils.h"
#include "mpegvideo.h"
#include "mpegvideodata.h"
#include "profiles.h"
#include "thread.h"
#include "version.h"
#include "xvmc_internal.h"

typedef struct Mpeg1Context {
    MpegEncContext mpeg_enc_ctx;
    int mpeg_enc_ctx_allocated; /* true if decoding context allocated */
    int repeat_field;           /* true if we must repeat the field */
    AVPanScan pan_scan;         /* some temporary storage for the panscan */
    AVStereo3D stereo3d;
    int has_stereo3d;
    AVBufferRef *a53_buf_ref;
    uint8_t afd;
    int has_afd;
    int slice_count;
    AVRational save_aspect;
    int save_width, save_height, save_progressive_seq;
    int rc_buffer_size;
    AVRational frame_rate_ext;  /* MPEG-2 specific framerate modificator */
    int sync;                   /* Did we reach a sync point like a GOP/SEQ/KEYFrame? */
    int tmpgexs;
    int first_slice;
    int extradata_decoded;
} Mpeg1Context;

#define MB_TYPE_ZERO_MV   0x20000000

static const uint32_t ptype2mb_type[7] = {
                    MB_TYPE_INTRA,
                    MB_TYPE_L0 | MB_TYPE_CBP | MB_TYPE_ZERO_MV | MB_TYPE_16x16,
                    MB_TYPE_L0,
                    MB_TYPE_L0 | MB_TYPE_CBP,
    MB_TYPE_QUANT | MB_TYPE_INTRA,
    MB_TYPE_QUANT | MB_TYPE_L0 | MB_TYPE_CBP | MB_TYPE_ZERO_MV | MB_TYPE_16x16,
    MB_TYPE_QUANT | MB_TYPE_L0 | MB_TYPE_CBP,
};

static const uint32_t btype2mb_type[11] = {
                    MB_TYPE_INTRA,
                    MB_TYPE_L1,
                    MB_TYPE_L1   | MB_TYPE_CBP,
                    MB_TYPE_L0,
                    MB_TYPE_L0   | MB_TYPE_CBP,
                    MB_TYPE_L0L1,
                    MB_TYPE_L0L1 | MB_TYPE_CBP,
    MB_TYPE_QUANT | MB_TYPE_INTRA,
    MB_TYPE_QUANT | MB_TYPE_L1   | MB_TYPE_CBP,
    MB_TYPE_QUANT | MB_TYPE_L0   | MB_TYPE_CBP,
    MB_TYPE_QUANT | MB_TYPE_L0L1 | MB_TYPE_CBP,
};

/* as H.263, but only 17 codes */
{

        return pred;
        return 0xffff;

    }

    /* modulo decoding */
}

#define MAX_INDEX (64 - 1)
#define check_scantable_index(ctx, x)                                         \
    do {                                                                      \
        if ((x) > MAX_INDEX) {                                                \
            av_log(ctx->avctx, AV_LOG_ERROR, "ac-tex damaged at %d %d\n",     \
                   ctx->mb_x, ctx->mb_y);                                     \
            return AVERROR_INVALIDDATA;                                       \
        }                                                                     \
    } while (0)

                                           int16_t *block, int n)
{

    {
        // special case for first coefficient, no need to add second VLC table
        }
        /* now quantify & encode AC coefficients */
                       TEX_VLC_BITS, 2, 0);

                    break;
            } else {
                /* escape */
                    level = SHOW_UBITS(re, &s->gb, 8) - 256;
                    SKIP_BITS(re, &s->gb, 8);
                    level = SHOW_UBITS(re, &s->gb, 8);
                    SKIP_BITS(re, &s->gb, 8);
                }
                    break;
                } else {
                }
            }

                break;
        }
    }


}

/**
 * Changing this would eat up any speed benefits it has.
 * Do not use "fast" flag if you need the code to be robust.
 */
static inline int mpeg1_fast_decode_block_inter(MpegEncContext *s,
                                                int16_t *block, int n)
{
    int level, i, j, run;
    RLTable *rl              = &ff_rl_mpeg1;
    uint8_t *const scantable = s->intra_scantable.permutated;
    const int qscale         = s->qscale;

    {
        OPEN_READER(re, &s->gb);
        i = -1;
        // Special case for first coefficient, no need to add second VLC table.
        UPDATE_CACHE(re, &s->gb);
        if (((int32_t) GET_CACHE(re, &s->gb)) < 0) {
            level = (3 * qscale) >> 1;
            level = (level - 1) | 1;
            if (GET_CACHE(re, &s->gb) & 0x40000000)
                level = -level;
            block[0] = level;
            i++;
            SKIP_BITS(re, &s->gb, 2);
            if (((int32_t) GET_CACHE(re, &s->gb)) <= (int32_t) 0xBFFFFFFF)
                goto end;
        }

        /* now quantify & encode AC coefficients */
        for (;;) {
            GET_RL_VLC(level, run, re, &s->gb, rl->rl_vlc[0],
                       TEX_VLC_BITS, 2, 0);

            if (level != 0) {
                i += run;
                if (i > MAX_INDEX)
                    break;
                j = scantable[i];
                level = ((level * 2 + 1) * qscale) >> 1;
                level = (level - 1) | 1;
                level = (level ^ SHOW_SBITS(re, &s->gb, 1)) -
                        SHOW_SBITS(re, &s->gb, 1);
                SKIP_BITS(re, &s->gb, 1);
            } else {
                /* escape */
                run = SHOW_UBITS(re, &s->gb, 6) + 1;
                LAST_SKIP_BITS(re, &s->gb, 6);
                UPDATE_CACHE(re, &s->gb);
                level = SHOW_SBITS(re, &s->gb, 8);
                SKIP_BITS(re, &s->gb, 8);
                if (level == -128) {
                    level = SHOW_UBITS(re, &s->gb, 8) - 256;
                    SKIP_BITS(re, &s->gb, 8);
                } else if (level == 0) {
                    level = SHOW_UBITS(re, &s->gb, 8);
                    SKIP_BITS(re, &s->gb, 8);
                }
                i += run;
                if (i > MAX_INDEX)
                    break;
                j = scantable[i];
                if (level < 0) {
                    level = -level;
                    level = ((level * 2 + 1) * qscale) >> 1;
                    level = (level - 1) | 1;
                    level = -level;
                } else {
                    level = ((level * 2 + 1) * qscale) >> 1;
                    level = (level - 1) | 1;
                }
            }

            block[j] = level;
            if (((int32_t) GET_CACHE(re, &s->gb)) <= (int32_t) 0xBFFFFFFF)
                break;
            UPDATE_CACHE(re, &s->gb);
        }
end:
        LAST_SKIP_BITS(re, &s->gb, 2);
        CLOSE_READER(re, &s->gb);
    }

    check_scantable_index(s, i);

    s->block_last_index[n] = i;
    return 0;
}

                                               int16_t *block, int n)
{


    {
        else

        // Special case for first coefficient, no need to add second VLC table.
        }

        /* now quantify & encode AC coefficients */
                       TEX_VLC_BITS, 2, 0);

                    break;
            } else {
                /* escape */

                    break;
                } else {
                }
            }

                break;
        }
    }


}

/**
 * Changing this would eat up any speed benefits it has.
 * Do not use "fast" flag if you need the code to be robust.
 */
static inline int mpeg2_fast_decode_block_non_intra(MpegEncContext *s,
                                                    int16_t *block, int n)
{
    int level, i, j, run;
    RLTable *rl              = &ff_rl_mpeg1;
    uint8_t *const scantable = s->intra_scantable.permutated;
    const int qscale         = s->qscale;
    OPEN_READER(re, &s->gb);
    i = -1;

    // special case for first coefficient, no need to add second VLC table
    UPDATE_CACHE(re, &s->gb);
    if (((int32_t) GET_CACHE(re, &s->gb)) < 0) {
        level = (3 * qscale) >> 1;
        if (GET_CACHE(re, &s->gb) & 0x40000000)
            level = -level;
        block[0] = level;
        i++;
        SKIP_BITS(re, &s->gb, 2);
        if (((int32_t) GET_CACHE(re, &s->gb)) <= (int32_t) 0xBFFFFFFF)
            goto end;
    }

    /* now quantify & encode AC coefficients */
    for (;;) {
        GET_RL_VLC(level, run, re, &s->gb, rl->rl_vlc[0], TEX_VLC_BITS, 2, 0);

        if (level != 0) {
            i += run;
            if (i > MAX_INDEX)
                break;
            j = scantable[i];
            level = ((level * 2 + 1) * qscale) >> 1;
            level = (level ^ SHOW_SBITS(re, &s->gb, 1)) -
                    SHOW_SBITS(re, &s->gb, 1);
            SKIP_BITS(re, &s->gb, 1);
        } else {
            /* escape */
            run = SHOW_UBITS(re, &s->gb, 6) + 1;
            LAST_SKIP_BITS(re, &s->gb, 6);
            UPDATE_CACHE(re, &s->gb);
            level = SHOW_SBITS(re, &s->gb, 12);
            SKIP_BITS(re, &s->gb, 12);

            i += run;
            if (i > MAX_INDEX)
                break;
            j = scantable[i];
            if (level < 0) {
                level = ((-level * 2 + 1) * qscale) >> 1;
                level = -level;
            } else {
                level = ((level * 2 + 1) * qscale) >> 1;
            }
        }

        block[j] = level;
        if (((int32_t) GET_CACHE(re, &s->gb)) <= (int32_t) 0xBFFFFFFF || i > 63)
            break;

        UPDATE_CACHE(re, &s->gb);
    }
end:
    LAST_SKIP_BITS(re, &s->gb, 2);
    CLOSE_READER(re, &s->gb);

    check_scantable_index(s, i);

    s->block_last_index[n] = i;
    return 0;
}

                                           int16_t *block, int n)
{

    /* DC coefficient */
    } else {
    }
        return AVERROR_INVALIDDATA;
        rl = &ff_rl_mpeg2;
    else

    {
        /* now quantify & encode AC coefficients */
                       TEX_VLC_BITS, 2, 0);

                break;
                    break;
            } else {
                /* escape */
                    break;
                } else {
                }
            }

        }
    }


}

/**
 * Changing this would eat up any speed benefits it has.
 * Do not use "fast" flag if you need the code to be robust.
 */
static inline int mpeg2_fast_decode_block_intra(MpegEncContext *s,
                                                int16_t *block, int n)
{
    int level, dc, diff, i, j, run;
    int component;
    RLTable *rl;
    uint8_t *const scantable = s->intra_scantable.permutated;
    const uint16_t *quant_matrix;
    const int qscale = s->qscale;

    /* DC coefficient */
    if (n < 4) {
        quant_matrix = s->intra_matrix;
        component    = 0;
    } else {
        quant_matrix = s->chroma_intra_matrix;
        component    = (n & 1) + 1;
    }
    diff = decode_dc(&s->gb, component);
    if (diff >= 0xffff)
        return AVERROR_INVALIDDATA;
    dc = s->last_dc[component];
    dc += diff;
    s->last_dc[component] = dc;
    block[0] = dc * (1 << (3 - s->intra_dc_precision));
    i = 0;
    if (s->intra_vlc_format)
        rl = &ff_rl_mpeg2;
    else
        rl = &ff_rl_mpeg1;

    {
        OPEN_READER(re, &s->gb);
        /* now quantify & encode AC coefficients */
        for (;;) {
            UPDATE_CACHE(re, &s->gb);
            GET_RL_VLC(level, run, re, &s->gb, rl->rl_vlc[0],
                       TEX_VLC_BITS, 2, 0);

            if (level >= 64 || i > 63) {
                break;
            } else if (level != 0) {
                i += run;
                j = scantable[i];
                level = (level * qscale * quant_matrix[j]) >> 4;
                level = (level ^ SHOW_SBITS(re, &s->gb, 1)) -
                        SHOW_SBITS(re, &s->gb, 1);
                LAST_SKIP_BITS(re, &s->gb, 1);
            } else {
                /* escape */
                run = SHOW_UBITS(re, &s->gb, 6) + 1;
                LAST_SKIP_BITS(re, &s->gb, 6);
                UPDATE_CACHE(re, &s->gb);
                level = SHOW_SBITS(re, &s->gb, 12);
                SKIP_BITS(re, &s->gb, 12);
                i += run;
                j = scantable[i];
                if (level < 0) {
                    level = (-level * qscale * quant_matrix[j]) >> 4;
                    level = -level;
                } else {
                    level = (level * qscale * quant_matrix[j]) >> 4;
                }
            }

            block[j] = level;
        }
        CLOSE_READER(re, &s->gb);
    }

    check_scantable_index(s, i);

    s->block_last_index[n] = i;
    return 0;
}

/******************************************/
/* decoding */

{
    else
        return 0;
}

/* motion type (for MPEG-2) */
#define MT_FIELD 1
#define MT_FRAME 2
#define MT_16X8  2
#define MT_DMV   3

static int mpeg_decode_mb(MpegEncContext *s, int16_t block[12][64])
{
    int i, j, k, cbp, val, mb_type, motion_type;
    const int mb_block_count = 4 + (1 << s->chroma_format);
    int ret;

    ff_tlog(s->avctx, "decode_mb: x=%d y=%d\n", s->mb_x, s->mb_y);

    av_assert2(s->mb_skipped == 0);

    if (s->mb_skip_run-- != 0) {
        if (s->pict_type == AV_PICTURE_TYPE_P) {
            s->mb_skipped = 1;
            s->current_picture.mb_type[s->mb_x + s->mb_y * s->mb_stride] =
                MB_TYPE_SKIP | MB_TYPE_L0 | MB_TYPE_16x16;
        } else {
            int mb_type;

            if (s->mb_x)
                mb_type = s->current_picture.mb_type[s->mb_x + s->mb_y * s->mb_stride - 1];
            else
                // FIXME not sure if this is allowed in MPEG at all
                mb_type = s->current_picture.mb_type[s->mb_width + (s->mb_y - 1) * s->mb_stride - 1];
            if (IS_INTRA(mb_type)) {
                av_log(s->avctx, AV_LOG_ERROR, "skip with previntra\n");
                return AVERROR_INVALIDDATA;
            }
            s->current_picture.mb_type[s->mb_x + s->mb_y * s->mb_stride] =
                mb_type | MB_TYPE_SKIP;

            if ((s->mv[0][0][0] | s->mv[0][0][1] | s->mv[1][0][0] | s->mv[1][0][1]) == 0)
                s->mb_skipped = 1;
        }

        return 0;
    }

    switch (s->pict_type) {
    default:
    case AV_PICTURE_TYPE_I:
        if (get_bits1(&s->gb) == 0) {
            if (get_bits1(&s->gb) == 0) {
                av_log(s->avctx, AV_LOG_ERROR,
                       "Invalid mb type in I-frame at %d %d\n",
                       s->mb_x, s->mb_y);
                return AVERROR_INVALIDDATA;
            }
            mb_type = MB_TYPE_QUANT | MB_TYPE_INTRA;
        } else {
            mb_type = MB_TYPE_INTRA;
        }
        break;
    case AV_PICTURE_TYPE_P:
        mb_type = get_vlc2(&s->gb, ff_mb_ptype_vlc.table, MB_PTYPE_VLC_BITS, 1);
        if (mb_type < 0) {
            av_log(s->avctx, AV_LOG_ERROR,
                   "Invalid mb type in P-frame at %d %d\n", s->mb_x, s->mb_y);
            return AVERROR_INVALIDDATA;
        }
        mb_type = ptype2mb_type[mb_type];
        break;
    case AV_PICTURE_TYPE_B:
        mb_type = get_vlc2(&s->gb, ff_mb_btype_vlc.table, MB_BTYPE_VLC_BITS, 1);
        if (mb_type < 0) {
            av_log(s->avctx, AV_LOG_ERROR,
                   "Invalid mb type in B-frame at %d %d\n", s->mb_x, s->mb_y);
            return AVERROR_INVALIDDATA;
        }
        mb_type = btype2mb_type[mb_type];
        break;
    }
    ff_tlog(s->avctx, "mb_type=%x\n", mb_type);
//    motion_type = 0; /* avoid warning */
    if (IS_INTRA(mb_type)) {
        s->bdsp.clear_blocks(s->block[0]);

        if (!s->chroma_y_shift)
            s->bdsp.clear_blocks(s->block[6]);

        /* compute DCT type */
        // FIXME: add an interlaced_dct coded var?
        if (s->picture_structure == PICT_FRAME &&
            !s->frame_pred_frame_dct)
            s->interlaced_dct = get_bits1(&s->gb);

        if (IS_QUANT(mb_type))
            s->qscale = mpeg_get_qscale(s);

        if (s->concealment_motion_vectors) {
            /* just parse them */
            if (s->picture_structure != PICT_FRAME)
                skip_bits1(&s->gb);  /* field select */

            s->mv[0][0][0]      =
            s->last_mv[0][0][0] =
            s->last_mv[0][1][0] = mpeg_decode_motion(s, s->mpeg_f_code[0][0],
                                                     s->last_mv[0][0][0]);
            s->mv[0][0][1]      =
            s->last_mv[0][0][1] =
            s->last_mv[0][1][1] = mpeg_decode_motion(s, s->mpeg_f_code[0][1],
                                                     s->last_mv[0][0][1]);

            check_marker(s->avctx, &s->gb, "after concealment_motion_vectors");
        } else {
            /* reset mv prediction */
            memset(s->last_mv, 0, sizeof(s->last_mv));
        }
        s->mb_intra = 1;
        // if 1, we memcpy blocks in xvmcvideo
        if ((CONFIG_MPEG1_XVMC_HWACCEL || CONFIG_MPEG2_XVMC_HWACCEL) && s->pack_pblocks)
            ff_xvmc_pack_pblocks(s, -1); // inter are always full blocks

        if (s->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
            if (s->avctx->flags2 & AV_CODEC_FLAG2_FAST) {
                for (i = 0; i < 6; i++)
                    mpeg2_fast_decode_block_intra(s, *s->pblocks[i], i);
            } else {
                for (i = 0; i < mb_block_count; i++)
                    if ((ret = mpeg2_decode_block_intra(s, *s->pblocks[i], i)) < 0)
                        return ret;
            }
        } else {
            for (i = 0; i < 6; i++) {
                ret = ff_mpeg1_decode_block_intra(&s->gb,
                                                  s->intra_matrix,
                                                  s->intra_scantable.permutated,
                                                  s->last_dc, *s->pblocks[i],
                                                  i, s->qscale);
                if (ret < 0) {
                    av_log(s->avctx, AV_LOG_ERROR, "ac-tex damaged at %d %d\n",
                           s->mb_x, s->mb_y);
                    return ret;
                }

                s->block_last_index[i] = ret;
            }
        }
    } else {
        if (mb_type & MB_TYPE_ZERO_MV) {
            av_assert2(mb_type & MB_TYPE_CBP);

            s->mv_dir = MV_DIR_FORWARD;
            if (s->picture_structure == PICT_FRAME) {
                if (s->picture_structure == PICT_FRAME
                    && !s->frame_pred_frame_dct)
                    s->interlaced_dct = get_bits1(&s->gb);
                s->mv_type = MV_TYPE_16X16;
            } else {
                s->mv_type            = MV_TYPE_FIELD;
                mb_type              |= MB_TYPE_INTERLACED;
                s->field_select[0][0] = s->picture_structure - 1;
            }

            if (IS_QUANT(mb_type))
                s->qscale = mpeg_get_qscale(s);

            s->last_mv[0][0][0] = 0;
            s->last_mv[0][0][1] = 0;
            s->last_mv[0][1][0] = 0;
            s->last_mv[0][1][1] = 0;
            s->mv[0][0][0]      = 0;
            s->mv[0][0][1]      = 0;
        } else {
            av_assert2(mb_type & MB_TYPE_L0L1);
            // FIXME decide if MBs in field pictures are MB_TYPE_INTERLACED
            /* get additional motion vector type */
            if (s->picture_structure == PICT_FRAME && s->frame_pred_frame_dct) {
                motion_type = MT_FRAME;
            } else {
                motion_type = get_bits(&s->gb, 2);
                if (s->picture_structure == PICT_FRAME && HAS_CBP(mb_type))
                    s->interlaced_dct = get_bits1(&s->gb);
            }

            if (IS_QUANT(mb_type))
                s->qscale = mpeg_get_qscale(s);

            /* motion vectors */
            s->mv_dir = (mb_type >> 13) & 3;
            ff_tlog(s->avctx, "motion_type=%d\n", motion_type);
            switch (motion_type) {
            case MT_FRAME: /* or MT_16X8 */
                if (s->picture_structure == PICT_FRAME) {
                    mb_type   |= MB_TYPE_16x16;
                    s->mv_type = MV_TYPE_16X16;
                    for (i = 0; i < 2; i++) {
                        if (USES_LIST(mb_type, i)) {
                            /* MT_FRAME */
                            s->mv[i][0][0]      =
                            s->last_mv[i][0][0] =
                            s->last_mv[i][1][0] =
                                mpeg_decode_motion(s, s->mpeg_f_code[i][0],
                                                   s->last_mv[i][0][0]);
                            s->mv[i][0][1]      =
                            s->last_mv[i][0][1] =
                            s->last_mv[i][1][1] =
                                mpeg_decode_motion(s, s->mpeg_f_code[i][1],
                                                   s->last_mv[i][0][1]);
                            /* full_pel: only for MPEG-1 */
                            if (s->full_pel[i]) {
                                s->mv[i][0][0] *= 2;
                                s->mv[i][0][1] *= 2;
                            }
                        }
                    }
                } else {
                    mb_type   |= MB_TYPE_16x8 | MB_TYPE_INTERLACED;
                    s->mv_type = MV_TYPE_16X8;
                    for (i = 0; i < 2; i++) {
                        if (USES_LIST(mb_type, i)) {
                            /* MT_16X8 */
                            for (j = 0; j < 2; j++) {
                                s->field_select[i][j] = get_bits1(&s->gb);
                                for (k = 0; k < 2; k++) {
                                    val = mpeg_decode_motion(s, s->mpeg_f_code[i][k],
                                                             s->last_mv[i][j][k]);
                                    s->last_mv[i][j][k] = val;
                                    s->mv[i][j][k]      = val;
                                }
                            }
                        }
                    }
                }
                break;
            case MT_FIELD:
                s->mv_type = MV_TYPE_FIELD;
                if (s->picture_structure == PICT_FRAME) {
                    mb_type |= MB_TYPE_16x8 | MB_TYPE_INTERLACED;
                    for (i = 0; i < 2; i++) {
                        if (USES_LIST(mb_type, i)) {
                            for (j = 0; j < 2; j++) {
                                s->field_select[i][j] = get_bits1(&s->gb);
                                val = mpeg_decode_motion(s, s->mpeg_f_code[i][0],
                                                         s->last_mv[i][j][0]);
                                s->last_mv[i][j][0] = val;
                                s->mv[i][j][0]      = val;
                                ff_tlog(s->avctx, "fmx=%d\n", val);
                                val = mpeg_decode_motion(s, s->mpeg_f_code[i][1],
                                                         s->last_mv[i][j][1] >> 1);
                                s->last_mv[i][j][1] = 2 * val;
                                s->mv[i][j][1]      = val;
                                ff_tlog(s->avctx, "fmy=%d\n", val);
                            }
                        }
                    }
                } else {
                    av_assert0(!s->progressive_sequence);
                    mb_type |= MB_TYPE_16x16 | MB_TYPE_INTERLACED;
                    for (i = 0; i < 2; i++) {
                        if (USES_LIST(mb_type, i)) {
                            s->field_select[i][0] = get_bits1(&s->gb);
                            for (k = 0; k < 2; k++) {
                                val = mpeg_decode_motion(s, s->mpeg_f_code[i][k],
                                                         s->last_mv[i][0][k]);
                                s->last_mv[i][0][k] = val;
                                s->last_mv[i][1][k] = val;
                                s->mv[i][0][k]      = val;
                            }
                        }
                    }
                }
                break;
            case MT_DMV:
                if (s->progressive_sequence){
                    av_log(s->avctx, AV_LOG_ERROR, "MT_DMV in progressive_sequence\n");
                    return AVERROR_INVALIDDATA;
                }
                s->mv_type = MV_TYPE_DMV;
                for (i = 0; i < 2; i++) {
                    if (USES_LIST(mb_type, i)) {
                        int dmx, dmy, mx, my, m;
                        const int my_shift = s->picture_structure == PICT_FRAME;

                        mx = mpeg_decode_motion(s, s->mpeg_f_code[i][0],
                                                s->last_mv[i][0][0]);
                        s->last_mv[i][0][0] = mx;
                        s->last_mv[i][1][0] = mx;
                        dmx = get_dmv(s);
                        my  = mpeg_decode_motion(s, s->mpeg_f_code[i][1],
                                                 s->last_mv[i][0][1] >> my_shift);
                        dmy = get_dmv(s);


                        s->last_mv[i][0][1] = my * (1 << my_shift);
                        s->last_mv[i][1][1] = my * (1 << my_shift);

                        s->mv[i][0][0] = mx;
                        s->mv[i][0][1] = my;
                        s->mv[i][1][0] = mx; // not used
                        s->mv[i][1][1] = my; // not used

                        if (s->picture_structure == PICT_FRAME) {
                            mb_type |= MB_TYPE_16x16 | MB_TYPE_INTERLACED;

                            // m = 1 + 2 * s->top_field_first;
                            m = s->top_field_first ? 1 : 3;

                            /* top -> top pred */
                            s->mv[i][2][0] = ((mx * m + (mx > 0)) >> 1) + dmx;
                            s->mv[i][2][1] = ((my * m + (my > 0)) >> 1) + dmy - 1;
                            m = 4 - m;
                            s->mv[i][3][0] = ((mx * m + (mx > 0)) >> 1) + dmx;
                            s->mv[i][3][1] = ((my * m + (my > 0)) >> 1) + dmy + 1;
                        } else {
                            mb_type |= MB_TYPE_16x16;

                            s->mv[i][2][0] = ((mx + (mx > 0)) >> 1) + dmx;
                            s->mv[i][2][1] = ((my + (my > 0)) >> 1) + dmy;
                            if (s->picture_structure == PICT_TOP_FIELD)
                                s->mv[i][2][1]--;
                            else
                                s->mv[i][2][1]++;
                        }
                    }
                }
                break;
            default:
                av_log(s->avctx, AV_LOG_ERROR,
                       "00 motion_type at %d %d\n", s->mb_x, s->mb_y);
                return AVERROR_INVALIDDATA;
            }
        }

        s->mb_intra = 0;
        if (HAS_CBP(mb_type)) {
            s->bdsp.clear_blocks(s->block[0]);

            cbp = get_vlc2(&s->gb, ff_mb_pat_vlc.table, MB_PAT_VLC_BITS, 1);
            if (mb_block_count > 6) {
                cbp *= 1 << mb_block_count - 6;
                cbp  |= get_bits(&s->gb, mb_block_count - 6);
                s->bdsp.clear_blocks(s->block[6]);
            }
            if (cbp <= 0) {
                av_log(s->avctx, AV_LOG_ERROR,
                       "invalid cbp %d at %d %d\n", cbp, s->mb_x, s->mb_y);
                return AVERROR_INVALIDDATA;
            }

            // if 1, we memcpy blocks in xvmcvideo
            if ((CONFIG_MPEG1_XVMC_HWACCEL || CONFIG_MPEG2_XVMC_HWACCEL) && s->pack_pblocks)
                ff_xvmc_pack_pblocks(s, cbp);

            if (s->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
                if (s->avctx->flags2 & AV_CODEC_FLAG2_FAST) {
                    for (i = 0; i < 6; i++) {
                        if (cbp & 32)
                            mpeg2_fast_decode_block_non_intra(s, *s->pblocks[i], i);
                        else
                            s->block_last_index[i] = -1;
                        cbp += cbp;
                    }
                } else {
                    cbp <<= 12 - mb_block_count;

                    for (i = 0; i < mb_block_count; i++) {
                        if (cbp & (1 << 11)) {
                            if ((ret = mpeg2_decode_block_non_intra(s, *s->pblocks[i], i)) < 0)
                                return ret;
                        } else {
                            s->block_last_index[i] = -1;
                        }
                        cbp += cbp;
                    }
                }
            } else {
                if (s->avctx->flags2 & AV_CODEC_FLAG2_FAST) {
                    for (i = 0; i < 6; i++) {
                        if (cbp & 32)
                            mpeg1_fast_decode_block_inter(s, *s->pblocks[i], i);
                        else
                            s->block_last_index[i] = -1;
                        cbp += cbp;
                    }
                } else {
                    for (i = 0; i < 6; i++) {
                        if (cbp & 32) {
                            if ((ret = mpeg1_decode_block_inter(s, *s->pblocks[i], i)) < 0)
                                return ret;
                        } else {
                            s->block_last_index[i] = -1;
                        }
                        cbp += cbp;
                    }
                }
            }
        } else {
            for (i = 0; i < 12; i++)
                s->block_last_index[i] = -1;
        }
    }

    s->current_picture.mb_type[s->mb_x + s->mb_y * s->mb_stride] = mb_type;

    return 0;
}

{




    /* we need some permutation to store matrices,
     * until the decoder sets the real permutation. */

}

#if HAVE_THREADS
static int mpeg_decode_update_thread_context(AVCodecContext *avctx,
                                             const AVCodecContext *avctx_from)
{
    Mpeg1Context *ctx = avctx->priv_data, *ctx_from = avctx_from->priv_data;
    MpegEncContext *s = &ctx->mpeg_enc_ctx, *s1 = &ctx_from->mpeg_enc_ctx;
    int err;

    if (avctx == avctx_from               ||
        !ctx_from->mpeg_enc_ctx_allocated ||
        !s1->context_initialized)
        return 0;

    err = ff_mpeg_update_thread_context(avctx, avctx_from);
    if (err)
        return err;

    if (!ctx->mpeg_enc_ctx_allocated)
        memcpy(s + 1, s1 + 1, sizeof(Mpeg1Context) - sizeof(MpegEncContext));

    if (!(s->pict_type == AV_PICTURE_TYPE_B || s->low_delay))
        s->picture_number++;

    return 0;
}
#endif

                                 const uint8_t *new_perm)
{



static const enum AVPixelFormat mpeg1_hwaccel_pixfmt_list_420[] = {
#if CONFIG_MPEG1_NVDEC_HWACCEL
    AV_PIX_FMT_CUDA,
#endif
#if CONFIG_MPEG1_XVMC_HWACCEL
    AV_PIX_FMT_XVMC,
#endif
#if CONFIG_MPEG1_VDPAU_HWACCEL
    AV_PIX_FMT_VDPAU,
#endif
    AV_PIX_FMT_YUV420P,
    AV_PIX_FMT_NONE
};

static const enum AVPixelFormat mpeg2_hwaccel_pixfmt_list_420[] = {
#if CONFIG_MPEG2_NVDEC_HWACCEL
    AV_PIX_FMT_CUDA,
#endif
#if CONFIG_MPEG2_XVMC_HWACCEL
    AV_PIX_FMT_XVMC,
#endif
#if CONFIG_MPEG2_VDPAU_HWACCEL
    AV_PIX_FMT_VDPAU,
#endif
#if CONFIG_MPEG2_DXVA2_HWACCEL
    AV_PIX_FMT_DXVA2_VLD,
#endif
#if CONFIG_MPEG2_D3D11VA_HWACCEL
    AV_PIX_FMT_D3D11VA_VLD,
    AV_PIX_FMT_D3D11,
#endif
#if CONFIG_MPEG2_VAAPI_HWACCEL
    AV_PIX_FMT_VAAPI,
#endif
#if CONFIG_MPEG2_VIDEOTOOLBOX_HWACCEL
    AV_PIX_FMT_VIDEOTOOLBOX,
#endif
    AV_PIX_FMT_YUV420P,
    AV_PIX_FMT_NONE
};

static const enum AVPixelFormat mpeg12_pixfmt_list_422[] = {
    AV_PIX_FMT_YUV422P,
    AV_PIX_FMT_NONE
};

static const enum AVPixelFormat mpeg12_pixfmt_list_444[] = {
    AV_PIX_FMT_YUV444P,
    AV_PIX_FMT_NONE
};

{

        return AV_PIX_FMT_GRAY8;

                                mpeg2_hwaccel_pixfmt_list_420;
        pix_fmts = mpeg12_pixfmt_list_422;
    else
        pix_fmts = mpeg12_pixfmt_list_444;

}

{
    // until then pix_fmt may be changed right after codec init
        if (avctx->idct_algo == FF_IDCT_AUTO)
            avctx->idct_algo = FF_IDCT_NONE;

        Mpeg1Context *s1 = avctx->priv_data;
        MpegEncContext *s = &s1->mpeg_enc_ctx;

        s->pack_pblocks = 1;
    }
}

/* Call this function when we know all parameters.
 * It may be called in different places for MPEG-1 and MPEG-2. */
{

        // MPEG-1 aspect
    } else { // MPEG-2
        // MPEG-2 aspect

            /* We ignore the spec here and guess a bit as reality does not
             * match the spec, see for example res_change_ffmpeg_aspect.ts
             * and sequence-display-aspect.mpg.
             * issue1613, 621, 562 */
            } else {
// issue1613 4/3 16/9 -> 16/9
// res_change_ffmpeg_aspect.ts 4/3 225/44 ->4/3
// widescreen-issue562.mpg 4/3 16/9 -> 16/9
//                s->avctx->sample_aspect_ratio = av_mul_q(s->avctx->sample_aspect_ratio, (AVRational) {s->width, s->height});
                        ff_mpeg2_aspect[s->aspect_ratio_info].num,
                        ff_mpeg2_aspect[s->aspect_ratio_info].den);
                        s->avctx->sample_aspect_ratio.den);
            }
        } else {
                ff_mpeg2_aspect[s->aspect_ratio_info];
        }
    } // MPEG-2

                           avctx->sample_aspect_ratio) < 0) {
        av_log(avctx, AV_LOG_WARNING, "ignoring invalid SAR: %u/%u\n",
                avctx->sample_aspect_ratio.num,
                avctx->sample_aspect_ratio.den);
        avctx->sample_aspect_ratio = (AVRational){ 0, 1 };
    }

        0) {
        }

            return ret;

        }

        /* low_delay may be forced, in this case we will have B-frames
         * that behave like P-frames. */

            // MPEG-1 fps

        } else { // MPEG-2
            // MPEG-2 fps
                      &s->avctx->framerate.den,
                      1 << 30);

            default: av_assert0(0);
            }
        } // MPEG-2


        /* Quantization matrices may need reordering
         * if DCT permutation is changed. */

            return ret;


    }
    return 0;
}

                                int buf_size)
{


        return AVERROR_INVALIDDATA;

        s->pict_type == AV_PICTURE_TYPE_B) {
            return AVERROR_INVALIDDATA;
    }
            return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_DEBUG,
               "vbv_delay %d, ref %d type:%d\n", vbv_delay, ref, s->pict_type);

}

{


        s->chroma_format = 1;
        av_log(s->avctx, AV_LOG_WARNING, "Chroma format invalid\n");
    }


        s->low_delay = 1;



    }

        av_log(s->avctx, AV_LOG_DEBUG,
               "profile: %d, level: %d ps: %d cf:%d vbv buffer: %d, bitrate:%"PRId64"\n",
               s->avctx->profile, s->avctx->level, s->progressive_sequence, s->chroma_format,
               s1->rc_buffer_size, s->bit_rate);

{

    }
    // remaining 3 bits are zero padding


        av_log(s->avctx, AV_LOG_DEBUG, "sde w:%d, h:%d\n", w, h);

{

        if (s->repeat_first_field) {
            nofco++;
            if (s->top_field_first)
                nofco++;
        }
    } else {
        }
    }
    }

        av_log(s->avctx, AV_LOG_DEBUG,
               "pde (%"PRId16",%"PRId16") (%"PRId16",%"PRId16") (%"PRId16",%"PRId16")\n",
               s1->pan_scan.position[0][0], s1->pan_scan.position[0][1],
               s1->pan_scan.position[1][0], s1->pan_scan.position[1][1],
               s1->pan_scan.position[2][0], s1->pan_scan.position[2][1]);

                       uint16_t matrix1[64], int intra)
{

            av_log(s->avctx, AV_LOG_ERROR, "matrix damaged\n");
            return AVERROR_INVALIDDATA;
        }
            av_log(s->avctx, AV_LOG_DEBUG, "intra matrix specifies invalid DC quantizer %d, ignoring\n", v);
            v = 8; // needed by pink.mpg / issue1046
        }
    }
    return 0;
}

{


{

        av_log(s->avctx, AV_LOG_ERROR,
               "Missing picture start code, guessing missing values\n");
        if (s->mpeg_f_code[1][0] == 15 && s->mpeg_f_code[1][1] == 15) {
            if (s->mpeg_f_code[0][0] == 15 && s->mpeg_f_code[0][1] == 15)
                s->pict_type = AV_PICTURE_TYPE_I;
            else
                s->pict_type = AV_PICTURE_TYPE_P;
        } else
            s->pict_type = AV_PICTURE_TYPE_B;
        s->current_picture.f->pict_type = s->pict_type;
        s->current_picture.f->key_frame = s->pict_type == AV_PICTURE_TYPE_I;
    }


    } else {
    }

    /* composite display not parsed */

{

            return AVERROR_INVALIDDATA;
    }

    /* start frame decoding */

            return ret;


        /* first check if we must repeat the frame */
                if (s->top_field_first)
                    s->current_picture_ptr->f->repeat_pict = 4;
                else
                    s->current_picture_ptr->f->repeat_pict = 2;
            }
        }

                                          AV_FRAME_DATA_PANSCAN,
                                          sizeof(s1->pan_scan));
            return AVERROR(ENOMEM);

                s1->a53_buf_ref);
                av_buffer_unref(&s1->a53_buf_ref);
        }

            AVStereo3D *stereo = av_stereo3d_create_side_data(s->current_picture_ptr->f);
            if (!stereo)
                return AVERROR(ENOMEM);

            *stereo = s1->stereo3d;
            s1->has_stereo3d = 0;
        }

                                       AV_FRAME_DATA_AFD, 1);
                return AVERROR(ENOMEM);

        }

            ff_thread_finish_setup(avctx);
    } else { // second field

            av_log(s->avctx, AV_LOG_ERROR, "first field missing\n");
            return AVERROR_INVALIDDATA;
        }

            if ((ret = s->avctx->hwaccel->end_frame(s->avctx)) < 0) {
                av_log(avctx, AV_LOG_ERROR,
                       "hardware accelerator failed to decode first field\n");
                return ret;
            }
        }

        }
    }

        if ((ret = avctx->hwaccel->start_frame(avctx, buf, buf_size)) < 0)
            return ret;
    }

    return 0;
}

#define DECODE_SLICE_ERROR -1
#define DECODE_SLICE_OK     0

/**
 * Decode a slice.
 * MpegEncContext.mb_y must be set to the MB row from the startcode.
 * @return DECODE_SLICE_ERROR if the slice is damaged,
 *         DECODE_SLICE_OK if this slice is OK
 */
                             const uint8_t **buf, int buf_size)
{



        skip_bits(&s->gb, 3);



        av_log(s->avctx, AV_LOG_ERROR, "qscale == 0\n");
        return AVERROR_INVALIDDATA;
    }

    /* extra slice info */
        return AVERROR_INVALIDDATA;


        skip_bits1(&s->gb);
    } else {
                                MBINCR_VLC_BITS, 2);
                av_log(s->avctx, AV_LOG_ERROR, "first mb_incr damaged\n");
                return AVERROR_INVALIDDATA;
            }
                /* otherwise, stuffing, nothing to do */
            } else {
            }
        }
    }

        av_log(s->avctx, AV_LOG_ERROR, "initial skip overflow\n");
        return AVERROR_INVALIDDATA;
    }

        const uint8_t *buf_end, *buf_start = *buf - 4; /* include start_code */
        int start_code = -1;
        buf_end = avpriv_find_start_code(buf_start + 2, *buf + buf_size, &start_code);
        if (buf_end < *buf + buf_size)
            buf_end -= 4;
        s->mb_y = mb_y;
        if (avctx->hwaccel->decode_slice(avctx, buf_start, buf_end - buf_start) < 0)
            return DECODE_SLICE_ERROR;
        *buf = buf_end;
        return DECODE_SLICE_OK;
    }


            av_log(s->avctx, AV_LOG_DEBUG,
                   "qp:%d fc:%2d%2d%2d%2d %s %s %s %s %s dc:%d pstruct:%d fdct:%d cmv:%d qtype:%d ivlc:%d rff:%d %s\n",
                   s->qscale,
                   s->mpeg_f_code[0][0], s->mpeg_f_code[0][1],
                   s->mpeg_f_code[1][0], s->mpeg_f_code[1][1],
                   s->pict_type  == AV_PICTURE_TYPE_I ? "I" :
                   (s->pict_type == AV_PICTURE_TYPE_P ? "P" :
                   (s->pict_type == AV_PICTURE_TYPE_B ? "B" : "S")),
                   s->progressive_sequence ? "ps"  : "",
                   s->progressive_frame    ? "pf"  : "",
                   s->alternate_scan       ? "alt" : "",
                   s->top_field_first      ? "top" : "",
                   s->intra_dc_precision, s->picture_structure,
                   s->frame_pred_frame_dct, s->concealment_motion_vectors,
                   s->q_scale_type, s->intra_vlc_format,
                   s->repeat_first_field, s->chroma_420_type ? "420" : "");
        }
    }

        // If 1, we memcpy blocks in xvmcvideo.
            ff_xvmc_init_block(s); // set s->block


        // Note motion_val is normally NULL unless we want to extract the MVs.
            const int wrap = s->b8_stride;
            int xy         = s->mb_x * 2 + s->mb_y * 2 * wrap;
            int b8_xy      = 4 * (s->mb_x + s->mb_y * s->mb_stride);
            int motion_x, motion_y, dir, i;

            for (i = 0; i < 2; i++) {
                for (dir = 0; dir < 2; dir++) {
                    if (s->mb_intra ||
                        (dir == 1 && s->pict_type != AV_PICTURE_TYPE_B)) {
                        motion_x = motion_y = 0;
                    } else if (s->mv_type == MV_TYPE_16X16 ||
                               (s->mv_type == MV_TYPE_FIELD && field_pic)) {
                        motion_x = s->mv[dir][0][0];
                        motion_y = s->mv[dir][0][1];
                    } else { /* if ((s->mv_type == MV_TYPE_FIELD) || (s->mv_type == MV_TYPE_16X8)) */
                        motion_x = s->mv[dir][i][0];
                        motion_y = s->mv[dir][i][1];
                    }

                    s->current_picture.motion_val[dir][xy][0]     = motion_x;
                    s->current_picture.motion_val[dir][xy][1]     = motion_y;
                    s->current_picture.motion_val[dir][xy + 1][0] = motion_x;
                    s->current_picture.motion_val[dir][xy + 1][1] = motion_y;
                    s->current_picture.ref_index [dir][b8_xy]     =
                    s->current_picture.ref_index [dir][b8_xy + 1] = s->field_select[dir][i];
                    av_assert2(s->field_select[dir][i] == 0 ||
                               s->field_select[dir][i] == 1);
                }
                xy    += wrap;
                b8_xy += 2;
            }
        }






                             /* vbv_delay == 0xBBB || 0xE10 */;

                        av_log(avctx, AV_LOG_DEBUG, "Invalid MXF data found in video stream\n");
                        is_d10 = 1;
                    }
                        av_log(avctx, AV_LOG_DEBUG, "skipping m704 alpha (unsupported)\n");
                        goto eos;
                    }
                }

                    av_log(avctx, AV_LOG_ERROR, "end mismatch left=%d %0X at %d %d\n",
                           left, left>0 ? show_bits(&s->gb, FFMIN(left, 23)) : 0, s->mb_x, s->mb_y);
                    return AVERROR_INVALIDDATA;
                } else
            }
            // There are some files out there which are missing the last slice
            // in cases where the slice is completely outside the visible
            // area, we detect this here instead of running into the end expecting
            // more data
                left >= 0 &&
                s->mb_skip_run == -1 &&
                (!left || show_bits(&s->gb, left) == 0))
                goto eos;

        }

        /* skip mb handling */
            /* read increment again */
                                    MBINCR_VLC_BITS, 2);
                    av_log(s->avctx, AV_LOG_ERROR, "mb incr damaged\n");
                    return AVERROR_INVALIDDATA;
                }
                            av_log(s->avctx, AV_LOG_ERROR, "slice mismatch\n");
                            return AVERROR_INVALIDDATA;
                        }
                    }
                    /* otherwise, stuffing, nothing to do */
                } else {
                }
            }
                    av_log(s->avctx, AV_LOG_ERROR,
                           "skipped MB in I-frame at %d %d\n", s->mb_x, s->mb_y);
                    return AVERROR_INVALIDDATA;
                }

                /* skip mb */
                else
                    /* if P type, zero motion vector is implied */
                } else {
                    /* if B type, reuse previous vectors and directions */
                }
            }
        }
    }
        av_log(s, AV_LOG_ERROR, "overread %d\n", -get_bits_left(&s->gb));
        return AVERROR_INVALIDDATA;
    }
}

{



                ret, s->resync_mb_x, s->resync_mb_y, s->mb_x, s->mb_y,
                s->start_mb_y, s->end_mb_y, s->er.error_count);
            if (c->err_recognition & AV_EF_EXPLODE)
                return ret;
            if (s->resync_mb_x >= 0 && s->resync_mb_y >= 0)
                ff_er_add_slice(&s->er, s->resync_mb_x, s->resync_mb_y,
                                s->mb_x, s->mb_y,
                                ER_AC_ERROR | ER_DC_ERROR | ER_MV_ERROR);
        } else {
                            ER_AC_END | ER_DC_END | ER_MV_END);
        }

            return 0;

            return AVERROR_INVALIDDATA;
            mb_y += (*buf&0xE0)<<2;
            mb_y++;
            return AVERROR_INVALIDDATA;
    }
}

/**
 * Handle slice ends.
 * @return 1 if it seems to be the last slice
 */
{

        return 0;

        int ret = s->avctx->hwaccel->end_frame(s->avctx);
        if (ret < 0) {
            av_log(avctx, AV_LOG_ERROR,
                   "hardware accelerator failed to decode picture\n");
            return ret;
        }
    }

    /* end of slice reached */
        /* end of image */



                return ret;
        } else {
                s->picture_number++;
            /* latency of 1 frame for I- and P-frames */
            /* XXX: use another variable than picture_number */
                    return ret;
            }
        }

    } else {
        return 0;
    }
}

                                 const uint8_t *buf, int buf_size)
{


        av_log(avctx, AV_LOG_WARNING,
               "Invalid horizontal or vertical size value.\n");
        if (avctx->err_recognition & (AV_EF_BITSTREAM | AV_EF_COMPLIANT))
            return AVERROR_INVALIDDATA;
    }
        av_log(avctx, AV_LOG_ERROR, "aspect ratio has forbidden 0 value\n");
        if (avctx->err_recognition & (AV_EF_BITSTREAM | AV_EF_COMPLIANT))
            return AVERROR_INVALIDDATA;
    }
        av_log(avctx, AV_LOG_WARNING,
               "frame_rate_index %d is invalid\n", s->frame_rate_index);
        s->frame_rate_index = 1;
    }
        return AVERROR_INVALIDDATA;
    }


    /* get matrix */
    } else {
        }
    }
    } else {
        }
    }

        av_log(s->avctx, AV_LOG_ERROR, "sequence header damaged\n");
        return AVERROR_INVALIDDATA;
    }


    /* We set MPEG-2 parameters so that it emulates MPEG-1. */
        s->low_delay = 1;

        av_log(s->avctx, AV_LOG_DEBUG, "vbv buffer: %d, bitrate:%"PRId64", aspect_ratio_info: %d \n",
               s1->rc_buffer_size, s->bit_rate, s->aspect_ratio_info);

    return 0;
}

{

    /* start new MPEG-1 context decoding */
        ff_mpv_common_end(s);
        s1->mpeg_enc_ctx_allocated = 0;
    }


        return ret;


    }

        s->codec_id              = s->avctx->codec_id = AV_CODEC_ID_MPEG1VIDEO;
    } else {
    }
}

static int mpeg_decode_a53_cc(AVCodecContext *avctx,
                              const uint8_t *p, int buf_size)
{
    Mpeg1Context *s1 = avctx->priv_data;

    if (buf_size >= 6 &&
        p[0] == 'G' && p[1] == 'A' && p[2] == '9' && p[3] == '4' &&
        p[4] == 3 && (p[5] & 0x40)) {
        /* extract A53 Part 4 CC data */
        int cc_count = p[5] & 0x1f;
        if (cc_count > 0 && buf_size >= 7 + cc_count * 3) {
            int old_size = s1->a53_buf_ref ? s1->a53_buf_ref->size : 0;
            const uint64_t new_size = (old_size + cc_count
                                            * UINT64_C(3));
            int ret;

            if (new_size > INT_MAX)
                return AVERROR(EINVAL);

            ret = av_buffer_realloc(&s1->a53_buf_ref, new_size);
            if (ret >= 0)
                memcpy(s1->a53_buf_ref->data + old_size, p + 7, cc_count * UINT64_C(3));

            avctx->properties |= FF_CODEC_PROPERTY_CLOSED_CAPTIONS;
        }
        return 1;
    } else if (buf_size >= 2 &&
               p[0] == 0x03 && (p[1]&0x7f) == 0x01) {
        /* extract SCTE-20 CC data */
        GetBitContext gb;
        int cc_count = 0;
        int i, ret;

        init_get_bits(&gb, p + 2, buf_size - 2);
        cc_count = get_bits(&gb, 5);
        if (cc_count > 0) {
            int old_size = s1->a53_buf_ref ? s1->a53_buf_ref->size : 0;
            const uint64_t new_size = (old_size + cc_count
                                            * UINT64_C(3));
            if (new_size > INT_MAX)
                return AVERROR(EINVAL);

            ret = av_buffer_realloc(&s1->a53_buf_ref, new_size);
            if (ret >= 0) {
                uint8_t field, cc1, cc2;
                uint8_t *cap = s1->a53_buf_ref->data;

                memset(s1->a53_buf_ref->data + old_size, 0, cc_count * 3);
                for (i = 0; i < cc_count && get_bits_left(&gb) >= 26; i++) {
                    skip_bits(&gb, 2); // priority
                    field = get_bits(&gb, 2);
                    skip_bits(&gb, 5); // line_offset
                    cc1 = get_bits(&gb, 8);
                    cc2 = get_bits(&gb, 8);
                    skip_bits(&gb, 1); // marker

                    if (!field) { // forbidden
                        cap[0] = cap[1] = cap[2] = 0x00;
                    } else {
                        field = (field == 2 ? 1 : 0);
                        if (!s1->mpeg_enc_ctx.top_field_first) field = !field;
                        cap[0] = 0x04 | field;
                        cap[1] = ff_reverse[cc1];
                        cap[2] = ff_reverse[cc2];
                    }
                    cap += 3;
                }
            }
            avctx->properties |= FF_CODEC_PROPERTY_CLOSED_CAPTIONS;
        }
        return 1;
    } else if (buf_size >= 11 &&
               p[0] == 'C' && p[1] == 'C' && p[2] == 0x01 && p[3] == 0xf8) {
        /* extract DVD CC data
         *
         * uint32_t   user_data_start_code        0x000001B2    (big endian)
         * uint16_t   user_identifier             0x4343 "CC"
         * uint8_t    user_data_type_code         0x01
         * uint8_t    caption_block_size          0xF8
         * uint8_t
         *   bit 7    caption_odd_field_first     1=odd field (CC1/CC2) first  0=even field (CC3/CC4) first
         *   bit 6    caption_filler              0
         *   bit 5:1  caption_block_count         number of caption blocks (pairs of caption words = frames). Most DVDs use 15 per start of GOP.
         *   bit 0    caption_extra_field_added   1=one additional caption word
         *
         * struct caption_field_block {
         *   uint8_t
         *     bit 7:1 caption_filler             0x7F (all 1s)
         *     bit 0   caption_field_odd          1=odd field (this is CC1/CC2)  0=even field (this is CC3/CC4)
         *   uint8_t   caption_first_byte
         *   uint8_t   caption_second_byte
         * } caption_block[(caption_block_count * 2) + caption_extra_field_added];
         *
         * Some DVDs encode caption data for both fields with caption_field_odd=1. The only way to decode the fields
         * correctly is to start on the field indicated by caption_odd_field_first and count between odd/even fields.
         * Don't assume that the first caption word is the odd field. There do exist MPEG files in the wild that start
         * on the even field. There also exist DVDs in the wild that encode an odd field count and the
         * caption_extra_field_added/caption_odd_field_first bits change per packet to allow that. */
        int cc_count = 0;
        int i, ret;
        // There is a caption count field in the data, but it is often
        // incorrect.  So count the number of captions present.
        for (i = 5; i + 6 <= buf_size && ((p[i] & 0xfe) == 0xfe); i += 6)
            cc_count++;
        // Transform the DVD format into A53 Part 4 format
        if (cc_count > 0) {
            int old_size = s1->a53_buf_ref ? s1->a53_buf_ref->size : 0;
            const uint64_t new_size = (old_size + cc_count
                                            * UINT64_C(6));
            if (new_size > INT_MAX)
                return AVERROR(EINVAL);

            ret = av_buffer_realloc(&s1->a53_buf_ref, new_size);
            if (ret >= 0) {
                uint8_t field1 = !!(p[4] & 0x80);
                uint8_t *cap = s1->a53_buf_ref->data;
                p += 5;
                for (i = 0; i < cc_count; i++) {
                    cap[0] = (p[0] == 0xff && field1) ? 0xfc : 0xfd;
                    cap[1] = p[1];
                    cap[2] = p[2];
                    cap[3] = (p[3] == 0xff && !field1) ? 0xfc : 0xfd;
                    cap[4] = p[4];
                    cap[5] = p[5];
                    cap += 6;
                    p += 6;
                }
            }
            avctx->properties |= FF_CODEC_PROPERTY_CLOSED_CAPTIONS;
        }
        return 1;
    }
    return 0;
}

                                  const uint8_t *p, int buf_size)
{

#if 0
    int i;
    for(i=0; !(!p[i-2] && !p[i-1] && p[i]==1) && i<buf_size; i++){
        av_log(avctx, AV_LOG_ERROR, "%c", p[i]);
    }
    av_log(avctx, AV_LOG_ERROR, "\n");
#endif

        int i;
                s->tmpgexs= 1;
            }
    }
    /* we parse the DTG active format information */
            /* skip event id */
            p += 2;
        }
                return;
        }
               p[4] == 0x03) { // S3D_video_format_length
        // the 0x7F mask ignores the reserved_bit value
        const uint8_t S3D_video_format_type = p[5] & 0x7F;

        if (S3D_video_format_type == 0x03 ||
            S3D_video_format_type == 0x04 ||
            S3D_video_format_type == 0x08 ||
            S3D_video_format_type == 0x23) {

            s1->has_stereo3d = 1;

            switch (S3D_video_format_type) {
            case 0x03:
                s1->stereo3d.type = AV_STEREO3D_SIDEBYSIDE;
                break;
            case 0x04:
                s1->stereo3d.type = AV_STEREO3D_TOPBOTTOM;
                break;
            case 0x08:
                s1->stereo3d.type = AV_STEREO3D_2D;
                break;
            case 0x23:
                s1->stereo3d.type = AV_STEREO3D_SIDEBYSIDE_QUINCUNX;
                break;
            }
        }
        return;
    }
}

static void mpeg_decode_gop(AVCodecContext *avctx,
                            const uint8_t *buf, int buf_size)
{
    Mpeg1Context *s1  = avctx->priv_data;
    MpegEncContext *s = &s1->mpeg_enc_ctx;
    int broken_link;
    int64_t tc;

    init_get_bits(&s->gb, buf, buf_size * 8);

    tc = s-> timecode_frame_start = get_bits(&s->gb, 25);

#if FF_API_PRIVATE_OPT
FF_DISABLE_DEPRECATION_WARNINGS
    avctx->timecode_frame_start = tc;
FF_ENABLE_DEPRECATION_WARNINGS
#endif

    s->closed_gop = get_bits1(&s->gb);
    /* broken_link indicates that after editing the
     * reference frames of the first B-Frames after GOP I-Frame
     * are missing (open gop) */
    broken_link = get_bits1(&s->gb);

    if (s->avctx->debug & FF_DEBUG_PICT_INFO) {
        char tcbuf[AV_TIMECODE_STR_SIZE];
        av_timecode_make_mpeg_tc_string(tcbuf, tc);
        av_log(s->avctx, AV_LOG_DEBUG,
               "GOP (%s) closed_gop=%d broken_link=%d\n",
               tcbuf, s->closed_gop, broken_link);
    }
}

                         int *got_output, const uint8_t *buf, int buf_size)
{

        /* find next start code */

                                   s->slice_count, sizeof(void *));
                }

                    // FIXME: merge with the stuff in mpeg_decode_slice
                }
            }

                return AVERROR_INVALIDDATA;

        }


            av_log(avctx, AV_LOG_DEBUG, "%3"PRIX32" at %"PTRDIFF_SPECIFIER" left %d\n",
                   start_code, buf_ptr - buf, input_size);

        /* prepare data for next start code */
            } else {
                av_log(avctx, AV_LOG_ERROR,
                       "ignoring SEQ_START_CODE after %X\n", last_code);
                if (avctx->err_recognition & AV_EF_EXPLODE)
                    return AVERROR_INVALIDDATA;
            }
            break;

               /* If it's a frame picture, there can't be more than one picture header.
                  Yet, it does happen and we need to handle it. */
               av_log(avctx, AV_LOG_WARNING, "ignoring extra picture following a frame-picture\n");
               break;
            }

                       s2->width, s2->height);
            }

                s2->intra_dc_precision= 3;
                s2->intra_matrix[0]= 1;
            }
                int i;

                avctx->execute(avctx, slice_decode_thread,
                               s2->thread_context, NULL,
                               s->slice_count, sizeof(void *));
                for (i = 0; i < s->slice_count; i++)
                    s2->er.error_count += s2->thread_context[i]->er.error_count;
                s->slice_count = 0;
            }
                    av_log(avctx, AV_LOG_ERROR,
                           "mpeg_decode_postinit() failure\n");
                    return ret;
                }

                /* We have a complete image: we try to decompress it. */
                    s2->pict_type = 0;
            } else {
                av_log(avctx, AV_LOG_ERROR,
                       "ignoring pic after %X\n", last_code);
                if (avctx->err_recognition & AV_EF_EXPLODE)
                    return AVERROR_INVALIDDATA;
            }
            break;

                } else {
                    av_log(avctx, AV_LOG_ERROR,
                           "ignoring seq ext after %X\n", last_code);
                    if (avctx->err_recognition & AV_EF_EXPLODE)
                        return AVERROR_INVALIDDATA;
                }
                break;
                } else {
                    av_log(avctx, AV_LOG_ERROR,
                           "ignoring pic cod ext after %X\n", last_code);
                    if (avctx->err_recognition & AV_EF_EXPLODE)
                        return AVERROR_INVALIDDATA;
                }
                break;
            }
            break;
            } else {
                av_log(avctx, AV_LOG_ERROR,
                       "ignoring GOP_START_CODE after %X\n", last_code);
                if (avctx->err_recognition & AV_EF_EXPLODE)
                    return AVERROR_INVALIDDATA;
            }
            break;
                    s2->progressive_frame = 1;
                    av_log(s2->avctx, AV_LOG_ERROR,
                           "interlaced frame in progressive sequence, ignoring\n");
                }

                    av_log(s2->avctx, AV_LOG_ERROR,
                           "picture_structure %d invalid, ignoring\n",
                           s2->picture_structure);
                    s2->picture_structure = PICT_FRAME;
                }

                    av_log(s2->avctx, AV_LOG_WARNING, "invalid frame_pred_frame_dct\n");

                } else {
                }
            }
                    mb_y += (*buf_ptr&0xE0)<<2;


                    av_log(s2->avctx, AV_LOG_ERROR, "slice too small\n");
                    return AVERROR_INVALIDDATA;
                }

                    av_log(s2->avctx, AV_LOG_ERROR,
                           "slice below image (%d >= %d)\n", mb_y, s2->mb_height);
                    return AVERROR_INVALIDDATA;
                }

                    /* Skip B-frames if we do not have reference frames and
                     * GOP is not closed. */
                                   "Skipping B slice due to open GOP\n");
                        }
                    }
                }
                    /* Skip P-frames if we do not have a reference frame or
                     * we have an invalid header. */
                               "Skipping P slice due to !sync\n");
                    }
                }
                    avctx->skip_frame >= AVDISCARD_ALL) {
                    skip_frame = 1;
                    break;
                }

                    break;

                        break;
                }

                    av_log(avctx, AV_LOG_ERROR, "Missing picture start code\n");
                    if (avctx->err_recognition & AV_EF_EXPLODE)
                        return AVERROR_INVALIDDATA;
                    break;
                }

                        return ret;
                }
                    av_log(avctx, AV_LOG_ERROR,
                           "current_picture not initialized\n");
                    return AVERROR_INVALIDDATA;
                }

                                    s2->slice_context_count;

                                return ret;
                        }
                    }
                } else {

                            return ret;
                                            s2->resync_mb_y, s2->mb_x, s2->mb_y,
                                            ER_AC_ERROR | ER_DC_ERROR | ER_MV_ERROR);
                    } else {
                                        ER_AC_END | ER_DC_END | ER_MV_END);
                    }
                }
            }
            break;
        }
    }
}

                             int *got_output, AVPacket *avpkt)
{

        /* special case for last picture */
                return ret;


        }
    }

                                           buf_size, NULL);

                             (const uint8_t **) &buf, &buf_size) < 0)
    }

                                          ))


                            avctx->extradata, avctx->extradata_size);
            av_log(avctx, AV_LOG_ERROR, "picture in extradata\n");
            av_frame_unref(picture);
            *got_output = 0;
        }
            s2->current_picture_ptr = NULL;
            return ret;
        }
    }


                                                             AV_FRAME_DATA_GOP_TIMECODE,
                                                             sizeof(int64_t));
                return AVERROR(ENOMEM);


        }
    }

    return ret;
}

static void flush(AVCodecContext *avctx)
{
    Mpeg1Context *s = avctx->priv_data;

    s->sync       = 0;

    ff_mpeg_flush(avctx);
}

{

}

AVCodec ff_mpeg1video_decoder = {
    .name                  = "mpeg1video",
    .long_name             = NULL_IF_CONFIG_SMALL("MPEG-1 video"),
    .type                  = AVMEDIA_TYPE_VIDEO,
    .id                    = AV_CODEC_ID_MPEG1VIDEO,
    .priv_data_size        = sizeof(Mpeg1Context),
    .init                  = mpeg_decode_init,
    .close                 = mpeg_decode_end,
    .decode                = mpeg_decode_frame,
    .capabilities          = AV_CODEC_CAP_DRAW_HORIZ_BAND | AV_CODEC_CAP_DR1 |
                             AV_CODEC_CAP_TRUNCATED | AV_CODEC_CAP_DELAY |
                             AV_CODEC_CAP_SLICE_THREADS,
    .caps_internal         = FF_CODEC_CAP_SKIP_FRAME_FILL_PARAM | FF_CODEC_CAP_INIT_CLEANUP,
    .flush                 = flush,
    .max_lowres            = 3,
    .update_thread_context = ONLY_IF_THREADS_ENABLED(mpeg_decode_update_thread_context),
    .hw_configs            = (const AVCodecHWConfigInternal*[]) {
#if CONFIG_MPEG1_NVDEC_HWACCEL
                               HWACCEL_NVDEC(mpeg1),
#endif
#if CONFIG_MPEG1_VDPAU_HWACCEL
                               HWACCEL_VDPAU(mpeg1),
#endif
#if CONFIG_MPEG1_VIDEOTOOLBOX_HWACCEL
                               HWACCEL_VIDEOTOOLBOX(mpeg1),
#endif
#if CONFIG_MPEG1_XVMC_HWACCEL
                               HWACCEL_XVMC(mpeg1),
#endif
                               NULL
                           },
};

AVCodec ff_mpeg2video_decoder = {
    .name           = "mpeg2video",
    .long_name      = NULL_IF_CONFIG_SMALL("MPEG-2 video"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_MPEG2VIDEO,
    .priv_data_size = sizeof(Mpeg1Context),
    .init           = mpeg_decode_init,
    .close          = mpeg_decode_end,
    .decode         = mpeg_decode_frame,
    .capabilities   = AV_CODEC_CAP_DRAW_HORIZ_BAND | AV_CODEC_CAP_DR1 |
                      AV_CODEC_CAP_TRUNCATED | AV_CODEC_CAP_DELAY |
                      AV_CODEC_CAP_SLICE_THREADS,
    .caps_internal  = FF_CODEC_CAP_SKIP_FRAME_FILL_PARAM | FF_CODEC_CAP_INIT_CLEANUP,
    .flush          = flush,
    .max_lowres     = 3,
    .profiles       = NULL_IF_CONFIG_SMALL(ff_mpeg2_video_profiles),
    .hw_configs     = (const AVCodecHWConfigInternal*[]) {
#if CONFIG_MPEG2_DXVA2_HWACCEL
                        HWACCEL_DXVA2(mpeg2),
#endif
#if CONFIG_MPEG2_D3D11VA_HWACCEL
                        HWACCEL_D3D11VA(mpeg2),
#endif
#if CONFIG_MPEG2_D3D11VA2_HWACCEL
                        HWACCEL_D3D11VA2(mpeg2),
#endif
#if CONFIG_MPEG2_NVDEC_HWACCEL
                        HWACCEL_NVDEC(mpeg2),
#endif
#if CONFIG_MPEG2_VAAPI_HWACCEL
                        HWACCEL_VAAPI(mpeg2),
#endif
#if CONFIG_MPEG2_VDPAU_HWACCEL
                        HWACCEL_VDPAU(mpeg2),
#endif
#if CONFIG_MPEG2_VIDEOTOOLBOX_HWACCEL
                        HWACCEL_VIDEOTOOLBOX(mpeg2),
#endif
#if CONFIG_MPEG2_XVMC_HWACCEL
                        HWACCEL_XVMC(mpeg2),
#endif
                        NULL
                    },
};

//legacy decoder
AVCodec ff_mpegvideo_decoder = {
    .name           = "mpegvideo",
    .long_name      = NULL_IF_CONFIG_SMALL("MPEG-1 video"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_MPEG2VIDEO,
    .priv_data_size = sizeof(Mpeg1Context),
    .init           = mpeg_decode_init,
    .close          = mpeg_decode_end,
    .decode         = mpeg_decode_frame,
    .capabilities   = AV_CODEC_CAP_DRAW_HORIZ_BAND | AV_CODEC_CAP_DR1 | AV_CODEC_CAP_TRUNCATED | AV_CODEC_CAP_DELAY | AV_CODEC_CAP_SLICE_THREADS,
    .caps_internal  = FF_CODEC_CAP_SKIP_FRAME_FILL_PARAM | FF_CODEC_CAP_INIT_CLEANUP,
    .flush          = flush,
    .max_lowres     = 3,
};
