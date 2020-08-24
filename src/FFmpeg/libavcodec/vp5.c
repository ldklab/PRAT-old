/*
 * Copyright (C) 2006  Aurelien Jacobs <aurel@gnuage.org>
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
 * VP5 compatible video decoder
 */

#include <stdlib.h>
#include <string.h>

#include "avcodec.h"
#include "internal.h"

#include "vp56.h"
#include "vp56data.h"
#include "vp5data.h"


{

        return ret;
    {

            return AVERROR_INVALIDDATA;
            avpriv_report_missing_feature(s->avctx, "Interlacing");
            return AVERROR_PATCHWELCOME;
        }
            av_log(s->avctx, AV_LOG_ERROR, "Invalid size %dx%d\n",
                   cols << 4, rows << 4);
            return AVERROR_INVALIDDATA;
        }
            return AVERROR_INVALIDDATA;
                return ret;
        }
        return AVERROR_INVALIDDATA;
    return 0;
}

{

        }
        else
    }

{

    }


{


            }

                    }

    /* coeff_dcct is a linear combination of coeff_dccv */

    /* coeff_acct is a linear combination of coeff_ract */
}

{

    }




                    } else {
                        } else {
                        }
                    }
                    ct = 2;
                } else {
                }
            } else {
                    break;
            }
                break;

        }

    }
    return 0;
}

{

    }

{

        return ret;

}

AVCodec ff_vp5_decoder = {
    .name           = "vp5",
    .long_name      = NULL_IF_CONFIG_SMALL("On2 VP5"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_VP5,
    .priv_data_size = sizeof(VP56Context),
    .init           = vp5_decode_init,
    .close          = ff_vp56_free,
    .decode         = ff_vp56_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
