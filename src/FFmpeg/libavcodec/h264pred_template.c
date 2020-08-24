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
 * H.264 / AVC / MPEG-4 part10 prediction functions.
 * @author Michael Niedermayer <michaelni@gmx.at>
 */

#include "libavutil/intreadwrite.h"

#include "mathops.h"

#include "bit_depth_template.c"

                                    ptrdiff_t _stride)
{


                                      ptrdiff_t _stride)
{

                              ptrdiff_t _stride)
{


                                   ptrdiff_t _stride)
{


                                  ptrdiff_t _stride)
{


                                  ptrdiff_t _stride)
{


                                  ptrdiff_t _stride)
{


                                  ptrdiff_t _stride)
{



#define LOAD_TOP_RIGHT_EDGE\
    const unsigned av_unused t4 = topright[0];\
    const unsigned av_unused t5 = topright[1];\
    const unsigned av_unused t6 = topright[2];\
    const unsigned av_unused t7 = topright[3];\

#define LOAD_DOWN_LEFT_EDGE\
    const unsigned av_unused l4 = src[-1+4*stride];\
    const unsigned av_unused l5 = src[-1+5*stride];\
    const unsigned av_unused l6 = src[-1+6*stride];\
    const unsigned av_unused l7 = src[-1+7*stride];\

#define LOAD_LEFT_EDGE\
    const unsigned av_unused l0 = src[-1+0*stride];\
    const unsigned av_unused l1 = src[-1+1*stride];\
    const unsigned av_unused l2 = src[-1+2*stride];\
    const unsigned av_unused l3 = src[-1+3*stride];\

#define LOAD_TOP_EDGE\
    const unsigned av_unused t0 = src[ 0-1*stride];\
    const unsigned av_unused t1 = src[ 1-1*stride];\
    const unsigned av_unused t2 = src[ 2-1*stride];\
    const unsigned av_unused t3 = src[ 3-1*stride];\

                                      ptrdiff_t _stride)
{


                                     ptrdiff_t _stride)
{
//    LOAD_LEFT_EDGE


                                          const uint8_t *topright,
                                          ptrdiff_t _stride)
{


                                         const uint8_t *_topright,
                                         ptrdiff_t _stride)
{


                                         ptrdiff_t _stride)
{


                                           const uint8_t *topright,
                                           ptrdiff_t _stride)
{


{

    }

{


    }

#define PREDICT_16x16_DC(v)\
    for(i=0; i<16; i++){\
        AV_WN4PA(src+ 0, v);\
        AV_WN4PA(src+ 4, v);\
        AV_WN4PA(src+ 8, v);\
        AV_WN4PA(src+12, v);\
        src += stride;\
    }

{

    }

    }


{

    }


{

    }


#define PRED16x16_X(n, v) \
static void FUNCC(pred16x16_##n##_dc)(uint8_t *_src, ptrdiff_t stride)\
{\
    int i;\
    pixel *src = (pixel*)_src;\
    stride >>= sizeof(pixel)-1;\
    PREDICT_16x16_DC(PIXEL_SPLAT_X4(v));\
}


                                                 ptrdiff_t _stride,
                                                 const int svq3,
                                                 const int rv40)
{
  INIT_CLIP
  }

    /* required for 100% accuracy */
  }else{
  }

    }
  }

{

{

    }

{

    }

{

    }

{
    }

#define PRED8x8_X(n, v)\
static void FUNCC(pred8x8_##n##_dc)(uint8_t *_src, ptrdiff_t stride)\
{\
    int i;\
    const pixel4 a = PIXEL_SPLAT_X4(v);\
    pixel *src = (pixel*)_src;\
    stride >>= sizeof(pixel)-1;\
    for(i=0; i<8; i++){\
        AV_WN4PA(((pixel4*)(src+i*stride))+0, a);\
        AV_WN4PA(((pixel4*)(src+i*stride))+1, a);\
    }\
}


{

{

    }

    }
    }

{

{

    }

    }
    }

