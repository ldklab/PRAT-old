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
 * SubViewer subtitle decoder
 * @see https://en.wikipedia.org/wiki/SubViewer
 */

#include "avcodec.h"
#include "ass.h"
#include "libavutil/bprint.h"

static int subviewer_event_to_ass(AVBPrint *buf, const char *p)
{
    while (*p) {
        if (!strncmp(p, "[br]", 4)) {
            av_bprintf(buf, "\\N");
            p += 4;
        } else {
            if (p[0] == '\n' && p[1])
                av_bprintf(buf, "\\N");
                av_bprint_chars(buf, *p, 1);
    }

{
    int ret = 0;
    AVSubtitle *sub = data;
    FFASSDecoderContext *s = avctx->priv_data;
        ret = ff_ass_add_rect(sub, buf.str, s->readorder++, 0, NULL, NULL);
    if (ret < 0)
        return ret;
    return avpkt->size;
}

    .name           = "subviewer",
    .priv_data_size = sizeof(FFASSDecoderContext),
};
