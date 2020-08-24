/*
 * WebP (.webp) image decoder
 * Copyright (c) 2013 Aneesh Dogra <aneesh@sugarlabs.org>
 * Copyright (c) 2013 Justin Ruggles <justin.ruggles@gmail.com>
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
 * WebP image decoder
 *
 * @author Aneesh Dogra <aneesh@sugarlabs.org>
 * Container and Lossy decoding
 *
 * @author Justin Ruggles <justin.ruggles@gmail.com>
 * Lossless decoder
 * Compressed alpha for lossy
 *
 * @author James Almer <jamrial@gmail.com>
 * Exif metadata
 * ICC profile
 *
 * Unimplemented:
 *   - Animation
 *   - XMP metadata
 */

#include "libavutil/imgutils.h"

#define BITSTREAM_READER_LE
#include "avcodec.h"
#include "bytestream.h"
#include "exif.h"
#include "get_bits.h"
#include "internal.h"
#include "thread.h"
#include "vp8.h"

#define VP8X_FLAG_ANIMATION             0x02
#define VP8X_FLAG_XMP_METADATA          0x04
#define VP8X_FLAG_EXIF_METADATA         0x08
#define VP8X_FLAG_ALPHA                 0x10
#define VP8X_FLAG_ICC                   0x20

#define MAX_PALETTE_SIZE                256
#define MAX_CACHE_BITS                  11
#define NUM_CODE_LENGTH_CODES           19
#define HUFFMAN_CODES_PER_META_CODE     5
#define NUM_LITERAL_CODES               256
#define NUM_LENGTH_CODES                24
#define NUM_DISTANCE_CODES              40
#define NUM_SHORT_DISTANCES             120
#define MAX_HUFFMAN_CODE_LENGTH         15

static const uint16_t alphabet_sizes[HUFFMAN_CODES_PER_META_CODE] = {
    NUM_LITERAL_CODES + NUM_LENGTH_CODES,
    NUM_LITERAL_CODES, NUM_LITERAL_CODES, NUM_LITERAL_CODES,
    NUM_DISTANCE_CODES
};

