/*
 * RV30/40 parser
 * Copyright (c) 2011 Konstantin Shishkov
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
 * RV30/40 parser
 */

#include "parser.h"
#include "libavutil/intreadwrite.h"

typedef struct RV34ParseContext {
    ParseContext pc;
    int64_t key_dts;
    int key_pts;
} RV34ParseContext;

static const int rv_to_av_frame_type[4] = {
    AV_PICTURE_TYPE_I, AV_PICTURE_TYPE_I, AV_PICTURE_TYPE_P, AV_PICTURE_TYPE_B,
};

                      AVCodecContext *avctx,
                      const uint8_t **poutbuf, int *poutbuf_size,
                      const uint8_t *buf, int buf_size)
{

    }

    } else {
    }

    } else {
        else
    }

}

#if CONFIG_RV30_PARSER
AVCodecParser ff_rv30_parser = {
    .codec_ids      = { AV_CODEC_ID_RV30 },
    .priv_data_size = sizeof(RV34ParseContext),
    .parser_parse   = rv34_parse,
};
#endif

#if CONFIG_RV40_PARSER
AVCodecParser ff_rv40_parser = {
    .codec_ids      = { AV_CODEC_ID_RV40 },
    .priv_data_size = sizeof(RV34ParseContext),
    .parser_parse   = rv34_parse,
};
#endif