{

    }

    }

{

    }

    }
    }

{

    }

    }
    }
    }
    }

//the following 4 function should not be optimized!
{

{

{

{

{

{

{

{

{
  INIT_CLIP
  }

  }

{
  INIT_CLIP

  }
  }


  }

#define SRC(x,y) src[(x)+(y)*stride]
#define PL(y) \
    const int l##y = (SRC(-1,y-1) + 2*SRC(-1,y) + SRC(-1,y+1) + 2) >> 2;
#define PREDICT_8x8_LOAD_LEFT \
    const int l0 = ((has_topleft ? SRC(-1,-1) : SRC(-1,0)) \
                     + 2*SRC(-1,0) + SRC(-1,1) + 2) >> 2; \
    PL(1) PL(2) PL(3) PL(4) PL(5) PL(6) \
    const int l7 av_unused = (SRC(-1,6) + 3*SRC(-1,7) + 2) >> 2

#define PT(x) \
    const int t##x = (SRC(x-1,-1) + 2*SRC(x,-1) + SRC(x+1,-1) + 2) >> 2;
#define PREDICT_8x8_LOAD_TOP \
    const int t0 = ((has_topleft ? SRC(-1,-1) : SRC(0,-1)) \
                     + 2*SRC(0,-1) + SRC(1,-1) + 2) >> 2; \
    PT(1) PT(2) PT(3) PT(4) PT(5) PT(6) \
    const int t7 av_unused = ((has_topright ? SRC(8,-1) : SRC(7,-1)) \
                     + 2*SRC(7,-1) + SRC(6,-1) + 2) >> 2

#define PTR(x) \
    t##x = (SRC(x-1,-1) + 2*SRC(x,-1) + SRC(x+1,-1) + 2) >> 2;
#define PREDICT_8x8_LOAD_TOPRIGHT \
    int t8, t9, t10, t11, t12, t13, t14, t15; \
    if(has_topright) { \
        PTR(8) PTR(9) PTR(10) PTR(11) PTR(12) PTR(13) PTR(14) \
        t15 = (SRC(14,-1) + 3*SRC(15,-1) + 2) >> 2; \
    } else t8=t9=t10=t11=t12=t13=t14=t15= SRC(7,-1);

#define PREDICT_8x8_LOAD_TOPLEFT \
    const int lt = (SRC(-1,0) + 2*SRC(-1,-1) + SRC(0,-1) + 2) >> 2

#define PREDICT_8x8_DC(v) \
    int y; \
    for( y = 0; y < 8; y++ ) { \
        AV_WN4PA(((pixel4*)src)+0, v); \
        AV_WN4PA(((pixel4*)src)+1, v); \
        src += stride; \
    }

                                   int has_topright, ptrdiff_t _stride)
{

                                    int has_topright, ptrdiff_t _stride)
{

                                   int has_topright, ptrdiff_t _stride)
{

                               int has_topright, ptrdiff_t _stride)
{

                                     +t0+t1+t2+t3+t4+t5+t6+t7+8) >> 4);
                                       int has_topright, ptrdiff_t _stride)
{

#define ROW(y) a = PIXEL_SPLAT_X4(l##y); \
               AV_WN4PA(src+y*stride, a); \
               AV_WN4PA(src+y*stride+4, a);
#undef ROW
                                     int has_topright, ptrdiff_t _stride)
{

    }
                                      int has_topright, ptrdiff_t _stride)
{
                                       int has_topright, ptrdiff_t _stride)
{
                                           int has_topright, ptrdiff_t _stride)
{
                                            int has_topright, ptrdiff_t _stride)
{
                                          int has_topright, ptrdiff_t _stride)
{
                                          int has_topright, ptrdiff_t _stride)
{

static void FUNCC(pred8x8l_vertical_filter_add)(uint8_t *_src, int16_t *_block, int has_topleft,
                                     int has_topright, ptrdiff_t _stride)
{
    int i;
    pixel *src = (pixel*)_src;
    const dctcoef *block = (const dctcoef*)_block;
    pixel pix[8];
    int stride = _stride>>(sizeof(pixel)-1);
    PREDICT_8x8_LOAD_TOP;

    pix[0] = t0;
    pix[1] = t1;
    pix[2] = t2;
    pix[3] = t3;
    pix[4] = t4;
    pix[5] = t5;
    pix[6] = t6;
    pix[7] = t7;

    for(i=0; i<8; i++){
        pixel v = pix[i];
        src[0*stride]= v += block[0];
        src[1*stride]= v += block[8];
        src[2*stride]= v += block[16];
        src[3*stride]= v += block[24];
        src[4*stride]= v += block[32];
        src[5*stride]= v += block[40];
        src[6*stride]= v += block[48];
        src[7*stride]= v +  block[56];
        src++;
        block++;
    }

    memset(_block, 0, sizeof(dctcoef) * 64);
}

static void FUNCC(pred8x8l_horizontal_filter_add)(uint8_t *_src, int16_t *_block, int has_topleft,
                               int has_topright, ptrdiff_t _stride)
{
    int i;
    pixel *src = (pixel*)_src;
    const dctcoef *block = (const dctcoef*)_block;
    pixel pix[8];
    int stride = _stride>>(sizeof(pixel)-1);
    PREDICT_8x8_LOAD_LEFT;

    pix[0] = l0;
    pix[1] = l1;
    pix[2] = l2;
    pix[3] = l3;
    pix[4] = l4;
    pix[5] = l5;
    pix[6] = l6;
    pix[7] = l7;

    for(i=0; i<8; i++){
        pixel v = pix[i];
        src[0]= v += block[0];
        src[1]= v += block[1];
        src[2]= v += block[2];
        src[3]= v += block[3];
        src[4]= v += block[4];
        src[5]= v += block[5];
        src[6]= v += block[6];
        src[7]= v +  block[7];
        src+= stride;
        block+= 8;
    }

    memset(_block, 0, sizeof(dctcoef) * 64);
}

#undef PREDICT_8x8_LOAD_LEFT
#undef PREDICT_8x8_LOAD_TOP
#undef PREDICT_8x8_LOAD_TOPLEFT
#undef PREDICT_8x8_LOAD_TOPRIGHT
#undef PREDICT_8x8_DC
#undef PTR
#undef PT
#undef PL
#undef SRC

                                        ptrdiff_t stride)
{
    }


                                          ptrdiff_t stride)
{
    }


                                         ptrdiff_t stride)
{
    }


                                           ptrdiff_t stride)
{
    }


                                          int16_t *block,
                                          ptrdiff_t stride)
{

                                            const int *block_offset,
                                            int16_t *block,
                                            ptrdiff_t stride)
{

                                        int16_t *block, ptrdiff_t stride)
{

static void FUNCC(pred8x16_vertical_add)(uint8_t *pix, const int *block_offset,
                                         int16_t *block, ptrdiff_t stride)
{
    int i;
    for(i=0; i<4; i++)
        FUNCC(pred4x4_vertical_add)(pix + block_offset[i], block + i*16*sizeof(pixel), stride);
    for(i=4; i<8; i++)
        FUNCC(pred4x4_vertical_add)(pix + block_offset[i+4], block + i*16*sizeof(pixel), stride);
}

                                          int16_t *block,
                                          ptrdiff_t stride)
{

static void FUNCC(pred8x16_horizontal_add)(uint8_t *pix,
                                           const int *block_offset,
                                           int16_t *block, ptrdiff_t stride)
{
    int i;
    for(i=0; i<4; i++)
        FUNCC(pred4x4_horizontal_add)(pix + block_offset[i], block + i*16*sizeof(pixel), stride);
    for(i=4; i<8; i++)
        FUNCC(pred4x4_horizontal_add)(pix + block_offset[i+4], block + i*16*sizeof(pixel), stride);
}
