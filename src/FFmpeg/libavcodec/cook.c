/*
 * COOK compatible decoder
 * Copyright (c) 2003 Sascha Sommer
 * Copyright (c) 2005 Benjamin Larsson
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
 * Cook compatible decoder. Bastardization of the G.722.1 standard.
 * This decoder handles RealNetworks, RealAudio G2 data.
 * Cook is identified by the codec name cook in RM files.
 *
 * To use this decoder, a calling application must supply the extradata
 * bytes provided from the RM container; 8+ bytes for mono streams and
 * 16+ for stereo streams (maybe more).
 *
 * Codec technicalities (all this assume a buffer length of 1024):
 * Cook works with several different techniques to achieve its compression.
 * In the timedomain the buffer is divided into 8 pieces and quantized. If
 * two neighboring pieces have different quantization index a smooth
 * quantization curve is used to get a smooth overlap between the different
 * pieces.
 * To get to the transformdomain Cook uses a modulated lapped transform.
 * The transform domain has 50 subbands with 20 elements each. This
 * means only a maximum of 50*20=1000 coefficients are used out of the 1024
 * available.
 */

#include "libavutil/channel_layout.h"
#include "libavutil/lfg.h"

#include "audiodsp.h"
#include "avcodec.h"
#include "get_bits.h"
#include "bytestream.h"
#include "fft.h"
#include "internal.h"
#include "sinewin.h"
#include "unary.h"

#include "cookdata.h"

/* the different Cook versions */
#define MONO            0x1000001
#define STEREO          0x1000002
#define JOINT_STEREO    0x1000003
#define MC_COOK         0x2000000

#define SUBBAND_SIZE    20
#define MAX_SUBPACKETS   5

typedef struct cook_gains {
    int *now;
    int *previous;
} cook_gains;

typedef struct COOKSubpacket {
    int                 ch_idx;
    int                 size;
    int                 num_channels;
    int                 cookversion;
    int                 subbands;
    int                 js_subband_start;
    int                 js_vlc_bits;
    int                 samples_per_channel;
    int                 log2_numvector_size;
    unsigned int        channel_mask;
    VLC                 channel_coupling;
    int                 joint_stereo;
    int                 bits_per_subpacket;
    int                 bits_per_subpdiv;
    int                 total_subbands;
    int                 numvector_size;       // 1 << log2_numvector_size;

    float               mono_previous_buffer1[1024];
    float               mono_previous_buffer2[1024];

    cook_gains          gains1;
    cook_gains          gains2;
    int                 gain_1[9];
    int                 gain_2[9];
    int                 gain_3[9];
    int                 gain_4[9];
} COOKSubpacket;

typedef struct cook {
    /*
     * The following 5 functions provide the lowlevel arithmetic on
     * the internal audio buffers.
     */
    void (*scalar_dequant)(struct cook *q, int index, int quant_index,
                           int *subband_coef_index, int *subband_coef_sign,
                           float *mlt_p);

    void (*decouple)(struct cook *q,
                     COOKSubpacket *p,
                     int subband,
                     float f1, float f2,
                     float *decode_buffer,
                     float *mlt_buffer1, float *mlt_buffer2);

    void (*imlt_window)(struct cook *q, float *buffer1,
                        cook_gains *gains_ptr, float *previous_buffer);

    void (*interpolate)(struct cook *q, float *buffer,
                        int gain_index, int gain_index_next);

    void (*saturate_output)(struct cook *q, float *out);

    AVCodecContext*     avctx;
    AudioDSPContext     adsp;
    GetBitContext       gb;
    /* stream data */
    int                 num_vectors;
    int                 samples_per_channel;
    /* states */
    AVLFG               random_state;
    int                 discarded_packets;

    /* transform data */
    FFTContext          mdct_ctx;
    float*              mlt_window;

    /* VLC data */
    VLC                 envelope_quant_index[13];
    VLC                 sqvh[7];          // scalar quantization

    /* generate tables and related variables */
    int                 gain_size_factor;
    float               gain_table[31];

    /* data buffers */

    uint8_t*            decoded_bytes_buffer;
    DECLARE_ALIGNED(32, float, mono_mdct_output)[2048];
    float               decode_buffer_1[1024];
    float               decode_buffer_2[1024];
    float               decode_buffer_0[1060]; /* static allocation for joint decode */

    const float         *cplscales[5];
    int                 num_subpackets;
    COOKSubpacket       subpacket[MAX_SUBPACKETS];
} COOKContext;

