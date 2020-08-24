/*
 * MLP parser
 * Copyright (c) 2007 Ian Caulfield
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
 * MLP parser
 */

#include <stdint.h>

#include "libavutil/internal.h"
#include "get_bits.h"
#include "parser.h"
#include "mlp_parse.h"
#include "mlp.h"

typedef struct MLPParseContext
{
    ParseContext pc;

    int bytes_left;

    int in_sync;

    int num_substreams;
} MLPParseContext;

{
}

                     AVCodecContext *avctx,
                     const uint8_t **poutbuf, int *poutbuf_size,
                     const uint8_t *buf, int buf_size)
{


        return 0;

        next = buf_size;
    } else {
            // Not in sync - find a major sync header

                    // ignore if we do not have the data for the start of header
                }
            }

                    av_log(avctx, AV_LOG_WARNING, "ff_combine_frame failed\n");
            }

                av_log(avctx, AV_LOG_WARNING, "ff_combine_frame failed\n");
                return ret;
            }

            return i - 7;
        }

            // Find length of this packet

            /* Copy overread bytes from last frame into buffer. */
                mp->pc.buffer[mp->pc.index++]= mp->pc.buffer[mp->pc.overread_index++];
            }

                if (ff_combine_frame(&mp->pc, END_NOT_FOUND, &buf, &buf_size) != -1)
                    av_log(avctx, AV_LOG_WARNING, "ff_combine_frame failed\n");
                return buf_size;
            }

                goto lost_sync;
            }
        }


        }

    }


        /* The first nibble of a frame is a parity check of the 4-byte
         * access unit header and all the 2- or 4-byte substream headers. */
        // Only check when this isn't a sync frame - syncs have a checksum.



            }
        }

            av_log(avctx, AV_LOG_INFO, "mlpparse: Parity check failed.\n");
            goto lost_sync;
        }
    } else {

            goto lost_sync;


        else

            /* MLP stream */
        } else { /* mh.stream_type == 0xba */
            /* TrueHD stream */
                avctx->channels       = mh.channels_thd_stream1;
                avctx->channel_layout = mh.channel_layout_thd_stream1;
            } else {
            }
        }
        }

            avctx->bit_rate = mh.peak_bitrate;

    }



lost_sync:
    mp->in_sync = 0;
    return 1;
}

AVCodecParser ff_mlp_parser = {
    .codec_ids      = { AV_CODEC_ID_MLP, AV_CODEC_ID_TRUEHD },
    .priv_data_size = sizeof(MLPParseContext),
    .parser_init    = mlp_init,
    .parser_parse   = mlp_parse,
    .parser_close   = ff_parse_close,
};
