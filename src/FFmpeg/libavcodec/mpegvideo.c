/*
 * The simplest mpeg encoder (well, it was the simplest!)
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

/**
 * @file
 * The simplest mpeg encoder (well, it was the simplest!).
 */

#include "libavutil/attributes.h"
#include "libavutil/avassert.h"
#include "libavutil/imgutils.h"
#include "libavutil/internal.h"
#include "libavutil/motion_vector.h"
#include "avcodec.h"
#include "blockdsp.h"
#include "h264chroma.h"
#include "idctdsp.h"
#include "internal.h"
#include "mathops.h"
#include "mpeg_er.h"
#include "mpegutils.h"
#include "mpegvideo.h"
#include "mpegvideodata.h"
#include "mjpegenc.h"
#include "msmpeg4.h"
#include "qpeldsp.h"
#include "thread.h"
#include "wmv2.h"
#include <limits.h>

static void dct_unquantize_mpeg1_intra_c(MpegEncContext *s,
                                   int16_t *block, int n, int qscale)
{
    int i, level, nCoeffs;
    const uint16_t *quant_matrix;

    nCoeffs= s->block_last_index[n];

    block[0] *= n < 4 ? s->y_dc_scale : s->c_dc_scale;
    /* XXX: only MPEG-1 */
    quant_matrix = s->intra_matrix;
    for(i=1;i<=nCoeffs;i++) {
        int j= s->intra_scantable.permutated[i];
        level = block[j];
        if (level) {
            if (level < 0) {
                level = -level;
                level = (int)(level * qscale * quant_matrix[j]) >> 3;
                level = (level - 1) | 1;
                level = -level;
            } else {
                level = (int)(level * qscale * quant_matrix[j]) >> 3;
                level = (level - 1) | 1;
            }
            block[j] = level;
        }
    }
}

static void dct_unquantize_mpeg1_inter_c(MpegEncContext *s,
                                   int16_t *block, int n, int qscale)
{
    int i, level, nCoeffs;
    const uint16_t *quant_matrix;

    nCoeffs= s->block_last_index[n];

    quant_matrix = s->inter_matrix;
    for(i=0; i<=nCoeffs; i++) {
        int j= s->intra_scantable.permutated[i];
        level = block[j];
        if (level) {
            if (level < 0) {
                level = -level;
                level = (((level << 1) + 1) * qscale *
                         ((int) (quant_matrix[j]))) >> 4;
                level = (level - 1) | 1;
                level = -level;
            } else {
                level = (((level << 1) + 1) * qscale *
                         ((int) (quant_matrix[j]))) >> 4;
                level = (level - 1) | 1;
            }
            block[j] = level;
        }
    }
}

static void dct_unquantize_mpeg2_intra_c(MpegEncContext *s,
                                   int16_t *block, int n, int qscale)
{
    int i, level, nCoeffs;
    const uint16_t *quant_matrix;

    if (s->q_scale_type) qscale = ff_mpeg2_non_linear_qscale[qscale];
    else                 qscale <<= 1;

    if(s->alternate_scan) nCoeffs= 63;
    else nCoeffs= s->block_last_index[n];

    block[0] *= n < 4 ? s->y_dc_scale : s->c_dc_scale;
    quant_matrix = s->intra_matrix;
    for(i=1;i<=nCoeffs;i++) {
        int j= s->intra_scantable.permutated[i];
        level = block[j];
        if (level) {
            if (level < 0) {
                level = -level;
                level = (int)(level * qscale * quant_matrix[j]) >> 4;
                level = -level;
            } else {
                level = (int)(level * qscale * quant_matrix[j]) >> 4;
            }
            block[j] = level;
        }
    }
}

                                   int16_t *block, int n, int qscale)
{



            } else {
            }
        }
    }

static void dct_unquantize_mpeg2_inter_c(MpegEncContext *s,
                                   int16_t *block, int n, int qscale)
{
    int i, level, nCoeffs;
    const uint16_t *quant_matrix;
    int sum=-1;

    if (s->q_scale_type) qscale = ff_mpeg2_non_linear_qscale[qscale];
    else                 qscale <<= 1;

    if(s->alternate_scan) nCoeffs= 63;
    else nCoeffs= s->block_last_index[n];

    quant_matrix = s->inter_matrix;
    for(i=0; i<=nCoeffs; i++) {
        int j= s->intra_scantable.permutated[i];
        level = block[j];
        if (level) {
            if (level < 0) {
                level = -level;
                level = (((level << 1) + 1) * qscale *
                         ((int) (quant_matrix[j]))) >> 5;
                level = -level;
            } else {
                level = (((level << 1) + 1) * qscale *
                         ((int) (quant_matrix[j]))) >> 5;
            }
            block[j] = level;
            sum+=level;
        }
    }
    block[63]^=sum&1;
}

static void dct_unquantize_h263_intra_c(MpegEncContext *s,
                                  int16_t *block, int n, int qscale)
{
    int i, level, qmul, qadd;
    int nCoeffs;

    av_assert2(s->block_last_index[n]>=0 || s->h263_aic);

    qmul = qscale << 1;

    if (!s->h263_aic) {
        block[0] *= n < 4 ? s->y_dc_scale : s->c_dc_scale;
        qadd = (qscale - 1) | 1;
    }else{
        qadd = 0;
    }
    if(s->ac_pred)
        nCoeffs=63;
    else
        nCoeffs= s->intra_scantable.raster_end[ s->block_last_index[n] ];

    for(i=1; i<=nCoeffs; i++) {
        level = block[i];
        if (level) {
            if (level < 0) {
                level = level * qmul - qadd;
            } else {
                level = level * qmul + qadd;
            }
            block[i] = level;
        }
    }
}

