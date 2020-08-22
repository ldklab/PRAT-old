/*
 * Mpeg video formats-related picture management functions
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

#include <stdint.h>

#include "libavutil/avassert.h"
#include "libavutil/common.h"
#include "libavutil/pixdesc.h"
#include "libavutil/imgutils.h"

#include "avcodec.h"
#include "motion_est.h"
#include "mpegpicture.h"
#include "mpegutils.h"

{
#define MAKE_WRITABLE(table) \
do {\
    if (pic->table &&\
       (ret = av_buffer_make_writable(&pic->table)) < 0)\
    return ret;\
} while (0)


    }

    return 0;
}

                            ScratchpadContext *sc, int linesize)
{
#   define EMU_EDGE_HEIGHT (4 * 70)

        return 0;

        av_log(avctx, AV_LOG_ERROR, "Image too small, temporary buffers cannot function\n");
        return AVERROR_PATCHWELCOME;
    }

        return AVERROR(ENOMEM);

    // edge emu needs blocksize + filter length - 1
    // (= 17x17 for  halfpel / 21x21 for H.264)
    // VC-1 computes luma and chroma simultaneously and needs 19X19 + 9x9
    // at uvlinesize. It supports only YUV420 so 24x24 is enough
    // linesize * interlaced * MBsize
    // we also use this buffer for encoding in encode_mb_internal() needig an additional 32 lines
        return AVERROR(ENOMEM);

}

/**
 * Allocate a frame buffer
 */
                              MotionEstContext *me, ScratchpadContext *sc,
                              int chroma_x_shift, int chroma_y_shift,
                              int linesize, int uvlinesize)
{

        avctx->codec_id != AV_CODEC_ID_MSS2) {
        }

    } else {
    }

        av_log(avctx, AV_LOG_ERROR, "get_buffer() failed (%d %p)\n",
               r, pic->f->data[0]);
        return -1;
    }

        int i;
        }
    }

        assert(!pic->hwaccel_picture_private);
        if (avctx->hwaccel->frame_priv_data_size) {
            pic->hwaccel_priv_buf = av_buffer_allocz(avctx->hwaccel->frame_priv_data_size);
            if (!pic->hwaccel_priv_buf) {
                av_log(avctx, AV_LOG_ERROR, "alloc_frame_buffer() failed (hwaccel private data allocation)\n");
                return -1;
            }
            pic->hwaccel_picture_private = pic->hwaccel_priv_buf->data;
        }
    }

        av_log(avctx, AV_LOG_ERROR,
               "get_buffer() failed (stride changed: linesize=%d/%d uvlinesize=%d/%d)\n",
               linesize,   pic->f->linesize[0],
               uvlinesize, pic->f->linesize[1]);
        ff_mpeg_unref_picture(avctx, pic);
        return -1;
    }

        av_log(avctx, AV_LOG_ERROR,
               "get_buffer() failed (uv stride mismatch)\n");
        ff_mpeg_unref_picture(avctx, pic);
        return -1;
    }

        av_log(avctx, AV_LOG_ERROR,
               "get_buffer() failed to allocate context scratch buffers.\n");
        ff_mpeg_unref_picture(avctx, pic);
        return ret;
    }

    return 0;
}

