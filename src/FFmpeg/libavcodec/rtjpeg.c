/*
 * RTJpeg decoding functions
 * Copyright (c) 2006 Reimar Doeffinger
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
#include "libavutil/common.h"
#include "get_bits.h"
#include "rtjpeg.h"

#define PUT_COEFF(c) \
    i = scan[coeff--]; \
    block[i] = (c) * quant[i];

/// aligns the bitstream to the given power of two
#define ALIGN(a) \
    n = (-get_bits_count(gb)) & (a - 1); \
    if (n) {skip_bits(gb, n);}

/**
 * @brief read one block from stream
 * @param gb contains stream data
 * @param block where data is written to
 * @param scan array containing the mapping stream address -> block position
 * @param quant quantization factors
 * @return 0 means the block is not coded, < 0 means an error occurred.
 *
 * Note: GetBitContext is used to make the code simpler, since all data is
 * aligned this could be done faster in a different way, e.g. as it is done
 * in MPlayer libmpcodecs/native/rtjpegn.c.
 */
                            const uint32_t *quant) {

    // block not coded
       return 0;

    // number of non-zero coefficients
        return AVERROR_INVALIDDATA;

    // normally we would only need to clear the (63 - coeff) last values,
    // but since we do not know where they are we just clear the whole block

    // 2 bits per coefficient
            break; // continue with more bits
    }

    // 4 bits per coefficient
        return AVERROR_INVALIDDATA;
            break; // continue with more bits
    }

    // 8 bits per coefficient
        return AVERROR_INVALIDDATA;
    }

}

/**
 * @brief decode one rtjpeg YUV420 frame
 * @param c context, must be initialized via ff_rtjpeg_decode_init
 * @param f AVFrame to place decoded frame into. If parts of the frame
 *          are not coded they are left unchanged, so consider initializing it
 * @param buf buffer containing input data
 * @param buf_size length of input data in bytes
 * @return number of bytes consumed from the input buffer
 */
                                  const uint8_t *buf, int buf_size) {

        return ret;

#define BLOCK(quant, dst, stride) do { \
    int res = get_block(&gb, block, c->scan, quant); \
    if (res < 0) \
        return res; \
    if (res > 0) \
        c->idsp.idct_put(dst, stride, block); \
} while (0)
        }
    }
}

/**
 * @brief initialize an RTJpegContext, may be called multiple times
 * @param c context to initialize
 * @param width width of image, will be rounded down to the nearest multiple
 *              of 16 for decoding
 * @param height height of image, will be rounded down to the nearest multiple
 *              of 16 for decoding
 * @param lquant luma quantization table to use
 * @param cquant chroma quantization table to use
 */
                           const uint32_t *lquant, const uint32_t *cquant) {
    }

{



        // permute the scan and quantization tables for the chosen idct
    }
