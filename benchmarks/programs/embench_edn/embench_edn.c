/* Beyond Correctness source packaging notice, 2026-10-25.
 * This is the research-adapted/amalgamated benchmark version.
 * Its upstream copyright and license notices are retained below.
 * This release adds only this notice and a line-number directive.
 * See ../../licenses/ and the benchmark catalog for notices.
 */
#line 1
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define CPU_MHZ 1
/* BEEBS local library variants header

   Copyright (C) 2019 Embecosm Limited.

   Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

   This file is part of Embench and was formerly part of the Bristol/Embecosm
   Embedded Benchmark Suite.

   SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef BEEBSC_H
#define BEEBSC_H

#include <stddef.h>

/* BEEBS fixes RAND_MAX to its lowest permitted value, 2^15-1 */

#ifdef RAND_MAX
#undef RAND_MAX
#endif
#define RAND_MAX ((1U << 15) - 1)

/* Common understanding of a "small value" (epsilon) for floating point
   comparisons. */

#define VERIFY_DOUBLE_EPS 1.0e-13
#define VERIFY_FLOAT_EPS 1.0e-5

/* Simplified assert.

   The full complexity of assert is not needed for a benchmark. See the
   discussion at:

   https://lists.librecores.org/pipermail/embench/2019-August/000007.html 

   This function just*/

#define assert_beebs(expr) { if (!(expr)) exit (1); }

#define float_eq_beebs(exp, actual) (fabsf(exp - actual) < VERIFY_FLOAT_EPS)
#define float_neq_beebs(exp, actual) !float_eq_beebs(exp, actual)
#define double_eq_beebs(exp, actual) (fabs(exp - actual) < VERIFY_DOUBLE_EPS)
#define double_neq_beebs(exp, actual) !double_eq_beebs(exp, actual)

/* Local simplified versions of library functions */

int rand_beebs (void);
void srand_beebs (unsigned int new_seed);

void init_heap_beebs (void *heap, const size_t heap_size);
int check_heap_beebs (void *heap);
void *malloc_beebs (size_t size);
void *calloc_beebs (size_t nmemb, size_t size);
void *realloc_beebs (void *ptr, size_t size);
void free_beebs (void *ptr);
#endif /* BEEBSC_H */


/*
   Local Variables:
   mode: C
   c-file-style: "gnu"
   End:
*/

/* Support header for BEEBS.

   Copyright (C) 2014 Embecosm Limited and the University of Bristol
   Copyright (C) 2019 Embecosm Limited

   Contributor James Pallister <james.pallister@bristol.ac.uk>

   Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

   This file is part of Embench and was formerly part of the Bristol/Embecosm
   Embedded Benchmark Suite.

   SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef SUPPORT_H
#define SUPPORT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* Include board support header if we have one */

#ifdef HAVE_BOARDSUPPORT_H
#include "boardsupport.h"
#endif

/* Benchmarks must implement verify_benchmark, which must return -1 if no
   verification is done. */

int verify_benchmark (int result);

/* Standard functions implemented for each board */

void initialise_board (void);
void start_trigger (void);
void stop_trigger (void);

/* Every benchmark implements this for one-off data initialization.  This is
   only used for initialization that is independent of how often benchmark ()
   is called. */

void initialise_benchmark (void);

/* Every benchmark implements this for cache warm up, typically calling
   benchmark several times. The argument controls how much warming up is
   done, with 0 meaning no warming. */

void warm_caches (int temperature);

/* Every benchmark implements this as its entry point. Don't allow it to be
   inlined! */

int benchmark (void) __attribute__ ((noinline));

/* Every benchmark must implement this to validate the result of the
   benchmark. */

int verify_benchmark (int res);

/* Local simplified versions of library functions */



#endif /* SUPPORT_H */

/*
   Local Variables:
   mode: C
   c-file-style: "gnu"
   End:
*/


