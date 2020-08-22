/*
 * DCA compatible decoder data
 * Copyright (C) 2004 Gildas Bazin
 * Copyright (C) 2004 Benjamin Zores
 * Copyright (C) 2006 Benjamin Larsson
 * Copyright (C) 2007 Konstantin Shishkov
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
#include <string.h>

#include "libavutil/error.h"

#include "dca.h"
#include "dca_core.h"
#include "dca_syncwords.h"
#include "get_bits.h"
#include "put_bits.h"

const uint32_t avpriv_dca_sample_rates[16] = {
    0, 8000, 16000, 32000, 0, 0, 11025, 22050, 44100, 0, 0,
    12000, 24000, 48000, 96000, 192000
};

const uint32_t ff_dca_sampling_freqs[16] = {
      8000,  16000, 32000, 64000, 128000, 22050,  44100,  88200,
    176400, 352800, 12000, 24000,  48000, 96000, 192000, 384000,
};

const uint8_t ff_dca_freq_ranges[16] = {
    0, 1, 2, 3, 4, 1, 2, 3, 4, 4, 0, 1, 2, 3, 4, 4
};

const uint8_t ff_dca_bits_per_sample[8] = {
    16, 16, 20, 20, 0, 24, 24, 0
};

                             int max_size)
{

        src_size = max_size;

    case DCA_SYNCWORD_SUBSTREAM:
    case DCA_SYNCWORD_CORE_LE:
        for (i = 0; i < (src_size + 1) >> 1; i++) {
            AV_WB16(dst, AV_RL16(src));
            src += 2;
            dst += 2;
        }
        return src_size;
    case DCA_SYNCWORD_CORE_14B_BE:
    case DCA_SYNCWORD_CORE_14B_LE:
        init_put_bits(&pb, dst, max_size);
        for (i = 0; i < (src_size + 1) >> 1; i++, src += 2) {
            tmp = ((mrk == DCA_SYNCWORD_CORE_14B_BE) ? AV_RB16(src) : AV_RL16(src)) & 0x3FFF;
            put_bits(&pb, 14, tmp);
        }
        flush_put_bits(&pb);
        return (put_bits_count(&pb) + 7) >> 3;
    default:
        return AVERROR_INVALIDDATA;
    }
}

{
        return DCA_PARSE_ERROR_SYNC_WORD;

        return DCA_PARSE_ERROR_DEFICIT_SAMPLES;

        return DCA_PARSE_ERROR_PCM_BLOCKS;

        return DCA_PARSE_ERROR_FRAME_SIZE;

        return DCA_PARSE_ERROR_AMODE;

        return DCA_PARSE_ERROR_SAMPLE_RATE;

        return DCA_PARSE_ERROR_RESERVED_BIT;

        return DCA_PARSE_ERROR_LFE_FLAG;

        skip_bits(gb, 16);
        return DCA_PARSE_ERROR_PCM_RES;

}

{

        return ret;

        return AVERROR_INVALIDDATA;

    return 0;
}
