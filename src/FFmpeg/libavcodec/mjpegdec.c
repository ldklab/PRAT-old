/*
 * MJPEG decoder
 * Copyright (c) 2000, 2001 Fabrice Bellard
 * Copyright (c) 2003 Alex Beregszaszi
 * Copyright (c) 2003-2004 Michael Niedermayer
 *
 * Support for external huffman table, various fixes (AVID workaround),
 * aspecting, new decode_frame mechanism and apple mjpeg-b support
 *                                  by Alex Beregszaszi
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
 * MJPEG decoder.
 */

#include "libavutil/imgutils.h"
#include "libavutil/avassert.h"
#include "libavutil/opt.h"
#include "avcodec.h"
#include "blockdsp.h"
#include "copy_block.h"
#include "hwconfig.h"
#include "idctdsp.h"
#include "internal.h"
#include "jpegtables.h"
#include "mjpeg.h"
#include "mjpegdec.h"
#include "jpeglsdec.h"
#include "profiles.h"
#include "put_bits.h"
#include "tiff.h"
#include "exif.h"
#include "bytestream.h"


                     const uint8_t *val_table, int nb_codes,
                     int use_static, int is_ac)
{





                              huff_code, 2, 2, huff_sym, 2, 2, use_static);
}

{
        int class;
        int index;
        const uint8_t *bits;
        const uint8_t *values;
        int codes;
        int length;
    } ht[] = {
        { 0, 0, avpriv_mjpeg_bits_dc_luminance,
                avpriv_mjpeg_val_dc, 12, 12 },
        { 0, 1, avpriv_mjpeg_bits_dc_chrominance,
                avpriv_mjpeg_val_dc, 12, 12 },
        { 1, 0, avpriv_mjpeg_bits_ac_luminance,
                avpriv_mjpeg_val_ac_luminance,   251, 162 },
        { 1, 1, avpriv_mjpeg_bits_ac_chrominance,
                avpriv_mjpeg_val_ac_chrominance, 251, 162 },
        { 2, 0, avpriv_mjpeg_bits_ac_luminance,
                avpriv_mjpeg_val_ac_luminance,   251, 162 },
        { 2, 1, avpriv_mjpeg_bits_ac_chrominance,
                avpriv_mjpeg_val_ac_chrominance, 251, 162 },
    };

                        ht[i].bits, ht[i].values, ht[i].codes,
            return ret;

        }
    }

    return 0;
}

static void parse_avid(MJpegDecodeContext *s, uint8_t *buf, int len)
{
    s->buggy_avid = 1;
    if (len > 14 && buf[12] == 1) /* 1 - NTSC */
        s->interlace_polarity = 1;
    if (len > 14 && buf[12] == 2) /* 2 - PAL */
        s->interlace_polarity = 0;
    if (s->avctx->debug & FF_DEBUG_PICT_INFO)
        av_log(s->avctx, AV_LOG_INFO, "AVID: len:%d %d\n", len, len > 14 ? buf[12] : -1);
}

{

                      ff_zigzag_direct);

{

            return AVERROR(ENOMEM);
    }


        return ret;

        av_log(avctx, AV_LOG_INFO, "using external huffman table\n");
        if ((ret = init_get_bits(&s->gb, avctx->extradata, avctx->extradata_size * 8)) < 0)
            return ret;
        if (ff_mjpeg_decode_dht(s)) {
            av_log(avctx, AV_LOG_ERROR,
                   "error using external huffman table, switching back to internal\n");
            init_default_huffman_tables(s);
        }
    }
        s->interlace_polarity = 1;           /* bottom field first */
        av_log(avctx, AV_LOG_DEBUG, "bottom field first\n");
    }

        && AV_RL32(avctx->extradata) == 0x2C
        && AV_RL32(avctx->extradata+4) == 0x18) {
        parse_avid(s, avctx->extradata, avctx->extradata_size);
    }


    return 0;
}


/* quantize tables */
{


        av_log(s->avctx, AV_LOG_ERROR, "dqt: len %d is too large\n", len);
        return AVERROR_INVALIDDATA;
    }

            av_log(s->avctx, AV_LOG_ERROR, "dqt: invalid precision\n");
            return AVERROR_INVALIDDATA;
        }
            return -1;
        /* read quant table */
                av_log(s->avctx, AV_LOG_ERROR, "dqt: 0 quant value\n");
                return AVERROR_INVALIDDATA;
            }
        }

        // XXX FIXME fine-tune, and perhaps add dc too
               index, s->qscale[index]);
    }
    return 0;
}

/* decode huffman tables and build VLC decoders */
{


        av_log(s->avctx, AV_LOG_ERROR, "dht: len %d is too large\n", len);
        return AVERROR_INVALIDDATA;
    }

            return AVERROR_INVALIDDATA;
            return AVERROR_INVALIDDATA;
            return AVERROR_INVALIDDATA;
        n = 0;
        }
            return AVERROR_INVALIDDATA;

        code_max = 0;
                code_max = v;
        }

        /* build VLC and flush previous vlc if present */
               class, index, code_max + 1);
                             code_max + 1, 0, class > 0)) < 0)
            return ret;

                                 code_max + 1, 0, 0)) < 0)
                return ret;
        }

    }
    return 0;
}

