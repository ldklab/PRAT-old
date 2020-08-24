/*
 * Indeo Video Interactive v5 compatible decoder
 * Copyright (c) 2009 Maxim Poliakovski
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
 * Indeo Video Interactive version 5 decoder
 *
 * Indeo5 data is usually transported within .avi or .mov files.
 * Known FOURCCs: 'IV50'
 */

#define BITSTREAM_READER_LE
#include "avcodec.h"
#include "get_bits.h"
#include "ivi.h"
#include "ivi_dsp.h"
#include "indeo5data.h"

/**
 *  Indeo5 frame types.
 */
enum {
    FRAMETYPE_INTRA       = 0,
    FRAMETYPE_INTER       = 1,  ///< non-droppable P-frame
    FRAMETYPE_INTER_SCAL  = 2,  ///< droppable P-frame used in the scalability mode
    FRAMETYPE_INTER_NOREF = 3,  ///< droppable P-frame
    FRAMETYPE_NULL        = 4   ///< empty frame with no data
};

#define IVI5_PIC_SIZE_ESC       15

/**
 *  Decode Indeo5 GOP (Group of pictures) header.
 *  This header is present in key frames only.
 *  It defines parameters for all frames in a GOP.
 *
 *  @param[in,out] ctx    ptr to the decoder context
 *  @param[in]     avctx  ptr to the AVCodecContext
 *  @return         result code: 0 = OK, -1 = error
 */
{



        ctx->lock_word = get_bits_long(&ctx->gb, 32);

    if (tile_size > 256) {
        av_log(avctx, AV_LOG_ERROR, "Invalid tile size: %d\n", tile_size);
        return AVERROR_INVALIDDATA;
    }

    /* decode number of wavelet bands */
    /* num_levels * 3 + 1 */
        av_log(avctx, AV_LOG_ERROR, "Scalability: unsupported subdivision! Luma bands: %d, chroma bands: %d\n",
               pic_conf.luma_bands, pic_conf.chroma_bands);
        return AVERROR_INVALIDDATA;
    }

        pic_conf.pic_height = get_bits(&ctx->gb, 13);
        pic_conf.pic_width  = get_bits(&ctx->gb, 13);
    } else {
    }

        avpriv_report_missing_feature(avctx, "YV12 picture format");
        return AVERROR_PATCHWELCOME;
    }


    } else {
        pic_conf.tile_height = pic_conf.tile_width = tile_size;
    }

    /* check if picture layout was changed and reallocate buffers */
            av_log(avctx, AV_LOG_ERROR, "Couldn't reallocate color planes!\n");
            return result;
        }
    }




                av_log(avctx, AV_LOG_ERROR, "4x4 luma blocks are unsupported!\n");
                return AVERROR_PATCHWELCOME;
            }

            if (blk_size_changed) {
            }

                avpriv_report_missing_feature(avctx, "Extended transform info");
                return AVERROR_PATCHWELCOME;
            }

            /* select transform function and scan pattern according to plane and band number */

            case 1:
                band->inv_transform  = ff_ivi_row_slant8;
                band->dc_transform   = ff_ivi_dc_row_slant;
                band->scan           = ff_ivi_vertical_scan_8x8;
                band->transform_size = 8;
                break;

            case 2:
                band->inv_transform  = ff_ivi_col_slant8;
                band->dc_transform   = ff_ivi_dc_col_slant;
                band->scan           = ff_ivi_horizontal_scan_8x8;
                band->transform_size = 8;
                break;

            case 3:
                band->inv_transform  = ff_ivi_put_pixels_8x8;
                band->dc_transform   = ff_ivi_put_dc_pixel_8x8;
                band->scan           = ff_ivi_horizontal_scan_8x8;
                band->transform_size = 8;
                break;

            }

                                band->inv_transform == ff_ivi_inverse_slant_4x4;

                av_log(avctx, AV_LOG_ERROR, "transform and block size mismatch (%d != %d)\n", band->transform_size, band->blk_size);
                return AVERROR_INVALIDDATA;
            }

            /* select dequant matrix according to plane and band number */
            } else {
                quant_mat = 5;
            }

                    av_log(avctx, AV_LOG_ERROR, "quant_mat %d too large!\n", quant_mat);
                    return -1;
                }
            } else {
            }

                av_log(avctx, AV_LOG_ERROR, "End marker missing!\n");
                return AVERROR_INVALIDDATA;
            }
        }
    }

    /* copy chroma parameters into the 2nd chroma plane */

    }

    /* reallocate internal structures if needed */
            av_log(avctx, AV_LOG_ERROR,
                   "Couldn't reallocate internal structures!\n");
            return result;
        }
    }

        if (get_bits(&ctx->gb, 3)) {
            av_log(avctx, AV_LOG_ERROR, "Alignment bits are not zero!\n");
            return AVERROR_INVALIDDATA;
        }

        if (get_bits1(&ctx->gb))
            skip_bits(&ctx->gb, 24); /* skip transparency fill color */
    }



    /* skip GOP extension if any */
        do {
            i = get_bits(&ctx->gb, 16);
        } while (i & 0x8000);
    }


}


