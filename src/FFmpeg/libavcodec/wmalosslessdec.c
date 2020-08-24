/*
 * Windows Media Audio Lossless decoder
 * Copyright (c) 2007 Baptiste Coudurier, Benjamin Larsson, Ulion
 * Copyright (c) 2008 - 2011 Sascha Sommer, Benjamin Larsson
 * Copyright (c) 2011 Andreas Öman
 * Copyright (c) 2011 - 2012 Mashiat Sarker Shakkhar
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

#include <inttypes.h>

#include "libavutil/attributes.h"
#include "libavutil/avassert.h"

#include "avcodec.h"
#include "internal.h"
#include "get_bits.h"
#include "put_bits.h"
#include "lossless_audiodsp.h"
#include "wma.h"
#include "wma_common.h"

/** current decoder limitations */
#define WMALL_MAX_CHANNELS      8                       ///< max number of handled channels
#define MAX_SUBFRAMES          32                       ///< max number of subframes per channel
#define MAX_BANDS              29                       ///< max number of scale factor bands
#define MAX_FRAMESIZE       32768                       ///< maximum compressed frame size
#define MAX_ORDER             256

#define WMALL_BLOCK_MIN_BITS    6                       ///< log2 of min block size
#define WMALL_BLOCK_MAX_BITS   14                       ///< log2 of max block size
#define WMALL_BLOCK_MAX_SIZE (1 << WMALL_BLOCK_MAX_BITS)    ///< maximum block size
#define WMALL_BLOCK_SIZES    (WMALL_BLOCK_MAX_BITS - WMALL_BLOCK_MIN_BITS + 1) ///< possible block sizes

#define WMALL_COEFF_PAD_SIZE   16                       ///< pad coef buffers with 0 for use with SIMD

/**
 * @brief frame-specific decoder context for a single channel
 */
typedef struct WmallChannelCtx {
    int16_t     prev_block_len;                         ///< length of the previous block
    uint8_t     transmit_coefs;
    uint8_t     num_subframes;
    uint16_t    subframe_len[MAX_SUBFRAMES];            ///< subframe length in samples
    uint16_t    subframe_offsets[MAX_SUBFRAMES];        ///< subframe positions in the current frame
    uint8_t     cur_subframe;                           ///< current subframe number
    uint16_t    decoded_samples;                        ///< number of already processed samples
    int         quant_step;                             ///< quantization step for the current subframe
    int         transient_counter;                      ///< number of transient samples from the beginning of the transient zone
} WmallChannelCtx;

/**
 * @brief main decoder context
 */
