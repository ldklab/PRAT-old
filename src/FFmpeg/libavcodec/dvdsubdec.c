/*
 * DVD subtitle decoding
 * Copyright (c) 2005 Fabrice Bellard
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

#include "avcodec.h"
#include "get_bits.h"
#include "internal.h"

#include "libavutil/attributes.h"
#include "libavutil/colorspace.h"
#include "libavutil/opt.h"
#include "libavutil/imgutils.h"
#include "libavutil/bswap.h"

typedef struct DVDSubContext
{
  AVClass *class;
  uint32_t palette[16];
  char    *palette_str;
  char    *ifo_str;
  int      has_palette;
  uint8_t  colormap[4];
  uint8_t  alpha[256];
  uint8_t  buf[0x10000];
  int      buf_size;
  int      forced_subs_only;
  uint8_t  used_color[256];
#ifdef DEBUG
  int sub_id;
#endif
} DVDSubContext;

static void yuv_a_to_rgba(const uint8_t *ycbcr, const uint8_t *alpha, uint32_t *rgba, int num_values)
{
    const uint8_t *cm = ff_crop_tab + MAX_NEG_CROP;
    uint8_t r, g, b;
    int i, y, cb, cr;
    int r_add, g_add, b_add;

    for (i = num_values; i > 0; i--) {
        y = *ycbcr++;
        cr = *ycbcr++;
        cb = *ycbcr++;
        YUV_TO_RGB1_CCIR(cb, cr);
        YUV_TO_RGB2_CCIR(r, g, b, y);
        *rgba++ = ((unsigned)*alpha++ << 24) | (r << 16) | (g << 8) | b;
    }
}

{

        return INT_MAX;
    }
}

static int decode_run_8bit(GetBitContext *gb, int *color)
{
    int len;
    int has_run = get_bits1(gb);
    *color = get_bits(gb, 2 + 6*get_bits1(gb));
    if (has_run) {
        if (get_bits1(gb)) {
            len = get_bits(gb, 7);
            if (len == 0)
                len = INT_MAX;
            else
                len += 9;
        } else
            len = get_bits(gb, 3) + 2;
    } else
        len = 1;
    return len;
}

                      const uint8_t *buf, int start, int buf_size, int is_8bit)
{

        return -1;

        return -1;


            return -1;
            len = decode_run_8bit(&gb, &color);
        else
            return AVERROR_INVALIDDATA;
                break;
            /* byte align */
        }
    }
    return 0;
}

                          uint32_t *rgba_palette,
                          uint32_t subtitle_color)
{
        // this configuration (full range, lowest to highest) in tests
        // seemed most common, so assume this
        {0xff},
        {0x00, 0xff},
        {0x00, 0x80, 0xff},
        {0x00, 0x55, 0xaa, 0xff},
    };

    }

    for(i = 0; i < 4; i++)
        rgba_palette[i] = 0;

    nb_opaque_colors = 0;
    for(i = 0; i < 4; i++) {
        if (alpha[i] != 0 && !color_used[colormap[i]]) {
            color_used[colormap[i]] = 1;
            nb_opaque_colors++;
        }
    }

    if (nb_opaque_colors == 0)
        return;

    j = 0;
    memset(color_used, 0, 16);
    for(i = 0; i < 4; i++) {
        if (alpha[i] != 0) {
            if (!color_used[colormap[i]])  {
                level = level_map[nb_opaque_colors - 1][j];
                r = (((subtitle_color >> 16) & 0xff) * level) >> 8;
                g = (((subtitle_color >> 8) & 0xff) * level) >> 8;
                b = (((subtitle_color >> 0) & 0xff) * level) >> 8;
                rgba_palette[i] = b | (g << 8) | (r << 16) | ((alpha[i] * 17U) << 24);
                color_used[colormap[i]] = (i + 1);
                j++;
            } else {
                rgba_palette[i] = (rgba_palette[color_used[colormap[i]] - 1] & 0x00ffffff) |
                                    ((alpha[i] * 17U) << 24);
            }
        }
    }
}

{

        for (i = 0; i < sub_header->num_rects; i++) {
            av_freep(&sub_header->rects[i]->data[0]);
            av_freep(&sub_header->rects[i]->data[1]);
            av_freep(&sub_header->rects[i]);
        }
        av_freep(&sub_header->rects);
        sub_header->num_rects = 0;
    }

