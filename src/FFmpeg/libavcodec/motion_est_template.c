/*
 * Motion estimation
 * Copyright (c) 2002-2004 Michael Niedermayer
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
 * Motion estimation template.
 */

#include "libavutil/qsort.h"
#include "mpegvideo.h"

//Let us hope gcc will remove the unused vars ...(gcc 3.2.2 seems to do it ...)
#define LOAD_COMMON\
    uint32_t av_unused * const score_map= c->score_map;\
    const int av_unused xmin= c->xmin;\
    const int av_unused ymin= c->ymin;\
    const int av_unused xmax= c->xmax;\
    const int av_unused ymax= c->ymax;\
    uint8_t *mv_penalty= c->current_mv_penalty;\
    const int pred_x= c->pred_x;\
    const int pred_y= c->pred_y;\

#define CHECK_HALF_MV(dx, dy, x, y)\
{\
    const int hx= 2*(x)+(dx);\
    const int hy= 2*(y)+(dy);\
    d= cmp_hpel(s, x, y, dx, dy, size, h, ref_index, src_index, cmp_sub, chroma_cmp_sub, flags);\
    d += (mv_penalty[hx - pred_x] + mv_penalty[hy - pred_y])*penalty_factor;\
    COPY3_IF_LT(dmin, d, bx, hx, by, hy)\
}

                                  int *mx_ptr, int *my_ptr, int dmin,
                                  int src_index, int ref_index,
                                  int size, int h)
{


 //FIXME factorize


    }

    }


#if defined(ASSERT_LEVEL) && ASSERT_LEVEL > 1
        unsigned key;
        unsigned map_generation= c->map_generation;
        key= ((my-1)<<ME_MAP_MV_BITS) + (mx) + map_generation;
        av_assert2(c->map[(index-(1<<ME_MAP_SHIFT))&(ME_MAP_SIZE-1)] == key);
        key= ((my+1)<<ME_MAP_MV_BITS) + (mx) + map_generation;
        av_assert2(c->map[(index+(1<<ME_MAP_SHIFT))&(ME_MAP_SIZE-1)] == key);
        key= ((my)<<ME_MAP_MV_BITS) + (mx+1) + map_generation;
        av_assert2(c->map[(index+1)&(ME_MAP_SIZE-1)] == key);
        key= ((my)<<ME_MAP_MV_BITS) + (mx-1) + map_generation;
        av_assert2(c->map[(index-1)&(ME_MAP_SIZE-1)] == key);
#endif
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
    }


}

          int *mx_ptr, int *my_ptr, int dmin,
                                  int src_index, int ref_index,
                                  int size, int h)
{
}

                               int src_index, int ref_index, int size,
                               int h, int add_rate)
{


 //FIXME factorize


    //FIXME check cbp before adding penalty for (0,0) vector

}

                    int ref_index, int size, int h, int add_rate)
{
}

#define CHECK_QUARTER_MV(dx, dy, x, y)\
{\
    const int hx= 4*(x)+(dx);\
    const int hy= 4*(y)+(dy);\
    d= cmp_qpel(s, x, y, dx, dy, size, h, ref_index, src_index, cmpf, chroma_cmpf, flags);\
    d += (mv_penalty[hx - pred_x] + mv_penalty[hy - pred_y])*penalty_factor;\
    COPY3_IF_LT(dmin, d, bx, hx, by, hy)\
}

                                  int *mx_ptr, int *my_ptr, int dmin,
                                  int src_index, int ref_index,
                                  int size, int h)
{


 //FIXME factorize


    }

    }



                    //FIXME this could overflow (unlikely though)



//                    if(nx&1) score-=1024*c->penalty_factor;
//                    if(ny&1) score-=1024*c->penalty_factor;

                        }
                    }
                }
            }
        }else{
            //FIXME this could overflow (unlikely though)

            }else{
            }



                    //FIXME this could overflow (unlikely though)


//                    if(nx&1) score-=32*c->penalty_factor;
  //                  if(ny&1) score-=32*c->penalty_factor;

                        }
                    }
                }
            }
        }
        }


    }else{
    }

    return dmin;
}


