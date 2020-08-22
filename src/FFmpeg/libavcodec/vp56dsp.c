/*
 * Copyright (c) 2006 Aurelien Jacobs <aurel@gnuage.org>
 * Copyright (c) 2010 Mans Rullgard <mans@mansr.com>
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

#include <stdint.h>

#include "libavutil/attributes.h"
#include "avcodec.h"
#include "vp56dsp.h"
#include "libavutil/common.h"

#define VP56_EDGE_FILTER(pfx, suf, pix_inc, line_inc)                   \
static void pfx ## _edge_filter_ ## suf(uint8_t *yuv, ptrdiff_t stride, \
                                        int t)                          \
{                                                                       \
    int pix2_inc = 2 * pix_inc;                                         \
    int i, v;                                                           \
                                                                        \
    for (i=0; i<12; i++) {                                              \
        v = (yuv[-pix2_inc] + 3*(yuv[0]-yuv[-pix_inc]) - yuv[pix_inc] + 4)>>3;\
        v = pfx##_adjust(v, t);                                         \
        yuv[-pix_inc] = av_clip_uint8(yuv[-pix_inc] + v);               \
        yuv[0] = av_clip_uint8(yuv[0] - v);                             \
        yuv += line_inc;                                                \
    }                                                                   \
}

#if CONFIG_VP5_DECODER
/* Gives very similar result than the vp6 version except in a few cases */
{
}



{
#endif /* CONFIG_VP5_DECODER */

#if CONFIG_VP6_DECODER
{

        ff_vp6dsp_init_arm(s);
#endif /* CONFIG_VP6_DECODER */
