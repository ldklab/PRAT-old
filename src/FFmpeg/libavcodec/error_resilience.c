/*
 * Error resilience / concealment
 *
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
 * Error resilience / concealment.
 */

#include <limits.h>

#include "libavutil/internal.h"
#include "avcodec.h"
#include "error_resilience.h"
#include "me_cmp.h"
#include "mpegutils.h"
#include "mpegvideo.h"
#include "rectangle.h"
#include "thread.h"
#include "version.h"

/**
 * @param stride the number of MVs to get to the next row
 * @param mv_step the number of MVs per row or column in a macroblock
 */
{
    } else {
    }

/**
 * Replace the current MB with a flat dc-only version.
 */
                   uint8_t *dest_cr, int mb_x, int mb_y)
{
            dc = 0;
        else if (dc > 2040)
            dc = 2040;
            int x;
        }
    }
        dcu = 0;
    else if (dcu > 2040)
        dcu = 2040;
        dcv = 0;
    else if (dcv > 2040)
        dcv = 2040;

        int x;
        }
    }

{

    /* horizontal filter */

        }
    }

    /* vertical filter */


        }
    }

/**
 * guess the dc of blocks which do not have an undamaged dc
 * @param w     width in 8 pixel blocks
 * @param h     height in 8 pixel blocks
 */
                     int h, ptrdiff_t stride, int is_luma)
{

        av_log(s->avctx, AV_LOG_ERROR, "guess_dc() is out of memory\n");
        goto fail;
    }

        int color= 1024;
        int distance= -1;
            }
        }
            }
        }
    }
        int color= 1024;
        int distance= -1;
            }
        }
            }
        }
    }



            weight_sum = 0;
            guess      = 0;
            }
        }
    }


/**
 * simple horizontal deblocking filter used for error resilience
 * @param w     width in 8 pixel blocks
 * @param h     height in 8 pixel blocks
 */
                           int h, ptrdiff_t stride, int is_luma)
{







                }
                }
            }
        }
    }

/**
 * simple vertical deblocking filter used for error resilience
 * @param w     width in 8 pixel blocks
 * @param h     height in 8 pixel blocks
 */
                           ptrdiff_t stride, int is_luma)
{










                }
                }
            }
        }
    }

#define MV_FROZEN    8
#define MV_CHANGED   4
#define MV_UNCHANGED 2
#define MV_LISTED    1
{
        return;
}

{






        }
    }

        for (mb_y = 0; mb_y < mb_height; mb_y++) {
            for (mb_x = 0; mb_x < s->mb_width; mb_x++) {
                const int mb_xy = mb_x + mb_y * s->mb_stride;
                int mv_dir = (s->last_pic.f && s->last_pic.f->data[0]) ? MV_DIR_FORWARD : MV_DIR_BACKWARD;

                if (IS_INTRA(s->cur_pic.mb_type[mb_xy]))
                    continue;
                if (!(s->error_status_table[mb_xy] & ER_MV_ERROR))
                    continue;

                s->mv[0][0][0] = 0;
                s->mv[0][0][1] = 0;
                s->decode_mb(s->opaque, 0, mv_dir, MV_TYPE_16X16, &s->mv,
                             mb_x, mb_y, 0, 0);
            }
        }
        return;
    }

    blocklist_length = 0;
            }
        }
    }


            int score_sum = 0;

            changed = 0;








                }
                }
                }
                }
                    continue;

                    int sum_x = 0, sum_y = 0, sum_r = 0;
                    int max_x, max_y, min_x, min_y, max_r, min_r;

                            goto skip_mean_and_median;
                    }

                    /* mean */

                    /* median */
                        min_y = min_x = min_r =  99999;
                        max_y = max_x = max_r = -99999;
                    } else {
                    }
                    }

                    }
                }

                /* zero MV */


                /* last MV */



                    // predictor intra or otherwise not available
                        continue;

                                 MV_TYPE_16X16, &s->mv, mb_x, mb_y, 0, 0);

                        int k;
                                           src[k * linesize[0]]);
                    }
                        int k;
                                           src[k * linesize[0] + 16]);
                    }
                        int k;
                    }
                        int k;
                                           src[k + linesize[0] * 16]);
                    }

                    }
                }

                    }

                             MV_TYPE_16X16, &s->mv, mb_x, mb_y, 0, 0);


                } else
            }
        }

            return;

        next_blocklist_length = 0;


            }
        }
    }
}

{

        return 1; // no previous frame available -> use spatial prediction

        return 0;

    undamaged_count = 0;
    }

        return 0; // almost all MBs damaged -> use temporal prediction

    // prevent dsp.sad() check, that requires access to the image
        s->avctx->hwaccel && s->avctx->hwaccel->decode_mb &&
        s->cur_pic.f->pict_type == AV_PICTURE_TYPE_I)
        return 1;




            // skip a few to speed things up

                                       mb_x * 16 + mb_y * 16 * linesize[0];

                    // FIXME
                } else {
                }
                // FIXME need await_progress() here
            } else {
                else
            }
        }
    }
