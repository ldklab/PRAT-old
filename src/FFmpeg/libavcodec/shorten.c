/*
 * Shorten decoder
 * Copyright (c) 2005 Jeff Muizelaar
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
 * Shorten decoder
 * @author Jeff Muizelaar
 */

#include <limits.h>
#include "avcodec.h"
#include "bswapdsp.h"
#include "bytestream.h"
#include "get_bits.h"
#include "golomb.h"
#include "internal.h"

#define MAX_CHANNELS 8
#define MAX_BLOCKSIZE 65535

#define OUT_BUFFER_SIZE 16384

#define ULONGSIZE 2

#define WAVE_FORMAT_PCM 0x0001

#define DEFAULT_BLOCK_SIZE 256

#define TYPESIZE 4
#define CHANSIZE 0
#define LPCQSIZE 2
#define ENERGYSIZE 3
#define BITSHIFTSIZE 2

#define TYPE_S8    1
#define TYPE_U8    2
#define TYPE_S16HL 3
#define TYPE_U16HL 4
#define TYPE_S16LH 5
#define TYPE_U16LH 6

#define NWRAP 3
#define NSKIPSIZE 1

#define LPCQUANT 5
#define V2LPCQOFFSET (1 << LPCQUANT)

#define FNSIZE 2
#define FN_DIFF0        0
#define FN_DIFF1        1
#define FN_DIFF2        2
#define FN_DIFF3        3
#define FN_QUIT         4
#define FN_BLOCKSIZE    5
#define FN_BITSHIFT     6
#define FN_QLPC         7
#define FN_ZERO         8
#define FN_VERBATIM     9

/** indicates if the FN_* command is audio or non-audio */
static const uint8_t is_audio_command[10] = { 1, 1, 1, 1, 0, 0, 0, 1, 1, 0 };

#define VERBATIM_CKSIZE_SIZE 5
#define VERBATIM_BYTE_SIZE 8
#define CANONICAL_HEADER_SIZE 44

typedef struct ShortenContext {
    AVCodecContext *avctx;
    GetBitContext gb;

    int min_framesize, max_framesize;
    unsigned channels;

    int32_t *decoded[MAX_CHANNELS];
    int32_t *decoded_base[MAX_CHANNELS];
    int32_t *offset[MAX_CHANNELS];
    int *coeffs;
    uint8_t *bitstream;
    int bitstream_size;
    int bitstream_index;
    unsigned int allocated_bitstream_size;
    int header_size;
    uint8_t header[OUT_BUFFER_SIZE];
    int version;
    int cur_chan;
    int bitshift;
    int nmean;
    int internal_ftype;
    int nwrap;
    int blocksize;
    int bitindex;
    int32_t lpcqoffset;
    int got_header;
    int got_quit_command;
    int swap;
    BswapDSPContext bdsp;
} ShortenContext;

{


}

{

            av_log(s->avctx, AV_LOG_ERROR, "nmean too large\n");
            return AVERROR_INVALIDDATA;
        }
            av_log(s->avctx, AV_LOG_ERROR,
                   "s->blocksize + s->nwrap too large\n");
            return AVERROR_INVALIDDATA;
        }

                               sizeof(int32_t),
            return err;

                               sizeof(s->decoded_base[0][0]))) < 0)
            return err;
    }

        return err;

    return 0;
}

{
            return AVERROR_INVALIDDATA;
    }
}

static void fix_bitshift(ShortenContext *s, int32_t *buffer)
{
    int i;

    if (s->bitshift == 32) {
        for (i = 0; i < s->blocksize; i++)
            buffer[i] = 0;
    } else if (s->bitshift != 0) {
        for (i = 0; i < s->blocksize; i++)
            buffer[i] *= 1U << s->bitshift;
    }
}

{
    /* initialise offset */
    case TYPE_U8:
        s->avctx->sample_fmt = AV_SAMPLE_FMT_U8P;
        mean = 0x80;
        break;
    case TYPE_S16LH:
    default:
        av_log(s->avctx, AV_LOG_ERROR, "unknown audio type\n");
        return AVERROR_PATCHWELCOME;
    }

    return 0;
}

