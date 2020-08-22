/*
 * AAC Spectral Band Replication decoding functions
 * Copyright (c) 2008-2009 Robert Swain ( rob opendot cl )
 * Copyright (c) 2009-2010 Alex Converse <alex.converse@gmail.com>
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

{
    }

{
#if USE_FIXED
#else
#endif
    }

                                 const AAC_FLOAT *q_filt, int noise,
                                 int kx, int m_max)
{

                                 const AAC_FLOAT *q_filt, int noise,
                                 int kx, int m_max)
{

                                 const AAC_FLOAT *q_filt, int noise,
                                 int kx, int m_max)
{

                                 const AAC_FLOAT *q_filt, int noise,
                                 int kx, int m_max)
{

{


#if !USE_FIXED
        ff_sbrdsp_init_arm(s);
        ff_sbrdsp_init_aarch64(s);
        ff_sbrdsp_init_mips(s);
#endif /* !USE_FIXED */
