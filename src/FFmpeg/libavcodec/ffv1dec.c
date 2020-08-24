/*
 * FFV1 decoder
 *
 * Copyright (c) 2003-2013 Michael Niedermayer <michaelni@gmx.at>
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
 * FF Video Codec 1 (a lossless codec) decoder
 */

#include "libavutil/avassert.h"
#include "libavutil/crc.h"
#include "libavutil/opt.h"
#include "libavutil/imgutils.h"
#include "libavutil/pixdesc.h"
#include "avcodec.h"
#include "internal.h"
#include "get_bits.h"
#include "rangecoder.h"
#include "golomb.h"
#include "mathops.h"
#include "ffv1.h"

                                               int is_signed)
{
    else {
        int i, e;
        unsigned a;
        e = 0;
                return AVERROR_INVALIDDATA;
        }


    }
}

{
}

                                 int bits)
{

    }

            v, state->bias, state->error_sum, state->drift, state->count, k);




}

{
            return AVERROR_INVALIDDATA;
    } else {
            return AVERROR_INVALIDDATA;
    }
    return 0;
}

#define TYPE int16_t
#define RENAME(name) name
#include "ffv1dec_template.c"
#undef TYPE
#undef RENAME

#define TYPE int32_t
#define RENAME(name) name ## 32
#include "ffv1dec_template.c"

                         int w, int h, int stride, int plane_index,
                         int pixel_stride)
{






                return ret;
        } else {
                return ret;
                }
            } else {
                for (x = 0; x < w; x++) {
                    ((uint16_t*)(src + stride*y))[x*pixel_stride] = sample[1][x] << (16 - s->avctx->bits_per_raw_sample) | ((uint16_t **)sample)[1][x] >> (2 * s->avctx->bits_per_raw_sample - 16);
                }
            }
        }
    }
    return 0;
}

{



        return -1;
        return -1;

            av_log(f->avctx, AV_LOG_ERROR, "quant_table_index out of range\n");
            return -1;
        }

        }
    }

        f->cur->interlaced_frame = 1;
        f->cur->top_field_first  = 1;
        f->cur->interlaced_frame = 1;
        f->cur->top_field_first  = 0;
    }

        av_log(f->avctx, AV_LOG_WARNING, "ignoring invalid SAR: %u/%u\n",
               f->cur->sample_aspect_ratio.num,
               f->cur->sample_aspect_ratio.den);
        f->cur->sample_aspect_ratio = (AVRational){ 0, 1 };
    }

        fs->slice_reset_contexts = get_rac(c, state);
        fs->slice_coding_mode = get_symbol(c, state, 0);
        if (fs->slice_coding_mode != 1) {
            fs->slice_rct_by_coef = get_symbol(c, state, 0);
            fs->slice_rct_ry_coef = get_symbol(c, state, 0);
            if ((uint64_t)fs->slice_rct_by_coef + (uint64_t)fs->slice_rct_ry_coef > 4) {
                av_log(f->avctx, AV_LOG_ERROR, "slice_rct_y_coef out of range\n");
                return AVERROR_INVALIDDATA;
            }
        }
    }

    return 0;
}

