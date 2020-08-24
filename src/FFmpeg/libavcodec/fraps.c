/*
 * Fraps FPS1 decoder
 * Copyright (c) 2005 Roine Gustafsson
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
 * Lossless Fraps 'FPS1' decoder
 * @author Roine Gustafsson (roine at users sf net)
 * @author Konstantin Shishkov
 *
 * Codec algorithm for version 0 is taken from Transcode <www.transcoding.org>
 *
 * Version 2 files support by Konstantin Shishkov
 */

#include "avcodec.h"
#include "get_bits.h"
#include "huffman.h"
#include "bytestream.h"
#include "bswapdsp.h"
#include "internal.h"
#include "thread.h"

#define FPS_TAG MKTAG('F', 'P', 'S', 'x')
#define VLC_BITS 11

/**
 * local variable storage
 */
typedef struct FrapsContext {
    AVCodecContext *avctx;
    BswapDSPContext bdsp;
    uint8_t *tmpbuf;
    int tmpbuf_size;
} FrapsContext;


/**
 * initializes decoder
 * @param avctx codec context
 * @return 0 on success or negative if fails
 */
{



}

/**
 * Comparator - our nodes should ascend by count
 * but with preserved symbol order
 */
{
}

/**
 * decode Fraps v2 packed plane
 */
                               int h, const uint8_t *src, int size, int Uoff,
                               const int step)
{

                                  nodes, huff_cmp,
                                  FF_HUFFMAN_FLAG_ZERO_COUNT)) < 0)
        return ret;
    /* we have built Huffman table and are ready to decode plane */

    /* convert bits so they may be used by standard bitreader */
                      (const uint32_t *) src, size >> 2);

        return ret;

            /* lines are stored as deltas between previous lines
             * and we need to add 0x80 to the first lines of chroma planes
             */
            }
        }
    }
}

                        void *data, int *got_frame,
                        AVPacket *avpkt)
{

        av_log(avctx, AV_LOG_ERROR, "Packet is too short\n");
        return AVERROR_INVALIDDATA;
    }


        avpriv_report_missing_feature(avctx, "Fraps version %u", version);
        return AVERROR_PATCHWELCOME;
    }


        unsigned needed_size = avctx->width * avctx->height + 1024;
        needed_size += header_size;
        if (buf_size != needed_size) {
            av_log(avctx, AV_LOG_ERROR,
                   "Invalid frame length %d (should be %d)\n",
                   buf_size, needed_size);
            return AVERROR_INVALIDDATA;
        }
        /* bit 31 means same as previous pic */
            *got_frame = 0;
            return buf_size;
        }
                   "Invalid frame length %d (should be %d)\n",
                   buf_size, needed_size);
        }
    } else {
        /* skip frame */
        }
            av_log(avctx, AV_LOG_ERROR, "error in data stream\n");
            return AVERROR_INVALIDDATA;
        }
            }
        }
                return AVERROR(ENOMEM);
        }
    }



        return ret;

    default:
        /* Fraps v0 is a reordered YUV420 */
            av_log(avctx, AV_LOG_ERROR, "Invalid frame size %dx%d\n",
                   avctx->width, avctx->height);
            return AVERROR_INVALIDDATA;
        }

        buf32 = (const uint32_t*)buf;
            }
        }
        break;

            uint32_t *pal = (uint32_t *)f->data[1];

            for (y = 0; y < 256; y++) {
                pal[y] = AV_RL32(buf) | 0xFF000000;
                buf += 4;
            }

            for (y = 0; y <avctx->height; y++)
                memcpy(&f->data[0][y * f->linesize[0]],
                       &buf[y * avctx->width],
                       avctx->width);
        } else {
        /* Fraps v1 is an upside-down BGR24 */
        }
        break;

    case 2:
    case 4:
        /**
         * Fraps v2 is Huffman-coded YUV420 planes
         * Fraps v4 is virtually the same
         */
                                           is_chroma, 1)) < 0) {
                av_log(avctx, AV_LOG_ERROR, "Error decoding plane %i\n", i);
                return ret;
            }
        }
        break;
    case 3:
    case 5:
        /* Virtually the same as version 4, but is for RGB24 */
            }
        }
        // convert pseudo-YUV into real RGB
            }
        }
        break;
    }


}


/**
 * closes decoder
 * @param avctx codec context
 * @return 0 on success or negative if fails
 */
{

}


AVCodec ff_fraps_decoder = {
    .name           = "fraps",
    .long_name      = NULL_IF_CONFIG_SMALL("Fraps"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_FRAPS,
    .priv_data_size = sizeof(FrapsContext),
    .init           = decode_init,
    .close          = decode_end,
    .decode         = decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1 | AV_CODEC_CAP_FRAME_THREADS,
    .caps_internal  = FF_CODEC_CAP_INIT_THREADSAFE,
};
