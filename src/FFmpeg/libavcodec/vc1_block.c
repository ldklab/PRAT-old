/*
 * VC-1 and WMV3 decoder
 * Copyright (c) 2011 Mashiat Sarker Shakkhar
 * Copyright (c) 2006-2007 Konstantin Shishkov
 * Partly based on vc9.c (c) 2005 Anonymous, Alex Beregszaszi, Michael Niedermayer
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
 * VC-1 and WMV3 block decoding routines
 */

#include "avcodec.h"
#include "mpegutils.h"
#include "mpegvideo.h"
#include "msmpeg4data.h"
#include "unary.h"
#include "vc1.h"
#include "vc1_pred.h"
#include "vc1acdata.h"
#include "vc1data.h"

#define MB_INTRA_VLC_BITS 9
#define DC_VLC_BITS 9

// offset tables for interlaced picture MVDATA decoding
static const uint8_t offset_table[2][9] = {
    {  0,  1,  2,  4,  8, 16, 32,  64, 128 },
    {  0,  1,  3,  7, 15, 31, 63, 127, 255 },
};

// mapping table for internal block representation
static const int block_map[6] = {0, 2, 1, 3, 4, 5};

/***********************************************************************/
/**
 * @name VC-1 Bitplane decoding
 * @see 8.7, p56
 * @{
 */


{
    }

/** @} */ //Bitplane group

{

    /* The put pixels loop is one MB row and one MB column behind the decoding
     * loop because we can only put pixels when overlap filtering is done. For
     * interlaced frame pictures, however, the put pixels loop is only one
     * column behind the decoding loop as interlaced frame pictures only need
     * horizontal overlap filtering. */
                                                          i > 3 ? s->uvlinesize : s->linesize);
                    else
                                                   i > 3 ? s->uvlinesize : s->linesize);
                }
            }
        }
                                                          i > 3 ? s->uvlinesize : s->linesize);
                    else
                                                   i > 3 ? s->uvlinesize : s->linesize);
                }
            }
        }
    }
                    else
                    else
                }
            }
        }
                    else
                    else
                }
            }
        }
    }

#define inc_blk_idx(idx) do { \
        idx++; \
        if (idx >= v->n_allocated_blks) \
            idx = 0; \
    } while (0)

/***********************************************************************/
/**
 * @name VC-1 Block-level functions
 * @see 7.1.4, p91 and 8.1.1.7, p(1)04
 * @{
 */

/**
 * @def GET_MQUANT
 * @brief Get macroblock-level quantizer scale
 */
#define GET_MQUANT()                                           \
    if (v->dquantfrm) {                                        \
        int edges = 0;                                         \
        if (v->dqprofile == DQPROFILE_ALL_MBS) {               \
            if (v->dqbilevel) {                                \
                mquant = (get_bits1(gb)) ? -v->altpq : v->pq;  \
            } else {                                           \
                mqdiff = get_bits(gb, 3);                      \
                if (mqdiff != 7)                               \
                    mquant = -v->pq - mqdiff;                  \
                else                                           \
                    mquant = -get_bits(gb, 5);                 \
            }                                                  \
        }                                                      \
        if (v->dqprofile == DQPROFILE_SINGLE_EDGE)             \
            edges = 1 << v->dqsbedge;                          \
        else if (v->dqprofile == DQPROFILE_DOUBLE_EDGES)       \
            edges = (3 << v->dqsbedge) % 15;                   \
        else if (v->dqprofile == DQPROFILE_FOUR_EDGES)         \
            edges = 15;                                        \
        if ((edges&1) && !s->mb_x)                             \
            mquant = -v->altpq;                                \
        if ((edges&2) && !s->mb_y)                             \
            mquant = -v->altpq;                                \
        if ((edges&4) && s->mb_x == (s->mb_width - 1))         \
            mquant = -v->altpq;                                \
        if ((edges&8) &&                                       \
            s->mb_y == ((s->mb_height >> v->field_mode) - 1))  \
            mquant = -v->altpq;                                \
        if (!mquant || mquant > 31 || mquant < -31) {                          \
            av_log(v->s.avctx, AV_LOG_ERROR,                   \
                   "Overriding invalid mquant %d\n", mquant);  \
            mquant = 1;                                        \
        }                                                      \
    }

