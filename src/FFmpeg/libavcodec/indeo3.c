/*
 * Indeo Video v3 compatible decoder
 * Copyright (c) 2009 - 2011 Maxim Poliakovski
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
 * This is a decoder for Intel Indeo Video v3.
 * It is based on vector quantization, run-length coding and motion compensation.
 * Known container formats: .avi and .mov
 * Known FOURCCs: 'IV31', 'IV32'
 *
 * @see http://wiki.multimedia.cx/index.php?title=Indeo_3
 */

#include "libavutil/imgutils.h"
#include "libavutil/intreadwrite.h"
#include "avcodec.h"
#include "copy_block.h"
#include "bytestream.h"
#include "get_bits.h"
#include "hpeldsp.h"
#include "internal.h"

#include "indeo3data.h"

/* RLE opcodes. */
enum {
    RLE_ESC_F9    = 249, ///< same as RLE_ESC_FA + do the same with next block
    RLE_ESC_FA    = 250, ///< INTRA: skip block, INTER: copy data from reference
    RLE_ESC_FB    = 251, ///< apply null delta to N blocks / skip N blocks
    RLE_ESC_FC    = 252, ///< same as RLE_ESC_FD + do the same with next block
    RLE_ESC_FD    = 253, ///< apply null delta to all remaining lines of this block
    RLE_ESC_FE    = 254, ///< apply null delta to all lines up to the 3rd line
    RLE_ESC_FF    = 255  ///< apply null delta to all lines up to the 2nd line
};


/* Some constants for parsing frame bitstream flags. */
#define BS_8BIT_PEL     (1 << 1) ///< 8-bit pixel bitdepth indicator
#define BS_KEYFRAME     (1 << 2) ///< intra frame indicator
#define BS_MV_Y_HALF    (1 << 4) ///< vertical mv halfpel resolution indicator
#define BS_MV_X_HALF    (1 << 5) ///< horizontal mv halfpel resolution indicator
#define BS_NONREF       (1 << 8) ///< nonref (discardable) frame indicator
#define BS_BUFFER        9       ///< indicates which of two frame buffers should be used


typedef struct Plane {
    uint8_t         *buffers[2];
    uint8_t         *pixels[2]; ///< pointer to the actual pixel data of the buffers above
    uint32_t        width;
    uint32_t        height;
    ptrdiff_t       pitch;
} Plane;

#define CELL_STACK_MAX  20

typedef struct Cell {
    int16_t         xpos;       ///< cell coordinates in 4x4 blocks
    int16_t         ypos;
    int16_t         width;      ///< cell width  in 4x4 blocks
    int16_t         height;     ///< cell height in 4x4 blocks
    uint8_t         tree;       ///< tree id: 0- MC tree, 1 - VQ tree
    const int8_t    *mv_ptr;    ///< ptr to the motion vector if any
} Cell;

typedef struct Indeo3DecodeContext {
    AVCodecContext *avctx;
    HpelDSPContext  hdsp;

    GetBitContext   gb;
    int             need_resync;
    int             skip_bits;
    const uint8_t   *next_cell_data;
    const uint8_t   *last_byte;
    const int8_t    *mc_vectors;
    unsigned        num_vectors;    ///< number of motion vectors in mc_vectors

    int16_t         width, height;
    uint32_t        frame_num;      ///< current frame number (zero-based)
    int             data_size;      ///< size of the frame data in bytes
    uint16_t        frame_flags;    ///< frame properties
    uint8_t         cb_offset;      ///< needed for selecting VQ tables
    uint8_t         buf_sel;        ///< active frame buffer: 0 - primary, 1 -secondary
    const uint8_t   *y_data_ptr;
    const uint8_t   *v_data_ptr;
    const uint8_t   *u_data_ptr;
    int32_t         y_data_size;
    int32_t         v_data_size;
    int32_t         u_data_size;
    const uint8_t   *alt_quant;     ///< secondary VQ table set for the modes 1 and 4
    Plane           planes[3];
} Indeo3DecodeContext;


static uint8_t requant_tab[8][128];

