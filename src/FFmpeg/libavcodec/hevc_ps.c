/*
 * HEVC Parameter Set decoding
 *
 * Copyright (C) 2012 - 2103 Guillaume Martres
 * Copyright (C) 2012 - 2103 Mickael Raulet
 * Copyright (C) 2012 - 2013 Gildas Cocherel
 * Copyright (C) 2013 Vittorio Giovara
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

#include "libavutil/imgutils.h"
#include "golomb.h"
#include "hevc_data.h"
#include "hevc_ps.h"

static const uint8_t default_scaling_list_intra[] = {
    16, 16, 16, 16, 17, 18, 21, 24,
    16, 16, 16, 16, 17, 19, 22, 25,
    16, 16, 17, 18, 20, 22, 25, 29,
    16, 16, 18, 21, 24, 27, 31, 36,
    17, 17, 20, 24, 30, 35, 41, 47,
    18, 19, 22, 27, 35, 44, 54, 65,
    21, 22, 25, 31, 41, 54, 70, 88,
    24, 25, 29, 36, 47, 65, 88, 115
};

static const uint8_t default_scaling_list_inter[] = {
    16, 16, 16, 16, 17, 18, 20, 24,
    16, 16, 16, 17, 18, 20, 24, 25,
    16, 16, 17, 18, 20, 24, 25, 28,
    16, 17, 18, 20, 24, 25, 28, 33,
    17, 18, 20, 24, 25, 28, 33, 41,
    18, 20, 24, 25, 28, 33, 41, 54,
    20, 24, 25, 28, 33, 41, 54, 71,
    24, 25, 28, 33, 41, 54, 71, 91
};

static const AVRational vui_sar[] = {
    {  0,   1 },
    {  1,   1 },
    { 12,  11 },
    { 10,  11 },
    { 16,  11 },
    { 40,  33 },
    { 24,  11 },
    { 20,  11 },
    { 32,  11 },
    { 80,  33 },
    { 18,  11 },
    { 15,  11 },
    { 64,  33 },
    { 160, 99 },
    {  4,   3 },
    {  3,   2 },
    {  2,   1 },
};

static const uint8_t hevc_sub_width_c[] = {
    1, 2, 2, 1
};

static const uint8_t hevc_sub_height_c[] = {
    1, 2, 1, 1
};

{

{

        /* drop all PPS that depend on this SPS */

    }

{

    }

                                  ShortTermRPS *rps, const HEVCSPS *sps, int is_slice_header)
{



                av_log(avctx, AV_LOG_ERROR,
                       "Invalid value of delta_idx in slice header RPS: %d > %d.\n",
                       delta_idx, sps->nb_st_rps);
                return AVERROR_INVALIDDATA;
            }
        } else

            av_log(avctx, AV_LOG_ERROR,
                   "Invalid value of abs_delta_rps: %d\n",
                   abs_delta_rps);
            return AVERROR_INVALIDDATA;
        }


                else
                    delta_poc = delta_rps;
                else
            }
        }

            av_log(avctx, AV_LOG_ERROR,
                   "Invalid num_delta_pocs: %d\n", k);
            return AVERROR_INVALIDDATA;
        }

        // sort in increasing order (smallest first)
            int used, tmp;
                    }
                }
            }
        }
            // flip the negative values to largest first
            }
        }
    } else {

            nb_positive_pics >= HEVC_MAX_REFS) {
            av_log(avctx, AV_LOG_ERROR, "Too many refs in a short term RPS.\n");
            return AVERROR_INVALIDDATA;
        }

            prev = 0;
                    av_log(avctx, AV_LOG_ERROR,
                        "Invalid value of delta_poc: %d\n",
                        delta_poc);
                    return AVERROR_INVALIDDATA;
                }
            }
            prev = 0;
                    av_log(avctx, AV_LOG_ERROR,
                        "Invalid value of delta_poc: %d\n",
                        delta_poc);
                    return AVERROR_INVALIDDATA;
                }
            }
        }
    }
    return 0;
}


                                      PTLCommon *ptl)
{

        return -1;

    else


    }

#define check_profile_idc(idc) \
        ptl->profile_idc == idc || ptl->profile_compatibility_flag[idc]



            ptl->max_14bit_constraint_flag    = get_bits1(gb);
            skip_bits_long(gb, 33); // XXX_reserved_zero_33bits[0..32]
        } else {
        }
    } else {
    }

        check_profile_idc(4) || check_profile_idc(5) || check_profile_idc(9))
    else
        skip_bits1(gb);
