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

                                 H264RawNALUnitHeader *current,
                                 uint32_t valid_type_mask)
{


        av_log(ctx->log_ctx, AV_LOG_ERROR, "Invalid NAL unit type %d.\n",
               current->nal_unit_type);
        return AVERROR_INVALIDDATA;
    }

        current->nal_unit_type == 21) {
        if (current->nal_unit_type != 21)
            flag(svc_extension_flag);
        else
            flag(avc_3d_extension_flag);

        if (current->svc_extension_flag) {
            av_log(ctx->log_ctx, AV_LOG_ERROR, "SVC not supported.\n");
            return AVERROR_PATCHWELCOME;

        } else if (current->avc_3d_extension_flag) {
            av_log(ctx->log_ctx, AV_LOG_ERROR, "3DAVC not supported.\n");
            return AVERROR_PATCHWELCOME;

        } else {
            av_log(ctx->log_ctx, AV_LOG_ERROR, "MVC not supported.\n");
            return AVERROR_PATCHWELCOME;
        }
    }

    return 0;
}

static int FUNC(scaling_list)(CodedBitstreamContext *ctx, RWContext *rw,
                              H264RawScalingList *current,
                              int size_of_scaling_list)
{
    int err, i, scale;

    scale = 8;
    for (i = 0; i < size_of_scaling_list; i++) {
        ses(delta_scale[i], -128, +127, 1, i);
        scale = (scale + current->delta_scale[i] + 256) % 256;
        if (scale == 0)
            break;
    }

    return 0;
}

                                H264RawHRD *current)
{


    }


}

static int FUNC(vui_parameters)(CodedBitstreamContext *ctx, RWContext *rw,
                                H264RawVUI *current, H264RawSPS *sps)
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

    flag(timing_info_present_flag);
    if (current->timing_info_present_flag) {
        u(32, num_units_in_tick, 1, UINT32_MAX);
        u(32, time_scale,        1, UINT32_MAX);
        flag(fixed_frame_rate_flag);
    } else {
        infer(fixed_frame_rate_flag, 0);
    }

    flag(nal_hrd_parameters_present_flag);
    if (current->nal_hrd_parameters_present_flag)
        CHECK(FUNC(hrd_parameters)(ctx, rw, &current->nal_hrd_parameters));

    flag(vcl_hrd_parameters_present_flag);
    if (current->vcl_hrd_parameters_present_flag)
        CHECK(FUNC(hrd_parameters)(ctx, rw, &current->vcl_hrd_parameters));

    if (current->nal_hrd_parameters_present_flag ||
        current->vcl_hrd_parameters_present_flag)
        flag(low_delay_hrd_flag);
    else
        infer(low_delay_hrd_flag, 1 - current->fixed_frame_rate_flag);

    flag(pic_struct_present_flag);

    flag(bitstream_restriction_flag);
    if (current->bitstream_restriction_flag) {
        flag(motion_vectors_over_pic_boundaries_flag);
        ue(max_bytes_per_pic_denom, 0, 16);
        ue(max_bits_per_mb_denom,   0, 16);
        // The current version of the standard constrains this to be in
        // [0,15], but older versions allow 16.
        ue(log2_max_mv_length_horizontal, 0, 16);
        ue(log2_max_mv_length_vertical,   0, 16);
        ue(max_num_reorder_frames,  0, H264_MAX_DPB_FRAMES);
        ue(max_dec_frame_buffering, 0, H264_MAX_DPB_FRAMES);
    } else {
        infer(motion_vectors_over_pic_boundaries_flag, 1);
        infer(max_bytes_per_pic_denom, 2);
        infer(max_bits_per_mb_denom,   1);
        infer(log2_max_mv_length_horizontal, 15);
        infer(log2_max_mv_length_vertical,   15);

        if ((sps->profile_idc ==  44 || sps->profile_idc ==  86 ||
             sps->profile_idc == 100 || sps->profile_idc == 110 ||
             sps->profile_idc == 122 || sps->profile_idc == 244) &&
            sps->constraint_set3_flag) {
            infer(max_num_reorder_frames,  0);
            infer(max_dec_frame_buffering, 0);
        } else {
            infer(max_num_reorder_frames,  H264_MAX_DPB_FRAMES);
            infer(max_dec_frame_buffering, H264_MAX_DPB_FRAMES);
        }
    }

    return 0;
}

