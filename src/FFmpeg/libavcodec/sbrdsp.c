/*
 * AAC Spectral Band Replication decoding functions
 * Copyright (c) 2008-2009 Robert Swain ( rob opendot cl )
 * Copyright (c) 2009-2010 Alex Converse <alex.converse@gmail.com>
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

#define USE_FIXED 0

#include "aac.h"
#include "config.h"
#include "libavutil/attributes.h"
#include "libavutil/intfloat.h"
#include "sbrdsp.h"

{

    {
    }

}

{
    }

{
    }


{
    }

{
    }

#if 0
    /* This code is slower because it multiplies memory accesses.
     * It is left for educational purposes and because it may offer
     * a better reference for writing arch-specific DSP functions. */
static av_always_inline void autocorrelate(const float x[40][2],
                                           float phi[3][2][2], int lag)
{
    int i;
    float real_sum = 0.0f;
    float imag_sum = 0.0f;
    if (lag) {
        for (i = 1; i < 38; i++) {
            real_sum += x[i][0] * x[i+lag][0] + x[i][1] * x[i+lag][1];
            imag_sum += x[i][0] * x[i+lag][1] - x[i][1] * x[i+lag][0];
        }
        phi[2-lag][1][0] = real_sum + x[ 0][0] * x[lag][0] + x[ 0][1] * x[lag][1];
        phi[2-lag][1][1] = imag_sum + x[ 0][0] * x[lag][1] - x[ 0][1] * x[lag][0];
        if (lag == 1) {
            phi[0][0][0] = real_sum + x[38][0] * x[39][0] + x[38][1] * x[39][1];
            phi[0][0][1] = imag_sum + x[38][0] * x[39][1] - x[38][1] * x[39][0];
        }
    } else {
        for (i = 1; i < 38; i++) {
            real_sum += x[i][0] * x[i][0] + x[i][1] * x[i][1];
        }
        phi[2][1][0] = real_sum + x[ 0][0] * x[ 0][0] + x[ 0][1] * x[ 0][1];
        phi[1][0][0] = real_sum + x[38][0] * x[38][0] + x[38][1] * x[38][1];
    }
}

static void sbr_autocorrelate_c(const float x[40][2], float phi[3][2][2])
{
    autocorrelate(x, phi, 0);
    autocorrelate(x, phi, 1);
    autocorrelate(x, phi, 2);
}
#else
{
    }
#endif

                         const float alpha0[2], const float alpha1[2],
                         float bw, int start, int end)
{


    }

                            const float *g_filt, int m_max, intptr_t ixh)
{

    }

                                                const float *s_m,
                                                const float *q_filt,
                                                int noise,
                                                float phi_sign0,
                                                float phi_sign1,
                                                int m_max)
{

        } else {
            y0 += q_filt[m] * ff_sbr_noise_table[noise][0];
            y1 += q_filt[m] * ff_sbr_noise_table[noise][1];
        }
    }
}

#include "sbrdsp_template.c"
