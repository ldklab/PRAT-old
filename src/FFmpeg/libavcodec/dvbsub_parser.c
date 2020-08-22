/*
 * DVB subtitle parser for FFmpeg
 * Copyright (c) 2005 Ian Caulfield
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

#include <inttypes.h>
#include <string.h>

#include "libavutil/intreadwrite.h"
#include "libavutil/mem.h"

#include "avcodec.h"
#include "internal.h"

/* Parser (mostly) copied from dvdsub.c */

#define PARSE_BUF_SIZE  (65536)


/* parser definition */
typedef struct DVBSubParseContext {
    int packet_start;
    int packet_index;
    int in_packet;
    uint8_t packet_buf[PARSE_BUF_SIZE];
} DVBSubParseContext;

                        AVCodecContext *avctx,
                        const uint8_t **poutbuf, int *poutbuf_size,
                        const uint8_t *buf, int buf_size)
{

            s->pts, s->last_pts, s->cur_frame_pts[s->cur_frame_start_index]);

    {
        ff_dlog(avctx, "%02x ", buf[i]);
        if (i % 16 == 15)
            ff_dlog(avctx, "\n");
    }




    {
        {
                    pc->packet_index - pc->packet_start);
        }


            ff_dlog(avctx, "Bad packet header\n");
            return buf_size;
        }


    } else {
        {
            {

            } else {
                pc->packet_start = 0;
                pc->packet_index = 0;
            }
        }
    }

        return buf_size;

/* if not currently in a packet, pass data */
        return buf_size;



    {
        {
            {

                {

                } else
                    break;
            } else
                break;
            {
            }
        } else {
            av_log(avctx, AV_LOG_ERROR, "Junk in packet\n");

            pc->packet_index = p - pc->packet_buf;
            pc->in_packet = 0;
            break;
        }
    }

    {
    }


    return buf_size;
}

AVCodecParser ff_dvbsub_parser = {
    .codec_ids      = { AV_CODEC_ID_DVB_SUBTITLE },
    .priv_data_size = sizeof(DVBSubParseContext),
    .parser_parse   = dvbsub_parse,
};
