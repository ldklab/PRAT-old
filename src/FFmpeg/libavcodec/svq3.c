/*
 * Copyright (c) 2003 The FFmpeg Project
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

/*
 * How to use this decoder:
 * SVQ3 data is transported within Apple Quicktime files. Quicktime files
 * have stsd atoms to describe media trak properties. A stsd atom for a
 * video trak contains 1 or more ImageDescription atoms. These atoms begin
 * with the 4-byte length of the atom followed by the codec fourcc. Some
 * decoders need information in this atom to operate correctly. Such
 * is the case with SVQ3. In order to get the best use out of this decoder,
 * the calling app must make the SVQ3 ImageDescription atom available
 * via the AVCodecContext's extradata[_size] field:
 *
 * AVCodecContext.extradata = pointer to ImageDescription, first characters
 * are expected to be 'S', 'V', 'Q', and '3', NOT the 4-byte atom length
 * AVCodecContext.extradata_size = size of ImageDescription atom memory
 * buffer (which will be the same as the ImageDescription atom size field
 * from the QT file, minus 4 bytes since the length is missing)
 *
 * You will know you have these parameters passed correctly when the decoder
 * correctly decodes this file:
 *  http://samples.mplayerhq.hu/V-codecs/SVQ3/Vertical400kbit.sorenson3.mov
 */

#include <inttypes.h>

#include "libavutil/attributes.h"
#include "libavutil/crc.h"

#include "internal.h"
#include "avcodec.h"
#include "mpegutils.h"
#include "h264dec.h"
#include "h264data.h"
#include "golomb.h"
#include "hpeldsp.h"
#include "mathops.h"
#include "rectangle.h"
#include "tpeldsp.h"

#if CONFIG_ZLIB
#include <zlib.h>
#endif

#include "svq1.h"

/**
 * @file
 * svq3 decoder.
 */

typedef struct SVQ3Frame {
    AVFrame *f;

    AVBufferRef *motion_val_buf[2];
    int16_t (*motion_val[2])[2];

    AVBufferRef *mb_type_buf;
    uint32_t *mb_type;


    AVBufferRef *ref_index_buf[2];
    int8_t *ref_index[2];
} SVQ3Frame;

typedef struct SVQ3Context {
    AVCodecContext *avctx;

    H264DSPContext  h264dsp;
    H264PredContext hpc;
    HpelDSPContext hdsp;
    TpelDSPContext tdsp;
    VideoDSPContext vdsp;

    SVQ3Frame *cur_pic;
    SVQ3Frame *next_pic;
    SVQ3Frame *last_pic;
    GetBitContext gb;
    GetBitContext gb_slice;
    uint8_t *slice_buf;
    int slice_size;
    int halfpel_flag;
    int thirdpel_flag;
    int has_watermark;
    uint32_t watermark_key;
    uint8_t *buf;
    int buf_size;
    int adaptive_quant;
    int next_p_frame_damaged;
    int h_edge_pos;
    int v_edge_pos;
    int last_frame_output;
    int slice_num;
    int qscale;
    int cbp;
    int frame_num;
    int frame_num_offset;
    int prev_frame_num_offset;
    int prev_frame_num;

    enum AVPictureType pict_type;
    enum AVPictureType slice_type;
    int low_delay;

    int mb_x, mb_y;
    int mb_xy;
    int mb_width, mb_height;
    int mb_stride, mb_num;
    int b_stride;

    uint32_t *mb2br_xy;

    int chroma_pred_mode;
    int intra16x16_pred_mode;

    int8_t   intra4x4_pred_mode_cache[5 * 8];
    int8_t (*intra4x4_pred_mode);

    unsigned int top_samples_available;
    unsigned int topright_samples_available;
    unsigned int left_samples_available;

    uint8_t *edge_emu_buffer;

    DECLARE_ALIGNED(16, int16_t, mv_cache)[2][5 * 8][2];
    DECLARE_ALIGNED(8,  int8_t, ref_cache)[2][5 * 8];
    DECLARE_ALIGNED(16, int16_t, mb)[16 * 48 * 2];
    DECLARE_ALIGNED(16, int16_t, mb_luma_dc)[3][16 * 2];
    DECLARE_ALIGNED(8, uint8_t, non_zero_count_cache)[15 * 8];
    uint32_t dequant4_coeff[QP_MAX_NUM + 1][16];
    int block_offset[2 * (16 * 3)];
} SVQ3Context;

