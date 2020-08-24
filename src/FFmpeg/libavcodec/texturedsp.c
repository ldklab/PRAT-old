/*
 * Texture block decompression
 * Copyright (C) 2009 Benjamin Dobell, Glass Echidna
 * Copyright (C) 2012 Matthäus G. "Anteru" Chajdas (http://anteru.net)
 * Copyright (C) 2015 Vittorio Giovara <vittorio.giovara@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stddef.h>
#include <stdint.h>

#include "libavutil/attributes.h"
#include "libavutil/common.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/libm.h"

#include "texturedsp.h"

#define RGBA(r, g, b, a) (((uint8_t)(r) <<  0) | \
                          ((uint8_t)(g) <<  8) | \
                          ((uint8_t)(b) << 16) | \
                          ((unsigned)(uint8_t)(a) << 24))

                                           uint16_t color0,
                                           uint16_t color1,
                                           int dxtn, int alpha)
{



                         (2 * g0 + g1) / 3,
                         (2 * b0 + b1) / 3,
                         a);
                         (2 * g1 + g0) / 3,
                         (2 * b1 + b0) / 3,
                         a);
    } else {
                         (g0 + g1) / 2,
                         (b0 + b1) / 2,
                         a);
    }
}

                                       const uint8_t *block, uint8_t alpha)
{


        }
    }

/**
 * Decompress one block of a DXT1 texture and store the resulting
 * RGBA pixels in 'dst'. Alpha component is fully opaque.
 *
 * @param dst    output buffer.
 * @param stride scanline in bytes.
 * @param block  block to decompress.
 * @return how much texture data has been consumed.
 */
{

}

/**
 * Decompress one block of a DXT1 with 1-bit alpha texture and store
 * the resulting RGBA pixels in 'dst'. Alpha is either fully opaque or
 * fully transparent.
 *
 * @param dst    output buffer.
 * @param stride scanline in bytes.
 * @param block  block to decompress.
 * @return how much texture data has been consumed.
 */
{

}

                                       const uint8_t *block)
{





        }
    }

/** Convert a premultiplied alpha pixel to a straight alpha pixel. */
{

}

/**
 * Decompress one block of a DXT2 texture and store the resulting
 * RGBA pixels in 'dst'.
 *
 * @param dst    output buffer.
 * @param stride scanline in bytes.
 * @param block  block to decompress.
 * @return how much texture data has been consumed.
 */
{


    /* This format is DXT3, but returns premultiplied alpha. It needs to be
     * converted because it's what lavc outputs (and swscale expects). */

}

/**
 * Decompress one block of a DXT3 texture and store the resulting
 * RGBA pixels in 'dst'.
 *
 * @param dst    output buffer.
 * @param stride scanline in bytes.
 * @param block  block to decompress.
 * @return how much texture data has been consumed.
 */
{

}

/**
 * Decompress a BC 16x3 index block stored as
 *   h g f e
 *   d c b a
 *   p o n m
 *   l k j i
 *
 * Bits packed as
 *  | h | g | f | e | d | c | b | a | // Entry
 *  |765 432 107 654 321 076 543 210| // Bit
 *  |0000000000111111111112222222222| // Byte
 *
 * into 16 8-bit indices.
 */
{


        /* Unpack 8x3 bit from last 3 byte block */

    }

                                       const uint8_t *block)
{




                alpha = alpha0;
                alpha = alpha1;
            } else {
                } else {
                        alpha = 0;
                        alpha = 255;
                    } else {
                    }
                }
            }
        }
    }

/**
 * Decompress one block of a DXT4 texture and store the resulting
 * RGBA pixels in 'dst'.
 *
 * @param dst    output buffer.
 * @param stride scanline in bytes.
 * @param block  block to decompress.
 * @return how much texture data has been consumed.
 */
{


    /* This format is DXT5, but returns premultiplied alpha. It needs to be
     * converted because it's what lavc outputs (and swscale expects). */

}

/**
 * Decompress one block of a DXT5 texture and store the resulting
 * RGBA pixels in 'dst'.
 *
 * @param dst    output buffer.
 * @param stride scanline in bytes.
 * @param block  block to decompress.
 * @return how much texture data has been consumed.
 */
{

}

