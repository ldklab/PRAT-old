/*
 * JPEG 2000 image decoder
 * Copyright (c) 2007 Kamil Nowosad
 * Copyright (c) 2013 Nicolas Bertrand <nicoinattendu@gmail.com>
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
 * JPEG 2000 image decoder
 */

#include <inttypes.h>
#include <math.h>

#include "libavutil/attributes.h"
#include "libavutil/avassert.h"
#include "libavutil/common.h"
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
#include "libavutil/pixdesc.h"
#include "libavutil/thread.h"
#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"
#include "thread.h"
#include "jpeg2000.h"
#include "jpeg2000dsp.h"
#include "profiles.h"

#define JP2_SIG_TYPE    0x6A502020
#define JP2_SIG_VALUE   0x0D0A870A
#define JP2_CODESTREAM  0x6A703263
#define JP2_HEADER      0x6A703268

#define HAD_COC 0x01
#define HAD_QCC 0x02

#define MAX_POCS 32

typedef struct Jpeg2000POCEntry {
    uint16_t LYEpoc;
    uint16_t CSpoc;
    uint16_t CEpoc;
    uint8_t RSpoc;
    uint8_t REpoc;
    uint8_t Ppoc;
} Jpeg2000POCEntry;

typedef struct Jpeg2000POC {
    Jpeg2000POCEntry poc[MAX_POCS];
    int nb_poc;
    int is_default;
} Jpeg2000POC;

typedef struct Jpeg2000TilePart {
    uint8_t tile_index;                 // Tile index who refers the tile-part
    const uint8_t *tp_end;
    GetByteContext header_tpg;          // bit stream of header if PPM header is used
    GetByteContext tpg;                 // bit stream in tile-part
} Jpeg2000TilePart;

/* RMK: For JPEG2000 DCINEMA 3 tile-parts in a tile
 * one per component, so tile_part elements have a size of 3 */
typedef struct Jpeg2000Tile {
    Jpeg2000Component   *comp;
    uint8_t             properties[4];
    Jpeg2000CodingStyle codsty[4];
    Jpeg2000QuantStyle  qntsty[4];
    Jpeg2000POC         poc;
    Jpeg2000TilePart    tile_part[32];
    uint8_t             has_ppt;                // whether this tile has a ppt marker
    uint8_t             *packed_headers;        // contains packed headers. Used only along with PPT marker
    int                 packed_headers_size;    // size in bytes of the packed headers
    GetByteContext      packed_headers_stream;  // byte context corresponding to packed headers
    uint16_t tp_idx;                    // Tile-part index
    int coord[2][2];                    // border coordinates {{x0, x1}, {y0, y1}}
} Jpeg2000Tile;

typedef struct Jpeg2000DecoderContext {
    AVClass         *class;
    AVCodecContext  *avctx;
    GetByteContext  g;

    int             width, height;
    int             image_offset_x, image_offset_y;
    int             tile_offset_x, tile_offset_y;
    uint8_t         cbps[4];    // bits per sample in particular components
    uint8_t         sgnd[4];    // if a component is signed
    uint8_t         properties[4];

    uint8_t         has_ppm;
    uint8_t         *packed_headers; // contains packed headers. Used only along with PPM marker
    int             packed_headers_size;
    GetByteContext  packed_headers_stream;
    uint8_t         in_tile_headers;

    int             cdx[4], cdy[4];
    int             precision;
    int             ncomponents;
    int             colour_space;
    uint32_t        palette[256];
    int8_t          pal8;
    int             cdef[4];
    int             tile_width, tile_height;
    unsigned        numXtiles, numYtiles;
    int             maxtilelen;
    AVRational      sar;

    Jpeg2000CodingStyle codsty[4];
    Jpeg2000QuantStyle  qntsty[4];
    Jpeg2000POC         poc;
    uint8_t             roi_shift[4];

    int             bit_index;

    int             curtileno;

    Jpeg2000Tile    *tile;
    Jpeg2000DSPContext dsp;

    /*options parameters*/
    int             reduction_factor;
} Jpeg2000DecoderContext;

/* get_bits functions for JPEG2000 packet bitstream
 * It is a get_bit function with a bit-stuffing routine. If the value of the
 * byte is 0xFF, the next byte includes an extra zero bit stuffed into the MSB.
 * cf. ISO-15444-1:2002 / B.10.1 Bit-stuffing routine */
{

        }
    }
}

static void jpeg2000_flush(Jpeg2000DecoderContext *s)
{
    if (bytestream2_get_byte(&s->g) == 0xff)
        bytestream2_skip(&s->g, 1);
    s->bit_index = 8;
}

/* decode the value stored in node */
                           int threshold)
{

        av_log(s->avctx, AV_LOG_ERROR, "missing node\n");
        return AVERROR_INVALIDDATA;
    }

    }

    else

            curval = stack[sp]->val;
            else
                return ret;
        }
    }
    return curval;
}

                         int bpc, uint32_t log2_chroma_wh, int pal8)
{


        return 0;
    }

    case 4:
    case 3:
    case 2:

    case 1:
    }
    return match;
}

// pix_fmts with lower bpp have to be listed before
// similar pix_fmts with higher bpp.
#define RGB_PIXEL_FORMATS   AV_PIX_FMT_PAL8,AV_PIX_FMT_RGB24,AV_PIX_FMT_RGBA,AV_PIX_FMT_RGB48,AV_PIX_FMT_RGBA64
#define GRAY_PIXEL_FORMATS  AV_PIX_FMT_GRAY8,AV_PIX_FMT_GRAY8A,AV_PIX_FMT_GRAY16,AV_PIX_FMT_YA16
#define YUV_PIXEL_FORMATS   AV_PIX_FMT_YUV410P,AV_PIX_FMT_YUV411P,AV_PIX_FMT_YUVA420P, \
                            AV_PIX_FMT_YUV420P,AV_PIX_FMT_YUV422P,AV_PIX_FMT_YUVA422P, \
                            AV_PIX_FMT_YUV440P,AV_PIX_FMT_YUV444P,AV_PIX_FMT_YUVA444P, \
                            AV_PIX_FMT_YUV420P9,AV_PIX_FMT_YUV422P9,AV_PIX_FMT_YUV444P9, \
                            AV_PIX_FMT_YUVA420P9,AV_PIX_FMT_YUVA422P9,AV_PIX_FMT_YUVA444P9, \
                            AV_PIX_FMT_YUV420P10,AV_PIX_FMT_YUV422P10,AV_PIX_FMT_YUV444P10, \
                            AV_PIX_FMT_YUVA420P10,AV_PIX_FMT_YUVA422P10,AV_PIX_FMT_YUVA444P10, \
                            AV_PIX_FMT_YUV420P12,AV_PIX_FMT_YUV422P12,AV_PIX_FMT_YUV444P12, \
                            AV_PIX_FMT_YUV420P14,AV_PIX_FMT_YUV422P14,AV_PIX_FMT_YUV444P14, \
                            AV_PIX_FMT_YUV420P16,AV_PIX_FMT_YUV422P16,AV_PIX_FMT_YUV444P16, \
                            AV_PIX_FMT_YUVA420P16,AV_PIX_FMT_YUVA422P16,AV_PIX_FMT_YUVA444P16
#define XYZ_PIXEL_FORMATS   AV_PIX_FMT_XYZ12

static const enum AVPixelFormat rgb_pix_fmts[]  = {RGB_PIXEL_FORMATS};
static const enum AVPixelFormat gray_pix_fmts[] = {GRAY_PIXEL_FORMATS};
static const enum AVPixelFormat yuv_pix_fmts[]  = {YUV_PIXEL_FORMATS};
static const enum AVPixelFormat xyz_pix_fmts[]  = {XYZ_PIXEL_FORMATS,
                                                   YUV_PIXEL_FORMATS};
static const enum AVPixelFormat all_pix_fmts[]  = {RGB_PIXEL_FORMATS,
                                                   GRAY_PIXEL_FORMATS,
                                                   YUV_PIXEL_FORMATS,
                                                   XYZ_PIXEL_FORMATS};

