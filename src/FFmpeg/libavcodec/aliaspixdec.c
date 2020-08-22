/*
 * Alias PIX image decoder
 * Copyright (C) 2014 Vittorio Giovara <vittorio.giovara@gmail.com>
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

#include "libavutil/intreadwrite.h"

#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"

#define ALIAS_HEADER_SIZE 10

                        AVPacket *avpkt)
{


        av_log(avctx, AV_LOG_ERROR, "Header too small %d.\n", avpkt->size);
        return AVERROR_INVALIDDATA;
    }


    else {
        av_log(avctx, AV_LOG_ERROR, "Invalid pixel format.\n");
        return AVERROR_INVALIDDATA;
    }

        return ret;

        return AVERROR_INVALIDDATA;

        return ret;



        /* set buffer at the right position at every new line */
                av_log(avctx, AV_LOG_ERROR,
                       "Ended frame decoding with %d bytes left.\n",
                       bytestream2_get_bytes_left(&gb));
                return AVERROR_INVALIDDATA;
            }
        }

        /* read packet and copy data */
            av_log(avctx, AV_LOG_ERROR, "Invalid run length %d.\n", count);
            return AVERROR_INVALIDDATA;
        }

            }
        } else { // AV_PIX_FMT_GRAY8
        }

    }

        av_log(avctx, AV_LOG_ERROR, "Picture stopped at %d,%d.\n", x, y);
        return AVERROR_INVALIDDATA;
    }

}

AVCodec ff_alias_pix_decoder = {
    .name         = "alias_pix",
    .long_name    = NULL_IF_CONFIG_SMALL("Alias/Wavefront PIX image"),
    .type         = AVMEDIA_TYPE_VIDEO,
    .id           = AV_CODEC_ID_ALIAS_PIX,
    .decode       = decode_frame,
    .capabilities = AV_CODEC_CAP_DR1,
};
