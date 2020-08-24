/*
 * The simplest mpeg encoder (well, it was the simplest!)
 * Copyright (c) 2000,2001 Fabrice Bellard
 * Copyright (c) 2002-2004 Michael Niedermayer <michaelni@gmx.at>
 *
 * 4MV & hq & B-frame encoding stuff by Michael Niedermayer <michaelni@gmx.at>
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

/*
 * non linear quantizers with large QPs and VBV with restrictive qmin fixes sponsored by NOA GmbH
 */

/**
 * @file
 * The simplest mpeg encoder (well, it was the simplest!).
 */

#include <stdint.h>

#include "libavutil/internal.h"
#include "libavutil/intmath.h"
#include "libavutil/mathematics.h"
#include "libavutil/pixdesc.h"
#include "libavutil/opt.h"
#include "avcodec.h"
#include "dct.h"
#include "idctdsp.h"
#include "mpeg12.h"
#include "mpegvideo.h"
#include "mpegvideodata.h"
#include "h261.h"
#include "h263.h"
#include "h263data.h"
#include "mjpegenc_common.h"
#include "mathops.h"
#include "mpegutils.h"
#include "mjpegenc.h"
#include "msmpeg4.h"
#include "pixblockdsp.h"
#include "qpeldsp.h"
#include "faandct.h"
#include "thread.h"
#include "aandcttab.h"
#include "flv.h"
#include "mpeg4video.h"
#include "internal.h"
#include "bytestream.h"
#include "wmv2.h"
#include "rv10.h"
#include "packet_internal.h"
#include "libxvid.h"
#include <limits.h>
#include "sp5x.h"

#define QUANT_BIAS_SHIFT 8

#define QMAT_SHIFT_MMX 16
#define QMAT_SHIFT 21

static int encode_picture(MpegEncContext *s, int picture_number);
static int dct_quantize_refine(MpegEncContext *s, int16_t *block, int16_t *weight, int16_t *orig, int n, int qscale);
static int sse_mb(MpegEncContext *s);
static void denoise_dct_c(MpegEncContext *s, int16_t *block);
static int dct_quantize_trellis_c(MpegEncContext *s, int16_t *block, int n, int qscale, int *overflow);

static uint8_t default_mv_penalty[MAX_FCODE + 1][MAX_DMV * 2 + 1];
static uint8_t default_fcode_tab[MAX_MV * 2 + 1];

const AVOption ff_mpv_generic_options[] = {
    FF_MPV_COMMON_OPTS
    { NULL },
};

                       uint16_t (*qmat16)[2][64],
                       const uint16_t *quant_matrix,
                       int bias, int qmin, int qmax, int intra)
{



#if CONFIG_FAANDCT
#endif /* CONFIG_FAANDCT */
            fdsp->fdct == ff_jpeg_fdct_islow_10) {
                /* 16 <= qscale * quant_matrix[i] <= 7905
                 * Assume x = ff_aanscales[i] * qscale * quant_matrix[i]
                 *             19952 <=              x  <= 249205026
                 * (1 << 36) / 19952 >= (1 << 36) / (x) >= (1 << 36) / 249205026
                 *           3444240 >= (1 << 36) / (x) >= 275 */

            }
                /* 16 <= qscale * quant_matrix[i] <= 7905
                 * Assume x = ff_aanscales[i] * qscale * quant_matrix[i]
                 *             19952 <=              x  <= 249205026
                 * (1 << 36) / 19952 >= (1 << 36) / (x) >= (1 << 36) / 249205026
                 *           3444240 >= (1 << 36) / (x) >= 275 */

            }
        } else {
            for (i = 0; i < 64; i++) {
                const int j = s->idsp.idct_permutation[i];
                int64_t den = (int64_t) qscale2 * quant_matrix[j];
                /* We can safely suppose that 16 <= quant_matrix[i] <= 255
                 * Assume x = qscale * quant_matrix[i]
                 * So             16 <=              x  <= 7905
                 * so (1 << 19) / 16 >= (1 << 19) / (x) >= (1 << 19) / 7905
                 * so          32768 >= (1 << 19) / (x) >= 67 */
                qmat[qscale][i] = (int)((UINT64_C(2) << QMAT_SHIFT) / den);
                //qmat  [qscale][i] = (1 << QMAT_SHIFT_MMX) /
                //                    (qscale * quant_matrix[i]);
                qmat16[qscale][0][i] = (2 << QMAT_SHIFT_MMX) / den;

                if (qmat16[qscale][0][i] == 0 ||
                    qmat16[qscale][0][i] == 128 * 256)
                    qmat16[qscale][0][i] = 128 * 256 - 1;
                qmat16[qscale][1][i] =
                    ROUNDED_DIV(bias * (1<<(16 - QUANT_BIAS_SHIFT)),
                                qmat16[qscale][0][i]);
            }
        }

            }
                shift++;
            }
        }
    }
        av_log(s->avctx, AV_LOG_INFO,
               "Warning, QMAT_SHIFT is larger than %d, overflows possible\n",
               QMAT_SHIFT - shift);
    }

{
        int i;
        int bestdiff=INT_MAX;
        int best = 1;

        for (i = 0 ; i<FF_ARRAY_ELEMS(ff_mpeg2_non_linear_qscale); i++) {
            int diff = FFABS((ff_mpeg2_non_linear_qscale[i]<<(FF_LAMBDA_SHIFT + 6)) - (int)s->lambda * 139);
            if (ff_mpeg2_non_linear_qscale[i] < s->avctx->qmin ||
                (ff_mpeg2_non_linear_qscale[i] > s->avctx->qmax && !s->vbv_ignore_qmax))
                continue;
            if (diff < bestdiff) {
                bestdiff = diff;
                best = i;
            }
        }
        s->qscale = best;
    } else {
                    (FF_LAMBDA_SHIFT + 7);
    }

                 FF_LAMBDA_SHIFT;

{

        put_bits(pb, 1, 1);
        for (i = 0; i < 64; i++) {
            put_bits(pb, 8, matrix[ff_zigzag_direct[i]]);
        }
    } else

/**
 * init s->current_picture.qscale_table from s->lambda_table
 */
{

    }

                                              MpegEncContext *src)
{
#define COPY(a) dst->a= src->a
#undef COPY
}

/**
 * Set the given MpegEncContext to defaults for encoding.
 * the changed fields will not depend upon the prior state of the MpegEncContext.
 */
{

    }

}

{

        ff_h263dsp_init(&s->h263dsp);

}

/* init video encoder */
{


            avctx->pix_fmt != AV_PIX_FMT_YUV422P) {
            av_log(avctx, AV_LOG_ERROR,
                   "only YUV420 and YUV422 are supported\n");
            return AVERROR(EINVAL);
        }
        break;
    case AV_CODEC_ID_AMV:
        /* JPEG color space */
            avctx->pix_fmt == AV_PIX_FMT_YUVJ444P ||
            (avctx->color_range == AVCOL_RANGE_JPEG &&
             (avctx->pix_fmt == AV_PIX_FMT_YUV420P ||
              avctx->pix_fmt == AV_PIX_FMT_YUV422P ||
              avctx->pix_fmt == AV_PIX_FMT_YUV444P)))
            format_supported = 1;
        /* MPEG color space */
        else if (avctx->strict_std_compliance <= FF_COMPLIANCE_UNOFFICIAL &&
                 (avctx->pix_fmt == AV_PIX_FMT_YUV420P ||
                  avctx->pix_fmt == AV_PIX_FMT_YUV422P ||
                  avctx->pix_fmt == AV_PIX_FMT_YUV444P))
            format_supported = 1;

        if (!format_supported) {
            av_log(avctx, AV_LOG_ERROR, "colorspace not supported in jpeg\n");
            return AVERROR(EINVAL);
        }
        break;
            av_log(avctx, AV_LOG_ERROR, "only YUV420 is supported\n");
            return AVERROR(EINVAL);
        }
    }

    case AV_PIX_FMT_YUV444P:
    case AV_PIX_FMT_YUV422P:
    case AV_PIX_FMT_YUV420P:
    default:
    }


#if FF_API_PRIVATE_OPT
FF_DISABLE_DEPRECATION_WARNINGS
        s->rtp_payload_size = avctx->rtp_payload_size;
        s->me_pre = avctx->pre_me;
FF_ENABLE_DEPRECATION_WARNINGS
#endif

        avctx->strict_std_compliance > FF_COMPLIANCE_EXPERIMENTAL) {
        av_log(avctx, AV_LOG_WARNING,
               "keyframe interval too large!, reducing it from %d to %d\n",
               avctx->gop_size, 600);
        avctx->gop_size = 600;
    }
        av_log(avctx, AV_LOG_ERROR, "Too many B-frames requested, maximum "
               "is %d.\n", MAX_B_FRAMES);
        avctx->max_b_frames = MAX_B_FRAMES;
    }

    // workaround some differences between how applications specify dc precision
        s->intra_dc_precision += 8;

        av_log(avctx, AV_LOG_ERROR,
                "intra dc precision must be positive, note some applications use"
                " 0 and some 8 as base meaning 8bit, the value must not be smaller than that\n");
        return AVERROR(EINVAL);
    }


        av_log(avctx, AV_LOG_ERROR, "intra dc precision too large\n");
        return AVERROR(EINVAL);
    }

    } else {
    }

    /* Fixed QSCALE */

                        !s->fixed_qscale;


        switch(avctx->codec_id) {
        case AV_CODEC_ID_MPEG1VIDEO:
        case AV_CODEC_ID_MPEG2VIDEO:
            avctx->rc_buffer_size = FFMAX(avctx->rc_max_rate, 15000000) * 112LL / 15000000 * 16384;
            break;
        case AV_CODEC_ID_MPEG4:
        case AV_CODEC_ID_MSMPEG4V1:
        case AV_CODEC_ID_MSMPEG4V2:
        case AV_CODEC_ID_MSMPEG4V3:
            if       (avctx->rc_max_rate >= 15000000) {
                avctx->rc_buffer_size = 320 + (avctx->rc_max_rate - 15000000LL) * (760-320) / (38400000 - 15000000);
            } else if(avctx->rc_max_rate >=  2000000) {
                avctx->rc_buffer_size =  80 + (avctx->rc_max_rate -  2000000LL) * (320- 80) / (15000000 -  2000000);
            } else if(avctx->rc_max_rate >=   384000) {
                avctx->rc_buffer_size =  40 + (avctx->rc_max_rate -   384000LL) * ( 80- 40) / ( 2000000 -   384000);
            } else
                avctx->rc_buffer_size = 40;
            avctx->rc_buffer_size *= 16384;
            break;
        }
        if (avctx->rc_buffer_size) {
            av_log(avctx, AV_LOG_INFO, "Automatically choosing VBV buffer size of %d kbyte\n", avctx->rc_buffer_size/8192);
        }
    }

        av_log(avctx, AV_LOG_ERROR, "Either both buffer size and max rate or neither must be specified\n");
        return AVERROR(EINVAL);
    }

        av_log(avctx, AV_LOG_INFO,
               "Warning min_rate > 0 but min_rate != max_rate isn't recommended!\n");
    }

        av_log(avctx, AV_LOG_ERROR, "bitrate below min bitrate\n");
        return AVERROR(EINVAL);
    }

        av_log(avctx, AV_LOG_ERROR, "bitrate above max bitrate\n");
        return AVERROR(EINVAL);
    }

        avctx->rc_max_rate != avctx->rc_min_rate) {
        av_log(avctx, AV_LOG_INFO,
               "impossible bitrate constraints, this will fail\n");
    }

        av_log(avctx, AV_LOG_ERROR, "VBV buffer too small for bitrate\n");
        return AVERROR(EINVAL);
    }

        av_log(avctx, AV_LOG_WARNING,
               "bitrate tolerance %d too small for bitrate %"PRId64", overriding\n", avctx->bit_rate_tolerance, avctx->bit_rate);
        avctx->bit_rate_tolerance = 5 * avctx->bit_rate * av_q2d(avctx->time_base);
    }

        av_log(avctx, AV_LOG_INFO,
               "Warning vbv_delay will be set to 0xFFFF (=VBR) as the "
               "specified vbv buffer is too large for the given bitrate!\n");
    }

        s->codec_id != AV_CODEC_ID_H263 && s->codec_id != AV_CODEC_ID_H263P &&
        s->codec_id != AV_CODEC_ID_FLV1) {
        av_log(avctx, AV_LOG_ERROR, "4MV not supported by codec\n");
        return AVERROR(EINVAL);
    }

        av_log(avctx, AV_LOG_ERROR,
               "OBMC is only supported with simple mb decision\n");
        return AVERROR(EINVAL);
    }

        av_log(avctx, AV_LOG_ERROR, "qpel not supported by codec\n");
        return AVERROR(EINVAL);
    }

        s->codec_id != AV_CODEC_ID_MPEG2VIDEO) {
        av_log(avctx, AV_LOG_ERROR, "B-frames not supported by codec\n");
        return AVERROR(EINVAL);
    }
        av_log(avctx, AV_LOG_ERROR,
               "max b frames must be 0 or positive for mpegvideo based encoders\n");
        return AVERROR(EINVAL);
    }

        av_log(avctx, AV_LOG_WARNING,
               "Invalid pixel aspect ratio %i/%i, limit is 255/255 reducing\n",
               avctx->sample_aspect_ratio.num, avctx->sample_aspect_ratio.den);
        av_reduce(&avctx->sample_aspect_ratio.num, &avctx->sample_aspect_ratio.den,
                   avctx->sample_aspect_ratio.num,  avctx->sample_aspect_ratio.den, 255);
    }

        av_log(avctx, AV_LOG_ERROR, "H.263 does not support resolutions above 2048x1152\n");
        return AVERROR(EINVAL);
    }
        av_log(avctx, AV_LOG_ERROR, "w/h must be a multiple of 4\n");
        return AVERROR(EINVAL);
    }

        av_log(avctx, AV_LOG_ERROR, "MPEG-1 does not support resolutions above 4095x4095\n");
        return AVERROR(EINVAL);
    }

        av_log(avctx, AV_LOG_ERROR, "MPEG-2 does not support resolutions above 16383x16383\n");
        return AVERROR(EINVAL);
    }

        av_log(avctx, AV_LOG_ERROR, "width and height must be a multiple of 16\n");
        return AVERROR(EINVAL);
    }

        av_log(avctx, AV_LOG_ERROR, "width and height must be a multiple of 4\n");
        return AVERROR(EINVAL);
    }

        av_log(avctx, AV_LOG_ERROR, "width must be multiple of 2\n");
        return AVERROR(EINVAL);
    }

        av_log(avctx, AV_LOG_ERROR, "interlacing not supported by codec\n");
        return AVERROR(EINVAL);
    }

