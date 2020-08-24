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
 * H.264 / AVC / MPEG-4 part10 codec.
 * @author Michael Niedermayer <michaelni@gmx.at>
 */

#include "libavutil/avassert.h"
#include "libavutil/display.h"
#include "libavutil/imgutils.h"
#include "libavutil/stereo3d.h"
#include "internal.h"
#include "cabac.h"
#include "cabac_functions.h"
#include "error_resilience.h"
#include "avcodec.h"
#include "h264.h"
#include "h264dec.h"
#include "h264data.h"
#include "h264chroma.h"
#include "h264_mvpred.h"
#include "h264_ps.h"
#include "golomb.h"
#include "mathops.h"
#include "mpegutils.h"
#include "mpegvideo.h"
#include "rectangle.h"
#include "thread.h"

static const uint8_t field_scan[16+1] = {
    0 + 0 * 4, 0 + 1 * 4, 1 + 0 * 4, 0 + 2 * 4,
    0 + 3 * 4, 1 + 1 * 4, 1 + 2 * 4, 1 + 3 * 4,
    2 + 0 * 4, 2 + 1 * 4, 2 + 2 * 4, 2 + 3 * 4,
    3 + 0 * 4, 3 + 1 * 4, 3 + 2 * 4, 3 + 3 * 4,
};

static const uint8_t field_scan8x8[64+1] = {
    0 + 0 * 8, 0 + 1 * 8, 0 + 2 * 8, 1 + 0 * 8,
    1 + 1 * 8, 0 + 3 * 8, 0 + 4 * 8, 1 + 2 * 8,
    2 + 0 * 8, 1 + 3 * 8, 0 + 5 * 8, 0 + 6 * 8,
    0 + 7 * 8, 1 + 4 * 8, 2 + 1 * 8, 3 + 0 * 8,
    2 + 2 * 8, 1 + 5 * 8, 1 + 6 * 8, 1 + 7 * 8,
    2 + 3 * 8, 3 + 1 * 8, 4 + 0 * 8, 3 + 2 * 8,
    2 + 4 * 8, 2 + 5 * 8, 2 + 6 * 8, 2 + 7 * 8,
    3 + 3 * 8, 4 + 1 * 8, 5 + 0 * 8, 4 + 2 * 8,
    3 + 4 * 8, 3 + 5 * 8, 3 + 6 * 8, 3 + 7 * 8,
    4 + 3 * 8, 5 + 1 * 8, 6 + 0 * 8, 5 + 2 * 8,
    4 + 4 * 8, 4 + 5 * 8, 4 + 6 * 8, 4 + 7 * 8,
    5 + 3 * 8, 6 + 1 * 8, 6 + 2 * 8, 5 + 4 * 8,
    5 + 5 * 8, 5 + 6 * 8, 5 + 7 * 8, 6 + 3 * 8,
    7 + 0 * 8, 7 + 1 * 8, 6 + 4 * 8, 6 + 5 * 8,
    6 + 6 * 8, 6 + 7 * 8, 7 + 2 * 8, 7 + 3 * 8,
    7 + 4 * 8, 7 + 5 * 8, 7 + 6 * 8, 7 + 7 * 8,
};

static const uint8_t field_scan8x8_cavlc[64+1] = {
    0 + 0 * 8, 1 + 1 * 8, 2 + 0 * 8, 0 + 7 * 8,
    2 + 2 * 8, 2 + 3 * 8, 2 + 4 * 8, 3 + 3 * 8,
    3 + 4 * 8, 4 + 3 * 8, 4 + 4 * 8, 5 + 3 * 8,
    5 + 5 * 8, 7 + 0 * 8, 6 + 6 * 8, 7 + 4 * 8,
    0 + 1 * 8, 0 + 3 * 8, 1 + 3 * 8, 1 + 4 * 8,
    1 + 5 * 8, 3 + 1 * 8, 2 + 5 * 8, 4 + 1 * 8,
    3 + 5 * 8, 5 + 1 * 8, 4 + 5 * 8, 6 + 1 * 8,
    5 + 6 * 8, 7 + 1 * 8, 6 + 7 * 8, 7 + 5 * 8,
    0 + 2 * 8, 0 + 4 * 8, 0 + 5 * 8, 2 + 1 * 8,
    1 + 6 * 8, 4 + 0 * 8, 2 + 6 * 8, 5 + 0 * 8,
    3 + 6 * 8, 6 + 0 * 8, 4 + 6 * 8, 6 + 2 * 8,
    5 + 7 * 8, 6 + 4 * 8, 7 + 2 * 8, 7 + 6 * 8,
    1 + 0 * 8, 1 + 2 * 8, 0 + 6 * 8, 3 + 0 * 8,
    1 + 7 * 8, 3 + 2 * 8, 2 + 7 * 8, 4 + 2 * 8,
    3 + 7 * 8, 5 + 2 * 8, 4 + 7 * 8, 5 + 4 * 8,
    6 + 3 * 8, 6 + 5 * 8, 7 + 3 * 8, 7 + 7 * 8,
};

// zigzag_scan8x8_cavlc[i] = zigzag_scan8x8[(i/4) + 16*(i%4)]
static const uint8_t zigzag_scan8x8_cavlc[64+1] = {
    0 + 0 * 8, 1 + 1 * 8, 1 + 2 * 8, 2 + 2 * 8,
    4 + 1 * 8, 0 + 5 * 8, 3 + 3 * 8, 7 + 0 * 8,
    3 + 4 * 8, 1 + 7 * 8, 5 + 3 * 8, 6 + 3 * 8,
    2 + 7 * 8, 6 + 4 * 8, 5 + 6 * 8, 7 + 5 * 8,
    1 + 0 * 8, 2 + 0 * 8, 0 + 3 * 8, 3 + 1 * 8,
    3 + 2 * 8, 0 + 6 * 8, 4 + 2 * 8, 6 + 1 * 8,
    2 + 5 * 8, 2 + 6 * 8, 6 + 2 * 8, 5 + 4 * 8,
    3 + 7 * 8, 7 + 3 * 8, 4 + 7 * 8, 7 + 6 * 8,
    0 + 1 * 8, 3 + 0 * 8, 0 + 4 * 8, 4 + 0 * 8,
    2 + 3 * 8, 1 + 5 * 8, 5 + 1 * 8, 5 + 2 * 8,
    1 + 6 * 8, 3 + 5 * 8, 7 + 1 * 8, 4 + 5 * 8,
    4 + 6 * 8, 7 + 4 * 8, 5 + 7 * 8, 6 + 7 * 8,
    0 + 2 * 8, 2 + 1 * 8, 1 + 3 * 8, 5 + 0 * 8,
    1 + 4 * 8, 2 + 4 * 8, 6 + 0 * 8, 4 + 3 * 8,
    0 + 7 * 8, 4 + 4 * 8, 7 + 2 * 8, 3 + 6 * 8,
    5 + 5 * 8, 6 + 5 * 8, 6 + 6 * 8, 7 + 7 * 8,
};

{

    /* release non reference frames */
        }
    }

{

    // edge emu needs blocksize + filter length - 1
    // (= 21x21 for  H.264)


        av_freep(&sl->bipred_scratchpad);
        av_freep(&sl->edge_emu_buffer);
        av_freep(&sl->top_borders[0]);
        av_freep(&sl->top_borders[1]);

        sl->bipred_scratchpad_allocated = 0;
        sl->edge_emu_buffer_allocated   = 0;
        sl->top_borders_allocated[0]    = 0;
        sl->top_borders_allocated[1]    = 0;
        return AVERROR(ENOMEM);
    }

    return 0;
}

{

                                               av_buffer_allocz);
                                               sizeof(uint32_t), av_buffer_allocz);
                                               sizeof(int16_t), av_buffer_allocz);

        !h->ref_index_pool) {
        av_buffer_pool_uninit(&h->qscale_table_pool);
        av_buffer_pool_uninit(&h->mb_type_pool);
        av_buffer_pool_uninit(&h->motion_val_pool);
        av_buffer_pool_uninit(&h->ref_index_pool);
        return AVERROR(ENOMEM);
    }

    return 0;
}