static int FUNC(vui_parameters_default)(CodedBitstreamContext *ctx,
                                        RWContext *rw, H264RawVUI *current,
                                        H264RawSPS *sps)
{
    infer(aspect_ratio_idc, 0);

    infer(video_format,             5);
    infer(video_full_range_flag,    0);
    infer(colour_primaries,         2);
    infer(transfer_characteristics, 2);
    infer(matrix_coefficients,      2);

    infer(chroma_sample_loc_type_top_field,    0);
    infer(chroma_sample_loc_type_bottom_field, 0);

    infer(fixed_frame_rate_flag, 0);
    infer(low_delay_hrd_flag,    1);

    infer(pic_struct_present_flag, 0);

    infer(motion_vectors_over_pic_boundaries_flag, 1);
    infer(max_bytes_per_pic_denom, 2);
    infer(max_bits_per_mb_denom,   1);
    infer(log2_max_mv_length_horizontal, 15);
    infer(log2_max_mv_length_vertical,   15);

    if ((sps->profile_idc ==  44 || sps->profile_idc ==  86 ||
         sps->profile_idc == 100 || sps->profile_idc == 110 ||
         sps->profile_idc == 122 || sps->profile_idc == 244) &&
        sps->constraint_set3_flag) {
        infer(max_num_reorder_frames,  0);
        infer(max_dec_frame_buffering, 0);
    } else {
        infer(max_num_reorder_frames,  H264_MAX_DPB_FRAMES);
        infer(max_dec_frame_buffering, H264_MAX_DPB_FRAMES);
    }

    return 0;
}

                     H264RawSPS *current)
{


                                1 << H264_NAL_SPS));






        ue(chroma_format_idc, 0, 3);

        if (current->chroma_format_idc == 3)
            flag(separate_colour_plane_flag);
        else
            infer(separate_colour_plane_flag, 0);

        ue(bit_depth_luma_minus8,   0, 6);
        ue(bit_depth_chroma_minus8, 0, 6);

        flag(qpprime_y_zero_transform_bypass_flag);

        flag(seq_scaling_matrix_present_flag);
        if (current->seq_scaling_matrix_present_flag) {
            for (i = 0; i < ((current->chroma_format_idc != 3) ? 8 : 12); i++) {
                flags(seq_scaling_list_present_flag[i], 1, i);
                if (current->seq_scaling_list_present_flag[i]) {
                    if (i < 6)
                        CHECK(FUNC(scaling_list)(ctx, rw,
                                                 &current->scaling_list_4x4[i],
                                                 16));
                    else
                        CHECK(FUNC(scaling_list)(ctx, rw,
                                                 &current->scaling_list_8x8[i - 6],
                                                 64));
                }
            }
        }
    } else {

    }



    }





    }

    else


    return 0;
}

static int FUNC(sps_extension)(CodedBitstreamContext *ctx, RWContext *rw,
                               H264RawSPSExtension *current)
{
    int err;

    HEADER("Sequence Parameter Set Extension");

    CHECK(FUNC(nal_unit_header)(ctx, rw, &current->nal_unit_header,
                                1 << H264_NAL_SPS_EXT));

    ue(seq_parameter_set_id, 0, 31);

    ue(aux_format_idc, 0, 3);

    if (current->aux_format_idc != 0) {
        int bits;

        ue(bit_depth_aux_minus8, 0, 4);
        flag(alpha_incr_flag);

        bits = current->bit_depth_aux_minus8 + 9;
        ub(bits, alpha_opaque_value);
        ub(bits, alpha_transparent_value);
    }

    flag(additional_extension_flag);

    CHECK(FUNC(rbsp_trailing_bits)(ctx, rw));

    return 0;
}

                     H264RawPPS *current)
{


                                1 << H264_NAL_PPS));


        av_log(ctx->log_ctx, AV_LOG_ERROR, "SPS id %d not available.\n",
               current->seq_parameter_set_id);
        return AVERROR_INVALIDDATA;
    }






                    current->top_left[iGroup], pic_size - 1, 1, iGroup);
            }
                   current->slice_group_map_type == 5) {

                     current->pic_size_in_map_units_minus1 + 1);
                   slice_group_id[i], 0, current->num_slice_groups_minus1, 1, i);
        }
    }





    {
        flag(transform_8x8_mode_flag);

        flag(pic_scaling_matrix_present_flag);
        if (current->pic_scaling_matrix_present_flag) {
            for (i = 0; i < 6 + (((sps->chroma_format_idc != 3) ? 2 : 6) *
                                 current->transform_8x8_mode_flag); i++) {
                flags(pic_scaling_list_present_flag[i], 1, i);
                if (current->pic_scaling_list_present_flag[i]) {
                    if (i < 6)
                        CHECK(FUNC(scaling_list)(ctx, rw,
                                                 &current->scaling_list_4x4[i],
                                                 16));
                    else
                        CHECK(FUNC(scaling_list)(ctx, rw,
                                                 &current->scaling_list_8x8[i - 6],
                                                 64));
                }
            }
        }

        se(second_chroma_qp_index_offset, -12, +12);
    } else {
    }


    return 0;
}

                                      H264RawSEIBufferingPeriod *current)
{



        av_log(ctx->log_ctx, AV_LOG_ERROR, "SPS id %d not available.\n",
               current->seq_parameter_set_id);
        return AVERROR_INVALIDDATA;
    }

               current->nal.initial_cpb_removal_delay[i],
               1, MAX_UINT_BITS(length), 1, i);
               current->nal.initial_cpb_removal_delay_offset[i],
               0, MAX_UINT_BITS(length), 1, i);
        }
    }

               current->vcl.initial_cpb_removal_delay[i],
               1, MAX_UINT_BITS(length), 1, i);
               current->vcl.initial_cpb_removal_delay_offset[i],
               0, MAX_UINT_BITS(length), 1, i);
        }
    }

    return 0;
}

