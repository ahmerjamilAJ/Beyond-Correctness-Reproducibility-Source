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

/* BEEBS minver benchmark

   This version, copyright (C) 2014-2019 Embecosm Limited and University of
   Bristol

   Contributor Pierre Langlois <pierre.langlois@embecosm.com>
   Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

   This file is part of Embench and was formerly part of the Bristol/Embecosm
   Embedded Benchmark Suite.

   SPDX-License-Identifier: GPL-3.0-or-later

   *************************************************************************
   *                                                                       *
   *   SNU-RT Benchmark Suite for Worst Case Timing Analysis               *
   *   =====================================================               *
   *                              Collected and Modified by S.-S. Lim      *
   *                                           sslim@archi.snu.ac.kr       *
   *                                         Real-Time Research Group      *
   *                                        Seoul National University      *
   *                                                                       *
   *                                                                       *
   *        < Features > - restrictions for our experimental environment   *
   *                                                                       *
   *          1. Completely structured.                                    *
   *               - There are no unconditional jumps.                     *
   *               - There are no exit from loop bodies.                   *
   *                 (There are no 'break' or 'return' in loop bodies)     *
   *          2. No 'switch' statements.                                   *
   *          3. No 'do..while' statements.                                *
   *          4. Expressions are restricted.                               *
   *               - There are no multiple expressions joined by 'or',     *
   *                'and' operations.                                      *
   *          5. No library calls.                                         *
   *               - All the functions needed are implemented in the       *
   *                 source file.                                          *
   *                                                                       *
   *                                                                       *
   *************************************************************************
   *                                                                       *
   *  FILE: minver.c                                                       *
   *  SOURCE : Turbo C Programming for Engineering by Hyun Soo Ahn         *
   *                                                                       *
   *  DESCRIPTION :                                                        *
   *                                                                       *
   *     Matrix inversion for 3x3 floating point matrix.                   *
   *                                                                       *
   *  REMARK :                                                             *
   *                                                                       *
   *  EXECUTION TIME :                                                     *
   *                                                                       *
   *                                                                       *
   *************************************************************************

*/

#include <math.h>
#include <string.h>


/* This scale factor will be changed to equalise the runtime of the
   benchmarks. */
#define LOCAL_SCALE_FACTOR 555

int minver (int row, int col, float eps);
int mmul (int row_a, int col_a, int row_b, int col_b);

static float a_ref[3][3] = {
  {3.0, -6.0, 7.0},
  {9.0, 0.0, -5.0},
  {5.0, -8.0, 6.0},
};

static float b[3][3] = {
  {-3.0, 0.0, 2.0},
  {3.0, -2.0, 0.0},
  {0.0, 2.0, -3.0},
};

static float a[3][3], c[3][3], d[3][3], det;

static float
minver_fabs (float n)
{
  float f;

  if (n >= 0)
    f = n;
  else
    f = -n;
  return f;
}

int
mmul (int row_a, int col_a, int row_b, int col_b)
{
  int i, j, k, row_c, col_c;
  float w;

  row_c = row_a;
  col_c = col_b;

  if (row_c < 1 || row_b < 1 || col_c < 1 || col_a != row_b)
    return (999);
  for (i = 0; i < row_c; i++)
    {
      for (j = 0; j < col_c; j++)
	{
	  w = 0.0;
	  for (k = 0; k < row_b; k++)
	    w += a[i][k] * b[k][j];
	  c[i][j] = w;
	}
    }

  return (0);
}


int
minver (int row, int col, float eps)
{
  int work[500], i, j, k, r, iw, u, v;
  float w, wmax, pivot, api, w1;

  r = w = 0;
  if (row < 2 || row > 500 || eps <= 0.0)
    return (999);
  w1 = 1.0;
  for (i = 0; i < row; i++)
    work[i] = i;
  for (k = 0; k < row; k++)
    {
      wmax = 0.0;
      for (i = k; i < row; i++)
	{
	  w = minver_fabs (a[i][k]);
	  if (w > wmax)
	    {
	      wmax = w;
	      r = i;
	    }
	}
      pivot = a[r][k];
      api = minver_fabs (pivot);
      if (api <= eps)
	{
	  det = w1;
	  return (1);
	}
      w1 *= pivot;
      u = k * col;
      v = r * col;
      if (r != k)
	{
	  w1 = -w;
	  iw = work[k];
	  work[k] = work[r];
	  work[r] = iw;
	  for (j = 0; j < row; j++)
	    {
	      w = a[k][j];
	      a[k][j] = a[r][j];
	      a[r][j] = w;
	    }
	}
      for (i = 0; i < row; i++)
	a[k][i] /= pivot;
      for (i = 0; i < row; i++)
	{
	  if (i != k)
	    {
	      v = i * col;
	      w = a[i][k];
	      if (w != 0.0)
		{
		  for (j = 0; j < row; j++)
		    if (j != k)
		      a[i][j] -= w * a[k][j];
		  a[i][k] = -w / pivot;
		}
	    }
	}
      a[k][k] = 1.0 / pivot;
    }

  for (i = 0; i < row; i++)
    {
      while (1)
	{
	  k = work[i];
	  if (k == i)
	    break;
	  iw = work[k];
	  work[k] = work[i];
	  work[i] = iw;
	  for (j = 0; j < row; j++)
	    {
	      u = j * col;
	      w = a[k][i];
	      a[k][i] = a[k][k];
	      a[k][k] = w;
	    }
	}
    }

  det = w1;

  return (0);
}


int
verify_benchmark (int res __attribute ((unused)))
{
  int i, j;
  float eps = 1.0e-6;

  static float c_exp[3][3] = {
    {-27.0, 26.0, -15.0},
    {-27.0, -10.0, 33.0},
    {-39.0, 28.0, -8.0}
  };

  static float d_exp[3][3] = {
    {0.133333325, -0.199999958, 0.2666665910},
    {-0.519999862, 0.113333330, 0.5266665220},
    {0.479999840, -0.359999895, 0.0399999917}
  };

  /* Allow small errors in floating point */

  for (i = 0; i < 3; i++)
    for (j = 0; j < 3; j++)
      if (float_neq_beebs(c[i][j], c_exp[i][j]) || float_neq_beebs(d[i][j], d_exp[i][j]))
	return 0;

  return float_eq_beebs(det, -16.6666718);
}


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
  int i;

  for (i = 0; i < rpt; i++)
    {
      float eps = 1.0e-6;

      memcpy (a, a_ref, 3 * 3 * sizeof (a[0][0]));
      minver (3, 3, eps);
      memcpy (d, a, 3 * 3 * sizeof (a[0][0]));
      memcpy (a, a_ref, 3 * 3 * sizeof (a[0][0]));
      mmul (3, 3, 3, 3);
    }

  return 0;
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