static const uint8_t code_length_code_order[NUM_CODE_LENGTH_CODES] = {
    17, 18, 0, 1, 2, 3, 4, 5, 16, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

static const int8_t lz77_distance_offsets[NUM_SHORT_DISTANCES][2] = {
    {  0, 1 }, {  1, 0 }, {  1, 1 }, { -1, 1 }, {  0, 2 }, {  2, 0 }, {  1, 2 }, { -1, 2 },
    {  2, 1 }, { -2, 1 }, {  2, 2 }, { -2, 2 }, {  0, 3 }, {  3, 0 }, {  1, 3 }, { -1, 3 },
    {  3, 1 }, { -3, 1 }, {  2, 3 }, { -2, 3 }, {  3, 2 }, { -3, 2 }, {  0, 4 }, {  4, 0 },
    {  1, 4 }, { -1, 4 }, {  4, 1 }, { -4, 1 }, {  3, 3 }, { -3, 3 }, {  2, 4 }, { -2, 4 },
    {  4, 2 }, { -4, 2 }, {  0, 5 }, {  3, 4 }, { -3, 4 }, {  4, 3 }, { -4, 3 }, {  5, 0 },
    {  1, 5 }, { -1, 5 }, {  5, 1 }, { -5, 1 }, {  2, 5 }, { -2, 5 }, {  5, 2 }, { -5, 2 },
    {  4, 4 }, { -4, 4 }, {  3, 5 }, { -3, 5 }, {  5, 3 }, { -5, 3 }, {  0, 6 }, {  6, 0 },
    {  1, 6 }, { -1, 6 }, {  6, 1 }, { -6, 1 }, {  2, 6 }, { -2, 6 }, {  6, 2 }, { -6, 2 },
    {  4, 5 }, { -4, 5 }, {  5, 4 }, { -5, 4 }, {  3, 6 }, { -3, 6 }, {  6, 3 }, { -6, 3 },
    {  0, 7 }, {  7, 0 }, {  1, 7 }, { -1, 7 }, {  5, 5 }, { -5, 5 }, {  7, 1 }, { -7, 1 },
    {  4, 6 }, { -4, 6 }, {  6, 4 }, { -6, 4 }, {  2, 7 }, { -2, 7 }, {  7, 2 }, { -7, 2 },
    {  3, 7 }, { -3, 7 }, {  7, 3 }, { -7, 3 }, {  5, 6 }, { -5, 6 }, {  6, 5 }, { -6, 5 },
    {  8, 0 }, {  4, 7 }, { -4, 7 }, {  7, 4 }, { -7, 4 }, {  8, 1 }, {  8, 2 }, {  6, 6 },
    { -6, 6 }, {  8, 3 }, {  5, 7 }, { -5, 7 }, {  7, 5 }, { -7, 5 }, {  8, 4 }, {  6, 7 },
    { -6, 7 }, {  7, 6 }, { -7, 6 }, {  8, 5 }, {  7, 7 }, { -7, 7 }, {  8, 6 }, {  8, 7 }
};

enum AlphaCompression {
    ALPHA_COMPRESSION_NONE,
    ALPHA_COMPRESSION_VP8L,
};

enum AlphaFilter {
    ALPHA_FILTER_NONE,
    ALPHA_FILTER_HORIZONTAL,
    ALPHA_FILTER_VERTICAL,
    ALPHA_FILTER_GRADIENT,
};

enum TransformType {
    PREDICTOR_TRANSFORM      = 0,
    COLOR_TRANSFORM          = 1,
    SUBTRACT_GREEN           = 2,
    COLOR_INDEXING_TRANSFORM = 3,
};

enum PredictionMode {
    PRED_MODE_BLACK,
    PRED_MODE_L,
    PRED_MODE_T,
    PRED_MODE_TR,
    PRED_MODE_TL,
    PRED_MODE_AVG_T_AVG_L_TR,
    PRED_MODE_AVG_L_TL,
    PRED_MODE_AVG_L_T,
    PRED_MODE_AVG_TL_T,
    PRED_MODE_AVG_T_TR,
    PRED_MODE_AVG_AVG_L_TL_AVG_T_TR,
    PRED_MODE_SELECT,
    PRED_MODE_ADD_SUBTRACT_FULL,
    PRED_MODE_ADD_SUBTRACT_HALF,
};

enum HuffmanIndex {
    HUFF_IDX_GREEN = 0,
    HUFF_IDX_RED   = 1,
    HUFF_IDX_BLUE  = 2,
    HUFF_IDX_ALPHA = 3,
    HUFF_IDX_DIST  = 4
};

/* The structure of WebP lossless is an optional series of transformation data,
 * followed by the primary image. The primary image also optionally contains
 * an entropy group mapping if there are multiple entropy groups. There is a
 * basic image type called an "entropy coded image" that is used for all of
 * these. The type of each entropy coded image is referred to by the
 * specification as its role. */
enum ImageRole {
    /* Primary Image: Stores the actual pixels of the image. */
    IMAGE_ROLE_ARGB,

    /* Entropy Image: Defines which Huffman group to use for different areas of
     *                the primary image. */
    IMAGE_ROLE_ENTROPY,

    /* Predictors: Defines which predictor type to use for different areas of
     *             the primary image. */
    IMAGE_ROLE_PREDICTOR,

    /* Color Transform Data: Defines the color transformation for different
     *                       areas of the primary image. */
    IMAGE_ROLE_COLOR_TRANSFORM,

    /* Color Index: Stored as an image of height == 1. */
    IMAGE_ROLE_COLOR_INDEXING,

    IMAGE_ROLE_NB,
};

typedef struct HuffReader {
    VLC vlc;                            /* Huffman decoder context */
    int simple;                         /* whether to use simple mode */
    int nb_symbols;                     /* number of coded symbols */
    uint16_t simple_symbols[2];         /* symbols for simple mode */
} HuffReader;

typedef struct ImageContext {
    enum ImageRole role;                /* role of this image */
    AVFrame *frame;                     /* AVFrame for data */
    int color_cache_bits;               /* color cache size, log2 */
    uint32_t *color_cache;              /* color cache data */
    int nb_huffman_groups;              /* number of huffman groups */
    HuffReader *huffman_groups;         /* reader for each huffman group */
    int size_reduction;                 /* relative size compared to primary image, log2 */
    int is_alpha_primary;
} ImageContext;

typedef struct WebPContext {
    VP8Context v;                       /* VP8 Context used for lossy decoding */
    GetBitContext gb;                   /* bitstream reader for main image chunk */
    AVFrame *alpha_frame;               /* AVFrame for alpha data decompressed from VP8L */
    AVCodecContext *avctx;              /* parent AVCodecContext */
    int initialized;                    /* set once the VP8 context is initialized */
    int has_alpha;                      /* has a separate alpha chunk */
    enum AlphaCompression alpha_compression; /* compression type for alpha chunk */
    enum AlphaFilter alpha_filter;      /* filtering method for alpha chunk */
    uint8_t *alpha_data;                /* alpha chunk data */
    int alpha_data_size;                /* alpha chunk data size */
    int has_exif;                       /* set after an EXIF chunk has been processed */
    int has_iccp;                       /* set after an ICCP chunk has been processed */
    int width;                          /* image width */
    int height;                         /* image height */
    int lossless;                       /* indicates lossless or lossy */

    int nb_transforms;                  /* number of transforms */
    enum TransformType transforms[4];   /* transformations used in the image, in order */
    int reduced_width;                  /* reduced width for index image, if applicable */
    int nb_huffman_groups;              /* number of huffman groups in the primary image */
    ImageContext image[IMAGE_ROLE_NB];  /* image context for each role */
} WebPContext;

#define GET_PIXEL(frame, x, y) \
    ((frame)->data[0] + (y) * frame->linesize[0] + 4 * (x))

#define GET_PIXEL_COMP(frame, x, y, c) \
    (*((frame)->data[0] + (y) * frame->linesize[0] + 4 * (x) + c))

{

        }
    }


