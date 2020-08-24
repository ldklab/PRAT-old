/*
 * common functions for Indeo Video Interactive codecs (Indeo4 and Indeo5)
 *
 * Copyright (c) 2009 Maxim Poliakovski
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
 * This file contains functions and data shared by both Indeo4 and
 * Indeo5 decoders.
 */

#include <inttypes.h>

#include "libavutil/attributes.h"
#include "libavutil/imgutils.h"

#define BITSTREAM_READER_LE
#include "avcodec.h"
#include "get_bits.h"
#include "internal.h"
#include "mathops.h"
#include "ivi.h"
#include "ivi_dsp.h"

/**
 * These are 2x8 predefined Huffman codebooks for coding macroblock/block
 * signals. They are specified using "huffman descriptors" in order to
 * avoid huge static tables. The decoding tables will be generated at
 * startup from these descriptors.
 */
/** static macroblock huffman tables */
static const IVIHuffDesc ivi_mb_huff_desc[8] = {
    {8,  {0, 4, 5, 4, 4, 4, 6, 6}},
    {12, {0, 2, 2, 3, 3, 3, 3, 5, 3, 2, 2, 2}},
    {12, {0, 2, 3, 4, 3, 3, 3, 3, 4, 3, 2, 2}},
    {12, {0, 3, 4, 4, 3, 3, 3, 3, 3, 2, 2, 2}},
    {13, {0, 4, 4, 3, 3, 3, 3, 2, 3, 3, 2, 1, 1}},
    {9,  {0, 4, 4, 4, 4, 3, 3, 3, 2}},
    {10, {0, 4, 4, 4, 4, 3, 3, 2, 2, 2}},
    {12, {0, 4, 4, 4, 3, 3, 2, 3, 2, 2, 2, 2}}
};

/** static block huffman tables */
static const IVIHuffDesc ivi_blk_huff_desc[8] = {
    {10, {1, 2, 3, 4, 4, 7, 5, 5, 4, 1}},
    {11, {2, 3, 4, 4, 4, 7, 5, 4, 3, 3, 2}},
    {12, {2, 4, 5, 5, 5, 5, 6, 4, 4, 3, 1, 1}},
    {13, {3, 3, 4, 4, 5, 6, 6, 4, 4, 3, 2, 1, 1}},
    {11, {3, 4, 4, 5, 5, 5, 6, 5, 4, 2, 2}},
    {13, {3, 4, 5, 5, 5, 5, 6, 4, 3, 3, 2, 1, 1}},
    {13, {3, 4, 5, 5, 5, 6, 5, 4, 3, 3, 2, 1, 1}},
    {9,  {3, 4, 4, 5, 5, 5, 6, 5, 5}}
};

static VLC ivi_mb_vlc_tabs [8]; ///< static macroblock Huffman tables
static VLC ivi_blk_vlc_tabs[8]; ///< static block Huffman tables

typedef void (*ivi_mc_func) (int16_t *buf, const int16_t *ref_buf,
                             ptrdiff_t pitch, int mc_type);
typedef void (*ivi_mc_avg_func) (int16_t *buf, const int16_t *ref_buf1,
                                 const int16_t *ref_buf2,
                                 ptrdiff_t pitch, int mc_type, int mc_type2);

                  int offs, int mv_x, int mv_y, int mv_x2, int mv_y2,
                  int mc_type, int mc_type2)
{

    }

    } else {
        int ref_offs2 = offs + mv_y2 * band->pitch + mv_x2;
        int ref_size2 = (mc_type2 > 1) * band->pitch + (mc_type2 & 1);
        if (offs < 0 || ref_offs2 < 0 || !band->b_ref_buf)
            return AVERROR_INVALIDDATA;
        if (buf_size - min_size - ref_size2 < ref_offs2)
            return AVERROR_INVALIDDATA;

        if (mc_type == -1)
            mc(band->buf + offs, band->b_ref_buf + ref_offs2,
               band->pitch, mc_type2);
        else
            mc_avg(band->buf + offs, band->ref_buf + ref_offs,
                   band->b_ref_buf + ref_offs2, band->pitch,
                   mc_type, mc_type2);
    }

    return 0;
}

/**
 *  Reverse "nbits" bits of the value "val" and return the result
 *  in the least significant bits.
 */
{

    } else

}

/*
 *  Generate a huffman codebook from the given descriptor
 *  and convert it into the FFmpeg VLC table.
 *
 *  @param[in]   cb    pointer to codebook descriptor
 *  @param[out]  vlc   where to place the generated VLC table
 *  @param[in]   flag  flag: 1 - for static or 0 for dynamic tables
 *  @return     result code: 0 - OK, -1 = error (invalid codebook descriptor)
 */
{



                break;      /* elements, but only 256 codes are allowed! */

                return AVERROR_INVALIDDATA; /* invalid descriptor */

                bits[pos] = 1;

        }//for j
    }//for i

    /* number of codewords = pos */
                    (flag ? INIT_VLC_USE_NEW_STATIC : 0) | INIT_VLC_LE);
}

{

        return;
                                  &ivi_mb_vlc_tabs[i], 1);
                                  &ivi_blk_vlc_tabs[i], 1);
    }
}

/*
 *  Copy huffman codebook descriptors.
 *
 *  @param[out]  dst  ptr to the destination descriptor
 *  @param[in]   src  ptr to the source descriptor
 */
{
}

/*
 *  Compare two huffman codebook descriptors.
 *
 *  @param[in]  desc1  ptr to the 1st descriptor to compare
 *  @param[in]  desc2  ptr to the 2nd descriptor to compare
 *  @return         comparison result: 0 - equal, 1 - not equal
 */
                             const IVIHuffDesc *desc2)
{
}

                         IVIHuffTab *huff_tab, AVCodecContext *avctx)
{

        /* select default table */
    }

        /* custom huffman table (explicitly encoded) */
            av_log(avctx, AV_LOG_ERROR, "Empty custom Huffman table!\n");
            return AVERROR_INVALIDDATA;
        }


        /* Have we got the same custom table? Rebuild if not. */

                    &huff_tab->cust_tab, 0);
                // reset faulty description
                huff_tab->cust_desc.num_rows = 0;
                av_log(avctx, AV_LOG_ERROR,
                       "Error while initializing custom vlc table!\n");
                return result;
            }
        }
    } else {
        /* select one of predefined tables */
    }

    return 0;
}

/*
 *  Free planes, bands and macroblocks buffers.
 *
 *  @param[in]  planes  pointer to the array of the plane descriptors
 */
{


        }
    }

                               int is_indeo4)
{
             height_aligned, buf_size;


        return AVERROR_INVALIDDATA;

    /* fill in the descriptor of the luminance plane */

    /* fill in the descriptors of the chrominance planes */

            return AVERROR(ENOMEM);

        /* select band dimensions: if there is only one band then it
         *  has the full size, if there are several bands each of them
         *  has only half size */

        /* luma   band buffers will be aligned on 16x16 (max macroblock size) */
        /* chroma band buffers will be aligned on   8x8 (max macroblock size) */

                       !band->bufs[2] && !band->bufs[3]);

            /* reset custom vlc */
        }
    }

    return 0;
}

                          int p, int b, int t_height, int t_width)
{

            /* calculate number of macroblocks */
                                              band->mb_size);

                return AVERROR(ENOMEM);

                    av_log(NULL, AV_LOG_DEBUG, "ref_tile mismatch\n");
                    return AVERROR_INVALIDDATA;
                }
            }
        }
    }

    return 0;
}

                              int tile_width, int tile_height)
{


            if (t_width % 2 || t_height % 2) {
                avpriv_request_sample(NULL, "Odd tiles");
                return AVERROR_PATCHWELCOME;
            }
            t_width  >>= 1;
            t_height >>= 1;
        }
            return AVERROR(EINVAL);


                int t;
                for (t = 0; t < band->num_tiles; t++) {
                    av_freep(&band->tiles[t].mbs);
                }
            }


                return AVERROR(ENOMEM);

            /* use the first luma band as reference for motion vectors
             * and quant */
                                 p, b, t_height, t_width);
                return ret;
        }
    }

    return 0;
}

/*
 *  Decode size of the tile data.
 *  The size is stored as a variable-length field having the following format:
 *  if (tile_data_size < 255) than this field is only one byte long
 *  if (tile_data_size >= 255) than this field four is byte long: 0xFF X1 X2 X3
 *  where X1-X3 is size of the tile data
 *
 *  @param[in,out]  gb  the GetBit context
 *  @return     size of the tile data in bytes
 */
{

    }

    /* align the bitstream reader on the byte boundary */

}

