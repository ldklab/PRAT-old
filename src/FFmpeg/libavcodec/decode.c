/*
 * generic decoding-related code
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

#include <stdint.h>
#include <string.h>

#include "config.h"

#if CONFIG_ICONV
# include <iconv.h>
#endif

#include "libavutil/avassert.h"
#include "libavutil/avstring.h"
#include "libavutil/bprint.h"
#include "libavutil/common.h"
#include "libavutil/frame.h"
#include "libavutil/hwcontext.h"
#include "libavutil/imgutils.h"
#include "libavutil/internal.h"
#include "libavutil/intmath.h"
#include "libavutil/opt.h"

#include "avcodec.h"
#include "bytestream.h"
#include "decode.h"
#include "hwconfig.h"
#include "internal.h"
#include "thread.h"

typedef struct FramePool {
    /**
     * Pools for each data plane. For audio all the planes have the same size,
     * so only pools[0] is used.
     */
    AVBufferPool *pools[4];

    /*
     * Pool parameters
     */
    int format;
    int width, height;
    int stride_align[AV_NUM_DATA_POINTERS];
    int linesize[4];
    int planes;
    int channels;
    int samples;
} FramePool;

{

        return 0;

        av_log(avctx, AV_LOG_ERROR, "This decoder does not support parameter "
               "changes, but PARAM_CHANGE side data was sent to it.\n");
        ret = AVERROR(EINVAL);
        goto fail2;
    }

        goto fail;


        if (size < 4)
            goto fail;
        val = bytestream_get_le32(&data);
        if (val <= 0 || val > INT_MAX) {
            av_log(avctx, AV_LOG_ERROR, "Invalid channel count");
            ret = AVERROR_INVALIDDATA;
            goto fail2;
        }
        avctx->channels = val;
        size -= 4;
    }
        if (size < 8)
            goto fail;
        avctx->channel_layout = bytestream_get_le64(&data);
        size -= 8;
    }
        if (size < 4)
            goto fail;
        val = bytestream_get_le32(&data);
        if (val <= 0 || val > INT_MAX) {
            av_log(avctx, AV_LOG_ERROR, "Invalid sample rate");
            ret = AVERROR_INVALIDDATA;
            goto fail2;
        }
        avctx->sample_rate = val;
        size -= 4;
    }
            goto fail;
            goto fail2;
    }

    return 0;
fail:
    av_log(avctx, AV_LOG_ERROR, "PARAM_CHANGE side data too small.\n");
    ret = AVERROR_INVALIDDATA;
fail2:
    if (ret < 0) {
        av_log(avctx, AV_LOG_ERROR, "Error applying parameter changes.\n");
        if (avctx->err_recognition & AV_EF_EXPLODE)
            return ret;
    }
    return 0;
}

static int extract_packet_props(AVCodecInternal *avci, const AVPacket *pkt)
{
    int ret = 0;

    av_packet_unref(avci->last_pkt_props);
    if (pkt) {
        ret = av_packet_copy_props(avci->last_pkt_props, pkt);
        if (!ret)
            avci->last_pkt_props->size = pkt->size; // HACK: Needed for ff_decode_frame_props().
    }
    return ret;
}

static int unrefcount_frame(AVCodecInternal *avci, AVFrame *frame)
{
    int ret;

    /* move the original frame to our backup */
    av_frame_unref(avci->to_free);
    av_frame_move_ref(avci->to_free, frame);

    /* now copy everything except the AVBufferRefs back
     * note that we make a COPY of the side data, so calling av_frame_free() on
     * the caller's frame will work properly */
    ret = av_frame_copy_props(frame, avci->to_free);
    if (ret < 0)
        return ret;

    memcpy(frame->data,     avci->to_free->data,     sizeof(frame->data));
    memcpy(frame->linesize, avci->to_free->linesize, sizeof(frame->linesize));
    if (avci->to_free->extended_data != avci->to_free->data) {
        int planes = avci->to_free->channels;
        int size   = planes * sizeof(*frame->extended_data);

        if (!size) {
            av_frame_unref(frame);
            return AVERROR_BUG;
        }

        frame->extended_data = av_malloc(size);
        if (!frame->extended_data) {
            av_frame_unref(frame);
            return AVERROR(ENOMEM);
        }
        memcpy(frame->extended_data, avci->to_free->extended_data,
               size);
    } else
        frame->extended_data = frame->data;

    frame->format         = avci->to_free->format;
    frame->width          = avci->to_free->width;
    frame->height         = avci->to_free->height;
    frame->channel_layout = avci->to_free->channel_layout;
    frame->nb_samples     = avci->to_free->nb_samples;
    frame->channels       = avci->to_free->channels;

    return 0;
}

