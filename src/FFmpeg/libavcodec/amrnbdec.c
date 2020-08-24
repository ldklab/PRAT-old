/*
 * AMR narrowband decoder
 * Copyright (c) 2006-2007 Robert Swain
 * Copyright (c) 2009 Colin McQuillan
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
 * AMR narrowband decoder
 *
 * This decoder uses floats for simplicity and so is not bit-exact. One
 * difference is that differences in phase can accumulate. The test sequences
 * in 3GPP TS 26.074 can still be useful.
 *
 * - Comparing this file's output to the output of the ref decoder gives a
 *   PSNR of 30 to 80. Plotting the output samples shows a difference in
 *   phase in some areas.
 *
 * - Comparing both decoders against their input, this decoder gives a similar
 *   PSNR. If the test sequence homing frames are removed (this decoder does
 *   not detect them), the PSNR is at least as good as the reference on 140
 *   out of 169 tests.
 */


#include <string.h>
#include <math.h>

#include "libavutil/channel_layout.h"
#include "libavutil/float_dsp.h"
#include "avcodec.h"
#include "libavutil/common.h"
#include "libavutil/avassert.h"
#include "celp_math.h"
#include "celp_filters.h"
#include "acelp_filters.h"
#include "acelp_vectors.h"
#include "acelp_pitch_delay.h"
#include "lsp.h"
#include "amr.h"
#include "internal.h"

#include "amrnbdata.h"

#define AMR_BLOCK_SIZE              160   ///< samples per frame
#define AMR_SAMPLE_BOUND        32768.0   ///< threshold for synthesis overflow

/**
 * Scale from constructed speech to [-1,1]
 *
 * AMR is designed to produce 16-bit PCM samples (3GPP TS 26.090 4.2) but
 * upscales by two (section 6.2.2).
 *
 * Fundamentally, this scale is determined by energy_mean through
 * the fixed vector contribution to the excitation vector.
 */
#define AMR_SAMPLE_SCALE  (2.0 / 32768.0)

/** Prediction factor for 12.2kbit/s mode */
#define PRED_FAC_MODE_12k2             0.65

#define LSF_R_FAC          (8000.0 / 32768.0) ///< LSF residual tables to Hertz
#define MIN_LSF_SPACING    (50.0488 / 8000.0) ///< Ensures stability of LPC filter
#define PITCH_LAG_MIN_MODE_12k2          18   ///< Lower bound on decoded lag search in 12.2kbit/s mode

/** Initial energy in dB. Also used for bad frames (unimplemented). */
#define MIN_ENERGY -14.0

/** Maximum sharpening factor
 *
 * The specification says 0.8, which should be 13107, but the reference C code
 * uses 13017 instead. (Amusingly the same applies to SHARP_MAX in g729dec.c.)
 */
#define SHARP_MAX 0.79449462890625

/** Number of impulse response coefficients used for tilt factor */
#define AMR_TILT_RESPONSE   22
/** Tilt factor = 1st reflection coefficient * gamma_t */
#define AMR_TILT_GAMMA_T   0.8
/** Adaptive gain control factor used in post-filter */
#define AMR_AGC_ALPHA      0.9

