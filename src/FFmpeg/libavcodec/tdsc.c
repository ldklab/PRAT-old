/*
 * TDSC decoder
 * Copyright (C) 2015 Vittorio Giovara <vittorio.giovara@gmail.com>
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
 * TDSC decoder
 *
 * Fourcc: TSDC
 *
 * TDSC is very simple. It codes picture by tiles, storing them in raw BGR24
 * format or compressing them in JPEG. Frames can be full pictures or just
 * updates to the previous frame. Cursor is found in its own frame or at the
 * bottom of the picture. Every frame is then packed with zlib.
 *
 * Supports: BGR24
 */

#include <stdint.h>
#include <zlib.h>

#include "libavutil/imgutils.h"

#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"

#define BITMAPINFOHEADER_SIZE 0x28
#define TDSF_HEADER_SIZE      0x56
#define TDSB_HEADER_SIZE      0x08

typedef struct TDSCContext {
    AVCodecContext *jpeg_avctx;   // wrapper context for MJPEG

    int width, height;
    GetByteContext gbc;

    AVFrame *refframe;          // full decoded frame (without cursor)
    AVFrame *jpgframe;          // decoded JPEG tile
    uint8_t *tilebuffer;        // buffer containing tile data

    /* zlib interaction */
    uint8_t *deflatebuffer;
    uLongf deflatelen;

    /* All that is cursor */
    uint8_t    *cursor;
    int        cursor_stride;
    int        cursor_w, cursor_h, cursor_x, cursor_y;
    int        cursor_hot_x, cursor_hot_y;
} TDSCContext;

/* 1 byte bits, 1 byte planes, 2 bytes format (probably) */
enum TDSCCursorFormat {
    CUR_FMT_MONO = 0x01010004,
    CUR_FMT_BGRA = 0x20010004,
    CUR_FMT_RGBA = 0x20010008,
};

{


}

{


    /* These needs to be set to estimate buffer and frame size */
        av_log(avctx, AV_LOG_ERROR, "Video size not set.\n");
        return AVERROR_INVALIDDATA;
    }

    /* This value should be large enough for a RAW-only frame plus headers */
        return ret;

    /* Allocate reference and JPEG frame */
        return AVERROR(ENOMEM);

    /* Prepare everything needed for JPEG decoding */
        return AVERROR_BUG;
        return AVERROR(ENOMEM);
        return ret;

    /* Set the output pixel format on the reference frame */

}

#define APPLY_ALPHA(src, new, alpha) \
    src = (src * (256 - alpha) + new * alpha) >> 8

/* Paint a region over a buffer, without drawing out of its bounds. */
static void tdsc_paint_cursor(AVCodecContext *avctx, uint8_t *dst, int stride)
{
    TDSCContext *ctx = avctx->priv_data;
    const uint8_t *cursor = ctx->cursor;
    int x = ctx->cursor_x - ctx->cursor_hot_x;
    int y = ctx->cursor_y - ctx->cursor_hot_y;
    int w = ctx->cursor_w;
    int h = ctx->cursor_h;
    int i, j;

    if (!ctx->cursor)
        return;

    if (x + w > ctx->width)
        w = ctx->width - x;
    if (y + h > ctx->height)
        h = ctx->height - y;
    if (x < 0) {
        w      +=  x;
        cursor += -x * 4;
    } else {
        dst    +=  x * 3;
    }
    if (y < 0) {
        h      +=  y;
        cursor += -y * ctx->cursor_stride;
    } else {
        dst    +=  y * stride;
    }
    if (w < 0 || h < 0)
        return;

    for (j = 0; j < h; j++) {
        for (i = 0; i < w; i++) {
            uint8_t alpha = cursor[i * 4];
            APPLY_ALPHA(dst[i * 3 + 0], cursor[i * 4 + 1], alpha);
            APPLY_ALPHA(dst[i * 3 + 1], cursor[i * 4 + 2], alpha);
            APPLY_ALPHA(dst[i * 3 + 2], cursor[i * 4 + 3], alpha);
        }
        dst    += stride;
        cursor += ctx->cursor_stride;
    }
}

/* Load cursor data and store it in ABGR mode. */
{



        av_log(avctx, AV_LOG_ERROR,
               "Invalid cursor position (%d.%d outside %dx%d).\n",
               ctx->cursor_x, ctx->cursor_y, avctx->width, avctx->height);
        return AVERROR_INVALIDDATA;
    }
        av_log(avctx, AV_LOG_ERROR,
               "Invalid cursor dimensions %dx%d.\n",
               ctx->cursor_w, ctx->cursor_h);
        return AVERROR_INVALIDDATA;
    }
        ctx->cursor_hot_y > ctx->cursor_h) {
        av_log(avctx, AV_LOG_WARNING, "Invalid hotspot position %d.%d.\n",
               ctx->cursor_hot_x, ctx->cursor_hot_y);
        ctx->cursor_hot_x = FFMIN(ctx->cursor_hot_x, ctx->cursor_w - 1);
        ctx->cursor_hot_y = FFMIN(ctx->cursor_hot_y, ctx->cursor_h - 1);
    }

        av_log(avctx, AV_LOG_ERROR, "Cannot allocate cursor buffer.\n");
        return ret;
    }

    /* here data is packed in BE */
    case CUR_FMT_MONO:
                }
            }
        }

                    }
                }
            }
        }
        break;
    case CUR_FMT_RGBA:
        /* Skip monochrome version of the cursor */
                }
            }
        } else { // BGRA -> ABGR
            for (j = 0; j < ctx->cursor_h; j++) {
                for (i = 0; i < ctx->cursor_w; i++) {
                    int val = bytestream2_get_be32(&ctx->gbc);
                    *dst++ = val >>  0;
                    *dst++ = val >> 24;
                    *dst++ = val >> 16;
                    *dst++ = val >>  8;
                }
                dst += ctx->cursor_stride - ctx->cursor_w * 4;
            }
        }
        break;
    default:
        avpriv_request_sample(avctx, "Cursor format %08x", cursor_fmt);
        return AVERROR_PATCHWELCOME;
    }

    return 0;
}

