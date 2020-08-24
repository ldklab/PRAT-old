/*
 * H.26L/H.264/AVC/JVT/14496-10/... cavlc bitstream decoding
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
 * H.264 / AVC / MPEG-4 part10 cavlc bitstream decoding.
 * @author Michael Niedermayer <michaelni@gmx.at>
 */

#define CABAC(h) 0
#define UNCHECKED_BITSTREAM_READER 1

#include "internal.h"
#include "avcodec.h"
#include "h264dec.h"
#include "h264_mvpred.h"
#include "h264data.h"
#include "golomb.h"
#include "mpegutils.h"
#include "libavutil/avassert.h"


static const uint8_t golomb_to_inter_cbp_gray[16]={
 0, 1, 2, 4, 8, 3, 5,10,12,15, 7,11,13,14, 6, 9,
};

static const uint8_t golomb_to_intra4x4_cbp_gray[16]={
15, 0, 7,11,13,14, 3, 5,10,12, 1, 2, 4, 8, 6, 9,
};

static const uint8_t chroma_dc_coeff_token_len[4*5]={
 2, 0, 0, 0,
 6, 1, 0, 0,
 6, 6, 3, 0,
 6, 7, 7, 6,
 6, 8, 8, 7,
};

static const uint8_t chroma_dc_coeff_token_bits[4*5]={
 1, 0, 0, 0,
 7, 1, 0, 0,
 4, 6, 1, 0,
 3, 3, 2, 5,
 2, 3, 2, 0,
};

static const uint8_t chroma422_dc_coeff_token_len[4*9]={
  1,  0,  0,  0,
  7,  2,  0,  0,
  7,  7,  3,  0,
  9,  7,  7,  5,
  9,  9,  7,  6,
 10, 10,  9,  7,
 11, 11, 10,  7,
 12, 12, 11, 10,
 13, 12, 12, 11,
};

static const uint8_t chroma422_dc_coeff_token_bits[4*9]={
  1,   0,  0, 0,
 15,   1,  0, 0,
 14,  13,  1, 0,
  7,  12, 11, 1,
  6,   5, 10, 1,
  7,   6,  4, 9,
  7,   6,  5, 8,
  7,   6,  5, 4,
  7,   5,  4, 4,
};

static const uint8_t coeff_token_len[4][4*17]={
{
     1, 0, 0, 0,
     6, 2, 0, 0,     8, 6, 3, 0,     9, 8, 7, 5,    10, 9, 8, 6,
    11,10, 9, 7,    13,11,10, 8,    13,13,11, 9,    13,13,13,10,
    14,14,13,11,    14,14,14,13,    15,15,14,14,    15,15,15,14,
    16,15,15,15,    16,16,16,15,    16,16,16,16,    16,16,16,16,
},
{
     2, 0, 0, 0,
     6, 2, 0, 0,     6, 5, 3, 0,     7, 6, 6, 4,     8, 6, 6, 4,
     8, 7, 7, 5,     9, 8, 8, 6,    11, 9, 9, 6,    11,11,11, 7,
    12,11,11, 9,    12,12,12,11,    12,12,12,11,    13,13,13,12,
    13,13,13,13,    13,14,13,13,    14,14,14,13,    14,14,14,14,
},
{
     4, 0, 0, 0,
     6, 4, 0, 0,     6, 5, 4, 0,     6, 5, 5, 4,     7, 5, 5, 4,
     7, 5, 5, 4,     7, 6, 6, 4,     7, 6, 6, 4,     8, 7, 7, 5,
     8, 8, 7, 6,     9, 8, 8, 7,     9, 9, 8, 8,     9, 9, 9, 8,
    10, 9, 9, 9,    10,10,10,10,    10,10,10,10,    10,10,10,10,
},
{
     6, 0, 0, 0,
     6, 6, 0, 0,     6, 6, 6, 0,     6, 6, 6, 6,     6, 6, 6, 6,
     6, 6, 6, 6,     6, 6, 6, 6,     6, 6, 6, 6,     6, 6, 6, 6,
     6, 6, 6, 6,     6, 6, 6, 6,     6, 6, 6, 6,     6, 6, 6, 6,
     6, 6, 6, 6,     6, 6, 6, 6,     6, 6, 6, 6,     6, 6, 6, 6,
}
};