static int FUNC(sei_pic_timestamp)(CodedBitstreamContext *ctx, RWContext *rw,
                                   H264RawSEIPicTimestamp *current,
                                   const H264RawSPS *sps)
{
    uint8_t time_offset_length;
    int err;

    u(2, ct_type, 0, 2);
    flag(nuit_field_based_flag);
    u(5, counting_type, 0, 6);
    flag(full_timestamp_flag);
    flag(discontinuity_flag);
    flag(cnt_dropped_flag);
    ub(8, n_frames);
    if (current->full_timestamp_flag) {
            u(6, seconds_value, 0, 59);
            u(6, minutes_value, 0, 59);
            u(5, hours_value,   0, 23);
    } else {
        flag(seconds_flag);
        if (current->seconds_flag) {
            u(6, seconds_value, 0, 59);
            flag(minutes_flag);
            if (current->minutes_flag) {
                u(6, minutes_value, 0, 59);
                flag(hours_flag);
                if (current->hours_flag)
                    u(5, hours_value, 0, 23);
            }
        }
    }

    if (sps->vui.nal_hrd_parameters_present_flag)
        time_offset_length = sps->vui.nal_hrd_parameters.time_offset_length;
    else if (sps->vui.vcl_hrd_parameters_present_flag)
        time_offset_length = sps->vui.vcl_hrd_parameters.time_offset_length;
    else
        time_offset_length = 24;

    if (time_offset_length > 0)
        ib(time_offset_length, time_offset);
    else
        infer(time_offset, 0);

    return 0;
}

                                H264RawSEIPicTiming *current)
{


        // If there is exactly one possible SPS but it is not yet active
        // then just assume that it should be the active one.
        int i, k = -1;
        for (i = 0; i < H264_MAX_SPS_COUNT; i++) {
            if (h264->sps[i]) {
                if (k >= 0) {
                    k = -1;
                    break;
                }
                k = i;
            }
        }
        if (k >= 0)
            sps = h264->sps[k];
    }
        av_log(ctx->log_ctx, AV_LOG_ERROR,
               "No active SPS for pic_timing.\n");
        return AVERROR_INVALIDDATA;
    }

        sps->vui.vcl_hrd_parameters_present_flag) {

        else if (sps->vui.vcl_hrd_parameters_present_flag)
            hrd = &sps->vui.vcl_hrd_parameters;
        else {
            av_log(ctx->log_ctx, AV_LOG_ERROR,
                   "No HRD parameters for pic_timing.\n");
            return AVERROR_INVALIDDATA;
        }

    }

            1, 1, 1, 2, 2, 3, 3, 2, 3
        };

            return AVERROR_INVALIDDATA;

                                              &current->timestamp[i], sps));
        }
    }

    return 0;
}

                                   H264RawSEIPanScanRect *current)
{




        }

    }

    return 0;
}

                                          H264RawSEIUserDataRegistered *current,
                                          uint32_t *payload_size)
{


        i = 1;
    else {
        u(8, itu_t_t35_country_code_extension_byte, 0x00, 0xff);
        i = 2;
    }

#ifdef READ
        av_log(ctx->log_ctx, AV_LOG_ERROR,
               "Invalid SEI user data registered payload.\n");
        return AVERROR_INVALIDDATA;
    }
#else
#endif


    return 0;
}

