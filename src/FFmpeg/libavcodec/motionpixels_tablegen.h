/*
 * Header file for hardcoded motion pixels RGB to YUV table
 *
 * Copyright (c) 2009 Reimar Döffinger <Reimar.Doeffinger@gmx.de>
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

#ifndef AVCODEC_MOTIONPIXELS_TABLEGEN_H
#define AVCODEC_MOTIONPIXELS_TABLEGEN_H

#include <stdint.h>
#include "libavutil/attributes.h"

typedef struct YuvPixel {
    int8_t y, v, u;
} YuvPixel;


    return 1 << 15;
}

#if CONFIG_HARDCODED_TABLES
#define motionpixels_tableinit()
#include "libavcodec/motionpixels_tables.h"
#else
static YuvPixel mp_rgb_yuv_table[1 << 15];

{

    }

{

                }
            }

{
#endif /* CONFIG_HARDCODED_TABLES */

#endif /* AVCODEC_MOTIONPIXELS_TABLEGEN_H */
