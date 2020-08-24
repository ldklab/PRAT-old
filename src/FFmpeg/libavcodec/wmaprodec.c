/*
 * Wmapro compatible decoder
 * Copyright (c) 2007 Baptiste Coudurier, Benjamin Larsson, Ulion
 * Copyright (c) 2008 - 2011 Sascha Sommer, Benjamin Larsson
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
 * @brief wmapro decoder implementation
 * Wmapro is an MDCT based codec comparable to wma standard or AAC.
 * The decoding therefore consists of the following steps:
 * - bitstream decoding
 * - reconstruction of per-channel data
 * - rescaling and inverse quantization
 * - IMDCT
 * - windowing and overlapp-add
 *
 * The compressed wmapro bitstream is split into individual packets.
 * Every such packet contains one or more wma frames.
 * The compressed frames may have a variable length and frames may
 * cross packet boundaries.
 * Common to all wmapro frames is the number of samples that are stored in
 * a frame.
 * The number of samples and a few other decode flags are stored
 * as extradata that has to be passed to the decoder.
 *
 * The wmapro frames themselves are again split into a variable number of
 * subframes. Every subframe contains the data for 2^N time domain samples
 * where N varies between 7 and 12.
 *
 * Example wmapro bitstream (in samples):
 *
 * ||   packet 0           || packet 1 || packet 2      packets
 * ---------------------------------------------------
 * || frame 0      || frame 1       || frame 2    ||    frames
 * ---------------------------------------------------
 * ||   |      |   ||   |   |   |   ||            ||    subframes of channel 0
 * ---------------------------------------------------
 * ||      |   |   ||   |   |   |   ||            ||    subframes of channel 1
 * ---------------------------------------------------
 *
 * The frame layouts for the individual channels of a wma frame does not need
 * to be the same.
 *
 * However, if the offsets and lengths of several subframes of a frame are the
 * same, the subframes of the channels can be grouped.
 * Every group may then use special coding techniques like M/S stereo coding
 * to improve the compression ratio. These channel transformations do not
 * need to be applied to a whole subframe. Instead, they can also work on
 * individual scale factor bands (see below).
 * The coefficients that carry the audio signal in the frequency domain
 * are transmitted as huffman-coded vectors with 4, 2 and 1 elements.
 * In addition to that, the encoder can switch to a runlevel coding scheme
 * by transmitting subframe_length / 128 zero coefficients.
 *
 * Before the audio signal can be converted to the time domain, the
 * coefficients have to be rescaled and inverse quantized.
 * A subframe is therefore split into several scale factor bands that get
 * scaled individually.
 * Scale factors are submitted for every frame but they might be shared
 * between the subframes of a channel. Scale factors are initially DPCM-coded.
 * Once scale factors are shared, the differences are transmitted as runlevel
 * codes.
 * Every subframe length and offset combination in the frame layout shares a
 * common quantization factor that can be adjusted for every channel by a
 * modifier.
 * After the inverse quantization, the coefficients get processed by an IMDCT.
 * The resulting values are then windowed with a sine window and the first half
 * of the values are added to the second half of the output from the previous
 * subframe in order to reconstruct the output samples.
 */

#include <inttypes.h>

#include "libavutil/ffmath.h"
#include "libavutil/float_dsp.h"
#include "libavutil/intfloat.h"
#include "libavutil/intreadwrite.h"
#include "avcodec.h"
#include "internal.h"
#include "get_bits.h"
#include "put_bits.h"
#include "wmaprodata.h"
#include "sinewin.h"
#include "wma.h"
#include "wma_common.h"

/** current decoder limitations */
#define WMAPRO_MAX_CHANNELS    8                             ///< max number of handled channels
#define MAX_SUBFRAMES  32                                    ///< max number of subframes per channel
#define MAX_BANDS      29                                    ///< max number of scale factor bands
#define MAX_FRAMESIZE  32768                                 ///< maximum compressed frame size
#define XMA_MAX_STREAMS         8
#define XMA_MAX_CHANNELS_STREAM 2
#define XMA_MAX_CHANNELS        (XMA_MAX_STREAMS * XMA_MAX_CHANNELS_STREAM)

#define WMAPRO_BLOCK_MIN_BITS  6                                           ///< log2 of min block size
#define WMAPRO_BLOCK_MAX_BITS 13                                           ///< log2 of max block size
#define WMAPRO_BLOCK_MIN_SIZE (1 << WMAPRO_BLOCK_MIN_BITS)                 ///< minimum block size
#define WMAPRO_BLOCK_MAX_SIZE (1 << WMAPRO_BLOCK_MAX_BITS)                 ///< maximum block size
#define WMAPRO_BLOCK_SIZES    (WMAPRO_BLOCK_MAX_BITS - WMAPRO_BLOCK_MIN_BITS + 1) ///< possible block sizes


#define VLCBITS            9
#define SCALEVLCBITS       8
#define VEC4MAXDEPTH    ((HUFF_VEC4_MAXBITS+VLCBITS-1)/VLCBITS)
#define VEC2MAXDEPTH    ((HUFF_VEC2_MAXBITS+VLCBITS-1)/VLCBITS)
#define VEC1MAXDEPTH    ((HUFF_VEC1_MAXBITS+VLCBITS-1)/VLCBITS)
#define SCALEMAXDEPTH   ((HUFF_SCALE_MAXBITS+SCALEVLCBITS-1)/SCALEVLCBITS)
#define SCALERLMAXDEPTH ((HUFF_SCALE_RL_MAXBITS+VLCBITS-1)/VLCBITS)

static VLC              sf_vlc;           ///< scale factor DPCM vlc
static VLC              sf_rl_vlc;        ///< scale factor run length vlc
static VLC              vec4_vlc;         ///< 4 coefficients per symbol
static VLC              vec2_vlc;         ///< 2 coefficients per symbol
static VLC              vec1_vlc;         ///< 1 coefficient per symbol
static VLC              coef_vlc[2];      ///< coefficient run length vlc codes
static float            sin64[33];        ///< sine table for decorrelation

/**
 * @brief frame specific decoder context for a single channel
 */
