/*
 * MPEG-4 decoder / encoder common code
 * Copyright (c) 2000,2001 Fabrice Bellard
 * Copyright (c) 2002-2010 Michael Niedermayer <michaelni@gmx.at>
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

#include "mpegutils.h"
#include "mpegvideo.h"
#include "mpeg4video.h"
#include "mpeg4data.h"

uint8_t ff_mpeg4_static_rl_table_store[3][2][2 * MAX_RUN + MAX_LEVEL + 3];

{
    case AV_PICTURE_TYPE_I:
        return 16;
    case AV_PICTURE_TYPE_S:
    default:
        return -1;
    }
}

{


    /* clean AC */

    /* clean MV */
    // we can't clear the MVs as they might be needed by a B-frame

#define tab_size ((signed)FF_ARRAY_ELEMS(s->direct_scale_mv[0]))
#define tab_bias (tab_size / 2)

// used by MPEG-4 and rv10 decoder
{
                                   s->pp_time;
    }

                                              int my, int i)
{

    } else {
    }
    } else {
    }

#undef tab_size
#undef tab_bias

/**
 * @return the mb_type
 */
{

    // FIXME avoid divides
    // try special case with shifts for 1 and 3 B-frames?

        return MB_TYPE_DIRECT2 | MB_TYPE_8x8 | MB_TYPE_L0L1;
        s->mv_type = MV_TYPE_FIELD;
        for (i = 0; i < 2; i++) {
            int field_select = s->next_picture.ref_index[0][4 * mb_index + 2 * i];
            s->field_select[0][i] = field_select;
            s->field_select[1][i] = i;
            if (s->top_field_first) {
                time_pp = s->pp_field_time - field_select + i;
                time_pb = s->pb_field_time - field_select + i;
            } else {
                time_pp = s->pp_field_time + field_select - i;
                time_pb = s->pb_field_time + field_select - i;
            }
            s->mv[0][i][0] = s->p_field_mv_table[i][0][mb_index][0] *
                             time_pb / time_pp + mx;
            s->mv[0][i][1] = s->p_field_mv_table[i][0][mb_index][1] *
                             time_pb / time_pp + my;
            s->mv[1][i][0] = mx ? s->mv[0][i][0] -
                                  s->p_field_mv_table[i][0][mb_index][0]
                                : s->p_field_mv_table[i][0][mb_index][0] *
                                  (time_pb - time_pp) / time_pp;
            s->mv[1][i][1] = my ? s->mv[0][i][1] -
                                  s->p_field_mv_table[i][0][mb_index][1]
                                : s->p_field_mv_table[i][0][mb_index][1] *
                                  (time_pb - time_pp) / time_pp;
        }
        return MB_TYPE_DIRECT2 | MB_TYPE_16x8 |
               MB_TYPE_L0L1    | MB_TYPE_INTERLACED;
    } else {
        else
        // Note see prev line
    }
}
