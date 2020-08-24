/*
 * Copyright (C) 2003-2004 The FFmpeg project
 * Copyright (C) 2019 Peter Ross
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
 * On2 VP3/VP4 Video Decoder
 *
 * VP3 Video Decoder by Mike Melanson (mike at multimedia.cx)
 * For more information about the VP3 coding process, visit:
 *   http://wiki.multimedia.cx/index.php?title=On2_VP3
 *
 * Theora decoder by Alex Beregszaszi
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libavutil/imgutils.h"

#include "avcodec.h"
#include "get_bits.h"
#include "hpeldsp.h"
#include "internal.h"
#include "mathops.h"
#include "thread.h"
#include "videodsp.h"
#include "vp3data.h"
#include "vp4data.h"
#include "vp3dsp.h"
#include "xiph.h"

#define FRAGMENT_PIXELS 8

// FIXME split things out into their own arrays
typedef struct Vp3Fragment {
    int16_t dc;
    uint8_t coding_method;
    uint8_t qpi;
} Vp3Fragment;

#define SB_NOT_CODED        0
#define SB_PARTIALLY_CODED  1
#define SB_FULLY_CODED      2

// This is the maximum length of a single long bit run that can be encoded
// for superblock coding or block qps. Theora special-cases this to read a
// bit instead of flipping the current bit to allow for runs longer than 4129.
#define MAXIMUM_LONG_BIT_RUN 4129

#define MODE_INTER_NO_MV      0
#define MODE_INTRA            1
#define MODE_INTER_PLUS_MV    2
#define MODE_INTER_LAST_MV    3
#define MODE_INTER_PRIOR_LAST 4
#define MODE_USING_GOLDEN     5
#define MODE_GOLDEN_MV        6
#define MODE_INTER_FOURMV     7
#define CODING_MODE_COUNT     8

/* special internal mode */
#define MODE_COPY             8

static int theora_decode_header(AVCodecContext *avctx, GetBitContext *gb);
static int theora_decode_tables(AVCodecContext *avctx, GetBitContext *gb);


/* There are 6 preset schemes, plus a free-form scheme */
static const int ModeAlphabet[6][CODING_MODE_COUNT] = {
    /* scheme 1: Last motion vector dominates */
    { MODE_INTER_LAST_MV,    MODE_INTER_PRIOR_LAST,
      MODE_INTER_PLUS_MV,    MODE_INTER_NO_MV,
      MODE_INTRA,            MODE_USING_GOLDEN,
      MODE_GOLDEN_MV,        MODE_INTER_FOURMV },

    /* scheme 2 */
    { MODE_INTER_LAST_MV,    MODE_INTER_PRIOR_LAST,
      MODE_INTER_NO_MV,      MODE_INTER_PLUS_MV,
      MODE_INTRA,            MODE_USING_GOLDEN,
      MODE_GOLDEN_MV,        MODE_INTER_FOURMV },

    /* scheme 3 */
    { MODE_INTER_LAST_MV,    MODE_INTER_PLUS_MV,
      MODE_INTER_PRIOR_LAST, MODE_INTER_NO_MV,
      MODE_INTRA,            MODE_USING_GOLDEN,
      MODE_GOLDEN_MV,        MODE_INTER_FOURMV },

    /* scheme 4 */
    { MODE_INTER_LAST_MV,    MODE_INTER_PLUS_MV,
      MODE_INTER_NO_MV,      MODE_INTER_PRIOR_LAST,
      MODE_INTRA,            MODE_USING_GOLDEN,
      MODE_GOLDEN_MV,        MODE_INTER_FOURMV },

    /* scheme 5: No motion vector dominates */
    { MODE_INTER_NO_MV,      MODE_INTER_LAST_MV,
      MODE_INTER_PRIOR_LAST, MODE_INTER_PLUS_MV,
      MODE_INTRA,            MODE_USING_GOLDEN,
      MODE_GOLDEN_MV,        MODE_INTER_FOURMV },

    /* scheme 6 */
    { MODE_INTER_NO_MV,      MODE_USING_GOLDEN,
      MODE_INTER_LAST_MV,    MODE_INTER_PRIOR_LAST,
      MODE_INTER_PLUS_MV,    MODE_INTRA,
      MODE_GOLDEN_MV,        MODE_INTER_FOURMV },
};

static const uint8_t hilbert_offset[16][2] = {
    { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 },
    { 0, 2 }, { 0, 3 }, { 1, 3 }, { 1, 2 },
    { 2, 2 }, { 2, 3 }, { 3, 3 }, { 3, 2 },
    { 3, 1 }, { 2, 1 }, { 2, 0 }, { 3, 0 }
};

enum {
    VP4_DC_INTRA  = 0,
    VP4_DC_INTER  = 1,
    VP4_DC_GOLDEN = 2,
    NB_VP4_DC_TYPES,
    VP4_DC_UNDEFINED = NB_VP4_DC_TYPES
};

static const uint8_t vp4_pred_block_type_map[8] = {
    [MODE_INTER_NO_MV]      = VP4_DC_INTER,
    [MODE_INTRA]            = VP4_DC_INTRA,
    [MODE_INTER_PLUS_MV]    = VP4_DC_INTER,
    [MODE_INTER_LAST_MV]    = VP4_DC_INTER,
    [MODE_INTER_PRIOR_LAST] = VP4_DC_INTER,
    [MODE_USING_GOLDEN]     = VP4_DC_GOLDEN,
    [MODE_GOLDEN_MV]        = VP4_DC_GOLDEN,
    [MODE_INTER_FOURMV]     = VP4_DC_INTER,
};

typedef struct {
    int dc;
    int type;
} VP4Predictor;

#define MIN_DEQUANT_VAL 2

