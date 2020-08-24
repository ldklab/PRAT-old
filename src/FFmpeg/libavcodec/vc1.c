/*
 * VC-1 and WMV3 decoder common code
 * Copyright (c) 2011 Mashiat Sarker Shakkhar
 * Copyright (c) 2006-2007 Konstantin Shishkov
 * Partly based on vc9.c (c) 2005 Anonymous, Alex Beregszaszi, Michael Niedermayer
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
 * VC-1 and WMV3 decoder common code
 */

#include "libavutil/attributes.h"
#include "internal.h"
#include "avcodec.h"
#include "mpegvideo.h"
#include "vc1.h"
#include "vc1data.h"
#include "wmv2data.h"
#include "unary.h"
#include "simple_idct.h"

/***********************************************************************/
/**
 * @name VC-1 Bitplane decoding
 * @see 8.7, p56
 * @{
 */

/** Decode rows by checking if they are skipped
 * @param plane Buffer to store decoded bits
 * @param[in] width Width of this buffer
 * @param[in] height Height of this buffer
 * @param[in] stride of this buffer
 */
                           GetBitContext *gb)
{

        else
    }

/** Decode columns by checking if they are skipped
 * @param plane Buffer to store decoded bits
 * @param[in] width Width of this buffer
 * @param[in] height Height of this buffer
 * @param[in] stride of this buffer
 * @todo FIXME: Optimize
 */
                           GetBitContext *gb)
{

        else
    }

/** Decode a bitplane's bits
 * @param data bitplane where to store the decode bits
 * @param[out] raw_flag pointer to the flag indicating that this bitplane is not coded explicitly
 * @param v VC-1 context for bit reading and logging
 * @return Status
 * @todo FIXME: Optimize
 */
{



        //Data is actually read in the MB layer (same for all tests == "raw")
    case IMODE_NORM2:
                offset = 0;
                planep += stride - width;
            }
        }
        else
            y = offset = 0;
        // decode bitplane as one long line
            }
            }
        }
        break;
    case IMODE_NORM6:
                        av_log(v->s.avctx, AV_LOG_DEBUG, "invalid NORM-6 VLC\n");
                        return -1;
                    }
                }
            }
        } else { // 3x2
                        av_log(v->s.avctx, AV_LOG_DEBUG, "invalid NORM-6 VLC\n");
                        return -1;
                    }
                }
            }
        }
        break;
    default:
        break;
    }

    /* Applying diff operator */
            }
        }
    }
}

/** @} */ //Bitplane group

/***********************************************************************/
/** VOP Dquant decoding
 * @param v VC-1 Context
 */
{

    //variable size
            return 0;

        case DQPROFILE_DOUBLE_EDGES:
        case DQPROFILE_ALL_MBS:
            }
        default:
            break; //Forbidden ?
        }
    }

    else

    return 0;
}

static int decode_sequence_header_adv(VC1Context *v, GetBitContext *gb);

/**
 * Decode Simple/Main Profiles sequence header
 * @see Figure 7-8, p16-17
 * @param avctx Codec context
 * @param gb GetBit context initialized from Codec context extra_data
 * @return Status
 */
{
        av_log(avctx, AV_LOG_WARNING, "WMV3 Complex Profile is not fully supported\n");
    }

    } else {
            av_log(avctx, AV_LOG_ERROR,
                   "Old interlaced mode is not supported\n");
            return -1;
        }
    }

    // (fps-2)/4 (->30)
    // (bitrate-32kbps)/64kbps
        av_log(avctx, AV_LOG_ERROR,
               "LOOPFILTER shall not be enabled in Simple Profile\n");
    }
        v->s.loop_filter = 0;

        v->vc1dsp.vc1_inv_trans_8x8    = ff_simple_idct_int16_8bit;
        v->vc1dsp.vc1_inv_trans_8x4    = ff_simple_idct84_add;
        v->vc1dsp.vc1_inv_trans_4x8    = ff_simple_idct48_add;
        v->vc1dsp.vc1_inv_trans_4x4    = ff_simple_idct44_add;
        v->vc1dsp.vc1_inv_trans_8x8_dc = ff_simple_idct_add_int16_8bit;
        v->vc1dsp.vc1_inv_trans_8x4_dc = ff_simple_idct84_add;
        v->vc1dsp.vc1_inv_trans_4x8_dc = ff_simple_idct48_add;
        v->vc1dsp.vc1_inv_trans_4x4_dc = ff_simple_idct44_add;
    }

        av_log(avctx, AV_LOG_ERROR,
               "FASTUVMC unavailable in Simple Profile\n");
        return -1;
    }
        av_log(avctx, AV_LOG_ERROR,
               "Extended MVs unavailable in Simple Profile\n");
        return -1;
    }

        av_log(avctx, AV_LOG_ERROR,
               "1 for reserved RES_TRANSTAB is forbidden\n");
        return -1;
    }


        av_log(avctx, AV_LOG_INFO,
               "RANGERED should be set to 0 in Simple Profile\n");
    }



        int w = get_bits(gb, 11);
        int h = get_bits(gb, 11);
        int ret = ff_set_dimensions(v->s.avctx, w, h);
        if (ret < 0) {
            av_log(avctx, AV_LOG_ERROR, "Failed to set dimensions %d %d\n", w, h);
            return ret;
        }
        skip_bits(gb, 5); //frame rate
        v->res_x8 = get_bits1(gb);
        if (get_bits1(gb)) { // something to do with DC VLC selection
            av_log(avctx, AV_LOG_ERROR, "Unsupported sprite feature\n");
            return -1;
        }
        skip_bits(gb, 3); //slice code
        v->res_rtm_flag = 0;
    } else {
    }
    //TODO: figure out what they mean (always 0x402F)
        skip_bits(gb, 16);
           "Profile %i:\nfrmrtq_postproc=%i, bitrtq_postproc=%i\n"
           "LoopFilter=%i, MultiRes=%i, FastUVMC=%i, Extended MV=%i\n"
           "Rangered=%i, VSTransform=%i, Overlap=%i, SyncMarker=%i\n"
           "DQuant=%i, Quantizer mode=%i, Max B-frames=%i\n",
           v->profile, v->frmrtq_postproc, v->bitrtq_postproc,
           v->s.loop_filter, v->multires, v->fastuvmc, v->extended_mv,
           v->rangered, v->vstransform, v->overlap, v->resync_marker,
           v->dquant, v->quantizer_mode, avctx->max_b_frames);
}

{
        av_log(v->s.avctx, AV_LOG_ERROR, "Reserved LEVEL %i\n",v->level);
    }
        av_log(v->s.avctx, AV_LOG_ERROR,
               "Only 4:2:0 chroma format supported\n");
        return -1;
    }

    // (fps-2)/4 (->30)
    // (bitrate-32kbps)/64kbps


           "Advanced Profile level %i:\nfrmrtq_postproc=%i, bitrtq_postproc=%i\n"
           "LoopFilter=%i, ChromaFormat=%i, Pulldown=%i, Interlace: %i\n"
           "TFCTRflag=%i, FINTERPflag=%i\n",
           v->level, v->frmrtq_postproc, v->bitrtq_postproc,
           v->s.loop_filter, v->chromaformat, v->broadcast, v->interlace,
           v->tfcntrflag, v->finterpflag);

        av_log(v->s.avctx, AV_LOG_ERROR, "Progressive Segmented Frame mode: not supported (yet)\n");
        return -1;
    }
        } else {
                avpriv_request_sample(v->s.avctx, "Huge resolution");
            } else
                      &v->s.avctx->sample_aspect_ratio.den,
                      1 << 30);
        }
               v->s.avctx->sample_aspect_ratio.num,

                v->s.avctx->framerate.den = 32;
                v->s.avctx->framerate.num = get_bits(gb, 16) + 1;
            } else {
                }
            }
            }
        }

            v->color_prim    = get_bits(gb, 8);
            v->transfer_char = get_bits(gb, 8);
            v->matrix_coef   = get_bits(gb, 8);
        }
    }

        }
    }
    return 0;
}

