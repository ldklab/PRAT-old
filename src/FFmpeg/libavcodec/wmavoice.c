/*
 * Windows Media Audio Voice decoder.
 * Copyright (c) 2009 Ronald S. Bultje
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
 * @brief Windows Media Audio Voice compatible decoder
 * @author Ronald S. Bultje <rsbultje@gmail.com>
 */

#include <math.h>

#include "libavutil/channel_layout.h"
#include "libavutil/float_dsp.h"
#include "libavutil/mem.h"
#include "libavutil/thread.h"
#include "avcodec.h"
#include "internal.h"
#include "get_bits.h"
#include "put_bits.h"
#include "wmavoice_data.h"
#include "celp_filters.h"
#include "acelp_vectors.h"
#include "acelp_filters.h"
#include "lsp.h"
#include "dct.h"
#include "rdft.h"
#include "sinewin.h"

#define MAX_BLOCKS           8   ///< maximum number of blocks per frame
#define MAX_LSPS             16  ///< maximum filter order
#define MAX_LSPS_ALIGN16     16  ///< same as #MAX_LSPS; needs to be multiple
                                 ///< of 16 for ASM input buffer alignment
#define MAX_FRAMES           3   ///< maximum number of frames per superframe
#define MAX_FRAMESIZE        160 ///< maximum number of samples per frame
#define MAX_SIGNAL_HISTORY   416 ///< maximum excitation signal history
#define MAX_SFRAMESIZE       (MAX_FRAMESIZE * MAX_FRAMES)
                                 ///< maximum number of samples per superframe
#define SFRAME_CACHE_MAXSIZE 256 ///< maximum cache size for frame data that
                                 ///< was split over two packets
#define VLC_NBITS            6   ///< number of bits to read per VLC iteration

/**
 * Frame type VLC coding.
 */
static VLC frame_type_vlc;

/**
 * Adaptive codebook types.
 */
enum {
    ACB_TYPE_NONE       = 0, ///< no adaptive codebook (only hardcoded fixed)
    ACB_TYPE_ASYMMETRIC = 1, ///< adaptive codebook with per-frame pitch, which
                             ///< we interpolate to get a per-sample pitch.
                             ///< Signal is generated using an asymmetric sinc
                             ///< window function
                             ///< @note see #wmavoice_ipol1_coeffs
    ACB_TYPE_HAMMING    = 2  ///< Per-block pitch with signal generation using
                             ///< a Hamming sinc window function
                             ///< @note see #wmavoice_ipol2_coeffs
};

/**
 * Fixed codebook types.
 */
enum {
    FCB_TYPE_SILENCE    = 0, ///< comfort noise during silence
                             ///< generated from a hardcoded (fixed) codebook
                             ///< with per-frame (low) gain values
    FCB_TYPE_HARDCODED  = 1, ///< hardcoded (fixed) codebook with per-block
                             ///< gain values
    FCB_TYPE_AW_PULSES  = 2, ///< Pitch-adaptive window (AW) pulse signals,
                             ///< used in particular for low-bitrate streams
    FCB_TYPE_EXC_PULSES = 3, ///< Innovation (fixed) codebook pulse sets in
                             ///< combinations of either single pulses or
                             ///< pulse pairs
};

/**
 * Description of frame types.
 */
static const struct frame_type_desc {
    uint8_t n_blocks;     ///< amount of blocks per frame (each block
                          ///< (contains 160/#n_blocks samples)
    uint8_t log_n_blocks; ///< log2(#n_blocks)
    uint8_t acb_type;     ///< Adaptive codebook type (ACB_TYPE_*)
    uint8_t fcb_type;     ///< Fixed codebook type (FCB_TYPE_*)
    uint8_t dbl_pulses;   ///< how many pulse vectors have pulse pairs
                          ///< (rather than just one single pulse)
                          ///< only if #fcb_type == #FCB_TYPE_EXC_PULSES
} frame_descs[17] = {
    { 1, 0, ACB_TYPE_NONE,       FCB_TYPE_SILENCE,    0 },
    { 2, 1, ACB_TYPE_NONE,       FCB_TYPE_HARDCODED,  0 },
    { 2, 1, ACB_TYPE_ASYMMETRIC, FCB_TYPE_AW_PULSES,  0 },
    { 2, 1, ACB_TYPE_ASYMMETRIC, FCB_TYPE_EXC_PULSES, 2 },
    { 2, 1, ACB_TYPE_ASYMMETRIC, FCB_TYPE_EXC_PULSES, 5 },
    { 4, 2, ACB_TYPE_ASYMMETRIC, FCB_TYPE_EXC_PULSES, 0 },
    { 4, 2, ACB_TYPE_ASYMMETRIC, FCB_TYPE_EXC_PULSES, 2 },
    { 4, 2, ACB_TYPE_ASYMMETRIC, FCB_TYPE_EXC_PULSES, 5 },
    { 2, 1, ACB_TYPE_HAMMING,    FCB_TYPE_EXC_PULSES, 0 },
    { 2, 1, ACB_TYPE_HAMMING,    FCB_TYPE_EXC_PULSES, 2 },
    { 2, 1, ACB_TYPE_HAMMING,    FCB_TYPE_EXC_PULSES, 5 },
    { 4, 2, ACB_TYPE_HAMMING,    FCB_TYPE_EXC_PULSES, 0 },
    { 4, 2, ACB_TYPE_HAMMING,    FCB_TYPE_EXC_PULSES, 2 },
    { 4, 2, ACB_TYPE_HAMMING,    FCB_TYPE_EXC_PULSES, 5 },
    { 8, 3, ACB_TYPE_HAMMING,    FCB_TYPE_EXC_PULSES, 0 },
    { 8, 3, ACB_TYPE_HAMMING,    FCB_TYPE_EXC_PULSES, 2 },
    { 8, 3, ACB_TYPE_HAMMING,    FCB_TYPE_EXC_PULSES, 5 }
};

/**
 * WMA Voice decoding context.
 */