/* Convert a single YUV pixel to RGB. */
{

/* Convert a YUV420 buffer to a RGB buffer. */
                                       const uint8_t *srcy, int srcy_stride,
                                       const uint8_t *srcu, const uint8_t *srcv,
                                       int srcuv_stride, int width, int height)
{

    }
}

/* Invoke the MJPEG decoder to decode the tile. */
                                 int x, int y, int w, int h)
{

    /* Prepare a packet and send to the MJPEG decoder */

        av_log(avctx, AV_LOG_ERROR, "Error submitting a packet for decoding\n");
        return ret;
    }

        av_log(avctx, AV_LOG_ERROR,
               "JPEG decoding error (%d).\n", ret);

        /* Normally skip, error if explode */
        if (avctx->err_recognition & AV_EF_EXPLODE)
            return AVERROR_INVALIDDATA;
        else
            return 0;
    }

    /* Let's paint onto the buffer */
              ctx->jpgframe->linesize[1], w, h);


}

/* Parse frame and either copy data or decode JPEG. */
{

    /* Iterate over the number of tiles */

            av_log(avctx, AV_LOG_ERROR, "TDSB tag is too small.\n");
            return AVERROR_INVALIDDATA;
        }

            return AVERROR_INVALIDDATA;


        ) {
            av_log(avctx, AV_LOG_ERROR,
                   "Invalid tile position (%d.%d %d.%d outside %dx%d).\n",
                   x, y, x2, y2, ctx->width, ctx->height);
            return AVERROR_INVALIDDATA;
        }

            return ret;


            /* Decode JPEG tile and copy it in the reference frame */
                return ret;
            /* Just copy the buffer to output */
                                w * 3, w * 3, h);
        } else {
            av_log(avctx, AV_LOG_ERROR, "Unknown tile type %08x.\n", tile_mode);
            return AVERROR_INVALIDDATA;
        }
    }

    return 0;
}

{

    /* BITMAPINFOHEADER
     * http://msdn.microsoft.com/en-us/library/windows/desktop/dd183376.aspx */
        return AVERROR_INVALIDDATA;

    /* Store size, but wait for context reinit before updating avctx */

        return AVERROR_INVALIDDATA;


    /* Update sizes */
        av_log(avctx, AV_LOG_DEBUG, "Size update %dx%d -> %d%d.\n",
               avctx->width, avctx->height, ctx->width, ctx->height);
        ret = ff_set_dimensions(avctx, w, h);
        if (ret < 0)
            return ret;
        init_refframe = 1;
    }

    /* Allocate the reference frame if not already done or on size change */
            return ret;
    }

    /* Decode all tiles in a frame */
}

{


        /* Load cursor coordinates */

        /* Load a full cursor sprite */
            /* Do not consider cursor errors fatal unless in explode mode */
                return ret;
        }
    } else {
        avpriv_request_sample(avctx, "Cursor action %d", action);
    }

    return 0;
}

                             int *got_frame, AVPacket *avpkt)
{

    /* Resize deflate buffer on resolution change */
            return ret;
    }

    /* Frames are deflated, need to inflate them first */
    }


    /* Check for tag and for size info */
        av_log(avctx, AV_LOG_ERROR, "Frame is too small.\n");
        return AVERROR_INVALIDDATA;
    }

    /* Read tag */

            av_log(avctx, AV_LOG_ERROR, "TDSF tag is too small.\n");
            return AVERROR_INVALIDDATA;
        }
        /* First 4 bytes here are the number of GEPJ/WAR tiles in this frame */


            return ret;

        /* Check if there is anything else we are able to parse */
    }

    /* This tag can be after a TDSF block or on its own frame */
        /* First 4 bytes here are the total size in bytes for this frame */

            av_log(avctx, AV_LOG_ERROR, "DTSM tag is too small.\n");
            return AVERROR_INVALIDDATA;
        }

            return ret;
    }

    /* Get the output frame and copy the reference frame */
        return ret;

        return ret;

    /* Paint the cursor on the output frame */

    /* Frame is ready to be output */
    } else {
    }

}

AVCodec ff_tdsc_decoder = {
    .name           = "tdsc",
    .long_name      = NULL_IF_CONFIG_SMALL("TDSC"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_TDSC,
    .init           = tdsc_init,
    .decode         = tdsc_decode_frame,
    .close          = tdsc_close,
    .priv_data_size = sizeof(TDSCContext),
    .capabilities   = AV_CODEC_CAP_DR1,
    .caps_internal  = FF_CODEC_CAP_INIT_THREADSAFE |
                      FF_CODEC_CAP_INIT_CLEANUP,
};