#if FF_API_PRIVATE_OPT
    FF_DISABLE_DEPRECATION_WARNINGS
        s->mpeg_quant = avctx->mpeg_quant;
    FF_ENABLE_DEPRECATION_WARNINGS
#endif

    // FIXME mpeg2 uses that too
                          && s->codec_id != AV_CODEC_ID_MPEG2VIDEO)) {
        av_log(avctx, AV_LOG_ERROR,
               "mpeg2 style quantization not supported by codec\n");
        return AVERROR(EINVAL);
    }

        av_log(avctx, AV_LOG_ERROR, "CBP RD needs trellis quant\n");
        return AVERROR(EINVAL);
    }

        av_log(avctx, AV_LOG_ERROR, "QP RD needs mbd=2\n");
        return AVERROR(EINVAL);
    }

             s->codec_id == AV_CODEC_ID_MJPEG)) {
        // Used to produce garbage with MJPEG.
        av_log(avctx, AV_LOG_ERROR,
               "QP RD is no longer compatible with MJPEG or AMV\n");
        return AVERROR(EINVAL);
    }

#if FF_API_PRIVATE_OPT
FF_DISABLE_DEPRECATION_WARNINGS
        s->scenechange_threshold = avctx->scenechange_threshold;
FF_ENABLE_DEPRECATION_WARNINGS
#endif

        (s->avctx->flags & AV_CODEC_FLAG_CLOSED_GOP)) {
        av_log(avctx, AV_LOG_ERROR,
               "closed gop with scene change detection are not supported yet, "
               "set threshold to 1000000000\n");
        return AVERROR_PATCHWELCOME;
    }

            s->strict_std_compliance >= FF_COMPLIANCE_NORMAL) {
            av_log(avctx, AV_LOG_ERROR,
                   "low delay forcing is only available for mpeg2, "
                   "set strict_std_compliance to 'unofficial' or lower in order to allow it\n");
            return AVERROR(EINVAL);
        }
            av_log(avctx, AV_LOG_ERROR,
                   "B-frames cannot be used with low delay\n");
            return AVERROR(EINVAL);
        }
    }

            av_log(avctx, AV_LOG_ERROR,
                   "non linear quant only supports qmax <= 28 currently\n");
            return AVERROR_PATCHWELCOME;
        }
    }

        av_log(avctx, AV_LOG_ERROR, "Multiple slices are not supported by this codec\n");
        return AVERROR(EINVAL);
    }

        s->codec_id != AV_CODEC_ID_MPEG2VIDEO &&
        s->codec_id != AV_CODEC_ID_MJPEG      &&
        (s->codec_id != AV_CODEC_ID_H263P)) {
        av_log(avctx, AV_LOG_ERROR,
               "multi threaded encoding not supported by codec\n");
        return AVERROR_PATCHWELCOME;
    }

        av_log(avctx, AV_LOG_ERROR,
               "automatic thread number detection not supported by codec, "
               "patch welcome\n");
        return AVERROR_PATCHWELCOME;
    }

        av_log(avctx, AV_LOG_ERROR, "framerate not set\n");
        return AVERROR(EINVAL);
    }

#if FF_API_PRIVATE_OPT
FF_DISABLE_DEPRECATION_WARNINGS
        s->b_frame_strategy = avctx->b_frame_strategy;
        s->b_sensitivity = avctx->b_sensitivity;
FF_ENABLE_DEPRECATION_WARNINGS
#endif

        av_log(avctx, AV_LOG_INFO,
               "notice: b_frame_strategy only affects the first pass\n");
        s->b_frame_strategy = 0;
    }

        av_log(avctx, AV_LOG_INFO, "removing common factors from framerate\n");
        avctx->time_base.den /= i;
        avctx->time_base.num /= i;
        //return -1;
    }

        // (a + x * 3 / 8) / x
    } else {
        // (a - x / 4) / x
    }

        av_log(avctx, AV_LOG_ERROR, "qmin and or qmax are invalid, they must be 0 < min <= max\n");
        return AVERROR(EINVAL);
    }


        av_log(avctx, AV_LOG_ERROR,
               "timebase %d/%d not supported by MPEG 4 standard, "
               "the maximum admitted value for the timebase denominator "
               "is %d\n", s->avctx->time_base.num, s->avctx->time_base.den,
               (1 << 16) - 1);
        return AVERROR(EINVAL);
    }

    case AV_CODEC_ID_AMV:
        if ((ret = ff_mjpeg_encode_init(s)) < 0)
            return ret;
        avctx->delay = 0;
        s->low_delay = 1;
        break;
    case AV_CODEC_ID_H261:
        if (ff_h261_get_picture_format(s->width, s->height) < 0) {
                   "The specified picture size of %dx%d is not valid for the "
                   "H.261 codec.\nValid sizes are 176x144, 352x288\n",
                    s->width, s->height);
        }
        s->out_format = FMT_H261;
        avctx->delay  = 0;
        s->low_delay  = 1;
        s->rtp_mode   = 0; /* Sliced encoding not supported */
        break;
    case AV_CODEC_ID_H263:
        if (ff_match_2uint16(ff_h263_format, FF_ARRAY_ELEMS(ff_h263_format),
                             s->width, s->height) == 8) {
                   "The specified picture size of %dx%d is not valid for "
                   "the H.263 codec.\nValid sizes are 128x96, 176x144, "
                   "352x288, 704x576, and 1408x1152. "
                   "Try H.263+.\n", s->width, s->height);
        }
        s->out_format = FMT_H263;
        avctx->delay  = 0;
        s->low_delay  = 1;
        break;
        /* Fx */

        /* /Fx */
        /* These are just to be sure */
    default:
        return AVERROR(EINVAL);
    }

#if FF_API_PRIVATE_OPT
    FF_DISABLE_DEPRECATION_WARNINGS
        s->noise_reduction = avctx->noise_reduction;
    FF_ENABLE_DEPRECATION_WARNINGS
#endif



                                                AV_CODEC_FLAG_INTERLACED_ME) ||

    /* init */
        return ret;


            return AVERROR(ENOMEM);
    }

        return AVERROR(ENOMEM);

            return AVERROR(ENOMEM);
    }


        s->chroma_qscale_table = ff_h263_chroma_qscale_table;


            s->h263_slice_structured = 1;
    }


#if FF_API_PRIVATE_OPT
FF_DISABLE_DEPRECATION_WARNINGS
        s->frame_skip_threshold = avctx->frame_skip_threshold;
        s->frame_skip_factor = avctx->frame_skip_factor;
        s->frame_skip_exp = avctx->frame_skip_exp;
        s->frame_skip_cmp = avctx->frame_skip_cmp;
FF_ENABLE_DEPRECATION_WARNINGS
#endif


        ff_h261_encode_init(s);
        ff_h263_encode_init(s);
        if ((ret = ff_msmpeg4_encode_init(s)) < 0)
            return ret;
        && s->out_format == FMT_MPEG1)
        ff_mpeg1_encode_init(s);

    /* init q matrix */
            s->mpeg_quant) {
        } else {
            /* MPEG-1/2 */
        }
            s->intra_matrix[j] = s->avctx->intra_matrix[i];
            s->inter_matrix[j] = s->avctx->inter_matrix[i];
    }

    /* precompute matrix */
    /* for mjpeg, we do include qscale in the matrix */
                          31, 1);
                          31, 0);
    }

        return ret;

#if FF_API_PRIVATE_OPT
    FF_DISABLE_DEPRECATION_WARNINGS
        s->brd_scale = avctx->brd_scale;

        s->pred = avctx->prediction_method + 1;
    FF_ENABLE_DEPRECATION_WARNINGS
#endif

        for (i = 0; i < s->max_b_frames + 2; i++) {
            s->tmp_frames[i] = av_frame_alloc();
            if (!s->tmp_frames[i])
                return AVERROR(ENOMEM);

            s->tmp_frames[i]->format = AV_PIX_FMT_YUV420P;
            s->tmp_frames[i]->width  = s->width  >> s->brd_scale;
            s->tmp_frames[i]->height = s->height >> s->brd_scale;

            ret = av_frame_get_buffer(s->tmp_frames[i], 0);
            if (ret < 0)
                return ret;
        }
    }

        return AVERROR(ENOMEM);

}

{


        s->out_format == FMT_MJPEG)
        ff_mjpeg_encode_close(s);






}

static int get_sae(uint8_t *src, int ref, int stride)
{
    int x,y;
    int acc = 0;

    for (y = 0; y < 16; y++) {
        for (x = 0; x < 16; x++) {
            acc += FFABS(src[x + y * stride] - ref);
        }
    }

    return acc;
}

