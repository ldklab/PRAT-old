/*
 * ATRAC1 compatible decoder
 * Copyright (c) 2009 Maxim Poliakovski
 * Copyright (c) 2009 Benjamin Larsson
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
 * ATRAC1 compatible decoder.
 * This decoder handles raw ATRAC1 data and probably SDDS data.
 */

/* Many thanks to Tim Craig for all the help! */

#include <math.h>
#include <stddef.h>
#include <stdio.h>

#include "libavutil/float_dsp.h"
#include "avcodec.h"
#include "get_bits.h"
#include "fft.h"
#include "internal.h"
#include "sinewin.h"

#include "atrac.h"
#include "atrac1data.h"

#define AT1_MAX_BFU      52                 ///< max number of block floating units in a sound unit
#define AT1_SU_SIZE      212                ///< number of bytes in a sound unit
#define AT1_SU_SAMPLES   512                ///< number of samples in a sound unit
#define AT1_FRAME_SIZE   AT1_SU_SIZE * 2
#define AT1_SU_MAX_BITS  AT1_SU_SIZE * 8
#define AT1_MAX_CHANNELS 2

#define AT1_QMF_BANDS    3
#define IDX_LOW_BAND     0
#define IDX_MID_BAND     1
#define IDX_HIGH_BAND    2

/**
 * Sound unit struct, one unit is used per channel
 */
typedef struct AT1SUCtx {
    int                 log2_block_count[AT1_QMF_BANDS];    ///< log2 number of blocks in a band
    int                 num_bfus;                           ///< number of Block Floating Units
    float*              spectrum[2];
    DECLARE_ALIGNED(32, float, spec1)[AT1_SU_SAMPLES];     ///< mdct buffer
    DECLARE_ALIGNED(32, float, spec2)[AT1_SU_SAMPLES];     ///< mdct buffer
    DECLARE_ALIGNED(32, float, fst_qmf_delay)[46];         ///< delay line for the 1st stacked QMF filter
    DECLARE_ALIGNED(32, float, snd_qmf_delay)[46];         ///< delay line for the 2nd stacked QMF filter
    DECLARE_ALIGNED(32, float, last_qmf_delay)[256+39];    ///< delay line for the last stacked QMF filter
} AT1SUCtx;

/**
 * The atrac1 context, holds all needed parameters for decoding
 */
typedef struct AT1Ctx {
    AT1SUCtx            SUs[AT1_MAX_CHANNELS];              ///< channel sound unit
    DECLARE_ALIGNED(32, float, spec)[AT1_SU_SAMPLES];      ///< the mdct spectrum buffer

    DECLARE_ALIGNED(32, float,  low)[256];
    DECLARE_ALIGNED(32, float,  mid)[256];
    DECLARE_ALIGNED(32, float, high)[512];
    float*              bands[3];
    FFTContext          mdct_ctx[3];
    AVFloatDSPContext   *fdsp;
} AT1Ctx;

/** size of the transform in samples in the long mode for each QMF band */
static const uint16_t samples_per_band[3] = {128, 128, 256};
static const uint8_t   mdct_long_nbits[3] = {7, 7, 8};


                      int rev_spec)
{

        int i;
    }


{



        /* number of mdct blocks in the current QMF band: 1 - for long mode */
        /* 4 for short mode(low/middle bands) and 8 for short mode(high band)*/

            /* mdct block size in samples: 128 (long mode, low & mid bands), */
            /* 256 (long mode, high band) and 32 (short mode, all bands) */

            /* calc transform size in bits according to the block_size_mode */

                return AVERROR_INVALIDDATA;
        } else {
            block_size = 32;
            nbits = 5;
        }


            /* overlap and window */

        }


    }

    /* Swap buffers so the mdct overlap works */

}

/**
 * Parse the block size mode byte
 */

{

        /* low and mid band */
            return AVERROR_INVALIDDATA;
    }

    /* high band */
        return AVERROR_INVALIDDATA;

}


                              float spec[AT1_SU_SAMPLES])
{

    /* parse the info byte (2nd byte) telling how much BFUs were coded */

    /* calc number of consumed bits:
        num_BFUs * (idwl(4bits) + idsf(6bits)) + log2_block_count(8bits) + info_byte(8bits)
        + info_byte_copy(8bits) + log2_block_count_copy(8bits) */

    /* get word length index (idwl) for each BFU */

    /* get scalefactor index (idsf) for each BFU */

    /* zero idwl/idsf for empty BFUs */

    /* read in the spectral data and reconstruct MDCT spectrum of this channel */


            /* check for bitstream overflow */
                return AVERROR_INVALIDDATA;

            /* get the position of the 1st spec according to the block size mode */


                    /* read in a quantized spec and convert it to
                     * signed int and then inverse quantization
                     */
                }
            } else { /* word_len = 0 -> empty BFU, zero all specs in the empty BFU */
            }
        }
    }

    return 0;
}


{

    /* combine low and middle bands */

    /* delay the signal of the high band by 39 samples */

    /* combine (low + middle) and high bands */


                               int *got_frame_ptr, AVPacket *avpkt)
{


    }

    /* get output buffer */
        return ret;



        /* parse block_size_mode, 1st byte */
            return ret;

            return ret;

            return ret;
    }


}


{



}


{


        av_log(avctx, AV_LOG_ERROR, "Unsupported number of channels: %d\n",
               avctx->channels);
        return AVERROR(EINVAL);
    }

        av_log(avctx, AV_LOG_ERROR, "Unsupported block align.");
        return AVERROR_PATCHWELCOME;
    }

    /* Init the mdct transforms */
        av_log(avctx, AV_LOG_ERROR, "Error initializing MDCT\n");
        atrac1_decode_end(avctx);
        return ret;
    }





    /* Prepare the mdct overlap buffers */

}


AVCodec ff_atrac1_decoder = {
    .name           = "atrac1",
    .long_name      = NULL_IF_CONFIG_SMALL("ATRAC1 (Adaptive TRansform Acoustic Coding)"),
    .type           = AVMEDIA_TYPE_AUDIO,
    .id             = AV_CODEC_ID_ATRAC1,
    .priv_data_size = sizeof(AT1Ctx),
    .init           = atrac1_decode_init,
    .close          = atrac1_decode_end,
    .decode         = atrac1_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
    .sample_fmts    = (const enum AVSampleFormat[]) { AV_SAMPLE_FMT_FLTP,
                                                      AV_SAMPLE_FMT_NONE },
};
