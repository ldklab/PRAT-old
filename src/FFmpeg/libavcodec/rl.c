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

#include <stdint.h>
#include <string.h>

#include "libavutil/attributes.h"
#include "libavutil/avassert.h"
#include "libavutil/mem.h"

#include "rl.h"

void ff_rl_free(RLTable *rl)
{
    int i;

    for (i = 0; i < 2; i++) {
        av_freep(&rl->max_run[i]);
        av_freep(&rl->max_level[i]);
        av_freep(&rl->index_run[i]);
    }
}

                       uint8_t static_store[2][2 * MAX_RUN + MAX_LEVEL + 3])
{

    /* If table is static, we can quit if rl->max_level[0] is not NULL */
        return 0;

    /* compute max_level[], max_run[] and index_run[] */
        } else {
        }

        }
        else {
            rl->max_level[last] = av_malloc(MAX_RUN + 1);
            if (!rl->max_level[last])
                goto fail;
        }
        else {
            rl->max_run[last]   = av_malloc(MAX_LEVEL + 1);
            if (!rl->max_run[last])
                goto fail;
        }
        else {
            rl->index_run[last] = av_malloc(MAX_RUN + 1);
            if (!rl->index_run[last])
                goto fail;
        }
    }
    return 0;

fail:
    ff_rl_free(rl);
    return AVERROR(ENOMEM);
}

{


        }

                run   = 66;
                level = MAX_LEVEL;
                run   = 0;
                level = code;
            } else {
                    run   = 66;
                    level =  0;
                } else {
                }
            }
        }
    }
