/*
 * Westwood Studios VQA Video Decoder
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
 * VQA Video Decoder
 * @author Mike Melanson (melanson@pcisys.net)
 * @see http://wiki.multimedia.cx/index.php?title=VQA
 *
 * The VQA video decoder outputs PAL8 or RGB555 colorspace data, depending
 * on the type of data in the file.
 *
 * This decoder needs the 42-byte VQHD header from the beginning
 * of the VQA file passed through the extradata field. The VQHD header
 * is laid out as:
 *
 *   bytes 0-3   chunk fourcc: 'VQHD'
 *   bytes 4-7   chunk size in big-endian format, should be 0x0000002A
 *   bytes 8-49  VQHD chunk data
 *
 * Bytes 8-49 are what this decoder expects to see.
 *
 * Briefly, VQA is a vector quantized animation format that operates in a
 * VGA palettized colorspace. It operates on pixel vectors (blocks)
 * of either 4x2 or 4x4 in size. Compressed VQA chunks can contain vector
 * codebooks, palette information, and code maps for rendering vectors onto
 * frames. Any of these components can also be compressed with a run-length
 * encoding (RLE) algorithm commonly referred to as "format80".
 *
 * VQA takes a novel approach to rate control. Each group of n frames
 * (usually, n = 8) relies on a different vector codebook. Rather than
 * transporting an entire codebook every 8th frame, the new codebook is
 * broken up into 8 pieces and sent along with the compressed video chunks
 * for each of the 8 frames preceding the 8 frames which require the
 * codebook. A full codebook is also sent on the very first frame of a
 * file. This is an interesting technique, although it makes random file
 * seeking difficult despite the fact that the frames are all intracoded.
 *
 * V1,2 VQA uses 12-bit codebook indexes. If the 12-bit indexes were
 * packed into bytes and then RLE compressed, bytewise, the results would
 * be poor. That is why the coding method divides each index into 2 parts,
 * the top 4 bits and the bottom 8 bits, then RL encodes the 4-bit pieces
 * together and the 8-bit pieces together. If most of the vectors are
 * clustered into one group of 256 vectors, most of the 4-bit index pieces
 * should be the same.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libavutil/intreadwrite.h"
#include "libavutil/imgutils.h"
#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"

#define PALETTE_COUNT 256
#define VQA_HEADER_SIZE 0x2A

/* allocate the maximum vector space, regardless of the file version:
 * (0xFF00 codebook vectors + 0x100 solid pixel vectors) * (4x4 pixels/block) */
#define MAX_CODEBOOK_VECTORS 0xFF00
#define SOLID_PIXEL_VECTORS 0x100
#define MAX_VECTORS (MAX_CODEBOOK_VECTORS + SOLID_PIXEL_VECTORS)
#define MAX_CODEBOOK_SIZE (MAX_VECTORS * 4 * 4)

#define CBF0_TAG MKBETAG('C', 'B', 'F', '0')
#define CBFZ_TAG MKBETAG('C', 'B', 'F', 'Z')
#define CBP0_TAG MKBETAG('C', 'B', 'P', '0')
#define CBPZ_TAG MKBETAG('C', 'B', 'P', 'Z')
#define CPL0_TAG MKBETAG('C', 'P', 'L', '0')
#define CPLZ_TAG MKBETAG('C', 'P', 'L', 'Z')
#define VPTZ_TAG MKBETAG('V', 'P', 'T', 'Z')

typedef struct VqaContext {

    AVCodecContext *avctx;
    GetByteContext gb;

    uint32_t palette[PALETTE_COUNT];

    int width;   /* width of a frame */
    int height;   /* height of a frame */
    int vector_width;  /* width of individual vector */
    int vector_height;  /* height of individual vector */
    int vqa_version;  /* this should be either 1, 2 or 3 */

    unsigned char *codebook;         /* the current codebook */
    int codebook_size;
    unsigned char *next_codebook_buffer;  /* accumulator for next codebook */
    int next_codebook_buffer_index;

    unsigned char *decode_buffer;
    int decode_buffer_size;

    /* number of frames to go before replacing codebook */
    int partial_countdown;
    int partial_count;

} VqaContext;