static int FUNC(sei_user_data_unregistered)(CodedBitstreamContext *ctx, RWContext *rw,
                                            H264RawSEIUserDataUnregistered *current,
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

                                    H264RawSEIRecoveryPoint *current)
{



}

                                         H264RawSEIDisplayOrientation *current)
{


    }

    return 0;
}

                                                     H264RawSEIMasteringDisplayColourVolume *current)
{


    }



}

static int FUNC(sei_alternative_transfer_characteristics)(CodedBitstreamContext *ctx,
                                                          RWContext *rw,
                                                          H264RawSEIAlternativeTransferCharacteristics *current)
{
    int err;

    HEADER("Alternative Transfer Characteristics");

    ub(8, preferred_transfer_characteristics);

    return 0;
}

                             H264RawSEIPayload *current)
{

#ifdef READ
#else
#endif

              (ctx, rw, &current->payload.buffering_period));
        break;
              (ctx, rw, &current->payload.pic_timing));
        break;
              (ctx, rw, &current->payload.pan_scan_rect));
        break;
    case H264_SEI_TYPE_FILLER_PAYLOAD:
        {
            for (i = 0; i  < current->payload_size; i++)
                fixed(8, ff_byte, 0xff);
        }
        break;
              (ctx, rw, &current->payload.user_data_registered, &current->payload_size));
        break;
    case H264_SEI_TYPE_USER_DATA_UNREGISTERED:
        CHECK(FUNC(sei_user_data_unregistered)
              (ctx, rw, &current->payload.user_data_unregistered, &current->payload_size));
        break;
              (ctx, rw, &current->payload.recovery_point));
        break;
              (ctx, rw, &current->payload.display_orientation));
        break;
              (ctx, rw, &current->payload.mastering_display_colour_volume));
        break;
    case H264_SEI_TYPE_ALTERNATIVE_TRANSFER:
        CHECK(FUNC(sei_alternative_transfer_characteristics)
              (ctx, rw, &current->payload.alternative_transfer_characteristics));
        break;
        {
#ifdef READ
#endif
        }
    }

    }

#ifdef READ
        av_log(ctx->log_ctx, AV_LOG_ERROR, "Incorrect SEI payload length: "
               "header %"PRIu32" bits, actually %d bits.\n",
               8 * current->payload_size,
               end_position - start_position);
        return AVERROR_INVALIDDATA;
    }
#else
#endif

}

                     H264RawSEI *current)
{


                                1 << H264_NAL_SEI));

#ifdef READ
        uint32_t payload_type = 0;
        uint32_t tmp;

        }

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

            }

            }

        }
    }
