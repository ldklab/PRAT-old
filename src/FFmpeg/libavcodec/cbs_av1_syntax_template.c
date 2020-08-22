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

                            AV1RawOBUHeader *current)
{





    }

    return 0;
}

{



    }

    return 0;
}

{


    return 0;
}

                              AV1RawColorConfig *current, int seq_profile)
{


        current->high_bitdepth) {
        flag(twelve_bit);
        priv->bit_depth = current->twelve_bit ? 12 : 10;
    } else {
    }

        infer(mono_chrome, 0);
    else

        fb(8, color_primaries);
        fb(8, transfer_characteristics);
        fb(8, matrix_coefficients);
    } else {
    }

        flag(color_range);

        infer(subsampling_x, 1);
        infer(subsampling_y, 1);
        infer(chroma_sample_position, AV1_CSP_UNKNOWN);
        infer(separate_uv_delta_q, 0);

               current->transfer_characteristics == AVCOL_TRC_IEC61966_2_1 &&
               current->matrix_coefficients      == AVCOL_SPC_RGB) {
        infer(color_range,   1);
        infer(subsampling_x, 0);
        infer(subsampling_y, 0);
        flag(separate_uv_delta_q);

    } else {

        } else if (seq_profile == FF_PROFILE_AV1_HIGH) {
            infer(subsampling_x, 0);
            infer(subsampling_y, 0);
        } else {
            if (priv->bit_depth == 12) {
                fb(1, subsampling_x);
                if (current->subsampling_x)
                    fb(1, subsampling_y);
                else
                    infer(subsampling_y, 0);
            } else {
                infer(subsampling_x, 1);
                infer(subsampling_y, 0);
            }
        }
                                          AV1_CSP_COLOCATED);
        }

    }

    return 0;
}

                             AV1RawTimingInfo *current)
{


        uvlc(num_ticks_per_picture_minus_1, 0, MAX_UINT_BITS(32) - 1);

    return 0;
}

                                    AV1RawDecoderModelInfo *current)
{


}

                                     AV1RawSequenceHeader *current)
{


                       FF_PROFILE_AV1_PROFESSIONAL);

        infer(timing_info_present_flag,           0);
        infer(decoder_model_info_present_flag,    0);
        infer(initial_display_delay_present_flag, 0);
        infer(operating_points_cnt_minus_1,       0);
        infer(operating_point_idc[0],             0);

        fb(5, seq_level_idx[0]);

        infer(seq_tier[0], 0);
        infer(decoder_model_present_for_this_op[0],         0);
        infer(initial_display_delay_present_for_this_op[0], 0);

    } else {

                          (ctx, rw, &current->decoder_model_info));
            }
        } else {
        }



                flags(seq_tier[i], 1, i);
            else

                }
            } else {
            }

            }
        }
    }



        infer(frame_id_numbers_present_flag, 0);
    else
        fb(4, delta_frame_id_length_minus_2);
        fb(3, additional_frame_id_length_minus_1);
    }


        infer(enable_interintra_compound, 0);
        infer(enable_masked_compound,     0);
        infer(enable_warped_motion,       0);
        infer(enable_dual_filter,         0);
        infer(enable_order_hint,          0);
        infer(enable_jnt_comp,            0);
        infer(enable_ref_frame_mvs,       0);

        infer(seq_force_screen_content_tools,
              AV1_SELECT_SCREEN_CONTENT_TOOLS);
        infer(seq_force_integer_mv,
              AV1_SELECT_INTEGER_MV);
    } else {

        } else {
            infer(enable_jnt_comp,      0);
            infer(enable_ref_frame_mvs, 0);
        }

                  AV1_SELECT_SCREEN_CONTENT_TOOLS);
        else
            fb(1, seq_force_screen_content_tools);
                      AV1_SELECT_INTEGER_MV);
            else
                fb(1, seq_force_integer_mv);
        } else {
            infer(seq_force_integer_mv, AV1_SELECT_INTEGER_MV);
        }

    }


                             current->seq_profile));


}

