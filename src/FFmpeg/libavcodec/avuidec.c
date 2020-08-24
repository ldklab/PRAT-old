/*
 * AVID Meridien decoder
 *
 * Copyright (c) 2012 Carl Eugen Hoyos
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

#include "avcodec.h"
#include "internal.h"
#include "libavutil/intreadwrite.h"

{
}

                             int *got_frame, AVPacket *avpkt)
{

        }
        if (atom_size && atom_size <= extradata_size) {
            extradata      += atom_size;
            extradata_size -= atom_size;
        } else {
            break;
        }
    }
        skip = 10;
    } else {
    }
        av_log(avctx, AV_LOG_ERROR, "Insufficient input data.\n");
        return AVERROR(EINVAL);
    }
                  avpkt->size >= opaque_length * 2 + 4;

        return ret;


    }

            y = pic->data[0] + (1 - i) * pic->linesize[0];
            u = pic->data[1] + (1 - i) * pic->linesize[1];
            v = pic->data[2] + (1 - i) * pic->linesize[2];
            a = pic->data[3] + (1 - i) * pic->linesize[3];
        } else {
        }

            }

        }
    }

}

AVCodec ff_avui_decoder = {
    .name         = "avui",
    .long_name    = NULL_IF_CONFIG_SMALL("Avid Meridien Uncompressed"),
    .type         = AVMEDIA_TYPE_VIDEO,
    .id           = AV_CODEC_ID_AVUI,
    .init         = avui_decode_init,
    .decode       = avui_decode_frame,
    .capabilities = AV_CODEC_CAP_DR1,
    .caps_internal  = FF_CODEC_CAP_INIT_THREADSAFE,
};
