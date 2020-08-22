/*
 * Quicktime Graphics (SMC) Video Decoder
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
 * QT SMC Video Decoder by Mike Melanson (melanson@pcisys.net)
 * For more information about the SMC format, visit:
 *   http://www.pcisys.net/~melanson/codecs/
 *
 * The SMC decoder outputs PAL8 colorspace data.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libavutil/intreadwrite.h"
#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"

#define CPAIR 2
#define CQUAD 4
#define COCTET 8

#define COLORS_PER_TABLE 256

typedef struct SmcContext {

    AVCodecContext *avctx;
    AVFrame *frame;

    GetByteContext gb;

    /* SMC color tables */
    unsigned char color_pairs[COLORS_PER_TABLE * CPAIR];
    unsigned char color_quads[COLORS_PER_TABLE * CQUAD];
    unsigned char color_octets[COLORS_PER_TABLE * COCTET];

    uint32_t pal[256];
} SmcContext;

#define GET_BLOCK_COUNT() \
  (opcode & 0x10) ? (1 + bytestream2_get_byte(&s->gb)) : 1 + (opcode & 0x0F);

#define ADVANCE_BLOCK() \
{ \
    pixel_ptr += 4; \
    if (pixel_ptr >= width) \
    { \
        pixel_ptr = 0; \
        row_ptr += stride * 4; \
    } \
    total_blocks--; \
    if (total_blocks < !!n_blocks) \
    { \
        av_log(s->avctx, AV_LOG_INFO, "warning: block counter just went negative (this should not happen)\n"); \
        return; \
    } \
}

{




    /* make the palette available */

        av_log(s->avctx, AV_LOG_INFO, "warning: MOV chunk size != encoded chunk size (%d != %d); using MOV chunk size\n",
            chunk_size, buf_size);


    /* traverse through the blocks */
        /* sanity checks */
        /* make sure the row pointer hasn't gone wild */
            av_log(s->avctx, AV_LOG_INFO, "SMC decoder just went out of bounds (row ptr = %d, height = %d)\n",
                row_ptr, image_size);
            return;
        }
            av_log(s->avctx, AV_LOG_ERROR, "input too small\n");
            return;
        }

        /* skip n blocks */
        case 0x10:
            }
            break;

        /* repeat last block n times */
        case 0x30:

            /* sanity check */
                av_log(s->avctx, AV_LOG_INFO, "encountered repeat block opcode (%02X) but no blocks rendered yet\n",
                    opcode & 0xF0);
                return;
            }

            /* figure out where the previous block started */
                prev_block_ptr1 =
                    (row_ptr - s->avctx->width * 4) + s->avctx->width - 4;
            else

                    }
                }
            }
            break;

        /* repeat previous pair of blocks n times */
        case 0x40:
        case 0x50:
            n_blocks = GET_BLOCK_COUNT();
            n_blocks *= 2;

            /* sanity check */
            if ((row_ptr == 0) && (pixel_ptr < 2 * 4)) {
                av_log(s->avctx, AV_LOG_INFO, "encountered repeat block opcode (%02X) but not enough blocks rendered yet\n",
                    opcode & 0xF0);
                return;
            }

            /* figure out where the previous 2 blocks started */
            if (pixel_ptr == 0)
                prev_block_ptr1 = (row_ptr - s->avctx->width * 4) +
                    s->avctx->width - 4 * 2;
            else if (pixel_ptr == 4)
                prev_block_ptr1 = (row_ptr - s->avctx->width * 4) + row_inc;
            else
                prev_block_ptr1 = row_ptr + pixel_ptr - 4 * 2;

            if (pixel_ptr == 0)
                prev_block_ptr2 = (row_ptr - s->avctx->width * 4) + row_inc;
            else
                prev_block_ptr2 = row_ptr + pixel_ptr - 4;

            prev_block_flag = 0;
            while (n_blocks--) {
                block_ptr = row_ptr + pixel_ptr;
                if (prev_block_flag)
                    prev_block_ptr = prev_block_ptr2;
                else
                    prev_block_ptr = prev_block_ptr1;
                prev_block_flag = !prev_block_flag;

                for (pixel_y = 0; pixel_y < 4; pixel_y++) {
                    for (pixel_x = 0; pixel_x < 4; pixel_x++) {
                        pixels[block_ptr++] = pixels[prev_block_ptr++];
                    }
                    block_ptr += row_inc;
                    prev_block_ptr += row_inc;
                }
                ADVANCE_BLOCK();
            }
            break;

        /* 1-color block encoding */
        case 0x70:

                    }
                }
            }
            break;

        /* 2-color block encoding */
        case 0x90:

            /* figure out which color pair to use to paint the 2-color block */
                /* fetch the next 2 colors from bytestream and store in next
                 * available entry in the color pair table */
                }
                /* this is the base index to use for this block */
                /* wraparound */
                    color_pair_index = 0;
            } else

                        else
                            pixel = color_table_index;
                    }
                }
            }
            break;

        /* 4-color block encoding */
        case 0xB0:

            /* figure out which color quad to use to paint the 4-color block */
                /* fetch the next 4 colors from bytestream and store in next
                 * available entry in the color quad table */
                }
                /* this is the base index to use for this block */
                /* wraparound */
                    color_quad_index = 0;
            } else

                /* flag mask actually acts as a bit shift count here */
                    }
                }
            }
            break;

        /* 8-color block encoding */
        case 0xD0:

            /* figure out which color octet to use to paint the 8-color block */
                /* fetch the next 8 colors from bytestream and store in next
                 * available entry in the color octet table */
                }
                /* this is the base index to use for this block */
                /* wraparound */
                    color_octet_index = 0;
            } else

                /*
                  For this input of 6 hex bytes:
                    01 23 45 67 89 AB
                  Mangle it to this output:
                    flags_a = xx012456, flags_b = xx89A37B
                */
                /* build the color flags */

                /* flag mask actually acts as a bit shift count here */
                    /* reload flags at third row (iteration pixel_y == 2) */
                    }
                    }
                }
            }
            break;

        /* 16-color block encoding (every pixel is a different color) */

                    }
                }
            }
            break;

        case 0xF0:
            avpriv_request_sample(s->avctx, "0xF0 opcode");
            break;
        }
    }

    return;
}

{


        return AVERROR(ENOMEM);

    return 0;
}

                             void *data, int *got_frame,
                             AVPacket *avpkt)
{

        return AVERROR_INVALIDDATA;


        return ret;

        av_log(avctx, AV_LOG_ERROR, "Palette size %d is wrong\n", pal_size);
    }


        return ret;

    /* always report that the buffer was completely consumed */
    return buf_size;
}

{


}

AVCodec ff_smc_decoder = {
    .name           = "smc",
    .long_name      = NULL_IF_CONFIG_SMALL("QuickTime Graphics (SMC)"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_SMC,
    .priv_data_size = sizeof(SmcContext),
    .init           = smc_decode_init,
    .close          = smc_decode_end,
    .decode         = smc_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
