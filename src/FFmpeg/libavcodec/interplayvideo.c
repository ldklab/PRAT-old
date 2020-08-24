/*
 * Interplay MVE Video Decoder
 * Copyright (C) 2003 The FFmpeg project
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
 * Interplay MVE Video Decoder by Mike Melanson (melanson@pcisys.net)
 * For more information about the Interplay MVE format, visit:
 *   http://www.pcisys.net/~melanson/codecs/interplay-mve.txt
 * This code is written in such a way that the identifiers match up
 * with the encoding descriptions in the document.
 *
 * This decoder presently only supports a PAL8 output colorspace.
 *
 * An Interplay video frame consists of 2 parts: The decoding map and
 * the video data. A demuxer must load these 2 parts together in a single
 * buffer before sending it through the stream to this decoder.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libavutil/intreadwrite.h"

#define BITSTREAM_READER_LE
#include "avcodec.h"
#include "bytestream.h"
#include "get_bits.h"
#include "hpeldsp.h"
#include "internal.h"

#define PALETTE_COUNT 256

typedef struct IpvideoContext {

    AVCodecContext *avctx;
    HpelDSPContext hdsp;
    AVFrame *second_last_frame;
    AVFrame *last_frame;

    /* For format 0x10 */
    AVFrame *cur_decode_frame;
    AVFrame *prev_decode_frame;

    const unsigned char *decoding_map;
    int decoding_map_size;
    const unsigned char *skip_map;
    int skip_map_size;

    int is_16bpp;
    GetByteContext stream_ptr, mv_ptr;
    unsigned char *pixel_ptr;
    int line_inc;
    int stride;
    int upper_motion_limit_offset;

    uint32_t pal[256];
} IpvideoContext;

static int copy_from(IpvideoContext *s, AVFrame *src, AVFrame *dst, int delta_x, int delta_y)
{
    int width = dst->width;
    int current_offset = s->pixel_ptr - dst->data[0];
    int x = (current_offset % dst->linesize[0]) / (1 + s->is_16bpp);
    int y = current_offset / dst->linesize[0];
    int dx = delta_x + x - ((delta_x + x >= width) - (delta_x + x < 0)) * width;
    int dy = delta_y + y + (delta_x + x >= width) - (delta_x + x < 0);
    int motion_offset = dy * src->linesize[0] + dx * (1 + s->is_16bpp);

    if (motion_offset < 0) {
        av_log(s->avctx, AV_LOG_ERROR, "motion offset < 0 (%d)\n", motion_offset);
        return AVERROR_INVALIDDATA;
    } else if (motion_offset > s->upper_motion_limit_offset) {
        av_log(s->avctx, AV_LOG_ERROR, "motion offset above limit (%d >= %d)\n",
            motion_offset, s->upper_motion_limit_offset);
        return AVERROR_INVALIDDATA;
    }
    if (!src->data[0]) {
        av_log(s->avctx, AV_LOG_ERROR, "Invalid decode type, corrupted header?\n");
        return AVERROR(EINVAL);
    }
    s->hdsp.put_pixels_tab[!s->is_16bpp][0](s->pixel_ptr, src->data[0] + motion_offset,
                                            dst->linesize[0], 8);
    return 0;
}

{
}

{
}

{

    /* copy block from 2 frames ago using a motion vector; need 1 more byte */
    } else {
    }

    } else {
    }

}

{

    /* copy 8x8 block from current frame from an up/left block */

    /* need 1 more byte for motion */
    } else {
    }

    } else {
    }

}

{

    /* copy a block from the previous frame; need 1 more byte */
    } else {
    }


}

{

    /* copy a block from the previous frame using an expanded range;
     * need 2 more bytes */

}

static int ipvideo_decode_block_opcode_0x6(IpvideoContext *s, AVFrame *frame)
{
    /* mystery opcode? skip multiple blocks? */
    av_log(s->avctx, AV_LOG_ERROR, "Help! Mystery opcode 0x6 seen\n");

    /* report success */
    return 0;
}

