/*
 * ATRAC3+ compatible decoder
 *
 * Copyright (c) 2010-2013 Maxim Poliakovski
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
 * Sony ATRAC3+ compatible decoder.
 *
 * Container formats used to store its data:
 * RIFF WAV (.at3) and Sony OpenMG (.oma, .aa3).
 *
 * Technical description of this codec can be found here:
 * http://wiki.multimedia.cx/index.php?title=ATRAC3plus
 *
 * Kudos to Benjamin Larsson and Michael Karcher
 * for their precious technical help!
 */

#include <stdint.h>
#include <string.h>

#include "libavutil/channel_layout.h"
#include "libavutil/float_dsp.h"
#include "avcodec.h"
#include "get_bits.h"
#include "internal.h"
#include "atrac.h"
#include "atrac3plus.h"

typedef struct ATRAC3PContext {
    GetBitContext gb;
    AVFloatDSPContext *fdsp;

    DECLARE_ALIGNED(32, float, samples)[2][ATRAC3P_FRAME_SAMPLES];  ///< quantized MDCT spectrum
    DECLARE_ALIGNED(32, float, mdct_buf)[2][ATRAC3P_FRAME_SAMPLES]; ///< output of the IMDCT
    DECLARE_ALIGNED(32, float, time_buf)[2][ATRAC3P_FRAME_SAMPLES]; ///< output of the gain compensation
    DECLARE_ALIGNED(32, float, outp_buf)[2][ATRAC3P_FRAME_SAMPLES];

    AtracGCContext gainc_ctx;   ///< gain compensation context
    FFTContext mdct_ctx;
    FFTContext ipqf_dct_ctx;    ///< IDCT context used by IPQF

    Atrac3pChanUnitCtx *ch_units;   ///< global channel units

    int num_channel_blocks;     ///< number of channel blocks
    uint8_t channel_blocks[5];  ///< channel configuration descriptor
    uint64_t my_channel_layout; ///< current channel layout
} ATRAC3PContext;

{



}

                                      AVCodecContext *avctx)
{

    case 1:
        if (avctx->channel_layout != AV_CH_FRONT_LEFT)
            avctx->channel_layout = AV_CH_LAYOUT_MONO;

        ctx->num_channel_blocks = 1;
        ctx->channel_blocks[0]  = CH_UNIT_MONO;
        break;
    case 3:
        avctx->channel_layout   = AV_CH_LAYOUT_SURROUND;
        ctx->num_channel_blocks = 2;
        ctx->channel_blocks[0]  = CH_UNIT_STEREO;
        ctx->channel_blocks[1]  = CH_UNIT_MONO;
        break;
    case 4:
        avctx->channel_layout   = AV_CH_LAYOUT_4POINT0;
        ctx->num_channel_blocks = 3;
        ctx->channel_blocks[0]  = CH_UNIT_STEREO;
        ctx->channel_blocks[1]  = CH_UNIT_MONO;
        ctx->channel_blocks[2]  = CH_UNIT_MONO;
        break;
    case 6:
        avctx->channel_layout   = AV_CH_LAYOUT_5POINT1_BACK;
        ctx->num_channel_blocks = 4;
        ctx->channel_blocks[0]  = CH_UNIT_STEREO;
        ctx->channel_blocks[1]  = CH_UNIT_MONO;
        ctx->channel_blocks[2]  = CH_UNIT_STEREO;
        ctx->channel_blocks[3]  = CH_UNIT_MONO;
        break;
    case 7:
        avctx->channel_layout   = AV_CH_LAYOUT_6POINT1_BACK;
        ctx->num_channel_blocks = 5;
        ctx->channel_blocks[0]  = CH_UNIT_STEREO;
        ctx->channel_blocks[1]  = CH_UNIT_MONO;
        ctx->channel_blocks[2]  = CH_UNIT_STEREO;
        ctx->channel_blocks[3]  = CH_UNIT_MONO;
        ctx->channel_blocks[4]  = CH_UNIT_MONO;
        break;
    case 8:
        avctx->channel_layout   = AV_CH_LAYOUT_7POINT1;
        ctx->num_channel_blocks = 5;
        ctx->channel_blocks[0]  = CH_UNIT_STEREO;
        ctx->channel_blocks[1]  = CH_UNIT_MONO;
        ctx->channel_blocks[2]  = CH_UNIT_STEREO;
        ctx->channel_blocks[3]  = CH_UNIT_STEREO;
        ctx->channel_blocks[4]  = CH_UNIT_MONO;
        break;
    default:
        av_log(avctx, AV_LOG_ERROR,
               "Unsupported channel count: %d!\n", avctx->channels);
        return AVERROR_INVALIDDATA;
    }

    return 0;
}

{

        av_log(avctx, AV_LOG_ERROR, "block_align is not set\n");
        return AVERROR(EINVAL);
    }


    /* initialize IPQF */




        return ret;



        return AVERROR(ENOMEM);
    }

        }

    }


}

