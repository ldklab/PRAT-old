/*
 * Copyright (c) 2012 Konstantin Shishkov
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
 * Common functions for Microsoft Screen 1 and 2
 */

#include <inttypes.h>

#include "libavutil/intfloat.h"
#include "libavutil/intreadwrite.h"
#include "avcodec.h"
#include "mss12.h"

enum SplitMode {
    SPLIT_VERT = 0,
    SPLIT_HOR,
    SPLIT_NONE
};

static const int sec_order_sizes[4] = { 1, 7, 6, 1 };

enum ContextDirection {
    TOP_LEFT = 0,
    TOP,
    TOP_RIGHT,
    LEFT
};

{


}

{

    }
}

{
}

{

        }
    }

{




        }
    }

{

    else {
    }



                                int full_model_syms, int special_initial_cache)
{



                           i ? THRESH_LOW : THRESH_ADAPTIVE);

                                         uint8_t *ngb, int num_ngb, int any_ngb)
{

        return AVERROR_INVALIDDATA;
            int idx, j;

            idx = 0;
                        break;
                        break;
                }
            }
        }
    } else {
                break;
        val = i;
    }
    }

    return pix;
}

                                   uint8_t *src, ptrdiff_t stride, int x, int y,
                                   int has_right)
{

    } else {
        } else {
        }
        else
    }


                break;
    }

    case 1:
        layer = 0;
        break;
                layer = 1;
                layer = 2;
            else
                layer = 4;
            else
            layer = 6;
        } else {
        }
        break;
            layer = 8;
            layer = 9;
            layer = 10;
            layer = 11;
            layer = 12;
        else
        break;
    }

                                &pctx->sec_models[layer][sub]);
    else
}

                         int x, int y, int width, int height, ptrdiff_t stride,
                         ptrdiff_t rgb_stride, PixContext *pctx,
                         const uint32_t *pal)
{


            else
                return p;

        }
    }

    return 0;
}

                            int x, int y, int width, int height)
{

                   width);
        }

                               int x, int y, int width, int height)
{
        return -1;
    else {
        } else {
        }
        }
    }
    return 0;
}

                                uint8_t *dst, ptrdiff_t stride, uint8_t *mask,
                                ptrdiff_t mask_stride, int x, int y,
                                int width, int height,
                                PixContext *pctx)
{


                ( c->rgb_pic && mask[i] != 0x01 && mask[i] != 0x02 && mask[i] != 0x04 ||
                 !c->rgb_pic && mask[i] != 0x80 && mask[i] != 0xFF))
                return -1;

                    return -1;
                else
                    return p;
            }
        }
    }

    return 0;
}

                                      int version, int full_model_syms)
{


                full_model_syms, version ? 1 : 0);

{

{


            return -1;

    }

        return -1;

}

                               int x, int y, int width, int height)
{



            return pix;
        }
    } else {
                             x, y, width, height, c->pal_stride, c->rgb_stride,
                             &sc->intra_pix_ctx, &c->pal[0]);
    }

    return 0;
}

                               int x, int y, int width, int height)
{


            return mode;

            ( c->rgb_pic && mode != 0x01 && mode != 0x02 && mode != 0x04 ||
             !c->rgb_pic && mode != 0x80 && mode != 0xFF))
            return -1;

            return motion_compensation(c, x, y, width, height);
    } else {
                          x, y, width, height, c->mask_stride, 0,
                          &sc->inter_pix_ctx, &c->pal[0]) < 0)
            return -1;
                                    c->pal_stride, c->mask,
                                    c->mask_stride,
                                    x, y, width, height,
                                    &sc->intra_pix_ctx);
    }

    return 0;
}

                         int x, int y, int width, int height)
{
        return AVERROR_INVALIDDATA;


            return -1;
            return -1;
            return -1;
        break;
            return -1;
            return -1;
            return -1;
        break;
        else
    default:
        return -1;
    }

    return 0;
}

                                 SliceContext* sc1, SliceContext *sc2)
{

        av_log(avctx, AV_LOG_ERROR, "Insufficient extradata size %d\n",
               avctx->extradata_size);
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR,
               "Insufficient extradata size: expected %"PRIu32" got %d\n",
               AV_RB32(avctx->extradata),
               avctx->extradata_size);
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "Frame dimensions %dx%d too large",
               avctx->coded_width, avctx->coded_height);
        return AVERROR_INVALIDDATA;
    }
        av_log(avctx, AV_LOG_ERROR, "Frame dimensions %dx%d too small",
               avctx->coded_width, avctx->coded_height);
        return AVERROR_INVALIDDATA;
    }

           AV_RB32(avctx->extradata + 4), AV_RB32(avctx->extradata + 8));
        av_log(avctx, AV_LOG_ERROR,
               "Header version doesn't match codec tag\n");
        return -1;
    }

        av_log(avctx, AV_LOG_ERROR,
               "Incorrect number of changeable palette entries: %d\n",
               c->free_colours);
        return AVERROR_INVALIDDATA;
    }

           avctx->coded_width, avctx->coded_height);

            av_log(avctx, AV_LOG_ERROR,
                   "Insufficient extradata size %d for v2\n",
                   avctx->extradata_size);
            return AVERROR_INVALIDDATA;
        }


            av_log(avctx, AV_LOG_ERROR,
                   "Incorrect number of used colours %d\n",
                   c->full_model_syms);
            return AVERROR_INVALIDDATA;
        }
               c->full_model_syms);
    } else {
    }

                            (version ? 8 : 0) + i * 3);

        av_log(avctx, AV_LOG_ERROR, "Cannot allocate mask plane\n");
        return AVERROR(ENOMEM);
    }

    }

}

{

}
