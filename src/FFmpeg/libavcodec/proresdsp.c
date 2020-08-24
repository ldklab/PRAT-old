/*
 * Apple ProRes compatible decoder
 *
 * Copyright (c) 2010-2011 Maxim Poliakovski
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

#include "config.h"
#include "libavutil/attributes.h"
#include "libavutil/common.h"
#include "idctdsp.h"
#include "proresdsp.h"
#include "simple_idct.h"

#define CLIP_MIN (1 << 2)                     ///< minimum value for clipping resulting pixels
#define CLIP_MAX_10 (1 << 10) - CLIP_MIN - 1  ///< maximum value for clipping resulting pixels
#define CLIP_MAX_12 (1 << 12) - CLIP_MIN - 1  ///< maximum value for clipping resulting pixels

#define CLIP_10(x) (av_clip((x), CLIP_MIN, CLIP_MAX_10))
#define CLIP_12(x) (av_clip((x), CLIP_MIN, CLIP_MAX_12))

/**
 * Add bias value, clamp and output pixels of a slice
 */



                dst[dst_offset + x] = CLIP_10(in[src_offset]);
            } else {//12b
            }
        }
    }
}

static void put_pixels_10(uint16_t *dst, ptrdiff_t linesize, const int16_t *in)
{
    put_pixel(dst, linesize, in, 10);
}

{
}

static void prores_idct_put_10_c(uint16_t *out, ptrdiff_t linesize, int16_t *block, const int16_t *qmat)
{
    ff_prores_idct_10(block, qmat);
    put_pixels_10(out, linesize >> 1, block);
}

{

{
    } else {
        return AVERROR_BUG;
    }


}
