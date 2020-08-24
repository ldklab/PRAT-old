/*
 * Copyright (c) 2015-2016 Kieran Kunhya <kieran@kunhya.com>
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
 * Cineform HD video decoder
 */

#include "libavutil/attributes.h"
#include "libavutil/buffer.h"
#include "libavutil/common.h"
#include "libavutil/imgutils.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/opt.h"

#include "avcodec.h"
#include "bytestream.h"
#include "get_bits.h"
#include "internal.h"
#include "thread.h"
#include "cfhd.h"

#define ALPHA_COMPAND_DC_OFFSET 256
#define ALPHA_COMPAND_GAIN 9400

{



            }

        }

    }


}

{

{
}

{

{
    } else
        return level * quantisation;
}

static inline void difference_coding(int16_t *band, int width, int height)
{

    int i,j;
    for (i = 0; i < height; i++) {
        for (j = 1; j < width; j++) {
          band[j] += band[j-1];
        }
        band += width;
    }
}

static inline void peak_table(int16_t *band, Peak *peak, int length)
{
    int i;
    for (i = 0; i < length; i++)
        if (abs(band[i]) > peak->level)
            band[i] = bytestream2_get_le16(&peak->base);
}

static inline void process_alpha(int16_t *alpha, int width)
{
    int i, channel;
    for (i = 0; i < width; i++) {
        channel   = alpha[i];
        channel  -= ALPHA_COMPAND_DC_OFFSET;
        channel <<= 3;
        channel  *= ALPHA_COMPAND_GAIN;
        channel >>= 16;
        channel   = av_clip_uintp2(channel, 12);
        alpha[i]  = channel;
    }
}

static inline void process_bayer(AVFrame *frame, int bpc)
{
    const int linesize = frame->linesize[0];
    uint16_t *r = (uint16_t *)frame->data[0];
    uint16_t *g1 = (uint16_t *)(frame->data[0] + 2);
    uint16_t *g2 = (uint16_t *)(frame->data[0] + frame->linesize[0]);
    uint16_t *b = (uint16_t *)(frame->data[0] + frame->linesize[0] + 2);
    const int mid = 1 << (bpc - 1);
    const int factor = 1 << (16 - bpc);

    for (int y = 0; y < frame->height >> 1; y++) {
        for (int x = 0; x < frame->width; x += 2) {
            int R, G1, G2, B;
            int g, rg, bg, gd;

            g  = r[x];
            rg = g1[x];
            bg = g2[x];
            gd = b[x];
            gd -= mid;

            R  = (rg - mid) * 2 + g;
            G1 = g + gd;
            G2 = g - gd;
            B  = (bg - mid) * 2 + g;

            R  = av_clip_uintp2(R  * factor, 16);
            G1 = av_clip_uintp2(G1 * factor, 16);
            G2 = av_clip_uintp2(G2 * factor, 16);
            B  = av_clip_uintp2(B  * factor, 16);

            r[x]  = R;
            g1[x] = G1;
            g2[x] = G2;
            b[x]  = B;
        }

        r  += linesize;
        g1 += linesize;
        g2 += linesize;
        b  += linesize;
    }
}

                          int16_t *low, ptrdiff_t low_stride,
                          int16_t *high, ptrdiff_t high_stride,
                          int len, int clip)
{




    }



static inline void interlaced_vertical_filter(int16_t *output, int16_t *low, int16_t *high,
                         int width, int linesize, int plane)
{
    int i;
    int16_t even, odd;
    for (i = 0; i < width; i++) {
        even = (low[i] - high[i])/2;
        odd  = (low[i] + high[i])/2;
        output[i]            = av_clip_uintp2(even, 10);
        output[i + linesize] = av_clip_uintp2(odd, 10);
    }
}

static inline void inverse_temporal_filter(int16_t *output, int16_t *low, int16_t *high,
                                           int width)
{
    for (int i = 0; i < width; i++) {
        int even = (low[i] - high[i]) / 2;
        int odd  = (low[i] + high[i]) / 2;

        low[i]  = even;
        high[i] = odd;
    }
}

                         int width)
{
}

                              int width, int clip)
{

static void horiz_filter_clip_bayer(int16_t *output, int16_t *low, int16_t *high,
                                    int width, int clip)
{
    filter(output, 2, low, 1, high, 1, width, clip);
}

                        int16_t *low, ptrdiff_t low_stride,
                        int16_t *high, ptrdiff_t high_stride, int len)
{
}