/*
 *  Build the static requantization table.
 *  This table is used to remap pixel values according to a specific
 *  quant index and thus avoid overflows while adding deltas.
 */
{


    }

    /* some last elements calculated above will have values >= 128 */
    /* pixel values shall never exceed 127 so set them to non-overflowing values */
    /* according with the quantization step of the respective section */

    /* Patch for compatibility with the Intel's binary decoders */


{


    }


                                          AVCodecContext *avctx, int luma_width, int luma_height)
{

        av_log(avctx, AV_LOG_ERROR, "Invalid picture dimensions: %d x %d!\n",
               luma_width, luma_height);
        return AVERROR_INVALIDDATA;
    }




    /* Calculate size of the luminance plane.  */
    /* Add one line more for INTRA prediction. */

    /* Calculate size of a chrominance planes. */
    /* Add one line more for INTRA prediction. */

    /* allocate frame buffers */


            free_frame_buffers(ctx);
            return AVERROR(ENOMEM);
        }

        /* fill the INTRA prediction lines with the middle pixel value = 64 */

        /* set buffer pointers = buf_ptr + pitch and thus skip the INTRA prediction line */
    }

    return 0;
}

/**
 *  Copy pixels of the cell(x + mv_x, y + mv_y) from the previous frame into
 *  the cell(x, y) in the current frame.
 *
 *  @param ctx      pointer to the decoder context
 *  @param plane    pointer to the plane descriptor
 *  @param cell     pointer to the cell  descriptor
 */
{

    /* setup output and reference pointers */
    }else
        mv_x= mv_y= 0;

    /* -1 because there is an extra line on top for prediction */
        av_log(ctx->avctx, AV_LOG_ERROR,
               "Motion vectors point out of the frame.\n");
        return AVERROR_INVALIDDATA;
    }



        /* copy using 16xH blocks */
        }

        /* copy using 8xH blocks */
            ctx->hdsp.put_pixels_tab[2][0](dst, src, plane->pitch, h);
            w--;
            src += 4;
            dst += 4;
        }
    }

    return 0;
}


/* Average 4/8 pixels at once without rounding using SWAR */
#define AVG_32(dst, src, ref) \
    AV_WN32A(dst, ((AV_RN32(src) + AV_RN32(ref)) >> 1) & 0x7F7F7F7FUL)

#define AVG_64(dst, src, ref) \
    AV_WN64A(dst, ((AV_RN64(src) + AV_RN64(ref)) >> 1) & 0x7F7F7F7F7F7F7F7FULL)


/*
 *  Replicate each even pixel as follows:
 *  ABCDEFGH -> AACCEEGG
 */
static inline uint64_t replicate64(uint64_t a) {
#if HAVE_BIGENDIAN
    a &= 0xFF00FF00FF00FF00ULL;
    a |= a >> 8;
#else
    a &= 0x00FF00FF00FF00FFULL;
    a |= a << 8;
#endif
    return a;
}

static inline uint32_t replicate32(uint32_t a) {
#if HAVE_BIGENDIAN
    a &= 0xFF00FF00UL;
    a |= a >> 8;
#else
    a &= 0x00FF00FFUL;
    a |= a << 8;
#endif
    return a;
}


/* Fill n lines with 64-bit pixel value pix */
static inline void fill_64(uint8_t *dst, const uint64_t pix, int32_t n,
                           int32_t row_offset)
{
    for (; n > 0; dst += row_offset, n--)
        AV_WN64A(dst, pix);
}


/* Error codes for cell decoding. */
enum {
    IV3_NOERR       = 0,
    IV3_BAD_RLE     = 1,
    IV3_BAD_DATA    = 2,
    IV3_BAD_COUNTER = 3,
    IV3_UNSUPPORTED = 4,
    IV3_OUT_OF_DATA = 5
};


#define BUFFER_PRECHECK \
if (*data_ptr >= last_ptr) \
    return IV3_OUT_OF_DATA; \

#define RLE_BLOCK_COPY \
    if (cell->mv_ptr || !skip_flag) \
        copy_block4(dst, ref, row_offset, row_offset, 4 << v_zoom)