static void decode_residual_spectrum(ATRAC3PContext *ctx, Atrac3pChanUnitCtx *ch_unit,
                                     float out[2][ATRAC3P_FRAME_SAMPLES],
                                     int num_channels,
                                     AVCodecContext *avctx)
{
    int i, sb, ch, qu, nspeclines, RNG_index;
    float *dst, q;
    int16_t *src;
    /* calculate RNG table index for each subband */
    int sb_RNG_index[ATRAC3P_SUBBANDS] = { 0 };

    if (ch_unit->mute_flag) {
        for (ch = 0; ch < num_channels; ch++)
            memset(out[ch], 0, ATRAC3P_FRAME_SAMPLES * sizeof(*out[ch]));
        return;
    }

    for (qu = 0, RNG_index = 0; qu < ch_unit->used_quant_units; qu++)
        RNG_index += ch_unit->channels[0].qu_sf_idx[qu] +
                     ch_unit->channels[1].qu_sf_idx[qu];

    for (sb = 0; sb < ch_unit->num_coded_subbands; sb++, RNG_index += 128)
        sb_RNG_index[sb] = RNG_index & 0x3FC;

    /* inverse quant and power compensation */
    for (ch = 0; ch < num_channels; ch++) {
        /* clear channel's residual spectrum */
        memset(out[ch], 0, ATRAC3P_FRAME_SAMPLES * sizeof(*out[ch]));

        for (qu = 0; qu < ch_unit->used_quant_units; qu++) {
            src        = &ch_unit->channels[ch].spectrum[ff_atrac3p_qu_to_spec_pos[qu]];
            dst        = &out[ch][ff_atrac3p_qu_to_spec_pos[qu]];
            nspeclines = ff_atrac3p_qu_to_spec_pos[qu + 1] -
                         ff_atrac3p_qu_to_spec_pos[qu];

            if (ch_unit->channels[ch].qu_wordlen[qu] > 0) {
                q = ff_atrac3p_sf_tab[ch_unit->channels[ch].qu_sf_idx[qu]] *
                    ff_atrac3p_mant_tab[ch_unit->channels[ch].qu_wordlen[qu]];
                for (i = 0; i < nspeclines; i++)
                    dst[i] = src[i] * q;
            }
        }

        for (sb = 0; sb < ch_unit->num_coded_subbands; sb++)
            ff_atrac3p_power_compensation(ch_unit, ctx->fdsp, ch, &out[ch][0],
                                          sb_RNG_index[sb], sb);
    }

    if (ch_unit->unit_type == CH_UNIT_STEREO) {
        for (sb = 0; sb < ch_unit->num_coded_subbands; sb++) {
            if (ch_unit->swap_channels[sb]) {
                for (i = 0; i < ATRAC3P_SUBBAND_SAMPLES; i++)
                    FFSWAP(float, out[0][sb * ATRAC3P_SUBBAND_SAMPLES + i],
                                  out[1][sb * ATRAC3P_SUBBAND_SAMPLES + i]);
            }

            /* flip coefficients' sign if requested */
            if (ch_unit->negate_coeffs[sb])
                for (i = 0; i < ATRAC3P_SUBBAND_SAMPLES; i++)
                    out[1][sb * ATRAC3P_SUBBAND_SAMPLES + i] = -(out[1][sb * ATRAC3P_SUBBAND_SAMPLES + i]);
        }
    }
}