{

        return 0;

        av_log(avctx, AV_LOG_ERROR, "Error parsing decoder bitstream filters '%s': %s\n", avctx->codec->bsfs, av_err2str(ret));
        if (ret != AVERROR(ENOMEM))
            ret = AVERROR_BUG;
        goto fail;
    }

    /* We do not currently have an API for passing the input timebase into decoders,
     * but no filters used here should actually need it.
     * So we make up some plausible-looking number (the MPEG 90kHz timebase) */
        goto fail;

        goto fail;

    return 0;
fail:
    av_bsf_free(&avci->bsf);
    return ret;
}

{

        return AVERROR_EOF;

        return ret;

        goto finish;

        goto finish;


    return 0;
finish:
    av_packet_unref(pkt);
    return ret;
}

/**
 * Attempt to guess proper monotonic timestamps for decoded video frames
 * which might have incorrect times. Input timestamps may wrap around, in
 * which case the output will as well.
 *
 * @param pts the pts field of the decoded AVPacket, as passed through
 * AVFrame.pts
 * @param dts the dts field of the decoded AVPacket
 * @return one of the input values, may be AV_NOPTS_VALUE
 */
                                 int64_t reordered_pts, int64_t dts)
{



        pts = reordered_pts;
    else

}

/*
 * The core of the receive_frame_wrapper for the decoders implementing
 * the simple API. Certain decoders might consume partial packets without
 * returning any output, so this function needs to be called in a loop until it
 * returns EAGAIN.
 **/
{
    // copy to ensure we do not change pkt

            return ret;
    }

    // Some codecs (at least wma lossless) will crash when feeding drain packets
    // after EOF was signaled.
        return AVERROR_EOF;

        return AVERROR_EOF;


    } else {

            //FIXME these should be under if(!avctx->has_b_frames)
            /* get_buffer is supposed to set frame parameters */
            }
        }
    }

                                                             frame->pts,
                                                             frame->pkt_dts);

                                                             frame->pts,
                                                             frame->pkt_dts);
                frame->format = avctx->sample_fmt;
                frame->channels = avctx->channels;
        }

                   avci->skip_samples, (int)discard_padding);
        }

        }

                       avci->skip_samples);
            } else {
                                                   avctx->pkt_timebase);
#if FF_API_PKT_PTS
FF_DISABLE_DEPRECATION_WARNINGS
FF_ENABLE_DEPRECATION_WARNINGS
#endif
                } else {
                    av_log(avctx, AV_LOG_WARNING, "Could not update timestamps for skipped samples.\n");
                }
                       avci->skip_samples, frame->nb_samples);
            }
        }

            } else {
                                                   avctx->pkt_timebase);
                } else {
                    av_log(avctx, AV_LOG_WARNING, "Could not update timestamps for discarded samples.\n");
                }
                       (int)discard_padding, frame->nb_samples);
            }
        }

            AVFrameSideData *fside = av_frame_new_side_data(frame, AV_FRAME_DATA_SKIP_SAMPLES, 10);
            if (fside) {
                AV_WL32(fside->data, avci->skip_samples);
                AV_WL32(fside->data + 4, discard_padding);
                AV_WL8(fside->data + 8, skip_reason);
                AV_WL8(fside->data + 9, discard_reason);
                avci->skip_samples = 0;
            }
        }
    }

    }



#if FF_API_AVCTX_TIMEBASE
#endif

    /* do not stop draining when actual_got_frame != 0 or ret < 0 */
    /* got_frame == 0 but actual_got_frame != 0 when frame is discarded */
            /* prevent infinite loop if a decoder wrongly always return error on draining */
            /* reasonable nb_errors_max = maximum b frames + thread count */
                                avctx->thread_count : 1);

                av_log(avctx, AV_LOG_ERROR, "Too many errors when draining, this is a bug. "
                       "Stop draining and force EOF.\n");
                avci->draining_done = 1;
                ret = AVERROR_BUG;
            }
        } else {
        }
    }


    } else {

    }


}

static int decode_simple_receive_frame(AVCodecContext *avctx, AVFrame *frame)
{
    int ret;

            return ret;
    }

    return 0;
}

{


    else


        /* the only case where decode data is not set should be decoders
         * that do not call ff_get_buffer() */
                   !(avctx->codec->capabilities & AV_CODEC_CAP_DR1));


                ret = fdd->post_process(avctx, frame);
                if (ret < 0) {
                    av_frame_unref(frame);
                    return ret;
                }
            }
        }
    }

    /* free the per-frame decode data */

}

