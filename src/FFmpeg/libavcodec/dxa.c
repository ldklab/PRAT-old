/*
 * Feeble Files/ScummVM DXA decoder
 * Copyright (c) 2007 Konstantin Shishkov
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
 * DXA Video decoder
 */

#include <stdio.h>
#include <stdlib.h>

#include "libavutil/common.h"
#include "libavutil/intreadwrite.h"
#include "bytestream.h"
#include "avcodec.h"
#include "internal.h"

#include <zlib.h>

/*
 * Decoder context
typedef struct DxaDecContext {
    int dsize;
    uint32_t pal[256];
static const int shift2[6] = { 0, 0, 8, 4, 0, 4 };

                     int stride, uint8_t *src, int srcsize, uint8_t *ref)
{
    uint8_t *src_end = src + srcsize;
    mv   = data + AV_RB32(src + 0);
    for(j = 0; j < avctx->height; j += 4){
            tmp  = dst + i;
            tmp2 = ref + i;
            type = *code++;
            switch(type){
                y = (*mv++) & 0xF; if(y & 8) y = 8 - y;
                if (i < -x || avctx->width  - i - 4 < x ||
                }
                tmp2 += x + y*stride;
            case 0: // skip
            case 5: // skip in method 12
                    tmp2 += stride;
            case 1:  // masked change
            case 10: // masked change with only half of pixels changed
            case 11: // cases 10-15 are for method 12 only
                }else{
                    msk++;
                    for(x = 0; x < 4; x++){
                    }
                for(y = 0; y < 4; y++){
                for(y = 0; y < 4; y++){
                    memcpy(tmp, data, 4);
                    data += 4;
                    d  = ((k & 1) << 1) + ((k & 2) * stride);
                    d2 = ((k & 1) << 1) + ((k & 2) * stride);
                    tmp2 = ref + i + d2;
                    switch(mask & 0xC0){
                        y = (*mv++) & 0xF; if(y & 8) y = 8 - y;
                        tmp2 += x + y*stride;
                        tmp[d + 0         ] = tmp2[0];
                        tmp[d + 1         ] = tmp2[1];
                        tmp[d + 1 + stride] = tmp2[1 + stride];
                        tmp[d + 1 + stride] = data[0];
                        break;
                break;
                    for(x = 0; x < 4; x++){
                        tmp[x] = data[mask & 1];
                        mask >>= 1;
                break;
            default:
                av_log(avctx, AV_LOG_ERROR, "Unknown opcode %d\n", type);
        }
        dst += stride * 4;
        ref += stride * 4;
    }

    DxaDecContext * const c = avctx->priv_data;
    uint8_t *outptr, *srcptr, *tmpptr;
    bytestream2_init(&gb, avpkt->data, avpkt->size);

    /* make the palette available on the way out */
    if (bytestream2_peek_le32(&gb) == MKTAG('C','M','A','P')) {
        bytestream2_skip(&gb, 4);
        for(i = 0; i < 256; i++){
            c->pal[i] = 0xFFU << 24 | bytestream2_get_be24(&gb);
        }
        pc = 1;
    }

    if ((ret = ff_get_buffer(avctx, frame, AV_GET_BUFFER_FLAG_REF)) < 0)
        return ret;
    memcpy(frame->data[1], c->pal, AVPALETTE_SIZE);
    frame->palette_has_changed = pc;

    outptr = frame->data[0];
    srcptr = c->decomp_buf;
    tmpptr = c->prev->data[0];
    stride = frame->linesize[0];

    if (bytestream2_get_le32(&gb) == MKTAG('N','U','L','L'))
        compr = -1;
    else
        compr = bytestream2_get_byte(&gb);

    dsize = c->dsize;
    if (compr != 4 && compr != -1) {
        bytestream2_skip(&gb, 4);
        if (uncompress(c->decomp_buf, &dsize, avpkt->data + bytestream2_tell(&gb),
                       bytestream2_get_bytes_left(&gb)) != Z_OK) {
            av_log(avctx, AV_LOG_ERROR, "Uncompress failed!\n");
            return AVERROR_UNKNOWN;
        }
        memset(c->decomp_buf + dsize, 0, DECOMP_BUF_PADDING);
    }

    if (avctx->debug & FF_DEBUG_PICT_INFO)
        av_log(avctx, AV_LOG_DEBUG, "compr:%2d, dsize:%d\n", compr, (int)dsize);

    switch(compr){
    case -1:
        frame->key_frame = 0;
        frame->pict_type = AV_PICTURE_TYPE_P;
        if (c->prev->data[0])
            memcpy(frame->data[0], c->prev->data[0], frame->linesize[0] * avctx->height);
        else{ // Should happen only when first frame is 'NULL'
            memset(frame->data[0], 0, frame->linesize[0] * avctx->height);
            frame->key_frame = 1;
            frame->pict_type = AV_PICTURE_TYPE_I;
        }
        break;
    case 2:
    case 4:
        frame->key_frame = 1;
        frame->pict_type = AV_PICTURE_TYPE_I;
        for (j = 0; j < avctx->height; j++) {
                memcpy(outptr, srcptr, avctx->width);
            outptr += stride;
            srcptr += avctx->width;
        }
        break;
    case 3:
    case 5:
        if (!tmpptr) {
            av_log(avctx, AV_LOG_ERROR, "Missing reference frame.\n");
            if (!(avctx->flags2 & AV_CODEC_FLAG2_SHOW_ALL))
                return AVERROR_INVALIDDATA;
        }
        frame->key_frame = 0;
        frame->pict_type = AV_PICTURE_TYPE_P;
        for (j = 0; j < avctx->height; j++) {
            if(tmpptr){
                for(i = 0; i < avctx->width; i++)
                    outptr[i] = srcptr[i] ^ tmpptr[i];
                tmpptr += stride;
            }else
                memcpy(outptr, srcptr, avctx->width);
            outptr += stride;
            srcptr += avctx->width;
        }
        break;
    case 12: // ScummVM coding
    case 13:
        frame->key_frame = 0;
        frame->pict_type = AV_PICTURE_TYPE_P;
        if (!c->prev->data[0]) {
            av_log(avctx, AV_LOG_ERROR, "Missing reference frame\n");
            return AVERROR_INVALIDDATA;
        }
        decode_13(avctx, c, frame->data[0], frame->linesize[0], srcptr, dsize, c->prev->data[0]);
        break;
    default:
        av_log(avctx, AV_LOG_ERROR, "Unknown/unsupported compression type %d\n", compr);
        return AVERROR_INVALIDDATA;
    }

    av_frame_unref(c->prev);
    if ((ret = av_frame_ref(c->prev, frame)) < 0)
        return ret;

    *got_frame = 1;

    /* always report that the buffer was completely consumed */
    return avpkt->size;
}

