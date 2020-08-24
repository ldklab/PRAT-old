/*
 * Rate control for video encoders
 *
 * Copyright (c) 2002-2004 Michael Niedermayer <michaelni@gmx.at>
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
 * Rate control for video encoders.
 */

#include "libavutil/attributes.h"
#include "libavutil/internal.h"

#include "avcodec.h"
#include "internal.h"
#include "ratecontrol.h"
#include "mpegutils.h"
#include "mpegvideo.h"
#include "libavutil/eval.h"

void ff_write_pass1_stats(MpegEncContext *s)
{
    snprintf(s->avctx->stats_out, 256,
             "in:%d out:%d type:%d q:%d itex:%d ptex:%d mv:%d misc:%d "
             "fcode:%d bcode:%d mc-var:%"PRId64" var:%"PRId64" icount:%d skipcount:%d hbits:%d;\n",
             s->current_picture_ptr->f->display_picture_number,
             s->current_picture_ptr->f->coded_picture_number,
             s->pict_type,
             s->current_picture.f->quality,
             s->i_tex_bits,
             s->p_tex_bits,
             s->mv_bits,
             s->misc_bits,
             s->f_code,
             s->b_code,
             s->current_picture.mc_mb_var_sum,
             s->current_picture.mb_var_sum,
             s->i_count, s->skip_count,
             s->header_bits);
}

{
    return 1.0 / av_q2d(avctx->time_base) / FFMAX(avctx->ticks_per_frame, 1);
}

static inline double qp2bits(RateControlEntry *rce, double qp)
{
    if (qp <= 0.0) {
        av_log(NULL, AV_LOG_ERROR, "qp<=0.0\n");
    }
    return rce->qscale * (double)(rce->i_tex_bits + rce->p_tex_bits + 1) / qp;
}

{
        av_log(NULL, AV_LOG_ERROR, "bits<0.9\n");
    }
}

static double get_diff_limited_q(MpegEncContext *s, RateControlEntry *rce, double q)
{
    RateControlContext *rcc   = &s->rc_context;
    AVCodecContext *a         = s->avctx;
    const int pict_type       = rce->new_pict_type;
    const double last_p_q     = rcc->last_qscale_for[AV_PICTURE_TYPE_P];
    const double last_non_b_q = rcc->last_qscale_for[rcc->last_non_b_pict_type];

    if (pict_type == AV_PICTURE_TYPE_I &&
        (a->i_quant_factor > 0.0 || rcc->last_non_b_pict_type == AV_PICTURE_TYPE_P))
        q = last_p_q * FFABS(a->i_quant_factor) + a->i_quant_offset;
    else if (pict_type == AV_PICTURE_TYPE_B &&
             a->b_quant_factor > 0.0)
        q = last_non_b_q * a->b_quant_factor + a->b_quant_offset;
    if (q < 1)
        q = 1;

    /* last qscale / qdiff stuff */
    if (rcc->last_non_b_pict_type == pict_type || pict_type != AV_PICTURE_TYPE_I) {
        double last_q     = rcc->last_qscale_for[pict_type];
        const int maxdiff = FF_QP2LAMBDA * a->max_qdiff;

        if (q > last_q + maxdiff)
            q = last_q + maxdiff;
        else if (q < last_q - maxdiff)
            q = last_q - maxdiff;
    }

    rcc->last_qscale_for[pict_type] = q; // Note we cannot do that after blurring

    if (pict_type != AV_PICTURE_TYPE_B)
        rcc->last_non_b_pict_type = pict_type;

    return q;
}