/* marker segments */
/* get sizes and offsets of image, tiles; number of components */
{

        av_log(s->avctx, AV_LOG_ERROR, "Insufficient space for SIZ\n");
        return AVERROR_INVALIDDATA;
    }


        avpriv_request_sample(s->avctx, "Large Dimensions");
        return AVERROR_PATCHWELCOME;
    }

        av_log(s->avctx, AV_LOG_ERROR, "Invalid number of components: %d\n",
               s->ncomponents);
        return AVERROR_INVALIDDATA;
    }

        avpriv_request_sample(s->avctx, "Support for %d components",
                              ncomponents);
        return AVERROR_PATCHWELCOME;
    }

    ) {
        av_log(s->avctx, AV_LOG_ERROR, "Tile offsets are invalid\n");
        return AVERROR_INVALIDDATA;
    }


        av_log(s->avctx, AV_LOG_ERROR, "Invalid tile dimension %dx%d.\n",
               s->tile_width, s->tile_height);
        return AVERROR_INVALIDDATA;
    }

        av_log(s->avctx, AV_LOG_ERROR, "Insufficient space for %d components in SIZ\n", s->ncomponents);
        return AVERROR_INVALIDDATA;
    }

            av_log(s->avctx, AV_LOG_ERROR, "Invalid sample separation %d/%d\n", s->cdx[i], s->cdy[i]);
            return AVERROR_INVALIDDATA;
        }
    }


    // There must be at least a SOT and SOD per tile, their minimum size is 14
    ) {
        s->numXtiles = s->numYtiles = 0;
        return AVERROR(EINVAL);
    }

        s->numXtiles = s->numYtiles = 0;
        return AVERROR(ENOMEM);
    }


            return AVERROR(ENOMEM);
    }

    /* compute image size with reduction factor */
                                               s->reduction_factor);
                                               s->reduction_factor);
    }

        return ret;

        s->avctx->profile == FF_PROFILE_JPEG2000_DCINEMA_4K) {
        possible_fmts = xyz_pix_fmts;
        possible_fmts_nb = FF_ARRAY_ELEMS(xyz_pix_fmts);
    } else {
        case 16:
            possible_fmts = rgb_pix_fmts;
            possible_fmts_nb = FF_ARRAY_ELEMS(rgb_pix_fmts);
            break;
        case 17:
            possible_fmts = gray_pix_fmts;
            possible_fmts_nb = FF_ARRAY_ELEMS(gray_pix_fmts);
            break;
        case 18:
            possible_fmts = yuv_pix_fmts;
            possible_fmts_nb = FF_ARRAY_ELEMS(yuv_pix_fmts);
            break;
        default:
            possible_fmts = all_pix_fmts;
            possible_fmts_nb = FF_ARRAY_ELEMS(all_pix_fmts);
            break;
        }
    }
            s->avctx->pix_fmt = AV_PIX_FMT_NONE;
            }
        }

        if (ncomponents == 4 &&
            s->cdy[0] == 1 && s->cdx[0] == 1 &&
            s->cdy[1] == 1 && s->cdx[1] == 1 &&
            s->cdy[2] == s->cdy[3] && s->cdx[2] == s->cdx[3]) {
            if (s->precision == 8 && s->cdy[2] == 2 && s->cdx[2] == 2 && !s->pal8) {
                s->avctx->pix_fmt = AV_PIX_FMT_YUVA420P;
                s->cdef[0] = 0;
                s->cdef[1] = 1;
                s->cdef[2] = 2;
                s->cdef[3] = 3;
                i = 0;
            }
        } else if (ncomponents == 3 && s->precision == 8 &&
                   s->cdx[0] == s->cdx[1] && s->cdx[0] == s->cdx[2] &&
                   s->cdy[0] == s->cdy[1] && s->cdy[0] == s->cdy[2]) {
            s->avctx->pix_fmt = AV_PIX_FMT_RGB24;
            i = 0;
        } else if (ncomponents == 2 && s->precision == 8 &&
                   s->cdx[0] == s->cdx[1] && s->cdy[0] == s->cdy[1]) {
            s->avctx->pix_fmt = AV_PIX_FMT_YA8;
            i = 0;
        } else if (ncomponents == 1 && s->precision == 8) {
            s->avctx->pix_fmt = AV_PIX_FMT_GRAY8;
            i = 0;
        }
    }


        av_log(s->avctx, AV_LOG_ERROR,
               "Unknown pix_fmt, profile: %d, colour_space: %d, "
               "components: %d, precision: %d\n"
               "cdx[0]: %d, cdy[0]: %d\n"
               "cdx[1]: %d, cdy[1]: %d\n"
               "cdx[2]: %d, cdy[2]: %d\n"
               "cdx[3]: %d, cdy[3]: %d\n",
               s->avctx->profile, s->colour_space, ncomponents, s->precision,
               s->cdx[0],
               s->cdy[0],
               ncomponents > 1 ? s->cdx[1] : 0,
               ncomponents > 1 ? s->cdy[1] : 0,
               ncomponents > 2 ? s->cdx[2] : 0,
               ncomponents > 2 ? s->cdy[2] : 0,
               ncomponents > 3 ? s->cdx[3] : 0,
               ncomponents > 3 ? s->cdy[3] : 0);
        return AVERROR_PATCHWELCOME;
    }
}

/* get common part for COD and COC segments */
{

        av_log(s->avctx, AV_LOG_ERROR, "Insufficient space for COX\n");
        return AVERROR_INVALIDDATA;
    }

    /*  nreslevels = number of resolution levels
                   = number of decomposition level +1 */
        av_log(s->avctx, AV_LOG_ERROR, "nreslevels %d is invalid\n", c->nreslevels);
        return AVERROR_INVALIDDATA;
    }

        /* we are forced to update reduction_factor as its requested value is
           not compatible with this bitstream, and as we might have used it
           already in setup earlier we have to fail this frame until
           reinitialization is implemented */
        av_log(s->avctx, AV_LOG_ERROR, "reduction_factor too large for this bitstream, max is %d\n", c->nreslevels - 1);
        s->reduction_factor = c->nreslevels - 1;
        return AVERROR(EINVAL);
    }

    /* compute number of resolution levels to decode */


        av_log(s->avctx, AV_LOG_ERROR, "cblk size invalid\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(s->avctx, AV_LOG_WARNING, "extra cblk styles %X\n", c->cblk_style);
        if (c->cblk_style & JPEG2000_CBLK_BYPASS)
            av_log(s->avctx, AV_LOG_WARNING, "Selective arithmetic coding bypass\n");
    }
    /* set integer 9/7 DWT in case of BITEXACT flag */
    }

        int i;
                    av_log(s->avctx, AV_LOG_ERROR, "PPx %d PPy %d invalid\n",
                           c->log2_prec_widths[i], c->log2_prec_heights[i]);
                    c->log2_prec_widths[i] = c->log2_prec_heights[i] = 1;
                    return AVERROR_INVALIDDATA;
                }
        }
    } else {
    }
    return 0;
}

/* get coding parameters for a particular tile or whole image*/
                   uint8_t *properties)
{

        av_log(s->avctx, AV_LOG_ERROR, "Insufficient space for COD\n");
        return AVERROR_INVALIDDATA;
    }


    // get progression order


        av_log(s->avctx, AV_LOG_ERROR,
               "MCT %"PRIu8" with too few components (%d)\n",
               tmp.mct, s->ncomponents);
        return AVERROR_INVALIDDATA;
    }

        return ret;
    return 0;
}

/* Get coding parameters for a component in the whole image or a
 * particular tile. */
                   uint8_t *properties)
{

        av_log(s->avctx, AV_LOG_ERROR, "Insufficient space for COC\n");
        return AVERROR_INVALIDDATA;
    }


        av_log(s->avctx, AV_LOG_ERROR,
               "Invalid compno %d. There are %d components in the image.\n",
               compno, s->ncomponents);
        return AVERROR_INVALIDDATA;
    }


        return ret;

}

static int get_rgn(Jpeg2000DecoderContext *s, int n)
{
    uint16_t compno;
    compno = (s->ncomponents < 257)? bytestream2_get_byte(&s->g):
                                     bytestream2_get_be16u(&s->g);
    if (bytestream2_get_byte(&s->g)) {
        av_log(s->avctx, AV_LOG_ERROR, "Invalid RGN header.\n");
        return AVERROR_INVALIDDATA; // SRgn field value is 0
    }
    // SPrgn field
    // Currently compno cannot be greater than 4.
    // However, future implementation should support compno up to 65536
    if (compno < s->ncomponents) {
        int v;
        if (s->curtileno == -1) {
            v =  bytestream2_get_byte(&s->g);
            if (v > 30)
                return AVERROR_PATCHWELCOME;
            s->roi_shift[compno] = v;
        } else {
            if (s->tile[s->curtileno].tp_idx != 0)
                return AVERROR_INVALIDDATA; // marker occurs only in first tile part of tile
            v = bytestream2_get_byte(&s->g);
            if (v > 30)
                return AVERROR_PATCHWELCOME;
            s->tile[s->curtileno].comp[compno].roi_shift = v;
        }
        return 0;
    }
    return AVERROR_INVALIDDATA;
}

/* Get common part for QCD and QCC segments. */
{

        return AVERROR_INVALIDDATA;



            n > JPEG2000_MAX_DECLEVELS*3)
            return AVERROR_INVALIDDATA;
        if (bytestream2_get_bytes_left(&s->g) < 2)
            return AVERROR_INVALIDDATA;
        x          = bytestream2_get_be16u(&s->g);
        q->expn[0] = x >> 11;
        q->mant[0] = x & 0x7ff;
        for (i = 1; i < JPEG2000_MAX_DECLEVELS * 3; i++) {
            int curexpn = FFMAX(0, q->expn[0] - (i - 1) / 3);
            q->expn[i] = curexpn;
            q->mant[i] = q->mant[0];
        }
    } else {
            n > JPEG2000_MAX_DECLEVELS*3)
            return AVERROR_INVALIDDATA;
        }
    }
    return 0;
}

