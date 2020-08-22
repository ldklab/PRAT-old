/*
 * Indeo Video Interactive v4 compatible decoder
 * Copyright (c) 2009-2011 Maxim Poliakovski
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
 * Indeo Video Interactive version 4 decoder
 *
 * Indeo 4 data is usually transported within .avi or .mov files.
 * Known FOURCCs: 'IV41'
 */

#define BITSTREAM_READER_LE
#include "avcodec.h"
#include "get_bits.h"
#include "libavutil/imgutils.h"
#include "indeo4data.h"
#include "internal.h"
#include "ivi.h"
#include "ivi_dsp.h"

#define IVI4_PIC_SIZE_ESC   7


static const struct {
    InvTransformPtr *inv_trans;
    DCTransformPtr  *dc_trans;
    int             is_2d_trans;
} transforms[18] = {
    { ff_ivi_inverse_haar_8x8,  ff_ivi_dc_haar_2d,       1 },
    { ff_ivi_row_haar8,         ff_ivi_dc_haar_2d,       0 },
    { ff_ivi_col_haar8,         ff_ivi_dc_haar_2d,       0 },
    { ff_ivi_put_pixels_8x8,    ff_ivi_put_dc_pixel_8x8, 1 },
    { ff_ivi_inverse_slant_8x8, ff_ivi_dc_slant_2d,      1 },
    { ff_ivi_row_slant8,        ff_ivi_dc_row_slant,     1 },
    { ff_ivi_col_slant8,        ff_ivi_dc_col_slant,     1 },
    { NULL, NULL, 0 }, /* inverse DCT 8x8 */
    { NULL, NULL, 0 }, /* inverse DCT 8x1 */
    { NULL, NULL, 0 }, /* inverse DCT 1x8 */
    { ff_ivi_inverse_haar_4x4,  ff_ivi_dc_haar_2d,       1 },
    { ff_ivi_inverse_slant_4x4, ff_ivi_dc_slant_2d,      1 },
    { NULL, NULL, 0 }, /* no transform 4x4 */
    { ff_ivi_row_haar4,         ff_ivi_dc_haar_2d,       0 },
    { ff_ivi_col_haar4,         ff_ivi_dc_haar_2d,       0 },
    { ff_ivi_row_slant4,        ff_ivi_dc_row_slant,     0 },
    { ff_ivi_col_slant4,        ff_ivi_dc_col_slant,     0 },
    { NULL, NULL, 0 }, /* inverse DCT 4x4 */
};

/**
 *  Decode subdivision of a plane.
 *  This is a simplified version that checks for two supported subdivisions:
 *  - 1 wavelet band  per plane, size factor 1:1, code pattern: 3
 *  - 4 wavelet bands per plane, size factor 1:4, code pattern: 2,3,3,3,3
 *  Anything else is either unsupported or corrupt.
 *
 *  @param[in,out] gb    the GetBit context
 *  @return        number of wavelet bands or 0 on error
 */
{

    case 3:
        return 1;
    case 2:
        for (i = 0; i < 4; i++)
            if (get_bits(gb, 2) != 3)
                return 0;
        return 4;
    default:
        return 0;
    }
}

static inline int scale_tile_size(int def_size, int size_factor)
{
    return size_factor == 15 ? def_size : (size_factor + 1) << 5;
}

