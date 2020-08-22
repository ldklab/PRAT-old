/*
 * VP9 compatible video decoder
 *
 * Copyright (C) 2013 Ronald S. Bultje <rsbultje gmail com>
 * Copyright (C) 2013 Clément Bœsch <u pkh me>
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

#include "avcodec.h"
#include "get_bits.h"
#include "hwconfig.h"
#include "internal.h"
#include "profiles.h"
#include "thread.h"
#include "videodsp.h"
#include "vp56.h"
#include "vp9.h"
#include "vp9data.h"
#include "vp9dec.h"
#include "libavutil/avassert.h"
#include "libavutil/pixdesc.h"
#include "libavutil/video_enc_params.h"

#define VP9_SYNCCODE 0x498342

#if HAVE_THREADS
static void vp9_free_entries(AVCodecContext *avctx) {
    VP9Context *s = avctx->priv_data;

    if (avctx->active_thread_type & FF_THREAD_SLICE)  {
        pthread_mutex_destroy(&s->progress_mutex);
        pthread_cond_destroy(&s->progress_cond);
        av_freep(&s->entries);
    }
}

static int vp9_alloc_entries(AVCodecContext *avctx, int n) {
    VP9Context *s = avctx->priv_data;
    int i;

    if (avctx->active_thread_type & FF_THREAD_SLICE)  {
        if (s->entries)
            av_freep(&s->entries);

        s->entries = av_malloc_array(n, sizeof(atomic_int));

        if (!s->entries) {
            av_freep(&s->entries);
            return AVERROR(ENOMEM);
        }

        for (i  = 0; i < n; i++)
            atomic_init(&s->entries[i], 0);

        pthread_mutex_init(&s->progress_mutex, NULL);
        pthread_cond_init(&s->progress_cond, NULL);
    }
    return 0;
}

static void vp9_report_tile_progress(VP9Context *s, int field, int n) {
    pthread_mutex_lock(&s->progress_mutex);
    atomic_fetch_add_explicit(&s->entries[field], n, memory_order_release);
    pthread_cond_signal(&s->progress_cond);
    pthread_mutex_unlock(&s->progress_mutex);
}

static void vp9_await_tile_progress(VP9Context *s, int field, int n) {
    if (atomic_load_explicit(&s->entries[field], memory_order_acquire) >= n)
        return;

    pthread_mutex_lock(&s->progress_mutex);
    while (atomic_load_explicit(&s->entries[field], memory_order_relaxed) != n)
        pthread_cond_wait(&s->progress_cond, &s->progress_mutex);
    pthread_mutex_unlock(&s->progress_mutex);
}
#else
static void vp9_free_entries(AVCodecContext *avctx) {}
static int vp9_alloc_entries(AVCodecContext *avctx, int n) { return 0; }
#endif

{

{

{

        return ret;

            s->frame_extradata_pool_size = 0;
            goto fail;
        }
    }
        goto fail;
    }


        const AVHWAccel *hwaccel = avctx->hwaccel;
        av_assert0(!f->hwaccel_picture_private);
        if (hwaccel->frame_priv_data_size) {
            f->hwaccel_priv_buf = av_buffer_allocz(hwaccel->frame_priv_data_size);
            if (!f->hwaccel_priv_buf)
                goto fail;
            f->hwaccel_picture_private = f->hwaccel_priv_buf->data;
        }
    }

    return 0;

fail:
    vp9_frame_unref(avctx, f);
    return AVERROR(ENOMEM);
}

{

        return ret;

        goto fail;


        dst->hwaccel_priv_buf = av_buffer_ref(src->hwaccel_priv_buf);
        if (!dst->hwaccel_priv_buf)
            goto fail;
        dst->hwaccel_picture_private = dst->hwaccel_priv_buf->data;
    }

    return 0;

fail:
    vp9_frame_unref(avctx, dst);
    return AVERROR(ENOMEM);
}

{
#define HWACCEL_MAX (CONFIG_VP9_DXVA2_HWACCEL + \
                     CONFIG_VP9_D3D11VA_HWACCEL * 2 + \
                     CONFIG_VP9_NVDEC_HWACCEL + \
                     CONFIG_VP9_VAAPI_HWACCEL + \
                     CONFIG_VP9_VDPAU_HWACCEL)


            return ret;

        case AV_PIX_FMT_YUV420P:
#if CONFIG_VP9_VDPAU_HWACCEL
            *fmtp++ = AV_PIX_FMT_VDPAU;
#endif
        case AV_PIX_FMT_YUV420P10:
#if CONFIG_VP9_DXVA2_HWACCEL
            *fmtp++ = AV_PIX_FMT_DXVA2_VLD;
#endif
#if CONFIG_VP9_D3D11VA_HWACCEL
            *fmtp++ = AV_PIX_FMT_D3D11VA_VLD;
            *fmtp++ = AV_PIX_FMT_D3D11;
#endif
#if CONFIG_VP9_NVDEC_HWACCEL
            *fmtp++ = AV_PIX_FMT_CUDA;
#endif
#if CONFIG_VP9_VAAPI_HWACCEL
            *fmtp++ = AV_PIX_FMT_VAAPI;
#endif
            break;
        case AV_PIX_FMT_YUV420P12:
#if CONFIG_VP9_NVDEC_HWACCEL
            *fmtp++ = AV_PIX_FMT_CUDA;
#endif
#if CONFIG_VP9_VAAPI_HWACCEL
            *fmtp++ = AV_PIX_FMT_VAAPI;
#endif
            break;
        }


            return ret;

    }


        return 0;


#define assign(var, type, n) var = (type) p; p += s->sb_cols * (n) * sizeof(*var)
    // FIXME we slightly over-allocate here for subsampled chroma, but a little
    // bit of padding shouldn't affect performance...
        return AVERROR(ENOMEM);
#undef assign

    }

    }

    return 0;
}

static int update_block_buffers(AVCodecContext *avctx)
{
    int i;
    VP9Context *s = avctx->priv_data;
    int chroma_blocks, chroma_eobs, bytesperpixel = s->bytesperpixel;
    VP9TileData *td = &s->td[0];

    if (td->b_base && td->block_base && s->block_alloc_using_2pass == s->s.frames[CUR_FRAME].uses_2pass)
        return 0;

    vp9_tile_data_free(td);
    chroma_blocks = 64 * 64 >> (s->ss_h + s->ss_v);
    chroma_eobs   = 16 * 16 >> (s->ss_h + s->ss_v);
    if (s->s.frames[CUR_FRAME].uses_2pass) {
        int sbs = s->sb_cols * s->sb_rows;

        td->b_base = av_malloc_array(s->cols * s->rows, sizeof(VP9Block));
        td->block_base = av_mallocz(((64 * 64 + 2 * chroma_blocks) * bytesperpixel * sizeof(int16_t) +
                                    16 * 16 + 2 * chroma_eobs) * sbs);
        if (!td->b_base || !td->block_base)
            return AVERROR(ENOMEM);
        td->uvblock_base[0] = td->block_base + sbs * 64 * 64 * bytesperpixel;
        td->uvblock_base[1] = td->uvblock_base[0] + sbs * chroma_blocks * bytesperpixel;
        td->eob_base = (uint8_t *) (td->uvblock_base[1] + sbs * chroma_blocks * bytesperpixel);
        td->uveob_base[0] = td->eob_base + 16 * 16 * sbs;
        td->uveob_base[1] = td->uveob_base[0] + chroma_eobs * sbs;

        if (avctx->export_side_data & AV_CODEC_EXPORT_DATA_VIDEO_ENC_PARAMS) {
            td->block_structure = av_malloc_array(s->cols * s->rows, sizeof(*td->block_structure));
            if (!td->block_structure)
                return AVERROR(ENOMEM);
        }
    } else {
        for (i = 1; i < s->active_tile_cols; i++)
            vp9_tile_data_free(&s->td[i]);

        for (i = 0; i < s->active_tile_cols; i++) {
            s->td[i].b_base = av_malloc(sizeof(VP9Block));
            s->td[i].block_base = av_mallocz((64 * 64 + 2 * chroma_blocks) * bytesperpixel * sizeof(int16_t) +
                                       16 * 16 + 2 * chroma_eobs);
            if (!s->td[i].b_base || !s->td[i].block_base)
                return AVERROR(ENOMEM);
            s->td[i].uvblock_base[0] = s->td[i].block_base + 64 * 64 * bytesperpixel;
            s->td[i].uvblock_base[1] = s->td[i].uvblock_base[0] + chroma_blocks * bytesperpixel;
            s->td[i].eob_base = (uint8_t *) (s->td[i].uvblock_base[1] + chroma_blocks * bytesperpixel);
            s->td[i].uveob_base[0] = s->td[i].eob_base + 16 * 16;
            s->td[i].uveob_base[1] = s->td[i].uveob_base[0] + chroma_eobs;

            if (avctx->export_side_data & AV_CODEC_EXPORT_DATA_VIDEO_ENC_PARAMS) {
                s->td[i].block_structure = av_malloc_array(s->cols * s->rows, sizeof(*td->block_structure));
                if (!s->td[i].block_structure)
                    return AVERROR(ENOMEM);
            }
        }
    }
    s->block_alloc_using_2pass = s->s.frames[CUR_FRAME].uses_2pass;

    return 0;
}

// The sign bit is at the end, not the start, of a bit sequence
{
}

{
        return v;
}

// differential forward probability updates
{
          7,  20,  33,  46,  59,  72,  85,  98, 111, 124, 137, 150, 163, 176,
        189, 202, 215, 228, 241, 254,   1,   2,   3,   4,   5,   6,   8,   9,
         10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  21,  22,  23,  24,
         25,  26,  27,  28,  29,  30,  31,  32,  34,  35,  36,  37,  38,  39,
         40,  41,  42,  43,  44,  45,  47,  48,  49,  50,  51,  52,  53,  54,
         55,  56,  57,  58,  60,  61,  62,  63,  64,  65,  66,  67,  68,  69,
         70,  71,  73,  74,  75,  76,  77,  78,  79,  80,  81,  82,  83,  84,
         86,  87,  88,  89,  90,  91,  92,  93,  94,  95,  96,  97,  99, 100,
        101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 112, 113, 114, 115,
        116, 117, 118, 119, 120, 121, 122, 123, 125, 126, 127, 128, 129, 130,
        131, 132, 133, 134, 135, 136, 138, 139, 140, 141, 142, 143, 144, 145,
        146, 147, 148, 149, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160,
        161, 162, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
        177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 190, 191,
        192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 203, 204, 205, 206,
        207, 208, 209, 210, 211, 212, 213, 214, 216, 217, 218, 219, 220, 221,
        222, 223, 224, 225, 226, 227, 229, 230, 231, 232, 233, 234, 235, 236,
        237, 238, 239, 240, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251,
        252, 253, 253,
    };

    /* This code is trying to do a differential probability update. For a
     * current probability A in the range [1, 255], the difference to a new
     * probability of any value can be expressed differentially as 1-A, 255-A
     * where some part of this (absolute range) exists both in positive as
     * well as the negative part, whereas another part only exists in one
     * half. We're trying to code this shared part differentially, i.e.
     * times two where the value of the lowest bit specifies the sign, and
     * the single part is then coded on top of this. This absolute difference
     * then again has a value of [0, 254], but a bigger value in this range
     * indicates that we're further away from the original value A, so we
     * can code this as a VLC code, since higher values are increasingly
     * unlikely. The first 20 values in inv_map_table[] allow 'cheap, rough'
     * updates vs. the 'fine, exact' updates further down the range, which
     * adds one extra dimension to this differential update model. */

    } else {
    }

}

