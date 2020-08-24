/*
 * DV decoder
 * Copyright (c) 2002 Fabrice Bellard
 * Copyright (c) 2004 Roman Shaposhnik
 *
 * 50 Mbps (DVCPRO50) support
 * Copyright (c) 2006 Daniel Maas <dmaas@maasdigital.com>
 *
 * 100 Mbps (DVCPRO HD) support
 * Initial code by Daniel Maas <dmaas@maasdigital.com> (funded by BBC R&D)
 * Final code by Roman Shaposhnik
 *
 * Many thanks to Dan Dennedy <dan@dennedy.org> for providing wealth
 * of DV technical info.
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
 * DV decoder
 */

#include "libavutil/avassert.h"
#include "libavutil/imgutils.h"
#include "libavutil/internal.h"
#include "libavutil/pixdesc.h"

#include "avcodec.h"
#include "dv.h"
#include "dv_profile_internal.h"
#include "dvdata.h"
#include "get_bits.h"
#include "internal.h"
#include "put_bits.h"
#include "simple_idct.h"
#include "thread.h"

typedef struct BlockInfo {
    const uint32_t *factor_table;
    const uint8_t *scan_table;
    uint8_t pos; /* position in block */
    void (*idct_put)(uint8_t *dest, ptrdiff_t stride, int16_t *block);
    uint8_t partial_bit_count;
    uint32_t partial_bit_buffer;
    int shift_offset;
} BlockInfo;

static const int dv_iweight_bits = 14;

static const uint16_t dv_iweight_88[64] = {
    32768, 16705, 16705, 17734, 17032, 17734, 18205, 18081,
    18081, 18205, 18725, 18562, 19195, 18562, 18725, 19266,
    19091, 19705, 19705, 19091, 19266, 21407, 19643, 20267,
    20228, 20267, 19643, 21407, 22725, 21826, 20853, 20806,
    20806, 20853, 21826, 22725, 23170, 23170, 21407, 21400,
    21407, 23170, 23170, 24598, 23786, 22018, 22018, 23786,
    24598, 25251, 24465, 22654, 24465, 25251, 25972, 25172,
    25172, 25972, 26722, 27969, 26722, 29692, 29692, 31521,
};
static const uint16_t dv_iweight_248[64] = {
    32768, 16384, 16705, 16705, 17734, 17734, 17734, 17734,
    18081, 18081, 18725, 18725, 21407, 21407, 19091, 19091,
    19195, 19195, 18205, 18205, 18725, 18725, 19705, 19705,
    20267, 20267, 21826, 21826, 23170, 23170, 20806, 20806,
    20267, 20267, 19266, 19266, 21407, 21407, 20853, 20853,
    21400, 21400, 23786, 23786, 24465, 24465, 22018, 22018,
    23170, 23170, 22725, 22725, 24598, 24598, 24465, 24465,
    25172, 25172, 27969, 27969, 25972, 25972, 29692, 29692
};

/**
 * The "inverse" DV100 weights are actually just the spec weights (zig-zagged).
 */
static const uint16_t dv_iweight_1080_y[64] = {
    128,  16,  16,  17,  17,  17,  18,  18,
     18,  18,  18,  18,  19,  18,  18,  19,
     19,  19,  19,  19,  19,  42,  38,  40,
     40,  40,  38,  42,  44,  43,  41,  41,
     41,  41,  43,  44,  45,  45,  42,  42,
     42,  45,  45,  48,  46,  43,  43,  46,
     48,  49,  48,  44,  48,  49, 101,  98,
     98, 101, 104, 109, 104, 116, 116, 123,
};
static const uint16_t dv_iweight_1080_c[64] = {
    128,  16,  16,  17,  17,  17,  25,  25,
     25,  25,  26,  25,  26,  25,  26,  26,
     26,  27,  27,  26,  26,  42,  38,  40,
     40,  40,  38,  42,  44,  43,  41,  41,
     41,  41,  43,  44,  91,  91,  84,  84,
     84,  91,  91,  96,  93,  86,  86,  93,
     96, 197, 191, 177, 191, 197, 203, 197,
    197, 203, 209, 219, 209, 232, 232, 246,
};
static const uint16_t dv_iweight_720_y[64] = {
    128,  16,  16,  17,  17,  17,  18,  18,
     18,  18,  18,  18,  19,  18,  18,  19,
     19,  19,  19,  19,  19,  42,  38,  40,
     40,  40,  38,  42,  44,  43,  41,  41,
     41,  41,  43,  44,  68,  68,  63,  63,
     63,  68,  68,  96,  92,  86,  86,  92,
     96,  98,  96,  88,  96,  98, 202, 196,
    196, 202, 208, 218, 208, 232, 232, 246,
};
static const uint16_t dv_iweight_720_c[64] = {
    128,  24,  24,  26,  26,  26,  36,  36,
     36,  36,  36,  36,  38,  36,  36,  38,
     38,  38,  38,  38,  38,  84,  76,  80,
     80,  80,  76,  84,  88,  86,  82,  82,
     82,  82,  86,  88, 182, 182, 168, 168,
    168, 182, 182, 192, 186, 192, 172, 186,
    192, 394, 382, 354, 382, 394, 406, 394,
    394, 406, 418, 438, 418, 464, 464, 492,
};