{


    /* make sure the extradata made it */
        av_log(s->avctx, AV_LOG_ERROR, "expected extradata size of %d\n", VQA_HEADER_SIZE);
        return AVERROR(EINVAL);
    }

    /* load up the VQA parameters from the header */
    case 1:
    case 2:
    case 3:
        avpriv_report_missing_feature(avctx, "VQA Version %d", s->vqa_version);
        return AVERROR_PATCHWELCOME;
    default:
        avpriv_request_sample(avctx, "VQA Version %i", s->vqa_version);
        return AVERROR_PATCHWELCOME;
    }
        s->width= s->height= 0;
        return ret;
    }

    /* the vector dimensions have to meet very stringent requirements */
        /* return without further initialization */
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "Image size not multiple of block size\n");
        return AVERROR_INVALIDDATA;
    }

    /* allocate codebooks */
        goto fail;
        goto fail;

    /* allocate decode buffer */
        goto fail;

    /* initialize the solid-color vectors */
        codebook_index = 0xFF00 * 16;
        for (i = 0; i < 256; i++)
            for (j = 0; j < 16; j++)
                s->codebook[codebook_index++] = i;
    } else {
        codebook_index = 0xF00 * 8;
    }

fail:
    av_freep(&s->codebook);
    av_freep(&s->next_codebook_buffer);
    av_freep(&s->decode_buffer);
    return AVERROR(ENOMEM);
}

#define CHECK_COUNT() \
    if (dest_index + count > dest_size) { \
        av_log(s->avctx, AV_LOG_ERROR, "decode_format80 problem: next op would overflow dest_index\n"); \
        av_log(s->avctx, AV_LOG_ERROR, "current dest_index = %d, count = %d, dest_size = %d\n", \
            dest_index, count, dest_size); \
        return AVERROR_INVALIDDATA; \
    }

#define CHECK_COPY(idx) \
    if (idx < 0 || idx + count > dest_size) { \
        av_log(s->avctx, AV_LOG_ERROR, "decode_format80 problem: next op would overflow dest_index\n"); \
        av_log(s->avctx, AV_LOG_ERROR, "current src_pos = %d, count = %d, dest_size = %d\n", \
            src_pos, count, dest_size); \
        return AVERROR_INVALIDDATA; \
    }


    unsigned char *dest, int dest_size, int check_size) {


        av_log(s->avctx, AV_LOG_ERROR, "Chunk size %d is out of range\n",
               src_size);
        return AVERROR_INVALIDDATA;
    }


        /* 0x80 means that frame is finished */
            break;

            av_log(s->avctx, AV_LOG_ERROR, "decode_format80 problem: dest_index (%d) exceeded dest_size (%d)\n",
                dest_index, dest_size);
            return AVERROR_INVALIDDATA;
        }


            dest_index += count;




            dest_index += count;



        } else {

            dest_index += count;
        }
    }

    /* validate that the entire destination buffer was filled; this is
     * important for decoding frame maps since each vector needs to have a
     * codebook entry; it is not important for compressed codebooks because
     * not every entry needs to be filled */
            av_log(s->avctx, AV_LOG_ERROR, "decode_format80 problem: decode finished with dest_index (%d) < dest_size (%d)\n",
                dest_index, dest_size);
            memset(dest + dest_index, 0, dest_size - dest_index);
        }

    return 0; // let's display what we decoded anyway
}

