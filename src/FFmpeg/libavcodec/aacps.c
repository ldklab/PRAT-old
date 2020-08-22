/*
 * MPEG-4 Parametric Stereo decoding functions
 * Copyright (c) 2010 Alex Converse <alex.converse@gmail.com>
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
 *
 * Note: Rounding-to-nearest used unless otherwise stated
 *
 */

#include <stdint.h>
#include "libavutil/common.h"
#include "libavutil/mathematics.h"
#include "avcodec.h"
#include "get_bits.h"
#include "aacps.h"
#if USE_FIXED
#include "aacps_fixed_tablegen.h"
#else
#include "libavutil/internal.h"
#include "aacps_tablegen.h"
#endif /* USE_FIXED */
#include "aacpsdata.c"

#define PS_BASELINE 0  ///< Operate in Baseline PS mode
                       ///< Baseline implies 10 or 20 stereo bands,
                       ///< mixing mode A, and no ipd/opd

#define numQMFSlots 32 //numTimeSlots * RATE

static const int8_t num_env_tab[2][4] = {
    { 0, 1, 2, 4, },
    { 1, 2, 3, 4, },
};

static const int8_t nr_iidicc_par_tab[] = {
    10, 20, 34, 10, 20, 34,
};

static const int8_t nr_iidopd_par_tab[] = {
     5, 11, 17,  5, 11, 17,
};

enum {
    huff_iid_df1,
    huff_iid_dt1,
    huff_iid_df0,
    huff_iid_dt0,
    huff_icc_df,
    huff_icc_dt,
    huff_ipd_df,
    huff_ipd_dt,
    huff_opd_df,
    huff_opd_dt,
};

static const int huff_iid[] = {
    huff_iid_df0,
    huff_iid_df1,
    huff_iid_dt0,
    huff_iid_dt1,
};

static VLC vlc_ps[10];

#define READ_PAR_DATA(PAR, OFFSET, MASK, ERR_CONDITION) \
/** \
 * Read Inter-channel Intensity Difference/Inter-Channel Coherence/ \
 * Inter-channel Phase Difference/Overall Phase Difference parameters from the \
 * bitstream. \
 * \
 * @param avctx contains the current codec context \
 * @param gb    pointer to the input bitstream \
 * @param ps    pointer to the Parametric Stereo context \
 * @param PAR   pointer to the parameter to be read \
 * @param e     envelope to decode \
 * @param dt    1: time delta-coded, 0: frequency delta-coded \
 */ \
static int read_ ## PAR ## _data(AVCodecContext *avctx, GetBitContext *gb, PSContext *ps, \
                        int8_t (*PAR)[PS_MAX_NR_IIDICC], int table_idx, int e, int dt) \
{ \
    int b, num = ps->nr_ ## PAR ## _par; \
    VLC_TYPE (*vlc_table)[2] = vlc_ps[table_idx].table; \
    if (dt) { \
        int e_prev = e ? e - 1 : ps->num_env_old - 1; \
        e_prev = FFMAX(e_prev, 0); \
        for (b = 0; b < num; b++) { \
            int val = PAR[e_prev][b] + get_vlc2(gb, vlc_table, 9, 3) - OFFSET; \
            if (MASK) val &= MASK; \
            PAR[e][b] = val; \
            if (ERR_CONDITION) \
                goto err; \
        } \
    } else { \
        int val = 0; \
        for (b = 0; b < num; b++) { \
            val += get_vlc2(gb, vlc_table, 9, 3) - OFFSET; \
            if (MASK) val &= MASK; \
            PAR[e][b] = val; \
            if (ERR_CONDITION) \
                goto err; \
        } \
    } \
    return 0; \
err: \
    av_log(avctx, AV_LOG_ERROR, "illegal "#PAR"\n"); \
    return AVERROR_INVALIDDATA; \
}

READ_PAR_DATA(ipdopd,                      0, 0x07, 0)

{

        return 0;

        }
    }
}

{
    }
}

