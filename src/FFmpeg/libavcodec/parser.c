/*
 * Audio and Video frame extraction
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

#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#include "libavutil/avassert.h"
#include "libavutil/internal.h"
#include "libavutil/mem.h"

#include "internal.h"
#include "parser.h"

{

        return NULL;

    }
    return NULL;

        goto err_out;
        goto err_out;
            goto err_out;
    }
#if FF_API_CONVERGENCE_DURATION
FF_DISABLE_DEPRECATION_WARNINGS
FF_ENABLE_DEPRECATION_WARNINGS
#endif


err_out:
    if (s)
        av_freep(&s->priv_data);
    av_free(s);
    return NULL;
}

{

    }
            // check disabled since MPEG-TS does not send complete PES packets

            }
                break;
        }
    }

                     uint8_t **poutbuf, int *poutbuf_size,
                     const uint8_t *buf, int buf_size,
                     int64_t pts, int64_t dts, int64_t pos)
{


    /* Parsers only work for the specified codec ids. */
               avctx->codec_id == s->parser->codec_ids[1] ||
               avctx->codec_id == s->parser->codec_ids[2] ||
               avctx->codec_id == s->parser->codec_ids[3] ||
               avctx->codec_id == s->parser->codec_ids[4]);

    }

        /* padding is always necessary even if EOF, so we add it here */
        /* add a new packet descriptor */
    }

    }
    /* WARNING: the returned index can be negative */
                                    poutbuf_size, buf, buf_size);
#define FILL(name) if(s->name > 0 && avctx->name <= 0) avctx->name = s->name
    }

    /* update the file pointer */
        /* fill the data for the current frame */

        /* offset of the next frame */
    }
        index = 0;
}

int av_parser_change(AVCodecParserContext *s, AVCodecContext *avctx,
                     uint8_t **poutbuf, int *poutbuf_size,
                     const uint8_t *buf, int buf_size, int keyframe)
{
    if (s && s->parser->split) {
        if (avctx->flags  & AV_CODEC_FLAG_GLOBAL_HEADER ||
            avctx->flags2 & AV_CODEC_FLAG2_LOCAL_HEADER) {
            int i = s->parser->split(avctx, buf, buf_size);
            buf      += i;
            buf_size -= i;
        }
    }

    /* cast to avoid warning about discarding qualifiers */
    *poutbuf      = (uint8_t *) buf;
    *poutbuf_size = buf_size;
    if (avctx->extradata) {
        if (keyframe && (avctx->flags2 & AV_CODEC_FLAG2_LOCAL_HEADER)) {
            int size = buf_size + avctx->extradata_size;

            *poutbuf_size = size;
            *poutbuf      = av_malloc(size + AV_INPUT_BUFFER_PADDING_SIZE);
            if (!*poutbuf)
                return AVERROR(ENOMEM);

            memcpy(*poutbuf, avctx->extradata, avctx->extradata_size);
            memcpy(*poutbuf + avctx->extradata_size, buf,
                   buf_size + AV_INPUT_BUFFER_PADDING_SIZE);
            return 1;
        }
    }

    return 0;
}

{
    }

                     const uint8_t **buf, int *buf_size)
{
        ff_dlog(NULL, "overread %d, state:%"PRIX32" next:%d index:%d o_index:%d\n",
                pc->overread, pc->state, next, pc->index, pc->overread_index);
        ff_dlog(NULL, "%X %X %X %X\n",
                (*buf)[0], (*buf)[1], (*buf)[2], (*buf)[3]);
    }

    /* Copy overread bytes from last frame into buffer. */

        return AVERROR(EINVAL);

    /* flush remaining if EOF */


    /* copy into buffer end return */
                                           AV_INPUT_BUFFER_PADDING_SIZE);

            av_log(NULL, AV_LOG_ERROR, "Failed to reallocate parser buffer to %d\n", *buf_size + pc->index + AV_INPUT_BUFFER_PADDING_SIZE);
            pc->index = 0;
            return AVERROR(ENOMEM);
        }
    }



    /* append to buffer */
                                           AV_INPUT_BUFFER_PADDING_SIZE);
            av_log(NULL, AV_LOG_ERROR, "Failed to reallocate parser buffer to %d\n", next + pc->index + AV_INPUT_BUFFER_PADDING_SIZE);
            pc->overread_index =
            pc->index = 0;
            return AVERROR(ENOMEM);
        }
    }

    }
    /* store overread bytes */
    }

    if (pc->overread) {
        ff_dlog(NULL, "overread %d, state:%"PRIX32" next:%d index:%d o_index:%d\n",
                pc->overread, pc->state, next, pc->index, pc->overread_index);
        ff_dlog(NULL, "%X %X %X %X\n",
                (*buf)[0], (*buf)[1], (*buf)[2], (*buf)[3]);
    }

    return 0;
}

{


{

    }

    return 0;
}
