/*
 * LucasArts Smush video decoder
 * Copyright (c) 2006 Cyril Zorin
 * Copyright (c) 2011 Konstantin Shishkov
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

#include "libavutil/avassert.h"
#include "libavutil/bswap.h"
#include "libavutil/imgutils.h"

#include "avcodec.h"
#include "bytestream.h"
#include "copy_block.h"
#include "internal.h"

#define NGLYPHS 256
#define GLYPH_COORD_VECT_SIZE 16
#define PALETTE_SIZE 256
#define PALETTE_DELTA 768

static const int8_t glyph4_x[GLYPH_COORD_VECT_SIZE] = {
    0, 1, 2, 3, 3, 3, 3, 2, 1, 0, 0, 0, 1, 2, 2, 1
};

static const int8_t glyph4_y[GLYPH_COORD_VECT_SIZE] = {
    0, 0, 0, 0, 1, 2, 3, 3, 3, 3, 2, 1, 1, 1, 2, 2
};

static const int8_t glyph8_x[GLYPH_COORD_VECT_SIZE] = {
    0, 2, 5, 7, 7, 7, 7, 7, 7, 5, 2, 0, 0, 0, 0, 0
};

static const int8_t glyph8_y[GLYPH_COORD_VECT_SIZE] = {
    0, 0, 0, 0, 1, 3, 4, 6, 7, 7, 7, 7, 6, 4, 3, 1
};

static const int8_t motion_vectors[256][2] = {
    {   0,   0 }, {  -1, -43 }, {   6, -43 }, {  -9, -42 }, {  13, -41 },
    { -16, -40 }, {  19, -39 }, { -23, -36 }, {  26, -34 }, {  -2, -33 },
    {   4, -33 }, { -29, -32 }, {  -9, -32 }, {  11, -31 }, { -16, -29 },
    {  32, -29 }, {  18, -28 }, { -34, -26 }, { -22, -25 }, {  -1, -25 },
    {   3, -25 }, {  -7, -24 }, {   8, -24 }, {  24, -23 }, {  36, -23 },
    { -12, -22 }, {  13, -21 }, { -38, -20 }, {   0, -20 }, { -27, -19 },
    {  -4, -19 }, {   4, -19 }, { -17, -18 }, {  -8, -17 }, {   8, -17 },
    {  18, -17 }, {  28, -17 }, {  39, -17 }, { -12, -15 }, {  12, -15 },
    { -21, -14 }, {  -1, -14 }, {   1, -14 }, { -41, -13 }, {  -5, -13 },
    {   5, -13 }, {  21, -13 }, { -31, -12 }, { -15, -11 }, {  -8, -11 },
    {   8, -11 }, {  15, -11 }, {  -2, -10 }, {   1, -10 }, {  31, -10 },
    { -23,  -9 }, { -11,  -9 }, {  -5,  -9 }, {   4,  -9 }, {  11,  -9 },
    {  42,  -9 }, {   6,  -8 }, {  24,  -8 }, { -18,  -7 }, {  -7,  -7 },
    {  -3,  -7 }, {  -1,  -7 }, {   2,  -7 }, {  18,  -7 }, { -43,  -6 },
    { -13,  -6 }, {  -4,  -6 }, {   4,  -6 }, {   8,  -6 }, { -33,  -5 },
    {  -9,  -5 }, {  -2,  -5 }, {   0,  -5 }, {   2,  -5 }, {   5,  -5 },
    {  13,  -5 }, { -25,  -4 }, {  -6,  -4 }, {  -3,  -4 }, {   3,  -4 },
    {   9,  -4 }, { -19,  -3 }, {  -7,  -3 }, {  -4,  -3 }, {  -2,  -3 },
    {  -1,  -3 }, {   0,  -3 }, {   1,  -3 }, {   2,  -3 }, {   4,  -3 },
    {   6,  -3 }, {  33,  -3 }, { -14,  -2 }, { -10,  -2 }, {  -5,  -2 },
    {  -3,  -2 }, {  -2,  -2 }, {  -1,  -2 }, {   0,  -2 }, {   1,  -2 },
    {   2,  -2 }, {   3,  -2 }, {   5,  -2 }, {   7,  -2 }, {  14,  -2 },
    {  19,  -2 }, {  25,  -2 }, {  43,  -2 }, {  -7,  -1 }, {  -3,  -1 },
    {  -2,  -1 }, {  -1,  -1 }, {   0,  -1 }, {   1,  -1 }, {   2,  -1 },
    {   3,  -1 }, {  10,  -1 }, {  -5,   0 }, {  -3,   0 }, {  -2,   0 },
    {  -1,   0 }, {   1,   0 }, {   2,   0 }, {   3,   0 }, {   5,   0 },
    {   7,   0 }, { -10,   1 }, {  -7,   1 }, {  -3,   1 }, {  -2,   1 },
    {  -1,   1 }, {   0,   1 }, {   1,   1 }, {   2,   1 }, {   3,   1 },
    { -43,   2 }, { -25,   2 }, { -19,   2 }, { -14,   2 }, {  -5,   2 },
    {  -3,   2 }, {  -2,   2 }, {  -1,   2 }, {   0,   2 }, {   1,   2 },
    {   2,   2 }, {   3,   2 }, {   5,   2 }, {   7,   2 }, {  10,   2 },
    {  14,   2 }, { -33,   3 }, {  -6,   3 }, {  -4,   3 }, {  -2,   3 },
    {  -1,   3 }, {   0,   3 }, {   1,   3 }, {   2,   3 }, {   4,   3 },
    {  19,   3 }, {  -9,   4 }, {  -3,   4 }, {   3,   4 }, {   7,   4 },
    {  25,   4 }, { -13,   5 }, {  -5,   5 }, {  -2,   5 }, {   0,   5 },
    {   2,   5 }, {   5,   5 }, {   9,   5 }, {  33,   5 }, {  -8,   6 },
    {  -4,   6 }, {   4,   6 }, {  13,   6 }, {  43,   6 }, { -18,   7 },
    {  -2,   7 }, {   0,   7 }, {   2,   7 }, {   7,   7 }, {  18,   7 },
    { -24,   8 }, {  -6,   8 }, { -42,   9 }, { -11,   9 }, {  -4,   9 },
    {   5,   9 }, {  11,   9 }, {  23,   9 }, { -31,  10 }, {  -1,  10 },
    {   2,  10 }, { -15,  11 }, {  -8,  11 }, {   8,  11 }, {  15,  11 },
    {  31,  12 }, { -21,  13 }, {  -5,  13 }, {   5,  13 }, {  41,  13 },
    {  -1,  14 }, {   1,  14 }, {  21,  14 }, { -12,  15 }, {  12,  15 },
    { -39,  17 }, { -28,  17 }, { -18,  17 }, {  -8,  17 }, {   8,  17 },
    {  17,  18 }, {  -4,  19 }, {   0,  19 }, {   4,  19 }, {  27,  19 },
    {  38,  20 }, { -13,  21 }, {  12,  22 }, { -36,  23 }, { -24,  23 },
    {  -8,  24 }, {   7,  24 }, {  -3,  25 }, {   1,  25 }, {  22,  25 },
    {  34,  26 }, { -18,  28 }, { -32,  29 }, {  16,  29 }, { -11,  31 },
    {   9,  32 }, {  29,  32 }, {  -4,  33 }, {   2,  33 }, { -26,  34 },
    {  23,  36 }, { -19,  39 }, {  16,  40 }, { -13,  41 }, {   9,  42 },
    {  -6,  43 }, {   1,  43 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
};

static const int8_t c37_mv[] = {
    0,   0,   1,   0,   2,   0,   3,   0,   5,   0,
    8,   0,  13,   0,  21,   0,  -1,   0,  -2,   0,
   -3,   0,  -5,   0,  -8,   0, -13,   0, -17,   0,
  -21,   0,   0,   1,   1,   1,   2,   1,   3,   1,
    5,   1,   8,   1,  13,   1,  21,   1,  -1,   1,
   -2,   1,  -3,   1,  -5,   1,  -8,   1, -13,   1,
  -17,   1, -21,   1,   0,   2,   1,   2,   2,   2,
    3,   2,   5,   2,   8,   2,  13,   2,  21,   2,
   -1,   2,  -2,   2,  -3,   2,  -5,   2,  -8,   2,
  -13,   2, -17,   2, -21,   2,   0,   3,   1,   3,
    2,   3,   3,   3,   5,   3,   8,   3,  13,   3,
   21,   3,  -1,   3,  -2,   3,  -3,   3,  -5,   3,
   -8,   3, -13,   3, -17,   3, -21,   3,   0,   5,
    1,   5,   2,   5,   3,   5,   5,   5,   8,   5,
   13,   5,  21,   5,  -1,   5,  -2,   5,  -3,   5,
   -5,   5,  -8,   5, -13,   5, -17,   5, -21,   5,
    0,   8,   1,   8,   2,   8,   3,   8,   5,   8,
    8,   8,  13,   8,  21,   8,  -1,   8,  -2,   8,
   -3,   8,  -5,   8,  -8,   8, -13,   8, -17,   8,
  -21,   8,   0,  13,   1,  13,   2,  13,   3,  13,
    5,  13,   8,  13,  13,  13,  21,  13,  -1,  13,
   -2,  13,  -3,  13,  -5,  13,  -8,  13, -13,  13,
  -17,  13, -21,  13,   0,  21,   1,  21,   2,  21,
    3,  21,   5,  21,   8,  21,  13,  21,  21,  21,
   -1,  21,  -2,  21,  -3,  21,  -5,  21,  -8,  21,
  -13,  21, -17,  21, -21,  21,   0,  -1,   1,  -1,
    2,  -1,   3,  -1,   5,  -1,   8,  -1,  13,  -1,
   21,  -1,  -1,  -1,  -2,  -1,  -3,  -1,  -5,  -1,
   -8,  -1, -13,  -1, -17,  -1, -21,  -1,   0,  -2,
    1,  -2,   2,  -2,   3,  -2,   5,  -2,   8,  -2,
   13,  -2,  21,  -2,  -1,  -2,  -2,  -2,  -3,  -2,
   -5,  -2,  -8,  -2, -13,  -2, -17,  -2, -21,  -2,
    0,  -3,   1,  -3,   2,  -3,   3,  -3,   5,  -3,
    8,  -3,  13,  -3,  21,  -3,  -1,  -3,  -2,  -3,
   -3,  -3,  -5,  -3,  -8,  -3, -13,  -3, -17,  -3,
  -21,  -3,   0,  -5,   1,  -5,   2,  -5,   3,  -5,
    5,  -5,   8,  -5,  13,  -5,  21,  -5,  -1,  -5,
   -2,  -5,  -3,  -5,  -5,  -5,  -8,  -5, -13,  -5,
  -17,  -5, -21,  -5,   0,  -8,   1,  -8,   2,  -8,
    3,  -8,   5,  -8,   8,  -8,  13,  -8,  21,  -8,
   -1,  -8,  -2,  -8,  -3,  -8,  -5,  -8,  -8,  -8,
  -13,  -8, -17,  -8, -21,  -8,   0, -13,   1, -13,
    2, -13,   3, -13,   5, -13,   8, -13,  13, -13,
   21, -13,  -1, -13,  -2, -13,  -3, -13,  -5, -13,
   -8, -13, -13, -13, -17, -13, -21, -13,   0, -17,
    1, -17,   2, -17,   3, -17,   5, -17,   8, -17,
   13, -17,  21, -17,  -1, -17,  -2, -17,  -3, -17,
   -5, -17,  -8, -17, -13, -17, -17, -17, -21, -17,
    0, -21,   1, -21,   2, -21,   3, -21,   5, -21,
    8, -21,  13, -21,  21, -21,  -1, -21,  -2, -21,
   -3, -21,  -5, -21,  -8, -21, -13, -21, -17, -21,
    0,   0,  -8, -29,   8, -29, -18, -25,  17, -25,
    0, -23,  -6, -22,   6, -22, -13, -19,  12, -19,
    0, -18,  25, -18, -25, -17,  -5, -17,   5, -17,
  -10, -15,  10, -15,   0, -14,  -4, -13,   4, -13,
   19, -13, -19, -12,  -8, -11,  -2, -11,   0, -11,
    2, -11,   8, -11, -15, -10,  -4, -10,   4, -10,
   15, -10,  -6,  -9,  -1,  -9,   1,  -9,   6,  -9,
  -29,  -8, -11,  -8,  -8,  -8,  -3,  -8,   3,  -8,
    8,  -8,  11,  -8,  29,  -8,  -5,  -7,  -2,  -7,
    0,  -7,   2,  -7,   5,  -7, -22,  -6,  -9,  -6,
   -6,  -6,  -3,  -6,  -1,  -6,   1,  -6,   3,  -6,
    6,  -6,   9,  -6,  22,  -6, -17,  -5,  -7,  -5,
   -4,  -5,  -2,  -5,   0,  -5,   2,  -5,   4,  -5,
    7,  -5,  17,  -5, -13,  -4, -10,  -4,  -5,  -4,
   -3,  -4,  -1,  -4,   0,  -4,   1,  -4,   3,  -4,
    5,  -4,  10,  -4,  13,  -4,  -8,  -3,  -6,  -3,
   -4,  -3,  -3,  -3,  -2,  -3,  -1,  -3,   0,  -3,
    1,  -3,   2,  -3,   4,  -3,   6,  -3,   8,  -3,
  -11,  -2,  -7,  -2,  -5,  -2,  -3,  -2,  -2,  -2,
   -1,  -2,   0,  -2,   1,  -2,   2,  -2,   3,  -2,
    5,  -2,   7,  -2,  11,  -2,  -9,  -1,  -6,  -1,
   -4,  -1,  -3,  -1,  -2,  -1,  -1,  -1,   0,  -1,
    1,  -1,   2,  -1,   3,  -1,   4,  -1,   6,  -1,
    9,  -1, -31,   0, -23,   0, -18,   0, -14,   0,
  -11,   0,  -7,   0,  -5,   0,  -4,   0,  -3,   0,
   -2,   0,  -1,   0,   0, -31,   1,   0,   2,   0,
    3,   0,   4,   0,   5,   0,   7,   0,  11,   0,
   14,   0,  18,   0,  23,   0,  31,   0,  -9,   1,
   -6,   1,  -4,   1,  -3,   1,  -2,   1,  -1,   1,
    0,   1,   1,   1,   2,   1,   3,   1,   4,   1,
    6,   1,   9,   1, -11,   2,  -7,   2,  -5,   2,
   -3,   2,  -2,   2,  -1,   2,   0,   2,   1,   2,
    2,   2,   3,   2,   5,   2,   7,   2,  11,   2,
   -8,   3,  -6,   3,  -4,   3,  -2,   3,  -1,   3,
    0,   3,   1,   3,   2,   3,   3,   3,   4,   3,
    6,   3,   8,   3, -13,   4, -10,   4,  -5,   4,
   -3,   4,  -1,   4,   0,   4,   1,   4,   3,   4,
    5,   4,  10,   4,  13,   4, -17,   5,  -7,   5,
   -4,   5,  -2,   5,   0,   5,   2,   5,   4,   5,
    7,   5,  17,   5, -22,   6,  -9,   6,  -6,   6,
   -3,   6,  -1,   6,   1,   6,   3,   6,   6,   6,
    9,   6,  22,   6,  -5,   7,  -2,   7,   0,   7,
    2,   7,   5,   7, -29,   8, -11,   8,  -8,   8,
   -3,   8,   3,   8,   8,   8,  11,   8,  29,   8,
   -6,   9,  -1,   9,   1,   9,   6,   9, -15,  10,
   -4,  10,   4,  10,  15,  10,  -8,  11,  -2,  11,
    0,  11,   2,  11,   8,  11,  19,  12, -19,  13,
   -4,  13,   4,  13,   0,  14, -10,  15,  10,  15,
   -5,  17,   5,  17,  25,  17, -25,  18,   0,  18,
  -12,  19,  13,  19,  -6,  22,   6,  22,   0,  23,
  -17,  25,  18,  25,  -8,  29,   8,  29,   0,  31,
    0,   0,  -6, -22,   6, -22, -13, -19,  12, -19,
    0, -18,  -5, -17,   5, -17, -10, -15,  10, -15,
    0, -14,  -4, -13,   4, -13,  19, -13, -19, -12,
   -8, -11,  -2, -11,   0, -11,   2, -11,   8, -11,
  -15, -10,  -4, -10,   4, -10,  15, -10,  -6,  -9,
   -1,  -9,   1,  -9,   6,  -9, -11,  -8,  -8,  -8,
   -3,  -8,   0,  -8,   3,  -8,   8,  -8,  11,  -8,
   -5,  -7,  -2,  -7,   0,  -7,   2,  -7,   5,  -7,
  -22,  -6,  -9,  -6,  -6,  -6,  -3,  -6,  -1,  -6,
    1,  -6,   3,  -6,   6,  -6,   9,  -6,  22,  -6,
  -17,  -5,  -7,  -5,  -4,  -5,  -2,  -5,  -1,  -5,
    0,  -5,   1,  -5,   2,  -5,   4,  -5,   7,  -5,
   17,  -5, -13,  -4, -10,  -4,  -5,  -4,  -3,  -4,
   -2,  -4,  -1,  -4,   0,  -4,   1,  -4,   2,  -4,
    3,  -4,   5,  -4,  10,  -4,  13,  -4,  -8,  -3,
   -6,  -3,  -4,  -3,  -3,  -3,  -2,  -3,  -1,  -3,
    0,  -3,   1,  -3,   2,  -3,   3,  -3,   4,  -3,
    6,  -3,   8,  -3, -11,  -2,  -7,  -2,  -5,  -2,
   -4,  -2,  -3,  -2,  -2,  -2,  -1,  -2,   0,  -2,
    1,  -2,   2,  -2,   3,  -2,   4,  -2,   5,  -2,
    7,  -2,  11,  -2,  -9,  -1,  -6,  -1,  -5,  -1,
   -4,  -1,  -3,  -1,  -2,  -1,  -1,  -1,   0,  -1,
    1,  -1,   2,  -1,   3,  -1,   4,  -1,   5,  -1,
    6,  -1,   9,  -1, -23,   0, -18,   0, -14,   0,
  -11,   0,  -7,   0,  -5,   0,  -4,   0,  -3,   0,
   -2,   0,  -1,   0,   0, -23,   1,   0,   2,   0,
    3,   0,   4,   0,   5,   0,   7,   0,  11,   0,
   14,   0,  18,   0,  23,   0,  -9,   1,  -6,   1,
   -5,   1,  -4,   1,  -3,   1,  -2,   1,  -1,   1,
    0,   1,   1,   1,   2,   1,   3,   1,   4,   1,
    5,   1,   6,   1,   9,   1, -11,   2,  -7,   2,
   -5,   2,  -4,   2,  -3,   2,  -2,   2,  -1,   2,
    0,   2,   1,   2,   2,   2,   3,   2,   4,   2,
    5,   2,   7,   2,  11,   2,  -8,   3,  -6,   3,
   -4,   3,  -3,   3,  -2,   3,  -1,   3,   0,   3,
    1,   3,   2,   3,   3,   3,   4,   3,   6,   3,
    8,   3, -13,   4, -10,   4,  -5,   4,  -3,   4,
   -2,   4,  -1,   4,   0,   4,   1,   4,   2,   4,
    3,   4,   5,   4,  10,   4,  13,   4, -17,   5,
   -7,   5,  -4,   5,  -2,   5,  -1,   5,   0,   5,
    1,   5,   2,   5,   4,   5,   7,   5,  17,   5,
  -22,   6,  -9,   6,  -6,   6,  -3,   6,  -1,   6,
    1,   6,   3,   6,   6,   6,   9,   6,  22,   6,
   -5,   7,  -2,   7,   0,   7,   2,   7,   5,   7,
  -11,   8,  -8,   8,  -3,   8,   0,   8,   3,   8,
    8,   8,  11,   8,  -6,   9,  -1,   9,   1,   9,
    6,   9, -15,  10,  -4,  10,   4,  10,  15,  10,
   -8,  11,  -2,  11,   0,  11,   2,  11,   8,  11,
   19,  12, -19,  13,  -4,  13,   4,  13,   0,  14,
  -10,  15,  10,  15,  -5,  17,   5,  17,   0,  18,
  -12,  19,  13,  19,  -6,  22,   6,  22,   0,  23,
};

typedef struct SANMVideoContext {
    AVCodecContext *avctx;
    GetByteContext gb;

    int version, subversion;
    uint32_t pal[PALETTE_SIZE];
    int16_t delta_pal[PALETTE_DELTA];

    ptrdiff_t pitch;
    int width, height;
    int aligned_width, aligned_height;
    int prev_seq;

    AVFrame *frame;
    uint16_t *frm0, *frm1, *frm2;
    uint8_t *stored_frame;
    uint32_t frm0_size, frm1_size, frm2_size;
    uint32_t stored_frame_size;

    uint8_t *rle_buf;
    unsigned int rle_buf_size;

    int rotate_code;

    long npixels, buf_size;

    uint16_t codebook[256];
    uint16_t small_codebook[4];

    int8_t p4x4glyphs[NGLYPHS][16];
    int8_t p8x8glyphs[NGLYPHS][64];
} SANMVideoContext;

typedef struct SANMFrameHeader {
    int seq_num, codec, rotate_code, rle_output_size;

    uint16_t bg_color;
    uint32_t width, height;
} SANMFrameHeader;

enum GlyphEdge {
    LEFT_EDGE,
    TOP_EDGE,
    RIGHT_EDGE,
    BOTTOM_EDGE,
    NO_EDGE
};

enum GlyphDir {
    DIR_LEFT,
    DIR_UP,
    DIR_RIGHT,
    DIR_DOWN,
    NO_DIR
};

/**
 * Return enum GlyphEdge of box where point (x, y) lies.
 *
 * @param x x point coordinate
 * @param y y point coordinate
 * @param edge_size box width/height.
 */
{

        return BOTTOM_EDGE;
        return TOP_EDGE;
        return LEFT_EDGE;
        return RIGHT_EDGE;
    else
}

