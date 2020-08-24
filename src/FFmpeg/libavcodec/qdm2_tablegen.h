/*
 * Header file for hardcoded QDM2 tables
 *
 * Copyright (c) 2010 Reimar Döffinger <Reimar.Doeffinger@gmx.de>
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

#ifndef AVCODEC_QDM2_TABLEGEN_H
#define AVCODEC_QDM2_TABLEGEN_H

#include <stdint.h>
#include <math.h>
#include "libavutil/attributes.h"
#include "qdm2data.h"

#define SOFTCLIP_THRESHOLD 27600
#define HARDCLIP_THRESHOLD 35716

#if CONFIG_HARDCODED_TABLES
#define softclip_table_init()
#define rnd_table_init()
#define init_noise_samples()
#define qdm2_init_vlc()
#include "libavcodec/qdm2_tables.h"
#else
static uint16_t softclip_table[HARDCLIP_THRESHOLD - SOFTCLIP_THRESHOLD + 1];
static float noise_table[4096 + 20];
static uint8_t random_dequant_index[256][5];
static uint8_t random_dequant_type24[128][3];
static float noise_samples[128];



// random generated table
    }

        }
    }
        }
    }


    }

static VLC vlc_tab_level;
static VLC vlc_tab_diff;
static VLC vlc_tab_run;
static VLC fft_level_exp_alt_vlc;
static VLC fft_level_exp_vlc;
static VLC fft_stereo_exp_vlc;
static VLC fft_stereo_phase_vlc;
static VLC vlc_tab_tone_level_idx_hi1;
static VLC vlc_tab_tone_level_idx_mid;
static VLC vlc_tab_tone_level_idx_hi2;
static VLC vlc_tab_type30;
static VLC vlc_tab_type34;
static VLC vlc_tab_fft_tone_offset[5];

static const uint16_t qdm2_vlc_offs[] = {
    0,260,566,598,894,1166,1230,1294,1678,1950,2214,2278,2310,2570,2834,3124,3448,3838,
};

static VLC_TYPE qdm2_table[3838][2];

{
             vlc_tab_level_huffbits, 1, 1,
             vlc_tab_level_huffcodes, 2, 2,
             INIT_VLC_USE_NEW_STATIC | INIT_VLC_LE);

             vlc_tab_diff_huffbits, 1, 1,
             vlc_tab_diff_huffcodes, 2, 2,
             INIT_VLC_USE_NEW_STATIC | INIT_VLC_LE);

             vlc_tab_run_huffbits, 1, 1,
             vlc_tab_run_huffcodes, 1, 1,
             INIT_VLC_USE_NEW_STATIC | INIT_VLC_LE);

                                            qdm2_vlc_offs[3];
             fft_level_exp_alt_huffbits, 1, 1,
             fft_level_exp_alt_huffcodes, 2, 2,
             INIT_VLC_USE_NEW_STATIC | INIT_VLC_LE);

             fft_level_exp_huffbits, 1, 1,
             fft_level_exp_huffcodes, 2, 2,
             INIT_VLC_USE_NEW_STATIC | INIT_VLC_LE);

                                         qdm2_vlc_offs[5];
             fft_stereo_exp_huffbits, 1, 1,
             fft_stereo_exp_huffcodes, 1, 1,
             INIT_VLC_USE_NEW_STATIC | INIT_VLC_LE);

                                           qdm2_vlc_offs[6];
             fft_stereo_phase_huffbits, 1, 1,
             fft_stereo_phase_huffcodes, 1, 1,
             INIT_VLC_USE_NEW_STATIC | INIT_VLC_LE);

        &qdm2_table[qdm2_vlc_offs[7]];
                                                 qdm2_vlc_offs[7];
             vlc_tab_tone_level_idx_hi1_huffbits, 1, 1,
             vlc_tab_tone_level_idx_hi1_huffcodes, 2, 2,
             INIT_VLC_USE_NEW_STATIC | INIT_VLC_LE);

        &qdm2_table[qdm2_vlc_offs[8]];
                                                 qdm2_vlc_offs[8];
             vlc_tab_tone_level_idx_mid_huffbits, 1, 1,
             vlc_tab_tone_level_idx_mid_huffcodes, 2, 2,
             INIT_VLC_USE_NEW_STATIC | INIT_VLC_LE);

        &qdm2_table[qdm2_vlc_offs[9]];
                                                 qdm2_vlc_offs[9];
             vlc_tab_tone_level_idx_hi2_huffbits, 1, 1,
             vlc_tab_tone_level_idx_hi2_huffcodes, 2, 2,
             INIT_VLC_USE_NEW_STATIC | INIT_VLC_LE);

             vlc_tab_type30_huffbits, 1, 1,
             vlc_tab_type30_huffcodes, 1, 1,
             INIT_VLC_USE_NEW_STATIC | INIT_VLC_LE);

             vlc_tab_type34_huffbits, 1, 1,
             vlc_tab_type34_huffcodes, 1, 1,
             INIT_VLC_USE_NEW_STATIC | INIT_VLC_LE);

        &qdm2_table[qdm2_vlc_offs[12]];
                                                 qdm2_vlc_offs[12];
             vlc_tab_fft_tone_offset_0_huffbits, 1, 1,
             vlc_tab_fft_tone_offset_0_huffcodes, 2, 2,
             INIT_VLC_USE_NEW_STATIC | INIT_VLC_LE);

        &qdm2_table[qdm2_vlc_offs[13]];
                                                 qdm2_vlc_offs[13];
             vlc_tab_fft_tone_offset_1_huffbits, 1, 1,
             vlc_tab_fft_tone_offset_1_huffcodes, 2, 2,
             INIT_VLC_USE_NEW_STATIC | INIT_VLC_LE);

        &qdm2_table[qdm2_vlc_offs[14]];
                                                 qdm2_vlc_offs[14];
             vlc_tab_fft_tone_offset_2_huffbits, 1, 1,
             vlc_tab_fft_tone_offset_2_huffcodes, 2, 2,
             INIT_VLC_USE_NEW_STATIC | INIT_VLC_LE);

        &qdm2_table[qdm2_vlc_offs[15]];
                                                 qdm2_vlc_offs[15];
             vlc_tab_fft_tone_offset_3_huffbits, 1, 1,
             vlc_tab_fft_tone_offset_3_huffcodes, 2, 2,
             INIT_VLC_USE_NEW_STATIC | INIT_VLC_LE);

        &qdm2_table[qdm2_vlc_offs[16]];
                                                 qdm2_vlc_offs[16];
             vlc_tab_fft_tone_offset_4_huffbits, 1, 1,
             vlc_tab_fft_tone_offset_4_huffcodes, 2, 2,
             INIT_VLC_USE_NEW_STATIC | INIT_VLC_LE);

#endif /* CONFIG_HARDCODED_TABLES */

#endif /* AVCODEC_QDM2_TABLEGEN_H */
