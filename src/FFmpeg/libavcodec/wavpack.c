/*
 * WavPack lossless audio decoder
 * Copyright (c) 2006,2011 Konstantin Shishkov
 * Copyright (c) 2020 David Bryant
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

#include "libavutil/buffer.h"
#include "libavutil/channel_layout.h"

#define BITSTREAM_READER_LE
#include "avcodec.h"
#include "bytestream.h"
#include "get_bits.h"
#include "internal.h"
#include "thread.h"
#include "unary.h"
#include "wavpack.h"
#include "dsd.h"

/**
 * @file
 * WavPack lossless audio decoder
 */

#define DSD_BYTE_READY(low,high) (!(((low) ^ (high)) & 0xff000000))

#define PTABLE_BITS 8
#define PTABLE_BINS (1<<PTABLE_BITS)
#define PTABLE_MASK (PTABLE_BINS-1)

#define UP   0x010000fe
#define DOWN 0x00010000
#define DECAY 8

#define PRECISION 20
#define VALUE_ONE (1 << PRECISION)
#define PRECISION_USE 12

#define RATE_S 20

#define MAX_HISTORY_BITS    5
#define MAX_HISTORY_BINS    (1 << MAX_HISTORY_BITS)
#define MAX_BIN_BYTES       1280    // for value_lookup, per bin (2k - 512 - 256)

typedef enum {
    MODULATION_PCM,     // pulse code modulation
    MODULATION_DSD      // pulse density modulation (aka DSD)
} Modulation;

typedef struct WavpackFrameContext {
    AVCodecContext *avctx;
    int frame_flags;
    int stereo, stereo_in;
    int joint;
    uint32_t CRC;
    GetBitContext gb;
    int got_extra_bits;
    uint32_t crc_extra_bits;
    GetBitContext gb_extra_bits;
    int samples;
    int terms;
    Decorr decorr[MAX_TERMS];
    int zero, one, zeroes;
    int extra_bits;
    int and, or, shift;
    int post_shift;
    int hybrid, hybrid_bitrate;
    int hybrid_maxclip, hybrid_minclip;
    int float_flag;
    int float_shift;
    int float_max_exp;
    WvChannel ch[2];

    GetByteContext gbyte;
    int ptable [PTABLE_BINS];
    uint8_t value_lookup_buffer[MAX_HISTORY_BINS*MAX_BIN_BYTES];
    uint16_t summed_probabilities[MAX_HISTORY_BINS][256];
    uint8_t probabilities[MAX_HISTORY_BINS][256];
    uint8_t *value_lookup[MAX_HISTORY_BINS];
} WavpackFrameContext;

#define WV_MAX_FRAME_DECODERS 14

typedef struct WavpackContext {
    AVCodecContext *avctx;

    WavpackFrameContext *fdec[WV_MAX_FRAME_DECODERS];
    int fdec_num;

    int block;
    int samples;
    int ch_offset;

    AVFrame *frame;
    ThreadFrame curr_frame, prev_frame;
    Modulation modulation;

    AVBufferRef *dsd_ref;
    DSDContext *dsdctx;
    int dsd_channels;
} WavpackContext;

#define LEVEL_DECAY(a)  (((a) + 0x80) >> 8)

{

        return 0;
}

{

            return AVERROR_INVALIDDATA;
    }
        } else {
        }
    }
            else
        } else {
            ctx->ch[i].error_limit = wp_exp2(br[i]);
        }
    }

    return 0;
}

                        int channel, int *last)
{


            }
        } else {
                    goto error;
            } else {
                    goto error;
            }
            }
        }
    }

    } else {
            goto error;
                    goto error;
            } else {
                    goto error;
            }
        }

        } else {
        }
    }

            goto error;
    }

    } else {
    }
            av_log(ctx->avctx, AV_LOG_ERROR, "k %d is too large\n", add);
            goto error;
        }
            goto error;
    } else {
                goto error;
            } else
        }
        ret = mid;
    }

error:
    ret = get_bits_left(gb);
    if (ret <= 0) {
        av_log(ctx->avctx, AV_LOG_ERROR, "Too few bits (%d) left\n", ret);
    }
    *last = 1;
    return 0;
}

                                       unsigned S)
{


        }
    }



}