{

        av_log(s->avctx, AV_LOG_ERROR, "too little data for opcode 0x7\n");
        return AVERROR_INVALIDDATA;
    }

    /* 2-color encoding */


        /* need 8 more bytes from the stream */
        }

    } else {

        /* need 2 more bytes from the stream */
            }
        }
    }

    /* report success */
    return 0;
}

{

        av_log(s->avctx, AV_LOG_ERROR, "too little data for opcode 0x8\n");
        return AVERROR_INVALIDDATA;
    }

    /* 2-color encoding for each 4x4 quadrant, or 2-color encoding on
     * either top and bottom or left and right halves */

            // new values for each 4x4 block
                }
            }

            // switch to right half
        }

    } else {


            /* vertical split; left & right halves are 2-color encoded */

                // switch to right half
                }
            }

        } else {

            /* horizontal split; top & bottom halves are 2-color encoded */

                }

            }
        }
    }

    /* report success */
    return 0;
}

{

        av_log(s->avctx, AV_LOG_ERROR, "too little data for opcode 0x9\n");
        return AVERROR_INVALIDDATA;
    }

    /* 4-color encoding */


            /* 1 of 4 colors for each pixel, need 16 more bytes */
                /* get the next set of 8 2-bit flags */
            }

        } else {

            /* 1 of 4 colors for each 2x2 block, need 4 more bytes */

                }
            }

        }
    } else {

        /* 1 of 4 colors for each 2x1 or 1x2 block, need 8 more bytes */
                }
            }
        } else {
                }
            }
        }
    }

    /* report success */
    return 0;
}

{

        av_log(s->avctx, AV_LOG_ERROR, "too little data for opcode 0xA\n");
        return AVERROR_INVALIDDATA;
    }


    /* 4-color encoding for each 4x4 quadrant, or 4-color encoding on
     * either top and bottom or left and right halves */

        /* 4-color encoding for each quadrant; need 32 bytes */
            // new values for each 4x4 block
            }


            // switch to right half
        }

    } else {
        // vertical split?


        /* 4-color encoding for either left and right or top and bottom
         * halves */


                // switch to right half

            // load values for second half
            }
        }
    }

    /* report success */
    return 0;
}

{

    /* 64-color encoding (each pixel in block is a different color) */
    }

    /* report success */
}

{

    /* 16-color block encoding: each 2x2 block is a different color */
        }
    }

    /* report success */
}

{

        av_log(s->avctx, AV_LOG_ERROR, "too little data for opcode 0xD\n");
        return AVERROR_INVALIDDATA;
    }

    /* 4-color block encoding: each 4x4 block is a different color */
        }
    }

    /* report success */
    return 0;
}

{

    /* 1-color encoding: the whole block is 1 solid color */

    }

    /* report success */
}

{

    /* dithered encoding */

        }
    }

    /* report success */
}

static int ipvideo_decode_block_opcode_0x6_16(IpvideoContext *s, AVFrame *frame)
{
    signed char x, y;

    /* copy a block from the second last frame using an expanded range */
    x = bytestream2_get_byte(&s->stream_ptr);
    y = bytestream2_get_byte(&s->stream_ptr);

    ff_tlog(s->avctx, "motion bytes = %d, %d\n", x, y);
    return copy_from(s, s->second_last_frame, frame, x, y);
}

{

    /* 2-color encoding */


        }

    } else {

            }
        }
    }

}

{

    /* 2-color encoding for each 4x4 quadrant, or 2-color encoding on
     * either top and bottom or left and right halves */


            // new values for each 4x4 block
                }
            }

            // switch to right half
        }

    } else {



            /* vertical split; left & right halves are 2-color encoded */

                // switch to right half
                }
            }

        } else {

            /* horizontal split; top & bottom halves are 2-color encoded */

                }

            }
        }
    }

    /* report success */
}

