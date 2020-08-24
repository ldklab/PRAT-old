/*
 * Go2Webinar / Go2Meeting decoder
 * Copyright (c) 2012 Konstantin Shishkov
 * Copyright (c) 2013 Maxim Poliakovski
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
 * Go2Webinar / Go2Meeting decoder
 */

#include <inttypes.h>
#include <zlib.h>

#include "libavutil/imgutils.h"
#include "libavutil/intreadwrite.h"

#include "avcodec.h"
#include "blockdsp.h"
#include "bytestream.h"
#include "elsdec.h"
#include "get_bits.h"
#include "idctdsp.h"
#include "internal.h"
#include "jpegtables.h"
#include "mjpeg.h"

#define EPIC_PIX_STACK_SIZE 1024
#define EPIC_PIX_STACK_MAX  (EPIC_PIX_STACK_SIZE - 1)

enum ChunkType {
    DISPLAY_INFO = 0xC8,
    TILE_DATA,
    CURSOR_POS,
    CURSOR_SHAPE,
    CHUNK_CC,
    CHUNK_CD
};

enum Compression {
    COMPR_EPIC_J_B = 2,
    COMPR_KEMPF_J_B,
};

static const uint8_t luma_quant[64] = {
     8,  6,  5,  8, 12, 20, 26, 31,
     6,  6,  7, 10, 13, 29, 30, 28,
     7,  7,  8, 12, 20, 29, 35, 28,
     7,  9, 11, 15, 26, 44, 40, 31,
     9, 11, 19, 28, 34, 55, 52, 39,
    12, 18, 28, 32, 41, 52, 57, 46,
    25, 32, 39, 44, 52, 61, 60, 51,
    36, 46, 48, 49, 56, 50, 52, 50
};

static const uint8_t chroma_quant[64] = {
     9,  9, 12, 24, 50, 50, 50, 50,
     9, 11, 13, 33, 50, 50, 50, 50,
    12, 13, 28, 50, 50, 50, 50, 50,
    24, 33, 50, 50, 50, 50, 50, 50,
    50, 50, 50, 50, 50, 50, 50, 50,
    50, 50, 50, 50, 50, 50, 50, 50,
    50, 50, 50, 50, 50, 50, 50, 50,
    50, 50, 50, 50, 50, 50, 50, 50,
};

typedef struct ePICPixListElem {
    struct ePICPixListElem *next;
    uint32_t               pixel;
    uint8_t                rung;
} ePICPixListElem;

typedef struct ePICPixHashElem {
    uint32_t                pix_id;
    struct ePICPixListElem  *list;
} ePICPixHashElem;

#define EPIC_HASH_SIZE 256
typedef struct ePICPixHash {
    ePICPixHashElem *bucket[EPIC_HASH_SIZE];
    int              bucket_size[EPIC_HASH_SIZE];
    int              bucket_fill[EPIC_HASH_SIZE];
} ePICPixHash;

typedef struct ePICContext {
    ElsDecCtx        els_ctx;
    int              next_run_pos;
    ElsUnsignedRung  unsigned_rung;
    uint8_t          W_flag_rung;
    uint8_t          N_flag_rung;
    uint8_t          W_ctx_rung[256];
    uint8_t          N_ctx_rung[512];
    uint8_t          nw_pred_rung[256];
    uint8_t          ne_pred_rung[256];
    uint8_t          prev_row_rung[14];
    uint8_t          runlen_zeroes[14];
    uint8_t          runlen_one;
    int              stack_pos;
    uint32_t         stack[EPIC_PIX_STACK_SIZE];
    ePICPixHash      hash;
} ePICContext;

typedef struct JPGContext {
    BlockDSPContext bdsp;
    IDCTDSPContext idsp;
    ScanTable  scantable;

    VLC        dc_vlc[2], ac_vlc[2];
    int        prev_dc[3];
    DECLARE_ALIGNED(32, int16_t, block)[6][64];

    uint8_t    *buf;
} JPGContext;

typedef struct G2MContext {
    ePICContext ec;
    JPGContext jc;

    int        version;

    int        compression;
    int        width, height, bpp;
    int        orig_width, orig_height;
    int        tile_width, tile_height;
    int        tiles_x, tiles_y, tile_x, tile_y;

    int        got_header;

    uint8_t    *framebuf;
    int        framebuf_stride, old_width, old_height;

    uint8_t    *synth_tile, *jpeg_tile, *epic_buf, *epic_buf_base;
    int        tile_stride, epic_buf_stride, old_tile_w, old_tile_h;
    int        swapuv;

    uint8_t    *kempf_buf, *kempf_flags;

    uint8_t    *cursor;
    int        cursor_stride;
    int        cursor_fmt;
    int        cursor_w, cursor_h, cursor_x, cursor_y;
    int        cursor_hot_x, cursor_hot_y;
} G2MContext;

                             const uint8_t *val_table, int nb_codes,
                             int is_ac)
{




                              huff_code, 2, 2, huff_sym, 2, 2, 0);
}

