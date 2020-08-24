/*
 * common code shared by all WMA variants
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

#include "libavutil/attributes.h"

#include "wma_common.h"

/**
 *@brief Get the samples per frame for this stream.
 *@param sample_rate output sample_rate
 *@param version wma version
 *@param decode_flags codec compression features
 *@return log2 of the number of output samples per frame
 */
                                      unsigned int decode_flags)
{

        frame_len_bits = 9;
        frame_len_bits = 10;
        frame_len_bits = 11;
    else if (sample_rate <= 96000)
        frame_len_bits = 12;
    else
        frame_len_bits = 13;

            ++frame_len_bits;
            --frame_len_bits;
            frame_len_bits -= 2;
    }

}
