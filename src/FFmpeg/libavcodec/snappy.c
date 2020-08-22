/*
 * Snappy decompression algorithm
 * Copyright (c) 2015 Luca Barbato
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

#include "libavutil/mem.h"

#include "bytestream.h"
#include "snappy.h"

enum {
    SNAPPY_LITERAL,
    SNAPPY_COPY_1,
    SNAPPY_COPY_2,
    SNAPPY_COPY_4,
};

{

            return AVERROR_INVALIDDATA;

}

{

    case 63:
        len += bytestream2_get_le32(gb);
        break;
    case 62:
        len += bytestream2_get_le24(gb);
        break;
    case 61:
    case 60:
    }

        return AVERROR_INVALIDDATA;


}

                       unsigned int off, int len)
{
        return AVERROR_INVALIDDATA;



    return len;
}

                        int size, int val)
{

}

                        int size, int val)
{

}

static int snappy_copy4(GetByteContext *gb, uint8_t *start, uint8_t *p,
                        int size, int val)
{
    int len          = 1 + val;
    unsigned int off = bytestream2_get_le32(gb);

    return snappy_copy(start, p, size, off, len);
}

{

        return AVERROR_INVALIDDATA;

    return len;
}

{


}

{

        return len;

        return AVERROR_BUFFER_TOO_SMALL;



            break;
        case SNAPPY_COPY_4:
            ret = snappy_copy4(gb, buf, p, len, val);
            break;
        }

            return ret;

    }

    return 0;
}