{
        return DIR_UP;
        return DIR_DOWN;
        return DIR_LEFT;

    return NO_DIR;
}

/* Interpolate two points. */
                         int pos, int npoints)
{
    } else {
        points[0] = x0;
        points[1] = y0;
    }
}

/**
 * Construct glyphs by iterating through vector coordinates.
 *
 * @param pglyphs pointer to table where glyphs are stored
 * @param xvec pointer to x component of vector coordinates
 * @param yvec pointer to y component of vector coordinates
 * @param side_length glyph width/height.
 */
                        const int side_length)
{





                    break;

                    break;

                    break;

                    break;
                }
        }
    }

{


}

{

{
        av_fast_padded_mallocz(&ctx->stored_frame,
                              &ctx->stored_frame_size, ctx->buf_size);

        destroy_buffers(ctx);
        return AVERROR(ENOMEM);
    }

    return 0;
}

{

{

    // early sanity check before allocations to avoid need for deallocation code.
        av_log(avctx, AV_LOG_ERROR, "Not enough extradata.\n");
        return AVERROR_INVALIDDATA;
    }


        av_log(avctx, AV_LOG_ERROR, "Error allocating buffers.\n");
        return AVERROR(ENOMEM);
    }


        int i;

        ctx->subversion = AV_RL16(avctx->extradata);
        for (i = 0; i < PALETTE_SIZE; i++)
            ctx->pal[i] = 0xFFU << 24 | AV_RL32(avctx->extradata + 2 + i * 4);
    }

    return 0;
}

