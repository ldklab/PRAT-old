/*
 * ITU H.263 bitstream decoder
 * Copyright (c) 2000,2001 Fabrice Bellard
 * H.263+ support.
 * Copyright (c) 2001 Juan J. Sierralta P
 * Copyright (c) 2002-2004 Michael Niedermayer <michaelni@gmx.at>
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
 * H.263 decoder.
 */

#define UNCHECKED_BITSTREAM_READER 1
#include <limits.h>

#include "libavutil/attributes.h"
#include "libavutil/imgutils.h"
#include "libavutil/internal.h"
#include "libavutil/mathematics.h"
#include "avcodec.h"
#include "mpegvideo.h"
#include "h263.h"
#include "h263data.h"
#include "internal.h"
#include "mathops.h"
#include "mpegutils.h"
#include "unary.h"
#include "flv.h"
#include "rv10.h"
#include "mpeg4video.h"
#include "mpegvideodata.h"

// The defines below define the number of bits that are read at once for
// reading vlc values. Changing these may improve speed and data cache needs
// be aware though that decreasing them may need the number of stages that is
// passed to get_vlc* to be increased.
#define MV_VLC_BITS 9
#define H263_MBTYPE_B_VLC_BITS 6
#define CBPC_B_VLC_BITS 3

static const int h263_mb_type_b_map[15]= {
    MB_TYPE_DIRECT2 | MB_TYPE_L0L1,
    MB_TYPE_DIRECT2 | MB_TYPE_L0L1 | MB_TYPE_CBP,
    MB_TYPE_DIRECT2 | MB_TYPE_L0L1 | MB_TYPE_CBP | MB_TYPE_QUANT,
                      MB_TYPE_L0                                 | MB_TYPE_16x16,
                      MB_TYPE_L0   | MB_TYPE_CBP                 | MB_TYPE_16x16,
                      MB_TYPE_L0   | MB_TYPE_CBP | MB_TYPE_QUANT | MB_TYPE_16x16,
                      MB_TYPE_L1                                 | MB_TYPE_16x16,
                      MB_TYPE_L1   | MB_TYPE_CBP                 | MB_TYPE_16x16,
                      MB_TYPE_L1   | MB_TYPE_CBP | MB_TYPE_QUANT | MB_TYPE_16x16,
                      MB_TYPE_L0L1                               | MB_TYPE_16x16,
                      MB_TYPE_L0L1 | MB_TYPE_CBP                 | MB_TYPE_16x16,
                      MB_TYPE_L0L1 | MB_TYPE_CBP | MB_TYPE_QUANT | MB_TYPE_16x16,
    0, //stuffing
    MB_TYPE_INTRA4x4                | MB_TYPE_CBP,
    MB_TYPE_INTRA4x4                | MB_TYPE_CBP | MB_TYPE_QUANT,
};

    av_log(s->avctx, AV_LOG_DEBUG, "qp:%d %c size:%d rnd:%d%s%s%s%s%s%s%s%s%s %d/%d\n",
         s->qscale, av_get_picture_type_char(s->pict_type),
         s->gb.size_in_bits, 1-s->no_rounding,
         s->obmc ? " AP" : "",
         s->umvplus ? " UMV" : "",
         s->h263_long_vectors ? " LONG" : "",
         s->h263_plus ? " +" : "",
         s->h263_aic ? " AIC" : "",
         s->alt_inter_vlc ? " AIV" : "",
         s->modified_quant ? " MQ" : "",
         s->loop_filter ? " LOOP" : "",
         s->h263_slice_structured ? " SS" : "",
         s->avctx->framerate.num, s->avctx->framerate.den
    );
    }

/***********************************************/
/* decoding */

VLC ff_h263_intra_MCBPC_vlc;
VLC ff_h263_inter_MCBPC_vlc;
VLC ff_h263_cbpy_vlc;
static VLC mv_vlc;
static VLC h263_mbtype_b_vlc;
static VLC cbpc_b_vlc;

