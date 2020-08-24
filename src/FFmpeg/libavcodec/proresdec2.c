/*
 * Copyright (c) 2010-2011 Maxim Poliakovski
 * Copyright (c) 2010-2011 Elvis Presley
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
 * Known FOURCCs: 'apch' (HQ), 'apcn' (SD), 'apcs' (LT), 'acpo' (Proxy), 'ap4h' (4444)
 */

//#define DEBUG

#define LONG_BITSTREAM_READER

#include "libavutil/internal.h"
#include "avcodec.h"
#include "get_bits.h"
#include "idctdsp.h"
#include "internal.h"
#include "profiles.h"
#include "simple_idct.h"
#include "proresdec.h"
#include "proresdata.h"
#include "thread.h"

{
}

#define ALPHA_SHIFT_16_TO_10(alpha_val) (alpha_val >> 6)
#define ALPHA_SHIFT_8_TO_10(alpha_val)  ((alpha_val << 2) | (alpha_val >> 6))
#define ALPHA_SHIFT_16_TO_12(alpha_val) (alpha_val >> 4)
#define ALPHA_SHIFT_8_TO_12(alpha_val)  ((alpha_val << 4) | (alpha_val >> 4))

                                const int num_bits, const int decode_precision) {

            } else {
                    val = -val;
            }
                    dst[idx++] = ALPHA_SHIFT_16_TO_10(alpha_val);
                } else { /* 12b */
                }
            } else {
                if (decode_precision == 10) {
                    dst[idx++] = ALPHA_SHIFT_8_TO_10(alpha_val);
                } else { /* 12b */
                    dst[idx++] = ALPHA_SHIFT_8_TO_12(alpha_val);
                }
            }
                break;
                    dst[idx++] = ALPHA_SHIFT_16_TO_10(alpha_val);
                } else { /* 12b */
                }
            }
        } else {
            for (i = 0; i < val; i++) {
                if (decode_precision == 10) {
                    dst[idx++] = ALPHA_SHIFT_8_TO_10(alpha_val);
                } else { /* 12b */
                    dst[idx++] = ALPHA_SHIFT_8_TO_12(alpha_val);
                }
            }
        }

static void unpack_alpha_10(GetBitContext *gb, uint16_t *dst, int num_coeffs,
                            const int num_bits)
{
    if (num_bits == 16) {
        unpack_alpha(gb, dst, num_coeffs, 16, 10);
    } else { /* 8 bits alpha */
        unpack_alpha(gb, dst, num_coeffs, 8, 10);
    }
}

                            const int num_bits)
{
    } else { /* 8 bits alpha */
        unpack_alpha(gb, dst, num_coeffs, 8, 12);
    }

{


    case MKTAG('a','p','4','x'):
        avctx->profile = FF_PROFILE_PRORES_XQ;
        avctx->bits_per_raw_sample = 12;
        break;
    default:
        avctx->profile = FF_PROFILE_UNKNOWN;
        av_log(avctx, AV_LOG_WARNING, "Unknown prores profile %d\n", avctx->codec_tag);
    }

    } else { /* 12b */
    }

        av_log(avctx, AV_LOG_ERROR, "Fail to init proresdsp for bits per raw sample %d\n", avctx->bits_per_raw_sample);
        return ret;
    }



    } else {
        av_log(avctx, AV_LOG_ERROR, "Fail to set unpack_alpha for bits per raw sample %d\n", avctx->bits_per_raw_sample);
        return AVERROR_BUG;
    }
    return ret;
}

                               const int data_size, AVCodecContext *avctx)
{

        av_log(avctx, AV_LOG_ERROR, "error, wrong header size\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "unsupported version: %d\n", version);
        return AVERROR_PATCHWELCOME;
    }


        int ret;

        av_log(avctx, AV_LOG_WARNING, "picture resolution change: %dx%d -> %dx%d\n",
               avctx->width, avctx->height, width, height);
        if ((ret = ff_set_dimensions(avctx, width, height)) < 0)
            return ret;
    }


        av_log(avctx, AV_LOG_ERROR, "Invalid alpha mode %d\n", ctx->alpha_info);
        return AVERROR_INVALIDDATA;
    }


    } else {
    }

            avctx->pix_fmt = (buf[12] & 0xC0) == 0xC0 ? AV_PIX_FMT_YUVA444P10 : AV_PIX_FMT_YUVA422P10;
        } else { /* 12b */
        }
    } else {
        } else { /* 12b */
        }
    }



            av_log(avctx, AV_LOG_ERROR, "Header truncated\n");
            return AVERROR_INVALIDDATA;
        }
    } else {
        memset(ctx->qmat_luma, 4, 64);
    }

            av_log(avctx, AV_LOG_ERROR, "Header truncated\n");
            return AVERROR_INVALIDDATA;
        }
    } else {
        memcpy(ctx->qmat_chroma, ctx->qmat_luma, 64);
    }

    return hdr_size;
}