typedef struct WMAVoiceContext {
    /**
     * @name Global values specified in the stream header / extradata or used all over.
     * @{
     */
    GetBitContext gb;             ///< packet bitreader. During decoder init,
                                  ///< it contains the extradata from the
                                  ///< demuxer. During decoding, it contains
                                  ///< packet data.
    int8_t vbm_tree[25];          ///< converts VLC codes to frame type

    int spillover_bitsize;        ///< number of bits used to specify
                                  ///< #spillover_nbits in the packet header
                                  ///< = ceil(log2(ctx->block_align << 3))
    int history_nsamples;         ///< number of samples in history for signal
                                  ///< prediction (through ACB)

    /* postfilter specific values */
    int do_apf;                   ///< whether to apply the averaged
                                  ///< projection filter (APF)
    int denoise_strength;         ///< strength of denoising in Wiener filter
                                  ///< [0-11]
    int denoise_tilt_corr;        ///< Whether to apply tilt correction to the
                                  ///< Wiener filter coefficients (postfilter)
    int dc_level;                 ///< Predicted amount of DC noise, based
                                  ///< on which a DC removal filter is used

    int lsps;                     ///< number of LSPs per frame [10 or 16]
    int lsp_q_mode;               ///< defines quantizer defaults [0, 1]
    int lsp_def_mode;             ///< defines different sets of LSP defaults
                                  ///< [0, 1]

    int min_pitch_val;            ///< base value for pitch parsing code
    int max_pitch_val;            ///< max value + 1 for pitch parsing
    int pitch_nbits;              ///< number of bits used to specify the
                                  ///< pitch value in the frame header
    int block_pitch_nbits;        ///< number of bits used to specify the
                                  ///< first block's pitch value
    int block_pitch_range;        ///< range of the block pitch
    int block_delta_pitch_nbits;  ///< number of bits used to specify the
                                  ///< delta pitch between this and the last
                                  ///< block's pitch value, used in all but
                                  ///< first block
    int block_delta_pitch_hrange; ///< 1/2 range of the delta (full range is
                                  ///< from -this to +this-1)
    uint16_t block_conv_table[4]; ///< boundaries for block pitch unit/scale
                                  ///< conversion

    /**
     * @}
     *
     * @name Packet values specified in the packet header or related to a packet.
     *
     * A packet is considered to be a single unit of data provided to this
     * decoder by the demuxer.
     * @{
     */
    int spillover_nbits;          ///< number of bits of the previous packet's
                                  ///< last superframe preceding this
                                  ///< packet's first full superframe (useful
                                  ///< for re-synchronization also)
    int has_residual_lsps;        ///< if set, superframes contain one set of
                                  ///< LSPs that cover all frames, encoded as
                                  ///< independent and residual LSPs; if not
                                  ///< set, each frame contains its own, fully
                                  ///< independent, LSPs
    int skip_bits_next;           ///< number of bits to skip at the next call
                                  ///< to #wmavoice_decode_packet() (since
                                  ///< they're part of the previous superframe)

    uint8_t sframe_cache[SFRAME_CACHE_MAXSIZE + AV_INPUT_BUFFER_PADDING_SIZE];
                                  ///< cache for superframe data split over
                                  ///< multiple packets
    int sframe_cache_size;        ///< set to >0 if we have data from an
                                  ///< (incomplete) superframe from a previous
                                  ///< packet that spilled over in the current
                                  ///< packet; specifies the amount of bits in
                                  ///< #sframe_cache
    PutBitContext pb;             ///< bitstream writer for #sframe_cache

    /**
     * @}
     *
     * @name Frame and superframe values
     * Superframe and frame data - these can change from frame to frame,
     * although some of them do in that case serve as a cache / history for
     * the next frame or superframe.
     * @{
     */
    double prev_lsps[MAX_LSPS];   ///< LSPs of the last frame of the previous
                                  ///< superframe
    int last_pitch_val;           ///< pitch value of the previous frame
    int last_acb_type;            ///< frame type [0-2] of the previous frame
    int pitch_diff_sh16;          ///< ((cur_pitch_val - #last_pitch_val)
                                  ///< << 16) / #MAX_FRAMESIZE
    float silence_gain;           ///< set for use in blocks if #ACB_TYPE_NONE

    int aw_idx_is_ext;            ///< whether the AW index was encoded in
                                  ///< 8 bits (instead of 6)
    int aw_pulse_range;           ///< the range over which #aw_pulse_set1()
                                  ///< can apply the pulse, relative to the
                                  ///< value in aw_first_pulse_off. The exact
                                  ///< position of the first AW-pulse is within
                                  ///< [pulse_off, pulse_off + this], and
                                  ///< depends on bitstream values; [16 or 24]
    int aw_n_pulses[2];           ///< number of AW-pulses in each block; note
                                  ///< that this number can be negative (in
                                  ///< which case it basically means "zero")
    int aw_first_pulse_off[2];    ///< index of first sample to which to
                                  ///< apply AW-pulses, or -0xff if unset
    int aw_next_pulse_off_cache;  ///< the position (relative to start of the
                                  ///< second block) at which pulses should
                                  ///< start to be positioned, serves as a
                                  ///< cache for pitch-adaptive window pulses
                                  ///< between blocks

    int frame_cntr;               ///< current frame index [0 - 0xFFFE]; is
                                  ///< only used for comfort noise in #pRNG()
    int nb_superframes;           ///< number of superframes in current packet
    float gain_pred_err[6];       ///< cache for gain prediction
    float excitation_history[MAX_SIGNAL_HISTORY];
                                  ///< cache of the signal of previous
                                  ///< superframes, used as a history for
                                  ///< signal generation
    float synth_history[MAX_LSPS]; ///< see #excitation_history
    /**
     * @}
     *
     * @name Postfilter values
     *
     * Variables used for postfilter implementation, mostly history for
     * smoothing and so on, and context variables for FFT/iFFT.
     * @{
     */
    RDFTContext rdft, irdft;      ///< contexts for FFT-calculation in the
                                  ///< postfilter (for denoise filter)
    DCTContext dct, dst;          ///< contexts for phase shift (in Hilbert
                                  ///< transform, part of postfilter)
    float sin[511], cos[511];     ///< 8-bit cosine/sine windows over [-pi,pi]
                                  ///< range
    float postfilter_agc;         ///< gain control memory, used in
                                  ///< #adaptive_gain_control()
    float dcf_mem[2];             ///< DC filter history
    float zero_exc_pf[MAX_SIGNAL_HISTORY + MAX_SFRAMESIZE];
                                  ///< zero filter output (i.e. excitation)
                                  ///< by postfilter
    float denoise_filter_cache[MAX_FRAMESIZE];
    int   denoise_filter_cache_size; ///< samples in #denoise_filter_cache
    DECLARE_ALIGNED(32, float, tilted_lpcs_pf)[0x80];
                                  ///< aligned buffer for LPC tilting
    DECLARE_ALIGNED(32, float, denoise_coeffs_pf)[0x80];
                                  ///< aligned buffer for denoise coefficients
    DECLARE_ALIGNED(32, float, synth_filter_out_buf)[0x80 + MAX_LSPS_ALIGN16];
                                  ///< aligned buffer for postfilter speech
                                  ///< synthesis
    /**
     * @}
     */
} WMAVoiceContext;

