/*
 * Monkey's Audio lossless audio decoder
 * Copyright (c) 2007 Benjamin Zores <ben@geexbox.org>
 *  based upon libdemac from Dave Chapman.
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

#include "libavutil/avassert.h"
#include "libavutil/channel_layout.h"
#include "libavutil/crc.h"
#include "libavutil/opt.h"
#include "lossless_audiodsp.h"
#include "avcodec.h"
#include "bswapdsp.h"
#include "bytestream.h"
#include "internal.h"
#include "get_bits.h"
#include "unary.h"

/**
 * @file
 * Monkey's Audio lossless audio decoder
 */

#define MAX_CHANNELS        2
#define MAX_BYTESPERSAMPLE  3

#define APE_FRAMECODE_MONO_SILENCE    1
#define APE_FRAMECODE_STEREO_SILENCE  3
#define APE_FRAMECODE_PSEUDO_STEREO   4

#define HISTORY_SIZE 512
#define PREDICTOR_ORDER 8
/** Total size of all predictor histories */
#define PREDICTOR_SIZE 50

#define YDELAYA (18 + PREDICTOR_ORDER*4)
#define YDELAYB (18 + PREDICTOR_ORDER*3)
#define XDELAYA (18 + PREDICTOR_ORDER*2)
#define XDELAYB (18 + PREDICTOR_ORDER)

#define YADAPTCOEFFSA 18
#define XADAPTCOEFFSA 14
#define YADAPTCOEFFSB 10
#define XADAPTCOEFFSB 5

/**
 * Possible compression levels
 * @{
 */
enum APECompressionLevel {
    COMPRESSION_LEVEL_FAST       = 1000,
    COMPRESSION_LEVEL_NORMAL     = 2000,
    COMPRESSION_LEVEL_HIGH       = 3000,
    COMPRESSION_LEVEL_EXTRA_HIGH = 4000,
    COMPRESSION_LEVEL_INSANE     = 5000
};
/** @} */

#define APE_FILTER_LEVELS 3

/** Filter orders depending on compression level */
static const uint16_t ape_filter_orders[5][APE_FILTER_LEVELS] = {
    {  0,   0,    0 },
    { 16,   0,    0 },
    { 64,   0,    0 },
    { 32, 256,    0 },
    { 16, 256, 1280 }
};

/** Filter fraction bits depending on compression level */
static const uint8_t ape_filter_fracbits[5][APE_FILTER_LEVELS] = {
    {  0,  0,  0 },
    { 11,  0,  0 },
    { 11,  0,  0 },
    { 10, 13,  0 },
    { 11, 13, 15 }
};


/** Filters applied to the decoded data */
typedef struct APEFilter {
    int16_t *coeffs;        ///< actual coefficients used in filtering
    int16_t *adaptcoeffs;   ///< adaptive filter coefficients used for correcting of actual filter coefficients
    int16_t *historybuffer; ///< filter memory
    int16_t *delay;         ///< filtered values

    int avg;
} APEFilter;

typedef struct APERice {
    uint32_t k;
    uint32_t ksum;
} APERice;

typedef struct APERangecoder {
    uint32_t low;           ///< low end of interval
    uint32_t range;         ///< length of interval
    uint32_t help;          ///< bytes_to_follow resp. intermediate value
    unsigned int buffer;    ///< buffer for input/output
} APERangecoder;

/** Filter histories */
typedef struct APEPredictor {
    int32_t *buf;

    int32_t lastA[2];

    int32_t filterA[2];
    int32_t filterB[2];

    uint32_t coeffsA[2][4];  ///< adaption coefficients
    uint32_t coeffsB[2][5];  ///< adaption coefficients
    int32_t historybuffer[HISTORY_SIZE + PREDICTOR_SIZE];

    unsigned int sample_pos;
} APEPredictor;

