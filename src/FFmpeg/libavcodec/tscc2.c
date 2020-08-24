/*
 * TechSmith Screen Codec 2 (aka Dora) decoder
 * Copyright (c) 2012 Konstantin Shishkov
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
 * TechSmith Screen Codec 2 decoder
 */

#include <inttypes.h>

#define BITSTREAM_READER_LE
#include "avcodec.h"
#include "bytestream.h"
#include "get_bits.h"
#include "internal.h"
#include "mathops.h"
#include "tscc2data.h"

typedef struct TSCC2Context {
    AVCodecContext *avctx;
    AVFrame       *pic;
    int            mb_width, mb_height;
    uint8_t        *slice_quants;
    int            quant[2];
    int            q[2][3];
    GetBitContext  gb;

    VLC            dc_vlc, nc_vlc[NUM_VLC_SETS], ac_vlc[NUM_VLC_SETS];
    int            block[16];
} TSCC2Context;

{

    }

{

                             tscc2_dc_vlc_bits,  1, 1,
                             tscc2_dc_vlc_codes, 2, 2,
                             tscc2_dc_vlc_syms,  2, 2, INIT_VLC_LE);
        return ret;

                                 tscc2_nc_vlc_syms,     1, 1, INIT_VLC_LE);
            free_vlcs(c);
            return ret;
        }
            free_vlcs(c);
            return ret;
        }
    }

    return 0;
}

#define DEQUANT(val, q) (((q) * (val) + 0x80) >> 8)
#define DCT1D(d0, d1, d2, d3, s0, s1, s2, s3, OP) \
    OP(d0, 5 * ((s0) + (s1) + (s2)) + 2 * (s3));  \
    OP(d1, 5 * ((s0) - (s2) - (s3)) + 2 * (s1));  \
    OP(d2, 5 * ((s0) - (s2) + (s3)) - 2 * (s1));  \
    OP(d3, 5 * ((s0) - (s1) + (s2)) - 2 * (s3));  \

#define COL_OP(a, b)  a = (b)
#define ROW_OP(a, b)  a = ((b) + 0x20) >> 6

{

              tblk[2 * 4 + i], tblk[3 * 4 + i],
    }
              tblk[i * 4 + 0], tblk[i * 4 + 1],
    }

static int tscc2_decode_mb(TSCC2Context *c, int *q, int vlc_set,
                           uint8_t *dst, int stride, int plane)
{
    GetBitContext *gb = &c->gb;
    int prev_dc, dc, nc, ac, bpos, val;
    int i, j, k, l;

    if (get_bits1(gb)) {
        if (get_bits1(gb)) {
            val = get_bits(gb, 8);
            for (i = 0; i < 8; i++, dst += stride)
                memset(dst, val, 16);
        } else {
            if (get_bits_left(gb) < 16 * 8 * 8)
                return AVERROR_INVALIDDATA;
            for (i = 0; i < 8; i++) {
                for (j = 0; j < 16; j++)
                    dst[j] = get_bits(gb, 8);
                dst += stride;
            }
        }
        return 0;
    }

    prev_dc = 0;
    for (j = 0; j < 2; j++) {
        for (k = 0; k < 4; k++) {
            if (!(j | k)) {
                dc = get_bits(gb, 8);
            } else {
                dc = get_vlc2(gb, c->dc_vlc.table, 9, 2);
                if (dc == -1)
                    return AVERROR_INVALIDDATA;
                if (dc == 0x100)
                    dc = get_bits(gb, 8);
            }
            dc          = (dc + prev_dc) & 0xFF;
            prev_dc     = dc;
            c->block[0] = dc;

            nc = get_vlc2(gb, c->nc_vlc[vlc_set].table, 9, 1);
            if (nc == -1)
                return AVERROR_INVALIDDATA;

            bpos = 1;
            memset(c->block + 1, 0, 15 * sizeof(*c->block));
            for (l = 0; l < nc; l++) {
                ac = get_vlc2(gb, c->ac_vlc[vlc_set].table, 9, 2);
                if (ac == -1)
                    return AVERROR_INVALIDDATA;
                if (ac == 0x1000)
                    ac = get_bits(gb, 12);
                bpos += ac & 0xF;
                if (bpos >= 16)
                    return AVERROR_INVALIDDATA;
                val = sign_extend(ac >> 4, 8);
                c->block[ff_zigzag_scan[bpos++]] = val;
            }
            tscc2_idct4_put(c->block, q, dst + k * 4, stride);
        }
        dst += 4 * stride;
    }
    return 0;
}

                              const uint8_t *buf, int buf_size)
{

        return ret;


                return ret;
        }
    }

    return 0;
}

                              int *got_frame, AVPacket *avpkt)
{

        av_log(avctx, AV_LOG_ERROR, "Incorrect frame type %"PRIu32"\n",
               frame_type);
        return AVERROR_INVALIDDATA;
    }

        // Skip duplicate frames
        return buf_size;
    }

        return ret;
    }

        av_log(avctx, AV_LOG_ERROR, "Frame is too short\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "Invalid quantisers %d / %d\n",
               c->quant[0], c->quant[1]);
        return AVERROR_INVALIDDATA;
    }

    }


        av_log(avctx, AV_LOG_ERROR, "Slice properties chunk is too large\n");
        return AVERROR_INVALIDDATA;
    }

            av_log(avctx, AV_LOG_ERROR, "Too many slice properties\n");
            return AVERROR_INVALIDDATA;
        }
    }
        av_log(avctx, AV_LOG_ERROR, "Too few slice properties (%d / %d)\n",
               pos, num_mb);
        return AVERROR_INVALIDDATA;
    }

        } else {
        }
                    c->slice_quants[off + j] == 2) {
                    skip_row = 0;
                    break;
                }
            }
                av_log(avctx, AV_LOG_ERROR, "Non-skip row with zero size\n");
                return AVERROR_INVALIDDATA;
            }
        }
                   size, bytestream2_get_bytes_left(&gb));
        }
            av_log(avctx, AV_LOG_ERROR, "Error decoding slice %d\n", i);
            return ret;
        }
    }

        return ret;

    /* always report that the buffer was completely consumed */
    return buf_size;
}

{


}

{



        av_log(avctx, AV_LOG_ERROR, "Cannot initialise VLCs\n");
        return ret;
    }

        av_log(avctx, AV_LOG_ERROR, "Cannot allocate slice information\n");
        free_vlcs(c);
        return AVERROR(ENOMEM);
    }

        tscc2_decode_end(avctx);
        return AVERROR(ENOMEM);
    }

    return 0;
}

AVCodec ff_tscc2_decoder = {
    .name           = "tscc2",
    .long_name      = NULL_IF_CONFIG_SMALL("TechSmith Screen Codec 2"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_TSCC2,
    .priv_data_size = sizeof(TSCC2Context),
    .init           = tscc2_decode_init,
    .close          = tscc2_decode_end,
    .decode         = tscc2_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