{
        AVCOL_SPC_UNSPECIFIED, AVCOL_SPC_BT470BG, AVCOL_SPC_BT709, AVCOL_SPC_SMPTE170M,
        AVCOL_SPC_SMPTE240M, AVCOL_SPC_BT2020_NCL, AVCOL_SPC_RESERVED, AVCOL_SPC_RGB,
    };

        static const enum AVPixelFormat pix_fmt_rgb[3] = {
            AV_PIX_FMT_GBRP, AV_PIX_FMT_GBRP10, AV_PIX_FMT_GBRP12
        };
        s->ss_h = s->ss_v = 0;
        avctx->color_range = AVCOL_RANGE_JPEG;
        s->pix_fmt = pix_fmt_rgb[bits];
        if (avctx->profile & 1) {
            if (get_bits1(&s->gb)) {
                av_log(avctx, AV_LOG_ERROR, "Reserved bit set in RGB\n");
                return AVERROR_INVALIDDATA;
            }
        } else {
            av_log(avctx, AV_LOG_ERROR, "RGB not supported in profile %d\n",
                   avctx->profile);
            return AVERROR_INVALIDDATA;
        }
    } else {
            { { AV_PIX_FMT_YUV444P, AV_PIX_FMT_YUV422P },
              { AV_PIX_FMT_YUV440P, AV_PIX_FMT_YUV420P } },
            { { AV_PIX_FMT_YUV444P10, AV_PIX_FMT_YUV422P10 },
              { AV_PIX_FMT_YUV440P10, AV_PIX_FMT_YUV420P10 } },
            { { AV_PIX_FMT_YUV444P12, AV_PIX_FMT_YUV422P12 },
              { AV_PIX_FMT_YUV440P12, AV_PIX_FMT_YUV420P12 } }
        };
                av_log(avctx, AV_LOG_ERROR, "YUV 4:2:0 not supported in profile %d\n",
                       avctx->profile);
                return AVERROR_INVALIDDATA;
                av_log(avctx, AV_LOG_ERROR, "Profile %d color details reserved bit set\n",
                       avctx->profile);
                return AVERROR_INVALIDDATA;
            }
        } else {
        }
    }

    return 0;
}

                               const uint8_t *data, int size, int *ref)
{

    /* general header */
        av_log(avctx, AV_LOG_ERROR, "Failed to initialize bitstream reader\n");
        return ret;
    }
        av_log(avctx, AV_LOG_ERROR, "Invalid frame marker\n");
        return AVERROR_INVALIDDATA;
    }
        av_log(avctx, AV_LOG_ERROR, "Profile %d is not yet supported\n", avctx->profile);
        return AVERROR_INVALIDDATA;
    }
    }



            av_log(avctx, AV_LOG_ERROR, "Invalid sync code\n");
            return AVERROR_INVALIDDATA;
        }
            return ret;
        // for profile 1, here follows the subsampling bits
            skip_bits(&s->gb, 32);
    } else {
                av_log(avctx, AV_LOG_ERROR, "Invalid sync code\n");
                return AVERROR_INVALIDDATA;
            }
                if ((ret = read_colorspace_details(avctx)) < 0)
                    return ret;
            } else {
            }
                skip_bits(&s->gb, 32);
        } else {
                av_log(avctx, AV_LOG_ERROR, "Not all references are available\n");
                return AVERROR_INVALIDDATA;
            }
                w = s->s.refs[s->s.h.refidx[2]].f->width;
                h = s->s.refs[s->s.h.refidx[2]].f->height;
            } else {
            }
            // Note that in this code, "CUR_FRAME" is actually before we
            // have formally allocated a frame, and thus actually represents
            // the _last_ frame
                } else if (s->s.h.signbias[0] == s->s.h.signbias[2]) {
                    s->s.h.fixcompref    = 1;
                    s->s.h.varcompref[0] = 0;
                    s->s.h.varcompref[1] = 2;
                } else {
                    s->s.h.fixcompref    = 0;
                    s->s.h.varcompref[0] = 1;
                    s->s.h.varcompref[1] = 2;
                }
            }
        }
    }

    /* loopfilter header data */
        // reset loopfilter defaults
    }
    // if sharpness changed, reinit lim/mblim LUTs. if it didn't change, keep
    // the old cache values since they are still valid

            }

        }
    }
        }
    }

    /* quantization header data */

    /* segmentation header info */
        }

            }
        }
    }

    // set qmul[] based on Y/UV, AC/DC and segmentation Q idx deltas

                qyac = av_clip_uintp2(s->s.h.segmentation.feat[i].q_val, 8);
            else
        } else {
        }


                lflvl = av_clip_uintp2(s->s.h.segmentation.feat[i].lf_val, 6);
            else
        } else {
        }
            }
        } else {
                   sizeof(s->s.h.segmentation.feat[i].lflvl));
        }
    }

    /* tiling info */
        av_log(avctx, AV_LOG_ERROR, "Failed to initialize decoder for %dx%d @ %d\n",
               w, h, s->pix_fmt);
        return ret;
    }
         s->s.h.tiling.log2_tile_cols++) ;
        else
            break;
    }

            for (i = 0; i < s->active_tile_cols; i++)
                vp9_tile_data_free(&s->td[i]);
            av_free(s->td);
        }

            n_range_coders = 4; // max_tile_rows
        } else {
        }
                                 n_range_coders * sizeof(VP56RangeCoder));
            return AVERROR(ENOMEM);
        }
    }

    /* check reference frames */
        int valid_ref_frame = 0;

                av_log(avctx, AV_LOG_ERROR,
                       "Ref pixfmt (%s) did not match current frame (%s)",
                       av_get_pix_fmt_name(ref->format),
                       av_get_pix_fmt_name(avctx->pix_fmt));
                return AVERROR_INVALIDDATA;
            } else {
                /* Check to make sure at least one of frames that */
                /* this frame references has valid dimensions     */
                    av_log(avctx, AV_LOG_WARNING,
                           "Invalid ref frame dimensions %dx%d for frame size %dx%d\n",
                           refw, refh, w, h);
                    s->mvscale[i][0] = s->mvscale[i][1] = REF_INVALID_SCALE;
                    continue;
                }
            }
        }
            av_log(avctx, AV_LOG_ERROR, "No valid reference frame is found, bitstream not supported\n");
            return AVERROR_INVALIDDATA;
        }
    }

               sizeof(ff_vp9_default_coef_probs));
               sizeof(ff_vp9_default_coef_probs));
               sizeof(ff_vp9_default_coef_probs));
               sizeof(ff_vp9_default_coef_probs));
               sizeof(ff_vp9_default_coef_probs));
    }

    // next 16 bits is size of the rest of the header (arith-coded)

        av_log(avctx, AV_LOG_ERROR, "Invalid compressed header size\n");
        return AVERROR_INVALIDDATA;
    }
        return ret;

        av_log(avctx, AV_LOG_ERROR, "Marker bit was set\n");
        return AVERROR_INVALIDDATA;
    }

        } else {
        }
    }

    /* FIXME is it faster to not copy here, but do it down in the fw updates
     * as explicit copies if the fw update is missing (and skip the copy upon
     * fw update)? */

    // txfm updates
    } else {

        }
    }

    // coef updates
                                break;
                                else
                            }
                        }
        } else {
                                break;
                        }
        }
            break;
    }

    // mode updates



        } else {
        }

            }
        }

        }



        // mv fields don't use the update_prob subexp model for some reason




        }


        }


            }
        }
    }

}

                      ptrdiff_t yoff, ptrdiff_t uvoff, enum BlockLevel bl)
{
                                                     s->prob.p.partition[bl][c];

                          yoff + 8 * hbs * bytesperpixel,
                          yoff + 8 * hbs * bytesperpixel,
            default:
                av_assert0(0);
            }
                      yoff + 8 * hbs * bytesperpixel,
        } else {
        }
        } else {
        }
    } else {
    }