static int get_intra_count(MpegEncContext *s, uint8_t *src,
                           uint8_t *ref, int stride)
{
    int x, y, w, h;
    int acc = 0;

    w = s->width  & ~15;
    h = s->height & ~15;

    for (y = 0; y < h; y += 16) {
        for (x = 0; x < w; x += 16) {
            int offset = x + y * stride;
            int sad  = s->mecc.sad[0](NULL, src + offset, ref + offset,
                                      stride, 16);
            int mean = (s->mpvencdsp.pix_sum(src + offset, stride) + 128) >> 8;
            int sae  = get_sae(src + offset, mean, stride);

            acc += sae + 500 < sad;
        }
    }
    return acc;
}

{
                            s->mb_stride, s->mb_width, s->mb_height, s->b8_stride,
                            &s->linesize, &s->uvlinesize);
}

{



                    av_log(s->avctx, AV_LOG_ERROR,
                           "Invalid pts (%"PRId64") <= last (%"PRId64")\n",
                           pts, last);
                    return AVERROR(EINVAL);
                }

            }
        } else {
            if (s->user_specified_pts != AV_NOPTS_VALUE) {
                s->user_specified_pts =
                pts = s->user_specified_pts + 1;
                av_log(s->avctx, AV_LOG_INFO,
                       "Warning: AVFrame.pts=? trying to guess (%"PRId64")\n",
                       pts);
            } else {
                pts = display_picture_number;
            }
        }

            direct = 0;

                pic_arg->linesize[1], s->linesize, s->uvlinesize);

            return i;


                return ret;
        }
            return ret;

                pic->f->data[1] + INPLACE_OFFSET == pic_arg->data[1] &&
                pic->f->data[2] + INPLACE_OFFSET == pic_arg->data[2]) {
                // empty
            } else {
                                                 &h_chroma_shift,
                                                 &v_chroma_shift);




                    else {
                        int h2 = h;
                        uint8_t *dst2 = dst;
                        }
                    }
                                                w, h,
                                                16 >> h_shift,
                                                vpad >> v_shift,
                                                EDGE_BOTTOM);
                    }
                }
            }
        }
            return ret;

    } else {
        /* Flushing: When we have not received enough input frames,
         * ensure s->input_picture[0] contains the first picture */
                break;

            flush_offset = 1;
        else
            encoding_delay = encoding_delay - flush_offset + 1;
    }

    /* shift buffer entries */


}

static int skip_check(MpegEncContext *s, Picture *p, Picture *ref)
{
    int x, y, plane;
    int score = 0;
    int64_t score64 = 0;

    for (plane = 0; plane < 3; plane++) {
        const int stride = p->f->linesize[plane];
        const int bw = plane ? 1 : 2;
        for (y = 0; y < s->mb_height * bw; y++) {
            for (x = 0; x < s->mb_width * bw; x++) {
                int off = p->shared ? 0 : 16;
                uint8_t *dptr = p->f->data[plane] + 8 * (x + y * stride) + off;
                uint8_t *rptr = ref->f->data[plane] + 8 * (x + y * stride);
                int v = s->mecc.frame_skip_cmp[1](s, dptr, rptr, stride, 8);

                switch (FFABS(s->frame_skip_exp)) {
                case 0: score    =  FFMAX(score, v);          break;
                case 1: score   += FFABS(v);                  break;
                case 2: score64 += v * (int64_t)v;                       break;
                case 3: score64 += FFABS(v * (int64_t)v * v);            break;
                case 4: score64 += (v * (int64_t)v) * (v * (int64_t)v);  break;
                }
            }
        }
    }
    emms_c();

    if (score)
        score64 = score;
    if (s->frame_skip_exp < 0)
        score64 = pow(score64 / (double)(s->mb_width * s->mb_height),
                      -1.0/s->frame_skip_exp);

    if (score64 < s->frame_skip_threshold)
        return 1;
    if (score64 < ((s->frame_skip_factor * (int64_t) s->lambda) >> 8))
        return 1;
    return 0;
}

static int encode_frame(AVCodecContext *c, AVFrame *frame)
{
    AVPacket pkt = { 0 };
    int ret;
    int size = 0;

    av_init_packet(&pkt);

    ret = avcodec_send_frame(c, frame);
    if (ret < 0)
        return ret;

    do {
        ret = avcodec_receive_packet(c, &pkt);
        if (ret >= 0) {
            size += pkt.size;
            av_packet_unref(&pkt);
        } else if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
            return ret;
    } while (ret >= 0);

    return size;
}

static int estimate_best_b_count(MpegEncContext *s)
{
    const AVCodec *codec = avcodec_find_encoder(s->avctx->codec_id);
    const int scale = s->brd_scale;
    int width  = s->width  >> scale;
    int height = s->height >> scale;
    int i, j, out_size, p_lambda, b_lambda, lambda2;
    int64_t best_rd  = INT64_MAX;
    int best_b_count = -1;
    int ret = 0;

    av_assert0(scale >= 0 && scale <= 3);

    //emms_c();
    //s->next_picture_ptr->quality;
    p_lambda = s->last_lambda_for[AV_PICTURE_TYPE_P];
    //p_lambda * FFABS(s->avctx->b_quant_factor) + s->avctx->b_quant_offset;
    b_lambda = s->last_lambda_for[AV_PICTURE_TYPE_B];
    if (!b_lambda) // FIXME we should do this somewhere else
        b_lambda = p_lambda;
    lambda2  = (b_lambda * b_lambda + (1 << FF_LAMBDA_SHIFT) / 2) >>
               FF_LAMBDA_SHIFT;

    for (i = 0; i < s->max_b_frames + 2; i++) {
        Picture pre_input, *pre_input_ptr = i ? s->input_picture[i - 1] :
                                                s->next_picture_ptr;
        uint8_t *data[4];

        if (pre_input_ptr && (!i || s->input_picture[i - 1])) {
            pre_input = *pre_input_ptr;
            memcpy(data, pre_input_ptr->f->data, sizeof(data));

            if (!pre_input.shared && i) {
                data[0] += INPLACE_OFFSET;
                data[1] += INPLACE_OFFSET;
                data[2] += INPLACE_OFFSET;
            }

            s->mpvencdsp.shrink[scale](s->tmp_frames[i]->data[0],
                                       s->tmp_frames[i]->linesize[0],
                                       data[0],
                                       pre_input.f->linesize[0],
                                       width, height);
            s->mpvencdsp.shrink[scale](s->tmp_frames[i]->data[1],
                                       s->tmp_frames[i]->linesize[1],
                                       data[1],
                                       pre_input.f->linesize[1],
                                       width >> 1, height >> 1);
            s->mpvencdsp.shrink[scale](s->tmp_frames[i]->data[2],
                                       s->tmp_frames[i]->linesize[2],
                                       data[2],
                                       pre_input.f->linesize[2],
                                       width >> 1, height >> 1);
        }
    }

    for (j = 0; j < s->max_b_frames + 1; j++) {
        AVCodecContext *c;
        int64_t rd = 0;

        if (!s->input_picture[j])
            break;

        c = avcodec_alloc_context3(NULL);
        if (!c)
            return AVERROR(ENOMEM);

        c->width        = width;
        c->height       = height;
        c->flags        = AV_CODEC_FLAG_QSCALE | AV_CODEC_FLAG_PSNR;
        c->flags       |= s->avctx->flags & AV_CODEC_FLAG_QPEL;
        c->mb_decision  = s->avctx->mb_decision;
        c->me_cmp       = s->avctx->me_cmp;
        c->mb_cmp       = s->avctx->mb_cmp;
        c->me_sub_cmp   = s->avctx->me_sub_cmp;
        c->pix_fmt      = AV_PIX_FMT_YUV420P;
        c->time_base    = s->avctx->time_base;
        c->max_b_frames = s->max_b_frames;

        ret = avcodec_open2(c, codec, NULL);
        if (ret < 0)
            goto fail;

        s->tmp_frames[0]->pict_type = AV_PICTURE_TYPE_I;
        s->tmp_frames[0]->quality   = 1 * FF_QP2LAMBDA;

        out_size = encode_frame(c, s->tmp_frames[0]);
        if (out_size < 0) {
            ret = out_size;
            goto fail;
        }

        //rd += (out_size * lambda2) >> FF_LAMBDA_SHIFT;

        for (i = 0; i < s->max_b_frames + 1; i++) {
            int is_p = i % (j + 1) == j || i == s->max_b_frames;

            s->tmp_frames[i + 1]->pict_type = is_p ?
                                     AV_PICTURE_TYPE_P : AV_PICTURE_TYPE_B;
            s->tmp_frames[i + 1]->quality   = is_p ? p_lambda : b_lambda;

            out_size = encode_frame(c, s->tmp_frames[i + 1]);
            if (out_size < 0) {
                ret = out_size;
                goto fail;
            }

            rd += (out_size * lambda2) >> (FF_LAMBDA_SHIFT - 3);
        }

        /* get the delayed frames */
        out_size = encode_frame(c, NULL);
        if (out_size < 0) {
            ret = out_size;
            goto fail;
        }
        rd += (out_size * lambda2) >> (FF_LAMBDA_SHIFT - 3);

        rd += c->error[0] + c->error[1] + c->error[2];

        if (rd < best_rd) {
            best_rd = rd;
            best_b_count = j;
        }

fail:
        avcodec_free_context(&c);
        if (ret < 0)
            return ret;
    }

    return best_b_count;
}

{


    /* set next picture type & ordering */
            if (s->picture_in_gop_number < s->gop_size &&
                s->next_picture_ptr &&
                skip_check(s, s->input_picture[0], s->next_picture_ptr)) {
                // FIXME check that the gop check above is +-1 correct
                av_frame_unref(s->input_picture[0]->f);

                ff_vbv_update(s, 0);

                goto no_output_pic;
            }
        }

        } else {

                for (i = 0; i < s->max_b_frames + 1; i++) {
                    int pict_num = s->input_picture[0]->f->display_picture_number + i;

                    if (pict_num >= s->rc_context.num_entries)
                        break;
                    if (!s->input_picture[i]) {
                        s->rc_context.entry[pict_num - 1].new_pict_type = AV_PICTURE_TYPE_P;
                        break;
                    }

                    s->input_picture[i]->f->pict_type =
                        s->rc_context.entry[pict_num].new_pict_type;
                }
            }

            } else if (s->b_frame_strategy == 1) {
                for (i = 1; i < s->max_b_frames + 1; i++) {
                    if (s->input_picture[i] &&
                        s->input_picture[i]->b_frame_score == 0) {
                        s->input_picture[i]->b_frame_score =
                            get_intra_count(s,
                                            s->input_picture[i    ]->f->data[0],
                                            s->input_picture[i - 1]->f->data[0],
                                            s->linesize) + 1;
                    }
                }
                for (i = 0; i < s->max_b_frames + 1; i++) {
                    if (!s->input_picture[i] ||
                        s->input_picture[i]->b_frame_score - 1 >
                            s->mb_num / s->b_sensitivity)
                        break;
                }

                b_frames = FFMAX(0, i - 1);

                /* reset scores */
                for (i = 0; i < b_frames + 1; i++) {
                    s->input_picture[i]->b_frame_score = 0;
                }
            } else if (s->b_frame_strategy == 2) {
                b_frames = estimate_best_b_count(s);
                if (b_frames < 0)
                    return b_frames;
            }


                    b_frames = i;
            }
                b_frames == s->max_b_frames) {
                av_log(s->avctx, AV_LOG_ERROR,
                       "warning, too many B-frames in a row\n");
            }

                    s->gop_size > s->picture_in_gop_number) {
                    b_frames = s->gop_size - s->picture_in_gop_number - 1;
                } else {
                        b_frames = 0;
                }
            }

                s->input_picture[b_frames]->f->pict_type == AV_PICTURE_TYPE_I)
                b_frames--;

                    AV_PICTURE_TYPE_B;
            }
        }
    }


            return ret;

            // input is a shared pix, so we can't modify it -> allocate a new
            // one & ensure that the shared one is reuseable

                return i;

                return -1;
            }

                return ret;

            /* mark us unused / free shared pic */

        } else {
            // input is not a shared pix -> reuse buffer for current_pix
            }
        }
                                       s->current_picture_ptr)) < 0)
            return ret;

    }
    return 0;
}

