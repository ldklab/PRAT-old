/*
 * Copyright (c) 2019 James Almer <jamrial@gmail.com>
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

#include "bsf.h"
#include "bsf_internal.h"
#include "cbs.h"
#include "cbs_av1.h"

typedef struct AV1FMergeContext {
    CodedBitstreamContext *input;
    CodedBitstreamContext *output;
    CodedBitstreamFragment frag[2];
    AVPacket *pkt, *in;
    int idx;
} AV1FMergeContext;

static void av1_frame_merge_flush(AVBSFContext *bsf)
{
    AV1FMergeContext *ctx = bsf->priv_data;

    ff_cbs_fragment_reset(&ctx->frag[0]);
    ff_cbs_fragment_reset(&ctx->frag[1]);
    av_packet_unref(ctx->in);
    av_packet_unref(ctx->pkt);
}

{

        return err;
    }

        av_log(bsf, AV_LOG_ERROR, "Failed to read packet.\n");
        goto fail;
    }

        av_log(bsf, AV_LOG_ERROR, "No OBU in packet.\n");
        err = AVERROR_INVALIDDATA;
        goto fail;
    }

        av_log(bsf, AV_LOG_ERROR, "Missing Temporal Delimiter.\n");
        err = AVERROR_INVALIDDATA;
        goto fail;
    }

        if (frag->units[i].type == AV1_OBU_TEMPORAL_DELIMITER) {
            av_log(bsf, AV_LOG_ERROR, "Temporal Delimiter in the middle of a packet.\n");
            err = AVERROR_INVALIDDATA;
            goto fail;
        }
    }

            av_log(bsf, AV_LOG_ERROR, "Failed to write packet.\n");
            goto fail;
        }

        // Swap fragment index, to avoid copying fragment references.
    } else {
                goto fail;
        }

        err = AVERROR(EAGAIN);
    }

    // Buffer packets with timestamps. There should be at most one per TU, be it split or not.
        av_packet_move_ref(buffer_pkt, in);
    else


        av1_frame_merge_flush(bsf);

    return err;
}

{

        return AVERROR(ENOMEM);

        return err;

}

{


static const enum AVCodecID av1_frame_merge_codec_ids[] = {
    AV_CODEC_ID_AV1, AV_CODEC_ID_NONE,
};

const AVBitStreamFilter ff_av1_frame_merge_bsf = {
    .name           = "av1_frame_merge",
    .priv_data_size = sizeof(AV1FMergeContext),
    .init           = av1_frame_merge_init,
    .flush          = av1_frame_merge_flush,
    .close          = av1_frame_merge_close,
    .filter         = av1_frame_merge_filter,
    .codec_ids      = av1_frame_merge_codec_ids,
};
