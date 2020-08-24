/*
 * SIPR / ACELP.NET decoder
 *
 * Copyright (c) 2008 Vladimir Voroshilov
 * Copyright (c) 2009 Vitor Sessak
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

#include <math.h>
#include <stdint.h>
#include <string.h>

#include "libavutil/channel_layout.h"
#include "libavutil/float_dsp.h"
#include "libavutil/mathematics.h"

#define BITSTREAM_READER_LE
#include "avcodec.h"
#include "get_bits.h"
#include "internal.h"
#include "lsp.h"
#include "acelp_vectors.h"
#include "acelp_pitch_delay.h"
#include "acelp_filters.h"
#include "celp_filters.h"

#define MAX_SUBFRAME_COUNT   5

#include "sipr.h"
#include "siprdata.h"

typedef struct SiprModeParam {
    const char *mode_name;
    uint16_t bits_per_frame;
    uint8_t subframe_count;
    uint8_t frames_per_packet;
    float pitch_sharp_factor;

    /* bitstream parameters */
    uint8_t number_of_fc_indexes;
    uint8_t ma_predictor_bits;  ///< size in bits of the switched MA predictor

    /** size in bits of the i-th stage vector of quantizer */
    uint8_t vq_indexes_bits[5];

    /** size in bits of the adaptive-codebook index for every subframe */
    uint8_t pitch_delay_bits[5];

    uint8_t gp_index_bits;
    uint8_t fc_index_bits[10]; ///< size in bits of the fixed codebook indexes
    uint8_t gc_index_bits;     ///< size in bits of the gain  codebook indexes
} SiprModeParam;

static const SiprModeParam modes[MODE_COUNT] = {
    [MODE_16k] = {
        .mode_name          = "16k",
        .bits_per_frame     = 160,
        .subframe_count     = SUBFRAME_COUNT_16k,
        .frames_per_packet  = 1,
        .pitch_sharp_factor = 0.00,

        .number_of_fc_indexes = 10,
        .ma_predictor_bits    = 1,
        .vq_indexes_bits      = {7, 8, 7, 7, 7},
        .pitch_delay_bits     = {9, 6},
        .gp_index_bits        = 4,
        .fc_index_bits        = {4, 5, 4, 5, 4, 5, 4, 5, 4, 5},
        .gc_index_bits        = 5
    },

    [MODE_8k5] = {
        .mode_name          = "8k5",
        .bits_per_frame     = 152,
        .subframe_count     = 3,
        .frames_per_packet  = 1,
        .pitch_sharp_factor = 0.8,

        .number_of_fc_indexes = 3,
        .ma_predictor_bits    = 0,
        .vq_indexes_bits      = {6, 7, 7, 7, 5},
        .pitch_delay_bits     = {8, 5, 5},
        .gp_index_bits        = 0,
        .fc_index_bits        = {9, 9, 9},
        .gc_index_bits        = 7
    },

    [MODE_6k5] = {
        .mode_name          = "6k5",
        .bits_per_frame     = 232,
        .subframe_count     = 3,
        .frames_per_packet  = 2,
        .pitch_sharp_factor = 0.8,

        .number_of_fc_indexes = 3,
        .ma_predictor_bits    = 0,
        .vq_indexes_bits      = {6, 7, 7, 7, 5},
        .pitch_delay_bits     = {8, 5, 5},
        .gp_index_bits        = 0,
        .fc_index_bits        = {5, 5, 5},
        .gc_index_bits        = 7
    },

    [MODE_5k0] = {
        .mode_name          = "5k0",
        .bits_per_frame     = 296,
        .subframe_count     = 5,
        .frames_per_packet  = 2,
        .pitch_sharp_factor = 0.85,

        .number_of_fc_indexes = 1,
        .ma_predictor_bits    = 0,
        .vq_indexes_bits      = {6, 7, 7, 7, 5},
        .pitch_delay_bits     = {8, 5, 8, 5, 5},
        .gp_index_bits        = 0,
        .fc_index_bits        = {10},
        .gc_index_bits        = 7
    }
};