{



        av_log(s->avctx, AV_LOG_ERROR, "bits %d is invalid\n", bits);
        return AVERROR_INVALIDDATA;
    }

    }
        bits = 9;
        s->rct  = 1;    // FIXME ugly

        av_log(s->avctx, AV_LOG_ERROR, "lowres is not possible with lossless jpeg\n");
        return -1;
    }


    // HACK for odd_height.mov
        height= s->height;

        return AVERROR_INVALIDDATA;
        return AVERROR_INVALIDDATA;

        nb_components > MAX_COMPONENTS)
        return -1;
            av_log(s->avctx, AV_LOG_ERROR,
                   "nb_components changing in interlaced picture\n");
            return AVERROR_INVALIDDATA;
        }
    }
        avpriv_report_missing_feature(s->avctx,
                                      "JPEG-LS that is not <= 8 "
                                      "bits/component or 16-bit gray");
        return AVERROR_PATCHWELCOME;
    }
        av_log(s->avctx, AV_LOG_ERROR, "decode_sof0: error, len(%d) mismatch %d components\n", len, nb_components);
        return AVERROR_INVALIDDATA;
    }

        /* component id */
        /* compute hmax and vmax (only used in interleaved case) */
            av_log(s->avctx, AV_LOG_ERROR, "quant_index is invalid\n");
            return AVERROR_INVALIDDATA;
        }
            av_log(s->avctx, AV_LOG_ERROR,
                   "Invalid sampling factor in component %d %d:%d\n",
                   i, h_count[i], v_count[i]);
            return AVERROR_INVALIDDATA;
        }

               i, h_count[i], v_count[i],
               s->component_id[i], s->quant_index[i]);
    }
        && s->component_id[0] == 'C' - 1
        && s->component_id[1] == 'M' - 1
        && s->component_id[2] == 'Y' - 1
        && s->component_id[3] == 'K' - 1)
        s->adobe_transform = 0;

        avpriv_report_missing_feature(s->avctx, "Subsampling in JPEG-LS");
        return AVERROR_PATCHWELCOME;
    }

        if (nb_components == 2) {
            /* Bayer images embedded in DNGs can contain 2 interleaved components and the
               width stored in their SOF3 markers is the width of each one.  We only output
               a single component, therefore we need to adjust the output image width.  We
               handle the deinterleaving (but not the debayering) in this file. */
            width *= 2;
        }
        /* They can also contain 1 component, which is double the width and half the height
            of the final image (rows are interleaved).  We don't handle the decoding in this
            file, but leave that to the TIFF/DNG decoder. */
    }

    /* if different size, realloc/alloc picture */


        /* test interlaced mode */
        }

            return ret;

    } else {
        size_change = 0;
    }

            avpriv_request_sample(s->avctx, "progressively coded interlaced picture");
            return AVERROR_INVALIDDATA;
        }
    } else {
        /* XXX: not complete test ! */
        /* NOTE we do not allocate pictures large enough for the possible
         * padding of h/v_count being 4 */
            pix_fmt_id -= (pix_fmt_id & 0xF0F0F0F0) >> 1;



                if (i & 1) s->upscale_h[j/2] = 1;
                else       s->upscale_v[j/2] = 1;
            }
        }

            if (pix_fmt_id != 0x11110000 && pix_fmt_id != 0x11000000)
                goto unk_pixfmt;
        }

        case 0x11110000: /* for bayer-encoded huffman lossless JPEGs embedded in DNGs */
            if (!s->bayer)
                goto unk_pixfmt;
            s->avctx->pix_fmt = AV_PIX_FMT_GRAY16LE;
            break;
            else {
                    s->avctx->pix_fmt = s->bits <= 8 ? AV_PIX_FMT_GBRP : AV_PIX_FMT_GBRP16;
                } else {
                    else              s->avctx->pix_fmt = AV_PIX_FMT_YUV444P16;
                }
            }
            break;
        case 0x11111111:
            if (s->rgb)
                s->avctx->pix_fmt = s->bits <= 9 ? AV_PIX_FMT_ABGR : AV_PIX_FMT_RGBA64;
            else {
                if (s->adobe_transform == 0 && s->bits <= 8) {
                    s->avctx->pix_fmt = AV_PIX_FMT_GBRAP;
                } else {
                    s->avctx->pix_fmt = s->bits <= 8 ? AV_PIX_FMT_YUVA444P : AV_PIX_FMT_YUVA444P16;
                    s->avctx->color_range = s->cs_itu601 ? AVCOL_RANGE_MPEG : AVCOL_RANGE_JPEG;
                }
            }
            av_assert0(s->nb_components == 4);
            break;
        case 0x22111122:
        case 0x22111111:
            if (s->adobe_transform == 0 && s->bits <= 8) {
                s->avctx->pix_fmt = AV_PIX_FMT_GBRAP;
                s->upscale_v[1] = s->upscale_v[2] = 1;
                s->upscale_h[1] = s->upscale_h[2] = 1;
            } else if (s->adobe_transform == 2 && s->bits <= 8) {
                s->avctx->pix_fmt = AV_PIX_FMT_YUVA444P;
                s->upscale_v[1] = s->upscale_v[2] = 1;
                s->upscale_h[1] = s->upscale_h[2] = 1;
                s->avctx->color_range = s->cs_itu601 ? AVCOL_RANGE_MPEG : AVCOL_RANGE_JPEG;
            } else {
                if (s->bits <= 8) s->avctx->pix_fmt = AV_PIX_FMT_YUVA420P;
                else              s->avctx->pix_fmt = AV_PIX_FMT_YUVA420P16;
                s->avctx->color_range = s->cs_itu601 ? AVCOL_RANGE_MPEG : AVCOL_RANGE_JPEG;
            }
            av_assert0(s->nb_components == 4);
            break;
        case 0x12121100:
        case 0x22122100:
        case 0x21211100:
        case 0x22211200:
        case 0x22221100:
        case 0x22112200:
        case 0x11222200:
            if (s->bits <= 8) s->avctx->pix_fmt = s->cs_itu601 ? AV_PIX_FMT_YUV444P : AV_PIX_FMT_YUVJ444P;
            else
                goto unk_pixfmt;
            s->avctx->color_range = s->cs_itu601 ? AVCOL_RANGE_MPEG : AVCOL_RANGE_JPEG;
            break;
        case 0x13000000:
        case 0x14000000:
        case 0x31000000:
        case 0x33000000:
        case 0x34000000:
        case 0x41000000:
        case 0x43000000:
        case 0x44000000:
                s->avctx->pix_fmt = AV_PIX_FMT_GRAY8;
            else
            break;
        case 0x12111100:
        case 0x14121200:
        case 0x14111100:
        case 0x22211100:
        case 0x22112100:
            if (s->component_id[0] == 'Q' && s->component_id[1] == 'F' && s->component_id[2] == 'A') {
                if (s->bits <= 8) s->avctx->pix_fmt = AV_PIX_FMT_GBRP;
                else
                    goto unk_pixfmt;
                s->upscale_v[0] = s->upscale_v[1] = 1;
            } else {
                if (pix_fmt_id == 0x14111100)
                    s->upscale_v[1] = s->upscale_v[2] = 1;
                if (s->bits <= 8) s->avctx->pix_fmt = s->cs_itu601 ? AV_PIX_FMT_YUV440P : AV_PIX_FMT_YUVJ440P;
                else
                    goto unk_pixfmt;
                s->avctx->color_range = s->cs_itu601 ? AVCOL_RANGE_MPEG : AVCOL_RANGE_JPEG;
            }
            break;
                if (s->bits <= 8) s->avctx->pix_fmt = AV_PIX_FMT_GBRP;
                else
                    goto unk_pixfmt;
                s->upscale_h[0] = s->upscale_h[1] = 1;
            } else {
                else              s->avctx->pix_fmt = AV_PIX_FMT_YUV422P16;
            }
            break;
        case 0x31111100:
            if (s->bits > 8)
                goto unk_pixfmt;
            s->avctx->pix_fmt = s->cs_itu601 ? AV_PIX_FMT_YUV444P : AV_PIX_FMT_YUVJ444P;
            s->avctx->color_range = s->cs_itu601 ? AVCOL_RANGE_MPEG : AVCOL_RANGE_JPEG;
            s->upscale_h[1] = s->upscale_h[2] = 2;
            break;
        case 0x22121100:
        case 0x22111200:
            if (s->bits <= 8) s->avctx->pix_fmt = s->cs_itu601 ? AV_PIX_FMT_YUV422P : AV_PIX_FMT_YUVJ422P;
            else
                goto unk_pixfmt;
            s->avctx->color_range = s->cs_itu601 ? AVCOL_RANGE_MPEG : AVCOL_RANGE_JPEG;
            break;
        case 0x23111100:
        case 0x42111100:
        case 0x24111100:
            else              s->avctx->pix_fmt = AV_PIX_FMT_YUV420P16;
                if (s->bits > 8)
                    goto unk_pixfmt;
                s->upscale_h[1] = s->upscale_h[2] = 1;
                if (s->bits > 8)
                    goto unk_pixfmt;
                s->upscale_v[1] = s->upscale_v[2] = 1;
                if (s->bits > 8)
                    goto unk_pixfmt;
                s->upscale_v[1] = s->upscale_v[2] = 2;
            }
            break;
        case 0x41111100:
            if (s->bits <= 8) s->avctx->pix_fmt = s->cs_itu601 ? AV_PIX_FMT_YUV411P : AV_PIX_FMT_YUVJ411P;
            else
                goto unk_pixfmt;
            s->avctx->color_range = s->cs_itu601 ? AVCOL_RANGE_MPEG : AVCOL_RANGE_JPEG;
            break;
        default:
    unk_pixfmt:
            avpriv_report_missing_feature(s->avctx, "Pixel format 0x%x bits:%d", pix_fmt_id, s->bits);
            memset(s->upscale_h, 0, sizeof(s->upscale_h));
            memset(s->upscale_v, 0, sizeof(s->upscale_v));
            return AVERROR_PATCHWELCOME;
        }
            avpriv_report_missing_feature(s->avctx, "Lowres for weird subsampling");
            return AVERROR_PATCHWELCOME;
        }
            avpriv_report_missing_feature(s->avctx, "progressive for weird subsampling");
            return AVERROR_PATCHWELCOME;
        }
            } else if (s->nb_components != 1) {
                av_log(s->avctx, AV_LOG_ERROR, "Unsupported number of components %d\n", s->nb_components);
                return AVERROR_PATCHWELCOME;
            } else if (s->palette_index && s->bits <= 8)
                s->avctx->pix_fmt = AV_PIX_FMT_PAL8;
            else if (s->bits <= 8)
                s->avctx->pix_fmt = AV_PIX_FMT_GRAY8;
            else
                s->avctx->pix_fmt = AV_PIX_FMT_GRAY16;
        }

            av_log(s->avctx, AV_LOG_ERROR, "Could not get a pixel format descriptor.\n");
            return AVERROR_BUG;
        }

        } else {
#if CONFIG_MJPEG_NVDEC_HWACCEL
                AV_PIX_FMT_CUDA,
#endif
#if CONFIG_MJPEG_VAAPI_HWACCEL
                AV_PIX_FMT_VAAPI,
#endif
                s->avctx->pix_fmt,
                AV_PIX_FMT_NONE,
            };
                return AVERROR(EINVAL);

        }

        }

            return -1;


                s->width, s->height, s->linesize[0], s->linesize[1],
                s->interlaced, s->avctx->height);

    }

    ) {
        av_log(s->avctx, AV_LOG_ERROR, "Unsupported coding and pixel format combination\n");
        return AVERROR_PATCHWELCOME;
    }

    /* totally blank picture as progressive JPEG will only add details to it */
        int bw = (width  + s->h_max * 8 - 1) / (s->h_max * 8);
        int bh = (height + s->v_max * 8 - 1) / (s->v_max * 8);
        for (i = 0; i < s->nb_components; i++) {
            int size = bw * bh * s->h_count[i] * s->v_count[i];
            av_freep(&s->blocks[i]);
            av_freep(&s->last_nnz[i]);
            s->blocks[i]       = av_mallocz_array(size, sizeof(**s->blocks));
            s->last_nnz[i]     = av_mallocz_array(size, sizeof(**s->last_nnz));
            if (!s->blocks[i] || !s->last_nnz[i])
                return AVERROR(ENOMEM);
            s->block_stride[i] = bw * s->h_count[i];
        }
        memset(s->coefs_finished, 0, sizeof(s->coefs_finished));
    }

        s->hwaccel_picture_private =
            av_mallocz(s->avctx->hwaccel->frame_priv_data_size);
        if (!s->hwaccel_picture_private)
            return AVERROR(ENOMEM);

        ret = s->avctx->hwaccel->start_frame(s->avctx, s->raw_image_buffer,
                                             s->raw_image_buffer_size);
        if (ret < 0)
            return ret;
    }

    return 0;
}

