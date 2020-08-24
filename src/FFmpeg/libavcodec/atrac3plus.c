/*
 * ATRAC3+ compatible decoder
 *
 * Copyright (c) 2010-2013 Maxim Poliakovski
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
 * Bitstream parser for ATRAC3+ decoder.
 */

#include "libavutil/avassert.h"
#include "avcodec.h"
#include "get_bits.h"
#include "atrac3plus.h"
#include "atrac3plus_data.h"

static VLC_TYPE tables_data[154276][2];
static VLC wl_vlc_tabs[4];
static VLC sf_vlc_tabs[8];
static VLC ct_vlc_tabs[4];
static VLC spec_vlc_tabs[112];
static VLC gain_vlc_tabs[11];
static VLC tone_vlc_tabs[7];

/**
 * Generate canonical VLC table from given descriptor.
 *
 * @param[in]     cb          ptr to codebook descriptor
 * @param[in]     xlat        ptr to translation table or NULL
 * @param[in,out] tab_offset  starting offset to the generated vlc table
 * @param[out]    out_vlc     ptr to vlc table to be generated
 */
                                         int *tab_offset, VLC *out_vlc)
{

        }
    }


                       xlat, 1, 1, INIT_VLC_USE_NEW_STATIC);


{

        atrac3p_wl_huff_bits1, atrac3p_wl_huff_bits2,
        atrac3p_wl_huff_bits3, atrac3p_wl_huff_bits4
    };
        atrac3p_wl_huff_code1, atrac3p_wl_huff_code2,
        atrac3p_wl_huff_code3, atrac3p_wl_huff_code4
    };
        atrac3p_wl_huff_xlat1, atrac3p_wl_huff_xlat2, NULL, NULL
    };

        atrac3p_ct_huff_bits1, atrac3p_ct_huff_bits2,
        atrac3p_ct_huff_bits2, atrac3p_ct_huff_bits3
    };
        atrac3p_ct_huff_code1, atrac3p_ct_huff_code2,
        atrac3p_ct_huff_code2, atrac3p_ct_huff_code3
    };
        NULL, NULL, atrac3p_ct_huff_xlat1, NULL
    };

        atrac3p_sf_huff_bits1, atrac3p_sf_huff_bits1, atrac3p_sf_huff_bits2,
        atrac3p_sf_huff_bits3, atrac3p_sf_huff_bits4, atrac3p_sf_huff_bits4,
        atrac3p_sf_huff_bits5, atrac3p_sf_huff_bits6
    };
        atrac3p_sf_huff_code1, atrac3p_sf_huff_code1, atrac3p_sf_huff_code2,
        atrac3p_sf_huff_code3, atrac3p_sf_huff_code4, atrac3p_sf_huff_code4,
        atrac3p_sf_huff_code5, atrac3p_sf_huff_code6
    };
        atrac3p_sf_huff_xlat1, atrac3p_sf_huff_xlat2, NULL, NULL,
        atrac3p_sf_huff_xlat4, atrac3p_sf_huff_xlat5, NULL, NULL
    };

        atrac3p_huff_gain_npoints1_cb, atrac3p_huff_gain_npoints1_cb,
        atrac3p_huff_gain_lev1_cb, atrac3p_huff_gain_lev2_cb,
        atrac3p_huff_gain_lev3_cb, atrac3p_huff_gain_lev4_cb,
        atrac3p_huff_gain_loc3_cb, atrac3p_huff_gain_loc1_cb,
        atrac3p_huff_gain_loc4_cb, atrac3p_huff_gain_loc2_cb,
        atrac3p_huff_gain_loc5_cb
    };
        NULL, atrac3p_huff_gain_npoints2_xlat, atrac3p_huff_gain_lev1_xlat,
        atrac3p_huff_gain_lev2_xlat, atrac3p_huff_gain_lev3_xlat,
        atrac3p_huff_gain_lev4_xlat, atrac3p_huff_gain_loc3_xlat,
        atrac3p_huff_gain_loc1_xlat, atrac3p_huff_gain_loc4_xlat,
        atrac3p_huff_gain_loc2_xlat, atrac3p_huff_gain_loc5_xlat
    };

        atrac3p_huff_tonebands_cb,  atrac3p_huff_numwavs1_cb,
        atrac3p_huff_numwavs2_cb,   atrac3p_huff_wav_ampsf1_cb,
        atrac3p_huff_wav_ampsf2_cb, atrac3p_huff_wav_ampsf3_cb,
        atrac3p_huff_freq_cb
    };
        NULL, NULL, atrac3p_huff_numwavs2_xlat, atrac3p_huff_wav_ampsf1_xlat,
        atrac3p_huff_wav_ampsf2_xlat, atrac3p_huff_wav_ampsf3_xlat,
        atrac3p_huff_freq_xlat
    };


                           INIT_VLC_USE_NEW_STATIC);

                           INIT_VLC_USE_NEW_STATIC);

    }


                           INIT_VLC_USE_NEW_STATIC);
    }


    /* build huffman tables for spectrum decoding */
                                 atrac3p_spectra_tabs[i].xlat,
                                 &tab_offset, &spec_vlc_tabs[i]);
        else
    }

    /* build huffman tables for gain data decoding */

    /* build huffman tables for tone decoding */

/**
 * Decode number of coded quantization units.
 *
 * @param[in]     gb            the GetBit context
 * @param[in,out] chan          ptr to the channel parameters
 * @param[in,out] ctx           ptr to the channel unit context
 * @param[in]     avctx         ptr to the AVCodecContext
 * @return result code: 0 = OK, otherwise - error code
 */
