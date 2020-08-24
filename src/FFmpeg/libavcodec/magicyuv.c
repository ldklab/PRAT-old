/*
 * MagicYUV decoder
 * Copyright (c) 2016 Paul B Mahol
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

#include <stdlib.h>
#include <string.h>

#include "libavutil/pixdesc.h"
#include "libavutil/qsort.h"

#include "avcodec.h"
#include "bytestream.h"
#include "get_bits.h"
#include "huffyuvdsp.h"
#include "internal.h"
#include "lossless_videodsp.h"
#include "thread.h"

typedef struct Slice {
    uint32_t start;
    uint32_t size;
} Slice;

typedef enum Prediction {
    LEFT = 1,
    GRADIENT,
    MEDIAN,
} Prediction;

typedef struct HuffEntry {
    uint16_t sym;
    uint8_t  len;
    uint32_t code;
} HuffEntry;

typedef struct MagicYUVContext {
    AVFrame          *p;
    int               max;
    int               bps;
    int               slice_height;
    int               nb_slices;
    int               planes;         // number of encoded planes in bitstream
    int               decorrelate;    // postprocessing work
    int               color_matrix;   // video color matrix
    int               flags;
    int               interlaced;     // video is interlaced
    uint8_t          *buf;            // pointer to AVPacket->data
    int               hshift[4];
    int               vshift[4];
    Slice            *slices[4];      // slice bitstream positions for each plane
    unsigned int      slices_size[4]; // slice sizes for each plane
    uint8_t           len[4][4096];   // table of code lengths for each plane
    VLC               vlc[4];         // VLC for each plane
    int (*huff_build)(VLC *vlc, uint8_t *len);
    int (*magy_decode_slice)(AVCodecContext *avctx, void *tdata,
                             int j, int threadnr);
    LLVidDSPContext   llviddsp;
} MagicYUVContext;

{
}

static int huff_cmp_len10(const void *a, const void *b)
{
    const HuffEntry *aa = a, *bb = b;
    return (aa->len - bb->len) * 1024 + aa->sym - bb->sym;
}

static int huff_cmp_len12(const void *a, const void *b)
{
    const HuffEntry *aa = a, *bb = b;
    return (aa->len - bb->len) * 4096 + aa->sym - bb->sym;
}

static int huff_build10(VLC *vlc, uint8_t *len)
{
    HuffEntry he[1024];
    uint32_t codes[1024];
    uint8_t bits[1024];
    uint16_t syms[1024];
    uint32_t code;
    int i;

    for (i = 0; i < 1024; i++) {
        he[i].sym = 1023 - i;
        he[i].len = len[i];
        if (len[i] == 0 || len[i] > 32)
            return AVERROR_INVALIDDATA;
    }
    AV_QSORT(he, 1024, HuffEntry, huff_cmp_len10);

    code = 1;
    for (i = 1023; i >= 0; i--) {
        codes[i] = code >> (32 - he[i].len);
        bits[i]  = he[i].len;
        syms[i]  = he[i].sym;
        code += 0x80000000u >> (he[i].len - 1);
    }

    ff_free_vlc(vlc);
    return ff_init_vlc_sparse(vlc, FFMIN(he[1023].len, 12), 1024,
                              bits,  sizeof(*bits),  sizeof(*bits),
                              codes, sizeof(*codes), sizeof(*codes),
                              syms,  sizeof(*syms),  sizeof(*syms), 0);
}

static int huff_build12(VLC *vlc, uint8_t *len)
{
    HuffEntry he[4096];
    uint32_t codes[4096];
    uint8_t bits[4096];
    uint16_t syms[4096];
    uint32_t code;
    int i;

    for (i = 0; i < 4096; i++) {
        he[i].sym = 4095 - i;
        he[i].len = len[i];
        if (len[i] == 0 || len[i] > 32)
            return AVERROR_INVALIDDATA;
    }
    AV_QSORT(he, 4096, HuffEntry, huff_cmp_len12);

    code = 1;
    for (i = 4095; i >= 0; i--) {
        codes[i] = code >> (32 - he[i].len);
        bits[i]  = he[i].len;
        syms[i]  = he[i].sym;
        code += 0x80000000u >> (he[i].len - 1);
    }

    ff_free_vlc(vlc);
    return ff_init_vlc_sparse(vlc, FFMIN(he[4095].len, 14), 4096,
                              bits,  sizeof(*bits),  sizeof(*bits),
                              codes, sizeof(*codes), sizeof(*codes),
                              syms,  sizeof(*syms),  sizeof(*syms), 0);
}

{

            return AVERROR_INVALIDDATA;
    }

    }

                              bits,  sizeof(*bits),  sizeof(*bits),
                              codes, sizeof(*codes), sizeof(*codes),
                              syms,  sizeof(*syms),  sizeof(*syms), 0);
}

static void magicyuv_median_pred16(uint16_t *dst, const uint16_t *src1,
                                   const uint16_t *diff, intptr_t w,
                                   int *left, int *left_top, int max)
{
    int i;
    uint16_t l, lt;

    l  = *left;
    lt = *left_top;

    for (i = 0; i < w; i++) {
        l      = mid_pred(l, src1[i], (l + src1[i] - lt)) + diff[i];
        l     &= max;
        lt     = src1[i];
        dst[i] = l;
    }

    *left     = l;
    *left_top = lt;
}

static int magy_decode_slice10(AVCodecContext *avctx, void *tdata,
                               int j, int threadnr)
{
    MagicYUVContext *s = avctx->priv_data;
    int interlaced = s->interlaced;
    const int bps = s->bps;
    const int max = s->max - 1;
    AVFrame *p = s->p;
    int i, k, x;
    GetBitContext gb;
    uint16_t *dst;

    for (i = 0; i < s->planes; i++) {
        int left, lefttop, top;
        int height = AV_CEIL_RSHIFT(FFMIN(s->slice_height, avctx->coded_height - j * s->slice_height), s->vshift[i]);
        int width = AV_CEIL_RSHIFT(avctx->coded_width, s->hshift[i]);
        int sheight = AV_CEIL_RSHIFT(s->slice_height, s->vshift[i]);
        ptrdiff_t fake_stride = (p->linesize[i] / 2) * (1 + interlaced);
        ptrdiff_t stride = p->linesize[i] / 2;
        int flags, pred;
        int ret = init_get_bits8(&gb, s->buf + s->slices[i][j].start,
                                 s->slices[i][j].size);

        if (ret < 0)
            return ret;

        flags = get_bits(&gb, 8);
        pred  = get_bits(&gb, 8);

        dst = (uint16_t *)p->data[i] + j * sheight * stride;
        if (flags & 1) {
            if (get_bits_left(&gb) < bps * width * height)
                return AVERROR_INVALIDDATA;
            for (k = 0; k < height; k++) {
                for (x = 0; x < width; x++)
                    dst[x] = get_bits(&gb, bps);

                dst += stride;
            }
        } else {
            for (k = 0; k < height; k++) {
                for (x = 0; x < width; x++) {
                    int pix;
                    if (get_bits_left(&gb) <= 0)
                        return AVERROR_INVALIDDATA;

                    pix = get_vlc2(&gb, s->vlc[i].table, s->vlc[i].bits, 3);
                    if (pix < 0)
                        return AVERROR_INVALIDDATA;

                    dst[x] = max - pix;
                }
                dst += stride;
            }
        }

        switch (pred) {
        case LEFT:
            dst = (uint16_t *)p->data[i] + j * sheight * stride;
            s->llviddsp.add_left_pred_int16(dst, dst, max, width, 0);
            dst += stride;
            if (interlaced) {
                s->llviddsp.add_left_pred_int16(dst, dst, max, width, 0);
                dst += stride;
            }
            for (k = 1 + interlaced; k < height; k++) {
                s->llviddsp.add_left_pred_int16(dst, dst, max, width, dst[-fake_stride]);
                dst += stride;
            }
            break;
        case GRADIENT:
            dst = (uint16_t *)p->data[i] + j * sheight * stride;
            s->llviddsp.add_left_pred_int16(dst, dst, max, width, 0);
            dst += stride;
            if (interlaced) {
                s->llviddsp.add_left_pred_int16(dst, dst, max, width, 0);
                dst += stride;
            }
            for (k = 1 + interlaced; k < height; k++) {
                top = dst[-fake_stride];
                left = top + dst[0];
                dst[0] = left & max;
                for (x = 1; x < width; x++) {
                    top = dst[x - fake_stride];
                    lefttop = dst[x - (fake_stride + 1)];
                    left += top - lefttop + dst[x];
                    dst[x] = left & max;
                }
                dst += stride;
            }
            break;
        case MEDIAN:
            dst = (uint16_t *)p->data[i] + j * sheight * stride;
            s->llviddsp.add_left_pred_int16(dst, dst, max, width, 0);
            dst += stride;
            if (interlaced) {
                s->llviddsp.add_left_pred_int16(dst, dst, max, width, 0);
                dst += stride;
            }
            lefttop = left = dst[0];
            for (k = 1 + interlaced; k < height; k++) {
                magicyuv_median_pred16(dst, dst - fake_stride, dst, width, &left, &lefttop, max);
                lefttop = left = dst[0];
                dst += stride;
            }
            break;
        default:
            avpriv_request_sample(avctx, "Unknown prediction: %d", pred);
        }
    }

    if (s->decorrelate) {
        int height = FFMIN(s->slice_height, avctx->coded_height - j * s->slice_height);
        int width = avctx->coded_width;
        uint16_t *r = (uint16_t *)p->data[0] + j * s->slice_height * p->linesize[0] / 2;
        uint16_t *g = (uint16_t *)p->data[1] + j * s->slice_height * p->linesize[1] / 2;
        uint16_t *b = (uint16_t *)p->data[2] + j * s->slice_height * p->linesize[2] / 2;

        for (i = 0; i < height; i++) {
            for (k = 0; k < width; k++) {
                b[k] = (b[k] + g[k]) & max;
                r[k] = (r[k] + g[k]) & max;
            }
            b += p->linesize[0] / 2;
            g += p->linesize[1] / 2;
            r += p->linesize[2] / 2;
        }
    }

    return 0;
}

                             int j, int threadnr)
{


            return ret;


            if (get_bits_left(&gb) < 8* width * height)
                return AVERROR_INVALIDDATA;
            for (k = 0; k < height; k++) {
                for (x = 0; x < width; x++)
                    dst[x] = get_bits(&gb, 8);

                dst += stride;
            }
        } else {
                        return AVERROR_INVALIDDATA;

                        return AVERROR_INVALIDDATA;

                }
            }
        }

            }
            }
            break;
            }
                }
            }
            break;
            }
                                             dst, width, &left, &lefttop);
            }
            break;
        default:
            avpriv_request_sample(avctx, "Unknown prediction: %d", pred);
        }
    }


        }
    }

    return 0;
}

{



                av_log(avctx, AV_LOG_ERROR, "Cannot build Huffman codes\n");
                return AVERROR_INVALIDDATA;
            }
                break;
            }
            av_log(avctx, AV_LOG_ERROR, "Invalid Huffman codes\n");
            return AVERROR_INVALIDDATA;
        }
    }

        av_log(avctx, AV_LOG_ERROR, "Huffman tables too short\n");
        return AVERROR_INVALIDDATA;
    }

    return 0;
}

                             int *got_frame, AVPacket *avpkt)
{

        return AVERROR_INVALIDDATA;

        av_log(avctx, AV_LOG_ERROR,
               "header or packet too small %"PRIu32"\n", header_size);
        return AVERROR_INVALIDDATA;
    }

        avpriv_request_sample(avctx, "Version %d", version);
        return AVERROR_PATCHWELCOME;
    }


    case 0x6c:
        avctx->pix_fmt = AV_PIX_FMT_YUV422P10;
        s->hshift[1] =
        s->hshift[2] = 1;
        s->bps = 10;
        break;
    case 0x76:
        avctx->pix_fmt = AV_PIX_FMT_YUV444P10;
        s->bps = 10;
        break;
    case 0x6d:
        avctx->pix_fmt = AV_PIX_FMT_GBRP10;
        s->decorrelate = 1;
        s->bps = 10;
        break;
    case 0x6e:
        avctx->pix_fmt = AV_PIX_FMT_GBRAP10;
        s->decorrelate = 1;
        s->bps = 10;
        break;
    case 0x6f:
        avctx->pix_fmt = AV_PIX_FMT_GBRP12;
        s->decorrelate = 1;
        s->bps = 12;
        break;
    case 0x70:
        avctx->pix_fmt = AV_PIX_FMT_GBRAP12;
        s->decorrelate = 1;
        s->bps = 12;
        break;
    case 0x73:
        avctx->pix_fmt = AV_PIX_FMT_GRAY10;
        s->bps = 10;
        break;
    default:
        avpriv_request_sample(avctx, "Format 0x%X", format);
        return AVERROR_PATCHWELCOME;
    }
    else
        s->huff_build = s->bps == 10 ? huff_build10 : huff_build12;


        return ret;

        avpriv_request_sample(avctx, "Slice width %"PRIu32, slice_width);
        return AVERROR_PATCHWELCOME;
    }
        av_log(avctx, AV_LOG_ERROR,
               "invalid slice height: %d\n", s->slice_height);
        return AVERROR_INVALIDDATA;
    }


        av_log(avctx, AV_LOG_ERROR,
               "invalid number of slices: %d\n", s->nb_slices);
        return AVERROR_INVALIDDATA;
    }

            av_log(avctx, AV_LOG_ERROR, "impossible slice height\n");
            return AVERROR_INVALIDDATA;
        }
            av_log(avctx, AV_LOG_ERROR, "impossible height\n");
            return AVERROR_INVALIDDATA;
        }
    }

            return AVERROR(ENOMEM);

            return AVERROR_INVALIDDATA;



                return AVERROR_INVALIDDATA;

        }

    }

        return AVERROR_INVALIDDATA;


        return AVERROR_INVALIDDATA;

        return ret;

        return ret;


        return ret;


        avctx->pix_fmt == AV_PIX_FMT_GBRP12) {
    } else {
        case 1:
            p->colorspace = AVCOL_SPC_BT470BG;
            break;
        }
    }


}

{
}

{

    }

}

AVCodec ff_magicyuv_decoder = {
    .name             = "magicyuv",
    .long_name        = NULL_IF_CONFIG_SMALL("MagicYUV video"),
    .type             = AVMEDIA_TYPE_VIDEO,
    .id               = AV_CODEC_ID_MAGICYUV,
    .priv_data_size   = sizeof(MagicYUVContext),
    .init             = magy_decode_init,
    .close            = magy_decode_end,
    .decode           = magy_decode_frame,
    .capabilities     = AV_CODEC_CAP_DR1 |
                        AV_CODEC_CAP_FRAME_THREADS |
                        AV_CODEC_CAP_SLICE_THREADS,
    .caps_internal    = FF_CODEC_CAP_INIT_THREADSAFE,
};
