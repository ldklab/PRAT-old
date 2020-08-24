/*
 * Audio Frame Queue
 * Copyright (c) 2012 Justin Ruggles
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

#include "libavutil/attributes.h"
#include "libavutil/common.h"
#include "audio_frame_queue.h"
#include "internal.h"
#include "libavutil/avassert.h"

{

{
        av_log(afq->avctx, AV_LOG_WARNING, "%d frames left in the queue on closing\n", afq->frame_count);

{
        return AVERROR(ENOMEM);

    /* get frame parameters */
                                      afq->avctx->time_base,
            av_log(afq->avctx, AV_LOG_WARNING, "Queue input is backward in time\n");
    } else {
        new->pts = AV_NOPTS_VALUE;
    }

    /* add frame sample count */


}

                        int64_t *duration)
{

            out_pts = afq->frames->pts;
    }
        av_log(afq->avctx, AV_LOG_WARNING, "Trying to remove %d samples, but the queue is empty\n", nb_samples);

    }

    }