typedef struct WMAProChannelCtx {
    int16_t  prev_block_len;                          ///< length of the previous block
    uint8_t  transmit_coefs;
    uint8_t  num_subframes;
    uint16_t subframe_len[MAX_SUBFRAMES];             ///< subframe length in samples
    uint16_t subframe_offset[MAX_SUBFRAMES];          ///< subframe positions in the current frame
    uint8_t  cur_subframe;                            ///< current subframe number
    uint16_t decoded_samples;                         ///< number of already processed samples
    uint8_t  grouped;                                 ///< channel is part of a group
    int      quant_step;                              ///< quantization step for the current subframe
    int8_t   reuse_sf;                                ///< share scale factors between subframes
    int8_t   scale_factor_step;                       ///< scaling step for the current subframe
    int      max_scale_factor;                        ///< maximum scale factor for the current subframe
    int      saved_scale_factors[2][MAX_BANDS];       ///< resampled and (previously) transmitted scale factor values
    int8_t   scale_factor_idx;                        ///< index for the transmitted scale factor values (used for resampling)
    int*     scale_factors;                           ///< pointer to the scale factor values used for decoding
    uint8_t  table_idx;                               ///< index in sf_offsets for the scale factor reference block
    float*   coeffs;                                  ///< pointer to the subframe decode buffer
    uint16_t num_vec_coeffs;                          ///< number of vector coded coefficients
    DECLARE_ALIGNED(32, float, out)[WMAPRO_BLOCK_MAX_SIZE + WMAPRO_BLOCK_MAX_SIZE / 2]; ///< output buffer
} WMAProChannelCtx;

/**
 * @brief channel group for channel transformations
 */
typedef struct WMAProChannelGrp {
    uint8_t num_channels;                                     ///< number of channels in the group
    int8_t  transform;                                        ///< transform on / off
    int8_t  transform_band[MAX_BANDS];                        ///< controls if the transform is enabled for a certain band
    float   decorrelation_matrix[WMAPRO_MAX_CHANNELS*WMAPRO_MAX_CHANNELS];
    float*  channel_data[WMAPRO_MAX_CHANNELS];                ///< transformation coefficients
} WMAProChannelGrp;

/**
 * @brief main decoder context
 */
typedef struct WMAProDecodeCtx {
    /* generic decoder variables */
    AVCodecContext*  avctx;                         ///< codec context for av_log
    AVFloatDSPContext *fdsp;
    uint8_t          frame_data[MAX_FRAMESIZE +
                      AV_INPUT_BUFFER_PADDING_SIZE];///< compressed frame data
    PutBitContext    pb;                            ///< context for filling the frame_data buffer
    FFTContext       mdct_ctx[WMAPRO_BLOCK_SIZES];  ///< MDCT context per block size
    DECLARE_ALIGNED(32, float, tmp)[WMAPRO_BLOCK_MAX_SIZE]; ///< IMDCT output buffer
    const float*     windows[WMAPRO_BLOCK_SIZES];   ///< windows for the different block sizes

    /* frame size dependent frame information (set during initialization) */
    uint32_t         decode_flags;                  ///< used compression features
    uint8_t          len_prefix;                    ///< frame is prefixed with its length
    uint8_t          dynamic_range_compression;     ///< frame contains DRC data
    uint8_t          bits_per_sample;               ///< integer audio sample size for the unscaled IMDCT output (used to scale to [-1.0, 1.0])
    uint16_t         samples_per_frame;             ///< number of samples to output
    uint16_t         log2_frame_size;
    int8_t           lfe_channel;                   ///< lfe channel index
    uint8_t          max_num_subframes;
    uint8_t          subframe_len_bits;             ///< number of bits used for the subframe length
    uint8_t          max_subframe_len_bit;          ///< flag indicating that the subframe is of maximum size when the first subframe length bit is 1
    uint16_t         min_samples_per_subframe;
    int8_t           num_sfb[WMAPRO_BLOCK_SIZES];   ///< scale factor bands per block size
    int16_t          sfb_offsets[WMAPRO_BLOCK_SIZES][MAX_BANDS];                    ///< scale factor band offsets (multiples of 4)
    int8_t           sf_offsets[WMAPRO_BLOCK_SIZES][WMAPRO_BLOCK_SIZES][MAX_BANDS]; ///< scale factor resample matrix
    int16_t          subwoofer_cutoffs[WMAPRO_BLOCK_SIZES]; ///< subwoofer cutoff values

    /* packet decode state */
    GetBitContext    pgb;                           ///< bitstream reader context for the packet
    int              next_packet_start;             ///< start offset of the next wma packet in the demuxer packet
    uint8_t          packet_offset;                 ///< frame offset in the packet
    uint8_t          packet_sequence_number;        ///< current packet number
    int              num_saved_bits;                ///< saved number of bits
    int              frame_offset;                  ///< frame offset in the bit reservoir
    int              subframe_offset;               ///< subframe offset in the bit reservoir
    uint8_t          packet_loss;                   ///< set in case of bitstream error
    uint8_t          packet_done;                   ///< set when a packet is fully decoded
    uint8_t          eof_done;                      ///< set when EOF reached and extra subframe is written (XMA1/2)

    /* frame decode state */
    uint32_t         frame_num;                     ///< current frame number (not used for decoding)
    GetBitContext    gb;                            ///< bitstream reader context
    int              buf_bit_size;                  ///< buffer size in bits
    uint8_t          drc_gain;                      ///< gain for the DRC tool
    int8_t           skip_frame;                    ///< skip output step
    int8_t           parsed_all_subframes;          ///< all subframes decoded?
    uint8_t          skip_packets;                  ///< packets to skip to find next packet in a stream (XMA1/2)

    /* subframe/block decode state */
    int16_t          subframe_len;                  ///< current subframe length
    int8_t           nb_channels;                   ///< number of channels in stream (XMA1/2)
    int8_t           channels_for_cur_subframe;     ///< number of channels that contain the subframe
    int8_t           channel_indexes_for_cur_subframe[WMAPRO_MAX_CHANNELS];
    int8_t           num_bands;                     ///< number of scale factor bands
    int8_t           transmit_num_vec_coeffs;       ///< number of vector coded coefficients is part of the bitstream
    int16_t*         cur_sfb_offsets;               ///< sfb offsets for the current block
    uint8_t          table_idx;                     ///< index for the num_sfb, sfb_offsets, sf_offsets and subwoofer_cutoffs tables
    int8_t           esc_len;                       ///< length of escaped coefficients

    uint8_t          num_chgroups;                  ///< number of channel groups
    WMAProChannelGrp chgroup[WMAPRO_MAX_CHANNELS];  ///< channel group information

    WMAProChannelCtx channel[WMAPRO_MAX_CHANNELS];  ///< per channel data
} WMAProDecodeCtx;

