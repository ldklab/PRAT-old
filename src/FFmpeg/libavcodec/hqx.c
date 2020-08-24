/*
 * Canopus HQX decoder
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

#include <inttypes.h>

#include "libavutil/imgutils.h"
#include "libavutil/intreadwrite.h"

#include "avcodec.h"
#include "canopus.h"
#include "get_bits.h"
#include "internal.h"
#include "thread.h"

#include "hqx.h"
#include "hqxdsp.h"

/* HQX has four modes - 422, 444, 422alpha and 444alpha - all 12-bit */
enum HQXFormat {
    HQX_422 = 0,
    HQX_444,
    HQX_422A,
    HQX_444A,
};

#define HQX_HEADER_SIZE 59

/* macroblock selects a group of 4 possible quants and
 * a block can use any of those four quantisers
 * one column is powers of 2, the other one is powers of 2 * 3,
 * then there is the special one, powers of 2 * 5 */
static const int hqx_quants[16][4] = {
    {  0x1,   0x2,   0x4,   0x8 }, {  0x1,  0x3,   0x6,   0xC },
    {  0x2,   0x4,   0x8,  0x10 }, {  0x3,  0x6,   0xC,  0x18 },
    {  0x4,   0x8,  0x10,  0x20 }, {  0x6,  0xC,  0x18,  0x30 },
    {  0x8,  0x10,  0x20,  0x40 },
                      { 0xA, 0x14, 0x28, 0x50 },
                                   {  0xC, 0x18,  0x30,  0x60 },
    { 0x10,  0x20,  0x40,  0x80 }, { 0x18, 0x30,  0x60,  0xC0 },
    { 0x20,  0x40,  0x80, 0x100 }, { 0x30, 0x60,  0xC0, 0x180 },
    { 0x40,  0x80, 0x100, 0x200 }, { 0x60, 0xC0, 0x180, 0x300 },
    { 0x80, 0x100, 0x200, 0x400 }
};

static const uint8_t hqx_quant_luma[64] = {
    16,  16,  16,  19,  19,  19,  42,  44,
    16,  16,  19,  19,  19,  38,  43,  45,
    16,  19,  19,  19,  40,  41,  45,  48,
    19,  19,  19,  40,  41,  42,  46,  49,
    19,  19,  40,  41,  42,  43,  48, 101,
    19,  38,  41,  42,  43,  44,  98, 104,
    42,  43,  45,  46,  48,  98, 109, 116,
    44,  45,  48,  49, 101, 104, 116, 123,
};

static const uint8_t hqx_quant_chroma[64] = {
    16,  16,  19,  25,  26,  26,  42,  44,
    16,  19,  25,  25,  26,  38,  43,  91,
    19,  25,  26,  27,  40,  41,  91,  96,
    25,  25,  27,  40,  41,  84,  93, 197,
    26,  26,  40,  41,  84,  86, 191, 203,
    26,  38,  41,  84,  86, 177, 197, 209,
    42,  43,  91,  93, 191, 197, 219, 232,
    44,  91,  96, 197, 203, 209, 232, 246,
};

static inline void put_blocks(HQXContext *ctx, int plane,
                              int x, int y, int ilace,
                              int16_t *block0, int16_t *block1,
                              const uint8_t *quant)
{
    int fields = ilace ? 2 : 1;
    int lsize = ctx->pic->linesize[plane];
    uint8_t *p = ctx->pic->data[plane] + x * 2;

    ctx->hqxdsp.idct_put((uint16_t *)(p + y * lsize),
                         lsize * fields, block0, quant);
    ctx->hqxdsp.idct_put((uint16_t *)(p + (y + (ilace ? 1 : 8)) * lsize),
                         lsize * fields, block1, quant);
}

                              int *run, int *lev)
{

    }

