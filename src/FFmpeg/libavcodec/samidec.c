/*
 * Copyright (c) 2012 Clément Bœsch
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
 * SAMI subtitle decoder
 * @see http://msdn.microsoft.com/en-us/library/ms971327.aspx
 */

#include "ass.h"
#include "libavutil/avstring.h"
#include "libavutil/bprint.h"
#include "htmlsubtitles.h"

typedef struct {
    AVBPrint source;
    AVBPrint content;
    AVBPrint encoded_source;
    AVBPrint encoded_content;
    AVBPrint full;
    int readorder;

    char *tag = NULL;
    char *p = dupsrc;
    AVBPrint *dst_content = &sami->encoded_content;

        char *saveptr = NULL;
        AVBPrint *dst = &sami->content;
        if (!p)
            p++;
        tag = av_strtok(p, ">", &saveptr);
            break;
        p = saveptr;
            av_bprint_clear(dst);
        }
        while (av_isspace(*p))
            p++;
            goto end;
        /* extract the text, stripping most of the tags */
        while (*p) {
            if (*p == '<') {
                p++;
                while (*p && *p != '>')
                if (!*p)
                    break;
                continue;
            }
                av_bprint_chars(dst, *p, 1);
            prev_chr_is_space = av_isspace(*p);
    }

    if (sami->source.len) {
            goto end;
        av_bprintf(&sami->full, "{\\i1}%s{\\i0}\\N", sami->encoded_source.str);
    }
    ret = ff_htmlmarkup_to_ass(avctx, dst_content, sami->content.str);
    if (ret < 0)
        goto end;
    av_bprintf(&sami->full, "%s", sami->encoded_content.str);

end:
    av_free(dupsrc);
}
{
    AVSubtitle *sub = data;
    const char *ptr = avpkt->data;
    SAMIContext *sami = avctx->priv_data;

    if (ptr && avpkt->size > 0) {
        int ret = sami_paragraph_to_ass(avctx, ptr);
        if (ret < 0)
            return ret;
        // TODO: pass escaped sami->encoded_source.str as source
        ret = ff_ass_add_rect(sub, sami->full.str, sami->readorder++, 0, NULL, NULL);
        if (ret < 0)
            return ret;
    }
    *got_sub_ptr = sub->num_rects > 0;
    return avpkt->size;
}

static av_cold int sami_init(AVCodecContext *avctx)
{
    SAMIContext *sami = avctx->priv_data;
    av_bprint_init(&sami->source,  0, 2048);
    av_bprint_init(&sami->content, 0, 2048);
    av_bprint_init(&sami->encoded_source,  0, 2048);
    av_bprint_init(&sami->encoded_content, 0, 2048);
    av_bprint_init(&sami->full,    0, 2048);
    return ff_ass_subtitle_header_default(avctx);
}

static av_cold int sami_close(AVCodecContext *avctx)
{
    SAMIContext *sami = avctx->priv_data;
    av_bprint_finalize(&sami->source,  NULL);
    av_bprint_finalize(&sami->content, NULL);
    av_bprint_finalize(&sami->encoded_source,  NULL);
    av_bprint_finalize(&sami->encoded_content, NULL);
    av_bprint_finalize(&sami->full,    NULL);
    return 0;
}

static void sami_flush(AVCodecContext *avctx)
{
    SAMIContext *sami = avctx->priv_data;
    if (!(avctx->flags2 & AV_CODEC_FLAG2_RO_FLUSH_NOOP))
        sami->readorder = 0;
}

AVCodec ff_sami_decoder = {
    .name           = "sami",
    .long_name      = NULL_IF_CONFIG_SMALL("SAMI subtitle"),
    .type           = AVMEDIA_TYPE_SUBTITLE,
    .id             = AV_CODEC_ID_SAMI,
    .priv_data_size = sizeof(SAMIContext),
    .init           = sami_init,
    .close          = sami_close,
    .decode         = sami_decode_frame,
    .flush          = sami_flush,
};