typedef struct XMADecodeCtx {
    WMAProDecodeCtx xma[XMA_MAX_STREAMS];
    AVFrame *frames[XMA_MAX_STREAMS];
    int current_stream;
    int num_streams;
    float samples[XMA_MAX_CHANNELS][512 * 64];
    int offset[XMA_MAX_STREAMS];
    int start_channel[XMA_MAX_STREAMS];
} XMADecodeCtx;

/**
 *@brief helper function to print the most important members of the context
 *@param s context
 */
static av_cold void dump_context(WMAProDecodeCtx *s)
{
#define PRINT(a, b)     av_log(s->avctx, AV_LOG_DEBUG, " %s = %d\n", a, b);
#define PRINT_HEX(a, b) av_log(s->avctx, AV_LOG_DEBUG, " %s = %"PRIx32"\n", a, b);

    PRINT("ed sample bit depth", s->bits_per_sample);
    PRINT_HEX("ed decode flags", s->decode_flags);
    PRINT("samples per frame",   s->samples_per_frame);
    PRINT("log2 frame size",     s->log2_frame_size);
    PRINT("max num subframes",   s->max_num_subframes);
    PRINT("len prefix",          s->len_prefix);
    PRINT("num channels",        s->nb_channels);
}

/**
 *@brief Uninitialize the decoder and free all resources.
 *@param avctx codec context
 *@return 0 on success, < 0 otherwise
 */
{



}

{


}

static av_cold int get_rate(AVCodecContext *avctx)
{
    if (avctx->codec_id != AV_CODEC_ID_WMAPRO) { // XXX: is this really only for XMA?
        if (avctx->sample_rate > 44100)
            return 48000;
        else if (avctx->sample_rate > 32000)
            return 44100;
        else if (avctx->sample_rate > 24000)
            return 32000;
        return 24000;
    }

    return avctx->sample_rate;
}

/**
 *@brief Initialize the decoder.
 *@param avctx codec context
 *@return 0 on success, -1 otherwise
 */
{

        avctx->block_align = 2048;

        av_log(avctx, AV_LOG_ERROR, "block_align is not set\n");
        return AVERROR(EINVAL);
    }




    /** dump the extradata */

        s->decode_flags    = 0x10d6;
        s->bits_per_sample = 16;
        channel_mask       = 0; //AV_RL32(edata_ptr+2); /* not always in expected order */
        if ((num_stream+1) * XMA_MAX_CHANNELS_STREAM > avctx->channels) /* stream config is 2ch + 2ch + ... + 1/2ch */
            s->nb_channels = 1;
        else
            s->nb_channels = 2;
        s->decode_flags    = 0x10d6;
        s->bits_per_sample = 16;
        channel_mask       = 0; /* would need to aggregate from all streams */
        s->nb_channels = edata_ptr[32 + ((edata_ptr[0]==3)?0:8) + 4*num_stream + 0]; /* nth stream config */
        s->decode_flags    = 0x10d6;
        s->bits_per_sample = 16;
        channel_mask       = 0; /* would need to aggregate from all streams */
        s->nb_channels     = edata_ptr[8 + 20*num_stream + 17]; /* nth stream config */

            avpriv_request_sample(avctx, "bits per sample is %d", s->bits_per_sample);
            return AVERROR_PATCHWELCOME;
        }
    } else {
        avpriv_request_sample(avctx, "Unknown extradata size");
        return AVERROR_PATCHWELCOME;
    }

    /** generic init */
        avpriv_request_sample(avctx, "Large block align");
        return AVERROR_PATCHWELCOME;
    }

    /** frame info */
        s->skip_frame = 0;
    else


    /** get frame len */
            avpriv_request_sample(avctx, "14-bit block sizes");
            return AVERROR_PATCHWELCOME;
        }
    } else {
        s->samples_per_frame = 512;
    }

    /** subframe info */


        av_log(avctx, AV_LOG_ERROR, "invalid number of subframes %"PRId8"\n",
               s->max_num_subframes);
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "min_samples_per_subframe of %d too small\n",
               s->min_samples_per_subframe);
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "invalid sample rate\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "invalid number of channels %d\n",
               s->nb_channels);
        return AVERROR_INVALIDDATA;
        av_log(avctx, AV_LOG_ERROR, "invalid number of channels per XMA stream %d\n",
               s->nb_channels);
        return AVERROR_INVALIDDATA;
        avpriv_request_sample(avctx,
                              "More than %d channels", WMAPRO_MAX_CHANNELS);
        return AVERROR_PATCHWELCOME;
    }

    /** init previous block len */

    /** extract lfe channel position */

        unsigned int mask;
        }
    }

                    scale_huffbits, 1, 1,
                    scale_huffcodes, 2, 2, 616);

                    scale_rl_huffbits, 1, 1,
                    scale_rl_huffcodes, 4, 4, 1406);

                    coef0_huffbits, 1, 1,
                    coef0_huffcodes, 4, 4, 2108);

                    coef1_huffbits, 1, 1,
                    coef1_huffcodes, 4, 4, 3912);

                    vec4_huffbits, 1, 1,
                    vec4_huffcodes, 2, 2, 604);

                    vec2_huffbits, 1, 1,
                    vec2_huffcodes, 2, 2, 562);

                    vec1_huffbits, 1, 1,
                    vec1_huffcodes, 2, 2, 562);

    /** calculate number of scale factor bands and their offsets
        for every possible block size */



                break;
        }
            av_log(avctx, AV_LOG_ERROR, "num_sfb invalid\n");
            return AVERROR_INVALIDDATA;
        }
    }


    /** Scale factors can be shared between blocks of different size
        as every block has a different scale factor band layout.
        The matrix sf_offsets is needed to find the correct scale factor.
     */

        int b;
                int v = 0;
                    v++;
                }
            }
        }
    }

        return AVERROR(ENOMEM);

    /** init MDCT, FIXME: only init needed sizes */

    /** init MDCT windows: simple sine window */
    }

    /** calculate subwoofer cutoff values */
    }

    /** calculate sine values for the decorrelation matrix */

        dump_context(s);


}