{

        av_log(avctx, AV_LOG_ERROR, "error, wrong picture header size\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "error, wrong picture data size\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "unsupported slice resolution: %dx%d\n",
               1 << log2_slice_mb_width, 1 << log2_slice_mb_height);
        return AVERROR_INVALIDDATA;
    }

    else

    // QT ignores the written value
    // slice_count = AV_RB16(buf + 5);

            return AVERROR(ENOMEM);
    }

        return AVERROR(EINVAL);

        av_log(avctx, AV_LOG_ERROR, "error, wrong slice count\n");
        return AVERROR_INVALIDDATA;
    }

    // parse slice information






            av_log(avctx, AV_LOG_ERROR, "error, wrong slice data size\n");
            return AVERROR_INVALIDDATA;
        }

        }
            av_log(avctx, AV_LOG_ERROR, "error, slice out of bounds\n");
            return AVERROR_INVALIDDATA;
        }
    }

        av_log(avctx, AV_LOG_ERROR, "error wrong mb count y %d h %d\n",
               mb_y, ctx->mb_height);
        return AVERROR_INVALIDDATA;
    }

}

#define DECODE_CODEWORD(val, codebook, SKIP)                            \
    do {                                                                \
        unsigned int rice_order, exp_order, switch_bits;                \
        unsigned int q, buf, bits;                                      \
                                                                        \
        UPDATE_CACHE(re, gb);                                           \
        buf = GET_CACHE(re, gb);                                        \
                                                                        \
        /* number of bits to switch between rice and exp golomb */      \
        switch_bits =  codebook & 3;                                    \
        rice_order  =  codebook >> 5;                                   \
        exp_order   = (codebook >> 2) & 7;                              \
                                                                        \
        q = 31 - av_log2(buf);                                          \
                                                                        \
        if (q > switch_bits) { /* exp golomb */                         \
            bits = exp_order - switch_bits + (q<<1);                    \
            if (bits > FFMIN(MIN_CACHE_BITS, 31))                       \
                return AVERROR_INVALIDDATA;                             \
            val = SHOW_UBITS(re, gb, bits) - (1 << exp_order) +         \
                ((switch_bits + 1) << rice_order);                      \
            SKIP(re, gb, bits);                                         \
        } else if (rice_order) {                                        \
            SKIP_BITS(re, gb, q+1);                                     \
            val = (q << rice_order) + SHOW_UBITS(re, gb, rice_order);   \
            SKIP(re, gb, rice_order);                                   \
        } else {                                                        \
            val = q;                                                    \
            SKIP(re, gb, q+1);                                          \
        }                                                               \
    } while (0)

#define TOSIGNED(x) (((x) >> 1) ^ (-((x) & 1)))

#define FIRST_DC_CB 0xB8

static const uint8_t dc_codebook[7] = { 0x04, 0x28, 0x28, 0x4D, 0x4D, 0x70, 0x70};