static const uint8_t coeff_token_bits[4][4*17]={
{
     1, 0, 0, 0,
     5, 1, 0, 0,     7, 4, 1, 0,     7, 6, 5, 3,     7, 6, 5, 3,
     7, 6, 5, 4,    15, 6, 5, 4,    11,14, 5, 4,     8,10,13, 4,
    15,14, 9, 4,    11,10,13,12,    15,14, 9,12,    11,10,13, 8,
    15, 1, 9,12,    11,14,13, 8,     7,10, 9,12,     4, 6, 5, 8,
},
{
     3, 0, 0, 0,
    11, 2, 0, 0,     7, 7, 3, 0,     7,10, 9, 5,     7, 6, 5, 4,
     4, 6, 5, 6,     7, 6, 5, 8,    15, 6, 5, 4,    11,14,13, 4,
    15,10, 9, 4,    11,14,13,12,     8,10, 9, 8,    15,14,13,12,
    11,10, 9,12,     7,11, 6, 8,     9, 8,10, 1,     7, 6, 5, 4,
},
{
    15, 0, 0, 0,
    15,14, 0, 0,    11,15,13, 0,     8,12,14,12,    15,10,11,11,
    11, 8, 9,10,     9,14,13, 9,     8,10, 9, 8,    15,14,13,13,
    11,14,10,12,    15,10,13,12,    11,14, 9,12,     8,10,13, 8,
    13, 7, 9,12,     9,12,11,10,     5, 8, 7, 6,     1, 4, 3, 2,
},
{
     3, 0, 0, 0,
     0, 1, 0, 0,     4, 5, 6, 0,     8, 9,10,11,    12,13,14,15,
    16,17,18,19,    20,21,22,23,    24,25,26,27,    28,29,30,31,
    32,33,34,35,    36,37,38,39,    40,41,42,43,    44,45,46,47,
    48,49,50,51,    52,53,54,55,    56,57,58,59,    60,61,62,63,
}
};

static const uint8_t total_zeros_len[16][16]= {
    {1,3,3,4,4,5,5,6,6,7,7,8,8,9,9,9},
    {3,3,3,3,3,4,4,4,4,5,5,6,6,6,6},
    {4,3,3,3,4,4,3,3,4,5,5,6,5,6},
    {5,3,4,4,3,3,3,4,3,4,5,5,5},
    {4,4,4,3,3,3,3,3,4,5,4,5},
    {6,5,3,3,3,3,3,3,4,3,6},
    {6,5,3,3,3,2,3,4,3,6},
    {6,4,5,3,2,2,3,3,6},
    {6,6,4,2,2,3,2,5},
    {5,5,3,2,2,2,4},
    {4,4,3,3,1,3},
    {4,4,2,1,3},
    {3,3,1,2},
    {2,2,1},
    {1,1},
};

static const uint8_t total_zeros_bits[16][16]= {
    {1,3,2,3,2,3,2,3,2,3,2,3,2,3,2,1},
    {7,6,5,4,3,5,4,3,2,3,2,3,2,1,0},
    {5,7,6,5,4,3,4,3,2,3,2,1,1,0},
    {3,7,5,4,6,5,4,3,3,2,2,1,0},
    {5,4,3,7,6,5,4,3,2,1,1,0},
    {1,1,7,6,5,4,3,2,1,1,0},
    {1,1,5,4,3,3,2,1,1,0},
    {1,1,1,3,3,2,2,1,0},
    {1,0,1,3,2,1,1,1},
    {1,0,1,3,2,1,1},
    {0,1,1,2,1,3},
    {0,1,1,1,1},
    {0,1,1,1},
    {0,1,1},
    {0,1},
};

static const uint8_t chroma_dc_total_zeros_len[3][4]= {
    { 1, 2, 3, 3,},
    { 1, 2, 2, 0,},
    { 1, 1, 0, 0,},
};

