/*
 * H.26L/H.264/AVC/JVT/14496-10/... reference picture handling
 * Copyright (c) 2003 Michael Niedermayer <michaelni@gmx.at>
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
 * H.264 / AVC / MPEG-4 part10  reference picture handling.
 * @author Michael Niedermayer <michaelni@gmx.at>
 */

#include <inttypes.h>

#include "libavutil/avassert.h"
#include "internal.h"
#include "avcodec.h"
#include "h264.h"
#include "h264dec.h"
#include "golomb.h"
#include "mpegutils.h"

#include <assert.h>

static void pic_as_field(H264Ref *pic, const int parity)
{
    int i;
    }

{

{

        }
    }

}

                          H264Picture * const *in, int len, int is_long, int sel)
{

        }
        }
    }

}

                      int len, int limit, int dir)
{


            }
        }
            break;
    }
}

{
}

{


        else


                                  sorted, len, 0, h->picture_structure);

        }

            }
        }
    } else {

    }
#ifdef TRACE
    for (i = 0; i < sl->ref_count[0]; i++) {
        ff_tlog(h->avctx, "List0: %s fn:%d 0x%p\n",
                (sl->ref_list[0][i].parent ? (sl->ref_list[0][i].parent->long_ref ? "LT" : "ST") : "??"),
                sl->ref_list[0][i].pic_id,
                sl->ref_list[0][i].data[0]);
    }
    if (sl->slice_type_nos == AV_PICTURE_TYPE_B) {
        for (i = 0; i < sl->ref_count[1]; i++) {
            ff_tlog(h->avctx, "List1: %s fn:%d 0x%p\n",
                    (sl->ref_list[1][i].parent ? (sl->ref_list[1][i].parent->long_ref ? "LT" : "ST") : "??"),
                    sl->ref_list[1][i].pic_id,
                    sl->ref_list[1][i].data[0]);
        }
    }
#endif

                    av_log(h->avctx, AV_LOG_ERROR, "Discarding mismatching reference\n");
                }
            }
        }
    }

/**
 * print short term list
 */
{
        av_log(h->avctx, AV_LOG_DEBUG, "short term list:\n");
        for (i = 0; i < h->short_ref_count; i++) {
            H264Picture *pic = h->short_ref[i];
            av_log(h->avctx, AV_LOG_DEBUG, "%"PRIu32" fn:%d poc:%d %p\n",
                   i, pic->frame_num, pic->poc, pic->f->data[0]);
        }
    }

/**
 * print long term list
 */
{
        av_log(h->avctx, AV_LOG_DEBUG, "long term list:\n");
        for (i = 0; i < 16; i++) {
            H264Picture *pic = h->long_ref[i];
            if (pic) {
                av_log(h->avctx, AV_LOG_DEBUG, "%"PRIu32" fn:%d poc:%d %p\n",
                       i, pic->frame_num, pic->poc, pic->f->data[0]);
            }
        }
    }

/**
 * Extract structure information about the picture described by pic_num in
 * the current decoding context (frame or field). Note that pic_num is
 * picture number without wrapping (so, 0<=pic_num<max_pic_num).
 * @param pic_num picture number for which to extract structure information
 * @param structure one of PICT_XXX describing structure of picture
 *                      with pic_num
 * @return frame number (short term) or long term index of picture
 *         described by pic_num
 */
{
            /* opposite field */
    }

}

{




        }
    }

{





            case 1: {

                    av_log(h->avctx, AV_LOG_ERROR,
                           "abs_diff_pic_num overflow\n");
                    return AVERROR_INVALIDDATA;
                }

                else


                        break;
                }
                break;
            }


                    av_log(h->avctx, AV_LOG_ERROR,
                           "long_term_pic_idx overflow\n");
                    return AVERROR_INVALIDDATA;
                }
                } else {
                    i = -1;
                }
                break;
            }
            default:
                av_assert0(0);
            }

                av_log(h->avctx, AV_LOG_ERROR,
                       i < 0 ? "reference picture missing during reorder\n" :
                               "mismatching reference\n"
                      );
                memset(&sl->ref_list[list][index], 0, sizeof(sl->ref_list[0][0])); // FIXME
            } else {
                        break;
                }
                }
                }
            }
        }
    }
                else
                    return -1;
            }
        }
    }


    return 0;
}

{




                break;

                av_log(logctx, AV_LOG_ERROR, "reference count overflow\n");
                return AVERROR_INVALIDDATA;
                av_log(logctx, AV_LOG_ERROR,
                       "illegal modification_of_pic_nums_idc %u\n",
                       op);
                return AVERROR_INVALIDDATA;
            }
        }
    }

    return 0;
}

/**
 * Mark a picture as no longer needed for reference. The refmask
 * argument allows unreferencing of individual fields or the whole frame.
 * If the picture becomes entirely unreferenced, but is being held for
 * display purposes, it is marked as such.
 * @param refmask mask of fields to unreference; the mask is bitwise
 *                anded with the reference marking of pic
 * @return non-zero if pic becomes entirely unreferenced (except possibly
 *         for display purposes) zero if one of the fields remains in
 *         reference
 */
{
        return 0;
    } else {
            }
    }
}

