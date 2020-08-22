/*
 * 012v decoder
 *
 * Copyright (C) 2012 Carl Eugen Hoyos
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
#include "libavutil/intreadwrite.h"

{

        avpriv_request_sample(avctx, "transparency");

}

                                int *got_frame, AVPacket *avpkt)
{

        av_log(avctx, AV_LOG_ERROR, "Dimensions %dx%d not supported.\n", width, avctx->height);
        return AVERROR_INVALIDDATA;
    }


        av_log(avctx, AV_LOG_ERROR, "Packet too small: %d instead of %d\n",
               avpkt->size, avctx->height * stride);
        return AVERROR_INVALIDDATA;
    }

        return ret;




            }

                break;


                break;


                break;


                break;


                break;
        }

        }

    }


}

AVCodec ff_zero12v_decoder = {
    .name           = "012v",
    .long_name      = NULL_IF_CONFIG_SMALL("Uncompressed 4:2:2 10-bit"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_012V,
    .init           = zero12v_decode_init,
    .decode         = zero12v_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
