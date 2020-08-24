/*
 * H.26L/H.264/AVC/JVT/14496-10/... SEI decoding
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
 * H.264 / AVC / MPEG-4 part10 SEI decoding.
 * @author Michael Niedermayer <michaelni@gmx.at>
 */

#include "atsc_a53.h"
#include "avcodec.h"
#include "get_bits.h"
#include "golomb.h"
#include "h264_ps.h"
#include "h264_sei.h"
#include "internal.h"

#define AVERROR_PS_NOT_FOUND      FFERRTAG(0xF8,'?','P','S')

static const uint8_t sei_num_clock_ts_table[9] = {
    1, 1, 1, 2, 2, 3, 3, 2, 3
};

{




                                       void *logctx)
{


    }


            return AVERROR_INVALIDDATA;

                    tc->dropframe = 1;
                } else {
                            tc->minutes = get_bits(&gb, 6);
                            if (get_bits(&gb, 1))       /* hours_flag */
                                tc->hours = get_bits(&gb, 5);
                        }
                    }
                }

                              sps->time_offset_length); /* time_offset */
            }
        }

    }

    return 0;
}

                                 void *logctx)
{

        av_log(logctx, AV_LOG_ERROR, "Unaligned SEI payload\n");
        return AVERROR_INVALIDDATA;
    }
        av_log(logctx, AV_LOG_ERROR, "Picture timing SEI payload too large\n");
        return AVERROR_INVALIDDATA;
    }


}

static int decode_registered_user_data_afd(H264SEIAFD *h, GetBitContext *gb, int size)
{
    int flag;

    if (size-- < 1)
        return AVERROR_INVALIDDATA;
    skip_bits(gb, 1);               // 0
    flag = get_bits(gb, 1);         // active_format_flag
    skip_bits(gb, 6);               // reserved

    if (flag) {
        if (size-- < 1)
            return AVERROR_INVALIDDATA;
        skip_bits(gb, 4);           // reserved
        h->active_format_description = get_bits(gb, 4);
        h->present                   = 1;
    }

    return 0;
}

static int decode_registered_user_data_closed_caption(H264SEIA53Caption *h,
                                                     GetBitContext *gb, void *logctx,
                                                     int size)
{
    if (size < 3)
        return AVERROR(EINVAL);

    return ff_parse_a53_cc(&h->buf_ref, gb->buffer + get_bits_count(gb) / 8, size);
}

static int decode_registered_user_data(H264SEIContext *h, GetBitContext *gb,
                                       void *logctx, int size)
{
    uint32_t country_code;
    uint32_t user_identifier;

    if (size < 7)
        return AVERROR_INVALIDDATA;
    size -= 7;

    country_code = get_bits(gb, 8); // itu_t_t35_country_code
    if (country_code == 0xFF) {
        skip_bits(gb, 8);           // itu_t_t35_country_code_extension_byte
        size--;
    }

    /* itu_t_t35_payload_byte follows */
    skip_bits(gb, 8);              // terminal provider code
    skip_bits(gb, 8);              // terminal provider oriented code
    user_identifier = get_bits_long(gb, 32);

    switch (user_identifier) {
        case MKBETAG('D', 'T', 'G', '1'):       // afd_data
            return decode_registered_user_data_afd(&h->afd, gb, size);
        case MKBETAG('G', 'A', '9', '4'):       // closed captions
            return decode_registered_user_data_closed_caption(&h->a53_caption, gb,
                                                              logctx, size);
        default:
            skip_bits(gb, size * 8);
            break;
    }

    return 0;
}

static int decode_unregistered_user_data(H264SEIUnregistered *h, GetBitContext *gb,
                                         void *logctx, int size)
{
    uint8_t *user_data;
    int e, build, i;
    AVBufferRef *buf_ref, **tmp;

    if (size < 16 || size >= INT_MAX - 1)
        return AVERROR_INVALIDDATA;

    tmp = av_realloc_array(h->buf_ref, h->nb_buf_ref + 1, sizeof(*h->buf_ref));
    if (!tmp)
        return AVERROR(ENOMEM);
    h->buf_ref = tmp;

    buf_ref = av_buffer_alloc(size + 1);
    if (!buf_ref)
        return AVERROR(ENOMEM);
    user_data = buf_ref->data;

    for (i = 0; i < size; i++)
        user_data[i] = get_bits(gb, 8);

    user_data[i] = 0;
    buf_ref->size = size;
    h->buf_ref[h->nb_buf_ref++] = buf_ref;

    e = sscanf(user_data + 16, "x264 - core %d", &build);
    if (e == 1 && build > 0)
        h->x264_build = build;
    if (e == 1 && build == 1 && !strncmp(user_data+16, "x264 - core 0000", 16))
        h->x264_build = 67;

    return 0;
}

