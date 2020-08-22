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
#include "dcadata.h"
#include "dcamath.h"
#include "dca_syncwords.h"
#include "unary.h"

{
}

{
}

{
}

{

}

{


{


{
    // Size of downmix coefficient matrix


        // Downmix scale (only for non-primary channel sets)
                av_log(s->avctx, AV_LOG_ERROR, "Invalid XLL downmix scale index\n");
                return AVERROR_INVALIDDATA;
            }
        }

        // Downmix coefficients
                av_log(s->avctx, AV_LOG_ERROR, "Invalid XLL downmix coefficient index\n");
                return AVERROR_INVALIDDATA;
            }
                // Multiply by |InvDmixScale| to get |UndoDmixScale|
        }
    }

    return 0;
}

static int chs_parse_header(DCAXllDecoder *s, DCAXllChSet *c, DCAExssAsset *asset)
{
    int i, j, k, ret, band, header_size, header_pos = get_bits_count(&s->gb);
    DCAXllChSet *p = &s->chset[0];
    DCAXllBand *b;

    // Size of channel set sub-header
    header_size = get_bits(&s->gb, 10) + 1;

    // Check CRC
    if (ff_dca_check_crc(s->avctx, &s->gb, header_pos, header_pos + header_size * 8)) {
        av_log(s->avctx, AV_LOG_ERROR, "Invalid XLL sub-header checksum\n");
        return AVERROR_INVALIDDATA;
    }

    // Number of channels in the channel set
    c->nchannels = get_bits(&s->gb, 4) + 1;
    if (c->nchannels > DCA_XLL_CHANNELS_MAX) {
        avpriv_request_sample(s->avctx, "%d XLL channels", c->nchannels);
        return AVERROR_PATCHWELCOME;
    }

    // Residual type
    c->residual_encode = get_bits(&s->gb, c->nchannels);

    // PCM bit resolution
    c->pcm_bit_res = get_bits(&s->gb, 5) + 1;

    // Storage unit width
    c->storage_bit_res = get_bits(&s->gb, 5) + 1;
    if (c->storage_bit_res != 16 && c->storage_bit_res != 20 && c->storage_bit_res != 24) {
        avpriv_request_sample(s->avctx, "%d-bit XLL storage resolution", c->storage_bit_res);
        return AVERROR_PATCHWELCOME;
    }

    if (c->pcm_bit_res > c->storage_bit_res) {
        av_log(s->avctx, AV_LOG_ERROR, "Invalid PCM bit resolution for XLL channel set (%d > %d)\n", c->pcm_bit_res, c->storage_bit_res);
        return AVERROR_INVALIDDATA;
    }

    // Original sampling frequency
    c->freq = ff_dca_sampling_freqs[get_bits(&s->gb, 4)];
    if (c->freq > 192000) {
        avpriv_request_sample(s->avctx, "%d Hz XLL sampling frequency", c->freq);
        return AVERROR_PATCHWELCOME;
    }

    // Sampling frequency modifier
    if (get_bits(&s->gb, 2)) {
        avpriv_request_sample(s->avctx, "XLL sampling frequency modifier");
        return AVERROR_PATCHWELCOME;
    }

    // Which replacement set this channel set is member of
    if (get_bits(&s->gb, 2)) {
        avpriv_request_sample(s->avctx, "XLL replacement set");
        return AVERROR_PATCHWELCOME;
    }

    if (asset->one_to_one_map_ch_to_spkr) {
        // Primary channel set flag
        c->primary_chset = get_bits1(&s->gb);
        if (c->primary_chset != (c == p)) {
            av_log(s->avctx, AV_LOG_ERROR, "The first (and only) XLL channel set must be primary\n");
            return AVERROR_INVALIDDATA;
        }

        // Downmix coefficients present in stream
        c->dmix_coeffs_present = get_bits1(&s->gb);

        // Downmix already performed by encoder
        c->dmix_embedded = c->dmix_coeffs_present && get_bits1(&s->gb);

        // Downmix type
        if (c->dmix_coeffs_present && c->primary_chset) {
            c->dmix_type = get_bits(&s->gb, 3);
            if (c->dmix_type >= DCA_DMIX_TYPE_COUNT) {
                av_log(s->avctx, AV_LOG_ERROR, "Invalid XLL primary channel set downmix type\n");
                return AVERROR_INVALIDDATA;
            }
        }

        // Whether the channel set is part of a hierarchy
        c->hier_chset = get_bits1(&s->gb);
        if (!c->hier_chset && s->nchsets != 1) {
            avpriv_request_sample(s->avctx, "XLL channel set outside of hierarchy");
            return AVERROR_PATCHWELCOME;
        }

        // Downmix coefficients
        if (c->dmix_coeffs_present && (ret = parse_dmix_coeffs(s, c)) < 0)
            return ret;

        // Channel mask enabled
        if (!get_bits1(&s->gb)) {
            avpriv_request_sample(s->avctx, "Disabled XLL channel mask");
            return AVERROR_PATCHWELCOME;
        }

        // Channel mask for set
        c->ch_mask = get_bits_long(&s->gb, s->ch_mask_nbits);
        if (av_popcount(c->ch_mask) != c->nchannels) {
            av_log(s->avctx, AV_LOG_ERROR, "Invalid XLL channel mask\n");
            return AVERROR_INVALIDDATA;
        }

        // Build the channel to speaker map
        for (i = 0, j = 0; i < s->ch_mask_nbits; i++)
            if (c->ch_mask & (1U << i))
                c->ch_remap[j++] = i;
    } else {
        // Mapping coeffs present flag
        if (c->nchannels != 2 || s->nchsets != 1 || get_bits1(&s->gb)) {
            avpriv_request_sample(s->avctx, "Custom XLL channel to speaker mapping");
            return AVERROR_PATCHWELCOME;
        }

        // Setup for LtRt decoding
        c->primary_chset = 1;
        c->dmix_coeffs_present = 0;
        c->dmix_embedded = 0;
        c->hier_chset = 0;
        c->ch_mask = DCA_SPEAKER_LAYOUT_STEREO;
        c->ch_remap[0] = DCA_SPEAKER_L;
        c->ch_remap[1] = DCA_SPEAKER_R;
    }

    if (c->freq > 96000) {
        // Extra frequency bands flag
        if (get_bits1(&s->gb)) {
            avpriv_request_sample(s->avctx, "Extra XLL frequency bands");
            return AVERROR_PATCHWELCOME;
        }
        c->nfreqbands = 2;
    } else {
        c->nfreqbands = 1;
    }

    // Set the sampling frequency to that of the first frequency band.
    // Frequency will be doubled again after bands assembly.
    c->freq >>= c->nfreqbands - 1;

    // Verify that all channel sets have the same audio characteristics
    if (c != p && (c->nfreqbands != p->nfreqbands || c->freq != p->freq
                   || c->pcm_bit_res != p->pcm_bit_res
                   || c->storage_bit_res != p->storage_bit_res)) {
        avpriv_request_sample(s->avctx, "Different XLL audio characteristics");
        return AVERROR_PATCHWELCOME;
    }

    // Determine number of bits to read bit allocation coding parameter
    if (c->storage_bit_res > 16)
        c->nabits = 5;
    else if (c->storage_bit_res > 8)
        c->nabits = 4;
    else
        c->nabits = 3;

    // Account for embedded downmix and decimator saturation
    if ((s->nchsets > 1 || c->nfreqbands > 1) && c->nabits < 5)
        c->nabits++;

    for (band = 0, b = c->bands; band < c->nfreqbands; band++, b++) {
        // Pairwise channel decorrelation
        if ((b->decor_enabled = get_bits1(&s->gb)) && c->nchannels > 1) {
            int ch_nbits = av_ceil_log2(c->nchannels);

            // Original channel order
            for (i = 0; i < c->nchannels; i++) {
                b->orig_order[i] = get_bits(&s->gb, ch_nbits);
                if (b->orig_order[i] >= c->nchannels) {
                    av_log(s->avctx, AV_LOG_ERROR, "Invalid XLL original channel order\n");
                    return AVERROR_INVALIDDATA;
                }
            }

            // Pairwise channel coefficients
            for (i = 0; i < c->nchannels / 2; i++)
                b->decor_coeff[i] = get_bits1(&s->gb) ? get_linear(&s->gb, 7) : 0;
        } else {
            for (i = 0; i < c->nchannels; i++)
                b->orig_order[i] = i;
            for (i = 0; i < c->nchannels / 2; i++)
                b->decor_coeff[i] = 0;
        }

        // Adaptive predictor order
        b->highest_pred_order = 0;
        for (i = 0; i < c->nchannels; i++) {
            b->adapt_pred_order[i] = get_bits(&s->gb, 4);
            if (b->adapt_pred_order[i] > b->highest_pred_order)
                b->highest_pred_order = b->adapt_pred_order[i];
        }
        if (b->highest_pred_order > s->nsegsamples) {
            av_log(s->avctx, AV_LOG_ERROR, "Invalid XLL adaptive predicition order\n");
            return AVERROR_INVALIDDATA;
        }

        // Fixed predictor order
        for (i = 0; i < c->nchannels; i++)
            b->fixed_pred_order[i] = b->adapt_pred_order[i] ? 0 : get_bits(&s->gb, 2);

        // Adaptive predictor quantized reflection coefficients
        for (i = 0; i < c->nchannels; i++) {
            for (j = 0; j < b->adapt_pred_order[i]; j++) {
                k = get_linear(&s->gb, 8);
                if (k == -128) {
                    av_log(s->avctx, AV_LOG_ERROR, "Invalid XLL reflection coefficient index\n");
                    return AVERROR_INVALIDDATA;
                }
                if (k < 0)
                    b->adapt_refl_coeff[i][j] = -(int)ff_dca_xll_refl_coeff[-k];
                else
                    b->adapt_refl_coeff[i][j] =  (int)ff_dca_xll_refl_coeff[ k];
            }
        }

        // Downmix performed by encoder in extension frequency band
        b->dmix_embedded = c->dmix_embedded && (band == 0 || get_bits1(&s->gb));

        // MSB/LSB split flag in extension frequency band
        if ((band == 0 && s->scalable_lsbs) || (band != 0 && get_bits1(&s->gb))) {
            // Size of LSB section in any segment
            b->lsb_section_size = get_bits_long(&s->gb, s->seg_size_nbits);
            if (b->lsb_section_size < 0 || b->lsb_section_size > s->frame_size) {
                av_log(s->avctx, AV_LOG_ERROR, "Invalid LSB section size\n");
                return AVERROR_INVALIDDATA;
            }

            // Account for optional CRC bytes after LSB section
            if (b->lsb_section_size && (s->band_crc_present > 2 ||
                                        (band == 0 && s->band_crc_present > 1)))
                b->lsb_section_size += 2;

            // Number of bits to represent the samples in LSB part
            for (i = 0; i < c->nchannels; i++) {
                b->nscalablelsbs[i] = get_bits(&s->gb, 4);
                if (b->nscalablelsbs[i] && !b->lsb_section_size) {
                    av_log(s->avctx, AV_LOG_ERROR, "LSB section missing with non-zero LSB width\n");
                    return AVERROR_INVALIDDATA;
                }
            }
        } else {
            b->lsb_section_size = 0;
            for (i = 0; i < c->nchannels; i++)
                b->nscalablelsbs[i] = 0;
        }

        // Scalable resolution flag in extension frequency band
        if ((band == 0 && s->scalable_lsbs) || (band != 0 && get_bits1(&s->gb))) {
            // Number of bits discarded by authoring
            for (i = 0; i < c->nchannels; i++)
                b->bit_width_adjust[i] = get_bits(&s->gb, 4);
        } else {
            for (i = 0; i < c->nchannels; i++)
                b->bit_width_adjust[i] = 0;
        }
    }

    // Reserved
    // Byte align
    // CRC16 of channel set sub-header
    if (ff_dca_seek_bits(&s->gb, header_pos + header_size * 8)) {
        av_log(s->avctx, AV_LOG_ERROR, "Read past end of XLL sub-header\n");
        return AVERROR_INVALIDDATA;
    }

    return 0;
}

