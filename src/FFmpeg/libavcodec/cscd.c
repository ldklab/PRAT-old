/*
 * CamStudio decoder
 * Copyright (c) 2006 Reimar Doeffinger
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
#include <stdio.h>
#include <stdlib.h>

#include "avcodec.h"
#include "internal.h"
#include "libavutil/common.h"

#if CONFIG_ZLIB
#include <zlib.h>
#endif
#include "libavutil/lzo.h"

typedef struct CamStudioContext {
    AVFrame *pic;
    int linelen, height, bpp;
    unsigned int decomp_size;
    unsigned char* decomp_buf;
} CamStudioContext;

static void copy_frame_default(AVFrame *f, const uint8_t *src,
                               int linelen, int height)
{
    int i, src_stride = FFALIGN(linelen, 4);
    uint8_t *dst = f->data[0];
    dst += (height - 1) * f->linesize[0];
    for (i = height; i; i--) {
        memcpy(dst, src, linelen);
        src += src_stride;
        dst -= f->linesize[0];
    }
}

static void add_frame_default(AVFrame *f, const uint8_t *src,
                              int linelen, int height)
{
    int i, j, src_stride = FFALIGN(linelen, 4);
    uint8_t *dst = f->data[0];
    dst += (height - 1) * f->linesize[0];
    for (i = height; i; i--) {
        for (j = linelen; j; j--)
            *dst++ += *src++;
        src += src_stride - linelen;
        dst -= f->linesize[0] + linelen;
    }
}

                        AVPacket *avpkt)
{

        av_log(avctx, AV_LOG_ERROR, "coded frame too small\n");
        return AVERROR_INVALIDDATA;
    }

        return ret;

    // decompress data
            av_log(avctx, AV_LOG_ERROR, "error during lzo decompression\n");
            return AVERROR_INVALIDDATA;
        }
    }
    case 1: { // zlib compression
#if CONFIG_ZLIB
        unsigned long dlen = c->decomp_size;
        if (uncompress(c->decomp_buf, &dlen, &buf[2], buf_size - 2) != Z_OK) {
            av_log(avctx, AV_LOG_ERROR, "error during zlib decompression\n");
            return AVERROR_INVALIDDATA;
        }
        break;
#else
        av_log(avctx, AV_LOG_ERROR, "compiled without zlib support\n");
        return AVERROR(ENOSYS);
#endif
    }
    default:
        av_log(avctx, AV_LOG_ERROR, "unknown compression\n");
        return AVERROR_INVALIDDATA;
    }

    // flip upside down, add difference frame
                                 c->linelen, c->height);
    } else {
                                c->linelen, c->height);
    }

        return ret;

    return buf_size;
}

{
    case 16: avctx->pix_fmt = AV_PIX_FMT_RGB555LE; break;
    case 24: avctx->pix_fmt = AV_PIX_FMT_BGR24; break;
    default:
        av_log(avctx, AV_LOG_ERROR,
               "CamStudio codec error: invalid depth %i bpp\n",
               avctx->bits_per_coded_sample);
        return AVERROR_INVALIDDATA;
    }
        av_log(avctx, AV_LOG_ERROR, "Can't allocate decompression buffer.\n");
        return AVERROR(ENOMEM);
    }
        return AVERROR(ENOMEM);
    return 0;
}

{
}

AVCodec ff_cscd_decoder = {
    .name           = "camstudio",
    .long_name      = NULL_IF_CONFIG_SMALL("CamStudio"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_CSCD,
    .priv_data_size = sizeof(CamStudioContext),
    .init           = decode_init,
    .close          = decode_end,
    .decode         = decode_frame,
    .caps_internal  = FF_CODEC_CAP_INIT_CLEANUP,
    .capabilities   = AV_CODEC_CAP_DR1,
};
