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

/**
 * @file
 * Frame multithreading support functions
 * @see doc/multithreading.txt
 */

#include "config.h"

#include <stdatomic.h>
#include <stdint.h>

#include "avcodec.h"
#include "hwconfig.h"
#include "internal.h"
#include "pthread_internal.h"
#include "thread.h"
#include "version.h"

#include "libavutil/avassert.h"
#include "libavutil/buffer.h"
#include "libavutil/common.h"
#include "libavutil/cpu.h"
#include "libavutil/frame.h"
#include "libavutil/internal.h"
#include "libavutil/log.h"
#include "libavutil/mem.h"
#include "libavutil/opt.h"
#include "libavutil/thread.h"

enum {
    ///< Set when the thread is awaiting a packet.
    STATE_INPUT_READY,
    ///< Set before the codec has called ff_thread_finish_setup().
    STATE_SETTING_UP,
    /**
     * Set when the codec calls get_buffer().
     * State is returned to STATE_SETTING_UP afterwards.
     */
    STATE_GET_BUFFER,
     /**
      * Set when the codec calls get_format().
      * State is returned to STATE_SETTING_UP afterwards.
      */
    STATE_GET_FORMAT,
    ///< Set after the codec has called ff_thread_finish_setup().
    STATE_SETUP_FINISHED,
};

/**
 * Context used by codec threads and stored in their AVCodecInternal thread_ctx.
 */
typedef struct PerThreadContext {
    struct FrameThreadContext *parent;

    pthread_t      thread;
    int            thread_init;
    pthread_cond_t input_cond;      ///< Used to wait for a new packet from the main thread.
    pthread_cond_t progress_cond;   ///< Used by child threads to wait for progress to change.
    pthread_cond_t output_cond;     ///< Used by the main thread to wait for frames to finish.

    pthread_mutex_t mutex;          ///< Mutex used to protect the contents of the PerThreadContext.
    pthread_mutex_t progress_mutex; ///< Mutex used to protect frame progress values and progress_cond.

    AVCodecContext *avctx;          ///< Context used to decode packets passed to this thread.

    AVPacket       avpkt;           ///< Input packet (for decoding) or output (for encoding).

    AVFrame *frame;                 ///< Output frame (for decoding) or input (for encoding).
    int     got_frame;              ///< The output of got_picture_ptr from the last avcodec_decode_video() call.
    int     result;                 ///< The result of the last codec decode/encode() call.

    atomic_int state;

    /**
     * Array of frames passed to ff_thread_release_buffer().
     * Frames are released after all threads referencing them are finished.
     */
    AVFrame **released_buffers;
    int   num_released_buffers;
    int       released_buffers_allocated;

    AVFrame *requested_frame;       ///< AVFrame the codec passed to get_buffer()
    int      requested_flags;       ///< flags passed to get_buffer() for requested_frame

    const enum AVPixelFormat *available_formats; ///< Format array for get_format()
    enum AVPixelFormat result_format;            ///< get_format() result

    int die;                        ///< Set when the thread should exit.

    int hwaccel_serializing;
    int async_serializing;

    atomic_int debug_threads;       ///< Set if the FF_DEBUG_THREADS option is set.
} PerThreadContext;

/**
 * Context stored in the client AVCodecInternal thread_ctx.
 */
typedef struct FrameThreadContext {
    PerThreadContext *threads;     ///< The contexts for each thread.
    PerThreadContext *prev_thread; ///< The last thread submit_packet() was called on.

    pthread_mutex_t buffer_mutex;  ///< Mutex used to protect get/release_buffer().
    /**
     * This lock is used for ensuring threads run in serial when hwaccel
     * is used.
     */
    pthread_mutex_t hwaccel_mutex;
    pthread_mutex_t async_mutex;
    pthread_cond_t async_cond;
    int async_lock;

    int next_decoding;             ///< The next context to submit a packet to.
    int next_finished;             ///< The next context to return output from.

    int delaying;                  /**<
                                    * Set for the first N packets, where N is the number of threads.
                                    * While it is set, ff_thread_en/decode_frame won't return any results.
                                    */
} FrameThreadContext;

#define THREAD_SAFE_CALLBACKS(avctx) \
((avctx)->thread_safe_callbacks || (avctx)->get_buffer2 == avcodec_default_get_buffer2)

