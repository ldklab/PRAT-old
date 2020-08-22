/*
 * Header file for hardcoded Parametric Stereo tables
 *
 * Copyright (c) 2010 Alex Converse <alex.converse@gmail.com>
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

#ifndef AVCODEC_AACPS_TABLEGEN_H
#define AVCODEC_AACPS_TABLEGEN_H

#include <math.h>
#include <stdint.h>

#if CONFIG_HARDCODED_TABLES
#define ps_tableinit()
#define TABLE_CONST const
#include "libavcodec/aacps_tables.h"
#else
#include "libavutil/common.h"
#include "libavutil/libm.h"
#include "libavutil/mathematics.h"
#include "libavutil/mem.h"
#define NR_ALLPASS_BANDS20 30
#define NR_ALLPASS_BANDS34 50
#define PS_AP_LINKS 3
#define TABLE_CONST
static float pd_re_smooth[8*8*8];
static float pd_im_smooth[8*8*8];
static float HA[46][8][4];
static float HB[46][8][4];
static DECLARE_ALIGNED(16, float, f20_0_8) [ 8][8][2];
static DECLARE_ALIGNED(16, float, f34_0_12)[12][8][2];
static DECLARE_ALIGNED(16, float, f34_1_8) [ 8][8][2];
static DECLARE_ALIGNED(16, float, f34_2_4) [ 4][8][2];
static TABLE_CONST DECLARE_ALIGNED(16, float, Q_fract_allpass)[2][50][3][2];
static DECLARE_ALIGNED(16, float, phi_fract)[2][50][2];

static const float g0_Q8[] = {
    0.00746082949812f, 0.02270420949825f, 0.04546865930473f, 0.07266113929591f,
    0.09885108575264f, 0.11793710567217f, 0.125f
};

static const float g0_Q12[] = {
    0.04081179924692f, 0.03812810994926f, 0.05144908135699f, 0.06399831151592f,
    0.07428313801106f, 0.08100347892914f, 0.08333333333333f
};

static const float g1_Q8[] = {
    0.01565675600122f, 0.03752716391991f, 0.05417891378782f, 0.08417044116767f,
    0.10307344158036f, 0.12222452249753f, 0.125f
};

static const float g2_Q4[] = {
    -0.05908211155639f, -0.04871498374946f, 0.0f,   0.07778723915851f,
     0.16486303567403f,  0.23279856662996f, 0.25f
};

{
        }
    }

{

        //iid_par_dequant_default
        0.05623413251903, 0.12589254117942, 0.19952623149689, 0.31622776601684,
        0.44668359215096, 0.63095734448019, 0.79432823472428, 1,
        1.25892541179417, 1.58489319246111, 2.23872113856834, 3.16227766016838,
        5.01187233627272, 7.94328234724282, 17.7827941003892,
        //iid_par_dequant_fine
        0.00316227766017, 0.00562341325190, 0.01,             0.01778279410039,
        0.03162277660168, 0.05623413251903, 0.07943282347243, 0.11220184543020,
        0.15848931924611, 0.22387211385683, 0.31622776601684, 0.39810717055350,
        0.50118723362727, 0.63095734448019, 0.79432823472428, 1,
        1.25892541179417, 1.58489319246111, 1.99526231496888, 2.51188643150958,
        3.16227766016838, 4.46683592150963, 6.30957344480193, 8.91250938133745,
        12.5892541179417, 17.7827941003892, 31.6227766016838, 56.2341325190349,
        100,              177.827941003892, 316.227766016837,
    };
        1, 0.937,      0.84118,    0.60092,    0.36764,   0,      -0.589,    -1
    };
        0, 0.35685527, 0.57133466, 0.92614472, 1.1943263, M_PI/2, 2.2006171, M_PI
    };

        -3, -1, 1, 3, 5, 7, 10, 14, 18, 22,
    };
         2,  6, 10, 14, 18, 22, 26, 30,
        34,-10, -6, -2, 51, 57, 15, 21,
        27, 33, 39, 45, 54, 66, 78, 42,
       102, 66, 78, 90,102,114,126, 90,
    };

            }
        }
    }

            /*if (PS_BASELINE || ps->icc_mode < 3)*/ {
            } /* else */ {
            }
        }
    }

        else
        }
    }
        else
        }
    }

#endif /* CONFIG_HARDCODED_TABLES */

#endif /* AVCODEC_AACPS_TABLEGEN_H */
