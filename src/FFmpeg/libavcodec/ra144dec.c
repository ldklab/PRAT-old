/*
 * Real Audio 1.0 (14.4K)
 *
 * Copyright (c) 2008 Vitor Sessak
 * Copyright (c) 2003 Nick Kurshev
 *     Based on public domain decoder at http://www.honeypot.net/audio
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

#include "libavutil/channel_layout.h"
#include "avcodec.h"
#include "get_bits.h"
#include "internal.h"
#include "ra144.h"


{




}

                               int gval, GetBitContext *gb)
{

                          gain);

/** Uncompress one block (20 bytes -> 160*2 bytes). */
                              int *got_frame_ptr, AVPacket *avpkt)
{


               "Frame too small (%d bytes). Truncated file?\n", buf_size);
    }

    /* get output buffer */
        return ret;








    }




}

AVCodec ff_ra_144_decoder = {
    .name           = "real_144",
    .long_name      = NULL_IF_CONFIG_SMALL("RealAudio 1.0 (14.4K)"),
    .type           = AVMEDIA_TYPE_AUDIO,
    .id             = AV_CODEC_ID_RA_144,
    .priv_data_size = sizeof(RA144Context),
    .init           = ra144_decode_init,
    .decode         = ra144_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
