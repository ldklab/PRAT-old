/*
 * RV10/RV20 decoder
 * Copyright (c) 2000,2001 Fabrice Bellard
 * Copyright (c) 2002-2004 Michael Niedermayer
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
 * RV10/RV20 decoder
 */

#include <inttypes.h>

#include "libavutil/imgutils.h"

#include "avcodec.h"
#include "error_resilience.h"
#include "h263.h"
#include "h263data.h"
#include "internal.h"
#include "mpeg_er.h"
#include "mpegutils.h"
#include "mpegvideo.h"
#include "mpeg4video.h"
#include "mpegvideodata.h"
#include "rv10.h"

#define RV_GET_MAJOR_VER(x)  ((x) >> 28)
#define RV_GET_MINOR_VER(x) (((x) >> 20) & 0xFF)
#define RV_GET_MICRO_VER(x) (((x) >> 12) & 0xFF)

#define DC_VLC_BITS 14 // FIXME find a better solution

typedef struct RVDecContext {
    MpegEncContext m;
    int sub_id;
    int orig_width, orig_height;
} RVDecContext;

static const uint16_t rv_lum_code[256] = {
    0x3e7f, 0x0f00, 0x0f01, 0x0f02, 0x0f03, 0x0f04, 0x0f05, 0x0f06,
    0x0f07, 0x0f08, 0x0f09, 0x0f0a, 0x0f0b, 0x0f0c, 0x0f0d, 0x0f0e,
    0x0f0f, 0x0f10, 0x0f11, 0x0f12, 0x0f13, 0x0f14, 0x0f15, 0x0f16,
    0x0f17, 0x0f18, 0x0f19, 0x0f1a, 0x0f1b, 0x0f1c, 0x0f1d, 0x0f1e,
    0x0f1f, 0x0f20, 0x0f21, 0x0f22, 0x0f23, 0x0f24, 0x0f25, 0x0f26,
    0x0f27, 0x0f28, 0x0f29, 0x0f2a, 0x0f2b, 0x0f2c, 0x0f2d, 0x0f2e,
    0x0f2f, 0x0f30, 0x0f31, 0x0f32, 0x0f33, 0x0f34, 0x0f35, 0x0f36,
    0x0f37, 0x0f38, 0x0f39, 0x0f3a, 0x0f3b, 0x0f3c, 0x0f3d, 0x0f3e,
    0x0f3f, 0x0380, 0x0381, 0x0382, 0x0383, 0x0384, 0x0385, 0x0386,
    0x0387, 0x0388, 0x0389, 0x038a, 0x038b, 0x038c, 0x038d, 0x038e,
    0x038f, 0x0390, 0x0391, 0x0392, 0x0393, 0x0394, 0x0395, 0x0396,
    0x0397, 0x0398, 0x0399, 0x039a, 0x039b, 0x039c, 0x039d, 0x039e,
    0x039f, 0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6,
    0x00c7, 0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce,
    0x00cf, 0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056,
    0x0057, 0x0020, 0x0021, 0x0022, 0x0023, 0x000c, 0x000d, 0x0004,
    0x0000, 0x0005, 0x000e, 0x000f, 0x0024, 0x0025, 0x0026, 0x0027,
    0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
    0x00d0, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7,
    0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df,
    0x03a0, 0x03a1, 0x03a2, 0x03a3, 0x03a4, 0x03a5, 0x03a6, 0x03a7,
    0x03a8, 0x03a9, 0x03aa, 0x03ab, 0x03ac, 0x03ad, 0x03ae, 0x03af,
    0x03b0, 0x03b1, 0x03b2, 0x03b3, 0x03b4, 0x03b5, 0x03b6, 0x03b7,
    0x03b8, 0x03b9, 0x03ba, 0x03bb, 0x03bc, 0x03bd, 0x03be, 0x03bf,
    0x0f40, 0x0f41, 0x0f42, 0x0f43, 0x0f44, 0x0f45, 0x0f46, 0x0f47,
    0x0f48, 0x0f49, 0x0f4a, 0x0f4b, 0x0f4c, 0x0f4d, 0x0f4e, 0x0f4f,
    0x0f50, 0x0f51, 0x0f52, 0x0f53, 0x0f54, 0x0f55, 0x0f56, 0x0f57,
    0x0f58, 0x0f59, 0x0f5a, 0x0f5b, 0x0f5c, 0x0f5d, 0x0f5e, 0x0f5f,
    0x0f60, 0x0f61, 0x0f62, 0x0f63, 0x0f64, 0x0f65, 0x0f66, 0x0f67,
    0x0f68, 0x0f69, 0x0f6a, 0x0f6b, 0x0f6c, 0x0f6d, 0x0f6e, 0x0f6f,
    0x0f70, 0x0f71, 0x0f72, 0x0f73, 0x0f74, 0x0f75, 0x0f76, 0x0f77,
    0x0f78, 0x0f79, 0x0f7a, 0x0f7b, 0x0f7c, 0x0f7d, 0x0f7e, 0x0f7f,
};