{

        v->s.loop_filter = 0;

        }
    }

    } else {
    }
        av_log(avctx, AV_LOG_ERROR, "Failed to set dimensions %d %d\n", w, h);
        return ret;
    }

        av_log(avctx, AV_LOG_ERROR, "Luma scaling is not supported, expect wrong picture\n");
        v->range_mapy = get_bits(gb, 3);
    }
        av_log(avctx, AV_LOG_ERROR, "Chroma scaling is not supported, expect wrong picture\n");
        v->range_mapuv = get_bits(gb, 3);
    }

           "BrokenLink=%i, ClosedEntry=%i, PanscanFlag=%i\n"
           "RefDist=%i, Postproc=%i, FastUVMC=%i, ExtMV=%i\n"
           "DQuant=%i, VSTransform=%i, Overlap=%i, Qmode=%i\n",
           v->fastuvmc, v->extended_mv, v->dquant, v->vstransform, v->overlap, v->quantizer_mode);

}

/* fill lookup tables for intensity compensation */
#define INIT_LUT(lumscale, lumshift, luty, lutuv, chain) do {                 \
        int scale, shift, i;                                                  \
        if (!lumscale) {                                                      \
            scale = -64;                                                      \
            shift = (255 - lumshift * 2) * 64;                                \
            if (lumshift > 31)                                                \
                shift += 128 << 6;                                            \
        } else {                                                              \
            scale = lumscale + 32;                                            \
            if (lumshift > 31)                                                \
                shift = (lumshift - 64) * 64;                                 \
            else                                                              \
                shift = lumshift << 6;                                        \
        }                                                                     \
        for (i = 0; i < 256; i++) {                                           \
            int iy = chain ? luty[i]  : i;                                    \
            int iu = chain ? lutuv[i] : i;                                    \
            luty[i]  = av_clip_uint8((scale * iy + shift + 32) >> 6);         \
            lutuv[i] = av_clip_uint8((scale * (iu - 128) + 128*64 + 32) >> 6);\
        }                                                                     \
    } while(0)

{
#define ROTATE(DEF, L, N, C, A) do {                          \
        if (v->s.pict_type == AV_PICTURE_TYPE_BI || v->s.pict_type == AV_PICTURE_TYPE_B) { \
            C = A;                                            \
        } else {                                              \
            DEF;                                              \
            memcpy(&tmp, L   , sizeof(tmp));                  \
            memcpy(L   , N   , sizeof(tmp));                  \
            memcpy(N   , &tmp, sizeof(tmp));                  \
            C = N;                                            \
        }                                                     \
    } while(0)




        av_log(v->s.avctx, AV_LOG_ERROR, "bfraction invalid\n");
        return AVERROR_INVALIDDATA;
    }
}

{

        v->interpfrm = get_bits1(gb);
        return -1;
    else
    } else {
        } else
    }

            return AVERROR_INVALIDDATA;
            v->s.pict_type = AV_PICTURE_TYPE_BI;
        }
    }

        return 0;

    /* calculate RND */

    /* Quantizer stuff */
        return -1;
    else
        v->pq = ff_vc1_pquant_table[1][pqindex];
    else
    case QUANT_NON_UNIFORM:
        v->pquantizer = 0;
        break;
    case QUANT_FRAME_EXPLICIT:
        v->pquantizer = get_bits1(gb);
        break;
    default:
        v->pquantizer = 1;
        break;
    }
        v->mvrange = get_unary(gb, 0, 3);

        v->x8_type = get_bits1(gb);
    } else
            (v->s.pict_type == AV_PICTURE_TYPE_P) ? 'P' : ((v->s.pict_type == AV_PICTURE_TYPE_I) ? 'I' : 'B'),
            pqindex, v->pq, v->halfpq, v->rangeredfrm);



            /* fill lookup tables for intensity compensation */
        }
                                   v->mv_mode2 != MV_PMODE_1MV_HPEL_BILIN);
        } else {
                                   v->mv_mode != MV_PMODE_1MV_HPEL_BILIN);
        }

            v->mv_mode   == MV_PMODE_MIXED_MV) {
                return -1;
                   "Imode: %i, Invert: %i\n", status>>1, status&1);
        } else {
        }
            return -1;
               "Imode: %i, Invert: %i\n", status>>1, status&1);

        /* Hopefully this is correct for P-frames */

        }

            } else
        } else {
        }
        break;


            return -1;
               "Imode: %i, Invert: %i\n", status>>1, status&1);
            return -1;
               "Imode: %i, Invert: %i\n", status>>1, status&1);


            av_log(v->s.avctx, AV_LOG_DEBUG, "VOP DQuant info\n");
            vop_dquant_decoding(v);
        }

                v->ttfrm = ff_vc1_ttfrm_to_tt[get_bits(gb, 2)];
            } else
        } else {
            v->ttmbf = 1;
            v->ttfrm = TT_8X8;
        }
        break;
    }

        /* AC Syntax */
        }
        /* DC Syntax */
    }

        v->s.pict_type = AV_PICTURE_TYPE_B;
        v->bi_type     = 1;
    }
    return 0;
}

