/*
 * Copyright (C) 2003 Mike Melanson
 * Copyright (C) 2003 Dr. Tim Ferguson
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
 * id RoQ Video common functions based on work by Dr. Tim Ferguson
 */

#include "avcodec.h"
#include "roqvideo.h"

                              int outstride, int instride, int sz)
{
    }
}

{






{







                                        int deltay, int sz)
{


    /* check MV against frame boundaries */
        av_log(ri->avctx, AV_LOG_ERROR, "motion vector out of bounds: MV = (%d, %d), boundaries = (0, 0, %d, %d)\n",
            mx, my, ri->width, ri->height);
        return;
    }

        av_log(ri->avctx, AV_LOG_ERROR, "Invalid decode type. Invalid header?\n");
        return;
    }

                   outstride, instride, sz);
    }
}


                             int deltax, int deltay)
{

                             int deltax, int deltay)
{