{
                                s->h_edge_pos, s->v_edge_pos,
                                EDGE_WIDTH, EDGE_WIDTH,
                                EDGE_TOP | EDGE_BOTTOM);
                                EDGE_WIDTH >> hshift,
                                EDGE_WIDTH >> vshift,
                                EDGE_TOP | EDGE_BOTTOM);
                                EDGE_WIDTH >> hshift,
                                EDGE_WIDTH >> vshift,
                                EDGE_TOP | EDGE_BOTTOM);
    }



#if FF_API_CODED_FRAME
FF_DISABLE_DEPRECATION_WARNINGS
FF_ENABLE_DEPRECATION_WARNINGS
#endif
#if FF_API_ERROR_FRAME
FF_DISABLE_DEPRECATION_WARNINGS
           sizeof(s->current_picture.encoding_error));
FF_ENABLE_DEPRECATION_WARNINGS
#endif

{

            }
        }

        }
    }

{

    /* mark & release old frames */
    }


                                   s->current_picture_ptr)) < 0)
        return ret;

    }

                                       s->last_picture_ptr)) < 0)
            return ret;
    }
                                       s->next_picture_ptr)) < 0)
            return ret;
    }

        int i;
        for (i = 0; i < 4; i++) {
            if (s->picture_structure == PICT_BOTTOM_FIELD) {
                s->current_picture.f->data[i] +=
                    s->current_picture.f->linesize[i];
            }
            s->current_picture.f->linesize[i] *= 2;
            s->last_picture.f->linesize[i]    *= 2;
            s->next_picture.f->linesize[i]    *= 2;
        }
    }

    } else {
    }

    }

    return 0;
}

                          const AVFrame *pic_arg, int *got_packet)
{



        return -1;

        return -1;
    }

    /* output? */
            return ret;
            s->mb_info_ptr = av_packet_new_side_data(pkt,
                                 AV_PKT_DATA_H263_MB_INFO,
                                 s->mb_width*s->mb_height*12);
            s->prev_mb_info = s->last_mb_info = s->mb_info_size = 0;
        }


        }

        //emms_c();
            return ret;
        }
            return -1;

#if FF_API_STAT_BITS
FF_DISABLE_DEPRECATION_WARNINGS
        // FIXME f/b_count in avctx
FF_ENABLE_DEPRECATION_WARNINGS
#endif


            ff_mjpeg_encode_picture_trailer(&s->pb, s->header_bits);

            int min_step = hq ? 1 : (1<<(FF_LAMBDA_SHIFT + 7))/139;

                                       (s->qscale + 1) / s->qscale);
                    int i;
                    for (i = 0; i < s->mb_height * s->mb_stride; i++)
                        s->lambda_table[i] =
                            FFMAX(s->lambda_table[i] + min_step,
                                  s->lambda_table[i] * (s->qscale + 1) /
                                  s->qscale);
                }
                // done in encode_picture() so we must undo it
                    if (s->flipflop_rounding          ||
                        s->codec_id == AV_CODEC_ID_H263P ||
                        s->codec_id == AV_CODEC_ID_MPEG4)
                        s->no_rounding ^= 1;
                }
                }
                }
            }

        }

            ff_write_pass1_stats(s);

        }
                                       s->pict_type);

                                             s->misc_bits + s->i_tex_bits +
                                             s->p_tex_bits);

                av_log(s->avctx, AV_LOG_ERROR, "stuffing too large\n");
                return -1;
            }

            case AV_CODEC_ID_MPEG1VIDEO:
            case AV_CODEC_ID_MPEG2VIDEO:
                }
            break;
            case AV_CODEC_ID_MPEG4:
                put_bits(&s->pb, 16, 0);
                put_bits(&s->pb, 16, 0x1C3);
                stuffing_count -= 4;
                while (stuffing_count--) {
                    put_bits(&s->pb, 8, 0xFF);
                }
            break;
            default:
                av_log(s->avctx, AV_LOG_ERROR, "vbv buffer overflow\n");
            }
        }

        /* update MPEG-1/2 vbv_delay for CBR */


                av_log(s->avctx, AV_LOG_ERROR,
                       "Internal error, negative bits\n");


                        s->avctx->rc_max_rate;




                return AVERROR(ENOMEM);

                                          (uint8_t*)props, props_size);
                av_freep(&props);
                return ret;
            }

#if FF_API_VBV_DELAY
FF_DISABLE_DEPRECATION_WARNINGS
FF_ENABLE_DEPRECATION_WARNINGS
#endif
        }
#if FF_API_STAT_BITS
FF_DISABLE_DEPRECATION_WARNINGS
FF_ENABLE_DEPRECATION_WARNINGS
#endif


            else
        } else
            av_packet_shrink_side_data(pkt, AV_PKT_DATA_H263_MB_INFO, s->mb_info_size);
    } else {
    }

    /* release non-reference frames */
    }


}

static inline void dct_single_coeff_elimination(MpegEncContext *s,
                                                int n, int threshold)
{
    static const char tab[64] = {
        3, 2, 2, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    };
    int score = 0;
    int run = 0;
    int i;
    int16_t *block = s->block[n];
    const int last_index = s->block_last_index[n];
    int skip_dc;

    if (threshold < 0) {
        skip_dc = 0;
        threshold = -threshold;
    } else
        skip_dc = 1;

    /* Are all we could set to zero already zero? */
    if (last_index <= skip_dc - 1)
        return;

    for (i = 0; i <= last_index; i++) {
        const int j = s->intra_scantable.permutated[i];
        const int level = FFABS(block[j]);
        if (level == 1) {
            if (skip_dc && i == 0)
                continue;
            score += tab[run];
            run = 0;
        } else if (level > 1) {
            return;
        } else {
            run++;
        }
    }
    if (score >= threshold)
        return;
    for (i = skip_dc; i <= last_index; i++) {
        const int j = s->intra_scantable.permutated[i];
        block[j] = 0;
    }
    if (block[0])
        s->block_last_index[n] = 0;
    else
        s->block_last_index[n] = -1;
}

static inline void clip_coeffs(MpegEncContext *s, int16_t *block,
                               int last_index)
{
    int i;
    const int maxlevel = s->max_qcoeff;
    const int minlevel = s->min_qcoeff;
    int overflow = 0;

    if (s->mb_intra) {
        i = 1; // skip clipping of intra dc
    } else
        i = 0;

    for (; i <= last_index; i++) {
        const int j = s->intra_scantable.permutated[i];
        int level = block[j];

        if (level > maxlevel) {
            level = maxlevel;
            overflow++;
        } else if (level < minlevel) {
            level = minlevel;
            overflow++;
        }

        block[j] = level;
    }

    if (overflow && s->avctx->mb_decision == FF_MB_DECISION_SIMPLE)
        av_log(s->avctx, AV_LOG_INFO,
               "warning, clipping %d dct coefficients to %d..%d\n",
               overflow, minlevel, maxlevel);
}