typedef struct AMRContext {
    AMRNBFrame                        frame; ///< decoded AMR parameters (lsf coefficients, codebook indexes, etc)
    uint8_t             bad_frame_indicator; ///< bad frame ? 1 : 0
    enum Mode                cur_frame_mode;

    int16_t     prev_lsf_r[LP_FILTER_ORDER]; ///< residual LSF vector from previous subframe
    double          lsp[4][LP_FILTER_ORDER]; ///< lsp vectors from current frame
    double   prev_lsp_sub4[LP_FILTER_ORDER]; ///< lsp vector for the 4th subframe of the previous frame

    float         lsf_q[4][LP_FILTER_ORDER]; ///< Interpolated LSF vector for fixed gain smoothing
    float          lsf_avg[LP_FILTER_ORDER]; ///< vector of averaged lsf vector

    float           lpc[4][LP_FILTER_ORDER]; ///< lpc coefficient vectors for 4 subframes

    uint8_t                   pitch_lag_int; ///< integer part of pitch lag from current subframe

    float excitation_buf[PITCH_DELAY_MAX + LP_FILTER_ORDER + 1 + AMR_SUBFRAME_SIZE]; ///< current excitation and all necessary excitation history
    float                       *excitation; ///< pointer to the current excitation vector in excitation_buf

    float   pitch_vector[AMR_SUBFRAME_SIZE]; ///< adaptive code book (pitch) vector
    float   fixed_vector[AMR_SUBFRAME_SIZE]; ///< algebraic codebook (fixed) vector (must be kept zero between frames)

    float               prediction_error[4]; ///< quantified prediction errors {20log10(^gamma_gc)} for previous four subframes
    float                     pitch_gain[5]; ///< quantified pitch gains for the current and previous four subframes
    float                     fixed_gain[5]; ///< quantified fixed gains for the current and previous four subframes

    float                              beta; ///< previous pitch_gain, bounded by [0.0,SHARP_MAX]
    uint8_t                      diff_count; ///< the number of subframes for which diff has been above 0.65
    uint8_t                      hang_count; ///< the number of subframes since a hangover period started

    float            prev_sparse_fixed_gain; ///< previous fixed gain; used by anti-sparseness processing to determine "onset"
    uint8_t               prev_ir_filter_nr; ///< previous impulse response filter "impNr": 0 - strong, 1 - medium, 2 - none
    uint8_t                 ir_filter_onset; ///< flag for impulse response filter strength

    float                postfilter_mem[10]; ///< previous intermediate values in the formant filter
    float                          tilt_mem; ///< previous input to tilt compensation filter
    float                    postfilter_agc; ///< previous factor used for adaptive gain control
    float                  high_pass_mem[2]; ///< previous intermediate values in the high-pass filter

    float samples_in[LP_FILTER_ORDER + AMR_SUBFRAME_SIZE]; ///< floating point samples

    ACELPFContext                     acelpf_ctx; ///< context for filters for ACELP-based codecs
    ACELPVContext                     acelpv_ctx; ///< context for vector operations for ACELP-based codecs
    CELPFContext                       celpf_ctx; ///< context for filters for CELP-based codecs
    CELPMContext                       celpm_ctx; ///< context for fixed point math operations

} AMRContext;

/** Double version of ff_weighted_vector_sumf() */
                                 const double *in_b, double weight_coeff_a,
                                 double weight_coeff_b, int length)
{

}

{

        avpriv_report_missing_feature(avctx, "multi-channel AMR");
        return AVERROR_PATCHWELCOME;
    }

        avctx->sample_rate = 8000;

    // p->excitation always points to the same position in p->excitation_buf

    }



}


/**
 * Unpack an RFC4867 speech frame into the AMR frame mode and parameters.
 *
 * The order of speech bits is specified by 3GPP TS 26.101.
 *
 * @param p the context
 * @param buf               pointer to the input buffer
 * @param buf_size          size of the input buffer
 *
 * @return the frame mode
 */
                                  int buf_size)
{

    // Decode the first octet.

        return NO_DATA;
    }


    return mode;
}


/// @name AMR pitch LPC coefficient decoding functions
/// @{

/**
 * Interpolate the LSF vector (used for fixed gain smoothing).
 * The interpolation is done over all four subframes even in MODE_12k2.
 *
 * @param[in]     ctx       The Context
 * @param[in,out] lsf_q     LSFs in [0,1] for each subframe
 * @param[in]     lsf_new   New LSFs in [0,1] for subframe 4
 */
static void interpolate_lsf(ACELPVContext *ctx, float lsf_q[4][LP_FILTER_ORDER], float *lsf_new)
{
    int i;

    for (i = 0; i < 4; i++)
        ctx->weighted_vector_sumf(lsf_q[i], lsf_q[3], lsf_new,
                                0.25 * (3 - i), 0.25 * (i + 1),
                                LP_FILTER_ORDER);
}

