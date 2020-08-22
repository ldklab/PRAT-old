/*
 * lzf decompression algorithm
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

/**
 * @file
 * lzf decompression
 *
 * LZF is a fast compression/decompression algorithm that takes very little
 * code space and working memory, ideal for real-time and block compression.
 *
 * https://en.wikibooks.org/wiki/Data_Compression/Dictionary_compression#LZF
 */

#include "libavutil/mem.h"

#include "bytestream.h"
#include "lzf.h"

#define LZF_LITERAL_MAX (1 << 5)
#define LZF_LONG_BACKREF 7 + 2

{


                *size += s + *size /2;
                ret = av_reallocp(buf, *size);
                if (ret < 0)
                    return ret;
                p = *buf + len;
            }

        } else {



                return AVERROR_INVALIDDATA;

                *size += l + *size / 2;
                ret = av_reallocp(buf, *size);
                if (ret < 0)
                    return ret;
                p = *buf + len;
            }


        }
    }


}
