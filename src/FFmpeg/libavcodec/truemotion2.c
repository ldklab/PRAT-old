/*
 * Duck/ON2 TrueMotion 2 Decoder
 * Copyright (c) 2005 Konstantin Shishkov
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
 * Duck TrueMotion2 decoder.
 */

#include <inttypes.h>

#include "avcodec.h"
#include "bswapdsp.h"
#include "bytestream.h"
#include "get_bits.h"
#include "internal.h"

#define TM2_ESCAPE 0x80000000
#define TM2_DELTAS 64

/* Huffman-coded streams of different types of blocks */
enum TM2_STREAMS {
    TM2_C_HI = 0,
    TM2_C_LO,
    TM2_L_HI,
    TM2_L_LO,
    TM2_UPD,
    TM2_MOT,
    TM2_TYPE,
    TM2_NUM_STREAMS
};

/* Block types */
enum TM2_BLOCKS {
    TM2_HI_RES = 0,
    TM2_MED_RES,
    TM2_LOW_RES,
    TM2_NULL_RES,
    TM2_UPDATE,
    TM2_STILL,
    TM2_MOTION
};

typedef struct TM2Context {
    AVCodecContext *avctx;
    AVFrame *pic;

    GetBitContext gb;
    int error;
    BswapDSPContext bdsp;

    uint8_t *buffer;
    int buffer_size;

    /* TM2 streams */
    int *tokens[TM2_NUM_STREAMS];
    int tok_lens[TM2_NUM_STREAMS];
    int tok_ptrs[TM2_NUM_STREAMS];
    int deltas[TM2_NUM_STREAMS][TM2_DELTAS];
    /* for blocks decoding */
    int D[4];
    int CD[4];
    int *last;
    int *clast;

    /* data for current and previous frame */
    int *Y1_base, *U1_base, *V1_base, *Y2_base, *U2_base, *V2_base;
    int *Y1, *U1, *V1, *Y2, *U2, *V2;
    int y_stride, uv_stride;
    int cur;
} TM2Context;

/**
* Huffman codes for each of streams
*/
typedef struct TM2Codes {
    VLC vlc; ///< table for FFmpeg bitstream reader
    int bits;
    int *recode; ///< table for converting from code indexes to values
    int length;
} TM2Codes;

/**
* structure for gathering Huffman codes information
*/
typedef struct TM2Huff {
    int val_bits; ///< length of literal
    int max_bits; ///< maximum length of code
    int min_bits; ///< minimum length of code
    int nodes; ///< total number of nodes in tree
    int num; ///< current number filled
    int max_num; ///< total number of codes
    int *nums; ///< literals
    uint32_t *bits; ///< codes
    int *lens; ///< codelengths
} TM2Huff;

/**
 *
 * @returns the length of the longest code or an AVERROR code
 */
{
        av_log(ctx->avctx, AV_LOG_ERROR, "Tree exceeded its given depth (%i)\n",
               huff->max_bits);
        return AVERROR_INVALIDDATA;
    }

        }
            av_log(ctx->avctx, AV_LOG_DEBUG, "Too many literals\n");
            return AVERROR_INVALIDDATA;
        }
    } else { /* non-terminal node */
            return ret2;
            return ret;
    }
}

{


    /* check for correct codes parameters */
        av_log(ctx->avctx, AV_LOG_ERROR, "Incorrect tree parameters - literal "
               "length: %i, max code length: %i\n", huff.val_bits, huff.max_bits);
        return AVERROR_INVALIDDATA;
    }
        av_log(ctx->avctx, AV_LOG_ERROR, "Incorrect number of Huffman tree "
               "nodes: %i\n", huff.nodes);
        return AVERROR_INVALIDDATA;
    }
    /* one-node tree */

    /* allocate space for codes - it is exactly ceil(nodes / 2) entries */

        res = AVERROR(ENOMEM);
        goto out;
    }


        av_log(ctx->avctx, AV_LOG_ERROR, "Got less bits than expected: %i of %i\n",
               res, huff.max_bits);
        res = AVERROR_INVALIDDATA;
    }
        av_log(ctx->avctx, AV_LOG_ERROR, "Got less codes than expected: %i of %i\n",
               huff.num, huff.max_num);
        res = AVERROR_INVALIDDATA;
    }

    /* convert codes to vlc_table */

                       huff.lens, sizeof(int), sizeof(int),
                       huff.bits, sizeof(uint32_t), sizeof(uint32_t), 0);
            av_log(ctx->avctx, AV_LOG_ERROR, "Cannot build VLC table\n");
        else {
                res = AVERROR(ENOMEM);
                goto out;
            }
        }
    }

    /* free allocated memory */

}

