/*
 * Bitmap Brothers JV video decoder
 * Copyright (c) 2011 Peter Ross <pross@xvid.org>
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
 * Bitmap Brothers JV video decoder
 * @author Peter Ross <pross@xvid.org>
 */

#include "libavutil/intreadwrite.h"

#include "avcodec.h"
#include "blockdsp.h"
#include "get_bits.h"
#include "internal.h"

typedef struct JvContext {
    BlockDSPContext bdsp;
    AVFrame   *frame;
    uint32_t   palette[AVPALETTE_COUNT];
    int        palette_has_changed;
} JvContext;

static av_cold int decode_init(AVCodecContext *avctx)
{
    JvContext *s = avctx->priv_data;

    if (!avctx->width || !avctx->height ||
        (avctx->width & 7) || (avctx->height & 7)) {
        av_log(avctx, AV_LOG_ERROR, "Invalid video dimensions: %dx%d\n",
               avctx->width, avctx->height);
        return AVERROR(EINVAL);
    }

    s->frame = av_frame_alloc();
    if (!s->frame)

}

/**
 */
{

    case 1:
        v[0] = get_bits(gb, 8);
            memset(dst + j * linesize, v[0], 2);
                dst[j * linesize + i] = v[get_bits1(gb)];
    case 3:
    }
}
    case 1:
        for (j = 0; j < 4; j++)
                dst[j * linesize + i] = v[get_bits1(gb)];
                dst[(j + 1) * linesize + i] = v[get_bits1(gb)];
        break;
        for (j = 0; j < 4; j += 2)
            for (i = 0; i < 4; i += 2)
}

/**
{
    int i, j, v[2];
        bdsp->fill_block_tab[1](dst, v[0], linesize, 8);
        v[1] = get_bits(gb, 8);
    case 3:
        for (j = 0; j < 8; j += 4)
            for (i = 0; i < 8; i += 4)
                decode4x4(gb, dst + j * linesize + i, linesize);
    }
}

static int decode_frame(AVCodecContext *avctx, void *data, int *got_frame,
                        AVPacket *avpkt)
{
    JvContext *s = avctx->priv_data;
    const uint8_t *buf_end = buf + avpkt->size;
    int video_size, video_type, i, j, ret;
        return AVERROR_INVALIDDATA;
    video_type = buf[4];

    if (video_size) {
            return AVERROR_INVALIDDATA;
        }
        if (video_type == 0 || video_type == 1) {
                return ret;
                return AVERROR_INVALIDDATA;
                              s->frame->data[0] + j * s->frame->linesize[0] + i,
            int v = *buf++;

            for (j = 0; j < avctx->height; j++)
                memset(s->frame->data[0] + j * s->frame->linesize[0],
            return AVERROR_INVALIDDATA;

        for (i = 0; i < AVPALETTE_COUNT; i++) {

    if (video_size) {
        s->frame->pict_type           = AV_PICTURE_TYPE_I;
        memcpy(s->frame->data[1], s->palette, AVPALETTE_SIZE);

        if ((ret = av_frame_ref(data, s->frame)) < 0)
    }

    return avpkt->size;
}

static av_cold int decode_close(AVCodecContext *avctx)
{
    JvContext *s = avctx->priv_data;

    av_frame_free(&s->frame);

    return 0;
}

AVCodec ff_jv_decoder = {
    .name           = "jv",
    .long_name      = NULL_IF_CONFIG_SMALL("Bitmap Brothers JV video"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_JV,
    .priv_data_size = sizeof(JvContext),
    .init           = decode_init,
    .close          = decode_close,
    .decode         = decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
