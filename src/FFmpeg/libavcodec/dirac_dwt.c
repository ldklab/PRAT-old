/*
 * Copyright (C) 2004-2010 Michael Niedermayer <michaelni@gmx.at>
 * Copyright (C) 2008 David Conrad
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

#include "libavutil/attributes.h"
#include "libavutil/avassert.h"
#include "libavutil/common.h"
#include "dirac_dwt.h"

#define TEMPLATE_8bit
#include "dirac_dwt_template.c"

#define TEMPLATE_10bit
#include "dirac_dwt_template.c"

#define TEMPLATE_12bit
#include "dirac_dwt_template.c"

                         int decomposition_count, int bit_depth)
{


    else
        av_log(NULL, AV_LOG_WARNING, "Unsupported bit depth = %i\n", bit_depth);

        av_log(NULL, AV_LOG_ERROR, "Unknown wavelet type %d\n", type);
        return AVERROR_INVALIDDATA;
    }

    return 0;
}

{


    }