typedef struct WmallDecodeCtx {
    /* generic decoder variables */
    AVCodecContext  *avctx;
    AVFrame         *frame;
    LLAudDSPContext dsp;                           ///< accelerated DSP functions
    uint8_t         *frame_data;                    ///< compressed frame data
    int             max_frame_size;                 ///< max bitstream size
    PutBitContext   pb;                             ///< context for filling the frame_data buffer

    /* frame size dependent frame information (set during initialization) */
    uint32_t        decode_flags;                   ///< used compression features
    int             len_prefix;                     ///< frame is prefixed with its length
    int             dynamic_range_compression;      ///< frame contains DRC data
    uint8_t         bits_per_sample;                ///< integer audio sample size for the unscaled IMDCT output (used to scale to [-1.0, 1.0])
    uint16_t        samples_per_frame;              ///< number of samples to output
    uint16_t        log2_frame_size;
    int8_t          num_channels;                   ///< number of channels in the stream (same as AVCodecContext.num_channels)
    int8_t          lfe_channel;                    ///< lfe channel index
    uint8_t         max_num_subframes;
    uint8_t         subframe_len_bits;              ///< number of bits used for the subframe length
    uint8_t         max_subframe_len_bit;           ///< flag indicating that the subframe is of maximum size when the first subframe length bit is 1
    uint16_t        min_samples_per_subframe;

    /* packet decode state */
    GetBitContext   pgb;                            ///< bitstream reader context for the packet
    int             next_packet_start;              ///< start offset of the next WMA packet in the demuxer packet
    uint8_t         packet_offset;                  ///< offset to the frame in the packet
    uint8_t         packet_sequence_number;         ///< current packet number
    int             num_saved_bits;                 ///< saved number of bits
    int             frame_offset;                   ///< frame offset in the bit reservoir
    int             subframe_offset;                ///< subframe offset in the bit reservoir
    uint8_t         packet_loss;                    ///< set in case of bitstream error
    uint8_t         packet_done;                    ///< set when a packet is fully decoded

    /* frame decode state */
    uint32_t        frame_num;                      ///< current frame number (not used for decoding)
    GetBitContext   gb;                             ///< bitstream reader context
    int             buf_bit_size;                   ///< buffer size in bits
    int16_t         *samples_16[WMALL_MAX_CHANNELS]; ///< current sample buffer pointer (16-bit)
    int32_t         *samples_32[WMALL_MAX_CHANNELS]; ///< current sample buffer pointer (24-bit)
    uint8_t         drc_gain;                       ///< gain for the DRC tool
    int8_t          skip_frame;                     ///< skip output step
    int8_t          parsed_all_subframes;           ///< all subframes decoded?

    /* subframe/block decode state */
    int16_t         subframe_len;                   ///< current subframe length
    int8_t          channels_for_cur_subframe;      ///< number of channels that contain the subframe
    int8_t          channel_indexes_for_cur_subframe[WMALL_MAX_CHANNELS];

    WmallChannelCtx channel[WMALL_MAX_CHANNELS];    ///< per channel data

    // WMA Lossless-specific

    uint8_t do_arith_coding;
    uint8_t do_ac_filter;
    uint8_t do_inter_ch_decorr;
    uint8_t do_mclms;
    uint8_t do_lpc;

    int8_t  acfilter_order;
    int8_t  acfilter_scaling;
    int16_t acfilter_coeffs[16];
    int     acfilter_prevvalues[WMALL_MAX_CHANNELS][16];

    int8_t  mclms_order;
    int8_t  mclms_scaling;
    int16_t mclms_coeffs[WMALL_MAX_CHANNELS * WMALL_MAX_CHANNELS * 32];
    int16_t mclms_coeffs_cur[WMALL_MAX_CHANNELS * WMALL_MAX_CHANNELS];
    int32_t mclms_prevvalues[WMALL_MAX_CHANNELS * 2 * 32];
    int32_t mclms_updates[WMALL_MAX_CHANNELS * 2 * 32];
    int     mclms_recent;

    int     movave_scaling;
    int     quant_stepsize;

    struct {
        int order;
        int scaling;
        int coefsend;
        int bitsend;
        DECLARE_ALIGNED(16, int16_t, coefs)[MAX_ORDER + WMALL_COEFF_PAD_SIZE/sizeof(int16_t)];
        DECLARE_ALIGNED(16, int32_t, lms_prevvalues)[MAX_ORDER * 2 + WMALL_COEFF_PAD_SIZE/sizeof(int16_t)];
        DECLARE_ALIGNED(16, int16_t, lms_updates)[MAX_ORDER * 2 + WMALL_COEFF_PAD_SIZE/sizeof(int16_t)];
        int recent;
    } cdlms[WMALL_MAX_CHANNELS][9];

    int cdlms_ttl[WMALL_MAX_CHANNELS];

    int bV3RTM;

    int is_channel_coded[WMALL_MAX_CHANNELS];
    int update_speed[WMALL_MAX_CHANNELS];

    int transient[WMALL_MAX_CHANNELS];
    int transient_pos[WMALL_MAX_CHANNELS];
    int seekable_tile;

    unsigned ave_sum[WMALL_MAX_CHANNELS];

    int channel_residues[WMALL_MAX_CHANNELS][WMALL_BLOCK_MAX_SIZE];

    int lpc_coefs[WMALL_MAX_CHANNELS][40];
    int lpc_order;
    int lpc_scaling;
    int lpc_intbits;
} WmallDecodeCtx;

/** Get sign of integer (1 for positive, -1 for negative and 0 for zero) */
#define WMASIGN(x) (((x) > 0) - ((x) < 0))

{

        av_log(avctx, AV_LOG_ERROR, "block_align is not set or invalid\n");
        return AVERROR(EINVAL);
    }

        avpriv_request_sample(avctx,
                              "More than " AV_STRINGIFY(WMALL_MAX_CHANNELS) " channels");
        return AVERROR_PATCHWELCOME;
    }

        return AVERROR(ENOMEM);


        } else {
            av_log(avctx, AV_LOG_ERROR, "Unknown bit-depth: %"PRIu8"\n",
                   s->bits_per_sample);
            return AVERROR_INVALIDDATA;
        }
        /* dump the extradata */
            ff_dlog(avctx, "[%x] ", avctx->extradata[i]);

    } else {
        avpriv_request_sample(avctx, "Unsupported extradata size");
        return AVERROR_PATCHWELCOME;
    }

    /* generic init */

    /* frame info */

    /* get frame len */
                                                          3, s->decode_flags);

    /* init previous block len */

    /* subframe info */


        av_log(avctx, AV_LOG_ERROR, "invalid number of subframes %"PRIu8"\n",
               s->max_num_subframes);
        return AVERROR_INVALIDDATA;
    }


    /* extract lfe channel position */

        unsigned int mask;
        for (mask = 1; mask < 16; mask <<= 1)
            if (channel_mask & mask)
                ++s->lfe_channel;
    }

        return AVERROR(ENOMEM);

}

