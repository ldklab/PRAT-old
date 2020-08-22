/*
 * This file is part of the Independent JPEG Group's software.
 *
 * The authors make NO WARRANTY or representation, either express or implied,
 * with respect to this software, its quality, accuracy, merchantability, or
 * fitness for a particular purpose.  This software is provided "AS IS", and
 * you, its user, assume the entire risk as to its quality and accuracy.
 *
 * This software is copyright (C) 1991, 1992, Thomas G. Lane.
 * All Rights Reserved except as specified below.
 *
 * Permission is hereby granted to use, copy, modify, and distribute this
 * software (or portions thereof) for any purpose, without fee, subject to
 * these conditions:
 * (1) If any part of the source code for this software is distributed, then
 * this README file must be included, with this copyright and no-warranty
 * notice unaltered; and any additions, deletions, or changes to the original
 * files must be clearly indicated in accompanying documentation.
 * (2) If only executable code is distributed, then the accompanying
 * documentation must state that "this software is based in part on the work
 * of the Independent JPEG Group".
 * (3) Permission for use of this software is granted only if the user accepts
 * full responsibility for any undesirable consequences; the authors accept
 * NO LIABILITY for damages of any kind.
 *
 * These conditions apply to any software derived from or based on the IJG
 * code, not just to the unmodified library.  If you use our work, you ought
 * to acknowledge us.
 *
 * Permission is NOT granted for the use of any IJG author's name or company
 * name in advertising or publicity relating to this software or products
 * derived from it.  This software may be referred to only as "the Independent
 * JPEG Group's software".
 *
 * We specifically permit and encourage the use of this software as the basis
 * of commercial products, provided that all warranty or liability claims are
 * assumed by the product vendor.
 *
 * This file contains the basic inverse-DCT transformation subroutine.
 *
 * This implementation is based on an algorithm described in
 *   C. Loeffler, A. Ligtenberg and G. Moschytz, "Practical Fast 1-D DCT
 *   Algorithms with 11 Multiplications", Proc. Int'l. Conf. on Acoustics,
 *   Speech, and Signal Processing 1989 (ICASSP '89), pp. 988-991.
 * The primary algorithm described there uses 11 multiplies and 29 adds.
 * We use their alternate method with 12 multiplies and 32 adds.
 * The advantage of this method is that no data path contains more than one
 * multiplication; this allows a very simple and accurate implementation in
 * scaled fixed-point arithmetic, with a minimal number of shifts.
 *
 * I've made lots of modifications to attempt to take advantage of the
 * sparse nature of the DCT matrices we're getting.  Although the logic
 * is cumbersome, it's straightforward and the resulting code is much
 * faster.
 *
 * A better way to do this would be to pass in the DCT block as a sparse
 * matrix, perhaps with the difference cases encoded.
 */

/**
 * @file
 * Independent JPEG Group's LLM idct.
 */

#include "libavutil/common.h"
#include "libavutil/intreadwrite.h"

#include "dct.h"
#include "idctdsp.h"

#define EIGHT_BIT_SAMPLES

#define DCTSIZE 8
#define DCTSIZE2 64

#define GLOBAL

#define RIGHT_SHIFT(x, n) ((x) >> (n))

typedef int16_t DCTBLOCK[DCTSIZE2];

#define CONST_BITS 13

/*
 * This routine is specialized to the case DCTSIZE = 8.
 */

#if DCTSIZE != 8
  Sorry, this code only copes with 8x8 DCTs. /* deliberate syntax err */
#endif


/*
 * A 2-D IDCT can be done by 1-D IDCT on each row followed by 1-D IDCT
 * on each column.  Direct algorithms are also available, but they are
 * much more complex and seem not to be any faster when reduced to code.
 *
 * The poop on this scaling stuff is as follows:
 *
 * Each 1-D IDCT step produces outputs which are a factor of sqrt(N)
 * larger than the true IDCT outputs.  The final outputs are therefore
 * a factor of N larger than desired; since N=8 this can be cured by
 * a simple right shift at the end of the algorithm.  The advantage of
 * this arrangement is that we save two multiplications per 1-D IDCT,
 * because the y0 and y4 inputs need not be divided by sqrt(N).
 *
 * We have to do addition and subtraction of the integer inputs, which
 * is no problem, and multiplication by fractional constants, which is
 * a problem to do in integer arithmetic.  We multiply all the constants
 * by CONST_SCALE and convert them to integer constants (thus retaining
 * CONST_BITS bits of precision in the constants).  After doing a
 * multiplication we have to divide the product by CONST_SCALE, with proper
 * rounding, to produce the correct output.  This division can be done
 * cheaply as a right shift of CONST_BITS bits.  We postpone shifting
 * as long as possible so that partial sums can be added together with
 * full fractional precision.
 *
 * The outputs of the first pass are scaled up by PASS1_BITS bits so that
 * they are represented to better-than-integral precision.  These outputs
 * require BITS_IN_JSAMPLE + PASS1_BITS + 3 bits; this fits in a 16-bit word
 * with the recommended scaling.  (To scale up 12-bit sample data further, an
 * intermediate int32 array would be needed.)
 *
 * To avoid overflow of the 32-bit intermediate results in pass 2, we must
 * have BITS_IN_JSAMPLE + CONST_BITS + PASS1_BITS <= 26.  Error analysis
 * shows that the values given below are the most effective.
 */