{

        return AVERROR(EINVAL);

        return AVERROR_EOF;

        return AVERROR(EINVAL);

            return ret;
    }

        av_packet_unref(avci->buffer_pkt);
        return ret;
    }

    }

    return 0;
}

{
    /* make sure we are noisy about decoders returning invalid cropping data */
        av_log(avctx, AV_LOG_WARNING,
               "Invalid cropping information set by a decoder: "
               "%"SIZE_SPECIFIER"/%"SIZE_SPECIFIER"/%"SIZE_SPECIFIER"/%"SIZE_SPECIFIER" "
               "(frame size %dx%d). This is a bug, please report it\n",
               frame->crop_left, frame->crop_right, frame->crop_top, frame->crop_bottom,
               frame->width, frame->height);
        frame->crop_left   = 0;
        frame->crop_right  = 0;
        frame->crop_top    = 0;
        frame->crop_bottom = 0;
        return 0;
    }

        return 0;

                                          AV_FRAME_CROP_UNALIGNED : 0);
}

{


        return AVERROR(EINVAL);

    } else {
            return ret;
    }

            av_frame_unref(frame);
            return ret;
        }
    }



        if (avctx->frame_number == 1) {
            avci->initial_format = frame->format;
            switch(avctx->codec_type) {
            case AVMEDIA_TYPE_VIDEO:
                avci->initial_width  = frame->width;
                avci->initial_height = frame->height;
                break;
            case AVMEDIA_TYPE_AUDIO:
                avci->initial_sample_rate = frame->sample_rate ? frame->sample_rate :
                                                                 avctx->sample_rate;
                avci->initial_channels       = frame->channels;
                avci->initial_channel_layout = frame->channel_layout;
                break;
            }
        }

        if (avctx->frame_number > 1) {
            changed = avci->initial_format != frame->format;

            switch(avctx->codec_type) {
            case AVMEDIA_TYPE_VIDEO:
                changed |= avci->initial_width  != frame->width ||
                           avci->initial_height != frame->height;
                break;
            case AVMEDIA_TYPE_AUDIO:
                changed |= avci->initial_sample_rate    != frame->sample_rate ||
                           avci->initial_sample_rate    != avctx->sample_rate ||
                           avci->initial_channels       != frame->channels ||
                           avci->initial_channel_layout != frame->channel_layout;
                break;
            }

            if (changed) {
                avci->changed_frames_dropped++;
                av_log(avctx, AV_LOG_INFO, "dropped changed frame #%d pts %"PRId64
                                            " drop count: %d \n",
                                            avctx->frame_number, frame->pts,
                                            avci->changed_frames_dropped);
                av_frame_unref(frame);
                return AVERROR_INPUT_CHANGED;
            }
        }
    }
    return 0;
}

                         int *got_frame, const AVPacket *pkt)
{


        av_log(avctx, AV_LOG_WARNING, "Got unexpected packet after EOF\n");
        avcodec_flush_buffers(avctx);
    }


        avci->compat_decode_partial_size != pkt->size) {
        av_log(avctx, AV_LOG_ERROR,
               "Got unexpected packet size after a partial decode\n");
        ret = AVERROR(EINVAL);
        goto finish;
    }

            ret = 0;
            /* we fully drain all the output in each decode call, so this should not
             * ever happen */
            ret = AVERROR_BUG;
            goto finish;
            goto finish;
    }

            goto finish;
        }

                    goto finish;
            }

        } else {
            if (!avci->compat_decode_warned) {
                av_log(avctx, AV_LOG_WARNING, "The deprecated avcodec_decode_* "
                       "API cannot return all the frames for this decoder. "
                       "Some frames will be dropped. Update your code to the "
                       "new decoding API to fix this.\n");
                avci->compat_decode_warned = 1;
            }
        }

            break;
    }

        /* if there are any bsfs then assume full packet is always consumed */
            ret = pkt->size;
        else
    }

}

                                              int *got_picture_ptr,
                                              const AVPacket *avpkt)
{
}

                                              AVFrame *frame,
                                              int *got_frame_ptr,
                                              const AVPacket *avpkt)
{
}

{
}

