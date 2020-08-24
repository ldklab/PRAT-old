/*
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

#include <stdint.h>
#include "libavutil/avutil.h"
#include "mathops.h"

#undef FUNC
#undef sum_type
#undef MUL
#undef CLIP
#undef FSUF

#define FUNC(n) AV_JOIN(n ## _, SAMPLE_SIZE)

#if SAMPLE_SIZE == 32
#   define sum_type  int64_t
#   define MUL(a, b) MUL64(a, b)
#   define CLIP(x) av_clipl_int32(x)
#else
#   define sum_type  int32_t
#   define MUL(a, b) ((a) * (b))
#   define CLIP(x) (x)
#endif

#define LPC1(x) {           \
    int c = coefs[(x)-1];   \
    p0   += MUL(c, s);      \
    s     = smp[i-(x)+1];   \
    p1   += MUL(c, s);      \
}

static av_always_inline void FUNC(lpc_encode_unrolled)(int32_t *res,
                                  const int32_t *smp, int len, int order,
                                  const int32_t *coefs, int shift, int big)
{
    int i;
            case 32: LPC1(32)
            case 31: LPC1(31)
            case 30: LPC1(30)
            case 29: LPC1(29)
            case 28: LPC1(28)
            case 27: LPC1(27)
            case 26: LPC1(26)
            case 25: LPC1(25)
            case 24: LPC1(24)
            case 23: LPC1(23)
            case 22: LPC1(22)
            case 21: LPC1(21)
            case 20: LPC1(20)
            case 19: LPC1(19)
            case 18: LPC1(18)
            case 17: LPC1(17)
            case 16: LPC1(16)
            case 15: LPC1(15)
            case 14: LPC1(14)
            case 13: LPC1(13)
            }
        } else {
            }
        }
    }
}

                                    int order, const int32_t *coefs, int shift)
{
#if CONFIG_SMALL
    for (i = order; i < len; i += 2) {
        int j;
        int s  = smp[i];
        sum_type p0 = 0, p1 = 0;
        for (j = 0; j < order; j++) {
            int c = coefs[j];
            p1   += MUL(c, s);
            s     = smp[i-j-1];
            p0   += MUL(c, s);
        }
        res[i  ] = smp[i  ] - CLIP(p0 >> shift);
        res[i+1] = smp[i+1] - CLIP(p1 >> shift);
    }
#else
    case  2: FUNC(lpc_encode_unrolled)(res, smp, len,     2, coefs, shift, 0); break;
    }
#endif

/* Comment for clarity/de-obfuscation.
 *
 * for (int i = order; i < len; i++) {
 *     int32_t p = 0;
 *     for (int j = 0; j < order; j++) {
 *         int c = coefs[j];
 *         int s = smp[(i-1)-j];
 *         p    += c*s;
 *     }
 *     res[i] = smp[i] - (p >> shift);
 * }
 *
 * The CONFIG_SMALL code above simplifies to this, in the case of SAMPLE_SIZE
 * not being equal to 32 (at the present time that means for 16-bit audio). The
 * code above does 2 samples per iteration.  Commit bfdd5bc (made all the way
 * back in 2007) says that way is faster.
 */
