/*
 * NellyMoser audio decoder
 * Copyright (c) 2007 a840bda5870ba11f19698ff6eb9581dfb0f95fa5,
 *                    539459aeb7d425140b62a3ec7dbf6dc8e408a306, and
 *                    520e17cd55896441042b14df2566a6eb610ed444
 * Copyright (c) 2007 Loic Minier <lool at dooz.org>
 *                    Benjamin Larsson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * @file
 * The 3 alphanumeric copyright notices are md5summed they are from the original
 * implementors. The original code is available from http://code.google.com/p/nelly2pcm/
 */

#include "libavutil/channel_layout.h"
#include "libavutil/float_dsp.h"
#include "libavutil/lfg.h"
#include "libavutil/random_seed.h"

#define BITSTREAM_READER_LE
#include "avcodec.h"
#include "fft.h"
#include "get_bits.h"
#include "internal.h"
#include "nellymoser.h"
#include "sinewin.h"


typedef struct NellyMoserDecodeContext {
    AVCodecContext* avctx;
    AVLFG           random_state;
    GetBitContext   gb;
    float           scale_bias;
    AVFloatDSPContext *fdsp;
    FFTContext      imdct_ctx;
    DECLARE_ALIGNED(32, float, imdct_buf)[2][NELLY_BUF_LEN];
    float          *imdct_out;
    float          *imdct_prev;
} NellyMoserDecodeContext;

                               const unsigned char block[NELLY_BLOCK_LEN],
                               float audio[NELLY_SAMPLES])
{


        }

    }




            } else {
            }
        }
               (NELLY_BUF_LEN - NELLY_FILL_LEN) * sizeof(float));

                                   NELLY_BUF_LEN / 2);
    }



        return AVERROR(ENOMEM);


    /* Generate overlap window */


}

                      int *got_frame_ptr, AVPacket *avpkt)
{


        av_log(avctx, AV_LOG_ERROR, "Packet is too small\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_WARNING, "Leftover bytes: %d.\n",
               buf_size % NELLY_BLOCK_LEN);
    }

    /* get output buffer */
        return ret;

    }


}



}

AVCodec ff_nellymoser_decoder = {
    .name           = "nellymoser",
    .long_name      = NULL_IF_CONFIG_SMALL("Nellymoser Asao"),
    .type           = AVMEDIA_TYPE_AUDIO,
    .id             = AV_CODEC_ID_NELLYMOSER,
    .priv_data_size = sizeof(NellyMoserDecodeContext),
    .init           = decode_init,
    .close          = decode_end,
    .decode         = decode_tag,
    .capabilities   = AV_CODEC_CAP_DR1 | AV_CODEC_CAP_PARAM_CHANGE,
    .sample_fmts    = (const enum AVSampleFormat[]) { AV_SAMPLE_FMT_FLT,
                                                      AV_SAMPLE_FMT_NONE },
};
