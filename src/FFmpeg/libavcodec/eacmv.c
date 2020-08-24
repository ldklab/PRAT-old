/*
 * Electronic Arts CMV Video Decoder
 * Copyright (c) 2007-2008 Peter Ross
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * @file
 * Electronic Arts CMV Video Decoder
 * by Peter Ross (pross@xvid.org)
 *
 * Technical details here:
 * http://wiki.multimedia.cx/index.php?title=Electronic_Arts_CMV
 */

#include "libavutil/common.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/imgutils.h"
#include "avcodec.h"
#include "internal.h"

typedef struct CmvContext {
    AVCodecContext *avctx;
    AVFrame *last_frame;   ///< last
    AVFrame *last2_frame;  ///< second-last
    int width, height;
    unsigned int palette[AVPALETTE_COUNT];
} CmvContext;



        av_frame_free(&s->last_frame);
        av_frame_free(&s->last2_frame);
        return AVERROR(ENOMEM);
    }

    return 0;
}

static void cmv_decode_intra(CmvContext * s, AVFrame *frame,
                             const uint8_t *buf, const uint8_t *buf_end)
{
    unsigned char *dst = frame->data[0];
    int i;

    for (i=0; i < s->avctx->height && buf_end - buf >= s->avctx->width; i++) {
        memcpy(dst, buf, s->avctx->width);
        dst += frame->linesize[0];
        buf += s->avctx->width;
    }
}

                        const unsigned char *src, ptrdiff_t src_stride,
                        int x, int y,
                        int xoffset, int yoffset,
                        int width, int height){

    {
        }else{
            dst[j*dst_stride + i] = 0;
        }
    }

static void cmv_decode_inter(CmvContext *s, AVFrame *frame, const uint8_t *buf,
                             const uint8_t *buf_end)
{
    const uint8_t *raw = buf + (s->avctx->width*s->avctx->height/16);
    int x,y,i;

    i = 0;
    for(y=0; y<s->avctx->height/4; y++)
    for(x=0; x<s->avctx->width/4 && buf_end - buf > i; x++) {
        if (buf[i]==0xFF) {
            unsigned char *dst = frame->data[0] + (y*4)*frame->linesize[0] + x*4;
            if (raw+16<buf_end && *raw==0xFF) { /* intra */
                raw++;
                memcpy(dst, raw, 4);
                memcpy(dst +     frame->linesize[0], raw+4, 4);
                memcpy(dst + 2 * frame->linesize[0], raw+8, 4);
                memcpy(dst + 3 * frame->linesize[0], raw+12, 4);
                raw+=16;
            }else if(raw<buf_end) {  /* inter using second-last frame as reference */
                int xoffset = (*raw & 0xF) - 7;
                int yoffset = ((*raw >> 4)) - 7;
                if (s->last2_frame->data[0])
                    cmv_motcomp(frame->data[0], frame->linesize[0],
                                s->last2_frame->data[0], s->last2_frame->linesize[0],
                                x*4, y*4, xoffset, yoffset, s->avctx->width, s->avctx->height);
                raw++;
            }
        }else{  /* inter using last frame as reference */
            int xoffset = (buf[i] & 0xF) - 7;
            int yoffset = ((buf[i] >> 4)) - 7;
            if (s->last_frame->data[0])
                cmv_motcomp(frame->data[0], frame->linesize[0],
                            s->last_frame->data[0], s->last_frame->linesize[0],
                            x*4, y*4, xoffset, yoffset, s->avctx->width, s->avctx->height);
        }
        i++;
    }
}

{

        av_log(s->avctx, AV_LOG_WARNING, "truncated header\n");
        return AVERROR_INVALIDDATA;
    }


    }

        return ret;



    }

    return 0;
}

#define EA_PREAMBLE_SIZE 8
#define MVIh_TAG MKTAG('M', 'V', 'I', 'h')

                            void *data, int *got_frame,
                            AVPacket *avpkt)
{

        return AVERROR_INVALIDDATA;

            return ret;
            return AVERROR_INVALIDDATA;
    }

        return ret;

        return ret;


    }else{
    }

        return ret;


}



}

AVCodec ff_eacmv_decoder = {
    .name           = "eacmv",
    .long_name      = NULL_IF_CONFIG_SMALL("Electronic Arts CMV video"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_CMV,
    .priv_data_size = sizeof(CmvContext),
    .init           = cmv_decode_init,
    .close          = cmv_decode_end,
    .decode         = cmv_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
