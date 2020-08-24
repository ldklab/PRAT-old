/*
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

#include "libavutil/attributes.h"
#include "libavutil/mem.h"
#include "avfft.h"
#include "fft.h"
#include "rdft.h"
#include "dct.h"

/* FFT */

{

        av_freep(&s);

}

{

{

{
    }

#if CONFIG_MDCT

{

        av_freep(&s);

}

{

void av_imdct_half(FFTContext *s, FFTSample *output, const FFTSample *input)
{
    s->imdct_half(s, output, input);
}

{

{
    }

#endif /* CONFIG_MDCT */

#if CONFIG_RDFT

{

        av_freep(&s);

}

{

{
    }

#endif /* CONFIG_RDFT */

#if CONFIG_DCT

{

        av_freep(&s);

}

{

{
    }

#endif /* CONFIG_DCT */
