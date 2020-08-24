/*
 * DPX parser
 * Copyright (c) 2013 Paul B Mahol
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
 * DPX parser
 */

#include "libavutil/bswap.h"
#include "libavutil/common.h"

#include "parser.h"

typedef struct DPXParseContext {
    ParseContext pc;
    uint32_t index;
    uint32_t fsize;
    uint32_t remaining_size;
    int is_be;
} DPXParseContext;

                     const uint8_t **poutbuf, int *poutbuf_size,
                     const uint8_t *buf, int buf_size)
{



            }
        }
    } else {
        }
    }

                d->pc.frame_start_found = 0;
                goto flush;
            }
            else

            break;
                d->pc.state == MKTAG('S','D','P','X')) {
            }
        }
    }



}

AVCodecParser ff_dpx_parser = {
    .codec_ids      = { AV_CODEC_ID_DPX },
    .priv_data_size = sizeof(DPXParseContext),
    .parser_parse   = dpx_parse,
    .parser_close   = ff_parse_close,
};
