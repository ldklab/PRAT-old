/*
 * MDCT/IMDCT transforms
 * Copyright (c) 2002 Fabrice Bellard
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
#include <string.h>
#include "libavutil/common.h"
#include "libavutil/libm.h"
#include "libavutil/mathematics.h"
#include "fft.h"
#include "fft-internal.h"

/**
 * @file
 * MDCT/IMDCT transforms.
 */

#if FFT_FLOAT
#   define RSCALE(x, y) ((x) + (y))
#else
#if FFT_FIXED_32
#   define RSCALE(x, y) ((int)((x) + (unsigned)(y) + 32) >> 6)
#else /* FFT_FIXED_32 */
#   define RSCALE(x, y) ((int)((x) + (unsigned)(y)) >> 1)
#endif /* FFT_FIXED_32 */
#endif

/**
 * init MDCT or IMDCT computation.
 */
{


        goto fail;

        goto fail;

    case FF_MDCT_PERM_INTERLEAVE:
        s->tsin = s->tcos + 1;
        tstep = 2;
        break;
    default:
        goto fail;
    }

#if FFT_FIXED_32
#else
#endif
    }
    return 0;
 fail:
    ff_mdct_end(s);
    return -1;
}

/**
 * Compute the middle half of the inverse MDCT of size N = 2^nbits,
 * thus excluding the parts that can be derived by symmetry
 * @param output N/2 samples
 * @param input N/2 samples
 */
{


    /* pre rotation */
    }

    /* post rotation + reordering */
    }

/**
 * Compute inverse MDCT of size N = 2^nbits
 * @param output N samples
 * @param input N/2 samples
 */
{


    }

/**
 * Compute MDCT of size N = 2^nbits
 * @param input N samples
 * @param out N/2 samples
 */
{


    /* pre rotation */

    }


    /* post rotation */
    }

{
