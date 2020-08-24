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

                                 MPEG2RawSequenceHeader *current)
{








    }

    }

    return 0;
}

                           MPEG2RawUserData *current)
{



#ifdef READ
            return AVERROR(ENOMEM);
    }
#endif


    return 0;
}

                                    MPEG2RawSequenceExtension *current)
{





}

                                            MPEG2RawSequenceDisplayExtension *current)
{



#ifdef READ
#define READ_AND_PATCH(name) do { \
        ui(8, name); \
        if (current->name == 0) { \
            current->name = 2; \
            av_log(ctx->log_ctx, AV_LOG_WARNING, "%s in a sequence display " \
                   "extension had the invalid value 0. Setting it to 2 " \
                   "(meaning unknown) instead.\n", #name); \
        } \
    } while (0)
#undef READ_AND_PATCH
#else
#endif
    } else {
    }


}

                                          MPEG2RawGroupOfPicturesHeader *current)
{




}

                                   MPEG2RawExtraInformation *current,
                                   const char *element_name, const char *marker_name)
{
#ifdef READ

        skip_bits(rw, 1 + 8);
        *rw = start;
        current->extra_information_ref =
            av_buffer_allocz(k + AV_INPUT_BUFFER_PADDING_SIZE);
        if (!current->extra_information_ref)
            return AVERROR(ENOMEM);
        current->extra_information = current->extra_information_ref->data;
    }
#endif

        bit(marker_name, 1);
        xuia(8, element_name,
             current->extra_information[k], 0, 255, 1, k);
    }


}

                                MPEG2RawPictureHeader *current)
{




        current->picture_coding_type == 3) {
    }

    }

                                  "extra_information_picture[k]", "extra_bit_picture"));

    return 0;
}

                                          MPEG2RawPictureCodingExtension *current)
{




        if (current->repeat_first_field) {
            if (current->top_field_first)
                mpeg2->number_of_frame_centre_offsets = 3;
            else
                mpeg2->number_of_frame_centre_offsets = 2;
        } else {
            mpeg2->number_of_frame_centre_offsets = 1;
        }
    } else {
            current->picture_structure == 2) { // Bottom field.
        } else {
            else
        }
    }

        ui(1, v_axis);
        ui(3, field_sequence);
        ui(1, sub_carrier);
        ui(7, burst_amplitude);
        ui(8, sub_carrier_phase);
    }

    return 0;
}

                                        MPEG2RawQuantMatrixExtension *current)
{


    }

    }

    }

    }

    return 0;
}

                                           MPEG2RawPictureDisplayExtension *current)
{


    }

    return 0;
}

                                MPEG2RawExtensionData *current)
{



            (ctx, rw, &current->data.sequence);
            (ctx, rw, &current->data.sequence_display);
            (ctx, rw, &current->data.quant_matrix);
            (ctx, rw, &current->data.picture_display);
            (ctx, rw, &current->data.picture_coding);
    default:
        av_log(ctx->log_ctx, AV_LOG_ERROR, "Extension ID %d not supported.\n",
               current->extension_start_code_identifier);
        return AVERROR_PATCHWELCOME;
    }
}

                              MPEG2RawSliceHeader *current)
{



        ui(3, slice_vertical_position_extension);
        if (mpeg2->scalable_mode == 0)
            ui(7, priority_breakpoint);
    }


    }

                                  "extra_information_slice[k]", "extra_bit_slice"));

    return 0;
}

static int FUNC(sequence_end)(CodedBitstreamContext *ctx, RWContext *rw,
                              MPEG2RawSequenceEnd *current)
{
    int err;

    HEADER("Sequence End");

    ui(8, sequence_end_code);

    return 0;
}