static int decode_block(GetBitContext *gb, VLC *vlc,
                        const int *quants, int dcb,
                        int16_t block[64], int *last_dc)
{
    int q, dc;
    int ac_idx;
    int run, lev, pos = 1;

    memset(block, 0, 64 * sizeof(*block));
    dc = get_vlc2(gb, vlc->table, HQX_DC_VLC_BITS, 2);
    if (dc < 0)
        return AVERROR_INVALIDDATA;
    *last_dc += dc;

    block[0] = sign_extend(*last_dc << (12 - dcb), 12);

    q = quants[get_bits(gb, 2)];
    if (q >= 128)
        ac_idx = HQX_AC_Q128;
    else if (q >= 64)
        ac_idx = HQX_AC_Q64;
    else if (q >= 32)
        ac_idx = HQX_AC_Q32;
    else if (q >= 16)
        ac_idx = HQX_AC_Q16;
    else if (q >= 8)
        ac_idx = HQX_AC_Q8;
    else
        ac_idx = HQX_AC_Q0;

    do {
        hqx_get_ac(gb, &ff_hqx_ac[ac_idx], &run, &lev);
        pos += run;
        if (pos >= 64)
            break;
        block[ff_zigzag_direct[pos++]] = lev * q;
    } while (pos < 64);

    return 0;
}

{

    else
        flag = 0;


            return ret;
    }


}

{




                    return ret;
            }
        }
    }


}

static int hqx_decode_444(HQXContext *ctx, int slice_no, int x, int y)
{
    HQXSlice *slice = &ctx->slice[slice_no];
    GetBitContext *gb = &slice->gb;
    const int *quants;
    int flag;
    int last_dc;
    int i, ret;

    if (ctx->interlaced)
        flag = get_bits1(gb);
    else
        flag = 0;

    quants = hqx_quants[get_bits(gb, 4)];

    for (i = 0; i < 12; i++) {
        int vlc_index = ctx->dcb - 9;
        if (i == 0 || i == 4 || i == 8)
            last_dc = 0;
        ret = decode_block(gb, &ctx->dc_vlc[vlc_index], quants,
                           ctx->dcb, slice->block[i], &last_dc);
        if (ret < 0)
            return ret;
    }

    put_blocks(ctx, 0, x,     y, flag, slice->block[0], slice->block[ 2], hqx_quant_luma);
    put_blocks(ctx, 0, x + 8, y, flag, slice->block[1], slice->block[ 3], hqx_quant_luma);
    put_blocks(ctx, 2, x,     y, flag, slice->block[4], slice->block[ 6], hqx_quant_chroma);
    put_blocks(ctx, 2, x + 8, y, flag, slice->block[5], slice->block[ 7], hqx_quant_chroma);
    put_blocks(ctx, 1, x,     y, flag, slice->block[8], slice->block[10], hqx_quant_chroma);
    put_blocks(ctx, 1, x + 8, y, flag, slice->block[9], slice->block[11], hqx_quant_chroma);

    return 0;
}

static int hqx_decode_444a(HQXContext *ctx, int slice_no, int x, int y)
{
    HQXSlice *slice = &ctx->slice[slice_no];
    GetBitContext *gb = &slice->gb;
    const int *quants;
    int flag = 0;
    int last_dc;
    int i, ret;
    int cbp;

    cbp = get_vlc2(gb, ctx->cbp_vlc.table, ctx->cbp_vlc.bits, 1);

    for (i = 0; i < 16; i++)
        memset(slice->block[i], 0, sizeof(**slice->block) * 64);
    for (i = 0; i < 16; i++)
        slice->block[i][0] = -0x800;
    if (cbp) {
        if (ctx->interlaced)
            flag = get_bits1(gb);

        quants = hqx_quants[get_bits(gb, 4)];

        cbp |= cbp << 4; // alpha CBP
        cbp |= cbp << 8; // chroma CBP
        for (i = 0; i < 16; i++) {
            if (i == 0 || i == 4 || i == 8 || i == 12)
                last_dc = 0;
            if (cbp & (1 << i)) {
                int vlc_index = ctx->dcb - 9;
                ret = decode_block(gb, &ctx->dc_vlc[vlc_index], quants,
                                   ctx->dcb, slice->block[i], &last_dc);
                if (ret < 0)
                    return ret;
            }
        }
    }

    put_blocks(ctx, 3, x,     y, flag, slice->block[ 0], slice->block[ 2], hqx_quant_luma);
    put_blocks(ctx, 3, x + 8, y, flag, slice->block[ 1], slice->block[ 3], hqx_quant_luma);
    put_blocks(ctx, 0, x,     y, flag, slice->block[ 4], slice->block[ 6], hqx_quant_luma);
    put_blocks(ctx, 0, x + 8, y, flag, slice->block[ 5], slice->block[ 7], hqx_quant_luma);
    put_blocks(ctx, 2, x,     y, flag, slice->block[ 8], slice->block[10], hqx_quant_chroma);
    put_blocks(ctx, 2, x + 8, y, flag, slice->block[ 9], slice->block[11], hqx_quant_chroma);
    put_blocks(ctx, 1, x,     y, flag, slice->block[12], slice->block[14], hqx_quant_chroma);
    put_blocks(ctx, 1, x + 8, y, flag, slice->block[13], slice->block[15], hqx_quant_chroma);

    return 0;
}