/**
 *@brief Initialize the decoder.
 *@param avctx codec context
 *@return 0 on success, -1 otherwise
 */
{

}

/**
 *@brief Decode the subframe length.
 *@param s context
 *@param offset sample offset in the frame
 *@return decoded subframe length on success, < 0 in case of an error
 */
{

    /** no need to read from the bitstream when only one length is possible */
        return s->min_samples_per_subframe;

        return AVERROR_INVALIDDATA;

    /** 1 bit indicates if the subframe is of maximum length */
    } else
        frame_len_shift = get_bits(&s->gb, s->subframe_len_bits);


    /** sanity check the length */
        subframe_len > s->samples_per_frame) {
        av_log(s->avctx, AV_LOG_ERROR, "broken frame: subframe_len %i\n",
               subframe_len);
        return AVERROR_INVALIDDATA;
    }
    return subframe_len;
}

/**
 *@brief Decode how the data in the frame is split into subframes.
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
 *@param s context
 *@return 0 on success, < 0 in case of an error
 */
{

    /* Should never consume more than 3073 bits (256 iterations for the
     * while loop when always the minimum amount of 128 samples is subtracted
     * from missing samples in the 8 channel case).
     * 1 + BLOCK_MAX_SIZE * MAX_CHANNELS / BLOCK_MIN_SIZE * (MAX_CHANNELS  + 4)
     */

    /** reset tiling information */


    /** loop until the frame data is split between the subframes */

        /** check which channels contain the subframe */
                else
            } else
        }

        /** get subframe length, subframe_len == 0 is not allowed */
            return AVERROR_INVALIDDATA;

        /** add subframes to the individual channels and find new min_channel_len */

                    av_log(s->avctx, AV_LOG_ERROR,
                           "broken frame: num subframes > 31\n");
                    return AVERROR_INVALIDDATA;
                }
                    av_log(s->avctx, AV_LOG_ERROR, "broken frame: "
                           "channel len > samples_per_frame\n");
                    return AVERROR_INVALIDDATA;
                }
                }
            }
        }

        int i;
        int offset = 0;
                    " len %i\n", s->frame_num, c, i,
                    s->channel[c].subframe_len[i]);
        }
    }

    return 0;
}

/**
 *@brief Calculate a decorrelation matrix from the bitstream parameters.
 *@param s codec context
 *@param chgroup channel group for which the matrix needs to be calculated
 */
                                        WMAProChannelGrp *chgroup)
{
           s->nb_channels * sizeof(*chgroup->decorrelation_matrix));



        int x;
            int y;

                } else {
                }

            }
        }
    }

/**
 *@brief Decode channel transformation parameters
 *@param s codec context
 *@return >= 0 in case of success, < 0 in case of bitstream errors
 */
{
    /* should never consume more than 1921 bits for the 8 channel case
     * 1 + MAX_CHANNELS * (MAX_CHANNELS + 2 + 3 * MAX_CHANNELS * MAX_CHANNELS
     * + MAX_CHANNELS + MAX_BANDS + 1)
     */

    /** in the one channel case channel transforms are pointless */

            avpriv_request_sample(s->avctx,
                                  "Channel transform bit");
            return AVERROR_PATCHWELCOME;
        }


            /** decode channel mask */
                    }
                }
            } else {
                }
            }

            /** decode transform type */
                        avpriv_request_sample(s->avctx,
                                              "Unknown channel transform type");
                        return AVERROR_PATCHWELCOME;
                    }
                } else {
                    } else {
                        /** cos(pi/4) */
                    }
                }
                    } else {
                        /** FIXME: more than 6 coupled channels not supported */
                        if (chgroup->num_channels > 6) {
                            avpriv_request_sample(s->avctx,
                                                  "Coupled channels > 6");
                        } else {
                                   default_decorrelation[chgroup->num_channels],
                                   chgroup->num_channels * chgroup->num_channels *
                                   sizeof(*chgroup->decorrelation_matrix));
                        }
                    }
                }
            }

            /** decode transform on / off */
                    int i;
                    /** transform can be enabled for individual bands */
                    for (i = 0; i < s->num_bands; i++) {
                        chgroup->transform_band[i] = get_bits1(&s->gb);
                    }
                } else {
                }
            }
        }
    }
    return 0;
}

/**
 *@brief Extract the coefficients from the bitstream.
 *@param s codec context
 *@param c current channel number
 *@return 0 on success, < 0 in case of bitstream errors
 */
{
    /* Integers 0..15 as single-precision floats.  The table saves a
       costly int to float conversion, and storing the values as
       integers allows fast sign-flipping. */
        0x00000000, 0x3f800000, 0x40000000, 0x40400000,
        0x40800000, 0x40a00000, 0x40c00000, 0x40e00000,
        0x41000000, 0x41100000, 0x41200000, 0x41300000,
        0x41400000, 0x41500000, 0x41600000, 0x41700000,
    };



    } else {
    }

    /** decode vector coefficients (consumes up to 167 bits per iteration for
      4 vector coded large values) */


                } else {
                }
            }
        } else {
        }

        /** decode sign */
            } else {
                /** switch to run level mode when subframe_len / 128 zeros
                    were found in a row */
            }
        }
    }

    /** decode run level coded coefficients */
                                    cur_coeff, s->subframe_len,
            return AVERROR_INVALIDDATA;
    }

    return 0;
}

