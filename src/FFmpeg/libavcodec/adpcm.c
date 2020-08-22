/*
 * Copyright (c) 2001-2003 The FFmpeg project
 *
 * first version by Francois Revol (revol@free.fr)
 * fringe ADPCM codecs (e.g., DK3, DK4, Westwood)
 *   by Mike Melanson (melanson@pcisys.net)
 * CD-ROM XA ADPCM codec by BERO
 * EA ADPCM decoder by Robin Kay (komadori@myrealbox.com)
 * EA ADPCM R1/R2/R3 decoder by Peter Ross (pross@xvid.org)
 * EA IMA EACS decoder by Peter Ross (pross@xvid.org)
 * EA IMA SEAD decoder by Peter Ross (pross@xvid.org)
 * EA ADPCM XAS decoder by Peter Ross (pross@xvid.org)
 * MAXIS EA ADPCM decoder by Robert Marston (rmarston@gmail.com)
 * THP ADPCM decoder by Marco Gerards (mgerards@xs4all.nl)
 * Argonaut Games ADPCM decoder by Zane van Iperen (zane@zanevaniperen.com)
 * Simon & Schuster Interactive ADPCM decoder by Zane van Iperen (zane@zanevaniperen.com)
 * Ubisoft ADPCM decoder by Zane van Iperen (zane@zanevaniperen.com)
 * High Voltage Software ALP decoder by Zane van Iperen (zane@zanevaniperen.com)
 * Cunning Developments decoder by Zane van Iperen (zane@zanevaniperen.com)
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
#include "avcodec.h"
#include "get_bits.h"
#include "bytestream.h"
#include "adpcm.h"
#include "adpcm_data.h"
#include "internal.h"

/**
 * @file
 * ADPCM decoders
 * Features and limitations:
 *
 * Reference documents:
 * http://wiki.multimedia.cx/index.php?title=Category:ADPCM_Audio_Codecs
 * http://www.pcisys.net/~melanson/codecs/simpleaudio.html [dead]
 * http://www.geocities.com/SiliconValley/8682/aud3.txt [dead]
 * http://openquicktime.sourceforge.net/
 * XAnim sources (xa_codec.c) http://xanim.polter.net/
 * http://www.cs.ucla.edu/~leec/mediabench/applications.html [dead]
 * SoX source code http://sox.sourceforge.net/
 *
 * CD-ROM XA:
 * http://ku-www.ss.titech.ac.jp/~yatsushi/xaadpcm.html [dead]
 * vagpack & depack http://homepages.compuserve.de/bITmASTER32/psx-index.html [dead]
 * readstr http://www.geocities.co.jp/Playtown/2004/
 */

/* These are for CD-ROM XA ADPCM */
static const int8_t xa_adpcm_table[5][2] = {
    {   0,   0 },
    {  60,   0 },
    { 115, -52 },
    {  98, -55 },
    { 122, -60 }
};

static const int16_t ea_adpcm_table[] = {
    0,  240,  460,  392,
    0,    0, -208, -220,
    0,    1,    3,    4,
    7,    8,   10,   11,
    0,   -1,   -3,   -4
};

// padded to zero where table size is less then 16
static const int8_t swf_index_tables[4][16] = {
    /*2*/ { -1, 2 },
    /*3*/ { -1, -1, 2, 4 },
    /*4*/ { -1, -1, -1, -1, 2, 4, 6, 8 },
    /*5*/ { -1, -1, -1, -1, -1, -1, -1, -1, 1, 2, 4, 6, 8, 10, 13, 16 }
};

static const int8_t zork_index_table[8] = {
    -1, -1, -1, 1, 4, 7, 10, 12,
};

static const int8_t mtf_index_table[16] = {
     8,  6,  4,  2, -1, -1, -1, -1,
    -1, -1, -1, -1,  2,  4,  6,  8,
};

/* end of tables */

typedef struct ADPCMDecodeContext {
    ADPCMChannelStatus status[14];
    int vqa_version;                /**< VQA version. Used for ADPCM_IMA_WS */
    int has_status;
} ADPCMDecodeContext;

{

    case AV_CODEC_ID_ADPCM_EA:
    case AV_CODEC_ID_ADPCM_EA_R1:
    case AV_CODEC_ID_ADPCM_EA_R2:
    case AV_CODEC_ID_ADPCM_EA_R3:
    case AV_CODEC_ID_ADPCM_EA_XAS:
    case AV_CODEC_ID_ADPCM_MS:
    case AV_CODEC_ID_ADPCM_MTAF:
        min_channels = 2;
        max_channels = 8;
        if (avctx->channels & 1) {
            avpriv_request_sample(avctx, "channel count %d\n", avctx->channels);
            return AVERROR_PATCHWELCOME;
        }
        break;
    case AV_CODEC_ID_ADPCM_PSX:
        max_channels = 8;
        break;
    case AV_CODEC_ID_ADPCM_THP:
    case AV_CODEC_ID_ADPCM_THP_LE:
    }
        av_log(avctx, AV_LOG_ERROR, "Invalid number of channels\n");
        return AVERROR(EINVAL);
    }

            return AVERROR_INVALIDDATA;
        break;
        }
        break;
            } else if (avctx->extradata_size >= 16) {
                c->status[0].predictor  = av_clip_intp2(AV_RL32(avctx->extradata +  0), 18);
                c->status[0].step_index = av_clip(AV_RL32(avctx->extradata +  4), 0, 88);
                c->status[1].predictor  = av_clip_intp2(AV_RL32(avctx->extradata +  8), 18);
                c->status[1].step_index = av_clip(AV_RL32(avctx->extradata + 12), 0, 88);
            }
        }
        break;
        break;
            return AVERROR_INVALIDDATA;
        break;
    case AV_CODEC_ID_ADPCM_ZORK:
        if (avctx->bits_per_coded_sample != 8)
            return AVERROR_INVALIDDATA;
        break;
    default:
        break;
    }

    case AV_CODEC_ID_ADPCM_IMA_DAT4:
    case AV_CODEC_ID_ADPCM_IMA_QT:
    case AV_CODEC_ID_ADPCM_IMA_WAV:
    case AV_CODEC_ID_ADPCM_4XM:
    case AV_CODEC_ID_ADPCM_XA:
    case AV_CODEC_ID_ADPCM_EA_R1:
    case AV_CODEC_ID_ADPCM_EA_R2:
    case AV_CODEC_ID_ADPCM_EA_R3:
    case AV_CODEC_ID_ADPCM_EA_XAS:
    case AV_CODEC_ID_ADPCM_THP:
    case AV_CODEC_ID_ADPCM_THP_LE:
    case AV_CODEC_ID_ADPCM_AFC:
    case AV_CODEC_ID_ADPCM_DTK:
    case AV_CODEC_ID_ADPCM_PSX:
    case AV_CODEC_ID_ADPCM_MTAF:
    case AV_CODEC_ID_ADPCM_ARGO:
                                                  AV_SAMPLE_FMT_S16;
                                                  AV_SAMPLE_FMT_S16;
    }

    return 0;
}

