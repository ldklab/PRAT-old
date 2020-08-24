/*
 * Sony PlayStation MDEC (Motion DECoder)
 * Copyright (c) 2003 Michael Niedermayer
 *
 * based upon code from Sebastian Jedruszkiewicz <elf@frogger.rules.pl>
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
 * Sony PlayStation MDEC (Motion DECoder)
 * This is very similar to intra-only MPEG-1.
 */

#include "avcodec.h"
#include "blockdsp.h"
#include "bswapdsp.h"
#include "idctdsp.h"
#include "mpeg12.h"
#include "thread.h"

typedef struct MDECContext {
    AVCodecContext *avctx;
    BlockDSPContext bdsp;
    BswapDSPContext bbdsp;
    IDCTDSPContext idsp;
    ThreadFrame frame;
    GetBitContext gb;
    ScanTable scantable;
    int version;
    int qscale;
    int last_dc[3];
    int mb_width;
    int mb_height;
    int mb_x, mb_y;
    DECLARE_ALIGNED(32, int16_t, block)[6][64];
    DECLARE_ALIGNED(16, uint16_t, quant_matrix)[64];
    uint8_t *bitstream_buffer;
    unsigned int bitstream_buffer_size;
    int block_last_index[6];
} MDECContext;

//very similar to MPEG-1
{

    /* DC coefficient */
    } else {
            return AVERROR_INVALIDDATA;
    }

    {
        /* now quantify & encode AC coefficients */

                break;
                           "ac-tex damaged at %d %d\n", a->mb_x, a->mb_y);
                }
            } else {
                /* escape */
                    av_log(a->avctx, AV_LOG_ERROR,
                           "ac-tex damaged at %d %d\n", a->mb_x, a->mb_y);
                    return AVERROR_INVALIDDATA;
                }
                } else {
                }
            }

        }
    }
}

{


                                           block_index[i])) < 0)
            return AVERROR_INVALIDDATA;
    }
    return 0;
}

{



    }

                        void *data, int *got_frame,
                        AVPacket *avpkt)
{

        return ret;

        return AVERROR(ENOMEM);
        return ret;

    /* skip over 4 preamble bytes in stream (typically 0xXX 0xXX 0x00 0x38) */




        }
    }


}

{



                      ff_zigzag_direct);


    /* init q matrix */

    }

}

{


}

AVCodec ff_mdec_decoder = {
    .name             = "mdec",
    .long_name        = NULL_IF_CONFIG_SMALL("Sony PlayStation MDEC (Motion DECoder)"),
    .type             = AVMEDIA_TYPE_VIDEO,
    .id               = AV_CODEC_ID_MDEC,
    .priv_data_size   = sizeof(MDECContext),
    .init             = decode_init,
    .close            = decode_end,
    .decode           = decode_frame,
    .capabilities     = AV_CODEC_CAP_DR1 | AV_CODEC_CAP_FRAME_THREADS,
};
