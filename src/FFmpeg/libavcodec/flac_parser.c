/*
 * FLAC parser
 * Copyright (c) 2010 Michael Chinen
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
 * FLAC parser
 *
 * The FLAC parser buffers input until FLAC_MIN_HEADERS has been found.
 * Each time it finds and verifies a CRC-8 header it sees which of the
 * FLAC_MAX_SEQUENTIAL_HEADERS that came before it have a valid CRC-16 footer
 * that ends at the newly found header.
 * Headers are scored by FLAC_HEADER_BASE_SCORE plus the max of its crc-verified
 * children, penalized by changes in sample rate, frame number, etc.
 * The parser returns the frame with the highest score.
 **/

#include "libavutil/attributes.h"
#include "libavutil/crc.h"
#include "libavutil/fifo.h"
#include "bytestream.h"
#include "parser.h"
#include "flac.h"

/** maximum number of adjacent headers that compare CRCs against each other   */
#define FLAC_MAX_SEQUENTIAL_HEADERS 4
/** minimum number of headers buffered and checked before returning frames    */
#define FLAC_MIN_HEADERS 10
/** estimate for average size of a FLAC frame                                 */
#define FLAC_AVG_FRAME_SIZE 8192

/** scoring settings for score_header */
#define FLAC_HEADER_BASE_SCORE        10
#define FLAC_HEADER_CHANGED_PENALTY   7
#define FLAC_HEADER_CRC_FAIL_PENALTY  50
#define FLAC_HEADER_NOT_PENALIZED_YET 100000
#define FLAC_HEADER_NOT_SCORED_YET    -100000

/** largest possible size of flac header */
#define MAX_FRAME_HEADER_SIZE 16

typedef struct FLACHeaderMarker {
    int offset;       /**< byte offset from start of FLACParseContext->buffer */
    int link_penalty[FLAC_MAX_SEQUENTIAL_HEADERS]; /**< array of local scores
                           between this header and the one at a distance equal
                           array position                                     */
    int max_score;    /**< maximum score found after checking each child that
                           has a valid CRC                                    */
    FLACFrameInfo fi; /**< decoded frame header info                          */
    struct FLACHeaderMarker *next;       /**< next CRC-8 verified header that
                                              immediately follows this one in
                                              the bytestream                  */
    struct FLACHeaderMarker *best_child; /**< following frame header with
                                              which this frame has the best
                                              score with                      */
} FLACHeaderMarker;

typedef struct FLACParseContext {
    AVCodecParserContext *pc;      /**< parent context                        */
    AVCodecContext *avctx;         /**< codec context pointer for logging     */
    FLACHeaderMarker *headers;     /**< linked-list that starts at the first
                                        CRC-8 verified header within buffer   */
    FLACHeaderMarker *best_header; /**< highest scoring header within buffer  */
    int nb_headers_found;          /**< number of headers found in the last
                                        flac_parse() call                     */
    int nb_headers_buffered;       /**< number of headers that are buffered   */
    int best_header_valid;         /**< flag set when the parser returns junk;
                                        if set return best_header next time   */
    AVFifoBuffer *fifo_buf;        /**< buffer to store all data until headers
                                        can be verified                       */
    int end_padded;                /**< specifies if fifo_buf's end is padded */
    uint8_t *wrap_buf;             /**< general fifo read buffer when wrapped */
    int wrap_buf_allocated_size;   /**< actual allocated size of the buffer   */
    FLACFrameInfo last_fi;         /**< last decoded frame header info        */
    int last_fi_valid;             /**< set if last_fi is valid               */
} FLACParseContext;

                                 FLACFrameInfo *fi)
{
}

/**
 * Non-destructive fast fifo pointer fetching
 * Returns a pointer from the specified offset.
 * If possible the pointer points within the fifo buffer.
 * Otherwise (if it would cause a wrap around,) a pointer to a user-specified
 * buffer is used.
 * The pointer can be NULL.  In any case it will be reallocated to hold the size.
 * If the returned pointer will be used after subsequent calls to flac_fifo_read_wrap
 * then the subsequent calls should pass in a different wrap_buf so as to not
 * overwrite the contents of the previous wrap_buf.
 * This function is based on av_fifo_generic_read, which is why there is a comment
 * about a memory barrier for SMP.
 */
