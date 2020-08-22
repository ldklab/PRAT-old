/*
 * Miro VideoXL codec
 * Copyright (c) 2004 Konstantin Shishkov
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
 * Miro VideoXL codec.
 */

#include "libavutil/common.h"
#include "libavutil/intreadwrite.h"
#include "avcodec.h"
#include "internal.h"

static const int xl_table[32] = {
   0,   1,   2,   3,   4,   5,   6,   7,
   8,   9,  12,  15,  20,  25,  34,  46,
  64,  82,  94, 103, 108, 113, 116, 119,
 120, 121, 122, 123, 124, 125, 126, 127
};

                        void *data, int *got_frame,
                        AVPacket *avpkt)
{

        av_log(avctx, AV_LOG_ERROR, "width is not a multiple of 4\n");
        return AVERROR_INVALIDDATA;
    }
        av_log(avctx, AV_LOG_ERROR, "Packet is too small\n");
        return AVERROR_INVALIDDATA;
    }

        return ret;



        /* lines are stored in reversed order */

            /* value is stored in LE dword with word swapped */

            else
            else
            else


        }

    }


}

{

}

AVCodec ff_xl_decoder = {
    .name         = "xl",
    .long_name    = NULL_IF_CONFIG_SMALL("Miro VideoXL"),
    .type         = AVMEDIA_TYPE_VIDEO,
    .id           = AV_CODEC_ID_VIXL,
    .init         = decode_init,
    .decode       = decode_frame,
    .capabilities = AV_CODEC_CAP_DR1,
};
