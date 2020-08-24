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

#include <stdint.h>

#include "config.h"
#include "libavutil/attributes.h"
#include "libavutil/intreadwrite.h"
#include "avcodec.h"
#include "pixblockdsp.h"

                            ptrdiff_t stride)
{

                           ptrdiff_t stride)
{

    /* read the pixels */
    }

                          const uint8_t *s2, ptrdiff_t stride)
{

    /* read the pixels */
    }

{


    case 10:
    case 12:
    case 14:
        }
        break;
    }

        ff_pixblockdsp_init_aarch64(c, avctx, high_bit_depth);
        ff_pixblockdsp_init_alpha(c, avctx, high_bit_depth);
        ff_pixblockdsp_init_arm(c, avctx, high_bit_depth);
        ff_pixblockdsp_init_ppc(c, avctx, high_bit_depth);
        ff_pixblockdsp_init_mips(c, avctx, high_bit_depth);
