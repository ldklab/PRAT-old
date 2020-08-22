/*
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

#include "config.h"
#include "libavutil/attributes.h"
#include "h264chroma.h"

#define BIT_DEPTH 8
#include "h264chroma_template.c"
#undef BIT_DEPTH

#define BIT_DEPTH 16
#include "h264chroma_template.c"
#undef BIT_DEPTH

#define SET_CHROMA(depth)                                                   \
    c->put_h264_chroma_pixels_tab[0] = put_h264_chroma_mc8_ ## depth ## _c; \
    c->put_h264_chroma_pixels_tab[1] = put_h264_chroma_mc4_ ## depth ## _c; \
    c->put_h264_chroma_pixels_tab[2] = put_h264_chroma_mc2_ ## depth ## _c; \
    c->put_h264_chroma_pixels_tab[3] = put_h264_chroma_mc1_ ## depth ## _c; \
    c->avg_h264_chroma_pixels_tab[0] = avg_h264_chroma_mc8_ ## depth ## _c; \
    c->avg_h264_chroma_pixels_tab[1] = avg_h264_chroma_mc4_ ## depth ## _c; \
    c->avg_h264_chroma_pixels_tab[2] = avg_h264_chroma_mc2_ ## depth ## _c; \
    c->avg_h264_chroma_pixels_tab[3] = avg_h264_chroma_mc1_ ## depth ## _c; \

{
    } else {
    }

        ff_h264chroma_init_aarch64(c, bit_depth);
        ff_h264chroma_init_arm(c, bit_depth);
        ff_h264chroma_init_ppc(c, bit_depth);
        ff_h264chroma_init_mips(c, bit_depth);
