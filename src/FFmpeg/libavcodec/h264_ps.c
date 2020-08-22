/*
 * H.26L/H.264/AVC/JVT/14496-10/... parameter set decoding
 * Copyright (c) 2003 Michael Niedermayer <michaelni@gmx.at>
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
 * H.264 / AVC / MPEG-4 part10 parameter set decoding.
 * @author Michael Niedermayer <michaelni@gmx.at>
 */

#include <inttypes.h>

#include "libavutil/imgutils.h"
#include "internal.h"
#include "mathops.h"
#include "avcodec.h"
#include "h264data.h"
#include "h264_ps.h"
#include "golomb.h"

#define MIN_LOG2_MAX_FRAME_NUM    4

#define EXTENDED_SAR       255

static const uint8_t default_scaling4[2][16] = {
    {  6, 13, 20, 28, 13, 20, 28, 32,
      20, 28, 32, 37, 28, 32, 37, 42 },
    { 10, 14, 20, 24, 14, 20, 24, 27,
      20, 24, 27, 30, 24, 27, 30, 34 }
};

static const uint8_t default_scaling8[2][64] = {
    {  6, 10, 13, 16, 18, 23, 25, 27,
      10, 11, 16, 18, 23, 25, 27, 29,
      13, 16, 18, 23, 25, 27, 29, 31,
      16, 18, 23, 25, 27, 29, 31, 33,
      18, 23, 25, 27, 29, 31, 33, 36,
      23, 25, 27, 29, 31, 33, 36, 38,
      25, 27, 29, 31, 33, 36, 38, 40,
      27, 29, 31, 33, 36, 38, 40, 42 },
    {  9, 13, 15, 17, 19, 21, 22, 24,
      13, 13, 17, 19, 21, 22, 24, 25,
      15, 17, 19, 21, 22, 24, 25, 27,
      17, 19, 21, 22, 24, 25, 27, 28,
      19, 21, 22, 24, 25, 27, 28, 30,
      21, 22, 24, 25, 27, 28, 30, 32,
      22, 24, 25, 27, 28, 30, 32, 33,
      24, 25, 27, 28, 30, 32, 33, 35 }
};

/* maximum number of MBs in the DPB for a given level */
static const int level_max_dpb_mbs[][2] = {
    { 10, 396       },
    { 11, 900       },
    { 12, 2376      },
    { 13, 2376      },
    { 20, 2376      },
    { 21, 4752      },
    { 22, 8100      },
    { 30, 8100      },
    { 31, 18000     },
    { 32, 20480     },
    { 40, 32768     },
    { 41, 32768     },
    { 42, 34816     },
    { 50, 110400    },
    { 51, 184320    },
    { 52, 184320    },
};

{
}

{
#if 0
    int i;
    if (s->sps_list[id]) {
        /* drop all PPS that depend on this SPS */
        for (i = 0; i < FF_ARRAY_ELEMS(s->pps_list); i++)
            if (s->pps_list[i] && ((PPS*)s->pps_list[i]->data)->sps_id == id)
                remove_pps(s, i);
    }
#endif
}

                                        SPS *sps)
{

        av_log(logctx, AV_LOG_ERROR, "cpb_count %d invalid\n", cpb_count);
        return AVERROR_INVALIDDATA;
    }

    }
}

                                        SPS *sps)
{


        } else {
            av_log(logctx, AV_LOG_ERROR, "illegal aspect ratio\n");
            return AVERROR_INVALIDDATA;
        }
    } else {
    }




            // Set invalid values to "unspecified"
                sps->color_primaries = AVCOL_PRI_UNSPECIFIED;
                sps->color_trc = AVCOL_TRC_UNSPECIFIED;
                sps->colorspace = AVCOL_SPC_UNSPECIFIED;
        }
    }

    /* chroma_location_info_present_flag */
        /* chroma_sample_location_type_top_field */
    } else

        av_log(logctx, AV_LOG_WARNING, "Truncated VUI (%d)\n", get_bits_left(gb));
        return 0;
    }

            av_log(logctx, AV_LOG_ERROR,
                   "time_scale/num_units_in_tick invalid or unsupported (%u/%u)\n",
                   time_scale, num_units_in_tick);
            sps->timing_info_present_flag = 0;
        } else {
        }
    }

            return AVERROR_INVALIDDATA;
            return AVERROR_INVALIDDATA;
        return 0;

        }

            /* max_dec_frame_buffering || max_dec_frame_buffering > 16 */) {
            av_log(logctx, AV_LOG_ERROR,
                   "Clipping illegal num_reorder_frames %d\n",
                   sps->num_reorder_frames);
            sps->num_reorder_frames = 16;
            return AVERROR_INVALIDDATA;
        }
    }

    return 0;
}

                                const uint8_t *jvt_list,
                                const uint8_t *fallback_list)
{
    else
                    av_log(NULL, AV_LOG_ERROR, "delta scale %d is invalid\n", v);
                    return AVERROR_INVALIDDATA;
                }
            }
                break;
            }
        }
    return 0;
}

