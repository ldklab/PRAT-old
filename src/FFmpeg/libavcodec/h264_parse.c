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

#include "bytestream.h"
#include "get_bits.h"
#include "golomb.h"
#include "h264.h"
#include "h264dec.h"
#include "h264_parse.h"
#include "h264_ps.h"

                              const int *ref_count, int slice_type_nos,
                              H264PredWeightTable *pwt,
                              int picture_structure, void *logctx)
{


        av_log(logctx, AV_LOG_ERROR, "luma_log2_weight_denom %d is out of range\n", pwt->luma_log2_weight_denom);
        pwt->luma_log2_weight_denom = 0;
    }

            av_log(logctx, AV_LOG_ERROR, "chroma_log2_weight_denom %d is out of range\n", pwt->chroma_log2_weight_denom);
            pwt->chroma_log2_weight_denom = 0;
        }
    }


                    goto out_range_weight;
                    pwt->luma_weight[i][list][1] != 0) {
                }
            } else {
            }

                    int j;
                            pwt->chroma_weight[i][list][j][0] = chroma_def;
                            pwt->chroma_weight[i][list][j][1] = 0;
                            goto out_range_weight;
                        }
                            pwt->chroma_weight[i][list][j][1] != 0) {
                        }
                    }
                } else {
                    int j;
                    }
                }
            }

            // for MBAFF
                    }
                }
            }
        }
            break;
    }
out_range_weight:
    avpriv_request_sample(logctx, "Out of range weight");
    return AVERROR_INVALIDDATA;
}

/**
 * Check if the top & left blocks are available if needed and
 * change the dc mode so it only uses the available blocks.
 */
                                     int top_samples_available, int left_samples_available)
{
        -1, 0, LEFT_DC_PRED, -1, -1, -1, -1, -1, 0
    };
        0, -1, TOP_DC_PRED, 0, -1, -1, -1, 0, -1, DC_128_PRED
    };

                av_log(logctx, AV_LOG_ERROR,
                       "top block unavailable for requested intra mode %d\n",
                       status);
                return AVERROR_INVALIDDATA;
            }
        }
    }

        static const int mask[4] = { 0x8000, 0x2000, 0x80, 0x20 };
                    av_log(logctx, AV_LOG_ERROR,
                           "left block unavailable for requested intra4x4 mode %d\n",
                           status);
                    return AVERROR_INVALIDDATA;
                }
            }
    }

    return 0;
}

/**
 * Check if the top & left blocks are available if needed and
 * change the dc mode so it only uses the available blocks.
 */
                                  int left_samples_available,
                                  int mode, int is_chroma)
{

        av_log(logctx, AV_LOG_ERROR,
               "out of range intra chroma pred mode\n");
        return AVERROR_INVALIDDATA;
    }

            av_log(logctx, AV_LOG_ERROR,
                   "top block unavailable for requested intra mode\n");
            return AVERROR_INVALIDDATA;
        }
    }

            av_log(logctx, AV_LOG_ERROR,
                   "left block unavailable for requested intra mode\n");
            return AVERROR_INVALIDDATA;
        }
            // mad cow disease mode, aka MBAFF + constrained_intra_pred
        }
    }

    return mode;
}

                            GetBitContext *gb, const PPS *pps,
                            int slice_type_nos, int picture_structure, void *logctx)
{

    // set defaults, might be overridden a few lines later



            } else
                // full range is spec-ok in this case, even for frames
        }

            list_count = 2;
        else

            av_log(logctx, AV_LOG_ERROR, "reference overflow %u > %u or %u > %u\n",
                   ref_count[0] - 1, max[0], ref_count[1] - 1, max[1]);
            ref_count[0] = ref_count[1] = 0;
            *plist_count = 0;
            goto fail;
            av_log(logctx, AV_LOG_DEBUG, "reference overflow %u > %u \n",
                   ref_count[1] - 1, max[1]);
            ref_count[1] = 0;
        }

    } else {
    }