{


        ff_thread_await_progress(&f->last_picture, si, 0);

        FFV1Context *fssrc = f->fsrc->slice_context[si];
        FFV1Context *fsdst = f->slice_context[si];
        av_assert1(fsdst->plane_count == fssrc->plane_count);
        av_assert1(fsdst == fs);

        if (!p->key_frame)
            fsdst->slice_damaged |= fssrc->slice_damaged;

        for (i = 0; i < f->plane_count; i++) {
            PlaneContext *psrc = &fssrc->plane[i];
            PlaneContext *pdst = &fsdst->plane[i];

            av_free(pdst->state);
            av_free(pdst->vlc_state);
            memcpy(pdst, psrc, sizeof(*pdst));
            pdst->state = NULL;
            pdst->vlc_state = NULL;

            if (fssrc->ac) {
                pdst->state = av_malloc_array(CONTEXT_SIZE,  psrc->context_count);
                memcpy(pdst->state, psrc->state, CONTEXT_SIZE * psrc->context_count);
            } else {
                pdst->vlc_state = av_malloc_array(sizeof(*pdst->vlc_state), psrc->context_count);
                memcpy(pdst->vlc_state, psrc->vlc_state, sizeof(*pdst->vlc_state) * psrc->context_count);
            }
        }
    }


            return AVERROR(ENOMEM);
            fs->slice_x = fs->slice_y = fs->slice_height = fs->slice_width = 0;
            fs->slice_damaged = 1;
            return AVERROR_INVALIDDATA;
        }
    }
        return ret;


    }


        }
            decode_plane(fs, p->data[3] + ps*x + y*p->linesize[3], width, height, p->linesize[3], (f->version >= 4 && !f->chroma_planes) ? 1 : 2, 1);
         decode_plane(fs, p->data[0] + ps*x + y*p->linesize[0]    , width, height, p->linesize[0], 0, 2);
         decode_plane(fs, p->data[0] + ps*x + y*p->linesize[0] + 1, width, height, p->linesize[0], 1, 2);
    } else {
    }
            av_log(f->avctx, AV_LOG_ERROR, "bytestream end mismatching by %d\n", v);
            fs->slice_damaged = 1;
        }
    }



}

{



            return AVERROR_INVALIDDATA;

        }
    }


}

                             int16_t quant_table[MAX_CONTEXT_INPUTS][256])
{

            return ret;
            return AVERROR_INVALIDDATA;
        }
    }
}

{



        av_log(f->avctx, AV_LOG_ERROR, "Invalid version in global header\n");
        return AVERROR_INVALIDDATA;
    }
            return AVERROR_INVALIDDATA;
    }

    }


        av_log(f->avctx, AV_LOG_ERROR, "chroma shift parameters %d %d are invalid\n",
               f->chroma_h_shift, f->chroma_v_shift);
        return AVERROR_INVALIDDATA;
    }

       ) {
        av_log(f->avctx, AV_LOG_ERROR, "slice count invalid\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(f->avctx, AV_LOG_ERROR, "quant table count %d is invalid\n", f->quant_table_count);
        f->quant_table_count = 0;
        return AVERROR_INVALIDDATA;
    }

            av_log(f->avctx, AV_LOG_ERROR, "read_quant_table error\n");
            return AVERROR_INVALIDDATA;
        }
    }
        return ret;

            for (j = 0; j < f->context_count[i]; j++)
                for (k = 0; k < CONTEXT_SIZE; k++) {
                    int pred = j ? f->initial_states[i][j - 1][k] : 128;
                    f->initial_states[i][j][k] =
                        (pred + get_symbol(c, state2[k], 1)) & 0xFF;
                }
        }

    }

            av_log(f->avctx, AV_LOG_ERROR, "CRC mismatch %X!\n", v);
            return AVERROR_INVALIDDATA;
        }
    }

        av_log(f->avctx, AV_LOG_DEBUG,
               "global: ver:%d.%d, coder:%d, colorspace: %d bpr:%d chroma:%d(%d:%d), alpha:%d slices:%dx%d qtabs:%d ec:%d intra:%d CRC:0x%08X\n",
               f->version, f->micro_version,
               f->ac,
               f->colorspace,
               f->avctx->bits_per_raw_sample,
               f->chroma_planes, f->chroma_h_shift, f->chroma_v_shift,
               f->transparency,
               f->num_h_slices, f->num_v_slices,
               f->quant_table_count,
               f->ec,
               f->intra,
               crc
              );
    return 0;
}