/**
 *@brief Extract scale factors from the bitstream.
 *@param s codec context
 *@return 0 on success, < 0 in case of bitstream errors
 */
{

    /** should never consume more than 5344 bits
     *  MAX_CHANNELS * (1 +  MAX_BANDS * 23)
     */


        /** resample scale factors for the new block size
         *  as the scale factors might need to be resampled several times
         *  before some  new values are transmitted, a backup of the last
         *  transmitted scale factors is kept in saved_scale_factors
         */
        }


                /** decode DPCM coded scale factors */
                }
            } else {
                int i;
                /** run level decode differences to the resampled factors */


                        break;
                    } else {
                    }

                        av_log(s->avctx, AV_LOG_ERROR,
                               "invalid scale factor coding\n");
                        return AVERROR_INVALIDDATA;
                    }
                }
            }
            /** swap buffers */
        }

        /** calculate new scale factor maximum */
        }

    }
    return 0;
}

/**
 *@brief Reconstruct the individual channel data.
 *@param s codec context
 */
{


            /** multichannel decorrelation */
                    /** multiply values with the decorrelation_matrix */


                            float sum = 0;
                            data_ptr = data;

                        }
                    }
                } else if (s->nb_channels == 2) {
                    int len = FFMIN(sfb[1], s->subframe_len) - sfb[0];
                    s->fdsp->vector_fmul_scalar(ch_data[0] + sfb[0],
                                               ch_data[0] + sfb[0],
                                               181.0 / 128, len);
                    s->fdsp->vector_fmul_scalar(ch_data[1] + sfb[0],
                                               ch_data[1] + sfb[0],
                                               181.0 / 128, len);
                }
            }
        }
    }

/**
 *@brief Apply sine window and reconstruct the output buffer.
 *@param s codec context
 */
{

        }



                                   window, winlen);

    }

/**
 *@brief Decode a single subframe (block).
 *@param s codec context
 *@return 0 on success, < 0 when decoding failed
 */
{


    /** reset channel context and find the next block offset and size
        == the next block of the channel with the smallest number of
        decoded samples
    */
        }
    }

            "processing subframe with offset %i len %i\n", offset, subframe_len);

    /** get a list of all channels that contain the estimated block */
        /** subtract already processed samples */

        /** and count if there are multiple subframes that match our profile */
                s->channel[i].subframe_len[cur_subframe];
        }
    }

    /** check if the frame will be complete after processing the
        estimated block */


            s->channels_for_cur_subframe);

    /** calculate number of scale factor bands and their offsets */

    /** configure the decoder for the current subframe */


    }


    /** skip extended header if any */
        }

                av_log(s->avctx, AV_LOG_ERROR, "invalid number of fill bits\n");
                return AVERROR_INVALIDDATA;
            }

        }
    }

    /** no idea for what the following bit is used */
        avpriv_request_sample(s->avctx, "Reserved bit");
        return AVERROR_PATCHWELCOME;
    }


        return AVERROR_INVALIDDATA;


    }


        /** decode number of vector coded coefficients */
            int num_bits = av_log2((s->subframe_len + 3)/4) + 1;
            for (i = 0; i < s->channels_for_cur_subframe; i++) {
                int c = s->channel_indexes_for_cur_subframe[i];
                int num_vec_coeffs = get_bits(&s->gb, num_bits) << 2;
                if (num_vec_coeffs > s->subframe_len) {
                    av_log(s->avctx, AV_LOG_ERROR, "num_vec_coeffs %d is too large\n", num_vec_coeffs);
                    return AVERROR_INVALIDDATA;
                }
                av_assert0(num_vec_coeffs + offset <= FF_ARRAY_ELEMS(s->channel[c].out));
                s->channel[c].num_vec_coeffs = num_vec_coeffs;
            }
        } else {
            }
        }
        /** decode quantization step */
                quant += 31;
            }
        }
            av_log(s->avctx, AV_LOG_DEBUG, "negative quant step\n");
        }

        /** decode quantization step modifiers for every channel */

        } else {
                    } else
                }
            }
        }

        /** decode scale factors */
            return AVERROR_INVALIDDATA;
    }

    ff_dlog(s->avctx, "BITSTREAM: subframe header length was %i\n",
            get_bits_count(&s->gb) - s->subframe_offset);

    /** parse coefficients */
        } else
                   sizeof(*s->channel[c].coeffs) * subframe_len);
    }

            get_bits_count(&s->gb) - s->subframe_offset);

        /** reconstruct the per channel data */


            /** inverse quantization and rescaling */
                                           quant, end - start);
            }

            /** apply imdct (imdct_half == DCTIV with reverse) */
        }
    }

    /** window and overlapp-add */

    /** handled one subframe */
            av_log(s->avctx, AV_LOG_ERROR, "broken subframe\n");
            return AVERROR_INVALIDDATA;
        }
    }

    return 0;
}

/**
 *@brief Decode one WMA frame.
 *@param s codec context
 *@return 0 if the trailer bit indicates that this is the last frame,
 *        1 if there are additional frames
 */
{

    /** get frame length */


    /** decode tile information */
        s->packet_loss = 1;
        return 0;
    }

    /** read postproc transform */
        if (get_bits1(gb)) {
            for (i = 0; i < s->nb_channels * s->nb_channels; i++)
                skip_bits(gb, 4);
        }
    }

    /** read drc info */
    }

    /** no idea what these are for, might be the number of samples
        that need to be skipped at the beginning or end of a stream */

        /** usually true for the first frame */
        }

        /** sometimes true for the last frame */
        }

    }

            get_bits_count(gb) - s->frame_offset);

    /** reset subframe states */
    }

    /** decode all subframes */
            s->packet_loss = 1;
            return 0;
        }
    }

    /** copy samples to the output buffer */

        /** reuse second half of the IMDCT output for the next frame */
    }

    } else {
    }

            /** FIXME: not sure if this is always an error */
            av_log(s->avctx, AV_LOG_ERROR,
                   "frame[%"PRIu32"] would have to skip %i bits\n",
                   s->frame_num,
                   len - (get_bits_count(gb) - s->frame_offset) - 1);
            s->packet_loss = 1;
            return 0;
        }

        /** skip the rest of the frame data */
    } else {
        while (get_bits_count(gb) < s->num_saved_bits && get_bits1(gb) == 0) {
        }
    }

    /** decode trailer bit */

}