/**
 * Decode a set of 5 split-matrix quantized lsf indexes into an lsp vector.
 *
 * @param p the context
 * @param lsp output LSP vector
 * @param lsf_no_r LSF vector without the residual vector added
 * @param lsf_quantizer pointers to LSF dictionary tables
 * @param quantizer_offset offset in tables
 * @param sign for the 3 dictionary table
 * @param update store data for computing the next frame's LSFs
 */
                                 const float lsf_no_r[LP_FILTER_ORDER],
                                 const int16_t *lsf_quantizer[5],
                                 const int quantizer_offset,
                                 const int sign, const int update)
{

               2 * sizeof(*lsf_r));

    }






/**
 * Decode a set of 5 split-matrix quantized lsf indexes into 2 lsp vectors.
 *
 * @param p                 pointer to the AMRContext
 */
{




    // interpolate LSP vectors at subframes 1 and 3

/**
 * Decode a set of 3 split-matrix quantized lsf indexes into an lsp vector.
 *
 * @param p                 pointer to the AMRContext
 */
{




    // calculate mean-removed LSF vector and add mean


    // store data for computing the next frame's LSFs


    // interpolate LSP vectors at subframes 1, 2 and 3

/// @}


/// @name AMR pitch vector decoding functions
/// @{

/**
 * Like ff_decode_pitch_lag(), but with 1/6 resolution
 */
                                 const int prev_lag_int, const int subframe)
{
        } else {
        }
    } else {
                            PITCH_DELAY_MAX - 9);
    }

static void decode_pitch_vector(AMRContext *p,
                                const AMRNBSubframe *amr_subframe,
                                const int subframe)
{
    int pitch_lag_int, pitch_lag_frac;
    enum Mode mode = p->cur_frame_mode;

    if (p->cur_frame_mode == MODE_12k2) {
        decode_pitch_lag_1_6(&pitch_lag_int, &pitch_lag_frac,
                             amr_subframe->p_lag, p->pitch_lag_int,
                             subframe);
    } else {
        ff_decode_pitch_lag(&pitch_lag_int, &pitch_lag_frac,
                            amr_subframe->p_lag,
                            p->pitch_lag_int, subframe,
                            mode != MODE_4k75 && mode != MODE_5k15,
                            mode <= MODE_6k7 ? 4 : (mode == MODE_7k95 ? 5 : 6));
        pitch_lag_frac *= 2;
    }

    p->pitch_lag_int = pitch_lag_int; // store previous lag in a uint8_t

    pitch_lag_int += pitch_lag_frac > 0;

    /* Calculate the pitch vector by interpolating the past excitation at the
       pitch lag using a b60 hamming windowed sinc function.   */
    p->acelpf_ctx.acelp_interpolatef(p->excitation,
                          p->excitation + 1 - pitch_lag_int,
                          ff_b60_sinc, 6,
                          pitch_lag_frac + 6 - 6*(pitch_lag_frac > 0),
                          10, AMR_SUBFRAME_SIZE);

    memcpy(p->pitch_vector, p->excitation, AMR_SUBFRAME_SIZE * sizeof(float));
}

/// @}


/// @name AMR algebraic code book (fixed) vector decoding functions
/// @{

/**
 * Decode a 10-bit algebraic codebook index from a 10.2 kbit/s frame.
 */
                               int i1, int i2, int i3)
{
    // coded using 7+3 bits with the 3 LSBs being, individually, the LSB of 1 of
    // the 3 pulses and the upper 7 bits being coded in base 5
}

/**
 * Decode the algebraic codebook index to pulse positions and signs and
 * construct the algebraic codebook vector for MODE_10k2.
 *
 * @param fixed_index          positions of the eight pulses
 * @param fixed_sparse         pointer to the algebraic codebook vector
 */
                                   AMRFixed *fixed_sparse)
{


    // coded using 5+2 bits with the 2 LSBs being, individually, the LSB of 1 of
    // the 2 pulses and the upper 5 bits being coded in base 5

    }

/**
 * Decode the algebraic codebook index to pulse positions and signs,
 * then construct the algebraic codebook vector.
 *
 *                              nb of pulses | bits encoding pulses
 * For MODE_4k75 or MODE_5k15,             2 | 1-3, 4-6, 7
 *                  MODE_5k9,              2 | 1,   2-4, 5-6, 7-9
 *                  MODE_6k7,              3 | 1-3, 4,   5-7, 8,  9-11
 *      MODE_7k4 or MODE_7k95,             4 | 1-3, 4-6, 7-9, 10, 11-13
 *
 * @param fixed_sparse pointer to the algebraic codebook vector
 * @param pulses       algebraic codebook indexes
 * @param mode         mode of the current frame
 * @param subframe     current subframe number
 */
                                const enum Mode mode, const int subframe)
{

    } else {

        } else { // mode <= MODE_7k95
        }
    }

