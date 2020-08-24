/*
 * FFV1 decoder template
 *
 * Copyright (c) 2003-2016 Michael Niedermayer <michaelni@gmx.at>
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

                                                 TYPE *sample[2],
                                                 int plane_index, int bits)
{

        return AVERROR_INVALIDDATA;

        int i;
        for (x = 0; x < w; x++) {
            int v = 0;
            for (i=0; i<bits; i++) {
                uint8_t state = 128;
                v += v + get_rac(c, &state);
            }
            sample[1][x] = v;
        }
        return 0;
    }


                return AVERROR_INVALIDDATA;
        }

        } else
            sign = 0;


        } else {
                run_mode = 1;

                    } else {
                        else
                            run_count = 0;
                        run_mode = 2;
                    }
                }
                    }
                } else {
                }
                }
                                               bits);
                } else
                    diff = 0;
            } else

                    run_count, run_index, run_mode, x, get_bits_count(&s->gb));
        }


    }
}

{

    }





            else
                return ret;
        }

            }

                    *((uint16_t*)(src[3] + x*2 + stride[3]*y)) = a;
            } else {
                *((uint16_t*)(src[0] + x*2 + stride[0]*y)) = b;
                *((uint16_t*)(src[1] + x*2 + stride[1]*y)) = g;
                *((uint16_t*)(src[2] + x*2 + stride[2]*y)) = r;
            }
        }
    }
    return 0;
}
