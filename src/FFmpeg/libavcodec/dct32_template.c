/*
 * Template for the Discrete Cosine Transform for 32 samples
 * Copyright (c) 2001, 2002 Fabrice Bellard
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

#include "dct32.h"
#include "mathops.h"
#include "libavutil/internal.h"

#ifdef CHECKED
#define SUINT   int
#define SUINT32 int32_t
#else
#define SUINT   unsigned
#define SUINT32 uint32_t
#endif

#if DCT32_FLOAT
#   define dct32 ff_dct32_float
#   define FIXHR(x)       ((float)(x))
#   define MULH3(x, y, s) ((s)*(y)*(x))
#   define INTFLOAT float
#   define SUINTFLOAT float
#else
#   define dct32 ff_dct32_fixed
#   define FIXHR(a)       ((int)((a) * (1LL<<32) + 0.5))
#   define MULH3(x, y, s) MULH((s)*(x), y)
#   define INTFLOAT int
#   define SUINTFLOAT SUINT
#endif


/* tab[i][j] = 1.0 / (2.0 * cos(pi*(2*k+1) / 2^(6 - j))) */

/* cos(i*pi/64) */

#define COS0_0  FIXHR(0.50060299823519630134/2)
#define COS0_1  FIXHR(0.50547095989754365998/2)
#define COS0_2  FIXHR(0.51544730992262454697/2)
#define COS0_3  FIXHR(0.53104259108978417447/2)
#define COS0_4  FIXHR(0.55310389603444452782/2)
#define COS0_5  FIXHR(0.58293496820613387367/2)
#define COS0_6  FIXHR(0.62250412303566481615/2)
#define COS0_7  FIXHR(0.67480834145500574602/2)
#define COS0_8  FIXHR(0.74453627100229844977/2)
#define COS0_9  FIXHR(0.83934964541552703873/2)
#define COS0_10 FIXHR(0.97256823786196069369/2)
#define COS0_11 FIXHR(1.16943993343288495515/4)
#define COS0_12 FIXHR(1.48416461631416627724/4)
#define COS0_13 FIXHR(2.05778100995341155085/8)
#define COS0_14 FIXHR(3.40760841846871878570/8)
#define COS0_15 FIXHR(10.19000812354805681150/32)

#define COS1_0 FIXHR(0.50241928618815570551/2)
#define COS1_1 FIXHR(0.52249861493968888062/2)
#define COS1_2 FIXHR(0.56694403481635770368/2)
#define COS1_3 FIXHR(0.64682178335999012954/2)
#define COS1_4 FIXHR(0.78815462345125022473/2)
#define COS1_5 FIXHR(1.06067768599034747134/4)
#define COS1_6 FIXHR(1.72244709823833392782/4)
#define COS1_7 FIXHR(5.10114861868916385802/16)

#define COS2_0 FIXHR(0.50979557910415916894/2)
#define COS2_1 FIXHR(0.60134488693504528054/2)
#define COS2_2 FIXHR(0.89997622313641570463/2)
#define COS2_3 FIXHR(2.56291544774150617881/8)

#define COS3_0 FIXHR(0.54119610014619698439/2)
#define COS3_1 FIXHR(1.30656296487637652785/4)

#define COS4_0 FIXHR(M_SQRT1_2/2)

/* butterfly operator */
#define BF(a, b, c, s)\
{\
    tmp0 = val##a + val##b;\
    tmp1 = val##a - val##b;\
    val##a = tmp0;\
    val##b = MULH3(tmp1, c, 1<<(s));\
}

#define BF0(a, b, c, s)\
{\
    tmp0 = tab[a] + tab[b];\
    tmp1 = tab[a] - tab[b];\
    val##a = tmp0;\
    val##b = MULH3(tmp1, c, 1<<(s));\
}

#define BF1(a, b, c, d)\
{\
    BF(a, b, COS4_0, 1);\
    BF(c, d,-COS4_0, 1);\
    val##c += val##d;\
}

#define BF2(a, b, c, d)\
{\
    BF(a, b, COS4_0, 1);\
    BF(c, d,-COS4_0, 1);\
    val##c += val##d;\
    val##a += val##c;\
    val##c += val##b;\
    val##b += val##d;\
}

#define ADD(a, b) val##a += val##b

/* DCT32 without 1/sqrt(2) coef zero scaling. */
{

             val8 , val9 , val10, val11, val12, val13, val14, val15,
             val16, val17, val18, val19, val20, val21, val22, val23,
             val24, val25, val26, val27, val28, val29, val30, val31;

    /* pass 1 */
    /* pass 2 */
    /* pass 1 */
    /* pass 2 */
    /* pass 3 */
    /* pass 1 */
    /* pass 2 */
    /* pass 1 */
    /* pass 2 */
    /* pass 3 */
    /* pass 4 */



    /* pass 1 */
    /* pass 2 */
    /* pass 1 */
    /* pass 2 */
    /* pass 3 */

    /* pass 1 */
    /* pass 2 */
    /* pass 1 */
    /* pass 2 */
    /* pass 3 */
    /* pass 4 */

    /* pass 5 */

    /* pass 6 */