static uint8_t *flac_fifo_read_wrap(FLACParseContext *fpc, int offset, int len,
                                    uint8_t **wrap_buf, int *allocated_size)
{
    AVFifoBuffer *f   = fpc->fifo_buf;
    uint8_t *start    = f->rptr + offset;
    uint8_t *tmp_buf;

    if (start >= f->end)
        start -= f->end - f->buffer;
    if (f->end - start >= len)
        return start;

    tmp_buf = av_fast_realloc(*wrap_buf, allocated_size, len);

    if (!tmp_buf) {
        av_log(fpc->avctx, AV_LOG_ERROR,
               "couldn't reallocate wrap buffer of size %d", len);
        return NULL;
    }
    *wrap_buf = tmp_buf;
    do {
        int seg_len = FFMIN(f->end - start, len);
        memcpy(tmp_buf, start, seg_len);
        tmp_buf = (uint8_t*)tmp_buf + seg_len;
// memory barrier needed for SMP here in theory

        start += seg_len - (f->end - f->buffer);
        len -= seg_len;
    } while (len > 0);

    return *wrap_buf;
}

/**
 * Return a pointer in the fifo buffer where the offset starts at until
 * the wrap point or end of request.
 * len will contain the valid length of the returned buffer.
 * A second call to flac_fifo_read (with new offset and len) should be called
 * to get the post-wrap buf if the returned len is less than the requested.
 **/
{

}

{
                                     MAX_FRAME_HEADER_SIZE,
                                     &fpc->wrap_buf,
                                     &fpc->wrap_buf_allocated_size);

        }

            av_log(fpc->avctx, AV_LOG_ERROR,
                   "couldn't allocate FLACHeaderMarker\n");
            return AVERROR(ENOMEM);
        }


    }
    return size;
}

                               int buf_size, int search_start)
{

        }
    }

                }
            }
        }
    }
}

{

    /* Search for a new header of at most 16 bytes. */

    /* If fifo end was hit do the wrap around. */

        /* search_start + 1 is the post-wrap offset in the fifo. */


            temp = find_headers_search_validate(fpc, search_start);
            size = FFMAX(size, temp);
        }

        /* Continue to do the last half of the wrap. */
    }

    /* Return the size even if no new headers were found. */
}