/**
 *@brief Calculate remaining input buffer length.
 *@param s codec context
 *@param gb bitstream reader context
 *@return remaining size in bits
 */
static int remaining_bits(WMAProDecodeCtx *s, GetBitContext *gb)
{
    return s->buf_bit_size - get_bits_count(gb);
}

/**
 *@brief Fill the bit reservoir with a (partial) frame.
 *@param s codec context
 *@param gb bitstream reader context
 *@param len length of the partial frame
 *@param append decides whether to reset the buffer or not
 */
                      int append)
{

    /** when the frame data does not need to be concatenated, the input buffer
        is reset and additional bits from the previous frame are copied
        and skipped later so that a fast byte copy is possible */

    } else

        avpriv_request_sample(s->avctx, "Too small input buffer");
        s->packet_loss = 1;
        return;
    }


                     s->num_saved_bits);
    } else {
    }

    {
    }

}

static int decode_packet(AVCodecContext *avctx, WMAProDecodeCtx *s,
                         void *data, int *got_frame_ptr, AVPacket *avpkt)
{
    GetBitContext* gb  = &s->pgb;
    const uint8_t* buf = avpkt->data;
    int buf_size       = avpkt->size;
    int num_bits_prev_frame;
    int packet_sequence_number;

    *got_frame_ptr = 0;

    if (!buf_size) {
        AVFrame *frame = data;
        int i;

        /** Must output remaining samples after stream end. WMAPRO 5.1 created
         * by XWMA encoder don't though (maybe only 1/2ch streams need it). */
        s->packet_done = 0;
        if (s->eof_done)
            return 0;

        /** clean output buffer and copy last IMDCT samples */
        for (i = 0; i < s->nb_channels; i++) {
            memset(frame->extended_data[i], 0,
            s->samples_per_frame * sizeof(*s->channel[i].out));

            memcpy(frame->extended_data[i], s->channel[i].out,
                   s->samples_per_frame * sizeof(*s->channel[i].out) >> 1);
        }

        /* TODO: XMA should output 128 samples only (instead of 512) and WMAPRO
         * maybe 768 (with 2048), XMA needs changes in multi-stream handling though. */

        s->eof_done = 1;
        s->packet_done = 1;
        *got_frame_ptr = 1;
        return 0;
    }
    else if (s->packet_done || s->packet_loss) {
        s->packet_done = 0;

        /** sanity check for the buffer length */
        if (avctx->codec_id == AV_CODEC_ID_WMAPRO && buf_size < avctx->block_align) {
            av_log(avctx, AV_LOG_ERROR, "Input packet too small (%d < %d)\n",
                   buf_size, avctx->block_align);
            s->packet_loss = 1;
            return AVERROR_INVALIDDATA;
        }

        if (avctx->codec_id == AV_CODEC_ID_WMAPRO) {
            s->next_packet_start = buf_size - avctx->block_align;
            buf_size = avctx->block_align;
        } else {
            s->next_packet_start = buf_size - FFMIN(buf_size, avctx->block_align);
            buf_size = FFMIN(buf_size, avctx->block_align);
        }
        s->buf_bit_size = buf_size << 3;

        /** parse packet header */
        init_get_bits(gb, buf, s->buf_bit_size);
        if (avctx->codec_id != AV_CODEC_ID_XMA2) {
            packet_sequence_number = get_bits(gb, 4);
            skip_bits(gb, 2);
        } else {
            int num_frames = get_bits(gb, 6);
            ff_dlog(avctx, "packet[%d]: number of frames %d\n", avctx->frame_number, num_frames);
            packet_sequence_number = 0;
        }

        /** get number of bits that need to be added to the previous frame */
        num_bits_prev_frame = get_bits(gb, s->log2_frame_size);
        if (avctx->codec_id != AV_CODEC_ID_WMAPRO) {
            skip_bits(gb, 3);
            s->skip_packets = get_bits(gb, 8);
            ff_dlog(avctx, "packet[%d]: skip packets %d\n", avctx->frame_number, s->skip_packets);
        }

        ff_dlog(avctx, "packet[%d]: nbpf %x\n", avctx->frame_number,
                num_bits_prev_frame);

        /** check for packet loss */
        if (avctx->codec_id == AV_CODEC_ID_WMAPRO && !s->packet_loss &&
            ((s->packet_sequence_number + 1) & 0xF) != packet_sequence_number) {
            s->packet_loss = 1;
            av_log(avctx, AV_LOG_ERROR,
                   "Packet loss detected! seq %"PRIx8" vs %x\n",
                   s->packet_sequence_number, packet_sequence_number);
        }
        s->packet_sequence_number = packet_sequence_number;

        if (num_bits_prev_frame > 0) {
            int remaining_packet_bits = s->buf_bit_size - get_bits_count(gb);
            if (num_bits_prev_frame >= remaining_packet_bits) {
                num_bits_prev_frame = remaining_packet_bits;
                s->packet_done = 1;
            }

            /** append the previous frame data to the remaining data from the
                previous packet to create a full frame */
            save_bits(s, gb, num_bits_prev_frame, 1);
            ff_dlog(avctx, "accumulated %x bits of frame data\n",
                    s->num_saved_bits - s->frame_offset);

            /** decode the cross packet frame if it is valid */
            if (!s->packet_loss)
                decode_frame(s, data, got_frame_ptr);
        } else if (s->num_saved_bits - s->frame_offset) {
            ff_dlog(avctx, "ignoring %x previously saved bits\n",
                    s->num_saved_bits - s->frame_offset);
        }

        if (s->packet_loss) {
            /** reset number of saved bits so that the decoder
                does not start to decode incomplete frames in the
                s->len_prefix == 0 case */
            s->num_saved_bits = 0;
            s->packet_loss = 0;
        }
    } else {
        int frame_size;
        s->buf_bit_size = (avpkt->size - s->next_packet_start) << 3;
        init_get_bits(gb, avpkt->data, s->buf_bit_size);
        skip_bits(gb, s->packet_offset);
        if (s->len_prefix && remaining_bits(s, gb) > s->log2_frame_size &&
            (frame_size = show_bits(gb, s->log2_frame_size)) &&
            frame_size <= remaining_bits(s, gb)) {
            save_bits(s, gb, frame_size, 0);
            if (!s->packet_loss)
                s->packet_done = !decode_frame(s, data, got_frame_ptr);
        } else if (!s->len_prefix
                   && s->num_saved_bits > get_bits_count(&s->gb)) {
            /** when the frames do not have a length prefix, we don't know
                the compressed length of the individual frames
                however, we know what part of a new packet belongs to the
                previous frame
                therefore we save the incoming packet first, then we append
                the "previous frame" data from the next packet so that
                we get a buffer that only contains full frames */
            s->packet_done = !decode_frame(s, data, got_frame_ptr);
        } else {
            s->packet_done = 1;
        }
    }

    if (remaining_bits(s, gb) < 0) {
        av_log(avctx, AV_LOG_ERROR, "Overread %d\n", -remaining_bits(s, gb));
        s->packet_loss = 1;
    }

    if (s->packet_done && !s->packet_loss &&
        remaining_bits(s, gb) > 0) {
        /** save the rest of the data so that it can be decoded
            with the next packet */
        save_bits(s, gb, remaining_bits(s, gb), 0);
    }

    s->packet_offset = get_bits_count(gb) & 7;
    if (s->packet_loss)
        return AVERROR_INVALIDDATA;

    return get_bits_count(gb) >> 3;
}