static void reconstruct_frame(ATRAC3PContext *ctx, Atrac3pChanUnitCtx *ch_unit,
                              int num_channels, AVCodecContext *avctx)
{
    int ch, sb;

    for (ch = 0; ch < num_channels; ch++) {
        for (sb = 0; sb < ch_unit->num_subbands; sb++) {
            /* inverse transform and windowing */
            ff_atrac3p_imdct(ctx->fdsp, &ctx->mdct_ctx,
                             &ctx->samples[ch][sb * ATRAC3P_SUBBAND_SAMPLES],
                             &ctx->mdct_buf[ch][sb * ATRAC3P_SUBBAND_SAMPLES],
                             (ch_unit->channels[ch].wnd_shape_prev[sb] << 1) +
                             ch_unit->channels[ch].wnd_shape[sb], sb);

            /* gain compensation and overlapping */
            ff_atrac_gain_compensation(&ctx->gainc_ctx,
                                       &ctx->mdct_buf[ch][sb * ATRAC3P_SUBBAND_SAMPLES],
                                       &ch_unit->prev_buf[ch][sb * ATRAC3P_SUBBAND_SAMPLES],
                                       &ch_unit->channels[ch].gain_data_prev[sb],
                                       &ch_unit->channels[ch].gain_data[sb],
                                       ATRAC3P_SUBBAND_SAMPLES,
                                       &ctx->time_buf[ch][sb * ATRAC3P_SUBBAND_SAMPLES]);
        }

        /* zero unused subbands in both output and overlapping buffers */
        memset(&ch_unit->prev_buf[ch][ch_unit->num_subbands * ATRAC3P_SUBBAND_SAMPLES],
               0,
               (ATRAC3P_SUBBANDS - ch_unit->num_subbands) *
               ATRAC3P_SUBBAND_SAMPLES *
               sizeof(ch_unit->prev_buf[ch][ch_unit->num_subbands * ATRAC3P_SUBBAND_SAMPLES]));
        memset(&ctx->time_buf[ch][ch_unit->num_subbands * ATRAC3P_SUBBAND_SAMPLES],
               0,
               (ATRAC3P_SUBBANDS - ch_unit->num_subbands) *
               ATRAC3P_SUBBAND_SAMPLES *
               sizeof(ctx->time_buf[ch][ch_unit->num_subbands * ATRAC3P_SUBBAND_SAMPLES]));

        /* resynthesize and add tonal signal */
        if (ch_unit->waves_info->tones_present ||
            ch_unit->waves_info_prev->tones_present) {
            for (sb = 0; sb < ch_unit->num_subbands; sb++)
                if (ch_unit->channels[ch].tones_info[sb].num_wavs ||
                    ch_unit->channels[ch].tones_info_prev[sb].num_wavs) {
                    ff_atrac3p_generate_tones(ch_unit, ctx->fdsp, ch, sb,
                                              &ctx->time_buf[ch][sb * 128]);
                }
        }

        /* subband synthesis and acoustic signal output */
        ff_atrac3p_ipqf(&ctx->ipqf_dct_ctx, &ch_unit->ipqf_ctx[ch],
                        &ctx->time_buf[ch][0], &ctx->outp_buf[ch][0]);
    }

    /* swap window shape and gain control buffers. */
    for (ch = 0; ch < num_channels; ch++) {
        FFSWAP(uint8_t *, ch_unit->channels[ch].wnd_shape,
               ch_unit->channels[ch].wnd_shape_prev);
        FFSWAP(AtracGainInfo *, ch_unit->channels[ch].gain_data,
               ch_unit->channels[ch].gain_data_prev);
        FFSWAP(Atrac3pWavesData *, ch_unit->channels[ch].tones_info,
               ch_unit->channels[ch].tones_info_prev);
    }

    FFSWAP(Atrac3pWaveSynthParams *, ch_unit->waves_info, ch_unit->waves_info_prev);
}

                                int *got_frame_ptr, AVPacket *avpkt)
{

        return ret;

        return ret;

        av_log(avctx, AV_LOG_ERROR, "Invalid start bit!\n");
        return AVERROR_INVALIDDATA;
    }

            avpriv_report_missing_feature(avctx, "Channel unit extension");
            return AVERROR_PATCHWELCOME;
        }
            av_log(avctx, AV_LOG_ERROR,
                   "Frame data doesn't match channel configuration!\n");
            return AVERROR_INVALIDDATA;
        }


                                                  &ctx->ch_units[ch_block],
                                                  channels_to_process,
                                                  avctx)) < 0)
            return ret;

                                 channels_to_process, avctx);
                          channels_to_process, avctx);

                   ATRAC3P_FRAME_SAMPLES * sizeof(**samples_p));

    }


}

AVCodec ff_atrac3p_decoder = {
    .name           = "atrac3plus",
    .long_name      = NULL_IF_CONFIG_SMALL("ATRAC3+ (Adaptive TRansform Acoustic Coding 3+)"),
    .type           = AVMEDIA_TYPE_AUDIO,
    .id             = AV_CODEC_ID_ATRAC3P,
    .capabilities   = AV_CODEC_CAP_DR1,
    .caps_internal  = FF_CODEC_CAP_INIT_CLEANUP,
    .priv_data_size = sizeof(ATRAC3PContext),
    .init           = atrac3p_decode_init,
    .close          = atrac3p_decode_close,
    .decode         = atrac3p_decode_frame,
};

AVCodec ff_atrac3pal_decoder = {
    .name           = "atrac3plusal",
    .long_name      = NULL_IF_CONFIG_SMALL("ATRAC3+ AL (Adaptive TRansform Acoustic Coding 3+ Advanced Lossless)"),
    .type           = AVMEDIA_TYPE_AUDIO,
    .id             = AV_CODEC_ID_ATRAC3PAL,
    .capabilities   = AV_CODEC_CAP_DR1,
    .caps_internal  = FF_CODEC_CAP_INIT_CLEANUP,
    .priv_data_size = sizeof(ATRAC3PContext),
    .init           = atrac3p_decode_init,
    .close          = atrac3p_decode_close,
    .decode         = atrac3p_decode_frame,
};
