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

#include "config.h"
#include "libavutil/attributes.h"
#include "libavutil/common.h"
#include "avcodec.h"
#include "dct.h"
#include "faanidct.h"
#include "idctdsp.h"
#include "simple_idct.h"
#include "xvididct.h"

                               const uint8_t *src_scantable)
{


    }

    end = -1;
            end = j;
    }

                                           enum idct_permutation_type perm_type)
{

                                              perm_type))
            return;

    case FF_IDCT_PERM_NONE:
        break;
    case FF_IDCT_PERM_LIBMPEG2:
        break;
    case FF_IDCT_PERM_TRANSPOSE:
        break;
    case FF_IDCT_PERM_PARTTRANS:
        for (i = 0; i < 64; i++)
            idct_permutation[i] = (i & 0x24) | ((i & 3) << 3) | ((i >> 3) & 3);
        break;
    default:
        av_log(NULL, AV_LOG_ERROR,
               "Internal error, IDCT permutation not set\n");
    }
}

                             ptrdiff_t line_size)
{

    /* read the pixels */

    }

                                 int line_size)
{

    /* read the pixels */

    }

static void put_pixels_clamped2_c(const int16_t *block, uint8_t *av_restrict pixels,
                                 int line_size)
{
    int i;

    /* read the pixels */
    for(i=0;i<2;i++) {
        pixels[0] = av_clip_uint8(block[0]);
        pixels[1] = av_clip_uint8(block[1]);

        pixels += line_size;
        block += 8;
    }
}

static void put_signed_pixels_clamped_c(const int16_t *block,
                                        uint8_t *av_restrict pixels,
                                        ptrdiff_t line_size)
{
    int i, j;

    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            if (*block < -128)
                *pixels = 0;
            else if (*block > 127)
                *pixels = 255;
            else
                *pixels = (uint8_t) (*block + 128);
            block++;
            pixels++;
        }
        pixels += (line_size - 8);
    }
}

                             ptrdiff_t line_size)
{

    /* read the pixels */
    }

                          int line_size)
{

    /* read the pixels */
    }

static void add_pixels_clamped2_c(const int16_t *block, uint8_t *av_restrict pixels,
                          int line_size)
{
    int i;

    /* read the pixels */
    for(i=0;i<2;i++) {
        pixels[0] = av_clip_uint8(pixels[0] + block[0]);
        pixels[1] = av_clip_uint8(pixels[1] + block[1]);
        pixels += line_size;
        block += 8;
    }
}

{
{

static void ff_jref_idct2_put(uint8_t *dest, ptrdiff_t line_size, int16_t *block)
{
    ff_j_rev_dct2 (block);
    put_pixels_clamped2_c(block, dest, line_size);
}
static void ff_jref_idct2_add(uint8_t *dest, ptrdiff_t line_size, int16_t *block)
{
    ff_j_rev_dct2 (block);
    add_pixels_clamped2_c(block, dest, line_size);
}

static void ff_jref_idct1_put(uint8_t *dest, ptrdiff_t line_size, int16_t *block)
{
    dest[0] = av_clip_uint8((block[0] + 4)>>3);
}
static void ff_jref_idct1_add(uint8_t *dest, ptrdiff_t line_size, int16_t *block)
{
    dest[0] = av_clip_uint8(dest[0] + ((block[0] + 4)>>3));
}

{

        c->idct_put  = ff_jref_idct2_put;
        c->idct_add  = ff_jref_idct2_add;
        c->idct      = ff_j_rev_dct2;
        c->perm_type = FF_IDCT_PERM_NONE;
        c->idct_put  = ff_jref_idct1_put;
        c->idct_add  = ff_jref_idct1_add;
        c->idct      = ff_j_rev_dct1;
        c->perm_type = FF_IDCT_PERM_NONE;
    } else {
            /* 10-bit MPEG-4 Simple Studio Profile requires a higher precision IDCT
               However, it only uses idct_put */
                c->idct_put              = ff_simple_idct_put_int32_10bit;
                c->idct_add              = NULL;
                c->idct                  = NULL;
            } else {
            }
        } else {
#if CONFIG_FAANIDCT
                c->idct_put  = ff_faanidct_put;
                c->idct_add  = ff_faanidct_add;
                c->idct      = ff_faanidct;
                c->perm_type = FF_IDCT_PERM_NONE;
#endif /* CONFIG_FAANIDCT */
            } else { // accurate/default
                /* Be sure FF_IDCT_NONE will select this one, since it uses FF_IDCT_PERM_NONE */
            }
        }
    }



        ff_idctdsp_init_aarch64(c, avctx, high_bit_depth);
        ff_idctdsp_init_alpha(c, avctx, high_bit_depth);
        ff_idctdsp_init_arm(c, avctx, high_bit_depth);
        ff_idctdsp_init_ppc(c, avctx, high_bit_depth);
        ff_idctdsp_init_mips(c, avctx, high_bit_depth);

                                  c->perm_type);
