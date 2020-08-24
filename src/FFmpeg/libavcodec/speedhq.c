/*
 * NewTek SpeedHQ codec
 * Copyright 2017 Steinar H. Gunderson
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
 * NewTek SpeedHQ decoder.
 */

#define BITSTREAM_READER_LE

#include "libavutil/attributes.h"

#include "avcodec.h"
#include "get_bits.h"
#include "internal.h"
#include "libavutil/thread.h"
#include "mathops.h"
#include "mpeg12.h"
#include "mpeg12data.h"
#include "mpeg12vlc.h"

#define MAX_INDEX (64 - 1)

/*
 * 5 bits makes for very small tables, with no more than two lookups needed
 * for the longest (10-bit) codes.
 */
#define ALPHA_VLC_BITS 5

typedef struct SHQContext {
    AVCodecContext *avctx;
    BlockDSPContext bdsp;
    IDCTDSPContext idsp;
    ScanTable intra_scantable;
    int quant_matrix[64];
    enum { SHQ_SUBSAMPLING_420, SHQ_SUBSAMPLING_422, SHQ_SUBSAMPLING_444 }
        subsampling;
    enum { SHQ_NO_ALPHA, SHQ_RLE_ALPHA, SHQ_DCT_ALPHA } alpha_type;
} SHQContext;


/* AC codes: Very similar but not identical to MPEG-2. */
static const uint16_t speedhq_vlc[123][2] = {
    {0x0001,  2}, {0x0003,  3}, {0x000E,  4}, {0x0007,  5},
    {0x0017,  5}, {0x0028,  6}, {0x0008,  6}, {0x006F,  7},
    {0x001F,  7}, {0x00C4,  8}, {0x0044,  8}, {0x005F,  8},
    {0x00DF,  8}, {0x007F,  8}, {0x00FF,  8}, {0x3E00, 14},
    {0x1E00, 14}, {0x2E00, 14}, {0x0E00, 14}, {0x3600, 14},
    {0x1600, 14}, {0x2600, 14}, {0x0600, 14}, {0x3A00, 14},
    {0x1A00, 14}, {0x2A00, 14}, {0x0A00, 14}, {0x3200, 14},
    {0x1200, 14}, {0x2200, 14}, {0x0200, 14}, {0x0C00, 15},
    {0x7400, 15}, {0x3400, 15}, {0x5400, 15}, {0x1400, 15},
    {0x6400, 15}, {0x2400, 15}, {0x4400, 15}, {0x0400, 15},
    {0x0002,  3}, {0x000C,  5}, {0x004F,  7}, {0x00E4,  8},
    {0x0004,  8}, {0x0D00, 13}, {0x1500, 13}, {0x7C00, 15},
    {0x3C00, 15}, {0x5C00, 15}, {0x1C00, 15}, {0x6C00, 15},
    {0x2C00, 15}, {0x4C00, 15}, {0xC800, 16}, {0x4800, 16},
    {0x8800, 16}, {0x0800, 16}, {0x0300, 13}, {0x1D00, 13},
    {0x0014,  5}, {0x0070,  7}, {0x003F,  8}, {0x00C0, 10},
    {0x0500, 13}, {0x0180, 12}, {0x0280, 12}, {0x0C80, 12},
    {0x0080, 12}, {0x0B00, 13}, {0x1300, 13}, {0x001C,  5},
    {0x0064,  8}, {0x0380, 12}, {0x1900, 13}, {0x0D80, 12},
    {0x0018,  6}, {0x00BF,  8}, {0x0480, 12}, {0x0B80, 12},
    {0x0038,  6}, {0x0040,  9}, {0x0900, 13}, {0x0030,  7},
    {0x0780, 12}, {0x2800, 16}, {0x0010,  7}, {0x0A80, 12},
    {0x0050,  7}, {0x0880, 12}, {0x000F,  7}, {0x1100, 13},
    {0x002F,  7}, {0x0100, 13}, {0x0084,  8}, {0x5800, 16},
    {0x00A4,  8}, {0x9800, 16}, {0x0024,  8}, {0x1800, 16},
    {0x0140,  9}, {0xE800, 16}, {0x01C0,  9}, {0x6800, 16},
    {0x02C0, 10}, {0xA800, 16}, {0x0F80, 12}, {0x0580, 12},
    {0x0980, 12}, {0x0E80, 12}, {0x0680, 12}, {0x1F00, 13},
    {0x0F00, 13}, {0x1700, 13}, {0x0700, 13}, {0x1B00, 13},
    {0xF800, 16}, {0x7800, 16}, {0xB800, 16}, {0x3800, 16},
    {0xD800, 16},
    {0x0020,  6}, /* escape */
    {0x0006,  4}  /* EOB */
};

