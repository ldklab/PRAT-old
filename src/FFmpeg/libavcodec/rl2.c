/*
 * RL2 Video Decoder
 * Copyright (C) 2008 Sascha Sommer (saschasommer@freenet.de)
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
 * RL2 Video Decoder
 * @author Sascha Sommer (saschasommer@freenet.de)
 * @see http://wiki.multimedia.cx/index.php?title=RL2
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libavutil/internal.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/mem.h"
#include "avcodec.h"
#include "internal.h"


#define EXTRADATA1_SIZE (6 + 256 * 3) ///< video base, clr count, palette

typedef struct Rl2Context {
    AVCodecContext *avctx;

    uint16_t video_base;  ///< initial drawing offset
    uint32_t clr_count;   ///< number of used colors (currently unused)
    uint8_t *back_frame;  ///< background frame
    uint32_t palette[AVPALETTE_COUNT];
} Rl2Context;

/**
 * Run Length Decode a single 320x200 frame
 * @param s rl2 context
 * @param in input buffer
 * @param size input buffer size
 * @param out output buffer
 * @param stride stride of the output buffer
 */
static void rl2_rle_decode(Rl2Context *s, const uint8_t *in, int size,
{
    int base_x = video_base % s->avctx->width;
    int stride_adj = stride - s->avctx->width;
    const uint8_t *back_frame = s->back_frame;
    const uint8_t *in_end     = in + size;
    const uint8_t *out_end    = out + stride * s->avctx->height;
    uint8_t *line_end;

    /** copy start of the background frame */
    for (i = 0; i <= base_y; i++) {
        if (s->back_frame)
            memcpy(out, back_frame, s->avctx->width);
        back_frame += s->avctx->width;
            break;

        while (len--) {
            back_frame++;
            if (out == line_end) {
        }
    }
    if (s->back_frame) {
        while (out < out_end) {
    }
}

 * @param avctx decoder context
static av_cold int rl2_decode_init(AVCodecContext *avctx)
{
    Rl2Context *s = avctx->priv_data;
    int ret;

    s->avctx       = avctx;
    avctx->pix_fmt = AV_PIX_FMT_PAL8;
    if (ret < 0)
    /** parse extra data */
    s->clr_count  = AV_RL32(&avctx->extradata[2]);
    if (s->video_base >= avctx->width * avctx->height) {
        return AVERROR_INVALIDDATA;
    }
    /** initialize palette */
    /** decode background frame if present */

    if (back_size > 0) {
        uint8_t *back_frame = av_mallocz(avctx->width*avctx->height);
        if (!back_frame)
            return AVERROR(ENOMEM);
        rl2_rle_decode(s, avctx->extradata + EXTRADATA1_SIZE, back_size,
                       back_frame, avctx->width, 0);
    return 0;
}

static int rl2_decode_frame(AVCodecContext *avctx,
{
    AVFrame *frame     = data;
    const uint8_t *buf = avpkt->data;
    Rl2Context *s = avctx->priv_data;
        return ret;

    /** run length decode */
                   s->video_base);

    /** make the palette available on the way out */
    *got_frame = 1;

    /** report that the buffer was completely consumed */
    return buf_size;

 * Uninit decoder
static av_cold int rl2_decode_end(AVCodecContext *avctx)
{
    return 0;


AVCodec ff_rl2_decoder = {
    .name           = "rl2",
    .long_name      = NULL_IF_CONFIG_SMALL("RL2 video"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_RL2,
    .priv_data_size = sizeof(Rl2Context),
    .close          = rl2_decode_end,
    .decode         = rl2_decode_frame,