static int num_coded_units(GetBitContext *gb, Atrac3pChanParams *chan,
                           Atrac3pChanUnitCtx *ctx, AVCodecContext *avctx)
{
    chan->fill_mode = get_bits(gb, 2);
    if (!chan->fill_mode) {
        chan->num_coded_vals = ctx->num_quant_units;
    } else {
        chan->num_coded_vals = get_bits(gb, 5);
        if (chan->num_coded_vals > ctx->num_quant_units) {
            av_log(avctx, AV_LOG_ERROR,
                   "Invalid number of transmitted units!\n");
            return AVERROR_INVALIDDATA;
        }

        if (chan->fill_mode == 3)
            chan->split_point = get_bits(gb, 2) + (chan->ch_num << 1) + 1;
    }

    return 0;
}

/**
 * Add weighting coefficients to the decoded word-length information.
 *
 * @param[in,out] ctx           ptr to the channel unit context
 * @param[in,out] chan          ptr to the channel parameters
 * @param[in]     wtab_idx      index of the table of weights
 * @param[in]     avctx         ptr to the AVCodecContext
 * @return result code: 0 = OK, otherwise - error code
 */
static int add_wordlen_weights(Atrac3pChanUnitCtx *ctx,
                               Atrac3pChanParams *chan, int wtab_idx,
                               AVCodecContext *avctx)
{
    int i;
    const int8_t *weights_tab =
        &atrac3p_wl_weights[chan->ch_num * 3 + wtab_idx - 1][0];

    for (i = 0; i < ctx->num_quant_units; i++) {
        chan->qu_wordlen[i] += weights_tab[i];
        if (chan->qu_wordlen[i] < 0 || chan->qu_wordlen[i] > 7) {
            av_log(avctx, AV_LOG_ERROR,
                   "WL index out of range: pos=%d, val=%d!\n",
                   i, chan->qu_wordlen[i]);
            return AVERROR_INVALIDDATA;
        }
    }

    return 0;
}

/**
 * Subtract weighting coefficients from decoded scalefactors.
 *
 * @param[in,out] ctx           ptr to the channel unit context
 * @param[in,out] chan          ptr to the channel parameters
 * @param[in]     wtab_idx      index of table of weights
 * @param[in]     avctx         ptr to the AVCodecContext
 * @return result code: 0 = OK, otherwise - error code
 */
static int subtract_sf_weights(Atrac3pChanUnitCtx *ctx,
                               Atrac3pChanParams *chan, int wtab_idx,
                               AVCodecContext *avctx)
{
    int i;
    const int8_t *weights_tab = &atrac3p_sf_weights[wtab_idx - 1][0];

    for (i = 0; i < ctx->used_quant_units; i++) {
        chan->qu_sf_idx[i] -= weights_tab[i];
        if (chan->qu_sf_idx[i] < 0 || chan->qu_sf_idx[i] > 63) {
            av_log(avctx, AV_LOG_ERROR,
                   "SF index out of range: pos=%d, val=%d!\n",
                   i, chan->qu_sf_idx[i]);
            return AVERROR_INVALIDDATA;
        }
    }

    return 0;
}

/**
 * Unpack vector quantization tables.
 *
 * @param[in]    start_val    start value for the unpacked table
 * @param[in]    shape_vec    ptr to table to unpack
 * @param[out]   dst          ptr to output array
 * @param[in]    num_values   number of values to unpack
 */
                                   int *dst, int num_values)
{

    }
}

#define UNPACK_SF_VQ_SHAPE(gb, dst, num_vals)                            \
    start_val = get_bits((gb), 6);                                       \
    unpack_vq_shape(start_val, &atrac3p_sf_shapes[get_bits((gb), 6)][0], \
                    (dst), (num_vals))

/**
 * Decode word length for each quantization unit of a channel.
 *
 * @param[in]     gb            the GetBit context
 * @param[in,out] ctx           ptr to the channel unit context
 * @param[in]     ch_num        channel to process
 * @param[in]     avctx         ptr to the AVCodecContext
 * @return result code: 0 = OK, otherwise - error code
 */
                                  int ch_num, AVCodecContext *avctx)
{
        ret, start_val;


    case 0: /* coded using constant number of bits */
        break;
                return ret;


                }
            }
        } else {
                return ret;

                    av_log(avctx, AV_LOG_ERROR,
                           "WL mode 1: invalid position!\n");
                    return AVERROR_INVALIDDATA;
                }



            }
        }
        break;
            return ret;


            }


                }
            } else {
                    }

            }
        }
        break;
            return ret;


            /* first coefficient is coded directly */

            }
        }
        break;
    }

            av_log(avctx, AV_LOG_ERROR, "Split point beyond array\n");
            pos = FF_ARRAY_ELEMS(chan->qu_wordlen);
        }
    }


    return 0;
}

/**
 * Decode scale factor indexes for each quant unit of a channel.
 *
 * @param[in]     gb            the GetBit context
 * @param[in,out] ctx           ptr to the channel unit context
 * @param[in]     ch_num        channel to process
 * @param[in]     avctx         ptr to the AVCodecContext
 * @return result code: 0 = OK, otherwise - error code
 */
                                 int ch_num, AVCodecContext *avctx)
{
        delta_bits, min_val, vlc_sel, start_val;

    case 0: /* coded using constant number of bits */
        break;

            }
        } else {



                /* all others are: min_val + delta */
            } else {
                    av_log(avctx, AV_LOG_ERROR,
                           "SF mode 1: invalid parameters!\n");
                    return AVERROR_INVALIDDATA;
                }

                /* read full-precision SF indexes */

                /* all others are: min_val + delta */
            }
        }
        break;


            }
        } else {


            }
        }
        break;
            /* copy coefficients from reference channel */
            for (i = 0; i < ctx->used_quant_units; i++)
                chan->qu_sf_idx[i] = ref_chan->qu_sf_idx[i];
        } else {




                }
            } else {
                /* 1st coefficient is coded directly */

                }
            }
        }
        break;
    }


    return 0;
}