/**
 *  Decode Indeo 4 picture header.
 *
 *  @param[in,out] ctx       pointer to the decoder context
 *  @param[in]     avctx     pointer to the AVCodecContext
 *  @return        result code: 0 = OK, negative number = error
 */
{

        av_log(avctx, AV_LOG_ERROR, "Invalid picture start code!\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "Invalid frame type: %d\n", ctx->frame_type);
        return AVERROR_INVALIDDATA;
    }

        ctx->has_b_frames = 1;


    /* unknown bit: Mac decoder ignores this bit, XANIM returns error */
        av_log(avctx, AV_LOG_ERROR, "Sync bit is set!\n");
        return AVERROR_INVALIDDATA;
    }


    /* null frames don't contain anything else so we just return */
        ff_dlog(avctx, "Null frame encountered!\n");
        return 0;
    }

    /* Check key lock status. If enabled - ignore lock word.         */
    /* Usually we have to prompt the user for the password, but      */
    /* we don't do that because Indeo 4 videos can be decoded anyway */
        skip_bits_long(&ctx->gb, 32);
    }

        pic_conf.pic_height = get_bits(&ctx->gb, 16);
        pic_conf.pic_width  = get_bits(&ctx->gb, 16);
    } else {
    }

    /* Decode tile dimensions. */
        pic_conf.tile_height = scale_tile_size(pic_conf.pic_height, get_bits(&ctx->gb, 4));
        pic_conf.tile_width  = scale_tile_size(pic_conf.pic_width,  get_bits(&ctx->gb, 4));
    } else {
    }

    /* Decode chroma subsampling. We support only 4:4 aka YVU9. */
        av_log(avctx, AV_LOG_ERROR, "Only YVU9 picture format is supported!\n");
        return AVERROR_INVALIDDATA;
    }

    /* decode subdivision of the planes */

        av_log(avctx, AV_LOG_ERROR, "picture dimensions %d %d cannot be decoded\n",
               pic_conf.pic_width, pic_conf.pic_height);
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "Scalability: unsupported subdivision! Luma bands: %d, chroma bands: %d\n",
               pic_conf.luma_bands, pic_conf.chroma_bands);
        return AVERROR_INVALIDDATA;
    }

    /* check if picture layout was changed and reallocate buffers */
            av_log(avctx, AV_LOG_ERROR, "Couldn't reallocate color planes!\n");
            ctx->pic_conf.luma_bands = 0;
            return AVERROR(ENOMEM);
        }


        /* set default macroblock/block dimensions */
            }
        }

            av_log(avctx, AV_LOG_ERROR,
                   "Couldn't reallocate internal structures!\n");
            return AVERROR(ENOMEM);
        }
    }


    /* skip decTimeEst field if present */
        skip_bits(&ctx->gb, 8);

    /* decode macroblock and block huffman codebooks */
        return AVERROR_INVALIDDATA;




    /* TODO: ignore this parameter if unused */


    /* skip picture header extension if any */
            return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "Bad blocks bits encountered!\n");
    }


}


/**
 *  Decode Indeo 4 band header.
 *
 *  @param[in,out] ctx       pointer to the decoder context
 *  @param[in,out] band      pointer to the band descriptor
 *  @param[in]     avctx     pointer to the AVCodecContext
 *  @return        result code: 0 = OK, negative number = error
 */
                           AVCodecContext *avctx)
{

        av_log(avctx, AV_LOG_ERROR, "Invalid band header sequence!\n");
        return AVERROR_INVALIDDATA;
    }

        /* skip header size
         * If header size is not given, header size is 4 bytes. */

            av_log(avctx, AV_LOG_ERROR, "Invalid/unsupported mv resolution: %d!\n",
                   band->is_halfpel);
            return AVERROR_INVALIDDATA;
        }
            ctx->uses_fullpel = 1;


            av_log(avctx, AV_LOG_ERROR, "Invalid block size!\n");
            return AVERROR_INVALIDDATA;
        }



                avpriv_request_sample(avctx, "Transform %d", transform_id);
                return AVERROR_PATCHWELCOME;
            }
                avpriv_request_sample(avctx, "DCT transform");
                return AVERROR_PATCHWELCOME;
            }

                av_log(avctx, AV_LOG_ERROR, "wrong transform size!\n");
                return AVERROR_INVALIDDATA;
            }
                ctx->uses_haar = 1;


            else

                av_log(avctx, AV_LOG_ERROR, "transform and block size mismatch (%d != %d)\n", band->transform_size, band->blk_size);
                return AVERROR_INVALIDDATA;
            }

                av_log(avctx, AV_LOG_ERROR, "Custom scan pattern encountered!\n");
                return AVERROR_INVALIDDATA;
            }
                    av_log(avctx, AV_LOG_ERROR, "mismatching scan table!\n");
                    return AVERROR_INVALIDDATA;
                }
                av_log(avctx, AV_LOG_ERROR, "mismatching scan table!\n");
                return AVERROR_INVALIDDATA;
            }


                av_log(avctx, AV_LOG_ERROR, "Custom quant matrix encountered!\n");
                return AVERROR_INVALIDDATA;
            }
                avpriv_request_sample(avctx, "Quantization matrix %d",
                                      quant_mat);
                return AVERROR_INVALIDDATA;
            }
        } else {
                av_log(avctx, AV_LOG_ERROR,
                       "The band block size does not match the configuration "
                       "inherited\n");
                return AVERROR_INVALIDDATA;
            }
        }
            av_log(avctx, AV_LOG_ERROR, "Invalid quant matrix for 4x4 block encountered!\n");
            band->quant_mat = 0;
            return AVERROR_INVALIDDATA;
        }
            av_log(avctx, AV_LOG_ERROR, "mismatching scan table!\n");
            return AVERROR_INVALIDDATA;
        }
            av_log(avctx, AV_LOG_ERROR, "mismatching transform_size!\n");
            return AVERROR_INVALIDDATA;
        }

        /* decode block huffman codebook */
            arg_band->blk_vlc.tab = ctx->blk_vlc.tab;
        else
                                     &arg_band->blk_vlc, avctx))
                return AVERROR_INVALIDDATA;

        /* select appropriate rvmap table for this band */

        /* decode rvmap probability corrections if any */
                av_log(avctx, AV_LOG_ERROR, "Too many corrections: %d\n",
                       band->num_corr);
                return AVERROR_INVALIDDATA;
            }

            /* read correction pairs */
        }
    }

    } else {
    }

    /* Indeo 4 doesn't use scale tables */


        av_log(avctx, AV_LOG_ERROR, "band->scan not set\n");
        return AVERROR_INVALIDDATA;
    }


}


