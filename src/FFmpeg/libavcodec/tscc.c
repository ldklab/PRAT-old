/*
 * TechSmith Camtasia decoder
 * Copyright (c) 2004 Konstantin Shishkov
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
 * TechSmith Camtasia decoder
 *
 * Fourcc: TSCC
 *
 * Codec is very simple:
 *  it codes picture (picture difference, really)
 *  with algorithm almost identical to Windows RLE8,
 *  only without padding and with greater pixel sizes,
 *  then this coded picture is packed with ZLib
 *
 * Supports: BGR8,BGR555,BGR24 - only BGR8 and BGR555 tested
 */

#include <stdio.h>
#include <stdlib.h>

#include "avcodec.h"
#include "internal.h"
#include "msrledec.h"

#include <zlib.h>

typedef struct TsccContext {

    AVCodecContext *avctx;
    AVFrame *frame;

    // Bits per pixel
    int bpp;
    // Decompressed data size
    unsigned int decomp_size;
    // Decompression buffer
    unsigned char* decomp_buf;
    GetByteContext gb;
    int height;
    z_stream zstream;

    uint32_t pal[256];
} CamtasiaContext;

                        AVPacket *avpkt)
{

        int size;
        const uint8_t *pal = av_packet_get_side_data(avpkt, AV_PKT_DATA_PALETTE, &size);

        if (pal && size == AVPALETTE_SIZE) {
            palette_has_changed = 1;
            memcpy(c->pal, pal, AVPALETTE_SIZE);
        } else if (pal) {
            av_log(avctx, AV_LOG_ERROR, "Palette size %d is wrong\n", size);
        }
    }

        av_log(avctx, AV_LOG_ERROR, "Inflate reset error: %d\n", ret);
        return AVERROR_UNKNOWN;
    }
    // Z_DATA_ERROR means empty picture
        return buf_size;
    }

    }

        return ret;

    }

    /* make the palette available on the way out */
        frame->palette_has_changed = palette_has_changed;
        memcpy(frame->data[1], c->pal, AVPALETTE_SIZE);
    }

        return ret;

    /* always report that the buffer was completely consumed */
}

{



    // Needed if zlib unused or init aborted before inflateInit
    case  8: avctx->pix_fmt = AV_PIX_FMT_PAL8; break;
    case 24:
             avctx->pix_fmt = AV_PIX_FMT_BGR24;
             break;
    default: av_log(avctx, AV_LOG_ERROR, "Camtasia error: unknown depth %i bpp\n", avctx->bits_per_coded_sample);
             return AVERROR_PATCHWELCOME;
    }
    // buffer size for RLE 'best' case when 2-byte code precedes each pixel and there may be padding after it too

    /* Allocate decompression buffer */
            av_log(avctx, AV_LOG_ERROR, "Can't allocate decompression buffer.\n");
            return AVERROR(ENOMEM);
        }
    }

        av_log(avctx, AV_LOG_ERROR, "Inflate init error: %d\n", zret);
        return AVERROR_UNKNOWN;
    }

        return AVERROR(ENOMEM);

    return 0;
}

{



}

AVCodec ff_tscc_decoder = {
    .name           = "camtasia",
    .long_name      = NULL_IF_CONFIG_SMALL("TechSmith Screen Capture Codec"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_TSCC,
    .priv_data_size = sizeof(CamtasiaContext),
    .init           = decode_init,
    .close          = decode_end,
    .decode         = decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
    .caps_internal  = FF_CODEC_CAP_INIT_CLEANUP,
};