/* returns non zero if the provided SPS scaling matrix has been filled */
static int decode_scaling_matrices(GetBitContext *gb, const SPS *sps,
                                    const PPS *pps, int is_sps,
                                    uint8_t(*scaling_matrix4)[16],
                                    uint8_t(*scaling_matrix8)[64])
{
    int fallback_sps = !is_sps && sps->scaling_matrix_present;
    const uint8_t *fallback[4] = {
        fallback_sps ? sps->scaling_matrix4[0] : default_scaling4[0],
        fallback_sps ? sps->scaling_matrix4[3] : default_scaling4[1],
        fallback_sps ? sps->scaling_matrix8[0] : default_scaling8[0],
        fallback_sps ? sps->scaling_matrix8[3] : default_scaling8[1]
    };
    int ret = 0;
    if (get_bits1(gb)) {
        ret |= decode_scaling_list(gb, scaling_matrix4[0], 16, default_scaling4[0], fallback[0]);        // Intra, Y
        ret |= decode_scaling_list(gb, scaling_matrix4[1], 16, default_scaling4[0], scaling_matrix4[0]); // Intra, Cr
        ret |= decode_scaling_list(gb, scaling_matrix4[2], 16, default_scaling4[0], scaling_matrix4[1]); // Intra, Cb
        ret |= decode_scaling_list(gb, scaling_matrix4[3], 16, default_scaling4[1], fallback[1]);        // Inter, Y
        ret |= decode_scaling_list(gb, scaling_matrix4[4], 16, default_scaling4[1], scaling_matrix4[3]); // Inter, Cr
        ret |= decode_scaling_list(gb, scaling_matrix4[5], 16, default_scaling4[1], scaling_matrix4[4]); // Inter, Cb
        if (is_sps || pps->transform_8x8_mode) {
            ret |= decode_scaling_list(gb, scaling_matrix8[0], 64, default_scaling8[0], fallback[2]); // Intra, Y
            ret |= decode_scaling_list(gb, scaling_matrix8[3], 64, default_scaling8[1], fallback[3]); // Inter, Y
            if (sps->chroma_format_idc == 3) {
                ret |= decode_scaling_list(gb, scaling_matrix8[1], 64, default_scaling8[0], scaling_matrix8[0]); // Intra, Cr
                ret |= decode_scaling_list(gb, scaling_matrix8[4], 64, default_scaling8[1], scaling_matrix8[3]); // Inter, Cr
                ret |= decode_scaling_list(gb, scaling_matrix8[2], 64, default_scaling8[0], scaling_matrix8[1]); // Intra, Cb
                ret |= decode_scaling_list(gb, scaling_matrix8[5], 64, default_scaling8[1], scaling_matrix8[4]); // Inter, Cb
            }
        }
        if (!ret)
            ret = is_sps;
    }

    return ret;
}

{





                                     H264ParamSets *ps, int ignore_truncation)
{

        return AVERROR(ENOMEM);

        av_log(avctx, AV_LOG_DEBUG, "Truncating likely oversized SPS\n");
        sps->data_size = sizeof(sps->data);
    }


        av_log(avctx, AV_LOG_ERROR, "sps_id %u out of range\n", sps_id);
        goto fail;
    }



        sps->profile_idc == 144) {  // old High444 profile
            avpriv_request_sample(avctx, "chroma_format_idc %u",
                                  sps->chroma_format_idc);
            goto fail;
                av_log(avctx, AV_LOG_ERROR, "separate color planes are not supported\n");
                goto fail;
            }
        }
            avpriv_request_sample(avctx,
                                  "Different chroma and luma bit depth");
            goto fail;
        }
            av_log(avctx, AV_LOG_ERROR, "illegal bit depth value (%d, %d)\n",
                   sps->bit_depth_luma, sps->bit_depth_chroma);
            goto fail;
        }
                                      sps->scaling_matrix4, sps->scaling_matrix8);
            goto fail;
    } else {
    }

        log2_max_frame_num_minus4 > MAX_LOG2_MAX_FRAME_NUM - 4) {
        av_log(avctx, AV_LOG_ERROR,
               "log2_max_frame_num_minus4 out of range (0-12): %d\n",
               log2_max_frame_num_minus4);
        goto fail;
    }


            av_log(avctx, AV_LOG_ERROR, "log2_max_poc_lsb (%d) is out of range\n", t);
            goto fail;
        }

        ) {
            av_log(avctx, AV_LOG_ERROR,
                   "offset_for_non_ref_pic or offset_for_top_to_bottom_field is out of range\n");
            goto fail;
        }


            FF_ARRAY_ELEMS(sps->offset_for_ref_frame)) {
            av_log(avctx, AV_LOG_ERROR,
                   "poc_cycle_length overflow %d\n", sps->poc_cycle_length);
            goto fail;
        }

                av_log(avctx, AV_LOG_ERROR,
                       "offset_for_ref_frame is out of range\n");
                goto fail;
            }
        }
        av_log(avctx, AV_LOG_ERROR, "illegal POC type %d\n", sps->poc_type);
        goto fail;
    }

        sps->ref_frame_count = FFMAX(2, sps->ref_frame_count);
        av_log(avctx, AV_LOG_ERROR,
               "too many reference frames %d\n", sps->ref_frame_count);
        goto fail;
    }


        av_log(avctx, AV_LOG_ERROR, "height overflow\n");
        goto fail;
    }

    else

        av_log(avctx, AV_LOG_ERROR, "mb_width/height overflow\n");
        goto fail;
    }


