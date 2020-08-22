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

#include "libavutil/attributes.h"
#include "libavutil/avassert.h"

#include "bytestream.h"
#include "cbs.h"
#include "cbs_internal.h"
#include "cbs_h264.h"
#include "cbs_h265.h"
#include "h264.h"
#include "h264_sei.h"
#include "h2645_parse.h"
#include "hevc.h"
#include "hevc_sei.h"


                              const char *name, const int *subscripts,
                              uint32_t *write_to,
                              uint32_t range_min, uint32_t range_max)
{


            av_log(ctx->log_ctx, AV_LOG_ERROR, "Invalid ue-golomb code at "
                   "%s: bitstream ended.\n", name);
            return AVERROR_INVALIDDATA;
        }
            break;
    }
        av_log(ctx->log_ctx, AV_LOG_ERROR, "Invalid ue-golomb code at "
               "%s: more than 31 zeroes.\n", name);
        return AVERROR_INVALIDDATA;
    }
    value = 1;
    }

        ff_cbs_trace_syntax_element(ctx, position, name, subscripts,
                                    bits, value);

        av_log(ctx->log_ctx, AV_LOG_ERROR, "%s out of range: "
               "%"PRIu32", but must be in [%"PRIu32",%"PRIu32"].\n",
               name, value, range_min, range_max);
        return AVERROR_INVALIDDATA;
    }

}

                              const char *name, const int *subscripts,
                              int32_t *write_to,
                              int32_t range_min, int32_t range_max)
{


            av_log(ctx->log_ctx, AV_LOG_ERROR, "Invalid se-golomb code at "
                   "%s: bitstream ended.\n", name);
            return AVERROR_INVALIDDATA;
        }
            break;
    }
        av_log(ctx->log_ctx, AV_LOG_ERROR, "Invalid se-golomb code at "
               "%s: more than 31 zeroes.\n", name);
        return AVERROR_INVALIDDATA;
    }
    v = 1;
    }
    else

        ff_cbs_trace_syntax_element(ctx, position, name, subscripts,
                                    bits, value);

        av_log(ctx->log_ctx, AV_LOG_ERROR, "%s out of range: "
               "%"PRId32", but must be in [%"PRId32",%"PRId32"].\n",
               name, value, range_min, range_max);
        return AVERROR_INVALIDDATA;
    }

}

                               const char *name, const int *subscripts,
                               uint32_t value,
                               uint32_t range_min, uint32_t range_max)
{

        av_log(ctx->log_ctx, AV_LOG_ERROR, "%s out of range: "
               "%"PRIu32", but must be in [%"PRIu32",%"PRIu32"].\n",
               name, value, range_min, range_max);
        return AVERROR_INVALIDDATA;
    }

        return AVERROR(ENOSPC);

        char bits[65];
        int i;

        for (i = 0; i < len; i++)
            bits[i] = '0';
        bits[len] = '1';
        for (i = 0; i < len; i++)
            bits[len + i + 1] = (value + 1) >> (len - i - 1) & 1 ? '1' : '0';
        bits[len + len + 1] = 0;

        ff_cbs_trace_syntax_element(ctx, put_bits_count(pbc),
                                    name, subscripts, bits, value);
    }

    else
        put_bits32(pbc, value + 1);

    return 0;
}

                               const char *name, const int *subscripts,
                               int32_t value,
                               int32_t range_min, int32_t range_max)
{

        av_log(ctx->log_ctx, AV_LOG_ERROR, "%s out of range: "
               "%"PRId32", but must be in [%"PRId32",%"PRId32"].\n",
               name, value, range_min, range_max);
        return AVERROR_INVALIDDATA;
    }

        uvalue = 0;
    else

        return AVERROR(ENOSPC);

        char bits[65];
        int i;

        for (i = 0; i < len; i++)
            bits[i] = '0';
        bits[len] = '1';
        for (i = 0; i < len; i++)
            bits[len + i + 1] = (uvalue + 1) >> (len - i - 1) & 1 ? '1' : '0';
        bits[len + len + 1] = 0;

        ff_cbs_trace_syntax_element(ctx, put_bits_count(pbc),
                                    name, subscripts, bits, value);
    }

    else
        put_bits32(pbc, uvalue + 1);

    return 0;
}

// payload_extension_present() - true if we are before the last 1-bit
// in the payload structure, which must be in the last byte.
                                              int cur_pos)
{
}

#define HEADER(name) do { \
        ff_cbs_trace_header(ctx, name); \
    } while (0)

