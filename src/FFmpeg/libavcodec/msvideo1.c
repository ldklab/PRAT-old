/*
 * Microsoft Video-1 Decoder
 * Copyright (C) 2003 The FFmpeg project
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
 * Microsoft Video-1 Decoder by Mike Melanson (melanson@pcisys.net)
 * For more information about the MS Video-1 format, visit:
 *   http://www.pcisys.net/~melanson/codecs/
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libavutil/internal.h"
#include "libavutil/intreadwrite.h"
#include "avcodec.h"
#include "internal.h"

#define PALETTE_COUNT 256
#define CHECK_STREAM_PTR(n) \
  if ((stream_ptr + n) > s->size ) { \
    av_log(s->avctx, AV_LOG_ERROR, " MS Video-1 warning: stream_ptr out of bounds (%d >= %d)\n", \
      stream_ptr + n, s->size); \
    return; \
  }

typedef struct Msvideo1Context {

    AVCodecContext *avctx;
    AVFrame *frame;

    const unsigned char *buf;
    int size;

    int mode_8bit;  /* if it's not 8-bit, it's 16-bit */

    uint32_t pal[256];
} Msvideo1Context;

{


        return AVERROR_INVALIDDATA;

    /* figure out the colorspace based on the presence of a palette */
    } else {
    }

        return AVERROR(ENOMEM);

    return 0;
}

{

    /* decoding parameters */


            /* check if this block should be skipped */
            }


            /* get the next two bytes in the encoded data stream */

            /* check if the decode is finished */
                return;
                /* skip code, but don't count the current block */
                /* 2-color encoding */


                }
                /* 8-color encoding */


                }
            } else {
                /* 1-color encoding */

                }
            }

        }
    }

    /* make the palette available on the way out */
}

{

    /* decoding parameters */


            /* check if this block should be skipped */
            }


            /* get the next two bytes in the encoded data stream */

            /* check if the decode is finished */
                return;
                /* skip code, but don't count the current block */
                /* 2- or 8-color encoding modes */


                    /* 8-color encoding */

                    }
                } else {
                    /* 2-color encoding */
                    }
                }
            } else {
                /* otherwise, it's a 1-color block */

                }
            }

        }
    }
}

                                void *data, int *got_frame,
                                AVPacket *avpkt)
{


    // Discard frame if its smaller than the minimum frame size
        av_log(avctx, AV_LOG_ERROR, "Packet is too small\n");
        return AVERROR_INVALIDDATA;
    }

        return ret;


            av_log(avctx, AV_LOG_ERROR, "Palette size %d is wrong\n", size);
        }
    }

    else

        return ret;


    /* report that the buffer was completely consumed */
}

{


}

AVCodec ff_msvideo1_decoder = {
    .name           = "msvideo1",
    .long_name      = NULL_IF_CONFIG_SMALL("Microsoft Video 1"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_MSVIDEO1,
    .priv_data_size = sizeof(Msvideo1Context),
    .init           = msvideo1_decode_init,
    .close          = msvideo1_decode_end,
    .decode         = msvideo1_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
