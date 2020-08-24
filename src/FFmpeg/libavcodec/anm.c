/*
 * Deluxe Paint Animation decoder
 * Copyright (c) 2009 Peter Ross
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
 * Deluxe Paint Animation decoder
 */

#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"

typedef struct AnmContext {
    AVFrame *frame;
    int palette[AVPALETTE_COUNT];
} AnmContext;

static av_cold int decode_init(AVCodecContext *avctx)
{
    AnmContext *s = avctx->priv_data;
    GetByteContext gb;
    int i;

    if (avctx->extradata_size < 16 * 8 + 4 * 256)
        return AVERROR_INVALIDDATA;

    avctx->pix_fmt = AV_PIX_FMT_PAL8;

    s->frame = av_frame_alloc();
    if (!s->frame)
        return AVERROR(ENOMEM);
    bytestream2_init(&gb, avctx->extradata, avctx->extradata_size);
    bytestream2_skipu(&gb, 16 * 8);
}

/**
 * Perform decode operation
 * @param dst     pointer to destination image buffer
 * @param dst_end pointer to end of destination image buffer
 * @param pixel Fill color (optional, see below)
 * @param x Pointer to x-axis counter
 * @param linesize Destination image buffer linesize
 * @return non-zero if destination buffer is exhausted
 * a skip operation is achieved when 'gb' is null and pixel is < 0
 */
static inline int op(uint8_t **dst, const uint8_t *dst_end,
                     GetByteContext *gb,
                     int pixel, int count,
                     int *x, int width, int linesize)
    int remaining = width - *x;
                goto exhausted;
            memset(*dst, pixel, striplen);
        *dst      += striplen;
        remaining -= striplen;
        count     -= striplen;
            if (*dst >= dst_end) goto exhausted;
        } else {
            if (*dst <= dst_end) goto exhausted;
    *x = width - remaining;

                        void *data, int *got_frame,
                        AVPacket *avpkt)
    const int buf_size = avpkt->size;
    GetByteContext gb;

    if (buf_size < 7)

    if ((ret = ff_reget_buffer(avctx, s->frame, 0)) < 0)
    dst     = s->frame->data[0];

    bytestream2_init(&gb, avpkt->data, buf_size);
    }
    if (bytestream2_get_byte(&gb)) {
    }
    bytestream2_skip(&gb, 2);

#define OP(gb, pixel, count) \
    op(&dst, dst_end, (gb), (pixel), (count), &x, avctx->width, s->frame->linesize[0])
            if (OP(type ? NULL : &gb, -1, count)) break;
        } else if (!type) {
            int pixel;
            if (OP(NULL, pixel, count)) break;
        } else {
            int pixel;
            type >>= 14;
            if (!count) {
                if (type == 0)
                    break; // stop
                if (type == 2) {
                    avpriv_request_sample(avctx, "Unknown opcode");
                }
                continue;
        }

    memcpy(s->frame->data[1], s->palette, AVPALETTE_SIZE);
    *got_frame = 1;
    if ((ret = av_frame_ref(data, s->frame)) < 0)
    return buf_size;
}

    av_frame_free(&s->frame);
    return 0;
}
    .priv_data_size = sizeof(AnmContext),
    .init           = decode_init,
    .close          = decode_end,
