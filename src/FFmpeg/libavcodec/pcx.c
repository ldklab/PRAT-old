/*
 * PC Paintbrush PCX (.pcx) image decoder
 * Copyright (c) 2007, 2008 Ivo van Poorten
 *
 * This decoder does not support CGA palettes. I am unable to find samples
 * and Netpbm cannot generate them.
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

#include "libavutil/imgutils.h"
#include "avcodec.h"
#include "bytestream.h"
#include "get_bits.h"
#include "internal.h"

#define PCX_HEADER_SIZE 128

                           uint8_t *dst,
                           unsigned int bytes_per_scanline,
                           int compressed)
{

        return AVERROR_INVALIDDATA;

            }
        }
    } else {
        bytestream2_get_buffer(gb, dst, bytes_per_scanline);
    }
    return 0;
}

static void pcx_palette(GetByteContext *gb, uint32_t *dst, int pallen)
{
    int i;

    pallen = FFMIN(pallen, bytestream2_get_bytes_left(gb) / 3);
    for (i = 0; i < pallen; i++)
        *dst++ = 0xFF000000 | bytestream2_get_be24u(gb);
    if (pallen < 256)
        memset(dst, 0, (256 - pallen) * sizeof(*dst));
}

                            AVPacket *avpkt)
{
                 bytes_per_scanline;

        av_log(avctx, AV_LOG_ERROR, "Packet too small\n");
        return AVERROR_INVALIDDATA;
    }


        av_log(avctx, AV_LOG_ERROR, "this is not PCX encoded data\n");
        return AVERROR_INVALIDDATA;
    }


        av_log(avctx, AV_LOG_ERROR, "invalid image dimensions\n");
        return AVERROR_INVALIDDATA;
    }



        (!compressed && bytes_per_scanline > bytestream2_get_bytes_left(&gb) / h)) {
        av_log(avctx, AV_LOG_ERROR, "PCX data is corrupted\n");
        return AVERROR_INVALIDDATA;
    }

    case 0x0108:
    case 0x0104:
    case 0x0102:
    case 0x0101:
    case 0x0401:
    case 0x0301:
    case 0x0201:
        avctx->pix_fmt = AV_PIX_FMT_PAL8;
        break;
    default:
        av_log(avctx, AV_LOG_ERROR, "invalid PCX file\n");
        return AVERROR_INVALIDDATA;
    }


        return ret;

        return ret;



        return AVERROR(ENOMEM);

                goto end;

            }

        }
    } else if (nplanes == 1 && bits_per_pixel == 8) {
        int palstart = avpkt->size - 769;

        if (avpkt->size < 769) {
            av_log(avctx, AV_LOG_ERROR, "File is too short\n");
            ret = avctx->err_recognition & AV_EF_EXPLODE ?
                  AVERROR_INVALIDDATA : avpkt->size;
            goto end;
        }

        for (y = 0; y < h; y++, ptr += stride) {
            ret = pcx_rle_decode(&gb, scanline, bytes_per_scanline, compressed);
            if (ret < 0)
                goto end;
            memcpy(ptr, scanline, w);
        }

        if (bytestream2_tell(&gb) != palstart) {
            av_log(avctx, AV_LOG_WARNING, "image data possibly corrupted\n");
            bytestream2_seek(&gb, palstart, SEEK_SET);
        }
        if (bytestream2_get_byte(&gb) != 12) {
            av_log(avctx, AV_LOG_ERROR, "expected palette after image data\n");
            ret = avctx->err_recognition & AV_EF_EXPLODE ?
                  AVERROR_INVALIDDATA : avpkt->size;
            goto end;
        }
    } else if (nplanes == 1) {   /* all packed formats, max. 16 colors */
        GetBitContext s;

        for (y = 0; y < h; y++) {
            init_get_bits8(&s, scanline, bytes_per_scanline);

            ret = pcx_rle_decode(&gb, scanline, bytes_per_scanline, compressed);
            if (ret < 0)
                goto end;

            for (x = 0; x < w; x++)
                ptr[x] = get_bits(&s, bits_per_pixel);
            ptr += stride;
        }
    } else {    /* planar, 4, 8 or 16 colors */
        int i;

        for (y = 0; y < h; y++) {
            ret = pcx_rle_decode(&gb, scanline, bytes_per_scanline, compressed);
            if (ret < 0)
                goto end;

            for (x = 0; x < w; x++) {
                int m = 0x80 >> (x & 7), v = 0;
                for (i = nplanes - 1; i >= 0; i--) {
                    v <<= 1;
                    v  += !!(scanline[i * bytes_per_line + (x >> 3)] & m);
                }
                ptr[x] = v;
            }
            ptr += stride;
        }
    }

        pcx_palette(&gb, (uint32_t *)p->data[1], 256);
        ret += 256 * 3;
        AV_WN32A(p->data[1]  , 0xFF000000);
        AV_WN32A(p->data[1]+4, 0xFFFFFFFF);
        bytestream2_seek(&gb, 16, SEEK_SET);
        pcx_palette(&gb, (uint32_t *)p->data[1], 16);
    }


}

AVCodec ff_pcx_decoder = {
    .name         = "pcx",
    .long_name    = NULL_IF_CONFIG_SMALL("PC Paintbrush PCX image"),
    .type         = AVMEDIA_TYPE_VIDEO,
    .id           = AV_CODEC_ID_PCX,
    .decode       = pcx_decode_frame,
    .capabilities = AV_CODEC_CAP_DR1,
};
