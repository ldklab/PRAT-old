/*
 * Lossless video DSP utils
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
#include "avcodec.h"
#include "lossless_videodsp.h"
#include "libavcodec/mathops.h"

// 0x7f7f7f7f or 0x7f7f7f7f7f7f7f7f or whatever, depending on the cpu's native arithmetic size
#define pb_7f (~0UL / 255 * 0x7f)
#define pb_80 (~0UL / 255 * 0x80)

{

    }
        dst[i + 0] += src[i + 0];

                              const uint8_t *diff, ptrdiff_t w,
                              int *left, int *left_top)
{


    }


                           int acc)
{

    }

        acc   += src[i];
        dst[i] = acc;
    }

}


    }

        acc+= src[i];
        dst[i]= acc &= mask;
    }

}


    }

{


        ff_llviddsp_init_ppc(c);
