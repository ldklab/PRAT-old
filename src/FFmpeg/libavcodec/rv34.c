/*
 * RV30/40 decoder common data
 * Copyright (c) 2007 Mike Melanson, Konstantin Shishkov
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
 * RV30/40 decoder common data
 */

#include "libavutil/imgutils.h"
#include "libavutil/internal.h"

#include "avcodec.h"
#include "error_resilience.h"
#include "mpegutils.h"
#include "mpegvideo.h"
#include "golomb.h"
#include "internal.h"
#include "mathops.h"
#include "mpeg_er.h"
#include "qpeldsp.h"
#include "rectangle.h"
#include "thread.h"

#include "rv34vlc.h"
#include "rv34data.h"
#include "rv34.h"

{

/** translation of RV30/40 macroblock types to lavc ones */
static const int rv34_mb_type_to_lavc[12] = {
    MB_TYPE_INTRA,
    MB_TYPE_INTRA16x16              | MB_TYPE_SEPARATE_DC,
    MB_TYPE_16x16   | MB_TYPE_L0,
    MB_TYPE_8x8     | MB_TYPE_L0,
    MB_TYPE_16x16   | MB_TYPE_L0,
    MB_TYPE_16x16   | MB_TYPE_L1,
    MB_TYPE_SKIP,
    MB_TYPE_DIRECT2 | MB_TYPE_16x16,
    MB_TYPE_16x8    | MB_TYPE_L0,
    MB_TYPE_8x16    | MB_TYPE_L0,
    MB_TYPE_16x16   | MB_TYPE_L0L1,
    MB_TYPE_16x16   | MB_TYPE_L0    | MB_TYPE_SEPARATE_DC
};


static RV34VLC intra_vlcs[NUM_INTRA_TABLES], inter_vlcs[NUM_INTER_TABLES];

static int rv34_decode_mv(RV34DecContext *r, int block_type);

/**
 * @name RV30/40 VLC generating functions
 * @{
 */

static const int table_offs[] = {
      0,   1818,   3622,   4144,   4698,   5234,   5804,   5868,   5900,   5932,
   5996,   6252,   6316,   6348,   6380,   7674,   8944,  10274,  11668,  12250,
  14060,  15846,  16372,  16962,  17512,  18148,  18180,  18212,  18244,  18308,
  18564,  18628,  18660,  18692,  20036,  21314,  22648,  23968,  24614,  26384,
  28190,  28736,  29366,  29938,  30608,  30640,  30672,  30704,  30768,  31024,
  31088,  31120,  31184,  32570,  33898,  35236,  36644,  37286,  39020,  40802,
  41368,  42052,  42692,  43348,  43380,  43412,  43444,  43476,  43604,  43668,
  43700,  43732,  45100,  46430,  47778,  49160,  49802,  51550,  53340,  53972,
  54648,  55348,  55994,  56122,  56154,  56186,  56218,  56346,  56410,  56442,
  56474,  57878,  59290,  60636,  62036,  62682,  64460,  64524,  64588,  64716,
  64844,  66076,  67466,  67978,  68542,  69064,  69648,  70296,  72010,  72074,
  72138,  72202,  72330,  73572,  74936,  75454,  76030,  76566,  77176,  77822,
  79582,  79646,  79678,  79742,  79870,  81180,  82536,  83064,  83672,  84242,
  84934,  85576,  87384,  87448,  87480,  87544,  87672,  88982,  90340,  90902,
  91598,  92182,  92846,  93488,  95246,  95278,  95310,  95374,  95502,  96878,
  98266,  98848,  99542, 100234, 100884, 101524, 103320, 103352, 103384, 103416,
 103480, 104874, 106222, 106910, 107584, 108258, 108902, 109544, 111366, 111398,
 111430, 111462, 111494, 112878, 114320, 114988, 115660, 116310, 116950, 117592
};

static VLC_TYPE table_data[117592][2];

/**
 * Generate VLC from codeword lengths.
 * @param bits   codeword lengths (zeroes are accepted)
 * @param size   length of input data
 * @param vlc    output VLC
 * @param insyms symbols for input codes (NULL for default ones)
 * @param num    VLC table number (for static initialization)
 */
                         const int num)
{

        }
    }


                       bits2, 1, 1,
                       cw,    2, 2,
                       syms,  2, 2, INIT_VLC_USE_NEW_STATIC);