{


}

{

            return AVERROR_INVALIDDATA;

        } else {
            if (bytestream2_get_bytes_left(&ctx->gb) < run_len)
                return AVERROR_INVALIDDATA;
            bytestream2_get_bufferu(&ctx->gb, dst, run_len);
        }

    }

    return 0;
}

static int old_codec1(SANMVideoContext *ctx, int top,
                      int left, int width, int height)
{
    uint8_t *dst = ((uint8_t *)ctx->frm0) + left + top * ctx->pitch;
    int i, j, len, flag, code, val, pos, end;

    for (i = 0; i < height; i++) {
        pos = 0;

        if (bytestream2_get_bytes_left(&ctx->gb) < 2)
            return AVERROR_INVALIDDATA;

        len = bytestream2_get_le16u(&ctx->gb);
        end = bytestream2_tell(&ctx->gb) + len;

        while (bytestream2_tell(&ctx->gb) < end) {
            if (bytestream2_get_bytes_left(&ctx->gb) < 2)
                return AVERROR_INVALIDDATA;

            code = bytestream2_get_byteu(&ctx->gb);
            flag = code & 1;
            code = (code >> 1) + 1;
            if (pos + code > width)
                return AVERROR_INVALIDDATA;
            if (flag) {
                val = bytestream2_get_byteu(&ctx->gb);
                if (val)
                    memset(dst + pos, val, code);
                pos += code;
            } else {
                if (bytestream2_get_bytes_left(&ctx->gb) < code)
                    return AVERROR_INVALIDDATA;
                for (j = 0; j < code; j++) {
                    val = bytestream2_get_byteu(&ctx->gb);
                    if (val)
                        dst[pos] = val;
                    pos++;
                }
            }
        }
        dst += ctx->pitch;
    }
    ctx->rotate_code = 0;

    return 0;
}