#define CHECK_MV(x,y)\
{\
    const unsigned key = ((unsigned)(y)<<ME_MAP_MV_BITS) + (x) + map_generation;\
    const int index= (((unsigned)(y)<<ME_MAP_SHIFT) + (x))&(ME_MAP_SIZE-1);\
    av_assert2((x) >= xmin);\
    av_assert2((x) <= xmax);\
    av_assert2((y) >= ymin);\
    av_assert2((y) <= ymax);\
    if(map[index]!=key){\
        d= cmp(s, x, y, 0, 0, size, h, ref_index, src_index, cmpf, chroma_cmpf, flags);\
        map[index]= key;\
        score_map[index]= d;\
        d += (mv_penalty[((x)*(1<<shift))-pred_x] + mv_penalty[((y)*(1<<shift))-pred_y])*penalty_factor;\
        COPY3_IF_LT(dmin, d, best[0], x, best[1], y)\
    }\
}

#define CHECK_CLIPPED_MV(ax,ay)\
{\
    const int Lx= ax;\
    const int Ly= ay;\
    const int Lx2= FFMAX(xmin, FFMIN(Lx, xmax));\
    const int Ly2= FFMAX(ymin, FFMIN(Ly, ymax));\
    CHECK_MV(Lx2, Ly2)\
}

#define CHECK_MV_DIR(x,y,new_dir)\
{\
    const unsigned key = ((unsigned)(y)<<ME_MAP_MV_BITS) + (x) + map_generation;\
    const int index= (((unsigned)(y)<<ME_MAP_SHIFT) + (x))&(ME_MAP_SIZE-1);\
    if(map[index]!=key){\
        d= cmp(s, x, y, 0, 0, size, h, ref_index, src_index, cmpf, chroma_cmpf, flags);\
        map[index]= key;\
        score_map[index]= d;\
        d += (mv_penalty[(int)((unsigned)(x)<<shift)-pred_x] + mv_penalty[(int)((unsigned)(y)<<shift)-pred_y])*penalty_factor;\
        if(d<dmin){\
            best[0]=x;\
            best[1]=y;\
            dmin=d;\
            next_dir= new_dir;\
        }\
    }\
}

#define check(x,y,S,v)\
if( (x)<(xmin<<(S)) ) av_log(NULL, AV_LOG_ERROR, "%d %d %d %d %d xmin" #v, xmin, (x), (y), s->mb_x, s->mb_y);\
if( (x)>(xmax<<(S)) ) av_log(NULL, AV_LOG_ERROR, "%d %d %d %d %d xmax" #v, xmax, (x), (y), s->mb_x, s->mb_y);\
if( (y)<(ymin<<(S)) ) av_log(NULL, AV_LOG_ERROR, "%d %d %d %d %d ymin" #v, ymin, (x), (y), s->mb_x, s->mb_y);\
if( (y)>(ymax<<(S)) ) av_log(NULL, AV_LOG_ERROR, "%d %d %d %d %d ymax" #v, ymax, (x), (y), s->mb_x, s->mb_y);\

#define LOAD_COMMON2\
    uint32_t *map= c->map;\
    const int qpel= flags&FLAG_QPEL;\
    const int shift= 1+qpel;\

                                       int src_index, int ref_index, const int penalty_factor,
                                       int size, int h, int flags)
{


    { /* ensure that the best point is in the MAP as h/qpel refinement needs it */
        }
    }



            return dmin;
        }
    }
}

static int funny_diamond_search(MpegEncContext * s, int *best, int dmin,
                                       int src_index, int ref_index, const int penalty_factor,
                                       int size, int h, int flags)
{
    MotionEstContext * const c= &s->me;
    me_cmp_func cmpf, chroma_cmpf;
    int dia_size;
    LOAD_COMMON
    LOAD_COMMON2
    unsigned map_generation = c->map_generation;

    cmpf        = s->mecc.me_cmp[size];
    chroma_cmpf = s->mecc.me_cmp[size + 1];

    for(dia_size=1; dia_size<=4; dia_size++){
        int dir;
        const int x= best[0];
        const int y= best[1];

        if(dia_size&(dia_size-1)) continue;

        if(   x + dia_size > xmax
           || x - dia_size < xmin
           || y + dia_size > ymax
           || y - dia_size < ymin)
           continue;

        for(dir= 0; dir<dia_size; dir+=2){
            int d;

            CHECK_MV(x + dir           , y + dia_size - dir);
            CHECK_MV(x + dia_size - dir, y - dir           );
            CHECK_MV(x - dir           , y - dia_size + dir);
            CHECK_MV(x - dia_size + dir, y + dir           );
        }

        if(x!=best[0] || y!=best[1])
            dia_size=0;
    }
    return dmin;
}

