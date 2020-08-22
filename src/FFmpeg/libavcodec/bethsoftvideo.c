/*
 * Bethesda VID video decoder
 * Copyright (C) 2007 Nicholas Tung
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
 * @brief Bethesda Softworks VID Video Decoder
 * @author Nicholas Tung [ntung (at. ntung com] (2007-03)
 * @see http://wiki.multimedia.cx/index.php?title=Bethsoft_VID
 * @see http://www.svatopluk.com/andux/docs/dfvid.html
 */

#include "libavutil/common.h"
#include "avcodec.h"
#include "bethsoftvideo.h"
#include "bytestream.h"
#include "internal.h"

typedef struct BethsoftvidContext {
    AVFrame *frame;
    GetByteContext g;
} BethsoftvidContext;

{

        return AVERROR(ENOMEM);

    return 0;
}

{

        return AVERROR_INVALIDDATA;

    }
}

                              void *data, int *got_frame,
                              AVPacket *avpkt)
{

        return ret;

                         avpkt->side_data[0].size);
            return ret;
    }


        case PALETTE_BLOCK: {
            *got_frame = 0;
            if ((ret = set_palette(vid)) < 0) {
                av_log(avctx, AV_LOG_ERROR, "error reading palette\n");
                return ret;
            }
            return bytestream2_tell(&vid->g);
        }
                return AVERROR_INVALIDDATA;
        case VIDEO_P_FRAME:
        case VIDEO_I_FRAME:
            break;
        default:
            return AVERROR_INVALIDDATA;
    }

    // main code

        // copy any bytes starting at the current position, and ending at the frame width
                bytestream2_get_buffer(&vid->g, dst, remaining);
                goto end;
        }

        // copy any remaining bytes after / if line overflows
    }

        return ret;


}

{
}

AVCodec ff_bethsoftvid_decoder = {
    .name           = "bethsoftvid",
    .long_name      = NULL_IF_CONFIG_SMALL("Bethesda VID video"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_BETHSOFTVID,
    .priv_data_size = sizeof(BethsoftvidContext),
    .init           = bethsoftvid_decode_init,
    .close          = bethsoftvid_decode_end,
    .decode         = bethsoftvid_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
    .caps_internal  = FF_CODEC_CAP_INIT_THREADSAFE,
};