{
        pthread_cond_wait(&fctx->async_cond, &fctx->async_mutex);

{

/**
 * Codec worker thread.
 *
 * Automatically calls ff_thread_finish_setup() if the codec does
 * not provide an update_thread_context method, or if the codec returns
 * before calling it.
 */
{




        /* If a decoder supports hwaccel, then it must call ff_get_format().
         * Since that call must happen before ff_thread_finish_setup(), the
         * decoder is required to implement update_thread_context() and call
         * ff_thread_finish_setup() manually. Therefore the above
         * ff_thread_finish_setup() call did not happen and hwaccel_serializing
         * cannot be true here. */

        /* if the previous thread uses hwaccel then we take the lock to ensure
         * the threads don't run concurrently */
            pthread_mutex_lock(&p->parent->hwaccel_mutex);
            p->hwaccel_serializing = 1;
        }


            if (avctx->codec->caps_internal & FF_CODEC_CAP_ALLOCATE_PROGRESS)
                av_log(avctx, AV_LOG_ERROR, "A frame threaded decoder did not "
                       "free the frame on failure. This is a bug, please report it.\n");
            av_frame_unref(p->frame);
        }


            p->hwaccel_serializing = 0;
            pthread_mutex_unlock(&p->parent->hwaccel_mutex);
        }

            p->async_serializing = 0;

            async_unlock(p->parent);
        }



    }

}

/**
 * Update the next thread's AVCodecContext with values from the reference thread's context.
 *
 * @param dst The destination context.
 * @param src The source context.
 * @param for_user 0 if the destination is a codec thread, 1 if the destination is the user's thread
 * @return 0 on success, negative error code on failure
 */
{










            (dst->hw_frames_ctx && dst->hw_frames_ctx->data != src->hw_frames_ctx->data)) {
            av_buffer_unref(&dst->hw_frames_ctx);

            if (src->hw_frames_ctx) {
                dst->hw_frames_ctx = av_buffer_ref(src->hw_frames_ctx);
                if (!dst->hw_frames_ctx)
                    return AVERROR(ENOMEM);
            }
        }



                    return AVERROR(ENOMEM);
            }
        }
    }

#if FF_API_CODED_FRAME
FF_DISABLE_DEPRECATION_WARNINGS
FF_ENABLE_DEPRECATION_WARNINGS
#endif
    } else {
    }

    return err;
}

/**
 * Update the next thread's AVCodecContext with values set by the user.
 *
 * @param dst The destination context.
 * @param src The source context.
 * @return 0 on success, negative error code on failure
 */
{






        if (dst->slice_count < src->slice_count) {
            int err = av_reallocp_array(&dst->slice_offset, src->slice_count,
                                        sizeof(*dst->slice_offset));
            if (err < 0)
                return err;
        }
               src->slice_count * sizeof(*dst->slice_offset));
    }
}

