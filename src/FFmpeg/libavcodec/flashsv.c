/*
 * Flash Screen Video decoder
 * Copyright (C) 2004 Alex Beregszaszi
 * Copyright (C) 2006 Benjamin Larsson
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
 * Flash Screen Video decoder
 * @author Alex Beregszaszi
 * @author Benjamin Larsson
 * @author Daniel Verkamp
 * @author Konstantin Shishkov
 *
 * A description of the bitstream format for Flash Screen Video version 1/2
 * is part of the SWF File Format Specification (version 10), which can be
 * downloaded from http://www.adobe.com/devnet/swf.html.
 */

#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>

#include "libavutil/intreadwrite.h"
#include "avcodec.h"
#include "bytestream.h"
#include "get_bits.h"
#include "internal.h"

typedef struct BlockInfo {
    uint8_t *pos;
    int      size;
} BlockInfo;

typedef struct FlashSVContext {
    AVCodecContext *avctx;
    AVFrame        *frame;
    int             image_width, image_height;
    int             block_width, block_height;
    uint8_t        *tmpblock;
    int             block_size;
    z_stream        zstream;
    int             ver;
    const uint32_t *pal;
    int             is_keyframe;
    uint8_t        *keyframedata;
    uint8_t        *keyframe;
    BlockInfo      *blocks;
    uint8_t        *deflate_block;
    int             deflate_block_size;
    int             color_depth;
    int             zlibprime_curr, zlibprime_prev;
    int             diff_start, diff_height;
} FlashSVContext;

                         int h, int w, int stride, const uint32_t *pal)
{

                /* 15-bit color */
                /* 000aaabb -> aaabbaaa  */
            } else {
                /* palette index */
            }
        }
    }
}

{
    /* release the frame if needed */

    /* free the tmpblock */

}

{

        av_log(avctx, AV_LOG_ERROR, "Inflate init error: %d\n", zret);
        return 1;
    }

        return AVERROR(ENOMEM);
    }

    return 0;
}

{

        return AVERROR_INVALIDDATA;



        return -1;

        av_log(s->avctx, AV_LOG_ERROR, "Inflate reset error: %d\n", zret);
        return AVERROR_UNKNOWN;
    }


}

static int flashsv_decode_block(AVCodecContext *avctx, AVPacket *avpkt,
                                GetBitContext *gb, int block_size,
                                int width, int height, int x_pos, int y_pos,
                                int blk_idx)
{
    struct FlashSVContext *s = avctx->priv_data;
    uint8_t *line = s->tmpblock;
    int k;
    int ret = inflateReset(&s->zstream);
    if (ret != Z_OK) {
        av_log(avctx, AV_LOG_ERROR, "Inflate reset error: %d\n", ret);
        return AVERROR_UNKNOWN;
    }
    if (s->zlibprime_curr || s->zlibprime_prev) {
        ret = flashsv2_prime(s,
                             s->blocks[blk_idx].pos,
                             s->blocks[blk_idx].size);
        if (ret < 0)
            return ret;
    }
    s->zstream.next_in   = avpkt->data + get_bits_count(gb) / 8;
    s->zstream.avail_in  = block_size;
    s->zstream.next_out  = s->tmpblock;
    s->zstream.avail_out = s->block_size * 3;
    ret = inflate(&s->zstream, Z_FINISH);
    if (ret == Z_DATA_ERROR) {
        av_log(avctx, AV_LOG_ERROR, "Zlib resync occurred\n");
        inflateSync(&s->zstream);
        ret = inflate(&s->zstream, Z_FINISH);
    }

    if (ret != Z_OK && ret != Z_STREAM_END) {
        //return -1;
    }

    if (s->is_keyframe) {
        s->blocks[blk_idx].pos  = s->keyframedata + (get_bits_count(gb) / 8);
        s->blocks[blk_idx].size = block_size;
    }

    y_pos += s->diff_start;

    if (!s->color_depth) {
        /* Flash Screen Video stores the image upside down, so copy
         * lines to destination in reverse order. */
        for (k = 1; k <= s->diff_height; k++) {
            memcpy(s->frame->data[0] + x_pos * 3 +
                   (s->image_height - y_pos - k) * s->frame->linesize[0],
                   line, width * 3);
            /* advance source pointer to next line */
            line += width * 3;
        }
    } else {
        /* hybrid 15-bit/palette mode */
        ret = decode_hybrid(s->tmpblock, s->zstream.next_out,
                      s->frame->data[0],
                      s->image_height - (y_pos + 1 + s->diff_height),
                      x_pos, s->diff_height, width,
                      s->frame->linesize[0], s->pal);
        if (ret < 0) {
            av_log(avctx, AV_LOG_ERROR, "decode_hybrid failed\n");
            return ret;
        }
    }
    skip_bits_long(gb, 8 * block_size); /* skip the consumed bits */
    return 0;
}