#endif


    return 0;
}

                     H264RawAUD *current)
{


                                1 << H264_NAL_AUD));



    return 0;
}

                                           H264RawSliceHeader *current)
{

        current->slice_type % 5 != 4) {
                    current->rplm_l0[i].modification_of_pic_nums_idc, 0, 3, 0);

                    break;

                        current->rplm_l0[i].abs_diff_pic_num_minus1,
                        0, (1 + current->field_pic_flag) *
                        (1 << (sps->log2_max_frame_num_minus4 + 4)), 0);
                        current->rplm_l0[i].long_term_pic_num,
                        0, sps->max_num_ref_frames - 1, 0);
            }
        }
    }

            for (i = 0; i < H264_MAX_RPLM_COUNT; i++) {
                xue(modification_of_pic_nums_idc,
                    current->rplm_l1[i].modification_of_pic_nums_idc, 0, 3, 0);

                mopn = current->rplm_l1[i].modification_of_pic_nums_idc;
                if (mopn == 3)
                    break;

                if (mopn == 0 || mopn == 1)
                    xue(abs_diff_pic_num_minus1,
                        current->rplm_l1[i].abs_diff_pic_num_minus1,
                        0, (1 + current->field_pic_flag) *
                        (1 << (sps->log2_max_frame_num_minus4 + 4)), 0);
                else if (mopn == 2)
                    xue(long_term_pic_num,
                        current->rplm_l1[i].long_term_pic_num,
                        0, sps->max_num_ref_frames - 1, 0);
            }
        }
    }

    return 0;
}

                                   H264RawSliceHeader *current)
{



        }
                }
            }
        }
    }

        for (i = 0; i <= current->num_ref_idx_l1_active_minus1; i++) {
            flags(luma_weight_l1_flag[i], 1, i);
            if (current->luma_weight_l1_flag[i]) {
                ses(luma_weight_l1[i], -128, +127, 1, i);
                ses(luma_offset_l1[i], -128, +127, 1, i);
            }
            if (chroma) {
                flags(chroma_weight_l1_flag[i], 1, i);
                if (current->chroma_weight_l1_flag[i]) {
                    for (j = 0; j < 2; j++) {
                        ses(chroma_weight_l1[i][j], -128, +127, 2, i, j);
                        ses(chroma_offset_l1[i][j], -128, +127, 2, i, j);
                    }
                }
            }
        }
    }

    return 0;
}

                                     H264RawSliceHeader *current, int idr_pic_flag)
{

    } else {
                    current->mmco[i].memory_management_control_operation,
                    0, 6, 0);

                    break;

                        current->mmco[i].difference_of_pic_nums_minus1,
                        0, INT32_MAX, 0);
                    xue(long_term_pic_num,
                        current->mmco[i].long_term_pic_num,
                        0, sps->max_num_ref_frames - 1, 0);
                        current->mmco[i].long_term_frame_idx,
                        0, sps->max_num_ref_frames - 1, 0);
                        current->mmco[i].max_long_term_frame_idx_plus1,
                        0, sps->max_num_ref_frames, 0);
            }
                av_log(ctx->log_ctx, AV_LOG_ERROR, "Too many "
                       "memory management control operations.\n");
                return AVERROR_INVALIDDATA;
            }
        }
    }

    return 0;
}

                              H264RawSliceHeader *current)
{


                                1 << H264_NAL_SLICE     |
                                1 << H264_NAL_IDR_SLICE |
                                1 << H264_NAL_AUXILIARY_SLICE));

        if (!h264->last_slice_nal_unit_type) {
            av_log(ctx->log_ctx, AV_LOG_ERROR, "Auxiliary slice "
                   "is not decodable without the main picture "
                   "in the same access unit.\n");
            return AVERROR_INVALIDDATA;
        }
        idr_pic_flag = h264->last_slice_nal_unit_type == H264_NAL_IDR_SLICE;
    } else {
    }



        av_log(ctx->log_ctx, AV_LOG_ERROR, "Invalid slice type %d "
               "for IDR picture.\n", current->slice_type);
        return AVERROR_INVALIDDATA;
    }


        av_log(ctx->log_ctx, AV_LOG_ERROR, "PPS id %d not available.\n",
               current->pic_parameter_set_id);
        return AVERROR_INVALIDDATA;
    }

        av_log(ctx->log_ctx, AV_LOG_ERROR, "SPS id %d not available.\n",
               pps->seq_parameter_set_id);
        return AVERROR_INVALIDDATA;
    }

        u(2, colour_plane_id, 0, 2);


        else
    } else {
    }



                !current->field_pic_flag)
                se(delta_pic_order_cnt[1], INT32_MIN + 1, INT32_MAX);
            else
        } else {
        }
    }

        ue(redundant_pic_cnt, 0, 127);
    else

            current->nal_unit_header.nal_unit_type;


        } else {
                  pps->num_ref_idx_l0_default_active_minus1);
                  pps->num_ref_idx_l1_default_active_minus1);
        }
    }

        current->nal_unit_header.nal_unit_type == 21) {
        av_log(ctx->log_ctx, AV_LOG_ERROR, "MVC / 3DAVC not supported.\n");
        return AVERROR_PATCHWELCOME;
    } else {
    }

    }

    }

    }

                       + 51 + 6 * sps->bit_depth_luma_minus8);
    }

        } else {
        }
    } else {
    }

        pps->slice_group_map_type <= 5) {


    }

    }

    return 0;
}

static int FUNC(filler)(CodedBitstreamContext *ctx, RWContext *rw,
                        H264RawFiller *current)
{
    int err;

    HEADER("Filler Data");

    CHECK(FUNC(nal_unit_header)(ctx, rw, &current->nal_unit_header,
                                1 << H264_NAL_FILLER_DATA));

#ifdef READ
    while (show_bits(rw, 8) == 0xff) {
        fixed(8, ff_byte, 0xff);
        ++current->filler_size;
    }
#else
    {
        uint32_t i;
        for (i = 0; i < current->filler_size; i++)
            fixed(8, ff_byte, 0xff);
    }
#endif

    CHECK(FUNC(rbsp_trailing_bits)(ctx, rw));

    return 0;
}

                                 H264RawNALUnitHeader *current)
{

                                 1 << H264_NAL_END_SEQUENCE);
}

                               H264RawNALUnitHeader *current)
{

                                 1 << H264_NAL_END_STREAM);
}