/**
 * Initialize all tables.
 */
{

            }
        }
        }
    }

        }
        }
    }

/** @} */ // vlc group

/**
 * @name RV30/40 4x4 block decoding functions
 * @{
 */

/**
 * Decode coded block pattern.
 */
{



    }

    }
}

/**
 * Get one coefficient value from the bitstream and store it.
 */
static inline void decode_coeff(int16_t *dst, int coef, int esc, GetBitContext *gb, VLC* vlc, int q)
{
    if(coef){
        if(coef == esc){
            coef = get_vlc2(gb, vlc->table, 9, 2);
            if(coef > 23){
                coef -= 23;
                coef = 22 + ((1 << coef) | get_bits(gb, coef));
            }
            coef += esc;
        }
        if(get_bits1(gb))
            coef = -coef;
        *dst = (coef*q + 8) >> 4;
    }
}

/**
 * Decode 2x2 subblock of coefficients.
 */
{

    }else{
    }

/**
 * Decode a single coefficient.
 */
{
}

                                    int q_dc, int q_ac1, int q_ac2)
{


/**
 * Decode coefficients for 4x4 block.
 *
 * This is done by filling 2x2 subblocks with decoded coefficients
 * in this order (the same for subblocks and subblock coefficients):
 *  o--o
 *    /
 *   /
 *  o--o
 */

{




    } else {
            return 0;
        has_ac = 0;
    }

    }
    }
    }
}

/**
 * @name RV30/40 bitstream parsing
 * @{
 */

/**
 * Decode starting slice position.
 * @todo Maybe replace with ff_h263_decode_mba() ?
 */
{
            break;
}

/**
 * Select VLC set for decoding from current quantizer, modifier and frame type.
 */
{
}

/**
 * Decode intra macroblock header and return CBP in case of success, -1 otherwise.
 */
{

    }else{
                av_log(s->avctx, AV_LOG_ERROR, "Need DQUANT\n");
        }
            return -1;
    }


}

/**
 * Decode inter macroblock header and return CBP in case of success, -1 otherwise.
 */
{

        return -1;
    }
        return -1;
    }

        }else{
                return -1;
        }
    }else{
        }
    }

}

/** @} */ //bitstream functions

/**
 * @name motion vector related code (prediction, reconstruction, motion compensation)
 * @{
 */

/** macroblock partition width in 8x8 blocks */
static const uint8_t part_sizes_w[RV34_MB_TYPES] = { 2, 2, 2, 1, 2, 2, 2, 2, 2, 1, 2, 2 };

/** macroblock partition height in 8x8 blocks */
static const uint8_t part_sizes_h[RV34_MB_TYPES] = { 2, 2, 2, 1, 2, 2, 2, 2, 1, 2, 2, 2 };

/** availability index for subblocks */
static const uint8_t avail_indexes[4] = { 6, 7, 10, 11 };

/**
 * motion vector prediction
 *
 * Motion prediction performed for the block by using median prediction of
 * motion vectors from the left, top and right top blocks but in corner cases
 * some other vectors may be used instead.
 */
{


    }
    }else{
        B[0] = A[0];
        B[1] = A[1];
    }
        }else{
            C[0] = A[0];
            C[1] = A[1];
        }
    }else{
    }
        }
    }

#define GET_PTS_DIFF(a, b) (((a) - (b) + 8192) & 0x1FFF)

/**
 * Calculate motion vector component that should be added for direct blocks.
 */
{

}

/**
 * Predict motion vector for B-frame macroblock.
 */
                                      int A_avail, int B_avail, int C_avail,
                                      int *mx, int *my)
{
        }
    }else{
    }

/**
 * motion vector prediction for B-frames
 */
{

    }
    }
    }



        }
    }
    }

/**
 * motion vector prediction - RV3 version
 */
