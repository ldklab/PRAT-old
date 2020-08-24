/*
 * HEVC MP4 to Annex B byte stream format filter
 * copyright (c) 2015 Anton Khirnov
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

#include <string.h>

#include "libavutil/intreadwrite.h"
#include "libavutil/mem.h"

#include "avcodec.h"
#include "bsf.h"
#include "bsf_internal.h"
#include "bytestream.h"
#include "hevc.h"

#define MIN_HEVCC_LENGTH 23

typedef struct HEVCBSFContext {
    uint8_t  length_size;
    int      extradata_parsed;
} HEVCBSFContext;

{





        if (!(type == HEVC_NAL_VPS || type == HEVC_NAL_SPS || type == HEVC_NAL_PPS ||
            av_log(ctx, AV_LOG_ERROR, "Invalid NAL unit type in extradata: %d\n",
                   type);
            ret = AVERROR_INVALIDDATA;
            goto fail;
        }


                ret = AVERROR_INVALIDDATA;
                goto fail;
            }
                goto fail;

        }
    }


        av_log(ctx, AV_LOG_WARNING, "No parameter sets in the extradata\n");

    return length_size;
fail:
    av_freep(&new_extradata);
    return ret;
}

{

        av_log(ctx, AV_LOG_VERBOSE,
               "The input looks like it is Annex B already\n");
    } else {
            return ret;
    }

    return 0;
}

{


        return ret;

        av_packet_move_ref(out, in);
        av_packet_free(&in);
        return 0;
    }



            ret = AVERROR_INVALIDDATA;
            goto fail;
        }

            ret = AVERROR_INVALIDDATA;
            goto fail;
        }


        /* prepend extradata to IRAP frames */

            ret = AVERROR_INVALIDDATA;
            goto fail;
        }


            goto fail;

    }

        goto fail;

        av_packet_unref(out);

}

static const enum AVCodecID codec_ids[] = {
    AV_CODEC_ID_HEVC, AV_CODEC_ID_NONE,
};

const AVBitStreamFilter ff_hevc_mp4toannexb_bsf = {
    .name           = "hevc_mp4toannexb",
    .priv_data_size = sizeof(HEVCBSFContext),
    .init           = hevc_mp4toannexb_init,
    .filter         = hevc_mp4toannexb_filter,
    .codec_ids      = codec_ids,
};
