/*
 * thirdpel DSP functions
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
 * thirdpel DSP functions
 */

#include <stdint.h>

#include "libavutil/attributes.h"
#include "tpeldsp.h"

#define BIT_DEPTH 8
#include "pel_template.c"

                                          int stride, int width, int height)
{
    case 2:
        put_pixels2_8_c(dst, src, stride, height);
        break;
    case 4:
        break;
    case 8:
        break;
    }

                                          int stride, int width, int height)
{

    }

                                          int stride, int width, int height)
{

    }

                                          int stride, int width, int height)
{

    }

                                          int stride, int width, int height)
{

    }

                                          int stride, int width, int height)
{

    }

                                          int stride, int width, int height)
{

    }

                                          int stride, int width, int height)
{

    }

                                          int stride, int width, int height)
{

    }

                                          int stride, int width, int height)
{
    case 2:
        avg_pixels2_8_c(dst, src, stride, height);
        break;
    case 4:
        avg_pixels4_8_c(dst, src, stride, height);
        break;
    }

                                          int stride, int width, int height)
{

    }

                                          int stride, int width, int height)
{

    }

                                          int stride, int width, int height)
{

    }

                                          int stride, int width, int height)
{

    }

                                          int stride, int width, int height)
{

    }

                                          int stride, int width, int height)
{

    }

                                          int stride, int width, int height)
{

    }

static inline void avg_tpel_pixels_mc22_c(uint8_t *dst, const uint8_t *src,
                                          int stride, int width, int height)
{
    int i, j;

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++)
            dst[j] = (dst[j] +
                      (((2 * src[j]          + 3 * src[j + 1] +
                         3 * src[j + stride] + 4 * src[j + stride + 1] + 6) *
                        2731) >> 15) + 1) >> 1;
        src += stride;
        dst += stride;
    }
}

{

