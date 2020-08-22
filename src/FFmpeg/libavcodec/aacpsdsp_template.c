/*
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

#include "config.h"
#include "libavutil/attributes.h"
#include "aacpsdsp.h"

{

                                 int n)
{
    }

                                 const INTFLOAT (*filter)[8][2],
                                 ptrdiff_t stride, int n)
{


        }
#if USE_FIXED
        out[i * stride][0] = (int)((sum_re + 0x40000000) >> 31);
        out[i * stride][1] = (int)((sum_im + 0x40000000) >> 31);
#else
#endif /* USE_FIXED */
    }

                                      int i, int len)
{

        }
    }

                                      INTFLOAT (*in)[32][2],
                                      int i, int len)
{

        }
    }

                             INTFLOAT (*ap_delay)[PS_QMF_TIME_SLOTS + PS_MAX_AP_DELAY][2],
                             const INTFLOAT phi_fract[2], const INTFLOAT (*Q_fract)[2],
                             const INTFLOAT *transient_gain,
                             INTFLOAT g_decay_slope,
                             int len)
{
                               Q31(0.56471812200776f),
                               Q31(0.48954165955695f) };


                    link_delay_im, fractional_delay_im);
                    link_delay_im, fractional_delay_re);
        }
    }

                                    INTFLOAT h[2][4], INTFLOAT h_step[2][4],
                                    int len)
{

        //l is s, r is d
    }

                                           INTFLOAT h[2][4], INTFLOAT h_step[2][4],
                                           int len)
{

        //l is s, r is d

    }

{

#if !USE_FIXED
        ff_psdsp_init_arm(s);
        ff_psdsp_init_aarch64(s);
        ff_psdsp_init_mips(s);
#endif /* !USE_FIXED */