/* init vlcs */

/* XXX: find a better solution to handle static init */
{

                 ff_h263_intra_MCBPC_bits, 1, 1,
                 ff_h263_intra_MCBPC_code, 1, 1, 72);
                 ff_h263_inter_MCBPC_bits, 1, 1,
                 ff_h263_inter_MCBPC_code, 1, 1, 198);
                 &ff_h263_cbpy_tab[0][1], 2, 1,
                 &ff_h263_cbpy_tab[0][0], 2, 1, 64);
                 &ff_mvtab[0][1], 2, 1,
                 &ff_mvtab[0][0], 2, 1, 538);
                 &ff_h263_mbtype_b_tab[0][1], 2, 1,
                 &ff_h263_mbtype_b_tab[0][0], 2, 1, 80);
                 &ff_cbpc_b_tab[0][1], 2, 1,
                 &ff_cbpc_b_tab[0][0], 2, 1, 8);
    }

{

            break;

}

/**
 * Decode the group of blocks header or slice header.
 * @return <0 if an error occurred
 */
{

    /* Check for GOB Start Code */
        return -1;

        /* We have a GBSC probably with GSTUFF */
    //MN: we must check the bits left or we might end in an infinite loop (or segfault)
    }
        return -1;

        if(check_marker(s->avctx, &s->gb, "before MBA")==0)
            return -1;

        ff_h263_decode_mba(s);

        if(s->mb_num > 1583)
            if(check_marker(s->avctx, &s->gb, "after MBA")==0)
                return -1;

        s->qscale = get_bits(&s->gb, 5); /* SQUANT */
        if(check_marker(s->avctx, &s->gb, "after SQUANT")==0)
            return -1;
        skip_bits(&s->gb, 2); /* GFID */
    }else{
    }

        return -1;

        return -1;

    return 0;
}

/**
 * Decode the group of blocks / video packet header / slice header (MPEG-4 Studio).
 * @return bit position of the resync_marker, or <0 if none was found
 */

    /* In MPEG-4 studio mode look for a new slice startcode
     * and decode slice header */
        align_get_bits(&s->gb);

        while (get_bits_left(&s->gb) >= 32 && show_bits_long(&s->gb, 32) != SLICE_START_CODE) {
            get_bits(&s->gb, 8);
        }

        if (get_bits_left(&s->gb) >= 32 && show_bits_long(&s->gb, 32) == SLICE_START_CODE)
            return get_bits_count(&s->gb);
        else
            return -1;
    }

    }

        else
            return pos;
    }
    //OK, it's not where it is supposed to be ...
    s->gb= s->last_resync_gb;
    align_get_bits(&s->gb);
    left= get_bits_left(&s->gb);

    for(;left>16+1+5+5; left-=8){
        if(show_bits(&s->gb, 16)==0){
            GetBitContext bak= s->gb;

            pos= get_bits_count(&s->gb);
            if(CONFIG_MPEG4_DECODER && s->codec_id==AV_CODEC_ID_MPEG4)
                ret= ff_mpeg4_decode_video_packet_header(s->avctx->priv_data);
            else
                ret= h263_decode_gob_header(s);
            if(ret>=0)
                return pos;

            s->gb= bak;
        }
        skip_bits(&s->gb, 8);
    }

    return -1;
}

{

        return pred;
        return 0xffff;

    }

    /* modulo decoding */
    } else {
        /* horrible H.263 long vector mode */
        if (pred < -31 && val < -63)
            val += 64;
        if (pred > 32 && val > 63)
            val -= 64;

    }
    return val;
}


/* Decode RVLC of H.263+ UMV */
{

      return pred;


   {
          avpriv_request_sample(s->avctx, "Huge DMV");
          return 0xffff;
      }
   }

   ff_tlog(s->avctx,"H.263+ UMV Motion = %d\n", code);
   return code;

}