{

    /* 4-color encoding */


            /* 1 of 4 colors for each pixel */
                /* get the next set of 8 2-bit flags */
            }

        } else {

            /* 1 of 4 colors for each 2x2 block */

                }
            }

        }
    } else {

        /* 1 of 4 colors for each 2x1 or 1x2 block */
                }
            }
        } else {
                }
            }
        }
    }

    /* report success */
}

{


    /* 4-color encoding for each 4x4 quadrant, or 4-color encoding on
     * either top and bottom or left and right halves */

        /* 4-color encoding for each quadrant */
            // new values for each 4x4 block
            }


            // switch to right half
        }

    } else {
        // vertical split?


        /* 4-color encoding for either left and right or top and bottom
         * halves */


                // switch to right half

            // load values for second half
            }
        }
    }

    /* report success */
}

{

    /* 64-color encoding (each pixel in block is a different color) */
    }

    /* report success */
}

{

    /* 16-color block encoding: each 2x2 block is a different color */
        }
    }

    /* report success */
}

{

    /* 4-color block encoding: each 4x4 block is a different color */
        }
    }

    /* report success */
}

{

    /* 1-color encoding: the whole block is 1 solid color */

    }

    /* report success */
}

static int (* const ipvideo_decode_block[])(IpvideoContext *s, AVFrame *frame) = {
    ipvideo_decode_block_opcode_0x0, ipvideo_decode_block_opcode_0x1,
    ipvideo_decode_block_opcode_0x2, ipvideo_decode_block_opcode_0x3,
    ipvideo_decode_block_opcode_0x4, ipvideo_decode_block_opcode_0x5,
    ipvideo_decode_block_opcode_0x6, ipvideo_decode_block_opcode_0x7,
    ipvideo_decode_block_opcode_0x8, ipvideo_decode_block_opcode_0x9,
    ipvideo_decode_block_opcode_0xA, ipvideo_decode_block_opcode_0xB,
    ipvideo_decode_block_opcode_0xC, ipvideo_decode_block_opcode_0xD,
    ipvideo_decode_block_opcode_0xE, ipvideo_decode_block_opcode_0xF,
};

static int (* const ipvideo_decode_block16[])(IpvideoContext *s, AVFrame *frame) = {
    ipvideo_decode_block_opcode_0x0,    ipvideo_decode_block_opcode_0x1,
    ipvideo_decode_block_opcode_0x2,    ipvideo_decode_block_opcode_0x3,
    ipvideo_decode_block_opcode_0x4,    ipvideo_decode_block_opcode_0x5,
    ipvideo_decode_block_opcode_0x6_16, ipvideo_decode_block_opcode_0x7_16,
    ipvideo_decode_block_opcode_0x8_16, ipvideo_decode_block_opcode_0x9_16,
    ipvideo_decode_block_opcode_0xA_16, ipvideo_decode_block_opcode_0xB_16,
    ipvideo_decode_block_opcode_0xC_16, ipvideo_decode_block_opcode_0xD_16,
    ipvideo_decode_block_opcode_0xE_16, ipvideo_decode_block_opcode_0x1,
};

static void ipvideo_format_06_firstpass(IpvideoContext *s, AVFrame *frame, int16_t opcode)
{
    int line;

    if (!opcode) {
        for (line = 0; line < 8; ++line) {
            bytestream2_get_buffer(&s->stream_ptr, s->pixel_ptr, 8);
            s->pixel_ptr += s->stride;
        }
    } else {
        /* Don't try to copy second_last_frame data on the first frames */
        if (s->avctx->frame_number > 2)
            copy_from(s, s->second_last_frame, frame, 0, 0);
    }
}