{


                                                   AV_GET_BUFFER_FLAG_REF : 0);
        goto fail;

        const AVHWAccel *hwaccel = h->avctx->hwaccel;
        av_assert0(!pic->hwaccel_picture_private);
        if (hwaccel->frame_priv_data_size) {
            pic->hwaccel_priv_buf = av_buffer_allocz(hwaccel->frame_priv_data_size);
            if (!pic->hwaccel_priv_buf)
                return AVERROR(ENOMEM);
            pic->hwaccel_picture_private = pic->hwaccel_priv_buf->data;
        }
    }
        int h_chroma_shift, v_chroma_shift;
        av_pix_fmt_get_chroma_sub_sample(pic->f->format,
                                         &h_chroma_shift, &v_chroma_shift);

        for(i=0; i<AV_CEIL_RSHIFT(pic->f->height, v_chroma_shift); i++) {
            memset(pic->f->data[1] + pic->f->linesize[1]*i,
                   0x80, AV_CEIL_RSHIFT(pic->f->width, h_chroma_shift));
            memset(pic->f->data[2] + pic->f->linesize[2]*i,
                   0x80, AV_CEIL_RSHIFT(pic->f->width, h_chroma_shift));
        }
    }

            goto fail;
    }

        goto fail;


            goto fail;

    }

        goto fail;


fail:
    ff_h264_unref_picture(h, pic);
    return (ret < 0) ? ret : AVERROR(ENOMEM);
}

{

            return i;
    }
    return AVERROR_INVALIDDATA;
}


#define IN_RANGE(a, b, size) (((void*)(a) >= (void*)(b)) && ((void*)(a) < (void*)((b) + (size))))

#define REBASE_PICTURE(pic, new_ctx, old_ctx)             \
    (((pic) && (pic) >= (old_ctx)->DPB &&                       \
      (pic) < (old_ctx)->DPB + H264_MAX_PICTURE_COUNT) ?          \
     &(new_ctx)->DPB[(pic) - (old_ctx)->DPB] : NULL)

static void copy_picture_range(H264Picture **to, H264Picture **from, int count,
                               H264Context *new_base,
                               H264Context *old_base)
{
    int i;

    for (i = 0; i < count; i++) {
        av_assert1(!from[i] ||
                   IN_RANGE(from[i], old_base, 1) ||
                   IN_RANGE(from[i], old_base->DPB, H264_MAX_PICTURE_COUNT));
        to[i] = REBASE_PICTURE(from[i], new_base, old_base);
    }
}

static int h264_slice_header_init(H264Context *h);

int ff_h264_update_thread_context(AVCodecContext *dst,
                                  const AVCodecContext *src)
{
    H264Context *h = dst->priv_data, *h1 = src->priv_data;
    int inited = h->context_initialized, err = 0;
    int need_reinit = 0;
    int i, ret;

    if (dst == src)
        return 0;

    // We can't fail if SPS isn't set at it breaks current skip_frame code
    //if (!h1->ps.sps)
    //    return AVERROR_INVALIDDATA;

    if (inited &&
        (h->width                 != h1->width                 ||
         h->height                != h1->height                ||
         h->mb_width              != h1->mb_width              ||
         h->mb_height             != h1->mb_height             ||
         !h->ps.sps                                            ||
         h->ps.sps->bit_depth_luma    != h1->ps.sps->bit_depth_luma    ||
         h->ps.sps->chroma_format_idc != h1->ps.sps->chroma_format_idc ||
         h->ps.sps->colorspace        != h1->ps.sps->colorspace)) {
        need_reinit = 1;
    }

    /* copy block_offset since frame_start may not be called */
    memcpy(h->block_offset, h1->block_offset, sizeof(h->block_offset));

    // SPS/PPS
    for (i = 0; i < FF_ARRAY_ELEMS(h->ps.sps_list); i++) {
        av_buffer_unref(&h->ps.sps_list[i]);
        if (h1->ps.sps_list[i]) {
            h->ps.sps_list[i] = av_buffer_ref(h1->ps.sps_list[i]);
            if (!h->ps.sps_list[i])
                return AVERROR(ENOMEM);
        }
    }
    for (i = 0; i < FF_ARRAY_ELEMS(h->ps.pps_list); i++) {
        av_buffer_unref(&h->ps.pps_list[i]);
        if (h1->ps.pps_list[i]) {
            h->ps.pps_list[i] = av_buffer_ref(h1->ps.pps_list[i]);
            if (!h->ps.pps_list[i])
                return AVERROR(ENOMEM);
        }
    }

    av_buffer_unref(&h->ps.pps_ref);
    h->ps.pps = NULL;
    h->ps.sps = NULL;
    if (h1->ps.pps_ref) {
        h->ps.pps_ref = av_buffer_ref(h1->ps.pps_ref);
        if (!h->ps.pps_ref)
            return AVERROR(ENOMEM);
        h->ps.pps = (const PPS*)h->ps.pps_ref->data;
        h->ps.sps = h->ps.pps->sps;
    }

    if (need_reinit || !inited) {
        h->width     = h1->width;
        h->height    = h1->height;
        h->mb_height = h1->mb_height;
        h->mb_width  = h1->mb_width;
        h->mb_num    = h1->mb_num;
        h->mb_stride = h1->mb_stride;
        h->b_stride  = h1->b_stride;
        h->x264_build = h1->x264_build;

        if (h->context_initialized || h1->context_initialized) {
            if ((err = h264_slice_header_init(h)) < 0) {
                av_log(h->avctx, AV_LOG_ERROR, "h264_slice_header_init() failed");
                return err;
            }
        }

        /* copy block_offset since frame_start may not be called */
        memcpy(h->block_offset, h1->block_offset, sizeof(h->block_offset));
    }

    h->avctx->coded_height  = h1->avctx->coded_height;
    h->avctx->coded_width   = h1->avctx->coded_width;
    h->avctx->width         = h1->avctx->width;
    h->avctx->height        = h1->avctx->height;
    h->width_from_caller    = h1->width_from_caller;
    h->height_from_caller   = h1->height_from_caller;
    h->coded_picture_number = h1->coded_picture_number;
    h->first_field          = h1->first_field;
    h->picture_structure    = h1->picture_structure;
    h->mb_aff_frame         = h1->mb_aff_frame;
    h->droppable            = h1->droppable;

    for (i = 0; i < H264_MAX_PICTURE_COUNT; i++) {
        ff_h264_unref_picture(h, &h->DPB[i]);
        if (h1->DPB[i].f->buf[0] &&
            (ret = ff_h264_ref_picture(h, &h->DPB[i], &h1->DPB[i])) < 0)
            return ret;
    }

    h->cur_pic_ptr = REBASE_PICTURE(h1->cur_pic_ptr, h, h1);
    ff_h264_unref_picture(h, &h->cur_pic);
    if (h1->cur_pic.f->buf[0]) {
        ret = ff_h264_ref_picture(h, &h->cur_pic, &h1->cur_pic);
        if (ret < 0)
            return ret;
    }

    h->enable_er       = h1->enable_er;
    h->workaround_bugs = h1->workaround_bugs;
    h->droppable       = h1->droppable;

    // extradata/NAL handling
    h->is_avc = h1->is_avc;
    h->nal_length_size = h1->nal_length_size;

    memcpy(&h->poc,        &h1->poc,        sizeof(h->poc));

    memcpy(h->short_ref,   h1->short_ref,   sizeof(h->short_ref));
    memcpy(h->long_ref,    h1->long_ref,    sizeof(h->long_ref));
    memcpy(h->delayed_pic, h1->delayed_pic, sizeof(h->delayed_pic));
    memcpy(h->last_pocs,   h1->last_pocs,   sizeof(h->last_pocs));

    h->next_output_pic   = h1->next_output_pic;
    h->next_outputed_poc = h1->next_outputed_poc;

    memcpy(h->mmco, h1->mmco, sizeof(h->mmco));
    h->nb_mmco         = h1->nb_mmco;
    h->mmco_reset      = h1->mmco_reset;
    h->explicit_ref_marking = h1->explicit_ref_marking;
    h->long_ref_count  = h1->long_ref_count;
    h->short_ref_count = h1->short_ref_count;

    copy_picture_range(h->short_ref, h1->short_ref, 32, h, h1);
    copy_picture_range(h->long_ref, h1->long_ref, 32, h, h1);
    copy_picture_range(h->delayed_pic, h1->delayed_pic,
                       MAX_DELAYED_PIC_COUNT + 2, h, h1);

    h->frame_recovered       = h1->frame_recovered;

    av_buffer_unref(&h->sei.a53_caption.buf_ref);
    if (h1->sei.a53_caption.buf_ref) {
        h->sei.a53_caption.buf_ref = av_buffer_ref(h1->sei.a53_caption.buf_ref);
        if (!h->sei.a53_caption.buf_ref)
            return AVERROR(ENOMEM);
    }

    if (!h->cur_pic_ptr)
        return 0;

    if (!h->droppable) {
        err = ff_h264_execute_ref_pic_marking(h);
        h->poc.prev_poc_msb = h->poc.poc_msb;
        h->poc.prev_poc_lsb = h->poc.poc_lsb;
    }
    h->poc.prev_frame_num_offset = h->poc.frame_num_offset;
    h->poc.prev_frame_num        = h->poc.frame_num;

    h->recovery_frame        = h1->recovery_frame;

    return err;
}