{



}

static int FUNC(set_frame_refs)(CodedBitstreamContext *ctx, RWContext *rw,
                                AV1RawFrameHeader *current)
{
    CodedBitstreamAV1Context *priv = ctx->priv_data;
    const AV1RawSequenceHeader *seq = priv->sequence_header;
    static const uint8_t ref_frame_list[AV1_NUM_REF_FRAMES - 2] = {
        AV1_REF_FRAME_LAST2, AV1_REF_FRAME_LAST3, AV1_REF_FRAME_BWDREF,
        AV1_REF_FRAME_ALTREF2, AV1_REF_FRAME_ALTREF
    };
    int8_t ref_frame_idx[AV1_REFS_PER_FRAME], used_frame[AV1_NUM_REF_FRAMES];
    int8_t shifted_order_hints[AV1_NUM_REF_FRAMES];
    int cur_frame_hint, latest_order_hint, earliest_order_hint, ref;
    int i, j;

    for (i = 0; i < AV1_REFS_PER_FRAME; i++)
        ref_frame_idx[i] = -1;
    ref_frame_idx[AV1_REF_FRAME_LAST - AV1_REF_FRAME_LAST] = current->last_frame_idx;
    ref_frame_idx[AV1_REF_FRAME_GOLDEN - AV1_REF_FRAME_LAST] = current->golden_frame_idx;

    for (i = 0; i < AV1_NUM_REF_FRAMES; i++)
        used_frame[i] = 0;
    used_frame[current->last_frame_idx] = 1;
    used_frame[current->golden_frame_idx] = 1;

    cur_frame_hint = 1 << (seq->order_hint_bits_minus_1);
    for (i = 0; i < AV1_NUM_REF_FRAMES; i++)
        shifted_order_hints[i] = cur_frame_hint +
                                 cbs_av1_get_relative_dist(seq, priv->ref[i].order_hint,
                                                           current->order_hint);

    latest_order_hint = shifted_order_hints[current->last_frame_idx];
    earliest_order_hint = shifted_order_hints[current->golden_frame_idx];

    ref = -1;
    for (i = 0; i < AV1_NUM_REF_FRAMES; i++) {
        int hint = shifted_order_hints[i];
        if (!used_frame[i] && hint >= cur_frame_hint &&
            (ref < 0 || hint >= latest_order_hint)) {
            ref = i;
            latest_order_hint = hint;
        }
    }
    if (ref >= 0) {
        ref_frame_idx[AV1_REF_FRAME_ALTREF - AV1_REF_FRAME_LAST] = ref;
        used_frame[ref] = 1;
    }

    ref = -1;
    for (i = 0; i < AV1_NUM_REF_FRAMES; i++) {
        int hint = shifted_order_hints[i];
        if (!used_frame[i] && hint >= cur_frame_hint &&
            (ref < 0 || hint < earliest_order_hint)) {
            ref = i;
            earliest_order_hint = hint;
        }
    }
    if (ref >= 0) {
        ref_frame_idx[AV1_REF_FRAME_BWDREF - AV1_REF_FRAME_LAST] = ref;
        used_frame[ref] = 1;
    }

    ref = -1;
    for (i = 0; i < AV1_NUM_REF_FRAMES; i++) {
        int hint = shifted_order_hints[i];
        if (!used_frame[i] && hint >= cur_frame_hint &&
            (ref < 0 || hint < earliest_order_hint)) {
            ref = i;
            earliest_order_hint = hint;
        }
    }
    if (ref >= 0) {
        ref_frame_idx[AV1_REF_FRAME_ALTREF2 - AV1_REF_FRAME_LAST] = ref;
        used_frame[ref] = 1;
    }

    for (i = 0; i < AV1_REFS_PER_FRAME - 2; i++) {
        int ref_frame = ref_frame_list[i];
        if (ref_frame_idx[ref_frame - AV1_REF_FRAME_LAST] < 0 ) {
            ref = -1;
            for (j = 0; j < AV1_NUM_REF_FRAMES; j++) {
                int hint = shifted_order_hints[j];
                if (!used_frame[j] && hint < cur_frame_hint &&
                    (ref < 0 || hint >= latest_order_hint)) {
                    ref = j;
                    latest_order_hint = hint;
                }
            }
            if (ref >= 0) {
                ref_frame_idx[ref_frame - AV1_REF_FRAME_LAST] = ref;
                used_frame[ref] = 1;
            }
        }
    }

    ref = -1;
    for (i = 0; i < AV1_NUM_REF_FRAMES; i++) {
        int hint = shifted_order_hints[i];
        if (ref < 0 || hint < earliest_order_hint) {
            ref = i;
            earliest_order_hint = hint;
        }
    }
    for (i = 0; i < AV1_REFS_PER_FRAME; i++) {
        if (ref_frame_idx[i] < 0)
            ref_frame_idx[i] = ref;
        infer(ref_frame_idx[i], ref_frame_idx[i]);
    }

    return 0;
}

