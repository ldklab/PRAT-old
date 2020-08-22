/*
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
 *@brief IntraX8 frame subdecoder image manipulation routines
 */

#include "intrax8dsp.h"
#include "libavutil/common.h"

/*
 * area positions, #3 is 1 pixel only, other are 8 pixels
 *    |66666666|
 *   3|44444444|55555555|
 * - -+--------+--------+
 * 1 2|XXXXXXXX|
 * 1 2|XXXXXXXX|
 * 1 2|XXXXXXXX|
 * 1 2|XXXXXXXX|
 * 1 2|XXXXXXXX|
 * 1 2|XXXXXXXX|
 * 1 2|XXXXXXXX|
 * 1 2|XXXXXXXX|
 * ^-start
 */

#define area1 (0)
#define area2 (8)
#define area3 (8 + 8)
#define area4 (8 + 8 + 1)
#define area5 (8 + 8 + 1 + 8)
#define area6 (8 + 8 + 1 + 16)

/**
 Collect statistics and prepare the edge pixels required by the other spatial compensation functions.

 * @param src pointer to the beginning of the processed block
 * @param dst pointer to emu_edge, edge pixels are stored the way other compensation routines do.
 * @param linesize byte offset between 2 vertical pixels in the source image
 * @param range pointer to the variable where the edge pixel range is to be stored (max-min values)
 * @param psum  pointer to the variable where the edge pixel sum is to be stored
 * @param edges Informs this routine that the block is on an image border, so it has to interpolate the missing edge pixels.
                and some of the edge pixels should be interpolated, the flag has the following meaning:
                1   - mb_x==0 - first block in the row, interpolate area #1,#2,#3;
                2   - mb_y==0 - first row, interpolate area #3,#4,#5,#6;
        note:   1|2 - mb_x==mb_y==0 - first block, use 0x80 value for all areas;
                4   - mb_x>= (mb_width-1) last block in the row, interpolate area #5;
-*/
                                          ptrdiff_t stride, int *range,
                                          int *psum, int edges)
{

        /* this triggers flat_dc for sure. flat_dc avoids all (other)
         * prediction modes, but requires dc_level decoding. */
    }





        }
    }

        }
        } else {
        }
        // area6 always present in the above block
    }
    // now calculate the stuff we need

        else // implies y == 0 x != 0

    } else {
        // the edge pixel, in the top line and left column
        // edge pixel is not part of min/max
    }
}

static const uint16_t zero_prediction_weights[64 * 2] = {
    640,  640, 669,  480, 708,  354, 748, 257,
    792,  198, 760,  143, 808,  101, 772,  72,
    480,  669, 537,  537, 598,  416, 661, 316,
    719,  250, 707,  185, 768,  134, 745,  97,
    354,  708, 416,  598, 488,  488, 564, 388,
    634,  317, 642,  241, 716,  179, 706, 132,
    257,  748, 316,  661, 388,  564, 469, 469,
    543,  395, 571,  311, 655,  238, 660, 180,
    198,  792, 250,  719, 317,  634, 395, 543,
    469,  469, 507,  380, 597,  299, 616, 231,
    161,  855, 206,  788, 266,  710, 340, 623,
    411,  548, 455,  455, 548,  366, 576, 288,
    122,  972, 159,  914, 211,  842, 276, 758,
    341,  682, 389,  584, 483,  483, 520, 390,
    110, 1172, 144, 1107, 193, 1028, 254, 932,
    317,  846, 366,  731, 458,  611, 499, 499,
};

{

        }
    }

        }
    }
        }
    }
        }
    }

    }
    }

{

    }

{

    }

{

    }

{

    }

{

            else
        }
    }

{

    }

{

            else
        }
    }

{

    }

{

    }

{

    }

{

    }

                           const ptrdiff_t b_stride, int quant)
{



        // You need at least 1 to be able to reach a total score of 6.

                    }
                }
            }
        }
        {






                        x = m;


                }
            }
        }
    }

{

{

{
