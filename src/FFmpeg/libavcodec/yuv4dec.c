/*
 * libquicktime yuv4 decoder
 *
 * Copyright (c) 2011 Carl Eugen Hoyos
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

AVCodec ff_yuv4_decoder = {
    .name         = "yuv4",
    .long_name    = NULL_IF_CONFIG_SMALL("Uncompressed packed 4:2:0"),
    .type         = AVMEDIA_TYPE_VIDEO,
    .id           = AV_CODEC_ID_YUV4,
    .init         = yuv4_decode_init,
    .decode       = yuv4_decode_frame,
    .capabilities = AV_CODEC_CAP_DR1,
};