{
        float    f;
        uint32_t u;
    } value;



            return 0.0;
    }

            if (s->got_extra_bits && get_bits1(&s->gb_extra_bits))
                S = get_bits(&s->gb_extra_bits, 23);
            else
                S = 0;
            exp = 255;
                shift = --exp;

                     get_bits1(&s->gb_extra_bits))) {
                    S |= (1 << shift) - 1;
                }
            }
        } else {
            exp = s->float_max_exp;
        }
    } else {
            } else {
                if (s->float_flag & WV_FLT_ZERO_SIGN)
                    sign = get_bits1(&s->gb_extra_bits);
            }
        }
    }


}

static inline int wv_check_crc(WavpackFrameContext *s, uint32_t crc,
                               uint32_t crc_extra_bits)
{
    if (crc != s->CRC) {
        av_log(s->avctx, AV_LOG_ERROR, "CRC error\n");
        return AVERROR_INVALIDDATA;
    }
    if (s->got_extra_bits && crc_extra_bits != s->crc_extra_bits) {
        av_log(s->avctx, AV_LOG_ERROR, "Extra bits CRC error\n");
        return AVERROR_INVALIDDATA;
    }

    return 0;
}

static void init_ptable(int *table, int rate_i, int rate_s)
{
    int value = 0x808000, rate = rate_i << 8;

    for (int c = (rate + 128) >> 8; c--;)
        value += (DOWN - value) >> DECAY;

    for (int i = 0; i < PTABLE_BINS/2; i++) {
        table[i] = value;
        table[PTABLE_BINS-1-i] = 0x100ffff - value;

        if (value > 0x010000) {
            rate += (rate * rate_s + 128) >> 8;

            for (int c = (rate + 64) >> 7; c--;)
                value += (DOWN - value) >> DECAY;
        }
    }
}

typedef struct {
    int32_t value, fltr0, fltr1, fltr2, fltr3, fltr4, fltr5, fltr6, factor;
    unsigned int byte;
} DSDfilters;