#ifndef ALLOW_INTERLACE
    if (sps->mb_aff)
        av_log(avctx, AV_LOG_ERROR,
               "MBAFF support not included; enable it at compile-time.\n");
#endif

            av_log(avctx, AV_LOG_DEBUG, "discarding sps cropping, original "
                                           "values are l:%d r:%d t:%d b:%d\n",
                   crop_left, crop_right, crop_top, crop_bottom);

            sps->crop_left   =
            sps->crop_right  =
            sps->crop_top    =
            sps->crop_bottom = 0;
        } else {

            ) {
                av_log(avctx, AV_LOG_ERROR, "crop values invalid %d %d %d %d / %d %d\n", crop_left, crop_right, crop_top, crop_bottom, width, height);
                goto fail;
            }

        }
    } else {
    }

            goto fail;
    }

                    &ps->overread_warning_printed[sps->vui_parameters_present_flag],
    }

    /* if the maximum delay is not stored in the SPS, derive it based on the
     * level */
                                                sps->num_reorder_frames);
            }
        }
    }


        static const char csp[4][5] = { "Gray", "420", "422", "444" };
        av_log(avctx, AV_LOG_DEBUG,
               "sps:%u profile:%d/%d poc:%d ref:%d %dx%d %s %s crop:%u/%u/%u/%u %s %s %"PRId32"/%"PRId32" b%d reo:%d\n",
               sps_id, sps->profile_idc, sps->level_idc,
               sps->poc_type,
               sps->ref_frame_count,
               sps->mb_width, sps->mb_height,
               sps->frame_mbs_only_flag ? "FRM" : (sps->mb_aff ? "MB-AFF" : "PIC-AFF"),
               sps->direct_8x8_inference_flag ? "8B8" : "",
               sps->crop_left, sps->crop_right,
               sps->crop_top, sps->crop_bottom,
               sps->vui_parameters_present_flag ? "VUI" : "",
               csp[sps->chroma_format_idc],
               sps->timing_info_present_flag ? sps->num_units_in_tick : 0,
               sps->timing_info_present_flag ? sps->time_scale : 0,
               sps->bit_depth_luma,
               sps->bitstream_restriction_flag ? sps->num_reorder_frames : -1
               );
    }

    /* check if this is a repeat of an already parsed SPS, then keep the
     * original one.
     * otherwise drop all PPSes that depend on it */
    } else {
    }

    return 0;

}

static void init_dequant8_coeff_table(PPS *pps, const SPS *sps)
{
    int i, j, q, x;
    const int max_qp = 51 + 6 * (sps->bit_depth_luma - 8);

    for (i = 0; i < 6; i++) {
        pps->dequant8_coeff[i] = pps->dequant8_buffer[i];
        for (j = 0; j < i; j++)
            if (!memcmp(pps->scaling_matrix8[j], pps->scaling_matrix8[i],
                        64 * sizeof(uint8_t))) {
                pps->dequant8_coeff[i] = pps->dequant8_buffer[j];
                break;
            }
        if (j < i)
            continue;

        for (q = 0; q < max_qp + 1; q++) {
            int shift = ff_h264_quant_div6[q];
            int idx   = ff_h264_quant_rem6[q];
            for (x = 0; x < 64; x++)
                pps->dequant8_coeff[i][q][(x >> 3) | ((x & 7) << 3)] =
                    ((uint32_t)ff_h264_dequant8_coeff_init[idx][ff_h264_dequant8_coeff_init_scan[((x >> 1) & 12) | (x & 3)]] *
                     pps->scaling_matrix8[i][x]) << shift;
        }
    }
}

