/*
 * Motion estimation
 * Copyright (c) 2000,2001 Fabrice Bellard
 * Copyright (c) 2002-2004 Michael Niedermayer
 *
 * new motion estimation (X1/EPZS) by Michael Niedermayer <michaelni@gmx.at>
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
 * Motion estimation.
 */

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include "avcodec.h"
#include "internal.h"
#include "mathops.h"
#include "motion_est.h"
#include "mpegutils.h"
#include "mpegvideo.h"

#define P_LEFT P[1]
#define P_TOP P[2]
#define P_TOPRIGHT P[3]
#define P_MEDIAN P[4]
#define P_MV1 P[9]

#define ME_MAP_SHIFT 3
#define ME_MAP_MV_BITS 11

static int sad_hpel_motion_search(MpegEncContext * s,
                                  int *mx_ptr, int *my_ptr, int dmin,
                                  int src_index, int ref_index,
                                  int size, int h);

{
    }
}

/* shape adaptive search stuff */
typedef struct Minima{
    int height;
    int x, y;
    int checked;
}Minima;

static int minima_cmp(const void *a, const void *b){
    const Minima *da = (const Minima *) a;
    const Minima *db = (const Minima *) b;

    return da->height - db->height;
}

#define FLAG_QPEL   1 //must be 1
#define FLAG_CHROMA 2
#define FLAG_DIRECT 4

        ((y*c->uvstride + x)>>1),
    };
    }
        }
    }

           + (direct ? FLAG_DIRECT : 0)
}

                      const int size, const int h, int ref_index, int src_index,
                      me_cmp_func cmp_func, me_cmp_func chroma_cmp_func, int qpel){
    //FIXME check chroma 4mv, (no crashes ...)
                int i;

                    }else{
                    }
                }
            }else{

                }else{

                }
            }
        }else
            d= 256*256*256*32;
}

                      const int size, const int h, int ref_index, int src_index,
                      me_cmp_func cmp_func, me_cmp_func chroma_cmp_func, int qpel, int chroma){
    //FIXME check chroma 4mv, (no crashes ...)
                } else if (size == 0 && h == 8) {
                    c->qpel_put[1][dxy](c->temp    , ref[0] + x + y*stride    , stride);
                    c->qpel_put[1][dxy](c->temp + 8, ref[0] + x + y*stride + 8, stride);
                } else
                    int cx= hx/2;
                    int cy= hy/2;
                    cx= (cx>>1)|(cx&1);
                    cy= (cy>>1)|(cy&1);
                    uvdxy= (cx&1) + 2*(cy&1);
                    // FIXME x/y wrong, but MPEG-4 qpel is sick anyway, we should drop as much of it as possible in favor for H.264
                }
            }else{
                    uvdxy= dxy | (x&1) | (2*(y&1));
            }
        }else{
                uvdxy= (x&1) + 2*(y&1);
        }
            uint8_t * const uvtemp= c->temp + 16*stride;
            c->hpel_put[size+1][uvdxy](uvtemp  , ref[1] + (x>>1) + (y>>1)*uvstride, uvstride, h>>1);
            c->hpel_put[size+1][uvdxy](uvtemp+8, ref[2] + (x>>1) + (y>>1)*uvstride, uvstride, h>>1);
            d += chroma_cmp_func(s, uvtemp  , src[1], uvstride, h>>1);
            d += chroma_cmp_func(s, uvtemp+8, src[2], uvstride, h>>1);
        }
}

static int cmp_simple(MpegEncContext *s, const int x, const int y,
                      int ref_index, int src_index,
                      me_cmp_func cmp_func, me_cmp_func chroma_cmp_func){
    return cmp_inline(s,x,y,0,0,0,16,ref_index,src_index, cmp_func, chroma_cmp_func, 0, 0);
}

                      const int size, const int h, int ref_index, int src_index,
                      me_cmp_func cmp_func, me_cmp_func chroma_cmp_func, const int flags){
    }else{
    }
}

                      const int size, const int h, int ref_index, int src_index,
                      me_cmp_func cmp_func, me_cmp_func chroma_cmp_func, const int flags){
        return cmp_direct_inline(s,x,y,subx,suby,size,h,ref_index,src_index, cmp_func, chroma_cmp_func, flags&FLAG_QPEL);
    }else{
    }
}