/** Decoder context */
typedef struct APEContext {
    AVClass *class;                          ///< class for AVOptions
    AVCodecContext *avctx;
    BswapDSPContext bdsp;
    LLAudDSPContext adsp;
    int channels;
    int samples;                             ///< samples left to decode in current frame
    int bps;

    int fileversion;                         ///< codec version, very important in decoding process
    int compression_level;                   ///< compression levels
    int fset;                                ///< which filter set to use (calculated from compression level)
    int flags;                               ///< global decoder flags

    uint32_t CRC;                            ///< signalled frame CRC
    uint32_t CRC_state;                      ///< accumulated CRC
    int frameflags;                          ///< frame flags
    APEPredictor predictor;                  ///< predictor used for final reconstruction

    int32_t *decoded_buffer;
    int decoded_size;
    int32_t *decoded[MAX_CHANNELS];          ///< decoded data for each channel
    int blocks_per_loop;                     ///< maximum number of samples to decode for each call

    int16_t* filterbuf[APE_FILTER_LEVELS];   ///< filter memory

    APERangecoder rc;                        ///< rangecoder used to decode actual values
    APERice riceX;                           ///< rice code parameters for the second channel
    APERice riceY;                           ///< rice code parameters for the first channel
    APEFilter filters[APE_FILTER_LEVELS][2]; ///< filters used for reconstruction
    GetBitContext gb;

    uint8_t *data;                           ///< current frame data
    uint8_t *data_end;                       ///< frame data end
    int data_size;                           ///< frame data allocated size
    const uint8_t *ptr;                      ///< current position in frame data

    int error;

    void (*entropy_decode_mono)(struct APEContext *ctx, int blockstodecode);
    void (*entropy_decode_stereo)(struct APEContext *ctx, int blockstodecode);
    void (*predictor_decode_mono)(struct APEContext *ctx, int count);
    void (*predictor_decode_stereo)(struct APEContext *ctx, int count);
} APEContext;

static void ape_apply_filters(APEContext *ctx, int32_t *decoded0,
                              int32_t *decoded1, int count);

static void entropy_decode_mono_0000(APEContext *ctx, int blockstodecode);
static void entropy_decode_stereo_0000(APEContext *ctx, int blockstodecode);
static void entropy_decode_mono_3860(APEContext *ctx, int blockstodecode);
static void entropy_decode_stereo_3860(APEContext *ctx, int blockstodecode);
static void entropy_decode_mono_3900(APEContext *ctx, int blockstodecode);
static void entropy_decode_stereo_3900(APEContext *ctx, int blockstodecode);
static void entropy_decode_stereo_3930(APEContext *ctx, int blockstodecode);
static void entropy_decode_mono_3990(APEContext *ctx, int blockstodecode);
static void entropy_decode_stereo_3990(APEContext *ctx, int blockstodecode);

static void predictor_decode_mono_3800(APEContext *ctx, int count);
static void predictor_decode_stereo_3800(APEContext *ctx, int count);
static void predictor_decode_mono_3930(APEContext *ctx, int count);
static void predictor_decode_stereo_3930(APEContext *ctx, int count);
static void predictor_decode_mono_3950(APEContext *ctx, int count);
static void predictor_decode_stereo_3950(APEContext *ctx, int count);

{



}

{

        av_log(avctx, AV_LOG_ERROR, "Incorrect extradata\n");
        return AVERROR(EINVAL);
    }
        av_log(avctx, AV_LOG_ERROR, "Only mono and stereo is supported\n");
        return AVERROR(EINVAL);
    }
    case 8:
        avctx->sample_fmt = AV_SAMPLE_FMT_U8P;
        break;
    case 24:
        avctx->sample_fmt = AV_SAMPLE_FMT_S32P;
        break;
    default:
        avpriv_request_sample(avctx,
                              "%d bits per coded sample", s->bps);
        return AVERROR_PATCHWELCOME;
    }

           s->compression_level, s->flags);
        av_log(avctx, AV_LOG_ERROR, "Incorrect compression level %d\n",
               s->compression_level);
        return AVERROR_INVALIDDATA;
    }
            break;
            return AVERROR(ENOMEM);
    }

    } else {
    }

    } else {
    }


}

/**
 * @name APE range decoding functions
 * @{
 */

#define CODE_BITS    32
#define TOP_VALUE    ((unsigned int)1 << (CODE_BITS-1))
#define SHIFT_BITS   (CODE_BITS - 9)
#define EXTRA_BITS   ((CODE_BITS-2) % 8 + 1)
#define BOTTOM_VALUE (TOP_VALUE >> 8)

/** Start the decoder */
{

/** Perform normalization */
{
        } else {
        }
    }