static void get_visual_weight(int16_t *weight, uint8_t *ptr, int stride)
{
    int x, y;
    // FIXME optimize
    for (y = 0; y < 8; y++) {
        for (x = 0; x < 8; x++) {
            int x2, y2;
            int sum = 0;
            int sqr = 0;
            int count = 0;

            for (y2 = FFMAX(y - 1, 0); y2 < FFMIN(8, y + 2); y2++) {
                for (x2= FFMAX(x - 1, 0); x2 < FFMIN(8, x + 2); x2++) {
                    int v = ptr[x2 + y2 * stride];
                    sum += v;
                    sqr += v * v;
                    count++;
                }
            }
            weight[x + 8 * y]= (36 * ff_sqrt(count * sqr - sum * sum)) / count;
        }
    }
}

                                                int motion_x, int motion_y,
                                                int mb_block_height,
                                                int mb_block_width,
                                                int mb_block_count)
{






                        }
                            s->dquant = 0;
                    }
                }
            }
        }
        ff_set_qscale(s, s->qscale + s->dquant);

             (mb_y * mb_block_height * wrap_c) + mb_x * mb_block_width;

                                 wrap_y, wrap_y,
                                 16, 16, mb_x * 16, mb_y * 16,
                                 s->width, s->height);
                                 wrap_c, wrap_c,
                                 mb_block_width, mb_block_height,
                                 mb_x * mb_block_width, mb_y * mb_block_height,
                                 cw, ch);
                                 wrap_c, wrap_c,
                                 mb_block_width, mb_block_height,
                                 mb_x * mb_block_width, mb_y * mb_block_height,
                                 cw, ch);
    }


                                                     NULL, wrap_y, 8) - 400;

                                                        NULL, wrap_y * 2, 8) +
                                                        NULL, wrap_y * 2, 8);

                        s->chroma_format == CHROMA_444)
                        wrap_c <<= 1;
                }
            }
        }


            skip_dct[4] = 1;
            skip_dct[5] = 1;
        } else {
            }
        }
    } else {


        } else {
        }

                          op_pix, op_qpix);
        }
                          op_pix, op_qpix);
        }


                                                     wrap_y, 8) - 400;

                progressive_score -= 400;

                                                        wrap_y * 2, 8) +
                                                        ptr_y + wrap_y,
                                                        wrap_y * 2, 8);


                        wrap_c <<= 1;
                }
            }
        }


            skip_dct[4] = 1;
            skip_dct[5] = 1;
        } else {
                s->pdsp.diff_pixels(s->block[6], ptr_cb + uv_dct_offset,
                                    dest_cb + uv_dct_offset, wrap_c);
                s->pdsp.diff_pixels(s->block[7], ptr_cr + uv_dct_offset,
                                    dest_cr + uv_dct_offset, wrap_c);
            }
        }
        /* pre quantization */
            // FIXME optimize
                if (s->mecc.sad[1](NULL, ptr_cb + uv_dct_offset,
                                   dest_cb + uv_dct_offset,
                                   wrap_c, 8) < 20 * s->qscale)
                    skip_dct[6] = 1;
                if (s->mecc.sad[1](NULL, ptr_cr + uv_dct_offset,
                                   dest_cr + uv_dct_offset,
                                   wrap_c, 8) < 20 * s->qscale)
                    skip_dct[7] = 1;
            }
        }
    }

        if (!skip_dct[0])
            get_visual_weight(weight[0], ptr_y                 , wrap_y);
        if (!skip_dct[1])
            get_visual_weight(weight[1], ptr_y              + 8, wrap_y);
        if (!skip_dct[2])
            get_visual_weight(weight[2], ptr_y + dct_offset    , wrap_y);
        if (!skip_dct[3])
            get_visual_weight(weight[3], ptr_y + dct_offset + 8, wrap_y);
        if (!skip_dct[4])
            get_visual_weight(weight[4], ptr_cb                , wrap_c);
        if (!skip_dct[5])
            get_visual_weight(weight[5], ptr_cr                , wrap_c);
        if (!s->chroma_y_shift) { /* 422 */
            if (!skip_dct[6])
                get_visual_weight(weight[6], ptr_cb + uv_dct_offset,
                                  wrap_c);
            if (!skip_dct[7])
                get_visual_weight(weight[7], ptr_cr + uv_dct_offset,
                                  wrap_c);
        }
        memcpy(orig[0], s->block[0], sizeof(int16_t) * 64 * mb_block_count);
    }

    /* DCT & quantize */
    av_assert2(s->out_format != FMT_MJPEG || s->qscale == 8);
    {
                // FIXME we could decide to change to quantizer instead of
                // clipping
                // JS: I don't think that would be a good idea it could lower
                //     quality instead of improve it. Just INTRADC clipping
                //     deserves changes in quantizer
                    clip_coeffs(s, s->block[i], s->block_last_index[i]);
            } else
        }
            for (i = 0; i < mb_block_count; i++) {
                if (!skip_dct[i]) {
                    s->block_last_index[i] =
                        dct_quantize_refine(s, s->block[i], weight[i],
                                            orig[i], i, s->qscale);
                }
            }
        }

            for (i = 0; i < 4; i++)
                dct_single_coeff_elimination(s, i, s->luma_elim_threshold);
            for (i = 4; i < mb_block_count; i++)
                dct_single_coeff_elimination(s, i, s->chroma_elim_threshold);

            for (i = 0; i < mb_block_count; i++) {
                if (s->block_last_index[i] == -1)
                    s->coded_score[i] = INT_MAX / 256;
            }
        }
    }

        s->block_last_index[4] =
        s->block_last_index[5] = 0;
        s->block[4][0] =
        s->block[5][0] = (1024 + s->c_dc_scale / 2) / s->c_dc_scale;
        if (!s->chroma_y_shift) { /* 422 / 444 */
            for (i=6; i<12; i++) {
                s->block_last_index[i] = 0;
                s->block[i][0] = s->block[4][0];
            }
        }
    }

    // non c quantize code returns incorrect block_last_index FIXME
        for (i = 0; i < mb_block_count; i++) {
            int j;
            if (s->block_last_index[i] > 0) {
                for (j = 63; j > 0; j--) {
                    if (s->block[i][s->intra_scantable.permutated[j]])
                        break;
                }
                s->block_last_index[i] = j;
            }
        }
    }

    /* huffman encode */
    case AV_CODEC_ID_MPEG1VIDEO:
    case AV_CODEC_ID_MPEG2VIDEO:
        if (CONFIG_MPEG1VIDEO_ENCODER || CONFIG_MPEG2VIDEO_ENCODER)
            ff_mpeg1_encode_mb(s, s->block, motion_x, motion_y);
        break;
    case AV_CODEC_ID_MPEG4:
        if (CONFIG_MPEG4_ENCODER)
            ff_mpeg4_encode_mb(s, s->block, motion_x, motion_y);
        break;
    case AV_CODEC_ID_MSMPEG4V2:
    case AV_CODEC_ID_MSMPEG4V3:
    case AV_CODEC_ID_WMV1:
        if (CONFIG_MSMPEG4_ENCODER)
            ff_msmpeg4_encode_mb(s, s->block, motion_x, motion_y);
        break;
    case AV_CODEC_ID_WMV2:
        if (CONFIG_WMV2_ENCODER)
            ff_wmv2_encode_mb(s, s->block, motion_x, motion_y);
        break;
    case AV_CODEC_ID_H261:
        if (CONFIG_H261_ENCODER)
            ff_h261_encode_mb(s, s->block, motion_x, motion_y);
        break;
    case AV_CODEC_ID_H263:
    case AV_CODEC_ID_H263P:
    case AV_CODEC_ID_FLV1:
    case AV_CODEC_ID_RV10:
    case AV_CODEC_ID_RV20:
        if (CONFIG_H263_ENCODER)
            ff_h263_encode_mb(s, s->block, motion_x, motion_y);
        break;
    case AV_CODEC_ID_MJPEG:
    case AV_CODEC_ID_AMV:
        if (CONFIG_MJPEG_ENCODER)
            ff_mjpeg_encode_mb(s, s->block);
        break;
    default:
    }

{
}

static inline void copy_context_before_encode(MpegEncContext *d, MpegEncContext *s, int type){
    int i;

    memcpy(d->last_mv, s->last_mv, 2*2*2*sizeof(int)); //FIXME is memcpy faster than a loop?

    /* MPEG-1 */
    d->mb_skip_run= s->mb_skip_run;
    for(i=0; i<3; i++)
        d->last_dc[i] = s->last_dc[i];

    /* statistics */
    d->mv_bits= s->mv_bits;
    d->i_tex_bits= s->i_tex_bits;
    d->p_tex_bits= s->p_tex_bits;
    d->i_count= s->i_count;
    d->f_count= s->f_count;
    d->b_count= s->b_count;
    d->skip_count= s->skip_count;
    d->misc_bits= s->misc_bits;
    d->last_bits= 0;

    d->mb_skipped= 0;
    d->qscale= s->qscale;
    d->dquant= s->dquant;

    d->esc3_level_length= s->esc3_level_length;
}

static inline void copy_context_after_encode(MpegEncContext *d, MpegEncContext *s, int type){
    int i;

    memcpy(d->mv, s->mv, 2*4*2*sizeof(int));
    memcpy(d->last_mv, s->last_mv, 2*2*2*sizeof(int)); //FIXME is memcpy faster than a loop?

    /* MPEG-1 */
    d->mb_skip_run= s->mb_skip_run;
    for(i=0; i<3; i++)
        d->last_dc[i] = s->last_dc[i];

    /* statistics */
    d->mv_bits= s->mv_bits;
    d->i_tex_bits= s->i_tex_bits;
    d->p_tex_bits= s->p_tex_bits;
    d->i_count= s->i_count;
    d->f_count= s->f_count;
    d->b_count= s->b_count;
    d->skip_count= s->skip_count;
    d->misc_bits= s->misc_bits;

    d->mb_intra= s->mb_intra;
    d->mb_skipped= s->mb_skipped;
    d->mv_type= s->mv_type;
    d->mv_dir= s->mv_dir;
    d->pb= s->pb;
    if(s->data_partitioning){
        d->pb2= s->pb2;
        d->tex_pb= s->tex_pb;
    }
    d->block= s->block;
    for(i=0; i<8; i++)
        d->block_last_index[i]= s->block_last_index[i];
    d->interlaced_dct= s->interlaced_dct;
    d->qscale= s->qscale;

    d->esc3_level_length= s->esc3_level_length;
}

static inline void encode_mb_hq(MpegEncContext *s, MpegEncContext *backup, MpegEncContext *best, int type,
                           PutBitContext pb[2], PutBitContext pb2[2], PutBitContext tex_pb[2],
                           int *dmin, int *next_block, int motion_x, int motion_y)
{
    int score;
    uint8_t *dest_backup[3];

    copy_context_before_encode(s, backup, type);

    s->block= s->blocks[*next_block];
    s->pb= pb[*next_block];
    if(s->data_partitioning){
        s->pb2   = pb2   [*next_block];
        s->tex_pb= tex_pb[*next_block];
    }

    if(*next_block){
        memcpy(dest_backup, s->dest, sizeof(s->dest));
        s->dest[0] = s->sc.rd_scratchpad;
        s->dest[1] = s->sc.rd_scratchpad + 16*s->linesize;
        s->dest[2] = s->sc.rd_scratchpad + 16*s->linesize + 8;
        av_assert0(s->linesize >= 32); //FIXME
    }

    encode_mb(s, motion_x, motion_y);

    score= put_bits_count(&s->pb);
    if(s->data_partitioning){
        score+= put_bits_count(&s->pb2);
        score+= put_bits_count(&s->tex_pb);
    }

    if(s->avctx->mb_decision == FF_MB_DECISION_RD){
        ff_mpv_reconstruct_mb(s, s->block);

        score *= s->lambda2;
        score += sse_mb(s) << FF_LAMBDA_SHIFT;
    }

    if(*next_block){
        memcpy(s->dest, dest_backup, sizeof(s->dest));
    }

    if(score<*dmin){
        *dmin= score;
        *next_block^=1;

        copy_context_after_encode(best, s, type);
    }
}

static int sse(MpegEncContext *s, uint8_t *src1, uint8_t *src2, int w, int h, int stride){
    const uint32_t *sq = ff_square_tab + 256;
    int acc=0;
    int x,y;

    if(w==16 && h==16)
        return s->mecc.sse[0](NULL, src1, src2, stride, 16);
    else if(w==8 && h==8)
        return s->mecc.sse[1](NULL, src1, src2, stride, 8);

    for(y=0; y<h; y++){
        for(x=0; x<w; x++){
            acc+= sq[src1[x + y*stride] - src2[x + y*stride]];
        }
    }

    av_assert2(acc>=0);

    return acc;
}



        return s->mecc.nsse[0](s, s->new_picture.f->data[0] + s->mb_x * 16 + s->mb_y * s->linesize   * 16, s->dest[0], s->linesize,   16) +
               s->mecc.nsse[1](s, s->new_picture.f->data[1] + s->mb_x *  8 + s->mb_y * s->uvlinesize *  8, s->dest[1], s->uvlinesize,  8) +
               s->mecc.nsse[1](s, s->new_picture.f->data[2] + s->mb_x *  8 + s->mb_y * s->uvlinesize *  8, s->dest[2], s->uvlinesize,  8);
      }else{
      }
    else
}

static int pre_estimate_motion_thread(AVCodecContext *c, void *arg){
    MpegEncContext *s= *(void**)arg;


    s->me.pre_pass=1;
    s->me.dia_size= s->avctx->pre_dia_size;
    s->first_slice_line=1;
    for(s->mb_y= s->end_mb_y-1; s->mb_y >= s->start_mb_y; s->mb_y--) {
        for(s->mb_x=s->mb_width-1; s->mb_x >=0 ;s->mb_x--) {
            ff_pre_estimate_p_frame_motion(s, s->mb_x, s->mb_y);
        }
        s->first_slice_line=0;
    }

    s->me.pre_pass=0;

    return 0;
}




            /* compute motion vector & mb_type and store in context */
            else
        }
    }
}





        }
    }
}

        if(s->partitioned_frame){
            ff_mpeg4_merge_partitions(s);
        }

        ff_mpeg4_stuffing(&s->pb);
        ff_mjpeg_encode_stuffing(s);
    }


        s->misc_bits+= get_bits_diff(s);

static void write_mb_info(MpegEncContext *s)
{
    uint8_t *ptr = s->mb_info_ptr + s->mb_info_size - 12;
    int offset = put_bits_count(&s->pb);
    int mba  = s->mb_x + s->mb_width * (s->mb_y % s->gob_index);
    int gobn = s->mb_y / s->gob_index;
    int pred_x, pred_y;
    if (CONFIG_H263_ENCODER)
    bytestream_put_le32(&ptr, offset);
    bytestream_put_byte(&ptr, s->qscale);
    bytestream_put_byte(&ptr, gobn);
    bytestream_put_le16(&ptr, mba);
    bytestream_put_byte(&ptr, pred_x); /* hmv1 */
    bytestream_put_byte(&ptr, pred_y); /* vmv1 */
    /* 4MV not implemented */
    bytestream_put_byte(&ptr, 0); /* hmv2 */
    bytestream_put_byte(&ptr, 0); /* vmv2 */
}