static inline int16_t adpcm_agm_expand_nibble(ADPCMChannelStatus *c, int8_t nibble)
{
    int delta, pred, step, add;

    pred = c->predictor;
    delta = nibble & 7;
    step = c->step;
    add = (delta * 2 + 1) * step;
    if (add < 0)
        add = add + 7;

    if ((nibble & 8) == 0)
        pred = av_clip(pred + (add >> 3), -32767, 32767);
    else
        pred = av_clip(pred - (add >> 3), -32767, 32767);

    switch (delta) {
    case 7:
        step *= 0x99;
        break;
    case 6:
        c->step = av_clip(c->step * 2, 127, 24576);
        c->predictor = pred;
        return pred;
    case 5:
        step *= 0x66;
        break;
    case 4:
        step *= 0x4d;
        break;
    default:
        step *= 0x39;
        break;
    }

    if (step < 0)
        step += 0x3f;

    c->step = step >> 6;
    c->step = av_clip(c->step, 127, 24576);
    c->predictor = pred;
    return pred;
}

{


    /* perform direct multiplication instead of series of jumps proposed by
     * the reference ADPCM implementation since modern CPUs can do the mults
     * quickly enough */


}

{




}

static inline int16_t adpcm_ima_mtf_expand_nibble(ADPCMChannelStatus *c, int nibble)
{
    int step_index, step, delta, predictor;

    step = ff_adpcm_step_table[c->step_index];

    delta = step * (2 * nibble - 15);
    predictor = c->predictor + delta;

    step_index = c->step_index + mtf_index_table[(unsigned)nibble];
    c->predictor = av_clip_int16(predictor >> 4);
    c->step_index = av_clip(step_index, 0, 88);

    return (int16_t)c->predictor;
}

{





}

static inline int16_t adpcm_ima_wav_expand_nibble(ADPCMChannelStatus *c, GetBitContext *gb, int bps)
{
    int nibble, step_index, predictor, sign, delta, diff, step, shift;

    shift = bps - 1;
    nibble = get_bits_le(gb, bps),
    step = ff_adpcm_step_table[c->step_index];
    step_index = c->step_index + ff_adpcm_index_tables[bps - 2][nibble];
    step_index = av_clip(step_index, 0, 88);

    sign = nibble & (1 << shift);
    delta = av_mod_uintp2(nibble, shift);
    diff = ((2 * delta + 1) * step) >> shift;
    predictor = c->predictor;
    if (sign) predictor -= diff;
    else predictor += diff;

    c->predictor = av_clip_int16(predictor);
    c->step_index = step_index;

    return (int16_t)c->predictor;
}

static inline int adpcm_ima_qt_expand_nibble(ADPCMChannelStatus *c, int nibble)
{
    int step_index;
    int predictor;
    int diff, step;

    step = ff_adpcm_step_table[c->step_index];
    step_index = c->step_index + ff_adpcm_index_table[nibble];
    step_index = av_clip(step_index, 0, 88);

    diff = step >> 3;
    if (nibble & 4) diff += step;
    if (nibble & 2) diff += step >> 1;
    if (nibble & 1) diff += step >> 2;

    if (nibble & 8)
        predictor = c->predictor - diff;
    else
        predictor = c->predictor + diff;

    c->predictor = av_clip_int16(predictor);
    c->step_index = step_index;

    return c->predictor;
}

{


        av_log(NULL, AV_LOG_WARNING, "idelta overflow\n");
        c->idelta = INT_MAX/768;
    }

}

static inline int16_t adpcm_ima_oki_expand_nibble(ADPCMChannelStatus *c, int nibble)
{
    int step_index, predictor, sign, delta, diff, step;

    step = ff_adpcm_oki_step_table[c->step_index];
    step_index = c->step_index + ff_adpcm_index_table[(unsigned)nibble];
    step_index = av_clip(step_index, 0, 48);

    sign = nibble & 8;
    delta = nibble & 7;
    diff = ((2 * delta + 1) * step) >> 3;
    predictor = c->predictor;
    if (sign) predictor -= diff;
    else predictor += diff;

    c->predictor = av_clip_intp2(predictor, 11);
    c->step_index = step_index;

    return c->predictor * 16;
}

{

    /* perform direct multiplication instead of series of jumps proposed by
     * the reference ADPCM implementation since modern CPUs can do the mults
     * quickly enough */
    /* predictor update is not so trivial: predictor is multiplied on 254/256 before updating */
    /* calculate new step and clamp it to range 511..32767 */

}

{


    /* clamp result */

    /* calculate new step */

}

{
    }

}

