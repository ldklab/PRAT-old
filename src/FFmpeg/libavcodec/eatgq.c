/*
 * Electronic Arts TGQ Video Decoder
 * Copyright (c) 2007-2008 Peter Ross <pross@xvid.org>
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
 * Electronic Arts TGQ Video Decoder
 * @author Peter Ross <pross@xvid.org>
 *
 * Technical details here:
 * http://wiki.multimedia.cx/index.php?title=Electronic_Arts_TGQ
 */

#define BITSTREAM_READER_LE
#include "aandcttab.h"
#include "avcodec.h"
#include "bytestream.h"
#include "eaidct.h"
#include "get_bits.h"
#include "idctdsp.h"
#include "internal.h"

typedef struct TgqContext {
    AVCodecContext *avctx;
    int width, height;
    ScanTable scantable;
    int qtable[64];
    DECLARE_ALIGNED(16, int16_t, block)[6][64];
    GetByteContext gb;
} TgqContext;

{
}

{
            break;
        case 1:
            break;
        case 3: // 011b
            } else {
            }
        }
    }

static void tgq_idct_put_mb(TgqContext *s, int16_t (*block)[64], AVFrame *frame,
                            int mb_x, int mb_y)
{
    ptrdiff_t linesize = frame->linesize[0];
    uint8_t *dest_y  = frame->data[0] + (mb_y * 16 * linesize)           + mb_x * 16;
    uint8_t *dest_cb = frame->data[1] + (mb_y * 8  * frame->linesize[1]) + mb_x * 8;
    uint8_t *dest_cr = frame->data[2] + (mb_y * 8  * frame->linesize[2]) + mb_x * 8;

    ff_ea_idct_put_c(dest_y                   , linesize, block[0]);
    ff_ea_idct_put_c(dest_y                + 8, linesize, block[1]);
    ff_ea_idct_put_c(dest_y + 8 * linesize    , linesize, block[2]);
    ff_ea_idct_put_c(dest_y + 8 * linesize + 8, linesize, block[3]);
    if (!(s->avctx->flags & AV_CODEC_FLAG_GRAY)) {
         ff_ea_idct_put_c(dest_cb, frame->linesize[1], block[4]);
         ff_ea_idct_put_c(dest_cr, frame->linesize[2], block[5]);
    }
}

static inline void tgq_dconly(TgqContext *s, unsigned char *dst,
                              ptrdiff_t dst_stride, int dc)
{
    int level = av_clip_uint8((dc*s->qtable[0] + 2056) >> 4);
    int j;
    for (j = 0; j < 8; j++)
        memset(dst + j * dst_stride, level, 8);
}

                                   int mb_x, int mb_y, const int8_t *dc)
{
    }

{

            return ret;

    } else {
            memset(dc, bytestream2_get_byte(&s->gb), 4);
            dc[4] = bytestream2_get_byte(&s->gb);
            dc[5] = bytestream2_get_byte(&s->gb);
            bytestream2_get_buffer(&s->gb, dc, 6);
            }
        } else {
            av_log(s->avctx, AV_LOG_ERROR, "unsupported mb mode %i\n", mode);
            return -1;
        }
    }
    return 0;
}

{

                            void *data, int *got_frame,
                            AVPacket *avpkt)
{

        av_log(avctx, AV_LOG_WARNING, "truncated header\n");
        return AVERROR_INVALIDDATA;
    }
    } else {
        s->width  = bytestream2_get_le16u(&s->gb);
        s->height = bytestream2_get_le16u(&s->gb);
    }

        return ret;


        return ret;

                return AVERROR_INVALIDDATA;


}

AVCodec ff_eatgq_decoder = {
    .name           = "eatgq",
    .long_name      = NULL_IF_CONFIG_SMALL("Electronic Arts TGQ video"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_TGQ,
    .priv_data_size = sizeof(TgqContext),
    .init           = tgq_decode_init,
    .decode         = tgq_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
