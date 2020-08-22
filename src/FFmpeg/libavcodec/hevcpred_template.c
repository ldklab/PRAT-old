/*
 * HEVC video decoder
 *
 * Copyright (C) 2012 - 2013 Guillaume Martres
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

#include "libavutil/pixdesc.h"

#include "bit_depth_template.c"
#include "hevcpred.h"

#define POS(x, y) src[(x) + stride * (y)]

                                              int log2_size, int c_idx)
{
#define PU(x) \
    ((x) >> s->ps.sps->log2_min_pu_size)
#define MVF(x, y) \
    (s->ref->tab_mvf[(x) + (y) * min_pu_width])
#define MVF_PU(x, y) \
    MVF(PU(x0 + ((x) * (1 << hshift))), PU(y0 + ((y) * (1 << vshift))))
#define IS_INTRA(x, y) \
    (MVF_PU(x, y).pred_flag == PF_INTRA)
#define MIN_TB_ADDR_ZS(x, y) \
    s->ps.pps->min_tb_addr_zs[(y) * (s->ps.sps->tb_mask+2) + (x)]
#define EXTEND(ptr, val, len)         \
do {                                  \
    pixel4 pix = PIXEL_SPLAT_X4(val); \
    for (i = 0; i < (len); i += 4)    \
        AV_WN4P(ptr + i, pix);        \
} while (0)

#define EXTEND_RIGHT_CIP(ptr, start, length)                                   \
        for (i = start; i < (start) + (length); i += 4)                        \
            if (!IS_INTRA(i, -1))                                              \
                AV_WN4P(&ptr[i], a);                                           \
            else                                                               \
                a = PIXEL_SPLAT_X4(ptr[i+3])
#define EXTEND_LEFT_CIP(ptr, start, length) \
        for (i = start; i > (start) - (length); i--) \
            if (!IS_INTRA(i - 1, -1)) \
                ptr[i - 1] = ptr[i]
#define EXTEND_UP_CIP(ptr, start, length)                                      \
        for (i = (start); i > (start) - (length); i -= 4)                      \
            if (!IS_INTRA(-1, i - 3))                                          \
                AV_WN4P(&ptr[i - 3], a);                                       \
            else                                                               \
                a = PIXEL_SPLAT_X4(ptr[i - 3])
#define EXTEND_DOWN_CIP(ptr, start, length)                                    \
        for (i = start; i < (start) + (length); i += 4)                        \
            if (!IS_INTRA(-1, i))                                              \
                AV_WN4P(&ptr[i], a);                                           \
            else                                                               \
                a = PIXEL_SPLAT_X4(ptr[i + 3])








            size_in_luma_pu_h++;
        }
        }
        }
        }
        }
    }
    }
               size - top_right_size);
    }
               size - bottom_left_size);
    }

            }
            }
                    j = 0;
                    while (j < size_max_x && !IS_INTRA(j, -1))
                        j++;
                    EXTEND_LEFT_CIP(top, j, j + 1);
                    left[-1] = top[-1];
                }
            } else {
                j = 0;
                    } else {
                    }
                left[-1] = top[-1];
            }
            }
            } else {
            }
            }
        }
    }
    // Infer the unavailable samples
            cand_left = 1;
            cand_up_left = 1;
            cand_left    = 1;
            cand_up      = 1;
            cand_up_left = 1;
            cand_left    = 1;
        } else { // No samples available
        }
    }

    }


    // Filtering process
                                          FFABS((int)(mode - 10U)));
                    // We can't just overwrite values in top because it could be
                    // a pointer into src
                    top = filtered_top;
                } else {
                    left = filtered_left;
                    top  = filtered_top;
                }
            }
        }
    }

                                          (uint8_t *)left, stride);
                       (uint8_t *)left, stride, log2_size, c_idx);
                                           (uint8_t *)left, stride, c_idx,
                                           mode);
    }
}

#define INTRA_PRED(size)                                                            \
static void FUNC(intra_pred_ ## size)(HEVCContext *s, int x0, int y0, int c_idx)    \
{                                                                                   \
    FUNC(intra_pred)(s, x0, y0, size, c_idx);                                       \
}


#undef INTRA_PRED

                                  const uint8_t *_left, ptrdiff_t stride,
                                  int trafo_size)
{
}

#define PRED_PLANAR(size)\
static void FUNC(pred_planar_ ## size)(uint8_t *src, const uint8_t *top,        \
                                       const uint8_t *left, ptrdiff_t stride)   \
{                                                                               \
    FUNC(pred_planar)(src, top, left, stride, size + 2);                        \
}


#undef PRED_PLANAR

                          const uint8_t *_left,
                          ptrdiff_t stride, int log2_size, int c_idx)
{




    }

                                                const uint8_t *_top,
                                                const uint8_t *_left,
                                                ptrdiff_t stride, int c_idx,
                                                int mode, int size)
{

         32,  26,  21,  17, 13,  9,  5, 2, 0, -2, -5, -9, -13, -17, -21, -26, -32,
        -26, -21, -17, -13, -9, -5, -2, 0, 2,  5,  9, 13,  17,  21,  26,  32
    };
        -4096, -1638, -910, -630, -482, -390, -315, -256, -315, -390, -482,
        -630, -910, -1638, -4096
    };


            ref = ref_tmp;
        }

                }
            } else {
            }
        }
        }
    } else {
            ref = ref_tmp;
        }

                }
            } else {
            }
        }
            }
        }
    }
}

                                 const uint8_t *left,
                                 ptrdiff_t stride, int c_idx, int mode)
{

                                 const uint8_t *left,
                                 ptrdiff_t stride, int c_idx, int mode)
{

                                 const uint8_t *left,
                                 ptrdiff_t stride, int c_idx, int mode)
{

                                 const uint8_t *left,
                                 ptrdiff_t stride, int c_idx, int mode)
{

#undef EXTEND_LEFT_CIP
#undef EXTEND_RIGHT_CIP
#undef EXTEND_UP_CIP
#undef EXTEND_DOWN_CIP
#undef IS_INTRA
#undef MVF_PU
#undef MVF
#undef PU
#undef EXTEND
#undef MIN_TB_ADDR_ZS
#undef POS