static int chs_alloc_msb_band_data(DCAXllDecoder *s, DCAXllChSet *c)
{
    int ndecisamples = c->nfreqbands > 1 ? DCA_XLL_DECI_HISTORY_MAX : 0;
    int nchsamples = s->nframesamples + ndecisamples;
    int i, j, nsamples = nchsamples * c->nchannels * c->nfreqbands;
    int32_t *ptr;

    // Reallocate MSB sample buffer
    av_fast_malloc(&c->sample_buffer[0], &c->sample_size[0], nsamples * sizeof(int32_t));
    if (!c->sample_buffer[0])
        return AVERROR(ENOMEM);

    ptr = c->sample_buffer[0] + ndecisamples;
    for (i = 0; i < c->nfreqbands; i++) {
        for (j = 0; j < c->nchannels; j++) {
            c->bands[i].msb_sample_buffer[j] = ptr;
            ptr += nchsamples;
        }
    }

    return 0;
}

static int chs_alloc_lsb_band_data(DCAXllDecoder *s, DCAXllChSet *c)
{
    int i, j, nsamples = 0;
    int32_t *ptr;

    // Determine number of frequency bands that have MSB/LSB split
    for (i = 0; i < c->nfreqbands; i++)
        if (c->bands[i].lsb_section_size)
            nsamples += s->nframesamples * c->nchannels;
    if (!nsamples)
        return 0;

    // Reallocate LSB sample buffer
    av_fast_malloc(&c->sample_buffer[1], &c->sample_size[1], nsamples * sizeof(int32_t));
    if (!c->sample_buffer[1])
        return AVERROR(ENOMEM);

    ptr = c->sample_buffer[1];
    for (i = 0; i < c->nfreqbands; i++) {
        if (c->bands[i].lsb_section_size) {
            for (j = 0; j < c->nchannels; j++) {
                c->bands[i].lsb_sample_buffer[j] = ptr;
                ptr += s->nframesamples;
            }
        } else {
            for (j = 0; j < c->nchannels; j++)
                c->bands[i].lsb_sample_buffer[j] = NULL;
        }
    }

    return 0;
}

