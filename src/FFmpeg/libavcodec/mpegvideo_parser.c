/*
 * MPEG-1 / MPEG-2 video parser
 * Copyright (c) 2000,2001 Fabrice Bellard
 * Copyright (c) 2002-2004 Michael Niedermayer <michaelni@gmx.at>
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

#include "parser.h"
#include "mpeg12.h"
#include "internal.h"

struct MpvParseContext {
    ParseContext pc;
    AVRational frame_rate;
    int progressive_sequence;
    int width, height;
};


                                      AVCodecContext *avctx,
                                      const uint8_t *buf, int buf_size)
{
//FIXME replace the crap with get_bits()

            }
            break;
                }
            }
            break;

                        case 1: pix_fmt = AV_PIX_FMT_YUV420P; break;
                        case 2: pix_fmt = AV_PIX_FMT_YUV422P; break;
                        case 3: pix_fmt = AV_PIX_FMT_YUV444P; break;
                        }

                    }
                    break;

                        /* check if we must repeat the frame */
                                if (top_field_first)
                                    s->repeat_pict = 5;
                                else
                                    s->repeat_pict = 3;
                            }
                        }

                            else
                        } else
                    }
                    break;
                }
            }
            break;
        case -1:
            goto the_end;
            /* we stop parsing when we encounter a slice. It ensures
               that this function takes a negligible amount of time */
                start_code <= SLICE_MAX_START_CODE)
            break;
        }
    }
        av_log(avctx, AV_LOG_ERROR, "Failed to set dimensions\n");

    }
    }

    }

#if FF_API_AVCTX_TIMEBASE
#endif

                           AVCodecContext *avctx,
                           const uint8_t **poutbuf, int *poutbuf_size,
                           const uint8_t *buf, int buf_size)
{

    }else{

        }

    }
    /* we have a full frame : we just parse the first few MPEG headers
       to have the full timing information. The time take by this
       function should be negligible for uncorrupted streams */
            s->pict_type, av_q2d(avctx->framerate), s->repeat_pict);

}

static int mpegvideo_split(AVCodecContext *avctx,
                           const uint8_t *buf, int buf_size)
{
    int i;
    uint32_t state= -1;
    int found=0;

    for(i=0; i<buf_size; i++){
        state= (state<<8) | buf[i];
        if(state == 0x1B3){
            found=1;
        }else if(found && state != 0x1B5 && state < 0x200 && state >= 0x100)
            return i-3;
    }
    return 0;
}

{
}

AVCodecParser ff_mpegvideo_parser = {
    .codec_ids      = { AV_CODEC_ID_MPEG1VIDEO, AV_CODEC_ID_MPEG2VIDEO },
    .priv_data_size = sizeof(struct MpvParseContext),
    .parser_init    = mpegvideo_parse_init,
    .parser_parse   = mpegvideo_parse,
    .parser_close   = ff_parse_close,
    .split          = mpegvideo_split,
};
