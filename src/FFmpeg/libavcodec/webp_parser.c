/*
 * WebP parser
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
 * WebP parser
 */

#include "libavutil/bswap.h"
#include "libavutil/common.h"

#include "parser.h"

typedef struct WebPParseContext {
    ParseContext pc;
    uint32_t fsize;
    uint32_t remaining_size;
} WebPParseContext;

                      const uint8_t **poutbuf, int *poutbuf_size,
                      const uint8_t *buf, int buf_size)
{


restart:
                    }
                }
                    ctx->pc.frame_start_found = 0;
                    continue;
                }
                    next = i - 15;
                    state = 0;
                    break;
                } else {
                }
        }
    } else {

        }
    }

flush:

        ctx->pc.frame_start_found = FFMAX(ctx->pc.frame_start_found - i - 1, 0);
    else


}

AVCodecParser ff_webp_parser = {
    .codec_ids      = { AV_CODEC_ID_WEBP },
    .priv_data_size = sizeof(WebPParseContext),
    .parser_parse   = webp_parse,
    .parser_close   = ff_parse_close,
};