static void ipvideo_format_06_secondpass(IpvideoContext *s, AVFrame *frame, int16_t opcode)
{
    int off_x, off_y;

    if (opcode < 0) {
        off_x = ((uint16_t)opcode - 0xC000) % frame->width;
        off_y = ((uint16_t)opcode - 0xC000) / frame->width;
        copy_from(s, s->last_frame, frame, off_x, off_y);
    } else if (opcode > 0) {
        off_x = ((uint16_t)opcode - 0x4000) % frame->width;
        off_y = ((uint16_t)opcode - 0x4000) / frame->width;
        copy_from(s, frame, frame, off_x, off_y);
    }
}

static void (* const ipvideo_format_06_passes[])(IpvideoContext *s, AVFrame *frame, int16_t op) = {
    ipvideo_format_06_firstpass, ipvideo_format_06_secondpass,
};

static void ipvideo_decode_format_06_opcodes(IpvideoContext *s, AVFrame *frame)
{
    int pass, x, y;
    int16_t opcode;
    GetByteContext decoding_map_ptr;

    /* this is PAL8, so make the palette available */
    memcpy(frame->data[1], s->pal, AVPALETTE_SIZE);
    s->stride = frame->linesize[0];

    s->line_inc = s->stride - 8;
    s->upper_motion_limit_offset = (s->avctx->height - 8) * frame->linesize[0]
                                  + (s->avctx->width - 8) * (1 + s->is_16bpp);

    bytestream2_init(&decoding_map_ptr, s->decoding_map, s->decoding_map_size);

    for (pass = 0; pass < 2; ++pass) {
        bytestream2_seek(&decoding_map_ptr, 0, SEEK_SET);
        for (y = 0; y < s->avctx->height; y += 8) {
            for (x = 0; x < s->avctx->width; x += 8) {
                opcode = bytestream2_get_le16(&decoding_map_ptr);

                ff_tlog(s->avctx,
                        "  block @ (%3d, %3d): opcode 0x%X, data ptr offset %d\n",
                        x, y, opcode, bytestream2_tell(&s->stream_ptr));

                s->pixel_ptr = frame->data[0] + x + y * frame->linesize[0];
                ipvideo_format_06_passes[pass](s, frame, opcode);
            }
        }
    }

    if (bytestream2_get_bytes_left(&s->stream_ptr) > 1) {
        av_log(s->avctx, AV_LOG_DEBUG,
               "decode finished with %d bytes left over\n",
               bytestream2_get_bytes_left(&s->stream_ptr));
    }
}

static void ipvideo_format_10_firstpass(IpvideoContext *s, AVFrame *frame, int16_t opcode)
{
    int line;

    if (!opcode) {
        for (line = 0; line < 8; ++line) {
            bytestream2_get_buffer(&s->stream_ptr, s->pixel_ptr, 8);
            s->pixel_ptr += s->stride;
        }
    }
}

static void ipvideo_format_10_secondpass(IpvideoContext *s, AVFrame *frame, int16_t opcode)
{
    int off_x, off_y;

    if (opcode < 0) {
        off_x = ((uint16_t)opcode - 0xC000) % s->cur_decode_frame->width;
        off_y = ((uint16_t)opcode - 0xC000) / s->cur_decode_frame->width;
        copy_from(s, s->prev_decode_frame, s->cur_decode_frame, off_x, off_y);
    } else if (opcode > 0) {
        off_x = ((uint16_t)opcode - 0x4000) % s->cur_decode_frame->width;
        off_y = ((uint16_t)opcode - 0x4000) / s->cur_decode_frame->width;
        copy_from(s, s->cur_decode_frame, s->cur_decode_frame, off_x, off_y);
    }
}

static void (* const ipvideo_format_10_passes[])(IpvideoContext *s, AVFrame *frame, int16_t op) = {
    ipvideo_format_10_firstpass, ipvideo_format_10_secondpass,
};