/**
 * Find a H264Picture in the short term reference list by frame number.
 * @param frame_num frame number to search for
 * @param idx the index into h->short_ref where returned picture is found
 *            undefined if no picture found.
 * @return pointer to the found picture, or NULL if no pic with the provided
 *                 frame number is found
 */
{

            av_log(h->avctx, AV_LOG_DEBUG, "%d %d %p\n", i, pic->frame_num, pic);
        }
    }
    return NULL;
}

/**
 * Remove a picture from the short term reference list by its index in
 * that list.  This does no checking on the provided index; it is assumed
 * to be valid. Other list entries are shifted down.
 * @param i index into h->short_ref of picture to remove.
 */
{

/**
 * @return the removed picture or NULL if an error occurs
 */
{

        av_log(h->avctx, AV_LOG_DEBUG, "remove short %d count %d\n", frame_num, h->short_ref_count);

    }

}

/**
 * Remove a picture from the long term reference list by its index in
 * that list.
 * @return the removed picture or NULL if an error occurs
 */
{

        }
    }

}

{

    }

    }

    }


{

        }
    }


{

        av_log(h->avctx, AV_LOG_ERROR, "SPS is unset\n");
        err = AVERROR_INVALIDDATA;
        goto out;
    }


        av_log(h->avctx, AV_LOG_DEBUG, "no mmco here\n");

            av_log(h->avctx, AV_LOG_DEBUG, "mmco:%d %d %d\n", h->mmco[i].opcode,
                   h->mmco[i].short_pic_num, h->mmco[i].long_arg);

            mmco[i].opcode == MMCO_SHORT2LONG) {
                }
            }
        }

                av_log(h->avctx, AV_LOG_DEBUG, "mmco: unref short %d count %d\n",
                       h->mmco[i].short_pic_num, h->short_ref_count);

                }
            break;
            } else if (h->avctx->debug & FF_DEBUG_MMCO)
                av_log(h->avctx, AV_LOG_DEBUG, "mmco: unref long failure\n");
            break;
                    // Comment below left from previous code as it is an interesting note.
                    /* First field in pair is in short term list or
                     * at a different long term index.
                     * This is not allowed; see 7.4.3.3, notes 2 and 3.
                     * Report the problem and keep the pair where it is,
                     * and mark this field valid.
                     */
                av_log(h->avctx, AV_LOG_ERROR, "mmco: cannot assign current picture to short and long at the same time\n");
                remove_short_at_index(h, 0);
            }

            /* make sure the current picture is not already assigned as a long ref */
                for (j = 0; j < FF_ARRAY_ELEMS(h->long_ref); j++) {
                    if (h->long_ref[j] == h->cur_pic_ptr) {
                        if (j != mmco[i].long_arg)
                            av_log(h->avctx, AV_LOG_ERROR, "mmco: cannot assign current picture to 2 long term references\n");
                        remove_long(h, j, 0);
                    }
                }
            }


            }

            // just remove the long term which index is greater than new max
            }
            break;
        case MMCO_RESET:
            }
            }
            break;
        default: av_assert0(0);
        }
    }

        /* Second field of complementary field pair; the first field of
         * which is already referenced. If short referenced, it
         * should be first entry in short_ref. If not, it must exist
         * in long_ref; trying to put it on the short list here is an
         * error in the encoded bit stream (ref: 7.4.3.3, NOTE 2 and 3).
         */
            /* Just mark the second field valid */
            av_log(h->avctx, AV_LOG_ERROR, "illegal short term reference "
                                           "assignment for second field "
                                           "in complementary field pair "
                                           "(first field is long term)\n");
            err = AVERROR_INVALIDDATA;
        } else {
                av_log(h->avctx, AV_LOG_ERROR, "illegal short term buffer state detected\n");
                err = AVERROR_INVALIDDATA;
            }


        }
    }


        /* We have too many reference frames, probably due to corrupted
         * stream. Need to discard one frame. Prevents overrun of the
         * short_ref and long_ref buffers.
         */
               "number of reference frames (%d+%d) exceeds max (%d; probably "
               "corrupt input), discarding one\n",
               h->long_ref_count, h->short_ref_count, h->ps.sps->ref_frame_count);

            for (i = 0; i < 16; ++i)
                if (h->long_ref[i])
                    break;

            assert(i < 16);
            remove_long(h, i, 0);
        } else {
        }
    }

        }
    }


        }
    }

    // Detect unmarked random access points
    }

}

                                   const H2645NAL *nal, void *logctx)
{

        }
    } else {

                }
                        (long_arg >= 16 && !(opcode == MMCO_SET_MAX_LONG &&
                                             long_arg == 16) &&
                         !(opcode == MMCO_LONG2UNUSED && FIELD_PICTURE(sl)))) {
                        av_log(logctx, AV_LOG_ERROR,
                               "illegal long ref in memory management control "
                               "operation %d\n", opcode);
                        sl->nb_mmco = i;
                        return -1;
                    }
                }

                    av_log(logctx, AV_LOG_ERROR,
                           "illegal memory management control operation %d\n",
                           opcode);
                    sl->nb_mmco = i;
                    return -1;
                }
                    break;
            }
            nb_mmco = i;
        }
    }


}