static void decode_sb_mem(VP9TileData *td, int row, int col, VP9Filter *lflvl,
                          ptrdiff_t yoff, ptrdiff_t uvoff, enum BlockLevel bl)
{
    const VP9Context *s = td->s;
    VP9Block *b = td->b;
    ptrdiff_t hbs = 4 >> bl;
    AVFrame *f = s->s.frames[CUR_FRAME].tf.f;
    ptrdiff_t y_stride = f->linesize[0], uv_stride = f->linesize[1];
    int bytesperpixel = s->bytesperpixel;

    if (bl == BL_8X8) {
        av_assert2(b->bl == BL_8X8);
        ff_vp9_decode_block(td, row, col, lflvl, yoff, uvoff, b->bl, b->bp);
    } else if (td->b->bl == bl) {
        ff_vp9_decode_block(td, row, col, lflvl, yoff, uvoff, b->bl, b->bp);
        if (b->bp == PARTITION_H && row + hbs < s->rows) {
            yoff  += hbs * 8 * y_stride;
            uvoff += hbs * 8 * uv_stride >> s->ss_v;
            ff_vp9_decode_block(td, row + hbs, col, lflvl, yoff, uvoff, b->bl, b->bp);
        } else if (b->bp == PARTITION_V && col + hbs < s->cols) {
            yoff  += hbs * 8 * bytesperpixel;
            uvoff += hbs * 8 * bytesperpixel >> s->ss_h;
            ff_vp9_decode_block(td, row, col + hbs, lflvl, yoff, uvoff, b->bl, b->bp);
        }
    } else {
        decode_sb_mem(td, row, col, lflvl, yoff, uvoff, bl + 1);
        if (col + hbs < s->cols) { // FIXME why not <=?
            if (row + hbs < s->rows) {
                decode_sb_mem(td, row, col + hbs, lflvl, yoff + 8 * hbs * bytesperpixel,
                              uvoff + (8 * hbs * bytesperpixel >> s->ss_h), bl + 1);
                yoff  += hbs * 8 * y_stride;
                uvoff += hbs * 8 * uv_stride >> s->ss_v;
                decode_sb_mem(td, row + hbs, col, lflvl, yoff, uvoff, bl + 1);
                decode_sb_mem(td, row + hbs, col + hbs, lflvl,
                              yoff + 8 * hbs * bytesperpixel,
                              uvoff + (8 * hbs * bytesperpixel >> s->ss_h), bl + 1);
            } else {
                yoff  += hbs * 8 * bytesperpixel;
                uvoff += hbs * 8 * bytesperpixel >> s->ss_h;
                decode_sb_mem(td, row, col + hbs, lflvl, yoff, uvoff, bl + 1);
            }
        } else if (row + hbs < s->rows) {
            yoff  += hbs * 8 * y_stride;
            uvoff += hbs * 8 * uv_stride >> s->ss_v;
            decode_sb_mem(td, row + hbs, col, lflvl, yoff, uvoff, bl + 1);
        }
    }
}

