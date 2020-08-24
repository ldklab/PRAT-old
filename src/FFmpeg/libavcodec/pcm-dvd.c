/*
 * LPCM codecs for PCM formats found in Video DVD streams
 * Copyright (c) 2013 Christian Schmidt
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
 * LPCM codecs for PCM formats found in Video DVD streams
 */

#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"

typedef struct PCMDVDContext {
    uint32_t last_header;    // Cached header to see if parsing is needed
    int block_size;          // Size of a block of samples in bytes
    int last_block_size;     // Size of the last block of samples in bytes
    int samples_per_block;   // Number of samples per channel per block
    int groups_per_block;    // Number of 20/24-bit sample groups per block
    uint8_t *extra_samples;  // Pointer to leftover samples from a frame
    int extra_sample_count;  // Number of leftover samples in the buffer
} PCMDVDContext;

{

    /* Invalid header to force parsing of the first header */
    /* reserve space for 8 channels, 3 bytes/sample, 4 samples/block */
        return AVERROR(ENOMEM);

    return 0;
}

{


}

{
    /* no traces of 44100 and 32000Hz in any commercial software or player */

    /* early exit if the header didn't change apart from the frame number */
        return 0;

        av_log(avctx, AV_LOG_DEBUG, "pcm_dvd_parse_header: header = %02x%02x%02x\n",
                header[0], header[1], header[2]);
    /*
     * header[0] emphasis (1), muse(1), reserved(1), frame number(5)
     * header[1] quant (2), freq(2), reserved(1), channels(3)
     * header[2] dynamic range control (0x80 = off)
     */

    /* Discard potentially existing leftover samples from old channel layout */

    /* get the sample depth and derive the sample format from it */
        av_log(avctx, AV_LOG_ERROR,
               "PCM DVD unsupported sample depth %i\n",
               avctx->bits_per_coded_sample);
        return AVERROR_INVALIDDATA;
    }

    /* get the sample rate */

    /* get the number of channels */
    /* calculate the bitrate */
                      avctx->bits_per_coded_sample;

    /* 4 samples form a group in 20/24-bit PCM on DVD Video.
     * A block is formed by the number of groups that are
     * needed to complete a set of samples for each channel. */
    } else {
        case 2:
        case 4:
            /* one group has all the samples needed */
        case 8:
            /* two groups have all the samples needed */
            s->block_size        = 8 * avctx->bits_per_coded_sample / 8;
            s->samples_per_block = 1;
            s->groups_per_block  = 2;
            break;
        default:
            /* need avctx->channels groups */
            s->block_size        = 4 * avctx->channels *
                                   avctx->bits_per_coded_sample / 8;
            s->samples_per_block = 4;
            s->groups_per_block  = avctx->channels;
            break;
        }
    }

                "pcm_dvd_parse_header: %d channels, %d bits per sample, %d Hz, %"PRId64" bit/s\n",
                avctx->channels, avctx->bits_per_coded_sample,
                avctx->sample_rate, avctx->bit_rate);


}

                                    void *dst, int blocks)
{

#if HAVE_BIGENDIAN
        bytestream2_get_buffer(&gb, dst16, blocks * s->block_size);
        dst16 += blocks * s->block_size / 2;
#else
#endif
        return dst16;
    }
    case 20:
        if (avctx->channels == 1) {
            do {
                for (i = 2; i; i--) {
                    dst32[0] = bytestream2_get_be16u(&gb) << 16;
                    dst32[1] = bytestream2_get_be16u(&gb) << 16;
                    t = bytestream2_get_byteu(&gb);
                    *dst32++ += (t & 0xf0) << 8;
                    *dst32++ += (t & 0x0f) << 12;
                }
            } while (--blocks);
        } else {
        do {
            for (i = s->groups_per_block; i; i--) {
                dst32[0] = bytestream2_get_be16u(&gb) << 16;
                dst32[1] = bytestream2_get_be16u(&gb) << 16;
                dst32[2] = bytestream2_get_be16u(&gb) << 16;
                dst32[3] = bytestream2_get_be16u(&gb) << 16;
                t = bytestream2_get_byteu(&gb);
                *dst32++ += (t & 0xf0) << 8;
                *dst32++ += (t & 0x0f) << 12;
                t = bytestream2_get_byteu(&gb);
                *dst32++ += (t & 0xf0) << 8;
                *dst32++ += (t & 0x0f) << 12;
            }
        } while (--blocks);
        }
        return dst32;
            do {
                for (i = 2; i; i--) {
                    dst32[0] = bytestream2_get_be16u(&gb) << 16;
                    dst32[1] = bytestream2_get_be16u(&gb) << 16;
                    *dst32++ += bytestream2_get_byteu(&gb) << 8;
                    *dst32++ += bytestream2_get_byteu(&gb) << 8;
                }
            } while (--blocks);
        } else {
            }
        }
        return dst32;
    default:
        return NULL;
    }
}

                                int *got_frame_ptr, AVPacket *avpkt)
{

        av_log(avctx, AV_LOG_ERROR, "PCM packet too small\n");
        return AVERROR_INVALIDDATA;
    }

        return retval;
        av_log(avctx, AV_LOG_WARNING, "block_size has changed %d != %d\n", s->last_block_size, s->block_size);
        s->extra_sample_count = 0;
    }


    /* get output buffer */
        return retval;

    /* consume leftover samples from last packet */
        int missing_samples = s->block_size - s->extra_sample_count;
        if (buf_size >= missing_samples) {
            memcpy(s->extra_samples + s->extra_sample_count, src,
                   missing_samples);
            dst = pcm_dvd_decode_samples(avctx, s->extra_samples, dst, 1);
            src += missing_samples;
            buf_size -= missing_samples;
            s->extra_sample_count = 0;
            blocks--;
        } else {
            /* new packet still doesn't have enough samples */
            memcpy(s->extra_samples + s->extra_sample_count, src, buf_size);
            s->extra_sample_count += buf_size;
            return avpkt->size;
        }
    }

    /* decode remaining complete samples */
    }

    /* store leftover samples */
        src += blocks * s->block_size;
        memcpy(s->extra_samples, src, buf_size);
        s->extra_sample_count = buf_size;
    }


}

AVCodec ff_pcm_dvd_decoder = {
    .name           = "pcm_dvd",
    .long_name      = NULL_IF_CONFIG_SMALL("PCM signed 16|20|24-bit big-endian for DVD media"),
    .type           = AVMEDIA_TYPE_AUDIO,
    .id             = AV_CODEC_ID_PCM_DVD,
    .priv_data_size = sizeof(PCMDVDContext),
    .init           = pcm_dvd_decode_init,
    .decode         = pcm_dvd_decode_frame,
    .close          = pcm_dvd_decode_uninit,
    .capabilities   = AV_CODEC_CAP_DR1,
    .sample_fmts    = (const enum AVSampleFormat[]) {
        AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_S32, AV_SAMPLE_FMT_NONE
    }
};