#define RLE_BLOCK_COPY_8 \
    pix64 = AV_RN64(ref);\
    if (is_first_row) {/* special prediction case: top line of a cell */\
        pix64 = replicate64(pix64);\
        fill_64(dst + row_offset, pix64, 7, row_offset);\
        AVG_64(dst, ref, dst + row_offset);\
    } else \
        fill_64(dst, pix64, 8, row_offset)

#define RLE_LINES_COPY \
    copy_block4(dst, ref, row_offset, row_offset, num_lines << v_zoom)

#define RLE_LINES_COPY_M10 \
    pix64 = AV_RN64(ref);\
    if (is_top_of_cell) {\
        pix64 = replicate64(pix64);\
        fill_64(dst + row_offset, pix64, (num_lines << 1) - 1, row_offset);\
        AVG_64(dst, ref, dst + row_offset);\
    } else \
        fill_64(dst, pix64, num_lines << 1, row_offset)

#define APPLY_DELTA_4 \
    AV_WN16A(dst + line_offset    ,\
             (AV_RN16(ref    ) + delta_tab->deltas[dyad1]) & 0x7F7F);\
    AV_WN16A(dst + line_offset + 2,\
             (AV_RN16(ref + 2) + delta_tab->deltas[dyad2]) & 0x7F7F);\
    if (mode >= 3) {\
        if (is_top_of_cell && !cell->ypos) {\
            AV_COPY32U(dst, dst + row_offset);\
        } else {\
            AVG_32(dst, ref, dst + row_offset);\
        }\
    }

#define APPLY_DELTA_8 \
    /* apply two 32-bit VQ deltas to next even line */\
    if (is_top_of_cell) { \
        AV_WN32A(dst + row_offset    , \
                 (replicate32(AV_RN32(ref    )) + delta_tab->deltas_m10[dyad1]) & 0x7F7F7F7F);\
        AV_WN32A(dst + row_offset + 4, \
                 (replicate32(AV_RN32(ref + 4)) + delta_tab->deltas_m10[dyad2]) & 0x7F7F7F7F);\
    } else { \
        AV_WN32A(dst + row_offset    , \
                 (AV_RN32(ref    ) + delta_tab->deltas_m10[dyad1]) & 0x7F7F7F7F);\
        AV_WN32A(dst + row_offset + 4, \
                 (AV_RN32(ref + 4) + delta_tab->deltas_m10[dyad2]) & 0x7F7F7F7F);\
    } \
    /* odd lines are not coded but rather interpolated/replicated */\
    /* first line of the cell on the top of image? - replicate */\
    /* otherwise - interpolate */\
    if (is_top_of_cell && !cell->ypos) {\
        AV_COPY64U(dst, dst + row_offset);\
    } else \
        AVG_64(dst, ref, dst + row_offset);


#define APPLY_DELTA_1011_INTER \
    if (mode == 10) { \
        AV_WN32A(dst                 , \
                 (AV_RN32(dst                 ) + delta_tab->deltas_m10[dyad1]) & 0x7F7F7F7F);\
        AV_WN32A(dst + 4             , \
                 (AV_RN32(dst + 4             ) + delta_tab->deltas_m10[dyad2]) & 0x7F7F7F7F);\
        AV_WN32A(dst + row_offset    , \
                 (AV_RN32(dst + row_offset    ) + delta_tab->deltas_m10[dyad1]) & 0x7F7F7F7F);\
        AV_WN32A(dst + row_offset + 4, \
                 (AV_RN32(dst + row_offset + 4) + delta_tab->deltas_m10[dyad2]) & 0x7F7F7F7F);\
    } else { \
        AV_WN16A(dst                 , \
                 (AV_RN16(dst                 ) + delta_tab->deltas[dyad1]) & 0x7F7F);\
        AV_WN16A(dst + 2             , \
                 (AV_RN16(dst + 2             ) + delta_tab->deltas[dyad2]) & 0x7F7F);\
        AV_WN16A(dst + row_offset    , \
                 (AV_RN16(dst + row_offset    ) + delta_tab->deltas[dyad1]) & 0x7F7F);\
        AV_WN16A(dst + row_offset + 2, \
                 (AV_RN16(dst + row_offset + 2) + delta_tab->deltas[dyad2]) & 0x7F7F);\
    }