/**
 * @brief Decode the subframe length.
 * @param s      context
 * @param offset sample offset in the frame
 * @return decoded subframe length on success, < 0 in case of an error
 */
{

    /* no need to read from the bitstream when only one length is possible */
        return s->min_samples_per_subframe;


    /* sanity check the length */
        av_log(s->avctx, AV_LOG_ERROR, "broken frame: subframe_len %i\n",
               subframe_len);
        return AVERROR_INVALIDDATA;
    }
    return subframe_len;
}

/**
 * @brief Decode how the data in the frame is split into subframes.
 *       Every WMA frame contains the encoded data for a fixed number of
 *       samples per channel. The data for every channel might be split
 *       into several subframes. This function will reconstruct the list of
 *       subframes for every channel.
 *
 *       If the subframes are not evenly split, the algorithm estimates the
 *       channels with the lowest number of total samples.
 *       Afterwards, for each of these channels a bit is read from the
 *       bitstream that indicates if the channel contains a subframe with the
 *       next subframe size that is going to be read from the bitstream or not.
 *       If a channel contains such a subframe, the subframe size gets added to
 *       the channel's subframe list.
 *       The algorithm repeats these steps until the frame is properly divided
 *       between the individual channels.
 *
 * @param s context
 * @return 0 on success, < 0 in case of an error
 */
{

    /* reset tiling information */

        fixed_channel_layout = 1;

    /* loop until the frame data is split between the subframes */

        /* check which channels contain the subframe */
                    contains_subframe[c] = 1;
                } else {
                }
            } else
                contains_subframe[c] = 0;
        }

            av_log(s->avctx, AV_LOG_ERROR,
                   "Found empty subframe\n");
            return AVERROR_INVALIDDATA;
        }

        /* get subframe length, subframe_len == 0 is not allowed */
            return AVERROR_INVALIDDATA;
        /* add subframes to the individual channels and find new min_channel_len */

                    av_log(s->avctx, AV_LOG_ERROR,
                           "broken frame: num subframes > 31\n");
                    return AVERROR_INVALIDDATA;
                }
                    av_log(s->avctx, AV_LOG_ERROR, "broken frame: "
                           "channel len(%"PRIu16") > samples_per_frame(%"PRIu16")\n",
                           num_samples[c], s->samples_per_frame);
                    return AVERROR_INVALIDDATA;
                }
            } else if (num_samples[c] <= min_channel_len) {
                if (num_samples[c] < min_channel_len) {
                    channels_for_cur_subframe = 0;
                    min_channel_len = num_samples[c];
                }
                ++channels_for_cur_subframe;
            }
        }

        int i, offset = 0;
        }
    }

    return 0;
}

{


{
        int i, send_coef_bits;
        int cbits = av_log2(s->mclms_scaling + 1);
        if (1 << cbits < s->mclms_scaling + 1)
            cbits++;

        send_coef_bits = get_bitsz(&s->gb, cbits) + 2;

        for (i = 0; i < s->mclms_order * s->num_channels * s->num_channels; i++)
            s->mclms_coeffs[i] = get_bits(&s->gb, send_coef_bits);

        for (i = 0; i < s->num_channels; i++) {
            int c;
            for (c = 0; c < i; c++)
                s->mclms_coeffs_cur[i * s->num_channels + c] = get_bits(&s->gb, send_coef_bits);
        }
    }

{

                av_log(s->avctx, AV_LOG_ERROR,
                       "Order[%d][%d] %d > max (%d), not supported\n",
                       c, i, s->cdlms[c][i].order, MAX_ORDER);
                s->cdlms[0][0].order = 0;
                return AVERROR_INVALIDDATA;
            }
                static int warned;
                if(!warned)
                    avpriv_request_sample(s->avctx, "CDLMS of order %d",
                                          s->cdlms[c][i].order);
                warned = 1;
            }
        }


            for (i = 0; i < s->cdlms_ttl[c]; i++) {
                int cbits, shift_l, shift_r, j;
                cbits = av_log2(s->cdlms[c][i].order);
                if ((1 << cbits) < s->cdlms[c][i].order)
                    cbits++;
                s->cdlms[c][i].coefsend = get_bits(&s->gb, cbits) + 1;

                cbits = av_log2(s->cdlms[c][i].scaling + 1);
                if ((1 << cbits) < s->cdlms[c][i].scaling + 1)
                    cbits++;

                s->cdlms[c][i].bitsend = get_bitsz(&s->gb, cbits) + 2;
                shift_l = 32 - s->cdlms[c][i].bitsend;
                shift_r = 32 - s->cdlms[c][i].scaling - 2;
                for (j = 0; j < s->cdlms[c][i].coefsend; j++)
                    s->cdlms[c][i].coefs[j] =
                        (get_bits(&s->gb, s->cdlms[c][i].bitsend) << shift_l) >> shift_r;
            }
        }

                   0, WMALL_COEFF_PAD_SIZE);
    }

    return 0;
}