static int FUNC(superres_params)(CodedBitstreamContext *ctx, RWContext *rw,
                                 AV1RawFrameHeader *current)
{
    CodedBitstreamAV1Context  *priv = ctx->priv_data;
    const AV1RawSequenceHeader *seq = priv->sequence_header;
    int denom, err;

    if (seq->enable_superres)
        flag(use_superres);
    else
        infer(use_superres, 0);

    if (current->use_superres) {
        fb(3, coded_denom);
        denom = current->coded_denom + AV1_SUPERRES_DENOM_MIN;
    } else {
        denom = AV1_SUPERRES_NUM;
    }

    priv->upscaled_width = priv->frame_width;
    priv->frame_width = (priv->upscaled_width * AV1_SUPERRES_NUM +
                         denom / 2) / denom;

    return 0;
}

                            AV1RawFrameHeader *current)
{


    } else {
    }


    return 0;
}

                             AV1RawFrameHeader *current)
{


        fb(16, render_width_minus_1);
        fb(16, render_height_minus_1);

        priv->render_width  = current->render_width_minus_1  + 1;
        priv->render_height = current->render_height_minus_1 + 1;
    } else {
    }

    return 0;
}

                                      AV1RawFrameHeader *current)
{


                av_log(ctx->log_ctx, AV_LOG_ERROR,
                       "Missing reference frame needed for frame size "
                       "(ref = %d, ref_frame_idx = %d).\n",
                       i, current->ref_frame_idx[i]);
                return AVERROR_INVALIDDATA;
            }

        }
    }

        CHECK(FUNC(frame_size)(ctx, rw, current));
        CHECK(FUNC(render_size)(ctx, rw, current));
    } else {
    }

    return 0;
}