static inline int16_t adpcm_mtaf_expand_nibble(ADPCMChannelStatus *c, uint8_t nibble)
{
    c->predictor += ff_adpcm_mtaf_stepsize[c->step][nibble];
    c->predictor = av_clip_int16(c->predictor);
    c->step += ff_adpcm_index_table[nibble];
    c->step = av_clip_uintp2(c->step, 5);
    return c->predictor;
}

static inline int16_t adpcm_zork_expand_nibble(ADPCMChannelStatus *c, uint8_t nibble)
{
    int16_t index = c->step_index;
    uint32_t lookup_sample = ff_adpcm_step_table[index];
    int32_t sample = 0;

    if (nibble & 0x40)
        sample += lookup_sample;
    if (nibble & 0x20)
        sample += lookup_sample >> 1;
    if (nibble & 0x10)
        sample += lookup_sample >> 2;
    if (nibble & 0x08)
        sample += lookup_sample >> 3;
    if (nibble & 0x04)
        sample += lookup_sample >> 4;
    if (nibble & 0x02)
        sample += lookup_sample >> 5;
    if (nibble & 0x01)
        sample += lookup_sample >> 6;
    if (nibble & 0x80)
        sample = -sample;

    sample += c->predictor;
    sample = av_clip_int16(sample);

    index += zork_index_table[(nibble >> 4) & 7];
    index = av_clip(index, 0, 88);

    c->predictor = sample;
    c->step_index = index;

    return sample;
}

static int xa_decode(AVCodecContext *avctx, int16_t *out0, int16_t *out1,
                     const uint8_t *in, ADPCMChannelStatus *left,
                     ADPCMChannelStatus *right, int channels, int sample_offset)
{
    int i, j;
    int shift,filter,f0,f1;
    int s_1,s_2;
    int d,s,t;

    out0 += sample_offset;
    if (channels == 1)
        out1 = out0 + 28;
    else
        out1 += sample_offset;

    for(i=0;i<4;i++) {
        shift  = 12 - (in[4+i*2] & 15);
        filter = in[4+i*2] >> 4;
        if (filter >= FF_ARRAY_ELEMS(xa_adpcm_table)) {
            avpriv_request_sample(avctx, "unknown XA-ADPCM filter %d", filter);
            filter=0;
        }
        if (shift < 0) {
            avpriv_request_sample(avctx, "unknown XA-ADPCM shift %d", shift);
            shift = 0;
        }
        f0 = xa_adpcm_table[filter][0];
        f1 = xa_adpcm_table[filter][1];

        s_1 = left->sample1;
        s_2 = left->sample2;

        for(j=0;j<28;j++) {
            d = in[16+i+j*4];

            t = sign_extend(d, 4);
            s = t*(1<<shift) + ((s_1*f0 + s_2*f1+32)>>6);
            s_2 = s_1;
            s_1 = av_clip_int16(s);
            out0[j] = s_1;
        }

        if (channels == 2) {
            left->sample1 = s_1;
            left->sample2 = s_2;
            s_1 = right->sample1;
            s_2 = right->sample2;
        }

        shift  = 12 - (in[5+i*2] & 15);
        filter = in[5+i*2] >> 4;
        if (filter >= FF_ARRAY_ELEMS(xa_adpcm_table) || shift < 0) {
            avpriv_request_sample(avctx, "unknown XA-ADPCM filter %d", filter);
            filter=0;
        }
        if (shift < 0) {
            avpriv_request_sample(avctx, "unknown XA-ADPCM shift %d", shift);
            shift = 0;
        }

        f0 = xa_adpcm_table[filter][0];
        f1 = xa_adpcm_table[filter][1];

        for(j=0;j<28;j++) {
            d = in[16+i+j*4];

            t = sign_extend(d >> 4, 4);
            s = t*(1<<shift) + ((s_1*f0 + s_2*f1+32)>>6);
            s_2 = s_1;
            s_1 = av_clip_int16(s);
            out1[j] = s_1;
        }

        if (channels == 2) {
            right->sample1 = s_1;
            right->sample2 = s_2;
        } else {
            left->sample1 = s_1;
            left->sample2 = s_2;
        }

        out0 += 28 * (3 - channels);
        out1 += 28 * (3 - channels);
    }

    return 0;
}

static void adpcm_swf_decode(AVCodecContext *avctx, const uint8_t *buf, int buf_size, int16_t *samples)
{
    ADPCMDecodeContext *c = avctx->priv_data;
    GetBitContext gb;
    const int8_t *table;
    int k0, signmask, nb_bits, count;
    int size = buf_size*8;
    int i;

    init_get_bits(&gb, buf, size);

    //read bits & initial values
    nb_bits = get_bits(&gb, 2)+2;
    table = swf_index_tables[nb_bits-2];
    k0 = 1 << (nb_bits-2);
    signmask = 1 << (nb_bits-1);

    while (get_bits_count(&gb) <= size - 22*avctx->channels) {
        for (i = 0; i < avctx->channels; i++) {
            *samples++ = c->status[i].predictor = get_sbits(&gb, 16);
            c->status[i].step_index = get_bits(&gb, 6);
        }

        for (count = 0; get_bits_count(&gb) <= size - nb_bits*avctx->channels && count < 4095; count++) {
            int i;

            for (i = 0; i < avctx->channels; i++) {
                // similar to IMA adpcm
                int delta = get_bits(&gb, nb_bits);
                int step = ff_adpcm_step_table[c->status[i].step_index];
                int vpdiff = 0; // vpdiff = (delta+0.5)*step/4
                int k = k0;

                do {
                    if (delta & k)
                        vpdiff += step;
                    step >>= 1;
                    k >>= 1;
                } while(k);
                vpdiff += step;

                if (delta & signmask)
                    c->status[i].predictor -= vpdiff;
                else
                    c->status[i].predictor += vpdiff;

                c->status[i].step_index += table[delta & (~signmask)];

                c->status[i].step_index = av_clip(c->status[i].step_index, 0, 88);
                c->status[i].predictor = av_clip_int16(c->status[i].predictor);

                *samples++ = c->status[i].predictor;
            }
        }
    }
}

