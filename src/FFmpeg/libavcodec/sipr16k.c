/*
 * SIPR decoder for the 16k mode
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

#include "sipr.h"
#include "libavutil/attributes.h"
#include "libavutil/common.h"
#include "libavutil/float_dsp.h"
#include "libavutil/mathematics.h"
#include "lsp.h"
#include "acelp_vectors.h"
#include "acelp_pitch_delay.h"
#include "acelp_filters.h"
#include "celp_filters.h"

#include "sipr16kdata.h"

/**
 * Convert an lsf vector into an lsp vector.
 *
 * @param lsf               input lsf vector
 * @param lsp               output lsp vector
 */
{


{



                              const int* parm, int ma_pred)
{


    }


{
    } else
}

                          int pitch_lag_prev)
{
                                      pit_min, pit_max - 19);
    } else
        return 3 * pitch_lag_prev;
}

                       float* filt_mem[2], float* mem_preemph)
{


           LP_FILTER_ORDER_16k*sizeof(*buf));

                                 LP_FILTER_ORDER_16k);

           LP_FILTER_ORDER_16k * sizeof(*synth));

                                 LP_FILTER_ORDER_16k);

           LP_FILTER_ORDER_16k * sizeof(*synth));

                                 LP_FILTER_ORDER_16k);


           LP_FILTER_ORDER_16k * sizeof(*synth));


/**
 * Floating point version of ff_acelp_lp_decode().
 */
                             const double *lsp_2nd, const double *lsp_prev)
{

    /* LSP values for first subframe (3.2.5 of G.729, Equation 24) */


    /* LSP values for second subframe (3.2.5 of G.729) */

/**
 * Floating point version of ff_acelp_decode_gain_code().
 */
                                     float mr_energy, const float *quant_energy,
                                     const float *ma_prediction_coeff,
                                     int subframe_size, int ma_pred_order)
{
                                              ma_pred_order);

}

#define DIVIDE_BY_3(x) ((x) * 10923 >> 15)

                              float *out_data)
{



                      params->ma_pred_switch);





           LP_FILTER_ORDER_16k * sizeof(*synth));


        } else
                                            PITCH_MIN, PITCH_MAX,
                                            ctx->pitch_lag_prev);



                              sinc_win, 3, pitch_delay_frac + 1,
                              LP_FILTER_ORDER, L_SUBFR_16k);



                                   ff_fc_4pulses_8bits_tracks_13, 5, 4);


                                    19.0 - 15.0/(0.05*M_LN10/M_LN2),
                                    L_SUBFR_16k, 2);


                                fixed_vector, pitch_fac,
                                gain_code, L_SUBFR_16k);

                                     &excitation[i_subfr], L_SUBFR_16k,
                                     LP_FILTER_ORDER_16k);

    }
           LP_FILTER_ORDER_16k * sizeof(*synth));

            (L_INTERPOL+PITCH_MAX) * sizeof(float));



{



