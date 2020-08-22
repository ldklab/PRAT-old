/*
 * FLAC common code
 * Copyright (c) 2009 Justin Ruggles
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
#include "libavutil/crc.h"
#include "libavutil/log.h"
#include "bytestream.h"
#include "get_bits.h"
#include "flac.h"
#include "flacdata.h"

static const int8_t sample_size_table[] = { 0, 8, 12, 0, 16, 20, 24, 0 };

static const uint64_t flac_channel_layouts[8] = {
    AV_CH_LAYOUT_MONO,
    AV_CH_LAYOUT_STEREO,
    AV_CH_LAYOUT_SURROUND,
    AV_CH_LAYOUT_QUAD,
    AV_CH_LAYOUT_5POINT0,
    AV_CH_LAYOUT_5POINT1,
    AV_CH_LAYOUT_6POINT1,
    AV_CH_LAYOUT_7POINT1
};

{
}

                                FLACFrameInfo *fi, int log_level_offset)
{

    /* frame sync code */
    }

    /* variable block size stream code */

    /* block size and sample rate codes */

    /* channels and decorrelation */
    } else {
               "invalid channel mode: %d\n", fi->ch_mode);
    }

    /* bits per sample */
               "invalid sample size code (%d)\n",
               bps_code);
    }

    /* reserved bit */
               "broken stream, invalid padding\n");
    }

    /* sample or frame count */
               "sample/frame number invalid; utf8 fscked\n");
    }

    /* blocksize */
               "reserved blocksize code: 0\n");
    } else {
    }

    /* sample rate */
    } else {
               "illegal sample rate code %d\n",
               sr_code);
    }

    /* header CRC-8 check */
               "header crc mismatch\n");
    }

    return 0;
}

{
    /* Technically, there is no limit to FLAC frame size, but an encoder
       should not write a frame that is larger than if verbatim encoding mode
       were to be used. */


        /* for stereo, need to account for using decorrelation */
    } else {
    }

}

                               enum FLACExtradataFormat *format,
                               uint8_t **streaminfo_start)
{
        av_log(avctx, AV_LOG_ERROR, "extradata NULL or too small.\n");
        return 0;
    }
        /* extradata contains STREAMINFO only */
            av_log(avctx, AV_LOG_WARNING, "extradata contains %d bytes too many.\n",
                   FLAC_STREAMINFO_SIZE-avctx->extradata_size);
        }
    } else {
        if (avctx->extradata_size < 8+FLAC_STREAMINFO_SIZE) {
            av_log(avctx, AV_LOG_ERROR, "extradata too small.\n");
            return 0;
        }
        *format = FLAC_EXTRADATA_FORMAT_FULL_HEADER;
        *streaminfo_start = &avctx->extradata[8];
    }
    return 1;
}

{
    else
        avctx->channel_layout = 0;

                              const uint8_t *buffer)
{

        av_log(avctx, AV_LOG_WARNING, "invalid max blocksize: %d\n",
               s->max_blocksize);
        s->max_blocksize = 16;
        return AVERROR_INVALIDDATA;
    }



        av_log(avctx, AV_LOG_ERROR, "invalid bps: %d\n", s->bps);
        s->bps = 16;
        return AVERROR_INVALIDDATA;
    }





    return 0;
}