static int FUNC(interpolation_filter)(CodedBitstreamContext *ctx, RWContext *rw,
                                      AV1RawFrameHeader *current)
{
    int err;

    flag(is_filter_switchable);
    if (current->is_filter_switchable)
        infer(interpolation_filter,
              AV1_INTERPOLATION_FILTER_SWITCHABLE);
    else
        fb(2, interpolation_filter);

    return 0;
}

                           AV1RawFrameHeader *current)
{





                           cbs_av1_tile_log2(max_tile_area_sb, sb_rows * sb_cols));




            current->tile_cols_log2;



            current->tile_rows_log2;

    } else {
        int widest_tile_sb, start_sb, size_sb, max_width, max_height;

        widest_tile_sb = 0;

        start_sb = 0;
        }

            max_tile_area_sb = (sb_rows * sb_cols) >> (min_log2_tiles + 1);
        else
            max_tile_area_sb = sb_rows * sb_cols;

        }
    }

           context_update_tile_id);
    } else {
    }


}

                                     AV1RawFrameHeader *current)
{



            flag(diff_uv_delta);
        else


            delta_q(delta_q_v_dc);
            delta_q(delta_q_v_ac);
        } else {
        }
    } else {
        infer(delta_q_u_dc, 0);
        infer(delta_q_u_ac, 0);
        infer(delta_q_v_dc, 0);
        infer(delta_q_v_ac, 0);
    }

        fb(4, qm_y);
        fb(4, qm_u);
        if (seq->color_config.separate_uv_delta_q)
            fb(4, qm_v);
        else
            infer(qm_v, current->qm_u);
    }

    return 0;
}

                                     AV1RawFrameHeader *current)
{


        if (current->primary_ref_frame == AV1_PRIMARY_REF_NONE) {
            infer(segmentation_update_map,      1);
            infer(segmentation_temporal_update, 0);
            infer(segmentation_update_data,     1);
        } else {
            flag(segmentation_update_map);
            if (current->segmentation_update_map)
                flag(segmentation_temporal_update);
            else
                infer(segmentation_temporal_update, 0);
            flag(segmentation_update_data);
        }

        if (current->segmentation_update_data) {
            for (i = 0; i < AV1_MAX_SEGMENTS; i++) {
                for (j = 0; j < AV1_SEG_LVL_MAX; j++) {
                    flags(feature_enabled[i][j], 2, i, j);

                    if (current->feature_enabled[i][j] && bits[j] > 0) {
                        if (sign[j])
                            sus(1 + bits[j], feature_value[i][j], 2, i, j);
                        else
                            fbs(bits[j], feature_value[i][j], 2, i, j);
                    } else {
                        infer(feature_value[i][j], 0);
                    }
                }
            }
        }
    } else {
            }
        }
    }

    return 0;
}

                                AV1RawFrameHeader *current)
{

    else
        infer(delta_q_present, 0);


    return 0;
}

                                 AV1RawFrameHeader *current)
{

        else
            infer(delta_lf_present, 0);
            fb(2, delta_lf_res);
            flag(delta_lf_multi);
        } else {
        }
    } else {
    }

    return 0;
}

                                    AV1RawFrameHeader *current)
{

        infer(loop_filter_level[0], 0);
        infer(loop_filter_level[1], 0);
        infer(loop_filter_ref_deltas[AV1_REF_FRAME_INTRA],    1);
        infer(loop_filter_ref_deltas[AV1_REF_FRAME_LAST],     0);
        infer(loop_filter_ref_deltas[AV1_REF_FRAME_LAST2],    0);
        infer(loop_filter_ref_deltas[AV1_REF_FRAME_LAST3],    0);
        infer(loop_filter_ref_deltas[AV1_REF_FRAME_BWDREF],   0);
        infer(loop_filter_ref_deltas[AV1_REF_FRAME_GOLDEN],  -1);
        infer(loop_filter_ref_deltas[AV1_REF_FRAME_ALTREF],  -1);
        infer(loop_filter_ref_deltas[AV1_REF_FRAME_ALTREF2], -1);
        for (i = 0; i < 2; i++)
            infer(loop_filter_mode_deltas[i], 0);
        return 0;
    }


        }
    }


            }
            }
        }
    }

    return 0;
}

                             AV1RawFrameHeader *current)
{


    }



        }
    }

    return 0;
}

                           AV1RawFrameHeader *current)
{

        return 0;
    }

    uses_lr = uses_chroma_lr = 0;

        }
    }

        else

        } else {
        }
    }

    return 0;
}

static int FUNC(read_tx_mode)(CodedBitstreamContext *ctx, RWContext *rw,
                              AV1RawFrameHeader *current)
{
    CodedBitstreamAV1Context *priv = ctx->priv_data;
    int err;

    if (priv->coded_lossless)
        infer(tx_mode, 0);
    else
        increment(tx_mode, 1, 2);

    return 0;
}

