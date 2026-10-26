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

/* BEEBS nsichneu benchmark

   This version, copyright (C) 2014-2019 Embecosm Limited and University of
   Bristol

   Contributor James Pallister <james.pallister@bristol.ac.uk>
   Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

   This file is part of Embench and was formerly part of the Bristol/Embecosm
   Embedded Benchmark Suite.

   SPDX-License-Identifier: GPL-3.0-or-later */

/***************************************************************************
*
* Copyright (c) 1998/1999, C-LAB, Paderborn
*
* File generated by Sea2CHaRy
*
* This file containes a CHaRy description generated from the
* file: NSicherNeu.sea
* The name of the Pr/T-Net was: NSicherNeu
*
* Modified and maintained by
* Friedhelm Stappert
* C-LAB, Paderborn, Germany
* fst@c-lab.de
*
* Modifications:
* o made the local variables global and volatile,
*   so the compiler won't optimise everything away.
*

***************************************************************************/
// #define DO_TRACING



/* This scale factor will be changed to equalise the runtime of the
   benchmarks. */
#define LOCAL_SCALE_FACTOR 1231

#ifdef DO_TRACING		// ON PC

#include <stdio.h>
#define TRACE(x) trace((x))
#undef TEST			/* finished testing! */
void
trace (char *s)
{
  printf ("%s\n", s);
}

#else // ON TARGET

#define TRACE(x)
#undef TEST

#endif

volatile int P1_is_marked = 3;
volatile long P1_marking_member_0[3];
volatile int P2_is_marked = 5;
volatile long P2_marking_member_0[5];
volatile int P3_is_marked = 0;
volatile long P3_marking_member_0[6];

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