static int decode_cell_data(Indeo3DecodeContext *ctx, Cell *cell,
                            uint8_t *block, uint8_t *ref_block,
                            ptrdiff_t row_offset, int h_zoom, int v_zoom, int mode,
                            const vqEntry *delta[2], int swap_quads[2],
                            const uint8_t **data_ptr, const uint8_t *last_ptr)
{
    int           x, y, line, num_lines;
    int           rle_blocks = 0;
    uint8_t       code, *dst, *ref;
    const vqEntry *delta_tab;
    unsigned int  dyad1, dyad2;
    uint64_t      pix64;
    int           skip_flag = 0, is_top_of_cell, is_first_row = 1;
    int           blk_row_offset, line_offset;

    blk_row_offset = (row_offset << (2 + v_zoom)) - (cell->width << 2);
    line_offset    = v_zoom ? row_offset : 0;

    if (cell->height & v_zoom || cell->width & h_zoom)
        return IV3_BAD_DATA;

    for (y = 0; y < cell->height; is_first_row = 0, y += 1 + v_zoom) {
        for (x = 0; x < cell->width; x += 1 + h_zoom) {
            ref = ref_block;
            dst = block;

            if (rle_blocks > 0) {
                if (mode <= 4) {
                    RLE_BLOCK_COPY;
                } else if (mode == 10 && !cell->mv_ptr) {
                    RLE_BLOCK_COPY_8;
                }
                rle_blocks--;
            } else {
                for (line = 0; line < 4;) {
                    num_lines = 1;
                    is_top_of_cell = is_first_row && !line;

                    /* select primary VQ table for odd, secondary for even lines */
                    if (mode <= 4)
                        delta_tab = delta[line & 1];
                    else
                        delta_tab = delta[1];
                    BUFFER_PRECHECK;
                    code = bytestream_get_byte(data_ptr);
                    if (code < 248) {
                        if (code < delta_tab->num_dyads) {
                            BUFFER_PRECHECK;
                            dyad1 = bytestream_get_byte(data_ptr);
                            dyad2 = code;
                            if (dyad1 >= delta_tab->num_dyads || dyad1 >= 248)
                                return IV3_BAD_DATA;
                        } else {
                            /* process QUADS */
                            code -= delta_tab->num_dyads;
                            dyad1 = code / delta_tab->quad_exp;
                            dyad2 = code % delta_tab->quad_exp;
                            if (swap_quads[line & 1])
                                FFSWAP(unsigned int, dyad1, dyad2);
                        }
                        if (mode <= 4) {
                            APPLY_DELTA_4;
                        } else if (mode == 10 && !cell->mv_ptr) {
                            APPLY_DELTA_8;
                        } else {
                            APPLY_DELTA_1011_INTER;
                        }
                    } else {
                        /* process RLE codes */
                        switch (code) {
                        case RLE_ESC_FC:
                            skip_flag  = 0;
                            rle_blocks = 1;
                            code       = 253;
                            /* FALLTHROUGH */
                        case RLE_ESC_FF:
                        case RLE_ESC_FE:
                        case RLE_ESC_FD:
                            num_lines = 257 - code - line;
                            if (num_lines <= 0)
                                return IV3_BAD_RLE;
                            if (mode <= 4) {
                                RLE_LINES_COPY;
                            } else if (mode == 10 && !cell->mv_ptr) {
                                RLE_LINES_COPY_M10;
                            }
                            break;
                        case RLE_ESC_FB:
                            BUFFER_PRECHECK;
                            code = bytestream_get_byte(data_ptr);
                            rle_blocks = (code & 0x1F) - 1; /* set block counter */
                            if (code >= 64 || rle_blocks < 0)
                                return IV3_BAD_COUNTER;
                            skip_flag = code & 0x20;
                            num_lines = 4 - line; /* enforce next block processing */
                            if (mode >= 10 || (cell->mv_ptr || !skip_flag)) {
                                if (mode <= 4) {
                                    RLE_LINES_COPY;
                                } else if (mode == 10 && !cell->mv_ptr) {
                                    RLE_LINES_COPY_M10;
                                }
                            }
                            break;
                        case RLE_ESC_F9:
                            skip_flag  = 1;
                            rle_blocks = 1;
                            /* FALLTHROUGH */
                        case RLE_ESC_FA:
                            if (line)
                                return IV3_BAD_RLE;
                            num_lines = 4; /* enforce next block processing */
                            if (cell->mv_ptr) {
                                if (mode <= 4) {
                                    RLE_LINES_COPY;
                                } else if (mode == 10 && !cell->mv_ptr) {
                                    RLE_LINES_COPY_M10;
                                }
                            }
                            break;
                        default:
                            return IV3_UNSUPPORTED;
                        }
                    }

                    line += num_lines;
                    ref  += row_offset * (num_lines << v_zoom);
                    dst  += row_offset * (num_lines << v_zoom);
                }
            }

            /* move to next horizontal block */
            block     += 4 << h_zoom;
            ref_block += 4 << h_zoom;
        }

        /* move to next line of blocks */
        ref_block += blk_row_offset;
        block     += blk_row_offset;
    }
    return IV3_NOERR;
}