{

            return -1;
        else
    }

        }
    } else {
        fcm = PROGRESSIVE;
    }
        return AVERROR_INVALIDDATA;

                || v->s.mb_height == FFALIGN(v->s.height + 15 >> 4, 2));
        else
    } else {
        case 3:
            v->s.pict_type = AV_PICTURE_TYPE_BI;
            break;
        }
        skip_bits(gb, 8);
        } else {
        }
    } else {
        v->tff = 1;
    }
        avpriv_report_missing_feature(v->s.avctx, "Pan-scan");
        //...
    }
        return 0;
    }
        return 0; //parsing only, vlc tables havnt been allocated
            v->refdist = 0;
                v->refdist += get_unary(gb, 0, 14);
                return AVERROR_INVALIDDATA;
        }
                return AVERROR_INVALIDDATA;
                v->brfd = 0;
        }
    }
            v->interpfrm = get_bits1(gb);
                return AVERROR_INVALIDDATA;
                v->s.pict_type = AV_PICTURE_TYPE_BI; /* XXX: should not happen here */
            }
        }
    }

        return -1;
    else
    else
    case QUANT_NON_UNIFORM:
        v->pquantizer = 0;
        break;
    case QUANT_FRAME_EXPLICIT:
        v->pquantizer = get_bits1(gb);
        break;
    }
        v->postproc = get_bits(gb, 2);

        return 0;


    case AV_PICTURE_TYPE_BI:
                return -1;
                   "Imode: %i, Invert: %i\n", status>>1, status&1);
        } else
            return -1;
               "Imode: %i, Invert: %i\n", status>>1, status&1);
                    return -1;
                       "Imode: %i, Invert: %i\n", status>>1, status&1);
            }
        }
        break;
                v->reffield          = get_bits1(gb);
                v->ref_field_type[0] = v->reffield ^ !v->cur_field_type;
            }
        }
        else
                v->dmvrange = get_unary(gb, 0, 3);
            else
                    v->lumscale = get_bits(gb, 6);
                    v->lumshift = get_bits(gb, 6);
                    INIT_LUT(v->lumscale, v->lumshift, v->last_luty[0], v->last_lutuv[0], 1);
                    INIT_LUT(v->lumscale, v->lumshift, v->last_luty[1], v->last_lutuv[1], 1);
                    v->last_use_ic = 1;
                }
                    return -1;
                       "Imode: %i, Invert: %i\n", status>>1, status&1);
                else
                    v->mbmode_vlc = &ff_vc1_intfr_non4mv_mbmode_vlc[v->mbmodetab];
                // interlaced p-picture cbpcy range is [1, 63]
                }
            }
        }

            int mvmode;
                int mvmode2;
                mvmode2 = get_unary(gb, 1, 3);
                v->mv_mode2 = ff_vc1_mv_pmode_table2[lowquant][mvmode2];
                if (v->field_mode) {
                    v->intcompfield = decode210(gb) ^ 3;
                } else
                    v->intcompfield = 3;

                v->lumscale2 = v->lumscale = 32;
                v->lumshift2 = v->lumshift =  0;
                if (v->intcompfield & 1) {
                    v->lumscale = get_bits(gb, 6);
                    v->lumshift = get_bits(gb, 6);
                }
                if ((v->intcompfield & 2) && v->field_mode) {
                    v->lumscale2 = get_bits(gb, 6);
                    v->lumshift2 = get_bits(gb, 6);
                } else if(!v->field_mode) {
                    v->lumscale2 = v->lumscale;
                    v->lumshift2 = v->lumshift;
                }
                if (v->field_mode && v->second_field) {
                    if (v->cur_field_type) {
                        INIT_LUT(v->lumscale , v->lumshift , v->curr_luty[v->cur_field_type^1], v->curr_lutuv[v->cur_field_type^1], 0);
                        INIT_LUT(v->lumscale2, v->lumshift2, v->last_luty[v->cur_field_type  ], v->last_lutuv[v->cur_field_type  ], 1);
                    } else {
                        INIT_LUT(v->lumscale2, v->lumshift2, v->curr_luty[v->cur_field_type^1], v->curr_lutuv[v->cur_field_type^1], 0);
                        INIT_LUT(v->lumscale , v->lumshift , v->last_luty[v->cur_field_type  ], v->last_lutuv[v->cur_field_type  ], 1);
                    }
                    v->next_use_ic = *v->curr_use_ic = 1;
                } else {
                    INIT_LUT(v->lumscale , v->lumshift , v->last_luty[0], v->last_lutuv[0], 1);
                    INIT_LUT(v->lumscale2, v->lumshift2, v->last_luty[1], v->last_lutuv[1], 1);
                }
                v->last_use_ic = 1;
            }
                v->s.quarter_sample = (v->mv_mode2 != MV_PMODE_1MV_HPEL &&
                                       v->mv_mode2 != MV_PMODE_1MV_HPEL_BILIN);
                v->s.mspel          = (v->mv_mode2 != MV_PMODE_1MV_HPEL_BILIN);
            } else {
                                       v->mv_mode != MV_PMODE_1MV_HPEL_BILIN);
            }
        }
                 v->mv_mode2 == MV_PMODE_MIXED_MV)
                    return -1;
                       "Imode: %i, Invert: %i\n", status>>1, status&1);
            } else {
            }
                return -1;
                   "Imode: %i, Invert: %i\n", status>>1, status&1);

            /* Hopefully this is correct for P-frames */
        } else {    // field interlaced
                v->imv_vlc = &ff_vc1_1ref_mvdata_vlc[v->imvtab];
            else
            } else {
            }
        }
        }

                v->ttfrm = ff_vc1_ttfrm_to_tt[get_bits(gb, 2)];
            } else
        } else {
        }
        break;
                return AVERROR_INVALIDDATA;
                return -1;
            }
        }
        else


                v->dmvrange = get_unary(gb, 0, 3);
                return -1;
                   "Imode: %i, Invert: %i\n", status>>1, status&1);
            else
            }
                v->dmvrange = get_unary(gb, 0, 3);
                av_log(v->s.avctx, AV_LOG_WARNING, "Intensity compensation set for B picture\n");
                return -1;
                   "Imode: %i, Invert: %i\n", status>>1, status&1);
                return -1;
                   "Imode: %i, Invert: %i\n", status>>1, status&1);
            // interlaced p/b-picture cbpcy range is [1, 63]
        } else {
                return -1;
                   "Imode: %i, Invert: %i\n", status>>1, status&1);
                return -1;
                   "Imode: %i, Invert: %i\n", status>>1, status&1);
        }

        }

                v->ttfrm = ff_vc1_ttfrm_to_tt[get_bits(gb, 2)];
            } else
        } else {
            v->ttmbf = 1;
            v->ttfrm = TT_8X8;
        }
        break;
    }


    /* AC Syntax */
    }
    }

    /* DC Syntax */
    }

        v->s.pict_type = AV_PICTURE_TYPE_B;

    return 0;
}

