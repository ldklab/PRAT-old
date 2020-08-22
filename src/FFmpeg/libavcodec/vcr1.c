/*
 * ATI VCR1 codec
 * Copyright (c) 2003 Michael Niedermayer
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
 * ATI VCR1 codec
 */

#include "avcodec.h"
#include "internal.h"
#include "libavutil/avassert.h"
#include "libavutil/internal.h"

typedef struct VCR1Context {
    int delta[16];
    int offset[4];
} VCR1Context;

{

        avpriv_request_sample(avctx, "odd dimensions (%d x %d) support", avctx->width, avctx->height);
        return AVERROR_INVALIDDATA;
    }

    return 0;
}

                             int *got_frame, AVPacket *avpkt)
{

        av_log(avctx, AV_LOG_ERROR, "Insufficient input data. %d < %d\n", avpkt->size ,  32 + avctx->height + avctx->width*avctx->height*5/8);
        return AVERROR(EINVAL);
    }

        return ret;

    }







            }
        } else {


            }
        }
    }


}

AVCodec ff_vcr1_decoder = {
    .name           = "vcr1",
    .long_name      = NULL_IF_CONFIG_SMALL("ATI VCR1"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_VCR1,
    .priv_data_size = sizeof(VCR1Context),
    .init           = vcr1_decode_init,
    .decode         = vcr1_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