/**
 * Set up the variable bit mode (VBM) tree from container extradata.
 * @param gb bit I/O context.
 *           The bit context (s->gb) should be loaded with byte 23-46 of the
 *           container extradata (i.e. the ones containing the VBM tree).
 * @param vbm_tree pointer to array to which the decoded VBM tree will be
 *                 written.
 * @return 0 on success, <0 on error.
 */
{

            return -1;
    }
    return 0;
}

{
         2,  2,  2,  4,  4,  4,
         6,  6,  6,  8,  8,  8,
        10, 10, 10, 12, 12, 12,
        14, 14, 14, 14
    };
          0x0000, 0x0001, 0x0002,        //              00/01/10
          0x000c, 0x000d, 0x000e,        //           11+00/01/10
          0x003c, 0x003d, 0x003e,        //         1111+00/01/10
          0x00fc, 0x00fd, 0x00fe,        //       111111+00/01/10
          0x03fc, 0x03fd, 0x03fe,        //     11111111+00/01/10
          0x0ffc, 0x0ffd, 0x0ffe,        //   1111111111+00/01/10
          0x3ffc, 0x3ffd, 0x3ffe, 0x3fff // 111111111111+xx
    };

                    bits, 1, 1, codes, 2, 2, 132);

static av_cold void wmavoice_flush(AVCodecContext *ctx)
{
    WMAVoiceContext *s = ctx->priv_data;
    int n;

    s->postfilter_agc    = 0;
    s->sframe_cache_size = 0;
    s->skip_bits_next    = 0;
    for (n = 0; n < s->lsps; n++)
        s->prev_lsps[n] = M_PI * (n + 1.0) / (s->lsps + 1.0);
    memset(s->excitation_history, 0,
           sizeof(*s->excitation_history) * MAX_SIGNAL_HISTORY);
    memset(s->synth_history,      0,
           sizeof(*s->synth_history)      * MAX_LSPS);
    memset(s->gain_pred_err,      0,
           sizeof(s->gain_pred_err));

    if (s->do_apf) {
        memset(&s->synth_filter_out_buf[MAX_LSPS_ALIGN16 - s->lsps], 0,
               sizeof(*s->synth_filter_out_buf) * s->lsps);
        memset(s->dcf_mem,              0,
               sizeof(*s->dcf_mem)              * 2);
        memset(s->zero_exc_pf,          0,
               sizeof(*s->zero_exc_pf)          * s->history_nsamples);
        memset(s->denoise_filter_cache, 0, sizeof(s->denoise_filter_cache));
    }
}

/**
 * Set up decoder with parameters from demuxer (extradata etc.).
 */
{


    /**
     * Extradata layout:
     * - byte  0-18: WMAPro-in-WMAVoice extradata (see wmaprodec.c),
     * - byte 19-22: flags field (annoyingly in LE; see below for known
     *               values),
     * - byte 23-46: variable bitmode tree (really just 17 * 3 bits,
     *               rest is 0).
     */
        av_log(ctx, AV_LOG_ERROR,
               "Invalid extradata size %d (should be 46)\n",
               ctx->extradata_size);
        return AVERROR_INVALIDDATA;
    }
        av_log(ctx, AV_LOG_ERROR, "Invalid block alignment %d.\n", ctx->block_align);
        return AVERROR_INVALIDDATA;
    }


        }
    }
        av_log(ctx, AV_LOG_ERROR,
               "Invalid denoise filter strength %d (max=11)\n",
               s->denoise_strength);
        return AVERROR_INVALIDDATA;
    }
    } else {
    }

        av_log(ctx, AV_LOG_ERROR, "Invalid VBM tree; broken extradata?\n");
        return AVERROR_INVALIDDATA;
    }

        return AVERROR_INVALIDDATA;

        av_log(ctx, AV_LOG_ERROR, "Invalid pitch range; broken extradata?\n");
        return AVERROR_INVALIDDATA;
    }

        int min_sr = ((((1 << 8) - 50) * 400) + 0xFF) >> 8,
            max_sr = ((((MAX_SIGNAL_HISTORY - 8) << 8) + 205) * 2000 / 37) >> 8;

        av_log(ctx, AV_LOG_ERROR,
               "Unsupported samplerate %d (min=%d, max=%d)\n",
               ctx->sample_rate, min_sr, max_sr); // 322-22097 Hz

        return AVERROR(ENOSYS);
    }

        av_log(ctx, AV_LOG_ERROR, "Invalid delta pitch hrange; broken extradata?\n");
        return AVERROR_INVALIDDATA;
    }


}

/**
 * @name Postfilter functions
 * Postfilter functions (gain control, wiener denoise filter, DC filter,
 * kalman smoothening, plus surrounding code to wrap it)
 * @{
 */
