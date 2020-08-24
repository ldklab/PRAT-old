/*
 * HEVC Supplementary Enhancement Information messages
 *
 * Copyright (C) 2012 - 2013 Guillaume Martres
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

#include "atsc_a53.h"
#include "golomb.h"
#include "hevc_ps.h"
#include "hevc_sei.h"

{
    //uint16_t picture_crc;
    //uint32_t picture_checksum;

            // picture_crc = get_bits(gb, 16);
            // picture_checksum = get_bits_long(gb, 32);
        }
    }
}

static int decode_nal_sei_mastering_display_info(HEVCSEIMasteringDisplay *s, GetBitContext *gb)
{
    int i;
    // Mastering primaries
    for (i = 0; i < 3; i++) {
        s->display_primaries[i][0] = get_bits(gb, 16);
        s->display_primaries[i][1] = get_bits(gb, 16);
    }
    // White point (x, y)
    s->white_point[0] = get_bits(gb, 16);
    s->white_point[1] = get_bits(gb, 16);

    // Max and min luminance of mastering display
    s->max_luminance = get_bits_long(gb, 32);
    s->min_luminance = get_bits_long(gb, 32);

    // As this SEI message comes before the first frame that references it,
    // initialize the flag to 2 and decrement on IRAP access unit so it
    // persists for the coded video sequence (e.g., between two IRAPs)
    s->present = 2;
    return 0;
}

static int decode_nal_sei_content_light_info(HEVCSEIContentLight *s, GetBitContext *gb)
{
    // Max and average light levels
    s->max_content_light_level     = get_bits(gb, 16);
    s->max_pic_average_light_level = get_bits(gb, 16);
    // As this SEI message comes before the first frame that references it,
    // initialize the flag to 2 and decrement on IRAP access unit so it
    // persists for the coded video sequence (e.g., between two IRAPs)
    s->present = 2;
    return  0;
}

static int decode_nal_sei_frame_packing_arrangement(HEVCSEIFramePacking *s, GetBitContext *gb)
{
    get_ue_golomb_long(gb);             // frame_packing_arrangement_id
    s->present = !get_bits1(gb);

    if (s->present) {
        s->arrangement_type               = get_bits(gb, 7);
        s->quincunx_subsampling           = get_bits1(gb);
        s->content_interpretation_type    = get_bits(gb, 6);

        // spatial_flipping_flag, frame0_flipped_flag, field_views_flag
        skip_bits(gb, 3);
        s->current_frame_is_frame0_flag = get_bits1(gb);
        // frame0_self_contained_flag, frame1_self_contained_flag
        skip_bits(gb, 2);

        if (!s->quincunx_subsampling && s->arrangement_type != 5)
            skip_bits(gb, 16);  // frame[01]_grid_position_[xy]
        skip_bits(gb, 8);       // frame_packing_arrangement_reserved_byte
        skip_bits1(gb);         // frame_packing_arrangement_persistence_flag
    }
    skip_bits1(gb);             // upsampled_aspect_ratio_flag
    return 0;
}

static int decode_nal_sei_display_orientation(HEVCSEIDisplayOrientation *s, GetBitContext *gb)
{
    s->present = !get_bits1(gb);

    if (s->present) {
        s->hflip = get_bits1(gb);     // hor_flip
        s->vflip = get_bits1(gb);     // ver_flip

        s->anticlockwise_rotation = get_bits(gb, 16);
        skip_bits1(gb);     // display_orientation_persistence_flag
    }

    return 0;
}

                                     void *logctx, int size)
{

        return(AVERROR(ENOMEM));

            av_log(logctx, AV_LOG_DEBUG, "Frame/Field Doubling\n");
            h->picture_struct = HEVC_SEI_PIC_STRUCT_FRAME_DOUBLING;
            av_log(logctx, AV_LOG_DEBUG, "Frame/Field Tripling\n");
            h->picture_struct = HEVC_SEI_PIC_STRUCT_FRAME_TRIPLING;
        }
    }

}

static int decode_registered_user_data_closed_caption(HEVCSEIA53Caption *s, GetBitContext *gb,
                                                      int size)
{
    int ret;

    if (size < 3)
       return AVERROR(EINVAL);

    ret = ff_parse_a53_cc(&s->buf_ref, gb->buffer + get_bits_count(gb) / 8, size);

    if (ret < 0)
        return ret;

    skip_bits_long(gb, size * 8);

    return 0;
}

static int decode_nal_sei_user_data_unregistered(HEVCSEIUnregistered *s, GetBitContext *gb,
                                                      int size)
{
    AVBufferRef *buf_ref, **tmp;

    if (size < 16 || size >= INT_MAX - 1)
       return AVERROR_INVALIDDATA;

    tmp = av_realloc_array(s->buf_ref, s->nb_buf_ref + 1, sizeof(*s->buf_ref));
    if (!tmp)
        return AVERROR(ENOMEM);
    s->buf_ref = tmp;

    buf_ref = av_buffer_alloc(size + 1);
    if (!buf_ref)
        return AVERROR(ENOMEM);

    for (int i = 0; i < size; i++)
        buf_ref->data[i] = get_bits(gb, 8);
    buf_ref->data[size] = 0;
    buf_ref->size = size;
    s->buf_ref[s->nb_buf_ref++] = buf_ref;

    return 0;
}

                                                         int size)
{

        return AVERROR(EINVAL);

        skip_bits(gb, 8);
        size--;
    }



        case MKBETAG('G', 'A', '9', '4'):
            return decode_registered_user_data_closed_caption(&s->a53_caption, gb, size);
    }
}

static int decode_nal_sei_active_parameter_sets(HEVCSEI *s, GetBitContext *gb, void *logctx)
{
    int num_sps_ids_minus1;
    int i;
    unsigned active_seq_parameter_set_id;

    get_bits(gb, 4); // active_video_parameter_set_id
    get_bits(gb, 1); // self_contained_cvs_flag
    get_bits(gb, 1); // num_sps_ids_minus1
    num_sps_ids_minus1 = get_ue_golomb_long(gb); // num_sps_ids_minus1

    if (num_sps_ids_minus1 < 0 || num_sps_ids_minus1 > 15) {
        av_log(logctx, AV_LOG_ERROR, "num_sps_ids_minus1 %d invalid\n", num_sps_ids_minus1);
        return AVERROR_INVALIDDATA;
    }

    active_seq_parameter_set_id = get_ue_golomb_long(gb);
    if (active_seq_parameter_set_id >= HEVC_MAX_SPS_COUNT) {
        av_log(logctx, AV_LOG_ERROR, "active_parameter_set_id %d invalid\n", active_seq_parameter_set_id);
        return AVERROR_INVALIDDATA;
    }
    s->active_seq_parameter_set_id = active_seq_parameter_set_id;

    for (i = 1; i <= num_sps_ids_minus1; i++)
        get_ue_golomb_long(gb); // active_seq_parameter_set_id[i]

    return 0;
}

static int decode_nal_sei_alternative_transfer(HEVCSEIAlternativeTransfer *s, GetBitContext *gb)
{
    s->present = 1;
    s->preferred_transfer_characteristics = get_bits(gb, 8);
    return 0;
}

{


            s->units_field_based_flag[i] = get_bits(gb, 1);
            s->counting_type[i]          = get_bits(gb, 5);
            s->full_timestamp_flag[i]    = get_bits(gb, 1);
            s->discontinuity_flag[i]     = get_bits(gb, 1);
            s->cnt_dropped_flag[i]       = get_bits(gb, 1);

            s->n_frames[i]               = get_bits(gb, 9);

            if (s->full_timestamp_flag[i]) {
                s->seconds_value[i]      = av_clip(get_bits(gb, 6), 0, 59);
                s->minutes_value[i]      = av_clip(get_bits(gb, 6), 0, 59);
                s->hours_value[i]        = av_clip(get_bits(gb, 5), 0, 23);
            } else {
                s->seconds_flag[i] = get_bits(gb, 1);
                if (s->seconds_flag[i]) {
                    s->seconds_value[i] = av_clip(get_bits(gb, 6), 0, 59);
                    s->minutes_flag[i]  = get_bits(gb, 1);
                    if (s->minutes_flag[i]) {
                        s->minutes_value[i] = av_clip(get_bits(gb, 6), 0, 59);
                        s->hours_flag[i] =  get_bits(gb, 1);
                        if (s->hours_flag[i]) {
                            s->hours_value[i] = av_clip(get_bits(gb, 5), 0, 23);
                        }
                    }
                }
            }

            s->time_offset_length[i] = get_bits(gb, 5);
            if (s->time_offset_length[i] > 0) {
                s->time_offset_value[i] = get_bits(gb, s->time_offset_length[i]);
            }
        }
    }

}


                                 const HEVCParamSets *ps, int type, int size)
{
    case 256:  // Mismatched value from HM 8.1
        return decode_nal_sei_decoded_picture_hash(&s->picture_hash, gb);
    case HEVC_SEI_TYPE_FRAME_PACKING:
        return decode_nal_sei_frame_packing_arrangement(&s->frame_packing, gb);
    case HEVC_SEI_TYPE_DISPLAY_ORIENTATION:
        return decode_nal_sei_display_orientation(&s->display_orientation, gb);
    case HEVC_SEI_TYPE_MASTERING_DISPLAY_INFO:
        return decode_nal_sei_mastering_display_info(&s->mastering_display, gb);
    case HEVC_SEI_TYPE_CONTENT_LIGHT_LEVEL_INFO:
        return decode_nal_sei_content_light_info(&s->content_light, gb);
    case HEVC_SEI_TYPE_ALTERNATIVE_TRANSFER_CHARACTERISTICS:
        return decode_nal_sei_alternative_transfer(&s->alternative_transfer, gb);
    }
}

                                 int type, int size)
{
    default:
        av_log(logctx, AV_LOG_DEBUG, "Skipped SUFFIX SEI %d\n", type);
        skip_bits_long(gb, 8 * size);
        return 0;
    }
}

                                  const HEVCParamSets *ps, int nal_unit_type)
{

            return AVERROR_INVALIDDATA;
    }
    byte = 0xFF;
            return AVERROR_INVALIDDATA;
    }
    } else { /* nal_unit_type == NAL_SEI_SUFFIX */
    }
}

{
}

                           const HEVCParamSets *ps, int type)
{

    return 1;
}

{