static int ivi_dc_transform(const IVIBandDesc *band, int *prev_dc, int buf_offs,
                            int blk_size)
{
    band->dc_transform(prev_dc, band->buf + buf_offs,
                       band->pitch, blk_size);

    return 0;
}

                                   ivi_mc_func mc, ivi_mc_avg_func mc_avg,
                                   int mv_x, int mv_y,
                                   int mv_x2, int mv_y2,
                                   int *prev_dc, int is_intra,
                                   int mc_type, int mc_type2,
                                   uint32_t quant, int offs,
                                   AVCodecContext *avctx)
{

        return AVERROR_INVALIDDATA;

        av_log(avctx, AV_LOG_ERROR, "Scan pattern is not set.\n");
        return AVERROR_INVALIDDATA;
    }

    /* zero transform vector */
    /* zero column flags */
                       IVI_VLC_BITS, 1);
            break; /* End of block */

        /* Escape - run/val explicitly coded using 3 vlc codes */
            /* merge them and convert into signed val */
        } else {
                av_log(avctx, AV_LOG_ERROR, "Invalid sym encountered: %"PRIu32".\n", sym);
                return AVERROR_INVALIDDATA;
            }
        }

        /* de-zigzag and dequantize */
            break;


        /* track columns containing non-zero coeffs */
    }

        return AVERROR_INVALIDDATA; /* corrupt block data */

    /* undoing DC coeff prediction for intra-blocks */
    }

        av_log(NULL, AV_LOG_ERROR, "Too large transform\n");
        return AVERROR_INVALIDDATA;
    }

    /* apply inverse transform */
                        band->pitch, col_flags);

    /* apply motion compensation */
                      mc_type, mc_type2);

    return 0;
}
/*
 *  Decode block data:
 *  extract huffman-coded transform coefficients from the bitstream,
 *  dequantize them, apply inverse transform and motion compensation
 *  in order to reconstruct the picture.
 *
 *  @param[in,out]  gb    the GetBit context
 *  @param[in]      band  pointer to the band descriptor
 *  @param[in]      tile  pointer to the tile descriptor
 *  @return     result code: 0 - OK, -1 = error (corrupted blocks data)
 */
static int ivi_decode_blocks(GetBitContext *gb, const IVIBandDesc *band,
                             IVITile *tile, AVCodecContext *avctx)
{
    int mbn, blk, num_blocks, blk_size, ret, is_intra;
    int mc_type = 0, mc_type2 = -1;
    int mv_x = 0, mv_y = 0, mv_x2 = 0, mv_y2 = 0;
    int32_t prev_dc;
    uint32_t cbp, quant, buf_offs;
    IVIMbInfo *mb;
    ivi_mc_func mc_with_delta_func, mc_no_delta_func;
    ivi_mc_avg_func mc_avg_with_delta_func, mc_avg_no_delta_func;
    const uint8_t *scale_tab;

    /* init intra prediction for the DC coefficient */
    prev_dc    = 0;
    blk_size   = band->blk_size;
    /* number of blocks per mb */
    num_blocks = (band->mb_size != blk_size) ? 4 : 1;
    if (blk_size == 8) {
        mc_with_delta_func     = ff_ivi_mc_8x8_delta;
        mc_no_delta_func       = ff_ivi_mc_8x8_no_delta;
        mc_avg_with_delta_func = ff_ivi_mc_avg_8x8_delta;
        mc_avg_no_delta_func   = ff_ivi_mc_avg_8x8_no_delta;
    } else {
        mc_with_delta_func     = ff_ivi_mc_4x4_delta;
        mc_no_delta_func       = ff_ivi_mc_4x4_no_delta;
        mc_avg_with_delta_func = ff_ivi_mc_avg_4x4_delta;
        mc_avg_no_delta_func   = ff_ivi_mc_avg_4x4_no_delta;
    }

    for (mbn = 0, mb = tile->mbs; mbn < tile->num_MBs; mb++, mbn++) {
        is_intra = !mb->type;
        cbp      = mb->cbp;
        buf_offs = mb->buf_offs;

        quant = band->glob_quant + mb->q_delta;
        if (avctx->codec_id == AV_CODEC_ID_INDEO4)
            quant = av_clip_uintp2(quant, 5);
        else
            quant = av_clip(quant, 0, 23);

        scale_tab = is_intra ? band->intra_scale : band->inter_scale;
        if (scale_tab)
            quant = scale_tab[quant];

        if (!is_intra) {
            mv_x  = mb->mv_x;
            mv_y  = mb->mv_y;
            mv_x2 = mb->b_mv_x;
            mv_y2 = mb->b_mv_y;
            if (band->is_halfpel) {
                mc_type  = ((mv_y  & 1) << 1) | (mv_x  & 1);
                mc_type2 = ((mv_y2 & 1) << 1) | (mv_x2 & 1);
                mv_x  >>= 1;
                mv_y  >>= 1;
                mv_x2 >>= 1;
                mv_y2 >>= 1; /* convert halfpel vectors into fullpel ones */
            }
            if (mb->type == 2)
                mc_type = -1;
            if (mb->type != 2 && mb->type != 3)
                mc_type2 = -1;
            if (mb->type) {
                int dmv_x, dmv_y, cx, cy;

                dmv_x = mb->mv_x >> band->is_halfpel;
                dmv_y = mb->mv_y >> band->is_halfpel;
                cx    = mb->mv_x &  band->is_halfpel;
                cy    = mb->mv_y &  band->is_halfpel;

                if (mb->xpos + dmv_x < 0 ||
                    mb->xpos + dmv_x + band->mb_size + cx > band->pitch ||
                    mb->ypos + dmv_y < 0 ||
                    mb->ypos + dmv_y + band->mb_size + cy > band->aheight) {
                    return AVERROR_INVALIDDATA;
                }
            }
            if (mb->type == 2 || mb->type == 3) {
                int dmv_x, dmv_y, cx, cy;

                dmv_x = mb->b_mv_x >> band->is_halfpel;
                dmv_y = mb->b_mv_y >> band->is_halfpel;
                cx    = mb->b_mv_x &  band->is_halfpel;
                cy    = mb->b_mv_y &  band->is_halfpel;

                if (mb->xpos + dmv_x < 0 ||
                    mb->xpos + dmv_x + band->mb_size + cx > band->pitch ||
                    mb->ypos + dmv_y < 0 ||
                    mb->ypos + dmv_y + band->mb_size + cy > band->aheight) {
                    return AVERROR_INVALIDDATA;
                }
            }
        }

        for (blk = 0; blk < num_blocks; blk++) {
            /* adjust block position in the buffer according to its number */
            if (blk & 1) {
                buf_offs += blk_size;
            } else if (blk == 2) {
                buf_offs -= blk_size;
                buf_offs += blk_size * band->pitch;
            }

            if (cbp & 1) { /* block coded ? */
                ret = ivi_decode_coded_blocks(gb, band, mc_with_delta_func,
                                              mc_avg_with_delta_func,
                                              mv_x, mv_y, mv_x2, mv_y2,
                                              &prev_dc, is_intra,
                                              mc_type, mc_type2, quant,
                                              buf_offs, avctx);
                if (ret < 0)
                    return ret;
            } else {
                int buf_size = band->pitch * band->aheight - buf_offs;
                int min_size = (blk_size - 1) * band->pitch + blk_size;

                if (min_size > buf_size)
                    return AVERROR_INVALIDDATA;
                /* block not coded */
                /* for intra blocks apply the dc slant transform */
                /* for inter - perform the motion compensation without delta */
                if (is_intra) {
                    ret = ivi_dc_transform(band, &prev_dc, buf_offs, blk_size);
                    if (ret < 0)
                        return ret;
                } else {
                    ret = ivi_mc(band, mc_no_delta_func, mc_avg_no_delta_func,
                                 buf_offs, mv_x, mv_y, mv_x2, mv_y2,
                                 mc_type, mc_type2);
                    if (ret < 0)
                        return ret;
                }
            }

            cbp >>= 1;
        }// for blk
    }// for mbn

    align_get_bits(gb);

    return 0;
}

/**
 *  Handle empty tiles by performing data copying and motion
 *  compensation respectively.
 *
 *  @param[in]  avctx     ptr to the AVCodecContext
 *  @param[in]  band      pointer to the band descriptor
 *  @param[in]  tile      pointer to the tile descriptor
 *  @param[in]  mv_scale  scaling factor for motion vectors
 */