#define UTF8_MAX_BYTES 4 /* 5 and 6 bytes sequences should not be used */
static int recode_subtitle(AVCodecContext *avctx,
                           AVPacket *outpkt, const AVPacket *inpkt)
{
#if CONFIG_ICONV
    iconv_t cd = (iconv_t)-1;
    int ret = 0;
    char *inb, *outb;
    size_t inl, outl;
    AVPacket tmp;
#endif

    if (avctx->sub_charenc_mode != FF_SUB_CHARENC_MODE_PRE_DECODER || inpkt->size == 0)
        return 0;

#if CONFIG_ICONV
    cd = iconv_open("UTF-8", avctx->sub_charenc);
    av_assert0(cd != (iconv_t)-1);

    inb = inpkt->data;
    inl = inpkt->size;

    if (inl >= INT_MAX / UTF8_MAX_BYTES - AV_INPUT_BUFFER_PADDING_SIZE) {
        av_log(avctx, AV_LOG_ERROR, "Subtitles packet is too big for recoding\n");
        ret = AVERROR(ENOMEM);
        goto end;
    }

    ret = av_new_packet(&tmp, inl * UTF8_MAX_BYTES);
    if (ret < 0)
        goto end;
    outpkt->buf  = tmp.buf;
    outpkt->data = tmp.data;
    outpkt->size = tmp.size;
    outb = outpkt->data;
    outl = outpkt->size;

    if (iconv(cd, &inb, &inl, &outb, &outl) == (size_t)-1 ||
        iconv(cd, NULL, NULL, &outb, &outl) == (size_t)-1 ||
        outl >= outpkt->size || inl != 0) {
        ret = FFMIN(AVERROR(errno), -1);
        av_log(avctx, AV_LOG_ERROR, "Unable to recode subtitle event \"%s\" "
               "from %s to UTF-8\n", inpkt->data, avctx->sub_charenc);
        av_packet_unref(&tmp);
        goto end;
    }
    outpkt->size -= outl;
    memset(outpkt->data + outpkt->size, 0, outl);

end:
    if (cd != (iconv_t)-1)
        iconv_close(cd);
    return ret;
#else
    av_log(avctx, AV_LOG_ERROR, "requesting subtitles recoding without iconv");
    return AVERROR(EINVAL);
#endif
}

{

            return 0;
        str = byte;
    }
    return 1;
}

#if FF_API_ASS_TIMING
static void insert_ts(AVBPrint *buf, int ts)
{
    if (ts == -1) {
        av_bprintf(buf, "9:59:59.99,");
    } else {
        int h, m, s;

        h = ts/360000;  ts -= 360000*h;
        m = ts/  6000;  ts -=   6000*m;
        s = ts/   100;  ts -=    100*s;
        av_bprintf(buf, "%d:%02d:%02d.%02d,", h, m, s, ts);
    }
}

static int convert_sub_to_old_ass_form(AVSubtitle *sub, const AVPacket *pkt, AVRational tb)
{
    int i;
    AVBPrint buf;

    av_bprint_init(&buf, 0, AV_BPRINT_SIZE_UNLIMITED);

    for (i = 0; i < sub->num_rects; i++) {
        char *final_dialog;
        const char *dialog;
        AVSubtitleRect *rect = sub->rects[i];
        int ts_start, ts_duration = -1;
        long int layer;

        if (rect->type != SUBTITLE_ASS || !strncmp(rect->ass, "Dialogue: ", 10))
            continue;

        av_bprint_clear(&buf);

        /* skip ReadOrder */
        dialog = strchr(rect->ass, ',');
        if (!dialog)
            continue;
        dialog++;

        /* extract Layer or Marked */
        layer = strtol(dialog, (char**)&dialog, 10);
        if (*dialog != ',')
            continue;
        dialog++;

        /* rescale timing to ASS time base (ms) */
        ts_start = av_rescale_q(pkt->pts, tb, av_make_q(1, 100));
        if (pkt->duration != -1)
            ts_duration = av_rescale_q(pkt->duration, tb, av_make_q(1, 100));
        sub->end_display_time = FFMAX(sub->end_display_time, 10 * ts_duration);

        /* construct ASS (standalone file form with timestamps) string */
        av_bprintf(&buf, "Dialogue: %ld,", layer);
        insert_ts(&buf, ts_start);
        insert_ts(&buf, ts_duration == -1 ? -1 : ts_start + ts_duration);
        av_bprintf(&buf, "%s\r\n", dialog);

        final_dialog = av_strdup(buf.str);
        if (!av_bprint_is_complete(&buf) || !final_dialog) {
            av_freep(&final_dialog);
            av_bprint_finalize(&buf, NULL);
            return AVERROR(ENOMEM);
        }
        av_freep(&rect->ass);
        rect->ass = final_dialog;
    }

    av_bprint_finalize(&buf, NULL);
    return 0;
}
#endif

                             int *got_sub_ptr,
                             AVPacket *avpkt)
{

        av_log(avctx, AV_LOG_ERROR, "invalid packet: NULL data, size != 0\n");
        return AVERROR(EINVAL);
    }
        return AVERROR(EINVAL);
        av_log(avctx, AV_LOG_ERROR, "Invalid media type for subtitles\n");
        return AVERROR(EINVAL);
    }



            *got_sub_ptr = 0;
        } else {
                return ret;

                       !!*got_sub_ptr >= !!sub->num_rects);