/**
 * Decode word length information for each channel.
 *
 * @param[in]     gb            the GetBit context
 * @param[in,out] ctx           ptr to the channel unit context
 * @param[in]     num_channels  number of channels to process
 * @param[in]     avctx         ptr to the AVCodecContext
 * @return result code: 0 = OK, otherwise - error code
 */
                                int num_channels, AVCodecContext *avctx)
{

               sizeof(ctx->channels[ch_num].qu_wordlen));

            return ret;
    }

    /* scan for last non-zero coeff in both channels and
     * set number of quant units having coded spectrum */
            break;

}

/**
 * Decode scale factor indexes for each channel.
 *
 * @param[in]     gb            the GetBit context
 * @param[in,out] ctx           ptr to the channel unit context
 * @param[in]     num_channels  number of channels to process
 * @param[in]     avctx         ptr to the AVCodecContext
 * @return result code: 0 = OK, otherwise - error code
 */
                                int num_channels, AVCodecContext *avctx)
{

        return 0;

               sizeof(ctx->channels[ch_num].qu_sf_idx));

            return ret;
    }

    return 0;
}

/**
 * Decode number of code table values.
 *
 * @param[in]     gb            the GetBit context
 * @param[in,out] ctx           ptr to the channel unit context
 * @param[in]     avctx         ptr to the AVCodecContext
 * @return result code: 0 = OK, otherwise - error code
 */
static int get_num_ct_values(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
                             AVCodecContext *avctx)
{
    int num_coded_vals;

    if (get_bits1(gb)) {
        num_coded_vals = get_bits(gb, 5);
        if (num_coded_vals > ctx->used_quant_units) {
            av_log(avctx, AV_LOG_ERROR,
                   "Invalid number of code table indexes: %d!\n", num_coded_vals);
            return AVERROR_INVALIDDATA;
        }
        return num_coded_vals;
    } else
        return ctx->used_quant_units;
}

#define DEC_CT_IDX_COMMON(OP)                                           \
    num_vals = get_num_ct_values(gb, ctx, avctx);                       \
    if (num_vals < 0)                                                   \
        return num_vals;                                                \
                                                                        \
    for (i = 0; i < num_vals; i++) {                                    \
        if (chan->qu_wordlen[i]) {                                      \
            chan->qu_tab_idx[i] = OP;                                   \
        } else if (ch_num && ref_chan->qu_wordlen[i])                   \
            /* get clone master flag */                                 \
            chan->qu_tab_idx[i] = get_bits1(gb);                        \
    }

#define CODING_DIRECT get_bits(gb, num_bits)

#define CODING_VLC get_vlc2(gb, vlc_tab->table, vlc_tab->bits, 1)

#define CODING_VLC_DELTA                                                \
    (!i) ? CODING_VLC                                                   \
         : (pred + get_vlc2(gb, delta_vlc->table,                       \
                            delta_vlc->bits, 1)) & mask;                \
    pred = chan->qu_tab_idx[i]

#define CODING_VLC_DIFF                                                 \
    (ref_chan->qu_tab_idx[i] +                                          \
     get_vlc2(gb, vlc_tab->table, vlc_tab->bits, 1)) & mask

/**
 * Decode code table indexes for each quant unit of a channel.
 *
 * @param[in]     gb            the GetBit context
 * @param[in,out] ctx           ptr to the channel unit context
 * @param[in]     ch_num        channel to process
 * @param[in]     avctx         ptr to the AVCodecContext
 * @return result code: 0 = OK, otherwise - error code
 */
                                   int ch_num, AVCodecContext *avctx)
{


        break;
        break;
            vlc_tab   = &ct_vlc_tabs[1];
            delta_vlc = &ct_vlc_tabs[2];
        } else {
            vlc_tab   = ct_vlc_tabs;
            delta_vlc = ct_vlc_tabs;
        }
        break;
        }
        break;
    }

    return 0;
}

/**
 * Decode code table indexes for each channel.
 *
 * @param[in]     gb            the GetBit context
 * @param[in,out] ctx           ptr to the channel unit context
 * @param[in]     num_channels  number of channels to process
 * @param[in]     avctx         ptr to the AVCodecContext
 * @return result code: 0 = OK, otherwise - error code
 */
                                     int num_channels, AVCodecContext *avctx)
{

        return 0;


               sizeof(ctx->channels[ch_num].qu_tab_idx));

            return ret;
    }

    return 0;
}

/**
 * Decode huffman-coded spectral lines for a given quant unit.
 *
 * This is a generalized version for all known coding modes.
 * Its speed can be improved by creating separate functions for each mode.
 *
 * @param[in]   gb          the GetBit context
 * @param[in]   tab         code table telling how to decode spectral lines
 * @param[in]   vlc_tab     ptr to the huffman table associated with the code table
 * @param[out]  out         pointer to buffer where decoded data should be stored
 * @param[in]   num_specs   number of spectral lines to decode
 */
