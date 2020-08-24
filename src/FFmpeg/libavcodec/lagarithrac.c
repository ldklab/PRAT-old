/*
 * Lagarith range decoder
 * Copyright (c) 2009 Nathan Caldwell <saintdev (at) gmail.com>
 * Copyright (c) 2009 David Conrad
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
 * Lagarith range decoder
 * @author Nathan Caldwell
 * @author David Conrad
 */

#include "get_bits.h"
#include "lagarithrac.h"

{

    /* According to reference decoder "1st byte is garbage",
     * however, it gets skipped by the call to align_get_bits()
     */


            j++;
    }
