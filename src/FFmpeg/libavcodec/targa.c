/*
 * Targa (.tga) image decoder
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

#include "libavutil/intreadwrite.h"
#include "libavutil/imgutils.h"
#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"
#include "targa.h"

typedef struct TargaContext {
    GetByteContext gb;
} TargaContext;

                             int stride, int *y, int h, int interleave)
{

    } else {
            return start + *y * stride;
        } else {
            return NULL;
        }
    }
}

                            uint8_t *start, int w, int h, int stride,
                            int bpp, int interleave)
{

            av_log(avctx, AV_LOG_ERROR,
                   "Ran ouf of data before end-of-image\n");
            return AVERROR_INVALIDDATA;
        }
                }
        } else {
                }
        }
    }

        av_log(avctx, AV_LOG_ERROR, "Packet went out of bounds\n");
        return AVERROR_INVALIDDATA;
    }

    return 0;
}

                        void *data, int *got_frame,
                        AVPacket *avpkt)
{


    /* parse image header */


        av_log(avctx, AV_LOG_WARNING, "File without colormap has colormap information set.\n");
        // specification says we should ignore those value in this case
        first_clr = colors = csize = 0;
    }

        av_log(avctx, AV_LOG_ERROR,
                "Not enough data to read header\n");
        return AVERROR_INVALIDDATA;
    }

    // skip identifier if any

    case 16:
    default:
        av_log(avctx, AV_LOG_ERROR, "Bit depth %i is not supported\n", bpp);
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "Incorrect palette: %i colors with offset %i\n", colors, first_clr);
        return AVERROR_INVALIDDATA;
    }

        return ret;

        return ret;

    } else { //image is upside-down
    }



        case 32: pal_sample_size = 4; break;
        case 24: pal_sample_size = 3; break;
        default:
            av_log(avctx, AV_LOG_ERROR, "Palette entry size %i bits is not supported\n", csize);
            return AVERROR_INVALIDDATA;
        }
            bytestream2_skip(&s->gb, pal_size);
        else {

                av_log(avctx, AV_LOG_ERROR,
                       "Not enough data to read palette\n");
                return AVERROR_INVALIDDATA;
            }
            case 4:
                for (t = 0; t < colors; t++)
                    *pal++ = bytestream2_get_le32u(&s->gb);
                break;
            case 3:
                /* RGB24 */
                for (t = 0; t < colors; t++)
                    *pal++ = (0xffU<<24) | bytestream2_get_le24u(&s->gb);
                break;
            case 2:
                /* RGB555 */
                    /* left bit replication */
                }
                break;
            }
        }
    }

        memset(p->data[0], 0, p->linesize[0] * h);
    } else {
                return res;
        } else {
                av_log(avctx, AV_LOG_ERROR,
                       "Not enough data available for image\n");
                return AVERROR_INVALIDDATA;
            }

            line = dst;
            y = 0;
        }

            int x;
            for (y = 0; y < h; y++) {
                void *line = &p->data[0][y * p->linesize[0]];
                for (x = 0; x < w >> 1; x++) {
                    switch (bpp) {
                    case 32:
                        FFSWAP(uint32_t, ((uint32_t *)line)[x], ((uint32_t *)line)[w - x - 1]);
                        break;
                    case 24:
                        FFSWAP(uint8_t, ((uint8_t *)line)[3 * x    ], ((uint8_t *)line)[3 * w - 3 * x - 3]);
                        FFSWAP(uint8_t, ((uint8_t *)line)[3 * x + 1], ((uint8_t *)line)[3 * w - 3 * x - 2]);
                        FFSWAP(uint8_t, ((uint8_t *)line)[3 * x + 2], ((uint8_t *)line)[3 * w - 3 * x - 1]);
                        break;
                    case 16:
                        FFSWAP(uint16_t, ((uint16_t *)line)[x], ((uint16_t *)line)[w - x - 1]);
                        break;
                    case 8:
                        FFSWAP(uint8_t, ((uint8_t *)line)[x], ((uint8_t *)line)[w - x - 1]);
                    }
                }
            }
        }
    }



}

AVCodec ff_targa_decoder = {
    .name           = "targa",
    .long_name      = NULL_IF_CONFIG_SMALL("Truevision Targa image"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_TARGA,
    .priv_data_size = sizeof(TargaContext),
    .decode         = decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