/**
 *  Skip a header extension.
 *
 *  @param[in,out]  gb  the GetBit context
 */
static inline int skip_hdr_extension(GetBitContext *gb)
{
    int i, len;

    do {
        len = get_bits(gb, 8);
        if (8*len > get_bits_left(gb))
            return AVERROR_INVALIDDATA;
        for (i = 0; i < len; i++) skip_bits(gb, 8);
    } while(len);

    return 0;
}


/**
 *  Decode Indeo5 picture header.
 *
 *  @param[in,out]  ctx    ptr to the decoder context
 *  @param[in]      avctx  ptr to the AVCodecContext
 *  @return         result code: 0 = OK, -1 = error
 */
{

        av_log(avctx, AV_LOG_ERROR, "Invalid picture start code!\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "Invalid frame type: %d \n", ctx->frame_type);
        ctx->frame_type = FRAMETYPE_INTRA;
        return AVERROR_INVALIDDATA;
    }


            av_log(avctx, AV_LOG_ERROR, "Invalid GOP header, skipping frames.\n");
            ctx->gop_invalid = 1;
            return ret;
        }
    }

        av_log(avctx, AV_LOG_ERROR, "Scalable inter frame in non scalable stream\n");
        ctx->frame_type = FRAMETYPE_INTER;
        return AVERROR_INVALIDDATA;
    }




        /* skip unknown extension if any */
            skip_hdr_extension(&ctx->gb); /* XXX: untested */

        /* decode macroblock huffman codebook */
                                   IVI_MB_HUFF, &ctx->mb_vlc, avctx);
            return ret;

    }


}


/**
 *  Decode Indeo5 band header.
 *
 *  @param[in,out]  ctx    ptr to the decoder context
 *  @param[in,out]  band   ptr to the band descriptor
 *  @param[in]      avctx  ptr to the AVCodecContext
 *  @return         result code: 0 = OK, -1 = error
 */
                           AVCodecContext *avctx)
{


        band->is_empty = 1;
        return 0;
    }



    /* decode rvmap probability corrections if any */
            av_log(avctx, AV_LOG_ERROR, "Too many corrections: %d\n",
                   band->num_corr);
            return AVERROR_INVALIDDATA;
        }

        /* read correction pairs */
    }

    /* select appropriate rvmap table for this band */

    /* decode block huffman codebook */
                               &band->blk_vlc, avctx);
        return ret;



    /* skip unknown extension if any */
        align_get_bits(&ctx->gb);
        skip_hdr_extension(&ctx->gb);
    }


}