typedef struct Vp3DecodeContext {
    AVCodecContext *avctx;
    int theora, theora_tables, theora_header;
    int version;
    int width, height;
    int chroma_x_shift, chroma_y_shift;
    ThreadFrame golden_frame;
    ThreadFrame last_frame;
    ThreadFrame current_frame;
    int keyframe;
    uint8_t idct_permutation[64];
    uint8_t idct_scantable[64];
    HpelDSPContext hdsp;
    VideoDSPContext vdsp;
    VP3DSPContext vp3dsp;
    DECLARE_ALIGNED(16, int16_t, block)[64];
    int flipped_image;
    int last_slice_end;
    int skip_loop_filter;

    int qps[3];
    int nqps;
    int last_qps[3];

    int superblock_count;
    int y_superblock_width;
    int y_superblock_height;
    int y_superblock_count;
    int c_superblock_width;
    int c_superblock_height;
    int c_superblock_count;
    int u_superblock_start;
    int v_superblock_start;
    unsigned char *superblock_coding;

    int macroblock_count; /* y macroblock count */
    int macroblock_width;
    int macroblock_height;
    int c_macroblock_count;
    int c_macroblock_width;
    int c_macroblock_height;
    int yuv_macroblock_count; /* y+u+v macroblock count */

    int fragment_count;
    int fragment_width[2];
    int fragment_height[2];

    Vp3Fragment *all_fragments;
    int fragment_start[3];
    int data_offset[3];
    uint8_t offset_x;
    uint8_t offset_y;
    int offset_x_warned;

    int8_t (*motion_val[2])[2];

    /* tables */
    uint16_t coded_dc_scale_factor[2][64];
    uint32_t coded_ac_scale_factor[64];
    uint8_t base_matrix[384][64];
    uint8_t qr_count[2][3];
    uint8_t qr_size[2][3][64];
    uint16_t qr_base[2][3][64];

    /**
     * This is a list of all tokens in bitstream order. Reordering takes place
     * by pulling from each level during IDCT. As a consequence, IDCT must be
     * in Hilbert order, making the minimum slice height 64 for 4:2:0 and 32
     * otherwise. The 32 different tokens with up to 12 bits of extradata are
     * collapsed into 3 types, packed as follows:
     *   (from the low to high bits)
     *
     * 2 bits: type (0,1,2)
     *   0: EOB run, 14 bits for run length (12 needed)
     *   1: zero run, 7 bits for run length
     *                7 bits for the next coefficient (3 needed)
     *   2: coefficient, 14 bits (11 needed)
     *
     * Coefficients are signed, so are packed in the highest bits for automatic
     * sign extension.
     */
    int16_t *dct_tokens[3][64];
    int16_t *dct_tokens_base;
#define TOKEN_EOB(eob_run)              ((eob_run) << 2)
#define TOKEN_ZERO_RUN(coeff, zero_run) (((coeff) * 512) + ((zero_run) << 2) + 1)
#define TOKEN_COEFF(coeff)              (((coeff) * 4) + 2)

    /**
     * number of blocks that contain DCT coefficients at
     * the given level or higher
     */
    int num_coded_frags[3][64];
    int total_num_coded_frags;

    /* this is a list of indexes into the all_fragments array indicating
     * which of the fragments are coded */
    int *coded_fragment_list[3];

    int *kf_coded_fragment_list;
    int *nkf_coded_fragment_list;
    int num_kf_coded_fragment[3];

    VLC dc_vlc[16];
    VLC ac_vlc_1[16];
    VLC ac_vlc_2[16];
    VLC ac_vlc_3[16];
    VLC ac_vlc_4[16];

    VLC superblock_run_length_vlc; /* version < 2 */
    VLC fragment_run_length_vlc; /* version < 2 */
    VLC block_pattern_vlc[2]; /* version >= 2*/
    VLC mode_code_vlc;
    VLC motion_vector_vlc; /* version < 2 */
    VLC vp4_mv_vlc[2][7]; /* version >=2 */

    /* these arrays need to be on 16-byte boundaries since SSE2 operations
     * index into them */
    DECLARE_ALIGNED(16, int16_t, qmat)[3][2][3][64];     ///< qmat[qpi][is_inter][plane]

    /* This table contains superblock_count * 16 entries. Each set of 16
     * numbers corresponds to the fragment indexes 0..15 of the superblock.
     * An entry will be -1 to indicate that no entry corresponds to that
     * index. */
    int *superblock_fragments;

    /* This is an array that indicates how a particular macroblock
     * is coded. */
    unsigned char *macroblock_coding;

    uint8_t *edge_emu_buffer;

    /* Huffman decode */
    int hti;
    unsigned int hbits;
    int entries;
    int huff_code_size;
    uint32_t huffman_table[80][32][2];

    uint8_t filter_limit_values[64];
    DECLARE_ALIGNED(8, int, bounding_values_array)[256 + 2];

    VP4Predictor * dc_pred_row; /* dc_pred_row[y_superblock_width * 4] */
} Vp3DecodeContext;

/************************************************************************
 * VP3 specific functions
 ************************************************************************/

static av_cold void free_tables(AVCodecContext *avctx)
{
    Vp3DecodeContext *s = avctx->priv_data;

    av_freep(&s->superblock_coding);
    av_freep(&s->all_fragments);
    av_freep(&s->nkf_coded_fragment_list);
    av_freep(&s->kf_coded_fragment_list);
    av_freep(&s->dct_tokens_base);
    av_freep(&s->superblock_fragments);
    av_freep(&s->macroblock_coding);
    av_freep(&s->dc_pred_row);
    av_freep(&s->motion_val[0]);
    av_freep(&s->motion_val[1]);
}

{


{



    /* release all frames */

    }



}

/**
 * This function sets up all of the various blocks mappings:
 * superblocks <-> fragments, macroblocks <-> fragments,
 * superblocks <-> macroblocks
 *
 * @return 0 is successful; returns 1 if *anything* went wrong.
 */
{



                    else
                }
    }

}

/*
 * This function sets up the dequantization tables used for a particular
 * frame.
 */
{

                    break;
            }

            }
            /* all DC coefficients use the same quant so as not to interfere
             * with DC prediction */
        }
    }

/*
 * This function initializes the loop filter boundary limits if the frame's
 * quality index is different from the previous frame's.
 *
 * The filter_limit_values may not be larger than 127.
 */
{

/*
 * This function unpacks all of the superblock/macroblock/fragment coding
 * information from the bitstream.
 */
{
    };


    } else {
        /* unpack the list of partially-coded superblocks */

                bit = get_bits1(gb);
            else

                                   6, 2) + 1;

                av_log(s->avctx, AV_LOG_ERROR,
                       "Invalid partially coded superblock run length\n");
                return -1;
            }


        }

        /* unpack the list of fully coded superblocks if any of the blocks were
         * not marked as partially coded in the previous step */


                else

                                       6, 2) + 1;

                        av_log(s->avctx, AV_LOG_ERROR,
                               "Invalid fully coded superblock run length\n");
                        return -1;
                    }

                    /* skip any superblocks already marked as partially coded */
                    }
                }
            }
        }

        /* if there were partial blocks, initialize bitstream for
         * unpacking fragment codings */
            /* toggle the bit because as soon as the first run length is
             * fetched the bit will be toggled again */
        }
    }

    /* figure out which fragments are coded; iterate through each
     * superblock (all planes) */



                    /* iterate through all 16 fragments in a superblock */
                        /* if the fragment is in bounds, check its coding status */
                                current_fragment;
                        }
                    }
                }
            } else
                num_coded_frags = s->num_kf_coded_fragment[plane];
        } else {
                    return AVERROR_INVALIDDATA;
                }
                /* iterate through all 16 fragments in a superblock */
                    /* if the fragment is in bounds, check its coding status */

                            /* fragment may or may not be coded; this is the case
                             * that cares about the fragment coding runs */
                            }
                            coded = bit;
                        }

                            /* default mode; actual mode will be decoded in
                             * the next phase */
                                MODE_INTER_NO_MV;
                                current_fragment;
                        } else {
                            /* not coded; copy this fragment from the prior frame */
                                MODE_COPY;
                        }
                    }
                }
            }
        }
                                                num_coded_frags;
    }
    return 0;
}

#define BLOCK_X (2 * mb_x + (k & 1))
#define BLOCK_Y (2 * mb_y + (k >> 1))

#if CONFIG_VP4_DECODER
/**
 * @return number of blocks, or > yuv_macroblock_count on error.
 *         return value is always >= 1.
 */
static int vp4_get_mb_count(Vp3DecodeContext *s, GetBitContext *gb)
{
    int v = 1;
    int bits;
    while ((bits = show_bits(gb, 9)) == 0x1ff) {
        skip_bits(gb, 9);
        v += 256;
        if (v > s->yuv_macroblock_count) {
            av_log(s->avctx, AV_LOG_ERROR, "Invalid run length\n");
            return v;
        }
    }
#define body(n) { \
    skip_bits(gb, 2 + n); \
    v += (1 << n) + get_bits(gb, n); }
#define thresh(n) (0x200 - (0x80 >> n))
#define else_if(n) else if (bits < thresh(n)) body(n)
    if (bits < 0x100) {
        skip_bits(gb, 1);
    } else if (bits < thresh(0)) {
        skip_bits(gb, 2);
        v += 1;
    }
    else_if(1)
    else_if(2)
    else_if(3)
    else_if(4)
    else_if(5)
    else_if(6)
    else body(7)
#undef body
#undef thresh
#undef else_if
    return v;
}

{
        av_log(s->avctx, AV_LOG_ERROR, "Invalid block pattern\n");
        *next_block_pattern_table = 0;
        return 0;
    }
}