/* BEEBS local library variants

   Copyright (C) 2019 Embecosm Limited.

   Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

   This file is part of Embench and was formerly part of the Bristol/Embecosm
   Embedded Benchmark Suite.

   SPDX-License-Identifier: GPL-3.0-or-later */

/* These are very simple local versions of library routines, to ensure the
   code is compiled with the flags used for the benchmark.  Not all library
   routines are here, just ones that cause a lot of unecessary load, or where
   there is variation between platforms and architectures. */

#include <stddef.h>
#include <string.h>


/* Seed for the random number generator */

static long int seed = 0;

/* Heap records and sane initial values */

static void *heap_ptr = NULL;
static void *heap_end = NULL;
static size_t heap_requested = 0;


/* Yield a sequence of random numbers in the range [0, 2^15-1].

   long int is guaranteed to be at least 32 bits. The seed only ever uses 31
   bits (so is positive).

   For BEEBS this gets round different operating systems using different
   multipliers and offsets and RAND_MAX variations. */

int
rand_beebs (void)
{
  seed = (seed * 1103515245L + 12345) & ((1UL << 31) - 1);
  return (int) (seed >> 16);
}


/* Initialize the random number generator */

void
srand_beebs (unsigned int new_seed)
{
  seed = (long int) new_seed;
}


/* Initialize the BEEBS heap pointers. Note that the actual memory block is
   in the caller code. */

void
init_heap_beebs (void *heap, size_t heap_size)
{
  heap_ptr = (void *) heap;
  heap_end = (void *) ((char *) heap_ptr + heap_size);
  heap_requested = 0;
}


/* Report if malloc ever failed.

   Return non-zero (TRUE) if malloc did not reqest more than was available
   since the last call to init_heap_beebs, zero (FALSE) otherwise. */

int
check_heap_beebs (void *heap)
{
  return ((void *) ((char *) heap + heap_requested) <= heap_end);
}


/* BEEBS version of malloc.

   This is primarily to reduce library and OS dependencies. Malloc is
   generally not used in embedded code, or if it is, only in well defined
   contexts to pre-allocate a fixed amount of memory. So this simplistic
   implementation is just fine.

   Note in particular the assumption that memory will never be freed! */

void *
malloc_beebs (size_t size)
{
  void *new_ptr = heap_ptr;

  heap_requested += size;

  if (((void *) ((char *) heap_ptr + size) > heap_end) || (0 == size))
    return NULL;
  else
    {
      heap_ptr = (void *) ((char *) heap_ptr + size);
      return new_ptr;
    }
}


/* BEEBS version of calloc.

   Implement as wrapper for malloc */

void *
calloc_beebs (size_t nmemb, size_t size)
{
  void *new_ptr = malloc_beebs (nmemb * size);

  /* Calloc is defined to zero the memory. OK to use a function here, because
     it will be handled specially by the compiler anyway. */

  if (NULL != new_ptr)
    memset (new_ptr, 0, nmemb * size);

  return new_ptr;
}


/* BEEBS version of realloc.

   This is primarily to reduce library and OS dependencies. We just have to
   allocate new memory and copy stuff across. */

void *
realloc_beebs (void *ptr, size_t size)
{
  void *new_ptr = heap_ptr;

  heap_requested += size;

  if (((void *) ((char *) heap_ptr + size) > heap_end) || (0 == size))
    return NULL;
  else
    {
      heap_ptr = (void *) ((char *) heap_ptr + size);

      /* This is clunky, since we don't know the size of the original
         pointer. However it is a read only action and we know it must
         be big enough if we right off the end, or we couldn't have
         allocated here. If the size is smaller, it doesn't matter. */

      if (NULL != ptr)
	{
	  size_t i;

	  for (i = 0; i < size; i++)
	    ((char *) new_ptr)[i] = ((char *) ptr)[i];
	}

      return new_ptr;
    }
}


