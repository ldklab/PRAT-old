/*
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

{


    return 0;
}

                                 H265RawNALUnitHeader *current,
                                 int expected_nal_unit_type)
{


                            expected_nal_unit_type);
    else


}

{


    return 0;
}

static int FUNC(extension_data)(CodedBitstreamContext *ctx, RWContext *rw,
                                H265RawExtensionData *current)
{
    int err;
    size_t k;
#ifdef READ
    GetBitContext start;
    uint8_t bit;
    start = *rw;
    for (k = 0; cbs_h2645_read_more_rbsp_data(rw); k++)
        skip_bits(rw, 1);
    current->bit_length = k;
    if (k > 0) {
        *rw = start;
        allocate(current->data, (current->bit_length + 7) / 8);
        for (k = 0; k < current->bit_length; k++) {
            xu(1, extension_data, bit, 0, 1, 0);
            current->data[k / 8] |= bit << (7 - k % 8);
        }
    }
#else
    for (k = 0; k < current->bit_length; k++)
        xu(1, extension_data, current->data[k / 8] >> (7 - k % 8) & 1, 0, 1, 0);
#endif
    return 0;
}

                                    H265RawProfileTierLevel *current,
                                    int profile_present_flag,
                                    int max_num_sub_layers_minus1)
{




#define profile_compatible(x) (current->general_profile_idc == (x) || \
                               current->general_profile_compatibility_flag[x])
            flag(general_max_12bit_constraint_flag);
            flag(general_max_10bit_constraint_flag);
            flag(general_max_8bit_constraint_flag);
            flag(general_max_422chroma_constraint_flag);
            flag(general_max_420chroma_constraint_flag);
            flag(general_max_monochrome_constraint_flag);
            flag(general_intra_constraint_flag);
            flag(general_one_picture_only_constraint_flag);
            flag(general_lower_bit_rate_constraint_flag);

            if (profile_compatible(5) || profile_compatible(9) ||
                profile_compatible(10)) {
                flag(general_max_14bit_constraint_flag);
                fixed(24, general_reserved_zero_33bits, 0);
                fixed( 9, general_reserved_zero_33bits, 0);
            } else {
                fixed(24, general_reserved_zero_34bits, 0);
                fixed(10, general_reserved_zero_34bits, 0);
            }
        } else {
        }

            profile_compatible(3) || profile_compatible(4) ||
            profile_compatible(5) || profile_compatible(9)) {
        } else {
            fixed(1, general_reserved_zero_bit, 0);
        }
#undef profile_compatible
    }


    }

    }




#define profile_compatible(x) (current->sub_layer_profile_idc[i] == (x) ||   \
                               current->sub_layer_profile_compatibility_flag[i][x])
                flags(sub_layer_max_12bit_constraint_flag[i],        1, i);
                flags(sub_layer_max_10bit_constraint_flag[i],        1, i);
                flags(sub_layer_max_8bit_constraint_flag[i],         1, i);
                flags(sub_layer_max_422chroma_constraint_flag[i],    1, i);
                flags(sub_layer_max_420chroma_constraint_flag[i],    1, i);
                flags(sub_layer_max_monochrome_constraint_flag[i],   1, i);
                flags(sub_layer_intra_constraint_flag[i],            1, i);
                flags(sub_layer_one_picture_only_constraint_flag[i], 1, i);
                flags(sub_layer_lower_bit_rate_constraint_flag[i],   1, i);

                if (profile_compatible(5)) {
                    flags(sub_layer_max_14bit_constraint_flag[i], 1, i);
                    fixed(24, sub_layer_reserved_zero_33bits, 0);
                    fixed( 9, sub_layer_reserved_zero_33bits, 0);
                } else {
                    fixed(24, sub_layer_reserved_zero_34bits, 0);
                    fixed(10, sub_layer_reserved_zero_34bits, 0);
                }
                fixed(7, sub_layer_reserved_zero_7bits, 0);
                flags(sub_layer_one_picture_only_constraint_flag[i], 1, i);
                fixed(24, sub_layer_reserved_zero_43bits, 0);
                fixed(11, sub_layer_reserved_zero_43bits, 0);
            } else {
            }

                profile_compatible(3) || profile_compatible(4) ||
                profile_compatible(5) || profile_compatible(9)) {
            } else {
                fixed(1, sub_layer_reserved_zero_bit, 0);
            }
#undef profile_compatible
        }
    }

    return 0;
}

                                          H265RawHRDParameters *hrd,
                                          int nal, int sub_layer_id)
{

    else

        }
    }

    return 0;
}

                                H265RawHRDParameters *current, int common_inf_present_flag,
                                int max_num_sub_layers_minus1)
{


            current->vcl_hrd_parameters_present_flag) {
            }


        } else {
            infer(sub_pic_hrd_params_present_flag, 0);

            infer(initial_cpb_removal_delay_length_minus1, 23);
            infer(au_cpb_removal_delay_length_minus1,      23);
            infer(dpb_output_delay_length_minus1,          23);
        }
    }


        else

        } else

        else
            infer(cpb_cnt_minus1[i], 0);

    }

    return 0;
}

static int FUNC(vui_parameters)(CodedBitstreamContext *ctx, RWContext *rw,
                                H265RawVUI *current, const H265RawSPS *sps)
{
    int err;

    flag(aspect_ratio_info_present_flag);
    if (current->aspect_ratio_info_present_flag) {
        ub(8, aspect_ratio_idc);
        if (current->aspect_ratio_idc == 255) {
            ub(16, sar_width);
            ub(16, sar_height);
        }
    } else {
        infer(aspect_ratio_idc, 0);
    }

    flag(overscan_info_present_flag);
    if (current->overscan_info_present_flag)
        flag(overscan_appropriate_flag);

    flag(video_signal_type_present_flag);
    if (current->video_signal_type_present_flag) {
        ub(3, video_format);
        flag(video_full_range_flag);
        flag(colour_description_present_flag);
        if (current->colour_description_present_flag) {
            ub(8, colour_primaries);
            ub(8, transfer_characteristics);
            ub(8, matrix_coefficients);
        } else {
            infer(colour_primaries,         2);
            infer(transfer_characteristics, 2);
            infer(matrix_coefficients,      2);
        }
    } else {
        infer(video_format,             5);
        infer(video_full_range_flag,    0);
        infer(colour_primaries,         2);
        infer(transfer_characteristics, 2);
        infer(matrix_coefficients,      2);
    }

    flag(chroma_loc_info_present_flag);
    if (current->chroma_loc_info_present_flag) {
        ue(chroma_sample_loc_type_top_field,    0, 5);
        ue(chroma_sample_loc_type_bottom_field, 0, 5);
    } else {
        infer(chroma_sample_loc_type_top_field,    0);
        infer(chroma_sample_loc_type_bottom_field, 0);
    }

    flag(neutral_chroma_indication_flag);
    flag(field_seq_flag);
    flag(frame_field_info_present_flag);

    flag(default_display_window_flag);
    if (current->default_display_window_flag) {
        ue(def_disp_win_left_offset,   0, 16384);
        ue(def_disp_win_right_offset,  0, 16384);
        ue(def_disp_win_top_offset,    0, 16384);
        ue(def_disp_win_bottom_offset, 0, 16384);
    }

    flag(vui_timing_info_present_flag);
    if (current->vui_timing_info_present_flag) {
        u(32, vui_num_units_in_tick, 1, UINT32_MAX);
        u(32, vui_time_scale,        1, UINT32_MAX);
        flag(vui_poc_proportional_to_timing_flag);
        if (current->vui_poc_proportional_to_timing_flag)
            ue(vui_num_ticks_poc_diff_one_minus1, 0, UINT32_MAX - 1);

        flag(vui_hrd_parameters_present_flag);
        if (current->vui_hrd_parameters_present_flag) {
            CHECK(FUNC(hrd_parameters)(ctx, rw, &current->hrd_parameters,
                                       1, sps->sps_max_sub_layers_minus1));
        }
    }

    flag(bitstream_restriction_flag);
    if (current->bitstream_restriction_flag) {
        flag(tiles_fixed_structure_flag);
        flag(motion_vectors_over_pic_boundaries_flag);
        flag(restricted_ref_pic_lists_flag);
        ue(min_spatial_segmentation_idc,  0, 4095);
        ue(max_bytes_per_pic_denom,       0, 16);
        ue(max_bits_per_min_cu_denom,     0, 16);
        ue(log2_max_mv_length_horizontal, 0, 16);
        ue(log2_max_mv_length_vertical,   0, 16);
    } else {
        infer(tiles_fixed_structure_flag,    0);
        infer(motion_vectors_over_pic_boundaries_flag, 1);
        infer(min_spatial_segmentation_idc,  0);
        infer(max_bytes_per_pic_denom,       2);
        infer(max_bits_per_min_cu_denom,     1);
        infer(log2_max_mv_length_horizontal, 15);
        infer(log2_max_mv_length_vertical,   15);
    }

    return 0;
}

                     H265RawVPS *current)
{





        av_log(ctx->log_ctx, AV_LOG_ERROR, "Invalid stream: "
               "vps_temporal_id_nesting_flag must be 1 if "
               "vps_max_sub_layers_minus1 is 0.\n");
        return AVERROR_INVALIDDATA;
    }


                                   1, current->vps_max_sub_layers_minus1));

            0, HEVC_MAX_DPB_SIZE - 1,                        1, i);
            0, current->vps_max_dec_pic_buffering_minus1[i], 1, i);
            0, UINT32_MAX - 1,                               1, i);
    }
                  current->vps_max_dec_pic_buffering_minus1[current->vps_max_sub_layers_minus1]);
                  current->vps_max_num_reorder_pics[current->vps_max_sub_layers_minus1]);
                  current->vps_max_latency_increase_plus1[current->vps_max_sub_layers_minus1]);
        }
    }

        for (j = 0; j <= current->vps_max_layer_id; j++)
            flags(layer_id_included_flag[i][j], 2, i, j);
    }

            ues(hrd_layer_set_idx[i],
                current->vps_base_layer_internal_flag ? 0 : 1,
                current->vps_num_layer_sets_minus1, 1, i);
            if (i > 0)
                flags(cprms_present_flag[i], 1, i);
            else
                infer(cprms_present_flag[0], 1);

            CHECK(FUNC(hrd_parameters)(ctx, rw, &current->hrd_parameters[i],
                                       current->cprms_present_flag[i],
                                       current->vps_max_sub_layers_minus1));
        }
    }

        CHECK(FUNC(extension_data)(ctx, rw, &current->extension_data));


    return 0;
}

                                H265RawSTRefPicSet *current, int st_rps_idx,
                                const H265RawSPS *sps)
{

    else

                used_by_curr_pic_s1[HEVC_MAX_REFS];

        else



            else
        }
            av_log(ctx->log_ctx, AV_LOG_ERROR, "Invalid stream: "
                   "short-term ref pic set %d "
                   "contains too many pictures.\n", st_rps_idx);
            return AVERROR_INVALIDDATA;
        }

        // Since the stored form of an RPS here is actually the delta-step
        // form used when inter_ref_pic_set_prediction_flag is not set, we
        // need to reconstruct that here in order to be able to refer to
        // the RPS later (which is required for parsing, because we don't
        // even know what syntax elements appear without it).  Therefore,
        // this code takes the delta-step form of the reference set, turns
        // it into the delta-array form, applies the prediction process of
        // 7.4.8, converts the result back to the delta-step form, and
        // stores that as the current set for future use.  Note that the
        // inferences here mean that writers using prediction will need
        // to fill in the delta-step values correctly as well - since the
        // whole RPS prediction process is somewhat overly sophisticated,
        // this hopefully forms a useful check for them to ensure their
        // predicted form actually matches what was intended rather than
        // an onerous additional requirement.

        d_poc = 0;
        }
        d_poc = 0;
        }

            }
        }
        }
            }
        }

                  -(delta_poc_s0[i] - (i == 0 ? 0 : delta_poc_s0[i - 1])) - 1);
        }

                delta_poc_s1[i] = d_poc;
                used_by_curr_pic_s1[i++] = current->used_by_curr_pic_flag[j];
            }
        }
        }
            }
        }

                  delta_poc_s1[i] - (i == 0 ? 0 : delta_poc_s1[i - 1]) - 1);
        }

    } else {

        }

        }
    }

    return 0;
}

                                   H265RawScalingList *current)
{

                  2, sizeId, matrixId);
                    0, sizeId == 3 ? matrixId / 3 : matrixId,
                    2, sizeId, matrixId);
            } else {
                        2, sizeId - 2, matrixId);
                }
                        -128, +127, 3, sizeId, matrixId, i);
                }
            }
        }
    }

    return 0;
}

static int FUNC(sps_range_extension)(CodedBitstreamContext *ctx, RWContext *rw,
                                     H265RawSPS *current)
{
    int err;

    flag(transform_skip_rotation_enabled_flag);
    flag(transform_skip_context_enabled_flag);
    flag(implicit_rdpcm_enabled_flag);
    flag(explicit_rdpcm_enabled_flag);
    flag(extended_precision_processing_flag);
    flag(intra_smoothing_disabled_flag);
    flag(high_precision_offsets_enabled_flag);
    flag(persistent_rice_adaptation_enabled_flag);
    flag(cabac_bypass_alignment_enabled_flag);

    return 0;
}

static int FUNC(sps_scc_extension)(CodedBitstreamContext *ctx, RWContext *rw,
                                   H265RawSPS *current)
{
    int err, comp, i;

    flag(sps_curr_pic_ref_enabled_flag);

    flag(palette_mode_enabled_flag);
    if (current->palette_mode_enabled_flag) {
        ue(palette_max_size, 0, 64);
        ue(delta_palette_max_predictor_size, 0, 128);

        flag(sps_palette_predictor_initializer_present_flag);
        if (current->sps_palette_predictor_initializer_present_flag) {
            ue(sps_num_palette_predictor_initializer_minus1, 0, 128);
            for (comp = 0; comp < (current->chroma_format_idc ? 3 : 1); comp++) {
                int bit_depth = comp == 0 ? current->bit_depth_luma_minus8 + 8
                                          : current->bit_depth_chroma_minus8 + 8;
                for (i = 0; i <= current->sps_num_palette_predictor_initializer_minus1; i++)
                    ubs(bit_depth, sps_palette_predictor_initializers[comp][i], 2, comp, i);
            }
        }
    }

    u(2, motion_vector_resolution_control_idc, 0, 2);
    flag(intra_boundary_filtering_disable_flag);

    return 0;
}

                                        RWContext *rw, H265RawVUI *current,
                                        H265RawSPS *sps)
{




}

                     H265RawSPS *current)
{
                 min_cb_size_y,   min_tb_log2_size_y;




            av_log(ctx->log_ctx, AV_LOG_ERROR, "Invalid stream: "
                   "sps_max_sub_layers_minus1 (%d) must be less than or equal to "
                   "vps_max_sub_layers_minus1 (%d).\n",
                   vps->vps_max_sub_layers_minus1,
                   current->sps_max_sub_layers_minus1);
            return AVERROR_INVALIDDATA;
        }
            av_log(ctx->log_ctx, AV_LOG_ERROR, "Invalid stream: "
                   "sps_temporal_id_nesting_flag must be 1 if "
                   "vps_temporal_id_nesting_flag is 1.\n");
            return AVERROR_INVALIDDATA;
        }
    }

                                   1, current->sps_max_sub_layers_minus1));


        flag(separate_colour_plane_flag);
    else


    } else {
    }



            0, HEVC_MAX_DPB_SIZE - 1,                        1, i);
            0, current->sps_max_dec_pic_buffering_minus1[i], 1, i);
            0, UINT32_MAX - 1,                               1, i);
    }
                  current->sps_max_dec_pic_buffering_minus1[current->sps_max_sub_layers_minus1]);
                  current->sps_max_num_reorder_pics[current->sps_max_sub_layers_minus1]);
                  current->sps_max_latency_increase_plus1[current->sps_max_sub_layers_minus1]);
        }
    }



        av_log(ctx->log_ctx, AV_LOG_ERROR, "Invalid dimensions: %ux%u not divisible "
               "by MinCbSizeY = %u.\n", current->pic_width_in_luma_samples,
               current->pic_height_in_luma_samples, min_cb_size_y);
        return AVERROR_INVALIDDATA;
    }


       0, FFMIN(ctb_log2_size_y, 5) - min_tb_log2_size_y);

       0, ctb_log2_size_y - min_tb_log2_size_y);
       0, ctb_log2_size_y - min_tb_log2_size_y);

    } else {
    }


          0, current->bit_depth_luma_minus8 + 8 - 1);
          0, current->bit_depth_chroma_minus8 + 8 - 1);

           FFMIN(min_cb_log2_size_y, 5) - 3, FFMIN(ctb_log2_size_y, 5) - 3);
           0, FFMIN(ctb_log2_size_y, 5) - (current->log2_min_pcm_luma_coding_block_size_minus3 + 3));

    }


                lt_ref_pic_poc_lsb_sps[i], 1, i);
        }
    }


    else

        flag(sps_range_extension_flag);
        flag(sps_multilayer_extension_flag);
        flag(sps_3d_extension_flag);
        flag(sps_scc_extension_flag);
        ub(4, sps_extension_4bits);
    }

        CHECK(FUNC(sps_range_extension)(ctx, rw, current));
        return AVERROR_PATCHWELCOME;
        return AVERROR_PATCHWELCOME;
        CHECK(FUNC(sps_scc_extension)(ctx, rw, current));
        CHECK(FUNC(extension_data)(ctx, rw, &current->extension_data));


    return 0;
}

static int FUNC(pps_range_extension)(CodedBitstreamContext *ctx, RWContext *rw,
                                     H265RawPPS *current)
{
    CodedBitstreamH265Context *h265 = ctx->priv_data;
    const H265RawSPS *sps = h265->active_sps;
    int err, i;

    if (current->transform_skip_enabled_flag)
        ue(log2_max_transform_skip_block_size_minus2, 0, 3);
    flag(cross_component_prediction_enabled_flag);

    flag(chroma_qp_offset_list_enabled_flag);
    if (current->chroma_qp_offset_list_enabled_flag) {
        ue(diff_cu_chroma_qp_offset_depth,
           0, sps->log2_diff_max_min_luma_coding_block_size);
        ue(chroma_qp_offset_list_len_minus1, 0, 5);
        for (i = 0; i <= current->chroma_qp_offset_list_len_minus1; i++) {
            ses(cb_qp_offset_list[i], -12, +12, 1, i);
            ses(cr_qp_offset_list[i], -12, +12, 1, i);
        }
    }

    ue(log2_sao_offset_scale_luma,   0, FFMAX(0, sps->bit_depth_luma_minus8   - 2));
    ue(log2_sao_offset_scale_chroma, 0, FFMAX(0, sps->bit_depth_chroma_minus8 - 2));

    return 0;
}

static int FUNC(pps_scc_extension)(CodedBitstreamContext *ctx, RWContext *rw,
                                   H265RawPPS *current)
{
    int err, comp, i;

    flag(pps_curr_pic_ref_enabled_flag);

    flag(residual_adaptive_colour_transform_enabled_flag);
    if (current->residual_adaptive_colour_transform_enabled_flag) {
        flag(pps_slice_act_qp_offsets_present_flag);
        se(pps_act_y_qp_offset_plus5,  -7, +17);
        se(pps_act_cb_qp_offset_plus5, -7, +17);
        se(pps_act_cr_qp_offset_plus3, -9, +15);
    } else {
        infer(pps_slice_act_qp_offsets_present_flag, 0);
        infer(pps_act_y_qp_offset_plus5,  0);
        infer(pps_act_cb_qp_offset_plus5, 0);
        infer(pps_act_cr_qp_offset_plus3, 0);
    }

    flag(pps_palette_predictor_initializer_present_flag);
    if (current->pps_palette_predictor_initializer_present_flag) {
        ue(pps_num_palette_predictor_initializer, 0, 128);
        if (current->pps_num_palette_predictor_initializer > 0) {
            flag(monochrome_palette_flag);
            ue(luma_bit_depth_entry_minus8, 0, 8);
            if (!current->monochrome_palette_flag)
                ue(chroma_bit_depth_entry_minus8, 0, 8);
            for (comp = 0; comp < (current->monochrome_palette_flag ? 1 : 3); comp++) {
                int bit_depth = comp == 0 ? current->luma_bit_depth_entry_minus8 + 8
                                          : current->chroma_bit_depth_entry_minus8 + 8;
                for (i = 0; i < current->pps_num_palette_predictor_initializer; i++)
                    ubs(bit_depth, pps_palette_predictor_initializers[comp][i], 2, comp, i);
            }
        }
    }

    return 0;
}

                     H265RawPPS *current)
{



        av_log(ctx->log_ctx, AV_LOG_ERROR, "SPS id %d not available.\n",
               current->pps_seq_parameter_set_id);
        return AVERROR_INVALIDDATA;
    }




           0, sps->log2_diff_max_min_luma_coding_block_size);
    else




        }
    } else {
    }

        } else {
        }
    } else {
    }



       0, (sps->log2_min_luma_coding_block_size_minus3 + 3 +
           sps->log2_diff_max_min_luma_coding_block_size - 2));


        flag(pps_range_extension_flag);
        flag(pps_multilayer_extension_flag);
        flag(pps_3d_extension_flag);
        flag(pps_scc_extension_flag);
        ub(4, pps_extension_4bits);
    }
        CHECK(FUNC(pps_range_extension)(ctx, rw, current));
        return AVERROR_PATCHWELCOME;
        return AVERROR_PATCHWELCOME;
        CHECK(FUNC(pps_scc_extension)(ctx, rw, current));
        CHECK(FUNC(extension_data)(ctx, rw, &current->extension_data));


    return 0;
}

                     H265RawAUD *current)
{





    return 0;
}

                                            H265RawSliceHeader *current,
                                            unsigned int num_pic_total_curr)
{


    }

        }
    }

    return 0;
}

                                   H265RawSliceHeader *current)
{

    else
        infer(delta_chroma_log2_weight_denom, 0);

        else
    }
            else
        }
    }

                -(1 << (sps->bit_depth_luma_minus8 + 8 - 1)),
                ((1 << (sps->bit_depth_luma_minus8 + 8 - 1)) - 1), 1, i);
        } else {
        }
                    -(4 << (sps->bit_depth_chroma_minus8 + 8 - 1)),
                    ((4 << (sps->bit_depth_chroma_minus8 + 8 - 1)) - 1), 2, i, j);
            }
        } else {
            }
        }
    }

        for (i = 0; i <= current->num_ref_idx_l1_active_minus1; i++) {
            if (1 /* RefPicList1[i] is not CurrPic, nor is it in a different layer */)
                flags(luma_weight_l1_flag[i], 1, i);
            else
                infer(luma_weight_l1_flag[i], 0);
        }
        if (chroma) {
            for (i = 0; i <= current->num_ref_idx_l1_active_minus1; i++) {
                if (1 /* RefPicList1[i] is not CurrPic, nor is it in a different layer */)
                    flags(chroma_weight_l1_flag[i], 1, i);
                else
                    infer(chroma_weight_l1_flag[i], 0);
            }
        }

        for (i = 0; i <= current->num_ref_idx_l1_active_minus1; i++) {
            if (current->luma_weight_l1_flag[i]) {
                ses(delta_luma_weight_l1[i], -128, +127, 1, i);
                ses(luma_offset_l1[i],
                    -(1 << (sps->bit_depth_luma_minus8 + 8 - 1)),
                    ((1 << (sps->bit_depth_luma_minus8 + 8 - 1)) - 1), 1, i);
            } else {
                infer(delta_luma_weight_l1[i], 0);
                infer(luma_offset_l1[i],       0);
            }
            if (current->chroma_weight_l1_flag[i]) {
                for (j = 0; j < 2; j++) {
                    ses(delta_chroma_weight_l1[i][j], -128, +127, 2, i, j);
                    ses(chroma_offset_l1[i][j],
                        -(4 << (sps->bit_depth_chroma_minus8 + 8 - 1)),
                        ((4 << (sps->bit_depth_chroma_minus8 + 8 - 1)) - 1), 2, i, j);
                }
            } else {
                for (j = 0; j < 2; j++) {
                    infer(delta_chroma_weight_l1[i][j], 0);
                    infer(chroma_offset_l1[i][j],       0);
                }
            }
        }
    }

    return 0;
}

                                      H265RawSliceHeader *current)
{




        current->nal_unit_header.nal_unit_type <= HEVC_NAL_RSV_IRAP_VCL23)


        av_log(ctx->log_ctx, AV_LOG_ERROR, "PPS id %d not available.\n",
               current->slice_pic_parameter_set_id);
        return AVERROR_INVALIDDATA;
    }

        av_log(ctx->log_ctx, AV_LOG_ERROR, "SPS id %d not available.\n",
               pps->pps_seq_parameter_set_id);
        return AVERROR_INVALIDDATA;
    }


        else
    } else {
    }

            flags(slice_reserved_flag[i], 1, i);


            flag(pic_output_flag);

            u(2, colour_plane_id, 0, 2);

            current->nal_unit_header.nal_unit_type != HEVC_NAL_IDR_N_LP) {


                                           sps->num_short_term_ref_pic_sets, sps));
                rps = &current->short_term_ref_pic_set;
                  0, sps->num_short_term_ref_pic_sets - 1);
            } else {
                infer(short_term_ref_pic_set_idx, 0);
                rps = &sps->st_ref_pic_set[0];
            }



                } else {
                }

                               0, sps->num_long_term_ref_pics_sps - 1, 1, i);
                    } else {
                    }
                        ues(delta_poc_msb_cycle_lt[i], 0, UINT32_MAX - 1, 1, i);
                    else
                }
            }

            else

                ++num_pic_total_curr;
        }

            else
                infer(slice_sao_chroma_flag, 0);
        } else {
        }

            current->slice_type == HEVC_SLICE_B) {
                else
            } else {
            }

                                                       num_pic_total_curr));

            else
                else
                    else
                } else {
                    else
                }
            }


                flag(use_integer_mv_flag);
            else
        }

           - 6 * sps->bit_depth_luma_minus8 - (pps->init_qp_minus26 + 26),
           + 51 - (pps->init_qp_minus26 + 26));
            se(slice_cb_qp_offset, -12, +12);
            se(slice_cr_qp_offset, -12, +12);
        } else {
        }
            se(slice_act_y_qp_offset,
               -12 - (pps->pps_act_y_qp_offset_plus5 - 5),
               +12 - (pps->pps_act_y_qp_offset_plus5 - 5));
            se(slice_act_cb_qp_offset,
               -12 - (pps->pps_act_cb_qp_offset_plus5 - 5),
               +12 - (pps->pps_act_cb_qp_offset_plus5 - 5));
            se(slice_act_cr_qp_offset,
               -12 - (pps->pps_act_cr_qp_offset_plus3 - 3),
               +12 - (pps->pps_act_cr_qp_offset_plus3 - 3));
        } else {
        }
            flag(cu_chroma_qp_offset_enabled_flag);
        else

        else
                se(slice_beta_offset_div2, -6, +6);
                se(slice_tc_offset_div2,   -6, +6);
            } else {
            }
        } else {
                  pps->pps_deblocking_filter_disabled_flag);
        }
        else
                  pps->pps_loop_filter_across_slices_enabled_flag);
    }

        else
            num_entry_point_offsets_limit =
                (pps->num_tile_columns_minus1 + 1) * pic_height_in_ctbs_y - 1;

            av_log(ctx->log_ctx, AV_LOG_ERROR, "Too many entry points: "
                   "%"PRIu16".\n", current->num_entry_point_offsets);
            return AVERROR_PATCHWELCOME;
        }

        }
    }

    }


    return 0;
}