/**
 * read the next MVs for OBMC. yes this is an ugly hack, feel free to send a patch :)
 */




            /* skip mb */

        }

    }else{
            if(s->modified_quant){
                if(get_bits1(&s->gb)) skip_bits(&s->gb, 1);
                else                  skip_bits(&s->gb, 5);
            }else
                skip_bits(&s->gb, 2);
        }

                /* 16x16 motion prediction */
                    mx = h263p_decode_umotion(s, pred_x);
                else

                    my = h263p_decode_umotion(s, pred_y);
                else

        } else {
            s->current_picture.mb_type[xy] = MB_TYPE_8x8 | MB_TYPE_L0;
            for(i=0;i<4;i++) {
                mot_val = ff_h263_pred_motion(s, i, 0, &pred_x, &pred_y);
                if (s->umvplus)
                    mx = h263p_decode_umotion(s, pred_x);
                else
                    mx = ff_h263_decode_motion(s, pred_x, 1);

                if (s->umvplus)
                    my = h263p_decode_umotion(s, pred_y);
                else
                    my = ff_h263_decode_motion(s, pred_y, 1);
                if (s->umvplus && (mx - pred_x) == 1 && (my - pred_y) == 1)
                    skip_bits1(&s->gb); /* Bit stuffing to prevent PSC */
                mot_val[0] = mx;
                mot_val[1] = my;
            }
        }
    }
end:



static void h263_decode_dquant(MpegEncContext *s){
    static const int8_t quant_tab[4] = { -1, -2, 1, 2 };

    if(s->modified_quant){
        if(get_bits1(&s->gb))
            s->qscale= ff_modified_quant_tab[get_bits1(&s->gb)][ s->qscale ];
        else
            s->qscale= get_bits(&s->gb, 5);
    }else
        s->qscale += quant_tab[get_bits(&s->gb, 2)];
    ff_set_qscale(s, s->qscale);
}

                             int n, int coded)
{

            else
        }
        /* DC coef */
            int component, diff;
            component = (n <= 3 ? 0 : n - 4 + 1);
            level = s->last_dc[component];
            if (s->rv10_first_dc_coded[component]) {
                diff = ff_rv_decode_dc(s, n);
                if (diff == 0xffff)
                    return -1;
                level += diff;
                level = level & 0xff; /* handle wrap round */
                s->last_dc[component] = level;
            } else {
                s->rv10_first_dc_coded[component] = 1;
            }
          } else {
          }
        }else{
                av_log(s->avctx, AV_LOG_ERROR, "illegal dc %d at %d %d\n", level, s->mb_x, s->mb_y);
                if (s->avctx->err_recognition & (AV_EF_BITSTREAM|AV_EF_COMPLIANT))
                    return -1;
            }
        }
    } else {
        i = 0;
    }
    }
    {
                CLOSE_READER(re, &s->gb);
                av_log(s->avctx, AV_LOG_ERROR, "illegal ac vlc code at %dx%d\n", s->mb_x, s->mb_y);
                return -1;
            }
            /* escape */
                    SKIP_COUNTER(re, &s->gb, 1 + 7);
                    UPDATE_CACHE(re, &s->gb);
                    level = SHOW_SBITS(re, &s->gb, 11);
                    SKIP_COUNTER(re, &s->gb, 11);
                } else {
                }
            } else {
                        /* XXX: should patch encoder too */
                        level = SHOW_SBITS(re, &s->gb, 12);
                        SKIP_COUNTER(re, &s->gb, 12);
                    }else{
                    }
                }
            }
        } else {
        }
            // redo update without last flag, revert -1 offset
                // only last marker, no overrun
            }
                //Looks like a hack but no, it's the way it is supposed to work ...
            }
            av_log(s->avctx, AV_LOG_ERROR, "run overflow at %dx%d i:%d\n", s->mb_x, s->mb_y, s->mb_intra);
            return -1;
        }
    }
    }
    }
}