/**
 * Adaptive gain control (as used in postfilter).
 *
 * Identical to #ff_adaptive_gain_control() in acelp_vectors.c, except
 * that the energy here is calculated using sum(abs(...)), whereas the
 * other codecs (e.g. AMR-NB, SIPRO) use sqrt(dotproduct(...)).
 *
 * @param out output buffer for filtered samples
 * @param in input buffer containing the samples as they are after the
 *           postfilter steps so far
 * @param speech_synth input buffer containing speech synth before postfilter
 * @param size input buffer size
 * @param alpha exponential filter factor
 * @param gain_mem pointer to filter memory (single float)
 */
                                  const float *speech_synth,
                                  int size, float alpha, float *gain_mem)
{

    }

    }


/**
 * Kalman smoothing function.
 *
 * This function looks back pitch +/- 3 samples back into history to find
 * the best fitting curve (that one giving the optimal gain of the two
 * signals, i.e. the highest dot product between the two), and then
 * uses that signal history to smoothen the output of the speech synthesis
 * filter.
 *
 * @param s WMA Voice decoding context
 * @param pitch pitch of the speech signal
 * @param in input speech signal
 * @param out output pointer for smoothened signal
 * @param size input/output buffer size
 *
 * @returns -1 if no smoothening took place, e.g. because no optimal
 *          fit could be found, or 0 on success.
 */
static int kalman_smoothen(WMAVoiceContext *s, int pitch,
                           const float *in, float *out, int size)
{
    int n;
    float optimal_gain = 0, dot;
    const float *ptr = &in[-FFMAX(s->min_pitch_val, pitch - 3)],
                *end = &in[-FFMIN(s->max_pitch_val, pitch + 3)],
                *best_hist_ptr = NULL;

    /* find best fitting point in history */
    do {
        dot = avpriv_scalarproduct_float_c(in, ptr, size);
        if (dot > optimal_gain) {
            optimal_gain  = dot;
            best_hist_ptr = ptr;
        }
    } while (--ptr >= end);

    if (optimal_gain <= 0)
        return -1;
    dot = avpriv_scalarproduct_float_c(best_hist_ptr, best_hist_ptr, size);
    if (dot <= 0) // would be 1.0
        return -1;

    if (optimal_gain <= dot) {
        dot = dot / (dot + 0.6 * optimal_gain); // 0.625-1.000
    } else
        dot = 0.625;

    /* actual smoothing */
    for (n = 0; n < size; n++)
        out[n] = best_hist_ptr[n] + dot * (in[n] - best_hist_ptr[n]);

    return 0;
}

/**
 * Get the tilt factor of a formant filter from its transfer function
 * @see #tilt_factor() in amrnbdec.c, which does essentially the same,
 *      but somehow (??) it does a speech synthesis filter in the
 *      middle, which is missing here
 *
 * @param lpcs LPC coefficients
 * @param n_lpcs Size of LPC buffer
 * @returns the tilt factor
 */
{


}

/**
 * Derive denoise filter coefficients (in real domain) from the LPCs.
 */
                                int fcb_type, float *coeffs, int remainder)
{

    /* Create frequency power spectrum of speech input (i.e. RDFT of LPCs) */
#define log_range(var, assign) do { \
        float tmp = log10f(assign);  var = tmp; \
        max       = FFMAX(max, tmp); min = FFMIN(min, tmp); \
    } while (0)
                           lpcs[n * 2 + 1] * lpcs[n * 2 + 1]);
#undef log_range

    /* Now, use this spectrum to pick out these frequencies with higher
     * (relative) power/energy (which we then take to be "not noise"),
     * and set up a table (still in lpc[]) of (relative) gains per frequency.
     * These frequencies will be maintained, while others ("noise") will be
     * decreased in the filter output. */
                                                          (5.0 / 14.7));


        /* 70.57 =~ 1/log10(1.0331663) */

        } else
    }

    /* calculate the Hilbert transform of the gains, which we do (since this
     * is a sine input) by doing a phase shift (in theory, H(sin())=cos()).
     * Hilbert_Transform(RDFT(x)) = Laplace_Transform(x), which calculates the
     * "moment" of the LPCs in this filter. */

    /* Split out the coefficient indexes into phase/magnitude pairs */


    }

    /* move into real domain */

    /* tilt correction and normalize scale */

                             coeffs, remainder);
    }
                                                               remainder));

/**
 * This function applies a Wiener filter on the (noisy) speech signal as
 * a means to denoise it.
 *
 * - take RDFT of LPCs to get the power spectrum of the noise + speech;
 * - using this power spectrum, calculate (for each frequency) the Wiener
 *    filter gain, which depends on the frequency power and desired level
 *    of noise subtraction (when set too high, this leads to artifacts)
 *    We can do this symmetrically over the X-axis (so 0-4kHz is the inverse
 *    of 4-8kHz);
 * - by doing a phase shift, calculate the Hilbert transform of this array
 *    of per-frequency filter-gains to get the filtering coefficients;
 * - smoothen/normalize/de-tilt these filter coefficients as desired;
 * - take RDFT of noisy sound, apply the coefficients and take its IRDFT
 *    to get the denoised speech signal;
 * - the leftover (i.e. output of the IRDFT on denoised speech data beyond
 *    the frame boundary) are saved and applied to subsequent frames by an
 *    overlap-add method (otherwise you get clicking-artifacts).
 *
 * @param s WMA Voice decoding context
 * @param fcb_type Frame (codebook) type
 * @param synth_pf input: the noisy speech signal, output: denoised speech
 *                 data; should be 16-byte aligned (for ASM purposes)
 * @param size size of the speech data
 * @param lpcs LPCs used to synthesize this frame's speech data
 */
                           float *synth_pf, int size,
                           const float *lpcs)
{



        /* The IRDFT output (127 samples for 7-bit filter) beyond the frame
         * size is applied to the next frame. All input beyond this is zero,
         * and thus all output beyond this will go towards zero, hence we can
         * limit to min(size-1, 127-size) as a performance consideration. */

        /* apply coefficients (in frequency spectrum domain), i.e. complex
         * number multiplication */
        }
    }

    /* merge filter output with the history of previous runs */
    }

    /* move remainder of filter output into a cache for future runs */
            s->denoise_filter_cache[n] += synth_pf[size + n];
        }
    }

