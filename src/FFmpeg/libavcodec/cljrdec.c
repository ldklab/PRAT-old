/*
 * Cirrus Logic AccuPak (CLJR) decoder
 * Copyright (c) 2003 Alex Beregszaszi
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
 * Cirrus Logic AccuPak decoder.
 */

#include "avcodec.h"
#include "get_bits.h"
#include "internal.h"

                        void *data, int *got_frame,
                        AVPacket *avpkt)
{

        av_log(avctx, AV_LOG_ERROR, "Invalid width or height\n");
        return AVERROR_INVALIDDATA;
    }

               "Resolution larger than buffer size. Invalid header?\n");
    }

        return ret;


        }
    }


}

{
}

AVCodec ff_cljr_decoder = {
    .name           = "cljr",
    .long_name      = NULL_IF_CONFIG_SMALL("Cirrus Logic AccuPak"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_CLJR,
    .init           = decode_init,
    .decode         = decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};