{

                av_log(avctx, AV_LOG_ERROR, "iid_mode %d is reserved.\n",
                       iid_mode);
                goto err;
            }
        }
                av_log(avctx, AV_LOG_ERROR, "icc_mode %d is reserved.\n",
                       ps->icc_mode);
                goto err;
            }
        }
    }


                av_log(avctx, AV_LOG_ERROR, "border_position non monotone.\n");
                goto err;
            }
        }
    } else

                goto err;
        }
    } else

                goto err;
        }
    else

            cnt += get_bits(gb, 8);
        }
        }
            av_log(avctx, AV_LOG_ERROR, "ps extension overflow %d\n", cnt);
            goto err;
        }
    }


    //Fix up envelopes
        //Create a fake envelope
            }
            }
                memcpy(ps->ipd_par+ps->num_env, ps->ipd_par+source, sizeof(ps->ipd_par[0]));
                memcpy(ps->opd_par+ps->num_env, ps->opd_par+source, sizeof(ps->opd_par[0]));
            }
        }
                    av_log(avctx, AV_LOG_ERROR, "iid_par invalid\n");
                    goto err;
                }
            }
        }
                    av_log(avctx, AV_LOG_ERROR, "icc_par invalid\n");
                    goto err;
                }
            }
        }
    }



    //Baseline
    }


    }
    av_log(avctx, AV_LOG_ERROR, "Expected to read %d PS bits actually read %d.\n", bits_left, bits_consumed);
err:
    ps->start = 0;
    skip_bits_long(gb_host, bits_left);
    memset(ps->iid_par, 0, sizeof(ps->iid_par));
    memset(ps->icc_par, 0, sizeof(ps->icc_par));
    memset(ps->ipd_par, 0, sizeof(ps->ipd_par));
    memset(ps->opd_par, 0, sizeof(ps->opd_par));
    return bits_left;
}

/** Split one subband into 2 subsubbands with a symmetric real filter.
 * The filter must have its non-center even coefficients equal to zero. */
{
        }

#if USE_FIXED
        re_op = (re_op + 0x40000000) >> 31;
        im_op = (im_op + 0x40000000) >> 31;
#endif /* USE_FIXED */

    }

/** Split one subband into 6 subsubbands with a complex filter */
static void hybrid6_cx(PSDSPContext *dsp, INTFLOAT (*in)[2], INTFLOAT (*out)[32][2],
                       TABLE_CONST INTFLOAT (*filter)[8][2], int len)
{
    int i;
    int N = 8;
    LOCAL_ALIGNED_16(INTFLOAT, temp, [8], [2]);

    for (i = 0; i < len; i++, in++) {
        dsp->hybrid_analysis(temp, in, (const INTFLOAT (*)[8][2]) filter, 1, N);
        out[0][i][0] = temp[6][0];
        out[0][i][1] = temp[6][1];
        out[1][i][0] = temp[7][0];
        out[1][i][1] = temp[7][1];
        out[2][i][0] = temp[0][0];
        out[2][i][1] = temp[0][1];
        out[3][i][0] = temp[1][0];
        out[3][i][1] = temp[1][1];
        out[4][i][0] = temp[2][0] + temp[5][0];
        out[4][i][1] = temp[2][1] + temp[5][1];
        out[5][i][0] = temp[3][0] + temp[4][0];
        out[5][i][1] = temp[3][1] + temp[4][1];
    }
}

                            INTFLOAT (*in)[2], INTFLOAT (*out)[32][2],
                            TABLE_CONST INTFLOAT (*filter)[8][2], int N, int len)
{
    int i;

    }
}

                            INTFLOAT in[5][44][2], INTFLOAT L[2][38][64],
                            int is34, int len)
{
        }
    }
    } else {
    }
    //update in_buf
    }

static void hybrid_synthesis(PSDSPContext *dsp, INTFLOAT out[2][38][64],
                             INTFLOAT in[91][32][2], int is34, int len)
{
    int i, n;
    if (is34) {
        for (n = 0; n < len; n++) {
            memset(out[0][n], 0, 5*sizeof(out[0][n][0]));
            memset(out[1][n], 0, 5*sizeof(out[1][n][0]));
            for (i = 0; i < 12; i++) {
                out[0][n][0] += (UINTFLOAT)in[   i][n][0];
                out[1][n][0] += (UINTFLOAT)in[   i][n][1];
            }
            for (i = 0; i < 8; i++) {
                out[0][n][1] += (UINTFLOAT)in[12+i][n][0];
                out[1][n][1] += (UINTFLOAT)in[12+i][n][1];
            }
            for (i = 0; i < 4; i++) {
                out[0][n][2] += (UINTFLOAT)in[20+i][n][0];
                out[1][n][2] += (UINTFLOAT)in[20+i][n][1];
                out[0][n][3] += (UINTFLOAT)in[24+i][n][0];
                out[1][n][3] += (UINTFLOAT)in[24+i][n][1];
                out[0][n][4] += (UINTFLOAT)in[28+i][n][0];
                out[1][n][4] += (UINTFLOAT)in[28+i][n][1];
            }
        }
        dsp->hybrid_synthesis_deint(out, in + 27, 5, len);
    } else {
        for (n = 0; n < len; n++) {
            out[0][n][0] = (UINTFLOAT)in[0][n][0] + in[1][n][0] + in[2][n][0] +
                           (UINTFLOAT)in[3][n][0] + in[4][n][0] + in[5][n][0];
            out[1][n][0] = (UINTFLOAT)in[0][n][1] + in[1][n][1] + in[2][n][1] +
                           (UINTFLOAT)in[3][n][1] + in[4][n][1] + in[5][n][1];
            out[0][n][1] = (UINTFLOAT)in[6][n][0] + in[7][n][0];
            out[1][n][1] = (UINTFLOAT)in[6][n][1] + in[7][n][1];
            out[0][n][2] = (UINTFLOAT)in[8][n][0] + in[9][n][0];
            out[1][n][2] = (UINTFLOAT)in[8][n][1] + in[9][n][1];
        }
        dsp->hybrid_synthesis_deint(out, in + 7, 3, len);
    }
}