{


            av_log(f->avctx, AV_LOG_ERROR, "invalid version %d in ver01 header\n", v);
            return AVERROR_INVALIDDATA;
        }

            for (i = 1; i < 256; i++) {
                int st = get_symbol(c, state, 1) + c->one_state[i];
                if (st < 1 || st > 255) {
                    av_log(f->avctx, AV_LOG_ERROR, "invalid state transition %d\n", st);
                    return AVERROR_INVALIDDATA;
                }
                f->state_transition[i] = st;
            }
        }

            transparency = 0;

                av_log(f->avctx, AV_LOG_ERROR, "Invalid change of global parameters\n");
                return AVERROR_INVALIDDATA;
            }
        }

            av_log(f->avctx, AV_LOG_ERROR, "chroma shift parameters %d %d are invalid\n",
                   chroma_h_shift, chroma_v_shift);
            return AVERROR_INVALIDDATA;
        }


    }

            if (f->avctx->bits_per_raw_sample <= 8)
                f->avctx->pix_fmt = AV_PIX_FMT_GRAY8;
            else if (f->avctx->bits_per_raw_sample == 9) {
                f->packed_at_lsb = 1;
                f->avctx->pix_fmt = AV_PIX_FMT_GRAY9;
            } else if (f->avctx->bits_per_raw_sample == 10) {
                f->packed_at_lsb = 1;
                f->avctx->pix_fmt = AV_PIX_FMT_GRAY10;
            } else if (f->avctx->bits_per_raw_sample == 12) {
                f->packed_at_lsb = 1;
                f->avctx->pix_fmt = AV_PIX_FMT_GRAY12;
            } else if (f->avctx->bits_per_raw_sample == 16) {
                f->packed_at_lsb = 1;
                f->avctx->pix_fmt = AV_PIX_FMT_GRAY16;
            } else if (f->avctx->bits_per_raw_sample < 16) {
                f->avctx->pix_fmt = AV_PIX_FMT_GRAY16;
            } else
                return AVERROR(ENOSYS);
            if (f->avctx->bits_per_raw_sample <= 8)
                f->avctx->pix_fmt = AV_PIX_FMT_YA8;
            else
                return AVERROR(ENOSYS);
            case 0x00: f->avctx->pix_fmt = AV_PIX_FMT_YUV444P; break;
            case 0x01: f->avctx->pix_fmt = AV_PIX_FMT_YUV440P; break;
            case 0x10: f->avctx->pix_fmt = AV_PIX_FMT_YUV422P; break;
            case 0x20: f->avctx->pix_fmt = AV_PIX_FMT_YUV411P; break;
            case 0x22: f->avctx->pix_fmt = AV_PIX_FMT_YUV410P; break;
            }
            switch(16*f->chroma_h_shift + f->chroma_v_shift) {
            case 0x00: f->avctx->pix_fmt = AV_PIX_FMT_YUVA444P; break;
            case 0x10: f->avctx->pix_fmt = AV_PIX_FMT_YUVA422P; break;
            case 0x11: f->avctx->pix_fmt = AV_PIX_FMT_YUVA420P; break;
            }
            f->packed_at_lsb = 1;
            switch(16 * f->chroma_h_shift + f->chroma_v_shift) {
            case 0x00: f->avctx->pix_fmt = AV_PIX_FMT_YUV444P9; break;
            case 0x10: f->avctx->pix_fmt = AV_PIX_FMT_YUV422P9; break;
            case 0x11: f->avctx->pix_fmt = AV_PIX_FMT_YUV420P9; break;
            }
            f->packed_at_lsb = 1;
            switch(16 * f->chroma_h_shift + f->chroma_v_shift) {
            case 0x00: f->avctx->pix_fmt = AV_PIX_FMT_YUVA444P9; break;
            case 0x10: f->avctx->pix_fmt = AV_PIX_FMT_YUVA422P9; break;
            case 0x11: f->avctx->pix_fmt = AV_PIX_FMT_YUVA420P9; break;
            }
            case 0x00: f->avctx->pix_fmt = AV_PIX_FMT_YUV444P10; break;
            case 0x01: f->avctx->pix_fmt = AV_PIX_FMT_YUV440P10; break;
            case 0x11: f->avctx->pix_fmt = AV_PIX_FMT_YUV420P10; break;
            }
            f->packed_at_lsb = 1;
            switch(16 * f->chroma_h_shift + f->chroma_v_shift) {
            case 0x00: f->avctx->pix_fmt = AV_PIX_FMT_YUVA444P10; break;
            case 0x10: f->avctx->pix_fmt = AV_PIX_FMT_YUVA422P10; break;
            case 0x11: f->avctx->pix_fmt = AV_PIX_FMT_YUVA420P10; break;
            }
            f->packed_at_lsb = 1;
            switch(16 * f->chroma_h_shift + f->chroma_v_shift) {
            case 0x00: f->avctx->pix_fmt = AV_PIX_FMT_YUV444P12; break;
            case 0x01: f->avctx->pix_fmt = AV_PIX_FMT_YUV440P12; break;
            case 0x10: f->avctx->pix_fmt = AV_PIX_FMT_YUV422P12; break;
            case 0x11: f->avctx->pix_fmt = AV_PIX_FMT_YUV420P12; break;
            }
            f->packed_at_lsb = 1;
            switch(16 * f->chroma_h_shift + f->chroma_v_shift) {
            case 0x00: f->avctx->pix_fmt = AV_PIX_FMT_YUV444P14; break;
            case 0x10: f->avctx->pix_fmt = AV_PIX_FMT_YUV422P14; break;
            case 0x11: f->avctx->pix_fmt = AV_PIX_FMT_YUV420P14; break;
            }
            case 0x10: f->avctx->pix_fmt = AV_PIX_FMT_YUV422P16; break;
            case 0x11: f->avctx->pix_fmt = AV_PIX_FMT_YUV420P16; break;
            }
        } else if (f->avctx->bits_per_raw_sample == 16 && f->transparency){
            f->packed_at_lsb = 1;
            switch(16 * f->chroma_h_shift + f->chroma_v_shift) {
            case 0x00: f->avctx->pix_fmt = AV_PIX_FMT_YUVA444P16; break;
            case 0x10: f->avctx->pix_fmt = AV_PIX_FMT_YUVA422P16; break;
            case 0x11: f->avctx->pix_fmt = AV_PIX_FMT_YUVA420P16; break;
            }
        }
            av_log(f->avctx, AV_LOG_ERROR,
                   "chroma subsampling not supported in this colorspace\n");
            return AVERROR(ENOSYS);
        }
            f->avctx->pix_fmt = AV_PIX_FMT_RGB32;
            f->avctx->pix_fmt = AV_PIX_FMT_GBRP9;
            f->avctx->pix_fmt = AV_PIX_FMT_GBRP10;
            f->avctx->pix_fmt = AV_PIX_FMT_GBRAP10;
            f->avctx->pix_fmt = AV_PIX_FMT_GBRP12;
            f->avctx->pix_fmt = AV_PIX_FMT_GBRAP12;
            f->avctx->pix_fmt = AV_PIX_FMT_GBRP14;
        }
        else if (f->avctx->bits_per_raw_sample == 16 && f->transparency) {
            f->avctx->pix_fmt = AV_PIX_FMT_GBRAP16;
            f->use32bit = 1;
        }
    } else {
        av_log(f->avctx, AV_LOG_ERROR, "colorspace not supported\n");
        return AVERROR(ENOSYS);
    }
        av_log(f->avctx, AV_LOG_ERROR, "format not supported\n");
        return AVERROR(ENOSYS);
    }

            f->chroma_h_shift, f->chroma_v_shift, f->avctx->pix_fmt);
            av_log(f->avctx, AV_LOG_ERROR, "read_quant_table error\n");
            return AVERROR_INVALIDDATA;
        }
        f->slice_count = get_symbol(c, state, 0);
    } else {
                break;
        }
    }
        av_log(f->avctx, AV_LOG_ERROR, "slice count %d is invalid (max=%d)\n", f->slice_count, f->max_slice_count);
        return AVERROR_INVALIDDATA;
    }



            fs->slice_x      =  get_symbol(c, state, 0)      * f->width ;
            fs->slice_y      =  get_symbol(c, state, 0)      * f->height;
            fs->slice_width  = (get_symbol(c, state, 0) + 1) * f->width  + fs->slice_x;
            fs->slice_height = (get_symbol(c, state, 0) + 1) * f->height + fs->slice_y;

            fs->slice_x     /= f->num_h_slices;
            fs->slice_y     /= f->num_v_slices;
            fs->slice_width  = fs->slice_width  / f->num_h_slices - fs->slice_x;
            fs->slice_height = fs->slice_height / f->num_v_slices - fs->slice_y;
            if ((unsigned)fs->slice_width  > f->width ||
                (unsigned)fs->slice_height > f->height)
                return AVERROR_INVALIDDATA;
            if (   (unsigned)fs->slice_x + (uint64_t)fs->slice_width  > f->width
                || (unsigned)fs->slice_y + (uint64_t)fs->slice_height > f->height)
                return AVERROR_INVALIDDATA;
        }


                int idx = get_symbol(c, state, 0);
                if (idx > (unsigned)f->quant_table_count) {
                    av_log(f->avctx, AV_LOG_ERROR,
                           "quant_table_index out of range\n");
                    return AVERROR_INVALIDDATA;
                }
                p->quant_table_index = idx;
                memcpy(p->quant_table, f->quant_tables[idx],
                       sizeof(p->quant_table));
                context_count = f->context_count[idx];
            } else {
            }

                }
            }
        }
    }
    return 0;
}