static int FUNC(sei_buffering_period)(CodedBitstreamContext *ctx, RWContext *rw,
                                      H265RawSEIBufferingPeriod *current,
                                      uint32_t *payload_size,
                                      int *more_data)
{
    CodedBitstreamH265Context *h265 = ctx->priv_data;
    const H265RawSPS *sps;
    const H265RawHRDParameters *hrd;
    int err, i, length;

#ifdef READ
    int start_pos, end_pos;
    start_pos = get_bits_count(rw);
#endif

    HEADER("Buffering Period");

    ue(bp_seq_parameter_set_id, 0, HEVC_MAX_SPS_COUNT - 1);

    sps = h265->sps[current->bp_seq_parameter_set_id];
    if (!sps) {
        av_log(ctx->log_ctx, AV_LOG_ERROR, "SPS id %d not available.\n",
               current->bp_seq_parameter_set_id);
        return AVERROR_INVALIDDATA;
    }
    h265->active_sps = sps;

    if (!sps->vui_parameters_present_flag ||
        !sps->vui.vui_hrd_parameters_present_flag) {
        av_log(ctx->log_ctx, AV_LOG_ERROR, "Buffering period SEI requires "
               "HRD parameters to be present in SPS.\n");
        return AVERROR_INVALIDDATA;
    }
    hrd = &sps->vui.hrd_parameters;
    if (!hrd->nal_hrd_parameters_present_flag &&
        !hrd->vcl_hrd_parameters_present_flag) {
        av_log(ctx->log_ctx, AV_LOG_ERROR, "Buffering period SEI requires "
               "NAL or VCL HRD parameters to be present.\n");
        return AVERROR_INVALIDDATA;
    }

    if (!hrd->sub_pic_hrd_params_present_flag)
        flag(irap_cpb_params_present_flag);
    else
        infer(irap_cpb_params_present_flag, 0);
    if (current->irap_cpb_params_present_flag) {
        length = hrd->au_cpb_removal_delay_length_minus1 + 1;
        ub(length, cpb_delay_offset);
        length = hrd->dpb_output_delay_length_minus1 + 1;
        ub(length, dpb_delay_offset);
    } else {
        infer(cpb_delay_offset, 0);
        infer(dpb_delay_offset, 0);
    }

    flag(concatenation_flag);

    length = hrd->au_cpb_removal_delay_length_minus1 + 1;
    ub(length, au_cpb_removal_delay_delta_minus1);

    if (hrd->nal_hrd_parameters_present_flag) {
        for (i = 0; i <= hrd->cpb_cnt_minus1[0]; i++) {
            length = hrd->initial_cpb_removal_delay_length_minus1 + 1;

            ubs(length, nal_initial_cpb_removal_delay[i], 1, i);
            ubs(length, nal_initial_cpb_removal_offset[i], 1, i);

            if (hrd->sub_pic_hrd_params_present_flag ||
                current->irap_cpb_params_present_flag) {
                ubs(length, nal_initial_alt_cpb_removal_delay[i], 1, i);
                ubs(length, nal_initial_alt_cpb_removal_offset[i], 1, i);
            }
        }
    }
    if (hrd->vcl_hrd_parameters_present_flag) {
        for (i = 0; i <= hrd->cpb_cnt_minus1[0]; i++) {
            length = hrd->initial_cpb_removal_delay_length_minus1 + 1;

            ubs(length, vcl_initial_cpb_removal_delay[i], 1, i);
            ubs(length, vcl_initial_cpb_removal_offset[i], 1, i);

            if (hrd->sub_pic_hrd_params_present_flag ||
                current->irap_cpb_params_present_flag) {
                ubs(length, vcl_initial_alt_cpb_removal_delay[i], 1, i);
                ubs(length, vcl_initial_alt_cpb_removal_offset[i], 1, i);
            }
        }
    }

#ifdef READ
    end_pos = get_bits_count(rw);
    if (cbs_h265_payload_extension_present(rw, *payload_size,
                                           end_pos - start_pos))
        flag(use_alt_cpb_params_flag);
    else
        infer(use_alt_cpb_params_flag, 0);
#else
    // If unknown extension data exists, then use_alt_cpb_params_flag is
    // coded in the bitstream and must be written even if it's 0.
    if (current->use_alt_cpb_params_flag || *more_data) {
        flag(use_alt_cpb_params_flag);
        // Ensure this bit is not the last in the payload by making the
        // more_data_in_payload() check evaluate to true, so it may not
        // be mistaken as something else by decoders.
        *more_data = 1;
    }
#endif

    return 0;
}

                                H265RawSEIPicTiming *current)
{


        av_log(ctx->log_ctx, AV_LOG_ERROR,
               "No active SPS for pic_timing.\n");
        return AVERROR_INVALIDDATA;
    }


          expected_source_scan_type >= 0 ? expected_source_scan_type : 0,
          expected_source_scan_type >= 0 ? expected_source_scan_type : 2);
    } else {
              expected_source_scan_type >= 0 ? expected_source_scan_type : 2);
    }

    else
        hrd = NULL;
                hrd->vcl_hrd_parameters_present_flag)) {


        }

            // Each decoding unit must contain at least one slice segment.

                ub(length, du_common_cpb_removal_delay_increment_minus1);

                    0, HEVC_MAX_SLICE_SEGMENTS, 1, i);
            }
        }
    }

    return 0;
}

