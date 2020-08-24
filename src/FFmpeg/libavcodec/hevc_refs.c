/*
 * HEVC video decoder
 *
 * Copyright (C) 2012 - 2013 Guillaume Martres
 * Copyright (C) 2012 - 2013 Gildas Cocherel
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

#include "libavutil/avassert.h"
#include "libavutil/pixdesc.h"

#include "internal.h"
#include "thread.h"
#include "hevc.h"
#include "hevcdec.h"

{
    /* frame->frame can be NULL if context init failed */
        return;





    }
}

{
}

{
                            HEVC_FRAME_FLAG_SHORT_REF |
                            HEVC_FRAME_FLAG_LONG_REF);

void ff_hevc_flush_dpb(HEVCContext *s)
{
    int i;
    for (i = 0; i < FF_ARRAY_ELEMS(s->DPB); i++)
        ff_hevc_unref_frame(s, &s->DPB[i], ~0);
}

{

                                   AV_GET_BUFFER_FLAG_REF);
            return NULL;

            goto fail;

            goto fail;

            goto fail;


            const AVHWAccel *hwaccel = s->avctx->hwaccel;
            av_assert0(!frame->hwaccel_picture_private);
            if (hwaccel->frame_priv_data_size) {
                frame->hwaccel_priv_buf = av_buffer_allocz(hwaccel->frame_priv_data_size);
                if (!frame->hwaccel_priv_buf)
                    goto fail;
                frame->hwaccel_picture_private = frame->hwaccel_priv_buf->data;
            }
        }

        return frame;
fail:
        ff_hevc_unref_frame(s, frame, ~0);
        return NULL;
    }
    av_log(s->avctx, AV_LOG_ERROR, "Error allocating frame, DPB full.\n");
    return NULL;
}

{

    /* check that this POC doesn't already exist */

            av_log(s->avctx, AV_LOG_ERROR, "Duplicate POC in a sequence: %d.\n",
                   poc);
            return AVERROR_INVALIDDATA;
        }
    }

        return AVERROR(ENOMEM);


    else


}

{

                }
            }
        }

                }
            }
        }

        /* wait for more frames before output */
            return 0;


            else
                return ret;

                   "Output frame with POC %d.\n", frame->poc);
        }

        else
            break;

    return 0;
}

{

        }
    }

                    min_poc = frame->poc;
                }
            }
        }

            }
        }

    }

{

        return AVERROR_INVALIDDATA;



}

{


        return ret;

        av_log(s->avctx, AV_LOG_ERROR, "Zero refs in the frame RPS.\n");
        return AVERROR_INVALIDDATA;
    }


        /* The order of the elements is
         * ST_CURR_BEF - ST_CURR_AFT - LT_CURR for the L0 and
         * ST_CURR_AFT - ST_CURR_BEF - LT_CURR for the L1 */
                              LT_CURR };

        /* concatenate the candidate lists for the current frame */
                }
            }
        }

        /* reorder the references if necessary */

                    av_log(s->avctx, AV_LOG_ERROR, "Invalid reference index.\n");
                    return AVERROR_INVALIDDATA;
                }

            }
        } else {
        }

    }

    return 0;
}

{

        }
    }

               "Could not find ref with POC %d\n", poc);
    return NULL;
}

{

{

        return NULL;

        } else {
                }
        }
    }


        ff_thread_report_progress(&frame->tf, INT_MAX, 0);

    return frame;
}

/* add a reference with the given poc to the list and mark it as used in DPB */
                             int poc, int ref_flag, uint8_t use_msb)
{

        return AVERROR_INVALIDDATA;

            return AVERROR(ENOMEM);
    }


}

{

    }

    /* clear the reference flags on all frames except the current one */


    }


    /* add the short refs */

            list = ST_FOLL;
            list = ST_CURR_BEF;
        else

            goto fail;
    }

    /* add the long refs */

            goto fail;
    }

    /* release any frames that are now unused */

    return ret;
}

{

    }

    if (long_rps) {
    }
}
