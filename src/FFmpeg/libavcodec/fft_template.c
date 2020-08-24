/*
 * FFT/IFFT transforms
 * Copyright (c) 2008 Loren Merritt
 * Copyright (c) 2002 Fabrice Bellard
 * Partly based on libdjbfft by D. J. Bernstein
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
 * FFT/IFFT transforms.
 */

#include <stdlib.h>
#include <string.h>
#include "libavutil/mathematics.h"
#include "libavutil/thread.h"
#include "fft.h"
#include "fft-internal.h"

#if FFT_FIXED_32
#include "fft_table.h"

{

#else /* FFT_FIXED_32 */

/* cos(2*pi*x/n) for 0<=x<=n/4, followed by its reverse */
#if !CONFIG_HARDCODED_TABLES
COSTABLE(16);
COSTABLE(32);
COSTABLE(64);
COSTABLE(128);
COSTABLE(256);
COSTABLE(512);
COSTABLE(1024);
COSTABLE(2048);
COSTABLE(4096);
COSTABLE(8192);
COSTABLE(16384);
COSTABLE(32768);
COSTABLE(65536);
COSTABLE(131072);

{

typedef struct CosTabsInitOnce {
    void (*func)(void);
    AVOnce control;
} CosTabsInitOnce;

#define INIT_FF_COS_TABS_FUNC(index, size)          \
static av_cold void init_ff_cos_tabs_ ## size (void)\
{                                                   \
    init_ff_cos_tabs(index);                        \
}

INIT_FF_COS_TABS_FUNC(15, 32768)
INIT_FF_COS_TABS_FUNC(16, 65536)
INIT_FF_COS_TABS_FUNC(17, 131072)

static CosTabsInitOnce cos_tabs_init_once[] = {
    { NULL },
    { NULL },
    { NULL },
    { NULL },
    { init_ff_cos_tabs_16, AV_ONCE_INIT },
    { init_ff_cos_tabs_32, AV_ONCE_INIT },
    { init_ff_cos_tabs_64, AV_ONCE_INIT },
    { init_ff_cos_tabs_128, AV_ONCE_INIT },
    { init_ff_cos_tabs_256, AV_ONCE_INIT },
    { init_ff_cos_tabs_512, AV_ONCE_INIT },
    { init_ff_cos_tabs_1024, AV_ONCE_INIT },
    { init_ff_cos_tabs_2048, AV_ONCE_INIT },
    { init_ff_cos_tabs_4096, AV_ONCE_INIT },
    { init_ff_cos_tabs_8192, AV_ONCE_INIT },
    { init_ff_cos_tabs_16384, AV_ONCE_INIT },
    { init_ff_cos_tabs_32768, AV_ONCE_INIT },
    { init_ff_cos_tabs_65536, AV_ONCE_INIT },
    { init_ff_cos_tabs_131072, AV_ONCE_INIT },
};

#endif
COSTABLE_CONST FFTSample * const FFT_NAME(ff_cos_tabs)[] = {
    NULL, NULL, NULL, NULL,
    FFT_NAME(ff_cos_16),
    FFT_NAME(ff_cos_32),
    FFT_NAME(ff_cos_64),
    FFT_NAME(ff_cos_128),
    FFT_NAME(ff_cos_256),
    FFT_NAME(ff_cos_512),
    FFT_NAME(ff_cos_1024),
    FFT_NAME(ff_cos_2048),
    FFT_NAME(ff_cos_4096),
    FFT_NAME(ff_cos_8192),
    FFT_NAME(ff_cos_16384),
    FFT_NAME(ff_cos_32768),
    FFT_NAME(ff_cos_65536),
    FFT_NAME(ff_cos_131072),
};

#endif /* FFT_FIXED_32 */

static void fft_permute_c(FFTContext *s, FFTComplex *z);
static void fft_calc_c(FFTContext *s, FFTComplex *z);

{
}

{
#if (!CONFIG_HARDCODED_TABLES) && (!FFT_FIXED_32)
#endif

static const int avx_tab[] = {
    0, 4, 1, 5, 8, 12, 9, 13, 2, 6, 3, 7, 10, 14, 11, 15
};

static int is_second_half_of_fft32(int i, int n)
{
        return is_second_half_of_fft32(i, n/2);
    else
}

{

        int k;

        } else {
            }
        }
    }

{


        goto fail;

            goto fail;
    } else {
        s->revtab32 = av_malloc(n * sizeof(uint32_t));
        if (!s->revtab32)
            goto fail;
    }
        goto fail;

#if CONFIG_MDCT
#endif

#if FFT_FIXED_32
    {
    }
#else /* FFT_FIXED_32 */
#if FFT_FLOAT
#else
#endif
    }
#endif /* FFT_FIXED_32 */


    } else {
#define PROCESS_FFT_PERM_SWAP_LSBS(num) do {\
    for(i = 0; i < n; i++) {\
        int k;\
        j = i;\
        j = (j & ~3) | ((j >> 1) & 1) | ((j << 1) & 2);\
        k = -split_radix_permutation(i, n, s->inverse) & (n - 1);\
        s->revtab##num[k] = j;\
    } \
} while(0);