{
}

{


{

    }
    }

}

                        const uint8_t *data, int size)
{




            } else {
            }
                ff_thread_report_progress(&s->s.frames[CUR_FRAME].tf, INT_MAX, 0);
                return AVERROR_INVALIDDATA;
            }
                return ret;
                ff_thread_report_progress(&s->s.frames[CUR_FRAME].tf, INT_MAX, 0);
                return AVERROR_INVALIDDATA;
            }
        }


                    } else {
                    }

                }

                    // FIXME integrate with lf code (i.e. zero after each
                    // use, similar to invtxfm coefficients, or similar)
                    }

                        decode_sb_mem(td, row, col, lflvl_ptr,
                                      yoff2, uvoff2, BL_64X64);
                    } else {
                            return AVERROR_INVALIDDATA;
                        }
                                  yoff2, uvoff2, BL_64X64);
                    }
                }
            }

                continue;

            // backup pre-loopfilter reconstruction data for intra
            // prediction of next row of sb64s
            }

            // loopfilter one row
                                         yoff2, uvoff2);
                }
            }

            // FIXME maybe we can make this more finegrained by running the
            // loopfilter per-block instead of after each sbrow
            // In fact that would also make intra pred left preparation easier?
        }
    }
    return 0;
}

#if HAVE_THREADS
static av_always_inline
int decode_tiles_mt(AVCodecContext *avctx, void *tdata, int jobnr,
                              int threadnr)
{
    VP9Context *s = avctx->priv_data;
    VP9TileData *td = &s->td[jobnr];
    ptrdiff_t uvoff, yoff, ls_y, ls_uv;
    int bytesperpixel = s->bytesperpixel, row, col, tile_row;
    unsigned tile_cols_len;
    int tile_row_start, tile_row_end, tile_col_start, tile_col_end;
    VP9Filter *lflvl_ptr_base;
    AVFrame *f;

    f = s->s.frames[CUR_FRAME].tf.f;
    ls_y = f->linesize[0];
    ls_uv =f->linesize[1];

    set_tile_offset(&tile_col_start, &tile_col_end,
                    jobnr, s->s.h.tiling.log2_tile_cols, s->sb_cols);
    td->tile_col_start  = tile_col_start;
    uvoff = (64 * bytesperpixel >> s->ss_h)*(tile_col_start >> 3);
    yoff = (64 * bytesperpixel)*(tile_col_start >> 3);
    lflvl_ptr_base = s->lflvl+(tile_col_start >> 3);

    for (tile_row = 0; tile_row < s->s.h.tiling.tile_rows; tile_row++) {
        set_tile_offset(&tile_row_start, &tile_row_end,
                        tile_row, s->s.h.tiling.log2_tile_rows, s->sb_rows);

        td->c = &td->c_b[tile_row];
        for (row = tile_row_start; row < tile_row_end;
             row += 8, yoff += ls_y * 64, uvoff += ls_uv * 64 >> s->ss_v) {
            ptrdiff_t yoff2 = yoff, uvoff2 = uvoff;
            VP9Filter *lflvl_ptr = lflvl_ptr_base+s->sb_cols*(row >> 3);

            memset(td->left_partition_ctx, 0, 8);
            memset(td->left_skip_ctx, 0, 8);
            if (s->s.h.keyframe || s->s.h.intraonly) {
                memset(td->left_mode_ctx, DC_PRED, 16);
            } else {
                memset(td->left_mode_ctx, NEARESTMV, 8);
            }
            memset(td->left_y_nnz_ctx, 0, 16);
            memset(td->left_uv_nnz_ctx, 0, 32);
            memset(td->left_segpred_ctx, 0, 8);

            for (col = tile_col_start;
                 col < tile_col_end;
                 col += 8, yoff2 += 64 * bytesperpixel,
                 uvoff2 += 64 * bytesperpixel >> s->ss_h, lflvl_ptr++) {
                // FIXME integrate with lf code (i.e. zero after each
                // use, similar to invtxfm coefficients, or similar)
                memset(lflvl_ptr->mask, 0, sizeof(lflvl_ptr->mask));
                decode_sb(td, row, col, lflvl_ptr,
                            yoff2, uvoff2, BL_64X64);
            }

            // backup pre-loopfilter reconstruction data for intra
            // prediction of next row of sb64s
            tile_cols_len = tile_col_end - tile_col_start;
            if (row + 8 < s->rows) {
                memcpy(s->intra_pred_data[0] + (tile_col_start * 8 * bytesperpixel),
                       f->data[0] + yoff + 63 * ls_y,
                       8 * tile_cols_len * bytesperpixel);
                memcpy(s->intra_pred_data[1] + (tile_col_start * 8 * bytesperpixel >> s->ss_h),
                       f->data[1] + uvoff + ((64 >> s->ss_v) - 1) * ls_uv,
                       8 * tile_cols_len * bytesperpixel >> s->ss_h);
                memcpy(s->intra_pred_data[2] + (tile_col_start * 8 * bytesperpixel >> s->ss_h),
                       f->data[2] + uvoff + ((64 >> s->ss_v) - 1) * ls_uv,
                       8 * tile_cols_len * bytesperpixel >> s->ss_h);
            }

            vp9_report_tile_progress(s, row >> 3, 1);
        }
    }
    return 0;
}