static const uint8_t chroma_dc_total_zeros_bits[3][4]= {
    { 1, 1, 1, 0,},
    { 1, 1, 0, 0,},
    { 1, 0, 0, 0,},
};

static const uint8_t chroma422_dc_total_zeros_len[7][8]= {
    { 1, 3, 3, 4, 4, 4, 5, 5 },
    { 3, 2, 3, 3, 3, 3, 3 },
    { 3, 3, 2, 2, 3, 3 },
    { 3, 2, 2, 2, 3 },
    { 2, 2, 2, 2 },
    { 2, 2, 1 },
    { 1, 1 },
};

static const uint8_t chroma422_dc_total_zeros_bits[7][8]= {
    { 1, 2, 3, 2, 3, 1, 1, 0 },
    { 0, 1, 1, 4, 5, 6, 7 },
    { 0, 1, 1, 2, 6, 7 },
    { 6, 0, 1, 2, 7 },
    { 0, 1, 2, 3 },
    { 0, 1, 1 },
    { 0, 1 },
};

static const uint8_t run_len[7][16]={
    {1,1},
    {1,2,2},
    {2,2,2,2},
    {2,2,2,3,3},
    {2,2,3,3,3,3},
    {2,3,3,3,3,3,3},
    {3,3,3,3,3,3,3,4,5,6,7,8,9,10,11},
};

static const uint8_t run_bits[7][16]={
    {1,0},
    {1,1,0},
    {3,2,1,0},
    {3,2,1,1,0},
    {3,2,3,2,1,0},
    {3,0,1,3,2,5,4},
    {7,6,5,4,3,2,1,1,1,1,1,1,1,1,1},
};

static VLC coeff_token_vlc[4];
static VLC_TYPE coeff_token_vlc_tables[520+332+280+256][2];
static const int coeff_token_vlc_tables_size[4]={520,332,280,256};

static VLC chroma_dc_coeff_token_vlc;
static VLC_TYPE chroma_dc_coeff_token_vlc_table[256][2];
static const int chroma_dc_coeff_token_vlc_table_size = 256;

static VLC chroma422_dc_coeff_token_vlc;
static VLC_TYPE chroma422_dc_coeff_token_vlc_table[8192][2];
static const int chroma422_dc_coeff_token_vlc_table_size = 8192;

static VLC total_zeros_vlc[15+1];
static VLC_TYPE total_zeros_vlc_tables[15][512][2];
static const int total_zeros_vlc_tables_size = 512;

static VLC chroma_dc_total_zeros_vlc[3+1];
static VLC_TYPE chroma_dc_total_zeros_vlc_tables[3][8][2];
static const int chroma_dc_total_zeros_vlc_tables_size = 8;

static VLC chroma422_dc_total_zeros_vlc[7+1];
static VLC_TYPE chroma422_dc_total_zeros_vlc_tables[7][32][2];
static const int chroma422_dc_total_zeros_vlc_tables_size = 32;

static VLC run_vlc[6+1];
static VLC_TYPE run_vlc_tables[6][8][2];
static const int run_vlc_tables_size = 8;

static VLC run7_vlc;
static VLC_TYPE run7_vlc_table[96][2];
static const int run7_vlc_table_size = 96;

#define LEVEL_TAB_BITS 8
static int8_t cavlc_level_tab[7][1<<LEVEL_TAB_BITS][2];

#define CHROMA_DC_COEFF_TOKEN_VLC_BITS 8
#define CHROMA422_DC_COEFF_TOKEN_VLC_BITS 13
#define COEFF_TOKEN_VLC_BITS           8
#define TOTAL_ZEROS_VLC_BITS           9
#define CHROMA_DC_TOTAL_ZEROS_VLC_BITS 3
#define CHROMA422_DC_TOTAL_ZEROS_VLC_BITS 5
#define RUN_VLC_BITS                   3
#define RUN7_VLC_BITS                  6

/**
 * Get the predicted number of non-zero coefficients.
 * @param n block index
 */