{
        return;
    if (put_bits_count(&s->pb) - s->prev_mb_info*8 >= s->mb_info*8) {
        s->mb_info_size += 12;
        s->prev_mb_info = s->last_mb_info;
    }
    if (startcode) {
        s->prev_mb_info = put_bits_count(&s->pb)/8;
        /* This might have incremented mb_info_size above, and we return without
         * actually writing any info into that slot yet. But in that case,
         * this will be called again at the start of the after writing the
         * start code, actually writing the mb info. */
        return;
    }

    s->last_mb_info = put_bits_count(&s->pb)/8;
    if (!s->mb_info_size)
        s->mb_info_size += 12;
    write_mb_info(s);
}

{


            av_log(s->avctx, AV_LOG_ERROR, "Cannot reallocate putbit buffer\n");
            return AVERROR(ENOMEM);
        }


            return AVERROR(ENOMEM);

    }
        return AVERROR(EINVAL);
    return 0;
}



    }


        /* init last dc values */
        /* note: quant matrix value (8) is implied here */

    }
    }


    case AV_CODEC_ID_H263:
    case AV_CODEC_ID_H263P:
    case AV_CODEC_ID_FLV1:
        if (CONFIG_H263_ENCODER)
            s->gob_index = H263_GOB_HEIGHT(s->height);
        break;
    case AV_CODEC_ID_MPEG4:
        if(CONFIG_MPEG4_ENCODER && s->partitioned_frame)
            ff_mpeg4_init_partitions(s);
        break;
    }



//            int d;

                av_log(s->avctx, AV_LOG_ERROR, "encoded frame too large\n");
                return -1;
            }
                    av_log(s->avctx, AV_LOG_ERROR, "encoded partitioned frame too large\n");
                    return -1;
                }
            }


                ff_h261_reorder_mb_index(s);
                xy= s->mb_y*s->mb_stride + s->mb_x;
                mb_type= s->mb_type[xy];
            }

            /* write gob / video packet header  */




                case AV_CODEC_ID_H263P:
                    break;
                case AV_CODEC_ID_MPEG1VIDEO:
                    break;
                case AV_CODEC_ID_MJPEG:
                    if(s->mb_x==0 && s->mb_y!=0) is_gob_start=1;
                    break;
                }


                            ff_mpeg4_init_partitions(s);
                        }
                    }


                        }
                    }

#if FF_API_RTP_CALLBACK
FF_DISABLE_DEPRECATION_WARNINGS
                        int number_mb = (mb_y - s->resync_mb_y)*s->mb_width + mb_x - s->resync_mb_x;
                        s->avctx->rtp_callback(s->avctx, s->ptr_lastgob, current_packet_size, number_mb);
                    }
FF_ENABLE_DEPRECATION_WARNINGS
#endif

                    case AV_CODEC_ID_MPEG4:
                        if (CONFIG_MPEG4_ENCODER) {
                            ff_mpeg4_encode_video_packet_header(s);
                            ff_mpeg4_clean_buffers(s);
                        }
                    break;
                    case AV_CODEC_ID_MPEG1VIDEO:
                    case AV_CODEC_ID_MPEG2VIDEO:
                        if (CONFIG_MPEG1VIDEO_ENCODER || CONFIG_MPEG2VIDEO_ENCODER) {
                            ff_mpeg1_encode_slice_header(s);
                            ff_mpeg1_clean_buffers(s);
                        }
                    break;
                    case AV_CODEC_ID_H263:
                    case AV_CODEC_ID_H263P:
                        if (CONFIG_H263_ENCODER)
                            ff_h263_encode_gob_header(s, mb_y);
                    break;
                    }

                        int bits= put_bits_count(&s->pb);
                        s->misc_bits+= bits - s->last_bits;
                        s->last_bits= bits;
                    }

                }
            }

            }




                }

                                 &dmin, &next_block, s->mv[0][0][0], s->mv[0][0][1]);
                }
                    }
                                 &dmin, &next_block, 0, 0);
                }
                                 &dmin, &next_block, s->mv[0][0][0], s->mv[0][0][1]);
                }
                    }
                                 &dmin, &next_block, 0, 0);
                }
                                 &dmin, &next_block, s->mv[0][0][0], s->mv[0][0][1]);
                }
                                 &dmin, &next_block, s->mv[1][0][0], s->mv[1][0][1]);
                }
                                 &dmin, &next_block, 0, 0);
                }
                    }
                                 &dmin, &next_block, 0, 0);
                }
                    }
                                 &dmin, &next_block, 0, 0);
                }
                        }
                    }
                                 &dmin, &next_block, 0, 0);
                }
                                 &dmin, &next_block, 0, 0);
                        else
                    }
                }



                        //FIXME intra

                                }
                            }

                                         &dmin, &next_block, s->mv[mvdir][0][0], s->mv[mvdir][0][1]);
                                    }
                                }
                            }
                        }
                    }
                }
                    int mx= s->b_direct_mv_table[xy][0];
                    int my= s->b_direct_mv_table[xy][1];

                    backup_s.dquant = 0;
                    s->mv_dir = MV_DIR_FORWARD | MV_DIR_BACKWARD | MV_DIRECT;
                    s->mb_intra= 0;
                    ff_mpeg4_set_direct_mv(s, mx, my);
                    encode_mb_hq(s, &backup_s, &best_s, CANDIDATE_MB_TYPE_DIRECT, pb, pb2, tex_pb,
                                 &dmin, &next_block, mx, my);
                }
                    backup_s.dquant = 0;
                    s->mv_dir = MV_DIR_FORWARD | MV_DIR_BACKWARD | MV_DIRECT;
                    s->mb_intra= 0;
                    ff_mpeg4_set_direct_mv(s, 0, 0);
                    encode_mb_hq(s, &backup_s, &best_s, CANDIDATE_MB_TYPE_DIRECT, pb, pb2, tex_pb,
                                 &dmin, &next_block, 0, 0);
                }
                    int coded=0;
                    for(i=0; i<6; i++)
                        coded |= s->block_last_index[i];
                    if(coded){
                        int mx,my;
                        memcpy(s->mv, best_s.mv, sizeof(s->mv));
                        if(CONFIG_MPEG4_ENCODER && best_s.mv_dir & MV_DIRECT){
                        }else if(best_s.mv_dir&MV_DIR_BACKWARD){
                            mx= s->mv[1][0][0];
                            my= s->mv[1][0][1];
                        }else{
                        }

                        s->mv_dir= best_s.mv_dir;
                        s->mv_type = best_s.mv_type;
                        s->mb_intra= 0;
/*                        s->mv[0][0][0] = best_s.mv[0][0][0];
                        s->mv[0][0][1] = best_s.mv[0][0][1];
                        s->mv[1][0][0] = best_s.mv[1][0][0];
                        s->mv[1][0][1] = best_s.mv[1][0][1];*/
                        backup_s.dquant= 0;
                        s->skipdct=1;
                        encode_mb_hq(s, &backup_s, &best_s, CANDIDATE_MB_TYPE_INTER /* wrong but unused */, pb, pb2, tex_pb,
                                        &dmin, &next_block, mx, my);
                        s->skipdct=0;
                    }
                }





                }

                    s->out_format == FMT_H263 && s->pict_type!=AV_PICTURE_TYPE_B)
                    ff_h263_update_motion_val(s);

                }

            } else {
                // only one MB-Type possible

                    }
                    break;
                case CANDIDATE_MB_TYPE_INTER4V:
                    s->mv_dir = MV_DIR_FORWARD;
                    s->mv_type = MV_TYPE_8X8;
                    s->mb_intra= 0;
                    for(i=0; i<4; i++){
                        s->mv[0][i][0] = s->current_picture.motion_val[0][s->block_index[i]][0];
                        s->mv[0][i][1] = s->current_picture.motion_val[0][s->block_index[i]][1];
                    }
                    break;
                case CANDIDATE_MB_TYPE_DIRECT:
                    if (CONFIG_MPEG4_ENCODER) {
                        s->mv_dir = MV_DIR_FORWARD|MV_DIR_BACKWARD|MV_DIRECT;
                        s->mb_intra= 0;
                        motion_x=s->b_direct_mv_table[xy][0];
                        motion_y=s->b_direct_mv_table[xy][1];
                        ff_mpeg4_set_direct_mv(s, motion_x, motion_y);
                    }
                    break;
                case CANDIDATE_MB_TYPE_DIRECT0:
                    if (CONFIG_MPEG4_ENCODER) {
                        s->mv_dir = MV_DIR_FORWARD|MV_DIR_BACKWARD|MV_DIRECT;
                        s->mb_intra= 0;
                        ff_mpeg4_set_direct_mv(s, 0, 0);
                    }
                    break;
                    }
                    break;
                    }
                    break;
                case CANDIDATE_MB_TYPE_BIDIR_I:
                    s->mv_dir = MV_DIR_FORWARD | MV_DIR_BACKWARD;
                    s->mv_type = MV_TYPE_FIELD;
                    s->mb_intra= 0;
                    for(dir=0; dir<2; dir++){
                        for(i=0; i<2; i++){
                            j= s->field_select[dir][i] = s->b_field_select_table[dir][i][xy];
                            s->mv[dir][i][0] = s->b_field_mv_table[dir][i][j][xy][0];
                            s->mv[dir][i][1] = s->b_field_mv_table[dir][i][j][xy][1];
                        }
                    }
                    break;
                default:
                    av_log(s->avctx, AV_LOG_ERROR, "illegal MB type\n");
                }


                // RAL: Update last macroblock type

                    s->out_format == FMT_H263 && s->pict_type!=AV_PICTURE_TYPE_B)
                    ff_h263_update_motion_val(s);

            }

            /* clean the MV table in IPS frames for direct mode in B-frames */
            }

                int w= 16;
                int h= 16;

                if(s->mb_x*16 + 16 > s->width ) w= s->width - s->mb_x*16;
                if(s->mb_y*16 + 16 > s->height) h= s->height- s->mb_y*16;

                s->current_picture.encoding_error[0] += sse(
                    s, s->new_picture.f->data[0] + s->mb_x*16 + s->mb_y*s->linesize*16,
                    s->dest[0], w, h, s->linesize);
                s->current_picture.encoding_error[1] += sse(
                    s, s->new_picture.f->data[1] + s->mb_x*8  + s->mb_y*s->uvlinesize*chr_h,
                    s->dest[1], w>>1, h>>s->chroma_y_shift, s->uvlinesize);
                s->current_picture.encoding_error[2] += sse(
                    s, s->new_picture.f->data[2] + s->mb_x*8  + s->mb_y*s->uvlinesize*chr_h,
                    s->dest[2], w>>1, h>>s->chroma_y_shift, s->uvlinesize);
            }
                if(CONFIG_H263_ENCODER && s->out_format == FMT_H263)
                    ff_h263_loop_filter(s);
            }
                    s->mb_x + s->mb_y * s->mb_stride, put_bits_count(&s->pb));
        }
    }

    //not beautiful here but we must write it before flushing so it has to be here
        ff_msmpeg4_encode_ext_header(s);


#if FF_API_RTP_CALLBACK
FF_DISABLE_DEPRECATION_WARNINGS
    /* Send the last GOB if RTP */
        int number_mb = (mb_y - s->resync_mb_y)*s->mb_width - s->resync_mb_x;
        int pdif = put_bits_ptr(&s->pb) - s->ptr_lastgob;
        /* Call the RTP callback to send the last GOB */
        emms_c();
        s->avctx->rtp_callback(s->avctx, s->ptr_lastgob, pdif, number_mb);
    }
FF_ENABLE_DEPRECATION_WARNINGS
#endif

    return 0;
}