/** @brief compares a block (either a full macroblock or a partition thereof)
    against a proposed motion-compensated prediction of that block
 */
                      const int size, const int h, int ref_index, int src_index,
                      me_cmp_func cmp_func, me_cmp_func chroma_cmp_func, const int flags){
       && av_builtin_constant_p(subx) && av_builtin_constant_p(suby)
       && flags==0 && h==16 && size==0 && subx==0 && suby==0){
       && subx==0 && suby==0){
    }else{
    }
}

                      const int size, const int h, int ref_index, int src_index,
                      me_cmp_func cmp_func, me_cmp_func chroma_cmp_func, const int flags){
    }else{
    }
}

                      const int size, const int h, int ref_index, int src_index,
                      me_cmp_func cmp_func, me_cmp_func chroma_cmp_func, const int flags){
    }else{
    }
}

#include "motion_est_template.c"

static int zero_cmp(MpegEncContext *s, uint8_t *a, uint8_t *b,
                    ptrdiff_t stride, int h)
{
    return 0;
}

static void zero_hpel(uint8_t *a, const uint8_t *b, ptrdiff_t stride, int h){
}


        av_log(s->avctx, AV_LOG_ERROR, "ME_MAP size is too small for SAB diamond\n");
        return -1;
    }



        av_log(s->avctx, AV_LOG_INFO, "ME_MAP size may be a little small for the selected diamond size\n");
    }



/*FIXME s->no_rounding b_type*/
        else
    }else{
            c->sub_motion_search= hpel_motion_search;
        else
    }
    else

    }else{
        c->stride  = 16*s->mb_width + 32;
        c->uvstride=  8*s->mb_width + 16;
    }

    /* 8x8 fullpel search would need a 4x4 chroma compare, which we do
     * not have yet, and even if we had, the motion estimation code
     * does not expect it. */
            s->mecc.me_cmp[2] = zero_cmp;
            s->mecc.me_sub_cmp[2] = zero_cmp;
    }

    }

    return 0;
}

#define CHECK_SAD_HALF_MV(suffix, x, y) \
{\
    d  = s->mecc.pix_abs[size][(x ? 1 : 0) + (y ? 2 : 0)](NULL, pix, ptr + ((x) >> 1), stride, h); \
    d += (mv_penalty[pen_x + x] + mv_penalty[pen_y + y])*penalty_factor;\
    COPY3_IF_LT(dminh, d, dx, x, dy, y)\
}

                                  int *mx_ptr, int *my_ptr, int dmin,
                                  int src_index, int ref_index,
                                  int size, int h)
{


    }







                }else{
                }
            }else{
                }else{
                }
            }
        }else{
                }else{
                }
            }else{
                }else{
                }
            }
        }

    }else{
    }

}

{


    /* has already been set to the 4 MV if 4MV is done */


    }

/**
 * get fullpel ME search limits.
 */
{
/*
    if(c->avctx->me_range) c->range= c->avctx->me_range >> 1;
    else                   c->range= 16;
*/
        // Search range of H.261 is different from other codec standards
    } else {
    }
    }


}