{


        return 0;

            return AVERROR_INVALIDDATA;
            return -1;
    }

            return AVERROR_INVALIDDATA;
                }
            }
        }
            return -1;
    }





                        pattern = 0xF;
                    else
                        pattern = 0;

                            continue;
                        /* MODE_INTER_NO_MV is the default for coded fragments.
                           the actual method is decoded in the next phase. */
                    }
                }
            }
        }
    }
    return 0;
}
#endif

/*
 * This function unpacks all the coding mode data for individual macroblocks
 * from the bitstream.
 */
{

    } else {
        /* fetch the mode coding scheme for this frame */

        /* is it a custom coding scheme? */
            alphabet = custom_mode_alphabet;
        } else

        /* iterate through all of the macroblocks that contain 1 or more
         * coded fragments */
                    return -1;



                    /* coding modes are only stored if the macroblock has
                     * at least one luma block coded, otherwise it must be
                     * INTER_NO_MV */
                            break;
                    }
                    }

                    /* mode 7 means get 3 bits for each coding mode */
                    else

                    }

#define SET_CHROMA_MODES                                                      \
    if (frag[s->fragment_start[1]].coding_method != MODE_COPY)                \
        frag[s->fragment_start[1]].coding_method = coding_mode;               \
    if (frag[s->fragment_start[2]].coding_method != MODE_COPY)                \
        frag[s->fragment_start[2]].coding_method = coding_mode;

                    } else if (s->chroma_x_shift) {
                        frag = s->all_fragments +
                               2 * mb_y * s->fragment_width[1] + mb_x;
                        for (k = 0; k < 2; k++) {
                            SET_CHROMA_MODES
                            frag += s->fragment_width[1];
                        }
                    } else {
                        for (k = 0; k < 4; k++) {
                            frag = s->all_fragments +
                                   BLOCK_Y * s->fragment_width[1] + BLOCK_X;
                            SET_CHROMA_MODES
                        }
                    }
                }
            }
        }
    }

    return 0;
}

{
}

/*
 * This function unpacks all the motion vectors for the individual
 * macroblocks from the bitstream.
 */
{

        return 0;

    /* coding mode 0 is the VLC scheme; 1 is the fixed code scheme; 2 is VP4 code scheme */

    /* iterate through all of the macroblocks that contain 1 or more
     * coded fragments */
                return -1;



                    } /* otherwise fall through */
                case MODE_INTER_PLUS_MV:
                    /* all 6 fragments use the same motion vector */
                    } else { /* VP4 */
                    }

                    /* vector maintenance, only on MODE_INTER_PLUS_MV */
                    }
                    break;

                case MODE_INTER_FOURMV:
                    /* vector maintenance */

                    /* fetch 4 vectors from the bitstream, one for each
                     * Y fragment, then average for the C fragment vectors */
                            } else { /* VP4 */
                            }
                        } else {
                            motion_x[k] = 0;
                            motion_y[k] = 0;
                        }
                    }
                    break;

                    /* all 6 fragments use the last motion vector */

                    /* no vector maintenance (last vector remains the
                     * last vector) */

                    /* all 6 fragments use the motion vector prior to the
                     * last motion vector */

                    /* vector maintenance */

                    /* covers intra, inter without MV, golden without MV */

                    /* no vector maintenance */
                }

                /* assign the motion vectors to the correct fragments */
                    } else {
                    }
                }

                                             motion_x[2] + motion_x[3], 2);
                                             motion_y[2] + motion_y[3], 2);
                    }
                    }
                } else if (s->chroma_x_shift) {
                    if (s->macroblock_coding[current_macroblock] == MODE_INTER_FOURMV) {
                        motion_x[0] = RSHIFT(motion_x[0] + motion_x[1], 1);
                        motion_y[0] = RSHIFT(motion_y[0] + motion_y[1], 1);
                        motion_x[1] = RSHIFT(motion_x[2] + motion_x[3], 1);
                        motion_y[1] = RSHIFT(motion_y[2] + motion_y[3], 1);
                    } else {
                        motion_x[1] = motion_x[0];
                        motion_y[1] = motion_y[0];
                    }
                    if (s->version <= 2) {
                        motion_x[0] = (motion_x[0] >> 1) | (motion_x[0] & 1);
                        motion_x[1] = (motion_x[1] >> 1) | (motion_x[1] & 1);
                    }
                    frag = 2 * mb_y * s->fragment_width[1] + mb_x;
                    for (k = 0; k < 2; k++) {
                        s->motion_val[1][frag][0] = motion_x[k];
                        s->motion_val[1][frag][1] = motion_y[k];
                        frag += s->fragment_width[1];
                    }
                } else {
                    for (k = 0; k < 4; k++) {
                        frag = BLOCK_Y * s->fragment_width[1] + BLOCK_X;
                        if (s->macroblock_coding[current_macroblock] == MODE_INTER_FOURMV) {
                            s->motion_val[1][frag][0] = motion_x[k];
                            s->motion_val[1][frag][1] = motion_y[k];
                        } else {
                            s->motion_val[1][frag][0] = motion_x[0];
                            s->motion_val[1][frag][1] = motion_y[0];
                        }
                    }
                }
            }
        }
    }

    return 0;
}

{

        i = blocks_decoded = num_blocks_at_qpi = 0;

        bit        = get_bits1(gb) ^ 1;
        run_length = 0;

        do {
            if (run_length == MAXIMUM_LONG_BIT_RUN)
                bit = get_bits1(gb);
            else
                bit ^= 1;

            run_length = get_vlc2(gb, s->superblock_run_length_vlc.table, 6, 2) + 1;
            if (run_length == 34)
                run_length += get_bits(gb, 12);
            blocks_decoded += run_length;

            if (!bit)
                num_blocks_at_qpi += run_length;

            for (j = 0; j < run_length; i++) {
                if (i >= s->total_num_coded_frags)
                    return -1;

                if (s->all_fragments[s->coded_fragment_list[0][i]].qpi == qpi) {
                    s->all_fragments[s->coded_fragment_list[0][i]].qpi += bit;
                    j++;
                }
            }
        } while (blocks_decoded < num_blocks && get_bits_left(gb) > 0);

        num_blocks -= num_blocks_at_qpi;
    }

    return 0;
}

{
}

{



}

/*
 * This function is called by unpack_dct_coeffs() to extract the VLCs from
 * the bitstream. The VLCs encode tokens which are used to unpack DCT
 * data. This function unpacks all the VLCs for either the Y plane or both
 * C planes, and is called for DC coefficients or different AC coefficient
 * levels (since different coefficient types require different VLC tables.
 *
 * This function returns a residual eob run. E.g, if a particular token gave
 * instructions to EOB the next 5 fragments and there were only 2 fragments
 * left in the current fragment range, 3 would be returned so that it could
 * be passed into the next call to this same function.
 */
