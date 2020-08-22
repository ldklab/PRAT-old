/*
 * MPEG Audio parser
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

#include "parser.h"
#include "mpegaudiodecheader.h"
#include "libavutil/common.h"
#include "libavformat/apetag.h" // for APE tag.
#include "libavformat/id3v1.h" // for ID3v1_TAG_SIZE

typedef struct MpegAudioParseContext {
    ParseContext pc;
    int frame_size;
    uint32_t header;
    int header_count;
    int no_bitrate;
} MpegAudioParseContext;

#define MPA_HEADER_SIZE 4

/* header + layer + freq + lsf/mpeg25 */
#define SAME_HEADER_MASK \
   (0xffe00000 | (3 << 17) | (3 << 10) | (3 << 19))

                           AVCodecContext *avctx,
                           const uint8_t **poutbuf, int *poutbuf_size,
                           const uint8_t *buf, int buf_size)
{


                next= i;
                break;
            }
        }else{


                } else {

                        }
                    }

                        avpriv_report_missing_feature(avctx,
                            "MP3ADU full parser");
                        *poutbuf = NULL;
                        *poutbuf_size = 0;
                        return buf_size; /* parsers must not return error codes */
                    }

                }
            }
        }
    }

    }

    }

        *poutbuf = NULL;
        *poutbuf_size = 0;
        return next;
    }

}


AVCodecParser ff_mpegaudio_parser = {
    .codec_ids      = { AV_CODEC_ID_MP1, AV_CODEC_ID_MP2, AV_CODEC_ID_MP3, AV_CODEC_ID_MP3ADU },
    .priv_data_size = sizeof(MpegAudioParseContext),
    .parser_parse   = mpegaudio_parse,
    .parser_close   = ff_parse_close,
};