{

    }

        else
            s->channel_residues[ch][0] = get_sbits_long(&s->gb, s->bits_per_sample);
        i++;
    }
        int rem, rem_bits;
        unsigned quo = 0, residue;
                return -1;
        }
            quo += get_bits_long(&s->gb, get_bits(&s->gb, 5) + 1);

            residue = quo;
        else {
        }


    }

    return 0;

}

static void decode_lpc(WmallDecodeCtx *s)
{
    int ch, i, cbits;
    s->lpc_order   = get_bits(&s->gb, 5) + 1;
    s->lpc_scaling = get_bits(&s->gb, 4);
    s->lpc_intbits = get_bits(&s->gb, 3) + 1;
    cbits = s->lpc_scaling + s->lpc_intbits;
    for (ch = 0; ch < s->num_channels; ch++)
        for (i = 0; i < s->lpc_order; i++)
            s->lpc_coefs[ch][i] = get_sbits(&s->gb, cbits);
}

{



                   sizeof(s->cdlms[ich][ilms].coefs));
                   sizeof(s->cdlms[ich][ilms].lms_prevvalues));
                   sizeof(s->cdlms[ich][ilms].lms_updates));
        }
    }

/**
 * @brief Reset filter parameters and transient area at new seekable tile.
 */
{
        /* first sample of a seekable subframe is considered as the starting of
            a transient area which is samples_per_frame samples long */
    }
}

{

        }
    }

            -range, range - 1);
    }

               sizeof(int32_t) * order * num_channels);
    }

{

            continue;
    }

{
    }

{
            continue;
        } else {
        }
    }

{
        else
    }

#define CD_LMS(bits, ROUND) \
static void lms_update ## bits (WmallDecodeCtx *s, int ich, int ilms, int input) \
{ \
    int recent = s->cdlms[ich][ilms].recent; \
    int range  = 1 << s->bits_per_sample - 1; \
    int order  = s->cdlms[ich][ilms].order; \
    int ##bits##_t *prev = (int##bits##_t *)s->cdlms[ich][ilms].lms_prevvalues; \
 \
    if (recent) \
        recent--; \
    else { \
        memcpy(prev + order, prev, (bits/8) * order); \
        memcpy(s->cdlms[ich][ilms].lms_updates + order, \
               s->cdlms[ich][ilms].lms_updates, \
               sizeof(*s->cdlms[ich][ilms].lms_updates) * order); \
        recent = order - 1; \
    } \
 \
    prev[recent] = av_clip(input, -range, range - 1); \
    s->cdlms[ich][ilms].lms_updates[recent] = WMASIGN(input) * s->update_speed[ich]; \
 \
    s->cdlms[ich][ilms].lms_updates[recent + (order >> 4)] >>= 2; \
    s->cdlms[ich][ilms].lms_updates[recent + (order >> 3)] >>= 1; \
    s->cdlms[ich][ilms].recent = recent; \
    memset(s->cdlms[ich][ilms].lms_updates + recent + order, 0, \
           sizeof(s->cdlms[ich][ilms].lms_updates) - \
           sizeof(*s->cdlms[ich][ilms].lms_updates)*(recent+order)); \
} \
 \
