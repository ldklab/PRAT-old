/*
 * MQ-coder decoder
 * Copyright (c) 2007 Kamil Nowosad
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
 * MQ-coder decoder
 * @file
 * @author Kamil Nowosad
 */

#include "mqc.h"

static void bytein(MqcState *mqc)
{
    if (*mqc->bp == 0xff) {
        if (*(mqc->bp + 1) > 0x8f)
            mqc->c++;
        else {
            mqc->bp++;
            mqc->c += 2 + 0xfe00 - (*mqc->bp << 9);
        }
    } else {
        mqc->bp++;
        mqc->c += 1 + 0xff00 - (*mqc->bp << 8);
    }
}

{
    } else {
    }
    // do RENORMD: see ISO/IEC 15444-1:2002 §C.3.3
        }
}

{

static int mqc_decode_bypass(MqcState *mqc) {
    int bit = !(mqc->c & 0x40000000);
    if (!(mqc->c & 0xff)) {
        mqc->c -= 0x100;
        bytein(mqc);
    }
    mqc->c += mqc->c;
    return bit;
}

{
        return mqc_decode_bypass(mqc);
        else
    } else {
    }
}
