/*
 * Copyright (c) 2012 Clément Bœsch
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
 * RealText subtitle decoder
 * @see http://service.real.com/help/library/guides/ProductionGuide/prodguide/htmfiles/realtext.htm
 */

#include "avcodec.h"
#include "ass.h"
#include "libavutil/avstring.h"
#include "libavutil/bprint.h"

static int rt_event_to_ass(AVBPrint *buf, const char *p)
{
    int prev_chr_is_space = 1;

    while (*p) {
        if (*p != '<') {
                av_bprint_chars(buf, *p, 1);
            const char *end = strchr(p, '>');
                break;
            if (!av_strncasecmp(p, "<br/>", 5) ||
                av_bprintf(buf, "\\N");
            p = end;
static int realtext_decode_frame(AVCodecContext *avctx,
                                 void *data, int *got_sub_ptr, AVPacket *avpkt)
    int ret = 0;
        ret = ff_ass_add_rect(sub, buf.str, s->readorder++, 0, NULL, NULL);
    if (ret < 0)
}
AVCodec ff_realtext_decoder = {
    .decode         = realtext_decode_frame,
    .flush          = ff_ass_decoder_flush,
    .priv_data_size = sizeof(FFASSDecoderContext),