static int vqa_decode_chunk(VqaContext *s, AVFrame *frame)
{
    unsigned int chunk_type;
    unsigned int chunk_size;
    int byte_skip;
    unsigned int index = 0;
    int i;
    unsigned char r, g, b;
    int index_shift;
    int res;

    int cbf0_chunk = -1;
    int cbfz_chunk = -1;
    int cbp0_chunk = -1;
    int cbpz_chunk = -1;
    int cpl0_chunk = -1;
    int cplz_chunk = -1;
    int vptz_chunk = -1;

    int x, y;
    int lines = 0;
    int pixel_ptr;
    int vector_index = 0;
    int lobyte = 0;
    int hibyte = 0;
    int lobytes = 0;
    int hibytes = s->decode_buffer_size / 2;

    /* first, traverse through the frame and find the subchunks */
    while (bytestream2_get_bytes_left(&s->gb) >= 8) {

        chunk_type = bytestream2_get_be32u(&s->gb);
        index      = bytestream2_tell(&s->gb);
        chunk_size = bytestream2_get_be32u(&s->gb);

        switch (chunk_type) {

        case CBF0_TAG:
            cbf0_chunk = index;
            break;

        case CBFZ_TAG:
            cbfz_chunk = index;
            break;

        case CBP0_TAG:
            cbp0_chunk = index;
            break;

        case CBPZ_TAG:
            cbpz_chunk = index;
            break;

        case CPL0_TAG:
            cpl0_chunk = index;
            break;

        case CPLZ_TAG:
            cplz_chunk = index;
            break;

        case VPTZ_TAG:
            vptz_chunk = index;
            break;

        default:
            av_log(s->avctx, AV_LOG_ERROR, "Found unknown chunk type: %s (%08X)\n",
                   av_fourcc2str(av_bswap32(chunk_type)), chunk_type);
            break;
        }

        byte_skip = chunk_size & 0x01;
        bytestream2_skip(&s->gb, chunk_size + byte_skip);
    }

    /* next, deal with the palette */
    if ((cpl0_chunk != -1) && (cplz_chunk != -1)) {

        /* a chunk should not have both chunk types */
        av_log(s->avctx, AV_LOG_ERROR, "problem: found both CPL0 and CPLZ chunks\n");
        return AVERROR_INVALIDDATA;
    }

    /* decompress the palette chunk */
    if (cplz_chunk != -1) {

/* yet to be handled */

    }

    /* convert the RGB palette into the machine's endian format */
    if (cpl0_chunk != -1) {

        bytestream2_seek(&s->gb, cpl0_chunk, SEEK_SET);
        chunk_size = bytestream2_get_be32(&s->gb);
        /* sanity check the palette size */
        if (chunk_size / 3 > 256 || chunk_size > bytestream2_get_bytes_left(&s->gb)) {
            av_log(s->avctx, AV_LOG_ERROR, "problem: found a palette chunk with %d colors\n",
                chunk_size / 3);
            return AVERROR_INVALIDDATA;
        }
        for (i = 0; i < chunk_size / 3; i++) {
            /* scale by 4 to transform 6-bit palette -> 8-bit */
            r = bytestream2_get_byteu(&s->gb) * 4;
            g = bytestream2_get_byteu(&s->gb) * 4;
            b = bytestream2_get_byteu(&s->gb) * 4;
            s->palette[i] = 0xFFU << 24 | r << 16 | g << 8 | b;
            s->palette[i] |= s->palette[i] >> 6 & 0x30303;
        }
    }

    /* next, look for a full codebook */
    if ((cbf0_chunk != -1) && (cbfz_chunk != -1)) {

        /* a chunk should not have both chunk types */
        av_log(s->avctx, AV_LOG_ERROR, "problem: found both CBF0 and CBFZ chunks\n");
        return AVERROR_INVALIDDATA;
    }

    /* decompress the full codebook chunk */
    if (cbfz_chunk != -1) {

        bytestream2_seek(&s->gb, cbfz_chunk, SEEK_SET);
        chunk_size = bytestream2_get_be32(&s->gb);
        if ((res = decode_format80(s, chunk_size, s->codebook,
                                   s->codebook_size, 0)) < 0)
            return res;
    }

    /* copy a full codebook */
    if (cbf0_chunk != -1) {

        bytestream2_seek(&s->gb, cbf0_chunk, SEEK_SET);
        chunk_size = bytestream2_get_be32(&s->gb);
        /* sanity check the full codebook size */
        if (chunk_size > MAX_CODEBOOK_SIZE) {
            av_log(s->avctx, AV_LOG_ERROR, "problem: CBF0 chunk too large (0x%X bytes)\n",
                chunk_size);
            return AVERROR_INVALIDDATA;
        }

        bytestream2_get_buffer(&s->gb, s->codebook, chunk_size);
    }

    /* decode the frame */
    if (vptz_chunk == -1) {

        /* something is wrong if there is no VPTZ chunk */
        av_log(s->avctx, AV_LOG_ERROR, "problem: no VPTZ chunk found\n");
        return AVERROR_INVALIDDATA;
    }

    bytestream2_seek(&s->gb, vptz_chunk, SEEK_SET);
    chunk_size = bytestream2_get_be32(&s->gb);
    if ((res = decode_format80(s, chunk_size,
                               s->decode_buffer, s->decode_buffer_size, 1)) < 0)
        return res;

    /* render the final PAL8 frame */
    if (s->vector_height == 4)
        index_shift = 4;
    else
        index_shift = 3;
    for (y = 0; y < s->height; y += s->vector_height) {
        for (x = 0; x < s->width; x += 4, lobytes++, hibytes++) {
            pixel_ptr = y * frame->linesize[0] + x;

            /* get the vector index, the method for which varies according to
             * VQA file version */
            switch (s->vqa_version) {

            case 1:
                lobyte = s->decode_buffer[lobytes * 2];
                hibyte = s->decode_buffer[(lobytes * 2) + 1];
                vector_index = ((hibyte << 8) | lobyte) >> 3;
                vector_index <<= index_shift;
                lines = s->vector_height;
                /* uniform color fill - a quick hack */
                if (hibyte == 0xFF) {
                    while (lines--) {
                        frame->data[0][pixel_ptr + 0] = 255 - lobyte;
                        frame->data[0][pixel_ptr + 1] = 255 - lobyte;
                        frame->data[0][pixel_ptr + 2] = 255 - lobyte;
                        frame->data[0][pixel_ptr + 3] = 255 - lobyte;
                        pixel_ptr += frame->linesize[0];
                    }
                    lines=0;
                }
                break;

            case 2:
                lobyte = s->decode_buffer[lobytes];
                hibyte = s->decode_buffer[hibytes];
                vector_index = (hibyte << 8) | lobyte;
                vector_index <<= index_shift;
                lines = s->vector_height;
                break;

            case 3:
/* not implemented yet */
                lines = 0;
                break;
            }

            while (lines--) {
                frame->data[0][pixel_ptr + 0] = s->codebook[vector_index++];
                frame->data[0][pixel_ptr + 1] = s->codebook[vector_index++];
                frame->data[0][pixel_ptr + 2] = s->codebook[vector_index++];
                frame->data[0][pixel_ptr + 3] = s->codebook[vector_index++];
                pixel_ptr += frame->linesize[0];
            }
        }
    }

    /* handle partial codebook */
    if ((cbp0_chunk != -1) && (cbpz_chunk != -1)) {
        /* a chunk should not have both chunk types */
        av_log(s->avctx, AV_LOG_ERROR, "problem: found both CBP0 and CBPZ chunks\n");
        return AVERROR_INVALIDDATA;
    }

    if (cbp0_chunk != -1) {

        bytestream2_seek(&s->gb, cbp0_chunk, SEEK_SET);
        chunk_size = bytestream2_get_be32(&s->gb);

        if (chunk_size > MAX_CODEBOOK_SIZE - s->next_codebook_buffer_index) {
            av_log(s->avctx, AV_LOG_ERROR, "cbp0 chunk too large (%u bytes)\n",
                   chunk_size);
            return AVERROR_INVALIDDATA;
        }

        /* accumulate partial codebook */
        bytestream2_get_buffer(&s->gb, &s->next_codebook_buffer[s->next_codebook_buffer_index],
                               chunk_size);
        s->next_codebook_buffer_index += chunk_size;

        s->partial_countdown--;
        if (s->partial_countdown <= 0) {

            /* time to replace codebook */
            memcpy(s->codebook, s->next_codebook_buffer,
                s->next_codebook_buffer_index);

            /* reset accounting */
            s->next_codebook_buffer_index = 0;
            s->partial_countdown = s->partial_count;
        }
    }

    if (cbpz_chunk != -1) {

        bytestream2_seek(&s->gb, cbpz_chunk, SEEK_SET);
        chunk_size = bytestream2_get_be32(&s->gb);

        if (chunk_size > MAX_CODEBOOK_SIZE - s->next_codebook_buffer_index) {
            av_log(s->avctx, AV_LOG_ERROR, "cbpz chunk too large (%u bytes)\n",
                   chunk_size);
            return AVERROR_INVALIDDATA;
        }

        /* accumulate partial codebook */
        bytestream2_get_buffer(&s->gb, &s->next_codebook_buffer[s->next_codebook_buffer_index],
                               chunk_size);
        s->next_codebook_buffer_index += chunk_size;

        s->partial_countdown--;
        if (s->partial_countdown <= 0) {
            bytestream2_init(&s->gb, s->next_codebook_buffer, s->next_codebook_buffer_index);
            /* decompress codebook */
            if ((res = decode_format80(s, s->next_codebook_buffer_index,
                                       s->codebook, s->codebook_size, 0)) < 0)
                return res;

            /* reset accounting */
            s->next_codebook_buffer_index = 0;
            s->partial_countdown = s->partial_count;
        }
    }

    return 0;
}

                            void *data, int *got_frame,
                            AVPacket *avpkt)
{

        return res;

        return res;

    /* make the palette available on the way out */


    /* report that the buffer was completely consumed */
}

{


}

static const AVCodecDefault vqa_defaults[] = {
    { "max_pixels", "320*240" },
    { NULL },
};

AVCodec ff_vqa_decoder = {
    .name           = "vqavideo",
    .long_name      = NULL_IF_CONFIG_SMALL("Westwood Studios VQA (Vector Quantized Animation) video"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_WS_VQA,
    .priv_data_size = sizeof(VqaContext),
    .init           = vqa_decode_init,
    .close          = vqa_decode_end,
    .decode         = vqa_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
    .defaults       = vqa_defaults,
};