/**
 * @def GET_MVDATA(_dmv_x, _dmv_y)
 * @brief Get MV differentials
 * @see MVDATA decoding from 8.3.5.2, p(1)20
 * @param _dmv_x Horizontal differential for decoded MV
 * @param _dmv_y Vertical differential for decoded MV
 */
#define GET_MVDATA(_dmv_x, _dmv_y)                                      \
    index = 1 + get_vlc2(gb, ff_vc1_mv_diff_vlc[s->mv_table_index].table, \
                         VC1_MV_DIFF_VLC_BITS, 2);                      \
    if (index > 36) {                                                   \
        mb_has_coeffs = 1;                                              \
        index -= 37;                                                    \
    } else                                                              \
        mb_has_coeffs = 0;                                              \
    s->mb_intra = 0;                                                    \
    if (!index) {                                                       \
        _dmv_x = _dmv_y = 0;                                            \
    } else if (index == 35) {                                           \
        _dmv_x = get_bits(gb, v->k_x - 1 + s->quarter_sample);          \
        _dmv_y = get_bits(gb, v->k_y - 1 + s->quarter_sample);          \
    } else if (index == 36) {                                           \
        _dmv_x = 0;                                                     \
        _dmv_y = 0;                                                     \
        s->mb_intra = 1;                                                \
    } else {                                                            \
        index1 = index % 6;                                             \
        _dmv_x = offset_table[1][index1];                               \
        val = size_table[index1] - (!s->quarter_sample && index1 == 5); \
        if (val > 0) {                                                  \
            val = get_bits(gb, val);                                    \
            sign = 0 - (val & 1);                                       \
            _dmv_x = (sign ^ ((val >> 1) + _dmv_x)) - sign;             \
        }                                                               \
                                                                        \
        index1 = index / 6;                                             \
        _dmv_y = offset_table[1][index1];                               \
        val = size_table[index1] - (!s->quarter_sample && index1 == 5); \
        if (val > 0) {                                                  \
            val = get_bits(gb, val);                                    \
            sign = 0 - (val & 1);                                       \
            _dmv_y = (sign ^ ((val >> 1) + _dmv_y)) - sign;             \
        }                                                               \
    }

                                                   int *dmv_y, int *pred_flag)
{

        bits = VC1_2REF_MVDATA_VLC_BITS;
        esc  = 125;
    } else {
    }
        }
    }
    else {
        } else
        } else
    }
}

/** Reconstruct motion vector for B-frame and do motion compensation
 */
static inline void vc1_b_mc(VC1Context *v, int dmv_x[2], int dmv_y[2],
                            int direct, int mode)
{
    if (direct) {
        ff_vc1_mc_1mv(v, 0);
        ff_vc1_interp_mc(v);
        return;
    }
    if (mode == BMV_TYPE_INTERPOLATED) {
        ff_vc1_mc_1mv(v, 0);
        ff_vc1_interp_mc(v);
        return;
    }

    ff_vc1_mc_1mv(v, (mode == BMV_TYPE_BACKWARD));
}

/** Get predicted DC value for I-frames only
 * prediction dir: left=0, top=1
 * @param s MpegEncContext
 * @param overlap flag indicating that overlap filtering is used
 * @param pq integer part of picture quantizer
 * @param[in] n block index in the current MB
 * @param dc_val_ptr Pointer to DC predictor
 * @param dir_ptr Prediction direction for use in AC prediction
 */
                                int16_t **dc_val_ptr, int *dir_ptr)
{
        -1, 1024,  512,  341,  256,  205,  171,  146,  128,
             114,  102,   93,   85,   79,   73,   68,   64,
              60,   57,   54,   51,   49,   47,   45,   43,
              41,   39,   38,   37,   35,   34,   33
    };

    /* find prediction - wmv3_dc_scale always used here in fact */


    /* B A
     * C X
     */

        /* Set outer values */
    } else {
        /* Set outer values */
        if (s->first_slice_line && (n != 2 && n != 3))
            b = a = 0;
        if (s->mb_x == 0 && (n != 1 && n != 3))
            b = c = 0;
    }

    } else {
    }

    /* update predictor */
}