#ifdef EIGHT_BIT_SAMPLES
#define PASS1_BITS  2
#else
#define PASS1_BITS  1   /* lose a little precision to avoid overflow */
#endif

#define ONE         ((int32_t) 1)

#define CONST_SCALE (ONE << CONST_BITS)

/* Convert a positive real constant to an integer scaled by CONST_SCALE.
 * IMPORTANT: if your compiler doesn't do this arithmetic at compile time,
 * you will pay a significant penalty in run time.  In that case, figure
 * the correct integer constant values and insert them by hand.
 */

/* Actually FIX is no longer used, we precomputed them all */
#define FIX(x)  ((int32_t) ((x) * CONST_SCALE + 0.5))

/* Descale and correctly round an int32_t value that's scaled by N bits.
 * We assume RIGHT_SHIFT rounds towards minus infinity, so adding
 * the fudge factor is correct for either sign of X.
 */

#define DESCALE(x,n)  RIGHT_SHIFT((x) + (ONE << ((n)-1)), n)

/* Multiply an int32_t variable by an int32_t constant to yield an int32_t result.
 * For 8-bit samples with the recommended scaling, all the variable
 * and constant values involved are no more than 16 bits wide, so a
 * 16x16->32 bit multiply can be used instead of a full 32x32 multiply;
 * this provides a useful speedup on many machines.
 * There is no way to specify a 16x16->32 multiply in portable C, but
 * some C compilers will do the right thing if you provide the correct
 * combination of casts.
 * NB: for 12-bit samples, a full 32-bit multiplication will be needed.
 */

#ifdef EIGHT_BIT_SAMPLES
#ifdef SHORTxSHORT_32           /* may work if 'int' is 32 bits */
#define MULTIPLY(var,const)  (((int16_t) (var)) * ((int16_t) (const)))
#endif
#ifdef SHORTxLCONST_32          /* known to work with Microsoft C 6.0 */
#define MULTIPLY(var,const)  (((int16_t) (var)) * ((int32_t) (const)))
#endif
#endif

#ifndef MULTIPLY                /* default definition */
#define MULTIPLY(var,const)  ((var) * (const))
#endif


/*
  Unlike our decoder where we approximate the FIXes, we need to use exact
ones here or successive P-frames will drift too much with Reference frame coding
*/
#define FIX_0_211164243 1730
#define FIX_0_275899380 2260
#define FIX_0_298631336 2446
#define FIX_0_390180644 3196
#define FIX_0_509795579 4176
#define FIX_0_541196100 4433
#define FIX_0_601344887 4926
#define FIX_0_765366865 6270
#define FIX_0_785694958 6436
#define FIX_0_899976223 7373
#define FIX_1_061594337 8697
#define FIX_1_111140466 9102
#define FIX_1_175875602 9633
#define FIX_1_306562965 10703
#define FIX_1_387039845 11363
#define FIX_1_451774981 11893
#define FIX_1_501321110 12299
#define FIX_1_662939225 13623
#define FIX_1_847759065 15137
#define FIX_1_961570560 16069
#define FIX_2_053119869 16819
#define FIX_2_172734803 17799
#define FIX_2_562915447 20995
#define FIX_3_072711026 25172

/*
 * Perform the inverse DCT on one block of coefficients.
 */

