/*
 * Common code between the AC-3 encoder and decoder
 * Copyright (c) 2000 Fabrice Bellard
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
 * Common code between the AC-3 encoder and decoder.
 */

#include "libavutil/common.h"

#include "avcodec.h"
#include "ac3.h"

/**
 * Starting frequency coefficient bin for each critical band.
 */
const uint8_t ff_ac3_band_start_tab[AC3_CRITICAL_BANDS+1] = {
      0,  1,   2,   3,   4,   5,   6,   7,   8,   9,
     10,  11, 12,  13,  14,  15,  16,  17,  18,  19,
     20,  21, 22,  23,  24,  25,  26,  27,  28,  31,
     34,  37, 40,  43,  46,  49,  55,  61,  67,  73,
     79,  85, 97, 109, 121, 133, 157, 181, 205, 229, 253
};

/**
 * Map each frequency coefficient bin to the critical band that contains it.
 */
const uint8_t ff_ac3_bin_to_band_tab[253] = {
     0,
     1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12,
    13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
    25, 26, 27, 28, 28, 28, 29, 29, 29, 30, 30, 30,
    31, 31, 31, 32, 32, 32, 33, 33, 33, 34, 34, 34,
    35, 35, 35, 35, 35, 35, 36, 36, 36, 36, 36, 36,
    37, 37, 37, 37, 37, 37, 38, 38, 38, 38, 38, 38,
    39, 39, 39, 39, 39, 39, 40, 40, 40, 40, 40, 40,
    41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
    42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
    43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
    44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
    45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45,
    45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45,
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46,
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46,
    47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47,
    47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47,
    48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
    48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
    49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
    49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49
};

{
        a = c;
    }
}

{
    } else {
    }
}

                               int16_t *band_psd)
{

    /* exponent mapping to PSD */
    }

    /* PSD integration */
            /* logadd */
        }

                               int start, int end, int fast_gain, int is_lfe,
                               int dba_mode, int dba_nsegs, uint8_t *dba_offsets,
                               uint8_t *dba_lengths, uint8_t *dba_values,
                               int16_t *mask)
{

        return AVERROR_INVALIDDATA;

    /* excitation function */

                }
            }
        }

        }
        begin = 22;
    } else {
        /* coupling channel */
    }

    }

    /* compute masking curve */

        }
    }

    /* delta bit allocation */

        int i, seg, delta;
        if (dba_nsegs > 8)
            return -1;
        band = band_start;
        for (seg = 0; seg < dba_nsegs; seg++) {
            band += dba_offsets[seg];
            if (band >= AC3_CRITICAL_BANDS || dba_lengths[seg] > AC3_CRITICAL_BANDS-band)
                return -1;
            if (dba_values[seg] >= 4) {
                delta = (dba_values[seg] - 3) * 128;
            } else {
                delta = (dba_values[seg] - 4) * 128;
            }
            for (i = 0; i < dba_lengths[seg]; i++) {
                mask[band++] += delta;
            }
        }
    }
    return 0;
}
