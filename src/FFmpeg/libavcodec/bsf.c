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

#include <string.h>

#include "libavutil/avassert.h"
#include "libavutil/log.h"
#include "libavutil/mem.h"
#include "libavutil/opt.h"
#include "libavutil/avstring.h"
#include "libavutil/bprint.h"

#include "bsf.h"
#include "bsf_internal.h"
#include "codec_desc.h"
#include "codec_par.h"

#define IS_EMPTY(pkt) (!(pkt)->data && !(pkt)->side_data_elems)

struct AVBSFInternal {
    AVPacket *buffer_pkt;
    int eof;
};

{

        return;




}

static void *bsf_child_next(void *obj, void *prev)
{
    AVBSFContext *ctx = obj;
    if (!prev && ctx->filter->priv_class)
        return ctx->priv_data;
    return NULL;
}

static const char *bsf_to_name(void *bsf)
{
    return ((AVBSFContext *)bsf)->filter->name;
}

static const AVClass bsf_class = {
    .class_name       = "AVBSFContext",
    .item_name        = bsf_to_name,
    .version          = LIBAVUTIL_VERSION_INT,
    .child_next       = bsf_child_next,
#if FF_API_CHILD_CLASS_NEXT
    .child_class_next = ff_bsf_child_class_next,
#endif
    .child_class_iterate = ff_bsf_child_class_iterate,
    .category         = AV_CLASS_CATEGORY_BITSTREAM_FILTER,
};

const AVClass *av_bsf_get_class(void)
{
    return &bsf_class;
}

{

        return AVERROR(ENOMEM);


        ret = AVERROR(ENOMEM);
        goto fail;
    }

        ret = AVERROR(ENOMEM);
        goto fail;
    }

        ret = AVERROR(ENOMEM);
        goto fail;
    }

    /* allocate priv data and init private options */
            ret = AVERROR(ENOMEM);
            goto fail;
        }
        }
    }

fail:
    av_bsf_free(&ctx);
    return ret;
}

{

    /* check that the codec is supported */
                break;
            const AVCodecDescriptor *desc = avcodec_descriptor_get(ctx->par_in->codec_id);
            av_log(ctx, AV_LOG_ERROR, "Codec '%s' (%d) is not supported by the "
                   "bitstream filter '%s'. Supported codecs are: ",
                   desc ? desc->name : "unknown", ctx->par_in->codec_id, ctx->filter->name);
            for (i = 0; ctx->filter->codec_ids[i] != AV_CODEC_ID_NONE; i++) {
                desc = avcodec_descriptor_get(ctx->filter->codec_ids[i]);
                av_log(ctx, AV_LOG_ERROR, "%s (%d) ",
                       desc ? desc->name : "unknown", ctx->filter->codec_ids[i]);
            }
            av_log(ctx, AV_LOG_ERROR, "\n");
            return AVERROR(EINVAL);
        }
    }

    /* initialize output parameters to be the same as input
     * init below might overwrite that */
        return ret;


            return ret;
    }

    return 0;
}

{




{

    }

        av_log(ctx, AV_LOG_ERROR, "A non-NULL packet sent after an EOF.\n");
        return AVERROR(EINVAL);
    }

        return AVERROR(EAGAIN);

        return ret;

}

{
}

{

        return AVERROR_EOF;

        return AVERROR(EAGAIN);

        return AVERROR(ENOMEM);


}

{

        return AVERROR_EOF;

        return AVERROR(EAGAIN);


}

typedef struct BSFListContext {
    const AVClass *class;

    AVBSFContext **bsfs;
    int nb_bsfs;

    unsigned idx;           // index of currently processed BSF

    char * item_name;
} BSFListContext;


{

        ret = avcodec_parameters_copy(lst->bsfs[i]->par_in, cod_par);
        if (ret < 0)
            goto fail;

        lst->bsfs[i]->time_base_in = tb;

        ret = av_bsf_init(lst->bsfs[i]);
        if (ret < 0)
            goto fail;

        cod_par = lst->bsfs[i]->par_out;
        tb = lst->bsfs[i]->time_base_out;
    }


}