static int hex_search(MpegEncContext * s, int *best, int dmin,
                                       int src_index, int ref_index, const int penalty_factor,
                                       int size, int h, int flags, int dia_size)
{
    MotionEstContext * const c= &s->me;
    me_cmp_func cmpf, chroma_cmpf;
    LOAD_COMMON
    LOAD_COMMON2
    unsigned map_generation = c->map_generation;
    int x,y,d;
    const int dec= dia_size & (dia_size-1);

    cmpf        = s->mecc.me_cmp[size];
    chroma_cmpf = s->mecc.me_cmp[size + 1];

    for(;dia_size; dia_size= dec ? dia_size-1 : dia_size>>1){
        do{
            x= best[0];
            y= best[1];

            CHECK_CLIPPED_MV(x  -dia_size    , y);
            CHECK_CLIPPED_MV(x+  dia_size    , y);
            CHECK_CLIPPED_MV(x+( dia_size>>1), y+dia_size);
            CHECK_CLIPPED_MV(x+( dia_size>>1), y-dia_size);
            if(dia_size>1){
                CHECK_CLIPPED_MV(x+(-dia_size>>1), y+dia_size);
                CHECK_CLIPPED_MV(x+(-dia_size>>1), y-dia_size);
            }
        }while(best[0] != x || best[1] != y);
    }

    return dmin;
}

static int l2s_dia_search(MpegEncContext * s, int *best, int dmin,
                                       int src_index, int ref_index, const int penalty_factor,
                                       int size, int h, int flags)
{
    MotionEstContext * const c= &s->me;
    me_cmp_func cmpf, chroma_cmpf;
    LOAD_COMMON
    LOAD_COMMON2
    unsigned map_generation = c->map_generation;
    int x,y,i,d;
    int dia_size= c->dia_size&0xFF;
    const int dec= dia_size & (dia_size-1);
    static const int hex[8][2]={{-2, 0}, {-1,-1}, { 0,-2}, { 1,-1},
                                { 2, 0}, { 1, 1}, { 0, 2}, {-1, 1}};

    cmpf        = s->mecc.me_cmp[size];
    chroma_cmpf = s->mecc.me_cmp[size + 1];

    for(; dia_size; dia_size= dec ? dia_size-1 : dia_size>>1){
        do{
            x= best[0];
            y= best[1];
            for(i=0; i<8; i++){
                CHECK_CLIPPED_MV(x+hex[i][0]*dia_size, y+hex[i][1]*dia_size);
            }
        }while(best[0] != x || best[1] != y);
    }

    x= best[0];
    y= best[1];
    CHECK_CLIPPED_MV(x+1, y);
    CHECK_CLIPPED_MV(x, y+1);
    CHECK_CLIPPED_MV(x-1, y);
    CHECK_CLIPPED_MV(x, y-1);

    return dmin;
}