/** Get predicted DC value
 * prediction dir: left=0, top=1
 * @param s MpegEncContext
 * @param overlap flag indicating that overlap filtering is used
 * @param pq integer part of picture quantizer
 * @param[in] n block index in the current MB
 * @param a_avail flag indicating top block availability
 * @param c_avail flag indicating left block availability
 * @param dc_val_ptr Pointer to DC predictor
 * @param dir_ptr Prediction direction for use in AC prediction
 */
static inline int ff_vc1_pred_dc(MpegEncContext *s, int overlap, int pq, int n,
                              int a_avail, int c_avail,
                              int16_t **dc_val_ptr, int *dir_ptr)
{
    int a, b, c, wrap, pred;
    int16_t *dc_val;
    int mb_pos = s->mb_x + s->mb_y * s->mb_stride;
    int q1, q2 = 0;
    int dqscale_index;

    /* scale predictors if needed */
    q1 = FFABS(s->current_picture.qscale_table[mb_pos]);
    dqscale_index = s->y_dc_scale_table[q1] - 1;
    if (dqscale_index < 0)
        return 0;

    wrap = s->block_wrap[n];
    dc_val = s->dc_val[0] + s->block_index[n];

    /* B A
     * C X
     */
    c = dc_val[ - 1];
    b = dc_val[ - 1 - wrap];
    a = dc_val[ - wrap];

    if (c_avail && (n != 1 && n != 3)) {
        q2 = FFABS(s->current_picture.qscale_table[mb_pos - 1]);
        if (q2 && q2 != q1)
            c = (int)((unsigned)c * s->y_dc_scale_table[q2] * ff_vc1_dqscale[dqscale_index] + 0x20000) >> 18;
    }
    if (a_avail && (n != 2 && n != 3)) {
        q2 = FFABS(s->current_picture.qscale_table[mb_pos - s->mb_stride]);
        if (q2 && q2 != q1)
            a = (int)((unsigned)a * s->y_dc_scale_table[q2] * ff_vc1_dqscale[dqscale_index] + 0x20000) >> 18;
    }
    if (a_avail && c_avail && (n != 3)) {
        int off = mb_pos;
        if (n != 1)
            off--;
        if (n != 2)
            off -= s->mb_stride;
        q2 = FFABS(s->current_picture.qscale_table[off]);
        if (q2 && q2 != q1)
            b = (int)((unsigned)b * s->y_dc_scale_table[q2] * ff_vc1_dqscale[dqscale_index] + 0x20000) >> 18;
    }

    if (c_avail && (!a_avail || abs(a - b) <= abs(b - c))) {
        pred     = c;
        *dir_ptr = 1; // left
    } else if (a_avail) {
        pred     = a;
        *dir_ptr = 0; // top
    } else {
        pred     = 0;
        *dir_ptr = 1; // left
    }

    /* update predictor */
    *dc_val_ptr = &dc_val[0];
    return pred;
}

/** @} */ // Block group

/**
 * @name VC1 Macroblock-level functions in Simple/Main Profiles
 * @see 7.1.4, p91 and 8.1.1.7, p(1)04
 * @{
 */

                                       uint8_t **coded_block_ptr)
{


    /* B C
     * A X
     */

        pred = a;
    } else {
    }

    /* store value */

}

/**
 * Decode one AC coefficient
 * @param v The VC1 context
 * @param last Last coefficient
 * @param skip How much zero coefficients to skip
 * @param value Decoded AC coefficient value
 * @param codingset set of VLC to decode data
 * @see 8.1.3.4
 */
                                int *value, int codingset)
{

        return index;
    } else {
                return AVERROR_INVALIDDATA;
                else
            } else {
                else
            }
        } else {
                } else { // table 60
                }
            }
        }
    }


}