static inline void codec37_mv(uint8_t *dst, const uint8_t *src,
                              int height, int stride, int x, int y)
{
    int pos, i, j;

    pos = x + y * stride;
    for (j = 0; j < 4; j++) {
        for (i = 0; i < 4; i++) {
            if ((pos + i) < 0 || (pos + i) >= height * stride)
                dst[i] = 0;
            else
                dst[i] = src[i];
        }
        dst += stride;
        src += stride;
        pos += stride;
    }
}

static int old_codec37(SANMVideoContext *ctx, int top,
                       int left, int width, int height)
{
    ptrdiff_t stride = ctx->pitch;
    int i, j, k, t;
    uint8_t *dst, *prev;
    int skip_run = 0;
    int compr = bytestream2_get_byte(&ctx->gb);
    int mvoff = bytestream2_get_byte(&ctx->gb);
    int seq   = bytestream2_get_le16(&ctx->gb);
    uint32_t decoded_size = bytestream2_get_le32(&ctx->gb);
    int flags;

    bytestream2_skip(&ctx->gb, 4);
    flags = bytestream2_get_byte(&ctx->gb);
    bytestream2_skip(&ctx->gb, 3);

    if (decoded_size > ctx->height * stride - left - top * stride) {
        decoded_size = ctx->height * stride - left - top * stride;
        av_log(ctx->avctx, AV_LOG_WARNING, "Decoded size is too large.\n");
    }

    ctx->rotate_code = 0;

    if (((seq & 1) || !(flags & 1)) && (compr && compr != 2))
        rotate_bufs(ctx, 1);

    dst  = ((uint8_t*)ctx->frm0) + left + top * stride;
    prev = ((uint8_t*)ctx->frm2) + left + top * stride;

    if (mvoff > 2) {
        av_log(ctx->avctx, AV_LOG_ERROR, "Invalid motion base value %d.\n", mvoff);
        return AVERROR_INVALIDDATA;
    }

    switch (compr) {
    case 0:
        for (i = 0; i < height; i++) {
            bytestream2_get_buffer(&ctx->gb, dst, width);
            dst += stride;
        }
        memset(ctx->frm1, 0, ctx->height * stride);
        memset(ctx->frm2, 0, ctx->height * stride);
        break;
    case 2:
        if (rle_decode(ctx, dst, decoded_size))
            return AVERROR_INVALIDDATA;
        memset(ctx->frm1, 0, ctx->frm1_size);
        memset(ctx->frm2, 0, ctx->frm2_size);
        break;
    case 3:
    case 4:
        if (flags & 4) {
            for (j = 0; j < height; j += 4) {
                for (i = 0; i < width; i += 4) {
                    int code;
                    if (skip_run) {
                        skip_run--;
                        copy_block4(dst + i, prev + i, stride, stride, 4);
                        continue;
                    }
                    if (bytestream2_get_bytes_left(&ctx->gb) < 1)
                        return AVERROR_INVALIDDATA;
                    code = bytestream2_get_byteu(&ctx->gb);
                    switch (code) {
                    case 0xFF:
                        if (bytestream2_get_bytes_left(&ctx->gb) < 16)
                            return AVERROR_INVALIDDATA;
                        for (k = 0; k < 4; k++)
                            bytestream2_get_bufferu(&ctx->gb, dst + i + k * stride, 4);
                        break;
                    case 0xFE:
                        if (bytestream2_get_bytes_left(&ctx->gb) < 4)
                            return AVERROR_INVALIDDATA;
                        for (k = 0; k < 4; k++)
                            memset(dst + i + k * stride, bytestream2_get_byteu(&ctx->gb), 4);
                        break;
                    case 0xFD:
                        if (bytestream2_get_bytes_left(&ctx->gb) < 1)
                            return AVERROR_INVALIDDATA;
                        t = bytestream2_get_byteu(&ctx->gb);
                        for (k = 0; k < 4; k++)
                            memset(dst + i + k * stride, t, 4);
                        break;
                    default:
                        if (compr == 4 && !code) {
                            if (bytestream2_get_bytes_left(&ctx->gb) < 1)
                                return AVERROR_INVALIDDATA;
                            skip_run = bytestream2_get_byteu(&ctx->gb) + 1;
                            i -= 4;
                        } else {
                            int mx, my;

                            mx = c37_mv[(mvoff * 255 + code) * 2];
                            my = c37_mv[(mvoff * 255 + code) * 2 + 1];
                            codec37_mv(dst + i, prev + i + mx + my * stride,
                                       ctx->height, stride, i + mx, j + my);
                        }
                    }
                }
                dst  += stride * 4;
                prev += stride * 4;
            }
        } else {
            for (j = 0; j < height; j += 4) {
                for (i = 0; i < width; i += 4) {
                    int code;
                    if (skip_run) {
                        skip_run--;
                        copy_block4(dst + i, prev + i, stride, stride, 4);
                        continue;
                    }
                    code = bytestream2_get_byte(&ctx->gb);
                    if (code == 0xFF) {
                        if (bytestream2_get_bytes_left(&ctx->gb) < 16)
                            return AVERROR_INVALIDDATA;
                        for (k = 0; k < 4; k++)
                            bytestream2_get_bufferu(&ctx->gb, dst + i + k * stride, 4);
                    } else if (compr == 4 && !code) {
                        if (bytestream2_get_bytes_left(&ctx->gb) < 1)
                            return AVERROR_INVALIDDATA;
                        skip_run = bytestream2_get_byteu(&ctx->gb) + 1;
                        i -= 4;
                    } else {
                        int mx, my;

                        mx = c37_mv[(mvoff * 255 + code) * 2];
                        my = c37_mv[(mvoff * 255 + code) * 2 + 1];
                        codec37_mv(dst + i, prev + i + mx + my * stride,
                                   ctx->height, stride, i + mx, j + my);
                    }
                }
                dst  += stride * 4;
                prev += stride * 4;
            }
        }
        break;
    default:
        avpriv_report_missing_feature(ctx->avctx,
                                      "Subcodec 37 compression %d", compr);
        return AVERROR_PATCHWELCOME;
    }

    return 0;
}