static int check_header_fi_mismatch(FLACParseContext  *fpc,
                                    FLACFrameInfo     *header_fi,
                                    FLACFrameInfo     *child_fi,
                                    int                log_level_offset)
{
    int deduction = 0;
    if (child_fi->samplerate != header_fi->samplerate) {
        deduction += FLAC_HEADER_CHANGED_PENALTY;
        av_log(fpc->avctx, AV_LOG_WARNING + log_level_offset,
               "sample rate change detected in adjacent frames\n");
    }
    if (child_fi->bps != header_fi->bps) {
        deduction += FLAC_HEADER_CHANGED_PENALTY;
        av_log(fpc->avctx, AV_LOG_WARNING + log_level_offset,
               "bits per sample change detected in adjacent frames\n");
    }
    if (child_fi->is_var_size != header_fi->is_var_size) {
        /* Changing blocking strategy not allowed per the spec */
        deduction += FLAC_HEADER_BASE_SCORE;
        av_log(fpc->avctx, AV_LOG_WARNING + log_level_offset,
               "blocking strategy change detected in adjacent frames\n");
    }
    if (child_fi->channels != header_fi->channels) {
        deduction += FLAC_HEADER_CHANGED_PENALTY;
        av_log(fpc->avctx, AV_LOG_WARNING + log_level_offset,
               "number of channels change detected in adjacent frames\n");
    }
    return deduction;
}

                                 FLACHeaderMarker  *header,
                                 FLACHeaderMarker  *child,
                                 int                log_level_offset)
{
                                         log_level_offset);
    /* Check sample and frame numbers. */
        (child_fi->frame_or_sample_num
        FLACHeaderMarker *curr;
        int64_t expected_frame_num, expected_sample_num;
        /* If there are frames in the middle we expect this deduction,
           as they are probably valid and this one follows it */

        expected_frame_num = expected_sample_num = header_fi->frame_or_sample_num;
        curr = header;
            /* Ignore frames that failed all crc checks */
                }
            }
        }

            expected_sample_num == child_fi->frame_or_sample_num)

                   "sample/frame number mismatch in adjacent frames\n");
    }

    /* If we have suspicious headers, check the CRC between them */
        FLACHeaderMarker *curr;
        int read_len;
        uint8_t *buf;
        uint32_t crc = 1;
        int inverted_test = 0;

        /* Since CRC is expensive only do it if we haven't yet.
           This assumes a CRC penalty is greater than all other check penalties */
        curr = header->next;
        for (i = 0; i < FLAC_MAX_SEQUENTIAL_HEADERS && curr != child; i++)
            curr = curr->next;

        if (header->link_penalty[i] < FLAC_HEADER_CRC_FAIL_PENALTY ||
            header->link_penalty[i] == FLAC_HEADER_NOT_PENALIZED_YET) {
            FLACHeaderMarker *start, *end;

            /* Although overlapping chains are scored, the crc should never
               have to be computed twice for a single byte. */
            start = header;
            end   = child;
            if (i > 0 &&
                header->link_penalty[i - 1] >= FLAC_HEADER_CRC_FAIL_PENALTY) {
                while (start->next != child)
                    start = start->next;
                inverted_test = 1;
            } else if (i > 0 &&
                       header->next->link_penalty[i-1] >=
                       FLAC_HEADER_CRC_FAIL_PENALTY ) {
                end = header->next;
                inverted_test = 1;
            }

            read_len = end->offset - start->offset;
            buf      = flac_fifo_read(fpc, start->offset, &read_len);
            crc      = av_crc(av_crc_get_table(AV_CRC_16_ANSI), 0, buf, read_len);
            read_len = (end->offset - start->offset) - read_len;

            if (read_len) {
                buf = flac_fifo_read(fpc, end->offset - read_len, &read_len);
                crc = av_crc(av_crc_get_table(AV_CRC_16_ANSI), crc, buf, read_len);
            }
        }

        if (!crc ^ !inverted_test) {
            deduction += FLAC_HEADER_CRC_FAIL_PENALTY;
            av_log(fpc->avctx, AV_LOG_WARNING + log_level_offset,
                   "crc check failed from offset %i (frame %"PRId64") to %i (frame %"PRId64")\n",
                   header->offset, header_fi->frame_or_sample_num,
                   child->offset, child_fi->frame_or_sample_num);
        }
    }
}

/**
 * Score a header.
 *
 * Give FLAC_HEADER_BASE_SCORE points to a frame for existing.
 * If it has children, (subsequent frames of which the preceding CRC footer
 * validates against this one,) then take the maximum score of the children,
 * with a penalty of FLAC_HEADER_CHANGED_PENALTY applied for each change to
 * bps, sample rate, channels, but not decorrelation mode, or blocksize,
 * because it can change often.
 **/
{
        return header->max_score;

    /* Modify the base score with changes from the last output header */
        /* Silence the log since this will be repeated if selected */
                                               AV_LOG_DEBUG);
    }


    /* Check and compute the children's scores. */
        /* Look at the child's frame header info and penalize suspicious
           changes between the headers. */
                                                               child, AV_LOG_DEBUG);
        }

            /* Keep the child because the frame scoring is dynamic. */
        }
    }

}