static void revert_cdlms ## bits (WmallDecodeCtx *s, int ch, \
                                  int coef_begin, int coef_end) \
{ \
    int icoef, ilms, num_lms, residue, input; \
    unsigned pred;\
 \
    num_lms = s->cdlms_ttl[ch]; \
    for (ilms = num_lms - 1; ilms >= 0; ilms--) { \
        for (icoef = coef_begin; icoef < coef_end; icoef++) { \
            int##bits##_t *prevvalues = (int##bits##_t *)s->cdlms[ch][ilms].lms_prevvalues; \
            pred = (1 << s->cdlms[ch][ilms].scaling) >> 1; \
            residue = s->channel_residues[ch][icoef]; \
            pred += s->dsp.scalarproduct_and_madd_int## bits (s->cdlms[ch][ilms].coefs, \
                                                        prevvalues + s->cdlms[ch][ilms].recent, \
                                                        s->cdlms[ch][ilms].lms_updates + \
                                                        s->cdlms[ch][ilms].recent, \
                                                        FFALIGN(s->cdlms[ch][ilms].order, ROUND), \
                                                        WMASIGN(residue)); \
            input = residue + (unsigned)((int)pred >> s->cdlms[ch][ilms].scaling); \
            lms_update ## bits(s, ch, ilms, input); \
            s->channel_residues[ch][icoef] = input; \
        } \
    } \
    if (bits <= 16) emms_c(); \
}


{
        return;
        int icoef;
        }
    }
}

{

            pred = 0;
                else
                    pred += (uint32_t)s->channel_residues[ich][i - j - 1] * filter_coeffs[j];
            }
        }
            pred = 0;
        }
                prevvalues[j] = prevvalues[j - tile_size];
            }else
    }

{


    /* reset channel context and find the next block offset and size
        == the next block of the channel with the smallest number of
        decoded samples */
        }
    }

    /* get a list of all channels that contain the estimated block */
        /* subtract already processed samples */

        /* and count if there are multiple subframes that match our profile */
                s->channel[i].subframe_len[cur_subframe];
        }
    }

    /* check if the frame will be complete after processing the
        estimated block */



            avpriv_request_sample(s->avctx, "Arithmetic coding");
            return AVERROR_PATCHWELCOME;
        }



            return res;

    }


        av_log(s->avctx, AV_LOG_DEBUG,
               "Waiting for seekable tile\n");
        av_frame_unref(s->frame);
        return -1;
    }




            // LPC
                decode_lpc(s);
                avpriv_request_sample(s->avctx, "Expect wrong output since "
                                      "inverse LPC filter");
            }
        } else
    }


    else
        padding_zeroes = 0;

            av_log(s->avctx, AV_LOG_ERROR,
                   "Invalid number of padding bits in raw PCM tile\n");
            return AVERROR_INVALIDDATA;
        }
        ff_dlog(s->avctx, "RAWPCM %d bits per sample. "
                "total %d bits, remain=%d\n", bits,
                bits * s->num_channels * subframe_len, get_bits_count(&s->gb));
    } else {
            return AVERROR_INVALIDDATA;
                else
                else
            } else {
            }
        }


        /* Dequantize */
            for (i = 0; i < s->num_channels; i++)
                for (j = 0; j < subframe_len; j++)
                    s->channel_residues[i][j] *= (unsigned)s->quant_stepsize;
    }

    /* Write to proper output buffer depending on bit-depth */

            } else {
            }
        }
    }

    /* handled one subframe */
            av_log(s->avctx, AV_LOG_ERROR, "broken subframe\n");
            return AVERROR_INVALIDDATA;
        }
    }
    return 0;
}

/**
 * @brief Decode one WMA frame.
 * @param s codec context
 * @return 0 if the trailer bit indicates that this is the last frame,
 *         1 if there are additional frames
 */
{

        /* return an error if no frame could be decoded at all */
        s->packet_loss = 1;
        s->frame->nb_samples = 0;
        return ret;
    }
    }

    /* get frame length */
        len = get_bits(gb, s->log2_frame_size);

    /* decode tile information */
        s->packet_loss = 1;
        av_frame_unref(s->frame);
        return ret;
    }

    /* read drc info */

    /* no idea what these are for, might be the number of samples
       that need to be skipped at the beginning or end of a stream */

        /* usually true for the first frame */
            skip = get_bits(gb, av_log2(s->samples_per_frame * 2));
            ff_dlog(s->avctx, "start skip: %i\n", skip);
        }

        /* sometimes true for the last frame */
                return AVERROR_INVALIDDATA;
        }

    }

    /* reset subframe states */
    }

    /* decode all subframes */
            s->packet_loss = 1;
            if (s->frame->nb_samples)
                s->frame->nb_samples = decoded_samples;
            return 0;
        }
    }



        if (len != (get_bits_count(gb) - s->frame_offset) + 2) {
            /* FIXME: not sure if this is always an error */
            av_log(s->avctx, AV_LOG_ERROR,
                   "frame[%"PRIu32"] would have to skip %i bits\n",
                   s->frame_num,
                   len - (get_bits_count(gb) - s->frame_offset) - 1);
            s->packet_loss = 1;
            return 0;
        }

        /* skip the rest of the frame data */
        skip_bits_long(gb, len - (get_bits_count(gb) - s->frame_offset) - 1);
    }

    /* decode trailer bit */
}

