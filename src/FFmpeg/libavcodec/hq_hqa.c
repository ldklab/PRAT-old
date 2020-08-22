/*
 * Canopus HQ/HQA decoder
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

#include <stdint.h>

#include "libavutil/attributes.h"
#include "libavutil/intreadwrite.h"

#include "avcodec.h"
#include "canopus.h"
#include "get_bits.h"
#include "internal.h"

#include "hq_hqa.h"
#include "hq_hqadsp.h"

/* HQ/HQA slices are a set of macroblocks belonging to a frame, and
 * they usually form a pseudorandom pattern (probably because it is
 * nicer to display on partial decode).
 *
 * For HQA it just happens that each slice is on every 8th macroblock,
 * but they can be on any frame width like
 *   X.......X.
 *   ......X...
 *   ....X.....
 *   ..X.......
 * etc.
 *
 * The original decoder has special handling for edge macroblocks,
 * while lavc simply aligns coded_width and coded_height.
 */

static inline void put_blocks(HQContext *c, AVFrame *pic,
                              int plane, int x, int y, int ilace,
                              int16_t *block0, int16_t *block1)
{
    uint8_t *p = pic->data[plane] + x;

    c->hqhqadsp.idct_put(p + y * pic->linesize[plane],
                         pic->linesize[plane] << ilace, block0);
    c->hqhqadsp.idct_put(p + (y + (ilace ? 1 : 8)) * pic->linesize[plane],
                         pic->linesize[plane] << ilace, block1);
}

static int hq_decode_block(HQContext *c, GetBitContext *gb, int16_t block[64],
                           int qsel, int is_chroma, int is_hqa)
{
    const int32_t *q;
    int val, pos = 1;

    memset(block, 0, 64 * sizeof(*block));

    if (!is_hqa) {
        block[0] = get_sbits(gb, 9) * 64;
        q = ff_hq_quants[qsel][is_chroma][get_bits(gb, 2)];
    } else {
        q = ff_hq_quants[qsel][is_chroma][get_bits(gb, 2)];
        block[0] = get_sbits(gb, 9) * 64;
    }

    for (;;) {
        val = get_vlc2(gb, c->hq_ac_vlc.table, 9, 2);
        if (val < 0)
            return AVERROR_INVALIDDATA;

        pos += ff_hq_ac_skips[val];
        if (pos >= 64)
            break;
        block[ff_zigzag_direct[pos]] = (int)(ff_hq_ac_syms[val] * (unsigned)q[pos]) >> 12;
        pos++;
    }

    return 0;
}

                        GetBitContext *gb, int x, int y)
{


            return ret;
    }


}

                           int prof_num, size_t data_size)
{

        profile = &ff_hq_profile[0];
        avpriv_request_sample(ctx->avctx, "HQ Profile %d", prof_num);
    } else {
    }


        return ret;

    /* Offsets are stored from CUV position, so adjust them accordingly. */

    next_off = 0;

            av_log(ctx->avctx, AV_LOG_ERROR,
                   "Invalid slice size %"SIZE_SPECIFIER".\n", data_size);
            break;
        }

                av_log(ctx->avctx, AV_LOG_ERROR,
                       "Error decoding macroblock %d at slice %d.\n", i, slice);
                return ret;
            }
        }
    }

    return 0;
}

                         GetBitContext *gb, int x, int y)
{

        return AVERROR_INVALIDDATA;




                return ret;
        }
    }


}

                            int quant, int slice_no, int w, int h)
{

                av_log(ctx->avctx, AV_LOG_ERROR,
                       "Error decoding macroblock at %dx%d.\n", i, j);
                return ret;
            }
        }
    }

    return 0;
}

{

        return AVERROR_INVALIDDATA;


        return ret;



        av_log(ctx->avctx, AV_LOG_ERROR,
               "Invalid quantization matrix %d.\n", quant);
        return AVERROR_INVALIDDATA;
    }

        return ret;

    /* Offsets are stored from HQA1 position, so adjust them accordingly. */

            av_log(ctx->avctx, AV_LOG_ERROR,
                   "Invalid slice size %"SIZE_SPECIFIER".\n", data_size);
            break;
        }

            return ret;
    }

    return 0;
}

                               int *got_frame, AVPacket *avpkt)
{

        av_log(avctx, AV_LOG_ERROR, "Frame is too small (%d).\n", avpkt->size);
        return AVERROR_INVALIDDATA;
    }

            av_log(avctx, AV_LOG_ERROR, "Invalid INFO size (%d).\n", info_size);
            return AVERROR_INVALIDDATA;
        }

    }

        av_log(avctx, AV_LOG_ERROR, "Frame is too small (%d).\n", data_size);
        return AVERROR_INVALIDDATA;
    }

    /* HQ defines dimensions and number of slices, and thus slice traversal
     * order. HQA has no size constraint and a fixed number of slices, so it
     * needs a separate scheme for it. */
    } else {
        av_log(avctx, AV_LOG_ERROR, "Not a HQ/HQA frame.\n");
        return AVERROR_INVALIDDATA;
    }
        av_log(avctx, AV_LOG_ERROR, "Error decoding frame.\n");
        return ret;
    }



}

{


}

{


}

AVCodec ff_hq_hqa_decoder = {
    .name           = "hq_hqa",
    .long_name      = NULL_IF_CONFIG_SMALL("Canopus HQ/HQA"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_HQ_HQA,
    .priv_data_size = sizeof(HQContext),
    .init           = hq_hqa_decode_init,
    .decode         = hq_hqa_decode_frame,
    .close          = hq_hqa_decode_close,
    .capabilities   = AV_CODEC_CAP_DR1,
    .caps_internal  = FF_CODEC_CAP_INIT_THREADSAFE |
                      FF_CODEC_CAP_INIT_CLEANUP,
};