#undef check_profile_idc

    return 0;
}

                      PTL *ptl, int max_num_sub_layers)
{
        av_log(avctx, AV_LOG_ERROR, "PTL information too short\n");
        return -1;
    }


    }

            av_log(avctx, AV_LOG_ERROR,
                   "PTL information for sublayer %i too short\n", i);
            return -1;
        }
                av_log(avctx, AV_LOG_ERROR,
                       "Not enough data for sublayer %i level_idc\n", i);
                return -1;
            } else
        }
    }

    return 0;
}

                                int subpic_params_present)
{


        }
    }

                       int max_sublayers)
{



            }



        }
    }



        else

                av_log(NULL, AV_LOG_ERROR, "nb_cpb %d invalid\n", nb_cpb);
                return AVERROR_INVALIDDATA;
            }
        }

    }
    return 0;
}

                           HEVCParamSets *ps)
{

        return AVERROR(ENOMEM);


        av_log(avctx, AV_LOG_WARNING, "Truncating likely oversized VPS "
               "(%"PTRDIFF_SPECIFIER" > %"SIZE_SPECIFIER")\n",
               nal_size, sizeof(vps->data));
        vps->data_size = sizeof(vps->data);
    } else {
    }


        av_log(avctx, AV_LOG_ERROR, "vps_reserved_three_2bits is not three\n");
        goto err;
    }


        av_log(avctx, AV_LOG_ERROR, "vps_reserved_ffff_16bits is not 0xffff\n");
        goto err;
    }

        av_log(avctx, AV_LOG_ERROR, "vps_max_sub_layers out of range: %d\n",
               vps->vps_max_sub_layers);
        goto err;
    }

        goto err;



            av_log(avctx, AV_LOG_ERROR, "vps_max_dec_pic_buffering_minus1 out of range: %d\n",
                   vps->vps_max_dec_pic_buffering[i] - 1);
            goto err;
        }
            av_log(avctx, AV_LOG_WARNING, "vps_max_num_reorder_pics out of range: %d\n",
                   vps->vps_num_reorder_pics[i]);
            if (avctx->err_recognition & AV_EF_EXPLODE)
                goto err;
        }
    }

        av_log(avctx, AV_LOG_ERROR, "too many layer_id_included_flags\n");
        goto err;
    }

        for (j = 0; j <= vps->vps_max_layer_id; j++)
            skip_bits(gb, 1);  // layer_id_included_flag[i][j]

            av_log(avctx, AV_LOG_ERROR,
                   "vps_num_hrd_parameters %d is invalid\n", vps->vps_num_hrd_parameters);
            goto err;
        }

                common_inf_present = get_bits1(gb);
        }
    }

        av_log(avctx, AV_LOG_ERROR,
               "Overread VPS by %d bits\n", -get_bits_left(gb));
        if (ps->vps_list[vps_id])
            goto err;
    }

    } else {
    }

    return 0;