{

        av_log(h->avctx, AV_LOG_ERROR, "Attempt to start a frame outside SETUP state\n");
        return -1;
    }


        av_log(h->avctx, AV_LOG_ERROR, "no frame buffer available\n");
        return i;
    }

    /*
     * Zero key_frame here; IDR markings per slice in frame or fields are ORed
     * in later.
     * See decode_nal_units().
     */



        return ret;

    }

        return ret;

    }

    }

    }
    }

    /* We mark the current picture as non-reference after allocating it, so
     * that if we break out due to an error it can be released automatically
     * in the next ff_mpv_frame_start().
     */






    assert(h->cur_pic_ptr->long_ref == 0);

    return 0;
}

                                              uint8_t *src_y,
                                              uint8_t *src_cb, uint8_t *src_cr,
                                              int linesize, int uvlinesize,
                                              int simple)
{


                        if (pixel_shift) {
                            AV_COPY128(top_border + 32, src_cb + 15 * uvlinesize);
                            AV_COPY128(top_border + 48, src_cb + 15 * uvlinesize + 16);
                            AV_COPY128(top_border + 64, src_cr + 15 * uvlinesize);
                            AV_COPY128(top_border + 80, src_cr + 15 * uvlinesize + 16);
                        } else {
                            AV_COPY128(top_border + 16, src_cb + 15 * uvlinesize);
                            AV_COPY128(top_border + 32, src_cr + 15 * uvlinesize);
                        }
                        } else {
                            AV_COPY64(top_border + 16, src_cb + 15 * uvlinesize);
                            AV_COPY64(top_border + 24, src_cr + 15 * uvlinesize);
                        }
                    } else {
                        } else {
                        }
                    }
                }
            }
            top_idx = 0;
        } else
            return;
    }

    /* There are two lines saved, the line above the top macroblock
     * of a pair, and the line above the bottom macroblock. */

            } else {
            }
            } else {
            }
        } else {
            } else {
            }
        }
    }
}

/**
 * Initialize implicit_weight table.
 * @param field  0/1 initialize the weight for interlaced MBAFF
 *                -1 initializes the rest
 */
{

    }

        } else {
            cur_poc = h->cur_pic_ptr->field_poc[h->picture_structure - 1];
        }
        }
    } else {
    }


                }
            }
            } else {
            }
        }
    }
}

/**
 * initialize scan tables
 */
{
#define TRANSPOSE(x) ((x) >> 2) | (((x) << 2) & 0xF)
#undef TRANSPOSE
    }
#define TRANSPOSE(x) ((x) >> 3) | (((x) & 7) << 3)
#undef TRANSPOSE
    }
    } else {
    }

static enum AVPixelFormat get_pixel_format(H264Context *h, int force_callback)
{
#define HWACCEL_MAX (CONFIG_H264_DXVA2_HWACCEL + \
                     (CONFIG_H264_D3D11VA_HWACCEL * 2) + \
                     CONFIG_H264_NVDEC_HWACCEL + \
                     CONFIG_H264_VAAPI_HWACCEL + \
                     CONFIG_H264_VIDEOTOOLBOX_HWACCEL + \
                     CONFIG_H264_VDPAU_HWACCEL)
    enum AVPixelFormat pix_fmts[HWACCEL_MAX + 2], *fmt = pix_fmts;
    const enum AVPixelFormat *choices = pix_fmts;
    int i;

    switch (h->ps.sps->bit_depth_luma) {
    case 9:
        if (CHROMA444(h)) {
            if (h->avctx->colorspace == AVCOL_SPC_RGB) {
                *fmt++ = AV_PIX_FMT_GBRP9;
            } else
                *fmt++ = AV_PIX_FMT_YUV444P9;
        } else if (CHROMA422(h))
            *fmt++ = AV_PIX_FMT_YUV422P9;
        else
            *fmt++ = AV_PIX_FMT_YUV420P9;
        break;
    case 10:
        if (CHROMA444(h)) {
            if (h->avctx->colorspace == AVCOL_SPC_RGB) {
                *fmt++ = AV_PIX_FMT_GBRP10;
            } else
                *fmt++ = AV_PIX_FMT_YUV444P10;
        } else if (CHROMA422(h))
            *fmt++ = AV_PIX_FMT_YUV422P10;
        else
            *fmt++ = AV_PIX_FMT_YUV420P10;
        break;
    case 12:
        if (CHROMA444(h)) {
            if (h->avctx->colorspace == AVCOL_SPC_RGB) {
                *fmt++ = AV_PIX_FMT_GBRP12;
            } else
                *fmt++ = AV_PIX_FMT_YUV444P12;
        } else if (CHROMA422(h))
            *fmt++ = AV_PIX_FMT_YUV422P12;
        else
            *fmt++ = AV_PIX_FMT_YUV420P12;
        break;
    case 14:
        if (CHROMA444(h)) {
            if (h->avctx->colorspace == AVCOL_SPC_RGB) {
                *fmt++ = AV_PIX_FMT_GBRP14;
            } else
                *fmt++ = AV_PIX_FMT_YUV444P14;
        } else if (CHROMA422(h))
            *fmt++ = AV_PIX_FMT_YUV422P14;
        else
            *fmt++ = AV_PIX_FMT_YUV420P14;
        break;
    case 8:
#if CONFIG_H264_VDPAU_HWACCEL
        *fmt++ = AV_PIX_FMT_VDPAU;
#endif
#if CONFIG_H264_NVDEC_HWACCEL
        *fmt++ = AV_PIX_FMT_CUDA;
#endif
        if (CHROMA444(h)) {
            if (h->avctx->colorspace == AVCOL_SPC_RGB)
                *fmt++ = AV_PIX_FMT_GBRP;
            else if (h->avctx->color_range == AVCOL_RANGE_JPEG)
                *fmt++ = AV_PIX_FMT_YUVJ444P;
            else
                *fmt++ = AV_PIX_FMT_YUV444P;
        } else if (CHROMA422(h)) {
            if (h->avctx->color_range == AVCOL_RANGE_JPEG)
                *fmt++ = AV_PIX_FMT_YUVJ422P;
            else
                *fmt++ = AV_PIX_FMT_YUV422P;
        } else {
#if CONFIG_H264_DXVA2_HWACCEL
            *fmt++ = AV_PIX_FMT_DXVA2_VLD;
#endif
#if CONFIG_H264_D3D11VA_HWACCEL
            *fmt++ = AV_PIX_FMT_D3D11VA_VLD;
            *fmt++ = AV_PIX_FMT_D3D11;
#endif
#if CONFIG_H264_VAAPI_HWACCEL
            *fmt++ = AV_PIX_FMT_VAAPI;
#endif
#if CONFIG_H264_VIDEOTOOLBOX_HWACCEL
            *fmt++ = AV_PIX_FMT_VIDEOTOOLBOX;
#endif
            if (h->avctx->codec->pix_fmts)
                choices = h->avctx->codec->pix_fmts;
            else if (h->avctx->color_range == AVCOL_RANGE_JPEG)
                *fmt++ = AV_PIX_FMT_YUVJ420P;
            else
                *fmt++ = AV_PIX_FMT_YUV420P;
        }
        break;
    default:
        av_log(h->avctx, AV_LOG_ERROR,
               "Unsupported bit depth %d\n", h->ps.sps->bit_depth_luma);
        return AVERROR_INVALIDDATA;
    }

    *fmt = AV_PIX_FMT_NONE;

    for (i=0; choices[i] != AV_PIX_FMT_NONE; i++)
        if (choices[i] == h->avctx->pix_fmt && !force_callback)
            return choices[i];
    return ff_thread_get_format(h->avctx, choices);
}

