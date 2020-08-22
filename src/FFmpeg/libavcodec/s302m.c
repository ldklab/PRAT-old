/*
 * SMPTE 302M decoder
 * Copyright (c) 2008 Laurent Aimar <fenrir@videolan.org>
 * Copyright (c) 2009 Baptiste Coudurier <baptiste.coudurier@gmail.com>
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

#include "libavutil/intreadwrite.h"
#include "libavutil/opt.h"
#include "libavutil/log.h"
#include "avcodec.h"
#include "internal.h"
#include "mathops.h"

#define AES3_HEADER_LEN 4

typedef struct S302Context {
    AVClass *class;
    int non_pcm_mode;
} S302Context;

                                    int buf_size)
{

        av_log(avctx, AV_LOG_ERROR, "frame is too short\n");
        return AVERROR_INVALIDDATA;
    }

    /*
     * AES3 header :
     * size:            16
     * number channels   2
     * channel_id        8
     * bits per samples  2
     * alignments        4
     */


    }

    /* Set output properties */
        avctx->sample_fmt = AV_SAMPLE_FMT_S32;
    else

        case 4:
            avctx->channel_layout = AV_CH_LAYOUT_QUAD;
            break;
        case 6:
            avctx->channel_layout = AV_CH_LAYOUT_5POINT1_BACK;
            break;
        case 8:
            avctx->channel_layout = AV_CH_LAYOUT_5POINT1_BACK | AV_CH_LAYOUT_STEREO_DOWNMIX;
    }

    return frame_size;
}

                              int *got_frame_ptr, AVPacket *avpkt)
{

        return frame_size;


    /* get output buffer */
        return ret;


        uint32_t *o = (uint32_t *)frame->data[0];
        for (; buf_size > 6; buf_size -= 7) {
            *o++ = ((unsigned)ff_reverse[buf[2]]        << 24) |
                   (ff_reverse[buf[1]]        << 16) |
                   (ff_reverse[buf[0]]        <<  8);
            *o++ = ((unsigned)ff_reverse[buf[6] & 0xf0] << 28) |
                   (ff_reverse[buf[5]]        << 20) |
                   (ff_reverse[buf[4]]        << 12) |
                   (ff_reverse[buf[3] & 0x0f] <<  4);
            buf += 7;
        }
        o = (uint32_t *)frame->data[0];
        if (avctx->channels == 2)
            for (i=0; i<frame->nb_samples * 2 - 6; i+=2) {
                if (o[i] || o[i+1] || o[i+2] || o[i+3])
                    break;
                if (o[i+4] == 0x96F87200U && o[i+5] == 0xA54E1F00) {
                    non_pcm_data_type = (o[i+6] >> 16) & 0x1F;
                    break;
                }
            }
        uint32_t *o = (uint32_t *)frame->data[0];
        for (; buf_size > 5; buf_size -= 6) {
            *o++ = ((unsigned)ff_reverse[buf[2] & 0xf0] << 28) |
                   (ff_reverse[buf[1]]        << 20) |
                   (ff_reverse[buf[0]]        << 12);
            *o++ = ((unsigned)ff_reverse[buf[5] & 0xf0] << 28) |
                   (ff_reverse[buf[4]]        << 20) |
                   (ff_reverse[buf[3]]        << 12);
            buf += 6;
        }
        o = (uint32_t *)frame->data[0];
        if (avctx->channels == 2)
            for (i=0; i<frame->nb_samples * 2 - 6; i+=2) {
                if (o[i] || o[i+1] || o[i+2] || o[i+3])
                    break;
                if (o[i+4] == 0x6F872000U && o[i+5] == 0x54E1F000) {
                    non_pcm_data_type = (o[i+6] >> 16) & 0x1F;
                    break;
                }
            }
    } else {
        }
                    break;
                if (o[i+4] == 0xF872U && o[i+5] == 0x4E1F) {
                    non_pcm_data_type = (o[i+6] & 0x1F);
                    break;
                }
            }
    }

        if (s->non_pcm_mode == 3) {
            av_log(avctx, AV_LOG_ERROR,
                   "S302 non PCM mode with data type %d not supported\n",
                   non_pcm_data_type);
            return AVERROR_PATCHWELCOME;
        }
        if (s->non_pcm_mode & 1) {
            return avpkt->size;
        }
    }



}

#define FLAGS AV_OPT_FLAG_AUDIO_PARAM|AV_OPT_FLAG_DECODING_PARAM
static const AVOption s302m_options[] = {
    {"non_pcm_mode", "Chooses what to do with NON-PCM", offsetof(S302Context, non_pcm_mode), AV_OPT_TYPE_INT, {.i64 = 3}, 0, 3, FLAGS, "non_pcm_mode"},
    {"copy"        , "Pass NON-PCM through unchanged"     , 0, AV_OPT_TYPE_CONST, {.i64 = 0}, 0, 3, FLAGS, "non_pcm_mode"},
    {"drop"        , "Drop NON-PCM"                       , 0, AV_OPT_TYPE_CONST, {.i64 = 1}, 0, 3, FLAGS, "non_pcm_mode"},
    {"decode_copy" , "Decode if possible else passthrough", 0, AV_OPT_TYPE_CONST, {.i64 = 2}, 0, 3, FLAGS, "non_pcm_mode"},
    {"decode_drop" , "Decode if possible else drop"       , 0, AV_OPT_TYPE_CONST, {.i64 = 3}, 0, 3, FLAGS, "non_pcm_mode"},
    {NULL}
};

static const AVClass s302m_class = {
    .class_name = "SMPTE 302M Decoder",
    .item_name  = av_default_item_name,
    .option     = s302m_options,
    .version    = LIBAVUTIL_VERSION_INT,
};

AVCodec ff_s302m_decoder = {
    .name           = "s302m",
    .long_name      = NULL_IF_CONFIG_SMALL("SMPTE 302M"),
    .type           = AVMEDIA_TYPE_AUDIO,
    .id             = AV_CODEC_ID_S302M,
    .priv_data_size = sizeof(S302Context),
    .decode         = s302m_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
    .priv_class     = &s302m_class,
};