static int unpack_vlcs(Vp3DecodeContext *s, GetBitContext *gb,
                       VLC *table, int coeff_index,
                       int plane,
                       int eob_run)
{
    int i, j = 0;
    int token;
    int zero_run  = 0;
    int16_t coeff = 0;
    int blocks_ended;
    int coeff_i = 0;
    int num_coeffs      = s->num_coded_frags[plane][coeff_index];
    int16_t *dct_tokens = s->dct_tokens[plane][coeff_index];

    /* local references to structure members to avoid repeated dereferences */
    int *coded_fragment_list   = s->coded_fragment_list[plane];
    Vp3Fragment *all_fragments = s->all_fragments;
    VLC_TYPE(*vlc_table)[2] = table->table;

    if (num_coeffs < 0) {
        av_log(s->avctx, AV_LOG_ERROR,
               "Invalid number of coefficients at level %d\n", coeff_index);
        return AVERROR_INVALIDDATA;
    }

    if (eob_run > num_coeffs) {
        coeff_i      =
        blocks_ended = num_coeffs;
        eob_run     -= num_coeffs;
    } else {
        coeff_i      =
        blocks_ended = eob_run;
        eob_run      = 0;
    }

    // insert fake EOB token to cover the split between planes or zzi
    if (blocks_ended)
        dct_tokens[j++] = blocks_ended << 2;

    while (coeff_i < num_coeffs && get_bits_left(gb) > 0) {
        /* decode a VLC into a token */
        token = get_vlc2(gb, vlc_table, 11, 3);
        /* use the token to get a zero run, a coefficient, and an eob run */
        if ((unsigned) token <= 6U) {
            eob_run = get_eob_run(gb, token);
            if (!eob_run)
                eob_run = INT_MAX;

            // record only the number of blocks ended in this plane,
            // any spill will be recorded in the next plane.
            if (eob_run > num_coeffs - coeff_i) {
                dct_tokens[j++] = TOKEN_EOB(num_coeffs - coeff_i);
                blocks_ended   += num_coeffs - coeff_i;
                eob_run        -= num_coeffs - coeff_i;
                coeff_i         = num_coeffs;
            } else {
                dct_tokens[j++] = TOKEN_EOB(eob_run);
                blocks_ended   += eob_run;
                coeff_i        += eob_run;
                eob_run         = 0;
            }
        } else if (token >= 0) {
            zero_run = get_coeff(gb, token, &coeff);

            if (zero_run) {
                dct_tokens[j++] = TOKEN_ZERO_RUN(coeff, zero_run);
            } else {
                // Save DC into the fragment structure. DC prediction is
                // done in raster order, so the actual DC can't be in with
                // other tokens. We still need the token in dct_tokens[]
                // however, or else the structure collapses on itself.
                if (!coeff_index)
                    all_fragments[coded_fragment_list[coeff_i]].dc = coeff;

                dct_tokens[j++] = TOKEN_COEFF(coeff);
            }

            if (coeff_index + zero_run > 64) {
                av_log(s->avctx, AV_LOG_DEBUG,
                       "Invalid zero run of %d with %d coeffs left\n",
                       zero_run, 64 - coeff_index);
                zero_run = 64 - coeff_index;
            }

            // zero runs code multiple coefficients,
            // so don't try to decode coeffs for those higher levels
            for (i = coeff_index + 1; i <= coeff_index + zero_run; i++)
                s->num_coded_frags[plane][i]--;
            coeff_i++;
        } else {
            av_log(s->avctx, AV_LOG_ERROR, "Invalid token %d\n", token);
            return -1;
        }
    }

    if (blocks_ended > s->num_coded_frags[plane][coeff_index])
        av_log(s->avctx, AV_LOG_ERROR, "More blocks ended than coded!\n");

    // decrement the number of blocks that have higher coefficients for each
    // EOB run at this level
    if (blocks_ended)
        for (i = coeff_index + 1; i < 64; i++)
            s->num_coded_frags[plane][i] -= blocks_ended;

    // setup the next buffer
    if (plane < 2)
        s->dct_tokens[plane + 1][coeff_index] = dct_tokens + j;
    else if (coeff_index < 63)
        s->dct_tokens[0][coeff_index + 1] = dct_tokens + j;

    return eob_run;
}

static void reverse_dc_prediction(Vp3DecodeContext *s,
                                  int first_fragment,
                                  int fragment_width,
                                  int fragment_height);
/*
 * This function unpacks all of the DCT coefficient data from the
 * bitstream.
 */
{


        return AVERROR_INVALIDDATA;

    /* fetch the DC table indexes */

    /* unpack the Y plane DC coefficients */
                                   0, residual_eob_run);
        return residual_eob_run;
        return AVERROR_INVALIDDATA;

    /* reverse prediction of the Y-plane DC coefficients */

    /* unpack the C plane DC coefficients */
                                   1, residual_eob_run);
        return residual_eob_run;
                                   2, residual_eob_run);
        return residual_eob_run;

    /* reverse prediction of the C-plane DC coefficients */
                              s->fragment_width[1], s->fragment_height[1]);
                              s->fragment_width[1], s->fragment_height[1]);
    }

        return AVERROR_INVALIDDATA;
    /* fetch the AC table indexes */

    /* build tables of AC VLC tables */
    }
    }
    }
    }

    /* decode all AC coefficients */
                                       0, residual_eob_run);
            return residual_eob_run;

                                       1, residual_eob_run);
            return residual_eob_run;
                                       2, residual_eob_run);
            return residual_eob_run;
    }

    return 0;
}

#if CONFIG_VP4_DECODER
/**
 * eob_tracker[] is instead of TOKEN_EOB(value)
 * a dummy TOKEN_EOB(0) value is used to make vp3_dequant work
 *
 * @return < 0 on error
 */
                       VLC *vlc_tables[64],
                       int plane, int eob_tracker[64], int fragment)
{

            return AVERROR_INVALIDDATA;


        /* use the token to get a zero run, a coefficient, and an eob run */

                    av_log(s->avctx, AV_LOG_DEBUG,
                        "Invalid zero run of %d with %d coeffs left\n",
                        zero_run, 64 - coeff_i);
                    zero_run = 64 - coeff_i;
                }
            } else {

            }
                return 0; /* stop */
        } else {
            av_log(s->avctx, AV_LOG_ERROR, "Invalid token %d\n", token);
            return -1;
        }
    }
}

{
}

static void vp4_dc_pred_before(const Vp3DecodeContext *s, VP4Predictor dc_pred[6][6], int sb_x)
{
    int i, j;

    for (i = 0; i < 4; i++)
        dc_pred[0][i + 1] = s->dc_pred_row[sb_x * 4 + i];

    for (j = 1; j < 5; j++)
        for (i = 0; i < 4; i++)
            vp4_dc_predictor_reset(&dc_pred[j][i + 1]);
}

static void vp4_dc_pred_after(Vp3DecodeContext *s, VP4Predictor dc_pred[6][6], int sb_x)
{
    int i;


}

/* note: dc_pred points to the current block */
static int vp4_dc_pred(const Vp3DecodeContext *s, const VP4Predictor * dc_pred, const int * last_dc, int type, int plane)
{
    int count = 0;
    int dc = 0;

    if (dc_pred[-6].type == type) {
        dc += dc_pred[-6].dc;
        count++;
    }

    if (dc_pred[6].type == type) {
        dc += dc_pred[6].dc;
        count++;
    }

    if (count != 2 && dc_pred[-1].type == type) {
        dc += dc_pred[-1].dc;
        count++;
    }

    if (count != 2 && dc_pred[1].type == type) {
        dc += dc_pred[1].dc;
        count++;
    }

    /* using division instead of shift to correctly handle negative values */
    return count == 2 ? dc / 2 : last_dc[type];
}

{
        }
    }
}

{

        return AVERROR_INVALIDDATA;

    /* fetch the DC table indexes */


    /* build tables of DC/AC VLC tables */

    }
    }
    }
    }




        /* initialise dc prediction */






                            return -1;



                }
            }
        }
    }


    return 0;
}
#endif

/*
 * This function reverses the DC prediction for each coded fragment in
 * the frame. Much of this function is adapted directly from the original
 * VP3 source code.
 */
#define COMPATIBLE_FRAME(x)                                                   \
    (compatible_frame[s->all_fragments[x].coding_method] == current_frame_type)
#define DC_COEFF(u) s->all_fragments[u].dc