static void init_dequant4_coeff_table(PPS *pps, const SPS *sps)
{
    int i, j, q, x;
    const int max_qp = 51 + 6 * (sps->bit_depth_luma - 8);
    for (i = 0; i < 6; i++) {
        pps->dequant4_coeff[i] = pps->dequant4_buffer[i];
        for (j = 0; j < i; j++)
            if (!memcmp(pps->scaling_matrix4[j], pps->scaling_matrix4[i],
                        16 * sizeof(uint8_t))) {
                pps->dequant4_coeff[i] = pps->dequant4_buffer[j];
                break;
            }
        if (j < i)
            continue;

        for (q = 0; q < max_qp + 1; q++) {
            int shift = ff_h264_quant_div6[q] + 2;
            int idx   = ff_h264_quant_rem6[q];
            for (x = 0; x < 16; x++)
                pps->dequant4_coeff[i][q][(x >> 2) | ((x << 2) & 0xF)] =
                    ((uint32_t)ff_h264_dequant4_coeff_init[idx][(x & 1) + ((x >> 2) & 1)] *
                     pps->scaling_matrix4[i][x]) << shift;
        }
    }
}

{

    }

{
}

static int more_rbsp_data_in_pps(const SPS *sps, void *logctx)
{
    int profile_idc = sps->profile_idc;

    if ((profile_idc == 66 || profile_idc == 77 ||
         profile_idc == 88) && (sps->constraint_set_flags & 7)) {
        av_log(logctx, AV_LOG_VERBOSE,
               "Current profile doesn't provide more RBSP data in PPS, skipping\n");
        return 0;
    }

    return 1;
}

{



                                         H264ParamSets *ps, int bit_length)
{

        av_log(avctx, AV_LOG_ERROR, "pps_id %u out of range\n", pps_id);
        return AVERROR_INVALIDDATA;
    }

        return AVERROR(ENOMEM);
                               pps_free, NULL, 0);
        av_freep(&pps);
        return AVERROR(ENOMEM);
    }

        av_log(avctx, AV_LOG_DEBUG, "Truncating likely oversized PPS "
               "(%"SIZE_SPECIFIER" > %"SIZE_SPECIFIER")\n",
               pps->data_size, sizeof(pps->data));
        pps->data_size = sizeof(pps->data);
    }

        av_log(avctx, AV_LOG_ERROR, "sps_id %u out of range\n", pps->sps_id);
        ret = AVERROR_INVALIDDATA;
        goto fail;
    }
        ret = AVERROR(ENOMEM);
        goto fail;
    }

        av_log(avctx, AV_LOG_ERROR,
               "Invalid luma bit depth=%d\n",
               sps->bit_depth_luma);
        ret = AVERROR_INVALIDDATA;
        goto fail;
        avpriv_report_missing_feature(avctx,
               "Unimplemented luma bit depth=%d",
               sps->bit_depth_luma);
        ret = AVERROR_PATCHWELCOME;
        goto fail;
    }

    }
        av_log(avctx, AV_LOG_ERROR, "reference overflow (pps)\n");
        ret = AVERROR_INVALIDDATA;
        goto fail;
    }


        ret = AVERROR_INVALIDDATA;
        goto fail;
    }


           sizeof(pps->scaling_matrix4));
           sizeof(pps->scaling_matrix8));

            goto fail;
        // second_chroma_qp_index_offset
            ret = AVERROR_INVALIDDATA;
            goto fail;
        }
    } else {
    }

                   sps->bit_depth_luma);
                   sps->bit_depth_luma);



        av_log(avctx, AV_LOG_DEBUG,
               "pps:%u sps:%u %s slice_groups:%d ref:%u/%u %s qp:%d/%d/%d/%d %s %s %s %s\n",
               pps_id, pps->sps_id,
               pps->cabac ? "CABAC" : "CAVLC",
               pps->slice_group_count,
               pps->ref_count[0], pps->ref_count[1],
               pps->weighted_pred ? "weighted" : "",
               pps->init_qp, pps->init_qs, pps->chroma_qp_index_offset[0], pps->chroma_qp_index_offset[1],
               pps->deblocking_filter_parameters_present ? "LPAR" : "",
               pps->constrained_intra_pred ? "CONSTR" : "",
               pps->redundant_pic_cnt_present ? "REDU" : "",
               pps->transform_8x8_mode ? "8x8DCT" : "");
    }



}