static int FUNC(frame_reference_mode)(CodedBitstreamContext *ctx, RWContext *rw,
                                      AV1RawFrameHeader *current)
{
    int err;

    if (current->frame_type == AV1_FRAME_INTRA_ONLY ||
        current->frame_type == AV1_FRAME_KEY)
        infer(reference_select, 0);
    else
        flag(reference_select);

    return 0;
}

                                  AV1RawFrameHeader *current)
{

        skip_mode_allowed = 0;
    } else {
        int forward_idx,  backward_idx;
        int forward_hint, backward_hint;
        int ref_hint, dist, i;

        forward_idx  = -1;
        backward_idx = -1;
                                              forward_hint) > 0) {
                    forward_idx  = i;
                    forward_hint = ref_hint;
                }
                                              backward_hint) < 0) {
                    backward_idx  = i;
                    backward_hint = ref_hint;
                }
            }
        }

            skip_mode_allowed = 0;
            skip_mode_allowed = 1;
            // Frames for skip mode are forward_idx and backward_idx.
        } else {
            int second_forward_idx;
            int second_forward_hint;

            second_forward_idx = -1;
                                              forward_hint) < 0) {
                                                  second_forward_hint) > 0) {
                        second_forward_idx  = i;
                        second_forward_hint = ref_hint;
                    }
                }
            }

                skip_mode_allowed = 0;
            } else {
                skip_mode_allowed = 1;
                // Frames for skip mode are forward_idx and second_forward_idx.
            }
        }
    }

    if (skip_mode_allowed)
    else

    return 0;
}

                                     AV1RawFrameHeader *current,
                                     int type, int ref, int idx)
{

            abs_bits  = AV1_GM_ABS_TRANS_ONLY_BITS  - !current->allow_high_precision_mv;
            prec_bits = AV1_GM_TRANS_ONLY_PREC_BITS - !current->allow_high_precision_mv;
        } else {
            abs_bits  = AV1_GM_ABS_TRANS_BITS;
        }
    } else {
        abs_bits  = AV1_GM_ABS_ALPHA_BITS;
        prec_bits = AV1_GM_ALPHA_PREC_BITS;
    }


    // Actual gm_params value is not reconstructed here.

}

                                      AV1RawFrameHeader *current)
{

        current->frame_type == AV1_FRAME_INTRA_ONLY)
        return 0;

                type = AV1_WARP_MODEL_ROTZOOM;
            } else {
                flags(is_translation[ref], 1, ref);
                type = current->is_translation[ref] ? AV1_WARP_MODEL_TRANSLATION
                                                    : AV1_WARP_MODEL_AFFINE;
            }
        } else {
            type = AV1_WARP_MODEL_IDENTITY;
        }

        if (type >= AV1_WARP_MODEL_ROTZOOM) {
                CHECK(FUNC(global_motion_param)(ctx, rw, current, type, ref, 4));
                CHECK(FUNC(global_motion_param)(ctx, rw, current, type, ref, 5));
            } else {
                // gm_params[ref][4] = -gm_params[ref][3]
                // gm_params[ref][5] =  gm_params[ref][2]
        }
        }
    }

    return 0;
}

                                   AV1RawFrameHeader *current)
{

        return 0;


        return 0;


    else

        fb(3, film_grain_params_ref_idx);
        return 0;
    }

            i ? current->point_y_value[i - 1] + 1 : 0,
            MAX_UINT_BITS(8) - (current->num_y_points - i - 1),
            1, i);
    }

        infer(chroma_scaling_from_luma, 0);
    else

        infer(num_cb_points, 0);
        infer(num_cr_points, 0);
    } else {
                i ? current->point_cb_value[i - 1] + 1 : 0,
                MAX_UINT_BITS(8) - (current->num_cb_points - i - 1),
                1, i);
        }
                i ? current->point_cr_value[i - 1] + 1 : 0,
                MAX_UINT_BITS(8) - (current->num_cr_points - i - 1),
                1, i);
        }
    }

    } else {
        num_pos_chroma = num_pos_luma;
    }
    }
    }
    }
    }


}

                                     AV1RawFrameHeader *current)
{

        av_log(ctx->log_ctx, AV_LOG_ERROR, "No sequence header available: "
               "unable to decode frame header.\n");
        return AVERROR_INVALIDDATA;
    }


        infer(show_existing_frame, 0);
        infer(frame_type,     AV1_FRAME_KEY);
        infer(show_frame,     1);
        infer(showable_frame, 0);
        frame_is_intra = 1;

    } else {



                   frame_presentation_time);
            }

                fb(id_len, display_frame_id);

                infer(refresh_frame_flags, all_frames);
            else

        }

                          current->frame_type == AV1_FRAME_KEY);

               frame_presentation_time);
        }
        else

        else
    }

        }
    }


        AV1_SELECT_SCREEN_CONTENT_TOOLS) {
    } else {
        infer(allow_screen_content_tools,
              seq->seq_force_screen_content_tools);
    }
        else
            infer(force_integer_mv, seq->seq_force_integer_mv);
    } else {
    }

        fb(id_len, current_frame_id);

        diff_len = seq->delta_frame_id_length_minus_2 + 2;
        for (i = 0; i < AV1_NUM_REF_FRAMES; i++) {
            if (current->current_frame_id > (1 << diff_len)) {
                if (priv->ref[i].frame_id > current->current_frame_id ||
                    priv->ref[i].frame_id < (current->current_frame_id -
                                             (1 << diff_len)))
                    priv->ref[i].valid = 0;
            } else {
                if (priv->ref[i].frame_id > current->current_frame_id &&
                    priv->ref[i].frame_id < ((1 << id_len) +
                                             current->current_frame_id -
                                             (1 << diff_len)))
                    priv->ref[i].valid = 0;
            }
        }
    } else {
    }

        infer(frame_size_override_flag, 0);
    else

    else
        infer(order_hint, 0);

    else

                        in_temporal_layer || in_spatial_layer) {
                            buffer_removal_time[i], 1, i);
                    }
                }
            }
        }
    }

    else

                    priv->ref[i].valid = 0;
            }
        }
    }

        current->frame_type == AV1_FRAME_INTRA_ONLY) {

        else

    } else {
            infer(frame_refs_short_signaling, 0);
        } else {
            }
        }

                    delta_frame_id_minus1[i], 1, i);
            }
        }

        } else {
        }

            infer(allow_high_precision_mv, 0);
        else



        else

    }

        // Derive reference frame sign biases.

        infer(disable_frame_end_update_cdf, 1);
    else

        // Init non-coeff CDFs.
        // Setup past independence.
    } else {
        // Load CDF tables from previous frame.
        // Load params from previous frame.

        // Perform motion field estimation process.






    // Init coeff CDFs / load previous segments.

            qindex = (current->base_q_idx +
                      current->feature_value[i][AV1_SEG_LVL_ALT_Q]);
        } else {
        }

            current->delta_q_u_ac || current->delta_q_u_dc ||
            current->delta_q_v_ac || current->delta_q_v_dc) {
        }
    }
        priv->frame_width == priv->upscaled_width;







    else




                .valid          = 1,
            };
        }
    }

           "upscaled %d  render %dx%d  subsample %dx%d  "
           priv->frame_width, priv->frame_height, priv->upscaled_width,
           priv->render_width, priv->render_height,
           priv->tile_rows, priv->tile_cols);

}

                                  AV1RawFrameHeader *current, int redundant,
                                  AVBufferRef *rw_buffer_ref)
{

        if (!redundant) {
            av_log(ctx->log_ctx, AV_LOG_ERROR, "Invalid repeated "
                   "frame header OBU.\n");
            return AVERROR_INVALIDDATA;
        } else {
            GetBitContext fh;
            size_t i, b;
            uint32_t val;

            HEADER("Redundant Frame Header");

            av_assert0(priv->frame_header_ref && priv->frame_header);

            init_get_bits(&fh, priv->frame_header,
                          priv->frame_header_size);
            for (i = 0; i < priv->frame_header_size; i += 8) {
                b = FFMIN(priv->frame_header_size - i, 8);
                val = get_bits(&fh, b);
                xf(b, frame_header_copy[i],
                   val, val, val, 1, i / 8);
            }
        }
    } else {
            HEADER("Redundant Frame Header (used as Frame Header)");
        else

#ifdef READ
#else
#endif


        } else {


#ifdef READ
#else
            // Need to flush the bitwriter so that we can copy its output,
            // but use a copy so we don't affect the caller's structure.
            {
            }

#endif


                    return AVERROR(ENOMEM);
            } else {
                    return AVERROR(ENOMEM);
            }
        }
    }

    return 0;
}

                                AV1RawTileGroup *current)
{


    else

    } else {
    }


    // Reset header for next frame.

    // Tile data follows.

    return 0;
}

                           AV1RawFrame *current,
                           AVBufferRef *rw_buffer_ref)
{

                                 0, rw_buffer_ref));



    return 0;
}