static void reverse_dc_prediction(Vp3DecodeContext *s,
                                  int first_fragment,
                                  int fragment_width,
                                  int fragment_height)
{
#define PUL 8
#define PU 4
#define PUR 2
#define PL 1

    int x, y;
    int i = first_fragment;

    int predicted_dc;

    /* DC values for the left, up-left, up, and up-right fragments */
    int vl, vul, vu, vur;

    /* indexes for the left, up-left, up, and up-right fragments */
    int l, ul, u, ur;

    /*
     * The 6 fields mean:
     *   0: up-left multiplier
     *   1: up multiplier
     *   2: up-right multiplier
     *   3: left multiplier
     */
    static const int predictor_transform[16][4] = {
        {    0,   0,   0,   0 },
        {    0,   0,   0, 128 }, // PL
        {    0,   0, 128,   0 }, // PUR
        {    0,   0,  53,  75 }, // PUR|PL
        {    0, 128,   0,   0 }, // PU
        {    0,  64,   0,  64 }, // PU |PL
        {    0, 128,   0,   0 }, // PU |PUR
        {    0,   0,  53,  75 }, // PU |PUR|PL
        {  128,   0,   0,   0 }, // PUL
        {    0,   0,   0, 128 }, // PUL|PL
        {   64,   0,  64,   0 }, // PUL|PUR
        {    0,   0,  53,  75 }, // PUL|PUR|PL
        {    0, 128,   0,   0 }, // PUL|PU
        { -104, 116,   0, 116 }, // PUL|PU |PL
        {   24,  80,  24,   0 }, // PUL|PU |PUR
        { -104, 116,   0, 116 }  // PUL|PU |PUR|PL
    };

    /* This table shows which types of blocks can use other blocks for
     * prediction. For example, INTRA is the only mode in this table to
     * have a frame number of 0. That means INTRA blocks can only predict
     * from other INTRA blocks. There are 2 golden frame coding types;
     * blocks encoding in these modes can only predict from other blocks
     * that were encoded with these 1 of these 2 modes. */
    static const unsigned char compatible_frame[9] = {
        1,    /* MODE_INTER_NO_MV */
        0,    /* MODE_INTRA */
        1,    /* MODE_INTER_PLUS_MV */
        1,    /* MODE_INTER_LAST_MV */
        1,    /* MODE_INTER_PRIOR_MV */
        2,    /* MODE_USING_GOLDEN */
        2,    /* MODE_GOLDEN_MV */
        1,    /* MODE_INTER_FOUR_MV */
        3     /* MODE_COPY */
    };
    int current_frame_type;

    /* there is a last DC predictor for each of the 3 frame types */
    short last_dc[3];

    int transform = 0;

    vul =
    vu  =
    vur =
    vl  = 0;
    last_dc[0] =
    last_dc[1] =
    last_dc[2] = 0;

    /* for each fragment row... */
    for (y = 0; y < fragment_height; y++) {
        /* for each fragment in a row... */
        for (x = 0; x < fragment_width; x++, i++) {

            /* reverse prediction if this block was coded */
            if (s->all_fragments[i].coding_method != MODE_COPY) {
                current_frame_type =
                    compatible_frame[s->all_fragments[i].coding_method];

                transform = 0;
                if (x) {
                    l  = i - 1;
                    vl = DC_COEFF(l);
                    if (COMPATIBLE_FRAME(l))
                        transform |= PL;
                }
                if (y) {
                    u  = i - fragment_width;
                    vu = DC_COEFF(u);
                    if (COMPATIBLE_FRAME(u))
                        transform |= PU;
                    if (x) {
                        ul  = i - fragment_width - 1;
                        vul = DC_COEFF(ul);
                        if (COMPATIBLE_FRAME(ul))
                            transform |= PUL;
                    }
                    if (x + 1 < fragment_width) {
                        ur  = i - fragment_width + 1;
                        vur = DC_COEFF(ur);
                        if (COMPATIBLE_FRAME(ur))
                            transform |= PUR;
                    }
                }

                if (transform == 0) {
                    /* if there were no fragments to predict from, use last
                     * DC saved */
                    predicted_dc = last_dc[current_frame_type];
                } else {
                    /* apply the appropriate predictor transform */
                    predicted_dc =
                        (predictor_transform[transform][0] * vul) +
                        (predictor_transform[transform][1] * vu) +
                        (predictor_transform[transform][2] * vur) +
                        (predictor_transform[transform][3] * vl);

                    predicted_dc /= 128;

                    /* check for outranging on the [ul u l] and
                     * [ul u ur l] predictors */
                    if ((transform == 15) || (transform == 13)) {
                        if (FFABS(predicted_dc - vu) > 128)
                            predicted_dc = vu;
                        else if (FFABS(predicted_dc - vl) > 128)
                            predicted_dc = vl;
                        else if (FFABS(predicted_dc - vul) > 128)
                            predicted_dc = vul;
                    }
                }

                /* at long last, apply the predictor */
                DC_COEFF(i) += predicted_dc;
                /* save the DC */
                last_dc[current_frame_type] = DC_COEFF(i);
            }
        }
    }
}

                              int ystart, int yend)
{


            /* This code basically just deblocks on the edges of coded blocks.
             * However, it has to be much more complicated because of the
             * brain damaged deblock ordering used in VP3/Theora. Order matters
             * because some pixels get filtered twice. */
                /* do not perform left edge filter for left columns frags */
                        stride, bounding_values);
                }

                /* do not perform top edge filter for top row fragments */
                        stride, bounding_values);
                }

                /* do not perform right edge filter for right column
                 * fragments or if right fragment neighbor is also coded
                 * in this frame (it will be filtered in next iteration) */
                        stride, bounding_values);
                }

                /* do not perform bottom edge filter for bottom row
                 * fragments or if bottom fragment neighbor is also coded
                 * in this frame (it will be filtered in the next row) */
                        stride, bounding_values);
                }
            }

        }
    }

/**
 * Pull DCT tokens from the 64 levels to decode and dequant the coefficients
 * for the next block in coding order
 */
static inline int vp3_dequant(Vp3DecodeContext *s, Vp3Fragment *frag,
                              int plane, int inter, int16_t block[64])
{
    int16_t *dequantizer = s->qmat[frag->qpi][inter][plane];
    uint8_t *perm = s->idct_scantable;
    int i = 0;

    do {
        int token = *s->dct_tokens[plane][i];
        switch (token & 3) {
        case 0: // EOB
            if (--token < 4) // 0-3 are token types so the EOB run must now be 0
                s->dct_tokens[plane][i]++;
            else
                *s->dct_tokens[plane][i] = token & ~3;
            goto end;
        case 1: // zero run
            s->dct_tokens[plane][i]++;
            i += (token >> 2) & 0x7f;
            if (i > 63) {
                av_log(s->avctx, AV_LOG_ERROR, "Coefficient index overflow\n");
                return i;
            }
            block[perm[i]] = (token >> 9) * dequantizer[perm[i]];
            i++;
            break;
        case 2: // coeff
            block[perm[i]] = (token >> 2) * dequantizer[perm[i]];
            s->dct_tokens[plane][i++]++;
            break;
        default: // shouldn't happen
            return i;
        }
    } while (i < 64);
    // return value is expected to be a valid level
    i--;
end:
    // the actual DC+prediction is in the fragment structure
    block[0] = frag->dc * s->qmat[0][inter][plane][0];
    return i;
}

