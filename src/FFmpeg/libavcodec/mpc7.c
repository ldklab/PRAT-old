/*
 * Musepack SV7 decoder
 * Copyright (c) 2006 Konstantin Shishkov
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
 * MPEG Audio Layer 1/2 -like codec with frames of 1152 samples
 * divided into 32 subbands.
 */

#include "libavutil/channel_layout.h"
#include "libavutil/internal.h"
#include "libavutil/lfg.h"
#include "avcodec.h"
#include "get_bits.h"
#include "internal.h"
#include "mpegaudiodsp.h"

#include "mpc.h"
#include "mpc7data.h"

static VLC scfi_vlc, dscf_vlc, hdr_vlc, quant_vlc[MPC7_QUANT_VLC_TABLES][2];

static const uint16_t quant_offsets[MPC7_QUANT_VLC_TABLES*2 + 1] =
{
       0, 512, 1024, 1536, 2052, 2564, 3076, 3588, 4100, 4612, 5124,
       5636, 6164, 6676, 7224
};


{


    /* Musepack SV7 is always stereo */
        avpriv_request_sample(avctx, "%d channels", avctx->channels);
        return AVERROR_PATCHWELCOME;
    }

        av_log(avctx, AV_LOG_ERROR, "Too small extradata size (%i)!\n", avctx->extradata_size);
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "Too many bands: %i\n", c->maxbands);
        return AVERROR_INVALIDDATA;
    }
            c->IS, c->MSS, c->gapless, c->lastframelen, c->maxbands);


                &mpc7_scfi[1], 2, 1,
                &mpc7_scfi[0], 2, 1, INIT_VLC_USE_NEW_STATIC))) {
        av_log(avctx, AV_LOG_ERROR, "Cannot init SCFI VLC\n");
        return ret;
    }
                &mpc7_dscf[1], 2, 1,
                &mpc7_dscf[0], 2, 1, INIT_VLC_USE_NEW_STATIC))) {
        av_log(avctx, AV_LOG_ERROR, "Cannot init DSCF VLC\n");
        return ret;
    }
                &mpc7_hdr[1], 2, 1,
                &mpc7_hdr[0], 2, 1, INIT_VLC_USE_NEW_STATIC))) {
        av_log(avctx, AV_LOG_ERROR, "Cannot init HDR VLC\n");
        return ret;
    }
                        &mpc7_quant_vlc[i][j][1], 4, 2,
                        &mpc7_quant_vlc[i][j][0], 4, 2, INIT_VLC_USE_NEW_STATIC))) {
                av_log(avctx, AV_LOG_ERROR, "Cannot init QUANT VLC %i,%i\n",i,j);
                return ret;
            }
        }
    }

}

/**
 * Fill samples for given subband
 */
{
    case -1:
        for(i = 0; i < SAMPLES_PER_BAND; i++){
            *dst++ = (av_lfg_get(&c->rnd) & 0x3FC) - 510;
        }
        break;
    case 1:
        }
        break;
    case 2:
        }
        break;
    case  3: case  4: case  5: case  6: case  7:
        break;
    case 13: case 14: case 15: case 16: case 17:
        break;
    default: // case 0 and -2..-17
        return;
    }
}

{
}

                             int *got_frame_ptr, AVPacket *avpkt)
{


        av_log(avctx, AV_LOG_ERROR, "packet size is too small (%i bytes)\n",
               avpkt->size);
        return AVERROR_INVALIDDATA;
    }
        av_log(avctx, AV_LOG_WARNING, "packet size is not a multiple of 4. "
               "extra bytes at the end will be skipped.\n");
    }


    /* get output buffer */
        return ret;

        return AVERROR(ENOMEM);
                      buf_size >> 2);
        return ret;

    /* read subband indexes */
                av_log(avctx, AV_LOG_ERROR, "subband index invalid\n");
                return AVERROR_INVALIDDATA;
            }
        }

        }
    }
    /* get scale indexes coding method */
    /* get scale indexes */
                }
            }
        }
    }
    /* get quantizers */

        frame->nb_samples = c->lastframelen;

        av_log(avctx, AV_LOG_ERROR, "Error decoding frame: used %i of %i bits\n", bits_used, bits_avail);
        return AVERROR_INVALIDDATA;
    }
        c->frames_to_skip--;
        *got_frame_ptr = 0;
        return avpkt->size;
    }


}

static void mpc7_decode_flush(AVCodecContext *avctx)
{
    MPCContext *c = avctx->priv_data;

    memset(c->oldDSCF, 0, sizeof(c->oldDSCF));
    c->frames_to_skip = 32;
}

{
}

AVCodec ff_mpc7_decoder = {
    .name           = "mpc7",
    .long_name      = NULL_IF_CONFIG_SMALL("Musepack SV7"),
    .type           = AVMEDIA_TYPE_AUDIO,
    .id             = AV_CODEC_ID_MUSEPACK7,
    .priv_data_size = sizeof(MPCContext),
    .init           = mpc7_decode_init,
    .close          = mpc7_decode_close,
    .decode         = mpc7_decode_frame,
    .flush          = mpc7_decode_flush,
    .capabilities   = AV_CODEC_CAP_DR1,
    .sample_fmts    = (const enum AVSampleFormat[]) { AV_SAMPLE_FMT_S16P,
                                                      AV_SAMPLE_FMT_NONE },
};