static const uint8_t speedhq_level[121] = {
     1,  2,  3,  4,  5,  6,  7,  8,
     9, 10, 11, 12, 13, 14, 15, 16,
    17, 18, 19, 20, 21, 22, 23, 24,
    25, 26, 27, 28, 29, 30, 31, 32,
    33, 34, 35, 36, 37, 38, 39, 40,
     1,  2,  3,  4,  5,  6,  7,  8,
     9, 10, 11, 12, 13, 14, 15, 16,
    17, 18, 19, 20,  1,  2,  3,  4,
     5,  6,  7,  8,  9, 10, 11,  1,
     2,  3,  4,  5,  1,  2,  3,  4,
     1,  2,  3,  1,  2,  3,  1,  2,
     1,  2,  1,  2,  1,  2,  1,  2,
     1,  2,  1,  2,  1,  2,  1,  2,
     1,  2,  1,  1,  1,  1,  1,  1,
     1,  1,  1,  1,  1,  1,  1,  1,
     1,
};

static const uint8_t speedhq_run[121] = {
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     1,  1,  1,  1,  1,  1,  1,  1,
     1,  1,  1,  1,  1,  1,  1,  1,
     1,  1,  1,  1,  2,  2,  2,  2,
     2,  2,  2,  2,  2,  2,  2,  3,
     3,  3,  3,  3,  4,  4,  4,  4,
     5,  5,  5,  6,  6,  6,  7,  7,
     8,  8,  9,  9, 10, 10, 11, 11,
    12, 12, 13, 13, 14, 14, 15, 15,
    16, 16, 17, 18, 19, 20, 21, 22,
    23, 24, 25, 26, 27, 28, 29, 30,
    31,
};

static RLTable ff_rl_speedhq = {
    121,
    121,
    (const uint16_t (*)[])speedhq_vlc,
    speedhq_run,
    speedhq_level,
};

/* NOTE: The first element is always 16, unscaled. */
static const uint8_t unscaled_quant_matrix[64] = {
    16, 16, 19, 22, 26, 27, 29, 34,
    16, 16, 22, 24, 27, 29, 34, 37,
    19, 22, 26, 27, 29, 34, 34, 38,
    22, 22, 26, 27, 29, 34, 37, 40,
    22, 26, 27, 29, 32, 35, 40, 48,
    26, 27, 29, 32, 35, 40, 48, 58,
    26, 27, 29, 34, 38, 46, 56, 69,
    27, 29, 35, 38, 46, 56, 69, 83
};

static uint8_t ff_speedhq_static_rl_table_store[2][2*MAX_RUN + MAX_LEVEL + 3];

static VLC ff_dc_lum_vlc_le;
static VLC ff_dc_chroma_vlc_le;
static VLC ff_dc_alpha_run_vlc_le;
static VLC ff_dc_alpha_level_vlc_le;

{

    } else {
    }
        av_log(NULL, AV_LOG_ERROR, "invalid dc code at\n");
        return 0xffff;
    }
        diff = 0;
    } else {
    }
    return diff;
}

static inline int decode_alpha_block(const SHQContext *s, GetBitContext *gb, uint8_t last_alpha[16], uint8_t *dest, int linesize)
{
    uint8_t block[128];
    int i = 0, x, y;

    memset(block, 0, sizeof(block));

    {
        OPEN_READER(re, gb);

        for ( ;; ) {
            int run, level;

            UPDATE_CACHE_LE(re, gb);
            GET_VLC(run, re, gb, ff_dc_alpha_run_vlc_le.table, ALPHA_VLC_BITS, 2);

            if (run < 0) break;
            i += run;
            if (i >= 128)
                return AVERROR_INVALIDDATA;

            UPDATE_CACHE_LE(re, gb);
            GET_VLC(level, re, gb, ff_dc_alpha_level_vlc_le.table, ALPHA_VLC_BITS, 2);
            block[i++] = level;
        }

        CLOSE_READER(re, gb);
    }

    for (y = 0; y < 8; y++) {
        for (x = 0; x < 16; x++) {
            last_alpha[x] -= block[y * 16 + x];
        }
        memcpy(dest, last_alpha, 16);
        dest += linesize;
    }

    return 0;
}

