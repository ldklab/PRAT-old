/*
 * HAPQA extract bitstream filter
 * Copyright (c) 2017 Jokyo Images
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
 * HAPQA extract bitstream filter
 * extract one of the two textures of the HAQA
 */

#include "bsf.h"
#include "bsf_internal.h"
#include "bytestream.h"
#include "hap.h"

typedef struct HapqaExtractContext {
    const AVClass *class;
    int texture;/* index of the texture to keep (0 for rgb or 1 for alpha) */
} HapqaExtractContext;

    {
        return 1; /* the texture is the one to keep */
    } else {
    }
}

{

        return ret;

        goto fail;

        av_log(bsf, AV_LOG_ERROR, "Invalid section type for HAPQA %#04x.\n", section_type & 0x0F);
        ret = AVERROR_INVALIDDATA;
        goto fail;
    }



        goto fail;


            goto fail;


            av_log(bsf, AV_LOG_ERROR, "No valid texture found.\n");
            ret = AVERROR_INVALIDDATA;
            goto fail;
        }
    }


        av_packet_unref(pkt);
    return ret;
}

static const enum AVCodecID codec_ids[] = {
    AV_CODEC_ID_HAP, AV_CODEC_ID_NONE,
};

#define OFFSET(x) offsetof(HapqaExtractContext, x)
#define FLAGS (AV_OPT_FLAG_VIDEO_PARAM | AV_OPT_FLAG_BSF_PARAM)
static const AVOption options[] = {
    { "texture", "texture to keep", OFFSET(texture), AV_OPT_TYPE_INT, { .i64 = 0 }, 0, 1, FLAGS, "texture" },
        { "color", "keep HapQ texture",         0, AV_OPT_TYPE_CONST, { .i64 = 0 }, 0, 0, FLAGS, "texture" },
        { "alpha", "keep HapAlphaOnly texture", 0, AV_OPT_TYPE_CONST, { .i64 = 1 }, 0, 0, FLAGS, "texture" },
    { NULL },
};

static const AVClass hapqa_extract_class = {
    .class_name = "hapqa_extract_bsf",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
};

const AVBitStreamFilter ff_hapqa_extract_bsf = {
    .name       = "hapqa_extract",
    .filter     = hapqa_extract,
    .priv_data_size = sizeof(HapqaExtractContext),
    .priv_class = &hapqa_extract_class,
    .codec_ids  = codec_ids,
};
