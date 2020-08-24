/*
 * Packed Animation File audio decoder
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

#include "libavutil/intreadwrite.h"

#include "avcodec.h"
#include "internal.h"
#include "mathops.h"
#include "paf.h"

{
        av_log(avctx, AV_LOG_ERROR, "invalid number of channels\n");
        return AVERROR_INVALIDDATA;
    }


}

                            int *got_frame, AVPacket *pkt)
{

        return AVERROR_INVALIDDATA;

        return ret;

    // codebook of 256 16-bit samples and 8-bit indices to it
        // always 2 channels
    }

}

AVCodec ff_paf_audio_decoder = {
    .name         = "paf_audio",
    .long_name    = NULL_IF_CONFIG_SMALL("Amazing Studio Packed Animation File Audio"),
    .type         = AVMEDIA_TYPE_AUDIO,
    .id           = AV_CODEC_ID_PAF_AUDIO,
    .init         = paf_audio_init,
    .decode       = paf_audio_decode,
    .capabilities = AV_CODEC_CAP_DR1,
};
