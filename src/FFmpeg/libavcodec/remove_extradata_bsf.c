/*
 * copyright (c) 2006 Michael Niedermayer <michaelni@gmx.at>
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

#include "libavutil/log.h"
#include "libavutil/opt.h"

#include "avcodec.h"
#include "bsf.h"
#include "bsf_internal.h"

enum RemoveFreq {
    REMOVE_FREQ_KEYFRAME,
    REMOVE_FREQ_ALL,
    REMOVE_FREQ_NONKEYFRAME,
};

typedef struct RemoveExtradataContext {
    const AVClass *class;
    int freq;

    AVCodecParserContext *parser;
    AVCodecContext *avctx;
} RemoveExtradataContext;

{


        return ret;

        }
    }

    return 0;
}

{


            return AVERROR(ENOMEM);

            return ret;
    }

    return 0;
}

{


#define OFFSET(x) offsetof(RemoveExtradataContext, x)
#define FLAGS (AV_OPT_FLAG_VIDEO_PARAM|AV_OPT_FLAG_BSF_PARAM)
static const AVOption options[] = {
    { "freq", NULL, OFFSET(freq), AV_OPT_TYPE_INT, { .i64 = REMOVE_FREQ_KEYFRAME }, REMOVE_FREQ_KEYFRAME, REMOVE_FREQ_NONKEYFRAME, FLAGS, "freq" },
        { "k",        NULL, 0, AV_OPT_TYPE_CONST, { .i64 = REMOVE_FREQ_NONKEYFRAME }, .flags = FLAGS, .unit = "freq" },
        { "keyframe", NULL, 0, AV_OPT_TYPE_CONST, { .i64 = REMOVE_FREQ_KEYFRAME }, .flags = FLAGS, .unit = "freq" },
        { "e",        NULL, 0, AV_OPT_TYPE_CONST, { .i64 = REMOVE_FREQ_ALL      }, .flags = FLAGS, .unit = "freq" },
        { "all",      NULL, 0, AV_OPT_TYPE_CONST, { .i64 = REMOVE_FREQ_ALL      }, .flags = FLAGS, .unit = "freq" },
    { NULL },
};

static const AVClass remove_extradata_class = {
    .class_name = "remove_extradata",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
};

const AVBitStreamFilter ff_remove_extradata_bsf = {
    .name           = "remove_extra",
    .priv_data_size = sizeof(RemoveExtradataContext),
    .priv_class     = &remove_extradata_class,
    .init           = remove_extradata_init,
    .close          = remove_extradata_close,
    .filter         = remove_extradata,
};