static const uint32_t vc1_ac_tables[AC_MODES][186][2] = {
{
{ 0x0001,  2}, { 0x0005,  3}, { 0x000D,  4}, { 0x0012,  5}, { 0x000E,  6}, { 0x0015,  7},
{ 0x0013,  8}, { 0x003F,  8}, { 0x004B,  9}, { 0x011F,  9}, { 0x00B8, 10}, { 0x03E3, 10},
{ 0x0172, 11}, { 0x024D, 12}, { 0x03DA, 12}, { 0x02DD, 13}, { 0x1F55, 13}, { 0x05B9, 14},
{ 0x3EAE, 14}, { 0x0000,  4}, { 0x0010,  5}, { 0x0008,  7}, { 0x0020,  8}, { 0x0029,  9},
{ 0x01F4,  9}, { 0x0233, 10}, { 0x01E0, 11}, { 0x012A, 12}, { 0x03DD, 12}, { 0x050A, 13},
{ 0x1F29, 13}, { 0x0A42, 14}, { 0x1272, 15}, { 0x1737, 15}, { 0x0003,  5}, { 0x0011,  7},
{ 0x00C4,  8}, { 0x004B, 10}, { 0x00B4, 11}, { 0x07D4, 11}, { 0x0345, 12}, { 0x02D7, 13},
{ 0x07BF, 13}, { 0x0938, 14}, { 0x0BBB, 14}, { 0x095E, 15}, { 0x0013,  5}, { 0x0078,  7},
{ 0x0069,  9}, { 0x0232, 10}, { 0x0461, 11}, { 0x03EC, 12}, { 0x0520, 13}, { 0x1F2A, 13},
{ 0x3E50, 14}, { 0x3E51, 14}, { 0x1486, 15}, { 0x000C,  6}, { 0x0024,  9}, { 0x0094, 11},
{ 0x08C0, 12}, { 0x0F09, 14}, { 0x1EF0, 15}, { 0x003D,  6}, { 0x0053,  9}, { 0x01A0, 11},
{ 0x02D6, 13}, { 0x0F08, 14}, { 0x0013,  7}, { 0x007C,  9}, { 0x07C1, 11}, { 0x04AC, 14},
{ 0x001B,  7}, { 0x00A0, 10}, { 0x0344, 12}, { 0x0F79, 14}, { 0x0079,  7}, { 0x03E1, 10},
{ 0x02D4, 13}, { 0x2306, 14}, { 0x0021,  8}, { 0x023C, 10}, { 0x0FAE, 12}, { 0x23DE, 14},
{ 0x0035,  8}, { 0x0175, 11}, { 0x07B3, 13}, { 0x00C5,  8}, { 0x0174, 11}, { 0x0785, 13},
{ 0x0048,  9}, { 0x01A3, 11}, { 0x049E, 13}, { 0x002C,  9}, { 0x00FA, 10}, { 0x07D6, 11},
{ 0x0092, 10}, { 0x05CC, 13}, { 0x1EF1, 15}, { 0x00A3, 10}, { 0x03ED, 12}, { 0x093E, 14},
{ 0x01E2, 11}, { 0x1273, 15}, { 0x07C4, 11}, { 0x1487, 15}, { 0x0291, 12}, { 0x0293, 12},
{ 0x0F8A, 12}, { 0x0509, 13}, { 0x0508, 13}, { 0x078D, 13}, { 0x07BE, 13}, { 0x078C, 13},
{ 0x04AE, 14}, { 0x0BBA, 14}, { 0x2307, 14}, { 0x0B9A, 14}, { 0x1736, 15}, { 0x000E,  4},
{ 0x0045,  7}, { 0x01F3,  9}, { 0x047A, 11}, { 0x05DC, 13}, { 0x23DF, 14}, { 0x0019,  5},
{ 0x0028,  9}, { 0x0176, 11}, { 0x049D, 13}, { 0x23DD, 14}, { 0x0030,  6}, { 0x00A2, 10},
{ 0x02EF, 12}, { 0x05B8, 14}, { 0x003F,  6}, { 0x00A5, 10}, { 0x03DB, 12}, { 0x093F, 14},
{ 0x0044,  7}, { 0x07CB, 11}, { 0x095F, 15}, { 0x0063,  7}, { 0x03C3, 12}, { 0x0015,  8},
{ 0x08F6, 12}, { 0x0017,  8}, { 0x0498, 13}, { 0x002C,  8}, { 0x07B2, 13}, { 0x002F,  8},
{ 0x1F54, 13}, { 0x008D,  8}, { 0x07BD, 13}, { 0x008E,  8}, { 0x1182, 13}, { 0x00FB,  8},
{ 0x050B, 13}, { 0x002D,  8}, { 0x07C0, 11}, { 0x0079,  9}, { 0x1F5F, 13}, { 0x007A,  9},
{ 0x1F56, 13}, { 0x0231, 10}, { 0x03E4, 10}, { 0x01A1, 11}, { 0x0143, 11}, { 0x01F7, 11},
{ 0x016F, 12}, { 0x0292, 12}, { 0x02E7, 12}, { 0x016C, 12}, { 0x016D, 12}, { 0x03DC, 12},
{ 0x0F8B, 12}, { 0x0499, 13}, { 0x03D8, 12}, { 0x078E, 13}, { 0x02D5, 13}, { 0x1F5E, 13},
{ 0x1F2B, 13}, { 0x078F, 13}, { 0x04AD, 14}, { 0x3EAF, 14}, { 0x23DC, 14}, { 0x004A,  9}
},
{
{ 0x0000,  3}, { 0x0003,  4}, { 0x000B,  5}, { 0x0014,  6}, { 0x003F,  6}, { 0x005D,  7},
{ 0x00A2,  8}, { 0x00AC,  9}, { 0x016E,  9}, { 0x020A, 10}, { 0x02E2, 10}, { 0x0432, 11},
{ 0x05C9, 11}, { 0x0827, 12}, { 0x0B54, 12}, { 0x04E6, 13}, { 0x105F, 13}, { 0x172A, 13},
{ 0x20B2, 14}, { 0x2D4E, 14}, { 0x39F0, 14}, { 0x4175, 15}, { 0x5A9E, 15}, { 0x0004,  4},
{ 0x001E,  5}, { 0x0042,  7}, { 0x00B6,  8}, { 0x0173,  9}, { 0x0395, 10}, { 0x072E, 11},
{ 0x0B94, 12}, { 0x16A4, 13}, { 0x20B3, 14}, { 0x2E45, 14}, { 0x0005,  5}, { 0x0040,  7},
{ 0x0049,  9}, { 0x028F, 10}, { 0x05CB, 11}, { 0x048A, 13}, { 0x09DD, 14}, { 0x73E2, 15},
{ 0x0018,  5}, { 0x0025,  8}, { 0x008A, 10}, { 0x051B, 11}, { 0x0E5F, 12}, { 0x09C9, 14},
{ 0x139C, 15}, { 0x0029,  6}, { 0x004F,  9}, { 0x0412, 11}, { 0x048D, 13}, { 0x2E41, 14},
{ 0x0038,  6}, { 0x010E,  9}, { 0x05A8, 11}, { 0x105C, 13}, { 0x39F2, 14}, { 0x0058,  7},
{ 0x021F, 10}, { 0x0E7E, 12}, { 0x39FF, 14}, { 0x0023,  8}, { 0x02E3, 10}, { 0x04E5, 13},
{ 0x2E40, 14}, { 0x00A1,  8}, { 0x05BE, 11}, { 0x09C8, 14}, { 0x0083,  8}, { 0x013A, 11},
{ 0x1721, 13}, { 0x0044,  9}, { 0x0276, 12}, { 0x39F6, 14}, { 0x008B, 10}, { 0x04EF, 13},
{ 0x5A9B, 15}, { 0x0208, 10}, { 0x1CFE, 13}, { 0x0399, 10}, { 0x1CB4, 13}, { 0x039E, 10},
{ 0x39F3, 14}, { 0x05AB, 11}, { 0x73E3, 15}, { 0x0737, 11}, { 0x5A9F, 15}, { 0x082D, 12},
{ 0x0E69, 12}, { 0x0E68, 12}, { 0x0433, 11}, { 0x0B7B, 12}, { 0x2DF8, 14}, { 0x2E56, 14},
{ 0x2E57, 14}, { 0x39F7, 14}, { 0x51A5, 15}, { 0x0003,  3}, { 0x002A,  6}, { 0x00E4,  8},
{ 0x028E, 10}, { 0x0735, 11}, { 0x1058, 13}, { 0x1CFA, 13}, { 0x2DF9, 14}, { 0x4174, 15},
{ 0x0009,  4}, { 0x0054,  8}, { 0x0398, 10}, { 0x048B, 13}, { 0x139D, 15}, { 0x000D,  4},
{ 0x00AD,  9}, { 0x0826, 12}, { 0x2D4C, 14}, { 0x0011,  5}, { 0x016B,  9}, { 0x0B7F, 12},
{ 0x51A4, 15}, { 0x0019,  5}, { 0x021B, 10}, { 0x16FD, 13}, { 0x001D,  5}, { 0x0394, 10},
{ 0x28D3, 14}, { 0x002B,  6}, { 0x05BC, 11}, { 0x5A9A, 15}, { 0x002F,  6}, { 0x0247, 12},
{ 0x0010,  7}, { 0x0A35, 12}, { 0x003E,  6}, { 0x0B7A, 12}, { 0x0059,  7}, { 0x105E, 13},
{ 0x0026,  8}, { 0x09CF, 14}, { 0x0055,  8}, { 0x1CB5, 13}, { 0x0057,  8}, { 0x0E5B, 12},
{ 0x00A0,  8}, { 0x1468, 13}, { 0x0170,  9}, { 0x0090, 10}, { 0x01CE,  9}, { 0x021A, 10},
{ 0x0218, 10}, { 0x0168,  9}, { 0x021E, 10}, { 0x0244, 12}, { 0x0736, 11}, { 0x0138, 11},
{ 0x0519, 11}, { 0x0E5E, 12}, { 0x072C, 11}, { 0x0B55, 12}, { 0x09DC, 14}, { 0x20BB, 14},
{ 0x048C, 13}, { 0x1723, 13}, { 0x2E44, 14}, { 0x16A5, 13}, { 0x0518, 11}, { 0x39FE, 14},
{ 0x0169,  9}
},
{
{ 0x0001,  2}, { 0x0006,  3}, { 0x000F,  4}, { 0x0016,  5}, { 0x0020,  6}, { 0x0018,  7},
{ 0x0008,  8}, { 0x009A,  8}, { 0x0056,  9}, { 0x013E,  9}, { 0x00F0, 10}, { 0x03A5, 10},
{ 0x0077, 11}, { 0x01EF, 11}, { 0x009A, 12}, { 0x005D, 13}, { 0x0001,  4}, { 0x0011,  5},
{ 0x0002,  7}, { 0x000B,  8}, { 0x0012,  9}, { 0x01D6,  9}, { 0x027E, 10}, { 0x0191, 11},
{ 0x00EA, 12}, { 0x03DC, 12}, { 0x013B, 13}, { 0x0004,  5}, { 0x0014,  7}, { 0x009E,  8},
{ 0x0009, 10}, { 0x01AC, 11}, { 0x01E2, 11}, { 0x03CA, 12}, { 0x005F, 13}, { 0x0017,  5},
{ 0x004E,  7}, { 0x005E,  9}, { 0x00F3, 10}, { 0x01AD, 11}, { 0x00EC, 12}, { 0x05F0, 13},
{ 0x000E,  6}, { 0x00E1,  8}, { 0x03A4, 10}, { 0x009C, 12}, { 0x013D, 13}, { 0x003B,  6},
{ 0x001C,  9}, { 0x0014, 11}, { 0x09BE, 12}, { 0x0006,  7}, { 0x007A,  9}, { 0x0190, 11},
{ 0x0137, 13}, { 0x001B,  7}, { 0x0008, 10}, { 0x075C, 11}, { 0x0071,  7}, { 0x00D7, 10},
{ 0x09BF, 12}, { 0x0007,  8}, { 0x00AF, 10}, { 0x04CC, 11}, { 0x0034,  8}, { 0x0265, 10},
{ 0x009F, 12}, { 0x00E0,  8}, { 0x0016, 11}, { 0x0327, 12}, { 0x0015,  9}, { 0x017D, 11},
{ 0x0EBB, 12}, { 0x0014,  9}, { 0x00F6, 10}, { 0x01E4, 11}, { 0x00CB, 10}, { 0x099D, 12},
{ 0x00CA, 10}, { 0x02FC, 12}, { 0x017F, 11}, { 0x04CD, 11}, { 0x02FD, 12}, { 0x04FE, 11},
{ 0x013A, 13}, { 0x000A,  4}, { 0x0042,  7}, { 0x01D3,  9}, { 0x04DD, 11}, { 0x0012,  5},
{ 0x00E8,  8}, { 0x004C, 11}, { 0x0136, 13}, { 0x0039,  6}, { 0x0264, 10}, { 0x0EBA, 12},
{ 0x0000,  7}, { 0x00AE, 10}, { 0x099C, 12}, { 0x001F,  7}, { 0x04DE, 11}, { 0x0043,  7},
{ 0x04DC, 11}, { 0x0003,  8}, { 0x03CB, 12}, { 0x0006,  8}, { 0x099E, 12}, { 0x002A,  8},
{ 0x05F1, 13}, { 0x000F,  8}, { 0x09FE, 12}, { 0x0033,  8}, { 0x09FF, 12}, { 0x0098,  8},
{ 0x099F, 12}, { 0x00EA,  8}, { 0x013C, 13}, { 0x002E,  8}, { 0x0192, 11}, { 0x0136,  9},
{ 0x006A,  9}, { 0x0015, 11}, { 0x03AF, 10}, { 0x01E3, 11}, { 0x0074, 11}, { 0x00EB, 12},
{ 0x02F9, 12}, { 0x005C, 13}, { 0x00ED, 12}, { 0x03DD, 12}, { 0x0326, 12}, { 0x005E, 13},
{ 0x0016,  7}
},
{
{ 0x0004,  3}, { 0x0014,  5}, { 0x0017,  7}, { 0x007F,  8}, { 0x0154,  9}, { 0x01F2, 10},
{ 0x00BF, 11}, { 0x0065, 12}, { 0x0AAA, 12}, { 0x0630, 13}, { 0x1597, 13}, { 0x03B7, 14},
{ 0x2B22, 14}, { 0x0BE6, 15}, { 0x000B,  4}, { 0x0037,  7}, { 0x0062,  9}, { 0x0007, 11},
{ 0x0166, 12}, { 0x00CE, 13}, { 0x1590, 13}, { 0x05F6, 14}, { 0x0BE7, 15}, { 0x0007,  5},
{ 0x006D,  8}, { 0x0003, 11}, { 0x031F, 12}, { 0x05F2, 14}, { 0x0002,  6}, { 0x0061,  9},
{ 0x0055, 12}, { 0x01DF, 14}, { 0x001A,  6}, { 0x001E, 10}, { 0x0AC9, 12}, { 0x2B23, 14},
{ 0x001E,  6}, { 0x001F, 10}, { 0x0AC3, 12}, { 0x2B2B, 14}, { 0x0006,  7}, { 0x0004, 11},
{ 0x02F8, 13}, { 0x0019,  7}, { 0x0006, 11}, { 0x063D, 13}, { 0x0057,  7}, { 0x0182, 11},
{ 0x2AA2, 14}, { 0x0004,  8}, { 0x0180, 11}, { 0x059C, 14}, { 0x007D,  8}, { 0x0164, 12},
{ 0x076D, 15}, { 0x0002,  9}, { 0x018D, 11}, { 0x1581, 13}, { 0x00AD,  8}, { 0x0060, 12},
{ 0x0C67, 14}, { 0x001C,  9}, { 0x00EE, 13}, { 0x0003,  9}, { 0x02CF, 13}, { 0x00D9,  9},
{ 0x1580, 13}, { 0x0002, 11}, { 0x0183, 11}, { 0x0057, 12}, { 0x0061, 12}, { 0x0031, 11},
{ 0x0066, 12}, { 0x0631, 13}, { 0x0632, 13}, { 0x00AC, 13}, { 0x031D, 12}, { 0x0076, 12},
{ 0x003A, 11}, { 0x0165, 12}, { 0x0C66, 14}, { 0x0003,  2}, { 0x0054,  7}, { 0x02AB, 10},
{ 0x0016, 13}, { 0x05F7, 14}, { 0x0005,  4}, { 0x00F8,  9}, { 0x0AA9, 12}, { 0x005F, 15},
{ 0x0004,  4}, { 0x001C, 10}, { 0x1550, 13}, { 0x0004,  5}, { 0x0077, 11}, { 0x076C, 15},
{ 0x000E,  5}, { 0x000A, 12}, { 0x000C,  5}, { 0x0562, 11}, { 0x0004,  6}, { 0x031C, 12},
{ 0x0006,  6}, { 0x00C8, 13}, { 0x000D,  6}, { 0x01DA, 13}, { 0x0007,  6}, { 0x00C9, 13},
{ 0x0001,  7}, { 0x002E, 14}, { 0x0014,  7}, { 0x1596, 13}, { 0x000A,  7}, { 0x0AC2, 12},
{ 0x0016,  7}, { 0x015B, 14}, { 0x0015,  7}, { 0x015A, 14}, { 0x000F,  8}, { 0x005E, 15},
{ 0x007E,  8}, { 0x00AB,  8}, { 0x002D,  9}, { 0x00D8,  9}, { 0x000B,  9}, { 0x0014, 10},
{ 0x02B3, 10}, { 0x01F3, 10}, { 0x003A, 10}, { 0x0000, 10}, { 0x0058, 10}, { 0x002E,  9},
{ 0x005E, 10}, { 0x0563, 11}, { 0x00EC, 12}, { 0x0054, 12}, { 0x0AC1, 12}, { 0x1556, 13},
{ 0x02FA, 13}, { 0x0181, 11}, { 0x1557, 13}, { 0x059D, 14}, { 0x2AA3, 14}, { 0x2B2A, 14},
{ 0x01DE, 14}, { 0x063C, 13}, { 0x00CF, 13}, { 0x1594, 13}, { 0x000D,  9}
},
{
{ 0x0002,  2}, { 0x0006,  3}, { 0x000F,  4}, { 0x000D,  5}, { 0x000C,  5}, { 0x0015,  6},
{ 0x0013,  6}, { 0x0012,  6}, { 0x0017,  7}, { 0x001F,  8}, { 0x001E,  8}, { 0x001D,  8},
{ 0x0025,  9}, { 0x0024,  9}, { 0x0023,  9}, { 0x0021,  9}, { 0x0021, 10}, { 0x0020, 10},
{ 0x000F, 10}, { 0x000E, 10}, { 0x0007, 11}, { 0x0006, 11}, { 0x0020, 11}, { 0x0021, 11},
{ 0x0050, 12}, { 0x0051, 12}, { 0x0052, 12}, { 0x000E,  4}, { 0x0014,  6}, { 0x0016,  7},
{ 0x001C,  8}, { 0x0020,  9}, { 0x001F,  9}, { 0x000D, 10}, { 0x0022, 11}, { 0x0053, 12},
{ 0x0055, 12}, { 0x000B,  5}, { 0x0015,  7}, { 0x001E,  9}, { 0x000C, 10}, { 0x0056, 12},
{ 0x0011,  6}, { 0x001B,  8}, { 0x001D,  9}, { 0x000B, 10}, { 0x0010,  6}, { 0x0022,  9},
{ 0x000A, 10}, { 0x000D,  6}, { 0x001C,  9}, { 0x0008, 10}, { 0x0012,  7}, { 0x001B,  9},
{ 0x0054, 12}, { 0x0014,  7}, { 0x001A,  9}, { 0x0057, 12}, { 0x0019,  8}, { 0x0009, 10},
{ 0x0018,  8}, { 0x0023, 11}, { 0x0017,  8}, { 0x0019,  9}, { 0x0018,  9}, { 0x0007, 10},
{ 0x0058, 12}, { 0x0007,  4}, { 0x000C,  6}, { 0x0016,  8}, { 0x0017,  9}, { 0x0006, 10},
{ 0x0005, 11}, { 0x0004, 11}, { 0x0059, 12}, { 0x000F,  6}, { 0x0016,  9}, { 0x0005, 10},
{ 0x000E,  6}, { 0x0004, 10}, { 0x0011,  7}, { 0x0024, 11}, { 0x0010,  7}, { 0x0025, 11},
{ 0x0013,  7}, { 0x005A, 12}, { 0x0015,  8}, { 0x005B, 12}, { 0x0014,  8}, { 0x0013,  8},
{ 0x001A,  8}, { 0x0015,  9}, { 0x0014,  9}, { 0x0013,  9}, { 0x0012,  9}, { 0x0011,  9},
{ 0x0026, 11}, { 0x0027, 11}, { 0x005C, 12}, { 0x005D, 12}, { 0x005E, 12}, { 0x005F, 12},
{ 0x0003,  7}
},
{
{ 0x0002,  2}, { 0x000F,  4}, { 0x0015,  6}, { 0x0017,  7}, { 0x001F,  8}, { 0x0025,  9},
{ 0x0024,  9}, { 0x0021, 10}, { 0x0020, 10}, { 0x0007, 11}, { 0x0006, 11}, { 0x0020, 11},
{ 0x0006,  3}, { 0x0014,  6}, { 0x001E,  8}, { 0x000F, 10}, { 0x0021, 11}, { 0x0050, 12},
{ 0x000E,  4}, { 0x001D,  8}, { 0x000E, 10}, { 0x0051, 12}, { 0x000D,  5}, { 0x0023,  9},
{ 0x000D, 10}, { 0x000C,  5}, { 0x0022,  9}, { 0x0052, 12}, { 0x000B,  5}, { 0x000C, 10},
{ 0x0053, 12}, { 0x0013,  6}, { 0x000B, 10}, { 0x0054, 12}, { 0x0012,  6}, { 0x000A, 10},
{ 0x0011,  6}, { 0x0009, 10}, { 0x0010,  6}, { 0x0008, 10}, { 0x0016,  7}, { 0x0055, 12},
{ 0x0015,  7}, { 0x0014,  7}, { 0x001C,  8}, { 0x001B,  8}, { 0x0021,  9}, { 0x0020,  9},
{ 0x001F,  9}, { 0x001E,  9}, { 0x001D,  9}, { 0x001C,  9}, { 0x001B,  9}, { 0x001A,  9},
{ 0x0022, 11}, { 0x0023, 11}, { 0x0056, 12}, { 0x0057, 12}, { 0x0007,  4}, { 0x0019,  9},
{ 0x0005, 11}, { 0x000F,  6}, { 0x0004, 11}, { 0x000E,  6}, { 0x000D,  6}, { 0x000C,  6},
{ 0x0013,  7}, { 0x0012,  7}, { 0x0011,  7}, { 0x0010,  7}, { 0x001A,  8}, { 0x0019,  8},
{ 0x0018,  8}, { 0x0017,  8}, { 0x0016,  8}, { 0x0015,  8}, { 0x0014,  8}, { 0x0013,  8},
{ 0x0018,  9}, { 0x0017,  9}, { 0x0016,  9}, { 0x0015,  9}, { 0x0014,  9}, { 0x0013,  9},
{ 0x0012,  9}, { 0x0011,  9}, { 0x0007, 10}, { 0x0006, 10}, { 0x0005, 10}, { 0x0004, 10},
{ 0x0024, 11}, { 0x0025, 11}, { 0x0026, 11}, { 0x0027, 11}, { 0x0058, 12}, { 0x0059, 12},
{ 0x005A, 12}, { 0x005B, 12}, { 0x005C, 12}, { 0x005D, 12}, { 0x005E, 12}, { 0x005F, 12},
{ 0x0003,  7}
},
{
{ 0x0000,  2}, { 0x0003,  3}, { 0x000D,  4}, { 0x0005,  4}, { 0x001C,  5}, { 0x0016,  5},
{ 0x003F,  6}, { 0x003A,  6}, { 0x002E,  6}, { 0x0022,  6}, { 0x007B,  7}, { 0x0067,  7},
{ 0x005F,  7}, { 0x0047,  7}, { 0x0026,  7}, { 0x00EF,  8}, { 0x00CD,  8}, { 0x00C1,  8},
{ 0x00A9,  8}, { 0x004F,  8}, { 0x01F2,  9}, { 0x01DD,  9}, { 0x0199,  9}, { 0x0185,  9},
{ 0x015D,  9}, { 0x011B,  9}, { 0x03EF, 10}, { 0x03E1, 10}, { 0x03C8, 10}, { 0x0331, 10},
{ 0x0303, 10}, { 0x02F1, 10}, { 0x02A0, 10}, { 0x0233, 10}, { 0x0126, 10}, { 0x07C0, 11},
{ 0x076F, 11}, { 0x076C, 11}, { 0x0661, 11}, { 0x0604, 11}, { 0x0572, 11}, { 0x0551, 11},
{ 0x046A, 11}, { 0x0274, 11}, { 0x0F27, 12}, { 0x0F24, 12}, { 0x0EDB, 12}, { 0x0C8E, 12},
{ 0x0C0B, 12}, { 0x0C0A, 12}, { 0x0AE3, 12}, { 0x08D6, 12}, { 0x0490, 12}, { 0x0495, 12},
{ 0x1F19, 13}, { 0x1DB5, 13}, { 0x0009,  4}, { 0x0010,  5}, { 0x0029,  6}, { 0x0062,  7},
{ 0x00F3,  8}, { 0x00AD,  8}, { 0x01E5,  9}, { 0x0179,  9}, { 0x009C,  9}, { 0x03B1, 10},
{ 0x02AE, 10}, { 0x0127, 10}, { 0x076E, 11}, { 0x0570, 11}, { 0x0275, 11}, { 0x0F25, 12},
{ 0x0EC0, 12}, { 0x0AA0, 12}, { 0x08D7, 12}, { 0x1E4C, 13}, { 0x0008,  5}, { 0x0063,  7},
{ 0x00AF,  8}, { 0x017B,  9}, { 0x03B3, 10}, { 0x07DD, 11}, { 0x0640, 11}, { 0x0F8D, 12},
{ 0x0BC1, 12}, { 0x0491, 12}, { 0x0028,  6}, { 0x00C3,  8}, { 0x0151,  9}, { 0x02A1, 10},
{ 0x0573, 11}, { 0x0EC3, 12}, { 0x1F35, 13}, { 0x0065,  7}, { 0x01DA,  9}, { 0x02AF, 10},
{ 0x0277, 11}, { 0x08C9, 12}, { 0x1781, 13}, { 0x0025,  7}, { 0x0118,  9}, { 0x0646, 11},
{ 0x0AA6, 12}, { 0x1780, 13}, { 0x00C9,  8}, { 0x0321, 10}, { 0x0F9B, 12}, { 0x191E, 13},
{ 0x0048,  8}, { 0x07CC, 11}, { 0x0AA1, 12}, { 0x0180,  9}, { 0x0465, 11}, { 0x1905, 13},
{ 0x03E2, 10}, { 0x0EC1, 12}, { 0x3C9B, 14}, { 0x02F4, 10}, { 0x08C8, 12}, { 0x07C1, 11},
{ 0x0928, 13}, { 0x05E1, 11}, { 0x320D, 14}, { 0x0EC2, 12}, { 0x6418, 15}, { 0x1F34, 13},
{ 0x0078,  7}, { 0x0155,  9}, { 0x0552, 11}, { 0x191F, 13}, { 0x00FA,  8}, { 0x07DC, 11},
{ 0x1907, 13}, { 0x00AC,  8}, { 0x0249, 11}, { 0x13B1, 14}, { 0x01F6,  9}, { 0x0AE2, 12},
{ 0x01DC,  9}, { 0x04ED, 12}, { 0x0184,  9}, { 0x1904, 13}, { 0x0156,  9}, { 0x09D9, 13},
{ 0x03E7, 10}, { 0x0929, 13}, { 0x03B2, 10}, { 0x3B68, 14}, { 0x02F5, 10}, { 0x13B0, 14},
{ 0x0322, 10}, { 0x3B69, 14}, { 0x0234, 10}, { 0x7935, 15}, { 0x07C7, 11}, { 0xC833, 16},
{ 0x0660, 11}, { 0x7934, 15}, { 0x024B, 11}, { 0xC832, 16}, { 0x0AA7, 12}, { 0x1F18, 13},
{ 0x007A,  7}
},
{
{ 0x0002,  2}, { 0x0000,  3}, { 0x001E,  5}, { 0x0004,  5}, { 0x0012,  6}, { 0x0070,  7},
{ 0x001A,  7}, { 0x005F,  8}, { 0x0047,  8}, { 0x01D3,  9}, { 0x00B5,  9}, { 0x0057,  9},
{ 0x03B5, 10}, { 0x016D, 10}, { 0x0162, 10}, { 0x07CE, 11}, { 0x0719, 11}, { 0x0691, 11},
{ 0x02C6, 11}, { 0x0156, 11}, { 0x0F92, 12}, { 0x0D2E, 12}, { 0x0D20, 12}, { 0x059E, 12},
{ 0x0468, 12}, { 0x02A6, 12}, { 0x1DA2, 13}, { 0x1C60, 13}, { 0x1A43, 13}, { 0x0B1D, 13},
{ 0x08C0, 13}, { 0x055D, 13}, { 0x0003,  3}, { 0x000A,  5}, { 0x0077,  7}, { 0x00E5,  8},
{ 0x01D9,  9}, { 0x03E5, 10}, { 0x0166, 10}, { 0x0694, 11}, { 0x0152, 11}, { 0x059F, 12},
{ 0x1F3C, 13}, { 0x1A4B, 13}, { 0x055E, 13}, { 0x000C,  4}, { 0x007D,  7}, { 0x0044,  8},
{ 0x03E0, 10}, { 0x0769, 11}, { 0x0E31, 12}, { 0x1F26, 13}, { 0x055C, 13}, { 0x001B,  5},
{ 0x00E2,  8}, { 0x03A5, 10}, { 0x02C9, 11}, { 0x1F23, 13}, { 0x3B47, 14}, { 0x0007,  5},
{ 0x01D8,  9}, { 0x02D8, 11}, { 0x1F27, 13}, { 0x3494, 14}, { 0x0035,  6}, { 0x03E1, 10},
{ 0x059C, 12}, { 0x38C3, 14}, { 0x000C,  6}, { 0x0165, 10}, { 0x1D23, 13}, { 0x1638, 14},
{ 0x0068,  7}, { 0x0693, 11}, { 0x3A45, 14}, { 0x0020,  7}, { 0x0F90, 12}, { 0x7CF6, 15},
{ 0x00E8,  8}, { 0x058F, 12}, { 0x2CEF, 15}, { 0x0045,  8}, { 0x0B3A, 13}, { 0x01F1,  9},
{ 0x3B46, 14}, { 0x01A7,  9}, { 0x1676, 14}, { 0x0056,  9}, { 0x692A, 15}, { 0x038D, 10},
{ 0xE309, 16}, { 0x00AA, 10}, { 0x1C611, 17}, { 0x02DF, 11}, { 0xB3B9, 17}, { 0x02C8, 11},
{ 0x38C20, 18}, { 0x01B0, 11}, { 0x16390, 18}, { 0x0F9F, 12}, { 0x16771, 18}, { 0x0ED0, 12},
{ 0x71843, 19}, { 0x0D2A, 12}, { 0xF9E8C, 20}, { 0x0461, 12}, { 0xF9E8E, 20}, { 0x0B67, 13},
{ 0x055F, 13}, { 0x003F,  6}, { 0x006D,  9}, { 0x0E90, 12}, { 0x054E, 13}, { 0x0013,  6},
{ 0x0119, 10}, { 0x0B66, 13}, { 0x000B,  6}, { 0x0235, 11}, { 0x7CF5, 15}, { 0x0075,  7},
{ 0x0D24, 12}, { 0xF9E9, 16}, { 0x002E,  7}, { 0x1F22, 13}, { 0x0021,  7}, { 0x054F, 13},
{ 0x0014,  7}, { 0x3A44, 14}, { 0x00E4,  8}, { 0x7CF7, 15}, { 0x005E,  8}, { 0x7185, 15},
{ 0x0037,  8}, { 0x2C73, 15}, { 0x01DB,  9}, { 0x59DD, 16}, { 0x01C7,  9}, { 0x692B, 15},
{ 0x01A6,  9}, { 0x58E5, 16}, { 0x00B4,  9}, { 0x1F3D0, 17}, { 0x00B0,  9}, { 0xB1C9, 17},
{ 0x03E6, 10}, { 0x16770, 18}, { 0x016E, 10}, { 0x3E7A2, 18}, { 0x011B, 10}, { 0xF9E8D, 20},
{ 0x00D9, 10}, { 0xF9E8F, 20}, { 0x00A8, 10}, { 0x2C723, 19}, { 0x0749, 11}, { 0xE3084, 20},
{ 0x0696, 11}, { 0x58E45, 20}, { 0x02DE, 11}, { 0xB1C88, 21}, { 0x0231, 11}, { 0x1C610A, 21},
{ 0x01B1, 11}, { 0x71842D, 23}, { 0x0D2B, 12}, { 0x38C217, 22}, { 0x0D2F, 12}, { 0x163913, 22},
{ 0x05B2, 12}, { 0x163912, 22}, { 0x0469, 12}, { 0x71842C, 23}, { 0x1A42, 13}, { 0x08C1, 13},
{ 0x0073,  7}
}
};

