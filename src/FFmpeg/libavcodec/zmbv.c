/*
 * Zip Motion Blocks Video (ZMBV) decoder
 * Copyright (c) 2006 Konstantin Shishkov
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
 * Zip Motion Blocks Video decoder
 */

#include <stdio.h>
#include <stdlib.h>

#include "libavutil/common.h"
#include "libavutil/imgutils.h"
#include "libavutil/intreadwrite.h"
#include "avcodec.h"
#include "internal.h"

#include <zlib.h>

#define ZMBV_KEYFRAME 1
#define ZMBV_DELTAPAL 2

enum ZmbvFormat {
    ZMBV_FMT_NONE  = 0,
    ZMBV_FMT_1BPP  = 1,
    ZMBV_FMT_2BPP  = 2,
    ZMBV_FMT_4BPP  = 3,
    ZMBV_FMT_8BPP  = 4,
    ZMBV_FMT_15BPP = 5,
    ZMBV_FMT_16BPP = 6,
    ZMBV_FMT_24BPP = 7,
    ZMBV_FMT_32BPP = 8
};

/*
 * Decoder context
 */
typedef struct ZmbvContext {
    AVCodecContext *avctx;

    int bpp;
    int alloc_bpp;
    unsigned int decomp_size;
    uint8_t* decomp_buf;
    uint8_t pal[768];
    uint8_t *prev, *cur;
    int width, height;
    int fmt;
    int comp;
    int flags;
    int stride;
    int bw, bh, bx, by;
    int decomp_len;
    int got_keyframe;
    z_stream zstream;
    int (*decode_xor)(struct ZmbvContext *c);
} ZmbvContext;

/**
 * Decode XOR'ed frame - 8bpp version
 */

{


        for (i = 0; i < 768; i++)
            c->pal[i] ^= *src++;
    }





            /* copy block - motion vectors out of bounds are used to zero blocks */
                    memset(out, 0, bw2);
                } else {
                    for (i = 0; i < bw2; i++) {
                        if (mx + i < 0 || mx + i >= c->width)
                            out[i] = 0;
                        else
                            out[i] = tprev[i];
                    }
                }
            }

                out = output + x;
                }
            }
        }
    }
               src-c->decomp_buf, c->decomp_len);
}

/**
 * Decode XOR'ed frame - 15bpp and 16bpp version
 */

{






            /* copy block - motion vectors out of bounds are used to zero blocks */
                } else {
                        else
                    }
                }
            }

                out = output + x;
                    }
                }
            }
        }
    }
        av_log(c->avctx, AV_LOG_ERROR, "Used %"PTRDIFF_SPECIFIER" of %i bytes\n",
               src-c->decomp_buf, c->decomp_len);
}

#ifdef ZMBV_ENABLE_24BPP
/**
 * Decode XOR'ed frame - 24bpp version
 */

static int zmbv_decode_xor_24(ZmbvContext *c)
{
    uint8_t *src = c->decomp_buf;
    uint8_t *output, *prev;
    int8_t *mvec;
    int x, y;
    int d, dx, dy, bw2, bh2;
    int block;
    int i, j;
    int mx, my;
    int stride;

    output = c->cur;
    prev = c->prev;

    stride = c->width * 3;
    mvec = (int8_t*)src;
    src += ((c->bx * c->by * 2 + 3) & ~3);

    block = 0;
    for (y = 0; y < c->height; y += c->bh) {
        bh2 = ((c->height - y) > c->bh) ? c->bh : (c->height - y);
        for (x = 0; x < c->width; x += c->bw) {
            uint8_t *out, *tprev;

            d = mvec[block] & 1;
            dx = mvec[block] >> 1;
            dy = mvec[block + 1] >> 1;
            block += 2;

            bw2 = ((c->width - x) > c->bw) ? c->bw : (c->width - x);

            /* copy block - motion vectors out of bounds are used to zero blocks */
            out = output + x * 3;
            tprev = prev + (x + dx) * 3 + dy * stride;
            mx = x + dx;
            my = y + dy;
            for (j = 0; j < bh2; j++) {
                if (my + j < 0 || my + j >= c->height) {
                    memset(out, 0, bw2 * 3);
                } else if (mx >= 0 && mx + bw2 <= c->width){
                    memcpy(out, tprev, 3 * bw2);
                } else {
                    for (i = 0; i < bw2; i++){
                        if (mx + i < 0 || mx + i >= c->width) {
                            out[i * 3 + 0] = 0;
                            out[i * 3 + 1] = 0;
                            out[i * 3 + 2] = 0;
                        } else {
                            out[i * 3 + 0] = tprev[i * 3 + 0];
                            out[i * 3 + 1] = tprev[i * 3 + 1];
                            out[i * 3 + 2] = tprev[i * 3 + 2];
                        }
                    }
                }
                out += stride;
                tprev += stride;
            }

            if (d) { /* apply XOR'ed difference */
                out = output + x * 3;
                for (j = 0; j < bh2; j++) {
                    for (i = 0; i < bw2; i++) {
                        out[i * 3 + 0] ^= *src++;
                        out[i * 3 + 1] ^= *src++;
                        out[i * 3 + 2] ^= *src++;
                    }
                    out += stride;
                }
            }
        }
        output += stride * c->bh;
        prev += stride * c->bh;
    }
    if (src - c->decomp_buf != c->decomp_len)
        av_log(c->avctx, AV_LOG_ERROR, "Used %"PTRDIFF_SPECIFIER" of %i bytes\n",
               src-c->decomp_buf, c->decomp_len);
    return 0;
}
#endif //ZMBV_ENABLE_24BPP