{
        av_log(s->avctx, AV_LOG_WARNING,
               "mjpeg_decode_dc: bad vlc: %d:%d (%p)\n",
               0, dc_index, &s->vlcs[0][dc_index]);
        return 0xfffff;
    }

    else
        return 0;
}

/* decode block and dequantize */
                        int dc_index, int ac_index, uint16_t *quant_matrix)
{

    /* DC coef */
        av_log(s->avctx, AV_LOG_ERROR, "error dc\n");
        return AVERROR_INVALIDDATA;
    }
    /* AC coefs */


            {
            }


                av_log(s->avctx, AV_LOG_ERROR, "error count: %d\n", i);
                return AVERROR_INVALIDDATA;
            }
        }

}

static int decode_dc_progressive(MJpegDecodeContext *s, int16_t *block,
                                 int component, int dc_index,
                                 uint16_t *quant_matrix, int Al)
{
    unsigned val;
    s->bdsp.clear_block(block);
    val = mjpeg_decode_dc(s, dc_index);
    if (val == 0xfffff) {
        av_log(s->avctx, AV_LOG_ERROR, "error dc\n");
        return AVERROR_INVALIDDATA;
    }
    val = (val * (quant_matrix[0] << Al)) + s->last_dc[component];
    s->last_dc[component] = val;
    block[0] = val;
    return 0;
}

/* decode block and dequantize - progressive JPEG version */
static int decode_block_progressive(MJpegDecodeContext *s, int16_t *block,
                                    uint8_t *last_nnz, int ac_index,
                                    uint16_t *quant_matrix,
                                    int ss, int se, int Al, int *EOBRUN)
{
    int code, i, j, val, run;
    unsigned level;

    if (*EOBRUN) {
        (*EOBRUN)--;
        return 0;
    }

    {
        OPEN_READER(re, &s->gb);
        for (i = ss; ; i++) {
            UPDATE_CACHE(re, &s->gb);
            GET_VLC(code, re, &s->gb, s->vlcs[2][ac_index].table, 9, 2);

            run = ((unsigned) code) >> 4;
            code &= 0xF;
            if (code) {
                i += run;
                if (code > MIN_CACHE_BITS - 16)
                    UPDATE_CACHE(re, &s->gb);

                {
                    int cache = GET_CACHE(re, &s->gb);
                    int sign  = (~cache) >> 31;
                    level     = (NEG_USR32(sign ^ cache,code) ^ sign) - sign;
                }

                LAST_SKIP_BITS(re, &s->gb, code);

                if (i >= se) {
                    if (i == se) {
                        j = s->scantable.permutated[se];
                        block[j] = level * (quant_matrix[se] << Al);
                        break;
                    }
                    av_log(s->avctx, AV_LOG_ERROR, "error count: %d\n", i);
                    return AVERROR_INVALIDDATA;
                }
                j = s->scantable.permutated[i];
                block[j] = level * (quant_matrix[i] << Al);
            } else {
                if (run == 0xF) {// ZRL - skip 15 coefficients
                    i += 15;
                    if (i >= se) {
                        av_log(s->avctx, AV_LOG_ERROR, "ZRL overflow: %d\n", i);
                        return AVERROR_INVALIDDATA;
                    }
                } else {
                    val = (1 << run);
                    if (run) {
                        UPDATE_CACHE(re, &s->gb);
                        val += NEG_USR32(GET_CACHE(re, &s->gb), run);
                        LAST_SKIP_BITS(re, &s->gb, run);
                    }
                    *EOBRUN = val - 1;
                    break;
                }
            }
        }
        CLOSE_READER(re, &s->gb);
    }

    if (i > *last_nnz)
        *last_nnz = i;

    return 0;
}

#define REFINE_BIT(j) {                                             \
    UPDATE_CACHE(re, &s->gb);                                       \
    sign = block[j] >> 15;                                          \
    block[j] += SHOW_UBITS(re, &s->gb, 1) *                         \
                ((quant_matrix[i] ^ sign) - sign) << Al;            \
    LAST_SKIP_BITS(re, &s->gb, 1);                                  \
}

#define ZERO_RUN                                                    \
for (; ; i++) {                                                     \
    if (i > last) {                                                 \
        i += run;                                                   \
        if (i > se) {                                               \
            av_log(s->avctx, AV_LOG_ERROR, "error count: %d\n", i); \
            return -1;                                              \
        }                                                           \
        break;                                                      \
    }                                                               \
    j = s->scantable.permutated[i];                                 \
    if (block[j])                                                   \
        REFINE_BIT(j)                                               \
    else if (run-- == 0)                                            \
        break;                                                      \
}

/* decode block and dequantize - progressive JPEG refinement pass */
static int decode_block_refinement(MJpegDecodeContext *s, int16_t *block,
                                   uint8_t *last_nnz,
                                   int ac_index, uint16_t *quant_matrix,
                                   int ss, int se, int Al, int *EOBRUN)
{
    int code, i = ss, j, sign, val, run;
    int last    = FFMIN(se, *last_nnz);

    OPEN_READER(re, &s->gb);
    if (*EOBRUN) {
        (*EOBRUN)--;
    } else {
        for (; ; i++) {
            UPDATE_CACHE(re, &s->gb);
            GET_VLC(code, re, &s->gb, s->vlcs[2][ac_index].table, 9, 2);

            if (code & 0xF) {
                run = ((unsigned) code) >> 4;
                UPDATE_CACHE(re, &s->gb);
                val = SHOW_UBITS(re, &s->gb, 1);
                LAST_SKIP_BITS(re, &s->gb, 1);
                ZERO_RUN;
                j = s->scantable.permutated[i];
                val--;
                block[j] = ((quant_matrix[i] << Al) ^ val) - val;
                if (i == se) {
                    if (i > *last_nnz)
                        *last_nnz = i;
                    CLOSE_READER(re, &s->gb);
                    return 0;
                }
            } else {
                run = ((unsigned) code) >> 4;
                if (run == 0xF) {
                    ZERO_RUN;
                } else {
                    val = run;
                    run = (1 << run);
                    if (val) {
                        UPDATE_CACHE(re, &s->gb);
                        run += SHOW_UBITS(re, &s->gb, val);
                        LAST_SKIP_BITS(re, &s->gb, val);
                    }
                    *EOBRUN = run - 1;
                    break;
                }
            }
        }

        if (i > *last_nnz)
            *last_nnz = i;
    }

    for (; i <= last; i++) {
        j = s->scantable.permutated[i];
        if (block[j])
            REFINE_BIT(j)
    }
    CLOSE_READER(re, &s->gb);

    return 0;
}
#undef REFINE_BIT
#undef ZERO_RUN

{

        s->restart_count--;
        if(s->restart_count == 0 && s->avctx->codec_id == AV_CODEC_ID_THP){
            align_get_bits(&s->gb);
            for (i = 0; i < nb_components; i++) /* reset dc */
                s->last_dc[i] = (4 << s->bits);
        }

        i = 8 + ((-get_bits_count(&s->gb)) & 7);
        /* skip RSTn */
        if (s->restart_count == 0) {
            if(   show_bits(&s->gb, i) == (1 << i) - 1
               || show_bits(&s->gb, i) == 0xFF) {
                int pos = get_bits_count(&s->gb);
                align_get_bits(&s->gb);
                while (get_bits_left(&s->gb) >= 8 && show_bits(&s->gb, 8) == 0xFF)
                    skip_bits(&s->gb, 8);
                if (get_bits_left(&s->gb) >= 8 && (get_bits(&s->gb, 8) & 0xF8) == 0xD0) {
                    for (i = 0; i < nb_components; i++) /* reset dc */
                        s->last_dc[i] = (4 << s->bits);
                    reset = 1;
                } else
                    skip_bits_long(&s->gb, pos - get_bits_count(&s->gb));
            }
        }
    }
}