static int ivi_process_empty_tile(AVCodecContext *avctx, const IVIBandDesc *band,
                                  IVITile *tile, int32_t mv_scale)
{
    int             x, y, need_mc, mbn, blk, num_blocks, mv_x, mv_y, mc_type;
    int             offs, mb_offset, row_offset, ret;
    IVIMbInfo       *mb, *ref_mb;
    const int16_t   *src;
    int16_t         *dst;
    ivi_mc_func     mc_no_delta_func;
    int             clear_first = !band->qdelta_present && !band->plane && !band->band_num;
    int             mb_size     = band->mb_size;
    int             xend        = tile->xpos + tile->width;
    int             is_halfpel  = band->is_halfpel;
    int             pitch       = band->pitch;

    if (tile->num_MBs != IVI_MBs_PER_TILE(tile->width, tile->height, mb_size)) {
        av_log(avctx, AV_LOG_ERROR, "Allocated tile size %d mismatches "
               "parameters %d in ivi_process_empty_tile()\n",
               tile->num_MBs, IVI_MBs_PER_TILE(tile->width, tile->height, mb_size));
        return AVERROR_INVALIDDATA;
    }

    offs       = tile->ypos * pitch + tile->xpos;
    mb         = tile->mbs;
    ref_mb     = tile->ref_mbs;
    row_offset = mb_size * pitch;
    need_mc    = 0; /* reset the mc tracking flag */

    for (y = tile->ypos; y < (tile->ypos + tile->height); y += mb_size) {
        mb_offset = offs;

        for (x = tile->xpos; x < xend; x += mb_size) {
            mb->xpos     = x;
            mb->ypos     = y;
            mb->buf_offs = mb_offset;

            mb->type = 1; /* set the macroblocks type = INTER */
            mb->cbp  = 0; /* all blocks are empty */

            if (clear_first) {
                mb->q_delta = band->glob_quant;
                mb->mv_x    = 0;
                mb->mv_y    = 0;
            }

            if (ref_mb) {
                if (band->inherit_qdelta)
                    mb->q_delta = ref_mb->q_delta;

                if (band->inherit_mv) {
                    /* motion vector inheritance */
                    if (mv_scale) {
                        mb->mv_x = ivi_scale_mv(ref_mb->mv_x, mv_scale);
                        mb->mv_y = ivi_scale_mv(ref_mb->mv_y, mv_scale);
                    } else {
                        mb->mv_x = ref_mb->mv_x;
                        mb->mv_y = ref_mb->mv_y;
                    }
                    need_mc |= mb->mv_x || mb->mv_y; /* tracking non-zero motion vectors */
                    {
                        int dmv_x, dmv_y, cx, cy;

                        dmv_x = mb->mv_x >> is_halfpel;
                        dmv_y = mb->mv_y >> is_halfpel;
                        cx    = mb->mv_x &  is_halfpel;
                        cy    = mb->mv_y &  is_halfpel;

                        if (   mb->xpos + dmv_x < 0
                            || mb->xpos + dmv_x + mb_size + cx > pitch
                            || mb->ypos + dmv_y < 0
                            || mb->ypos + dmv_y + mb_size + cy > band->aheight) {
                            av_log(avctx, AV_LOG_ERROR, "MV out of bounds\n");
                            return AVERROR_INVALIDDATA;
                        }
                    }
                }
                ref_mb++;
            }

            mb++;
            mb_offset += mb_size;
        } // for x
        offs += row_offset;
    } // for y

    if (band->inherit_mv && need_mc) { /* apply motion compensation if there is at least one non-zero motion vector */
        num_blocks = (mb_size != band->blk_size) ? 4 : 1; /* number of blocks per mb */
        mc_no_delta_func = (band->blk_size == 8) ? ff_ivi_mc_8x8_no_delta
                                                 : ff_ivi_mc_4x4_no_delta;

        for (mbn = 0, mb = tile->mbs; mbn < tile->num_MBs; mb++, mbn++) {
            mv_x = mb->mv_x;
            mv_y = mb->mv_y;
            if (!band->is_halfpel) {
                mc_type = 0; /* we have only fullpel vectors */
            } else {
                mc_type = ((mv_y & 1) << 1) | (mv_x & 1);
                mv_x >>= 1;
                mv_y >>= 1; /* convert halfpel vectors into fullpel ones */
            }

            for (blk = 0; blk < num_blocks; blk++) {
                /* adjust block position in the buffer according with its number */
                offs = mb->buf_offs + band->blk_size * ((blk & 1) + !!(blk & 2) * pitch);
                ret = ivi_mc(band, mc_no_delta_func, 0, offs,
                             mv_x, mv_y, 0, 0, mc_type, -1);
                if (ret < 0)
                    return ret;
            }
        }
    } else {
        /* copy data from the reference tile into the current one */
        src = band->ref_buf + tile->ypos * pitch + tile->xpos;
        dst = band->buf     + tile->ypos * pitch + tile->xpos;
        for (y = 0; y < tile->height; y++) {
            memcpy(dst, src, tile->width*sizeof(band->buf[0]));
            src += pitch;
            dst += pitch;
        }
    }

    return 0;
}


#ifdef DEBUG
static uint16_t ivi_calc_band_checksum(const IVIBandDesc *band)
{
    int         x, y;
    int16_t     *src, checksum;

    src = band->buf;
    checksum = 0;

    for (y = 0; y < band->height; src += band->pitch, y++)
        for (x = 0; x < band->width; x++)
            checksum += src[x];

    return checksum;
}
#endif

/*
 *  Convert and output the current plane.
 *  This conversion is done by adding back the bias value of 128
 *  (subtracted in the encoder) and clipping the result.
 *
 *  @param[in]   plane      pointer to the descriptor of the plane being processed
 *  @param[out]  dst        pointer to the buffer receiving converted pixels
 *  @param[in]   dst_pitch  pitch for moving to the next y line
 */
{

        return;

        }
            for (x = 0; x < w; x++)
                dst[x] = av_clip_uint8(src[x] + 128);
    }
}

{
        return NULL;
}

/**
 *  Decode an Indeo 4 or 5 band.
 *
 *  @param[in,out]  ctx    ptr to the decoder context
 *  @param[in,out]  band   ptr to the band descriptor
 *  @param[in]      avctx  ptr to the AVCodecContext
 *  @return         result code: 0 = OK, -1 = error
 */
                       IVIBandDesc *band, AVCodecContext *avctx)
{

        av_log(avctx, AV_LOG_ERROR, "Band buffer points to no data!\n");
        return AVERROR_INVALIDDATA;
    }
        band->ref_buf   = prepare_buf(ctx, band, ctx->b_ref_buf);
        band->b_ref_buf = prepare_buf(ctx, band, ctx->ref_buf);
        if (!band->b_ref_buf)
            return AVERROR(ENOMEM);
    } else {
    }
        return AVERROR(ENOMEM);

        av_log(avctx, AV_LOG_ERROR, "Error while decoding band header: %d\n",
               result);
        return result;
    }

        av_log(avctx, AV_LOG_ERROR, "Empty band encountered!\n");
        return AVERROR_INVALIDDATA;
    }


    /* apply corrections to the selected rvmap table if present */
            band->rv_map->eob_sym ^= idx1 ^ idx2;
            band->rv_map->esc_sym ^= idx1 ^ idx2;
    }



            av_log(avctx, AV_LOG_ERROR, "MB sizes mismatch: %d vs. %d\n",
                   band->mb_size, tile->mb_size);
            return AVERROR_INVALIDDATA;
        }
            result = ivi_process_empty_tile(avctx, band, tile,
                                      (ctx->planes[0].bands[0].mb_size >> 3) - (band->mb_size >> 3));
            if (result < 0)
                break;
            ff_dlog(avctx, "Empty tile encountered!\n");
        } else {
                av_log(avctx, AV_LOG_ERROR, "Tile data size is zero!\n");
                result = AVERROR_INVALIDDATA;
                break;
            }

                break;

                       "Corrupted tile data encountered!\n");
            }

                av_log(avctx, AV_LOG_ERROR,
                       "Tile data_size mismatch!\n");
                result = AVERROR_INVALIDDATA;
                break;
            }

        }
    }

    /* restore the selected rvmap table by applying its corrections in
     * reverse order */
            band->rv_map->eob_sym ^= idx1 ^ idx2;
            band->rv_map->esc_sym ^= idx1 ^ idx2;
    }

