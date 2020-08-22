/*
 * MLP codec common code
 * Copyright (c) 2007-2008 Ian Caulfield
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

#include "libavutil/crc.h"
#include "libavutil/intreadwrite.h"
#include "mlp.h"

const uint8_t ff_mlp_huffman_tables[3][18][2] = {
    {    /* Huffman table 0, -7 - +10 */
        {0x01, 9}, {0x01, 8}, {0x01, 7}, {0x01, 6}, {0x01, 5}, {0x01, 4}, {0x01, 3},
        {0x04, 3}, {0x05, 3}, {0x06, 3}, {0x07, 3},
        {0x03, 3}, {0x05, 4}, {0x09, 5}, {0x11, 6}, {0x21, 7}, {0x41, 8}, {0x81, 9},
    }, { /* Huffman table 1, -7 - +8 */
        {0x01, 9}, {0x01, 8}, {0x01, 7}, {0x01, 6}, {0x01, 5}, {0x01, 4}, {0x01, 3},
        {0x02, 2}, {0x03, 2},
        {0x03, 3}, {0x05, 4}, {0x09, 5}, {0x11, 6}, {0x21, 7}, {0x41, 8}, {0x81, 9},
    }, { /* Huffman table 2, -7 - +7 */
        {0x01, 9}, {0x01, 8}, {0x01, 7}, {0x01, 6}, {0x01, 5}, {0x01, 4}, {0x01, 3},
        {0x01, 1},
        {0x03, 3}, {0x05, 4}, {0x09, 5}, {0x11, 6}, {0x21, 7}, {0x41, 8}, {0x81, 9},
    }
};

const ChannelInformation ff_mlp_ch_info[21] = {
    { 0x01, 0x01, 0x00, 0x1f }, { 0x03, 0x02, 0x00, 0x1b },
    { 0x07, 0x02, 0x01, 0x1f }, { 0x0F, 0x02, 0x02, 0x19 },
    { 0x07, 0x02, 0x01, 0x03 }, { 0x0F, 0x02, 0x02, 0x1f },
    { 0x1F, 0x02, 0x03, 0x01 }, { 0x07, 0x02, 0x01, 0x1a },
    { 0x0F, 0x02, 0x02, 0x1f }, { 0x1F, 0x02, 0x03, 0x18 },
    { 0x0F, 0x02, 0x02, 0x02 }, { 0x1F, 0x02, 0x03, 0x1f },
    { 0x3F, 0x02, 0x04, 0x00 }, { 0x0F, 0x03, 0x01, 0x1f },
    { 0x1F, 0x03, 0x02, 0x18 }, { 0x0F, 0x03, 0x01, 0x02 },
    { 0x1F, 0x03, 0x02, 0x1f }, { 0x3F, 0x03, 0x03, 0x00 },
    { 0x1F, 0x04, 0x01, 0x01 }, { 0x1F, 0x04, 0x01, 0x18 },
    { 0x3F, 0x04, 0x02, 0x00 },
};

const uint64_t ff_mlp_channel_layouts[12] = {
    AV_CH_LAYOUT_MONO, AV_CH_LAYOUT_STEREO, AV_CH_LAYOUT_2_1,
    AV_CH_LAYOUT_QUAD, AV_CH_LAYOUT_2POINT1, AV_CH_LAYOUT_SURROUND,
    AV_CH_LAYOUT_4POINT0, AV_CH_LAYOUT_5POINT0_BACK, AV_CH_LAYOUT_3POINT1,
    AV_CH_LAYOUT_4POINT1, AV_CH_LAYOUT_5POINT1_BACK, 0,
};

static int crc_init = 0;
#if CONFIG_SMALL
#define CRC_TABLE_SIZE 257
#else
#define CRC_TABLE_SIZE 1024
#endif
static AVCRC crc_63[CRC_TABLE_SIZE];
static AVCRC crc_1D[CRC_TABLE_SIZE];
static AVCRC crc_2D[CRC_TABLE_SIZE];

{
    }

{

}

{
}

{


    }

}

{




}