static float     pow2tab[127];
static float rootpow2tab[127];

/*************** init functions ***************/

/* table generator */
{
    /* fast way of computing 2^i and 2^(0.5*i) for -63 <= i < 64 */
    }

/* table generator */
{


{

                           envelope_quant_index_huffbits[i], 1, 1,
                           envelope_quant_index_huffcodes[i], 2, 2, 0);
    }
                           cvh_huffbits[i], 1, 1,
                           cvh_huffcodes[i], 2, 2, 0);
    }

                               (1 << q->subpacket[i].js_vlc_bits) - 1,
                               ccpl_huffbits[q->subpacket[i].js_vlc_bits - 2], 1, 1,
                               ccpl_huffcodes[q->subpacket[i].js_vlc_bits - 2], 2, 2, 0);
        }
    }

}

{

        return AVERROR(ENOMEM);

    /* Initialize the MLT window: simple sine window. */

    /* Initialize the MDCT. */
        av_freep(&q->mlt_window);
        return ret;
    }
           av_log2(mlt_size) + 1);

}

{

/*************** init functions end ***********/

#define DECODE_BYTES_PAD1(bytes) (3 - ((bytes) + 3) % 4)
#define DECODE_BYTES_PAD2(bytes) ((bytes) % 4 + DECODE_BYTES_PAD1(2 * (bytes)))

/**
 * Cook indata decoding, every 32 bits are XORed with 0x37c511f2.
 * Why? No idea, some checksum/error detection method maybe.
 *
 * Out buffer size: extra bytes are needed to cope with
 * padding/misalignment.
 * Subpackets passed to the decoder can contain two, consecutive
 * half-subpackets, of identical but arbitrary size.
 *          1234 1234 1234 1234  extraA extraB
 * Case 1:  AAAA BBBB              0      0
 * Case 2:  AAAA ABBB BB--         3      3
 * Case 3:  AAAA AABB BBBB         2      2
 * Case 4:  AAAA AAAB BBBB BB--    1      5
 *
 * Nice way to waste CPU cycles.
 *
 * @param inbuffer  pointer to byte array of indata
 * @param out       pointer to byte array of outdata
 * @param bytes     number of bytes
 */
static inline int decode_bytes(const uint8_t *inbuffer, uint8_t *out, int bytes)
{
    static const uint32_t tab[4] = {
        AV_BE2NE32C(0x37c511f2u), AV_BE2NE32C(0xf237c511u),
        AV_BE2NE32C(0x11f237c5u), AV_BE2NE32C(0xc511f237u),
    };
    int i, off;
    uint32_t c;
    const uint32_t *buf;
    uint32_t *obuf = (uint32_t *) out;
    /* FIXME: 64 bit platforms would be able to do 64 bits at a time.
     * I'm too lazy though, should be something like
     * for (i = 0; i < bitamount / 64; i++)
     *     (int64_t) out[i] = 0x37c511f237c511f2 ^ av_be2ne64(int64_t) in[i]);
     * Buffer alignment needs to be checked. */

    off = (intptr_t) inbuffer & 3;
    buf = (const uint32_t *) (inbuffer - off);
    c = tab[off];
    bytes += 3 + off;
    for (i = 0; i < bytes / 4; i++)
        obuf[i] = c ^ buf[i];

    return off;
}

{

    /* Free allocated memory buffers. */

    /* Free the transform. */

    /* Free the VLC tables. */


}

/**
 * Fill the gain array for the timedomain quantization.
 *
 * @param gb          pointer to the GetBitContext
 * @param gaininfo    array[9] of gain indexes
 */
{


    i = 0;

    }

/**
 * Create the quant index table needed for the envelope.
 *
 * @param q                 pointer to the COOKContext
 * @param quant_index_table pointer to the array
 */
static int decode_envelope(COOKContext *q, COOKSubpacket *p,
                           int *quant_index_table)
{
    int i, j, vlc_index;

    quant_index_table[0] = get_bits(&q->gb, 6) - 6; // This is used later in categorize

    for (i = 1; i < p->total_subbands; i++) {
        vlc_index = i;
        if (i >= p->js_subband_start * 2) {
            vlc_index -= p->js_subband_start;
        } else {
            vlc_index /= 2;
            if (vlc_index < 1)
                vlc_index = 1;
        }
        if (vlc_index > 13)
            vlc_index = 13; // the VLC tables >13 are identical to No. 13

        j = get_vlc2(&q->gb, q->envelope_quant_index[vlc_index - 1].table,
                     q->envelope_quant_index[vlc_index - 1].bits, 2);
        quant_index_table[i] = quant_index_table[i - 1] + j - 12; // differential encoding
        if (quant_index_table[i] > 63 || quant_index_table[i] < -63) {
            av_log(q->avctx, AV_LOG_ERROR,
                   "Invalid quantizer %d at position %d, outside [-63, 63] range\n",
                   quant_index_table[i], i);
            return AVERROR_INVALIDDATA;
        }
    }

    return 0;
}

/**
 * Calculate the category and category_index vector.
 *
 * @param q                     pointer to the COOKContext
 * @param quant_index_table     pointer to the array
 * @param category              pointer to the category array
 * @param category_index        pointer to the category_index array
 */
                       int *category, int *category_index)
{





    /* Estimate bias. */
        }
    }

    /* Calculate total number of bits. */
    num_bits = 0;
    }
    tmpbias1 = tmpbias2 = num_bits;

            int max = -999999;
            index = -1;
                    }
                }
            }
                break;
        } else {  /* <--- */
            int min = 999999;
            index = -1;
                    }
                }
            }
                break;
        }
    }