const float ff_pow_0_5[] = {
    1.0/(1 <<  1), 1.0/(1 <<  2), 1.0/(1 <<  3), 1.0/(1 <<  4),
    1.0/(1 <<  5), 1.0/(1 <<  6), 1.0/(1 <<  7), 1.0/(1 <<  8),
    1.0/(1 <<  9), 1.0/(1 << 10), 1.0/(1 << 11), 1.0/(1 << 12),
    1.0/(1 << 13), 1.0/(1 << 14), 1.0/(1 << 15), 1.0/(1 << 16)
};

{



                          const SiprParameters *parm)
{




    /* Note that a minimum distance is not enforced between the last value and
       the previous one, contrary to what is done in ff_acelp_reorder_lsf() */



/** Apply pitch lag to the fixed vector (AMR section 6.1.2). */
                             float *fixed_vector)
{
    int i;

}

/**
 * Extract decoding parameters from the input bitstream.
 * @param parms          parameters structure
 * @param pgb            pointer to initialized GetBitContext structure
 */
                              const SiprModeParam *p)
{





    }

                           int num_subfr)
{


    }

/**
 * Evaluate the adaptive impulse response.
 */
                    float pitch_sharp_factor)
{

    }

                                 LP_FILTER_ORDER);


/**
 * Evaluate the convolution of a vector with a sparse vector.
 */
                                  const float *shape, int length)
{


/**
 * Apply postfilter, very similar to AMR one.
 */
{


           LP_FILTER_ORDER*sizeof(float));

                                 LP_FILTER_ORDER);

           LP_FILTER_ORDER*sizeof(float));


           LP_FILTER_ORDER*sizeof(*pole_out));

           LP_FILTER_ORDER*sizeof(*pole_out));

                                      LP_FILTER_ORDER);


                                SiprMode mode, int low_gain)
{

    case MODE_6k5:
        }
    case MODE_8k5:


        }

    default:



            }
        } else {


        }
        break;
    }

                         float *out_data)
{
                                        // memory alignment








                              ff_b60_sinc, 6,
                              SUBFR_SIZE);



                              SUBFR_SIZE);

                                                          fixed_vector,
                     SUBFR_SIZE;


                                          34 - 15.0/(0.05*M_LN10/M_LN2),
                                          pred);

                                pitch_gain, gain_code, SUBFR_SIZE);





                                         pAz, excitation, SUBFR_SIZE,
                                         LP_FILTER_ORDER);
        }

                                     SUBFR_SIZE, LP_FILTER_ORDER);

    }

           LP_FILTER_ORDER * sizeof(float));

                                                        SUBFR_SIZE);
                                     SUBFR_SIZE, 0.9, &ctx->postfilter_agc);
        }

               LP_FILTER_ORDER*sizeof(float));
    }
           (PITCH_DELAY_MAX + L_INTERPOL) * sizeof(float));

                                             0.939805806,
                                             frame_size);

{

    default:
        if      (avctx->bit_rate > 12200) ctx->mode = MODE_16k;
        else if (avctx->bit_rate > 7500 ) ctx->mode = MODE_8k5;
        else if (avctx->bit_rate > 5750 ) ctx->mode = MODE_6k5;
        else                              ctx->mode = MODE_5k0;
        av_log(avctx, AV_LOG_WARNING,
               "Invalid block_align: %d. Mode %s guessed based on bitrate: %"PRId64"\n",
               avctx->block_align, modes[ctx->mode].mode_name, avctx->bit_rate);
    }


    } else {
    }




}

                             int *got_frame_ptr, AVPacket *avpkt)
{

        av_log(avctx, AV_LOG_ERROR,
               "Error processing packet: packet size (%d) too small\n",
               avpkt->size);
        return AVERROR_INVALIDDATA;
    }

    /* get output buffer */
        return ret;




    }


}

AVCodec ff_sipr_decoder = {
    .name           = "sipr",
    .long_name      = NULL_IF_CONFIG_SMALL("RealAudio SIPR / ACELP.NET"),
    .type           = AVMEDIA_TYPE_AUDIO,
    .id             = AV_CODEC_ID_SIPR,
    .priv_data_size = sizeof(SiprContext),
    .init           = sipr_decoder_init,
    .decode         = sipr_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