#if FF_API_ASS_TIMING
                const AVRational tb = avctx->pkt_timebase.num ? avctx->pkt_timebase
                                                              : avctx->time_base;
                int err = convert_sub_to_old_ass_form(sub, avpkt, tb);
                if (err < 0)
                    ret = err;
            }
#endif

                                                     avctx->pkt_timebase, ms);
            }


                    av_log(avctx, AV_LOG_ERROR,
                           "Invalid UTF-8 in decoded subtitles text; "
                           "maybe missing -sub_charenc option\n");
                    avsubtitle_free(sub);
                    ret = AVERROR_INVALIDDATA;
                    break;
                }
            }

                /* prevent from destroying side data from original packet */

            }
        }

    }

    return ret;
}

                                              const enum AVPixelFormat *fmt)
{

    // If a device was supplied when the codec was opened, assume that the
    // user wants to use it.
        AVHWDeviceContext *device_ctx =
            (AVHWDeviceContext*)avctx->hw_device_ctx->data;
        for (i = 0;; i++) {
            config = &avctx->codec->hw_configs[i]->public;
            if (!config)
                break;
            if (!(config->methods &
                  AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX))
                continue;
            if (device_ctx->type != config->device_type)
                continue;
            for (n = 0; fmt[n] != AV_PIX_FMT_NONE; n++) {
                if (config->pix_fmt == fmt[n])
                    return fmt[n];
            }
        }
    }
    // No device or other setup, so we have to choose from things which
    // don't any other external information.

    // If the last element of the list is a software format, choose it
    // (this should be best software format if any exist).

    // Finally, traverse the list in order and choose the first entry
    // with no external dependencies (if there is no hardware configuration
    // information available then this just picks the first entry).
    for (n = 0; fmt[n] != AV_PIX_FMT_NONE; n++) {
        for (i = 0;; i++) {
            config = avcodec_get_hw_config(avctx->codec, i);
            if (!config)
                break;
            if (config->pix_fmt == fmt[n])
                break;
        }
        if (!config) {
            // No specific config available, so the decoder must be able
            // to handle this format without any additional setup.
            return fmt[n];
        }
        if (config->methods & AV_CODEC_HW_CONFIG_METHOD_INTERNAL) {
            // Usable with only internal setup.
            return fmt[n];
        }
    }

    // Nothing is usable, give up.
    return AV_PIX_FMT_NONE;
}

int ff_decode_get_hw_frames_ctx(AVCodecContext *avctx,
                                enum AVHWDeviceType dev_type)
{
    AVHWDeviceContext *device_ctx;
    AVHWFramesContext *frames_ctx;
    int ret;

    if (!avctx->hwaccel)
        return AVERROR(ENOSYS);

    if (avctx->hw_frames_ctx)
        return 0;
    if (!avctx->hw_device_ctx) {
        av_log(avctx, AV_LOG_ERROR, "A hardware frames or device context is "
                "required for hardware accelerated decoding.\n");
        return AVERROR(EINVAL);
    }

    device_ctx = (AVHWDeviceContext *)avctx->hw_device_ctx->data;
    if (device_ctx->type != dev_type) {
        av_log(avctx, AV_LOG_ERROR, "Device type %s expected for hardware "
               "decoding, but got %s.\n", av_hwdevice_get_type_name(dev_type),
               av_hwdevice_get_type_name(device_ctx->type));
        return AVERROR(EINVAL);
    }

    ret = avcodec_get_hw_frames_parameters(avctx,
                                           avctx->hw_device_ctx,
                                           avctx->hwaccel->pix_fmt,
                                           &avctx->hw_frames_ctx);
    if (ret < 0)
        return ret;

    frames_ctx = (AVHWFramesContext*)avctx->hw_frames_ctx->data;


    if (frames_ctx->initial_pool_size) {
        // We guarantee 4 base work surfaces. The function above guarantees 1
        // (the absolute minimum), so add the missing count.
        frames_ctx->initial_pool_size += 3;
    }

    ret = av_hwframe_ctx_init(avctx->hw_frames_ctx);
    if (ret < 0) {
        av_buffer_unref(&avctx->hw_frames_ctx);
        return ret;
    }

    return 0;
}

