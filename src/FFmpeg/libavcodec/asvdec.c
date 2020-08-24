/*
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

/**
 * @file
 * ASUS V1/V2 decoder.
 */

#include "libavutil/attributes.h"
#include "libavutil/mem.h"

#include "asv.h"
#include "avcodec.h"
#include "blockdsp.h"
#include "idctdsp.h"
#include "internal.h"
#include "mathops.h"
#include "mpeg12data.h"

#define VLC_BITS             6
#define ASV2_LEVEL_VLC_BITS 10

static VLC ccp_vlc;
static VLC level_vlc;
static VLC dc_ccp_vlc;
static VLC ac_ccp_vlc;
static VLC asv2_level_vlc;

static av_cold void init_vlcs(ASV1Context *a)
{
    static int done = 0;

    if (!done) {
        done = 1;

        INIT_VLC_STATIC(&ccp_vlc, VLC_BITS, 17,
                        &ff_asv_ccp_tab[0][1], 2, 1,
                        &ff_asv_ccp_tab[0][0], 2, 1, 64);
        INIT_VLC_STATIC(&dc_ccp_vlc, VLC_BITS, 8,
                        &ff_asv_dc_ccp_tab[0][1], 2, 1,
                        &ff_asv_dc_ccp_tab[0][0], 2, 1, 64);
        INIT_VLC_STATIC(&ac_ccp_vlc, VLC_BITS, 16,
                        &ff_asv_ac_ccp_tab[0][1], 2, 1,
                        &ff_asv_ac_ccp_tab[0][0], 2, 1, 64);
        INIT_VLC_STATIC(&level_vlc,  VLC_BITS, 7,
                        &ff_asv_level_tab[0][1], 2, 1,
                        &ff_asv_level_tab[0][0], 2, 1, 64);
        INIT_VLC_STATIC(&asv2_level_vlc, ASV2_LEVEL_VLC_BITS, 63,
                        &ff_asv2_level_tab[0][1], 2, 1,
                        &ff_asv2_level_tab[0][0], 2, 1, 1024);
    }
}

// FIXME write a reversed bitstream reader to avoid the double reverse
{
}

{

    else
}

{

    else
}

{



                break;
                av_log(a->avctx, AV_LOG_ERROR, "coded coeff pattern damaged\n");
                return AVERROR_INVALIDDATA;
            }

        }
    }

    return 0;
}

{



    }


        }
    }

}

{


                return ret;
        }
    } else {
                return ret;
        }
    }
    return 0;
}

{



    }

                        AVPacket *avpkt)
{

        return AVERROR_INVALIDDATA;

        return ret;

                          buf_size);
        return AVERROR(ENOMEM);

                           (const uint32_t *) buf, buf_size / 4);
    } else {
        int i;
    }


                return ret;

        }
    }

                return ret;

        }
    }

                return ret;

        }
    }



}

{

        av_log(avctx, AV_LOG_WARNING, "No extradata provided\n");
    }


        av_log(avctx, AV_LOG_ERROR, "illegal qscale 0\n");
        if (avctx->codec_id == AV_CODEC_ID_ASV1)
            a->inv_qscale = 6;
        else
            a->inv_qscale = 10;
    }


    }

}

{


}

#if CONFIG_ASV1_DECODER
AVCodec ff_asv1_decoder = {
    .name           = "asv1",
    .long_name      = NULL_IF_CONFIG_SMALL("ASUS V1"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_ASV1,
    .priv_data_size = sizeof(ASV1Context),
    .init           = decode_init,
    .close          = decode_end,
    .decode         = decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
#endif

#if CONFIG_ASV2_DECODER
AVCodec ff_asv2_decoder = {
    .name           = "asv2",
    .long_name      = NULL_IF_CONFIG_SMALL("ASUS V2"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_ASV2,
    .priv_data_size = sizeof(ASV1Context),
    .init           = decode_init,
    .close          = decode_end,
    .decode         = decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
#endif