/**
 * Averaging projection filter, the postfilter used in WMAVoice.
 *
 * This uses the following steps:
 * - A zero-synthesis filter (generate excitation from synth signal)
 * - Kalman smoothing on excitation, based on pitch
 * - Re-synthesized smoothened output
 * - Iterative Wiener denoise filter
 * - Adaptive gain filter
 * - DC filter
 *
 * @param s WMAVoice decoding context
 * @param synth Speech synthesis output (before postfilter)
 * @param samples Output buffer for filtered samples
 * @param size Buffer size of synth & samples
 * @param lpcs Generated LPCs used for speech synthesis
 * @param zero_exc_pf destination for zero synthesis filter (16-byte aligned)
 * @param fcb_type Frame type (silence, hardcoded, AW-pulses or FCB-pulses)
 * @param pitch Pitch of the input signal
 */
                       float *samples,    int size,
                       const float *lpcs, float *zero_exc_pf,
                       int fcb_type,      int pitch)
{


    /* generate excitation from input signal */


    /* re-synthesize speech after smoothening, and keep history */
                                 synth_filter_in, size, s->lsps);


                          &s->postfilter_agc);

        /* remove ultra-low frequency DC noise / highpass filter;
         * coefficients are identical to those used in SIPR decoding,
         * and very closely resemble those used in AMR-NB decoding. */
        ff_acelp_apply_order_2_transfer_function(samples, samples,
            (const float[2]) { -1.99997,      1.0 },
            (const float[2]) { -1.9330735188, 0.93589198496 },
            0.93980580475, s->dcf_mem, size);
    }
/**
 * @}
 */

/**
 * Dequantize LSPs
 * @param lsps output pointer to the array that will hold the LSPs
 * @param num number of LSPs to be dequantized
 * @param values quantized values, contains n_stages values
 * @param sizes range (i.e. max value) of each quantized value
 * @param n_stages number of dequantization runs
 * @param table dequantization table to be used
 * @param mul_q LSF multiplier
 * @param base_q base (lowest) LSF values
 */
                         const uint16_t *values,
                         const uint16_t *sizes,
                         int n_stages, const uint8_t *table,
                         const double *mul_q,
                         const double *base_q)
{



    }

/**
 * @name LSP dequantization routines
 * LSP dequantization routines, for 10/16LSPs and independent/residual coding.
 * lsp10i() consumes 24 bits; lsp10r() consumes an additional 24 bits;
 * lsp16i() consumes 34 bits; lsp16r() consumes an additional 26 bits.
 * @{
 */
/**
 * Parse 10 independently-coded LSPs.
 */
{
        5.2187144800e-3,    1.4626986422e-3,
        9.6179549166e-4,    1.1325736225e-3
    };
        M_PI * -2.15522e-1, M_PI * -6.1646e-2,
        M_PI * -3.3486e-2,  M_PI * -5.7408e-2
    };


                 mul_lsf, base_lsf);

/**
 * Parse 10 independently-coded LSPs, and then derive the tables to
 * generate LSPs for the other frames from them (residual coding).
 */
                           double *i_lsps, const double *old,
                           double *a1, double *a2, int q_mode)
{
        2.5807601174e-3,    1.2354460219e-3,   1.1763821673e-3
    };
        M_PI * -1.07448e-1, M_PI * -5.2706e-2, M_PI * -5.1634e-2
    };



    }

                 mul_lsf, base_lsf);

/**
 * Parse 16 independently-coded LSPs.
 */
{
        3.3439586280e-3,    6.9908173703e-4,
        3.3216608306e-3,    1.0334960326e-3,
        3.1899104283e-3
    };
        M_PI * -1.27576e-1, M_PI * -2.4292e-2,
        M_PI * -1.28094e-1, M_PI * -3.2128e-2,
        M_PI * -1.29816e-1
    };


                 wmavoice_dq_lsp16i1,  mul_lsf,     base_lsf);
                 wmavoice_dq_lsp16i2, &mul_lsf[2], &base_lsf[2]);
                 wmavoice_dq_lsp16i3, &mul_lsf[4], &base_lsf[4]);

/**
 * Parse 16 independently-coded LSPs, and then derive the tables to
 * generate LSPs for the other frames from them (residual coding).
 */
                           double *i_lsps, const double *old,
                           double *a1, double *a2, int q_mode)
{
        1.2232979501e-3,   1.4062241527e-3,   1.6114744851e-3
    };
        M_PI * -5.5830e-2, M_PI * -5.2908e-2, M_PI * -5.4776e-2
    };



    }

                 wmavoice_dq_lsp16r1,  mul_lsf,     base_lsf);
                 wmavoice_dq_lsp16r2, &mul_lsf[1], &base_lsf[1]);
                 wmavoice_dq_lsp16r3, &mul_lsf[2], &base_lsf[2]);

/**
 * @}
 * @name Pitch-adaptive window coding functions
 * The next few functions are for pitch-adaptive window coding.
 * @{
 */
/**
 * Parse the offset of the first pitch-adaptive window pulses, and
 * the distribution of pulses between the two blocks in this frame.
 * @param s WMA Voice decoding context private data
 * @param gb bit I/O context
 * @param pitch pitch for each block in this frame
 */
                            const int *pitch)
{
        -11,  -9,  -7,  -5,  -3,  -1,   1,   3,   5,   7,   9,  11,
         13,  15,  18,  17,  19,  20,  21,  22,  23,  24,  25,  26,
         27,  28,  29,  30,  31,  32,  33,  35,  37,  39,  41,  43,
         45,  47,  49,  51,  53,  55,  57,  59,  61,  63,  65,  67,
         69,  71,  73,  75,  77,  79,  81,  83,  85,  87,  89,  91,
         93,  95,  97,  99, 101, 103, 105, 107, 109, 111, 113, 115,
        117, 119, 121, 123, 125, 127, 129, 131, 133, 135, 137, 139,
        141, 143, 145, 147, 149, 151, 153, 155, 157, 159
    };

    /* position of pulse */
    }

    /* for a repeated pulse at pulse_off with a pitch_lag of pitch[], count
     * the distribution of the pulses in each block contained in this frame. */

    /* if continuing from a position before the block, reset position to
     * start of block (when corrected for the range over which it can be
     * spread in aw_pulse_set1()). */
    }

