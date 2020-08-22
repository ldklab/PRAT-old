/*
 * Duck TrueMotion 1.0 Decoder
 * Copyright (C) 2003 Alex Beregszaszi & Mike Melanson
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
 * Duck TrueMotion v1 Video Decoder by
 * Alex Beregszaszi and
 * Mike Melanson (melanson@pcisys.net)
 *
 * The TrueMotion v1 decoder presently only decodes 16-bit TM1 data and
 * outputs RGB555 (or RGB565) data. 24-bit TM1 data is not supported yet.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "avcodec.h"
#include "internal.h"
#include "libavutil/imgutils.h"
#include "libavutil/internal.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/mem.h"

#include "truemotion1data.h"

typedef struct TrueMotion1Context {
    AVCodecContext *avctx;
    AVFrame *frame;

    const uint8_t *buf;
    int size;

    const uint8_t *mb_change_bits;
    int mb_change_bits_row_size;
    const uint8_t *index_stream;
    int index_stream_size;

    int flags;
    int x, y, w, h;

    uint32_t y_predictor_table[1024];
    uint32_t c_predictor_table[1024];
    uint32_t fat_y_predictor_table[1024];
    uint32_t fat_c_predictor_table[1024];

    int compression;
    int block_type;
    int block_width;
    int block_height;

    int16_t ydt[8];
    int16_t cdt[8];
    int16_t fat_ydt[8];
    int16_t fat_cdt[8];

    int last_deltaset, last_vectable;

    unsigned int *vert_pred;
    int vert_pred_size;

} TrueMotion1Context;

#define FLAG_SPRITE         32
#define FLAG_KEYFRAME       16
#define FLAG_INTERFRAME      8
#define FLAG_INTERPOLATED    4

struct frame_header {
    uint8_t header_size;
    uint8_t compression;
    uint8_t deltaset;
    uint8_t vectable;
    uint16_t ysize;
    uint16_t xsize;
    uint16_t checksum;
    uint8_t version;
    uint8_t header_type;
    uint8_t flags;
    uint8_t control;
    uint16_t xoffset;
    uint16_t yoffset;
    uint16_t width;
    uint16_t height;
};

#define ALGO_NOP        0
#define ALGO_RGB16V     1
#define ALGO_RGB16H     2
#define ALGO_RGB24H     3

/* these are the various block sizes that can occupy a 4x4 block */
#define BLOCK_2x2  0
#define BLOCK_2x4  1
#define BLOCK_4x2  2
#define BLOCK_4x4  3

typedef struct comp_types {
    int algorithm;
    int block_width; // vres
    int block_height; // hres
    int block_type;
} comp_types;

/* { valid for metatype }, algorithm, num of deltas, vert res, horiz res */
static const comp_types compression_types[17] = {
    { ALGO_NOP,    0, 0, 0 },

    { ALGO_RGB16V, 4, 4, BLOCK_4x4 },
    { ALGO_RGB16H, 4, 4, BLOCK_4x4 },
    { ALGO_RGB16V, 4, 2, BLOCK_4x2 },
    { ALGO_RGB16H, 4, 2, BLOCK_4x2 },

    { ALGO_RGB16V, 2, 4, BLOCK_2x4 },
    { ALGO_RGB16H, 2, 4, BLOCK_2x4 },
    { ALGO_RGB16V, 2, 2, BLOCK_2x2 },
    { ALGO_RGB16H, 2, 2, BLOCK_2x2 },

    { ALGO_NOP,    4, 4, BLOCK_4x4 },
    { ALGO_RGB24H, 4, 4, BLOCK_4x4 },
    { ALGO_NOP,    4, 2, BLOCK_4x2 },
    { ALGO_RGB24H, 4, 2, BLOCK_4x2 },

    { ALGO_NOP,    2, 4, BLOCK_2x4 },
    { ALGO_RGB24H, 2, 4, BLOCK_2x4 },
    { ALGO_NOP,    2, 2, BLOCK_2x2 },
    { ALGO_RGB24H, 2, 2, BLOCK_2x2 }
};

{

        return;


    /* Y skinny deltas need to be halved for some reason; maybe the
     * skinny Y deltas should be modified */
    {
        /* drop the lsb before dividing by 2-- net effect: round down
         * when dividing a negative number (e.g., -3/2 = -2, not -1) */
    }
}