/**
 * called when all pixels up to row y are complete
 */
{

        int y_flipped = s->flipped_image ? s->height - y : y;

        /* At the end of the frame, report INT_MAX instead of the height of
         * the frame. This makes the other threads' ff_thread_await_progress()
         * calls cheaper, because they don't have to clip their values. */
        ff_thread_report_progress(&s->current_frame,
                                  y_flipped == s->height ? INT_MAX
                                                         : y_flipped - 1,
                                  0);
    }


    h = y - s->last_slice_end;
    s->last_slice_end = y;
    y -= h;

    if (!s->flipped_image)
        y = s->height - y - h;

    cy        = y >> s->chroma_y_shift;
    offset[0] = s->current_frame.f->linesize[0] * y;
    offset[1] = s->current_frame.f->linesize[1] * cy;
    offset[2] = s->current_frame.f->linesize[2] * cy;
    for (i = 3; i < AV_NUM_DATA_POINTERS; i++)
        offset[i] = 0;

    emms_c();
    s->avctx->draw_horiz_band(s->avctx, s->current_frame.f, offset, y, 3, h);
}

/**
 * Wait for the reference frame of the current fragment.
 * The progress value is in luma pixel rows.
 */
static void await_reference_row(Vp3DecodeContext *s, Vp3Fragment *fragment,
                                int motion_y, int y)
{
    ThreadFrame *ref_frame;
    int ref_row;
    int border = motion_y & 1;

    if (fragment->coding_method == MODE_USING_GOLDEN ||
        fragment->coding_method == MODE_GOLDEN_MV)
        ref_frame = &s->golden_frame;
    else
        ref_frame = &s->last_frame;

    ref_row = y + (motion_y >> 1);
    ref_row = FFMAX(FFABS(ref_row), ref_row + 8 + border);

    ff_thread_await_progress(ref_frame, ref_row, 0);
}

#if CONFIG_VP4_DECODER
/**
 * @return non-zero if temp (edge_emu_buffer) was populated
 */
       uint8_t * motion_source, int stride, int src_x, int src_y, uint8_t *temp)
{



#define loop_stride 12

    /* using division instead of shift to correctly handle negative values */






            return 0;


            return 0;

             loop_stride, stride,
             12, 12, src_x - 1, src_y - 1,
             plane_width,
             plane_height);



    } else {


            return 0;

             loop_stride, stride,
             12, 12, src_x - 1, src_y - 1,
             plane_width,
             plane_height);

#define safe_loop_filter(name, ptr, stride, bounding_values) \
    if ((uintptr_t)(ptr) & 7) \
        s->vp3dsp.name##_unaligned(ptr, stride, bounding_values); \
    else \
        s->vp3dsp.name(ptr, stride, bounding_values);


    }


    return 1;
}
#endif

/*
 * Perform the final rendering for a particular slice of data.
 * The slice number ranges from 0..(c_superblock_height - 1).
 */
{

        return;

                              s->data_offset[plane];
                                s->data_offset[plane];




        if (CONFIG_GRAY && plane && (s->avctx->flags & AV_CODEC_FLAG_GRAY))
            continue;

        /* for each superblock row in the slice (both of them)... */
            /* for each superblock in a row... */
                /* for each block in a superblock... */


                    // bounds check


                        s->all_fragments[i].coding_method != MODE_INTRA)
                        await_reference_row(s, &s->all_fragments[i],
                                            motion_val[fragment][1],
                                            (16 * y) >> s->chroma_y_shift);

                    /* transform if this block was coded */
                            (s->all_fragments[i].coding_method == MODE_GOLDEN_MV))
                            motion_source = golden_plane;
                        else


                        /* sort out the motion vector if this fragment is coded
                         * using a motion vector method */
                            (s->all_fragments[i].coding_method != MODE_USING_GOLDEN)) {
#if CONFIG_VP4_DECODER
                            }
#endif




#if CONFIG_VP4_DECODER
                                    motion_source = temp;
                                    standard_mc = 0;
                                }
                            }
#endif


                                                         stride, stride,
                                                         9, 9, src_x, src_y,
                                                         plane_width,
                                                         plane_height);
                            }
                        }

                        /* first, take care of copying a block from either the
                         * previous or the golden frame */
                            /* Note, it is possible to implement all MC cases
                             * with put_no_rnd_pixels_l2 which would look more
                             * like the VP3 source but this would be slower as
                             * put_no_rnd_pixels_tab is better optimized */
                                    output_plane + first_pixel,
                                    motion_source, stride, 8);
                            } else {
                                /* d is 0 if motion_x and _y have the same sign,
                                 * else -1 */
                                                               stride, 8);
                            }
                        }

                        /* invert DCT and place (or add) in final output */

                                        plane, 0, block);
                                               stride,
                                               block);
                        } else {
                                            plane, 1, block)) {
                                                   stride,
                                                   block);
                            } else {
                                                      stride, block);
                            }
                        }
                    } else {
                        /* copy directly from the previous frame */
                            output_plane + first_pixel,
                            stride, 8);
                    }
                }
            }

            // Filter up to the last row in the superblock row
        }
    }

    /* this looks like a good place for slice dispatch... */
    /* algorithm:
     *   if (slice == s->macroblock_height - 1)
     *     dispatch (both last slice & 2nd-to-last slice);
     *   else if (slice > 0)
     *     dispatch (slice - 1);
     */

                                 s->height - 16));
}

/// Allocate tables for per-frame data in Vp3DecodeContext
{



    /* superblock_coding is used by unpack_superblocks (VP3/Theora) and vp4_unpack_macroblocks (VP4) */


                                          64 * sizeof(*s->dct_tokens_base));

    /* work out the block mapping tables */


        vp3_decode_end(avctx);
        return -1;
    }


}

{

        av_frame_free(&s->current_frame.f);
        av_frame_free(&s->last_frame.f);
        av_frame_free(&s->golden_frame.f);
        return AVERROR(ENOMEM);
    }

    return 0;
}

{
#if CONFIG_VP4_DECODER
#endif

        return ret;

        s->version = 0;
    else


#define TRANSPOSE(x) (((x) >> 3) | (((x) & 7) << 3))
#undef TRANSPOSE
    }

    /* initialize to an impossible value which will force a recalculation
     * in the first frame decode */

        return ret;


    /* work out the dimensions for the C planes */




    /* fragment count covers all 8x8 blocks for all 3 planes */

        }

            }
        }

        /* init VLC tables */
                /* DC histograms */
                         &dc_bias[i][0][1], 4, 2,
                         &dc_bias[i][0][0], 4, 2, 0);

                /* group 1 AC histograms */
                         &ac_bias_0[i][0][1], 4, 2,
                         &ac_bias_0[i][0][0], 4, 2, 0);

                /* group 2 AC histograms */
                         &ac_bias_1[i][0][1], 4, 2,
                         &ac_bias_1[i][0][0], 4, 2, 0);

                /* group 3 AC histograms */
                         &ac_bias_2[i][0][1], 4, 2,
                         &ac_bias_2[i][0][0], 4, 2, 0);

                /* group 4 AC histograms */
                         &ac_bias_3[i][0][1], 4, 2,
                         &ac_bias_3[i][0][0], 4, 2, 0);
            }
#if CONFIG_VP4_DECODER
        } else { /* version >= 2 */
                /* DC histograms */
                         &vp4_dc_bias[i][0][1], 4, 2,
                         &vp4_dc_bias[i][0][0], 4, 2, 0);

                /* group 1 AC histograms */
                         &vp4_ac_bias_0[i][0][1], 4, 2,
                         &vp4_ac_bias_0[i][0][0], 4, 2, 0);

                /* group 2 AC histograms */
                         &vp4_ac_bias_1[i][0][1], 4, 2,
                         &vp4_ac_bias_1[i][0][0], 4, 2, 0);

                /* group 3 AC histograms */
                         &vp4_ac_bias_2[i][0][1], 4, 2,
                         &vp4_ac_bias_2[i][0][0], 4, 2, 0);

                /* group 4 AC histograms */
                         &vp4_ac_bias_3[i][0][1], 4, 2,
                         &vp4_ac_bias_3[i][0][0], 4, 2, 0);
            }