/**
 * Expand the category vector.
 *
 * @param q                     pointer to the COOKContext
 * @param category              pointer to the category array
 * @param category_index        pointer to the category_index array
 */
                                   int *category_index)
{
    int i;
    {
            --category[idx];
    }
}

/**
 * The real requantization of the mltcoefs
 *
 * @param q                     pointer to the COOKContext
 * @param index                 index
 * @param quant_index           quantisation index
 * @param subband_coef_index    array of indexes to quant_centroid_tab
 * @param subband_coef_sign     signs of coefficients
 * @param mlt_p                 pointer into the mlt buffer
 */
                                 int *subband_coef_index, int *subband_coef_sign,
                                 float *mlt_p)
{

        } else {
            /* noise coding if subband_coef_index[i] == 0 */
        }
    }
/**
 * Unpack the subband_coef_index and subband_coef_sign vectors.
 *
 * @param q                     pointer to the COOKContext
 * @param category              pointer to the category array
 * @param subband_coef_index    array of indexes to quant_centroid_tab
 * @param subband_coef_sign     signs of coefficients
 */
static int unpack_SQVH(COOKContext *q, COOKSubpacket *p, int category,
                       int *subband_coef_index, int *subband_coef_sign)
{
    int i, j;
    int vlc, vd, tmp, result;

    vd = vd_tab[category];
    result = 0;
    for (i = 0; i < vpr_tab[category]; i++) {
        vlc = get_vlc2(&q->gb, q->sqvh[category].table, q->sqvh[category].bits, 3);
        if (p->bits_per_subpacket < get_bits_count(&q->gb)) {
            vlc = 0;
            result = 1;
        }
        for (j = vd - 1; j >= 0; j--) {
            tmp = (vlc * invradix_tab[category]) / 0x100000;
            subband_coef_index[vd * i + j] = vlc - tmp * (kmax_tab[category] + 1);
            vlc = tmp;
        }
        for (j = 0; j < vd; j++) {
            if (subband_coef_index[i * vd + j]) {
                if (get_bits_count(&q->gb) < p->bits_per_subpacket) {
                    subband_coef_sign[i * vd + j] = get_bits1(&q->gb);
                } else {
                    result = 1;
                    subband_coef_sign[i * vd + j] = 0;
                }
            } else {
                subband_coef_sign[i * vd + j] = 0;
            }
        }
    }
    return result;
}


/**
 * Fill the mlt_buffer with mlt coefficients.
 *
 * @param q                 pointer to the COOKContext
 * @param category          pointer to the category array
 * @param quant_index_table pointer to the array
 * @param mlt_buffer        pointer to mlt coefficients
 */
                           int *quant_index_table, float *mlt_buffer)
{
    /* A zero in this table means that the subband coefficient is
       random noise coded. */
    /* A zero in this table means that the subband coefficient is a
       positive multiplicator. */

                index = 7;
                for (j = 0; j < p->total_subbands; j++)
                    category[band + j] = 7;
            }
        }
        }
                          subband_coef_index, subband_coef_sign,
    }

    /* FIXME: should this be removed, or moved into loop above? */
}


{

        return res;
            return AVERROR_INVALIDDATA;
    }

}