#if HAVE_BIGENDIAN
static int make_ydt15_entry(int p2, int p1, int16_t *ydt)
#else
#endif
{

}

{

}

#if HAVE_BIGENDIAN
static int make_ydt16_entry(int p2, int p1, int16_t *ydt)
#else
static int make_ydt16_entry(int p1, int p2, int16_t *ydt)
#endif
{
    int lo, hi;

    lo = ydt[p1];
    lo += (lo << 6) + (lo << 11);
    hi = ydt[p2];
    hi += (hi << 6) + (hi << 11);
    return (lo + (hi << 16)) << 1;
}

static int make_cdt16_entry(int p1, int p2, int16_t *cdt)
{
    int r, b, lo;

    b = cdt[p2];
    r = cdt[p1] << 11;
    lo = b + r;
    return (lo + (lo * (1 << 16))) * 2;
}

{

}

{

}

{

    {
        {
        }
    }

static void gen_vector_table16(TrueMotion1Context *s, const uint8_t *sel_vector_table)
{
    int len, i, j;
    unsigned char delta_pair;

    for (i = 0; i < 1024; i += 4)
    {
        len = *sel_vector_table++ / 2;
        for (j = 0; j < len; j++)
        {
            delta_pair = *sel_vector_table++;
            s->y_predictor_table[i+j] = 0xfffffffe &
                make_ydt16_entry(delta_pair >> 4, delta_pair & 0xf, s->ydt);
            s->c_predictor_table[i+j] = 0xfffffffe &
                make_cdt16_entry(delta_pair >> 4, delta_pair & 0xf, s->cdt);
        }
        s->y_predictor_table[i+(j-1)] |= 1;
        s->c_predictor_table[i+(j-1)] |= 1;
    }
}

{

    {
        {
        }
    }

/* Returns the number of bytes consumed from the bytestream. Returns -1 if
 * there was an error while decoding the header */
{

    {
        av_log(s->avctx, AV_LOG_ERROR, "invalid header size (%d)\n", s->buf[0]);
        return AVERROR_INVALIDDATA;
    }

        av_log(s->avctx, AV_LOG_ERROR, "Input packet too small.\n");
        return AVERROR_INVALIDDATA;
    }

    /* unscramble the header bytes with a XOR operation */


    /* Version 2 */
    {
        {
            av_log(s->avctx, AV_LOG_ERROR, "invalid header type (%d)\n", header.header_type);
            return AVERROR_INVALIDDATA;
                s->flags |= FLAG_KEYFRAME;
        } else
            s->flags = FLAG_KEYFRAME;
    } else /* Version 1 */
        s->flags = FLAG_KEYFRAME;

        avpriv_request_sample(s->avctx, "Frame with sprite");
        /* FIXME header.width, height, xoffset and yoffset aren't initialized */
        return AVERROR_PATCHWELCOME;
    } else {
            if ((s->w < 213) && (s->h >= 176))
            {
                s->flags |= FLAG_INTERPOLATED;
                avpriv_request_sample(s->avctx, "Interpolated frame");
            }
        }
    }

        av_log(s->avctx, AV_LOG_ERROR, "invalid compression type (%d)\n", header.compression);
        return AVERROR_INVALIDDATA;
    }


        sel_vector_table = pc_tbl2;
    else {
        else {
            av_log(s->avctx, AV_LOG_ERROR, "invalid vector table id (%d)\n", header.vectable);
            return AVERROR_INVALIDDATA;
        }
    }

        new_pix_fmt = AV_PIX_FMT_0RGB32;
        width_shift = 1;
    } else

        avpriv_request_sample(s->avctx, "Frame with odd width");
        return AVERROR_PATCHWELCOME;
    }


            return ret;


            return AVERROR(ENOMEM);
    }

    /* There is 1 change bit per 4 pixels, so each change byte represents
     * 32 pixels; divide width by 4 to obtain the number of change bits and
     * then round up to the nearest byte. */

    {
        else
        else
            gen_vector_table16(s, sel_vector_table);
    }

    /* set up pointers to the other key data chunks */
        /* no change bits specified for a keyframe; only index bytes */
            return AVERROR_INVALIDDATA;
    } else {
        /* one change bit per 4x4 block */
    }


        av_log(s->avctx, AV_LOG_INFO, "tables: %d / %d c:%d %dx%d t:%d %s%s%s%s\n",
            s->last_deltaset, s->last_vectable, s->compression, s->block_width,
            s->block_height, s->block_type,
            s->flags & FLAG_KEYFRAME ? " KEY" : "",
            s->flags & FLAG_INTERFRAME ? " INTER" : "",
            s->flags & FLAG_SPRITE ? " SPRITE" : "",
            s->flags & FLAG_INTERPOLATED ? " INTERPOL" : "");

    return header.header_size;
}