static inline int pred_non_zero_count(const H264Context *h, H264SliceContext *sl, int n)
{
    const int index8= scan8[n];
    const int left = sl->non_zero_count_cache[index8 - 1];
    const int top  = sl->non_zero_count_cache[index8 - 8];
    int i= left + top;

    if(i<64) i= (i+1)>>1;

    ff_tlog(h->avctx, "pred_nnz L%X T%X n%d s%d P%X\n", left, top, n, scan8[n], i&31);

    return i&31;
}



            }else{
            }
        }
    }



                 &chroma_dc_coeff_token_len [0], 1, 1,
                 &chroma_dc_coeff_token_bits[0], 1, 1,
                 INIT_VLC_USE_NEW_STATIC);

                 &chroma422_dc_coeff_token_len [0], 1, 1,
                 &chroma422_dc_coeff_token_bits[0], 1, 1,
                 INIT_VLC_USE_NEW_STATIC);

                     &coeff_token_len [i][0], 1, 1,
                     &coeff_token_bits[i][0], 1, 1,
                     INIT_VLC_USE_NEW_STATIC);
        }
        /*
         * This is a one time safety check to make sure that
         * the packed static coeff_token_vlc table sizes
         * were initialized correctly.
         */

                     CHROMA_DC_TOTAL_ZEROS_VLC_BITS, 4,
                     &chroma_dc_total_zeros_len [i][0], 1, 1,
                     &chroma_dc_total_zeros_bits[i][0], 1, 1,
                     INIT_VLC_USE_NEW_STATIC);
        }

                     CHROMA422_DC_TOTAL_ZEROS_VLC_BITS, 8,
                     &chroma422_dc_total_zeros_len [i][0], 1, 1,
                     &chroma422_dc_total_zeros_bits[i][0], 1, 1,
                     INIT_VLC_USE_NEW_STATIC);
        }

                     TOTAL_ZEROS_VLC_BITS, 16,
                     &total_zeros_len [i][0], 1, 1,
                     &total_zeros_bits[i][0], 1, 1,
                     INIT_VLC_USE_NEW_STATIC);
        }

                     RUN_VLC_BITS, 7,
                     &run_len [i][0], 1, 1,
                     &run_bits[i][0], 1, 1,
                     INIT_VLC_USE_NEW_STATIC);
        }
                 &run_len [6][0], 1, 1,
                 &run_bits[6][0], 1, 1,
                 INIT_VLC_USE_NEW_STATIC);

    }

static inline int get_level_prefix(GetBitContext *gb){
    unsigned int buf;
    int log;

    OPEN_READER(re, gb);
    UPDATE_CACHE(re, gb);
    buf=GET_CACHE(re, gb);

    log= 32 - av_log2(buf);

    LAST_SKIP_BITS(re, gb, log);
    CLOSE_READER(re, gb);

    return log-1;
}

/**
 * Decode a residual block.
 * @param n block index
 * @param scantable scantable
 * @param max_coeff number of coefficients in the block
 * @return <0 if an error occurred
 */