#define CHECK(call) do { \
        err = (call); \
        if (err < 0) \
            return err; \
    } while (0)

#define FUNC_NAME(rw, codec, name) cbs_ ## codec ## _ ## rw ## _ ## name
#define FUNC_H264(rw, name) FUNC_NAME(rw, h264, name)
#define FUNC_H265(rw, name) FUNC_NAME(rw, h265, name)

#define SUBSCRIPTS(subs, ...) (subs > 0 ? ((int[subs + 1]){ subs, __VA_ARGS__ }) : NULL)

#define u(width, name, range_min, range_max) \
        xu(width, name, current->name, range_min, range_max, 0, )
#define ub(width, name) \
        xu(width, name, current->name, 0, MAX_UINT_BITS(width), 0, )
#define flag(name) ub(1, name)
#define ue(name, range_min, range_max) \
        xue(name, current->name, range_min, range_max, 0, )
#define i(width, name, range_min, range_max) \
        xi(width, name, current->name, range_min, range_max, 0, )
#define ib(width, name) \
        xi(width, name, current->name, MIN_INT_BITS(width), MAX_INT_BITS(width), 0, )
#define se(name, range_min, range_max) \
        xse(name, current->name, range_min, range_max, 0, )

#define us(width, name, range_min, range_max, subs, ...) \
        xu(width, name, current->name, range_min, range_max, subs, __VA_ARGS__)
#define ubs(width, name, subs, ...) \
        xu(width, name, current->name, 0, MAX_UINT_BITS(width), subs, __VA_ARGS__)
#define flags(name, subs, ...) \
        xu(1, name, current->name, 0, 1, subs, __VA_ARGS__)
#define ues(name, range_min, range_max, subs, ...) \
        xue(name, current->name, range_min, range_max, subs, __VA_ARGS__)
#define is(width, name, range_min, range_max, subs, ...) \
        xi(width, name, current->name, range_min, range_max, subs, __VA_ARGS__)
#define ibs(width, name, subs, ...) \
        xi(width, name, current->name, MIN_INT_BITS(width), MAX_INT_BITS(width), subs, __VA_ARGS__)
#define ses(name, range_min, range_max, subs, ...) \
        xse(name, current->name, range_min, range_max, subs, __VA_ARGS__)

#define fixed(width, name, value) do { \
        av_unused uint32_t fixed_value = value; \
        xu(width, name, fixed_value, value, value, 0, ); \
    } while (0)


#define READ
#define READWRITE read
#define RWContext GetBitContext

#define xu(width, name, var, range_min, range_max, subs, ...) do { \
        uint32_t value; \
        CHECK(ff_cbs_read_unsigned(ctx, rw, width, #name, \
                                   SUBSCRIPTS(subs, __VA_ARGS__), \
                                   &value, range_min, range_max)); \
        var = value; \
    } while (0)
#define xue(name, var, range_min, range_max, subs, ...) do { \
        uint32_t value; \
        CHECK(cbs_read_ue_golomb(ctx, rw, #name, \
                                 SUBSCRIPTS(subs, __VA_ARGS__), \
                                 &value, range_min, range_max)); \
        var = value; \
    } while (0)
#define xi(width, name, var, range_min, range_max, subs, ...) do { \
        int32_t value; \
        CHECK(ff_cbs_read_signed(ctx, rw, width, #name, \
                                 SUBSCRIPTS(subs, __VA_ARGS__), \
                                 &value, range_min, range_max)); \
        var = value; \
    } while (0)
#define xse(name, var, range_min, range_max, subs, ...) do { \
        int32_t value; \
        CHECK(cbs_read_se_golomb(ctx, rw, #name, \
                                 SUBSCRIPTS(subs, __VA_ARGS__), \
                                 &value, range_min, range_max)); \
        var = value; \
    } while (0)


#define infer(name, value) do { \
        current->name = value; \
    } while (0)

{
        return 1;
        return 0;
    return 0;
}

#define more_rbsp_data(var) ((var) = cbs_h2645_read_more_rbsp_data(rw))

#define byte_alignment(rw) (get_bits_count(rw) % 8)

#define allocate(name, size) do { \
        name ## _ref = av_buffer_allocz(size + \
                                        AV_INPUT_BUFFER_PADDING_SIZE); \
        if (!name ## _ref) \
            return AVERROR(ENOMEM); \
        name = name ## _ref->data; \
    } while (0)

