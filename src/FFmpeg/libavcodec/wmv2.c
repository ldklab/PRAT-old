/*
 * Copyright (c) 2002 The FFmpeg Project
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

#include "avcodec.h"
#include "idctdsp.h"
#include "mpegutils.h"
#include "mpegvideo.h"
#include "msmpeg4data.h"
#include "simple_idct.h"
#include "wmv2.h"
#include "wmv2data.h"


{

                                  w->wdsp.idct_perm);
                      ff_wmv2_scantableA);
                      ff_wmv2_scantableB);
                      ff_wmv1_scantable[1]);
                      ff_wmv1_scantable[2]);
                      ff_wmv1_scantable[3]);
                      ff_wmv1_scantable[0]);

                           uint8_t *dst, int stride, int n)
{

        default:
            av_log(s->avctx, AV_LOG_ERROR, "internal error in WMV2 abt\n");
        }

                    uint8_t *dest_y, uint8_t *dest_cb, uint8_t *dest_cr)
{


        return;

}

                     uint8_t *dest_cb, uint8_t *dest_cr,
                     uint8_t **ref_picture, op_pixels_func (*pix_op)[4],
                     int motion_x, int motion_y, int h)
{


    /* WARNING: do no forget half pels */



                                 s->linesize, s->linesize, 19, 19,
                                 src_x - 1, src_y - 1,
                                 s->h_edge_pos, s->v_edge_pos);
    }


        return;


        dxy &= ~1;
                                 s->uvlinesize, s->uvlinesize,
                                 9, 9,
                                 src_x, src_y,
    }

                                 s->uvlinesize, s->uvlinesize,
                                 9, 9,
                                 src_x, src_y,
    }
}