fail:
    *plist_count = 0;
    ref_count[0] = 0;
    ref_count[1] = 0;
    return AVERROR_INVALIDDATA;
}

                     const SPS *sps, H264POCContext *pc,
                     int picture_structure, int nal_ref_idc)
{



        else

        else
            abs_frame_num = 0;


            // FIXME integrate during sps parse


        } else
            expectedpoc = 0;



    } else {

            poc--;

    }

        return AVERROR_INVALIDDATA;


}

                               int is_avc, void *logctx)
{

        ret = 0;
        goto fail;
    }

                break;
                   "SPS decoding failure, trying again with the complete NAL\n");
                break;
            ret = ff_h264_decode_seq_parameter_set(&nal->gb, logctx, ps, 1);
            if (ret < 0)
                goto fail;
            break;
        }
                                                       nal->size_bits);
                goto fail;
            break;
        default:
            av_log(logctx, AV_LOG_VERBOSE, "Ignoring NAL type %d in extradata\n",
                   nal->type);
            break;
        }
    }

}

/* There are (invalid) samples in the wild with mp4-style extradata, where the
 * parameter sets are stored unescaped (i.e. as RBSP).
 * This function catches the parameter set decoding failure and tries again
 * after escaping it */
                                   int err_recognition, void *logctx)
{

        GetByteContext gbc;
        PutByteContext pbc;
        uint8_t *escaped_buf;
        int escaped_buf_size;

        av_log(logctx, AV_LOG_WARNING,
               "SPS decoding failure, trying again after escaping the NAL\n");

        if (buf_size / 2 >= (INT16_MAX - AV_INPUT_BUFFER_PADDING_SIZE) / 3)
            return AVERROR(ERANGE);
        escaped_buf_size = buf_size * 3 / 2 + AV_INPUT_BUFFER_PADDING_SIZE;
        escaped_buf = av_mallocz(escaped_buf_size);
        if (!escaped_buf)
            return AVERROR(ENOMEM);

        bytestream2_init(&gbc, buf, buf_size);
        bytestream2_init_writer(&pbc, escaped_buf, escaped_buf_size);

        while (bytestream2_get_bytes_left(&gbc)) {
            if (bytestream2_get_bytes_left(&gbc) >= 3 &&
                bytestream2_peek_be24(&gbc) <= 3) {
                bytestream2_put_be24(&pbc, 3);
                bytestream2_skip(&gbc, 2);
            } else
                bytestream2_put_byte(&pbc, bytestream2_get_byte(&gbc));
        }

        escaped_buf_size = bytestream2_tell_p(&pbc);
        AV_WB16(escaped_buf, escaped_buf_size - 2);

        (void)decode_extradata_ps(escaped_buf, escaped_buf_size, ps, 1, logctx);
        // lorex.mp4 decodes ok even with extradata decoding failing
        av_freep(&escaped_buf);
    }

    return 0;
}

                             int *is_avc, int *nal_length_size,
                             int err_recognition, void *logctx)
{

        return -1;



            av_log(logctx, AV_LOG_ERROR, "avcC %d too short\n", size);
            return AVERROR_INVALIDDATA;
        }

        // Decode sps from avcC
                return AVERROR_INVALIDDATA;
                av_log(logctx, AV_LOG_ERROR,
                       "Decoding sps %d from avcC failed\n", i);
                return ret;
            }
        }
        // Decode pps from avcC
                return AVERROR_INVALIDDATA;
                av_log(logctx, AV_LOG_ERROR,
                       "Decoding pps %d from avcC failed\n", i);
                return ret;
            }
        }
        // Store right nal length size that will be used to parse all other nals
    } else {
            return ret;
    }
    return size;
}

/**
 * Compute profile from profile_idc and constraint_set?_flags.
 *
 * @param sps SPS
 *
 * @return profile as defined by FF_PROFILE_H264_*
 */
{

        // constraint_set1_flag set to 1
    case FF_PROFILE_H264_HIGH_422:
    case FF_PROFILE_H264_HIGH_444_PREDICTIVE:
        // constraint_set3_flag set to 1
    }

}
