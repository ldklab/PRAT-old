/*
 * H.26L/H.264/AVC/JVT/14496-10/... parser
 * Copyright (c) 2003 Michael Niedermayer <michaelni@gmx.at>
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
 * H.264 / AVC / MPEG-4 part10 parser.
 * @author Michael Niedermayer <michaelni@gmx.at>
 */

#define UNCHECKED_BITSTREAM_READER 1

#include <assert.h>
#include <stdint.h>

#include "libavutil/avutil.h"
#include "libavutil/error.h"
#include "libavutil/log.h"
#include "libavutil/mem.h"
#include "libavutil/pixfmt.h"

#include "avcodec.h"
#include "get_bits.h"
#include "golomb.h"
#include "h264.h"
#include "h264_sei.h"
#include "h264_ps.h"
#include "h264data.h"
#include "internal.h"
#include "mpegutils.h"
#include "parser.h"

typedef struct H264ParseContext {
    ParseContext pc;
    H264ParamSets ps;
    H264DSPContext h264dsp;
    H264POCContext poc;
    H264SEIContext sei;
    int is_avc;
    int nal_length_size;
    int got_first;
    int picture_structure;
    uint8_t parse_history[6];
    int parse_history_count;
    int parse_last_mb;
    int64_t reference_dts;
    int last_frame_num, last_picture_structure;
} H264ParseContext;


                               int buf_size, void *logctx)
{

//    mb_addr= pc->mb_addr - 1;

        av_log(logctx, AV_LOG_ERROR, "AVC-parser: nal length size invalid\n");

            int nalsize = 0;
            i = next_avc;
            for (j = 0; j < p->nal_length_size; j++)
                nalsize = (nalsize << 8) | buf[i++];
            if (nalsize <= 0 || nalsize > buf_size - i) {
                av_log(logctx, AV_LOG_ERROR, "AVC-parser: nal size %d remaining %d\n", nalsize, buf_size - i);
                return buf_size;
            }
            next_avc = i + nalsize;
            state    = 5;
        }

                state = 7;
            else
                }
            }
            state = 7;
        } else {

                    }
                } else
            }
        }
    }
        return next_avc;
    return END_NOT_FOUND;

        return next_avc;
}

static int scan_mmco_reset(AVCodecParserContext *s, GetBitContext *gb,
                           void *logctx)
{
    H264PredWeightTable pwt;
    int slice_type_nos = s->pict_type & 3;
    H264ParseContext *p = s->priv_data;
    int list_count, ref_count[2];


    if (p->ps.pps->redundant_pic_cnt_present)
        get_ue_golomb(gb); // redundant_pic_count

    if (slice_type_nos == AV_PICTURE_TYPE_B)
        get_bits1(gb); // direct_spatial_mv_pred

    if (ff_h264_parse_ref_count(&list_count, ref_count, gb, p->ps.pps,
                                slice_type_nos, p->picture_structure, logctx) < 0)
        return AVERROR_INVALIDDATA;

    if (slice_type_nos != AV_PICTURE_TYPE_I) {
        int list;
        for (list = 0; list < list_count; list++) {
            if (get_bits1(gb)) {
                int index;
                for (index = 0; ; index++) {
                    unsigned int reordering_of_pic_nums_idc = get_ue_golomb_31(gb);

                    if (reordering_of_pic_nums_idc < 3)
                        get_ue_golomb_long(gb);
                    else if (reordering_of_pic_nums_idc > 3) {
                        av_log(logctx, AV_LOG_ERROR,
                               "illegal reordering_of_pic_nums_idc %d\n",
                               reordering_of_pic_nums_idc);
                        return AVERROR_INVALIDDATA;
                    } else
                        break;

                    if (index >= ref_count[list]) {
                        av_log(logctx, AV_LOG_ERROR,
                               "reference count %d overflow\n", index);
                        return AVERROR_INVALIDDATA;
                    }
                }
            }
        }
    }

    if ((p->ps.pps->weighted_pred && slice_type_nos == AV_PICTURE_TYPE_P) ||
        (p->ps.pps->weighted_bipred_idc == 1 && slice_type_nos == AV_PICTURE_TYPE_B))
        ff_h264_pred_weight_table(gb, p->ps.sps, ref_count, slice_type_nos,
                                  &pwt, p->picture_structure, logctx);

    if (get_bits1(gb)) { // adaptive_ref_pic_marking_mode_flag
        int i;
        for (i = 0; i < MAX_MMCO_COUNT; i++) {
            MMCOOpcode opcode = get_ue_golomb_31(gb);
            if (opcode > (unsigned) MMCO_LONG) {
                av_log(logctx, AV_LOG_ERROR,
                       "illegal memory management control operation %d\n",
                       opcode);
                return AVERROR_INVALIDDATA;
            }
            if (opcode == MMCO_END)
               return 0;
            else if (opcode == MMCO_RESET)
                return 1;

            if (opcode == MMCO_SHORT2UNUSED || opcode == MMCO_SHORT2LONG)
                get_ue_golomb_long(gb); // difference_of_pic_nums_minus1
            if (opcode == MMCO_SHORT2LONG || opcode == MMCO_LONG2UNUSED ||
                opcode == MMCO_LONG || opcode == MMCO_SET_MAX_LONG)
                get_ue_golomb_31(gb);
        }
    }

    return 0;
}

