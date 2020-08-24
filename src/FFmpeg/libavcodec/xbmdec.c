/*
 * XBM image format
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

#include "libavutil/avstring.h"

#include "avcodec.h"
#include "internal.h"
#include "mathops.h"

{
    else
}

{

            break;
    }
        return INT_MIN;

    }
    return INT_MIN;
}

                            int *got_frame, AVPacket *avpkt)
{



        return ret;

        return ret;

    // goto start of image data
        next = memchr(ptr, '(', avpkt->size);
        return AVERROR_INVALIDDATA;

            uint8_t val;


                }
            } else {
                av_log(avctx, AV_LOG_ERROR,
                       "Unexpected data at %.8s.\n", ptr);
                return AVERROR_INVALIDDATA;
            }
        }
    }



}

AVCodec ff_xbm_decoder = {
    .name         = "xbm",
    .long_name    = NULL_IF_CONFIG_SMALL("XBM (X BitMap) image"),
    .type         = AVMEDIA_TYPE_VIDEO,
    .id           = AV_CODEC_ID_XBM,
    .decode       = xbm_decode_frame,
    .capabilities = AV_CODEC_CAP_DR1,
};
