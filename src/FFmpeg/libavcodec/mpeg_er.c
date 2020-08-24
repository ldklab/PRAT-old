/*
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

#include "error_resilience.h"
#include "mpegvideo.h"
#include "mpeg_er.h"

{

    }


    }

}

{




                              int (*mv)[2][4][2], int mb_x, int mb_y,
                              int mb_intra, int mb_skipped)
{



        s->bdsp.clear_blocks(s->block[6]);

                 s->mb_x * (16 >> s->chroma_x_shift);

        av_log(s->avctx, AV_LOG_DEBUG,
               "Interlaced error concealment is not fully implemented\n");

{



        goto fail;




fail:
    av_freep(&er->er_temp_buffer);
    av_freep(&er->error_status_table);
    return AVERROR(ENOMEM);
}
