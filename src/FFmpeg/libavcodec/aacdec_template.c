/*
 * AAC decoder
 * Copyright (c) 2005-2006 Oded Shimon ( ods15 ods15 dyndns org )
 * Copyright (c) 2006-2007 Maxim Gavrilov ( maxim.gavrilov gmail com )
 * Copyright (c) 2008-2013 Alex Converse <alex.converse@gmail.com>
 *
 * AAC LATM decoder
 * Copyright (c) 2008-2010 Paul Kendall <paul@kcbbs.gen.nz>
 * Copyright (c) 2010      Janne Grunau <janne-libav@jannau.net>
 *
 * AAC decoder fixed-point implementation
 * Copyright (c) 2013
 *      MIPS Technologies, Inc., California.
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
 * AAC decoder
 * @author Oded Shimon  ( ods15 ods15 dyndns org )
 * @author Maxim Gavrilov ( maxim.gavrilov gmail com )
 *
 * AAC decoder fixed-point implementation
 * @author Stanislav Ocovaj ( stanislav.ocovaj imgtec com )
 * @author Nedeljko Babic ( nedeljko.babic imgtec com )
 */

/*
 * supported tools
 *
 * Support?                     Name
 * N (code in SoC repo)         gain control
 * Y                            block switching
 * Y                            window shapes - standard
 * N                            window shapes - Low Delay
 * Y                            filterbank - standard
 * N (code in SoC repo)         filterbank - Scalable Sample Rate
 * Y                            Temporal Noise Shaping
 * Y                            Long Term Prediction
 * Y                            intensity stereo
 * Y                            channel coupling
 * Y                            frequency domain prediction
 * Y                            Perceptual Noise Substitution
 * Y                            Mid/Side stereo
 * N                            Scalable Inverse AAC Quantization
 * N                            Frequency Selective Switch
 * N                            upsampling filter
 * Y                            quantization & coding - AAC
 * N                            quantization & coding - TwinVQ
 * N                            quantization & coding - BSAC
 * N                            AAC Error Resilience tools
 * N                            Error Resilience payload syntax
 * N                            Error Protection tool
 * N                            CELP
 * N                            Silence Compression
 * N                            HVXC
 * N                            HVXC 4kbits/s VR
 * N                            Structured Audio tools
 * N                            Structured Audio Sample Bank Format
 * N                            MIDI
 * N                            Harmonic and Individual Lines plus Noise
 * N                            Text-To-Speech Interface
 * Y                            Spectral Band Replication
 * Y (not in this code)         Layer-1
 * Y (not in this code)         Layer-2
 * Y (not in this code)         Layer-3
 * N                            SinuSoidal Coding (Transient, Sinusoid, Noise)
 * Y                            Parametric Stereo
 * N                            Direct Stream Transfer
 * Y  (not in fixed point code) Enhanced AAC Low Delay (ER AAC ELD)
 *
 * Note: - HE AAC v1 comprises LC AAC with Spectral Band Replication.
 *       - HE AAC v2 comprises LC AAC with Spectral Band Replication and
           Parametric Stereo.
 */

#include "libavutil/thread.h"

static VLC vlc_scalefactors;
static VLC vlc_spectral[11];

static int output_configure(AACContext *ac,
                            uint8_t layout_map[MAX_ELEM_ID*4][3], int tags,
                            enum OCStatus oc_type, int get_new_frame);

#define overread_err "Input buffer exhausted before END element found\n"

{
    }
}

/**
 * Check for the channel element in the current channel position configuration.
 * If it exists, make sure the appropriate element is allocated and map the
 * channel order to match the internal FFmpeg channel layout.
 *
 * @param   che_pos current channel position configuration
 * @param   type channel element type
 * @param   id channel element id
 * @param   channels count of the number of channels in the configuration
 *
 * @return  Returns error status. 0 - OK, !0 - error
 */
                                 enum ChannelPosition che_pos,
                                 int type, int id, int *channels)
{
        return AVERROR_INVALIDDATA;
                return AVERROR(ENOMEM);
        }
                av_log(ac->avctx, AV_LOG_ERROR, "Too many channels\n");
                return AVERROR_INVALIDDATA;
            }
            }
        }
    } else {
        if (ac->che[type][id])
            AAC_RENAME(ff_aac_sbr_ctx_close)(&ac->che[type][id]->sbr);
        av_freep(&ac->che[type][id]);
    }
    return 0;
}

{

    /* set channel pointers to internal buffers by default */
            }
        }
    }

    /* get output buffer */
        return 1;

        return ret;

    /* map output channel pointers to AVFrame data */
    }

    return 0;
}

struct elem_to_channel {
    uint64_t av_position;
    uint8_t syn_ele;
    uint8_t elem_id;
    uint8_t aac_position;
};

                       uint8_t (*layout_map)[3], int offset, uint64_t left,
                       uint64_t right, int pos)
{
            .syn_ele      = TYPE_CPE,
            .aac_position = pos
        };
    } else {
            .av_position  = left,
            .syn_ele      = TYPE_SCE,
            .aac_position = pos
        };
            .av_position  = right,
            .syn_ele      = TYPE_SCE,
            .aac_position = pos
        };
    }
}

                                 int *current)
{
            break;
                    sce_parity = 0;
                } else {
                    return -1;
                }
            }
        } else {
        }
    }
        return -1;
}

{

        return 0;

        return 0;
        return 0;
        return 0;

        num_side_channels = 2;
        num_back_channels -= 2;
    }

            .av_position  = AV_CH_FRONT_CENTER,
            .syn_ele      = TYPE_SCE,
            .aac_position = AAC_CHANNEL_FRONT
        };
    }
                         AV_CH_FRONT_LEFT_OF_CENTER,
                         AV_CH_FRONT_RIGHT_OF_CENTER,
                         AAC_CHANNEL_FRONT);
    }
                         AV_CH_FRONT_LEFT,
                         AV_CH_FRONT_RIGHT,
                         AAC_CHANNEL_FRONT);
    }
        i += assign_pair(e2c_vec, layout_map, i,
                         UINT64_MAX,
                         UINT64_MAX,
                         AAC_CHANNEL_FRONT);
        num_front_channels -= 2;
    }

        i += assign_pair(e2c_vec, layout_map, i,
                         AV_CH_SIDE_LEFT,
                         AV_CH_SIDE_RIGHT,
                         AAC_CHANNEL_FRONT);
        num_side_channels -= 2;
    }
        i += assign_pair(e2c_vec, layout_map, i,
                         UINT64_MAX,
                         UINT64_MAX,
                         AAC_CHANNEL_SIDE);
        num_side_channels -= 2;
    }

        i += assign_pair(e2c_vec, layout_map, i,
                         UINT64_MAX,
                         UINT64_MAX,
                         AAC_CHANNEL_BACK);
        num_back_channels -= 2;
    }
                         AV_CH_BACK_LEFT,
                         AV_CH_BACK_RIGHT,
                         AAC_CHANNEL_BACK);
    }
        e2c_vec[i] = (struct elem_to_channel) {
            .av_position  = AV_CH_BACK_CENTER,
            .syn_ele      = TYPE_SCE,
            .elem_id      = layout_map[i][1],
            .aac_position = AAC_CHANNEL_BACK
        };
        i++;
        num_back_channels--;
    }

            .av_position  = AV_CH_LOW_FREQUENCY,
            .syn_ele      = TYPE_LFE,
            .aac_position = AAC_CHANNEL_LFE
        };
    }
        e2c_vec[i] = (struct elem_to_channel) {
            .av_position  = AV_CH_LOW_FREQUENCY_2,
            .syn_ele      = TYPE_LFE,
            .elem_id      = layout_map[i][1],
            .aac_position = AAC_CHANNEL_LFE
        };
        i++;
    }
        e2c_vec[i] = (struct elem_to_channel) {
            .av_position  = UINT64_MAX,
            .syn_ele      = TYPE_LFE,
            .elem_id      = layout_map[i][1],
            .aac_position = AAC_CHANNEL_LFE
        };
        i++;
    }

    // The previous checks would end up at 8 at this point for 22.2
        e2c_vec[i] = (struct elem_to_channel) {
            .av_position  = AV_CH_TOP_FRONT_CENTER,
            .syn_ele      = layout_map[i][0],
            .elem_id      = layout_map[i][1],
            .aac_position = layout_map[i][2]
        }; i++;
        i += assign_pair(e2c_vec, layout_map, i,
                         AV_CH_TOP_FRONT_LEFT,
                         AV_CH_TOP_FRONT_RIGHT,
                         AAC_CHANNEL_FRONT);
        i += assign_pair(e2c_vec, layout_map, i,
                         AV_CH_TOP_SIDE_LEFT,
                         AV_CH_TOP_SIDE_RIGHT,
                         AAC_CHANNEL_SIDE);
        e2c_vec[i] = (struct elem_to_channel) {
            .av_position  = AV_CH_TOP_CENTER,
            .syn_ele      = layout_map[i][0],
            .elem_id      = layout_map[i][1],
            .aac_position = layout_map[i][2]
        }; i++;
        i += assign_pair(e2c_vec, layout_map, i,
                         AV_CH_TOP_BACK_LEFT,
                         AV_CH_TOP_BACK_RIGHT,
                         AAC_CHANNEL_BACK);
        e2c_vec[i] = (struct elem_to_channel) {
            .av_position  = AV_CH_TOP_BACK_CENTER,
            .syn_ele      = layout_map[i][0],
            .elem_id      = layout_map[i][1],
            .aac_position = layout_map[i][2]
        }; i++;
        e2c_vec[i] = (struct elem_to_channel) {
            .av_position  = AV_CH_BOTTOM_FRONT_CENTER,
            .syn_ele      = layout_map[i][0],
            .elem_id      = layout_map[i][1],
            .aac_position = layout_map[i][2]
        }; i++;
        i += assign_pair(e2c_vec, layout_map, i,
                         AV_CH_BOTTOM_FRONT_LEFT,
                         AV_CH_BOTTOM_FRONT_RIGHT,
                         AAC_CHANNEL_FRONT);
    }


        // For 22.2 reorder the result as needed
        FFSWAP(struct elem_to_channel, e2c_vec[2], e2c_vec[0]);   // FL & FR first (final), FC third
        FFSWAP(struct elem_to_channel, e2c_vec[2], e2c_vec[1]);   // FC second (final), FLc & FRc third
        FFSWAP(struct elem_to_channel, e2c_vec[6], e2c_vec[2]);   // LFE1 third (final), FLc & FRc seventh
        FFSWAP(struct elem_to_channel, e2c_vec[4], e2c_vec[3]);   // BL & BR fourth (final), SiL & SiR fifth
        FFSWAP(struct elem_to_channel, e2c_vec[6], e2c_vec[4]);   // FLc & FRc fifth (final), SiL & SiR seventh
        FFSWAP(struct elem_to_channel, e2c_vec[7], e2c_vec[6]);   // LFE2 seventh (final), SiL & SiR eight (final)
        FFSWAP(struct elem_to_channel, e2c_vec[9], e2c_vec[8]);   // TpFL & TpFR ninth (final), TFC tenth (final)
        FFSWAP(struct elem_to_channel, e2c_vec[11], e2c_vec[10]); // TC eleventh (final), TpSiL & TpSiR twelth
        FFSWAP(struct elem_to_channel, e2c_vec[12], e2c_vec[11]); // TpBL & TpBR twelth (final), TpSiL & TpSiR thirteenth (final)
    } else {
        // For everything else, utilize the AV channel position define as a
        // stable sort.
                }

    }

        }
    }

    return layout;
}

