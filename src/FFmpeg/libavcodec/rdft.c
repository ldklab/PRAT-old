/*
 * (I)RDFT transforms
 * Copyright (c) 2009 Alex Converse <alex dot converse at gmail dot com>
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
#include <stdlib.h>
#include <math.h>
#include "libavutil/mathematics.h"
#include "rdft.h"

/**
 * @file
 * (Inverse) Real Discrete Fourier Transforms.
 */

/** Map one real FFT into two parallel real even and odd FFTs. Then interleave
 * the two real FFTs into one complex FFT. Unmangle the results.
 * ref: http://www.engineeringproductivitytools.com/stuff/T0001/PT10.HTM
 */
{

    }
    /* i=0 is a special case because of packing, the DC term is real, so we
       are going to throw the N/2 term (also real) in with it. */

#define RDFT_UNMANGLE(sign0, sign1)                                         \
    for (i = 1; i < (n>>2); i++) {                                          \
        i1 = 2*i;                                                           \
        i2 = n-i1;                                                          \
        /* Separate even and odd FFTs */                                    \
        ev.re =  k1*(data[i1  ]+data[i2  ]);                                \
        od.im =  k2*(data[i2  ]-data[i1  ]);                                \
        ev.im =  k1*(data[i1+1]-data[i2+1]);                                \
        od.re =  k2*(data[i1+1]+data[i2+1]);                                \
        /* Apply twiddle factors to the odd FFT and add to the even FFT */  \
        odsum.re = od.re*tcos[i] sign0 od.im*tsin[i];                       \
        odsum.im = od.im*tcos[i] sign1 od.re*tsin[i];                       \
        data[i1  ] =  ev.re + odsum.re;                                     \
        data[i1+1] =  ev.im + odsum.im;                                     \
        data[i2  ] =  ev.re - odsum.re;                                     \
        data[i2+1] =  odsum.im - ev.im;                                     \
    }

    } else {
    }

    }

{


        return AVERROR(EINVAL);

        return ret;



}

{
