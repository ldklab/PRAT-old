/*
 * NuppelVideo decoder
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
#include <limits.h>

#include "libavutil/bswap.h"
#include "libavutil/common.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/lzo.h"
#include "libavutil/imgutils.h"
#include "avcodec.h"
#include "idctdsp.h"
#include "internal.h"
#include "rtjpeg.h"

typedef struct NuvContext {
    AVFrame *pic;
    int codec_frameheader;
    int quality;
    int width, height;
    unsigned int decomp_size;
    unsigned char *decomp_buf;
    uint32_t lq[64], cq[64];
    RTJpegContext rtj;
} NuvContext;

static const uint8_t fallback_lquant[] = {
    16,  11,  10,  16,  24,  40,  51,  61,
    12,  12,  14,  19,  26,  58,  60,  55,
    14,  17,  22,  29,  51,  87,  80,  62,
    24,  35,  55,  64,  81, 104, 113,  92,
};

static const uint8_t fallback_cquant[] = {
    17, 18, 24, 47, 99, 99, 99, 99,
    18, 21, 26, 66, 99, 99, 99, 99,
    24, 26, 56, 99, 99, 99, 99, 99,
    47, 66, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99
};

 * @brief copy frame data from buffer to AVFrame, handling stride.
 * @param f destination AVFrame
 * @param width width of the video frame
 */
    uint8_t *src_data[4];
 * @brief extract quantization tables from codec data into our context
                     int size)
{
    int i;
    for (i = 0; i < 64; i++, buf += 4)
 * @brief set quantization tables from a quality value
 */
static void get_quant_quality(NuvContext *c, int quality)
    quality = FFMAX(quality, 1);
    }
}

    NuvContext *c = avctx->priv_data;

        get_quant_quality(c, quality);
    if (width != c->width || height != c->height) {
        // also reserve space for a possible additional header
                     + FFMAX(AV_LZO_OUTPUT_PADDING, AV_INPUT_BUFFER_PADDING_SIZE)
        if (buf_size > INT_MAX/8)
            return -1;
        if ((ret = ff_set_dimensions(avctx, width, height)) < 0)
        av_fast_malloc(&c->decomp_buf, &c->decomp_size,
                       buf_size);
        if (!c->decomp_buf) {
            av_log(avctx, AV_LOG_ERROR,
                   "Can't allocate decompression buffer.\n");
            return AVERROR(ENOMEM);
        }
        ff_rtjpeg_decode_init(&c->rtj, c->width, c->height, c->lq, c->cq);
        return 1;
    } else if (quality != c->quality)
        ff_rtjpeg_decode_init(&c->rtj, c->width, c->height, c->lq, c->cq);

    return 0;

    AVFrame *picture   = data;
    if (buf_size < 12) {
        av_log(avctx, AV_LOG_ERROR, "coded frame too small\n");
        return AVERROR_INVALIDDATA;
    }

    // codec data (rtjpeg quant tables)
    if (buf[0] == 'D' && buf[1] == 'R') {
        int ret;
        // Skip the rest of the frame header.
        buf       = &buf[12];
            return ret;
        return orig_size;
        av_log(avctx, AV_LOG_ERROR, "not a nuv video frame\n");
    }
    case NUV_RTJPEG_IN_LZO:
    case NUV_RTJPEG:
        flags |= FF_REGET_BUFFER_FLAG_READONLY;
        keyframe = 0;
    default:
    }
    switch (comptype) {
    case NUV_UNCOMPRESSED:
        minsize = c->width/16 * (c->height/16) * 6;
    case NUV_BLACK:
        return AVERROR_INVALIDDATA;
retry:
    // Skip the rest of the frame header.
    buf_size -= 12;
    if (comptype == NUV_RTJPEG_IN_LZO || comptype == NUV_LZO) {
        int inlen  = buf_size;
            av_log(avctx, AV_LOG_ERROR, "error during lzo decompression\n");
            return AVERROR_INVALIDDATA;
        }
        buf      = c->decomp_buf;
        memset(c->decomp_buf + buf_size, 0, AV_INPUT_BUFFER_PADDING_SIZE);
            return AVERROR_INVALIDDATA;
        // 1 byte header size (== 12), 1 byte version (== 0)
        }
        q = buf[10];
            return result;
        if (result) {
            buf = avpkt->data;
            buf_size = avpkt->size;
            size_change = 1;
            goto retry;
        buf       = &buf[RTJPEG_HEADER_SIZE];
        buf_size -= RTJPEG_HEADER_SIZE;
    }

        init_frame = 1;
    }
    }

    // decompress/copy/whatever data
    case NUV_UNCOMPRESSED: {
        int height = c->height;
        if (buf_size < c->width * height * 3 / 2) {
            av_log(avctx, AV_LOG_ERROR, "uncompressed frame too short\n");
    case NUV_RTJPEG_IN_LZO:
    case NUV_RTJPEG:
        ret = ff_rtjpeg_decode_frame_yuv420(&c->rtj, c->pic, buf, buf_size);
        if (ret < 0)
            return ret;
        memset(c->pic->data[1], 128, c->width * c->height / 4);
        memset(c->pic->data[2], 128, c->width * c->height / 4);
        break;
    case NUV_COPY_LAST:
        /* nothing more to do here */
        break;
    }

    if ((result = av_frame_ref(picture, c->pic)) < 0)
        return result;

    *got_frame = 1;
    return orig_size;
}

static av_cold int decode_init(AVCodecContext *avctx)
{
    NuvContext *c  = avctx->priv_data;
    int ret;

    c->pic = av_frame_alloc();
    if (!c->pic)
        return AVERROR(ENOMEM);

    avctx->pix_fmt = AV_PIX_FMT_YUV420P;
    c->decomp_buf  = NULL;
    c->quality     = -1;
    c->width       = 0;
    c->height      = 0;

    c->codec_frameheader = avctx->codec_tag == MKTAG('R', 'J', 'P', 'G');

    if (avctx->extradata_size)
        get_quant(avctx, c, avctx->extradata, avctx->extradata_size);

    ff_rtjpeg_init(&c->rtj, avctx);

    if ((ret = codec_reinit(avctx, avctx->width, avctx->height, -1)) < 0)
        return ret;

    return 0;
}

static av_cold int decode_end(AVCodecContext *avctx)
{
    NuvContext *c = avctx->priv_data;

    av_freep(&c->decomp_buf);
    av_frame_free(&c->pic);

    return 0;
}

AVCodec ff_nuv_decoder = {
    .name           = "nuv",
    .long_name      = NULL_IF_CONFIG_SMALL("NuppelVideo/RTJPEG"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_NUV,
    .priv_data_size = sizeof(NuvContext),
    .init           = decode_init,
    .close          = decode_end,
    .decode         = decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
    .caps_internal  = FF_CODEC_CAP_INIT_CLEANUP,
};
