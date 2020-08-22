/*
 * MPEG Audio decoder
 * Copyright (c) 2001, 2002 Fabrice Bellard
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
 * MPEG Audio decoder
 */

#include "libavutil/attributes.h"
#include "libavutil/avassert.h"
#include "libavutil/channel_layout.h"
#include "libavutil/crc.h"
#include "libavutil/float_dsp.h"
#include "libavutil/libm.h"
#include "avcodec.h"
#include "get_bits.h"
#include "internal.h"
#include "mathops.h"
#include "mpegaudiodsp.h"

/*
 * TODO:
 *  - test lsf / mpeg25 extensively.
 */

#include "mpegaudio.h"
#include "mpegaudiodecheader.h"

#define BACKSTEP_SIZE 512
#define EXTRABYTES 24
#define LAST_BUF_SIZE 2 * BACKSTEP_SIZE + EXTRABYTES

/* layer 3 "granule" */
typedef struct GranuleDef {
    uint8_t scfsi;
    int part2_3_length;
    int big_values;
    int global_gain;
    int scalefac_compress;
    uint8_t block_type;
    uint8_t switch_point;
    int table_select[3];
    int subblock_gain[3];
    uint8_t scalefac_scale;
    uint8_t count1table_select;
    int region_size[3]; /* number of huffman codes in each region */
    int preflag;
    int short_start, long_end; /* long/short band indexes */
    uint8_t scale_factors[40];
    DECLARE_ALIGNED(16, INTFLOAT, sb_hybrid)[SBLIMIT * 18]; /* 576 samples */
} GranuleDef;

typedef struct MPADecodeContext {
    MPA_DECODE_HEADER
    uint8_t last_buf[LAST_BUF_SIZE];
    int last_buf_size;
    int extrasize;
    /* next header (used in free format parsing) */
    uint32_t free_format_next_header;
    GetBitContext gb;
    GetBitContext in_gb;
    DECLARE_ALIGNED(32, MPA_INT, synth_buf)[MPA_MAX_CHANNELS][512 * 2];
    int synth_buf_offset[MPA_MAX_CHANNELS];
    DECLARE_ALIGNED(32, INTFLOAT, sb_samples)[MPA_MAX_CHANNELS][36][SBLIMIT];
    INTFLOAT mdct_buf[MPA_MAX_CHANNELS][SBLIMIT * 18]; /* previous samples, for layer 3 MDCT */
    GranuleDef granules[2][2]; /* Used in Layer 3 */
    int adu_mode; ///< 0 for standard mp3, 1 for adu formatted mp3
    int dither_state;
    int err_recognition;
    AVCodecContext* avctx;
    MPADSPContext mpadsp;
    AVFloatDSPContext *fdsp;
    AVFrame *frame;
    uint32_t crc;
} MPADecodeContext;

#define HEADER_SIZE 4

#include "mpegaudiodata.h"
#include "mpegaudiodectab.h"

/* vlc structure for decoding layer 3 huffman tables */
static VLC huff_vlc[16];
static VLC_TYPE huff_vlc_tables[
    0 + 128 + 128 + 128 + 130 + 128 + 154 + 166 +
  142 + 204 + 190 + 170 + 542 + 460 + 662 + 414
  ][2];
static const int huff_vlc_tables_sizes[16] = {
    0,  128,  128,  128,  130,  128,  154,  166,
  142,  204,  190,  170,  542,  460,  662,  414
};
static VLC huff_quad_vlc[2];
static VLC_TYPE  huff_quad_vlc_tables[128+16][2];
static const int huff_quad_vlc_tables_sizes[2] = { 128, 16 };
/* computed from band_size_long */
static uint16_t band_index_long[9][23];
#include "mpegaudio_tablegen.h"
/* intensity stereo coef table */
static INTFLOAT is_table[2][16];
static INTFLOAT is_table_lsf[2][2][16];
static INTFLOAT csa_table[8][4];

static int16_t division_tab3[1 << 6 ];
static int16_t division_tab5[1 << 8 ];
static int16_t division_tab9[1 << 11];

static int16_t * const division_tabs[4] = {
    division_tab3, division_tab5, NULL, division_tab9
};

/* lower 2 bits: modulo 3, higher bits: shift */
static uint16_t scale_factor_modshift[64];
/* [i][j]:  2^(-j/3) * FRAC_ONE * 2^(i+2) / (2^(i+2) - 1) */
static int32_t scale_factor_mult[15][3];
/* mult table for layer 2 group quantization */

#define SCALE_GEN(v) \
{ FIXR_OLD(1.0 * (v)), FIXR_OLD(0.7937005259 * (v)), FIXR_OLD(0.6299605249 * (v)) }

static const int32_t scale_factor_mult2[3][3] = {
    SCALE_GEN(4.0 / 3.0), /* 3 steps */
    SCALE_GEN(4.0 / 5.0), /* 5 steps */
    SCALE_GEN(4.0 / 9.0), /* 9 steps */
};

/**
 * Convert region offsets to region sizes and truncate
 * size to big_values.
 */
{
    }
}