static const uint8_t rv_lum_bits[256] = {
    14, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12,
    12, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10,
    10,  8,  8,  8,  8,  8,  8,  8,
     8,  8,  8,  8,  8,  8,  8,  8,
     8,  7,  7,  7,  7,  7,  7,  7,
     7,  6,  6,  6,  6,  5,  5,  4,
     2,  4,  5,  5,  6,  6,  6,  6,
     7,  7,  7,  7,  7,  7,  7,  7,
     8,  8,  8,  8,  8,  8,  8,  8,
     8,  8,  8,  8,  8,  8,  8,  8,
    10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10,
    12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12,
};

static const uint16_t rv_chrom_code[256] = {
    0xfe7f, 0x3f00, 0x3f01, 0x3f02, 0x3f03, 0x3f04, 0x3f05, 0x3f06,
    0x3f07, 0x3f08, 0x3f09, 0x3f0a, 0x3f0b, 0x3f0c, 0x3f0d, 0x3f0e,
    0x3f0f, 0x3f10, 0x3f11, 0x3f12, 0x3f13, 0x3f14, 0x3f15, 0x3f16,
    0x3f17, 0x3f18, 0x3f19, 0x3f1a, 0x3f1b, 0x3f1c, 0x3f1d, 0x3f1e,
    0x3f1f, 0x3f20, 0x3f21, 0x3f22, 0x3f23, 0x3f24, 0x3f25, 0x3f26,
    0x3f27, 0x3f28, 0x3f29, 0x3f2a, 0x3f2b, 0x3f2c, 0x3f2d, 0x3f2e,
    0x3f2f, 0x3f30, 0x3f31, 0x3f32, 0x3f33, 0x3f34, 0x3f35, 0x3f36,
    0x3f37, 0x3f38, 0x3f39, 0x3f3a, 0x3f3b, 0x3f3c, 0x3f3d, 0x3f3e,
    0x3f3f, 0x0f80, 0x0f81, 0x0f82, 0x0f83, 0x0f84, 0x0f85, 0x0f86,
    0x0f87, 0x0f88, 0x0f89, 0x0f8a, 0x0f8b, 0x0f8c, 0x0f8d, 0x0f8e,
    0x0f8f, 0x0f90, 0x0f91, 0x0f92, 0x0f93, 0x0f94, 0x0f95, 0x0f96,
    0x0f97, 0x0f98, 0x0f99, 0x0f9a, 0x0f9b, 0x0f9c, 0x0f9d, 0x0f9e,
    0x0f9f, 0x03c0, 0x03c1, 0x03c2, 0x03c3, 0x03c4, 0x03c5, 0x03c6,
    0x03c7, 0x03c8, 0x03c9, 0x03ca, 0x03cb, 0x03cc, 0x03cd, 0x03ce,
    0x03cf, 0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6,
    0x00e7, 0x0030, 0x0031, 0x0032, 0x0033, 0x0008, 0x0009, 0x0002,
    0x0000, 0x0003, 0x000a, 0x000b, 0x0034, 0x0035, 0x0036, 0x0037,
    0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
    0x03d0, 0x03d1, 0x03d2, 0x03d3, 0x03d4, 0x03d5, 0x03d6, 0x03d7,
    0x03d8, 0x03d9, 0x03da, 0x03db, 0x03dc, 0x03dd, 0x03de, 0x03df,
    0x0fa0, 0x0fa1, 0x0fa2, 0x0fa3, 0x0fa4, 0x0fa5, 0x0fa6, 0x0fa7,
    0x0fa8, 0x0fa9, 0x0faa, 0x0fab, 0x0fac, 0x0fad, 0x0fae, 0x0faf,
    0x0fb0, 0x0fb1, 0x0fb2, 0x0fb3, 0x0fb4, 0x0fb5, 0x0fb6, 0x0fb7,
    0x0fb8, 0x0fb9, 0x0fba, 0x0fbb, 0x0fbc, 0x0fbd, 0x0fbe, 0x0fbf,
    0x3f40, 0x3f41, 0x3f42, 0x3f43, 0x3f44, 0x3f45, 0x3f46, 0x3f47,
    0x3f48, 0x3f49, 0x3f4a, 0x3f4b, 0x3f4c, 0x3f4d, 0x3f4e, 0x3f4f,
    0x3f50, 0x3f51, 0x3f52, 0x3f53, 0x3f54, 0x3f55, 0x3f56, 0x3f57,
    0x3f58, 0x3f59, 0x3f5a, 0x3f5b, 0x3f5c, 0x3f5d, 0x3f5e, 0x3f5f,
    0x3f60, 0x3f61, 0x3f62, 0x3f63, 0x3f64, 0x3f65, 0x3f66, 0x3f67,
    0x3f68, 0x3f69, 0x3f6a, 0x3f6b, 0x3f6c, 0x3f6d, 0x3f6e, 0x3f6f,
    0x3f70, 0x3f71, 0x3f72, 0x3f73, 0x3f74, 0x3f75, 0x3f76, 0x3f77,
    0x3f78, 0x3f79, 0x3f7a, 0x3f7b, 0x3f7c, 0x3f7d, 0x3f7e, 0x3f7f,
};