static int FUNC(sei_pan_scan_rect)(CodedBitstreamContext *ctx, RWContext *rw,
                                   H265RawSEIPanScanRect *current)
{
    int err, i;

    HEADER("Pan-Scan Rectangle");

    ue(pan_scan_rect_id, 0, UINT32_MAX - 1);
    flag(pan_scan_rect_cancel_flag);

    if (!current->pan_scan_rect_cancel_flag) {
        ue(pan_scan_cnt_minus1, 0, 2);

        for (i = 0; i <= current->pan_scan_cnt_minus1; i++) {
            ses(pan_scan_rect_left_offset[i],   INT32_MIN + 1, INT32_MAX, 1, i);
            ses(pan_scan_rect_right_offset[i],  INT32_MIN + 1, INT32_MAX, 1, i);
            ses(pan_scan_rect_top_offset[i],    INT32_MIN + 1, INT32_MAX, 1, i);
            ses(pan_scan_rect_bottom_offset[i], INT32_MIN + 1, INT32_MAX, 1, i);
        }

        flag(pan_scan_rect_persistence_flag);
    }

    return 0;
}

static int FUNC(sei_user_data_registered)(CodedBitstreamContext *ctx, RWContext *rw,
                                          H265RawSEIUserDataRegistered *current,
                                          uint32_t *payload_size)
{
    int err, i, j;

    HEADER("User Data Registered ITU-T T.35");

    u(8, itu_t_t35_country_code, 0x00, 0xff);
    if (current->itu_t_t35_country_code != 0xff)
        i = 1;
    else {
        u(8, itu_t_t35_country_code_extension_byte, 0x00, 0xff);
        i = 2;
    }

#ifdef READ
    if (*payload_size < i) {
        av_log(ctx->log_ctx, AV_LOG_ERROR,
               "Invalid SEI user data registered payload.\n");
        return AVERROR_INVALIDDATA;
    }
    current->data_length = *payload_size - i;
#else
    *payload_size = i + current->data_length;
#endif

    allocate(current->data, current->data_length);
    for (j = 0; j < current->data_length; j++)
        xu(8, itu_t_t35_payload_byte[i], current->data[j], 0x00, 0xff, 1, i + j);

    return 0;
}

