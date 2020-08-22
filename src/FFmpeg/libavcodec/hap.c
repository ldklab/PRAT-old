/*
 * Vidvox Hap utility functions
 * Copyright (C) 2015 Tom Butterworth <bangnoise@gmail.com>
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
 * Hap utilities
 */
#include "hap.h"

{
            ctx->chunk_count = 0;
        } else {
        }
        /* If this is not the first chunk count calculated for a frame and a
         * different count has already been encountered, then reject the frame:
         * each table in the Decode Instructions Container must describe the
         * same number of chunks. */
        ret = AVERROR_INVALIDDATA;
    }
}

{

                                enum HapSectionType *section_type)
{
        return AVERROR_INVALIDDATA;


            return AVERROR_INVALIDDATA;

    }

        return AVERROR_INVALIDDATA;
    else
}
