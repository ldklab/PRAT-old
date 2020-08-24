/*
 * Westwood SNDx codecs
 * Copyright (c) 2005 Konstantin Shishkov
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

#include <stdint.h>

#include "libavutil/channel_layout.h"
#include "libavutil/common.h"
#include "libavutil/intreadwrite.h"
#include "avcodec.h"
#include "internal.h"

/**
 * @file
 * Westwood SNDx codecs
 *
 * Reference documents about VQA format and its audio codecs
 * can be found here:
 * http://www.multimedia.cx
 */

static const int8_t ws_adpcm_4bit[] = {
    -9, -8, -6, -5, -4, -3, -2, -1,
     0,  1,  2,  3,  4,  5,  6,  8
};

{

}

                               int *got_frame_ptr, AVPacket *avpkt)
{


        return 0;

        av_log(avctx, AV_LOG_ERROR, "packet is too small\n");
        return AVERROR(EINVAL);
    }


        av_log(avctx, AV_LOG_ERROR, "Frame data is larger than input buffer\n");
        return AVERROR_INVALIDDATA;
    }

    /* get output buffer */
        return ret;

        memcpy(samples, buf, out_size);
        *got_frame_ptr = 1;
        return buf_size;
    }


        /* make sure we don't write past the output buffer */
        }
            break;

        /* make sure we don't read past the input buffer */
            break;

            }
            break;
            }
            break;
            } else { /* copy */
            }
            break;
        }
    }


}

AVCodec ff_ws_snd1_decoder = {
    .name           = "ws_snd1",
    .long_name      = NULL_IF_CONFIG_SMALL("Westwood Audio (SND1)"),
    .type           = AVMEDIA_TYPE_AUDIO,
    .id             = AV_CODEC_ID_WESTWOOD_SND1,
    .init           = ws_snd_decode_init,
    .decode         = ws_snd_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
