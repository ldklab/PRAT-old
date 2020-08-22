/*
 * Screenpresso decoder
 * Copyright (C) 2015 Vittorio Giovara <vittorio.giovara@gmail.com>
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
 * Screenpresso decoder
 *
 * Fourcc: SPV1
 *
 * Screenpresso simply horizontally flips and then deflates frames,
 * alternating full pictures and deltas. Deltas are related to the currently
 * rebuilt frame (not the reference), and since there is no coordinate system
 * they contain exactly as many pixel as the keyframe.
 *
 * Supports: BGR0, BGR24, RGB555
 */

#include <stdint.h>
#include <string.h>
#include <zlib.h>

#include "libavutil/imgutils.h"
#include "libavutil/internal.h"
#include "libavutil/mem.h"

#include "avcodec.h"
#include "internal.h"

typedef struct ScreenpressoContext {
    AVFrame *current;

    /* zlib interaction */
    uint8_t *inflated_buf;
    uLongf inflated_size;
} ScreenpressoContext;

{


}

{

    /* These needs to be set to estimate uncompressed buffer */
        av_log(avctx, AV_LOG_ERROR, "Invalid image size %dx%d.\n",
               avctx->width, avctx->height);
        return ret;
    }

    /* Allocate current frame */
        return AVERROR(ENOMEM);

    /* Allocate maximum size possible, a full RGBA frame */
        return AVERROR(ENOMEM);

    return 0;
}

                              const uint8_t *src, int src_linesize,
                              int bytewidth, int height)
{
    }
}

                                     int *got_frame, AVPacket *avpkt)
{

    /* Size check */
        av_log(avctx, AV_LOG_ERROR, "Packet too small (%d)\n", avpkt->size);
        return AVERROR_INVALIDDATA;
    }

    /* Compression level (4 bits) and keyframe information (1 bit) */

    /* Pixel size */
    default:
        av_log(avctx, AV_LOG_ERROR, "Invalid bits per pixel value (%d)\n",
               component_size);
        return AVERROR_INVALIDDATA;
    }

    /* Inflate the frame after the 2 byte header */
    }

        return ret;

    /* Codec has aligned strides */

    /* When a keyframe is found, copy it (flipped) */
                            avctx->width * component_size, avctx->height);
    /* Otherwise sum the delta on top of the current frame */
    else
                          avctx->width * component_size, avctx->height);

    /* Frame is ready to be output */
        return ret;

    /* Usual properties */
    } else {
    }

}

AVCodec ff_screenpresso_decoder = {
    .name           = "screenpresso",
    .long_name      = NULL_IF_CONFIG_SMALL("Screenpresso"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_SCREENPRESSO,
    .init           = screenpresso_init,
    .decode         = screenpresso_decode_frame,
    .close          = screenpresso_close,
    .priv_data_size = sizeof(ScreenpressoContext),
    .capabilities   = AV_CODEC_CAP_DR1,
    .caps_internal  = FF_CODEC_CAP_INIT_THREADSAFE |
                      FF_CODEC_CAP_INIT_CLEANUP,
};
