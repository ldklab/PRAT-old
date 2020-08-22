/*
 * AV1 common parsing code
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

#ifndef AVCODEC_AV1_PARSE_H
#define AVCODEC_AV1_PARSE_H

#include <stdint.h>

#include "av1.h"
#include "avcodec.h"
#include "get_bits.h"

// OBU header fields + max leb128 length
#define MAX_OBU_HEADER_SIZE (2 + 8)

typedef struct AV1OBU {
    /** Size of payload */
    int size;
    const uint8_t *data;

    /**
     * Size, in bits, of just the data, excluding the trailing_one_bit and
     * any trailing padding.
     */
    int size_bits;

    /** Size of entire OBU, including header */
    int raw_size;
    const uint8_t *raw_data;

    /** GetBitContext initialized to the start of the payload */
    GetBitContext gb;

    int type;

    int temporal_id;
    int spatial_id;
} AV1OBU;

/** An input packet split into OBUs */
typedef struct AV1Packet {
    AV1OBU *obus;
    int nb_obus;
    int obus_allocated;
    unsigned obus_allocated_size;
} AV1Packet;

/**
 * Extract an OBU from a raw bitstream.
 *
 * @note This function does not copy or store any bitstream data. All
 *       the pointers in the AV1OBU structure will be valid as long
 *       as the input buffer also is.
 */
int ff_av1_extract_obu(AV1OBU *obu, const uint8_t *buf, int length,
                       void *logctx);

/**
 * Split an input packet into OBUs.
 *
 * @note This function does not copy or store any bitstream data. All
 *       the pointers in the AV1Packet structure will be valid as
 *       long as the input buffer also is.
 */
int ff_av1_packet_split(AV1Packet *pkt, const uint8_t *buf, int length,
                        void *logctx);

/**
 * Free all the allocated memory in the packet.
 */
void ff_av1_packet_uninit(AV1Packet *pkt);


            break;
    }
}

                                   int64_t *obu_size, int *start_pos, int *type,
                                   int *temporal_id, int *spatial_id)
{

        return ret;

        return AVERROR_INVALIDDATA;


    } else {
    }


        return AVERROR_INVALIDDATA;



        return AVERROR_INVALIDDATA;

}

{

    /* There are no trailing bits on these */
        type == AV1_OBU_FRAME) {
        if (size > INT_MAX / 8)
            return AVERROR(ERANGE);
        else
            return size * 8;
    }

        size--;

        return 0;


        return AVERROR(ERANGE);

    /* Remove the trailing_one_bit and following trailing zeros */

    return size;
}

#endif /* AVCODEC_AV1_PARSE_H */