/**
 *  Decode a vector-quantized cell.
 *  It consists of several routines, each of which handles one or more "modes"
 *  with which a cell can be encoded.
 *
 *  @param ctx      pointer to the decoder context
 *  @param avctx    ptr to the AVCodecContext
 *  @param plane    pointer to the plane descriptor
 *  @param cell     pointer to the cell  descriptor
 *  @param data_ptr pointer to the compressed data
 *  @param last_ptr pointer to the last byte to catch reads past end of buffer
 *  @return         number of consumed bytes or negative number in case of error
 */
                       Plane *plane, Cell *cell, const uint8_t *data_ptr,
                       const uint8_t *last_ptr)
{

    /* get coding mode and VQ table index from the VQ descriptor byte */

    /* setup output and reference pointers */

        /* use previous line as reference for INTRA cells */
        /* for mode 10 and 11 INTER first copy the predicted cell into the current one */
        /* so we don't need to do data copying for each RLE code later */
            return ret;
    } else {
        /* set the pointer to the reference pixels for modes 0-4 INTER */

        /* -1 because there is an extra line on top for prediction */
            av_log(ctx->avctx, AV_LOG_ERROR,
                   "Motion vectors point out of the frame.\n");
            return AVERROR_INVALIDDATA;
        }

    }

    /* select VQ tables as follows: */
    /* modes 0 and 3 use only the primary table for all lines in a block */
    /* while modes 1 and 4 switch between primary and secondary tables on alternate lines */
        code        = ctx->alt_quant[vq_index];
        prim_indx   = (code >> 4)  + ctx->cb_offset;
        second_indx = (code & 0xF) + ctx->cb_offset;
    } else {
    }

        av_log(avctx, AV_LOG_ERROR, "Invalid VQ table indexes! Primary: %d, secondary: %d!\n",
               prim_indx, second_indx);
        return AVERROR_INVALIDDATA;
    }


    /* requantize the prediction if VQ index of this cell differs from VQ index */
    /* of the predicted cell in order to avoid overflows. */
    }


    case 1:
    case 3: /*------------------ MODES 3 & 4 (4x8 block processing) --------------------*/
    case 4:
            av_log(avctx, AV_LOG_ERROR, "Attempt to apply Mode 3/4 to an INTER cell!\n");
            return AVERROR_INVALIDDATA;
        }

                                 0, zoom_fac, mode, delta, swap_quads,
                                 &data_ptr, last_ptr);
    case 11: /*----------------- MODE 11 (4x8 INTER block processing) ------------------*/
                                     1, 1, mode, delta, swap_quads,
                                     &data_ptr, last_ptr);
        } else { /* mode 10 and 11 INTER processing */
               av_log(avctx, AV_LOG_ERROR, "Attempt to use Mode 11 for an INTRA cell!\n");
               return AVERROR_INVALIDDATA;
            }

                                     zoom_fac, 1, mode, delta, swap_quads,
                                     &data_ptr, last_ptr);
        }
        break;
    default:
        av_log(avctx, AV_LOG_ERROR, "Unsupported coding mode: %d\n", mode);
        return AVERROR_INVALIDDATA;
    }//switch mode

    case IV3_BAD_RLE:
        av_log(avctx, AV_LOG_ERROR, "Mode %d: RLE code %X is not allowed at the current line\n",
               mode, data_ptr[-1]);
        return AVERROR_INVALIDDATA;
    case IV3_BAD_DATA:
        av_log(avctx, AV_LOG_ERROR, "Mode %d: invalid VQ data\n", mode);
        return AVERROR_INVALIDDATA;
    case IV3_BAD_COUNTER:
        av_log(avctx, AV_LOG_ERROR, "Mode %d: RLE-FB invalid counter: %d\n", mode, code);
        return AVERROR_INVALIDDATA;
    case IV3_UNSUPPORTED:
        av_log(avctx, AV_LOG_ERROR, "Mode %d: unsupported RLE code: %X\n", mode, data_ptr[-1]);
        return AVERROR_INVALIDDATA;
    case IV3_OUT_OF_DATA:
        av_log(avctx, AV_LOG_ERROR, "Mode %d: attempt to read past end of buffer\n", mode);
        return AVERROR_INVALIDDATA;
    }

}


