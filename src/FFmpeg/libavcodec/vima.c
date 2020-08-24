/*
 * LucasArts VIMA decoder
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

/**
 * @file
 * LucasArts VIMA audio decoder
 * @author Paul B Mahol
 */

#include "libavutil/channel_layout.h"

#include "adpcm_data.h"
#include "avcodec.h"
#include "get_bits.h"
#include "internal.h"

static int predict_table_init = 0;
static uint16_t predict_table[5786 * 2];

static const uint8_t size_table[] = {
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
};

static const int8_t index_table1[] = {
    -1, 4, -1, 4
};

static const int8_t index_table2[] = {
    -1, -1, 2, 6, -1, -1, 2, 6
};

static const int8_t index_table3[] = {
    -1, -1, -1, -1, 1, 2, 4, 6, -1, -1, -1, -1, 1, 2, 4, 6
};

static const int8_t index_table4[] = {
    -1, -1, -1, -1, -1, -1, -1, -1, 1,  1,  1,  2,  2,  4,  5,  6,
    -1, -1, -1, -1, -1, -1, -1, -1, 1,  1,  1,  2,  2,  4,  5,  6
};

static const int8_t index_table5[] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
     1,  1,  1,  1,  1,  2,  2,  2,  2,  4,  4,  4,  5,  5,  6,  6,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
     1,  1,  1,  1,  1,  2,  2,  2,  2,  4,  4,  4,  5,  5,  6,  6
};

static const int8_t index_table6[] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,
     2,  2,  4,  4,  4,  4,  4,  4,  5,  5,  5,  5,  6,  6,  6,  6,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,
     2,  2,  4,  4,  4,  4,  4,  4,  5,  5,  5,  5,  6,  6,  6,  6
};

static const int8_t *const step_index_tables[] = {
    index_table1, index_table2, index_table3,
    index_table4, index_table5, index_table6
};

{


        return 0;



            }
        }
    }

}

                        int *got_frame_ptr, AVPacket *pkt)
{

        return AVERROR_INVALIDDATA;

        return ret;

    }

        return AVERROR_INVALIDDATA;

    }
                                            : AV_CH_LAYOUT_MONO;
    }

        return ret;




            else
                highbit = 0;

            } else {


            }


        }
    }


}

AVCodec ff_adpcm_vima_decoder = {
    .name         = "adpcm_vima",
    .long_name    = NULL_IF_CONFIG_SMALL("LucasArts VIMA audio"),
    .type         = AVMEDIA_TYPE_AUDIO,
    .id           = AV_CODEC_ID_ADPCM_VIMA,
    .init         = decode_init,
    .decode       = decode_frame,
    .capabilities = AV_CODEC_CAP_DR1,
};