/// All-pass filter decay slope
#define DECAY_SLOPE      Q30(0.05f)
/// Number of frequency bands that can be addressed by the parameter index, b(k)
static const int   NR_PAR_BANDS[]      = { 20, 34 };
static const int   NR_IPDOPD_BANDS[]   = { 11, 17 };
/// Number of frequency bands that can be addressed by the sub subband index, k
static const int   NR_BANDS[]          = { 71, 91 };
/// Start frequency band for the all-pass filter decay slope
static const int   DECAY_CUTOFF[]      = { 10, 32 };
/// Number of all-pass filer bands
static const int   NR_ALLPASS_BANDS[]  = { 30, 50 };
/// First stereo band using the short one sample delay
static const int   SHORT_DELAY_BAND[]  = { 42, 62 };

/** Table 8.46 */
{
        b = 9;
    else {
        b = 4;
        par_mapped[10] = 0;
    }
    }
}

{
    }

{
#if USE_FIXED
    par[ 0] = (int)(((int64_t)(par[ 0] + (unsigned)(par[ 1]>>1)) * 1431655765 + \
                      0x40000000) >> 31);
    par[ 1] = (int)(((int64_t)((par[ 1]>>1) + (unsigned)par[ 2]) * 1431655765 + \
                      0x40000000) >> 31);
    par[ 2] = (int)(((int64_t)(par[ 3] + (unsigned)(par[ 4]>>1)) * 1431655765 + \
                      0x40000000) >> 31);
    par[ 3] = (int)(((int64_t)((par[ 4]>>1) + (unsigned)par[ 5]) * 1431655765 + \
                      0x40000000) >> 31);
#else
#endif /* USE_FIXED */
#if USE_FIXED
    par[18] = (((par[28]+2)>>2) + ((par[29]+2)>>2) + ((par[30]+2)>>2) + ((par[31]+2)>>2));
#else
#endif /* USE_FIXED */

{
    } else {
        par_mapped[16] =      0;
    }

static void map_idx_20_to_34(int8_t *par_mapped, const int8_t *par, int full)
{
    if (full) {
        par_mapped[33] =  par[19];
        par_mapped[32] =  par[19];
        par_mapped[31] =  par[18];
        par_mapped[30] =  par[18];
        par_mapped[29] =  par[18];
        par_mapped[28] =  par[18];
        par_mapped[27] =  par[17];
        par_mapped[26] =  par[17];
        par_mapped[25] =  par[16];
        par_mapped[24] =  par[16];
        par_mapped[23] =  par[15];
        par_mapped[22] =  par[15];
        par_mapped[21] =  par[14];
        par_mapped[20] =  par[14];
        par_mapped[19] =  par[13];
        par_mapped[18] =  par[12];
        par_mapped[17] =  par[11];
    }
    par_mapped[16] =  par[10];
    par_mapped[15] =  par[ 9];
    par_mapped[14] =  par[ 9];
    par_mapped[13] =  par[ 8];
    par_mapped[12] =  par[ 8];
    par_mapped[11] =  par[ 7];
    par_mapped[10] =  par[ 6];
    par_mapped[ 9] =  par[ 5];
    par_mapped[ 8] =  par[ 5];
    par_mapped[ 7] =  par[ 4];
    par_mapped[ 6] =  par[ 4];
    par_mapped[ 5] =  par[ 3];
    par_mapped[ 4] = (par[ 2] + par[ 3]) / 2;
    par_mapped[ 3] =  par[ 2];
    par_mapped[ 2] =  par[ 1];
    par_mapped[ 1] = (par[ 0] + par[ 1]) / 2;
    par_mapped[ 0] =  par[ 0];
}

{

{
#if !USE_FIXED
#endif /* USE_FIXED */


    }

    }

    //Transient detection
#if USE_FIXED
    for (i = 0; i < NR_PAR_BANDS[is34]; i++) {
        for (n = n0; n < nL; n++) {
            int decayed_peak;
            decayed_peak = (int)(((int64_t)peak_decay_factor * \
                                           peak_decay_nrg[i] + 0x40000000) >> 31);
            peak_decay_nrg[i] = FFMAX(decayed_peak, power[i][n]);
            power_smooth[i] += (power[i][n] + 2LL - power_smooth[i]) >> 2;
            peak_decay_diff_smooth[i] += (peak_decay_nrg[i] + 2LL - power[i][n] - \
                                          peak_decay_diff_smooth[i]) >> 2;

            if (peak_decay_diff_smooth[i]) {
                transient_gain[i][n] = FFMIN(power_smooth[i]*43691LL / peak_decay_diff_smooth[i], 1<<16);
            } else
                transient_gain[i][n] = 1 << 16;
        }
    }
#else
        }
    }

