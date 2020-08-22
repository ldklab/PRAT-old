/*
 * Vidvox Hap decoder
 * Copyright (C) 2015 Vittorio Giovara <vittorio.giovara@gmail.com>
 * Copyright (C) 2015 Tom Butterworth <bangnoise@gmail.com>
 *
 * HapQA and HAPAlphaOnly added by Jokyo Images
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
 * Hap decoder
 *
 * Fourcc: Hap1, Hap5, HapY, HapA, HapM
 *
 * https://github.com/Vidvox/hap/blob/master/documentation/HapVideoDRAFT.md
 */

#include <stdint.h>

#include "libavutil/imgutils.h"

#include "avcodec.h"
#include "bytestream.h"
#include "hap.h"
#include "internal.h"
#include "snappy.h"
#include "texturedsp.h"
#include "thread.h"

{

            return ret;


                    return ret;
                }
                had_compressors = 1;
                is_first_table = 0;
                break;
                    return ret;
                }
                had_sizes = 1;
                is_first_table = 0;
                break;
            case HAP_ST_OFFSET_TABLE:
                ret = ff_hap_set_chunk_count(ctx, section_size / 4, is_first_table);
                if (ret != 0)
                    return ret;
                for (i = 0; i < section_size / 4; i++) {
                    ctx->chunks[i].compressed_offset = bytestream2_get_le32(gbc);
                }
                had_offsets = 1;
                is_first_table = 0;
                break;
            default:
                break;
        }
    }

        return AVERROR_INVALIDDATA;

    /* The offsets table is optional. If not present than calculate offsets by
     * summing the sizes of preceding chunks. */
        size_t running_size = 0;
        }
    }

    return 0;
}

{
    int i;
    size_t running_offset = 0;
            return 0;
    }
    return 1;
}

{

        return ret;

                                                        (section_type & 0x0F) != HAP_FMT_YCOCGDXT5)) {
        av_log(avctx, AV_LOG_ERROR,
               "Invalid texture format %#04x.\n", section_type & 0x0F);
        return AVERROR_INVALIDDATA;
    }

        case HAP_COMP_SNAPPY:
            }
                compressorstr = "none";
            } else {
            }
            break;
                ret = AVERROR_INVALIDDATA;
            compressorstr = "complex";
            break;
        default:
            ret = AVERROR_INVALIDDATA;
            break;
    }

        return ret;

    /* Check the frame is valid and read the uncompressed chunk sizes */

        /* Check the compressed buffer is valid */
            return AVERROR_INVALIDDATA;

        /* Chunks are unpacked sequentially, ctx->tex_size is the uncompressed
         * size thus far */

        /* Fill out uncompressed size */
                             chunk->compressed_size);
                return uncompressed_size;
            }
        } else {
            return AVERROR_INVALIDDATA;
        }
    }


}

                                    int chunk_nb, int thread_nb)
{




        /* Uncompress the frame */
             av_log(avctx, AV_LOG_ERROR, "Snappy uncompress error\n");
             return ret;
        }
    } else if (chunk->compressor == HAP_COMP_NONE) {
        bytestream2_get_buffer(&gbc, dst, chunk->compressed_size);
    }

    return 0;
}

static int decompress_texture_thread_internal(AVCodecContext *avctx, void *arg,
                                              int slice, int thread_nb, int texture_num)
{
    HapContext *ctx = avctx->priv_data;
    AVFrame *frame = arg;
    const uint8_t *d = ctx->tex_data;
    int w_block = avctx->coded_width / TEXTURE_BLOCK_W;
    int h_block = avctx->coded_height / TEXTURE_BLOCK_H;
    int x, y;
    int start_slice, end_slice;
    int base_blocks_per_slice = h_block / ctx->slice_count;
    int remainder_blocks = h_block % ctx->slice_count;

    /* When the frame height (in blocks) doesn't divide evenly between the
     * number of slices, spread the remaining blocks evenly between the first
     * operations */
    start_slice = slice * base_blocks_per_slice;
    /* Add any extra blocks (one per slice) that have been added before this slice */
    start_slice += FFMIN(slice, remainder_blocks);

    end_slice = start_slice + base_blocks_per_slice;
    /* Add an extra block if there are still remainder blocks to be accounted for */
    if (slice < remainder_blocks)
        end_slice++;

    for (y = start_slice; y < end_slice; y++) {
        uint8_t *p = frame->data[0] + y * frame->linesize[0] * TEXTURE_BLOCK_H;
        int off  = y * w_block;
        for (x = 0; x < w_block; x++) {
            if (texture_num == 0) {
                ctx->tex_fun(p + x * 4 * ctx->uncompress_pix_size, frame->linesize[0],
                             d + (off + x) * ctx->tex_rat);
            } else {
                ctx->tex_fun2(p + x * 4 * ctx->uncompress_pix_size, frame->linesize[0],
                              d + (off + x) * ctx->tex_rat2);
            }
        }
    }

    return 0;
}

                                     int slice, int thread_nb)
{
}

                                      int slice, int thread_nb)
{
}

                      int *got_frame, AVPacket *avpkt)
{



    /* check for multi texture header */
            return ret;
            av_log(avctx, AV_LOG_ERROR, "Invalid section type in 2 textures mode %#04x.\n", section_type);
            return AVERROR_INVALIDDATA;
        }
    }

    /* Get the output frame ready to receive data */
        return ret;


        /* Check for section header */
            return ret;

            av_log(avctx, AV_LOG_ERROR, "uncompressed size mismatches\n");
            return AVERROR_INVALIDDATA;
        }


            ff_thread_finish_setup(avctx);

        /* Unpack the DXT texture */
            /* Only DXTC texture compression in a contiguous block */
                av_log(avctx, AV_LOG_ERROR, "Insufficient data\n");
                return AVERROR_INVALIDDATA;
            }
        } else {
            /* Perform the second-stage decompression */
                return ret;

                            ctx->chunk_results, ctx->chunk_count);

                    return ctx->chunk_results[i];
            }

        }

        /* Use the decompress function on the texture, one block per thread */
        } else{
        }
    }

    /* Frame is ready to be output */

}

{

        av_log(avctx, AV_LOG_ERROR, "Invalid video size %dx%d.\n",
               avctx->width, avctx->height);
        return ret;
    }

    /* Since codec is based on 4x4 blocks, size is aligned to 4 */



    default:
        return AVERROR_DECODER_NOT_FOUND;
    }



}

{


}

AVCodec ff_hap_decoder = {
    .name           = "hap",
    .long_name      = NULL_IF_CONFIG_SMALL("Vidvox Hap"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_HAP,
    .init           = hap_init,
    .decode         = hap_decode,
    .close          = hap_close,
    .priv_data_size = sizeof(HapContext),
    .capabilities   = AV_CODEC_CAP_FRAME_THREADS | AV_CODEC_CAP_SLICE_THREADS |
                      AV_CODEC_CAP_DR1,
    .caps_internal  = FF_CODEC_CAP_INIT_THREADSAFE |
                      FF_CODEC_CAP_INIT_CLEANUP,
    .codec_tags     = (const uint32_t []){
        MKTAG('H','a','p','1'),
        MKTAG('H','a','p','5'),
        MKTAG('H','a','p','Y'),
        MKTAG('H','a','p','A'),
        MKTAG('H','a','p','M'),
        FF_CODEC_TAGS_END,
    },
};