static void rv34_pred_mv_rv3(RV34DecContext *r, int block_type, int dir)
{
    MpegEncContext *s = &r->s;
    int mv_pos = s->mb_x * 2 + s->mb_y * 2 * s->b8_stride;
    int A[2] = {0}, B[2], C[2];
    int i, j, k;
    int mx, my;
    int* avail = r->avail_cache + avail_indexes[0];

    if(avail[-1]){
        A[0] = s->current_picture_ptr->motion_val[0][mv_pos - 1][0];
        A[1] = s->current_picture_ptr->motion_val[0][mv_pos - 1][1];
    }
    if(avail[-4]){
        B[0] = s->current_picture_ptr->motion_val[0][mv_pos - s->b8_stride][0];
        B[1] = s->current_picture_ptr->motion_val[0][mv_pos - s->b8_stride][1];
    }else{
        B[0] = A[0];
        B[1] = A[1];
    }
    if(!avail[-4 + 2]){
        if(avail[-4] && (avail[-1])){
            C[0] = s->current_picture_ptr->motion_val[0][mv_pos - s->b8_stride - 1][0];
            C[1] = s->current_picture_ptr->motion_val[0][mv_pos - s->b8_stride - 1][1];
        }else{
            C[0] = A[0];
            C[1] = A[1];
        }
    }else{
        C[0] = s->current_picture_ptr->motion_val[0][mv_pos - s->b8_stride + 2][0];
        C[1] = s->current_picture_ptr->motion_val[0][mv_pos - s->b8_stride + 2][1];
    }
    mx = mid_pred(A[0], B[0], C[0]);
    my = mid_pred(A[1], B[1], C[1]);
    mx += r->dmv[0][0];
    my += r->dmv[0][1];
    for(j = 0; j < 2; j++){
        for(i = 0; i < 2; i++){
            for(k = 0; k < 2; k++){
                s->current_picture_ptr->motion_val[k][mv_pos + i + j*s->b8_stride][0] = mx;
                s->current_picture_ptr->motion_val[k][mv_pos + i + j*s->b8_stride][1] = my;
            }
        }
    }
}

static const int chroma_coeffs[3] = { 0, 3, 5 };

/**
 * generic motion compensation function
 *
 * @param r decoder context
 * @param block_type type of the current block
 * @param xoff horizontal offset from the start of the current block
 * @param yoff vertical offset from the start of the current block
 * @param mv_off offset to the motion vector information
 * @param width width of the current partition in 8x8 blocks
 * @param height height of the current partition in 8x8 blocks
 * @param dir motion compensation direction (i.e. from the last or the next reference frame)
 * @param thirdpel motion vectors are specified in 1/3 of pixel
 * @param qpel_mc a set of functions used to perform luma motion compensation
 * @param chroma_mc a set of functions used to perform chroma motion compensation
 */
                          const int xoff, const int yoff, int mv_off,
                          const int width, const int height, int dir,
                          const int thirdpel, int weighted,
                          qpel_mc_func (*qpel_mc)[16],
                          h264_chroma_mc_func (*chroma_mc))
{

    }else{
        //due to some flaw RV40 uses the same MC compensation routine for H2V2 and H3V3
    }

        /* wait for the referenced mb row to be finished */
        int mb_row = s->mb_y + ((yoff + my + 5 + 8 * height) >> 4);
        ThreadFrame *f = dir ? &s->next_picture_ptr->tf : &s->last_picture_ptr->tf;
        ff_thread_await_progress(f, mb_row, 0);
    }

                                 s->linesize, s->linesize,
                                 src_x - 2, src_y - 2,
                                 s->h_edge_pos, s->v_edge_pos);
    }
    }else{
    }

    }

                                 s->uvlinesize, s->uvlinesize,
                                 uvsrc_x, uvsrc_y,

                                 s->uvlinesize, s->uvlinesize,
                                 (width << 2) + 1, (height << 2) + 1,
                                 uvsrc_x, uvsrc_y,
    }

                        const int xoff, const int yoff, int mv_off,
                        const int width, const int height, int dir)
{

{
                                                        r->tmp_b_block_y[0],
                                                        r->tmp_b_block_y[1],
                                                        r->weight1,
                                                        r->weight2,
                                                        r->s.linesize);
                                                        r->tmp_b_block_uv[0],
                                                        r->tmp_b_block_uv[2],
                                                        r->weight1,
                                                        r->weight2,
                                                        r->s.uvlinesize);
                                                        r->tmp_b_block_uv[1],
                                                        r->tmp_b_block_uv[3],
                                                        r->weight1,
                                                        r->weight2,
                                                        r->s.uvlinesize);