static int umh_search(MpegEncContext * s, int *best, int dmin,
                                       int src_index, int ref_index, const int penalty_factor,
                                       int size, int h, int flags)
{
    MotionEstContext * const c= &s->me;
    me_cmp_func cmpf, chroma_cmpf;
    LOAD_COMMON
    LOAD_COMMON2
    unsigned map_generation = c->map_generation;
    int x,y,x2,y2, i, j, d;
    const int dia_size= c->dia_size&0xFE;
    static const int hex[16][2]={{-4,-2}, {-4,-1}, {-4, 0}, {-4, 1}, {-4, 2},
                                 { 4,-2}, { 4,-1}, { 4, 0}, { 4, 1}, { 4, 2},
                                 {-2, 3}, { 0, 4}, { 2, 3},
                                 {-2,-3}, { 0,-4}, { 2,-3},};

    cmpf        = s->mecc.me_cmp[size];
    chroma_cmpf = s->mecc.me_cmp[size + 1];

    x= best[0];
    y= best[1];
    for(x2=FFMAX(x-dia_size+1, xmin); x2<=FFMIN(x+dia_size-1,xmax); x2+=2){
        CHECK_MV(x2, y);
    }
    for(y2=FFMAX(y-dia_size/2+1, ymin); y2<=FFMIN(y+dia_size/2-1,ymax); y2+=2){
        CHECK_MV(x, y2);
    }

    x= best[0];
    y= best[1];
    for(y2=FFMAX(y-2, ymin); y2<=FFMIN(y+2,ymax); y2++){
        for(x2=FFMAX(x-2, xmin); x2<=FFMIN(x+2,xmax); x2++){
            CHECK_MV(x2, y2);
        }
    }

//FIXME prevent the CLIP stuff

    for(j=1; j<=dia_size/4; j++){
        for(i=0; i<16; i++){
            CHECK_CLIPPED_MV(x+hex[i][0]*j, y+hex[i][1]*j);
        }
    }

    return hex_search(s, best, dmin, src_index, ref_index, penalty_factor, size, h, flags, 2);
}

static int full_search(MpegEncContext * s, int *best, int dmin,
                                       int src_index, int ref_index, const int penalty_factor,
                                       int size, int h, int flags)
{
    MotionEstContext * const c= &s->me;
    me_cmp_func cmpf, chroma_cmpf;
    LOAD_COMMON
    LOAD_COMMON2
    unsigned map_generation = c->map_generation;
    int x,y, d;
    const int dia_size= c->dia_size&0xFF;

    cmpf        = s->mecc.me_cmp[size];
    chroma_cmpf = s->mecc.me_cmp[size + 1];

    for(y=FFMAX(-dia_size, ymin); y<=FFMIN(dia_size,ymax); y++){
        for(x=FFMAX(-dia_size, xmin); x<=FFMIN(dia_size,xmax); x++){
            CHECK_MV(x, y);
        }
    }

    x= best[0];
    y= best[1];
    d= dmin;
    CHECK_CLIPPED_MV(x  , y);
    CHECK_CLIPPED_MV(x+1, y);
    CHECK_CLIPPED_MV(x, y+1);
    CHECK_CLIPPED_MV(x-1, y);
    CHECK_CLIPPED_MV(x, y-1);
    best[0]= x;
    best[1]= y;

    return d;
}

#define SAB_CHECK_MV(ax,ay)\
{\
    const unsigned key = ((ay)<<ME_MAP_MV_BITS) + (ax) + map_generation;\
    const int index= (((ay)<<ME_MAP_SHIFT) + (ax))&(ME_MAP_SIZE-1);\
    if(map[index]!=key){\
        d= cmp(s, ax, ay, 0, 0, size, h, ref_index, src_index, cmpf, chroma_cmpf, flags);\
        map[index]= key;\
        score_map[index]= d;\
        d += (mv_penalty[((ax)<<shift)-pred_x] + mv_penalty[((ay)<<shift)-pred_y])*penalty_factor;\
        if(d < minima[minima_count-1].height){\
            int j=0;\
            \
            while(d >= minima[j].height) j++;\
\
            memmove(&minima [j+1], &minima [j], (minima_count - j - 1)*sizeof(Minima));\
\
            minima[j].checked= 0;\
            minima[j].height= d;\
            minima[j].x= ax;\
            minima[j].y= ay;\
            \
            i=-1;\
            continue;\
        }\
    }\
}

