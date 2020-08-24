/*
 * Apple Intermediate Codec decoder
 *
 * Copyright (c) 2013 Konstantin Shishkov
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

#include <inttypes.h>

#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"
#include "get_bits.h"
#include "golomb.h"
#include "idctdsp.h"
#include "thread.h"
#include "unary.h"

#define AIC_HDR_SIZE    24
#define AIC_BAND_COEFFS (64 + 32 + 192 + 96)

enum AICBands {
    COEFF_LUMA = 0,
    COEFF_CHROMA,
    COEFF_LUMA_EXT,
    COEFF_CHROMA_EXT,
    NUM_BANDS
};

static const uint8_t aic_num_band_coeffs[NUM_BANDS] = { 64, 32, 192, 96 };

static const uint16_t aic_band_off[NUM_BANDS] = { 0, 64, 96, 288 };

static const uint8_t aic_quant_matrix[64] = {
     8, 16, 19, 22, 22, 26, 26, 27,
    16, 16, 22, 22, 26, 27, 27, 29,
    19, 22, 26, 26, 27, 29, 29, 35,
    22, 24, 27, 27, 29, 32, 34, 38,
    26, 27, 29, 29, 32, 35, 38, 46,
    27, 29, 34, 34, 35, 40, 46, 56,
    29, 34, 34, 37, 40, 48, 56, 69,
    34, 37, 38, 40, 48, 58, 69, 83,
};

static const uint8_t aic_y_scan[64] = {
     0,  4,  1,  2,  5,  8, 12,  9,
     6,  3,  7, 10, 13, 14, 11, 15,
    47, 43, 46, 45, 42, 39, 35, 38,
    41, 44, 40, 37, 34, 33, 36, 32,
    16, 20, 17, 18, 21, 24, 28, 25,
    22, 19, 23, 26, 29, 30, 27, 31,
    63, 59, 62, 61, 58, 55, 51, 54,
    57, 60, 56, 53, 50, 49, 52, 48,
};

static const uint8_t aic_y_ext_scan[192] = {
     64,  72,  65,  66,  73,  80,  88,  81,
     74,  67,  75,  82,  89,  90,  83,  91,
      0,   4,   1,   2,   5,   8,  12,   9,
      6,   3,   7,  10,  13,  14,  11,  15,
     16,  20,  17,  18,  21,  24,  28,  25,
     22,  19,  23,  26,  29,  30,  27,  31,
    155, 147, 154, 153, 146, 139, 131, 138,
    145, 152, 144, 137, 130, 129, 136, 128,
     47,  43,  46,  45,  42,  39,  35,  38,
     41,  44,  40,  37,  34,  33,  36,  32,
     63,  59,  62,  61,  58,  55,  51,  54,
     57,  60,  56,  53,  50,  49,  52,  48,
     96, 104,  97,  98, 105, 112, 120, 113,
    106,  99, 107, 114, 121, 122, 115, 123,
     68,  76,  69,  70,  77,  84,  92,  85,
     78,  71,  79,  86,  93,  94,  87,  95,
    100, 108, 101, 102, 109, 116, 124, 117,
    110, 103, 111, 118, 125, 126, 119, 127,
    187, 179, 186, 185, 178, 171, 163, 170,
    177, 184, 176, 169, 162, 161, 168, 160,
    159, 151, 158, 157, 150, 143, 135, 142,
    149, 156, 148, 141, 134, 133, 140, 132,
    191, 183, 190, 189, 182, 175, 167, 174,
    181, 188, 180, 173, 166, 165, 172, 164,
};

static const uint8_t aic_c_scan[64] = {
     0,  4,  1,  2,  5,  8, 12,  9,
     6,  3,  7, 10, 13, 14, 11, 15,
    31, 27, 30, 29, 26, 23, 19, 22,
    25, 28, 24, 21, 18, 17, 20, 16,
    32, 36, 33, 34, 37, 40, 44, 41,
    38, 35, 39, 42, 45, 46, 43, 47,
    63, 59, 62, 61, 58, 55, 51, 54,
    57, 60, 56, 53, 50, 49, 52, 48,
};

static const uint8_t aic_c_ext_scan[192] = {
     16,  24,  17,  18,  25,  32,  40,  33,
     26,  19,  27,  34,  41,  42,  35,  43,
      0,   4,   1,   2,   5,   8,  12,   9,
      6,   3,   7,  10,  13,  14,  11,  15,
     20,  28,  21,  22,  29,  36,  44,  37,
     30,  23,  31,  38,  45,  46,  39,  47,
     95,  87,  94,  93,  86,  79,  71,  78,
     85,  92,  84,  77,  70,  69,  76,  68,
     63,  59,  62,  61,  58,  55,  51,  54,
     57,  60,  56,  53,  50,  49,  52,  48,
     91,  83,  90,  89,  82,  75,  67,  74,
     81,  88,  80,  73,  66,  65,  72,  64,
    112, 120, 113, 114, 121, 128, 136, 129,
    122, 115, 123, 130, 137, 138, 131, 139,
     96, 100,  97,  98, 101, 104, 108, 105,
    102,  99, 103, 106, 109, 110, 107, 111,
    116, 124, 117, 118, 125, 132, 140, 133,
    126, 119, 127, 134, 141, 142, 135, 143,
    191, 183, 190, 189, 182, 175, 167, 174,
    181, 188, 180, 173, 166, 165, 172, 164,
    159, 155, 158, 157, 154, 151, 147, 150,
    153, 156, 152, 149, 146, 145, 148, 144,
    187, 179, 186, 185, 178, 171, 163, 170,
    177, 184, 176, 169, 162, 161, 168, 160,
};

static const uint8_t * const aic_scan[NUM_BANDS] = {
    aic_y_scan, aic_c_scan, aic_y_ext_scan, aic_c_ext_scan
};

typedef struct AICContext {
    AVCodecContext *avctx;
    AVFrame        *frame;
    IDCTDSPContext idsp;
    ScanTable      scantable;

    int            num_x_slices;
    int            slice_width;
    int            mb_width, mb_height;
    int            quant;
    int            interlaced;

    int16_t        *slice_data;
    int16_t        *data_ptr[NUM_BANDS];

    DECLARE_ALIGNED(16, int16_t, block)[64];
    DECLARE_ALIGNED(16, uint8_t, quant_matrix)[64];
} AICContext;

{

        av_log(ctx->avctx, AV_LOG_ERROR, "Invalid version %d\n", src[0]);
        return AVERROR_INVALIDDATA;
    }
        av_log(ctx->avctx, AV_LOG_ERROR, "Invalid header size %d\n", src[1]);
        return AVERROR_INVALIDDATA;
    }
        av_log(ctx->avctx, AV_LOG_ERROR, "Frame size should be %"PRIu32" got %d\n",
               frame_size, size);
        return AVERROR_INVALIDDATA;
    }
        av_log(ctx->avctx, AV_LOG_ERROR,
               "Picture dimension changed: old: %d x %d, new: %d x %d\n",
               ctx->avctx->width, ctx->avctx->height, width, height);
        return AVERROR_INVALIDDATA;
    }

}

#define GET_CODE(val, type, add_bits)                         \
    do {                                                      \
        if (type)                                             \
            val = get_ue_golomb(gb);                          \
        else                                                  \
            val = get_unary(gb, 1, 31);                       \
        if (add_bits)                                         \
            val = (val << add_bits) + get_bits(gb, add_bits); \
    } while (0)

                             int band, int slice_width, int force_chroma)
{

        return AVERROR_INVALIDDATA;



            idx = -1;
                    return AVERROR_INVALIDDATA;
                    break;
                    return AVERROR_INVALIDDATA;
        }
    } else {
                    return AVERROR_INVALIDDATA;
            }
        }
    }
    return 0;
}

                            int16_t **base, int16_t **ext)
{

    }
    }

                               int16_t **base, int16_t **ext,
                               int block_no)
{

        }
    } else {
    }

{


    }
}

                            const uint8_t *src, int src_size)
{

    } else {
    }


           sizeof(*ctx->slice_data) * slice_width * AIC_BAND_COEFFS);
                                     i, slice_width,
            return ret;

                                &base_y, &ext_y);
            else
                                   &base_y, &ext_y, blk);

            } else {
            }
        }

                            &base_c, &ext_c);
        }
    }

    return 0;
}

                            AVPacket *avpkt)
{



        av_log(avctx, AV_LOG_ERROR, "Too small frame\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "Invalid header\n");
        return ret;
    }

        return ret;


                av_log(avctx, AV_LOG_ERROR,
                       "Incorrect slice size %d at %d.%d\n", slice_size, x, y);
                return AVERROR_INVALIDDATA;
            }

                av_log(avctx, AV_LOG_ERROR,
                       "Error decoding slice at %d.%d\n", x, y);
                return ret;
            }

        }
    }


}

{






        }
    }

                                * sizeof(*ctx->slice_data));
        av_log(avctx, AV_LOG_ERROR, "Error allocating slice buffer\n");

        return AVERROR(ENOMEM);
    }


    return 0;
}

{


}

AVCodec ff_aic_decoder = {
    .name           = "aic",
    .long_name      = NULL_IF_CONFIG_SMALL("Apple Intermediate Codec"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_AIC,
    .priv_data_size = sizeof(AICContext),
    .init           = aic_decode_init,
    .close          = aic_decode_close,
    .decode         = aic_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1 | AV_CODEC_CAP_FRAME_THREADS,
    .caps_internal  = FF_CODEC_CAP_INIT_THREADSAFE,
};
