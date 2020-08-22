/*
 * H.264 MP4 to Annex B byte stream format filter
 * Copyright (c) 2007 Benoit Fouet <benoit.fouet@free.fr>
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

#include "libavutil/avassert.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/mem.h"

#include "avcodec.h"
#include "bsf.h"
#include "bsf_internal.h"
#include "bytestream.h"
#include "h264.h"

typedef struct H264BSFContext {
    uint8_t *sps;
    uint8_t *pps;
    int      sps_size;
    int      pps_size;
    uint8_t  length_size;
    uint8_t  new_idr;
    uint8_t  idr_sps_seen;
    uint8_t  idr_pps_seen;
    int      extradata_parsed;
} H264BSFContext;

                          const uint8_t *in, int in_size, int ps, int copy)
{

        }
    }

{



    /* retrieve length coded size */

    /* retrieve sps and pps unit(s) */
        goto pps;
    }


        /* possible overread ok due to padding */
            av_log(ctx, AV_LOG_ERROR, "Global extradata truncated, "
                   "corrupted stream or invalid MP4/AVCC bitstream\n");
            av_free(out);
            return AVERROR_INVALIDDATA;
        }
            return err;
        }
    }


    } else {
        av_log(ctx, AV_LOG_WARNING,
               "Warning: SPS NALU missing or invalid. "
               "The resulting stream may not play.\n");
    }
    } else {
        av_log(ctx, AV_LOG_WARNING,
               "Warning: PPS NALU missing or invalid. "
               "The resulting stream may not play.\n");
    }


}

{

    /* retrieve sps and pps NAL units from extradata */
        av_log(ctx, AV_LOG_VERBOSE,
               "The input looks like it is Annex B already\n");
            return ret;

    } else {
        av_log(ctx, AV_LOG_ERROR, "Invalid extradata size: %d\n", extra_size);
        return AVERROR_INVALIDDATA;
    }

    return 0;
}

{

        return ret;

    /* nothing to filter */
        av_packet_move_ref(opkt, in);
        av_packet_free(&in);
        return 0;
    }


#define LOG_ONCE(...) \
    if (j) \
        av_log(__VA_ARGS__)


            /* possible overread ok due to padding */


            /* This check requires the cast as the right side might
             * otherwise be promoted to an unsigned value. */
            }

                continue;


                sps_seen = new_idr = 1;
                pps_seen = new_idr = 1;
                /* if SPS has not been seen yet, prepend the AVCC one to PPS */
                if (!sps_seen) {
                    if (!s->sps_size) {
                        LOG_ONCE(ctx, AV_LOG_WARNING, "SPS not present in the stream, nor in AVCC, stream may be unreadable\n");
                    } else {
                        count_or_copy(&out, &out_size, s->sps, s->sps_size, -1, j);
                        sps_seen = 1;
                    }
                }
            }

            /* If this is a new IDR picture following an IDR picture, reset the idr flag.
             * Just check first_mb_in_slice to be 0 as this is the simplest solution.
             * This could be checking idr_pic_id instead, but would complexify the parsing. */
                new_idr = 1;

            /* prepend only to the first type 5 NAL unit of an IDR picture, if no sps/pps are already present */
                                  ctx->par_out->extradata_size, -1, j);
                new_idr = 0;
            /* if only SPS has been seen, also insert PPS */
                if (!s->pps_size) {
                    LOG_ONCE(ctx, AV_LOG_WARNING, "PPS not present in the stream, nor in AVCC, stream may be unreadable\n");
                } else {
                    count_or_copy(&out, &out_size, s->pps, s->pps_size, -1, j);
                }
            }

            }


                ret = AVERROR_INVALIDDATA;
                goto fail;
            }
                goto fail;
        }
    }
#undef LOG_ONCE



        goto fail;


}

static void h264_mp4toannexb_flush(AVBSFContext *ctx)
{
    H264BSFContext *s = ctx->priv_data;

    s->idr_sps_seen = 0;
    s->idr_pps_seen = 0;
    s->new_idr      = s->extradata_parsed;
}

static const enum AVCodecID codec_ids[] = {
    AV_CODEC_ID_H264, AV_CODEC_ID_NONE,
};

const AVBitStreamFilter ff_h264_mp4toannexb_bsf = {
    .name           = "h264_mp4toannexb",
    .priv_data_size = sizeof(H264BSFContext),
    .init           = h264_mp4toannexb_init,
    .filter         = h264_mp4toannexb_filter,
    .flush          = h264_mp4toannexb_flush,
    .codec_ids      = codec_ids,
};
