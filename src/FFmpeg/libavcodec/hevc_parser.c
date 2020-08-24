/*
 * HEVC Annex B format parser
 *
 * Copyright (C) 2012 - 2013 Guillaume Martres
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

#include "libavutil/common.h"

#include "golomb.h"
#include "hevc.h"
#include "hevc_parse.h"
#include "hevc_ps.h"
#include "hevc_sei.h"
#include "h2645_parse.h"
#include "internal.h"
#include "parser.h"

#define START_CODE 0x000001 ///< start_code_prefix_one_3bytes

#define IS_IRAP_NAL(nal) (nal->type >= 16 && nal->type <= 23)
#define IS_IDR_NAL(nal) (nal->type == HEVC_NAL_IDR_W_RADL || nal->type == HEVC_NAL_IDR_N_LP)

typedef struct HEVCParserContext {
    ParseContext pc;

    H2645Packet pkt;
    HEVCParamSets ps;
    HEVCSEI sei;

    int is_avc;
    int nal_length_size;
    int parsed_extradata;

    int poc;
    int pocTid0;
} HEVCParserContext;

                                   AVCodecContext *avctx)
{



    }

    }

        av_log(avctx, AV_LOG_ERROR, "SPS id out of range: %d\n", ps->pps->sps_id);
        return AVERROR_INVALIDDATA;
    }
    }


    }

                  num, den, 1 << 30);

        unsigned int slice_segment_addr;
        int slice_address_length;

        if (ps->pps->dependent_slice_segments_enabled_flag)
            dependent_slice_segment_flag = get_bits1(gb);
        else
            dependent_slice_segment_flag = 0;

        slice_address_length = av_ceil_log2_c(ps->sps->ctb_width *
                                              ps->sps->ctb_height);
        slice_segment_addr = get_bitsz(gb, slice_address_length);
        if (slice_segment_addr >= ps->sps->ctb_width * ps->sps->ctb_height) {
            av_log(avctx, AV_LOG_ERROR, "Invalid slice segment address: %u.\n",
                   slice_segment_addr);
            return AVERROR_INVALIDDATA;
        }
    } else
        dependent_slice_segment_flag = 0;

    if (dependent_slice_segment_flag)
        return 0; /* break; */


          slice_type == HEVC_SLICE_B)) {
        av_log(avctx, AV_LOG_ERROR, "Unknown slice type: %d.\n",
               slice_type);
        return AVERROR_INVALIDDATA;
    }
                                                AV_PICTURE_TYPE_I;


        skip_bits(gb, 2);   // colour_plane_id

    } else

        nal->type != HEVC_NAL_RASL_R)

    return 1; /* no need to evaluate the rest */
}

/**
 * Parse NAL units of found picture and decode some basic information.
 *
 * @param s parser context.
 * @param avctx codec context.
 * @param buf buffer with field/frame data.
 * @param buf_size size of the buffer.
 */
                           int buf_size, AVCodecContext *avctx)
{

    /* set some sane default values */


                                ctx->nal_length_size, AV_CODEC_ID_HEVC, 1, 0);
        return ret;


            continue;

        case HEVC_NAL_SEI_SUFFIX:
        case HEVC_NAL_TRAIL_R:
        case HEVC_NAL_TSA_N:
        case HEVC_NAL_TSA_R:
        case HEVC_NAL_STSA_N:
        case HEVC_NAL_STSA_R:
        case HEVC_NAL_BLA_W_LP:
        case HEVC_NAL_BLA_W_RADL:
        case HEVC_NAL_BLA_N_LP:
        case HEVC_NAL_IDR_W_RADL:
        case HEVC_NAL_IDR_N_LP:
        case HEVC_NAL_CRA_NUT:
        case HEVC_NAL_RADL_N:
        case HEVC_NAL_RADL_R:
        case HEVC_NAL_RASL_N:
        case HEVC_NAL_RASL_R:
                s->repeat_pict = 1;
                s->repeat_pict = 2;
            }
            break;
        }
    /* didn't find a picture! */
}

/**
 * Find the end of the current frame in the bitstream.
 * @return the position of the first byte of the next frame, or END_NOT_FOUND
 */
static int hevc_find_frame_end(AVCodecParserContext *s, const uint8_t *buf,
                               int buf_size)
{
    HEVCParserContext *ctx = s->priv_data;
    ParseContext       *pc = &ctx->pc;
    int i;

    for (i = 0; i < buf_size; i++) {
        int nut;

        pc->state64 = (pc->state64 << 8) | buf[i];

        if (((pc->state64 >> 3 * 8) & 0xFFFFFF) != START_CODE)
            continue;

        nut = (pc->state64 >> 2 * 8 + 1) & 0x3F;
        // Beginning of access unit
        if ((nut >= HEVC_NAL_VPS && nut <= HEVC_NAL_EOB_NUT) || nut == HEVC_NAL_SEI_PREFIX ||
            (nut >= 41 && nut <= 44) || (nut >= 48 && nut <= 55)) {
            if (pc->frame_start_found) {
                pc->frame_start_found = 0;
                return i - 5;
            }
        } else if (nut <= HEVC_NAL_RASL_R ||
                   (nut >= HEVC_NAL_BLA_W_LP && nut <= HEVC_NAL_CRA_NUT)) {
            int first_slice_segment_in_pic_flag = buf[i] >> 7;
            if (first_slice_segment_in_pic_flag) {
                if (!pc->frame_start_found) {
                    pc->frame_start_found = 1;
                } else { // First slice of next frame found
                    pc->frame_start_found = 0;
                    return i - 5;
                }
            }
        }
    }

    return END_NOT_FOUND;
}

                      const uint8_t **poutbuf, int *poutbuf_size,
                      const uint8_t *buf, int buf_size)
{

                                 &ctx->is_avc, &ctx->nal_length_size, avctx->err_recognition,
                                 1, avctx);
    }

        next = buf_size;
    } else {
        }
    }



}

// Split after the parameter sets at the beginning of the stream if they exist.
static int hevc_split(AVCodecContext *avctx, const uint8_t *buf, int buf_size)
{
    const uint8_t *ptr = buf, *end = buf + buf_size;
    uint32_t state = -1;
    int has_vps = 0;
    int has_sps = 0;
    int has_pps = 0;
    int nut;

    while (ptr < end) {
        ptr = avpriv_find_start_code(ptr, end, &state);
        if ((state >> 8) != START_CODE)
            break;
        nut = (state >> 1) & 0x3F;
        if (nut == HEVC_NAL_VPS)
            has_vps = 1;
        else if (nut == HEVC_NAL_SPS)
            has_sps = 1;
        else if (nut == HEVC_NAL_PPS)
            has_pps = 1;
        else if ((nut != HEVC_NAL_SEI_PREFIX || has_pps) &&
                  nut != HEVC_NAL_AUD) {
            if (has_vps && has_sps) {
                while (ptr - 4 > buf && ptr[-5] == 0)
                    ptr--;
                return ptr - 4 - buf;
            }
        }
    }
    return 0;
}

{



AVCodecParser ff_hevc_parser = {
    .codec_ids      = { AV_CODEC_ID_HEVC },
    .priv_data_size = sizeof(HEVCParserContext),
    .parser_parse   = hevc_parse,
    .parser_close   = hevc_parser_close,
    .split          = hevc_split,
};
