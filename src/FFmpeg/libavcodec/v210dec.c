/*
 * V210 decoder
 *
 * Copyright (C) 2009 Michael Niedermayer <michaelni@gmx.at>
 * Copyright (c) 2009 Baptiste Coudurier <baptiste dot coudurier at gmail dot com>
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

#include "avcodec.h"
#include "internal.h"
#include "v210dec.h"
#include "libavutil/bswap.h"
#include "libavutil/internal.h"
#include "libavutil/mem.h"
#include "libavutil/intreadwrite.h"
#include "thread.h"

#define READ_PIXELS(a, b, c)         \
    do {                             \
        val  = av_le2ne32(*src++);   \
        *a++ =  val & 0x3FF;         \
        *b++ = (val >> 10) & 0x3FF;  \
        *c++ = (val >> 20) & 0x3FF;  \
    } while (0)

typedef struct ThreadData {
    AVFrame *frame;
    uint8_t *buf;
    int stride;
} ThreadData;

{

    }

{

{



}

{




        }



            }
        }

    }

}

                        AVPacket *avpkt)
{

        stride = s->custom_stride;
    else {
    }

            stride = avpkt->size / avctx->height;
            if (!s->stride_warning_shown)
                av_log(avctx, AV_LOG_WARNING, "Broken v210 with too small padding (64 byte) detected\n");
            s->stride_warning_shown = 1;
        } else {
        }
    }
        && avpkt->size > 64
        && AV_RN32(psrc) == AV_RN32("INFO")
        && avpkt->size - 64 >= stride * avctx->height)
        psrc += 64;

    }

        return ret;



        /* we have interlaced material flagged in container */
        pic->interlaced_frame = 1;
        if (avctx->field_order == AV_FIELD_TT || avctx->field_order == AV_FIELD_TB)
            pic->top_field_first = 1;
    }


}

#define V210DEC_FLAGS AV_OPT_FLAG_DECODING_PARAM | AV_OPT_FLAG_VIDEO_PARAM
static const AVOption v210dec_options[] = {
    {"custom_stride", "Custom V210 stride", offsetof(V210DecContext, custom_stride), AV_OPT_TYPE_INT,
     {.i64 = 0}, INT_MIN, INT_MAX, V210DEC_FLAGS},
    {NULL}
};

static const AVClass v210dec_class = {
    .class_name = "V210 Decoder",
    .item_name  = av_default_item_name,
    .option     = v210dec_options,
    .version    = LIBAVUTIL_VERSION_INT,
};

AVCodec ff_v210_decoder = {
    .name           = "v210",
    .long_name      = NULL_IF_CONFIG_SMALL("Uncompressed 4:2:2 10-bit"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_V210,
    .priv_data_size = sizeof(V210DecContext),
    .init           = decode_init,
    .decode         = decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1 |
                      AV_CODEC_CAP_SLICE_THREADS |
                      AV_CODEC_CAP_FRAME_THREADS,
    .priv_class     = &v210dec_class,
};