/**
 * Save current output configuration if and only if it has been locked.
 */

    }
}

/**
 * Restore the previous output configuration if and only if the current
 * configuration is unlocked.
 */
static void pop_output_configuration(AACContext *ac) {
    if (ac->oc[1].status != OC_LOCKED && ac->oc[0].status != OC_NONE) {
        ac->oc[1] = ac->oc[0];
        ac->avctx->channels = ac->oc[1].channels;
        ac->avctx->channel_layout = ac->oc[1].channel_layout;
        output_configure(ac, ac->oc[1].layout_map, ac->oc[1].layout_map_tags,
                         ac->oc[1].status, 0);
    }
}

/**
 * Configure output channel order based on the current program
 * configuration element.
 *
 * @return  Returns error status. 0 - OK, !0 - error
 */
                            uint8_t layout_map[MAX_ELEM_ID * 4][3], int tags,
                            enum OCStatus oc_type, int get_new_frame)
{

    }
            avpriv_request_sample(ac->avctx, "Too large remapped id");
            return AVERROR_PATCHWELCOME;
        }
    }
    // Try to sniff a reasonable channel order, otherwise output the
    // channels in the order the PCE declared them.
        // Allocate or free elements depending on if they are in the
        // current program configuration.
            return ret;
    }
            layout = AV_CH_FRONT_LEFT|AV_CH_FRONT_RIGHT;
        } else {
            layout = 0;
        }
    }


            return ret;
    }

    return 0;
}

static void flush(AVCodecContext *avctx)
{
    AACContext *ac= avctx->priv_data;
    int type, i, j;

    for (type = 3; type >= 0; type--) {
        for (i = 0; i < MAX_ELEM_ID; i++) {
            ChannelElement *che = ac->che[type][i];
            if (che) {
                for (j = 0; j <= 1; j++) {
                    memset(che->ch[j].saved, 0, sizeof(che->ch[j].saved));
                }
            }
        }
    }
}

/**
 * Set up channel positions based on a default channel configuration
 * as specified in table 1.17.
 *
 * @return  Returns error status. 0 - OK, !0 - error
 */
                                      uint8_t (*layout_map)[3],
                                      int *tags,
                                      int channel_config)
{
        channel_config > 13) {
        av_log(avctx, AV_LOG_ERROR,
               "invalid default channel configuration (%d)\n",
               channel_config);
        return AVERROR_INVALIDDATA;
    }

    /*
     * AAC specification has 7.1(wide) as a default layout for 8-channel streams.
     * However, at least Nero AAC encoder encodes 7.1 streams using the default
     * channel config 7, mapping the side channels of the original audio stream
     * to the second AAC_CHANNEL_FRONT pair in the AAC stream. Similarly, e.g. FAAD
     * decodes the second AAC_CHANNEL_FRONT pair as side channels, therefore decoding
     * the incorrect streams as if they were correct (and as the encoder intended).
     *
     * As actual intended 7.1(wide) streams are very rare, default to assuming a
     * 7.1 layout was intended.
     */
        av_log(avctx, AV_LOG_INFO, "Assuming an incorrectly encoded 7.1 channel layout"
               " instead of a spec-compliant 7.1(wide) layout, use -strict %d to decode"
               " according to the specification instead.\n", FF_COMPLIANCE_STRICT);
        layout_map[2][2] = AAC_CHANNEL_SIDE;
    }

    return 0;
}

{
    /* For PCE based channel configurations map the channels solely based
     * on tags. */
    }
    // Allow single CPE stereo files to be signalled with mono configuration.
        ac->oc[1].m4ac.chan_config == 1) {
        uint8_t layout_map[MAX_ELEM_ID*4][3];
        int layout_map_tags;
        push_output_configuration(ac);

        av_log(ac->avctx, AV_LOG_DEBUG, "mono with CPE\n");

        if (set_default_channel_config(ac, ac->avctx, layout_map,
                                       &layout_map_tags, 2) < 0)
            return NULL;
        if (output_configure(ac, layout_map, layout_map_tags,
                             OC_TRIAL_FRAME, 1) < 0)
            return NULL;

        ac->oc[1].m4ac.chan_config = 2;
        ac->oc[1].m4ac.ps = 0;
    }
    // And vice-versa
        uint8_t layout_map[MAX_ELEM_ID * 4][3];
        int layout_map_tags;
        push_output_configuration(ac);

        av_log(ac->avctx, AV_LOG_DEBUG, "stereo with SCE\n");

        if (set_default_channel_config(ac, ac->avctx, layout_map,
                                       &layout_map_tags, 1) < 0)
            return NULL;
        if (output_configure(ac, layout_map, layout_map_tags,
                             OC_TRIAL_FRAME, 1) < 0)
            return NULL;

        ac->oc[1].m4ac.chan_config = 1;
        if (ac->oc[1].m4ac.sbr)
            ac->oc[1].m4ac.ps = -1;
    }
    /* For indexed channel configurations map the channels solely based
     * on position. */
    case 13:
        if (ac->tags_mapped > 3 && ((type == TYPE_CPE && elem_id < 8) ||
                                    (type == TYPE_SCE && elem_id < 6) ||
                                    (type == TYPE_LFE && elem_id < 2))) {
            ac->tags_mapped++;
            return ac->tag_che_map[type][elem_id] = ac->che[type][elem_id];
        }
    case 12:
    case 7:
        if (ac->tags_mapped == 3 && type == TYPE_CPE) {
            ac->tags_mapped++;
            return ac->tag_che_map[TYPE_CPE][elem_id] = ac->che[TYPE_CPE][2];
        }
    case 11:
        if (ac->tags_mapped == 2 &&
            ac->oc[1].m4ac.chan_config == 11 &&
            type == TYPE_SCE) {
            ac->tags_mapped++;
            return ac->tag_che_map[TYPE_SCE][elem_id] = ac->che[TYPE_SCE][1];
        }
    case 6:
        /* Some streams incorrectly code 5.1 audio as
         * SCE[0] CPE[0] CPE[1] SCE[1]
         * instead of
         * SCE[0] CPE[0] CPE[1] LFE[0].
         * If we seem to have encountered such a stream, transfer
         * the LFE[0] element to the SCE[1]'s mapping */
                av_log(ac->avctx, AV_LOG_WARNING,
                   "This stream seems to incorrectly report its last channel as %s[%d], mapping to LFE[0]\n",
                   type == TYPE_SCE ? "SCE" : "LFE", elem_id);
                ac->warned_remapping_once++;
            }
        }
    case 5:
        }
    case 4:
        /* Some streams incorrectly code 4.0 audio as
         * SCE[0] CPE[0] LFE[0]
         * instead of
         * SCE[0] CPE[0] SCE[1].
         * If we seem to have encountered such a stream, transfer
         * the SCE[1] element to the LFE[0]'s mapping */
            if (!ac->warned_remapping_once && (type != TYPE_SCE || elem_id != 1)) {
                av_log(ac->avctx, AV_LOG_WARNING,
                   "This stream seems to incorrectly report its last channel as %s[%d], mapping to SCE[1]\n",
                   type == TYPE_SCE ? "SCE" : "LFE", elem_id);
                ac->warned_remapping_once++;
            }
            ac->tags_mapped++;
            return ac->tag_che_map[type][elem_id] = ac->che[TYPE_SCE][1];
        }
            ac->oc[1].m4ac.chan_config == 4 &&
            type == TYPE_SCE) {
            ac->tags_mapped++;
            return ac->tag_che_map[TYPE_SCE][elem_id] = ac->che[TYPE_SCE][1];
        }
    case 3:
    case 2:
            type == TYPE_CPE) {
            return NULL;
        }
    case 1:
        }
    default:
        return NULL;
    }
}