{

    }else{
                r->rdsp.put_pixels_tab,
                r->rdsp.put_chroma_pixels_tab);
    }

{

                     weighted,
                     weighted,
                     weighted ? r->rdsp.put_pixels_tab : r->rdsp.avg_pixels_tab,
                     weighted ? r->rdsp.put_chroma_pixels_tab : r->rdsp.avg_chroma_pixels_tab);
        }

/** number of motion vectors in each macroblock type */
static const int num_mvs[RV34_MB_TYPES] = { 0, 0, 1, 4, 1, 1, 0, 0, 2, 2, 2, 1 };

/**
 * Decode motion vector differences
 * and perform motion vector reconstruction and motion compensation.
 */
{

            r->dmv[i][1] == INVALID_VLC) {
            r->dmv[i][0] = r->dmv[i][1] = 0;
            return AVERROR_INVALIDDATA;
        }
    }
    case RV34_MB_TYPE_INTRA16x16:
            break;
        }
    case RV34_MB_B_DIRECT:
        //surprisingly, it uses motion scheme from next reference frame
        /* wait for the current mb row to be finished */
            ff_thread_await_progress(&s->next_picture_ptr->tf, FFMAX(0, s->mb_y-1), 0);

        }else
        else
        break;
    case RV34_MB_P_MIX16x16:
        break;
    case RV34_MB_B_BACKWARD:
        else
        break;
    case RV34_MB_P_8x16:
        }
        }
        break;
    case RV34_MB_P_8x8:
        }
        break;
    }

    return 0;
}
/** @} */ // mv group

/**
 * @name Macroblock reconstruction functions
 * @{
 */
/** mapping of RV30/40 intra prediction types to standard H.264 types */
static const int ittrans[9] = {
 DC_PRED, VERT_PRED, HOR_PRED, DIAG_DOWN_RIGHT_PRED, DIAG_DOWN_LEFT_PRED,
 VERT_RIGHT_PRED, VERT_LEFT_PRED, HOR_UP_PRED, HOR_DOWN_PRED,
};

/** mapping of RV30/40 intra 16x16 prediction types to standard H.264 types */
static const int ittrans16[4] = {
 DC_PRED8x8, VERT_PRED8x8, HOR_PRED8x8, PLANE_PRED8x8,
};

/**
 * Perform 4x4 intra prediction.
 */
{

        itype = DC_128_PRED;
    }
    }
    }

{
        itype = DC_128_PRED8x8;
    }
}

                                      uint8_t *pdst, int stride,
                                      int fc, int sc, int q_dc, int q_ac)
{
                                   fc, sc, q_dc, q_ac, q_ac);
    }else{
    }

{


    else



            }else
                has_ac = 0;

            }else
        }

    }




                               r->chroma_vlc, 1, q_dc, q_ac);
        }
    }

{

    // Set neighbour information.


                               r->luma_vlc, 0, q_ac, q_ac);
        }
    }






                                   r->chroma_vlc, 1, q_dc, q_ac);
            }

        }
    }

{
        return 1;
        return 1;
    return 0;
}

{
        }
    }
    }
}

{

    // Calculate which neighbours are available. Maybe it's worth optimizing too.


        return -1;

    }

        // Only for RV34_MB_P_MIX16x16
        else



                }else
                    has_ac = 0;

                }else
            }

        }

    }else{


                                   r->luma_vlc, 0, q_ac, q_ac);
            }
        }
    }



                               r->chroma_vlc, 1, q_dc, q_ac);
        }
    }

    return 0;
}

{

    // Calculate which neighbours are available. Maybe it's worth optimizing too.


        return -1;

    }

}

static int check_slice_end(RV34DecContext *r, MpegEncContext *s)
{
    int bits;
    if(s->mb_y >= s->mb_height)
        return 1;
    if(!s->mb_num_left)
        return 1;
    if(r->s.mb_skip_run > 1)
        return 0;
    bits = get_bits_left(&s->gb);
    if(bits <= 0 || (bits < 8 && !show_bits(&s->gb, bits)))
        return 1;
    return 0;
}