/**
 * Parse NAL units of found picture and decode some basic information.
 *
 * @param s parser context.
 * @param avctx codec context.
 * @param buf buffer with field/frame data.
 * @param buf_size size of the buffer.
 */
                                  AVCodecContext *avctx,
                                  const uint8_t * const buf, int buf_size)
{

    /* set some sane default values */


        return 0;

        return AVERROR(ENOMEM);


                break;
        } else {
                break;
                continue;
        }

        case H264_NAL_IDR_SLICE:
            // Do not walk the whole buffer just to decode slice header
                /* IDR or disposable slice
                 * No need to decode many bytes because MMCOs shall not be present. */
                    src_length = 60;
            } else {
                /* To decode up to MMCOs */
                    src_length = 1000;
            }
            break;
        }
            break;


            goto fail;

                                                 nal.size_bits);

        /* fall through */
                /* key frame, since recovery_frame_cnt is set */
            }
                av_log(avctx, AV_LOG_ERROR,
                       "pps_id %u out of range\n", pps_id);
                goto fail;
            }
                       "non-existing PPS %u referenced\n", pps_id);
            }

                goto fail;

            // heuristic to detect non marked keyframes


                s->width  = s->coded_width;
                s->height = s->coded_height;
            }

                break;
                break;
                break;
            default:
                s->format = AV_PIX_FMT_NONE;
            }


            } else {
                } else {
                }
            }


            }


            }

            /* Decode POC of this picture.
             * The prev_ values needed for decoding POC of the next picture are not set here. */
                             &p->poc, p->picture_structure, nal.ref_idc);
                goto fail;

            /* Continue parsing to check if MMCO_RESET is present.
             * FIXME: MMCO_RESET could appear in non-first slice.
             *        Maybe, we should parse all undisposable non-IDR slice of this
             *        picture until encountering MMCO_RESET in a slice of it. */
                    goto fail;
            }

            /* Set up the prev_ values for decoding POC of the next picture. */
                } else {
                }
            }

                                                         sps, avctx);
                    av_log(avctx, AV_LOG_ERROR, "Error processing the picture timing SEI\n");
                    p->sei.picture_timing.present = 0;
                }
            }

                case H264_SEI_PIC_STRUCT_BOTTOM_FIELD:
                case H264_SEI_PIC_STRUCT_TOP_BOTTOM:
                case H264_SEI_PIC_STRUCT_BOTTOM_TOP:
                case H264_SEI_PIC_STRUCT_BOTTOM_TOP_BOTTOM:
                case H264_SEI_PIC_STRUCT_FRAME_DOUBLING:
                    s->repeat_pict = 3;
                    break;
                case H264_SEI_PIC_STRUCT_FRAME_TRIPLING:
                    s->repeat_pict = 5;
                    break;
                default:
                    s->repeat_pict = p->picture_structure == PICT_FRAME ? 1 : 0;
                    break;
                }
            } else {
            }

                    case H264_SEI_PIC_STRUCT_TOP_BOTTOM_TOP:
                    case H264_SEI_PIC_STRUCT_BOTTOM_TOP_BOTTOM:
                    }
                } else {
                    else
                }
            } else {
                else
                    else
                } else {
                }
            }

        }
    }
        av_freep(&rbsp.rbsp_buffer);
        return 0;
    }
    /* didn't find a picture! */
}

                      AVCodecContext *avctx,
                      const uint8_t **poutbuf, int *poutbuf_size,
                      const uint8_t *buf, int buf_size)
{

                                     &p->ps, &p->is_avc, &p->nal_length_size,
                                     avctx->err_recognition, avctx);
        }
    }

    } else {

        }

        }
    }


    } else {
    }

        s->flags &= PARSER_FLAG_COMPLETE_FRAMES;
    }

                // got DTS from the stream, update reference timestamp
                // compute DTS based on reference timestamp
                s->dts = p->reference_dts + av_rescale(s->dts_ref_dts_delta, num, den);
            }

                s->pts = s->dts + av_rescale(s->pts_dts_delta, num, den);

        }
    }

}

static int h264_split(AVCodecContext *avctx,
                      const uint8_t *buf, int buf_size)
{
    uint32_t state = -1;
    int has_sps    = 0;
    int has_pps    = 0;
    const uint8_t *ptr = buf, *end = buf + buf_size;
    int nalu_type;

    while (ptr < end) {
        ptr = avpriv_find_start_code(ptr, end, &state);
        if ((state & 0xFFFFFF00) != 0x100)
            break;
        nalu_type = state & 0x1F;
        if (nalu_type == H264_NAL_SPS) {
            has_sps = 1;
        } else if (nalu_type == H264_NAL_PPS)
            has_pps = 1;
        /* else if (nalu_type == 0x01 ||
         *     nalu_type == 0x02 ||
         *     nalu_type == 0x05) {
         *  }
         */
        else if ((nalu_type != H264_NAL_SEI || has_pps) &&
                  nalu_type != H264_NAL_AUD && nalu_type != H264_NAL_SPS_EXT &&
                  nalu_type != 0x0f) {
            if (has_sps) {
                while (ptr - 4 > buf && ptr[-5] == 0)
                    ptr--;
                return ptr - 4 - buf;
            }
        }
    }

    return 0;
}

{



{

}

AVCodecParser ff_h264_parser = {
    .codec_ids      = { AV_CODEC_ID_H264 },
    .priv_data_size = sizeof(H264ParseContext),
    .parser_init    = init,
    .parser_parse   = h264_parse,
    .parser_close   = h264_close,
    .split          = h264_split,
};