/// Releases the buffers that this decoding thread was the last user of.
{

        AVFrame *f;

        pthread_mutex_lock(&fctx->buffer_mutex);

        // fix extended data in case the caller screwed it up
        av_assert0(p->avctx->codec_type == AVMEDIA_TYPE_VIDEO ||
                   p->avctx->codec_type == AVMEDIA_TYPE_AUDIO);
        f = p->released_buffers[--p->num_released_buffers];
        f->extended_data = f->data;
        av_frame_unref(f);

        pthread_mutex_unlock(&fctx->buffer_mutex);
    }

                         AVPacket *avpkt)
{

        return 0;


        pthread_mutex_unlock(&p->mutex);
        return ret;
    }
                          (p->avctx->debug & FF_DEBUG_THREADS) != 0,
                          memory_order_relaxed);


        }

            pthread_mutex_unlock(&p->mutex);
            return err;
        }
    }

        pthread_mutex_unlock(&p->mutex);
        av_log(p->avctx, AV_LOG_ERROR, "av_packet_ref() failed in submit_packet()\n");
        return ret;
    }


    /*
     * If the client doesn't have a thread-safe get_buffer(),
     * then decoding threads call back to the main thread,
     * and it calls back to the client here.
     */

        while (atomic_load(&p->state) != STATE_SETUP_FINISHED && atomic_load(&p->state) != STATE_INPUT_READY) {
            int call_done = 1;
            pthread_mutex_lock(&p->progress_mutex);
            while (atomic_load(&p->state) == STATE_SETTING_UP)
                pthread_cond_wait(&p->progress_cond, &p->progress_mutex);

            switch (atomic_load_explicit(&p->state, memory_order_acquire)) {
            case STATE_GET_BUFFER:
                p->result = ff_get_buffer(p->avctx, p->requested_frame, p->requested_flags);
                break;
            case STATE_GET_FORMAT:
                p->result_format = ff_get_format(p->avctx, p->available_formats);
                break;
            default:
                call_done = 0;
                break;
            }
            if (call_done) {
                atomic_store(&p->state, STATE_SETTING_UP);
                pthread_cond_signal(&p->progress_cond);
            }
            pthread_mutex_unlock(&p->progress_mutex);
        }
    }


}

                           AVFrame *picture, int *got_picture_ptr,
                           AVPacket *avpkt)
{

    /* release the async lock, permitting blocked hwaccel threads to
     * go forward while we are in this function */

    /*
     * Submit a packet to the next decoding thread.
     */

        goto finish;

    /*
     * If we're still receiving the initial packets, don't return a frame.
     */


        }
    }

    /*
     * Return the next available frame from the oldest thread.
     * If we're at the end of the stream, then we have to skip threads that
     * didn't output a frame/error, because we don't want to accidentally signal
     * EOF (avpkt->size == 0 && *got_picture_ptr == 0 && err >= 0).
     */


        }


        /*
         * A later call with avkpt->size == 0 may loop over all threads,
         * including this one, searching for a frame/error to return before being
         * stopped by the "finished != fctx->next_finished" condition.
         * Make sure we don't mistakenly return the same frame/error again.
         */





    /* return the size of the consumed packet if no error occurred */
finish:
}

{



        av_log(f->owner[field], AV_LOG_DEBUG,
               "%p finished %d field %d\n", progress, n, field);



}

{



        av_log(f->owner[field], AV_LOG_DEBUG,
               "thread awaiting %d field %d from %p\n", n, field, progress);

}



        pthread_mutex_lock(&p->parent->hwaccel_mutex);
        p->hwaccel_serializing = 1;
    }

    /* this assumes that no hwaccel calls happen before ff_thread_finish_setup() */
        !(avctx->hwaccel->caps_internal & HWACCEL_CAP_ASYNC_SAFE)) {
        p->async_serializing = 1;

        async_lock(p->parent);
    }

        av_log(avctx, AV_LOG_WARNING, "Multiple ff_thread_finish_setup() calls\n");
    }


}

/// Waits for all threads to finish.
{



            pthread_mutex_lock(&p->progress_mutex);
            while (atomic_load(&p->state) != STATE_INPUT_READY)
                pthread_cond_wait(&p->output_cond, &p->progress_mutex);
            pthread_mutex_unlock(&p->progress_mutex);
        }
    }


{


        if (update_context_from_thread(avctx, fctx->prev_thread->avctx, 1) < 0) {
            av_log(avctx, AV_LOG_ERROR, "Failed to update user thread.\n");
        }
    }

            av_log(avctx, AV_LOG_ERROR, "Final thread update failed\n");
            fctx->prev_thread->avctx->internal->is_copy = fctx->threads->avctx->internal->is_copy;
            fctx->threads->avctx->internal->is_copy = 1;
        }





    }



            av_frame_free(&p->released_buffers[j]);


        }

        }

    }




{

#if FF_API_DEBUG_MV
        if ((avctx->debug & (FF_DEBUG_VIS_QP | FF_DEBUG_VIS_MB_TYPE)) || avctx->debug_mv)
            nb_cpus = 1;
#endif
        // use number of cores + 1 as thread count if there is more than one
        else
            thread_count = avctx->thread_count = 1;
    }

        avctx->active_thread_type = 0;
        return 0;
    }

        return AVERROR(ENOMEM);

        av_freep(&avctx->internal->thread_ctx);
        return AVERROR(ENOMEM);
    }






            av_freep(&copy);
            err = AVERROR(ENOMEM);
            goto error;
        }


            err = AVERROR(ENOMEM);
            goto error;
        }


            copy->priv_data = NULL;
            err = AVERROR(ENOMEM);
            goto error;
        }


                err = AVERROR(ENOMEM);
                goto error;
            }

                    goto error;
            }
        }






            goto error;
    }

    return 0;

error:
    ff_frame_thread_free(avctx, i+1);

    return err;
}

