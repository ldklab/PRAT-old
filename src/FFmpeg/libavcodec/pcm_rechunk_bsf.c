/*
 * Copyright (c) 2020 Marton Balint
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

#include "avcodec.h"
#include "bsf_internal.h"
#include "libavutil/avassert.h"
#include "libavutil/opt.h"

typedef struct PCMContext {
    const AVClass *class;

    int nb_out_samples;
    int pad;
    AVRational frame_rate;

    AVPacket *in_pkt;
    AVPacket *out_pkt;
    int sample_size;
    int64_t n;
} PCMContext;

{

        return AVERROR(EINVAL);


    } else {
    }
        return AVERROR(EINVAL);

        return AVERROR(ENOMEM);

    return 0;
}

{

static void flush(AVBSFContext *ctx)
{
    PCMContext *s = ctx->priv_data;
    av_packet_unref(s->in_pkt);
    av_packet_unref(s->out_pkt);
    s->n = 0;
}

{
}

{
}

static int get_next_nb_samples(AVBSFContext *ctx)
{
    PCMContext *s = ctx->priv_data;
    if (s->frame_rate.num) {
        AVRational sr = av_make_q(ctx->par_in->sample_rate, 1);
        return av_rescale_q(s->n + 1, sr, s->frame_rate) - av_rescale_q(s->n, sr, s->frame_rate);
    } else {
        return s->nb_out_samples;
    }
}

{

                        return ret;
                        av_packet_unref(s->out_pkt);
                        return ret;
                    }
                }
                }
                ret = av_packet_ref(pkt, s->in_pkt);
                if (ret < 0)
                    return ret;
                pkt->size = data_size;
                drain_packet(s->in_pkt, data_size, nb_samples);
                return send_packet(s, nb_samples, pkt);
            } else {
            }
        }

            } else {
                nb_samples = s->out_pkt->size / s->sample_size;
            }
        }

    return ret;
}

#define OFFSET(x) offsetof(PCMContext, x)
#define FLAGS (AV_OPT_FLAG_AUDIO_PARAM | AV_OPT_FLAG_BSF_PARAM)
static const AVOption options[] = {
    { "nb_out_samples", "set the number of per-packet output samples", OFFSET(nb_out_samples),   AV_OPT_TYPE_INT, {.i64=1024}, 1, INT_MAX, FLAGS },
    { "n",              "set the number of per-packet output samples", OFFSET(nb_out_samples),   AV_OPT_TYPE_INT, {.i64=1024}, 1, INT_MAX, FLAGS },
    { "pad",            "pad last packet with zeros",                  OFFSET(pad),             AV_OPT_TYPE_BOOL, {.i64=1} ,   0,       1, FLAGS },
    { "p",              "pad last packet with zeros",                  OFFSET(pad),             AV_OPT_TYPE_BOOL, {.i64=1} ,   0,       1, FLAGS },
    { "frame_rate",     "set number of packets per second",            OFFSET(frame_rate),  AV_OPT_TYPE_RATIONAL, {.dbl=0},    0, INT_MAX, FLAGS },
    { "r",              "set number of packets per second",            OFFSET(frame_rate),  AV_OPT_TYPE_RATIONAL, {.dbl=0},    0, INT_MAX, FLAGS },
    { NULL },
};

static const AVClass pcm_rechunk_class = {
    .class_name = "pcm_rechunk_bsf",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
};

static const enum AVCodecID codec_ids[] = {
    AV_CODEC_ID_PCM_S16LE,
    AV_CODEC_ID_PCM_S16BE,
    AV_CODEC_ID_PCM_S8,
    AV_CODEC_ID_PCM_S32LE,
    AV_CODEC_ID_PCM_S32BE,
    AV_CODEC_ID_PCM_S24LE,
    AV_CODEC_ID_PCM_S24BE,
    AV_CODEC_ID_PCM_F32BE,
    AV_CODEC_ID_PCM_F32LE,
    AV_CODEC_ID_PCM_F64BE,
    AV_CODEC_ID_PCM_F64LE,
    AV_CODEC_ID_PCM_S64LE,
    AV_CODEC_ID_PCM_S64BE,
    AV_CODEC_ID_PCM_F16LE,
    AV_CODEC_ID_PCM_F24LE,
    AV_CODEC_ID_NONE,
};

const AVBitStreamFilter ff_pcm_rechunk_bsf = {
    .name           = "pcm_rechunk",
    .priv_data_size = sizeof(PCMContext),
    .priv_class     = &pcm_rechunk_class,
    .filter         = rechunk_filter,
    .init           = init,
    .flush          = flush,
    .close          = uninit,
    .codec_ids      = codec_ids,
};
