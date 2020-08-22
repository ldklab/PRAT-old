/*
 * Photoshop (PSD) image decoder
 * Copyright (c) 2016 Jokyo Images
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

#include "bytestream.h"
#include "internal.h"

enum PsdCompr {
    PSD_RAW,
    PSD_RLE,
    PSD_ZIP_WITHOUT_P,
    PSD_ZIP_WITH_P,
};

enum PsdColorMode {
    PSD_BITMAP,
    PSD_GRAYSCALE,
    PSD_INDEXED,
    PSD_RGB,
    PSD_CMYK,
    PSD_MULTICHANNEL,
    PSD_DUOTONE,
    PSD_LAB,
};

typedef struct PSDContext {
    AVClass *class;
    AVFrame *picture;
    AVCodecContext *avctx;
    GetByteContext gb;

    uint8_t * tmp;

    uint16_t channel_count;
    uint16_t channel_depth;

    uint64_t uncompressed_size;
    unsigned int pixel_size;/* 1 for 8 bits, 2 for 16 bits */
    uint64_t line_size;/* length of src data (even width) */

    int width;
    int height;

    enum PsdCompr compression;
    enum PsdColorMode color_mode;

    uint8_t palette[AVPALETTE_SIZE];
} PSDContext;

{

        av_log(s->avctx, AV_LOG_ERROR, "Header too short to parse.\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(s->avctx, AV_LOG_ERROR, "Wrong signature %d.\n", signature);
        return AVERROR_INVALIDDATA;
    }

        av_log(s->avctx, AV_LOG_ERROR, "Wrong version %d.\n", version);
        return AVERROR_INVALIDDATA;
    }


        av_log(s->avctx, AV_LOG_ERROR, "Invalid channel count %d.\n", s->channel_count);
        return AVERROR_INVALIDDATA;
    }


        av_log(s->avctx, AV_LOG_ERROR,
               "Height > 30000 is experimental, add "
               "'-strict %d' if you want to try to decode the picture.\n",
               FF_COMPLIANCE_EXPERIMENTAL);
        return AVERROR_EXPERIMENTAL;
    }

        av_log(s->avctx, AV_LOG_ERROR,
               "Width > 30000 is experimental, add "
               "'-strict %d' if you want to try to decode the picture.\n",
               FF_COMPLIANCE_EXPERIMENTAL);
        return AVERROR_EXPERIMENTAL;
    }

        return ret;


    case 4:
        s->color_mode = PSD_CMYK;
        break;
    case 7:
        s->color_mode = PSD_MULTICHANNEL;
        break;
    case 9:
        s->color_mode = PSD_LAB;
        break;
    default:
        av_log(s->avctx, AV_LOG_ERROR, "Unknown color mode %d.\n", color_mode);
        return AVERROR_INVALIDDATA;
    }

    /* color map data */
        av_log(s->avctx, AV_LOG_ERROR, "Negative size for color map data section.\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(s->avctx, AV_LOG_ERROR, "Incomplete file.\n");
        return AVERROR_INVALIDDATA;
    }
    }

    /* image ressources */
        av_log(s->avctx, AV_LOG_ERROR, "Negative size for image ressources section.\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(s->avctx, AV_LOG_ERROR, "Incomplete file.\n");
        return AVERROR_INVALIDDATA;
    }

    /* layers and masks */
        av_log(s->avctx, AV_LOG_ERROR, "Negative size for layers and masks data section.\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(s->avctx, AV_LOG_ERROR, "Incomplete file.\n");
        return AVERROR_INVALIDDATA;
    }

    /* image section */
        av_log(s->avctx, AV_LOG_ERROR, "File without image data section.\n");
        return AVERROR_INVALIDDATA;
    }

    case 0:
    case 1:
        break;
    case 2:
        avpriv_request_sample(s->avctx, "ZIP without predictor compression");
        return AVERROR_PATCHWELCOME;
    case 3:
        avpriv_request_sample(s->avctx, "ZIP with predictor compression");
        return AVERROR_PATCHWELCOME;
    default:
        av_log(s->avctx, AV_LOG_ERROR, "Unknown compression %d.\n", s->compression);
        return AVERROR_INVALIDDATA;
    }

    return ret;
}



    /* scanline table */
        av_log(s->avctx, AV_LOG_ERROR, "Not enough data for rle scanline table.\n");
        return AVERROR_INVALIDDATA;
    }

    /* decode rle data scanline by scanline */
        count = 0;



                    av_log(s->avctx, AV_LOG_ERROR, "Not enough data for rle scanline.\n");
                    return AVERROR_INVALIDDATA;
                }

                    av_log(s->avctx, AV_LOG_ERROR, "Invalid rle char.\n");
                    return AVERROR_INVALIDDATA;
                }

                }
            } else {
                    av_log(s->avctx, AV_LOG_ERROR, "Not enough data for rle scanline.\n");
                    return AVERROR_INVALIDDATA;
                }

                    av_log(s->avctx, AV_LOG_ERROR, "Invalid rle char.\n");
                    return AVERROR_INVALIDDATA;
                }

                }
            }
        }
    }

    return 0;
}

                        int *got_frame, AVPacket *avpkt)
{




        return ret;


            av_log(s->avctx, AV_LOG_ERROR,
                    "Invalid bitmap file (channel_depth %d, channel_count %d)\n",
                    s->channel_depth, s->channel_count);
            return AVERROR_INVALIDDATA;
        }
            av_log(s->avctx, AV_LOG_ERROR,
                   "Invalid indexed file (channel_depth %d, channel_count %d)\n",
                   s->channel_depth, s->channel_count);
            return AVERROR_INVALIDDATA;
        }
    case PSD_CMYK:
        if (s->channel_count == 4) {
            if (s->channel_depth == 8) {
                avctx->pix_fmt = AV_PIX_FMT_GBRP;
            } else if (s->channel_depth == 16) {
                avctx->pix_fmt = AV_PIX_FMT_GBRP16BE;
            } else {
                avpriv_report_missing_feature(avctx, "channel depth %d for cmyk", s->channel_depth);
                return AVERROR_PATCHWELCOME;
            }
        } else if (s->channel_count == 5) {
            if (s->channel_depth == 8) {
                avctx->pix_fmt = AV_PIX_FMT_GBRAP;
            } else if (s->channel_depth == 16) {
                avctx->pix_fmt = AV_PIX_FMT_GBRAP16BE;
            } else {
                avpriv_report_missing_feature(avctx, "channel depth %d for cmyk", s->channel_depth);
                return AVERROR_PATCHWELCOME;
            }
        } else {
            avpriv_report_missing_feature(avctx, "channel count %d for cmyk", s->channel_count);
            return AVERROR_PATCHWELCOME;
        }
        break;
            } else {
                avpriv_report_missing_feature(avctx, "channel depth %d for rgb", s->channel_depth);
                return AVERROR_PATCHWELCOME;
            }
            } else {
                avpriv_report_missing_feature(avctx, "channel depth %d for rgb", s->channel_depth);
                return AVERROR_PATCHWELCOME;
            }
        } else {
            avpriv_report_missing_feature(avctx, "channel count %d for rgb", s->channel_count);
            return AVERROR_PATCHWELCOME;
        }
        break;
            } else if (s->channel_depth == 32) {
                avctx->pix_fmt = AV_PIX_FMT_GRAYF32BE;
            } else {
                avpriv_report_missing_feature(avctx, "channel depth %d for grayscale", s->channel_depth);
                return AVERROR_PATCHWELCOME;
            }
            } else {
                avpriv_report_missing_feature(avctx, "channel depth %d for grayscale", s->channel_depth);
                return AVERROR_PATCHWELCOME;
            }
        } else {
            avpriv_report_missing_feature(avctx, "channel count %d for grayscale", s->channel_count);
            return AVERROR_PATCHWELCOME;
        }
        break;
    default:
        avpriv_report_missing_feature(avctx, "color mode %d", s->color_mode);
        return AVERROR_PATCHWELCOME;
    }


        return ret;

    /* decode picture if need */
            return AVERROR(ENOMEM);


            av_freep(&s->tmp);
            return ret;
        }

    } else {
            av_log(s->avctx, AV_LOG_ERROR, "Not enough data for raw image data section.\n");
            return AVERROR_INVALIDDATA;
        }
        ptr_data = s->gb.buffer;
    }

    /* Store data */
                    }
                }
            }
        }
        uint8_t *dst[4] = { picture->data[0], picture->data[1], picture->data[2], picture->data[3] };
        const uint8_t *src[5] = { ptr_data };
        src[1] = src[0] + s->line_size * s->height;
        src[2] = src[1] + s->line_size * s->height;
        src[3] = src[2] + s->line_size * s->height;
        src[4] = src[3] + s->line_size * s->height;
        if (s->channel_depth == 8) {
            for (y = 0; y < s->height; y++) {
                for (x = 0; x < s->width; x++) {
                    int k = src[3][x];
                    int r = src[0][x] * k;
                    int g = src[1][x] * k;
                    int b = src[2][x] * k;
                    dst[0][x] = g * 257 >> 16;
                    dst[1][x] = b * 257 >> 16;
                    dst[2][x] = r * 257 >> 16;
                }
                dst[0] += picture->linesize[0];
                dst[1] += picture->linesize[1];
                dst[2] += picture->linesize[2];
                src[0] += s->line_size;
                src[1] += s->line_size;
                src[2] += s->line_size;
                src[3] += s->line_size;
            }
            if (avctx->pix_fmt == AV_PIX_FMT_GBRAP) {
                for (y = 0; y < s->height; y++) {
                    memcpy(dst[3], src[4], s->line_size);
                    src[4] += s->line_size;
                    dst[3] += picture->linesize[3];
                }
            }
        } else {
            for (y = 0; y < s->height; y++) {
                for (x = 0; x < s->width; x++) {
                    int64_t k = AV_RB16(&src[3][x * 2]);
                    int64_t r = AV_RB16(&src[0][x * 2]) * k;
                    int64_t g = AV_RB16(&src[1][x * 2]) * k;
                    int64_t b = AV_RB16(&src[2][x * 2]) * k;
                    AV_WB16(&dst[0][x * 2], g * 65537 >> 32);
                    AV_WB16(&dst[1][x * 2], b * 65537 >> 32);
                    AV_WB16(&dst[2][x * 2], r * 65537 >> 32);
                }
                dst[0] += picture->linesize[0];
                dst[1] += picture->linesize[1];
                dst[2] += picture->linesize[2];
                src[0] += s->line_size;
                src[1] += s->line_size;
                src[2] += s->line_size;
                src[3] += s->line_size;
            }
            if (avctx->pix_fmt == AV_PIX_FMT_GBRAP16BE) {
                for (y = 0; y < s->height; y++) {
                    memcpy(dst[3], src[4], s->line_size);
                    src[4] += s->line_size;
                    dst[3] += picture->linesize[3];
                }
            }
        }
    } else {/* Planar */

            }
        }
    }

    }



}

AVCodec ff_psd_decoder = {
    .name             = "psd",
    .long_name        = NULL_IF_CONFIG_SMALL("Photoshop PSD file"),
    .type             = AVMEDIA_TYPE_VIDEO,
    .id               = AV_CODEC_ID_PSD,
    .priv_data_size   = sizeof(PSDContext),
    .decode           = decode_frame,
    .capabilities     = AV_CODEC_CAP_DR1 | AV_CODEC_CAP_FRAME_THREADS,
};