{
        else
            g->region_size[0] = (72 / 2);
    } else {
        else
            g->region_size[0] = (108 / 2);
    }

                             int ra1, int ra2)
{
    /* should not overflow */

static void compute_band_indexes(MPADecodeContext *s, GranuleDef *g)
{
    if (g->block_type == 2) {
        if (g->switch_point) {
            if(s->sample_rate_index == 8)
                avpriv_request_sample(s->avctx, "switch point in 8khz");
            /* if switched mode, we handle the 36 first samples as
                long blocks.  For 8000Hz, we handle the 72 first
                exponents as long blocks */
            if (s->sample_rate_index <= 2)
                g->long_end = 8;
            else
                g->long_end = 6;

            g->short_start = 3;
        } else {
            g->long_end    = 0;
            g->short_start = 0;
        }
    } else {
        g->short_start = 13;
        g->long_end    = 22;
    }
}

/* layer 1 unscaling */
/* n = number of bits of the mantissa minus 1 */
{

    /* NOTE: at this point, 1 <= shift >= 21 + 15 */
}

{


    /* NOTE: at this point, 0 <= shift <= 21 */
}

/* compute value^(4/3) * 2^(exponent/4). It normalized to FRAC_BITS */
{

#ifdef DEBUG
    if(e < 1)
        av_log(NULL, AV_LOG_WARNING, "l3_unscale: e is %d\n", e);
#endif
        return 0;

}

{

    /* scale factors table for layer 1/2 */
        /* 1.0 (i = 3) is normalized to 2 ^ FRAC_BITS */
    }

    /* scale factor multiply for layer 1 */
                (unsigned)norm,
                scale_factor_mult[i][0],
                scale_factor_mult[i][1],
                scale_factor_mult[i][2]);
    }


    /* huffman decode tables */


            }
        }

        /* XXX: fail test */
                 tmp_bits, 1, 1, tmp_codes, 2, 2,
                 INIT_VLC_USE_NEW_STATIC);
    }

    offset = 0;
                 mpa_quad_bits[i], 1, 1, mpa_quad_codes[i], 1, 1,
                 INIT_VLC_USE_NEW_STATIC);
    }

        k = 0;
        }
    }

    /* compute n ^ (4/3) and store it in mantissa/exp format */


            }
        }
    }


        } else {
            v = FIXR(1.0);
        }
    }
    /* invalid values */

        double f;
        int e, k;

                    i, j, (float) is_table_lsf[j][0][i],
                    (float) is_table_lsf[j][1][i]);
        }
    }

#if !USE_FLOATS
#else
#endif
    }

#if USE_FLOATS
{

}
#endif

{

    }


#if USE_FLOATS
        return AVERROR(ENOMEM);
#endif


        avctx->codec_id != AV_CODEC_ID_MP3ON4)
        avctx->sample_fmt = OUT_FMT;
    else

        s->adu_mode = 1;

}

#define C3 FIXHR(0.86602540378443864676/2)
#define C4 FIXHR(0.70710678118654752439/2) //0.5 / cos(pi*(9)/36)
#define C5 FIXHR(0.51763809020504152469/2) //0.5 / cos(pi*(5)/36)
#define C6 FIXHR(1.93185165257813657349/4) //0.5 / cos(pi*(15)/36)

/* 12 points IMDCT. We compute it "by hand" by factorizing obvious
   cases. */
{







{
        const uint8_t *buf = s->gb.buffer - HEADER_SIZE;
        int sec_byte_len  = sec_len >> 3;
        int sec_rem_bits  = sec_len & 7;
        const AVCRC *crc_tab = av_crc_get_table(AV_CRC_16_ANSI);
        uint8_t tmp_buf[4];
        uint32_t crc_val = av_crc(crc_tab, UINT16_MAX, &buf[2], 2);
        crc_val = av_crc(crc_tab, crc_val, &buf[6], sec_byte_len);

        AV_WB32(tmp_buf,
                ((buf[6 + sec_byte_len] & (0xFF00 >> sec_rem_bits)) << 24) +
                ((s->crc << 16) >> sec_rem_bits));

        crc_val = av_crc(crc_tab, crc_val, tmp_buf, 3);

        if (crc_val) {
            av_log(s->avctx, AV_LOG_ERROR, "CRC mismatch %X!\n", crc_val);
            if (s->err_recognition & AV_EF_EXPLODE)
                return AVERROR_INVALIDDATA;
        }
    }
    return 0;
}

