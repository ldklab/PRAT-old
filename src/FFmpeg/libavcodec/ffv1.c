/*
 * FFV1 codec for libavcodec
 *
 * Copyright (c) 2003-2013 Michael Niedermayer <michaelni@gmx.at>
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
 * FF Video Codec 1 (a lossless codec)
 */

#include "libavutil/attributes.h"
#include "libavutil/avassert.h"
#include "libavutil/crc.h"
#include "libavutil/opt.h"
#include "libavutil/imgutils.h"
#include "libavutil/pixdesc.h"

#include "avcodec.h"
#include "internal.h"
#include "rangecoder.h"
#include "mathops.h"
#include "ffv1.h"

{

        return AVERROR_INVALIDDATA;


        return AVERROR(ENOMEM);


    // defaults

}

{


                                     sizeof(uint8_t));
                return AVERROR(ENOMEM);
        } else {
                    return AVERROR(ENOMEM);
                }
            }
        }
    }

        //FIXME only redo if state_transition changed
        }
    }

    return 0;
}

{
            return AVERROR(ENOMEM);
    }
    return 0;
}

{



            goto memfail;



                                      sizeof(*fs->sample_buffer));
                                        sizeof(*fs->sample_buffer32));
            av_freep(&fs->sample_buffer);
            av_freep(&fs->sample_buffer32);
            av_freep(&f->slice_context[i]);
            goto memfail;
        }
    }
    return 0;

memfail:
    while(--i >= 0) {
        av_freep(&f->slice_context[i]->sample_buffer);
        av_freep(&f->slice_context[i]->sample_buffer32);
        av_freep(&f->slice_context[i]);
    }
    return AVERROR(ENOMEM);
}

{

                                         sizeof(*f->initial_states[i]));
            return AVERROR(ENOMEM);
    }
    return 0;
}

{



            } else
        } else {
            }
        }
    }


{




        }
    }

        }
    }


}
