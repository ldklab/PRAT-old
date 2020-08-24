/*
 * DNxHD/VC-3 parser
 * Copyright (c) 2008 Baptiste Coudurier <baptiste.coudurier@free.fr>
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
 * DNxHD/VC-3 parser
 */

#include "parser.h"
#include "dnxhddata.h"

typedef struct {
    ParseContext pc;
    int cur_byte;
    int remaining;
    int w, h;
} DNXHDParserContext;

                                const uint8_t *buf, int buf_size)
{

            }
        }
    }

            return 0;


                    continue;

                        continue;
                }
                } else {
                    // Update variables for correctness, they are currently not used beyond here
                }
            }
        }
        } else {

        }
    }
}

                       AVCodecContext *avctx,
                       const uint8_t **poutbuf, int *poutbuf_size,
                       const uint8_t *buf, int buf_size)
{

        next = buf_size;
    } else {
        }
    }
}

AVCodecParser ff_dnxhd_parser = {
    .codec_ids      = { AV_CODEC_ID_DNXHD },
    .priv_data_size = sizeof(DNXHDParserContext),
    .parser_parse   = dnxhd_parse,
    .parser_close   = ff_parse_close,
};
