/*
 * Copyright (C) 2016 foo86
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

#include "dcaadpcm.h"
#include "dcadec.h"
#include "dcadata.h"
#include "dcahuff.h"
#include "dcamath.h"
#include "dca_syncwords.h"

#if ARCH_ARM
#include "arm/dca.h"
#endif

enum HeaderType {
    HEADER_CORE,
    HEADER_XCH,
    HEADER_XXCH
};

static const int8_t prm_ch_to_spkr_map[DCA_AMODE_COUNT][5] = {
    { DCA_SPEAKER_C,            -1,             -1,             -1,             -1 },
    { DCA_SPEAKER_L, DCA_SPEAKER_R,             -1,             -1,             -1 },
    { DCA_SPEAKER_L, DCA_SPEAKER_R,             -1,             -1,             -1 },
    { DCA_SPEAKER_L, DCA_SPEAKER_R,             -1,             -1,             -1 },
    { DCA_SPEAKER_L, DCA_SPEAKER_R,             -1,             -1,             -1 },
    { DCA_SPEAKER_C, DCA_SPEAKER_L, DCA_SPEAKER_R ,             -1,             -1 },
    { DCA_SPEAKER_L, DCA_SPEAKER_R, DCA_SPEAKER_Cs,             -1,             -1 },
    { DCA_SPEAKER_C, DCA_SPEAKER_L, DCA_SPEAKER_R , DCA_SPEAKER_Cs,             -1 },
    { DCA_SPEAKER_L, DCA_SPEAKER_R, DCA_SPEAKER_Ls, DCA_SPEAKER_Rs,             -1 },
    { DCA_SPEAKER_C, DCA_SPEAKER_L, DCA_SPEAKER_R,  DCA_SPEAKER_Ls, DCA_SPEAKER_Rs }
};

static const uint8_t audio_mode_ch_mask[DCA_AMODE_COUNT] = {
    DCA_SPEAKER_LAYOUT_MONO,
    DCA_SPEAKER_LAYOUT_STEREO,
    DCA_SPEAKER_LAYOUT_STEREO,
    DCA_SPEAKER_LAYOUT_STEREO,
    DCA_SPEAKER_LAYOUT_STEREO,
    DCA_SPEAKER_LAYOUT_3_0,
    DCA_SPEAKER_LAYOUT_2_1,
    DCA_SPEAKER_LAYOUT_3_1,
    DCA_SPEAKER_LAYOUT_2_2,
    DCA_SPEAKER_LAYOUT_5POINT0
};

static const uint8_t block_code_nbits[7] = {
    7, 10, 12, 13, 15, 17, 19
};

{
}

{

}

// 5.3.1 - Bit stream header
{

        switch (err) {
        case DCA_PARSE_ERROR_DEFICIT_SAMPLES:
            av_log(s->avctx, AV_LOG_ERROR, "Deficit samples are not supported\n");
            return h.normal_frame ? AVERROR_INVALIDDATA : AVERROR_PATCHWELCOME;

        case DCA_PARSE_ERROR_PCM_BLOCKS:
            av_log(s->avctx, AV_LOG_ERROR, "Unsupported number of PCM sample blocks (%d)\n", h.npcmblocks);
            return (h.npcmblocks < 6 || h.normal_frame) ? AVERROR_INVALIDDATA : AVERROR_PATCHWELCOME;

        case DCA_PARSE_ERROR_FRAME_SIZE:
            av_log(s->avctx, AV_LOG_ERROR, "Invalid core frame size (%d bytes)\n", h.frame_size);
            return AVERROR_INVALIDDATA;

        case DCA_PARSE_ERROR_AMODE:
            av_log(s->avctx, AV_LOG_ERROR, "Unsupported audio channel arrangement (%d)\n", h.audio_mode);
            return AVERROR_PATCHWELCOME;

        case DCA_PARSE_ERROR_SAMPLE_RATE:
            av_log(s->avctx, AV_LOG_ERROR, "Invalid core audio sampling frequency\n");
            return AVERROR_INVALIDDATA;

        case DCA_PARSE_ERROR_RESERVED_BIT:
            av_log(s->avctx, AV_LOG_ERROR, "Reserved bit set\n");
            return AVERROR_INVALIDDATA;

        case DCA_PARSE_ERROR_LFE_FLAG:
            av_log(s->avctx, AV_LOG_ERROR, "Invalid low frequency effects flag\n");
            return AVERROR_INVALIDDATA;

        case DCA_PARSE_ERROR_PCM_RES:
            av_log(s->avctx, AV_LOG_ERROR, "Invalid source PCM resolution\n");
            return AVERROR_INVALIDDATA;

        default:
            av_log(s->avctx, AV_LOG_ERROR, "Unknown core frame header error\n");
            return AVERROR_INVALIDDATA;
        }
    }


}

// 5.3.2 - Primary audio coding header
{

        return AVERROR_INVALIDDATA;

        // Number of subframes

        // Number of primary audio channels
            av_log(s->avctx, AV_LOG_ERROR, "Invalid number of primary audio channels (%d) for audio channel arrangement (%d)\n", s->nchannels, s->audio_mode);
            return AVERROR_INVALIDDATA;
        }


        // Add LFE channel if present
        break;


        // Channel set header length

        // Check CRC
            av_log(s->avctx, AV_LOG_ERROR, "Invalid XXCH channel set header checksum\n");
            return AVERROR_INVALIDDATA;
        }

        // Number of channels in a channel set
            avpriv_request_sample(s->avctx, "%d XXCH channels", nchannels);
            return AVERROR_PATCHWELCOME;
        }

        // Loudspeaker layout mask

            av_log(s->avctx, AV_LOG_ERROR, "Invalid XXCH speaker layout mask (%#x)\n", s->xxch_spkr_mask);
            return AVERROR_INVALIDDATA;
        }

            av_log(s->avctx, AV_LOG_ERROR, "XXCH speaker layout mask (%#x) overlaps with core (%#x)\n", s->xxch_spkr_mask, s->xxch_core_mask);
            return AVERROR_INVALIDDATA;
        }

        // Combine core and XXCH masks together

        // Downmix coefficients present in stream

            // Downmix already performed by encoder

            // Downmix scale factor
                av_log(s->avctx, AV_LOG_ERROR, "Invalid XXCH downmix scale index (%d)\n", index);
                return AVERROR_INVALIDDATA;
            }

            // Downmix channel mapping mask
                    av_log(s->avctx, AV_LOG_ERROR, "Invalid XXCH downmix channel mapping mask (%#x)\n", mask);
                    return AVERROR_INVALIDDATA;
                }
            }

            // Downmix coefficients
                                av_log(s->avctx, AV_LOG_ERROR, "Invalid XXCH downmix coefficient index (%d)\n", index);
                                return AVERROR_INVALIDDATA;
                            }
                        } else {
                            *coeff_ptr++ = 0;
                        }
                    }
                }
            }
        } else {
            s->xxch_dmix_embedded = 0;
        }

        break;
    }

    // Subband activity count
            av_log(s->avctx, AV_LOG_ERROR, "Invalid subband activity count\n");
            return AVERROR_INVALIDDATA;
        }
    }

    // High frequency VQ start subband

    // Joint intensity coding index
            n += xch_base - 1;
            av_log(s->avctx, AV_LOG_ERROR, "Invalid joint intensity coding index\n");
            return AVERROR_INVALIDDATA;
        }
    }

    // Transient mode code book

    // Scale factor code book
            av_log(s->avctx, AV_LOG_ERROR, "Invalid scale factor code book\n");
            return AVERROR_INVALIDDATA;
        }
    }

    // Bit allocation quantizer select
            av_log(s->avctx, AV_LOG_ERROR, "Invalid bit allocation quantizer select\n");
            return AVERROR_INVALIDDATA;
        }
    }

    // Quantization index codebook select

    // Scale factor adjustment index

        // Reserved
        // Byte align
        // CRC16 of channel set header
            av_log(s->avctx, AV_LOG_ERROR, "Read past end of XXCH channel set header\n");
            return AVERROR_INVALIDDATA;
        }
    } else {
        // Audio header CRC check word
            skip_bits(&s->gb, 16);
    }

    return 0;
}

{

    // Select the root square table
        scale_table = ff_dca_scale_factor_quant7;
        scale_size = FF_ARRAY_ELEMS(ff_dca_scale_factor_quant7);
    } else {
    }

    // If Huffman code was used, the difference of scales was encoded
    else

    // Look up scale factor from the root square table
        av_log(s->avctx, AV_LOG_ERROR, "Invalid scale factor index\n");
        return AVERROR_INVALIDDATA;
    }

}

{

    // Absolute value was encoded even when Huffman code was used
    else
        scale_index = get_bits(&s->gb, sel + 1);

    // Bias by 64

    // Look up joint scale factor
        av_log(s->avctx, AV_LOG_ERROR, "Invalid joint scale factor index\n");
        return AVERROR_INVALIDDATA;
    }

}

// 5.4.1 - Primary audio coding side information
                                 enum HeaderType header, int xch_base)
{

        return AVERROR_INVALIDDATA;

        // Subsubframe count

        // Partial subsubframe sample count
    }

    // Prediction mode

    // Prediction coefficients VQ address

    // Bit allocation index


                abits = dca_get_vlc(&s->gb, &ff_dca_vlc_bit_allocation, sel);
            else

                av_log(s->avctx, AV_LOG_ERROR, "Invalid bit allocation index\n");
                return AVERROR_INVALIDDATA;
            }

        }
    }

    // Transition mode
        // Clear transition mode for all subbands

        // Transient possible only if more than one subsubframe
        }
    }

    // Scale factors

        // Extract scales for subbands up to VQ
                    return ret;
                        return ret;
                }
            } else {
                s->scale_factors[ch][band][0] = 0;
            }
        }

        // High frequency VQ subbands
                return ret;
        }
    }

    // Joint subband codebook select
                av_log(s->avctx, AV_LOG_ERROR, "Invalid joint scale factor code book\n");
                return AVERROR_INVALIDDATA;
            }
        }
    }

    // Scale factors for joint subband coding
                    return ret;
            }
        }
    }

    // Dynamic range coefficient
        skip_bits(&s->gb, 8);

    // Side information CRC check word
        skip_bits(&s->gb, 16);

    return 0;
}

#ifndef decode_blockcodes
{

    }
    }

}
#endif

{
    // Extract block code indices from the bit stream

    // Look up samples from the block code book
        av_log(s->avctx, AV_LOG_ERROR, "Failed to decode block code(s)\n");
        return AVERROR_INVALIDDATA;
    }

    return 0;
}

static inline int parse_huffman_codes(DCACoreDecoder *s, int32_t *audio, int abits, int sel)
{
    int i;

    // Extract Huffman codes from the bit stream

    return 1;
}

{

        // No bits allocated
        memset(audio, 0, DCA_SUBBAND_SAMPLES * sizeof(*audio));
        return 0;
    }

            // Huffman codes
        }
            // Block codes
        }
    }

    // No further encoding
    return 0;
}

                                 const int16_t *vq_index,
                                 const int8_t *prediction_mode,
                                 int sb_start, int sb_end,
                                 int ofs, int len)
{

            }
        }
    }

// 5.5 - Primary audio data arrays
                                int xch_base, int *sub_pos, int *lfe_pos)
{

    // Check number of subband samples in this subframe
        av_log(s->avctx, AV_LOG_ERROR, "Subband sample buffer overflow\n");
        return AVERROR_INVALIDDATA;
    }

        return AVERROR_INVALIDDATA;

    // VQ encoded subbands

            // Extract the VQ address from the bit stream

                                 s->subband_vq_start[ch], s->nsubbands[ch],
        }
    }

    // Low frequency effect data

        // Determine number of LFE samples in this subframe

        // Extract LFE samples from the bit stream

        // Extract scale factor index from the bit stream
            av_log(s->avctx, AV_LOG_ERROR, "Invalid LFE scale factor index\n");
            return AVERROR_INVALIDDATA;
        }

        // Look up the 7-bit root square quantization table

        // Account for quantizer step size which is 0.035

        // Scale and take the LFE samples

        // Advance LFE sample pointer for the next subframe
    }

    // Audio data
                return AVERROR_INVALIDDATA;

            // Not high frequency VQ subbands

                // Extract bits from the bit stream
                    return ret;

                // Select quantization step size table and look up
                // quantization step size
                    step_size = ff_dca_lossless_quant[abits];
                else

                // Identify transient location

                // Determine proper scale factor
                else

                // Adjust scale factor when SEL indicates Huffman code
                }

                           audio, step_size, scale, 0, DCA_SUBBAND_SAMPLES);
            }
        }

        // DSYNC
            av_log(s->avctx, AV_LOG_ERROR, "DSYNC check failed\n");
            return AVERROR_INVALIDDATA;
        }

    }

    // Inverse ADPCM
                      *sub_pos, nsamples);
    }

    // Joint subband coding
        }
    }

    // Advance subband sample pointer for the next subframe
}

static void erase_adpcm_history(DCACoreDecoder *s)
{
    int ch, band;

    // Erase ADPCM history from previous frame if
    // predictor history switch was disabled
    for (ch = 0; ch < DCA_CHANNELS; ch++)
        for (band = 0; band < DCA_SUBBANDS; band++)
            AV_ZERO128(s->subband_samples[ch][band] - DCA_ADPCM_COEFFS);

    emms_c();
}

{

    // Reallocate subband sample buffer
        return AVERROR(ENOMEM);

    }

        erase_adpcm_history(s);

    return 0;
}

{

        return ret;

            return ret;
            return ret;
    }

        // Determine number of active subbands for this channel

        // Update history for ADPCM
        }

        // Clear inactive subbands
        }
    }


}

{

        av_log(s->avctx, AV_LOG_ERROR, "XCH with Cs speaker already present\n");
        return AVERROR_INVALIDDATA;
    }

        return ret;

    // Seek to the end of core frame, don't trust XCH frame size
        av_log(s->avctx, AV_LOG_ERROR, "Read past end of XCH frame\n");
        return AVERROR_INVALIDDATA;
    }

    return 0;
}

{

    // XXCH sync word
        av_log(s->avctx, AV_LOG_ERROR, "Invalid XXCH sync word\n");
        return AVERROR_INVALIDDATA;
    }

    // XXCH frame header length

    // Check XXCH frame header CRC
        av_log(s->avctx, AV_LOG_ERROR, "Invalid XXCH frame header checksum\n");
        return AVERROR_INVALIDDATA;
    }

    // CRC presence flag for channel set header

    // Number of bits for loudspeaker mask
        av_log(s->avctx, AV_LOG_ERROR, "Invalid number of bits for XXCH speaker mask (%d)\n", s->xxch_mask_nbits);
        return AVERROR_INVALIDDATA;
    }

    // Number of channel sets
        avpriv_request_sample(s->avctx, "%d XXCH channel sets", xxch_nchsets);
        return AVERROR_PATCHWELCOME;
    }

    // Channel set 0 data byte size

    // Core loudspeaker activity mask

    // Validate the core mask



        av_log(s->avctx, AV_LOG_ERROR, "XXCH core speaker activity mask (%#x) disagrees with core (%#x)\n", s->xxch_core_mask, mask);
        return AVERROR_INVALIDDATA;
    }

    // Reserved
    // Byte align
    // CRC16 of XXCH frame header
        av_log(s->avctx, AV_LOG_ERROR, "Read past end of XXCH frame header\n");
        return AVERROR_INVALIDDATA;
    }

    // Parse XXCH channel set 0
        return ret;

        av_log(s->avctx, AV_LOG_ERROR, "Read past end of XXCH channel set\n");
        return AVERROR_INVALIDDATA;
    }

    return 0;
}

                              int *xbr_nsubbands, int xbr_transition_mode, int sf, int *sub_pos)
{

    // Check number of subband samples in this subframe
        av_log(s->avctx, AV_LOG_ERROR, "Subband sample buffer overflow\n");
        return AVERROR_INVALIDDATA;
    }

        return AVERROR_INVALIDDATA;

    // Number of bits for XBR bit allocation index

    // XBR bit allocation index
                av_log(s->avctx, AV_LOG_ERROR, "Invalid XBR bit allocation index\n");
                return AVERROR_INVALIDDATA;
            }
        }
    }

    // Number of bits for scale indices
            av_log(s->avctx, AV_LOG_ERROR, "Invalid number of bits for XBR scale factor index\n");
            return AVERROR_INVALIDDATA;
        }
    }

    // XBR scale factors

        // Select the root square table
            scale_table = ff_dca_scale_factor_quant7;
            scale_size = FF_ARRAY_ELEMS(ff_dca_scale_factor_quant7);
        } else {
            scale_table = ff_dca_scale_factor_quant6;
            scale_size = FF_ARRAY_ELEMS(ff_dca_scale_factor_quant6);
        }

        // Parse scale factor indices and look up scale factors from the root
        // square table
                    av_log(s->avctx, AV_LOG_ERROR, "Invalid XBR scale factor index\n");
                    return AVERROR_INVALIDDATA;
                }
                    scale_index = get_bits(&s->gb, xbr_scale_nbits[ch]);
                    if (scale_index >= scale_size) {
                        av_log(s->avctx, AV_LOG_ERROR, "Invalid XBR scale factor index\n");
                        return AVERROR_INVALIDDATA;
                    }
                    xbr_scale_factors[ch][band][1] = scale_table[scale_index];
                }
            }
        }
    }

    // Audio data
                return AVERROR_INVALIDDATA;


                // Extract bits from the bit stream
                    // No further encoding
                    // Block codes
                        return ret;
                } else {
                    // No bits allocated
                }

                // Look up quantization step size

                // Identify transient location
                    trans_ssf = s->transition_mode[sf][ch][band];
                else
                    trans_ssf = 0;

                // Determine proper scale factor
                else
                    scale = xbr_scale_factors[ch][band][1];

                           audio, step_size, scale, 1, DCA_SUBBAND_SAMPLES);
            }
        }

        // DSYNC
            av_log(s->avctx, AV_LOG_ERROR, "XBR-DSYNC check failed\n");
            return AVERROR_INVALIDDATA;
        }

    }

    // Advance subband sample pointer for the next subframe
}

{

    // XBR sync word
        av_log(s->avctx, AV_LOG_ERROR, "Invalid XBR sync word\n");
        return AVERROR_INVALIDDATA;
    }

    // XBR frame header length

    // Check XBR frame header CRC
        av_log(s->avctx, AV_LOG_ERROR, "Invalid XBR frame header checksum\n");
        return AVERROR_INVALIDDATA;
    }

    // Number of channel sets

    // Channel set data byte size

    // Transition mode flag

    // Channel set headers
                av_log(s->avctx, AV_LOG_ERROR, "Invalid number of active XBR subbands (%d)\n", xbr_nsubbands[ch2]);
                return AVERROR_INVALIDDATA;
            }
        }
    }

    // Reserved
    // Byte align
    // CRC16 of XBR frame header
        av_log(s->avctx, AV_LOG_ERROR, "Read past end of XBR frame header\n");
        return AVERROR_INVALIDDATA;
    }

    // Channel set data


                                              xbr_base_ch + xbr_nchannels[i],
                                              xbr_nsubbands, xbr_transition_mode,
                                              sf, &sub_pos)) < 0)
                    return ret;
            }
        }


            av_log(s->avctx, AV_LOG_ERROR, "Read past end of XBR channel set\n");
            return AVERROR_INVALIDDATA;
        }
    }

    return 0;
}

// Modified ISO/IEC 9899 linear congruential generator
// Returns pseudorandom integer in range [-2^30, 2^30 - 1]
static int rand_x96(DCACoreDecoder *s)
{
    s->x96_rand = 1103515245U * s->x96_rand + 12345U;
    return (s->x96_rand & 0x7fffffff) - 0x40000000;
}

{

    // Check number of subband samples in this subframe
        av_log(s->avctx, AV_LOG_ERROR, "Subband sample buffer overflow\n");
        return AVERROR_INVALIDDATA;
    }

        return AVERROR_INVALIDDATA;

    // VQ encoded or unallocated subbands
            // Get the sample pointer and scale factor

            case 0: // No bits allocated for subband
                if (scale <= 1)
                    memset(samples, 0, nsamples * sizeof(int32_t));
                else for (n = 0; n < nsamples; n++)
                    // Generate scaled random samples
                    samples[n] = mul31(rand_x96(s), scale);
                break;

            case 1: // VQ encoded subband
                    // Extract the VQ address from the bit stream and look up
                    // the VQ code book for up to 16 subband samples
                    // Scale and take the samples
                }
                break;
            }
    }

    // Audio data
                return AVERROR_INVALIDDATA;


                // Not VQ encoded or unallocated subbands

                // Extract bits from the bit stream
                    return ret;

                // Select quantization step size table and look up quantization
                // step size
                    step_size = ff_dca_lossless_quant[abits];
                else

                // Get the scale factor

                           audio, step_size, scale, 0, DCA_SUBBAND_SAMPLES);
            }
        }

        // DSYNC
            av_log(s->avctx, AV_LOG_ERROR, "X96-DSYNC check failed\n");
            return AVERROR_INVALIDDATA;
        }

    }

    // Inverse ADPCM
                      *sub_pos, nsamples);
    }

    // Joint subband coding
        }
    }

    // Advance subband sample pointer for the next subframe
}

static void erase_x96_adpcm_history(DCACoreDecoder *s)
{
    int ch, band;

    // Erase ADPCM history from previous frame if
    // predictor history switch was disabled
    for (ch = 0; ch < DCA_CHANNELS; ch++)
        for (band = 0; band < DCA_SUBBANDS_X96; band++)
            AV_ZERO128(s->x96_subband_samples[ch][band] - DCA_ADPCM_COEFFS);

    emms_c();
}

{

    // Reallocate subband sample buffer
                    nframesamples * sizeof(int32_t));
        return AVERROR(ENOMEM);

    }

        erase_x96_adpcm_history(s);

    return 0;
}

{

        return AVERROR_INVALIDDATA;

    // Prediction mode

    // Prediction coefficients VQ address

    // Bit allocation index

            // If Huffman code was used, the difference of abits was encoded
            else

                av_log(s->avctx, AV_LOG_ERROR, "Invalid X96 bit allocation index\n");
                return AVERROR_INVALIDDATA;
            }

        }
    }

    // Scale factors

        // Extract scales for subbands which are transmitted even for
        // unallocated subbands
                return ret;
        }
    }

    // Joint subband codebook select
                av_log(s->avctx, AV_LOG_ERROR, "Invalid X96 joint scale factor code book\n");
                return AVERROR_INVALIDDATA;
            }
        }
    }

    // Scale factors for joint subband coding
                    return ret;
            }
        }
    }

    // Side information CRC check word
        skip_bits(&s->gb, 16);

    return 0;
}

{

        return AVERROR_INVALIDDATA;

        // Channel set header length

        // Check CRC
            av_log(s->avctx, AV_LOG_ERROR, "Invalid X96 channel set header checksum\n");
            return AVERROR_INVALIDDATA;
        }
    }

    // High resolution flag

    // First encoded subband
            av_log(s->avctx, AV_LOG_ERROR, "Invalid X96 subband start index (%d)\n", s->x96_subband_start);
            return AVERROR_INVALIDDATA;
        }
    } else {
        s->x96_subband_start = DCA_SUBBANDS;
    }

    // Subband activity count
            av_log(s->avctx, AV_LOG_ERROR, "Invalid X96 subband activity count (%d)\n", s->nsubbands[ch]);
            return AVERROR_INVALIDDATA;
        }
    }

    // Joint intensity coding index
            av_log(s->avctx, AV_LOG_ERROR, "Invalid X96 joint intensity coding index\n");
            return AVERROR_INVALIDDATA;
        }
    }

    // Scale factor code book
            av_log(s->avctx, AV_LOG_ERROR, "Invalid X96 scale factor code book\n");
            return AVERROR_INVALIDDATA;
        }
    }

    // Bit allocation quantizer select

    // Quantization index codebook select

        // Reserved
        // Byte align
        // CRC16 of channel set header
            av_log(s->avctx, AV_LOG_ERROR, "Read past end of X96 channel set header\n");
            return AVERROR_INVALIDDATA;
        }
    } else {
            skip_bits(&s->gb, 16);
    }

    return 0;
}

{

        return ret;

            return ret;
            return ret;
    }

        // Determine number of active subbands for this channel

        // Update history for ADPCM and clear inactive subbands
            else
        }
    }


}

{

    // Revision number
        av_log(s->avctx, AV_LOG_ERROR, "Invalid X96 revision (%d)\n", s->x96_rev_no);
        return AVERROR_INVALIDDATA;
    }


        return ret;

        return ret;

    // Seek to the end of core frame
        av_log(s->avctx, AV_LOG_ERROR, "Read past end of X96 frame\n");
        return AVERROR_INVALIDDATA;
    }

    return 0;
}

{

    // X96 sync word
        av_log(s->avctx, AV_LOG_ERROR, "Invalid X96 sync word\n");
        return AVERROR_INVALIDDATA;
    }

    // X96 frame header length

    // Check X96 frame header CRC
        av_log(s->avctx, AV_LOG_ERROR, "Invalid X96 frame header checksum\n");
        return AVERROR_INVALIDDATA;
    }

    // Revision number
        av_log(s->avctx, AV_LOG_ERROR, "Invalid X96 revision (%d)\n", s->x96_rev_no);
        return AVERROR_INVALIDDATA;
    }

    // CRC presence flag for channel set header

    // Number of channel sets

    // Channel set data byte size

    // Number of channels in channel set

    // Reserved
    // Byte align
    // CRC16 of X96 frame header
        av_log(s->avctx, AV_LOG_ERROR, "Read past end of X96 frame header\n");
        return AVERROR_INVALIDDATA;
    }

        return ret;

    // Channel set data

                return ret;
        }


            av_log(s->avctx, AV_LOG_ERROR, "Read past end of X96 channel set\n");
            return AVERROR_INVALIDDATA;
        }
    }

    return 0;
}

{

        return AVERROR_INVALIDDATA;

    // Auxiliary data byte count (can't be trusted)

    // 4-byte align

    // Auxiliary data sync word
        av_log(s->avctx, AV_LOG_ERROR, "Invalid auxiliary data sync word\n");
        return AVERROR_INVALIDDATA;
    }


    // Auxiliary decode time stamp flag
        skip_bits_long(&s->gb, 47);

    // Auxiliary dynamic downmix flag

        // Auxiliary primary channel downmix type
            av_log(s->avctx, AV_LOG_ERROR, "Invalid primary channel set downmix type\n");
            return AVERROR_INVALIDDATA;
        }

        // Size of downmix coefficients matrix

        // Dynamic downmix code coefficients
                av_log(s->avctx, AV_LOG_ERROR, "Invalid downmix coefficient index\n");
                return AVERROR_INVALIDDATA;
            }
        }
    }

    // Byte align

    // CRC16 of auxiliary data

    // Check CRC
        av_log(s->avctx, AV_LOG_ERROR, "Invalid auxiliary data checksum\n");
        return AVERROR_INVALIDDATA;
    }

    return 0;
}

{

    // Time code stamp
        skip_bits_long(&s->gb, 32);

    // Auxiliary data
        && (s->avctx->err_recognition & AV_EF_EXPLODE))
        return ret;


    // Core extensions

        // Search for extension sync words aligned on 4-byte boundary. Search
        // must be done backwards from the end of core frame to work around
        // sync word aliasing issues.
                break;

            // The distance between XCH sync word and end of the core frame
            // must be equal to XCH frame size. Off by one error is allowed for
            // compatibility with legacy bitstreams. Minimum XCH frame size is
            // 96 bytes. AMODE and PCHS are further checked to reduce
            // probability of alias sync detection.
                    }
                }
            }

                av_log(s->avctx, AV_LOG_ERROR, "XCH sync word not found\n");
                if (s->avctx->err_recognition & AV_EF_EXPLODE)
                    return AVERROR_INVALIDDATA;
            }
            break;

        case DCA_EXT_AUDIO_X96:
            // The distance between X96 sync word and end of the core frame
            // must be equal to X96 frame size. Minimum X96 frame size is 96
            // bytes.
                    }
                }
            }

                av_log(s->avctx, AV_LOG_ERROR, "X96 sync word not found\n");
                if (s->avctx->err_recognition & AV_EF_EXPLODE)
                    return AVERROR_INVALIDDATA;
            }
            break;

        case DCA_EXT_AUDIO_XXCH:
            if (dca->request_channel_layout)
                break;

            // XXCH frame header CRC must be valid. Minimum XXCH frame header
            // size is 11 bytes.
            for (; sync_pos >= last_pos; sync_pos--, w2 = w1) {
                w1 = AV_RB32(s->gb.buffer + sync_pos * 4);
                if (w1 == DCA_SYNCWORD_XXCH) {
                    size = (w2 >> 26) + 1;
                    dist = s->gb.size_in_bits / 8 - sync_pos * 4;
                    if (size >= 11 && size <= dist &&
                        !av_crc(dca->crctab, 0xffff, s->gb.buffer +
                                (sync_pos + 1) * 4, size - 4)) {
                        s->xxch_pos = sync_pos * 32;
                        break;
                    }
                }
            }

            if (!s->xxch_pos) {
                av_log(s->avctx, AV_LOG_ERROR, "XXCH sync word not found\n");
                if (s->avctx->err_recognition & AV_EF_EXPLODE)
                    return AVERROR_INVALIDDATA;
            }
            break;
        }

    return 0;
}

{


        return ret;

        return ret;
        return ret;
        return ret;
        return ret;

    // Workaround for DTS in WAV
        s->frame_size = size;

        av_log(s->avctx, AV_LOG_ERROR, "Read past end of core frame\n");
        if (s->avctx->err_recognition & AV_EF_EXPLODE)
            return AVERROR_INVALIDDATA;
    }

    return 0;
}

{

    // Parse (X)XCH unless downmixing
                return ret;
            s->gb = s->gb_in;
            skip_bits_long(&s->gb, s->xxch_pos);
            ret = parse_xxch_frame(s);
            ext = DCA_CSS_XXCH;
        }

        // Revert to primary channel set in case (X)XCH parsing fails
            if (avctx->err_recognition & AV_EF_EXPLODE)
                return ret;
            s->nchannels = ff_dca_channels[s->audio_mode];
            s->ch_mask = audio_mode_ch_mask[s->audio_mode];
            if (s->lfe_present)
                s->ch_mask |= DCA_SPEAKER_MASK_LFE1;
        } else {
        }
    }

    // Parse XBR
            return ret;
            if (avctx->err_recognition & AV_EF_EXPLODE)
                return ret;
        } else {
        }
    }

    // Parse X96 unless decoding XLL
                return ret;
                if (ret == AVERROR(ENOMEM) || (avctx->err_recognition & AV_EF_EXPLODE))
                    return ret;
            } else {
            }
                if (ret == AVERROR(ENOMEM) || (avctx->err_recognition & AV_EF_EXPLODE))
                    return ret;
            } else {
            }
        }
    }

    return 0;
}

{

    // Try to map this channel to core first
                return spkr;
                return DCA_SPEAKER_Lss;
                return DCA_SPEAKER_Rss;
            return -1;
        }
        return spkr;
    }

    // Then XCH
        return DCA_SPEAKER_Cs;

    // Then XXCH
    }

    // No mapping
    return -1;
}

{
}

{
    }
}

{

    // Externally set x96_synth flag implies that X96 synthesis should be
    // enabled, yet actual X96 subband data should be discarded. This is a
    // special case for lossless residual decoder that ignores X96 data if
    // present.
        x96_nchannels = s->x96_nchannels;
        x96_synth = 1;
    }
        x96_synth = 0;


    // Reallocate PCM output buffer
        return AVERROR(ENOMEM);

    ptr = (int32_t *)s->output_buffer;
        } else {
        }
    }

    // Handle change of filtering mode

    // Select filter
        filter_coeff = ff_dca_fir_64bands_fixed;
        filter_coeff = ff_dca_fir_32bands_perfect_fixed;
    else

    // Filter primary channels
        // Map this primary channel to speaker
            return AVERROR(EINVAL);

        // Filter bank reconstruction
        s->dcadsp->sub_qmf_fixed[x96_synth](
            &s->synth,
            &s->dcadct,
            s->output_samples[spkr],
            ch < x96_nchannels ? s->x96_subband_samples[ch] : NULL,
            &s->dcadsp_data[ch].offset,
            filter_coeff,
    }

    // Filter LFE channel

        // Check LFF
            av_log(s->avctx, AV_LOG_ERROR, "Fixed point mode doesn't support LFF=1\n");
            return AVERROR(EINVAL);
        }

        // Offset intermediate buffer for X96

        // Interpolate LFE channel
                                 ff_dca_lfe_fir_64_fixed, s->npcmblocks);

            // Filter 96 kHz oversampled LFE PCM to attenuate high frequency
            // (47.6 - 48.0 kHz) components of interpolation image
                                     samples, &s->output_history_lfe_fixed,

        }

        // Update LFE history
    }

    return 0;
}

{

    // Don't filter twice when falling back from XLL
        return ret;


        return ret;

    // Undo embedded XCH downmix
        && s->audio_mode >= DCA_AMODE_2F2R) {
        s->dcadsp->dmix_sub_xch(s->output_samples[DCA_SPEAKER_Ls],
                                s->output_samples[DCA_SPEAKER_Rs],
                                s->output_samples[DCA_SPEAKER_Cs],
                                nsamples);

    }

    // Undo embedded XXCH downmix
        && s->xxch_dmix_embedded) {
        int scale_inv   = s->xxch_dmix_scale_inv;
        int *coeff_ptr  = s->xxch_dmix_coeff;
        int xch_base    = ff_dca_channels[s->audio_mode];
        av_assert1(s->nchannels - xch_base <= DCA_XXCH_CHANNELS_MAX);

        // Undo embedded core downmix pre-scaling
        for (spkr = 0; spkr < s->xxch_mask_nbits; spkr++) {
            if (s->xxch_core_mask & (1U << spkr)) {
                s->dcadsp->dmix_scale_inv(s->output_samples[spkr],
                                          scale_inv, nsamples);
            }
        }

        // Undo downmix
        for (ch = xch_base; ch < s->nchannels; ch++) {
            int src_spkr = map_prm_ch_to_spkr(s, ch);
            if (src_spkr < 0)
                return AVERROR(EINVAL);
            for (spkr = 0; spkr < s->xxch_mask_nbits; spkr++) {
                if (s->xxch_dmix_mask[ch - xch_base] & (1U << spkr)) {
                    int coeff = mul16(*coeff_ptr++, scale_inv);
                    if (coeff) {
                        s->dcadsp->dmix_sub(s->output_samples[spkr    ],
                                            s->output_samples[src_spkr],
                                            coeff, nsamples);
                    }
                }
            }
        }
    }

        // Front sum/difference decoding
            s->fixed_dsp->butterflies_fixed(s->output_samples[DCA_SPEAKER_L],
                                            s->output_samples[DCA_SPEAKER_R],
                                            nsamples);
        }

        // Surround sum/difference decoding
            s->fixed_dsp->butterflies_fixed(s->output_samples[DCA_SPEAKER_Ls],
                                            s->output_samples[DCA_SPEAKER_Rs],
                                            nsamples);
        }
    }

    // Downmix primary channel set to stereo
        ff_dca_downmix_to_stereo_fixed(s->dcadsp,
                                       s->output_samples,
                                       s->prim_dmix_coeff,
                                       nsamples, s->ch_mask);
    }

    }

    return 0;
}

{

    }


        return ret;

    // Build reverse speaker to channel mapping

    // Allocate space for extra channels
            return AVERROR(ENOMEM);

        ptr = (float *)s->output_buffer;
        }
    }

    // Handle change of filtering mode

    // Select filter
        filter_coeff = ff_dca_fir_64bands;
        filter_coeff = ff_dca_fir_32bands_perfect;
    else

    // Filter primary channels
        // Map this primary channel to speaker
            return AVERROR(EINVAL);

        // Filter bank reconstruction
            &s->synth,
            &s->imdct[x96_synth],
            output_samples[spkr],
            ch < x96_nchannels ? s->x96_subband_samples[ch] : NULL,
            &s->dcadsp_data[ch].offset,
            filter_coeff,
    }

    // Filter LFE channel

        // Offset intermediate buffer for X96

        // Select filter
            filter_coeff = ff_dca_lfe_fir_128;
        else

        // Interpolate LFE channel
            filter_coeff, s->npcmblocks);

            // Filter 96 kHz oversampled LFE PCM to attenuate high frequency
            // (47.6 - 48.0 kHz) components of interpolation image
                                     samples, &s->output_history_lfe_float,
        }

        // Update LFE history
    }

    // Undo embedded XCH downmix
                                         -M_SQRT1_2, nsamples);
                                         -M_SQRT1_2, nsamples);
    }

    // Undo embedded XXCH downmix

        // Undo downmix
                return AVERROR(EINVAL);
                                                         coeff * (-1.0f / (1 << 15)),
                                                         nsamples);
                    }
                }
            }
        }

        // Undo embedded core downmix pre-scaling
                                                 scale_inv, nsamples);
            }
        }
    }

        // Front sum/difference decoding
            s->float_dsp->butterflies_float(output_samples[DCA_SPEAKER_L],
                                            output_samples[DCA_SPEAKER_R],
                                            nsamples);
        }

        // Surround sum/difference decoding
            s->float_dsp->butterflies_float(output_samples[DCA_SPEAKER_Ls],
                                            output_samples[DCA_SPEAKER_Rs],
                                            nsamples);
        }
    }

    // Downmix primary channel set to stereo
                                       nsamples, s->ch_mask);
    }

    return 0;
}

{

    // Handle downmixing to stereo request
            s->prim_dmix_type == DCA_DMIX_TYPE_LtRt))
    else
        return AVERROR(EINVAL);

    // Force fixed point mode when falling back from XLL
    else
        return ret;

    // Set profile, bit rate, etc
    else

    else

        matrix_encoding = AV_MATRIX_ENCODING_DOLBY;
    else
        return ret;

    return 0;
}

av_cold void ff_dca_core_flush(DCACoreDecoder *s)
{
    if (s->subband_buffer) {
        erase_adpcm_history(s);
        memset(s->lfe_samples, 0, DCA_LFE_HISTORY * sizeof(int32_t));
    }

    if (s->x96_subband_buffer)
        erase_x96_adpcm_history(s);

    erase_dsp_history(s);
}

{
        return -1;
        return -1;

        return -1;
        return -1;

}

{