/**
 * Get the qmin & qmax for pict_type.
 */
{


    }


        qmax = qmin;


                            double q, int frame_num)
{


    /* modulation */
        frame_num % s->rc_qmod_freq == 0 &&
        pict_type == AV_PICTURE_TYPE_P)
        q *= s->rc_qmod_amp;

    /* buffer overflow/underflow protection */
        double expected_size = rcc->buffer_index;
        double q_limit;

        if (min_rate) {
            double d = 2 * (buffer_size - expected_size) / buffer_size;
            if (d > 1.0)
                d = 1.0;
            else if (d < 0.0001)
                d = 0.0001;
            q *= pow(d, 1.0 / s->rc_buffer_aggressivity);

            q_limit = bits2qp(rce,
                              FFMAX((min_rate - buffer_size + rcc->buffer_index) *
                                    s->avctx->rc_min_vbv_overflow_use, 1));

            if (q > q_limit) {
                if (s->avctx->debug & FF_DEBUG_RC)
                    av_log(s->avctx, AV_LOG_DEBUG,
                           "limiting QP %f -> %f\n", q, q_limit);
                q = q_limit;
            }
        }

        if (max_rate) {
            double d = 2 * expected_size / buffer_size;
            if (d > 1.0)
                d = 1.0;
            else if (d < 0.0001)
                d = 0.0001;
            q /= pow(d, 1.0 / s->rc_buffer_aggressivity);

            q_limit = bits2qp(rce,
                              FFMAX(rcc->buffer_index *
                                    s->avctx->rc_max_available_vbv_use,
                                    1));
            if (q < q_limit) {
                if (s->avctx->debug & FF_DEBUG_RC)
                    av_log(s->avctx, AV_LOG_DEBUG,
                           "limiting QP %f -> %f\n", q, q_limit);
                q = q_limit;
            }
        }
    }
            q, max_rate, min_rate, buffer_size, rcc->buffer_index,
            s->rc_buffer_aggressivity);
            q = qmin;
    } else {
        double min2 = log(qmin);
        double max2 = log(qmax);

        q  = log(q);
        q  = (q - min2) / (max2 - min2) - 0.5;
        q *= -4.0;
        q  = 1.0 / (1.0 + exp(q));
        q  = q * (max2 - min2) + min2;

        q = exp(q);
    }

}

/**
 * Modify the bitrate curve from pass1 for one frame.
 */
                         double rate_factor, int frame_num)
{

        M_PI,
        M_E,
        rce->pict_type == AV_PICTURE_TYPE_I,
        rce->pict_type == AV_PICTURE_TYPE_P,
        rce->pict_type == AV_PICTURE_TYPE_B,
        0
    };

        av_log(s->avctx, AV_LOG_ERROR, "Error evaluating rc_eq \"%s\"\n", s->rc_eq);
        return -1;
    }

        bits = 0.0;

    /* user override */
        RcOverride *rco = s->avctx->rc_override;
        if (rco[i].start_frame > frame_num)
            continue;
        if (rco[i].end_frame < frame_num)
            continue;

        if (rco[i].qscale)
            bits = qp2bits(rce, rco[i].qscale);  // FIXME move at end to really force it?
        else
            bits *= rco[i].quality_factor;
    }


    /* I/B difference */
        q = -q * s->avctx->b_quant_factor + s->avctx->b_quant_offset;
        q = 1;

    return q;
}