{



    }

{

        return ret;

                                                &chroma_x_shift,
                                                &chroma_y_shift)) < 0)
        return ret;
        planes = 4;
        chroma_x_shift = 1;
        chroma_y_shift = 1;
        bayer = 1;
    }


            height = FFALIGN(height / 8, 2) * 8;


        } else {
            s->plane[i].idwt_size = FFALIGN(height, 8) * stride * 2;
            s->plane[i].idwt_buf =
                av_mallocz_array(s->plane[i].idwt_size, sizeof(*s->plane[i].idwt_buf));
            s->plane[i].idwt_tmp =
                av_malloc_array(s->plane[i].idwt_size, sizeof(*s->plane[i].idwt_tmp));
        }

            return AVERROR(ENOMEM);

        } else {
            int16_t *frame2 =
            s->plane[i].subband[7]  = s->plane[i].idwt_buf + 4 * w2 * h2;
            s->plane[i].subband[8]  = frame2 + 2 * w4 * h4;
            s->plane[i].subband[9]  = frame2 + 1 * w4 * h4;
            s->plane[i].subband[10] = frame2 + 3 * w4 * h4;
            s->plane[i].subband[11] = frame2 + 2 * w2 * h2;
            s->plane[i].subband[12] = frame2 + 1 * w2 * h2;
            s->plane[i].subband[13] = frame2 + 3 * w2 * h2;
            s->plane[i].subband[14] = s->plane[i].idwt_buf + 2 * w2 * h2;
            s->plane[i].subband[15] = s->plane[i].idwt_buf + 1 * w2 * h2;
            s->plane[i].subband[16] = s->plane[i].idwt_buf + 3 * w2 * h2;
        }

                }
            }
        } else {
            for (j = 0; j < DWT_LEVELS_3D; j++) {
                int t = j < 1 ? 0 : (j < 3 ? 1 : 2);

                for (k = 0; k < FF_ARRAY_ELEMS(s->plane[i].band[j]); k++) {
                    s->plane[i].band[j][k].a_width  = w8 << t;
                    s->plane[i].band[j][k].a_height = h8 << t;
                }
            }
        }

        /* ll2 and ll1 commented out because they are done in-place */
        // s->plane[i].l_h[2] = ll2;
        // s->plane[i].l_h[5] = ll1;
            int16_t *frame2 = s->plane[i].idwt_tmp + 4 * w2 * h2;

            s->plane[i].l_h[8] = frame2;
            s->plane[i].l_h[9] = frame2 + 2 * w2 * h2;
        }
    }


}

                       AVPacket *avpkt)
{



        /* Bit weird but implement the tag parsing as the spec says */
            av_log(avctx, AV_LOG_DEBUG, "large len %x\n", ((tagu & 0xff) << 16) | data);
            s->frame_type = data;
            av_log(avctx, AV_LOG_DEBUG, "Frame type %"PRIu16"\n", data);
            av_log(avctx, AV_LOG_DEBUG, "Version major %"PRIu16"\n", data);
            av_log(avctx, AV_LOG_DEBUG, "Version minor %"PRIu16"\n", data);
            av_log(avctx, AV_LOG_DEBUG, "Version revision %"PRIu16"\n", data);
            av_log(avctx, AV_LOG_DEBUG, "Version edit %"PRIu16"\n", data);
                av_log(avctx, AV_LOG_ERROR, "Channel Count of %"PRIu16" is unsupported\n", data);
                ret = AVERROR_PATCHWELCOME;
                break;
            }
                av_log(avctx, AV_LOG_ERROR, "Subband Count of %"PRIu16" is unsupported\n", data);
                ret = AVERROR_PATCHWELCOME;
                break;
            }
                av_log(avctx, AV_LOG_ERROR, "Invalid channel number\n");
                ret = AVERROR(EINVAL);
                break;
            }
                (s->transform_type == 2 && s->level >= DWT_LEVELS_3D)) {
                av_log(avctx, AV_LOG_ERROR, "Invalid level\n");
                ret = AVERROR(EINVAL);
                break;
            }
                av_log(avctx, AV_LOG_ERROR, "Invalid subband number\n");
                ret = AVERROR(EINVAL);
                break;
            }
                (s->transform_type == 2 && s->subband_num_actual >= SUBBAND_COUNT_3D && s->subband_num_actual != 255)) {
                av_log(avctx, AV_LOG_ERROR, "Invalid subband number actual\n");
                ret = AVERROR(EINVAL);
                break;
            }
                av_log(avctx, AV_LOG_ERROR, "Invalid band encoding\n");
                ret = AVERROR(EINVAL);
                break;
            }
                av_log(avctx, AV_LOG_ERROR, "Invalid transform type\n");
                ret = AVERROR(EINVAL);
                break;
            }
                s->peak.level = 0;
            av_log(avctx, AV_LOG_DEBUG, "Frame index %"PRIu16"\n", data);
            s->frame_index = data;
                av_log(avctx, AV_LOG_ERROR, "too many values (%d)\n", data);
                ret = AVERROR_INVALIDDATA;
                break;
            }
            }
                av_log(avctx, AV_LOG_ERROR, "Invalid highpass width\n");
                ret = AVERROR(EINVAL);
                break;
            }
                av_log(avctx, AV_LOG_ERROR, "Invalid highpass height\n");
                ret = AVERROR(EINVAL);
                break;
            }
                av_log(avctx, AV_LOG_ERROR, "Invalid highpass width2\n");
                ret = AVERROR(EINVAL);
                break;
            }
                av_log(avctx, AV_LOG_ERROR, "Invalid highpass height2\n");
                ret = AVERROR(EINVAL);
                break;
            }
            av_log(avctx, AV_LOG_DEBUG, "Input format %i\n", data);
            if (s->coded_format == AV_PIX_FMT_NONE ||
                s->coded_format == AV_PIX_FMT_YUV422P10) {
                if (data >= 100 && data <= 105) {
                    s->coded_format = AV_PIX_FMT_BAYER_RGGB16;
                } else if (data >= 122 && data <= 128) {
                    s->coded_format = AV_PIX_FMT_GBRP12;
                } else if (data == 30) {
                    s->coded_format = AV_PIX_FMT_GBRAP12;
                } else {
                    s->coded_format = AV_PIX_FMT_YUV422P10;
                }
                s->planes = s->coded_format == AV_PIX_FMT_BAYER_RGGB16 ? 4 : av_pix_fmt_count_planes(s->coded_format);
            }
                av_log(avctx, AV_LOG_ERROR, "Invalid bits per channel\n");
                ret = AVERROR(EINVAL);
                break;
            }
                s->coded_format = AV_PIX_FMT_BAYER_RGGB16;
            } else if (data == 4) {
                s->coded_format = AV_PIX_FMT_GBRAP12;
            } else {
                avpriv_report_missing_feature(avctx, "Sample format of %"PRIu16, data);
                ret = AVERROR_PATCHWELCOME;
                break;
            }
            s->peak.offset &= ~0xffff;
            s->peak.offset |= (data & 0xffff);
            s->peak.base    = gb;
            s->peak.level   = 0;
            s->peak.offset &= 0xffff;
            s->peak.offset |= (data & 0xffffU)<<16;
            s->peak.base    = gb;
            s->peak.level   = 0;
            s->peak.level = data;
            bytestream2_seek(&s->peak.base, s->peak.offset - 4, SEEK_CUR);
        } else


            }

            }

                s->coded_width = lowpass_width * factor * 8;
            }

                s->coded_height = lowpass_height * factor * 8;
            }

                s->coded_width = s->a_width;
                s->coded_height = s->a_height;

                    free_buffers(s);
                    return ret;
                }
            }
                return ret;
                    return AVERROR_INVALIDDATA;
            }

                return ret;

            frame.f->width =
            frame.f->height = 0;

            if ((ret = ff_thread_get_buffer(avctx, &frame, 0)) < 0)
                return ret;
            s->coded_width = 0;
            s->coded_height = 0;
            s->coded_format = AV_PIX_FMT_NONE;
            got_buffer = 1;
        }

            goto finish;

        /* Lowpass coefficients */

                av_log(avctx, AV_LOG_ERROR, "Invalid lowpass width\n");
                ret = AVERROR(EINVAL);
                goto end;
            }

                av_log(avctx, AV_LOG_ERROR, "Invalid lowpass height\n");
                ret = AVERROR(EINVAL);
                goto end;
            }

                av_log(avctx, AV_LOG_ERROR, "No end of header tag found\n");
                ret = AVERROR(EINVAL);
                goto end;
            }

                av_log(avctx, AV_LOG_ERROR, "Too many lowpass coefficients\n");
                ret = AVERROR(EINVAL);
                goto end;
            }


            }

            /* Align to mod-4 position to continue reading tags */

            /* Copy last line of coefficients if odd height */
                       lowpass_width * sizeof(*coeff_data));
            }

        }


                av_log(avctx, AV_LOG_ERROR, "No end of header tag found\n");
                ret = AVERROR(EINVAL);
                goto end;
            }

                av_log(avctx, AV_LOG_ERROR, "Too many highpass coefficients\n");
                ret = AVERROR(EINVAL);
                goto end;
            }


                goto end;
            {


                    s->codebook = 1;
                    while (1) {
                        UPDATE_CACHE(re, &s->gb);
                        GET_RL_VLC(level, run, re, &s->gb, s->table_9_rl_vlc,
                                   VLC_BITS, 3, 1);

                        /* escape */
                        if (level == 64)
                            break;

                        count += run;

                        if (count > expected)
                            break;

                        if (!lossless)
                            coeff = dequant_and_decompand(s, level, s->quantisation, 0);
                        else
                            coeff = level;
                        if (tag == BandSecondPass) {
                            const uint16_t q = s->quantisation;

                            for (i = 0; i < run; i++) {
                                *coeff_data |= coeff << 8;
                                *coeff_data++ *= q;
                            }
                        } else {
                            for (i = 0; i < run; i++)
                                *coeff_data++ = coeff;
                        }
                    }
                } else {
                                   VLC_BITS, 3, 1);

                        /* escape */
                            break;


                            break;

                        else
                            coeff = level;
                            const uint16_t q = s->quantisation;

                            for (i = 0; i < run; i++) {
                                *coeff_data |= coeff << 8;
                                *coeff_data++ *= q;
                            }
                        } else {
                        }
                    }
                }
            }

                av_log(avctx, AV_LOG_ERROR, "Escape codeword not found, probably corrupt data\n");
                ret = AVERROR(EINVAL);
                goto end;
            }
                peak_table(coeff_data - count, &s->peak, count);
                difference_coding(s->plane[s->channel_num].subband[s->subband_num_actual], highpass_width, highpass_height);

                av_log(avctx, AV_LOG_ERROR, "Bitstream overread error\n");
                ret = AVERROR(EINVAL);
                goto end;
            } else


            /* Copy last line of coefficients if odd height */
                       highpass_stride * sizeof(*coeff_data));
            }
        }
    }

        s->progressive = 1;
        s->planes = 4;
    }


        av_log(avctx, AV_LOG_ERROR, "Invalid dimensions\n");
        ret = AVERROR(EINVAL);
        goto end;
    }

        av_log(avctx, AV_LOG_ERROR, "No end of header tag found\n");
        ret = AVERROR(EINVAL);
        goto end;
    }

            /* level 1 */

                act_plane = 0;
                dst_linesize = pic->linesize[act_plane];
            } else {
            }

                av_log(avctx, AV_LOG_ERROR, "Invalid plane dimensions\n");
                ret = AVERROR(EINVAL);
                goto end;
            }


            }


                // note the stride of "low" is highpass_stride
            }

            }

                }
            }

            /* level 2 */

                !highpass_stride || s->plane[plane].band[1][1].width > s->plane[plane].band[1][1].a_width) {
                av_log(avctx, AV_LOG_ERROR, "Invalid plane dimensions\n");
                ret = AVERROR(EINVAL);
                goto end;
            }


            }

            }

            }


            }

            /* level 3 */

                !highpass_stride || s->plane[plane].band[2][1].width > s->plane[plane].band[2][1].a_width) {
                av_log(avctx, AV_LOG_ERROR, "Invalid plane dimensions\n");
                ret = AVERROR(EINVAL);
                goto end;
            }

                }

                }

                    if (plane & 1)
                        dst++;
                    if (plane > 1)
                        dst += pic->linesize[act_plane] >> 1;
                }

                    (lowpass_height * 2 > avctx->coded_height / 2 ||
                     lowpass_width  * 2 > avctx->coded_width  / 2    )
                    ) {
                    ret = AVERROR_INVALIDDATA;
                    goto end;
                }

                        horiz_filter_clip_bayer(dst, low, high, lowpass_width, s->bpc);
                    else
                        process_alpha(dst, lowpass_width * 2);
                }
            } else {
                av_log(avctx, AV_LOG_DEBUG, "interlaced frame ? %d", pic->interlaced_frame);
                pic->interlaced_frame = 1;
                low    = s->plane[plane].subband[0];
                high   = s->plane[plane].subband[7];
                output = s->plane[plane].l_h[6];
                for (i = 0; i < lowpass_height; i++) {
                    horiz_filter(output, low, high, lowpass_width);
                    low    += lowpass_width;
                    high   += lowpass_width;
                    output += lowpass_width * 2;
                }

                low    = s->plane[plane].subband[8];
                high   = s->plane[plane].subband[9];
                output = s->plane[plane].l_h[7];
                for (i = 0; i < lowpass_height; i++) {
                    horiz_filter(output, low, high, lowpass_width);
                    low    += lowpass_width;
                    high   += lowpass_width;
                    output += lowpass_width * 2;
                }

                dst  = (int16_t *)pic->data[act_plane];
                low  = s->plane[plane].l_h[6];
                high = s->plane[plane].l_h[7];
                for (i = 0; i < lowpass_height; i++) {
                    interlaced_vertical_filter(dst, low, high, lowpass_width * 2,  pic->linesize[act_plane]/2, act_plane);
                    low  += lowpass_width * 2;
                    high += lowpass_width * 2;
                    dst  += pic->linesize[act_plane];
                }
            }
        }
    } else if (s->transform_type == 2 && (avctx->internal->is_copy || s->frame_index == 1 || s->sample_type != 1)) {
        for (plane = 0; plane < s->planes && !ret; plane++) {
            int lowpass_height  = s->plane[plane].band[0][0].height;
            int lowpass_width   = s->plane[plane].band[0][0].width;
            int highpass_stride = s->plane[plane].band[0][1].stride;
            int act_plane = plane == 1 ? 2 : plane == 2 ? 1 : plane;
            int16_t *low, *high, *output, *dst;
            ptrdiff_t dst_linesize;

            if (avctx->pix_fmt == AV_PIX_FMT_BAYER_RGGB16) {
                act_plane = 0;
                dst_linesize = pic->linesize[act_plane];
            } else {
                dst_linesize = pic->linesize[act_plane] / 2;
            }

            if (lowpass_height > s->plane[plane].band[0][0].a_height || lowpass_width > s->plane[plane].band[0][0].a_width ||
                !highpass_stride || s->plane[plane].band[0][1].width > s->plane[plane].band[0][1].a_width) {
                av_log(avctx, AV_LOG_ERROR, "Invalid plane dimensions\n");
                ret = AVERROR(EINVAL);
                goto end;
            }

            av_log(avctx, AV_LOG_DEBUG, "Decoding level 1 plane %i %i %i %i\n", plane, lowpass_height, lowpass_width, highpass_stride);

            low    = s->plane[plane].subband[0];
            high   = s->plane[plane].subband[2];
            output = s->plane[plane].l_h[0];
            for (i = 0; i < lowpass_width; i++) {
                vert_filter(output, lowpass_width, low, lowpass_width, high, highpass_stride, lowpass_height);
                low++;
                high++;
                output++;
            }

            low    = s->plane[plane].subband[1];
            high   = s->plane[plane].subband[3];
            output = s->plane[plane].l_h[1];
            for (i = 0; i < lowpass_width; i++) {
                vert_filter(output, lowpass_width, low, highpass_stride, high, highpass_stride, lowpass_height);
                low++;
                high++;
                output++;
            }

            low    = s->plane[plane].l_h[0];
            high   = s->plane[plane].l_h[1];
            output = s->plane[plane].l_h[7];
            for (i = 0; i < lowpass_height * 2; i++) {
                horiz_filter(output, low, high, lowpass_width);
                low    += lowpass_width;
                high   += lowpass_width;
                output += lowpass_width * 2;
            }
            if (s->bpc == 12) {
                output = s->plane[plane].l_h[7];
                for (i = 0; i < lowpass_height * 2; i++) {
                    for (j = 0; j < lowpass_width * 2; j++)
                        output[j] *= 4;

                    output += lowpass_width * 2;
                }
            }

            lowpass_height  = s->plane[plane].band[1][1].height;
            lowpass_width   = s->plane[plane].band[1][1].width;
            highpass_stride = s->plane[plane].band[1][1].stride;

            if (lowpass_height > s->plane[plane].band[1][1].a_height || lowpass_width > s->plane[plane].band[1][1].a_width ||
                !highpass_stride || s->plane[plane].band[1][1].width > s->plane[plane].band[1][1].a_width) {
                av_log(avctx, AV_LOG_ERROR, "Invalid plane dimensions\n");
                ret = AVERROR(EINVAL);
                goto end;
            }

            av_log(avctx, AV_LOG_DEBUG, "Level 2 lowpass plane %i %i %i %i\n", plane, lowpass_height, lowpass_width, highpass_stride);

            low    = s->plane[plane].l_h[7];
            high   = s->plane[plane].subband[5];
            output = s->plane[plane].l_h[3];
            for (i = 0; i < lowpass_width; i++) {
                vert_filter(output, lowpass_width, low, lowpass_width, high, highpass_stride, lowpass_height);
                low++;
                high++;
                output++;
            }

            low    = s->plane[plane].subband[4];
            high   = s->plane[plane].subband[6];
            output = s->plane[plane].l_h[4];
            for (i = 0; i < lowpass_width; i++) {
                vert_filter(output, lowpass_width, low, highpass_stride, high, highpass_stride, lowpass_height);
                low++;
                high++;
                output++;
            }

            low    = s->plane[plane].l_h[3];
            high   = s->plane[plane].l_h[4];
            output = s->plane[plane].l_h[7];
            for (i = 0; i < lowpass_height * 2; i++) {
                horiz_filter(output, low, high, lowpass_width);
                low    += lowpass_width;
                high   += lowpass_width;
                output += lowpass_width * 2;
            }

            output = s->plane[plane].l_h[7];
            for (i = 0; i < lowpass_height * 2; i++) {
                for (j = 0; j < lowpass_width * 2; j++)
                    output[j] *= 4;
                output += lowpass_width * 2;
            }

            low    = s->plane[plane].subband[7];
            high   = s->plane[plane].subband[9];
            output = s->plane[plane].l_h[3];
            for (i = 0; i < lowpass_width; i++) {
                vert_filter(output, lowpass_width, low, lowpass_width, high, highpass_stride, lowpass_height);
                low++;
                high++;
                output++;
            }

            low    = s->plane[plane].subband[8];
            high   = s->plane[plane].subband[10];
            output = s->plane[plane].l_h[4];
            for (i = 0; i < lowpass_width; i++) {
                vert_filter(output, lowpass_width, low, highpass_stride, high, highpass_stride, lowpass_height);
                low++;
                high++;
                output++;
            }

            low    = s->plane[plane].l_h[3];
            high   = s->plane[plane].l_h[4];
            output = s->plane[plane].l_h[9];
            for (i = 0; i < lowpass_height * 2; i++) {
                horiz_filter(output, low, high, lowpass_width);
                low    += lowpass_width;
                high   += lowpass_width;
                output += lowpass_width * 2;
            }

            lowpass_height  = s->plane[plane].band[4][1].height;
            lowpass_width   = s->plane[plane].band[4][1].width;
            highpass_stride = s->plane[plane].band[4][1].stride;
            av_log(avctx, AV_LOG_DEBUG, "temporal level %i %i %i %i\n", plane, lowpass_height, lowpass_width, highpass_stride);

            if (lowpass_height > s->plane[plane].band[4][1].a_height || lowpass_width > s->plane[plane].band[4][1].a_width ||
                !highpass_stride || s->plane[plane].band[4][1].width > s->plane[plane].band[4][1].a_width) {
                av_log(avctx, AV_LOG_ERROR, "Invalid plane dimensions\n");
                ret = AVERROR(EINVAL);
                goto end;
            }

            low    = s->plane[plane].l_h[7];
            high   = s->plane[plane].l_h[9];
            output = s->plane[plane].l_h[7];
            for (i = 0; i < lowpass_height; i++) {
                inverse_temporal_filter(output, low, high, lowpass_width);
                low    += lowpass_width;
                high   += lowpass_width;
            }
            if (s->progressive) {
                low    = s->plane[plane].l_h[7];
                high   = s->plane[plane].subband[15];
                output = s->plane[plane].l_h[6];
                for (i = 0; i < lowpass_width; i++) {
                    vert_filter(output, lowpass_width, low, lowpass_width, high, highpass_stride, lowpass_height);
                    low++;
                    high++;
                    output++;
                }

                low    = s->plane[plane].subband[14];
                high   = s->plane[plane].subband[16];
                output = s->plane[plane].l_h[7];
                for (i = 0; i < lowpass_width; i++) {
                    vert_filter(output, lowpass_width, low, highpass_stride, high, highpass_stride, lowpass_height);
                    low++;
                    high++;
                    output++;
                }

                low    = s->plane[plane].l_h[9];
                high   = s->plane[plane].subband[12];
                output = s->plane[plane].l_h[8];
                for (i = 0; i < lowpass_width; i++) {
                    vert_filter(output, lowpass_width, low, lowpass_width, high, highpass_stride, lowpass_height);
                    low++;
                    high++;
                    output++;
                }

                low    = s->plane[plane].subband[11];
                high   = s->plane[plane].subband[13];
                output = s->plane[plane].l_h[9];
                for (i = 0; i < lowpass_width; i++) {
                    vert_filter(output, lowpass_width, low, highpass_stride, high, highpass_stride, lowpass_height);
                    low++;
                    high++;
                    output++;
                }

                if (s->sample_type == 1)
                    continue;

                dst = (int16_t *)pic->data[act_plane];
                if (avctx->pix_fmt == AV_PIX_FMT_BAYER_RGGB16) {
                    if (plane & 1)
                        dst++;
                    if (plane > 1)
                        dst += pic->linesize[act_plane] >> 1;
                }

                if (avctx->pix_fmt == AV_PIX_FMT_BAYER_RGGB16 &&
                    (lowpass_height * 2 > avctx->coded_height / 2 ||
                     lowpass_width  * 2 > avctx->coded_width  / 2    )
                    ) {
                    ret = AVERROR_INVALIDDATA;
                    goto end;
                }

                low  = s->plane[plane].l_h[6];
                high = s->plane[plane].l_h[7];
                for (i = 0; i < lowpass_height * 2; i++) {
                    if (avctx->pix_fmt == AV_PIX_FMT_BAYER_RGGB16)
                        horiz_filter_clip_bayer(dst, low, high, lowpass_width, s->bpc);
                    else
                        horiz_filter_clip(dst, low, high, lowpass_width, s->bpc);
                    low  += lowpass_width;
                    high += lowpass_width;
                    dst  += dst_linesize;
                }
            } else {
                pic->interlaced_frame = 1;
                low    = s->plane[plane].l_h[7];
                high   = s->plane[plane].subband[14];
                output = s->plane[plane].l_h[6];
                for (i = 0; i < lowpass_height; i++) {
                    horiz_filter(output, low, high, lowpass_width);
                    low    += lowpass_width;
                    high   += lowpass_width;
                    output += lowpass_width * 2;
                }

                low    = s->plane[plane].subband[15];
                high   = s->plane[plane].subband[16];
                output = s->plane[plane].l_h[7];
                for (i = 0; i < lowpass_height; i++) {
                    horiz_filter(output, low, high, lowpass_width);
                    low    += lowpass_width;
                    high   += lowpass_width;
                    output += lowpass_width * 2;
                }

                low    = s->plane[plane].l_h[9];
                high   = s->plane[plane].subband[11];
                output = s->plane[plane].l_h[8];
                for (i = 0; i < lowpass_height; i++) {
                    horiz_filter(output, low, high, lowpass_width);
                    low    += lowpass_width;
                    high   += lowpass_width;
                    output += lowpass_width * 2;
                }

                low    = s->plane[plane].subband[12];
                high   = s->plane[plane].subband[13];
                output = s->plane[plane].l_h[9];
                for (i = 0; i < lowpass_height; i++) {
                    horiz_filter(output, low, high, lowpass_width);
                    low    += lowpass_width;
                    high   += lowpass_width;
                    output += lowpass_width * 2;
                }

                if (s->sample_type == 1)
                    continue;

                dst  = (int16_t *)pic->data[act_plane];
                low  = s->plane[plane].l_h[6];
                high = s->plane[plane].l_h[7];
                for (i = 0; i < lowpass_height; i++) {
                    interlaced_vertical_filter(dst, low, high, lowpass_width * 2,  pic->linesize[act_plane]/2, act_plane);
                    low  += lowpass_width * 2;
                    high += lowpass_width * 2;
                    dst  += pic->linesize[act_plane];
                }
            }
        }
    }

        int16_t *low, *high, *dst;
        int lowpass_height, lowpass_width, highpass_stride;
        ptrdiff_t dst_linesize;

        for (plane = 0; plane < s->planes; plane++) {
            int act_plane = plane == 1 ? 2 : plane == 2 ? 1 : plane;

            if (avctx->pix_fmt == AV_PIX_FMT_BAYER_RGGB16) {
                act_plane = 0;
                dst_linesize = pic->linesize[act_plane];
            } else {
                dst_linesize = pic->linesize[act_plane] / 2;
            }

            lowpass_height  = s->plane[plane].band[4][1].height;
            lowpass_width   = s->plane[plane].band[4][1].width;
            highpass_stride = s->plane[plane].band[4][1].stride;

            if (s->progressive) {
                dst = (int16_t *)pic->data[act_plane];
                low  = s->plane[plane].l_h[8];
                high = s->plane[plane].l_h[9];

                if (avctx->pix_fmt == AV_PIX_FMT_BAYER_RGGB16) {
                    if (plane & 1)
                        dst++;
                    if (plane > 1)
                        dst += pic->linesize[act_plane] >> 1;
                }

                if (avctx->pix_fmt == AV_PIX_FMT_BAYER_RGGB16 &&
                    (lowpass_height * 2 > avctx->coded_height / 2 ||
                     lowpass_width  * 2 > avctx->coded_width  / 2    )
                    ) {
                    ret = AVERROR_INVALIDDATA;
                    goto end;
                }

                for (i = 0; i < lowpass_height * 2; i++) {
                    if (avctx->pix_fmt == AV_PIX_FMT_BAYER_RGGB16)
                        horiz_filter_clip_bayer(dst, low, high, lowpass_width, s->bpc);
                    else
                        horiz_filter_clip(dst, low, high, lowpass_width, s->bpc);
                    low  += lowpass_width;
                    high += lowpass_width;
                    dst  += dst_linesize;
                }
            } else {
                dst  = (int16_t *)pic->data[act_plane];
                low  = s->plane[plane].l_h[8];
                high = s->plane[plane].l_h[9];
                for (i = 0; i < lowpass_height; i++) {
                    interlaced_vertical_filter(dst, low, high, lowpass_width * 2,  pic->linesize[act_plane]/2, act_plane);
                    low  += lowpass_width * 2;
                    high += lowpass_width * 2;
                    dst  += pic->linesize[act_plane];
                }
            }
        }
    }

        process_bayer(pic, s->bpc);
        return ret;

}

