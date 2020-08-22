/*
 * PGS subtitle decoder
 * Copyright (c) 2009 Stephen Backway
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
 * PGS subtitle decoder
 */

#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"
#include "mathops.h"

#include "libavutil/colorspace.h"
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"

#define RGBA(r,g,b,a) (((unsigned)(a) << 24) | ((r) << 16) | ((g) << 8) | (b))
#define MAX_EPOCH_PALETTES 8   // Max 8 allowed per PGS epoch
#define MAX_EPOCH_OBJECTS  64  // Max 64 allowed per PGS epoch
#define MAX_OBJECT_REFS    2   // Max objects per display set

enum SegmentType {
    PALETTE_SEGMENT      = 0x14,
    OBJECT_SEGMENT       = 0x15,
    PRESENTATION_SEGMENT = 0x16,
    WINDOW_SEGMENT       = 0x17,
    DISPLAY_SEGMENT      = 0x80,
};

typedef struct PGSSubObjectRef {
    int     id;
    int     window_id;
    uint8_t composition_flag;
    int     x;
    int     y;
    int     crop_x;
    int     crop_y;
    int     crop_w;
    int     crop_h;
} PGSSubObjectRef;

typedef struct PGSSubPresentation {
    int id_number;
    int palette_id;
    int object_count;
    PGSSubObjectRef objects[MAX_OBJECT_REFS];
    int64_t pts;
} PGSSubPresentation;

typedef struct PGSSubObject {
    int          id;
    int          w;
    int          h;
    uint8_t      *rle;
    unsigned int rle_buffer_size, rle_data_len;
    unsigned int rle_remaining_len;
} PGSSubObject;

typedef struct PGSSubObjects {
    int          count;
    PGSSubObject object[MAX_EPOCH_OBJECTS];
} PGSSubObjects;

typedef struct PGSSubPalette {
    int         id;
    uint32_t    clut[256];
} PGSSubPalette;

typedef struct PGSSubPalettes {
    int           count;
    PGSSubPalette palette[MAX_EPOCH_PALETTES];
} PGSSubPalettes;

typedef struct PGSSubContext {
    AVClass *class;
    PGSSubPresentation presentation;
    PGSSubPalettes     palettes;
    PGSSubObjects      objects;
    int forced_subs_only;
} PGSSubContext;

{

    }

{
    int i;

            return &objects->object[i];
    }
    return NULL;
}

{
    int i;

        if (palettes->palette[i].id == id)
            return &palettes->palette[i];
    }
    return NULL;
}

{

}

{

}

/**
 * Decode the RLE data.
 *
 * The subtitle is stored as a Run Length Encoded image.
 *
 * @param avctx contains the current codec context
 * @param sub pointer to the processed subtitle data
 * @param buf pointer to the RLE data to process
 * @param buf_size size of the RLE data to process
 */
                      const uint8_t *buf, unsigned int buf_size)
{



        return AVERROR(ENOMEM);

    pixel_count = 0;
    line_count  = 0;



        }

            /*
             * New Line. Check if correct pixels decoded, if not display warning
             * and adjust bitmap pointer to correct new line position.
             */
                av_log(avctx, AV_LOG_ERROR, "Decoded %d pixels, when line should be %d pixels\n",
                       pixel_count % rect->w, rect->w);
                if (avctx->err_recognition & AV_EF_EXPLODE) {
                    return AVERROR_INVALIDDATA;
                }
            }
        }
    }

        av_log(avctx, AV_LOG_ERROR, "Insufficient RLE data for subtitle\n");
        return AVERROR_INVALIDDATA;
    }

    ff_dlog(avctx, "Pixel Count = %d, Area = %d\n", pixel_count, rect->w * rect->h);

    return 0;
}