static int FUNC(sei_user_data_unregistered)(CodedBitstreamContext *ctx, RWContext *rw,
                                            H265RawSEIUserDataUnregistered *current,
                                            uint32_t *payload_size)
{
    int err, i;

    HEADER("User Data Unregistered");

#ifdef READ
    if (*payload_size < 16) {
        av_log(ctx->log_ctx, AV_LOG_ERROR,
               "Invalid SEI user data unregistered payload.\n");
        return AVERROR_INVALIDDATA;
    }
    current->data_length = *payload_size - 16;
#else
    *payload_size = 16 + current->data_length;
#endif

    for (i = 0; i < 16; i++)
        us(8, uuid_iso_iec_11578[i], 0x00, 0xff, 1, i);

    allocate(current->data, current->data_length);

    for (i = 0; i < current->data_length; i++)
        xu(8, user_data_payload_byte[i], current->data[i], 0x00, 0xff, 1, i);

    return 0;
}

                                    H265RawSEIRecoveryPoint *current)
{




}

static int FUNC(sei_display_orientation)(CodedBitstreamContext *ctx, RWContext *rw,
                                         H265RawSEIDisplayOrientation *current)
{
    int err;

    HEADER("Display Orientation");

    flag(display_orientation_cancel_flag);
    if (!current->display_orientation_cancel_flag) {
        flag(hor_flip);
        flag(ver_flip);
        ub(16, anticlockwise_rotation);
        flag(display_orientation_persistence_flag);
    }

    return 0;
}

                                           H265RawSEIActiveParameterSets *current)
{


        av_log(ctx->log_ctx, AV_LOG_ERROR, "VPS id %d not available for active "
               "parameter sets.\n", current->active_video_parameter_set_id);
        return AVERROR_INVALIDDATA;
    }



        ues(layer_sps_idx[i], 0, current->num_sps_ids_minus1, 1, i);

        if (i == 0)
            h265->active_sps = h265->sps[current->active_seq_parameter_set_id[current->layer_sps_idx[0]]];
    }

    return 0;
}

                                          H265RawSEIDecodedPictureHash *current)
{


        av_log(ctx->log_ctx, AV_LOG_ERROR,
               "No active SPS for decoded picture hash.\n");
        return AVERROR_INVALIDDATA;
    }


            us(16, picture_crc[c], 0x0000, 0xffff, 1, c);
        }
    }

    return 0;
}

