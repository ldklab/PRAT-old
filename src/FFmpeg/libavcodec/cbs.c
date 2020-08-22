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

#include <string.h>

#include "config.h"

#include "libavutil/avassert.h"
#include "libavutil/buffer.h"
#include "libavutil/common.h"

#include "cbs.h"
#include "cbs_internal.h"


static const CodedBitstreamType *cbs_type_table[] = {
#if CONFIG_CBS_AV1
    &ff_cbs_type_av1,
#endif
#if CONFIG_CBS_H264
    &ff_cbs_type_h264,
#endif
#if CONFIG_CBS_H265
    &ff_cbs_type_h265,
#endif
#if CONFIG_CBS_JPEG
    &ff_cbs_type_jpeg,
#endif
#if CONFIG_CBS_MPEG2
    &ff_cbs_type_mpeg2,
#endif
#if CONFIG_CBS_VP9
    &ff_cbs_type_vp9,
#endif
};

const enum AVCodecID ff_cbs_all_codec_ids[] = {
#if CONFIG_CBS_AV1
    AV_CODEC_ID_AV1,
#endif
#if CONFIG_CBS_H264
    AV_CODEC_ID_H264,
#endif
#if CONFIG_CBS_H265
    AV_CODEC_ID_H265,
#endif
#if CONFIG_CBS_JPEG
    AV_CODEC_ID_MJPEG,
#endif
#if CONFIG_CBS_MPEG2
    AV_CODEC_ID_MPEG2VIDEO,
#endif
#if CONFIG_CBS_VP9
    AV_CODEC_ID_VP9,
#endif
    AV_CODEC_ID_NONE
};

                enum AVCodecID codec_id, void *log_ctx)
{

            type = cbs_type_table[i];
            break;
        }
    }
        return AVERROR(EINVAL);

        return AVERROR(ENOMEM);


            av_freep(&ctx);
            return AVERROR(ENOMEM);
        }
    }



}

{

        return;


}

{

}

{



{


static int cbs_read_fragment_content(CodedBitstreamContext *ctx,
                                     CodedBitstreamFragment *frag)
{
    int err, i, j;

    for (i = 0; i < frag->nb_units; i++) {
        CodedBitstreamUnit *unit = &frag->units[i];

        if (ctx->decompose_unit_types) {
            for (j = 0; j < ctx->nb_decompose_unit_types; j++) {
                if (ctx->decompose_unit_types[j] == unit->type)
                    break;
            }
            if (j >= ctx->nb_decompose_unit_types)
                continue;
        }

        av_buffer_unref(&unit->content_ref);
        unit->content = NULL;

        av_assert0(unit->data && unit->data_ref);

        err = ctx->codec->read_unit(ctx, unit);
        if (err == AVERROR(ENOSYS)) {
            av_log(ctx->log_ctx, AV_LOG_VERBOSE,
                   "Decomposition unimplemented for unit %d "
                   "(type %"PRIu32").\n", i, unit->type);
        } else if (err < 0) {
            av_log(ctx->log_ctx, AV_LOG_ERROR, "Failed to read unit %d "
                   "(type %"PRIu32").\n", i, unit->type);
            return err;
        }
    }

    return 0;
}

                                  const uint8_t *data, size_t size)
{

        return AVERROR(ENOMEM);


           AV_INPUT_BUFFER_PADDING_SIZE);

}

                          CodedBitstreamFragment *frag,
                          const AVCodecParameters *par)
{

        return err;

        return err;

}

                       CodedBitstreamFragment *frag,
                       const AVPacket *pkt)
{

            return AVERROR(ENOMEM);


    } else {
        err = cbs_fill_fragment_data(frag, pkt->data, pkt->size);
        if (err < 0)
            return err;
    }

        return err;

}

                CodedBitstreamFragment *frag,
                const uint8_t *data, size_t size)
{

        return err;

        return err;

}

                               CodedBitstreamUnit *unit)
{

        // Initial write buffer size is 1MB.

            av_log(ctx->log_ctx, AV_LOG_ERROR, "Unable to allocate a "
                   "sufficiently large write buffer (last attempt "
                   "%"SIZE_SPECIFIER" bytes).\n", ctx->write_buffer_size);
            return ret;
        }
    }


        if (ret == AVERROR(ENOSPC)) {
            // Overflow.
            if (ctx->write_buffer_size == INT_MAX / 8)
                return AVERROR(ENOMEM);
            ctx->write_buffer_size = FFMIN(2 * ctx->write_buffer_size, INT_MAX / 8);
            goto reallocate_and_try_again;
        }
        // Write failed for some other reason.
        return ret;
    }

    // Overflow but we didn't notice.

    else


        return ret;


}

                               CodedBitstreamFragment *frag)
{


            continue;


            av_log(ctx->log_ctx, AV_LOG_ERROR, "Failed to write unit %d "
                   "(type %"PRIu32").\n", i, unit->type);
            return err;
        }
    }


        av_log(ctx->log_ctx, AV_LOG_ERROR, "Failed to assemble fragment.\n");
        return err;
    }

    return 0;
}

                           AVCodecParameters *par,
                           CodedBitstreamFragment *frag)
{

        return err;


                               AV_INPUT_BUFFER_PADDING_SIZE);
        return AVERROR(ENOMEM);

           AV_INPUT_BUFFER_PADDING_SIZE);

}

                        AVPacket *pkt,
                        CodedBitstreamFragment *frag)
{

        return err;

        return AVERROR(ENOMEM);



}


                         const char *name)
{
        return;

    av_log(ctx->log_ctx, ctx->trace_level, "%s\n", name);
}