static void decode_qu_spectra(GetBitContext *gb, const Atrac3pSpecCodeTab *tab,
                              VLC *vlc_tab, int16_t *out, const int num_specs)
{
    int i, j, pos, cf;
    int group_size = tab->group_size;
    int num_coeffs = tab->num_coeffs;
    int bits       = tab->bits;
    int is_signed  = tab->is_signed;
    unsigned val;

    for (pos = 0; pos < num_specs;) {
        if (group_size == 1 || get_bits1(gb)) {
            for (j = 0; j < group_size; j++) {
                val = get_vlc2(gb, vlc_tab->table, vlc_tab->bits, 1);

                for (i = 0; i < num_coeffs; i++) {
                    cf = av_mod_uintp2(val, bits);
                    if (is_signed)
                        cf = sign_extend(cf, bits);
                    else if (cf && get_bits1(gb))
                        cf = -cf;

                    out[pos++] = cf;
                    val      >>= bits;
                }
            }
        } else /* group skipped */
            pos += group_size * num_coeffs;
    }
}

/**
 * Decode huffman-coded IMDCT spectrum for all channels.
 *
 * @param[in]     gb            the GetBit context
 * @param[in,out] ctx           ptr to the channel unit context
 * @param[in]     num_channels  number of channels to process
 * @param[in]     avctx         ptr to the AVCodecContext
 */
static void decode_spectrum(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
                            int num_channels, AVCodecContext *avctx)
{
    int i, ch_num, qu, wordlen, codetab, tab_index, num_specs;
    const Atrac3pSpecCodeTab *tab;
    Atrac3pChanParams *chan;

    for (ch_num = 0; ch_num < num_channels; ch_num++) {
        chan = &ctx->channels[ch_num];

        memset(chan->spectrum, 0, sizeof(chan->spectrum));

        /* set power compensation level to disabled */
        memset(chan->power_levs, ATRAC3P_POWER_COMP_OFF, sizeof(chan->power_levs));

        for (qu = 0; qu < ctx->used_quant_units; qu++) {
            num_specs = ff_atrac3p_qu_to_spec_pos[qu + 1] -
                        ff_atrac3p_qu_to_spec_pos[qu];

            wordlen = chan->qu_wordlen[qu];
            codetab = chan->qu_tab_idx[qu];
            if (wordlen) {
                if (!ctx->use_full_table)
                    codetab = atrac3p_ct_restricted_to_full[chan->table_type][wordlen - 1][codetab];

                tab_index = (chan->table_type * 8 + codetab) * 7 + wordlen - 1;
                tab       = &atrac3p_spectra_tabs[tab_index];

                /* this allows reusing VLC tables */
                if (tab->redirect >= 0)
                    tab_index = tab->redirect;

                decode_qu_spectra(gb, tab, &spec_vlc_tabs[tab_index],
                                  &chan->spectrum[ff_atrac3p_qu_to_spec_pos[qu]],
                                  num_specs);
            } else if (ch_num && ctx->channels[0].qu_wordlen[qu] && !codetab) {
                /* copy coefficients from master */
                memcpy(&chan->spectrum[ff_atrac3p_qu_to_spec_pos[qu]],
                       &ctx->channels[0].spectrum[ff_atrac3p_qu_to_spec_pos[qu]],
                       num_specs *
                       sizeof(chan->spectrum[ff_atrac3p_qu_to_spec_pos[qu]]));
                chan->qu_wordlen[qu] = ctx->channels[0].qu_wordlen[qu];
            }
        }

        /* Power compensation levels only present in the bitstream
         * if there are more than 2 quant units. The lowest two units
         * correspond to the frequencies 0...351 Hz, whose shouldn't
         * be affected by the power compensation. */
        if (ctx->used_quant_units > 2) {
            num_specs = atrac3p_subband_to_num_powgrps[ctx->num_coded_subbands - 1];
            for (i = 0; i < num_specs; i++)
                chan->power_levs[i] = get_bits(gb, 4);
        }
    }
}

/**
 * Retrieve specified amount of flag bits from the input bitstream.
 * The data can be shortened in the case of the following two common conditions:
 * if all bits are zero then only one signal bit = 0 will be stored,
 * if all bits are ones then two signal bits = 1,0 will be stored.
 * Otherwise, all necessary bits will be directly stored
 * prefixed by two signal bits = 1,1.
 *
 * @param[in]   gb              ptr to the GetBitContext
 * @param[out]  out             where to place decoded flags
 * @param[in]   num_flags       number of flags to process
 * @return: 0 = all flag bits are zero, 1 = there is at least one non-zero flag bit
 */
{


        else
    }

}

/**
 * Decode mdct window shape flags for all channels.
 *
 * @param[in]     gb            the GetBit context
 * @param[in,out] ctx           ptr to the channel unit context
 * @param[in]     num_channels  number of channels to process
 */
static void decode_window_shape(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
                                int num_channels)
{
    int ch_num;

                          ctx->num_subbands);
}

/**
 * Decode number of gain control points.
 *
 * @param[in]     gb              the GetBit context
 * @param[in,out] ctx             ptr to the channel unit context
 * @param[in]     ch_num          channel to process
 * @param[in]     coded_subbands  number of subbands to process
 * @return result code: 0 = OK, otherwise - error code
 */
                                int ch_num, int coded_subbands)
{

    case 0: /* fixed-length coding */
        break;
    case 1: /* variable-length coding */
                         gain_vlc_tabs[0].bits, 1);
        break;
                                 gain_vlc_tabs[1].bits, 1);
            }
        } else { /* VLC modulo delta to previous */
                         gain_vlc_tabs[0].bits, 1);

                                 gain_vlc_tabs[1].bits, 1);
            }
        }
        break;
        } else { /* shorter delta to min */

                    return AVERROR_INVALIDDATA;
            }
        }
    }

    return 0;
}