static int h263_skip_b_part(MpegEncContext *s, int cbp)
{
    LOCAL_ALIGNED_32(int16_t, dblock, [64]);
    int i, mbi;
    int bli[6];

    /* we have to set s->mb_intra to zero to decode B-part of PB-frame correctly
     * but real value should be restored in order to be used later (in OBMC condition)
     */
    mbi = s->mb_intra;
    memcpy(bli, s->block_last_index, sizeof(bli));
    s->mb_intra = 0;
    for (i = 0; i < 6; i++) {
        if (h263_decode_block(s, dblock, i, cbp&32) < 0)
            return -1;
        cbp+=cbp;
    }
    s->mb_intra = mbi;
    memcpy(s->block_last_index, bli, sizeof(bli));
    return 0;
}

static int h263_get_modb(GetBitContext *gb, int pb_frame, int *cbpb)
{
    int c, mv = 1;

    if (pb_frame < 3) { // h.263 Annex G and i263 PB-frame
        c = get_bits1(gb);
        if (pb_frame == 2 && c)
            mv = !get_bits1(gb);
    } else { // h.263 Annex M improved PB-frame
        mv = get_unary(gb, 0, 4) + 1;
        c = mv & 1;
        mv = !!(mv & 2);
    }
    if(c)
        *cbpb = get_bits(gb, 6);
    return mv;
}

#define tab_size ((signed)FF_ARRAY_ELEMS(s->direct_scale_mv[0]))
#define tab_bias (tab_size / 2)
static inline void set_one_direct_mv(MpegEncContext *s, Picture *p, int i)
{
    int xy           = s->block_index[i];
    uint16_t time_pp = s->pp_time;
    uint16_t time_pb = s->pb_time;
    int p_mx, p_my;

    p_mx = p->motion_val[0][xy][0];
    if ((unsigned)(p_mx + tab_bias) < tab_size) {
        s->mv[0][i][0] = s->direct_scale_mv[0][p_mx + tab_bias];
        s->mv[1][i][0] = s->direct_scale_mv[1][p_mx + tab_bias];
    } else {
        s->mv[0][i][0] = p_mx * time_pb / time_pp;
        s->mv[1][i][0] = p_mx * (time_pb - time_pp) / time_pp;
    }
    p_my = p->motion_val[0][xy][1];
    if ((unsigned)(p_my + tab_bias) < tab_size) {
        s->mv[0][i][1] = s->direct_scale_mv[0][p_my + tab_bias];
        s->mv[1][i][1] = s->direct_scale_mv[1][p_my + tab_bias];
    } else {
        s->mv[0][i][1] = p_my * time_pb / time_pp;
        s->mv[1][i][1] = p_my * (time_pb - time_pp) / time_pp;
    }
}

/**
 * @return the mb_type
 */
{

        p = &s->last_picture;
        colocated_mb_type = p->mb_type[mb_index];
    }

        return MB_TYPE_DIRECT2 | MB_TYPE_8x8 | MB_TYPE_L0L1;
    } else {
        // Note see prev line
    }
}

                      int16_t block[6][64])
{


                /* skip mb */
            }
                av_log(s->avctx, AV_LOG_ERROR, "cbpc damaged at %d %d\n", s->mb_x, s->mb_y);
                return SLICE_ERROR;
            }



            pb_mv_count = h263_get_modb(&s->gb, s->pb_frame, &cbpb);

            av_log(s->avctx, AV_LOG_ERROR, "cbpy damaged at %d %d\n", s->mb_x, s->mb_y);
            return SLICE_ERROR;
        }


            h263_decode_dquant(s);
        }

            /* 16x16 motion prediction */
            else

                return SLICE_ERROR;

            else

                return SLICE_ERROR;

        } else {
                    mx = h263p_decode_umotion(s, pred_x);
                else
                    return SLICE_ERROR;

                    my = h263p_decode_umotion(s, pred_y);
                else
                    return SLICE_ERROR;
                  skip_bits1(&s->gb); /* Bit stuffing to prevent PSC */
            }
        }
