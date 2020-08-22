/*
 * DCA parser
 * Copyright (C) 2004 Gildas Bazin
 * Copyright (C) 2004 Benjamin Zores
 * Copyright (C) 2006 Benjamin Larsson
 * Copyright (C) 2007 Konstantin Shishkov
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

#include "dca.h"
#include "dca_core.h"
#include "dca_exss.h"
#include "dca_lbr.h"
#include "dca_syncwords.h"
#include "get_bits.h"
#include "parser.h"

typedef struct DCAParseContext {
    ParseContext pc;
    uint32_t lastmarker;
    int size;
    int framesize;
    unsigned int startpos;
    DCAExssParser exss;
    unsigned int sr_code;
} DCAParseContext;

#define IS_CORE_MARKER(state) \
    (((state & 0xFFFFFFFFF0FF) == (((uint64_t)DCA_SYNCWORD_CORE_14B_LE << 16) | 0xF007)) || \
     ((state & 0xFFFFFFFFFFF0) == (((uint64_t)DCA_SYNCWORD_CORE_14B_BE << 16) | 0x07F0)) || \
     ((state & 0xFFFFFFFF00FC) == (((uint64_t)DCA_SYNCWORD_CORE_LE     << 16) | 0x00FC)) || \
     ((state & 0xFFFFFFFFFC00) == (((uint64_t)DCA_SYNCWORD_CORE_BE     << 16) | 0xFC00)))

#define IS_EXSS_MARKER(state)   ((state & 0xFFFFFFFF) == DCA_SYNCWORD_SUBSTREAM)

#define IS_MARKER(state)        (IS_CORE_MARKER(state) || IS_EXSS_MARKER(state))

#define CORE_MARKER(state)      ((state >> 16) & 0xFFFFFFFF)
#define EXSS_MARKER(state)      (state & 0xFFFFFFFF)

#define STATE_LE(state)     (((state & 0xFF00FF00) >> 8) | ((state & 0x00FF00FF) << 8))
#define STATE_14(state)     (((state & 0x3FFF0000) >> 8) | ((state & 0x00003FFF) >> 6))

#define CORE_FRAMESIZE(state)   (((state >> 4) & 0x3FFF) + 1)
#define EXSS_FRAMESIZE(state)   ((state & 0x2000000000) ? \
                                 ((state >>  5) & 0xFFFFF) + 1 : \
                                 ((state >> 13) & 0x0FFFF) + 1)

/**
 * Find the end of the current frame in the bitstream.
 * @return the position of the first byte of the next frame, or -1
 */
                              int buf_size)
{



                  pc1->lastmarker == DCA_SYNCWORD_SUBSTREAM)) {

                else


            }
        }
    }


                    }
                    break;
                case DCA_SYNCWORD_CORE_LE:
                    if (size == 2) {
                        pc1->framesize = CORE_FRAMESIZE(STATE_LE(state));
                        start_found    = 4;
                    }
                    break;
                case DCA_SYNCWORD_CORE_14B_BE:
                    if (size == 4) {
                        pc1->framesize = CORE_FRAMESIZE(STATE_14(state));
                        start_found    = 4;
                    }
                    break;
                case DCA_SYNCWORD_CORE_14B_LE:
                    if (size == 4) {
                        pc1->framesize = CORE_FRAMESIZE(STATE_14(STATE_LE(state)));
                        start_found    = 4;
                    }
                    break;
                    }
                    break;
                default:
                    av_assert0(0);
                }
            }

            }

                }
            }


                 pc1->lastmarker == DCA_SYNCWORD_SUBSTREAM)) {
            }
        }
    }

}

{

}

                            int buf_size, int *duration, int *sample_rate,
                            int *profile)
{

        return AVERROR_INVALIDDATA;

            return ret;

            if ((ret = init_get_bits8(&gb, buf + asset->lbr_offset, asset->lbr_size)) < 0)
                return ret;

            if (get_bits_long(&gb, 32) != DCA_SYNCWORD_LBR)
                return AVERROR_INVALIDDATA;

            switch (get_bits(&gb, 8)) {
            case DCA_LBR_HEADER_DECODER_INIT:
                pc1->sr_code = get_bits(&gb, 8);
            case DCA_LBR_HEADER_SYNC_ONLY:
                break;
            default:
                return AVERROR_INVALIDDATA;
            }

            if (pc1->sr_code >= FF_ARRAY_ELEMS(ff_dca_sampling_freqs))
                return AVERROR_INVALIDDATA;

            *sample_rate = ff_dca_sampling_freqs[pc1->sr_code];
            *duration = 1024 << ff_dca_freq_ranges[pc1->sr_code];
            *profile = FF_PROFILE_DTS_EXPRESS;
            return 0;
        }


                return ret;

                return AVERROR_INVALIDDATA;

                return AVERROR_INVALIDDATA;

                return AVERROR_INVALIDDATA;

        }

        return AVERROR_INVALIDDATA;
    }

                                            hdr, DCA_CORE_FRAME_HEADER_SIZE)) < 0)
        return ret;
        return AVERROR_INVALIDDATA;

        return 0;

        case DCA_EXT_AUDIO_XXCH:
        }

        return 0;

        return 0;
        return 0;


    return 0;
}

                     const uint8_t **poutbuf, int *poutbuf_size,
                     const uint8_t *buf, int buf_size)
{

    } else {

        }

        /* skip initial padding */
        }
    }

    /* read the duration and sample rate from the frame header */
    } else

}

AVCodecParser ff_dca_parser = {
    .codec_ids      = { AV_CODEC_ID_DTS },
    .priv_data_size = sizeof(DCAParseContext),
    .parser_init    = dca_parse_init,
    .parser_parse   = dca_parse,
    .parser_close   = ff_parse_close,
};