static int wv_unpack_dsd_high(WavpackFrameContext *s, uint8_t *dst_left, uint8_t *dst_right)
{
    uint32_t checksum = 0xFFFFFFFF;
    uint8_t *dst_l = dst_left, *dst_r = dst_right;
    int total_samples = s->samples, stereo = dst_r ? 1 : 0;
    DSDfilters filters[2], *sp = filters;
    int rate_i, rate_s;
    uint32_t low, high, value;

    if (bytestream2_get_bytes_left(&s->gbyte) < (stereo ? 20 : 13))
        return AVERROR_INVALIDDATA;

    rate_i = bytestream2_get_byte(&s->gbyte);
    rate_s = bytestream2_get_byte(&s->gbyte);

    if (rate_s != RATE_S)
        return AVERROR_INVALIDDATA;

    init_ptable(s->ptable, rate_i, rate_s);

    for (int channel = 0; channel < stereo + 1; channel++) {
        DSDfilters *sp = filters + channel;

        sp->fltr1 = bytestream2_get_byte(&s->gbyte) << (PRECISION - 8);
        sp->fltr2 = bytestream2_get_byte(&s->gbyte) << (PRECISION - 8);
        sp->fltr3 = bytestream2_get_byte(&s->gbyte) << (PRECISION - 8);
        sp->fltr4 = bytestream2_get_byte(&s->gbyte) << (PRECISION - 8);
        sp->fltr5 = bytestream2_get_byte(&s->gbyte) << (PRECISION - 8);
        sp->fltr6 = 0;
        sp->factor = bytestream2_get_byte(&s->gbyte) & 0xff;
        sp->factor |= (bytestream2_get_byte(&s->gbyte) << 8) & 0xff00;
        sp->factor = (int32_t)((uint32_t)sp->factor << 16) >> 16;
    }

    value = bytestream2_get_be32(&s->gbyte);
    high = 0xffffffff;
    low = 0x0;

    while (total_samples--) {
        int bitcount = 8;

        sp[0].value = sp[0].fltr1 - sp[0].fltr5 + ((sp[0].fltr6 * sp[0].factor) >> 2);

        if (stereo)
            sp[1].value = sp[1].fltr1 - sp[1].fltr5 + ((sp[1].fltr6 * sp[1].factor) >> 2);

        while (bitcount--) {
            int32_t *pp = s->ptable + ((sp[0].value >> (PRECISION - PRECISION_USE)) & PTABLE_MASK);
            uint32_t split = low + ((high - low) >> 8) * (*pp >> 16);

            if (value <= split) {
                high = split;
                *pp += (UP - *pp) >> DECAY;
                sp[0].fltr0 = -1;
            } else {
                low = split + 1;
                *pp += (DOWN - *pp) >> DECAY;
                sp[0].fltr0 = 0;
            }

            while (DSD_BYTE_READY(high, low) && bytestream2_get_bytes_left(&s->gbyte)) {
                value = (value << 8) | bytestream2_get_byte(&s->gbyte);
                high = (high << 8) | 0xff;
                low <<= 8;
            }

            sp[0].value += sp[0].fltr6 * 8;
            sp[0].byte = (sp[0].byte << 1) | (sp[0].fltr0 & 1);
            sp[0].factor += (((sp[0].value ^ sp[0].fltr0) >> 31) | 1) &
                ((sp[0].value ^ (sp[0].value - (sp[0].fltr6 * 16))) >> 31);
            sp[0].fltr1 += ((sp[0].fltr0 & VALUE_ONE) - sp[0].fltr1) >> 6;
            sp[0].fltr2 += ((sp[0].fltr0 & VALUE_ONE) - sp[0].fltr2) >> 4;
            sp[0].fltr3 += (sp[0].fltr2 - sp[0].fltr3) >> 4;
            sp[0].fltr4 += (sp[0].fltr3 - sp[0].fltr4) >> 4;
            sp[0].value = (sp[0].fltr4 - sp[0].fltr5) >> 4;
            sp[0].fltr5 += sp[0].value;
            sp[0].fltr6 += (sp[0].value - sp[0].fltr6) >> 3;
            sp[0].value = sp[0].fltr1 - sp[0].fltr5 + ((sp[0].fltr6 * sp[0].factor) >> 2);

            if (!stereo)
                continue;

            pp = s->ptable + ((sp[1].value >> (PRECISION - PRECISION_USE)) & PTABLE_MASK);
            split = low + ((high - low) >> 8) * (*pp >> 16);

            if (value <= split) {
                high = split;
                *pp += (UP - *pp) >> DECAY;
                sp[1].fltr0 = -1;
            } else {
                low = split + 1;
                *pp += (DOWN - *pp) >> DECAY;
                sp[1].fltr0 = 0;
            }

            while (DSD_BYTE_READY(high, low) && bytestream2_get_bytes_left(&s->gbyte)) {
                value = (value << 8) | bytestream2_get_byte(&s->gbyte);
                high = (high << 8) | 0xff;
                low <<= 8;
            }

            sp[1].value += sp[1].fltr6 * 8;
            sp[1].byte = (sp[1].byte << 1) | (sp[1].fltr0 & 1);
            sp[1].factor += (((sp[1].value ^ sp[1].fltr0) >> 31) | 1) &
                ((sp[1].value ^ (sp[1].value - (sp[1].fltr6 * 16))) >> 31);
            sp[1].fltr1 += ((sp[1].fltr0 & VALUE_ONE) - sp[1].fltr1) >> 6;
            sp[1].fltr2 += ((sp[1].fltr0 & VALUE_ONE) - sp[1].fltr2) >> 4;
            sp[1].fltr3 += (sp[1].fltr2 - sp[1].fltr3) >> 4;
            sp[1].fltr4 += (sp[1].fltr3 - sp[1].fltr4) >> 4;
            sp[1].value = (sp[1].fltr4 - sp[1].fltr5) >> 4;
            sp[1].fltr5 += sp[1].value;
            sp[1].fltr6 += (sp[1].value - sp[1].fltr6) >> 3;
            sp[1].value = sp[1].fltr1 - sp[1].fltr5 + ((sp[1].fltr6 * sp[1].factor) >> 2);
        }

        checksum += (checksum << 1) + (*dst_l = sp[0].byte & 0xff);
        sp[0].factor -= (sp[0].factor + 512) >> 10;
        dst_l += 4;

        if (stereo) {
            checksum += (checksum << 1) + (*dst_r = filters[1].byte & 0xff);
            filters[1].factor -= (filters[1].factor + 512) >> 10;
            dst_r += 4;
        }
    }

    if (wv_check_crc(s, checksum, 0)) {
        if (s->avctx->err_recognition & AV_EF_CRCCHECK)
            return AVERROR_INVALIDDATA;

        memset(dst_left, 0x69, s->samples * 4);

        if (dst_r)
            memset(dst_right, 0x69, s->samples * 4);
    }

    return 0;
}

