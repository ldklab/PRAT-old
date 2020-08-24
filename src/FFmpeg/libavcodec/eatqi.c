/*
 * Electronic Arts TQI Video Decoder
 * Copyright (c) 2007-2009 Peter Ross <pross@xvid.org>
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
 * Electronic Arts TQI Video Decoder
 * @author Peter Ross <pross@xvid.org>
 * @see http://wiki.multimedia.cx/index.php?title=Electronic_Arts_TQI
 */

#include "avcodec.h"
#include "blockdsp.h"
#include "bswapdsp.h"
#include "get_bits.h"
#include "aandcttab.h"
#include "eaidct.h"
#include "idctdsp.h"
#include "internal.h"
#include "mpeg12.h"

typedef struct TqiContext {
    AVCodecContext *avctx;
    GetBitContext gb;
    BlockDSPContext bdsp;
    BswapDSPContext bsdsp;
    IDCTDSPContext idsp;
    ScanTable intra_scantable;

    void *bitstream_buf;
    unsigned int bitstream_buf_size;

    int mb_x, mb_y;
    uint16_t intra_matrix[64];
    int last_dc[3];

    DECLARE_ALIGNED(32, int16_t, block)[6][64];
} TqiContext;

{


}

{

            av_log(t->avctx, AV_LOG_ERROR, "ac-tex damaged at %d %d\n",
                   t->mb_x, t->mb_y);
            return ret;
        }
    }

    return 0;
}

static inline void tqi_idct_put(AVCodecContext *avctx, AVFrame *frame,
                                int16_t (*block)[64])
{
    TqiContext *t = avctx->priv_data;
    ptrdiff_t linesize = frame->linesize[0];
    uint8_t *dest_y  = frame->data[0] + t->mb_y * 16 * linesize           + t->mb_x * 16;
    uint8_t *dest_cb = frame->data[1] + t->mb_y *  8 * frame->linesize[1] + t->mb_x *  8;
    uint8_t *dest_cr = frame->data[2] + t->mb_y *  8 * frame->linesize[2] + t->mb_x *  8;

    ff_ea_idct_put_c(dest_y                 , linesize, block[0]);
    ff_ea_idct_put_c(dest_y              + 8, linesize, block[1]);
    ff_ea_idct_put_c(dest_y + 8*linesize    , linesize, block[2]);
    ff_ea_idct_put_c(dest_y + 8*linesize + 8, linesize, block[3]);

    if (!(avctx->flags & AV_CODEC_FLAG_GRAY)) {
        ff_ea_idct_put_c(dest_cb, frame->linesize[1], block[4]);
        ff_ea_idct_put_c(dest_cr, frame->linesize[2], block[5]);
    }
}

{

}

                            void *data, int *got_frame,
                            AVPacket *avpkt)
{

        return AVERROR_INVALIDDATA;



        return ret;

        return ret;

        return AVERROR(ENOMEM);

                goto end;
        }
    }

}

{
}

AVCodec ff_eatqi_decoder = {
    .name           = "eatqi",
    .long_name      = NULL_IF_CONFIG_SMALL("Electronic Arts TQI Video"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_TQI,
    .priv_data_size = sizeof(TqiContext),
    .init           = tqi_decode_init,
    .close          = tqi_decode_end,
    .decode         = tqi_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