{

    else



}

/**
 * Get the number of samples (per channel) that will be decoded from the packet.
 * In one case, this is actually the maximum number of samples possible to
 * decode with the given buf_size.
 *
 * @param[out] coded_samples set to the number of samples as coded in the
 *                           packet, or 0 if the codec does not encode the
 *                           number of samples in each frame.
 * @param[out] approx_nb_samples set to non-zero if the number of samples
 *                               returned is an approximation.
 */
                          int buf_size, int *coded_samples, int *approx_nb_samples)
{


        return 0;

    /* constant, only check buf_size */
            return 0;
        nb_samples = 128;
        break;
            return 0;
        nb_samples = 64;
        break;
            return 0;
        nb_samples = 32;
        break;
    /* simple 4-bit adpcm */
    case AV_CODEC_ID_ADPCM_IMA_APC:
    case AV_CODEC_ID_ADPCM_IMA_CUNNING:
    case AV_CODEC_ID_ADPCM_IMA_EA_SEAD:
    case AV_CODEC_ID_ADPCM_IMA_OKI:
    case AV_CODEC_ID_ADPCM_IMA_WS:
    case AV_CODEC_ID_ADPCM_YAMAHA:
    case AV_CODEC_ID_ADPCM_AICA:
    case AV_CODEC_ID_ADPCM_IMA_SSI:
    case AV_CODEC_ID_ADPCM_IMA_APM:
    case AV_CODEC_ID_ADPCM_IMA_ALP:
    case AV_CODEC_ID_ADPCM_IMA_MTF:
    }

    /* simple 4-bit adpcm, with header */
        case AV_CODEC_ID_ADPCM_AGM:
        case AV_CODEC_ID_ADPCM_IMA_DAT4:
        case AV_CODEC_ID_ADPCM_IMA_AMV:     header_size = 8;           break;
    }
    if (header_size > 0)

    /* more complex formats */
    case AV_CODEC_ID_ADPCM_EA_R2:
    case AV_CODEC_ID_ADPCM_EA_R3:
        /* maximum number of samples */
        /* has internal offsets and a per-frame switch to signal raw 16-bit */
        }
            return AVERROR_INVALIDDATA;
    {
            return AVERROR_INVALIDDATA;
    }
    case AV_CODEC_ID_ADPCM_MTAF:
        if (avctx->block_align > 0)
            buf_size = FFMIN(buf_size, avctx->block_align);
        nb_samples = (buf_size - 16 * (ch / 2)) * 2 / ch;
        break;
    case AV_CODEC_ID_ADPCM_SBPRO_3:
    case AV_CODEC_ID_ADPCM_SBPRO_4:
    {
        case AV_CODEC_ID_ADPCM_SBPRO_2: samples_per_byte = 4; break;
        case AV_CODEC_ID_ADPCM_SBPRO_3: samples_per_byte = 3; break;
        case AV_CODEC_ID_ADPCM_SBPRO_4: samples_per_byte = 2; break;
        }
                return AVERROR_INVALIDDATA;
        }
    }
    {
        break;
    }
    case AV_CODEC_ID_ADPCM_THP_LE:
            nb_samples = buf_size * 14 / (8 * ch);
            break;
        }
            nb_samples     += (buf_size % 8 - 1) * 2;
    case AV_CODEC_ID_ADPCM_PSX:
    case AV_CODEC_ID_ADPCM_ZORK:
        nb_samples = buf_size / ch;
        break;
    }

    /* validate coded sample count */
        return AVERROR_INVALIDDATA;

    return nb_samples;
}

                              int *got_frame_ptr, AVPacket *avpkt)
{

        av_log(avctx, AV_LOG_ERROR, "invalid number of samples in packet\n");
        return AVERROR_INVALIDDATA;
    }

    /* get output buffer */
        return ret;

    /* use coded_samples when applicable */
    /* it is always <= nb_samples, so the output buffer will be large enough */
            av_log(avctx, AV_LOG_WARNING, "mismatch in coded sample count\n");
    }


    case AV_CODEC_ID_ADPCM_IMA_QT:
        /* In QuickTime, IMA is encoded by chunks of 34 bytes (=64 samples).
           Channel data is interleaved per-chunk. */
            /* (pppppp) (piiiiiii) */

            /* Bits 15-7 are the _top_ 9 bits of the 16-bit initial predictor value */

            } else {
            update:
            }

                av_log(avctx, AV_LOG_ERROR, "ERROR: step_index[%d] = %i\n",
                       channel, cs->step_index);
                return AVERROR_INVALIDDATA;
            }


            }
        }
        break;
    case AV_CODEC_ID_ADPCM_IMA_WAV:

                av_log(avctx, AV_LOG_ERROR, "ERROR: step_index[%d] = %i\n",
                       i, cs->step_index);
                return AVERROR_INVALIDDATA;
            }
        }

            int samples_per_block = ff_adpcm_ima_block_samples[avctx->bits_per_coded_sample - 2];
            int block_size = ff_adpcm_ima_block_sizes[avctx->bits_per_coded_sample - 2];
            uint8_t temp[20 + AV_INPUT_BUFFER_PADDING_SIZE] = { 0 };
            GetBitContext g;

            for (n = 0; n < (nb_samples - 1) / samples_per_block; n++) {
                for (i = 0; i < avctx->channels; i++) {
                    int j;

                    cs = &c->status[i];
                    samples = &samples_p[i][1 + n * samples_per_block];
                    for (j = 0; j < block_size; j++) {
                        temp[j] = buf[4 * avctx->channels + block_size * n * avctx->channels +
                                        (j % 4) + (j / 4) * (avctx->channels * 4) + i * 4];
                    }
                    ret = init_get_bits8(&g, (const uint8_t *)&temp, block_size);
                    if (ret < 0)
                        return ret;
                    for (m = 0; m < samples_per_block; m++) {
                        samples[m] = adpcm_ima_wav_expand_nibble(cs, &g,
                                          avctx->bits_per_coded_sample);
                    }
                }
            }
            bytestream2_skip(&gb, avctx->block_align - avctx->channels * 4);
        } else {
                }
            }
        }
        }
        break;
    case AV_CODEC_ID_ADPCM_4XM:

                av_log(avctx, AV_LOG_ERROR, "ERROR: step_index[%d] = %i\n",
                       i, c->status[i].step_index);
                return AVERROR_INVALIDDATA;
            }
        }

            }
        }
        break;
    case AV_CODEC_ID_ADPCM_AGM:
        for (i = 0; i < avctx->channels; i++)
            c->status[i].predictor = sign_extend(bytestream2_get_le16u(&gb), 16);
        for (i = 0; i < avctx->channels; i++)
            c->status[i].step = sign_extend(bytestream2_get_le16u(&gb), 16);

        for (n = 0; n < nb_samples >> (1 - st); n++) {
            int v = bytestream2_get_byteu(&gb);
            *samples++ = adpcm_agm_expand_nibble(&c->status[0], v & 0xF);
            *samples++ = adpcm_agm_expand_nibble(&c->status[st], v >> 4 );
        }
        break;
    {

            for (channel = 0; channel < avctx->channels; channel++) {
                samples = samples_p[channel];
                block_predictor = bytestream2_get_byteu(&gb);
                if (block_predictor > 6) {
                    av_log(avctx, AV_LOG_ERROR, "ERROR: block_predictor[%d] = %d\n",
                           channel, block_predictor);
                    return AVERROR_INVALIDDATA;
                }
                c->status[channel].coeff1 = ff_adpcm_AdaptCoeff1[block_predictor];
                c->status[channel].coeff2 = ff_adpcm_AdaptCoeff2[block_predictor];
                c->status[channel].idelta = sign_extend(bytestream2_get_le16u(&gb), 16);
                c->status[channel].sample1 = sign_extend(bytestream2_get_le16u(&gb), 16);
                c->status[channel].sample2 = sign_extend(bytestream2_get_le16u(&gb), 16);
                *samples++ = c->status[channel].sample2;
                *samples++ = c->status[channel].sample1;
                for(n = (nb_samples - 2) >> 1; n > 0; n--) {
                    int byte = bytestream2_get_byteu(&gb);
                    *samples++ = adpcm_ms_expand_nibble(&c->status[channel], byte >> 4  );
                    *samples++ = adpcm_ms_expand_nibble(&c->status[channel], byte & 0x0F);
                }
            }
        } else {
                av_log(avctx, AV_LOG_ERROR, "ERROR: block_predictor[0] = %d\n",
                       block_predictor);
                return AVERROR_INVALIDDATA;
            }
                    av_log(avctx, AV_LOG_ERROR, "ERROR: block_predictor[1] = %d\n",
                           block_predictor);
                    return AVERROR_INVALIDDATA;
                }
            }
            }


            }
        }
        break;
    }
    case AV_CODEC_ID_ADPCM_MTAF:
        for (channel = 0; channel < avctx->channels; channel+=2) {
            bytestream2_skipu(&gb, 4);
            c->status[channel    ].step      = bytestream2_get_le16u(&gb) & 0x1f;
            c->status[channel + 1].step      = bytestream2_get_le16u(&gb) & 0x1f;
            c->status[channel    ].predictor = sign_extend(bytestream2_get_le16u(&gb), 16);
            bytestream2_skipu(&gb, 2);
            c->status[channel + 1].predictor = sign_extend(bytestream2_get_le16u(&gb), 16);
            bytestream2_skipu(&gb, 2);
            for (n = 0; n < nb_samples; n+=2) {
                int v = bytestream2_get_byteu(&gb);
                samples_p[channel][n    ] = adpcm_mtaf_expand_nibble(&c->status[channel], v & 0x0F);
                samples_p[channel][n + 1] = adpcm_mtaf_expand_nibble(&c->status[channel], v >> 4  );
            }
            for (n = 0; n < nb_samples; n+=2) {
                int v = bytestream2_get_byteu(&gb);
                samples_p[channel + 1][n    ] = adpcm_mtaf_expand_nibble(&c->status[channel + 1], v & 0x0F);
                samples_p[channel + 1][n + 1] = adpcm_mtaf_expand_nibble(&c->status[channel + 1], v >> 4  );
            }
        }
        break;
    case AV_CODEC_ID_ADPCM_IMA_DK4:
                av_log(avctx, AV_LOG_ERROR, "ERROR: step_index[%d] = %i\n",
                       channel, cs->step_index);
                return AVERROR_INVALIDDATA;
            }
        }
        }
        break;
    {

            av_log(avctx, AV_LOG_ERROR, "ERROR: step_index = %i/%i\n",
                   c->status[0].step_index, c->status[1].step_index);
            return AVERROR_INVALIDDATA;
        }
        /* sign extend the predictors */
        diff_channel = c->status[1].predictor;

        /* DK3 ADPCM support macro */