static int wv_unpack_dsd_fast(WavpackFrameContext *s, uint8_t *dst_left, uint8_t *dst_right)
{
    uint8_t *dst_l = dst_left, *dst_r = dst_right;
    uint8_t history_bits, max_probability;
    int total_summed_probabilities  = 0;
    int total_samples               = s->samples;
    uint8_t *vlb                    = s->value_lookup_buffer;
    int history_bins, p0, p1, chan;
    uint32_t checksum               = 0xFFFFFFFF;
    uint32_t low, high, value;

    if (!bytestream2_get_bytes_left(&s->gbyte))
        return AVERROR_INVALIDDATA;

    history_bits = bytestream2_get_byte(&s->gbyte);

    if (!bytestream2_get_bytes_left(&s->gbyte) || history_bits > MAX_HISTORY_BITS)
        return AVERROR_INVALIDDATA;

    history_bins = 1 << history_bits;
    max_probability = bytestream2_get_byte(&s->gbyte);

    if (max_probability < 0xff) {
        uint8_t *outptr = (uint8_t *)s->probabilities;
        uint8_t *outend = outptr + sizeof(*s->probabilities) * history_bins;

        while (outptr < outend && bytestream2_get_bytes_left(&s->gbyte)) {
            int code = bytestream2_get_byte(&s->gbyte);

            if (code > max_probability) {
                int zcount = code - max_probability;

                while (outptr < outend && zcount--)
                    *outptr++ = 0;
            } else if (code) {
                *outptr++ = code;
            }
            else {
                break;
            }
        }

        if (outptr < outend ||
            (bytestream2_get_bytes_left(&s->gbyte) && bytestream2_get_byte(&s->gbyte)))
                return AVERROR_INVALIDDATA;
    } else if (bytestream2_get_bytes_left(&s->gbyte) > (int)sizeof(*s->probabilities) * history_bins) {
        bytestream2_get_buffer(&s->gbyte, (uint8_t *)s->probabilities,
            sizeof(*s->probabilities) * history_bins);
    } else {
        return AVERROR_INVALIDDATA;
    }

    for (p0 = 0; p0 < history_bins; p0++) {
        int32_t sum_values = 0;

        for (int i = 0; i < 256; i++)
            s->summed_probabilities[p0][i] = sum_values += s->probabilities[p0][i];

        if (sum_values) {
            total_summed_probabilities += sum_values;

            if (total_summed_probabilities > history_bins * MAX_BIN_BYTES)
                return AVERROR_INVALIDDATA;

            s->value_lookup[p0] = vlb;

            for (int i = 0; i < 256; i++) {
                int c = s->probabilities[p0][i];

                while (c--)
                    *vlb++ = i;
            }
        }
    }

    if (bytestream2_get_bytes_left(&s->gbyte) < 4)
        return AVERROR_INVALIDDATA;

    chan = p0 = p1 = 0;
    low = 0; high = 0xffffffff;
    value = bytestream2_get_be32(&s->gbyte);

    if (dst_r)
        total_samples *= 2;

    while (total_samples--) {
        unsigned int mult, index, code;

        if (!s->summed_probabilities[p0][255])
            return AVERROR_INVALIDDATA;

        mult = (high - low) / s->summed_probabilities[p0][255];

        if (!mult) {
            if (bytestream2_get_bytes_left(&s->gbyte) >= 4)
                value = bytestream2_get_be32(&s->gbyte);

            low = 0;
            high = 0xffffffff;
            mult = high / s->summed_probabilities[p0][255];

            if (!mult)
                return AVERROR_INVALIDDATA;
        }

        index = (value - low) / mult;

        if (index >= s->summed_probabilities[p0][255])
            return AVERROR_INVALIDDATA;

        if (!dst_r) {
            if ((*dst_l = code = s->value_lookup[p0][index]))
                low += s->summed_probabilities[p0][code-1] * mult;

            dst_l += 4;
        } else {
            if ((code = s->value_lookup[p0][index]))
                low += s->summed_probabilities[p0][code-1] * mult;

            if (chan) {
                *dst_r = code;
                dst_r += 4;
            }
            else {
                *dst_l = code;
                dst_l += 4;
            }

            chan ^= 1;
        }

        high = low + s->probabilities[p0][code] * mult - 1;
        checksum += (checksum << 1) + code;

        if (!dst_r) {
            p0 = code & (history_bins-1);
        } else {
            p0 = p1;
            p1 = code & (history_bins-1);
        }

        while (DSD_BYTE_READY(high, low) && bytestream2_get_bytes_left(&s->gbyte)) {
            value = (value << 8) | bytestream2_get_byte(&s->gbyte);
            high = (high << 8) | 0xff;
            low <<= 8;
        }
    }

    if (wv_check_crc(s, checksum, 0)) {
        if (s->avctx->err_recognition & AV_EF_CRCCHECK)
            return AVERROR_INVALIDDATA;

        memset(dst_left, 0x69, s->samples * 4);

        if (dst_r)
            memset(dst_right, 0x69, s->samples * 4);
    }

    return 0;
}