#define READ_OFFSET(a) (big_offsets ? AV_RB32(a) : AV_RB16(a))

                                const uint8_t *buf, int buf_size)
{

        return -1;

        big_offsets = 1;
        offset_size = 4;
        cmd_pos = 6;
    } else {
    }


            av_log(ctx, AV_LOG_ERROR, "Discarding invalid packet\n");
            return 0;
        }
        return AVERROR(EAGAIN);
    }

                cmd_pos, next_cmd_pos, date);
            case 0x00:
                /* menu subpicture */
                is_menu = 1;
                break;
                /* set start date */
                /* set end date */
                /* set colormap */
                    goto fail;
                /* set alpha */
                    goto fail;
            case 0x85:
                    goto fail;
                    is_8bit = 1;
                    goto fail;
            case 0x86:
                if ((buf_size - pos) < 8)
                    goto fail;
                offset1 = AV_RB32(buf + pos);
                offset2 = AV_RB32(buf + pos + 4);
                ff_dlog(NULL, "offset1=0x%04"PRIx64" offset2=0x%04"PRIx64"\n", offset1, offset2);
                pos += 8;
                break;

            case 0x83:
                /* HD set palette */
                if ((buf_size - pos) < 768)
                    goto fail;
                yuv_palette = buf + pos;
                pos += 768;
                break;
            case 0x84:
                /* HD set contrast (alpha) */
                if ((buf_size - pos) < 256)
                    goto fail;
                for (i = 0; i < 256; i++)
                    alpha[i] = 0xFF - buf[pos+i];
                pos += 256;
                break;

            default:
                ff_dlog(NULL, "unrecognised subpicture command 0x%x\n", cmd);
                goto the_end;
            }
        }
    the_end:
            goto fail;


            /* decode the bitmap */
                w = 0;
                h = 0;
                    goto fail;
                    goto fail;
                    goto fail;
                               buf, offset1, buf_size, is_8bit) < 0)
                    goto fail;
                               buf, offset2, buf_size, is_8bit) < 0)
                    goto fail;
                    goto fail;
                    if (!yuv_palette)
                        goto fail;
                    sub_header->rects[0]->nb_colors = 256;
                    yuv_a_to_rgba(yuv_palette, alpha,
                                  (uint32_t *)sub_header->rects[0]->data[1],
                                  256);
                } else {
                                  0xffff00);
                }

#if FF_API_AVPICTURE
FF_DISABLE_DEPRECATION_WARNINGS
                }
FF_ENABLE_DEPRECATION_WARNINGS
#endif
            }
        }
            av_log(ctx, AV_LOG_ERROR, "Invalid command offset\n");
            break;
        }
            break;
        cmd_pos = next_cmd_pos;
    }
        return is_menu;
 fail:
    reset_rects(sub_header);
    return -1;
}

static int is_transp(const uint8_t *buf, int pitch, int n,
                     const uint8_t *transp_color)
{
    int i;
    for(i = 0; i < n; i++) {
        if (!transp_color[*buf])
            return 0;
        buf += pitch;
    }
    return 1;
}