/* return the number of decoded frames */
static int mp_decode_layer1(MPADecodeContext *s)
{
    int bound, i, v, n, ch, j, mant;
    uint8_t allocation[MPA_MAX_CHANNELS][SBLIMIT];
    uint8_t scale_factors[MPA_MAX_CHANNELS][SBLIMIT];
    int ret;

    ret = handle_crc(s, (s->nb_channels == 1) ? 8*16  : 8*32);
    if (ret < 0)
        return ret;

    if (s->mode == MPA_JSTEREO)
        bound = (s->mode_ext + 1) * 4;
    else
        bound = SBLIMIT;

    /* allocation bits */
    for (i = 0; i < bound; i++) {
        for (ch = 0; ch < s->nb_channels; ch++) {
            allocation[ch][i] = get_bits(&s->gb, 4);
        }
    }
    for (i = bound; i < SBLIMIT; i++)
        allocation[0][i] = get_bits(&s->gb, 4);

    /* scale factors */
    for (i = 0; i < bound; i++) {
        for (ch = 0; ch < s->nb_channels; ch++) {
            if (allocation[ch][i])
                scale_factors[ch][i] = get_bits(&s->gb, 6);
        }
    }
    for (i = bound; i < SBLIMIT; i++) {
        if (allocation[0][i]) {
            scale_factors[0][i] = get_bits(&s->gb, 6);
            scale_factors[1][i] = get_bits(&s->gb, 6);
        }
    }

    /* compute samples */
    for (j = 0; j < 12; j++) {
        for (i = 0; i < bound; i++) {
            for (ch = 0; ch < s->nb_channels; ch++) {
                n = allocation[ch][i];
                if (n) {
                    mant = get_bits(&s->gb, n + 1);
                    v = l1_unscale(n, mant, scale_factors[ch][i]);
                } else {
                    v = 0;
                }
                s->sb_samples[ch][j][i] = v;
            }
        }
        for (i = bound; i < SBLIMIT; i++) {
            n = allocation[0][i];
            if (n) {
                mant = get_bits(&s->gb, n + 1);
                v = l1_unscale(n, mant, scale_factors[0][i]);
                s->sb_samples[0][j][i] = v;
                v = l1_unscale(n, mant, scale_factors[1][i]);
                s->sb_samples[1][j][i] = v;
            } else {
                s->sb_samples[0][j][i] = 0;
                s->sb_samples[1][j][i] = 0;
            }
        }
    }
    return 12;
}

{

    /* select decoding table */
                                   s->sample_rate, s->lsf);

        bound = (s->mode_ext + 1) * 4;
    else
        bound = sblimit;


    /* sanity check */
        bound = sblimit;

    /* parse bit allocation */
    }
        bit_alloc_bits = alloc_table[j];
        v = get_bits(&s->gb, bit_alloc_bits);
        bit_alloc[0][i] = v;
        bit_alloc[1][i] = v;
        j += 1 << bit_alloc_bits;
    }

    /* scale codes */
        }
    }

        return ret;

    /* scale factors */
                case 0:
                }
        }
    }

    /* samples */
            j = 0;
                            /* 3 values at the same time */

                        } else {
                            }
                        }
                    } else {
                    }
                }
                /* next subband in alloc table */
            }
            /* XXX: find a way to avoid this duplication of code */
                bit_alloc_bits = alloc_table[j];
                b = bit_alloc[0][i];
                if (b) {
                    int mant, scale0, scale1;
                    scale0 = scale_factors[0][i][k];
                    scale1 = scale_factors[1][i][k];
                    qindex = alloc_table[j + b];
                    bits = ff_mpa_quant_bits[qindex];
                    if (bits < 0) {
                        /* 3 values at the same time */
                        v = get_bits(&s->gb, -bits);
                        steps = ff_mpa_quant_steps[qindex];
                        mant = v % steps;
                        v = v / steps;
                        s->sb_samples[0][k * 12 + l + 0][i] =
                            l2_unscale_group(steps, mant, scale0);
                        s->sb_samples[1][k * 12 + l + 0][i] =
                            l2_unscale_group(steps, mant, scale1);
                        mant = v % steps;
                        v = v / steps;
                        s->sb_samples[0][k * 12 + l + 1][i] =
                            l2_unscale_group(steps, mant, scale0);
                        s->sb_samples[1][k * 12 + l + 1][i] =
                            l2_unscale_group(steps, mant, scale1);
                        s->sb_samples[0][k * 12 + l + 2][i] =
                            l2_unscale_group(steps, v, scale0);
                        s->sb_samples[1][k * 12 + l + 2][i] =
                            l2_unscale_group(steps, v, scale1);
                    } else {
                        for (m = 0; m < 3; m++) {
                            mant = get_bits(&s->gb, bits);
                            s->sb_samples[0][k * 12 + l + m][i] =
                                l1_unscale(bits - 1, mant, scale0);
                            s->sb_samples[1][k * 12 + l + m][i] =
                                l1_unscale(bits - 1, mant, scale1);
                        }
                    }
                } else {
                    s->sb_samples[0][k * 12 + l + 0][i] = 0;
                    s->sb_samples[0][k * 12 + l + 1][i] = 0;
                    s->sb_samples[0][k * 12 + l + 2][i] = 0;
                    s->sb_samples[1][k * 12 + l + 0][i] = 0;
                    s->sb_samples[1][k * 12 + l + 1][i] = 0;
                    s->sb_samples[1][k * 12 + l + 2][i] = 0;
                }
                /* next subband in alloc table */
                j += 1 << bit_alloc_bits;
            }
            /* fill remaining samples to zero */
                }
            }
        }
    }
    return 3 * 12;
}

#define SPLIT(dst,sf,n)             \
    if (n == 3) {                   \
        int m = (sf * 171) >> 9;    \
        dst   = sf - 3 * m;         \
        sf    = m;                  \
    } else if (n == 4) {            \
        dst  = sf & 3;              \
        sf >>= 2;                   \
    } else if (n == 5) {            \
        int m = (sf * 205) >> 10;   \
        dst   = sf - 5 * m;         \
        sf    = m;                  \
    } else if (n == 6) {            \
        int m = (sf * 171) >> 10;   \
        dst   = sf - 6 * m;         \
        sf    = m;                  \
    } else {                        \
        dst = 0;                    \
    }

                                           int n3)
{
}