/**
 * Apply second set of pitch-adaptive window pulses.
 * @param s WMA Voice decoding context private data
 * @param gb bit I/O context
 * @param block_idx block index in frame [0, 1]
 * @param fcb structure containing fixed codebook vector info
 * @return -1 on error, 0 otherwise
 */
                         int block_idx, AMRFixed *fcb)
{
    /* in this function, idx is the index in the 80-bit (+ padding) use_mask
     * bit-array. Since use_mask consists of 16-bit values, the lower 4 bits
     * of idx are the position of the bit within a particular item in the
     * array (0 being the most significant bit, and 15 being the least
     * significant bit), and the remainder (>> 4) is the index in the
     * use_mask[]-array. This is faster and uses less memory than using a
     * 80-byte/80-int array. */

    /* set offset of first pulse to within this block */
            pulse_off += fcb->pitch_lag;

    /* find range per pulse */
            range = 32;
        } else /* block_idx = 1 */ {
        }
    } else
        range = 16;

    /* aw_pulse_set1() already applies pulses around pulse_off (to be exactly,
     * in the range of [pulse_off, pulse_off + s->aw_pulse_range], and thus
     * we exclude that range from being pulsed again in this function. */
            } else
        }

    /* find the 'aidx'th offset that is not excluded */
            else if (use_mask[3]) idx = 0x3F;
            else if (use_mask[4]) idx = 0x4F;
            else return -1;
        }
        }
    }


    /* set offset for next block, relative to start of that block */
}

/**
 * Apply first set of pitch-adaptive window pulses.
 * @param s WMA Voice decoding context private data
 * @param gb bit I/O context
 * @param block_idx block index in frame [0, 1]
 * @param fcb storage location for fixed codebook pulse info
 */
                          int block_idx, AMRFixed *fcb)
{


            n_pulses = 3;
            v_mask   = 8;
            i_mask   = 7;
            sh       = 4;
        } else { // 4 pulses, 1:sign + 2:index each
        }

        }
    } else {


    }

/**
 * @}
 *
 * Generate a random number from frame_cntr and block_idx, which will live
 * in the range [0, 1000 - block_size] (so it can be used as an index in a
 * table of size 1000 of which you want to read block_size entries).
 *
 * @param frame_cntr current frame number
 * @param block_num current block index
 * @param block_size amount of entries we want to read from a table
 *                   that has 1000 entries
 * @return a (non-)random number in the [0, 1000 - block_size] range.
 */
static int pRNG(int frame_cntr, int block_num, int block_size)
{
    /* array to simplify the calculation of z:
     * y = (x % 9) * 5 + 6;
     * z = (49995 * x) / y;
     * Since y only has 9 values, we can remove the division by using a
     * LUT and using FASTDIV-style divisions. For each of the 9 values
     * of y, we can rewrite z as:
     * z = x * (49995 / y) + x * ((49995 % y) / y)
     * In this table, each col represents one possible value of y, the
     * first number is 49995 / y, and the second is the FASTDIV variant
     * of 49995 % y / y. */
    static const unsigned int div_tbl[9][2] = {
        { 8332,  3 * 715827883U }, // y =  6
        { 4545,  0 * 390451573U }, // y = 11
        { 3124, 11 * 268435456U }, // y = 16
        { 2380, 15 * 204522253U }, // y = 21
        { 1922, 23 * 165191050U }, // y = 26
        { 1612, 23 * 138547333U }, // y = 31
        { 1388, 27 * 119304648U }, // y = 36
        { 1219, 16 * 104755300U }, // y = 41
        { 1086, 39 *  93368855U }  // y = 46
    };
    unsigned int z, y, x = MUL16(block_num, 1877) + frame_cntr;
    if (x >= 0xFFFF) x -= 0xFFFF;   // max value of x is 8*1877+0xFFFE=0x13AA6,
                                    // so this is effectively a modulo (%)
    y = x - 9 * MULH(477218589, x); // x % 9
    z = (uint16_t) (x * div_tbl[y][0] + UMULH(x, div_tbl[y][1]));
                                    // z = x * 49995 / (y * 5 + 6)
    return z % (1000 - block_size);
}

/**
 * Parse hardcoded signal for a single block.
 * @note see #synth_block().
 */
static void synth_block_hardcoded(WMAVoiceContext *s, GetBitContext *gb,
                                 int block_idx, int size,
                                 const struct frame_type_desc *frame_desc,
                                 float *excitation)
{
    float gain;
    int n, r_idx;

    av_assert0(size <= MAX_FRAMESIZE);

    /* Set the offset from which we start reading wmavoice_std_codebook */
    if (frame_desc->fcb_type == FCB_TYPE_SILENCE) {
        r_idx = pRNG(s->frame_cntr, block_idx, size);
        gain  = s->silence_gain;
    } else /* FCB_TYPE_HARDCODED */ {
        r_idx = get_bits(gb, 8);
        gain  = wmavoice_gain_universal[get_bits(gb, 6)];
    }

    /* Clear gain prediction parameters */
    memset(s->gain_pred_err, 0, sizeof(s->gain_pred_err));

    /* Apply gain to hardcoded codebook and use that as excitation signal */
    for (n = 0; n < size; n++)
        excitation[n] = wmavoice_std_codebook[r_idx + n] * gain;
}