/**
 * @brief Calculate remaining input buffer length.
 * @param s  codec context
 * @param gb bitstream reader context
 * @return remaining size in bits
 */
{
}

/**
 * @brief Fill the bit reservoir with a (partial) frame.
 * @param s      codec context
 * @param gb     bitstream reader context
 * @param len    length of the partial frame
 * @param append decides whether to reset the buffer or not
 */
                      int append)
{

    /* when the frame data does not need to be concatenated, the input buffer
        is reset and additional bits from the previous frame are copied
        and skipped later so that a fast byte copy is possible */

    }


        avpriv_request_sample(s->avctx, "Too small input buffer");
        s->packet_loss = 1;
        s->num_saved_bits = 0;
        return;
    }

                         s->num_saved_bits);
    } else {
    }


}

                         AVPacket* avpkt)
{



            return 0;


        /* parse packet header */
            avpriv_request_sample(avctx, "Bitstream splicing");

        /* get number of bits that need to be added to the previous frame */

        /* check for packet loss */
            s->packet_loss = 1;
            av_log(avctx, AV_LOG_ERROR,
                   "Packet loss detected! seq %"PRIx8" vs %x\n",
                   s->packet_sequence_number, packet_sequence_number);
        }

            }

            /* Append the previous frame data to the remaining data from the
             * previous packet to create a full frame. */

            /* decode the cross packet frame if it is valid */
        } else if (s->num_saved_bits - s->frame_offset) {
                    s->num_saved_bits - s->frame_offset);
        }

            /* Reset number of saved bits so that the decoder does not start
             * to decode incomplete frames in the s->len_prefix == 0 case. */
        }

    } else {


            (frame_size = show_bits(gb, s->log2_frame_size)) &&
            frame_size <= remaining_bits(s, gb)) {
            save_bits(s, gb, frame_size, 0);

            if (!s->packet_loss)
                s->packet_done = !decode_frame(s);
            /* when the frames do not have a length prefix, we don't know the
             * compressed length of the individual frames however, we know what
             * part of a new packet belongs to the previous frame therefore we
             * save the incoming packet first, then we append the "previous
             * frame" data from the next packet so that we get a buffer that
             * only contains full frames */
        } else {
        }
    }

        av_log(avctx, AV_LOG_ERROR, "Overread %d\n", -remaining_bits(s, gb));
        s->packet_loss = 1;
    }

        /* save the rest of the data so that it can be decoded
         * with the next packet */
    }



}

static void flush(AVCodecContext *avctx)
{
    WmallDecodeCtx *s    = avctx->priv_data;
    s->packet_loss       = 1;
    s->packet_done       = 0;
    s->num_saved_bits    = 0;
    s->frame_offset      = 0;
    s->next_packet_start = 0;
    s->cdlms[0][0].order = 0;
    s->frame->nb_samples = 0;
    init_put_bits(&s->pb, s->frame_data, s->max_frame_size);
}

{


}

AVCodec ff_wmalossless_decoder = {
    .name           = "wmalossless",
    .long_name      = NULL_IF_CONFIG_SMALL("Windows Media Audio Lossless"),
    .type           = AVMEDIA_TYPE_AUDIO,
    .id             = AV_CODEC_ID_WMALOSSLESS,
    .priv_data_size = sizeof(WmallDecodeCtx),
    .init           = decode_init,
    .close          = decode_close,
    .decode         = decode_packet,
    .flush          = flush,
    .capabilities   = AV_CODEC_CAP_SUBFRAMES | AV_CODEC_CAP_DR1 | AV_CODEC_CAP_DELAY,
    .caps_internal  = FF_CODEC_CAP_INIT_CLEANUP,
    .sample_fmts    = (const enum AVSampleFormat[]) { AV_SAMPLE_FMT_S16P,
                                                      AV_SAMPLE_FMT_S32P,
                                                      AV_SAMPLE_FMT_NONE },
};
