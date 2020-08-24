/*
 * PNG parser
 * Copyright (c) 2009 Peter Holik
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
 * PNG parser
 */

#include "parser.h"
#include "png.h"

typedef struct PNGParseContext {
    ParseContext pc;
    uint32_t chunk_pos;           ///< position inside current chunk
    uint32_t chunk_length;        ///< length of the current chunk
    uint32_t remaining_size;      ///< remaining size of the current chunk
} PNGParseContext;

                     const uint8_t **poutbuf, int *poutbuf_size,
                     const uint8_t *buf, int buf_size)
{



            }
        }
            next = i;
            goto flush;
        }
    }

                ppc->chunk_pos = ppc->pc.frame_start_found = 0;
                goto flush;
            }
                    ppc->chunk_pos = -1;
                else
                break;
            } else {
                    break;
                else
            }
        }
    }



}

AVCodecParser ff_png_parser = {
    .codec_ids      = { AV_CODEC_ID_PNG },
    .priv_data_size = sizeof(PNGParseContext),
    .parser_parse   = png_parse,
    .parser_close   = ff_parse_close,
};
