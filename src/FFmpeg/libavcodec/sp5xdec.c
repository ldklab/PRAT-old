/*
 * Sunplus JPEG decoder (SP5X)
 * Copyright (c) 2003 Alex Beregszaszi
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
 * Sunplus JPEG decoder (SP5X).
 */

#include "avcodec.h"
#include "internal.h"
#include "mjpeg.h"
#include "mjpegdec.h"
#include "sp5x.h"


                              void *data, int *got_frame,
                              AVPacket *avpkt)
{

        return -1;

        return -1;

    /* SOI */





    else
    {
    }

    /* EOI */



}

#if CONFIG_SP5X_DECODER
AVCodec ff_sp5x_decoder = {
    .name           = "sp5x",
    .long_name      = NULL_IF_CONFIG_SMALL("Sunplus JPEG (SP5X)"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_SP5X,
    .priv_data_size = sizeof(MJpegDecodeContext),
    .init           = ff_mjpeg_decode_init,
    .close          = ff_mjpeg_decode_end,
    .decode         = sp5x_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
    .max_lowres     = 3,
    .caps_internal  = FF_CODEC_CAP_INIT_THREADSAFE,
};
#endif
#if CONFIG_AMV_DECODER
AVCodec ff_amv_decoder = {
    .name           = "amv",
    .long_name      = NULL_IF_CONFIG_SMALL("AMV Video"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_AMV,
    .priv_data_size = sizeof(MJpegDecodeContext),
    .init           = ff_mjpeg_decode_init,
    .close          = ff_mjpeg_decode_end,
    .decode         = sp5x_decode_frame,
    .max_lowres     = 3,
    .capabilities   = AV_CODEC_CAP_DR1,
    .caps_internal  = FF_CODEC_CAP_INIT_THREADSAFE,
};
#endif
