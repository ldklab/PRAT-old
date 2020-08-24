/*
 * RealAudio 2.0 (28.8K)
 * Copyright (c) 2003 The FFmpeg project
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

#include "libavutil/channel_layout.h"
#include "libavutil/float_dsp.h"
#include "libavutil/internal.h"

#define BITSTREAM_READER_LE
#include "avcodec.h"
#include "celp_filters.h"
#include "get_bits.h"
#include "internal.h"
#include "lpc.h"
#include "ra288.h"

#define MAX_BACKWARD_FILTER_ORDER  36
#define MAX_BACKWARD_FILTER_LEN    40
#define MAX_BACKWARD_FILTER_NONREC 35

#define RA288_BLOCK_SIZE        5
#define RA288_BLOCKS_PER_FRAME 32

typedef struct RA288Context {
    AVFloatDSPContext *fdsp;
    DECLARE_ALIGNED(32, float,   sp_lpc)[FFALIGN(36, 16)];   ///< LPC coefficients for speech data (spec: A)
    DECLARE_ALIGNED(32, float, gain_lpc)[FFALIGN(10, 16)];   ///< LPC coefficients for gain        (spec: GB)

    /** speech data history                                      (spec: SB).
     *  Its first 70 coefficients are updated only at backward filtering.
     */
    float sp_hist[111];

    /// speech part of the gain autocorrelation                  (spec: REXP)
    float sp_rec[37];

    /** log-gain history                                         (spec: SBLG).
     *  Its first 28 coefficients are updated only at backward filtering.
     */
    float gain_hist[38];

    /// recursive part of the gain autocorrelation               (spec: REXPLG)
    float gain_rec[11];
} RA288Context;

{


}

{


        av_log(avctx, AV_LOG_ERROR, "unsupported block align\n");
        return AVERROR_PATCHWELCOME;
    }

        return AVERROR(ENOMEM);

    return 0;
}

{


{


    /* block 46 of G.728 spec */

    /* block 47 of G.728 spec */

    /* block 48 of G.728 spec */
    /* exp(sum * 0.1151292546497) == pow(10.0,sum/20) */




    /* shift and store */



/**
 * Hybrid window filtering, see blocks 36 and 49 of the G.728 specification.
 *
 * @param order   filter order
 * @param n       input length
 * @param non_rec number of non-recursive samples
 * @param out     filter output
 * @param hist    pointer to the input history of the filter
 * @param out     pointer to the non-recursive part of the output
 * @param out2    pointer to the recursive part of the output
 * @param window  pointer to the windowing function table
 */
static void do_hybrid_window(RA288Context *ractx,
                             int order, int n, int non_rec, float *out,
                             float *hist, float *out2, const float *window)
{
    int i;
    float buffer1[MAX_BACKWARD_FILTER_ORDER + 1];
    float buffer2[MAX_BACKWARD_FILTER_ORDER + 1];
    LOCAL_ALIGNED(32, float, work, [FFALIGN(MAX_BACKWARD_FILTER_ORDER +
                                            MAX_BACKWARD_FILTER_LEN   +
                                            MAX_BACKWARD_FILTER_NONREC, 16)]);

    av_assert2(order>=0);

    ractx->fdsp->vector_fmul(work, window, hist, FFALIGN(order + n + non_rec, 16));

    convolve(buffer1, work + order    , n      , order);
    convolve(buffer2, work + order + n, non_rec, order);

    for (i=0; i <= order; i++) {
        out2[i] = out2[i] * 0.5625 + buffer1[i];
        out [i] = out2[i]          + buffer2[i];
    }

    /* Multiply by the white noise correcting factor (WNCF). */
    *out *= 257.0 / 256.0;
}

/**
 * Backward synthesis filter, find the LPC coefficients from past speech data.
 */
                            float *hist, float *rec, const float *window,
                            float *lpc, const float *tab,
                            int order, int n, int non_rec, int move_size)
{




                              int *got_frame_ptr, AVPacket *avpkt)
{

        av_log(avctx, AV_LOG_ERROR,
               "Error! Input buffer is too small [%d<%d]\n",
               buf_size, avctx->block_align);
        return AVERROR_INVALIDDATA;
    }

        return ret;

    /* get output buffer */
        return ret;





        }
    }


}

AVCodec ff_ra_288_decoder = {
    .name           = "real_288",
    .long_name      = NULL_IF_CONFIG_SMALL("RealAudio 2.0 (28.8K)"),
    .type           = AVMEDIA_TYPE_AUDIO,
    .id             = AV_CODEC_ID_RA_288,
    .priv_data_size = sizeof(RA288Context),
    .init           = ra288_decode_init,
    .decode         = ra288_decode_frame,
    .close          = ra288_decode_close,
    .capabilities   = AV_CODEC_CAP_DR1,
};
