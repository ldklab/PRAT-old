/*
 * generic encoding-related code
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

#include "libavutil/attributes.h"
#include "libavutil/avassert.h"
#include "libavutil/frame.h"
#include "libavutil/imgutils.h"
#include "libavutil/internal.h"
#include "libavutil/samplefmt.h"

#include "avcodec.h"
#include "encode.h"
#include "frame_thread_encoder.h"
#include "internal.h"

{
        av_log(avctx, AV_LOG_ERROR, "Invalid minimum required packet size %"PRId64" (max allowed is %d)\n",
               size, INT_MAX - AV_INPUT_BUFFER_PADDING_SIZE);
        return AVERROR(EINVAL);
    }


    }

            av_log(avctx, AV_LOG_ERROR, "Failed to allocate packet of size %"PRId64"\n", size);
    }

    return 0;
}

/**
 * Pad last frame with silence.
 */
{

        goto fail;

        goto fail;

                               src->nb_samples, s->channels, s->sample_fmt)) < 0)
        goto fail;
                                      s->channels, s->sample_fmt)) < 0)
        goto fail;

    return 0;

fail:
    av_frame_unref(frame);
    return ret;
}

                            const AVSubtitle *sub)
{
        av_log(avctx, AV_LOG_ERROR, "start_display_time must be 0.\n");
        return -1;
    }

}

{

        return AVERROR_EOF;

        return AVERROR(EAGAIN);


}

{

        return AVERROR_EOF;

            return ret;
    }

            return AVERROR_EOF;

        // Flushing is signaled with a NULL frame
        frame = NULL;
    }



        avci->frame_thread_encoder && (avctx->active_thread_type & FF_THREAD_FRAME))
        ret = ff_thread_video_encode_frame(avctx, avpkt, frame, &got_packet);
    else {
    }



                goto end;
        }

            }
        }
            /* NOTE: if we add any audio encoders which output non-keyframe packets,
             *       this needs to be moved to the encoders, but for now we can do it
             *       here to simplify things */
        }
    }



    }

        // Encoders must always return ref-counted buffers.
        // Side-data only packets have no data and can be not ref-counted.

    return ret;
}

static int encode_simple_receive_packet(AVCodecContext *avctx, AVPacket *avpkt)
{
    int ret;

            return ret;
    }

    return 0;
}

{

        return AVERROR_EOF;


            avctx->stats_out[0] = '\0';
            return AVERROR(EINVAL);
    }

        ret = avctx->codec->receive_packet(avctx, avpkt);
        if (!ret)
            // Encoders must always return ref-counted buffers.
            // Side-data only packets have no data and can be not ref-counted.
            av_assert0(!avpkt->data || avpkt->buf);
    } else


    return ret;
}

{

        /* extract audio service type metadata */
            avctx->audio_service_type = *(enum AVAudioServiceType*)sd->data;

        /* check for valid frame size */
                av_log(avctx, AV_LOG_ERROR, "more samples than frame size\n");
                return AVERROR(EINVAL);
            }
            /* if we already got an undersized frame, that must have been the last */
                av_log(avctx, AV_LOG_ERROR, "frame_size (%d) was not respected for a non-last frame\n", avctx->frame_size);
                return AVERROR(EINVAL);
            }

                    return ret;

                av_log(avctx, AV_LOG_ERROR, "nb_samples (%d) != frame_size (%d)\n", src->nb_samples, avctx->frame_size);
                return AVERROR(EINVAL);
            }
        }
    }

             return ret;
    }

    return 0;
}

{

        return AVERROR(EINVAL);

        return AVERROR_EOF;

        return AVERROR(EAGAIN);

    } else {
            return ret;
    }

            return ret;
    }

    return 0;
}

{


        return AVERROR(EINVAL);

    } else {
            return ret;
    }

    return 0;
}

                         int *got_packet, const AVFrame *frame)
{


        if (frame->format == AV_PIX_FMT_NONE)
            av_log(avctx, AV_LOG_WARNING, "AVFrame.format is not set\n");
        if (frame->width == 0 || frame->height == 0)
            av_log(avctx, AV_LOG_WARNING, "AVFrame.width or height is not set\n");
    }

        ret = 0;
        /* we fully drain all the output in each encode call, so this should not
         * ever happen */
        return AVERROR_BUG;
        return ret;

            goto finish;
        }

                if (user_pkt.size >= avpkt->size) {
                    memcpy(user_pkt.data, avpkt->data, avpkt->size);
                    av_buffer_unref(&avpkt->buf);
                    avpkt->buf  = user_pkt.buf;
                    avpkt->data = user_pkt.data;
                    av_init_packet(&user_pkt);
                } else {
                    av_log(avctx, AV_LOG_ERROR, "Provided packet is too small, needs to be %d\n", avpkt->size);
                    av_packet_unref(avpkt);
                    ret = AVERROR(EINVAL);
                    goto finish;
                }
            }

        } else {
            if (!avci->compat_decode_warned) {
                av_log(avctx, AV_LOG_WARNING, "The deprecated avcodec_encode_* "
                       "API cannot return all the packets for this encoder. "
                       "Some packets will be dropped. Update your code to the "
                       "new encoding API to fix this.\n");
                avci->compat_decode_warned = 1;
                av_packet_unref(avpkt);
            }
        }

            break;
    }

finish:
        av_packet_unref(&user_pkt);

    return ret;
}

                                              AVPacket *avpkt,
                                              const AVFrame *frame,
                                              int *got_packet_ptr)
{

        av_packet_unref(avpkt);

}

int attribute_align_arg avcodec_encode_video2(AVCodecContext *avctx,
                                              AVPacket *avpkt,
                                              const AVFrame *frame,
                                              int *got_packet_ptr)
{
    int ret = compat_encode(avctx, avpkt, got_packet_ptr, frame);

    if (ret < 0)
        av_packet_unref(avpkt);

    return ret;
}