static const uint8_t rv_chrom_bits[256] = {
    16, 14, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14,
    14, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12,
    12, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10,
    10,  8,  8,  8,  8,  8,  8,  8,
     8,  6,  6,  6,  6,  4,  4,  3,
     2,  3,  4,  4,  6,  6,  6,  6,
     8,  8,  8,  8,  8,  8,  8,  8,
    10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10,
    12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12,
    14, 14, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14,
};

static VLC rv_dc_lum, rv_dc_chrom;

int ff_rv_decode_dc(MpegEncContext *s, int n)
{
    int code;

    if (n < 4) {
        code = get_vlc2(&s->gb, rv_dc_lum.table, DC_VLC_BITS, 2);
        if (code < 0) {
            /* XXX: I don't understand why they use LONGER codes than
             * necessary. The following code would be completely useless
             * if they had thought about it !!! */
            code = get_bits(&s->gb, 7);
            if (code == 0x7c) {
                code = (int8_t) (get_bits(&s->gb, 7) + 1);
            } else if (code == 0x7d) {
                code = -128 + get_bits(&s->gb, 7);
            } else if (code == 0x7e) {
                if (get_bits1(&s->gb) == 0)
                    code = (int8_t) (get_bits(&s->gb, 8) + 1);
                else
                    code = (int8_t) (get_bits(&s->gb, 8));
            } else if (code == 0x7f) {
                skip_bits(&s->gb, 11);
                code = 1;
            }
        } else {
            code -= 128;
        }
    } else {
        code = get_vlc2(&s->gb, rv_dc_chrom.table, DC_VLC_BITS, 2);
        /* same remark */
        if (code < 0) {
            code = get_bits(&s->gb, 9);
            if (code == 0x1fc) {
                code = (int8_t) (get_bits(&s->gb, 7) + 1);
            } else if (code == 0x1fd) {
                code = -128 + get_bits(&s->gb, 7);
            } else if (code == 0x1fe) {
                skip_bits(&s->gb, 9);
                code = 1;
            } else {
                av_log(s->avctx, AV_LOG_ERROR, "chroma dc error\n");
                return 0xffff;
            }
        } else {
            code -= 128;
        }
    }
    return -code;
}