{
    /* First pass to clear all old scores. */

    /* Do a second pass to score them all. */
        }
    }

                           int *poutbuf_size)
{
    } else {

        /* If the child has suspicious changes, log them */
    }

        fpc->avctx->channels = header->fi.channels;
        ff_flac_set_channel_layout(fpc->avctx);
    }
                                        &fpc->wrap_buf,
                                        &fpc->wrap_buf_allocated_size);


          fpc->pc->pts = header->fi.frame_or_sample_num;
    }


    /* Return the negative overread index so the client can compute pos.
       This should be the amount overread to the beginning of the child */
    return 0;
}

                      const uint8_t **poutbuf, int *poutbuf_size,
                      const uint8_t *buf, int buf_size)
{

                avctx->sample_rate = fi.samplerate;
                fpc->pc->pts = fi.frame_or_sample_num;
                if (!fi.is_var_size)
                  fpc->pc->pts *= fi.blocksize;
            }
        }
    }


    /* If a best_header was found last call remove it with the buffer data. */

        /* Remove headers in list until the end of the best_header. */
                av_log(avctx, AV_LOG_DEBUG,
                       "dropping low score %i frame header from offset %i to %i\n",
                       curr->max_score, curr->offset, curr->next->offset);
            }
        }
        /* Release returned data from ring buffer. */

        /* Fix the offset for the headers remaining to match the new buffer. */

        }
        /* No end frame no need to delete the buffer; probably eof */

            temp = curr->next;
            av_free(curr);
            fpc->nb_headers_buffered--;
        }
    }

    /* Find and score new headers.                                     */
    /* buf_size is zero when flushing, so check for this since we do   */
    /* not want to try to read more input once we have found the end.  */
    /* Also note that buf can't be NULL.                               */

        /* Pad the end once if EOF, to check the final region for headers. */
        } else {
            /* The maximum read size is the upper-bound of what the parser
               needs to have the required number of frames buffered */
                                              nb_desired * FLAC_AVG_FRAME_SIZE);
        }

            av_fifo_size(fpc->fifo_buf) / FLAC_AVG_FRAME_SIZE >
            fpc->nb_headers_buffered * 20) {
            /* There is less than one valid flac header buffered for 20 headers
             * buffered. Therefore the fifo is most likely filled with invalid
             * data and the input is not a flac file. */
            goto handle_error;
        }

        /* Fill the buffer. */
            av_log(avctx, AV_LOG_ERROR,
                   "couldn't reallocate buffer of size %"PTRDIFF_SPECIFIER"\n",
                   (read_end - read_start) + av_fifo_size(fpc->fifo_buf));
            goto handle_error;
        }

                                  read_end - read_start, NULL);
        } else {
        }

        /* Tag headers and update sequences. */
                       ((read_end - read_start) + (MAX_FRAME_HEADER_SIZE - 1));

            av_log(avctx, AV_LOG_ERROR,
                   "find_new_headers couldn't allocate FLAC header\n");
            goto handle_error;
        }

        /* Wait till FLAC_MIN_HEADERS to output a valid frame. */
                read_start = read_end;
                continue;
            } else {
            }
        }

        /* If headers found, update the scores since we have longer chains. */

        /* restore the state pre-padding */
            /* HACK: drain the tail of the fifo */
                fpc->fifo_buf->wptr += fpc->fifo_buf->end -
                    fpc->fifo_buf->buffer;
            }
            read_start = read_end = NULL;
        }
    }

        }
    }

        // Only accept a bad header if there is no other option to continue
        if (!buf_size || read_end != buf || fpc->nb_headers_buffered < FLAC_MIN_HEADERS)
            fpc->best_header = NULL;
    }

            /* Output a junk frame. */
                   fpc->best_header->offset);

            /* Set duration to 0. It is unknown or invalid in a junk frame. */
                                                &fpc->wrap_buf,
                                                &fpc->wrap_buf_allocated_size);
        }
    }

}

{
    /* There will generally be FLAC_MIN_HEADERS buffered in the fifo before
       it drains.  This is allocated early to avoid slow reallocation. */
        av_log(fpc->avctx, AV_LOG_ERROR,
                "couldn't allocate fifo_buf\n");
        return AVERROR(ENOMEM);
    }
    return 0;
}

{

    }

AVCodecParser ff_flac_parser = {
    .codec_ids      = { AV_CODEC_ID_FLAC },
    .priv_data_size = sizeof(FLACParseContext),
    .parser_init    = flac_parse_init,
    .parser_parse   = flac_parse,
    .parser_close   = flac_parse_close,
};
