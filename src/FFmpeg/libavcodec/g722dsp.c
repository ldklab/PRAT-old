/*
 * Copyright (c) 2015 Peter Meerwald <pmeerw@pmeerw.net>
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

#include "g722dsp.h"
#include "mathops.h"

/*
 * quadrature mirror filter (QMF) coefficients (ITU-T G.722 Table 11) inlined
 * in code below: 3, -11, 12, 32, -210, 951, 3876, -805, 362, -156, 53, -11
 */

{












{

        ff_g722dsp_init_arm(c);