/**
 * the actual requantization of the timedomain samples
 *
 * @param q                 pointer to the COOKContext
 * @param buffer            pointer to the timedomain buffer
 * @param gain_index        index for the block multiplier
 * @param gain_index_next   index for the next block multiplier
 */
                              int gain_index, int gain_index_next)
{

    } else {                                        // smooth gain
        }
    }

/**
 * Apply transform window, overlap buffers.
 *
 * @param q                 pointer to the COOKContext
 * @param inbuffer          pointer to the mltcoefficients
 * @param gains_ptr         current and previous gains
 * @param previous_buffer   pointer to the previous buffer to be used for overlapping
 */
                              cook_gains *gains_ptr, float *previous_buffer)
{
    /* The weird thing here, is that the two halves of the time domain
     * buffer are swapped. Also, the newest data, that we save away for
     * next frame, has the wrong sign. Hence the subtraction below.
     * Almost sounds like a complex conjugate/reverse data/FFT effect.
     */

    /* Apply window and overlap */

/**
 * The modulated lapped transform, this takes transform coefficients
 * and transforms them into timedomain samples.
 * Apply transform window, overlap buffers, apply gain profile
 * and buffer management.
 *
 * @param q                 pointer to the COOKContext
 * @param inbuffer          pointer to the mltcoefficients
 * @param gains_ptr         current and previous gains
 * @param previous_buffer   pointer to the previous buffer to be used for overlapping
 */
                      cook_gains *gains_ptr, float *previous_buffer)
{

    /* Inverse modified discrete cosine transform */


    /* Apply gain profile */

    /* Save away the current to be previous block. */


/**
 * function for getting the jointstereo coupling information
 *
 * @param q                 pointer to the COOKContext
 * @param decouple_tab      decoupling array
 */
{

        return 0;

                                               p->channel_coupling.table,
                                               p->channel_coupling.bits, 3);
    else
                av_log(q->avctx, AV_LOG_ERROR, "decouple value too large\n");
                return AVERROR_INVALIDDATA;
            }
        }
    return 0;
}

/**
 * function decouples a pair of signals from a single signal via multiplication.
 *
 * @param q                 pointer to the COOKContext
 * @param subband           index of the current subband
 * @param f1                multiplier for channel 1 extraction
 * @param f2                multiplier for channel 2 extraction
 * @param decode_buffer     input buffer
 * @param mlt_buffer1       pointer to left channel mlt coefficients
 * @param mlt_buffer2       pointer to right channel mlt coefficients
 */
                           COOKSubpacket *p,
                           int subband,
                           float f1, float f2,
                           float *decode_buffer,
                           float *mlt_buffer1, float *mlt_buffer2)
{
    }

/**
 * function for decoding joint stereo data
 *
 * @param q                 pointer to the COOKContext
 * @param mlt_buffer1       pointer to left channel mlt coefficients
 * @param mlt_buffer2       pointer to right channel mlt coefficients
 */
                        float *mlt_buffer_left, float *mlt_buffer_right)
{


    /* Make sure the buffers are zeroed out. */
        return res;
        return res;
    /* The two channels are stored interleaved in decode_buffer. */
        }
    }

    /* When we reach js_subband_start (the higher frequencies)
       the coefficients are stored in a coupling scheme. */
                    mlt_buffer_left, mlt_buffer_right);
    }

    return 0;
}

/**
 * First part of subpacket decoding:
 *  decode raw stream bytes and read gain info.
 *
 * @param q                 pointer to the COOKContext
 * @param inbuffer          pointer to raw stream data
 * @param gains_ptr         array of current/prev gain pointers
 */
static inline void decode_bytes_and_gain(COOKContext *q, COOKSubpacket *p,
                                         const uint8_t *inbuffer,
                                         cook_gains *gains_ptr)
{
    int offset;

    offset = decode_bytes(inbuffer, q->decoded_bytes_buffer,
                          p->bits_per_subpacket / 8);
    init_get_bits(&q->gb, q->decoded_bytes_buffer + offset,
                  p->bits_per_subpacket);
    decode_gain_info(&q->gb, gains_ptr->now);

    /* Swap current and previous gains */
    FFSWAP(int *, gains_ptr->now, gains_ptr->previous);
}