static av_always_inline int decode_dc_coeffs(GetBitContext *gb, int16_t *out,
                                              int blocks_per_slice)
{
    int16_t prev_dc;
    int code, i, sign;

    OPEN_READER(re, gb);

    DECODE_CODEWORD(code, FIRST_DC_CB, LAST_SKIP_BITS);
    prev_dc = TOSIGNED(code);
    out[0] = prev_dc;

    out += 64; // dc coeff for the next block

    code = 5;
    sign = 0;
    for (i = 1; i < blocks_per_slice; i++, out += 64) {
        DECODE_CODEWORD(code, dc_codebook[FFMIN(code, 6U)], LAST_SKIP_BITS);
        if(code) sign ^= -(code & 1);
        else     sign  = 0;
        prev_dc += (((code + 1) >> 1) ^ sign) - sign;
        out[0] = prev_dc;
    }
    CLOSE_READER(re, gb);
    return 0;
}

// adaptive codebook switching lut according to previous run/level values
static const uint8_t run_to_cb[16] = { 0x06, 0x06, 0x05, 0x05, 0x04, 0x29, 0x29, 0x29, 0x29, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x4C };
static const uint8_t lev_to_cb[10] = { 0x04, 0x0A, 0x05, 0x06, 0x04, 0x28, 0x28, 0x28, 0x28, 0x4C };

static av_always_inline int decode_ac_coeffs(AVCodecContext *avctx, GetBitContext *gb,
                                             int16_t *out, int blocks_per_slice)
{
    ProresContext *ctx = avctx->priv_data;
    int block_mask, sign;
    unsigned pos, run, level;
    int max_coeffs, i, bits_left;
    int log2_block_count = av_log2(blocks_per_slice);

    OPEN_READER(re, gb);
    UPDATE_CACHE(re, gb);                                           \
    run   = 4;
    level = 2;

    max_coeffs = 64 << log2_block_count;
    block_mask = blocks_per_slice - 1;

    for (pos = block_mask;;) {
        bits_left = gb->size_in_bits - re_index;
        if (!bits_left || (bits_left < 32 && !SHOW_UBITS(re, gb, bits_left)))
            break;

        DECODE_CODEWORD(run, run_to_cb[FFMIN(run,  15)], LAST_SKIP_BITS);
        pos += run + 1;
        if (pos >= max_coeffs) {
            av_log(avctx, AV_LOG_ERROR, "ac tex damaged %d, %d\n", pos, max_coeffs);
            return AVERROR_INVALIDDATA;
        }

        DECODE_CODEWORD(level, lev_to_cb[FFMIN(level, 9)], SKIP_BITS);
        level += 1;

        i = pos >> log2_block_count;

        sign = SHOW_SBITS(re, gb, 1);
        SKIP_BITS(re, gb, 1);
        out[((pos & block_mask) << 6) + ctx->scan[i]] = ((level ^ sign) - sign);
    }

    CLOSE_READER(re, gb);
    return 0;
}

static int decode_slice_luma(AVCodecContext *avctx, SliceContext *slice,
                             uint16_t *dst, int dst_stride,
                             const uint8_t *buf, unsigned buf_size,
                             const int16_t *qmat)
{
    ProresContext *ctx = avctx->priv_data;
    LOCAL_ALIGNED_32(int16_t, blocks, [8*4*64]);
    int16_t *block;
    GetBitContext gb;
    int i, blocks_per_slice = slice->mb_count<<2;
    int ret;

    for (i = 0; i < blocks_per_slice; i++)
        ctx->bdsp.clear_block(blocks+(i<<6));

    init_get_bits(&gb, buf, buf_size << 3);

    if ((ret = decode_dc_coeffs(&gb, blocks, blocks_per_slice)) < 0)
        return ret;
    if ((ret = decode_ac_coeffs(avctx, &gb, blocks, blocks_per_slice)) < 0)
        return ret;

    block = blocks;
    for (i = 0; i < slice->mb_count; i++) {
        ctx->prodsp.idct_put(dst, dst_stride, block+(0<<6), qmat);
        ctx->prodsp.idct_put(dst             +8, dst_stride, block+(1<<6), qmat);
        ctx->prodsp.idct_put(dst+4*dst_stride  , dst_stride, block+(2<<6), qmat);
        ctx->prodsp.idct_put(dst+4*dst_stride+8, dst_stride, block+(3<<6), qmat);
        block += 4*64;
        dst += 16;
    }
    return 0;
}

