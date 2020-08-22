/*
 * Floating point AAN IDCT
 * Copyright (c) 2008 Michael Niedermayer <michaelni@gmx.at>
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
#include "faanidct.h"
#include "libavutil/common.h"

/* To allow switching to double. */
typedef float FLOAT;

#define B0 1.0000000000000000000000
#define B1 1.3870398453221474618216 // cos(pi*1/16)sqrt(2)
#define B2 1.3065629648763765278566 // cos(pi*2/16)sqrt(2)
#define B3 1.1758756024193587169745 // cos(pi*3/16)sqrt(2)
#define B4 1.0000000000000000000000 // cos(pi*4/16)sqrt(2)
#define B5 0.7856949583871021812779 // cos(pi*5/16)sqrt(2)
#define B6 0.5411961001461969843997 // cos(pi*6/16)sqrt(2)
#define B7 0.2758993792829430123360 // cos(pi*7/16)sqrt(2)

#define A4 0.70710678118654752438 // cos(pi*4/16)
#define A2 0.92387953251128675613 // cos(pi*2/16)

static const FLOAT prescale[64]={
B0*B0/8, B0*B1/8, B0*B2/8, B0*B3/8, B0*B4/8, B0*B5/8, B0*B6/8, B0*B7/8,
B1*B0/8, B1*B1/8, B1*B2/8, B1*B3/8, B1*B4/8, B1*B5/8, B1*B6/8, B1*B7/8,
B2*B0/8, B2*B1/8, B2*B2/8, B2*B3/8, B2*B4/8, B2*B5/8, B2*B6/8, B2*B7/8,
B3*B0/8, B3*B1/8, B3*B2/8, B3*B3/8, B3*B4/8, B3*B5/8, B3*B6/8, B3*B7/8,
B4*B0/8, B4*B1/8, B4*B2/8, B4*B3/8, B4*B4/8, B4*B5/8, B4*B6/8, B4*B7/8,
B5*B0/8, B5*B1/8, B5*B2/8, B5*B3/8, B5*B4/8, B5*B5/8, B5*B6/8, B5*B7/8,
B6*B0/8, B6*B1/8, B6*B2/8, B6*B3/8, B6*B4/8, B6*B5/8, B6*B6/8, B6*B7/8,
B7*B0/8, B7*B1/8, B7*B2/8, B7*B3/8, B7*B4/8, B7*B5/8, B7*B6/8, B7*B7/8,
};

                          ptrdiff_t stride, int x, int y, int type)
{








        }else if(type==2){
            dest[0*stride + i]= av_clip_uint8(((int)dest[0*stride + i]) + lrintf(os07 + od07));
            dest[7*stride + i]= av_clip_uint8(((int)dest[7*stride + i]) + lrintf(os07 - od07));
            dest[1*stride + i]= av_clip_uint8(((int)dest[1*stride + i]) + lrintf(os16 + od16));
            dest[6*stride + i]= av_clip_uint8(((int)dest[6*stride + i]) + lrintf(os16 - od16));
            dest[2*stride + i]= av_clip_uint8(((int)dest[2*stride + i]) + lrintf(os25 + od25));
            dest[5*stride + i]= av_clip_uint8(((int)dest[5*stride + i]) + lrintf(os25 - od25));
            dest[3*stride + i]= av_clip_uint8(((int)dest[3*stride + i]) + lrintf(os34 - od34));
            dest[4*stride + i]= av_clip_uint8(((int)dest[4*stride + i]) + lrintf(os34 + od34));
        }else{
            dest[0*stride + i]= av_clip_uint8(lrintf(os07 + od07));
            dest[7*stride + i]= av_clip_uint8(lrintf(os07 - od07));
            dest[1*stride + i]= av_clip_uint8(lrintf(os16 + od16));
            dest[6*stride + i]= av_clip_uint8(lrintf(os16 - od16));
            dest[2*stride + i]= av_clip_uint8(lrintf(os25 + od25));
            dest[5*stride + i]= av_clip_uint8(lrintf(os25 - od25));
            dest[3*stride + i]= av_clip_uint8(lrintf(os34 - od34));
            dest[4*stride + i]= av_clip_uint8(lrintf(os34 + od34));
        }
    }





void ff_faanidct_add(uint8_t *dest, ptrdiff_t line_size, int16_t block[64])
{
    FLOAT temp[64];
    int i;

    emms_c();

    for(i=0; i<64; i++)
        temp[i] = block[i] * prescale[i];

    p8idct(block, temp, NULL,         0, 1, 8, 0);
    p8idct(NULL , temp, dest, line_size, 8, 1, 2);
}

void ff_faanidct_put(uint8_t *dest, ptrdiff_t line_size, int16_t block[64])
{
    FLOAT temp[64];
    int i;

    emms_c();

    for(i=0; i<64; i++)
        temp[i] = block[i] * prescale[i];

    p8idct(block, temp, NULL,         0, 1, 8, 0);
    p8idct(NULL , temp, dest, line_size, 8, 1, 3);
}
