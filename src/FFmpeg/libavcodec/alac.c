/*
 * ALAC (Apple Lossless Audio Codec) decoder
 * Copyright (c) 2005 David Hammerton
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
 * ALAC (Apple Lossless Audio Codec) decoder
 * @author 2005 David Hammerton
 * @see http://crazney.net/programs/itunes/alac.html
 *
 * Note: This decoder expects a 36-byte QuickTime atom to be
 * passed through the extradata[_size] fields. This atom is tacked onto
 * the end of an 'alac' stsd atom and has the following format:
 *
 * 32 bits  atom size
 * 32 bits  tag                  ("alac")
 * 32 bits  tag version          (0)
 * 32 bits  samples per frame    (used when not set explicitly in the frames)
 *  8 bits  compatible version   (0)
 *  8 bits  sample size
 *  8 bits  history mult         (40)
 *  8 bits  initial history      (10)
 *  8 bits  rice param limit     (14)
 *  8 bits  channels
 * 16 bits  maxRun               (255)
 * 32 bits  max coded frame size (0 means unknown)
 * 32 bits  average bitrate      (0 means unknown)
 * 32 bits  samplerate
 */

#include <inttypes.h>

#include "libavutil/channel_layout.h"
#include "libavutil/opt.h"
#include "avcodec.h"
#include "get_bits.h"
#include "bytestream.h"
#include "internal.h"
#include "thread.h"
#include "unary.h"
#include "mathops.h"
#include "alac_data.h"
#include "alacdsp.h"

#define ALAC_EXTRADATA_SIZE 36

typedef struct ALACContext {
    AVClass *class;
    AVCodecContext *avctx;
    GetBitContext gb;
    int channels;

    int32_t *predict_error_buffer[2];
    int32_t *output_samples_buffer[2];
    int32_t *extra_bits_buffer[2];

    uint32_t max_samples_per_frame;
    uint8_t  sample_size;
    uint8_t  rice_history_mult;
    uint8_t  rice_initial_history;
    uint8_t  rice_limit;
    int      sample_rate;

    int extra_bits;     /**< number of extra bits beyond 16-bit */
    int nb_samples;     /**< number of samples in the current frame */

    int direct_output;
    int extra_bit_bug;

    ALACDSPContext dsp;
} ALACContext;

{

        /* use alternative encoding */

        /* multiply x by 2^k - 1, as part of their strange algorithm */

        } else
    }
}

                            int nb_samples, int bps, int rice_history_mult)
{


            return AVERROR_INVALIDDATA;

        /* calculate rice param and decode next value */

        /* update the history */
            history = 0xffff;
        else

        /* special case: there may be compressed blocks of 0 */

            /* calculate rice param and decode block size */

                    av_log(alac->avctx, AV_LOG_ERROR,
                           "invalid zero block size of %d %d %d\n", block_size,
                           nb_samples, i);
                    block_size = nb_samples - i - 1;
                }
                       block_size * sizeof(*output_buffer));
            }
            history = 0;
        }
    }
    return 0;
}

