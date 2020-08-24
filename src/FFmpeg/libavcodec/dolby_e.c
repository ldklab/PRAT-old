/*
 * Copyright (C) 2017 foo86
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

#include "libavutil/float_dsp.h"
#include "libavutil/thread.h"
#include "libavutil/mem.h"

#include "internal.h"
#include "get_bits.h"
#include "put_bits.h"
#include "dolby_e.h"
#include "fft.h"

{
        av_log(s->avctx, AV_LOG_ERROR, "Packet too short\n");
        return AVERROR_INVALIDDATA;
    }

}

{
            return ret;
    }
    return 0;
}

{


        av_log(s->avctx, AV_LOG_ERROR, "Packet too short\n");
        return AVERROR_INVALIDDATA;
    }

    case 16:
        break;
    case 20:
        init_put_bits(&pb, s->buffer, sizeof(s->buffer));
        for (i = 0; i < nb_words; i++, src += 3)
            put_bits(&pb, 20, AV_RB24(src) >> 4 ^ key);
        flush_put_bits(&pb);
        break;
    case 24:
        for (i = 0; i < nb_words; i++, src += 3, dst += 3)
            AV_WB24(dst, AV_RB24(src) ^ key);
        break;
    default:
        av_assert0(0);
    }

}

{

        return key;
        return ret;

        av_log(s->avctx, AV_LOG_ERROR, "Invalid metadata size\n");
        return AVERROR_INVALIDDATA;
    }

        return ret;

        av_log(s->avctx, AV_LOG_ERROR, "Invalid program configuration\n");
        return AVERROR_INVALIDDATA;
    }


        av_log(s->avctx, AV_LOG_ERROR, "Invalid frame rate code\n");
        return AVERROR_INVALIDDATA;
    }


    }

        av_log(s->avctx, AV_LOG_ERROR, "Read past end of metadata\n");
        return AVERROR_INVALIDDATA;
    }

}

{
    return 0;
}

{




{

        } else {
        }
    }

}

{
}

{


            }
        }

        }
    }

    }

                         int *exp, int *bap,
                         int fg_spc, int fg_ofs, int msk_mod, int snr_ofs)
{


    }

    }



    }

{

        } else {
        }
    }

        avpriv_report_missing_feature(s->avctx, "Delta bit allocation");
        return AVERROR_PATCHWELCOME;
    }

        memset(c->bap, 0, sizeof(c->bap));
        return 0;
    }

                         fg_spc[i], fg_ofs[i], msk_mod[i], snr_ofs);
        } else {
        }
    }

    return 0;
}

{


                av_log(s->avctx, AV_LOG_ERROR, "Invalid start index\n");
                return AVERROR_INVALIDDATA;
            }


        } else {
        }
    }

    return 0;
}

{





                    } else {
                        else
                    }
                }
            } else {
            }

        }

        }
    }

}

{

        avpriv_report_missing_feature(s->avctx, "Encoder revision %d", s->rev_id[ch]);
        return AVERROR_PATCHWELCOME;
    }

    } else {
            av_log(s->avctx, AV_LOG_ERROR, "Invalid group type code\n");
            return AVERROR_INVALIDDATA;
        }
    }


        }
    }

        return ret;
        return ret;
        return ret;
        return ret;

        av_log(s->avctx, AV_LOG_ERROR, "Read past end of channel %d\n", ch);
        return AVERROR_INVALIDDATA;
    }

    return 0;
}

{

        return key;

            s->channels[seg_id][ch].nb_groups = 0;
            continue;
        }
            return ret;
            if (s->avctx->err_recognition & AV_EF_EXPLODE)
                return ret;
            s->channels[seg_id][ch].nb_groups = 0;
        }
            return ret;
    }

}

{
    return 0;
}

static void imdct_calc(DBEContext *s, DBEGroup *g, float *result, float *values)
{
    FFTContext *imdct = &s->imdct[g->imdct_idx];
    int n   = 1 << imdct_bits_tab[g->imdct_idx];
    int n2  = n >> 1;
    int i;

    switch (g->imdct_phs) {
    case 0:
        imdct->imdct_half(imdct, result, values);
        for (i = 0; i < n2; i++)
            result[n2 + i] = result[n2 - i - 1];
        break;
    case 1:
        imdct->imdct_calc(imdct, result, values);
        break;
    case 2:
        imdct->imdct_half(imdct, result + n2, values);
        for (i = 0; i < n2; i++)
            result[i] = -result[n - i - 1];
        break;
    default:
        av_assert0(0);
    }
}

{


    }


static void apply_gain(DBEContext *s, int begin, int end, float *output)
{
    if (begin == 960 && end == 960)
        return;

    if (begin == end) {
        s->fdsp->vector_fmul_scalar(output, output, gain_tab[end], FRAME_SAMPLES);
    } else {
        float a = gain_tab[begin] * (1.0f / (FRAME_SAMPLES - 1));
        float b = gain_tab[end  ] * (1.0f / (FRAME_SAMPLES - 1));
        int i;

        for (i = 0; i < FRAME_SAMPLES; i++)
            output[i] *= a * (FRAME_SAMPLES - i - 1) + b * i;
    }
}

{

        reorder = ch_reorder_4;
        reorder = ch_reorder_6;
    else if (s->nb_programs == 1 && !(s->avctx->request_channel_layout & AV_CH_LAYOUT_NATIVE))
        reorder = ch_reorder_8;
    else
        reorder = ch_reorder_n;

        return ret;

    }

    return 0;
}

                                int *got_frame_ptr, AVPacket *avpkt)
{

        return AVERROR_INVALIDDATA;

        s->word_bits = 24;
        s->word_bits = 20;
    } else {
        av_log(avctx, AV_LOG_ERROR, "Invalid frame header\n");
        return AVERROR_INVALIDDATA;
    }


        return ret;

        av_log(avctx, AV_LOG_WARNING, "Stream has %d programs (configuration %d), "
               "channels will be output in native order.\n", s->nb_programs, s->prog_conf);
        s->multi_prog_warned = 1;
    }

    case 4:
        avctx->channel_layout = AV_CH_LAYOUT_4POINT0;
        break;
    case 8:
        avctx->channel_layout = AV_CH_LAYOUT_7POINT1;
        break;
    }


        return ret;
        return ret;
        return ret;
        return ret;
        return ret;
        return ret;
        return ret;

}

static av_cold void dolby_e_flush(AVCodecContext *avctx)
{
    DBEContext *s = avctx->priv_data;

    memset(s->history, 0, sizeof(s->history));
}

{


}


{


    }


    }


    }


    // short 1

    // start

    // short 2

    // short 3

    // bridge

    // long

    // reverse start

    // reverse short 2

    // reverse short 3

    // reverse bridge

{

        return AVERROR_UNKNOWN;

            return AVERROR(ENOMEM);

        return AVERROR(ENOMEM);

}

AVCodec ff_dolby_e_decoder = {
    .name           = "dolby_e",
    .long_name      = NULL_IF_CONFIG_SMALL("Dolby E"),
    .type           = AVMEDIA_TYPE_AUDIO,
    .id             = AV_CODEC_ID_DOLBY_E,
    .priv_data_size = sizeof(DBEContext),
    .init           = dolby_e_init,
    .decode         = dolby_e_decode_frame,
    .close          = dolby_e_close,
    .flush          = dolby_e_flush,
    .capabilities   = AV_CODEC_CAP_DR1 | AV_CODEC_CAP_CHANNEL_CONF,
    .sample_fmts    = (const enum AVSampleFormat[]) { AV_SAMPLE_FMT_FLTP, AV_SAMPLE_FMT_NONE },
    .caps_internal  = FF_CODEC_CAP_INIT_THREADSAFE | FF_CODEC_CAP_INIT_CLEANUP,
};