/** Decode intra block in intra frames - should be faster than decode_intra_block
 * @param v VC1Context
 * @param block block to decode
 * @param[in] n subblock index
 * @param coded are AC coeffs present or not
 * @param codingset set of VLC to decode data
 */
                              int coded, int codingset)
{

    /* Get DC differential */
    } else {
    }
        av_log(s->avctx, AV_LOG_ERROR, "Illegal DC VLC\n");
        return -1;
    }
        } else {
        }
    }

    /* Prediction */

    /* Store the quantized DC coeff, used for prediction */
    else

    else // top


    //AC Decoding


            else
        } else

                return ret;
                break;
        }

        /* apply AC prediction if needed */
            } else { // top
            }
        }
        /* save AC coeffs for further prediction */
        }

        /* scale AC coeffs */
            }

    } else {


        /* apply AC prediction if needed */
            } else { // top
            }
            }
        }
    }

}

/** Decode intra block in intra frames - should be faster than decode_intra_block
 * @param v VC1Context
 * @param block block to decode
 * @param[in] n subblock number
 * @param coded are AC coeffs present or not
 * @param codingset set of VLC to decode data
 * @param mquant quantizer value for this macroblock
 */
                                  int coded, int codingset, int mquant)
{

    /* Get DC differential */
    } else {
    }
        av_log(s->avctx, AV_LOG_ERROR, "Illegal DC VLC\n");
        return -1;
    }
        } else {
        }
    }

    /* Prediction */

    /* Store the quantized DC coeff, used for prediction */
    else

    /* check if AC is needed at all */


    else // top

        q2 = q1;
            q2 = q1;
    } else {
            q2 = q1;
    }

    //AC Decoding


            } else {
                else // left
            }
        } else {
            else
        }

                return ret;
                break;
        }

        /* apply AC prediction if needed */
            } else { // top
            }
            /* scale predictors if needed*/
                return AVERROR_INVALIDDATA;
            } else {
            }
        }
        /* save AC coeffs for further prediction */
        }

        /* scale AC coeffs */
                    block[k] += (block[k] < 0) ? -quant : quant;
            }

    } else { // no AC coeffs


        /* apply AC prediction if needed */
            } else { // top
            }
                return AVERROR_INVALIDDATA;
            }
                    block[k << sh] += (block[k << sh] < 0) ? -quant : quant;
            }
        }
    }

}

/** Decode intra block in inter frames - more generic version than vc1_decode_i_block
 * @param v VC1Context
 * @param block block to decode
 * @param[in] n subblock index
 * @param coded are AC coeffs present or not
 * @param mquant block quantizer
 * @param codingset set of VLC to decode data
 */
                                  int coded, int mquant, int codingset)
{


    /* XXX: Guard against dumb values of mquant */

    /* Set DC scale - y and c use the same */

    /* Get DC differential */
    } else {
    }
        av_log(s->avctx, AV_LOG_ERROR, "Illegal DC VLC\n");
        return -1;
    }
        } else {
        }
    }

    /* Prediction */

    /* Store the quantized DC coeff, used for prediction */

    } else {
    }

    //AC Decoding

    /* check if AC is needed at all and adjust direction if needed */


    else //top



                return ret;
                break;
            else {
                    else // left
                } else {
                }
            }
        }

        /* apply AC prediction if needed */
            /* scale predictors if needed*/
                return AVERROR_INVALIDDATA;
                } else { //top
                }
            } else {
                } else { // top
                }
            }
        }
        /* save AC coeffs for further prediction */
        }

        /* scale AC coeffs */
            }

    } else { // no AC coeffs

                    return AVERROR_INVALIDDATA;
                }
            }
        } else { // top
                    return AVERROR_INVALIDDATA;
                }
            }
        }

        /* apply AC prediction if needed */
                }
            } else { // top
                }
            }
            i = 63;
        }
    }

}