static int FUNC(tile_list_obu)(CodedBitstreamContext *ctx, RWContext *rw,
                               AV1RawTileList *current)
{
    int err;

    fb(8, output_frame_width_in_tiles_minus_1);
    fb(8, output_frame_height_in_tiles_minus_1);

    fb(16, tile_count_minus_1);

    // Tile data follows.

    return 0;
}

static int FUNC(metadata_hdr_cll)(CodedBitstreamContext *ctx, RWContext *rw,
                                  AV1RawMetadataHDRCLL *current)
{
    int err;

    fb(16, max_cll);
    fb(16, max_fall);

    return 0;
}

                                   AV1RawMetadataHDRMDCV *current)
{

    }


    // luminance_min must be lower than luminance_max. Convert luminance_max from
    // 24.8 fixed point to 18.14 fixed point in order to compare them.
                                   MAX_UINT_BITS(32)));

}

static int FUNC(scalability_structure)(CodedBitstreamContext *ctx, RWContext *rw,
                                       AV1RawMetadataScalability *current)
{
    CodedBitstreamAV1Context *priv = ctx->priv_data;
    const AV1RawSequenceHeader *seq;
    int err, i, j;

    if (!priv->sequence_header) {
        av_log(ctx->log_ctx, AV_LOG_ERROR, "No sequence header available: "
               "unable to parse scalability metadata.\n");
        return AVERROR_INVALIDDATA;
    }
    seq = priv->sequence_header;

    fb(2, spatial_layers_cnt_minus_1);
    flag(spatial_layer_dimensions_present_flag);
    flag(spatial_layer_description_present_flag);
    flag(temporal_group_description_present_flag);
    fc(3, scalability_structure_reserved_3bits, 0, 0);
    if (current->spatial_layer_dimensions_present_flag) {
        for (i = 0; i <= current->spatial_layers_cnt_minus_1; i++) {
            fcs(16, spatial_layer_max_width[i],
                0, seq->max_frame_width_minus_1 + 1, 1, i);
            fcs(16, spatial_layer_max_height[i],
                0, seq->max_frame_height_minus_1 + 1, 1, i);
        }
    }
    if (current->spatial_layer_description_present_flag) {
        for (i = 0; i <= current->spatial_layers_cnt_minus_1; i++)
            fbs(8, spatial_layer_ref_id[i], 1, i);
    }
    if (current->temporal_group_description_present_flag) {
        fb(8, temporal_group_size);
        for (i = 0; i < current->temporal_group_size; i++) {
            fbs(3, temporal_group_temporal_id[i], 1, i);
            flags(temporal_group_temporal_switching_up_point_flag[i], 1, i);
            flags(temporal_group_spatial_switching_up_point_flag[i], 1, i);
            fbs(3, temporal_group_ref_cnt[i], 1, i);
            for (j = 0; j < current->temporal_group_ref_cnt[i]; j++) {
                fbs(8, temporal_group_ref_pic_diff[i][j], 2, i, j);
            }
        }
    }

    return 0;
}

