/*
 * Kega Game Video (KGV1) decoder
 * Copyright (c) 2010 Daniel Verkamp
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
 * Kega Game Video decoder
 */

#include "libavutil/common.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/imgutils.h"
#include "avcodec.h"
#include "internal.h"

typedef struct KgvContext {
    uint16_t *frame_buffer;
    uint16_t *last_frame_buffer;
} KgvContext;

{

    av_freep(&c->frame_buffer);
}

                        AVPacket *avpkt)
{

        return AVERROR_INVALIDDATA;


        return AVERROR_INVALIDDATA;

        av_freep(&c->frame_buffer);
        av_freep(&c->last_frame_buffer);
        if ((res = ff_set_dimensions(avctx, w, h)) < 0)
            return res;
    }

            decode_flush(avctx);
            return AVERROR(ENOMEM);
        }
    }


        return res;



        } else {

                // copy from previous frame


                        break;
                }


                    break;

                    av_log(avctx, AV_LOG_ERROR,
                           "Frame reference does not exist\n");
                    break;
                }

            } else {
                // copy from earlier in this frame

                    count = 2;
                    count = 3;
                } else {
                        break;
                }

                    break;

            }
        }
    }

        av_log(avctx, AV_LOG_DEBUG, "frame finished with %d diff\n", outcnt - maxcnt);



}

{

}

{
}

AVCodec ff_kgv1_decoder = {
    .name           = "kgv1",
    .long_name      = NULL_IF_CONFIG_SMALL("Kega Game Video"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_KGV1,
    .priv_data_size = sizeof(KgvContext),
    .init           = decode_init,
    .close          = decode_end,
    .decode         = decode_frame,
    .flush          = decode_flush,
    .capabilities   = AV_CODEC_CAP_DR1,
};