{


{

                                    sizeof(*r->cbp_chroma));
                                    sizeof(*r->cbp_luma));
                                    sizeof(*r->deblock_coefs));
                                    sizeof(*r->intra_types_hist));
                                     sizeof(*r->mb_type));

        rv34_decoder_free(r);
        return AVERROR(ENOMEM);
    }


}


static int rv34_decoder_realloc(RV34DecContext *r)
{
    rv34_decoder_free(r);
    return rv34_decoder_alloc(r);
}


{

        av_log(s->avctx, AV_LOG_ERROR, "Incorrect or unknown slice header\n");
        return -1;
    }

        av_log(s->avctx, AV_LOG_ERROR, "Slice type mismatch\n");
        return AVERROR_INVALIDDATA;
    }
        av_log(s->avctx, AV_LOG_ERROR, "Size mismatch\n");
        return AVERROR_INVALIDDATA;
    }


        av_log(s->avctx, AV_LOG_ERROR, "Slice indicates MB offset %d, got %d\n", r->si.start, mb_pos);
        s->mb_x = r->si.start % s->mb_width;
        s->mb_y = r->si.start / s->mb_width;
    }


        else
            ff_er_add_slice(&s->er, s->resync_mb_x, s->resync_mb_y, s->mb_x-1, s->mb_y, ER_MB_ERROR);
            return -1;
        }



                ff_thread_report_progress(&s->current_picture_ptr->tf,
                                          s->mb_y - 2, 0);

        }
    }

}

/** @} */ // reconstruction group end

/**
 * Initialize decoder.
 */
{



        return ret;


#if CONFIG_RV30_DECODER
#endif
#if CONFIG_RV40_DECODER
#endif

        ff_mpv_common_end(&r->s);
        return ret;
    }


    return 0;
}

int ff_rv34_decode_update_thread_context(AVCodecContext *dst, const AVCodecContext *src)
{
    RV34DecContext *r = dst->priv_data, *r1 = src->priv_data;
    MpegEncContext * const s = &r->s, * const s1 = &r1->s;
    int err;

    if (dst == src || !s1->context_initialized)
        return 0;

    if (s->height != s1->height || s->width != s1->width) {
        s->height = s1->height;
        s->width  = s1->width;
        if ((err = ff_mpv_common_frame_size_change(s)) < 0)
            return err;
        if ((err = rv34_decoder_realloc(r)) < 0)
            return err;
    }

    r->cur_pts  = r1->cur_pts;
    r->last_pts = r1->last_pts;
    r->next_pts = r1->next_pts;

    memset(&r->si, 0, sizeof(r->si));

    // Do no call ff_mpeg_update_thread_context on a partially initialized
    // decoder context.
    if (!s1->context_initialized)
        return 0;

    return ff_mpeg_update_thread_context(dst, src);
}

static int get_slice_offset(AVCodecContext *avctx, const uint8_t *buf, int n, int slice_count, int buf_size)
{
    if (n < slice_count) {
        if(avctx->slice_count) return avctx->slice_offset[n];
        else                   return AV_RL32(buf + n*8 - 4) == 1 ? AV_RL32(buf + n*8) :  AV_RB32(buf + n*8);
    } else
        return buf_size;
}

static int finish_frame(AVCodecContext *avctx, AVFrame *pict)
{
    RV34DecContext *r = avctx->priv_data;
    MpegEncContext *s = &r->s;
    int got_picture = 0, ret;

    ff_er_frame_end(&s->er);
    ff_mpv_frame_end(s);
    s->mb_num_left = 0;

    if (HAVE_THREADS && (s->avctx->active_thread_type & FF_THREAD_FRAME))
        ff_thread_report_progress(&s->current_picture_ptr->tf, INT_MAX, 0);

    if (s->pict_type == AV_PICTURE_TYPE_B || s->low_delay) {
        if ((ret = av_frame_ref(pict, s->current_picture_ptr->f)) < 0)
            return ret;
        ff_print_debug_info(s, s->current_picture_ptr, pict);
        ff_mpv_export_qp_table(s, pict, s->current_picture_ptr, FF_QSCALE_TYPE_MPEG1);
        got_picture = 1;
    } else if (s->last_picture_ptr) {
        if ((ret = av_frame_ref(pict, s->last_picture_ptr->f)) < 0)
            return ret;
        ff_print_debug_info(s, s->last_picture_ptr, pict);
        ff_mpv_export_qp_table(s, pict, s->last_picture_ptr, FF_QSCALE_TYPE_MPEG1);
        got_picture = 1;
    }

    return got_picture;
}