static av_always_inline
int loopfilter_proc(AVCodecContext *avctx)
{
    VP9Context *s = avctx->priv_data;
    ptrdiff_t uvoff, yoff, ls_y, ls_uv;
    VP9Filter *lflvl_ptr;
    int bytesperpixel = s->bytesperpixel, col, i;
    AVFrame *f;

    f = s->s.frames[CUR_FRAME].tf.f;
    ls_y = f->linesize[0];
    ls_uv =f->linesize[1];

    for (i = 0; i < s->sb_rows; i++) {
        vp9_await_tile_progress(s, i, s->s.h.tiling.tile_cols);

        if (s->s.h.filter.level) {
            yoff = (ls_y * 64)*i;
            uvoff =  (ls_uv * 64 >> s->ss_v)*i;
            lflvl_ptr = s->lflvl+s->sb_cols*i;
            for (col = 0; col < s->cols;
                 col += 8, yoff += 64 * bytesperpixel,
                 uvoff += 64 * bytesperpixel >> s->ss_h, lflvl_ptr++) {
                ff_vp9_loopfilter_sb(avctx, lflvl_ptr, i << 3, col,
                                     yoff, uvoff);
            }
        }
    }
    return 0;
}
#endif

static int vp9_export_enc_params(VP9Context *s, VP9Frame *frame)
{
    AVVideoEncParams *par;
    unsigned int tile, nb_blocks = 0;

    if (s->s.h.segmentation.enabled) {
        for (tile = 0; tile < s->active_tile_cols; tile++)
            nb_blocks += s->td[tile].nb_block_structure;
    }

    par = av_video_enc_params_create_side_data(frame->tf.f,
        AV_VIDEO_ENC_PARAMS_VP9, nb_blocks);
    if (!par)
        return AVERROR(ENOMEM);

    par->qp             = s->s.h.yac_qi;
    par->delta_qp[0][0] = s->s.h.ydc_qdelta;
    par->delta_qp[1][0] = s->s.h.uvdc_qdelta;
    par->delta_qp[2][0] = s->s.h.uvdc_qdelta;
    par->delta_qp[1][1] = s->s.h.uvac_qdelta;
    par->delta_qp[2][1] = s->s.h.uvac_qdelta;

    if (nb_blocks) {
        unsigned int block = 0;
        unsigned int tile, block_tile;

        for (tile = 0; tile < s->active_tile_cols; tile++) {
            VP9TileData *td = &s->td[tile];

            for (block_tile = 0; block_tile < td->nb_block_structure; block_tile++) {
                AVVideoBlockParams *b = av_video_enc_params_block(par, block++);
                unsigned int      row = td->block_structure[block_tile].row;
                unsigned int      col = td->block_structure[block_tile].col;
                uint8_t        seg_id = frame->segmentation_map[row * 8 * s->sb_cols + col];

                b->src_x = col * 8;
                b->src_y = row * 8;
                b->w     = 1 << (3 + td->block_structure[block_tile].block_size_idx_x);
                b->h     = 1 << (3 + td->block_structure[block_tile].block_size_idx_y);

                if (s->s.h.segmentation.feat[seg_id].q_enabled) {
                    b->delta_qp = s->s.h.segmentation.feat[seg_id].q_val;
                    if (s->s.h.segmentation.absolute_vals)
                        b->delta_qp -= par->qp;
                }
            }
        }
    }

    return 0;
}

                            int *got_frame, AVPacket *pkt)
{

        return ret;
            av_log(avctx, AV_LOG_ERROR, "Requested reference %d not available\n", ref);
            return AVERROR_INVALIDDATA;
        }
            return ret;