/**
 *@brief Decode a single WMA packet.
 *@param avctx codec context
 *@param data the output buffer
 *@param avpkt input packet
 *@return number of bytes that were read from the input buffer
 */
                                int *got_frame_ptr, AVPacket *avpkt)
{

    /* get output buffer */
        s->packet_loss = 1;
        return 0;
    }

}

static int xma_decode_packet(AVCodecContext *avctx, void *data,
                             int *got_frame_ptr, AVPacket *avpkt)
{
    XMADecodeCtx *s = avctx->priv_data;
    int got_stream_frame_ptr = 0;
    AVFrame *frame = data;
    int i, ret, offset = INT_MAX;

    if (!s->frames[s->current_stream]->data[0]) {
        s->frames[s->current_stream]->nb_samples = 512;
        if ((ret = ff_get_buffer(avctx, s->frames[s->current_stream], 0)) < 0) {
            return ret;
        }
    }
    /* decode current stream packet */
    ret = decode_packet(avctx, &s->xma[s->current_stream], s->frames[s->current_stream],
                        &got_stream_frame_ptr, avpkt);

    if (got_stream_frame_ptr && s->offset[s->current_stream] >= 64) {
        got_stream_frame_ptr = 0;
        ret = AVERROR_INVALIDDATA;
    }

    /* copy stream samples (1/2ch) to sample buffer (Nch) */
    if (got_stream_frame_ptr) {
        int start_ch = s->start_channel[s->current_stream];
        memcpy(&s->samples[start_ch + 0][s->offset[s->current_stream] * 512],
               s->frames[s->current_stream]->extended_data[0], 512 * 4);
        if (s->xma[s->current_stream].nb_channels > 1)
            memcpy(&s->samples[start_ch + 1][s->offset[s->current_stream] * 512],
                   s->frames[s->current_stream]->extended_data[1], 512 * 4);
        s->offset[s->current_stream]++;
    } else if (ret < 0) {
        memset(s->offset, 0, sizeof(s->offset));
        s->current_stream = 0;
        return ret;
    }

    /* find next XMA packet's owner stream, and update.
     * XMA streams find their packets following packet_skips
     * (at start there is one packet per stream, then interleave non-linearly). */
    if (s->xma[s->current_stream].packet_done ||
        s->xma[s->current_stream].packet_loss) {

        /* select stream with 0 skip_packets (= uses next packet) */
        if (s->xma[s->current_stream].skip_packets != 0) {
            int min[2];

            min[0] = s->xma[0].skip_packets;
            min[1] = i = 0;

            for (i = 1; i < s->num_streams; i++) {
                if (s->xma[i].skip_packets < min[0]) {
                    min[0] = s->xma[i].skip_packets;
                    min[1] = i;
                }
            }

            s->current_stream = min[1];
        }

        /* all other streams skip next packet */
        for (i = 0; i < s->num_streams; i++) {
            s->xma[i].skip_packets = FFMAX(0, s->xma[i].skip_packets - 1);
        }

        /* copy samples from buffer to output if possible */
        for (i = 0; i < s->num_streams; i++) {
            offset = FFMIN(offset, s->offset[i]);
        }
        if (offset > 0) {
            int bret;

            frame->nb_samples = 512 * offset;
            if ((bret = ff_get_buffer(avctx, frame, 0)) < 0)
                return bret;

            /* copy samples buffer (Nch) to frame samples (Nch), move unconsumed samples */
            for (i = 0; i < s->num_streams; i++) {
                int start_ch = s->start_channel[i];
                memcpy(frame->extended_data[start_ch + 0], s->samples[start_ch + 0], frame->nb_samples * 4);
                if (s->xma[i].nb_channels > 1)
                    memcpy(frame->extended_data[start_ch + 1], s->samples[start_ch + 1], frame->nb_samples * 4);

                s->offset[i] -= offset;
                if (s->offset[i]) {
                    memmove(s->samples[start_ch + 0], s->samples[start_ch + 0] + frame->nb_samples, s->offset[i] * 4 * 512);
                    if (s->xma[i].nb_channels > 1)
                        memmove(s->samples[start_ch + 1], s->samples[start_ch + 1] + frame->nb_samples, s->offset[i] * 4 * 512);
                }
            }

            *got_frame_ptr = 1;
        }
    }

    return ret;
}