/**
 * Apply pitch lag to obtain the sharpened fixed vector (section 6.1.2)
 *
 * @param p the context
 * @param subframe unpacked amr subframe
 * @param mode mode of the current frame
 * @param fixed_sparse sparse representation of the fixed vector
 */
                             AMRFixed *fixed_sparse)
{
    // The spec suggests the current pitch gain is always used, but in other
    // modes the pitch and codebook gains are jointly quantized (sec 5.8.2)
    // so the codebook gain cannot depend on the quantized pitch gain.


    // Save pitch sharpening factor for the next subframe
    // MODE_4k75 only updates on the 2nd and 4th subframes - this follows from
    // the fact that the gains for two subframes are jointly quantized.
}
/// @}


/// @name AMR gain decoding functions
/// @{

/**
 * fixed gain smoothing
 * Note that where the spec specifies the "spectrum in the q domain"
 * in section 6.1.4, in fact frequencies should be used.
 *
 * @param p the context
 * @param lsf LSFs for the current subframe, in the range [0,1]
 * @param lsf_avg averaged LSFs
 * @param mode mode of the current frame
 *
 * @return fixed gain smoothed
 */
                               const float *lsf_avg, const enum Mode mode)
{


    // If diff is large for ten subframes, disable smoothing for a 40-subframe
    // hangover period.

    }

    }
}

/**
 * Decode pitch gain and fixed gain factor (part of section 6.1.3).
 *
 * @param p the context
 * @param amr_subframe unpacked amr subframe
 * @param mode mode of the current frame
 * @param subframe current subframe number
 * @param fixed_gain_factor decoded gain correction factor
 */
static void decode_gains(AMRContext *p, const AMRNBSubframe *amr_subframe,
                         const enum Mode mode, const int subframe,
                         float *fixed_gain_factor)
{
    if (mode == MODE_12k2 || mode == MODE_7k95) {
        p->pitch_gain[4]   = qua_gain_pit [amr_subframe->p_gain    ]
            * (1.0 / 16384.0);
        *fixed_gain_factor = qua_gain_code[amr_subframe->fixed_gain]
            * (1.0 /  2048.0);
    } else {
        const uint16_t *gains;

        if (mode >= MODE_6k7) {
            gains = gains_high[amr_subframe->p_gain];
        } else if (mode >= MODE_5k15) {
            gains = gains_low [amr_subframe->p_gain];
        } else {
            // gain index is only coded in subframes 0,2 for MODE_4k75
            gains = gains_MODE_4k75[(p->frame.subframe[subframe & 2].p_gain << 1) + (subframe & 1)];
        }

        p->pitch_gain[4]   = gains[0] * (1.0 / 16384.0);
        *fixed_gain_factor = gains[1] * (1.0 /  4096.0);
    }
}

/// @}


/// @name AMR preprocessing functions
/// @{

/**
 * Circularly convolve a sparse fixed vector with a phase dispersion impulse
 * response filter (D.6.2 of G.729 and 6.1.5 of AMR).
 *
 * @param out vector with filter applied
 * @param in source vector
 * @param filter phase filter coefficients
 *
 *  out[n] = sum(i,0,len-1){ in[i] * filter[(len + n - i)%len] }
 */
                            const float *filter)
{
          filter2[AMR_SUBFRAME_SIZE];

                          AMR_SUBFRAME_SIZE);

            ff_celp_circ_addf(filter2, filter, filter1, lag, fac,
                              AMR_SUBFRAME_SIZE);
    }


            filterp = filter;
            filterp = filter1;
        } else
            filterp = filter2;

    }