int avcodec_get_hw_frames_parameters(AVCodecContext *avctx,
                                     AVBufferRef *device_ref,
                                     enum AVPixelFormat hw_pix_fmt,
                                     AVBufferRef **out_frames_ref)
{
    AVBufferRef *frames_ref = NULL;
    const AVCodecHWConfigInternal *hw_config;
    const AVHWAccel *hwa;
    int i, ret;

    for (i = 0;; i++) {
        hw_config = avctx->codec->hw_configs[i];
        if (!hw_config)
            return AVERROR(ENOENT);
        if (hw_config->public.pix_fmt == hw_pix_fmt)
            break;
    }

    hwa = hw_config->hwaccel;
    if (!hwa || !hwa->frame_params)
        return AVERROR(ENOENT);

    frames_ref = av_hwframe_ctx_alloc(device_ref);
    if (!frames_ref)
        return AVERROR(ENOMEM);

    ret = hwa->frame_params(avctx, frames_ref);
    if (ret >= 0) {
        AVHWFramesContext *frames_ctx = (AVHWFramesContext*)frames_ref->data;

        if (frames_ctx->initial_pool_size) {
            // If the user has requested that extra output surfaces be
            // available then add them here.
            if (avctx->extra_hw_frames > 0)
                frames_ctx->initial_pool_size += avctx->extra_hw_frames;

            // If frame threading is enabled then an extra surface per thread
            // is also required.
            if (avctx->active_thread_type & FF_THREAD_FRAME)
                frames_ctx->initial_pool_size += avctx->thread_count;
        }

        *out_frames_ref = frames_ref;
    } else {
        av_buffer_unref(&frames_ref);
    }
    return ret;
}

static int hwaccel_init(AVCodecContext *avctx,
                        const AVCodecHWConfigInternal *hw_config)
{
    const AVHWAccel *hwaccel;
    int err;

    hwaccel = hw_config->hwaccel;
    if (hwaccel->capabilities & AV_HWACCEL_CODEC_CAP_EXPERIMENTAL &&
        avctx->strict_std_compliance > FF_COMPLIANCE_EXPERIMENTAL) {
        av_log(avctx, AV_LOG_WARNING, "Ignoring experimental hwaccel: %s\n",
               hwaccel->name);
        return AVERROR_PATCHWELCOME;
    }

    if (hwaccel->priv_data_size) {
        avctx->internal->hwaccel_priv_data =
            av_mallocz(hwaccel->priv_data_size);
        if (!avctx->internal->hwaccel_priv_data)
            return AVERROR(ENOMEM);
    }

    avctx->hwaccel = hwaccel;
    if (hwaccel->init) {
        err = hwaccel->init(avctx);
        if (err < 0) {
            av_log(avctx, AV_LOG_ERROR, "Failed setup for format %s: "
                   "hwaccel initialisation returned error.\n",
                   av_get_pix_fmt_name(hw_config->public.pix_fmt));
            av_freep(&avctx->internal->hwaccel_priv_data);
            avctx->hwaccel = NULL;
            return err;
        }
    }

    return 0;
}

