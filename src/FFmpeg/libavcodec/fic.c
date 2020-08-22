/*
 * Mirillis FIC decoder
 *
 * Copyright (c) 2014 Konstantin Shishkov
 * Copyright (c) 2014 Derek Buitenhuis
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

#include "libavutil/common.h"
#include "libavutil/opt.h"
#include "avcodec.h"
#include "internal.h"
#include "get_bits.h"
#include "golomb.h"

typedef struct FICThreadContext {
    DECLARE_ALIGNED(16, int16_t, block)[64];
    uint8_t *src;
    int slice_h;
    int src_size;
    int y_off;
    int p_frame;
} FICThreadContext;

typedef struct FICContext {
    AVClass *class;
    AVCodecContext *avctx;
    AVFrame *frame;
    AVFrame *final_frame;

    FICThreadContext *slice_data;
    int slice_data_size;

    const uint8_t *qmat;

    enum AVPictureType cur_frame_type;

    int aligned_width, aligned_height;
    int num_slices, slice_h;

    uint8_t cursor_buf[4096];
    int skip_cursor;
} FICContext;

static const uint8_t fic_qmat_hq[64] = {
    1, 2, 2, 2, 3, 3, 3, 4,
    2, 2, 2, 3, 3, 3, 4, 4,
    2, 2, 3, 3, 3, 4, 4, 4,
    2, 2, 3, 3, 3, 4, 4, 5,
    2, 3, 3, 3, 4, 4, 5, 6,
    3, 3, 3, 4, 4, 5, 6, 7,
    3, 3, 3, 4, 4, 5, 7, 7,
    3, 3, 4, 4, 5, 7, 7, 7,
};

static const uint8_t fic_qmat_lq[64] = {
    1,  5,  6,  7,  8,  9,  9, 11,
    5,  5,  7,  8,  9,  9, 11, 12,
    6,  7,  8,  9,  9, 11, 11, 12,
    7,  7,  8,  9,  9, 11, 12, 13,
    7,  8,  9,  9, 10, 11, 13, 16,
    8,  9,  9, 10, 11, 13, 16, 19,
    8,  9,  9, 11, 12, 15, 18, 23,
    9,  9, 11, 12, 15, 18, 23, 27
};

static const uint8_t fic_header[7] = { 0, 0, 1, 'F', 'I', 'C', 'V' };

#define FIC_HEADER_SIZE 27
#define CURSOR_OFFSET 59

{
}

{

    }

    ptr = block;
    }

    ptr = block;
    }
static int fic_decode_block(FICContext *ctx, GetBitContext *gb,
                            uint8_t *dst, int stride, int16_t *block, int *is_p)
{
    int i, num_coeff;

    if (get_bits_left(gb) < 8)
        return AVERROR_INVALIDDATA;

    /* Is it a skip block? */
    if (get_bits1(gb)) {
        *is_p = 1;
        return 0;
    }

    memset(block, 0, sizeof(*block) * 64);

    num_coeff = get_bits(gb, 7);
    if (num_coeff > 64)
        return AVERROR_INVALIDDATA;

    for (i = 0; i < num_coeff; i++) {
        int v = get_se_golomb(gb);
        if (v < -2048 || v > 2048)
             return AVERROR_INVALIDDATA;
        block[ff_zigzag_direct[i]] = v *
                                     ctx->qmat[ff_zigzag_direct[i]];
    }

    fic_idct_put(dst, stride, block);

    return 0;
}

{

        return ret;



            }

        }
    }

    return 0;
}

                                             int size, uint8_t *alpha)
{

}

{

    /* Convert to YUVA444. */

    }

    /* Subsample chroma. */

    /* Seek to x/y pos of cursor. */

    /* Copy. */

                        chroma[1] + (i / 2) * 16, csize, chroma[2] + (i / 2) * 16);

    }

                            int *got_frame, AVPacket *avpkt)
{

        return ret;

    /* Header + at least one slice (4) */
        av_log(avctx, AV_LOG_ERROR, "Frame data is too small.\n");
        return AVERROR_INVALIDDATA;
    }

    /* Check for header. */
        av_log(avctx, AV_LOG_WARNING, "Invalid FIC Header.\n");

    /* Is it a skip frame? */
            av_log(avctx, AV_LOG_WARNING, "Initial frame is skipped\n");
            return AVERROR_INVALIDDATA;
        }
    }

        av_log(avctx, AV_LOG_ERROR, "Zero slices found.\n");
        return AVERROR_INVALIDDATA;
    }

    /* High or Low Quality Matrix? */

    /* Skip cursor data. */
        av_log(avctx, AV_LOG_ERROR,
               "Packet is too small to contain cursor (%d vs %d bytes).\n",
               tsize, avpkt->size - FIC_HEADER_SIZE);
        return AVERROR_INVALIDDATA;
    }

        skip_cursor = 1;

        av_log(avctx, AV_LOG_WARNING,
               "Cursor data too small. Skipping cursor.\n");
        skip_cursor = 1;
    }

    /* Cursor position. */
        av_log(avctx, AV_LOG_DEBUG,
               "Invalid cursor position: (%d,%d). Skipping cursor.\n",
               cur_x, cur_y);
        skip_cursor = 1;
    }

        av_log(avctx, AV_LOG_WARNING,
               "Invalid cursor size. Skipping cursor.\n");
        skip_cursor = 1;
    }

        skip_cursor = 1;
    }

    /* Slice height for all but the last slice. */
        ctx->slice_h = FFALIGN(ctx->slice_h - 16, 16);

    /* First slice offset and remaining data. */

        av_log(avctx, AV_LOG_ERROR, "Not enough frame data to decode.\n");
        return AVERROR_INVALIDDATA;
    }

    /* Allocate slice data. */
                   nslices * sizeof(ctx->slice_data[0]));
        av_log(avctx, AV_LOG_ERROR, "Could not allocate slice data.\n");
        return AVERROR(ENOMEM);
    }


        /*
         * Either read the slice size, or consume all data left.
         * Also, special case the last slight height.
         */
        } else {
                return AVERROR_INVALIDDATA;
        }



    }

                              NULL, nslices, sizeof(ctx->slice_data[0]))) < 0)
        return ret;

        }
    }
        av_log(avctx, AV_LOG_ERROR, "Could not clone frame buffer.\n");
        return AVERROR(ENOMEM);
    }

    /* Make sure we use a user-supplied buffer. */
        av_log(avctx, AV_LOG_ERROR, "Could not make frame writable.\n");
        return ret;
    }

    /* Draw cursor. */
    }

skip:
        return ret;

}

{


}

{

    /* Initialize various context values */


        return AVERROR(ENOMEM);

    return 0;
}

static const AVOption options[] = {
{ "skip_cursor", "skip the cursor", offsetof(FICContext, skip_cursor), AV_OPT_TYPE_BOOL, {.i64 = 0 }, 0, 1, AV_OPT_FLAG_DECODING_PARAM | AV_OPT_FLAG_VIDEO_PARAM },
{ NULL },
};

static const AVClass fic_decoder_class = {
    .class_name = "FIC decoder",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
};

AVCodec ff_fic_decoder = {
    .name           = "fic",
    .long_name      = NULL_IF_CONFIG_SMALL("Mirillis FIC"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_FIC,
    .priv_data_size = sizeof(FICContext),
    .init           = fic_decode_init,
    .decode         = fic_decode_frame,
    .close          = fic_decode_close,
    .capabilities   = AV_CODEC_CAP_DR1 | AV_CODEC_CAP_SLICE_THREADS,
    .priv_class     = &fic_decoder_class,
};