static void dv_init_weight_tables(DVVideoContext *ctx, const AVDVProfile *d)
{
    int j, i, c, s;
    uint32_t *factor1 = &ctx->idct_factor[0],
             *factor2 = &ctx->idct_factor[DV_PROFILE_IS_HD(d) ? 4096 : 2816];

    if (DV_PROFILE_IS_HD(d)) {
        /* quantization quanta by QNO for DV100 */
        static const uint8_t dv100_qstep[16] = {
            1, /* QNO = 0 and 1 both have no quantization */
            1,
            2, 3, 4, 5, 6, 7, 8, 16, 18, 20, 22, 24, 28, 52
        };
        const uint16_t *iweight1, *iweight2;

        if (d->height == 720) {
            iweight1 = &dv_iweight_720_y[0];
            iweight2 = &dv_iweight_720_c[0];
        } else {
            iweight1 = &dv_iweight_1080_y[0];
            iweight2 = &dv_iweight_1080_c[0];
        }
        for (c = 0; c < 4; c++) {
            for (s = 0; s < 16; s++) {
                for (i = 0; i < 64; i++) {
                    *factor1++ = (dv100_qstep[s] << (c + 9)) * iweight1[i];
                    *factor2++ = (dv100_qstep[s] << (c + 9)) * iweight2[i];
                }
            }
        }
    } else {
        static const uint8_t dv_quant_areas[4] = { 6, 21, 43, 64 };
        const uint16_t *iweight1 = &dv_iweight_88[0];
        for (j = 0; j < 2; j++, iweight1 = &dv_iweight_248[0]) {
            for (s = 0; s < 22; s++) {
                for (i = c = 0; c < 4; c++) {
                    for (; i < dv_quant_areas[c]; i++) {
                        *factor1   = iweight1[i] << (ff_dv_quant_shifts[s][c] + 1);
                        *factor2++ = (*factor1++) << 1;
                    }
                }
            }
        }
    }
}

{



        for (i = 0; i < 64; i++){
            int j = ff_dv_zigzag248_direct[i];
            s->dv_zigzag[1][i] = s->idsp.idct_permutation[(j & 7) + (j & 8) * 4 + (j & 48) / 2];
        }
    }else


}

/* decode AC coefficients */
{


    /* if we must parse a partial VLC, we do it here */
    }

    /* get the AC coefficients until last_index is reached */
                pos, SHOW_UBITS(re, gb, 16), re_index);
        /* our own optimized GET_RL_VLC */
        }

        /* gotta check if we're still within gb boundaries */
            /* should be < 16 bits otherwise a codeword could have been parsed */
        }

            break;

                dv_iweight_bits;

    }

{
    }

static av_always_inline void put_block_8x4(int16_t *block, uint8_t *av_restrict p, int stride)
{
    int i, j;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 8; j++)
            p[j] = av_clip_uint8(block[j]);
        block += 8;
        p += stride;
    }
}