#ifdef DEBUG
    if (band->checksum_present) {
        uint16_t chksum = ivi_calc_band_checksum(band);
        if (chksum != band->checksum) {
            av_log(avctx, AV_LOG_ERROR,
                   "Band checksum mismatch! Plane %d, band %d, "
                   "received: %"PRIx32", calculated: %"PRIx16"\n",
                   band->plane, band->band_num, band->checksum, chksum);
        }
    }
#endif


}

                        AVPacket *avpkt)
{

        return result;

        av_log(avctx, AV_LOG_ERROR,
               "Error while decoding picture header: %d\n", result);
        return result;
    }
        return AVERROR_INVALIDDATA;

        if (ctx->got_p_frame) {
            av_frame_move_ref(data, ctx->p_frame);
            *got_frame = 1;
            ctx->got_p_frame = 0;
        } else {
            *got_frame = 0;
        }
        return buf_size;
    }

        avpriv_report_missing_feature(avctx, "Password-protected clip");
        return AVERROR_PATCHWELCOME;
    }

        av_log(avctx, AV_LOG_ERROR, "Color planes not initialized yet\n");
        return AVERROR_INVALIDDATA;
    }


                           "Error while decoding band: %d, plane: %d\n", b, p);
                }
            }
        }
    } else {
            return AVERROR_INVALIDDATA;

                return AVERROR_INVALIDDATA;
        }
    }
        return -1;

        return buf_size;

        return result;

        return result;

        if (ctx->is_indeo4)
            ff_ivi_recompose_haar(&ctx->planes[0], frame->data[0], frame->linesize[0]);
        else
            ff_ivi_recompose53   (&ctx->planes[0], frame->data[0], frame->linesize[0]);
    } else {
    }



    /* If the bidirectional mode is enabled, next I and the following P
     * frame will be sent together. Unfortunately the approach below seems
     * to be the only way to handle the B-frames mode.
     * That's exactly the same Intel decoders do.
     */
        int left;

            // skip version string
                return AVERROR_INVALIDDATA;
        }
            show_bits(&ctx->gb, 21) == 0xBFFF8) { // syncheader + inter type
            AVPacket pkt;
            pkt.data = avpkt->data + (get_bits_count(&ctx->gb) >> 3);
            pkt.size = get_bits_left(&ctx->gb) >> 3;
            ctx->got_p_frame = 0;
            av_frame_unref(ctx->p_frame);
            ff_ivi_decode_frame(avctx, ctx->p_frame, &ctx->got_p_frame, &pkt);
        }
    }

            av_log(avctx, AV_LOG_DEBUG, "This video uses scalability mode\n");
            av_log(avctx, AV_LOG_DEBUG, "This video uses local decoding\n");
            av_log(avctx, AV_LOG_DEBUG, "This video contains B-frames\n");
            av_log(avctx, AV_LOG_DEBUG, "Transparency mode is enabled\n");
            av_log(avctx, AV_LOG_DEBUG, "This video uses Haar transform\n");
            av_log(avctx, AV_LOG_DEBUG, "This video uses fullpel motion vectors\n");
    }

    return buf_size;
}

/**
 *  Close Indeo5 decoder and clean up its context.
 */
{



        ff_free_vlc(&ctx->blk_vlc.cust_tab);


}


/**
 *  Scan patterns shared between indeo4 and indeo5
 */
const uint8_t ff_ivi_vertical_scan_8x8[64] = {
    0,  8, 16, 24, 32, 40, 48, 56,
    1,  9, 17, 25, 33, 41, 49, 57,
    2, 10, 18, 26, 34, 42, 50, 58,
    3, 11, 19, 27, 35, 43, 51, 59,
    4, 12, 20, 28, 36, 44, 52, 60,
    5, 13, 21, 29, 37, 45, 53, 61,
    6, 14, 22, 30, 38, 46, 54, 62,
    7, 15, 23, 31, 39, 47, 55, 63
};

const uint8_t ff_ivi_horizontal_scan_8x8[64] = {
     0,  1,  2,  3,  4,  5,  6,  7,
     8,  9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23,
    24, 25, 26, 27, 28, 29, 30, 31,
    32, 33, 34, 35, 36, 37, 38, 39,
    40, 41, 42, 43, 44, 45, 46, 47,
    48, 49, 50, 51, 52, 53, 54, 55,
    56, 57, 58, 59, 60, 61, 62, 63
};

const uint8_t ff_ivi_direct_scan_4x4[16] = {
    0, 1, 4, 8, 5, 2, 3, 6, 9, 12, 13, 10, 7, 11, 14, 15
};


/**
 *  Run-value (RLE) tables.
 */
