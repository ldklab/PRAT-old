/*
 * Sun Rasterfile (.sun/.ras/im{1,8,24}/.sunras) image decoder
 * Copyright (c) 2007, 2008 Ivo van Poorten
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

#include "libavutil/common.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/imgutils.h"
#include "avcodec.h"
#include "internal.h"
#include "sunrast.h"

                                int *got_frame, AVPacket *avpkt)
{

        return AVERROR_INVALIDDATA;

        av_log(avctx, AV_LOG_ERROR, "this is not sunras encoded data\n");
        return AVERROR_INVALIDDATA;
    }


        avpriv_request_sample(avctx, "TIFF/IFF/EXPERIMENTAL (compression) type");
        return AVERROR_PATCHWELCOME;
    }
        av_log(avctx, AV_LOG_ERROR, "invalid (compression) type\n");
        return AVERROR_INVALIDDATA;
    }
        avpriv_request_sample(avctx, "Unknown colormap type");
        return AVERROR_PATCHWELCOME;
    }
        av_log(avctx, AV_LOG_ERROR, "invalid colormap type\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "unsupported (compression) type\n");
        return AVERROR_PATCHWELCOME;
    }

        case 4:
            avctx->pix_fmt = maplength ? AV_PIX_FMT_PAL8 : AV_PIX_FMT_NONE;
            break;
        case 32:
            avctx->pix_fmt = (type == RT_FORMAT_RGB) ? AV_PIX_FMT_0RGB : AV_PIX_FMT_0BGR;
            break;
        default:
            av_log(avctx, AV_LOG_ERROR, "invalid depth\n");
            return AVERROR_INVALIDDATA;
    }

        return ret;

    /* scanlines are aligned on 16 bit boundaries */

        return AVERROR_INVALIDDATA;

        return ret;


        av_log(avctx, AV_LOG_WARNING, "useless colormap found or file is corrupted, trying to recover\n");


            av_log(avctx, AV_LOG_WARNING, "invalid colormap length\n");
            return AVERROR_INVALIDDATA;
        }

    }


        ptr = ptr2 = av_malloc_array((w + 15), h);
        if (!ptr)
            return AVERROR(ENOMEM);
        stride = (w + 15 >> 3) * depth;
    } else {
    }


                return AVERROR_INVALIDDATA;

            }
                        break;
                }
            }
        }
    } else {
                break;
        }
    }
        uint8_t *ptr_free = ptr2;
        ptr = p->data[0];
        for (y=0; y<h; y++) {
            for (x = 0; x < (w + 7 >> 3) * depth; x++) {
                if (depth == 1) {
                    ptr[8*x]   = ptr2[x] >> 7;
                    ptr[8*x+1] = ptr2[x] >> 6 & 1;
                    ptr[8*x+2] = ptr2[x] >> 5 & 1;
                    ptr[8*x+3] = ptr2[x] >> 4 & 1;
                    ptr[8*x+4] = ptr2[x] >> 3 & 1;
                    ptr[8*x+5] = ptr2[x] >> 2 & 1;
                    ptr[8*x+6] = ptr2[x] >> 1 & 1;
                    ptr[8*x+7] = ptr2[x]      & 1;
                } else {
                    ptr[2*x]   = ptr2[x] >> 4;
                    ptr[2*x+1] = ptr2[x] & 0xF;
                }
            }
            ptr  += p->linesize[0];
            ptr2 += (w + 15 >> 3) * depth;
        }
        av_freep(&ptr_free);
    }


}

AVCodec ff_sunrast_decoder = {
    .name           = "sunrast",
    .long_name      = NULL_IF_CONFIG_SMALL("Sun Rasterfile image"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_SUNRAST,
    .decode         = sunrast_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