/**
 * Decode an array of 4 bit element IDs, optionally interleaved with a
 * stereo/mono switching bit.
 *
 * @param type speaker type/position for these channels
 */
                               enum ChannelPosition type,
                               GetBitContext *gb, int n)
{
        case AAC_CHANNEL_FRONT:
        case AAC_CHANNEL_BACK:
        case AAC_CHANNEL_SIDE:
        case AAC_CHANNEL_CC:
        case AAC_CHANNEL_LFE:
            syn_ele = TYPE_LFE;
            break;
        default:
            // AAC_CHANNEL_OFF has no channel map
            av_assert0(0);
        }
    }

static inline void relative_align_get_bits(GetBitContext *gb,
                                           int reference_position) {
    int n = (reference_position - get_bits_count(gb) & 7);
    if (n)
        skip_bits(gb, n);
}

/**
 * Decode program configuration element; reference: table 4.2.
 *
 * @return  Returns error status. 0 - OK, !0 - error
 */
static int decode_pce(AVCodecContext *avctx, MPEG4AudioConfig *m4ac,
                      uint8_t (*layout_map)[3],
                      GetBitContext *gb, int byte_align_ref)
{
    int num_front, num_side, num_back, num_lfe, num_assoc_data, num_cc;
    int sampling_index;
    int comment_len;
    int tags;

    skip_bits(gb, 2);  // object_type

    sampling_index = get_bits(gb, 4);
    if (m4ac->sampling_index != sampling_index)
        av_log(avctx, AV_LOG_WARNING,
               "Sample rate index in program config element does not "
               "match the sample rate index configured by the container.\n");

    num_front       = get_bits(gb, 4);
    num_side        = get_bits(gb, 4);
    num_back        = get_bits(gb, 4);
    num_lfe         = get_bits(gb, 2);
    num_assoc_data  = get_bits(gb, 3);
    num_cc          = get_bits(gb, 4);

    if (get_bits1(gb))
        skip_bits(gb, 4); // mono_mixdown_tag
    if (get_bits1(gb))
        skip_bits(gb, 4); // stereo_mixdown_tag

    if (get_bits1(gb))
        skip_bits(gb, 3); // mixdown_coeff_index and pseudo_surround

    if (get_bits_left(gb) < 5 * (num_front + num_side + num_back + num_cc) + 4 *(num_lfe + num_assoc_data + num_cc)) {
        av_log(avctx, AV_LOG_ERROR, "decode_pce: " overread_err);
        return -1;
    }
    decode_channel_map(layout_map       , AAC_CHANNEL_FRONT, gb, num_front);
    tags = num_front;
    decode_channel_map(layout_map + tags, AAC_CHANNEL_SIDE,  gb, num_side);
    tags += num_side;
    decode_channel_map(layout_map + tags, AAC_CHANNEL_BACK,  gb, num_back);
    tags += num_back;
    decode_channel_map(layout_map + tags, AAC_CHANNEL_LFE,   gb, num_lfe);
    tags += num_lfe;

    skip_bits_long(gb, 4 * num_assoc_data);

    decode_channel_map(layout_map + tags, AAC_CHANNEL_CC,    gb, num_cc);
    tags += num_cc;

    relative_align_get_bits(gb, byte_align_ref);

    /* comment field, first byte is length */
    comment_len = get_bits(gb, 8) * 8;
    if (get_bits_left(gb) < comment_len) {
        av_log(avctx, AV_LOG_ERROR, "decode_pce: " overread_err);
        return AVERROR_INVALIDDATA;
    }
    skip_bits_long(gb, comment_len);
    return tags;
}

/**
 * Decode GA "General Audio" specific configuration; reference: table 4.1.
 *
 * @param   ac          pointer to AACContext, may be null
 * @param   avctx       pointer to AVCCodecContext, used for logging
 *
 * @return  Returns error status. 0 - OK, !0 - error
 */
                                     GetBitContext *gb,
                                     int get_bit_alignment,
                                     MPEG4AudioConfig *m4ac,
                                     int channel_config)
{

#if USE_FIXED
        avpriv_report_missing_feature(avctx, "Fixed point 960/120 MDCT window");
        return AVERROR_PATCHWELCOME;
    }
#else
      avpriv_report_missing_feature(avctx, "SBR with 960 frame length");
      if (ac) ac->warned_960_sbr = 1;
      m4ac->sbr = 0;
      m4ac->ps = 0;
    }
#endif

        skip_bits(gb, 14);   // coreCoderDelay

        m4ac->object_type == AOT_ER_AAC_SCALABLE)
        skip_bits(gb, 3);     // layerNr

            return tags;
    } else {
                                              &tags, channel_config)))
            return ret;
    }


        return ret;

        case AOT_ER_BSAC:
            skip_bits(gb, 5);    // numOfSubFrame
            skip_bits(gb, 11);   // layer_length
            break;
        case AOT_ER_AAC_LTP:
        case AOT_ER_AAC_SCALABLE:
        case AOT_ER_AAC_LD:
                avpriv_report_missing_feature(avctx,
                                              "AAC data resilience (flags %x)",
                                              res_flags);
                return AVERROR_PATCHWELCOME;
            }
            break;
        }
    }
    case AOT_ER_AAC_LTP:
    case AOT_ER_AAC_SCALABLE:
    case AOT_ER_AAC_LD:
            avpriv_report_missing_feature(avctx,
                                          "epConfig %d", ep_config);
            return AVERROR_PATCHWELCOME;
        }
    }
    return 0;
}

                                     GetBitContext *gb,
                                     MPEG4AudioConfig *m4ac,
                                     int channel_config)
{

#if USE_FIXED
    if (get_bits1(gb)) { // frameLengthFlag
        avpriv_request_sample(avctx, "960/120 MDCT window");
        return AVERROR_PATCHWELCOME;
    }
#else
#endif
        avpriv_report_missing_feature(avctx,
                                      "AAC data resilience (flags %x)",
                                      res_flags);
        return AVERROR_PATCHWELCOME;
    }

        avpriv_report_missing_feature(avctx,
                                      "Low Delay SBR");
        return AVERROR_PATCHWELCOME;
    }

        int len = get_bits(gb, 4);
        if (len == 15)
            len += get_bits(gb, 8);
        if (len == 15 + 255)
            len += get_bits(gb, 16);
        if (get_bits_left(gb) < len * 8 + 4) {
            av_log(avctx, AV_LOG_ERROR, overread_err);
            return AVERROR_INVALIDDATA;
        }
    }

                                          &tags, channel_config)))
        return ret;

        return ret;

        avpriv_report_missing_feature(avctx,
                                      "epConfig %d", ep_config);
        return AVERROR_PATCHWELCOME;
    }
    return 0;
}