/* Handles 1 to 4 components */
static int ljpeg_decode_rgb_scan(MJpegDecodeContext *s, int nb_components, int predictor, int point_transform)
{
    int i, mb_x, mb_y;
    unsigned width;
    uint16_t (*buffer)[4];
    int left[4], top[4], topleft[4];
    const int linesize = s->linesize[0];
    const int mask     = ((1 << s->bits) - 1) << point_transform;
    int resync_mb_y = 0;
    int resync_mb_x = 0;
    int vpred[6];

    if (!s->bayer && s->nb_components < 3)
        return AVERROR_INVALIDDATA;
    if (s->bayer && s->nb_components > 2)
        return AVERROR_INVALIDDATA;
    if (s->nb_components <= 0 || s->nb_components > 4)
        return AVERROR_INVALIDDATA;
    if (s->v_max != 1 || s->h_max != 1 || !s->lossless)
        return AVERROR_INVALIDDATA;


    s->restart_count = s->restart_interval;

    if (s->restart_interval == 0)
        s->restart_interval = INT_MAX;

    if (s->bayer)
        width = s->mb_width / nb_components; /* Interleaved, width stored is the total so need to divide */
    else
        width = s->mb_width;

    av_fast_malloc(&s->ljpeg_buffer, &s->ljpeg_buffer_size, width * 4 * sizeof(s->ljpeg_buffer[0][0]));
    if (!s->ljpeg_buffer)
        return AVERROR(ENOMEM);

    buffer = s->ljpeg_buffer;

    for (i = 0; i < 4; i++)
        buffer[0][i] = 1 << (s->bits - 1);

    for (mb_y = 0; mb_y < s->mb_height; mb_y++) {
        uint8_t *ptr = s->picture_ptr->data[0] + (linesize * mb_y);

        if (s->interlaced && s->bottom_field)
            ptr += linesize >> 1;

        for (i = 0; i < 4; i++)
            top[i] = left[i] = topleft[i] = buffer[0][i];

        if ((mb_y * s->width) % s->restart_interval == 0) {
            for (i = 0; i < 6; i++)
                vpred[i] = 1 << (s->bits-1);
        }

        for (mb_x = 0; mb_x < width; mb_x++) {
            int modified_predictor = predictor;

            if (get_bits_left(&s->gb) < 1) {
                av_log(s->avctx, AV_LOG_ERROR, "bitstream end in rgb_scan\n");
                return AVERROR_INVALIDDATA;
            }

            if (s->restart_interval && !s->restart_count){
                s->restart_count = s->restart_interval;
                resync_mb_x = mb_x;
                resync_mb_y = mb_y;
                for(i=0; i<4; i++)
                    top[i] = left[i]= topleft[i]= 1 << (s->bits - 1);
            }
            if (mb_y == resync_mb_y || mb_y == resync_mb_y+1 && mb_x < resync_mb_x || !mb_x)
                modified_predictor = 1;

            for (i=0;i<nb_components;i++) {
                int pred, dc;

                topleft[i] = top[i];
                top[i]     = buffer[mb_x][i];

                dc = mjpeg_decode_dc(s, s->dc_index[i]);
                if(dc == 0xFFFFF)
                    return -1;

                if (!s->bayer || mb_x) {
                    pred = left[i];
                } else { /* This path runs only for the first line in bayer images */
                    vpred[i] += dc;
                    pred = vpred[i] - dc;
                }

                PREDICT(pred, topleft[i], top[i], pred, modified_predictor);

                left[i] = buffer[mb_x][i] =
                    mask & (pred + (unsigned)(dc * (1 << point_transform)));
            }

            if (s->restart_interval && !--s->restart_count) {
                align_get_bits(&s->gb);
                skip_bits(&s->gb, 16); /* skip RSTn */
            }
        }
        if (s->rct && s->nb_components == 4) {
            for (mb_x = 0; mb_x < s->mb_width; mb_x++) {
                ptr[4*mb_x + 2] = buffer[mb_x][0] - ((buffer[mb_x][1] + buffer[mb_x][2] - 0x200) >> 2);
                ptr[4*mb_x + 1] = buffer[mb_x][1] + ptr[4*mb_x + 2];
                ptr[4*mb_x + 3] = buffer[mb_x][2] + ptr[4*mb_x + 2];
                ptr[4*mb_x + 0] = buffer[mb_x][3];
            }
        } else if (s->nb_components == 4) {
            for(i=0; i<nb_components; i++) {
                int c= s->comp_index[i];
                if (s->bits <= 8) {
                    for(mb_x = 0; mb_x < s->mb_width; mb_x++) {
                        ptr[4*mb_x+3-c] = buffer[mb_x][i];
                    }
                } else if(s->bits == 9) {
                    return AVERROR_PATCHWELCOME;
                } else {
                    for(mb_x = 0; mb_x < s->mb_width; mb_x++) {
                        ((uint16_t*)ptr)[4*mb_x+c] = buffer[mb_x][i];
                    }
                }
            }
        } else if (s->rct) {
            for (mb_x = 0; mb_x < s->mb_width; mb_x++) {
                ptr[3*mb_x + 1] = buffer[mb_x][0] - ((buffer[mb_x][1] + buffer[mb_x][2] - 0x200) >> 2);
                ptr[3*mb_x + 0] = buffer[mb_x][1] + ptr[3*mb_x + 1];
                ptr[3*mb_x + 2] = buffer[mb_x][2] + ptr[3*mb_x + 1];
            }
        } else if (s->pegasus_rct) {
            for (mb_x = 0; mb_x < s->mb_width; mb_x++) {
                ptr[3*mb_x + 1] = buffer[mb_x][0] - ((buffer[mb_x][1] + buffer[mb_x][2]) >> 2);
                ptr[3*mb_x + 0] = buffer[mb_x][1] + ptr[3*mb_x + 1];
                ptr[3*mb_x + 2] = buffer[mb_x][2] + ptr[3*mb_x + 1];
            }
        } else if (s->bayer) {
            if (nb_components == 1) {
                /* Leave decoding to the TIFF/DNG decoder (see comment in ff_mjpeg_decode_sof) */
                for (mb_x = 0; mb_x < width; mb_x++)
                    ((uint16_t*)ptr)[mb_x] = buffer[mb_x][0];
            } else if (nb_components == 2) {
                for (mb_x = 0; mb_x < width; mb_x++) {
                    ((uint16_t*)ptr)[2*mb_x + 0] = buffer[mb_x][0];
                    ((uint16_t*)ptr)[2*mb_x + 1] = buffer[mb_x][1];
                }
            }
        } else {
            for(i=0; i<nb_components; i++) {
                int c= s->comp_index[i];
                if (s->bits <= 8) {
                    for(mb_x = 0; mb_x < s->mb_width; mb_x++) {
                        ptr[3*mb_x+2-c] = buffer[mb_x][i];
                    }
                } else if(s->bits == 9) {
                    return AVERROR_PATCHWELCOME;
                } else {
                    for(mb_x = 0; mb_x < s->mb_width; mb_x++) {
                        ((uint16_t*)ptr)[3*mb_x+2-c] = buffer[mb_x][i];
                    }
                }
            }
        }
    }
    return 0;
}

                                 int point_transform, int nb_components)
{



                av_log(s->avctx, AV_LOG_ERROR, "bitstream end in yuv_scan\n");
                return AVERROR_INVALIDDATA;
            }
                s->restart_count = s->restart_interval;
                resync_mb_x = mb_x;
                resync_mb_y = mb_y;
            }




                            return -1;
                            // Nothing to do
                                }else{
                                }
                            }else{
                                }else{
                                }
                            }

                                ptr += linesize >> 1;
                        }else{
                            ptr16 = (uint16_t*)(s->picture_ptr->data[c] + 2*(linesize * (v * mb_y + y)) + 2*(h * mb_x + x)); //FIXME optimize this crap
                            if(y==0 && toprow){
                                if(x==0 && leftcol){
                                    pred= 1 << (bits - 1);
                                }else{
                                    pred= ptr16[-1];
                                }
                            }else{
                                if(x==0 && leftcol){
                                    pred= ptr16[-linesize];
                                }else{
                                    PREDICT(pred, ptr16[-linesize-1], ptr16[-linesize], ptr16[-1], predictor);
                                }
                            }

                            if (s->interlaced && s->bottom_field)
                                ptr16 += linesize >> 1;
                            pred &= mask;
                            *ptr16= pred + ((unsigned)dc << point_transform);
                        }
                        }
                    }
                }
            } else {



                            return -1;
                            // Nothing to do

                        }else{
                            ptr16 = (uint16_t*)(s->picture_ptr->data[c] + 2*(linesize * (v * mb_y + y)) + 2*(h * mb_x + x)); //FIXME optimize this crap
                            PREDICT(pred, ptr16[-linesize-1], ptr16[-linesize], ptr16[-1], predictor);

                            pred &= mask;
                            *ptr16= pred + ((unsigned)dc << point_transform);
                        }

                        }
                    }
                }
            }
                align_get_bits(&s->gb);
            }
        }
    }
    return 0;
}

                                              uint8_t *dst, const uint8_t *src,
                                              int linesize, int lowres)
{
    case 1: copy_block4(dst, src, linesize, linesize, 4);
        break;
    case 2: copy_block2(dst, src, linesize, linesize, 2);
        break;
    case 3: *dst = *src;
        break;
    }
}