/* export coded and cropped frame dimensions to AVCodecContext */
{

    /* handle container cropping */
        h->height_from_caller <= height) {
    } else {
    }


{

                                     &h->chroma_x_shift, &h->chroma_y_shift);

            den *= 2;
    }



        av_log(h->avctx, AV_LOG_ERROR, "Could not allocate memory\n");
        goto fail;
    }

    ) {
        av_log(h->avctx, AV_LOG_ERROR, "Unsupported bit depth %d\n",
               sps->bit_depth_luma);
        ret = AVERROR_INVALIDDATA;
        goto fail;
    }


                    sps->chroma_format_idc);
                      sps->chroma_format_idc);

            av_log(h->avctx, AV_LOG_ERROR, "context_init() failed.\n");
            goto fail;
        }
    } else {


                av_log(h->avctx, AV_LOG_ERROR, "context_init() failed.\n");
                goto fail;
            }
        }
    }


fail:
    ff_h264_free_tables(h);
    h->context_initialized = 0;
    return ret;
}

{
    case AV_PIX_FMT_YUVJ420P: return AV_PIX_FMT_YUV420P;
    case AV_PIX_FMT_YUVJ422P: return AV_PIX_FMT_YUV422P;
    case AV_PIX_FMT_YUVJ444P: return AV_PIX_FMT_YUV444P;
    default:
        return a;
    }
}

{

            return AVERROR(ENOMEM);
    }


        )

    }

                    ));
        must_reinit = 1;








            }
        }

            av_color_transfer_name(h->sei.alternative_transfer.preferred_transfer_characteristics) &&
            h->sei.alternative_transfer.preferred_transfer_characteristics != AVCOL_TRC_UNSPECIFIED) {
            h->avctx->color_trc = h->sei.alternative_transfer.preferred_transfer_characteristics;
        }
    }

            av_log(h->avctx, AV_LOG_ERROR,
                   "changing width %d -> %d / height %d -> %d on "
                   "slice %d\n",
                   h->width, h->avctx->coded_width,
                   h->height, h->avctx->coded_height,
                   h->current_slice + 1);
            return AVERROR_INVALIDDATA;
        }



            return ret;

               "pix_fmt: %s\n", h->width, h->height, av_get_pix_fmt_name(h->avctx->pix_fmt));

            av_log(h->avctx, AV_LOG_ERROR,
                   "h264_slice_header_init() failed\n");
            return ret;
        }
    }

    return 0;
}

{


    /* Signal interlacing information externally. */
    /* Prioritize picture timing SEI information over used
     * decoding process if it exists. */
            av_log(h->avctx, AV_LOG_ERROR, "Error processing a picture timing SEI\n");
            if (h->avctx->err_recognition & AV_EF_EXPLODE)
                return ret;
            h->sei.picture_timing.present = 0;
        }
    }

        case H264_SEI_PIC_STRUCT_FRAME:
            break;
        case H264_SEI_PIC_STRUCT_BOTTOM_FIELD:
        case H264_SEI_PIC_STRUCT_BOTTOM_TOP:
            else
                // try to flag soft telecine progressive
            break;
        case H264_SEI_PIC_STRUCT_BOTTOM_TOP_BOTTOM:
            /* Signal the possibility of telecined film externally
             * (pic_struct 5,6). From these hints, let the applications
             * decide if they apply deinterlacing. */
        case H264_SEI_PIC_STRUCT_FRAME_DOUBLING:
            out->repeat_pict = 2;
            break;
        case H264_SEI_PIC_STRUCT_FRAME_TRIPLING:
            out->repeat_pict = 4;
            break;
        }

            pt->pic_struct <= H264_SEI_PIC_STRUCT_BOTTOM_TOP)
    } else {
        /* Derive interlacing flag from used decoding process. */
    }

        /* Derive top_field_first from field pocs. */
    } else {
            /* Use picture timing SEI information. Even if it is a
             * information of a past frame, better than nothing. */
                h->sei.picture_timing.pic_struct == H264_SEI_PIC_STRUCT_TOP_BOTTOM_TOP)
            else
            /* Default to top field first when pic_struct_present_flag
             * is not set but interlaced frame detected */
        } else {
            /* Most likely progressive */
        }
    }

        h->sei.frame_packing.arrangement_type <= 6 &&
        h->sei.frame_packing.content_interpretation_type > 0 &&
        h->sei.frame_packing.content_interpretation_type < 3) {
        H264SEIFramePacking *fp = &h->sei.frame_packing;
        AVStereo3D *stereo = av_stereo3d_create_side_data(out);
        if (stereo) {
        switch (fp->arrangement_type) {
        case H264_SEI_FPA_TYPE_CHECKERBOARD:
            stereo->type = AV_STEREO3D_CHECKERBOARD;
            break;
        case H264_SEI_FPA_TYPE_INTERLEAVE_COLUMN:
            stereo->type = AV_STEREO3D_COLUMNS;
            break;
        case H264_SEI_FPA_TYPE_INTERLEAVE_ROW:
            stereo->type = AV_STEREO3D_LINES;
            break;
        case H264_SEI_FPA_TYPE_SIDE_BY_SIDE:
            if (fp->quincunx_sampling_flag)
                stereo->type = AV_STEREO3D_SIDEBYSIDE_QUINCUNX;
            else
                stereo->type = AV_STEREO3D_SIDEBYSIDE;
            break;
        case H264_SEI_FPA_TYPE_TOP_BOTTOM:
            stereo->type = AV_STEREO3D_TOPBOTTOM;
            break;
        case H264_SEI_FPA_TYPE_INTERLEAVE_TEMPORAL:
            stereo->type = AV_STEREO3D_FRAMESEQUENCE;
            break;
        case H264_SEI_FPA_TYPE_2D:
            stereo->type = AV_STEREO3D_2D;
            break;
        }

        if (fp->content_interpretation_type == 2)
            stereo->flags = AV_STEREO3D_FLAG_INVERT;

        if (fp->arrangement_type == H264_SEI_FPA_TYPE_INTERLEAVE_TEMPORAL) {
            if (fp->current_frame_is_frame0_flag)
                stereo->view = AV_STEREO3D_VIEW_LEFT;
            else
                stereo->view = AV_STEREO3D_VIEW_RIGHT;
        }
        }
    }

         h->sei.display_orientation.vflip)) {
        H264SEIDisplayOrientation *o = &h->sei.display_orientation;
        double angle = o->anticlockwise_rotation * 360 / (double) (1 << 16);
        AVFrameSideData *rotation = av_frame_new_side_data(out,
                                                           AV_FRAME_DATA_DISPLAYMATRIX,
                                                           sizeof(int32_t) * 9);
        if (rotation) {
            av_display_rotation_set((int32_t *)rotation->data, angle);
            av_display_matrix_flip((int32_t *)rotation->data,
                                   o->hflip, o->vflip);
        }
    }

                                                     sizeof(uint8_t));

        }
    }


            av_buffer_unref(&a53->buf_ref);

    }


                    AV_FRAME_DATA_SEI_UNREGISTERED,
                    unreg->buf_ref[i]);
                av_buffer_unref(&unreg->buf_ref[i]);
        }
    }


                                                         AV_FRAME_DATA_S12M_TIMECODE,
                                                         sizeof(uint32_t)*4);
            return AVERROR(ENOMEM);



        }
    }

    return 0;
}

