/*
 * H.26L/H.264/AVC/JVT/14496-10/... direct mb/block decoding
 * Copyright (c) 2003 Michael Niedermayer <michaelni@gmx.at>
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
 * H.264 / AVC / MPEG-4 part10 direct mb/block decoding.
 * @author Michael Niedermayer <michaelni@gmx.at>
 */

#include "internal.h"
#include "avcodec.h"
#include "h264dec.h"
#include "h264_ps.h"
#include "mpegutils.h"
#include "rectangle.h"
#include "thread.h"

#include <assert.h>

                            int poc, int poc1, int i)
{

        avpriv_request_sample(sl->h264->avctx, "pocdiff overflow\n");

        return 256;
    } else {

            av_log(sl->h264->avctx, AV_LOG_DEBUG, "pocdiff0 overflow\n");

    }
}

                                      H264SliceContext *sl)
{

        }


static void fill_colmap(const H264Context *h, H264SliceContext *sl,
                        int map[2][16 + 32], int list,
                        int field, int colfield, int mbafi)
{
    H264Picture *const ref1 = sl->ref_list[1][0].parent;
    int j, old_ref, rfield;
    int start  = mbafi ? 16                       : 0;
    int end    = mbafi ? 16 + 2 * sl->ref_count[0] : sl->ref_count[0];
    int interl = mbafi || h->picture_structure != PICT_FRAME;

    /* bogus; fills in for missing frames */
    memset(map[list], 0, sizeof(map[list]));

    for (rfield = 0; rfield < 2; rfield++) {
        for (old_ref = 0; old_ref < ref1->ref_count[colfield][list]; old_ref++) {
            int poc = ref1->ref_poc[colfield][list][old_ref];

            if (!interl)
                poc |= 3;
            // FIXME: store all MBAFF references so this is not needed
            else if (interl && (poc & 3) == 3)
                poc = (poc & ~3) + rfield + 1;

            for (j = start; j < end; j++) {
                if (4 * sl->ref_list[0][j].parent->frame_num +
                    (sl->ref_list[0][j].reference & 3) == poc) {
                    int cur_ref = mbafi ? (j - 16) ^ field : j;
                    if (ref1->mbaff)
                        map[list][2 * old_ref + (rfield ^ field) + 16] = cur_ref;
                    if (rfield == field || !interl)
                        map[list][old_ref] = cur_ref;
                    break;
                }
            }
        }
    }
}

{

    }

    }

    } else {
    }


        return;

        } else
    // FL -> FL & differ parity
    }

        return;

                            field, 1);
    }
}

static void await_reference_mb_row(const H264Context *const h, H264Ref *ref,
                                   int mb_y)
{
    int ref_field         = ref->reference - 1;
    int ref_field_picture = ref->parent->field_picture;
    int ref_height        = 16 * h->mb_height >> ref_field_picture;

    if (!HAVE_THREADS || !(h->avctx->active_thread_type & FF_THREAD_FRAME))
        return;

    /* FIXME: It can be safe to access mb stuff
     * even if pixels aren't deblocked yet. */

    ff_thread_await_progress(&ref->parent->tf,
                             FFMIN(16 * mb_y >> ref_field_picture,
                                   ref_height - 1),
                             ref_field_picture && ref_field);
}

                                       int *mb_type)
{



#define MB_TYPE_16x16_OR_INTRA (MB_TYPE_16x16 | MB_TYPE_INTRA4x4 | \
                                MB_TYPE_INTRA16x16 | MB_TYPE_INTRA_PCM)

    /* ref = min(neighbors) */
        }
                           (unsigned)top_ref,
                           (unsigned)refc);
            /* This is just pred_motion() but with the cases removed that
             * cannot happen for direct blocks. */


            } else {
                else
            }
            av_assert2(ref[list] < (sl->ref_count[list] << !!FRAME_MBAFF(h)));
        } else {
        }
    }
        sub_mb_type |= MB_TYPE_L0L1;
    }

                                 MB_TYPE_P1L0 | MB_TYPE_P1L1)) |
    }

        } else {
        }
    } else {                                             // AFL/AFR/FR/FL -> AFR/FR
                IS_INTERLACED(mb_type_col[1])) {
                mb_type_col[0] &= ~MB_TYPE_INTERLACED;
                mb_type_col[1] &= ~MB_TYPE_INTERLACED;
            }

                !is_b8x8) {
            } else {
            }
        } else {                                         //     AFR/FR    -> AFR/FR

                            (mb_type_col[0] & (MB_TYPE_16x8 | MB_TYPE_8x16));
            } else {
                    /* FIXME: Save sub mb types from previous frames (or derive
                     * from MVs) so we know exactly what block size to use. */
                }
            }
        }
    }


        }
    }

        int n = 0;


            } else {
                a = mv[0];
                b = mv[1];
            }
        }
                                     MB_TYPE_P1L0 | MB_TYPE_P1L1)) |

        } else {
            a = mv[0];
            b = mv[1];
        }
    } else {
        int n = 0;



            /* col_zero_flag */
                                           8, 0, 4);
                                           8, 0, 4);
                    }
                } else {
                    int m = 0;
                        }
                    }
                }
            }
        }
                                     MB_TYPE_P1L0 | MB_TYPE_P1L1)) |
    }
}

                                    int *mb_type)
{



        } else {
        }
    } else {                                        // AFL/AFR/FR/FL -> AFR/FR
                IS_INTERLACED(mb_type_col[1])) {
                mb_type_col[0] &= ~MB_TYPE_INTERLACED;
                mb_type_col[1] &= ~MB_TYPE_INTERLACED;
            }

                          MB_TYPE_DIRECT2;                  /* B_SUB_8x8 */

                !is_b8x8) {
                            MB_TYPE_DIRECT2;                /* B_16x8 */
            } else {
            }
        } else {                                    //     AFR/FR    -> AFR/FR

                          MB_TYPE_DIRECT2;                  /* B_SUB_8x8 */
                            MB_TYPE_DIRECT2;                /* B_16x16 */
                            (mb_type_col[0] & (MB_TYPE_16x8 | MB_TYPE_8x16));
            } else {
                    /* FIXME: save sub mb types from previous frames (or derive
                     * from MVs) so we know exactly what block size to use */
                                  MB_TYPE_DIRECT2;          /* B_SUB_4x4 */
                }
            }
        }
    }


        }
    }

    {

        }




                }

                else {
                                               ref_offset];
                }
                               ref0, 1);

                {
                                   pack16to32(mx, my), 4);
                }
            }
        }

        /* one-to-one mv scaling */


                ref = mv0 = mv1 = 0;
            } else {
            }
        } else {

                }

                else {
                }

                               ref0, 1);
                                   pack16to32(mx, my), 4);
                } else {
                                 pack16to32(mv_l0[0] - mv_col[0],
                                            mv_l0[1] - mv_col[1]));
                    }
                }
            }
        }
    }
}

                                int *mb_type)
{
    else