{

                    avpriv_mjpeg_val_dc, 12, 0);
        return ret;
                    avpriv_mjpeg_val_dc, 12, 0);
        return ret;
                    avpriv_mjpeg_val_ac_luminance, 251, 1);
        return ret;
                    avpriv_mjpeg_val_ac_chrominance, 251, 1);
        return ret;

                      ff_zigzag_direct);

}

{

    }


                         uint8_t *dst, int *dst_size)
{



    }
}

                            int plane, int16_t *block)
{

        return AVERROR_INVALIDDATA;

        return AVERROR_INVALIDDATA;

            return AVERROR_INVALIDDATA;

        }
    }
    return 0;
}

{

                           const uint8_t *src, int src_size,
                           uint8_t *dst, int dst_stride,
                           const uint8_t *mask, int mask_stride, int num_mbs,
                           int swapuv)
{

        return ret;
        return ret;


        num_mbs = mb_w * mb_h * 4;

            }
                        return ret;
                }
            }
                    return ret;
            }


                }
            }

                return 0;
        }
    }

    return 0;
}

#define LOAD_NEIGHBOURS(x)      \
    W   = curr_row[(x)   - 1];  \
    N   = above_row[(x)];       \
    WW  = curr_row[(x)   - 2];  \
    NW  = above_row[(x)  - 1];  \
    NE  = above_row[(x)  + 1];  \
    NN  = above2_row[(x)];      \
    NNW = above2_row[(x) - 1];  \
    NWW = above_row[(x)  - 2];  \
    NNE = above2_row[(x) + 1]

#define UPDATE_NEIGHBOURS(x)    \
    NNW = NN;                   \
    NN  = NNE;                  \
    NWW = NW;                   \
    NW  = N;                    \
    N   = NE;                   \
    NE  = above_row[(x)  + 1];  \
    NNE = above2_row[(x) + 1]

#define R_shift 16
#define G_shift  8
#define B_shift  0

/* improved djb2 hash from http://www.cse.yorku.ca/~oz/hash.html */
{


}

{
}

{


    return NULL;
}

{

        return NULL;

            return NULL;
    }

}

{

            return AVERROR(ENOMEM);
    }

        return AVERROR(ENOMEM);


}

                                               uint32_t pix)
{

        return 1;

    return 0;
}

{

            }
        }
    }

{

            break;

}

#define TOSIGNED(val) (((val) >> 1) ^ -((val) & 1))

                                             int N, int W, int NW)
{
}

                                       const uint32_t *curr_row,
                                       const uint32_t *above_row)
{





    } else {
        else



    }

        avpriv_request_sample(NULL, "RGB %d %d %d is out of range\n", R, G, B);
        return 0;
    }

}

                              uint32_t *pPix, uint32_t pix)
{
    }
}

                             const uint32_t *curr_row,
                             const uint32_t *above_row, uint32_t *pPix)
{

        /* the top-left pixel is coded independently with 3 unsigned numbers */
    }

    }

        }
    }

    return 0;
}