/* Differs from get_vlc2() in the following ways:
 *   - codes are bit-reversed
 *   - assumes 8-bit table to make reversal simpler
 *   - assumes max depth of 2 since the max code length for WebP is 15
 */
{





    }


}

{
        else
    } else
}

                                       int alphabet_size)
{

    /* special-case 1 symbol since the vlc reader cannot handle it */
                break;
        }
    }
        r->nb_symbols = 1;
        r->simple_symbols[0] = code;
        r->simple = 1;
        return 0;
    }


        return AVERROR(EINVAL);

        return AVERROR(ENOMEM);

        }
    }
        av_free(codes);
        return AVERROR_INVALIDDATA;
    }

                   code_lengths, sizeof(*code_lengths), sizeof(*code_lengths),
                   codes, sizeof(*codes), sizeof(*codes), 0);
        av_free(codes);
        return ret;
    }

}

{

    else



                                    int alphabet_size)
{

        return AVERROR_INVALIDDATA;


                                      NUM_CODE_LENGTH_CODES);
        goto finish;

        ret = AVERROR(ENOMEM);
        goto finish;
    }

            av_log(s->avctx, AV_LOG_ERROR, "max symbol %d > alphabet size %d\n",
                   max_symbol, alphabet_size);
            ret = AVERROR_INVALIDDATA;
            goto finish;
        }
    } else {
        max_symbol = alphabet_size;
    }

    prev_code_len = 8;
    symbol        = 0;

            break;
            /* Code length code [0..15] indicates literal code lengths. */
        } else {
                /* Code 16 repeats the previous non-zero value [3..6] times,
                 * i.e., 3 + ReadBits(2) times. If code 16 is used before a
                 * non-zero value has been emitted, a value of 8 is repeated. */
                /* Code 17 emits a streak of zeros [3..10], i.e.,
                 * 3 + ReadBits(3) times. */
                /* Code 18 emits a streak of zeros of length [11..138], i.e.,
                 * 11 + ReadBits(7) times. */
            }
                av_log(s->avctx, AV_LOG_ERROR,
                       "invalid symbol %d + repeat %d > alphabet size %d\n",
                       symbol, repeat, alphabet_size);
                ret = AVERROR_INVALIDDATA;
                goto finish;
            }
        }
    }


}