/**
 * Implements coding mode 3 (slave) for gain compensation levels.
 *
 * @param[out]   dst   ptr to the output array
 * @param[in]    ref   ptr to the reference channel
 */
{

}

/**
 * Implements coding mode 1 (master) for gain compensation levels.
 *
 * @param[in]     gb     the GetBit context
 * @param[in]     ctx    ptr to the channel unit context
 * @param[out]    dst    ptr to the output array
 */
static inline void gainc_level_mode1m(GetBitContext *gb,
                                      Atrac3pChanUnitCtx *ctx,
                                      AtracGainInfo *dst)
{
    int i, delta;

    if (dst->num_points > 0)
        dst->lev_code[0] = get_vlc2(gb, gain_vlc_tabs[2].table,
                                    gain_vlc_tabs[2].bits, 1);

    for (i = 1; i < dst->num_points; i++) {
        delta = get_vlc2(gb, gain_vlc_tabs[3].table,
                         gain_vlc_tabs[3].bits, 1);
        dst->lev_code[i] = (dst->lev_code[i - 1] + delta) & 0xF;
    }
}

/**
 * Decode level code for each gain control point.
 *
 * @param[in]     gb              the GetBit context
 * @param[in,out] ctx             ptr to the channel unit context
 * @param[in]     ch_num          channel to process
 * @param[in]     coded_subbands  number of subbands to process
 * @return result code: 0 = OK, otherwise - error code
 */
                               int ch_num, int coded_subbands)
{

    case 0: /* fixed-length coding */
        break;
                                     gain_vlc_tabs[5].bits, 1);
                }
        } else { /* VLC modulo delta to previous */
        }
        break;
                    else
                }
        } else { /* VLC modulo delta to lev_codes of previous subband */

                                     gain_vlc_tabs[4].bits, 1);
                }
        }
        break;
        } else { /* shorter delta to min */
            delta_bits = get_bits(gb, 2);
            min_val    = get_bits(gb, 4);

            for (sb = 0; sb < coded_subbands; sb++)
                for (i = 0; i < chan->gain_data[sb].num_points; i++) {
                    chan->gain_data[sb].lev_code[i] = min_val + get_bitsz(gb, delta_bits);
                    if (chan->gain_data[sb].lev_code[i] > 15)
                        return AVERROR_INVALIDDATA;
                }
        }
        break;
    }

    return 0;
}

/**
 * Implements coding mode 0 for gain compensation locations.
 *
 * @param[in]     gb     the GetBit context
 * @param[in]     ctx    ptr to the channel unit context
 * @param[out]    dst    ptr to the output array
 * @param[in]     pos    position of the value to be processed
 */
static inline void gainc_loc_mode0(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
                                   AtracGainInfo *dst, int pos)
{
    int delta_bits;

    if (!pos || dst->loc_code[pos - 1] < 15)
        dst->loc_code[pos] = get_bits(gb, 5);
    else if (dst->loc_code[pos - 1] >= 30)
        dst->loc_code[pos] = 31;
    else {
        delta_bits         = av_log2(30 - dst->loc_code[pos - 1]) + 1;
        dst->loc_code[pos] = dst->loc_code[pos - 1] +
                             get_bits(gb, delta_bits) + 1;
    }
}

/**
 * Implements coding mode 1 for gain compensation locations.
 *
 * @param[in]     gb     the GetBit context
 * @param[in]     ctx    ptr to the channel unit context
 * @param[out]    dst    ptr to the output array
 */
static inline void gainc_loc_mode1(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
                                   AtracGainInfo *dst)
{
    int i;
    VLC *tab;

    if (dst->num_points > 0) {
        /* 1st coefficient is stored directly */
        dst->loc_code[0] = get_bits(gb, 5);

        for (i = 1; i < dst->num_points; i++) {
            /* switch VLC according to the curve direction
             * (ascending/descending) */
            tab              = (dst->lev_code[i] <= dst->lev_code[i - 1])
                               ? &gain_vlc_tabs[7]
                               : &gain_vlc_tabs[9];
            dst->loc_code[i] = dst->loc_code[i - 1] +
                               get_vlc2(gb, tab->table, tab->bits, 1);
        }
    }
}

/**
 * Decode location code for each gain control point.
 *
 * @param[in]     gb              the GetBit context
 * @param[in,out] ctx             ptr to the channel unit context
 * @param[in]     ch_num          channel to process
 * @param[in]     coded_subbands  number of subbands to process
 * @param[in]     avctx           ptr to the AVCodecContext
 * @return result code: 0 = OK, otherwise - error code
 */
                                  int ch_num, int coded_subbands,
                                  AVCodecContext *avctx)
{

    case 0: /* sequence of numbers in ascending order */
        break;

                /* 1st value is vlc-coded modulo delta to master */
                                 gain_vlc_tabs[10].bits, 1);

                        /* ascending curve */
                        if (more_than_ref) {
                            delta =
                                get_vlc2(gb, gain_vlc_tabs[9].table,
                                         gain_vlc_tabs[9].bits, 1);
                            dst->loc_code[i] = dst->loc_code[i - 1] + delta;
                        } else {
                            if (get_bits1(gb))
                                gainc_loc_mode0(gb, ctx, dst, i);  // direct coding
                            else
                                dst->loc_code[i] = ref->loc_code[i];  // clone master
                        }
                    } else { /* descending curve */
                        else
                    }
                }
            }
        } else /* VLC delta to previous */
        break;
                else /* clone master for the whole subband */
            }
        } else {
            /* data for the first subband is coded directly */


                /* 1st value is vlc-coded modulo delta to the corresponding
                 * value of the previous subband if any or zero */
                                 gain_vlc_tabs[6].bits, 1);

                    /* Select VLC table according to curve direction and
                     * presence of prediction. */
                    else
                }
            }
        }
        break;
                    else
                }
        } else { /* shorter delta to min */

        }
        break;
    }

    /* Validate decoded information */
                av_log(avctx, AV_LOG_ERROR,
                       "Invalid gain location: ch=%d, sb=%d, pos=%d, val=%d\n",
                       ch_num, sb, i, dst->loc_code[i]);
                return AVERROR_INVALIDDATA;
            }
        }
    }

    return 0;
}