{


    }

        }
    }
        av_log(h->avctx, AV_LOG_VERBOSE, "Invalid POC %d<%d\n", cur->poc, h->last_pocs[0]);
        for (i = 1; i < MAX_DELAYED_PIC_COUNT; i++)
            h->last_pocs[i] = INT_MIN;
        h->last_pocs[0] = cur->poc;
        cur->mmco_reset = 1;
    }




        }

    }
        } else

            // We have reached an recovery point and all frames after it in
            // display order are "recovered".
        }

            } else {
                out->f->flags |= AV_FRAME_FLAG_CORRUPT;
            }
        }
    } else {
    }

}

/* This function is called right after decoding the slice header for a first
 * slice in a field (or a frame). It decides whether we are decoding a new frame
 * or a second field in a pair and does the necessary setup.
 */
static int h264_field_start(H264Context *h, const H264SliceContext *sl,
                            const H2645NAL *nal, int first_slice)
{
    int i;
    const SPS *sps;

    int last_pic_structure, last_pic_droppable, ret;

    ret = h264_init_ps(h, sl, first_slice);
    if (ret < 0)
        return ret;

    sps = h->ps.sps;

    if (sps && sps->bitstream_restriction_flag &&
        h->avctx->has_b_frames < sps->num_reorder_frames) {
        h->avctx->has_b_frames = sps->num_reorder_frames;
    }

    last_pic_droppable   = h->droppable;
    last_pic_structure   = h->picture_structure;
    h->droppable         = (nal->ref_idc == 0);
    h->picture_structure = sl->picture_structure;

    h->poc.frame_num        = sl->frame_num;
    h->poc.poc_lsb          = sl->poc_lsb;
    h->poc.delta_poc_bottom = sl->delta_poc_bottom;
    h->poc.delta_poc[0]     = sl->delta_poc[0];
    h->poc.delta_poc[1]     = sl->delta_poc[1];

    /* Shorten frame num gaps so we don't have to allocate reference
     * frames just to throw them away */
    if (h->poc.frame_num != h->poc.prev_frame_num) {
        int unwrap_prev_frame_num = h->poc.prev_frame_num;
        int max_frame_num         = 1 << sps->log2_max_frame_num;

        if (unwrap_prev_frame_num > h->poc.frame_num)
            unwrap_prev_frame_num -= max_frame_num;

        if ((h->poc.frame_num - unwrap_prev_frame_num) > sps->ref_frame_count) {
            unwrap_prev_frame_num = (h->poc.frame_num - sps->ref_frame_count) - 1;
            if (unwrap_prev_frame_num < 0)
                unwrap_prev_frame_num += max_frame_num;

            h->poc.prev_frame_num = unwrap_prev_frame_num;
        }
    }

    /* See if we have a decoded first field looking for a pair...
     * Here, we're using that to see if we should mark previously
     * decode frames as "finished".
     * We have to do that before the "dummy" in-between frame allocation,
     * since that can modify h->cur_pic_ptr. */
    if (h->first_field) {
        int last_field = last_pic_structure == PICT_BOTTOM_FIELD;
        av_assert0(h->cur_pic_ptr);
        av_assert0(h->cur_pic_ptr->f->buf[0]);
        assert(h->cur_pic_ptr->reference != DELAYED_PIC_REF);

        /* Mark old field/frame as completed */
        if (h->cur_pic_ptr->tf.owner[last_field] == h->avctx) {
            ff_thread_report_progress(&h->cur_pic_ptr->tf, INT_MAX, last_field);
        }

        /* figure out if we have a complementary field pair */
        if (!FIELD_PICTURE(h) || h->picture_structure == last_pic_structure) {
            /* Previous field is unmatched. Don't display it, but let it
             * remain for reference if marked as such. */
            if (last_pic_structure != PICT_FRAME) {
                ff_thread_report_progress(&h->cur_pic_ptr->tf, INT_MAX,
                                          last_pic_structure == PICT_TOP_FIELD);
            }
        } else {
            if (h->cur_pic_ptr->frame_num != h->poc.frame_num) {
                /* This and previous field were reference, but had
                 * different frame_nums. Consider this field first in
                 * pair. Throw away previous field except for reference
                 * purposes. */
                if (last_pic_structure != PICT_FRAME) {
                    ff_thread_report_progress(&h->cur_pic_ptr->tf, INT_MAX,
                                              last_pic_structure == PICT_TOP_FIELD);
                }
            } else {
                /* Second field in complementary pair */
                if (!((last_pic_structure   == PICT_TOP_FIELD &&
                       h->picture_structure == PICT_BOTTOM_FIELD) ||
                      (last_pic_structure   == PICT_BOTTOM_FIELD &&
                       h->picture_structure == PICT_TOP_FIELD))) {
                    av_log(h->avctx, AV_LOG_ERROR,
                           "Invalid field mode combination %d/%d\n",
                           last_pic_structure, h->picture_structure);
                    h->picture_structure = last_pic_structure;
                    h->droppable         = last_pic_droppable;
                    return AVERROR_INVALIDDATA;
                } else if (last_pic_droppable != h->droppable) {
                    avpriv_request_sample(h->avctx,
                                          "Found reference and non-reference fields in the same frame, which");
                    h->picture_structure = last_pic_structure;
                    h->droppable         = last_pic_droppable;
                    return AVERROR_PATCHWELCOME;
                }
            }
        }
    }

    while (h->poc.frame_num != h->poc.prev_frame_num && !h->first_field &&
           h->poc.frame_num != (h->poc.prev_frame_num + 1) % (1 << sps->log2_max_frame_num)) {
        H264Picture *prev = h->short_ref_count ? h->short_ref[0] : NULL;
        av_log(h->avctx, AV_LOG_DEBUG, "Frame num gap %d %d\n",
               h->poc.frame_num, h->poc.prev_frame_num);
        if (!sps->gaps_in_frame_num_allowed_flag)
            for(i=0; i<FF_ARRAY_ELEMS(h->last_pocs); i++)
                h->last_pocs[i] = INT_MIN;
        ret = h264_frame_start(h);
        if (ret < 0) {
            h->first_field = 0;
            return ret;
        }

        h->poc.prev_frame_num++;
        h->poc.prev_frame_num        %= 1 << sps->log2_max_frame_num;
        h->cur_pic_ptr->frame_num = h->poc.prev_frame_num;
        h->cur_pic_ptr->invalid_gap = !sps->gaps_in_frame_num_allowed_flag;
        ff_thread_report_progress(&h->cur_pic_ptr->tf, INT_MAX, 0);
        ff_thread_report_progress(&h->cur_pic_ptr->tf, INT_MAX, 1);

        h->explicit_ref_marking = 0;
        ret = ff_h264_execute_ref_pic_marking(h);
        if (ret < 0 && (h->avctx->err_recognition & AV_EF_EXPLODE))
            return ret;
        /* Error concealment: If a ref is missing, copy the previous ref
         * in its place.
         * FIXME: Avoiding a memcpy would be nice, but ref handling makes
         * many assumptions about there being no actual duplicates.
         * FIXME: This does not copy padding for out-of-frame motion
         * vectors.  Given we are concealing a lost frame, this probably
         * is not noticeable by comparison, but it should be fixed. */
        if (h->short_ref_count) {
            int c[4] = {
                1<<(h->ps.sps->bit_depth_luma-1),
                1<<(h->ps.sps->bit_depth_chroma-1),
                1<<(h->ps.sps->bit_depth_chroma-1),
                -1
            };

            if (prev &&
                h->short_ref[0]->f->width == prev->f->width &&
                h->short_ref[0]->f->height == prev->f->height &&
                h->short_ref[0]->f->format == prev->f->format) {
                ff_thread_await_progress(&prev->tf, INT_MAX, 0);
                if (prev->field_picture)
                    ff_thread_await_progress(&prev->tf, INT_MAX, 1);
                av_image_copy(h->short_ref[0]->f->data,
                              h->short_ref[0]->f->linesize,
                              (const uint8_t **)prev->f->data,
                              prev->f->linesize,
                              prev->f->format,
                              prev->f->width,
                              prev->f->height);
                h->short_ref[0]->poc = prev->poc + 2;
            } else if (!h->frame_recovered && !h->avctx->hwaccel)
                ff_color_frame(h->short_ref[0]->f, c);
            h->short_ref[0]->frame_num = h->poc.prev_frame_num;
        }
    }

    /* See if we have a decoded first field looking for a pair...
     * We're using that to see whether to continue decoding in that
     * frame, or to allocate a new one. */
    if (h->first_field) {
        av_assert0(h->cur_pic_ptr);
        av_assert0(h->cur_pic_ptr->f->buf[0]);
        assert(h->cur_pic_ptr->reference != DELAYED_PIC_REF);

        /* figure out if we have a complementary field pair */
        if (!FIELD_PICTURE(h) || h->picture_structure == last_pic_structure) {
            /* Previous field is unmatched. Don't display it, but let it
             * remain for reference if marked as such. */
            h->missing_fields ++;
            h->cur_pic_ptr = NULL;
            h->first_field = FIELD_PICTURE(h);
        } else {
            h->missing_fields = 0;
            if (h->cur_pic_ptr->frame_num != h->poc.frame_num) {
                ff_thread_report_progress(&h->cur_pic_ptr->tf, INT_MAX,
                                          h->picture_structure==PICT_BOTTOM_FIELD);
                /* This and the previous field had different frame_nums.
                 * Consider this field first in pair. Throw away previous
                 * one except for reference purposes. */
                h->first_field = 1;
                h->cur_pic_ptr = NULL;
            } else if (h->cur_pic_ptr->reference & DELAYED_PIC_REF) {
                /* This frame was already output, we cannot draw into it
                 * anymore.
                 */
                h->first_field = 1;
                h->cur_pic_ptr = NULL;
            } else {
                /* Second field in complementary pair */
                h->first_field = 0;
            }
        }
    } else {
        /* Frame or first field in a potentially complementary pair */
        h->first_field = FIELD_PICTURE(h);
    }

    if (!FIELD_PICTURE(h) || h->first_field) {
        if (h264_frame_start(h) < 0) {
            h->first_field = 0;
            return AVERROR_INVALIDDATA;
        }
    } else {
        int field = h->picture_structure == PICT_BOTTOM_FIELD;
        release_unused_pictures(h, 0);
        h->cur_pic_ptr->tf.owner[field] = h->avctx;
    }
    /* Some macroblocks can be accessed before they're available in case
    * of lost slices, MBAFF or threading. */
    if (FIELD_PICTURE(h)) {
        for(i = (h->picture_structure == PICT_BOTTOM_FIELD); i<h->mb_height; i++)
            memset(h->slice_table + i*h->mb_stride, -1, (h->mb_stride - (i+1==h->mb_height)) * sizeof(*h->slice_table));
    } else {
        memset(h->slice_table, -1,
            (h->mb_height * h->mb_stride - 1) * sizeof(*h->slice_table));
    }

    ret = ff_h264_init_poc(h->cur_pic_ptr->field_poc, &h->cur_pic_ptr->poc,
                     h->ps.sps, &h->poc, h->picture_structure, nal->ref_idc);
    if (ret < 0)
        return ret;

    memcpy(h->mmco, sl->mmco, sl->nb_mmco * sizeof(*h->mmco));
    h->nb_mmco = sl->nb_mmco;
    h->explicit_ref_marking = sl->explicit_ref_marking;

    h->picture_idr = nal->type == H264_NAL_IDR_SLICE;

    if (h->sei.recovery_point.recovery_frame_cnt >= 0) {
        const int sei_recovery_frame_cnt = h->sei.recovery_point.recovery_frame_cnt;

        if (h->poc.frame_num != sei_recovery_frame_cnt || sl->slice_type_nos != AV_PICTURE_TYPE_I)
            h->valid_recovery_point = 1;

        if (   h->recovery_frame < 0
            || av_mod_uintp2(h->recovery_frame - h->poc.frame_num, h->ps.sps->log2_max_frame_num) > sei_recovery_frame_cnt) {
            h->recovery_frame = av_mod_uintp2(h->poc.frame_num + sei_recovery_frame_cnt, h->ps.sps->log2_max_frame_num);

            if (!h->valid_recovery_point)
                h->recovery_frame = h->poc.frame_num;
        }
    }

    h->cur_pic_ptr->f->key_frame |= (nal->type == H264_NAL_IDR_SLICE);

    if (nal->type == H264_NAL_IDR_SLICE ||
        (h->recovery_frame == h->poc.frame_num && nal->ref_idc)) {
        h->recovery_frame         = -1;
        h->cur_pic_ptr->recovered = 1;
    }
    // If we have an IDR, all frames after it in decoded order are
    // "recovered".
    if (nal->type == H264_NAL_IDR_SLICE)
        h->frame_recovered |= FRAME_RECOVERED_IDR;
#if 1
    h->cur_pic_ptr->recovered |= h->frame_recovered;
#else
    h->cur_pic_ptr->recovered |= !!(h->frame_recovered & FRAME_RECOVERED_IDR);
#endif

    /* Set the frame properties/side data. Only done for the second field in
     * field coded frames, since some SEI information is present for each field
     * and is merged by the SEI parsing code. */
    if (!FIELD_PICTURE(h) || !h->first_field || h->missing_fields > 1) {
        ret = h264_export_frame_props(h);
        if (ret < 0)
            return ret;

        ret = h264_select_output_frame(h);
        if (ret < 0)
            return ret;
    }

    return 0;
}

                                   const H2645NAL *nal)
{



        av_log(h->avctx, AV_LOG_ERROR,
               "slice type %d too large at %d\n",
               slice_type, sl->first_mb_addr);
        return AVERROR_INVALIDDATA;
    }
    } else


        sl->slice_type_nos != AV_PICTURE_TYPE_I) {
        av_log(h->avctx, AV_LOG_ERROR, "A non-intra slice in an IDR NAL unit.\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(h->avctx, AV_LOG_ERROR, "pps_id %u out of range\n", sl->pps_id);
        return AVERROR_INVALIDDATA;
    }
               "non-existing PPS %u referenced\n",
               sl->pps_id);
    }

            av_log(h->avctx, AV_LOG_ERROR, "Frame num change from %d to %d\n",
                   h->poc.frame_num, sl->frame_num);
            return AVERROR_INVALIDDATA;
        }
    }


        picture_structure = PICT_FRAME;
    } else {
            av_log(h->avctx, AV_LOG_ERROR, "This stream was generated by a broken encoder, invalid 8x8 inference\n");
            return -1;
        }
        } else {
            picture_structure = PICT_FRAME;
        }
    }

    } else {
    }



    }


    }

        sl->redundant_pic_count = get_ue_golomb(&sl->gb);


                                  &sl->gb, pps, sl->slice_type_nos,
        return ret;

           sl->ref_count[1] = sl->ref_count[0] = 0;
           return ret;
       }
    }

    }
                                  sl->slice_type_nos, &sl->pwt,
            return ret;
    }

            return AVERROR_INVALIDDATA;
    }

            av_log(h->avctx, AV_LOG_ERROR, "cabac_init_idc %u overflow\n", tmp);
            return AVERROR_INVALIDDATA;
        }
    }

        av_log(h->avctx, AV_LOG_ERROR, "QP %u out of range\n", tmp);
        return AVERROR_INVALIDDATA;
    }
    // FIXME qscale / qp ... stuff
        get_bits1(&sl->gb); /* sp_for_switch_flag */
        sl->slice_type == AV_PICTURE_TYPE_SI)
        get_se_golomb(&sl->gb); /* slice_qs_delta */

            av_log(h->avctx, AV_LOG_ERROR,
                   "deblocking_filter_idc %u out of range\n", tmp);
            return AVERROR_INVALIDDATA;
        }

                slice_alpha_c0_offset_div2 < -6 ||
                slice_beta_offset_div2 < -6) {
                av_log(h->avctx, AV_LOG_ERROR,
                       "deblocking filter parameters %d %d out of range\n",
                       slice_alpha_c0_offset_div2, slice_beta_offset_div2);
                return AVERROR_INVALIDDATA;
            }
        }
    }

    return 0;
}

