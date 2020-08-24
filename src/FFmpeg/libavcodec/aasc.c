/*
 * Autodesk RLE Decoder
 * Copyright (C) 2005 The FFmpeg project
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
 * Autodesk RLE Video Decoder by Konstantin Shishkov
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "avcodec.h"
#include "internal.h"
#include "msrledec.h"

typedef struct AascContext {
    AVCodecContext *avctx;
    GetByteContext gb;
    AVFrame *frame;

    uint32_t palette[AVPALETTE_COUNT];
    int palette_size;
} AascContext;

{

    case 8:
        avctx->pix_fmt = AV_PIX_FMT_PAL8;

        ptr = avctx->extradata;
        s->palette_size = FFMIN(avctx->extradata_size, AVPALETTE_SIZE);
        for (i = 0; i < s->palette_size / 4; i++) {
            s->palette[i] = 0xFFU << 24 | AV_RL32(ptr);
            ptr += 4;
        }
        break;
    case 16:
        avctx->pix_fmt = AV_PIX_FMT_RGB555LE;
        break;
    default:
        av_log(avctx, AV_LOG_ERROR, "Unsupported bit depth: %d\n", avctx->bits_per_coded_sample);
        return -1;
    }

        return AVERROR(ENOMEM);

    return 0;
}

                              void *data, int *got_frame,
                              AVPacket *avpkt)
{

        av_log(avctx, AV_LOG_ERROR, "frame too short\n");
        return AVERROR_INVALIDDATA;
    }

        return ret;

    case MKTAG('A', 'A', 'S', '4'):
        bytestream2_init(&s->gb, buf - 4, buf_size + 4);
        ff_msrle_decode(avctx, s->frame, 8, &s->gb);
        break;
    case 0:
        stride = (avctx->width * psize + psize) & ~psize;
        if (buf_size < stride * avctx->height)
            return AVERROR_INVALIDDATA;
        for (i = avctx->height - 1; i >= 0; i--) {
            memcpy(s->frame->data[0] + i * s->frame->linesize[0], buf, avctx->width * psize);
            buf += stride;
            buf_size -= stride;
        }
        break;
    default:
        av_log(avctx, AV_LOG_ERROR, "Unknown compression type %d\n", compr);
        return AVERROR_INVALIDDATA;
    }
        break;
    default:
        av_log(avctx, AV_LOG_ERROR, "Unknown FourCC: %X\n", avctx->codec_tag);
        return -1;
    }

        memcpy(s->frame->data[1], s->palette, s->palette_size);

        return ret;

    /* report that the buffer was completely consumed */
}

{


}

AVCodec ff_aasc_decoder = {
    .name           = "aasc",
    .long_name      = NULL_IF_CONFIG_SMALL("Autodesk RLE"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_AASC,
    .priv_data_size = sizeof(AascContext),
    .init           = aasc_decode_init,
    .close          = aasc_decode_end,
    .decode         = aasc_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
    .caps_internal  = FF_CODEC_CAP_INIT_THREADSAFE,
};
