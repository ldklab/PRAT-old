/*
 * JPEG-LS common code
 * Copyright (c) 2003 Michael Niedermayer
 * Copyright (c) 2006 Konstantin Shishkov
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
 * JPEG-LS common code.
 */

#include "internal.h"
#include "jpegls.h"

{


    // QBPP = ceil(log2(RANGE))


    }

/**
 * Custom value clipping function used in T1, T2, T3 calculation
 */
{
        return vmin;
    else
}

{



                             s->T1, s->maxval);
                             s->T2, s->maxval);
    } else {
        factor = 256 / (s->maxval + 1);

        if (s->T1 == 0 || reset_all)
            s->T1 = iso_clip(FFMAX(2, basic_t1 / factor + 3 * s->near),
                             s->near + 1, s->maxval);
        if (s->T2 == 0 || reset_all)
            s->T2 = iso_clip(FFMAX(3, basic_t2 / factor + 5 * s->near),
                             s->T1, s->maxval);
        if (s->T3 == 0 || reset_all)
            s->T3 = iso_clip(FFMAX(4, basic_t3 / factor + 7 * s->near),
                             s->T2, s->maxval);
    }