void ff_cbs_trace_syntax_element(CodedBitstreamContext *ctx, int position,
                                 const char *str, const int *subscripts,
                                 const char *bits, int64_t value)
{
    char name[256];
    size_t name_len, bits_len;
    int pad, subs, i, j, k, n;

    if (!ctx->trace_enable)
        return;

    av_assert0(value >= INT_MIN && value <= UINT32_MAX);

    subs = subscripts ? subscripts[0] : 0;
    n = 0;
    for (i = j = 0; str[i];) {
        if (str[i] == '[') {
            if (n < subs) {
                ++n;
                k = snprintf(name + j, sizeof(name) - j, "[%d", subscripts[n]);
                av_assert0(k > 0 && j + k < sizeof(name));
                j += k;
                for (++i; str[i] && str[i] != ']'; i++);
                av_assert0(str[i] == ']');
            } else {
                while (str[i] && str[i] != ']')
                    name[j++] = str[i++];
                av_assert0(str[i] == ']');
            }
        } else {
            av_assert0(j + 1 < sizeof(name));
            name[j++] = str[i++];
        }
    }
    av_assert0(j + 1 < sizeof(name));
    name[j] = 0;
    av_assert0(n == subs);

    name_len = strlen(name);
    bits_len = strlen(bits);

    if (name_len + bits_len > 60)
        pad = bits_len + 2;
    else
        pad = 61 - name_len;

    av_log(ctx->log_ctx, ctx->trace_level, "%-10d  %s%*s = %"PRId64"\n",
           position, name, pad, bits, value);
}

                         int width, const char *name,
                         const int *subscripts, uint32_t *write_to,
                         uint32_t range_min, uint32_t range_max)
{


        av_log(ctx->log_ctx, AV_LOG_ERROR, "Invalid value at "
               "%s: bitstream ended.\n", name);
        return AVERROR_INVALIDDATA;
    }

        position = get_bits_count(gbc);


        char bits[33];
        int i;
        for (i = 0; i < width; i++)
            bits[i] = value >> (width - i - 1) & 1 ? '1' : '0';
        bits[i] = 0;

        ff_cbs_trace_syntax_element(ctx, position, name, subscripts,
                                    bits, value);
    }

        av_log(ctx->log_ctx, AV_LOG_ERROR, "%s out of range: "
               "%"PRIu32", but must be in [%"PRIu32",%"PRIu32"].\n",
               name, value, range_min, range_max);
        return AVERROR_INVALIDDATA;
    }

}

                          int width, const char *name,
                          const int *subscripts, uint32_t value,
                          uint32_t range_min, uint32_t range_max)
{

        av_log(ctx->log_ctx, AV_LOG_ERROR, "%s out of range: "
               "%"PRIu32", but must be in [%"PRIu32",%"PRIu32"].\n",
               name, value, range_min, range_max);
        return AVERROR_INVALIDDATA;
    }

        return AVERROR(ENOSPC);

        char bits[33];
        int i;
        for (i = 0; i < width; i++)
            bits[i] = value >> (width - i - 1) & 1 ? '1' : '0';
        bits[i] = 0;

        ff_cbs_trace_syntax_element(ctx, put_bits_count(pbc),
                                    name, subscripts, bits, value);
    }

    else

    return 0;
}

                       int width, const char *name,
                       const int *subscripts, int32_t *write_to,
                       int32_t range_min, int32_t range_max)
{


        av_log(ctx->log_ctx, AV_LOG_ERROR, "Invalid value at "
               "%s: bitstream ended.\n", name);
        return AVERROR_INVALIDDATA;
    }

        position = get_bits_count(gbc);


        char bits[33];
        int i;
        for (i = 0; i < width; i++)
            bits[i] = value & (1U << (width - i - 1)) ? '1' : '0';
        bits[i] = 0;

        ff_cbs_trace_syntax_element(ctx, position, name, subscripts,
                                    bits, value);
    }

        av_log(ctx->log_ctx, AV_LOG_ERROR, "%s out of range: "
               "%"PRId32", but must be in [%"PRId32",%"PRId32"].\n",
               name, value, range_min, range_max);
        return AVERROR_INVALIDDATA;
    }

}

                        int width, const char *name,
                        const int *subscripts, int32_t value,
                        int32_t range_min, int32_t range_max)
{

        av_log(ctx->log_ctx, AV_LOG_ERROR, "%s out of range: "
               "%"PRId32", but must be in [%"PRId32",%"PRId32"].\n",
               name, value, range_min, range_max);
        return AVERROR_INVALIDDATA;
    }

        return AVERROR(ENOSPC);

        char bits[33];
        int i;
        for (i = 0; i < width; i++)
            bits[i] = value & (1U << (width - i - 1)) ? '1' : '0';
        bits[i] = 0;

        ff_cbs_trace_syntax_element(ctx, put_bits_count(pbc),
                                    name, subscripts, bits, value);
    }

    else
        put_bits32(pbc, value);

    return 0;
}


                              size_t size,
                              void (*free)(void *opaque, uint8_t *data))
{

        return AVERROR(ENOMEM);

                                         free, NULL, 0);
        av_freep(&unit->content);
        return AVERROR(ENOMEM);
    }

    return 0;
}

                           size_t size)
{

        return AVERROR(ENOMEM);



}

                           int position)
{


                    (frag->nb_units - position) * sizeof(*units));
    } else {
            return AVERROR(ENOMEM);



                   (frag->nb_units - position) * sizeof(*units));
    }


    }


}

                               int position,
                               CodedBitstreamUnitType type,
                               void *content,
                               AVBufferRef *content_buf)
{


            return AVERROR(ENOMEM);
    } else {
        content_ref = NULL;
    }

        av_buffer_unref(&content_ref);
        return err;
    }


}

                            int position,
                            CodedBitstreamUnitType type,
                            uint8_t *data, size_t data_size,
                            AVBufferRef *data_buf)
{


    else
        data_ref = av_buffer_create(data, data_size, NULL, NULL, 0);
        if (!data_buf)
            av_free(data);
        return AVERROR(ENOMEM);
    }

        av_buffer_unref(&data_ref);
        return err;
    }


}

void ff_cbs_delete_unit(CodedBitstreamFragment *frag,
                        int position)
{
    av_assert0(0 <= position && position < frag->nb_units
                             && "Unit to be deleted not in fragment.");

    cbs_unit_uninit(&frag->units[position]);

    --frag->nb_units;

    if (frag->nb_units > 0)
        memmove(frag->units + position,
                frag->units + position + 1,
                (frag->nb_units - position) * sizeof(*frag->units));
}