static int epic_decode_run_length(ePICContext *dc, int x, int y, int tile_width,
                                  const uint32_t *curr_row,
                                  const uint32_t *above_row,
                                  const uint32_t *above2_row,
                                  uint32_t *pPix, int *pRun)
{
    int idx, got_pixel = 0, WWneW, old_WWneW = 0;
    uint32_t W, WW, N, NN, NW, NE, NWW, NNW, NNE;

    *pRun = 0;

    LOAD_NEIGHBOURS(x);

    if (dc->next_run_pos == x) {
        /* can't reuse W for the new pixel in this case */
        WWneW = 1;
    } else {
        idx = (WW  != W)  << 7 |
              (NW  != W)  << 6 |
              (N   != NE) << 5 |
              (NW  != N)  << 4 |
              (NWW != NW) << 3 |
              (NNE != NE) << 2 |
              (NN  != N)  << 1 |
              (NNW != NW);
        WWneW = ff_els_decode_bit(&dc->els_ctx, &dc->W_ctx_rung[idx]);
        if (WWneW < 0)
            return WWneW;
    }

    if (WWneW)
        dc->stack[dc->stack_pos++ & EPIC_PIX_STACK_MAX] = W;
    else {
        *pPix     = W;
        got_pixel = 1;
    }

    do {
        int NWneW = 1;
        if (got_pixel) // pixel value already known (derived from either W or N)
            NWneW = *pPix != N;
        else { // pixel value is unknown and will be decoded later
            NWneW = *pRun ? NWneW : NW != W;

            /* TODO: RFC this mess! */
            switch (((NW != N) << 2) | (NWneW << 1) | WWneW) {
            case 0:
                break; // do nothing here
            case 3:
            case 5:
            case 6:
            case 7:
                if (!is_pixel_on_stack(dc, N)) {
                    idx = WWneW       << 8 |
                          (*pRun ? old_WWneW : WW != W) << 7 |
                          NWneW       << 6 |
                          (N   != NE) << 5 |
                          (NW  != N)  << 4 |
                          (NWW != NW) << 3 |
                          (NNE != NE) << 2 |
                          (NN  != N)  << 1 |
                          (NNW != NW);
                    if (!ff_els_decode_bit(&dc->els_ctx, &dc->N_ctx_rung[idx])) {
                        NWneW = 0;
                        *pPix = N;
                        got_pixel = 1;
                        break;
                    }
                }
                /* fall through */
            default:
                NWneW = 1;
                old_WWneW = WWneW;
                if (!is_pixel_on_stack(dc, N))
                    dc->stack[dc->stack_pos++ & EPIC_PIX_STACK_MAX] = N;
            }
        }

        (*pRun)++;
        if (x + *pRun >= tile_width - 1)
            break;

        UPDATE_NEIGHBOURS(x + *pRun);

        if (!NWneW && NW == N && N == NE) {
            int pos, run, rle;
            int start_pos = x + *pRun;

            /* scan for a run of pix in the line above */
            uint32_t pix = above_row[start_pos + 1];
            for (pos = start_pos + 2; pos < tile_width; pos++)
                if (!(above_row[pos] == pix))
                    break;
            run = pos - start_pos - 1;
            idx = av_ceil_log2(run);
            if (ff_els_decode_bit(&dc->els_ctx, &dc->prev_row_rung[idx]))
                *pRun += run;
            else {
                int flag;
                /* run-length is coded as plain binary number of idx - 1 bits */
                for (pos = idx - 1, rle = 0, flag = 0; pos >= 0; pos--) {
                    if ((1 << pos) + rle < run &&
                        ff_els_decode_bit(&dc->els_ctx,
                                          flag ? &dc->runlen_one
                                               : &dc->runlen_zeroes[pos])) {
                        flag = 1;
                        rle |= 1 << pos;
                    }
                }
                *pRun += rle;
                break; // return immediately
            }
            if (x + *pRun >= tile_width - 1)
                break;

            LOAD_NEIGHBOURS(x + *pRun);
            WWneW = 0;
            NWneW = 0;
        }

        idx = WWneW       << 7 |
              NWneW       << 6 |
              (N   != NE) << 5 |
              (NW  != N)  << 4 |
              (NWW != NW) << 3 |
              (NNE != NE) << 2 |
              (NN  != N)  << 1 |
              (NNW != NW);
        WWneW = ff_els_decode_bit(&dc->els_ctx, &dc->W_ctx_rung[idx]);
    } while (!WWneW);

    dc->next_run_pos = x + *pRun;
    return got_pixel;
}

                               uint32_t *pPix, uint32_t pix)
{
    }
}

                                   int tile_width, const uint32_t *curr_row,
                                   const uint32_t *above_row, uint32_t *pPix)
{

    /* try to reuse the NW pixel first */
        }
    }

    /* try to reuse the NE[x + run, y] pixel */
        }
    }

    return 0;
}