static AVRational update_sar(int old_w, int old_h, AVRational sar, int new_w, int new_h)
{
    // attempt to keep aspect during typical resolution switches
    if (!sar.num)
        sar = (AVRational){1, 1};

    sar = av_mul_q(sar, av_mul_q((AVRational){new_h, new_w}, (AVRational){old_w, old_h}));
    return sar;
}

                            void *data, int *got_picture_ptr,
                            AVPacket *avpkt)
{

    /* no supplementary picture */
        /* special case for last picture */
                return ret;

        }
    }

    }else
        slice_count = avctx->slice_count;

    //parse first slice header to check whether this frame can be decoded
        av_log(avctx, AV_LOG_ERROR, "Slice offset is invalid\n");
        return AVERROR_INVALIDDATA;
    }
        av_log(avctx, AV_LOG_ERROR, "First slice header is incorrect\n");
        return AVERROR_INVALIDDATA;
    }
        av_log(avctx, AV_LOG_ERROR, "Invalid decoder state: B-frame without "
               "reference data.\n");
        faulty_b = 1;
    }
        return avpkt->size;

    /* first slice */
            av_log(avctx, AV_LOG_ERROR, "New frame but still %d MB left.\n",
                   s->mb_num_left);
            ff_er_frame_end(&s->er);
            ff_mpv_frame_end(s);
        }

            int err;

            av_log(s->avctx, AV_LOG_WARNING, "Changing dimensions to %dx%d\n",
                   si.width, si.height);

            if (av_image_check_size(si.width, si.height, 0, s->avctx))
                return AVERROR_INVALIDDATA;

            s->avctx->sample_aspect_ratio = update_sar(
                s->width, s->height, s->avctx->sample_aspect_ratio,
                si.width, si.height);
            s->width  = si.width;
            s->height = si.height;

            err = ff_set_dimensions(s->avctx, s->width, s->height);
            if (err < 0)
                return err;

            if ((err = ff_mpv_common_frame_size_change(s)) < 0)
                return err;
            if ((err = rv34_decoder_realloc(r)) < 0)
                return err;
        }
            return AVERROR_INVALIDDATA;
            return -1;

        }
        } else {

                r->mv_weight1 = r->mv_weight2 = r->weight1 = r->weight2 = 8192;
                r->scaled_weight = 0;
            }else{
                    av_log(avctx, AV_LOG_TRACE, "distance overflow\n");

                }else{
                }
            }
        }
    } else if (HAVE_THREADS &&
               (s->avctx->active_thread_type & FF_THREAD_FRAME)) {
        av_log(s->avctx, AV_LOG_ERROR, "Decoder needs full frames in frame "
               "multithreading mode (start MB is %d).\n", si.start);
        return AVERROR_INVALIDDATA;
    }


            av_log(avctx, AV_LOG_ERROR, "Slice offset is invalid\n");
            break;
        }


                av_log(avctx, AV_LOG_ERROR, "Slice offset is invalid\n");
                break;
            }
            }else
        }
            break;
    }


                return ret;
        } else if (HAVE_THREADS &&
                   (s->avctx->active_thread_type & FF_THREAD_FRAME)) {
            av_log(avctx, AV_LOG_INFO, "marking unfished frame as finished\n");
            /* always mark the current frame as finished, frame-mt supports
             * only complete frames */
            ff_er_frame_end(&s->er);
            ff_mpv_frame_end(s);
            s->mb_num_left = 0;
            ff_thread_report_progress(&s->current_picture_ptr->tf, INT_MAX, 0);
            return AVERROR_INVALIDDATA;
        }
    }

}

{


}