{

    // Start unpacking MSB portion of the segment
        // Unpack segment type
        // 0 - distinct coding parameters for each channel
        // 1 - common coding parameters for all channels

        // Determine number of coding parameters encoded in segment

        // Unpack Rice coding parameters
            // Unpack Rice coding flag
            // 0 - linear code, 1 - Rice code
            // Unpack Hybrid Rice coding flag
            // 0 - Rice code, 1 - Hybrid Rice code
                // Unpack binary code length for isolated samples
            else
                // 0 indicates no Hybrid Rice coding
        }

        // Unpack coding parameters
                // Unpack coding parameter for part A of segment 0

                // Adjust for the linear code

                else
            } else {
            }

            // Unpack coding parameter for part B of segment

            // Adjust for the linear code
        }
    }

    // Unpack entropy codes

        // Select index of coding parameters

        // Slice the segment into parts A and B

            return AVERROR_INVALIDDATA;

            // Linear codes
            // Unpack all residuals of part A of segment 0
                             c->bitalloc_part_a[k]);

            // Unpack all residuals of part B of segment 0 and others
                             c->bitalloc_part_b[k]);
        } else {
            // Rice codes
            // Unpack all residuals of part A of segment 0
                           c->bitalloc_part_a[k]);

                // Hybrid Rice codes
                // Unpack the number of isolated samples

                // Set all locations to 0

                // Extract the locations of isolated samples and flag by -1
                        av_log(s->avctx, AV_LOG_ERROR, "Invalid isolated sample location\n");
                        return AVERROR_INVALIDDATA;
                    }
                }

                // Unpack all residuals of part B of segment 0 and others
                    else
                }
            } else {
                // Rice codes
                // Unpack all residuals of part B of segment 0 and others
            }
        }
    }

    // Unpack decimator history for frequency band 1
    }

    // Start unpacking LSB portion of the segment
        // Skip to the start of LSB portion
            av_log(s->avctx, AV_LOG_ERROR, "Read past end of XLL band data\n");
            return AVERROR_INVALIDDATA;
        }

        // Unpack all LSB parts of residuals of this segment
                          s->nsegsamples, b->nscalablelsbs[i]);
            }
        }
    }

    // Skip to the end of band data
        av_log(s->avctx, AV_LOG_ERROR, "Read past end of XLL band data\n");
        return AVERROR_INVALIDDATA;
    }

    return 0;
}