#if FF_API_PKT_PTS
FF_DISABLE_DEPRECATION_WARNINGS
FF_ENABLE_DEPRECATION_WARNINGS
#endif
                return ret;
        }
    }

            return ret;
    }
        return ret;
        return ret;

         s->s.frames[REF_FRAME_MVPAIR].tf.f->height != s->s.frames[CUR_FRAME].tf.f->height)) {
    }

    // ref frame setup
        }
            return ret;
    }

        ret = avctx->hwaccel->start_frame(avctx, NULL, 0);
        if (ret < 0)
            return ret;
        ret = avctx->hwaccel->decode_slice(avctx, pkt->data, pkt->size);
        if (ret < 0)
            return ret;
        ret = avctx->hwaccel->end_frame(avctx);
        if (ret < 0)
            return ret;
        goto finish;
    }

    // main tile decode loop
    } else {
    }
        av_log(avctx, AV_LOG_ERROR,
               "Failed to allocate block buffers\n");
        return ret;
    }
        int j, k, l, m;

                break;
        }
    }

#if HAVE_THREADS
        for (i = 0; i < s->sb_rows; i++)
            atomic_store(&s->entries[i], 0);
    }
#endif

        }

#if HAVE_THREADS
            int tile_row, tile_col;

            av_assert1(!s->pass);

            for (tile_row = 0; tile_row < s->s.h.tiling.tile_rows; tile_row++) {
                for (tile_col = 0; tile_col < s->s.h.tiling.tile_cols; tile_col++) {
                    int64_t tile_size;

                    if (tile_col == s->s.h.tiling.tile_cols - 1 &&
                        tile_row == s->s.h.tiling.tile_rows - 1) {
                        tile_size = size;
                    } else {
                        tile_size = AV_RB32(data);
                        data += 4;
                        size -= 4;
                    }
                    if (tile_size > size)
                        return AVERROR_INVALIDDATA;
                    ret = ff_vp56_init_range_decoder(&s->td[tile_col].c_b[tile_row], data, tile_size);
                    if (ret < 0)
                        return ret;
                    if (vp56_rac_get_prob_branchy(&s->td[tile_col].c_b[tile_row], 128)) // marker bit
                        return AVERROR_INVALIDDATA;
                    data += tile_size;
                    size -= tile_size;
                }
            }

            ff_slice_thread_execute_with_mainfunc(avctx, decode_tiles_mt, loopfilter_proc, s->td, NULL, s->s.h.tiling.tile_cols);
        } else
