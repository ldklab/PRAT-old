/*
 * AVPacket functions for libavcodec
 * Copyright (c) 2000, 2001, 2002 Fabrice Bellard
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

#include <string.h>

#include "libavutil/avassert.h"
#include "libavutil/common.h"
#include "libavutil/internal.h"
#include "libavutil/mathematics.h"
#include "libavutil/mem.h"

#include "bytestream.h"
#include "internal.h"
#include "packet.h"
#include "packet_internal.h"

{
#if FF_API_CONVERGENCE_DURATION
FF_DISABLE_DEPRECATION_WARNINGS
FF_ENABLE_DEPRECATION_WARNINGS
#endif

{
        return pkt;


}

{
        return;

}

{
        return AVERROR(EINVAL);

        return ret;


}

{
        return ret;


}

{
        return;
}

{
        return AVERROR(ENOMEM);

            data_offset = 0;
            pkt->data = pkt->buf->data;
        } else {
                return AVERROR(ENOMEM);
        }

                pkt->data = old_data;
                return ret;
            }
        }
    } else {
            return AVERROR(ENOMEM);
            memcpy(pkt->buf->data, pkt->data, pkt->size);
    }

}

{
        return AVERROR(EINVAL);

                                av_buffer_default_free, NULL, 0);
        return AVERROR(ENOMEM);


}

#if FF_API_AVPACKET_OLD_API
FF_DISABLE_DEPRECATION_WARNINGS
#define ALLOC_MALLOC(data, size) data = av_malloc(size)
#define ALLOC_BUF(data, size)                \
do {                                         \
    av_buffer_realloc(&pkt->buf, size);      \
    data = pkt->buf ? pkt->buf->data : NULL; \
} while (0)

#define DUP_DATA(dst, src, size, padding, ALLOC)                        \
    do {                                                                \
        void *data;                                                     \
        if (padding) {                                                  \
            if ((unsigned)(size) >                                      \
                (unsigned)(size) + AV_INPUT_BUFFER_PADDING_SIZE)        \
                goto failed_alloc;                                      \
            ALLOC(data, size + AV_INPUT_BUFFER_PADDING_SIZE);           \
        } else {                                                        \
            ALLOC(data, size);                                          \
        }                                                               \
        if (!data)                                                      \
            goto failed_alloc;                                          \
        memcpy(data, src, size);                                        \
        if (padding)                                                    \
            memset((uint8_t *)data + size, 0,                           \
                   AV_INPUT_BUFFER_PADDING_SIZE);                       \
        dst = data;                                                     \
    } while (0)

/* Makes duplicates of data, side_data, but does not copy any other fields */
static int copy_packet_data(AVPacket *pkt, const AVPacket *src, int dup)
{
    pkt->data      = NULL;
    pkt->side_data = NULL;
    pkt->side_data_elems = 0;
    if (pkt->buf) {
        AVBufferRef *ref = av_buffer_ref(src->buf);
        if (!ref)
            return AVERROR(ENOMEM);
        pkt->buf  = ref;
        pkt->data = ref->data;
    } else {
        DUP_DATA(pkt->data, src->data, pkt->size, 1, ALLOC_BUF);
    }
    if (src->side_data_elems && dup) {
        pkt->side_data = src->side_data;
        pkt->side_data_elems = src->side_data_elems;
    }
    if (src->side_data_elems && !dup) {
        return av_copy_packet_side_data(pkt, src);
    }
    return 0;

failed_alloc:
    av_packet_unref(pkt);
    return AVERROR(ENOMEM);
}

int av_copy_packet_side_data(AVPacket *pkt, const AVPacket *src)
{
    if (src->side_data_elems) {
        int i;
        DUP_DATA(pkt->side_data, src->side_data,
                src->side_data_elems * sizeof(*src->side_data), 0, ALLOC_MALLOC);
        if (src != pkt) {
            memset(pkt->side_data, 0,
                   src->side_data_elems * sizeof(*src->side_data));
        }
        for (i = 0; i < src->side_data_elems; i++) {
            DUP_DATA(pkt->side_data[i].data, src->side_data[i].data,
                    src->side_data[i].size, 1, ALLOC_MALLOC);
            pkt->side_data[i].size = src->side_data[i].size;
            pkt->side_data[i].type = src->side_data[i].type;
        }
    }
    pkt->side_data_elems = src->side_data_elems;
    return 0;

failed_alloc:
    av_packet_unref(pkt);
    return AVERROR(ENOMEM);
}