/* Binary tree codes. */
enum {
    H_SPLIT    = 0,
    V_SPLIT    = 1,
    INTRA_NULL = 2,
    INTER_DATA = 3
};


#define SPLIT_CELL(size, new_size) (new_size) = ((size) > 2) ? ((((size) + 2) >> 2) << 1) : 1

#define UPDATE_BITPOS(n) \
    ctx->skip_bits  += (n); \
    ctx->need_resync = 1

#define RESYNC_BITSTREAM \
    if (ctx->need_resync && !(get_bits_count(&ctx->gb) & 7)) { \
        skip_bits_long(&ctx->gb, ctx->skip_bits);              \
        ctx->skip_bits   = 0;                                  \
        ctx->need_resync = 0;                                  \
    }

#define CHECK_CELL \
    if (curr_cell.xpos + curr_cell.width > (plane->width >> 2) ||               \
        curr_cell.ypos + curr_cell.height > (plane->height >> 2)) {             \
        av_log(avctx, AV_LOG_ERROR, "Invalid cell: x=%d, y=%d, w=%d, h=%d\n",   \
               curr_cell.xpos, curr_cell.ypos, curr_cell.width, curr_cell.height); \
        return AVERROR_INVALIDDATA;                                                              \
    }


                         Plane *plane, int code, Cell *ref_cell,
                         const int depth, const int strip_width)
{

        av_log(avctx, AV_LOG_ERROR, "Stack overflow (corrupted binary tree)!\n");
        return AVERROR_INVALIDDATA; // unwind recursion
    }

            return AVERROR_INVALIDDATA;
            /* split strip */
        } else
            return AVERROR_INVALIDDATA;
    }

        case V_SPLIT:
                return AVERROR_INVALIDDATA;
            break;
            } else { /* VQ tree NULL code */
                    av_log(avctx, AV_LOG_ERROR, "Invalid VQ_NULL code: %d\n", code);
                    return AVERROR_INVALIDDATA;
                }
                    av_log(avctx, AV_LOG_ERROR, "SkipCell procedure not implemented yet!\n");

                    return AVERROR_INVALIDDATA;

            }
                /* get motion vector index and setup the pointer to the mv set */
                    av_log(avctx, AV_LOG_ERROR, "motion vector out of array\n");
                    return AVERROR_INVALIDDATA;
                }
                    av_log(avctx, AV_LOG_ERROR, "motion vector index out of range\n");
                    return AVERROR_INVALIDDATA;
                }
            } else { /* VQ tree DATA code */

                                         ctx->next_cell_data, ctx->last_byte);
                    return AVERROR_INVALIDDATA;

            }
        }
    }//while

    return AVERROR_INVALIDDATA;
}


                        Plane *plane, const uint8_t *data, int32_t data_size,
                        int32_t strip_width)
{

    /* each plane data starts with mc_vector_count field, */
    /* an optional array of motion vectors followed by the vq data */
        av_log(ctx->avctx, AV_LOG_ERROR,
               "Read invalid number of motion vectors %d\n", num_vectors);
        return AVERROR_INVALIDDATA;
    }
        return AVERROR_INVALIDDATA;


    /* init the bitreader */


    /* initialize the 1st cell and set its dimensions to whole plane */

}


