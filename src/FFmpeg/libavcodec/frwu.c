/*
 * Forward Uncompressed
 *
 * Copyright (c) 2009 Reimar Döffinger <Reimar.Doeffinger@gmx.de>
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
#include "bytestream.h"
#include "internal.h"
#include "libavutil/opt.h"

typedef struct {
    AVClass *av_class;
    int change_field_order;
} FRWUContext;

{
        av_log(avctx, AV_LOG_ERROR, "frwu needs even width\n");
        return AVERROR(EINVAL);
    }

}

                        AVPacket *avpkt)
{

        av_log(avctx, AV_LOG_ERROR, "Packet is too small.\n");
        return AVERROR_INVALIDDATA;
    }
        av_log(avctx, AV_LOG_ERROR, "incorrect marker\n");
        return AVERROR_INVALIDDATA;
    }

        return ret;


            return AVERROR_INVALIDDATA;
            av_log(avctx, AV_LOG_ERROR, "Field size %i is too small (required %i)\n", field_size, min_field_size);
            return AVERROR_INVALIDDATA;
        }
            av_log(avctx, AV_LOG_ERROR, "Packet is too small, need %i, have %i\n", field_size, (int)(buf_end - buf));
            return AVERROR_INVALIDDATA;
        }
            dst += 2 * pic->linesize[0];
        }
                dst = pic->data[0];
        }
    }


}

static const AVOption frwu_options[] = {
    {"change_field_order", "Change field order", offsetof(FRWUContext, change_field_order), AV_OPT_TYPE_BOOL,
     {.i64 = 0}, 0, 1, AV_OPT_FLAG_DECODING_PARAM | AV_OPT_FLAG_VIDEO_PARAM},
    {NULL}
};

static const AVClass frwu_class = {
    .class_name = "frwu Decoder",
    .item_name  = av_default_item_name,
    .option     = frwu_options,
    .version    = LIBAVUTIL_VERSION_INT,
};

AVCodec ff_frwu_decoder = {
    .name           = "frwu",
    .long_name      = NULL_IF_CONFIG_SMALL("Forward Uncompressed"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_FRWU,
    .priv_data_size = sizeof(FRWUContext),
    .init           = decode_init,
    .decode         = decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
    .priv_class     = &frwu_class,
};
