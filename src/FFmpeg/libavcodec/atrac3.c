/*
 * ATRAC3 compatible decoder
 * Copyright (c) 2006-2008 Maxim Poliakovski
 * Copyright (c) 2006-2008 Benjamin Larsson
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
 * ATRAC3 compatible decoder.
 * This decoder handles Sony's ATRAC3 data.
 *
 * Container formats used to store ATRAC3 data:
 * RealMedia (.rm), RIFF WAV (.wav, .at3), Sony OpenMG (.oma, .aa3).
 *
 * To use this decoder, a calling application must supply the extradata
 * bytes provided in the containers above.
 */

#include <math.h>
#include <stddef.h>
#include <stdio.h>

#include "libavutil/attributes.h"
#include "libavutil/float_dsp.h"
#include "libavutil/libm.h"
#include "avcodec.h"
#include "bytestream.h"
#include "fft.h"
#include "get_bits.h"
#include "internal.h"

#include "atrac.h"
#include "atrac3data.h"

#define MIN_CHANNELS    1
#define MAX_CHANNELS    8
#define MAX_JS_PAIRS    8 / 2

#define JOINT_STEREO    0x12
#define SINGLE          0x2

#define SAMPLES_PER_FRAME 1024
#define MDCT_SIZE          512

typedef struct GainBlock {
    AtracGainInfo g_block[4];
} GainBlock;

typedef struct TonalComponent {
    int pos;
    int num_coefs;
    float coef[8];
} TonalComponent;

typedef struct ChannelUnit {
    int            bands_coded;
    int            num_components;
    float          prev_frame[SAMPLES_PER_FRAME];
    int            gc_blk_switch;
    TonalComponent components[64];
    GainBlock      gain_block[2];

    DECLARE_ALIGNED(32, float, spectrum)[SAMPLES_PER_FRAME];
    DECLARE_ALIGNED(32, float, imdct_buf)[SAMPLES_PER_FRAME];

    float          delay_buf1[46]; ///<qmf delay buffers
    float          delay_buf2[46];
    float          delay_buf3[46];
} ChannelUnit;

typedef struct ATRAC3Context {
    GetBitContext gb;
    //@{
    /** stream data */
    int coding_mode;

    ChannelUnit *units;
    //@}
    //@{
    /** joint-stereo related variables */
    int matrix_coeff_index_prev[MAX_JS_PAIRS][4];
    int matrix_coeff_index_now[MAX_JS_PAIRS][4];
    int matrix_coeff_index_next[MAX_JS_PAIRS][4];
    int weighting_delay[MAX_JS_PAIRS][6];
    //@}
    //@{
    /** data buffers */
    uint8_t *decoded_bytes_buffer;
    float temp_buf[1070];
    //@}
    //@{
    /** extradata */
    int scrambled_stream;
    //@}

    AtracGCContext    gainc_ctx;
    FFTContext        mdct_ctx;
    AVFloatDSPContext *fdsp;
} ATRAC3Context;

static DECLARE_ALIGNED(32, float, mdct_window)[MDCT_SIZE];
static VLC_TYPE atrac3_vlc_table[4096][2];
static VLC   spectral_coeff_tab[7];

/**
 * Regular 512 points IMDCT without overlapping, with the exception of the
 * swapping of odd bands caused by the reverse spectra of the QMF.
 *
 * @param odd_band  1 if the band is an odd band
 */
{

        /**
         * Reverse the odd bands before IMDCT, this is an effect of the QMF
         * transform or it gives better compression to do it this way.
         * FIXME: It should be possible to handle this in imdct_calc
         * for that to happen a modification of the prerotation step of
         * all SIMD code and C code is needed.
         * Or fix the functions before so they generate a pre reversed spectrum.
         */
    }


    /* Perform windowing on the output. */

/*
 * indata descrambling, only used for data coming from the rm container
 */