#endif
        }
    } else {
            /* DC histograms */
                         &s->huffman_table[i][0][1], 8, 4,
                         &s->huffman_table[i][0][0], 8, 4, 0) < 0)
                goto vlc_fail;

            /* group 1 AC histograms */
                         &s->huffman_table[i + 16][0][1], 8, 4,
                         &s->huffman_table[i + 16][0][0], 8, 4, 0) < 0)
                goto vlc_fail;

            /* group 2 AC histograms */
                         &s->huffman_table[i + 16 * 2][0][1], 8, 4,
                         &s->huffman_table[i + 16 * 2][0][0], 8, 4, 0) < 0)
                goto vlc_fail;

            /* group 3 AC histograms */
                         &s->huffman_table[i + 16 * 3][0][1], 8, 4,
                         &s->huffman_table[i + 16 * 3][0][0], 8, 4, 0) < 0)
                goto vlc_fail;

            /* group 4 AC histograms */
                         &s->huffman_table[i + 16 * 4][0][1], 8, 4,
                         &s->huffman_table[i + 16 * 4][0][0], 8, 4, 0) < 0)
                goto vlc_fail;
        }
    }

             &superblock_run_length_vlc_table[0][1], 4, 2,
             &superblock_run_length_vlc_table[0][0], 4, 2, 0);

             &fragment_run_length_vlc_table[0][1], 4, 2,
             &fragment_run_length_vlc_table[0][0], 4, 2, 0);

             &mode_code_vlc_table[0][1], 2, 1,
             &mode_code_vlc_table[0][0], 2, 1, 0);

             &motion_vector_vlc_table[0][1], 2, 1,
             &motion_vector_vlc_table[0][0], 2, 1, 0);

#if CONFIG_VP4_DECODER
                 &vp4_mv_vlc[j][i][0][1], 4, 2,
                 &vp4_mv_vlc[j][i][0][0], 4, 2, 0);

    /* version >= 2 */
             &vp4_block_pattern_vlc[i][0][1], 2, 1,
             &vp4_block_pattern_vlc[i][0][0], 2, 1, 0);
#endif


vlc_fail:
    av_log(avctx, AV_LOG_FATAL, "Invalid huffman table\n");
    return -1;
}

/// Release and shuffle frames after decode finishes
{

    /* shuffle frames (last = current) */
        goto fail;

    }

}

#if HAVE_THREADS
static int ref_frame(Vp3DecodeContext *s, ThreadFrame *dst, ThreadFrame *src)
{
    ff_thread_release_buffer(s->avctx, dst);
    if (src->f->data[0])
        return ff_thread_ref_frame(dst, src);
    return 0;
}

static int ref_frames(Vp3DecodeContext *dst, Vp3DecodeContext *src)
{
    int ret;
    if ((ret = ref_frame(dst, &dst->current_frame, &src->current_frame)) < 0 ||
        (ret = ref_frame(dst, &dst->golden_frame,  &src->golden_frame)) < 0  ||
        (ret = ref_frame(dst, &dst->last_frame,    &src->last_frame)) < 0)
        return ret;
    return 0;
}

static int vp3_update_thread_context(AVCodecContext *dst, const AVCodecContext *src)
{
    Vp3DecodeContext *s = dst->priv_data, *s1 = src->priv_data;
    int qps_changed = 0, i, err;

    if (!s1->current_frame.f->data[0] ||
        s->width != s1->width || s->height != s1->height) {
        if (s != s1)
            ref_frames(s, s1);
        return -1;
    }

    if (s != s1) {
        // copy previous frame data
        if ((err = ref_frames(s, s1)) < 0)
            return err;

        s->keyframe = s1->keyframe;

        // copy qscale data if necessary
        for (i = 0; i < 3; i++) {
            if (s->qps[i] != s1->qps[1]) {
                qps_changed = 1;
                memcpy(&s->qmat[i], &s1->qmat[i], sizeof(s->qmat[i]));
            }
        }

        if (s->qps[0] != s1->qps[0])
            memcpy(&s->bounding_values_array, &s1->bounding_values_array,
                   sizeof(s->bounding_values_array));

        if (qps_changed) {
            memcpy(s->qps,      s1->qps,      sizeof(s->qps));
            memcpy(s->last_qps, s1->last_qps, sizeof(s->last_qps));
            s->nqps = s1->nqps;
        }
    }

    return update_frames(dst);
}
#endif

                            void *data, int *got_frame,
                            AVPacket *avpkt)
{

        return ret;

#if CONFIG_THEORA_DECODER
        int type = get_bits(&gb, 7);
        skip_bits_long(&gb, 6*8); /* "theora" */

        if (s->avctx->active_thread_type&FF_THREAD_FRAME) {
            av_log(avctx, AV_LOG_ERROR, "midstream reconfiguration with multithreading is unsupported, try -threads 1\n");
            return AVERROR_PATCHWELCOME;
        }
        if (type == 0) {
            vp3_decode_end(avctx);
            ret = theora_decode_header(avctx, &gb);

            if (ret >= 0)
                ret = vp3_decode_init(avctx);
            if (ret < 0) {
                vp3_decode_end(avctx);
                return ret;
            }
            return buf_size;
        } else if (type == 2) {
            vp3_decode_end(avctx);
            ret = theora_decode_tables(avctx, &gb);
            if (ret >= 0)
                ret = vp3_decode_init(avctx);
            if (ret < 0) {
                vp3_decode_end(avctx);
                return ret;
            }
            return buf_size;
        }

        av_log(avctx, AV_LOG_ERROR,
               "Header packet passed to frame decoder, skipping\n");
        return -1;
    }
#endif

        av_log(avctx, AV_LOG_ERROR, "Data packet without prior valid headers\n");
        return -1;
    }


        av_log(s->avctx, AV_LOG_INFO, " VP3 %sframe #%d: Q index = %d\n",
               s->keyframe ? "key" : "", avctx->frame_number + 1, s->qps[0]);



        // reinit all dequantizers if the first one changed, because
        // the DC of the first quantizer must be used for all matrices

        return buf_size;

        goto error;


                           "VP version: %d\n", s->version);
            }
        }
                av_log(s->avctx, AV_LOG_ERROR,
                       "Warning, unsupported keyframe coding type?!\n");

#if CONFIG_VP4_DECODER

                    avpriv_request_sample(s->avctx, "macroblock dimension mismatch");

                    avpriv_request_sample(s->avctx, "unexpected macroblock dimension multipler/divider");

                    avpriv_request_sample(s->avctx, "unknown bits");
            }
#endif
        }
    } else {
            av_log(s->avctx, AV_LOG_WARNING,
                   "vp3: first frame not a keyframe\n");

            s->golden_frame.f->pict_type = AV_PICTURE_TYPE_I;
            if ((ret = ff_thread_get_buffer(avctx, &s->golden_frame,
                                     AV_GET_BUFFER_FLAG_REF)) < 0)
                goto error;
            ff_thread_release_buffer(avctx, &s->last_frame);
            if ((ret = ff_thread_ref_frame(&s->last_frame,
                                           &s->golden_frame)) < 0)
                goto error;
            ff_thread_report_progress(&s->last_frame, INT_MAX, 0);
        }
    }


            av_log(s->avctx, AV_LOG_ERROR, "error in unpack_superblocks\n");
            goto error;
        }
#if CONFIG_VP4_DECODER
    } else {
            av_log(s->avctx, AV_LOG_ERROR, "error in vp4_unpack_macroblocks\n");
            goto error;
    }
#endif
    }
        av_log(s->avctx, AV_LOG_ERROR, "error in unpack_modes\n");
        goto error;
    }
        av_log(s->avctx, AV_LOG_ERROR, "error in unpack_vectors\n");
        goto error;
    }
        av_log(s->avctx, AV_LOG_ERROR, "error in unpack_block_qpis\n");
        goto error;
    }

            av_log(s->avctx, AV_LOG_ERROR, "error in unpack_dct_coeffs\n");
            goto error;
        }