/* do all the per-slice initialization needed before we can start decoding the
 * actual MBs */
static int h264_slice_init(H264Context *h, H264SliceContext *sl,
                           const H2645NAL *nal)
{
    int i, j, ret = 0;

    if (h->picture_idr && nal->type != H264_NAL_IDR_SLICE) {
        av_log(h->avctx, AV_LOG_ERROR, "Invalid mix of IDR and non-IDR slices\n");
        return AVERROR_INVALIDDATA;
    }

    av_assert1(h->mb_num == h->mb_width * h->mb_height);
    if (sl->first_mb_addr << FIELD_OR_MBAFF_PICTURE(h) >= h->mb_num ||
        sl->first_mb_addr >= h->mb_num) {
        av_log(h->avctx, AV_LOG_ERROR, "first_mb_in_slice overflow\n");
        return AVERROR_INVALIDDATA;
    }
    sl->resync_mb_x = sl->mb_x =  sl->first_mb_addr % h->mb_width;
    sl->resync_mb_y = sl->mb_y = (sl->first_mb_addr / h->mb_width) <<
                                 FIELD_OR_MBAFF_PICTURE(h);
    if (h->picture_structure == PICT_BOTTOM_FIELD)
        sl->resync_mb_y = sl->mb_y = sl->mb_y + 1;
    av_assert1(sl->mb_y < h->mb_height);

    ret = ff_h264_build_ref_list(h, sl);
    if (ret < 0)
        return ret;

    if (h->ps.pps->weighted_bipred_idc == 2 &&
        sl->slice_type_nos == AV_PICTURE_TYPE_B) {
        implicit_weight_table(h, sl, -1);
        if (FRAME_MBAFF(h)) {
            implicit_weight_table(h, sl, 0);
            implicit_weight_table(h, sl, 1);
        }
    }

    if (sl->slice_type_nos == AV_PICTURE_TYPE_B && !sl->direct_spatial_mv_pred)
        ff_h264_direct_dist_scale_factor(h, sl);
    if (!h->setup_finished)
        ff_h264_direct_ref_list_init(h, sl);

    if (h->avctx->skip_loop_filter >= AVDISCARD_ALL ||
        (h->avctx->skip_loop_filter >= AVDISCARD_NONKEY &&
         h->nal_unit_type != H264_NAL_IDR_SLICE) ||
        (h->avctx->skip_loop_filter >= AVDISCARD_NONINTRA &&
         sl->slice_type_nos != AV_PICTURE_TYPE_I) ||
        (h->avctx->skip_loop_filter >= AVDISCARD_BIDIR  &&
         sl->slice_type_nos == AV_PICTURE_TYPE_B) ||
        (h->avctx->skip_loop_filter >= AVDISCARD_NONREF &&
         nal->ref_idc == 0))
        sl->deblocking_filter = 0;

    if (sl->deblocking_filter == 1 && h->nb_slice_ctx > 1) {
        if (h->avctx->flags2 & AV_CODEC_FLAG2_FAST) {
            /* Cheat slightly for speed:
             * Do not bother to deblock across slices. */
            sl->deblocking_filter = 2;
        } else {
            h->postpone_filter = 1;
        }
    }
    sl->qp_thresh = 15 -
                   FFMIN(sl->slice_alpha_c0_offset, sl->slice_beta_offset) -
                   FFMAX3(0,
                          h->ps.pps->chroma_qp_index_offset[0],
                          h->ps.pps->chroma_qp_index_offset[1]) +
                   6 * (h->ps.sps->bit_depth_luma - 8);

    sl->slice_num       = ++h->current_slice;

    if (sl->slice_num)
        h->slice_row[(sl->slice_num-1)&(MAX_SLICES-1)]= sl->resync_mb_y;
    if (   h->slice_row[sl->slice_num&(MAX_SLICES-1)] + 3 >= sl->resync_mb_y
        && h->slice_row[sl->slice_num&(MAX_SLICES-1)] <= sl->resync_mb_y
        && sl->slice_num >= MAX_SLICES) {
        //in case of ASO this check needs to be updated depending on how we decide to assign slice numbers in this case
        av_log(h->avctx, AV_LOG_WARNING, "Possibly too many slices (%d >= %d), increase MAX_SLICES and recompile if there are artifacts\n", sl->slice_num, MAX_SLICES);
    }

    for (j = 0; j < 2; j++) {
        int id_list[16];
        int *ref2frm = h->ref2frm[sl->slice_num & (MAX_SLICES - 1)][j];
        for (i = 0; i < 16; i++) {
            id_list[i] = 60;
            if (j < sl->list_count && i < sl->ref_count[j] &&
                sl->ref_list[j][i].parent->f->buf[0]) {
                int k;
                AVBuffer *buf = sl->ref_list[j][i].parent->f->buf[0]->buffer;
                for (k = 0; k < h->short_ref_count; k++)
                    if (h->short_ref[k]->f->buf[0]->buffer == buf) {
                        id_list[i] = k;
                        break;
                    }
                for (k = 0; k < h->long_ref_count; k++)
                    if (h->long_ref[k] && h->long_ref[k]->f->buf[0]->buffer == buf) {
                        id_list[i] = h->short_ref_count + k;
                        break;
                    }
            }
        }

        ref2frm[0] =
        ref2frm[1] = -1;
        for (i = 0; i < 16; i++)
            ref2frm[i + 2] = 4 * id_list[i] + (sl->ref_list[j][i].reference & 3);
        ref2frm[18 + 0] =
        ref2frm[18 + 1] = -1;
        for (i = 16; i < 48; i++)
            ref2frm[i + 4] = 4 * id_list[(i - 16) >> 1] +
                             (sl->ref_list[j][i].reference & 3);
    }

    if (h->avctx->debug & FF_DEBUG_PICT_INFO) {
        av_log(h->avctx, AV_LOG_DEBUG,
               "slice:%d %s mb:%d %c%s%s frame:%d poc:%d/%d ref:%d/%d qp:%d loop:%d:%d:%d weight:%d%s %s\n",
               sl->slice_num,
               (h->picture_structure == PICT_FRAME ? "F" : h->picture_structure == PICT_TOP_FIELD ? "T" : "B"),
               sl->mb_y * h->mb_width + sl->mb_x,
               av_get_picture_type_char(sl->slice_type),
               sl->slice_type_fixed ? " fix" : "",
               nal->type == H264_NAL_IDR_SLICE ? " IDR" : "",
               h->poc.frame_num,
               h->cur_pic_ptr->field_poc[0],
               h->cur_pic_ptr->field_poc[1],
               sl->ref_count[0], sl->ref_count[1],
               sl->qscale,
               sl->deblocking_filter,
               sl->slice_alpha_c0_offset, sl->slice_beta_offset,
               sl->pwt.use_weight,
               sl->pwt.use_weight == 1 && sl->pwt.use_weight_chroma ? "c" : "",
               sl->slice_type == AV_PICTURE_TYPE_B ? (sl->direct_spatial_mv_pred ? "SPAT" : "TEMP") : "");
    }

    return 0;
}

