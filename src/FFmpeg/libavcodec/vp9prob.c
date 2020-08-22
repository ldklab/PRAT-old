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

#include "vp56.h"
#include "vp9.h"
#include "vp9data.h"
#include "vp9dec.h"

                                        int max_count, int update_factor)
{

        return;


    // (p1 * (256 - update_factor) + p2 * update_factor + 128) >> 8
}

{

    // coefficients

                            break;

                    }

    }

    // skip flag

    // intra/inter flag

    // comppred flag
    }

    // reference frames
    }


        }
    }

    // block partitioning

        }

    // tx size

                       s->td[0].counts.tx8p[i][1], 20, 128);
        }
    }

    // interpolation filter

        }
    }

    // inter modes

    }

    // mv joints
    {

    }

    // mv components




        }

                       s->td[0].counts.mv_comp[i].class0_hp[0],
                       s->td[0].counts.mv_comp[i].hp[1], 20, 128);
        }
    }

    // y intra modes

                   20, 128);
    }

    // uv intra modes

                   20, 128);
    }
}