static int decode_residual(const H264Context *h, H264SliceContext *sl,
                           GetBitContext *gb, int16_t *block, int n,
                           const uint8_t *scantable, const uint32_t *qmul,
                           int max_coeff)
{
    static const int coeff_token_table_index[17]= {0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3};
    int level[16];
    int zeros_left, coeff_token, total_coeff, i, trailing_ones, run_before;

    //FIXME put trailing_onex into the context

    if(max_coeff <= 8){
        if (max_coeff == 4)
            coeff_token = get_vlc2(gb, chroma_dc_coeff_token_vlc.table, CHROMA_DC_COEFF_TOKEN_VLC_BITS, 1);
        else
            coeff_token = get_vlc2(gb, chroma422_dc_coeff_token_vlc.table, CHROMA422_DC_COEFF_TOKEN_VLC_BITS, 1);
        total_coeff= coeff_token>>2;
    }else{
        if(n >= LUMA_DC_BLOCK_INDEX){
            total_coeff= pred_non_zero_count(h, sl, (n - LUMA_DC_BLOCK_INDEX)*16);
            coeff_token= get_vlc2(gb, coeff_token_vlc[ coeff_token_table_index[total_coeff] ].table, COEFF_TOKEN_VLC_BITS, 2);
            total_coeff= coeff_token>>2;
        }else{
            total_coeff= pred_non_zero_count(h, sl, n);
            coeff_token= get_vlc2(gb, coeff_token_vlc[ coeff_token_table_index[total_coeff] ].table, COEFF_TOKEN_VLC_BITS, 2);
            total_coeff= coeff_token>>2;
        }
    }
    sl->non_zero_count_cache[scan8[n]] = total_coeff;

    //FIXME set last_non_zero?

    if(total_coeff==0)
        return 0;
    if(total_coeff > (unsigned)max_coeff) {
        av_log(h->avctx, AV_LOG_ERROR, "corrupted macroblock %d %d (total_coeff=%d)\n", sl->mb_x, sl->mb_y, total_coeff);
        return -1;
    }

    trailing_ones= coeff_token&3;
    ff_tlog(h->avctx, "trailing:%d, total:%d\n", trailing_ones, total_coeff);
    av_assert2(total_coeff<=16);

    i = show_bits(gb, 3);
    skip_bits(gb, trailing_ones);
    level[0] = 1-((i&4)>>1);
    level[1] = 1-((i&2)   );
    level[2] = 1-((i&1)<<1);

    if(trailing_ones<total_coeff) {
        int mask, prefix;
        int suffix_length = total_coeff > 10 & trailing_ones < 3;
        int bitsi= show_bits(gb, LEVEL_TAB_BITS);
        int level_code= cavlc_level_tab[suffix_length][bitsi][0];

        skip_bits(gb, cavlc_level_tab[suffix_length][bitsi][1]);
        if(level_code >= 100){
            prefix= level_code - 100;
            if(prefix == LEVEL_TAB_BITS)
                prefix += get_level_prefix(gb);

            //first coefficient has suffix_length equal to 0 or 1
            if(prefix<14){ //FIXME try to build a large unified VLC table for all this
                if(suffix_length)
                    level_code= (prefix<<1) + get_bits1(gb); //part
                else
                    level_code= prefix; //part
            }else if(prefix==14){
                if(suffix_length)
                    level_code= (prefix<<1) + get_bits1(gb); //part
                else
                    level_code= prefix + get_bits(gb, 4); //part
            }else{
                level_code= 30;
                if(prefix>=16){
                    if(prefix > 25+3){
                        av_log(h->avctx, AV_LOG_ERROR, "Invalid level prefix\n");
                        return -1;
                    }
                    level_code += (1<<(prefix-3))-4096;
                }
                level_code += get_bits(gb, prefix-3); //part
            }

            if(trailing_ones < 3) level_code += 2;

            suffix_length = 2;
            mask= -(level_code&1);
            level[trailing_ones]= (((2+level_code)>>1) ^ mask) - mask;
        }else{
            level_code += ((level_code>>31)|1) & -(trailing_ones < 3);

            suffix_length = 1 + (level_code + 3U > 6U);
            level[trailing_ones]= level_code;
        }

        //remaining coefficients have suffix_length > 0
        for(i=trailing_ones+1;i<total_coeff;i++) {
            static const unsigned int suffix_limit[7] = {0,3,6,12,24,48,INT_MAX };
            int bitsi= show_bits(gb, LEVEL_TAB_BITS);
            level_code= cavlc_level_tab[suffix_length][bitsi][0];

            skip_bits(gb, cavlc_level_tab[suffix_length][bitsi][1]);
            if(level_code >= 100){
                prefix= level_code - 100;
                if(prefix == LEVEL_TAB_BITS){
                    prefix += get_level_prefix(gb);
                }
                if(prefix<15){
                    level_code = (prefix<<suffix_length) + get_bits(gb, suffix_length);
                }else{
                    level_code = 15<<suffix_length;
                    if (prefix>=16) {
                        if(prefix > 25+3){
                            av_log(h->avctx, AV_LOG_ERROR, "Invalid level prefix\n");
                            return AVERROR_INVALIDDATA;
                        }
                        level_code += (1<<(prefix-3))-4096;
                    }
                    level_code += get_bits(gb, prefix-3);
                }
                mask= -(level_code&1);
                level_code= (((2+level_code)>>1) ^ mask) - mask;
            }
            level[i]= level_code;
            suffix_length+= suffix_limit[suffix_length] + level_code > 2U*suffix_limit[suffix_length];
        }
    }

    if(total_coeff == max_coeff)
        zeros_left=0;
    else{
        if (max_coeff <= 8) {
            if (max_coeff == 4)
                zeros_left = get_vlc2(gb, chroma_dc_total_zeros_vlc[total_coeff].table,
                                      CHROMA_DC_TOTAL_ZEROS_VLC_BITS, 1);
            else
                zeros_left = get_vlc2(gb, chroma422_dc_total_zeros_vlc[total_coeff].table,
                                      CHROMA422_DC_TOTAL_ZEROS_VLC_BITS, 1);
        } else {
            zeros_left= get_vlc2(gb, total_zeros_vlc[ total_coeff ].table, TOTAL_ZEROS_VLC_BITS, 1);
        }
    }

#define STORE_BLOCK(type) \
    scantable += zeros_left + total_coeff - 1; \
    if(n >= LUMA_DC_BLOCK_INDEX){ \
        ((type*)block)[*scantable] = level[0]; \
        for(i=1;i<total_coeff && zeros_left > 0;i++) { \
            if(zeros_left < 7) \
                run_before= get_vlc2(gb, run_vlc[zeros_left].table, RUN_VLC_BITS, 1); \
            else \
                run_before= get_vlc2(gb, run7_vlc.table, RUN7_VLC_BITS, 2); \
            zeros_left -= run_before; \
            scantable -= 1 + run_before; \
            ((type*)block)[*scantable]= level[i]; \
        } \
        for(;i<total_coeff;i++) { \
            scantable--; \
            ((type*)block)[*scantable]= level[i]; \
        } \
    }else{ \
        ((type*)block)[*scantable] = ((int)(level[0] * qmul[*scantable] + 32))>>6; \
        for(i=1;i<total_coeff && zeros_left > 0;i++) { \
            if(zeros_left < 7) \
                run_before= get_vlc2(gb, run_vlc[zeros_left].table, RUN_VLC_BITS, 1); \
            else \
                run_before= get_vlc2(gb, run7_vlc.table, RUN7_VLC_BITS, 2); \
            zeros_left -= run_before; \
            scantable -= 1 + run_before; \
            ((type*)block)[*scantable]= ((int)(level[i] * qmul[*scantable] + 32))>>6; \
        } \
        for(;i<total_coeff;i++) { \
            scantable--; \
            ((type*)block)[*scantable]= ((int)(level[i] * qmul[*scantable] + 32))>>6; \
        } \
    }

    if (h->pixel_shift) {
        STORE_BLOCK(int32_t)
    } else {
        STORE_BLOCK(int16_t)
    }

    if(zeros_left<0){
        av_log(h->avctx, AV_LOG_ERROR, "negative number of zero coeffs at %d %d\n", sl->mb_x, sl->mb_y);
        return -1;
    }

    return 0;
}

