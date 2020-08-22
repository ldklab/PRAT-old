/*
 * Pictor/PC Paint decoder
 * Copyright (c) 2010 Peter Ross <pross@xvid.org>
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
 * Pictor/PC Paint decoder
 */

#include "libavutil/imgutils.h"
#include "avcodec.h"
#include "bytestream.h"
#include "cga_data.h"
#include "internal.h"

typedef struct PicContext {
    int width, height;
    int nb_planes;
    GetByteContext g;
} PicContext;

static void picmemset_8bpp(PicContext *s, AVFrame *frame, int value, int run,
                           int *x, int *y)
{
    while (run > 0) {
        uint8_t *d = frame->data[0] + *y * frame->linesize[0];
        if (*x + run >= s->width) {
            int n = s->width - *x;
            memset(d + *x, value, n);
            run -= n;
            *x = 0;
            *y -= 1;
            if (*y < 0)
                break;
        } else {
            memset(d + *x, value, run);
            *x += run;
            break;
        }
    }
}

static void picmemset(PicContext *s, AVFrame *frame, unsigned value, int run,
                      int *x, int *y, int *plane, int bits_per_plane)
{
    uint8_t *d;
    int shift = *plane * bits_per_plane;
    unsigned mask  = ((1U << bits_per_plane) - 1) << shift;
    int xl = *x;
    int yl = *y;
    int planel = *plane;
    int pixels_per_value = 8/bits_per_plane;
    value   <<= shift;

    d = frame->data[0] + yl * frame->linesize[0];
    while (run > 0) {
        int j;
        for (j = 8-bits_per_plane; j >= 0; j -= bits_per_plane) {
            d[xl] |= (value >> j) & mask;
            xl += 1;
            while (xl == s->width) {
                yl -= 1;
                xl = 0;
                if (yl < 0) {
                   yl = s->height - 1;
                   planel += 1;
                   if (planel >= s->nb_planes)
                       goto end;
                   value <<= bits_per_plane;
                   mask  <<= bits_per_plane;
                }
                d = frame->data[0] + yl * frame->linesize[0];
                if (s->nb_planes == 1 &&
                    run*pixels_per_value >= s->width &&
                    pixels_per_value < s->width &&
                    s->width % pixels_per_value == 0
                    ) {
                    for (; xl < pixels_per_value; xl ++) {
                        j = (j < bits_per_plane ? 8 : j) - bits_per_plane;
                        d[xl] |= (value >> j) & mask;
                    }
                    av_memcpy_backptr(d+xl, pixels_per_value, s->width - xl);
                    run -= s->width / pixels_per_value;
                    xl = s->width;
                }
            }
        }
        run--;
    }
end:
    *x = xl;
    *y = yl;
    *plane = planel;
}

static const uint8_t cga_mode45_index[6][4] = {
    [0] = { 0, 3,  5,   7 }, // mode4, palette#1, low intensity
    [1] = { 0, 2,  4,   6 }, // mode4, palette#2, low intensity
    [2] = { 0, 3,  4,   7 }, // mode5, low intensity
    [3] = { 0, 11, 13, 15 }, // mode4, palette#1, high intensity
    [4] = { 0, 10, 12, 14 }, // mode4, palette#2, high intensity
    [5] = { 0, 11, 12, 15 }, // mode5, high intensity
};

                        void *data, int *got_frame,
                        AVPacket *avpkt)
{


        return AVERROR_INVALIDDATA;

        return AVERROR_INVALIDDATA;

        avpriv_request_sample(avctx, "Unsupported bit depth");
        return AVERROR_PATCHWELCOME;
    }

            return AVERROR_INVALIDDATA;
    } else {
        etype = -1;
        esize = 0;
    }


        return -1;
            return ret;
    }

        return ret;

        int idx = bytestream2_get_byte(&s->g);
        npal = 4;
        for (i = 0; i < npal; i++)
            palette[i] = ff_cga_palette[ cga_mode45_index[idx][i] ];
        npal = FFMIN(esize, 16);
        for (i = 0; i < npal; i++) {
            int pal_idx = bytestream2_get_byte(&s->g);
            palette[i]  = ff_cga_palette[FFMIN(pal_idx, 15)];
        }
        npal = FFMIN(esize, 16);
        for (i = 0; i < npal; i++) {
            int pal_idx = bytestream2_get_byte(&s->g);
            palette[i]  = ff_ega_palette[FFMIN(pal_idx, 63)];
        }
        }
    } else {
        if (bpp == 1) {
            npal = 2;
            palette[0] = 0xFF000000;
            palette[1] = 0xFFFFFFFF;
        } else if (bpp == 2) {
            npal = 4;
            for (i = 0; i < npal; i++)
                palette[i] = ff_cga_palette[ cga_mode45_index[0][i] ];
        } else {
            npal = 16;
            memcpy(palette, ff_cga_palette, npal * 4);
        }
    }
    // fill remaining palette entries
    // skip remaining palette bytes


            // ignore uncompressed block size

                        run = bytestream2_get_le16(&s->g);
                }
                    break;

                    picmemset_8bpp(s, frame, val, run, &x, &y);
                    if (y < 0)
                        goto finish;
                } else {
                }
            }
        }

            return AVERROR_INVALIDDATA;

                picmemset_8bpp(s, frame, val, run, &x, &y);
            else
        }
    } else {
        while (y >= 0 && bytestream2_get_bytes_left(&s->g) > 0) {
            memcpy(frame->data[0] + y * frame->linesize[0], s->g.buffer, FFMIN(avctx->width, bytestream2_get_bytes_left(&s->g)));
            bytestream2_skip(&s->g, avctx->width);
            y--;
        }
    }
finish:

}

AVCodec ff_pictor_decoder = {
    .name           = "pictor",
    .long_name      = NULL_IF_CONFIG_SMALL("Pictor/PC Paint"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_PICTOR,
    .priv_data_size = sizeof(PicContext),
    .decode         = decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
