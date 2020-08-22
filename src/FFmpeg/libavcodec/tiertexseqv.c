/*
 * Tiertex Limited SEQ Video Decoder
 * Copyright (c) 2006 Gregory Montoir (cyx@users.sourceforge.net)
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
 * Tiertex Limited SEQ video decoder
 */

#define BITSTREAM_READER_LE
#include "avcodec.h"
#include "get_bits.h"
#include "internal.h"


typedef struct SeqVideoContext {
    AVCodecContext *avctx;
    AVFrame *frame;
} SeqVideoContext;


                                                 const unsigned char *src_end,
                                                 unsigned char *dst, int dst_size)
{

    /* get the rle codes */
            return NULL;
    }

    /* do the rle unpacking */
                return NULL;
        } else {
            if (src_end - src < len)
                return NULL;
            memcpy(dst, src, FFMIN(len, dst_size));
            src += len;
        }
    }
    return src;
}

static const unsigned char *seq_decode_op1(SeqVideoContext *seq,
                                           const unsigned char *src,
                                           const unsigned char *src_end,
                                           unsigned char *dst)
{
    const unsigned char *color_table;
    int b, i, len, bits;
    GetBitContext gb;
    unsigned char block[8 * 8];

    if (src_end - src < 1)
        return NULL;
    len = *src++;
    if (len & 0x80) {
        switch (len & 3) {
        case 1:
            src = seq_unpack_rle_block(src, src_end, block, sizeof(block));
            for (b = 0; b < 8; b++) {
                memcpy(dst, &block[b * 8], 8);
                dst += seq->frame->linesize[0];
            }
            break;
        case 2:
            src = seq_unpack_rle_block(src, src_end, block, sizeof(block));
            for (i = 0; i < 8; i++) {
                for (b = 0; b < 8; b++)
                    dst[b * seq->frame->linesize[0]] = block[i * 8 + b];
                ++dst;
            }
            break;
        }
    } else {
        if (len <= 0)
            return NULL;
        bits = ff_log2_tab[len - 1] + 1;
        if (src_end - src < len + 8 * bits)
            return NULL;
        color_table = src;
        src += len;
        init_get_bits(&gb, src, bits * 8 * 8); src += bits * 8;
        for (b = 0; b < 8; b++) {
            for (i = 0; i < 8; i++)
                dst[i] = color_table[get_bits(&gb, bits)];
            dst += seq->frame->linesize[0];
        }
    }

    return src;
}

static const unsigned char *seq_decode_op2(SeqVideoContext *seq,
                                           const unsigned char *src,
                                           const unsigned char *src_end,
                                           unsigned char *dst)
{
    int i;

    if (src_end - src < 8 * 8)
        return NULL;

    for (i = 0; i < 8; i++) {
        memcpy(dst, src, 8);
        src += 8;
        dst += seq->frame->linesize[0];
    }

    return src;
}

static const unsigned char *seq_decode_op3(SeqVideoContext *seq,
                                           const unsigned char *src,
                                           const unsigned char *src_end,
                                           unsigned char *dst)
{

            return NULL;

    return src;
}

{


            return AVERROR_INVALIDDATA;
        }
    }

            return AVERROR_INVALIDDATA;
                case 3:
                    break;
                }
                    return AVERROR_INVALIDDATA;
            }
    }
    return 0;
}

{


        return ret;

        return AVERROR(ENOMEM);

    return 0;
}

                                 void *data, int *got_frame,
                                 AVPacket *avpkt)
{


        return ret;

        return AVERROR_INVALIDDATA;

        return ret;

}

{


}

AVCodec ff_tiertexseqvideo_decoder = {
    .name           = "tiertexseqvideo",
    .long_name      = NULL_IF_CONFIG_SMALL("Tiertex Limited SEQ video"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_TIERTEXSEQVIDEO,
    .priv_data_size = sizeof(SeqVideoContext),
    .init           = seqvideo_decode_init,
    .close          = seqvideo_decode_end,
    .decode         = seqvideo_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