static int decode_aiff_header(AVCodecContext *avctx, const uint8_t *header,
                              int header_size)
{
    ShortenContext *s = avctx->priv_data;
    int len, bps, exp;
    GetByteContext gb;
    uint64_t val;
    uint32_t tag;

    bytestream2_init(&gb, header, header_size);

    if (bytestream2_get_le32(&gb) != MKTAG('F', 'O', 'R', 'M')) {
        av_log(avctx, AV_LOG_ERROR, "missing FORM tag\n");
        return AVERROR_INVALIDDATA;
    }

    bytestream2_skip(&gb, 4); /* chunk size */

    tag = bytestream2_get_le32(&gb);
    if (tag != MKTAG('A', 'I', 'F', 'F') &&
        tag != MKTAG('A', 'I', 'F', 'C')) {
        av_log(avctx, AV_LOG_ERROR, "missing AIFF tag\n");
        return AVERROR_INVALIDDATA;
    }

    while (bytestream2_get_le32(&gb) != MKTAG('C', 'O', 'M', 'M')) {
        len = bytestream2_get_be32(&gb);
        if (len < 0 || bytestream2_get_bytes_left(&gb) < 18LL + len + (len&1)) {
            av_log(avctx, AV_LOG_ERROR, "no COMM chunk found\n");
            return AVERROR_INVALIDDATA;
        }
        bytestream2_skip(&gb, len + (len & 1));
    }
    len = bytestream2_get_be32(&gb);

    if (len < 18) {
        av_log(avctx, AV_LOG_ERROR, "COMM chunk was too short\n");
        return AVERROR_INVALIDDATA;
    }

    bytestream2_skip(&gb, 6);
    bps = bytestream2_get_be16(&gb);
    avctx->bits_per_coded_sample = bps;

    s->swap = tag == MKTAG('A', 'I', 'F', 'C');

    if (bps != 16 && bps != 8) {
        av_log(avctx, AV_LOG_ERROR, "unsupported number of bits per sample: %d\n", bps);
        return AVERROR(ENOSYS);
    }

    exp = bytestream2_get_be16(&gb) - 16383 - 63;
    val = bytestream2_get_be64(&gb);
    if (exp < -63 || exp > 63) {
        av_log(avctx, AV_LOG_ERROR, "exp %d is out of range\n", exp);
        return AVERROR_INVALIDDATA;
    }
    if (exp >= 0)
        avctx->sample_rate = val << exp;
    else
        avctx->sample_rate = (val + (1ULL<<(-exp-1))) >> -exp;
    len -= 18;
    if (len > 0)
        av_log(avctx, AV_LOG_INFO, "%d header bytes unparsed\n", len);

    return 0;
}

                              int header_size)
{


        av_log(avctx, AV_LOG_ERROR, "missing RIFF tag\n");
        return AVERROR_INVALIDDATA;
    }


        av_log(avctx, AV_LOG_ERROR, "missing WAVE tag\n");
        return AVERROR_INVALIDDATA;
    }

        len = bytestream2_get_le32(&gb);
        bytestream2_skip(&gb, len);
        if (len < 0 || bytestream2_get_bytes_left(&gb) < 16) {
            av_log(avctx, AV_LOG_ERROR, "no fmt chunk found\n");
            return AVERROR_INVALIDDATA;
        }
    }

        av_log(avctx, AV_LOG_ERROR, "fmt chunk was too short\n");
        return AVERROR_INVALIDDATA;
    }


    case WAVE_FORMAT_PCM:
    default:
        av_log(avctx, AV_LOG_ERROR, "unsupported wave format\n");
        return AVERROR(ENOSYS);
    }


        av_log(avctx, AV_LOG_ERROR, "unsupported number of bits per sample: %d\n", bps);
        return AVERROR(ENOSYS);
    }

        av_log(avctx, AV_LOG_INFO, "%d header bytes unparsed\n", len);

    return 0;
}

