/*
 * ZeroCodec Decoder
 *
 * Copyright (c) 2012, Derek Buitenhuis
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <zlib.h>

#include "avcodec.h"
#include "internal.h"
#include "libavutil/common.h"

typedef struct ZeroCodecContext {
    AVFrame  *previous_frame;
    z_stream zstream;
} ZeroCodecContext;

                                  int *got_frame, AVPacket *avpkt)
{

    } else {
            av_log(avctx, AV_LOG_ERROR, "Missing reference frame.\n");
            return AVERROR_INVALIDDATA;
        }


    }

        av_log(avctx, AV_LOG_ERROR, "Could not reset inflate: %d.\n", zret);
        return AVERROR_INVALIDDATA;
    }

        return ret;



    /**
     * ZeroCodec has very simple interframe compression. If a value
     * is the same as the previous frame, set it to 0.
     */


            av_log(avctx, AV_LOG_ERROR,
                   "Inflate failed with return code: %d.\n", zret);
            return AVERROR_INVALIDDATA;
        }


    }

        return ret;


}

{



}

{



        av_log(avctx, AV_LOG_ERROR, "Could not initialize inflate: %d.\n", zret);
        return AVERROR(ENOMEM);
    }

        return AVERROR(ENOMEM);

    return 0;
}

static void zerocodec_decode_flush(AVCodecContext *avctx)
{
    ZeroCodecContext *zc = avctx->priv_data;

    av_frame_unref(zc->previous_frame);
}

AVCodec ff_zerocodec_decoder = {
    .type           = AVMEDIA_TYPE_VIDEO,
    .name           = "zerocodec",
    .long_name      = NULL_IF_CONFIG_SMALL("ZeroCodec Lossless Video"),
    .id             = AV_CODEC_ID_ZEROCODEC,
    .priv_data_size = sizeof(ZeroCodecContext),
    .init           = zerocodec_decode_init,
    .decode         = zerocodec_decode_frame,
    .flush          = zerocodec_decode_flush,
    .close          = zerocodec_decode_close,
    .capabilities   = AV_CODEC_CAP_DR1,
    .caps_internal  = FF_CODEC_CAP_INIT_THREADSAFE |
                      FF_CODEC_CAP_INIT_CLEANUP,
};
