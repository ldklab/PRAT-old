/*
 * Canopus common routines
 * Copyright (c) 2015 Vittorio Giovara <vittorio.giovara@gmail.com>
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

#include <stdint.h>

#include "libavutil/rational.h"

#include "avcodec.h"
#include "bytestream.h"
#include "canopus.h"

                              const uint8_t *src, size_t size)
{


    /* Parse aspect ratio. */
                  &avctx->sample_aspect_ratio.den,
                  par_x, par_y, 255);

    /* Short INFO tag (used in CLLC) has only AR data. */
        return 0;


    /* Parse FIEL tag. */
    case 1: avctx->field_order = AV_FIELD_BB; break;
    }

    return 0;
}
