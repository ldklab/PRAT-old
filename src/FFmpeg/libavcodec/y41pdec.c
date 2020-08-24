/*
 * y41p decoder
 *
 * Copyright (c) 2012 Paul B Mahol
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
#include "internal.h"

{

        av_log(avctx, AV_LOG_WARNING, "y41p requires width to be divisible by 8.\n");
    }

}

                             int *got_frame, AVPacket *avpkt)
{

        av_log(avctx, AV_LOG_ERROR, "Insufficient input data.\n");
        return AVERROR(EINVAL);
    }

        return ret;




        }
    }


}

AVCodec ff_y41p_decoder = {
    .name         = "y41p",
    .long_name    = NULL_IF_CONFIG_SMALL("Uncompressed YUV 4:1:1 12-bit"),
    .type         = AVMEDIA_TYPE_VIDEO,
    .id           = AV_CODEC_ID_Y41P,
    .init         = y41p_decode_init,
    .decode       = y41p_decode_frame,
    .capabilities = AV_CODEC_CAP_DR1,
};
