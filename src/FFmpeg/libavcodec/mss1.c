/*
 * Microsoft Screen 1 (aka Windows Media Video V7 Screen) decoder
 * Copyright (c) 2012 Konstantin Shishkov
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
 * Microsoft Screen 1 (aka Windows Media Video V7 Screen) decoder
 */

#include "avcodec.h"
#include "internal.h"
#include "mss12.h"

typedef struct MSS1Context {
    MSS12Context   ctx;
    AVFrame       *pic;
    SliceContext   sc;
} MSS1Context;

{
                } else {
                }
            } else {
            }
        }
    }
}


{



}

{



}

{



}


{
}

{

        return 0;

    }

}

                             AVPacket *avpkt)
{

        return ret;


        return ret;

    } else {
            return AVERROR_INVALIDDATA;
    }
                                        avctx->width, avctx->height);
        return AVERROR_INVALIDDATA;

        return ret;


    /* always report that the buffer was completely consumed */
}

{


        return AVERROR(ENOMEM);

        av_frame_free(&c->pic);


}

{


}

AVCodec ff_mss1_decoder = {
    .name           = "mss1",
    .long_name      = NULL_IF_CONFIG_SMALL("MS Screen 1"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_MSS1,
    .priv_data_size = sizeof(MSS1Context),
    .init           = mss1_decode_init,
    .close          = mss1_decode_end,
    .decode         = mss1_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