static int init_pass2(MpegEncContext *s)
{
    RateControlContext *rcc = &s->rc_context;
    AVCodecContext *a       = s->avctx;
    int i, toobig;
    double fps             = get_fps(s->avctx);
    double complexity[5]   = { 0 }; // approximate bits at quant=1
    uint64_t const_bits[5] = { 0 }; // quantizer independent bits
    uint64_t all_const_bits;
    uint64_t all_available_bits = (uint64_t)(s->bit_rate *
                                             (double)rcc->num_entries / fps);
    double rate_factor          = 0;
    double step;
    const int filter_size = (int)(a->qblur * 4) | 1;
    double expected_bits = 0; // init to silence gcc warning
    double *qscale, *blurred_qscale, qscale_sum;

    /* find complexity & const_bits & decide the pict_types */
    for (i = 0; i < rcc->num_entries; i++) {
        RateControlEntry *rce = &rcc->entry[i];

        rce->new_pict_type                = rce->pict_type;
        rcc->i_cplx_sum[rce->pict_type]  += rce->i_tex_bits * rce->qscale;
        rcc->p_cplx_sum[rce->pict_type]  += rce->p_tex_bits * rce->qscale;
        rcc->mv_bits_sum[rce->pict_type] += rce->mv_bits;
        rcc->frame_count[rce->pict_type]++;

        complexity[rce->new_pict_type] += (rce->i_tex_bits + rce->p_tex_bits) *
                                          (double)rce->qscale;
        const_bits[rce->new_pict_type] += rce->mv_bits + rce->misc_bits;
    }

    all_const_bits = const_bits[AV_PICTURE_TYPE_I] +
                     const_bits[AV_PICTURE_TYPE_P] +
                     const_bits[AV_PICTURE_TYPE_B];

    if (all_available_bits < all_const_bits) {
        av_log(s->avctx, AV_LOG_ERROR, "requested bitrate is too low\n");
        return -1;
    }

    qscale         = av_malloc_array(rcc->num_entries, sizeof(double));
    blurred_qscale = av_malloc_array(rcc->num_entries, sizeof(double));
    if (!qscale || !blurred_qscale) {
        av_free(qscale);
        av_free(blurred_qscale);
        return AVERROR(ENOMEM);
    }
    toobig = 0;

    for (step = 256 * 256; step > 0.0000001; step *= 0.5) {
        expected_bits = 0;
        rate_factor  += step;

        rcc->buffer_index = s->avctx->rc_buffer_size / 2;

        /* find qscale */
        for (i = 0; i < rcc->num_entries; i++) {
            RateControlEntry *rce = &rcc->entry[i];

            qscale[i] = get_qscale(s, &rcc->entry[i], rate_factor, i);
            rcc->last_qscale_for[rce->pict_type] = qscale[i];
        }
        av_assert0(filter_size % 2 == 1);

        /* fixed I/B QP relative to P mode */
        for (i = FFMAX(0, rcc->num_entries - 300); i < rcc->num_entries; i++) {
            RateControlEntry *rce = &rcc->entry[i];

            qscale[i] = get_diff_limited_q(s, rce, qscale[i]);
        }

        for (i = rcc->num_entries - 1; i >= 0; i--) {
            RateControlEntry *rce = &rcc->entry[i];

            qscale[i] = get_diff_limited_q(s, rce, qscale[i]);
        }

        /* smooth curve */
        for (i = 0; i < rcc->num_entries; i++) {
            RateControlEntry *rce = &rcc->entry[i];
            const int pict_type   = rce->new_pict_type;
            int j;
            double q = 0.0, sum = 0.0;

            for (j = 0; j < filter_size; j++) {
                int index    = i + j - filter_size / 2;
                double d     = index - i;
                double coeff = a->qblur == 0 ? 1.0 : exp(-d * d / (a->qblur * a->qblur));

                if (index < 0 || index >= rcc->num_entries)
                    continue;
                if (pict_type != rcc->entry[index].new_pict_type)
                    continue;
                q   += qscale[index] * coeff;
                sum += coeff;
            }
            blurred_qscale[i] = q / sum;
        }

        /* find expected bits */
        for (i = 0; i < rcc->num_entries; i++) {
            RateControlEntry *rce = &rcc->entry[i];
            double bits;

            rce->new_qscale = modify_qscale(s, rce, blurred_qscale[i], i);

            bits  = qp2bits(rce, rce->new_qscale) + rce->mv_bits + rce->misc_bits;
            bits += 8 * ff_vbv_update(s, bits);

            rce->expected_bits = expected_bits;
            expected_bits     += bits;
        }

        ff_dlog(s->avctx,
                "expected_bits: %f all_available_bits: %d rate_factor: %f\n",
                expected_bits, (int)all_available_bits, rate_factor);
        if (expected_bits > all_available_bits) {
            rate_factor -= step;
            ++toobig;
        }
    }
    av_free(qscale);
    av_free(blurred_qscale);

    /* check bitrate calculations and print info */
    qscale_sum = 0.0;
    for (i = 0; i < rcc->num_entries; i++) {
        ff_dlog(s, "[lavc rc] entry[%d].new_qscale = %.3f  qp = %.3f\n",
                i,
                rcc->entry[i].new_qscale,
                rcc->entry[i].new_qscale / FF_QP2LAMBDA);
        qscale_sum += av_clip(rcc->entry[i].new_qscale / FF_QP2LAMBDA,
                              s->avctx->qmin, s->avctx->qmax);
    }
    av_assert0(toobig <= 40);
    av_log(s->avctx, AV_LOG_DEBUG,
           "[lavc rc] requested bitrate: %"PRId64" bps  expected bitrate: %"PRId64" bps\n",
           s->bit_rate,
           (int64_t)(expected_bits / ((double)all_available_bits / s->bit_rate)));
    av_log(s->avctx, AV_LOG_DEBUG,
           "[lavc rc] estimated target average qp: %.3f\n",
           (float)qscale_sum / rcc->num_entries);
    if (toobig == 0) {
        av_log(s->avctx, AV_LOG_INFO,
               "[lavc rc] Using all of requested bitrate is not "
               "necessary for this video with these parameters.\n");
    } else if (toobig == 40) {
        av_log(s->avctx, AV_LOG_ERROR,
               "[lavc rc] Error: bitrate too low for this video "
               "with these parameters.\n");
        return -1;
    } else if (fabs(expected_bits / all_available_bits - 1.0) > 0.01) {
        av_log(s->avctx, AV_LOG_ERROR,
               "[lavc rc] Error: 2pass curve failed to converge\n");
        return -1;
    }

    return 0;
}