/* Get quantization parameters for a particular tile or a whole image. */
                   uint8_t *properties)
{


        return ret;
    return 0;
}

/* Get quantization parameters for a component in the whole image
 * on in a particular tile. */
                   uint8_t *properties)
{

        return AVERROR_INVALIDDATA;


        av_log(s->avctx, AV_LOG_ERROR,
               "Invalid compno %d. There are %d components in the image.\n",
               compno, s->ncomponents);
        return AVERROR_INVALIDDATA;
    }

}

static int get_poc(Jpeg2000DecoderContext *s, int size, Jpeg2000POC *p)
{
    int i;
    int elem_size = s->ncomponents <= 257 ? 7 : 9;
    Jpeg2000POC tmp = {{{0}}};

    if (bytestream2_get_bytes_left(&s->g) < 5 || size < 2 + elem_size) {
        av_log(s->avctx, AV_LOG_ERROR, "Insufficient space for POC\n");
        return AVERROR_INVALIDDATA;
    }

    if (elem_size > 7) {
        avpriv_request_sample(s->avctx, "Fat POC not supported");
        return AVERROR_PATCHWELCOME;
    }

    tmp.nb_poc = (size - 2) / elem_size;
    if (tmp.nb_poc > MAX_POCS) {
        avpriv_request_sample(s->avctx, "Too many POCs (%d)", tmp.nb_poc);
        return AVERROR_PATCHWELCOME;
    }

    for (i = 0; i<tmp.nb_poc; i++) {
        Jpeg2000POCEntry *e = &tmp.poc[i];
        e->RSpoc  = bytestream2_get_byteu(&s->g);
        e->CSpoc  = bytestream2_get_byteu(&s->g);
        e->LYEpoc = bytestream2_get_be16u(&s->g);
        e->REpoc  = bytestream2_get_byteu(&s->g);
        e->CEpoc  = bytestream2_get_byteu(&s->g);
        e->Ppoc   = bytestream2_get_byteu(&s->g);
        if (!e->CEpoc)
            e->CEpoc = 256;
        if (e->CEpoc > s->ncomponents)
            e->CEpoc = s->ncomponents;
        if (   e->RSpoc >= e->REpoc || e->REpoc > 33
            || e->CSpoc >= e->CEpoc || e->CEpoc > s->ncomponents
            || !e->LYEpoc) {
            av_log(s->avctx, AV_LOG_ERROR, "POC Entry %d is invalid (%d, %d, %d, %d, %d, %d)\n", i,
                e->RSpoc, e->CSpoc, e->LYEpoc, e->REpoc, e->CEpoc, e->Ppoc
            );
            return AVERROR_INVALIDDATA;
        }
    }

    if (!p->nb_poc || p->is_default) {
        *p = tmp;
    } else {
        if (p->nb_poc + tmp.nb_poc > MAX_POCS) {
            av_log(s->avctx, AV_LOG_ERROR, "Insufficient space for POC\n");
            return AVERROR_INVALIDDATA;
        }
        memcpy(p->poc + p->nb_poc, tmp.poc, tmp.nb_poc * sizeof(tmp.poc[0]));
        p->nb_poc += tmp.nb_poc;
    }

    p->is_default = 0;

    return 0;
}


/* Get start of tile segment. */
{

        return AVERROR_INVALIDDATA;

        return AVERROR_INVALIDDATA;


    /* Read TNSot but not used */

        Psot = bytestream2_get_bytes_left(&s->g) - 2 + n + 2;

        av_log(s->avctx, AV_LOG_ERROR, "Psot %"PRIu32" too big\n", Psot);
        return AVERROR_INVALIDDATA;
    }

        avpriv_request_sample(s->avctx, "Too many tile parts");
        return AVERROR_PATCHWELCOME;
    }



        /* copy defaults */
    }

    return 0;
}

static int read_crg(Jpeg2000DecoderContext *s, int n)
{
    if (s->ncomponents*4 != n - 2) {
        av_log(s->avctx, AV_LOG_ERROR, "Invalid CRG marker.\n");
        return AVERROR_INVALIDDATA;
    }
    bytestream2_skip(&s->g, n - 2);
    return 0;
}
/* Tile-part lengths: see ISO 15444-1:2002, section A.7.1
 * Used to know the number of tile parts and lengths.
 * There may be multiple TLMs in the header.
 * TODO: The function is not used for tile-parts management, nor anywhere else.
 * It can be useful to allocate memory for tile parts, before managing the SOT
 * markers. Parsing the TLM header is needed to increment the input header
 * buffer.
 * This marker is mandatory for DCI. */
{

    // too complex ? ST = ((Stlm >> 4) & 0x01) + ((Stlm >> 4) & 0x02);
        av_log(s->avctx, AV_LOG_ERROR, "TLM marker contains invalid ST value.\n");
        return AVERROR_INVALIDDATA;
    }

        case 0:
            break;
            break;
        case 2:
            bytestream2_get_be16(&s->g);
            break;
        case 3:
            break;
        }
            bytestream2_get_be16(&s->g);
        } else {
        }
    }
    return 0;
}

static int get_plt(Jpeg2000DecoderContext *s, int n)
{
    int i;
    int v;

    av_log(s->avctx, AV_LOG_DEBUG,
            "PLT marker at pos 0x%X\n", bytestream2_tell(&s->g) - 4);

    if (n < 4)
        return AVERROR_INVALIDDATA;

    /*Zplt =*/ bytestream2_get_byte(&s->g);

    for (i = 0; i < n - 3; i++) {
        v = bytestream2_get_byte(&s->g);
    }
    if (v & 0x80)
        return AVERROR_INVALIDDATA;

    return 0;
}

static int get_ppm(Jpeg2000DecoderContext *s, int n)
{
    void *new;

    if (n < 3) {
        av_log(s->avctx, AV_LOG_ERROR, "Invalid length for PPM data.\n");
        return AVERROR_INVALIDDATA;
    }
    bytestream2_get_byte(&s->g); //Zppm is skipped and not used
    new = av_realloc(s->packed_headers,
                     s->packed_headers_size + n - 3);
    if (new) {
        s->packed_headers = new;
    } else
        return AVERROR(ENOMEM);
    s->has_ppm = 1;
    memset(&s->packed_headers_stream, 0, sizeof(s->packed_headers_stream));
    bytestream_get_buffer(&s->g.buffer, s->packed_headers + s->packed_headers_size,
                          n - 3);
    s->packed_headers_size += n - 3;

    return 0;
}

static int get_ppt(Jpeg2000DecoderContext *s, int n)
{
    Jpeg2000Tile *tile;
    void *new;

    if (n < 3) {
        av_log(s->avctx, AV_LOG_ERROR, "Invalid length for PPT data.\n");
        return AVERROR_INVALIDDATA;
    }
    if (s->curtileno < 0)
        return AVERROR_INVALIDDATA;

    tile = &s->tile[s->curtileno];
    if (tile->tp_idx != 0) {
        av_log(s->avctx, AV_LOG_ERROR,
               "PPT marker can occur only on first tile part of a tile.\n");
        return AVERROR_INVALIDDATA;
    }

    tile->has_ppt = 1;  // this tile has a ppt marker
    bytestream2_get_byte(&s->g); // Zppt is skipped and not used
    new = av_realloc(tile->packed_headers,
                     tile->packed_headers_size + n - 3);
    if (new) {
        tile->packed_headers = new;
    } else
        return AVERROR(ENOMEM);
    memset(&tile->packed_headers_stream, 0, sizeof(tile->packed_headers_stream));
    memcpy(tile->packed_headers + tile->packed_headers_size,
           s->g.buffer, n - 3);
    tile->packed_headers_size += n - 3;
    bytestream2_skip(&s->g, n - 3);

    return 0;
}

{

        return AVERROR(ENOMEM);






            return AVERROR_INVALIDDATA;
                                             s->cdy[compno], s->avctx))
            return ret;
    }
    return 0;
}

/* Read the number of coding passes. */
{
        return 1;
        return 2;
}

static int getlblockinc(Jpeg2000DecoderContext *s)
{
    int res = 0, ret;
    while (ret = get_bits(s, 1)) {
        if (ret < 0)
            return ret;
        res++;
    }
    return res;
}

static inline void select_header(Jpeg2000DecoderContext *s, Jpeg2000Tile *tile,
                                 int *tp_index)
{
    s->g = tile->tile_part[*tp_index].header_tpg;
    if (bytestream2_get_bytes_left(&s->g) == 0 && s->bit_index == 8) {
        if (*tp_index < FF_ARRAY_ELEMS(tile->tile_part) - 1) {
            s->g = tile->tile_part[++(*tp_index)].tpg;
        }
    }
}