static int process_block(SANMVideoContext *ctx, uint8_t *dst, uint8_t *prev1,
                         uint8_t *prev2, int stride, int tbl, int size)
{
    int code, k, t;
    uint8_t colors[2];
    int8_t *pglyph;

    if (bytestream2_get_bytes_left(&ctx->gb) < 1)
        return AVERROR_INVALIDDATA;

    code = bytestream2_get_byteu(&ctx->gb);
    if (code >= 0xF8) {
        switch (code) {
        case 0xFF:
            if (size == 2) {
                if (bytestream2_get_bytes_left(&ctx->gb) < 4)
                    return AVERROR_INVALIDDATA;
                dst[0]          = bytestream2_get_byteu(&ctx->gb);
                dst[1]          = bytestream2_get_byteu(&ctx->gb);
                dst[0 + stride] = bytestream2_get_byteu(&ctx->gb);
                dst[1 + stride] = bytestream2_get_byteu(&ctx->gb);
            } else {
                size >>= 1;
                if (process_block(ctx, dst, prev1, prev2, stride, tbl, size))
                    return AVERROR_INVALIDDATA;
                if (process_block(ctx, dst + size, prev1 + size, prev2 + size,
                                  stride, tbl, size))
                    return AVERROR_INVALIDDATA;
                dst   += size * stride;
                prev1 += size * stride;
                prev2 += size * stride;
                if (process_block(ctx, dst, prev1, prev2, stride, tbl, size))
                    return AVERROR_INVALIDDATA;
                if (process_block(ctx, dst + size, prev1 + size, prev2 + size,
                                  stride, tbl, size))
                    return AVERROR_INVALIDDATA;
            }
            break;
        case 0xFE:
            if (bytestream2_get_bytes_left(&ctx->gb) < 1)
                return AVERROR_INVALIDDATA;

            t = bytestream2_get_byteu(&ctx->gb);
            for (k = 0; k < size; k++)
                memset(dst + k * stride, t, size);
            break;
        case 0xFD:
            if (bytestream2_get_bytes_left(&ctx->gb) < 3)
                return AVERROR_INVALIDDATA;

            code = bytestream2_get_byteu(&ctx->gb);
            pglyph = (size == 8) ? ctx->p8x8glyphs[code] : ctx->p4x4glyphs[code];
            bytestream2_get_bufferu(&ctx->gb, colors, 2);

            for (k = 0; k < size; k++)
                for (t = 0; t < size; t++)
                    dst[t + k * stride] = colors[!*pglyph++];
            break;
        case 0xFC:
            for (k = 0; k < size; k++)
                memcpy(dst + k * stride, prev1 + k * stride, size);
            break;
        default:
            k = bytestream2_tell(&ctx->gb);
            bytestream2_seek(&ctx->gb, tbl + (code & 7), SEEK_SET);
            t = bytestream2_get_byte(&ctx->gb);
            bytestream2_seek(&ctx->gb, k, SEEK_SET);
            for (k = 0; k < size; k++)
                memset(dst + k * stride, t, size);
        }
    } else {
        int mx = motion_vectors[code][0];
        int my = motion_vectors[code][1];
        int index = prev2 - (const uint8_t *)ctx->frm2;

        av_assert2(index >= 0 && index < (ctx->buf_size >> 1));

        if (index < -mx - my * stride ||
            (ctx->buf_size >> 1) - index < mx + size + (my + size - 1) * stride) {
            av_log(ctx->avctx, AV_LOG_ERROR, "MV is invalid.\n");
            return AVERROR_INVALIDDATA;
        }

        for (k = 0; k < size; k++)
            memcpy(dst + k * stride, prev2 + mx + (my + k) * stride, size);
    }

    return 0;
}

