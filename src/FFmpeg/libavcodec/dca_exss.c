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

#include "dcadec.h"

{
    // Size of XLL data in extension substream

    // XLL sync word present flag

        // Peak bit rate smoothing buffer size

        // Number of bits for XLL decoding delay

        // Initial XLL decoding delay in frames

        // Number of bytes offset to XLL sync
    } else {
        asset->xll_delay_nframes = 0;
        asset->xll_sync_offset = 0;
    }

static void parse_lbr_parameters(DCAExssParser *s, DCAExssAsset *asset)
{
    // Size of LBR component in extension substream
    asset->lbr_size = get_bits(&s->gb, 14) + 1;

    // LBR sync word present flag
    if (get_bits1(&s->gb))
        // LBR sync distance
        skip_bits(&s->gb, 2);
}

{

    // Size of audio asset descriptor in bytes

    // Audio asset identifier

    //
    // Per stream static metadata
    //

        // Asset type descriptor presence
            // Asset type descriptor
            skip_bits(&s->gb, 4);

        // Language descriptor presence
            // Language descriptor
            skip_bits(&s->gb, 24);

        // Additional textual information presence
            // Byte size of additional text info
            int text_size = get_bits(&s->gb, 10) + 1;

            // Sanity check available size
            if (get_bits_left(&s->gb) < text_size * 8)
                return AVERROR_INVALIDDATA;

            // Additional textual information string
            skip_bits_long(&s->gb, text_size * 8);
        }

        // PCM bit resolution

        // Maximum sample rate

        // Total number of channels

        // One to one map channel to speakers

            // Embedded stereo flag

            // Embedded 6 channels flag

            // Speaker mask enabled flag
                // Number of bits for speaker activity mask

                // Loudspeaker activity mask
            }

            // Number of speaker remapping sets
                if (s->avctx)
                    av_log(s->avctx, AV_LOG_ERROR, "Speaker mask disabled yet there are remapping sets\n");
                return AVERROR_INVALIDDATA;
            }

            // Standard loudspeaker layout mask

                // Number of channels to be decoded for speaker remapping

                    // Decoded channels to output speaker mapping mask

                    // Loudspeaker remapping codes
                }
            }
        } else {
            asset->embedded_stereo = 0;
            asset->embedded_6ch = 0;
            asset->spkr_mask_enabled = 0;
            asset->spkr_mask = 0;

            // Representation type
            asset->representation_type = get_bits(&s->gb, 3);
        }
    }

    //
    // DRC, DNC and mixing metadata
    //

    // Dynamic range coefficient presence flag

    // Code for dynamic range coefficient
        skip_bits(&s->gb, 8);

    // Dialog normalization presence flag
        // Dialog normalization code
        skip_bits(&s->gb, 5);

    // DRC for stereo downmix
        skip_bits(&s->gb, 8);

    // Mixing metadata presence flag
        int nchannels_dmix;

        // External mixing flag
        skip_bits1(&s->gb);

        // Post mixing / replacement gain adjustment
        skip_bits(&s->gb, 6);

        // DRC prior to mixing
        if (get_bits(&s->gb, 2) == 3)
            // Custom code for mixing DRC
            skip_bits(&s->gb, 8);
        else
            // Limit for mixing DRC
            skip_bits(&s->gb, 3);

        // Scaling type for channels of main audio
        // Scaling parameters of main audio
        if (get_bits1(&s->gb))
            for (i = 0; i < s->nmixoutconfigs; i++)
                skip_bits_long(&s->gb, 6 * s->nmixoutchs[i]);
        else
            skip_bits_long(&s->gb, 6 * s->nmixoutconfigs);

        nchannels_dmix = asset->nchannels_total;
        if (asset->embedded_6ch)
            nchannels_dmix += 6;
        if (asset->embedded_stereo)
            nchannels_dmix += 2;

        for (i = 0; i < s->nmixoutconfigs; i++) {
            if (!s->nmixoutchs[i]) {
                if (s->avctx)
                    av_log(s->avctx, AV_LOG_ERROR, "Invalid speaker layout mask for mixing configuration\n");
                return AVERROR_INVALIDDATA;
            }
            for (j = 0; j < nchannels_dmix; j++) {
                // Mix output mask
                int mix_map_mask = get_bits(&s->gb, s->nmixoutchs[i]);

                // Mixing coefficients
                skip_bits_long(&s->gb, av_popcount(mix_map_mask) * 6);
            }
        }
    }

    //
    // Decoder navigation data
    //

    // Coding mode for the asset

    // Coding components used in asset

            // Size of core component in extension substream
            asset->core_size = get_bits(&s->gb, 14) + 1;
            // Core sync word present flag
            if (get_bits1(&s->gb))
                // Core sync distance
                skip_bits(&s->gb, 2);
        }

            // Size of XBR extension in extension substream

            // Size of XXCH extension in extension substream

            // Size of X96 extension in extension substream

            parse_lbr_parameters(s, asset);


            skip_bits(&s->gb, 16);

            skip_bits(&s->gb, 16);
        break;


    case 2: // Low bit rate mode
        asset->extension_mask = DCA_EXSS_LBR;
        parse_lbr_parameters(s, asset);
        break;

    case 3: // Auxiliary coding mode
        asset->extension_mask = 0;

        // Size of auxiliary coded data
        skip_bits(&s->gb, 14);

        // Auxiliary codec identification
        skip_bits(&s->gb, 8);

        // Aux sync word present flag
        if (get_bits1(&s->gb))
            // Aux sync distance
            skip_bits(&s->gb, 3);
        break;
    }

        // DTS-HD stream ID

    // One to one mixing flag
    // Per channel main audio scaling flag
    // Main audio scaling codes
    // Decode asset in secondary decoder flag
    // Revision 2 DRC metadata
    // Reserved
    // Zero pad
        if (s->avctx)
            av_log(s->avctx, AV_LOG_ERROR, "Read past end of EXSS asset descriptor\n");
        return AVERROR_INVALIDDATA;
    }

    return 0;
}

