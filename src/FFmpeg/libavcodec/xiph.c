/*
 * Copyright (C) 2007 The FFmpeg Project
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

#include "libavutil/intreadwrite.h"
#include "xiph.h"

                          int first_header_size, const uint8_t *header_start[3],
                          int header_len[3])
{

        int overall_len = 6;
                return AVERROR_INVALIDDATA;
        }
                header_len[i] += 0xff;
                overall_len   += 0xff + 1;
            }
                return AVERROR_INVALIDDATA;
        }
    } else {
        return -1;
    }
    return 0;
}