static inline void select_stream(Jpeg2000DecoderContext *s, Jpeg2000Tile *tile,
                                 int *tp_index, Jpeg2000CodingStyle *codsty)
{
    s->g = tile->tile_part[*tp_index].tpg;
    if (bytestream2_get_bytes_left(&s->g) == 0 && s->bit_index == 8) {
        if (*tp_index < FF_ARRAY_ELEMS(tile->tile_part) - 1) {
            s->g = tile->tile_part[++(*tp_index)].tpg;
        }
    }
    if (codsty->csty & JPEG2000_CSTY_SOP) {
        if (bytestream2_peek_be32(&s->g) == JPEG2000_SOP_FIXED_BYTES)
            bytestream2_skip(&s->g, JPEG2000_SOP_BYTE_LENGTH);
        else
            av_log(s->avctx, AV_LOG_ERROR, "SOP marker not found. instead %X\n", bytestream2_peek_be32(&s->g));
    }
}

static int jpeg2000_decode_packet(Jpeg2000DecoderContext *s, Jpeg2000Tile *tile, int *tp_index,
                                  Jpeg2000CodingStyle *codsty,
                                  Jpeg2000ResLevel *rlevel, int precno,
                                  int layno, uint8_t *expn, int numgbits)
{
    int bandno, cblkno, ret, nb_code_blocks;
    int cwsno;

    if (layno < rlevel->band[0].prec[precno].decoded_layers)
        return 0;
    rlevel->band[0].prec[precno].decoded_layers = layno + 1;
    // Select stream to read from
    if (s->has_ppm)
        select_header(s, tile, tp_index);
    else if (tile->has_ppt)
        s->g = tile->packed_headers_stream;
    else
        select_stream(s, tile, tp_index, codsty);

    if (!(ret = get_bits(s, 1))) {
        jpeg2000_flush(s);
        goto skip_data;
    } else if (ret < 0)
        return ret;

    for (bandno = 0; bandno < rlevel->nbands; bandno++) {
        Jpeg2000Band *band = rlevel->band + bandno;
        Jpeg2000Prec *prec = band->prec + precno;

        if (band->coord[0][0] == band->coord[0][1] ||
            band->coord[1][0] == band->coord[1][1])
            continue;
        nb_code_blocks =  prec->nb_codeblocks_height *
                          prec->nb_codeblocks_width;
        for (cblkno = 0; cblkno < nb_code_blocks; cblkno++) {
            Jpeg2000Cblk *cblk = prec->cblk + cblkno;
            int incl, newpasses, llen;
            void *tmp;

            if (cblk->npasses)
                incl = get_bits(s, 1);
            else
                incl = tag_tree_decode(s, prec->cblkincl + cblkno, layno + 1) == layno;
            if (!incl)
                continue;
            else if (incl < 0)
                return incl;

            if (!cblk->npasses) {
                int v = expn[bandno] + numgbits - 1 -
                        tag_tree_decode(s, prec->zerobits + cblkno, 100);
                if (v < 0 || v > 30) {
                    av_log(s->avctx, AV_LOG_ERROR,
                           "nonzerobits %d invalid or unsupported\n", v);
                    return AVERROR_INVALIDDATA;
                }
                cblk->nonzerobits = v;
            }
            if ((newpasses = getnpasses(s)) < 0)
                return newpasses;
            av_assert2(newpasses > 0);
            if (cblk->npasses + newpasses >= JPEG2000_MAX_PASSES) {
                avpriv_request_sample(s->avctx, "Too many passes");
                return AVERROR_PATCHWELCOME;
            }
            if ((llen = getlblockinc(s)) < 0)
                return llen;
            if (cblk->lblock + llen + av_log2(newpasses) > 16) {
                avpriv_request_sample(s->avctx,
                                      "Block with length beyond 16 bits");
                return AVERROR_PATCHWELCOME;
            }

            cblk->lblock += llen;

            cblk->nb_lengthinc = 0;
            cblk->nb_terminationsinc = 0;
            av_free(cblk->lengthinc);
            cblk->lengthinc  = av_mallocz_array(newpasses    , sizeof(*cblk->lengthinc));
            if (!cblk->lengthinc)
                return AVERROR(ENOMEM);
            tmp = av_realloc_array(cblk->data_start, cblk->nb_terminations + newpasses + 1, sizeof(*cblk->data_start));
            if (!tmp)
                return AVERROR(ENOMEM);
            cblk->data_start = tmp;
            do {
                int newpasses1 = 0;

                while (newpasses1 < newpasses) {
                    newpasses1 ++;
                    if (needs_termination(codsty->cblk_style, cblk->npasses + newpasses1 - 1)) {
                        cblk->nb_terminationsinc ++;
                        break;
                    }
                }

                if ((ret = get_bits(s, av_log2(newpasses1) + cblk->lblock)) < 0)
                    return ret;
                if (ret > cblk->data_allocated) {
                    size_t new_size = FFMAX(2*cblk->data_allocated, ret);
                    void *new = av_realloc(cblk->data, new_size);
                    if (new) {
                        cblk->data = new;
                        cblk->data_allocated = new_size;
                    }
                }
                if (ret > cblk->data_allocated) {
                    avpriv_request_sample(s->avctx,
                                        "Block with lengthinc greater than %"SIZE_SPECIFIER"",
                                        cblk->data_allocated);
                    return AVERROR_PATCHWELCOME;
                }
                cblk->lengthinc[cblk->nb_lengthinc++] = ret;
                cblk->npasses  += newpasses1;
                newpasses -= newpasses1;
            } while(newpasses);
        }
    }
    jpeg2000_flush(s);

    if (codsty->csty & JPEG2000_CSTY_EPH) {
        if (bytestream2_peek_be16(&s->g) == JPEG2000_EPH)
            bytestream2_skip(&s->g, 2);
        else
            av_log(s->avctx, AV_LOG_ERROR, "EPH marker not found. instead %X\n", bytestream2_peek_be32(&s->g));
    }

    // Save state of stream
    if (s->has_ppm) {
        tile->tile_part[*tp_index].header_tpg = s->g;
        select_stream(s, tile, tp_index, codsty);
    } else if (tile->has_ppt) {
        tile->packed_headers_stream = s->g;
        select_stream(s, tile, tp_index, codsty);
    }
    for (bandno = 0; bandno < rlevel->nbands; bandno++) {
        Jpeg2000Band *band = rlevel->band + bandno;
        Jpeg2000Prec *prec = band->prec + precno;

        nb_code_blocks = prec->nb_codeblocks_height * prec->nb_codeblocks_width;
        for (cblkno = 0; cblkno < nb_code_blocks; cblkno++) {
            Jpeg2000Cblk *cblk = prec->cblk + cblkno;
            if (!cblk->nb_terminationsinc && !cblk->lengthinc)
                continue;
            for (cwsno = 0; cwsno < cblk->nb_lengthinc; cwsno ++) {
                if (cblk->data_allocated < cblk->length + cblk->lengthinc[cwsno] + 4) {
                    size_t new_size = FFMAX(2*cblk->data_allocated, cblk->length + cblk->lengthinc[cwsno] + 4);
                    void *new = av_realloc(cblk->data, new_size);
                    if (new) {
                        cblk->data = new;
                        cblk->data_allocated = new_size;
                    }
                }
                if (   bytestream2_get_bytes_left(&s->g) < cblk->lengthinc[cwsno]
                    || cblk->data_allocated < cblk->length + cblk->lengthinc[cwsno] + 4
                ) {
                    av_log(s->avctx, AV_LOG_ERROR,
                        "Block length %"PRIu16" or lengthinc %d is too large, left %d\n",
                        cblk->length, cblk->lengthinc[cwsno], bytestream2_get_bytes_left(&s->g));
                    return AVERROR_INVALIDDATA;
                }

                bytestream2_get_bufferu(&s->g, cblk->data + cblk->length, cblk->lengthinc[cwsno]);
                cblk->length   += cblk->lengthinc[cwsno];
                cblk->lengthinc[cwsno] = 0;
                if (cblk->nb_terminationsinc) {
                    cblk->nb_terminationsinc--;
                    cblk->nb_terminations++;
                    cblk->data[cblk->length++] = 0xFF;
                    cblk->data[cblk->length++] = 0xFF;
                    cblk->data_start[cblk->nb_terminations] = cblk->length;
                }
            }
            av_freep(&cblk->lengthinc);
        }
    }
    // Save state of stream
    tile->tile_part[*tp_index].tpg = s->g;
    return 0;

skip_data:
    if (codsty->csty & JPEG2000_CSTY_EPH) {
        if (bytestream2_peek_be16(&s->g) == JPEG2000_EPH)
            bytestream2_skip(&s->g, 2);
        else
            av_log(s->avctx, AV_LOG_ERROR, "EPH marker not found. instead %X\n", bytestream2_peek_be32(&s->g));
    }
    if (s->has_ppm) {
        tile->tile_part[*tp_index].header_tpg = s->g;
        select_stream(s, tile, tp_index, codsty);
    } else if (tile->has_ppt) {
        tile->packed_headers_stream = s->g;
        select_stream(s, tile, tp_index, codsty);
    }
    tile->tile_part[*tp_index].tpg = s->g;
    return 0;
}

                                             int RSpoc, int CSpoc,
                                             int LYEpoc, int REpoc, int CEpoc,
                                             int Ppoc, int *tp_index)
{

    case JPEG2000_PGOD_RLCP:
        av_log(s->avctx, AV_LOG_DEBUG, "Progression order RLCP\n");
        ok_reslevel = 1;
        for (reslevelno = RSpoc; ok_reslevel && reslevelno < REpoc; reslevelno++) {
            ok_reslevel = 0;
            for (layno = 0; layno < LYEpoc; layno++) {
                for (compno = CSpoc; compno < CEpoc; compno++) {
                    Jpeg2000CodingStyle *codsty = tile->codsty + compno;
                    Jpeg2000QuantStyle *qntsty  = tile->qntsty + compno;
                    if (reslevelno < codsty->nreslevels) {
                        Jpeg2000ResLevel *rlevel = tile->comp[compno].reslevel +
                                                reslevelno;
                        ok_reslevel = 1;
                        for (precno = 0; precno < rlevel->num_precincts_x * rlevel->num_precincts_y; precno++)
                            if ((ret = jpeg2000_decode_packet(s, tile, tp_index,
                                                              codsty, rlevel,
                                                              precno, layno,
                                                              qntsty->expn + (reslevelno ? 3 * (reslevelno - 1) + 1 : 0),
                                                              qntsty->nguardbits)) < 0)
                                return ret;
                    }
                }
            }
        }
        break;

            ok_reslevel = 1;
                ok_reslevel = 0;
                                                reslevelno;
                                                              codsty, rlevel,
                                                              precno, layno,
                                return ret;
                    }
                }
            }
        }
        break;


                continue;

            }
                avpriv_request_sample(s->avctx, "CPRL with large step");
                return AVERROR_PATCHWELCOME;
            }




                        // check if a precinct exists


                            av_log(s->avctx, AV_LOG_WARNING, "prc %d %d outside limits %d %d\n",
                                   prcx, prcy, rlevel->num_precincts_x, rlevel->num_precincts_y);
                            continue;
                        }

                                                              precno, layno,
                                return ret;
                        }
                    }
                }
            }
        }
        break;

    case JPEG2000_PGOD_RPCL:
        av_log(s->avctx, AV_LOG_WARNING, "Progression order RPCL\n");
        ok_reslevel = 1;
        for (reslevelno = RSpoc; ok_reslevel && reslevelno < REpoc; reslevelno++) {
            ok_reslevel = 0;
            step_x = 30;
            step_y = 30;
            for (compno = CSpoc; compno < CEpoc; compno++) {
                Jpeg2000Component *comp     = tile->comp + compno;
                Jpeg2000CodingStyle *codsty = tile->codsty + compno;

                if (reslevelno < codsty->nreslevels) {
                    uint8_t reducedresno = codsty->nreslevels - 1 -reslevelno; //  ==> N_L - r
                    Jpeg2000ResLevel *rlevel = comp->reslevel + reslevelno;
                    step_x = FFMIN(step_x, rlevel->log2_prec_width  + reducedresno);
                    step_y = FFMIN(step_y, rlevel->log2_prec_height + reducedresno);
                }
            }
            step_x = 1<<step_x;
            step_y = 1<<step_y;

            for (y = tile->coord[1][0]; y < tile->coord[1][1]; y = (y/step_y + 1)*step_y) {
                for (x = tile->coord[0][0]; x < tile->coord[0][1]; x = (x/step_x + 1)*step_x) {
                    for (compno = CSpoc; compno < CEpoc; compno++) {
                        Jpeg2000Component *comp     = tile->comp + compno;
                        Jpeg2000CodingStyle *codsty = tile->codsty + compno;
                        Jpeg2000QuantStyle *qntsty  = tile->qntsty + compno;
                        uint8_t reducedresno = codsty->nreslevels - 1 -reslevelno; //  ==> N_L - r
                        Jpeg2000ResLevel *rlevel = comp->reslevel + reslevelno;
                        unsigned prcx, prcy;
                        int trx0, try0;

                        if (!s->cdx[compno] || !s->cdy[compno])
                            return AVERROR_INVALIDDATA;

                        if (reslevelno >= codsty->nreslevels)
                            continue;

                        trx0 = ff_jpeg2000_ceildiv(tile->coord[0][0], (int64_t)s->cdx[compno] << reducedresno);
                        try0 = ff_jpeg2000_ceildiv(tile->coord[1][0], (int64_t)s->cdy[compno] << reducedresno);

                        if (!(y % ((uint64_t)s->cdy[compno] << (rlevel->log2_prec_height + reducedresno)) == 0 ||
                             (y == tile->coord[1][0] && ((int64_t)try0 << reducedresno) % (1ULL << (reducedresno + rlevel->log2_prec_height)))))
                            continue;

                        if (!(x % ((uint64_t)s->cdx[compno] << (rlevel->log2_prec_width + reducedresno)) == 0 ||
                             (x == tile->coord[0][0] && ((int64_t)trx0 << reducedresno) % (1ULL << (reducedresno + rlevel->log2_prec_width)))))
                            continue;

                        // check if a precinct exists
                        prcx   = ff_jpeg2000_ceildiv(x, (int64_t)s->cdx[compno] << reducedresno) >> rlevel->log2_prec_width;
                        prcy   = ff_jpeg2000_ceildiv(y, (int64_t)s->cdy[compno] << reducedresno) >> rlevel->log2_prec_height;
                        prcx  -= ff_jpeg2000_ceildivpow2(comp->coord_o[0][0], reducedresno) >> rlevel->log2_prec_width;
                        prcy  -= ff_jpeg2000_ceildivpow2(comp->coord_o[1][0], reducedresno) >> rlevel->log2_prec_height;

                        precno = prcx + rlevel->num_precincts_x * prcy;

                        ok_reslevel = 1;
                        if (prcx >= rlevel->num_precincts_x || prcy >= rlevel->num_precincts_y) {
                            av_log(s->avctx, AV_LOG_WARNING, "prc %d %d outside limits %d %d\n",
                                   prcx, prcy, rlevel->num_precincts_x, rlevel->num_precincts_y);
                            continue;
                        }

                        for (layno = 0; layno < LYEpoc; layno++) {
                            if ((ret = jpeg2000_decode_packet(s, tile, tp_index,
                                                              codsty, rlevel,
                                                              precno, layno,
                                                              qntsty->expn + (reslevelno ? 3 * (reslevelno - 1) + 1 : 0),
                                                              qntsty->nguardbits)) < 0)
                                return ret;
                        }
                    }
                }
            }
        }
        break;

    case JPEG2000_PGOD_PCRL:
        av_log(s->avctx, AV_LOG_WARNING, "Progression order PCRL\n");
        step_x = 32;
        step_y = 32;
        for (compno = CSpoc; compno < CEpoc; compno++) {
            Jpeg2000Component *comp     = tile->comp + compno;
            Jpeg2000CodingStyle *codsty = tile->codsty + compno;

            for (reslevelno = RSpoc; reslevelno < FFMIN(codsty->nreslevels, REpoc); reslevelno++) {
                uint8_t reducedresno = codsty->nreslevels - 1 -reslevelno; //  ==> N_L - r
                Jpeg2000ResLevel *rlevel = comp->reslevel + reslevelno;
                step_x = FFMIN(step_x, rlevel->log2_prec_width  + reducedresno);
                step_y = FFMIN(step_y, rlevel->log2_prec_height + reducedresno);
            }
        }
        if (step_x >= 31 || step_y >= 31){
            avpriv_request_sample(s->avctx, "PCRL with large step");
            return AVERROR_PATCHWELCOME;
        }
        step_x = 1<<step_x;
        step_y = 1<<step_y;

        for (y = tile->coord[1][0]; y < tile->coord[1][1]; y = (y/step_y + 1)*step_y) {
            for (x = tile->coord[0][0]; x < tile->coord[0][1]; x = (x/step_x + 1)*step_x) {
                for (compno = CSpoc; compno < CEpoc; compno++) {
                    Jpeg2000Component *comp     = tile->comp + compno;
                    Jpeg2000CodingStyle *codsty = tile->codsty + compno;
                    Jpeg2000QuantStyle *qntsty  = tile->qntsty + compno;

                    if (!s->cdx[compno] || !s->cdy[compno])
                        return AVERROR_INVALIDDATA;

                    for (reslevelno = RSpoc; reslevelno < FFMIN(codsty->nreslevels, REpoc); reslevelno++) {
                        unsigned prcx, prcy;
                        uint8_t reducedresno = codsty->nreslevels - 1 -reslevelno; //  ==> N_L - r
                        Jpeg2000ResLevel *rlevel = comp->reslevel + reslevelno;
                        int trx0, try0;

                        trx0 = ff_jpeg2000_ceildiv(tile->coord[0][0], (int64_t)s->cdx[compno] << reducedresno);
                        try0 = ff_jpeg2000_ceildiv(tile->coord[1][0], (int64_t)s->cdy[compno] << reducedresno);

                        if (!(y % ((uint64_t)s->cdy[compno] << (rlevel->log2_prec_height + reducedresno)) == 0 ||
                             (y == tile->coord[1][0] && ((int64_t)try0 << reducedresno) % (1ULL << (reducedresno + rlevel->log2_prec_height)))))
                             continue;

                        if (!(x % ((uint64_t)s->cdx[compno] << (rlevel->log2_prec_width + reducedresno)) == 0 ||
                             (x == tile->coord[0][0] && ((int64_t)trx0 << reducedresno) % (1ULL << (reducedresno + rlevel->log2_prec_width)))))
                             continue;

                        // check if a precinct exists
                        prcx   = ff_jpeg2000_ceildiv(x, (int64_t)s->cdx[compno] << reducedresno) >> rlevel->log2_prec_width;
                        prcy   = ff_jpeg2000_ceildiv(y, (int64_t)s->cdy[compno] << reducedresno) >> rlevel->log2_prec_height;
                        prcx  -= ff_jpeg2000_ceildivpow2(comp->coord_o[0][0], reducedresno) >> rlevel->log2_prec_width;
                        prcy  -= ff_jpeg2000_ceildivpow2(comp->coord_o[1][0], reducedresno) >> rlevel->log2_prec_height;

                        precno = prcx + rlevel->num_precincts_x * prcy;

                        if (prcx >= rlevel->num_precincts_x || prcy >= rlevel->num_precincts_y) {
                            av_log(s->avctx, AV_LOG_WARNING, "prc %d %d outside limits %d %d\n",
                                   prcx, prcy, rlevel->num_precincts_x, rlevel->num_precincts_y);
                            continue;
                        }

                        for (layno = 0; layno < LYEpoc; layno++) {
                            if ((ret = jpeg2000_decode_packet(s, tile, tp_index, codsty, rlevel,
                                                              precno, layno,
                                                              qntsty->expn + (reslevelno ? 3 * (reslevelno - 1) + 1 : 0),
                                                              qntsty->nguardbits)) < 0)
                                return ret;
                        }
                    }
                }
            }
        }
        break;

    default:
        break;
    }

    return ret;
}

