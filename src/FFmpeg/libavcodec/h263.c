/*
 * H.263/MPEG-4 backend for encoder and decoder
 * Copyright (c) 2000,2001 Fabrice Bellard
 * H.263+ support.
 * Copyright (c) 2001 Juan J. Sierralta P
 * Copyright (c) 2002-2004 Michael Niedermayer <michaelni@gmx.at>
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
 * H.263/MPEG-4 codec.
 */

#include <limits.h>

#include "avcodec.h"
#include "mpegvideo.h"
#include "h263.h"
#include "h263data.h"
#include "mathops.h"
#include "mpegutils.h"
#include "flv.h"
#include "mpeg4video.h"


               //FIXME a lot of that is only needed for !low_delay


            motion_x = 0;
            motion_y = 0;
        } else /*if (s->mv_type == MV_TYPE_FIELD)*/ {
            int i;
            motion_x = s->mv[0][0][0] + s->mv[0][1][0];
            motion_y = s->mv[0][0][1] + s->mv[0][1][1];
            motion_x = (motion_x>>1) | (motion_x&1);
            for(i=0; i<2; i++){
                s->p_field_mv_table[i][0][mb_xy][0]= s->mv[0][i][0];
                s->p_field_mv_table[i][0][mb_xy][1]= s->mv[0][i][1];
            }
            s->current_picture.ref_index[0][4*mb_xy    ] =
            s->current_picture.ref_index[0][4*mb_xy + 1] = s->field_select[0][0];
            s->current_picture.ref_index[0][4*mb_xy + 2] =
            s->current_picture.ref_index[0][4*mb_xy + 3] = s->field_select[0][1];
        }

        /* no update if 8X8 because it has been done during parsing */
    }

        else
    }

{

    /* find prediction */
    } else {
    }
    /* B C
     * A X
     */

    /* No prediction outside GOB boundary */
    }
    /* just DC prediction */
        pred_dc = a;
    else

    /* we assume pred is positive */
}


    /*
       Diag Top
       Left Center
    */
    }else
        qp_c= 0;


            qp_tt=0;
        else

            qp_tc= qp_c;
        else


        }


                qp_dt= qp_tt;
            else

            }
        }
    }

    }

            qp_lc= qp_c;
        else

            }
        }
    }

{

    /* find prediction */
    } else {
    }


    /* B C
     * A X
     */

    /* No prediction outside GOB boundary */
    }

            /* left prediction */
                }
                pred_dc = a;
            }
        } else {
            /* top prediction */
                }
                pred_dc = c;
            }
        }
    } else {
        /* just DC prediction */
            pred_dc = a;
        else
    }

    /* we assume pred is positive */

        block[0] = 0;
    else

    /* Update AC/DC tables */

    /* left copy */
    /* top copy */

                             int *px, int *py)
{


    /* special case for first (slice) line */
        // we can't just change some MVs to simulate that as we need them for the B-frames (and ME)
        // and if we ever support non rectangular objects than we need to do a few ifs here anyway :(
                }else{
                }
            }else{
            }
            }else{
            }
        }else{ /* block==2*/

        }
    } else {
    }
}