#define FULLPEL_MODE  1
#define HALFPEL_MODE  2
#define THIRDPEL_MODE 3
#define PREDICT_MODE  4

/* dual scan (from some older H.264 draft)
 * o-->o-->o   o
 *         |  /|
 * o   o   o / o
 * | / |   |/  |
 * o   o   o   o
 *   /
 * o-->o-->o-->o
 */
static const uint8_t svq3_scan[16] = {
    0 + 0 * 4, 1 + 0 * 4, 2 + 0 * 4, 2 + 1 * 4,
    2 + 2 * 4, 3 + 0 * 4, 3 + 1 * 4, 3 + 2 * 4,
    0 + 1 * 4, 0 + 2 * 4, 1 + 1 * 4, 1 + 2 * 4,
    0 + 3 * 4, 1 + 3 * 4, 2 + 3 * 4, 3 + 3 * 4,
};

static const uint8_t luma_dc_zigzag_scan[16] = {
    0 * 16 + 0 * 64, 1 * 16 + 0 * 64, 2 * 16 + 0 * 64, 0 * 16 + 2 * 64,
    3 * 16 + 0 * 64, 0 * 16 + 1 * 64, 1 * 16 + 1 * 64, 2 * 16 + 1 * 64,
    1 * 16 + 2 * 64, 2 * 16 + 2 * 64, 3 * 16 + 2 * 64, 0 * 16 + 3 * 64,
    3 * 16 + 1 * 64, 1 * 16 + 3 * 64, 2 * 16 + 3 * 64, 3 * 16 + 3 * 64,
};

static const uint8_t svq3_pred_0[25][2] = {
    { 0, 0 },
    { 1, 0 }, { 0, 1 },
    { 0, 2 }, { 1, 1 }, { 2, 0 },
    { 3, 0 }, { 2, 1 }, { 1, 2 }, { 0, 3 },
    { 0, 4 }, { 1, 3 }, { 2, 2 }, { 3, 1 }, { 4, 0 },
    { 4, 1 }, { 3, 2 }, { 2, 3 }, { 1, 4 },
    { 2, 4 }, { 3, 3 }, { 4, 2 },
    { 4, 3 }, { 3, 4 },
    { 4, 4 }
};

static const int8_t svq3_pred_1[6][6][5] = {
    { { 2, -1, -1, -1, -1 }, { 2, 1, -1, -1, -1 }, { 1, 2, -1, -1, -1 },
      { 2,  1, -1, -1, -1 }, { 1, 2, -1, -1, -1 }, { 1, 2, -1, -1, -1 } },
    { { 0,  2, -1, -1, -1 }, { 0, 2,  1,  4,  3 }, { 0, 1,  2,  4,  3 },
      { 0,  2,  1,  4,  3 }, { 2, 0,  1,  3,  4 }, { 0, 4,  2,  1,  3 } },
    { { 2,  0, -1, -1, -1 }, { 2, 1,  0,  4,  3 }, { 1, 2,  4,  0,  3 },
      { 2,  1,  0,  4,  3 }, { 2, 1,  4,  3,  0 }, { 1, 2,  4,  0,  3 } },
    { { 2,  0, -1, -1, -1 }, { 2, 0,  1,  4,  3 }, { 1, 2,  0,  4,  3 },
      { 2,  1,  0,  4,  3 }, { 2, 1,  3,  4,  0 }, { 2, 4,  1,  0,  3 } },
    { { 0,  2, -1, -1, -1 }, { 0, 2,  1,  3,  4 }, { 1, 2,  3,  0,  4 },
      { 2,  0,  1,  3,  4 }, { 2, 1,  3,  0,  4 }, { 2, 0,  4,  3,  1 } },
    { { 0,  2, -1, -1, -1 }, { 0, 2,  4,  1,  3 }, { 1, 4,  2,  0,  3 },
      { 4,  2,  0,  1,  3 }, { 2, 0,  1,  4,  3 }, { 4, 2,  1,  0,  3 } },
};