{
        avctx->hwaccel->uninit(avctx);




{

    // Find end of list.
    // Must contain at least one entry.
    // If a software format is available, it must be the last entry.
        // No software format is available.
    } else {
    }

        return AV_PIX_FMT_NONE;


        // Remove the previous hwaccel, if there was one.

            // Explicitly chose nothing, give up.
            ret = AV_PIX_FMT_NONE;
            break;
        }

            av_log(avctx, AV_LOG_ERROR, "Invalid format returned by "
                   "get_format() callback.\n");
            ret = AV_PIX_FMT_NONE;
            break;
        }
               desc->name);

                break;
        }
            av_log(avctx, AV_LOG_ERROR, "Invalid return from get_format(): "
                   "%s not in possible list.\n", desc->name);
            ret = AV_PIX_FMT_NONE;
            break;
        }

            for (i = 0;; i++) {
                    break;
                if (hw_config->public.pix_fmt == user_choice)
                    break;
            }
        } else {
            hw_config = NULL;
        }

            // No config available, so no extra setup required.
            ret = user_choice;
            break;
        }
        config = &hw_config->public;

        if (config->methods &
            AV_CODEC_HW_CONFIG_METHOD_HW_FRAMES_CTX &&
            avctx->hw_frames_ctx) {
            const AVHWFramesContext *frames_ctx =
                (AVHWFramesContext*)avctx->hw_frames_ctx->data;
            if (frames_ctx->format != user_choice) {
                av_log(avctx, AV_LOG_ERROR, "Invalid setup for format %s: "
                       "does not match the format of the provided frames "
                       "context.\n", desc->name);
                goto try_again;
            }
        } else if (config->methods &
                   AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
                   avctx->hw_device_ctx) {
            const AVHWDeviceContext *device_ctx =
                (AVHWDeviceContext*)avctx->hw_device_ctx->data;
            if (device_ctx->type != config->device_type) {
                av_log(avctx, AV_LOG_ERROR, "Invalid setup for format %s: "
                       "does not match the type of the provided device "
                       "context.\n", desc->name);
                goto try_again;
            }
        } else if (config->methods &
                   AV_CODEC_HW_CONFIG_METHOD_INTERNAL) {
            // Internal-only setup, no additional configuration.
        } else if (config->methods &
                   AV_CODEC_HW_CONFIG_METHOD_AD_HOC) {
            // Some ad-hoc configuration we can't see and can't check.
        } else {
            av_log(avctx, AV_LOG_ERROR, "Invalid setup for format %s: "
                   "missing configuration.\n", desc->name);
            goto try_again;
        }
        if (hw_config->hwaccel) {
            av_log(avctx, AV_LOG_DEBUG, "Format %s requires hwaccel "
                   "initialisation.\n", desc->name);
            err = hwaccel_init(avctx, hw_config);
            if (err < 0)
                goto try_again;
        }
        ret = user_choice;
        break;

    try_again:
        av_log(avctx, AV_LOG_DEBUG, "Format %s not usable, retrying "
               "get_format() without it.\n", desc->name);
        for (i = 0; i < n; i++) {
            if (choices[i] == user_choice)
                break;
        }
        for (; i + 1 < n; i++)
            choices[i] = choices[i + 1];
        --n;
    }

}

{



{

        return NULL;

                           frame_pool_free, NULL, 0);
        av_freep(&pool);
        return NULL;
    }

    return buf;
}

{

    }

            return 0;
            return 0;
    }

        return AVERROR(ENOMEM);



            // NOTE: do not align linesizes individually, this breaks e.g. assumptions
            // that linesize[0] == 2*linesize[1] in the MPEG-encoder for 4:2:2
                goto fail;
            // increase alignment of w for next try (rhs gives the lowest bit set in w)


            goto fail;

                    ret = AVERROR(EINVAL);
                    goto fail;
                }
                                                     CONFIG_MEMORY_POISONING ?
                                                        NULL :
                                                        av_buffer_allocz);
                    ret = AVERROR(ENOMEM);
                    goto fail;
                }
            }
        }

        }
            goto fail;

            ret = AVERROR(ENOMEM);
            goto fail;
        }

        }
    default: av_assert0(0);
    }


fail:
    av_buffer_unref(&pool_buf);
    return ret;
}

{


        frame->extended_data = av_mallocz_array(planes, sizeof(*frame->extended_data));
        frame->nb_extended_buf = planes - AV_NUM_DATA_POINTERS;
        frame->extended_buf  = av_mallocz_array(frame->nb_extended_buf,
                                          sizeof(*frame->extended_buf));
        if (!frame->extended_data || !frame->extended_buf) {
            av_freep(&frame->extended_data);
            av_freep(&frame->extended_buf);
            return AVERROR(ENOMEM);
        }
    } else {
    }

            goto fail;
    }
        frame->extended_buf[i] = av_buffer_pool_get(pool->pools[0]);
        if (!frame->extended_buf[i])
            goto fail;
        frame->extended_data[i + AV_NUM_DATA_POINTERS] = frame->extended_buf[i]->data;
    }

        av_log(avctx, AV_LOG_DEBUG, "default_get_buffer called on frame %p", frame);

    return 0;
fail:
    av_frame_unref(frame);
    return AVERROR(ENOMEM);
}

{

        av_log(s, AV_LOG_ERROR, "pic->data[*]!=NULL in avcodec_default_get_buffer\n");
        return -1;
    }

        av_log(s, AV_LOG_ERROR,
            "Unable to get pixel format descriptor for format %s\n",
            av_get_pix_fmt_name(pic->format));
        return AVERROR(EINVAL);
    }



            goto fail;

    }
    }

        av_log(s, AV_LOG_DEBUG, "default_get_buffer called on pic %p\n", pic);

    return 0;
fail:
    av_frame_unref(pic);
    return AVERROR(ENOMEM);
}

{

        ret = av_hwframe_get_buffer(avctx->hw_frames_ctx, frame, 0);
        frame->width  = avctx->coded_width;
        frame->height = avctx->coded_height;
        return ret;
    }

        return ret;

    default:
        return -1;
    }
}