static int decode_bytes(const uint8_t *input, uint8_t *out, int bytes)
{
    int i, off;
    uint32_t c;
    const uint32_t *buf;
    uint32_t *output = (uint32_t *)out;

    off = (intptr_t)input & 3;
    buf = (const uint32_t *)(input - off);
    if (off)
        c = av_be2ne32((0x537F6103U >> (off * 8)) | (0x537F6103U << (32 - (off * 8))));
    else
        c = av_be2ne32(0x537F6103U);
    bytes += 3 + off;
    for (i = 0; i < bytes / 4; i++)
        output[i] = c ^ buf[i];

    if (off)
        avpriv_request_sample(NULL, "Offset of %d", off);

    return off;
}

{

    /* generate the mdct window, for details see
     * http://wiki.multimedia.cx/index.php?title=RealAudio_atrc#Windows */
    }

{



}

/**
 * Mantissa decoding
 *
 * @param selector     which table the output values are coded with
 * @param coding_flag  constant length coding or variable length coding
 * @param mantissas    mantissa output table
 * @param num_codes    number of values to get
 */
                                       int coding_flag, int *mantissas,
                                       int num_codes)
{


        /* constant length coding (CLC) */
        int num_bits = clc_length_tab[selector];

        if (selector > 1) {
            for (i = 0; i < num_codes; i++) {
                if (num_bits)
                    code = get_sbits(gb, num_bits);
                else
                    code = 0;
                mantissas[i] = code;
            }
        } else {
            for (i = 0; i < num_codes; i++) {
                if (num_bits)
                    code = get_bits(gb, num_bits); // num_bits is always 4 in this case
                else
                    code = 0;
                mantissas[i * 2    ] = mantissa_clc_tab[code >> 2];
                mantissas[i * 2 + 1] = mantissa_clc_tab[code &  3];
            }
        }
    } else {
        /* variable length coding (VLC) */
            }
        } else {
                                     spectral_coeff_tab[selector - 1].bits, 3);
            }
        }
    }

/**
 * Restore the quantized band spectrum coefficients
 *
 * @return subband count, fix for broken specification/files
 */
{


    /* get the VLC selector table for the subbands, 0 means not coded */

    /* read the scale factor indexes from the stream */
    }



            /* decode spectral coefficients for this subband */
            /* TODO: This can be done faster is several blocks share the
             * same VLC selector (subband_vlc_index) */
                                       mantissas, subband_size);

            /* decode the scale factor for this subband */

            /* inverse quantize the coefficients */
        } else {
            /* this subband was not coded, so zero the entire subband */
        }
    }

    /* clear the subbands that were not coded */
}

/**
 * Restore the quantized tonal components
 *
 * @param components tonal components
 * @param num_bands  number of coded bands
 */
                                   TonalComponent *components, int num_bands)
{


    /* no tonal components */
        return 0;

    coding_mode_selector = get_bits(gb, 2);
    if (coding_mode_selector == 2)
        return AVERROR_INVALIDDATA;

    coding_mode = coding_mode_selector & 1;

    for (i = 0; i < nb_components; i++) {
        int coded_values_per_component, quant_step_index;

        for (b = 0; b <= num_bands; b++)
            band_flags[b] = get_bits1(gb);

        coded_values_per_component = get_bits(gb, 3);

        quant_step_index = get_bits(gb, 3);
        if (quant_step_index <= 1)
            return AVERROR_INVALIDDATA;

        if (coding_mode_selector == 3)
            coding_mode = get_bits1(gb);

        for (b = 0; b < (num_bands + 1) * 4; b++) {
            int coded_components;

            if (band_flags[b >> 2] == 0)
                continue;

            coded_components = get_bits(gb, 3);

            for (c = 0; c < coded_components; c++) {
                TonalComponent *cmp = &components[component_count];
                int sf_index, coded_values, max_coded_values;
                float scale_factor;

                sf_index = get_bits(gb, 6);
                if (component_count >= 64)
                    return AVERROR_INVALIDDATA;

                cmp->pos = b * 64 + get_bits(gb, 6);

                max_coded_values = SAMPLES_PER_FRAME - cmp->pos;
                coded_values     = coded_values_per_component + 1;
                coded_values     = FFMIN(max_coded_values, coded_values);

                scale_factor = ff_atrac_sf_table[sf_index] *
                               inv_max_quant[quant_step_index];

                read_quant_spectral_coeffs(gb, quant_step_index, coding_mode,
                                           mantissa, coded_values);

                cmp->num_coefs = coded_values;

                /* inverse quant */
                for (m = 0; m < coded_values; m++)
                    cmp->coef[m] = mantissa[m] * scale_factor;

                component_count++;
            }
        }
    }

    return component_count;
}