{


        return ret;

    // discard redundant pictures
        sl->ref_count[0] = sl->ref_count[1] = 0;
        return 0;
    }

            av_log(h->avctx, AV_LOG_ERROR, "Too many fields\n");
            return AVERROR_INVALIDDATA;
        }
    }

            // this slice starts a new field
            // first decode any pending queued slices
                H264SliceContext tmp_ctx;

                ret = ff_h264_execute_decode_slices(h);
                if (ret < 0 && (h->avctx->err_recognition & AV_EF_EXPLODE))
                    return ret;

                memcpy(&tmp_ctx, h->slice_ctx, sizeof(tmp_ctx));
                memcpy(h->slice_ctx, sl, sizeof(tmp_ctx));
                memcpy(sl, &tmp_ctx, sizeof(tmp_ctx));
                sl = h->slice_ctx;
            }

                    return ret;
            } else if (h->cur_pic_ptr && !FIELD_PICTURE(h) && !h->first_field && h->nal_unit_type  == H264_NAL_IDR_SLICE) {
                av_log(h, AV_LOG_WARNING, "Broken frame packetizing\n");
                ret = ff_h264_field_end(h, h->slice_ctx, 1);
                ff_thread_report_progress(&h->cur_pic_ptr->tf, INT_MAX, 0);
                ff_thread_report_progress(&h->cur_pic_ptr->tf, INT_MAX, 1);
                h->cur_pic_ptr = NULL;
                if (ret < 0)
                    return ret;
            } else
                return AVERROR_INVALIDDATA;
        }

            }
        }
    }


            h->avctx->skip_frame >= AVDISCARD_ALL) {
            return 0;
        }
    }


            (h->setup_finished && h->ps.pps != pps)*/) {
            av_log(h->avctx, AV_LOG_ERROR, "PPS changed between slices\n");
            return AVERROR_INVALIDDATA;
        }
            av_log(h->avctx, AV_LOG_ERROR,
               "SPS changed in the middle of the frame\n");
            return AVERROR_INVALIDDATA;
        }
    }

            return ret;
    } else {
            av_log(h->avctx, AV_LOG_ERROR,
                   "Changing field mode (%d -> %d) between slices is not allowed\n",
                   h->picture_structure, sl->picture_structure);
            return AVERROR_INVALIDDATA;
            av_log(h->avctx, AV_LOG_ERROR,
                   "unset cur_pic_ptr on slice %d\n",
                   h->current_slice + 1);
            return AVERROR_INVALIDDATA;
        }
    }

        return ret;


}