static void dct_unquantize_h263_inter_c(MpegEncContext *s,
                                  int16_t *block, int n, int qscale)
{
    int i, level, qmul, qadd;
    int nCoeffs;

    av_assert2(s->block_last_index[n]>=0);

    qadd = (qscale - 1) | 1;
    qmul = qscale << 1;

    nCoeffs= s->inter_scantable.raster_end[ s->block_last_index[n] ];

    for(i=0; i<=nCoeffs; i++) {
        level = block[i];
        if (level) {
            if (level < 0) {
                level = level * qmul - qadd;
            } else {
                level = level * qmul + qadd;
            }
            block[i] = level;
        }
    }
}


static void gray16(uint8_t *dst, const uint8_t *src, ptrdiff_t linesize, int h)
{
    while(h--)
        memset(dst + h*linesize, 128, 16);
}

static void gray8(uint8_t *dst, const uint8_t *src, ptrdiff_t linesize, int h)
{
    while(h--)
        memset(dst + h*linesize, 128, 8);
}

/* init common dct for both encoder and decoder */
{

        int i;
        for (i=0; i<4; i++) {
            s->hdsp.avg_pixels_tab[0][i] = gray16;
            s->hdsp.put_pixels_tab[0][i] = gray16;
            s->hdsp.put_no_rnd_pixels_tab[0][i] = gray16;

            s->hdsp.avg_pixels_tab[1][i] = gray8;
            s->hdsp.put_pixels_tab[1][i] = gray8;
            s->hdsp.put_no_rnd_pixels_tab[1][i] = gray8;
        }
    }


        ff_mpv_common_init_neon(s);

        ff_mpv_common_init_axp(s);
        ff_mpv_common_init_arm(s);
        ff_mpv_common_init_ppc(s);
        ff_mpv_common_init_mips(s);

}

{

    /* load & permutate scantables
     * note: only wmv uses different ones
     */
        ff_init_scantable(s->idsp.idct_permutation, &s->inter_scantable, ff_alternate_vertical_scan);
        ff_init_scantable(s->idsp.idct_permutation, &s->intra_scantable, ff_alternate_vertical_scan);
    } else {
    }

{
                            s->mb_stride, s->mb_width, s->mb_height, s->b8_stride,
                            &s->linesize, &s->uvlinesize);
}

{



            return AVERROR(ENOMEM);

                return AVERROR(ENOMEM);
        }
    }
        return AVERROR(ENOMEM);

    }

        return AVERROR(ENOMEM);

        // exchange uv
    }

        /* ac values */
            return AVERROR(ENOMEM);
    }

    return 0;
}

{
        return;


}

{
#define COPY(a) bak->a = src->a
#undef COPY

{
    // FIXME copy only needed parts
    }
        // exchange uv
        FFSWAP(void *, dst->pblocks[4], dst->pblocks[5]);
    }
        av_log(dst->avctx, AV_LOG_ERROR, "failed to allocate context "
               "scratch buffers.\n");
        return ret;
    }
    return 0;
}