static int decode_entropy_coded_image(WebPContext *s, enum ImageRole role,
                                      int w, int h);

#define PARSE_BLOCK_SIZE(w, h) do {                                         \
    block_bits = get_bits(&s->gb, 3) + 2;                                   \
    blocks_w   = FFALIGN((w), 1 << block_bits) >> block_bits;               \
    blocks_h   = FFALIGN((h), 1 << block_bits) >> block_bits;               \
} while (0)

{

        width = s->reduced_width;


        return ret;


    /* the number of huffman groups is determined by the maximum group number
     * coded in the entropy image */
        }
    }

}

{


                                     blocks_h);
        return ret;


}

{


                                     blocks_h);
        return ret;


}

{


        width_bits = 3;
        width_bits = 2;
        width_bits = 1;
    else

                                     index_size, 1);
        return ret;


    /* color index values are delta-coded */

    return 0;
}

static HuffReader *get_huffman_group(WebPContext *s, ImageContext *img,
                                     int x, int y)
{
    ImageContext *gimg = &s->image[IMAGE_ROLE_ENTROPY];
    int group = 0;

    if (gimg->size_reduction > 0) {
        int group_x = x >> gimg->size_reduction;
        int group_y = y >> gimg->size_reduction;
        int g0      = GET_PIXEL_COMP(gimg->frame, group_x, group_y, 1);
        int g1      = GET_PIXEL_COMP(gimg->frame, group_x, group_y, 2);
        group       = g0 << 8 | g1;
    }

    return &img->huffman_groups[group * HUFFMAN_CODES_PER_META_CODE];
}

static av_always_inline void color_cache_put(ImageContext *img, uint32_t c)
{
    uint32_t cache_idx = (0x1E35A7BD * c) >> (32 - img->color_cache_bits);
    img->color_cache[cache_idx] = c;
}

                                      int w, int h)
{


            return AVERROR(ENOMEM);
    }


    } else
        return ret;

        img->color_cache_bits = get_bits(&s->gb, 4);
        if (img->color_cache_bits < 1 || img->color_cache_bits > 11) {
            av_log(s->avctx, AV_LOG_ERROR, "invalid color cache bits: %d\n",
                   img->color_cache_bits);
            return AVERROR_INVALIDDATA;
        }
        img->color_cache = av_mallocz_array(1 << img->color_cache_bits,
                                            sizeof(*img->color_cache));
        if (!img->color_cache)
            return AVERROR(ENOMEM);
    } else {
    }

            return ret;
    }
                                           HUFFMAN_CODES_PER_META_CODE,
                                           sizeof(*img->huffman_groups));
        return AVERROR(ENOMEM);

                alphabet_size += 1 << img->color_cache_bits;

            } else {
                    return ret;
            }
        }
    }



            /* literal pixel values */
                color_cache_put(img, AV_RB32(p));
            }
            /* LZ77 backwards mapping */

            /* parse length and distance */
            } else {
            }
                av_log(s->avctx, AV_LOG_ERROR,
                       "distance prefix code too large: %d\n", prefix_code);
                return AVERROR_INVALIDDATA;
            }
            } else {
                int extra_bits = prefix_code - 2 >> 1;
                int offset     = 2 + (prefix_code & 1) << extra_bits;
                distance = offset + get_bits(&s->gb, extra_bits) + 1;
            }

            /* find reference location */
            } else {
                distance -= NUM_SHORT_DISTANCES;
            }
            } else {
            }
            }
            }

            /* copy pixels
             * source and dest regions can overlap and wrap lines, so just
             * copy per-pixel */

                    color_cache_put(img, AV_RB32(p));
                }
                }
                    break;
            }
        } else {
            /* read from color cache */
            uint8_t *p = GET_PIXEL(img->frame, x, y);
            int cache_idx = v - (NUM_LITERAL_CODES + NUM_LENGTH_CODES);

            if (!img->color_cache_bits) {
                av_log(s->avctx, AV_LOG_ERROR, "color cache not found\n");
                return AVERROR_INVALIDDATA;
            }
            if (cache_idx >= 1 << img->color_cache_bits) {
                av_log(s->avctx, AV_LOG_ERROR,
                       "color cache index out-of-bounds\n");
                return AVERROR_INVALIDDATA;
            }
            AV_WB32(p, img->color_cache[cache_idx]);
            x++;
            if (x == width) {
                x = 0;
                y++;
            }
        }
    }

    return 0;
}