static const int shuffle_16[16] = {
    0, 5, 11, 14, 2, 7, 9, 13, 1, 4, 10, 15, 3, 6, 8, 12
};

{

        }
            else
            } else {
            }
                mb_x +=            pos % grp_v_rest;
                mb_y  = loc_row + (pos / grp_v_rest);
            } else {
            }
        }
    }

}

                               int slice_no, int threadnr)
{

        av_log(avctx, AV_LOG_ERROR, "Invalid slice size %d.\n", ctx->data_size);
        return AVERROR_INVALIDDATA;
    }

        return ret;

}

                            int *got_picture_ptr, AVPacket *avpkt)
{

        av_log(avctx, AV_LOG_ERROR, "Frame is too small %d.\n", avpkt->size);
        return AVERROR_INVALIDDATA;
    }

            av_log(avctx, AV_LOG_ERROR,
                   "Invalid INFO header offset: 0x%08"PRIX32" is too large.\n",
                   info_offset);
            return AVERROR_INVALIDDATA;
        }

    }


        av_log(avctx, AV_LOG_ERROR, "Frame too small.\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "Not an HQX frame.\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "Invalid DC precision %d.\n", ctx->dcb);
        return AVERROR_INVALIDDATA;
    }
        av_log(avctx, AV_LOG_ERROR, "Invalid stored dimensions %dx%d.\n",
               ctx->width, ctx->height);
        return AVERROR_INVALIDDATA;
    }


    //The minimum size is 2bit per macroblock
    // hqx_decode_422 & hqx_decode_444 have a unconditionally stored 4bits hqx_quants index
    // hqx_decode_422a & hqx_decode_444a use cbp_vlc which has a minimum length of 2 bits for its VLCs
    // The code rejects slices overlapping in their input data
        return AVERROR_INVALIDDATA;

    case HQX_444:
        avctx->pix_fmt = AV_PIX_FMT_YUV444P16;
        ctx->decode_func = hqx_decode_444;
        break;
    case HQX_444A:
        avctx->pix_fmt = AV_PIX_FMT_YUVA444P16;
        ctx->decode_func = hqx_decode_444a;
        break;
    default:
        av_log(avctx, AV_LOG_ERROR, "Invalid format: %d.\n", ctx->format);
        return AVERROR_INVALIDDATA;
    }

        return ret;




}

{

    }

}

{


}

AVCodec ff_hqx_decoder = {
    .name           = "hqx",
    .long_name      = NULL_IF_CONFIG_SMALL("Canopus HQX"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_HQX,
    .priv_data_size = sizeof(HQXContext),
    .init           = hqx_decode_init,
    .decode         = hqx_decode_frame,
    .close          = hqx_decode_close,
    .capabilities   = AV_CODEC_CAP_DR1 | AV_CODEC_CAP_SLICE_THREADS |
                      AV_CODEC_CAP_FRAME_THREADS,
    .caps_internal  = FF_CODEC_CAP_INIT_THREADSAFE |
                      FF_CODEC_CAP_INIT_CLEANUP,
};
