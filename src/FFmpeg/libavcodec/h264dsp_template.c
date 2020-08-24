/*
 * H.26L/H.264/AVC/JVT/14496-10/... encoder/decoder
 * Copyright (c) 2003-2011 Michael Niedermayer <michaelni@gmx.at>
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
 * H.264 / AVC / MPEG-4 part10 DSP functions.
 * @author Michael Niedermayer <michaelni@gmx.at>
 */

#include "bit_depth_template.c"

#define op_scale1(x)  block[x] = av_clip_pixel( (block[x]*weight + offset) >> log2_denom )
#define op_scale2(x)  dst[x] = av_clip_pixel( (src[x]*weights + dst[x]*weightd + offset) >> (log2_denom+1))
#define H264_WEIGHT(W) \
static void FUNCC(weight_h264_pixels ## W)(uint8_t *_block, ptrdiff_t stride, int height, \
                                           int log2_denom, int weight, int offset) \
{ \
    int y; \
    pixel *block = (pixel*)_block; \
    stride >>= sizeof(pixel)-1; \
    offset = (unsigned)offset << (log2_denom + (BIT_DEPTH-8)); \
    if(log2_denom) offset += 1<<(log2_denom-1); \
    for (y = 0; y < height; y++, block += stride) { \
        op_scale1(0); \
        op_scale1(1); \
        if(W==2) continue; \
        op_scale1(2); \
        op_scale1(3); \
        if(W==4) continue; \
        op_scale1(4); \
        op_scale1(5); \
        op_scale1(6); \
        op_scale1(7); \
        if(W==8) continue; \
        op_scale1(8); \
        op_scale1(9); \
        op_scale1(10); \
        op_scale1(11); \
        op_scale1(12); \
        op_scale1(13); \
        op_scale1(14); \
        op_scale1(15); \
    } \
} \
static void FUNCC(biweight_h264_pixels ## W)(uint8_t *_dst, uint8_t *_src, ptrdiff_t stride, int height, \
                                             int log2_denom, int weightd, int weights, int offset) \
{ \
    int y; \
    pixel *dst = (pixel*)_dst; \
    pixel *src = (pixel*)_src; \
    stride >>= sizeof(pixel)-1; \
    offset = (unsigned)offset << (BIT_DEPTH-8); \
    offset = (unsigned)((offset + 1) | 1) << log2_denom; \
    for (y = 0; y < height; y++, dst += stride, src += stride) { \
        op_scale2(0); \
        op_scale2(1); \
        if(W==2) continue; \
        op_scale2(2); \
        op_scale2(3); \
        if(W==4) continue; \
        op_scale2(4); \
        op_scale2(5); \
        op_scale2(6); \
        op_scale2(7); \
        if(W==8) continue; \
        op_scale2(8); \
        op_scale2(9); \
        op_scale2(10); \
        op_scale2(11); \
        op_scale2(12); \
        op_scale2(13); \
        op_scale2(14); \
        op_scale2(15); \
    } \
}


#undef op_scale1
#undef op_scale2
#undef H264_WEIGHT

{
        }



                }
                }

            }
        }
    }
}
{
{
{

{



                {
                    /* p0', p1', p2' */
                } else {
                    /* p0' */
                }
                {
                    /* q0', q1', q2' */
                } else {
                    /* q0' */
                }
            }else{
                /* p0', q0' */
            }
        }
    }
}
{
{
{

{
        }



            }
        }
    }
}
{
{
{
{
{

{


        }
    }
}
{
{
{
{
{