/* PRED_MODE_BLACK */
                          const uint8_t *p_t, const uint8_t *p_tr)
{

/* PRED_MODE_L */
                          const uint8_t *p_t, const uint8_t *p_tr)
{

/* PRED_MODE_T */
                          const uint8_t *p_t, const uint8_t *p_tr)
{

/* PRED_MODE_TR */
                          const uint8_t *p_t, const uint8_t *p_tr)
{

/* PRED_MODE_TL */
                          const uint8_t *p_t, const uint8_t *p_tr)
{

/* PRED_MODE_AVG_T_AVG_L_TR */
                          const uint8_t *p_t, const uint8_t *p_tr)
{

/* PRED_MODE_AVG_L_TL */
                          const uint8_t *p_t, const uint8_t *p_tr)
{

/* PRED_MODE_AVG_L_T */
                          const uint8_t *p_t, const uint8_t *p_tr)
{

/* PRED_MODE_AVG_TL_T */
                          const uint8_t *p_t, const uint8_t *p_tr)
{

/* PRED_MODE_AVG_T_TR */
                          const uint8_t *p_t, const uint8_t *p_tr)
{

/* PRED_MODE_AVG_AVG_L_TL_AVG_T_TR */
                           const uint8_t *p_t, const uint8_t *p_tr)
{

/* PRED_MODE_SELECT */
                           const uint8_t *p_t, const uint8_t *p_tr)
{
    else

/* PRED_MODE_ADD_SUBTRACT_FULL */
static void inv_predict_12(uint8_t *p, const uint8_t *p_l, const uint8_t *p_tl,
                           const uint8_t *p_t, const uint8_t *p_tr)
{
    p[0] = av_clip_uint8(p_l[0] + p_t[0] - p_tl[0]);
    p[1] = av_clip_uint8(p_l[1] + p_t[1] - p_tl[1]);
    p[2] = av_clip_uint8(p_l[2] + p_t[2] - p_tl[2]);
    p[3] = av_clip_uint8(p_l[3] + p_t[3] - p_tl[3]);
}

{
}

/* PRED_MODE_ADD_SUBTRACT_HALF */
                           const uint8_t *p_t, const uint8_t *p_tr)
{

typedef void (*inv_predict_func)(uint8_t *p, const uint8_t *p_l,
                                 const uint8_t *p_tl, const uint8_t *p_t,
                                 const uint8_t *p_tr);

static const inv_predict_func inverse_predict[14] = {
    inv_predict_0,  inv_predict_1,  inv_predict_2,  inv_predict_3,
    inv_predict_4,  inv_predict_5,  inv_predict_6,  inv_predict_7,
    inv_predict_8,  inv_predict_9,  inv_predict_10, inv_predict_11,
    inv_predict_12, inv_predict_13,
};

{

    else



{


                    m = PRED_MODE_BLACK;
                else
                m = PRED_MODE_L;

                av_log(s->avctx, AV_LOG_ERROR,
                       "invalid predictor mode: %d\n", m);
                return AVERROR_INVALIDDATA;
            }
        }
    }
    return 0;
}

                                                      uint8_t color)
{
}

{



        }
    }
}

{

        }
    }
}

