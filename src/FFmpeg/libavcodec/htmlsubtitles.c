/*
 * Copyright (c) 2010  Aurelien Jacobs <aurel@gnuage.org>
 * Copyright (c) 2017  Clément Bœsch <u@pkh.me>
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

#include "libavutil/avassert.h"
#include "libavutil/avstring.h"
#include "libavutil/common.h"
#include "libavutil/parseutils.h"
#include "htmlsubtitles.h"
#include <ctype.h>

{
        return -1;
}

{
}

/*
 * Fast code for scanning text enclosed in braces. Functionally
 * equivalent to this sscanf call:
 *
 * sscanf(in, "{\\an%*1u}%n", &len) >= 0 && len > 0
 */
        return 0;
    }
        return 0;
    }
        return 0;
    }
    return 1;
}

/* skip all {\xxx} substrings except for {\an%d}
   and all microdvd like styles such as {Y:xxx} */
{


            } else
                *closing_brace_missing = 1;
        }
    }

}

struct font_tag {
    char face[128];
    int size;
    uint32_t color;
};

/*
 * Fast code for scanning the rest of a tag. Functionally equivalent to
 * this sscanf call:
 *
 * sscanf(in, "%127[^<>]>%n", buffer, lenp) == 2
 */

        case '\0':
            return 0;
        case '<':
            return 0;
        default:
        }
    }
    return 0;
}

/*
 * The general politic of the convert is to mask unsupported tags or formatting
 * errors (but still alert the user/subtitles writer with an error/warning)
 * without dropping any actual text content for the final user.
 */
{

    /*
     * state stack is only present for fonts since they are the only tags where
     * the state is not binary. Here is a typical use case:
     *
     *   <font color="red" size=10>
     *     red 10
     *     <font size=50> RED AND BIG </font>
     *     red 10 again
     *   </font>
     *
     * On the other hand, using the state system for all the tags should be
     * avoided because it breaks wrongly nested tags such as:
     *
     *   <b> foo <i> bar </b> bla </i>
     *
     * We don't want to break here; instead, we will treat all these tags as
     * binary state markers. Basically, "<b>" will activate bold, and "</b>"
     * will deactivate it, whatever the current state.
     *
     * This will also prevents cases where we have a random closing tag
     * remaining after the opening one was dropped. Yes, this happens and we
     * still don't want to print a "</b>" at the end of the dialog event.
     */


        case '\r':
            break;
                end = 1;
                break;
            }
            break;
        case '<':
            /*
             * "<<" are likely latin guillemets in ASCII or some kind of random
             * style effect; see sub/badsyntax.srt in the FATE samples
             * directory for real test cases.
             */

            likely_a_tag = 1;
            }




                }

                /* Check if this is likely a tag */
#define LIKELY_A_TAG_CHAR(x) (((x) >= '0' && (x) <= '9') || \
                              ((x) >= 'a' && (x) <= 'z') || \
                              ((x) >= 'A' && (x) <= 'Z') || \
                               (x) == '_' || (x) == '/')
                        likely_a_tag = 0;
                        break;
                    }
                }


                        }

                        }

                            else if (strcmp(last_tag->face, cur_tag->face))
                                av_bprintf(dst, "{\\fn%s}", last_tag->face);
                        }


                                }
                            }
                        }
                    }
                           (!tagname[2] || (tagname[2] == '/' && !tagname[3]))) {
                    av_bprintf(dst, "\\N");
                    in += skip;
                } else {
                }
            } else {
            }
            break;
        }
    }

        return AVERROR(ENOMEM);

        dst->len -= 2;

    return 0;
}
