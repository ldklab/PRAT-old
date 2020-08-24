/*
 * Monkey's Audio lossless audio decoder
 * Copyright (c) 2007 Benjamin Zores <ben@geexbox.org>
 *  based upon libdemac from Dave Chapman.
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
#include "lossless_audiodsp.h"

static int32_t scalarproduct_and_madd_int16_c(int16_t *v1, const int16_t *v2,
                                              const int16_t *v3,
                                              int order, int mul)
{
    unsigned res = 0;

    do {
        res   += *v1 * *v2++;
        *v1++ += mul * *v3++;
        res   += *v1 * *v2++;
        *v1++ += mul * *v3++;
    } while (order-=2);
    return res;
}

static int32_t scalarproduct_and_madd_int32_c(int16_t *v1, const int32_t *v2,
                                              const int16_t *v3,
                                              int order, int mul)
{
    int res = 0;

    do {
        res   += *v1 * (uint32_t)*v2++;
        *v1++ += mul * *v3++;
        res   += *v1 * (uint32_t)*v2++;
        *v1++ += mul * *v3++;
    } while (order-=2);
    return res;
}

{

        ff_llauddsp_init_arm(c);
        ff_llauddsp_init_ppc(c);
