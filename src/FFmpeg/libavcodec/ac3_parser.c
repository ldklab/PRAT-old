/*
 * AC-3 parser
 * Copyright (c) 2003 Fabrice Bellard
 * Copyright (c) 2003 Michael Niedermayer
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

#include "config.h"

#include "libavutil/channel_layout.h"
#include "parser.h"
#include "ac3_parser.h"
#include "ac3_parser_internal.h"
#include "aac_ac3_parser.h"
#include "get_bits.h"


#define AC3_HEADER_SIZE 7

#if CONFIG_AC3_PARSER

static const uint8_t eac3_blocks[4] = {
    1, 2, 3, 6
};

/**
 * Table for center mix levels
 * reference: Section 5.4.2.4 cmixlev
 */
static const uint8_t center_levels[4] = { 4, 5, 6, 5 };

/**
 * Table for surround mix levels
 * reference: Section 5.4.2.5 surmixlev
 */
static const uint8_t surround_levels[4] = { 4, 6, 7, 6 };


{


        return AAC_AC3_PARSE_ERROR_SYNC;

    /* read ahead to bsid to distinguish between AC-3 and E-AC-3 */
        return AAC_AC3_PARSE_ERROR_BSID;


    /* set default mix levels */

    /* set default dolby surround mode */

        /* Normal AC-3 */
            return AAC_AC3_PARSE_ERROR_SAMPLE_RATE;

            return AAC_AC3_PARSE_ERROR_FRAME_SIZE;



        } else {
        }

    } else {
        /* Enhanced AC-3 */
            return AAC_AC3_PARSE_ERROR_FRAME_TYPE;


            return AAC_AC3_PARSE_ERROR_FRAME_SIZE;

                return AAC_AC3_PARSE_ERROR_SAMPLE_RATE;
        } else {
        }


    }

    return 0;
}

// TODO: Better way to pass AC3HeaderInfo fields to mov muxer.
                            size_t size)
{

        return AVERROR(ENOMEM);

        return AVERROR_INVALIDDATA;
        return AVERROR_INVALIDDATA;

}

                        uint8_t *bitstream_id, uint16_t *frame_size)
{

        return AVERROR_INVALIDDATA;


}

        int *need_next_header, int *new_frame_start)
{
        uint64_t u64;
        uint8_t  u8[8 + AV_INPUT_BUFFER_PADDING_SIZE];

        FFSWAP(uint8_t, tmp.u8[1], tmp.u8[2]);
        FFSWAP(uint8_t, tmp.u8[3], tmp.u8[4]);
        FFSWAP(uint8_t, tmp.u8[5], tmp.u8[6]);
    }


        return 0;

        hdr_info->service_type = AV_AUDIO_SERVICE_TYPE_KARAOKE;

}

{
}


AVCodecParser ff_ac3_parser = {
    .codec_ids      = { AV_CODEC_ID_AC3, AV_CODEC_ID_EAC3 },
    .priv_data_size = sizeof(AACAC3ParseContext),
    .parser_init    = ac3_parse_init,
    .parser_parse   = ff_aac_ac3_parse,
    .parser_close   = ff_parse_close,
};

#else

int avpriv_ac3_parse_header(AC3HeaderInfo **phdr, const uint8_t *buf,
                            size_t size)
{
    return AVERROR(ENOSYS);
}

int av_ac3_parse_header(const uint8_t *buf, size_t size,
                        uint8_t *bitstream_id, uint16_t *frame_size)
{
    return AVERROR(ENOSYS);
}
#endif