/**
 * Decode gain control data for all channels.
 *
 * @param[in]     gb            the GetBit context
 * @param[in,out] ctx           ptr to the channel unit context
 * @param[in]     num_channels  number of channels to process
 * @param[in]     avctx         ptr to the AVCodecContext
 * @return result code: 0 = OK, otherwise - error code
 */
                             int num_channels, AVCodecContext *avctx)
{

               sizeof(*ctx->channels[ch_num].gain_data) * ATRAC3P_SUBBANDS);

            else

                return ret;

            }
        } else {
        }
    }

    return 0;
}

/**
 * Decode envelope for all tones of a channel.
 *
 * @param[in]     gb                the GetBit context
 * @param[in,out] ctx               ptr to the channel unit context
 * @param[in]     ch_num            channel to process
 * @param[in]     band_has_tones    ptr to an array of per-band-flags:
 *                                  1 - tone data present
 */
static void decode_tones_envelope(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
                                  int ch_num, int band_has_tones[])
{
    int sb;
    Atrac3pWavesData *dst = ctx->channels[ch_num].tones_info;
    Atrac3pWavesData *ref = ctx->channels[0].tones_info;

    if (!ch_num || !get_bits1(gb)) { /* mode 0: fixed-length coding */
        for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++) {
            if (!band_has_tones[sb])
                continue;
            dst[sb].pend_env.has_start_point = get_bits1(gb);
            dst[sb].pend_env.start_pos       = dst[sb].pend_env.has_start_point
                                               ? get_bits(gb, 5) : -1;
            dst[sb].pend_env.has_stop_point  = get_bits1(gb);
            dst[sb].pend_env.stop_pos        = dst[sb].pend_env.has_stop_point
                                               ? get_bits(gb, 5) : 32;
        }
    } else { /* mode 1(slave only): copy master */
        for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++) {
            if (!band_has_tones[sb])
                continue;
            dst[sb].pend_env.has_start_point = ref[sb].pend_env.has_start_point;
            dst[sb].pend_env.has_stop_point  = ref[sb].pend_env.has_stop_point;
            dst[sb].pend_env.start_pos       = ref[sb].pend_env.start_pos;
            dst[sb].pend_env.stop_pos        = ref[sb].pend_env.stop_pos;
        }
    }
}

/**
 * Decode number of tones for each subband of a channel.
 *
 * @param[in]     gb                the GetBit context
 * @param[in,out] ctx               ptr to the channel unit context
 * @param[in]     ch_num            channel to process
 * @param[in]     band_has_tones    ptr to an array of per-band-flags:
 *                                  1 - tone data present
 * @param[in]     avctx             ptr to the AVCodecContext
 * @return result code: 0 = OK, otherwise - error code
 */
static int decode_band_numwavs(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
                               int ch_num, int band_has_tones[],
                               AVCodecContext *avctx)
{
    int mode, sb, delta;
    Atrac3pWavesData *dst = ctx->channels[ch_num].tones_info;
    Atrac3pWavesData *ref = ctx->channels[0].tones_info;

    mode = get_bits(gb, ch_num + 1);
    switch (mode) {
    case 0: /** fixed-length coding */
        for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++)
            if (band_has_tones[sb])
                dst[sb].num_wavs = get_bits(gb, 4);
        break;
    case 1: /** variable-length coding */
        for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++)
            if (band_has_tones[sb])
                dst[sb].num_wavs =
                    get_vlc2(gb, tone_vlc_tabs[1].table,
                             tone_vlc_tabs[1].bits, 1);
        break;
    case 2: /** VLC modulo delta to master (slave only) */
        for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++)
            if (band_has_tones[sb]) {
                delta = get_vlc2(gb, tone_vlc_tabs[2].table,
                                 tone_vlc_tabs[2].bits, 1);
                delta = sign_extend(delta, 3);
                dst[sb].num_wavs = (ref[sb].num_wavs + delta) & 0xF;
            }
        break;
    case 3: /** copy master (slave only) */
        for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++)
            if (band_has_tones[sb])
                dst[sb].num_wavs = ref[sb].num_wavs;
        break;
    }

    /** initialize start tone index for each subband */
    for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++)
        if (band_has_tones[sb]) {
            if (ctx->waves_info->tones_index + dst[sb].num_wavs > 48) {
                av_log(avctx, AV_LOG_ERROR,
                       "Too many tones: %d (max. 48), frame: %d!\n",
                       ctx->waves_info->tones_index + dst[sb].num_wavs,
                       avctx->frame_number);
                return AVERROR_INVALIDDATA;
            }
            dst[sb].start_index           = ctx->waves_info->tones_index;
            ctx->waves_info->tones_index += dst[sb].num_wavs;
        }

    return 0;
}

