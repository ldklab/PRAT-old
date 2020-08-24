/*
 * BMP parser
 * Copyright (c) 2012 Paul B Mahol
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
 * BMP parser
 */

#include "libavutil/bswap.h"
#include "libavutil/common.h"

#include "parser.h"

typedef struct BMPParseContext {
    ParseContext pc;
    uint32_t fsize;
    uint32_t remaining_size;
} BMPParseContext;

                     const uint8_t **poutbuf, int *poutbuf_size,
                     const uint8_t *buf, int buf_size)
{


restart:
                }
//                 unsigned hsize = av_bswap32(state>>32);
                    bpc->pc.frame_start_found = 0;
                    continue;
                }

                } else {
                }
        }
    } else {

        }
    }

flush:

        bpc->pc.frame_start_found = FFMAX(bpc->pc.frame_start_found - i - 1, 0);
    else

}

AVCodecParser ff_bmp_parser = {
    .codec_ids      = { AV_CODEC_ID_BMP },
    .priv_data_size = sizeof(BMPParseContext),
    .parser_parse   = bmp_parse,
    .parser_close   = ff_parse_close,
};