{


    // FIXME: it may change ?
//    if (avctx->bits_per_sample == 24)
//        avctx->pix_fmt = AV_PIX_FMT_RGB24;
//    else
//        avctx->pix_fmt = AV_PIX_FMT_RGB555;

        return AVERROR(ENOMEM);

    /* there is a vertical predictor for each pixel in a line; each vertical
     * predictor is 0 to start with */
        av_frame_free(&s->frame);
        return AVERROR(ENOMEM);
    }

    return 0;
}

/*
Block decoding order:

dxi: Y-Y
dxic: Y-C-Y
dxic2: Y-C-Y-C

hres,vres,i,i%vres (0 < i < 4)
2x2 0: 0 dxic2
2x2 1: 1 dxi
2x2 2: 0 dxic2
2x2 3: 1 dxi
2x4 0: 0 dxic2
2x4 1: 1 dxi
2x4 2: 2 dxi
2x4 3: 3 dxi
4x2 0: 0 dxic
4x2 1: 1 dxi
4x2 2: 0 dxic
4x2 3: 1 dxi
4x4 0: 0 dxic
4x4 1: 1 dxi
4x4 2: 2 dxi
4x4 3: 3 dxi
*/

#define GET_NEXT_INDEX() \
{\
    if (index_stream_index >= s->index_stream_size) { \
        av_log(s->avctx, AV_LOG_INFO, " help! truemotion1 decoder went out of bounds\n"); \
        return; \
    } \
    index = s->index_stream[index_stream_index++] * 4; \
}

#define INC_INDEX                                                   \
do {                                                                \
    if (index >= 1023) {                                            \
        av_log(s->avctx, AV_LOG_ERROR, "Invalid index value.\n");   \
        return;                                                     \
    }                                                               \
    index++;                                                        \
} while (0)

#define APPLY_C_PREDICTOR() \
    predictor_pair = s->c_predictor_table[index]; \
    horiz_pred += (predictor_pair >> 1); \
    if (predictor_pair & 1) { \
        GET_NEXT_INDEX() \
        if (!index) { \
            GET_NEXT_INDEX() \
            predictor_pair = s->c_predictor_table[index]; \
            horiz_pred += ((predictor_pair >> 1) * 5); \
            if (predictor_pair & 1) \
                GET_NEXT_INDEX() \
            else \
                INC_INDEX; \
        } \
    } else \
        INC_INDEX;

#define APPLY_C_PREDICTOR_24() \
    predictor_pair = s->c_predictor_table[index]; \
    horiz_pred += (predictor_pair >> 1); \
    if (predictor_pair & 1) { \
        GET_NEXT_INDEX() \
        if (!index) { \
            GET_NEXT_INDEX() \
            predictor_pair = s->fat_c_predictor_table[index]; \
            horiz_pred += (predictor_pair >> 1); \
            if (predictor_pair & 1) \
                GET_NEXT_INDEX() \
            else \
                INC_INDEX; \
        } \
    } else \
        INC_INDEX;


#define APPLY_Y_PREDICTOR() \
    predictor_pair = s->y_predictor_table[index]; \
    horiz_pred += (predictor_pair >> 1); \
    if (predictor_pair & 1) { \
        GET_NEXT_INDEX() \
        if (!index) { \
            GET_NEXT_INDEX() \
            predictor_pair = s->y_predictor_table[index]; \
            horiz_pred += ((predictor_pair >> 1) * 5); \
            if (predictor_pair & 1) \
                GET_NEXT_INDEX() \
            else \
                INC_INDEX; \
        } \
    } else \
        INC_INDEX;