/**
 * Calculate cumulative frequency for next symbol. Does NO update!
 * @param ctx decoder context
 * @param tot_f is the total frequency or (code_value)1<<shift
 * @return the cumulative frequency
 */
{
}

/**
 * Decode value with given size in bits
 * @param ctx decoder context
 * @param shift number of bits to decode
 */
{
}


/**
 * Update decoding state
 * @param ctx decoder context
 * @param sy_f the interval length (frequency of the symbol)
 * @param lt_f the lower end (frequency sum of < symbols)
 */
{

/** Decode n bits (n <= 16) without modelling */
{
}


#define MODEL_ELEMENTS 64

/**
 * Fixed probabilities for symbols in Monkey Audio version 3.97
 */
static const uint16_t counts_3970[22] = {
        0, 14824, 28224, 39348, 47855, 53994, 58171, 60926,
    62682, 63786, 64463, 64878, 65126, 65276, 65365, 65419,
    65450, 65469, 65480, 65487, 65491, 65493,
};

/**
 * Probability ranges for symbols in Monkey Audio version 3.97
 */
static const uint16_t counts_diff_3970[21] = {
    14824, 13400, 11124, 8507, 6139, 4177, 2755, 1756,
    1104, 677, 415, 248, 150, 89, 54, 31,
    19, 11, 7, 4, 2,
};

/**
 * Fixed probabilities for symbols in Monkey Audio version 3.98
 */
static const uint16_t counts_3980[22] = {
        0, 19578, 36160, 48417, 56323, 60899, 63265, 64435,
    64971, 65232, 65351, 65416, 65447, 65466, 65476, 65482,
    65485, 65488, 65490, 65491, 65492, 65493,
};

/**
 * Probability ranges for symbols in Monkey Audio version 3.98
 */
static const uint16_t counts_diff_3980[21] = {
    19578, 16582, 12257, 7906, 4576, 2366, 1170, 536,
    261, 119, 65, 31, 19, 10, 6, 3,
    3, 2, 1, 1, 1,
};

/**
 * Decode symbol
 * @param ctx decoder context
 * @param counts probability range start position
 * @param counts_diff probability range widths
 */
                                   const uint16_t counts[],
                                   const uint16_t counts_diff[])
{


    }
    /* figure out the symbol inefficiently; a binary search would be much better */


}
/** @} */ // group rangecoder

static inline void update_rice(APERice *rice, unsigned int x)
{
    int lim = rice->k ? (1 << (rice->k + 4)) : 0;
    rice->ksum += ((x + 1) / 2) - ((rice->ksum + 16) >> 5);

    if (rice->ksum < lim)
        rice->k--;
    else if (rice->ksum >= (1 << (rice->k + 5)) && rice->k < 24)
        rice->k++;
}

{



}

static inline int ape_decode_value_3860(APEContext *ctx, GetBitContext *gb,
                                        APERice *rice)
{
    unsigned int x, overflow;

    overflow = get_unary(gb, 1, get_bits_left(gb));

    if (ctx->fileversion > 3880) {
        while (overflow >= 16) {
            overflow -= 16;
            rice->k  += 4;
        }
    }

    if (!rice->k)
        x = overflow;
    else if(rice->k <= MIN_CACHE_BITS) {
        x = (overflow << rice->k) + get_bits(gb, rice->k);
    } else {
        av_log(ctx->avctx, AV_LOG_ERROR, "Too many bits: %"PRIu32"\n", rice->k);
        ctx->error = 1;
        return AVERROR_INVALIDDATA;
    }
    rice->ksum += x - (rice->ksum + 8 >> 4);
    if (rice->ksum < (rice->k ? 1 << (rice->k + 4) : 0))
        rice->k--;
    else if (rice->ksum >= (1 << (rice->k + 5)) && rice->k < 24)
        rice->k++;

    /* Convert to signed */
    return ((x >> 1) ^ ((x & 1) - 1)) + 1;
}

{


        tmpk = range_decode_bits(ctx, 5);
        overflow = 0;
    } else

            av_log(ctx->avctx, AV_LOG_ERROR, "Too many bits: %d\n", tmpk);
            return AVERROR_INVALIDDATA;
        }
    } else if (tmpk <= 31) {
        x = range_decode_bits(ctx, 16);
        x |= (range_decode_bits(ctx, tmpk - 16) << 16);
    } else {
        av_log(ctx->avctx, AV_LOG_ERROR, "Too many bits: %d\n", tmpk);
        return AVERROR_INVALIDDATA;
    }


    /* Convert to signed */
}