{
}

                           int nb_samples, int bps, int16_t *lpc_coefs,
                           int lpc_order, int lpc_quant)
{

    /* first sample always copies */

        return;

        memcpy(&buffer_out[1], &error_buffer[1],
               (nb_samples - 1) * sizeof(*buffer_out));
        return;
    }

        /* simple 1st-order prediction */
        for (i = 1; i < nb_samples; i++) {
            buffer_out[i] = sign_extend(buffer_out[i - 1] + error_buffer[i],
                                        bps);
        }
        return;
    }

    /* read warm-up samples */

    /* NOTE: 4 and 8 are very common cases that could be optimized. */


        /* LPC prediction */

        /* adapt LPC coefficients */
            }
        }
    }
}

                          int channels)
{


    /* the number of output samples is stored in the frame */

        avpriv_report_missing_feature(avctx, "bps %d", bps);
        return AVERROR_PATCHWELCOME;
    }
        return AVERROR_INVALIDDATA;

    /* whether the frame is compressed */

    else
        av_log(avctx, AV_LOG_ERROR, "invalid samples per frame: %"PRIu32"\n",
               output_samples);
        return AVERROR_INVALIDDATA;
    }
        /* get output buffer */
            return ret;
    } else if (output_samples != alac->nb_samples) {
        av_log(avctx, AV_LOG_ERROR, "sample count mismatch: %"PRIu32" != %d\n",
               output_samples, alac->nb_samples);
        return AVERROR_INVALIDDATA;
    }
    }


            avpriv_request_sample(alac->avctx,
                                  "Compression with rice limit 0");
            return AVERROR(ENOSYS);
        }


            return AVERROR_INVALIDDATA;


                return AVERROR_INVALIDDATA;

            /* read the predictor table */
        }

                    return AVERROR_INVALIDDATA;
            }
        }
                            alac->nb_samples, bps,
                return ret;

            /* adaptive FIR filter */
                /* Prediction type 15 runs the adaptive FIR twice.
                 * The first pass uses the special-case coef_num = 31, while
                 * the second pass uses the coefs from the bitstream.
                 *
                 * However, this prediction type is not currently used by the
                 * reference encoder.
                 */
                lpc_prediction(alac->predict_error_buffer[ch],
                               alac->predict_error_buffer[ch],
                               alac->nb_samples, bps, NULL, 31, 0);
                av_log(avctx, AV_LOG_WARNING, "unknown prediction type: %i\n",
                       prediction_type[ch]);
            }
        }
    } else {
        /* not compressed, easy case */
                return AVERROR_INVALIDDATA;
            }
        }
    }

            alac->dsp.append_extra_bits[1](alac->output_samples_buffer, alac->extra_bits_buffer,
                                           alac->extra_bits, channels, alac->nb_samples);
        }

                                         decorr_shift, decorr_left_weight);
        }

                                           alac->extra_bits, channels, alac->nb_samples);
        }
    } else if (alac->extra_bits) {
        alac->dsp.append_extra_bits[0](alac->output_samples_buffer, alac->extra_bits_buffer,
                                       alac->extra_bits, channels, alac->nb_samples);
    }

    case 16: {
        }}
        break;
    case 20: {
        for (ch = 0; ch < channels; ch++) {
            for (i = 0; i < alac->nb_samples; i++)
                alac->output_samples_buffer[ch][i] *= 1U << 12;
        }}
        break;
    case 24: {
        }}
        break;
    }

    return 0;
}

                             int *got_frame_ptr, AVPacket *avpkt)
{

        return ret;

            got_end = 1;
            break;
        }
            avpriv_report_missing_feature(avctx, "Syntax element %d", element);
            return AVERROR_PATCHWELCOME;
        }

            av_log(avctx, AV_LOG_ERROR, "invalid element channel count\n");
            return AVERROR_INVALIDDATA;
        }

                             ff_alac_channel_layout_offsets[alac->channels - 1][ch],
                             channels);
            return ret;

        ch += channels;
    }
        av_log(avctx, AV_LOG_ERROR, "no end tag found. incomplete packet.\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "Error : %d bits left\n",
               avpkt->size * 8 - get_bits_count(&alac->gb));
    }

    else
        av_log(avctx, AV_LOG_WARNING, "Failed to decode all channels\n");

}

{

    }

}

{

    }

            return AVERROR(ENOMEM);

                return AVERROR(ENOMEM);
        }

            return AVERROR(ENOMEM);
    }
    return 0;
}

{



        alac->max_samples_per_frame > 4096 * 4096) {
        av_log(alac->avctx, AV_LOG_ERROR,
               "max samples per frame invalid: %"PRIu32"\n",
               alac->max_samples_per_frame);
        return AVERROR_INVALIDDATA;
    }

}

{

    /* initialize from the extradata */
        av_log(avctx, AV_LOG_ERROR, "extradata is too small\n");
        return AVERROR_INVALIDDATA;
    }
        av_log(avctx, AV_LOG_ERROR, "set_info failed\n");
        return ret;
    }

    case 24:
    default: avpriv_request_sample(avctx, "Sample depth %d", alac->sample_size);
             return AVERROR_PATCHWELCOME;
    }

        av_log(avctx, AV_LOG_WARNING, "Invalid channel count\n");
        alac->channels = avctx->channels;
    } else {
            alac->channels = avctx->channels;
        else
    }
        avpriv_report_missing_feature(avctx, "Channel count %d",
                                      avctx->channels);
        return AVERROR_PATCHWELCOME;
    }

        av_log(avctx, AV_LOG_ERROR, "Error allocating buffers\n");
        return ret;
    }


}

static const AVOption options[] = {
    { "extra_bits_bug", "Force non-standard decoding process",
      offsetof(ALACContext, extra_bit_bug), AV_OPT_TYPE_BOOL, { .i64 = 0 },
      0, 1, AV_OPT_FLAG_AUDIO_PARAM | AV_OPT_FLAG_DECODING_PARAM },
    { NULL },
};

static const AVClass alac_class = {
    .class_name = "alac",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
};

AVCodec ff_alac_decoder = {
    .name           = "alac",
    .long_name      = NULL_IF_CONFIG_SMALL("ALAC (Apple Lossless Audio Codec)"),
    .type           = AVMEDIA_TYPE_AUDIO,
    .id             = AV_CODEC_ID_ALAC,
    .priv_data_size = sizeof(ALACContext),
    .init           = alac_decode_init,
    .close          = alac_decode_close,
    .decode         = alac_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1 | AV_CODEC_CAP_FRAME_THREADS,
    .caps_internal  = FF_CODEC_CAP_INIT_CLEANUP,
    .priv_class     = &alac_class
};
