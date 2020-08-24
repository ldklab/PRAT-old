/*
 * Copyright (c) 2012 Andrew D'Addesio
 * Copyright (c) 2013-2014 Mozilla Corporation
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
 * Opus decoder/parser shared code
 */

#include <stdint.h>

#include "libavutil/error.h"
#include "libavutil/ffmath.h"

#include "opus_celt.h"
#include "opustab.h"
#include "internal.h"
#include "vorbis.h"

static const uint16_t opus_frame_duration[32] = {
    480, 960, 1920, 2880,
    480, 960, 1920, 2880,
    480, 960, 1920, 2880,
    480, 960,
    480, 960,
    120, 240,  480,  960,
    120, 240,  480,  960,
    120, 240,  480,  960,
    120, 240,  480,  960,
};

/**
 * Read a 1- or 2-byte frame length
 */
{

        return AVERROR_INVALIDDATA;
            return AVERROR_INVALIDDATA;
    }
    return val;
}

/**
 * Read a multi-byte length (used for code 3 packet padding size)
 */
static inline int xiph_lacing_full(const uint8_t **ptr, const uint8_t *end)
{
    int val = 0;

            return AVERROR_INVALIDDATA;
            break;
        else
    }
    return val;
}

/**
 * Parse Opus packet info from raw packet data
 */
                         int self_delimiting)
{

        goto fail;

    /* TOC byte */

    /* code 2 and code 3 packets have at least 1 byte after the TOC */
        goto fail;

        /* 1 frame */

                goto fail;
        }

            goto fail;
        /* 2 frames, equal size */

            int len = xiph_lacing_16bit(&ptr, end);
            if (len < 0 || 2 * len > end - ptr)
                goto fail;
            end      = ptr + 2 * len;
            buf_size = end - buf;
        }

            goto fail;
        /* 2 frames, different sizes */

        /* read 1st frame size */
            goto fail;

            int len = xiph_lacing_16bit(&ptr, end);
            if (len < 0 || len + frame_bytes > end - ptr)
                goto fail;
            end      = ptr + frame_bytes + len;
            buf_size = end - buf;
        }


        /* calculate 2nd frame size */
            goto fail;
        /* 1 to 48 frames, can be different sizes */

            goto fail;

        /* read padding size */
                goto fail;
        }

        /* read frame sizes */
            /* for VBR, all frames except the final one have their size coded
               in the bitstream. the last frame size is implicit. */
            int total_bytes = 0;
                    goto fail;
            }

                int len = xiph_lacing_16bit(&ptr, end);
                if (len < 0 || len + total_bytes + padding > end - ptr)
                    goto fail;
                end      = ptr + total_bytes + len + padding;
                buf_size = end - buf;
            }

                goto fail;
        } else {
            /* for CBR, the remaining packet bytes are divided evenly between
               the frames */
                frame_bytes = xiph_lacing_16bit(&ptr, end);
                if (frame_bytes < 0 || pkt->frame_count * frame_bytes + padding > end - ptr)
                    goto fail;
                end      = ptr + pkt->frame_count * frame_bytes + padding;
                buf_size = end - buf;
            } else {
                    goto fail;
                frame_bytes /= pkt->frame_count;
            }

            }
        }
    }


    /* total packet duration cannot be larger than 120ms */
        goto fail;

    /* set mode and bandwidth */
    } else {
        /* skip medium band */
    }

    return 0;

fail:
    memset(pkt, 0, sizeof(*pkt));
    return AVERROR_INVALIDDATA;
}

{
}

{
}

                                    OpusContext *s)
{



            av_log(avctx, AV_LOG_ERROR,
                   "Multichannel configuration without extradata.\n");
            return AVERROR(EINVAL);
        }
        extradata      = opus_default_extradata;
        extradata_size = sizeof(opus_default_extradata);
    } else {
    }

        av_log(avctx, AV_LOG_ERROR, "Invalid extradata size: %d\n",
               extradata_size);
        return AVERROR_INVALIDDATA;
    }

        avpriv_request_sample(avctx, "Extradata version %d", version);
        return AVERROR_PATCHWELCOME;
    }


        av_log(avctx, AV_LOG_ERROR, "Zero channel count specified in the extradata\n");
        return AVERROR_INVALIDDATA;
    }

        s->gain = ff_exp10(s->gain_i / (20.0 * 256));

            av_log(avctx, AV_LOG_ERROR,
                   "Channel mapping 0 is only specified for up to 2 channels\n");
            return AVERROR_INVALIDDATA;
        }
            av_log(avctx, AV_LOG_ERROR, "Invalid extradata size: %d\n",
                   extradata_size);
            return AVERROR_INVALIDDATA;
        }

            av_log(avctx, AV_LOG_ERROR,
                   "Invalid stream/stereo stream count: %d/%d\n", streams, stereo_streams);
            return AVERROR_INVALIDDATA;
        }

                av_log(avctx, AV_LOG_ERROR,
                       "Channel mapping 1 is only specified for up to 8 channels\n");
                return AVERROR_INVALIDDATA;
            }
        } else if (map_type == 2) {
            int ambisonic_order = ff_sqrt(channels) - 1;
            if (channels != ((ambisonic_order + 1) * (ambisonic_order + 1)) &&
                channels != ((ambisonic_order + 1) * (ambisonic_order + 1) + 2)) {
                av_log(avctx, AV_LOG_ERROR,
                       "Channel mapping 2 is only specified for channel counts"
                       " which can be written as (n + 1)^2 or (n + 1)^2 + 2"
                       " for nonnegative integer n\n");
                return AVERROR_INVALIDDATA;
            }
            if (channels > 227) {
                av_log(avctx, AV_LOG_ERROR, "Too many channels\n");
                return AVERROR_INVALIDDATA;
            }
            layout = 0;
        } else
            layout = 0;

    } else {
        avpriv_request_sample(avctx, "Mapping type %d", map_type);
        return AVERROR_PATCHWELCOME;
    }

        return AVERROR(ENOMEM);


            map->silence = 1;
            continue;
            av_log(avctx, AV_LOG_ERROR,
                   "Invalid channel map for output channel %d: %d\n", i, idx);
            av_freep(&s->channel_maps);
            return AVERROR_INVALIDDATA;
        }

        /* check that we did not see this index yet */
                map->copy     = 1;
                map->copy_idx = j;
                break;
            }

        } else {
        }
    }


}