int ff_mpeg_update_thread_context(AVCodecContext *dst,
                                  const AVCodecContext *src)
{
    int i, ret;
    MpegEncContext *s = dst->priv_data, *s1 = src->priv_data;

    if (dst == src)
        return 0;

    av_assert0(s != s1);

    // FIXME can parameters change on I-frames?
    // in that case dst may need a reinit
    if (!s->context_initialized) {
        int err;
        memcpy(s, s1, sizeof(MpegEncContext));

        s->avctx                 = dst;
        s->bitstream_buffer      = NULL;
        s->bitstream_buffer_size = s->allocated_bitstream_buffer_size = 0;

        if (s1->context_initialized){
//             s->picture_range_start  += MAX_PICTURE_COUNT;
//             s->picture_range_end    += MAX_PICTURE_COUNT;
            ff_mpv_idct_init(s);
            if((err = ff_mpv_common_init(s)) < 0){
                memset(s, 0, sizeof(MpegEncContext));
                s->avctx = dst;
                return err;
            }
        }
    }

    if (s->height != s1->height || s->width != s1->width || s->context_reinit) {
        s->context_reinit = 0;
        s->height = s1->height;
        s->width  = s1->width;
        if ((ret = ff_mpv_common_frame_size_change(s)) < 0)
            return ret;
    }

    s->avctx->coded_height  = s1->avctx->coded_height;
    s->avctx->coded_width   = s1->avctx->coded_width;
    s->avctx->width         = s1->avctx->width;
    s->avctx->height        = s1->avctx->height;

    s->quarter_sample       = s1->quarter_sample;

    s->coded_picture_number = s1->coded_picture_number;
    s->picture_number       = s1->picture_number;

    av_assert0(!s->picture || s->picture != s1->picture);
    if(s->picture)
    for (i = 0; i < MAX_PICTURE_COUNT; i++) {
        ff_mpeg_unref_picture(s->avctx, &s->picture[i]);
        if (s1->picture && s1->picture[i].f->buf[0] &&
            (ret = ff_mpeg_ref_picture(s->avctx, &s->picture[i], &s1->picture[i])) < 0)
            return ret;
    }

#define UPDATE_PICTURE(pic)\
do {\
    ff_mpeg_unref_picture(s->avctx, &s->pic);\
    if (s1->pic.f && s1->pic.f->buf[0])\
        ret = ff_mpeg_ref_picture(s->avctx, &s->pic, &s1->pic);\
    else\
        ret = ff_update_picture_tables(&s->pic, &s1->pic);\
    if (ret < 0)\
        return ret;\
} while (0)

    UPDATE_PICTURE(current_picture);
    UPDATE_PICTURE(last_picture);
    UPDATE_PICTURE(next_picture);

#define REBASE_PICTURE(pic, new_ctx, old_ctx)                                 \
    ((pic && pic >= old_ctx->picture &&                                       \
      pic < old_ctx->picture + MAX_PICTURE_COUNT) ?                           \
        &new_ctx->picture[pic - old_ctx->picture] : NULL)

    s->last_picture_ptr    = REBASE_PICTURE(s1->last_picture_ptr,    s, s1);
    s->current_picture_ptr = REBASE_PICTURE(s1->current_picture_ptr, s, s1);
    s->next_picture_ptr    = REBASE_PICTURE(s1->next_picture_ptr,    s, s1);

    // Error/bug resilience
    s->next_p_frame_damaged = s1->next_p_frame_damaged;
    s->workaround_bugs      = s1->workaround_bugs;
    s->padding_bug_score    = s1->padding_bug_score;

    // MPEG-4 timing info
    memcpy(&s->last_time_base, &s1->last_time_base,
           (char *) &s1->pb_field_time + sizeof(s1->pb_field_time) -
           (char *) &s1->last_time_base);

    // B-frame info
    s->max_b_frames = s1->max_b_frames;
    s->low_delay    = s1->low_delay;
    s->droppable    = s1->droppable;

    // DivX handling (doesn't work)
    s->divx_packed  = s1->divx_packed;

    if (s1->bitstream_buffer) {
        if (s1->bitstream_buffer_size +
            AV_INPUT_BUFFER_PADDING_SIZE > s->allocated_bitstream_buffer_size) {
            av_fast_malloc(&s->bitstream_buffer,
                           &s->allocated_bitstream_buffer_size,
                           s1->allocated_bitstream_buffer_size);
            if (!s->bitstream_buffer) {
                s->bitstream_buffer_size = 0;
                return AVERROR(ENOMEM);
            }
        }
        s->bitstream_buffer_size = s1->bitstream_buffer_size;
        memcpy(s->bitstream_buffer, s1->bitstream_buffer,
               s1->bitstream_buffer_size);
        memset(s->bitstream_buffer + s->bitstream_buffer_size, 0,
               AV_INPUT_BUFFER_PADDING_SIZE);
    }

    // linesize-dependent scratch buffer allocation
    if (!s->sc.edge_emu_buffer)
        if (s1->linesize) {
            if (ff_mpeg_framesize_alloc(s->avctx, &s->me,
                                        &s->sc, s1->linesize) < 0) {
                av_log(s->avctx, AV_LOG_ERROR, "Failed to allocate context "
                       "scratch buffers.\n");
                return AVERROR(ENOMEM);
            }
        } else {
            av_log(s->avctx, AV_LOG_ERROR, "Context scratch buffers could not "
                   "be allocated due to unknown size.\n");
        }

    // MPEG-2/interlacing info
    memcpy(&s->progressive_sequence, &s1->progressive_sequence,
           (char *) &s1->rtp_mode - (char *) &s1->progressive_sequence);

    if (!s1->first_field) {
        s->last_pict_type = s1->pict_type;
        if (s1->current_picture_ptr)
            s->last_lambda_for[s1->pict_type] = s1->current_picture_ptr->f->quality;
    }

    return 0;
}

/**
 * Set the given MpegEncContext to common defaults
 * (same for encoding and decoding).
 * The changed fields will not depend upon the
 * prior state of the MpegEncContext.
 */
{




/**
 * Set the given MpegEncContext to defaults for decoding.
 * the changed fields will not depend upon
 * the prior state of the MpegEncContext.
 */
{

{

    /* convert fourcc to upper case */

/**
 * Initialize and allocates MpegEncContext fields dependent on the resolution.
 */
{


    /* set default edge pos, will be overridden
     * in decode_header if needed */





        return AVERROR(ENOMEM);


        /* Allocate MV tables */
            return AVERROR(ENOMEM);

        /* Allocate MB type table */
            return AVERROR(ENOMEM);
    }

        /* interlaced direct mode decoding tables */
            int j, k;
                        return AVERROR(ENOMEM);
                }
                    return AVERROR(ENOMEM);
            }
                return AVERROR(ENOMEM);
        }
    }
        /* cbp values, cbp, ac_pred, pred_dir */
            return AVERROR(ENOMEM);
    }

        /* dc values */
        // MN: we need these for error resilience of intra-frames
            return AVERROR(ENOMEM);
    }

    /* which mb is an intra block,  init macroblock skip table */
        // Note the + 1 is for a quicker MPEG-4 slice_end detection
        return AVERROR(ENOMEM);

}

