/*
 * PNG image format
 * Copyright (c) 2003 Fabrice Bellard
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
#include "png.h"

/* Mask to determine which y pixels are valid in a pass */
const uint8_t ff_png_pass_ymask[NB_PASSES] = {
    0x80, 0x80, 0x08, 0x88, 0x22, 0xaa, 0x55,
};

/* minimum x value */
static const uint8_t ff_png_pass_xmin[NB_PASSES] = {
    0, 4, 0, 2, 0, 1, 0
};

/* x shift to get row width */
static const uint8_t ff_png_pass_xshift[NB_PASSES] = {
    3, 3, 2, 2, 1, 1, 0
};

{
}

{

{
        PNG_COLOR_MASK_COLOR)
}

/* compute the row size of an interleaved pass */
{

        return 0;
}