static void exponents_from_scale_factors(MPADecodeContext *s, GranuleDef *g,
                                         int16_t *exponents)
{
    const uint8_t *bstab, *pretab;
    int len, i, j, k, l, v0, shift, gain, gains[3];
    int16_t *exp_ptr;

    exp_ptr = exponents;
    gain    = g->global_gain - 210;
    shift   = g->scalefac_scale + 1;

    bstab  = band_size_long[s->sample_rate_index];
    pretab = mpa_pretab[g->preflag];
    for (i = 0; i < g->long_end; i++) {
        v0 = gain - ((g->scale_factors[i] + pretab[i]) << shift) + 400;
        len = bstab[i];
        for (j = len; j > 0; j--)
            *exp_ptr++ = v0;
    }

    if (g->short_start < 13) {
        bstab    = band_size_short[s->sample_rate_index];
        gains[0] = gain - (g->subblock_gain[0] << 3);
        gains[1] = gain - (g->subblock_gain[1] << 3);
        gains[2] = gain - (g->subblock_gain[2] << 3);
        k        = g->long_end;
        for (i = g->short_start; i < 13; i++) {
            len = bstab[i];
            for (l = 0; l < 3; l++) {
                v0 = gains[l] - (g->scale_factors[k++] << shift) + 400;
                for (j = len; j > 0; j--)
                    *exp_ptr++ = v0;
            }
        }
    }
}

                          int *end_pos2)
{
    }

/* Following is an optimized code for
            INTFLOAT v = *src
            if(get_bits1(&s->gb))
                v = -v;
            *dst = v;
*/
#if USE_FLOATS
#define READ_FLIP_SIGN(dst,src)                     \
    v = AV_RN32A(src) ^ (get_bits1(&s->gb) << 31);  \
    AV_WN32A(dst, v);
#else
#define READ_FLIP_SIGN(dst,src)     \
    v      = -get_bits1(&s->gb);    \
    *(dst) = (*(src) ^ v) - v;
#endif

                          int16_t *exponents, int end_pos2)
{

    /* low frequencies (called big values) */
        /* select vlc table */

        }

        /* read huffcode and compute each couple */

                    break;
            }

            }


                    i, g->region_size[i] - j, y, exponent);
                } else {
                }
                } else {
                }
            } else {
                } else {
                }
            }
        }
    }

    /* high frequencies */
                /* some encoders generate an incorrect size for this
                   part. We must go back into the data */
                s_index -= 4;
                skip_bits_long(&s->gb, last_pos - pos);
                av_log(s->avctx, AV_LOG_INFO, "overread, skip %d enddists: %d %d\n", last_pos - pos, end_pos-pos, end_pos2-pos);
                if(s->err_recognition & (AV_EF_BITSTREAM|AV_EF_COMPLIANT))
                    s_index=0;
            }
                break;
        }

        }
    }
    /* skip extension bits */
        av_log(s->avctx, AV_LOG_ERROR, "bits_left=%d\n", bits_left);
        s_index=0;
        av_log(s->avctx, AV_LOG_ERROR, "bits_left=%d\n", bits_left);
        s_index = 0;
    }


}

/* Reorder short blocks from bitstream order to interleaved order. It
   would be faster to do it in parsing, but the code would be far more
   complicated */
static void reorder_block(MPADecodeContext *s, GranuleDef *g)
{
    int i, j, len;
    INTFLOAT *ptr, *dst, *ptr1;
    INTFLOAT tmp[576];

    if (g->block_type != 2)
        return;

    if (g->switch_point) {
        if (s->sample_rate_index != 8)
            ptr = g->sb_hybrid + 36;
        else
            ptr = g->sb_hybrid + 72;
    } else {
        ptr = g->sb_hybrid;
    }

    for (i = g->short_start; i < 13; i++) {
        len  = band_size_short[s->sample_rate_index][i];
        ptr1 = ptr;
        dst  = tmp;
        for (j = len; j > 0; j--) {
            *dst++ = ptr[0*len];
            *dst++ = ptr[1*len];
            *dst++ = ptr[2*len];
            ptr++;
        }
        ptr += 2 * len;
        memcpy(ptr1, tmp, len * 3 * sizeof(*ptr1));
    }
}

#define ISQRT2 FIXR(0.70710678118654752440)