#define MAX_SAB_SIZE ME_MAP_SIZE
static int sab_diamond_search(MpegEncContext * s, int *best, int dmin,
                                       int src_index, int ref_index, const int penalty_factor,
                                       int size, int h, int flags)
{
    MotionEstContext * const c= &s->me;
    me_cmp_func cmpf, chroma_cmpf;
    Minima minima[MAX_SAB_SIZE];
    const int minima_count= FFABS(c->dia_size);
    int i, j;
    LOAD_COMMON
    LOAD_COMMON2
    unsigned map_generation = c->map_generation;

    av_assert1(minima_count <= MAX_SAB_SIZE);

    cmpf        = s->mecc.me_cmp[size];
    chroma_cmpf = s->mecc.me_cmp[size + 1];

    /*Note j<MAX_SAB_SIZE is needed if MAX_SAB_SIZE < ME_MAP_SIZE as j can
      become larger due to MVs overflowing their ME_MAP_MV_BITS bits space in map
     */
    for(j=i=0; i<ME_MAP_SIZE && j<MAX_SAB_SIZE; i++){
        uint32_t key= map[i];

        key += (1<<(ME_MAP_MV_BITS-1)) + (1<<(2*ME_MAP_MV_BITS-1));

        if ((key & (-(1 << (2 * ME_MAP_MV_BITS)))) != map_generation)
            continue;

        minima[j].height= score_map[i];
        minima[j].x= key & ((1<<ME_MAP_MV_BITS)-1); key>>=ME_MAP_MV_BITS;
        minima[j].y= key & ((1<<ME_MAP_MV_BITS)-1);
        minima[j].x-= (1<<(ME_MAP_MV_BITS-1));
        minima[j].y-= (1<<(ME_MAP_MV_BITS-1));

        // all entries in map should be in range except if the mv overflows their ME_MAP_MV_BITS bits space
        if(   minima[j].x > xmax || minima[j].x < xmin
           || minima[j].y > ymax || minima[j].y < ymin)
            continue;

        minima[j].checked=0;
        if(minima[j].x || minima[j].y)
            minima[j].height+= (mv_penalty[((minima[j].x)<<shift)-pred_x] + mv_penalty[((minima[j].y)<<shift)-pred_y])*penalty_factor;

        j++;
    }

    AV_QSORT(minima, j, Minima, minima_cmp);

    for(; j<minima_count; j++){
        minima[j].height=256*256*256*64;
        minima[j].checked=0;
        minima[j].x= minima[j].y=0;
    }

    for(i=0; i<minima_count; i++){
        const int x= minima[i].x;
        const int y= minima[i].y;
        int d;

        if(minima[i].checked) continue;

        if(   x >= xmax || x <= xmin
           || y >= ymax || y <= ymin)
           continue;

        SAB_CHECK_MV(x-1, y)
        SAB_CHECK_MV(x+1, y)
        SAB_CHECK_MV(x  , y-1)
        SAB_CHECK_MV(x  , y+1)

        minima[i].checked= 1;
    }

    best[0]= minima[0].x;
    best[1]= minima[0].y;
    dmin= minima[0].height;

    if(   best[0] < xmax && best[0] > xmin
       && best[1] < ymax && best[1] > ymin){
        int d;
        // ensure that the reference samples for hpel refinement are in the map
        CHECK_MV(best[0]-1, best[1])
        CHECK_MV(best[0]+1, best[1])
        CHECK_MV(best[0], best[1]-1)
        CHECK_MV(best[0], best[1]+1)
    }
    return dmin;
}

                                       int src_index, int ref_index, const int penalty_factor,
                                       int size, int h, int flags)
{




//check(x + dir,y + dia_size - dir,0, a0)
        }


//check(x + dia_size - dir, y - dir,0, a1)
        }


//check(x - dir,y - dia_size + dir,0, a2)
        }


//check(x - dia_size + dir, y + dir,0, a3)
        }

    }
}

                                       int src_index, int ref_index, const int penalty_factor,
                                       int size, int h, int flags){
        return funny_diamond_search(s, best, dmin, src_index, ref_index, penalty_factor, size, h, flags);
        return   sab_diamond_search(s, best, dmin, src_index, ref_index, penalty_factor, size, h, flags);
        return          full_search(s, best, dmin, src_index, ref_index, penalty_factor, size, h, flags);
        return           umh_search(s, best, dmin, src_index, ref_index, penalty_factor, size, h, flags);
        return           hex_search(s, best, dmin, src_index, ref_index, penalty_factor, size, h, flags, c->dia_size&0xFF);
        return       l2s_dia_search(s, best, dmin, src_index, ref_index, penalty_factor, size, h, flags);
    else
}