static int alloc_picture_tables(AVCodecContext *avctx, Picture *pic, int encoding, int out_format,
                                int mb_stride, int mb_width, int mb_height, int b8_stride)
{
    const int big_mb_num    = mb_stride * (mb_height + 1) + 1;
    const int mb_array_size = mb_stride * mb_height;
    const int b8_array_size = b8_stride * mb_height * 2;
    int i;


    pic->mbskip_table_buf = av_buffer_allocz(mb_array_size + 2);
    pic->qscale_table_buf = av_buffer_allocz(big_mb_num + mb_stride);
    pic->mb_type_buf      = av_buffer_allocz((big_mb_num + mb_stride) *
                                             sizeof(uint32_t));
    if (!pic->mbskip_table_buf || !pic->qscale_table_buf || !pic->mb_type_buf)
        return AVERROR(ENOMEM);

    if (encoding) {
        pic->mb_var_buf    = av_buffer_allocz(mb_array_size * sizeof(int16_t));
        pic->mc_mb_var_buf = av_buffer_allocz(mb_array_size * sizeof(int16_t));
        pic->mb_mean_buf   = av_buffer_allocz(mb_array_size);
        if (!pic->mb_var_buf || !pic->mc_mb_var_buf || !pic->mb_mean_buf)
            return AVERROR(ENOMEM);
    }

    if (out_format == FMT_H263 || encoding ||
#if FF_API_DEBUG_MV
        avctx->debug_mv ||
#endif
        (avctx->export_side_data & AV_CODEC_EXPORT_DATA_MVS)) {
        int mv_size        = 2 * (b8_array_size + 4) * sizeof(int16_t);
        int ref_index_size = 4 * mb_array_size;

        for (i = 0; mv_size && i < 2; i++) {
            pic->motion_val_buf[i] = av_buffer_allocz(mv_size);
            pic->ref_index_buf[i]  = av_buffer_allocz(ref_index_size);
            if (!pic->motion_val_buf[i] || !pic->ref_index_buf[i])
                return AVERROR(ENOMEM);
        }
    }

    pic->alloc_mb_width  = mb_width;
    pic->alloc_mb_height = mb_height;

    return 0;
}

/**
 * Allocate a Picture.
 * The pixels are allocated/set by calling get_buffer() if shared = 0
 */
                     ScratchpadContext *sc, int shared, int encoding,
                     int chroma_x_shift, int chroma_y_shift, int out_format,
                     int mb_stride, int mb_width, int mb_height, int b8_stride,
                     ptrdiff_t *linesize, ptrdiff_t *uvlinesize)
{

            ff_free_picture_tables(pic);

    } else {
                               chroma_x_shift, chroma_y_shift,
            return -1;

    }

                                   mb_stride, mb_width, mb_height, b8_stride);
    else
        goto fail;

    }


        }
    }

    return 0;
fail:
    av_log(avctx, AV_LOG_ERROR, "Error allocating a picture.\n");
    ff_mpeg_unref_picture(avctx, pic);
    ff_free_picture_tables(pic);
    return AVERROR(ENOMEM);
}

/**
 * Deallocate a picture.
 */
{

    /* WM Image / Screen codecs allocate internal buffers with different
     * dimensions / colorspaces; ignore user-defined callbacks for these. */
        avctx->codec_id != AV_CODEC_ID_MSS2)




{

#define UPDATE_TABLE(table)                                                   \
do {                                                                          \
    if (src->table &&                                                         \
        (!dst->table || dst->table->buffer != src->table->buffer)) {          \
        av_buffer_unref(&dst->table);                                         \
        dst->table = av_buffer_ref(src->table);                               \
        if (!dst->table) {                                                    \
            ff_free_picture_tables(dst);                                      \
            return AVERROR(ENOMEM);                                           \
        }                                                                     \
    }                                                                         \
} while (0)

    }

    }


}

{


        goto fail;

        goto fail;

        dst->hwaccel_priv_buf = av_buffer_ref(src->hwaccel_priv_buf);
        if (!dst->hwaccel_priv_buf) {
            ret = AVERROR(ENOMEM);
            goto fail;
        }
        dst->hwaccel_picture_private = dst->hwaccel_priv_buf->data;
    }


           sizeof(dst->encoding_error));

fail:
    ff_mpeg_unref_picture(avctx, dst);
    return ret;
}

{
        return 1;
        return 1;
    return 0;
}

{

        }
    } else {
        }
    }

    av_log(avctx, AV_LOG_FATAL,
           "Internal error, picture buffer overflow\n");
    /* We could return -1, but the codec would crash trying to draw into a
     * non-existing frame anyway. This is safer than waiting for a random crash.
     * Also the return of this is never useful, an encoder must only allocate
     * as much as allowed in the specification. This has no relationship to how
     * much libavcodec could allocate (and MAX_PICTURE_COUNT is always large
     * enough for such valid streams).
     * Plus, a decoder has to check stream validity and remove frames if too
     * many reference frames are around. Waiting for "OOM" is not correct at
     * all. Similarly, missing reference frames have to be replaced by
     * interpolated/MC frames, anything else is a bug in the codec ...
     */
    abort();
    return -1;
}

{

        }
    }
}

{



    }
