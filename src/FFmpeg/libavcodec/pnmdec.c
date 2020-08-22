/*
 * PNM image format
 * Copyright (c) 2002, 2003 Fabrice Bellard
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

#include "avcodec.h"
#include "internal.h"
#include "put_bits.h"
#include "pnm.h"

{
    } else {
        int i;
        }
    }

                            int *got_frame, AVPacket *avpkt)
{


        return ret;

        return ret;

    default:
        return AVERROR(EINVAL);
    case AV_PIX_FMT_RGBA64:
        n = avctx->width * 8;
        components=4;
        sample_len=16;
        if (s->maxval < 65535)
            upgrade = 2;
        goto do_read;
            upgrade = 2;
            upgrade = 1;
            upgrade = 1;
    case AV_PIX_FMT_GRAY8A:
        n = avctx->width * 2;
        components=2;
        sample_len=8;
        goto do_read;
            upgrade = 2;
    case AV_PIX_FMT_YA16:
        n =  avctx->width * 4;
        components=2;
        sample_len=16;
        if (s->maxval < 65535)
            upgrade = 2;
        goto do_read;
    case AV_PIX_FMT_MONOBLACK:
            return AVERROR_INVALIDDATA;
                    while(s->bytestream < s->bytestream_end && (*s->bytestream < '0' || *s->bytestream > '9' ))
                        s->bytestream++;
                        return AVERROR_INVALIDDATA;
                        /* read a single digit */
                    } else {
                        /* read a sequence of digits */
                        for (k = 0; k < 6 && c <= 9; k += 1) {
                            v = 10*v + c;
                            c = (*s->bytestream++) - '0';
                        }
                        if (v > s->maxval) {
                            av_log(avctx, AV_LOG_ERROR, "value %d larger than maxval %d\n", v, s->maxval);
                            return AVERROR_INVALIDDATA;
                        }
                    }
                        ((uint16_t*)ptr)[j] = (((1<<sample_len)-1)*v + (s->maxval>>1))/s->maxval;
                    } else
                }
            }
        }else{
            else if (upgrade == 1) {
                unsigned int j, f = (255 * 128 + s->maxval / 2) / s->maxval;
                for (j = 0; j < n; j++)
                    ptr[j] = (s->bytestream[j] * f + 64) >> 7;
            } else if (upgrade == 2) {
                unsigned int j, v, f = (65535 * 32768 + s->maxval / 2) / s->maxval;
                for (j = 0; j < n / 2; j++) {
                    v = AV_RB16(s->bytestream + 2*j);
                    ((uint16_t *)ptr)[j] = (v * f + 16384) >> 15;
                }
            }
        }
        }
        break;
    case AV_PIX_FMT_YUV420P9:
    case AV_PIX_FMT_YUV420P10:
        {

                n *= 2;
                return AVERROR_INVALIDDATA;
            }
            }
        }
        break;
    case AV_PIX_FMT_YUV420P16:
        {
            uint16_t *ptr1, *ptr2;
            const int f = (65535 * 32768 + s->maxval / 2) / s->maxval;
            unsigned int j, v;

            n        = avctx->width * 2;
            ptr      = p->data[0];
            linesize = p->linesize[0];
            if (n * avctx->height * 3 / 2 > s->bytestream_end - s->bytestream)
                return AVERROR_INVALIDDATA;
            for (i = 0; i < avctx->height; i++) {
                for (j = 0; j < n / 2; j++) {
                    v = AV_RB16(s->bytestream + 2*j);
                    ((uint16_t *)ptr)[j] = (v * f + 16384) >> 15;
                }
                s->bytestream += n;
                ptr           += linesize;
            }
            ptr1 = (uint16_t*)p->data[1];
            ptr2 = (uint16_t*)p->data[2];
            n >>= 1;
            h = avctx->height >> 1;
            for (i = 0; i < h; i++) {
                for (j = 0; j < n / 2; j++) {
                    v = AV_RB16(s->bytestream + 2*j);
                    ptr1[j] = (v * f + 16384) >> 15;
                }
                s->bytestream += n;

                for (j = 0; j < n / 2; j++) {
                    v = AV_RB16(s->bytestream + 2*j);
                    ptr2[j] = (v * f + 16384) >> 15;
                }
                s->bytestream += n;

                ptr1 += p->linesize[1] / 2;
                ptr2 += p->linesize[2] / 2;
            }
        }
        break;
    case AV_PIX_FMT_GBRPF32:
        if (avctx->width * avctx->height * 12 > s->bytestream_end - s->bytestream)
            return AVERROR_INVALIDDATA;
        scale = 1.f / s->scale;
        if (s->endian) {
            float *r, *g, *b;

            r = (float *)p->data[2];
            g = (float *)p->data[0];
            b = (float *)p->data[1];
            for (int i = 0; i < avctx->height; i++) {
                for (int j = 0; j < avctx->width; j++) {
                    r[j] = av_int2float(AV_RL32(s->bytestream+0)) * scale;
                    g[j] = av_int2float(AV_RL32(s->bytestream+4)) * scale;
                    b[j] = av_int2float(AV_RL32(s->bytestream+8)) * scale;
                    s->bytestream += 12;
                }

                r += p->linesize[2] / 4;
                g += p->linesize[0] / 4;
                b += p->linesize[1] / 4;
            }
        } else {
            float *r, *g, *b;

            r = (float *)p->data[2];
            g = (float *)p->data[0];
            b = (float *)p->data[1];
            for (int i = 0; i < avctx->height; i++) {
                for (int j = 0; j < avctx->width; j++) {
                    r[j] = av_int2float(AV_RB32(s->bytestream+0)) * scale;
                    g[j] = av_int2float(AV_RB32(s->bytestream+4)) * scale;
                    b[j] = av_int2float(AV_RB32(s->bytestream+8)) * scale;
                    s->bytestream += 12;
                }

                r += p->linesize[2] / 4;
                g += p->linesize[0] / 4;
                b += p->linesize[1] / 4;
            }
        }
        break;
    }

}