/**
 * Convert a YCoCg buffer to RGBA.
 *
 * @param src    input buffer.
 * @param scaled variant with scaled chroma components and opaque alpha.
 */
{


}

/**
 * Decompress one block of a DXT5 texture with classic YCoCg and store
 * the resulting RGBA pixels in 'dst'. Alpha component is fully opaque.
 *
 * @param dst    output buffer.
 * @param stride scanline in bytes.
 * @param block  block to decompress.
 * @return how much texture data has been consumed.
 */
{

    /* This format is basically DXT5, with luma stored in alpha.
     * Run a normal decompress and then reorder the components. */


}

/**
 * Decompress one block of a DXT5 texture with scaled YCoCg and store
 * the resulting RGBA pixels in 'dst'. Alpha component is fully opaque.
 *
 * @param dst    output buffer.
 * @param stride scanline in bytes.
 * @param block  block to decompress.
 * @return how much texture data has been consumed.
 */
{

    /* This format is basically DXT5, with luma stored in alpha.
     * Run a normal decompress and then reorder the components. */


}

                                       const uint8_t *block,
                                       const int *color_tab, int mono, int offset, int pix_size)
{


    /* Only one or two channels are stored at most, since it only used to
     * compress specular (black and white) or normal (red and green) maps.
     * Although the standard says to zero out unused components, many
     * implementations fill all of them with the same value. */
            /* Interval expansion from [-1 1] or [0 1] to [0 255]. */

            }
            else{
            }
        }
    }

                                        const uint8_t *block, int sign, int mono, int offset, int pix_size)
{

        /* signed data is in [-128 127] so just offset it to unsigned
         * and it can be treated exactly the same */
    } else {
    }


        /* 6 interpolated color values */
    } else {
        /* 4 interpolated color values */
    }


/**
 * Decompress one block of a RGRC1 texture with signed components
 * and store the resulting RGBA pixels in 'dst'.
 *
 * @param dst    output buffer.
 * @param stride scanline in bytes.
 * @param block  block to decompress.
 * @return how much texture data has been consumed.
 */
{

}

/**
 * Decompress one block of a RGRC1 texture with unsigned components
 * and store the resulting RGBA pixels in 'dst'.
 *
 * @param dst    output buffer.
 * @param stride scanline in bytes.
 * @param block  block to decompress.
 * @return how much texture data has been consumed.
 */
{

}

/**
 * Decompress one block of a RGTC1 texture with unsigned components
 * and overwrite the alpha component in 'dst' (RGBA data).
 *
 * @param dst    output buffer.
 * @param stride scanline in bytes.
 * @param block  block to decompress.
 * @return how much texture data has been consumed.
 */
{

}

/**
 * Decompress one block of a RGTC1 texture with unsigned components
 * to Gray 8.
 *
 * @param dst    output buffer.
 * @param stride scanline in bytes.
 * @param block  block to decompress.
 * @return how much texture data has been consumed.
 */
{

}

                                        const uint8_t *block, int sign)
{
    /* 4x4 block containing 4 component pixels. */

    /* Decompress the two channels separately and interleave them afterwards. */

    /* B is rebuilt exactly like a normal map. */


        }
    }

/**
 * Decompress one block of a RGRC2 texture with signed components
 * and store the resulting RGBA pixels in 'dst'. Alpha is fully opaque.
 *
 * @param dst    output buffer.
 * @param stride scanline in bytes.
 * @param block  block to decompress.
 * @return how much texture data has been consumed.
 */
{

}

/**
 * Decompress one block of a RGRC2 texture with unsigned components
 * and store the resulting RGBA pixels in 'dst'. Alpha is fully opaque.
 *
 * @param dst    output buffer.
 * @param stride scanline in bytes.
 * @param block  block to decompress.
 * @return how much texture data has been consumed.
 */
{

}

/**
 * Decompress one block of a 3Dc texture with unsigned components
 * and store the resulting RGBA pixels in 'dst'. Alpha is fully opaque.
 *
 * @param dst    output buffer.
 * @param stride scanline in bytes.
 * @param block  block to decompress.
 * @return how much texture data has been consumed.
 */
{

    /* This is the 3Dc variant of RGTC2, with swapped R and G. */
        }
    }

}

{