{

        return ret;

        return ret;

        return ret;

    return 0;
}

{



        /* we have interlaced material flagged in container */
        p->interlaced_frame = 1;
        if (avctx->field_order == AV_FIELD_TT || avctx->field_order == AV_FIELD_TB)
            p->top_field_first = 1;
    }


            return ret;
    } else {
            av_log(avctx, AV_LOG_ERROR,
                   "Cannot decode non-keyframe without valid keyframe\n");
            return AVERROR_INVALIDDATA;
        }
    }

        return ret;

        av_log(avctx, AV_LOG_DEBUG, "ver:%d keyframe:%d coder:%d ec:%d slices:%d bps:%d\n",
               f->version, p->key_frame, f->ac, f->ec, f->slice_count, f->avctx->bits_per_raw_sample);



            av_log(avctx, AV_LOG_ERROR, "Slice pointer chain broken\n");
            ff_thread_report_progress(&f->picture, INT_MAX, 0);
            return AVERROR_INVALIDDATA;
        }

                int64_t ts = avpkt->pts != AV_NOPTS_VALUE ? avpkt->pts : avpkt->dts;
                av_log(f->avctx, AV_LOG_ERROR, "slice CRC mismatch %X!", crc);
                if (ts != AV_NOPTS_VALUE && avctx->pkt_timebase.num) {
                    av_log(f->avctx, AV_LOG_ERROR, "at %f seconds\n", ts*av_q2d(avctx->pkt_timebase));
                } else if (ts != AV_NOPTS_VALUE) {
                    av_log(f->avctx, AV_LOG_ERROR, "at %"PRId64"\n", ts);
                } else {
                    av_log(f->avctx, AV_LOG_ERROR, "\n");
                }
                fs->slice_damaged = 1;
            }
                av_log(avctx, AV_LOG_DEBUG, "slice %d, CRC: 0x%08"PRIX32"\n", i, AV_RB32(buf_p + v - 4));
            }
        }

        } else

    }

                   decode_slice,
                   NULL,
                   f->slice_count,
                   sizeof(void*));

            const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(avctx->pix_fmt);
            const uint8_t *src[4];
            uint8_t *dst[4];
            ff_thread_await_progress(&f->last_picture, INT_MAX, 0);
            for (j = 0; j < desc->nb_components; j++) {
                int pixshift = desc->comp[j].depth > 8;
                int sh = (j == 1 || j == 2) ? f->chroma_h_shift : 0;
                int sv = (j == 1 || j == 2) ? f->chroma_v_shift : 0;
                dst[j] = p->data[j] + p->linesize[j] *
                         (fs->slice_y >> sv) + ((fs->slice_x >> sh) << pixshift);
                src[j] = f->last_picture.f->data[j] + f->last_picture.f->linesize[j] *
                         (fs->slice_y >> sv) + ((fs->slice_x >> sh) << pixshift);

            }
            if (desc->flags & AV_PIX_FMT_FLAG_PAL ||
                desc->flags & FF_PSEUDOPAL) {
                dst[1] = p->data[1];
                src[1] = f->last_picture.f->data[1];
            }
            av_image_copy(dst, p->linesize, src,
                          f->last_picture.f->linesize,
                          avctx->pix_fmt,
                          fs->slice_width,
                          fs->slice_height);
        }
    }


        return ret;


}