{
        "PI",
        "E",
        "iTex",
        "pTex",
        "tex",
        "mv",
        "fCode",
        "iCount",
        "mcVar",
        "var",
        "isI",
        "isP",
        "isB",
        "avgQP",
        "qComp",
        "avgIITex",
        "avgPITex",
        "avgPPTex",
        "avgBPTex",
        "avgTex",
        NULL
    };
        (double (*)(void *, double)) bits2qp,
        (double (*)(void *, double)) qp2bits,
        NULL
    };
        "bits2qp",
        "qp2bits",
        NULL
    };

        if (s->avctx->rc_max_rate) {
            s->avctx->rc_max_available_vbv_use = av_clipf(s->avctx->rc_max_rate/(s->avctx->rc_buffer_size*get_fps(s->avctx)), 1.0/3, 1.0);
        } else
            s->avctx->rc_max_available_vbv_use = 1.0;
    }

                        const_names, func1_names, func1,
                        NULL, NULL, 0, s->avctx);
        av_log(s->avctx, AV_LOG_ERROR, "Error parsing rc_eq \"%s\"\n", s->rc_eq);
        return res;
    }



    }

        int i;
        char *p;

        /* find number of pics */
        p = s->avctx->stats_in;
        for (i = -1; p; i++)
            p = strchr(p + 1, ';');
        i += s->max_b_frames;
        if (i <= 0 || i >= INT_MAX / sizeof(RateControlEntry))
            return -1;
        rcc->entry       = av_mallocz(i * sizeof(RateControlEntry));
        if (!rcc->entry)
            return AVERROR(ENOMEM);
        rcc->num_entries = i;

        /* init all to skipped P-frames
         * (with B-frames we might have a not encoded frame at the end FIXME) */
        for (i = 0; i < rcc->num_entries; i++) {
            RateControlEntry *rce = &rcc->entry[i];

            rce->pict_type  = rce->new_pict_type = AV_PICTURE_TYPE_P;
            rce->qscale     = rce->new_qscale    = FF_QP2LAMBDA * 2;
            rce->misc_bits  = s->mb_num + 10;
            rce->mb_var_sum = s->mb_num * 100;
        }

        /* read stats */
        p = s->avctx->stats_in;
        for (i = 0; i < rcc->num_entries - s->max_b_frames; i++) {
            RateControlEntry *rce;
            int picture_number;
            int e;
            char *next;

            next = strchr(p, ';');
            if (next) {
                (*next) = 0; // sscanf is unbelievably slow on looong strings // FIXME copy / do not write
                next++;
            }
            e = sscanf(p, " in:%d ", &picture_number);

            av_assert0(picture_number >= 0);
            av_assert0(picture_number < rcc->num_entries);
            rce = &rcc->entry[picture_number];

            e += sscanf(p, " in:%*d out:%*d type:%d q:%f itex:%d ptex:%d mv:%d misc:%d fcode:%d bcode:%d mc-var:%"SCNd64" var:%"SCNd64" icount:%d skipcount:%d hbits:%d",
                        &rce->pict_type, &rce->qscale, &rce->i_tex_bits, &rce->p_tex_bits,
                        &rce->mv_bits, &rce->misc_bits,
                        &rce->f_code, &rce->b_code,
                        &rce->mc_mb_var_sum, &rce->mb_var_sum,
                        &rce->i_count, &rce->skip_count, &rce->header_bits);
            if (e != 14) {
                av_log(s->avctx, AV_LOG_ERROR,
                       "statistics are damaged at line %d, parser out=%d\n",
                       i, e);
                return -1;
            }

            p = next;
        }

        if (init_pass2(s) < 0) {
            ff_rate_control_uninit(s);
            return -1;
        }
    }



            av_log(s->avctx, AV_LOG_ERROR, "qblur too large\n");
            return -1;
        }
        /* init stuff with the user specified complexity */
            for (i = 0; i < 60 * 30; i++) {
                double bits = s->rc_initial_cplx * (i / 10000.0 + 1.0) * s->mb_num;
                RateControlEntry rce;

                if (i % ((s->gop_size + 3) / 4) == 0)
                    rce.pict_type = AV_PICTURE_TYPE_I;
                else if (i % (s->max_b_frames + 1))
                    rce.pict_type = AV_PICTURE_TYPE_B;
                else
                    rce.pict_type = AV_PICTURE_TYPE_P;

                rce.new_pict_type = rce.pict_type;
                rce.mc_mb_var_sum = bits * s->mb_num / 100000;
                rce.mb_var_sum    = s->mb_num;

                rce.qscale    = FF_QP2LAMBDA * 2;
                rce.f_code    = 2;
                rce.b_code    = 1;
                rce.misc_bits = 1;

                if (s->pict_type == AV_PICTURE_TYPE_I) {
                    rce.i_count    = s->mb_num;
                    rce.i_tex_bits = bits;
                    rce.p_tex_bits = 0;
                    rce.mv_bits    = 0;
                } else {
                    rce.i_count    = 0; // FIXME we do know this approx
                    rce.i_tex_bits = 0;
                    rce.p_tex_bits = bits * 0.9;
                    rce.mv_bits    = bits * 0.1;
                }
                rcc->i_cplx_sum[rce.pict_type]  += rce.i_tex_bits * rce.qscale;
                rcc->p_cplx_sum[rce.pict_type]  += rce.p_tex_bits * rce.qscale;
                rcc->mv_bits_sum[rce.pict_type] += rce.mv_bits;
                rcc->frame_count[rce.pict_type]++;

                get_qscale(s, &rce, rcc->pass1_wanted_bits / rcc->pass1_rc_eq_output_sum, i);

                // FIXME misbehaves a little for variable fps
                rcc->pass1_wanted_bits += s->bit_rate / get_fps(s->avctx);
            }
        }
    }

    return 0;
}