#define FUNC(name) FUNC_H264(READWRITE, name)
#include "cbs_h264_syntax_template.c"
#undef FUNC

#define FUNC(name) FUNC_H265(READWRITE, name)
#include "cbs_h265_syntax_template.c"
#undef FUNC

#undef READ
#undef READWRITE
#undef RWContext
#undef xu
#undef xi
#undef xue
#undef xse
#undef infer
#undef more_rbsp_data
#undef byte_alignment
#undef allocate


#define WRITE
#define READWRITE write
#define RWContext PutBitContext

#define xu(width, name, var, range_min, range_max, subs, ...) do { \
        uint32_t value = var; \
        CHECK(ff_cbs_write_unsigned(ctx, rw, width, #name, \
                                    SUBSCRIPTS(subs, __VA_ARGS__), \
                                    value, range_min, range_max)); \
    } while (0)
#define xue(name, var, range_min, range_max, subs, ...) do { \
        uint32_t value = var; \
        CHECK(cbs_write_ue_golomb(ctx, rw, #name, \
                                  SUBSCRIPTS(subs, __VA_ARGS__), \
                                  value, range_min, range_max)); \
    } while (0)
#define xi(width, name, var, range_min, range_max, subs, ...) do { \
        int32_t value = var; \
        CHECK(ff_cbs_write_signed(ctx, rw, width, #name, \
                                  SUBSCRIPTS(subs, __VA_ARGS__), \
                                  value, range_min, range_max)); \
    } while (0)
#define xse(name, var, range_min, range_max, subs, ...) do { \
        int32_t value = var; \
        CHECK(cbs_write_se_golomb(ctx, rw, #name, \
                                  SUBSCRIPTS(subs, __VA_ARGS__), \
                                  value, range_min, range_max)); \
    } while (0)

#define infer(name, value) do { \
        if (current->name != (value)) { \
            av_log(ctx->log_ctx, AV_LOG_ERROR, \
                   "%s does not match inferred value: " \
                   "%"PRId64", but should be %"PRId64".\n", \
                   #name, (int64_t)current->name, (int64_t)(value)); \
            return AVERROR_INVALIDDATA; \
        } \
    } while (0)

#define more_rbsp_data(var) (var)

#define byte_alignment(rw) (put_bits_count(rw) % 8)

#define allocate(name, size) do { \
        if (!name) { \
            av_log(ctx->log_ctx, AV_LOG_ERROR, "%s must be set " \
                   "for writing.\n", #name); \
            return AVERROR_INVALIDDATA; \
        } \
    } while (0)

#define FUNC(name) FUNC_H264(READWRITE, name)
#include "cbs_h264_syntax_template.c"
#undef FUNC

#define FUNC(name) FUNC_H265(READWRITE, name)
#include "cbs_h265_syntax_template.c"
#undef FUNC

#undef WRITE
#undef READWRITE
#undef RWContext
#undef xu
#undef xi
#undef xue
#undef xse
#undef u
#undef i
#undef flag
#undef ue
#undef se
#undef infer
#undef more_rbsp_data
#undef byte_alignment
#undef allocate