{

        asset->core_offset = offs;
        if (asset->core_size > size)
            return AVERROR_INVALIDDATA;
        offs += asset->core_size;
        size -= asset->core_size;
    }

            return AVERROR_INVALIDDATA;
    }

            return AVERROR_INVALIDDATA;
    }

            return AVERROR_INVALIDDATA;
    }

        asset->lbr_offset = offs;
        if (asset->lbr_size > size)
            return AVERROR_INVALIDDATA;
        offs += asset->lbr_size;
        size -= asset->lbr_size;
    }

            return AVERROR_INVALIDDATA;
    }

    return 0;
}

{

        return ret;

    // Extension substream sync word

    // User defined bits

    // Extension substream index

    // Flag indicating short or long header size

    // Extension substream header length

    // Check CRC
        av_log(s->avctx, AV_LOG_ERROR, "Invalid EXSS header checksum\n");
        return AVERROR_INVALIDDATA;
    }


    // Number of bytes of extension substream
        if (s->avctx)
            av_log(s->avctx, AV_LOG_ERROR, "Packet too short for EXSS frame\n");
        return AVERROR_INVALIDDATA;
    }

    // Per stream static fields presence flag

        // Reference clock code

        // Extension substream frame duration

        // Timecode presence flag
            // Timecode data
            skip_bits_long(&s->gb, 36);

        // Number of defined audio presentations
            if (s->avctx)
                avpriv_request_sample(s->avctx, "%d audio presentations", s->npresents);
            return AVERROR_PATCHWELCOME;
        }

        // Number of audio assets in extension substream
            if (s->avctx)
                avpriv_request_sample(s->avctx, "%d audio assets", s->nassets);
            return AVERROR_PATCHWELCOME;
        }

        // Active extension substream mask for audio presentation

        // Active audio asset mask

        // Mixing metadata enable flag
            int spkr_mask_nbits;

            // Mixing metadata adjustment level
            skip_bits(&s->gb, 2);

            // Number of bits for mixer output speaker activity mask
            spkr_mask_nbits = (get_bits(&s->gb, 2) + 1) << 2;

            // Number of mixing configurations
            s->nmixoutconfigs = get_bits(&s->gb, 2) + 1;

            // Speaker layout mask for mixer output channels
            for (i = 0; i < s->nmixoutconfigs; i++)
                s->nmixoutchs[i] = ff_dca_count_chs_for_mask(get_bits(&s->gb, spkr_mask_nbits));
        }
    } else {
        s->npresents = 1;
        s->nassets = 1;
    }

    // Size of encoded asset data in bytes
    offset = header_size;
            if (s->avctx)
                av_log(s->avctx, AV_LOG_ERROR, "EXSS asset out of bounds\n");
            return AVERROR_INVALIDDATA;
        }
    }

    // Audio asset descriptor
            return ret;
            if (s->avctx)
                av_log(s->avctx, AV_LOG_ERROR, "Invalid extension size in EXSS asset descriptor\n");
            return ret;
        }
    }

    // Backward compatible core present
    // Backward compatible core substream index
    // Backward compatible core asset index
    // Reserved
    // Byte align
    // CRC16 of extension substream header
        if (s->avctx)
            av_log(s->avctx, AV_LOG_ERROR, "Read past end of EXSS header\n");
        return AVERROR_INVALIDDATA;
    }

    return 0;
}
