/*
 * E-AC-3 decoder
 * Copyright (c) 2007 Bartlomiej Wolowiec <bartek.wolowiec@gmail.com>
 * Copyright (c) 2008 Justin Ruggles
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

/*
 * There are several features of E-AC-3 that this decoder does not yet support.
 *
 * Enhanced Coupling
 *     No known samples exist.  If any ever surface, this feature should not be
 *     too difficult to implement.
 *
 * Reduced Sample Rates
 *     No known samples exist.  The spec also does not give clear information
 *     on how this is to be implemented.
 *
 * Transient Pre-noise Processing
 *     This is side information which a decoder should use to reduce artifacts
 *     caused by transients.  There are samples which are known to have this
 *     information, but this decoder currently ignores it.
 */


#include "avcodec.h"
#include "internal.h"
#include "aac_ac3_parser.h"
#include "ac3.h"
#include "ac3dec.h"
#include "ac3dec_data.h"
#include "eac3_data.h"

/** gain adaptive quantization mode */
typedef enum {
    EAC3_GAQ_NO =0,
    EAC3_GAQ_12,
    EAC3_GAQ_14,
    EAC3_GAQ_124
} EAC3GaqMode;

#define EAC3_SR_CODE_REDUCED  3

{

    /* Set copy index mapping table. Set wrap flags to apply a notch filter at
       wrap points later on. */
            copy_sizes[num_copy_sections++] = bin - s->spx_dst_start_freq;
            bin = s->spx_dst_start_freq;
            wrapflag[bnd] = 1;
        }
                copy_sizes[num_copy_sections++] = bin - s->spx_dst_start_freq;
                bin = s->spx_dst_start_freq;
            }
        }
    }

            continue;

        /* Copy coeffs from normal bands to extension bands */
        }

        /* Calculate RMS energy for each SPX band. */
            }
        }

        /* Apply a notch filter at transitions between normal and extension
           bands and at all wrap points. */
                }
            }
        }

        /* Apply noise-blended coefficient scaling based on previously
           calculated RMS energy, blending factors, and SPX coordinates for
           each band. */
        bin = s->spx_src_start_freq;
#if USE_FIXED
            // spx_noise_blend and spx_signal_blend are both FP.23
            nscale *= 1.0 / (1<<23);
            sscale *= 1.0 / (1<<23);
#endif
            }
        }
    }


/** lrint(M_SQRT2*cos(2*M_PI/12)*(1<<23)) */
#define COEFF_0 10273905LL

/** lrint(M_SQRT2*cos(0*M_PI/12)*(1<<23)) = lrint(M_SQRT2*(1<<23)) */
#define COEFF_1 11863283LL

/** lrint(M_SQRT2*cos(5*M_PI/12)*(1<<23)) */
#define COEFF_2  3070444LL

/**
 * Calculate 6-point IDCT of the pre-mantissas.
 * All calculations are 24-bit fixed-point.
 */
{







{


    /* if GAQ gain is used, decode gain codes for bins with hebap between
       8 and end_bap */
        /* read 1-bit GAQ gain codes */
        }
        /* read 1.67-bit GAQ gain codes (3 codes in 5 bits) */
                        av_log(s->avctx, AV_LOG_WARNING, "GAQ gain group code out-of-range\n");
                        group_code = 26;
                    }
                }
            }
        }
    }

            /* zero-mantissa dithering */
            }
            /* Vector Quantization */
            }
        } else {
            /* Gain Adaptive Quantization */
            } else {
                log_gain = 0;
            }

                    /* large mantissa */
                    /* remap mantissa value to correct for asymmetric quantization */
                    else
                } else {
                    /* small mantissa, no GAQ, or Gk=1 */
                        /* remap mantissa value for no GAQ or Gk=1 */
                    }
                }
            }
        }
    }