/**
 * Decode frequency information for each subband of a channel.
 *
 * @param[in]     gb                the GetBit context
 * @param[in,out] ctx               ptr to the channel unit context
 * @param[in]     ch_num            channel to process
 * @param[in]     band_has_tones    ptr to an array of per-band-flags:
 *                                  1 - tone data present
 */
static void decode_tones_frequency(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
                                   int ch_num, int band_has_tones[])
{
    int sb, i, direction, nbits, pred, delta;
    Atrac3pWaveParam *iwav, *owav;
    Atrac3pWavesData *dst = ctx->channels[ch_num].tones_info;
    Atrac3pWavesData *ref = ctx->channels[0].tones_info;

    if (!ch_num || !get_bits1(gb)) { /* mode 0: fixed-length coding */
        for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++) {
            if (!band_has_tones[sb] || !dst[sb].num_wavs)
                continue;
            iwav      = &ctx->waves_info->waves[dst[sb].start_index];
            direction = (dst[sb].num_wavs > 1) ? get_bits1(gb) : 0;
            if (direction) { /** packed numbers in descending order */
                if (dst[sb].num_wavs)
                    iwav[dst[sb].num_wavs - 1].freq_index = get_bits(gb, 10);
                for (i = dst[sb].num_wavs - 2; i >= 0 ; i--) {
                    nbits = av_log2(iwav[i+1].freq_index) + 1;
                    iwav[i].freq_index = get_bits(gb, nbits);
                }
            } else { /** packed numbers in ascending order */
                for (i = 0; i < dst[sb].num_wavs; i++) {
                    if (!i || iwav[i - 1].freq_index < 512)
                        iwav[i].freq_index = get_bits(gb, 10);
                    else {
                        nbits = av_log2(1023 - iwav[i - 1].freq_index) + 1;
                        iwav[i].freq_index = get_bits(gb, nbits) +
                                             1024 - (1 << nbits);
                    }
                }
            }
        }
    } else { /* mode 1: VLC modulo delta to master (slave only) */
        for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++) {
            if (!band_has_tones[sb] || !dst[sb].num_wavs)
                continue;
            iwav = &ctx->waves_info->waves[ref[sb].start_index];
            owav = &ctx->waves_info->waves[dst[sb].start_index];
            for (i = 0; i < dst[sb].num_wavs; i++) {
                delta = get_vlc2(gb, tone_vlc_tabs[6].table,
                                 tone_vlc_tabs[6].bits, 1);
                delta = sign_extend(delta, 8);
                pred  = (i < ref[sb].num_wavs) ? iwav[i].freq_index :
                        (ref[sb].num_wavs ? iwav[ref[sb].num_wavs - 1].freq_index : 0);
                owav[i].freq_index = (pred + delta) & 0x3FF;
            }
        }
    }
}

/**
 * Decode amplitude information for each subband of a channel.
 *
 * @param[in]     gb                the GetBit context
 * @param[in,out] ctx               ptr to the channel unit context
 * @param[in]     ch_num            channel to process
 * @param[in]     band_has_tones    ptr to an array of per-band-flags:
 *                                  1 - tone data present
 */
static void decode_tones_amplitude(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
                                   int ch_num, int band_has_tones[])
{
    int mode, sb, j, i, diff, maxdiff, fi, delta, pred;
    Atrac3pWaveParam *wsrc, *wref;
    int refwaves[48] = { 0 };
    Atrac3pWavesData *dst = ctx->channels[ch_num].tones_info;
    Atrac3pWavesData *ref = ctx->channels[0].tones_info;

    if (ch_num) {
        for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++) {
            if (!band_has_tones[sb] || !dst[sb].num_wavs)
                continue;
            wsrc = &ctx->waves_info->waves[dst[sb].start_index];
            wref = &ctx->waves_info->waves[ref[sb].start_index];
            for (j = 0; j < dst[sb].num_wavs; j++) {
                for (i = 0, fi = 0, maxdiff = 1024; i < ref[sb].num_wavs; i++) {
                    diff = FFABS(wsrc[j].freq_index - wref[i].freq_index);
                    if (diff < maxdiff) {
                        maxdiff = diff;
                        fi      = i;
                    }
                }

                if (maxdiff < 8)
                    refwaves[dst[sb].start_index + j] = fi + ref[sb].start_index;
                else if (j < ref[sb].num_wavs)
                    refwaves[dst[sb].start_index + j] = j + ref[sb].start_index;
                else
                    refwaves[dst[sb].start_index + j] = -1;
            }
        }
    }

    mode = get_bits(gb, ch_num + 1);

    switch (mode) {
    case 0: /** fixed-length coding */
        for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++) {
            if (!band_has_tones[sb] || !dst[sb].num_wavs)
                continue;
            if (ctx->waves_info->amplitude_mode)
                for (i = 0; i < dst[sb].num_wavs; i++)
                    ctx->waves_info->waves[dst[sb].start_index + i].amp_sf = get_bits(gb, 6);
            else
                ctx->waves_info->waves[dst[sb].start_index].amp_sf = get_bits(gb, 6);
        }
        break;
    case 1: /** min + VLC delta */
        for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++) {
            if (!band_has_tones[sb] || !dst[sb].num_wavs)
                continue;
            if (ctx->waves_info->amplitude_mode)
                for (i = 0; i < dst[sb].num_wavs; i++)
                    ctx->waves_info->waves[dst[sb].start_index + i].amp_sf =
                        get_vlc2(gb, tone_vlc_tabs[3].table,
                                 tone_vlc_tabs[3].bits, 1) + 20;
            else
                ctx->waves_info->waves[dst[sb].start_index].amp_sf =
                    get_vlc2(gb, tone_vlc_tabs[4].table,
                             tone_vlc_tabs[4].bits, 1) + 24;
        }
        break;
    case 2: /** VLC modulo delta to master (slave only) */
        for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++) {
            if (!band_has_tones[sb] || !dst[sb].num_wavs)
                continue;
            for (i = 0; i < dst[sb].num_wavs; i++) {
                delta = get_vlc2(gb, tone_vlc_tabs[5].table,
                                 tone_vlc_tabs[5].bits, 1);
                delta = sign_extend(delta, 5);
                pred  = refwaves[dst[sb].start_index + i] >= 0 ?
                        ctx->waves_info->waves[refwaves[dst[sb].start_index + i]].amp_sf : 34;
                ctx->waves_info->waves[dst[sb].start_index + i].amp_sf = (pred + delta) & 0x3F;
            }
        }
        break;
    case 3: /** clone master (slave only) */
        for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++) {
            if (!band_has_tones[sb])
                continue;
            for (i = 0; i < dst[sb].num_wavs; i++)
                ctx->waves_info->waves[dst[sb].start_index + i].amp_sf =
                    refwaves[dst[sb].start_index + i] >= 0
                    ? ctx->waves_info->waves[refwaves[dst[sb].start_index + i]].amp_sf
                    : 32;
        }
        break;
    }
}