/**
 * Reduce fixed vector sparseness by smoothing with one of three IR filters.
 * Also know as "adaptive phase dispersion".
 *
 * This implements 3GPP TS 26.090 section 6.1(5).
 *
 * @param p the context
 * @param fixed_sparse algebraic codebook vector
 * @param fixed_vector unfiltered fixed vector
 * @param fixed_gain smoothed gain
 * @param out space for modified vector if necessary
 */
                                    const float *fixed_vector,
                                    float fixed_gain, float *out)
{

        ir_filter_nr = 0;      // strong filtering
        ir_filter_nr = 1;      // medium filtering
    } else

    // detect 'onset'

        int i, count = 0;



    // Disable filtering for very low level of fixed_gain.
    // Note this step is not specified in the technical description but is in
    // the reference source in the function Ph_disp.

                        (p->cur_frame_mode == MODE_7k95 ?
    }

    // update ir filter strength history

}

/// @}


/// @name AMR synthesis functions
/// @{

/**
 * Conduct 10th order linear predictive coding synthesis.
 *
 * @param p             pointer to the AMRContext
 * @param lpc           pointer to the LPC coefficients
 * @param fixed_gain    fixed codebook gain for synthesis
 * @param fixed_vector  algebraic codebook vector
 * @param samples       pointer to the output speech samples
 * @param overflow      16-bit overflow flag
 */
                     float fixed_gain, const float *fixed_vector,
                     float *samples, uint8_t overflow)
{

    // if an overflow has been detected, the pitch vector is scaled down by a
    // factor of 4
        for (i = 0; i < AMR_SUBFRAME_SIZE; i++)
            p->pitch_vector[i] *= 0.25;

                            p->pitch_gain[4], fixed_gain, AMR_SUBFRAME_SIZE);

    // emphasize pitch vector contribution
                                                    AMR_SUBFRAME_SIZE);


                                                AMR_SUBFRAME_SIZE);
    }

                                 AMR_SUBFRAME_SIZE,
                                 LP_FILTER_ORDER);

    // detect overflow
            return 1;
        }

    return 0;
}

/// @}


/// @name AMR update functions
/// @{

/**
 * Update buffers and history at the end of decoding a subframe.
 *
 * @param p             pointer to the AMRContext
 */
{

            (PITCH_DELAY_MAX + LP_FILTER_ORDER + 1) * sizeof(float));


            LP_FILTER_ORDER * sizeof(float));

/// @}


/// @name AMR Postprocessing functions
/// @{

/**
 * Get the tilt factor of a formant filter from its transfer function
 *
 * @param p     The Context
 * @param lpc_n LP_FILTER_ORDER coefficients of the numerator
 * @param lpc_d LP_FILTER_ORDER coefficients of the denominator
 */
static float tilt_factor(AMRContext *p, float *lpc_n, float *lpc_d)
{
    float rh0, rh1; // autocorrelation at lag 0 and 1

    // LP_FILTER_ORDER prior zeros are needed for ff_celp_lp_synthesis_filterf
    float impulse_buffer[LP_FILTER_ORDER + AMR_TILT_RESPONSE] = { 0 };
    float *hf = impulse_buffer + LP_FILTER_ORDER; // start of impulse response

    hf[0] = 1.0;
    memcpy(hf + 1, lpc_n, sizeof(float) * LP_FILTER_ORDER);
    p->celpf_ctx.celp_lp_synthesis_filterf(hf, lpc_d, hf,
                                 AMR_TILT_RESPONSE,
                                 LP_FILTER_ORDER);

    rh0 = p->celpm_ctx.dot_productf(hf, hf,     AMR_TILT_RESPONSE);
    rh1 = p->celpm_ctx.dot_productf(hf, hf + 1, AMR_TILT_RESPONSE - 1);

    // The spec only specifies this check for 12.2 and 10.2 kbit/s
    // modes. But in the ref source the tilt is always non-negative.
    return rh1 >= 0.0 ? rh1 / rh0 * AMR_TILT_GAMMA_T : 0.0;
}