#define DK3_GET_NEXT_NIBBLE() \
    if (decode_top_nibble_next) { \
        nibble = last_byte >> 4; \
        decode_top_nibble_next = 0; \
    } else { \
        last_byte = bytestream2_get_byteu(&gb); \
        nibble = last_byte & 0x0F; \
        decode_top_nibble_next = 1; \
    }


            /* for this algorithm, c->status[0] is the sum channel and
             * c->status[1] is the diff channel */

            /* process the first predictor of the sum channel */

            /* process the diff channel predictor */

            /* process the first pair of stereo PCM samples */

            /* process the second predictor of the sum channel */

            /* process the second pair of stereo PCM samples */
        }

        break;
    }
    case AV_CODEC_ID_ADPCM_IMA_ISS:
                av_log(avctx, AV_LOG_ERROR, "ERROR: step_index[%d] = %i\n",
                       channel, cs->step_index);
                return AVERROR_INVALIDDATA;
            }
        }

            /* nibbles are swapped for mono */
                v1 = v >> 4;
                v2 = v & 0x0F;
            } else {
            }
        }
        break;
    case AV_CODEC_ID_ADPCM_IMA_DAT4:
        for (channel = 0; channel < avctx->channels; channel++) {
            cs = &c->status[channel];
            samples = samples_p[channel];
            bytestream2_skip(&gb, 4);
            for (n = 0; n < nb_samples; n += 2) {
                int v = bytestream2_get_byteu(&gb);
                *samples++ = adpcm_ima_expand_nibble(cs, v >> 4  , 3);
                *samples++ = adpcm_ima_expand_nibble(cs, v & 0x0F, 3);
            }
        }
        break;
        }
        break;
        }
        break;
            }
        }
        break;
            }
        }
        break;
    case AV_CODEC_ID_ADPCM_IMA_CUNNING:
        }
        break;
        }
        break;
    case AV_CODEC_ID_ADPCM_IMA_RAD:
                av_log(avctx, AV_LOG_ERROR, "ERROR: step_index[%d] = %i\n",
                       channel, cs->step_index);
                return AVERROR_INVALIDDATA;
            }
        }

            }
            }
        }
        break;
            for (channel = 0; channel < avctx->channels; channel++) {
                int16_t *smp = samples_p[channel];

                for (n = nb_samples / 2; n > 0; n--) {
                    int v = bytestream2_get_byteu(&gb);
                    *smp++ = adpcm_ima_expand_nibble(&c->status[channel], v >> 4  , 3);
                    *smp++ = adpcm_ima_expand_nibble(&c->status[channel], v & 0x0F, 3);
                }
            }
        } else {
                }
            }
        }
        break;
    {
                                 &c->status[0], &c->status[1],
                                 avctx->channels, sample_offset)) < 0)
                return ret;
        }
        /* Less than a full block of data left, e.g. when reading from
         * 2324 byte per sector XA; the remainder is padding */
            bytestream2_skip(&gb, bytes_remaining);
        }
        break;
    }
    case AV_CODEC_ID_ADPCM_IMA_EA_EACS:
                av_log(avctx, AV_LOG_ERROR, "ERROR: step_index[%d] = %i\n",
                       i, c->status[i].step_index);
                return AVERROR_INVALIDDATA;
            }
        }
                return AVERROR_INVALIDDATA;
        }

        }
        break;
        }
        break;
    {

        /* Each EA ADPCM frame has a 12-byte header followed by 30-byte pieces,
           each coding 28 stereo samples. */

            return AVERROR_INVALIDDATA;






            }
        }


        break;
    }
    case AV_CODEC_ID_ADPCM_EA_MAXIS_XA:
    {
        int coeff[2][2], shift[2];

        }

                }
            }
        }
    }
    case AV_CODEC_ID_ADPCM_EA_R1:
    case AV_CODEC_ID_ADPCM_EA_R2:
    case AV_CODEC_ID_ADPCM_EA_R3: {
        /* channel numbering
           2chan: 0=fl, 1=fr
           4chan: 0=fl, 1=rl, 2=fr, 3=rr
           6chan: 0=fl, 1=c,  2=fr, 3=rl,  4=rr, 5=sub */
        int previous_sample, current_sample, next_sample;
        int coeff1, coeff2;
        int shift;
        unsigned int channel;
        uint16_t *samplesC;
        int offsets[6];



            } else {
            }


                } else {

                        else {
                        }


                    }
                }
            }
                count = count1;
                av_log(avctx, AV_LOG_WARNING, "per-channel sample count mismatch\n");
                count = FFMAX(count, count1);
            }

            }
        }

    }
    case AV_CODEC_ID_ADPCM_EA_XAS:

            }



                }
            }
        }
        break;
    case AV_CODEC_ID_ADPCM_IMA_AMV:
            av_log(avctx, AV_LOG_ERROR, "ERROR: step_index = %i\n",
                   c->status[0].step_index);
            return AVERROR_INVALIDDATA;
        }


        }
        break;
    case AV_CODEC_ID_ADPCM_IMA_SMJPEG:
                av_log(avctx, AV_LOG_ERROR, "ERROR: step_index = %i\n",
                       c->status[i].step_index);
                return AVERROR_INVALIDDATA;
            }
        }


        }
        break;
        }
        break;
    case AV_CODEC_ID_ADPCM_SBPRO_3:
    case AV_CODEC_ID_ADPCM_SBPRO_2:
            /* the first byte is a raw sample */
                *samples++ = 128 * (bytestream2_get_byteu(&gb) - 0x80);
        }
            }
            }
        } else {
            }
        }
        break;
        break;
        }
        break;
    case AV_CODEC_ID_ADPCM_AICA:
        if (!c->has_status) {
            for (channel = 0; channel < avctx->channels; channel++)
                c->status[channel].step = 0;
            c->has_status = 1;
        }
        for (channel = 0; channel < avctx->channels; channel++) {
            samples = samples_p[channel];
            for (n = nb_samples >> 1; n > 0; n--) {
                int v = bytestream2_get_byteu(&gb);
                *samples++ = adpcm_yamaha_expand_nibble(&c->status[channel], v & 0x0F);
                *samples++ = adpcm_yamaha_expand_nibble(&c->status[channel], v >> 4  );
            }
        }
        break;
    {

            samples_per_block = avctx->extradata[0] / 16;
            blocks = nb_samples / avctx->extradata[0];
        } else {
        }


            /* Read in every sample for this channel.  */

                /* Decode 16 samples.  */

                    } else {
                    }

                }
            }

        }
        }
        break;
    }
    case AV_CODEC_ID_ADPCM_THP_LE:
    {

#define THP_GET16(g) \
    sign_extend( \
        avctx->codec->id == AV_CODEC_ID_ADPCM_THP_LE ? \
        bytestream2_get_le16u(&(g)) : \
        bytestream2_get_be16u(&(g)), 16)

            GetByteContext tb;
            if (avctx->extradata_size < 32 * avctx->channels) {
                av_log(avctx, AV_LOG_ERROR, "Missing coeff table\n");
                return AVERROR_INVALIDDATA;
            }

            bytestream2_init(&tb, avctx->extradata, avctx->extradata_size);
            for (i = 0; i < avctx->channels; i++)
                for (n = 0; n < 16; n++)
                    table[i][n] = THP_GET16(tb);
        } else {

                /* Initialize the previous sample.  */
                }
            } else {
            }
        }


            /* Read in every sample for this channel.  */

                /* Decode 14 samples.  */

                    } else {
                    }

                }
            }
        }
    }
    case AV_CODEC_ID_ADPCM_DTK:

            /* Read in every sample for this channel.  */

                /* Decode 28 samples.  */

                    default:
                        prev = 0;
                    }


                    else

                }
            }
        }
        break;
    case AV_CODEC_ID_ADPCM_PSX:
        for (channel = 0; channel < avctx->channels; channel++) {
            samples = samples_p[channel];

            /* Read in every sample for this channel.  */
            for (i = 0; i < nb_samples / 28; i++) {
                int filter, shift, flag, byte;

                filter = bytestream2_get_byteu(&gb);
                shift  = filter & 0xf;
                filter = filter >> 4;
                if (filter >= FF_ARRAY_ELEMS(xa_adpcm_table))
                    return AVERROR_INVALIDDATA;
                flag   = bytestream2_get_byteu(&gb);

                /* Decode 28 samples.  */
                for (n = 0; n < 28; n++) {
                    int sample = 0, scale;

                    if (flag < 0x07) {
                        if (n & 1) {
                            scale = sign_extend(byte >> 4, 4);
                        } else {
                            byte  = bytestream2_get_byteu(&gb);
                            scale = sign_extend(byte, 4);
                        }

                        scale  = scale * (1 << 12);
                        sample = (int)((scale >> shift) + (c->status[channel].sample1 * xa_adpcm_table[filter][0] + c->status[channel].sample2 * xa_adpcm_table[filter][1]) / 64);
                    }
                    *samples++ = av_clip_int16(sample);
                    c->status[channel].sample2 = c->status[channel].sample1;
                    c->status[channel].sample1 = sample;
                }
            }
        }
        break;
    case AV_CODEC_ID_ADPCM_ARGO:
        /*
         * The format of each block:
         *   uint8_t left_control;
         *   uint4_t left_samples[nb_samples];
         *   ---- and if stereo ----
         *   uint8_t right_control;
         *   uint4_t right_samples[nb_samples];
         *
         * Format of the control byte:
         * MSB [SSSSRDRR] LSB
         *   S = (Shift Amount - 2)
         *   D = Decoder flag.
         *   R = Reserved
         *
         * Each block relies on the previous two samples of each channel.
         * They should be 0 initially.
         */


            /* Get the control byte and decode the samples, 2 at a time. */

            }
        }
        break;
    case AV_CODEC_ID_ADPCM_ZORK:
        if (!c->has_status) {
            for (channel = 0; channel < avctx->channels; channel++) {
                c->status[channel].predictor  = 0;
                c->status[channel].step_index = 0;
            }
            c->has_status = 1;
        }
        for (n = 0; n < nb_samples * avctx->channels; n++) {
            int v = bytestream2_get_byteu(&gb);
            *samples++ = adpcm_zork_expand_nibble(&c->status[n % avctx->channels], v);
        }
        break;
    case AV_CODEC_ID_ADPCM_IMA_MTF:
        for (n = nb_samples / 2; n > 0; n--) {
            for (channel = 0; channel < avctx->channels; channel++) {
                int v = bytestream2_get_byteu(&gb);
                *samples++  = adpcm_ima_mtf_expand_nibble(&c->status[channel], v >> 4);
                samples[st] = adpcm_ima_mtf_expand_nibble(&c->status[channel], v & 0x0F);
            }
            samples += avctx->channels;
        }
        break;
    default:
        av_assert0(0); // unsupported codec_id should not happen
    }

        av_log(avctx, AV_LOG_ERROR, "Nothing consumed\n");
        return AVERROR_INVALIDDATA;
    }


        av_log(avctx, AV_LOG_ERROR, "Overread of %d < %d\n", avpkt->size, bytestream2_tell(&gb));
        return avpkt->size;
    }

}