{



        overflow  = (unsigned)range_decode_bits(ctx, 16) << 16;
        overflow |= range_decode_bits(ctx, 16);
    }

    } else {
        int base_hi = pivot, base_lo;
        int bbits = 0;

        while (base_hi & ~0xFFFF) {
            base_hi >>= 1;
            bbits++;
        }
        base_hi = range_decode_culfreq(ctx, base_hi + 1);
        range_decode_update(ctx, 1, base_hi);
        base_lo = range_decode_culfreq(ctx, 1 << bbits);
        range_decode_update(ctx, 1, base_lo);

        base = (base_hi << bbits) + base_lo;
    }



    /* Convert to signed */
}

static int get_k(int ksum)
{
    return av_log2(ksum) + !!ksum;
}

static void decode_array_0000(APEContext *ctx, GetBitContext *gb,
                              int32_t *out, APERice *rice, int blockstodecode)
{
    int i;
    unsigned ksummax, ksummin;

    rice->ksum = 0;
    for (i = 0; i < FFMIN(blockstodecode, 5); i++) {
        out[i] = get_rice_ook(&ctx->gb, 10);
        rice->ksum += out[i];
    }

    if (blockstodecode <= 5)
        goto end;

    rice->k = get_k(rice->ksum / 10);
    if (rice->k >= 24)
        return;
    for (; i < FFMIN(blockstodecode, 64); i++) {
        out[i] = get_rice_ook(&ctx->gb, rice->k);
        rice->ksum += out[i];
        rice->k = get_k(rice->ksum / ((i + 1) * 2));
        if (rice->k >= 24)
            return;
    }

    if (blockstodecode <= 64)
        goto end;

    rice->k = get_k(rice->ksum >> 7);
    ksummax = 1 << rice->k + 7;
    ksummin = rice->k ? (1 << rice->k + 6) : 0;
    for (; i < blockstodecode; i++) {
        if (get_bits_left(&ctx->gb) < 1) {
            ctx->error = 1;
            return;
        }
        out[i] = get_rice_ook(&ctx->gb, rice->k);
        rice->ksum += out[i] - (unsigned)out[i - 64];
        while (rice->ksum < ksummin) {
            rice->k--;
            ksummin = rice->k ? ksummin >> 1 : 0;
            ksummax >>= 1;
        }
        while (rice->ksum >= ksummax) {
            rice->k++;
            if (rice->k > 24)
                return;
            ksummax <<= 1;
            ksummin = ksummin ? ksummin << 1 : 128;
        }
    }

end:
    for (i = 0; i < blockstodecode; i++)
        out[i] = ((out[i] >> 1) ^ ((out[i] & 1) - 1)) + 1;
}

static void entropy_decode_mono_0000(APEContext *ctx, int blockstodecode)
{
    decode_array_0000(ctx, &ctx->gb, ctx->decoded[0], &ctx->riceY,
                      blockstodecode);
}

{
                      blockstodecode);
                      blockstodecode);

static void entropy_decode_mono_3860(APEContext *ctx, int blockstodecode)
{
    int32_t *decoded0 = ctx->decoded[0];

    while (blockstodecode--)
        *decoded0++ = ape_decode_value_3860(ctx, &ctx->gb, &ctx->riceY);
}

{


static void entropy_decode_mono_3900(APEContext *ctx, int blockstodecode)
{
    int32_t *decoded0 = ctx->decoded[0];

    while (blockstodecode--)
        *decoded0++ = ape_decode_value_3900(ctx, &ctx->riceY);
}

{

    // because of some implementation peculiarities we need to backpedal here

{

    }

static void entropy_decode_mono_3990(APEContext *ctx, int blockstodecode)
{
    int32_t *decoded0 = ctx->decoded[0];

    while (blockstodecode--)
        *decoded0++ = ape_decode_value_3990(ctx, &ctx->riceY);
}

{

    }

{
    /* Read the CRC */
            return AVERROR_INVALIDDATA;
    } else {
    }

    /* Read the frame flags if they exist */
        ctx->CRC &= ~0x80000000;

        if (ctx->data_end - ctx->ptr < 6)
            return AVERROR_INVALIDDATA;
        ctx->frameflags = bytestream_get_be32(&ctx->ptr);
    }

    /* Initialize the rice structs */

        /* The first 8 bits of input are ignored. */

    }

    return 0;
}