{

        for (i=0; i<tile->poc.nb_poc; i++) {
            Jpeg2000POCEntry *e = &tile->poc.poc[i];
            ret = jpeg2000_decode_packets_po_iteration(s, tile,
                e->RSpoc, e->CSpoc,
                FFMIN(e->LYEpoc, tile->codsty[0].nlayers),
                e->REpoc,
                FFMIN(e->CEpoc, s->ncomponents),
                e->Ppoc, &tp_index
                );
            if (ret < 0)
                return ret;
        }
    } else {
            0, 0,
            33,
            s->ncomponents,
            &tp_index
        );
    }
    /* EOC marker reached */

}

/* TIER-1 routines */
                           int bpno, int bandno,
                           int vert_causal_ctx_csty_symbol)
{

                    flags_mask &= ~(JPEG2000_T1_SIG_S | JPEG2000_T1_SIG_SW | JPEG2000_T1_SIG_SE | JPEG2000_T1_SGN_S);
                             t1->data[(y) * t1->stride + x] = ff_mqc_decode(&t1->mqc, t1->mqc.cx_states + ctxno) ? -mask : mask;
                        else

                    }
                }
            }

                           int bpno, int vert_causal_ctx_csty_symbol)
{


                    int flags_mask = (vert_causal_ctx_csty_symbol && y == y0 + 3) ?
                }