/**
 * Parse the picture segment packet.
 *
 * The picture segment contains details on the sequence id,
 * width, height and Run Length Encoded (RLE) bitmap data.
 *
 * @param avctx contains the current codec context
 * @param buf pointer to the packet to process
 * @param buf_size size of packet to process
 */
                                  const uint8_t *buf, int buf_size)
{


        return AVERROR_INVALIDDATA;

            av_log(avctx, AV_LOG_ERROR, "Too many objects in epoch\n");
            return AVERROR_INVALIDDATA;
        }
    }

    /* skip object version number */

    /* Read the Sequence Description to determine if start of RLE data or appended to previous RLE */

        /* Additional RLE data */
        if (buf_size > object->rle_remaining_len)
            return AVERROR_INVALIDDATA;

        memcpy(object->rle + object->rle_data_len, buf, buf_size);
        object->rle_data_len += buf_size;
        object->rle_remaining_len -= buf_size;

        return 0;
    }

        return AVERROR_INVALIDDATA;

    /* Decode rle bitmap length, stored size includes width/height data */

        av_log(avctx, AV_LOG_ERROR,
               "Buffer dimension %d larger than the expected RLE data %d\n",
               buf_size, rle_bitmap_len);
        return AVERROR_INVALIDDATA;
    }

    /* Get bitmap dimensions from data */

    /* Make sure the bitmap is not too large */
        av_log(avctx, AV_LOG_ERROR, "Bitmap dimensions (%dx%d) invalid.\n", width, height);
        return AVERROR_INVALIDDATA;
    }



        object->rle_data_len = 0;
        object->rle_remaining_len = 0;
        return AVERROR(ENOMEM);
    }


}

/**
 * Parse the palette segment packet.
 *
 * The palette segment contains details of the palette,
 * a maximum of 256 colors can be defined.
 *
 * @param avctx contains the current codec context
 * @param buf pointer to the packet to process
 * @param buf_size size of packet to process
 */
                                  const uint8_t *buf, int buf_size)
{


            av_log(avctx, AV_LOG_ERROR, "Too many palettes in epoch\n");
            return AVERROR_INVALIDDATA;
        }
    }

    /* Skip palette version */


        /* Default to BT.709 colorspace. In case of <= 576 height use BT.601 */
        } else {
            YUV_TO_RGB1_CCIR(cb, cr);
        }



        /* Store color in palette */
    }
    return 0;
}

/**
 * Parse the presentation segment packet.
 *
 * The presentation segment contains details on the video
 * width, video height, x & y subtitle position.
 *
 * @param avctx contains the current codec context
 * @param buf pointer to the packet to process
 * @param buf_size size of packet to process
 * @todo TODO: Implement cropping
 */
                                      const uint8_t *buf, int buf_size,
                                      int64_t pts)
{

    // Video descriptor


            w, h);
        return ret;

    /* Skip 1 bytes of unknown, frame rate */

    // Composition descriptor
    /*
     * state is a 2 bit field that defines pgs epoch boundaries
     * 00 - Normal, previously defined objects and palettes are still valid
     * 01 - Acquisition point, previous objects and palettes can be released
     * 10 - Epoch start, previous objects and palettes can be released
     * 11 - Epoch continue, previous objects and palettes can be released
     *
     * reserved 6 bits discarded
     */
    }

    /*
     * skip palette_update_flag (0x80),
     */
        av_log(avctx, AV_LOG_ERROR,
               "Invalid number of presentation objects %d\n",
               ctx->presentation.object_count);
        ctx->presentation.object_count = 2;
        if (avctx->err_recognition & AV_EF_EXPLODE) {
            return AVERROR_INVALIDDATA;
        }
    }


    {

            av_log(avctx, AV_LOG_ERROR, "Insufficent space for object\n");
            ctx->presentation.object_count = i;
            return AVERROR_INVALIDDATA;
        }



        // If cropping
            ctx->presentation.objects[i].crop_x = bytestream_get_be16(&buf);
            ctx->presentation.objects[i].crop_y = bytestream_get_be16(&buf);
            ctx->presentation.objects[i].crop_w = bytestream_get_be16(&buf);
            ctx->presentation.objects[i].crop_h = bytestream_get_be16(&buf);
        }

                ctx->presentation.objects[i].x, ctx->presentation.objects[i].y);

            av_log(avctx, AV_LOG_ERROR, "Subtitle out of video bounds. x = %d, y = %d, video width = %d, video height = %d.\n",
                   ctx->presentation.objects[i].x,
                   ctx->presentation.objects[i].y,
                    avctx->width, avctx->height);
            ctx->presentation.objects[i].x = 0;
            ctx->presentation.objects[i].y = 0;
            if (avctx->err_recognition & AV_EF_EXPLODE) {
                return AVERROR_INVALIDDATA;
            }
        }
    }

    return 0;
}