static void shift_output(MJpegDecodeContext *s, uint8_t *ptr, int linesize)
{
    int block_x, block_y;
    int size = 8 >> s->avctx->lowres;
    if (s->bits > 8) {
        for (block_y=0; block_y<size; block_y++)
            for (block_x=0; block_x<size; block_x++)
                *(uint16_t*)(ptr + 2*block_x + block_y*linesize) <<= 16 - s->bits;
    } else {
        for (block_y=0; block_y<size; block_y++)
            for (block_x=0; block_x<size; block_x++)
                *(ptr + block_x + block_y*linesize) <<= 8 - s->bits;
    }
}

                             int Al, const uint8_t *mb_bitmask,
                             int mb_bitmask_size,
                             const AVFrame *reference)
{

            av_log(s->avctx, AV_LOG_ERROR, "mb_bitmask_size mismatches\n");
            return AVERROR_INVALIDDATA;
        }
    }


                                     &chroma_v_shift);

    }


                s->restart_count = s->restart_interval;

                av_log(s->avctx, AV_LOG_ERROR, "overread %d\n",
                       -get_bits_left(&s->gb));
                return AVERROR_INVALIDDATA;
            }

                    } else
                        ptr = NULL;
                                                linesize[c], s->avctx->lowres);

                        } else {
                                             s->dc_index[i], s->ac_index[i],
                                av_log(s->avctx, AV_LOG_ERROR,
                                       "error y=%d x=%d\n", mb_y, mb_x);
                                return AVERROR_INVALIDDATA;
                            }
                            }
                        }
                    } else {
                        int block_idx  = s->block_stride[c] * (v * mb_y + y) +
                                         (h * mb_x + x);
                        int16_t *block = s->blocks[c][block_idx];
                        if (Ah)
                            block[0] += get_bits1(&s->gb) *
                                        s->quant_matrixes[s->quant_sindex[i]][0] << Al;
                        else if (decode_dc_progressive(s, block, i, s->dc_index[i],
                                                       s->quant_matrixes[s->quant_sindex[i]],
                                                       Al) < 0) {
                            av_log(s->avctx, AV_LOG_ERROR,
                                   "error y=%d x=%d\n", mb_y, mb_x);
                            return AVERROR_INVALIDDATA;
                        }
                    }
                            mb_x, mb_y, x, y, c, s->bottom_field,
                            (v * mb_y + y) * 8, (h * mb_x + x) * 8);
                    }
                }
            }

        }
    }
    return 0;
}

static int mjpeg_decode_scan_progressive_ac(MJpegDecodeContext *s, int ss,
                                            int se, int Ah, int Al)
{
    int mb_x, mb_y;
    int EOBRUN = 0;
    int c = s->comp_index[0];
    uint16_t *quant_matrix = s->quant_matrixes[s->quant_sindex[0]];

    av_assert0(ss>=0 && Ah>=0 && Al>=0);
    if (se < ss || se > 63) {
        av_log(s->avctx, AV_LOG_ERROR, "SS/SE %d/%d is invalid\n", ss, se);
        return AVERROR_INVALIDDATA;
    }

    // s->coefs_finished is a bitmask for coefficients coded
    // ss and se are parameters telling start and end coefficients
    s->coefs_finished[c] |= (2ULL << se) - (1ULL << ss);

    s->restart_count = 0;

    for (mb_y = 0; mb_y < s->mb_height; mb_y++) {
        int block_idx    = mb_y * s->block_stride[c];
        int16_t (*block)[64] = &s->blocks[c][block_idx];
        uint8_t *last_nnz    = &s->last_nnz[c][block_idx];
        if (get_bits_left(&s->gb) <= 0) {
            av_log(s->avctx, AV_LOG_ERROR, "bitstream truncated in mjpeg_decode_scan_progressive_ac\n");
            return AVERROR_INVALIDDATA;
        }
        for (mb_x = 0; mb_x < s->mb_width; mb_x++, block++, last_nnz++) {
                int ret;
                if (s->restart_interval && !s->restart_count)
                    s->restart_count = s->restart_interval;

                if (Ah)
                    ret = decode_block_refinement(s, *block, last_nnz, s->ac_index[0],
                                                  quant_matrix, ss, se, Al, &EOBRUN);
                else
                    ret = decode_block_progressive(s, *block, last_nnz, s->ac_index[0],
                                                   quant_matrix, ss, se, Al, &EOBRUN);
                if (ret < 0) {
                    av_log(s->avctx, AV_LOG_ERROR,
                           "error y=%d x=%d\n", mb_y, mb_x);
                    return AVERROR_INVALIDDATA;
                }

            if (handle_rstn(s, 0))
                EOBRUN = 0;
        }
    }
    return 0;
}

static void mjpeg_idct_scan_progressive_ac(MJpegDecodeContext *s)
{
    int mb_x, mb_y;
    int c;
    const int bytes_per_pixel = 1 + (s->bits > 8);
    const int block_size = s->lossless ? 1 : 8;

    for (c = 0; c < s->nb_components; c++) {
        uint8_t *data = s->picture_ptr->data[c];
        int linesize  = s->linesize[c];
        int h = s->h_max / s->h_count[c];
        int v = s->v_max / s->v_count[c];
        int mb_width     = (s->width  + h * block_size - 1) / (h * block_size);
        int mb_height    = (s->height + v * block_size - 1) / (v * block_size);

        if (~s->coefs_finished[c])
            av_log(s->avctx, AV_LOG_WARNING, "component %d is incomplete\n", c);

        if (s->interlaced && s->bottom_field)
            data += linesize >> 1;

        for (mb_y = 0; mb_y < mb_height; mb_y++) {
            uint8_t *ptr     = data + (mb_y * linesize * 8 >> s->avctx->lowres);
            int block_idx    = mb_y * s->block_stride[c];
            int16_t (*block)[64] = &s->blocks[c][block_idx];
            for (mb_x = 0; mb_x < mb_width; mb_x++, block++) {
                s->idsp.idct_put(ptr, linesize, *block);
                if (s->bits & 7)
                    shift_output(s, ptr, linesize);
                ptr += bytes_per_pixel*8 >> s->avctx->lowres;
            }
        }
    }
}

                        int mb_bitmask_size, const AVFrame *reference)
{

        av_log(s->avctx, AV_LOG_WARNING,
                "Can not process SOS before SOF, skipping\n");
        return -1;
    }

            av_log(s->avctx, AV_LOG_ERROR, "Reference mismatching\n");
            return AVERROR_INVALIDDATA;
        }
    }

    /* XXX: verify len field validity */
        avpriv_report_missing_feature(s->avctx,
                                      "decode_sos: nb_components (%d)",
                                      nb_components);
        return AVERROR_PATCHWELCOME;
    }
        av_log(s->avctx, AV_LOG_ERROR, "decode_sos: invalid len (%d)\n", len);
        return AVERROR_INVALIDDATA;
    }
        /* find component index */
                break;
            av_log(s->avctx, AV_LOG_ERROR,
                   "decode_sos: index(%d) out of components\n", index);
            return AVERROR_INVALIDDATA;
        }
        /* Metasoft MJPEG codec has Cb and Cr swapped */
            && nb_components == 3 && s->nb_components == 3 && i)
            index = 3 - i;


            index = (index+2)%3;



            goto out_of_range;
            goto out_of_range;
    }

    }else
        prev_shift = point_transform = 0;

        /* interleaved stream */
    }

        av_log(s->avctx, AV_LOG_DEBUG, "%s %s p:%d >>:%d ilv:%d bits:%d skip:%d %s comp:%d\n",
               s->lossless ? "lossless" : "sequential DCT", s->rgb ? "RGB" : "",
               predictor, point_transform, ilv, s->bits, s->mjpb_skiptosod,
               s->pegasus_rct ? "PRCT" : (s->rct ? "RCT" : ""), nb_components);


    /* mjpeg-b can have padding bytes between sos and image data, skip them */


        int bytes_to_start = get_bits_count(&s->gb) / 8;
        av_assert0(bytes_to_start >= 0 &&
                   s->raw_scan_buffer_size >= bytes_to_start);

        ret = s->avctx->hwaccel->decode_slice(s->avctx,
                                              s->raw_scan_buffer      + bytes_to_start,
                                              s->raw_scan_buffer_size - bytes_to_start);
        if (ret < 0)
            return ret;