{

{
    case H264_SEI_TYPE_BUFFERING_PERIOD:
    case H264_SEI_TYPE_PIC_TIMING:
    case H264_SEI_TYPE_PAN_SCAN_RECT:
    case H264_SEI_TYPE_RECOVERY_POINT:
    case H264_SEI_TYPE_DISPLAY_ORIENTATION:
    case H264_SEI_TYPE_MASTERING_DISPLAY_COLOUR_VOLUME:
    case H264_SEI_TYPE_ALTERNATIVE_TRANSFER:
        break;
    case H264_SEI_TYPE_USER_DATA_UNREGISTERED:
        av_buffer_unref(&payload->payload.user_data_unregistered.data_ref);
        break;
    }

{

{

{

{

{

{

{
    case HEVC_SEI_TYPE_BUFFERING_PERIOD:
    case HEVC_SEI_TYPE_PICTURE_TIMING:
    case HEVC_SEI_TYPE_PAN_SCAN_RECT:
    case HEVC_SEI_TYPE_RECOVERY_POINT:
    case HEVC_SEI_TYPE_DISPLAY_ORIENTATION:
    case HEVC_SEI_TYPE_ACTIVE_PARAMETER_SETS:
    case HEVC_SEI_TYPE_DECODED_PICTURE_HASH:
    case HEVC_SEI_TYPE_TIME_CODE:
    case HEVC_SEI_TYPE_MASTERING_DISPLAY_INFO:
    case HEVC_SEI_TYPE_CONTENT_LIGHT_LEVEL_INFO:
    case HEVC_SEI_TYPE_ALTERNATIVE_TRANSFER_CHARACTERISTICS:
    case HEVC_SEI_TYPE_ALPHA_CHANNEL_INFO:
        break;
    case HEVC_SEI_TYPE_USER_DATA_REGISTERED_ITU_T_T35:
        av_buffer_unref(&payload->payload.user_data_registered.data_ref);
        break;
    case HEVC_SEI_TYPE_USER_DATA_UNREGISTERED:
        av_buffer_unref(&payload->payload.user_data_unregistered.data_ref);
        break;
    default:
        av_buffer_unref(&payload->payload.other.data_ref);
        break;
    }

{

static int cbs_h2645_fragment_add_nals(CodedBitstreamContext *ctx,
                                       CodedBitstreamFragment *frag,
                                       const H2645Packet *packet)
{
    int err, i;

    for (i = 0; i < packet->nb_nals; i++) {
        const H2645NAL *nal = &packet->nals[i];
        AVBufferRef *ref;
        size_t size = nal->size;

        if (nal->nuh_layer_id > 0)
            continue;

        // Remove trailing zeroes.
        while (size > 0 && nal->data[size - 1] == 0)
            --size;
        if (size == 0) {
            av_log(ctx->log_ctx, AV_LOG_VERBOSE, "Discarding empty 0 NAL unit\n");
            continue;
        }

        ref = (nal->data == nal->raw_data) ? frag->data_ref
                                           : packet->rbsp.rbsp_buffer_ref;

        err = ff_cbs_insert_unit_data(frag, -1, nal->type,
                            (uint8_t*)nal->data, size, ref);
        if (err < 0)
            return err;
    }

    return 0;
}

                                    CodedBitstreamFragment *frag,
                                    int header)
{

        return 0;

        // AVCC header.
        size_t size, start, end;
        int i, count, version;

        priv->mp4 = 1;

        bytestream2_init(&gbc, frag->data, frag->data_size);

        if (bytestream2_get_bytes_left(&gbc) < 6)
            return AVERROR_INVALIDDATA;

        version = bytestream2_get_byte(&gbc);
        if (version != 1) {
            av_log(ctx->log_ctx, AV_LOG_ERROR, "Invalid AVCC header: "
                   "first byte %u.\n", version);
            return AVERROR_INVALIDDATA;
        }

        bytestream2_skip(&gbc, 3);
        priv->nal_length_size = (bytestream2_get_byte(&gbc) & 3) + 1;

        // SPS array.
        count = bytestream2_get_byte(&gbc) & 0x1f;
        start = bytestream2_tell(&gbc);
        for (i = 0; i < count; i++) {
            if (bytestream2_get_bytes_left(&gbc) < 2 * (count - i))
                return AVERROR_INVALIDDATA;
            size = bytestream2_get_be16(&gbc);
            if (bytestream2_get_bytes_left(&gbc) < size)
                return AVERROR_INVALIDDATA;
            bytestream2_skip(&gbc, size);
        }
        end = bytestream2_tell(&gbc);

        err = ff_h2645_packet_split(&priv->read_packet,
                                    frag->data + start, end - start,
                                    ctx->log_ctx, 1, 2, AV_CODEC_ID_H264, 1, 1);
        if (err < 0) {
            av_log(ctx->log_ctx, AV_LOG_ERROR, "Failed to split AVCC SPS array.\n");
            return err;
        }
        err = cbs_h2645_fragment_add_nals(ctx, frag, &priv->read_packet);
        if (err < 0)
            return err;

        // PPS array.
        count = bytestream2_get_byte(&gbc);
        start = bytestream2_tell(&gbc);
        for (i = 0; i < count; i++) {
            if (bytestream2_get_bytes_left(&gbc) < 2 * (count - i))
                return AVERROR_INVALIDDATA;
            size = bytestream2_get_be16(&gbc);
            if (bytestream2_get_bytes_left(&gbc) < size)
                return AVERROR_INVALIDDATA;
            bytestream2_skip(&gbc, size);
        }
        end = bytestream2_tell(&gbc);

        err = ff_h2645_packet_split(&priv->read_packet,
                                    frag->data + start, end - start,
                                    ctx->log_ctx, 1, 2, AV_CODEC_ID_H264, 1, 1);
        if (err < 0) {
            av_log(ctx->log_ctx, AV_LOG_ERROR, "Failed to split AVCC PPS array.\n");
            return err;
        }
        err = cbs_h2645_fragment_add_nals(ctx, frag, &priv->read_packet);
        if (err < 0)
            return err;

        if (bytestream2_get_bytes_left(&gbc) > 0) {
            av_log(ctx->log_ctx, AV_LOG_WARNING, "%u bytes left at end of AVCC "
                   "header.\n", bytestream2_get_bytes_left(&gbc));
        }

        // HVCC header.
        size_t size, start, end;
        int i, j, nb_arrays, nal_unit_type, nb_nals, version;

        priv->mp4 = 1;

        bytestream2_init(&gbc, frag->data, frag->data_size);

        if (bytestream2_get_bytes_left(&gbc) < 23)
            return AVERROR_INVALIDDATA;

        version = bytestream2_get_byte(&gbc);
        if (version != 1) {
            av_log(ctx->log_ctx, AV_LOG_ERROR, "Invalid HVCC header: "
                   "first byte %u.\n", version);
            return AVERROR_INVALIDDATA;
        }

        bytestream2_skip(&gbc, 20);
        priv->nal_length_size = (bytestream2_get_byte(&gbc) & 3) + 1;

        nb_arrays = bytestream2_get_byte(&gbc);
        for (i = 0; i < nb_arrays; i++) {
            nal_unit_type = bytestream2_get_byte(&gbc) & 0x3f;
            nb_nals = bytestream2_get_be16(&gbc);

            start = bytestream2_tell(&gbc);
            for (j = 0; j < nb_nals; j++) {
                if (bytestream2_get_bytes_left(&gbc) < 2)
                    return AVERROR_INVALIDDATA;
                size = bytestream2_get_be16(&gbc);
                if (bytestream2_get_bytes_left(&gbc) < size)
                    return AVERROR_INVALIDDATA;
                bytestream2_skip(&gbc, size);
            }
            end = bytestream2_tell(&gbc);

            err = ff_h2645_packet_split(&priv->read_packet,
                                        frag->data + start, end - start,
                                        ctx->log_ctx, 1, 2, AV_CODEC_ID_HEVC, 1, 1);
            if (err < 0) {
                av_log(ctx->log_ctx, AV_LOG_ERROR, "Failed to split "
                       "HVCC array %d (%d NAL units of type %d).\n",
                       i, nb_nals, nal_unit_type);
                return err;
            }
            err = cbs_h2645_fragment_add_nals(ctx, frag, &priv->read_packet);
            if (err < 0)
                return err;
        }

    } else {
        // Annex B, or later MP4 with already-known parameters.

                                    frag->data, frag->data_size,
                                    ctx->log_ctx,
                                    priv->mp4, priv->nal_length_size,
                                    codec_id, 1, 1);
            return err;

            return err;
    }

    return 0;
}

#define cbs_h2645_replace_ps(h26n, ps_name, ps_var, id_element) \
static int cbs_h26 ## h26n ## _replace_ ## ps_var(CodedBitstreamContext *ctx, \
                                                  CodedBitstreamUnit *unit)  \
{ \
    CodedBitstreamH26 ## h26n ## Context *priv = ctx->priv_data; \
    H26 ## h26n ## Raw ## ps_name *ps_var = unit->content; \
    unsigned int id = ps_var->id_element; \
    if (id >= FF_ARRAY_ELEMS(priv->ps_var)) { \
        av_log(ctx->log_ctx, AV_LOG_ERROR, "Invalid " #ps_name \
               " id : %d.\n", id); \
        return AVERROR_INVALIDDATA; \
    } \
    if (priv->ps_var[id] == priv->active_ ## ps_var) \
        priv->active_ ## ps_var = NULL ; \
    av_buffer_unref(&priv->ps_var ## _ref[id]); \
    if (unit->content_ref) \
        priv->ps_var ## _ref[id] = av_buffer_ref(unit->content_ref); \
    else \
        priv->ps_var ## _ref[id] = av_buffer_alloc(sizeof(*ps_var)); \
    if (!priv->ps_var ## _ref[id]) \
        return AVERROR(ENOMEM); \
    priv->ps_var[id] = (H26 ## h26n ## Raw ## ps_name *)priv->ps_var ## _ref[id]->data; \
    if (!unit->content_ref) \
        memcpy(priv->ps_var[id], ps_var, sizeof(*ps_var)); \
    return 0; \
}

cbs_h2645_replace_ps(4, SPS, sps, seq_parameter_set_id)
cbs_h2645_replace_ps(4, PPS, pps, pic_parameter_set_id)
cbs_h2645_replace_ps(5, VPS, vps, vps_video_parameter_set_id)
cbs_h2645_replace_ps(5, SPS, sps, sps_seq_parameter_set_id)
cbs_h2645_replace_ps(5, PPS, pps, pps_pic_parameter_set_id)

                                  CodedBitstreamUnit *unit)
{

        return err;

        {

                return err;

                return err;

                return err;
        }
        break;

    case H264_NAL_SPS_EXT:
        {
            err = ff_cbs_alloc_unit_content(unit,
                                            sizeof(H264RawSPSExtension),
                                            NULL);
            if (err < 0)
                return err;

            err = cbs_h264_read_sps_extension(ctx, &gbc, unit->content);
            if (err < 0)
                return err;
        }
        break;

        {

                                            &cbs_h264_free_pps);
                return err;

                return err;

                return err;
        }
        break;

    case H264_NAL_IDR_SLICE:
    case H264_NAL_AUXILIARY_SLICE:
        {

                                            &cbs_h264_free_slice);
                return err;

                return err;

                return AVERROR_INVALIDDATA;


                return AVERROR(ENOMEM);
        }

        {
                                            sizeof(H264RawAUD), NULL);
                return err;

                return err;
        }
        break;

        {
                                            &cbs_h264_free_sei);
                return err;

                return err;
        }
        break;

    case H264_NAL_FILLER_DATA:
        {
            err = ff_cbs_alloc_unit_content(unit,
                                            sizeof(H264RawFiller), NULL);
            if (err < 0)
                return err;

            err = cbs_h264_read_filler(ctx, &gbc, unit->content);
            if (err < 0)
                return err;
        }
        break;

    case H264_NAL_END_STREAM:
        {
                                            sizeof(H264RawNALUnitHeader),
                                            NULL);
                return err;

                return err;
        }
        break;

    default:
        return AVERROR(ENOSYS);
    }

    return 0;
}

                                  CodedBitstreamUnit *unit)
{

        return err;

        {

                                            &cbs_h265_free_vps);
                return err;

                return err;

                return err;
        }
        break;
        {

                                            &cbs_h265_free_sps);
                return err;

                return err;

                return err;
        }
        break;

        {

                                            &cbs_h265_free_pps);
                return err;

                return err;

                return err;
        }
        break;

    case HEVC_NAL_TRAIL_R:
    case HEVC_NAL_TSA_N:
    case HEVC_NAL_TSA_R:
    case HEVC_NAL_STSA_N:
    case HEVC_NAL_STSA_R:
    case HEVC_NAL_RADL_N:
    case HEVC_NAL_RADL_R:
    case HEVC_NAL_RASL_N:
    case HEVC_NAL_RASL_R:
    case HEVC_NAL_BLA_W_LP:
    case HEVC_NAL_BLA_W_RADL:
    case HEVC_NAL_BLA_N_LP:
    case HEVC_NAL_IDR_W_RADL:
    case HEVC_NAL_IDR_N_LP:
    case HEVC_NAL_CRA_NUT:
        {

                                            &cbs_h265_free_slice);
                return err;

                return err;

                return AVERROR_INVALIDDATA;


                return AVERROR(ENOMEM);
        }

        {
                                            sizeof(H265RawAUD), NULL);
                return err;

                return err;
        }
        break;

    case HEVC_NAL_SEI_SUFFIX:
        {
                                            &cbs_h265_free_sei);

                return err;


                return err;
        }
        break;

    default:
        return AVERROR(ENOSYS);
    }

    return 0;
}

