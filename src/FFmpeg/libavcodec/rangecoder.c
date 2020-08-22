/*
 * Range coder
 * Copyright (c) 2004 Michael Niedermayer <michaelni@gmx.at>
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
 * Range coder.
 * based upon
 *    "Range encoding: an algorithm for removing redundancy from a digitised
 *                     message.
 *     G. N. N. Martin                  Presented in March 1979 to the Video &
 *                                      Data Recording Conference,
 *     IBM UK Scientific Center         held in Southampton July 24-27 1979."
 */

#include <string.h>

#include "libavutil/attributes.h"
#include "libavutil/avassert.h"
#include "libavutil/intreadwrite.h"

#include "avcodec.h"
#include "rangecoder.h"

{

                                   int buf_size)
{
    /* cast to avoid compiler warning */

        c->low = 0xFF00;
        c->bytestream_end = c->bytestream;
    }

{



    }


            p8 = max_p;
    }


/* Return the number of bytes written. */
{


}

{


            return AVERROR_INVALIDDATA;
    } else {
            return AVERROR_INVALIDDATA;
    }
    return 0;
}
