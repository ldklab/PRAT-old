/*
 * Cinepak Video Decoder
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
 * Cinepak video decoder
 * @author Ewald Snel <ewald@rambo.its.tudelft.nl>
 *
 * @see For more information on the Cinepak algorithm, visit:
 *   http://www.csse.monash.edu.au/~timf/
 * @see For more information on the quirky data inside Sega FILM/CPK files, visit:
 *   http://wiki.multimedia.cx/index.php?title=Sega_FILM
 *
 * Cinepak colorspace support (c) 2013 Rl, Aetey Global Technologies AB
 * @author Cinepak colorspace, Rl, Aetey Global Technologies AB
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libavutil/common.h"
#include "libavutil/intreadwrite.h"
#include "avcodec.h"
#include "internal.h"


typedef uint8_t cvid_codebook[12];

#define MAX_STRIPS      32

typedef struct cvid_strip {
    uint16_t          id;
    uint16_t          x1, y1;
    uint16_t          x2, y2;
    cvid_codebook     v4_codebook[256];
    cvid_codebook     v1_codebook[256];
} cvid_strip;

typedef struct CinepakContext {

    AVCodecContext *avctx;
    AVFrame *frame;

    const unsigned char *data;
    int size;

    int width, height;

    int palette_video;
    cvid_strip strips[MAX_STRIPS];

    int sega_film_skip_bytes;

    uint32_t pal[256];
} CinepakContext;

                                     int chunk_id, int size, const uint8_t *data)
{

    /* check if this chunk contains 4- or 6-element vectors */

                break;

        }


                break;

            }
                }
            }
        } else {
        }
    }

                                   int chunk_id, int size, const uint8_t *data)
{



/* take care of y dimension not being multiple of 4, such streams exist */
                }
            }
        }
/* to get the correct picture for not-multiple-of-4 cases let us fill each
 * block from the bottom up, thus possibly overwriting the bottommost line
 * more than once but ending with the correct data in place
 * (instead of in-loop checking) */

                    return AVERROR_INVALIDDATA;

            }

                        return AVERROR_INVALIDDATA;

                }

                        return AVERROR_INVALIDDATA;

                        ip3[0] = ip3[1] = ip2[0] = ip2[1] = p[6];
                        ip3[2] = ip3[3] = ip2[2] = ip2[3] = p[9];
                        ip1[0] = ip1[1] = ip0[0] = ip0[1] = p[0];
                        ip1[2] = ip1[3] = ip0[2] = ip0[3] = p[3];
                    } else {
                    }

                        return AVERROR_INVALIDDATA;

                    } else {
                    }

                }
            }

            } else {
            }
        }
    }

    return 0;
}

                                 cvid_strip *strip, const uint8_t *data, int size)
{

    /* coordinate sanity checks */
        return AVERROR_INVALIDDATA;

            return AVERROR_INVALIDDATA;



        case 0x21:
        case 0x24:
        case 0x25:
                chunk_size, data);

        case 0x23:
        case 0x26:
        case 0x27:
                chunk_size, data);

        case 0x31:
        case 0x32:
                chunk_size, data);
        }

    }

    return AVERROR_INVALIDDATA;
}

{


        return AVERROR_INVALIDDATA;

    /* if this is the first frame, check for deviant Sega FILM data */
            avpriv_request_sample(s->avctx, "encoded_buf_size 0");
            return AVERROR_PATCHWELCOME;
        }
            /* If the encoded frame size differs from the frame size as indicated
             * by the container file, this data likely comes from a Sega FILM/CPK file.
             * If the frame header is followed by the bytes FE 00 00 06 00 00 then
             * this is probably one of the two known files that have 6 extra bytes
             * after the frame header. Else, assume 2 extra bytes. The container
             * size also cannot be a multiple of the encoded size. */
                (s->data[11] == 0x00) &&
                (s->data[12] == 0x00) &&
                (s->data[13] == 0x06) &&
                (s->data[14] == 0x00) &&
                (s->data[15] == 0x00))
                s->sega_film_skip_bytes = 6;
            else
        } else
    }

        return AVERROR_INVALIDDATA;

            return AVERROR_INVALIDDATA;
    }

    return 0;
}

{





            return AVERROR_INVALIDDATA;

/* zero y1 means "relative to the previous stripe" */
        else
            s->strips[i].y2 = AV_RB16 (&s->data[8]);


            return AVERROR_INVALIDDATA;

                sizeof(s->strips[i].v4_codebook));
                sizeof(s->strips[i].v1_codebook));
        }



    }
    return 0;
}

{



    // check for paletted data
    } else {
    }

        return AVERROR(ENOMEM);

    return 0;
}

                                void *data, int *got_frame,
                                AVPacket *avpkt)
{


        return AVERROR_INVALIDDATA;


    //Empty frame, do not waste time
        return buf_size;

        av_log(avctx, AV_LOG_ERROR, "cinepak_predecode_check failed\n");
        return ret;
    }

        return ret;

            av_log(avctx, AV_LOG_ERROR, "Palette size %d is wrong\n", size);
        }
    }

    }


        return ret;


    /* report that the buffer was completely consumed */
}

{


}

AVCodec ff_cinepak_decoder = {
    .name           = "cinepak",
    .long_name      = NULL_IF_CONFIG_SMALL("Cinepak"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_CINEPAK,
    .priv_data_size = sizeof(CinepakContext),
    .init           = cinepak_decode_init,
    .close          = cinepak_decode_end,
    .decode         = cinepak_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
