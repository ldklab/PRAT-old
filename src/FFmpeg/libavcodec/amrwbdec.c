/*
 * AMR wideband decoder
 * Copyright (c) 2010 Marcelo Galvao Povoa
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
 * MERCHANTABILITY or FITNESS FOR A particular PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * AMR wideband decoder
 */

#include "libavutil/channel_layout.h"
#include "libavutil/common.h"
#include "libavutil/float_dsp.h"
#include "libavutil/lfg.h"

#include "avcodec.h"
#include "lsp.h"
#include "celp_filters.h"
#include "celp_math.h"
#include "acelp_filters.h"
#include "acelp_vectors.h"
#include "acelp_pitch_delay.h"
#include "internal.h"

#define AMR_USE_16BIT_TABLES
#include "amr.h"

#include "amrwbdata.h"
#include "mips/amrwbdec_mips.h"

typedef struct AMRWBContext {
    AMRWBFrame                             frame; ///< AMRWB parameters decoded from bitstream
    enum Mode                        fr_cur_mode; ///< mode index of current frame
    uint8_t                           fr_quality; ///< frame quality index (FQI)
    float                      isf_cur[LP_ORDER]; ///< working ISF vector from current frame
    float                   isf_q_past[LP_ORDER]; ///< quantized ISF vector of the previous frame
    float               isf_past_final[LP_ORDER]; ///< final processed ISF vector of the previous frame
    double                      isp[4][LP_ORDER]; ///< ISP vectors from current frame
    double               isp_sub4_past[LP_ORDER]; ///< ISP vector for the 4th subframe of the previous frame

    float                   lp_coef[4][LP_ORDER]; ///< Linear Prediction Coefficients from ISP vector

    uint8_t                       base_pitch_lag; ///< integer part of pitch lag for the next relative subframe
    uint8_t                        pitch_lag_int; ///< integer part of pitch lag of the previous subframe

    float excitation_buf[AMRWB_P_DELAY_MAX + LP_ORDER + 2 + AMRWB_SFR_SIZE]; ///< current excitation and all necessary excitation history
    float                            *excitation; ///< points to current excitation in excitation_buf[]

    float           pitch_vector[AMRWB_SFR_SIZE]; ///< adaptive codebook (pitch) vector for current subframe
    float           fixed_vector[AMRWB_SFR_SIZE]; ///< algebraic codebook (fixed) vector for current subframe

    float                    prediction_error[4]; ///< quantified prediction errors {20log10(^gamma_gc)} for previous four subframes
    float                          pitch_gain[6]; ///< quantified pitch gains for the current and previous five subframes
    float                          fixed_gain[2]; ///< quantified fixed gains for the current and previous subframes

    float                              tilt_coef; ///< {beta_1} related to the voicing of the previous subframe

    float                 prev_sparse_fixed_gain; ///< previous fixed gain; used by anti-sparseness to determine "onset"
    uint8_t                    prev_ir_filter_nr; ///< previous impulse response filter "impNr": 0 - strong, 1 - medium, 2 - none
    float                           prev_tr_gain; ///< previous initial gain used by noise enhancer for threshold

    float samples_az[LP_ORDER + AMRWB_SFR_SIZE];         ///< low-band samples and memory from synthesis at 12.8kHz
    float samples_up[UPS_MEM_SIZE + AMRWB_SFR_SIZE];     ///< low-band samples and memory processed for upsampling
    float samples_hb[LP_ORDER_16k + AMRWB_SFR_SIZE_16k]; ///< high-band samples and memory from synthesis at 16kHz

    float          hpf_31_mem[2], hpf_400_mem[2]; ///< previous values in the high pass filters
    float                           demph_mem[1]; ///< previous value in the de-emphasis filter
    float               bpf_6_7_mem[HB_FIR_SIZE]; ///< previous values in the high-band band pass filter
    float                 lpf_7_mem[HB_FIR_SIZE]; ///< previous values in the high-band low pass filter

    AVLFG                                   prng; ///< random number generator for white noise excitation
    uint8_t                          first_frame; ///< flag active during decoding of the first frame
    ACELPFContext                     acelpf_ctx; ///< context for filters for ACELP-based codecs
    ACELPVContext                     acelpv_ctx; ///< context for vector operations for ACELP-based codecs
    CELPFContext                       celpf_ctx; ///< context for filters for CELP-based codecs
    CELPMContext                       celpm_ctx; ///< context for fixed point math operations

} AMRWBContext;