{

        return -1;

}

                                int *got_frame, AVPacket *avpkt)
{

    /* no supplementary picture */
        return 0;
        return -1;

        return ret;

    /* start to parse the bitstream */


            avpriv_request_sample(avctx, "iframe");
            return AVERROR_PATCHWELCOME;
        }
            avpriv_request_sample(avctx, "Custom palette");
            return AVERROR_PATCHWELCOME;
        }
    }

    /* calculate number of blocks and size of border (partial) blocks */

    /* the block size could change between frames, make sure the buffer
     * is large enough, if not, get a larger one */

            s->block_size = 0;
            av_log(avctx, AV_LOG_ERROR,
                   "Cannot allocate decompression buffer.\n");
            return err;
        }
                av_log(avctx, AV_LOG_ERROR,
                       "Cannot determine deflate buffer size.\n");
                return -1;
            }
                s->block_size = 0;
                av_log(avctx, AV_LOG_ERROR, "Cannot allocate deflate buffer.\n");
                return err;
            }
        }
    }

    /* initialize the image size once */
            return ret;
    }

    /* check for changes of image width and image height */
        av_log(avctx, AV_LOG_ERROR,
               "Frame width or height differs from first frame!\n");
        av_log(avctx, AV_LOG_ERROR, "fh = %d, fv %d  vs  ch = %d, cv = %d\n",
               avctx->height, avctx->width, s->image_height, s->image_width);
        return AVERROR_INVALIDDATA;
    }

    /* we care for keyframes only in Screen Video v2 */
            return err;
    }
                               sizeof(s->blocks[0]));

            s->image_width, s->image_height, s->block_width, s->block_height,
            h_blocks, v_blocks, h_part, v_part);

        return ret;

    /* loop over all block columns */


        /* loop over all block rows */

            /* get the size of the compressed zlib chunk */


                av_frame_unref(s->frame);
                return AVERROR_INVALIDDATA;
            }


                    av_log(avctx, AV_LOG_ERROR,
                           "%dx%d invalid color depth %d\n",
                           i, j, s->color_depth);
                    return AVERROR_INVALIDDATA;
                }

                    if (size < 3) {
                        av_log(avctx, AV_LOG_ERROR, "size too small for diff\n");
                        return AVERROR_INVALIDDATA;
                    }
                    if (!s->keyframe) {
                        av_log(avctx, AV_LOG_ERROR,
                               "Inter frame without keyframe\n");
                        return AVERROR_INVALIDDATA;
                    }
                    s->diff_start  = get_bits(&gb, 8);
                    s->diff_height = get_bits(&gb, 8);
                    if (s->diff_start + s->diff_height > cur_blk_height) {
                        av_log(avctx, AV_LOG_ERROR,
                               "Block parameters invalid: %d + %d > %d\n",
                               s->diff_start, s->diff_height, cur_blk_height);
                        return AVERROR_INVALIDDATA;
                    }
                    av_log(avctx, AV_LOG_DEBUG,
                           "%dx%d diff start %d height %d\n",
                           i, j, s->diff_start, s->diff_height);
                    size -= 2;
                }


                    int col = get_bits(&gb, 8);
                    int row = get_bits(&gb, 8);
                    av_log(avctx, AV_LOG_DEBUG, "%dx%d zlibprime_curr %dx%d\n",
                           i, j, col, row);
                    if (size < 3) {
                        av_log(avctx, AV_LOG_ERROR, "size too small for zlibprime_curr\n");
                        return AVERROR_INVALIDDATA;
                    }
                    size -= 2;
                    avpriv_request_sample(avctx, "zlibprime_curr");
                    return AVERROR_PATCHWELCOME;
                }
                    av_log(avctx, AV_LOG_ERROR,
                           "no data available for zlib priming\n");
                    return AVERROR_INVALIDDATA;
                }
            }

                int k;
                int off = (s->image_height - y_pos - 1) * s->frame->linesize[0];

                for (k = 0; k < cur_blk_height; k++) {
                    int x = off - k * s->frame->linesize[0] + x_pos * 3;
                    memcpy(s->frame->data[0] + x, s->keyframe + x,
                           cur_blk_width * 3);
                }
            }

            /* skip unchanged blocks, which have size 0 */
                                         cur_blk_width, cur_blk_height,
                                         x_pos, y_pos,
                    av_log(avctx, AV_LOG_ERROR,
                           "error in decompression of block %dx%d\n", i, j);
            }
        }
    }
                av_log(avctx, AV_LOG_ERROR, "Cannot allocate image data\n");
                return AVERROR(ENOMEM);
            }
        }
    }

        return ret;


        av_log(avctx, AV_LOG_ERROR, "buffer not fully consumed (%d != %d)\n",
               buf_size, (get_bits_count(&gb) / 8));

    /* report that the buffer was completely consumed */
    return buf_size;
}

