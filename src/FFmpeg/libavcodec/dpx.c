/*
 * DPX (.dpx) image decoder
 * Copyright (c) 2009 Jimmy Christensen
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

#include "libavutil/avstring.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/intfloat.h"
#include "libavutil/imgutils.h"
#include "bytestream.h"
#include "avcodec.h"
#include "internal.h"

{
    } else {
    }
}

{
    } else {
    }
}

static uint16_t read10in32_gray(const uint8_t **ptr, uint32_t *lbuf,
                                int *n_datum, int is_big, int shift)
{
    uint16_t temp;

    if (*n_datum)
        (*n_datum)--;
    else {
        *lbuf = read32(ptr, is_big);
        *n_datum = 2;
    }

    temp = *lbuf >> shift & 0x3FF;
    *lbuf = *lbuf >> 10;

    return temp;
}

                           int *n_datum, int is_big, int shift)
{
    else {
    }


}

static uint16_t read12in32(const uint8_t **ptr, uint32_t *lbuf,
                           int *n_datum, int is_big)
{
    if (*n_datum)
        (*n_datum)--;
    else {
        *lbuf = read32(ptr, is_big);
        *n_datum = 7;
    }

    switch (*n_datum){
    case 7: return *lbuf & 0xFFF;
    case 6: return (*lbuf >> 12) & 0xFFF;
    case 5: {
            uint32_t c = *lbuf >> 24;
            *lbuf = read32(ptr, is_big);
            c |= *lbuf << 8;
            return c & 0xFFF;
            }
    case 4: return (*lbuf >> 4) & 0xFFF;
    case 3: return (*lbuf >> 16) & 0xFFF;
    case 2: {
            uint32_t c = *lbuf >> 28;
            *lbuf = read32(ptr, is_big);
            c |= *lbuf << 4;
            return c & 0xFFF;
            }
    case 1: return (*lbuf >> 8) & 0xFFF;
    default: return *lbuf >> 20;
    }
}

                        void *data,
                        int *got_frame,
                        AVPacket *avpkt)
{



        av_log(avctx, AV_LOG_ERROR, "Packet too small for DPX header\n");
        return AVERROR_INVALIDDATA;
    }


    /* Check if the files "magic number" is "SDPX" which means it uses
     * big-endian or XPDS which is for little-endian files */
        endian = 0;
        endian = 1;
    } else {
        av_log(avctx, AV_LOG_ERROR, "DPX marker not found\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "Invalid data start offset\n");
        return AVERROR_INVALIDDATA;
    }

        version = 2;
        av_log(avctx, AV_LOG_WARNING, "Unknown header format version %s.\n",
               av_fourcc2str(header_version));

    // Check encryption
        avpriv_report_missing_feature(avctx, "Encryption");
        av_log(avctx, AV_LOG_WARNING, "The image is encrypted and may "
               "not properly decode.\n");
    }

    // Need to end in 0x304 offset from start of file

        return ret;

    // Need to end in 0x320 to read the descriptor

    // Need to end in 0x323 to read the bits per color

        avpriv_report_missing_feature(avctx, "Encoding %d", encoding);
        return AVERROR_PATCHWELCOME;
    }

        av_reduce(&avctx->sample_aspect_ratio.num, &avctx->sample_aspect_ratio.den,
                   avctx->sample_aspect_ratio.num,  avctx->sample_aspect_ratio.den,
                  0x10000);
    else

            AVRational q = av_d2q(av_int2float(i), 4096);
            if (q.num > 0 && q.den > 0)
                avctx->framerate = q;
        }
    }

    case 6:  // Y
        elements = 1;
        break;
    case 51: // RGBA
    case 103: // UYVA4444
    case 102: // UYV444
    case 100: // UYVY422
        elements = 2;
        break;
    default:
        avpriv_report_missing_feature(avctx, "Descriptor %d", descriptor);
        return AVERROR_PATCHWELCOME;
    }

            av_log(avctx, AV_LOG_ERROR, "Packing to 32bit required\n");
            return -1;
        }
        } else {
            stride *= 3;
            if (stride % 8) {
                stride /= 8;
                stride++;
                stride *= 8;
            }
            stride /= 2;
        }
        break;
    case 1:
    case 32:
    case 64:
        avpriv_report_missing_feature(avctx, "Depth %d", bits_per_color);
        return AVERROR_PATCHWELCOME;
    default:
        return AVERROR_INVALIDDATA;
    }

    // Table 3c: Runs will always break at scan line boundaries. Packing
    // will always break to the next 32-bit word at scan-line boundaries.
    // Unfortunately, the encoder produced invalid files, so attempt
    // to detect it
        // Alignment seems unappliable, try without
        if (stride*avctx->height + (int64_t)offset > avpkt->size) {
            av_log(avctx, AV_LOG_ERROR, "Overread buffer. Invalid header?\n");
            return AVERROR_INVALIDDATA;
        } else {
            av_log(avctx, AV_LOG_INFO, "Decoding DPX without scanline "
                   "alignment.\n");
            need_align = 0;
        }
    } else {
    }

    case 6081:
    case 6080:
        avctx->pix_fmt = AV_PIX_FMT_GRAY8;
        break;
    case 6121:
    case 6120:
        avctx->pix_fmt = AV_PIX_FMT_GRAY12;
        break;
    case 50080:
    case 52081:
    case 52080:
        avctx->pix_fmt = AV_PIX_FMT_ABGR;
        break;
    case 51081:
    case 51080:
        avctx->pix_fmt = AV_PIX_FMT_RGBA;
        break;
    case 50101:
    case 51100:
    case 51101:
        avctx->pix_fmt = AV_PIX_FMT_GBRAP10;
        break;
    case 50121:
    case 51120:
    case 51121:
        avctx->pix_fmt = AV_PIX_FMT_GBRAP12;
        break;
    case 6100:
    case 6101:
        avctx->pix_fmt = AV_PIX_FMT_GRAY10;
        break;
    case 6161:
        avctx->pix_fmt = AV_PIX_FMT_GRAY16BE;
        break;
    case 6160:
        avctx->pix_fmt = AV_PIX_FMT_GRAY16LE;
        break;
    case 51161:
        avctx->pix_fmt = AV_PIX_FMT_RGBA64BE;
        break;
    case 100081:
        avctx->pix_fmt = AV_PIX_FMT_UYVY422;
        break;
    case 102081:
        avctx->pix_fmt = AV_PIX_FMT_YUV444P;
        break;
    case 103081:
        avctx->pix_fmt = AV_PIX_FMT_YUVA444P;
        break;
    default:
        av_log(avctx, AV_LOG_ERROR, "Unsupported format\n");
        return AVERROR_PATCHWELCOME;
    }


        return ret;



    // Move pointer to offset from start of file


    case 10:
                                           &n_datum, endian, shift);
                    *dst[0]++ = read10in32_gray(&buf, &rgbBuffer,
                                                &n_datum, endian, shift);
                else
                                           &n_datum, endian, shift);
                                           &n_datum, endian, shift);
                    *dst[3]++ =
                    read10in32(&buf, &rgbBuffer,
                               &n_datum, endian, shift);
            }
        }
        break;
    case 12:
                        *dst[3]++ = read16(&buf, endian) >> shift & 0xFFF;
                } else {
                    if (elements >= 3)
                        *dst[2]++ = read12in32(&buf, &rgbBuffer,
                                               &n_datum, endian);
                    *dst[0]++ = read12in32(&buf, &rgbBuffer,
                                           &n_datum, endian);
                    if (elements >= 2)
                        *dst[1]++ = read12in32(&buf, &rgbBuffer,
                                               &n_datum, endian);
                    if (elements == 4)
                        *dst[3]++ = read12in32(&buf, &rgbBuffer,
                                               &n_datum, endian);
                }
            }
            // Jump to next aligned position
        }
        break;
            for (x = 0; x < avctx->height; x++) {
                ptr[0] = p->data[0] + x * p->linesize[0];
                ptr[1] = p->data[1] + x * p->linesize[1];
                ptr[2] = p->data[2] + x * p->linesize[2];
                ptr[3] = p->data[3] + x * p->linesize[3];
                for (y = 0; y < avctx->width; y++) {
                    *ptr[1]++ = *buf++;
                    *ptr[0]++ = *buf++;
                    *ptr[2]++ = *buf++;
                    if (avctx->pix_fmt == AV_PIX_FMT_YUVA444P)
                        *ptr[3]++ = *buf++;
                }
            }
        } else {
                            buf, stride,
        }
        break;
    }


}

AVCodec ff_dpx_decoder = {
    .name           = "dpx",
    .long_name      = NULL_IF_CONFIG_SMALL("DPX (Digital Picture Exchange) image"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_DPX,
    .decode         = decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
