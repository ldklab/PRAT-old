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
#include "h2645_parse.h"
#include "hevc.h"
#include "hevc_parse.h"

                                 HEVCSEI *sei, int is_nalff, int nal_length_size,
                                 int err_recognition, int apply_defdispwin, void *logctx)
{

                                nal_length_size, AV_CODEC_ID_HEVC, 1, 0);
        goto done;
    }

            continue;

        /* ignore everything except parameter sets and VCL NALUs */
                goto done;
            break;
                goto done;
            break;
            break;
        case HEVC_NAL_SEI_SUFFIX:
                goto done;
            break;
        default:
            av_log(logctx, AV_LOG_VERBOSE, "Ignoring NAL type %d in extradata\n", nal->type);
            break;
        }
    }

        return ret;

    return 0;
}

                             HEVCSEI *sei, int *is_nalff, int *nal_length_size,
                             int err_recognition, int apply_defdispwin, void *logctx)
{


        /* It seems the extradata is encoded as hvcC format.
         * Temporarily, we support configurationVersion==0 until 14496-15 3rd
         * is finalized. When finalized, configurationVersion will be 1 and we
         * can recognize hvcC by checking if avctx->extradata[0]==1 or not. */



        /* nal units in the hvcC always have length coded with 2 bytes,
         * so put a fake nal_length_size = 2 while parsing them */

        /* Decode nal units from hvcC. */

                // +2 for the nal size field
                    av_log(logctx, AV_LOG_ERROR,
                           "Invalid NAL unit size in extradata.\n");
                    return AVERROR_INVALIDDATA;
                }

                                            *nal_length_size, err_recognition, apply_defdispwin,
                                            logctx);
                    av_log(logctx, AV_LOG_ERROR,
                           "Decoding nal unit %d %d from hvcC failed\n",
                           type, i);
                    return ret;
                }
            }
        }

        /* Now store right nal length size, that will be used to parse
         * all other nals */
    } else {
                                    err_recognition, apply_defdispwin, logctx);
            return ret;
    }

    return ret;
}
