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

#include "vp9dec.h"

                                               uint8_t *lvl, uint8_t (*mask)[4],
                                               uint8_t *dst, ptrdiff_t ls)
{

    // filter edges between columns (e.g. block1 | block2)


                        } else {
                        }
                    } else {
                    }

                }
            }
            } else {

                    } else {
                    }

                }
            }
        }
    }
}

                                               uint8_t *lvl, uint8_t (*mask)[4],
                                               uint8_t *dst, ptrdiff_t ls)
{

    //                                 block1
    // filter edges between rows (e.g. ------)
    //                                 block2


                        } else {
                        }
                    } else {
                    }

                }
            }

                    } else {
                    }

                }
            }
        }
        } else {
        }
    }
}

                          int row, int col, ptrdiff_t yoff, ptrdiff_t uvoff)
{

    /* FIXME: In how far can we interleave the v/h loopfilter calls? E.g.
     * if you think of them as acting on a 8x8 block max, we can interleave
     * each v/h within the single x loop, but that only works if we work on
     * 8 pixel blocks, and we won't always do that (we want at least 16px
     * to use SSE2 optimizations, perhaps 32 for AVX2) */


    }