static int FUNC(metadata_scalability)(CodedBitstreamContext *ctx, RWContext *rw,
                                      AV1RawMetadataScalability *current)
{
    int err;

    fb(8, scalability_mode_idc);

    if (current->scalability_mode_idc == AV1_SCALABILITY_SS)
        CHECK(FUNC(scalability_structure)(ctx, rw, current));

    return 0;
}

static int FUNC(metadata_itut_t35)(CodedBitstreamContext *ctx, RWContext *rw,
                                   AV1RawMetadataITUTT35 *current)
{
    int err;
    size_t i;

    fb(8, itu_t_t35_country_code);
    if (current->itu_t_t35_country_code == 0xff)
        fb(8, itu_t_t35_country_code_extension_byte);

#ifdef READ
    // The payload runs up to the start of the trailing bits, but there might
    // be arbitrarily many trailing zeroes so we need to read through twice.
    current->payload_size = cbs_av1_get_payload_bytes_left(rw);

    current->payload_ref = av_buffer_alloc(current->payload_size);
    if (!current->payload_ref)
        return AVERROR(ENOMEM);
    current->payload = current->payload_ref->data;
#endif

    for (i = 0; i < current->payload_size; i++)
        xf(8, itu_t_t35_payload_bytes[i], current->payload[i],
           0x00, 0xff, 1, i);

    return 0;
}