/**void NSicherNeu()**/
static int __attribute__ ((noinline))
benchmark_body (int rpt)
{
  int j;

  for (j = 0; j < rpt; j++)
    {
      P1_is_marked = 3;
      P2_is_marked = 5;
      P3_is_marked = 0;

      /* Permutation for Place P1 : 0, 1, 2 */
      /* Transition T1 */
      if ((P1_is_marked >= 3) &&
	  (P3_is_marked + 3 <= 6) &&
	  (P1_marking_member_0[1] == P1_marking_member_0[2]))
	{

	  long x;
	  long y;
	  long z;

	  x = P1_marking_member_0[0];
	  y = P1_marking_member_0[1];

	  /* Transition condition */
	  if (x < y)
	    {

	      /* demarking of input places */
	      P1_is_marked -= 3;

	      /* preaction */
	      z = x - y;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = x;
	      P3_marking_member_0[P3_is_marked + 1] = y;
	      P3_marking_member_0[P3_is_marked + 2] = z;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}

      /* Permutation for Place P1 : 0, 2, 1 */
      /* Transition T1 */
      if ((P1_is_marked >= 3) &&
	  (P3_is_marked + 3 <= 6) &&
	  (P1_marking_member_0[2] == P1_marking_member_0[1]))
	{

	  long x;
	  long y;
	  long z;

	  x = P1_marking_member_0[0];
	  y = P1_marking_member_0[2];

	  /* Transition condition */
	  if ((x < y))
	    {


	      /* demarking of input places */
	      P1_is_marked -= 3;

	      /* preaction */
	      z = x - y;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = x;
	      P3_marking_member_0[P3_is_marked + 1] = y;
	      P3_marking_member_0[P3_is_marked + 2] = z;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}

      /* Permutation for Place P1 : 1, 0, 2 */
      /* Transition T1 */
      if ((P1_is_marked >= 3) &&
	  (P3_is_marked + 3 <= 6) &&
	  (P1_marking_member_0[0] == P1_marking_member_0[2]))
	{

	  long x;
	  long y;
	  long z;

	  x = P1_marking_member_0[1];
	  y = P1_marking_member_0[0];

	  /* Transition condition */
	  if (x < y)
	    {


	      /* demarking of input places */
	      P1_is_marked -= 3;

	      /* preaction */
	      z = x - y;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = x;
	      P3_marking_member_0[P3_is_marked + 1] = y;
	      P3_marking_member_0[P3_is_marked + 2] = z;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}

      /* Permutation for Place P1 : 1, 2, 0 */
      /* Transition T1 */
      if ((P1_is_marked >= 3) &&
	  (P3_is_marked + 3 <= 6) &&
	  (P1_marking_member_0[2] == P1_marking_member_0[0]))
	{

	  long x;
	  long y;
	  long z;

	  x = P1_marking_member_0[1];
	  y = P1_marking_member_0[2];

	  /* Transition condition */
	  if ((x < y))
	    {


	      /* demarking of input places */
	      P1_is_marked -= 3;

	      /* preaction */
	      z = x - y;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = x;
	      P3_marking_member_0[P3_is_marked + 1] = y;
	      P3_marking_member_0[P3_is_marked + 2] = z;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}

      /* Permutation for Place P1 : 2, 0, 1 */
      /* Transition T1 */
      if ((P1_is_marked >= 3) &&
	  (P3_is_marked + 3 <= 6) &&
	  (P1_marking_member_0[0] == P1_marking_member_0[1]))
	{
	  long x;
	  long y;
	  long z;

	  x = P1_marking_member_0[2];
	  y = P1_marking_member_0[0];

	  /* Transition condition */
	  if ((x < y))
	    {

	      /* demarking of input places */
	      P1_is_marked -= 3;

	      /* preaction */
	      z = x - y;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = x;
	      P3_marking_member_0[P3_is_marked + 1] = y;
	      P3_marking_member_0[P3_is_marked + 2] = z;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}

      /* Permutation for Place P1 : 2, 1, 0 */
      /* Transition T1 */
      if ((P1_is_marked >= 3) &&
	  (P3_is_marked + 3 <= 6) &&
	  (P1_marking_member_0[1] == P1_marking_member_0[0]))
	{
	  long x;
	  long y;
	  long z;

	  x = P1_marking_member_0[2];
	  y = P1_marking_member_0[1];

	  /* Transition condition */
	  if ((x < y))
	    {

	      /* demarking of input places */
	      P1_is_marked -= 3;

	      /* preaction */
	      z = x - y;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = x;
	      P3_marking_member_0[P3_is_marked + 1] = y;
	      P3_marking_member_0[P3_is_marked + 2] = z;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}

      /* Permutation for Place P2 : 0, 1, 2, 3 */
      /* Transition T2 */
      if ((P2_is_marked >= 4) &&
	  (((P3_is_marked + 3) <= 6)) &&
	  (((P2_marking_member_0[1] == P2_marking_member_0[2])) &&
	   ((P2_marking_member_0[1] == P2_marking_member_0[3]))))
	{
	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[0];
	  b = P2_marking_member_0[1];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}

      /* Permutation for Place P2 : 0, 1, 3, 2 */
      /* Transition T2 */
      if ((P2_is_marked >= 4) &&
	  (((P3_is_marked + 3) <= 6)) &&
	  ((P2_marking_member_0[1] == P2_marking_member_0[3]) &&
	   (P2_marking_member_0[1] == P2_marking_member_0[2])))
	{
	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[0];
	  b = P2_marking_member_0[1];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}

      /* Permutation for Place P2 : 0, 2, 1, 3 */
      /* Transition T2 */
      if ((P2_is_marked >= 4) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[2] == P2_marking_member_0[1]) &&
	   (P2_marking_member_0[2] == P2_marking_member_0[3])))
	{
	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[0];
	  b = P2_marking_member_0[2];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}

      /* Permutation for Place P2 : 0, 2, 3, 1 */
      /* Transition T2 */
      if ((P2_is_marked >= 4) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[2] == P2_marking_member_0[3]) &&
	   (P2_marking_member_0[2] == P2_marking_member_0[1])))
	{
	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[0];
	  b = P2_marking_member_0[2];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}

      /* Permutation for Place P2 : 0, 3, 1, 2 */
      /* Transition T2 */
      if ((P2_is_marked >= 4) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[3] == P2_marking_member_0[1]) &&
	   (P2_marking_member_0[3] == P2_marking_member_0[2])))
	{
	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[0];
	  b = P2_marking_member_0[3];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}

      /* Permutation for Place P2 : 0, 3, 2, 1 */
      /* Transition T2 */
      if ((P2_is_marked >= 4) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[3] == P2_marking_member_0[2]) &&
	   (P2_marking_member_0[3] == P2_marking_member_0[1])))
	{
	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[0];
	  b = P2_marking_member_0[3];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}

      /* Permutation for Place P2 : 1, 0, 2, 3 */
      /* Transition T2 */
      if ((P2_is_marked >= 4) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[0] == P2_marking_member_0[2]) &&
	   (P2_marking_member_0[0] == P2_marking_member_0[3])))
	{
	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[1];
	  b = P2_marking_member_0[0];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}

      /* Permutation for Place P2 : 1, 0, 3, 2 */
      /* Transition T2 */
      if ((P2_is_marked >= 4) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[0] == P2_marking_member_0[3]) &&
	   (P2_marking_member_0[0] == P2_marking_member_0[2])))
	{
	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[1];
	  b = P2_marking_member_0[0];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}

      /* Permutation for Place P2 : 1, 2, 0, 3 */
      /* Transition T2 */
      if ((P2_is_marked >= 4) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[2] == P2_marking_member_0[0]) &&
	   (P2_marking_member_0[2] == P2_marking_member_0[3])))
	{
	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[1];
	  b = P2_marking_member_0[2];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}

      /* Permutation for Place P2 : 1, 2, 3, 0 */
      /* Transition T2 */
      if ((P2_is_marked >= 4) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[2] == P2_marking_member_0[3]) &&
	   (P2_marking_member_0[2] == P2_marking_member_0[0])))
	{
	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[1];
	  b = P2_marking_member_0[2];

	  /* Transition condition */
	  if ((b > a))
	    {
	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}

      /* Permutation for Place P2 : 1, 3, 0, 2 */
      /* Transition T2 */
      if ((P2_is_marked >= 4) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[3] == P2_marking_member_0[0]) &&
	   (P2_marking_member_0[3] == P2_marking_member_0[2])))
	{
	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[1];
	  b = P2_marking_member_0[3];

	  /* Transition condition */
	  if ((b > a))
	    {
	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 1, 3, 2, 0 */
      /* Transition T2 */
      if ((P2_is_marked >= 4) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[3] == P2_marking_member_0[2]) &&
	   (P2_marking_member_0[3] == P2_marking_member_0[0])))
	{
	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[1];
	  b = P2_marking_member_0[3];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 2, 0, 1, 3 */
      /* Transition T2 */
      if ((P2_is_marked >= 4) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[0] == P2_marking_member_0[1]) &&
	   (P2_marking_member_0[0] == P2_marking_member_0[3])))
	{
	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[2];
	  b = P2_marking_member_0[0];

	  /* Transition condition */
	  if ((b > a))
	    {
	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}

      /* Permutation for Place P2 : 2, 0, 3, 1 */
      /* Transition T2 */
      if ((P2_is_marked >= 4) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[0] == P2_marking_member_0[3]) &&
	   (P2_marking_member_0[0] == P2_marking_member_0[1])))
	{
	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[2];
	  b = P2_marking_member_0[0];

	  /* Transition condition */
	  if ((b > a))
	    {
	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}

      /* Permutation for Place P2 : 2, 1, 0, 3 */
      /* Transition T2 */
      if ((P2_is_marked >= 4) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[1] == P2_marking_member_0[0]) &&
	   (P2_marking_member_0[1] == P2_marking_member_0[3])))
	{
	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[2];
	  b = P2_marking_member_0[1];

	  /* Transition condition */
	  if ((b > a))
	    {
	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}

      /* Permutation for Place P2 : 2, 1, 3, 0 */
      /* Transition T2 */
      if ((P2_is_marked >= 4) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[1] == P2_marking_member_0[3]) &&
	   (P2_marking_member_0[1] == P2_marking_member_0[0])))
	{
	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[2];
	  b = P2_marking_member_0[1];

	  /* Transition condition */
	  if ((b > a))
	    {
	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}

      /* Permutation for Place P2 : 2, 3, 0, 1 */
      /* Transition T2 */
      if ((P2_is_marked >= 4) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[3] == P2_marking_member_0[0]) &&
	   (P2_marking_member_0[3] == P2_marking_member_0[1])))
	{
	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[2];
	  b = P2_marking_member_0[3];

	  /* Transition condition */
	  if ((b > a))
	    {
	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}

      /* Permutation for Place P2 : 2, 3, 1, 0 */
      /* Transition T2 */
      if ((P2_is_marked >= 4) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[3] == P2_marking_member_0[1]) &&
	   (P2_marking_member_0[3] == P2_marking_member_0[0])))
	{
	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[2];
	  b = P2_marking_member_0[3];

	  /* Transition condition */
	  if ((b > a))
	    {
	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}

      /* Permutation for Place P2 : 3, 0, 1, 2 */
      /* Transition T2 */
      if ((P2_is_marked >= 4) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[0] == P2_marking_member_0[1]) &&
	   (P2_marking_member_0[0] == P2_marking_member_0[2])))
	{
	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[3];
	  b = P2_marking_member_0[0];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 3, 0, 2, 1 */
      /* Transition T2 */
      if ((P2_is_marked >= 4) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[0] == P2_marking_member_0[2]) &&
	   (P2_marking_member_0[0] == P2_marking_member_0[1])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[3];
	  b = P2_marking_member_0[0];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 3, 1, 0, 2 */
      /* Transition T2 */
      if ((P2_is_marked >= 4) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[1] == P2_marking_member_0[0]) &&
	   (P2_marking_member_0[1] == P2_marking_member_0[2])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[3];
	  b = P2_marking_member_0[1];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 3, 1, 2, 0 */
      /* Transition T2 */
      if ((P2_is_marked >= 4) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[1] == P2_marking_member_0[2]) &&
	   (P2_marking_member_0[1] == P2_marking_member_0[0])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[3];
	  b = P2_marking_member_0[1];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 3, 2, 0, 1 */
      /* Transition T2 */
      if ((P2_is_marked >= 4) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[2] == P2_marking_member_0[0]) &&
	   (P2_marking_member_0[2] == P2_marking_member_0[1])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[3];
	  b = P2_marking_member_0[2];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 3, 2, 1, 0 */
      /* Transition T2 */
      if ((P2_is_marked >= 4) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[2] == P2_marking_member_0[1]) &&
	   (P2_marking_member_0[2] == P2_marking_member_0[0])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[3];
	  b = P2_marking_member_0[2];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 0, 1, 2, 4 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[1] == P2_marking_member_0[2]) &&
	   (P2_marking_member_0[1] == P2_marking_member_0[4])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[0];
	  b = P2_marking_member_0[1];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[3];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 0, 1, 3, 4 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[1] == P2_marking_member_0[3]) &&
	   (P2_marking_member_0[1] == P2_marking_member_0[4])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[0];
	  b = P2_marking_member_0[1];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[2];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 0, 1, 4, 2 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[1] == P2_marking_member_0[4]) &&
	   (P2_marking_member_0[1] == P2_marking_member_0[2])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[0];
	  b = P2_marking_member_0[1];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[3];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 0, 1, 4, 3 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[1] == P2_marking_member_0[4]) &&
	   (P2_marking_member_0[1] == P2_marking_member_0[3])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[0];
	  b = P2_marking_member_0[1];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[2];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 0, 2, 1, 4 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[2] == P2_marking_member_0[1]) &&
	   (P2_marking_member_0[2] == P2_marking_member_0[4])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[0];
	  b = P2_marking_member_0[2];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[3];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 0, 2, 3, 4 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[2] == P2_marking_member_0[3]) &&
	   (P2_marking_member_0[2] == P2_marking_member_0[4])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[0];
	  b = P2_marking_member_0[2];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[1];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 0, 2, 4, 1 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[2] == P2_marking_member_0[4]) &&
	   (P2_marking_member_0[2] == P2_marking_member_0[1])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[0];
	  b = P2_marking_member_0[2];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[3];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 0, 2, 4, 3 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[2] == P2_marking_member_0[4]) &&
	   (P2_marking_member_0[2] == P2_marking_member_0[3])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[0];
	  b = P2_marking_member_0[2];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[1];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 0, 3, 1, 4 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[3] == P2_marking_member_0[1]) &&
	   (P2_marking_member_0[3] == P2_marking_member_0[4])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[0];
	  b = P2_marking_member_0[3];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[2];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 0, 3, 2, 4 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[3] == P2_marking_member_0[2]) &&
	   (P2_marking_member_0[3] == P2_marking_member_0[4])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[0];
	  b = P2_marking_member_0[3];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[1];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 0, 3, 4, 1 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[3] == P2_marking_member_0[4]) &&
	   (P2_marking_member_0[3] == P2_marking_member_0[1])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[0];
	  b = P2_marking_member_0[3];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[2];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 0, 3, 4, 2 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[3] == P2_marking_member_0[4]) &&
	   (P2_marking_member_0[3] == P2_marking_member_0[2])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[0];
	  b = P2_marking_member_0[3];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[1];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 0, 4, 1, 2 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[4] == P2_marking_member_0[1]) &&
	   (P2_marking_member_0[4] == P2_marking_member_0[2])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[0];
	  b = P2_marking_member_0[4];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[3];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 0, 4, 1, 3 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[4] == P2_marking_member_0[1]) &&
	   (P2_marking_member_0[4] == P2_marking_member_0[3])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[0];
	  b = P2_marking_member_0[4];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[2];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 0, 4, 2, 1 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[4] == P2_marking_member_0[2]) &&
	   (P2_marking_member_0[4] == P2_marking_member_0[1])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[0];
	  b = P2_marking_member_0[4];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[3];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 0, 4, 2, 3 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[4] == P2_marking_member_0[2]) &&
	   (P2_marking_member_0[4] == P2_marking_member_0[3])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[0];
	  b = P2_marking_member_0[4];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[1];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 0, 4, 3, 1 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[4] == P2_marking_member_0[3]) &&
	   (P2_marking_member_0[4] == P2_marking_member_0[1])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[0];
	  b = P2_marking_member_0[4];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[2];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 0, 4, 3, 2 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[4] == P2_marking_member_0[3]) &&
	   (P2_marking_member_0[4] == P2_marking_member_0[2])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[0];
	  b = P2_marking_member_0[4];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[1];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 1, 0, 2, 4 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[0] == P2_marking_member_0[2]) &&
	   (P2_marking_member_0[0] == P2_marking_member_0[4])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[1];
	  b = P2_marking_member_0[0];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[3];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 1, 0, 3, 4 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[0] == P2_marking_member_0[3]) &&
	   (P2_marking_member_0[0] == P2_marking_member_0[4])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[1];
	  b = P2_marking_member_0[0];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[2];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 1, 0, 4, 2 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[0] == P2_marking_member_0[4]) &&
	   (P2_marking_member_0[0] == P2_marking_member_0[2])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[1];
	  b = P2_marking_member_0[0];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[3];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 1, 0, 4, 3 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[0] == P2_marking_member_0[4]) &&
	   (P2_marking_member_0[0] == P2_marking_member_0[3])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[1];
	  b = P2_marking_member_0[0];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[2];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 1, 2, 0, 4 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[2] == P2_marking_member_0[0]) &&
	   (P2_marking_member_0[2] == P2_marking_member_0[4])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[1];
	  b = P2_marking_member_0[2];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[3];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 1, 2, 3, 4 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[2] == P2_marking_member_0[3]) &&
	   (P2_marking_member_0[2] == P2_marking_member_0[4])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[1];
	  b = P2_marking_member_0[2];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 1, 2, 4, 0 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[2] == P2_marking_member_0[4]) &&
	   (P2_marking_member_0[2] == P2_marking_member_0[0])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[1];
	  b = P2_marking_member_0[2];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[3];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 1, 2, 4, 3 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[2] == P2_marking_member_0[4]) &&
	   (P2_marking_member_0[2] == P2_marking_member_0[3])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[1];
	  b = P2_marking_member_0[2];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 1, 3, 0, 4 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[3] == P2_marking_member_0[0]) &&
	   (P2_marking_member_0[3] == P2_marking_member_0[4])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[1];
	  b = P2_marking_member_0[3];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[2];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 1, 3, 2, 4 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[3] == P2_marking_member_0[2]) &&
	   (P2_marking_member_0[3] == P2_marking_member_0[4])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[1];
	  b = P2_marking_member_0[3];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 1, 3, 4, 0 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[3] == P2_marking_member_0[4]) &&
	   (P2_marking_member_0[3] == P2_marking_member_0[0])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[1];
	  b = P2_marking_member_0[3];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[2];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 1, 3, 4, 2 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[3] == P2_marking_member_0[4]) &&
	   (P2_marking_member_0[3] == P2_marking_member_0[2])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[1];
	  b = P2_marking_member_0[3];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 1, 4, 0, 2 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[4] == P2_marking_member_0[0]) &&
	   (P2_marking_member_0[4] == P2_marking_member_0[2])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[1];
	  b = P2_marking_member_0[4];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[3];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 1, 4, 0, 3 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[4] == P2_marking_member_0[0]) &&
	   (P2_marking_member_0[4] == P2_marking_member_0[3])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[1];
	  b = P2_marking_member_0[4];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[2];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 1, 4, 2, 0 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[4] == P2_marking_member_0[2]) &&
	   (P2_marking_member_0[4] == P2_marking_member_0[0])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[1];
	  b = P2_marking_member_0[4];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[3];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 1, 4, 2, 3 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[4] == P2_marking_member_0[2]) &&
	   (P2_marking_member_0[4] == P2_marking_member_0[3])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[1];
	  b = P2_marking_member_0[4];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 1, 4, 3, 0 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[4] == P2_marking_member_0[3]) &&
	   (P2_marking_member_0[4] == P2_marking_member_0[0])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[1];
	  b = P2_marking_member_0[4];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[2];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 1, 4, 3, 2 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[4] == P2_marking_member_0[3]) &&
	   (P2_marking_member_0[4] == P2_marking_member_0[2])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[1];
	  b = P2_marking_member_0[4];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 2, 0, 1, 4 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[0] == P2_marking_member_0[1]) &&
	   (P2_marking_member_0[0] == P2_marking_member_0[4])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[2];
	  b = P2_marking_member_0[0];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[3];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 2, 0, 3, 4 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[0] == P2_marking_member_0[3]) &&
	   (P2_marking_member_0[0] == P2_marking_member_0[4])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[2];
	  b = P2_marking_member_0[0];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[1];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 2, 0, 4, 1 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[0] == P2_marking_member_0[4]) &&
	   (P2_marking_member_0[0] == P2_marking_member_0[1])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[2];
	  b = P2_marking_member_0[0];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[3];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 2, 0, 4, 3 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[0] == P2_marking_member_0[4]) &&
	   (P2_marking_member_0[0] == P2_marking_member_0[3])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[2];
	  b = P2_marking_member_0[0];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[1];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 2, 1, 0, 4 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[1] == P2_marking_member_0[0]) &&
	   (P2_marking_member_0[1] == P2_marking_member_0[4])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[2];
	  b = P2_marking_member_0[1];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[3];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 2, 1, 3, 4 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[1] == P2_marking_member_0[3]) &&
	   (P2_marking_member_0[1] == P2_marking_member_0[4])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[2];
	  b = P2_marking_member_0[1];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 2, 1, 4, 0 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[1] == P2_marking_member_0[4]) &&
	   (P2_marking_member_0[1] == P2_marking_member_0[0])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[2];
	  b = P2_marking_member_0[1];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[3];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 2, 1, 4, 3 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[1] == P2_marking_member_0[4]) &&
	   (P2_marking_member_0[1] == P2_marking_member_0[3])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[2];
	  b = P2_marking_member_0[1];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 2, 3, 0, 4 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[3] == P2_marking_member_0[0]) &&
	   (P2_marking_member_0[3] == P2_marking_member_0[4])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[2];
	  b = P2_marking_member_0[3];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[1];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 2, 3, 1, 4 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[3] == P2_marking_member_0[1]) &&
	   (P2_marking_member_0[3] == P2_marking_member_0[4])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[2];
	  b = P2_marking_member_0[3];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 2, 3, 4, 0 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[3] == P2_marking_member_0[4]) &&
	   (P2_marking_member_0[3] == P2_marking_member_0[0])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[2];
	  b = P2_marking_member_0[3];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[1];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 2, 3, 4, 1 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[3] == P2_marking_member_0[4]) &&
	   (P2_marking_member_0[3] == P2_marking_member_0[1])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[2];
	  b = P2_marking_member_0[3];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 2, 4, 0, 1 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[4] == P2_marking_member_0[0]) &&
	   (P2_marking_member_0[4] == P2_marking_member_0[1])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[2];
	  b = P2_marking_member_0[4];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[3];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 2, 4, 0, 3 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[4] == P2_marking_member_0[0]) &&
	   (P2_marking_member_0[4] == P2_marking_member_0[3])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[2];
	  b = P2_marking_member_0[4];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[1];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 2, 4, 1, 0 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[4] == P2_marking_member_0[1]) &&
	   (P2_marking_member_0[4] == P2_marking_member_0[0])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[2];
	  b = P2_marking_member_0[4];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[3];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 2, 4, 1, 3 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[4] == P2_marking_member_0[1]) &&
	   (P2_marking_member_0[4] == P2_marking_member_0[3])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[2];
	  b = P2_marking_member_0[4];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 2, 4, 3, 0 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[4] == P2_marking_member_0[3]) &&
	   (P2_marking_member_0[4] == P2_marking_member_0[0])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[2];
	  b = P2_marking_member_0[4];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[1];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 2, 4, 3, 1 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[4] == P2_marking_member_0[3]) &&
	   (P2_marking_member_0[4] == P2_marking_member_0[1])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[2];
	  b = P2_marking_member_0[4];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 3, 0, 1, 4 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[0] == P2_marking_member_0[1]) &&
	   (P2_marking_member_0[0] == P2_marking_member_0[4])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[3];
	  b = P2_marking_member_0[0];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[2];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 3, 0, 2, 4 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[0] == P2_marking_member_0[2]) &&
	   (P2_marking_member_0[0] == P2_marking_member_0[4])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[3];
	  b = P2_marking_member_0[0];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[1];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 3, 0, 4, 1 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[0] == P2_marking_member_0[4]) &&
	   (P2_marking_member_0[0] == P2_marking_member_0[1])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[3];
	  b = P2_marking_member_0[0];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[2];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 3, 0, 4, 2 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[0] == P2_marking_member_0[4]) &&
	   (P2_marking_member_0[0] == P2_marking_member_0[2])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[3];
	  b = P2_marking_member_0[0];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[1];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 3, 1, 0, 4 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[1] == P2_marking_member_0[0]) &&
	   (P2_marking_member_0[1] == P2_marking_member_0[4])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[3];
	  b = P2_marking_member_0[1];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[2];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 3, 1, 2, 4 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[1] == P2_marking_member_0[2]) &&
	   (P2_marking_member_0[1] == P2_marking_member_0[4])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[3];
	  b = P2_marking_member_0[1];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 3, 1, 4, 0 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[1] == P2_marking_member_0[4]) &&
	   (P2_marking_member_0[1] == P2_marking_member_0[0])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[3];
	  b = P2_marking_member_0[1];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[2];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 3, 1, 4, 2 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[1] == P2_marking_member_0[4]) &&
	   (P2_marking_member_0[1] == P2_marking_member_0[2])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[3];
	  b = P2_marking_member_0[1];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 3, 2, 0, 4 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[2] == P2_marking_member_0[0]) &&
	   (P2_marking_member_0[2] == P2_marking_member_0[4])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[3];
	  b = P2_marking_member_0[2];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[1];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 3, 2, 1, 4 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[2] == P2_marking_member_0[1]) &&
	   (P2_marking_member_0[2] == P2_marking_member_0[4])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[3];
	  b = P2_marking_member_0[2];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 3, 2, 4, 0 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[2] == P2_marking_member_0[4]) &&
	   (P2_marking_member_0[2] == P2_marking_member_0[0])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[3];
	  b = P2_marking_member_0[2];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[1];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 3, 2, 4, 1 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[2] == P2_marking_member_0[4]) &&
	   (P2_marking_member_0[2] == P2_marking_member_0[1])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[3];
	  b = P2_marking_member_0[2];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 3, 4, 0, 1 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[4] == P2_marking_member_0[0]) &&
	   (P2_marking_member_0[4] == P2_marking_member_0[1])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[3];
	  b = P2_marking_member_0[4];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[2];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 3, 4, 0, 2 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[4] == P2_marking_member_0[0]) &&
	   (P2_marking_member_0[4] == P2_marking_member_0[2])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[3];
	  b = P2_marking_member_0[4];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[1];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 3, 4, 1, 0 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[4] == P2_marking_member_0[1]) &&
	   (P2_marking_member_0[4] == P2_marking_member_0[0])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[3];
	  b = P2_marking_member_0[4];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[2];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 3, 4, 1, 2 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[4] == P2_marking_member_0[1]) &&
	   (P2_marking_member_0[4] == P2_marking_member_0[2])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[3];
	  b = P2_marking_member_0[4];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 3, 4, 2, 0 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[4] == P2_marking_member_0[2]) &&
	   (P2_marking_member_0[4] == P2_marking_member_0[0])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[3];
	  b = P2_marking_member_0[4];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[1];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 3, 4, 2, 1 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[4] == P2_marking_member_0[2]) &&
	   (P2_marking_member_0[4] == P2_marking_member_0[1])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[3];
	  b = P2_marking_member_0[4];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 4, 0, 1, 2 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[0] == P2_marking_member_0[1]) &&
	   (P2_marking_member_0[0] == P2_marking_member_0[2])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[4];
	  b = P2_marking_member_0[0];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[3];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 4, 0, 1, 3 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[0] == P2_marking_member_0[1]) &&
	   (P2_marking_member_0[0] == P2_marking_member_0[3])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[4];
	  b = P2_marking_member_0[0];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[2];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 4, 0, 2, 1 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[0] == P2_marking_member_0[2]) &&
	   (P2_marking_member_0[0] == P2_marking_member_0[1])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[4];
	  b = P2_marking_member_0[0];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[3];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 4, 0, 2, 3 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[0] == P2_marking_member_0[2]) &&
	   (P2_marking_member_0[0] == P2_marking_member_0[3])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[4];
	  b = P2_marking_member_0[0];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[1];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 4, 0, 3, 1 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[0] == P2_marking_member_0[3]) &&
	   (P2_marking_member_0[0] == P2_marking_member_0[1])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[4];
	  b = P2_marking_member_0[0];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[2];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 4, 0, 3, 2 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[0] == P2_marking_member_0[3]) &&
	   (P2_marking_member_0[0] == P2_marking_member_0[2])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[4];
	  b = P2_marking_member_0[0];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[1];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 4, 1, 0, 2 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[1] == P2_marking_member_0[0]) &&
	   (P2_marking_member_0[1] == P2_marking_member_0[2])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[4];
	  b = P2_marking_member_0[1];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[3];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 4, 1, 0, 3 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[1] == P2_marking_member_0[0]) &&
	   (P2_marking_member_0[1] == P2_marking_member_0[3])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[4];
	  b = P2_marking_member_0[1];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[2];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 4, 1, 2, 0 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[1] == P2_marking_member_0[2]) &&
	   (P2_marking_member_0[1] == P2_marking_member_0[0])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[4];
	  b = P2_marking_member_0[1];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[3];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 4, 1, 2, 3 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[1] == P2_marking_member_0[2]) &&
	   (P2_marking_member_0[1] == P2_marking_member_0[3])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[4];
	  b = P2_marking_member_0[1];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 4, 1, 3, 0 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[1] == P2_marking_member_0[3]) &&
	   (P2_marking_member_0[1] == P2_marking_member_0[0])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[4];
	  b = P2_marking_member_0[1];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[2];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 4, 1, 3, 2 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[1] == P2_marking_member_0[3]) &&
	   (P2_marking_member_0[1] == P2_marking_member_0[2])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[4];
	  b = P2_marking_member_0[1];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 4, 2, 0, 1 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[2] == P2_marking_member_0[0]) &&
	   (P2_marking_member_0[2] == P2_marking_member_0[1])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[4];
	  b = P2_marking_member_0[2];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[3];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 4, 2, 0, 3 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[2] == P2_marking_member_0[0]) &&
	   (P2_marking_member_0[2] == P2_marking_member_0[3])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[4];
	  b = P2_marking_member_0[2];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[1];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 4, 2, 1, 0 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[2] == P2_marking_member_0[1]) &&
	   (P2_marking_member_0[2] == P2_marking_member_0[0])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[4];
	  b = P2_marking_member_0[2];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[3];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 4, 2, 1, 3 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[2] == P2_marking_member_0[1]) &&
	   (P2_marking_member_0[2] == P2_marking_member_0[3])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[4];
	  b = P2_marking_member_0[2];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 4, 2, 3, 0 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[2] == P2_marking_member_0[3]) &&
	   (P2_marking_member_0[2] == P2_marking_member_0[0])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[4];
	  b = P2_marking_member_0[2];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[1];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 4, 2, 3, 1 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[2] == P2_marking_member_0[3]) &&
	   (P2_marking_member_0[2] == P2_marking_member_0[1])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[4];
	  b = P2_marking_member_0[2];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 4, 3, 0, 1 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[3] == P2_marking_member_0[0]) &&
	   (P2_marking_member_0[3] == P2_marking_member_0[1])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[4];
	  b = P2_marking_member_0[3];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[2];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 4, 3, 0, 2 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[3] == P2_marking_member_0[0]) &&
	   (P2_marking_member_0[3] == P2_marking_member_0[2])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[4];
	  b = P2_marking_member_0[3];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[1];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 4, 3, 1, 0 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[3] == P2_marking_member_0[1]) &&
	   (P2_marking_member_0[3] == P2_marking_member_0[0])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[4];
	  b = P2_marking_member_0[3];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[2];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 4, 3, 1, 2 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[3] == P2_marking_member_0[1]) &&
	   (P2_marking_member_0[3] == P2_marking_member_0[2])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[4];
	  b = P2_marking_member_0[3];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 4, 3, 2, 0 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[3] == P2_marking_member_0[2]) &&
	   (P2_marking_member_0[3] == P2_marking_member_0[0])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[4];
	  b = P2_marking_member_0[3];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_marking_member_0[0] = P2_marking_member_0[1];
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}


      /* Permutation for Place P2 : 4, 3, 2, 1 */
      /* Transition T2 */
      if ((P2_is_marked >= 5) &&
	  ((P3_is_marked + 3) <= 6) &&
	  ((P2_marking_member_0[3] == P2_marking_member_0[2]) &&
	   (P2_marking_member_0[3] == P2_marking_member_0[1])))
	{

	  long a;
	  long b;
	  long c;

	  a = P2_marking_member_0[4];
	  b = P2_marking_member_0[3];

	  /* Transition condition */
	  if ((b > a))
	    {

	      /* demarking of input places */
	      P2_is_marked -= 4;

	      /* preaction */
	      c = a + b;

	      /* marking of output places */
	      P3_marking_member_0[P3_is_marked + 0] = a;
	      P3_marking_member_0[P3_is_marked + 1] = b;
	      P3_marking_member_0[P3_is_marked + 2] = c;
	      P3_is_marked += 3;

	    }			/* end of if (Transition condition) */
	}
    }

  return 0;
}


