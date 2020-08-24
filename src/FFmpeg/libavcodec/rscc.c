/*
 * innoHeim/Rsupport Screen Capture Codec
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
 * innoHeim/Rsupport Screen Capture Codec decoder
 *
 * Fourcc: ISCC, RSCC
 *
 * Lossless codec, data stored in tiles, with optional deflate compression.
 *
 * Header contains the number of tiles in a frame with the tile coordinates,
 * and it can be deflated or not. Similarly, pixel data comes after the header
 * and a variable size value, and it can be deflated or just raw.
 *
 * Supports: PAL8, BGRA, BGR24, RGB555
 */

#include <stdint.h>
#include <string.h>
#include <zlib.h>

#include "libavutil/imgutils.h"
#include "libavutil/internal.h"

#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"

#define TILE_SIZE 8

typedef struct Tile {
    int x, y;
    int w, h;
} Tile;

typedef struct RsccContext {
    GetByteContext gbc;
    AVFrame *reference;
    Tile *tiles;
    unsigned int tiles_size;
    int component_size;

    uint8_t palette[AVPALETTE_SIZE];

    /* zlib interaction */
    uint8_t *inflated_buf;
    uLongf inflated_size;
    int valid_pixels;
} RsccContext;

{

    /* These needs to be set to estimate uncompressed buffer */
        av_log(avctx, AV_LOG_ERROR, "Invalid image size %dx%d.\n",
               avctx->width, avctx->height);
        return ret;
    }

    /* Allocate reference frame */
        return AVERROR(ENOMEM);

    /* Get pixel format and the size of the pixel */
            } else {
                avctx->pix_fmt = AV_PIX_FMT_BGR24;
                ctx->component_size = 3;
            }
        } else {
            avctx->pix_fmt = AV_PIX_FMT_BGRA;
            ctx->component_size = 4;
        }
        default:
            av_log(avctx, AV_LOG_ERROR, "Invalid bits per pixel value (%d)\n",
                   avctx->bits_per_coded_sample);
            return AVERROR_INVALIDDATA;
        }
    } else {
        avctx->pix_fmt = AV_PIX_FMT_BGR0;
        ctx->component_size = 4;
        av_log(avctx, AV_LOG_WARNING, "Invalid codec tag\n");
    }

    /* Store the value to check for keyframes */

    /* Allocate maximum size possible, a full frame */
        return AVERROR(ENOMEM);

    return 0;
}

{


}

                                     int *got_frame, AVPacket *avpkt)
{


    /* Size check */
        av_log(avctx, AV_LOG_ERROR, "Packet too small (%d)\n", avpkt->size);
        return AVERROR_INVALIDDATA;
    }

    /* Read number of tiles, and allocate the array */

        av_log(avctx, AV_LOG_DEBUG, "no tiles\n");
        return avpkt->size;
    }

                   tiles_nb * sizeof(*ctx->tiles));
        ret = AVERROR(ENOMEM);
        goto end;
    }


    /* When there are more than 5 tiles, they are packed together with
     * a size header. When that size does not match the number of tiles
     * times the tile size, it means it needs to be inflated as well */

        else


        /* If necessary, uncompress tiles, and hijack the bytestream reader */

                ret = AVERROR_INVALIDDATA;
                goto end;
            }

                ret = AVERROR(ENOMEM);
                goto end;
            }

                av_log(avctx, AV_LOG_ERROR, "Tile deflate error %d.\n", ret);
                ret = AVERROR_UNKNOWN;
                goto end;
            }

            /* Skip the compressed tile section in the main byte reader,
             * and point it to read the newly uncompressed data */
        }
    }

    /* Fill in array of tiles, keeping track of how many pixels are updated */

            av_log(avctx, AV_LOG_ERROR, "Invalid tile dimensions\n");
            ret = AVERROR_INVALIDDATA;
            goto end;
        }


                ctx->tiles[i].x, ctx->tiles[i].y,
                ctx->tiles[i].w, ctx->tiles[i].h);

            av_log(avctx, AV_LOG_ERROR,
                   "invalid tile %d at (%d.%d) with size %dx%d.\n", i,
                   ctx->tiles[i].x, ctx->tiles[i].y,
                   ctx->tiles[i].w, ctx->tiles[i].h);
            ret = AVERROR_INVALIDDATA;
            goto end;
            av_log(avctx, AV_LOG_ERROR,
                   "out of bounds tile %d at (%d.%d) with size %dx%d.\n", i,
                   ctx->tiles[i].x, ctx->tiles[i].y,
                   ctx->tiles[i].w, ctx->tiles[i].h);
            ret = AVERROR_INVALIDDATA;
            goto end;
        }
    }

    /* Reset the reader in case it had been modified before */

    /* Extract how much pixel data the tiles contain */
        packed_size = bytestream2_get_byte(gbc);
        packed_size = bytestream2_get_le16(gbc);
    else
        packed_size = bytestream2_get_le32(gbc);


        av_log(avctx, AV_LOG_ERROR, "Invalid tile size %d\n", packed_size);
        ret = AVERROR_INVALIDDATA;
        goto end;
    }

    /* Get pixels buffer, it may be deflated or just raw */
        if (bytestream2_get_bytes_left(gbc) < pixel_size) {
            av_log(avctx, AV_LOG_ERROR, "Insufficient input for %d\n", pixel_size);
            ret = AVERROR_INVALIDDATA;
            goto end;
        }
        pixels = gbc->buffer;
    } else {
        }
            av_log(avctx, AV_LOG_ERROR, "Pixel deflate error %d.\n", ret);
            ret = AVERROR_UNKNOWN;
            goto end;
        }
    }

    /* Allocate when needed */
        goto end;

    /* Pointer to actual pixels, will be updated when data is consumed */
    raw = pixels;
                            raw, ctx->tiles[i].w * ctx->component_size,
                            ctx->tiles[i].h);
    }

    /* Frame is ready to be output */
        goto end;

    /* Keyframe when the number of pixels updated matches the whole surface */
    } else {
    }

    /* Palette handling */
                                                         AV_PKT_DATA_PALETTE,
                                                         &size);
        } else if (palette) {
            av_log(avctx, AV_LOG_ERROR, "Palette size %d is wrong\n", size);
        }
    }
    // We only return a picture when enough of it is undamaged, this avoids copying nearly broken frames around

}

AVCodec ff_rscc_decoder = {
    .name           = "rscc",
    .long_name      = NULL_IF_CONFIG_SMALL("innoHeim/Rsupport Screen Capture Codec"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_RSCC,
    .init           = rscc_init,
    .decode         = rscc_decode_frame,
    .close          = rscc_close,
    .priv_data_size = sizeof(RsccContext),
    .capabilities   = AV_CODEC_CAP_DR1,
    .caps_internal  = FF_CODEC_CAP_INIT_THREADSAFE |
                      FF_CODEC_CAP_INIT_CLEANUP,
};
