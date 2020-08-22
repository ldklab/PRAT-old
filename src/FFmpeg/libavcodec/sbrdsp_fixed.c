/*
 * AAC Spectral Band Replication decoding functions
 * Copyright (c) 2008-2009 Robert Swain ( rob opendot cl )
 * Copyright (c) 2009-2010 Alex Converse <alex.converse@gmail.com>
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
 *
 * Note: Rounding-to-nearest used unless otherwise stated
 *
 */

#define USE_FIXED 1

#include "aac.h"
#include "config.h"
#include "libavutil/attributes.h"
#include "libavutil/intfloat.h"
#include "sbrdsp.h"

{

                accu0 >>= 1;
                accu1 >>= 1;
                accu2 >>= 1;
                accu3 >>= 1;
                accu  >>= 1;
                nz ++;
            }
        }
    }


        nz = 33;
        }
    } else
        nz = 1;


}

{

{
    }

{
    }

{
    }

{
            nz = 1;
        } else {
            nz = 0;
            }
        }

}

{

        }





        }
    } else {
        }



    }

{

                       const int alpha0[2], const int alpha1[2],
                       int bw, int start, int end)
{



    }

                          const SoftFloat *g_filt, int m_max, intptr_t ixh)
{


        }
    }

                                                const SoftFloat *s_m,
                                                const SoftFloat *q_filt,
                                                int noise,
                                                int phi_sign0,
                                                int phi_sign1,
                                                int m_max)
{


                av_log(NULL, AV_LOG_ERROR, "Overflow in sbr_hf_apply_noise, shift=%d\n", shift);
                return AVERROR(ERANGE);
            }
        } else {

                av_log(NULL, AV_LOG_ERROR, "Overflow in sbr_hf_apply_noise, shift=%d\n", shift);
                return AVERROR(ERANGE);


            }
        }
    }
    return 0;
}

#include "sbrdsp_template.c"
