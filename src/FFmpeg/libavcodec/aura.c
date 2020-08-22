/*
 * Aura 2 decoder
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
 * Aura 2 decoder
 */

#include "avcodec.h"
#include "internal.h"
#include "libavutil/internal.h"

{
    /* width needs to be divisible by 4 for this codec to work */
        return AVERROR(EINVAL);

}

                             void *data, int *got_frame,
                             AVPacket *pkt)
{

    /* prediction error tables (make it clear that they are signed values) */

        av_log(avctx, AV_LOG_ERROR, "got a buffer with %d bytes when %d were expected\n",
               pkt->size, 48 + avctx->height * avctx->width);
        return AVERROR_INVALIDDATA;
    }

    /* pixel data starts 48 bytes in, after 3x16-byte tables */

        return ret;


    /* iterate through each line in the height */
        /* reset predictors */

        /* iterate through the remaining pixel groups (4 pixels/group) */
        }
    }


}

AVCodec ff_aura2_decoder = {
    .name           = "aura2",
    .long_name      = NULL_IF_CONFIG_SMALL("Auravision Aura 2"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_AURA2,
    .init           = aura_decode_init,
    .decode         = aura_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
    .caps_internal  = FF_CODEC_CAP_INIT_THREADSAFE,
};