/**
 * Decode audio specific configuration; reference: table 1.13.
 *
 * @param   ac          pointer to AACContext, may be null
 * @param   avctx       pointer to AVCCodecContext, used for logging
 * @param   m4ac        pointer to MPEG4AudioConfig, used for parsing
 * @param   gb          buffer holding an audio specific config
 * @param   get_bit_alignment relative alignment for byte align operations
 * @param   sync_extension look for an appended sync extension
 *
 * @return  Returns error status or number of consumed bits. <0 - error
 */
                                           AVCodecContext *avctx,
                                           MPEG4AudioConfig *m4ac,
                                           GetBitContext *gb,
                                           int get_bit_alignment,
                                           int sync_extension)
{

        return AVERROR_INVALIDDATA;

        av_log(avctx, AV_LOG_ERROR,
               "invalid sampling rate index %d\n",
               m4ac->sampling_index);
        return AVERROR_INVALIDDATA;
    }
        av_log(avctx, AV_LOG_ERROR,
               "invalid low delay sampling rate index %d\n",
               m4ac->sampling_index);
        return AVERROR_INVALIDDATA;
    }


    case AOT_AAC_LC:
    case AOT_AAC_SSR:
    case AOT_AAC_LTP:
    case AOT_ER_AAC_LC:
    case AOT_ER_AAC_LD:
                                            m4ac, m4ac->chan_config)) < 0)
            return ret;
        break;
                                              m4ac, m4ac->chan_config)) < 0)
            return ret;
        break;
    default:
        avpriv_report_missing_feature(avctx,
                                      "Audio object type %s%d",
                                      m4ac->sbr == 1 ? "SBR+" : "",
                                      m4ac->object_type);
        return AVERROR(ENOSYS);
    }

            "AOT %d chan config %d sampling index %d (%d) SBR %d PS %d\n",
            m4ac->object_type, m4ac->chan_config, m4ac->sampling_index,
            m4ac->sample_rate, m4ac->sbr,
            m4ac->ps);

}

                                        AVCodecContext *avctx,
                                        MPEG4AudioConfig *m4ac,
                                        const uint8_t *data, int64_t bit_size,
                                        int sync_extension)
{

        av_log(avctx, AV_LOG_ERROR, "Audio specific config size is invalid\n");
        return AVERROR_INVALIDDATA;
    }

    ff_dlog(avctx, "audio specific config size %d\n", (int)bit_size >> 3);
        ff_dlog(avctx, "%02x ", data[i]);

        return ret;

                                           sync_extension);
}

/**
 * linear congruential pseudorandom number generator
 *
 * @param   previous_val    pointer to the current state of the generator
 *
 * @return  Returns a 32-bit pseudorandom integer
 */
{
}

static void reset_all_predictors(PredictorState *ps)
{
    int i;
    for (i = 0; i < MAX_PREDICTORS; i++)
        reset_predict_state(&ps[i]);
}

{
}

static void reset_predictor_group(PredictorState *ps, int group_num)
{
    int i;
    for (i = group_num - 1; i < MAX_PREDICTORS; i += 30)
        reset_predict_state(&ps[i]);
}

#define AAC_INIT_VLC_STATIC(num, size)                                     \
    INIT_VLC_STATIC(&vlc_spectral[num], 8, ff_aac_spectral_sizes[num],     \
         ff_aac_spectral_bits[num], sizeof(ff_aac_spectral_bits[num][0]),  \
                                    sizeof(ff_aac_spectral_bits[num][0]),  \
        ff_aac_spectral_codes[num], sizeof(ff_aac_spectral_codes[num][0]), \
                                    sizeof(ff_aac_spectral_codes[num][0]), \
        size);

static void aacdec_init(AACContext *ac);

{



                    FF_ARRAY_ELEMS(ff_aac_scalefactor_code),
                    ff_aac_scalefactor_bits,
                    sizeof(ff_aac_scalefactor_bits[0]),
                    sizeof(ff_aac_scalefactor_bits[0]),
                    ff_aac_scalefactor_code,
                    sizeof(ff_aac_scalefactor_code[0]),
                    sizeof(ff_aac_scalefactor_code[0]),
                    352);

    // window initialization
#if !USE_FIXED
#endif


static AVOnce aac_table_init = AV_ONCE_INIT;

{

        return AVERROR_INVALIDDATA;

        return AVERROR_UNKNOWN;


#if USE_FIXED
#else
#endif /* USE_FIXED */

                                                1)) < 0)
            return ret;
    } else {


                break;
            i = 0;
        }

                &layout_map_tags, ac->oc[1].m4ac.chan_config);
                                 OC_GLOBAL_HDR, 0);
            else if (avctx->err_recognition & AV_EF_EXPLODE)
                return AVERROR_INVALIDDATA;
        }
    }

        av_log(avctx, AV_LOG_ERROR, "Too many channels\n");
        return AVERROR_INVALIDDATA;
    }

#if USE_FIXED
#else
#endif /* USE_FIXED */
        return AVERROR(ENOMEM);
    }


#if !USE_FIXED
        return ret;
        return ret;
        return ret;
#endif

}

/**
 * Skip data_stream_element; reference: table 4.10.
 */
static int skip_data_stream_element(AACContext *ac, GetBitContext *gb)
{
    int byte_align = get_bits1(gb);
    int count = get_bits(gb, 8);
    if (count == 255)
        count += get_bits(gb, 8);
    if (byte_align)
        align_get_bits(gb);

    if (get_bits_left(gb) < 8 * count) {
        av_log(ac->avctx, AV_LOG_ERROR, "skip_data_stream_element: "overread_err);
        return AVERROR_INVALIDDATA;
    }
    skip_bits_long(gb, 8 * count);
    return 0;
}

static int decode_prediction(AACContext *ac, IndividualChannelStream *ics,
                             GetBitContext *gb)
{
    int sfb;
    if (get_bits1(gb)) {
        ics->predictor_reset_group = get_bits(gb, 5);
        if (ics->predictor_reset_group == 0 ||
            ics->predictor_reset_group > 30) {
            av_log(ac->avctx, AV_LOG_ERROR,
                   "Invalid Predictor Reset Group.\n");
            return AVERROR_INVALIDDATA;
        }
    }
    for (sfb = 0; sfb < FFMIN(ics->max_sfb, ff_aac_pred_sfb_max[ac->oc[1].m4ac.sampling_index]); sfb++) {
        ics->prediction_used[sfb] = get_bits1(gb);
    }
    return 0;
}

/**
 * Decode Long Term Prediction data; reference: table 4.xx.
 */
                       GetBitContext *gb, uint8_t max_sfb)
{


/**
 * Decode Individual Channel Stream info; reference: table 4.6.
 */
                           GetBitContext *gb)
{

            av_log(ac->avctx, AV_LOG_ERROR, "Reserved bit set.\n");
            if (ac->avctx->err_recognition & AV_EF_BITSTREAM)
                return AVERROR_INVALIDDATA;
        }
            ics->window_sequence[0] != ONLY_LONG_SEQUENCE) {
            av_log(ac->avctx, AV_LOG_ERROR,
                   "AAC LD is only defined for ONLY_LONG_SEQUENCE but "
                   "window sequence %d found.\n", ics->window_sequence[0]);
            ics->window_sequence[0] = ONLY_LONG_SEQUENCE;
            return AVERROR_INVALIDDATA;
        }
    }
            } else {
            }
        }
        } else {
        }
    } else {
            } else {
            }
                ret_fail = AVERROR_BUG;
                goto fail;
            }
        } else {
            } else {
            }
        }
        }
                    goto fail;
                }
                av_log(ac->avctx, AV_LOG_ERROR,
                       "Prediction is not allowed in AAC-LC.\n");
                goto fail;
            } else {
                    av_log(ac->avctx, AV_LOG_ERROR,
                           "LTP in ER AAC LD not yet implemented.\n");
                    ret_fail = AVERROR_PATCHWELCOME;
                    goto fail;
                }
            }
        }
    }

        av_log(ac->avctx, AV_LOG_ERROR,
               "Number of scalefactor bands in group (%d) "
               "exceeds limit (%d).\n",
               ics->max_sfb, ics->num_swb);
        goto fail;
    }

    return 0;
fail:
    ics->max_sfb = 0;
    return ret_fail;
}

/**
 * Decode band types (section_data payload); reference: table 4.46.
 *
 * @param   band_type           array of the used band type
 * @param   band_type_run_end   array of the last scalefactor band of a band type run
 *
 * @return  Returns error status. 0 - OK, !0 - error
 */
static int decode_band_types(AACContext *ac, enum BandType band_type[120],
                             int band_type_run_end[120], GetBitContext *gb,
                             IndividualChannelStream *ics)
{
    int g, idx = 0;
    const int bits = (ics->window_sequence[0] == EIGHT_SHORT_SEQUENCE) ? 3 : 5;
    for (g = 0; g < ics->num_window_groups; g++) {
        int k = 0;
        while (k < ics->max_sfb) {
            uint8_t sect_end = k;
            int sect_len_incr;
            int sect_band_type = get_bits(gb, 4);
            if (sect_band_type == 12) {
                av_log(ac->avctx, AV_LOG_ERROR, "invalid band type\n");
                return AVERROR_INVALIDDATA;
            }
            do {
                sect_len_incr = get_bits(gb, bits);
                sect_end += sect_len_incr;
                if (get_bits_left(gb) < 0) {
                    av_log(ac->avctx, AV_LOG_ERROR, "decode_band_types: "overread_err);
                    return AVERROR_INVALIDDATA;
                }
                if (sect_end > ics->max_sfb) {
                    av_log(ac->avctx, AV_LOG_ERROR,
                           "Number of bands (%d) exceeds limit (%d).\n",
                           sect_end, ics->max_sfb);
                    return AVERROR_INVALIDDATA;
                }
            } while (sect_len_incr == (1 << bits) - 1);
            for (; k < sect_end; k++) {
                band_type        [idx]   = sect_band_type;
                band_type_run_end[idx++] = sect_end;
            }
        }
    }
    return 0;
}