{

        avpriv_report_missing_feature(avctx, "multi-channel AMR");
        return AVERROR_PATCHWELCOME;
    }

        avctx->sample_rate = 16000;






}

/**
 * Decode the frame header in the "MIME/storage" format. This format
 * is simpler and does not carry the auxiliary frame information.
 *
 * @param[in] ctx                  The Context
 * @param[in] buf                  Pointer to the input buffer
 *
 * @return The decoded header length in bytes
 */
{
    /* Decode frame header (1st octet) */

}

/**
 * Decode quantized ISF vectors using 36-bit indexes (6K60 mode only).
 *
 * @param[in]  ind                 Array of 5 indexes
 * @param[out] isf_q               Buffer for isf_q[LP_ORDER]
 */
{






/**
 * Decode quantized ISF vectors using 46-bit indexes (except 6K60 mode).
 *
 * @param[in]  ind                 Array of 7 indexes
 * @param[out] isf_q               Buffer for isf_q[LP_ORDER]
 */
{








/**
 * Apply mean and past ISF values using the prediction factor.
 * Updates past ISF vector.
 *
 * @param[in,out] isf_q            Current quantized ISF
 * @param[in,out] isf_past         Past quantized ISF
 */
{

    }
}

/**
 * Interpolate the fourth ISP vector from current and past frames
 * to obtain an ISP vector for each subframe.
 *
 * @param[in,out] isp_q            ISPs for each subframe
 * @param[in]     isp4_past        Past ISP for subframe 4
 */
{

    }

/**
 * Decode an adaptive codebook index into pitch lag (except 6k60, 8k85 modes).
 * Calculate integer lag and fractional lag always using 1/4 resolution.
 * In 1st and 3rd subframes the index is relative to last subframe integer lag.
 *
 * @param[out]    lag_int          Decoded integer pitch lag
 * @param[out]    lag_frac         Decoded fractional pitch lag
 * @param[in]     pitch_index      Adaptive codebook pitch index
 * @param[in,out] base_lag_int     Base integer lag used in relative subframes
 * @param[in]     subframe         Current subframe index (0 to 3)
 */
                                  uint8_t *base_lag_int, int subframe)
{
            /* the actual resolution is 1/2 but expressed as 1/4 */
        } else {
        }
        /* minimum lag for next subframe */
                                AMRWB_P_DELAY_MIN, AMRWB_P_DELAY_MAX - 15);
        // XXX: the spec states clearly that *base_lag_int should be
        // the nearest integer to *lag_int (minus 8), but the ref code
        // actually always uses its floor, I'm following the latter
    } else {
    }

/**
 * Decode an adaptive codebook index into pitch lag for 8k85 and 6k60 modes.
 * The description is analogous to decode_pitch_lag_high, but in 6k60 the
 * relative index is used for all subframes except the first.
 */
                                 uint8_t *base_lag_int, int subframe, enum Mode mode)
{
        } else {
        }
        // XXX: same problem as before
                                AMRWB_P_DELAY_MIN, AMRWB_P_DELAY_MAX - 15);
    } else {
    }

/**
 * Find the pitch vector by interpolating the past excitation at the
 * pitch delay, which is obtained in this function.
 *
 * @param[in,out] ctx              The context
 * @param[in]     amr_subframe     Current subframe data
 * @param[in]     subframe         Current subframe index (0 to 3)
 */