{



        }



        /* special case for first line */
        } else {


        }
            }




                else
            }else{

                else
            }
        }else

        }else{
        }


    }

        return INT_MAX;

                                      c->scratchpad, stride, 16);
    }

        int dxy;
        int mx, my;
        int offset;

        mx= ff_h263_round_chroma(mx4_sum);
        my= ff_h263_round_chroma(my4_sum);
        dxy = ((my & 1) << 1) | (mx & 1);

        offset= (s->mb_x*8 + (mx>>1)) + (s->mb_y*8 + (my>>1))*s->uvlinesize;

        if(s->no_rounding){
            s->hdsp.put_no_rnd_pixels_tab[1][dxy](c->scratchpad    , s->last_picture.f->data[1] + offset, s->uvlinesize, 8);
            s->hdsp.put_no_rnd_pixels_tab[1][dxy](c->scratchpad + 8, s->last_picture.f->data[2] + offset, s->uvlinesize, 8);
        }else{
            s->hdsp.put_pixels_tab       [1][dxy](c->scratchpad    , s->last_picture.f->data[1] + offset, s->uvlinesize, 8);
            s->hdsp.put_pixels_tab       [1][dxy](c->scratchpad + 8, s->last_picture.f->data[2] + offset, s->uvlinesize, 8);
        }

        dmin_sum += s->mecc.mb_cmp[1](s, s->new_picture.f->data[1] + s->mb_x * 8 + s->mb_y * 8 * s->uvlinesize, c->scratchpad,     s->uvlinesize, 8);
        dmin_sum += s->mecc.mb_cmp[1](s, s->new_picture.f->data[2] + s->mb_x * 8 + s->mb_y * 8 * s->uvlinesize, c->scratchpad + 8, s->uvlinesize, 8);
    }


    /*case FF_CMP_SSE:
        return dmin_sum+ 32*s->qscale*s->qscale;*/
    case FF_CMP_RD:
        return dmin_sum;
    }
}


        c->ref[1+ref_index][1] = c->ref[0+ref_index][1] + s->uvlinesize;
        c->ref[1+ref_index][2] = c->ref[0+ref_index][2] + s->uvlinesize;
        c->src[1][1] = c->src[0][1] + s->uvlinesize;
        c->src[1][2] = c->src[0][2] + s->uvlinesize;
    }

                             int16_t (*mv_tables[2][2])[2], uint8_t *field_select_tables[2], int mx, int my, int user_field_select)
{


        int field_select;
        int best_dmin= INT_MAX;
        int best_field= -1;


                av_assert1(field_select==0 || field_select==1);
                av_assert1(field_select_tables[block][xy]==0 || field_select_tables[block][xy]==1);
                if(field_select_tables[block][xy] != field_select)
                    continue;
            }




            }




                int dxy;

                //FIXME chroma ME
                uint8_t *ref= c->ref[field_select+ref_index][0] + (mx_i>>1) + (my_i>>1)*stride;
                dxy = ((my_i & 1) << 1) | (mx_i & 1);

                if(s->no_rounding){
                    s->hdsp.put_no_rnd_pixels_tab[size][dxy](c->scratchpad, ref    , stride, h);
                }else{
                    s->hdsp.put_pixels_tab       [size][dxy](c->scratchpad, ref    , stride, h);
                }
                dmin = s->mecc.mb_cmp[size](s, c->src[block][0], c->scratchpad, stride, h);
                dmin+= (mv_penalty[mx_i-c->pred_x] + mv_penalty[my_i-c->pred_y] + 1)*c->mb_penalty_factor;
            }else


            }
        }
        {

        }

    }


        return INT_MAX;

    /*case FF_CMP_SSE:
        return dmin_sum+ 32*s->qscale*s->qscale;*/
    case FF_CMP_RD:
        return dmin_sum;
    }
}

    case FF_CMP_SAD:
    case FF_CMP_DCT:
        return (3*lambda)>>(FF_LAMBDA_SHIFT+1);
    case FF_CMP_W53:
        return (4*lambda)>>(FF_LAMBDA_SHIFT);
    case FF_CMP_W97:
        return (2*lambda)>>(FF_LAMBDA_SHIFT);
    case FF_CMP_DCT264:
    case FF_CMP_PSNR:
    case FF_CMP_SSE:
    case FF_CMP_NSSE:
    case FF_CMP_BIT:
    case FF_CMP_MEDIAN_SAD:
        return 1;
    }
}

                                int mb_x, int mb_y)
{





    /* intra / predictive decision */







            } else { /* MPEG-1 at least */
            }
        } else {
        }
    }

    /* At this point (mx,my) are full-pell and the relative displacement */