#define PROCESS_FFT_PERM_DEFAULT(num) do {\
    for(i = 0; i < n; i++) {\
        int k;\
        j = i;\
        k = -split_radix_permutation(i, n, s->inverse) & (n - 1);\
        s->revtab##num[k] = j;\
    } \
} while(0);

#define SPLIT_RADIX_PERMUTATION(num) do { \
    if (s->fft_permutation == FF_FFT_PERM_SWAP_LSBS) {\
        PROCESS_FFT_PERM_SWAP_LSBS(num) \
    } else {\
        PROCESS_FFT_PERM_DEFAULT(num) \
    }\
} while(0);

        SPLIT_RADIX_PERMUTATION(32)

#undef PROCESS_FFT_PERM_DEFAULT
#undef PROCESS_FFT_PERM_SWAP_LSBS
#undef SPLIT_RADIX_PERMUTATION
    }

    return 0;
 fail:
    av_freep(&s->revtab);
    av_freep(&s->revtab32);
    av_freep(&s->tmp_buf);
    return -1;
}

{
    /* TODO: handle split-radix permute in a more optimal way, probably in-place */
    } else
        for(j=0;j<np;j++) s->tmp_buf[revtab32[j]] = z[j];


{

#if FFT_FIXED_32






    }

        return;







    }

    step = 1 << ((MAX_LOG2_NFFT-4) - 4);
    n4 = 4;








            }
        }
    }
}

#else /* FFT_FIXED_32 */

#define BUTTERFLIES(a0,a1,a2,a3) {\
    BF(t3, t5, t5, t1);\
    BF(a2.re, a0.re, a0.re, t5);\
    BF(a3.im, a1.im, a1.im, t3);\
    BF(t4, t6, t2, t6);\
    BF(a3.re, a1.re, a1.re, t4);\
    BF(a2.im, a0.im, a0.im, t6);\
}

// force loading all the inputs before storing any.
// this is slightly slower for small data, but avoids store->load aliasing
// for addresses separated by large powers of 2.
#define BUTTERFLIES_BIG(a0,a1,a2,a3) {\
    FFTSample r0=a0.re, i0=a0.im, r1=a1.re, i1=a1.im;\
    BF(t3, t5, t5, t1);\
    BF(a2.re, a0.re, r0, t5);\
    BF(a3.im, a1.im, i1, t3);\
    BF(t4, t6, t2, t6);\
    BF(a3.re, a1.re, r1, t4);\
    BF(a2.im, a0.im, i0, t6);\
}

#define TRANSFORM(a0,a1,a2,a3,wre,wim) {\
    CMUL(t1, t2, a2.re, a2.im, wre, -wim);\
    CMUL(t5, t6, a3.re, a3.im, wre,  wim);\
    BUTTERFLIES(a0,a1,a2,a3)\
}

#define TRANSFORM_ZERO(a0,a1,a2,a3) {\
    t1 = a2.re;\
    t2 = a2.im;\
    t5 = a3.re;\
    t6 = a3.im;\
    BUTTERFLIES(a0,a1,a2,a3)\
}

/* z[0...8n-1], w[1...2n-1] */
#define PASS(name)\
static void name(FFTComplex *z, const FFTSample *wre, unsigned int n)\
{\
    FFTDouble t1, t2, t3, t4, t5, t6;\
    int o1 = 2*n;\
    int o2 = 4*n;\
    int o3 = 6*n;\
    const FFTSample *wim = wre+o1;\
    n--;\
\
    TRANSFORM_ZERO(z[0],z[o1],z[o2],z[o3]);\
    TRANSFORM(z[1],z[o1+1],z[o2+1],z[o3+1],wre[1],wim[-1]);\
    do {\
        z += 2;\
        wre += 2;\
        wim -= 2;\
        TRANSFORM(z[0],z[o1],z[o2],z[o3],wre[0],wim[0]);\
        TRANSFORM(z[1],z[o1+1],z[o2+1],z[o3+1],wre[1],wim[-1]);\
    } while(--n);\
}

#if !CONFIG_SMALL
#undef BUTTERFLIES
#define BUTTERFLIES BUTTERFLIES_BIG
#endif

#define DECL_FFT(n,n2,n4)\
static void fft##n(FFTComplex *z)\
{\
    fft##n2(z);\
    fft##n4(z+n4*2);\
    fft##n4(z+n4*3);\
    pass(z,FFT_NAME(ff_cos_##n),n4/2);\
}

{


{




#if !CONFIG_SMALL
{


#else
DECL_FFT(16,8,4)
#endif
#if !CONFIG_SMALL
#define pass pass_big
#endif
DECL_FFT(8192,4096,2048)
DECL_FFT(16384,8192,4096)
DECL_FFT(32768,16384,8192)
DECL_FFT(65536,32768,16384)
DECL_FFT(131072,65536,32768)

static void (* const fft_dispatch[])(FFTComplex*) = {
    fft4, fft8, fft16, fft32, fft64, fft128, fft256, fft512, fft1024,
    fft2048, fft4096, fft8192, fft16384, fft32768, fft65536, fft131072
};

{
#endif /* FFT_FIXED_32 */