static void ipvideo_decode_format_10_opcodes(IpvideoContext *s, AVFrame *frame)
{
    int pass, x, y, changed_block;
    int16_t opcode, skip;
    GetByteContext decoding_map_ptr;
    GetByteContext skip_map_ptr;

    bytestream2_skip(&s->stream_ptr, 14); /* data starts 14 bytes in */

    /* this is PAL8, so make the palette available */
    memcpy(frame->data[1], s->pal, AVPALETTE_SIZE);
    s->stride = frame->linesize[0];

    s->line_inc = s->stride - 8;
    s->upper_motion_limit_offset = (s->avctx->height - 8) * frame->linesize[0]
                                  + (s->avctx->width - 8) * (1 + s->is_16bpp);

    bytestream2_init(&decoding_map_ptr, s->decoding_map, s->decoding_map_size);
    bytestream2_init(&skip_map_ptr, s->skip_map, s->skip_map_size);

    for (pass = 0; pass < 2; ++pass) {
        bytestream2_seek(&decoding_map_ptr, 0, SEEK_SET);
        bytestream2_seek(&skip_map_ptr, 0, SEEK_SET);
        skip = bytestream2_get_le16(&skip_map_ptr);

        for (y = 0; y < s->avctx->height; y += 8) {
            for (x = 0; x < s->avctx->width; x += 8) {
                s->pixel_ptr = s->cur_decode_frame->data[0] + x + y * s->cur_decode_frame->linesize[0];

                while (skip <= 0)  {
                    if (skip != -0x8000 && skip) {
                        opcode = bytestream2_get_le16(&decoding_map_ptr);
                        ipvideo_format_10_passes[pass](s, frame, opcode);
                        break;
                    }
                    if (bytestream2_get_bytes_left(&skip_map_ptr) < 2)
                        return;
                    skip = bytestream2_get_le16(&skip_map_ptr);
                }
                skip *= 2;
            }
        }
    }

    bytestream2_seek(&skip_map_ptr, 0, SEEK_SET);
    skip = bytestream2_get_le16(&skip_map_ptr);
    for (y = 0; y < s->avctx->height; y += 8) {
        for (x = 0; x < s->avctx->width; x += 8) {
            changed_block = 0;
            s->pixel_ptr = frame->data[0] + x + y*frame->linesize[0];

            while (skip <= 0)  {
                if (skip != -0x8000 && skip) {
                    changed_block = 1;
                    break;
                }
                if (bytestream2_get_bytes_left(&skip_map_ptr) < 2)
                    return;
                skip = bytestream2_get_le16(&skip_map_ptr);
            }

            if (changed_block) {
                copy_from(s, s->cur_decode_frame, frame, 0, 0);
            } else {
                /* Don't try to copy last_frame data on the first frame */
                if (s->avctx->frame_number)
                    copy_from(s, s->last_frame, frame, 0, 0);
            }
            skip *= 2;
        }
    }

    FFSWAP(AVFrame*, s->prev_decode_frame, s->cur_decode_frame);

    if (bytestream2_get_bytes_left(&s->stream_ptr) > 1) {
        av_log(s->avctx, AV_LOG_DEBUG,
               "decode finished with %d bytes left over\n",
               bytestream2_get_bytes_left(&s->stream_ptr));
    }
}

{

        /* this is PAL8, so make the palette available */

    } else {
    }

                return;

                    "  block @ (%3d, %3d): encoding 0x%X, data ptr offset %d\n",
                    x, y, opcode, bytestream2_tell(&s->stream_ptr));

            } else {
            }
                av_log(s->avctx, AV_LOG_ERROR, "decode problem on frame %d, @ block (%d, %d)\n",
                       s->avctx->frame_number, x, y);
                return;
            }
        }
    }
               "decode finished with %d bytes left over\n",
               bytestream2_get_bytes_left(&s->stream_ptr));
    }
}

{




        ret = AVERROR(ENOMEM);
        goto error;
    }


