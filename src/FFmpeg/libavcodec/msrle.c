/*
 * Microsoft RLE video decoder
 * Copyright (C) 2003 The FFmpeg project
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
 * MS RLE video decoder by Mike Melanson (melanson@pcisys.net)
 * For more information about the MS RLE format, visit:
 *   http://www.pcisys.net/~melanson/codecs/
 *
 * The MS RLE decoder outputs PAL8 colorspace data.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "avcodec.h"
#include "internal.h"
#include "msrledec.h"
#include "libavutil/imgutils.h"

typedef struct MsrleContext {
    AVCodecContext *avctx;
    AVFrame *frame;

    GetByteContext gb;

    uint32_t pal[256];
} MsrleContext;

{


    case 1:
        avctx->pix_fmt = AV_PIX_FMT_MONOWHITE;
        break;
    case 8:
    case 24:
        avctx->pix_fmt = AV_PIX_FMT_BGR24;
        break;
    default:
        av_log(avctx, AV_LOG_ERROR, "unsupported bits per sample\n");
        return AVERROR_INVALIDDATA;
    }

        return AVERROR(ENOMEM);


    return 0;
}

                              void *data, int *got_frame,
                              AVPacket *avpkt)
{

        return AVERROR_INVALIDDATA;

        return ret;


            av_log(avctx, AV_LOG_ERROR, "Palette size %d is wrong\n", size);
        }
        /* make the palette available */
    }

    /* FIXME how to correctly detect RLE ??? */
        int linesize = av_image_get_linesize(avctx->pix_fmt, avctx->width, 0);
        uint8_t *ptr = s->frame->data[0];
        uint8_t *buf = avpkt->data + (avctx->height-1)*istride;
        int i, j;

        if (linesize < 0)
            return linesize;

        for (i = 0; i < avctx->height; i++) {
            if (avctx->bits_per_coded_sample == 4) {
                for (j = 0; j < avctx->width - 1; j += 2) {
                    ptr[j+0] = buf[j>>1] >> 4;
                    ptr[j+1] = buf[j>>1] & 0xF;
                }
                if (avctx->width & 1)
                    ptr[j+0] = buf[j>>1] >> 4;
            } else {
                memcpy(ptr, buf, linesize);
            }
            buf -= istride;
            ptr += s->frame->linesize[0];
        }
    } else {
    }

        return ret;


    /* report that the buffer was completely consumed */
}

static void msrle_decode_flush(AVCodecContext *avctx)
{
    MsrleContext *s = avctx->priv_data;

    av_frame_unref(s->frame);
}

{

    /* release the last frame */

}

AVCodec ff_msrle_decoder = {
    .name           = "msrle",
    .long_name      = NULL_IF_CONFIG_SMALL("Microsoft RLE"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_MSRLE,
    .priv_data_size = sizeof(MsrleContext),
    .init           = msrle_decode_init,
    .close          = msrle_decode_end,
    .decode         = msrle_decode_frame,
    .flush          = msrle_decode_flush,
    .capabilities   = AV_CODEC_CAP_DR1,
};