#if CONFIG_VP4_DECODER
    } else {
            av_log(s->avctx, AV_LOG_ERROR, "error in vp4_unpack_dct_coeffs\n");
            goto error;
        }
#endif
    }

            s->data_offset[i] = 0;
        else
    }


    // filter the last row
        }

    /* output frame, offset as needed */
        return ret;



            return ret;
    }

    return buf_size;

error:
    ff_thread_report_progress(&s->current_frame, INT_MAX, 0);

    if (!HAVE_THREADS || !(s->avctx->active_thread_type & FF_THREAD_FRAME))
        av_frame_unref(s->current_frame.f);

    return ret;
}

{

            av_log(avctx, AV_LOG_ERROR, "huffman tree overflow\n");
            return -1;
        }
                s->hti, s->hbits, token, s->entries, s->huff_code_size);
    } else {
            av_log(avctx, AV_LOG_ERROR, "huffman tree overflow\n");
            return -1;
        }
            return -1;
            return -1;
    }
    return 0;
}

#if CONFIG_THEORA_DECODER
static const enum AVPixelFormat theora_pix_fmts[4] = {
    AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE, AV_PIX_FMT_YUV422P, AV_PIX_FMT_YUV444P
};

{

        s->theora = 1;
        avpriv_request_sample(s->avctx, "theora 0");
    }

    /* 3.2.0 aka alpha3 has the same frame orientation as original vp3
     * but previous versions have the image flipped relative to vp3 */
        s->flipped_image = 1;
        av_log(avctx, AV_LOG_DEBUG,
               "Old (<alpha3) Theora bitstream, flipped image\n");
    }



    }

    /* sanity check */
        av_log(avctx, AV_LOG_ERROR,
               "Invalid frame dimensions - w:%d h:%d x:%d y:%d (%dx%d).\n",
               visible_width, visible_height, offset_x, offset_y,
               s->width, s->height);
        return AVERROR_INVALIDDATA;
    }

            av_log(avctx, AV_LOG_ERROR, "Invalid framerate\n");
            return AVERROR_INVALIDDATA;
        }
                  fps.den, fps.num, 1 << 30);
    }

                  &avctx->sample_aspect_ratio.den,
                  aspect.num, aspect.den, 1 << 30);
    }

        skip_bits(gb, 5); /* keyframe frequency force */


            av_log(avctx, AV_LOG_ERROR, "Invalid pixel format\n");
            return AVERROR_INVALIDDATA;
        }
    } else
        avctx->pix_fmt = AV_PIX_FMT_YUV420P;

        return ret;
        // translate offsets from theora axis ([0,0] lower left)
        // to normal axis ([0,0] upper left)
    }

        avctx->color_primaries = AVCOL_PRI_BT470M;
        avctx->color_primaries = AVCOL_PRI_BT470BG;

        avctx->colorspace = AVCOL_SPC_BT470BG;
        avctx->color_trc  = AVCOL_TRC_BT709;
    }

}

{

        return AVERROR_INVALIDDATA;

        /* loop filter limit values table */
    }

    else
        n = 16;
    /* quality threshold table */

    else
        n = 16;
    /* dc scale factor table */

    else
        matrices = 3;

        av_log(avctx, AV_LOG_ERROR, "invalid number of base matrixes\n");
        return -1;
    }


                    qtj = 0;
                    plj = plane;
                } else {
                }
                       sizeof(s->qr_size[0][0]));
                       sizeof(s->qr_base[0][0]));
            } else {
                int qri = 0;
                int qi  = 0;

                        av_log(avctx, AV_LOG_ERROR,
                               "invalid base matrix index\n");
                        return -1;
                    }
                        break;
                }

                    av_log(avctx, AV_LOG_ERROR, "invalid qi %d > 63\n", qi);
                    return -1;
                }
            }
        }
    }

    /* Huffman tables */
                return -1;
                return -1;
        }
    }


}

{



        av_log(avctx, AV_LOG_ERROR, "Missing extradata!\n");
        return -1;
    }

                                  42, header_start, header_len) < 0) {
        av_log(avctx, AV_LOG_ERROR, "Corrupt extradata\n");
        return -1;
    }

            continue;
            return ret;


            av_log(avctx, AV_LOG_ERROR, "Invalid extradata!\n");
//          return -1;
        }

        // FIXME: Check for this as well.

                return -1;
            break;
        case 0x81:
// FIXME: is this needed? it breaks sometimes
//            theora_decode_comments(avctx, gb);
            break;
                return -1;
            break;
        default:
            av_log(avctx, AV_LOG_ERROR,
                   "Unknown Theora config packet: %d\n", ptype & ~0x80);
            break;
        }
                   "%d bits left in packet %X\n",
            break;
    }

}

AVCodec ff_theora_decoder = {
    .name                  = "theora",
    .long_name             = NULL_IF_CONFIG_SMALL("Theora"),
    .type                  = AVMEDIA_TYPE_VIDEO,
    .id                    = AV_CODEC_ID_THEORA,
    .priv_data_size        = sizeof(Vp3DecodeContext),
    .init                  = theora_decode_init,
    .close                 = vp3_decode_end,
    .decode                = vp3_decode_frame,
    .capabilities          = AV_CODEC_CAP_DR1 | AV_CODEC_CAP_DRAW_HORIZ_BAND |
                             AV_CODEC_CAP_FRAME_THREADS,
    .flush                 = vp3_decode_flush,
    .update_thread_context = ONLY_IF_THREADS_ENABLED(vp3_update_thread_context),
    .caps_internal         = FF_CODEC_CAP_EXPORTS_CROPPING | FF_CODEC_CAP_ALLOCATE_PROGRESS,
};
#endif

AVCodec ff_vp3_decoder = {
    .name                  = "vp3",
    .long_name             = NULL_IF_CONFIG_SMALL("On2 VP3"),
    .type                  = AVMEDIA_TYPE_VIDEO,
    .id                    = AV_CODEC_ID_VP3,
    .priv_data_size        = sizeof(Vp3DecodeContext),
    .init                  = vp3_decode_init,
    .close                 = vp3_decode_end,
    .decode                = vp3_decode_frame,
    .capabilities          = AV_CODEC_CAP_DR1 | AV_CODEC_CAP_DRAW_HORIZ_BAND |
                             AV_CODEC_CAP_FRAME_THREADS,
    .flush                 = vp3_decode_flush,
    .update_thread_context = ONLY_IF_THREADS_ENABLED(vp3_update_thread_context),
    .caps_internal         = FF_CODEC_CAP_ALLOCATE_PROGRESS,
};

#if CONFIG_VP4_DECODER
AVCodec ff_vp4_decoder = {
    .name                  = "vp4",
    .long_name             = NULL_IF_CONFIG_SMALL("On2 VP4"),
    .type                  = AVMEDIA_TYPE_VIDEO,
    .id                    = AV_CODEC_ID_VP4,
    .priv_data_size        = sizeof(Vp3DecodeContext),
    .init                  = vp3_decode_init,
    .close                 = vp3_decode_end,
    .decode                = vp3_decode_frame,
    .capabilities          = AV_CODEC_CAP_DR1 | AV_CODEC_CAP_DRAW_HORIZ_BAND |
                             AV_CODEC_CAP_FRAME_THREADS,
    .flush                 = vp3_decode_flush,
    .update_thread_context = ONLY_IF_THREADS_ENABLED(vp3_update_thread_context),
    .caps_internal         = FF_CODEC_CAP_ALLOCATE_PROGRESS,
};
#endif
