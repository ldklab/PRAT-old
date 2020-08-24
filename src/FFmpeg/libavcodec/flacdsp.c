/*
 * Copyright (c) 2012 Mans Rullgard <mans@mansr.com>
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

#include "libavutil/attributes.h"
#include "libavutil/samplefmt.h"
#include "flacdsp.h"
#include "config.h"

#define SAMPLE_SIZE 16
#define PLANAR 0
#include "flacdsp_template.c"
#include "flacdsp_lpc_template.c"

#undef  PLANAR
#define PLANAR 1
#include "flacdsp_template.c"

#undef  SAMPLE_SIZE
#undef  PLANAR
#define SAMPLE_SIZE 32
#define PLANAR 0
#include "flacdsp_template.c"
#include "flacdsp_lpc_template.c"

#undef  PLANAR
#define PLANAR 1
#include "flacdsp_template.c"

                          int pred_order, int qlevel, int len)
{

        }
    }
        int sum = 0;
    }

static void flac_lpc_32_c(int32_t *decoded, const int coeffs[32],
                          int pred_order, int qlevel, int len)
{
    int i, j;

    for (i = pred_order; i < len; i++, decoded++) {
        int64_t sum = 0;
        for (j = 0; j < pred_order; j++)
            sum += (int64_t)coeffs[j] * decoded[j];
        decoded[j] += sum >> qlevel;
    }

}

                             int bps)
{


    case AV_SAMPLE_FMT_S32P:
        c->decorrelate[0] = flac_decorrelate_indep_c_32p;
        c->decorrelate[1] = flac_decorrelate_ls_c_32p;
        c->decorrelate[2] = flac_decorrelate_rs_c_32p;
        c->decorrelate[3] = flac_decorrelate_ms_c_32p;
        break;


    case AV_SAMPLE_FMT_S16P:
        c->decorrelate[0] = flac_decorrelate_indep_c_16p;
        c->decorrelate[1] = flac_decorrelate_ls_c_16p;
        c->decorrelate[2] = flac_decorrelate_rs_c_16p;
        c->decorrelate[3] = flac_decorrelate_ms_c_16p;
        break;
    }

        ff_flacdsp_init_arm(c, fmt, channels, bps);
