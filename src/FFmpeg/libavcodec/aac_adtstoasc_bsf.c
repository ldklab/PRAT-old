/*
 * MPEG-2/4 AAC ADTS to MPEG-4 Audio Specific Configuration bitstream filter
 * Copyright (c) 2009 Alex Converse <alex.converse@gmail.com>
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

#include "adts_header.h"
#include "adts_parser.h"
#include "bsf.h"
#include "bsf_internal.h"
#include "put_bits.h"
#include "get_bits.h"
#include "mpeg4audio.h"
#include "internal.h"

typedef struct AACBSFContext {
    int first_frame_done;
} AACBSFContext;

/**
 * This filter creates an MPEG-4 AudioSpecificConfig from an MPEG-2/4
 * ADTS header and removes the ADTS header.
 */
{


        return ret;

        return 0;

        goto packet_too_small;


        av_log(bsfc, AV_LOG_ERROR, "Error parsing ADTS frame header!\n");
        ret = AVERROR_INVALIDDATA;
        goto fail;
    }

        avpriv_report_missing_feature(bsfc,
                                      "Multiple RDBs per frame with CRC");
        ret = AVERROR_PATCHWELCOME;
        goto fail;
    }

        goto packet_too_small;


            init_get_bits(&gb, pkt->data, pkt->size * 8);
            if (get_bits(&gb, 3) != 5) {
                avpriv_report_missing_feature(bsfc,
                                              "PCE-based channel configuration "
                                              "without PCE as first syntax "
                                              "element");
                ret = AVERROR_PATCHWELCOME;
                goto fail;
            }
            init_put_bits(&pb, pce_data, MAX_PCE_SIZE);
            pce_size = ff_copy_pce_data(&pb, &gb) / 8;
            flush_put_bits(&pb);
            pkt->size -= get_bits_count(&gb)/8;
            pkt->data += get_bits_count(&gb)/8;
        }

                                            2 + pce_size);
            ret = AVERROR(ENOMEM);
            goto fail;
        }

            memcpy(extradata + 2, pce_data, pce_size);
        }

    }

    return 0;

packet_too_small:
    av_log(bsfc, AV_LOG_ERROR, "Input packet too small\n");
    ret = AVERROR_INVALIDDATA;
fail:
    av_packet_unref(pkt);
    return ret;
}

{
    /* Validate the extradata if the stream is already MPEG-4 AudioSpecificConfig */
        MPEG4AudioConfig mp4ac;
        int ret = avpriv_mpeg4audio_get_config2(&mp4ac, ctx->par_in->extradata,
                                                ctx->par_in->extradata_size, 1, ctx);
        if (ret < 0) {
            av_log(ctx, AV_LOG_ERROR, "Error parsing AudioSpecificConfig extradata!\n");
            return ret;
        }
    }

    return 0;
}

static const enum AVCodecID codec_ids[] = {
    AV_CODEC_ID_AAC, AV_CODEC_ID_NONE,
};

const AVBitStreamFilter ff_aac_adtstoasc_bsf = {
    .name           = "aac_adtstoasc",
    .priv_data_size = sizeof(AACBSFContext),
    .init           = aac_adtstoasc_init,
    .filter         = aac_adtstoasc_filter,
    .codec_ids      = codec_ids,
};
