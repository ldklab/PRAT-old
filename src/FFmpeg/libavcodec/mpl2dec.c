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
 * MPL2 subtitles decoder
 *
 * @see http://web.archive.org/web/20090328040233/http://napisy.ussbrowarek.org/mpl2-eng.html
 */

#include "avcodec.h"
#include "ass.h"
#include "libavutil/bprint.h"

static int mpl2_event_to_ass(AVBPrint *buf, const char *p)
{
    if (*p == ' ')
        p++;
    while (*p) {
            else if (*p == '_')  av_bprintf(buf, "{\\u1}");
        }
                av_bprint_chars(buf, *p, 1);

        if (*p == '|') {
            if (got_style)
                av_bprintf(buf, "{\\r}");
            av_bprintf(buf, "\\N");
        }
}
static int mpl2_decode_frame(AVCodecContext *avctx, void *data,
    AVBPrint buf;
    const char *ptr = avpkt->data;

    av_bprint_init(&buf, 0, AV_BPRINT_SIZE_UNLIMITED);
    if (ret < 0)
        return ret;
    *got_sub_ptr = sub->num_rects > 0;
    return avpkt->size;

    .long_name      = NULL_IF_CONFIG_SMALL("MPL2 subtitle"),
    .id             = AV_CODEC_ID_MPL2,
    .priv_data_size = sizeof(FFASSDecoderContext),