{





            }
        }
    }





/**
 * init common structure for both encoder and decoder.
 * this assumes that some variables like width/height are already set
 */
{



    else

        av_log(s->avctx, AV_LOG_ERROR,
               "decoding to AV_PIX_FMT_NONE is not supported.\n");
        return AVERROR(EINVAL);
    }

        int max_slices;
        if (s->mb_height)
            max_slices = FFMIN(MAX_THREADS, s->mb_height);
        else
            max_slices = MAX_THREADS;
        av_log(s->avctx, AV_LOG_WARNING, "too many threads/slices (%d),"
               " reducing to %d\n", nb_slices, max_slices);
        nb_slices = max_slices;
    }

        return AVERROR(EINVAL);


    /* set chroma shifts */
                                           &s->chroma_x_shift,
                                           &s->chroma_y_shift);
        return ret;

        return AVERROR(ENOMEM);
            return AVERROR(ENOMEM);
    }

        return AVERROR(ENOMEM);

        return AVERROR(ENOMEM);



//     if (s->width && s->height) {
                    return AVERROR(ENOMEM);
            }
                return ret;
        }
    } else {
            return ret;
    }
//     }

}

/**
 * Frees and resets MpegEncContext fields depending on the resolution.
 * Is used during resolution changes to avoid a full reinitialization of the
 * codec.
 */
{

            }
        }
    }






{

        return AVERROR(EINVAL);

        for (i = 0; i < s->slice_context_count; i++) {
            free_duplicate_context(s->thread_context[i]);
        }
        for (i = 1; i < s->slice_context_count; i++) {
            av_freep(&s->thread_context[i]);
        }
    } else


        }


    // init
        s->mb_height = (s->height + 31) / 32 * 2;
    else

        return err;

        return err;


            for (i = 0; i < nb_slices; i++) {
                if (i) {
                    s->thread_context[i] = av_memdup(s, sizeof(MpegEncContext));
                    if (!s->thread_context[i]) {
                        return AVERROR(ENOMEM);
                    }
                }
                if ((err = init_duplicate_context(s->thread_context[i])) < 0)
                    return err;
                s->thread_context[i]->start_mb_y =
                    (s->mb_height * (i) + nb_slices / 2) / nb_slices;
                s->thread_context[i]->end_mb_y   =
                    (s->mb_height * (i + 1) + nb_slices / 2) / nb_slices;
            }
        } else {
                return err;
        }
    }

    return 0;
}

/* init common structure for both encoder and decoder */
{

        return;

        }
        }



        return;

        }
    }


}


static void gray_frame(AVFrame *frame)
{
    int i, h_chroma_shift, v_chroma_shift;

    av_pix_fmt_get_chroma_sub_sample(frame->format, &h_chroma_shift, &v_chroma_shift);

    for(i=0; i<frame->height; i++)
        memset(frame->data[0] + frame->linesize[0]*i, 0x80, frame->width);
    for(i=0; i<AV_CEIL_RSHIFT(frame->height, v_chroma_shift); i++) {
        memset(frame->data[1] + frame->linesize[1]*i,
               0x80, AV_CEIL_RSHIFT(frame->width, h_chroma_shift));
        memset(frame->data[2] + frame->linesize[2]*i,
               0x80, AV_CEIL_RSHIFT(frame->width, h_chroma_shift));
    }
}

/**
 * generic function called after decoding
 * the header and before a frame is decoded.
 */
{

        av_log(avctx, AV_LOG_ERROR, "Attempt to start a frame outside SETUP state\n");
        return -1;
    }

    /* mark & release old frames */
    }

    /* release forgotten pictures */
    /* if (MPEG-124 / H.263) */
            ff_mpeg_unref_picture(s->avctx, &s->picture[i]);
        }
    }


    /* release non reference frames */
    }

        // we already have an unused image
        // (maybe it was set before reading the header)
        pic = s->current_picture_ptr;
    } else {
            av_log(s->avctx, AV_LOG_ERROR, "no frame buffer available\n");
            return i;
        }
    }

    }


        return -1;

    // FIXME use only the vars from current_pic
        s->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
    }

    // if (s->avctx->flags && AV_CODEC_FLAG_QSCALE)
    //     s->current_picture_ptr->quality = s->new_picture_ptr->quality;

                                   s->current_picture_ptr)) < 0)
        return ret;

    }
            s->last_picture_ptr, s->next_picture_ptr,s->current_picture_ptr,
            s->last_picture_ptr    ? s->last_picture_ptr->f->data[0]    : NULL,
            s->next_picture_ptr    ? s->next_picture_ptr->f->data[0]    : NULL,
            s->current_picture_ptr ? s->current_picture_ptr->f->data[0] : NULL,
            s->pict_type, s->droppable);

        (s->pict_type != AV_PICTURE_TYPE_I)) {
                                         &h_chroma_shift, &v_chroma_shift);
            av_log(avctx, AV_LOG_DEBUG,
                   "allocating dummy last picture for B frame\n");
                   "warning: first frame is no keyframe\n");

        /* Allocate a dummy frame */
            av_log(s->avctx, AV_LOG_ERROR, "no frame buffer available\n");
            return i;
        }


            s->last_picture_ptr = NULL;
            return -1;
        }

                }
            }

                for(i=0; i<avctx->height; i++)
                memset(s->last_picture_ptr->f->data[0] + s->last_picture_ptr->f->linesize[0]*i, 16, avctx->width);
            }
        }

    }
        s->pict_type == AV_PICTURE_TYPE_B) {
        /* Allocate a dummy frame */
        i = ff_find_unused_picture(s->avctx, s->picture, 0);
        if (i < 0) {
            av_log(s->avctx, AV_LOG_ERROR, "no frame buffer available\n");
            return i;
        }
        s->next_picture_ptr = &s->picture[i];

        s->next_picture_ptr->reference   = 3;
        s->next_picture_ptr->f->key_frame = 0;
        s->next_picture_ptr->f->pict_type = AV_PICTURE_TYPE_P;

        if (alloc_picture(s, s->next_picture_ptr) < 0) {
            s->next_picture_ptr = NULL;
            return -1;
        }
        ff_thread_report_progress(&s->next_picture_ptr->tf, INT_MAX, 0);
        ff_thread_report_progress(&s->next_picture_ptr->tf, INT_MAX, 1);
    }