/**
 * Decode scalefactors; reference: table 4.47.
 *
 * @param   global_gain         first scalefactor value as scalefactors are differentially coded
 * @param   band_type           array of the used band type
 * @param   band_type_run_end   array of the last scalefactor band of a band type run
 * @param   sf                  array of scalefactors or intensity stereo positions
 *
 * @return  Returns error status. 0 - OK, !0 - error
 */
static int decode_scalefactors(AACContext *ac, INTFLOAT sf[120], GetBitContext *gb,
                               unsigned int global_gain,
                               IndividualChannelStream *ics,
                               enum BandType band_type[120],
                               int band_type_run_end[120])
{
    int g, i, idx = 0;
    int offset[3] = { global_gain, global_gain - NOISE_OFFSET, 0 };
    int clipped_offset;
    int noise_flag = 1;
    for (g = 0; g < ics->num_window_groups; g++) {
        for (i = 0; i < ics->max_sfb;) {
            int run_end = band_type_run_end[idx];
            if (band_type[idx] == ZERO_BT) {
                for (; i < run_end; i++, idx++)
                    sf[idx] = FIXR(0.);
            } else if ((band_type[idx] == INTENSITY_BT) ||
                       (band_type[idx] == INTENSITY_BT2)) {
                for (; i < run_end; i++, idx++) {
                    offset[2] += get_vlc2(gb, vlc_scalefactors.table, 7, 3) - SCALE_DIFF_ZERO;
                    clipped_offset = av_clip(offset[2], -155, 100);
                    if (offset[2] != clipped_offset) {
                        avpriv_request_sample(ac->avctx,
                                              "If you heard an audible artifact, there may be a bug in the decoder. "
                                              "Clipped intensity stereo position (%d -> %d)",
                                              offset[2], clipped_offset);
                    }
#if USE_FIXED
                    sf[idx] = 100 - clipped_offset;
#else
                    sf[idx] = ff_aac_pow2sf_tab[-clipped_offset + POW_SF2_ZERO];
#endif /* USE_FIXED */
                }
            } else if (band_type[idx] == NOISE_BT) {
                for (; i < run_end; i++, idx++) {
                    if (noise_flag-- > 0)
                        offset[1] += get_bits(gb, NOISE_PRE_BITS) - NOISE_PRE;
                    else
                        offset[1] += get_vlc2(gb, vlc_scalefactors.table, 7, 3) - SCALE_DIFF_ZERO;
                    clipped_offset = av_clip(offset[1], -100, 155);
                    if (offset[1] != clipped_offset) {
                        avpriv_request_sample(ac->avctx,
                                              "If you heard an audible artifact, there may be a bug in the decoder. "
                                              "Clipped noise gain (%d -> %d)",
                                              offset[1], clipped_offset);
                    }
#if USE_FIXED
                    sf[idx] = -(100 + clipped_offset);
#else
                    sf[idx] = -ff_aac_pow2sf_tab[clipped_offset + POW_SF2_ZERO];
#endif /* USE_FIXED */
                }
            } else {
                for (; i < run_end; i++, idx++) {
                    offset[0] += get_vlc2(gb, vlc_scalefactors.table, 7, 3) - SCALE_DIFF_ZERO;
                    if (offset[0] > 255U) {
                        av_log(ac->avctx, AV_LOG_ERROR,
                               "Scalefactor (%d) out of range.\n", offset[0]);
                        return AVERROR_INVALIDDATA;
                    }
#if USE_FIXED
                    sf[idx] = -offset[0];
#else
                    sf[idx] = -ff_aac_pow2sf_tab[offset[0] - 100 + POW_SF2_ZERO];
#endif /* USE_FIXED */
                }
            }
        }
    }
    return 0;
}

/**
 * Decode pulse data; reference: table 4.7.
 */
                         const uint16_t *swb_offset, int num_swb)
{
        return -1;
        return -1;
            return -1;
    }
    return 0;
}

/**
 * Decode Temporal Noise Shaping data; reference: table 4.48.
 *
 * @return  Returns error status. 0 - OK, !0 - error
 */
static int decode_tns(AACContext *ac, TemporalNoiseShaping *tns,
                      GetBitContext *gb, const IndividualChannelStream *ics)
{
    int w, filt, i, coef_len, coef_res, coef_compress;
    const int is8 = ics->window_sequence[0] == EIGHT_SHORT_SEQUENCE;
    const int tns_max_order = is8 ? 7 : ac->oc[1].m4ac.object_type == AOT_AAC_MAIN ? 20 : 12;
    for (w = 0; w < ics->num_windows; w++) {
        if ((tns->n_filt[w] = get_bits(gb, 2 - is8))) {
            coef_res = get_bits1(gb);

            for (filt = 0; filt < tns->n_filt[w]; filt++) {
                int tmp2_idx;
                tns->length[w][filt] = get_bits(gb, 6 - 2 * is8);

                if ((tns->order[w][filt] = get_bits(gb, 5 - 2 * is8)) > tns_max_order) {
                    av_log(ac->avctx, AV_LOG_ERROR,
                           "TNS filter order %d is greater than maximum %d.\n",
                           tns->order[w][filt], tns_max_order);
                    tns->order[w][filt] = 0;
                    return AVERROR_INVALIDDATA;
                }
                if (tns->order[w][filt]) {
                    tns->direction[w][filt] = get_bits1(gb);
                    coef_compress = get_bits1(gb);
                    coef_len = coef_res + 3 - coef_compress;
                    tmp2_idx = 2 * coef_compress + coef_res;

                    for (i = 0; i < tns->order[w][filt]; i++)
                        tns->coef[w][filt][i] = tns_tmp2_map[tmp2_idx][get_bits(gb, coef_len)];
                }
            }
        }
    }
    return 0;
}

/**
 * Decode Mid/Side data; reference: table 4.54.
 *
 * @param   ms_present  Indicates mid/side stereo presence. [0] mask is all 0s;
 *                      [1] mask is decoded from bitstream; [2] mask is all 1s;
 *                      [3] reserved for scalable AAC
 */
                                   int ms_present)
{
    }

/**
 * Decode spectral data; reference: table 4.50.
 * Dequantize and scale spectral data; reference: 4.6.3.3.
 *
 * @param   coef            array of dequantized, scaled spectral data
 * @param   sf              array of scalefactors or intensity stereo positions
 * @param   pulse_present   set if pulses are present
 * @param   pulse           pointer to pulse data struct
 * @param   band_type       array of the used band type
 *
 * @return  Returns error status. 0 - OK, !0 - error
 */
                                       GetBitContext *gb, const INTFLOAT sf[120],
                                       int pulse_present, const Pulse *pulse,
                                       const IndividualChannelStream *ics,
                                       enum BandType band_type[120])
{




                }
                    INTFLOAT band_energy;
#if USE_FIXED
                    }

#else
                    float scale;

                    }

#endif /* USE_FIXED */
                }
            } else {
#if !USE_FIXED
#endif /* !USE_FIXED */

                case 0:
                        INTFLOAT *cf = cfo;
                        int len = off_len;


#if USE_FIXED
#else
#endif /* USE_FIXED */
                    }
                    break;

                case 1:
                        INTFLOAT *cf = cfo;
                        int len = off_len;


#if USE_FIXED
#else
#endif /* USE_FIXED */
                    }
                    break;

                case 2:
                        INTFLOAT *cf = cfo;
                        int len = off_len;


#if USE_FIXED
#else
#endif /* USE_FIXED */
                    }
                    break;

                case 3:
                case 4:
                        INTFLOAT *cf = cfo;
                        int len = off_len;


#if USE_FIXED
#else
#endif /* USE_FIXED */
                    }
                    break;

                default:
#if USE_FIXED
                        int *icf = cfo;
                        int v;
#else
                        uint32_t *icf = (uint32_t *) cf;
#endif /* USE_FIXED */
                        int len = off_len;



                            }


                                    /* The total length of escape_sequence must be < 22 bits according
                                       to the specification (i.e. max is 111111110xxxxxxxxxxxx). */

                                        av_log(ac->avctx, AV_LOG_ERROR, "error in spectral data, ESC overflow\n");
                                        return AVERROR_INVALIDDATA;
                                    }

#if USE_FIXED
#else
#endif /* USE_FIXED */
                                } else {
#if USE_FIXED
#else
#endif /* USE_FIXED */
                                }
                            }
#if !USE_FIXED
#endif /* !USE_FIXED */
                    }
                }

            }
        }
    }

        idx = 0;
#if USE_FIXED
                }
#else
                }
#endif /* USE_FIXED */
            }
        }
    }
#if USE_FIXED
    coef = coef_base;
    idx = 0;


                }
            }
        }
    }
#endif /* USE_FIXED */
    return 0;
}

/**
 * Apply AAC-Main style frequency domain prediction.
 */
