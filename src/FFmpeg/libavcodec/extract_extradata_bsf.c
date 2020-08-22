/*
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

#include <stdint.h>

#include "libavutil/common.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/log.h"
#include "libavutil/opt.h"

#include "av1.h"
#include "av1_parse.h"
#include "bsf.h"
#include "bsf_internal.h"
#include "bytestream.h"
#include "h2645_parse.h"
#include "h264.h"
#include "hevc.h"
#include "vc1_common.h"

typedef struct ExtractExtradataContext {
    const AVClass *class;

    int (*extract)(AVBSFContext *ctx, AVPacket *pkt,
                   uint8_t **data, int *size);

    /* AV1 specific fields */
    AV1Packet av1_pkt;

    /* H264/HEVC specific fields */
    H2645Packet h2645_pkt;

    /* AVOptions */
    int remove;
} ExtractExtradataContext;

{
            return 1;
    return 0;
}

                                 uint8_t **data, int *size)
{
        AV1_OBU_SEQUENCE_HEADER, AV1_OBU_METADATA,
    };


        return ret;

            filtered_size += obu->raw_size;
        }
    }


            filtered_buf = av_buffer_alloc(filtered_size + AV_INPUT_BUFFER_PADDING_SIZE);
            if (!filtered_buf) {
                return AVERROR(ENOMEM);
            }
            memset(filtered_buf->data + filtered_size, 0, AV_INPUT_BUFFER_PADDING_SIZE);
        }

            av_buffer_unref(&filtered_buf);
            return AVERROR(ENOMEM);
        }


            bytestream2_init_writer(&pb_filtered_data, filtered_buf->data, filtered_size);

                             obu->type)) {
            }
        }

            av_buffer_unref(&pkt->buf);
            pkt->buf  = filtered_buf;
            pkt->data = filtered_buf->data;
            pkt->size = filtered_size;
        }
    }

    return 0;
}

                                   uint8_t **data, int *size)
{
        HEVC_NAL_VPS, HEVC_NAL_SPS, HEVC_NAL_PPS,
    };
        H264_NAL_SPS, H264_NAL_PPS,
    };



        extradata_nal_types    = extradata_nal_types_hevc;
        nb_extradata_nal_types = FF_ARRAY_ELEMS(extradata_nal_types_hevc);
    } else {
    }

                                ctx, 0, 0, ctx->par_in->codec_id, 1, 0);
        return ret;

            } else {
            }
            filtered_size += nal->raw_size + 3;
        }
    }


            filtered_buf = av_buffer_alloc(filtered_size + AV_INPUT_BUFFER_PADDING_SIZE);
            if (!filtered_buf) {
                return AVERROR(ENOMEM);
            }
            memset(filtered_buf->data + filtered_size, 0, AV_INPUT_BUFFER_PADDING_SIZE);
        }

            av_buffer_unref(&filtered_buf);
            return AVERROR(ENOMEM);
        }


            bytestream2_init_writer(&pb_filtered_data, filtered_buf->data, filtered_size);

                             nal->type)) {
                bytestream2_put_be24u(&pb_filtered_data, 1); // startcode
            }
        }

            av_buffer_unref(&pkt->buf);
            pkt->buf  = filtered_buf;
            pkt->data = filtered_buf->data;
            pkt->size = filtered_size;
        }
    }

    return 0;
}

                                 uint8_t **data, int *size)
{

            has_extradata = 1;
        }
    }

            return AVERROR(ENOMEM);


            pkt->data += extradata_size;
            pkt->size -= extradata_size;
        }
    }

    return 0;
}

                                     uint8_t **data, int *size)
{

            found = 1;
                    return AVERROR(ENOMEM);


                    pkt->data += *size;
                    pkt->size -= *size;
                }
            }
            break;
        }
    }
    return 0;
}

                                   uint8_t **data, int *size)
{

                    return AVERROR(ENOMEM);


                    pkt->data += *size;
                    pkt->size -= *size;
                }
            }
            break;
        }
    }
    return 0;
}

static const struct {
    enum AVCodecID id;
    int (*extract)(AVBSFContext *ctx, AVPacket *pkt,
                   uint8_t **data, int *size);
} extract_tab[] = {
    { AV_CODEC_ID_AV1,        extract_extradata_av1     },
    { AV_CODEC_ID_AVS2,       extract_extradata_mpeg4   },
    { AV_CODEC_ID_CAVS,       extract_extradata_mpeg4   },
    { AV_CODEC_ID_H264,       extract_extradata_h2645   },
    { AV_CODEC_ID_HEVC,       extract_extradata_h2645   },
    { AV_CODEC_ID_MPEG1VIDEO, extract_extradata_mpeg12  },
    { AV_CODEC_ID_MPEG2VIDEO, extract_extradata_mpeg12  },
    { AV_CODEC_ID_MPEG4,      extract_extradata_mpeg4   },
    { AV_CODEC_ID_VC1,        extract_extradata_vc1     },
};

{

        }
    }
        return AVERROR_BUG;

    return 0;
}

{

        return ret;

        goto fail;

                                      extradata, extradata_size);
            av_freep(&extradata);
            goto fail;
        }
    }

    return 0;

fail:
    av_packet_unref(pkt);
    return ret;
}

{

static const enum AVCodecID codec_ids[] = {
    AV_CODEC_ID_AV1,
    AV_CODEC_ID_AVS2,
    AV_CODEC_ID_CAVS,
    AV_CODEC_ID_H264,
    AV_CODEC_ID_HEVC,
    AV_CODEC_ID_MPEG1VIDEO,
    AV_CODEC_ID_MPEG2VIDEO,
    AV_CODEC_ID_MPEG4,
    AV_CODEC_ID_VC1,
    AV_CODEC_ID_NONE,
};

#define OFFSET(x) offsetof(ExtractExtradataContext, x)
#define FLAGS (AV_OPT_FLAG_VIDEO_PARAM|AV_OPT_FLAG_BSF_PARAM)
static const AVOption options[] = {
    { "remove", "remove the extradata from the bitstream", OFFSET(remove), AV_OPT_TYPE_INT,
        { .i64 = 0 }, 0, 1, FLAGS },
    { NULL },
};

static const AVClass extract_extradata_class = {
    .class_name = "extract_extradata",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
};

const AVBitStreamFilter ff_extract_extradata_bsf = {
    .name           = "extract_extradata",
    .codec_ids      = codec_ids,
    .priv_data_size = sizeof(ExtractExtradataContext),
    .priv_class     = &extract_extradata_class,
    .init           = extract_extradata_init,
    .filter         = extract_extradata_filter,
    .close          = extract_extradata_close,
};
