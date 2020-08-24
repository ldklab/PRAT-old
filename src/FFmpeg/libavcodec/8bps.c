/*
 * Quicktime Planar RGB (8BPS) Video Decoder
 * Copyright (C) 2003 Roberto Togni
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
 * QT 8BPS Video Decoder by Roberto Togni
 * For more information about the 8BPS format, visit:
 *   http://www.pcisys.net/~melanson/codecs/
 *
 * Supports: PAL8 (RGB 8bpp, paletted)
 *         : BGR24 (RGB 24bpp) (can also output it as RGB32)
 *         : RGB32 (RGB 32bpp, 4th plane is alpha)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libavutil/internal.h"
#include "libavutil/intreadwrite.h"
#include "avcodec.h"
#include "internal.h"


static const enum AVPixelFormat pixfmt_rgb24[] = {
    AV_PIX_FMT_BGR24, AV_PIX_FMT_0RGB32, AV_PIX_FMT_NONE };

typedef struct EightBpsContext {
    AVCodecContext *avctx;

    unsigned char planes;
    unsigned char planemap[4];

    uint32_t pal[256];
} EightBpsContext;

                        int *got_frame, AVPacket *avpkt)
{

        return ret;


    /* Set data pointer after line lengths */


        /* Lines length pointer for this plane */

        /* Decode a plane */
                return AVERROR_INVALIDDATA;
            /* Decode a row of this plane */
                    return AVERROR_INVALIDDATA;
                        break;
                        return AVERROR_INVALIDDATA;
                    }
                } else {
                        break;
                    }
                }
            }
        }
    }

                                                     AV_PKT_DATA_PALETTE,
                                                     &size);
            av_log(avctx, AV_LOG_ERROR, "Palette size %d is wrong\n", size);
        }

    }


    /* always report that the buffer was completely consumed */
}

{


    case 24:
        avctx->pix_fmt = ff_get_format(avctx, pixfmt_rgb24);
        c->planes      = 3;
        c->planemap[0] = 2; // 1st plane is red
        c->planemap[1] = 1; // 2nd plane is green
        c->planemap[2] = 0; // 3rd plane is blue
        break;
    case 32:
        avctx->pix_fmt = AV_PIX_FMT_RGB32;
        c->planes      = 4;
        /* handle planemap setup later for decoding rgb24 data as rbg32 */
        break;
    default:
        av_log(avctx, AV_LOG_ERROR, "Error: Unsupported color depth: %u.\n",
               avctx->bits_per_coded_sample);
        return AVERROR_INVALIDDATA;
    }

        c->planemap[0] = HAVE_BIGENDIAN ? 1 : 2; // 1st plane is red
        c->planemap[1] = HAVE_BIGENDIAN ? 2 : 1; // 2nd plane is green
        c->planemap[2] = HAVE_BIGENDIAN ? 3 : 0; // 3rd plane is blue
        c->planemap[3] = HAVE_BIGENDIAN ? 0 : 3; // 4th plane is alpha
    }
    return 0;
}

AVCodec ff_eightbps_decoder = {
    .name           = "8bps",
    .long_name      = NULL_IF_CONFIG_SMALL("QuickTime 8BPS video"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_8BPS,
    .priv_data_size = sizeof(EightBpsContext),
    .init           = decode_init,
    .decode         = decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