static int cbs_h2645_write_slice_data(CodedBitstreamContext *ctx,
                                      PutBitContext *pbc, const uint8_t *data,
                                      size_t data_size, int data_bit_start)
{
    size_t rest  = data_size - (data_bit_start + 7) / 8;
    const uint8_t *pos = data + data_bit_start / 8;

    av_assert0(data_bit_start >= 0 &&
               data_size > data_bit_start / 8);

    if (data_size * 8 + 8 > put_bits_left(pbc))
        return AVERROR(ENOSPC);

    if (!rest)
        goto rbsp_stop_one_bit;

    // First copy the remaining bits of the first byte
    // The above check ensures that we do not accidentally
    // copy beyond the rbsp_stop_one_bit.
    if (data_bit_start % 8)
        put_bits(pbc, 8 - data_bit_start % 8,
                 *pos++ & MAX_UINT_BITS(8 - data_bit_start % 8));

    if (put_bits_count(pbc) % 8 == 0) {
        // If the writer is aligned at this point,
        // memcpy can be used to improve performance.
        // This happens normally for CABAC.
        flush_put_bits(pbc);
        memcpy(put_bits_ptr(pbc), pos, rest);
        skip_put_bytes(pbc, rest);
    } else {
        // If not, we have to copy manually.
        // rbsp_stop_one_bit forces us to special-case
        // the last byte.
        uint8_t temp;
        int i;

        for (; rest > 4; rest -= 4, pos += 4)
            put_bits32(pbc, AV_RB32(pos));

        for (; rest > 1; rest--, pos++)
            put_bits(pbc, 8, *pos);

    rbsp_stop_one_bit:
        temp = rest ? *pos : *pos & MAX_UINT_BITS(8 - data_bit_start % 8);

        av_assert0(temp);
        i = ff_ctz(*pos);
        temp = temp >> i;
        i = rest ? (8 - i) : (8 - i - data_bit_start % 8);
        put_bits(pbc, i, temp);
        if (put_bits_count(pbc) % 8)
            put_bits(pbc, 8 - put_bits_count(pbc) % 8, 0);
    }

    return 0;
}

                                   CodedBitstreamUnit *unit,
                                   PutBitContext *pbc)
{

        {

                return err;

                return err;
        }
        break;

    case H264_NAL_SPS_EXT:
        {
            H264RawSPSExtension *sps_ext = unit->content;

            err = cbs_h264_write_sps_extension(ctx, pbc, sps_ext);
            if (err < 0)
                return err;
        }
        break;

        {

                return err;

                return err;
        }
        break;

    case H264_NAL_IDR_SLICE:
    case H264_NAL_AUXILIARY_SLICE:
        {

                return err;

                                                 slice->data_size,
                                                 slice->data_bit_start);
                    return err;
            } else {
                // No slice data - that was just the header.
                // (Bitstream may be unaligned!)
            }
        }
        break;

        {
                return err;
        }
        break;

        {
                return err;
        }
        break;

    case H264_NAL_FILLER_DATA:
        {
            err = cbs_h264_write_filler(ctx, pbc, unit->content);
            if (err < 0)
                return err;
        }
        break;

        {
                return err;
        }
        break;

        {
                return err;
        }
        break;

    default:
        av_log(ctx->log_ctx, AV_LOG_ERROR, "Write unimplemented for "
               "NAL unit type %"PRIu32".\n", unit->type);
        return AVERROR_PATCHWELCOME;
    }

    return 0;
}

                                   CodedBitstreamUnit *unit,
                                   PutBitContext *pbc)
{

        {

                return err;

                return err;
        }
        break;

        {

                return err;

                return err;
        }
        break;

        {

                return err;

                return err;
        }
        break;

    case HEVC_NAL_TRAIL_R:
    case HEVC_NAL_TSA_N:
    case HEVC_NAL_TSA_R:
    case HEVC_NAL_STSA_N:
    case HEVC_NAL_STSA_R:
    case HEVC_NAL_RADL_N:
    case HEVC_NAL_RADL_R:
    case HEVC_NAL_RASL_N:
    case HEVC_NAL_RASL_R:
    case HEVC_NAL_BLA_W_LP:
    case HEVC_NAL_BLA_W_RADL:
    case HEVC_NAL_BLA_N_LP:
    case HEVC_NAL_IDR_W_RADL:
    case HEVC_NAL_IDR_N_LP:
    case HEVC_NAL_CRA_NUT:
        {

                return err;

                                                 slice->data_size,
                                                 slice->data_bit_start);
                    return err;
            } else {
                // No slice data - that was just the header.
            }
        }
        break;

        {
                return err;
        }
        break;

    case HEVC_NAL_SEI_SUFFIX:
        {
                                     unit->type == HEVC_NAL_SEI_PREFIX);

                return err;
        }
        break;

    default:
        av_log(ctx->log_ctx, AV_LOG_ERROR, "Write unimplemented for "
               "NAL unit type %"PRIu32".\n", unit->type);
        return AVERROR_PATCHWELCOME;
    }

    return 0;
}

                                       CodedBitstreamFragment *frag)
{

        // Data should already all have been written when we get here.
    }

    max_size = 0;
        // Start code + content with worst-case emulation prevention.
    }

        return AVERROR(ENOMEM);

    dp = 0;

            if (i < frag->nb_units - 1)
                av_log(ctx->log_ctx, AV_LOG_WARNING, "Probably invalid "
                       "unaligned padding on non-final NAL unit.\n");
            else
                frag->data_bit_padding = unit->data_bit_padding;
        }

            i == 0 /* (Assume this is the start of an access unit.) */) {
            // zero_byte
        }
        // start_code_prefix_one_3bytes

                else
                    zero_run = 0;
            } else {
                    // emulation_prevention_three_byte
                }
            }
        }
    }

        return err;

                                      NULL, NULL, 0);
        av_freep(&data);
        return AVERROR(ENOMEM);
    }


}

