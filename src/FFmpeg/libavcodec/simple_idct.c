/*
 * Simple IDCT
 *
 * Copyright (c) 2001 Michael Niedermayer <michaelni@gmx.at>
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
 * simpleidct in C.
 */

#include "libavutil/intreadwrite.h"
#include "avcodec.h"
#include "mathops.h"
#include "simple_idct.h"

#define IN_IDCT_DEPTH 16

#define BIT_DEPTH 8
#include "simple_idct_template.c"
#undef BIT_DEPTH

#define BIT_DEPTH 10
#include "simple_idct_template.c"

#define EXTRA_SHIFT  2
#include "simple_idct_template.c"

#undef EXTRA_SHIFT
#undef BIT_DEPTH

#define BIT_DEPTH 12
#include "simple_idct_template.c"
#undef BIT_DEPTH
#undef IN_IDCT_DEPTH

#define IN_IDCT_DEPTH 32
#define BIT_DEPTH 10
#include "simple_idct_template.c"
#undef BIT_DEPTH
#undef IN_IDCT_DEPTH

/* 2x4x8 idct */

#define CN_SHIFT 12
#define C_FIX(x) ((int)((x) * (1 << CN_SHIFT) + 0.5))
#define C1 C_FIX(0.6532814824)
#define C2 C_FIX(0.2705980501)

/* row idct is multiple by 16 * sqrt(2.0), col idct4 is normalized,
   and the butterfly must be multiplied by 0.5 * sqrt(2.0) */
#define C_SHIFT (4+1+12)

{


#define BF(k) \
{\
    int a0, a1;\
    a0 = ptr[k];\
    a1 = ptr[8 + k];\
    ptr[k] = a0 + a1;\
    ptr[8 + k] = a0 - a1;\
}

/* only used by DV codec. The input must be interlaced. 128 is added
   to the pixels before clamping to avoid systematic error
   (1024*sqrt(2)) offset would be needed otherwise. */
/* XXX: I think a 1.0/sqrt(2) normalization should be needed to
   compensate the extra butterfly stage - I don't have the full DV
   specification */
{

    /* butterfly */
    }

    /* IDCT8 on each line */
    }

    /* IDCT4 and store */
    }

/* 8x4 & 4x8 WMV2 IDCT */
#undef CN_SHIFT
#undef C_SHIFT
#undef C_FIX
#undef C1
#undef C2
#define CN_SHIFT 12
#define C_FIX(x) ((int)((x) * M_SQRT2 * (1 << CN_SHIFT) + 0.5))
#define C1 C_FIX(0.6532814824)
#define C2 C_FIX(0.2705980501)
#define C3 C_FIX(0.5)
#define C_SHIFT (4+1+12)
{


#define RN_SHIFT 15
#define R_FIX(x) ((int)((x) * M_SQRT2 * (1 << RN_SHIFT) + 0.5))
#define R1 R_FIX(0.6532814824)
#define R2 R_FIX(0.2705980501)
#define R3 R_FIX(0.5)
#define R_SHIFT 11
{


{

    /* IDCT8 on each line */
    }

    /* IDCT4 and store */
    }

{

    /* IDCT4 on each line */
    }

    /* IDCT8 and store */
    }

void ff_simple_idct44_add(uint8_t *dest, ptrdiff_t line_size, int16_t *block)
{
    int i;

    /* IDCT4 on each line */
    for(i=0; i<4; i++) {
        idct4row(block + i*8);
    }

    /* IDCT4 and store */
    for(i=0; i<4; i++){
        idct4col_add(dest + i, line_size, block + i);
    }
}

{



    }

{



    }