err:
    av_buffer_unref(&vps_buf);
    return AVERROR_INVALIDDATA;
}

                       int apply_defdispwin, HEVCSPS *sps)
{


        else if (sar_idx == 255) {
            vui->sar.num = get_bits(gb, 16);
            vui->sar.den = get_bits(gb, 16);
        } else
            av_log(avctx, AV_LOG_WARNING,
                   "Unknown SAR index: %u.\n", sar_idx);
    }

        vui->overscan_appropriate_flag = get_bits1(gb);


            // Set invalid values to "unspecified"
                vui->colour_primaries = AVCOL_PRI_UNSPECIFIED;
                vui->transfer_characteristic = AVCOL_TRC_UNSPECIFIED;
                vui->matrix_coeffs = AVCOL_SPC_UNSPECIFIED;
                switch (sps->pix_fmt) {
                case AV_PIX_FMT_YUV444P:
                    sps->pix_fmt = AV_PIX_FMT_GBRP;
                    break;
                case AV_PIX_FMT_YUV444P10:
                    sps->pix_fmt = AV_PIX_FMT_GBRP10;
                    break;
                case AV_PIX_FMT_YUV444P12:
                    sps->pix_fmt = AV_PIX_FMT_GBRP12;
                    break;
                }
        }
    }

    }


    // Backup context in case an alternate header is detected
        vui->default_display_window_flag = 0;
        av_log(avctx, AV_LOG_WARNING, "Invalid default display window\n");
    } else


            av_log(avctx, AV_LOG_DEBUG,
                   "discarding vui default display window, "
                   "original values are l:%u r:%u t:%u b:%u\n",
                   vui->def_disp_win.left_offset,
                   vui->def_disp_win.right_offset,
                   vui->def_disp_win.top_offset,
                   vui->def_disp_win.bottom_offset);

            vui->def_disp_win.left_offset   =
            vui->def_disp_win.right_offset  =
            vui->def_disp_win.top_offset    =
            vui->def_disp_win.bottom_offset = 0;
        }
    }


            // The alternate syntax seem to have timing info located
            // at where def_disp_win is normally located
            av_log(avctx, AV_LOG_WARNING,
                   "Strange VUI timing information, retrying...\n");
            memcpy(vui, &backup_vui, sizeof(backup_vui));
            memcpy(gb, &backup, sizeof(backup));
            alt = 1;
            goto timing_info;
        }
            av_log(avctx, AV_LOG_INFO, "Retry got %"PRIu32"/%"PRIu32" fps\n",
                   vui->vui_time_scale, vui->vui_num_units_in_tick);
        }
    }

            av_log(avctx, AV_LOG_WARNING,
                   "Strange VUI bitstream restriction information, retrying"
                   " from timing information...\n");
            memcpy(vui, &backup_vui, sizeof(backup_vui));
            memcpy(gb, &backup, sizeof(backup));
            alt = 1;
            goto timing_info;
        }
    }

        // XXX: Alternate syntax when sps_range_extension_flag != 0?
        av_log(avctx, AV_LOG_WARNING,
               "Overread in VUI, retrying from timing information...\n");
        memcpy(vui, &backup_vui, sizeof(backup_vui));
        memcpy(gb, &backup, sizeof(backup));
        alt = 1;
        goto timing_info;
    }

{

        // 4x4 default is 16
    }

static int scaling_list_data(GetBitContext *gb, AVCodecContext *avctx, ScalingList *sl, HEVCSPS *sps)
{
    uint8_t scaling_list_pred_mode_flag;
    int32_t scaling_list_dc_coef[2][6];
    int size_id, matrix_id, pos;
    int i;

    for (size_id = 0; size_id < 4; size_id++)
        for (matrix_id = 0; matrix_id < 6; matrix_id += ((size_id == 3) ? 3 : 1)) {
            scaling_list_pred_mode_flag = get_bits1(gb);
            if (!scaling_list_pred_mode_flag) {
                unsigned int delta = get_ue_golomb_long(gb);
                /* Only need to handle non-zero delta. Zero means default,
                 * which should already be in the arrays. */
                if (delta) {
                    // Copy from previous array.
                    delta *= (size_id == 3) ? 3 : 1;
                    if (matrix_id < delta) {
                        av_log(avctx, AV_LOG_ERROR,
                               "Invalid delta in scaling list data: %d.\n", delta);
                        return AVERROR_INVALIDDATA;
                    }

                    memcpy(sl->sl[size_id][matrix_id],
                           sl->sl[size_id][matrix_id - delta],
                           size_id > 0 ? 64 : 16);
                    if (size_id > 1)
                        sl->sl_dc[size_id - 2][matrix_id] = sl->sl_dc[size_id - 2][matrix_id - delta];
                }
            } else {
                int next_coef, coef_num;
                int32_t scaling_list_delta_coef;

                next_coef = 8;
                coef_num  = FFMIN(64, 1 << (4 + (size_id << 1)));
                if (size_id > 1) {
                    scaling_list_dc_coef[size_id - 2][matrix_id] = get_se_golomb(gb) + 8;
                    next_coef = scaling_list_dc_coef[size_id - 2][matrix_id];
                    sl->sl_dc[size_id - 2][matrix_id] = next_coef;
                }
                for (i = 0; i < coef_num; i++) {
                    if (size_id == 0)
                        pos = 4 * ff_hevc_diag_scan4x4_y[i] +
                                  ff_hevc_diag_scan4x4_x[i];
                    else
                        pos = 8 * ff_hevc_diag_scan8x8_y[i] +
                                  ff_hevc_diag_scan8x8_x[i];

                    scaling_list_delta_coef = get_se_golomb(gb);
                    next_coef = (next_coef + 256U + scaling_list_delta_coef) % 256;
                    sl->sl[size_id][matrix_id][pos] = next_coef;
                }
            }
        }

    if (sps->chroma_format_idc == 3) {
        for (i = 0; i < 64; i++) {
            sl->sl[3][1][i] = sl->sl[2][1][i];
            sl->sl[3][2][i] = sl->sl[2][2][i];
            sl->sl[3][4][i] = sl->sl[2][4][i];
            sl->sl[3][5][i] = sl->sl[2][5][i];
        }
        sl->sl_dc[1][1] = sl->sl_dc[0][1];
        sl->sl_dc[1][2] = sl->sl_dc[0][2];
        sl->sl_dc[1][4] = sl->sl_dc[0][4];
        sl->sl_dc[1][5] = sl->sl_dc[0][5];
    }


    return 0;
}