{



{



const CodedBitstreamType ff_cbs_type_h264 = {
    .codec_id          = AV_CODEC_ID_H264,

    .priv_data_size    = sizeof(CodedBitstreamH264Context),

    .split_fragment    = &cbs_h2645_split_fragment,
    .read_unit         = &cbs_h264_read_nal_unit,
    .write_unit        = &cbs_h264_write_nal_unit,
    .assemble_fragment = &cbs_h2645_assemble_fragment,

    .close             = &cbs_h264_close,
};

const CodedBitstreamType ff_cbs_type_h265 = {
    .codec_id          = AV_CODEC_ID_HEVC,

    .priv_data_size    = sizeof(CodedBitstreamH265Context),

    .split_fragment    = &cbs_h2645_split_fragment,
    .read_unit         = &cbs_h265_read_nal_unit,
    .write_unit        = &cbs_h265_write_nal_unit,
    .assemble_fragment = &cbs_h2645_assemble_fragment,

    .close             = &cbs_h265_close,
};

int ff_cbs_h264_add_sei_message(CodedBitstreamFragment *au,
                                H264RawSEIPayload *payload)
{
    H264RawSEI *sei = NULL;
    int err, i;

    // Find an existing SEI NAL unit to add to.
    for (i = 0; i < au->nb_units; i++) {
        if (au->units[i].type == H264_NAL_SEI) {
            sei = au->units[i].content;
            if (sei->payload_count < H264_MAX_SEI_PAYLOADS)
                break;

            sei = NULL;
        }
    }

    if (!sei) {
        // Need to make a new SEI NAL unit.  Insert it before the first
        // slice data NAL unit; if no slice data, add at the end.
        AVBufferRef *sei_ref;

        sei = av_mallocz(sizeof(*sei));
        if (!sei) {
            err = AVERROR(ENOMEM);
            goto fail;
        }

        sei->nal_unit_header.nal_unit_type = H264_NAL_SEI;
        sei->nal_unit_header.nal_ref_idc   = 0;

        sei_ref = av_buffer_create((uint8_t*)sei, sizeof(*sei),
                                   &cbs_h264_free_sei, NULL, 0);
        if (!sei_ref) {
            av_freep(&sei);
            err = AVERROR(ENOMEM);
            goto fail;
        }

        for (i = 0; i < au->nb_units; i++) {
            if (au->units[i].type == H264_NAL_SLICE ||
                au->units[i].type == H264_NAL_IDR_SLICE)
                break;
        }

        err = ff_cbs_insert_unit_content(au, i, H264_NAL_SEI,
                                         sei, sei_ref);
        av_buffer_unref(&sei_ref);
        if (err < 0)
            goto fail;
    }

    memcpy(&sei->payload[sei->payload_count], payload, sizeof(*payload));
    ++sei->payload_count;

    return 0;
fail:
    cbs_h264_free_sei_payload(payload);
    return err;
}

void ff_cbs_h264_delete_sei_message(CodedBitstreamFragment *au,
                                    CodedBitstreamUnit *nal,
                                    int position)
{
    H264RawSEI *sei = nal->content;

    av_assert0(nal->type == H264_NAL_SEI);
    av_assert0(position >= 0 && position < sei->payload_count);

    if (position == 0 && sei->payload_count == 1) {
        // Deleting NAL unit entirely.
        int i;

        for (i = 0; i < au->nb_units; i++) {
            if (&au->units[i] == nal)
                break;
        }

        ff_cbs_delete_unit(au, i);
    } else {
        cbs_h264_free_sei_payload(&sei->payload[position]);

        --sei->payload_count;
        memmove(sei->payload + position,
                sei->payload + position + 1,
                (sei->payload_count - position) * sizeof(*sei->payload));
    }
}
