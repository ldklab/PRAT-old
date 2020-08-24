/*
 * VMware Screen Codec (VMnc) decoder
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
 * VMware Screen Codec (VMnc) decoder
 * As Alex Beregszaszi discovered, this is effectively RFB data dump
 */

#include <stdio.h>
#include <stdlib.h>

#include "libavutil/common.h"
#include "libavutil/intreadwrite.h"
#include "avcodec.h"
#include "internal.h"
#include "bytestream.h"

enum EncTypes {
    MAGIC_WMVd = 0x574D5664,
    MAGIC_WMVe,
    MAGIC_WMVf,
    MAGIC_WMVg,
    MAGIC_WMVh,
    MAGIC_WMVi,
    MAGIC_WMVj
};

enum HexTile_Flags {
    HT_RAW =  1, // tile is raw
    HT_BKG =  2, // background color is present
    HT_FG  =  4, // foreground color is present
    HT_SUB =  8, // subrects are present
    HT_CLR = 16  // each subrect has own color
};

/*
 * Decoder context
 */
typedef struct VmncContext {
    AVCodecContext *avctx;
    AVFrame *pic;

    int bpp;
    int bpp2;
    int bigendian;
    uint8_t pal[768];
    int width, height;
    GetByteContext gb;

    /* cursor data */
    int cur_w, cur_h;
    int cur_x, cur_y;
    int cur_hx, cur_hy;
    uint8_t *curbits, *curmask;
    uint8_t *screendta;
} VmncContext;

/* read pixel value from stream */
{
    case 2:
    case 3:
        return bytestream2_get_byte(gb);
    case 4:
    case 5:
        return bytestream2_get_be16(gb);
    case 8:
    case 9:
        return bytestream2_get_be32(gb);
    default: return 0;
    }
}

{

                *dst8++ = p;
                *dst16++ = p;
        }
    }
                *dst8++ = p;
                *dst16++ = p;
        }
    }

static void put_cursor(uint8_t *dst, int stride, VmncContext *c, int dx, int dy)
{
    int i, j;
    int w, h, x, y;
    w = c->cur_w;
    if (c->width < c->cur_x + c->cur_w)
        w = c->width - c->cur_x;
    h = c->cur_h;
    if (c->height < c->cur_y + c->cur_h)
        h = c->height - c->cur_y;
    x = c->cur_x;
    y = c->cur_y;
    if (x < 0) {
        w += x;
        x  = 0;
    }
    if (y < 0) {
        h += y;
        y  = 0;
    }

    if ((w < 1) || (h < 1))
        return;
    dst += x * c->bpp2 + y * stride;

    if (c->bpp2 == 1) {
        uint8_t *cd = c->curbits, *msk = c->curmask;
        for (j = 0; j < h; j++) {
            for (i = 0; i < w; i++)
                dst[i] = (dst[i] & cd[i]) ^ msk[i];
            msk += c->cur_w;
            cd  += c->cur_w;
            dst += stride;
        }
    } else if (c->bpp2 == 2) {
        uint16_t *cd = (uint16_t*)c->curbits, *msk = (uint16_t*)c->curmask;
        uint16_t *dst2;
        for (j = 0; j < h; j++) {
            dst2 = (uint16_t*)dst;
            for (i = 0; i < w; i++)
                dst2[i] = (dst2[i] & cd[i]) ^ msk[i];
            msk += c->cur_w;
            cd  += c->cur_w;
            dst += stride;
        }
    } else if (c->bpp2 == 4) {
        uint32_t *cd = (uint32_t*)c->curbits, *msk = (uint32_t*)c->curmask;
        uint32_t *dst2;
        for (j = 0; j < h; j++) {
            dst2 = (uint32_t*)dst;
            for (i = 0; i < w; i++)
                dst2[i] = (dst2[i] & cd[i]) ^ msk[i];
            msk += c->cur_w;
            cd  += c->cur_w;
            dst += stride;
        }
    }
}

/* fill rectangle with given color */
                                        int w, int h, int color,
                                        int bpp, int stride)
{
        for (j = 0; j < h; j++) {
            memset(dst, color, w);
            dst += stride;
        }
        uint16_t *dst2;
            dst2 = (uint16_t*)dst;
        }
        uint32_t *dst2;
        }
    }
}

                                       GetByteContext *gb, int bpp,
                                       int be, int stride)
{
            case 1:
                dst[i] = p;
                break;
            case 2:
                ((uint16_t*)dst)[i] = p;
                break;
            }
    }
}

                          int w, int h, int stride)
{

                av_log(c->avctx, AV_LOG_ERROR, "Premature end of data!\n");
                return AVERROR_INVALIDDATA;
            }
                    av_log(c->avctx, AV_LOG_ERROR, "Premature end of data!\n");
                    return AVERROR_INVALIDDATA;
                }
            } else {


                }


                        av_log(c->avctx, AV_LOG_ERROR, "Rectangle outside picture\n");
                        return AVERROR_INVALIDDATA;
                    }

                               rect_w, rect_h, fg, bpp, stride);
                }
            }
        }
    }
    return 0;
}