{

{
        return -1;
}

#define TM2_OLD_HEADER_MAGIC 0x00000100
#define TM2_NEW_HEADER_MAGIC 0x00000101

static inline int tm2_read_header(TM2Context *ctx, const uint8_t *buf)
{
    uint32_t magic = AV_RL32(buf);

    switch (magic) {
    case TM2_OLD_HEADER_MAGIC:
        avpriv_request_sample(ctx->avctx, "Old TM2 header");
        return 0;
    case TM2_NEW_HEADER_MAGIC:
        return 0;
    default:
        av_log(ctx->avctx, AV_LOG_ERROR, "Not a TM2 header: 0x%08"PRIX32"\n",
               magic);
        return AVERROR_INVALIDDATA;
    }
}

{


        av_log(ctx->avctx, AV_LOG_ERROR, "Incorrect delta table: %i deltas x %i bits\n", d, mb);
        return AVERROR_INVALIDDATA;
    }

        else
    }

    return 0;
}

{

        av_log(ctx->avctx, AV_LOG_ERROR, "not enough space for len left\n");
        return AVERROR_INVALIDDATA;
    }

    /* get stream length in dwords */

        return 4;

        av_log(ctx->avctx, AV_LOG_ERROR, "Error, invalid stream size.\n");
        return AVERROR_INVALIDDATA;
    }

        }
                return AVERROR_INVALIDDATA;
                return ret;
        }
    }
    /* skip unused fields */
        bytestream2_skip(&gb, 8); /* unused by decoder */
    } else {
    }

        return AVERROR_INVALIDDATA;
        return ret;

    /* check if we have sane number of tokens */
        av_log(ctx->avctx, AV_LOG_ERROR, "Incorrect number of tokens: %i\n", toks);
        ret = AVERROR_INVALIDDATA;
        goto end;
    }
        ctx->tok_lens[stream_id] = 0;
        goto end;
    }
            ret = AVERROR_INVALIDDATA;
            goto end;
        }
                av_log(ctx->avctx, AV_LOG_ERROR, "Incorrect number of tokens: %i\n", toks);
                ret = AVERROR_INVALIDDATA;
                goto end;
            }
                av_log(ctx->avctx, AV_LOG_ERROR, "Invalid delta token index %d for type %d, n=%d\n",
                       ctx->tokens[stream_id][i], stream_id, i);
                ret = AVERROR_INVALIDDATA;
                goto end;
            }
        }
    } else {
            ret = AVERROR_INVALIDDATA;
            goto end;
        }
                av_log(ctx->avctx, AV_LOG_ERROR, "Invalid delta token index %d for type %d, n=%d\n",
                       ctx->tokens[stream_id][i], stream_id, i);
                ret = AVERROR_INVALIDDATA;
                goto end;
            }
        }
    }

    ret = skip;

}

{
        av_log(ctx->avctx, AV_LOG_ERROR, "Read token from stream %i out of bounds (%i>=%i)\n", type, ctx->tok_ptrs[type], ctx->tok_lens[type]);
        ctx->error = 1;
        return 0;
    }
            av_log(ctx->avctx, AV_LOG_ERROR, "token %d is too large\n", ctx->tokens[type][ctx->tok_ptrs[type]]);
            return 0;
        }
    }
}