/**
 * Parse the display segment packet.
 *
 * The display segment controls the updating of the display.
 *
 * @param avctx contains the current codec context
 * @param data pointer to the data pertaining the subtitle to display
 * @param buf pointer to the packet to process
 * @param buf_size size of packet to process
 */
static int display_end_segment(AVCodecContext *avctx, void *data,
                               const uint8_t *buf, int buf_size)
{
    AVSubtitle    *sub = data;
    PGSSubContext *ctx = avctx->priv_data;
    int64_t pts;
    PGSSubPalette *palette;
    int i, ret;

    pts = ctx->presentation.pts != AV_NOPTS_VALUE ? ctx->presentation.pts : sub->pts;
    memset(sub, 0, sizeof(*sub));
    sub->pts = pts;
    ctx->presentation.pts = AV_NOPTS_VALUE;
    sub->start_display_time = 0;
    // There is no explicit end time for PGS subtitles.  The end time
    // is defined by the start of the next sub which may contain no
    // objects (i.e. clears the previous sub)
    sub->end_display_time   = UINT32_MAX;
    sub->format             = 0;

    // Blank if last object_count was 0.
    if (!ctx->presentation.object_count)
        return 1;
    sub->rects = av_mallocz_array(ctx->presentation.object_count, sizeof(*sub->rects));
    if (!sub->rects) {
        return AVERROR(ENOMEM);
    }
    palette = find_palette(ctx->presentation.palette_id, &ctx->palettes);
    if (!palette) {
        // Missing palette.  Should only happen with damaged streams.
        av_log(avctx, AV_LOG_ERROR, "Invalid palette id %d\n",
               ctx->presentation.palette_id);
        avsubtitle_free(sub);
        return AVERROR_INVALIDDATA;
    }
    for (i = 0; i < ctx->presentation.object_count; i++) {
        PGSSubObject *object;

        sub->rects[i]  = av_mallocz(sizeof(*sub->rects[0]));
        if (!sub->rects[i]) {
            avsubtitle_free(sub);
            return AVERROR(ENOMEM);
        }
        sub->num_rects++;
        sub->rects[i]->type = SUBTITLE_BITMAP;

        /* Process bitmap */
        object = find_object(ctx->presentation.objects[i].id, &ctx->objects);
        if (!object) {
            // Missing object.  Should only happen with damaged streams.
            av_log(avctx, AV_LOG_ERROR, "Invalid object id %d\n",
                   ctx->presentation.objects[i].id);
            if (avctx->err_recognition & AV_EF_EXPLODE) {
                avsubtitle_free(sub);
                return AVERROR_INVALIDDATA;
            }
            // Leaves rect empty with 0 width and height.
            continue;
        }
        if (ctx->presentation.objects[i].composition_flag & 0x40)
            sub->rects[i]->flags |= AV_SUBTITLE_FLAG_FORCED;

        sub->rects[i]->x    = ctx->presentation.objects[i].x;
        sub->rects[i]->y    = ctx->presentation.objects[i].y;

        if (object->rle) {
            sub->rects[i]->w    = object->w;
            sub->rects[i]->h    = object->h;

            sub->rects[i]->linesize[0] = object->w;

            if (object->rle_remaining_len) {
                av_log(avctx, AV_LOG_ERROR, "RLE data length %u is %u bytes shorter than expected\n",
                       object->rle_data_len, object->rle_remaining_len);
                if (avctx->err_recognition & AV_EF_EXPLODE) {
                    avsubtitle_free(sub);
                    return AVERROR_INVALIDDATA;
                }
            }
            ret = decode_rle(avctx, sub->rects[i], object->rle, object->rle_data_len);
            if (ret < 0) {
                if ((avctx->err_recognition & AV_EF_EXPLODE) ||
                    ret == AVERROR(ENOMEM)) {
                    avsubtitle_free(sub);
                    return ret;
                }
                sub->rects[i]->w = 0;
                sub->rects[i]->h = 0;
                continue;
            }
        }
        /* Allocate memory for colors */
        sub->rects[i]->nb_colors    = 256;
        sub->rects[i]->data[1] = av_mallocz(AVPALETTE_SIZE);
        if (!sub->rects[i]->data[1]) {
            avsubtitle_free(sub);
            return AVERROR(ENOMEM);
        }

        if (!ctx->forced_subs_only || ctx->presentation.objects[i].composition_flag & 0x40)
        memcpy(sub->rects[i]->data[1], palette->clut, sub->rects[i]->nb_colors * sizeof(uint32_t));

#if FF_API_AVPICTURE
FF_DISABLE_DEPRECATION_WARNINGS
{
        AVSubtitleRect *rect;
        int j;
        rect = sub->rects[i];
        for (j = 0; j < 4; j++) {
            rect->pict.data[j] = rect->data[j];
            rect->pict.linesize[j] = rect->linesize[j];
        }
}
FF_ENABLE_DEPRECATION_WARNINGS
#endif
    }
    return 1;
}

                  AVPacket *avpkt)
{



        ff_dlog(avctx, "%02x ", buf[i]);
        if (i % 16 == 15)
            ff_dlog(avctx, "\n");
    }



    /* Ensure that we have received at a least a segment code and segment length */
        return -1;


    /* Step through buffer to identify segments */


            break;

        case WINDOW_SEGMENT:
            /*
             * Window Segment Structure (No new information provided):
             *     2 bytes: Unknown,
             *     2 bytes: X position of subtitle,
             *     2 bytes: Y position of subtitle,
             *     2 bytes: Width of subtitle,
             *     2 bytes: Height of subtitle.
             */
            break;
                av_log(avctx, AV_LOG_ERROR, "Duplicate display segment\n");
                ret = AVERROR_INVALIDDATA;
                break;
            }
            break;
        default:
            av_log(avctx, AV_LOG_ERROR, "Unknown subtitle segment type 0x%x, length %d\n",
                   segment_type, segment_length);
            ret = AVERROR_INVALIDDATA;
            break;
        }
            avsubtitle_free(data);
            *got_sub_ptr = 0;
            return ret;
        }

    }

    return buf_size;
}

#define OFFSET(x) offsetof(PGSSubContext, x)
#define SD AV_OPT_FLAG_SUBTITLE_PARAM | AV_OPT_FLAG_DECODING_PARAM
static const AVOption options[] = {
    {"forced_subs_only", "Only show forced subtitles", OFFSET(forced_subs_only), AV_OPT_TYPE_BOOL, {.i64 = 0}, 0, 1, SD},
    { NULL },
};

static const AVClass pgsdec_class = {
    .class_name = "PGS subtitle decoder",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
};

AVCodec ff_pgssub_decoder = {
    .name           = "pgssub",
    .long_name      = NULL_IF_CONFIG_SMALL("HDMV Presentation Graphic Stream subtitles"),
    .type           = AVMEDIA_TYPE_SUBTITLE,
    .id             = AV_CODEC_ID_HDMV_PGS_SUBTITLE,
    .priv_data_size = sizeof(PGSSubContext),
    .init           = init_decoder,
    .close          = close_decoder,
    .decode         = decode,
    .priv_class     = &pgsdec_class,
};