/** Decode P block
 */
                              int mquant, int ttmb, int first_block,
                              uint8_t *dst, int linesize, int skip_block,
                              int *ttmb_out)
{


    }
    }
    }

    // convert transforms like 8X4_TOP to generic TT and SUBBLKPAT
        ttblk     = TT_8X4;
    }
        ttblk     = TT_4X8;
    }
                return ret;
                break;
            else
        }
            else {
            }
        }
        break;
                    return ret;
                    break;
                else
            }
                else
            }
        }
        break;
                    return ret;
                    break;
                else
            }
                else
            }
        }
        break;
                    return ret;
                    break;
                else
            }
                else
            }
        }
        break;
    }
    return pat;
}

/** @} */ // Macroblock group

static const uint8_t size_table[6] = { 0, 2, 3, 4,  5,  8 };

/** Decode one P-frame MB
 */
{



    else
    else


            }

            /* FIXME Set DC val for inter block ? */
            } else {
            }

                                VC1_TTMB_VLC_BITS, 2);
            dst_idx = 0;
                    /* check if prediction blocks A and C are available */

                                           (i & 4) ? v->codingset2 : v->codingset);
                        continue;
                        for (j = 0; j < 64; j++)
                            v->block[v->cur_blk_idx][block_map[i]][j] *= 2;
                                             CONFIG_GRAY && (i & 4) && (s->avctx->flags & AV_CODEC_FLAG_GRAY), &block_tt);
                        return pat;
                    first_block = 0;
                }
            }
        } else { // skipped
            }
        }
    } else { // 4MV mode
            /* Get CBPCY */
                    }
                }
                }
            }
            // if there are no coded blocks then don't do anything more
            /* test if block is intra and has pred */
            {
                            intrapred = 1;
                            break;
                        }
                    }
                else
            }
                    /* check if prediction blocks A and C are available */

                                           (i & 4) ? v->codingset2 : v->codingset);
                        continue;
                        for (j = 0; j < 64; j++)
                            v->block[v->cur_blk_idx][block_map[i]][j] *= 2;
                                             CONFIG_GRAY && (i & 4) && (s->avctx->flags & AV_CODEC_FLAG_GRAY),
                                             &block_tt);
                        return pat;
                    first_block = 0;
                }
            }
        } else { // skipped MB
            }
            }
        }
    }
        ff_vc1_p_overlap_filter(v);


}

/* Decode one macroblock in an interlaced frame p picture */

{



        skipped = get_bits1(gb);
    else
        else
            idx_mbmode = get_vlc2(gb, v->mbmode_vlc->table, VC1_INTFR_NON4MV_MBMODE_VLC_BITS, 2); // in a single line
        /* store the motion vector type in a flag (useful later) */
        }
            }
            /* Set DC scale - y and c use the same (not sure if necessary here) */

                    continue;
                    off = (fieldtx) ? ((i & 1) * 8) + ((i & 2) >> 1) * s->linesize : (i & 1) * 8 + 4 * (i & 2) * s->linesize;
                else
            }

        } else { // inter MB
            } else {
                }
            }
            /* for all motion vector read MVDATA and motion compensate each block */
                }
                }
                }
            } else {
                }
            }
                else
                                             CONFIG_GRAY && (i & 4) && (s->avctx->flags & AV_CODEC_FLAG_GRAY), &block_tt);
                        return pat;
                    first_block = 0;
                }
            }
        }
    } else { // skipped
        }
    }
        ff_vc1_p_overlap_filter(v);


}

{



        /* Set DC scale - y and c use the same (not sure if necessary here) */

                continue;
        }
    } else {
            }
        } else { // 4-MV
            }
        }
        }
        }
        dst_idx = 0;
                                         CONFIG_GRAY && (i & 4) && (s->avctx->flags & AV_CODEC_FLAG_GRAY),
                                         &block_tt);
                    return pat;
                first_block = 0;
            }
        }
    }
        ff_vc1_p_overlap_filter(v);


}

