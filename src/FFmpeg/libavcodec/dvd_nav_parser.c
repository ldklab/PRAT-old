/*
 * DVD navigation block parser for FFmpeg
 * Copyright (c) 2013 The FFmpeg Project
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
#include "avcodec.h"
#include "get_bits.h"
#include "parser.h"

#define PCI_SIZE  980
#define DSI_SIZE 1018

/* parser definition */
typedef struct DVDNavParseContext {
    uint32_t     lba;
    uint8_t      buffer[PCI_SIZE+DSI_SIZE];
    int          copied;
} DVDNavParseContext;

{

}

                         AVCodecContext *avctx,
                         const uint8_t **poutbuf, int *poutbuf_size,
                         const uint8_t *buf, int buf_size)
{



                    /* PCI */


                    }
                }
                break;

                    /* DSI */

                    }
                }
                break;
        }

    }

    } else {
    }

}

AVCodecParser ff_dvd_nav_parser = {
    .codec_ids      = { AV_CODEC_ID_DVD_NAV },
    .priv_data_size = sizeof(DVDNavParseContext),
    .parser_init    = dvd_nav_parse_init,
    .parser_parse   = dvd_nav_parse,
};