/* read RV 1.0 compatible frame header */
{


    else

        av_log(s->avctx, AV_LOG_ERROR, "marker missing\n");



        avpriv_request_sample(s->avctx, "PB-frame");
        return AVERROR_PATCHWELCOME;
    }

        av_log(s->avctx, AV_LOG_ERROR, "Invalid qscale value: 0\n");
        return AVERROR_INVALIDDATA;
    }

            /* specific MPEG like DC coding not used */
            s->last_dc[0] = get_bits(&s->gb, 8);
            s->last_dc[1] = get_bits(&s->gb, 8);
            s->last_dc[2] = get_bits(&s->gb, 8);
            ff_dlog(s->avctx, "DC:%d %d %d\n", s->last_dc[0],
                    s->last_dc[1], s->last_dc[2]);
        }
    }
    /* if multiple packets per frame are sent, the position at which
     * to display the macroblocks is coded here */

    } else {
        s->mb_x  = 0;
        s->mb_y  = 0;
        mb_count = s->mb_width * s->mb_height;
    }

}

{

    default:
        av_log(s->avctx, AV_LOG_ERROR, "unknown frame type\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(s->avctx, AV_LOG_ERROR, "low delay B\n");
        return -1;
    }
        av_log(s->avctx, AV_LOG_ERROR, "early B-frame\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(s->avctx, AV_LOG_ERROR, "reserved bit set\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(s->avctx, AV_LOG_ERROR, "Invalid qscale value: 0\n");
        return AVERROR_INVALIDDATA;
    }


    else



            if (s->avctx->extradata_size < 8 + 2 * f) {
                av_log(s->avctx, AV_LOG_ERROR, "Extradata too small.\n");
                return AVERROR_INVALIDDATA;
            }

            new_w = 4 * ((uint8_t *) s->avctx->extradata)[6 + 2 * f];
            new_h = 4 * ((uint8_t *) s->avctx->extradata)[7 + 2 * f];
        } else {
        }
            AVRational old_aspect = s->avctx->sample_aspect_ratio;
            av_log(s->avctx, AV_LOG_DEBUG,
                   "attempting to change resolution to %dx%d\n", new_w, new_h);
            if (av_image_check_size(new_w, new_h, 0, s->avctx) < 0)
                return AVERROR_INVALIDDATA;
            ff_mpv_common_end(s);

            // attempt to keep aspect during typical resolution switches
            if (!old_aspect.num)
                old_aspect = (AVRational){1, 1};
            if (2 * (int64_t)new_w * s->height == (int64_t)new_h * s->width)
                s->avctx->sample_aspect_ratio = av_mul_q(old_aspect, (AVRational){2, 1});
            if ((int64_t)new_w * s->height == 2 * (int64_t)new_h * s->width)
                s->avctx->sample_aspect_ratio = av_mul_q(old_aspect, (AVRational){1, 2});

            ret = ff_set_dimensions(s->avctx, new_w, new_h);
            if (ret < 0)
                return ret;

            s->width  = new_w;
            s->height = new_h;
            if ((ret = ff_mpv_common_init(s)) < 0)
                return ret;
        }

            av_log(s->avctx, AV_LOG_DEBUG, "F %d/%d/%d\n", f, rpr_bits, rpr_max);
        }
    }
        return AVERROR_INVALIDDATA;


        seq -= 0x8000;
        seq += 0x8000;

        } else {
        }
    }
            av_log(s->avctx, AV_LOG_DEBUG,
                   "messed up order, possible from seeking? skipping current B-frame\n");
#define ERROR_SKIP_FRAME -123
            return ERROR_SKIP_FRAME;
        }
    }


        // binary decoder reads 3+2 bits here but they don't seem to be used
        skip_bits(&s->gb, 5);


        av_log(s->avctx, AV_LOG_INFO,
               "num:%5d x:%2d y:%2d type:%d qscale:%2d rnd:%d\n",
               seq, s->mb_x, s->mb_y, s->pict_type, s->qscale,
               s->no_rounding);
    }


}

