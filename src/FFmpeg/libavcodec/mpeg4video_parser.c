/*
 * MPEG-4 video frame extraction
 * Copyright (c) 2003 Fabrice Bellard
 * Copyright (c) 2003 Michael Niedermayer
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

#define UNCHECKED_BITSTREAM_READER 1

#include "internal.h"
#include "parser.h"
#include "mpegvideo.h"
#include "mpeg4video.h"
#include "mpeg4video_parser.h"

struct Mp4vParseContext {
    ParseContext pc;
    Mpeg4DecContext dec_ctx;
    int first_picture;
};

{


            }
        }
    }

        /* EOF considered as end of frame */
            return 0;
                    continue;
            }
        }
    }
}

/* XXX: make it use less memory */
                               const uint8_t *buf, int buf_size)
{


            av_log(avctx, AV_LOG_WARNING, "Failed to parse extradata\n");
    }

            return ret;
    }

    }

}

{


}

                            AVCodecContext *avctx,
                            const uint8_t **poutbuf, int *poutbuf_size,
                            const uint8_t *buf, int buf_size)
{

    } else {

        }
    }

}

AVCodecParser ff_mpeg4video_parser = {
    .codec_ids      = { AV_CODEC_ID_MPEG4 },
    .priv_data_size = sizeof(struct Mp4vParseContext),
    .parser_init    = mpeg4video_parse_init,
    .parser_parse   = mpeg4video_parse,
    .parser_close   = ff_parse_close,
    .split          = ff_mpeg4video_split,
};