#if 0 // BUFREF-FIXME
    memset(s->last_picture.f->data, 0, sizeof(s->last_picture.f->data));
    memset(s->next_picture.f->data, 0, sizeof(s->next_picture.f->data));
#endif
                                       s->last_picture_ptr)) < 0)
            return ret;
    }
                                       s->next_picture_ptr)) < 0)
            return ret;
    }

                                                 s->last_picture_ptr->f->buf[0]));

        int i;
            }
        }
    }

    /* set dequantizer, we can't do it during init as
     * it might change for MPEG-4 and we can't do it in the header
     * decode as init is not called for MPEG-4 there yet */
    } else {
    }

        gray_frame(s->current_picture_ptr->f);
    }

    return 0;
}

/* called after a frame has been decoded. */
{


{
                         s->mb_width, s->mb_height, s->mb_stride, s->quarter_sample);

{
        return AVERROR(ENOMEM);
}

static inline int hpel_motion_lowres(MpegEncContext *s,
                                     uint8_t *dest, uint8_t *src,
                                     int field_based, int field_select,
                                     int src_x, int src_y,
                                     int width, int height, ptrdiff_t stride,
                                     int h_edge_pos, int v_edge_pos,
                                     int w, int h, h264_chroma_mc_func *pix_op,
                                     int motion_x, int motion_y)
{
    const int lowres   = s->avctx->lowres;
    const int op_index = FFMIN(lowres, 3);
    const int s_mask   = (2 << lowres) - 1;
    int emu = 0;
    int sx, sy;

    if (s->quarter_sample) {
        motion_x /= 2;
        motion_y /= 2;
    }

    sx = motion_x & s_mask;
    sy = motion_y & s_mask;
    src_x += motion_x >> lowres + 1;
    src_y += motion_y >> lowres + 1;

    src   += src_y * stride + src_x;

    if ((unsigned)src_x > FFMAX( h_edge_pos - (!!sx) - w,                 0) ||
        (unsigned)src_y > FFMAX((v_edge_pos >> field_based) - (!!sy) - h, 0)) {
        s->vdsp.emulated_edge_mc(s->sc.edge_emu_buffer, src,
                                 s->linesize, s->linesize,
                                 w + 1, (h + 1) << field_based,
                                 src_x, src_y   << field_based,
                                 h_edge_pos, v_edge_pos);
        src = s->sc.edge_emu_buffer;
        emu = 1;
    }

    sx = (sx << 2) >> lowres;
    sy = (sy << 2) >> lowres;
    if (field_select)
        src += s->linesize;
    pix_op[op_index](dest, src, stride, h, sx, sy);
    return emu;
}

/* apply one mpeg motion vector to the three components */
                                                uint8_t *dest_y,
                                                uint8_t *dest_cb,
                                                uint8_t *dest_cr,
                                                int field_based,
                                                int bottom_field,
                                                int field_select,
                                                uint8_t **ref_picture,
                                                h264_chroma_mc_func *pix_op,
                                                int motion_x, int motion_y,
                                                int h, int mb_y)
{

    // FIXME obviously not perfect but qpel will not work in lowres anyway
        motion_x /= 2;
        motion_y /= 2;
    }

        motion_y += (bottom_field - field_select)*((1 << lowres)-1);
    }


    } else if (s->out_format == FMT_H261) {
        // even chroma mv's are full pel in H261
        mx      = motion_x / 4;
        my      = motion_y / 4;
        uvsx    = (2 * mx) & s_mask;
        uvsy    = (2 * my) & s_mask;
        uvsrc_x = s->mb_x * block_s + (mx >> lowres);
        uvsrc_y =    mb_y * block_s + (my >> lowres);
    } else {
        if(s->chroma_y_shift){
            mx      = motion_x / 2;
            my      = motion_y / 2;
            uvsx    = mx & s_mask;
            uvsy    = my & s_mask;
            uvsrc_x = s->mb_x * block_s                 + (mx >> lowres + 1);
            uvsrc_y =   (mb_y * block_s >> field_based) + (my >> lowres + 1);
        } else {
            if(s->chroma_x_shift){
            //Chroma422
                mx = motion_x / 2;
                uvsx = mx & s_mask;
                uvsy = motion_y & s_mask;
                uvsrc_y = src_y;
                uvsrc_x = s->mb_x*block_s               + (mx >> (lowres+1));
            } else {
            //Chroma444
                uvsx = motion_x & s_mask;
                uvsy = motion_y & s_mask;
                uvsrc_x = src_x;
                uvsrc_y = src_y;
            }
        }
    }


                                 linesize >> field_based, linesize >> field_based,
                                 17, 17 + field_based,
                                src_x, src_y << field_based, h_edge_pos,
                                v_edge_pos);
                vbuf -= s->uvlinesize;
                                     uvlinesize >> field_based, uvlinesize >> field_based,
                                     9, 9 + field_based,
                                    uvsrc_x, uvsrc_y << field_based,
                                    h_edge_pos >> 1, v_edge_pos >> 1);
                                     uvlinesize >> field_based,uvlinesize >> field_based,
                                     9, 9 + field_based,
                                    uvsrc_x, uvsrc_y << field_based,
                                    h_edge_pos >> 1, v_edge_pos >> 1);
        }
    }

    // FIXME use this for field pix too instead of the obnoxious hack which changes picture.f->data
    if (bottom_field) {
        dest_y  += s->linesize;
        dest_cb += s->uvlinesize;
        dest_cr += s->uvlinesize;
    }

    if (field_select) {
        ptr_y   += s->linesize;
        ptr_cb  += s->uvlinesize;
        ptr_cr  += s->uvlinesize;
    }


        }
    }
    // FIXME h261 lowres loop filter
}

                                            uint8_t *dest_cb, uint8_t *dest_cr,
                                            uint8_t **ref_picture,
                                            h264_chroma_mc_func * pix_op,
                                            int mx, int my)
{

        mx /= 2;
        my /= 2;
    }

    /* In case of 8X8, we construct a single chroma motion vector
       with a special rounding */


                                 s->uvlinesize, s->uvlinesize,
                                 9, 9,
                                 src_x, src_y, h_edge_pos, v_edge_pos);
    }

                                 s->uvlinesize, s->uvlinesize,
                                 9, 9,
                                 src_x, src_y, h_edge_pos, v_edge_pos);
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
 * the motion vectors are taken from s->mv and the MV type from s->mv_type
 */
                                     uint8_t *dest_y, uint8_t *dest_cb,
                                     uint8_t *dest_cr,
                                     int dir, uint8_t **ref_picture,
                                     h264_chroma_mc_func *pix_op)
{


                           0, 0, 0,
                           ref_picture, pix_op,
                           s->mv[dir][0][0], s->mv[dir][0][1],
                           2 * block_s, mb_y);
        break;
    case MV_TYPE_8X8:
        mx = 0;
        my = 0;
                               ref_picture[0], 0, 0,
                               s->width, s->height, s->linesize,
                               block_s, block_s, pix_op,
                               s->mv[dir][i][0], s->mv[dir][i][1]);

        }

                                     pix_op, mx, my);
    case MV_TYPE_FIELD:
        if (s->picture_structure == PICT_FRAME) {
            /* top field */
            mpeg_motion_lowres(s, dest_y, dest_cb, dest_cr,
                               1, 0, s->field_select[dir][0],
                               ref_picture, pix_op,
                               s->mv[dir][0][0], s->mv[dir][0][1],
                               block_s, mb_y);
            /* bottom field */
            mpeg_motion_lowres(s, dest_y, dest_cb, dest_cr,
                               1, 1, s->field_select[dir][1],
                               ref_picture, pix_op,
                               s->mv[dir][1][0], s->mv[dir][1][1],
                               block_s, mb_y);
        } else {
            if (s->picture_structure != s->field_select[dir][0] + 1 &&
                s->pict_type != AV_PICTURE_TYPE_B && !s->first_field) {
                ref_picture = s->current_picture_ptr->f->data;

            }
            mpeg_motion_lowres(s, dest_y, dest_cb, dest_cr,
                               0, 0, s->field_select[dir][0],
                               ref_picture, pix_op,
                               s->mv[dir][0][0],
                               s->mv[dir][0][1], 2 * block_s, mb_y >> 1);
            }
        break;
    case MV_TYPE_16X8:
        for (i = 0; i < 2; i++) {
            uint8_t **ref2picture;

            if (s->picture_structure == s->field_select[dir][i] + 1 ||
                s->pict_type == AV_PICTURE_TYPE_B || s->first_field) {
                ref2picture = ref_picture;
            } else {
                ref2picture = s->current_picture_ptr->f->data;
            }

            mpeg_motion_lowres(s, dest_y, dest_cb, dest_cr,
                               0, 0, s->field_select[dir][i],
                               ref2picture, pix_op,
                               s->mv[dir][i][0], s->mv[dir][i][1] +
                               2 * block_s * i, block_s, mb_y >> 1);

            dest_y  +=  2 * block_s *  s->linesize;
            dest_cb += (2 * block_s >> s->chroma_y_shift) * s->uvlinesize;
            dest_cr += (2 * block_s >> s->chroma_y_shift) * s->uvlinesize;
        }
        break;
    case MV_TYPE_DMV:
        if (s->picture_structure == PICT_FRAME) {
            for (i = 0; i < 2; i++) {
                int j;
                for (j = 0; j < 2; j++) {
                    mpeg_motion_lowres(s, dest_y, dest_cb, dest_cr,
                                       1, j, j ^ i,
                                       ref_picture, pix_op,
                                       s->mv[dir][2 * i + j][0],
                                       s->mv[dir][2 * i + j][1],
                                       block_s, mb_y);
                }
                pix_op = s->h264chroma.avg_h264_chroma_pixels_tab;
            }
        } else {
            for (i = 0; i < 2; i++) {
                mpeg_motion_lowres(s, dest_y, dest_cb, dest_cr,
                                   0, 0, s->picture_structure != i + 1,
                                   ref_picture, pix_op,
                                   s->mv[dir][2 * i][0],s->mv[dir][2 * i][1],
                                   2 * block_s, mb_y >> 1);

                // after put we make avg of the same block
                pix_op = s->h264chroma.avg_h264_chroma_pixels_tab;

                // opposite parity is always in the same
                // frame if this is second field
                if (!s->first_field) {
                    ref_picture = s->current_picture_ptr->f->data;
                }
            }
        }
        break;
    }