{

  /* Pass 1: process rows. */
  /* Note results are scaled up by sqrt(8) compared to a true IDCT; */
  /* furthermore, we scale the results by 2**PASS1_BITS. */


    /* Due to quantization, we will usually find that many of the input
     * coefficients are zero, especially the AC terms.  We can exploit this
     * by short-circuiting the IDCT calculation for any row in which all
     * the AC terms are zero.  In that case each output is equal to the
     * DC coefficient (with scale factor as needed).
     * With typical images and quantization tables, half or more of the
     * row DCT calculations can be simplified this way.
     */


    /* WARNING: we do the same permutation as MMX idct to simplify the
       video core */

      /* AC terms all zero */
          /* Compute a 32 bit value to assign. */

      }

    }

    /* Even part: reverse the even part of the forward DCT. */
    /* The rotator is sqrt(2)*c(-6). */
{
                    /* d0 != 0, d2 != 0, d4 != 0, d6 != 0 */


            } else {
                    /* d0 != 0, d2 == 0, d4 != 0, d6 != 0 */


            }
    } else {
                    /* d0 != 0, d2 != 0, d4 != 0, d6 == 0 */


            } else {
                    /* d0 != 0, d2 == 0, d4 != 0, d6 == 0 */
            }
      }

    /* Odd part per figure 8; the matrix is unitary and hence its
     * transpose is its inverse.  i0..i3 are y7,y5,y3,y1 respectively.
     */

                    /* d1 != 0, d3 != 0, d5 != 0, d7 != 0 */



                } else {
                    /* d1 == 0, d3 != 0, d5 != 0, d7 != 0 */



                }
            } else {
                    /* d1 != 0, d3 == 0, d5 != 0, d7 != 0 */



                } else {
                    /* d1 == 0, d3 == 0, d5 != 0, d7 != 0 */


                }
            }
        } else {
                    /* d1 != 0, d3 != 0, d5 == 0, d7 != 0 */



                } else {
                    /* d1 == 0, d3 != 0, d5 == 0, d7 != 0 */


                }
            } else {
                    /* d1 != 0, d3 == 0, d5 == 0, d7 != 0 */


                } else {
                    /* d1 == 0, d3 == 0, d5 == 0, d7 != 0 */
                }
            }
        }
    } else {
                    /* d1 != 0, d3 != 0, d5 != 0, d7 == 0 */



                } else {
                    /* d1 == 0, d3 != 0, d5 != 0, d7 == 0 */


                }
            } else {
                    /* d1 != 0, d3 == 0, d5 != 0, d7 == 0 */


                } else {
                    /* d1 == 0, d3 == 0, d5 != 0, d7 == 0 */
                }
            }
        } else {
                    /* d1 != 0, d3 != 0, d5 == 0, d7 == 0 */

                } else {
                    /* d1 == 0, d3 != 0, d5 == 0, d7 == 0 */
                }
            } else {
                    /* d1 != 0, d3 == 0, d5 == 0, d7 == 0 */
                } else {
                    /* d1 == 0, d3 == 0, d5 == 0, d7 == 0 */
                    tmp0 = tmp1 = tmp2 = tmp3 = 0;
                }
            }
        }
    }
}
    /* Final output stage: inputs are tmp10..tmp13, tmp0..tmp3 */


  }

  /* Pass 2: process columns. */
  /* Note that we must descale the results by a factor of 8 == 2**3, */
  /* and also undo the PASS1_BITS scaling. */

  dataptr = data;
    /* Columns of zeroes can be exploited in the same way as we did with rows.
     * However, the row calculation has created many nonzero AC terms, so the
     * simplification applies less often (typically 5% to 10% of the time).
     * On machines with very fast multiplication, it's possible that the
     * test takes more time than it's worth.  In that case this section
     * may be commented out.
     */


    /* Even part: reverse the even part of the forward DCT. */
    /* The rotator is sqrt(2)*c(-6). */
                    /* d0 != 0, d2 != 0, d4 != 0, d6 != 0 */


            } else {
                    /* d0 != 0, d2 == 0, d4 != 0, d6 != 0 */


            }
    } else {
                    /* d0 != 0, d2 != 0, d4 != 0, d6 == 0 */


            } else {
                    /* d0 != 0, d2 == 0, d4 != 0, d6 == 0 */
            }
    }

    /* Odd part per figure 8; the matrix is unitary and hence its
     * transpose is its inverse.  i0..i3 are y7,y5,y3,y1 respectively.
     */
                    /* d1 != 0, d3 != 0, d5 != 0, d7 != 0 */



                } else {
                    /* d1 == 0, d3 != 0, d5 != 0, d7 != 0 */



                }
            } else {
                    /* d1 != 0, d3 == 0, d5 != 0, d7 != 0 */



                } else {
                    /* d1 == 0, d3 == 0, d5 != 0, d7 != 0 */


                }
            }
        } else {
                    /* d1 != 0, d3 != 0, d5 == 0, d7 != 0 */



                } else {
                    /* d1 == 0, d3 != 0, d5 == 0, d7 != 0 */


                }
            } else {
                    /* d1 != 0, d3 == 0, d5 == 0, d7 != 0 */


                } else {
                    /* d1 == 0, d3 == 0, d5 == 0, d7 != 0 */
                }
            }
        }
    } else {
                    /* d1 != 0, d3 != 0, d5 != 0, d7 == 0 */



                } else {
                    /* d1 == 0, d3 != 0, d5 != 0, d7 == 0 */


                }
            } else {
                    /* d1 != 0, d3 == 0, d5 != 0, d7 == 0 */


                } else {
                    /* d1 == 0, d3 == 0, d5 != 0, d7 == 0 */
                }
            }
        } else {
                    /* d1 != 0, d3 != 0, d5 == 0, d7 == 0 */

                } else {
                    /* d1 == 0, d3 != 0, d5 == 0, d7 == 0 */
                }
            } else {
                    /* d1 != 0, d3 == 0, d5 == 0, d7 == 0 */
                } else {
                    /* d1 == 0, d3 == 0, d5 == 0, d7 == 0 */
                    tmp0 = tmp1 = tmp2 = tmp3 = 0;
                }
            }
        }
    }

    /* Final output stage: inputs are tmp10..tmp13, tmp0..tmp3 */

                                           CONST_BITS+PASS1_BITS+3);
                                           CONST_BITS+PASS1_BITS+3);
                                           CONST_BITS+PASS1_BITS+3);
                                           CONST_BITS+PASS1_BITS+3);
                                           CONST_BITS+PASS1_BITS+3);
                                           CONST_BITS+PASS1_BITS+3);
                                           CONST_BITS+PASS1_BITS+3);
                                           CONST_BITS+PASS1_BITS+3);

  }