/**
 * Decode XOR'ed frame - 32bpp version
 */

{






            /* copy block - motion vectors out of bounds are used to zero blocks */
                } else {
                        else
                    }
                }
            }

                out = output + x;
                    }
                }
            }
        }
    }
        av_log(c->avctx, AV_LOG_ERROR, "Used %"PTRDIFF_SPECIFIER" of %i bytes\n",
               src-c->decomp_buf, c->decomp_len);
}

/**
 * Decode intraframe
 */
{

    /* make the palette available on the way out */
    }

}

{

    /* parse header */
        return AVERROR_INVALIDDATA;

            return AVERROR_INVALIDDATA;

               "Flags=%X ver=%i.%i comp=%i fmt=%i blk=%ix%i\n",
               c->flags,hi_ver,lo_ver,c->comp,c->fmt,c->bw,c->bh);
            avpriv_request_sample(avctx, "Version %i.%i", hi_ver, lo_ver);
            return AVERROR_PATCHWELCOME;
        }
            avpriv_request_sample(avctx, "Block size %ix%i", c->bw, c->bh);
            return AVERROR_PATCHWELCOME;
        }
            avpriv_request_sample(avctx, "Compression type %i", c->comp);
            return AVERROR_PATCHWELCOME;
        }

        case ZMBV_FMT_16BPP:
            else
#ifdef ZMBV_ENABLE_24BPP
        case ZMBV_FMT_24BPP:
            c->bpp = 24;
            c->decode_xor = zmbv_decode_xor_24;
            avctx->pix_fmt = AV_PIX_FMT_BGR24;
            c->stride = c->width * 3;
            break;
#endif //ZMBV_ENABLE_24BPP
        default:
            c->decode_xor = NULL;
            avpriv_request_sample(avctx, "Format %i", c->fmt);
            return AVERROR_PATCHWELCOME;
        }

            av_log(avctx, AV_LOG_ERROR, "Inflate reset error: %d\n", zret);
            return AVERROR_UNKNOWN;
        }

        }
            c->alloc_bpp = 0;
            return AVERROR(ENOMEM);
        }
    }
    } else {
    }

        av_log(avctx, AV_LOG_ERROR, "Error! Got no format or no keyframe!\n");
        return AVERROR_INVALIDDATA;
    }

        if (c->decomp_size < len) {
            av_log(avctx, AV_LOG_ERROR, "Buffer too small\n");
            return AVERROR_INVALIDDATA;
        }
        memcpy(c->decomp_buf, buf, len);
        c->decomp_len = len;
    } else { // ZLIB-compressed data
            av_log(avctx, AV_LOG_ERROR, "inflate error %d\n", zret);
            return AVERROR_INVALIDDATA;
        }
    }
        av_log(avctx, AV_LOG_ERROR, "decompressed size %d is incorrect, expected %d\n", c->decomp_len, expected_size);
        return AVERROR_INVALIDDATA;
    }
        return ret;

    } else {
            return AVERROR_INVALIDDATA;
    }

    /* update frames */
    {

        case ZMBV_FMT_8BPP:
        case ZMBV_FMT_15BPP:
        case ZMBV_FMT_16BPP:
#ifdef ZMBV_ENABLE_24BPP
        case ZMBV_FMT_24BPP:
#endif
        case ZMBV_FMT_32BPP:
                                c->stride, c->height);
        default:
            av_log(avctx, AV_LOG_ERROR, "Cannot handle format %i\n", c->fmt);
        }
    }

    /* always report that the buffer was completely consumed */
}

{




    // Needed if zlib unused or init aborted before inflateInit

        av_log(avctx, AV_LOG_ERROR, "Internal buffer (decomp_size) larger than max_pixels or too large\n");
        return AVERROR_INVALIDDATA;
    }


    /* Allocate decompression buffer */
        av_log(avctx, AV_LOG_ERROR,
                "Can't allocate decompression buffer.\n");
        return AVERROR(ENOMEM);
    }

        av_log(avctx, AV_LOG_ERROR, "Inflate init error: %d\n", zret);
        return AVERROR_UNKNOWN;
    }

    return 0;
}

{



}

AVCodec ff_zmbv_decoder = {
    .name           = "zmbv",
    .long_name      = NULL_IF_CONFIG_SMALL("Zip Motion Blocks Video"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_ZMBV,
    .priv_data_size = sizeof(ZmbvContext),
    .init           = decode_init,
    .close          = decode_end,
    .decode         = decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
    .caps_internal  = FF_CODEC_CAP_INIT_CLEANUP,
};