/**
 * Decode gain parameters for the coded bands
 *
 * @param block      the gainblock for the current band
 * @param num_bands  amount of coded bands
 */
                               int num_bands)
{



                return AVERROR_INVALIDDATA;
        }
    }

    /* Clear the unused blocks. */

    return 0;
}

/**
 * Combine the tonal band spectrum and regular band spectrum
 *
 * @param spectrum        output spectrum buffer
 * @param num_components  number of tonal components
 * @param components      tonal components for this band
 * @return                position of the last tonal coefficient
 */
                                TonalComponent *components)
{

        last_pos = FFMAX(components[i].pos + components[i].num_coefs, last_pos);
        input    = components[i].coef;
        output   = &spectrum[components[i].pos];

        for (j = 0; j < components[i].num_coefs; j++)
            output[j] += input[j];
    }

}

#define INTERPOLATE(old, new, nsample) \
    ((old) + (nsample) * 0.125 * ((new) - (old)))

                              int *curr_code)
{


            /* Selector value changed, interpolation needed. */

            /* Interpolation is done over the first eight samples. */
            }
        }

        /* Apply the matrix without interpolation. */
        case 0:     /* M/S decoding */
            }
            break;
        case 1:
            for (; nsample < band + 256; nsample++) {
                float c1 = su1[nsample];
                float c2 = su2[nsample];
                su1[nsample] = (c1 + c2) *  2.0;
                su2[nsample] =  c2       * -2.0;
            }
            break;
        case 2:
        case 3:
            }
            break;
        }
    }

{
    } else {
    }

{
    /* w[x][y] y=0 is left y=1 is right */


            }
            }
        }
    }

/**
 * Decode a Sound Unit
 *
 * @param snd           the channel unit to be used
 * @param output        the decoded samples before IQMF in float representation
 * @param channel_num   channel number
 * @param coding_mode   the coding mode (JOINT_STEREO or single channels)
 */
                                     ChannelUnit *snd, float *output,
                                     int channel_num, int coding_mode)
{

            av_log(NULL,AV_LOG_ERROR,"JS mono Sound Unit id != 3.\n");
            return AVERROR_INVALIDDATA;
        }
    } else {
            av_log(NULL,AV_LOG_ERROR,"Sound Unit id != 0x28.\n");
            return AVERROR_INVALIDDATA;
        }
    }

    /* number of coded QMF bands */

        return ret;

                                                  snd->bands_coded);
        return snd->num_components;


    /* Merge the decoded spectrum and tonal components. */
                                      snd->components);


    /* calculate number of used MLT/QMF bands according to the amount of coded
       spectral lines */
        num_bands = FFMAX((last_tonal + 256) >> 8, num_bands);


    /* Reconstruct time domain samples. */
        /* Perform the IMDCT step without overlapping. */
        else

        /* gain compensation and overlapping */
                                   &snd->prev_frame[band * 256],
                                   &gain1->g_block[band], &gain2->g_block[band],
    }

    /* Swap the gain control buffers for the next frame. */

}

                        float **out_samples)
{

        /* channel coupling mode */

        /* Decode sound unit pairs (channels are expected to be even).
         * Multichannel joint stereo interleaves pairs (6ch: 2ch + 2ch + 2ch) */



            /* Set the bitstream reader at the start of first channel sound unit. */
                          js_databuf, js_block_align * 8);

            /* decode Sound Unit 1 */
                return ret;

            /* Framedata of the su2 in the joint-stereo mode is encoded in
             * reverse byte order so we need to swap it first. */
                uint8_t *ptr2 = q->decoded_bytes_buffer + js_block_align - 1;
                ptr1          = q->decoded_bytes_buffer;
                for (i = 0; i < js_block_align / 2; i++, ptr1++, ptr2--)
                    FFSWAP(uint8_t, *ptr1, *ptr2);
            } else {
            }

            /* Skip the sync codes (0xF8). */
                if (i >= js_block_align)
                    return AVERROR_INVALIDDATA;
            }


            /* set the bitstream reader at the start of the second Sound Unit */
                return ret;

            /* Fill the Weighting coeffs delay buffer */
                    4 * sizeof(*q->weighting_delay[js_pair]));

            }

            /* Decode Sound Unit 2. */
                return ret;

            /* Reconstruct the channel coefficients. */

        }
    } else {
        /* single channels */
        /* Decode the channel sound units. */
            /* Set the bitstream reader at the start of a channel sound unit. */

                return ret;
        }
    }

    /* Apply the iQMF synthesis filter. */
    }

    return 0;
}