/* BEEBS version of free.

   For our simplified version of memory handling, free can just do nothing. */

void
free_beebs (void *ptr __attribute__ ((unused)))
{
}


/*
   Local Variables:
   mode: C
   c-file-style: "gnu"
   End:
*/

/* BEEBS edn benchmark

   This version, copyright (C) 2014-2019 Embecosm Limited and University of
   Bristol

   Contributor James Pallister <james.pallister@bristol.ac.uk>
   Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

   This file is part of Embench and was formerly part of the Bristol/Embecosm
   Embedded Benchmark Suite.

   SPDX-License-Identifier: GPL-3.0-or-later

   Original code from: WCET Benchmarks,
       http://www.mrtc.mdh.se/projects/wcet/benchmarks.html

   Permission to license under GPL obtained by email from Björn Lisper */

/*
 * MDH WCET BENCHMARK SUITE.
 */

/************************************************************************
*	Simple vector multiply				*
************************************************************************/

/*
 * Changes: JG 2005/12/22: Inserted prototypes, changed type of main to int
 * etc. Added parenthesis in expressions in jpegdct. Removed unused variable
 * dx. Changed int to long to avoid problems when compiling to 16 bit target
 * Indented program.
 * JG 2006-01-27: Removed code in codebook
 */

#include <string.h>


/* This scale factor will be changed to equalise the runtime of the
   benchmarks. */
#define LOCAL_SCALE_FACTOR 87


#define N 100
#define ORDER 50

void vec_mpy1 (short y[], const short x[], short scaler);
long int mac (const short *a, const short *b, long int sqr, long int *sum);
void fir (const short array1[], const short coeff[], long int output[]);
void fir_no_red_ld (const short x[], const short h[], long int y[]);
long int latsynth (short b[], const short k[], long int n, long int f);
void iir1 (const short *coefs, const short *input, long int *optr,
	   long int *state);
long int codebook (long int mask, long int bitchanged, long int numbasis,
		   long int codeword, long int g, const short *d, short ddim,
		   short theta);
void jpegdct (short *d, short *r);

void
vec_mpy1 (short y[], const short x[], short scaler)
{
  long int i;

  for (i = 0; i < 150; i++)
    y[i] += ((scaler * x[i]) >> 15);
}


/*****************************************************
*			Dot Product	      *
*****************************************************/
long int
mac (const short *a, const short *b, long int sqr, long int *sum)
{
  long int i;
  long int dotp = *sum;

  for (i = 0; i < 150; i++)
    {
      dotp += b[i] * a[i];
      sqr += b[i] * b[i];
    }

  *sum = dotp;
  return sqr;
}


/*****************************************************
*		FIR Filter		     *
*****************************************************/
void
fir (const short array1[], const short coeff[], long int output[])
{
  long int i, j, sum;

  for (i = 0; i < N - ORDER; i++)
    {
      sum = 0;
      for (j = 0; j < ORDER; j++)
	{
	  sum += array1[i + j] * coeff[j];
	}
      output[i] = sum >> 15;
    }
}

/****************************************************
*	FIR Filter with Redundant Load Elimination

By doing two outer loops simultaneously, you can potentially  reuse data (depending on the DSP architecture).
x and h  only  need to be loaded once, therefore reducing redundant loads.
This reduces memory bandwidth and power.
*****************************************************/
void
fir_no_red_ld (const short x[], const short h[], long int y[])
{
  long int i, j;
  long int sum0, sum1;
  short x0, x1, h0, h1;
  for (j = 0; j < 100; j += 2)
    {
      sum0 = 0;
      sum1 = 0;
      x0 = x[j];
      for (i = 0; i < 32; i += 2)
	{
	  x1 = x[j + i + 1];
	  h0 = h[i];
	  sum0 += x0 * h0;
	  sum1 += x1 * h0;
	  x0 = x[j + i + 2];
	  h1 = h[i + 1];
	  sum0 += x1 * h1;
	  sum1 += x0 * h1;
	}
      y[j] = sum0 >> 15;
      y[j + 1] = sum1 >> 15;
    }
}