static void dv100_idct_put_last_row_field_chroma(DVVideoContext *s, uint8_t *data,
                                                 int stride, int16_t *blocks)
{
    s->idsp.idct(blocks + 0*64);
    s->idsp.idct(blocks + 1*64);

    put_block_8x4(blocks+0*64,       data,              stride<<1);
    put_block_8x4(blocks+0*64 + 4*8, data + 8,          stride<<1);
    put_block_8x4(blocks+1*64,       data + stride,     stride<<1);
    put_block_8x4(blocks+1*64 + 4*8, data + 8 + stride, stride<<1);
}

static void dv100_idct_put_last_row_field_luma(DVVideoContext *s, uint8_t *data,
                                               int stride, int16_t *blocks)
{
    s->idsp.idct(blocks + 0*64);
    s->idsp.idct(blocks + 1*64);
    s->idsp.idct(blocks + 2*64);
    s->idsp.idct(blocks + 3*64);

    put_block_8x4(blocks+0*64,       data,               stride<<1);
    put_block_8x4(blocks+0*64 + 4*8, data + 16,          stride<<1);
    put_block_8x4(blocks+1*64,       data + 8,           stride<<1);
    put_block_8x4(blocks+1*64 + 4*8, data + 24,          stride<<1);
    put_block_8x4(blocks+2*64,       data + stride,      stride<<1);
    put_block_8x4(blocks+2*64 + 4*8, data + 16 + stride, stride<<1);
    put_block_8x4(blocks+3*64,       data + 8  + stride, stride<<1);
    put_block_8x4(blocks+3*64 + 4*8, data + 24 + stride, stride<<1);
}

/* mb_x and mb_y are in units of 8 pixels */
{




    /* pass 1: read DC and AC coefficients in blocks */
        /* skip header */
                vs_bit_buffer_damaged = 1;
                vs_bit_buffer_damaged = 1;
        }

            /* get the DC */
            } else {
            }
            /* convert to unsigned because 128 is not added in the
             * standard IDCT */


            /* write the remaining bits in a new buffer only if the
             * block is finished */
                vs_bit_buffer_damaged = mb_bit_buffer_damaged[mb_index] = 1;

        }


        /* pass 2: we can do it just after */
                /* if still not finished, no need to parse other blocks */
                    break;
            }
        }
        /* all blocks are finished, so the extra bytes can be used at
         * the video segment level */
    }

    /* we need a pass over the whole video segment */
            }

                       "AC EOB marker is absent pos=%d\n", mb->pos);
            }
        }
    }
    }

    /* compute idct and place blocks */
    block = &sblock[0][0];
    mb    = mb_data;

        /* idct_put'ting luminance */
        } else {
        }
            dv100_idct_put_last_row_field_luma(s, y_ptr, s->frame->linesize[0], block);
        } else {
            } else {
            }
        }

        /* idct_put'ting chrominance */
                    }
                }
            } else {
                    dv100_idct_put_last_row_field_chroma(s, c_ptr, s->frame->linesize[j], block);
                    mb += 2;
                    block += 2*64;
                } else {
                    }
                }
            }
        }
    }
}

/* NOTE: exactly one frame must be given (120000 bytes for NTSC,
 * 144000 bytes for PAL - or twice those for 50Mbps) */
                                int *got_frame, AVPacket *avpkt)
{

        av_log(avctx, AV_LOG_ERROR, "could not find dv frame profile\n");
        return -1; /* NOTE: we only accept several full frames */
    }

            av_log(avctx, AV_LOG_ERROR, "Error initializing the work tables.\n");
            return ret;
        }
    }


        return ret;

    /* Determine the codec's sample_aspect ratio from the packet */
    }

        return ret;

    /* Determine the codec's field order from the packet */
        } else {
        }
    }

                   dv_work_pool_size(s->sys), sizeof(DVwork_chunk));


    /* return image */

}

AVCodec ff_dvvideo_decoder = {
    .name           = "dvvideo",
    .long_name      = NULL_IF_CONFIG_SMALL("DV (Digital Video)"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_DVVIDEO,
    .priv_data_size = sizeof(DVVideoContext),
    .init           = dvvideo_decode_init,
    .decode         = dvvideo_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1 | AV_CODEC_CAP_FRAME_THREADS | AV_CODEC_CAP_SLICE_THREADS,
    .max_lowres     = 3,
};