static void apply_prediction(AACContext *ac, SingleChannelElement *sce)
{
    int sfb, k;

    if (!sce->ics.predictor_initialized) {
        reset_all_predictors(sce->predictor_state);
        sce->ics.predictor_initialized = 1;
    }

    if (sce->ics.window_sequence[0] != EIGHT_SHORT_SEQUENCE) {
        for (sfb = 0;
             sfb < ff_aac_pred_sfb_max[ac->oc[1].m4ac.sampling_index];
             sfb++) {
            for (k = sce->ics.swb_offset[sfb];
                 k < sce->ics.swb_offset[sfb + 1];
                 k++) {
                predict(&sce->predictor_state[k], &sce->coeffs[k],
                        sce->ics.predictor_present &&
                        sce->ics.prediction_used[sfb]);
            }
        }
        if (sce->ics.predictor_reset_group)
            reset_predictor_group(sce->predictor_state,
                                  sce->ics.predictor_reset_group);
    } else
        reset_all_predictors(sce->predictor_state);
}

static void decode_gain_control(SingleChannelElement * sce, GetBitContext * gb)
{
    // wd_num, wd_test, aloc_size
    static const uint8_t gain_mode[4][3] = {
        {1, 0, 5},  // ONLY_LONG_SEQUENCE = 0,
        {2, 1, 2},  // LONG_START_SEQUENCE,
        {8, 0, 2},  // EIGHT_SHORT_SEQUENCE,
        {2, 1, 5},  // LONG_STOP_SEQUENCE
    };

    const int mode = sce->ics.window_sequence[0];
    uint8_t bd, wd, ad;

    // FIXME: Store the gain control data on |sce| and do something with it.
    uint8_t max_band = get_bits(gb, 2);
    for (bd = 0; bd < max_band; bd++) {
        for (wd = 0; wd < gain_mode[mode][0]; wd++) {
            uint8_t adjust_num = get_bits(gb, 3);
            for (ad = 0; ad < adjust_num; ad++) {
                skip_bits(gb, 4 + ((wd == 0 && gain_mode[mode][1])
                                     ? 4
                                     : gain_mode[mode][2]));
            }
        }
    }
}

/**
 * Decode an individual_channel_stream payload; reference: table 4.44.
 *
 * @param   common_window   Channels have independent [0], or shared [1], Individual Channel Stream information.
 * @param   scale_flag      scalable [1] or non-scalable [0] AAC (Unused until scalable AAC is implemented.)
 *
 * @return  Returns error status. 0 - OK, !0 - error
 */
                      GetBitContext *gb, int common_window, int scale_flag)
{

                 ac->oc[1].m4ac.object_type == AOT_ER_AAC_ELD;

    /* This assignment is to silence a GCC warning about the variable being used
     * uninitialized when in fact it always is.
     */


            goto fail;
    }

        goto fail;
                                  sce->band_type, sce->band_type_run_end)) < 0)
        goto fail;

                av_log(ac->avctx, AV_LOG_ERROR,
                       "Pulse tool not allowed in eight short sequence.\n");
                ret = AVERROR_INVALIDDATA;
                goto fail;
            }
                av_log(ac->avctx, AV_LOG_ERROR,
                       "Pulse data corrupt or invalid.\n");
                ret = AVERROR_INVALIDDATA;
                goto fail;
            }
        }
                goto fail;
        }
            decode_gain_control(sce, gb);
            if (!ac->warned_gain_control) {
                avpriv_report_missing_feature(ac->avctx, "Gain control");
                ac->warned_gain_control = 1;
            }
        }
        // I see no textual basis in the spec for this occurring after SSR gain
        // control, but this is what both reference and real implmentations do
                goto fail;
        }
    }

                                    &pulse, ics, sce->band_type);
        goto fail;


    return 0;
fail:
    tns->present = 0;
    return ret;
}

/**
 * Mid/Side stereo decoding; reference: 4.6.8.1.3.
 */
static void apply_mid_side_stereo(AACContext *ac, ChannelElement *cpe)
{
    const IndividualChannelStream *ics = &cpe->ch[0].ics;
    INTFLOAT *ch0 = cpe->ch[0].coeffs;
    INTFLOAT *ch1 = cpe->ch[1].coeffs;
    int g, i, group, idx = 0;
    const uint16_t *offsets = ics->swb_offset;
    for (g = 0; g < ics->num_window_groups; g++) {
        for (i = 0; i < ics->max_sfb; i++, idx++) {
            if (cpe->ms_mask[idx] &&
                cpe->ch[0].band_type[idx] < NOISE_BT &&
                cpe->ch[1].band_type[idx] < NOISE_BT) {
#if USE_FIXED
                for (group = 0; group < ics->group_len[g]; group++) {
                    ac->fdsp->butterflies_fixed(ch0 + group * 128 + offsets[i],
                                                ch1 + group * 128 + offsets[i],
                                                offsets[i+1] - offsets[i]);
#else
                for (group = 0; group < ics->group_len[g]; group++) {
                    ac->fdsp->butterflies_float(ch0 + group * 128 + offsets[i],
                                               ch1 + group * 128 + offsets[i],
                                               offsets[i+1] - offsets[i]);
#endif /* USE_FIXED */
                }
            }
        }
        ch0 += ics->group_len[g] * 128;
        ch1 += ics->group_len[g] * 128;
    }
}

/**
 * intensity stereo decoding; reference: 4.6.8.2.3
 *
 * @param   ms_present  Indicates mid/side stereo presence. [0] mask is all 0s;
 *                      [1] mask is decoded from bitstream; [2] mask is all 1s;
 *                      [3] reserved for scalable AAC
 */
static void apply_intensity_stereo(AACContext *ac,
                                   ChannelElement *cpe, int ms_present)
{
    const IndividualChannelStream *ics = &cpe->ch[1].ics;
    SingleChannelElement         *sce1 = &cpe->ch[1];
    INTFLOAT *coef0 = cpe->ch[0].coeffs, *coef1 = cpe->ch[1].coeffs;
    const uint16_t *offsets = ics->swb_offset;
    int g, group, i, idx = 0;
    int c;
    INTFLOAT scale;
    for (g = 0; g < ics->num_window_groups; g++) {
        for (i = 0; i < ics->max_sfb;) {
            if (sce1->band_type[idx] == INTENSITY_BT ||
                sce1->band_type[idx] == INTENSITY_BT2) {
                const int bt_run_end = sce1->band_type_run_end[idx];
                for (; i < bt_run_end; i++, idx++) {
                    c = -1 + 2 * (sce1->band_type[idx] - 14);
                    if (ms_present)
                        c *= 1 - 2 * cpe->ms_mask[idx];
                    scale = c * sce1->sf[idx];
                    for (group = 0; group < ics->group_len[g]; group++)
#if USE_FIXED
                        ac->subband_scale(coef1 + group * 128 + offsets[i],
                                      coef0 + group * 128 + offsets[i],
                                      scale,
                                      23,
                                      offsets[i + 1] - offsets[i] ,ac->avctx);
#else
                        ac->fdsp->vector_fmul_scalar(coef1 + group * 128 + offsets[i],
                                                    coef0 + group * 128 + offsets[i],
                                                    scale,
                                                    offsets[i + 1] - offsets[i]);
#endif /* USE_FIXED */
                }
            } else {
                int bt_run_end = sce1->band_type_run_end[idx];
                idx += bt_run_end - i;
                i    = bt_run_end;
            }
        }
        coef0 += ics->group_len[g] * 128;
        coef1 += ics->group_len[g] * 128;
    }
}

/**
 * Decode a channel_pair_element; reference: table 4.4.
 *
 * @return  Returns error status. 0 - OK, !0 - error
 */
{

            return AVERROR_INVALIDDATA;
            av_log(ac->avctx, AV_LOG_ERROR, "ms_present = 3 is reserved.\n");
            return AVERROR_INVALIDDATA;
    }
        return ret;
        return ret;

        }
    }

}

static const float cce_scale[] = {
    1.09050773266525765921, //2^(1/8)
    1.18920711500272106672, //2^(1/4)
    M_SQRT2,
    2,
};

/**
 * Decode coupling_channel_element; reference: table 4.8.
 *
 * @return  Returns error status. 0 - OK, !0 - error
 */
{

        } else
    }

#if USE_FIXED
#else
#endif

        return ret;

#if USE_FIXED
                return AVERROR(ERANGE);
#endif
        }
        } else {
                                }
#if USE_FIXED
                                if ((abs(gain_cache)-1024) >> 3 > 30)
                                    return AVERROR(ERANGE);
#endif
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}

/**
 * Parse whether channels are to be excluded from Dynamic Range Compression; reference: table 4.53.
 *
 * @return  Returns number of bytes consumed.
 */
                                         GetBitContext *gb)
{


}

