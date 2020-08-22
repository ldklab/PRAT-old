/*
 * IBM Ultimotion Video Decoder
 * Copyright (C) 2004 Konstantin Shishkov
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
 * IBM Ultimotion Video Decoder.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"

#include "ulti_cb.h"

typedef struct UltimotionDecodeContext {
    AVCodecContext *avctx;
    int width, height, blocks;
    AVFrame *frame;
    const uint8_t *ulti_codebook;
    GetByteContext gb;
} UltimotionDecodeContext;

{

        return AVERROR_INVALIDDATA;

        return AVERROR(ENOMEM);

    return 0;
}

{


}

static const int block_coords[8] = // 4x4 block coords in 8x8 superblock
    { 0, 0, 0, 4, 4, 4, 4, 0};

static const int angle_by_index[4] = { 0, 2, 6, 12};

/* Lookup tables for luma and chroma - used by ulti_convert_yuv() */
static const uint8_t ulti_lumas[64] =
    { 0x10, 0x13, 0x17, 0x1A, 0x1E, 0x21, 0x25, 0x28,
      0x2C, 0x2F, 0x33, 0x36, 0x3A, 0x3D, 0x41, 0x44,
      0x48, 0x4B, 0x4F, 0x52, 0x56, 0x59, 0x5C, 0x60,
      0x63, 0x67, 0x6A, 0x6E, 0x71, 0x75, 0x78, 0x7C,
      0x7F, 0x83, 0x86, 0x8A, 0x8D, 0x91, 0x94, 0x98,
      0x9B, 0x9F, 0xA2, 0xA5, 0xA9, 0xAC, 0xB0, 0xB3,
      0xB7, 0xBA, 0xBE, 0xC1, 0xC5, 0xC8, 0xCC, 0xCF,
      0xD3, 0xD6, 0xDA, 0xDD, 0xE1, 0xE4, 0xE8, 0xEB};

static const uint8_t ulti_chromas[16] =
    { 0x60, 0x67, 0x6D, 0x73, 0x7A, 0x80, 0x86, 0x8D,
      0x93, 0x99, 0xA0, 0xA6, 0xAC, 0xB3, 0xB9, 0xC0};

/* convert Ultimotion YUV block (sixteen 6-bit Y samples and
 two 4-bit chroma samples) into standard YUV and put it into frame */
                             uint8_t *luma,int chroma)
{





        }
    }

/* generate block like in MS Video1 */
                         int f0, int f1, int Y0, int Y1, int chroma)
{
        else
    }

        else
    }


/* fill block with some gradient */
{
    }
    default:
        Luma[0]  = Y[0]; Luma[1]  = Y[0]; Luma[2]  = Y[1]; Luma[3]  = Y[1];
        Luma[4]  = Y[0]; Luma[5]  = Y[0]; Luma[6]  = Y[1]; Luma[7]  = Y[1];
        Luma[8]  = Y[2]; Luma[9]  = Y[2]; Luma[10] = Y[3]; Luma[11] = Y[3];
        Luma[12] = Y[2]; Luma[13] = Y[2]; Luma[14] = Y[3]; Luma[15] = Y[3];
        break;
    }


                             void *data, int *got_frame,
                             AVPacket *avpkt)
{

        return ret;


            break;//all blocks decoded

            goto err;
            case 0x70: //change modifier
                modifier = bytestream2_get_byte(&s->gb);
                if(modifier>1)
                    av_log(avctx, AV_LOG_INFO, "warning: modifier must be 0 or 1, got %i\n", modifier);
                break;
            case 0x71: // set uniq flag
                uniq = 1;
                break;
            case 0x73: //end-of-frame
                done = 1;
                break;
                    break;
                }
                break;
            default:
                av_log(avctx, AV_LOG_INFO, "warning: unknown escape 0x%02X\n", idx);
            }
        } else { //handle one block
                uniq = 0;
                cf = 1;
                chroma = 0;
            } else {
                }
            }
                }



                            Y[2] = 0x3F;
                    } else {
                    }
                    break;

                        tmp = bytestream2_get_be24(&s->gb);

                        Y[0] = (tmp >> 18) & 0x3F;
                        Y[1] = (tmp >> 12) & 0x3F;
                        Y[2] = (tmp >> 6) & 0x3F;
                        Y[3] = tmp & 0x3F;
                        angle = 16;
                    } else { // retrieve luma samples from codebook

                    }
                    break;

                        uint8_t Luma[16];

                        if (bytestream2_get_bytes_left(&s->gb) < 12)
                            goto err;
                        tmp = bytestream2_get_be24u(&s->gb);
                        Luma[0] = (tmp >> 18) & 0x3F;
                        Luma[1] = (tmp >> 12) & 0x3F;
                        Luma[2] = (tmp >> 6) & 0x3F;
                        Luma[3] = tmp & 0x3F;

                        tmp = bytestream2_get_be24u(&s->gb);
                        Luma[4] = (tmp >> 18) & 0x3F;
                        Luma[5] = (tmp >> 12) & 0x3F;
                        Luma[6] = (tmp >> 6) & 0x3F;
                        Luma[7] = tmp & 0x3F;

                        tmp = bytestream2_get_be24u(&s->gb);
                        Luma[8] = (tmp >> 18) & 0x3F;
                        Luma[9] = (tmp >> 12) & 0x3F;
                        Luma[10] = (tmp >> 6) & 0x3F;
                        Luma[11] = tmp & 0x3F;

                        tmp = bytestream2_get_be24u(&s->gb);
                        Luma[12] = (tmp >> 18) & 0x3F;
                        Luma[13] = (tmp >> 12) & 0x3F;
                        Luma[14] = (tmp >> 6) & 0x3F;
                        Luma[15] = tmp & 0x3F;

                        ulti_convert_yuv(s->frame, tx, ty, Luma, chroma);
                    } else {
                            goto err;
                        } else { // some patterns
                        }
                    }
                    break;
                }
            }
            }
        }
    }

        return ret;

    return buf_size;

err:
    av_log(avctx, AV_LOG_ERROR,
           "Insufficient data\n");
    return AVERROR_INVALIDDATA;
}

AVCodec ff_ulti_decoder = {
    .name           = "ultimotion",
    .long_name      = NULL_IF_CONFIG_SMALL("IBM UltiMotion"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_ULTI,
    .priv_data_size = sizeof(UltimotionDecodeContext),
    .init           = ulti_decode_init,
    .close          = ulti_decode_end,
    .decode         = ulti_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
