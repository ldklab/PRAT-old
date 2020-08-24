/*
 * AV1 common parsing code
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

#include "config.h"

#include "libavutil/mem.h"

#include "av1.h"
#include "av1_parse.h"
#include "bytestream.h"

{

                           &type, &temporal_id, &spatial_id);
        return len;



           "obu_type: %d, temporal_id: %d, spatial_id: %d, payload size: %d\n",
           obu->type, obu->temporal_id, obu->spatial_id, obu->size);

}

{




                return AVERROR(ENOMEM);
                return AVERROR(ENOMEM);

        }

            return consumed;



            av_log(logctx, AV_LOG_ERROR, "Invalid OBU of type %d, skipping.\n", obu->type);
            continue;
        }


            return ret;
    }

    return 0;
}

{
