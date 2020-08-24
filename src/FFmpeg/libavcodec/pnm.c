/*
 * PNM image format
 * Copyright (c) 2002, 2003 Fabrice Bellard
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

#include <stdlib.h>
#include <string.h>

#include "libavutil/avassert.h"
#include "libavutil/imgutils.h"
#include "libavutil/avstring.h"
#include "avcodec.h"
#include "internal.h"
#include "pnm.h"

{
    return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

static void pnm_get(PNMContext *sc, char *str, int buf_size)
{
    char *s;
    int c;
    uint8_t *bs  = sc->bytestream;
    const uint8_t *end = sc->bytestream_end;

    /* skip spaces and comments */
    while (bs < end) {
        c = *bs++;
        if (c == '#')  {
            while (c != '\n' && bs < end) {
                c = *bs++;
            }
        } else if (!pnm_space(c)) {
            break;
        }
    }

    s = str;
    while (bs < end && !pnm_space(c) && (s - str) < buf_size - 1) {
        *s++ = c;
        c = *bs++;
    }
    *s = '\0';
    sc->bytestream = bs;
}

{

         s->bytestream[1] != 'F')) {
    }

        avctx->pix_fmt = AV_PIX_FMT_GBRPF32;
        else
                       /* libavcodec used to write invalid files */
                break;
            } else {
                return AVERROR_INVALIDDATA;
            }
        }
            return AVERROR_INVALIDDATA;

        /* check that all tags are present */
            return AVERROR_INVALIDDATA;

            return ret;
            } else {
            }
            if (maxval < 256) {
                avctx->pix_fmt = AV_PIX_FMT_GRAY8A;
            } else {
                avctx->pix_fmt = AV_PIX_FMT_YA16;
            }
            } else {
            }
            } else {
                avctx->pix_fmt = AV_PIX_FMT_RGBA64;
            }
        } else {
            return AVERROR_INVALIDDATA;
        }
    } else {
        av_assert0(0);
    }
        return AVERROR_INVALIDDATA;

        return ret;

        pnm_get(s, buf1, sizeof(buf1));
        if (av_sscanf(buf1, "%f", &s->scale) != 1 || s->scale == 0.0 || !isfinite(s->scale)) {
            av_log(avctx, AV_LOG_ERROR, "Invalid scale.\n");
            return AVERROR_INVALIDDATA;
        }
        s->endian = s->scale < 0.f;
        s->scale = fabsf(s->scale);
        s->maxval = (1ULL << 32) - 1;
            av_log(avctx, AV_LOG_ERROR, "Invalid maxval: %d\n", s->maxval);
            s->maxval = 255;
        }
            if (avctx->pix_fmt == AV_PIX_FMT_GRAY8) {
                avctx->pix_fmt = AV_PIX_FMT_GRAY16;
            } else if (avctx->pix_fmt == AV_PIX_FMT_RGB24) {
                avctx->pix_fmt = AV_PIX_FMT_RGB48;
            } else if (avctx->pix_fmt == AV_PIX_FMT_YUV420P && s->maxval < 65536) {
                if (s->maxval < 512)
                    avctx->pix_fmt = AV_PIX_FMT_YUV420P9;
                else if (s->maxval < 1024)
                    avctx->pix_fmt = AV_PIX_FMT_YUV420P10;
                else
                    avctx->pix_fmt = AV_PIX_FMT_YUV420P16;
            } else {
                av_log(avctx, AV_LOG_ERROR, "Unsupported pixel format\n");
                avctx->pix_fmt = AV_PIX_FMT_NONE;
                return AVERROR_INVALIDDATA;
            }
        }
    }else

        return AVERROR_INVALIDDATA;

    /* more check if YUV420 */
            return AVERROR_INVALIDDATA;
            return AVERROR_INVALIDDATA;
    }
    return 0;
}