{



    /* Read AC coefficients. */
    {
                       TEX_VLC_BITS, 2, 0);
                break;
                    return AVERROR_INVALIDDATA;
                /* If next bit is 1, level = -level */
            } else {
                /* Escape. */
#if MIN_CACHE_BITS < 6 + 6 + 12
#error MIN_CACHE_BITS is too small for the escape code, add UPDATE_CACHE
#endif

                    return AVERROR_INVALIDDATA;
            }

        }
    }


}

{

        linesize_a = frame->linesize[3] * line_stride;

        return AVERROR_INVALIDDATA;



            return AVERROR_INVALIDDATA;
    }



            return ret;



                dest_cb = frame->data[1] + frame->linesize[1] * (y/2 + field_number);
                dest_cr = frame->data[2] + frame->linesize[2] * (y/2 + field_number);
            } else {
            }
                dest_a = frame->data[3] + frame->linesize[3] * (y + field_number);
            }

                /* Decode the four luma blocks. */
                    return ret;
                    return ret;
                    return ret;
                    return ret;

                /*
                 * Decode the first chroma block. For 4:2:0, this is the only one;
                 * for 4:2:2, it's the top block; for 4:4:4, it's the top-left block.
                 */
                    return ret;
                    return ret;

                    /* For 4:2:2, this is the bottom block; for 4:4:4, it's the bottom-left block. */
                        return ret;
                        return ret;

                        /* Top-right and bottom-right blocks. */
                        if ((ret = decode_dct_block(s, &gb, last_dc, 1, dest_cb + 8, linesize_cb)) < 0)
                            return ret;
                        if ((ret = decode_dct_block(s, &gb, last_dc, 2, dest_cr + 8, linesize_cr)) < 0)
                            return ret;
                        if ((ret = decode_dct_block(s, &gb, last_dc, 1, dest_cb + 8 * linesize_cb + 8, linesize_cb)) < 0)
                            return ret;
                        if ((ret = decode_dct_block(s, &gb, last_dc, 2, dest_cr + 8 * linesize_cr + 8, linesize_cr)) < 0)
                            return ret;

                        dest_cb += 8;
                        dest_cr += 8;
                    }
                }

                    /* Alpha coded using 16x8 RLE blocks. */
                    if ((ret = decode_alpha_block(s, &gb, last_alpha, dest_a, linesize_a)) < 0)
                        return ret;
                    if ((ret = decode_alpha_block(s, &gb, last_alpha, dest_a + 8 * linesize_a, linesize_a)) < 0)
                        return ret;
                    dest_a += 16;
                    /* Alpha encoded exactly like luma. */
                    if ((ret = decode_dct_block(s, &gb, last_dc, 3, dest_a, linesize_a)) < 0)
                        return ret;
                    if ((ret = decode_dct_block(s, &gb, last_dc, 3, dest_a + 8, linesize_a)) < 0)
                        return ret;
                    if ((ret = decode_dct_block(s, &gb, last_dc, 3, dest_a + 8 * linesize_a, linesize_a)) < 0)
                        return ret;
                    if ((ret = decode_dct_block(s, &gb, last_dc, 3, dest_a + 8 * linesize_a + 8, linesize_a)) < 0)
                        return ret;
                    dest_a += 16;
                }
            }
        }
    }

    return 0;
}

{
}

                                void *data, int *got_frame,
                                AVPacket *avpkt)
{

        return AVERROR_INVALIDDATA;

        return AVERROR_INVALIDDATA;
    }


        return AVERROR_INVALIDDATA;
    }


        return ret;
    }

        /*
         * Overlapping first and second fields is used to signal
         * encoding only a single field. In this case, "height"
         * is ambiguous; it could mean either the height of the
         * frame as a whole, or of the field. The former would make
         * more sense for compatibility with legacy decoders,
         * but this matches the convention used in NDI, which is
         * the primary user of this trick.
         */
            return ret;
    } else {
            return ret;
            return ret;
    }

}