const RVMapDesc ff_ivi_rvmap_tabs[9] = {
{   /* MapTab0 */
    5, /* eob_sym */
    2, /* esc_sym */
    /* run table */
    {1,  1,  0,  1,  1,  0,  1,  1,  2,  2,  1,  1,  1,  1,  3,  3,
     1,  1,  2,  2,  1,  1,  4,  4,  1,  1,  1,  1,  2,  2,  5,  5,
     1,  1,  3,  3,  1,  1,  6,  6,  1,  2,  1,  2,  7,  7,  1,  1,
     8,  8,  1,  1,  4,  2,  1,  4,  2,  1,  3,  3,  1,  1,  1,  9,
     9,  1,  2,  1,  2,  1,  5,  5,  1,  1, 10, 10,  1,  1,  3,  3,
     2,  2,  1,  1, 11, 11,  6,  4,  4,  1,  6,  1,  2,  1,  2, 12,
     8,  1, 12,  7,  8,  7,  1, 16,  1, 16,  1,  3,  3, 13,  1, 13,
     2,  2,  1, 15,  1,  5, 14, 15,  1,  5, 14,  1, 17,  8, 17,  8,
     1,  4,  4,  2,  2,  1, 25, 25, 24, 24,  1,  3,  1,  3,  1,  8,
     6,  7,  6,  1, 18,  8, 18,  1,  7, 23,  2,  2, 23,  1,  1, 21,
    22,  9,  9, 22, 19,  1, 21,  5, 19,  5,  1, 33, 20, 33, 20,  8,
     4,  4,  1, 32,  2,  2,  8,  3, 32, 26,  3,  1,  7,  7, 26,  6,
     1,  6,  1,  1, 16,  1, 10,  1, 10,  2, 16, 29, 28,  2, 29, 28,
     1, 27,  5,  8,  5, 27,  1,  8,  3,  7,  3, 31, 41, 31,  1, 41,
     6,  1,  6,  7,  4,  4,  1,  1,  2,  1,  2, 11, 34, 30, 11,  1,
    30, 15, 15, 34, 36, 40, 36, 40, 35, 35, 37, 37, 39, 39, 38, 38},

    /* value table */
    { 1,  -1,   0,   2,  -2,   0,   3,  -3,   1,  -1,   4,  -4,   5,  -5,   1,  -1,
      6,  -6,   2,  -2,   7,  -7,   1,  -1,   8,  -8,   9,  -9,   3,  -3,   1,  -1,
     10, -10,   2,  -2,  11, -11,   1,  -1,  12,   4, -12,  -4,   1,  -1,  13, -13,
      1,  -1,  14, -14,   2,   5,  15,  -2,  -5, -15,  -3,   3,  16, -16,  17,   1,
     -1, -17,   6,  18,  -6, -18,   2,  -2,  19, -19,   1,  -1,  20, -20,   4,  -4,
      7,  -7,  21, -21,   1,  -1,   2,   3,  -3,  22,  -2, -22,   8,  23,  -8,   1,
      2, -23,  -1,   2,  -2,  -2,  24,   1, -24,  -1,  25,   5,  -5,   1, -25,  -1,
      9,  -9,  26,   1, -26,   3,   1,  -1,  27,  -3,  -1, -27,   1,   3,  -1,  -3,
     28,  -4,   4,  10, -10, -28,   1,  -1,   1,  -1,  29,   6, -29,  -6,  30,  -4,
      3,   3,  -3, -30,   1,   4,  -1,  31,  -3,   1,  11, -11,  -1, -31,  32,  -1,
     -1,   2,  -2,   1,   1, -32,   1,   4,  -1,  -4,  33,  -1,   1,   1,  -1,   5,
      5,  -5, -33,  -1, -12,  12,  -5,  -7,   1,   1,   7,  34,   4,  -4,  -1,   4,
    -34,  -4,  35,  36,  -2, -35,  -2, -36,   2,  13,   2,  -1,   1, -13,   1,  -1,
     37,   1,  -5,   6,   5,  -1,  38,  -6,  -8,   5,   8,  -1,   1,   1, -37,  -1,
      5,  39,  -5,  -5,   6,  -6, -38, -39, -14,  40,  14,   2,   1,   1,  -2, -40,
     -1,  -2,   2,  -1,  -1,  -1,   1,   1,   1,  -1,   1,  -1,   1,  -1,   1,  -1}
},{
    /* MapTab1 */
    0,  /* eob_sym */
    38, /* esc_sym */
    /* run table */
    {0,  1,  1,  2,  2,  3,  3,  4,  4,  5,  5,  6,  8,  6,  8,  7,
     7,  9,  9, 10, 10, 11, 11,  1, 12,  1, 12, 13, 13, 16, 14, 16,
    14, 15, 15, 17, 17, 18,  0, 18, 19, 20, 21, 19, 22, 21, 20, 22,
    25, 24,  2, 25, 24, 23, 23,  2, 26, 28, 26, 28, 29, 27, 29, 27,
    33, 33,  1, 32,  1,  3, 32, 30, 36,  3, 36, 30, 31, 31, 35, 34,
    37, 41, 34, 35, 37,  4, 41,  4, 49,  8,  8, 49, 40, 38,  5, 38,
    40, 39,  5, 39, 42, 43, 42,  7, 57,  6, 43, 44,  6, 50,  7, 44,
    57, 48, 50, 48, 45, 45, 46, 47, 51, 46, 47, 58,  1, 51, 58,  1,
    52, 59, 53,  9, 52, 55, 55, 59, 53, 56, 54, 56, 54,  9, 64, 64,
    60, 63, 60, 63, 61, 62, 61, 62,  2, 10,  2, 10, 11,  1, 11, 13,
    12,  1, 12, 13, 16, 16,  8,  8, 14,  3,  3, 15, 14, 15,  4,  4,
     1, 17, 17,  5,  1,  7,  7,  5,  6,  1,  2,  2,  6, 22,  1, 25,
    21, 22,  8, 24,  1, 21, 25, 24,  8, 18, 18, 23,  9, 20, 23, 33,
    29, 33, 20,  1, 19,  1, 29, 36,  9, 36, 19, 41, 28, 57, 32,  3,
    28,  3,  1, 27, 49, 49,  1, 32, 26, 26,  2,  4,  4,  7, 57, 41,
     2,  7, 10,  5, 37, 16, 10, 27,  8,  8, 13, 16, 37, 13,  1,  5},

    /* value table */
    {0,   1,  -1,   1,  -1,   1,  -1,   1,  -1,   1,  -1,   1,   1,  -1,  -1,   1,
    -1,   1,  -1,   1,  -1,   1,  -1,   2,   1,  -2,  -1,   1,  -1,   1,   1,  -1,
    -1,   1,  -1,   1,  -1,   1,   0,  -1,   1,   1,   1,  -1,   1,  -1,  -1,  -1,
     1,   1,   2,  -1,  -1,   1,  -1,  -2,   1,   1,  -1,  -1,   1,   1,  -1,  -1,
     1,  -1,   3,   1,  -3,   2,  -1,   1,   1,  -2,  -1,  -1,  -1,   1,   1,   1,
     1,   1,  -1,  -1,  -1,   2,  -1,  -2,   1,   2,  -2,  -1,   1,   1,   2,  -1,
    -1,   1,  -2,  -1,   1,   1,  -1,   2,   1,   2,  -1,   1,  -2,  -1,  -2,  -1,
    -1,   1,   1,  -1,   1,  -1,   1,   1,   1,  -1,  -1,   1,   4,  -1,  -1,  -4,
     1,   1,   1,   2,  -1,  -1,   1,  -1,  -1,   1,  -1,  -1,   1,  -2,   1,  -1,
     1,   1,  -1,  -1,   1,   1,  -1,  -1,   3,   2,  -3,  -2,   2,   5,  -2,   2,
     2,  -5,  -2,  -2,  -2,   2,  -3,   3,   2,   3,  -3,   2,  -2,  -2,   3,  -3,
     6,   2,  -2,   3,  -6,   3,  -3,  -3,   3,   7,  -4,   4,  -3,   2,  -7,   2,
     2,  -2,  -4,   2,   8,  -2,  -2,  -2,   4,   2,  -2,   2,   3,   2,  -2,  -2,
     2,   2,  -2,  -8,  -2,   9,  -2,   2,  -3,  -2,   2,  -2,   2,   2,   2,   4,
    -2,  -4,  10,   2,   2,  -2,  -9,  -2,   2,  -2,   5,   4,  -4,   4,  -2,   2,
    -5,  -4,  -3,   4,   2,  -3,   3,  -2,  -5,   5,   3,   3,  -2,  -3, -10,  -4}
},{
    /* MapTab2 */
    2,  /* eob_sym */
    11, /* esc_sym */
    /* run table */
    {1,  1,  0,  2,  2,  1,  1,  3,  3,  4,  4,  0,  1,  1,  5,  5,
     2,  2,  6,  6,  7,  7,  1,  8,  1,  8,  3,  3,  9,  9,  1,  2,
     2,  1,  4, 10,  4, 10, 11, 11,  1,  5, 12, 12,  1,  5, 13, 13,
     3,  3,  6,  6,  2,  2, 14, 14, 16, 16, 15,  7, 15,  8,  8,  7,
     1,  1, 17, 17,  4,  4,  1,  1, 18, 18,  2,  2,  5,  5, 25,  3,
     9,  3, 25,  9, 19, 24, 19, 24,  1, 21, 20,  1, 21, 22, 20, 22,
    23, 23,  8,  6, 33,  6,  8, 33,  7,  7, 26, 26,  1, 32,  1, 32,
    28,  4, 28, 10, 29, 27, 27, 10, 41,  4, 29,  2,  2, 41, 36, 31,
    49, 31, 34, 30, 34, 36, 30, 35,  1, 49, 11,  5, 35, 11,  1,  3,
     3,  5, 37, 37,  8, 40,  8, 40, 12, 12, 42, 42,  1, 38, 16, 57,
     1,  6, 16, 39, 38,  6,  7,  7, 13, 13, 39, 43,  2, 43, 57,  2,
    50,  9, 44,  9, 50,  4, 15, 48, 44,  4,  1, 15, 48, 14, 14,  1,
    45, 45,  8,  3,  5,  8, 51, 47,  3, 46, 46, 47,  5, 51,  1, 17,
    17, 58,  1, 58,  2, 52, 52,  2, 53,  7, 59,  6,  6, 56, 53, 55,
     7, 55,  1, 54, 59, 56, 54, 10,  1, 10,  4, 60,  1, 60,  8,  4,
     8, 64, 64, 61,  1, 63,  3, 63, 62, 61,  5, 11,  5,  3, 11, 62},

    /* value table */
    { 1,  -1,   0,   1,  -1,   2,  -2,   1,  -1,   1,  -1,   0,   3,  -3,   1,  -1,
      2,  -2,   1,  -1,   1,  -1,   4,   1,  -4,  -1,   2,  -2,   1,  -1,   5,   3,
     -3,  -5,   2,   1,  -2,  -1,   1,  -1,   6,   2,   1,  -1,  -6,  -2,   1,  -1,
      3,  -3,   2,  -2,   4,  -4,   1,  -1,   1,  -1,   1,   2,  -1,   2,  -2,  -2,
      7,  -7,   1,  -1,   3,  -3,   8,  -8,   1,  -1,   5,  -5,   3,  -3,   1,   4,
      2,  -4,  -1,  -2,   1,   1,  -1,  -1,   9,   1,   1,  -9,  -1,   1,  -1,  -1,
      1,  -1,   3,  -3,   1,   3,  -3,  -1,   3,  -3,   1,  -1,  10,   1, -10,  -1,
      1,   4,  -1,   2,   1,  -1,   1,  -2,   1,  -4,  -1,   6,  -6,  -1,   1,   1,
      1,  -1,   1,   1,  -1,  -1,  -1,   1,  11,  -1,  -2,   4,  -1,   2, -11,   5,
     -5,  -4,  -1,   1,   4,   1,  -4,  -1,  -2,   2,   1,  -1,  12,   1,  -2,   1,
    -12,   4,   2,   1,  -1,  -4,   4,  -4,   2,  -2,  -1,   1,   7,  -1,  -1,  -7,
     -1,  -3,   1,   3,   1,   5,   2,   1,  -1,  -5,  13,  -2,  -1,   2,  -2, -13,
      1,  -1,   5,   6,   5,  -5,   1,   1,  -6,   1,  -1,  -1,  -5,  -1,  14,   2,
     -2,   1, -14,  -1,   8,   1,  -1,  -8,   1,   5,   1,   5,  -5,   1,  -1,   1,
     -5,  -1,  15,   1,  -1,  -1,  -1,   3, -15,  -3,   6,   1,  16,  -1,   6,  -6,
     -6,   1,  -1,   1, -16,   1,   7,  -1,   1,  -1,  -6,  -3,   6,  -7,   3,  -1}
},{
    /* MapTab3 */
    0,  /* eob_sym */
    35, /* esc_sym */
    /* run table */
    {0,  1,  1,  2,  2,  3,  3,  4,  4,  1,  1,  5,  5,  6,  6,  7,
     7,  8,  8,  9,  9,  2,  2, 10, 10,  1,  1, 11, 11, 12, 12,  3,
     3, 13, 13,  0, 14, 14, 16, 15, 16, 15,  4,  4, 17,  1, 17,  1,
     5,  5, 18, 18,  2,  2,  6,  6,  8, 19,  7,  8,  7, 19, 20, 20,
    21, 21, 22, 24, 22, 24, 23, 23,  1,  1, 25, 25,  3,  3, 26, 26,
     9,  9, 27, 27, 28, 28, 33, 29,  4, 33, 29,  1,  4,  1, 32, 32,
     2,  2, 31, 10, 30, 10, 30, 31, 34, 34,  5,  5, 36, 36, 35, 41,
    35, 11, 41, 11, 37,  1,  8,  8, 37,  6,  1,  6, 40,  7,  7, 40,
    12, 38, 12, 39, 39, 38, 49, 13, 49, 13,  3, 42,  3, 42, 16, 16,
    43, 43, 14, 14,  1,  1, 44, 15, 44, 15,  2,  2, 57, 48, 50, 48,
    57, 50,  4, 45, 45,  4, 46, 47, 47, 46,  1, 51,  1, 17, 17, 51,
     8,  9,  9,  5, 58,  8, 58,  5, 52, 52, 55, 56, 53, 56, 55, 59,
    59, 53, 54,  1,  6, 54,  7,  7,  6,  1,  2,  3,  2,  3, 64, 60,
    60, 10, 10, 64, 61, 62, 61, 63,  1, 63, 62,  1, 18, 24, 18,  4,
    25,  4,  8, 21, 21,  1, 24, 22, 25, 22,  8, 11, 19, 11, 23,  1,
    20, 23, 19, 20,  5, 12,  5,  1, 16,  2, 12, 13,  2, 13,  1, 16},

    /* value table */
    { 0,   1,  -1,   1,  -1,   1,  -1,   1,  -1,   2,  -2,   1,  -1,   1,  -1,   1,
     -1,   1,  -1,   1,  -1,   2,  -2,   1,  -1,   3,  -3,   1,  -1,   1,  -1,   2,
     -2,   1,  -1,   0,   1,  -1,   1,   1,  -1,  -1,   2,  -2,   1,   4,  -1,  -4,
      2,  -2,   1,  -1,  -3,   3,   2,  -2,   2,   1,   2,  -2,  -2,  -1,   1,  -1,
      1,  -1,   1,   1,  -1,  -1,   1,  -1,   5,  -5,   1,  -1,   3,  -3,   1,  -1,
      2,  -2,   1,  -1,   1,  -1,   1,   1,   3,  -1,  -1,   6,  -3,  -6,  -1,   1,
      4,  -4,   1,   2,   1,  -2,  -1,  -1,   1,  -1,   3,  -3,   1,  -1,   1,   1,
     -1,   2,  -1,  -2,   1,   7,  -3,   3,  -1,   3,  -7,  -3,   1,  -3,   3,  -1,
      2,   1,  -2,   1,  -1,  -1,   1,   2,  -1,  -2,  -4,  -1,   4,   1,   2,  -2,
      1,  -1,  -2,   2,   8,  -8,  -1,   2,   1,  -2,  -5,   5,   1,  -1,  -1,   1,
     -1,   1,   4,  -1,   1,  -4,  -1,  -1,   1,   1,   9,   1,  -9,   2,  -2,  -1,
     -4,   3,  -3,  -4,  -1,   4,   1,   4,   1,  -1,   1,  -1,   1,   1,  -1,   1,
     -1,  -1,  -1,  10,   4,   1,   4,  -4,  -4, -10,   6,   5,  -6,  -5,   1,  -1,
      1,   3,  -3,  -1,   1,  -1,  -1,  -1,  11,   1,   1, -11,  -2,  -2,   2,   5,
     -2,  -5,  -5,   2,  -2,  12,   2,  -2,   2,   2,   5,  -3,  -2,   3,  -2, -12,
     -2,   2,   2,   2,  -5,   3,   5,  13,  -3,   7,  -3,  -3,  -7,   3, -13,   3}
},{
    /* MapTab4 */
    0,  /* eob_sym */
    34, /* esc_sym */
    /* run table */
    {0,  1,  1,  1,  2,  2,  1,  3,  3,  1,  1,  1,  4,  4,  1,  5,
     2,  1,  5,  2,  1,  1,  6,  6,  1,  1,  1,  1,  1,  7,  3,  1,
     2,  3,  0,  1,  2,  7,  1,  1,  1,  8,  1,  1,  8,  1,  1,  1,
     9,  1,  9,  1,  2,  1,  1,  2,  1,  1, 10,  4,  1, 10,  1,  4,
     1,  1,  1,  1,  1,  3,  1,  1,  1,  3,  2,  1,  5,  1,  1,  1,
     2,  5,  1, 11,  1, 11,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
     2,  1,  6,  1,  6,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1, 12,
     3,  1, 12,  1,  1,  1,  2,  1,  1,  3,  1,  1,  1,  1,  1,  1,
     4,  1,  1,  1,  2,  1,  1,  4,  1,  1,  1,  1,  1,  1,  2,  1,
     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  3,  1,  2,  1,  1,  5,
     1,  1,  1,  1,  1,  7,  1,  7,  1,  1,  2,  3,  1,  1,  1,  1,
     5,  1,  1,  1,  1,  1,  1,  2, 13,  1,  1,  1,  1,  1,  1,  1,
     1,  1,  1,  1,  1,  1,  1,  1, 13,  2,  1,  1,  4,  1,  1,  1,
     3,  1,  6,  1,  1,  1, 14,  1,  1,  1,  1,  1, 14,  6,  1,  1,
     1,  1, 15,  2,  4,  1,  2,  3, 15,  1,  1,  1,  8,  1,  1,  8,
     1,  1,  1,  1,  1,  1,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1},

    /* value table */
    { 0,   1,  -1,   2,   1,  -1,  -2,   1,  -1,   3,  -3,   4,   1,  -1,  -4,   1,
      2,   5,  -1,  -2,  -5,   6,   1,  -1,  -6,   7,  -7,   8,  -8,   1,   2,   9,
      3,  -2,   0,  -9,  -3,  -1,  10, -10,  11,   1, -11,  12,  -1, -12,  13, -13,
      1,  14,  -1, -14,   4,  15, -15,  -4,  16, -16,   1,   2,  17,  -1, -17,  -2,
     18, -18,  19, -19,  20,   3, -20,  21, -21,  -3,   5,  22,   2, -22, -23,  23,
     -5,  -2,  24,   1, -24,  -1,  25, -25,  26, -26, -27,  27,  28,  29, -28, -29,
      6,  30,   2, -31,  -2, -30,  31,  -6, -32,  32,  33, -33,  34, -35, -34,   1,
      4, -36,  -1,  35,  37,  36,   7, -37,  38,  -4, -38,  39,  41,  40, -40, -39,
      3,  42, -43, -41,  -7, -42,  43,  -3,  44, -44,  45, -45,  46,  47,   8, -47,
    -48, -46,  50, -50,  48,  49,  51, -49,  52, -52,   5, -51,  -8, -53,  53,   3,
    -56,  56,  55,  54, -54,   2,  60,  -2, -55,  58,   9,  -5,  59,  57, -57, -63,
     -3, -58, -60, -61,  61, -59, -62,  -9,   1,  64,  62,  69, -64,  63,  65, -67,
    -68,  66, -65,  68, -66, -69,  67, -70,  -1,  10,  71, -71,   4,  73,  72,  70,
      6, -76,  -3,  74, -78, -74,   1,  78,  80, -72, -75,  76,  -1,   3, -73,  79,
     75,  77,   1,  11,  -4, -79, -10,  -6,  -1, -77, -83, -80,   2,  81, -84,  -2,
     83, -81,  82, -82,  84, -87, -86,  85, -11, -85,  86, -89,  87, -88,  88,  89}
},{
    /* MapTab5 */
    2,  /* eob_sym */
    33, /* esc_sym */
    /* run table */
    {1,  1,  0,  2,  1,  2,  1,  3,  3,  1,  1,  4,  4,  2,  2,  1,
     1,  5,  5,  6,  1,  6,  1,  7,  7,  3,  3,  2,  8,  2,  8,  1,
     1,  0,  9,  9,  1,  1, 10,  4, 10,  4, 11, 11,  2,  1,  2,  1,
    12, 12,  3,  3,  1,  1, 13,  5,  5, 13, 14,  1,  1, 14,  2,  2,
     6,  6, 15,  1,  1, 15, 16,  4,  7, 16,  4,  7,  1,  1,  3,  3,
     8,  8,  2,  2,  1,  1, 17, 17,  1,  1, 18, 18,  5,  5,  2,  2,
     1,  1,  9, 19,  9, 19, 20,  3,  3, 20,  1, 10, 21,  1, 10,  4,
     4, 21, 22,  6,  6, 22,  1,  1, 23, 24,  2,  2, 23, 24, 11,  1,
     1, 11,  7, 25,  7,  1,  1, 25,  8,  8,  3, 26,  3,  1, 12,  2,
     2, 26,  1, 12,  5,  5, 27,  4,  1,  4,  1, 27, 28,  1, 28, 13,
     1, 13,  2, 29,  2,  1, 32,  6,  1, 30, 14, 29, 14,  6,  3, 31,
     3,  1, 30,  1, 32, 31, 33,  9, 33,  1,  1,  7,  9,  7,  2,  2,
     1,  1,  4, 36, 34,  4,  5, 10, 10,  5, 34,  1,  1, 35,  8,  8,
    36,  3, 35,  1, 15,  3,  2,  1, 16, 15, 16,  2, 37,  1, 37,  1,
     1,  1,  6,  6, 38,  1, 38, 11,  1, 39, 39, 40, 11,  2, 41,  4,
    40,  1,  2,  4,  1,  1,  1, 41,  3,  1,  3,  1,  5,  7,  5,  7},

    /* value table */
    { 1,  -1,   0,   1,   2,  -1,  -2,   1,  -1,   3,  -3,   1,  -1,   2,  -2,   4,
     -4,   1,  -1,   1,   5,  -1,  -5,   1,  -1,   2,  -2,   3,   1,  -3,  -1,   6,
     -6,   0,   1,  -1,   7,  -7,   1,   2,  -1,  -2,   1,  -1,   4,   8,  -4,  -8,
      1,  -1,   3,  -3,   9,  -9,   1,   2,  -2,  -1,   1,  10, -10,  -1,   5,  -5,
      2,  -2,   1,  11, -11,  -1,   1,   3,   2,  -1,  -3,  -2,  12, -12,   4,  -4,
      2,  -2,  -6,   6,  13, -13,   1,  -1,  14, -14,   1,  -1,   3,  -3,   7,  -7,
     15, -15,   2,   1,  -2,  -1,   1,   5,  -5,  -1, -16,   2,   1,  16,  -2,   4,
     -4,  -1,   1,   3,  -3,  -1,  17, -17,   1,   1,  -8,   8,  -1,  -1,   2,  18,
    -18,  -2,   3,   1,  -3,  19, -19,  -1,   3,  -3,   6,   1,  -6,  20,   2,   9,
     -9,  -1, -20,  -2,   4,  -4,   1,  -5,  21,   5, -21,  -1,   1, -22,  -1,   2,
     22,  -2,  10,   1, -10,  23,   1,   4, -23,   1,   2,  -1,  -2,  -4,  -7,   1,
      7, -24,  -1,  24,  -1,  -1,   1,   3,  -1, -25,  25,   4,  -3,  -4,  11, -11,
     26, -26,   6,   1,   1,  -6,  -5,  -3,   3,   5,  -1, -27,  27,   1,   4,  -4,
     -1,  -8,  -1,  28,   2,   8, -12, -28,  -2,  -2,   2,  12,  -1,  29,   1, -29,
     30, -30,   5,  -5,   1, -31,  -1,   3,  31,  -1,   1,   1,  -3, -13,   1,  -7,
     -1, -32,  13,   7,  32,  33, -33,  -1,  -9, -34,   9,  34,  -6,   5,   6,  -5}
},{
    /* MapTab6 */
    2,  /* eob_sym */
    13, /* esc_sym */
    /* run table */
    {1,  1,  0,  1,  1,  2,  2,  1,  1,  3,  3,  1,  1,  0,  2,  2,
     4,  1,  4,  1,  1,  1,  5,  5,  1,  1,  6,  6,  2,  2,  1,  1,
     3,  3,  7,  7,  1,  1,  8,  8,  1,  1,  2,  2,  1,  9,  1,  9,
     4,  4, 10,  1,  1, 10,  1,  1, 11, 11,  3,  3,  1,  2,  1,  2,
     1,  1, 12, 12,  5,  5,  1,  1, 13,  1,  1, 13,  2,  2,  1,  1,
     6,  6,  1,  1,  4, 14,  4, 14,  3,  1,  3,  1,  1,  1, 15,  7,
    15,  2,  2,  7,  1,  1,  1,  8,  1,  8, 16, 16,  1,  1,  1,  1,
     2,  1,  1,  2,  1,  1,  3,  5,  5,  3,  4,  1,  1,  4,  1,  1,
    17, 17,  9,  1,  1,  9,  2,  2,  1,  1, 10, 10,  1,  6,  1,  1,
     6, 18,  1,  1, 18,  1,  1,  1,  2,  2,  3,  1,  3,  1,  1,  1,
     4,  1, 19,  1, 19,  7,  1,  1, 20,  1,  4, 20,  1,  7, 11,  2,
     1, 11, 21,  2,  8,  5,  1,  8,  1,  5, 21,  1,  1,  1, 22,  1,
     1, 22,  1,  1,  3,  3,  1, 23,  2, 12, 24,  1,  1,  2,  1,  1,
    12, 23,  1,  1, 24,  1,  1,  1,  4,  1,  1,  1,  2,  1,  6,  6,
     4,  2,  1,  1,  1,  1,  1,  1,  1, 14, 13,  3,  1, 25,  9, 25,
    14,  1,  9,  3, 13,  1,  1,  1,  1,  1, 10,  1,  1,  2, 10,  2},

    /* value table */
    {-20,  -1,   0,   2,  -2,   1,  -1,   3,  -3,   1,  -1,   4,  -4,   0,   2,  -2,
       1,   5,  -1,  -5,   6,  -6,   1,  -1,   7,  -7,   1,  -1,   3,  -3,   8,  -8,
       2,  -2,   1,  -1,   9,  -9,   1,  -1,  10, -10,   4,  -4,  11,   1, -11,  -1,
       2,  -2,   1,  12, -12,  -1,  13, -13,   1,  -1,   3,  -3,  14,   5, -14,  -5,
     -15,  15,  -1,   1,   2,  -2,  16, -16,   1,  17, -17,  -1,   6,  -6,  18, -18,
       2,  -2, -19,  19,  -3,   1,   3,  -1,   4,  20,  -4,   1, -21,  21,   1,   2,
      -1,  -7,   7,  -2,  22, -22,  23,   2, -23,  -2,   1,  -1, -24,  24, -25,  25,
      -8, -26,  26,   8, -27,  27,   5,   3,  -3,  -5,  -4,  28, -28,   4,  29, -29,
       1,  -1,  -2, -30,  30,   2,   9,  -9, -31,  31,   2,  -2, -32,   3,  32, -33,
      -3,   1,  33, -34,  -1,  34, -35,  35, -10,  10,  -6,  36,   6, -36,  37, -37,
      -5,  38,   1, -38,  -1,   3,  39, -39,  -1,  40,   5,   1, -40,  -3,   2, -11,
     -41,  -2,   1,  11,  -3,  -4,  41,   3,  42,   4,  -1, -43, -42,  43,   1, -44,
      45,  -1,  44, -45,  -7,   7, -46,   1, -12,   2,   1, -47,  46,  12,  47,  48,
      -2,  -1, -48,  49,  -1, -50, -49,  50,  -6, -51,  51,  52, -13,  53,  -4,   4,
       6,  13, -53, -52, -54,  55,  54, -55, -56,  -2,   2,  -8,  56,   1,  -3,  -1,
       2,  58,   3,   8,  -2,  57, -58, -60, -59, -57,  -3,  60,  59, -14,   3,  14}
},{
    /* MapTab7 */
    2,  /* eob_sym */
    38, /* esc_sym */
    /* run table */
    {1,  1,  0,  2,  2,  1,  1,  3,  3,  4,  4,  5,  5,  1,  1,  6,
     6,  2,  2,  7,  7,  8,  8,  1,  1,  3,  3,  9,  9, 10, 10,  1,
     1,  2,  2,  4,  4, 11,  0, 11, 12, 12, 13, 13,  1,  1,  5,  5,
    14, 14, 15, 16, 15, 16,  3,  3,  1,  6,  1,  6,  2,  2,  7,  7,
     8,  8, 17, 17,  1,  1,  4,  4, 18, 18,  2,  2,  1, 19,  1, 20,
    19, 20, 21, 21,  3,  3, 22, 22,  5,  5, 24,  1,  1, 23,  9, 23,
    24,  9,  2,  2, 10,  1,  1, 10,  6,  6, 25,  4,  4, 25,  7,  7,
    26,  8,  1,  8,  3,  1, 26,  3, 11, 11, 27, 27,  2, 28,  1,  2,
    28,  1, 12, 12,  5,  5, 29, 13, 13, 29, 32,  1,  1, 33, 31, 30,
    32,  4, 30, 33,  4, 31,  3, 14,  1,  1,  3, 34, 34,  2,  2, 14,
     6,  6, 35, 36, 35, 36,  1, 15,  1, 16, 16, 15,  7,  9,  7,  9,
    37,  8,  8, 37,  1,  1, 39,  2, 38, 39,  2, 40,  5, 38, 40,  5,
     3,  3,  4,  4, 10, 10,  1,  1,  1,  1, 41,  2, 41,  2,  6,  6,
     1,  1, 11, 42, 11, 43,  3, 42,  3, 17,  4, 43,  1, 17,  7,  1,
     8, 44,  4,  7, 44,  5,  8,  2,  5,  1,  2, 48, 45,  1, 12, 45,
    12, 48, 13, 13,  1,  9,  9, 46,  1, 46, 47, 47, 49, 18, 18, 49},

    /* value table */
    { 1,  -1,   0,   1,  -1,   2,  -2,   1,  -1,   1,  -1,   1,  -1,   3,  -3,   1,
     -1,  -2,   2,   1,  -1,   1,  -1,   4,  -4,  -2,   2,   1,  -1,   1,  -1,   5,
     -5,  -3,   3,   2,  -2,   1,   0,  -1,   1,  -1,   1,  -1,   6,  -6,   2,  -2,
      1,  -1,   1,   1,  -1,  -1,  -3,   3,   7,   2,  -7,  -2,  -4,   4,   2,  -2,
      2,  -2,   1,  -1,   8,  -8,   3,  -3,   1,  -1,  -5,   5,   9,   1,  -9,   1,
     -1,  -1,   1,  -1,  -4,   4,   1,  -1,   3,  -3,   1, -10,  10,   1,   2,  -1,
     -1,  -2,   6,  -6,   2,  11, -11,  -2,   3,  -3,   1,  -4,   4,  -1,   3,  -3,
      1,   3,  12,  -3,  -5, -12,  -1,   5,   2,  -2,   1,  -1,  -7,   1,  13,   7,
     -1, -13,   2,  -2,   4,  -4,   1,   2,  -2,  -1,   1,  14, -14,   1,   1,   1,
     -1,  -5,  -1,  -1,   5,  -1,  -6,   2, -15,  15,   6,   1,  -1,  -8,   8,  -2,
     -4,   4,   1,   1,  -1,  -1,  16,   2, -16,  -2,   2,  -2,   4,   3,  -4,  -3,
     -1,  -4,   4,   1, -17,  17,  -1,  -9,   1,   1,   9,   1,  -5,  -1,  -1,   5,
     -7,   7,   6,  -6,   3,  -3,  18, -18,  19, -19,   1, -10,  -1,  10,  -5,   5,
     20, -20,  -3,   1,   3,   1,   8,  -1,  -8,   2,   7,  -1, -21,  -2,   5,  21,
      5,  -1,  -7,  -5,   1,  -6,  -5, -11,   6,  22,  11,   1,   1, -22,  -3,  -1,
      3,  -1,   3,  -3, -23,   4,  -4,   1,  23,  -1,   1,  -1,   1,  -2,   2,  -1}
},{
    /* MapTab8 */
    4,  /* eob_sym */
    11, /* esc_sym */
    /* run table */
    {1,  1,  1,  1,  0,  2,  2,  1,  1,  3,  3,  0,  1,  1,  2,  2,
     4,  4,  1,  1,  5,  5,  1,  1,  2,  2,  3,  3,  6,  6,  1,  1,
     7,  7,  8,  1,  8,  2,  2,  1,  4,  4,  1,  3,  1,  3,  9,  9,
     2,  2,  1,  5,  1,  5, 10, 10,  1,  1, 11, 11,  3,  6,  3,  4,
     4,  6,  2,  2,  1, 12,  1, 12,  7, 13,  7, 13,  1,  1,  8,  8,
     2,  2, 14, 14, 16, 15, 16,  5,  5,  1,  3, 15,  1,  3,  4,  4,
     1,  1, 17, 17,  2,  2,  6,  6,  1, 18,  1, 18, 22, 21, 22, 21,
    25, 24, 25, 19,  9, 20,  9, 23, 19, 24, 20,  3, 23,  7,  3,  1,
     1,  7, 28, 26, 29,  5, 28, 26,  5,  8, 29,  4,  8, 27,  2,  2,
     4, 27,  1,  1, 10, 36, 10, 33, 33, 36, 30,  1, 32, 32,  1, 30,
     6, 31, 31, 35,  3,  6, 11, 11,  3,  2, 35,  2, 34,  1, 34,  1,
    37, 37, 12,  7, 12,  5, 41,  5,  4,  7,  1,  8, 13,  4,  1, 41,
    13, 38,  8, 38,  9,  1, 40, 40,  9,  1, 39,  2,  2, 49, 39, 42,
     3,  3, 14, 16, 49, 14, 16, 42, 43, 43,  6,  6, 15,  1,  1, 15,
    44, 44,  1,  1, 50, 48,  4,  5,  4,  7,  5,  2, 10, 10, 48,  7,
    50, 45,  2,  1, 45,  8,  8,  1, 46, 46,  3, 47, 47,  3,  1,  1},

    /* value table */
    { 1,  -1,   2,  -2,   0,   1,  -1,   3,  -3,   1,  -1,   0,   4,  -4,   2,  -2,
      1,  -1,   5,  -5,   1,  -1,   6,  -6,   3,  -3,   2,  -2,   1,  -1,   7,  -7,
      1,  -1,   1,   8,  -1,   4,  -4,  -8,   2,  -2,   9,   3,  -9,  -3,   1,  -1,
      5,  -5,  10,   2, -10,  -2,   1,  -1,  11, -11,   1,  -1,  -4,   2,   4,   3,
     -3,  -2,   6,  -6,  12,   1, -12,  -1,   2,   1,  -2,  -1,  13, -13,   2,  -2,
      7,  -7,   1,  -1,   1,   1,  -1,   3,  -3,  14,   5,  -1, -14,  -5,   4,  -4,
     15, -15,   1,  -1,   8,  -8,  -3,   3,  16,   1, -16,  -1,   1,   1,  -1,  -1,
      1,   1,  -1,   1,   2,   1,  -2,   1,  -1,  -1,  -1,   6,  -1,   3,  -6,  17,
    -17,  -3,   1,   1,   1,   4,  -1,  -1,  -4,   3,  -1,   5,  -3,  -1,  -9,   9,
     -5,   1,  18, -18,   2,   1,  -2,   1,  -1,  -1,   1,  19,  -1,   1, -19,  -1,
      4,   1,  -1,   1,   7,  -4,  -2,   2,  -7,  10,  -1, -10,   1,  20,  -1, -20,
      1,  -1,   2,   4,  -2,   5,   1,  -5,   6,  -4,  21,   4,   2,  -6, -21,  -1,
     -2,   1,  -4,  -1,  -3,  22,  -1,   1,   3, -22,  -1,  11, -11,   1,   1,   1,
      8,  -8,   2,   2,  -1,  -2,  -2,  -1,   1,  -1,  -5,   5,   2,  23, -23,  -2,
      1,  -1,  24, -24,  -1,  -1,   7,   6,  -7,   5,  -6,  12,  -3,   3,   1,  -5,
      1,   1, -12,  25,  -1,  -5,   5, -25,  -1,   1,   9,   1,  -1,  -9,  26, -26}
}
};
