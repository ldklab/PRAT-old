/*
 * VP9 compatible video decoder
 *
 * Copyright (C) 2013 Ronald S. Bultje <rsbultje gmail com>
 * Copyright (C) 2013 Clément Bœsch <u pkh me>
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

#define ROUNDED_DIV_MVx2(a, b) \
    (VP56mv) { .x = ROUNDED_DIV(a.x + b.x, 2), .y = ROUNDED_DIV(a.y + b.y, 2) }
#define ROUNDED_DIV_MVx4(a, b, c, d) \
    (VP56mv) { .x = ROUNDED_DIV(a.x + b.x + c.x + d.x, 4), \
               .y = ROUNDED_DIV(a.y + b.y + c.y + d.y, 4) }

{
        { 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4 },
        { 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 4, 4 },
    };

    }

    // y inter pred

#if SCALED == 0
                        ref1->data[0], ref1->linesize[0], tref1,
                        row << 3, col << 3, &b->mv[0][0],,,,, 8, 4, w1, h1, 0);
                        td->dst[0] + 4 * ls_y, ls_y,
                        ref1->data[0], ref1->linesize[0], tref1,
                        (row << 3) + 4, col << 3, &b->mv[2][0],,,,, 8, 4, w1, h1, 0);
                              td->dst[1], td->dst[2], ls_uv,
                              ref1->data[1], ref1->linesize[1],
                              ref1->data[2], ref1->linesize[2], tref1,
                              row << 2, col << (3 - s->ss_h),
                              &uvmv,,,,, 8 >> s->ss_h, 4, w1, h1, 0);
            } else {
                              td->dst[1], td->dst[2], ls_uv,
                              ref1->data[1], ref1->linesize[1],
                              ref1->data[2], ref1->linesize[2], tref1,
                              row << 3, col << (3 - s->ss_h),
                              &b->mv[0][0],,,,, 8 >> s->ss_h, 4, w1, h1, 0);
                // BUG for 4:2:2 bs=8x4, libvpx uses the wrong block index
                // to get the motion vector for the bottom 4x4 block
                // https://code.google.com/p/webm/issues/detail?id=993
                } else {
                }
                              td->dst[1] + 4 * ls_uv, td->dst[2] + 4 * ls_uv, ls_uv,
                              ref1->data[1], ref1->linesize[1],
                              ref1->data[2], ref1->linesize[2], tref1,
                              (row << 3) + 4, col << (3 - s->ss_h),
                              &uvmv,,,,, 8 >> s->ss_h, 4, w1, h1, 0);
            }

                            ref2->data[0], ref2->linesize[0], tref2,
                            row << 3, col << 3, &b->mv[0][1],,,,, 8, 4, w2, h2, 1);
                            td->dst[0] + 4 * ls_y, ls_y,
                            ref2->data[0], ref2->linesize[0], tref2,
                            (row << 3) + 4, col << 3, &b->mv[2][1],,,,, 8, 4, w2, h2, 1);
                                  td->dst[1], td->dst[2], ls_uv,
                                  ref2->data[1], ref2->linesize[1],
                                  ref2->data[2], ref2->linesize[2], tref2,
                                  row << 2, col << (3 - s->ss_h),
                                  &uvmv,,,,, 8 >> s->ss_h, 4, w2, h2, 1);
                } else {
                                  td->dst[1], td->dst[2], ls_uv,
                                  ref2->data[1], ref2->linesize[1],
                                  ref2->data[2], ref2->linesize[2], tref2,
                                  row << 3, col << (3 - s->ss_h),
                                  &b->mv[0][1],,,,, 8 >> s->ss_h, 4, w2, h2, 1);
                    // BUG for 4:2:2 bs=8x4, libvpx uses the wrong block index
                    // to get the motion vector for the bottom 4x4 block
                    // https://code.google.com/p/webm/issues/detail?id=993
                    } else {
                    }
                                  td->dst[1] + 4 * ls_uv, td->dst[2] + 4 * ls_uv, ls_uv,
                                  ref2->data[1], ref2->linesize[1],
                                  ref2->data[2], ref2->linesize[2], tref2,
                                  (row << 3) + 4, col << (3 - s->ss_h),
                                  &uvmv,,,,, 8 >> s->ss_h, 4, w2, h2, 1);
                }
            }
                        ref1->data[0], ref1->linesize[0], tref1,
                        row << 3, col << 3, &b->mv[0][0],,,,, 4, 8, w1, h1, 0);
                        ref1->data[0], ref1->linesize[0], tref1,
                        row << 3, (col << 3) + 4, &b->mv[1][0],,,,, 4, 8, w1, h1, 0);
                              td->dst[1], td->dst[2], ls_uv,
                              ref1->data[1], ref1->linesize[1],
                              ref1->data[2], ref1->linesize[2], tref1,
                              row << (3 - s->ss_v), col << 2,
                              &uvmv,,,,, 4, 8 >> s->ss_v, w1, h1, 0);
            } else {
                              td->dst[1], td->dst[2], ls_uv,
                              ref1->data[1], ref1->linesize[1],
                              ref1->data[2], ref1->linesize[2], tref1,
                              row << (3 - s->ss_v), col << 3,
                              &b->mv[0][0],,,,, 4, 8 >> s->ss_v, w1, h1, 0);
                              td->dst[1] + 4 * bytesperpixel,
                              td->dst[2] + 4 * bytesperpixel, ls_uv,
                              ref1->data[1], ref1->linesize[1],
                              ref1->data[2], ref1->linesize[2], tref1,
                              row << (3 - s->ss_v), (col << 3) + 4,
                              &b->mv[1][0],,,,, 4, 8 >> s->ss_v, w1, h1, 0);
            }

                            ref2->data[0], ref2->linesize[0], tref2,
                            row << 3, col << 3, &b->mv[0][1],,,,, 4, 8, w2, h2, 1);
                            ref2->data[0], ref2->linesize[0], tref2,
                            row << 3, (col << 3) + 4, &b->mv[1][1],,,,, 4, 8, w2, h2, 1);
                                  td->dst[1], td->dst[2], ls_uv,
                                  ref2->data[1], ref2->linesize[1],
                                  ref2->data[2], ref2->linesize[2], tref2,
                                  row << (3 - s->ss_v), col << 2,
                                  &uvmv,,,,, 4, 8 >> s->ss_v, w2, h2, 1);
                } else {
                                  td->dst[1], td->dst[2], ls_uv,
                                  ref2->data[1], ref2->linesize[1],
                                  ref2->data[2], ref2->linesize[2], tref2,
                                  row << (3 - s->ss_v), col << 3,
                                  &b->mv[0][1],,,,, 4, 8 >> s->ss_v, w2, h2, 1);
                                  td->dst[1] + 4 * bytesperpixel,
                                  td->dst[2] + 4 * bytesperpixel, ls_uv,
                                  ref2->data[1], ref2->linesize[1],
                                  ref2->data[2], ref2->linesize[2], tref2,
                                  row << (3 - s->ss_v), (col << 3) + 4,
                                  &b->mv[1][1],,,,, 4, 8 >> s->ss_v, w2, h2, 1);
                }
            }
        } else
#endif
        {
#if SCALED == 0
#endif

            // FIXME if two horizontally adjacent blocks have the same MV,
            // do a w8 instead of a w4 call
                        ref1->data[0], ref1->linesize[0], tref1,
                        row << 3, col << 3, &b->mv[0][0],
                        0, 0, 8, 8, 4, 4, w1, h1, 0);
                        ref1->data[0], ref1->linesize[0], tref1,
                        row << 3, (col << 3) + 4, &b->mv[1][0],
                        4, 0, 8, 8, 4, 4, w1, h1, 0);
                        td->dst[0] + 4 * ls_y, ls_y,
                        ref1->data[0], ref1->linesize[0], tref1,
                        (row << 3) + 4, col << 3, &b->mv[2][0],
                        0, 4, 8, 8, 4, 4, w1, h1, 0);
                        td->dst[0] + 4 * ls_y + 4 * bytesperpixel, ls_y,
                        ref1->data[0], ref1->linesize[0], tref1,
                        (row << 3) + 4, (col << 3) + 4, &b->mv[3][0],
                        4, 4, 8, 8, 4, 4, w1, h1, 0);
                                            b->mv[2][0], b->mv[3][0]);
                                  td->dst[1], td->dst[2], ls_uv,
                                  ref1->data[1], ref1->linesize[1],
                                  ref1->data[2], ref1->linesize[2], tref1,
                                  row << 2, col << 2,
                                  &uvmv, 0, 0, 4, 4, 4, 4, w1, h1, 0);
                } else {
                                  td->dst[1], td->dst[2], ls_uv,
                                  ref1->data[1], ref1->linesize[1],
                                  ref1->data[2], ref1->linesize[2], tref1,
                                  row << 2, col << 3,
                                  &uvmv, 0, 0, 8, 4, 4, 4, w1, h1, 0);
                                  td->dst[1] + 4 * bytesperpixel,
                                  td->dst[2] + 4 * bytesperpixel, ls_uv,
                                  ref1->data[1], ref1->linesize[1],
                                  ref1->data[2], ref1->linesize[2], tref1,
                                  row << 2, (col << 3) + 4,
                                  &uvmv, 4, 0, 8, 4, 4, 4, w1, h1, 0);
                }
            } else {
                                  td->dst[1], td->dst[2], ls_uv,
                                  ref1->data[1], ref1->linesize[1],
                                  ref1->data[2], ref1->linesize[2], tref1,
                                  row << 3, col << 2,
                                  &uvmv, 0, 0, 4, 8, 4, 4, w1, h1, 0);
                    // BUG libvpx uses wrong block index for 4:2:2 bs=4x4
                    // bottom block
                    // https://code.google.com/p/webm/issues/detail?id=993
                                  td->dst[1] + 4 * ls_uv, td->dst[2] + 4 * ls_uv, ls_uv,
                                  ref1->data[1], ref1->linesize[1],
                                  ref1->data[2], ref1->linesize[2], tref1,
                                  (row << 3) + 4, col << 2,
                                  &uvmv, 0, 4, 4, 8, 4, 4, w1, h1, 0);
                } else {
                                  td->dst[1], td->dst[2], ls_uv,
                                  ref1->data[1], ref1->linesize[1],
                                  ref1->data[2], ref1->linesize[2], tref1,
                                  row << 3, col << 3,
                                  &b->mv[0][0], 0, 0, 8, 8, 4, 4, w1, h1, 0);
                                  td->dst[1] + 4 * bytesperpixel,
                                  td->dst[2] + 4 * bytesperpixel, ls_uv,
                                  ref1->data[1], ref1->linesize[1],
                                  ref1->data[2], ref1->linesize[2], tref1,
                                  row << 3, (col << 3) + 4,
                                  &b->mv[1][0], 4, 0, 8, 8, 4, 4, w1, h1, 0);
                                  td->dst[1] + 4 * ls_uv, td->dst[2] + 4 * ls_uv, ls_uv,
                                  ref1->data[1], ref1->linesize[1],
                                  ref1->data[2], ref1->linesize[2], tref1,
                                  (row << 3) + 4, col << 3,
                                  &b->mv[2][0], 0, 4, 8, 8, 4, 4, w1, h1, 0);
                                  td->dst[1] + 4 * ls_uv + 4 * bytesperpixel,
                                  td->dst[2] + 4 * ls_uv + 4 * bytesperpixel, ls_uv,
                                  ref1->data[1], ref1->linesize[1],
                                  ref1->data[2], ref1->linesize[2], tref1,
                                  (row << 3) + 4, (col << 3) + 4,
                                  &b->mv[3][0], 4, 4, 8, 8, 4, 4, w1, h1, 0);
                }
            }

                            ref2->data[0], ref2->linesize[0], tref2,
                            row << 3, col << 3, &b->mv[0][1], 0, 0, 8, 8, 4, 4, w2, h2, 1);
                            ref2->data[0], ref2->linesize[0], tref2,
                            row << 3, (col << 3) + 4, &b->mv[1][1], 4, 0, 8, 8, 4, 4, w2, h2, 1);
                            td->dst[0] + 4 * ls_y, ls_y,
                            ref2->data[0], ref2->linesize[0], tref2,
                            (row << 3) + 4, col << 3, &b->mv[2][1], 0, 4, 8, 8, 4, 4, w2, h2, 1);
                            td->dst[0] + 4 * ls_y + 4 * bytesperpixel, ls_y,
                            ref2->data[0], ref2->linesize[0], tref2,
                            (row << 3) + 4, (col << 3) + 4, &b->mv[3][1], 4, 4, 8, 8, 4, 4, w2, h2, 1);
                                                b->mv[2][1], b->mv[3][1]);
                                      td->dst[1], td->dst[2], ls_uv,
                                      ref2->data[1], ref2->linesize[1],
                                      ref2->data[2], ref2->linesize[2], tref2,
                                      row << 2, col << 2,
                                      &uvmv, 0, 0, 4, 4, 4, 4, w2, h2, 1);
                    } else {
                                      td->dst[1], td->dst[2], ls_uv,
                                      ref2->data[1], ref2->linesize[1],
                                      ref2->data[2], ref2->linesize[2], tref2,
                                      row << 2, col << 3,
                                      &uvmv, 0, 0, 8, 4, 4, 4, w2, h2, 1);
                                      td->dst[1] + 4 * bytesperpixel,
                                      td->dst[2] + 4 * bytesperpixel, ls_uv,
                                      ref2->data[1], ref2->linesize[1],
                                      ref2->data[2], ref2->linesize[2], tref2,
                                      row << 2, (col << 3) + 4,
                                      &uvmv, 4, 0, 8, 4, 4, 4, w2, h2, 1);
                    }
                } else {
                                      td->dst[1], td->dst[2], ls_uv,
                                      ref2->data[1], ref2->linesize[1],
                                      ref2->data[2], ref2->linesize[2], tref2,
                                      row << 3, col << 2,
                                      &uvmv, 0, 0, 4, 8, 4, 4, w2, h2, 1);
                        // BUG libvpx uses wrong block index for 4:2:2 bs=4x4
                        // bottom block
                        // https://code.google.com/p/webm/issues/detail?id=993
                                      td->dst[1] + 4 * ls_uv, td->dst[2] + 4 * ls_uv, ls_uv,
                                      ref2->data[1], ref2->linesize[1],
                                      ref2->data[2], ref2->linesize[2], tref2,
                                      (row << 3) + 4, col << 2,
                                      &uvmv, 0, 4, 4, 8, 4, 4, w2, h2, 1);
                    } else {
                                      td->dst[1], td->dst[2], ls_uv,
                                      ref2->data[1], ref2->linesize[1],
                                      ref2->data[2], ref2->linesize[2], tref2,
                                      row << 3, col << 3,
                                      &b->mv[0][1], 0, 0, 8, 8, 4, 4, w2, h2, 1);
                                      td->dst[1] + 4 * bytesperpixel,
                                      td->dst[2] + 4 * bytesperpixel, ls_uv,
                                      ref2->data[1], ref2->linesize[1],
                                      ref2->data[2], ref2->linesize[2], tref2,
                                      row << 3, (col << 3) + 4,
                                      &b->mv[1][1], 4, 0, 8, 8, 4, 4, w2, h2, 1);
                                      td->dst[1] + 4 * ls_uv, td->dst[2] + 4 * ls_uv, ls_uv,
                                      ref2->data[1], ref2->linesize[1],
                                      ref2->data[2], ref2->linesize[2], tref2,
                                      (row << 3) + 4, col << 3,
                                      &b->mv[2][1], 0, 4, 8, 8, 4, 4, w2, h2, 1);
                                      td->dst[1] + 4 * ls_uv + 4 * bytesperpixel,
                                      td->dst[2] + 4 * ls_uv + 4 * bytesperpixel, ls_uv,
                                      ref2->data[1], ref2->linesize[1],
                                      ref2->data[2], ref2->linesize[2], tref2,
                                      (row << 3) + 4, (col << 3) + 4,
                                      &b->mv[3][1], 4, 4, 8, 8, 4, 4, w2, h2, 1);
                    }
                }
            }
        }
    } else {

                    ref1->data[0], ref1->linesize[0], tref1,
                    row << 3, col << 3, &b->mv[0][0], 0, 0, bw, bh, bw, bh, w1, h1, 0);
                      td->dst[1], td->dst[2], ls_uv,
                      ref1->data[1], ref1->linesize[1],
                      ref1->data[2], ref1->linesize[2], tref1,
                      row << (3 - s->ss_v), col << (3 - s->ss_h),
                      &b->mv[0][0], 0, 0, uvbw, uvbh, uvbw, uvbh, w1, h1, 0);

                        ref2->data[0], ref2->linesize[0], tref2,
                        row << 3, col << 3, &b->mv[0][1], 0, 0, bw, bh, bw, bh, w2, h2, 1);
                          td->dst[1], td->dst[2], ls_uv,
                          ref2->data[1], ref2->linesize[1],
                          ref2->data[2], ref2->linesize[2], tref2,
                          row << (3 - s->ss_v), col << (3 - s->ss_h),
                          &b->mv[0][1], 0, 0, uvbw, uvbh, uvbw, uvbh, w2, h2, 1);
        }
    }
