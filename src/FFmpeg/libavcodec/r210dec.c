/*
 * R210 decoder
 *
 * Copyright (c) 2009 Reimar Doeffinger <Reimar.Doeffinger@gmx.de>
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
#include "libavutil/bswap.h"
#include "libavutil/common.h"

{

}

                        AVPacket *avpkt)
{
                                avctx->codec_id == AV_CODEC_ID_R10K ? 1 : 64);
             !avctx->extradata[11];

        av_log(avctx, AV_LOG_ERROR, "packet too small\n");
        return AVERROR_INVALIDDATA;
    }

        return ret;


        uint16_t *dstg = (uint16_t *)g_line;
        uint16_t *dstb = (uint16_t *)b_line;
        uint16_t *dstr = (uint16_t *)r_line;
                pixel = av_le2ne32(*src++);
            } else {
            }
            } else if (r10) {
                r =  pixel & 0x3ff;
                g = (pixel >> 10) & 0x3ff;
                b = (pixel >> 20) & 0x3ff;
            } else {
                b = (pixel >>  2) & 0x3ff;
                g = (pixel >> 12) & 0x3ff;
                r = (pixel >> 22) & 0x3ff;
            }
        }
    }


}

#if CONFIG_R210_DECODER
AVCodec ff_r210_decoder = {
    .name           = "r210",
    .long_name      = NULL_IF_CONFIG_SMALL("Uncompressed RGB 10-bit"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_R210,
    .init           = decode_init,
    .decode         = decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
    .caps_internal  = FF_CODEC_CAP_INIT_THREADSAFE,
};
#endif
#if CONFIG_R10K_DECODER
AVCodec ff_r10k_decoder = {
    .name           = "r10k",
    .long_name      = NULL_IF_CONFIG_SMALL("AJA Kona 10-bit RGB Codec"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_R10K,
    .init           = decode_init,
    .decode         = decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
    .caps_internal  = FF_CODEC_CAP_INIT_THREADSAFE,
};
#endif
#if CONFIG_AVRP_DECODER
AVCodec ff_avrp_decoder = {
    .name           = "avrp",
    .long_name      = NULL_IF_CONFIG_SMALL("Avid 1:1 10-bit RGB Packer"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_AVRP,
    .init           = decode_init,
    .decode         = decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
    .caps_internal  = FF_CODEC_CAP_INIT_THREADSAFE,
};
#endif