static void copy_fields(FFV1Context *fsdst, FFV1Context *fssrc, FFV1Context *fsrc)
{
    fsdst->version             = fsrc->version;
    fsdst->micro_version       = fsrc->micro_version;
    fsdst->chroma_planes       = fsrc->chroma_planes;
    fsdst->chroma_h_shift      = fsrc->chroma_h_shift;
    fsdst->chroma_v_shift      = fsrc->chroma_v_shift;
    fsdst->transparency        = fsrc->transparency;
    fsdst->plane_count         = fsrc->plane_count;
    fsdst->ac                  = fsrc->ac;
    fsdst->colorspace          = fsrc->colorspace;

    fsdst->ec                  = fsrc->ec;
    fsdst->intra               = fsrc->intra;
    fsdst->slice_damaged       = fssrc->slice_damaged;
    fsdst->key_frame_ok        = fsrc->key_frame_ok;

    fsdst->bits_per_raw_sample = fsrc->bits_per_raw_sample;
    fsdst->packed_at_lsb       = fsrc->packed_at_lsb;
    fsdst->slice_count         = fsrc->slice_count;
    if (fsrc->version<3){
        fsdst->slice_x             = fssrc->slice_x;
        fsdst->slice_y             = fssrc->slice_y;
        fsdst->slice_width         = fssrc->slice_width;
        fsdst->slice_height        = fssrc->slice_height;
    }
}