{

    /* intensity stereo */
            is_tab = is_table;
            sf_max = 7;
        } else {
        }


            /* for last band, use previous scale factor */
            if (i != 11)
                k -= 3;
            len = band_size_short[s->sample_rate_index][i];
            for (l = 2; l >= 0; l--) {
                tab0 -= len;
                tab1 -= len;
                if (!non_zero_found_short[l]) {
                    /* test if non zero band. if so, stop doing i-stereo */
                    for (j = 0; j < len; j++) {
                        if (tab1[j] != 0) {
                            non_zero_found_short[l] = 1;
                            goto found1;
                        }
                    }
                    sf = g1->scale_factors[k + l];
                    if (sf >= sf_max)
                        goto found1;

                    v1 = is_tab[0][sf];
                    v2 = is_tab[1][sf];
                    for (j = 0; j < len; j++) {
                        tmp0    = tab0[j];
                        tab0[j] = MULLx(tmp0, v1, FRAC_BITS);
                        tab1[j] = MULLx(tmp0, v2, FRAC_BITS);
                    }
                } else {
found1:
                    if (s->mode_ext & MODE_EXT_MS_STEREO) {
                        /* lower part of the spectrum : do ms stereo
                           if enabled */
                        for (j = 0; j < len; j++) {
                            tmp0    = tab0[j];
                            tmp1    = tab1[j];
                            tab0[j] = MULLx(tmp0 + tmp1, ISQRT2, FRAC_BITS);
                            tab1[j] = MULLx(tmp0 - tmp1, ISQRT2, FRAC_BITS);
                        }
                    }
                }
            }
        }


            /* test if non zero band. if so, stop doing i-stereo */
                        non_zero_found = 1;
                        goto found2;
                    }
                }
                /* for last band, use previous scale factor */
                    goto found2;
                }
            } else {
found2:
                if (s->mode_ext & MODE_EXT_MS_STEREO) {
                    /* lower part of the spectrum : do ms stereo
                       if enabled */
                    for (j = 0; j < len; j++) {
                        tmp0    = tab0[j];
                        tmp1    = tab1[j];
                        tab0[j] = MULLx(tmp0 + tmp1, ISQRT2, FRAC_BITS);
                        tab1[j] = MULLx(tmp0 - tmp1, ISQRT2, FRAC_BITS);
                    }
                }
            }
        }
        /* ms stereo ONLY */
        /* NOTE: the 1/sqrt(2) normalization factor is included in the
           global gain */
#if USE_FLOATS
#else
        }
#endif
    }

#if USE_FLOATS
#if HAVE_MIPSFPU
#   include "mips/compute_antialias_float.h"
#endif /* HAVE_MIPSFPU */
#else
#if HAVE_MIPSDSP
#   include "mips/compute_antialias_fixed.h"
#endif /* HAVE_MIPSDSP */
#endif /* USE_FLOATS */

#ifndef compute_antialias
#if USE_FLOATS
#define AA(j) do {                                                      \
        float tmp0 = ptr[-1-j];                                         \
        float tmp1 = ptr[   j];                                         \
        ptr[-1-j] = tmp0 * csa_table[j][0] - tmp1 * csa_table[j][1];    \
        ptr[   j] = tmp0 * csa_table[j][1] + tmp1 * csa_table[j][0];    \
    } while (0)
#else
#define AA(j) do {                                              \
        SUINT tmp0 = ptr[-1-j];                                   \
        SUINT tmp1 = ptr[   j];                                   \
        SUINT tmp2 = MULH(tmp0 + tmp1, csa_table[j][0]);          \
        ptr[-1-j] = 4 * (tmp2 - MULH(tmp1, csa_table[j][2]));   \
        ptr[   j] = 4 * (tmp2 + MULH(tmp0, csa_table[j][3]));   \
    } while (0)
#endif

static void compute_antialias(MPADecodeContext *s, GranuleDef *g)
{
    INTFLOAT *ptr;
    int n, i;

    /* we antialias only "long" bands */
    if (g->block_type == 2) {
        if (!g->switch_point)
            return;
        /* XXX: check this for 8000Hz case */
        n = 1;
    } else {
        n = SBLIMIT - 1;
    }

    ptr = g->sb_hybrid + 18;
    for (i = n; i > 0; i--) {
        AA(0);
        AA(1);
        AA(2);
        AA(3);
        AA(4);
        AA(5);
        AA(6);
        AA(7);

        ptr += 18;
    }
}
#endif /* compute_antialias */

static void compute_imdct(MPADecodeContext *s, GranuleDef *g,
                          INTFLOAT *sb_samples, INTFLOAT *mdct_buf)
{
    INTFLOAT *win, *out_ptr, *ptr, *buf, *ptr1;
    INTFLOAT out2[12];
    int i, j, mdct_long_end, sblimit;

    /* find last non zero block */
    ptr  = g->sb_hybrid + 576;
    ptr1 = g->sb_hybrid + 2 * 18;
    while (ptr >= ptr1) {
        int32_t *p;
        ptr -= 6;
        p    = (int32_t*)ptr;
        if (p[0] | p[1] | p[2] | p[3] | p[4] | p[5])
            break;
    }
    sblimit = ((ptr - g->sb_hybrid) / 18) + 1;

    if (g->block_type == 2) {
        /* XXX: check for 8000 Hz */
        if (g->switch_point)
            mdct_long_end = 2;
        else
            mdct_long_end = 0;
    } else {
        mdct_long_end = sblimit;
    }

    s->mpadsp.RENAME(imdct36_blocks)(sb_samples, mdct_buf, g->sb_hybrid,
                                     mdct_long_end, g->switch_point,
                                     g->block_type);

    buf = mdct_buf + 4*18*(mdct_long_end >> 2) + (mdct_long_end & 3);
    ptr = g->sb_hybrid + 18 * mdct_long_end;

    for (j = mdct_long_end; j < sblimit; j++) {
        /* select frequency inversion */
        win     = RENAME(ff_mdct_win)[2 + (4  & -(j & 1))];
        out_ptr = sb_samples + j;

        for (i = 0; i < 6; i++) {
            *out_ptr = buf[4*i];
            out_ptr += SBLIMIT;
        }
        imdct12(out2, ptr + 0);
        for (i = 0; i < 6; i++) {
            *out_ptr     = MULH3(out2[i    ], win[i    ], 1) + buf[4*(i + 6*1)];
            buf[4*(i + 6*2)] = MULH3(out2[i + 6], win[i + 6], 1);
            out_ptr += SBLIMIT;
        }
        imdct12(out2, ptr + 1);
        for (i = 0; i < 6; i++) {
            *out_ptr     = MULH3(out2[i    ], win[i    ], 1) + buf[4*(i + 6*2)];
            buf[4*(i + 6*0)] = MULH3(out2[i + 6], win[i + 6], 1);
            out_ptr += SBLIMIT;
        }
        imdct12(out2, ptr + 2);
        for (i = 0; i < 6; i++) {
            buf[4*(i + 6*0)] = MULH3(out2[i    ], win[i    ], 1) + buf[4*(i + 6*0)];
            buf[4*(i + 6*1)] = MULH3(out2[i + 6], win[i + 6], 1);
            buf[4*(i + 6*2)] = 0;
        }
        ptr += 18;
        buf += (j&3) != 3 ? 1 : (4*18-3);
    }
    /* zero bands */
    for (j = sblimit; j < SBLIMIT; j++) {
        /* overlap */
        out_ptr = sb_samples + j;
        for (i = 0; i < 18; i++) {
            *out_ptr = buf[4*i];
            buf[4*i]   = 0;
            out_ptr += SBLIMIT;
        }
        buf += (j&3) != 3 ? 1 : (4*18-3);
    }
}

