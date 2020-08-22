/*
 * AV1 parser
 *
 * Copyright (C) 2018 James Almer <jamrial@gmail.com>
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

#include "av1_parse.h"
#include "cbs.h"
#include "cbs_av1.h"
#include "internal.h"
#include "parser.h"

typedef struct AV1ParseContext {
    CodedBitstreamContext *cbc;
    CodedBitstreamFragment temporal_unit;
    int parsed_extradata;
} AV1ParseContext;

static const enum AVPixelFormat pix_fmts_8bit[2][2] = {
    { AV_PIX_FMT_YUV444P, AV_PIX_FMT_NONE },
    { AV_PIX_FMT_YUV422P, AV_PIX_FMT_YUV420P },
};
static const enum AVPixelFormat pix_fmts_10bit[2][2] = {
    { AV_PIX_FMT_YUV444P10, AV_PIX_FMT_NONE },
    { AV_PIX_FMT_YUV422P10, AV_PIX_FMT_YUV420P10 },
};
static const enum AVPixelFormat pix_fmts_12bit[2][2] = {
    { AV_PIX_FMT_YUV444P12, AV_PIX_FMT_NONE },
    { AV_PIX_FMT_YUV422P12, AV_PIX_FMT_YUV420P12 },
};

static const enum AVPixelFormat pix_fmts_rgb[3] = {
    AV_PIX_FMT_GBRP, AV_PIX_FMT_GBRP10, AV_PIX_FMT_GBRP12,
};

                            AVCodecContext *avctx,
                            const uint8_t **out_data, int *out_size,
                            const uint8_t *data, int size)
{





            av_log(avctx, AV_LOG_WARNING, "Failed to parse extradata.\n");
        }

    }

        av_log(avctx, AV_LOG_ERROR, "Failed to parse temporal unit.\n");
        goto end;
    }

        av_log(avctx, AV_LOG_ERROR, "No sequence header available\n");
        goto end;
    }



        else



                av_log(avctx, AV_LOG_ERROR, "Invalid reference frame\n");
                goto end;
            }


        } else {

        }

        case AV1_FRAME_INTRA_ONLY:
        }
    }

    case 12:
        ctx->format = color->mono_chrome ? AV_PIX_FMT_GRAY12
                                         : pix_fmts_12bit[color->subsampling_x][color->subsampling_y];
        break;
    }

        color->matrix_coefficients       == AVCOL_SPC_RGB &&
        color->color_primaries           == AVCOL_PRI_BT709 &&
        color->transfer_characteristics  == AVCOL_TRC_IEC61966_2_1)
        ctx->format = pix_fmts_rgb[color->high_bitdepth + color->twelve_bit];




            goto end;
    }




}

static const CodedBitstreamUnitType decompose_unit_types[] = {
    AV1_OBU_TEMPORAL_DELIMITER,
    AV1_OBU_SEQUENCE_HEADER,
    AV1_OBU_FRAME_HEADER,
    AV1_OBU_TILE_GROUP,
    AV1_OBU_FRAME,
};

{

        return ret;


}

{


static int av1_parser_split(AVCodecContext *avctx,
                            const uint8_t *buf, int buf_size)
{
    AV1OBU obu;
    const uint8_t *ptr = buf, *end = buf + buf_size;

    while (ptr < end) {
        int len = ff_av1_extract_obu(&obu, ptr, buf_size, avctx);
        if (len < 0)
            break;

        if (obu.type == AV1_OBU_FRAME_HEADER ||
            obu.type == AV1_OBU_FRAME) {
            return ptr - buf;
        }
        ptr      += len;
        buf_size -= len;
    }

    return 0;
}

AVCodecParser ff_av1_parser = {
    .codec_ids      = { AV_CODEC_ID_AV1 },
    .priv_data_size = sizeof(AV1ParseContext),
    .parser_init    = av1_parser_init,
    .parser_close   = av1_parser_close,
    .parser_parse   = av1_parser_parse,
    .split          = av1_parser_split,
};