static void decode_pitch_vector(AMRWBContext *ctx,
                                const AMRWBSubFrame *amr_subframe,
                                const int subframe)
{
    int pitch_lag_int, pitch_lag_frac;
    int i;
    float *exc     = ctx->excitation;
    enum Mode mode = ctx->fr_cur_mode;

    if (mode <= MODE_8k85) {
        decode_pitch_lag_low(&pitch_lag_int, &pitch_lag_frac, amr_subframe->adap,
                              &ctx->base_pitch_lag, subframe, mode);
    } else
        decode_pitch_lag_high(&pitch_lag_int, &pitch_lag_frac, amr_subframe->adap,
                              &ctx->base_pitch_lag, subframe);

    ctx->pitch_lag_int = pitch_lag_int;
    pitch_lag_int += pitch_lag_frac > 0;

    /* Calculate the pitch vector by interpolating the past excitation at the
       pitch lag using a hamming windowed sinc function */
    ctx->acelpf_ctx.acelp_interpolatef(exc,
                          exc + 1 - pitch_lag_int,
                          ac_inter, 4,
                          pitch_lag_frac + (pitch_lag_frac > 0 ? 0 : 4),
                          LP_ORDER, AMRWB_SFR_SIZE + 1);

    /* Check which pitch signal path should be used
     * 6k60 and 8k85 modes have the ltp flag set to 0 */
    if (amr_subframe->ltp) {
        memcpy(ctx->pitch_vector, exc, AMRWB_SFR_SIZE * sizeof(float));
    } else {
        for (i = 0; i < AMRWB_SFR_SIZE; i++)
            ctx->pitch_vector[i] = 0.18 * exc[i - 1] + 0.64 * exc[i] +
                                   0.18 * exc[i + 1];
        memcpy(exc, ctx->pitch_vector, AMRWB_SFR_SIZE * sizeof(float));
    }
}

/** Get x bits in the index interval [lsb,lsb+len-1] inclusive */
#define BIT_STR(x,lsb,len) av_mod_uintp2((x) >> (lsb), (len))

/** Get the bit at specified position */
#define BIT_POS(x, p) (((x) >> (p)) & 1)

/**
 * The next six functions decode_[i]p_track decode exactly i pulses
 * positions and amplitudes (-1 or 1) in a subframe track using
 * an encoded pulse indexing (TS 26.190 section 5.8.2).
 *
 * The results are given in out[], in which a negative number means
 * amplitude -1 and vice versa (i.e., ampl(x) = x / abs(x) ).
 *
 * @param[out] out                 Output buffer (writes i elements)
 * @param[in]  code                Pulse index (no. of bits varies, see below)
 * @param[in]  m                   (log2) Number of potential positions
 * @param[in]  off                 Offset for decoded positions
 */
{


{


{

                    m - 1, off + half_2p);

{


                        m - 1, off + half_4p);
                        m - 1, off);
                        m - 1, off + b_offset);
                        m - 1, off);
                        m - 1, off + b_offset);
                        m - 1, off);
                        m - 1, off + b_offset);
        break;
    }

{

                    m - 1, off + half_3p);


{
    /* which half has more pulses in cases 0 to 2 */

                        m - 1, off + half_more);
                        m - 1, off + half_more);
                        m - 1, off + half_other);
                        m - 1, off + half_more);
                        m - 1, off + half_other);
                        m - 1, off + half_more);
                        m - 1, off);
                        m - 1, off + b_offset);
    }

/**
 * Decode the algebraic codebook index to pulse positions and signs,
 * then construct the algebraic codebook vector.
 *
 * @param[out] fixed_vector        Buffer for the fixed codebook excitation
 * @param[in]  pulse_hi            MSBs part of the pulse index array (higher modes only)
 * @param[in]  pulse_lo            LSBs part of the pulse index array
 * @param[in]  mode                Mode of the current frame
 */
                                const uint16_t *pulse_lo, const enum Mode mode)
{
    /* sig_pos stores for each track the decoded pulse position indexes
     * (1-based) multiplied by its corresponding amplitude (+1 or -1) */

    case MODE_6k60:
        break;
    case MODE_8k85:
        break;
    case MODE_12k65:
        break;
    case MODE_14k25:
        break;
    case MODE_15k85:
        break;
    case MODE_18k25:
        break;
    case MODE_19k85:
        break;
    case MODE_23k05:
    case MODE_23k85:
        break;
    }



        }

/**
 * Decode pitch gain and fixed gain correction factor.
 *
 * @param[in]  vq_gain             Vector-quantized index for gains
 * @param[in]  mode                Mode of the current frame
 * @param[out] fixed_gain_factor   Decoded fixed gain correction factor
 * @param[out] pitch_gain          Decoded pitch gain
 */
                         float *fixed_gain_factor, float *pitch_gain)
{
                                                qua_gain_7b[vq_gain]);

}

/**
 * Apply pitch sharpening filters to the fixed codebook vector.
 *
 * @param[in]     ctx              The context
 * @param[in,out] fixed_vector     Fixed codebook excitation
 */