#if CONFIG_FLASHSV_DECODER
AVCodec ff_flashsv_decoder = {
    .name           = "flashsv",
    .long_name      = NULL_IF_CONFIG_SMALL("Flash Screen Video v1"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_FLASHSV,
    .priv_data_size = sizeof(FlashSVContext),
    .init           = flashsv_decode_init,
    .close          = flashsv_decode_end,
    .decode         = flashsv_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
    .caps_internal  = FF_CODEC_CAP_INIT_CLEANUP,
    .pix_fmts       = (const enum AVPixelFormat[]) { AV_PIX_FMT_BGR24, AV_PIX_FMT_NONE },
};
#endif /* CONFIG_FLASHSV_DECODER */

#if CONFIG_FLASHSV2_DECODER
static const uint32_t ff_flashsv2_default_palette[128] = {
    0x000000, 0x333333, 0x666666, 0x999999, 0xCCCCCC, 0xFFFFFF,
    0x330000, 0x660000, 0x990000, 0xCC0000, 0xFF0000, 0x003300,
    0x006600, 0x009900, 0x00CC00, 0x00FF00, 0x000033, 0x000066,
    0x000099, 0x0000CC, 0x0000FF, 0x333300, 0x666600, 0x999900,
    0xCCCC00, 0xFFFF00, 0x003333, 0x006666, 0x009999, 0x00CCCC,
    0x00FFFF, 0x330033, 0x660066, 0x990099, 0xCC00CC, 0xFF00FF,
    0xFFFF33, 0xFFFF66, 0xFFFF99, 0xFFFFCC, 0xFF33FF, 0xFF66FF,
    0xFF99FF, 0xFFCCFF, 0x33FFFF, 0x66FFFF, 0x99FFFF, 0xCCFFFF,
    0xCCCC33, 0xCCCC66, 0xCCCC99, 0xCCCCFF, 0xCC33CC, 0xCC66CC,
    0xCC99CC, 0xCCFFCC, 0x33CCCC, 0x66CCCC, 0x99CCCC, 0xFFCCCC,
    0x999933, 0x999966, 0x9999CC, 0x9999FF, 0x993399, 0x996699,
    0x99CC99, 0x99FF99, 0x339999, 0x669999, 0xCC9999, 0xFF9999,
    0x666633, 0x666699, 0x6666CC, 0x6666FF, 0x663366, 0x669966,
    0x66CC66, 0x66FF66, 0x336666, 0x996666, 0xCC6666, 0xFF6666,
    0x333366, 0x333399, 0x3333CC, 0x3333FF, 0x336633, 0x339933,
    0x33CC33, 0x33FF33, 0x663333, 0x993333, 0xCC3333, 0xFF3333,
    0x003366, 0x336600, 0x660033, 0x006633, 0x330066, 0x663300,
    0x336699, 0x669933, 0x993366, 0x339966, 0x663399, 0x996633,
    0x6699CC, 0x99CC66, 0xCC6699, 0x66CC99, 0x9966CC, 0xCC9966,
    0x99CCFF, 0xCCFF99, 0xFF99CC, 0x99FFCC, 0xCC99FF, 0xFFCC99,
    0x111111, 0x222222, 0x444444, 0x555555, 0xAAAAAA, 0xBBBBBB,
    0xDDDDDD, 0xEEEEEE
};

{

        return ret;

}

{


}

AVCodec ff_flashsv2_decoder = {
    .name           = "flashsv2",
    .long_name      = NULL_IF_CONFIG_SMALL("Flash Screen Video v2"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_FLASHSV2,
    .priv_data_size = sizeof(FlashSVContext),
    .init           = flashsv2_decode_init,
    .close          = flashsv2_decode_end,
    .decode         = flashsv_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
    .caps_internal  = FF_CODEC_CAP_INIT_CLEANUP,
    .pix_fmts       = (const enum AVPixelFormat[]) { AV_PIX_FMT_BGR24, AV_PIX_FMT_NONE },
};
#endif /* CONFIG_FLASHSV2_DECODER */
