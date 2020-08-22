/*
 * DVD subtitle decoding
 * Copyright (c) 2005 Fabrice Bellard
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

#include "libavutil/intreadwrite.h"
#include "libavutil/mem.h"
#include "avcodec.h"

/* parser definition */
typedef struct DVDSubParseContext {
    uint8_t *packet;
    int packet_len;
    int packet_index;
} DVDSubParseContext;

                        AVCodecContext *avctx,
                        const uint8_t **poutbuf, int *poutbuf_size,
                        const uint8_t *buf, int buf_size)
{


                av_log(avctx, AV_LOG_DEBUG, "Parser input %d too small\n", buf_size);
        }
            pc->packet_len = AV_RB32(buf+2);
            av_log(avctx, AV_LOG_ERROR, "packet length %d is invalid\n", pc->packet_len);
            return buf_size;
        }
    }
            }
        } else {
            /* erroneous size */
            pc->packet_index = 0;
        }
    }
}

{

AVCodecParser ff_dvdsub_parser = {
    .codec_ids      = { AV_CODEC_ID_DVD_SUBTITLE },
    .priv_data_size = sizeof(DVDSubParseContext),
    .parser_parse   = dvdsub_parse,
    .parser_close   = dvdsub_parse_close,
};