static void decode_clnpass(Jpeg2000DecoderContext *s, Jpeg2000T1Context *t1,
                           int width, int height, int bpno, int bandno,
                           int seg_symbols, int vert_causal_ctx_csty_symbol)
{
    int mask = 3 << (bpno - 1), y0, x, y, runlen, dec;

    for (y0 = 0; y0 < height; y0 += 4) {
        for (x = 0; x < width; x++) {
            int flags_mask = -1;
            if (vert_causal_ctx_csty_symbol)
                flags_mask &= ~(JPEG2000_T1_SIG_S | JPEG2000_T1_SIG_SW | JPEG2000_T1_SIG_SE | JPEG2000_T1_SGN_S);
            if (y0 + 3 < height &&
                !((t1->flags[(y0 + 1) * t1->stride + x + 1] & (JPEG2000_T1_SIG_NB | JPEG2000_T1_VIS | JPEG2000_T1_SIG)) ||
                  (t1->flags[(y0 + 2) * t1->stride + x + 1] & (JPEG2000_T1_SIG_NB | JPEG2000_T1_VIS | JPEG2000_T1_SIG)) ||
                  (t1->flags[(y0 + 3) * t1->stride + x + 1] & (JPEG2000_T1_SIG_NB | JPEG2000_T1_VIS | JPEG2000_T1_SIG)) ||
                  (t1->flags[(y0 + 4) * t1->stride + x + 1] & (JPEG2000_T1_SIG_NB | JPEG2000_T1_VIS | JPEG2000_T1_SIG) & flags_mask))) {
                if (!ff_mqc_decode(&t1->mqc, t1->mqc.cx_states + MQC_CX_RL))
                    continue;
                runlen = ff_mqc_decode(&t1->mqc,
                                       t1->mqc.cx_states + MQC_CX_UNI);
                runlen = (runlen << 1) | ff_mqc_decode(&t1->mqc,
                                                       t1->mqc.cx_states +
                                                       MQC_CX_UNI);
                dec = 1;
            } else {
                runlen = 0;
                dec    = 0;
            }

            for (y = y0 + runlen; y < y0 + 4 && y < height; y++) {
                int flags_mask = -1;
                if (vert_causal_ctx_csty_symbol && y == y0 + 3)
                    flags_mask &= ~(JPEG2000_T1_SIG_S | JPEG2000_T1_SIG_SW | JPEG2000_T1_SIG_SE | JPEG2000_T1_SGN_S);
                if (!dec) {
                    if (!(t1->flags[(y+1) * t1->stride + x+1] & (JPEG2000_T1_SIG | JPEG2000_T1_VIS))) {
                        dec = ff_mqc_decode(&t1->mqc, t1->mqc.cx_states + ff_jpeg2000_getsigctxno(t1->flags[(y+1) * t1->stride + x+1] & flags_mask,
                                                                                             bandno));
                    }
                }
                if (dec) {
                    int xorbit;
                    int ctxno = ff_jpeg2000_getsgnctxno(t1->flags[(y + 1) * t1->stride + x + 1] & flags_mask,
                                                        &xorbit);
                    t1->data[(y) * t1->stride + x] = (ff_mqc_decode(&t1->mqc,
                                                    t1->mqc.cx_states + ctxno) ^
                                      xorbit)
                                     ? -mask : mask;
                    ff_jpeg2000_set_significance(t1, x, y, t1->data[(y) * t1->stride + x] < 0);
                }
                dec = 0;
                t1->flags[(y + 1) * t1->stride + x + 1] &= ~JPEG2000_T1_VIS;
            }
        }
    }
    if (seg_symbols) {
        int val;
        val = ff_mqc_decode(&t1->mqc, t1->mqc.cx_states + MQC_CX_UNI);
        val = (val << 1) + ff_mqc_decode(&t1->mqc, t1->mqc.cx_states + MQC_CX_UNI);
        val = (val << 1) + ff_mqc_decode(&t1->mqc, t1->mqc.cx_states + MQC_CX_UNI);
        val = (val << 1) + ff_mqc_decode(&t1->mqc, t1->mqc.cx_states + MQC_CX_UNI);
        if (val != 0xa)
            av_log(s->avctx, AV_LOG_ERROR,
                   "Segmentation symbol value incorrect\n");
    }
}

                       Jpeg2000T1Context *t1, Jpeg2000Cblk *cblk,
                       int width, int height, int bandpos, uint8_t roi_shift)
{



    /* If code-block contains no compressed data: nothing to do. */
        return 0;



            av_log(s->avctx, AV_LOG_ERROR, "bpno became invalid\n");
            return AVERROR_INVALIDDATA;
        }
                           vert_causal_ctx_csty_symbol);
                           vert_causal_ctx_csty_symbol);
        }
            ff_mqc_init_contexts(&t1->mqc);

            if (term_cnt >= cblk->nb_terminations) {
                av_log(s->avctx, AV_LOG_ERROR, "Missing needed termination \n");
                return AVERROR_INVALIDDATA;
            }
            if (FFABS(cblk->data + cblk->data_start[term_cnt + 1] - 2 - t1->mqc.bp) > 0) {
                av_log(s->avctx, AV_LOG_WARNING, "Mid mismatch %"PTRDIFF_SPECIFIER" in pass %d of %d\n",
                    cblk->data + cblk->data_start[term_cnt + 1] - 2 - t1->mqc.bp,
                    pass_cnt, cblk->npasses);
            }

            ff_mqc_initdec(&t1->mqc, cblk->data + cblk->data_start[++term_cnt], coder_type == 2, 0);
        }

        }
    }

        av_log(s->avctx, AV_LOG_WARNING, "End mismatch %"PTRDIFF_SPECIFIER"\n",
               cblk->data + cblk->length - 2 - t1->mqc.bp);
    }

        av_log(s->avctx, AV_LOG_WARNING, "Synthetic End of Stream Marker Read.\n");
    }

    return 1;
}