/** Decode one B-frame MB (in Main profile)
 */
{


        direct = get_bits1(gb);
    else
        skipped = get_bits1(gb);
    else

    }

        }
            }
    }

    }
    } else {
            /* no coded blocks - effectively skipped */
        }
            GET_MQUANT();
            s->current_picture.qscale_table[mb_pos] = mquant;
            s->ac_pred = get_bits1(gb);
            cbp = 0;
            ff_vc1_pred_b_mv(v, dmv_x, dmv_y, direct, bmvtype);
        } else {
                    /* interpolated skipped block */
                }
            }
            }
        }
    }
            /* check if prediction blocks A and C are available */

                                   (i & 4) ? v->codingset2 : v->codingset);
                continue;
                for (j = 0; j < 64; j++)
                    s->block[i][j] *= 2;
                                              i & 4 ? s->uvlinesize
                                                    : s->linesize);
                                         CONFIG_GRAY && (i & 4) && (s->avctx->flags & AV_CODEC_FLAG_GRAY), NULL);
                return pat;
            first_block = 0;
        }
    }
    return 0;
}

/** Decode one B-frame MB (in interlaced field B picture)
 */
{


        /* Set DC scale - y and c use the same (not sure if necessary here) */

                continue;
                for (j = 0; j < 64; j++)
                    s->block[i][j] <<= 1;
                                              (i & 4) ? s->uvlinesize
                                                      : s->linesize);
        }
    } else {
        else
                bmvtype = BMV_TYPE_FORWARD;
            else {
                }
            }
            }
                    av_log(s->avctx, AV_LOG_ERROR, "Mixed field/frame direct mode not supported\n");
                    return AVERROR_INVALIDDATA;
                }
            }
        } else { // 4-MV
                                             &dmv_y[bmvtype == BMV_TYPE_BACKWARD],
                }
            }
        }
        }
        }
        dst_idx = 0;
                                         CONFIG_GRAY && (i & 4) && (s->avctx->flags & AV_CODEC_FLAG_GRAY), &block_tt);
                    return pat;
                first_block = 0;
            }
        }
    }

}

/** Decode one B-frame MB (in interlaced frame B picture)
 */
{

        skipped = get_bits1(gb);
    else

        } else {
        }
    }

        }
        /* Set DC scale - y and c use the same (not sure if necessary here) */

                continue;
            } else {
            }
                                              stride_y);
        }
    } else {

            direct = get_bits1(gb);
        else

                av_log(s->avctx, AV_LOG_WARNING, "Mixed frame/field direct mode not supported\n");


                }
            } else {
                }
            }
        }

                }
            }

        }

                }
            }

            /* for all motion vector read MVDATA and motion compensate each block */
                    }
                } else {
                }
                }






                    }
                } else {
                }

            } else {


                }
            }

                else
                                             CONFIG_GRAY && (i & 4) && (s->avctx->flags & AV_CODEC_FLAG_GRAY), &block_tt);
                        return pat;
                    first_block = 0;
                }
            }

        } else { // skipped
            }

                } else {
                        int dir2 = dir;
                        if (mvsw)
                            dir2 = !dir;
                        for (i = 0; i < 2; i++) {
                            s->mv[dir][i+2][0] = s->mv[dir][i][0] = s->current_picture.motion_val[dir][s->block_index[i+2]][0] = s->current_picture.motion_val[dir][s->block_index[i]][0];
                            s->mv[dir][i+2][1] = s->mv[dir][i][1] = s->current_picture.motion_val[dir][s->block_index[i+2]][1] = s->current_picture.motion_val[dir][s->block_index[i]][1];
                            s->mv[dir2][i+2][0] = s->mv[dir2][i][0] = s->current_picture.motion_val[dir2][s->block_index[i]][0] = s->current_picture.motion_val[dir2][s->block_index[i+2]][0];
                            s->mv[dir2][i+2][1] = s->mv[dir2][i][1] = s->current_picture.motion_val[dir2][s->block_index[i]][1] = s->current_picture.motion_val[dir2][s->block_index[i+2]][1];
                        }
                    } else {
                        }
                    }
                }
            }

            }
        }
    }

}

