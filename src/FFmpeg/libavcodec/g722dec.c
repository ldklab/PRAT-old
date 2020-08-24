/*
 * Copyright (c) CMU 1993 Computer Science, Speech Group
 *                        Chengxiang Lu and Alex Hauptmann
 * Copyright (c) 2005 Steve Underwood <steveu at coppice.org>
 * Copyright (c) 2009 Kenan Gillet
 * Copyright (c) 2010 Martin Storsjo
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
 * G.722 ADPCM audio decoder
 *
 * This G.722 decoder is a bit-exact implementation of the ITU G.722
 * specification for all three specified bitrates - 64000bps, 56000bps
 * and 48000bps. It passes the ITU tests.
 *
 * @note For the 56000bps and 48000bps bitrates, the lowest 1 or 2 bits
 *       respectively of each byte are ignored.
 */

#include "libavutil/channel_layout.h"
#include "libavutil/opt.h"
#include "avcodec.h"
#include "get_bits.h"
#include "g722.h"
#include "internal.h"

#define OFFSET(x) offsetof(G722Context, x)
#define AD AV_OPT_FLAG_AUDIO_PARAM | AV_OPT_FLAG_DECODING_PARAM
static const AVOption options[] = {
    { "bits_per_codeword", "Bits per G722 codeword", OFFSET(bits_per_codeword), AV_OPT_TYPE_INT, { .i64 = 8 }, 6, 8, AD },
    { NULL }
};

static const AVClass g722_decoder_class = {
    .class_name = "g722 decoder",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
};

{




}

static const int16_t low_inv_quant5[32] = {
     -35,   -35, -2919, -2195, -1765, -1458, -1219, -1023,
    -858,  -714,  -587,  -473,  -370,  -276,  -190,  -110,
    2919,  2195,  1765,  1458,  1219,  1023,   858,   714,
     587,   473,   370,   276,   190,   110,    35,   -35
};

static const int16_t * const low_inv_quants[3] = { ff_g722_low_inv_quant6,
                                                           low_inv_quant5,
                                                   ff_g722_low_inv_quant4 };

                             int *got_frame_ptr, AVPacket *avpkt)
{

    /* get output buffer */
        return ret;

        return ret;







                    22 * sizeof(c->prev_samples[0]));
        }
    }


}

AVCodec ff_adpcm_g722_decoder = {
    .name           = "g722",
    .long_name      = NULL_IF_CONFIG_SMALL("G.722 ADPCM"),
    .type           = AVMEDIA_TYPE_AUDIO,
    .id             = AV_CODEC_ID_ADPCM_G722,
    .priv_data_size = sizeof(G722Context),
    .init           = g722_decode_init,
    .decode         = g722_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
    .priv_class     = &g722_decoder_class,
};
