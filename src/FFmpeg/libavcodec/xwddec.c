/*
 * XWD image format
 *
 * Copyright (c) 2012 Paul B Mahol
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

#include <inttypes.h>

#include "libavutil/imgutils.h"
#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"
#include "xwd.h"

                            int *got_frame, AVPacket *avpkt)
{

        return AVERROR_INVALIDDATA;


        av_log(avctx, AV_LOG_ERROR, "unsupported version\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "invalid header size\n");
        return AVERROR_INVALIDDATA;
    }


        return ret;

           "pixformat %"PRIu32", pixdepth %"PRIu32", bunit %"PRIu32", bitorder %"PRIu32", bpad %"PRIu32"\n",
           pixformat, pixdepth, bunit, bitorder, bpad);
           "vclass %"PRIu32", ncolors %"PRIu32", bpp %"PRIu32", be %"PRIu32", lsize %"PRIu32", xoffset %"PRIu32"\n",
           vclass, ncolors, bpp, be, lsize, xoffset);
           "red %0"PRIx32", green %0"PRIx32", blue %0"PRIx32"\n",
           rgb[0], rgb[1], rgb[2]);

        av_log(avctx, AV_LOG_ERROR, "invalid pixmap format\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "invalid pixmap depth\n");
        return AVERROR_INVALIDDATA;
    }

        avpriv_request_sample(avctx, "xoffset %"PRIu32"", xoffset);
        return AVERROR_PATCHWELCOME;
    }

        av_log(avctx, AV_LOG_ERROR, "invalid byte order\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "invalid bitmap bit order\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "invalid bitmap unit\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "invalid bitmap scan-line pad\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "invalid bits per pixel\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "invalid number of entries in colormap\n");
        return AVERROR_INVALIDDATA;
    }

        return ret;

        av_log(avctx, AV_LOG_ERROR, "invalid bytes per scan-line\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "input buffer too small\n");
        return AVERROR_INVALIDDATA;
    }

        avpriv_report_missing_feature(avctx, "Pixmap format %"PRIu32, pixformat);
        return AVERROR_PATCHWELCOME;
    }

    case XWD_GRAY_SCALE:
            return AVERROR_INVALIDDATA;
        }
        break;
    case XWD_PSEUDO_COLOR:
        break;
    case XWD_DIRECT_COLOR:
            return AVERROR_INVALIDDATA;
            else if (rgb[0] == 0x1F && rgb[1] == 0x3E0 && rgb[2] == 0x7C00)
                avctx->pix_fmt = be ? AV_PIX_FMT_BGR555BE : AV_PIX_FMT_BGR555LE;
            else if (rgb[0] == 0x1F && rgb[1] == 0x7E0 && rgb[2] == 0xF800)
                avctx->pix_fmt = be ? AV_PIX_FMT_BGR565BE : AV_PIX_FMT_BGR565LE;
            else if (rgb[0] == 0xFF && rgb[1] == 0xFF00 && rgb[2] == 0xFF0000)
                avctx->pix_fmt = be ? AV_PIX_FMT_BGR24 : AV_PIX_FMT_RGB24;
                avctx->pix_fmt = be ? AV_PIX_FMT_ARGB : AV_PIX_FMT_BGRA;
        }
        break;
    default:
        av_log(avctx, AV_LOG_ERROR, "invalid visual class\n");
        return AVERROR_INVALIDDATA;
    }

        avpriv_request_sample(avctx,
                              "Unknown file: bpp %"PRIu32", pixdepth %"PRIu32", vclass %"PRIu32"",
                              bpp, pixdepth, vclass);
        return AVERROR_PATCHWELCOME;
    }

        return ret;





        }
    }

    }


}

AVCodec ff_xwd_decoder = {
    .name           = "xwd",
    .long_name      = NULL_IF_CONFIG_SMALL("XWD (X Window Dump) image"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_XWD,
    .decode         = xwd_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