#undef DCTSIZE
#define DCTSIZE 4
#define DCTSTRIDE 8

{

  /* Pass 1: process rows. */
  /* Note results are scaled up by sqrt(8) compared to a true IDCT; */
  /* furthermore, we scale the results by 2**PASS1_BITS. */



    /* Due to quantization, we will usually find that many of the input
     * coefficients are zero, especially the AC terms.  We can exploit this
     * by short-circuiting the IDCT calculation for any row in which all
     * the AC terms are zero.  In that case each output is equal to the
     * DC coefficient (with scale factor as needed).
     * With typical images and quantization tables, half or more of the
     * row DCT calculations can be simplified this way.
     */



      /* AC terms all zero */
          /* Compute a 32 bit value to assign. */

      }

    }

    /* Even part: reverse the even part of the forward DCT. */
    /* The rotator is sqrt(2)*c(-6). */
                    /* d0 != 0, d2 != 0, d4 != 0, d6 != 0 */


            } else {
                    /* d0 != 0, d2 == 0, d4 != 0, d6 != 0 */


            }
    } else {
                    /* d0 != 0, d2 != 0, d4 != 0, d6 == 0 */


            } else {
                    /* d0 != 0, d2 == 0, d4 != 0, d6 == 0 */
            }
      }

    /* Final output stage: inputs are tmp10..tmp13, tmp0..tmp3 */


  }

  /* Pass 2: process columns. */
  /* Note that we must descale the results by a factor of 8 == 2**3, */
  /* and also undo the PASS1_BITS scaling. */

  dataptr = data;
    /* Columns of zeroes can be exploited in the same way as we did with rows.
     * However, the row calculation has created many nonzero AC terms, so the
     * simplification applies less often (typically 5% to 10% of the time).
     * On machines with very fast multiplication, it's possible that the
     * test takes more time than it's worth.  In that case this section
     * may be commented out.
     */


    /* Even part: reverse the even part of the forward DCT. */
    /* The rotator is sqrt(2)*c(-6). */
                    /* d0 != 0, d2 != 0, d4 != 0, d6 != 0 */


            } else {
                    /* d0 != 0, d2 == 0, d4 != 0, d6 != 0 */


            }
    } else {
                    /* d0 != 0, d2 != 0, d4 != 0, d6 == 0 */


            } else {
                    /* d0 != 0, d2 == 0, d4 != 0, d6 == 0 */
            }
    }

    /* Final output stage: inputs are tmp10..tmp13, tmp0..tmp3 */


  }

void ff_j_rev_dct2(DCTBLOCK data){
  int d00, d01, d10, d11;

  data[0] += 4;
  d00 = data[0+0*DCTSTRIDE] + data[1+0*DCTSTRIDE];
  d01 = data[0+0*DCTSTRIDE] - data[1+0*DCTSTRIDE];
  d10 = data[0+1*DCTSTRIDE] + data[1+1*DCTSTRIDE];
  d11 = data[0+1*DCTSTRIDE] - data[1+1*DCTSTRIDE];

  data[0+0*DCTSTRIDE]= (d00 + d10)>>3;
  data[1+0*DCTSTRIDE]= (d01 + d11)>>3;
  data[0+1*DCTSTRIDE]= (d00 - d10)>>3;
  data[1+1*DCTSTRIDE]= (d01 - d11)>>3;
}

void ff_j_rev_dct1(DCTBLOCK data){
  data[0] = (data[0] + 4)>>3;
}

#undef FIX
#undef CONST_BITS

{

{