static av_cold int decode_init(AVCodecContext *avctx)
{
    DxaDecContext * const c = avctx->priv_data;

    if (avctx->width%4 || avctx->height%4) {
        avpriv_request_sample(avctx, "dimensions are not a multiple of 4");
        return AVERROR_INVALIDDATA;
    }

    c->prev = av_frame_alloc();
    if (!c->prev)
        return AVERROR(ENOMEM);

    avctx->pix_fmt = AV_PIX_FMT_PAL8;

    c->dsize = avctx->width * avctx->height * 2;
    c->decomp_buf = av_malloc(c->dsize + DECOMP_BUF_PADDING);
    if (!c->decomp_buf) {
        av_frame_free(&c->prev);
        av_log(avctx, AV_LOG_ERROR, "Can't allocate decompression buffer.\n");
        return AVERROR(ENOMEM);
    }

    return 0;
}

static av_cold int decode_end(AVCodecContext *avctx)
{
    DxaDecContext * const c = avctx->priv_data;

    av_freep(&c->decomp_buf);
    av_frame_free(&c->prev);

    return 0;
}

AVCodec ff_dxa_decoder = {
    .name           = "dxa",
    .long_name      = NULL_IF_CONFIG_SMALL("Feeble Files/ScummVM DXA"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_DXA,
    .priv_data_size = sizeof(DxaDecContext),
    .init           = decode_init,
    .close          = decode_end,
    .decode         = decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