/* main layer3 decoding function */
{

    /* read side info */
    } else {
        else
        }
    }
        return ret;

                av_log(s->avctx, AV_LOG_ERROR, "big_values too big\n");
                return AVERROR_INVALIDDATA;
            }

            /* if MS stereo only is selected, we precompute the
               1/sqrt(2) renormalization factor */
                MODE_EXT_MS_STEREO)
            else
                    av_log(s->avctx, AV_LOG_ERROR, "invalid block type\n");
                    return AVERROR_INVALIDDATA;
                }
            } else {
                /* compute huffman coded region sizes */
                        region_address1, region_address2);
            }

                    g->block_type, g->switch_point);
        }
    }

        /* now we get bits from the main_data_begin offset */
                main_data_begin, s->last_buf_size);

            }
        }
        } else {
        }
    } else {
        gr = 0;
        s->extrasize = 0;
    }



                /* MPEG-1 scale factors */
                    } else {
                    }
                    } else {
                    }
                } else {
                            } else {
                            }
                        } else {
                            /* simply copy from last granule */
                            }
                        }
                    }
                }
            } else {

                /* LSF scale factors */
                    tindex = g->switch_point ? 2 : 1;
                else
                    tindex = 0;

                    /* intensity stereo case */
                    } else if (sf < 244) {
                        lsf_sf_expand(slen, sf - 180, 4, 4, 0);
                        tindex2 = 4;
                    } else {
                        lsf_sf_expand(slen, sf - 244, 3, 0, 0);
                        tindex2 = 5;
                    }
                } else {
                    /* normal case */
                    } else {
                        lsf_sf_expand(slen, sf - 500, 3, 0, 0);
                        tindex2 = 2;
                        g->preflag = 1;
                    }
                }

                    } else {
                    }
                }
                /* XXX: should compute exact size */
            }


            /* read Huffman coded residue */
        } /* ch */



        }
    } /* gr */
        skip_bits_long(&s->gb, -get_bits_count(&s->gb));
}

                           const uint8_t *buf, int buf_size)
{


    case 1:
        s->avctx->frame_size = 384;
        nb_frames = mp_decode_layer1(s);
        break;

            } else
                av_log(s->avctx, AV_LOG_ERROR, "invalid old backstep %d\n", i);
        }

                av_log(s->avctx, AV_LOG_ERROR, "invalid new backstep %d\n", i);
        }
    }

        return nb_frames;

    /* get output buffer */
            return ret;
    }

    /* apply the synthesis filter */
        } else {
            samples_ptr   = samples[0] + ch;
            sample_stride = s->nb_channels;
        }
                                        &(s->synth_buf_offset[ch]),
                                        RENAME(ff_mpa_synth_window),
                                        &s->dither_state, samples_ptr,
        }
    }

}

                        AVPacket *avpkt)
{

        buf++;
        buf_size--;
        skipped++;
    }

        return AVERROR_INVALIDDATA;

        av_log(avctx, AV_LOG_DEBUG, "discarding ID3 tag\n");
        return buf_size + skipped;
    }
        /* free format: prepare to compute frame size */
        s->frame_size = -1;
        return AVERROR_INVALIDDATA;
    }
    /* update codec info */

        av_log(avctx, AV_LOG_ERROR, "incomplete frame\n");
        return AVERROR_INVALIDDATA;
        av_log(avctx, AV_LOG_DEBUG, "incorrect frame size - multiple frames in buffer?\n");
        buf_size= s->frame_size;
    }


        //FIXME maybe move the other codec info stuff from above here too
    } else {
        av_log(avctx, AV_LOG_ERROR, "Error while decoding MPEG audio frame.\n");
        /* Only return an error if the bad frame makes up the whole packet or
         * the error is related to buffer management.
         * If there is more data in the packet, just consume the bad frame
         * instead of returning an error, which would discard the whole
         * packet. */
        *got_frame_ptr = 0;
        if (buf_size == avpkt->size || ret != AVERROR_INVALIDDATA)
            return ret;
    }
}

