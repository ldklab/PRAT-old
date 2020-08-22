/*
 * v410 decoder
 *
 * Copyright (c) 2011 Derek Buitenhuis
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
#include "avcodec.h"
#include "internal.h"
#include "thread.h"

typedef struct ThreadData {
    AVFrame *frame;
    uint8_t *buf;
    int stride;
} ThreadData;

{

        if (avctx->err_recognition & AV_EF_EXPLODE) {
            av_log(avctx, AV_LOG_ERROR, "v410 requires width to be even.\n");
            return AVERROR_INVALIDDATA;
        } else {
            av_log(avctx, AV_LOG_WARNING, "v410 requires width to be even, continuing anyway.\n");
        }
    }

    return 0;
}

{




        }

    }

}

                             int *got_frame, AVPacket *avpkt)
{

        av_log(avctx, AV_LOG_ERROR, "Insufficient input data.\n");
        return AVERROR(EINVAL);
    }

        return ret;




}

AVCodec ff_v410_decoder = {
    .name         = "v410",
    .long_name    = NULL_IF_CONFIG_SMALL("Uncompressed 4:4:4 10-bit"),
    .type         = AVMEDIA_TYPE_VIDEO,
    .id           = AV_CODEC_ID_V410,
    .init         = v410_decode_init,
    .decode       = v410_decode_frame,
    .capabilities = AV_CODEC_CAP_DR1 | AV_CODEC_CAP_SLICE_THREADS |
                    AV_CODEC_CAP_FRAME_THREADS
};