{



            return AVERROR(ENOMEM);

                }
            }
        }
    }

    // switch to local palette if it's worth initializing it
        uint8_t palette[256 * 4];
        const int size = pal->frame->width * 4;
        av_assert0(size <= 1024U);
        memcpy(palette, GET_PIXEL(pal->frame, 0, 0), size);   // copy palette
        // set extra entries to transparent black
        memset(palette + size, 0, 256 * 4 - size);
        for (y = 0; y < img->frame->height; y++) {
            for (x = 0; x < img->frame->width; x++) {
                p = GET_PIXEL(img->frame, x, y);
                i = p[2];
                AV_COPY32(p, &palette[i * 4]);
            }
        }
    } else {
                    AV_WB32(p, 0x00000000);
                } else {
                }
            }
        }
    }

    return 0;
}

{
        av_log(avctx, AV_LOG_WARNING, "Width mismatch. %d != %d\n",
               s->width, w);
    }
        av_log(avctx, AV_LOG_WARNING, "Height mismatch. %d != %d\n",
               s->height, h);
    }

                                     int *got_frame, uint8_t *data_start,
                                     unsigned int data_size, int is_alpha_chunk)
{

    }

        return ret;

            av_log(avctx, AV_LOG_ERROR, "Invalid WebP Lossless signature\n");
            return AVERROR_INVALIDDATA;
        }



            return ret;


            av_log(avctx, AV_LOG_ERROR, "Invalid WebP Lossless version\n");
            return AVERROR_INVALIDDATA;
        }
    } else {
            return AVERROR_BUG;
        w = s->width;
        h = s->height;
    }

    /* parse transformations */
            av_log(avctx, AV_LOG_ERROR, "Transform %d used more than once\n",
                   transform);
            ret = AVERROR_INVALIDDATA;
            goto free_and_return;
        }
        }
            goto free_and_return;
    }

    /* decode primary image */
        goto free_and_return;

    /* apply transformations */
        }
            goto free_and_return;
    }



    return ret;
}

