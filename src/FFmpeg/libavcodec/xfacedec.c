/*
 * Copyright (c) 1990 James Ashton - Sydney University
 * Copyright (c) 2012 Stefano Sabatini
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
 * X-Face decoder, based on libcompface, by James Ashton.
 */

#include "libavutil/pixdesc.h"
#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"
#include "xface.h"

{

    /* extract the last byte into r, and shift right b by 8 bits */

    }
}

{
    } else {
    }

{
    case XFACE_COLOR_WHITE:
        return;
    }
}

typedef struct XFaceContext {
    uint8_t bitmap[XFACE_PIXELS]; ///< image used internally for decoding
} XFaceContext;

{
            av_log(avctx, AV_LOG_ERROR,
                   "Size value %dx%d not supported, only accepts a size of %dx%d\n",
                   avctx->width, avctx->height, XFACE_WIDTH, XFACE_HEIGHT);
            return AVERROR(EINVAL);
        }
    }


}

                              void *data, int *got_frame,
                              AVPacket *avpkt)
{

        return ret;


        /* ignore invalid digits */

            av_log(avctx, AV_LOG_WARNING,
                   "Buffer is longer than expected, truncating at byte %d\n", i);
            break;
        }
    }

    /* decode image and put it in bitmap */


    /* convert image from 1=black 0=white bitmap to MONOWHITE */
        } else {
        }
        }
    }


}

AVCodec ff_xface_decoder = {
    .name           = "xface",
    .long_name      = NULL_IF_CONFIG_SMALL("X-face image"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_XFACE,
    .priv_data_size = sizeof(XFaceContext),
    .init           = xface_decode_init,
    .decode         = xface_decode_frame,
    .pix_fmts       = (const enum AVPixelFormat[]) { AV_PIX_FMT_MONOWHITE, AV_PIX_FMT_NONE },
};
