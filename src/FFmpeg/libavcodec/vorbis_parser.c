/*
 * Copyright (c) 2012 Justin Ruggles
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
 * Vorbis audio parser
 *
 * Determines the duration for each packet.
 */

#include "libavutil/log.h"

#include "get_bits.h"
#include "parser.h"
#include "xiph.h"
#include "vorbis_parser_internal.h"

static const AVClass vorbis_parser_class = {
    .class_name = "Vorbis parser",
    .item_name  = av_default_item_name,
    .version    = LIBAVUTIL_VERSION_INT,
};

                           const uint8_t *buf, int buf_size)
{
    /* Id header should be 30 bytes */
        av_log(s, AV_LOG_ERROR, "Id header is too short\n");
        return AVERROR_INVALIDDATA;
    }

    /* make sure this is the Id header */
        av_log(s, AV_LOG_ERROR, "Wrong packet type in Id header\n");
        return AVERROR_INVALIDDATA;
    }

    /* check for header signature */
        av_log(s, AV_LOG_ERROR, "Invalid packet signature in Id header\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(s, AV_LOG_ERROR, "Invalid framing bit in Id header\n");
        return AVERROR_INVALIDDATA;
    }


}

                              const uint8_t *buf, int buf_size)
{

    /* avoid overread */
        av_log(s, AV_LOG_ERROR, "Setup header is too short\n");
        return AVERROR_INVALIDDATA;
    }

    /* make sure this is the Setup header */
        av_log(s, AV_LOG_ERROR, "Wrong packet type in Setup header\n");
        return AVERROR_INVALIDDATA;
    }

    /* check for header signature */
        av_log(s, AV_LOG_ERROR, "Invalid packet signature in Setup header\n");
        return AVERROR_INVALIDDATA;
    }

    /* reverse bytes so we can easily read backwards with get_bits() */
        av_log(s, AV_LOG_ERROR, "Out of memory\n");
        return AVERROR(ENOMEM);
    }

            got_framing_bit = get_bits_count(&gb);
            break;
        }
    }
        av_log(s, AV_LOG_ERROR, "Invalid Setup header\n");
        ret = AVERROR_INVALIDDATA;
        goto bad_header;
    }

    /* Now we search backwards to find possible valid mode counts. This is not
     * fool-proof because we could have false positive matches and read too
     * far, but there isn't really any way to be sure without parsing through
     * all the many variable-sized fields before the modes. This approach seems
     * to work well in testing, and it is similar to how it is handled in
     * liboggz. */
    mode_count = 0;
    got_mode_header = 0;
            break;
            break;
        }
    }
        av_log(s, AV_LOG_ERROR, "Invalid Setup header\n");
        ret = AVERROR_INVALIDDATA;
        goto bad_header;
    }
    /* All samples I've seen use <= 2 modes, so ask for a sample if we find
     * more than that, as it is most likely a false positive. If we get any
     * we may need to approach this the long way and parse the whole Setup
     * header, but I hope very much that it never comes to that. */
        avpriv_request_sample(s,
                              "%d modes (either a false positive or a "
                              "sample from an unknown encoder)",
                              last_mode_count);
    }
    /* We're limiting the mode count to 63 so that we know that the previous
     * block flag will be in the first packet byte. */
        av_log(s, AV_LOG_ERROR, "Unsupported mode count: %d\n",
               last_mode_count);
        ret = AVERROR_INVALIDDATA;
        goto bad_header;
    }
    /* Determine the number of bits required to code the mode and turn that
     * into a bitmask to directly access the mode from the first frame byte. */
    /* The previous window flag is the next bit after the mode */

    }

}

                             const uint8_t *extradata, int extradata_size)
{


                                         extradata_size, 30,
                                         header_start, header_len)) < 0) {
        av_log(s, AV_LOG_ERROR, "Extradata corrupt.\n");
        return ret;
    }

        return ret;

        return ret;


}

                                int buf_size, int *flags)
{


            /* If the user doesn't care about special packets, it's a bad one. */
            if (!flags)
                goto bad_packet;

            /* Set the flag for which kind of special packet it is. */
            if (buf[0] == 1)
                *flags |= VORBIS_FLAG_HEADER;
            else if (buf[0] == 3)
                *flags |= VORBIS_FLAG_COMMENT;
            else if (buf[0] == 5)
                *flags |= VORBIS_FLAG_SETUP;
            else
                goto bad_packet;

            /* Special packets have no duration. */
            return 0;

bad_packet:
            av_log(s, AV_LOG_ERROR, "Invalid packet\n");
            return AVERROR_INVALIDDATA;
        }
            mode = 0;
        else
            av_log(s, AV_LOG_ERROR, "Invalid mode in packet\n");
            return AVERROR_INVALIDDATA;
        }
        }
    }

    return duration;
}

                          int buf_size)
{
}

{

{

                                           int extradata_size)
{

        return NULL;

        av_vorbis_parse_free(&s);
        return NULL;
    }

}

#if CONFIG_VORBIS_PARSER

typedef struct VorbisParseContext {
    AVVorbisParseContext *vp;
} VorbisParseContext;

                        const uint8_t **poutbuf, int *poutbuf_size,
                        const uint8_t *buf, int buf_size)
{

    }
        goto end;


end:
    /* always return the full packet. this parser isn't doing any splitting or
       combining, only packet analysis */
}

{

AVCodecParser ff_vorbis_parser = {
    .codec_ids      = { AV_CODEC_ID_VORBIS },
    .priv_data_size = sizeof(VorbisParseContext),
    .parser_parse   = vorbis_parse,
    .parser_close   = vorbis_parser_close,
};
#endif /* CONFIG_VORBIS_PARSER */