//            for () {
//            reset_ls_coding_parameters(s, 0);

                                                point_transform, ilv)) < 0)
                return ret;
        } else {
                if ((ret = ljpeg_decode_rgb_scan(s, nb_components, predictor, point_transform)) < 0)
                    return ret;
            } else {
                                                 point_transform,
                                                 nb_components)) < 0)
                    return ret;
            }
        }
    } else {
            av_assert0(s->picture_ptr == s->picture);
            if ((ret = mjpeg_decode_scan_progressive_ac(s, predictor,
                                                        ilv, prev_shift,
                                                        point_transform)) < 0)
                return ret;
        } else {
                                         prev_shift, point_transform,
                                         mb_bitmask, mb_bitmask_size, reference)) < 0)
                return ret;
        }
    }

        GetBitContext bak = s->gb;
        align_get_bits(&bak);
        if (show_bits(&bak, 16) == 0xFFD1) {
            av_log(s->avctx, AV_LOG_DEBUG, "AVRn interlaced picture marker found\n");
            s->gb = bak;
            skip_bits(&s->gb, 16);
            s->bottom_field ^= 1;

            goto next_field;
        }
    }

 out_of_range:
    av_log(s->avctx, AV_LOG_ERROR, "decode_sos: ac/dc index out of range\n");
    return AVERROR_INVALIDDATA;
}

{
        return AVERROR_INVALIDDATA;
           s->restart_interval);

}

{

        if (s->bayer) {
            // Pentax K-1 (digital camera) JPEG images embedded in DNG images contain unknown APP0 markers
            av_log(s->avctx, AV_LOG_WARNING, "skipping APPx (len=%"PRId32") for bayer-encoded image\n", len);
            skip_bits(&s->gb, len);
            return 0;
        } else
            return AVERROR_INVALIDDATA;
    }
        return AVERROR_INVALIDDATA;


        av_log(s->avctx, AV_LOG_DEBUG, "APPx (%s / %8X) len=%d\n",
               av_fourcc2str(av_bswap32(id)), id, len);

    /* Buggy AVID, it puts EOI only at every 10th frame. */
    /* Also, this fourcc is used by non-avid files too, it holds some
       information, but it's always present in AVID-created files. */
        /* structure:
            4bytes      AVI1
            1bytes      polarity
            1bytes      always zero
            4bytes      field_size
            4bytes      field_size_less_padding
        */
    }

            goto out;

            s->avctx->sample_aspect_ratio.num = 0;
            s->avctx->sample_aspect_ratio.den = 1;
        }

            av_log(s->avctx, AV_LOG_INFO,
                   "mjpeg: JFIF header found (version: %x.%x) SAR=%d/%d\n",
                   v1, v2,
                   s->avctx->sample_aspect_ratio.num,
                   s->avctx->sample_aspect_ratio.den);

                /* skip thumbnail */
                if (len -10 - (t_w * t_h * 3) > 0)
                    len -= t_w * t_h * 3;
            }
        }
    }

            av_log(s->avctx, AV_LOG_INFO, "mjpeg: Adobe header found, transform=%d\n", s->adobe_transform);
    }

        int rgb = s->rgb;
        int pegasus_rct = s->pegasus_rct;
        if (s->avctx->debug & FF_DEBUG_PICT_INFO)
            av_log(s->avctx, AV_LOG_INFO,
                   "Pegasus lossless jpeg header found\n");
        skip_bits(&s->gb, 16); /* version ? */
        skip_bits(&s->gb, 16); /* unknown always 0? */
        skip_bits(&s->gb, 16); /* unknown always 0? */
        skip_bits(&s->gb, 16); /* unknown always 0? */
        switch (i=get_bits(&s->gb, 8)) {
        case 1:
            rgb         = 1;
            pegasus_rct = 0;
            break;
        case 2:
            rgb         = 1;
            pegasus_rct = 1;
            break;
        default:
            av_log(s->avctx, AV_LOG_ERROR, "unknown colorspace %d\n", i);
        }

        len -= 9;
        if (s->got_picture)
            if (rgb != s->rgb || pegasus_rct != s->pegasus_rct) {
                av_log(s->avctx, AV_LOG_WARNING, "Mismatching LJIF tag\n");
                goto out;
            }

        s->rgb = rgb;
        s->pegasus_rct = pegasus_rct;

        goto out;
    }
        s->colr = get_bits(&s->gb, 8);
        if (s->avctx->debug & FF_DEBUG_PICT_INFO)
            av_log(s->avctx, AV_LOG_INFO, "COLR %d\n", s->colr);
        len --;
        goto out;
    }
        s->xfrm = get_bits(&s->gb, 8);
        if (s->avctx->debug & FF_DEBUG_PICT_INFO)
            av_log(s->avctx, AV_LOG_INFO, "XFRM %d\n", s->xfrm);
        len --;
        goto out;
    }

    /* JPS extension by VRex */
        int flags, layout, type;
        if (s->avctx->debug & FF_DEBUG_PICT_INFO)
            av_log(s->avctx, AV_LOG_INFO, "_JPSJPS_\n");

        skip_bits(&s->gb, 32); len -= 4;  /* JPS_ */
        skip_bits(&s->gb, 16); len -= 2;  /* block length */
        skip_bits(&s->gb, 8);             /* reserved */
        flags  = get_bits(&s->gb, 8);
        layout = get_bits(&s->gb, 8);
        type   = get_bits(&s->gb, 8);
        len -= 4;

        av_freep(&s->stereo3d);
        s->stereo3d = av_stereo3d_alloc();
        if (!s->stereo3d) {
            goto out;
        }
        if (type == 0) {
            s->stereo3d->type = AV_STEREO3D_2D;
        } else if (type == 1) {
            switch (layout) {
            case 0x01:
                s->stereo3d->type = AV_STEREO3D_LINES;
                break;
            case 0x02:
                s->stereo3d->type = AV_STEREO3D_SIDEBYSIDE;
                break;
            case 0x03:
                s->stereo3d->type = AV_STEREO3D_TOPBOTTOM;
                break;
            }
            if (!(flags & 0x04)) {
                s->stereo3d->flags = AV_STEREO3D_FLAG_INVERT;
            }
        }
        goto out;
    }

    /* EXIF metadata */


        // init byte wise reading

        // read TIFF header
            av_log(s->avctx, AV_LOG_ERROR, "mjpeg: invalid TIFF header in EXIF data\n");
        } else {

            // read 0th IFD and store the metadata
            // (return values > 0 indicate the presence of subimage metadata)
                av_log(s->avctx, AV_LOG_ERROR, "mjpeg: error decoding EXIF data\n");
            }
        }


    }

    /* Apple MJPEG-A */
        /* Apple MJPEG-A */
            /* structure:
                4bytes      field size
                4bytes      pad field size
                4bytes      next off
                4bytes      quant off
                4bytes      huff off
                4bytes      image off
                4bytes      scan off
                4bytes      data off
            */
            if (s->avctx->debug & FF_DEBUG_PICT_INFO)
                av_log(s->avctx, AV_LOG_INFO, "mjpeg: Apple MJPEG-A header found\n");
        }
    }

        int id2;
        unsigned seqno;
        unsigned nummarkers;

        id   = get_bits_long(&s->gb, 32);
        id2  = get_bits(&s->gb, 24);
        len -= 7;
        if (id != AV_RB32("PROF") || id2 != AV_RB24("ILE")) {
            av_log(s->avctx, AV_LOG_WARNING, "Invalid ICC_PROFILE header in APP2\n");
            goto out;
        }

        skip_bits(&s->gb, 8);
        seqno  = get_bits(&s->gb, 8);
        len   -= 2;
        if (seqno == 0) {
            av_log(s->avctx, AV_LOG_WARNING, "Invalid sequence number in APP2\n");
            goto out;
        }

        nummarkers  = get_bits(&s->gb, 8);
        len        -= 1;
        if (nummarkers == 0) {
            av_log(s->avctx, AV_LOG_WARNING, "Invalid number of markers coded in APP2\n");
            goto out;
        } else if (s->iccnum != 0 && nummarkers != s->iccnum) {
            av_log(s->avctx, AV_LOG_WARNING, "Mistmatch in coded number of ICC markers between markers\n");
            goto out;
        } else if (seqno > nummarkers) {
            av_log(s->avctx, AV_LOG_WARNING, "Mismatching sequence number and coded number of ICC markers\n");
            goto out;
        }

        /* Allocate if this is the first APP2 we've seen. */
        if (s->iccnum == 0) {
            s->iccdata     = av_mallocz(nummarkers * sizeof(*(s->iccdata)));
            s->iccdatalens = av_mallocz(nummarkers * sizeof(*(s->iccdatalens)));
            if (!s->iccdata || !s->iccdatalens) {
                av_log(s->avctx, AV_LOG_ERROR, "Could not allocate ICC data arrays\n");
                return AVERROR(ENOMEM);
            }
            s->iccnum = nummarkers;
        }

        if (s->iccdata[seqno - 1]) {
            av_log(s->avctx, AV_LOG_WARNING, "Duplicate ICC sequence number\n");
            goto out;
        }

        s->iccdatalens[seqno - 1]  = len;
        s->iccdata[seqno - 1]      = av_malloc(len);
        if (!s->iccdata[seqno - 1]) {
            av_log(s->avctx, AV_LOG_ERROR, "Could not allocate ICC data buffer\n");
            return AVERROR(ENOMEM);
        }

        memcpy(s->iccdata[seqno - 1], align_get_bits(&s->gb), len);
        skip_bits(&s->gb, len << 3);
        len = 0;
        s->iccread++;

        if (s->iccread > s->iccnum)
            av_log(s->avctx, AV_LOG_WARNING, "Read more ICC markers than are supposed to be coded\n");
    }

    /* slow but needed for extreme adobe jpegs */
        av_log(s->avctx, AV_LOG_ERROR,
               "mjpeg: error, decode_app parser read over the end\n");

    return 0;
}

