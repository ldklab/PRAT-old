/*
 * LZW decoder
 * Copyright (c) 2003 Fabrice Bellard
 * Copyright (c) 2006 Konstantin Shishkov
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
 * @brief LZW decoding routines
 * @author Fabrice Bellard
 * @author modified for use in TIFF by Konstantin Shishkov
 */

#include "avcodec.h"
#include "bytestream.h"
#include "lzw.h"
#include "libavutil/mem.h"

#define LZW_MAXBITS                 12
#define LZW_SIZTABLE                (1<<LZW_MAXBITS)

static const uint16_t mask[17] =
{
    0x0000, 0x0001, 0x0003, 0x0007,
    0x000F, 0x001F, 0x003F, 0x007F,
    0x00FF, 0x01FF, 0x03FF, 0x07FF,
    0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF
};

struct LZWState {
    GetByteContext gb;
    int bbits;
    unsigned int bbuf;

    int mode;                   ///< Decoder mode
    int cursize;                ///< The current code size
    int curmask;
    int codesize;
    int clear_code;
    int end_code;
    int newcodes;               ///< First available code
    int top_slot;               ///< Highest code for current size
    int extra_slot;
    int slot;                   ///< Last read code
    int fc, oc;
    uint8_t *sp;
    uint8_t stack[LZW_SIZTABLE];
    uint8_t suffix[LZW_SIZTABLE];
    uint16_t prefix[LZW_SIZTABLE];
    int bs;                     ///< current buffer size for GIF
};

/* get one code from stream */
{

        return s->end_code;

            }
        }
    } else { // TIFF
        while (s->bbits < s->cursize) {
            s->bbuf = (s->bbuf << 8) | bytestream2_get_byte(&s->gb);
            s->bbits += 8;
        }
        c = s->bbuf >> (s->bbits - s->cursize);
    }
}

{

        }
    }else
        bytestream2_skip(&s->gb, bytestream2_get_bytes_left(&s->gb));
}

{

{

/**
 * Initialize LZW decoder
 * @param p LZW context
 * @param csize initial code size in bits
 * @param buf input data
 * @param buf_size input data size
 * @param mode decoder working mode - either GIF or TIFF
 */
{

        return -1;
    /* read buffer */

    /* decoder */

}

/**
 * Decode given number of bytes
 * NOTE: the algorithm here is inspired from the LZW GIF decoder
 *  written by Steven A. Bennett in 1987.
 *
 * @param p LZW context
 * @param buf output buffer
 * @param len number of bytes to decode
 * @return number of bytes decoded
 */

        return 0;


    for (;;) {
        }
            break;
        } else {
                break;
            }
            }
                }
            }
        }
    }
    s->end_code = -1;
}