#define MERGE(field) dst->field += src->field; src->field=0
}



        for(i=0; i<64; i++){
            MERGE(dct_error_sum[0][i]);
            MERGE(dct_error_sum[1][i]);
        }
    }


            return -1;
    }

        case AV_CODEC_ID_MPEG4:
            if (CONFIG_MPEG4_ENCODER)
                ff_clean_mpeg4_qscales(s);
            break;
        case AV_CODEC_ID_H263:
        case AV_CODEC_ID_H263P:
        case AV_CODEC_ID_FLV1:
        }

        //FIXME broken
    }else
}

/* must be called before writing the header */

    }else{
        av_assert1(s->picture_number==0 || s->pp_time > 0);
    }
}

{


    /* Reset the average MB variance */

    /* we need to initialize some time vars before we can encode B-frames */
    // RAL: Condition added for MPEG1VIDEO
        ff_set_mpeg4_time(s);


//    s->lambda= s->current_picture_ptr->quality; //FIXME qscale / ... stuff for ME rate distortion

    }

        if (estimate_qp(s,1) < 0)
            return -1;
        ff_get_2pass_fcode(s);
        else
    }

    }

            return ret;
    }

        return -1;

    /* Estimate motion for every MB */
                s->me_pre == 2) {
                s->avctx->execute(s->avctx, pre_estimate_motion_thread, &s->thread_context[0], NULL, context_count, sizeof(void*));
            }
        }

    }else /* if(s->pict_type == AV_PICTURE_TYPE_I) */{
        /* I-Frame */

            /* finding spatial complexity for I-frame rate control */
        }
    }
    }

                s->current_picture.mb_var_sum, s->current_picture.mc_mb_var_sum);
    }


            }

                int j;
                }
            }
        }




                int dir, j;
                                            s->b_field_mv_table[dir][i][j], dir ? s->b_code : s->f_code, type, 1);
                        }
                    }
                }
            }
        }
    }

        return -1;

        s->pict_type == AV_PICTURE_TYPE_I &&
        !(s->avctx->flags & AV_CODEC_FLAG_QSCALE))
        s->qscale= 3; //reduce clipping problems


            chroma_matrix =
            luma_matrix = s->avctx->intra_matrix;
        }
            chroma_matrix = s->avctx->chroma_intra_matrix;

        /* for mjpeg, we do include qscale in the matrix */

        }
    }
        static const uint8_t y[32]={13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13};
        static const uint8_t c[32]={14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14};

        }
    }

    //FIXME var duplication


    case FMT_MJPEG:
        if (CONFIG_MJPEG_ENCODER && s->huffman != HUFFMAN_TABLE_OPTIMAL)
            ff_mjpeg_encode_picture_header(s->avctx, &s->pb, &s->intra_scantable,
                                           s->pred, s->intra_matrix, s->chroma_intra_matrix);
        break;
    case FMT_H261:
        if (CONFIG_H261_ENCODER)
            ff_h261_encode_picture_header(s, picture_number);
        break;
    case FMT_H263:
        if (CONFIG_WMV2_ENCODER && s->codec_id == AV_CODEC_ID_WMV2)
            ff_wmv2_encode_picture_header(s, picture_number);
        else if (CONFIG_MSMPEG4_ENCODER && s->msmpeg4_version)
            ff_msmpeg4_encode_picture_header(s, picture_number);
        else if (CONFIG_MPEG4_ENCODER && s->h263_pred) {
            ret = ff_mpeg4_encode_picture_header(s, picture_number);
            if (ret < 0)
                return ret;
        } else if (CONFIG_RV10_ENCODER && s->codec_id == AV_CODEC_ID_RV10) {
            ret = ff_rv10_encode_picture_header(s, picture_number);
            if (ret < 0)
                return ret;
        }
        else if (CONFIG_RV20_ENCODER && s->codec_id == AV_CODEC_ID_RV20)
            ff_rv20_encode_picture_header(s, picture_number);
        else if (CONFIG_FLV_ENCODER && s->codec_id == AV_CODEC_ID_FLV1)
            ff_flv_encode_picture_header(s, picture_number);
        else if (CONFIG_H263_ENCODER)
            ff_h263_encode_picture_header(s, picture_number);
        break;
    case FMT_MPEG1:
        if (CONFIG_MPEG1VIDEO_ENCODER || CONFIG_MPEG2VIDEO_ENCODER)
            ff_mpeg1_encode_picture_header(s, picture_number);
        break;
    default:
        av_assert0(0);
    }

    }
    }
}




            }else{
            }
        }
    }

                                  int16_t *block, int n,
                                  int qscale, int *overflow){


        s->denoise_dct(s, block);


            else
        } else{
            /* For AIC we skip quant/dequant of INTRADC */
            q = 1 << 3;
            qadd=0;
        }

        /* note: block[0] is assumed to be positive */

        } else {
        }
    } else {
    }



            last_non_zero = i;
            break;
        }
    }


//        if(   bias+level >= (1<<(QMAT_SHIFT - 3))
//           || bias-level >= (1<<(QMAT_SHIFT - 3))){
//                coeff[2][k]= level-2;
            }else{
//                coeff[2][k]= -level+2;
            }
        }else{
        }
    }


    }






            }else{ // MPEG-1
                }else{
                }
            }


                    }
                }

                        }
                    }
                }
            }else{

                    }
                }

                        }
                    }
                }
            }
        }


        // Note: there is a vlc code in MPEG-4 which is 1 bit shorter then another one with a shorter run and the same level
                    break;
            }
        }else{
                    break;
            }
        }

    }


            }
        }
    }



        return last_non_zero;



            } else{ // MPEG-1
            }


            }
        }
    }



    }

    return last_non_zero;
}

static int16_t basis[64][64];

static void build_basis(uint8_t *perm){
    int i, j, x, y;
    emms_c();
    for(i=0; i<8; i++){
        for(j=0; j<8; j++){
            for(y=0; y<8; y++){
                for(x=0; x<8; x++){
                    double s= 0.25*(1<<BASIS_SHIFT);
                    int index= 8*i + j;
                    int perm_index= perm[index];
                    if(i==0) s*= sqrt(0.5);
                    if(j==0) s*= sqrt(0.5);
                    basis[perm_index][8*x + y]= lrintf(s * cos((M_PI/8.0)*i*(x+0.5)) * cos((M_PI/8.0)*j*(y+0.5)));
                }
            }
        }
    }
}

static int dct_quantize_refine(MpegEncContext *s, //FIXME breaks denoise?
                        int16_t *block, int16_t *weight, int16_t *orig,
                        int n, int qscale){
    int16_t rem[64];
    LOCAL_ALIGNED_16(int16_t, d1, [64]);
    const uint8_t *scantable;
    const uint8_t *perm_scantable;
//    unsigned int threshold1, threshold2;
//    int bias=0;
    int run_tab[65];
    int prev_run=0;
    int prev_level=0;
    int qmul, qadd, start_i, last_non_zero, i, dc;
    uint8_t * length;
    uint8_t * last_length;
    int lambda;
    int rle_index, run, q = 1, sum; //q is only used when s->mb_intra is true

    if(basis[0][0] == 0)
        build_basis(s->idsp.idct_permutation);

    qmul= qscale*2;
    qadd= (qscale-1)|1;
    if (s->mb_intra) {
        scantable= s->intra_scantable.scantable;
        perm_scantable= s->intra_scantable.permutated;
        if (!s->h263_aic) {
            if (n < 4)
                q = s->y_dc_scale;
            else
                q = s->c_dc_scale;
        } else{
            /* For AIC we skip quant/dequant of INTRADC */
            q = 1;
            qadd=0;
        }
        q <<= RECON_SHIFT-3;
        /* note: block[0] is assumed to be positive */
        dc= block[0]*q;
//        block[0] = (block[0] + (q >> 1)) / q;
        start_i = 1;
//        if(s->mpeg_quant || s->out_format == FMT_MPEG1)
//            bias= 1<<(QMAT_SHIFT-1);
        if (n > 3 && s->intra_chroma_ac_vlc_length) {
            length     = s->intra_chroma_ac_vlc_length;
            last_length= s->intra_chroma_ac_vlc_last_length;
        } else {
            length     = s->intra_ac_vlc_length;
            last_length= s->intra_ac_vlc_last_length;
        }
    } else {
        scantable= s->inter_scantable.scantable;
        perm_scantable= s->inter_scantable.permutated;
        dc= 0;
        start_i = 0;
        length     = s->inter_ac_vlc_length;
        last_length= s->inter_ac_vlc_last_length;
    }
    last_non_zero = s->block_last_index[n];

    dc += (1<<(RECON_SHIFT-1));
    for(i=0; i<64; i++){
        rem[i] = dc - (orig[i] << RECON_SHIFT); // FIXME use orig directly instead of copying to rem[]
    }

    sum=0;
    for(i=0; i<64; i++){
        int one= 36;
        int qns=4;
        int w;

        w= FFABS(weight[i]) + qns*one;
        w= 15 + (48*qns*one + w/2)/w; // 16 .. 63

        weight[i] = w;
//        w=weight[i] = (63*qns + (w/2)) / w;

        av_assert2(w>0);
        av_assert2(w<(1<<6));
        sum += w*w;
    }
    lambda= sum*(uint64_t)s->lambda2 >> (FF_LAMBDA_SHIFT - 6 + 6 + 6 + 6);

    run=0;
    rle_index=0;
    for(i=start_i; i<=last_non_zero; i++){
        int j= perm_scantable[i];
        const int level= block[j];
        int coeff;

        if(level){
            if(level<0) coeff= qmul*level - qadd;
            else        coeff= qmul*level + qadd;
            run_tab[rle_index++]=run;
            run=0;

            s->mpvencdsp.add_8x8basis(rem, basis[j], coeff);
        }else{
            run++;
        }
    }

    for(;;){
        int best_score = s->mpvencdsp.try_8x8basis(rem, weight, basis[0], 0);
        int best_coeff=0;
        int best_change=0;
        int run2, best_unquant_change=0, analyze_gradient;
        analyze_gradient = last_non_zero > 2 || s->quantizer_noise_shaping >= 3;

        if(analyze_gradient){
            for(i=0; i<64; i++){
                int w= weight[i];

                d1[i] = (rem[i]*w*w + (1<<(RECON_SHIFT+12-1)))>>(RECON_SHIFT+12);
            }
            s->fdsp.fdct(d1);
        }

        if(start_i){
            const int level= block[0];
            int change, old_coeff;

            av_assert2(s->mb_intra);

            old_coeff= q*level;

            for(change=-1; change<=1; change+=2){
                int new_level= level + change;
                int score, new_coeff;

                new_coeff= q*new_level;
                if(new_coeff >= 2048 || new_coeff < 0)
                    continue;

                score = s->mpvencdsp.try_8x8basis(rem, weight, basis[0],
                                                  new_coeff - old_coeff);
                if(score<best_score){
                    best_score= score;
                    best_coeff= 0;
                    best_change= change;
                    best_unquant_change= new_coeff - old_coeff;
                }
            }
        }

        run=0;
        rle_index=0;
        run2= run_tab[rle_index++];
        prev_level=0;
        prev_run=0;

        for(i=start_i; i<64; i++){
            int j= perm_scantable[i];
            const int level= block[j];
            int change, old_coeff;

            if(s->quantizer_noise_shaping < 3 && i > last_non_zero + 1)
                break;

            if(level){
                if(level<0) old_coeff= qmul*level - qadd;
                else        old_coeff= qmul*level + qadd;
                run2= run_tab[rle_index++]; //FIXME ! maybe after last
            }else{
                old_coeff=0;
                run2--;
                av_assert2(run2>=0 || i >= last_non_zero );
            }

            for(change=-1; change<=1; change+=2){
                int new_level= level + change;
                int score, new_coeff, unquant_change;

                score=0;
                if(s->quantizer_noise_shaping < 2 && FFABS(new_level) > FFABS(level))
                   continue;

                if(new_level){
                    if(new_level<0) new_coeff= qmul*new_level - qadd;
                    else            new_coeff= qmul*new_level + qadd;
                    if(new_coeff >= 2048 || new_coeff <= -2048)
                        continue;
                    //FIXME check for overflow

                    if(level){
                        if(level < 63 && level > -63){
                            if(i < last_non_zero)
                                score +=   length[UNI_AC_ENC_INDEX(run, new_level+64)]
                                         - length[UNI_AC_ENC_INDEX(run, level+64)];
                            else
                                score +=   last_length[UNI_AC_ENC_INDEX(run, new_level+64)]
                                         - last_length[UNI_AC_ENC_INDEX(run, level+64)];
                        }
                    }else{
                        av_assert2(FFABS(new_level)==1);

                        if(analyze_gradient){
                            int g= d1[ scantable[i] ];
                            if(g && (g^new_level) >= 0)
                                continue;
                        }

                        if(i < last_non_zero){
                            int next_i= i + run2 + 1;
                            int next_level= block[ perm_scantable[next_i] ] + 64;

                            if(next_level&(~127))
                                next_level= 0;

                            if(next_i < last_non_zero)
                                score +=   length[UNI_AC_ENC_INDEX(run, 65)]
                                         + length[UNI_AC_ENC_INDEX(run2, next_level)]
                                         - length[UNI_AC_ENC_INDEX(run + run2 + 1, next_level)];
                            else
                                score +=  length[UNI_AC_ENC_INDEX(run, 65)]
                                        + last_length[UNI_AC_ENC_INDEX(run2, next_level)]
                                        - last_length[UNI_AC_ENC_INDEX(run + run2 + 1, next_level)];
                        }else{
                            score += last_length[UNI_AC_ENC_INDEX(run, 65)];
                            if(prev_level){
                                score +=  length[UNI_AC_ENC_INDEX(prev_run, prev_level)]
                                        - last_length[UNI_AC_ENC_INDEX(prev_run, prev_level)];
                            }
                        }
                    }
                }else{
                    new_coeff=0;
                    av_assert2(FFABS(level)==1);

                    if(i < last_non_zero){
                        int next_i= i + run2 + 1;
                        int next_level= block[ perm_scantable[next_i] ] + 64;

                        if(next_level&(~127))
                            next_level= 0;

                        if(next_i < last_non_zero)
                            score +=   length[UNI_AC_ENC_INDEX(run + run2 + 1, next_level)]
                                     - length[UNI_AC_ENC_INDEX(run2, next_level)]
                                     - length[UNI_AC_ENC_INDEX(run, 65)];
                        else
                            score +=   last_length[UNI_AC_ENC_INDEX(run + run2 + 1, next_level)]
                                     - last_length[UNI_AC_ENC_INDEX(run2, next_level)]
                                     - length[UNI_AC_ENC_INDEX(run, 65)];
                    }else{
                        score += -last_length[UNI_AC_ENC_INDEX(run, 65)];
                        if(prev_level){
                            score +=  last_length[UNI_AC_ENC_INDEX(prev_run, prev_level)]
                                    - length[UNI_AC_ENC_INDEX(prev_run, prev_level)];
                        }
                    }
                }

                score *= lambda;

                unquant_change= new_coeff - old_coeff;
                av_assert2((score < 100*lambda && score > -100*lambda) || lambda==0);

                score += s->mpvencdsp.try_8x8basis(rem, weight, basis[j],
                                                   unquant_change);
                if(score<best_score){
                    best_score= score;
                    best_coeff= i;
                    best_change= change;
                    best_unquant_change= unquant_change;
                }
            }
            if(level){
                prev_level= level + 64;
                if(prev_level&(~127))
                    prev_level= 0;
                prev_run= run;
                run=0;
            }else{
                run++;
            }
        }

        if(best_change){
            int j= perm_scantable[ best_coeff ];

            block[j] += best_change;

            if(best_coeff > last_non_zero){
                last_non_zero= best_coeff;
                av_assert2(block[j]);
            }else{
                for(; last_non_zero>=start_i; last_non_zero--){
                    if(block[perm_scantable[last_non_zero]])
                        break;
                }
            }

            run=0;
            rle_index=0;
            for(i=start_i; i<=last_non_zero; i++){
                int j= perm_scantable[i];
                const int level= block[j];

                 if(level){
                     run_tab[rle_index++]=run;
                     run=0;
                 }else{
                     run++;
                 }
            }

            s->mpvencdsp.add_8x8basis(rem, basis[j], best_unquant_change);
        }else{
            break;
        }
    }

    return last_non_zero;
}