static void adpcm_flush(AVCodecContext *avctx)
{
    ADPCMDecodeContext *c = avctx->priv_data;
    c->has_status = 0;
}


static const enum AVSampleFormat sample_fmts_s16[]  = { AV_SAMPLE_FMT_S16,
                                                        AV_SAMPLE_FMT_NONE };
static const enum AVSampleFormat sample_fmts_s16p[] = { AV_SAMPLE_FMT_S16P,
                                                        AV_SAMPLE_FMT_NONE };
static const enum AVSampleFormat sample_fmts_both[] = { AV_SAMPLE_FMT_S16,
                                                        AV_SAMPLE_FMT_S16P,
                                                        AV_SAMPLE_FMT_NONE };

#define ADPCM_DECODER(id_, sample_fmts_, name_, long_name_) \
AVCodec ff_ ## name_ ## _decoder = {                        \
    .name           = #name_,                               \
    .long_name      = NULL_IF_CONFIG_SMALL(long_name_),     \
    .type           = AVMEDIA_TYPE_AUDIO,                   \
    .id             = id_,                                  \
    .priv_data_size = sizeof(ADPCMDecodeContext),           \
    .init           = adpcm_decode_init,                    \
    .decode         = adpcm_decode_frame,                   \
    .flush          = adpcm_flush,                          \
    .capabilities   = AV_CODEC_CAP_DR1,                     \
    .sample_fmts    = sample_fmts_,                         \
}