static void mp_flush(MPADecodeContext *ctx)
{
    memset(ctx->synth_buf, 0, sizeof(ctx->synth_buf));
    memset(ctx->mdct_buf, 0, sizeof(ctx->mdct_buf));
    ctx->last_buf_size = 0;
    ctx->dither_state = 0;
}

static void flush(AVCodecContext *avctx)
{
    mp_flush(avctx->priv_data);
}

#if CONFIG_MP3ADU_DECODER || CONFIG_MP3ADUFLOAT_DECODER
static int decode_frame_adu(AVCodecContext *avctx, void *data,
                            int *got_frame_ptr, AVPacket *avpkt)
{
    const uint8_t *buf  = avpkt->data;
    int buf_size        = avpkt->size;
    MPADecodeContext *s = avctx->priv_data;
    uint32_t header;
    int len, ret;
    int av_unused out_size;

    len = buf_size;

    // Discard too short frames
    if (buf_size < HEADER_SIZE) {
        av_log(avctx, AV_LOG_ERROR, "Packet is too small\n");
        return AVERROR_INVALIDDATA;
    }


    if (len > MPA_MAX_CODED_FRAME_SIZE)
        len = MPA_MAX_CODED_FRAME_SIZE;

    // Get header and restore sync word
    header = AV_RB32(buf) | 0xffe00000;

    ret = avpriv_mpegaudio_decode_header((MPADecodeHeader *)s, header);
    if (ret < 0) {
        av_log(avctx, AV_LOG_ERROR, "Invalid frame header\n");
        return ret;
    }
    /* update codec info */
    avctx->sample_rate = s->sample_rate;
    avctx->channels    = s->nb_channels;
    avctx->channel_layout = s->nb_channels == 1 ? AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO;
    if (!avctx->bit_rate)
        avctx->bit_rate = s->bit_rate;

    s->frame_size = len;

    s->frame = data;

    ret = mp_decode_frame(s, NULL, buf, buf_size);
    if (ret < 0) {
        av_log(avctx, AV_LOG_ERROR, "Error while decoding MPEG audio frame.\n");
        return ret;
    }

    *got_frame_ptr = 1;

    return buf_size;
}
#endif /* CONFIG_MP3ADU_DECODER || CONFIG_MP3ADUFLOAT_DECODER */

#if CONFIG_MP3ON4_DECODER || CONFIG_MP3ON4FLOAT_DECODER

/**
 * Context for MP3On4 decoder
 */
typedef struct MP3On4DecodeContext {
    int frames;                     ///< number of mp3 frames per block (number of mp3 decoder instances)
    int syncword;                   ///< syncword patch
    const uint8_t *coff;            ///< channel offsets in output buffer
    MPADecodeContext *mp3decctx[5]; ///< MPADecodeContext for every decoder instance
} MP3On4DecodeContext;

#include "mpeg4audio.h"

/* Next 3 arrays are indexed by channel config number (passed via codecdata) */

/* number of mp3 decoder instances */
static const uint8_t mp3Frames[8] = { 0, 1, 1, 2, 3, 3, 4, 5 };

/* offsets into output buffer, assume output order is FL FR C LFE BL BR SL SR */
static const uint8_t chan_offset[8][5] = {
    { 0             },
    { 0             },  // C
    { 0             },  // FLR
    { 2, 0          },  // C FLR
    { 2, 0, 3       },  // C FLR BS
    { 2, 0, 3       },  // C FLR BLRS
    { 2, 0, 4, 3    },  // C FLR BLRS LFE
    { 2, 0, 6, 4, 3 },  // C FLR BLRS BLR LFE
};

/* mp3on4 channel layouts */
static const int16_t chan_layout[8] = {
    0,
    AV_CH_LAYOUT_MONO,
    AV_CH_LAYOUT_STEREO,
    AV_CH_LAYOUT_SURROUND,
    AV_CH_LAYOUT_4POINT0,
    AV_CH_LAYOUT_5POINT0,
    AV_CH_LAYOUT_5POINT1,
    AV_CH_LAYOUT_7POINT1
};

static av_cold int decode_close_mp3on4(AVCodecContext * avctx)
{
    MP3On4DecodeContext *s = avctx->priv_data;
    int i;

    if (s->mp3decctx[0])
        av_freep(&s->mp3decctx[0]->fdsp);

    for (i = 0; i < s->frames; i++)
        av_freep(&s->mp3decctx[i]);

    return 0;
}


