/*
 * Apple MJPEG-B decoder
 * Copyright (c) 2002 Alex Beregszaszi
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
 * Apple MJPEG-B decoder.
 */

#include <inttypes.h>

#include "avcodec.h"
#include "internal.h"
#include "mjpeg.h"
#include "mjpegdec.h"

        av_log(avctx, AV_LOG_WARNING, err_msg, offs, size);
        return 0;
    }
    return offs;
}

                              void *data, int *got_frame,
                              AVPacket *avpkt)
{


    /* reset on every SOI */

        return AVERROR_INVALIDDATA;



        av_log(avctx, AV_LOG_WARNING, "not mjpeg-b (bad fourcc)\n");
        return AVERROR_INVALIDDATA;
    }

           second_field_offs);

            return ret;
    }

    }

            return ret;
    }

            return ret;
    }

        /* if not bottom field, do not output image yet */
        }
    }

    //XXX FIXME factorize, this looks very similar to the EOI code

        av_log(avctx, AV_LOG_WARNING, "no picture\n");
        return buf_size;
    }

        return ret;

        av_log(avctx, AV_LOG_DEBUG, "QP: %d\n",
               FFMAX3(s->qscale[0], s->qscale[1], s->qscale[2]));
    }

    return buf_size;
}

AVCodec ff_mjpegb_decoder = {
    .name           = "mjpegb",
    .long_name      = NULL_IF_CONFIG_SMALL("Apple MJPEG-B"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_MJPEGB,
    .priv_data_size = sizeof(MJpegDecodeContext),
    .init           = ff_mjpeg_decode_init,
    .close          = ff_mjpeg_decode_end,
    .decode         = mjpegb_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
    .max_lowres     = 3,
    .caps_internal  = FF_CODEC_CAP_INIT_THREADSAFE,
};