static av_cold void chs_clear_band_data(DCAXllDecoder *s, DCAXllChSet *c, int band, int seg)
{
    DCAXllBand *b = &c->bands[band];
    int i, offset, nsamples;

    if (seg < 0) {
        offset = 0;
        nsamples = s->nframesamples;
    } else {
        offset = seg * s->nsegsamples;
        nsamples = s->nsegsamples;
    }

    for (i = 0; i < c->nchannels; i++) {
        memset(b->msb_sample_buffer[i] + offset, 0, nsamples * sizeof(int32_t));
        if (b->lsb_section_size)
            memset(b->lsb_sample_buffer[i] + offset, 0, nsamples * sizeof(int32_t));
    }

    if (seg <= 0 && band)
        memset(c->deci_history, 0, sizeof(c->deci_history));

    if (seg < 0) {
        memset(b->nscalablelsbs, 0, sizeof(b->nscalablelsbs));
        memset(b->bit_width_adjust, 0, sizeof(b->bit_width_adjust));
    }
}

{

    // Inverse adaptive or fixed prediction
            int coeff[DCA_XLL_ADAPT_PRED_ORDER_MAX];
            // Conversion from reflection coefficients to direct form coefficients
                }
            }
            // Inverse adaptive prediction
                int64_t err = 0;
            }
        } else {
            // Inverse fixed coefficient prediction
        }
    }

    // Inverse pairwise channel decorrellation
        int32_t *tmp[DCA_XLL_CHANNELS_MAX];

                                 coeff, nsamples);
            }
        }

        // Reorder channel pointers to the original order

    }

    // Map output channel pointers for frequency band 0