/* return 0 if empty rectangle, 1 if non empty */
static int find_smallest_bounding_rectangle(DVDSubContext *ctx, AVSubtitle *s)
{
    uint8_t transp_color[256] = { 0 };
    int y1, y2, x1, x2, y, w, h, i;
    uint8_t *bitmap;
    int transparent = 1;

    if (s->num_rects == 0 || !s->rects || s->rects[0]->w <= 0 || s->rects[0]->h <= 0)
        return 0;

    for(i = 0; i < s->rects[0]->nb_colors; i++) {
        if ((((uint32_t *)s->rects[0]->data[1])[i] >> 24) == 0) {
            transp_color[i] = 1;
        } else if (ctx->used_color[i])
            transparent = 0;
    }
    if (transparent)
        return 0;
    y1 = 0;
    while (y1 < s->rects[0]->h && is_transp(s->rects[0]->data[0] + y1 * s->rects[0]->linesize[0],
                                  1, s->rects[0]->w, transp_color))
        y1++;
    if (y1 == s->rects[0]->h) {
        av_freep(&s->rects[0]->data[0]);
        s->rects[0]->w = s->rects[0]->h = 0;
        return 0;
    }

    y2 = s->rects[0]->h - 1;
    while (y2 > 0 && is_transp(s->rects[0]->data[0] + y2 * s->rects[0]->linesize[0], 1,
                               s->rects[0]->w, transp_color))
        y2--;
    x1 = 0;
    while (x1 < (s->rects[0]->w - 1) && is_transp(s->rects[0]->data[0] + x1, s->rects[0]->linesize[0],
                                        s->rects[0]->h, transp_color))
        x1++;
    x2 = s->rects[0]->w - 1;
    while (x2 > 0 && is_transp(s->rects[0]->data[0] + x2, s->rects[0]->linesize[0], s->rects[0]->h,
                                  transp_color))
        x2--;
    w = x2 - x1 + 1;
    h = y2 - y1 + 1;
    bitmap = av_malloc(w * h);
    if (!bitmap)
        return 1;
    for(y = 0; y < h; y++) {
        memcpy(bitmap + w * y, s->rects[0]->data[0] + x1 + (y1 + y) * s->rects[0]->linesize[0], w);
    }
    av_freep(&s->rects[0]->data[0]);
    s->rects[0]->data[0] = bitmap;
    s->rects[0]->linesize[0] = w;
    s->rects[0]->w = w;
    s->rects[0]->h = h;
    s->rects[0]->x += x1;
    s->rects[0]->y += y1;

#if FF_API_AVPICTURE
FF_DISABLE_DEPRECATION_WARNINGS
    for (i = 0; i < 4; i++) {
        s->rects[0]->pict.data[i] = s->rects[0]->data[i];
        s->rects[0]->pict.linesize[i] = s->rects[0]->linesize[i];
    }
FF_ENABLE_DEPRECATION_WARNINGS
#endif

    return 1;
}

#ifdef DEBUG
#define ALPHA_MIX(A,BACK,FORE) (((255-(A)) * (BACK) + (A) * (FORE)) / 255)
static void ppm_save(const char *filename, uint8_t *bitmap, int w, int h,
                     uint32_t *rgba_palette)
{
    int x, y, alpha;
    uint32_t v;
    int back[3] = {0, 255, 0};  /* green background */
    FILE *f;

    f = fopen(filename, "w");
    if (!f) {
        perror(filename);
        return;
    }
    fprintf(f, "P6\n"
            "%d %d\n"
            "%d\n",
            w, h, 255);
    for(y = 0; y < h; y++) {
        for(x = 0; x < w; x++) {
            v = rgba_palette[bitmap[y * w + x]];
            alpha = v >> 24;
            putc(ALPHA_MIX(alpha, back[0], (v >> 16) & 0xff), f);
            putc(ALPHA_MIX(alpha, back[1], (v >> 8) & 0xff), f);
            putc(ALPHA_MIX(alpha, back[2], (v >> 0) & 0xff), f);
        }
    }
    fclose(f);
}
#endif

                                const uint8_t *buf, int buf_size)
{

        av_log(avctx, AV_LOG_WARNING, "Attempt to reconstruct "
               "too large SPU packets aborted.\n");
        ctx->buf_size = 0;
        return AVERROR_INVALIDDATA;
    }
}

                         void *data, int *data_size,
                         AVPacket *avpkt)
{

            *data_size = 0;
            return ret;
        }
    }

    }

        ctx->buf_size = 0;
    no_subtitle:
        reset_rects(sub);
        *data_size = 0;

        return buf_size;
    }
        goto no_subtitle;

        goto no_subtitle;

#if defined(DEBUG)
    {
    char ppm_name[32];

    snprintf(ppm_name, sizeof(ppm_name), "/tmp/%05d.ppm", ctx->sub_id++);
    ff_dlog(NULL, "start=%d ms end =%d ms\n",
            sub->start_display_time,
            sub->end_display_time);
    ppm_save(ppm_name, sub->rects[0]->data[0],
             sub->rects[0]->w, sub->rects[0]->h, (uint32_t*) sub->rects[0]->data[1]);
    }
#endif

}