/*******************************************************
*	Lattice Synthesis	           *
* This function doesn't follow the typical DSP multiply two  vector operation, but it will point out the compiler's flexibility   ********************************************************/
long int
latsynth (short b[], const short k[], long int n, long int f)
{
  long int i;

  f -= b[n - 1] * k[n - 1];
  for (i = n - 2; i >= 0; i--)
    {
      f -= b[i] * k[i];
      b[i + 1] = b[i] + ((k[i] * (f >> 16)) >> 16);
    }
  b[0] = f >> 16;
  return f;
}

/*****************************************************
*			IIR Filter		     *
*****************************************************/
void
iir1 (const short *coefs, const short *input, long int *optr, long int *state)
{
  long int x;
  long int t;
  long int n;

  x = input[0];
  for (n = 0; n < 50; n++)
    {
      t = x + ((coefs[2] * state[0] + coefs[3] * state[1]) >> 15);
      x = t + ((coefs[0] * state[0] + coefs[1] * state[1]) >> 15);
      state[1] = state[0];
      state[0] = t;
      coefs += 4;		/* point to next filter coefs  */
      state += 2;		/* point to next filter states */
    }
  *optr++ = x;
}

/*****************************************************
*	Vocoder Codebook Search 	     *
*****************************************************/
long int
codebook (long int mask, long int bitchanged, long int numbasis,
	  long int codeword, long int g, const short *d, short ddim,
	  short theta)
/*
 * dfm (mask=d  bitchanged=1 numbasis=17  codeword=e[0] , g=d, d=a, ddim=c,
 * theta =1
 */
{
  long int j;


  /*
   * Remove along with the code below.
   *
   long int        tmpMask;

   tmpMask = mask << 1;
   */
  for (j = bitchanged + 1; j <= numbasis; j++)
    {



/*
 * The following code is removed since it gave a memory access exception.
 * It is OK since the return value does not control the flow.
 * The loop always iterates a fixed number of times independent of the loop body.

    if (theta == !(!(codeword & tmpMask)))
			g += *(d + bitchanged * ddim + j);
		else
			g -= *(d + bitchanged * ddim + j);
		tmpMask <<= 1;
*/
    }
  return g;
}


/*****************************************************
*		JPEG Discrete Cosine Transform 		     *
*****************************************************/
void
jpegdct (short *d, short *r)
{
  long int t[12];
  short i, j, k, m, n, p;
  for (k = 1, m = 0, n = 13, p = 8; k <= 8;
       k += 7, m += 3, n += 3, p -= 7, d -= 64)
    {
      for (i = 0; i < 8; i++, d += p)
	{
	  for (j = 0; j < 4; j++)
	    {
	      t[j] = d[k * j] + d[k * (7 - j)];
	      t[7 - j] = d[k * j] - d[k * (7 - j)];
	    }
	  t[8] = t[0] + t[3];
	  t[9] = t[0] - t[3];
	  t[10] = t[1] + t[2];
	  t[11] = t[1] - t[2];
	  d[0] = (t[8] + t[10]) >> m;
	  d[4 * k] = (t[8] - t[10]) >> m;
	  t[8] = (short) (t[11] + t[9]) * r[10];
	  d[2 * k] = t[8] + (short) ((t[9] * r[9]) >> n);
	  d[6 * k] = t[8] + (short) ((t[11] * r[11]) >> n);
	  t[0] = (short) (t[4] + t[7]) * r[2];
	  t[1] = (short) (t[5] + t[6]) * r[0];
	  t[2] = t[4] + t[6];
	  t[3] = t[5] + t[7];
	  t[8] = (short) (t[2] + t[3]) * r[8];
	  t[2] = (short) t[2] * r[1] + t[8];
	  t[3] = (short) t[3] * r[3] + t[8];
	  d[7 * k] = (short) (t[4] * r[4] + t[0] + t[2]) >> n;
	  d[5 * k] = (short) (t[5] * r[6] + t[1] + t[3]) >> n;
	  d[3 * k] = (short) (t[6] * r[5] + t[1] + t[2]) >> n;
	  d[1 * k] = (short) (t[7] * r[7] + t[0] + t[3]) >> n;
	}
    }
}