{


{

            buffer_size, rcc->buffer_index, frame_size, min_rate, max_rate);


            av_log(s->avctx, AV_LOG_ERROR, "rc buffer underflow\n");
            if (frame_size > max_rate && s->qscale == s->avctx->qmax) {
                av_log(s->avctx, AV_LOG_ERROR, "max bitrate possibly too small or try trellis with large lmax or increase qmax\n");
            }
            rcc->buffer_index = 0;
        }



                stuffing = 4;

                av_log(s->avctx, AV_LOG_DEBUG, "stuffing %d bytes\n", stuffing);

        }
    }
    return 0;
}

{
}

{
        return;

}

{


        } else {
        }

        else

        }
                                (float)mb_distance / (float)(mb_height / 5));
                                (float)mb_distance / (float)(mb_height / 5));
        }


            factor = 0.00001;

    }

    /* handle qmin/qmax clipping */
        float factor = bits_sum / cplx_sum;
        for (i = 0; i < s->mb_num; i++) {
            float newq = q * cplx_tab[i] / bits_tab[i];
            newq *= factor;

            if (newq > qmax) {
                bits_sum -= bits_tab[i];
                cplx_sum -= cplx_tab[i] * q / qmax;
            } else if (newq < qmin) {
                bits_sum -= bits_tab[i];
                cplx_sum -= cplx_tab[i] * q / qmin;
            }
        }
        if (bits_sum < 0.001)
            bits_sum = 0.001;
        if (cplx_sum < 0.001)
            cplx_sum = 0.001;
    }


            newq *= bits_sum / cplx_sum;
        }


            intq = qmax;
            intq = qmin;
    }

void ff_get_2pass_fcode(MpegEncContext *s)
{
    RateControlContext *rcc = &s->rc_context;
    RateControlEntry *rce   = &rcc->entry[s->picture_number];

    s->f_code = rce->f_code;
    s->b_code = rce->b_code;
}

// FIXME rd or at least approx for dquant

{


    /* update predictors */
                         rcc->last_qscale,
                         sqrt(last_var),
    }

        av_assert0(picture_number >= 0);
        if (picture_number >= rcc->num_entries) {
            av_log(s, AV_LOG_ERROR, "Input is longer than 2-pass log file\n");
            return -1;
        }
        rce         = &rcc->entry[picture_number];
        wanted_bits = rce->expected_bits;
    } else {

        /* FIXME add a dts field to AVFrame and ensure it is set and use it
         * here instead of reordering but the reordering is simpler for now
         * until H.264 B-pyramid must be handled. */
        else

        else
    }



        if (pict_type != AV_PICTURE_TYPE_I)
            av_assert0(pict_type == rce->new_pict_type);

        q = rce->new_qscale / br_compensation;
        ff_dlog(s, "%f %f %f last:%d var:%"PRId64" type:%d//\n", q, rce->new_qscale,
                br_compensation, s->frame_bits, var, pict_type);
    } else {

        } else {
        }


            return -1;


        // FIXME type dependent blur like in 2-pass

        }



    }

        av_log(s->avctx, AV_LOG_DEBUG,
               "%c qp:%d<%2.1f<%d %d want:%"PRId64" total:%"PRId64" comp:%f st_q:%2.2f "
               "size:%d var:%"PRId64"/%"PRId64" br:%"PRId64" fps:%d\n",
               av_get_picture_type_char(pict_type),
               qmin, q, qmax, picture_number,
               wanted_bits / 1000, s->total_bits / 1000,
               br_compensation, short_term_q, s->frame_bits,
               pic->mb_var_sum, pic->mc_mb_var_sum,
               s->bit_rate / 1000, (int)fps);
    }

        q = qmin;
        q = qmax;

    else

    }
    return q;
}