static int decode_slice_chroma(AVCodecContext *avctx, SliceContext *slice,
                               uint16_t *dst, int dst_stride,
                               const uint8_t *buf, unsigned buf_size,
                               const int16_t *qmat, int log2_blocks_per_mb)
{
    ProresContext *ctx = avctx->priv_data;
    LOCAL_ALIGNED_32(int16_t, blocks, [8*4*64]);
    int16_t *block;
    GetBitContext gb;
    int i, j, blocks_per_slice = slice->mb_count << log2_blocks_per_mb;
    int ret;

    for (i = 0; i < blocks_per_slice; i++)
        ctx->bdsp.clear_block(blocks+(i<<6));

    init_get_bits(&gb, buf, buf_size << 3);

    if ((ret = decode_dc_coeffs(&gb, blocks, blocks_per_slice)) < 0)
        return ret;
    if ((ret = decode_ac_coeffs(avctx, &gb, blocks, blocks_per_slice)) < 0)
        return ret;

    block = blocks;
    for (i = 0; i < slice->mb_count; i++) {
        for (j = 0; j < log2_blocks_per_mb; j++) {
            ctx->prodsp.idct_put(dst,              dst_stride, block+(0<<6), qmat);
            ctx->prodsp.idct_put(dst+4*dst_stride, dst_stride, block+(1<<6), qmat);
            block += 2*64;
            dst += 8;
        }
    }
    return 0;
}

/**
 * Decode alpha slice plane.
 */
                               uint16_t *dst, int dst_stride,
                               const uint8_t *buf, int buf_size,
                               int blocks_per_slice)
{



    } else {
        ctx->unpack_alpha(&gb, blocks, blocks_per_slice * 4 * 64, 8);
    }

    block = blocks;

    }

{

    //av_log(avctx, AV_LOG_INFO, "slice %d mb width %d mb x %d y %d\n",
    //       jobnr, slice->mb_count, slice->mb_x, slice->mb_y);

    // slice header

        av_log(avctx, AV_LOG_ERROR, "invalid plane data size\n");
        return AVERROR_INVALIDDATA;
    }


    }

    } else {
    }

        mb_x_shift = 5;
        log2_chroma_blocks_per_mb = 2;
    } else {
    }


    }

                            buf, y_data_size, qmat_luma_scaled);
        return ret;

                                  buf + y_data_size, u_data_size,
                                  qmat_chroma_scaled, log2_chroma_blocks_per_mb);
            return ret;

                                  qmat_chroma_scaled, log2_chroma_blocks_per_mb);
            return ret;
    }
    else {
            val_no_chroma = 511;
        } else { /* 12b */
            val_no_chroma = 511 * 4;
        }
            }
    }

    /* decode alpha plane if available */

}

{



        ctx->frame->decode_error_flags = FF_DECODE_ERROR_INVALID_BITSTREAM;
        return 0;

    return ctx->slices[0].ret;
}

                        AVPacket *avpkt)
{

        av_log(avctx, AV_LOG_ERROR, "invalid frame header\n");
        return AVERROR_INVALIDDATA;
    }



        return frame_hdr_size;


        av_log(avctx, AV_LOG_ERROR, "error decoding picture header\n");
        return pic_size;
    }

            return ret;

        av_log(avctx, AV_LOG_ERROR, "error decoding picture\n");
        return ret;
    }


    }


}

{


}

AVCodec ff_prores_decoder = {
    .name           = "prores",
    .long_name      = NULL_IF_CONFIG_SMALL("ProRes (iCodec Pro)"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_PRORES,
    .priv_data_size = sizeof(ProresContext),
    .init           = decode_init,
    .close          = decode_close,
    .decode         = decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1 | AV_CODEC_CAP_SLICE_THREADS | AV_CODEC_CAP_FRAME_THREADS,
    .profiles       = NULL_IF_CONFIG_SMALL(ff_prores_profiles),
};
