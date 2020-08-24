/*
 * Xvid MPEG-4 IDCT
 *
 * Copyright (C) 2006-2011 Xvid Solutions GmbH
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
 * Walken IDCT
 * Alternative IDCT implementation for decoding compatibility.
 *
 * @author Skal
 * @note This C version is not the original IDCT, but a modified one that
 *       yields the same error profile as the MMX/MMXEXT/SSE2 versions.
 */

#include "config.h"
#include "libavutil/attributes.h"
#include "avcodec.h"
#include "idctdsp.h"
#include "xvididct.h"

#define ROW_SHIFT 11
#define COL_SHIFT  6

// #define FIX(x)   (int)((x) * (1 << ROW_SHIFT))
#define RND0 65536 // 1 << (COL_SHIFT + ROW_SHIFT - 1);
#define RND1 3597  // FIX (1.75683487303);
#define RND2 2260  // FIX (1.10355339059);
#define RND3 1203  // FIX (0.587788325588);
#define RND4 0
#define RND5 120   // FIX (0.058658283817);
#define RND6 512   // FIX (0.25);
#define RND7 512   // FIX (0.25);

static const int TAB04[] = { 22725, 21407, 19266, 16384, 12873,  8867, 4520 };
static const int TAB17[] = { 31521, 29692, 26722, 22725, 17855, 12299, 6270 };
static const int TAB26[] = { 29692, 27969, 25172, 21407, 16819, 11585, 5906 };
static const int TAB35[] = { 26722, 25172, 22654, 19266, 15137, 10426, 5315 };

{



        } else {
            } else
                return 0;
        }

    } else {


    }
    return 1;
}

#define TAN1  0x32EC
#define TAN2  0x6A0A
#define TAN3  0xAB0E
#define SQRT2 0x5A82

#define MULT(c, x, n)  ((unsigned)((int)((c) * (unsigned)(x)) >> (n)))
// 12b version => #define MULT(c,x, n)  ((((c) >> 3) * (x)) >> ((n) - 3))
// 12b zero-testing version:

#define BUTTERFLY(a, b, tmp)     \
    (tmp) = (a) + (b);           \
    (b)   = (a) - (b);           \
    (a)   = (tmp)

#define LOAD_BUTTERFLY(m1, m2, a, b, tmp, s)   \
    (m1) = (s)[(a)] + (s)[(b)];                \
    (m2) = (s)[(a)] - (s)[(b)]

{

    // odd



                                    // the pmulhw used in MMX/MMXEXT/SSE2 versions

    // even





{

    // odd




    // even




{

    // odd



    // even




{


    } else {
    }

static void xvid_idct_put(uint8_t *dest, ptrdiff_t line_size, int16_t *block)
{
    ff_xvid_idct(block);
    ff_put_pixels_clamped_c(block, dest, line_size);
}

static void xvid_idct_add(uint8_t *dest, ptrdiff_t line_size, int16_t *block)
{
    ff_xvid_idct(block);
    ff_add_pixels_clamped_c(block, dest, line_size);
}

{

          avctx->idct_algo == FF_IDCT_XVID))
        return;

    }

        ff_xvid_idct_init_mips(c, avctx, high_bit_depth);

}
