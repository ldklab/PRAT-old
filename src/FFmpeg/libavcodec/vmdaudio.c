/*
 * Sierra VMD audio decoder
 * Copyright (c) 2004 The FFmpeg Project
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
 * Sierra VMD audio decoder
 * by Vladimir "VAG" Gneushev (vagsoft at mail.ru)
 * for more information on the Sierra VMD format, visit:
 *   http://www.pcisys.net/~melanson/codecs/
 *
 * The audio decoder, expects each encoded data
 * chunk to be prepended with the appropriate 16-byte frame information
 * record from the VMD file. It does not require the 0x330-byte VMD file
 * header, but it does need the audio setup parameters passed in through
 * normal libavcodec API means.
 */

#include <string.h>

#include "libavutil/avassert.h"
#include "libavutil/channel_layout.h"
#include "libavutil/common.h"
#include "libavutil/intreadwrite.h"

#include "avcodec.h"
#include "internal.h"

#define BLOCK_TYPE_AUDIO    1
#define BLOCK_TYPE_INITIAL  2
#define BLOCK_TYPE_SILENCE  3

typedef struct VmdAudioContext {
    int out_bps;
    int chunk_size;
} VmdAudioContext;

static const uint16_t vmdaudio_table[128] = {
    0x000, 0x008, 0x010, 0x020, 0x030, 0x040, 0x050, 0x060, 0x070, 0x080,
    0x090, 0x0A0, 0x0B0, 0x0C0, 0x0D0, 0x0E0, 0x0F0, 0x100, 0x110, 0x120,
    0x130, 0x140, 0x150, 0x160, 0x170, 0x180, 0x190, 0x1A0, 0x1B0, 0x1C0,
    0x1D0, 0x1E0, 0x1F0, 0x200, 0x208, 0x210, 0x218, 0x220, 0x228, 0x230,
    0x238, 0x240, 0x248, 0x250, 0x258, 0x260, 0x268, 0x270, 0x278, 0x280,
    0x288, 0x290, 0x298, 0x2A0, 0x2A8, 0x2B0, 0x2B8, 0x2C0, 0x2C8, 0x2D0,
    0x2D8, 0x2E0, 0x2E8, 0x2F0, 0x2F8, 0x300, 0x308, 0x310, 0x318, 0x320,
    0x328, 0x330, 0x338, 0x340, 0x348, 0x350, 0x358, 0x360, 0x368, 0x370,
    0x378, 0x380, 0x388, 0x390, 0x398, 0x3A0, 0x3A8, 0x3B0, 0x3B8, 0x3C0,
    0x3C8, 0x3D0, 0x3D8, 0x3E0, 0x3E8, 0x3F0, 0x3F8, 0x400, 0x440, 0x480,
    0x4C0, 0x500, 0x540, 0x580, 0x5C0, 0x600, 0x640, 0x680, 0x6C0, 0x700,
    0x740, 0x780, 0x7C0, 0x800, 0x900, 0xA00, 0xB00, 0xC00, 0xD00, 0xE00,
    0xF00, 0x1000, 0x1400, 0x1800, 0x1C00, 0x2000, 0x3000, 0x4000
};

{

        av_log(avctx, AV_LOG_ERROR, "invalid number of channels\n");
        return AVERROR(EINVAL);
    }
    ) {
        av_log(avctx, AV_LOG_ERROR, "invalid block align\n");
        return AVERROR(EINVAL);
    }

                                                   AV_CH_LAYOUT_STEREO;

        avctx->sample_fmt = AV_SAMPLE_FMT_S16;
    else


           "block align = %d, sample rate = %d\n",
           avctx->channels, avctx->bits_per_coded_sample, avctx->block_align,
           avctx->sample_rate);

}

static void decode_audio_s16(int16_t *out, const uint8_t *buf, int buf_size,
                             int channels)
{
    int ch;
    const uint8_t *buf_end = buf + buf_size;
    int predictor[2];
    int st = channels - 1;

    /* decode initial raw sample */
    for (ch = 0; ch < channels; ch++) {
        predictor[ch] = (int16_t)AV_RL16(buf);
        buf += 2;
        *out++ = predictor[ch];
    }

    /* decode DPCM samples */
    ch = 0;
    while (buf < buf_end) {
        uint8_t b = *buf++;
        if (b & 0x80)
            predictor[ch] -= vmdaudio_table[b & 0x7F];
        else
            predictor[ch] += vmdaudio_table[b];
        predictor[ch] = av_clip_int16(predictor[ch]);
        *out++ = predictor[ch];
        ch ^= st;
    }
}

                                 int *got_frame_ptr, AVPacket *avpkt)
{

        av_log(avctx, AV_LOG_WARNING, "skipping small junk packet\n");
        *got_frame_ptr = 0;
        return buf_size;
    }

        av_log(avctx, AV_LOG_ERROR, "unknown block type: %d\n", block_type);
        return AVERROR(EINVAL);
    }

    /* get number of silent chunks */
            av_log(avctx, AV_LOG_ERROR, "packet is too small\n");
            return AVERROR(EINVAL);
        }
    }

    /* ensure output buffer is large enough */

    /* drop incomplete chunks */

        return AVERROR_INVALIDDATA;

    /* get output buffer */
        return ret;

    /* decode silent chunks */

            memset(output_samples_s16, 0x00, silent_size * 2);
            output_samples_s16 += silent_size;
        } else {
        }
    }

    /* decode audio chunks */
                decode_audio_s16(output_samples_s16, buf, s->chunk_size,
                                 avctx->channels);
                output_samples_s16 += avctx->block_align;
            } else {
            }
        }
    }


}

AVCodec ff_vmdaudio_decoder = {
    .name           = "vmdaudio",
    .long_name      = NULL_IF_CONFIG_SMALL("Sierra VMD audio"),
    .type           = AVMEDIA_TYPE_AUDIO,
    .id             = AV_CODEC_ID_VMDAUDIO,
    .priv_data_size = sizeof(VmdAudioContext),
    .init           = vmdaudio_decode_init,
    .decode         = vmdaudio_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