#define OS_HDR_ID   MKBETAG('F', 'R', 'M', 'H')

                                const uint8_t *buf, int buf_size)
{


    /* parse and check the OS header */

        av_log(avctx, AV_LOG_ERROR, "OS header checksum mismatch!\n");
        return AVERROR_INVALIDDATA;
    }

    /* parse the bitstream header */

        av_log(avctx, AV_LOG_ERROR, "Unsupported codec version!\n");
        return AVERROR_INVALIDDATA;
    }


        return 4;


    /* check frame dimensions */
        return AVERROR_INVALIDDATA;

        int res;

        ff_dlog(avctx, "Frame dimensions changed!\n");

        if (width  < 16 || width  > 640 ||
            height < 16 || height > 480 ||
            width  &  3 || height &   3) {
            av_log(avctx, AV_LOG_ERROR,
                   "Invalid picture dimensions: %d x %d!\n", width, height);
            return AVERROR_INVALIDDATA;
        }
        free_frame_buffers(ctx);
        if ((res = allocate_frame_buffers(ctx, avctx, width, height)) < 0)
             return res;
        if ((res = ff_set_dimensions(avctx, width, height)) < 0)
            return res;
    }


    /* unfortunately there is no common order of planes in the buffer */
    /* so we use that sorting algo for determining planes data sizes  */

    }

        av_log(avctx, AV_LOG_ERROR, "One of the y/u/v offsets is invalid\n");
        return AVERROR_INVALIDDATA;
    }


        av_log(avctx, AV_LOG_DEBUG, "Sync frame encountered!\n");
        return 16;
    }

        avpriv_request_sample(avctx, "8-bit pixel format");
        return AVERROR_PATCHWELCOME;
    }

        avpriv_request_sample(avctx, "Halfpel motion vectors");
        return AVERROR_PATCHWELCOME;
    }

    return 0;
}


/**
 *  Convert and output the current plane.
 *  All pixel values will be upsampled by shifting right by one bit.
 *
 *  @param[in]  plane        pointer to the descriptor of the plane being processed
 *  @param[in]  buf_sel      indicates which frame buffer the input data stored in
 *  @param[out] dst          pointer to the buffer receiving converted pixels
 *  @param[in]  dst_pitch    pitch for moving to the next y line
 *  @param[in]  dst_height   output plane height
 */
                         ptrdiff_t dst_pitch, int dst_height)
{

        /* convert four pixels at once using SWAR */
        }

            *dst++ = *src++ << 1;

    }


{




}


                        AVPacket *avpkt)
{

        return res;

    /* skip sync(null) frames */
        // we have processed 16 bytes but no data was decoded
        *got_frame = 0;
        return buf_size;
    }

    /* skip droppable INTER frames if requested */
        return 0;

    /* skip INTER frames if requested */
        return 0;

    /* use BS_BUFFER flag for buffer switching */

        return res;

    /* decode luma plane */
        return res;

    /* decode chroma planes */
        return res;

        return res;

                 avctx->height);


}


{

}

AVCodec ff_indeo3_decoder = {
    .name           = "indeo3",
    .long_name      = NULL_IF_CONFIG_SMALL("Intel Indeo 3"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_INDEO3,
    .priv_data_size = sizeof(Indeo3DecodeContext),
    .init           = decode_init,
    .close          = decode_close,
    .decode         = decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