static inline int roi_shift_param(Jpeg2000Component *comp,
                                   int quan_parameter)
{
    uint8_t roi_shift;
    int val;
    roi_shift = comp->roi_shift;
    val = (quan_parameter < 0)?-quan_parameter:quan_parameter;

    if (val > (1 << roi_shift))
        return (quan_parameter < 0)?-(val >> roi_shift):(val >> roi_shift);
    return quan_parameter;
}

/* TODO: Verify dequantization for lossless case
 * comp->data can be float or int
 * band->stepsize can be float or int
 * depending on the type of DWT transformation.
 * see ISO/IEC 15444-1:2002 A.6.1 */

/* Float dequantization of a codeblock.*/
static void dequantization_float(int x, int y, Jpeg2000Cblk *cblk,
                                 Jpeg2000Component *comp,
                                 Jpeg2000T1Context *t1, Jpeg2000Band *band)
{
    int i, j;
    int w = cblk->coord[0][1] - cblk->coord[0][0];
    for (j = 0; j < (cblk->coord[1][1] - cblk->coord[1][0]); ++j) {
        float *datap = &comp->f_data[(comp->coord[0][1] - comp->coord[0][0]) * (y + j) + x];
        int *src = t1->data + j*t1->stride;
        for (i = 0; i < w; ++i)
            datap[i] = src[i] * band->f_stepsize;
    }
}

/* Integer dequantization of a codeblock.*/
static void dequantization_int(int x, int y, Jpeg2000Cblk *cblk,
                               Jpeg2000Component *comp,
                               Jpeg2000T1Context *t1, Jpeg2000Band *band)
{
    int i, j;
    int w = cblk->coord[0][1] - cblk->coord[0][0];
    for (j = 0; j < (cblk->coord[1][1] - cblk->coord[1][0]); ++j) {
        int32_t *datap = &comp->i_data[(comp->coord[0][1] - comp->coord[0][0]) * (y + j) + x];
        int *src = t1->data + j*t1->stride;
        if (band->i_stepsize == 32768) {
            for (i = 0; i < w; ++i)
                datap[i] = src[i] / 2;
        } else {
            // This should be VERY uncommon
            for (i = 0; i < w; ++i)
                datap[i] = (src[i] * (int64_t)band->i_stepsize) / 65536;
        }
    }
}

static void dequantization_int_97(int x, int y, Jpeg2000Cblk *cblk,
                               Jpeg2000Component *comp,
                               Jpeg2000T1Context *t1, Jpeg2000Band *band)
{
    int i, j;
    int w = cblk->coord[0][1] - cblk->coord[0][0];
    for (j = 0; j < (cblk->coord[1][1] - cblk->coord[1][0]); ++j) {
        int32_t *datap = &comp->i_data[(comp->coord[0][1] - comp->coord[0][0]) * (y + j) + x];
        int *src = t1->data + j*t1->stride;
        for (i = 0; i < w; ++i)
            datap[i] = (src[i] * (int64_t)band->i_stepsize + (1<<15)) >> 16;
    }
}

{

            av_log(s->avctx, AV_LOG_ERROR, "Transforms mismatch, MCT not supported\n");
            return;
        }
            av_log(s->avctx, AV_LOG_ERROR, "Coords mismatch, MCT not supported\n");
            return;
        }
    }

            src[i] = tile->comp[i].f_data;
        else


}

static inline void roi_scale_cblk(Jpeg2000Cblk *cblk,
                                  Jpeg2000Component *comp,
                                  Jpeg2000T1Context *t1)
{
    int i, j;
    int w = cblk->coord[0][1] - cblk->coord[0][0];
    for (j = 0; j < (cblk->coord[1][1] - cblk->coord[1][0]); ++j) {
        int *src = t1->data + j*t1->stride;
        for (i = 0; i < w; ++i)
            src[i] = roi_shift_param(comp, src[i]);
    }
}

{


    /* Loop on tile components */


        /* Loop on resolution levels */
            /* Loop on bands */



                /* Loop on precincts */

                    /* Loop on codeblocks */
                        else

                            roi_scale_cblk(cblk, comp, &t1);
                        else
                   } /* end cblk */
                } /*end prec */
            } /* end band */
        } /* end reslevel */

        /* inverse DWT */

    } /*end comp */

#define WRITE_FRAME(D, PIXEL)                                                                     \
    static inline void write_frame_ ## D(Jpeg2000DecoderContext * s, Jpeg2000Tile * tile,         \
                                         AVFrame * picture, int precision)                        \
    {                                                                                             \
        const AVPixFmtDescriptor *pixdesc = av_pix_fmt_desc_get(s->avctx->pix_fmt);               \
        int planar    = !!(pixdesc->flags & AV_PIX_FMT_FLAG_PLANAR);                              \
        int pixelsize = planar ? 1 : pixdesc->nb_components;                                      \
                                                                                                  \
        int compno;                                                                               \
        int x, y;                                                                                 \
                                                                                                  \
        for (compno = 0; compno < s->ncomponents; compno++) {                                     \
            Jpeg2000Component *comp     = tile->comp + compno;                                    \
            Jpeg2000CodingStyle *codsty = tile->codsty + compno;                                  \
            PIXEL *line;                                                                          \
            float *datap     = comp->f_data;                                                      \
            int32_t *i_datap = comp->i_data;                                                      \
            int cbps         = s->cbps[compno];                                                   \
            int w            = tile->comp[compno].coord[0][1] -                                   \
                               ff_jpeg2000_ceildiv(s->image_offset_x, s->cdx[compno]);            \
            int h            = tile->comp[compno].coord[1][1] -                                   \
                               ff_jpeg2000_ceildiv(s->image_offset_y, s->cdy[compno]);            \
            int plane        = 0;                                                                 \
                                                                                                  \
            if (planar)                                                                           \
                plane = s->cdef[compno] ? s->cdef[compno]-1 : (s->ncomponents-1);                 \
                                                                                                  \
            y    = tile->comp[compno].coord[1][0] -                                               \
                   ff_jpeg2000_ceildiv(s->image_offset_y, s->cdy[compno]);                        \
            line = (PIXEL *)picture->data[plane] + y * (picture->linesize[plane] / sizeof(PIXEL));\
            for (; y < h; y++) {                                                                  \
                PIXEL *dst;                                                                       \
                                                                                                  \
                x   = tile->comp[compno].coord[0][0] -                                            \
                      ff_jpeg2000_ceildiv(s->image_offset_x, s->cdx[compno]);                     \
                dst = line + x * pixelsize + compno*!planar;                                      \
                                                                                                  \
                if (codsty->transform == FF_DWT97) {                                              \
                    for (; x < w; x++) {                                                          \
                        int val = lrintf(*datap) + (1 << (cbps - 1));                             \
                        /* DC level shift and clip see ISO 15444-1:2002 G.1.2 */                  \
                        val  = av_clip(val, 0, (1 << cbps) - 1);                                  \
                        *dst = val << (precision - cbps);                                         \
                        datap++;                                                                  \
                        dst += pixelsize;                                                         \
                    }                                                                             \
                } else {                                                                          \
                    for (; x < w; x++) {                                                          \
                        int val = *i_datap + (1 << (cbps - 1));                                   \
                        /* DC level shift and clip see ISO 15444-1:2002 G.1.2 */                  \
                        val  = av_clip(val, 0, (1 << cbps) - 1);                                  \
                        *dst = val << (precision - cbps);                                         \
                        i_datap++;                                                                \
                        dst += pixelsize;                                                         \
                    }                                                                             \
                }                                                                                 \
                line += picture->linesize[plane] / sizeof(PIXEL);                                 \
            }                                                                                     \
        }                                                                                         \
                                                                                                  \
    }


#undef WRITE_FRAME

                                int jobnr, int threadnr)
{


    /* inverse MCT transformation */

            }
            break;
        }
    }

    } else {
                        picture->format == AV_PIX_FMT_RGBA64 ||

    }

}

{

            }
        }
    }