static int FUNC(metadata_timecode)(CodedBitstreamContext *ctx, RWContext *rw,
                                   AV1RawMetadataTimecode *current)
{
    int err;

    fb(5, counting_type);
    flag(full_timestamp_flag);
    flag(discontinuity_flag);
    flag(cnt_dropped_flag);
    fb(9, n_frames);

    if (current->full_timestamp_flag) {
        fc(6, seconds_value, 0, 59);
        fc(6, minutes_value, 0, 59);
        fc(5, hours_value,   0, 23);
    } else {
        flag(seconds_flag);
        if (current->seconds_flag) {
            fc(6, seconds_value, 0, 59);
            flag(minutes_flag);
            if (current->minutes_flag) {
                fc(6, minutes_value, 0, 59);
                flag(hours_flag);
                if (current->hours_flag)
                    fc(5, hours_value, 0, 23);
            }
        }
    }

    fb(5, time_offset_length);
    if (current->time_offset_length > 0)
        fb(current->time_offset_length, time_offset_value);
    else
        infer(time_offset_length, 0);

    return 0;
}

                              AV1RawMetadata *current)
{


        break;
        break;
    case AV1_METADATA_TYPE_SCALABILITY:
        CHECK(FUNC(metadata_scalability)(ctx, rw, &current->metadata.scalability));
        break;
    case AV1_METADATA_TYPE_ITUT_T35:
        CHECK(FUNC(metadata_itut_t35)(ctx, rw, &current->metadata.itut_t35));
        break;
    case AV1_METADATA_TYPE_TIMECODE:
        CHECK(FUNC(metadata_timecode)(ctx, rw, &current->metadata.timecode));
        break;
    default:
        // Unknown metadata type.
        return AVERROR_PATCHWELCOME;
    }

    return 0;
}

static int FUNC(padding_obu)(CodedBitstreamContext *ctx, RWContext *rw,
                             AV1RawPadding *current)
{
    int i, err;

    HEADER("Padding");

#ifdef READ
    // The payload runs up to the start of the trailing bits, but there might
    // be arbitrarily many trailing zeroes so we need to read through twice.
    current->payload_size = cbs_av1_get_payload_bytes_left(rw);

    current->payload_ref = av_buffer_alloc(current->payload_size);
    if (!current->payload_ref)
        return AVERROR(ENOMEM);
    current->payload = current->payload_ref->data;
#endif

    for (i = 0; i < current->payload_size; i++)
        xf(8, obu_padding_byte[i], current->payload[i], 0x00, 0xff, 1, i);

    return 0;
}