#define APPLY_Y_PREDICTOR_24() \
    predictor_pair = s->y_predictor_table[index]; \
    horiz_pred += (predictor_pair >> 1); \
    if (predictor_pair & 1) { \
        GET_NEXT_INDEX() \
        if (!index) { \
            GET_NEXT_INDEX() \
            predictor_pair = s->fat_y_predictor_table[index]; \
            horiz_pred += (predictor_pair >> 1); \
            if (predictor_pair & 1) \
                GET_NEXT_INDEX() \
            else \
                INC_INDEX; \
        } \
    } else \
        INC_INDEX;

#define OUTPUT_PIXEL_PAIR() \
    *current_pixel_pair = *vert_pred + horiz_pred; \
    *vert_pred++ = *current_pixel_pair++;

{

    /* these variables are for managing the stream of macroblock change bits */

    /* these variables are for managing the main index stream */

    /* clean out the line buffer */



        /* re-init variables for the next line iteration */



                    /* if macroblock width is 2, apply C-Y-C-Y; else
                     * apply C-Y-Y */
                        APPLY_C_PREDICTOR();
                        APPLY_Y_PREDICTOR();
                        OUTPUT_PIXEL_PAIR();
                        APPLY_C_PREDICTOR();
                        APPLY_Y_PREDICTOR();
                        OUTPUT_PIXEL_PAIR();
                    } else {
                    }
                    break;

                case 3:
                    /* always apply 2 Y predictors on these iterations */

                    /* this iteration might be C-Y-C-Y, Y-Y, or C-Y-Y
                     * depending on the macroblock type */
                        APPLY_C_PREDICTOR();
                        APPLY_Y_PREDICTOR();
                        OUTPUT_PIXEL_PAIR();
                        APPLY_C_PREDICTOR();
                        APPLY_Y_PREDICTOR();
                        OUTPUT_PIXEL_PAIR();
                        APPLY_C_PREDICTOR();
                        APPLY_Y_PREDICTOR();
                        OUTPUT_PIXEL_PAIR();
                        APPLY_Y_PREDICTOR();
                        OUTPUT_PIXEL_PAIR();
                    } else {
                    }
                    break;
                }

            } else {

                /* skip (copy) four pixels, but reassign the horizontal
                 * predictor */

            }


                /* next byte */
                }
            }

        }

        /* next change row */

    }
}

{

    /* these variables are for managing the stream of macroblock change bits */

    /* these variables are for managing the main index stream */

    /* clean out the line buffer */



        /* re-init variables for the next line iteration */



                    /* if macroblock width is 2, apply C-Y-C-Y; else
                     * apply C-Y-Y */
                    } else {
                        APPLY_C_PREDICTOR_24();
                        APPLY_Y_PREDICTOR_24();
                        OUTPUT_PIXEL_PAIR();
                        APPLY_Y_PREDICTOR_24();
                        OUTPUT_PIXEL_PAIR();
                    }
                    break;

                case 3:
                    /* always apply 2 Y predictors on these iterations */

                    /* this iteration might be C-Y-C-Y, Y-Y, or C-Y-Y
                     * depending on the macroblock type */
                    } else if (s->block_type == BLOCK_4x2) {
                        APPLY_C_PREDICTOR_24();
                        APPLY_Y_PREDICTOR_24();
                        OUTPUT_PIXEL_PAIR();
                        APPLY_Y_PREDICTOR_24();
                        OUTPUT_PIXEL_PAIR();
                    } else {
                        APPLY_Y_PREDICTOR_24();
                        OUTPUT_PIXEL_PAIR();
                        APPLY_Y_PREDICTOR_24();
                        OUTPUT_PIXEL_PAIR();
                    }
                    break;
                }

            } else {

                /* skip (copy) four pixels, but reassign the horizontal
                 * predictor */

            }


                /* next byte */
                }
            }

        }

        /* next change row */

    }
}


                                    void *data, int *got_frame,
                                    AVPacket *avpkt)
{


        return ret;

        return ret;

    }

        return ret;


    /* report that the buffer was completely consumed */
}

{


}

AVCodec ff_truemotion1_decoder = {
    .name           = "truemotion1",
    .long_name      = NULL_IF_CONFIG_SMALL("Duck TrueMotion 1.0"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_TRUEMOTION1,
    .priv_data_size = sizeof(TrueMotion1Context),
    .init           = truemotion1_decode_init,
    .close          = truemotion1_decode_end,
    .decode         = truemotion1_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
