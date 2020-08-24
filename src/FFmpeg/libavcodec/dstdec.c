/*
 * Direct Stream Transfer (DST) decoder
 * Copyright (c) 2014 Peter Ross <pross@xvid.org>
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
 * Direct Stream Transfer (DST) decoder
 * ISO/IEC 14496-3 Part 3 Subpart 10: Technical description of lossless coding of oversampled audio
 */

#include "libavutil/avassert.h"
#include "libavutil/intreadwrite.h"
#include "internal.h"
#include "get_bits.h"
#include "avcodec.h"
#include "golomb.h"
#include "mathops.h"
#include "dsd.h"

#define DST_MAX_CHANNELS 6
#define DST_MAX_ELEMENTS (2 * DST_MAX_CHANNELS)

#define DSD_FS44(sample_rate) (sample_rate * 8LL / 44100)

#define DST_SAMPLES_PER_FRAME(sample_rate) (588 * DSD_FS44(sample_rate))

static const int8_t fsets_code_pred_coeff[3][3] = {
    {  -8 },
    { -16,  8 },
    {  -9, -5, 6 },
};

static const int8_t probs_code_pred_coeff[3][3] = {
    {  -8 },
    { -16,  8 },
    { -24, 24, -8 },
};

typedef struct ArithCoder {
    unsigned int a;
    unsigned int c;
} ArithCoder;

typedef struct Table {
    unsigned int elements;
    unsigned int length[DST_MAX_ELEMENTS];
    int coeff[DST_MAX_ELEMENTS][128];
} Table;

typedef struct DSTContext {
    AVClass *class;

    GetBitContext gb;
    ArithCoder ac;
    Table fsets, probs;
    DECLARE_ALIGNED(16, uint8_t, status)[DST_MAX_CHANNELS][16];
    DECLARE_ALIGNED(16, int16_t, filter)[DST_MAX_ELEMENTS][16][256];
    DSDContext dsdctx[DST_MAX_CHANNELS];
} DSTContext;

{

        avpriv_request_sample(avctx, "Channel count %d", avctx->channels);
        return AVERROR_PATCHWELCOME;
    }

    // the sample rate is only allowed to be 64,128,256 * 44100 by ISO/IEC 14496-3:2005(E)
    // We are a bit more tolerant here, but this check is needed to bound the size and duration
        return AVERROR_INVALIDDATA;


        return AVERROR_PATCHWELCOME;
    }




}

static int read_map(GetBitContext *gb, Table *t, unsigned int map[DST_MAX_CHANNELS], int channels)
{
    int ch;
    t->elements = 1;
    map[0] = 0;
    if (!get_bits1(gb)) {
        for (ch = 1; ch < channels; ch++) {
            int bits = av_log2(t->elements) + 1;
            map[ch] = get_bits(gb, bits);
            if (map[ch] == t->elements) {
                t->elements++;
                if (t->elements >= DST_MAX_ELEMENTS)
                    return AVERROR_INVALIDDATA;
            } else if (map[ch] > t->elements) {
                return AVERROR_INVALIDDATA;
            }
        }
    } else {
        memset(map, 0, sizeof(*map) * DST_MAX_CHANNELS);
    }
    return 0;
}

static av_always_inline int get_sr_golomb_dst(GetBitContext *gb, unsigned int k)
{
    int v = get_ur_golomb_jpegls(gb, k, get_bits_left(gb), 0);
    if (v && get_bits1(gb))
        v = -v;
    return v;
}

                               int coeff_bits, int is_signed, int offset)
{

    }

                      int length_bits, int coeff_bits, int is_signed, int offset)
{
        } else {
            int method = get_bits(gb, 2), lsb_size;
            if (method == 3)
                return AVERROR_INVALIDDATA;

            read_uncoded_coeff(gb, t->coeff[i], method + 1, coeff_bits, is_signed, offset);

            lsb_size  = get_bits(gb, 3);
            for (j = method + 1; j < t->length[i]; j++) {
                int c, x = 0;
                for (k = 0; k < method + 1; k++)
                    x += code_pred_coeff[method][k] * (unsigned)t->coeff[i][j - k - 1];
                c = get_sr_golomb_dst(gb, lsb_size);
                if (x >= 0)
                    c -= (x + 4) / 8;
                else
                    c += (-x + 3) / 8;
                if (!is_signed) {
                    if (c < offset || c >= offset + (1<<coeff_bits))
                        return AVERROR_INVALIDDATA;
                }
                t->coeff[i][j] = c;
            }
        }
    }
    return 0;
}

{
}

{

    } else {
    }

    }
}

{
}

{



                int v = 0;

            }
        }
    }

                        int *got_frame_ptr, AVPacket *avpkt)
{

        return AVERROR_INVALIDDATA;

        return ret;

        return ret;

        skip_bits1(gb);
        if (get_bits(gb, 6))
            return AVERROR_INVALIDDATA;
        memcpy(frame->data[0], avpkt->data + 1, FFMIN(avpkt->size - 1, frame->nb_samples * channels));
        goto dsd;
    }

    /* Segmentation (10.4, 10.5, 10.6) */

        avpriv_request_sample(avctx, "Not Same Segmentation");
        return AVERROR_PATCHWELCOME;
    }

        avpriv_request_sample(avctx, "Not Same Segmentation For All Channels");
        return AVERROR_PATCHWELCOME;
    }

        avpriv_request_sample(avctx, "Not End Of Channel Segmentation");
        return AVERROR_PATCHWELCOME;
    }

    /* Mapping (10.7, 10.8, 10.9) */


        return ret;

    } else {
        avpriv_request_sample(avctx, "Not Same Mapping");
        if ((ret = read_map(gb, &s->probs, map_ch_to_pelem, channels)) < 0)
            return ret;
    }

    /* Half Probability (10.10) */


    /* Filter Coef Sets (10.12) */

        return ret;

    /* Probability Tables (10.13) */

        return ret;

    /* Arithmetic Coded Data (10.11) */

        return AVERROR_INVALIDDATA;





#define F(x) filter[(x)][status[(x)]]
#undef F

            } else {
                prob = 128;
            }


        }
    }

    }


}

AVCodec ff_dst_decoder = {
    .name           = "dst",
    .long_name      = NULL_IF_CONFIG_SMALL("DST (Digital Stream Transfer)"),
    .type           = AVMEDIA_TYPE_AUDIO,
    .id             = AV_CODEC_ID_DST,
    .priv_data_size = sizeof(DSTContext),
    .init           = decode_init,
    .decode         = decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
    .sample_fmts    = (const enum AVSampleFormat[]) { AV_SAMPLE_FMT_FLT,
                                                      AV_SAMPLE_FMT_NONE },
};
