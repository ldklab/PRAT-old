/*
 * QPEG codec
 * Copyright (c) 2004 Konstantin Shishkov
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
 * QPEG codec.
 */

#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"

typedef struct QpegContext{
    AVCodecContext *avctx;
    AVFrame *ref;
    uint32_t pal[256];
    GetByteContext buffer;
} QpegContext;

                              int stride, int width, int height)
{


            break;
            c0 = bytestream2_get_byte(&qctx->buffer);
            c1 = bytestream2_get_byte(&qctx->buffer);
            run = ((code & 0x7) << 16) + (c0 << 8) + c1 + 2;
            c0 = bytestream2_get_byte(&qctx->buffer);
            c1 = bytestream2_get_byte(&qctx->buffer);
            copy = ((code & 0x3F) << 16) + (c0 << 8) + c1 + 1;
            c0 = bytestream2_get_byte(&qctx->buffer);
            copy = ((code & 0x7F) << 8) + c0 + 1;
        } else { /* short copy */
        }

        /* perform actual run or copy */

                        memset(dst, p, width);
                        dst -= stride;
                        rows_to_go--;
                        i += width;
                    }
                        break;
                }
            }
        } else {
                copy = bytestream2_get_bytes_left(&qctx->buffer);
                        break;
                }
            }
        }
    }

static const int qpeg_table_h[16] =
 { 0x00, 0x20, 0x20, 0x20, 0x18, 0x10, 0x10, 0x20, 0x10, 0x08, 0x18, 0x08, 0x08, 0x18, 0x10, 0x04};
static const int qpeg_table_w[16] =
 { 0x00, 0x20, 0x18, 0x08, 0x18, 0x10, 0x20, 0x10, 0x08, 0x10, 0x20, 0x20, 0x08, 0x10, 0x18, 0x04};

/* Decodes delta frames */
                              int stride, int width, int height,
                              int delta, const uint8_t *ctable,
                              uint8_t *refdata)
{

        /* copy prev frame */
    } else {
        refdata = dst;
    }



            /* motion compensation */

                    /* get block size by index */

                    /* extract motion vector */



                    /* check motion vector */
                        av_log(qctx->avctx, AV_LOG_ERROR, "Bogus motion vector (%i,%i), block size %ix%i at %i,%i\n",
                               me_x, me_y, me_w, me_h, filled, height);
                    else {
                        /* do motion compensation */
                        }
                    }
                }
            }
        }

            break;

                        break;
                }
            }

                break;

                        break;
                }
            }

            /* codes 0x80 and 0x81 are actually escape codes,
               skip value minus constant is in the next byte */
            else
                skip = code;
                    break;
            }
        } else {
            /* zero code treated as one-pixel skip */
            }
            else
            }
        }
    }

                        void *data, int *got_frame,
                        AVPacket *avpkt)
{

        av_log(avctx, AV_LOG_ERROR, "Packet is too small\n");
        return AVERROR_INVALIDDATA;
    }


        return ret;

    } else {
    }

    /* make the palette available on the way out */
        av_log(avctx, AV_LOG_ERROR, "Palette size %d is wrong\n", pal_size);
    }

        return ret;



}





{


}



        return AVERROR(ENOMEM);


}

AVCodec ff_qpeg_decoder = {
    .name           = "qpeg",
    .long_name      = NULL_IF_CONFIG_SMALL("Q-team QPEG"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_QPEG,
    .priv_data_size = sizeof(QpegContext),
    .init           = decode_init,
    .close          = decode_end,
    .decode         = decode_frame,
    .flush          = decode_flush,
    .capabilities   = AV_CODEC_CAP_DR1,
    .caps_internal  = FF_CODEC_CAP_INIT_THREADSAFE |
                      FF_CODEC_CAP_INIT_CLEANUP,
};
