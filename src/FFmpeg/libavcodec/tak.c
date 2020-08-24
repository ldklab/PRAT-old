/*
 * TAK common code
 * Copyright (c) 2012 Paul B Mahol
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

#include "libavutil/crc.h"
#include "libavutil/intreadwrite.h"

#define BITSTREAM_READER_LE
#include "tak.h"

static const int64_t tak_channel_layouts[] = {
    0,
    AV_CH_FRONT_LEFT,
    AV_CH_FRONT_RIGHT,
    AV_CH_FRONT_CENTER,
    AV_CH_LOW_FREQUENCY,
    AV_CH_BACK_LEFT,
    AV_CH_BACK_RIGHT,
    AV_CH_FRONT_LEFT_OF_CENTER,
    AV_CH_FRONT_RIGHT_OF_CENTER,
    AV_CH_BACK_CENTER,
    AV_CH_SIDE_LEFT,
    AV_CH_SIDE_RIGHT,
    AV_CH_TOP_CENTER,
    AV_CH_TOP_FRONT_LEFT,
    AV_CH_TOP_FRONT_CENTER,
    AV_CH_TOP_FRONT_RIGHT,
    AV_CH_TOP_BACK_LEFT,
    AV_CH_TOP_BACK_CENTER,
    AV_CH_TOP_BACK_RIGHT,
};

static const uint16_t frame_duration_type_quants[] = {
    3, 4, 6, 8, 4096, 8192, 16384, 512, 1024, 2048,
};

{

                         TAK_FRAME_DURATION_QUANT_SHIFT;
                         frame_duration_type_quants[TAK_FST_250ms] >>
                         TAK_FRAME_DURATION_QUANT_SHIFT;
    } else {
        return AVERROR_INVALIDDATA;
    }

        return AVERROR_INVALIDDATA;

    return nb_samples;
}

{

        return AVERROR_INVALIDDATA;


    return 0;
}

{



                     TAK_SAMPLE_RATE_MIN;
                     TAK_BPS_MIN;
                     TAK_CHANNELS_MIN;


            }
        }
    }


{

        return AVERROR_INVALIDDATA;


}

                               TAKStreamInfo *ti, int log_level_offset)
{
        av_log(avctx, AV_LOG_ERROR + log_level_offset, "missing sync id\n");
        return AVERROR_INVALIDDATA;
    }


    } else {
    }


    }

        return AVERROR_INVALIDDATA;


}