static int al_decode_frame(AVCodecContext *avctx, const uint8_t *databuf,
                           int size, float **out_samples)
{
    ATRAC3Context *q = avctx->priv_data;
    int ret, i;

    /* Set the bitstream reader at the start of a channel sound unit. */
    init_get_bits(&q->gb, databuf, size * 8);
    /* single channels */
    /* Decode the channel sound units. */
    for (i = 0; i < avctx->channels; i++) {
        ret = decode_channel_sound_unit(q, &q->gb, &q->units[i],
                                        out_samples[i], i, q->coding_mode);
        if (ret != 0)
            return ret;
        while (i < avctx->channels && get_bits_left(&q->gb) > 6 && show_bits(&q->gb, 6) != 0x28) {
            skip_bits(&q->gb, 1);
        }
    }

    /* Apply the iQMF synthesis filter. */
    for (i = 0; i < avctx->channels; i++) {
        float *p1 = out_samples[i];
        float *p2 = p1 + 256;
        float *p3 = p2 + 256;
        float *p4 = p3 + 256;
        ff_atrac_iqmf(p1, p2, 256, p1, q->units[i].delay_buf1, q->temp_buf);
        ff_atrac_iqmf(p4, p3, 256, p3, q->units[i].delay_buf2, q->temp_buf);
        ff_atrac_iqmf(p1, p3, 512, p1, q->units[i].delay_buf3, q->temp_buf);
    }

    return 0;
}

                               int *got_frame_ptr, AVPacket *avpkt)
{

               "Frame too small (%d bytes). Truncated file?\n", buf_size);
    }

    /* get output buffer */
        return ret;

    /* Check if we need to descramble and what buffer to pass on. */
        decode_bytes(buf, q->decoded_bytes_buffer, avctx->block_align);
        databuf = q->decoded_bytes_buffer;
    } else {
        databuf = buf;
    }

        av_log(avctx, AV_LOG_ERROR, "Frame decoding error!\n");
        return ret;
    }


}

static int atrac3al_decode_frame(AVCodecContext *avctx, void *data,
                                 int *got_frame_ptr, AVPacket *avpkt)
{
    AVFrame *frame = data;
    int ret;

    frame->nb_samples = SAMPLES_PER_FRAME;
    if ((ret = ff_get_buffer(avctx, frame, 0)) < 0)
        return ret;

    ret = al_decode_frame(avctx, avpkt->data, avpkt->size,
                          (float **)frame->extended_data);
    if (ret) {
        av_log(avctx, AV_LOG_ERROR, "Frame decoding error!\n");
        return ret;
    }

    *got_frame_ptr = 1;

    return avpkt->size;
}

{


    /* Initialize the VLC tables. */
                                                atrac3_vlc_offs[i    ];
                 huff_bits[i],  1, 1,
                 huff_codes[i], 1, 1, INIT_VLC_USE_NEW_STATIC);
    }

