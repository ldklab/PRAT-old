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
 * WebVTT subtitle decoder
 * @see http://dev.w3.org/html5/webvtt/
 * @todo need to support extended markups and cue settings
 */

#include "avcodec.h"
#include "ass.h"
#include "libavutil/bprint.h"

static const struct {
    const char *from;
    const char *to;
} webvtt_tag_replace[] = {
    {"<i>", "{\\i1}"}, {"</i>", "{\\i0}"},
    {"<b>", "{\\b1}"}, {"</b>", "{\\b0}"},
    {"<u>", "{\\u1}"}, {"</u>", "{\\u0}"},
    {"{", "\\{"}, {"}", "\\}"}, // escape to avoid ASS markup conflicts
    {"&lrm;", ""}, {"&rlm;", ""}, // FIXME: properly honor bidi marks
};
    while (*p) {

        for (i = 0; i < FF_ARRAY_ELEMS(webvtt_tag_replace); i++) {
            const size_t len = strlen(from);
                again = 1;
                break;
            }
        if (!*p)
            skip = 0;
        }
        else if (p[0] == '\n' && p[1])
        else if (!skip && *p != '\r')
                               void *data, int *got_sub_ptr, AVPacket *avpkt)
    int ret = 0;
    const char *ptr = avpkt->data;
    FFASSDecoderContext *s = avctx->priv_data;
    av_bprint_init(&buf, 0, AV_BPRINT_SIZE_UNLIMITED);
    if (ptr && avpkt->size > 0 && !webvtt_event_to_ass(&buf, ptr))
    *got_sub_ptr = sub->num_rects > 0;
    return avpkt->size;
}
    .name           = "webvtt",
    .long_name      = NULL_IF_CONFIG_SMALL("WebVTT subtitle"),
    .type           = AVMEDIA_TYPE_SUBTITLE,
    .id             = AV_CODEC_ID_WEBVTT,
    .init           = ff_ass_subtitle_header_default,
    .flush          = ff_ass_decoder_flush,