static int decode_recovery_point(H264SEIRecoveryPoint *h, GetBitContext *gb, void *logctx)
{
    unsigned recovery_frame_cnt = get_ue_golomb_long(gb);

    if (recovery_frame_cnt >= (1<<MAX_LOG2_MAX_FRAME_NUM)) {
        av_log(logctx, AV_LOG_ERROR, "recovery_frame_cnt %u is out of range\n", recovery_frame_cnt);
        return AVERROR_INVALIDDATA;
    }

    h->recovery_frame_cnt = recovery_frame_cnt;
    /* 1b exact_match_flag,
     * 1b broken_link_flag,
     * 2b changing_slice_group_idc */
    skip_bits(gb, 4);

    return 0;
}

                                   const H264ParamSets *ps, void *logctx)
{

               "non-existing SPS %d referenced in buffering period\n", sps_id);
    }

    // NOTE: This is really so duplicated in the standard... See H.264, D.1.1
            // initial_cpb_removal_delay_offset
        }
    }
            // initial_cpb_removal_delay_offset
        }
    }

}

static int decode_frame_packing_arrangement(H264SEIFramePacking *h,
                                            GetBitContext *gb)
{
    h->arrangement_id          = get_ue_golomb_long(gb);
    h->arrangement_cancel_flag = get_bits1(gb);
    h->present = !h->arrangement_cancel_flag;

    if (h->present) {
        h->arrangement_type = get_bits(gb, 7);
        h->quincunx_sampling_flag         = get_bits1(gb);
        h->content_interpretation_type    = get_bits(gb, 6);

        // spatial_flipping_flag, frame0_flipped_flag, field_views_flag
        skip_bits(gb, 3);
        h->current_frame_is_frame0_flag = get_bits1(gb);
        // frame0_self_contained_flag, frame1_self_contained_flag
        skip_bits(gb, 2);

        if (!h->quincunx_sampling_flag && h->arrangement_type != 5)
            skip_bits(gb, 16);      // frame[01]_grid_position_[xy]
        skip_bits(gb, 8);           // frame_packing_arrangement_reserved_byte
        h->arrangement_repetition_period = get_ue_golomb_long(gb);
    }
    skip_bits1(gb);                 // frame_packing_arrangement_extension_flag

    return 0;
}

                                      GetBitContext *gb)
{


    }

}

static int decode_green_metadata(H264SEIGreenMetaData *h, GetBitContext *gb)
{
    h->green_metadata_type = get_bits(gb, 8);

    if (h->green_metadata_type == 0) {
        h->period_type = get_bits(gb, 8);

        if (h->period_type == 2)
            h->num_seconds = get_bits(gb, 16);
        else if (h->period_type == 3)
            h->num_pictures = get_bits(gb, 16);

        h->percent_non_zero_macroblocks            = get_bits(gb, 8);
        h->percent_intra_coded_macroblocks         = get_bits(gb, 8);
        h->percent_six_tap_filtering               = get_bits(gb, 8);
        h->percent_alpha_point_deblocking_instance = get_bits(gb, 8);

    } else if (h->green_metadata_type == 1) {
        h->xsd_metric_type  = get_bits(gb, 8);
        h->xsd_metric_value = get_bits(gb, 16);
    }

    return 0;
}

static int decode_alternative_transfer(H264SEIAlternativeTransfer *h,
                                       GetBitContext *gb)
{
    h->present = 1;
    h->preferred_transfer_characteristics = get_bits(gb, 8);
    return 0;
}

                       const H264ParamSets *ps, void *logctx)
{

        GetBitContext gb_payload;
        int type = 0;


                return AVERROR_INVALIDDATA;

                   type, 8*size, get_bits_left(gb));
        }

            return ret;

        case H264_SEI_TYPE_FRAME_PACKING:
            ret = decode_frame_packing_arrangement(&h->frame_packing, &gb_payload);
            break;
        case H264_SEI_TYPE_GREEN_METADATA:
            ret = decode_green_metadata(&h->green_metadata, &gb_payload);
            break;
        case H264_SEI_TYPE_ALTERNATIVE_TRANSFER:
            ret = decode_alternative_transfer(&h->alternative_transfer, &gb_payload);
            break;
        }
            return ret;

        }

    }

    return master_ret;
}

{
        switch (h->arrangement_type) {
            case H264_SEI_FPA_TYPE_CHECKERBOARD:
                if (h->content_interpretation_type == 2)
                    return "checkerboard_rl";
                else
                    return "checkerboard_lr";
            case H264_SEI_FPA_TYPE_INTERLEAVE_COLUMN:
                if (h->content_interpretation_type == 2)
                    return "col_interleaved_rl";
                else
                    return "col_interleaved_lr";
            case H264_SEI_FPA_TYPE_INTERLEAVE_ROW:
                if (h->content_interpretation_type == 2)
                    return "row_interleaved_rl";
                else
                    return "row_interleaved_lr";
            case H264_SEI_FPA_TYPE_SIDE_BY_SIDE:
                if (h->content_interpretation_type == 2)
                    return "right_left";
                else
                    return "left_right";
            case H264_SEI_FPA_TYPE_TOP_BOTTOM:
                if (h->content_interpretation_type == 2)
                    return "bottom_top";
                else
                    return "top_bottom";
            case H264_SEI_FPA_TYPE_INTERLEAVE_TEMPORAL:
                if (h->content_interpretation_type == 2)
                    return "block_rl";
                else
                    return "block_lr";
            case H264_SEI_FPA_TYPE_2D:
            default:
                return "mono";
        }
        return "mono";
    } else {
    }
}