{

    /* An E-AC-3 stream can have multiple independent streams which the
       application can select from. each independent stream can also contain
       dependent streams which are used to add or replace channels. */
        av_log(s->avctx, AV_LOG_ERROR, "Reserved frame type\n");
        return AAC_AC3_PARSE_ERROR_FRAME_TYPE;
    }

    /* The substream id indicates which substream this frame belongs to. each
       independent stream has its own substream id, and the dependent streams
       associated to an independent stream have matching substream id's. */
        /* only decode substream with id=0. skip any additional substreams. */
        if (!s->eac3_subsbtreamid_found) {
            s->eac3_subsbtreamid_found = 1;
            avpriv_request_sample(s->avctx, "Additional substreams");
        }
        return AAC_AC3_PARSE_ERROR_FRAME_TYPE;
    }

        /* The E-AC-3 specification does not tell how to handle reduced sample
           rates in bit allocation.  The best assumption would be that it is
           handled like AC-3 DolbyNet, but we cannot be sure until we have a
           sample which utilizes this feature. */
        avpriv_request_sample(s->avctx, "Reduced sampling rate");
        return AVERROR_PATCHWELCOME;
    }

    /* volume control params */
            s->dialog_normalization[i] = -31;
        }
            s->level_gain[i] = powf(2.0f,
                (float)(s->target_level - s->dialog_normalization[i])/6.0f);
        }
        }
    }

    /* dependent stream channel map */


                return AVERROR_INVALIDDATA;
            }
        }
    }

    /* mixing metadata */
        /* center and surround mix levels */
                /* if three front channels exist */
            }
                /* if a surround channel exists */
            }
        }

        /* lfe mix level */
        }

        /* info for mixing with other streams and substreams */
                // TODO: apply program scale factor
                }
            }
                skip_bits(gbc, 6);  // skip external program scale factor
            }
            /* skip mixing parameter data */
                case 1: skip_bits(gbc, 5);  break;
                case 2: skip_bits(gbc, 12); break;
                case 3: {
                    int mix_data_size = (get_bits(gbc, 5) + 2) << 3;
                    skip_bits_long(gbc, mix_data_size);
                    break;
                }
            }
            /* skip pan information for mono or dual mono source */
                for (i = 0; i < (s->channel_mode ? 1 : 2); i++) {
                    if (get_bits1(gbc)) {
                        /* note: this is not in the ATSC A/52B specification
                           reference: ETSI TS 102 366 V1.1.1
                                      section: E.1.3.1.25 */
                        skip_bits(gbc, 8);  // skip pan mean direction index
                        skip_bits(gbc, 6);  // skip reserved paninfo bits
                    }
                }
            }
            /* skip mixing configuration information */
                for (blk = 0; blk < s->num_blocks; blk++) {
                    if (s->num_blocks == 1 || get_bits1(gbc)) {
                        skip_bits(gbc, 5);
                    }
                }
            }
        }
    }

    /* informational metadata */
        }
        }
            }
        }
        }
    }

    /* converter synchronization flag
       If frames are less than six blocks, this bit should be turned on
       once every 6 blocks to indicate the start of a frame set.
       reference: RFC 4598, Section 2.1.3  Frame Sets */
    }

    /* original frame size code if this stream was converted from AC-3 */
            (s->num_blocks == 6 || get_bits1(gbc))) {
        skip_bits(gbc, 6); // skip frame size code
    }

    /* additional bitstream info */
        }
    }

    /* audio frame syntax flags, strategy data, and per-frame data */

    } else {
        /* less than 6 blocks, so use AC-3-style exponent strategy syntax, and
           do not use AHT */
        ac3_exponent_strategy = 1;
        parse_aht_info = 0;
    }



    }

        /* set default bit allocation parameters */
    }


    /* coupling strategy occurrence and coupling use per block */
            } else {
            }
        }
    } else {
        memset(s->cpl_in_use, 0, sizeof(s->cpl_in_use));
    }

    /* exponent strategy data */
        /* AC-3-style exponent strategy syntax */
            }
        }
    } else {
        /* LUT-based exponent strategy syntax */
            }
        }
    }
    /* LFE exponent strategy */
        }
    }
    /* original exponent strategies if this stream was converted from AC-3 */
    }

    /* determine which channels use AHT */
        /* For AHT to be used, all non-zero blocks must reuse exponents from
           the first block.  Furthermore, for AHT to be used in the coupling
           channel, all blocks must use coupling and use the same coupling
           strategy. */
                        (!ch && s->cpl_strategy_exists[blk])) {
                    use_aht = 0;
                    break;
                }
            }
        }
    } else {
    }

    /* per-frame SNR offset */
    }

    /* transient pre-noise processing data */
        for (ch = 1; ch <= s->fbw_channels; ch++) {
            if (get_bits1(gbc)) { // channel in transient processing
                skip_bits(gbc, 10); // skip transient processing location
                skip_bits(gbc, 8);  // skip transient processing length
            }
        }
    }

    /* spectral extension attenuation data */
        } else {
        }
    }

    /* block start information */
        /* reference: Section E2.3.2.27
           nblkstrtbits = (numblks - 1) * (4 + ceiling(log2(words_per_frame)))
           The spec does not say what this data is or what it's used for.
           It is likely the offset of each block within the frame. */
        int block_start_bits = (s->num_blocks-1) * (4 + av_log2(s->frame_size-2));
        skip_bits_long(gbc, block_start_bits);
        avpriv_request_sample(s->avctx, "Block start info");
    }

    /* syntax state initialization */
    }

}
