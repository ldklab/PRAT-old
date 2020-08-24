/*
 * Common AAC and AC-3 parser
 * Copyright (c) 2003 Fabrice Bellard
 * Copyright (c) 2003 Michael Niedermayer
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

#include "libavutil/channel_layout.h"
#include "libavutil/common.h"
#include "parser.h"
#include "aac_ac3_parser.h"

                     AVCodecContext *avctx,
                     const uint8_t **poutbuf, int *poutbuf_size,
                     const uint8_t *buf, int buf_size)
{

        }else{ //we need a header first
            len=0;
                    break;
            }
                i=END_NOT_FOUND;
            }else{
                }
                    s->remaining_size += i;
                }
            }
        }
    }

    }


    /* update codec info */

        /* Due to backwards compatible HE-AAC the sample rate, channel count,
           and total number of samples found in an AAC ADTS header are not
           reliable. Bit rate is still accurate because the total frame
           duration in seconds is still correct (as is the number of bits in
           the frame). */
            }
        }

        /* Calculate the average bit rate */
        }
    }

    return i;
}
