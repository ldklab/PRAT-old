/*
 * EXIF metadata parser
 * Copyright (c) 2013 Thilo Borgmann <thilo.borgmann _at_ mail.de>
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
 * EXIF metadata parser
 * @author Thilo Borgmann <thilo.borgmann _at_ mail.de>
 */

#include "exif.h"


{

    }

    return NULL;
}


                             const char *name, const char *sep,
                             GetByteContext *gb, int le,
                             AVDictionary **metadata)
{
    case 0:
        av_log(logctx, AV_LOG_WARNING,
               "Invalid TIFF tag type 0 found for %s with size %d\n",
               name, count);
        return 0;
    case TIFF_DOUBLE   : return ff_tadd_doubles_metadata(count, name, sep, gb, le, metadata);
    case TIFF_SSHORT   : return ff_tadd_shorts_metadata(count, name, sep, gb, le, 1, metadata);
    case TIFF_SBYTE    : return ff_tadd_bytes_metadata(count, name, sep, gb, le, 1, metadata);
    default:
        avpriv_request_sample(logctx, "TIFF tag type (%u)", type);
        return 0;
}


                           int depth, AVDictionary **metadata)
{

        return 0;
    }


        bytestream2_seek(gbytes, cur_pos, SEEK_SET);
        return 0;
    }

    // read count values and add it metadata
    // store metadata or proceed with next IFD
    } else {

                return AVERROR(ENOMEM);
            }
        }

                                gbytes, le, metadata);

        }
    }


}


                       int le, int depth, AVDictionary **metadata)
{


        return AVERROR_INVALIDDATA;
    }

            return ret;
        }
    }

    // return next IDF offset or 0x000000000 or a value < 0 for failure
}

int avpriv_exif_decode_ifd(void *logctx, const uint8_t *buf, int size,
                           int le, int depth, AVDictionary **metadata)
{
    GetByteContext gb;

    bytestream2_init(&gb, buf, size);

    return ff_exif_decode_ifd(logctx, &gb, le, depth, metadata);
}