/**
   @param P a list of candidate mvs to check before starting the
   iterative search. If one of the candidates is close to the optimal mv, then
   it takes fewer iterations. And it increases the chance that we find the
   optimal mv.
 */
                             int P[10][2], int src_index, int ref_index, int16_t (*last_mv)[2],
                             int ref_mv_scale, int flags, int size, int h)
{
                               i.e. the difference between the position of the
                               block currently being encoded and the position of
                               the block chosen to predict it from. */
                               corresponding to the mv stored in best[]. */


        penalty_factor= c->pre_penalty_factor;
        cmpf           = s->mecc.me_pre_cmp[size];
        chroma_cmpf    = s->mecc.me_pre_cmp[size + 1];
    }else{
    }



    //FIXME precalc first term below?

    /* first line */
                        (last_mv[ref_mv_xy][1]*ref_mv_scale + (1<<15))>>16)
    }else{
        }
                        (last_mv[ref_mv_xy][1]*ref_mv_scale + (1<<15))>>16)
    }
            CHECK_CLIPPED_MV((last_mv[ref_mv_xy-1][0]*ref_mv_scale + (1<<15))>>16,
                            (last_mv[ref_mv_xy-1][1]*ref_mv_scale + (1<<15))>>16)
            if(!s->first_slice_line)
                CHECK_CLIPPED_MV((last_mv[ref_mv_xy-ref_mv_stride][0]*ref_mv_scale + (1<<15))>>16,
                                (last_mv[ref_mv_xy-ref_mv_stride][1]*ref_mv_scale + (1<<15))>>16)
        }else{
                            (last_mv[ref_mv_xy+1][1]*ref_mv_scale + (1<<15))>>16)
                                (last_mv[ref_mv_xy+ref_mv_stride][1]*ref_mv_scale + (1<<15))>>16)
        }
    }

        const int count= c->avctx->last_predictor_count;
        const int xstart= FFMAX(0, s->mb_x - count);
        const int ystart= FFMAX(0, s->mb_y - count);
        const int xend= FFMIN(s->mb_width , s->mb_x + count + 1);
        const int yend= FFMIN(s->mb_height, s->mb_y + count + 1);
        int mb_y;

        for(mb_y=ystart; mb_y<yend; mb_y++){
            int mb_x;
            for(mb_x=xstart; mb_x<xend; mb_x++){
                const int xy= mb_x + 1 + (mb_y + 1)*ref_mv_stride;
                int mx= (last_mv[xy][0]*ref_mv_scale + (1<<15))>>16;
                int my= (last_mv[xy][1]*ref_mv_scale + (1<<15))>>16;

                if(mx>xmax || mx<xmin || my>ymax || my<ymin) continue;
                CHECK_MV(mx,my)
            }
        }
    }

//check(best[0],best[1],0, b0)

//check(best[0],best[1],0, b1)

}

//this function is dedicated to the brain damaged gcc
                          int P[10][2], int src_index, int ref_index,
                          int16_t (*last_mv)[2], int ref_mv_scale,
                          int size, int h)
{
//FIXME convert other functions in the same way if faster
//    case FLAG_QPEL:
//        return epzs_motion_search_internal(s, mx_ptr, my_ptr, P, src_index, ref_index, last_mv, ref_mv_scale, FLAG_QPEL);
    }else{
    }
}

                             int *mx_ptr, int *my_ptr, int P[10][2],
                             int src_index, int ref_index, int16_t (*last_mv)[2],
                             int ref_mv_scale, const int size)
{




    /* first line */
                        (last_mv[ref_mv_xy][1]*ref_mv_scale + (1<<15))>>16)
    }else{
        //FIXME try some early stop
                        (last_mv[ref_mv_xy][1]*ref_mv_scale + (1<<15))>>16)
    }
                        (last_mv[ref_mv_xy+1][1]*ref_mv_scale + (1<<15))>>16)
                            (last_mv[ref_mv_xy+ref_mv_stride][1]*ref_mv_scale + (1<<15))>>16)
    }



}