static const int32_t initial_coeffs_fast_3320[1] = {
    375,
};

static const int32_t initial_coeffs_a_3800[3] = {
    64, 115, 64,
};

static const int32_t initial_coeffs_b_3800[2] = {
    740, 0
};

static const int32_t initial_coeffs_3930[4] = {
    360, 317, -109, 98
};

{

    /* Zero the history buffers */

    /* Initialize and zero the coefficients */
            memcpy(p->coeffsA[0], initial_coeffs_fast_3320,
                   sizeof(initial_coeffs_fast_3320));
            memcpy(p->coeffsA[1], initial_coeffs_fast_3320,
                   sizeof(initial_coeffs_fast_3320));
        } else {
                   sizeof(initial_coeffs_a_3800));
                   sizeof(initial_coeffs_a_3800));
        }
    } else {
    }
               sizeof(initial_coeffs_b_3800));
               sizeof(initial_coeffs_b_3800));
    }



/** Get inverse sign of integer (-1 for positive, 1 for negative and 0 for zero) */
}

static av_always_inline int filter_fast_3320(APEPredictor *p,
                                             const int decoded, const int filter,
                                             const int delayA)
{
    int32_t predictionA;

    p->buf[delayA] = p->lastA[filter];
    if (p->sample_pos < 3) {
        p->lastA[filter]   = decoded;
        p->filterA[filter] = decoded;
        return decoded;
    }

    predictionA = p->buf[delayA] * 2U - p->buf[delayA - 1];
    p->lastA[filter] = decoded + ((int32_t)(predictionA  * p->coeffsA[filter][0]) >> 9);

    if ((decoded ^ predictionA) > 0)
        p->coeffsA[filter][0]++;
    else
        p->coeffsA[filter][0]--;

    p->filterA[filter] += (unsigned)p->lastA[filter];

    return p->filterA[filter];
}

                                        const unsigned decoded, const int filter,
                                        const int delayA,  const int delayB,
                                        const int start,   const int shift)
{

    }





}

{

        return;

        }
    }
}

{

        }
    }

{

        start = 16;
        long_filter_high_3800(decoded0, 16, 9, count);
        long_filter_high_3800(decoded1, 16, 9, count);

        }
    }

            *decoded0 = filter_fast_3320(p, Y, 0, YDELAYA);
            decoded0++;
            *decoded1 = filter_fast_3320(p, X, 1, XDELAYA);
            decoded1++;
        } else {
                                    start, shift);
                                    start, shift);
        }

        /* Combined */

        /* Have we filled the history buffer? */
                    PREDICTOR_SIZE * sizeof(*p->historybuffer));
        }
    }

static void predictor_decode_mono_3800(APEContext *ctx, int count)
{
    APEPredictor *p = &ctx->predictor;
    int32_t *decoded0 = ctx->decoded[0];
    int start = 4, shift = 10;

    if (ctx->compression_level == COMPRESSION_LEVEL_HIGH) {
        start = 16;
        long_filter_high_3800(decoded0, 16, 9, count);
    } else if (ctx->compression_level == COMPRESSION_LEVEL_EXTRA_HIGH) {
        int order = 128, shift2 = 11;

        if (ctx->fileversion >= 3830) {
            order <<= 1;
            shift++;
            shift2++;
            long_filter_ehigh_3830(decoded0 + order, count - order);
        }
        start = order;
        long_filter_high_3800(decoded0, order, shift2, count);
    }

    while (count--) {
        if (ctx->compression_level == COMPRESSION_LEVEL_FAST) {
            *decoded0 = filter_fast_3320(p, *decoded0, 0, YDELAYA);
            decoded0++;
        } else {
            *decoded0 = filter_3800(p, *decoded0, 0, YDELAYA, YDELAYB,
                                    start, shift);
            decoded0++;
        }

        /* Combined */
        p->buf++;
        p->sample_pos++;

        /* Have we filled the history buffer? */
        if (p->buf == p->historybuffer + HISTORY_SIZE) {
            memmove(p->historybuffer, p->buf,
                    PREDICTOR_SIZE * sizeof(*p->historybuffer));
            p->buf = p->historybuffer;
        }
    }
}

                                                  const int decoded, const int filter,
                                                  const int delayA)
{





}