{

        av_log(avctx, AV_LOG_ERROR, "Extradata is too small.\n");
        return AVERROR_INVALIDDATA;
    }
        return ret;






        }
        break;
    default:
        av_log(s->avctx, AV_LOG_ERROR, "unknown header %X\n", rv->sub_id);
        avpriv_request_sample(avctx, "RV1/2 version");
        return AVERROR_PATCHWELCOME;
    }

        av_log(avctx, AV_LOG_DEBUG, "ver:%X ver0:%"PRIX32"\n", rv->sub_id,
               ((uint32_t *) avctx->extradata)[0]);
    }


        return ret;


    /* init rv vlc */
                        rv_lum_bits, 1, 1,
                        rv_lum_code, 2, 2, 16384);
                        rv_chrom_bits, 1, 1,
                        rv_chrom_code, 2, 2, 16388);
    }

    return 0;
}

{

}

                              int buf_size, int buf_size2, int whole_size)
{

    else
        if (mb_count != ERROR_SKIP_FRAME)
            av_log(s->avctx, AV_LOG_ERROR, "HEADER ERROR\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(s->avctx, AV_LOG_ERROR, "POS ERROR %d %d\n", s->mb_x, s->mb_y);
        return AVERROR_INVALIDDATA;
    }
        av_log(s->avctx, AV_LOG_ERROR, "COUNT ERROR\n");
        return AVERROR_INVALIDDATA;
    }

        return AVERROR_INVALIDDATA;

        // FIXME write parser so we always have complete frames?
            ff_er_frame_end(&s->er);
            ff_mpv_frame_end(s);
            s->mb_x = s->mb_y = s->resync_mb_x = s->resync_mb_y = 0;
        }
            return ret;
    } else {
            av_log(s->avctx, AV_LOG_ERROR, "Slice type mismatch\n");
            return AVERROR_INVALIDDATA;
        }
    }



    /* default quantization values */
    } else {
    }
    } else {
    }




    /* decode each macroblock */


        // Repeat the slice end check from ff_h263_decode_mb with our active
        // bitstream size


                ret = SLICE_END;
        }
            8 * buf_size2 >= get_bits_count(&s->gb)) {
            active_bits_size = buf_size2 * 8;
            av_log(avctx, AV_LOG_DEBUG, "update size from %d to %d\n",
                   8 * buf_size, active_bits_size);
            ret = SLICE_OK;
        }

            av_log(s->avctx, AV_LOG_ERROR, "ERROR at MB %d %d\n", s->mb_x,
                   s->mb_y);
            return AVERROR_INVALIDDATA;
        }

        }
            break;
    }

                    ER_MB_END);

}

{
        return avctx->slice_offset[n];
    else
}

                             AVPacket *avpkt)
{


    /* no supplementary picture */
        return 0;
    }


            av_log(avctx, AV_LOG_ERROR, "Invalid slice count: %d.\n",
                   slice_count);
            return AVERROR_INVALIDDATA;
        }

    } else
        slice_count = avctx->slice_count;


            return AVERROR_INVALIDDATA;

        else

        else

            return AVERROR_INVALIDDATA;

            return ret;

            i++;
    }


                return ret;
                return ret;
        }

        }

        // so we can detect if frame_end was not called (find some nicer solution...)
    }

}

AVCodec ff_rv10_decoder = {
    .name           = "rv10",
    .long_name      = NULL_IF_CONFIG_SMALL("RealVideo 1.0"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_RV10,
    .priv_data_size = sizeof(RVDecContext),
    .init           = rv10_decode_init,
    .close          = rv10_decode_end,
    .decode         = rv10_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
    .caps_internal  = FF_CODEC_CAP_INIT_CLEANUP,
    .max_lowres     = 3,
    .pix_fmts       = (const enum AVPixelFormat[]) {
        AV_PIX_FMT_YUV420P,
        AV_PIX_FMT_NONE
    },
};

AVCodec ff_rv20_decoder = {
    .name           = "rv20",
    .long_name      = NULL_IF_CONFIG_SMALL("RealVideo 2.0"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_RV20,
    .priv_data_size = sizeof(RVDecContext),
    .init           = rv10_decode_init,
    .close          = rv10_decode_end,
    .decode         = rv10_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1 | AV_CODEC_CAP_DELAY,
    .caps_internal  = FF_CODEC_CAP_INIT_CLEANUP,
    .flush          = ff_mpeg_flush,
    .max_lowres     = 3,
    .pix_fmts       = (const enum AVPixelFormat[]) {
        AV_PIX_FMT_YUV420P,
        AV_PIX_FMT_NONE
    },
};
