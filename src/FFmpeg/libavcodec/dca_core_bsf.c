/*
 * Copyright (c) 2016 Paul B Mahol
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
#include "bytestream.h"
#include "dca_syncwords.h"

{

        return ret;


    case DCA_SYNCWORD_CORE_BE:
    }

    }

    return 0;
}

static const enum AVCodecID codec_ids[] = {
    AV_CODEC_ID_DTS, AV_CODEC_ID_NONE,
};

const AVBitStreamFilter ff_dca_core_bsf = {
    .name      = "dca_core",
    .filter    = dca_core_filter,
    .codec_ids = codec_ids,
};