static const struct {
    uint8_t run;
    uint8_t level;
} svq3_dct_tables[2][16] = {
    { { 0, 0 }, { 0, 1 }, { 1, 1 }, { 2, 1 }, { 0, 2 }, { 3, 1 }, { 4, 1 }, { 5, 1 },
      { 0, 3 }, { 1, 2 }, { 2, 2 }, { 6, 1 }, { 7, 1 }, { 8, 1 }, { 9, 1 }, { 0, 4 } },
    { { 0, 0 }, { 0, 1 }, { 1, 1 }, { 0, 2 }, { 2, 1 }, { 0, 3 }, { 0, 4 }, { 0, 5 },
      { 3, 1 }, { 4, 1 }, { 1, 2 }, { 1, 3 }, { 0, 6 }, { 0, 7 }, { 0, 8 }, { 0, 9 } }
};

static const uint32_t svq3_dequant_coeff[32] = {
     3881,  4351,  4890,  5481,   6154,   6914,   7761,   8718,
     9781, 10987, 12339, 13828,  15523,  17435,  19561,  21873,
    24552, 27656, 30847, 34870,  38807,  43747,  49103,  54683,
    61694, 68745, 77615, 89113, 100253, 109366, 126635, 141533
};

static int svq3_decode_end(AVCodecContext *avctx);

{
#define stride 16


    }


    }
#undef stride

                            int stride, int qp, int dc)
{

    }


    }


    }


                                    int index, const int type)
{
        luma_dc_zigzag_scan, ff_zigzag_scan, svq3_scan, ff_h264_chroma_dc_scan
    };


                return -1;


                    run   = 1;
                    level = 1;
                } else {
                }
            } else {
                } else {
                }
            }


                return -1;

        }

            break;
        }
    }

    return 0;
}

static av_always_inline int
                       int i, int list, int part_width)
{

    } else {
    }
}

/**
 * Get the predicted MV.
 * @param n the block index
 * @param part_width the width of the partition (4, 8,16) -> (1, 2, 4)
 * @param mx the x component of the predicted motion vector
 * @param my the y component of the predicted motion vector
 */
                                              int part_width, int list,
                                              int ref, int *const mx, int *const my)
{

/* mv_cache
 * B . . A T T T T
 * U . . L . . , .
 * U . . L . . . .
 * U . . L . . , .
 * . . . L . . . .
 */

        } else if (top_ref == ref) {
            *mx = B[0];
            *my = B[1];
        } else {
            *mx = C[0];
            *my = C[1];
        }
    } else {
        if (top_ref      == PART_NOT_AVAILABLE &&
            diagonal_ref == PART_NOT_AVAILABLE &&
            left_ref     != PART_NOT_AVAILABLE) {
            *mx = A[0];
            *my = A[1];
        } else {
            *mx = mid_pred(A[0], B[0], C[0]);
            *my = mid_pred(A[1], B[1], C[1]);
        }
    }
}

                                    int x, int y, int width, int height,
                                    int mx, int my, int dxy,
                                    int thirdpel, int dir, int avg)
{


    }

    /* form component predictions */

                                 linesize, linesize,
                                 width + 1, height + 1,
                                 mx, my, s->h_edge_pos, s->v_edge_pos);
    }
                                                 width, height);
    else
                                                       height);



                                         uvlinesize, uvlinesize,
                                         width + 1, height + 1,
            }
                                                         uvlinesize,
                                                         width, height);
            else
                                                               uvlinesize,
                                                               height);
        }
    }

                              int dir, int avg)
{


            } else {

                } else {
                }
            }

            /* clip motion vector prediction to frame border */

            /* get (optional) motion vector differential */
                dx = dy = 0;
            } else {

                    av_log(s->avctx, AV_LOG_ERROR, "invalid MV vlc\n");
                    return -1;
                }
            }

            /* compute motion vector */

                                 fx, fy, dxy, 1, dir, avg);

                                 mx >> 1, my >> 1, dxy, 0, dir, avg);
            } else {

                                 mx, my, 0, 0, dir, avg);
            }

            /* update mv_cache */


                }
            }

            /* write back motion vectors */
                           part_width >> 2, part_height >> 2, s->b_stride,
                           pack16to32(mx, my), 4);
        }

    return 0;
}

                                                    int mb_type, const int *block_offset,
                                                    int linesize, uint8_t *dest_y)
{
                                s->qscale, IS_INTRA(mb_type) ? 1 : 0);
            }
    }
}

                                                       int mb_type,
                                                       const int *block_offset,
                                                       int linesize,
                                                       uint8_t *dest_y)
{


                    tr       = ptr[3 - linesize] * 0x01010101u;
                    topright = (uint8_t *)&tr;
                } else
            } else
                topright = NULL;

            }
        }
    } else {
    }
}

