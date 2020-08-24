/*
 * Header file for hardcoded sine windows
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

#ifndef AVCODEC_SINEWIN_TABLEGEN_H
#define AVCODEC_SINEWIN_TABLEGEN_H

#include <assert.h>
// do not use libavutil/libm.h since this is compiled both
// for the host and the target and config.h is only valid for the target
#include <math.h>
#include "libavcodec/aac_defines.h"
#include "libavutil/attributes.h"
#include "libavutil/common.h"

#if !USE_FIXED
SINETABLE120960(120);
SINETABLE120960(960);
#endif
#if !CONFIG_HARDCODED_TABLES
SINETABLE(  32);
SINETABLE(  64);
SINETABLE( 128);
SINETABLE( 256);
SINETABLE( 512);
SINETABLE(1024);
SINETABLE(2048);
SINETABLE(4096);
SINETABLE(8192);
#else
#if USE_FIXED
#include "libavcodec/sinewin_fixed_tables.h"
#else
#include "libavcodec/sinewin_tables.h"
#endif
#endif

#if USE_FIXED
#define SIN_FIX(a) (int)floor((a) * 0x80000000 + 0.5)
#else
#define SIN_FIX(a) a
#endif

SINETABLE_CONST INTFLOAT * const AAC_RENAME(ff_sine_windows)[] = {
    NULL, NULL, NULL, NULL, NULL, // unused
    AAC_RENAME(ff_sine_32) , AAC_RENAME(ff_sine_64), AAC_RENAME(ff_sine_128),
    AAC_RENAME(ff_sine_256), AAC_RENAME(ff_sine_512), AAC_RENAME(ff_sine_1024),
    AAC_RENAME(ff_sine_2048), AAC_RENAME(ff_sine_4096), AAC_RENAME(ff_sine_8192),
};

// Generate a sine window.

#if !CONFIG_HARDCODED_TABLES
#endif

#endif /* AVCODEC_SINEWIN_TABLEGEN_H */