{



}

#if HAVE_THREADS
static int update_thread_context(AVCodecContext *dst, const AVCodecContext *src)
{
    CFHDContext *psrc = src->priv_data;
    CFHDContext *pdst = dst->priv_data;
    int ret;

    if (dst == src || psrc->transform_type == 0)
        return 0;

    pdst->a_format = psrc->a_format;
    pdst->a_width  = psrc->a_width;
    pdst->a_height = psrc->a_height;
    pdst->transform_type = psrc->transform_type;
    pdst->progressive = psrc->progressive;
    pdst->planes = psrc->planes;

    if (!pdst->plane[0].idwt_buf) {
        pdst->coded_width  = pdst->a_width;
        pdst->coded_height = pdst->a_height;
        pdst->coded_format = pdst->a_format;
        ret = alloc_buffers(dst);
        if (ret < 0)
            return ret;
    }

    for (int plane = 0; plane < pdst->planes; plane++) {
        memcpy(pdst->plane[plane].band, psrc->plane[plane].band, sizeof(pdst->plane[plane].band));
        memcpy(pdst->plane[plane].idwt_buf, psrc->plane[plane].idwt_buf,
               pdst->plane[plane].idwt_size * sizeof(int16_t));
    }

    return 0;
}
#endif

AVCodec ff_cfhd_decoder = {
    .name             = "cfhd",
    .long_name        = NULL_IF_CONFIG_SMALL("GoPro CineForm HD"),
    .type             = AVMEDIA_TYPE_VIDEO,
    .id               = AV_CODEC_ID_CFHD,
    .priv_data_size   = sizeof(CFHDContext),
    .init             = cfhd_init,
    .close            = cfhd_close,
    .decode           = cfhd_decode,
    .update_thread_context = ONLY_IF_THREADS_ENABLED(update_thread_context),
    .capabilities     = AV_CODEC_CAP_DR1 | AV_CODEC_CAP_FRAME_THREADS,
    .caps_internal    = FF_CODEC_CAP_INIT_THREADSAFE | FF_CODEC_CAP_INIT_CLEANUP,
};