static int parse_ifo_palette(DVDSubContext *ctx, char *p)
{
    FILE *ifo;
    char ifostr[12];
    uint32_t sp_pgci, pgci, off_pgc, pgc;
    uint8_t r, g, b, yuv[65], *buf;
    int i, y, cb, cr, r_add, g_add, b_add;
    int ret = 0;
    const uint8_t *cm = ff_crop_tab + MAX_NEG_CROP;

    ctx->has_palette = 0;
    if ((ifo = fopen(p, "r")) == NULL) {
        av_log(ctx, AV_LOG_WARNING, "Unable to open IFO file \"%s\": %s\n", p, av_err2str(AVERROR(errno)));
        return AVERROR_EOF;
    }
    if (fread(ifostr, 12, 1, ifo) != 1 || memcmp(ifostr, "DVDVIDEO-VTS", 12)) {
        av_log(ctx, AV_LOG_WARNING, "\"%s\" is not a proper IFO file\n", p);
        ret = AVERROR_INVALIDDATA;
        goto end;
    }
    if (fseek(ifo, 0xCC, SEEK_SET) == -1) {
        ret = AVERROR(errno);
        goto end;
    }
    if (fread(&sp_pgci, 4, 1, ifo) == 1) {
        pgci = av_be2ne32(sp_pgci) * 2048;
        if (fseek(ifo, pgci + 0x0C, SEEK_SET) == -1) {
            ret = AVERROR(errno);
            goto end;
        }
        if (fread(&off_pgc, 4, 1, ifo) == 1) {
            pgc = pgci + av_be2ne32(off_pgc);
            if (fseek(ifo, pgc + 0xA4, SEEK_SET) == -1) {
                ret = AVERROR(errno);
                goto end;
            }
            if (fread(yuv, 64, 1, ifo) == 1) {
                buf = yuv;
                for(i=0; i<16; i++) {
                    y  = *++buf;
                    cr = *++buf;
                    cb = *++buf;
                    YUV_TO_RGB1_CCIR(cb, cr);
                    YUV_TO_RGB2_CCIR(r, g, b, y);
                    ctx->palette[i] = (r << 16) + (g << 8) + b;
                    buf++;
                }
                ctx->has_palette = 1;
            }
        }
    }
    if (ctx->has_palette == 0) {
        av_log(ctx, AV_LOG_WARNING, "Failed to read palette from IFO file \"%s\"\n", p);
        ret = AVERROR_INVALIDDATA;
    }
end:
    fclose(ifo);
    return ret;
}

{

        return 1;

        return AVERROR(ENOMEM);

            break;

                   goto fail;
            }
        }

    }

}

{

        return ret;

        parse_ifo_palette(ctx, ctx->ifo_str);
        ctx->has_palette = 1;
        ff_dvdsub_parse_palette(ctx->palette, ctx->palette_str);
    }
    }

    return 1;
}

{
}

{
}

#define OFFSET(field) offsetof(DVDSubContext, field)
#define SD AV_OPT_FLAG_SUBTITLE_PARAM | AV_OPT_FLAG_DECODING_PARAM
static const AVOption options[] = {
    { "palette", "set the global palette", OFFSET(palette_str), AV_OPT_TYPE_STRING, { .str = NULL }, 0, 0, SD },
    { "ifo_palette", "obtain the global palette from .IFO file", OFFSET(ifo_str), AV_OPT_TYPE_STRING, { .str = NULL }, 0, 0, SD },
    { "forced_subs_only", "Only show forced subtitles", OFFSET(forced_subs_only), AV_OPT_TYPE_BOOL, {.i64 = 0}, 0, 1, SD},
    { NULL }
};
static const AVClass dvdsub_class = {
    .class_name = "dvdsubdec",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
};

AVCodec ff_dvdsub_decoder = {
    .name           = "dvdsub",
    .long_name      = NULL_IF_CONFIG_SMALL("DVD subtitles"),
    .type           = AVMEDIA_TYPE_SUBTITLE,
    .id             = AV_CODEC_ID_DVD_SUBTITLE,
    .priv_data_size = sizeof(DVDSubContext),
    .init           = dvdsub_init,
    .decode         = dvdsub_decode,
    .flush          = dvdsub_flush,
    .close          = dvdsub_close,
    .priv_class     = &dvdsub_class,
};