{

        shift = s->fixed_lsb_width;
        shift += adj - 1;
    else

}

{

            } else {
                for (n = 0; n < nsamples; n++)
                    msb[n] = msb[n] * (SUINT)(1 << shift);
            }
        }
    }

{


    // Reallocate frequency band assembly buffer
        return AVERROR(ENOMEM);

    // Assemble frequency bands 0 and 1
    ptr = c->sample_buffer[2];

        // Copy decimator history

        // Filter
                                       ff_dca_xll_band_coeff,
                                       nsamples);

        // Remap output channel pointer to assembly buffer
    }

    return 0;
}

{

    // XLL extension sync word
        av_log(s->avctx, AV_LOG_VERBOSE, "Invalid XLL sync word\n");
        return AVERROR(EAGAIN);
    }

    // Version number
        avpriv_request_sample(s->avctx, "XLL stream version %d", stream_ver);
        return AVERROR_PATCHWELCOME;
    }

    // Lossless frame header length

    // Check CRC
        av_log(s->avctx, AV_LOG_ERROR, "Invalid XLL common header checksum\n");
        return AVERROR_INVALIDDATA;
    }

    // Number of bits used to read frame size

    // Number of bytes in a lossless frame
        av_log(s->avctx, AV_LOG_ERROR, "Invalid XLL frame size (%d bytes)\n", s->frame_size);
        return AVERROR_INVALIDDATA;
    }

    // Number of channels sets per frame
        avpriv_request_sample(s->avctx, "%d XLL channel sets", s->nchsets);
        return AVERROR_PATCHWELCOME;
    }

    // Number of segments per frame
        av_log(s->avctx, AV_LOG_ERROR, "Too many segments per XLL frame\n");
        return AVERROR_INVALIDDATA;
    }

    // Samples in segment per one frequency band for the first channel set
    // Maximum value is 256 for sampling frequencies <= 48 kHz
    // Maximum value is 512 for sampling frequencies > 48 kHz
        av_log(s->avctx, AV_LOG_ERROR, "Too few samples per XLL segment\n");
        return AVERROR_INVALIDDATA;
    }
        av_log(s->avctx, AV_LOG_ERROR, "Too many samples per XLL segment\n");
        return AVERROR_INVALIDDATA;
    }

    // Samples in frame per one frequency band for the first channel set
        av_log(s->avctx, AV_LOG_ERROR, "Too many samples per XLL frame\n");
        return AVERROR_INVALIDDATA;
    }

    // Number of bits used to read segment size

    // Presence of CRC16 within each frequency band
    // 0 - No CRC16 within band
    // 1 - CRC16 placed at the end of MSB0
    // 2 - CRC16 placed at the end of MSB0 and LSB0
    // 3 - CRC16 placed at the end of MSB0 and LSB0 and other frequency bands

    // MSB/LSB split flag

    // Channel position mask

    // Fixed LSB width
    else

    // Reserved
    // Byte align
    // Header CRC16 protection
        av_log(s->avctx, AV_LOG_ERROR, "Read past end of XLL common header\n");
        return AVERROR_INVALIDDATA;
    }

    return 0;
}

