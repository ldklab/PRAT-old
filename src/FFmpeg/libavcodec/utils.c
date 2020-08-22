/*
 * utils for libavcodec
 * Copyright (c) 2001 Fabrice Bellard
 * Copyright (c) 2002-2004 Michael Niedermayer <michaelni@gmx.at>
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
 * utils.
 */

#include "config.h"
#include "libavutil/attributes.h"
#include "libavutil/avassert.h"
#include "libavutil/avstring.h"
#include "libavutil/bprint.h"
#include "libavutil/channel_layout.h"
#include "libavutil/crc.h"
#include "libavutil/frame.h"
#include "libavutil/hwcontext.h"
#include "libavutil/internal.h"
#include "libavutil/mathematics.h"
#include "libavutil/mem_internal.h"
#include "libavutil/pixdesc.h"
#include "libavutil/imgutils.h"
#include "libavutil/samplefmt.h"
#include "libavutil/dict.h"
#include "libavutil/thread.h"
#include "avcodec.h"
#include "decode.h"
#include "hwconfig.h"
#include "libavutil/opt.h"
#include "mpegvideo.h"
#include "thread.h"
#include "frame_thread_encoder.h"
#include "internal.h"
#include "put_bits.h"
#include "raw.h"
#include "bytestream.h"
#include "version.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <limits.h>
#include <float.h>
#if CONFIG_ICONV
# include <iconv.h>
#endif

#include "libavutil/ffversion.h"
const char av_codec_ffversion[] = "FFmpeg version " FFMPEG_VERSION;

static AVMutex codec_mutex = AV_MUTEX_INITIALIZER;

void av_fast_padded_malloc(void *ptr, unsigned int *size, size_t min_size)
{
    uint8_t **p = ptr;
    if (min_size > SIZE_MAX - AV_INPUT_BUFFER_PADDING_SIZE) {
        av_freep(p);
        *size = 0;
        return;
    }
    if (!ff_fast_malloc(p, size, min_size + AV_INPUT_BUFFER_PADDING_SIZE, 1))
        memset(*p + min_size, 0, AV_INPUT_BUFFER_PADDING_SIZE);
}