//        if (varc*2 + 200*256 + 50*(s->lambda2>>FF_LAMBDA_SHIFT) > vard){
        }else{
        }

        }else
        }
    }else{

            dmin= get_mb_score(s, mx, my, 0, 0, 0, 16, 1);

           && !c->skip && varc>50<<8 && vard>10<<8){
            int dmin4= h263_mv4_search(s, mx, my, shift);
            if(dmin4 < dmin){
                mb_type= CANDIDATE_MB_TYPE_INTER4V;
                dmin=dmin4;
            }
        }
            }
        }


        /* get intra luma score */
            intra_score= varc - 500;
        }else{

            }

        }

        }else

        {
        }
    }


int ff_pre_estimate_p_frame_motion(MpegEncContext * s,
                                    int mb_x, int mb_y)
{
    MotionEstContext * const c= &s->me;
    int mx, my, dmin;
    int P[10][2];
    const int shift= 1+s->quarter_sample;
    const int xy= mb_x + mb_y*s->mb_stride;
    init_ref(c, s->new_picture.f->data, s->last_picture.f->data, NULL, 16*mb_x, 16*mb_y, 0);

    av_assert0(s->quarter_sample==0 || s->quarter_sample==1);

    c->pre_penalty_factor    = get_penalty_factor(s->lambda, s->lambda2, c->avctx->me_pre_cmp);
    c->current_mv_penalty= c->mv_penalty[s->f_code] + MAX_DMV;

    get_limits(s, 16*mb_x, 16*mb_y);
    c->skip=0;

    P_LEFT[0]       = s->p_mv_table[xy + 1][0];
    P_LEFT[1]       = s->p_mv_table[xy + 1][1];

    if(P_LEFT[0]       < (c->xmin<<shift)) P_LEFT[0]       = (c->xmin<<shift);

    /* special case for first line */
    if (s->first_slice_line) {
        c->pred_x= P_LEFT[0];
        c->pred_y= P_LEFT[1];
        P_TOP[0]= P_TOPRIGHT[0]= P_MEDIAN[0]=
        P_TOP[1]= P_TOPRIGHT[1]= P_MEDIAN[1]= 0; //FIXME
    } else {
        P_TOP[0]      = s->p_mv_table[xy + s->mb_stride    ][0];
        P_TOP[1]      = s->p_mv_table[xy + s->mb_stride    ][1];
        P_TOPRIGHT[0] = s->p_mv_table[xy + s->mb_stride - 1][0];
        P_TOPRIGHT[1] = s->p_mv_table[xy + s->mb_stride - 1][1];
        if(P_TOP[1]      < (c->ymin<<shift)) P_TOP[1]     = (c->ymin<<shift);
        if(P_TOPRIGHT[0] > (c->xmax<<shift)) P_TOPRIGHT[0]= (c->xmax<<shift);
        if(P_TOPRIGHT[1] < (c->ymin<<shift)) P_TOPRIGHT[1]= (c->ymin<<shift);

        P_MEDIAN[0]= mid_pred(P_LEFT[0], P_TOP[0], P_TOPRIGHT[0]);
        P_MEDIAN[1]= mid_pred(P_LEFT[1], P_TOP[1], P_TOPRIGHT[1]);

        c->pred_x = P_MEDIAN[0];
        c->pred_y = P_MEDIAN[1];
    }

    dmin = ff_epzs_motion_search(s, &mx, &my, P, 0, 0, s->p_mv_table, (1<<16)>>shift, 0, 16);

    s->p_mv_table[xy][0] = mx<<shift;
    s->p_mv_table[xy][1] = my<<shift;

    return dmin;
}

                             int16_t (*mv_table)[2], int ref_index, int f_code)
{





        /* special case for first line */

        }

        }else{
        }

    }