//        const int mv_xy= s->mb_x + 1 + s->mb_y * s->mb_stride;

        //FIXME ugly

                av_log(s->avctx, AV_LOG_ERROR, "b mb_type damaged at %d %d\n", s->mb_x, s->mb_y);
                return SLICE_ERROR;
            }


            }


                av_log(s->avctx, AV_LOG_ERROR, "b cbpy damaged at %d %d\n", s->mb_x, s->mb_y);
                return SLICE_ERROR;
            }


        }else
            cbp=0;


            h263_decode_dquant(s);
        }

        }else{
//FIXME UMV


                    mx = h263p_decode_umotion(s, pred_x);
                else
                    return SLICE_ERROR;

                    my = h263p_decode_umotion(s, pred_y);
                else
                    return SLICE_ERROR;

                    skip_bits1(&s->gb); /* Bit stuffing to prevent PSC */

            }


                    mx = h263p_decode_umotion(s, pred_x);
                else
                    return SLICE_ERROR;

                    my = h263p_decode_umotion(s, pred_y);
                else
                    return SLICE_ERROR;

                    skip_bits1(&s->gb); /* Bit stuffing to prevent PSC */

            }
        }

    } else { /* I-Frame */
                av_log(s->avctx, AV_LOG_ERROR, "I cbpc damaged at %d %d\n", s->mb_x, s->mb_y);
                return SLICE_ERROR;
            }



            }
        }else

            pb_mv_count = h263_get_modb(&s->gb, s->pb_frame, &cbpb);
            av_log(s->avctx, AV_LOG_ERROR, "I cbpy damaged at %d %d\n", s->mb_x, s->mb_y);
            return SLICE_ERROR;
        }
            h263_decode_dquant(s);
        }

    }

        ff_h263_decode_motion(s, 0, 1);
        ff_h263_decode_motion(s, 0, 1);
    }

    /* decode each block */
            return -1;
    }

        return -1;
    }

        return AVERROR_INVALIDDATA;

        /* per-MB end of slice check */
    {

        }

    }

    return SLICE_OK;
}