static int old_codec47(SANMVideoContext *ctx, int top,
                       int left, int width, int height)
{
    uint32_t decoded_size;
    int i, j;
    ptrdiff_t stride = ctx->pitch;
    uint8_t *dst   = (uint8_t *)ctx->frm0 + left + top * stride;
    uint8_t *prev1 = (uint8_t *)ctx->frm1;
    uint8_t *prev2 = (uint8_t *)ctx->frm2;
    int tbl_pos = bytestream2_tell(&ctx->gb);
    int seq     = bytestream2_get_le16(&ctx->gb);
    int compr   = bytestream2_get_byte(&ctx->gb);
    int new_rot = bytestream2_get_byte(&ctx->gb);
    int skip    = bytestream2_get_byte(&ctx->gb);

    bytestream2_skip(&ctx->gb, 9);
    decoded_size = bytestream2_get_le32(&ctx->gb);
    bytestream2_skip(&ctx->gb, 8);

    if (decoded_size > ctx->height * stride - left - top * stride) {
        decoded_size = ctx->height * stride - left - top * stride;
        av_log(ctx->avctx, AV_LOG_WARNING, "Decoded size is too large.\n");
    }

    if (skip & 1)
        bytestream2_skip(&ctx->gb, 0x8080);
    if (!seq) {
        ctx->prev_seq = -1;
        memset(prev1, 0, ctx->height * stride);
        memset(prev2, 0, ctx->height * stride);
    }

    switch (compr) {
    case 0:
        if (bytestream2_get_bytes_left(&ctx->gb) < width * height)
            return AVERROR_INVALIDDATA;
        for (j = 0; j < height; j++) {
            bytestream2_get_bufferu(&ctx->gb, dst, width);
            dst += stride;
        }
        break;
    case 1:
        if (bytestream2_get_bytes_left(&ctx->gb) < ((width + 1) >> 1) * ((height + 1) >> 1))
            return AVERROR_INVALIDDATA;
        for (j = 0; j < height; j += 2) {
            for (i = 0; i < width; i += 2) {
                dst[i] =
                dst[i + 1] =
                dst[stride + i] =
                dst[stride + i + 1] = bytestream2_get_byteu(&ctx->gb);
            }
            dst += stride * 2;
        }
        break;
    case 2:
        if (seq == ctx->prev_seq + 1) {
            for (j = 0; j < height; j += 8) {
                for (i = 0; i < width; i += 8)
                    if (process_block(ctx, dst + i, prev1 + i, prev2 + i, stride,
                                      tbl_pos + 8, 8))
                        return AVERROR_INVALIDDATA;
                dst   += stride * 8;
                prev1 += stride * 8;
                prev2 += stride * 8;
            }
        }
        break;
    case 3:
        memcpy(ctx->frm0, ctx->frm2, ctx->pitch * ctx->height);
        break;
    case 4:
        memcpy(ctx->frm0, ctx->frm1, ctx->pitch * ctx->height);
        break;
    case 5:
        if (rle_decode(ctx, dst, decoded_size))
            return AVERROR_INVALIDDATA;
        break;
    default:
        avpriv_report_missing_feature(ctx->avctx,
                                      "Subcodec 47 compression %d", compr);
        return AVERROR_PATCHWELCOME;
    }
    if (seq == ctx->prev_seq + 1)
        ctx->rotate_code = new_rot;
    else
        ctx->rotate_code = 0;
    ctx->prev_seq = seq;

    return 0;
}