{


        /* Predictor Y */

        /* Combined */

        /* Have we filled the history buffer? */
                    PREDICTOR_SIZE * sizeof(*p->historybuffer));
        }
    }

static void predictor_decode_mono_3930(APEContext *ctx, int count)
{
    APEPredictor *p = &ctx->predictor;
    int32_t *decoded0 = ctx->decoded[0];

    ape_apply_filters(ctx, ctx->decoded[0], NULL, count);

    while (count--) {
        *decoded0 = predictor_update_3930(p, *decoded0, 0, YDELAYA);
        decoded0++;

        p->buf++;

        /* Have we filled the history buffer? */
        if (p->buf == p->historybuffer + HISTORY_SIZE) {
            memmove(p->historybuffer, p->buf,
                    PREDICTOR_SIZE * sizeof(*p->historybuffer));
            p->buf = p->historybuffer;
        }
    }
}

                                                    const int decoded, const int filter,
                                                    const int delayA,  const int delayB,
                                                    const int adaptA,  const int adaptB)
{



    /*  Apply a scaled first-order filter compression */




}

{


        /* Predictor Y */
                                            YADAPTCOEFFSA, YADAPTCOEFFSB);
                                            XADAPTCOEFFSA, XADAPTCOEFFSB);

        /* Combined */

        /* Have we filled the history buffer? */
                    PREDICTOR_SIZE * sizeof(*p->historybuffer));
        }
    }

static void predictor_decode_mono_3950(APEContext *ctx, int count)
{
    APEPredictor *p = &ctx->predictor;
    int32_t *decoded0 = ctx->decoded[0];
    int32_t predictionA, currentA, A, sign;

    ape_apply_filters(ctx, ctx->decoded[0], NULL, count);

    currentA = p->lastA[0];

    while (count--) {
        A = *decoded0;

        p->buf[YDELAYA] = currentA;
        p->buf[YDELAYA - 1] = p->buf[YDELAYA] - (unsigned)p->buf[YDELAYA - 1];

        predictionA = p->buf[YDELAYA    ] * p->coeffsA[0][0] +
                      p->buf[YDELAYA - 1] * p->coeffsA[0][1] +
                      p->buf[YDELAYA - 2] * p->coeffsA[0][2] +
                      p->buf[YDELAYA - 3] * p->coeffsA[0][3];

        currentA = A + (unsigned)(predictionA >> 10);

        p->buf[YADAPTCOEFFSA]     = APESIGN(p->buf[YDELAYA    ]);
        p->buf[YADAPTCOEFFSA - 1] = APESIGN(p->buf[YDELAYA - 1]);

        sign = APESIGN(A);
        p->coeffsA[0][0] += p->buf[YADAPTCOEFFSA    ] * sign;
        p->coeffsA[0][1] += p->buf[YADAPTCOEFFSA - 1] * sign;
        p->coeffsA[0][2] += p->buf[YADAPTCOEFFSA - 2] * sign;
        p->coeffsA[0][3] += p->buf[YADAPTCOEFFSA - 3] * sign;

        p->buf++;

        /* Have we filled the history buffer? */
        if (p->buf == p->historybuffer + HISTORY_SIZE) {
            memmove(p->historybuffer, p->buf,
                    PREDICTOR_SIZE * sizeof(*p->historybuffer));
            p->buf = p->historybuffer;
        }

        p->filterA[0] = currentA + (unsigned)((int)(p->filterA[0] * 31U) >> 5);
        *(decoded0++) = p->filterA[0];
    }

    p->lastA[0] = currentA;
}