// XXX: Spec states this procedure should be applied when the pitch
// lag is less than 64, but this checking seems absent in reference and AMR-NB
static void pitch_sharpening(AMRWBContext *ctx, float *fixed_vector)
{
    int i;

    /* Tilt part */
    for (i = AMRWB_SFR_SIZE - 1; i != 0; i--)
        fixed_vector[i] -= fixed_vector[i - 1] * ctx->tilt_coef;

    /* Periodicity enhancement part */
    for (i = ctx->pitch_lag_int; i < AMRWB_SFR_SIZE; i++)
        fixed_vector[i] += fixed_vector[i - ctx->pitch_lag_int] * 0.85;
}

/**
 * Calculate the voicing factor (-1.0 = unvoiced to 1.0 = voiced).
 *
 * @param[in] p_vector, f_vector   Pitch and fixed excitation vectors
 * @param[in] p_gain, f_gain       Pitch and fixed gains
 * @param[in] ctx                  The context
 */
// XXX: There is something wrong with the precision here! The magnitudes
// of the energies are not correct. Please check the reference code carefully
static float voice_factor(float *p_vector, float p_gain,
                          float *f_vector, float f_gain,
                          CELPMContext *ctx)
{
    double p_ener = (double) ctx->dot_productf(p_vector, p_vector,
                                                          AMRWB_SFR_SIZE) *
                    p_gain * p_gain;
    double f_ener = (double) ctx->dot_productf(f_vector, f_vector,
                                                          AMRWB_SFR_SIZE) *
                    f_gain * f_gain;

    return (p_ener - f_ener) / (p_ener + f_ener + 0.01);
}

/**
 * Reduce fixed vector sparseness by smoothing with one of three IR filters,
 * also known as "adaptive phase dispersion".
 *
 * @param[in]     ctx              The context
 * @param[in,out] fixed_vector     Unfiltered fixed vector
 * @param[out]    buf              Space for modified vector if necessary
 *
 * @return The potentially overwritten filtered fixed vector address
 */
                              float *fixed_vector, float *buf)
{

        return fixed_vector;

        ir_filter_nr = 0;      // strong filtering
        ir_filter_nr = 1;      // medium filtering
    } else

    /* detect 'onset' */
    } else {
        int i, count = 0;



    }

    /* update ir filter strength history */



        /* Circular convolution code in the reference
         * decoder was modified to avoid using one
         * extra array. The filtered vector is given by:
         *
         * c2(n) = sum(i,0,len-1){ c(i) * coef( (n - i + len) % len ) }
         */

                                  AMRWB_SFR_SIZE);
        fixed_vector = buf;
    }

    return fixed_vector;
}

/**
 * Calculate a stability factor {teta} based on distance between
 * current and past isf. A value of 1 shows maximum signal stability.
 */
{
    int i;
    float acc = 0.0;


    // XXX: This part is not so clear from the reference code
    // the result is more accurate changing the "/ 256" to "* 512"
}

/**
 * Apply a non-linear fixed gain smoothing in order to reduce
 * fluctuation in the energy of excitation.
 *
 * @param[in]     fixed_gain       Unsmoothed fixed gain
 * @param[in,out] prev_tr_gain     Previous threshold gain (updated)
 * @param[in]     voice_fac        Frame voicing factor
 * @param[in]     stab_fac         Frame stability factor
 *
 * @return The smoothed gain
 */
                            float voice_fac,  float stab_fac)
{

    // XXX: the following fixed-point constants used to in(de)crement
    // gain by 1.5dB were taken from the reference code, maybe it could
    // be simpler
                     (6226 * (1.0f / (1 << 15)))); // +1.5 dB
    } else
                    (27536 * (1.0f / (1 << 15)))); // -1.5 dB


}

/**
 * Filter the fixed_vector to emphasize the higher frequencies.
 *
 * @param[in,out] fixed_vector     Fixed codebook vector
 * @param[in]     voice_fac        Frame voicing factor
 */
{



    }


/**
 * Conduct 16th order linear predictive coding synthesis from excitation.
 *
 * @param[in]     ctx              Pointer to the AMRWBContext
 * @param[in]     lpc              Pointer to the LPC coefficients
 * @param[out]    excitation       Buffer for synthesis final excitation
 * @param[in]     fixed_gain       Fixed codebook gain for synthesis
 * @param[in]     fixed_vector     Algebraic codebook vector
 * @param[in,out] samples          Pointer to the output samples and memory
 */
                      float fixed_gain, const float *fixed_vector,
                      float *samples)
{
                            ctx->pitch_gain[0], fixed_gain, AMRWB_SFR_SIZE);

    /* emphasize pitch vector contribution in low bitrate modes */
                                                    AMRWB_SFR_SIZE);

        // XXX: Weird part in both ref code and spec. A unknown parameter
        // {beta} seems to be identical to the current pitch gain


                                                energy, AMRWB_SFR_SIZE);
    }

                                 AMRWB_SFR_SIZE, LP_ORDER);