static void alpha_inverse_prediction(AVFrame *frame, enum AlphaFilter m)
{
    int x, y, ls;
    uint8_t *dec;

    ls = frame->linesize[3];

    /* filter first row using horizontal filter */
    dec = frame->data[3] + 1;
    for (x = 1; x < frame->width; x++, dec++)
        *dec += *(dec - 1);

    /* filter first column using vertical filter */
    dec = frame->data[3] + ls;
    for (y = 1; y < frame->height; y++, dec += ls)
        *dec += *(dec - ls);

    /* filter the rest using the specified filter */
    switch (m) {
    case ALPHA_FILTER_HORIZONTAL:
        for (y = 1; y < frame->height; y++) {
            dec = frame->data[3] + y * ls + 1;
            for (x = 1; x < frame->width; x++, dec++)
                *dec += *(dec - 1);
        }
        break;
    case ALPHA_FILTER_VERTICAL:
        for (y = 1; y < frame->height; y++) {
            dec = frame->data[3] + y * ls + 1;
            for (x = 1; x < frame->width; x++, dec++)
                *dec += *(dec - ls);
        }
        break;
    case ALPHA_FILTER_GRADIENT:
        for (y = 1; y < frame->height; y++) {
            dec = frame->data[3] + y * ls + 1;
            for (x = 1; x < frame->width; x++, dec++)
                dec[0] += av_clip_uint8(*(dec - 1) + *(dec - ls) - *(dec - ls - 1));
        }
        break;
    }
}

                                  uint8_t *data_start,
                                  unsigned int data_size)
{

        GetByteContext gb;

        bytestream2_init(&gb, data_start, data_size);
        for (y = 0; y < s->height; y++)
            bytestream2_get_buffer(&gb, p->data[3] + p->linesize[3] * y,
                                   s->width);

            return AVERROR(ENOMEM);

                                        data_start, data_size, 1);
            av_frame_free(&s->alpha_frame);
            return ret;
        }
            av_frame_free(&s->alpha_frame);
            return AVERROR_INVALIDDATA;
        }

        /* copy green component of alpha image to alpha plane of primary image */
            }
        }
    }

    /* apply alpha filtering */
        alpha_inverse_prediction(p, s->alpha_filter);

    return 0;
}

                                  int *got_frame, uint8_t *data_start,
                                  unsigned int data_size)
{

    }

        av_log(avctx, AV_LOG_ERROR, "unsupported chunk size\n");
        return AVERROR_PATCHWELCOME;
    }


        return ret;

        return AVERROR_INVALIDDATA;


            return ret;
    }
    return ret;
}

                             AVPacket *avpkt)
{


        return AVERROR_INVALIDDATA;

        av_log(avctx, AV_LOG_ERROR, "missing RIFF tag\n");
        return AVERROR_INVALIDDATA;
    }

        return AVERROR_INVALIDDATA;

        av_log(avctx, AV_LOG_ERROR, "missing WEBP tag\n");
        return AVERROR_INVALIDDATA;
    }


            return AVERROR_INVALIDDATA;

           /* we seem to be running out of data, but it could also be that the
              bitstream has trailing junk leading to bogus chunk_size. */
            break;
        }

                                             chunk_size);
                    return ret;
            }
            break;
                                                chunk_size, 0);
                    return ret;
            }
            break;
                av_log(avctx, AV_LOG_ERROR, "Canvas dimensions are already set\n");
                return AVERROR_INVALIDDATA;
            }
                return ret;
            break;

                av_log(avctx, AV_LOG_WARNING,
                       "ALPHA chunk present, but alpha bit not set in the "
                       "VP8X header\n");
            }
                av_log(avctx, AV_LOG_ERROR, "invalid ALPHA chunk size\n");
                return AVERROR_INVALIDDATA;
            }


                av_log(avctx, AV_LOG_VERBOSE,
                       "skipping unsupported ALPHA chunk\n");
            } else {
            }

            break;
        }
        case MKTAG('E', 'X', 'I', 'F'): {

                av_log(avctx, AV_LOG_VERBOSE, "Ignoring extra EXIF chunk\n");
                goto exif_end;
            }
                av_log(avctx, AV_LOG_WARNING,
                       "EXIF chunk present, but Exif bit not set in the "
                       "VP8X header\n");

                av_log(avctx, AV_LOG_ERROR, "invalid TIFF header "
                       "in Exif data\n");
                goto exif_end;
            }

                av_log(avctx, AV_LOG_ERROR, "error decoding Exif data\n");
                goto exif_end;
            }


        }
        case MKTAG('I', 'C', 'C', 'P'): {
            AVFrameSideData *sd;

            if (s->has_iccp) {
                av_log(avctx, AV_LOG_VERBOSE, "Ignoring extra ICCP chunk\n");
                bytestream2_skip(&gb, chunk_size);
                break;
            }
            if (!(vp8x_flags & VP8X_FLAG_ICC))
                av_log(avctx, AV_LOG_WARNING,
                       "ICCP chunk present, but ICC Profile bit not set in the "
                       "VP8X header\n");

            s->has_iccp = 1;
            sd = av_frame_new_side_data(p, AV_FRAME_DATA_ICC_PROFILE, chunk_size);
            if (!sd)
                return AVERROR(ENOMEM);

            bytestream2_get_buffer(&gb, sd->data, chunk_size);
            break;
        }
        case MKTAG('A', 'N', 'I', 'M'):
        case MKTAG('A', 'N', 'M', 'F'):
        case MKTAG('X', 'M', 'P', ' '):
            AV_WL32(chunk_str, chunk_type);
            av_log(avctx, AV_LOG_WARNING, "skipping unsupported chunk: %s\n",
                   chunk_str);
            bytestream2_skip(&gb, chunk_size);
            break;
        default:
            AV_WL32(chunk_str, chunk_type);
            av_log(avctx, AV_LOG_VERBOSE, "skipping unknown chunk: %s\n",
                   chunk_str);
            break;
        }
    }

        av_log(avctx, AV_LOG_ERROR, "image data not found\n");
        return AVERROR_INVALIDDATA;
    }

}

{


    return 0;
}

AVCodec ff_webp_decoder = {
    .name           = "webp",
    .long_name      = NULL_IF_CONFIG_SMALL("WebP image"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_WEBP,
    .priv_data_size = sizeof(WebPContext),
    .decode         = webp_decode_frame,
    .close          = webp_decode_close,
    .capabilities   = AV_CODEC_CAP_DR1 | AV_CODEC_CAP_FRAME_THREADS,
};