/**
 *  Decode information (block type, cbp, quant delta, motion vector)
 *  for all macroblocks in the current tile.
 *
 *  @param[in,out] ctx       pointer to the decoder context
 *  @param[in,out] band      pointer to the band descriptor
 *  @param[in,out] tile      pointer to the tile descriptor
 *  @param[in]     avctx     pointer to the AVCodecContext
 *  @return        result code: 0 = OK, negative number = error
 */
                          IVITile *tile, AVCodecContext *avctx)
{
                mv_scale, mb_type_bits, s;



    /* scale factor for motion vectors */

        av_log(avctx, AV_LOG_ERROR, "num_MBs mismatch %d %d %d %d\n", tile->width, tile->height, band->mb_size, tile->num_MBs);
        return -1;
    }

        mb_offset = offs;


                av_log(avctx, AV_LOG_ERROR, "Insufficient input for mb info\n");
                return AVERROR_INVALIDDATA;
            }

                    av_log(avctx, AV_LOG_ERROR, "Empty macroblock in an INTRA picture!\n");
                    return AVERROR_INVALIDDATA;
                }

                                           IVI_VLC_BITS, 1);
                }

                    /* motion vector inheritance */
                    } else {
                        mb->mv_x = ref_mb->mv_x;
                        mb->mv_y = ref_mb->mv_y;
                    }
                }
            } else {
                    /* copy mb_type from corresponding reference mb */
                        av_log(avctx, AV_LOG_ERROR, "ref_mb unavailable\n");
                        return AVERROR_INVALIDDATA;
                    }
                           ctx->frame_type == IVI4_FRAMETYPE_INTRA1) {
                } else {
                }


                                           IVI_VLC_BITS, 1);
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
                            mv_delta = get_vlc2(&ctx->gb,
                                                ctx->mb_vlc.tab->table,
                                                IVI_VLC_BITS, 1);
                            mv_y += IVI_TOSIGNED(mv_delta);
                            mv_delta = get_vlc2(&ctx->gb,
                                                ctx->mb_vlc.tab->table,
                                                IVI_VLC_BITS, 1);
                            mv_x += IVI_TOSIGNED(mv_delta);
                            mb->b_mv_x = -mv_x;
                            mb->b_mv_y = -mv_y;
                        }
                    }
                        mb->b_mv_x = -mb->mv_x;
                        mb->b_mv_y = -mb->mv_y;
                        mb->mv_x = 0;
                        mb->mv_y = 0;
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
 *  Rearrange decoding and reference buffers.
 *
 *  @param[in,out] ctx       pointer to the decoder context
 */
{

    case IVI4_FRAMETYPE_INTRA1:
    case IVI4_FRAMETYPE_INTER:
    }

    case IVI4_FRAMETYPE_INTRA1:
    case IVI4_FRAMETYPE_INTER:
    }

    }


{
}


{


    /* copy rvmap tables in our context so we can apply changes to them */

    /* Force allocation of the internal buffers */
    /* during picture header decoding.          */




        return AVERROR(ENOMEM);

    return 0;
}


AVCodec ff_indeo4_decoder = {
    .name           = "indeo4",
    .long_name      = NULL_IF_CONFIG_SMALL("Intel Indeo Video Interactive 4"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_INDEO4,
    .priv_data_size = sizeof(IVI45DecContext),
    .init           = decode_init,
    .close          = ff_ivi_decode_close,
    .decode         = ff_ivi_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