static av_cold int decode_init_mp3on4(AVCodecContext * avctx)
{
    MP3On4DecodeContext *s = avctx->priv_data;
    MPEG4AudioConfig cfg;
    int i;

    if ((avctx->extradata_size < 2) || !avctx->extradata) {
        av_log(avctx, AV_LOG_ERROR, "Codec extradata missing or too short.\n");
        return AVERROR_INVALIDDATA;
    }

    avpriv_mpeg4audio_get_config2(&cfg, avctx->extradata,
                                  avctx->extradata_size, 1, avctx);
    if (!cfg.chan_config || cfg.chan_config > 7) {
        av_log(avctx, AV_LOG_ERROR, "Invalid channel config number.\n");
        return AVERROR_INVALIDDATA;
    }
    s->frames             = mp3Frames[cfg.chan_config];
    s->coff               = chan_offset[cfg.chan_config];
    avctx->channels       = ff_mpeg4audio_channels[cfg.chan_config];
    avctx->channel_layout = chan_layout[cfg.chan_config];

    if (cfg.sample_rate < 16000)
        s->syncword = 0xffe00000;
    else
        s->syncword = 0xfff00000;

    /* Init the first mp3 decoder in standard way, so that all tables get builded
     * We replace avctx->priv_data with the context of the first decoder so that
     * decode_init() does not have to be changed.
     * Other decoders will be initialized here copying data from the first context
     */
    // Allocate zeroed memory for the first decoder context
    s->mp3decctx[0] = av_mallocz(sizeof(MPADecodeContext));
    if (!s->mp3decctx[0])
        goto alloc_fail;
    // Put decoder context in place to make init_decode() happy
    avctx->priv_data = s->mp3decctx[0];
    decode_init(avctx);
    // Restore mp3on4 context pointer
    avctx->priv_data = s;
    s->mp3decctx[0]->adu_mode = 1; // Set adu mode

    /* Create a separate codec/context for each frame (first is already ok).
     * Each frame is 1 or 2 channels - up to 5 frames allowed
     */
    for (i = 1; i < s->frames; i++) {
        s->mp3decctx[i] = av_mallocz(sizeof(MPADecodeContext));
        if (!s->mp3decctx[i])
            goto alloc_fail;
        s->mp3decctx[i]->adu_mode = 1;
        s->mp3decctx[i]->avctx = avctx;
        s->mp3decctx[i]->mpadsp = s->mp3decctx[0]->mpadsp;
        s->mp3decctx[i]->fdsp = s->mp3decctx[0]->fdsp;
    }

    return 0;
alloc_fail:
    decode_close_mp3on4(avctx);
    return AVERROR(ENOMEM);
}


static void flush_mp3on4(AVCodecContext *avctx)
{
    int i;
    MP3On4DecodeContext *s = avctx->priv_data;

    for (i = 0; i < s->frames; i++)
        mp_flush(s->mp3decctx[i]);
}


static int decode_frame_mp3on4(AVCodecContext *avctx, void *data,
                               int *got_frame_ptr, AVPacket *avpkt)
{
    AVFrame *frame         = data;
    const uint8_t *buf     = avpkt->data;
    int buf_size           = avpkt->size;
    MP3On4DecodeContext *s = avctx->priv_data;
    MPADecodeContext *m;
    int fsize, len = buf_size, out_size = 0;
    uint32_t header;
    OUT_INT **out_samples;
    OUT_INT *outptr[2];
    int fr, ch, ret;

    /* get output buffer */
    frame->nb_samples = MPA_FRAME_SIZE;
    if ((ret = ff_get_buffer(avctx, frame, 0)) < 0)
        return ret;
    out_samples = (OUT_INT **)frame->extended_data;

    // Discard too short frames
    if (buf_size < HEADER_SIZE)
        return AVERROR_INVALIDDATA;

    avctx->bit_rate = 0;

    ch = 0;
    for (fr = 0; fr < s->frames; fr++) {
        fsize = AV_RB16(buf) >> 4;
        fsize = FFMIN3(fsize, len, MPA_MAX_CODED_FRAME_SIZE);
        m     = s->mp3decctx[fr];
        av_assert1(m);

        if (fsize < HEADER_SIZE) {
            av_log(avctx, AV_LOG_ERROR, "Frame size smaller than header size\n");
            return AVERROR_INVALIDDATA;
        }
        header = (AV_RB32(buf) & 0x000fffff) | s->syncword; // patch header

        ret = avpriv_mpegaudio_decode_header((MPADecodeHeader *)m, header);
        if (ret < 0) {
            av_log(avctx, AV_LOG_ERROR, "Bad header, discard block\n");
            return AVERROR_INVALIDDATA;
        }

        if (ch + m->nb_channels > avctx->channels ||
            s->coff[fr] + m->nb_channels > avctx->channels) {
            av_log(avctx, AV_LOG_ERROR, "frame channel count exceeds codec "
                                        "channel count\n");
            return AVERROR_INVALIDDATA;
        }
        ch += m->nb_channels;

        outptr[0] = out_samples[s->coff[fr]];
        if (m->nb_channels > 1)
            outptr[1] = out_samples[s->coff[fr] + 1];

        if ((ret = mp_decode_frame(m, outptr, buf, fsize)) < 0) {
            av_log(avctx, AV_LOG_ERROR, "failed to decode channel %d\n", ch);
            memset(outptr[0], 0, MPA_FRAME_SIZE*sizeof(OUT_INT));
            if (m->nb_channels > 1)
                memset(outptr[1], 0, MPA_FRAME_SIZE*sizeof(OUT_INT));
            ret = m->nb_channels * MPA_FRAME_SIZE*sizeof(OUT_INT);
        }

        out_size += ret;
        buf      += fsize;
        len      -= fsize;

        avctx->bit_rate += m->bit_rate;
    }
    if (ch != avctx->channels) {
        av_log(avctx, AV_LOG_ERROR, "failed to decode all channels\n");
        return AVERROR_INVALIDDATA;
    }

    /* update codec info */
    avctx->sample_rate = s->mp3decctx[0]->sample_rate;

    frame->nb_samples = out_size / (avctx->channels * sizeof(OUT_INT));
    *got_frame_ptr    = 1;

    return buf_size;
}
#endif /* CONFIG_MP3ON4_DECODER || CONFIG_MP3ON4FLOAT_DECODER */