{


static void init_filter(APEContext *ctx, APEFilter *f, int16_t *buf, int order)
{
    do_init_filter(&f[0], buf, order);
    do_init_filter(&f[1], buf + order * 3 + HISTORY_SIZE, order);
}

static void do_apply_filter(APEContext *ctx, int version, APEFilter *f,
                            int32_t *data, int count, int order, int fracbits)
{
    int res;
    int absres;

    while (count--) {
        /* round fixedpoint scalar product */
        res = ctx->adsp.scalarproduct_and_madd_int16(f->coeffs,
                                                     f->delay - order,
                                                     f->adaptcoeffs - order,
                                                     order, APESIGN(*data));
        res = (int)(res + (1U << (fracbits - 1))) >> fracbits;
        res += (unsigned)*data;
        *data++ = res;

        /* Update the output history */
        *f->delay++ = av_clip_int16(res);

        if (version < 3980) {
            /* Version ??? to < 3.98 files (untested) */
            f->adaptcoeffs[0]  = (res == 0) ? 0 : ((res >> 28) & 8) - 4;
            f->adaptcoeffs[-4] >>= 1;
            f->adaptcoeffs[-8] >>= 1;
        } else {
            /* Version 3.98 and later files */

            /* Update the adaption coefficients */
            absres = res < 0 ? -(unsigned)res : res;
            if (absres)
                *f->adaptcoeffs = APESIGN(res) *
                                  (8 << ((absres > f->avg * 3) + (absres > f->avg * 4 / 3)));
                /* equivalent to the following code
                    if (absres <= f->avg * 4 / 3)
                        *f->adaptcoeffs = APESIGN(res) * 8;
                    else if (absres <= f->avg * 3)
                        *f->adaptcoeffs = APESIGN(res) * 16;
                    else
                        *f->adaptcoeffs = APESIGN(res) * 32;
                */
            else
                *f->adaptcoeffs = 0;

            f->avg += (int)(absres - (unsigned)f->avg) / 16;

            f->adaptcoeffs[-1] >>= 1;
            f->adaptcoeffs[-2] >>= 1;
            f->adaptcoeffs[-8] >>= 1;
        }

        f->adaptcoeffs++;

        /* Have we filled the history buffer? */
        if (f->delay == f->historybuffer + HISTORY_SIZE + (order * 2)) {
            memmove(f->historybuffer, f->delay - (order * 2),
                    (order * 2) * sizeof(*f->historybuffer));
            f->delay = f->historybuffer + order * 2;
            f->adaptcoeffs = f->historybuffer + order;
        }
    }
}

                         int32_t *data0, int32_t *data1,
                         int count, int order, int fracbits)
{

                              int32_t *decoded1, int count)
{

            break;
                     ape_filter_orders[ctx->fset][i],
    }

{
        return ret;

            break;
                    ape_filter_orders[ctx->fset][i]);
    }
    return 0;
}

static void ape_unpack_mono(APEContext *ctx, int count)
{
    if (ctx->frameflags & APE_FRAMECODE_STEREO_SILENCE) {
        /* We are pure silence, so we're done. */
        av_log(ctx->avctx, AV_LOG_DEBUG, "pure silence mono\n");
        return;
    }

    ctx->entropy_decode_mono(ctx, count);
    if (ctx->error)
        return;

    /* Now apply the predictor decoding */
    ctx->predictor_decode_mono(ctx, count);

    /* Pseudo-stereo - just copy left channel to right channel */
    if (ctx->channels == 2) {
        memcpy(ctx->decoded[1], ctx->decoded[0], count * sizeof(*ctx->decoded[1]));
    }
}