/**
 * Saturate the output signal and interleave.
 *
 * @param q                 pointer to the COOKContext
 * @param out               pointer to the output vector
 */
{


/**
 * Final part of subpacket decoding:
 *  Apply modulated lapped transform, gain compensation,
 *  clip and convert to integer.
 *
 * @param q                 pointer to the COOKContext
 * @param decode_buffer     pointer to the mlt coefficients
 * @param gains_ptr         array of current/prev gain pointers
 * @param previous_buffer   pointer to the previous buffer to be used for overlapping
 * @param out               pointer to the output buffer
 */
                                         cook_gains *gains_ptr, float *previous_buffer,
                                         float *out)
{
}


/**
 * Cook subpacket decoding. This function returns one decoded subpacket,
 * usually 1024 samples per channel.
 *
 * @param q                 pointer to the COOKContext
 * @param inbuffer          pointer to the inbuffer
 * @param outbuffer         pointer to the outbuffer
 */
                            const uint8_t *inbuffer, float **outbuffer)
{


            return res;
    } else {
        if ((res = mono_decode(q, p, q->decode_buffer_1)) < 0)
            return res;

        if (p->num_channels == 2) {
            decode_bytes_and_gain(q, p, inbuffer + sub_packet_size / 2, &p->gains2);
            if ((res = mono_decode(q, p, q->decode_buffer_2)) < 0)
                return res;
        }
    }


        else
            mlt_compensate_output(q, q->decode_buffer_2, &p->gains2,
                                  p->mono_previous_buffer2,
                                  outbuffer ? outbuffer[p->ch_idx + 1] : NULL);
    }

    return 0;
}


                             int *got_frame_ptr, AVPacket *avpkt)
{

        return buf_size;

    /* get output buffer */
            return ret;
    }

    /* estimate subpacket sizes */

        q->subpacket[i].size = 2 * buf[avctx->block_align - q->num_subpackets + i];
        q->subpacket[0].size -= q->subpacket[i].size + 1;
        if (q->subpacket[0].size < 0) {
            av_log(avctx, AV_LOG_DEBUG,
                   "frame subpacket size total > avctx->block_align!\n");
            return AVERROR_INVALIDDATA;
        }
    }

    /* decode supbackets */
               "subpacket[%i] size %i js %i %i block_align %i\n",
               i, q->subpacket[i].size, q->subpacket[i].joint_stereo, offset,
               avctx->block_align);

            return ret;
    }

    /* Discard the first two frames: no valid audio. */
    }


}

static void dump_cook_context(COOKContext *q)
{
    //int i=0;
#define PRINT(a, b) ff_dlog(q->avctx, " %s = %d\n", a, b);
    ff_dlog(q->avctx, "COOKextradata\n");
    ff_dlog(q->avctx, "cookversion=%x\n", q->subpacket[0].cookversion);
    if (q->subpacket[0].cookversion > STEREO) {
        PRINT("js_subband_start", q->subpacket[0].js_subband_start);
        PRINT("js_vlc_bits", q->subpacket[0].js_vlc_bits);
    }
    ff_dlog(q->avctx, "COOKContext\n");
    PRINT("nb_channels", q->avctx->channels);
    PRINT("bit_rate", (int)q->avctx->bit_rate);
    PRINT("sample_rate", q->avctx->sample_rate);
    PRINT("samples_per_channel", q->subpacket[0].samples_per_channel);
    PRINT("subbands", q->subpacket[0].subbands);
    PRINT("js_subband_start", q->subpacket[0].js_subband_start);
    PRINT("log2_numvector_size", q->subpacket[0].log2_numvector_size);
    PRINT("numvector_size", q->subpacket[0].numvector_size);
    PRINT("total_subbands", q->subpacket[0].total_subbands);
}