{

        return 0;

    list = hash_elem->list;
                }
            }
        }
    }

    return 0;
}

                            int tile_width, int stride)
{


                return AVERROR_INVALIDDATA; // bail out in the case of ELS overflow


            } else {

                } else {
                                                       curr_row, above_row,
                                                       above2_row, &pix, &run);
                        return got_pixel;
                }

                                                           tile_width, curr_row,
                                                           above_row, &pix)) {
                            return AVERROR_INVALIDDATA;

                                                              ref_pix,
                                                              pix);
                                return ret;
                        }
                    }
                }
            }
        }
    }

    return 0;
}

                               const uint8_t *src, size_t src_size,
                               AVCodecContext *avctx)
{

        return 0;

    /* get data size of the ELS partition as unsigned variable-length integer */
        av_log(avctx, AV_LOG_ERROR, "ePIC: invalid data size VLI\n");
        return AVERROR_INVALIDDATA;
    }

    }

        av_log(avctx, AV_LOG_ERROR, "ePIC: data too short, needed %"SIZE_SPECIFIER", got %"SIZE_SPECIFIER"\n",
               els_dsize, src_size);
        return AVERROR_INVALIDDATA;
    }


        avpriv_request_sample(avctx, "large tile width");
        return AVERROR_INVALIDDATA;
    }

        /* ELS decoder initializations */

        /* decode transparent pixel value */
            av_log(avctx, AV_LOG_ERROR,
                   "ePIC: couldn't decode transparency pixel!\n");
            ff_els_decoder_uninit(&c->ec.unsigned_rung);
            return AVERROR_INVALIDDATA;
        }

                               c->epic_buf_stride);


            av_log(avctx, AV_LOG_ERROR,
                   "ePIC: tile decoding failed, frame=%d, tile_x=%d, tile_y=%d\n",
                   avctx->frame_number, tile_x, tile_y);
            return AVERROR_INVALIDDATA;
        }


            uint8_t *out = dst;
            }
        }




                        }
                    }
                }
            }

                            c->jpeg_tile, c->tile_stride,

            }
        }
    } else {
        dst = c->framebuf + tile_x * c->tile_width * 3 +
              tile_y * c->tile_height * c->framebuf_stride;
        return jpg_decode_data(&c->jc, tile_width, tile_height, src, src_size,
                               dst, c->framebuf_stride, NULL, 0, 0, c->swapuv);
    }

    return 0;
}

                              uint8_t *dst, int stride,
                              const uint8_t *jpeg_tile, int tile_stride,
                              int width, int height,
                              const uint8_t *pal, int npal, int tidx)
{

        return ret;


            continue;
            else
        }
    }

    return 0;
}

                             const uint8_t *src, int src_size)
{

        return AVERROR_INVALIDDATA;


        return 0;
        return jpg_decode_data(&c->jc, width, height, src, src_end - src,
                               dst, c->framebuf_stride, NULL, 0, 0, 0);
    }

    }
        return AVERROR_INVALIDDATA;
                tidx = i;
                break;
            }
        }
    }

        return 0;

        return AVERROR_INVALIDDATA;

        return AVERROR_INVALIDDATA;

                          NULL, 0, width, height, pal, npal, tidx);
    }

    // blocks are coded LSB and we need normal bitreader for JPEG data
                    return AVERROR_INVALIDDATA;
            }
                return AVERROR_INVALIDDATA;
        }
    }

                    c->jpeg_tile, c->tile_stride,

                      width, height, pal, npal, tidx);

}

{

            return AVERROR(ENOMEM);
    }
        c->old_tile_h < c->tile_height) {
                                    AV_INPUT_BUFFER_PADDING_SIZE);
            return AVERROR(ENOMEM);
                return AVERROR(ENOMEM);
        }
    }

    return 0;
}

                           GetByteContext *gb)
{



        av_log(avctx, AV_LOG_ERROR, "Invalid cursor dimensions %"PRIu32"x%"PRIu32"\n",
               cursor_w, cursor_h);
        return AVERROR_INVALIDDATA;
    }
        av_log(avctx, AV_LOG_WARNING, "Invalid hotspot position %"PRIu32",%"PRIu32"\n",
               cursor_hot_x, cursor_hot_y);
        cursor_hot_x = FFMIN(cursor_hot_x, cursor_w - 1);
        cursor_hot_y = FFMIN(cursor_hot_y, cursor_h - 1);
    }
        av_log(avctx, AV_LOG_ERROR, "Invalid cursor data size %"PRIu32"/%u\n",
               cur_size, bytestream2_get_bytes_left(gb));
        return AVERROR_INVALIDDATA;
    }
        avpriv_report_missing_feature(avctx, "Cursor format %d",
                                      cursor_fmt);
        return AVERROR_PATCHWELCOME;
    }

        av_log(avctx, AV_LOG_ERROR, "Cannot allocate cursor buffer\n");
        return err;
    }


    case 1: // old monochrome
                }
            }
        }

                    }
                }
            }
        }
        break;
        /* skip monochrome version of the cursor and decode RGBA instead */
            }
        }
        break;
    default:
        return AVERROR_PATCHWELCOME;
    }
    return 0;
}

#define APPLY_ALPHA(src, new, alpha) \
    src = (src * (256 - alpha) + new * alpha) >> 8

