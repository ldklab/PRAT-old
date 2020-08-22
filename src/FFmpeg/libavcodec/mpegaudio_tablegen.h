/*
 * Header file for hardcoded mpegaudiodec tables
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

#ifndef AVCODEC_MPEGAUDIO_TABLEGEN_H
#define AVCODEC_MPEGAUDIO_TABLEGEN_H

#include <stdint.h>
#include <math.h>
#include "libavutil/attributes.h"

#define TABLE_4_3_SIZE (8191 + 16)*4
#if CONFIG_HARDCODED_TABLES
#define mpegaudio_tableinit()
#include "libavcodec/mpegaudio_tables.h"
#else
static int8_t   table_4_3_exp[TABLE_4_3_SIZE];
static uint32_t table_4_3_value[TABLE_4_3_SIZE];
static uint32_t exp_table_fixed[512];
static uint32_t expval_table_fixed[512][16];
static float exp_table_float[512];
static float expval_table_float[512][16];

#define FRAC_BITS 23
#define IMDCT_SCALAR 1.759

{
        1.00000000000000000000, /* 2 ^ (0 * 0.25) */
        1.18920711500272106672, /* 2 ^ (1 * 0.25) */
        M_SQRT2               , /* 2 ^ (2 * 0.25) */
        1.68179283050742908606, /* 2 ^ (3 * 0.25) */
    };


        /* normalized to FRAC_BITS */
    }
        }
    }
#endif /* CONFIG_HARDCODED_TABLES */

#endif /* AVCODEC_MPEGAUDIO_TABLEGEN_H */
