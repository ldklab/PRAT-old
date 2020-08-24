/*
 * Copyright (C) 2016 foo86
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

#include "libavutil/opt.h"
#include "libavutil/channel_layout.h"

#include "dcadec.h"
#include "dcahuff.h"
#include "dca_syncwords.h"
#include "profiles.h"

#define MIN_PACKET_SIZE     16
#define MAX_PACKET_SIZE     0x104000

{
         2,  0, 1, 9, 10,  3,  8,  4,  5,  9, 10, 6, 7, 12,
        13, 14, 3, 6,  7, 11, 12, 14, 16, 15, 17, 8, 4,  5,
    };

         2,  0, 1, 4,  5,  3,  8,  4,  5,  9, 10, 6, 7, 12,
        13, 14, 3, 9, 10, 11, 12, 14, 16, 15, 17, 8, 4,  5,
    };


        for (dca_ch = 0; dca_ch < DCA_SPEAKER_COUNT; dca_ch++)
            if (dca_mask & (1U << dca_ch))
                ch_remap[nchannels++] = dca_ch;
        avctx->channel_layout = dca_mask;
    } else {
            dca2wav = dca2wav_wide;
        else
                }
            }
        }
    }

}

                                    int *coeff_l, int nsamples, int ch_mask)
{


    // Scale left and right channels

    // Downmix remaining channels
            continue;

                             *coeff_l, nsamples);

                             *coeff_r, nsamples);

    }

                                    int *coeff_l, int nsamples, int ch_mask)
{


    // Scale left and right channels

    // Downmix remaining channels
            continue;

                                     *coeff_l * scale, nsamples);

                                     *coeff_r * scale, nsamples);

    }

                               int *got_frame_ptr, AVPacket *avpkt)
{

        av_log(avctx, AV_LOG_ERROR, "Invalid packet size\n");
        return AVERROR_INVALIDDATA;
    }

    // Convert input to BE format
        av_fast_padded_malloc(&s->buffer, &s->buffer_size, input_size);
        if (!s->buffer)
            return AVERROR(ENOMEM);

        for (i = 0, ret = AVERROR_INVALIDDATA; i < input_size - MIN_PACKET_SIZE + 1 && ret < 0; i++)
            ret = avpriv_dca_convert_bitstream(input + i, input_size - i, s->buffer, s->buffer_size);

        if (ret < 0) {
            av_log(avctx, AV_LOG_ERROR, "Not a valid DCA frame\n");
            return ret;
        }

        input      = s->buffer;
        input_size = ret;
    }


    // Parse backward compatible core sub-stream

            return ret;


        // EXXS data must be aligned on 4-byte boundary
        }
    }


        // Parse extension sub-stream (EXSS)
                if (avctx->err_recognition & AV_EF_EXPLODE)
                    return ret;
            } else {
            }
        }

        // Parse XLL component in EXSS
                // Conceal XLL synchronization error
                if (ret == AVERROR(EAGAIN)
                    && (prev_packet & DCA_PACKET_XLL)
                    && (s->packet & DCA_PACKET_CORE))
                    s->packet |= DCA_PACKET_XLL | DCA_PACKET_RECOVERY;
                else if (ret == AVERROR(ENOMEM) || (avctx->err_recognition & AV_EF_EXPLODE))
                    return ret;
            } else {
            }
        }

        // Parse LBR component in EXSS
            if ((ret = ff_dca_lbr_parse(&s->lbr, input, asset)) < 0) {
                if (ret == AVERROR(ENOMEM) || (avctx->err_recognition & AV_EF_EXPLODE))
                    return ret;
            } else {
                s->packet |= DCA_PACKET_LBR;
            }
        }

        // Parse core extensions in EXSS or backward compatible core sub-stream
            return ret;
    }

    // Filter the frame
        if ((ret = ff_dca_lbr_filter_frame(&s->lbr, frame)) < 0)
            return ret;

            // Enable X96 synthesis if needed

                return ret;

            // Force lossy downmixed output on the first core frame filtered.
            // This prevents audible clicks when seeking and is consistent with
            // what reference decoder does when there are multiple channel sets.
            }

            // Set 'residual ok' flag for the next frame
        }

            // Fall back to core unless hard error
            if (!(s->packet & DCA_PACKET_CORE))
                return ret;
            if (ret != AVERROR_INVALIDDATA || (avctx->err_recognition & AV_EF_EXPLODE))
                return ret;
            if ((ret = ff_dca_core_filter_frame(&s->core, frame)) < 0)
                return ret;
        }
            return ret;
    } else {
        av_log(avctx, AV_LOG_ERROR, "No valid DCA sub-stream found\n");
        if (s->core_only)
            av_log(avctx, AV_LOG_WARNING, "Consider disabling 'core_only' option\n");
        return AVERROR_INVALIDDATA;
    }


}

static av_cold void dcadec_flush(AVCodecContext *avctx)
{
    DCAContext *s = avctx->priv_data;

    ff_dca_core_flush(&s->core);
    ff_dca_xll_flush(&s->xll);
    ff_dca_lbr_flush(&s->lbr);

    s->packet &= DCA_PACKET_MASK;
}

{



}

{



        return AVERROR(ENOMEM);

        return AVERROR(ENOMEM);



    case AV_CH_LAYOUT_STEREO_DOWNMIX:
    case AV_CH_LAYOUT_5POINT0:
        s->request_channel_layout = DCA_SPEAKER_LAYOUT_5POINT0;
        break;
    default:
        av_log(avctx, AV_LOG_WARNING, "Invalid request_channel_layout\n");
        break;
    }

    return 0;
}

#define OFFSET(x) offsetof(DCAContext, x)
#define PARAM AV_OPT_FLAG_AUDIO_PARAM | AV_OPT_FLAG_DECODING_PARAM

static const AVOption dcadec_options[] = {
    { "core_only", "Decode core only without extensions", OFFSET(core_only), AV_OPT_TYPE_BOOL, { .i64 = 0 }, 0, 1, PARAM },
    { NULL }
};

static const AVClass dcadec_class = {
    .class_name = "DCA decoder",
    .item_name  = av_default_item_name,
    .option     = dcadec_options,
    .version    = LIBAVUTIL_VERSION_INT,
    .category   = AV_CLASS_CATEGORY_DECODER,
};

AVCodec ff_dca_decoder = {
    .name           = "dca",
    .long_name      = NULL_IF_CONFIG_SMALL("DCA (DTS Coherent Acoustics)"),
    .type           = AVMEDIA_TYPE_AUDIO,
    .id             = AV_CODEC_ID_DTS,
    .priv_data_size = sizeof(DCAContext),
    .init           = dcadec_init,
    .decode         = dcadec_decode_frame,
    .close          = dcadec_close,
    .flush          = dcadec_flush,
    .capabilities   = AV_CODEC_CAP_DR1 | AV_CODEC_CAP_CHANNEL_CONF,
    .sample_fmts    = (const enum AVSampleFormat[]) { AV_SAMPLE_FMT_S16P, AV_SAMPLE_FMT_S32P,
                                                      AV_SAMPLE_FMT_FLTP, AV_SAMPLE_FMT_NONE },
    .priv_class     = &dcadec_class,
    .profiles       = NULL_IF_CONFIG_SMALL(ff_dca_profiles),
    .caps_internal  = FF_CODEC_CAP_INIT_CLEANUP,
};