/** Decode blocks of I-frame
 */
{

    /* select coding mode used for VLC tables selection */
    }

    }

    /* Set DC scale - y and c use the same */

    //do frame decode
            }

            // do actual MB decoding and displaying



                }


                    continue;
            }

                ff_vc1_i_overlap_filter(v);
                if (v->rangeredfrm)
                    for (k = 0; k < 6; k++)
                        for (j = 0; j < 64; j++)
                            v->block[v->cur_blk_idx][block_map[k]][j] *= 2;
                vc1_put_blocks_clamped(v, 1);
            } else {
            }


                ff_er_add_slice(&s->er, 0, 0, s->mb_x, s->mb_y, ER_MB_ERROR);
                av_log(s->avctx, AV_LOG_ERROR, "Bits overconsumption: %i > %i\n",
                       get_bits_count(&s->gb), s->gb.size_in_bits);
                return;
            }

        }

    }

    /* This is intentionally mb_height and not end_mb_y - unlike in advanced
     * profile, these only differ are when decoding MSS2 rectangles. */
}

/** Decode blocks of I-frame for advanced profile
 */
{

        return AVERROR_INVALIDDATA;

    /* select coding mode used for VLC tables selection */
    }

    case 1:
        v->codingset2 = CS_HIGH_MOT_INTER;
        break;
    }

    // do frame decode
    }
            }

            // do actual MB decoding and displaying
                v->fieldtx_plane[mb_pos] = get_bits1(&v->s.gb);
                ff_er_add_slice(&s->er, 0, s->start_mb_y, s->mb_x, s->mb_y, ER_MB_ERROR);
                return 0;
            }

            else

                v->over_flags_plane[mb_pos] = get_bits1(&v->s.gb);


            /* Set DC scale - y and c use the same */



                }


                                       (k < 4) ? v->codingset : v->codingset2, mquant);

                    continue;
            }


                // TODO: may need modification to handle slice coding
                ff_er_add_slice(&s->er, 0, s->start_mb_y, s->mb_x, s->mb_y, ER_MB_ERROR);
                av_log(s->avctx, AV_LOG_ERROR, "Bits overconsumption: %i > %i\n",
                       get_bits_count(&s->gb), s->gb.size_in_bits);
                return 0;
            }
        }
    }

}

{

    /* select coding mode used for VLC tables selection */
    }

    }


                    ff_er_add_slice(&s->er, 0, s->start_mb_y, s->mb_x, s->mb_y, ER_MB_ERROR);
                    return;
                }

            } else {
            }
                // TODO: may need modification to handle slice coding
                ff_er_add_slice(&s->er, 0, s->start_mb_y, s->mb_x, s->mb_y, ER_MB_ERROR);
                av_log(s->avctx, AV_LOG_ERROR, "Bits overconsumption: %i > %i at %ix%i\n",
                       get_bits_count(&s->gb), s->gb.size_in_bits, s->mb_x, s->mb_y);
                return;
            }
        }
    }
}

{

    /* select coding mode used for VLC tables selection */
    }

    }


                    ff_er_add_slice(&s->er, 0, s->start_mb_y, s->mb_x, s->mb_y, ER_MB_ERROR);
                    return;
                }

            } else {
            }
                // TODO: may need modification to handle slice coding
                ff_er_add_slice(&s->er, 0, s->start_mb_y, s->mb_x, s->mb_y, ER_MB_ERROR);
                av_log(s->avctx, AV_LOG_ERROR, "Bits overconsumption: %i > %i at %ix%i\n",
                       get_bits_count(&s->gb), s->gb.size_in_bits, s->mb_x, s->mb_y);
                return;
            }
        }
    }
}

{

        return;

    }
}

{

        ff_intrax8_decode_picture(&v->x8, &v->s.current_picture,
                                  &v->s.gb, &v->s.mb_x, &v->s.mb_y,
                                  2 * v->pq + v->halfpq, v->pq * !v->pquantizer,
                                  v->s.loop_filter, v->s.low_delay);

        ff_er_add_slice(&v->s.er, 0, 0,
                        (v->s.mb_x >> 1) - 1, (v->s.mb_y >> 1) - 1,
                        ER_MB_END);
    } else {
            else
            break;
            else
            break;
                if (v->profile == PROFILE_ADVANCED)
                    vc1_decode_i_blocks_adv(v);
                else
                    vc1_decode_i_blocks(v);
            } else
            break;
        }