int
verify_benchmark (int unused)
{
  int expP1_is_marked = 3;
  long expP1_marking_member_0[3] = { 0, 0, 0 };
  int expP2_is_marked = 5;
  long expP2_marking_member_0[5] = { 0, 0, 0, 0, 0 };
  int expP3_is_marked = 0;
  long expP3_marking_member_0[6] = { 0, 0, 0, 0, 0, 0 };
  int i;
  if (expP1_is_marked != P1_is_marked ||
      expP2_is_marked != P2_is_marked || expP3_is_marked != P3_is_marked)
    {
      return 0;
    }
  // for (i=0; i<3; i++)
  //   printf("%ld, ", P1_marking_member_0[i]);
  // printf("\n");
  // for (i=0; i<5; i++)
  //   printf("%ld, ", P2_marking_member_0[i]);
  // printf("\n");
  // for (i=0; i<6; i++)
  //   printf("%ld, ", P3_marking_member_0[i]);
  // printf("\n");
  for (i = 0; i < 3; i++)
    {
      if (expP1_marking_member_0[i] != P1_marking_member_0[i] ||
	  expP2_marking_member_0[i] != P2_marking_member_0[i] ||
	  expP3_marking_member_0[i] != P3_marking_member_0[i])
	{
	  return 0;
	}
    }
  for (i = 3; i < 5; i++)
    {
      if (expP2_marking_member_0[i] != P2_marking_member_0[i] ||
	  expP3_marking_member_0[i] != P3_marking_member_0[i])
	{
	  return 0;
	}
    }
  for (i = 5; i < 6; i++)
    {
      if (expP3_marking_member_0[i] != P3_marking_member_0[i])
	{
	  return 0;
	}
    }

  return 1;
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
