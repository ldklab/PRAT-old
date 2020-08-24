/*
 * Winnov WNV1 codec
 * Copyright (c) 2005 Konstantin Shishkov
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
 * Winnov WNV1 codec.
 */

#include "avcodec.h"
#include "get_bits.h"
#include "internal.h"
#include "mathops.h"


typedef struct WNV1Context {
    int shift;
    GetBitContext gb;
} WNV1Context;

static const uint16_t code_tab[16][2] = {
    { 0x1FD, 9 }, { 0xFD, 8 }, { 0x7D, 7 }, { 0x3D, 6 }, { 0x1D, 5 }, { 0x0D, 4 }, { 0x005, 3 },
    { 0x000, 1 },
    { 0x004, 3 }, { 0x0C, 4 }, { 0x1C, 5 }, { 0x3C, 6 }, { 0x7C, 7 }, { 0xFC, 8 }, { 0x1FC, 9 }, { 0xFF, 8 }
};

#define CODE_VLC_BITS 9
static VLC code_vlc;

/* returns modified base_value */
{

    else
}

                        void *data, int *got_frame,
                        AVPacket *avpkt)
{

        av_log(avctx, AV_LOG_ERROR, "Packet size %d is too small\n", buf_size);
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "Cannot allocate temporary buffer\n");
        return AVERROR(ENOMEM);
    }

        av_free(rbuf);
        return ret;
    }


        return ret;

    else {
        l->shift = 8 - (buf[2] >> 4);
        if (l->shift > 4) {
            avpriv_request_sample(avctx,
                                  "Unknown WNV1 frame header value %i",
                                  buf[2] >> 4);
            l->shift = 4;
        }
        if (l->shift < 1) {
            avpriv_request_sample(avctx,
                                  "Unknown WNV1 frame header value %i",
                                  buf[2] >> 4);
            l->shift = 1;
        }
    }

        }
    }



}

{


             &code_tab[0][1], 4, 2,
             &code_tab[0][0], 4, 2, INIT_VLC_USE_NEW_STATIC);

}

AVCodec ff_wnv1_decoder = {
    .name           = "wnv1",
    .long_name      = NULL_IF_CONFIG_SMALL("Winnov WNV1"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_WNV1,
    .priv_data_size = sizeof(WNV1Context),
    .init           = decode_init,
    .decode         = decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
