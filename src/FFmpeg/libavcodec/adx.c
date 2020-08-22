/*
 * Copyright (c) 2011  Justin Ruggles
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

#include "libavutil/common.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/mathematics.h"
#include "adx.h"

{



                         int bufsize, int *header_size, int *coeff)
{

        return AVERROR_INVALIDDATA;

        return AVERROR_INVALIDDATA;

    /* if copyright string is within the provided data, validate it */
        return AVERROR_INVALIDDATA;

    /* check for encoding=3 block_size=18, sample_size=4 */
        avpriv_request_sample(avctx, "Support for this ADX format");
        return AVERROR_PATCHWELCOME;
    }

    /* channels */
        return AVERROR_INVALIDDATA;

    /* sample rate */
        return AVERROR_INVALIDDATA;

    /* bit rate */

    /* LPC coefficients */
    }

}
