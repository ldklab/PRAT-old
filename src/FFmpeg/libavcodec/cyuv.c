/*
 * Creative YUV (CYUV) Video Decoder
 *   by Mike Melanson (melanson@pcisys.net)
 * based on "Creative YUV (CYUV) stream format for AVI":
 *   http://www.csse.monash.edu.au/~timf/videocodec/cyuv.txt
 *
 * Copyright (C) 2003 The FFmpeg project
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
 * Creative YUV (CYUV) Video Decoder.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "avcodec.h"
#include "internal.h"
#include "libavutil/internal.h"


typedef struct CyuvDecodeContext {
    AVCodecContext *avctx;
    int width, height;
} CyuvDecodeContext;

{

    /* width needs to be divisible by 4 for this codec to work */
        return AVERROR_INVALIDDATA;

}

                             void *data, int *got_frame,
                             AVPacket *avpkt)
{


    /* prediction error tables (make it clear that they are signed values) */


    }
    /* sanity check the buffer size: A buffer has 3x16-bytes tables
     * followed by (height) lines each with 3 bytes to represent groups
     * of 4 pixels. Thus, the total size of the buffer ought to be:
     *    (3 * 16) + height * (width * 3 / 4) */
        avctx->pix_fmt = AV_PIX_FMT_UYVY422;
    } else {
               buf_size, 48 + s->height * (s->width * 3 / 4));
    }

    /* pixel data starts 48 bytes in, after 3x16-byte tables */

        return ret;


        int linesize = FFALIGN(s->width,2) * 2;
        y_plane += frame->linesize[0] * s->height;
        for (stream_ptr = 0; stream_ptr < rawsize; stream_ptr += linesize) {
            y_plane -= frame->linesize[0];
            memcpy(y_plane, buf+stream_ptr, linesize);
        }
    } else {

    /* iterate through each line in the height */

        /* reset predictors */



        /* iterate through the remaining pixel groups (4 pixels/group) */




        }
    }
    }


}

#if CONFIG_AURA_DECODER
AVCodec ff_aura_decoder = {
    .name           = "aura",
    .long_name      = NULL_IF_CONFIG_SMALL("Auravision AURA"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_AURA,
    .priv_data_size = sizeof(CyuvDecodeContext),
    .init           = cyuv_decode_init,
    .decode         = cyuv_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
    .caps_internal  = FF_CODEC_CAP_INIT_THREADSAFE,
};
#endif

#if CONFIG_CYUV_DECODER
AVCodec ff_cyuv_decoder = {
    .name           = "cyuv",
    .long_name      = NULL_IF_CONFIG_SMALL("Creative YUV (CYUV)"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_CYUV,
    .priv_data_size = sizeof(CyuvDecodeContext),
    .init           = cyuv_decode_init,
    .decode         = cyuv_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
    .caps_internal  = FF_CODEC_CAP_INIT_THREADSAFE,
};
#endif
