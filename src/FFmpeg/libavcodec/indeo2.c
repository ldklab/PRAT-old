/*
 * Intel Indeo 2 codec
 * Copyright (c) 2005 Konstantin Shishkov
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
 * Intel Indeo 2 decoder.
 */

#include "libavutil/attributes.h"

#define BITSTREAM_READER_LE
#include "avcodec.h"
#include "get_bits.h"
#include "indeo2data.h"
#include "internal.h"
#include "mathops.h"

typedef struct Ir2Context{
    AVCodecContext *avctx;
    AVFrame *picture;
    GetBitContext gb;
    int decode_delta;
} Ir2Context;

#define CODE_VLC_BITS 14
static VLC ir2_vlc;

/* Indeo 2 codes are in range 0x01..0x7F and 0x81..0x90 */
{
}

                            int pitch, const uint8_t *table)
{

        return AVERROR_INVALIDDATA;

    /* first line contain absolute values, other lines contain deltas */
                return AVERROR_INVALIDDATA;
        } else { /* copy two values from table */
                return AVERROR_INVALIDDATA;
        }
    }

        out = 0;
                return AVERROR_INVALIDDATA;
                    return AVERROR_INVALIDDATA;
                }
            } else { /* add two deltas from table */
                    return AVERROR_INVALIDDATA;
            }
        }
    }
    return 0;
}

                                  int pitch, const uint8_t *table)
{

        return AVERROR_INVALIDDATA;

        out = 0;
                return AVERROR_INVALIDDATA;
            } else { /* add two deltas from table */
                    return AVERROR_INVALIDDATA;
            }
        }
    }
    return 0;
}

                        void *data, int *got_frame,
                        AVPacket *avpkt)
{

        return ret;


        av_log(s->avctx, AV_LOG_ERROR, "input buffer size too small (%d)\n", buf_size);
        return AVERROR_INVALIDDATA;
    }


    /* decide whether frame uses deltas or not */
#ifndef BITSTREAM_READER_LE
    for (i = 0; i < buf_size; i++)
        buf[i] = ff_reverse[buf[i]];
#endif

        return ret;


        av_log(avctx, AV_LOG_ERROR, "ctab %d is invalid\n", ctab);
        return AVERROR_INVALIDDATA;
    }

                                    p->data[0], p->linesize[0],
            return ret;

        /* swapped U and V */
                                    p->data[2], p->linesize[2],
            return ret;
                                    p->data[1], p->linesize[1],
                                    ir2_delta_table[ctab])) < 0)
            return ret;
    } else { /* interframe */
                                          p->data[0], p->linesize[0],
            return ret;
        /* swapped U and V */
                                          p->data[2], p->linesize[2],
            return ret;
                                          p->data[1], p->linesize[1],
                                          ir2_delta_table[ctab])) < 0)
            return ret;
    }

        return ret;


}

{



        return AVERROR(ENOMEM);

#ifdef BITSTREAM_READER_LE
                 &ir2_codes[0][1], 4, 2,
                 &ir2_codes[0][0], 4, 2, INIT_VLC_USE_NEW_STATIC | INIT_VLC_LE);
#else
        init_vlc(&ir2_vlc, CODE_VLC_BITS, IR2_CODES,
                 &ir2_codes[0][1], 4, 2,
                 &ir2_codes[0][0], 4, 2, INIT_VLC_USE_NEW_STATIC);
#endif

}

{


}

AVCodec ff_indeo2_decoder = {
    .name           = "indeo2",
    .long_name      = NULL_IF_CONFIG_SMALL("Intel Indeo 2"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_INDEO2,
    .priv_data_size = sizeof(Ir2Context),
    .init           = ir2_decode_init,
    .close          = ir2_decode_end,
    .decode         = ir2_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