int av_dup_packet(AVPacket *pkt)
{
    AVPacket tmp_pkt;

    if (!pkt->buf && pkt->data) {
        tmp_pkt = *pkt;
        return copy_packet_data(pkt, &tmp_pkt, 1);
    }
    return 0;
}

int av_copy_packet(AVPacket *dst, const AVPacket *src)
{
    *dst = *src;
    return copy_packet_data(dst, src, 0);
}
FF_ENABLE_DEPRECATION_WARNINGS
#endif

{

#if FF_API_AVPACKET_OLD_API
FF_DISABLE_DEPRECATION_WARNINGS
void av_free_packet(AVPacket *pkt)
{
    if (pkt) {
        if (pkt->buf)
            av_buffer_unref(&pkt->buf);
        pkt->data            = NULL;
        pkt->size            = 0;

        av_packet_free_side_data(pkt);
    }
}
FF_ENABLE_DEPRECATION_WARNINGS
#endif

                            uint8_t *data, size_t size)
{


            av_free(sd->data);
            sd->data = data;
            sd->size = size;
            return 0;
        }
    }

        return AVERROR(ERANGE);

        return AVERROR(ENOMEM);


}


                                 int size)
{

        return NULL;
        return NULL;

        av_freep(&data);
        return NULL;
    }

}

                                 int *size)
{

        }
    }
    return NULL;
}

{
    case AV_PKT_DATA_PALETTE:                    return "Palette";
    case AV_PKT_DATA_PARAM_CHANGE:               return "Param Change";
    case AV_PKT_DATA_H263_MB_INFO:               return "H263 MB Info";
    case AV_PKT_DATA_REPLAYGAIN:                 return "Replay Gain";
    case AV_PKT_DATA_QUALITY_STATS:              return "Quality stats";
    case AV_PKT_DATA_FALLBACK_TRACK:             return "Fallback track";
    case AV_PKT_DATA_JP_DUALMONO:                return "JP Dual Mono";
    case AV_PKT_DATA_SUBTITLE_POSITION:          return "Subtitle Position";
    case AV_PKT_DATA_MATROSKA_BLOCKADDITIONAL:   return "Matroska BlockAdditional";
    case AV_PKT_DATA_WEBVTT_IDENTIFIER:          return "WebVTT ID";
    case AV_PKT_DATA_WEBVTT_SETTINGS:            return "WebVTT Settings";
    case AV_PKT_DATA_METADATA_UPDATE:            return "Metadata Update";
    case AV_PKT_DATA_MASTERING_DISPLAY_METADATA: return "Mastering display metadata";
    case AV_PKT_DATA_CONTENT_LIGHT_LEVEL:        return "Content light level metadata";
    case AV_PKT_DATA_A53_CC:                     return "A53 Closed Captions";
    case AV_PKT_DATA_ENCRYPTION_INIT_INFO:       return "Encryption initialization data";
    case AV_PKT_DATA_ENCRYPTION_INFO:            return "Encryption info";
    case AV_PKT_DATA_AFD:                        return "Active Format Description data";
    case AV_PKT_DATA_PRFT:                       return "Producer Reference Time";
    case AV_PKT_DATA_ICC_PROFILE:                return "ICC Profile";
    case AV_PKT_DATA_DOVI_CONF:                  return "DOVI configuration record";
    case AV_PKT_DATA_S12M_TIMECODE:              return "SMPTE ST 12-1:2014 timecode";
    }
    return NULL;
}

#if FF_API_MERGE_SD_API

#define FF_MERGE_MARKER 0x8c4d9d108e25e9feULL

int av_packet_merge_side_data(AVPacket *pkt){
    if(pkt->side_data_elems){
        AVBufferRef *buf;
        int i;
        uint8_t *p;
        uint64_t size= pkt->size + 8LL + AV_INPUT_BUFFER_PADDING_SIZE;
        AVPacket old= *pkt;
        for (i=0; i<old.side_data_elems; i++) {
            size += old.side_data[i].size + 5LL;
        }
        if (size > INT_MAX)
            return AVERROR(EINVAL);
        buf = av_buffer_alloc(size);
        if (!buf)
            return AVERROR(ENOMEM);
        pkt->buf = buf;
        pkt->data = p = buf->data;
        pkt->size = size - AV_INPUT_BUFFER_PADDING_SIZE;
        bytestream_put_buffer(&p, old.data, old.size);
        for (i=old.side_data_elems-1; i>=0; i--) {
            bytestream_put_buffer(&p, old.side_data[i].data, old.side_data[i].size);
            bytestream_put_be32(&p, old.side_data[i].size);
            *p++ = old.side_data[i].type | ((i==old.side_data_elems-1)*128);
        }
        bytestream_put_be64(&p, FF_MERGE_MARKER);
        av_assert0(p-pkt->data == pkt->size);
        memset(p, 0, AV_INPUT_BUFFER_PADDING_SIZE);
        av_packet_unref(&old);
        pkt->side_data_elems = 0;
        pkt->side_data = NULL;
        return 1;
    }
    return 0;
}