{
       break;
    case 9:
        if (sps->chroma_format_idc == 0) sps->pix_fmt = AV_PIX_FMT_GRAY9;
        if (sps->chroma_format_idc == 1) sps->pix_fmt = AV_PIX_FMT_YUV420P9;
        if (sps->chroma_format_idc == 2) sps->pix_fmt = AV_PIX_FMT_YUV422P9;
        if (sps->chroma_format_idc == 3) sps->pix_fmt = AV_PIX_FMT_YUV444P9;
        break;
        break;
        break;
    default:
        av_log(avctx, AV_LOG_ERROR,
               "The following bit-depths are currently specified: 8, 9, 10 and 12 bits, "
               "chroma_format_idc is %d, depth is %d\n",
               sps->chroma_format_idc, sps->bit_depth);
        return AVERROR_INVALIDDATA;
    }

        return AVERROR(EINVAL);



}

                      int apply_defdispwin, AVBufferRef **vps_list, AVCodecContext *avctx)
{

    // Coded parameters


        av_log(avctx, AV_LOG_ERROR, "VPS %d does not exist\n",
               sps->vps_id);
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "sps_max_sub_layers out of range: %d\n",
               sps->max_sub_layers);
        return AVERROR_INVALIDDATA;
    }


        return ret;

        av_log(avctx, AV_LOG_ERROR, "SPS id out of range: %d\n", *sps_id);
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "chroma_format_idc %d is invalid\n", sps->chroma_format_idc);
        return AVERROR_INVALIDDATA;
    }


        sps->chroma_format_idc = 0;

                                   sps->height, 0, avctx)) < 0)
        return ret;


            av_log(avctx, AV_LOG_DEBUG,
                   "discarding sps conformance window, "
                   "original values are l:%u r:%u t:%u b:%u\n",
                   sps->pic_conf_win.left_offset,
                   sps->pic_conf_win.right_offset,
                   sps->pic_conf_win.top_offset,
                   sps->pic_conf_win.bottom_offset);

            sps->pic_conf_win.left_offset   =
            sps->pic_conf_win.right_offset  =
            sps->pic_conf_win.top_offset    =
            sps->pic_conf_win.bottom_offset = 0;
        }
    }

        av_log(avctx, AV_LOG_ERROR,
               "Luma bit depth (%d) is different from chroma bit depth (%d), "
               "this is unsupported.\n",
               sps->bit_depth, bit_depth_chroma);
        return AVERROR_INVALIDDATA;
    }

        return ret;

        av_log(avctx, AV_LOG_ERROR, "log2_max_pic_order_cnt_lsb_minus4 out range: %d\n",
               sps->log2_max_poc_lsb - 4);
        return AVERROR_INVALIDDATA;
    }

            av_log(avctx, AV_LOG_ERROR, "sps_max_dec_pic_buffering_minus1 out of range: %d\n",
                   sps->temporal_layer[i].max_dec_pic_buffering - 1U);
            return AVERROR_INVALIDDATA;
        }
            av_log(avctx, AV_LOG_WARNING, "sps_max_num_reorder_pics out of range: %d\n",
                   sps->temporal_layer[i].num_reorder_pics);
            if (avctx->err_recognition & AV_EF_EXPLODE ||
                sps->temporal_layer[i].num_reorder_pics > HEVC_MAX_DPB_SIZE - 1) {
                return AVERROR_INVALIDDATA;
            }
            sps->temporal_layer[i].max_dec_pic_buffering = sps->temporal_layer[i].num_reorder_pics + 1;
        }
    }

        }
    }


        av_log(avctx, AV_LOG_ERROR, "Invalid value %d for log2_min_cb_size", sps->log2_min_cb_size);
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "Invalid value %d for log2_diff_max_min_coding_block_size", sps->log2_diff_max_min_coding_block_size);
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "Invalid value for log2_min_tb_size");
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "Invalid value %d for log2_diff_max_min_transform_block_size", log2_diff_max_min_transform_block_size);
        return AVERROR_INVALIDDATA;
    }



                return ret;
        }
    }


            av_log(avctx, AV_LOG_ERROR,
                   "PCM bit depth (%d, %d) is greater than normal bit depth (%d)\n",
                   sps->pcm.bit_depth, sps->pcm.bit_depth_chroma, sps->bit_depth);
            return AVERROR_INVALIDDATA;
        }

    }

        av_log(avctx, AV_LOG_ERROR, "Too many short term RPS: %d.\n",
               sps->nb_st_rps);
        return AVERROR_INVALIDDATA;
    }
                                                 sps, 0)) < 0)
            return ret;
    }

            av_log(avctx, AV_LOG_ERROR, "Too many long term ref pics: %d.\n",
                   sps->num_long_term_ref_pics_sps);
            return AVERROR_INVALIDDATA;
        }
        }
    }




                av_log(avctx, AV_LOG_WARNING,
                   "extended_precision_processing_flag not yet implemented\n");

                   "high_precision_offsets_enabled_flag not yet implemented\n");


                av_log(avctx, AV_LOG_WARNING,
                   "cabac_bypass_alignment_enabled_flag not yet implemented\n");
        }
    }
    }

        av_log(avctx, AV_LOG_WARNING, "Invalid cropping offsets: %u/%u/%u/%u\n",
               ow->left_offset, ow->right_offset, ow->top_offset, ow->bottom_offset);
        if (avctx->err_recognition & AV_EF_EXPLODE) {
            return AVERROR_INVALIDDATA;
        }
        av_log(avctx, AV_LOG_WARNING,
               "Displaying the whole video surface.\n");
        memset(ow, 0, sizeof(*ow));
        memset(&sps->pic_conf_win, 0, sizeof(sps->pic_conf_win));
    }

    // Inferred parameters

        av_log(avctx, AV_LOG_ERROR, "CTB size out of range: 2^%d\n", sps->log2_ctb_size);
        return AVERROR_INVALIDDATA;
    }
        av_log(avctx,
               AV_LOG_ERROR,
               "log2_ctb_size %d differs from the bounds of any known profile\n",
               sps->log2_ctb_size);
        avpriv_request_sample(avctx, "log2_ctb_size %d", sps->log2_ctb_size);
        return AVERROR_INVALIDDATA;
    }




        av_log(avctx, AV_LOG_ERROR, "Invalid coded frame dimensions.\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "max_transform_hierarchy_depth_inter out of range: %d\n",
               sps->max_transform_hierarchy_depth_inter);
        return AVERROR_INVALIDDATA;
    }
        av_log(avctx, AV_LOG_ERROR, "max_transform_hierarchy_depth_intra out of range: %d\n",
               sps->max_transform_hierarchy_depth_intra);
        return AVERROR_INVALIDDATA;
    }
        av_log(avctx, AV_LOG_ERROR,
               "max transform block size out of range: %d\n",
               sps->log2_max_trafo_size);
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR,
               "Overread SPS by %d bits\n", -get_bits_left(gb));
        return AVERROR_INVALIDDATA;
    }

    return 0;
}

                           HEVCParamSets *ps, int apply_defdispwin)
{

        return AVERROR(ENOMEM);


        av_log(avctx, AV_LOG_WARNING, "Truncating likely oversized SPS "
               "(%"PTRDIFF_SPECIFIER" > %"SIZE_SPECIFIER")\n",
               nal_size, sizeof(sps->data));
        sps->data_size = sizeof(sps->data);
    } else {
    }

                            apply_defdispwin,
        av_buffer_unref(&sps_buf);
        return ret;
    }

        av_log(avctx, AV_LOG_DEBUG,
               "Parsed SPS: id %d; coded wxh: %dx%d; "
               "cropped wxh: %dx%d; pix_fmt: %s.\n",
               sps_id, sps->width, sps->height,
               sps->width - (sps->output_window.left_offset + sps->output_window.right_offset),
               sps->height - (sps->output_window.top_offset + sps->output_window.bottom_offset),
               av_get_pix_fmt_name(sps->pix_fmt));
    }

    /* check if this is a repeat of an already parsed SPS, then keep the
     * original one.
     * otherwise drop all PPSes that depend on it */
    } else {
    }

    return 0;
}