{


                                            AV_PKT_DATA_STRINGS_METADATA, &size);
}

{
        enum AVPacketSideDataType packet;
        enum AVFrameSideDataType frame;
    } sd[] = {
        { AV_PKT_DATA_REPLAYGAIN ,                AV_FRAME_DATA_REPLAYGAIN },
        { AV_PKT_DATA_DISPLAYMATRIX,              AV_FRAME_DATA_DISPLAYMATRIX },
        { AV_PKT_DATA_SPHERICAL,                  AV_FRAME_DATA_SPHERICAL },
        { AV_PKT_DATA_STEREO3D,                   AV_FRAME_DATA_STEREO3D },
        { AV_PKT_DATA_AUDIO_SERVICE_TYPE,         AV_FRAME_DATA_AUDIO_SERVICE_TYPE },
        { AV_PKT_DATA_MASTERING_DISPLAY_METADATA, AV_FRAME_DATA_MASTERING_DISPLAY_METADATA },
        { AV_PKT_DATA_CONTENT_LIGHT_LEVEL,        AV_FRAME_DATA_CONTENT_LIGHT_LEVEL },
        { AV_PKT_DATA_A53_CC,                     AV_FRAME_DATA_A53_CC },
        { AV_PKT_DATA_ICC_PROFILE,                AV_FRAME_DATA_ICC_PROFILE },
        { AV_PKT_DATA_S12M_TIMECODE,              AV_FRAME_DATA_S12M_TIMECODE },
    };

#if FF_API_PKT_PTS
FF_DISABLE_DEPRECATION_WARNINGS
FF_ENABLE_DEPRECATION_WARNINGS
#endif

                                                                   sd[i].frame,
                                                                   size);
                    return AVERROR(ENOMEM);

            }
        }

        } else {
        }
    }



                               frame->sample_aspect_ratio) < 0) {
            av_log(avctx, AV_LOG_WARNING, "ignoring invalid SAR: %u/%u\n",
                   frame->sample_aspect_ratio.num,
                   frame->sample_aspect_ratio.den);
            frame->sample_aspect_ratio = (AVRational){ 0, 1 };
        }

        break;
                     av_log(avctx, AV_LOG_ERROR, "Inconsistent channel "
                            "configuration.\n");
                     return AVERROR(EINVAL);
                 }

            } else {
                    av_log(avctx, AV_LOG_ERROR, "Too many channels: %d.\n",
                           avctx->channels);
                    return AVERROR(ENOSYS);
                }
            }
        }
    }
    return 0;
}

{
        }
        // For formats without data like hwaccel allow unused pointers to be non-NULL.
                av_log(avctx, AV_LOG_ERROR, "Buffer returned by get_buffer2() did not zero unused plane pointers\n");
        }
    }

{

        fdd->post_process_opaque_free(fdd->post_process_opaque);

        fdd->hwaccel_priv_free(fdd->hwaccel_priv);


{


        return AVERROR(ENOMEM);

                               NULL, AV_BUFFER_FLAG_READONLY);
        av_freep(&fdd);
        return AVERROR(ENOMEM);
    }


}

{

            av_log(avctx, AV_LOG_ERROR, "video_get_buffer: image parameters invalid\n");
            ret = AVERROR(EINVAL);
            goto fail;
        }

        }

            av_log(avctx, AV_LOG_ERROR, "pic->data[*]!=NULL in get_buffer_internal\n");
            ret = AVERROR(EINVAL);
            goto fail;
        }
            av_log(avctx, AV_LOG_ERROR, "samples per frame %d, exceeds max_samples %"PRId64"\n", frame->nb_samples, avctx->max_samples);
            ret = AVERROR(EINVAL);
            goto fail;
        }
    }
        goto fail;

        if (hwaccel->alloc_frame) {
            ret = hwaccel->alloc_frame(avctx, frame);
            goto end;
        }
    } else

        goto fail;


        goto fail;

    }

        av_log(avctx, AV_LOG_ERROR, "get_buffer() failed\n");
        av_frame_unref(frame);
    }

}

{


        av_log(avctx, AV_LOG_WARNING, "Picture changed from size:%dx%d fmt:%s to size:%dx%d fmt:%s in reget buffer()\n",
               frame->width, frame->height, av_get_pix_fmt_name(frame->format), avctx->width, avctx->height, av_get_pix_fmt_name(avctx->pix_fmt));
        av_frame_unref(frame);
    }



        return AVERROR(ENOMEM);


        av_frame_free(&tmp);
        return ret;
    }


}

{
        av_log(avctx, AV_LOG_ERROR, "reget_buffer() failed\n");
}