int av_packet_split_side_data(AVPacket *pkt){
    if (!pkt->side_data_elems && pkt->size >12 && AV_RB64(pkt->data + pkt->size - 8) == FF_MERGE_MARKER){
        int i;
        unsigned int size;
        uint8_t *p;

        p = pkt->data + pkt->size - 8 - 5;
        for (i=1; ; i++){
            size = AV_RB32(p);
            if (size>INT_MAX - 5 || p - pkt->data < size)
                return 0;
            if (p[4]&128)
                break;
            if (p - pkt->data < size + 5)
                return 0;
            p-= size+5;
        }

        if (i > AV_PKT_DATA_NB)
            return AVERROR(ERANGE);

        pkt->side_data = av_malloc_array(i, sizeof(*pkt->side_data));
        if (!pkt->side_data)
            return AVERROR(ENOMEM);

        p= pkt->data + pkt->size - 8 - 5;
        for (i=0; ; i++){
            size= AV_RB32(p);
            av_assert0(size<=INT_MAX - 5 && p - pkt->data >= size);
            pkt->side_data[i].data = av_mallocz(size + AV_INPUT_BUFFER_PADDING_SIZE);
            pkt->side_data[i].size = size;
            pkt->side_data[i].type = p[4]&127;
            if (!pkt->side_data[i].data)
                return AVERROR(ENOMEM);
            memcpy(pkt->side_data[i].data, p-size, size);
            pkt->size -= size + 5;
            if(p[4]&128)
                break;
            p-= size+5;
        }
        pkt->size -= 8;
        pkt->side_data_elems = i+1;
        return 1;
    }
    return 0;
}
#endif

{

        return NULL;


            goto fail;
            goto fail;


    }


fail:
    av_freep(&data);
    *size = 0;
    return NULL;
}

{

        return 0;
        return AVERROR_INVALIDDATA;

            return AVERROR_INVALIDDATA;

            return ret;
    }

    return 0;
}

int av_packet_shrink_side_data(AVPacket *pkt, enum AVPacketSideDataType type,
                               int size)
{
    int i;

    for (i = 0; i < pkt->side_data_elems; i++) {
        if (pkt->side_data[i].type == type) {
            if (size > pkt->side_data[i].size)
                return AVERROR(ENOMEM);
            pkt->side_data[i].size = size;
            return 0;
        }
    }
    return AVERROR(ENOENT);
}

{

#if FF_API_CONVERGENCE_DURATION
FF_DISABLE_DEPRECATION_WARNINGS
FF_ENABLE_DEPRECATION_WARNINGS
#endif


            av_packet_free_side_data(dst);
            return AVERROR(ENOMEM);
        }
    }

    return 0;
}

{

{


        goto fail;

            goto fail;

    } else {
            ret = AVERROR(ENOMEM);
            goto fail;
        }
    }


fail:
    av_packet_unref(dst);
    return ret;
}

{

        return ret;

        av_packet_free(&ret);

}

{

{

        return 0;

        return ret;


}

{

        return 0;

        return ret;


}

{
#if FF_API_CONVERGENCE_DURATION
FF_DISABLE_DEPRECATION_WARNINGS
        pkt->convergence_duration = av_rescale_q(pkt->convergence_duration, src_tb, dst_tb);
FF_ENABLE_DEPRECATION_WARNINGS
#endif

{

                                            side_data_size);
    }

        return AVERROR(ENOMEM);

        AV_WL64(side_data+8 + 8*i , error[i]);

    return 0;
}

int ff_side_data_set_prft(AVPacket *pkt, int64_t timestamp)
{
    AVProducerReferenceTime *prft;
    uint8_t *side_data;
    int side_data_size;

    side_data = av_packet_get_side_data(pkt, AV_PKT_DATA_PRFT, &side_data_size);
    if (!side_data) {
        side_data_size = sizeof(AVProducerReferenceTime);
        side_data = av_packet_new_side_data(pkt, AV_PKT_DATA_PRFT, side_data_size);
    }

    if (!side_data || side_data_size < sizeof(AVProducerReferenceTime))
        return AVERROR(ENOMEM);

    prft = (AVProducerReferenceTime *)side_data;
    prft->wallclock = timestamp;
    prft->flags = 0;

    return 0;
}