{






        /* Compute how many bits we want to allocate to this band */
        }


            /* Special Hybrid Folding (RFC 8251 section 9). Copy the first band into
            the second to ensure the second band never has to use the LCG. */


        }

        /* Get a conservative estimate of the collapse_mask's for the bands we're
           going to be folding from. */

            /* This ensures we never repeat spectral content within one band */
                                      ff_celt_freq_bands[lowband_offset] - ff_celt_freq_range[i]);

            cm[0] = cm[1] = 0;
            }
        }

            /* Switch off dual stereo to do intensity */
        }


                                       f->blocks, norm_loc1, f->size,
                                       norm1 + band_offset, 0, 1.0f,
                                       lowband_scratch, cm[0]);

                                       norm2 + band_offset, 0, 1.0f,
                                       lowband_scratch, cm[1]);
        } else {
                                       f->blocks, norm_loc1, f->size,
                                       norm1 + band_offset, 0, 1.0f,
        }


        /* Update the folding position only as long as we have 1 bit/sample depth */
    }

#define NORMC(bits) ((bits) << (f->channels - 1) << f->size >> 2)

{


    /* Spread */
            ff_opus_rc_enc_cdf(rc, f->spread, ff_celt_model_spread);
        else
    } else {
    }

    /* Initialize static allocation caps */

    /* Band boosts */

                is_boost = boost_amount--;
                ff_opus_rc_enc_log(rc, is_boost, b_dynalloc);
            } else {
            }

                break;


        }

    }

    /* Allocation trim */
            ff_opus_rc_enc_cdf(rc, f->alloc_trim, ff_celt_model_alloc_trim);
        else

    /* Anti-collapse bit reservation */

    /* Band skip bit reservation */

    /* Intensity/dual stereo bit reservation */
            }
        } else {
            intensitystereo_bit = 0;
        }
    }

    /* Trim offsets */

        /* PVQ minimum allocation threshold, below this value the band is
         * skipped */
                             f->channels << 3);


    }

    /* Bisection */
    low  = 1;
    high = CELT_VECTORS - 1;



            }
        }

        else
    }

    /* Bisection */



    }

    /* Bisection */
    low  = 0;
    high = 1 << CELT_ALLOC_STEPS;


        }
            high = center;
        else
    }

    /* Bisection */

            done = 1;
        else

    }

    /* Band skipping */

            /* all remaining bands are not skipped */
        }

        /* determine the number of bits available for coding "do not skip" markers */

        /* a "do not skip" marker is only coded if the allocation is
         * above the chosen threshold */
                do_not_skip = f->coded_bands <= f->skip_band_floor;
                ff_opus_rc_enc_log(rc, do_not_skip, 1);
            } else {
            }

                break;

        }

        /* the band is skipped, so reclaim its bits */
        }

    }

    /* IS start band */
        if (intensitystereo_bit) {
            f->intensity_stereo = FFMIN(f->intensity_stereo, f->coded_bands);
            ff_opus_rc_enc_uint(rc, f->intensity_stereo, f->coded_bands + 1 - f->start_band);
        }
    } else {
    }

    /* DS flag */
            ff_opus_rc_enc_log(rc, f->dual_stereo, 1);
        else

    /* Supply the remaining bits in this frame to lower bands */
    }

    /* Finally determine the allocation */

                             * extra bits assigned over the standard
                             * totalbits/dof */


            /* intensity stereo makes use of an extra degree of freedom */

            /* grant an additional bias for the first and second pulses */


            /* If fine_bits was rounded down or capped,
             * give priority for the final fine energy pass */

            /* the remaining bits are assigned to PVQ */
        } else {
            /* all bits go to fine energy except for the sign bit */
        }

        /* hand back a limited number of extra fine energy bits to this band */
                                  CELT_MAX_FINE_BITS - f->fine_bits[i]);

        }
    }

    /* skipped bands dedicate all of their bits for fine energy */
    }
