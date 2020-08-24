/*
 * SMV JPEG decoder
 * Copyright (c) 2013 Ash Hughes
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
 * SMV JPEG decoder.
 */

// #define DEBUG
#include "avcodec.h"
#include "libavutil/opt.h"
#include "libavutil/imgutils.h"
#include "mjpegdec.h"
#include "internal.h"

typedef struct SMVJpegDecodeContext {
    MJpegDecodeContext jpg;
    AVFrame *picture[2]; /* pictures array */
    AVCodecContext* avctx;
    int frames_per_jpeg;
    int mjpeg_data_size;
} SMVJpegDecodeContext;

static inline void smv_img_pnt_plane(uint8_t      **dst, uint8_t *src,
                                     int src_linesize, int height, int nlines)
{
    if (!dst || !src)
        return;
    src += (nlines) * src_linesize * height;
    *dst = src;
}

static inline void smv_img_pnt(uint8_t *dst_data[4], uint8_t *src_data[4],
                               const int src_linesizes[4],
                               enum AVPixelFormat pix_fmt, int width, int height,
                               int nlines)
{
    const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(pix_fmt);
    int i, planes_nb = 0;

    if (desc->flags & AV_PIX_FMT_FLAG_HWACCEL)
        return;

    for (i = 0; i < desc->nb_components; i++)
        planes_nb = FFMAX(planes_nb, desc->comp[i].plane + 1);

    for (i = 0; i < planes_nb; i++) {
        int h = height;
        if (i == 1 || i == 2) {
            h = AV_CEIL_RSHIFT(height, desc->log2_chroma_h);
        }
        smv_img_pnt_plane(&dst_data[i], src_data[i],
            src_linesizes[i], h, nlines);
    }
    if (desc->flags & AV_PIX_FMT_FLAG_PAL ||
        desc->flags & FF_PSEUDOPAL)
        dst_data[1] = src_data[1];
}

{

}

{


        return AVERROR(ENOMEM);

        av_frame_free(&s->picture[0]);
        return AVERROR(ENOMEM);
    }



        av_log(avctx, AV_LOG_ERROR, "Invalid number of frames per jpeg.\n");
        ret = AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "MJPEG codec not found\n");
        smvjpeg_decode_end(avctx);
        return AVERROR_DECODER_NOT_FOUND;
    }


        av_log(avctx, AV_LOG_ERROR, "MJPEG codec failed to open\n");
        ret = r;
    }

        smvjpeg_decode_end(avctx);
    return ret;
}

                            AVPacket *avpkt)
{


    /* cur_frame is later used to calculate the buffer offset, so it mustn't be negative */
        cur_frame += s->frames_per_jpeg;

    /* Are we at the start of a block? */
            s->mjpeg_data_size = 0;
            return ret;
        }
        return AVERROR(EINVAL);


        av_log(avctx, AV_LOG_ERROR, "Invalid height\n");
        return AVERROR_INVALIDDATA;
    }

    /*use the last lot... */


    /* We shouldn't get here if frames_per_jpeg <= 0 because this was rejected
       in init */
        av_log(s, AV_LOG_ERROR, "Failed to set dimensions\n");
        return ret;
    }

                    avctx->pix_fmt, avctx->width, avctx->height, cur_frame);

            return ret;
    }

}

static const AVClass smvjpegdec_class = {
    .class_name = "SMVJPEG decoder",
    .item_name  = av_default_item_name,
    .version    = LIBAVUTIL_VERSION_INT,
};

AVCodec ff_smvjpeg_decoder = {
    .name           = "smvjpeg",
    .long_name      = NULL_IF_CONFIG_SMALL("SMV JPEG"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_SMVJPEG,
    .priv_data_size = sizeof(SMVJpegDecodeContext),
    .init           = smvjpeg_decode_init,
    .close          = smvjpeg_decode_end,
    .decode         = smvjpeg_decode_frame,
    .priv_class     = &smvjpegdec_class,
};