int ff_h264_get_slice_type(const H264SliceContext *sl)
{
    switch (sl->slice_type) {
    case AV_PICTURE_TYPE_P:
        return 0;
    case AV_PICTURE_TYPE_B:
        return 1;
    case AV_PICTURE_TYPE_I:
        return 2;
    case AV_PICTURE_TYPE_SP:
        return 3;
    case AV_PICTURE_TYPE_SI:
        return 4;
    default:
        return AVERROR_INVALIDDATA;
    }
}

                                                      H264SliceContext *sl,
                                                      int mb_type, int top_xy,
                                                      int left_xy[LEFT_MBS],
                                                      int top_type,
                                                      int left_type[LEFT_MBS],
                                                      int mb_xy, int list)
{
        } else {
        }

            } else {
            }
        }
    }

    }

    {
    }

    {
    }
}

/**
 * @return non zero if the loop filter can be skipped
 */
{


        } else {
        }
    }

    {
        /* For sufficiently low qp, filtering wouldn't do anything.
         * This is a conservative estimate: could also check beta_offset
         * and more accurate chroma_qp. */
                return 1;
                return 1;
        }
    }

    } else {
    }

        return 0;

                             top_type, left_type, mb_xy, 0);
                                 top_type, left_type, mb_xy, 1);


    }

    }

    /* CAVLC 8x8dct requires NNZ values for residual decoding that differ
     * from what the loop filter needs */
        }
        }
        }




        }
    }

    return 0;
}

{

        return;



                          (mb_x << pixel_shift) * (8 << CHROMA444(h)) +
                          mb_y * sl->uvlinesize * block_h;
                // FIXME simplify above

                    }
                } else {
                }
                                 uvlinesize, 0);

                                      linesize, uvlinesize);
                } else {
                                           dest_cr, linesize, uvlinesize);
                }
            }
    }
}

{

/**
 * Draw edges and report progress for the last MB row.
 */
{

    }

        return;

    }


        return;

}

                         int startx, int starty,
                         int endx, int endy, int status)
{
        return;


    }
}

{


        return ret;



        sl->deblocking_filter = 0;

                     (CONFIG_GRAY && (h->flags & AV_CODEC_FLAG_GRAY));

                h->slice_ctx[0].er.error_occurred = 1;
        }
    }

        /* realign */

        /* init cabac */
            return ret;


                av_log(h->avctx, AV_LOG_ERROR, "Slice overlaps with next at %d\n",
                       sl->next_slice_idx);
                er_add_slice(sl, sl->resync_mb_x, sl->resync_mb_y, sl->mb_x,
                             sl->mb_y, ER_MB_ERROR);
                return AVERROR_INVALIDDATA;
            }



            // FIXME optimal? or let mb_decode decode 16x32 ?


            }

                sl->cabac.bytestream > sl->cabac.bytestream_end + 2) {
                er_add_slice(sl, sl->resync_mb_x, sl->resync_mb_y, sl->mb_x - 1,
                             sl->mb_y, ER_MB_END);
                if (sl->mb_x >= lf_x_start)
                    loop_filter(h, sl, lf_x_start, sl->mb_x + 1);
                goto finish;
            }
                       "error while decoding MB %d %d, bytestream %"PTRDIFF_SPECIFIER"\n",
                       sl->mb_x, sl->mb_y,
                             sl->mb_y, ER_MB_ERROR);
            }

                }
            }

                        get_bits_count(&sl->gb), sl->gb.size_in_bits);
                             sl->mb_y, ER_MB_END);
            }
        }
    } else {

                av_log(h->avctx, AV_LOG_ERROR, "Slice overlaps with next at %d\n",
                       sl->next_slice_idx);
                er_add_slice(sl, sl->resync_mb_x, sl->resync_mb_y, sl->mb_x,
                             sl->mb_y, ER_MB_ERROR);
                return AVERROR_INVALIDDATA;
            }



            // FIXME optimal? or let mb_decode decode 16x32 ?

            }

                av_log(h->avctx, AV_LOG_ERROR,
                       "error while decoding MB %d %d\n", sl->mb_x, sl->mb_y);
                er_add_slice(sl, sl->resync_mb_x, sl->resync_mb_y, sl->mb_x,
                             sl->mb_y, ER_MB_ERROR);
                return ret;
            }

                }
                            get_bits_count(&sl->gb), sl->gb.size_in_bits);

                        || get_bits_left(&sl->gb) > 0 && !(h->avctx->err_recognition & AV_EF_AGGRESSIVE)) {

                    } else {
                        er_add_slice(sl, sl->resync_mb_x, sl->resync_mb_y,
                                     sl->mb_x, sl->mb_y, ER_MB_END);

                        return AVERROR_INVALIDDATA;
                    }
                }
            }

                        get_bits_count(&sl->gb), sl->gb.size_in_bits);


                } else {
                    er_add_slice(sl, sl->resync_mb_x, sl->resync_mb_y, sl->mb_x,
                                 sl->mb_y, ER_MB_ERROR);

                    return AVERROR_INVALIDDATA;
                }
            }
        }
    }

}

/**
 * Call decode_slice() for each context.
 *
 * @param h h264 master context
 */
{


        return 0;




    } else {
        av_assert0(context_count > 0);

            }

            /* make sure none of those slices overlap */

            }
        }

                       NULL, context_count, sizeof(h->slice_ctx[0]));

        /* pull back stuff from slices to master context */
        }

            h->postpone_filter = 0;

            for (i = 0; i < context_count; i++) {
                int y_end, x_end;

                sl = &h->slice_ctx[i];
                y_end = FFMIN(sl->mb_y + 1, h->mb_height);
                x_end = (sl->mb_y >= h->mb_height) ? h->mb_width : sl->mb_x;

                for (j = sl->resync_mb_y; j < y_end; j += 1 + FIELD_OR_MBAFF_PICTURE(h)) {
                    sl->mb_y = j;
                    loop_filter(h, sl, j > sl->resync_mb_y ? 0 : sl->resync_mb_x,
                                j == y_end - 1 ? x_end : h->mb_width);
                }
            }
        }
    }

}
