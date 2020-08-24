/*
 * TIFF Common Routines
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
 * TIFF Common Routines
 * @author Thilo Borgmann <thilo.borgmann _at_ mail.de>
 */

#include "tiff_common.h"


{
        }
    }
    return 0;
}


{
}


{
}


double ff_tget_double(GetByteContext *gb, int le)
{
    av_alias64 i = { .u64 = le ? bytestream2_get_le64(gb) : bytestream2_get_be64(gb)};
    return i.f64;
}


{
    case TIFF_BYTE:  return bytestream2_get_byte(gb);
    default:         return UINT_MAX;
    }
}

{
        return ", ";
    } else
}

                              GetByteContext *gb, int le, AVDictionary **metadata)
{

        return AVERROR_INVALIDDATA;
        return AVERROR_INVALIDDATA;


    }

        return i;
    }
        return AVERROR(ENOMEM);
    }


}


                          GetByteContext *gb, int le, AVDictionary **metadata)
{

        return AVERROR_INVALIDDATA;
        return AVERROR_INVALIDDATA;


    }

        return i;
    }
        return AVERROR(ENOMEM);
    }


}


int ff_tadd_doubles_metadata(int count, const char *name, const char *sep,
                             GetByteContext *gb, int le, AVDictionary **metadata)
{
    AVBPrint bp;
    char *ap;
    int i;

    if (count >= INT_MAX / sizeof(int64_t) || count <= 0)
        return AVERROR_INVALIDDATA;
    if (bytestream2_get_bytes_left(gb) < count * sizeof(int64_t))
        return AVERROR_INVALIDDATA;

    av_bprint_init(&bp, 10 * count, 100 * count);

    for (i = 0; i < count; i++) {
        av_bprintf(&bp, "%s%.15g", auto_sep(count, sep, i, 4), ff_tget_double(gb, le));
    }

    if ((i = av_bprint_finalize(&bp, &ap))) {
        return i;
    }
    if (!ap) {
        return AVERROR(ENOMEM);
    }

    av_dict_set(metadata, name, ap, AV_DICT_DONT_STRDUP_VAL);

    return 0;
}


                            GetByteContext *gb, int le, int is_signed, AVDictionary **metadata)
{

        return AVERROR_INVALIDDATA;
        return AVERROR_INVALIDDATA;


    }

        return i;
    }
        return AVERROR(ENOMEM);
    }


}


                           GetByteContext *gb, int le, int is_signed, AVDictionary **metadata)
{

        return AVERROR_INVALIDDATA;
        return AVERROR_INVALIDDATA;


    }

        return i;
    }
        return AVERROR(ENOMEM);
    }


}

                            GetByteContext *gb, int le, AVDictionary **metadata)
{

        return AVERROR_INVALIDDATA;

        return AVERROR(ENOMEM);


}


{
        return AVERROR_INVALIDDATA;
    }

    } else {
        return AVERROR_INVALIDDATA;
    }

        return AVERROR_INVALIDDATA;
    }


}


                 unsigned *count, int *next)
{




    // check for valid type
        return AVERROR_INVALIDDATA;
    }

    // seek to offset if this is an IFD-tag or
    // if count values do not fit into the offset value
    }

    return 0;
}
