/*
 * huffyuv codec for libavcodec
 *
 * Copyright (c) 2002-2014 Michael Niedermayer <michaelni@gmx.at>
 *
 * see http://www.pcisys.net/~melanson/codecs/huffyuv.txt for a description of
 * the algorithm used
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
 * huffyuv codec for libavcodec.
 */

#include <stdint.h>

#include "libavutil/mem.h"

#include "avcodec.h"
#include "bswapdsp.h"
#include "huffyuv.h"

{

        }
            av_log(NULL, AV_LOG_ERROR, "Error generating huffman table\n");
            return -1;
        }
    }
    return 0;
}

{

            return AVERROR(ENOMEM);
    }
    return 0;
}

{





{

    }