#if CONFIG_PGM_DECODER
AVCodec ff_pgm_decoder = {
    .name           = "pgm",
    .long_name      = NULL_IF_CONFIG_SMALL("PGM (Portable GrayMap) image"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_PGM,
    .priv_data_size = sizeof(PNMContext),
    .decode         = pnm_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
#endif

#if CONFIG_PGMYUV_DECODER
AVCodec ff_pgmyuv_decoder = {
    .name           = "pgmyuv",
    .long_name      = NULL_IF_CONFIG_SMALL("PGMYUV (Portable GrayMap YUV) image"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_PGMYUV,
    .priv_data_size = sizeof(PNMContext),
    .decode         = pnm_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
#endif

#if CONFIG_PPM_DECODER
AVCodec ff_ppm_decoder = {
    .name           = "ppm",
    .long_name      = NULL_IF_CONFIG_SMALL("PPM (Portable PixelMap) image"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_PPM,
    .priv_data_size = sizeof(PNMContext),
    .decode         = pnm_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
#endif

#if CONFIG_PBM_DECODER
AVCodec ff_pbm_decoder = {
    .name           = "pbm",
    .long_name      = NULL_IF_CONFIG_SMALL("PBM (Portable BitMap) image"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_PBM,
    .priv_data_size = sizeof(PNMContext),
    .decode         = pnm_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
#endif

#if CONFIG_PAM_DECODER
AVCodec ff_pam_decoder = {
    .name           = "pam",
    .long_name      = NULL_IF_CONFIG_SMALL("PAM (Portable AnyMap) image"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_PAM,
    .priv_data_size = sizeof(PNMContext),
    .decode         = pnm_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
#endif

#if CONFIG_PFM_DECODER
AVCodec ff_pfm_decoder = {
    .name           = "pfm",
    .long_name      = NULL_IF_CONFIG_SMALL("PFM (Portable FloatMap) image"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_PFM,
    .priv_data_size = sizeof(PNMContext),
    .decode         = pnm_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
#endif