static const int fixed_coeffs[][3] = {
    { 0,  0,  0 },
    { 1,  0,  0 },
    { 2, -1,  0 },
    { 3, -3,  1 }
};

                               int residual_size, int32_t coffset)
{

        /* read/validate prediction order */
        pred_order = get_ur_golomb_shorten(&s->gb, LPCQSIZE);
        if ((unsigned)pred_order > s->nwrap) {
            av_log(s->avctx, AV_LOG_ERROR, "invalid pred_order %d\n",
                   pred_order);
            return AVERROR(EINVAL);
        }
        /* read LPC coefficients */
        for (i = 0; i < pred_order; i++)
            s->coeffs[i] = get_sr_golomb_shorten(&s->gb, LPCQUANT);
        coeffs = s->coeffs;

        qshift = LPCQUANT;
    } else {
        /* fixed LPC coeffs */
            av_log(s->avctx, AV_LOG_ERROR, "invalid pred_order %d\n",
                   pred_order);
            return AVERROR_INVALIDDATA;
        }
    }

    /* subtract offset from previous samples to use in prediction */
        for (i = -pred_order; i < 0; i++)
            s->decoded[channel][i] -= (unsigned)coffset;

    /* decode residual and do LPC prediction */
        sum = init_sum;
    }

    /* add offset to current samples */
        for (i = 0; i < s->blocksize; i++)
            s->decoded[channel][i] += (unsigned)coffset;

    return 0;
}