static void reset_buffers(VmncContext *c)
{
    av_freep(&c->curbits);
    av_freep(&c->curmask);
    av_freep(&c->screendta);
    c->cur_w = c->cur_h = 0;
    c->cur_hx = c->cur_hy = 0;

}

                        AVPacket *avpkt)
{

        return AVERROR_INVALIDDATA;

        return ret;


    // restore screen after cursor
            w = c->width - c->cur_x;
            h = c->height - c->cur_y;
            w += dx;
            dx = 0;
        }
            h += dy;
            dy = 0;
        }
            }
        }
    }

            av_log(avctx, AV_LOG_ERROR, "Premature end of data!\n");
            return -1;
        }
            av_log(avctx, AV_LOG_ERROR,
                    "Incorrect frame size: %ix%i+%ix%i of %ix%i\n",
                    w, h, dx, dy, c->width, c->height);
            return AVERROR_INVALIDDATA;
        }
                av_log(avctx, AV_LOG_ERROR, "dimensions too large\n");
                return AVERROR_INVALIDDATA;
            }
                av_log(avctx, AV_LOG_ERROR,
                       "Premature end of data! (need %i got %i)\n",
                       2 + w * h * c->bpp2 * 2, size_left);
                return AVERROR_INVALIDDATA;
            }
                av_log(avctx, AV_LOG_ERROR,
                       "Cursor hot spot is not in image: "
                       "%ix%i of %ix%i cursor size\n",
                       c->cur_hx, c->cur_hy, c->cur_w, c->cur_h);
                c->cur_hx = c->cur_hy = 0;
            }
                reset_buffers(c);
                return AVERROR(EINVAL);
            } else {
                    reset_buffers(c);
                    return ret;
                }
            }
        case MAGIC_WMVe: // unknown
            break;
        case MAGIC_WMVg: // unknown
            break;
        case MAGIC_WMVh: // unknown
            break;
                av_log(avctx, AV_LOG_INFO,
                       "Depth mismatch. Container %i bpp, "
                       "Frame data: %i bpp\n",
                       c->bpp, depth);
            }
                av_log(avctx, AV_LOG_INFO,
                       "Invalid header: bigendian flag = %i\n", c->bigendian);
                return AVERROR_INVALIDDATA;
            }
            //skip the rest of pixel format data
            break;
        case MAGIC_WMVj: // unknown
            break;
                av_log(avctx, AV_LOG_ERROR,
                       "Premature end of data! (need %i got %i)\n",
                       w * h * c->bpp2, size_left);
                return AVERROR_INVALIDDATA;
            }
                      c->pic->linesize[0]);
            break;
            break;
        default:
            av_log(avctx, AV_LOG_ERROR, "Unsupported block type 0x%08X\n", enc);
            chunks = 0; // leave chunks decoding loop
        }
    }
        // save screen data before painting cursor
            w = c->width - c->cur_x;
            h = c->height - c->cur_y;
            w += dx;
            dx = 0;
        }
            h += dy;
            dy = 0;
        }
            }
        }
    }
        return ret;

    /* always report that the buffer was completely consumed */
    return buf_size;
}

{


    case 8:
        avctx->pix_fmt = AV_PIX_FMT_PAL8;
        break;
    case 24:
        /* 24 bits is not technically supported, but some clients might
         * mistakenly set it, so let's assume they actually meant 32 bits */
        c->bpp = 32;
    default:
        av_log(avctx, AV_LOG_ERROR, "Unsupported bitdepth %i\n", c->bpp);
        return AVERROR_INVALIDDATA;
    }

        return AVERROR(ENOMEM);

    return 0;
}

{


}

AVCodec ff_vmnc_decoder = {
    .name           = "vmnc",
    .long_name      = NULL_IF_CONFIG_SMALL("VMware Screen Codec / VMware Video"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_VMNC,
    .priv_data_size = sizeof(VmncContext),
    .init           = decode_init,
    .close          = decode_end,
    .decode         = decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
