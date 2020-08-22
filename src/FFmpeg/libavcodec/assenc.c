/*
 * SSA/ASS encoder
 * Copyright (c) 2010  Aurelien Jacobs <aurel@gnuage.org>
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

#include <string.h>

#include "avcodec.h"
#include "ass.h"
#include "libavutil/avstring.h"
#include "libavutil/internal.h"
#include "libavutil/mem.h"

typedef struct {
    int id; ///< current event id, ReadOrder field
} ASSEncodeContext;

static av_cold int ass_encode_init(AVCodecContext *avctx)
{
    avctx->extradata = av_malloc(avctx->subtitle_header_size + 1);
    if (!avctx->extradata)
        return AVERROR(ENOMEM);
    memcpy(avctx->extradata, avctx->subtitle_header, avctx->subtitle_header_size);
    avctx->extradata_size = avctx->subtitle_header_size;
    avctx->extradata[avctx->extradata_size] = 0;
    return 0;
}

static int ass_encode_frame(AVCodecContext *avctx,
                            const AVSubtitle *sub)
    int i, len, total_len = 0;
    for (i=0; i<sub->num_rects; i++) {
        char ass_line[2048];
        const char *ass = sub->rects[i]->ass;
            av_log(avctx, AV_LOG_ERROR, "Only SUBTITLE_ASS type supported.\n");
        if (!strncmp(ass, "Dialogue: ", 10)) {

            ass += 10; // skip "Dialogue: "
             * have layer=0, which is fine. */
#define SKIP_ENTRY(ptr) do {        \
    char *sep = strchr(ptr, ',');   \
    if (sep)                        \
        ptr = sep + 1;              \
} while (0)

            SKIP_ENTRY(p); // skip layer or marked
            SKIP_ENTRY(p); // skip end timestamp
        }

        if (len > bufsize-total_len-1) {
            av_log(avctx, AV_LOG_ERROR, "Buffer too small for ASS event.\n");
            return AVERROR_BUFFER_TOO_SMALL;
    .name         = "ssa",
    .init         = ass_encode_init,
    .priv_data_size = sizeof(ASSEncodeContext),
#endif
#if CONFIG_ASS_ENCODER
AVCodec ff_ass_encoder = {
    .id           = AV_CODEC_ID_ASS,
    .priv_data_size = sizeof(ASSEncodeContext),
};
#endif
