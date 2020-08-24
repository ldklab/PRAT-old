/*
 * Quicktime Video (RPZA) Video Decoder
 * Copyright (C) 2003 The FFmpeg project
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
 * QT RPZA Video Decoder by Roberto Togni
 * For more information about the RPZA format, visit:
 *   http://www.pcisys.net/~melanson/codecs/
 *
 * The RPZA decoder outputs RGB555 colorspace data.
 *
 * Note that this decoder reads big endian RGB555 pixel values from the
 * bytestream, arranges them in the host's endian order, and outputs
 * them to the final rendered map in the same host endian order. This is
 * intended behavior as the libavcodec documentation states that RGB555
 * pixels shall be stored in native CPU endianness.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libavutil/internal.h"
#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"

typedef struct RpzaContext {

    AVCodecContext *avctx;
    AVFrame *frame;

    GetByteContext gb;
} RpzaContext;

#define CHECK_BLOCK()                                                         \
    if (total_blocks < 1) {                                                    \
        av_log(s->avctx, AV_LOG_ERROR,                                         \
               "Block counter just went negative (this should not happen)\n"); \
        return AVERROR_INVALIDDATA;                                            \
    }                                                                          \

#define ADVANCE_BLOCK()             \
    {                               \
        pixel_ptr += 4;             \
        if (pixel_ptr >= width)     \
        {                           \
            pixel_ptr = 0;          \
            row_ptr  += stride * 4; \
        }                           \
        total_blocks--;             \
    }

{


    /* First byte is always 0xe1. Warn if it's different */
        av_log(s->avctx, AV_LOG_ERROR, "First chunk byte is 0x%02x instead of 0xe1\n",
               bytestream2_peek_byte(&s->gb));

    /* Get chunk size, ignoring first byte */

    /* If length mismatch use size from MOV file and try to decode anyway */
        av_log(s->avctx, AV_LOG_WARNING,
               "MOV chunk size %d != encoded chunk size %d\n",
               chunk_size,
               bytestream2_get_bytes_left(&s->gb) + 4
              );

    /* Number of 4x4 blocks in frame. */

        return AVERROR_INVALIDDATA;

        return ret;

    /* Process chunk data */


        /* If opcode MSbit is 0, we need more data to decide what to do */
                /* Must behave as opcode 110xxxxx, using colorA computed
                 * above. Use fake opcode 0x20 to enter switch block at
                 * the right place */
            }
        }



        /* Skip blocks */
        case 0x80:
            }
            break;

        /* Fill blocks with one color */
                    }
                }
            }
            break;

        /* Fill blocks with 4 colors */
        case 0xc0:
            colorA = bytestream2_get_be16(&s->gb);

            /* sort out the colors */

            /* red components */

            /* green components */

            /* blue components */

                return AVERROR_INVALIDDATA;
                    }
                }
            }
            break;

        /* Fill block with 16 colors */
        case 0x00:
            if (bytestream2_get_bytes_left(&s->gb) < 30)
                return AVERROR_INVALIDDATA;
            CHECK_BLOCK();
            block_ptr = row_ptr + pixel_ptr;
            for (pixel_y = 0; pixel_y < 4; pixel_y++) {
                for (pixel_x = 0; pixel_x < 4; pixel_x++){
                    /* We already have color of upper left pixel */
                    if ((pixel_y != 0) || (pixel_x != 0))
                        colorA = bytestream2_get_be16u(&s->gb);
                    pixels[block_ptr] = colorA;
                    block_ptr++;
                }
                block_ptr += row_inc;
            }
            ADVANCE_BLOCK();
            break;

        /* Unknown opcode */
        default:
            av_log(s->avctx, AV_LOG_ERROR, "Unknown opcode %d in rpza chunk."
                 " Skip remaining %d bytes of chunk data.\n", opcode,
                 bytestream2_get_bytes_left(&s->gb));
            return AVERROR_INVALIDDATA;
        } /* Opcode switch */
    }

    return 0;
}

{


        return AVERROR(ENOMEM);

    return 0;
}

                             void *data, int *got_frame,
                             AVPacket *avpkt)
{


        return ret;

        return ret;


    /* always report that the buffer was completely consumed */
}

{


}

AVCodec ff_rpza_decoder = {
    .name           = "rpza",
    .long_name      = NULL_IF_CONFIG_SMALL("QuickTime video (RPZA)"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_RPZA,
    .priv_data_size = sizeof(RpzaContext),
    .init           = rpza_decode_init,
    .close          = rpza_decode_end,
    .decode         = rpza_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