{





    }


                }
        }
    }

{


                             0, 0, 0, 0, 0, 0);

                                 0, 0, 0, 0, 1, 1);

            mb_type = MB_TYPE_SKIP;
        } else {
                return -1;
                return -1;

            mb_type = MB_TYPE_16x16;
        }
            mode = THIRDPEL_MODE;
            mode = HALFPEL_MODE;
        else
            mode = FULLPEL_MODE;

        /* fill caches */
        /* note ref_cache should contain here:
         *  ????????
         *  ???11111
         *  N??11111
         *  N??11111
         *  N??11111
         */

                              s->cur_pic->motion_val[m][b_xy - 1 + i * s->b_stride]);
            } else {
            }
                       4 * 2 * sizeof(int16_t));

                              s->cur_pic->motion_val[m][b_xy - s->b_stride + 4]);
                } else
                              s->cur_pic->motion_val[m][b_xy - s->b_stride - 1]);
                } else
            } else
                       PART_NOT_AVAILABLE, 8);

                break;
        }

        /* decode motion vector(s) and form prediction(s) */
                return -1;
        } else {        /* AV_PICTURE_TYPE_B */
                    return -1;
            } else {
                           0, 4 * 2 * sizeof(int16_t));
            }
                    return -1;
            } else {
                           0, 4 * 2 * sizeof(int16_t));
            }
        }

        mb_type = MB_TYPE_16x16;


                    s->left_samples_available = 0x5F5F;
            }

                    s->top_samples_available = 0x33FF;
            }

            /* decode prediction codes for luma blocks */

                    av_log(s->avctx, AV_LOG_ERROR,
                           "luma prediction:%"PRIu32"\n", vlc);
                    return -1;
                }



                    av_log(s->avctx, AV_LOG_ERROR, "weird prediction\n");
                    return -1;
                }
            }
        } else {    /* mb_type == 33, DC_128_PRED block type */
            for (i = 0; i < 4; i++)
                memset(&s->intra4x4_pred_mode_cache[scan8[0] + 8 * i], DC_PRED, 4);
        }



        } else {
            for (i = 0; i < 4; i++)
                memset(&s->intra4x4_pred_mode_cache[scan8[0] + 8 * i], DC_128_PRED, 4);

            s->top_samples_available  = 0x33FF;
            s->left_samples_available = 0x5F5F;
        }

        mb_type = MB_TYPE_INTRA4x4;
    } else {                      /* INTRA16x16 */

                                                                     s->left_samples_available, dir, 0)) < 0) {
            av_log(s->avctx, AV_LOG_ERROR, "ff_h264_check_intra_pred_mode < 0\n");
            return s->intra16x16_pred_mode;
        }

    }

                   0, 4 * 2 * sizeof(int16_t));
                       0, 4 * 2 * sizeof(int16_t));
        }
    }
    }
    }

            av_log(s->avctx, AV_LOG_ERROR, "cbp_vlc=%"PRIu32"\n", vlc);
            return -1;
        }

    }

            av_log(s->avctx, AV_LOG_ERROR, "qscale:%d\n", s->qscale);
            return -1;
        }
    }
            av_log(s->avctx, AV_LOG_ERROR,
                   "error while decoding intra luma dc\n");
            return -1;
        }
    }



                        av_log(s->avctx, AV_LOG_ERROR,
                               "error while decoding block\n");
                        return -1;
                    }
                }
            }

                    av_log(s->avctx, AV_LOG_ERROR,
                           "error while decoding chroma dc block\n");
                    return -1;
                }


                            av_log(s->avctx, AV_LOG_ERROR,
                                   "error while decoding chroma ac block\n");
                            return -1;
                        }
                    }
                }
            }
        }
    }



    return 0;
}