{


            av_log(s->avctx, AV_LOG_ERROR, "Missing EOC\n");
            break;
        }

            continue;

                av_log(s->avctx, AV_LOG_ERROR, "Missing SIZ\n");
                return AVERROR_INVALIDDATA;
            }
                av_log(s->avctx, AV_LOG_ERROR, "Missing SOT\n");
                return AVERROR_INVALIDDATA;
            }

                av_log(s->avctx, AV_LOG_ERROR, "Invalid tpend\n");
                return AVERROR_INVALIDDATA;
            }

                uint32_t tp_header_size = bytestream2_get_be32(&s->packed_headers_stream);
                if (bytestream2_get_bytes_left(&s->packed_headers_stream) < tp_header_size)
                    return AVERROR_INVALIDDATA;
                bytestream2_init(&tp->header_tpg, s->packed_headers_stream.buffer, tp_header_size);
                bytestream2_skip(&s->packed_headers_stream, tp_header_size);
            }
                bytestream2_init(&tile->packed_headers_stream, tile->packed_headers, tile->packed_headers_size);
            }


        }
            break;

            if (s->avctx->strict_std_compliance >= FF_COMPLIANCE_STRICT) {
                av_log(s->avctx, AV_LOG_ERROR, "Invalid len %d left=%d\n", len, bytestream2_get_bytes_left(&s->g));
                return AVERROR_INVALIDDATA;
            }
            av_log(s->avctx, AV_LOG_WARNING, "Missing EOC Marker.\n");
            break;
        }

                av_log(s->avctx, AV_LOG_ERROR, "Duplicate SIZ\n");
                return AVERROR_INVALIDDATA;
            }
                s->numXtiles = s->numYtiles = 0;
            break;
        case JPEG2000_RGN:
            ret = get_rgn(s, len);
            break;
        case JPEG2000_POC:
            ret = get_poc(s, len, poc);
            break;
                    bytestream2_init(&s->packed_headers_stream, s->packed_headers, s->packed_headers_size);
                }
            }
            }
            break;
            // the PLM marker is ignored
        case JPEG2000_COM:
            // the comment is ignored
            break;
        case JPEG2000_CRG:
            ret = read_crg(s, len);
            break;
            // Tile-part lengths
        case JPEG2000_PLT:
            // Packet length, tile-part header
            ret = get_plt(s, len);
            break;
        case JPEG2000_PPM:
            // Packed headers, main header
            if (s->in_tile_headers) {
                av_log(s->avctx, AV_LOG_ERROR, "PPM Marker can only be in Main header\n");
                return AVERROR_INVALIDDATA;
            }
            ret = get_ppm(s, len);
            break;
        case JPEG2000_PPT:
            // Packed headers, tile-part header
            if (s->has_ppm) {
                av_log(s->avctx, AV_LOG_ERROR,
                       "Cannot have both PPT and PPM marker.\n");
                return AVERROR_INVALIDDATA;
            }

            ret = get_ppt(s, len);
            break;
        default:
            av_log(s->avctx, AV_LOG_ERROR,
                   "unsupported marker 0x%.4"PRIX16" at pos 0x%X\n",
                   marker, bytestream2_tell(&s->g) - 4);
            bytestream2_skip(&s->g, len - 2);
            break;
        }
            av_log(s->avctx, AV_LOG_ERROR,
                   "error during processing marker segment %.4"PRIx16"\n",
                   marker);
            return ret ? ret : -1;
        }
    }
    return 0;
}

/* Read bit stream packets --> T2 operation. */
{


            return ret;

            return ret;
    }

    return 0;
}

{

            if (bytestream2_get_be32u(&s->g)) {
                avpriv_request_sample(s->avctx, "Huge atom");
                return 0;
            }
            atom_size = bytestream2_get_be32u(&s->g);
            atom_end  = bytestream2_tell(&s->g) + atom_size - 16;
        } else {
        }

            return 1;

            return 0;

                    break;
                    return 1;
                    }
                    int i, size, colour_count, colour_channels, colour_depth[3];
                    colour_count = bytestream2_get_be16u(&s->g);
                    colour_channels = bytestream2_get_byteu(&s->g);
                    // FIXME: Do not ignore channel_sign
                    colour_depth[0] = (bytestream2_get_byteu(&s->g) & 0x7f) + 1;
                    colour_depth[1] = (bytestream2_get_byteu(&s->g) & 0x7f) + 1;
                    colour_depth[2] = (bytestream2_get_byteu(&s->g) & 0x7f) + 1;
                    size = (colour_depth[0] + 7 >> 3) * colour_count +
                           (colour_depth[1] + 7 >> 3) * colour_count +
                           (colour_depth[2] + 7 >> 3) * colour_count;
                    if (colour_count > AVPALETTE_COUNT ||
                        colour_channels != 3 ||
                        colour_depth[0] > 16 ||
                        colour_depth[1] > 16 ||
                        colour_depth[2] > 16 ||
                        atom2_size < size) {
                        avpriv_request_sample(s->avctx, "Unknown palette");
                        bytestream2_seek(&s->g, atom2_end, SEEK_SET);
                        continue;
                    }
                    s->pal8 = 1;
                    for (i = 0; i < colour_count; i++) {
                        uint32_t r, g, b;
                        if (colour_depth[0] <= 8) {
                            r = bytestream2_get_byteu(&s->g) << 8 - colour_depth[0];
                            r |= r >> colour_depth[0];
                        } else {
                            r = bytestream2_get_be16u(&s->g) >> colour_depth[0] - 8;
                        }
                        if (colour_depth[1] <= 8) {
                            g = bytestream2_get_byteu(&s->g) << 8 - colour_depth[1];
                            g |= g >> colour_depth[1];
                        } else {
                            g = bytestream2_get_be16u(&s->g) >> colour_depth[1] - 8;
                        }
                        if (colour_depth[2] <= 8) {
                            b = bytestream2_get_byteu(&s->g) << 8 - colour_depth[2];
                            b |= b >> colour_depth[2];
                        } else {
                            b = bytestream2_get_be16u(&s->g) >> colour_depth[2] - 8;
                        }
                        s->palette[i] = 0xffu << 24 | r << 16 | g << 8 | b;
                    }
                    int n = bytestream2_get_be16u(&s->g);
                    for (; n>0; n--) {
                        int cn   = bytestream2_get_be16(&s->g);
                        int av_unused typ  = bytestream2_get_be16(&s->g);
                        int asoc = bytestream2_get_be16(&s->g);
                        if (cn < 4 && asoc < 4)
                            s->cdef[cn] = asoc;
                    }
                    int64_t vnum, vden, hnum, hden, vexp, hexp;
                    uint32_t resx;
                    bytestream2_skip(&s->g, 4);
                    resx = bytestream2_get_be32u(&s->g);
                    if (resx != MKBETAG('r','e','s','c') && resx != MKBETAG('r','e','s','d')) {
                        bytestream2_seek(&s->g, atom2_end, SEEK_SET);
                        continue;
                    }
                    vnum = bytestream2_get_be16u(&s->g);
                    vden = bytestream2_get_be16u(&s->g);
                    hnum = bytestream2_get_be16u(&s->g);
                    hden = bytestream2_get_be16u(&s->g);
                    vexp = bytestream2_get_byteu(&s->g);
                    hexp = bytestream2_get_byteu(&s->g);
                    if (!vnum || !vden || !hnum || !hden) {
                        bytestream2_seek(&s->g, atom2_end, SEEK_SET);
                        av_log(s->avctx, AV_LOG_WARNING, "RES box invalid\n");
                        continue;
                    }
                    if (vexp > hexp) {
                        vexp -= hexp;
                        hexp = 0;
                    } else {
                        hexp -= vexp;
                        vexp = 0;
                    }
                    if (   INT64_MAX / (hnum * vden) > pow(10, hexp)
                        && INT64_MAX / (vnum * hden) > pow(10, vexp))
                        av_reduce(&s->sar.den, &s->sar.num,
                                  hnum * vden * pow(10, hexp),
                                  vnum * hden * pow(10, vexp),
                                  INT32_MAX);
                }
        } else {
        }
    }

    return 0;
}

{

{


}

                                 int *got_frame, AVPacket *avpkt)
{


        ret = AVERROR_INVALIDDATA;
        goto end;
    }

    // check if the image is in jp2 format
            av_log(avctx, AV_LOG_ERROR,
                   "Could not find Jpeg2000 codestream atom.\n");
            ret = AVERROR_INVALIDDATA;
            goto end;
        }
    } else {
    }


        av_log(avctx, AV_LOG_ERROR, "SOC marker not present\n");
        ret = AVERROR_INVALIDDATA;
        goto end;
    }
        goto end;

    /* get picture buffer */
        goto end;

        goto end;




        memcpy(picture->data[1], s->palette, 256 * sizeof(uint32_t));
        avctx->sample_aspect_ratio = s->sar;


end:
    jpeg2000_dec_cleanup(s);
    return ret;
}

#define OFFSET(x) offsetof(Jpeg2000DecoderContext, x)
#define VD AV_OPT_FLAG_VIDEO_PARAM | AV_OPT_FLAG_DECODING_PARAM

static const AVOption options[] = {
    { "lowres",  "Lower the decoding resolution by a power of two",
        OFFSET(reduction_factor), AV_OPT_TYPE_INT, { .i64 = 0 }, 0, JPEG2000_MAX_RESLEVELS - 1, VD },
    { NULL },
};

static const AVClass jpeg2000_class = {
    .class_name = "jpeg2000",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
};

AVCodec ff_jpeg2000_decoder = {
    .name             = "jpeg2000",
    .long_name        = NULL_IF_CONFIG_SMALL("JPEG 2000"),
    .type             = AVMEDIA_TYPE_VIDEO,
    .id               = AV_CODEC_ID_JPEG2000,
    .capabilities     = AV_CODEC_CAP_SLICE_THREADS | AV_CODEC_CAP_FRAME_THREADS | AV_CODEC_CAP_DR1,
    .priv_data_size   = sizeof(Jpeg2000DecoderContext),
    .init             = jpeg2000_decode_init,
    .decode           = jpeg2000_decode_frame,
    .priv_class       = &jpeg2000_class,
    .max_lowres       = 5,
    .profiles         = NULL_IF_CONFIG_SMALL(ff_jpeg2000_profiles)
};