/* Note: Do not forget to add new entries to the Makefile as well. */
ADPCM_DECODER(AV_CODEC_ID_ADPCM_4XM,         sample_fmts_s16p, adpcm_4xm,         "ADPCM 4X Movie");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_AFC,         sample_fmts_s16p, adpcm_afc,         "ADPCM Nintendo Gamecube AFC");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_AGM,         sample_fmts_s16,  adpcm_agm,         "ADPCM AmuseGraphics Movie");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_AICA,        sample_fmts_s16p, adpcm_aica,        "ADPCM Yamaha AICA");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_ARGO,        sample_fmts_s16p, adpcm_argo,        "ADPCM Argonaut Games");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_CT,          sample_fmts_s16,  adpcm_ct,          "ADPCM Creative Technology");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_DTK,         sample_fmts_s16p, adpcm_dtk,         "ADPCM Nintendo Gamecube DTK");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_EA,          sample_fmts_s16,  adpcm_ea,          "ADPCM Electronic Arts");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_EA_MAXIS_XA, sample_fmts_s16,  adpcm_ea_maxis_xa, "ADPCM Electronic Arts Maxis CDROM XA");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_EA_R1,       sample_fmts_s16p, adpcm_ea_r1,       "ADPCM Electronic Arts R1");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_EA_R2,       sample_fmts_s16p, adpcm_ea_r2,       "ADPCM Electronic Arts R2");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_EA_R3,       sample_fmts_s16p, adpcm_ea_r3,       "ADPCM Electronic Arts R3");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_EA_XAS,      sample_fmts_s16p, adpcm_ea_xas,      "ADPCM Electronic Arts XAS");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_IMA_AMV,     sample_fmts_s16,  adpcm_ima_amv,     "ADPCM IMA AMV");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_IMA_APC,     sample_fmts_s16,  adpcm_ima_apc,     "ADPCM IMA CRYO APC");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_IMA_APM,     sample_fmts_s16,  adpcm_ima_apm,     "ADPCM IMA Ubisoft APM");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_IMA_CUNNING, sample_fmts_s16,  adpcm_ima_cunning, "ADPCM IMA Cunning Developments");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_IMA_DAT4,    sample_fmts_s16,  adpcm_ima_dat4,    "ADPCM IMA Eurocom DAT4");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_IMA_DK3,     sample_fmts_s16,  adpcm_ima_dk3,     "ADPCM IMA Duck DK3");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_IMA_DK4,     sample_fmts_s16,  adpcm_ima_dk4,     "ADPCM IMA Duck DK4");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_IMA_EA_EACS, sample_fmts_s16,  adpcm_ima_ea_eacs, "ADPCM IMA Electronic Arts EACS");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_IMA_EA_SEAD, sample_fmts_s16,  adpcm_ima_ea_sead, "ADPCM IMA Electronic Arts SEAD");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_IMA_ISS,     sample_fmts_s16,  adpcm_ima_iss,     "ADPCM IMA Funcom ISS");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_IMA_MTF,     sample_fmts_s16,  adpcm_ima_mtf,     "ADPCM IMA Capcom's MT Framework");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_IMA_OKI,     sample_fmts_s16,  adpcm_ima_oki,     "ADPCM IMA Dialogic OKI");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_IMA_QT,      sample_fmts_s16p, adpcm_ima_qt,      "ADPCM IMA QuickTime");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_IMA_RAD,     sample_fmts_s16,  adpcm_ima_rad,     "ADPCM IMA Radical");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_IMA_SSI,     sample_fmts_s16,  adpcm_ima_ssi,     "ADPCM IMA Simon & Schuster Interactive");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_IMA_SMJPEG,  sample_fmts_s16,  adpcm_ima_smjpeg,  "ADPCM IMA Loki SDL MJPEG");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_IMA_ALP,     sample_fmts_s16,  adpcm_ima_alp,     "ADPCM IMA High Voltage Software ALP");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_IMA_WAV,     sample_fmts_s16p, adpcm_ima_wav,     "ADPCM IMA WAV");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_IMA_WS,      sample_fmts_both, adpcm_ima_ws,      "ADPCM IMA Westwood");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_MS,          sample_fmts_both, adpcm_ms,          "ADPCM Microsoft");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_MTAF,        sample_fmts_s16p, adpcm_mtaf,        "ADPCM MTAF");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_PSX,         sample_fmts_s16p, adpcm_psx,         "ADPCM Playstation");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_SBPRO_2,     sample_fmts_s16,  adpcm_sbpro_2,     "ADPCM Sound Blaster Pro 2-bit");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_SBPRO_3,     sample_fmts_s16,  adpcm_sbpro_3,     "ADPCM Sound Blaster Pro 2.6-bit");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_SBPRO_4,     sample_fmts_s16,  adpcm_sbpro_4,     "ADPCM Sound Blaster Pro 4-bit");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_SWF,         sample_fmts_s16,  adpcm_swf,         "ADPCM Shockwave Flash");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_THP_LE,      sample_fmts_s16p, adpcm_thp_le,      "ADPCM Nintendo THP (little-endian)");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_THP,         sample_fmts_s16p, adpcm_thp,         "ADPCM Nintendo THP");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_XA,          sample_fmts_s16p, adpcm_xa,          "ADPCM CDROM XA");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_YAMAHA,      sample_fmts_s16,  adpcm_yamaha,      "ADPCM Yamaha");
ADPCM_DECODER(AV_CODEC_ID_ADPCM_ZORK,        sample_fmts_s16,  adpcm_zork,        "ADPCM Zork");