/**
 * Apply to synthesis a de-emphasis filter of the form:
 * H(z) = 1 / (1 - m * z^-1)
 *
 * @param[out]    out              Output buffer
 * @param[in]     in               Input samples array with in[-1]
 * @param[in]     m                Filter coefficient
 * @param[in,out] mem              State from last filtering
 */
{



}

/**
 * Upsample a signal by 5/4 ratio (from 12.8kHz to 16kHz) using
 * a FIR interpolation filter. Uses past data from before *in address.
 *
 * @param[out] out                 Buffer for interpolated signal
 * @param[in]  in                  Current signal data (length 0.8*o_size)
 * @param[in]  o_size              Output signal length
 * @param[in] ctx                  The context
 */
static void upsample_5_4(float *out, const float *in, int o_size, CELPMContext *ctx)
{
    const float *in0 = in - UPS_FIR_SIZE + 1;
    int i, j, k;
    int int_part = 0, frac_part;

    i = 0;
    for (j = 0; j < o_size / 5; j++) {
        out[i] = in[int_part];
        frac_part = 4;
        i++;

        for (k = 1; k < 5; k++) {
            out[i] = ctx->dot_productf(in0 + int_part,
                                                  upsample_fir[4 - frac_part],
                                                  UPS_MEM_SIZE);
            int_part++;
            frac_part--;
            i++;
        }
    }
}

/**
 * Calculate the high-band gain based on encoded index (23k85 mode) or
 * on the low-band speech signal and the Voice Activity Detection flag.
 *
 * @param[in] ctx                  The context
 * @param[in] synth                LB speech synthesis at 12.8k
 * @param[in] hb_idx               Gain index for mode 23k85 only
 * @param[in] vad                  VAD flag for the frame
 */
                          uint16_t hb_idx, uint8_t vad)
{



    } else
        tilt = 0;

    /* return gain bounded by [0.1, 1.0] */
}

/**
 * Generate the high-band excitation with the same energy from the lower
 * one and scaled by the given gain.
 *
 * @param[in]  ctx                 The context
 * @param[out] hb_exc              Buffer for the excitation
 * @param[in]  synth_exc           Low-band excitation used for synthesis
 * @param[in]  hb_gain             Wanted excitation gain
 */
                                 const float *synth_exc, float hb_gain)
{
                                                AMRWB_SFR_SIZE);

    /* Generate a white-noise excitation */

                                            AMRWB_SFR_SIZE_16k);

/**
 * Calculate the auto-correlation for the ISF difference vector.
 */
{

    }
}

/**
 * Extrapolate a ISF vector to the 16kHz range (20th order LP)
 * used at mode 6k60 LP filter for the high frequency band.
 *
 * @param[out] isf Buffer for extrapolated isf; contains LP_ORDER
 *                 values on input
 */
{


    /* Calculate the difference vector */

    diff_mean = 0.0;

    /* Find which is the maximum autocorrelation */
    i_max_corr = 0;

    }


    /* Calculate an estimate for ISF(18) and scale ISF based on the error */


    /* Stability insurance */
            if (diff_isf[i] > diff_isf[i - 1]) {
                diff_isf[i - 1] = 5.0 - diff_isf[i];
            } else
                diff_isf[i] = 5.0 - diff_isf[i - 1];
        }


    /* Scale the ISF vector for 16000 Hz */

/**
 * Spectral expand the LP coefficients using the equation:
 *   y[i] = x[i] * (gamma ** i)
 *
 * @param[out] out                 Output buffer (may use input array)
 * @param[in]  lpc                 LP coefficients array
 * @param[in]  gamma               Weighting factor
 * @param[in]  size                LP array size
 */
{

    }
}

