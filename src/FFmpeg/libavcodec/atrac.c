/*
 * common functions for the ATRAC family of decoders
 *
 * Copyright (c) 2006-2013 Maxim Poliakovski
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
 */

#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "avcodec.h"
#include "atrac.h"

float ff_atrac_sf_table[64];
static float qmf_window[48];

static const float qmf_48tap_half[24] = {
   -0.00001461907, -0.00009205479,-0.000056157569,0.00030117269,
    0.0002422519,  -0.00085293897,-0.0005205574,  0.0020340169,
    0.00078333891, -0.0042153862, -0.00075614988, 0.0078402944,
   -0.000061169922,-0.01344162,    0.0024626821,  0.021736089,
   -0.007801671,   -0.034090221,   0.01880949,    0.054326009,
   -0.043596379,   -0.099384367,   0.13207909,    0.46424159
};

{

    /* Generate scale factors */

    /* Generate the QMF window. */
        }

                                             int loc_scale)
{


    /* Generate gain level table. */

    /* Generate gain interpolation table. */

                                AtracGainInfo *gc_now, AtracGainInfo *gc_next,
                                int num_samples, float *out)
{


    } else {
        pos = 0;



            /* apply constant gain level and overlap */

            /* interpolate between two different gain levels */
            }
        }

    }

    /* copy the overlapping part into the delay buffer */

                   float *delayBuf, float *temp)
{



    /* loop1 */
    }

    /* loop2 */
        float s1 = 0.0;
        float s2 = 0.0;

        }


    }

    /* Update the delay buffer. */