//    s->mb_type[mb_y*s->mb_width + mb_x]= mb_type;

}

                   int motion_fx, int motion_fy,
                   int motion_bx, int motion_by,
                   int pred_fx, int pred_fy,
                   int pred_bx, int pred_by,
                   int size, int h)
{
    //FIXME optimize?
    //FIXME better f_code prediction (max mv & distance)
    //FIXME pointers




    }else{



    }


    //FIXME CHROMA !!!

}

/* refine the bidir vectors in hq mode and return the score in both lq & hq mode*/
{
#define HASH(fx,fy,bx,by) ((fx)+17*(fy)+63*(bx)+117*(by))
#define HASH8(fx,fy,bx,by) ((uint8_t)HASH(fx,fy,bx,by))


                          motion_bx, motion_by,
                          pred_fx, pred_fy,
                          pred_bx, pred_by,
                          0, 16);

{ 0, 0, 0, 1}, { 0, 0, 0,-1}, { 0, 0, 1, 0}, { 0, 0,-1, 0}, { 0, 1, 0, 0}, { 0,-1, 0, 0}, { 1, 0, 0, 0}, {-1, 0, 0, 0},

{ 0, 0, 1, 1}, { 0, 0,-1,-1}, { 0, 1, 1, 0}, { 0,-1,-1, 0}, { 1, 1, 0, 0}, {-1,-1, 0, 0}, { 1, 0, 0, 1}, {-1, 0, 0,-1},
{ 0, 1, 0, 1}, { 0,-1, 0,-1}, { 1, 0, 1, 0}, {-1, 0,-1, 0},
{ 0, 0,-1, 1}, { 0, 0, 1,-1}, { 0,-1, 1, 0}, { 0, 1,-1, 0}, {-1, 1, 0, 0}, { 1,-1, 0, 0}, { 1, 0, 0,-1}, {-1, 0, 0, 1},
{ 0,-1, 0, 1}, { 0, 1, 0,-1}, {-1, 0, 1, 0}, { 1, 0,-1, 0},

{ 0, 1, 1, 1}, { 0,-1,-1,-1}, { 1, 1, 1, 0}, {-1,-1,-1, 0}, { 1, 1, 0, 1}, {-1,-1, 0,-1}, { 1, 0, 1, 1}, {-1, 0,-1,-1},
{ 0,-1, 1, 1}, { 0, 1,-1,-1}, {-1, 1, 1, 0}, { 1,-1,-1, 0}, { 1, 1, 0,-1}, {-1,-1, 0, 1}, { 1, 0,-1, 1}, {-1, 0, 1,-1},
{ 0, 1,-1, 1}, { 0,-1, 1,-1}, { 1,-1, 1, 0}, {-1, 1,-1, 0}, {-1, 1, 0, 1}, { 1,-1, 0,-1}, { 1, 0, 1,-1}, {-1, 0,-1, 1},
{ 0, 1, 1,-1}, { 0,-1,-1, 1}, { 1, 1,-1, 0}, {-1,-1, 1, 0}, { 1,-1, 0, 1}, {-1, 1, 0,-1}, {-1, 0, 1, 1}, { 1, 0,-1,-1},

{ 1, 1, 1, 1}, {-1,-1,-1,-1},
{ 1, 1, 1,-1}, {-1,-1,-1, 1}, { 1, 1,-1, 1}, {-1,-1, 1,-1}, { 1,-1, 1, 1}, {-1, 1,-1,-1}, {-1, 1, 1, 1}, { 1,-1,-1,-1},
{ 1, 1,-1,-1}, {-1,-1, 1, 1}, { 1,-1,-1, 1}, {-1, 1, 1,-1}, { 1,-1, 1,-1}, {-1, 1,-1, 1},
        };
