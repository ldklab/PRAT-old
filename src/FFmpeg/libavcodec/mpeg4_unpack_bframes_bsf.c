/*
 * Bitstream filter for unpacking DivX-style packed B-frames in MPEG-4 (divx_packed)
 * Copyright (c) 2015 Andreas Cadhalpun <Andreas.Cadhalpun@googlemail.com>
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
#include "internal.h"
#include "mpeg4video.h"

typedef struct UnpackBFramesBSFContext {
    AVBufferRef *b_frame_ref;
} UnpackBFramesBSFContext;

/* determine the position of the packed marker in the userdata,
 * the number of VOPs and the position of the second VOP */
                        int *pos_p, int *nb_vop, int *pos_vop2) {


            /* check if the (DivX) userdata string ends with 'p' (packed) */
                }
            }
            }
        }
    }

{

        return ret;


            av_log(ctx, AV_LOG_WARNING,
                   "Missing one N-VOP packet, discarding one B-frame.\n");
            av_buffer_unref(&s->b_frame_ref);
        }
        /* store a reference to the packed B-frame's data in the BSFContext */
            ret = AVERROR(ENOMEM);
            goto fail;
        }
    }

        av_log(ctx, AV_LOG_WARNING,
       "Found %d VOP headers in one packet, only unpacking one.\n", nb_vop);
    }


        /* make tmp accurately reflect the packet's data */

        /* replace data in packet with stored data */

        /* store reference to data into BSFContext */

            /* N-VOP - discard stored data */
        }
        /* use first frame of the packet */
            goto fail;
        /* remove 'p' (packed) from the end of the (DivX) userdata string */
    } else {
        /* use packet as is */

        av_packet_unref(pkt);

    return ret;
}

{
                   "Updating DivX userdata (remove trailing 'p') in extradata.\n");
        }
    }

}

{

static const enum AVCodecID codec_ids[] = {
    AV_CODEC_ID_MPEG4, AV_CODEC_ID_NONE,
};

const AVBitStreamFilter ff_mpeg4_unpack_bframes_bsf = {
    .name           = "mpeg4_unpack_bframes",
    .priv_data_size = sizeof(UnpackBFramesBSFContext),
    .init           = mpeg4_unpack_bframes_init,
    .filter         = mpeg4_unpack_bframes_filter,
    .flush          = mpeg4_unpack_bframes_close_flush,
    .close          = mpeg4_unpack_bframes_close_flush,
    .codec_ids      = codec_ids,
};