/* Most is hardcoded; should extend to handle all H.263 streams. */
{


         av_log(s->avctx, AV_LOG_WARNING, "Header looks like RTP instead of H.263\n");
    }



            break;
    }

        av_log(s->avctx, AV_LOG_ERROR, "Bad picture start code\n");
        return -1;
    }
    /* temporal reference */



    /* PTYPE starts here */
        return -1;
    }
        av_log(s->avctx, AV_LOG_ERROR, "Bad H.263 id\n");
        return -1;      /* H.263 id */
    }

    /*
        0    forbidden
        1    sub-QCIF
        10   QCIF
        7       extended PTYPE (PLUSPTYPE)
    */

        /* H.263v1 */
            return -1;



            av_log(s->avctx, AV_LOG_ERROR, "H.263 SAC not supported\n");
            return -1; /* SAC: off */
        }


    } else {

        /* H.263v2 */

        /* ufep other than 0 and 1 are reserved */
            /* OPPTYPE */
                av_log(s->avctx, AV_LOG_ERROR, "Syntax-based Arithmetic Coding (SAC) not supported\n");
            }
                s->loop_filter = 0;

                av_log(s->avctx, AV_LOG_ERROR, "Reference Picture Selection not supported\n");
            }
                av_log(s->avctx, AV_LOG_ERROR, "Independent Segment Decoding not supported\n");
            }


        } else if (ufep != 0) {
            av_log(s->avctx, AV_LOG_ERROR, "Bad UFEP type (%d)\n", ufep);
            return -1;
        }

        /* MPPTYPE */
        case 2: s->pict_type= AV_PICTURE_TYPE_P;s->pb_frame = 3;break;
        case 3: s->pict_type= AV_PICTURE_TYPE_B;break;
        case 7: s->pict_type= AV_PICTURE_TYPE_I;break; //ZYGO
        default:
            return -1;
        }

        /* Get the picture dimensions */
                /* Custom Picture Format (CPFMT) */
                s->aspect_ratio_info = get_bits(&s->gb, 4);
                ff_dlog(s->avctx, "aspect: %d\n", s->aspect_ratio_info);
                /* aspect ratios:
                0 - forbidden
                1 - 1:1
                2 - 12:11 (CIF 4:3)
                3 - 10:11 (525-type 4:3)
                4 - 16:11 (CIF 16:9)
                5 - 40:33 (525-type 16:9)
                6-14 - reserved
                */
                width = (get_bits(&s->gb, 9) + 1) * 4;
                check_marker(s->avctx, &s->gb, "in dimensions");
                height = get_bits(&s->gb, 9) * 4;
                ff_dlog(s->avctx, "\nH.263+ Custom picture: %dx%d\n",width,height);
                if (s->aspect_ratio_info == FF_ASPECT_EXTENDED) {
                    /* expected dimensions */
                    s->avctx->sample_aspect_ratio.num= get_bits(&s->gb, 8);
                    s->avctx->sample_aspect_ratio.den= get_bits(&s->gb, 8);
                }else{
                    s->avctx->sample_aspect_ratio= ff_h263_pixel_aspect[s->aspect_ratio_info];
                }
            } else {
            }
                return -1;

                    av_log(s, AV_LOG_ERROR, "zero framerate\n");
                    return -1;
                }
            }else{
                s->avctx->framerate = (AVRational){ 30000, 1001 };
            }
        }

        }

            }
                if (get_bits1(&s->gb) != 0) {
                    av_log(s->avctx, AV_LOG_ERROR, "rectangular slices not supported\n");
                }
                if (get_bits1(&s->gb) != 0) {
                    av_log(s->avctx, AV_LOG_ERROR, "unordered slices not supported\n");
                }
            }
                skip_bits(&s->gb, 4); //ELNUM
                if (ufep == 1) {
                    skip_bits(&s->gb, 4); // RLNUM
                }
            }
        }

    }

        return ret;

            return AVERROR_INVALIDDATA;
    }


        skip_bits(&s->gb, 3); /* Temporal reference for B-pictures */
        if (s->custom_pcf)
            skip_bits(&s->gb, 2); //extended Temporal reference
        skip_bits(&s->gb, 2); /* Quantization information for B-pictures */
    }

    }else{
        s->time    = s->picture_number;
        s->pb_time = s->pp_time - (s->last_non_b_time - s->time);
        if (s->pp_time <=s->pb_time ||
            s->pp_time <= s->pp_time - s->pb_time ||
            s->pp_time <= 0){
            s->pp_time = 2;
            s->pb_time = 1;
        }
        ff_mpeg4_init_direct_mv(s);
    }

    /* PEI */
        return AVERROR_INVALIDDATA;

        if (check_marker(s->avctx, &s->gb, "SEPB1") != 1) {
            return -1;
        }

        ff_h263_decode_mba(s);

        if (check_marker(s->avctx, &s->gb, "SEPB2") != 1) {
            return -1;
        }
    }

        s->low_delay = 0;

    }else{
    }

        int i,j;
        for(i=0; i<85; i++) av_log(s->avctx, AV_LOG_DEBUG, "%d", get_bits1(&s->gb));
        av_log(s->avctx, AV_LOG_DEBUG, "\n");
        for(i=0; i<13; i++){
            for(j=0; j<3; j++){
                int v= get_bits(&s->gb, 8);
                v |= get_sbits(&s->gb, 8) * (1 << 8);
                av_log(s->avctx, AV_LOG_DEBUG, " %5d", v);
            }
            av_log(s->avctx, AV_LOG_DEBUG, "\n");
        }
        for(i=0; i<50; i++) av_log(s->avctx, AV_LOG_DEBUG, "%d", get_bits1(&s->gb));
    }

    return 0;
}