{

        av_log(avctx, AV_LOG_ERROR, "Channel configuration error!\n");
        return AVERROR(EINVAL);
    }


    /* Take care of the codec-specific extradata. */
        version           = 4;
        samples_per_frame = SAMPLES_PER_FRAME * avctx->channels;
        delay             = 0x88E;
        q->coding_mode    = SINGLE;
        /* Parse the extradata, WAV format */
               bytestream_get_le16(&edata_ptr));  // Unknown value always 1
               bytestream_get_le16(&edata_ptr));  //Dupe of coding mode
               bytestream_get_le16(&edata_ptr));  // Unknown always 0

        /* setup */

            av_log(avctx, AV_LOG_ERROR, "Unknown frame/channel/frame_factor "
                   "configuration %d/%d/%d\n", avctx->block_align,
                   avctx->channels, frame_factor);
            return AVERROR_INVALIDDATA;
        }
    } else if (avctx->extradata_size == 12 || avctx->extradata_size == 10) {
        /* Parse the extradata, RM format. */
        version                = bytestream_get_be32(&edata_ptr);
        samples_per_frame      = bytestream_get_be16(&edata_ptr);
        delay                  = bytestream_get_be16(&edata_ptr);
        q->coding_mode         = bytestream_get_be16(&edata_ptr);
        q->scrambled_stream    = 1;

    } else {
        av_log(avctx, AV_LOG_ERROR, "Unknown extradata size %d.\n",
               avctx->extradata_size);
        return AVERROR(EINVAL);
    }

    /* Check the extradata */

        av_log(avctx, AV_LOG_ERROR, "Version %d != 4.\n", version);
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "Unknown amount of samples per frame %d.\n",
               samples_per_frame);
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "Unknown amount of delay %x != 0x88E.\n",
               delay);
        return AVERROR_INVALIDDATA;
    }

            av_log(avctx, AV_LOG_ERROR, "Invalid joint stereo channel configuration.\n");
            return AVERROR_INVALIDDATA;
        }
    } else {
        av_log(avctx, AV_LOG_ERROR, "Unknown channel coding mode %x!\n",
               q->coding_mode);
        return AVERROR_INVALIDDATA;
    }

        return AVERROR(EINVAL);

                                         AV_INPUT_BUFFER_PADDING_SIZE);
        return AVERROR(ENOMEM);


    /* initialize the MDCT transform */
        av_log(avctx, AV_LOG_ERROR, "Error initializing MDCT\n");
        av_freep(&q->decoded_bytes_buffer);
        return ret;
    }

    /* init the joint-stereo decoding data */

        }
    }


        atrac3_decode_close(avctx);
        return AVERROR(ENOMEM);
    }

    return 0;
}

AVCodec ff_atrac3_decoder = {
    .name             = "atrac3",
    .long_name        = NULL_IF_CONFIG_SMALL("ATRAC3 (Adaptive TRansform Acoustic Coding 3)"),
    .type             = AVMEDIA_TYPE_AUDIO,
    .id               = AV_CODEC_ID_ATRAC3,
    .priv_data_size   = sizeof(ATRAC3Context),
    .init             = atrac3_decode_init,
    .close            = atrac3_decode_close,
    .decode           = atrac3_decode_frame,
    .capabilities     = AV_CODEC_CAP_SUBFRAMES | AV_CODEC_CAP_DR1,
    .sample_fmts      = (const enum AVSampleFormat[]) { AV_SAMPLE_FMT_FLTP,
                                                        AV_SAMPLE_FMT_NONE },
};

AVCodec ff_atrac3al_decoder = {
    .name             = "atrac3al",
    .long_name        = NULL_IF_CONFIG_SMALL("ATRAC3 AL (Adaptive TRansform Acoustic Coding 3 Advanced Lossless)"),
    .type             = AVMEDIA_TYPE_AUDIO,
    .id               = AV_CODEC_ID_ATRAC3AL,
    .priv_data_size   = sizeof(ATRAC3Context),
    .init             = atrac3_decode_init,
    .close            = atrac3_decode_close,
    .decode           = atrac3al_decode_frame,
    .capabilities     = AV_CODEC_CAP_SUBFRAMES | AV_CODEC_CAP_DR1,
    .sample_fmts      = (const enum AVSampleFormat[]) { AV_SAMPLE_FMT_FLTP,
                                                        AV_SAMPLE_FMT_NONE },
};