/**
 * Parse FCB/ACB signal for a single block.
 * @note see #synth_block().
 */
                                int block_idx, int size,
                                int block_pitch_sh2,
                                const struct frame_type_desc *frame_desc,
                                float *excitation)
{
        0.8169, -0.06545, 0.1726, 0.0185, -0.0359, 0.0458
    };



    /* For the other frame types, this is where we apply the innovation
     * (fixed) codebook pulses of the speech signal. */
            /* Conceal the block with silence and return.
             * Skip the correct amount of bits to read the next
             * block from the correct offset. */
            int r_idx = pRNG(s->frame_cntr, block_idx, size);

            for (n = 0; n < size; n++)
                excitation[n] =
                    wmavoice_std_codebook[r_idx + n] * s->silence_gain;
            skip_bits(gb, 7 + 1);
            return;
        }
    } else /* FCB_TYPE_EXC_PULSES */ {

        /* similar to ff_decode_10_pulses_35bits(), but with single pulses
         * (instead of double) for a subset of pulses */

            }
        }
    }

    /* Calculate gain for adaptive & fixed codebook signal.
     * see ff_amr_set_fixed_gain(). */
                        -2.9957322736 /* log(0.05) */,
                         1.6094379124 /* log(5.0)  */);


    /* Calculation of adaptive codebook */
        int len;
                } else
                              1, size - n);
            } else
                len = size;

                                  wmavoice_ipol1_coeffs, 17,
                                  idx, 9, len);
        }
    } else /* ACB_TYPE_HAMMING */ {
                                  wmavoice_ipol2_coeffs, 4,
                                  idx, 8, size);
        } else
                              sizeof(float) * size);
    }

    /* Interpolate ACB/FCB and use as excitation signal */
                            acb_gain, fcb_gain, size);
}

/**
 * Parse data in a single block.
 *
 * @param s WMA Voice decoding context private data
 * @param gb bit I/O context
 * @param block_idx index of the to-be-read block
 * @param size amount of samples to be read in this block
 * @param block_pitch_sh2 pitch for this block << 2
 * @param lsps LSPs for (the end of) this frame
 * @param prev_lsps LSPs for the last frame
 * @param frame_desc frame type descriptor
 * @param excitation target memory for the ACB+FCB interpolated signal
 * @param synth target memory for the speech synthesis filter output
 * @return 0 on success, <0 on error.
 */
                        int block_idx, int size,
                        int block_pitch_sh2,
                        const double *lsps, const double *prev_lsps,
                        const struct frame_type_desc *frame_desc,
                        float *excitation, float *synth)
{

    else
                            frame_desc, excitation);

    /* convert interpolated LSPs to LPCs */

    /* Speech synthesis */

/**
 * Synthesize output samples for a single frame.
 *
 * @param ctx WMA Voice decoder context
 * @param gb bit I/O context (s->gb or one for cross-packet superframes)
 * @param frame_idx Frame number within superframe [0-2]
 * @param samples pointer to output sample buffer, has space for at least 160
 *                samples
 * @param lsps LSP array
 * @param prev_lsps array of previous frame's LSPs
 * @param excitation target buffer for excitation signal
 * @param synth target buffer for synthesized speech data
 * @return 0 on success, <0 on error.
 */
                       float *samples,
                       const double *lsps, const double *prev_lsps,
                       float *excitation, float *synth)
{

    /* Parse frame type ("frame header"), see frame_descs */

        av_log(ctx, AV_LOG_ERROR,
               "Invalid frame type VLC code, skipping\n");
        return AVERROR_INVALIDDATA;
    }


    /* Pitch calculation for ACB_TYPE_ASYMMETRIC ("pitch-per-frame") */
        /* Pitch is provided per frame, which is interpreted as the pitch of
         * the last sample of the last block of this frame. We can interpolate
         * the pitch of other blocks (and even pitch-per-sample) by gradually
         * incrementing/decrementing prev_frame_pitch to cur_pitch_val. */

        /* pitch per block */

        }

        /* "pitch-diff-per-sample" for calculation of pitch per sample */
    }

    /* Global gain (if silence) and pitch-adaptive window coordinates */
    }


        /* Pitch calculation for ACB_TYPE_HAMMING ("pitch-per-block") */
            /* Pitch is given per block. Per-block pitches are encoded as an
             * absolute value for the first block, and then delta values
             * relative to this value) for all subsequent blocks. The scale of
             * this pitch value is semi-logarithmic compared to its use in the
             * decoder, so we convert it to normal scale also. */

            } else
            /* Convert last_ so that any next delta is within _range */
                                       s->block_delta_pitch_hrange,

            /* Convert semi-log-style scale back to normal scale */
            } else {
                } else {
                    } else
                        bl_pitch_sh2 = s->block_conv_table[3] << 2;
                }
            }
        }

        }

        default: // ACB_TYPE_NONE has no pitch
            bl_pitch_sh2 = 0;
            break;
        }

                    lsps, prev_lsps, &frame_descs[bd_idx],
                    &excitation[n * block_nsamples],
    }

    /* Averaging projection filter, if applicable. Else, just copy samples
     * from synthesis buffer */
        double i_lsps[MAX_LSPS];
        float lpcs[MAX_LSPS];

                   frame_descs[bd_idx].fcb_type, pitch[0]);

                   frame_descs[bd_idx].fcb_type, pitch[0]);
    } else
        memcpy(samples, synth, 160 * sizeof(synth[0]));

    /* Cache values for next frame */
    }

    return 0;
}

/**
 * Ensure minimum value for first item, maximum value for last value,
 * proper spacing between each value and proper ordering.
 *
 * @param lsps array of LSPs
 * @param num size of LSP array
 *
 * @note basically a double version of #ff_acelp_reorder_lsf(), might be
 *       useful to put in a generic location later on. Parts are also
 *       present in #ff_set_min_dist_lsf() + #ff_sort_nearly_sorted_floats(),
 *       which is in float.
 */
{

    /* set minimum value for first, maximum value for last and minimum
     * spacing between LSF values.
     * Very similar to ff_set_min_dist_lsf(), but in double. */

    /* reorder (looks like one-time / non-recursed bubblesort).
     * Very similar to ff_sort_nearly_sorted_floats(), but in double. */
            for (m = 1; m < num; m++) {
                double tmp = lsps[m];
                for (l = m - 1; l >= 0; l--) {
                    if (lsps[l] <= tmp) break;
                    lsps[l + 1] = lsps[l];
                }
                lsps[l + 1] = tmp;
            }
            break;
        }
    }

