/*
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
 * This bitstream filter splits VP9 superframes into packets containing
 * just one frame.
 */

#include <stddef.h>

#include "bsf.h"
#include "bsf_internal.h"
#include "bytestream.h"
#include "get_bits.h"

typedef struct VP9SFSplitContext {
    AVPacket *buffer_pkt;

    int nb_frames;
    int next_frame;
    size_t next_frame_offset;
    int sizes[8];
} VP9SFSplitContext;

{

            return ret;



                                 nb_frames * length_size);

                    int frame_size = 0;

                        av_log(ctx, AV_LOG_ERROR,
                               "Invalid frame size in a superframe: %d\n", frame_size);
                        ret = AVERROR(EINVAL);
                        goto fail;
                    }
                }
            }
        }
    }


            goto fail;




            goto fail;

        }


    } else {
    }

    return 0;
fail:
    if (ret < 0)
        av_packet_unref(out);
    av_packet_unref(s->buffer_pkt);
    return ret;
}

{

        return AVERROR(ENOMEM);

    return 0;
}

static void vp9_superframe_split_flush(AVBSFContext *ctx)
{
    VP9SFSplitContext *s = ctx->priv_data;
    av_packet_unref(s->buffer_pkt);
}

{

const AVBitStreamFilter ff_vp9_superframe_split_bsf = {
    .name = "vp9_superframe_split",
    .priv_data_size = sizeof(VP9SFSplitContext),
    .init           = vp9_superframe_split_init,
    .flush          = vp9_superframe_split_flush,
    .close          = vp9_superframe_split_uninit,
    .filter         = vp9_superframe_split_filter,
    .codec_ids      = (const enum AVCodecID []){ AV_CODEC_ID_VP9, AV_CODEC_ID_NONE },
};
