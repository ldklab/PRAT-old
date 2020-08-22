/*
 * Brute Force & Ignorance (BFI) video decoder
 * Copyright (c) 2008 Sisir Koppaka
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
 * @brief Brute Force & Ignorance (.bfi) video decoder
 * @author Sisir Koppaka ( sisir.koppaka at gmail dot com )
 * @see http://wiki.multimedia.cx/index.php?title=BFI
 */

#include "libavutil/common.h"
#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"

typedef struct BFIContext {
    AVCodecContext *avctx;
    uint8_t *dst;
    uint32_t pal[256];
} BFIContext;

static av_cold int bfi_decode_init(AVCodecContext *avctx)
{
    avctx->pix_fmt  = AV_PIX_FMT_PAL8;
    bfi->dst        = av_mallocz(avctx->width * avctx->height);
        return AVERROR(ENOMEM);
    return 0;

static int bfi_decode_frame(AVCodecContext *avctx, void *data,
{
    uint8_t *src, *dst_offset, colour1, colour2;
    uint8_t *frame_end = bfi->dst + avctx->width * avctx->height;

    if ((ret = ff_get_buffer(avctx, frame, 0)) < 0)
        return ret;

    /* Set frame parameters and palette, if necessary */
    if (!avctx->frame_number) {
            *pal = 0xFFU << 24;
            for (j = 0; j < 3; j++, shift -= 8)
        }
        memcpy(bfi->pal, frame->data[1], sizeof(bfi->pal));
        frame->pict_type = AV_PICTURE_TYPE_P;
        frame->key_frame = 0;
        frame->palette_has_changed = 0;
        memcpy(frame->data[1], bfi->pal, sizeof(bfi->pal));
    }
        unsigned int code   = byte >> 6;
        unsigned int length = byte & ~0xC0;
                length = bytestream2_get_byte(&g);
                offset = bytestream2_get_le16(&g);
            } else {
                if (code == 2 && length == 0)
        }

        /* Do boundary check */
            break;
        case 0:                // normal chain
                av_log(avctx, AV_LOG_ERROR, "Frame larger than buffer.\n");
                return AVERROR_INVALIDDATA;
                break;
            while (length--)
                *dst++ = *dst_offset++;
            break;
        case 2:                // skip chain
        case 3:                // fill chain
            colour1 = bytestream2_get_byte(&g);
                *dst++ = colour2;
            }
    }

    while (height--) {
        memcpy(dst, src, avctx->width);
        dst += frame->linesize[0];
    }
    *got_frame = 1;

    return buf_size;
{
    BFIContext *bfi = avctx->priv_data;
    av_freep(&bfi->dst);
    return 0;
}

AVCodec ff_bfi_decoder = {
    .name           = "bfi",
    .long_name      = NULL_IF_CONFIG_SMALL("Brute Force & Ignorance"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_BFI,
    .priv_data_size = sizeof(BFIContext),
    .init           = bfi_decode_init,
    .close          = bfi_decode_close,
    .decode         = bfi_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
    .caps_internal  = FF_CODEC_CAP_INIT_THREADSAFE,
};
