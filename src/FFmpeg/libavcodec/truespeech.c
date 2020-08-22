/*
 * DSP Group TrueSpeech compatible decoder
 * Copyright (c) 2005 Konstantin Shishkov
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

#include "libavutil/channel_layout.h"
#include "libavutil/intreadwrite.h"
#include "avcodec.h"
#include "bswapdsp.h"
#include "get_bits.h"
#include "internal.h"

#include "truespeech_data.h"
/**
 * @file
 * TrueSpeech decoder.
 */

/**
 * TrueSpeech decoder context
 */
typedef struct TSContext {
    BswapDSPContext bdsp;
    /* input data */
    DECLARE_ALIGNED(16, uint8_t, buffer)[32];
    int16_t vector[8];  ///< input vector: 5/5/4/4/4/3/3/3
    int offset1[2];     ///< 8-bit value, used in one copying offset
    int offset2[4];     ///< 7-bit value, encodes offsets for copying and for two-point filter
    int pulseoff[4];    ///< 4-bit offset of pulse values block
    int pulsepos[4];    ///< 27-bit variable, encodes 7 pulse positions
    int pulseval[4];    ///< 7x2-bit pulse values
    int flag;           ///< 1-bit flag, shows how to choose filters
    /* temporary data */
    int filtbuf[146];   // some big vector used for storing filters
    int prevfilt[8];    // filter from previous frame
    int16_t tmp1[8];    // coefficients for adding to out
    int16_t tmp2[8];    // coefficients for adding to out
    int16_t tmp3[8];    // coefficients for adding to out
    int16_t cvector[8]; // correlated input vector
    int filtval;        // gain value for one function
    int16_t newvec[60]; // tmp vector
    int16_t filters[32]; // filters for every subframe
} TSContext;

{

        avpriv_request_sample(avctx, "Channel count %d", avctx->channels);
        return AVERROR_PATCHWELCOME;
    }



}

{










{

        }
    }


{

        }
    }else{
        }
    }
    }

{

    }
    }
}

{

    }

        else{
        }
    }
        else{
        }
    }


static void truespeech_update_filters(TSContext *dec, int16_t *out, int quart)
{
    int i;

    memmove(dec->filtbuf, &dec->filtbuf[60], 86 * sizeof(*dec->filtbuf));
    for(i = 0; i < 60; i++){
        dec->filtbuf[i + 86] = out[i] + dec->newvec[i] - (dec->newvec[i] >> 3);
        out[i] += dec->newvec[i];
    }
}

{

        int sum = 0;
    }


        int sum = 0;
    }



    }

static void truespeech_save_prevvec(TSContext *c)
{
    int i;

}

                                   int *got_frame_ptr, AVPacket *avpkt)
{



        av_log(avctx, AV_LOG_ERROR,
               "Too small input buffer (%d bytes), need at least 32 bytes\n", buf_size);
        return -1;
    }

    /* get output buffer */
        return ret;




        }

    }


}

AVCodec ff_truespeech_decoder = {
    .name           = "truespeech",
    .long_name      = NULL_IF_CONFIG_SMALL("DSP Group TrueSpeech"),
    .type           = AVMEDIA_TYPE_AUDIO,
    .id             = AV_CODEC_ID_TRUESPEECH,
    .priv_data_size = sizeof(TSContext),
    .init           = truespeech_decode_init,
    .decode         = truespeech_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