/**
 * Permute an 8x8 block according to permutation.
 * @param block the block which will be permuted according to
 *              the given permutation vector
 * @param permutation the permutation vector
 * @param last the last non zero coefficient in scantable order, used to
 *             speed the permutation up
 * @param scantable the used scantable, this is only used to speed the
 *                  permutation up, the block is not (inverse) permutated
 *                  to scantable order!
 */
                      const uint8_t *scantable, int last)
{

    //FIXME it is ok but not clean and might fail for some permutations
    // if (permutation[1] == 1)
    // return;

    }

    }
}

                        int16_t *block, int n,
                        int qscale, int *overflow)
{



            else
        } else
            /* For AIC we skip quant/dequant of INTRADC */
            q = 1 << 3;

        /* note: block[0] is assumed to be positive */
    } else {
    }

            last_non_zero = i;
            break;
        }else{
        }
    }

//        if(   bias+level >= (1<<QMAT_SHIFT)
//           || bias-level >= (1<<QMAT_SHIFT)){
            }else{
            }
        }else{
        }
    }

    /* we need this permutation so that we correct the IDCT, we only permute the !=0 elements */
                      scantable, last_non_zero);

}

#define OFFSET(x) offsetof(MpegEncContext, x)
#define VE AV_OPT_FLAG_VIDEO_PARAM | AV_OPT_FLAG_ENCODING_PARAM
static const AVOption h263_options[] = {
    { "obmc",         "use overlapped block motion compensation.", OFFSET(obmc), AV_OPT_TYPE_BOOL, { .i64 = 0 }, 0, 1, VE },
    { "mb_info",      "emit macroblock info for RFC 2190 packetization, the parameter value is the maximum payload size", OFFSET(mb_info), AV_OPT_TYPE_INT, { .i64 = 0 }, 0, INT_MAX, VE },
    FF_MPV_COMMON_OPTS
    { NULL },
};

static const AVClass h263_class = {
    .class_name = "H.263 encoder",
    .item_name  = av_default_item_name,
    .option     = h263_options,
    .version    = LIBAVUTIL_VERSION_INT,
};

AVCodec ff_h263_encoder = {
    .name           = "h263",
    .long_name      = NULL_IF_CONFIG_SMALL("H.263 / H.263-1996"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_H263,
    .priv_data_size = sizeof(MpegEncContext),
    .init           = ff_mpv_encode_init,
    .encode2        = ff_mpv_encode_picture,
    .close          = ff_mpv_encode_end,
    .caps_internal  = FF_CODEC_CAP_INIT_CLEANUP,
    .pix_fmts= (const enum AVPixelFormat[]){AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE},
    .priv_class     = &h263_class,
};

static const AVOption h263p_options[] = {
    { "umv",        "Use unlimited motion vectors.",    OFFSET(umvplus),       AV_OPT_TYPE_BOOL, { .i64 = 0 }, 0, 1, VE },
    { "aiv",        "Use alternative inter VLC.",       OFFSET(alt_inter_vlc), AV_OPT_TYPE_BOOL, { .i64 = 0 }, 0, 1, VE },
    { "obmc",       "use overlapped block motion compensation.", OFFSET(obmc), AV_OPT_TYPE_BOOL, { .i64 = 0 }, 0, 1, VE },
    { "structured_slices", "Write slice start position at every GOB header instead of just GOB number.", OFFSET(h263_slice_structured), AV_OPT_TYPE_BOOL, { .i64 = 0 }, 0, 1, VE},
    FF_MPV_COMMON_OPTS
    { NULL },
};
static const AVClass h263p_class = {
    .class_name = "H.263p encoder",
    .item_name  = av_default_item_name,
    .option     = h263p_options,
    .version    = LIBAVUTIL_VERSION_INT,
};

AVCodec ff_h263p_encoder = {
    .name           = "h263p",
    .long_name      = NULL_IF_CONFIG_SMALL("H.263+ / H.263-1998 / H.263 version 2"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_H263P,
    .priv_data_size = sizeof(MpegEncContext),
    .init           = ff_mpv_encode_init,
    .encode2        = ff_mpv_encode_picture,
    .close          = ff_mpv_encode_end,
    .capabilities   = AV_CODEC_CAP_SLICE_THREADS,
    .caps_internal  = FF_CODEC_CAP_INIT_CLEANUP,
    .pix_fmts       = (const enum AVPixelFormat[]){ AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE },
    .priv_class     = &h263p_class,
};

static const AVClass msmpeg4v2_class = {
    .class_name = "msmpeg4v2 encoder",
    .item_name  = av_default_item_name,
    .option     = ff_mpv_generic_options,
    .version    = LIBAVUTIL_VERSION_INT,
};

AVCodec ff_msmpeg4v2_encoder = {
    .name           = "msmpeg4v2",
    .long_name      = NULL_IF_CONFIG_SMALL("MPEG-4 part 2 Microsoft variant version 2"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_MSMPEG4V2,
    .priv_data_size = sizeof(MpegEncContext),
    .init           = ff_mpv_encode_init,
    .encode2        = ff_mpv_encode_picture,
    .close          = ff_mpv_encode_end,
    .caps_internal  = FF_CODEC_CAP_INIT_CLEANUP,
    .pix_fmts       = (const enum AVPixelFormat[]){ AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE },
    .priv_class     = &msmpeg4v2_class,
};

static const AVClass msmpeg4v3_class = {
    .class_name = "msmpeg4v3 encoder",
    .item_name  = av_default_item_name,
    .option     = ff_mpv_generic_options,
    .version    = LIBAVUTIL_VERSION_INT,
};

AVCodec ff_msmpeg4v3_encoder = {
    .name           = "msmpeg4",
    .long_name      = NULL_IF_CONFIG_SMALL("MPEG-4 part 2 Microsoft variant version 3"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_MSMPEG4V3,
    .priv_data_size = sizeof(MpegEncContext),
    .init           = ff_mpv_encode_init,
    .encode2        = ff_mpv_encode_picture,
    .close          = ff_mpv_encode_end,
    .caps_internal  = FF_CODEC_CAP_INIT_CLEANUP,
    .pix_fmts       = (const enum AVPixelFormat[]){ AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE },
    .priv_class     = &msmpeg4v3_class,
};

static const AVClass wmv1_class = {
    .class_name = "wmv1 encoder",
    .item_name  = av_default_item_name,
    .option     = ff_mpv_generic_options,
    .version    = LIBAVUTIL_VERSION_INT,
};

AVCodec ff_wmv1_encoder = {
    .name           = "wmv1",
    .long_name      = NULL_IF_CONFIG_SMALL("Windows Media Video 7"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_WMV1,
    .priv_data_size = sizeof(MpegEncContext),
    .init           = ff_mpv_encode_init,
    .encode2        = ff_mpv_encode_picture,
    .close          = ff_mpv_encode_end,
    .caps_internal  = FF_CODEC_CAP_INIT_CLEANUP,
    .pix_fmts       = (const enum AVPixelFormat[]){ AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE },
    .priv_class     = &wmv1_class,
};