{



static int pps_range_extensions(GetBitContext *gb, AVCodecContext *avctx,
                                HEVCPPS *pps, HEVCSPS *sps) {
    int i;

    if (pps->transform_skip_enabled_flag) {
        pps->log2_max_transform_skip_block_size = get_ue_golomb_long(gb) + 2;
    }
    pps->cross_component_prediction_enabled_flag = get_bits1(gb);
    pps->chroma_qp_offset_list_enabled_flag = get_bits1(gb);
    if (pps->chroma_qp_offset_list_enabled_flag) {
        pps->diff_cu_chroma_qp_offset_depth = get_ue_golomb_long(gb);
        pps->chroma_qp_offset_list_len_minus1 = get_ue_golomb_long(gb);
        if (pps->chroma_qp_offset_list_len_minus1 > 5) {
            av_log(avctx, AV_LOG_ERROR,
                   "chroma_qp_offset_list_len_minus1 shall be in the range [0, 5].\n");
            return AVERROR_INVALIDDATA;
        }
        for (i = 0; i <= pps->chroma_qp_offset_list_len_minus1; i++) {
            pps->cb_qp_offset_list[i] = get_se_golomb_long(gb);
            if (pps->cb_qp_offset_list[i]) {
                av_log(avctx, AV_LOG_WARNING,
                       "cb_qp_offset_list not tested yet.\n");
            }
            pps->cr_qp_offset_list[i] = get_se_golomb_long(gb);
            if (pps->cr_qp_offset_list[i]) {
                av_log(avctx, AV_LOG_WARNING,
                       "cb_qp_offset_list not tested yet.\n");
            }
        }
    }
    pps->log2_sao_offset_scale_luma = get_ue_golomb_long(gb);
    pps->log2_sao_offset_scale_chroma = get_ue_golomb_long(gb);

    if (   pps->log2_sao_offset_scale_luma   > FFMAX(sps->bit_depth        - 10, 0)
        || pps->log2_sao_offset_scale_chroma > FFMAX(sps->bit_depth_chroma - 10, 0)
    )
        return AVERROR_INVALIDDATA;

    return(0);
}

static inline int setup_pps(AVCodecContext *avctx, GetBitContext *gb,
                            HEVCPPS *pps, HEVCSPS *sps)
{
    int log2_diff;
    int pic_area_in_ctbs;
    int i, j, x, y, ctb_addr_rs, tile_id;

    // Inferred parameters
    pps->col_bd   = av_malloc_array(pps->num_tile_columns + 1, sizeof(*pps->col_bd));
    pps->row_bd   = av_malloc_array(pps->num_tile_rows + 1,    sizeof(*pps->row_bd));
    pps->col_idxX = av_malloc_array(sps->ctb_width,    sizeof(*pps->col_idxX));
    if (!pps->col_bd || !pps->row_bd || !pps->col_idxX)
        return AVERROR(ENOMEM);

    if (pps->uniform_spacing_flag) {
        if (!pps->column_width) {
            pps->column_width = av_malloc_array(pps->num_tile_columns, sizeof(*pps->column_width));
            pps->row_height   = av_malloc_array(pps->num_tile_rows,    sizeof(*pps->row_height));
        }
        if (!pps->column_width || !pps->row_height)
            return AVERROR(ENOMEM);

        for (i = 0; i < pps->num_tile_columns; i++) {
            pps->column_width[i] = ((i + 1) * sps->ctb_width) / pps->num_tile_columns -
                                   (i * sps->ctb_width) / pps->num_tile_columns;
        }

        for (i = 0; i < pps->num_tile_rows; i++) {
            pps->row_height[i] = ((i + 1) * sps->ctb_height) / pps->num_tile_rows -
                                 (i * sps->ctb_height) / pps->num_tile_rows;
        }
    }

    pps->col_bd[0] = 0;
    for (i = 0; i < pps->num_tile_columns; i++)
        pps->col_bd[i + 1] = pps->col_bd[i] + pps->column_width[i];

    pps->row_bd[0] = 0;
    for (i = 0; i < pps->num_tile_rows; i++)
        pps->row_bd[i + 1] = pps->row_bd[i] + pps->row_height[i];

    for (i = 0, j = 0; i < sps->ctb_width; i++) {
        if (i > pps->col_bd[j])
            j++;
        pps->col_idxX[i] = j;
    }

    /**
     * 6.5
     */
    pic_area_in_ctbs     = sps->ctb_width    * sps->ctb_height;

    pps->ctb_addr_rs_to_ts = av_malloc_array(pic_area_in_ctbs,    sizeof(*pps->ctb_addr_rs_to_ts));
    pps->ctb_addr_ts_to_rs = av_malloc_array(pic_area_in_ctbs,    sizeof(*pps->ctb_addr_ts_to_rs));
    pps->tile_id           = av_malloc_array(pic_area_in_ctbs,    sizeof(*pps->tile_id));
    pps->min_tb_addr_zs_tab = av_malloc_array((sps->tb_mask+2) * (sps->tb_mask+2), sizeof(*pps->min_tb_addr_zs_tab));
    if (!pps->ctb_addr_rs_to_ts || !pps->ctb_addr_ts_to_rs ||
        !pps->tile_id || !pps->min_tb_addr_zs_tab) {
        return AVERROR(ENOMEM);
    }

    for (ctb_addr_rs = 0; ctb_addr_rs < pic_area_in_ctbs; ctb_addr_rs++) {
        int tb_x   = ctb_addr_rs % sps->ctb_width;
        int tb_y   = ctb_addr_rs / sps->ctb_width;
        int tile_x = 0;
        int tile_y = 0;
        int val    = 0;

        for (i = 0; i < pps->num_tile_columns; i++) {
            if (tb_x < pps->col_bd[i + 1]) {
                tile_x = i;
                break;
            }
        }

        for (i = 0; i < pps->num_tile_rows; i++) {
            if (tb_y < pps->row_bd[i + 1]) {
                tile_y = i;
                break;
            }
        }

        for (i = 0; i < tile_x; i++)
            val += pps->row_height[tile_y] * pps->column_width[i];
        for (i = 0; i < tile_y; i++)
            val += sps->ctb_width * pps->row_height[i];

        val += (tb_y - pps->row_bd[tile_y]) * pps->column_width[tile_x] +
               tb_x - pps->col_bd[tile_x];

        pps->ctb_addr_rs_to_ts[ctb_addr_rs] = val;
        pps->ctb_addr_ts_to_rs[val]         = ctb_addr_rs;
    }

    for (j = 0, tile_id = 0; j < pps->num_tile_rows; j++)
        for (i = 0; i < pps->num_tile_columns; i++, tile_id++)
            for (y = pps->row_bd[j]; y < pps->row_bd[j + 1]; y++)
                for (x = pps->col_bd[i]; x < pps->col_bd[i + 1]; x++)
                    pps->tile_id[pps->ctb_addr_rs_to_ts[y * sps->ctb_width + x]] = tile_id;

    pps->tile_pos_rs = av_malloc_array(tile_id, sizeof(*pps->tile_pos_rs));
    if (!pps->tile_pos_rs)
        return AVERROR(ENOMEM);

    for (j = 0; j < pps->num_tile_rows; j++)
        for (i = 0; i < pps->num_tile_columns; i++)
            pps->tile_pos_rs[j * pps->num_tile_columns + i] =
                pps->row_bd[j] * sps->ctb_width + pps->col_bd[i];

    log2_diff = sps->log2_ctb_size - sps->log2_min_tb_size;
    pps->min_tb_addr_zs = &pps->min_tb_addr_zs_tab[1*(sps->tb_mask+2)+1];
    for (y = 0; y < sps->tb_mask+2; y++) {
        pps->min_tb_addr_zs_tab[y*(sps->tb_mask+2)] = -1;
        pps->min_tb_addr_zs_tab[y]    = -1;
    }
    for (y = 0; y < sps->tb_mask+1; y++) {
        for (x = 0; x < sps->tb_mask+1; x++) {
            int tb_x = x >> log2_diff;
            int tb_y = y >> log2_diff;
            int rs   = sps->ctb_width * tb_y + tb_x;
            int val  = pps->ctb_addr_rs_to_ts[rs] << (log2_diff * 2);
            for (i = 0; i < log2_diff; i++) {
                int m = 1 << i;
                val += (m & x ? m * m : 0) + (m & y ? 2 * m * m : 0);
            }
            pps->min_tb_addr_zs[y * (sps->tb_mask+2) + x] = val;
        }
    }

    return 0;
}

                           HEVCParamSets *ps)
{


        return AVERROR(ENOMEM);

                               hevc_pps_free, NULL, 0);
        av_freep(&pps);
        return AVERROR(ENOMEM);
    }


        av_log(avctx, AV_LOG_WARNING, "Truncating likely oversized PPS "
               "(%"PTRDIFF_SPECIFIER" > %"SIZE_SPECIFIER")\n",
               nal_size, sizeof(pps->data));
        pps->data_size = sizeof(pps->data);
    } else {
    }

    // Default values

    // Coded parameters
        av_log(avctx, AV_LOG_ERROR, "PPS id out of range: %d\n", pps_id);
        ret = AVERROR_INVALIDDATA;
        goto err;
    }
        av_log(avctx, AV_LOG_ERROR, "SPS id out of range: %d\n", pps->sps_id);
        ret = AVERROR_INVALIDDATA;
        goto err;
    }
    }








        av_log(avctx, AV_LOG_ERROR, "diff_cu_qp_delta_depth %d is invalid\n",
               pps->diff_cu_qp_delta_depth);
        ret = AVERROR_INVALIDDATA;
        goto err;
    }

        av_log(avctx, AV_LOG_ERROR, "pps_cb_qp_offset out of range: %d\n",
               pps->cb_qp_offset);
        ret = AVERROR_INVALIDDATA;
        goto err;
    }
        av_log(avctx, AV_LOG_ERROR, "pps_cr_qp_offset out of range: %d\n",
               pps->cr_qp_offset);
        ret = AVERROR_INVALIDDATA;
        goto err;
    }




            av_log(avctx, AV_LOG_ERROR, "num_tile_columns_minus1 out of range: %d\n",
                   num_tile_columns_minus1);
            ret = num_tile_columns_minus1 < 0 ? num_tile_columns_minus1 : AVERROR_INVALIDDATA;
            goto err;
        }
            av_log(avctx, AV_LOG_ERROR, "num_tile_rows_minus1 out of range: %d\n",
                   num_tile_rows_minus1);
            ret = num_tile_rows_minus1 < 0 ? num_tile_rows_minus1 : AVERROR_INVALIDDATA;
            goto err;
        }

            ret = AVERROR(ENOMEM);
            goto err;
        }

            uint64_t sum = 0;
            }
                av_log(avctx, AV_LOG_ERROR, "Invalid tile widths.\n");
                ret = AVERROR_INVALIDDATA;
                goto err;
            }

            }
                av_log(avctx, AV_LOG_ERROR, "Invalid tile heights.\n");
                ret = AVERROR_INVALIDDATA;
                goto err;
            }
        }
    }


                av_log(avctx, AV_LOG_ERROR, "pps_beta_offset_div2 out of range: %d\n",
                       beta_offset_div2);
                ret = AVERROR_INVALIDDATA;
                goto err;
            }
                av_log(avctx, AV_LOG_ERROR, "pps_tc_offset_div2 out of range: %d\n",
                       tc_offset_div2);
                ret = AVERROR_INVALIDDATA;
                goto err;
            }
        }
    }

            goto err;
    }
        av_log(avctx, AV_LOG_ERROR, "log2_parallel_merge_level_minus2 out of range: %d\n",
               log2_parallel_merge_level_minus2);
        ret = AVERROR_INVALIDDATA;
        goto err;
    }


                goto err;
        }
    }

        goto err;

        av_log(avctx, AV_LOG_ERROR,
               "Overread PPS by %d bits\n", -get_bits_left(gb));
        goto err;
    }



}

{



{

    else
        poc_msb = prev_poc_msb;

    // For BLA picture types, POCmsb is set to 0.
        nal_unit_type == HEVC_NAL_BLA_N_LP)

}