static av_always_inline
                         GetBitContext *gb, const uint8_t *scan,
                         const uint8_t *scan8x8, int pixel_shift,
                         int mb_type, int cbp, int p)
{
            return -1; //FIXME continue if partitioned and other return -1 too
        }


                        return -1;
                    }
                }
            }
            return 0xf;
        }else{
        }
    }else{
        /* For CAVLC 4:4:4, we need to keep track of the luma 8x8 CBP for deblocking nnz purposes. */
                            return -1;
                    }
                }else{
                            return -1;
                        }
                    }
                }
            }else{
            }
        }
        return new_cbp;
    }
}

{


                down the code */
                av_log(h->avctx, AV_LOG_ERROR, "mb_skip_run %d is invalid\n", mb_skip_run);
                return AVERROR_INVALIDDATA;
            }
        }

            }
        }
    }
    }


        }else{
        }
        }else{
        }
    }else{
            mb_type--;
            av_log(h->avctx, AV_LOG_ERROR, "mb_type %d in %c slice too large at %d %d\n", mb_type, av_get_picture_type_char(sl->slice_type), sl->mb_x, sl->mb_y);
            return -1;
        }
    }




        // We assume these blocks are very rare so we do not optimize it.
            av_log(h->avctx, AV_LOG_ERROR, "Not enough data for an intra PCM block.\n");
            return AVERROR_INVALIDDATA;
        }

        // In deblocking, the quantizer is 0
        // All coeffs are present

    }


    //mb_pred