//      av_log(NULL, AV_LOG_ERROR, "is_intra_likely: %d type:%d\n", is_intra_likely, s->pict_type);
}

{
        return;

    }

}

{
    )
        return 0;
    return 1;
}

/**
 * Add a slice.
 * @param endx   x component of the last macroblock, can be -1
 *               for the last of the previous line
 * @param status the status at the end (ER_MV_END, ER_AC_ERROR, ...), it is
 *               assumed that no earlier end or error of the same type occurred
 */
                     int endx, int endy, int status)
{

        return;

        av_log(s->avctx, AV_LOG_ERROR,
               "internal error, slice end before start\n");
        return;
    }

        return;

    }
    }
    }

    }

    } else {
        int i;
    }

    else {
    }



        }
    }
}

{

    /* We do not support ER of field pictures yet,
     * though it should not crash if enabled. */
    }
            break;
    }

        && (FFALIGN(s->avctx->height, 16)&16)
        && atomic_load(&s->error_count) == 3 * s->mb_width * (s->avctx->skip_top + s->avctx->skip_bottom + 1)
    ) {
        av_log(s->avctx, AV_LOG_DEBUG, "ignoring last missing slice\n");
        return;
    }

            av_log(s->avctx, AV_LOG_WARNING, "Cannot use previous picture in error concealment\n");
            memset(&s->last_pic, 0, sizeof(s->last_pic));
        }
    }
            av_log(s->avctx, AV_LOG_WARNING, "Cannot use next picture in error concealment\n");
            memset(&s->next_pic, 0, sizeof(s->next_pic));
        }
    }

        av_log(s->avctx, AV_LOG_ERROR, "Warning MVs not available\n");

        for (i = 0; i < 2; i++) {
            s->ref_index_buf[i]  = av_buffer_allocz(s->mb_stride * s->mb_height * 4 * sizeof(uint8_t));
            s->motion_val_buf[i] = av_buffer_allocz((size + 4) * 2 * sizeof(uint16_t));
            if (!s->ref_index_buf[i] || !s->motion_val_buf[i])
                break;
            s->cur_pic.ref_index[i]  = s->ref_index_buf[i]->data;
            s->cur_pic.motion_val[i] = (int16_t (*)[2])s->motion_val_buf[i]->data + 4;
        }
        if (i < 2) {
            for (i = 0; i < 2; i++) {
                av_buffer_unref(&s->ref_index_buf[i]);
                av_buffer_unref(&s->motion_val_buf[i]);
                s->cur_pic.ref_index[i]  = NULL;
                s->cur_pic.motion_val[i] = NULL;
            }
            return;
        }
    }

        for (mb_y = 0; mb_y < s->mb_height; mb_y++) {
            for (mb_x = 0; mb_x < s->mb_width; mb_x++) {
                int status = s->error_status_table[mb_x + mb_y * s->mb_stride];

                av_log(s->avctx, AV_LOG_DEBUG, "%2X ", status);
            }
            av_log(s->avctx, AV_LOG_DEBUG, "\n");
        }
    }

#if 1
    /* handle overlapping slices */


                end_ok = 1;

                s->error_status_table[mb_xy] |= 1 << error_type;

        }
    }
#endif
#if 1
    /* handle slices with partitions of different length */


                (error & ER_DC_END) ||
                (error & ER_AC_ERROR))
                end_ok = 1;

                s->error_status_table[mb_xy]|= ER_AC_ERROR;

        }
    }
#endif
    /* handle missing slices */
        int end_ok = 1;

        // FIXME + 100 hack
        for (i = s->mb_num - 2; i >= s->mb_width + 100; i--) {
            const int mb_xy = s->mb_index2xy[i];
            int error1 = s->error_status_table[mb_xy];
            int error2 = s->error_status_table[s->mb_index2xy[i + 1]];

            if (error1 & VP_START)
                end_ok = 1;

            if (error2 == (VP_START | ER_MB_ERROR | ER_MB_END) &&
                error1 != (VP_START | ER_MB_ERROR | ER_MB_END) &&
                ((error1 & ER_AC_END) || (error1 & ER_DC_END) ||
                (error1 & ER_MV_END))) {
                // end & uninit
                end_ok = 0;
            }

            if (!end_ok)
                s->error_status_table[mb_xy] |= ER_MB_ERROR;
        }
    }

#if 1
    /* backward mark errors */
    distance = 9999999;


            } else {
            }

        }
    }
#endif

    /* forward mark errors */
    error = 0;

        } else {
        }
    }