static short a[200];
static short b[200];
static short c;
static long int d;
static int e;
static long int output[200];


void
initialise_benchmark (void)
{
}


static int benchmark_body (int  rpt);

void
warm_caches (int  heat)
{
  int  res = benchmark_body (heat);

  return;
}


int
benchmark (void)
{
  return benchmark_body (LOCAL_SCALE_FACTOR * CPU_MHZ);
}


static int __attribute__ ((noinline))
benchmark_body (int rpt)
{
  int j;

  for (j = 0; j < rpt; j++)
    {
      short unsigned int in_a[200] = {
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400
      };
      short unsigned int in_b[200] = {
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000
      };
      c = 0x3;
      d = 0xAAAA;
      e = 0xEEEE;

      for (int i = 0; i < 200; i++)
	{
	  a[i] = in_a[i];
	  b[i] = in_b[i];
	}
      /*
       * Declared as memory variable so it doesn't get optimized out
       */

      vec_mpy1 (a, b, c);
      c = mac (a, b, (long int) c, (long int *) output);
      fir (a, b, output);
      fir_no_red_ld (a, b, output);
      d = latsynth (a, b, N, d);
      iir1 (a, b, &output[100], output);
      e = codebook (d, 1, 17, e, d, a, c, 1);
      jpegdct (a, b);
    }
  return 0;
}

int
verify_benchmark (int unused)
{
  long int exp_output[200] =
    { 3760, 4269, 3126, 1030, 2453, -4601, 1981, -1056, 2621, 4269,
    3058, 1030, 2378, -4601, 1902, -1056, 2548, 4269, 2988, 1030,
    2300, -4601, 1822, -1056, 2474, 4269, 2917, 1030, 2220, -4601,
    1738, -1056, 2398, 4269, 2844, 1030, 2140, -4601, 1655, -1056,
    2321, 4269, 2770, 1030, 2058, -4601, 1569, -1056, 2242, 4269,
    2152, 1030, 1683, -4601, 1627, -1056, 2030, 4269, 2080, 1030,
    1611, -4601, 1555, -1056, 1958, 4269, 2008, 1030, 1539, -4601,
    1483, -1056, 1886, 4269, 1935, 1030, 1466, -4601, 1410, -1056,
    1813, 4269, 1862, 1030, 1393, -4601, 1337, -1056, 1740, 4269,
    1789, 1030, 1320, -4601, 1264, -1056, 1667, 4269, 1716, 1030,
    1968, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };

  return (0 == memcmp (output, exp_output, 200 * sizeof (output[0])))
    && (10243 == c) && (-441886230 == d) && (-441886230 == e);
}


/*
   Local Variables:
   mode: C
   c-file-style: "gnu"
   End:
*/


int main(int argc, char **argv) {
  if (argc != 2) return 2;
  FILE *fp = fopen(argv[1], "r");
  if (!fp) return 2;
  int runs = 0;
  int parsed = fscanf(fp, "%d", &runs);
  fclose(fp);
  if (parsed != 1 || runs < 1 || runs > 4) return 2;
  initialise_benchmark();
  for (int invocation = 0; invocation < runs; ++invocation) {
    volatile int result = benchmark();
    int verified = verify_benchmark(result);
    printf("invocation=%d result=%d verified=%d\n", invocation + 1, result, verified);
    if (verified != 1) return 3;
  }
  return 0;
}