static int wv_unpack_dsd_copy(WavpackFrameContext *s, uint8_t *dst_left, uint8_t *dst_right)
{
    uint8_t *dst_l = dst_left, *dst_r = dst_right;
    int total_samples           = s->samples;
    uint32_t checksum           = 0xFFFFFFFF;

    if (bytestream2_get_bytes_left(&s->gbyte) != total_samples * (dst_r ? 2 : 1))
        return AVERROR_INVALIDDATA;

    while (total_samples--) {
        checksum += (checksum << 1) + (*dst_l = bytestream2_get_byte(&s->gbyte));
        dst_l += 4;

        if (dst_r) {
            checksum += (checksum << 1) + (*dst_r = bytestream2_get_byte(&s->gbyte));
            dst_r += 4;
        }
    }

    if (wv_check_crc(s, checksum, 0)) {
        if (s->avctx->err_recognition & AV_EF_CRCCHECK)
            return AVERROR_INVALIDDATA;

        memset(dst_left, 0x69, s->samples * 4);

        if (dst_r)
            memset(dst_right, 0x69, s->samples * 4);
    }

    return 0;
}

                                   void *dst_l, void *dst_r, const int type)
{

            break;
            break;
                    } else {
                    }
                } else {
                }
                } else {
                }
                    L2 = L + ((s->decorr[i].weightA * (int64_t)s->decorr[i].samplesA[0] + 512) >> 10);
                else
                    R2 = R + ((s->decorr[i].weightB * (int64_t)L2 + 512) >> 10);
                else
            } else {
                else

                }

                else
            }
        }

            }
        }


        } else {
        }

        int size = av_get_bytes_per_sample(type);
        memset((uint8_t*)dst_l + count*size, 0, (s->samples-count)*size);
        memset((uint8_t*)dst_r + count*size, 0, (s->samples-count)*size);
    }

        wv_check_crc(s, crc, crc_extra_bits))
        return AVERROR_INVALIDDATA;

    return 0;
}

                                 void *dst, const int type)
{

            break;
                else
            } else {
            }
            else
        }

        } else {
        }

        int size = av_get_bytes_per_sample(type);
        memset((uint8_t*)dst + count*size, 0, (s->samples-count)*size);
    }

        int ret = wv_check_crc(s, crc, crc_extra_bits);
        if (ret < 0 && s->avctx->err_recognition & AV_EF_EXPLODE)
            return ret;
    }

    return 0;
}

{
        return -1;

        return -1;

}

{


        return 0;

    if (channels > INT_MAX / sizeof(*s->dsdctx))
        return AVERROR(EINVAL);

    s->dsd_ref = av_buffer_allocz(channels * sizeof(*s->dsdctx));
    if (!s->dsd_ref)
        return AVERROR(ENOMEM);
    s->dsdctx = (DSDContext*)s->dsd_ref->data;
    s->dsd_channels = channels;

    for (i = 0; i < channels; i++)
        memset(s->dsdctx[i].buf, 0x69, sizeof(s->dsdctx[i].buf));

    return 0;
}