#if HAVE_THREADS
static int update_thread_context(AVCodecContext *dst, const AVCodecContext *src)
{
    FFV1Context *fsrc = src->priv_data;
    FFV1Context *fdst = dst->priv_data;
    int i, ret;

    if (dst == src)
        return 0;

    {
        ThreadFrame picture = fdst->picture, last_picture = fdst->last_picture;
        uint8_t (*initial_states[MAX_QUANT_TABLES])[32];
        struct FFV1Context *slice_context[MAX_SLICES];
        memcpy(initial_states, fdst->initial_states, sizeof(fdst->initial_states));
        memcpy(slice_context,  fdst->slice_context , sizeof(fdst->slice_context));

        memcpy(fdst, fsrc, sizeof(*fdst));
        memcpy(fdst->initial_states, initial_states, sizeof(fdst->initial_states));
        memcpy(fdst->slice_context,  slice_context , sizeof(fdst->slice_context));
        fdst->picture      = picture;
        fdst->last_picture = last_picture;
        for (i = 0; i<fdst->num_h_slices * fdst->num_v_slices; i++) {
            FFV1Context *fssrc = fsrc->slice_context[i];
            FFV1Context *fsdst = fdst->slice_context[i];
            copy_fields(fsdst, fssrc, fsrc);
        }
        av_assert0(!fdst->plane[0].state);
        av_assert0(!fdst->sample_buffer);
    }

    av_assert1(fdst->max_slice_count == fsrc->max_slice_count);


    ff_thread_release_buffer(dst, &fdst->picture);
    if (fsrc->picture.f->data[0]) {
        if ((ret = ff_thread_ref_frame(&fdst->picture, &fsrc->picture)) < 0)
            return ret;
    }

    fdst->fsrc = fsrc;

    return 0;
}
#endif

AVCodec ff_ffv1_decoder = {
    .name           = "ffv1",
    .long_name      = NULL_IF_CONFIG_SMALL("FFmpeg video codec #1"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_FFV1,
    .priv_data_size = sizeof(FFV1Context),
    .init           = decode_init,
    .close          = ff_ffv1_close,
    .decode         = decode_frame,
    .update_thread_context = ONLY_IF_THREADS_ENABLED(update_thread_context),
    .capabilities   = AV_CODEC_CAP_DR1 /*| AV_CODEC_CAP_DRAW_HORIZ_BAND*/ |
                      AV_CODEC_CAP_FRAME_THREADS | AV_CODEC_CAP_SLICE_THREADS,
    .caps_internal  = FF_CODEC_CAP_INIT_CLEANUP | FF_CODEC_CAP_ALLOCATE_PROGRESS,
};