{


        /* TODO: what? */
        av_log(avctx, AV_LOG_ERROR, "unsupported slice header (%02X)\n", header);
        return -1;
    } else {



            return AVERROR(ENOMEM);

            av_log(avctx, AV_LOG_ERROR, "slice after bitstream end\n");
            return AVERROR_INVALIDDATA;
        }

        }

        }
    }

        av_log(s->avctx, AV_LOG_ERROR, "illegal slice type %u \n", slice_id);
        return -1;
    }


        i = (s->mb_num < 64) ? 6 : (1 + av_log2(s->mb_num - 1));
        get_bits(&s->gb_slice, i);
        avpriv_report_missing_feature(s->avctx, "Media key encryption");
        return AVERROR_PATCHWELCOME;
    }


    /* unknown fields */



        return AVERROR_INVALIDDATA;

    /* reset intra predictors and invalidate motion vector references */
        memset(s->intra4x4_pred_mode + s->mb2br_xy[mb_xy - 1] + 3,
               -1, 4 * sizeof(int8_t));
               -1, 8 * sizeof(int8_t) * s->mb_x);
    }
        memset(s->intra4x4_pred_mode + s->mb2br_xy[mb_xy - s->mb_stride],
               -1, 8 * sizeof(int8_t) * (s->mb_width - s->mb_x));

        if (s->mb_x > 0)
            s->intra4x4_pred_mode[s->mb2br_xy[mb_xy - s->mb_stride - 1] + 3] = -1;
    }

    return 0;
}

{

    }

{

        ret = AVERROR(ENOMEM);
        goto fail;
    }

        return AVERROR(ENOMEM);







    /* prowl for the "SEQH" marker in the extradata */
                marker_found = 1;
                break;
            }
        }
    }

    /* if a match was found, parse the extra data */

            ret = AVERROR_INVALIDDATA;
            goto fail;
        }

        /* 'frame size code' and optional 'width, height' */
        case 0:
            w = 160;
            h = 120;
            break;
        case 1:
            w = 128;
            h =  96;
            break;
        case 2:
            w = 176;
            h = 144;
            break;
        case 3:
            w = 352;
            h = 288;
            break;
        case 4:
            w = 704;
            h = 576;
            break;
        case 5:
            w = 240;
            h = 180;
            break;
        }
            goto fail;


        /* unknown fields */


        /* unknown field */

               unk0, unk1, unk2, unk3, unk4);

            ret = AVERROR_INVALIDDATA;
            goto fail;
        }

#if CONFIG_ZLIB

                ret = -1;
                goto fail;
            }

                ret = AVERROR(ENOMEM);
                goto fail;
            }
                   watermark_width, watermark_height);
                   "u1: %x u2: %x u3: %x compressed data size: %d offset: %d\n",
                   u1, u2, u3, u4, offset);
                av_log(avctx, AV_LOG_ERROR,
                       "could not uncompress watermark logo\n");
                av_free(buf);
                ret = -1;
                goto fail;
            }

                   "watermark key %#"PRIx32"\n", s->watermark_key);
#else
            av_log(avctx, AV_LOG_ERROR,
                   "this svq3 file contains watermark which need zlib support compiled in\n");
            ret = -1;
            goto fail;
#endif
        }
    }


        return AVERROR(ENOMEM);

                             sizeof(*s->mb2br_xy));
        return AVERROR(ENOMEM);


        }


fail:
    svq3_decode_end(avctx);
    return ret;
}

static void free_picture(AVCodecContext *avctx, SVQ3Frame *pic)
{
    int i;
    for (i = 0; i < 2; i++) {
        av_buffer_unref(&pic->motion_val_buf[i]);
        av_buffer_unref(&pic->ref_index_buf[i]);
    }
    av_buffer_unref(&pic->mb_type_buf);

    av_frame_unref(pic->f);
}