static int FUNC(sei_time_code)(CodedBitstreamContext *ctx, RWContext *rw,
                               H265RawSEITimeCode *current)
{
    int err, i;

    HEADER("Time Code");

    u(2, num_clock_ts, 1, 3);

    for (i = 0; i < current->num_clock_ts; i++) {
        flags(clock_timestamp_flag[i],   1, i);

        if (current->clock_timestamp_flag[i]) {
            flags(units_field_based_flag[i], 1, i);
            us(5, counting_type[i], 0, 6,    1, i);
            flags(full_timestamp_flag[i],    1, i);
            flags(discontinuity_flag[i],     1, i);
            flags(cnt_dropped_flag[i],       1, i);

            ubs(9, n_frames[i], 1, i);

            if (current->full_timestamp_flag[i]) {
                us(6, seconds_value[i], 0, 59, 1, i);
                us(6, minutes_value[i], 0, 59, 1, i);
                us(5, hours_value[i],   0, 23, 1, i);
            } else {
                flags(seconds_flag[i], 1, i);
                if (current->seconds_flag[i]) {
                    us(6, seconds_value[i], 0, 59, 1, i);
                    flags(minutes_flag[i], 1, i);
                    if (current->minutes_flag[i]) {
                        us(6, minutes_value[i], 0, 59, 1, i);
                        flags(hours_flag[i], 1, i);
                        if (current->hours_flag[i])
                            us(5, hours_value[i], 0, 23, 1, i);
                    }
                }
            }

            ubs(5, time_offset_length[i], 1, i);
            if (current->time_offset_length[i] > 0)
                ibs(current->time_offset_length[i], time_offset_value[i], 1, i);
            else
                infer(time_offset_value[i], 0);
        }
    }

    return 0;
}