#endif
        {
                ff_thread_report_progress(&s->s.frames[CUR_FRAME].tf, INT_MAX, 0);
                return ret;
            }
        }

        // Sum all counts fields into td[0].counts for tile threading
            for (i = 1; i < s->s.h.tiling.tile_cols; i++)
                for (j = 0; j < sizeof(s->td[i].counts) / sizeof(unsigned); j++)
                    ((unsigned *)&s->td[0].counts)[j] += ((unsigned *)&s->td[i].counts)[j];

        }

        av_log(avctx, AV_LOG_ERROR, "Failed to decode tile data\n");
        s->td->error_info = 0;
        return AVERROR_INVALIDDATA;
    }
            return ret;
    }

    // ref frame setup
            return ret;
    }

            return ret;
    }

}

static void vp9_decode_flush(AVCodecContext *avctx)
{
    VP9Context *s = avctx->priv_data;
    int i;

    for (i = 0; i < 3; i++)
        vp9_frame_unref(avctx, &s->s.frames[i]);
    for (i = 0; i < 8; i++)
        ff_thread_release_buffer(avctx, &s->s.refs[i]);
}

{

            vp9_decode_free(avctx);
            av_log(avctx, AV_LOG_ERROR, "Failed to allocate frame buffer %d\n", i);
            return AVERROR(ENOMEM);
        }
    }
            vp9_decode_free(avctx);
            av_log(avctx, AV_LOG_ERROR, "Failed to allocate frame buffer %d\n", i);
            return AVERROR(ENOMEM);
        }
    }

    return 0;
}