static int process_frame_obj(SANMVideoContext *ctx)
{
    uint16_t codec = bytestream2_get_le16u(&ctx->gb);
    uint16_t left  = bytestream2_get_le16u(&ctx->gb);
    uint16_t top   = bytestream2_get_le16u(&ctx->gb);
    uint16_t w     = bytestream2_get_le16u(&ctx->gb);
    uint16_t h     = bytestream2_get_le16u(&ctx->gb);

    if (!w || !h) {
        av_log(ctx->avctx, AV_LOG_ERROR, "Dimensions are invalid.\n");
        return AVERROR_INVALIDDATA;
    }

    if (ctx->width < left + w || ctx->height < top + h) {
        int ret = ff_set_dimensions(ctx->avctx, FFMAX(left + w, ctx->width),
                                    FFMAX(top + h, ctx->height));
        if (ret < 0)
            return ret;
        init_sizes(ctx, FFMAX(left + w, ctx->width),
                   FFMAX(top + h, ctx->height));
        if (init_buffers(ctx)) {
            av_log(ctx->avctx, AV_LOG_ERROR, "Error resizing buffers.\n");
            return AVERROR(ENOMEM);
        }
    }
    bytestream2_skip(&ctx->gb, 4);

    switch (codec) {
    case 1:
    case 3:
        return old_codec1(ctx, top, left, w, h);
    case 37:
        return old_codec37(ctx, top, left, w, h);
    case 47:
        return old_codec47(ctx, top, left, w, h);
    default:
        avpriv_request_sample(ctx->avctx, "Subcodec %d", codec);
        return AVERROR_PATCHWELCOME;
    }
}

static int decode_0(SANMVideoContext *ctx)
{
    uint16_t *frm = ctx->frm0;
    int x, y;

    if (bytestream2_get_bytes_left(&ctx->gb) < ctx->width * ctx->height * 2) {
        av_log(ctx->avctx, AV_LOG_ERROR, "Insufficient data for raw frame.\n");
        return AVERROR_INVALIDDATA;
    }
    for (y = 0; y < ctx->height; y++) {
        for (x = 0; x < ctx->width; x++)
            frm[x] = bytestream2_get_le16u(&ctx->gb);
        frm += ctx->pitch;
    }
    return 0;
}

static int decode_nop(SANMVideoContext *ctx)
{
    avpriv_request_sample(ctx->avctx, "Unknown/unsupported compression type");
    return AVERROR_PATCHWELCOME;
}

{

    case 2:
        break;
    case 4:
        break;
    case 8:
        break;
    }

{

}

                      uint16_t fg_color, uint16_t bg_color, int block_size,
                      ptrdiff_t pitch)
{

        av_log(ctx->avctx, AV_LOG_ERROR, "Ignoring nonexistent glyph #%u.\n", index);
        return AVERROR_INVALIDDATA;
    }


    return 0;
}

{


            return AVERROR_INVALIDDATA;

    } else {

            return AVERROR_INVALIDDATA;


    }
    return 0;
}

{

            return AVERROR_INVALIDDATA;

    } else {

            return AVERROR_INVALIDDATA;


    }
    return 0;
}

                     int block_size)
{


    if (!good)
        av_log(ctx->avctx, AV_LOG_ERROR,
               "Ignoring invalid motion vector (%i, %i)->(%u, %u), block size = %u\n",
               cx + mx, cy + my, cx, cy, block_size);

}

{

        return AVERROR_INVALIDDATA;



                       blk_size, ctx->pitch);
        }
        break;
            return AVERROR_INVALIDDATA;


                       blk_size, ctx->pitch);
        }
        break;
                   blk_size, ctx->pitch);

    case 0xFA:
    case 0xFB:
    case 0xFC:
        break;
            return AVERROR_INVALIDDATA;
        break;
            return AVERROR_INVALIDDATA;
        break;
            opcode_0xf8(ctx, cx, cy, blk_size, ctx->pitch);
        } else {
                return AVERROR_INVALIDDATA;
                return AVERROR_INVALIDDATA;
                return AVERROR_INVALIDDATA;
                return AVERROR_INVALIDDATA;
        }
        break;
    }
    return 0;
}

{


    return 0;
}

static int decode_3(SANMVideoContext *ctx)
{
    memcpy(ctx->frm0, ctx->frm2, ctx->frm2_size);
    return 0;
}

{
}

{
#if HAVE_BIGENDIAN
    uint16_t *frm;
    int npixels;
#endif

        return AVERROR_INVALIDDATA;

#if HAVE_BIGENDIAN
    npixels = ctx->npixels;
    frm = ctx->frm0;
    while (npixels--) {
        *frm = av_bswap16(*frm);
        frm++;
    }
#endif

    return 0;
}