{

        /* We are pure silence, so we're done. */
        av_log(ctx->avctx, AV_LOG_DEBUG, "pure silence stereo\n");
        return;
    }

        return;

    /* Now apply the predictor decoding */

    /* Decorrelate and scale to output depth */

    }
}

                            int *got_frame_ptr, AVPacket *avpkt)
{

    /* this should never be negative, but bad things will happen if it is, so
       check it just to make sure. */


        }
            av_log(avctx, AV_LOG_ERROR, "Packet is too small\n");
            return AVERROR_INVALIDDATA;
        }
            av_log(avctx, AV_LOG_WARNING, "packet size is not a multiple of 4. "
                   "extra bytes at the end will be skipped.\n");
        }
            return AVERROR(ENOMEM);
                          buf_size >> 2);

                av_log(avctx, AV_LOG_ERROR, "Incorrect offset passed\n");
                av_freep(&s->data);
                s->data_size = 0;
                return AVERROR_INVALIDDATA;
            }
                av_log(avctx, AV_LOG_ERROR, "Packet is too small\n");
                return AVERROR_INVALIDDATA;
            }
        } else {
                return ret;
            else
        }

            av_log(avctx, AV_LOG_ERROR, "Invalid sample count: %"PRIu32".\n",
                   nblocks);
            return AVERROR_INVALIDDATA;
        }

        /* Initialize the frame decoder */
            av_log(avctx, AV_LOG_ERROR, "Error reading frame header\n");
            return AVERROR_INVALIDDATA;
        }
    }

        *got_frame_ptr = 0;
        return avpkt->size;
    }

    // for old files coefficients were not interleaved,
    // so we need to decode all of them at once

    /* reallocate decoded sample buffer if needed */

    /* get output buffer */
        s->samples=0;
        return ret;
    }

        return AVERROR(ENOMEM);


        ape_unpack_mono(s, blockstodecode);
    else

    }

    case 8:
        for (ch = 0; ch < s->channels; ch++) {
            sample8 = (uint8_t *)frame->data[ch];
            for (i = 0; i < blockstodecode; i++)
                *sample8++ = (s->decoded[ch][i] + 0x80) & 0xff;
        }
        break;
    case 16:
        }
        break;
    case 24:
        for (ch = 0; ch < s->channels; ch++) {
            sample24 = (int32_t *)frame->data[ch];
            for (i = 0; i < blockstodecode; i++)
                *sample24++ = s->decoded[ch][i] * 256U;
        }
        break;
    }


        s->fileversion >= 3900 && s->bps < 24) {
        uint32_t crc = s->CRC_state;
        const AVCRC *crc_tab = av_crc_get_table(AV_CRC_32_IEEE_LE);
        for (i = 0; i < blockstodecode; i++) {
            for (ch = 0; ch < s->channels; ch++) {
                uint8_t *smp = frame->data[ch] + (i*(s->bps >> 3));
                crc = av_crc(crc_tab, crc, smp, s->bps >> 3);
            }
        }

        if (!s->samples && (~crc >> 1) ^ s->CRC) {
            av_log(avctx, AV_LOG_ERROR, "CRC mismatch! Previously decoded "
                   "frames may have been affected as well.\n");
            if (avctx->err_recognition & AV_EF_EXPLODE)
                return AVERROR_INVALIDDATA;
        }

        s->CRC_state = crc;
    }


}

static void ape_flush(AVCodecContext *avctx)
{
    APEContext *s = avctx->priv_data;
    s->samples= 0;
}

#define OFFSET(x) offsetof(APEContext, x)
#define PAR (AV_OPT_FLAG_DECODING_PARAM | AV_OPT_FLAG_AUDIO_PARAM)
static const AVOption options[] = {
    { "max_samples", "maximum number of samples decoded per call",             OFFSET(blocks_per_loop), AV_OPT_TYPE_INT,   { .i64 = 4608 },    1,       INT_MAX, PAR, "max_samples" },
    { "all",         "no maximum. decode all samples for each packet at once", 0,                       AV_OPT_TYPE_CONST, { .i64 = INT_MAX }, INT_MIN, INT_MAX, PAR, "max_samples" },
    { NULL},
};

static const AVClass ape_decoder_class = {
    .class_name = "APE decoder",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
};

AVCodec ff_ape_decoder = {
    .name           = "ape",
    .long_name      = NULL_IF_CONFIG_SMALL("Monkey's Audio"),
    .type           = AVMEDIA_TYPE_AUDIO,
    .id             = AV_CODEC_ID_APE,
    .priv_data_size = sizeof(APEContext),
    .init           = ape_decode_init,
    .close          = ape_decode_close,
    .decode         = ape_decode_frame,
    .capabilities   = AV_CODEC_CAP_SUBFRAMES | AV_CODEC_CAP_DELAY |
                      AV_CODEC_CAP_DR1,
    .caps_internal  = FF_CODEC_CAP_INIT_CLEANUP,
    .flush          = ape_flush,
    .sample_fmts    = (const enum AVSampleFormat[]) { AV_SAMPLE_FMT_U8P,
                                                      AV_SAMPLE_FMT_S16P,
                                                      AV_SAMPLE_FMT_S32P,
                                                      AV_SAMPLE_FMT_NONE },
    .priv_class     = &ape_decoder_class,
};