{
            return AVERROR(ENOMEM);

        else

            av_log(s->avctx, AV_LOG_INFO, "comment: '%s'\n", cbuf);

        /* buggy avid, it puts EOI only at every 10th frame */
            parse_avid(s, cbuf, len);
            s->flipped = 1;
            s->avctx->sample_aspect_ratio = (AVRational) { 1, 2 };
            s->multiscope = 2;
        }

    }

    return 0;
}

/* return the 8 bit start code value and update the search
   state. Return -1 if no start code found */
{

        }
        skipped++;
    }
    buf_ptr = buf_end;
    val = -1;
}

                         const uint8_t **buf_ptr, const uint8_t *buf_end,
                         const uint8_t **unescaped_buf_ptr,
                         int *unescaped_buf_size)
{

        return AVERROR(ENOMEM);

    /* unescape buffer of SOS, use special treatment for JPEG-LS */

        #define copy_data_segment(skip) do {       \
            ptrdiff_t length = (ptr - src) - (skip);  \
            if (length > 0) {                         \
                memcpy(dst, src, length);             \
                dst += length;                        \
                src = ptr;                            \
            }                                         \
        } while (0)

        } else {

                    ptrdiff_t skip = 0;
                    }

                    /* 0xFF, 0xFF, ... */
                        copy_data_segment(skip);

                        /* decrement src as it is equal to ptr after the
                         * copy_data_segment macro and we might want to
                         * copy the current value of x later on */
                        src--;
                    }

                            break;
                    }
                }
            }
                copy_data_segment(0);
        }
        #undef copy_data_segment

               AV_INPUT_BUFFER_PADDING_SIZE);


        /* find marker */
                }
            }
        }

        /* unescape bitstream */
                    av_log(s->avctx, AV_LOG_WARNING, "Invalid escape sequence\n");
                    x &= 0x7f;
                }
            }
        }

               AV_INPUT_BUFFER_PADDING_SIZE);
    } else {
    }

    return start_code;
}

{

        for (i = 0; i < s->iccnum; i++)
            av_freep(&s->iccdata[i]);


                          AVPacket *avpkt)
{



        reset_icc_profile(s);

        /* find start next marker */
                                          &unescaped_buf_ptr,
                                          &unescaped_buf_size);
        /* EOF */
            break;
            av_log(avctx, AV_LOG_ERROR,
                   "MJPEG packet 0x%x too big (%d/%d), corrupt data?\n",
                   start_code, unescaped_buf_size, buf_size);
            return AVERROR_INVALIDDATA;
        }
               start_code, buf_end - buf_ptr);


            av_log(avctx, AV_LOG_ERROR, "invalid buffer\n");
            goto fail;
        }

            av_log(avctx, AV_LOG_DEBUG, "startcode: %X\n", start_code);

        /* process markers */
                   "restart marker: %d\n", start_code & 0x0f);
            /* APP fields */
                av_log(avctx, AV_LOG_ERROR, "unable to decode APP fields: %s\n",
                       av_err2str(ret));
            /* Comment */
                return ret;
                return ret;
        }


            (start_code == SOF48 || start_code == LSE)) {
            av_log(avctx, AV_LOG_ERROR, "JPEG-LS support not enabled.\n");
            return AVERROR(ENOSYS);
        }

            case SOF0:
            case SOF1:
            case SOF2:
            case SOF3:
            case SOF48:
            case SOI:
            case SOS:
            case EOI:
                break;
            }

            /* nothing to do on SOI */
                av_log(avctx, AV_LOG_ERROR, "huffman table decode error\n");
                goto fail;
            }
            break;
        case SOF1:
            else
                goto fail;
            break;
        case SOF2:
            s->avctx->profile = FF_PROFILE_MJPEG_HUFFMAN_PROGRESSIVE_DCT;
            s->lossless    = 0;
            s->ls          = 0;
            s->progressive = 1;
            if ((ret = ff_mjpeg_decode_sof(s)) < 0)
                goto fail;
            break;
                goto fail;
            break;
                goto fail;
            break;
        case LSE:
            if (!CONFIG_JPEGLS_DECODER ||
                (ret = ff_jpegls_decode_lse(s)) < 0)
                goto fail;
            break;
        case EOI:
                mjpeg_idct_scan_progressive_ac(s);
                av_log(avctx, AV_LOG_WARNING,
                       "Found EOI before any SOF, ignoring\n");
                break;
            }
                /* if not bottom field, do not output image yet */
                    break;
            }
            }
                ret = s->avctx->hwaccel->end_frame(s->avctx);
                if (ret < 0)
                    return ret;

                av_freep(&s->hwaccel_picture_private);
            }
                return ret;

                                s->qscale[1],
                                s->qscale[2]);
                }

                    av_log(avctx, AV_LOG_DEBUG, "QP: %d\n", qp);
            }


                break;
            }

                (avctx->err_recognition & AV_EF_EXPLODE))
                goto fail;
            break;
                return ret;
            break;
        case SOF5:
        case SOF6:
        case SOF7:
        case SOF9:
        case SOF10:
        case SOF11:
        case SOF13:
        case SOF14:
        case SOF15:
        case JPG:
            av_log(avctx, AV_LOG_ERROR,
                   "mjpeg: unsupported coding type (%x)\n", start_code);
            break;
        }

        /* eof process start code */
               "marker parser used %d bytes (%d bits)\n",
    }
    }
