/*
 * Microsoft Screen 2 (aka Windows Media Video V9 Screen) decoder
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
 * Microsoft Screen 2 (aka Windows Media Video V9 Screen) decoder DSP routines
 */

#include "mss2dsp.h"
#include "libavutil/common.h"

                                                     ptrdiff_t dst_stride,
                                                     int gray,
                                                     int use_mask,
                                                     int maskcolor,
                                                     const uint8_t *mask,
                                                     ptrdiff_t mask_stride,
                                                     const uint8_t *srcy,
                                                     ptrdiff_t srcy_stride,
                                                     const uint8_t *srcu,
                                                     const uint8_t *srcv,
                                                     ptrdiff_t srcuv_stride,
                                                     int w, int h)
{
                    dst[k] = dst[k + 1] = dst[k + 2] = 0x80;
                } else {
                }
            }
        }
    }
}

static void mss2_blit_wmv9_c(uint8_t *dst, ptrdiff_t dst_stride,
                             const uint8_t *srcy, ptrdiff_t srcy_stride,
                             const uint8_t *srcu, const uint8_t *srcv,
                             ptrdiff_t srcuv_stride, int w, int h)
{
    mss2_blit_wmv9_template(dst, dst_stride, 0, 0,
                            0, NULL, 0,
                            srcy, srcy_stride,
                            srcu, srcv, srcuv_stride,
                            w, h);
}

                                    int maskcolor, const uint8_t *mask,
                                    ptrdiff_t mask_stride,
                                    const uint8_t *srcy, ptrdiff_t srcy_stride,
                                    const uint8_t *srcu, const uint8_t *srcv,
                                    ptrdiff_t srcuv_stride, int w, int h)
{
                            maskcolor, mask, mask_stride,
                            srcy, srcy_stride,
                            srcu, srcv, srcuv_stride,
                            w, h);

static void mss2_gray_fill_masked_c(uint8_t *dst, ptrdiff_t dst_stride,
                                    int maskcolor, const uint8_t *mask,
                                    ptrdiff_t mask_stride, int w, int h)
{
    mss2_blit_wmv9_template(dst, dst_stride, 1, 1,
                            maskcolor, mask, mask_stride,
                            NULL, 0,
                            NULL, NULL, 0,
                            w, h);
}

{

        return;



           w);


        }
    }



        }
    }
}

{