/**
 * Decode dynamic range information; reference: table 4.52.
 *
 * @return  Returns number of bytes consumed.
 */
                                GetBitContext *gb)
{

    /* pce_tag_present? */
        che_drc->pce_instance_tag  = get_bits(gb, 4);
        skip_bits(gb, 4); // tag_reserved_bits
        n++;
    }

    /* excluded_chns_present? */
    }

    /* drc_bands_present? */
        }
    }

    /* prog_ref_level_present? */
    }

    }

}

static int decode_fill(AACContext *ac, GetBitContext *gb, int len) {
    uint8_t buf[256];
    int i, major, minor;

    if (len < 13+7*8)
        goto unknown;

    get_bits(gb, 13); len -= 13;

    for(i=0; i+1<sizeof(buf) && len>=8; i++, len-=8)
        buf[i] = get_bits(gb, 8);

    buf[i] = 0;
    if (ac->avctx->debug & FF_DEBUG_PICT_INFO)
        av_log(ac->avctx, AV_LOG_DEBUG, "FILL:%s\n", buf);

    if (sscanf(buf, "libfaac %d.%d", &major, &minor) == 2){
        ac->avctx->internal->skip_samples = 1024;
    }

unknown:
    skip_bits_long(gb, len);

    return 0;
}

/**
 * Decode extension data (incomplete); reference: table 4.51.
 *
 * @param   cnt length of TYPE_FIL syntactic element in bytes
 *
 * @return Returns number of bytes consumed
 */
                                    ChannelElement *che, enum RawDataBlockType elem_type)
{

        av_log(ac->avctx, AV_LOG_DEBUG, "extension type: %d len:%d\n", type, cnt);

    case EXT_SBR_DATA_CRC:
        crc_flag++;
            av_log(ac->avctx, AV_LOG_ERROR, "SBR was found before the first channel element.\n");
            return res;
            if (!ac->warned_960_sbr)
              avpriv_report_missing_feature(ac->avctx,
                                            "SBR with 960 frame length");
            ac->warned_960_sbr = 1;
            skip_bits_long(gb, 8 * cnt - 4);
            return res;
            av_log(ac->avctx, AV_LOG_ERROR, "SBR signaled to be not-present but was found in the bitstream.\n");
            skip_bits_long(gb, 8 * cnt - 4);
            return res;
            av_log(ac->avctx, AV_LOG_ERROR, "Implicit SBR was found with a first occurrence after the first frame.\n");
            skip_bits_long(gb, 8 * cnt - 4);
            return res;
                             ac->oc[1].status, 1);
        } else {
        }
    case EXT_DATA_ELEMENT:
    default:
        break;
    };
    return res;
}

/**
 * Decode Temporal Noise Shaping filter coefficients and apply all-pole filters; reference: 4.6.9.3.
 *
 * @param   decode  1 if tool is used normally, 0 if tool is used in LTP.
 * @param   coef    spectral coefficients
 */
                      IndividualChannelStream *ics, int decode)
{



            // tns_decode_coef

            } else {
                inc = 1;
            }

                // ar filter
            } else {
                // ma filter
                }
            }
        }
    }
}

/**
 *  Apply windowing and MDCT to obtain the spectral
 *  coefficient from the predicted sample by LTP.
 */
                                   INTFLOAT *in, IndividualChannelStream *ics)
{

    } else {
    }
    } else {
        ac->fdsp->vector_fmul_reverse(in + 1024 + 448, in + 1024 + 448, swindow, 128);
        memset(in + 1024 + 576, 0, 448 * sizeof(*in));
    }

/**
 * Apply the long term prediction
 */
{


            num_samples = ltp->lag + 1024;



    }

/**
 * Update the LTP buffer for next frame
 */
{



    } else { // LONG_STOP or ONLY_LONG

    }


/**
 * Conduct IMDCT and windowing.
 */
{

    // imdct
    } else {
#if USE_FIXED
#endif /* USE_FIXED */
    }

    /* window overlapping
     * NOTE: To simplify the overlapping code, all 'meaningless' short to long
     * and long to short transitions are considered to be short to short
     * transitions. This leaves just two cases (long to long and short to short)
     * with a little special sauce for EIGHT_SHORT_SEQUENCE.
     */
    } else {

        } else {
        }
    }

    // buffer update
    } else { // LONG_STOP or ONLY_LONG
    }

/**
 * Conduct IMDCT and windowing.
 */
{
#if !USE_FIXED

    // imdct
    } else {
    }

    /* window overlapping
     * NOTE: To simplify the overlapping code, all 'meaningless' short to long
     * and long to short transitions are considered to be short to short
     * transitions. This leaves just two cases (long to long and short to short)
     * with a little special sauce for EIGHT_SHORT_SEQUENCE.
     */

    } else {

        } else {
        }
    }

    // buffer update
    } else { // LONG_STOP or ONLY_LONG
    }
#endif
{
#if USE_FIXED
#endif /* USE_FIXED */

    // imdct

#if USE_FIXED
#endif /* USE_FIXED */

    // window overlapping
        // AAC LD uses a low overlap sine window instead of a KBD window
    } else {
    }

    // buffer update

{
                                           AAC_RENAME(ff_aac_eld_window_512);

    // Inverse transform, mapped to the conventional IMDCT by
    // Chivukula, R.K.; Reznik, Y.A.; Devarajan, V.,
    // "Efficient algorithms for MPEG-4 AAC-ELD, AAC-LD and AAC-LC filterbanks,"
    // International Conference on Audio, Language and Image Processing, ICALIP 2008.
    // URL: http://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=4590245&isnumber=4589950
    }
#if !USE_FIXED
    else
#endif

#if USE_FIXED
#endif /* USE_FIXED */

    }
    // Like with the regular IMDCT at this point we still have the middle half
    // of a transform but with even symmetry on the left and odd symmetry on
    // the right

    // window overlapping
    // The spec says to use samples [0..511] but the reference decoder uses
    // samples [128..639].
    }
    }
    }

    // buffer update

/**
 * channel coupling transformation interface
 *
 * @param   apply_coupling_method   pointer to (in)dependent coupling function
 */
                                   enum RawDataBlockType type, int elem_id,
                                   enum CouplingPoint coupling_point,
                                   void (*apply_coupling_method)(AACContext *ac, SingleChannelElement *target, ChannelElement *cce, int index))
{



                    }
                } else
            }
        }
    }

/**
 * Convert spectral data to samples, applying all supported tools as appropriate.
 */
{
    case AOT_ER_AAC_LD:
        imdct_and_window = imdct_and_windowing_ld;
        break;
            imdct_and_window = imdct_and_windowing_960;
        else
    }
                    }
                }
                    }
                    }
                }

#if USE_FIXED
                {
                    int j;
                    /* preparation for resampler */
                    }
                }
#endif /* USE_FIXED */
                av_log(ac->avctx, AV_LOG_VERBOSE, "ChannelElement %d.%d missing \n", type, i);
            }
        }
    }

{

            // This is 2 for "VLB " audio in NSV files.
            // See samples/nsv/vlb_audio.
            avpriv_report_missing_feature(ac->avctx,
                                          "More than one AAC RDB per ADTS frame");
            ac->warned_num_aac_frames = 1;
        }
                                                  layout_map,
                                                  &layout_map_tags,
                                                  hdr_info.chan_config)) < 0)
                return ret;
                                              OC_TRIAL_FRAME), 0)) < 0)
                return ret;
        } else {
            ac->oc[1].m4ac.chan_config = 0;
            /**
             * dual mono frames in Japanese DTV can have chan_config 0
             * WITHOUT specifying PCE.
             *  thus, set dual mono as default.
             */
            if (ac->dmono_mode && ac->oc[0].status == OC_NONE) {
                layout_map_tags = 2;
                layout_map[0][0] = layout_map[1][0] = TYPE_SCE;
                layout_map[0][2] = layout_map[1][2] = AAC_CHANNEL_FRONT;
                layout_map[0][1] = 0;
                layout_map[1][1] = 1;
                if (output_configure(ac, layout_map, layout_map_tags,
                                     OC_TRIAL_FRAME, 0))
                    return -7;
            }
        }
        }
    }
    return size;
}

                               int *got_frame_ptr, GetBitContext *gb)
{



        return err;

    // The FF_PROFILE_AAC_* defines are all object_type - 1
    // This may lead to an undefined profile being signaled


        avpriv_request_sample(avctx, "Unknown ER channel configuration %d",
                              chan_config);
        return AVERROR_INVALIDDATA;
    }
            av_log(ac->avctx, AV_LOG_ERROR,
                   "channel element %d.%d is not allocated\n",
                   elem_type, elem_id);
            return AVERROR_INVALIDDATA;
        }
        }
            return err;
    }


        av_log(avctx, AV_LOG_ERROR, "no frame data found\n");
        return AVERROR_INVALIDDATA;
    }


}

