/*
 * Audio and Video frame extraction
 * Copyright (c) 2003 Fabrice Bellard
 * Copyright (c) 2003 Michael Niedermayer
 * Copyright (c) 2009 Alex Converse
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

#include "aac_ac3_parser.h"
#include "adts_header.h"
#include "adts_parser.h"
#include "get_bits.h"
#include "mpeg4audio.h"

{

        return AAC_AC3_PARSE_ERROR_SYNC;

        return AAC_AC3_PARSE_ERROR_SAMPLE_RATE;


    /* adts_variable_header */
        return AAC_AC3_PARSE_ERROR_FRAME_SIZE;



}