/*
 * Alpha VLC. Run and level are independently coded, and would be
 * outside the default limits for MAX_RUN/MAX_LEVEL, so we don't
 * bother with combining them into one table.
 */
{

    /* Initialize VLC for alpha run. */

    /* 0 -> 0. */

    /* 10xx -> xx plus 1. */
    }

    /* 111xxxxxxx -> xxxxxxx. */
    }

    /* 110 -> EOB. */


                              FF_ARRAY_ELEMS(run_code),
                              run_bits, 1, 1,
                              run_code, 2, 2,
                              run_symbols, 2, 2, 160);

    /* Initialize VLC for alpha level. */

        /* 1s -> -1 or +1 (depending on sign bit). */

        /* 01sxx -> xx plus 2 (2..5 or -2..-5, depending on sign bit). */
        }
    }

    /*
     * 00xxxxxxxx -> xxxxxxxx, in two's complement. There are many codes
     * here that would better be encoded in other ways (e.g. 0 would be
     * encoded by increasing run, and +/- 1 would be encoded with a
     * shorter code), but it doesn't hurt to allow everything.
     */
    }


                              FF_ARRAY_ELEMS(level_code),
                              level_bits, 1, 1,
                              level_code, 2, 2,
                              level_symbols, 2, 2, 288);

{
}

                         uint16_t *reversed_code, int num_entries)
{
    }

{

    /* Exactly the same as MPEG-2, except little-endian. */
                 ff_mpeg12_vlc_dc_lum_bits,
                 ff_mpeg12_vlc_dc_lum_code_reversed,
                 12);
                       ff_mpeg12_vlc_dc_lum_bits, 1, 1,
                       ff_mpeg12_vlc_dc_lum_code_reversed, 2, 2, 512);
                 ff_mpeg12_vlc_dc_chroma_bits,
                 ff_mpeg12_vlc_dc_chroma_code_reversed,
                 12);
                       ff_mpeg12_vlc_dc_chroma_bits, 1, 1,
                       ff_mpeg12_vlc_dc_chroma_code_reversed, 2, 2, 514);



{


        return AVERROR_UNKNOWN;


    case MKTAG('S', 'H', 'Q', '0'):
        s->subsampling = SHQ_SUBSAMPLING_420;
        s->alpha_type = SHQ_NO_ALPHA;
        avctx->pix_fmt = AV_PIX_FMT_YUV420P;
        break;
    case MKTAG('S', 'H', 'Q', '1'):
        s->subsampling = SHQ_SUBSAMPLING_420;
        s->alpha_type = SHQ_RLE_ALPHA;
        avctx->pix_fmt = AV_PIX_FMT_YUVA420P;
        break;
    case MKTAG('S', 'H', 'Q', '3'):
        s->subsampling = SHQ_SUBSAMPLING_422;
        s->alpha_type = SHQ_RLE_ALPHA;
        avctx->pix_fmt = AV_PIX_FMT_YUVA422P;
        break;
    case MKTAG('S', 'H', 'Q', '4'):
        s->subsampling = SHQ_SUBSAMPLING_444;
        s->alpha_type = SHQ_NO_ALPHA;
        avctx->pix_fmt = AV_PIX_FMT_YUV444P;
        break;
    case MKTAG('S', 'H', 'Q', '5'):
        s->subsampling = SHQ_SUBSAMPLING_444;
        s->alpha_type = SHQ_RLE_ALPHA;
        avctx->pix_fmt = AV_PIX_FMT_YUVA444P;
        break;
    case MKTAG('S', 'H', 'Q', '7'):
        s->subsampling = SHQ_SUBSAMPLING_422;
        s->alpha_type = SHQ_DCT_ALPHA;
        avctx->pix_fmt = AV_PIX_FMT_YUVA422P;
        break;
    case MKTAG('S', 'H', 'Q', '9'):
        s->subsampling = SHQ_SUBSAMPLING_444;
        s->alpha_type = SHQ_DCT_ALPHA;
        avctx->pix_fmt = AV_PIX_FMT_YUVA444P;
        break;
               avctx->codec_tag);
    }

    /* This matches what NDI's RGB -> Y'CbCr 4:2:2 converter uses. */

}

AVCodec ff_speedhq_decoder = {
    .name           = "speedhq",
    .long_name      = NULL_IF_CONFIG_SMALL("NewTek SpeedHQ"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_SPEEDHQ,
    .priv_data_size = sizeof(SHQContext),
    .init           = speedhq_decode_init,
    .decode         = speedhq_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