/**
 * find the lowest MB row referenced in the MVs
 */
static int lowest_referenced_row(MpegEncContext *s, int dir)
{
    int my_max = INT_MIN, my_min = INT_MAX, qpel_shift = !s->quarter_sample;
    int my, off, i, mvs;

    if (s->picture_structure != PICT_FRAME || s->mcsel)
        goto unhandled;

    switch (s->mv_type) {
        case MV_TYPE_16X16:
            mvs = 1;
            break;
        case MV_TYPE_16X8:
            mvs = 2;
            break;
        case MV_TYPE_8X8:
            mvs = 4;
            break;
        default:
            goto unhandled;
    }

    for (i = 0; i < mvs; i++) {
        my = s->mv[dir][i][1];
        my_max = FFMAX(my_max, my);
        my_min = FFMIN(my_min, my);
    }

    off = ((FFMAX(-my_min, my_max)<<qpel_shift) + 63) >> 6;

    return av_clip(s->mb_y + off, 0, s->mb_height - 1);
unhandled:
    return s->mb_height-1;
}

/* put block[] to dest[] */
                           int16_t *block, int i, uint8_t *dest, int line_size, int qscale)
{

/* add block[] to dest[] */
                           int16_t *block, int i, uint8_t *dest, int line_size)
{
    }

                           int16_t *block, int i, uint8_t *dest, int line_size, int qscale)
{

    }