HASH8( 0, 0, 0, 1), HASH8( 0, 0, 0,-1), HASH8( 0, 0, 1, 0), HASH8( 0, 0,-1, 0), HASH8( 0, 1, 0, 0), HASH8( 0,-1, 0, 0), HASH8( 1, 0, 0, 0), HASH8(-1, 0, 0, 0),

HASH8( 0, 0, 1, 1), HASH8( 0, 0,-1,-1), HASH8( 0, 1, 1, 0), HASH8( 0,-1,-1, 0), HASH8( 1, 1, 0, 0), HASH8(-1,-1, 0, 0), HASH8( 1, 0, 0, 1), HASH8(-1, 0, 0,-1),
HASH8( 0, 1, 0, 1), HASH8( 0,-1, 0,-1), HASH8( 1, 0, 1, 0), HASH8(-1, 0,-1, 0),
HASH8( 0, 0,-1, 1), HASH8( 0, 0, 1,-1), HASH8( 0,-1, 1, 0), HASH8( 0, 1,-1, 0), HASH8(-1, 1, 0, 0), HASH8( 1,-1, 0, 0), HASH8( 1, 0, 0,-1), HASH8(-1, 0, 0, 1),
HASH8( 0,-1, 0, 1), HASH8( 0, 1, 0,-1), HASH8(-1, 0, 1, 0), HASH8( 1, 0,-1, 0),

HASH8( 0, 1, 1, 1), HASH8( 0,-1,-1,-1), HASH8( 1, 1, 1, 0), HASH8(-1,-1,-1, 0), HASH8( 1, 1, 0, 1), HASH8(-1,-1, 0,-1), HASH8( 1, 0, 1, 1), HASH8(-1, 0,-1,-1),
HASH8( 0,-1, 1, 1), HASH8( 0, 1,-1,-1), HASH8(-1, 1, 1, 0), HASH8( 1,-1,-1, 0), HASH8( 1, 1, 0,-1), HASH8(-1,-1, 0, 1), HASH8( 1, 0,-1, 1), HASH8(-1, 0, 1,-1),
HASH8( 0, 1,-1, 1), HASH8( 0,-1, 1,-1), HASH8( 1,-1, 1, 0), HASH8(-1, 1,-1, 0), HASH8(-1, 1, 0, 1), HASH8( 1,-1, 0,-1), HASH8( 1, 0, 1,-1), HASH8(-1, 0,-1, 1),
HASH8( 0, 1, 1,-1), HASH8( 0,-1,-1, 1), HASH8( 1, 1,-1, 0), HASH8(-1,-1, 1, 0), HASH8( 1,-1, 0, 1), HASH8(-1, 1, 0,-1), HASH8(-1, 0, 1, 1), HASH8( 1, 0,-1,-1),

HASH8( 1, 1, 1, 1), HASH8(-1,-1,-1,-1),
HASH8( 1, 1, 1,-1), HASH8(-1,-1,-1, 1), HASH8( 1, 1,-1, 1), HASH8(-1,-1, 1,-1), HASH8( 1,-1, 1, 1), HASH8(-1, 1,-1,-1), HASH8(-1, 1, 1, 1), HASH8( 1,-1,-1,-1),
HASH8( 1, 1,-1,-1), HASH8(-1,-1, 1, 1), HASH8( 1,-1,-1, 1), HASH8(-1, 1, 1,-1), HASH8( 1,-1, 1,-1), HASH8(-1, 1,-1, 1),
};

