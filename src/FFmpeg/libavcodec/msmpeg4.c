/*
 * MSMPEG4 backend for encoder and decoder
 * Copyright (c) 2001 Fabrice Bellard
 * Copyright (c) 2002-2004 Michael Niedermayer <michaelni@gmx.at>
 *
 * msmpeg4v1 & v2 stuff by Michael Niedermayer <michaelni@gmx.at>
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
 * MSMPEG4 backend for encoder and decoder
 */

#include "avcodec.h"
#include "idctdsp.h"
#include "mpegvideo.h"
#include "msmpeg4.h"
#include "libavutil/x86/asm.h"
#include "h263.h"
#include "mpeg4video.h"
#include "msmpeg4data.h"
#include "mpegvideodata.h"
#include "vc1data.h"
#include "libavutil/imgutils.h"

/*
 * You can also call this codec: MPEG-4 with a twist!
 *
 * TODO:
 *        - (encoding) select best mv table (two choices)
 *        - (encoding) select best vlc/dc table
 */

/* This table is practically identical to the one from H.263
 * except that it is inverted. */
{

            return;

            /* find number of bits */
            }

            else
                l= level;

            /* luminance H.263 */

                }
            }

            /* chrominance H.263 */

                }
            }

        }
}

{
    case 2:
        } else{
        }
        break;
    case 5:
#if CONFIG_VC1_DECODER
#endif

    }


    }
    //Note the default tables are set in common_init in mpegvideo.c


/* predict coded block */
{


    /* B C
     * A X
     */

        pred = a;
    } else {
    }

    /* store value */

}

static int get_dc(uint8_t *src, int stride, int scale, int block_size)
{
    int y;
    int sum=0;
    for(y=0; y<block_size; y++){
        int x;
        for(x=0; x<block_size; x++){
            sum+=src[x + y*stride];
        }
    }
    return FASTDIV((sum + (scale>>1)), scale);
}

/* dir = 0: left, dir = 1: top prediction */
                       int16_t **dc_val_ptr, int *dir_ptr)
{

    /* find prediction */
    } else {
    }


    /* B C
     * A X
     */

    }

    /* XXX: the following solution consumes divisions, but it does not
       necessitate to modify mpegvideo.c. The problem comes from the
       fact they decided to store the quantized DC (which would lead
       to problems if Q could vary !) */
#if ARCH_X86 && HAVE_7REGS && HAVE_EBX_AVAILABLE
        "movl %3, %%eax         \n\t"
        "shrl $1, %%eax         \n\t"
        "addl %%eax, %2         \n\t"
        "addl %%eax, %1         \n\t"
        "addl %0, %%eax         \n\t"
        "imull %4               \n\t"
        "movl %%edx, %0         \n\t"
        "movl %1, %%eax         \n\t"
        "imull %4               \n\t"
        "movl %%edx, %1         \n\t"
        "movl %2, %%eax         \n\t"
        "imull %4               \n\t"
        "movl %%edx, %2         \n\t"
        : "+b" (a), "+c" (b), "+D" (c)
        : "%eax", "%edx"
    );
#else
    /* Divisions are costly everywhere; optimize the most common case. */
    if (scale == 8) {
        a = (a + (8 >> 1)) / 8;
        b = (b + (8 >> 1)) / 8;
        c = (c + (8 >> 1)) / 8;
    } else {
        a = FASTDIV((a + (scale >> 1)), scale);
        b = FASTDIV((b + (scale >> 1)), scale);
        c = FASTDIV((c + (scale >> 1)), scale);
    }
#endif
    /* XXX: WARNING: they did not choose the same test as MPEG-4. This
       is very important ! */
            uint8_t *dest;
            int wrap;

            if(n==1){
                pred=a;
                *dir_ptr = 0;
            }else if(n==2){
                pred=c;
                *dir_ptr = 1;
            }else if(n==3){
                if (abs(a - b) < abs(b - c)) {
                    pred = c;
                    *dir_ptr = 1;
                } else {
                    pred = a;
                    *dir_ptr = 0;
                }
            }else{
                int bs = 8 >> s->avctx->lowres;
                if(n<4){
                    wrap= s->linesize;
                    dest= s->current_picture.f->data[0] + (((n >> 1) + 2*s->mb_y) * bs*  wrap ) + ((n & 1) + 2*s->mb_x) * bs;
                }else{
                    wrap= s->uvlinesize;
                    dest= s->current_picture.f->data[n - 3] + (s->mb_y * bs * wrap) + s->mb_x * bs;
                }
                if(s->mb_x==0) a= (1024 + (scale>>1))/scale;
                else           a= get_dc(dest-bs, wrap, scale*8>>(2*s->avctx->lowres), bs);
                if(s->mb_y==0) c= (1024 + (scale>>1))/scale;
                else           c= get_dc(dest-bs*wrap, wrap, scale*8>>(2*s->avctx->lowres), bs);

                if (s->h263_aic_dir==0) {
                    pred= a;
                    *dir_ptr = 0;
                }else if (s->h263_aic_dir==1) {
                    if(n==0){
                        pred= c;
                        *dir_ptr = 1;
                    }else{
                        pred= a;
                        *dir_ptr = 0;
                    }
                }else if (s->h263_aic_dir==2) {
                    if(n==0){
                        pred= a;
                        *dir_ptr = 0;
                    }else{
                        pred= c;
                        *dir_ptr = 1;
                    }
                } else {
                    pred= c;
                    *dir_ptr = 1;
                }
            }
        }else{
            } else {
            }
        }
    }else{
        } else {
        }
    }

    /* update predictor */
}

