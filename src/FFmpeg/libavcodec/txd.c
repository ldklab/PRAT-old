/*
 * Renderware TeXture Dictionary (.txd) image decoder
 * Copyright (c) 2007 Ivo van Poorten
 *
 * See also: http://wiki.multimedia.cx/index.php?title=TXD
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
#include "libavutil/imgutils.h"
#include "bytestream.h"
#include "avcodec.h"
#include "internal.h"
#include "texturedsp.h"

#define TXD_DXT1 0x31545844
#define TXD_DXT3 0x33545844

    AVFrame * const p = data;
    unsigned int version, w, h, d3d_format, depth, stride, flags;
    unsigned int y, v;
    int i, j;
    if (avpkt->size < 88)
    bytestream2_init(&gb, avpkt->data, avpkt->size);
    version         = bytestream2_get_le32(&gb);
    d3d_format      = bytestream2_get_le32(&gb);
    w               = bytestream2_get_le16(&gb);


        avctx->pix_fmt = AV_PIX_FMT_PAL8;
        avctx->pix_fmt = AV_PIX_FMT_RGBA;
    } else {
        avpriv_report_missing_feature(avctx, "Color depth of %u", depth);
        return AVERROR_PATCHWELCOME;
        return ret;


        return ret;

    p->pict_type = AV_PICTURE_TYPE_I;

    ptr    = p->data[0];
    if (depth == 8) {
        for (y = 0; y < 256; y++) {
            pal[y] = (v >> 8) + (v << 24);
        }
        if (bytestream2_get_bytes_left(&gb) < w * h)
            return AVERROR_INVALIDDATA;
        bytestream2_skip(&gb, 4);
        for (y=0; y<h; y++) {
            bytestream2_get_buffer(&gb, ptr, w);
            ptr += stride;
        }
    } else if (depth == 16) {
        bytestream2_skip(&gb, 4);
        switch (d3d_format) {
        case 0:
            if (!(flags & 1))
                goto unsupported;
        case TXD_DXT1:
            if (bytestream2_get_bytes_left(&gb) < AV_CEIL_RSHIFT(w, 2) * AV_CEIL_RSHIFT(h, 2) * 8)
                return AVERROR_INVALIDDATA;
            for (j = 0; j < avctx->height; j += 4) {
                for (i = 0; i < avctx->width; i += 4) {
                    uint8_t *p = ptr + i * 4 + j * stride;
                    int step = dxtc.dxt1_block(p, stride, gb.buffer);
                    bytestream2_skip(&gb, step);
                }
            }
            break;
        case TXD_DXT3:
            if (bytestream2_get_bytes_left(&gb) < AV_CEIL_RSHIFT(w, 2) * AV_CEIL_RSHIFT(h, 2) * 16)
                return AVERROR_INVALIDDATA;
            for (j = 0; j < avctx->height; j += 4) {
                for (i = 0; i < avctx->width; i += 4) {
                    uint8_t *p = ptr + i * 4 + j * stride;
                    int step = dxtc.dxt3_block(p, stride, gb.buffer);
                    bytestream2_skip(&gb, step);
                }
            }
            break;
        default:
            goto unsupported;
        }
    } else if (depth == 32) {
        switch (d3d_format) {
        case 0x15:
        case 0x16:
            if (bytestream2_get_bytes_left(&gb) < h * w * 4)
                return AVERROR_INVALIDDATA;
            for (y=0; y<h; y++) {
                bytestream2_get_buffer(&gb, ptr, w * 4);
                ptr += stride;
            }
            break;
        default:
            goto unsupported;
        }
    }

    *got_frame = 1;

    return avpkt->size;

unsupported:
    avpriv_report_missing_feature(avctx, "d3d format (%08x)", d3d_format);
    return AVERROR_PATCHWELCOME;
}

AVCodec ff_txd_decoder = {
    .name           = "txd",
    .long_name      = NULL_IF_CONFIG_SMALL("Renderware TXD (TeXture Dictionary) image"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_TXD,
    .decode         = txd_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