#define CHECK_BIDIR(fx,fy,bx,by)\
    if( !map[(hashidx+HASH(fx,fy,bx,by))&255]\
       &&(fx<=0 || motion_fx+fx<=xmax) && (fy<=0 || motion_fy+fy<=ymax) && (bx<=0 || motion_bx+bx<=xmax) && (by<=0 || motion_by+by<=ymax)\
       &&(fx>=0 || motion_fx+fx>=xmin) && (fy>=0 || motion_fy+fy>=ymin) && (bx>=0 || motion_bx+bx>=xmin) && (by>=0 || motion_by+by>=ymin)){\
        int score;\
        map[(hashidx+HASH(fx,fy,bx,by))&255] = 1;\
        score= check_bidir_mv(s, motion_fx+fx, motion_fy+fy, motion_bx+bx, motion_by+by, pred_fx, pred_fy, pred_bx, pred_by, 0, 16);\
        if(score < fbmin){\
            hashidx += HASH(fx,fy,bx,by);\
            fbmin= score;\
            motion_fx+=fx;\
            motion_fy+=fy;\
            motion_bx+=bx;\
            motion_by+=by;\
            end=0;\
        }\
    }
#define CHECK_BIDIR2(a,b,c,d)\
CHECK_BIDIR(a,b,c,d)\
CHECK_BIDIR(-(a),-(b),-(c),-(d))



                int fx= motion_fx+vect[i][0];
                int fy= motion_fy+vect[i][1];
                int bx= motion_bx+vect[i][2];
                int by= motion_by+vect[i][3];
                if(borderdist<=0){
                    int a= (xmax - FFMAX(fx,bx))|(FFMIN(fx,bx) - xmin);
                    int b= (ymax - FFMAX(fy,by))|(FFMIN(fy,by) - ymin);
                    if((a|b) < 0)
                        map[(hashidx+hash[i])&255] = 1;
                }
                if(!map[(hashidx+hash[i])&255]){
                    int score;
                    map[(hashidx+hash[i])&255] = 1;
                    score= check_bidir_mv(s, fx, fy, bx, by, pred_fx, pred_fy, pred_bx, pred_by, 0, 16);
                    if(score < fbmin){
                        hashidx += hash[i];
                        fbmin= score;
                        motion_fx=fx;
                        motion_fy=fy;
                        motion_bx=bx;
                        motion_by=by;
                        end=0;
                        borderdist--;
                        if(borderdist<=0){
                            int a= FFMIN(xmax - FFMAX(fx,bx), FFMIN(fx,bx) - xmin);
                            int b= FFMIN(ymax - FFMAX(fy,by), FFMIN(fy,by) - ymin);
                            borderdist= FFMIN(a,b);
                        }
                    }
                }
            }
    }


}

{


    }else{
    }


//        c->direct_basis_mv[1][i][0]= c->co_located_mv[i][0]*(time_pb - time_pp)/time_pp + ((i &1)<<(shift+3);
//        c->direct_basis_mv[1][i][1]= c->co_located_mv[i][1]*(time_pb - time_pp)/time_pp + ((i>>1)<<(shift+3);



    }



    }



    /* special case for first line */

    }

    else




}

                             int mb_x, int mb_y)
{





    }

    else
        dmin= INT_MAX;
// FIXME penalty stuff for non-MPEG-4



//FIXME mb type penalty
    }else
        fimin= bimin= INT_MAX;

    {

        }
        }
        }
        }
        }

    }

        }
         //FIXME something smarter
    }

}

/* find best f_code for ME which do unlimited searches */
{



                                     fcode_tab[my + MAX_MV]);


                    }
                }
            }
        }

            }
        }

    }else{
        return 1;
    }
}

{





        /* clip / convert to intra 8x8 type MVs */

                    int block;

                        }
                    }
                }
            }
        }
    }

/**
 * @param truncate 1 for truncation, 0 for using intra
 */
                     int16_t (*mv_table)[2], int f_code, int type, int truncate)
{

    // RAL: 8 in MPEG-1, 16 in MPEG-4



    /* clip / convert to intra 16x16 type MVs */

                        }else{
                        }
                    }
                }
            }
        }
    }