fail:
    s->got_picture = 0;
    return ret;


        int p;
        av_assert0(avctx->pix_fmt == AV_PIX_FMT_YUVJ444P ||
                   avctx->pix_fmt == AV_PIX_FMT_YUV444P  ||
                   avctx->pix_fmt == AV_PIX_FMT_YUVJ440P ||
                   avctx->pix_fmt == AV_PIX_FMT_YUV440P  ||
                   avctx->pix_fmt == AV_PIX_FMT_YUVA444P ||
                   avctx->pix_fmt == AV_PIX_FMT_YUVJ420P ||
                   avctx->pix_fmt == AV_PIX_FMT_YUV420P  ||
                   avctx->pix_fmt == AV_PIX_FMT_YUV420P16||
                   avctx->pix_fmt == AV_PIX_FMT_YUVA420P  ||
                   avctx->pix_fmt == AV_PIX_FMT_YUVA420P16||
                   avctx->pix_fmt == AV_PIX_FMT_GBRP     ||
                   avctx->pix_fmt == AV_PIX_FMT_GBRAP
                  );
        ret = av_pix_fmt_get_chroma_sub_sample(s->avctx->pix_fmt, &hshift, &vshift);
        if (ret)
            return ret;

        av_assert0(s->nb_components == av_pix_fmt_count_planes(s->picture_ptr->format));
        for (p = 0; p<s->nb_components; p++) {
            uint8_t *line = s->picture_ptr->data[p];
            int w = s->width;
            int h = s->height;
            if (!s->upscale_h[p])
                continue;
            if (p==1 || p==2) {
                w = AV_CEIL_RSHIFT(w, hshift);
                h = AV_CEIL_RSHIFT(h, vshift);
            }
            if (s->upscale_v[p] == 1)
                h = (h+1)>>1;
            av_assert0(w > 0);
            for (i = 0; i < h; i++) {
                if (s->upscale_h[p] == 1) {
                    if (is16bit) ((uint16_t*)line)[w - 1] = ((uint16_t*)line)[(w - 1) / 2];
                    else                      line[w - 1] = line[(w - 1) / 2];
                    for (index = w - 2; index > 0; index--) {
                        if (is16bit)
                            ((uint16_t*)line)[index] = (((uint16_t*)line)[index / 2] + ((uint16_t*)line)[(index + 1) / 2]) >> 1;
                        else
                            line[index] = (line[index / 2] + line[(index + 1) / 2]) >> 1;
                    }
                } else if (s->upscale_h[p] == 2) {
                    if (is16bit) {
                        ((uint16_t*)line)[w - 1] = ((uint16_t*)line)[(w - 1) / 3];
                        if (w > 1)
                            ((uint16_t*)line)[w - 2] = ((uint16_t*)line)[w - 1];
                    } else {
                        line[w - 1] = line[(w - 1) / 3];
                        if (w > 1)
                            line[w - 2] = line[w - 1];
                    }
                    for (index = w - 3; index > 0; index--) {
                        line[index] = (line[index / 3] + line[(index + 1) / 3] + line[(index + 2) / 3] + 1) / 3;
                    }
                }
                line += s->linesize[p];
            }
        }
    }
        int p;
        av_assert0(avctx->pix_fmt == AV_PIX_FMT_YUVJ444P ||
                   avctx->pix_fmt == AV_PIX_FMT_YUV444P  ||
                   avctx->pix_fmt == AV_PIX_FMT_YUVJ422P ||
                   avctx->pix_fmt == AV_PIX_FMT_YUV422P  ||
                   avctx->pix_fmt == AV_PIX_FMT_YUVJ420P ||
                   avctx->pix_fmt == AV_PIX_FMT_YUV420P  ||
                   avctx->pix_fmt == AV_PIX_FMT_YUV440P  ||
                   avctx->pix_fmt == AV_PIX_FMT_YUVJ440P ||
                   avctx->pix_fmt == AV_PIX_FMT_YUVA444P ||
                   avctx->pix_fmt == AV_PIX_FMT_YUVA420P  ||
                   avctx->pix_fmt == AV_PIX_FMT_YUVA420P16||
                   avctx->pix_fmt == AV_PIX_FMT_GBRP     ||
                   avctx->pix_fmt == AV_PIX_FMT_GBRAP
                   );
        ret = av_pix_fmt_get_chroma_sub_sample(s->avctx->pix_fmt, &hshift, &vshift);
        if (ret)
            return ret;

        av_assert0(s->nb_components == av_pix_fmt_count_planes(s->picture_ptr->format));
        for (p = 0; p < s->nb_components; p++) {
            uint8_t *dst;
            int w = s->width;
            int h = s->height;
            if (!s->upscale_v[p])
                continue;
            if (p==1 || p==2) {
                w = AV_CEIL_RSHIFT(w, hshift);
                h = AV_CEIL_RSHIFT(h, vshift);
            }
            dst = &((uint8_t *)s->picture_ptr->data[p])[(h - 1) * s->linesize[p]];
            for (i = h - 1; i; i--) {
                uint8_t *src1 = &((uint8_t *)s->picture_ptr->data[p])[i * s->upscale_v[p] / (s->upscale_v[p] + 1) * s->linesize[p]];
                uint8_t *src2 = &((uint8_t *)s->picture_ptr->data[p])[(i + 1) * s->upscale_v[p] / (s->upscale_v[p] + 1) * s->linesize[p]];
                if (s->upscale_v[p] != 2 && (src1 == src2 || i == h - 1)) {
                    memcpy(dst, src1, w);
                } else {
                    for (index = 0; index < w; index++)
                        dst[index] = (src1[index] + src2[index]) >> 1;
                }
                dst -= s->linesize[p];
            }
        }
    }
            return ret;

            }
                }
            }
        }
    }
        int w = s->picture_ptr->width;
        int h = s->picture_ptr->height;
        av_assert0(s->nb_components == 4);
        for (i=0; i<h; i++) {
            int j;
            uint8_t *dst[4];
            for (index=0; index<4; index++) {
                dst[index] =   s->picture_ptr->data[index]
                             + s->picture_ptr->linesize[index]*i;
            }
            for (j=0; j<w; j++) {
                int k = dst[3][j];
                int r = dst[0][j] * k;
                int g = dst[1][j] * k;
                int b = dst[2][j] * k;
                dst[0][j] = g*257 >> 16;
                dst[1][j] = b*257 >> 16;
                dst[2][j] = r*257 >> 16;
                dst[3][j] = 255;
            }
        }
    }
        int w = s->picture_ptr->width;
        int h = s->picture_ptr->height;
        av_assert0(s->nb_components == 4);
        for (i=0; i<h; i++) {
            int j;
            uint8_t *dst[4];
            for (index=0; index<4; index++) {
                dst[index] =   s->picture_ptr->data[index]
                             + s->picture_ptr->linesize[index]*i;
            }
            for (j=0; j<w; j++) {
                int k = dst[3][j];
                int r = (255 - dst[0][j]) * k;
                int g = (128 - dst[1][j]) * k;
                int b = (128 - dst[2][j]) * k;
                dst[0][j] = r*257 >> 16;
                dst[1][j] = (g*257 >> 16) + 128;
                dst[2][j] = (b*257 >> 16) + 128;
                dst[3][j] = 255;
            }
        }
    }

        AVStereo3D *stereo = av_stereo3d_create_side_data(data);
        if (stereo) {
            stereo->type  = s->stereo3d->type;
            stereo->flags = s->stereo3d->flags;
        }
        av_freep(&s->stereo3d);
    }

        AVFrameSideData *sd;
        size_t offset = 0;
        int total_size = 0;
        int i;

        /* Sum size of all parts. */
        for (i = 0; i < s->iccnum; i++)
            total_size += s->iccdatalens[i];

        sd = av_frame_new_side_data(data, AV_FRAME_DATA_ICC_PROFILE, total_size);
        if (!sd) {
            av_log(s->avctx, AV_LOG_ERROR, "Could not allocate frame side data\n");
            return AVERROR(ENOMEM);
        }

        /* Reassemble the parts, which are now in-order. */
        for (i = 0; i < s->iccnum; i++) {
            memcpy(sd->data + offset, s->iccdata[i], s->iccdatalens[i]);
            offset += s->iccdatalens[i];
        }
    }


           buf_end - buf_ptr);
//  return buf_end - buf_ptr;
}

{

        av_log(avctx, AV_LOG_INFO, "Single field\n");
    }

        av_frame_unref(s->picture_ptr);


    }
    }



}

static void decode_flush(AVCodecContext *avctx)
{
    MJpegDecodeContext *s = avctx->priv_data;
    s->got_picture = 0;
}

#if CONFIG_MJPEG_DECODER
#define OFFSET(x) offsetof(MJpegDecodeContext, x)
#define VD AV_OPT_FLAG_VIDEO_PARAM | AV_OPT_FLAG_DECODING_PARAM
static const AVOption options[] = {
    { "extern_huff", "Use external huffman table.",
      OFFSET(extern_huff), AV_OPT_TYPE_BOOL, { .i64 = 0 }, 0, 1, VD },
    { NULL },
};

static const AVClass mjpegdec_class = {
    .class_name = "MJPEG decoder",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
};

AVCodec ff_mjpeg_decoder = {
    .name           = "mjpeg",
    .long_name      = NULL_IF_CONFIG_SMALL("MJPEG (Motion JPEG)"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_MJPEG,
    .priv_data_size = sizeof(MJpegDecodeContext),
    .init           = ff_mjpeg_decode_init,
    .close          = ff_mjpeg_decode_end,
    .decode         = ff_mjpeg_decode_frame,
    .flush          = decode_flush,
    .capabilities   = AV_CODEC_CAP_DR1,
    .max_lowres     = 3,
    .priv_class     = &mjpegdec_class,
    .profiles       = NULL_IF_CONFIG_SMALL(ff_mjpeg_profiles),
    .caps_internal  = FF_CODEC_CAP_INIT_THREADSAFE |
                      FF_CODEC_CAP_SKIP_FRAME_FILL_PARAM,
    .hw_configs     = (const AVCodecHWConfigInternal*[]) {
#if CONFIG_MJPEG_NVDEC_HWACCEL
                        HWACCEL_NVDEC(mjpeg),
#endif
#if CONFIG_MJPEG_VAAPI_HWACCEL
                        HWACCEL_VAAPI(mjpeg),
#endif
                        NULL
                    },
};
#endif
#if CONFIG_THP_DECODER
AVCodec ff_thp_decoder = {
    .name           = "thp",
    .long_name      = NULL_IF_CONFIG_SMALL("Nintendo Gamecube THP video"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_THP,
    .priv_data_size = sizeof(MJpegDecodeContext),
    .init           = ff_mjpeg_decode_init,
    .close          = ff_mjpeg_decode_end,
    .decode         = ff_mjpeg_decode_frame,
    .flush          = decode_flush,
    .capabilities   = AV_CODEC_CAP_DR1,
    .max_lowres     = 3,
    .caps_internal  = FF_CODEC_CAP_INIT_THREADSAFE,
};
#endif
