/*
 * MPEG-4 Audio common code
 * Copyright (c) 2008 Baptiste Coudurier <baptiste.coudurier@free.fr>
 * Copyright (c) 2009 Alex Converse <alex.converse@gmail.com>
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

#include "get_bits.h"
#include "put_bits.h"
#include "mpeg4audio.h"

/**
 * Parse MPEG-4 audio configuration for ALS object type.
 * @param[in] gb       bit reader context
 * @param[in] c        MPEG4AudioConfig structure to fill
 * @return on success 0 is returned, otherwise a value < 0
 */
{
        return AVERROR_INVALIDDATA;

        return AVERROR_INVALIDDATA;

    // override AudioSpecificConfig channel configuration and sample rate
    // which are buggy in old ALS conformance files

        av_log(NULL, AV_LOG_ERROR, "Invalid sample rate %d\n", c->sample_rate);
        return AVERROR_INVALIDDATA;
    }

    // skip number of samples

    // read number of channels

}

/* XXX: make sure to update the copies in the different encoders if you change
 * this table */
const int avpriv_mpeg4audio_sample_rates[16] = {
    96000, 88200, 64000, 48000, 44100, 32000,
    24000, 22050, 16000, 12000, 11025, 8000, 7350
};

const uint8_t ff_mpeg4audio_channels[14] = {
    0,
    1, // mono (1/0)
    2, // stereo (2/0)
    3, // 3/0
    4, // 3/1
    5, // 3/2
    6, // 3/2.1
    8, // 5/2.1
    0,
    0,
    0,
    7, // 3/3.1
    8, // 3/2/2.1
    24 // 3/3/3 - 5/2/3 - 3/0/0.2
};

{
}

{
        avpriv_mpeg4audio_sample_rates[*index];
}

                                int sync_extension, void *logctx)
{
    else {
        av_log(logctx, AV_LOG_ERROR, "Invalid chan_config %d\n", c->chan_config);
        return AVERROR_INVALIDDATA;
    }
        // check for W6132 Annex YYYY draft MP3onMP4
            c->ext_chan_config = get_bits(gb, 4);
    } else {
    }

            skip_bits(gb, 24);


            return ret;
    }

                }
                break;
            } else
        }
    }

    //PS requires SBR
    //Limit implicit PS to the HE-AACv2 Profile

}

#if LIBAVCODEC_VERSION_MAJOR < 59
int avpriv_mpeg4audio_get_config(MPEG4AudioConfig *c, const uint8_t *buf,
                                 int bit_size, int sync_extension)
{
    GetBitContext gb;
    int ret;

    if (bit_size <= 0)
        return AVERROR_INVALIDDATA;

    ret = init_get_bits(&gb, buf, bit_size);
    if (ret < 0)
        return ret;

    return ff_mpeg4audio_get_config_gb(c, &gb, sync_extension, NULL);
}
#endif

                                  int size, int sync_extension, void *logctx)
{

        return AVERROR_INVALIDDATA;

        return ret;

}