{
}

{
                return c;

    return NULL;
}

{

        }
    }

{

    // Parse channel set headers
            return ret;
    }

    // Pre-scale downmixing coefficients for all non-primary channel sets
        }
    }

    // Determine number of active channel sets to decode
    case DCA_SPEAKER_LAYOUT_5POINT1:
    }

    return 0;
}

{

    // Determine size of NAVI table
        av_log(s->avctx, AV_LOG_ERROR, "Too many NAVI entries (%d)\n", navi_nb);
        return AVERROR_INVALIDDATA;
    }

    // Reallocate NAVI table
        return AVERROR(ENOMEM);

    // Parse NAVI
                        av_log(s->avctx, AV_LOG_ERROR, "Invalid NAVI segment size (%d bytes)\n", size);
                        return AVERROR_INVALIDDATA;
                    }
                }
            }
        }
    }

    // Byte align
    // CRC16

    // Check CRC
        av_log(s->avctx, AV_LOG_ERROR, "Invalid NAVI checksum\n");
        return AVERROR_INVALIDDATA;
    }

    return 0;
}

{

            return ret;
            return ret;
    }

                        av_log(s->avctx, AV_LOG_ERROR, "Invalid NAVI position\n");
                        return AVERROR_INVALIDDATA;
                    }
                        if (s->avctx->err_recognition & AV_EF_EXPLODE)
                            return ret;
                        chs_clear_band_data(s, c, band, seg);
                    }
                }
            }
        }
    }

    return 0;
}

{

        return ret;
        return ret;
        return ret;
        return ret;
        return ret;
        av_log(s->avctx, AV_LOG_ERROR, "Read past end of XLL frame\n");
        return AVERROR_INVALIDDATA;
    }
    return ret;
}

{
}

static int copy_to_pbr(DCAXllDecoder *s, uint8_t *data, int size, int delay)
{
    if (size > DCA_XLL_PBR_BUFFER_MAX)
        return AVERROR(ENOSPC);

    if (!s->pbr_buffer && !(s->pbr_buffer = av_malloc(DCA_XLL_PBR_BUFFER_MAX + AV_INPUT_BUFFER_PADDING_SIZE)))
        return AVERROR(ENOMEM);

    memcpy(s->pbr_buffer, data, size);
    s->pbr_length = size;
    s->pbr_delay = delay;
    return 0;
}

{

    // If XLL packet data didn't start with a sync word, we must have jumped
    // right into the middle of PBR smoothing period
        // Skip to the next sync word in this packet
        data += asset->xll_sync_offset;
        size -= asset->xll_sync_offset;

        // If decoding delay is set, put the frame into PBR buffer and return
        // failure code. Higher level decoder is expected to switch to lossy
        // core decoding or mute its output until decoding delay expires.
        if (asset->xll_delay_nframes > 0) {
            if ((ret = copy_to_pbr(s, data, size, asset->xll_delay_nframes)) < 0)
                return ret;
            return AVERROR(EAGAIN);
        }

        // No decoding delay, just parse the frame in place
        ret = parse_frame(s, data, size, asset);
    }

        return ret;

        return AVERROR(EINVAL);

    // If the XLL decoder didn't consume full packet, start PBR smoothing period
        if ((ret = copy_to_pbr(s, data + s->frame_size, size - s->frame_size, 0)) < 0)
            return ret;

    return 0;
}