/**
 *  Decode info (block type, cbp, quant delta, motion vector)
 *  for all macroblocks in the current tile.
 *
 *  @param[in,out]  ctx    ptr to the decoder context
 *  @param[in,out]  band   ptr to the band descriptor
 *  @param[in,out]  tile   ptr to the tile descriptor
 *  @param[in]      avctx  ptr to the AVCodecContext
 *  @return         result code: 0 = OK, -1 = error
 */
                          IVITile *tile, AVCodecContext *avctx)
{
                mv_scale, blks_per_mb, s;


        return AVERROR_INVALIDDATA;

        av_log(avctx, AV_LOG_ERROR, "Allocated tile size %d mismatches parameters %d\n",
               tile->num_MBs, IVI_MBs_PER_TILE(tile->width, tile->height, band->mb_size));
        return AVERROR_INVALIDDATA;
    }

    /* scale factor for motion vectors */

        mb_offset = offs;


                    av_log(avctx, AV_LOG_ERROR, "Empty macroblock in an INTRA picture!\n");
                    return AVERROR_INVALIDDATA;
                }

                    mb->q_delta = get_vlc2(&ctx->gb, ctx->mb_vlc.tab->table,
                                           IVI_VLC_BITS, 1);
                    mb->q_delta = IVI_TOSIGNED(mb->q_delta);
                }

                    /* motion vector inheritance */
                    } else {
                        mb->mv_x = ref_mb->mv_x;
                        mb->mv_y = ref_mb->mv_y;
                    }
                }
            } else {
                } else {
                }


                    if (band->inherit_qdelta) {
                        if (ref_mb) mb->q_delta = ref_mb->q_delta;
                    } else if (mb->cbp || (!band->plane && !band->band_num &&
                                           (ctx->frame_flags & 8))) {
                        mb->q_delta = get_vlc2(&ctx->gb, ctx->mb_vlc.tab->table,
                                               IVI_VLC_BITS, 1);
                        mb->q_delta = IVI_TOSIGNED(mb->q_delta);
                    }
                }

                } else {
                        /* motion vector inheritance */
                        } else {
                            mb->mv_x = ref_mb->mv_x;
                            mb->mv_y = ref_mb->mv_y;
                        }
                    } else {
                        /* decode motion vector deltas */
                                            IVI_VLC_BITS, 1);
                                            IVI_VLC_BITS, 1);
                    }
                }
            }

                av_log(avctx, AV_LOG_ERROR, "motion vector %d %d outside reference\n", x*s + mb->mv_x, y*s + mb->mv_y);
                return AVERROR_INVALIDDATA;
            }

        }

    }


}


/**
 *  Switch buffers.
 *
 *  @param[in,out] ctx  ptr to the decoder context
 */
{
    case FRAMETYPE_INTER:
    case FRAMETYPE_INTER_SCAL:
        if (!ctx->inter_scal) {
            ctx->ref2_buf   = 2;
            ctx->inter_scal = 1;
        }
        FFSWAP(int, ctx->dst_buf, ctx->ref2_buf);
        ctx->ref_buf = ctx->ref2_buf;
        break;
    case FRAMETYPE_INTER_NOREF:
        break;
    }

        /* FALLTHROUGH */
    case FRAMETYPE_INTER_SCAL:
    case FRAMETYPE_INTER_NOREF:
    case FRAMETYPE_NULL:
        break;
    }


{
}


/**
 *  Initialize Indeo5 decoder.
 */
{



    /* copy rvmap tables in our context so we can apply changes to them */

    /* set the initial picture layout according to the basic profile:
       there is only one band per plane (no scalability), only one tile (no local decoding)
       and picture format = YVU9 */

        av_log(avctx, AV_LOG_ERROR, "Couldn't allocate color planes!\n");
        return AVERROR_INVALIDDATA;
    }





}

AVCodec ff_indeo5_decoder = {
    .name           = "indeo5",
    .long_name      = NULL_IF_CONFIG_SMALL("Intel Indeo Video Interactive 5"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_INDEO5,
    .priv_data_size = sizeof(IVI45DecContext),
    .init           = decode_init,
    .close          = ff_ivi_decode_close,
    .decode         = ff_ivi_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