static int aac_decode_frame_int(AVCodecContext *avctx, void *data,
                                int *got_frame_ptr, GetBitContext *gb, AVPacket *avpkt)
{
    AACContext *ac = avctx->priv_data;
    ChannelElement *che = NULL, *che_prev = NULL;
    enum RawDataBlockType elem_type, che_prev_type = TYPE_END;
    int err, elem_id;
    int samples = 0, multiplier, audio_found = 0, pce_found = 0;
    int is_dmono, sce_count = 0;
    int payload_alignment;
    uint8_t che_presence[4][MAX_ELEM_ID] = {{0}};

    ac->frame = data;

    if (show_bits(gb, 12) == 0xfff) {
        if ((err = parse_adts_frame_header(ac, gb)) < 0) {
            av_log(avctx, AV_LOG_ERROR, "Error decoding AAC frame header.\n");
            goto fail;
        }
        if (ac->oc[1].m4ac.sampling_index > 12) {
            av_log(ac->avctx, AV_LOG_ERROR, "invalid sampling rate index %d\n", ac->oc[1].m4ac.sampling_index);
            err = AVERROR_INVALIDDATA;
            goto fail;
        }
    }

    if ((err = frame_configure_elements(avctx)) < 0)
        goto fail;

    // The FF_PROFILE_AAC_* defines are all object_type - 1
    // This may lead to an undefined profile being signaled
    ac->avctx->profile = ac->oc[1].m4ac.object_type - 1;

    payload_alignment = get_bits_count(gb);
    ac->tags_mapped = 0;
    // parse
    while ((elem_type = get_bits(gb, 3)) != TYPE_END) {
        elem_id = get_bits(gb, 4);

        if (avctx->debug & FF_DEBUG_STARTCODE)
            av_log(avctx, AV_LOG_DEBUG, "Elem type:%x id:%x\n", elem_type, elem_id);

        if (!avctx->channels && elem_type != TYPE_PCE) {
            err = AVERROR_INVALIDDATA;
            goto fail;
        }

        if (elem_type < TYPE_DSE) {
            if (che_presence[elem_type][elem_id]) {
                int error = che_presence[elem_type][elem_id] > 1;
                av_log(ac->avctx, error ? AV_LOG_ERROR : AV_LOG_DEBUG, "channel element %d.%d duplicate\n",
                       elem_type, elem_id);
                if (error) {
                    err = AVERROR_INVALIDDATA;
                    goto fail;
                }
            }
            che_presence[elem_type][elem_id]++;

            if (!(che=get_che(ac, elem_type, elem_id))) {
                av_log(ac->avctx, AV_LOG_ERROR, "channel element %d.%d is not allocated\n",
                       elem_type, elem_id);
                err = AVERROR_INVALIDDATA;
                goto fail;
            }
            samples = ac->oc[1].m4ac.frame_length_short ? 960 : 1024;
            che->present = 1;
        }

        switch (elem_type) {

        case TYPE_SCE:
            err = decode_ics(ac, &che->ch[0], gb, 0, 0);
            audio_found = 1;
            sce_count++;
            break;

        case TYPE_CPE:
            err = decode_cpe(ac, gb, che);
            audio_found = 1;
            break;

        case TYPE_CCE:
            err = decode_cce(ac, gb, che);
            break;

        case TYPE_LFE:
            err = decode_ics(ac, &che->ch[0], gb, 0, 0);
            audio_found = 1;
            break;

        case TYPE_DSE:
            err = skip_data_stream_element(ac, gb);
            break;

        case TYPE_PCE: {
            uint8_t layout_map[MAX_ELEM_ID*4][3];
            int tags;

            int pushed = push_output_configuration(ac);
            if (pce_found && !pushed) {
                err = AVERROR_INVALIDDATA;
                goto fail;
            }

            tags = decode_pce(avctx, &ac->oc[1].m4ac, layout_map, gb,
                              payload_alignment);
            if (tags < 0) {
                err = tags;
                break;
            }
            if (pce_found) {
                av_log(avctx, AV_LOG_ERROR,
                       "Not evaluating a further program_config_element as this construct is dubious at best.\n");
                pop_output_configuration(ac);
            } else {
                err = output_configure(ac, layout_map, tags, OC_TRIAL_PCE, 1);
                if (!err)
                    ac->oc[1].m4ac.chan_config = 0;
                pce_found = 1;
            }
            break;
        }

        case TYPE_FIL:
            if (elem_id == 15)
                elem_id += get_bits(gb, 8) - 1;
            if (get_bits_left(gb) < 8 * elem_id) {
                    av_log(avctx, AV_LOG_ERROR, "TYPE_FIL: "overread_err);
                    err = AVERROR_INVALIDDATA;
                    goto fail;
            }
            err = 0;
            while (elem_id > 0) {
                int ret = decode_extension_payload(ac, gb, elem_id, che_prev, che_prev_type);
                if (ret < 0) {
                    err = ret;
                    break;
                }
                elem_id -= ret;
            }
            break;

        default:
            err = AVERROR_BUG; /* should not happen, but keeps compiler happy */
            break;
        }

        if (elem_type < TYPE_DSE) {
            che_prev      = che;
            che_prev_type = elem_type;
        }

        if (err)
            goto fail;

        if (get_bits_left(gb) < 3) {
            av_log(avctx, AV_LOG_ERROR, overread_err);
            err = AVERROR_INVALIDDATA;
            goto fail;
        }
    }

    if (!avctx->channels) {
        *got_frame_ptr = 0;
        return 0;
    }

    multiplier = (ac->oc[1].m4ac.sbr == 1) ? ac->oc[1].m4ac.ext_sample_rate > ac->oc[1].m4ac.sample_rate : 0;
    samples <<= multiplier;

    spectral_to_sample(ac, samples);

    if (ac->oc[1].status && audio_found) {
        avctx->sample_rate = ac->oc[1].m4ac.sample_rate << multiplier;
        avctx->frame_size = samples;
        ac->oc[1].status = OC_LOCKED;
    }

    if (multiplier)
        avctx->internal->skip_samples_multiplier = 2;

    if (!ac->frame->data[0] && samples) {
        av_log(avctx, AV_LOG_ERROR, "no frame data found\n");
        err = AVERROR_INVALIDDATA;
        goto fail;
    }

    if (samples) {
        ac->frame->nb_samples = samples;
        ac->frame->sample_rate = avctx->sample_rate;
    } else
        av_frame_unref(ac->frame);
    *got_frame_ptr = !!samples;

    /* for dual-mono audio (SCE + SCE) */
    is_dmono = ac->dmono_mode && sce_count == 2 &&
               ac->oc[1].channel_layout == (AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT);
    if (is_dmono) {
        if (ac->dmono_mode == 1)
            ((AVFrame *)data)->data[1] =((AVFrame *)data)->data[0];
        else if (ac->dmono_mode == 2)
            ((AVFrame *)data)->data[0] =((AVFrame *)data)->data[1];
    }

    return 0;
fail:
    pop_output_configuration(ac);
    return err;
}

                            int *got_frame_ptr, AVPacket *avpkt)
{
                                       AV_PKT_DATA_NEW_EXTRADATA,
                                       &new_extradata_size);
                                       AV_PKT_DATA_JP_DUALMONO,
                                       &jp_dualmono_size);

        /* discard previous configuration */
                                           new_extradata,
            return err;
        }
    }

        ac->dmono_mode =  1 + *jp_dualmono;

        return AVERROR_INVALIDDATA;

        return err;

    case AOT_ER_AAC_LTP:
    case AOT_ER_AAC_LD:
    case AOT_ER_AAC_ELD:
    }
        return err;

        if (buf[buf_offset])
            break;

}

{

        }
    }

#if !USE_FIXED
#endif
}

{
#if USE_FIXED
#endif

#if !USE_FIXED
        ff_aacdec_init_mips(c);
#endif /* !USE_FIXED */
}
/**
 * AVOptions for Japanese DTV specific extensions (ADTS only)
 */
#define AACDEC_FLAGS AV_OPT_FLAG_DECODING_PARAM | AV_OPT_FLAG_AUDIO_PARAM
static const AVOption options[] = {
    {"dual_mono_mode", "Select the channel to decode for dual mono",
     offsetof(AACContext, force_dmono_mode), AV_OPT_TYPE_INT, {.i64=-1}, -1, 2,
     AACDEC_FLAGS, "dual_mono_mode"},

    {"auto", "autoselection",            0, AV_OPT_TYPE_CONST, {.i64=-1}, INT_MIN, INT_MAX, AACDEC_FLAGS, "dual_mono_mode"},
    {"main", "Select Main/Left channel", 0, AV_OPT_TYPE_CONST, {.i64= 1}, INT_MIN, INT_MAX, AACDEC_FLAGS, "dual_mono_mode"},
    {"sub" , "Select Sub/Right channel", 0, AV_OPT_TYPE_CONST, {.i64= 2}, INT_MIN, INT_MAX, AACDEC_FLAGS, "dual_mono_mode"},
    {"both", "Select both channels",     0, AV_OPT_TYPE_CONST, {.i64= 0}, INT_MIN, INT_MAX, AACDEC_FLAGS, "dual_mono_mode"},

    {NULL},
};

static const AVClass aac_decoder_class = {
    .class_name = "AAC decoder",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
};