static av_cold int xma_decode_init(AVCodecContext *avctx)
{
    XMADecodeCtx *s = avctx->priv_data;
    int i, ret, start_channels = 0;

    if (avctx->channels <= 0 || avctx->extradata_size == 0)
        return AVERROR_INVALIDDATA;

    /* get stream config */
    if (avctx->codec_id == AV_CODEC_ID_XMA2 && avctx->extradata_size == 34) { /* XMA2WAVEFORMATEX */
        s->num_streams = (avctx->channels + 1) / 2;
    } else if (avctx->codec_id == AV_CODEC_ID_XMA2 && avctx->extradata_size >= 2) { /* XMA2WAVEFORMAT */
        s->num_streams = avctx->extradata[1];
        if (avctx->extradata_size != (32 + ((avctx->extradata[0]==3)?0:8) + 4*s->num_streams)) {
            av_log(avctx, AV_LOG_ERROR, "Incorrect XMA2 extradata size\n");
            s->num_streams = 0;
            return AVERROR(EINVAL);
        }
    } else if (avctx->codec_id == AV_CODEC_ID_XMA1 && avctx->extradata_size >= 4) { /* XMAWAVEFORMAT */
        s->num_streams = avctx->extradata[4];
        if (avctx->extradata_size != (8 + 20*s->num_streams)) {
            av_log(avctx, AV_LOG_ERROR, "Incorrect XMA1 extradata size\n");
            s->num_streams = 0;
            return AVERROR(EINVAL);
        }
    } else {
        av_log(avctx, AV_LOG_ERROR, "Incorrect XMA config\n");
        return AVERROR(EINVAL);
    }

    /* encoder supports up to 64 streams / 64*2 channels (would have to alloc arrays) */
    if (avctx->channels > XMA_MAX_CHANNELS || s->num_streams > XMA_MAX_STREAMS ||
        s->num_streams <= 0
    ) {
        avpriv_request_sample(avctx, "More than %d channels in %d streams", XMA_MAX_CHANNELS, s->num_streams);
        s->num_streams = 0;
        return AVERROR_PATCHWELCOME;
    }

    /* init all streams (several streams of 1/2ch make Nch files) */
    for (i = 0; i < s->num_streams; i++) {
        ret = decode_init(&s->xma[i], avctx, i);
        if (ret < 0)
            return ret;
        s->frames[i] = av_frame_alloc();
        if (!s->frames[i])
            return AVERROR(ENOMEM);

        s->start_channel[i] = start_channels;
        start_channels += s->xma[i].nb_channels;
    }
    if (start_channels != avctx->channels)
        return AVERROR_INVALIDDATA;

    return ret;
}

static av_cold int xma_decode_end(AVCodecContext *avctx)
{
    XMADecodeCtx *s = avctx->priv_data;
    int i;

    for (i = 0; i < s->num_streams; i++) {
        decode_end(&s->xma[i]);
        av_frame_free(&s->frames[i]);
    }
    s->num_streams = 0;

    return 0;
}

static void flush(WMAProDecodeCtx *s)
{
    int i;
    /** reset output buffer as a part of it is used during the windowing of a
        new frame */
    for (i = 0; i < s->nb_channels; i++)
        memset(s->channel[i].out, 0, s->samples_per_frame *
               sizeof(*s->channel[i].out));
    s->packet_loss = 1;
    s->skip_packets = 0;
    s->eof_done = 0;
}


/**
 *@brief Clear decoder buffers (for seeking).
 *@param avctx codec context
 */
static void wmapro_flush(AVCodecContext *avctx)
{
    WMAProDecodeCtx *s = avctx->priv_data;

    flush(s);
}

static void xma_flush(AVCodecContext *avctx)
{
    XMADecodeCtx *s = avctx->priv_data;
    int i;

    for (i = 0; i < s->num_streams; i++)
        flush(&s->xma[i]);

    memset(s->offset, 0, sizeof(s->offset));
    s->current_stream = 0;
}


/**
 *@brief wmapro decoder
 */
AVCodec ff_wmapro_decoder = {
    .name           = "wmapro",
    .long_name      = NULL_IF_CONFIG_SMALL("Windows Media Audio 9 Professional"),
    .type           = AVMEDIA_TYPE_AUDIO,
    .id             = AV_CODEC_ID_WMAPRO,
    .priv_data_size = sizeof(WMAProDecodeCtx),
    .init           = wmapro_decode_init,
    .close          = wmapro_decode_end,
    .decode         = wmapro_decode_packet,
    .capabilities   = AV_CODEC_CAP_SUBFRAMES | AV_CODEC_CAP_DR1,
    .caps_internal  = FF_CODEC_CAP_INIT_CLEANUP,
    .flush          = wmapro_flush,
    .sample_fmts    = (const enum AVSampleFormat[]) { AV_SAMPLE_FMT_FLTP,
                                                      AV_SAMPLE_FMT_NONE },
};

AVCodec ff_xma1_decoder = {
    .name           = "xma1",
    .long_name      = NULL_IF_CONFIG_SMALL("Xbox Media Audio 1"),
    .type           = AVMEDIA_TYPE_AUDIO,
    .id             = AV_CODEC_ID_XMA1,
    .priv_data_size = sizeof(XMADecodeCtx),
    .init           = xma_decode_init,
    .close          = xma_decode_end,
    .decode         = xma_decode_packet,
    .capabilities   = AV_CODEC_CAP_SUBFRAMES | AV_CODEC_CAP_DR1 | AV_CODEC_CAP_DELAY,
    .caps_internal  = FF_CODEC_CAP_INIT_CLEANUP,
    .sample_fmts    = (const enum AVSampleFormat[]) { AV_SAMPLE_FMT_FLTP,
                                                      AV_SAMPLE_FMT_NONE },
};

AVCodec ff_xma2_decoder = {
    .name           = "xma2",
    .long_name      = NULL_IF_CONFIG_SMALL("Xbox Media Audio 2"),
    .type           = AVMEDIA_TYPE_AUDIO,
    .id             = AV_CODEC_ID_XMA2,
    .priv_data_size = sizeof(XMADecodeCtx),
    .init           = xma_decode_init,
    .close          = xma_decode_end,
    .decode         = xma_decode_packet,
    .flush          = xma_flush,
    .capabilities   = AV_CODEC_CAP_SUBFRAMES | AV_CODEC_CAP_DR1 | AV_CODEC_CAP_DELAY,
    .caps_internal  = FF_CODEC_CAP_INIT_CLEANUP,
    .sample_fmts    = (const enum AVSampleFormat[]) { AV_SAMPLE_FMT_FLTP,
                                                      AV_SAMPLE_FMT_NONE },
};