#endif /* USE_FIXED */
    //Decorrelation and transient reduction
    //                         PS_AP_LINKS - 1
    //                               -----
    //                                | |  Q_fract_allpass[k][m]*z^-link_delay[m] - a[m]*g_decay_slope[k]
    //H[k][z] = z^-2 * phi_fract[k] * | | ----------------------------------------------------------------
    //                                | | 1 - a[m]*g_decay_slope[k]*Q_fract_allpass[k][m]*z^-link_delay[m]
    //                               m = 0
    //d[k][z] (out) = transient_gain_mapped[k][z] * H[k][z] * s[k][z]
#if USE_FIXED
        int g_decay_slope;

        if (k - DECAY_CUTOFF[is34] <= 0) {
          g_decay_slope = 1 << 30;
        }
        else if (k - DECAY_CUTOFF[is34] >= 20) {
          g_decay_slope = 0;
        }
        else {
          g_decay_slope = (1 << 30) - DECAY_SLOPE * (k - DECAY_CUTOFF[is34]);
        }
#else
#endif /* USE_FIXED */
        }
    }
        //H = delay 14
    }
        //H = delay 1
    }

                    int8_t           (*par)[PS_MAX_NR_IIDICC],
                    int num_par, int num_env, int full)
{
        for (e = 0; e < num_env; e++) {
            map_idx_20_to_34(par_mapped[e], par[e], full);
        }
        }
    } else {
    }

                    int8_t           (*par)[PS_MAX_NR_IIDICC],
                    int num_par, int num_env, int full)
{
        }
        }
    } else {
    }

{


    //Remapping
    }

            remap34(&ipd_mapped, ps->ipd_par, ps->nr_ipdopd_par, ps->num_env, 0);
            remap34(&opd_mapped, ps->opd_par, ps->nr_ipdopd_par, ps->num_env, 0);
        }
        }
    } else {
        }
        }
    }

    //Mixing

                //The spec say says to only run this smoother when enable_ipdopd
                //is set but the reference decoder appears to run it constantly

            }
        }
#if USE_FIXED
            width = FFMIN(2U*width, INT_MAX);
#endif
            //Is this necessary? ps_04_new seems unchanged
            } else {
            }
            }
            //Interpolation
            }
                    h, h_step, stop - start);
        }
    }

{

        memset(ps->ap_delay + top, 0, (NR_ALLPASS_BANDS[is34] - top)*sizeof(ps->ap_delay[0]));


}

#define PS_INIT_VLC_STATIC(num, size) \
    INIT_VLC_STATIC(&vlc_ps[num], 9, ps_tmp[num].table_size / ps_tmp[num].elem_size,    \
                    ps_tmp[num].ps_bits, 1, 1,                                          \
                    ps_tmp[num].ps_codes, ps_tmp[num].elem_size, ps_tmp[num].elem_size, \
                    size);

#define PS_VLC_ROW(name) \
    { name ## _codes, name ## _bits, sizeof(name ## _codes), sizeof(name ## _codes[0]) }

    // Syntax initialization
        const void *ps_codes, *ps_bits;
        const unsigned int table_size, elem_size;
    } ps_tmp[] = {
        PS_VLC_ROW(huff_iid_df1),
        PS_VLC_ROW(huff_iid_dt1),
        PS_VLC_ROW(huff_iid_df0),
        PS_VLC_ROW(huff_iid_dt0),
        PS_VLC_ROW(huff_icc_df),
        PS_VLC_ROW(huff_icc_dt),
        PS_VLC_ROW(huff_ipd_df),
        PS_VLC_ROW(huff_ipd_dt),
        PS_VLC_ROW(huff_opd_df),
        PS_VLC_ROW(huff_opd_dt),
    };



{