{
    /* shorten signature */
        av_log(s->avctx, AV_LOG_ERROR, "missing shorten magic 'ajkg'\n");
        return AVERROR_INVALIDDATA;
    }


        av_log(s->avctx, AV_LOG_ERROR, "No channels reported\n");
        return AVERROR_INVALIDDATA;
    }
        av_log(s->avctx, AV_LOG_ERROR, "too many channels: %d\n", s->channels);
        s->channels = 0;
        return AVERROR_INVALIDDATA;
    }

    /* get blocksize if version > 0 */

            av_log(s->avctx, AV_LOG_ERROR,
                   "invalid or unsupported block size: %d\n",
                   blocksize);
            return AVERROR(EINVAL);
        }

            av_log(s->avctx, AV_LOG_ERROR, "maxnlpc is: %d\n", maxnlpc);
            return AVERROR_INVALIDDATA;
        }
            av_log(s->avctx, AV_LOG_ERROR, "nmean is: %d\n", s->nmean);
            return AVERROR_INVALIDDATA;
        }

            av_log(s->avctx, AV_LOG_ERROR, "invalid skip_bytes: %d\n", skip_bytes);
            return AVERROR_INVALIDDATA;
        }

            skip_bits(&s->gb, 8);
    }


        goto end;

        av_log(s->avctx, AV_LOG_ERROR,
               "missing verbatim section at beginning of stream\n");
        return AVERROR_INVALIDDATA;
    }

        s->header_size < CANONICAL_HEADER_SIZE) {
        av_log(s->avctx, AV_LOG_ERROR, "header is wrong size: %d\n",
               s->header_size);
        return AVERROR_INVALIDDATA;
    }


            return ret;
    } else if (AV_RL32(s->header) == MKTAG('F','O','R','M')) {
        if ((ret = decode_aiff_header(s->avctx, s->header, s->header_size)) < 0)
            return ret;
    } else {
        avpriv_report_missing_feature(s->avctx, "unsupported bit packing %"
                                      PRIX32, AV_RL32(s->header));
        return AVERROR_PATCHWELCOME;
    }


        return ret;

        return ret;



}

                                int *got_frame_ptr, AVPacket *avpkt)
{

    /* allocate internal bitstream buffer */
                                  s->max_framesize + AV_INPUT_BUFFER_PADDING_SIZE);
            s->max_framesize = 0;
            av_log(avctx, AV_LOG_ERROR, "error allocating bitstream buffer\n");
            return AVERROR(ENOMEM);
        }
    }

    /* append current packet data to bitstream buffer */

                s->bitstream_size);
    }
               buf_size);

    /* do not decode until buffer has at least max_framesize bytes or
     * the end of the file has been reached */
    }
    /* init and position bitstream reader */
        return ret;

    /* process header or next subblock */

            return ret;


                av_log(avctx, AV_LOG_ERROR, "error allocating bitstream buffer\n");
                return AVERROR(ENOMEM);
            }
        }
    }

    /* if quit command was read previously, don't decode anything */
        *got_frame_ptr = 0;
        return avpkt->size;
    }


        }


            av_log(avctx, AV_LOG_ERROR, "unknown shorten function %d\n", cmd);
            *got_frame_ptr = 0;
            break;
        }

            /* process non-audio command */
            switch (cmd) {
            case FN_VERBATIM:
                len = get_ur_golomb_shorten(&s->gb, VERBATIM_CKSIZE_SIZE);
                if (len < 0 || len > get_bits_left(&s->gb)) {
                    av_log(avctx, AV_LOG_ERROR, "verbatim length %d invalid\n",
                           len);
                    return AVERROR_INVALIDDATA;
                }
                while (len--)
                    get_ur_golomb_shorten(&s->gb, VERBATIM_BYTE_SIZE);
                break;
            case FN_BITSHIFT: {
                unsigned bitshift = get_ur_golomb_shorten(&s->gb, BITSHIFTSIZE);
                if (bitshift > 32) {
                    av_log(avctx, AV_LOG_ERROR, "bitshift %d is invalid\n",
                           bitshift);
                    return AVERROR_INVALIDDATA;
                }
                s->bitshift = bitshift;
                break;
            }
            case FN_BLOCKSIZE: {
                unsigned blocksize = get_uint(s, av_log2(s->blocksize));
                if (blocksize > s->blocksize) {
                    avpriv_report_missing_feature(avctx,
                                                  "Increasing block size");
                    return AVERROR_PATCHWELCOME;
                }
                if (!blocksize || blocksize > MAX_BLOCKSIZE) {
                    av_log(avctx, AV_LOG_ERROR, "invalid or unsupported "
                                                "block size: %d\n", blocksize);
                    return AVERROR(EINVAL);
                }
                s->blocksize = blocksize;
                break;
            }
            case FN_QUIT:
                s->got_quit_command = 1;
                break;
            }
            if (cmd == FN_QUIT)
                break;
        } else {
            /* process audio command */

            /* get Rice code for residual decoding */
                /* This is a hack as version 0 differed in the definition
                 * of get_sr_golomb_shorten(). */
                    residual_size--;
                    av_log(avctx, AV_LOG_ERROR, "residual size unsupportd: %d\n", residual_size);
                    return AVERROR_INVALIDDATA;
                }
            }

            /* calculate sample offset using means from previous blocks */
                coffset = s->offset[channel][0];
            else {
            }

            /* decode samples for this channel */
                for (i = 0; i < s->blocksize; i++)
                    s->decoded[channel][i] = 0;
            } else {
                                               residual_size, coffset)) < 0)
                    return ret;
            }

            /* update means with info from the current block */


                    s->offset[channel][s->nmean - 1] = sum / s->blocksize;
                else
            }

            /* copy wrap samples for use with next block */

            /* shift samples to add in unused zero bits which were removed
             * during encoding */

            /* if this is the last channel in the block, output the samples */

                /* get output buffer */
                    return ret;

                        case TYPE_U8:
                            *samples_u8++ = av_clip_uint8(s->decoded[chan][i]);
                            break;
                        case TYPE_S16LH:
                        }
                        s->bdsp.bswap16_buf(((uint16_t **)frame->extended_data)[chan],
                                            ((uint16_t **)frame->extended_data)[chan],
                                            s->blocksize);

                }

            }
        }
    }

    }
    } else
        return i;
}

{

    }

}

AVCodec ff_shorten_decoder = {
    .name           = "shorten",
    .long_name      = NULL_IF_CONFIG_SMALL("Shorten"),
    .type           = AVMEDIA_TYPE_AUDIO,
    .id             = AV_CODEC_ID_SHORTEN,
    .priv_data_size = sizeof(ShortenContext),
    .init           = shorten_decode_init,
    .close          = shorten_decode_close,
    .decode         = shorten_decode_frame,
    .capabilities   = AV_CODEC_CAP_SUBFRAMES | AV_CODEC_CAP_DELAY | AV_CODEC_CAP_DR1,
    .sample_fmts    = (const enum AVSampleFormat[]) { AV_SAMPLE_FMT_S16P,
                                                      AV_SAMPLE_FMT_U8P,
                                                      AV_SAMPLE_FMT_NONE },
};