/* blocks decoding routines */

/* common Y, U, V pointers initialisation */
#define TM2_INIT_POINTERS() \
    int *last, *clast; \
    int *Y, *U, *V;\
    int Ystride, Ustride, Vstride;\
\
    Ystride = ctx->y_stride;\
    Vstride = ctx->uv_stride;\
    Ustride = ctx->uv_stride;\
    Y = (ctx->cur?ctx->Y2:ctx->Y1) + by * 4 * Ystride + bx * 4;\
    V = (ctx->cur?ctx->V2:ctx->V1) + by * 2 * Vstride + bx * 2;\
    U = (ctx->cur?ctx->U2:ctx->U1) + by * 2 * Ustride + bx * 2;\
    last = ctx->last + bx * 4;\
    clast = ctx->clast + bx * 4;

#define TM2_INIT_POINTERS_2() \
    unsigned *Yo, *Uo, *Vo;\
    int oYstride, oUstride, oVstride;\
\
    TM2_INIT_POINTERS();\
    oYstride = Ystride;\
    oVstride = Vstride;\
    oUstride = Ustride;\
    Yo = (ctx->cur?ctx->Y1:ctx->Y2) + by * 4 * oYstride + bx * 4;\
    Vo = (ctx->cur?ctx->V1:ctx->V2) + by * 2 * oVstride + bx * 2;\
    Uo = (ctx->cur?ctx->U1:ctx->U2) + by * 2 * oUstride + bx * 2;

/* recalculate last and delta values for next blocks */
#define TM2_RECALC_BLOCK(CHR, stride, last, CD) {\
    CD[0] = (unsigned)CHR[         1] - (unsigned)last[1];\
    CD[1] = (unsigned)CHR[stride + 1] - (unsigned) CHR[1];\
    last[0] = (int)CHR[stride + 0];\
    last[1] = (int)CHR[stride + 1];}

/* common operations - add deltas to 4x4 block of luma or 2x2 blocks of chroma */
{

        }
    }

{
        }
    }