/**
 * Clean dc, ac, coded_block for the current non-intra MB.
 */
{

    /* ac pred */
    }
    /* chroma */
    /* ac pred */


/* generic function called after a macroblock has been parsed by the
   decoder or after it has been encoded by the encoder.

   Important variables used:
   s->mb_intra : true if intra macroblock
   s->mv_dir   : motion vector direction
   s->mv_type  : motion vector type
   s->mv       : motion vector
   s->interlaced_dct : true if interlaced dct used (mpeg2)
 */
static av_always_inline
                            int lowres_flag, int is_mpeg12)
{

        s->avctx->hwaccel && s->avctx->hwaccel->decode_mb) {
        s->avctx->hwaccel->decode_mb(s);//xvmc uses pblocks
        return;
    }

       /* print DCT coefficients */
       int i,j;
       av_log(s->avctx, AV_LOG_DEBUG, "DCT coeffs of MB at %dx%d:\n", s->mb_x, s->mb_y);
       for(i=0; i<6; i++){
           for(j=0; j<64; j++){
               av_log(s->avctx, AV_LOG_DEBUG, "%5d",
                      block[i][s->idsp.idct_permutation[j]]);
           }
           av_log(s->avctx, AV_LOG_DEBUG, "\n");
       }
    }


    /* update DC predictors for P macroblocks */
        } else {
        }
    }


        /* avoid copy if macroblock skipped in last frame too */
        /* skip only during decoding as we might trash the buffers during encoding a bit */

            } else{
            }
        }


        }else{
        }

            /* motion handling */
            /* decoding or more than one mb_type (MC was already done otherwise) */

                    if (s->mv_dir & MV_DIR_FORWARD) {
                        ff_thread_await_progress(&s->last_picture_ptr->tf,
                                                 lowest_referenced_row(s, 0),
                                                 0);
                    }
                    if (s->mv_dir & MV_DIR_BACKWARD) {
                        ff_thread_await_progress(&s->next_picture_ptr->tf,
                                                 lowest_referenced_row(s, 1),
                                                 0);
                    }
                }


                    }
                    }
                }else{
                    }else{
                    }
                    }
                    }
                }
            }

            /* skip dequant / idct if we are really late ;) */
                if(  (s->avctx->skip_idct >= AVDISCARD_NONREF && s->pict_type == AV_PICTURE_TYPE_B)
                   ||(s->avctx->skip_idct >= AVDISCARD_NONKEY && s->pict_type != AV_PICTURE_TYPE_I)
                   || s->avctx->skip_idct >= AVDISCARD_ALL)
                    goto skip_idct;
            }

            /* add dct residue */

                    }else{
                    }
                }

                    }else{
                        //chroma422

                            add_dct(s, block[8], 8, dest_cb+block_size, dct_linesize);
                            add_dct(s, block[9], 9, dest_cr+block_size, dct_linesize);
                            add_dct(s, block[10], 10, dest_cb+block_size+dct_offset, dct_linesize);
                            add_dct(s, block[11], 11, dest_cr+block_size+dct_offset, dct_linesize);
                        }
                    }
                }//fi gray
            }
            }
        } else {
            /* Only MPEG-4 Simple Studio Profile is supported in > 8-bit mode.
               TODO: Integrate 10-bit properly into mpegvideo.c so that ER works properly */
                const int act_block_size = block_size * 2;

                if(s->dpcm_direction == 0) {
                    s->idsp.idct_put(dest_y,                           dct_linesize, (int16_t*)(*s->block32)[0]);
                    s->idsp.idct_put(dest_y              + act_block_size, dct_linesize, (int16_t*)(*s->block32)[1]);
                    s->idsp.idct_put(dest_y + dct_offset,              dct_linesize, (int16_t*)(*s->block32)[2]);
                    s->idsp.idct_put(dest_y + dct_offset + act_block_size, dct_linesize, (int16_t*)(*s->block32)[3]);

                    dct_linesize = uvlinesize << s->interlaced_dct;
                    dct_offset   = s->interlaced_dct ? uvlinesize : uvlinesize*block_size;

                    s->idsp.idct_put(dest_cb,              dct_linesize, (int16_t*)(*s->block32)[4]);
                    s->idsp.idct_put(dest_cr,              dct_linesize, (int16_t*)(*s->block32)[5]);
                    s->idsp.idct_put(dest_cb + dct_offset, dct_linesize, (int16_t*)(*s->block32)[6]);
                    s->idsp.idct_put(dest_cr + dct_offset, dct_linesize, (int16_t*)(*s->block32)[7]);
                    if(!s->chroma_x_shift){//Chroma444
                        s->idsp.idct_put(dest_cb + act_block_size,              dct_linesize, (int16_t*)(*s->block32)[8]);
                        s->idsp.idct_put(dest_cr + act_block_size,              dct_linesize, (int16_t*)(*s->block32)[9]);
                        s->idsp.idct_put(dest_cb + act_block_size + dct_offset, dct_linesize, (int16_t*)(*s->block32)[10]);
                        s->idsp.idct_put(dest_cr + act_block_size + dct_offset, dct_linesize, (int16_t*)(*s->block32)[11]);
                    }
                } else if(s->dpcm_direction == 1) {
                    int i, w, h;
                    uint16_t *dest_pcm[3] = {(uint16_t*)dest_y, (uint16_t*)dest_cb, (uint16_t*)dest_cr};
                    int linesize[3] = {dct_linesize, uvlinesize, uvlinesize};
                    for(i = 0; i < 3; i++) {
                        int idx = 0;
                        int vsub = i ? s->chroma_y_shift : 0;
                        int hsub = i ? s->chroma_x_shift : 0;
                        for(h = 0; h < (16 >> vsub); h++){
                            for(w = 0; w < (16 >> hsub); w++)
                                dest_pcm[i][w] = (*s->dpcm_macroblock)[i][idx++];
                            dest_pcm[i] += linesize[i] / 2;
                        }
                    }
                } else if(s->dpcm_direction == -1) {
                    int i, w, h;
                    uint16_t *dest_pcm[3] = {(uint16_t*)dest_y, (uint16_t*)dest_cb, (uint16_t*)dest_cr};
                    int linesize[3] = {dct_linesize, uvlinesize, uvlinesize};
                    for(i = 0; i < 3; i++) {
                        int idx = 0;
                        int vsub = i ? s->chroma_y_shift : 0;
                        int hsub = i ? s->chroma_x_shift : 0;
                        dest_pcm[i] += (linesize[i] / 2) * ((16 >> vsub) - 1);
                        for(h = (16 >> vsub)-1; h >= 1; h--){
                            for(w = (16 >> hsub)-1; w >= 1; w--)
                                dest_pcm[i][w] = (*s->dpcm_macroblock)[i][idx++];
                            dest_pcm[i] -= linesize[i] / 2;
                        }
                    }
                }
            }
            /* dct only in intra block */

                    }else{
                    }
                }
            }else{

                    }else{


                            s->idsp.idct_put(dest_cb + block_size,              dct_linesize, block[8]);
                            s->idsp.idct_put(dest_cr + block_size,              dct_linesize, block[9]);
                            s->idsp.idct_put(dest_cb + block_size + dct_offset, dct_linesize, block[10]);
                            s->idsp.idct_put(dest_cr + block_size + dct_offset, dct_linesize, block[11]);
                        }
                    }
                }//gray
            }
        }
            }
        }
    }
}

{
#if !CONFIG_SMALL
    } else
#endif

{
                       s->first_field, s->low_delay);


    //block_index is not used by mpeg2, so it is not affected by chroma_format


    {
        }else{
        }
    }


        return;




}

/**
 * set qscale and update qscale dependent variables.
 */
{
        qscale = 1;
    else if (qscale > 31)
        qscale = 31;



{