/**
 * Conduct 20th order linear predictive coding synthesis for the high
 * frequency band excitation at 16kHz.
 *
 * @param[in]     ctx              The context
 * @param[in]     subframe         Current subframe index (0 to 3)
 * @param[in,out] samples          Pointer to the output speech samples
 * @param[in]     exc              Generated white-noise scaled excitation
 * @param[in]     isf              Current frame isf vector
 * @param[in]     isf_past         Past frame final isf vector
 */
                         const float *exc, const float *isf, const float *isf_past)
{





    } else {
    }

                                 (mode == MODE_6k60) ? LP_ORDER_16k : LP_ORDER);

/**
 * Apply a 15th order filter to high-band samples.
 * The filter characteristic depends on the given coefficients.
 *
 * @param[out]    out              Buffer for filtered output
 * @param[in]     fir_coef         Filter coefficients
 * @param[in,out] mem              State from last filtering (updated)
 * @param[in]     in               Input speech data (high-band)
 *
 * @remark It is safe to pass the same array in in and out parameters
 */

#ifndef hb_fir_filter
                          float mem[HB_FIR_SIZE], const float *in)
{


    }

#endif /* hb_fir_filter */

/**
 * Update context state before the next subframe.
 */
{
            (AMRWB_P_DELAY_MAX + LP_ORDER + 1) * sizeof(float));


            LP_ORDER * sizeof(float));
            UPS_MEM_SIZE * sizeof(float));
            LP_ORDER_16k * sizeof(float));

                              int *got_frame_ptr, AVPacket *avpkt)
{

    /* get output buffer */
        return ret;


        av_log(avctx, AV_LOG_ERROR, "Encountered a bad or corrupted frame\n");

        /* The specification suggests a "random signal" and
           "a muting technique" to "gradually decrease the output level". */
        av_samples_set_silence(&frame->data[0], 0, frame->nb_samples, 1, AV_SAMPLE_FMT_FLT);
        *got_frame_ptr = 1;
        return expected_fr_size;
    }
        av_log(avctx, AV_LOG_ERROR,
               "Invalid mode %d\n", ctx->fr_cur_mode);
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR,
            "Frame too small (%d bytes). Truncated file?\n", buf_size);
        *got_frame_ptr = 0;
        return AVERROR_INVALIDDATA;
    }

        avpriv_request_sample(avctx, "SID mode");
        return AVERROR_PATCHWELCOME;
    }


    /* Decode the quantized ISF vector */
    } else {
    }




    /* Generate a ISP vector for each subframe */
    }



        /* Decode adaptive codebook (pitch vector) */
        /* Decode innovative codebook (fixed vector) */


                     &fixed_gain_factor, &ctx->pitch_gain[0]);

                                                               ctx->fixed_vector,
                                                               AMRWB_SFR_SIZE) /
                                  AMRWB_SFR_SIZE,
                       ENERGY_MEAN, energy_pred_fac);

        /* Calculate voice factor and store tilt for next subframe */
                                      ctx->fixed_vector, ctx->fixed_gain[0],
                                      &ctx->celpm_ctx);

        /* Construct current excitation */
        }

        /* Post-processing of excitation elements */
                                          voice_fac, stab_fac);

                                             spare_vector);


                  synth_fixed_vector, &ctx->samples_az[LP_ORDER]);

        /* Synthesis speech post-processing */

            &ctx->samples_up[UPS_MEM_SIZE], hpf_zeros, hpf_31_poles,

                     AMRWB_SFR_SIZE_16k, &ctx->celpm_ctx);

        /* High frequency band (6.4 - 7.0 kHz) generation part */
            &ctx->samples_up[UPS_MEM_SIZE], hpf_zeros, hpf_400_poles,



                     hb_exc, ctx->isf_cur, ctx->isf_past_final);

        /* High-band post-processing filters */
                      &ctx->samples_hb[LP_ORDER_16k]);

                          hb_samples);

        /* Add the low and high frequency bands */

        /* Update buffers and history */
    }

    /* update state for next frame */


}

AVCodec ff_amrwb_decoder = {
    .name           = "amrwb",
    .long_name      = NULL_IF_CONFIG_SMALL("AMR-WB (Adaptive Multi-Rate WideBand)"),
    .type           = AVMEDIA_TYPE_AUDIO,
    .id             = AV_CODEC_ID_AMR_WB,
    .priv_data_size = sizeof(AMRWBContext),
    .init           = amrwb_decode_init,
    .decode         = amrwb_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
    .sample_fmts    = (const enum AVSampleFormat[]){ AV_SAMPLE_FMT_FLT,
                                                     AV_SAMPLE_FMT_NONE },
};