static int FUNC(sei_mastering_display)(CodedBitstreamContext *ctx, RWContext *rw,
                                       H265RawSEIMasteringDisplayColourVolume *current)
{
    int err, c;

    HEADER("Mastering Display Colour Volume");

    for (c = 0; c < 3; c++) {
        us(16, display_primaries_x[c], 0, 50000, 1, c);
        us(16, display_primaries_y[c], 0, 50000, 1, c);
    }

    u(16, white_point_x, 0, 50000);
    u(16, white_point_y, 0, 50000);

    u(32, max_display_mastering_luminance,
      1, MAX_UINT_BITS(32));
    u(32, min_display_mastering_luminance,
      0, current->max_display_mastering_luminance - 1);

    return 0;
}

static int FUNC(sei_content_light_level)(CodedBitstreamContext *ctx, RWContext *rw,
                                         H265RawSEIContentLightLevelInfo *current)
{
    int err;

    HEADER("Content Light Level");

    ub(16, max_content_light_level);
    ub(16, max_pic_average_light_level);

    return 0;
}

static int FUNC(sei_alternative_transfer_characteristics)(CodedBitstreamContext *ctx,
                                                          RWContext *rw,
                                                          H265RawSEIAlternativeTransferCharacteristics *current)
{
    int err;

    HEADER("Alternative Transfer Characteristics");

    ub(8, preferred_transfer_characteristics);

    return 0;
}