/**
 * Synthesize output samples for a single superframe. If we have any data
 * cached in s->sframe_cache, that will be used instead of whatever is loaded
 * in s->gb.
 *
 * WMA Voice superframes contain 3 frames, each containing 160 audio samples,
 * to give a total of 480 samples per frame. See #synth_frame() for frame
 * parsing. In addition to 3 frames, superframes can also contain the LSPs
 * (if these are globally specified for all frames (residually); they can
 * also be specified individually per-frame. See the s->has_residual_lsps
 * option), and can specify the number of samples encoded in this superframe
 * (if less than 480), usually used to prevent blanks at track boundaries.
 *
 * @param ctx WMA Voice decoder context
 * @return 0 on success, <0 on error or 1 if there was not enough data to
 *         fully parse the superframe
 */
                            int *got_frame_ptr)
{


    }

    /* First bit is speech/music bit, it differentiates between WMAVoice
     * speech samples (the actual codec) and WMAVoice music samples, which
     * are really WMAPro-in-WMAVoice-superframes. I've never seen those in
     * the wild yet. */
        avpriv_request_sample(ctx, "WMAPro-in-WMAVoice");
        return AVERROR_PATCHWELCOME;
    }

    /* (optional) nr. of samples in superframe; always <= 480 and >= 0 */
            av_log(ctx, AV_LOG_ERROR,
                   "Superframe encodes > %d samples (%d), not allowed\n",
                   MAX_SFRAMESIZE, n_samples);
            return AVERROR_INVALIDDATA;
        }
    }

    /* Parse LSPs, if global for the superframe (can also be per-frame). */
        double prev_lsps[MAX_LSPS], a1[MAX_LSPS * 2], a2[MAX_LSPS * 2];


        } else /* s->lsps == 16 */

        }
    }

    /* synth_superframe can run multiple times per packet
     * free potential previous frame */

    /* get output buffer */
        return res;

    /* Parse frames, optionally preceded by per-frame (independent) LSPs. */
            int m;

            if (s->lsps == 10) {
                dequant_lsp10i(gb, lsps[n]);
            } else /* s->lsps == 16 */
                dequant_lsp16i(gb, lsps[n]);

            for (m = 0; m < s->lsps; m++)
                lsps[n][m] += mean_lsf[m];
            stabilize_lsps(lsps[n], s->lsps);
        }

            *got_frame_ptr = 0;
            return res;
        }
    }

    /* Statistics? FIXME - we don't check for length, a slight overrun
     * will be caught by internal buffer padding, and anything else
     * will be skipped, not read. */
        res = get_bits(gb, 4);
        skip_bits(gb, 10 * (res + 1));
    }

        wmavoice_flush(ctx);
        return AVERROR_INVALIDDATA;
    }


    /* Update history */
           s->lsps             * sizeof(*synth));
                s->history_nsamples * sizeof(*s->zero_exc_pf));

    return 0;
}

/**
 * Parse the packet header at the start of each packet (input data to this
 * decoder).
 *
 * @param s WMA Voice decoding context private data
 * @return <0 on error, nb_superframes on success.
 */
{

            return AVERROR_INVALIDDATA;

                               // (minus first one if there is spillover)

}

/**
 * Copy (unaligned) bits from gb/data/size to pb.
 *
 * @param pb target buffer to copy bits into
 * @param data source buffer to copy bits from
 * @param size size of the source data, in bytes
 * @param gb bit I/O context specifying the current position in the source.
 *           data. This function might use this to align the bit position to
 *           a whole-byte boundary before calling #avpriv_copy_bits() on aligned
 *           source data
 * @param nbits the amount of bits to copy from source to target
 *
 * @note after calling this function, the current position in the input bit
 *       I/O context is undefined.
 */
                      const uint8_t *data, int size,
                      GetBitContext *gb, int nbits)
{

        return;
        return;
}

/**
 * Packet decoding: a packet is anything that the (ASF) demuxer contains,
 * and we expect that the demuxer / application provides it to us as such
 * (else you'll probably get garbage as output). Every packet has a size of
 * ctx->block_align bytes, starts with a packet header (see
 * #parse_packet_header()), and then a series of superframes. Superframe
 * boundaries may exceed packets, i.e. superframes can split data over
 * multiple (two) packets.
 *
 * For more information about frames, see #synth_superframe().
 */
                                  int *got_frame_ptr, AVPacket *avpkt)
{

    /* Packets are sometimes a multiple of ctx->block_align, with a packet
     * header at each ctx->block_align bytes. However, FFmpeg's ASF demuxer
     * feeds us ASF packets, which may concatenate multiple "codec" packets
     * in a single "muxer" packet, so we artificially emulate that by
     * capping the packet size at ctx->block_align. */

    /* size == ctx->block_align is used to indicate whether we are dealing with
     * a new packet or a packet of which we already read the packet header
     * previously. */
        } else {
                return res;
        }

        /* If the packet header specifies a s->spillover_nbits, then we want
         * to push out all data of the previous packet (+ spillover) before
         * continuing to parse new superframes in the current packet. */
                s->spillover_nbits = avpkt->size * 8 - cnt;
            }
            } else
                                get_bits_count(gb)); // resync
            skip_bits_long(gb, s->spillover_nbits);  // resync
        }

    /* Try parsing superframes in current packet */
            return res;
        }
        /* ... cache it for spillover in next packet */
        // FIXME bad - just copy bytes as whole and add use the
        // skip_bits_next field
    }

    return size;
}

{

    }

}

AVCodec ff_wmavoice_decoder = {
    .name             = "wmavoice",
    .long_name        = NULL_IF_CONFIG_SMALL("Windows Media Audio Voice"),
    .type             = AVMEDIA_TYPE_AUDIO,
    .id               = AV_CODEC_ID_WMAVOICE,
    .priv_data_size   = sizeof(WMAVoiceContext),
    .init             = wmavoice_decode_init,
    .close            = wmavoice_decode_end,
    .decode           = wmavoice_decode_packet,
    .capabilities     = AV_CODEC_CAP_SUBFRAMES | AV_CODEC_CAP_DR1 | AV_CODEC_CAP_DELAY,
    .caps_internal    = FF_CODEC_CAP_INIT_CLEANUP,
    .flush            = wmavoice_flush,
};
