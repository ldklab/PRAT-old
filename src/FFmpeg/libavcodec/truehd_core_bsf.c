/*
 * Copyright (c) 2018 Paul B Mahol
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
#include "get_bits.h"
#include "mlp_parse.h"
#include "mlp.h"

typedef struct AccessUnit {
    uint8_t bits[4];
    uint16_t offset;
    uint16_t optional;
} AccessUnit;

typedef struct TrueHDCoreContext {
    MLPHeaderInfo hdr;
} TrueHDCoreContext;

{

        return ret;

        ret = AVERROR_INVALIDDATA;
        goto fail;
    }

        ret = AVERROR_INVALIDDATA;
        goto fail;
    }

        goto fail;

            goto fail;
        have_header = 1;
    }

        ret = AVERROR_INVALIDDATA;
        goto fail;
    }


        }

        }
    }



        }


            goto fail;







            }
        }



    }

fail:
        av_packet_unref(pkt);

    return ret;
}

static void truehd_core_flush(AVBSFContext *ctx)
{
    TrueHDCoreContext *s = ctx->priv_data;
    memset(&s->hdr, 0, sizeof(s->hdr));
}

static const enum AVCodecID codec_ids[] = {
    AV_CODEC_ID_TRUEHD, AV_CODEC_ID_NONE,
};

const AVBitStreamFilter ff_truehd_core_bsf = {
    .name           = "truehd_core",
    .priv_data_size = sizeof(TrueHDCoreContext),
    .filter         = truehd_core_filter,
    .flush          = truehd_core_flush,
    .codec_ids      = codec_ids,
};