#if 1
    /* handle not partitioned case */
        }
    }
#endif

    }



    /* set unknown mb-type to most likely */

        else
    }

    // change inter to intra blocks if no reference frames are available
                s->cur_pic.mb_type[mb_xy] = MB_TYPE_INTRA4x4;
        }

    /* handle inter blocks with damaged AC */
            const int mv_dir  = dir ? MV_DIR_BACKWARD : MV_DIR_FORWARD;



            if (IS_8X8(mb_type)) {
                int mb_index = mb_x * 2 + mb_y * 2 * s->b8_stride;
                int j;
                mv_type = MV_TYPE_8X8;
                for (j = 0; j < 4; j++) {
                    s->mv[0][j][0] = s->cur_pic.motion_val[dir][mb_index + (j & 1) + (j >> 1) * s->b8_stride][0];
                    s->mv[0][j][1] = s->cur_pic.motion_val[dir][mb_index + (j & 1) + (j >> 1) * s->b8_stride][1];
                }
            } else {
                mv_type     = MV_TYPE_16X16;
                s->mv[0][0][0] = s->cur_pic.motion_val[dir][mb_x * 2 + mb_y * 2 * s->b8_stride][0];
                s->mv[0][0][1] = s->cur_pic.motion_val[dir][mb_x * 2 + mb_y * 2 * s->b8_stride][1];
            }

            s->decode_mb(s->opaque, 0 /* FIXME H.264 partitioned slices need this set */,
                         mv_dir, mv_type, &s->mv, mb_x, mb_y, 0, 0);
        }
    }

    /* guess MVs */
        for (mb_y = 0; mb_y < s->mb_height; mb_y++) {
            for (mb_x = 0; mb_x < s->mb_width; mb_x++) {
                int       xy      = mb_x * 2 + mb_y * 2 * s->b8_stride;
                const int mb_xy   = mb_x + mb_y * s->mb_stride;
                const int mb_type = s->cur_pic.mb_type[mb_xy];
                int mv_dir = MV_DIR_FORWARD | MV_DIR_BACKWARD;

                int error = s->error_status_table[mb_xy];

                if (IS_INTRA(mb_type))
                    continue;
                if (!(error & ER_MV_ERROR))
                    continue; // inter with undamaged MV
                if (!(error & ER_AC_ERROR))
                    continue; // undamaged inter

                if (!(s->last_pic.f && s->last_pic.f->data[0]))
                    mv_dir &= ~MV_DIR_FORWARD;
                if (!(s->next_pic.f && s->next_pic.f->data[0]))
                    mv_dir &= ~MV_DIR_BACKWARD;

                if (s->pp_time) {
                    int time_pp = s->pp_time;
                    int time_pb = s->pb_time;

                    av_assert0(s->avctx->codec_id != AV_CODEC_ID_H264);
                    ff_thread_await_progress(s->next_pic.tf, mb_y, 0);

                    s->mv[0][0][0] = s->next_pic.motion_val[0][xy][0] *  time_pb            / time_pp;
                    s->mv[0][0][1] = s->next_pic.motion_val[0][xy][1] *  time_pb            / time_pp;
                    s->mv[1][0][0] = s->next_pic.motion_val[0][xy][0] * (time_pb - time_pp) / time_pp;
                    s->mv[1][0][1] = s->next_pic.motion_val[0][xy][1] * (time_pb - time_pp) / time_pp;
                } else {
                    s->mv[0][0][0] = 0;
                    s->mv[0][0][1] = 0;
                    s->mv[1][0][0] = 0;
                    s->mv[1][0][1] = 0;
                }

                s->decode_mb(s->opaque, 0, mv_dir, MV_TYPE_16X16, &s->mv,
                             mb_x, mb_y, 0, 0);
            }
        }
    } else

    /* the filters below manipulate raw image, skip them */
    if (CONFIG_XVMC && s->avctx->hwaccel && s->avctx->hwaccel->decode_mb)
        goto ec_clean;
    /* fill DC for inter blocks */

            // error = s->error_status_table[mb_xy];

            // if (error & ER_MV_ERROR)
            //     continue; // inter data damaged FIXME is this good?


                dc = 0;
                    int x;
                }
            }

                continue;

            dcu = dcv = 0;
                int x;
                }
            }
        }
    }
#if 1
    /* guess DC for damaged blocks */
#endif

    /* filter luma DC */

#if 1
    /* render DC only intra */



                dest_cb = dest_cr = NULL;

        }
    }
#endif

        /* filter horizontal block boundaries */

        /* filter vertical block boundaries */

        }
    }

ec_clean:
    /* clean a few tables */

            (error & (ER_DC_ERROR | ER_MV_ERROR | ER_AC_ERROR))) {
        }
    }

    }

}