error:
    av_frame_free(&s->last_frame);
    av_frame_free(&s->second_last_frame);
    av_frame_free(&s->cur_decode_frame);
    av_frame_free(&s->prev_decode_frame);
    return ret;
}

                                void *data, int *got_frame,
                                AVPacket *avpkt)
{

    }

            return ret;

            av_frame_unref(s->cur_decode_frame);
            return ret;
        }
    }

        return AVERROR_INVALIDDATA;


    case 0x06:
        if (s->decoding_map_size) {
            av_log(avctx, AV_LOG_ERROR, "Decoding map for format 0x06\n");
            return AVERROR_INVALIDDATA;
        }

        if (s->skip_map_size) {
            av_log(avctx, AV_LOG_ERROR, "Skip map for format 0x06\n");
            return AVERROR_INVALIDDATA;
        }

        if (s->is_16bpp) {
            av_log(avctx, AV_LOG_ERROR, "Video format 0x06 does not support 16bpp movies\n");
            return AVERROR_INVALIDDATA;
        }

        /* Decoding map for 0x06 frame format is at the top of pixeldata */
        s->decoding_map_size = ((s->avctx->width / 8) * (s->avctx->height / 8)) * 2;
        s->decoding_map = buf + 8 + 14; /* 14 bits of op data */
        video_data_size -= s->decoding_map_size + 14;
        if (video_data_size <= 0 || s->decoding_map_size == 0)
            return AVERROR_INVALIDDATA;

        if (buf_size < 8 + s->decoding_map_size + 14 + video_data_size)
            return AVERROR_INVALIDDATA;

        bytestream2_init(&s->stream_ptr, buf + 8 + s->decoding_map_size + 14, video_data_size);

        break;

    case 0x10:
        if (! s->decoding_map_size) {
            av_log(avctx, AV_LOG_ERROR, "Empty decoding map for format 0x10\n");
            return AVERROR_INVALIDDATA;
        }

        if (! s->skip_map_size) {
            av_log(avctx, AV_LOG_ERROR, "Empty skip map for format 0x10\n");
            return AVERROR_INVALIDDATA;
        }

        if (s->is_16bpp) {
            av_log(avctx, AV_LOG_ERROR, "Video format 0x10 does not support 16bpp movies\n");
            return AVERROR_INVALIDDATA;
        }

        if (buf_size < 8 + video_data_size + s->decoding_map_size + s->skip_map_size)
            return AVERROR_INVALIDDATA;

        bytestream2_init(&s->stream_ptr, buf + 8, video_data_size);
        s->decoding_map = buf + 8 + video_data_size;
        s->skip_map = buf + 8 + video_data_size + s->decoding_map_size;

        break;

            av_log(avctx, AV_LOG_ERROR, "Empty decoding map for format 0x11\n");
            return AVERROR_INVALIDDATA;
        }

            av_log(avctx, AV_LOG_ERROR, "Skip map for format 0x11\n");
            return AVERROR_INVALIDDATA;
        }

            return AVERROR_INVALIDDATA;



    default:
        av_log(avctx, AV_LOG_ERROR, "Frame type 0x%02X unsupported\n", frame_format);
    }

    /* ensure we can't overread the packet */
        av_log(avctx, AV_LOG_ERROR, "Invalid IP packet size\n");
        return AVERROR_INVALIDDATA;
    }

        return ret;

            av_log(avctx, AV_LOG_ERROR, "Palette size %d is wrong\n", size);
        }
    }

    case 0x06:
        ipvideo_decode_format_06_opcodes(s, frame);
        break;
    case 0x10:
        ipvideo_decode_format_10_opcodes(s, frame);
        break;
    }


    /* shuffle frames */
        return ret;

    /* report that the buffer was completely consumed */
    return buf_size;
}

{


}

AVCodec ff_interplay_video_decoder = {
    .name           = "interplayvideo",
    .long_name      = NULL_IF_CONFIG_SMALL("Interplay MVE video"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_INTERPLAY_VIDEO,
    .priv_data_size = sizeof(IpvideoContext),
    .init           = ipvideo_decode_init,
    .close          = ipvideo_decode_end,
    .decode         = ipvideo_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1 | AV_CODEC_CAP_PARAM_CHANGE,
};