static int parse_frame_pbr(DCAXllDecoder *s, uint8_t *data, int size, DCAExssAsset *asset)
{
    int ret;

    if (size > DCA_XLL_PBR_BUFFER_MAX - s->pbr_length) {
        ret = AVERROR(ENOSPC);
        goto fail;
    }

    memcpy(s->pbr_buffer + s->pbr_length, data, size);
    s->pbr_length += size;

    // Respect decoding delay after synchronization error
    if (s->pbr_delay > 0 && --s->pbr_delay)
        return AVERROR(EAGAIN);

    if ((ret = parse_frame(s, s->pbr_buffer, s->pbr_length, asset)) < 0)
        goto fail;

    if (s->frame_size > s->pbr_length) {
        ret = AVERROR(EINVAL);
        goto fail;
    }

    if (s->frame_size == s->pbr_length) {
        // End of PBR smoothing period
        clear_pbr(s);
    } else {
        s->pbr_length -= s->frame_size;
        memmove(s->pbr_buffer, s->pbr_buffer + s->frame_size, s->pbr_length);
    }

    return 0;

fail:
    // For now, throw out all PBR state on failure.
    // Perhaps we can be smarter and try to resync somehow.
    clear_pbr(s);
    return ret;
}

{

        clear_pbr(s);
        s->hd_stream_id = asset->hd_stream_id;
    }

        ret = parse_frame_pbr(s, data + asset->xll_offset, asset->xll_size, asset);
    else

}

{

            continue;

        av_assert1(band < c->nfreqbands);
                                            coeff, DCA_XLL_DECI_HISTORY_MAX);
                }
            }
        }

            break;
    }

{

            continue;

        av_assert1(band < c->nfreqbands);
                                          scale, DCA_XLL_DECI_HISTORY_MAX);
            }
        }

            break;
    }

// Clear all band data and replace non-residual encoded channels with lossy
// counterparts
{


    }

{

    // Verify that core is compatible
        av_log(s->avctx, AV_LOG_ERROR, "Residual encoded channels are present without core\n");
        return AVERROR(EINVAL);
    }

        av_log(s->avctx, AV_LOG_WARNING, "Sample rate mismatch between core (%d Hz) and XLL (%d Hz)\n", dca->core.output_rate, c->freq);
        return AVERROR_INVALIDDATA;
    }

        av_log(s->avctx, AV_LOG_WARNING, "Number of samples per frame mismatch between core (%d) and XLL (%d)\n", dca->core.npcmsamples, nsamples);
        return AVERROR_INVALIDDATA;
    }

    // See if this channel set is downmixed and find the next channel set in
    // hierarchy. If downmixed, undo core pre-scaling before combining with
    // residual (residual is not scaled).

    // Reduce core bit width and combine with residual


        // Map this channel to core speaker
            av_log(s->avctx, AV_LOG_WARNING, "Residual encoded channel (%d) references unavailable core channel\n", c->ch_remap[ch]);
            return AVERROR_INVALIDDATA;
        }

        // Account for LSB width
            av_log(s->avctx, AV_LOG_WARNING, "Invalid core shift (%d bits)\n", shift);
            return AVERROR_INVALIDDATA;
        }


            // Undo embedded core downmix pre-scaling
        } else {
            // No downmix scaling
        }
    }

    return 0;
}

{

    // Force lossy downmixed output during recovery

        }

    }

    // Filter frequency bands for active channel sets

            return ret;


        }

    }

    // Undo hierarchial downmix and/or apply scaling

            break;
        }

    }

    // Assemble frequency bands for active channel sets
                return ret;
    }

    // Normalize to regular 5.1 layout if downmixing
        }
        }
    }

    // Handle downmixing to stereo request
            p->dmix_type == DCA_DMIX_TYPE_LtRt))
        request_mask = DCA_SPEAKER_LAYOUT_STEREO;
    else
        return AVERROR(EINVAL);


    case 24:
    default:
        return AVERROR(EINVAL);
    }


        return ret;

    // Downmix primary channel set to stereo
                                       s->output_mask);
    }

        } else {
        }
    }

        if (asset->representation_type == DCA_REPR_TYPE_LtRt)
            matrix_encoding = AV_MATRIX_ENCODING_DOLBY;
        else if (asset->representation_type == DCA_REPR_TYPE_LhRh)
            matrix_encoding = AV_MATRIX_ENCODING_DOLBYHEADPHONE;
        matrix_encoding = AV_MATRIX_ENCODING_DOLBY;
    }
        return ret;

    return 0;
}

av_cold void ff_dca_xll_flush(DCAXllDecoder *s)
{
    clear_pbr(s);
}

{

        }
    }