{

        return;



        h = c->height - y;
        w      +=  x;
        cursor += -x * 4;
    } else {
    }

        return;
    } else {
    }

        }
    }
}

                            int *got_picture_ptr, AVPacket *avpkt)
{

        av_log(avctx, AV_LOG_ERROR,
               "Frame should have at least 12 bytes, got %d instead\n",
               buf_size);
        return AVERROR_INVALIDDATA;
    }


        av_log(avctx, AV_LOG_ERROR, "Wrong magic %08X\n", magic);
        return AVERROR_INVALIDDATA;
    }


            av_log(avctx, AV_LOG_ERROR, "Invalid chunk size %"PRIu32" type %02X\n",
                   chunk_size, chunk_type);
            break;
        }
                av_log(avctx, AV_LOG_ERROR, "Invalid display info size %"PRIu32"\n",
                       chunk_size);
                break;
            }
                av_log(avctx, AV_LOG_ERROR,
                       "Invalid frame dimensions %dx%d\n",
                       c->width, c->height);
                ret = AVERROR_INVALIDDATA;
                goto header_fail;
            }
                ret = ff_set_dimensions(avctx, c->width, c->height);
                if (ret < 0)
                    goto header_fail;
            }
                avpriv_report_missing_feature(avctx, "Compression method %d",
                                              c->compression);
                ret = AVERROR_PATCHWELCOME;
                goto header_fail;
            }
            ) {
                av_log(avctx, AV_LOG_ERROR,
                       "Invalid tile dimensions %dx%d\n",
                       c->tile_width, c->tile_height);
                ret = AVERROR_INVALIDDATA;
                goto header_fail;
            }
                    av_log(avctx, AV_LOG_ERROR,
                           "Display info: missing bitmasks!\n");
                    ret = AVERROR_INVALIDDATA;
                    goto header_fail;
                }
                    avpriv_report_missing_feature(avctx,
                                                  "Bitmasks: R=%"PRIX32", G=%"PRIX32", B=%"PRIX32,
                                                  r_mask, g_mask, b_mask);
                    ret = AVERROR_PATCHWELCOME;
                    goto header_fail;
                }
            } else {
                avpriv_request_sample(avctx, "bpp=%d", c->bpp);
                ret = AVERROR_PATCHWELCOME;
                goto header_fail;
            }
                ret = AVERROR(ENOMEM);
                goto header_fail;
            }
            got_header = 1;
            break;
                av_log(avctx, AV_LOG_WARNING,
                       "No display info - skipping tile\n");
                break;
            }
                av_log(avctx, AV_LOG_ERROR, "Invalid tile data size %"PRIu32"\n",
                       chunk_size);
                break;
            }
                av_log(avctx, AV_LOG_ERROR,
                       "Invalid tile pos %d,%d (in %dx%d grid)\n",
                       c->tile_x, c->tile_y, c->tiles_x, c->tiles_y);
                break;
            }
            }
                av_log(avctx, AV_LOG_ERROR, "Error decoding tile %d,%d\n",
                       c->tile_x, c->tile_y);
            break;
                av_log(avctx, AV_LOG_ERROR, "Invalid cursor pos size %"PRIu32"\n",
                       chunk_size);
                break;
            }
                av_log(avctx, AV_LOG_ERROR, "Invalid cursor data size %"PRIu32"\n",
                       chunk_size);
                break;
            }
        case CHUNK_CC:
        case CHUNK_CD:
            break;
        default:
            av_log(avctx, AV_LOG_WARNING, "Skipping chunk type %02d\n",
                   chunk_type);
        }

        /* navigate to next chunk */
    }

            return ret;



    }

    return buf_size;

header_fail:
    c->width   =
    c->height  = 0;
    c->tiles_x =
    c->tiles_y = 0;
    c->tile_width =
    c->tile_height = 0;
    return ret;
}

{

        av_log(avctx, AV_LOG_ERROR, "Cannot initialise VLCs\n");
        jpg_free_context(&c->jc);
        return AVERROR(ENOMEM);
    }


    // store original sizes and check against those if resize happens

}

{



}

AVCodec ff_g2m_decoder = {
    .name           = "g2m",
    .long_name      = NULL_IF_CONFIG_SMALL("Go2Meeting"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_G2M,
    .priv_data_size = sizeof(G2MContext),
    .init           = g2m_decode_init,
    .close          = g2m_decode_end,
    .decode         = g2m_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
    .caps_internal  = FF_CODEC_CAP_INIT_THREADSAFE,
};