//            init_top_left_availability(h);
            }

//                fill_intra4x4_pred_table(h);

                }

                else
            }
                return -1;
        }else{
                return -1;
        }
                return -1;
        } else {
        }

                    av_log(h->avctx, AV_LOG_ERROR, "B sub_mb_type %u out of range at %d %d\n", sl->sub_mb_type[i], sl->mb_x, sl->mb_y);
                    return -1;
                }
            }
            }
        }else{
            av_assert2(sl->slice_type_nos == AV_PICTURE_TYPE_P); //FIXME SP correct ?
                    av_log(h->avctx, AV_LOG_ERROR, "P sub_mb_type %u out of range at %d %d\n", sl->sub_mb_type[i], sl->mb_x, sl->mb_y);
                    return -1;
                }
            }
        }

                        tmp= 0;
                    }else{
                            av_log(h->avctx, AV_LOG_ERROR, "ref %u overflow\n", tmp);
                            return -1;
                        }
                    }
                }else{
                 //FIXME
                }
            }
        }


                }


                        }
                    }
                }else{
                }
            }
        }
    }else{
         //FIXME we should set ref_idx_l? to 0 if we use that later ...
                            val= 0;
                        }else{
                                av_log(h->avctx, AV_LOG_ERROR, "ref %u overflow\n", val);
                                return -1;
                            }
                        }
                    }
            }

                }
            }
        }
                                val= 0;
                            }else{
                                    av_log(h->avctx, AV_LOG_ERROR, "ref %u overflow\n", val);
                                    return -1;
                                }
                            }
                        }else
                            val= LIST_NOT_USED&0xFF;
                    }
            }

                    }else
                        val=0;
                }
            }
        }else{
            av_assert2(IS_8X16(mb_type));
                                val= 0;
                            }else{
                                    av_log(h->avctx, AV_LOG_ERROR, "ref %u overflow\n", val);
                                    return -1;
                                }
                            }
                        }else
                            val= LIST_NOT_USED&0xFF;
                    }
            }

                    }else
                        val=0;
                }
            }
        }
    }



                av_log(h->avctx, AV_LOG_ERROR, "cbp too large (%u) at %d %d\n", cbp, sl->mb_x, sl->mb_y);
                return -1;
            }
            else
        }else{
                av_log(h->avctx, AV_LOG_ERROR, "cbp too large (%u) at %d %d\n", cbp, sl->mb_x, sl->mb_y);
                return -1;
            }
        }
    } else {
            av_log(h->avctx, AV_LOG_ERROR, "gray chroma\n");
            return AVERROR_INVALIDDATA;
        }
    }

    }




                av_log(h->avctx, AV_LOG_ERROR, "dquant out of range (%d) at %d %d\n", dquant, sl->mb_x, sl->mb_y);
                sl->qscale = max_qp;
                return -1;
            }
        }


        }else{
        }

            return -1;
        }
            if (decode_luma_residual(h, sl, gb, scan, scan8x8, pixel_shift, mb_type, cbp, 1) < 0 ) {
                return -1;
            }
            if (decode_luma_residual(h, sl, gb, scan, scan8x8, pixel_shift, mb_type, cbp, 2) < 0 ) {
                return -1;
            }
        } else {

                                        CHROMA_DC_BLOCK_INDEX + chroma_idx,
                                        NULL, 4 * num_c8x8) < 0) {
                        return -1;
                    }
            }

                                return -1;
                        }
                    }
                }
            }else{
            }
        }
    }else{
    }

    return 0;
}