#if HAVE_THREADS
static int update_thread_context(AVCodecContext *dst, const AVCodecContext *src)
{
    WavpackContext *fsrc = src->priv_data;
    WavpackContext *fdst = dst->priv_data;
    int ret;

    if (dst == src)
        return 0;

    ff_thread_release_buffer(dst, &fdst->curr_frame);
    if (fsrc->curr_frame.f->data[0]) {
        if ((ret = ff_thread_ref_frame(&fdst->curr_frame, &fsrc->curr_frame)) < 0)
            return ret;
    }

    av_buffer_unref(&fdst->dsd_ref);
    fdst->dsdctx = NULL;
    fdst->dsd_channels = 0;
    if (fsrc->dsd_ref) {
        fdst->dsd_ref = av_buffer_ref(fsrc->dsd_ref);
        if (!fdst->dsd_ref)
            return AVERROR(ENOMEM);
        fdst->dsdctx = (DSDContext*)fdst->dsd_ref->data;
        fdst->dsd_channels = fsrc->dsd_channels;
    }

    return 0;
}
#endif

{




        return AVERROR(ENOMEM);


}

{





}

                                const uint8_t *buf, int buf_size)
{

        av_log(avctx, AV_LOG_ERROR, "Error creating frame decode context\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "Context for block %d is not present\n",
               block_no);
        return AVERROR_INVALIDDATA;
    }



        av_log(avctx, AV_LOG_ERROR, "Mismatching number of samples in "
               "a sequence: %d and %d\n", wc->samples, s->samples);
        return AVERROR_INVALIDDATA;
    }

        sample_fmt = AV_SAMPLE_FMT_FLTP;
        sample_fmt = AV_SAMPLE_FMT_S16P;
    else

        return AVERROR_INVALIDDATA;


        return AVERROR_INVALIDDATA;
    }

    // parse metadata blocks
            av_log(avctx, AV_LOG_ERROR,
                   "Got incorrect block %02X with size %i\n", id, size);
            break;
        }
            av_log(avctx, AV_LOG_ERROR,
                   "Block size %i is out of bounds\n", size);
            break;
        }
                av_log(avctx, AV_LOG_ERROR, "Too many decorrelation terms\n");
                s->terms = 0;
                bytestream2_skip(&gb, ssize);
                continue;
            }
            }
            got_terms = 1;
            break;
                av_log(avctx, AV_LOG_ERROR, "No decorrelation terms met\n");
                continue;
            }
                av_log(avctx, AV_LOG_ERROR, "Too many decorrelation weights\n");
                bytestream2_skip(&gb, ssize);
                continue;
            }
                }
            }
            got_weights = 1;
            break;
                av_log(avctx, AV_LOG_ERROR, "No decorrelation terms met\n");
                continue;
            }

                    }
                    s->decorr[i].samplesA[0] =
                        wp_exp2(bytestream2_get_le16(&gb));
                    s->decorr[i].samplesB[0] =
                        wp_exp2(bytestream2_get_le16(&gb));
                    t                       += 4;
                } else {
                        }
                    }
                }
            }
            got_samples = 1;
            break;
                av_log(avctx, AV_LOG_ERROR,
                       "Entropy vars size should be %i, got %i.\n",
                       6 * (s->stereo_in + 1), size);
                bytestream2_skip(&gb, ssize);
                continue;
            }
                }
            got_entropy = 1;
            break;
                }
            }
            }
                for (i = 0; i < (s->stereo_in + 1); i++) {
                    s->ch[i].bitrate_delta =
                        wp_exp2((int16_t)bytestream2_get_le16(&gb));
                }
            } else {
            }
            got_hybrid = 1;
            break;
                av_log(avctx, AV_LOG_ERROR,
                       "Invalid INT32INFO, size = %i\n",
                       size);
                bytestream2_skip(&gb, ssize - 4);
                continue;
            }
                av_log(avctx, AV_LOG_ERROR,
                       "Invalid INT32INFO, extra_bits = %d (> 30)\n", val[0]);
                continue;
            } else if (val[2]) {
                s->and   = s->or = 1;
                s->shift = val[2];
            } else if (val[3]) {
                s->and   = 1;
                s->shift = val[3];
            }
                av_log(avctx, AV_LOG_ERROR,
                       "Invalid INT32INFO, shift = %d (> 31)\n", s->shift);
                s->and = s->or = s->shift = 0;
                continue;
            }
            /* original WavPack decoder forces 32-bit lossy sound to be treated
             * as 24-bit one in order to have proper clipping */
                s->post_shift      += 8;
                s->shift           -= 8;
                s->hybrid_maxclip >>= 8;
                s->hybrid_minclip >>= 8;
            }
        }
                av_log(avctx, AV_LOG_ERROR,
                       "Invalid FLOATINFO, size = %i\n", size);
                bytestream2_skip(&gb, ssize);
                continue;
            }
                av_log(avctx, AV_LOG_ERROR,
                       "Invalid FLOATINFO, shift = %d (> 31)\n", s->float_shift);
                s->float_shift = 0;
                continue;
            }
            break;
                return ret;
        case WP_ID_DSD_DATA:
            if (size < 2) {
                av_log(avctx, AV_LOG_ERROR, "Invalid DSD_DATA, size = %i\n",
                       size);
                bytestream2_skip(&gb, ssize);
                continue;
            }
            rate_x = bytestream2_get_byte(&gb);
            if (rate_x > 30)
                return AVERROR_INVALIDDATA;
            rate_x = 1 << rate_x;
            dsd_mode = bytestream2_get_byte(&gb);
            if (dsd_mode && dsd_mode != 1 && dsd_mode != 3) {
                av_log(avctx, AV_LOG_ERROR, "Invalid DSD encoding mode: %d\n",
                    dsd_mode);
                return AVERROR_INVALIDDATA;
            }
            bytestream2_init(&s->gbyte, gb.buffer, size-2);
            bytestream2_skip(&gb, size-2);
            got_dsd      = 1;
            break;
                av_log(avctx, AV_LOG_ERROR, "Invalid EXTRABITS, size = %i\n",
                       size);
                bytestream2_skip(&gb, size);
                continue;
            }
                return ret;
                av_log(avctx, AV_LOG_ERROR,
                       "Insufficient channel information\n");
                return AVERROR_INVALIDDATA;
            }
            case 0:
            case 1:
            case 2:
                chmask = bytestream2_get_le24(&gb);
                break;
            case 3:
                chmask = bytestream2_get_le32(&gb);
                break;
            case 4:
                size = bytestream2_get_byte(&gb);
                chan  |= (bytestream2_get_byte(&gb) & 0xF) << 8;
                chan  += 1;
                if (avctx->channels != chan)
                    av_log(avctx, AV_LOG_WARNING, "%i channels signalled"
                           " instead of %i.\n", chan, avctx->channels);
                chmask = bytestream2_get_le24(&gb);
                break;
            case 5:
                size = bytestream2_get_byte(&gb);
                chan  |= (bytestream2_get_byte(&gb) & 0xF) << 8;
                chan  += 1;
                if (avctx->channels != chan)
                    av_log(avctx, AV_LOG_WARNING, "%i channels signalled"
                           " instead of %i.\n", chan, avctx->channels);
                chmask = bytestream2_get_le32(&gb);
                break;
            default:
                av_log(avctx, AV_LOG_ERROR, "Invalid channel info size %d\n",
                       size);
                chan   = avctx->channels;
                chmask = avctx->channel_layout;
            }
            break;
        case WP_ID_SAMPLE_RATE:
            if (size != 3) {
                av_log(avctx, AV_LOG_ERROR, "Invalid custom sample rate.\n");
                return AVERROR_INVALIDDATA;
            }
            sample_rate = bytestream2_get_le24(&gb);
            break;
        }
    }

            av_log(avctx, AV_LOG_ERROR, "No block with decorrelation terms\n");
            return AVERROR_INVALIDDATA;
        }
            av_log(avctx, AV_LOG_ERROR, "No block with decorrelation weights\n");
            return AVERROR_INVALIDDATA;
        }
            av_log(avctx, AV_LOG_ERROR, "No block with decorrelation samples\n");
            return AVERROR_INVALIDDATA;
        }
            av_log(avctx, AV_LOG_ERROR, "No block with entropy info\n");
            return AVERROR_INVALIDDATA;
        }
            av_log(avctx, AV_LOG_ERROR, "Hybrid config not found\n");
            return AVERROR_INVALIDDATA;
        }
            av_log(avctx, AV_LOG_ERROR, "Float information not found\n");
            return AVERROR_INVALIDDATA;
        }
                av_log(avctx, AV_LOG_ERROR, "Too small EXTRABITS\n");
                s->got_extra_bits = 0;
            }
        }
    }

        av_log(avctx, AV_LOG_ERROR, "Packed samples not found\n");
        return AVERROR_INVALIDDATA;
    }

        (got_dsd && wc->modulation != MODULATION_DSD)) {
            av_log(avctx, AV_LOG_ERROR, "Invalid PCM/DSD mix encountered\n");
            return AVERROR_INVALIDDATA;
    }

            if (!sample_rate) {
                av_log(avctx, AV_LOG_ERROR, "Custom sample rate missing.\n");
                return AVERROR_INVALIDDATA;
            }
            new_samplerate = sample_rate;
        } else

            return AVERROR_INVALIDDATA;

        } else {
                                       AV_CH_LAYOUT_MONO;
        }

            av_log(avctx, AV_LOG_ERROR, "Channel mask does not match the channel count\n");
            return AVERROR_INVALIDDATA;
        }

        /* clear DSD state if stream properties change */
            new_chmask     != avctx->channel_layout ||
            new_samplerate != avctx->sample_rate    ||
            !!got_dsd      != !!wc->dsdctx) {
                av_log(avctx, AV_LOG_ERROR, "Error reinitializing the DSD context\n");
                return ret;
            }
        }


        /* get output buffer */
            return ret;

    }

        av_log(avctx, AV_LOG_WARNING, "Too many channels coded in a packet.\n");
        return ((avctx->err_recognition & AV_EF_EXPLODE) || !wc->ch_offset) ? AVERROR_INVALIDDATA : 0;
    }



            if (dsd_mode == 3) {
                ret = wv_unpack_dsd_high(s, samples_l, samples_r);
            } else if (dsd_mode == 1) {
                ret = wv_unpack_dsd_fast(s, samples_l, samples_r);
            } else {
                ret = wv_unpack_dsd_copy(s, samples_l, samples_r);
            }
        } else {
        }
            return ret;
    } else {
            if (dsd_mode == 3) {
                ret = wv_unpack_dsd_high(s, samples_l, NULL);
            } else if (dsd_mode == 1) {
                ret = wv_unpack_dsd_fast(s, samples_l, NULL);
            } else {
                ret = wv_unpack_dsd_copy(s, samples_l, NULL);
            }
        } else {
        }
            return ret;

    }

    return 0;
}