/**
 * Decode phase information for each subband of a channel.
 *
 * @param[in]     gb                the GetBit context
 * @param[in,out] ctx               ptr to the channel unit context
 * @param[in]     ch_num            channel to process
 * @param[in]     band_has_tones    ptr to an array of per-band-flags:
 *                                  1 - tone data present
 */
static void decode_tones_phase(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
                               int ch_num, int band_has_tones[])
{
    int sb, i;
    Atrac3pWaveParam *wparam;
    Atrac3pWavesData *dst = ctx->channels[ch_num].tones_info;

    for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++) {
        if (!band_has_tones[sb])
            continue;
        wparam = &ctx->waves_info->waves[dst[sb].start_index];
        for (i = 0; i < dst[sb].num_wavs; i++)
            wparam[i].phase_index = get_bits(gb, 5);
    }
}

/**
 * Decode tones info for all channels.
 *
 * @param[in]     gb            the GetBit context
 * @param[in,out] ctx           ptr to the channel unit context
 * @param[in]     num_channels  number of channels to process
 * @param[in]     avctx         ptr to the AVCodecContext
 * @return result code: 0 = OK, otherwise - error code
 */
                             int num_channels, AVCodecContext *avctx)
{

               sizeof(*ctx->channels[ch_num].tones_info) * ATRAC3P_SUBBANDS);

        return 0;

    memset(ctx->waves_info->waves, 0, sizeof(ctx->waves_info->waves));

    ctx->waves_info->amplitude_mode = get_bits1(gb);
    if (!ctx->waves_info->amplitude_mode) {
        avpriv_report_missing_feature(avctx, "GHA amplitude mode 0");
        return AVERROR_PATCHWELCOME;
    }

    ctx->waves_info->num_tone_bands =
        get_vlc2(gb, tone_vlc_tabs[0].table,
                 tone_vlc_tabs[0].bits, 1) + 1;

    if (num_channels == 2) {
        get_subband_flags(gb, ctx->waves_info->tone_sharing, ctx->waves_info->num_tone_bands);
        get_subband_flags(gb, ctx->waves_info->tone_master,  ctx->waves_info->num_tone_bands);
        get_subband_flags(gb, ctx->waves_info->invert_phase, ctx->waves_info->num_tone_bands);
    }

    ctx->waves_info->tones_index = 0;

    for (ch_num = 0; ch_num < num_channels; ch_num++) {
        for (i = 0; i < ctx->waves_info->num_tone_bands; i++)
            band_has_tones[i] = !ch_num ? 1 : !ctx->waves_info->tone_sharing[i];

        decode_tones_envelope(gb, ctx, ch_num, band_has_tones);
        if ((ret = decode_band_numwavs(gb, ctx, ch_num, band_has_tones,
                                       avctx)) < 0)
            return ret;

        decode_tones_frequency(gb, ctx, ch_num, band_has_tones);
        decode_tones_amplitude(gb, ctx, ch_num, band_has_tones);
        decode_tones_phase(gb, ctx, ch_num, band_has_tones);
    }

    if (num_channels == 2) {
        for (i = 0; i < ctx->waves_info->num_tone_bands; i++) {
            if (ctx->waves_info->tone_sharing[i])
                ctx->channels[1].tones_info[i] = ctx->channels[0].tones_info[i];

            if (ctx->waves_info->tone_master[i])
                FFSWAP(Atrac3pWavesData, ctx->channels[0].tones_info[i],
                       ctx->channels[1].tones_info[i]);
        }
    }

    return 0;
}

                                   int num_channels, AVCodecContext *avctx)
{

    /* parse sound header */
        av_log(avctx, AV_LOG_ERROR,
               "Invalid number of quantization units: %d!\n",
               ctx->num_quant_units);
        return AVERROR_INVALIDDATA;
    }


    /* decode various sound parameters */
        return ret;


        return ret;

        return ret;


    }


        return ret;

        return ret;

    /* decode global noise info */
        ctx->noise_level_index = get_bits(gb, 4);
        ctx->noise_table_index = get_bits(gb, 4);
    }

    return 0;
}
