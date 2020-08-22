/*
 * MPEG Audio header decoder
 * Copyright (c) 2001, 2002 Fabrice Bellard
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
 * MPEG Audio header decoder.
 */

#include "libavutil/common.h"

#include "avcodec.h"
#include "internal.h"
#include "mpegaudio.h"
#include "mpegaudiodata.h"
#include "mpegaudiodecheader.h"


{

        return ret;

    } else {
    }

    /* extract frequency */
        sample_rate_index = 0;

    //extension = (header >> 8) & 1;
    //copyright = (header >> 3) & 1;
    //original = (header >> 2) & 1;
    //emphasis = header & 3;

    else

        case 3:
        }
    } else {
        /* if no frame size computed, signal it */
        return 1;
    }

#if defined(DEBUG)
    ff_dlog(NULL, "layer%d, %d Hz, %d kbits/s, ",
           s->layer, s->sample_rate, s->bit_rate);
    if (s->nb_channels == 2) {
        if (s->layer == 3) {
            if (s->mode_ext & MODE_EXT_MS_STEREO)
                ff_dlog(NULL, "ms-");
            if (s->mode_ext & MODE_EXT_I_STEREO)
                ff_dlog(NULL, "i-");
        }
        ff_dlog(NULL, "stereo");
    } else {
        ff_dlog(NULL, "mono");
    }
    ff_dlog(NULL, "\n");
#endif
}

{

        return -1;
    }

    case 3:
        else
        break;
    }

}