{


    while (1) {
        /* get a packet from the previous filter up the chain */
        if (lst->idx)
            ret = av_bsf_receive_packet(lst->bsfs[lst->idx-1], out);
        else
            ret = ff_bsf_get_packet_ref(bsf, out);
        if (ret == AVERROR(EAGAIN)) {
            if (!lst->idx)
                return ret;
            lst->idx--;
            continue;
        } else if (ret == AVERROR_EOF) {
            eof = 1;
        } else if (ret < 0)
            return ret;

        /* send it to the next filter down the chain */
        if (lst->idx < lst->nb_bsfs) {
            ret = av_bsf_send_packet(lst->bsfs[lst->idx], eof ? NULL : out);
            av_assert1(ret != AVERROR(EAGAIN));
            if (ret < 0) {
                av_packet_unref(out);
                return ret;
            }
            lst->idx++;
            eof = 0;
        } else if (eof) {
            return ret;
        } else {
            return 0;
        }
    }
}

{

        av_bsf_flush(lst->bsfs[i]);

{

        av_bsf_free(&lst->bsfs[i]);

static const char *bsf_list_item_name(void *ctx)
{
    static const char *null_filter_name = "null";
    AVBSFContext *bsf_ctx = ctx;
    BSFListContext *lst = bsf_ctx->priv_data;

    if (!lst->nb_bsfs)
        return null_filter_name;

    if (!lst->item_name) {
        int i;
        AVBPrint bp;
        av_bprint_init(&bp, 16, 128);

        av_bprintf(&bp, "bsf_list(");
        for (i = 0; i < lst->nb_bsfs; i++)
            av_bprintf(&bp, i ? ",%s" : "%s", lst->bsfs[i]->filter->name);
        av_bprintf(&bp, ")");

        av_bprint_finalize(&bp, &lst->item_name);
    }

    return lst->item_name;
}

static const AVClass bsf_list_class = {
        .class_name = "bsf_list",
        .item_name  = bsf_list_item_name,
        .version    = LIBAVUTIL_VERSION_INT,
};

const AVBitStreamFilter ff_list_bsf = {
        .name           = "bsf_list",
        .priv_data_size = sizeof(BSFListContext),
        .priv_class     = &bsf_list_class,
        .init           = bsf_list_init,
        .filter         = bsf_list_filter,
        .flush          = bsf_list_flush,
        .close          = bsf_list_close,
};

struct AVBSFList {
    AVBSFContext **bsfs;
    int nb_bsfs;
};

{
}

void av_bsf_list_free(AVBSFList **lst)
{
    int i;

    if (!*lst)
        return;

    for (i = 0; i < (*lst)->nb_bsfs; ++i)
        av_bsf_free(&(*lst)->bsfs[i]);
    av_free((*lst)->bsfs);
    av_freep(lst);
}

{
}

{

        return AVERROR_BSF_NOT_FOUND;

        return ret;



            goto end;
    }

        ret = av_opt_set_dict2(bsf, options_dict, AV_OPT_SEARCH_CHILDREN);
        if (ret < 0)
            goto end;
    }


        av_bsf_free(&bsf);

    return ret;
}

int av_bsf_list_append2(AVBSFList *lst, const char *bsf_name, AVDictionary ** options)
{
    return bsf_list_append_internal(lst, bsf_name, NULL, options);
}

{

    }

    ret = av_bsf_alloc(&ff_list_bsf, bsf);
    if (ret < 0)
        return ret;

    ctx = (*bsf)->priv_data;

    ctx->bsfs = (*lst)->bsfs;
    ctx->nb_bsfs = (*lst)->nb_bsfs;

}

{

        return AVERROR(EINVAL);

}

{


        return AVERROR(ENOMEM);

        ret = AVERROR(ENOMEM);
        goto end;
    }

            goto end;

        buf = NULL;
    }

        av_bsf_list_free(&lst);
}

{
}