{


}

#if HAVE_THREADS
static int vp9_decode_update_thread_context(AVCodecContext *dst, const AVCodecContext *src)
{
    int i, ret;
    VP9Context *s = dst->priv_data, *ssrc = src->priv_data;

    for (i = 0; i < 3; i++) {
        if (s->s.frames[i].tf.f->buf[0])
            vp9_frame_unref(dst, &s->s.frames[i]);
        if (ssrc->s.frames[i].tf.f->buf[0]) {
            if ((ret = vp9_frame_ref(dst, &s->s.frames[i], &ssrc->s.frames[i])) < 0)
                return ret;
        }
    }
    for (i = 0; i < 8; i++) {
        if (s->s.refs[i].f->buf[0])
            ff_thread_release_buffer(dst, &s->s.refs[i]);
        if (ssrc->next_refs[i].f->buf[0]) {
            if ((ret = ff_thread_ref_frame(&s->s.refs[i], &ssrc->next_refs[i])) < 0)
                return ret;
        }
    }

    s->s.h.invisible = ssrc->s.h.invisible;
    s->s.h.keyframe = ssrc->s.h.keyframe;
    s->s.h.intraonly = ssrc->s.h.intraonly;
    s->ss_v = ssrc->ss_v;
    s->ss_h = ssrc->ss_h;
    s->s.h.segmentation.enabled = ssrc->s.h.segmentation.enabled;
    s->s.h.segmentation.update_map = ssrc->s.h.segmentation.update_map;
    s->s.h.segmentation.absolute_vals = ssrc->s.h.segmentation.absolute_vals;
    s->bytesperpixel = ssrc->bytesperpixel;
    s->gf_fmt = ssrc->gf_fmt;
    s->w = ssrc->w;
    s->h = ssrc->h;
    s->s.h.bpp = ssrc->s.h.bpp;
    s->bpp_index = ssrc->bpp_index;
    s->pix_fmt = ssrc->pix_fmt;
    memcpy(&s->prob_ctx, &ssrc->prob_ctx, sizeof(s->prob_ctx));
    memcpy(&s->s.h.lf_delta, &ssrc->s.h.lf_delta, sizeof(s->s.h.lf_delta));
    memcpy(&s->s.h.segmentation.feat, &ssrc->s.h.segmentation.feat,
           sizeof(s->s.h.segmentation.feat));

    return 0;
}
#endif

AVCodec ff_vp9_decoder = {
    .name                  = "vp9",
    .long_name             = NULL_IF_CONFIG_SMALL("Google VP9"),
    .type                  = AVMEDIA_TYPE_VIDEO,
    .id                    = AV_CODEC_ID_VP9,
    .priv_data_size        = sizeof(VP9Context),
    .init                  = vp9_decode_init,
    .close                 = vp9_decode_free,
    .decode                = vp9_decode_frame,
    .capabilities          = AV_CODEC_CAP_DR1 | AV_CODEC_CAP_FRAME_THREADS | AV_CODEC_CAP_SLICE_THREADS,
    .caps_internal         = FF_CODEC_CAP_SLICE_THREAD_HAS_MF |
                             FF_CODEC_CAP_ALLOCATE_PROGRESS,
    .flush                 = vp9_decode_flush,
    .update_thread_context = ONLY_IF_THREADS_ENABLED(vp9_decode_update_thread_context),
    .profiles              = NULL_IF_CONFIG_SMALL(ff_vp9_profiles),
    .bsfs                  = "vp9_superframe_split",
    .hw_configs            = (const AVCodecHWConfigInternal*[]) {
#if CONFIG_VP9_DXVA2_HWACCEL
                               HWACCEL_DXVA2(vp9),
#endif
#if CONFIG_VP9_D3D11VA_HWACCEL
                               HWACCEL_D3D11VA(vp9),
#endif
#if CONFIG_VP9_D3D11VA2_HWACCEL
                               HWACCEL_D3D11VA2(vp9),
#endif
#if CONFIG_VP9_NVDEC_HWACCEL
                               HWACCEL_NVDEC(vp9),
#endif
#if CONFIG_VP9_VAAPI_HWACCEL
                               HWACCEL_VAAPI(vp9),
#endif
#if CONFIG_VP9_VDPAU_HWACCEL
                               HWACCEL_VDPAU(vp9),
#endif
                               NULL
                           },
};
