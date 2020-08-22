/*
 * H.26L/H.264/AVC/JVT/14496-10/... decoder
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

#undef FUNC
#undef PIXEL_SHIFT

#if SIMPLE
#   define FUNC(n) AV_JOIN(n ## _simple_, BITS)
#   define PIXEL_SHIFT (BITS >> 4)
#else
#   define FUNC(n) n ## _complex
#   define PIXEL_SHIFT h->pixel_shift
#endif

#undef  CHROMA_IDC
#define CHROMA_IDC 1
#include "h264_mc_template.c"

#undef  CHROMA_IDC
#define CHROMA_IDC 2
#include "h264_mc_template.c"

{




        }
            int list;
                } else {
                    }
                }
            }
        }
    } else {
        // dct_offset = s->linesize * 16;
    }

            int j;
            GetBitContext gb;
            init_get_bits(&gb, sl->intra_pcm_ptr,
                          ff_h264_mb_sizes[h->ps.sps->chroma_format_idc] * bit_depth);

            for (i = 0; i < 16; i++) {
                uint16_t *tmp_y = (uint16_t *)(dest_y + i * linesize);
                for (j = 0; j < 16; j++)
                    tmp_y[j] = get_bits(&gb, bit_depth);
            }
            if (SIMPLE || !CONFIG_GRAY || !(h->flags & AV_CODEC_FLAG_GRAY)) {
                if (!h->ps.sps->chroma_format_idc) {
                    for (i = 0; i < block_h; i++) {
                        uint16_t *tmp_cb = (uint16_t *)(dest_cb + i * uvlinesize);
                        uint16_t *tmp_cr = (uint16_t *)(dest_cr + i * uvlinesize);
                        for (j = 0; j < 8; j++) {
                            tmp_cb[j] = tmp_cr[j] = 1 << (bit_depth - 1);
                        }
                    }
                } else {
                    for (i = 0; i < block_h; i++) {
                        uint16_t *tmp_cb = (uint16_t *)(dest_cb + i * uvlinesize);
                        for (j = 0; j < 8; j++)
                            tmp_cb[j] = get_bits(&gb, bit_depth);
                    }
                    for (i = 0; i < block_h; i++) {
                        uint16_t *tmp_cr = (uint16_t *)(dest_cr + i * uvlinesize);
                        for (j = 0; j < 8; j++)
                            tmp_cr[j] = get_bits(&gb, bit_depth);
                    }
                }
            }
        } else {
                    for (i = 0; i < 8; i++) {
                        memset(dest_cb + i * uvlinesize, 1 << (bit_depth - 1), 8);
                        memset(dest_cr + i * uvlinesize, 1 << (bit_depth - 1), 8);
                    }
                } else {
                    }
                }
            }
        }
    } else {
                               uvlinesize, 1, 0, SIMPLE, PIXEL_SHIFT);

            }

                                      transform_bypass, PIXEL_SHIFT,
                                      block_offset, linesize, dest_y, 0);

                               uvlinesize, 0, 0, SIMPLE, PIXEL_SHIFT);
        } else {
            } else {
            }
        }

                               PIXEL_SHIFT, block_offset, linesize, dest_y, 0);

                     sl->chroma_pred_mode == HOR_PRED8x8)) {
                                                            block_offset + 16,
                                                            uvlinesize);
                                                            block_offset + 32,
                                                            uvlinesize);
                } else {
                                         uvlinesize);
                            for (i = j * 16 + 4; i < j * 16 + 8; i++)
                                if (sl->non_zero_count_cache[scan8[i + 4]] ||
                                    dctcoef_get(sl->mb, PIXEL_SHIFT, i * 16))
                                    idct_add(dest[j - 1] + block_offset[i + 4],
                                             sl->mb + (i * 16 << PIXEL_SHIFT),
                                             uvlinesize);
                        }
                    }
                }
            } else {
                } else {
                }
            }
        }
    }

#if !SIMPLE || BITS == 8

#undef  CHROMA_IDC
#define CHROMA_IDC 3
#include "h264_mc_template.c"

{

                         sl->linesize, 4);
    }


        linesize     = sl->mb_linesize = sl->mb_uvlinesize = sl->linesize * 2;
        block_offset = &h->block_offset[48];
        if (mb_y & 1) // FIXME move out of this function?
            for (p = 0; p < 3; p++)
                dest[p] -= sl->linesize * 15;
        if (FRAME_MBAFF(h)) {
            int list;
            for (list = 0; list < sl->list_count; list++) {
                if (!USES_LIST(mb_type, list))
                    continue;
                if (IS_16X16(mb_type)) {
                    int8_t *ref = &sl->ref_cache[list][scan8[0]];
                    fill_rectangle(ref, 4, 4, 8, (16 + *ref) ^ (sl->mb_y & 1), 1);
                } else {
                    for (i = 0; i < 16; i += 4) {
                        int ref = sl->ref_cache[list][scan8[i]];
                        if (ref >= 0)
                            fill_rectangle(&sl->ref_cache[list][scan8[i]], 2, 2,
                                           8, (16 + ref) ^ (sl->mb_y & 1), 1);
                    }
                }
            }
        }
    } else {
    }

        if (PIXEL_SHIFT) {
            const int bit_depth = h->ps.sps->bit_depth_luma;
            GetBitContext gb;
            init_get_bits(&gb, sl->intra_pcm_ptr, 768 * bit_depth);

            for (p = 0; p < plane_count; p++)
                for (i = 0; i < 16; i++) {
                    uint16_t *tmp = (uint16_t *)(dest[p] + i * linesize);
                    for (j = 0; j < 16; j++)
                        tmp[j] = get_bits(&gb, bit_depth);
                }
        } else {
            for (p = 0; p < plane_count; p++)
                for (i = 0; i < 16; i++)
                    memcpy(dest[p] + i * linesize,
                           sl->intra_pcm_ptr + p * 256 + i * 16, 16);
        }
    } else {
                               linesize, 1, 1, SIMPLE, PIXEL_SHIFT);

                                          transform_bypass, PIXEL_SHIFT,
                                          block_offset, linesize, dest[p], p);

                               linesize, 0, 1, SIMPLE, PIXEL_SHIFT);
        } else {
        }

                                   PIXEL_SHIFT, block_offset, linesize,
                                   dest[p], p);
    }

#endif