/**
 * Cook initialization
 *
 * @param avctx     pointer to the AVCodecContext
 */
{

    /* Take care of the codec specific extradata. */
        av_log(avctx, AV_LOG_ERROR, "Necessary extradata missing!\n");
        return AVERROR_INVALIDDATA;
    }


    /* Take data from the AVCodecContext (RM container). */
        av_log(avctx, AV_LOG_ERROR, "Invalid number of channels\n");
        return AVERROR_INVALIDDATA;
    }

        return AVERROR(EINVAL);

    /* Initialize RNG. */


        /* 8 for mono, 16 for stereo, ? for multichannel
           Swap to right endianness so we don't need to care later on. */
            av_log(avctx, AV_LOG_ERROR, "js_subband_start %d is too large\n", q->subpacket[s].js_subband_start);
            return AVERROR_INVALIDDATA;
        }

        /* Initialize extradata related variables. */

        /* Initialize default data states. */

        /* Initialize version-dependent variables */

               q->subpacket[s].cookversion);
        case MONO:
            if (avctx->channels != 1) {
                avpriv_request_sample(avctx, "Container channels != 1");
                return AVERROR_PATCHWELCOME;
            }
            av_log(avctx, AV_LOG_DEBUG, "MONO\n");
            break;
                q->subpacket[s].bits_per_subpdiv = 1;
                q->subpacket[s].num_channels = 2;
            }
                avpriv_request_sample(avctx, "Container channels != 2");
                return AVERROR_PATCHWELCOME;
            }
            }
            }
            }
            break;
        case MC_COOK:
            av_log(avctx, AV_LOG_DEBUG, "MULTI_CHANNEL\n");
            channel_mask |= q->subpacket[s].channel_mask = bytestream2_get_be32(&gb);

            if (av_get_channel_layout_nb_channels(q->subpacket[s].channel_mask) > 1) {
                q->subpacket[s].total_subbands = q->subpacket[s].subbands +
                                                 q->subpacket[s].js_subband_start;
                q->subpacket[s].joint_stereo = 1;
                q->subpacket[s].num_channels = 2;
                q->subpacket[s].samples_per_channel = samples_per_frame >> 1;

                if (q->subpacket[s].samples_per_channel > 256) {
                    q->subpacket[s].log2_numvector_size = 6;
                }
                if (q->subpacket[s].samples_per_channel > 512) {
                    q->subpacket[s].log2_numvector_size = 7;
                }
            } else
                q->subpacket[s].samples_per_channel = samples_per_frame;

            break;
        default:
            avpriv_request_sample(avctx, "Cook version %d",
                                  q->subpacket[s].cookversion);
            return AVERROR_PATCHWELCOME;
        }

            av_log(avctx, AV_LOG_ERROR, "different number of samples per channel!\n");
            return AVERROR_INVALIDDATA;
        } else


        /* Initialize variable relations */

        /* Try to catch some obviously faulty streams, otherwise it might be exploitable */
            avpriv_request_sample(avctx, "total_subbands > 53");
            return AVERROR_PATCHWELCOME;
        }

            av_log(avctx, AV_LOG_ERROR, "js_vlc_bits = %d, only >= %d and <= 6 allowed!\n",
                   q->subpacket[s].js_vlc_bits, 2 * q->subpacket[s].joint_stereo);
            return AVERROR_INVALIDDATA;
        }

            avpriv_request_sample(avctx, "subbands > 50");
            return AVERROR_PATCHWELCOME;
        }
            avpriv_request_sample(avctx, "subbands = 0");
            return AVERROR_PATCHWELCOME;
        }

            av_log(avctx, AV_LOG_ERROR, "Too many subpackets %d for channels %d\n", q->num_subpackets, q->avctx->channels);
            return AVERROR_INVALIDDATA;
        }

            avpriv_request_sample(avctx, "subpackets > %d", FFMIN(MAX_SUBPACKETS, avctx->block_align));
            return AVERROR_PATCHWELCOME;
        }
    }

    /* Try to catch some obviously faulty streams, otherwise it might be exploitable */
        q->samples_per_channel != 1024) {
        avpriv_request_sample(avctx, "samples_per_channel = %d",
                              q->samples_per_channel);
        return AVERROR_PATCHWELCOME;
    }

    /* Generate tables */

        return ret;

    /* Pad the databuffer with:
       DECODE_BYTES_PAD1 or DECODE_BYTES_PAD2 for decode_bytes(),
       AV_INPUT_BUFFER_PADDING_SIZE, for the bitstreamreader. */
        return AVERROR(ENOMEM);

    /* Initialize transform. */
        return ret;

    /* Initialize COOK signal arithmetic handling */
    }

        avctx->channel_layout = channel_mask;
    else


    dump_cook_context(q);

    return 0;
}

AVCodec ff_cook_decoder = {
    .name           = "cook",
    .long_name      = NULL_IF_CONFIG_SMALL("Cook / Cooker / Gecko (RealAudio G2)"),
    .type           = AVMEDIA_TYPE_AUDIO,
    .id             = AV_CODEC_ID_COOK,
    .priv_data_size = sizeof(COOKContext),
    .init           = cook_decode_init,
    .close          = cook_decode_close,
    .decode         = cook_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
    .caps_internal  = FF_CODEC_CAP_INIT_CLEANUP,
    .sample_fmts    = (const enum AVSampleFormat[]) { AV_SAMPLE_FMT_FLTP,
                                                      AV_SAMPLE_FMT_NONE },
};