{

    else
        prev = 0;


static inline void tm2_hi_res_block(TM2Context *ctx, AVFrame *pic, int bx, int by)
{
    int i;
    int deltas[16];
    TM2_INIT_POINTERS();

    /* hi-res chroma */
    for (i = 0; i < 4; i++) {
        deltas[i]     = GET_TOK(ctx, TM2_C_HI);
        deltas[i + 4] = GET_TOK(ctx, TM2_C_HI);
    }
    tm2_high_chroma(U, Ustride, clast,     ctx->CD,     deltas);
    tm2_high_chroma(V, Vstride, clast + 2, ctx->CD + 2, deltas + 4);

    /* hi-res luma */
    for (i = 0; i < 16; i++)
        deltas[i] = GET_TOK(ctx, TM2_L_HI);

    tm2_apply_deltas(ctx, Y, Ystride, deltas, last);
}

static inline void tm2_med_res_block(TM2Context *ctx, AVFrame *pic, int bx, int by)
{
    int i;
    int deltas[16];
    TM2_INIT_POINTERS();

    /* low-res chroma */
    deltas[0] = GET_TOK(ctx, TM2_C_LO);
    deltas[1] = deltas[2] = deltas[3] = 0;
    tm2_low_chroma(U, Ustride, clast, ctx->CD, deltas, bx);

    deltas[0] = GET_TOK(ctx, TM2_C_LO);
    deltas[1] = deltas[2] = deltas[3] = 0;
    tm2_low_chroma(V, Vstride, clast + 2, ctx->CD + 2, deltas, bx);

    /* hi-res luma */
    for (i = 0; i < 16; i++)
        deltas[i] = GET_TOK(ctx, TM2_L_HI);

    tm2_apply_deltas(ctx, Y, Ystride, deltas, last);
}

static inline void tm2_low_res_block(TM2Context *ctx, AVFrame *pic, int bx, int by)
{
    int i;
    int t1, t2;
    int deltas[16];
    TM2_INIT_POINTERS();

    /* low-res chroma */
    deltas[0] = GET_TOK(ctx, TM2_C_LO);
    deltas[1] = deltas[2] = deltas[3] = 0;
    tm2_low_chroma(U, Ustride, clast, ctx->CD, deltas, bx);

    deltas[0] = GET_TOK(ctx, TM2_C_LO);
    deltas[1] = deltas[2] = deltas[3] = 0;
    tm2_low_chroma(V, Vstride, clast + 2, ctx->CD + 2, deltas, bx);

    /* low-res luma */
    for (i = 0; i < 16; i++)
        deltas[i] = 0;

    deltas[ 0] = GET_TOK(ctx, TM2_L_LO);
    deltas[ 2] = GET_TOK(ctx, TM2_L_LO);
    deltas[ 8] = GET_TOK(ctx, TM2_L_LO);
    deltas[10] = GET_TOK(ctx, TM2_L_LO);

    if (bx > 0)
        last[0] = (int)((unsigned)last[-1] - ctx->D[0] - ctx->D[1] - ctx->D[2] - ctx->D[3] + last[1]) >> 1;
    else
        last[0] = (int)((unsigned)last[1]  - ctx->D[0] - ctx->D[1] - ctx->D[2] - ctx->D[3])>> 1;
    last[2] = (int)((unsigned)last[1] + last[3]) >> 1;

    t1 = ctx->D[0] + (unsigned)ctx->D[1];
    ctx->D[0] = t1 >> 1;
    ctx->D[1] = t1 - (t1 >> 1);
    t2 = ctx->D[2] + (unsigned)ctx->D[3];
    ctx->D[2] = t2 >> 1;
    ctx->D[3] = t2 - (t2 >> 1);

    tm2_apply_deltas(ctx, Y, Ystride, deltas, last);
}

static inline void tm2_null_res_block(TM2Context *ctx, AVFrame *pic, int bx, int by)
{
    int i;
    int ct;
    unsigned left, right;
    int diff;
    int deltas[16];
    TM2_INIT_POINTERS();

    /* null chroma */
    deltas[0] = deltas[1] = deltas[2] = deltas[3] = 0;
    tm2_low_chroma(U, Ustride, clast, ctx->CD, deltas, bx);

    deltas[0] = deltas[1] = deltas[2] = deltas[3] = 0;
    tm2_low_chroma(V, Vstride, clast + 2, ctx->CD + 2, deltas, bx);

    /* null luma */
    for (i = 0; i < 16; i++)
        deltas[i] = 0;

    ct = (unsigned)ctx->D[0] + ctx->D[1] + ctx->D[2] + ctx->D[3];

    if (bx > 0)
        left = last[-1] - (unsigned)ct;
    else
        left = 0;

    right   = last[3];
    diff    = right - left;
    last[0] = left + (diff >> 2);
    last[1] = left + (diff >> 1);
    last[2] = right - (diff >> 2);
    last[3] = right;
    {
        unsigned tp = left;

        ctx->D[0] = (tp + (ct >> 2)) - left;
        left     += ctx->D[0];
        ctx->D[1] = (tp + (ct >> 1)) - left;
        left     += ctx->D[1];
        ctx->D[2] = ((tp + ct) - (ct >> 2)) - left;
        left     += ctx->D[2];
        ctx->D[3] = (tp + ct) - left;
    }
    tm2_apply_deltas(ctx, Y, Ystride, deltas, last);
}

static inline void tm2_still_block(TM2Context *ctx, AVFrame *pic, int bx, int by)
{
    int i, j;
    TM2_INIT_POINTERS_2();

    /* update chroma */
    for (j = 0; j < 2; j++) {
        for (i = 0; i < 2; i++){
            U[i] = Uo[i];
            V[i] = Vo[i];
        }
        U  += Ustride; V += Vstride;
        Uo += oUstride; Vo += oVstride;
    }
    U -= Ustride * 2;
    V -= Vstride * 2;
    TM2_RECALC_BLOCK(U, Ustride, clast, ctx->CD);
    TM2_RECALC_BLOCK(V, Vstride, (clast + 2), (ctx->CD + 2));

    /* update deltas */
    ctx->D[0] = Yo[3] - last[3];
    ctx->D[1] = Yo[3 + oYstride] - Yo[3];
    ctx->D[2] = Yo[3 + oYstride * 2] - Yo[3 + oYstride];
    ctx->D[3] = Yo[3 + oYstride * 3] - Yo[3 + oYstride * 2];

    for (j = 0; j < 4; j++) {
        for (i = 0; i < 4; i++) {
            Y[i]    = Yo[i];
            last[i] = Yo[i];
        }
        Y  += Ystride;
        Yo += oYstride;
    }
}

static inline void tm2_update_block(TM2Context *ctx, AVFrame *pic, int bx, int by)
{
    int i, j;
    unsigned d;
    TM2_INIT_POINTERS_2();

    /* update chroma */
    for (j = 0; j < 2; j++) {
        for (i = 0; i < 2; i++) {
            U[i] = Uo[i] + GET_TOK(ctx, TM2_UPD);
            V[i] = Vo[i] + GET_TOK(ctx, TM2_UPD);
        }
        U  += Ustride;
        V  += Vstride;
        Uo += oUstride;
        Vo += oVstride;
    }
    U -= Ustride * 2;
    V -= Vstride * 2;
    TM2_RECALC_BLOCK(U, Ustride, clast, ctx->CD);
    TM2_RECALC_BLOCK(V, Vstride, (clast + 2), (ctx->CD + 2));

    /* update deltas */
    ctx->D[0] = Yo[3] - last[3];
    ctx->D[1] = Yo[3 + oYstride] - Yo[3];
    ctx->D[2] = Yo[3 + oYstride * 2] - Yo[3 + oYstride];
    ctx->D[3] = Yo[3 + oYstride * 3] - Yo[3 + oYstride * 2];

    for (j = 0; j < 4; j++) {
        d = last[3];
        for (i = 0; i < 4; i++) {
            Y[i]    = Yo[i] + (unsigned)GET_TOK(ctx, TM2_UPD);
            last[i] = Y[i];
        }
        ctx->D[j] = last[3] - d;
        Y  += Ystride;
        Yo += oYstride;
    }
}

static inline void tm2_motion_block(TM2Context *ctx, AVFrame *pic, int bx, int by)
{
    int i, j;
    int mx, my;
    TM2_INIT_POINTERS_2();

    mx = GET_TOK(ctx, TM2_MOT);
    my = GET_TOK(ctx, TM2_MOT);
    mx = av_clip(mx, -(bx * 4 + 4), ctx->avctx->width  - bx * 4);
    my = av_clip(my, -(by * 4 + 4), ctx->avctx->height - by * 4);

    if (4*bx+mx<0 || 4*by+my<0 || 4*bx+mx+4 > ctx->avctx->width || 4*by+my+4 > ctx->avctx->height) {
        av_log(ctx->avctx, AV_LOG_ERROR, "MV out of picture\n");
        return;
    }

    Yo += my * oYstride + mx;
    Uo += (my >> 1) * oUstride + (mx >> 1);
    Vo += (my >> 1) * oVstride + (mx >> 1);

    /* copy chroma */
    for (j = 0; j < 2; j++) {
        for (i = 0; i < 2; i++) {
            U[i] = Uo[i];
            V[i] = Vo[i];
        }
        U  += Ustride;
        V  += Vstride;
        Uo += oUstride;
        Vo += oVstride;
    }
    U -= Ustride * 2;
    V -= Vstride * 2;
    TM2_RECALC_BLOCK(U, Ustride, clast, ctx->CD);
    TM2_RECALC_BLOCK(V, Vstride, (clast + 2), (ctx->CD + 2));

    /* copy luma */
    for (j = 0; j < 4; j++) {
        for (i = 0; i < 4; i++) {
            Y[i] = Yo[i];
        }
        Y  += Ystride;
        Yo += oYstride;
    }
    /* calculate deltas */
    Y -= Ystride * 4;
    ctx->D[0] = (unsigned)Y[3] - last[3];
    ctx->D[1] = (unsigned)Y[3 + Ystride] - Y[3];
    ctx->D[2] = (unsigned)Y[3 + Ystride * 2] - Y[3 + Ystride];
    ctx->D[3] = (unsigned)Y[3 + Ystride * 3] - Y[3 + Ystride * 2];
    for (i = 0; i < 4; i++)
        last[i] = Y[i + Ystride * 3];
}

static int tm2_decode_blocks(TM2Context *ctx, AVFrame *p)
{
    int i, j;
    int w = ctx->avctx->width, h = ctx->avctx->height, bw = w >> 2, bh = h >> 2, cw = w >> 1;
    int type;
    int keyframe = 1;
    int *Y, *U, *V;
    uint8_t *dst;

    for (i = 0; i < TM2_NUM_STREAMS; i++)
        ctx->tok_ptrs[i] = 0;

    if (ctx->tok_lens[TM2_TYPE]<bw*bh) {
        av_log(ctx->avctx,AV_LOG_ERROR,"Got %i tokens for %i blocks\n",ctx->tok_lens[TM2_TYPE],bw*bh);
        return AVERROR_INVALIDDATA;
    }

    memset(ctx->last, 0, 4 * bw * sizeof(int));
    memset(ctx->clast, 0, 4 * bw * sizeof(int));

    for (j = 0; j < bh; j++) {
        memset(ctx->D, 0, 4 * sizeof(int));
        memset(ctx->CD, 0, 4 * sizeof(int));
        for (i = 0; i < bw; i++) {
            type = GET_TOK(ctx, TM2_TYPE);
            switch(type) {
            case TM2_HI_RES:
                tm2_hi_res_block(ctx, p, i, j);
                break;
            case TM2_MED_RES:
                tm2_med_res_block(ctx, p, i, j);
                break;
            case TM2_LOW_RES:
                tm2_low_res_block(ctx, p, i, j);
                break;
            case TM2_NULL_RES:
                tm2_null_res_block(ctx, p, i, j);
                break;
            case TM2_UPDATE:
                tm2_update_block(ctx, p, i, j);
                keyframe = 0;
                break;
            case TM2_STILL:
                tm2_still_block(ctx, p, i, j);
                keyframe = 0;
                break;
            case TM2_MOTION:
                tm2_motion_block(ctx, p, i, j);
                keyframe = 0;
                break;
            default:
                av_log(ctx->avctx, AV_LOG_ERROR, "Skipping unknown block type %i\n", type);
            }
            if (ctx->error)
                return AVERROR_INVALIDDATA;
        }
    }

    /* copy data from our buffer to AVFrame */
    Y = (ctx->cur?ctx->Y2:ctx->Y1);
    U = (ctx->cur?ctx->U2:ctx->U1);
    V = (ctx->cur?ctx->V2:ctx->V1);
    dst = p->data[0];
    for (j = 0; j < h; j++) {
        for (i = 0; i < w; i++) {
            unsigned y = Y[i], u = U[i >> 1], v = V[i >> 1];
            dst[3*i+0] = av_clip_uint8(y + v);
            dst[3*i+1] = av_clip_uint8(y);
            dst[3*i+2] = av_clip_uint8(y + u);
        }

        /* horizontal edge extension */
        Y[-4]    = Y[-3]    = Y[-2]    = Y[-1] = Y[0];
        Y[w + 3] = Y[w + 2] = Y[w + 1] = Y[w]  = Y[w - 1];

        /* vertical edge extension */
        if (j == 0) {
            memcpy(Y - 4 - 1 * ctx->y_stride, Y - 4, ctx->y_stride);
            memcpy(Y - 4 - 2 * ctx->y_stride, Y - 4, ctx->y_stride);
            memcpy(Y - 4 - 3 * ctx->y_stride, Y - 4, ctx->y_stride);
            memcpy(Y - 4 - 4 * ctx->y_stride, Y - 4, ctx->y_stride);
        } else if (j == h - 1) {
            memcpy(Y - 4 + 1 * ctx->y_stride, Y - 4, ctx->y_stride);
            memcpy(Y - 4 + 2 * ctx->y_stride, Y - 4, ctx->y_stride);
            memcpy(Y - 4 + 3 * ctx->y_stride, Y - 4, ctx->y_stride);
            memcpy(Y - 4 + 4 * ctx->y_stride, Y - 4, ctx->y_stride);
        }

        Y += ctx->y_stride;
        if (j & 1) {
            /* horizontal edge extension */
            U[-2]     = U[-1] = U[0];
            V[-2]     = V[-1] = V[0];
            U[cw + 1] = U[cw] = U[cw - 1];
            V[cw + 1] = V[cw] = V[cw - 1];

            /* vertical edge extension */
            if (j == 1) {
                memcpy(U - 2 - 1 * ctx->uv_stride, U - 2, ctx->uv_stride);
                memcpy(V - 2 - 1 * ctx->uv_stride, V - 2, ctx->uv_stride);
                memcpy(U - 2 - 2 * ctx->uv_stride, U - 2, ctx->uv_stride);
                memcpy(V - 2 - 2 * ctx->uv_stride, V - 2, ctx->uv_stride);
            } else if (j == h - 1) {
                memcpy(U - 2 + 1 * ctx->uv_stride, U - 2, ctx->uv_stride);
                memcpy(V - 2 + 1 * ctx->uv_stride, V - 2, ctx->uv_stride);
                memcpy(U - 2 + 2 * ctx->uv_stride, U - 2, ctx->uv_stride);
                memcpy(V - 2 + 2 * ctx->uv_stride, V - 2, ctx->uv_stride);
            }

            U += ctx->uv_stride;
            V += ctx->uv_stride;
        }
        dst += p->linesize[0];
    }

    return keyframe;
}

static const int tm2_stream_order[TM2_NUM_STREAMS] = {
    TM2_C_HI, TM2_C_LO, TM2_L_HI, TM2_L_LO, TM2_UPD, TM2_MOT, TM2_TYPE
};

#define TM2_HEADER_SIZE 40

                        void *data, int *got_frame,
                        AVPacket *avpkt)
{


        av_log(avctx, AV_LOG_ERROR, "Cannot allocate temporary buffer\n");
        return AVERROR(ENOMEM);
    }

        return ret;

                      buf_size >> 2);

        return ret;
    }

            av_log(avctx, AV_LOG_ERROR, "no space for tm2_read_stream\n");
            return AVERROR_INVALIDDATA;
        }

                            buf_size - offset);
            int j = tm2_stream_order[i];
            if (l->tok_lens[j])
                memset(l->tokens[j], 0, sizeof(**l->tokens) * l->tok_lens[j]);
            return t;
        }
    }
    else


}

{

        av_log(avctx, AV_LOG_ERROR, "Width and height must be multiple of 4\n");
        return AVERROR(EINVAL);
    }


        return AVERROR(ENOMEM);



    }

        av_freep(&l->Y1_base);
        av_freep(&l->Y2_base);
        av_freep(&l->U1_base);
        av_freep(&l->U2_base);
        av_freep(&l->V1_base);
        av_freep(&l->V2_base);
        av_freep(&l->last);
        av_freep(&l->clast);
        av_frame_free(&l->pic);
        return AVERROR(ENOMEM);
    }

}

{

    }


}

AVCodec ff_truemotion2_decoder = {
    .name           = "truemotion2",
    .long_name      = NULL_IF_CONFIG_SMALL("Duck TrueMotion 2.0"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_TRUEMOTION2,
    .priv_data_size = sizeof(TM2Context),
    .init           = decode_init,
    .close          = decode_end,
    .decode         = decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
