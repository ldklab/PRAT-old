/*
 * MJPEG encoder
 * Copyright (c) 2016 William Ma, Ted Ying, Jerry Jiang
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

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "libavutil/avassert.h"
#include "libavutil/common.h"
#include "libavutil/error.h"
#include "libavutil/qsort.h"
#include "mjpegenc_huffman.h"

/**
 * Comparison function for two PTables by prob
 *
 * @param a First PTable to compare
 * @param b Second PTable to compare
 * @return < 0 for less than, 0 for equals, > 0 for greater than
 */
{
}

/**
 * Comparison function for two HuffTables by length
 *
 * @param a First HuffTable to compare
 * @param b Second HuffTable to compare
 * @return < 0 for less than, 0 for equals, > 0 for greater than
 */
{
}

/**
 * Computes the length of the Huffman encoding for each distinct input value.
 * Uses package merge algorithm as follows:
 * 1. start with an empty list, lets call it list(0), set i = 0
 * 2. add 1 entry to list(i) for each symbol we have and give each a score equal to the probability of the respective symbol
 * 3. merge the 2 symbols of least score and put them in list(i+1), and remove them from list(i). The new score will be the sum of the 2 scores
 * 4. if there is more than 1 symbol left in the current list(i), then goto 3
 * 5. i++
 * 6. if i < 16 goto 2
 * 7. select the n-1 elements in the last list with the lowest score (n = the number of symbols)
 * 8. the length of the huffman code for symbol s will be equal to the number of times the symbol occurs in the select elements
 * Go to guru.multimedia.cx/small-tasks-for-ffmpeg/ for more details
 *
 * All probabilities should be positive integers. The output is sorted by code,
 * not by length.
 *
 * @param prob_table input array of a PTable for each distinct input value
 * @param distincts  output array of a HuffTable that will be populated by this function
 * @param size       size of the prob_table array
 * @param max_length max length of an encoding
 */
{








        }
            } else {
                }
            }
        }
    }

    }
    // we don't want to return the 256 bit count (it was just in here to prevent
    // all 1s encoding)
    j = 0;
        }
    }

{

/**
 * Produces a Huffman encoding with a given input
 *
 * @param s         input to encode
 * @param bits      output array where the ith character represents how many input values have i length encoding
 * @param val       output array of input values sorted by their encoded length
 * @param max_nval  maximum number of distinct input values
 */
                                   uint8_t val[], int max_nval)
{

    }

    j = 0;
        }
    }

    }
