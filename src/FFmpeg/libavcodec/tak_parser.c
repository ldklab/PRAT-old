/*
 * TAK parser
 * Copyright (c) 2012 Michael Niedermayer
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
 * TAK parser
 **/

#define BITSTREAM_READER_LE
#include "parser.h"
#include "tak.h"

typedef struct TAKParseContext {
    ParseContext  pc;
    TAKStreamInfo ti;
    int           index;
} TAKParseContext;

                     const uint8_t **poutbuf, int *poutbuf_size,
                     const uint8_t *buf, int buf_size)
{


        TAKStreamInfo ti;
        if ((ret = init_get_bits8(&gb, buf, buf_size)) < 0)
            return buf_size;
        if (!ff_tak_decode_frame_header(avctx, &gb, &ti, 127))
            s->duration = t->ti.last_frame_samples ? t->ti.last_frame_samples
                                                   : t->ti.frame_samples;
        return buf_size;
    }

                                           buf_size);

                goto fail;
        }


                                          pc->index - t->index)) < 0)
                    goto fail;
                                                t->ti.frame_samples;
                    } else {
                    }
                }
            }
        }
    }

    }

    }


}

AVCodecParser ff_tak_parser = {
    .codec_ids      = { AV_CODEC_ID_TAK },
    .priv_data_size = sizeof(TAKParseContext),
    .parser_parse   = tak_parse,
    .parser_close   = ff_parse_close,
};