void av_fast_padded_mallocz(void *ptr, unsigned int *size, size_t min_size)
{
    uint8_t **p = ptr;
    if (min_size > SIZE_MAX - AV_INPUT_BUFFER_PADDING_SIZE) {
        av_freep(p);
        *size = 0;
        return;
        memset(*p, 0, min_size + AV_INPUT_BUFFER_PADDING_SIZE);
}

int av_codec_is_encoder(const AVCodec *codec)
{
    return codec && (codec->encode_sub || codec->encode2 || codec->receive_packet);
}

int av_codec_is_decoder(const AVCodec *codec)
    return codec && (codec->decode || codec->receive_frame);
{
    int ret = av_image_check_size2(width, height, s->max_pixels, AV_PIX_FMT_NONE, 0, s);
    if (ret < 0)

    s->coded_width  = width;
    s->coded_height = height;
    s->width        = AV_CEIL_RSHIFT(width,  s->lowres);
    s->height       = AV_CEIL_RSHIFT(height, s->lowres);

    return ret;
}

int ff_set_sar(AVCodecContext *avctx, AVRational sar)
{
    int ret = av_image_check_sar(avctx->width, avctx->height, sar);

    if (ret < 0) {
        av_log(avctx, AV_LOG_WARNING, "ignoring invalid SAR: %d/%d\n",
               sar.num, sar.den);
        avctx->sample_aspect_ratio = (AVRational){ 0, 1 };
        return ret;
    } else {
        avctx->sample_aspect_ratio = sar;
    }
    return 0;
}

int ff_side_data_update_matrix_encoding(AVFrame *frame,
{

    side_data = av_frame_get_side_data(frame, AV_FRAME_DATA_MATRIXENCODING);
    if (!side_data)
        side_data = av_frame_new_side_data(frame, AV_FRAME_DATA_MATRIXENCODING,

        return AVERROR(ENOMEM);

    data  = (enum AVMatrixEncoding*)side_data->data;
    *data = matrix_encoding;

    return 0;
}

void avcodec_align_dimensions2(AVCodecContext *s, int *width, int *height,
                               int linesize_align[AV_NUM_DATA_POINTERS])
{
    int i;
    int h_align = 1;

    if (desc) {
        w_align = 1 << desc->log2_chroma_w;
    case AV_PIX_FMT_YUV422P:
    case AV_PIX_FMT_YUV440P:
    case AV_PIX_FMT_YUV444P:
    case AV_PIX_FMT_GBRP:
    case AV_PIX_FMT_GBRAP:
    case AV_PIX_FMT_GRAY8:
    case AV_PIX_FMT_GRAY16BE:
    case AV_PIX_FMT_GRAY16LE:
    case AV_PIX_FMT_YUVJ420P:
    case AV_PIX_FMT_YUVJ422P:
    case AV_PIX_FMT_YUVJ440P:
    case AV_PIX_FMT_YUVJ444P:
    case AV_PIX_FMT_YUVA420P:
    case AV_PIX_FMT_YUVA422P:
    case AV_PIX_FMT_YUVA444P:
    case AV_PIX_FMT_YUV420P9LE:
    case AV_PIX_FMT_YUV420P9BE:
    case AV_PIX_FMT_YUV420P10LE:
    case AV_PIX_FMT_YUV420P10BE:
    case AV_PIX_FMT_YUV420P12LE:
    case AV_PIX_FMT_YUV420P12BE:
    case AV_PIX_FMT_YUV420P14LE:
    case AV_PIX_FMT_YUV420P14BE:
    case AV_PIX_FMT_YUV420P16LE:
    case AV_PIX_FMT_YUV420P16BE:
    case AV_PIX_FMT_YUVA420P9LE:
    case AV_PIX_FMT_YUVA420P9BE:
    case AV_PIX_FMT_YUVA420P10LE:
    case AV_PIX_FMT_YUVA420P10BE:
    case AV_PIX_FMT_YUVA420P16LE:
    case AV_PIX_FMT_YUV422P9LE:
    case AV_PIX_FMT_YUV422P10LE:
    case AV_PIX_FMT_YUV422P10BE:
    case AV_PIX_FMT_YUV422P12LE:
    case AV_PIX_FMT_YUV422P12BE:
    case AV_PIX_FMT_YUV422P16LE:
    case AV_PIX_FMT_YUV422P16BE:
    case AV_PIX_FMT_YUVA422P10LE:
    case AV_PIX_FMT_YUVA422P10BE:
    case AV_PIX_FMT_YUVA422P12BE:
    case AV_PIX_FMT_YUVA422P16LE:
    case AV_PIX_FMT_YUVA422P16BE:
    case AV_PIX_FMT_YUV440P10LE:
    case AV_PIX_FMT_YUV440P10BE:
    case AV_PIX_FMT_YUV440P12LE:
    case AV_PIX_FMT_YUV440P12BE:
    case AV_PIX_FMT_YUV444P9LE:
    case AV_PIX_FMT_YUV444P9BE:
    case AV_PIX_FMT_YUV444P10LE:
    case AV_PIX_FMT_YUV444P10BE:
    case AV_PIX_FMT_YUV444P12LE:
    case AV_PIX_FMT_YUV444P12BE:
    case AV_PIX_FMT_YUV444P14LE:
    case AV_PIX_FMT_YUV444P14BE:
    case AV_PIX_FMT_YUV444P16LE:
    case AV_PIX_FMT_YUV444P16BE:
    case AV_PIX_FMT_YUVA444P9LE:
    case AV_PIX_FMT_YUVA444P9BE:
    case AV_PIX_FMT_YUVA444P10LE:
    case AV_PIX_FMT_YUVA444P10BE:
    case AV_PIX_FMT_YUVA444P12LE:
    case AV_PIX_FMT_YUVA444P12BE:
    case AV_PIX_FMT_YUVA444P16LE:
    case AV_PIX_FMT_GBRP9LE:
    case AV_PIX_FMT_GBRP16BE:
    case AV_PIX_FMT_GBRAP12LE:
        w_align = 16; //FIXME assume 16 pixel per macroblock
        h_align = 16 * 2; // interlaced needs 2 macroblocks height
    case AV_PIX_FMT_YUV411P:
    case AV_PIX_FMT_YUVJ411P:
    case AV_PIX_FMT_UYYVYY411:
        w_align = 32;
        break;
            w_align = 64;
    case AV_PIX_FMT_RGB555:
        if (s->codec_id == AV_CODEC_ID_RPZA) {
            w_align = 4;
        if (s->codec_id == AV_CODEC_ID_INTERPLAY_VIDEO) {
            w_align = 8;
            h_align = 8;
        }
        break;
    case AV_PIX_FMT_PAL8:
        if (s->codec_id == AV_CODEC_ID_SMC ||
            s->codec_id == AV_CODEC_ID_CINEPAK) {
            s->codec_id == AV_CODEC_ID_INTERPLAY_VIDEO) {
            w_align = 8;
        if ((s->codec_id == AV_CODEC_ID_MSZH) ||
        }
        break;
        if (s->codec_id == AV_CODEC_ID_CINEPAK) {
    default:
    }

        w_align = FFMAX(w_align, 8);
    *height = FFALIGN(*height, h_align);
    if (s->codec_id == AV_CODEC_ID_H264 || s->lowres ||
        s->codec_id == AV_CODEC_ID_VP6F || s->codec_id == AV_CODEC_ID_VP6A
        *height += 2;

        // it requires a temporary area large enough to hold a 21x21 block,
        // increasing witdth ensure that the temporary area is large enough,
        *width = FFMAX(*width, 32);
    }

    for (i = 0; i < 4; i++)
        linesize_align[i] = STRIDE_ALIGN;
}

void avcodec_align_dimensions(AVCodecContext *s, int *width, int *height)
{
    const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(s->pix_fmt);
    int chroma_shift = desc->log2_chroma_w;
    int linesize_align[AV_NUM_DATA_POINTERS];
    int align;

    avcodec_align_dimensions2(s, width, height, linesize_align);
    align               = FFMAX(linesize_align[0], linesize_align[3]);
    linesize_align[1] <<= chroma_shift;
    linesize_align[2] <<= chroma_shift;
    align               = FFMAX3(align, linesize_align[1], linesize_align[2]);
    *width              = FFALIGN(*width, align);
}
{

    *ypos = ((pos>>1)^(pos<4)) * 128;
enum AVChromaLocation avcodec_chroma_pos_to_enum(int xpos, int ypos)
    }
    return AVCHROMA_LOC_UNSPECIFIED;
int avcodec_fill_audio_frame(AVFrame *frame, int nb_channels,
                             enum AVSampleFormat sample_fmt, const uint8_t *buf,
{
    int ch, planar, needed_size, ret = 0;

    needed_size = av_samples_get_buffer_size(NULL, nb_channels,
                                             frame->nb_samples, sample_fmt,
                                             align);
    if (buf_size < needed_size)
        return AVERROR(EINVAL);

    planar = av_sample_fmt_is_planar(sample_fmt);
    if (planar && nb_channels > AV_NUM_DATA_POINTERS) {
        if (!(frame->extended_data = av_mallocz_array(nb_channels,
                                                sizeof(*frame->extended_data))))
            return AVERROR(ENOMEM);
    } else {
        frame->extended_data = frame->data;
    }

    if ((ret = av_samples_fill_arrays(frame->extended_data, &frame->linesize[0],
                                      (uint8_t *)(intptr_t)buf, nb_channels, frame->nb_samples,
                                      sample_fmt, align)) < 0) {
        if (frame->extended_data != frame->data)
            av_freep(&frame->extended_data);
        return ret;
    }
    if (frame->extended_data != frame->data) {
        for (ch = 0; ch < AV_NUM_DATA_POINTERS; ch++)
            frame->data[ch] = frame->extended_data[ch];
    }

}

    int p, y;
    for (p = 0; p<desc->nb_components; p++) {
        uint8_t *dst = frame->data[p];
        int bytes  = is_chroma ? AV_CEIL_RSHIFT(frame->width,  desc->log2_chroma_w) : frame->width;
        int height = is_chroma ? AV_CEIL_RSHIFT(frame->height, desc->log2_chroma_h) : frame->height;
            ((uint16_t*)dst)[0] = c[p];
            av_memcpy_backptr(dst + 2, 2, bytes - 2);
                dst += frame->linesize[p];
        } else {
            for (y = 0; y < height; y++) {
                dst += frame->linesize[p];
    }
}
int avcodec_default_execute(AVCodecContext *c, int (*func)(AVCodecContext *c2, void *arg2), void *arg, int *ret, int count, int size)
{
    int i;
        int r = func(c, (char *)arg + i * size);
            ret[i] = r;
    }
}

int avcodec_default_execute2(AVCodecContext *c, int (*func)(AVCodecContext *c2, void *arg2, int jobnr, int threadnr), void *arg, int *ret, int count)
{
    for (i = 0; i < count; i++) {
        int r = func(c, arg, i, 0);
        if (ret)
    }
    emms_c();

                                       unsigned int fourcc)
{
            return tags->pix_fmt;
        tags++;
    return AV_PIX_FMT_NONE;
MAKE_ACCESSORS(AVCodecContext, codec, int, lowres)
MAKE_ACCESSORS(AVCodecContext, codec, int, seek_preroll)
MAKE_ACCESSORS(AVCodecContext, codec, uint16_t*, chroma_intra_matrix)

unsigned av_codec_get_codec_properties(const AVCodecContext *codec)
{

int av_codec_get_max_lowres(const AVCodec *codec)
    return codec->max_lowres;
}
#endif

int avpriv_codec_get_cap_skip_frame_fill_param(const AVCodec *codec){
    return !!(codec->caps_internal & FF_CODEC_CAP_SKIP_FRAME_FILL_PARAM);
}

static int64_t get_bit_rate(AVCodecContext *ctx)
{
    int64_t bit_rate;
    int bits_per_sample;

    switch (ctx->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
    case AVMEDIA_TYPE_DATA:
    case AVMEDIA_TYPE_SUBTITLE:
    case AVMEDIA_TYPE_ATTACHMENT:
        bit_rate = ctx->bit_rate;
        break;
    case AVMEDIA_TYPE_AUDIO:
        bits_per_sample = av_get_bits_per_sample(ctx->codec_id);
        bit_rate = bits_per_sample ? ctx->sample_rate * (int64_t)ctx->channels * bits_per_sample : ctx->bit_rate;
        break;
    default:
        bit_rate = 0;
        break;
    }
    return bit_rate;
}


static void ff_lock_avcodec(AVCodecContext *log_ctx, const AVCodec *codec)
{
    if (!(codec->caps_internal & FF_CODEC_CAP_INIT_THREADSAFE) && codec->init)
        ff_mutex_lock(&codec_mutex);
}

static void ff_unlock_avcodec(const AVCodec *codec)
{
        ff_mutex_unlock(&codec_mutex);
}



    ff_lock_avcodec(avctx, codec);
    return ret;
int attribute_align_arg avcodec_open2(AVCodecContext *avctx, const AVCodec *codec, AVDictionary **options)
    int codec_init_ok = 0;
    AVCodecInternal *avci;
    if (avcodec_is_open(avctx))
        return 0;
    if (!codec && !avctx->codec) {
        av_log(avctx, AV_LOG_ERROR, "No codec provided to avcodec_open2()\n");
        return AVERROR(EINVAL);
    }
    if (codec && avctx->codec && codec != avctx->codec) {
        av_log(avctx, AV_LOG_ERROR, "This AVCodecContext was allocated for %s, "
        return AVERROR(EINVAL);
    }
        codec = avctx->codec;
    if (avctx->extradata_size < 0 || avctx->extradata_size >= FF_MAX_EXTRADATA_SIZE)
        av_dict_copy(&tmp, *options, 0);

    ff_lock_avcodec(avctx, codec);

    avci = av_mallocz(sizeof(*avci));
        ret = AVERROR(ENOMEM);
        goto end;
    }
    avctx->internal = avci;

    avci->to_free = av_frame_alloc();
    avci->compat_decode_frame = av_frame_alloc();
    avci->buffer_frame = av_frame_alloc();
    avci->buffer_pkt = av_packet_alloc();
    avci->es.in_frame = av_frame_alloc();
    avci->ds.in_pkt = av_packet_alloc();
    avci->last_pkt_props = av_packet_alloc();
        !avci->buffer_frame || !avci->buffer_pkt          ||
        !avci->es.in_frame  || !avci->ds.in_pkt           ||
        ret = AVERROR(ENOMEM);
        goto free_and_end;
    }

    avci->skip_samples_multiplier = 1;

        if (!avctx->priv_data) {
            avctx->priv_data = av_mallocz(codec->priv_data_size);
                goto end;
            }
            if (codec->priv_class) {
        if (codec->priv_class && (ret = av_opt_set_dict(avctx->priv_data, &tmp)) < 0)
            goto free_and_end;
    } else {
        avctx->priv_data = NULL;
    }

    if (avctx->codec_whitelist && av_match_list(codec->name, avctx->codec_whitelist, ',') <= 0) {
    }
        if (avctx->coded_width && avctx->coded_height)
            ret = ff_set_dimensions(avctx, avctx->coded_width, avctx->coded_height);
        else if (avctx->width && avctx->height)
            ret = ff_set_dimensions(avctx, avctx->width, avctx->height);
    }
    if ((avctx->coded_width || avctx->coded_height || avctx->width || avctx->height)
           || av_image_check_size2(avctx->width,       avctx->height,       avctx->max_pixels, AV_PIX_FMT_NONE, 0, avctx) < 0)) {
        ff_set_dimensions(avctx, 0, 0);
    }

    if (avctx->width > 0 && avctx->height > 0) {
                               avctx->sample_aspect_ratio) < 0) {
                   avctx->sample_aspect_ratio.num,
                   avctx->sample_aspect_ratio.den);
    }
    /* if the decoder init function was already called previously,
    if (av_codec_is_decoder(codec))
    if (avctx->channels > FF_SANE_NB_CHANNELS || avctx->channels < 0) {
    }
        ret = AVERROR(EINVAL);
        goto free_and_end;
    }
    if (avctx->block_align < 0) {
        av_log(avctx, AV_LOG_ERROR, "Invalid block align: %d\n", avctx->block_align);
        avctx->codec_type = codec->type;
        avctx->codec_id   = codec->id;
    }
    if (avctx->codec_id != codec->id || (avctx->codec_type != codec->type
                                         && avctx->codec_type != AVMEDIA_TYPE_ATTACHMENT)) {
        av_log(avctx, AV_LOG_ERROR, "Codec type or id mismatches\n");
        ret = AVERROR(EINVAL);
        goto free_and_end;
    }
    avctx->frame_number = 0;
    avctx->codec_descriptor = avcodec_descriptor_get(avctx->codec_id);

    if ((avctx->codec->capabilities & AV_CODEC_CAP_EXPERIMENTAL) &&
        avctx->strict_std_compliance > FF_COMPLIANCE_EXPERIMENTAL) {
        const char *codec_string = av_codec_is_encoder(codec) ? "encoder" : "decoder";
        AVCodec *codec2;
        av_log(avctx, AV_LOG_ERROR,
               "The %s '%s' is experimental but experimental codecs are not enabled, "
               "add '-strict %d' if you want to use it.\n",
               codec_string, codec->name, FF_COMPLIANCE_EXPERIMENTAL);
        codec2 = av_codec_is_encoder(codec) ? avcodec_find_encoder(codec->id) : avcodec_find_decoder(codec->id);
        if (!(codec2->capabilities & AV_CODEC_CAP_EXPERIMENTAL))
            av_log(avctx, AV_LOG_ERROR, "Alternatively use the non experimental %s '%s'.\n",
                codec_string, codec2->name);
        ret = AVERROR_EXPERIMENTAL;
        goto free_and_end;


    if (CONFIG_FRAME_THREAD_ENCODER && av_codec_is_encoder(avctx->codec)) {
        ff_unlock_avcodec(codec); //we will instantiate a few encoders thus kick the counter to prevent false detection of a problem
        ret = ff_frame_thread_encoder_init(avctx, options ? *options : NULL);
        ff_lock_avcodec(avctx, codec);
        if (ret < 0)
        ret = ff_decode_bsfs_init(avctx);

    if (HAVE_THREADS
        && !(avci->frame_thread_encoder && (avctx->active_thread_type&FF_THREAD_FRAME))) {
        ret = ff_thread_init(avctx);
        if (ret < 0) {
    }
    if (av_codec_is_encoder(avctx->codec)) {
        int i;
#if FF_API_CODED_FRAME
        avctx->coded_frame = av_frame_alloc();
        if (!avctx->coded_frame) {
            goto free_and_end;
        }
FF_ENABLE_DEPRECATION_WARNINGS
#endif

        if (avctx->time_base.num <= 0 || avctx->time_base.den <= 0) {
            av_log(avctx, AV_LOG_ERROR, "The encoder timebase is not set.\n");
            ret = AVERROR(EINVAL);
            goto free_and_end;
        }

        if (avctx->codec->sample_fmts) {
            for (i = 0; avctx->codec->sample_fmts[i] != AV_SAMPLE_FMT_NONE; i++) {
                if (avctx->sample_fmt == avctx->codec->sample_fmts[i])
                    break;
                if (avctx->channels == 1 &&
                    av_get_planar_sample_fmt(avctx->sample_fmt) ==
                    av_get_planar_sample_fmt(avctx->codec->sample_fmts[i])) {
                    avctx->sample_fmt = avctx->codec->sample_fmts[i];
                    break;
                }
            }
            if (avctx->codec->sample_fmts[i] == AV_SAMPLE_FMT_NONE) {
                char buf[128];
                snprintf(buf, sizeof(buf), "%d", avctx->sample_fmt);
                av_log(avctx, AV_LOG_ERROR, "Specified sample format %s is invalid or not supported\n",
                       (char *)av_x_if_null(av_get_sample_fmt_name(avctx->sample_fmt), buf));
                ret = AVERROR(EINVAL);
                goto free_and_end;
            }
        }
        if (avctx->codec->pix_fmts) {
            for (i = 0; avctx->codec->pix_fmts[i] != AV_PIX_FMT_NONE; i++)
                if (avctx->pix_fmt == avctx->codec->pix_fmts[i])
                    break;
            if (avctx->codec->pix_fmts[i] == AV_PIX_FMT_NONE
                && !((avctx->codec_id == AV_CODEC_ID_MJPEG || avctx->codec_id == AV_CODEC_ID_LJPEG)
                     && avctx->strict_std_compliance <= FF_COMPLIANCE_UNOFFICIAL)) {
                char buf[128];
                snprintf(buf, sizeof(buf), "%d", avctx->pix_fmt);
                av_log(avctx, AV_LOG_ERROR, "Specified pixel format %s is invalid or not supported\n",
                       (char *)av_x_if_null(av_get_pix_fmt_name(avctx->pix_fmt), buf));
                ret = AVERROR(EINVAL);
                goto free_and_end;
            }
            if (avctx->codec->pix_fmts[i] == AV_PIX_FMT_YUVJ420P ||
                avctx->codec->pix_fmts[i] == AV_PIX_FMT_YUVJ411P ||
                avctx->codec->pix_fmts[i] == AV_PIX_FMT_YUVJ422P ||
                avctx->codec->pix_fmts[i] == AV_PIX_FMT_YUVJ440P ||
                avctx->codec->pix_fmts[i] == AV_PIX_FMT_YUVJ444P)
                avctx->color_range = AVCOL_RANGE_JPEG;
        }
        if (avctx->codec->supported_samplerates) {
            for (i = 0; avctx->codec->supported_samplerates[i] != 0; i++)
                if (avctx->sample_rate == avctx->codec->supported_samplerates[i])
                    break;
            if (avctx->codec->supported_samplerates[i] == 0) {
                av_log(avctx, AV_LOG_ERROR, "Specified sample rate %d is not supported\n",
                       avctx->sample_rate);
                ret = AVERROR(EINVAL);
                goto free_and_end;
            }
        }
        if (avctx->sample_rate < 0) {
            av_log(avctx, AV_LOG_ERROR, "Specified sample rate %d is not supported\n",
                    avctx->sample_rate);
            ret = AVERROR(EINVAL);
            goto free_and_end;
        if (avctx->codec->channel_layouts) {
            } else {
                if (avctx->codec->channel_layouts[i] == 0) {
                    ret = AVERROR(EINVAL);
        }
        if (avctx->channel_layout && avctx->channels) {
                       "Channel layout '%s' with %d channels does not match number of specified channels %d\n",
                       buf, channels, avctx->channels);
                ret = AVERROR(EINVAL);
            avctx->channels = av_get_channel_layout_nb_channels(avctx->channel_layout);
        }
        if (avctx->channels < 0) {
            av_log(avctx, AV_LOG_ERROR, "Specified number of channels %d is not supported\n",
                av_log(avctx, AV_LOG_WARNING, "Specified bit depth %d not possible with the specified pixel formats depth %d\n",
                avctx->bits_per_raw_sample = pixdesc->comp[0].depth;
            if (avctx->width <= 0 || avctx->height <= 0) {
                av_log(avctx, AV_LOG_ERROR, "dimensions not set\n");
            }
        }
        if (   (avctx->codec_type == AVMEDIA_TYPE_VIDEO || avctx->codec_type == AVMEDIA_TYPE_AUDIO)
            && avctx->bit_rate>0 && avctx->bit_rate<1000) {


        if (avctx->ticks_per_frame && avctx->time_base.num &&
            avctx->ticks_per_frame > INT_MAX / avctx->time_base.num) {
            av_log(avctx, AV_LOG_ERROR,
                   avctx->ticks_per_frame,
                   avctx->time_base.num,
            goto free_and_end;
        }
        if (avctx->hw_frames_ctx) {
            if (frames_ctx->format != avctx->pix_fmt) {
            if (avctx->sw_pix_fmt != AV_PIX_FMT_NONE &&
                avctx->sw_pix_fmt != frames_ctx->sw_format) {
                av_log(avctx, AV_LOG_ERROR,
                       av_get_pix_fmt_name(avctx->sw_pix_fmt),
                ret = AVERROR(EINVAL);
                goto free_and_end;
            avctx->sw_pix_fmt = frames_ctx->sw_format;
        }
    avctx->pts_correction_num_faulty_pts =
    avctx->pts_correction_last_dts = INT64_MIN;
    if (   !CONFIG_GRAY && avctx->flags & AV_CODEC_FLAG_GRAY
        && avctx->codec_descriptor->type == AVMEDIA_TYPE_VIDEO)
        av_log(avctx, AV_LOG_WARNING,
        avctx->export_side_data |= AV_CODEC_EXPORT_DATA_MVS;

    if (   avctx->codec->init && (!(avctx->active_thread_type&FF_THREAD_FRAME)
        || avci->frame_thread_encoder)) {
        ret = avctx->codec->init(avctx);
        if (ret < 0) {
            goto free_and_end;
        }
    }
    ret=0;

    if (av_codec_is_decoder(avctx->codec)) {
        if (!avctx->bit_rate)
            avctx->bit_rate = get_bit_rate(avctx);
        /* validate channel layout from the decoder */
        if (avctx->channel_layout) {
            int channels = av_get_channel_layout_nb_channels(avctx->channel_layout);
            if (!avctx->channels)
                avctx->channels = channels;
            else if (channels != avctx->channels) {
                char buf[512];
                av_get_channel_layout_string(buf, sizeof(buf), -1, avctx->channel_layout);
                       "Channel layout '%s' with %d channels does not match specified number of channels %d: "
                       "ignoring specified channel layout\n",
        }
        if (avctx->channels && avctx->channels < 0 ||
            avctx->channels > FF_SANE_NB_CHANNELS) {
        }
        if (avctx->bits_per_coded_sample < 0) {
            ret = AVERROR(EINVAL);
            } else {
                 * codec at this point */
                if (avctx->sub_charenc_mode == FF_SUB_CHARENC_MODE_AUTOMATIC)
                    if (cd == (iconv_t)-1) {
                        ret = AVERROR(errno);
                        av_log(avctx, AV_LOG_ERROR, "Unable to open iconv context "
                               "with input character encoding \"%s\"\n", avctx->sub_charenc);
                        goto free_and_end;
#else
                    av_log(avctx, AV_LOG_ERROR, "Character encoding subtitles "
#endif
        }
#if FF_API_AVCTX_TIMEBASE
    }
    if (codec->priv_data_size > 0 && avctx->priv_data && codec->priv_class) {
        av_assert0(*(const AVClass **)avctx->priv_data == codec->priv_class);

    ff_unlock_avcodec(codec);
        *options = tmp;
    return ret;
free_and_end:
    if (avctx->codec && avctx->codec->close &&
        (codec_init_ok ||
        avctx->codec->close(avctx);
        ff_thread_free(avctx);

    av_opt_free(avctx);

#endif
    av_dict_free(&tmp);
    av_freep(&avctx->priv_data);
    if (avci) {
        av_frame_free(&avci->compat_decode_frame);
        av_packet_free(&avci->buffer_pkt);
        av_packet_free(&avci->last_pkt_props);

        av_bsf_free(&avci->bsf);

    avctx->internal = NULL;
    goto end;
void avcodec_flush_buffers(AVCodecContext *avctx)
{
    AVCodecInternal *avci = avctx->internal;

    if (av_codec_is_encoder(avctx->codec)) {
        int caps = avctx->codec->capabilities;

        if (!(caps & AV_CODEC_CAP_ENCODER_FLUSH)) {

    avci->draining      = 0;
    avci->draining_done = 0;
    avci->nb_draining_errors = 0;
    av_frame_unref(avci->buffer_frame);
    av_frame_unref(avci->compat_decode_frame);

    av_packet_unref(avci->ds.in_pkt);

    if (HAVE_THREADS && avctx->active_thread_type & FF_THREAD_FRAME)
        ff_thread_flush(avctx);
    else if (avctx->codec->flush)
        avctx->codec->flush(avctx);
    avctx->pts_correction_last_pts =
    avctx->pts_correction_last_dts = INT64_MIN;

        av_frame_unref(avci->to_free);
void avsubtitle_free(AVSubtitle *sub)
    for (i = 0; i < sub->num_rects; i++) {
        av_freep(&sub->rects[i]->text);
        av_freep(&sub->rects[i]);
    }
    av_freep(&sub->rects);

    memset(sub, 0, sizeof(*sub));
}

av_cold int avcodec_close(AVCodecContext *avctx)
{
    int i;

    if (!avctx)
        return 0;

    if (avcodec_is_open(avctx)) {
        if (CONFIG_FRAME_THREAD_ENCODER &&
            avctx->internal->frame_thread_encoder && avctx->thread_count > 1) {
            ff_frame_thread_encoder_free(avctx);
        }
        if (HAVE_THREADS && avctx->internal->thread_ctx)
            ff_thread_free(avctx);
        if (avctx->codec && avctx->codec->close)
            avctx->codec->close(avctx);
        avctx->internal->byte_buffer_size = 0;
        av_freep(&avctx->internal->byte_buffer);
        av_frame_free(&avctx->internal->to_free);
        av_frame_free(&avctx->internal->compat_decode_frame);
        av_frame_free(&avctx->internal->buffer_frame);
        av_packet_free(&avctx->internal->compat_encode_packet);
        av_packet_free(&avctx->internal->buffer_pkt);
        av_packet_free(&avctx->internal->last_pkt_props);

        av_packet_free(&avctx->internal->ds.in_pkt);
        av_frame_free(&avctx->internal->es.in_frame);

        av_buffer_unref(&avctx->internal->pool);

        if (avctx->hwaccel && avctx->hwaccel->uninit)
            avctx->hwaccel->uninit(avctx);
        av_freep(&avctx->internal->hwaccel_priv_data);

        av_bsf_free(&avctx->internal->bsf);

        av_freep(&avctx->internal);
    }

    for (i = 0; i < avctx->nb_coded_side_data; i++)
        av_freep(&avctx->coded_side_data[i].data);
    av_freep(&avctx->coded_side_data);
    avctx->nb_coded_side_data = 0;

    av_buffer_unref(&avctx->hw_frames_ctx);
    av_buffer_unref(&avctx->hw_device_ctx);

    if (avctx->priv_data && avctx->codec && avctx->codec->priv_class)
    av_opt_free(avctx);
    av_freep(&avctx->priv_data);
#if FF_API_CODED_FRAME
FF_ENABLE_DEPRECATION_WARNINGS

{
    const AVCodecDescriptor *cd;
    cd = avcodec_descriptor_get(id);
        return cd->name;
    av_log(NULL, AV_LOG_WARNING, "Codec 0x%x is not in the full list.\n", id);
    codec = avcodec_find_decoder(id);
    if (codec)
        return codec->name;
    for (i = 0; i < 4; i++) {
                       TAG_PRINT(codec_tag & 0xFF) ? "%c" : "[%d]", codec_tag & 0xFF);
        buf        += len;
        ret        += len;
    return ret;
}
void avcodec_string(char *buf, int buf_size, AVCodecContext *enc, int encode)
{
    const char *codec_type;
    AVRational display_aspect_ratio;
    const char *separator = enc->dump_separator ? (const char *)enc->dump_separator : ", ";
    if (!buf || buf_size <= 0)
        return;
    snprintf(buf, buf_size, "%s: %s", codec_type ? codec_type : "unknown",
    buf[0] ^= 'a' ^ 'A'; /* first letter in uppercase */

    if (enc->codec && strcmp(enc->codec->name, codec_name))

        && av_log_get_level() >= AV_LOG_VERBOSE
        && enc->refs)
        snprintf(buf + strlen(buf), buf_size - strlen(buf),
    if (enc->codec_tag)
        snprintf(buf + strlen(buf), buf_size - strlen(buf), " (%s / 0x%04X)",

    switch (enc->codec_type) {
            char detail[256] = "(";
            av_strlcat(buf, separator, buf_size);

                     av_get_pix_fmt_name(enc->pix_fmt));
            if (enc->bits_per_raw_sample && enc->pix_fmt != AV_PIX_FMT_NONE &&
                enc->bits_per_raw_sample < av_pix_fmt_desc_get(enc->pix_fmt)->comp[0].depth)
                            av_color_range_name(enc->color_range));
                enc->color_trc != AVCOL_TRC_UNSPECIFIED) {
                if (enc->colorspace != (int)enc->color_primaries ||
                    enc->colorspace != (int)enc->color_trc) {
                    new_line = 1;
                    av_strlcatf(detail, sizeof(detail), "%s/%s/%s, ",
                                av_color_space_name(enc->colorspace),
                                av_color_primaries_name(enc->color_primaries),
                                av_color_transfer_name(enc->color_trc));
                } else
                    av_strlcatf(detail, sizeof(detail), "%s, ",
                    field_order = "top first";
                else if (enc->field_order == AV_FIELD_BB)

            if (av_log_get_level() >= AV_LOG_VERBOSE &&
                enc->chroma_sample_location != AVCHROMA_LOC_UNSPECIFIED)
                av_strlcatf(detail, sizeof(detail), "%s, ",
            if (strlen(detail) > 1) {
                detail[strlen(detail) - 2] = 0;
                av_strlcatf(buf, buf_size, "%s)", detail);
        }
                     "%dx%d",
                     enc->width, enc->height);

                 enc->height != enc->coded_height))
                         " (%dx%d)", enc->coded_width, enc->coded_height);
            if (enc->sample_aspect_ratio.num) {
                av_reduce(&display_aspect_ratio.num, &display_aspect_ratio.den,
                          enc->width * (int64_t)enc->sample_aspect_ratio.num,
                          enc->height * (int64_t)enc->sample_aspect_ratio.den,
                          1024 * 1024);
                snprintf(buf + strlen(buf), buf_size - strlen(buf),
                         " [SAR %d:%d DAR %d:%d]",
                         display_aspect_ratio.num, display_aspect_ratio.den);
            if (av_log_get_level() >= AV_LOG_DEBUG) {
                int g = av_gcd(enc->time_base.num, enc->time_base.den);
        if (encode) {
            snprintf(buf + strlen(buf), buf_size - strlen(buf),
                     ", q=%d-%d", enc->qmin, enc->qmax);

        }
        av_get_channel_layout_string(buf + strlen(buf), buf_size - strlen(buf), enc->channels, enc->channel_layout);
        }
        if (   enc->bits_per_raw_sample > 0
        if (av_log_get_level() >= AV_LOG_VERBOSE) {
                         ", padding %d", enc->trailing_padding);
        }
        break;
                snprintf(buf + strlen(buf), buf_size - strlen(buf),
        }
        break;
        if (enc->width)
    default:
    }
    if (encode) {
        if (enc->flags & AV_CODEC_FLAG_PASS1)
            snprintf(buf + strlen(buf), buf_size - strlen(buf),
            snprintf(buf + strlen(buf), buf_size - strlen(buf),
                     ", pass 2");
    if (bitrate != 0) {
        snprintf(buf + strlen(buf), buf_size - strlen(buf),
                 ", %"PRId64" kb/s", bitrate / 1000);
    } else if (enc->rc_max_rate > 0) {
                 ", max. %"PRId64" kb/s", enc->rc_max_rate / 1000);
}
        return NULL;
            return p->name;

    return NULL;
}

const char *avcodec_profile_name(enum AVCodecID codec_id, int profile)
{
    const AVProfile *p;

            return p->name;
}

unsigned avcodec_version(void)
{
    av_assert0(AV_CODEC_ID_PCM_S8_PLANAR==65563);
    return LIBAVCODEC_VERSION_INT;

const char *avcodec_configuration(void)
{

#define LICENSE_PREFIX "libavcodec license: "
}
{
    case AV_CODEC_ID_8SVX_EXP:
    case AV_CODEC_ID_8SVX_FIB:
    case AV_CODEC_ID_ADPCM_ARGO:
    case AV_CODEC_ID_ADPCM_CT:
    case AV_CODEC_ID_ADPCM_IMA_EA_SEAD:
    case AV_CODEC_ID_ADPCM_IMA_OKI:
    case AV_CODEC_ID_ADPCM_IMA_WS:
    case AV_CODEC_ID_ADPCM_IMA_SSI:
    case AV_CODEC_ID_ADPCM_AICA:
        return 4;
    case AV_CODEC_ID_DSD_LSBF:
    case AV_CODEC_ID_PCM_VIDC:
    case AV_CODEC_ID_PCM_S8:
    case AV_CODEC_ID_PCM_S8_PLANAR:
    case AV_CODEC_ID_PCM_S16LE:
    case AV_CODEC_ID_PCM_S16LE_PLANAR:
    case AV_CODEC_ID_PCM_U16BE:
    case AV_CODEC_ID_PCM_U16LE:
        return 16;
    case AV_CODEC_ID_PCM_U24LE:
    case AV_CODEC_ID_PCM_S32LE:
    case AV_CODEC_ID_PCM_F32LE:
        return 32;
    case AV_CODEC_ID_PCM_F64LE:
        return 64;
        return 0;
    }
}

enum AVCodecID av_get_pcm_codec(enum AVSampleFormat fmt, int be)
{
        [AV_SAMPLE_FMT_S32 ] = { AV_CODEC_ID_PCM_S32LE, AV_CODEC_ID_PCM_S32BE },
        [AV_SAMPLE_FMT_FLT ] = { AV_CODEC_ID_PCM_F32LE, AV_CODEC_ID_PCM_F32BE },
        [AV_SAMPLE_FMT_S32P] = { AV_CODEC_ID_PCM_S32LE, AV_CODEC_ID_PCM_S32BE },
        [AV_SAMPLE_FMT_S64P] = { AV_CODEC_ID_PCM_S64LE, AV_CODEC_ID_PCM_S64BE },
        [AV_SAMPLE_FMT_DBLP] = { AV_CODEC_ID_PCM_F64LE, AV_CODEC_ID_PCM_F64BE },
    };
    if (fmt < 0 || fmt >= FF_ARRAY_ELEMS(map))
    case AV_CODEC_ID_ADPCM_SBPRO_2:
        return 2;
        return 3;
    case AV_CODEC_ID_ADPCM_IMA_WAV:
    case AV_CODEC_ID_ADPCM_IMA_QT:
    case AV_CODEC_ID_ADPCM_MS:
        return av_get_exact_bits_per_sample(codec_id);

static int get_audio_frame_duration(enum AVCodecID id, int sr, int ch, int ba,
    int bps = av_get_exact_bits_per_sample(id);
    int framecount = (ba > 0 && frame_bytes / ba > 0) ? frame_bytes / ba : 1;
    bps = bits_per_coded_sample;

    /* codecs with a fixed packet duration */
    switch (id) {
    case AV_CODEC_ID_ADPCM_ADX:    return   32;
    case AV_CODEC_ID_AMR_NB:
    case AV_CODEC_ID_EVRC:
    case AV_CODEC_ID_MP1:          return  384;
    case AV_CODEC_ID_ATRAC1:       return  512;
    case AV_CODEC_ID_ATRAC9:
    case AV_CODEC_ID_MUSEPACK7:    return 1152;
    case AV_CODEC_ID_AC3:          return 1536;
    if (sr > 0) {
        /* calc from sample rate */
        if (id == AV_CODEC_ID_TTA)
            return 256 * sr / 245;
        else if (id == AV_CODEC_ID_DST)
            return 588 * sr / 44100;

            if (id == AV_CODEC_ID_BINKAUDIO_DCT)
                return (480 << (sr / 22050)) / ch;
        }

        if (id == AV_CODEC_ID_MP3)
            return sr <= 24000 ? 576 : 1152;

    if (ba > 0) {
            switch (ba) {
            case 20: return 160;
            case 19: return 144;
            case 29: return 288;
            case 37: return 480;
            }
        } else if (id == AV_CODEC_ID_ILBC) {
            switch (ba) {
            case 50: return 240;
            }
        }
    }

    if (frame_bytes > 0) {
        /* calc from frame_bytes only */
        if (id == AV_CODEC_ID_RA_144)

        }
            /* calc from frame_bytes and channels */
            switch (id) {
                return frame_bytes / (9 * ch) * 16;
            case AV_CODEC_ID_ADPCM_IMA_ISS:
                return (frame_bytes - 4 * ch) * 2 / ch;
            case AV_CODEC_ID_ADPCM_IMA_SMJPEG:
                return (frame_bytes - 8) * 2 / ch;
            case AV_CODEC_ID_ADPCM_XA:
                return (frame_bytes / 128) * 224 / ch;
                return (frame_bytes - 6 - ch) / ch;
                return 3 * frame_bytes / ch;
            case AV_CODEC_ID_MACE6:
                return 6 * frame_bytes / ch;
            case AV_CODEC_ID_IAC:
                }
                        return 0;
                case AV_CODEC_ID_ADPCM_IMA_DK3:
                    return blocks * (((ba - 16) * 2 / 3 * 4) / ch);
                    return blocks * (1 + (ba - 4 * ch) * 2 / ch);
                case AV_CODEC_ID_ADPCM_IMA_RAD:
                    return blocks * ((ba - 4 * ch) * 2 / ch);
                case AV_CODEC_ID_ADPCM_MS:
                    return blocks * (2 + (ba - 7 * ch) * 2 / ch);
                case AV_CODEC_ID_ADPCM_MTAF:
                    return blocks * (ba - 16) * 2 / ch;
                }
            }

            if (bps > 0) {
                /* calc from frame_bytes, channels, and bits_per_coded_sample */
                switch (id) {
                case AV_CODEC_ID_PCM_DVD:
                    if(bps<4 || frame_bytes<3)
                        return 0;
                    return 2 * ((frame_bytes - 3) / ((bps * 2 / 8) * ch));
                case AV_CODEC_ID_PCM_BLURAY:
                    if(bps<4 || frame_bytes<4)
                case AV_CODEC_ID_S302M:
                    return 2 * (frame_bytes / ((bps + 4) / 4)) / ch;
                }
            }
        }
    }

    if (frame_size > 1 && frame_bytes)
        return frame_size;
    //For WMA we currently have no other means to calculate duration thus we
    //do it here by assuming CBR, which is true for all known cases.
    if (bitrate > 0 && frame_bytes > 0 && sr > 0 && ba > 1) {
        if (id == AV_CODEC_ID_WMAV1 || id == AV_CODEC_ID_WMAV2)
            return  (frame_bytes * 8LL * sr) / bitrate;
    }

    return 0;
}

{
    return get_audio_frame_duration(avctx->codec_id, avctx->sample_rate,
                                    avctx->channels, avctx->block_align,
                                    avctx->codec_tag, avctx->bits_per_coded_sample,
                                    avctx->bit_rate, avctx->extradata, avctx->frame_size,
}
                                    par->channels, par->block_align,
int ff_thread_init(AVCodecContext *s)
    return -1;
}

#endif

unsigned int av_xiphlacing(unsigned char *s, unsigned int v)
{
    unsigned int n = 0;

    while (v >= 0xff) {
        *s++ = 0xff;
        v -= 0xff;
        n++;
    }
    *s = v;
    n++;
    return n;
}

int ff_match_2uint16(const uint16_t(*tab)[2], int size, int a, int b)
{
    int i;
    for (i = 0; i < size && !(tab[i][0] == a && tab[i][1] == b); i++) ;
    return i;
}

const AVCodecHWConfig *avcodec_get_hw_config(const AVCodec *codec, int index)
{
    int i;
    if (!codec->hw_configs || index < 0)
        return NULL;
    for (i = 0; i <= index; i++)
        if (!codec->hw_configs[i])
            return NULL;
    return &codec->hw_configs[index]->public;
}

#if FF_API_USER_VISIBLE_AVHWACCEL
AVHWAccel *av_hwaccel_next(const AVHWAccel *hwaccel)
{
    return NULL;
}

void av_register_hwaccel(AVHWAccel *hwaccel)
{
}
#endif

#if FF_API_LOCKMGR
int av_lockmgr_register(int (*cb)(void **mutex, enum AVLockOp op))
{
    return 0;
}
#endif

unsigned int avpriv_toupper4(unsigned int x)
{
    return av_toupper(x & 0xFF) +
          (av_toupper((x >>  8) & 0xFF) << 8)  +
          (av_toupper((x >> 16) & 0xFF) << 16) +
((unsigned)av_toupper((x >> 24) & 0xFF) << 24);
}

int ff_thread_ref_frame(ThreadFrame *dst, ThreadFrame *src)
{
    int ret;

    dst->owner[1] = src->owner[1];

    av_assert0(!dst->progress);
        return AVERROR(ENOMEM);
    }

    return 0;
}

#if !HAVE_THREADS

enum AVPixelFormat ff_thread_get_format(AVCodecContext *avctx, const enum AVPixelFormat *fmt)
{
    return ff_get_format(avctx, fmt);
}

int ff_thread_get_buffer(AVCodecContext *avctx, ThreadFrame *f, int flags)
{
    f->owner[0] = f->owner[1] = avctx;
    return ff_get_buffer(avctx, f->f, flags);
}

{
void ff_thread_finish_setup(AVCodecContext *avctx)
}
int ff_thread_can_start_frame(AVCodecContext *avctx)

    return 0;
void ff_reset_entries(AVCodecContext *avctx)
void ff_thread_await_progress2(AVCodecContext *avctx, int field, int thread, int shift)
{
}

void ff_thread_report_progress2(AVCodecContext *avctx, int field, int thread, int n)
{

int avcodec_is_open(AVCodecContext *s)
    return !!s->internal;
}
{
    ret = av_bprint_finalize(buf, &str);
        return AVERROR(ENOMEM);
    }
    avctx->extradata = str;
     * string), but the ending character is not accounted in the size (in
     * zeros. */
    return 0;
}
const uint8_t *avpriv_find_start_code(const uint8_t *av_restrict p,
                                      uint32_t *av_restrict state)
    int i;
    av_assert0(p <= end);

            return p;

        if      (p[-1] > 1      ) p += 3;
        else {
            break;
        }
    }

    p = FFMIN(p, end) - 4;

}

AVCPBProperties *av_cpb_properties_alloc(size_t *size)
{
    AVCPBProperties *props = av_mallocz(sizeof(AVCPBProperties));

        *size = sizeof(*props);


    return props;

AVCPBProperties *ff_add_cpb_side_data(AVCodecContext *avctx)
    AVCPBProperties  *props;
    int i;
    for (i = 0; i < avctx->nb_coded_side_data; i++)
            return (AVCPBProperties *)avctx->coded_side_data[i].data;

    if (!props)

    tmp = av_realloc_array(avctx->coded_side_data, avctx->nb_coded_side_data + 1, sizeof(*tmp));

    avctx->coded_side_data[avctx->nb_coded_side_data - 1].type = AV_PKT_DATA_CPB_PROPERTIES;
}

static void codec_parameters_reset(AVCodecParameters *par)
{
    par->codec_id            = AV_CODEC_ID_NONE;
    par->field_order         = AV_FIELD_UNKNOWN;
    par->color_range         = AVCOL_RANGE_UNSPECIFIED;
    par->color_trc           = AVCOL_TRC_UNSPECIFIED;
    par->color_space         = AVCOL_SPC_UNSPECIFIED;
    par->profile             = FF_PROFILE_UNKNOWN;
    par->level               = FF_LEVEL_UNKNOWN;
}

{
    AVCodecParameters *par = av_mallocz(sizeof(*par));
        return NULL;
}

    AVCodecParameters *par = *ppar;
        return;
    codec_parameters_reset(par);

int avcodec_parameters_copy(AVCodecParameters *dst, const AVCodecParameters *src)
{
    codec_parameters_reset(dst);
    memcpy(dst, src, sizeof(*dst));
        if (!dst->extradata)
        memcpy(dst->extradata, src->extradata, src->extradata_size);
}
                                    const AVCodecContext *codec)
{
    codec_parameters_reset(par);
    par->codec_type = codec->codec_type;
    par->bit_rate              = codec->bit_rate;
    par->bits_per_raw_sample   = codec->bits_per_raw_sample;
    par->level                 = codec->level;
    switch (par->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
        par->format              = codec->pix_fmt;
        par->width               = codec->width;
        par->height              = codec->height;
        par->field_order         = codec->field_order;
        par->color_primaries     = codec->color_primaries;
        par->color_trc           = codec->color_trc;
        par->color_space         = codec->colorspace;
        par->chroma_location     = codec->chroma_sample_location;
        par->sample_aspect_ratio = codec->sample_aspect_ratio;
        par->video_delay         = codec->has_b_frames;
        break;
    case AVMEDIA_TYPE_AUDIO:
        par->format           = codec->sample_fmt;
        par->channel_layout   = codec->channel_layout;
        par->channels         = codec->channels;
        par->sample_rate      = codec->sample_rate;
        par->block_align      = codec->block_align;
        par->frame_size       = codec->frame_size;
        par->initial_padding  = codec->initial_padding;
        par->trailing_padding = codec->trailing_padding;
        par->seek_preroll     = codec->seek_preroll;
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        par->width  = codec->width;
        par->height = codec->height;
        break;
    }

    if (codec->extradata) {
        par->extradata = av_mallocz(codec->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
        if (!par->extradata)
            return AVERROR(ENOMEM);
        memcpy(par->extradata, codec->extradata, codec->extradata_size);
        par->extradata_size = codec->extradata_size;
    }

    return 0;
}

int avcodec_parameters_to_context(AVCodecContext *codec,
                                  const AVCodecParameters *par)
{
    codec->codec_type = par->codec_type;
    codec->codec_id   = par->codec_id;
    codec->codec_tag  = par->codec_tag;

    codec->bit_rate              = par->bit_rate;
    codec->bits_per_coded_sample = par->bits_per_coded_sample;
    codec->bits_per_raw_sample   = par->bits_per_raw_sample;
    codec->profile               = par->profile;
    codec->level                 = par->level;
    switch (par->codec_type) {
        codec->pix_fmt                = par->format;
        codec->width                  = par->width;
        codec->height                 = par->height;
        codec->color_range            = par->color_range;
        codec->color_primaries        = par->color_primaries;
        codec->has_b_frames           = par->video_delay;
        break;
        codec->sample_fmt       = par->format;
        codec->channel_layout   = par->channel_layout;
        codec->trailing_padding = par->trailing_padding;
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        codec->height = par->height;
    if (par->extradata) {
            return AVERROR(ENOMEM);
        memcpy(codec->extradata, par->extradata, par->extradata_size);
        codec->extradata_size = par->extradata_size;


        return 0;
    return low + 10*high;
}

                     void **data, size_t *sei_size)

    if (!sd) {
        *data = NULL;
    }
    m  = tc[0] & 3;

    *sei_size = sizeof(uint32_t) * 4;
    if (!*data)
        return AVERROR(ENOMEM);
    init_put_bits(&pb, sei_data, *sei_size);

    for (int j = 1; j <= m; j++) {
        unsigned hh   = bcd2uint(tcsmpte     & 0x3f);    // 6-bit hours
        unsigned mm   = bcd2uint(tcsmpte>>8  & 0x7f);    // 7-bit minutes
            if (av_cmp_q(rate, (AVRational) {50, 1}) == 0)
            ff = (ff + pc) & 0x7f;
        }
        put_bits(&pb, 5, 0); // counting_type
        put_bits(&pb, 1, 0); // discontinuity_flag
        put_bits(&pb, 5, hh);
        put_bits(&pb, 5, 0);
    }

int64_t ff_guess_coded_bitrate(AVCodecContext *avctx)
{
    AVRational framerate = avctx->framerate;
    int64_t bitrate;

    if (!(framerate.num && framerate.den))
        framerate = av_inv_q(avctx->time_base);
    if (!(framerate.num && framerate.den))
        return 0;
    }
              framerate.num / framerate.den;
int ff_int_from_list_or_default(void *ctx, const char * val_name, int val,
                                const int * array_valid_values, int default_value)
{
        ref_val = array_valid_values[i];
        if (ref_val == INT_MAX)
            return val;
    }
    /* val is not a valid value */
}