/**
 * Perform adaptive post-filtering to enhance the quality of the speech.
 * See section 6.2.1.
 *
 * @param p             pointer to the AMRContext
 * @param lpc           interpolated LP coefficients for this subframe
 * @param buf_out       output of the filter
 */
{

                                                           AMR_SUBFRAME_SIZE);


        gamma_n = ff_pow_0_7;
        gamma_d = ff_pow_0_75;
    } else {
    }

    }

                                 AMR_SUBFRAME_SIZE, LP_FILTER_ORDER);
           sizeof(float) * LP_FILTER_ORDER);

                                      pole_out + LP_FILTER_ORDER,
                                      AMR_SUBFRAME_SIZE, LP_FILTER_ORDER);

                         AMR_SUBFRAME_SIZE);

                             AMR_AGC_ALPHA, &p->postfilter_agc);

/// @}

                              int *got_frame_ptr, AVPacket *avpkt)
{


    /* get output buffer */
        return ret;

        av_log(avctx, AV_LOG_ERROR, "Corrupt bitstream\n");
        return AVERROR_INVALIDDATA;
    }
        avpriv_report_missing_feature(avctx, "dtx mode");
        av_log(avctx, AV_LOG_INFO, "Note: libopencore_amrnb supports dtx\n");
        return AVERROR_PATCHWELCOME;
    }

    } else




                            p->cur_frame_mode, subframe);

        // The fixed gain (section 6.1.3) depends on the fixed vector
        // (section 6.1.2), but the fixed vector calculation uses
        // pitch sharpening based on the on the pitch gain (section 6.1.3).
        // So the correct order is: pitch gain, pitch sharpening, fixed gain.
                     &fixed_gain_factor);


            av_log(avctx, AV_LOG_ERROR, "The file is corrupted, pitch_lag = 0 is not allowed\n");
            return AVERROR_INVALIDDATA;
        }
                            AMR_SUBFRAME_SIZE);

                                                               p->fixed_vector,
                                                               AMR_SUBFRAME_SIZE) /
                                  AMR_SUBFRAME_SIZE,

        // The excitation feedback is calculated without any processing such
        // as fixed gain smoothing. This isn't mentioned in the specification.
                            AMR_SUBFRAME_SIZE);

        // In the ref decoder, excitation is stored with no fractional bits.
        // This step prevents buzz in silent periods. The ref encoder can
        // emit long sequences with pitch factor greater than one. This
        // creates unwanted feedback if the excitation vector is nonzero.
        // (e.g. test sequence T19_795.COD in 3GPP TS 26.074)

        // Smooth fixed gain.
        // The specification is ambiguous, but in the reference source, the
        // smoothed value is NOT fed back into later fixed gain smoothing.

                                             synth_fixed_gain, spare_vector);

                      synth_fixed_vector, &p->samples_in[LP_FILTER_ORDER], 0))
            // overflow detected -> rerun synthesis scaling pitch vector down
            // by a factor of 4, skipping pitch vector contribution emphasis
            // and adaptive gain control
            synthesis(p, p->lpc[subframe], synth_fixed_gain,
                      synth_fixed_vector, &p->samples_in[LP_FILTER_ORDER], 1);


        // update buffers and history
    }

                                             buf_out, highpass_zeros,
                                             highpass_poles,
                                             highpass_gain * AMR_SAMPLE_SCALE,

    /* Update averaged lsf vector (used for fixed gain smoothing).
     *
     * Note that lsf_avg should not incorporate the current frame's LSFs
     * for fixed_gain_smooth.
     * The specification has an incorrect formula: the reference decoder uses
     * qbar(n-1) rather than qbar(n) in section 6.1(4) equation 71. */
                            0.84, 0.16, LP_FILTER_ORDER);


    /* return the amount of bytes consumed if everything was OK */
}


AVCodec ff_amrnb_decoder = {
    .name           = "amrnb",
    .long_name      = NULL_IF_CONFIG_SMALL("AMR-NB (Adaptive Multi-Rate NarrowBand)"),
    .type           = AVMEDIA_TYPE_AUDIO,
    .id             = AV_CODEC_ID_AMR_NB,
    .priv_data_size = sizeof(AMRContext),
    .init           = amrnb_decode_init,
    .decode         = amrnb_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
    .sample_fmts    = (const enum AVSampleFormat[]){ AV_SAMPLE_FMT_FLT,
                                                     AV_SAMPLE_FMT_NONE },
};
