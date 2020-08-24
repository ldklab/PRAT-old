/*
 * (I)DCT Transforms
 * Copyright (c) 2009 Peter Ross <pross@xvid.org>
 * Copyright (c) 2010 Alex Converse <alex.converse@gmail.com>
 * Copyright (c) 2010 Vitor Sessak
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * @file
 * (Inverse) Discrete Cosine Transforms. These are also known as the
 * type II and type III DCTs respectively.
 */

#include <math.h>
#include <string.h>

#include "libavutil/mathematics.h"
#include "dct.h"
#include "dct32.h"

/* sin((M_PI * x / (2 * n)) */
#define SIN(s, n, x) (s->costab[(n) - (x)])

/* cos((M_PI * x / (2 * n)) */
#define COS(s, n, x) (s->costab[x])

{


    }



    }


{




    }



{



    }




    }

{



    }





    }

{

{



    } else {

            return AVERROR(ENOMEM);

            av_freep(&s->csc2);
            return ret;
        }


        }


}

{