static int FUNC(sei_alpha_channel_info)(CodedBitstreamContext *ctx,
                                        RWContext *rw,
                                        H265RawSEIAlphaChannelInfo *current)
{
    int err, length;

    HEADER("Alpha Channel Information");

    flag(alpha_channel_cancel_flag);
    if (!current->alpha_channel_cancel_flag) {
        ub(3, alpha_channel_use_idc);
        ub(3, alpha_channel_bit_depth_minus8);
        length = current->alpha_channel_bit_depth_minus8 + 9;
        ub(length, alpha_transparent_value);
        ub(length, alpha_opaque_value);
        flag(alpha_channel_incr_flag);
        flag(alpha_channel_clip_flag);
        if (current->alpha_channel_clip_flag)
            flag(alpha_channel_clip_type_flag);
    } else {
       infer(alpha_channel_use_idc,   2);
       infer(alpha_channel_incr_flag, 0);
       infer(alpha_channel_clip_flag, 0);
    }

    return 0;
}

                                   H265RawExtensionData *current, uint32_t payload_size,
                                   int cur_pos)
{

#ifdef READ

        return 0;

    bits_left = 8 * payload_size - cur_pos;
    tmp = *rw;
    if (bits_left > 8)
        skip_bits_long(&tmp, bits_left - 8);
    payload_zero_bits = get_bits(&tmp, FFMIN(bits_left, 8));
    if (!payload_zero_bits)
        return AVERROR_INVALIDDATA;
    payload_zero_bits = ff_ctz(payload_zero_bits);
    current->bit_length = bits_left - payload_zero_bits - 1;
    allocate(current->data, (current->bit_length + 7) / 8);
#endif

    byte_length = (current->bit_length + 7) / 8;
    for (k = 0; k < byte_length; k++) {
        int length = FFMIN(current->bit_length - k * 8, 8);
        xu(length, reserved_payload_extension_data, current->data[k],
           0, MAX_UINT_BITS(length), 0);
    }

    return 0;
}

                             H265RawSEIPayload *current, int prefix)
{

#ifdef READ
#else
#endif

#define SEI_TYPE_CHECK_VALID(name, prefix_valid, suffix_valid) do { \
            if (prefix && !prefix_valid) { \
                av_log(ctx->log_ctx, AV_LOG_ERROR, "SEI type %s invalid " \
                       "as prefix SEI!\n", #name); \
                return AVERROR_INVALIDDATA; \
            } \
            if (!prefix && !suffix_valid) { \
                av_log(ctx->log_ctx, AV_LOG_ERROR, "SEI type %s invalid " \
                       "as suffix SEI!\n", #name); \
                return AVERROR_INVALIDDATA; \
            } \
        } while (0)
#define SEI_TYPE_N(type, prefix_valid, suffix_valid, name) \
    case HEVC_SEI_TYPE_ ## type: \
        SEI_TYPE_CHECK_VALID(name, prefix_valid, suffix_valid); \
        CHECK(FUNC(sei_ ## name)(ctx, rw, &current->payload.name)); \
        break
#define SEI_TYPE_S(type, prefix_valid, suffix_valid, name) \
    case HEVC_SEI_TYPE_ ## type: \
        SEI_TYPE_CHECK_VALID(name, prefix_valid, suffix_valid); \
        CHECK(FUNC(sei_ ## name)(ctx, rw, &current->payload.name, \
                                 &current->payload_size)); \
        break
#define SEI_TYPE_E(type, prefix_valid, suffix_valid, name) \
    case HEVC_SEI_TYPE_ ## type: \
        SEI_TYPE_CHECK_VALID(name, prefix_valid, suffix_valid); \
        CHECK(FUNC(sei_ ## name)(ctx, rw, &current->payload.name, \
                                 &current->payload_size, \
                                 &more_data)); \
        break

        SEI_TYPE_N(PAN_SCAN_RECT,            1, 0, pan_scan_rect);
        SEI_TYPE_S(USER_DATA_REGISTERED_ITU_T_T35,
                                             1, 1, user_data_registered);
        SEI_TYPE_S(USER_DATA_UNREGISTERED,   1, 1, user_data_unregistered);
        SEI_TYPE_N(DISPLAY_ORIENTATION,      1, 0, display_orientation);
        SEI_TYPE_N(TIME_CODE,                1, 0, time_code);
        SEI_TYPE_N(MASTERING_DISPLAY_INFO,   1, 0, mastering_display);
        SEI_TYPE_N(CONTENT_LIGHT_LEVEL_INFO, 1, 0, content_light_level);
        SEI_TYPE_N(ALTERNATIVE_TRANSFER_CHARACTERISTICS,
                                             1, 0, alternative_transfer_characteristics);
        SEI_TYPE_N(ALPHA_CHANNEL_INFO,       1, 0, alpha_channel_info);

#undef SEI_TYPE
    default:
        {
#ifdef READ
            current->payload.other.data_length = current->payload_size;
#endif
            allocate(current->payload.other.data, current->payload.other.data_length);

            for (i = 0; i < current->payload_size; i++)
                xu(8, payload_byte[i], current->payload.other.data[i], 0, 255,
                   1, i);
        }
    }

    // more_data_in_payload()
#ifdef READ
#else
#endif
                                      current->payload_size, current_position));
    }

#ifdef WRITE
#endif

}

                     H265RawSEI *current, int prefix)
{

    else

                                prefix ? HEVC_NAL_SEI_PREFIX
                                       : HEVC_NAL_SEI_SUFFIX));

#ifdef READ
        uint32_t payload_type = 0;
        uint32_t tmp;

            fixed(8, ff_byte, 0xff);
            payload_type += 255;
        }

            fixed(8, ff_byte, 0xff);
            payload_size += 255;
        }



            break;
    }
        av_log(ctx->log_ctx, AV_LOG_ERROR, "Too many payloads in "
               "SEI message: found %d.\n", k);
        return AVERROR_INVALIDDATA;
    }
#else

        // Somewhat clumsy: we write the payload twice when
        // we don't know the size in advance.  This will mess
        // with trace output, but is otherwise harmless.

                fixed(8, ff_byte, 0xff);
                tmp -= 255;
            }

                fixed(8, ff_byte, 0xff);
                tmp -= 255;
            }

        }
    }
#endif


    return 0;
}