{


            return AVERROR(ENOMEM);

                ret = AVERROR(ENOMEM);
                goto fail;
            }

        }
    }

                         AV_GET_BUFFER_FLAG_REF : 0);
        goto fail;

            return AVERROR(ENOMEM);
    }

    return 0;
fail:
    free_picture(avctx, pic);
    return ret;
}

                             int *got_frame, AVPacket *avpkt)
{

    /* special case for last picture */
                return ret;
        }
    }


            return AVERROR(ENOMEM);
    } else {
    }

        return ret;

        return -1;




    /* for skipping the frame */

        return ret;

    }
    }

            av_log(avctx, AV_LOG_ERROR, "Missing reference frame.\n");
            av_frame_unref(s->last_pic->f);
            ret = get_buffer(avctx, s->last_pic);
            if (ret < 0)
                return ret;
            memset(s->last_pic->f->data[0], 0, avctx->height * s->last_pic->f->linesize[0]);
            memset(s->last_pic->f->data[1], 0x80, (avctx->height / 2) *
                   s->last_pic->f->linesize[1]);
                   s->last_pic->f->linesize[2]);
        }

            av_log(avctx, AV_LOG_ERROR, "Missing reference frame.\n");
            av_frame_unref(s->next_pic->f);
            ret = get_buffer(avctx, s->next_pic);
            if (ret < 0)
                return ret;
            memset(s->next_pic->f->data[0], 0, avctx->height * s->next_pic->f->linesize[0]);
            memset(s->next_pic->f->data[1], 0x80, (avctx->height / 2) *
                   s->next_pic->f->linesize[1]);
                   s->next_pic->f->linesize[2]);
        }
    }

        av_log(s->avctx, AV_LOG_DEBUG,
               "%c hpel:%d, tpel:%d aqp:%d qp:%d, slice_num:%02X\n",
               av_get_picture_type_char(s->pict_type),
               s->halfpel_flag, s->thirdpel_flag,
               s->adaptive_quant, s->qscale, s->slice_num);

        avctx->skip_frame >= AVDISCARD_ALL)
        return 0;

        if (s->pict_type == AV_PICTURE_TYPE_B)
            return 0;
        else
            s->next_p_frame_damaged = 0;
    }


            s->frame_num_offset += 256;
            av_log(s->avctx, AV_LOG_ERROR, "error in B-frame picture id\n");
            return -1;
        }
    } else {

    }

        int i;
            int j;
        }
    }



                    if (svq3_decode_slice_header(avctx))
                        return -1;
                }
                    avpriv_request_sample(avctx, "non constant slice type");
                }
                /* TODO: support s->mb_skip_run */
            }


                av_log(s->avctx, AV_LOG_ERROR,
                       "error while decoding MB %d %d\n", s->mb_x, s->mb_y);
                return -1;
            }


        }

                           s->low_delay);
    }


        av_log(avctx, AV_LOG_INFO, "frame num %d incomplete pic x %d y %d left %d\n", avctx->frame_number, s->mb_y, s->mb_x, left);
        //av_hex_dump(stderr, buf+buf_size-8, 8);
    }

        av_log(avctx, AV_LOG_ERROR, "frame num %d left %d\n", avctx->frame_number, left);
        return -1;
    }

        return ret;

    /* Do not output the last pic after seeking. */

    } else {
    }

    return buf_size;
}

{




}

AVCodec ff_svq3_decoder = {
    .name           = "svq3",
    .long_name      = NULL_IF_CONFIG_SMALL("Sorenson Vector Quantizer 3 / Sorenson Video 3 / SVQ3"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_SVQ3,
    .priv_data_size = sizeof(SVQ3Context),
    .init           = svq3_decode_init,
    .close          = svq3_decode_end,
    .decode         = svq3_decode_frame,
    .capabilities   = AV_CODEC_CAP_DRAW_HORIZ_BAND |
                      AV_CODEC_CAP_DR1             |
                      AV_CODEC_CAP_DELAY,
    .pix_fmts       = (const enum AVPixelFormat[]) { AV_PIX_FMT_YUVJ420P,
                                                     AV_PIX_FMT_NONE},
};