static void wavpack_decode_flush(AVCodecContext *avctx)
{
    WavpackContext *s = avctx->priv_data;

    wv_dsd_reset(s, 0);
}

static int dsd_channel(AVCodecContext *avctx, void *frmptr, int jobnr, int threadnr)
{
    WavpackContext *s  = avctx->priv_data;
    AVFrame *frame = frmptr;

    ff_dsd2pcm_translate (&s->dsdctx [jobnr], s->samples, 0,
        (uint8_t *)frame->extended_data[jobnr], 4,
        (float *)frame->extended_data[jobnr], 1);

    return 0;
}

                                int *got_frame_ptr, AVPacket *avpkt)
{

        return AVERROR_INVALIDDATA;


    /* determine number of samples */
        av_log(avctx, AV_LOG_ERROR, "Invalid number of samples: %d\n",
               s->samples);
        return AVERROR_INVALIDDATA;
    }


            av_log(avctx, AV_LOG_ERROR,
                   "Block %d has invalid size (size %d vs. %d bytes left)\n",
                   s->block, frame_size, buf_size);
            ret = AVERROR_INVALIDDATA;
            goto error;
        }
    }

        av_log(avctx, AV_LOG_ERROR, "Not enough channels coded in a packet.\n");
        ret = AVERROR_INVALIDDATA;
        goto error;
    }


        avctx->execute2(avctx, dsd_channel, s->frame, NULL, avctx->channels);


        return ret;



    }

    return ret;
}

AVCodec ff_wavpack_decoder = {
    .name           = "wavpack",
    .long_name      = NULL_IF_CONFIG_SMALL("WavPack"),
    .type           = AVMEDIA_TYPE_AUDIO,
    .id             = AV_CODEC_ID_WAVPACK,
    .priv_data_size = sizeof(WavpackContext),
    .init           = wavpack_decode_init,
    .close          = wavpack_decode_end,
    .decode         = wavpack_decode_frame,
    .flush          = wavpack_decode_flush,
    .update_thread_context = ONLY_IF_THREADS_ENABLED(update_thread_context),
    .capabilities   = AV_CODEC_CAP_DR1 | AV_CODEC_CAP_FRAME_THREADS |
                      AV_CODEC_CAP_SLICE_THREADS,
    .caps_internal  = FF_CODEC_CAP_ALLOCATE_PROGRESS,
};