static int decode_6(SANMVideoContext *ctx)
{
    int npixels = ctx->npixels;
    uint16_t *frm = ctx->frm0;

    if (bytestream2_get_bytes_left(&ctx->gb) < npixels) {
        av_log(ctx->avctx, AV_LOG_ERROR, "Insufficient data for frame.\n");
        return AVERROR_INVALIDDATA;
    }
    while (npixels--)
        *frm++ = ctx->codebook[bytestream2_get_byteu(&ctx->gb)];

    return 0;
}

static int decode_8(SANMVideoContext *ctx)
{
    uint16_t *pdest = ctx->frm0;
    uint8_t *rsrc;
    long npixels = ctx->npixels;

    av_fast_malloc(&ctx->rle_buf, &ctx->rle_buf_size, npixels);
    if (!ctx->rle_buf) {
        av_log(ctx->avctx, AV_LOG_ERROR, "RLE buffer allocation failed.\n");
        return AVERROR(ENOMEM);
    }
    rsrc = ctx->rle_buf;

    if (rle_decode(ctx, rsrc, npixels))
        return AVERROR_INVALIDDATA;

    while (npixels--)
        *pdest++ = ctx->codebook[*rsrc++];

    return 0;
}

typedef int (*frm_decoder)(SANMVideoContext *ctx);

static const frm_decoder v1_decoders[] = {
    decode_0, decode_nop, decode_2, decode_3, decode_4, decode_5,
    decode_6, decode_nop, decode_8
};

{

        av_log(ctx->avctx, AV_LOG_ERROR, "Input frame too short (%d bytes).\n",
               ret);
        return AVERROR_INVALIDDATA;
    }


        avpriv_report_missing_feature(ctx->avctx, "Variable size frames");
        return AVERROR_PATCHWELCOME;
    }







}

{
    }
}

{

        return ret;


    }

    return 0;
}

                        int *got_frame_ptr, AVPacket *pkt)
{


        int to_store = 0;

        while (bytestream2_get_bytes_left(&ctx->gb) >= 8) {
            uint32_t sig, size;
            int pos;

            sig  = bytestream2_get_be32u(&ctx->gb);
            size = bytestream2_get_be32u(&ctx->gb);
            pos  = bytestream2_tell(&ctx->gb);

            if (bytestream2_get_bytes_left(&ctx->gb) < size) {
                av_log(avctx, AV_LOG_ERROR, "Incorrect chunk size %"PRIu32".\n", size);
                break;
            }
            switch (sig) {
            case MKBETAG('N', 'P', 'A', 'L'):
                if (size != PALETTE_SIZE * 3) {
                    av_log(avctx, AV_LOG_ERROR,
                           "Incorrect palette block size %"PRIu32".\n", size);
                    return AVERROR_INVALIDDATA;
                }
                for (i = 0; i < PALETTE_SIZE; i++)
                    ctx->pal[i] = 0xFFU << 24 | bytestream2_get_be24u(&ctx->gb);
                break;
            case MKBETAG('F', 'O', 'B', 'J'):
                if (size < 16)
                    return AVERROR_INVALIDDATA;
                if (ret = process_frame_obj(ctx))
                    return ret;
                break;
            case MKBETAG('X', 'P', 'A', 'L'):
                if (size == 6 || size == 4) {
                    uint8_t tmp[3];
                    int j;

                    for (i = 0; i < PALETTE_SIZE; i++) {
                        for (j = 0; j < 3; j++) {
                            int t = (ctx->pal[i] >> (16 - j * 8)) & 0xFF;
                            tmp[j] = av_clip_uint8((t * 129 + ctx->delta_pal[i * 3 + j]) >> 7);
                        }
                        ctx->pal[i] = 0xFFU << 24 | AV_RB24(tmp);
                    }
                } else {
                    if (size < PALETTE_DELTA * 2 + 4) {
                        av_log(avctx, AV_LOG_ERROR,
                               "Incorrect palette change block size %"PRIu32".\n",
                               size);
                        return AVERROR_INVALIDDATA;
                    }
                    bytestream2_skipu(&ctx->gb, 4);
                    for (i = 0; i < PALETTE_DELTA; i++)
                        ctx->delta_pal[i] = bytestream2_get_le16u(&ctx->gb);
                    if (size >= PALETTE_DELTA * 5 + 4) {
                        for (i = 0; i < PALETTE_SIZE; i++)
                            ctx->pal[i] = 0xFFU << 24 | bytestream2_get_be24u(&ctx->gb);
                    } else {
                        memset(ctx->pal, 0, sizeof(ctx->pal));
                    }
                }
                break;
            case MKBETAG('S', 'T', 'O', 'R'):
                to_store = 1;
                break;
            case MKBETAG('F', 'T', 'C', 'H'):
                memcpy(ctx->frm0, ctx->stored_frame, ctx->buf_size);
                break;
            default:
                bytestream2_skip(&ctx->gb, size);
                av_log(avctx, AV_LOG_DEBUG,
                       "Unknown/unsupported chunk %"PRIx32".\n", sig);
                break;
            }

            bytestream2_seek(&ctx->gb, pos + size, SEEK_SET);
            if (size & 1)
                bytestream2_skip(&ctx->gb, 1);
        }
        if (to_store)
            memcpy(ctx->stored_frame, ctx->frm0, ctx->buf_size);
        if ((ret = copy_output(ctx, NULL)))
            return ret;
        memcpy(ctx->frame->data[1], ctx->pal, 1024);
    } else {


        } else {
        }

                       "Subcodec %d: error decoding frame.\n", header.codec);
            }
        } else {
            avpriv_request_sample(avctx, "Subcodec %d", header.codec);
            return AVERROR_PATCHWELCOME;
        }

            return ret;
    }


}

AVCodec ff_sanm_decoder = {
    .name           = "sanm",
    .long_name      = NULL_IF_CONFIG_SMALL("LucasArts SANM/Smush video"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_SANM,
    .priv_data_size = sizeof(SANMVideoContext),
    .init           = decode_init,
    .close          = decode_end,
    .decode         = decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
