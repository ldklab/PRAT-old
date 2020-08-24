/*
 * Electronic Arts Madcow Video Decoder
 * Copyright (c) 2007-2009 Peter Ross
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
 * Electronic Arts Madcow Video Decoder
 * @author Peter Ross <pross@xvid.org>
 *
 * @see technical details at
 * http://wiki.multimedia.cx/index.php?title=Electronic_Arts_MAD
 */

#include "avcodec.h"
#include "blockdsp.h"
#include "bytestream.h"
#include "bswapdsp.h"
#include "get_bits.h"
#include "aandcttab.h"
#include "eaidct.h"
#include "idctdsp.h"
#include "internal.h"
#include "mpeg12data.h"
#include "mpeg12vlc.h"

#define EA_PREAMBLE_SIZE    8
#define MADk_TAG MKTAG('M', 'A', 'D', 'k')    /* MAD I-frame */
#define MADm_TAG MKTAG('M', 'A', 'D', 'm')    /* MAD P-frame */
#define MADe_TAG MKTAG('M', 'A', 'D', 'e')    /* MAD lqp-frame */

typedef struct MadContext {
    AVCodecContext *avctx;
    BlockDSPContext bdsp;
    BswapDSPContext bbdsp;
    IDCTDSPContext idsp;
    AVFrame *last_frame;
    GetBitContext gb;
    void *bitstream_buf;
    unsigned int bitstream_buf_size;
    DECLARE_ALIGNED(32, int16_t, block)[64];
    ScanTable scantable;
    uint16_t quant_matrix[64];
    int mb_x;
    int mb_y;
} MadContext;

{

        return AVERROR(ENOMEM);

    return 0;
}

static inline void comp(unsigned char *dst, ptrdiff_t dst_stride,
                        unsigned char *src, ptrdiff_t src_stride, int add)
{
    int j, i;
    for (j=0; j<8; j++)
        for (i=0; i<8; i++)
            dst[j*dst_stride + i] = av_clip_uint8(src[j*src_stride + i] + add);
}

static inline void comp_block(MadContext *t, AVFrame *frame,
                              int mb_x, int mb_y,
                              int j, int mv_x, int mv_y, int add)
{
    if (j < 4) {
        unsigned offset = (mb_y*16 + ((j&2)<<2) + mv_y)*t->last_frame->linesize[0] + mb_x*16 + ((j&1)<<3) + mv_x;
        if (offset >= (t->avctx->height - 7) * t->last_frame->linesize[0] - 7)
            return;
        comp(frame->data[0] + (mb_y*16 + ((j&2)<<2))*frame->linesize[0] + mb_x*16 + ((j&1)<<3),
             frame->linesize[0],
             t->last_frame->data[0] + offset,
             t->last_frame->linesize[0], add);
    } else if (!(t->avctx->flags & AV_CODEC_FLAG_GRAY)) {
        int index = j - 3;
        unsigned offset = (mb_y * 8 + (mv_y/2))*t->last_frame->linesize[index] + mb_x * 8 + (mv_x/2);
        if (offset >= (t->avctx->height/2 - 7) * t->last_frame->linesize[index] - 7)
            return;
        comp(frame->data[index] + (mb_y*8)*frame->linesize[index] + mb_x * 8,
             frame->linesize[index],
             t->last_frame->data[index] + offset,
             t->last_frame->linesize[index], add);
    }
}

static inline void idct_put(MadContext *t, AVFrame *frame, int16_t *block,
                            int mb_x, int mb_y, int j)
{
    if (j < 4) {
        ff_ea_idct_put_c(
            frame->data[0] + (mb_y*16 + ((j&2)<<2))*frame->linesize[0] + mb_x*16 + ((j&1)<<3),
            frame->linesize[0], block);
    } else if (!(t->avctx->flags & AV_CODEC_FLAG_GRAY)) {
        int index = j - 3;
        ff_ea_idct_put_c(
            frame->data[index] + (mb_y*8)*frame->linesize[index] + mb_x*8,
            frame->linesize[index], block);
    }
}

{


    /* The RL decoder is derived from mpeg1_decode_block_intra;
       Escaped level and run values a decoded differently */
    {
        /* now quantify & encode AC coefficients */

                break;
                    av_log(s->avctx, AV_LOG_ERROR,
                           "ac-tex damaged at %d %d\n", s->mb_x, s->mb_y);
                    return -1;
                }
            } else {
                /* escape */


                    av_log(s->avctx, AV_LOG_ERROR,
                           "ac-tex damaged at %d %d\n", s->mb_x, s->mb_y);
                    return -1;
                }
                } else {
                }
            }

        }
    }
}

{
    }
}

{

        }
    }

        } else {
                return -1;
        }
    }
    return 0;
}

{

}

                        void *data, int *got_frame,
                        AVPacket *avpkt)
{





        av_log(avctx, AV_LOG_ERROR, "Input data too small\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "Dimensions too small\n");
        return AVERROR_INVALIDDATA;
    }

            return AVERROR_INVALIDDATA;
            return ret;
    }

        return ret;

        av_log(avctx, AV_LOG_WARNING, "Missing reference frame.\n");
        ret = ff_get_buffer(avctx, s->last_frame, AV_GET_BUFFER_FLAG_REF);
        if (ret < 0)
            return ret;
        memset(s->last_frame->data[0], 0, s->last_frame->height *
               s->last_frame->linesize[0]);
        memset(s->last_frame->data[1], 0x80, s->last_frame->height / 2 *
               s->last_frame->linesize[1]);
               s->last_frame->linesize[2]);
    }

        return AVERROR(ENOMEM);

                return AVERROR_INVALIDDATA;


            return ret;
    }

    return buf_size;
}

{
}

AVCodec ff_eamad_decoder = {
    .name           = "eamad",
    .long_name      = NULL_IF_CONFIG_SMALL("Electronic Arts Madcow Video"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_MAD,
    .priv_data_size = sizeof(MadContext),
    .init           = decode_init,
    .close          = decode_end,
    .decode         = decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
