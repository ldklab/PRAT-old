/*
 * V.Flash PTX (.ptx) image decoder
 * Copyright (c) 2007 Ivo van Poorten
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

#include "libavutil/common.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/imgutils.h"
#include "avcodec.h"
#include "internal.h"

                            AVPacket *avpkt) {

        return AVERROR_INVALIDDATA;

        avpriv_request_sample(avctx, "Image format not RGB15");
        return AVERROR_PATCHWELCOME;
    }


        return AVERROR_INVALIDDATA;
        avpriv_request_sample(avctx, "offset != 0x2c");


        return AVERROR_INVALIDDATA;

        return ret;

        return ret;



    }


        av_log(avctx, AV_LOG_WARNING, "incomplete packet\n");
        return avpkt->size;
    }

}

AVCodec ff_ptx_decoder = {
    .name           = "ptx",
    .long_name      = NULL_IF_CONFIG_SMALL("V.Flash PTX image"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_PTX,
    .decode         = ptx_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