void ff_thread_flush(AVCodecContext *avctx)
{
    int i;
    FrameThreadContext *fctx = avctx->internal->thread_ctx;

    if (!fctx) return;

    park_frame_worker_threads(fctx, avctx->thread_count);
    if (fctx->prev_thread) {
        if (fctx->prev_thread != &fctx->threads[0])
            update_context_from_thread(fctx->threads[0].avctx, fctx->prev_thread->avctx, 0);
    }

    fctx->next_decoding = fctx->next_finished = 0;
    fctx->delaying = 1;
    fctx->prev_thread = NULL;
    for (i = 0; i < avctx->thread_count; i++) {
        PerThreadContext *p = &fctx->threads[i];
        // Make sure decode flush calls with size=0 won't return old frames
        p->got_frame = 0;
        av_frame_unref(p->frame);
        p->result = 0;

        release_delayed_buffers(p);

        if (avctx->codec->flush)
            avctx->codec->flush(p->avctx);
    }
}

{
        (avctx->codec->update_thread_context || !THREAD_SAFE_CALLBACKS(avctx))) {
        return 0;
    }
    return 1;
}

{



        av_log(avctx, AV_LOG_ERROR, "get_buffer() cannot be called after ff_thread_finish_setup()\n");
        return -1;
    }

            return AVERROR(ENOMEM);
        }

    }

    } else {
        pthread_mutex_lock(&p->progress_mutex);
        p->requested_frame = f->f;
        p->requested_flags = flags;
        atomic_store_explicit(&p->state, STATE_GET_BUFFER, memory_order_release);
        pthread_cond_broadcast(&p->progress_cond);

        while (atomic_load(&p->state) != STATE_SETTING_UP)
            pthread_cond_wait(&p->progress_cond, &p->progress_mutex);

        err = p->result;

        pthread_mutex_unlock(&p->progress_mutex);

    }
        ff_thread_finish_setup(avctx);
        av_buffer_unref(&f->progress);


}

{
        avctx->get_format == avcodec_default_get_format)
    if (atomic_load(&p->state) != STATE_SETTING_UP) {
        av_log(avctx, AV_LOG_ERROR, "get_format() cannot be called after ff_thread_finish_setup()\n");
        return -1;
    }
    pthread_mutex_lock(&p->progress_mutex);
    p->available_formats = fmt;
    atomic_store(&p->state, STATE_GET_FORMAT);
    pthread_cond_broadcast(&p->progress_cond);

    while (atomic_load(&p->state) != STATE_SETTING_UP)
        pthread_cond_wait(&p->progress_cond, &p->progress_mutex);

    res = p->result_format;

    pthread_mutex_unlock(&p->progress_mutex);

    return res;
}

{
        av_log(avctx, AV_LOG_ERROR, "thread_get_buffer() failed\n");
}

{

        return;

        av_log(avctx, AV_LOG_DEBUG, "thread_release_buffer called on pic %p\n", f);


    // when the frame buffers are not allocated, just reset it to clean state
    }

    fctx = p->parent;
    pthread_mutex_lock(&fctx->buffer_mutex);

    if (p->num_released_buffers == p->released_buffers_allocated) {
        AVFrame **tmp = av_realloc_array(p->released_buffers, p->released_buffers_allocated + 1,
                                         sizeof(*p->released_buffers));
        if (tmp) {
            tmp[p->released_buffers_allocated] = av_frame_alloc();
            p->released_buffers = tmp;
        }

        if (!tmp || !tmp[p->released_buffers_allocated]) {
            ret = AVERROR(ENOMEM);
            goto fail;
        }
        p->released_buffers_allocated++;
    }

    dst = p->released_buffers[p->num_released_buffers];
    av_frame_move_ref(dst, f->f);

    p->num_released_buffers++;

fail:
    pthread_mutex_unlock(&fctx->buffer_mutex);

    // make sure the frame is clean even if we fail to free it
    // this leaks, but it is better than crashing
    if (ret < 0) {
        av_log(avctx, AV_LOG_ERROR, "Could not queue a frame for freeing, this will leak\n");
        memset(f->f->buf, 0, sizeof(f->f->buf));
        if (f->f->extended_buf)
            memset(f->f->extended_buf, 0, f->f->nb_extended_buf * sizeof(*f->f->extended_buf));
        av_frame_unref(f->f);
    }
}