static const uint16_t vlc_offs[] = {
        0,   520,   552,   616,  1128,  1160,  1224,  1740,  1772,  1836,  1900,  2436,
     2986,  3050,  3610,  4154,  4218,  4746,  5326,  5390,  5902,  6554,  7658,  8342,
     9304,  9988, 10630, 11234, 12174, 13006, 13560, 14232, 14786, 15432, 16350, 17522,
    20372, 21818, 22330, 22394, 23166, 23678, 23742, 24820, 25332, 25396, 26460, 26980,
    27048, 27592, 27600, 27608, 27616, 27624, 28224, 28258, 28290, 28802, 28834, 28866,
    29378, 29412, 29444, 29960, 29994, 30026, 30538, 30572, 30604, 31120, 31154, 31186,
    31714, 31746, 31778, 32306, 32340, 32372
};

/**
 * Init VC-1 specific tables and VC1Context members
 * @param v The VC1Context to initialize
 * @return Status
 */
{


    /* VLC tables */
                        ff_vc1_bfraction_bits, 1, 1,
                        ff_vc1_bfraction_codes, 1, 1, 1 << VC1_BFRACTION_VLC_BITS);
                        ff_vc1_norm2_bits, 1, 1,
                        ff_vc1_norm2_codes, 1, 1, 1 << VC1_NORM2_VLC_BITS);
                        ff_vc1_norm6_bits, 1, 1,
                        ff_vc1_norm6_codes, 2, 2, 556);
                        ff_vc1_imode_bits, 1, 1,
                        ff_vc1_imode_codes, 1, 1, 1 << VC1_IMODE_VLC_BITS);
                     ff_vc1_ttmb_bits[i], 1, 1,
                     ff_vc1_ttmb_codes[i], 2, 2, INIT_VLC_USE_NEW_STATIC);
                     ff_vc1_ttblk_bits[i], 1, 1,
                     ff_vc1_ttblk_codes[i], 1, 1, INIT_VLC_USE_NEW_STATIC);
                     ff_vc1_subblkpat_bits[i], 1, 1,
                     ff_vc1_subblkpat_codes[i], 1, 1, INIT_VLC_USE_NEW_STATIC);
        }
                     ff_vc1_4mv_block_pattern_bits[i], 1, 1,
                     ff_vc1_4mv_block_pattern_codes[i], 1, 1, INIT_VLC_USE_NEW_STATIC);
                     ff_vc1_cbpcy_p_bits[i], 1, 1,
                     ff_vc1_cbpcy_p_codes[i], 2, 2, INIT_VLC_USE_NEW_STATIC);
                     ff_vc1_mv_diff_bits[i], 1, 1,
                     ff_vc1_mv_diff_codes[i], 2, 2, INIT_VLC_USE_NEW_STATIC);
        }
                     &vc1_ac_tables[i][0][1], 8, 4,
                     &vc1_ac_tables[i][0][0], 8, 4, INIT_VLC_USE_NEW_STATIC);
            /* initialize interlaced MVDATA tables (2-Ref) */
                     ff_vc1_2ref_mvdata_bits[i], 1, 1,
                     ff_vc1_2ref_mvdata_codes[i], 4, 4, INIT_VLC_USE_NEW_STATIC);
        }
            /* initialize 4MV MBMODE VLC tables for interlaced frame P picture */
                     ff_vc1_intfr_4mv_mbmode_bits[i], 1, 1,
                     ff_vc1_intfr_4mv_mbmode_codes[i], 2, 2, INIT_VLC_USE_NEW_STATIC);
            /* initialize NON-4MV MBMODE VLC tables for the same */
                     ff_vc1_intfr_non4mv_mbmode_bits[i], 1, 1,
                     ff_vc1_intfr_non4mv_mbmode_codes[i], 1, 1, INIT_VLC_USE_NEW_STATIC);
            /* initialize interlaced MVDATA tables (1-Ref) */
                     ff_vc1_1ref_mvdata_bits[i], 1, 1,
                     ff_vc1_1ref_mvdata_codes[i], 4, 4, INIT_VLC_USE_NEW_STATIC);
        }
            /* Initialize 2MV Block pattern VLC tables */
                     ff_vc1_2mv_block_pattern_bits[i], 1, 1,
                     ff_vc1_2mv_block_pattern_codes[i], 1, 1, INIT_VLC_USE_NEW_STATIC);
        }
            /* Initialize interlaced CBPCY VLC tables (Table 124 - Table 131) */
                     ff_vc1_icbpcy_p_bits[i], 1, 1,
                     ff_vc1_icbpcy_p_codes[i], 2, 2, INIT_VLC_USE_NEW_STATIC);
            /* Initialize interlaced field picture MBMODE VLC tables */
                     ff_vc1_if_mmv_mbmode_bits[i], 1, 1,
                     ff_vc1_if_mmv_mbmode_codes[i], 1, 1, INIT_VLC_USE_NEW_STATIC);
                     ff_vc1_if_1mv_mbmode_bits[i], 1, 1,
                     ff_vc1_if_1mv_mbmode_codes[i], 1, 1, INIT_VLC_USE_NEW_STATIC);
        }
    }

    /* Other defaults */


}
