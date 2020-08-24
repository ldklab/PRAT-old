/*
 * SGI image decoder
 * Todd Kirby <doubleshot@pacbell.net>
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
#include "libavutil/avassert.h"
#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"
#include "sgi.h"

typedef struct SgiState {
    AVCodecContext *avctx;
    unsigned int width;
    unsigned int height;
    unsigned int depth;
    unsigned int bytes_per_channel;
    int linesize;
    GetByteContext g;
} SgiState;

/**
 * Expand an RLE row into a channel.
 * @param s the current image state
 * @param out_buf Points to one line after the output buffer.
 * @param len length of out_buf in bytes
 * @param pixelstride pixel stride of input buffer
 * @return size of output in bytes, else return error code.
 */
                           int len, int pixelstride)
{

            return AVERROR_INVALIDDATA;
            break;
        }

        /* Check for buffer overflow. */
            av_log(s->avctx, AV_LOG_ERROR, "Invalid pixel count.\n");
            return AVERROR_INVALIDDATA;
        }

            }
        } else {

            }
        }
    }
}

                            int len, int pixelstride)
{

            return AVERROR_INVALIDDATA;
            break;

        /* Check for buffer overflow. */
            av_log(s->avctx, AV_LOG_ERROR, "Invalid pixel count.\n");
            return AVERROR_INVALIDDATA;
        }

            }
        } else {

            }
        }
    }
}


/**
 * Read a run length encoded SGI image.
 * @param out_buf output buffer
 * @param s the current image state
 * @return 0 if no error, else return error code.
 */
{

    /* size of  RLE offset and length tables */
        return AVERROR_INVALIDDATA;
    }

        dest_row = out_buf;
            else
                return AVERROR_INVALIDDATA;
        }
    }
    return 0;
}

/**
 * Read an uncompressed SGI image.
 * @param out_buf output buffer
 * @param s the current image state
 * @return 0 if read success, else return error code.
 */
{

    /* Test buffer size. */
        return AVERROR_INVALIDDATA;

    /* Create a reader for each plane */
    }

            for (x = s->width; x > 0; x--)
                for (z = 0; z < s->depth; z++)
                    *out_end++ = bytestream2_get_byteu(&gp[z]);
        } else {
        }
    }
    return 0;
}

                        void *data, int *got_frame,
                        AVPacket *avpkt)
{

        av_log(avctx, AV_LOG_ERROR, "buf_size too small (%d)\n", avpkt->size);
        return AVERROR_INVALIDDATA;
    }

    /* Test for SGI magic. */
        av_log(avctx, AV_LOG_ERROR, "bad magic number\n");
        return AVERROR_INVALIDDATA;
    }


        av_log(avctx, AV_LOG_ERROR, "wrong channel number\n");
        return AVERROR_INVALIDDATA;
    }

    /* Check for supported image dimensions. */
        av_log(avctx, AV_LOG_ERROR, "wrong dimension number\n");
        return AVERROR_INVALIDDATA;
    }

    } else {
        av_log(avctx, AV_LOG_ERROR, "wrong picture format\n");
        return AVERROR_INVALIDDATA;
    }

        return ret;

        return ret;




    /* Skip header. */
    } else {
    }
        return ret;

}

{


}

AVCodec ff_sgi_decoder = {
    .name           = "sgi",
    .long_name      = NULL_IF_CONFIG_SMALL("SGI image"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_SGI,
    .priv_data_size = sizeof(SgiState),
    .decode         = decode_frame,
    .init           = sgi_decode_init,
    .capabilities   = AV_CODEC_CAP_DR1,
};
